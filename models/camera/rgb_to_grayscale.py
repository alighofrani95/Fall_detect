import os
import random
from glob import glob

from PIL import Image


def rgb_to_grayscale(img_path, save_path):
    im = Image.open(img_path).convert('L').resize((80, 120))
    filename = os.path.basename(img_path)
    # im.save(os.path.join(save_path, filename))
    im.save(img_path)


if __name__ == "__main__":
    img_dir = "data"
    save_path = 'data/fall'
    list_img = glob(os.path.join(img_dir, "val/fall/*.jpg"))
    
    sub_list = random.sample(list_img, 800)
    for i in sub_list:
        print(i)
        rgb_to_grayscale(i, save_path)
