import keras
import tensorflow as tf
from keras.layers import *
from keras.models import *
from keras import backend as K
from keras.activations import *
from keras.utils import plot_model


BN_DECAY = 0.9
BN_EPSILON = 1e-5

def se_block(inputs, ratio=16):
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
    )(inputs)
    if relu:
        x = ReLU()(x)
    return x


def block1(inputs, outputs):
    x = Conv2D(outputs, kernel_size=3, strides=1, padding="same")(inputs)
    x = bn_relu(x, relu=True)
    x = Conv1D(outputs, kernel_size=3, strides=2, padding="same")(x)
    x = bn_relu(x, relu=True)
    x = se_basic_block(x, outputs)
    return x


def block2(inputs, outputs):
    x = Conv2D(outputs, kernel_size=3, strides=1, padding="same")(inputs)
    x = bn_relu(x, relu=True)
    x = Conv1D(outputs, kernel_size=3, strides=1, padding="same")(x)
    x = bn_relu(x, relu=True)
    x = se_basic_block(x, outputs)
    return x


def se_basic_block(inputs, outputs, stride=1, downsample=None):
    x = Conv2D(outputs, (3,3), strides=stride, padding="same")(inputs)
    x = bn_relu(x)
    x = Conv2D(outputs, (3,3), strides=1, padding="same")(x)
    x = bn_relu(x, relu=False)
    x = se_block(x)

    if downsample is not None:
        residual = downsample(x)

    x = Concatenate(axis=-1)([x, inputs])
    x = ReLU()(x)
    return x


def se_bottleneck(inputs, outputs, stride=1, downsample=None):
    x = Conv2D(outputs, (1,1), strides=stride)(inputs)
    x = bn_relu(x)
    x = Conv2D(outputs, (3,3), strides=stride)(x)
    x = bn_relu(x)
    x = Conv2D(outputs, (1,1), strides=stride)(x * 4)
    x = bn_relu(x * 4, relu=False)
    x = se_block(x * 4)

    if downsample is not None:
        residual = downsample(x)

    x = Concatenate(axis=-1)([x, inputs])
    x = ReLU()(x)
    return x


def GP_class(inputs, classes=100):
    x = GlobalAvgPool2D()(inputs)
    # x = Dropout(0.5)(x)
    x = ReLU()(x)
    return x



def tvn(inputs, out_chanels):
    # inputs = Input(shape=(2, 160, 120, 3))
    x = inputs
    for i in range(4):
        x = block1(x, out_chanels)
    for j in range(4):
        x = block2(x, out_chanels)

    x = GP_class(x)

    model = Model(inputs=inputs, outputs=x)
    return model


if __name__ == '__main__':
    inputs = Input(shape=(160, 120, 3))
    model = tvn(inputs, 10)
    model.summary()
    plot_model(model, to_file='tvn.png',
               show_layer_names=True, show_shapes=True)
