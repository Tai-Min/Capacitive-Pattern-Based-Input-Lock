import pathlib
import re

import numpy as np
import tensorflow as tf
from tensorflow.keras import activations, layers

__INPUT_TYPE = 0
__DENSE_TYPE = 1
__OUTPUT_TYPE = 255

__ACTIVATION_RELU = 0
__ACTIVATION_SOFTMAX = 1


def __get_dense_activation(activation):
    if activation == activations.relu:
        return __ACTIVATION_RELU
    elif activation == activations.softmax:
        return __ACTIVATION_SOFTMAX
    else:
        raise "Not supported activation type %d" % activation


def __get_matching_tensor_index(regex, interpreter):
    quantized_tensor_details = interpreter.get_tensor_details()

    idx = -1
    for i in range(len(quantized_tensor_details)):
        if re.search(regex, quantized_tensor_details[i]["name"]) and not ";" in quantized_tensor_details[i]["name"]:
            idx = i
            break

    if idx < 0:
        raise "Tensor not found %s" % regex

    return idx


def __get_dense_zero(layer_name, interpreter):
    op = layer_name + "\/MatMul"
    quantized_tensor_details = interpreter.get_tensor_details()

    for i in range(len(quantized_tensor_details)):
        if re.search(op, quantized_tensor_details[i]["name"]) and ";" in quantized_tensor_details[i]["name"]:
            return quantized_tensor_details[i]["quantization_parameters"]["zero_points"][0]

    raise "Weights not found %s" % layer_name


def __get_dense_weights(layer_name, interpreter):
    op = layer_name + "\/MatMul"
    idx = __get_matching_tensor_index(op, interpreter)
    weights = interpreter.get_tensor(idx)
    return weights


def __get_dense_biases(layer_name, interpreter, quantize):
    op = layer_name + "\/BiasAdd"

    idx = __get_matching_tensor_index(op, interpreter)

    tensor = interpreter.get_tensor(idx)
    num_biases = tensor.shape[0]
    biases = np.reshape(tensor, (num_biases, )).tolist()

    if quantize:
        res = []
        for bias in biases:
            # big endian
            res.append((bias >> 24) & 0xFF)
            res.append((bias >> 16) & 0xFF)
            res.append((bias >> 8) & 0xFF)
            res.append(bias & 0xFF)
        return res

    return biases


def __get_dense_byte_model(layer_name, interpreter, quantize):
    all_weights = __get_dense_weights(layer_name, interpreter)
    all_biases = __get_dense_biases(layer_name, interpreter, quantize)

    weights_biases = []

    if quantize:
        weights_biases.append(__get_dense_zero(layer_name, interpreter))

    num_neurons = all_weights.shape[0]
    for neuron in range(num_neurons):
        weights_biases += all_weights[neuron].tolist()
        weights_biases += all_biases[(neuron):(neuron + 1)]

    return weights_biases


def __get_dense_chunk(layer, interpreter, quantize):
    ssie_op_type = __DENSE_TYPE
    ssie_activation = __get_dense_activation(layer.activation)
    ssie_num_neurons = layer.units

    ssie_model = []
    ssie_model += __get_dense_byte_model(layer.name, interpreter, quantize)

    return ssie_op_type, ssie_activation, ssie_num_neurons, ssie_model


def generate_ssie_model(model, num_inputs, num_classes, quantize=False, data_gen=None):
    converter_t8 = tf.lite.TFLiteConverter.from_keras_model(model)

    if quantize:
        converter_t8.optimizations = [tf.lite.Optimize.DEFAULT]
        converter_t8.representative_dataset = data_gen
        converter_t8.target_spec.supported_ops = [
            tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
        converter_t8.inference_input_type = tf.uint8
        converter_t8.inference_output_type = tf.uint8

    quantized_model = converter_t8.convert()

    tflite_models_dir = pathlib.Path("./model")
    tflite_models_dir.mkdir(exist_ok=True, parents=True)

    tflite_model_quant_file = tflite_models_dir/"model.tflite"
    tflite_model_quant_file.write_bytes(quantized_model)

    interpreter = tf.lite.Interpreter(model_content=quantized_model)
    interpreter.allocate_tensors()

    ssie_ops = [__INPUT_TYPE]
    ssie_activations = []
    ssie_num_neurons = []
    ssie_byte_model = []
    ssie_max_tmp_buf_size = max([num_inputs, num_classes])

    for layer in model.layers:
        if type(layer) == layers.Dense:
            ssie_max_tmp_buf_size = max([ssie_max_tmp_buf_size, layer.units])

            op_type, activation, num_neurons, byte_model = __get_dense_chunk(
                layer, interpreter, quantize)
            ssie_ops.append(op_type)
            ssie_activations.append(activation)
            ssie_num_neurons.append(num_neurons)
            ssie_byte_model += byte_model
        else:
            raise "Not supported layer type %s" % type(layer)

    ssie_ops.append(__OUTPUT_TYPE)

    return ssie_ops, ssie_activations, ssie_num_neurons, ssie_byte_model, ssie_max_tmp_buf_size


def toEnumOperation(op):
    if op == __INPUT_TYPE:
        return "Operation::INPUT"
    elif op == __DENSE_TYPE:
        return "Operation::DENSE"
    elif op == __OUTPUT_TYPE:
        return "Operation::OUTPUT"
    raise "Unknown enum element" % type(op)


def toEnumActivation(act):
    if act == __ACTIVATION_RELU:
        return "Activation::RELU"
    elif act == __ACTIVATION_SOFTMAX:
        return "Activation::SOFTMAX"
    raise "Unknown enum element" % type(act)
