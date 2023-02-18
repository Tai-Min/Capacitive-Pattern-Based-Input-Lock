import tensorflow as tf
import numpy as np
import pprint

pp = pprint.PrettyPrinter()

test_input = np.array([0, 0, 48, 0, 0, 2, 0, 0, 0, 0, 4, 0, 0, 0, 4, 32, 0, 0, 0, 1, 0, 1, 0, 64, 0, 0, 0,
                      0, 64, 0, 0, 17, 0, 0, 0, 32, 64, 0, 0, 16, 8, 0, 0, 0, 32, 2, 0, 0, 0, 0], ndmin=2, dtype=np.uint8)

interpreter = tf.lite.Interpreter(model_path="../model/model.tflite")
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()[0]
if input_details["dtype"] != np.uint8:
    test_input = test_input.astype(np.float32)
    test_input /= 255.0

output_details = interpreter.get_output_details()[0]

interpreter.set_tensor(input_details['index'], test_input)
interpreter.invoke()

#["X", "triangle", "heart", "circle",  "square"]
res = interpreter.get_tensor(output_details['index'])
print(res)
