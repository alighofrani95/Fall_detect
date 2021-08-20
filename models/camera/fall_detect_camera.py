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


def bn_relu(inputs, relu="relu", init_zero=False):
    x = BatchNormalization()(inputs)
    if relu == "relu":
        x = ReLU()(x)
    elif relu == "relu6":
        x = Relu6(x)
    return x


def Conv_DWConv_Conv(inputs, out_channels, stride=1):
    x = Conv2D(out_channels, kernel_size=1, padding="same")(inputs)
    x = bn_relu(x, relu="relu")
    x = DepthwiseConv2D(kernel_size=3, strides=stride, padding="same")(x)
    x = bn_relu(x, relu=None)
    x = Conv2D(out_channels, kernel_size=1, padding="same")(x)
    x = bn_relu(x, relu="relu")
    return x


def channel_split(x):
    in_channles = x.shape.as_list()[-1]
    ip = in_channles // 2
    c_hat = Lambda(lambda z: z[:, :, :, 0:ip])(x)
    c = Lambda(lambda z: z[:, :, :, ip:])(x)
    return c_hat, c


def channel_shuffle(x, groups=2):
    n, h, w, c = x.get_shape().as_list()
    x = tf.reshape(x, [-1, h, w, groups, c // groups])
    x = tf.transpose(x, [0, 1, 2, 4, 3])
    x = tf.reshape(x, [-1, h, w, c])
    return x


def shuffle_block(inputs, out_channels):
    residual, x = channel_split(inputs)
    x = Conv_DWConv_Conv(x, out_channels, stride=1)
    x = Concatenate()([residual, x])
    x = channel_shuffle(x)
    return x


def bottleneck(inputs, out_channels):
    x = Conv_DWConv_Conv(inputs, out_channels)
    return x


def Block1(inputs, out_channel):
    x = Conv2D(out_channel, kernel_size=3, strides=2, padding="same")(inputs)
    x = Conv1D(out_channel, kernel_size=3, strides=1, padding="same")(x)
    # x = spatial_attention(x)
    x = shuffle_block(x, out_channel)
    return x


def Block2(inputs, out_channel):
    x = Conv2D(out_channel, kernel_size=3, strides=1, padding="same")(inputs)
    x = bn_relu(x)
    x = bottleneck(x, out_channel)
    x = MaxPooling2D()(x)
    x = bottleneck(x, out_channel)
    x = shuffle_block(x, out_channel)
    return x


def GlobalPool_Classifier(inputs, class_num):
    x = GlobalAveragePooling2D()(inputs)
    x = Dropout(0.5)(x)
    x = Dense(64, activation="relu")(x)
    out = Dense(class_num, activation="sigmoid")(x)
    return out


def FDC(out_channels):
    repeat = 2
    block1_out_channels = 32
    block2_out_channels = 128

    inputs = Input(shape=(120, 80, 1))
    t0 = slice(inputs, 0, 60, 0, 80)
    t1 = slice(inputs, 60, 120, 0, 80)

    x = Concatenate()([t0, t1])

    for i in range(repeat):
        x = Block1(x, block1_out_channels)
    for i in range(repeat):
        x = Block2(x, block2_out_channels)

    outputs = GlobalPool_Classifier(x, out_channels)
    model = Model(inputs=inputs, outputs=outputs)
    return model


if __name__ == '__main__':
    inputs = Input(shape=(120, 80, 1), batch_size=64)
    model = FDC(out_channels=1)
    model.summary()
    model.save('test.h5')
    # plot_model(model, to_file='FDC.png',
    #            show_layer_names=True, show_shapes=True)
