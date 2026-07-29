#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import numpy as np
from rknn.api import RKNN


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--tflite", type=str, default="resnet-20l-model_fp32.tflite", help="input .tflite path (FP32)")
    ap.add_argument("--out", type=str, default="../RK-model/resnet-20l-model_fp16.rknn", help="output .rknn path")
    ap.add_argument("--target", type=str, default="rk3588", help="rk3588/rk3568/rv1106/...")
    ap.add_argument("--no_test", action="store_true", help="disable simulator runtime test")
    args = ap.parse_args()

    rknn = RKNN(verbose=True)

    # 1) config（指定 target_platform；你的輸入不是影像，通常不需要 mean/std）
    # 參考：rknn.config(...) / target_platform / mean_values / std_values / float_dtype 
    print("--> config")
    rknn.config(
        target_platform=args.target,
        disable_rules=['fuse_mul_into_gemm'],
        # 你是特徵輸入(x,y)，不做影像前處理，所以保持預設即可
        # mean_values / std_values 不填：等同 mean=0, std=1 :contentReference[oaicite:2]{index=2}
    )

    # 2) load_tflite
    # 參考：rknn.load_tflite(model=...) :contentReference[oaicite:3]{index=3}
    print("--> load_tflite")
    ret = rknn.load_tflite(model=args.tflite)
    if ret != 0:
        rknn.release()
        raise RuntimeError("load_tflite failed")

    # 3) build：先不量化（FP16 路線）
    # 參考：rknn.build(do_quantization=...) :contentReference[oaicite:4]{index=4}
    print("--> build (quant, fp16 path)")
    ret = rknn.build(do_quantization=False)
    if ret != 0:
        rknn.release()
        raise RuntimeError("build failed")

    # 4) export_rknn
    # 參考：rknn.export_rknn(export_path=...) :contentReference[oaicite:5]{index=5}
    print("--> export_rknn")
    ret = rknn.export_rknn(export_path=args.out)
    if ret != 0:
        rknn.release()
        raise RuntimeError("export_rknn failed")

    # 5) 可選：init_runtime + inference（PC 上 simulator 測一下）
    # 參考：init_runtime / inference API 
    if not args.no_test:
        print("--> init_runtime (simulator)")
        ret = rknn.init_runtime()
        if ret != 0:
            rknn.release()
            raise RuntimeError("init_runtime failed")

        # 模型輸入是 (1,5)
        x = np.array([[66.383949, 33.168117, 353.36923, 0.14657, 0.07323]], dtype=np.float32)
        print("--> inference (simulator)")
        outputs = rknn.inference(inputs=[x])
        print("[OK] outputs[0] =", outputs[0])

    rknn.release()
    print("[DONE] wrote:", args.out)


if __name__ == "__main__":
    main()
