import numpy as np
from PIL import Image


data = np.loadtxt("utils/data.txt")
im = Image.fromarray(data.reshape((120, 80)))
im.show()


# if __name__ == '__main__':

