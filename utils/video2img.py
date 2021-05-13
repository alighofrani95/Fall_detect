import cv2
import numpy as np
import random


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

    print(train_x, test_x)


def get_frame_from_video(video_path, frame_num):
    cap = cv2.VideoCapture(video_path)
    cap.set(cv2.CAP_PROP_POS_FRAMES, frame_num)
    a, frame = cap.read()
    cv2.imshow("preview", frame)
    cv2.waitKey(0)


if __name__ == '__main__':
    get_frame_list(2, 8, 0, 20)
    get_frame_from_video("F:/fall_detect/fall_video/video/video_006_fall_0.avi", 25)
