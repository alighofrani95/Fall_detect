import tensorflow as tf

# Convert the model
converter = tf.lite.TFLiteConverter.from_saved_model('checkpoint/fd_cam_shuffle_ep21_val_acc1.00_val_loss0.01.tflite') # path to the SavedModel directory
tflite_model = converter.convert()

# Save the model.
with open('save_model/tflite/fd_cam_9971.tflite', 'wb') as f:
  f.write(tflite_model)