import cv2
import numpy as np
import random
import os


def get_basename(file_path)->str:
    file_name = os.path.basename(file_path).split(".")[0]
    return file_name


def get_frame_list(num_pick, video_fps, frame_start, frame_end) -> list:
    x_list = list()
    per_frame_skip_frame = round(float(video_fps)/float(num_pick))

    if per_frame_skip_frame > (frame_end - frame_start):
        raise "frames are not enough"

    scope_end = frame_end - per_frame_skip_frame

    for i in range(frame_start, scope_end):
        x_list.append([i, i+per_frame_skip_frame])

    random.shuffle(x_list)
    split_train = int(len(x_list) * 0.8)
    train_x = x_list[:split_train]
    test_x = x_list[split_train:]

    return (train_x, test_x)


def get_frame_from_video(video_path, frame_num)->bool:
    cap = cv2.VideoCapture(video_path)
    cap.set(cv2.CAP_PROP_POS_FRAMES, frame_num)
    success, frame = cap.read()
    cv2.imwrite("img_0.jpg", frame)
    # cv2.imshow("preview", frame)
    # cv2.waitKey(0)
    return success


if __name__ == '__main__':
    train_x, test_x = get_frame_list(2, 8, 0, 20)

    for x in train_x:
        print(x[0], x[1])

    x = get_basename("F:/fall_detect/fall_video/video/video_006_fall_0.avi")
    print(x)
    # get_frame_from_video("F:/fall_detect/fall_video/video/video_006_fall_0.avi", 25)
