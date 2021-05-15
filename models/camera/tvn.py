import keras
import tensorflow as tf
from keras.layers import *
from keras.models import *
from keras import backend as K
from keras.activations import *
from keras.utils import plot_model
from keras.engine.topology import Layer


BN_DECAY = 0.9
BN_EPSILON = 1e-5


# def hard_swish(x):
#     x = x * tf.nn.relu6(x + 3.) * 0.16666667
#     return x


class HardSwish(Layer):
    def __init__(self):
        super(HardSwish, self).__init__()

    def call(self, inputs):
        return inputs * tf.nn.relu6(inputs + 3.) * 0.16666667

    def compute_output_shape(self, input_shape):
        return input_shape


def se_block(inputs, ratio=2):
    channels = K.int_shape(inputs)[-1]
    x = GlobalAveragePooling2D()(inputs)
    x = Dense(channels // ratio, activation='relu')(x)
    x = Dense(channels, activation='sigmoid')(x)
    x = Multiply()([inputs, x])
    return x


def bn_relu(inputs, relu="relu", init_zero=False):
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
    if relu == "relu":
        x = ReLU()(x)
    elif relu == "h-swish":
        x = HardSwish()(x)
    else:
        pass
    return x


def block1(inputs):
    x = Conv2D(32, kernel_size=3, strides=2, padding="same")(inputs)
    x = bn_relu(x, relu="relu")
    x = Conv1D(32, kernel_size=3, strides=1, padding="same")(x)
    x = bn_relu(x, relu="relu")
    x = se_basic_block(x, 32)
    return x


def block2(inputs):
    # inv-bottle, 128x1x1
    x = Conv2D(128, kernel_size=1, strides=1, padding="same")(inputs)
    x = bn_relu(x, relu="h-swish")
    x = MaxPooling2D((1, 1))(x)
    x = bn_relu(x, relu="h-swish")
    x = se_basic_block(x, 128)
    return x


def se_basic_block(inputs, outputs=128):
    x = Conv2D(outputs, (3, 3), strides=1, padding="same")(inputs)
    x = bn_relu(x)
    x = Conv2D(outputs, (3, 3), strides=1, padding="same")(x)
    x = bn_relu(x, relu=None)
    x = se_block(x)

    x = Concatenate(axis=-1)([x, inputs])
    x = ReLU()(x)
    return x


def GP_class(inputs, classes=100):
    x = GlobalAvgPool2D()(inputs)
    x = Dropout(0.5)(x)
    x = Dense(128, activation="relu")(x)
    x = Dense(classes, activation="softmax")(x)
    return x


def TVN(inputs, out_chanels, vstack=True):
    repeat = 2
    block1_repeat = repeat
    block2_repeat = repeat

    x = inputs

    if vstack:
        _, h, w, c = inputs.shape.as_list()
        print("[INPUTS] shape: {}".format(_, h, w, c))
        x = tf.reshape(inputs, [2, h//2, w, c])

    for i in range(block1_repeat):
        x = block1(x)
    for j in range(block2_repeat):
        x = block2(x)

    x = GP_class(x, out_chanels)

    model = Model(inputs=inputs, outputs=x)
    return model


if __name__ == '__main__':
    inputs = Input(shape=(480, 320, 3))
    model = TVN(inputs, 10)
    model.summary()
    plot_model(model, to_file='TVN.png',
               show_layer_names=True, show_shapes=True)
