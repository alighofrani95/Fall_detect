from glob import glob
import random
import os
from shutil import copyfile
from pathlib import Path


def path_insert(file_path, root_path, insert_path) -> str:
    file_path = file_path.replace(root_path, "{}\{}".format(root_path, insert_path))
    return file_path


def split_dataset(root_path, ratio=0.8):
    dirs_list = glob("{}/*".format(root_path))

    Path(os.path.join(root_path, "train")).mkdir(parents=True, exist_ok=True)
    Path(os.path.join(root_path, "test")).mkdir(parents=True, exist_ok=True)
    
    for dirs in dirs_list:
        print(dirs)
        files_list = glob("{}/*.jpg".format(dirs))
        random.shuffle(files_list)
        files_num = len(files_list)

        split_point = int(files_num*ratio)

        train_list = files_list[:split_point]
        test_list = files_list[split_point:]

        print(files_num, len(train_list), len(test_list))

        for i in train_list:
            mv2path = path_insert(i, "data", "train")
            path_exist = Path(os.path.dirname(mv2path)).mkdir(parents=True, exist_ok=True)
            os.rename(i, mv2path)

        for j in test_list:
            mv2path = path_insert(j, "data", "test")
            path_exist = Path(os.path.dirname(mv2path)).mkdir(parents=True, exist_ok=True)
            os.rename(j, mv2path)


if __name__ == "__main__":
    split_dataset("data")
