#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import glob
import argparse
import pandas as pd

LABELS_9 = ["black","blue","cyan","green","orange","pink","purple","red","white","yellow"]
NEEDED_COLS = ["X", "Y", "Lv", "x", "y"]

def ensure_dir(p: str):
    os.makedirs(p, exist_ok=True)

def normalize_cols(df: pd.DataFrame) -> pd.DataFrame:
    # 去空白，避免 " X" 這種
    df = df.copy()
    df.columns = [c.strip() for c in df.columns]
    return df

def split_one_csv(csv_path: str, out_dir: str, prefix: str, strict: bool, add_index_col: bool):
    df = pd.read_csv(csv_path)
    df = normalize_cols(df)

    missing = [c for c in NEEDED_COLS if c not in df.columns]
    if missing:
        msg = f"[ERROR] {csv_path} 缺少欄位 {missing}，目前欄位={list(df.columns)}"
        if strict:
            raise ValueError(msg)
        else:
            print(msg)
            return 0

    # 只保留你要的 5 個欄位（順序固定）
    df = df[NEEDED_COLS]

    # 如果原本有 n 欄位想保留，你可以自己加；這裡用檔名索引即可
    n_rows = len(df)
    ensure_dir(out_dir)

    for i in range(n_rows):
        row = df.iloc[[i]].copy()  # 仍然是 DataFrame，to_csv 會有欄位名
        if add_index_col:
            row.insert(0, "n", i)

        out_path = os.path.join(out_dir, f"{prefix}_{i:04d}.csv")
        row.to_csv(out_path, index=False)

    return n_rows

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in_root", type=str, default="train_raw/",
                    help="原始資料根目錄，預期有 9 類資料夾：in_root/red/*.csv ...")
    ap.add_argument("--out_root", type=str, default="train/",
                    help="輸出根目錄：out_root/red/ 會產生 1000 個單筆csv")
    ap.add_argument("--glob", type=str, default="*.csv",
                    help="每個顏色資料夾內要處理哪些檔案（預設 *.csv）")
    ap.add_argument("--strict", action="store_true",
                    help="若缺欄位就直接停止（建議開）")
    ap.add_argument("--add_n", action="store_true",
                    help="在單筆檔案內額外加 n 欄位（0..999）")
    ap.add_argument("--prefix_mode", type=str, default="source",
                    choices=["source", "label"],
                    help="輸出檔名前綴：source=用原檔名；label=用顏色名")
    args = ap.parse_args()

    total_files = 0
    total_rows = 0

    for lab in LABELS_9:
        in_dir = os.path.join(args.in_root, lab)
        if not os.path.isdir(in_dir):
            msg = f"[WARN] 找不到資料夾：{in_dir}"
            if args.strict:
                raise FileNotFoundError(msg)
            print(msg)
            continue

        in_files = sorted(glob.glob(os.path.join(in_dir, args.glob)))
        if not in_files:
            print(f"[WARN] {lab} 沒有符合的檔案：{os.path.join(in_dir, args.glob)}")
            continue

        out_dir = os.path.join(args.out_root, lab)
        ensure_dir(out_dir)

        for csv_path in in_files:
            base = os.path.splitext(os.path.basename(csv_path))[0]
            prefix = base if args.prefix_mode == "source" else lab

            n = split_one_csv(
                csv_path=csv_path,
                out_dir=out_dir,
                prefix=prefix,
                strict=args.strict,
                add_index_col=args.add_n,
            )
            total_files += 1
            total_rows += n
            print(f"[OK] {lab}: {os.path.basename(csv_path)} -> {n} files")

    print(f"\n[Done] processed_csv_files={total_files}, generated_single_sample_files={total_rows}")

if __name__ == "__main__":
    main()
