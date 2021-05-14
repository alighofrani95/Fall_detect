import cv2
import numpy as np
import random
import os
from pathlib import Path


img_file_count = 0
save_path = "data"

def get_basename(file_path) -> str:
    file_name = os.path.basename(file_path).split(".")[0]
    return file_name


def get_total_frames_num(path:str) -> int:
    cap = cv2.VideoCapture(path)
    frames_num = cap.get(7)
    return int(frames_num)


def get_video_fps(path:str) -> int:
    cap = cv2.VideoCapture(path)
    fps = cap.get(5)
    return int(fps)


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


def get_frame_from_video(video_path, frame_num):
    cap = cv2.VideoCapture(video_path)
    cap.set(cv2.CAP_PROP_POS_FRAMES, frame_num)
    success, frame = cap.read()
    if not success:
        raise print("wrong frame number: {}".format(frame_num))
    # cv2.imwrite("img_0.jpg", frame)
    # cv2.imshow("preview", frame)
    # cv2.waitKey(0)
    return frame


def merge_imgs(img_list: list):
    img = cv2.vconcat(img_list)
    # cv2.imshow("preview", img)
    # cv2.waitKey(0)
    return img


def save_imgs(save_root:str, action:str, imgs):
    global img_file_count
    Path(os.path.join(save_root, action)).mkdir(parents=True, exist_ok=True)
    img_file_count += 1
    img_path = os.path.join("{}/{}_{}.jpg".format(save_root, action, img_file_count))
    cv2.imwrite(img_path, imgs)


def gen_img_by_frame_num_list(video_path:str, frame_list:list):
    print(video_path)
    for x in frame_list:
        img_stack = list()
        for i in x:
            img_stack.append(get_frame_from_video(video_path, i))
        merged_img = merge_imgs(img_stack)
        file_name = get_basename(video_path)
        action_name = file_name.split('_')[2]
        save_imgs(save_path, action_name, merged_img)


if __name__ == '__main__':
    # file_name = get_basename("F:/fall_detect/fall_video/video/video_006_fall_0.avi")
    # action_name = file_name.split('_')[2]
    # print(action_name)

    video_path = "F:/fall_detect/fall_video/video/video_006_fall_0.avi"
    total_frame_num = get_total_frames_num(video_path)
    fps = get_video_fps(video_path)

    train_x, test_x = get_frame_list(1.5, fps, 0, total_frame_num)

    gen_img_by_frame_num_list(video_path, train_x)


