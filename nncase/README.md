# .h5 to .kmodel

## .h5 -> .tflite

Check input output path first.

```shell
python nncase/h5_to_tflite.py
```

## .tflite -> .kmodel

### compile

```shell
nncase/ncc.exe compile  'C:/Projects/Fall_detect/save_model/tflite/fd_cam_shuffle_9911.tflite'  'C:/Projects/Fall_detect/save_model/kmodel/fd_cam_shuffle_9911.kmodel' -i tflite -o kmodel -t k210 --dataset 'C:/Projects/Fall_detect/data/train' --inference-type uint8
```

### infer

```shell
./ncc.exe infer 'models/mask_detect.kmodel' 'mask_alive_e.bin' --dataset 'datasets/mask/test'
```
