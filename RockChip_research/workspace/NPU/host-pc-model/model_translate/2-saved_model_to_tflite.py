import tensorflow as tf

SM_DIR = "resnet-20l-export_savedmodel_single"
OUT_TFLITE = "resnet-20l-model_fp32.tflite"

converter = tf.lite.TFLiteConverter.from_saved_model(SM_DIR)

# 你這模型是 Dense / Norm，轉換通常直接成功
converter.optimizations = []     # 先不做量化
tflite_model = converter.convert()

open(OUT_TFLITE, "wb").write(tflite_model)
print("Wrote:", OUT_TFLITE)
