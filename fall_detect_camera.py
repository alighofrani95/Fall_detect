# fall_detect_camera.py
import keras
import numpy as np
from keras.layers import *
from keras.models import *
from keras.losses import *
from keras.metrics import *
from keras.backend import *
from keras.callbacks import *
from keras.optimizers import *
from keras.activations import *
from keras.initializers import *


BATCH_NORM_DECAY = 0.9
BATCH_NORM_EPSILON = 1e-5

def slice(x, h1, h2, w1, w2):
    return x[:, h1:h2, w1:w2, :]


def ReLU6():
    x = ReLU(6.)(x)
    return x


def hard_swish(x):
    x = x * (ReLU6(x + 3.) / 6.)
    return x


def Block1(inputs):
    x = Conv2D(
        32,
        kernel_size=3,
        strides=1,
        padding="same",
        data_format="channels_last",
    )(inputs)
    x = Conv1D(
        32,
        kernel_size=3,
        strides=1,
        padding="same",
        data_format="channels_last",
    )(x)
    x = SEBasicBlock(x)
    return x


def Block2(inputs):
    x = Conv2D(
        32,
        kernel_size=3,
        strides=1,
        padding="same",
        data_format="channels_last",
    )(inputs)
    x = BatchNormalization()(x)
    x = ReLU()(x)

    x = Conv2D()(x)
    x = BatchNormalization()(x)
    x = hard_swish(x)

    x = DepthwiseConv2D()(x)
    x = BatchNormalization()(x)
    x = hard_swish(x)

    x = MaxPooling2D()(x)
    # x = SEBottleneck(x)
    return x


def SE_Block(inputs, ratio=16):
    channels = int_shape(inputs)[-1]
    x = GlobalAveragePooling2D()(inputs)
    x = Dense(
        channels // ratio,
        activation='relu',
        kernel_initializer='he_normal',
        use_bias=False
    )(x)
    x = Dense(
        channels,
        activation='sigmoid',
        kernel_initializer='he_normal',
        use_bias=False
    )(x)
    x = Multiply()([inputs, x])
    return x


def SEBasicBlock(inputs):
    x = Conv2D()(inputs)
    x = BatchNormalization()(x)
    x = ReLU()(x)

    x = Conv2D()(x)
    x = BatchNormalization()(x)
    x = SE_Block(x)

    x = ReLU()(x)
    return x


def SEBottleneck(inputs):
    x = Conv2D()(inputs)
    x = BatchNormalization()(x)
    x = ReLU()(x)

    x = Conv2D()(inputs)
    x = BatchNormalization()(x)
    x = ReLU()(x)

    x = Conv2D()(x)
    x = BatchNormalization()(x)
    x = SE_Block(x)

    x = ReLU()(x)
    return x


def GlobalPool_Classifier(inputs, class_num):
    x = GlobalAveragePooling2D()(inputs)
    x = Dropout(0.5)(x)
    x = Dense()(x)
    x = LeakyReLU()(x)
    outputs = Dense(class_num)(x)
    return outputs


def build_model():
    inputs = Input(shape=(120, 80, 1))
    t0 = slice(inputs, 0, 60, 0, 80)
    t1 = slice(inputs, 60, 120, 0, 80)
    # t0 = Lambda(slice, arguments={'h1': 0, 'h2': 60, 'w1': 0, 'w2': 80})(inputs)
    # t1 = Lambda(slice, arguments={'h1': 60, 'h2': 120, 'w1': 0, 'w2': 80})(inputs)

    x = stack((t0, t1), axis=1)

    x = Block1(x)
    x = Block1(x)
    x = Block1(x)
    x = Block1(x)

    x = Block2(x)
    x = Block2(x)
    x = Block2(x)
    x = Block2(x)

    outputs = GlobalPool_Classifier(x)
    model = Model(inputs=inputs, outputs=outputs)
    return model


if __name__ == '__main__':
    model = build_model()
    model.summary()
