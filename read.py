import serial
import matplotlib.pyplot as plt
from drawnow import *

values = []

plt.ion()
cnt=0

serialData = serial.Serial('COM3', 9600)

def plotValues():
    plt.title('Obstacal detection')
    plt.grid(True)
    plt.ylabel('Values')
    plt.plot(values, 'rx-', label='values')
    plt.legend(loc='upper right')

#pre-load dummy data
for i in range(0,26):
    values.append(0)
    
while True:
    while (serialData.inWaiting()==0):
        pass
    valueRead = serialData.readline()

    #check if valid value can be casted
    try:
        val_digital = int(valueRead)
        print(val_digital)
        if val_digital <= 1:
            if val_digital >= 0:
                values.append(val_digital)
                values.pop(0)
                drawnow(plotValues)
            else:
                print("Invalid! negative number")
        else:
            print("Invalid! too large")
    except ValueError:
        print("Invalid! cannot cast")
