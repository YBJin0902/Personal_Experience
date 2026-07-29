import keras

IN_KERAS = "../model/resnet-20l-best.keras"
OUT_SM   = "resnet-20l-export_savedmodel_single"

model = keras.models.load_model(IN_KERAS, compile=False)
model.export(OUT_SM)   # Keras 3 正式輸出 SavedModel
print("SavedModel exported to:", OUT_SM)
