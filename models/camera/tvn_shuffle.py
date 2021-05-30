import keras
import tensorflow as tf
from keras.layers import *
from keras.models import *
from keras import backend as K
from keras.activations import *
from keras.utils.vis_utils import plot_model


batch_size = 64
num_frames = 2

SE_RATIO = 2
BN_DECAY = 0.9
BN_EPSILON = 1e-5


def channel_split(x, num_splits=2):
    if num_splits == 2:
        return tf.split(x, axis=-1, num_or_size_splits=num_splits)
    else:
        raise ValueError('Error! num_splits should be 2')


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
    elif relu == "relu6":
        x = tf.nn.relu6(x)
    else:
        pass
    return x


def Conv_DWConv_Conv(inputs, out_channels, stride=1):
    x = Conv2D(out_channels, kernel_size=1, padding="same")(inputs)
    x = bn_relu(x, relu="relu")
    x = DepthwiseConv2D(kernel_size=3, strides=stride, padding="same")(x)
    x = bn_relu(x, relu=None)
    x = Conv2D(out_channels, kernel_size=1, padding="same")(x)
    x = bn_relu(x, relu="relu")
    return x


def channel_shuffle(x, groups=2):
    n, h, w, c = x.get_shape().as_list()
    x = tf.reshape(x, [-1, h, w, groups, c // groups])
    x = tf.transpose(x, [0, 1, 2, 4, 3])
    x = tf.reshape(x, [-1, h, w, c])
    return x


def shuffle_block(inputs, out_channels):
    residual, x = channel_split(inputs)
    x = Conv_DWConv_Conv(x, out_channels)
    x = Concatenate()([residual, x])
    x = channel_shuffle(x)
    return x


def shuffle_block_down_sampling(inputs, out_channels):
    x0 = DepthwiseConv2D(kernel_size=3, strides=2, padding="same")(inputs)
    x0 = bn_relu(x0, relu=None)
    x0 = Conv2D(out_channels, kernel_size=1, padding="same")(x0)
    x0 = bn_relu(x0, relu="relu")

    x = Conv_DWConv_Conv(inputs, out_channels, stride=2)

    out = Concatenate()([x0, x])
    out = channel_shuffle(out)
    return out


def block1(inputs):
    x = Conv2D(32, kernel_size=3, strides=2, padding="same")(inputs)
    x = bn_relu(x, relu="relu")
    x = Conv1D(32, kernel_size=3, strides=1, padding="same")(x)
    x = bn_relu(x, relu="relu")
    x = shuffle_block(x, 32)
    return x


def block2(inputs):
    # inv-bottle, 128x1x1
    x = Conv2D(128, kernel_size=1, strides=1, padding="same")(inputs)
    x = bn_relu(x, relu="relu6")
    x = MaxPooling2D((1, 1))(x)
    x = bn_relu(x, relu="relu6")
    x = shuffle_block_down_sampling(x, 128)
    return x


def GAP_classification(inputs, classes=100):
    feature_shape = inputs.shape.as_list()
    num_frames = feature_shape[0] // batch_size
    inputs = tf.reshape(inputs, [
        int(feature_shape[0] // num_frames),
        num_frames * feature_shape[1], feature_shape[2], -1
    ])
    x = tf.reduce_mean(inputs, axis=[1, 2])
    x = Dropout(0.5)(x)
    x = Dense(classes, activation="sigmoid", name='out')(x)
    print("[OUTPUT] shape:", x.shape.as_list())
    return x


def TVN(inputs, out_chanels, vstack=True):
    global batch_size

    repeat = 3
    block1_repeat = repeat
    block2_repeat = repeat

    bs, h, w, c = inputs.shape.as_list()
    batch_size = bs
    # num_frames = h//240

    print(inputs.shape.as_list())
    print("batch_size", batch_size)

    x = inputs

    if vstack:
        x = tf.reshape(x, [bs*num_frames, h//num_frames, w, c])

    for i in range(block1_repeat):
        x = block1(x)
    for j in range(block2_repeat):
        x = block2(x)

    out = GAP_classification(x, out_chanels)

    model = Model(inputs=inputs, outputs=out)
    return model


if __name__ == '__main__':
    # for testing
    num_frames = 2
    inputs = Input(shape=(num_frames*240, 320, 1), batch_size=batch_size)
    model = TVN(inputs, 100)
    model.summary()
    plot_model(model, to_file='TVN_shuffle.png',
               show_layer_names=True, show_shapes=True)
