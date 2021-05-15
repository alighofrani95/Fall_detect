import numpy as np
from keras.preprocessing import image
from keras.preprocessing.image import ImageDataGenerator
from keras import backend as K
from keras.callbacks import *
from matplotlib.pyplot as plt

from models.camera import TVN


def gen():
    pass


def build_model(num_frames=2, batch_size=128):
    inputs = Input(shape=(num_frames*240, 320, 3), batch_size=batch_size)
    model = TVN(inputs, 3)
    model.summary()
    return model


if __name__ == '__main__':
    model = build_model()
    plot_model(
        model,
        to_file='TVN_nf2_bs128.png',
        show_layer_names=True,
        show_shapes=True
    )
