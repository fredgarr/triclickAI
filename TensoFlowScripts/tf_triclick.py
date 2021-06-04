"""
 A record is a serie of 3 pulses, so 6 transition edges between 0 and 3000ms
 As the first pulse will always be recorded as '0' ms, it is ignored in the 
 following computations.

    Example :  '. . __'

        +------+        +----+          +--------------+
    ____|      |________|    |__________|              |________
       t0=0    t1       t2   t3         t4             t5

 Each pulse is either a short pulse (.) or a long pulse (__).
 By default, short pulse duration is 200ms and long pulse is 700ms.
 Randomisation of the pulse will add noise on the duration +/-299ms or +/-150ms on the transition
 date.
 
 '__' pulse duration will be between 700-299 = 399ms and 700+299 = 999ms
 '.' pulse duration will be between 200-150 = 50ms and 200+150 = 350ms


MIT License
Copyright (c) 2019 fredgarr
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

from __future__ import absolute_import, division, print_function, unicode_literals

# TensorFlow and tf.keras
import tensorflow as tf
from tensorflow import keras

# Helper libraries
import numpy as np
import matplotlib.pyplot as plt

import random as rnd
import logging

import serial


# Logger configuration
# =======================================================
logging.basicConfig(level=logging.DEBUG)

# Constants definition
# =======================================================
SHORT_PULSE = 200
LONG_PULSE = 700

MAX_NOISE_LONG = 299
MAX_NOISE_SHORT = 150

TOTAL_DURATION = 3000

CODED_DATA = {
 0 : ['.','.','.'],
 1 : ['.','.','__'],
 2 : ['.','__','.'],
 3 : ['__','.','.'],
 4 : ['.','__','__'],
 5 : ['__','.','__'],
 6 : ['__','__','.'],
 7 : ['__','__','__']
}

LEARNING_SEED = 1234
VERIFY_SEED = 4321

# Code part 
# =======================================================

def get_short_pulse():
    return rnd.randint(SHORT_PULSE - MAX_NOISE_SHORT, SHORT_PULSE + MAX_NOISE_SHORT)*1.0


def get_long_pulse():
    return rnd.randint(LONG_PULSE - MAX_NOISE_LONG, LONG_PULSE + MAX_NOISE_LONG)*1.0


def get_pulse(pulseType):

    if pulseType in '.':
        return get_short_pulse()
    elif pulseType in '__':
        return get_long_pulse()
    else:
        logging.error("Unkown pulse type")
        return None


def get_sequence(seqNumber):
    
    # Get 3 pulses, then translate into 6 edges
    pulses = []
    edgList = np.empty((0))
    
    codedSequence = CODED_DATA[seqNumber]

    # Get the 3 pulses
    for k in range(3):
        pulses.append(get_pulse(codedSequence[k]))
    
    # Compute total pulse time, then deduce 'idle' time
    pulseLen = pulses[0] + pulses[1] + pulses[2] 
    idleTime = TOTAL_DURATION - pulseLen

    # Build edge list
    linTime = 0
    for k in range(1,4):    
        edgList = np.append(edgList, [linTime + pulses[k-1]])
        linTime = linTime + pulses[k-1]
        idleSlot = rnd.randint(1, idleTime) - 1
        if k < 3:
            edgList = np.append(edgList, [linTime + idleSlot])
            linTime = linTime + idleSlot
            idleTime = idleTime - idleSlot

    return edgList


def makeInputData(numberOfOutputs):

    sequenceList = np.empty((0))
    pulsesList = np.empty((0,5))

    for __ in range(numberOfOutputs):
        for k in range(8):
            sequenceList = np.append(sequenceList, [k], axis=0)
            tutu = get_sequence(k)
            pulsesList = np.append(pulsesList, [tutu], axis=0)
    
    return sequenceList, pulsesList


def arduino_test(tfModel):
    ser = serial.Serial('/dev/ttyUSB0', 115200)

    while True:
        data = ser.readline()
        if data:
            dataSplit = data.split('\r\n')[0]
            dataSplit = np.array([map(int,dataSplit.split(','))]) / (TOTAL_DURATION*1.0)
            prediction = tfModel.predict(dataSplit)
            print(CODED_DATA[np.argmax(prediction)])


if __name__ == "__main__":


    # Build a training vector:
    rnd.seed(LEARNING_SEED)
    learningSeqNum, learningPulses = makeInputData(1000)
    
    # Normalize between [0..1]
    learningPulses = learningPulses / (TOTAL_DURATION*1.0)

    # Build a verification vector:
    rnd.seed(VERIFY_SEED)
    verifySeqNum, verifyPulses = makeInputData(500)

    verifyPulses = verifyPulses / (TOTAL_DURATION*1.0)

    #===============================================================
    #
    # Code derived from https://www.tensorflow.org/tutorials/keras/classification
    #
    # Instentiate neural net model:
    #  - Input layer takes 5 'edges'
    #  - Internal layer takes 64 'Neurons'
    #  - Output layer takes 8 nodes (probability detection result)
    model = keras.Sequential([
        keras.layers.Dense(5),
        keras.layers.Dense(128, activation='relu'),
        keras.layers.Dense(8, activation='softmax')
    ])

    # Compile the model:
    model.compile(optimizer='adam',
              loss='sparse_categorical_crossentropy',
              metrics=['accuracy'])
    
    # Train and save the Model:
    print("Input shape: %r, Output shape: %r" %(learningPulses.shape, learningSeqNum.shape))
    model.fit(learningPulses, learningSeqNum, epochs=20)

    # model.save("tf_model/saved")

    # # Convert model to fixpoint:
    # converter = tf.compat.v2.lite.TFLiteConverter.from_keras_model(model)
    # converter.optimizations = [tf.lite.Optimize.DEFAULT]
    # converter.target_spec.supported_types = [tf.lite.constants.QUANTIZED_UINT8]
    # tflite_quant_model = converter.convert()



    # Evaluate accuracy of the model:
    test_loss, test_acc = model.evaluate(verifyPulses,  verifySeqNum, verbose=2)
    print('\nTest accuracy:', test_acc)

    # Connect to Arduino and test the trained model
    arduino_test(model)