import keras
import tensorflow as tf
from keras.layers import *
from keras.models import *
from keras import backend as K
from keras.activations import *
from keras.utils import plot_model


BN_DECAY = 0.9
BN_EPSILON = 1e-5


def hard_swish(x):
    x = x * tf.nn.relu6(x + 3.) * 0.16666667
    return x


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
    else:
        x = hard_swish(x)
    return x


def block1(inputs):
    x = Conv2D(32, kernel_size=3, strides=1, padding="same")(inputs)
    x = bn_relu(x, relu=True)
    x = Conv1D(32, kernel_size=3, strides=1, padding="same")(x)
    x = bn_relu(x, relu=True)
    x = se_basic_block(x)
    return x


def block2(inputs):
    # inv-bottle, 128x1x1
    x = Conv2D(128, kernel_size=1, strides=1, padding="same")(inputs)
    x = bn_relu(x, relu=False)
    x = MaxPooling2D((1, 1))(x)
    x = bn_relu(x, relu=False)
    x = se_basic_block(x)
    return x


def se_basic_block(inputs, outputs=128, stride=1):
    x = Conv2D(outputs, (3, 3), strides=stride, padding="same")(inputs)
    x = bn_relu(x)
    x = Conv2D(outputs, (3, 3), strides=1, padding="same")(x)
    x = bn_relu(x, relu=False)
    x = se_block(x)

    x = Concatenate(axis=-1)([x, inputs])
    x = ReLU()(x)
    return x


def GP_class(inputs, classes=100):
    x = GlobalAvgPool2D()(inputs)
    x = Dropout(0.5)(x)
    x = Dense(128)(x)
    x = LeakyReLU(0.2)(x)
    x = Dense(classes)(x)
    x = Softmax()(x)
    return x


def TVN(inputs, out_chanels):
    repeat = 2
    block1_repeat = repeat
    block2_repeat = repeat

    x = inputs

    for i in range(block1_repeat):
        x = block1(x)
    for j in range(block2_repeat):
        x = block2(x)

    x = GP_class(x, out_chanels)

    model = Model(inputs=inputs, outputs=x)
    return model


if __name__ == '__main__':
    inputs = Input(shape=(320, 240, 3))
    model = TVN(inputs, 100)
    model.summary()
    plot_model(model, to_file='TVN.png',
               show_layer_names=True, show_shapes=True)
