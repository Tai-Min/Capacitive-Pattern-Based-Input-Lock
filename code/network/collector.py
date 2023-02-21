import csv
from os.path import exists

import matplotlib.pyplot as plt
import numpy as np
import serial
from matplotlib.animation import FuncAnimation

# EDIT AS DESIRED

label = "square"

img_width = 20
img_height = 20
baud = 115200
port = "COM4"

# DO NOT TOUCH BELOW
bits_in_byte = 8
num_bytes = img_width * img_height // bits_in_byte

def charactersToBinary(reading):
    return [1.0 if r == 'X' else 0.0 for r in reading]

def binaryToSample(reading):
    sample = [0] * num_bytes

    cntr = 0
    for i in range(num_bytes):
        for shift in range(bits_in_byte):
            sample[i] |= ((int(reading[cntr]) << shift))
            cntr += 1
    return sample

def sampleToBinary(sample):
    res = [0] * img_height * img_width

    for x in range(img_width):
        for y in range(img_height):
            bit_to_set = int(y * img_width + x)
            arr_idx = int(bit_to_set / 8)
            shift = int(bit_to_set % 8)
            if int(sample[arr_idx]) & (1 << shift):
                res[bit_to_set] = 1.0
    return res

def find_available_filename():
    fname = "./dataset/" + label + "." + str(find_available_filename.cntr) + ".csv"

    while exists(fname):
        find_available_filename.cntr += 1
        fname = "./dataset/" + label + "." + str(find_available_filename.cntr) + ".csv"

    return fname
find_available_filename.cntr = 1

def saveSample(sample):
    with open(find_available_filename(), "w", newline='') as file:
        writer = csv.writer(file)
        writer.writerow(saveSample.csv_header)
        writer.writerow(sample)
saveSample.csv_header = ["byte" + str(s) for s in range(num_bytes)]

def update(frame):
    update.ser.flush()
    reading = update.ser.readline()
    reading = reading.decode("utf-8").strip()

    reading = charactersToBinary(reading)

    if len(reading) == img_height * img_width:
        parsedSamlpe = binaryToSample(reading)
        print(parsedSamlpe)

        #reading = sampleToBinary(parsedSamlpe)
        #parsedSamlpe = binaryToSample(reading)
        #print(parsedSamlpe)

        saveSample(parsedSamlpe)

        update.image = np.reshape(reading, ((img_height, img_width)))
    elif len(reading) != 0:
        update.image = np.zeros((img_height, img_width))

    update.ax.clear()
    return update.ax.imshow(update.image)

if __name__ == "__main__":
    fig = plt.figure()

    update.ser = serial.Serial()
    update.ser.baudrate = baud
    update.ser.port = port
    update.ser.timeout = 0.05
    update.ser.open()
    update.image = np.zeros((img_height, img_width))
    update.ax = fig.gca()
    update.img = update.ax.imshow(update.image)

    animation = FuncAnimation(fig, update, interval=100)
    plt.show()