import keras
import tensorflow as tf
from keras.layers import *
from keras.models import *
from keras.backend import *
from keras.activations import *


BN_DECAY = 0.9
BN_EPSILON = 1e-5

def se_block(inputs, ratio):
    channels = K.int_shape(inputs)[-1]
    x = GlobalAveragePooling2D()(inputs)
    x = Dense(channels // ratio, activation='relu')(x)
    x = Dense(channels, activation='sigmoid')(x)
    x = Multiply()([inputs, x])
    return x


def bn_relu(inputs, relu=True, init_zero=False):
    initializer = tf.zeros_initializer() if init_zero else tf.ones_initializer()
    x = BatchNormalization(
        axis=-1,
        momentum=BN_DECAY,
        epsilon=BN_EPSILON,
        center=True,
        scale=True,
        fused=True,
        gamma_initializer=initializer,
    )(x)
    if relu:
        x = ReLU()(x)
    return x


def block1(inputs, outputs):
    x = Conv2D()(x)


def tvn(flag="TVN-M-1"):



