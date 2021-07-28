# fall_detect_camera.py
import keras
import tensorflow as tf
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
from keras.utils.vis_utils import plot_model


BATCH_NORM_DECAY = 0.9
BATCH_NORM_EPSILON = 1e-5

def slice(x, h1, h2, w1, w2):
    return x[:, h1:h2, w1:w2, :]


def ReLU6(inputs):
    x = tf.nn.relu6(inputs)
    return x


def hard_swish(x):
    x = x * (ReLU6(x + 3.) / 6.)
    return x


def SE_Block(inputs, ratio=16):
    channels = int_shape(inputs)[-1]
    se_shape = (1, 1, channels)
    x = GlobalAveragePooling2D()(inputs)
    x = Reshape(se_shape)(x)
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


def SEBasicBlock(inputs, out_channel):
    x = Conv2D(out_channel, 3, padding="same")(inputs)
    x = BatchNormalization()(x)
    x = ReLU()(x)

    x = Conv2D(out_channel, 3, padding="same")(x)
    x = BatchNormalization()(x)
    x = SE_Block(x)

    x = ReLU()(x)
    return x


def SEBottleneck(inputs, out_channel):
    x = Conv2D(out_channel, 1, padding="same")(inputs)
    x = BatchNormalization()(x)
    x = ReLU()(x)

    x = Conv2D(out_channel, 1, padding="same")(inputs)
    x = BatchNormalization()(x)
    x = ReLU()(x)

    x = Conv2D(out_channel, 1, padding="same")(x)
    x = BatchNormalization()(x)
    x = SE_Block(x)

    x = ReLU()(x)
    return x


def Block1(inputs, out_channel):
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
    x = SEBasicBlock(x, out_channel)
    return x


def Block2(inputs, out_channel):
    x = Conv2D(
        32,
        kernel_size=3,
        strides=1,
        padding="same",
        data_format="channels_last",
    )(inputs)
    x = BatchNormalization()(x)
    x = ReLU()(x)

    x = Conv2D(16, 1, padding="same")(x)
    x = BatchNormalization()(x)
    x = ReLU6(x)

    x = DepthwiseConv2D(3, 1, padding="same")(x)
    x = BatchNormalization()(x)
    x = ReLU6(x)

    # x = MaxPooling2D()(x)
    x = SEBottleneck(x, out_channel)
    return x


def GlobalPool_Classifier(inputs, class_num):
    x = GlobalAveragePooling2D()(inputs)
    x = Dropout(0.5)(x)
    x = Dense(32)(x)
    x = LeakyReLU()(x)
    out = Dense(class_num, activation="sigmoid")(x)
    return out


def FDC(inputs, out_channels):
    repeat = 2
    block1_out_channels = 32
    block2_out_channels = 128

    # inputs = Input(shape=(120, 80, 1))
    t0 = slice(inputs, 0, 60, 0, 80)
    t1 = slice(inputs, 60, 120, 0, 80)
    # # t0 = Lambda(slice, arguments={'h1': 0, 'h2': 60, 'w1': 0, 'w2': 80})(inputs)
    # # t1 = Lambda(slice, arguments={'h1': 60, 'h2': 120, 'w1': 0, 'w2': 80})(inputs)

    # x = stack(, axis=1)
    x = concatenate((t0, t1))

    for i in range(repeat):
        x = Block1(x, block1_out_channels)
    for i in range(repeat):
        x = Block2(x, block2_out_channels)

    outputs = GlobalPool_Classifier(x, out_channels)
    model = Model(inputs=inputs, outputs=outputs)
    return model


if __name__ == '__main__':
    inputs = Input(shape=(120, 80, 1), batch_size=64)
    model = FDC(inputs, out_channels=1)
    model.summary()
    plot_model(model, to_file='FDC.png',
               show_layer_names=True, show_shapes=True)