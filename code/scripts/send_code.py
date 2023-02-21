import serial
import numpy as np

ser = serial.Serial()
ser.port = "COM4"
ser.baudrate = 115200

RELAY_SET_PATTERN_CMD = 1

X = 0
TRIANGLE = 1
HEART = 2
CIRCLE = 3
SQUARE = 4

relay = 1
pattern = [X]

buf = [RELAY_SET_PATTERN_CMD, relay] + pattern 

def crc(arr):
    sum1 = 0
    sum2 = 0

    for val in arr:
        sum1 = np.uint8(sum1 + val) % 255
        sum2 = np.uint8(sum2 + sum1) % 255

    return [sum2, sum1]


buf += crc(buf)
buf = bytes(buf)

print(buf)

ser.open()

ser.write(buf)

res = ser.readline()
print(res)