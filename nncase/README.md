# .h5 to .kmodel

## .h5 -> .tflite

Check input output path first.

```shell
python nncase/h5_to_tflite.py
```

## .tflite -> .kmodel

### compile

```shell
nncase/ncc.exe compile  'save_model/tflite/fd_cam_shuffle.tflite'  'save_model/kmodel/fd_cam_shuffle.kmodel' -i tflite -o kmodel -t k210 --dataset 'data/test' --inference-type uint8
```

### infer

```shell
nncase/ncc.exe infer 'save_model/kmodel/fd_cam_shuffle.kmodel' 'nncase/out' --dataset 'nncase/val'
```
