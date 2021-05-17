import numpy as np
from keras.optimizers import *
from keras.preprocessing import image
from keras.preprocessing.image import ImageDataGenerator
from keras import backend as K
from keras.callbacks import *
from matplotlib import pyplot
from keras.layers import Input
from keras.utils import plot_model
from glob import glob

from tvn import TVN


seed_n = 7
batch_size = 128
train_path = "data/train"
test_path = "data/test"
trainset_size = len(glob("{}/*".format(train_path)))
testset_size = len(glob("{}/*".format(test_path)))
epochs = 100


trainDataGen = ImageDataGenerator(
    rescale=1./255,
    featurewise_center=True,
    featurewise_std_normalization=True,
    horizontal_flip=True,
    brightness_range=[0.2,1.0],
)

testDataGen = ImageDataGenerator(rescale=1./255)

train_data = trainDataGen.flow_from_directory(
    train_path,
    target_size=(480, 320, 1),
    class_mode='categorical',
    batch_size=batch_size,
    color_mode='grayscale',
    shuffle=True,
    seed=seed_n,
)

test_data = testDataGen.flow_from_directory(
    test_path,
    target_size=(480, 320, 1),
    class_mode='categorical',
    batch_size=batch_size,
    color_mode='grayscale',
    shuffle=True,
    seed=seed_n,
)


def build_model(num_frames=2, batch_size=128, outputs=10):
    inputs = Input(shape=(num_frames*240, 320), batch_size=batch_size)
    model = TVN(inputs, outputs)
    model.compile(optimizer=RMSprop(), loss="categorical_crossentropy", metrics=['acc'])
    return model


def train(bs=128):
    model = build_model(num_frames=2, batch_size=bs, outputs=4)
    # model.summary()
    # plot_model(
    #     model,
    #     to_file='TVN_nf2_bs128.png',
    #     show_layer_names=True,
    #     show_shapes=True
    # )
    callbacks = [ModelCheckpoint("fd_camera.h5", monitor='val_loss', mode='min', save_best_only=True)]
    history = model.fit(
        train_data,
        steps_per_epoch=trainset_size//batch_size,
        epochs=epochs,
        validation_data=test_data,
        validation_steps=testset_size//batch_size,
        shuffle=True,
        callbacks=callbacks,
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
