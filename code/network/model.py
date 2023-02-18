import csv
import os
import random

import numpy as np
import tensorflow as tf
from tensorflow import keras
from tensorflow.keras import layers

from model_exporter import (generate_ssie_model, toEnumActivation,
                            toEnumOperation)

QUANTIZE = False

EPOCHS = 100
BATCH_SIZE = 1
TEST_SPLIT = 0.2
INITIAL_LR = 0.005
DECAY_STEPS = 300
SGD_MOMENTUM = 0.99
AUGMENT_STRENGTH = 0.05

NUM_INPUTS = 50
CLASSES = ["X", "triangle", "heart", "circle",  "square"]
NUM_CLASSES = len(CLASSES)

model = keras.Sequential(
    [
        layers.Dense(10, activation="relu", name="layer1"),
        layers.Dense(20, activation="relu", name="layer2"),
        layers.Dense(NUM_CLASSES, activation="softmax", name="layer3"),
    ]
)

lr = tf.keras.optimizers.schedules.PolynomialDecay(INITIAL_LR, DECAY_STEPS)

optimizer = keras.optimizers.SGD(learning_rate=lr, momentum=SGD_MOMENTUM)


def augment_sample(sample):
    for i in range(len(sample)):

        op = -1.0 if bool(random.getrandbits(1)) else 1.0
        sample[i] = float(sample[i]) + op * float(sample[i]) * AUGMENT_STRENGTH

        if sample[i] < 0.0:
            sample[i] = 0.0
        elif sample[i] > 255.0:
            sample[i] = 255.0

    return sample


def load_sample(sample):
    sample = bytes.decode(sample.numpy())
    sample_label = os.path.basename(sample)
    sample_label = sample_label.split(".")[0]

    label = [0] * NUM_CLASSES
    idx = CLASSES.index(sample_label)
    label[idx] = 1

    label = np.array(label)

    with open(sample, newline='') as csvfile:
        reader = csv.reader(csvfile, delimiter=',')
        next(reader)
        data = next(reader)
        data = augment_sample(data)
        data = np.array(data, dtype="float") / 255.0
        return (data, label)


num_samples = len(os.listdir("./dataset"))
num_train_samples = int((1.0 - TEST_SPLIT) * num_samples)
num_test_samples = int(TEST_SPLIT * num_samples)

train_dataset = tf.data.Dataset.list_files("./dataset/*.csv", shuffle=True)

dataset = train_dataset.map(
    lambda x: tf.py_function(
        load_sample, [x], (tf.float32, tf.float32))
).shuffle(buffer_size=1024)

train_dataset = dataset.take(num_train_samples).batch(
    BATCH_SIZE).cache().prefetch(tf.data.AUTOTUNE).repeat(-1)
test_dataset = dataset.skip(num_train_samples).take(
    num_test_samples).batch(BATCH_SIZE)

model.build((BATCH_SIZE, NUM_INPUTS))
model.compile(optimizer=optimizer,
              loss=tf.keras.losses.CategoricalCrossentropy(),
              metrics=['accuracy'])

model.summary()

model.fit(train_dataset.as_numpy_iterator(),
          epochs=EPOCHS, steps_per_epoch=num_train_samples)

model.evaluate(test_dataset.as_numpy_iterator())

def data_gen(dataset):
    yield [np.array([0.0] * NUM_INPUTS, dtype=np.float32, ndmin=2)]
    yield [np.array([1.0] * NUM_INPUTS, dtype=np.float32, ndmin=2)]

    for i, _ in dataset.as_numpy_iterator():
        yield [i]



ops, activations, num_neurons, byte_model, max_tmp_buf_size = generate_ssie_model(
    model, NUM_INPUTS, NUM_CLASSES, QUANTIZE, lambda: data_gen(test_dataset))
num_bytes = len(byte_model)

print("#pragma once")
print()
print("namespace ssie_model {")
print("constexpr uint8_t intermediateBufferSize = %d;" % max_tmp_buf_size)
print()
print("constexpr uint8_t numInputs = %d;" % NUM_INPUTS)
print("constexpr uint8_t numClasses = %d;" % NUM_CLASSES)
print()
print("constexpr uint8_t numOps = %d;" % len(ops))
print("constexpr Operation ops[] = {")
for op in ops:
    print("    " + toEnumOperation(op) + ", ")
print("};")
print()
print("constexpr Activation acts[] = {")
for act in activations:
    print("    " + toEnumActivation(act) + ", ")
print("};")
print()
print("constexpr uint8_t numNeurons[] = {")
for neurons in num_neurons:
    print("    " + str(neurons) + ", ")
print("};")
print()
print("constexpr uint16_t length = %d;" % num_bytes)

row_cntr = 0
if QUANTIZE:
    print("constexpr uint8_t buffer[length] = {")
else:
    print("constexpr float buffer[length] = {")
for byte in byte_model:
    if QUANTIZE:
        print("{0:#0{1}x}, ".format(np.uint8(byte),4), end="")
    else:
        print("%f, " % byte, end="")
    row_cntr += 1
    if row_cntr > 10:
        row_cntr = 0
        print()
print("};")
print("};")
