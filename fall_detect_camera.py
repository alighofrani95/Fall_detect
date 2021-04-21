# fall_detect_camera.py
import keras
from keras.layers import *
from keras.models import *
from keras.losses import *
from keras.metrics import *
from keras.callbacks import *
from keras.optimizers import *


def slice(x, h1, h2, w1, w2):
    """ Define a tensor slice function
    """
    return x[:, h1:h2, w1:w2, :]


def block_1(inputs):
    x = Conv2D(32, (3, 3))(inputs)


def block_2(inputs):
    x = Conv2D(32, (3, 3))(inputs)


def GP_Classifier(inputs):
    x = GlobalAvgPool2D(padding='same')(inputs)


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
