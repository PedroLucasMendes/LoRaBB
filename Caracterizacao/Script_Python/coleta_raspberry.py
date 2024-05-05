import serial
import time
import os
import logging

porta_serial = '/dev/ttyUSB0'
baudrate = 115200
timeout = 1


logger=logging.getLogger('log')
logging.basicConfig(filename='/home/pi/Desktop/send_experimento.log',
        filemode='a',
        format='%(asctime)s,%(msecs)d;%(name)s;%(levelname)s;%(message)s',
        datefmt='%H:%M:%S',
        level=logging.INFO)

def write_read(arduino):
    while True:
        data = arduino.readline().decode()
        if data:
            print(data)
            logging.info(data + '\n')

def main():
    #Conexão com a porta USB0
    while True:
        print(f"Tentando conectar na porta {porta_serial}")
        try: 
            arduino = serial.Serial(port=porta_serial, baudrate=baudrate, timeout=0.5)
            break
        except:
            print(".", end=" ")

    print(f"Gravando dados")

    write_read(arduino)
    arduino.close()

if __name__ == '__main__':
    main()
