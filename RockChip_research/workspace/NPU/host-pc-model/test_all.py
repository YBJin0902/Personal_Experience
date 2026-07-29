#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import json
import numpy as np
import pandas as pd
import tensorflow as tf
import keras


# ============================================================
# 1. 這裡直接寫死你要測試的輸入數值
#    格式：X, Y, Lv, x, y
# ============================================================

TEST_SAMPLES = [
    {
        "sample_name": "test_001_red",
        "X": 88.872749,
        "Y": 55.711574,
        "Lv": 10.270649,
        "x": 0.57365,
        "y": 0.35952
    },
    {
        "sample_name": "test_002_green",
        "X": 103.12017,
        "Y": 163.64398,
        "Lv": 25.796300,
        "x": 0.35236,
        "y": 0.55925
    },
    {
        "sample_name": "test_003_blue",
        "X": 59.736846,
        "Y": 50.402309,
        "Lv": 267.34234,
        "x": 0.15827,
        "y": 0.13353
    },
    {
        "sample_name": "test_004_black",
        "X": 0.2490194,
        "Y": 0.2485646,
        "Lv": 0.4186166,
        "x": 0.27176,
        "y": 0.27127
    },
    {
        "sample_name": "test_005_white",
        "X": 255.00328,
        "Y": 273.92984,
        "Lv": 298.85488,
        "x": 0.30804,
        "y": 0.33089
    },

    # 如果你要一次測多筆，可以繼續往下加
    # {
    #     "sample_name": "test_002",
    #     "X": 10.2,
    #     "Y": 8.5,
    #     "Lv": 30.0,
    #     "x": 0.1500,
    #     "y": 0.0600,
    # },
]


# ============================================================
# 2. 模型資料夾與輸出 Excel 檔名
# ============================================================

MODEL_DIR = "model"
OUTPUT_EXCEL = "all_model_test_report.xlsx"


# ============================================================
# 3. 要測試的模型清單
# ============================================================

MODEL_LIST = [
    {
        "model_name": "MLP",
        "best_model": "mlp-best.keras",
        "final_model": "mlp-final.keras",
        "label_file": "mlp-labels.json",
    },
    {
        "model_name": "ResNet-5L",
        "best_model": "resnet-5l-best.keras",
        "final_model": "resnet-5l-final.keras",
        "label_file": "resnet-5l-labels.json",
    },
    {
        "model_name": "ResNet-10L",
        "best_model": "resnet-10l-best.keras",
        "final_model": "resnet-10l-final.keras",
        "label_file": "resnet-10l-labels.json",
    },
    {
        "model_name": "ResNet-20L",
        "best_model": "resnet-20l-best.keras",
        "final_model": "resnet-20l-final.keras",
        "label_file": "resnet-20l-labels.json",
    },
]


DEFAULT_LABELS = [
    "black", "blue", "cyan", "green", "orange",
    "pink", "purple", "red", "white", "yellow"
]


def load_labels(label_path):
    if os.path.exists(label_path):
        with open(label_path, "r", encoding="utf-8") as f:
            return json.load(f)

    print(f"[Warning] 找不到 label 檔案：{label_path}")
    print("[Warning] 使用預設 label 順序")
    return DEFAULT_LABELS


def build_input_array(sample):
    values = [
        sample["X"],
        sample["Y"],
        sample["Lv"],
        sample["x"],
        sample["y"],
    ]

    return np.array([values], dtype=np.float32)


def predict_model(model_path, labels, sample):
    model = keras.models.load_model(model_path)

    x_input = build_input_array(sample)
    pred = model.predict(x_input, verbose=0)[0]

    best_idx = int(np.argmax(pred))
    best_label = labels[best_idx]
    best_prob = float(pred[best_idx])

    sorted_idx = np.argsort(pred)[::-1]

    top1_idx = int(sorted_idx[0])
    top2_idx = int(sorted_idx[1])
    top3_idx = int(sorted_idx[2])

    result = {
        "predict_label": best_label,
        "confidence": best_prob,

        "top1_label": labels[top1_idx],
        "top1_prob": float(pred[top1_idx]),

        "top2_label": labels[top2_idx],
        "top2_prob": float(pred[top2_idx]),

        "top3_label": labels[top3_idx],
        "top3_prob": float(pred[top3_idx]),

        "all_probabilities": {
            labels[i]: float(pred[i]) for i in range(len(labels))
        }
    }

    return result


def main():
    print("TensorFlow:", tf.__version__)
    print("Keras:", keras.__version__)
    print("Output Excel:", OUTPUT_EXCEL)

    summary_rows = []
    probability_rows = []
    top3_rows = []

    for model_info in MODEL_LIST:
        model_name = model_info["model_name"]

        label_path = os.path.join(MODEL_DIR, model_info["label_file"])
        labels = load_labels(label_path)

        test_targets = [
            {
                "model_type": "best",
                "model_file": model_info["best_model"],
            },
            {
                "model_type": "final",
                "model_file": model_info["final_model"],
            },
        ]

        for target in test_targets:
            model_type = target["model_type"]
            model_file = target["model_file"]
            model_path = os.path.join(MODEL_DIR, model_file)

            if not os.path.exists(model_path):
                print(f"[Skip] 找不到模型：{model_path}")
                continue

            print(f"\n[Load] {model_name} - {model_type}")
            print(f"       {model_path}")

            for sample in TEST_SAMPLES:
                result = predict_model(model_path, labels, sample)

                base_info = {
                    "sample_name": sample["sample_name"],
                    "model_name": model_name,
                    "model_type": model_type,
                    "model_file": model_file,
                    "X": sample["X"],
                    "Y": sample["Y"],
                    "Lv": sample["Lv"],
                    "x": sample["x"],
                    "y": sample["y"],
                }

                summary_rows.append({
                    **base_info,
                    "predict_label": result["predict_label"],
                    "confidence": result["confidence"],
                    "top1_label": result["top1_label"],
                    "top1_prob": result["top1_prob"],
                    "top2_label": result["top2_label"],
                    "top2_prob": result["top2_prob"],
                    "top3_label": result["top3_label"],
                    "top3_prob": result["top3_prob"],
                })

                top3_rows.append({
                    **base_info,
                    "rank": 1,
                    "label": result["top1_label"],
                    "probability": result["top1_prob"],
                })

                top3_rows.append({
                    **base_info,
                    "rank": 2,
                    "label": result["top2_label"],
                    "probability": result["top2_prob"],
                })

                top3_rows.append({
                    **base_info,
                    "rank": 3,
                    "label": result["top3_label"],
                    "probability": result["top3_prob"],
                })

                prob_row = {
                    **base_info,
                    "predict_label": result["predict_label"],
                    "confidence": result["confidence"],
                }

                for label, prob in result["all_probabilities"].items():
                    prob_row[label] = prob

                probability_rows.append(prob_row)

                print(
                    f"[Result] {sample['sample_name']} | "
                    f"{model_name}-{model_type} | "
                    f"{result['predict_label']} | "
                    f"{result['confidence']:.6f}"
                )

    df_summary = pd.DataFrame(summary_rows)
    df_prob = pd.DataFrame(probability_rows)
    df_top3 = pd.DataFrame(top3_rows)

    with pd.ExcelWriter(OUTPUT_EXCEL, engine="openpyxl") as writer:
        df_summary.to_excel(writer, sheet_name="Summary", index=False)
        df_prob.to_excel(writer, sheet_name="All_Probabilities", index=False)
        df_top3.to_excel(writer, sheet_name="Top3", index=False)

        workbook = writer.book

        for sheet_name in writer.sheets:
            ws = writer.sheets[sheet_name]

            ws.freeze_panes = "A2"

            for col in ws.columns:
                max_length = 0
                col_letter = col[0].column_letter

                for cell in col:
                    value = cell.value
                    if value is not None:
                        max_length = max(max_length, len(str(value)))

                ws.column_dimensions[col_letter].width = min(max_length + 2, 25)

            for cell in ws[1]:
                cell.font = cell.font.copy(bold=True)

    print("\n====================================")
    print("測試完成")
    print(f"Excel 報告已輸出：{OUTPUT_EXCEL}")
    print("====================================")


if __name__ == "__main__":
    main()