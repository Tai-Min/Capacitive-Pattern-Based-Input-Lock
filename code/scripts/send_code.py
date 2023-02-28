import serial
import numpy as np
import time

ser = serial.Serial()
ser.port = "COM4"
ser.baudrate = 115200

RELAY_SET_PATTERN_CMD = 1

X = 0
TRIANGLE = 1
HEART = 2
CIRCLE = 3
SQUARE = 4

def crc(arr):
    sum1 = 0
    sum2 = 0

    for val in arr:
        sum1 = np.uint8(sum1 + val) % 255
        sum2 = np.uint8(sum2 + sum1) % 255

    return [sum2, sum1]

def send_code(serial, relay, code):
    buf = [RELAY_SET_PATTERN_CMD, relay] + code
    buf += crc(buf)
    buf = bytes(buf)

    ser.write(buf)
    res = ser.readline()
    print(res)

ser.open()
delay = 0.05

send_code(ser, 0, [X])
time.sleep(delay)
send_code(ser, 1, [X, HEART])
time.sleep(delay)
send_code(ser, 2, [HEART, TRIANGLE, SQUARE])
time.sleep(delay)
send_code(ser, 3, [CIRCLE, HEART])