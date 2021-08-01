import os
from glob import glob
from struct import unpack


def read_bin(file_path):
    # print(file_path)
    features = list()
    binfile = open(file_path, 'rb')
    size = os.path.getsize(file_path)

    for _ in range(size // 4):
        data = binfile.read(4)
        num = unpack('f', data)
        features.append(round(num[0], 4))
    binfile.close()
    print(file_path, "\t", features)
    return features

if __name__ == "__main__":
    for i in glob("nncase/out/*.bin"):
        read_bin(i)
