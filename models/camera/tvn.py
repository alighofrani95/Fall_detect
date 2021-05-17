import keras
import tensorflow as tf
from keras.layers import *
from keras.models import *
from keras import backend as K
from keras.activations import *
# from keras.utils.vis_utils import plot_model
from keras.engine.topology import Layer


batch_size = 32
num_frames = 2

SE_RATIO = 2
BN_DECAY = 0.9
BN_EPSILON = 1e-5


def slice(x, h1, h2, w1, w2):
    """ Define a tensor slice function
    """
    return x[:, h1:h2, w1:w2, :]


class HardSwish(Layer):
    def __init__(self):
        super(HardSwish, self).__init__()

    def call(self, inputs):
        return inputs * tf.nn.relu6(inputs + 3.) * 0.16666667

    def compute_output_shape(self, input_shape):
        return input_shape


def se_block(inputs, ratio=SE_RATIO):
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


def GAP_classification(inputs, classes=100):
    feature_shape = inputs.shape.as_list()
    num_frames = feature_shape[0] // batch_size
    inputs = tf.reshape(inputs, [
        int(feature_shape[0] // num_frames),
        num_frames * feature_shape[1], feature_shape[2], -1
    ])
    x = tf.reduce_mean(inputs, axis=[1, 2])
    x = Dropout(0.5)(x)
    x = Dense(classes, activation="softmax", name='out')(x)
    print("[OUTPUT] shape:", x.shape.as_list())
    return x


def TVN(inputs, out_chanels, vstack=True):
    global batch_size

    repeat = 2
    block1_repeat = repeat
    block2_repeat = repeat

    bs, h, w, c = inputs.shape.as_list()
    batch_size = bs
    num_frames = h//240

    print(inputs.shape.as_list())
    print("batch_size", batch_size)

    x = inputs

    if vstack:
        x = tf.reshape(x, [bs*num_frames, h//num_frames, w, c])
        # f1 = Lambda(slice, arguments={
        #     'h1': 0, 'h2': 240, 'w1': 0, 'w2': 320})(x)
        # f2 = Lambda(slice, arguments={
        #     'h1': 240, 'h2': 480, 'w1': 0, 'w2': 320})(x)
        # x = Add()([f1, f2])

    for i in range(block1_repeat):
        x = block1(x)
    for j in range(block2_repeat):
        x = block2(x)

    out = GAP_classification(x, out_chanels)

    model = Model(inputs=inputs, outputs=out)
    return model


if __name__ == '__main__':
    # for testing
    inputs = Input(shape=(num_frames*240, 320, 1), batch_size=batch_size)
    model = TVN(inputs, 100)
    model.summary()
    # plot_model(model, to_file='TVN.png',
    #            show_layer_names=True, show_shapes=True)
