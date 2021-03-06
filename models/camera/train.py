import os
import numpy as np
from keras.optimizers import *
from keras.preprocessing import image
from keras.preprocessing.image import ImageDataGenerator
from keras import backend as K
from keras.callbacks import *
from matplotlib import pyplot
from keras.layers import Input
# from keras.utils.vis_utils import plot_model
from glob import glob

# from tvn import TVN
# from tvn_shuffle import TVN
# from fall_detect_camera import FDC
from fall_detect import FDC

####################################################
from tensorflow.compat.v1 import ConfigProto
from tensorflow.compat.v1 import InteractiveSession


def fix_gpu():
    config = ConfigProto()
    config.gpu_options.allow_growth = True
    session = InteractiveSession(config=config)


fix_gpu()
####################################################

seed_n = 7
batch_size = 32
epochs = 50

img_width = 80
img_height = 60

# data_root = "/media/go/02BCD8A4BCD8940F/Users/visiongo/Desktop/github/Fall_detect/data"
data_root = "data"
model_save_path = "checkpoint"

train_path = os.path.join(data_root, "train")
test_path = os.path.join(data_root, "test")
trainset_size = len(glob("{}/*/*".format(train_path)))
testset_size = len(glob("{}/*/*".format(test_path)))

trainset_size = 4800
testset_size = 1200

print(trainset_size, testset_size, trainset_size//batch_size, testset_size//batch_size)

trainDataGen = ImageDataGenerator(
    rescale=1./255,
    # featurewise_center=True,
    # featurewise_std_normalization=True,
    horizontal_flip=True,
    channel_shift_range=10,
    width_shift_range=[0, 0.5],
    brightness_range=[0.3,1.0],
)

testDataGen = ImageDataGenerator(rescale=1./255)

train_data = trainDataGen.flow_from_directory(
    train_path,
    target_size=(img_height*2, img_width),
    class_mode='categorical',
    batch_size=batch_size,
    color_mode='grayscale',
    shuffle=True,
    seed=seed_n,
)

test_data = testDataGen.flow_from_directory(
    test_path,
    target_size=(img_height*2, img_width),
    class_mode='categorical',
    batch_size=batch_size,
    color_mode='grayscale',
    shuffle=True,
    seed=seed_n,
)


def build_model(num_frames=2, batch_size=64, outputs=10):
    # inputs = Input(shape=(num_frames*img_height, img_width, 1))
    # model = TVN(inputs, outputs)
    model = FDC(outputs)
    model.compile(optimizer="adam", loss="categorical_crossentropy", metrics=['acc'])
    return model


def train(bs=64):
    model = build_model(num_frames=2, batch_size=bs, outputs=3)
    callbacks = [
        ModelCheckpoint(
            os.path.join(model_save_path, "fd_cam_ep{epoch:02d}_val_acc{val_acc:.4f}_val_loss{val_loss:0.4f}.h5"),
            monitor='val_loss',
            mode='auto',
            save_best_only=True,
        )
    ]
    history = model.fit(
        train_data,
        steps_per_epoch=trainset_size//batch_size,
        epochs=epochs,
        validation_data=test_data,
        validation_steps=testset_size//batch_size,
        shuffle=False,
        callbacks=callbacks,
        workers=1,
    )
    return history


def plot_history(history):
    pyplot.plot(history.history['acc'], label='acc')
    pyplot.plot(history.history['val_acc'], label='val_acc')
    pyplot.plot(history.history['loss'], label='loss')
    pyplot.plot(history.history['val_loss'], label='val_loss')
    pyplot.legend()
    pyplot.show()


if __name__ == '__main__':
    history = train(batch_size)
    plot_history(history)
