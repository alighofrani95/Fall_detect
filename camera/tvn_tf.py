import keras
import tensorflow as tf
from keras.layers import *
from keras.models import *
from keras.backend import *
from keras.activations import *

import numpy as np
import tensorflow.compat.v1 as tf
from tiny_video_nets import batch_norm as bn
from tensorflow.contrib import framework as contrib_framework
from tensorflow.contrib import layers as contrib_layers


def hard_swish(x):
    with tf.name_scope('hard_swish'):
        return x * tf.nn.relu6(x + np.float32(3)) * np.float32(1. / 6.)


def reshape_temporal_conv1d_bn(inputs,
                               is_training,
                               filters,
                               kernel_size,
                               num_frames=32,
                               stride=1,
                               use_relu=True,
                               data_format='channels_last'):

    assert data_format == 'channels_last'

    feature_shape = inputs.shape
    batch_size = feature_shape[0] // num_frames

    inputs = tf.reshape(inputs, [
        int(feature_shape[0] // num_frames), num_frames,
        feature_shape[1] * feature_shape[2], -1
    ])

    inputs = tf.layers.conv2d(
        inputs=inputs,
        filters=filters,
        kernel_size=(kernel_size, 1),
        strides=[stride, 1],
        padding='SAME',
        use_bias=False,
        kernel_initializer=contrib_layers.variance_scaling_initializer(
            factor=2.0, mode='FAN_IN', uniform=False),
        data_format=data_format)

    new_frames = inputs.shape[1]
    inputs = tf.reshape(
        inputs, [batch_size * new_frames, feature_shape[1], feature_shape[2], -1])

    inputs = bn.batch_norm_relu(
        inputs,
        is_training,
        relu=use_relu,
        data_format=data_format)

    return inputs, new_frames


def conv2d(inputs,
           kernel,
           filters,
           stride,
           is_training,
           use_relu=True,
           init_zero=False,
           use_bn=True,
           data_format='channels_last'):

    inputs = tf.layers.conv2d(
        inputs=inputs,
        filters=filters,
        kernel_size=kernel,
        strides=stride,
        padding='SAME',
        use_bias=False,
        kernel_initializer=contrib_layers.variance_scaling_initializer(
            factor=2.0, mode='FAN_IN', uniform=False),
        data_format=data_format)
    if use_bn:
        inputs = bn.batch_norm_relu(
            inputs,
            is_training,
            relu=use_relu,
            init_zero=init_zero,
            data_format=data_format)
    if not use_bn and use_relu:
        inputs = tf.nn.relu(inputs)
    return inputs


def spatial_conv(inputs,
                 conv_type,
                 kernel,
                 filters,
                 stride,
                 is_training,
                 activation_fn='relu',
                 data_format='channels_last'):

    if kernel == 1:
        return inputs
    use_relu = (activation_fn == 'relu')
    if conv_type == 'std' or conv_type == 'depth':
        inputs = conv2d(inputs, 1, filters, 1, is_training, use_relu=use_relu)
        if not use_relu:
            inputs = hard_swish(inputs)

    if conv_type == 'std' or conv_type == '1std':
        inputs = conv2d(inputs, int(kernel), filters, int(stride), is_training,
                        use_relu=use_relu)
        if not use_relu:
            inputs = hard_swish(inputs)
    elif conv_type == 'depth':
        depth_multiplier = 1
        depthwise_kernel_shape = (int(kernel), int(kernel), inputs.shape[-1],
                                  depth_multiplier)

        depthwise_kernel = contrib_framework.model_variable(
            name='depthwise_kernel',
            shape=depthwise_kernel_shape,
            dtype=tf.float32,
            initializer=contrib_layers.variance_scaling_initializer(
                factor=2.0, mode='FAN_IN', uniform=False),
            trainable=True)

        inputs = tf.nn.depthwise_conv2d(
            inputs,
            tf.cast(depthwise_kernel, inputs.dtype),
            strides=[1, int(stride), int(stride), 1],
            padding='SAME',
            rate=[1, 1],
            data_format='NHWC' if data_format == 'channels_last' else 'NCHW')

        inputs = bn.batch_norm_relu(
            inputs,
            is_training,
            relu=use_relu,
            data_format=data_format)
        if not use_relu:
            inputs = hard_swish(inputs)

    elif conv_type == 'maxpool':
        inputs = tf.layers.max_pooling2d(
            inputs,
            int(kernel),
            int(stride),
            padding='same',
            data_format=data_format)

    elif conv_type == 'avgpool':
        inputs = tf.layers.average_pooling2d(
            inputs,
            int(kernel),
            int(stride),
            padding='same',
            data_format=data_format)

    return inputs


def temporal_conv(inputs,
                  conv_type,
                  kernel,
                  filters,
                  stride,
                  is_training,
                  num_frames,
                  activation_fn='relu',
                  data_format='channels_last'):

    use_relu = (activation_fn == 'relu')
    new_frames = num_frames
    if kernel == 1:
        return inputs, new_frames
    if conv_type == '1d':
        inputs, new_frames = reshape_temporal_conv1d_bn(
            inputs=inputs,
            is_training=is_training,
            filters=filters,
            kernel_size=kernel,
            stride=stride,
            num_frames=num_frames,
            use_relu=use_relu,
            data_format=data_format)
        if not use_relu:
            inputs = hard_swish(inputs)

    elif conv_type == 'maxpool' or conv_type == 'avgpool':
        feature_shape = inputs.shape
        batch_size = feature_shape[0] // num_frames

        inputs = tf.reshape(
            inputs,
            [batch_size, num_frames, feature_shape[1] * feature_shape[2], -1])

        if conv_type == 'maxpool':
            inputs = tf.layers.max_pooling2d(
                inputs, [kernel, 1], [stride, 1],
                padding='same',
                data_format=data_format)
        elif conv_type == 'avgpool':
            inputs = tf.layers.average_pooling2d(
                inputs, [kernel, 1], [stride, 1],
                padding='same',
                data_format=data_format)

        new_frames = inputs.shape[1]
        inputs = tf.reshape(
            inputs,
            [batch_size * new_frames, feature_shape[1], feature_shape[2], -1])
    return inputs, new_frames


def squeeze_and_excite(inputs, filters, ratio, num_frames):

    reduced_filters = max(int(filters * ratio), 8)
    # Shape is [batch*num_frames, width, height, channels].
    feature_shape = [inputs.shape[0], 1, 1, inputs.shape[3]]
    # Reshape to [batch, num_frames*width, height, channels]
    # for memory efficient spatio-temporal squeeze-exicte layer.
    squeeze_excite_inputs = tf.reshape(inputs, [
        int(tf.compat.dimension_value(inputs.shape[0]) // num_frames),
        num_frames*inputs.shape[1],
        inputs.shape[2], -1
    ])

    # Spatio-temporal averaging.
    squeeze_excite = tf.reduce_mean(
        squeeze_excite_inputs, [1, 2], keepdims=True)
    squeeze_excite = tf.layers.conv2d(
        inputs=squeeze_excite,
        filters=reduced_filters,
        kernel_size=1,
        strides=1,
        padding='SAME',
        use_bias=True,
        kernel_initializer=contrib_layers.variance_scaling_initializer(
            factor=2.0, mode='FAN_IN', uniform=False),
        data_format='channels_last')
    squeeze_excite = tf.layers.conv2d(
        inputs=tf.nn.relu(squeeze_excite),
        filters=filters,
        kernel_size=1,
        strides=1,
        padding='SAME',
        use_bias=True,
        kernel_initializer=contrib_layers.variance_scaling_initializer(
            factor=2.0, mode='FAN_IN', uniform=False),
        data_format='channels_last')
    squeeze_excite = tf.expand_dims(tf.nn.sigmoid(squeeze_excite), 1)
    # Add in number of frames.
    pattern = tf.stack([1, num_frames, 1, 1, 1])
    # Reshape to full spatio-temporal video size.
    return tf.reshape(tf.tile(squeeze_excite, pattern), feature_shape) * inputs


def context_gate(inputs, filters, num_frames):
    # Shape is [batch*num_frames, width, height, channels].
    feature_shape = [inputs.shape[0], 1, 1, inputs.shape[3]]

    # Reshape to [batch, num_frames*width, height, channels]
    # for memory efficient spatio-temporal squeeze-exicte layer.
    context_inputs = tf.reshape(inputs, [
        int(tf.compat.dimension_value(inputs.shape[0]) // num_frames),
        num_frames*inputs.shape[1],
        inputs.shape[2], -1
    ])

    feature = tf.reduce_mean(context_inputs, [1, 2], keepdims=True)
    feature = tf.layers.conv2d(
        inputs=feature,
        filters=filters,
        kernel_size=1,
        strides=1,
        padding='SAME',
        use_bias=True,
        kernel_initializer=contrib_layers.variance_scaling_initializer(
            factor=2.0, mode='FAN_IN', uniform=False),
        data_format='channels_last')
    feature = tf.expand_dims(tf.nn.sigmoid(feature), 1)
    pattern = tf.stack([1, num_frames, 1, 1, 1])
    return tf.reshape(tf.tile(feature, pattern), feature_shape) * inputs

