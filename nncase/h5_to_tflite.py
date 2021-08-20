import tensorflow as tf


def h5_to_tflite(saved_model_dir, tflite_path='model.tflite'):
    model = tf.keras.models.load_model(saved_model_dir)
    converter = tf.lite.TFLiteConverter.from_keras_model(model)
    tflite_model = converter.convert()
    open(tflite_path, "wb").write(tflite_model)


if __name__ == "__main__":
    saved_model_dir = "test.h5"
    tflite_path = "save_model/tflite/test.tflite"
    h5_to_tflite(saved_model_dir, tflite_path)
