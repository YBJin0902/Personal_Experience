#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os, glob, json, argparse, random
import numpy as np
import pandas as pd
import tensorflow as tf
import keras

LABELS_10 = ["black","blue","cyan","green","orange","pink","purple","red","white","yellow"]
REQ_COLS = ["X", "Y", "Lv", "x", "y"]

def set_seed(seed=42):
    random.seed(seed); np.random.seed(seed); tf.random.set_seed(seed)

def ensure_dir(p): os.makedirs(p, exist_ok=True)

def print_env():
    print("TensorFlow:", tf.__version__)
    print("Keras:", keras.__version__)
    print("GPUs:", tf.config.list_physical_devices("GPU"))

def _strip_cols(df: pd.DataFrame) -> pd.DataFrame:
    df = df.copy()
    df.columns = [c.strip() for c in df.columns]
    return df

def read_one_row_xy(csv_path: str) -> np.ndarray:
    df = pd.read_csv(csv_path)
    df = _strip_cols(df)

    missing = [c for c in REQ_COLS if c not in df.columns]

    if missing:
        raise ValueError(f"CSV缺少欄位 {missing}: {csv_path}, cols={list(df.columns)}")
    if len(df) != 1:
        raise ValueError(f"預期單筆CSV(1 row)，但 {csv_path} 有 {len(df)} rows")

    row = df.loc[df.index[0]]

    X  = float(row["X"])
    Y  = float(row["Y"])
    Lv = float(row["Lv"])
    x  = float(row["x"])
    y  = float(row["y"])

    return np.array([X, Y, Lv, x, y], dtype=np.float32)

def scan_dataset(data_root: str):
    X_list, y_list = [], []

    for lab in LABELS_10:
        d = os.path.join(data_root, lab)

        if not os.path.isdir(d):
            raise FileNotFoundError(f"缺少資料夾：{d}")

        files = sorted(glob.glob(os.path.join(d, "*.csv")))

        if not files:
            raise RuntimeError(f"{lab} 沒有任何 csv：{d}/*.csv")

        for f in files:
            X_list.append(read_one_row_xy(f))
            y_list.append(LABELS_10.index(lab))

        print(f"[Loaded] {lab}: {len(files)} samples")

    X = np.stack(X_list, axis=0).astype(np.float32)   # (N,2)
    y = np.array(y_list, dtype=np.int32)              # (N,)

    return X, y

def split_train_val(X, y, val_ratio=0.15, seed=42):
    rng = np.random.default_rng(seed)
    idx = np.arange(len(X))
    rng.shuffle(idx)
    n_val = int(round(len(X) * val_ratio))
    val_idx = idx[:n_val]
    tr_idx  = idx[n_val:]
    return X[tr_idx], y[tr_idx], X[val_idx], y[val_idx]

def build_model(n_classes=10, hidden=64, dropout=0.2):
    inp = keras.Input(shape=(5,), name="features")

    x = keras.layers.Normalization(axis=-1, name="norm")(inp)

    # stem
    x = keras.layers.Dense(hidden, activation="relu", name="fc1")(x)
    x = keras.layers.Dropout(dropout, name="drop1")(x)

    def res_block(x, name):
        skip = x
        x = keras.layers.Dense(hidden, activation="relu", name=f"{name}_fc1")(x)
        x = keras.layers.Dropout(dropout, name=f"{name}_drop")(x)
        x = keras.layers.Dense(hidden, activation=None, name=f"{name}_fc2")(x)
        x = keras.layers.Add(name=f"{name}_add")([skip, x])
        x = keras.layers.Activation("relu", name=f"{name}_relu")(x)
        return x

    for i in range(5) :
        x = res_block(x, f"rb{i}")

    # head
    x = keras.layers.Dense(hidden, activation="relu", name="fc2")(x)
    x = keras.layers.Dropout(dropout, name="drop2")(x)

    out = keras.layers.Dense(n_classes, activation="softmax", name="cls")(x)
    return keras.Model(inp, out)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--data_root", type=str, default="dataset/train/",
                    help="dataset/train/<label>/*.csv（每檔一筆，含X Y Lv x y）")
    ap.add_argument("--out_dir", type=str, default="model")

    ap.add_argument("--val_ratio", type=float, default=0.15)
    ap.add_argument("--batch", type=int, default=80)
    ap.add_argument("--epochs", type=int, default=100)
    ap.add_argument("--seed", type=int, default=42)
    ap.add_argument("--lr", type=float, default=1e-3)

    ap.add_argument("--hidden", type=int, default=128)
    ap.add_argument("--dropout", type=float, default=0.5)
    args = ap.parse_args()

    set_seed(args.seed)
    print_env()
    ensure_dir(args.out_dir)
    ensure_dir(os.path.join(args.out_dir, "tb"))

    X, y = scan_dataset(args.data_root)
    Xtr, ytr, Xva, yva = split_train_val(X, y, args.val_ratio, args.seed)
    print(f"[Split] train={len(Xtr)} val={len(Xva)}")

    model = build_model(n_classes=len(LABELS_10), hidden=args.hidden, dropout=args.dropout)

    # adapt normalization
    norm = model.get_layer("norm")
    _ = norm(tf.zeros((1,5), dtype=tf.float32))
    norm.adapt(Xtr)

    model.compile(
        optimizer=keras.optimizers.Adam(args.lr),
        loss=keras.losses.SparseCategoricalCrossentropy(),
        metrics=[
            keras.metrics.SparseCategoricalAccuracy(name="accuracy"),
            keras.metrics.SparseTopKCategoricalAccuracy(k=3, name="top3_acc"),
        ],
    )

    callbacks = [
        keras.callbacks.TensorBoard(log_dir=os.path.join(args.out_dir, "tb"), histogram_freq=0),
        keras.callbacks.ModelCheckpoint(os.path.join(args.out_dir, "resnet-5l-best.keras"),
                                        monitor="val_accuracy", mode="max",
                                        save_best_only=True, verbose=1),
        keras.callbacks.ReduceLROnPlateau(monitor="val_loss", mode="min",
                                          factor=0.5, patience=5, min_lr=1e-6, verbose=1),
    ]

    model.summary()
    hist = model.fit(
        Xtr, ytr,
        validation_data=(Xva, yva),
        epochs=args.epochs,
        batch_size=args.batch,
        callbacks=callbacks,
        verbose=1,
        shuffle=True,
    )

    with open(os.path.join(args.out_dir, "resnet-5l-labels.json"), "w", encoding="utf-8") as f:
        json.dump(LABELS_10, f, indent=2, ensure_ascii=False)

    with open(os.path.join(args.out_dir, "resnet-5l-history.json"), "w", encoding="utf-8") as f:
        json.dump(hist.history, f, indent=2, ensure_ascii=False)

    model.save(os.path.join(args.out_dir, "resnet-5l-final.keras"))
    print(f"\n[Saved] {args.out_dir}/resnet-5l-best.keras, resnet-5l-final.keras, resnet-5l-labels.json")

if __name__ == "__main__":
    main()
