import numpy as np
from keras.preprocessing import image
from keras.preprocessing.image import ImageDataGenerator
from keras import backend as K
from matplotlib.pyplot as plt

from models.camera import TVN





if __name__ == '__main__':
    inputs = Input(shape=(320, 240, 3))
    model = TVN(inputs, 100)
    model.summary()
    plot_model(model, to_file='TVN.png',
               show_layer_names=True, show_shapes=True)
