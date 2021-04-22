# fall_detect_camera.py
import keras
import numpy as np
from keras.layers import *
from keras.models import *
from keras.losses import *
from keras.metrics import *
from keras.callbacks import *
from keras.optimizers import *
from keras.activations import *
from keras.initializers import *


def slice(x, h1, h2, w1, w2):
    """ Define a tensor slice function
    """
    return x[:, h1:h2, w1:w2, :]


def relu6():
    x = ReLU(6.)(x)
    return x


def hard_swish(x):
    x = x * (relu6(x + 3.) / 6.)
    return x


def Block1(inputs):
    x = Conv2D()(inputs)
    x = Conv1D()(x)


def Block2(inputs):
    x = Conv2D()(inputs)
    x = MaxPooling1D()(x)


def SELayer(inputs):
    x = GlobalAveragePooling2D()(inputs)
    x = linear()(x)
    x = relu()(x)
    x = linear()(x)
    x = sigmoid()(x)
    return x


def SEBasicBlock(inputs):
    x = Conv2D()(inputs)
    x = BatchNormalization()(x)
    x = relu()(x)

    x = Conv2D()(x)
    x = BatchNormalization()(x)
    x = SELayer(x)

    x = relu()(x)
    return x


def SEBottleneck(inputs):
    x = Conv2D()(inputs)
    x = BatchNormalization()(x)
    x = relu()(x)

    x = Conv2D()(inputs)
    x = BatchNormalization()(x)
    x = relu()(x)

    x = Conv2D()(x)
    x = BatchNormalization()(x)
    x = SELayer(x)

    x = relu()(x)
    return x


def GlobalPool(inputs):
    x = GlobalAveragePooling2D()(inputs)


def build_model():
    inputs = Input((2, 80, 120, 1))
    t0 = slice(inputs, 0, 60, 0, 80)
    t1 = slice(inputs, 60, 120, 0, 80)
    # t0 = Lambda(slice, arguments={'h1': 0, 'h2': 60, 'w1': 0, 'w2': 80})(inputs)
    # t1 = Lambda(slice, arguments={'h1': 60, 'h2': 120, 'w1': 0, 'w2': 80})(inputs)

    outputs = GP_Classifier(inputs)
    model = Model(inputs=inputs, outputs=outputs)
    return model


model = build_model()
model.summary()
