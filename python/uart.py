#!/usr/bin/python

import serial
import time
import os

from utils import *


def REQ_GPIO(chip, line, value):
    gpio = chip.get_line(line)
    gpio.request(consumer='UART_DRVIER', type=gpiod.LINE_REQ_DIR_OUT)
    gpio.set_value(value)
    return gpio


if not DEBUG:
    import gpiod

    chip8 = gpiod.Chip('gpiochip8')
    chip7 = gpiod.Chip('gpiochip7')
    chip5 = gpiod.Chip('gpiochip5')
    chip1 = gpiod.Chip('gpiochip1')

    # CTRL 1 (LTC2872)-------------------------------------------------------------------

    # 1 ---> uart1 (485/422), 0 ---> uart_1_2 (232)
    UART_CTRL_1_485_232_1 = REQ_GPIO(chip=chip5, line=13, value=1)

    # 1 ---> uart4 (485/422), 0 ---> uart_4_5 (232)
    UART_CTRL_1_485_232_2 = REQ_GPIO(chip=chip5, line=14, value=1)

    # Loop Back
    UART_CTRL_1_LB = REQ_GPIO(chip=chip5, line=25, value=0)

    # 0 ---> Enable RX uart1,2 
    UART_CTRL_1_RXEN1 = REQ_GPIO(chip=chip5, line=27, value=0)

    # 1 ---> Enable Tx uart1,2 
    UART_CTRL_1_DXEN1 = REQ_GPIO(chip=chip5, line=28, value=1)

    # 1 ---> Enable Tx uart4,5 
    UART_CTRL_1_DXEN2 = REQ_GPIO(chip=chip5, line=29, value=1)

    # 0 ---> Enable Rx uart4,5
    UART_CTRL_1_RXEN2 = REQ_GPIO(chip=chip5, line=30, value=0)

    # CTRL 2 (LTC2872)-------------------------------------------------------------------

    # 1 ---> uart6 (485/422), 0 ---> uart_6_7 (232)
    UART_CTRL_2_485_232_1 = REQ_GPIO(chip=chip5, line=31, value=1)

    # 1 ---> uart8 (485/422), 0 ---> uart_8_9 (232)
    UART_CTRL_2_485_232_2 = REQ_GPIO(chip=chip1, line=22, value=1)

    # Loop Back
    UART_CTRL_2_LB = REQ_GPIO(chip=chip7, line=29, value=0)

    # 0 ---> Enable RX uart6,7
    UART_CTRL_2_RXEN1 = REQ_GPIO(chip=chip7, line=31, value=0)

    # 1 ---> Enable TX uart6,7
    UART_CTRL_2_DXEN1 = REQ_GPIO(chip=chip1, line=0, value=1)

    # 1 ---> Enable TX uart8,9
    UART_CTRL_2_DXEN2 = REQ_GPIO(chip=chip1, line=1, value=1)

    # 0 ---> Enable RX uart8,9
    UART_CTRL_2_RXEN2 = REQ_GPIO(chip=chip1, line=2, value=0)


MODE_485_422 = "(485/422)"
MODE_232 = "(232)"


UARTS = [
    {'name': 'UART1', 'mode': MODE_485_422, 'dev': '/dev/ttyS0', 'gpio': [1, None, None, None]},
    {'name': 'UART1', 'mode': MODE_232,     'dev': '/dev/ttyS0', 'gpio': [0, None, None, None]},
    {'name': 'UART2', 'mode': MODE_232,     'dev': '/dev/ttyS1', 'gpio': [0, None, None, None]},
    {'name': 'UART4', 'mode': MODE_485_422, 'dev': '/dev/ttyS3', 'gpio': [None, 1, None, None]},
    {'name': 'UART4', 'mode': MODE_232,     'dev': '/dev/ttyS3', 'gpio': [None, 0, None, None]},
    {'name': 'UART5', 'mode': MODE_232,     'dev': '/dev/ttyS4', 'gpio': [None, 0, None, None]},
    {'name': 'UART6', 'mode': MODE_485_422, 'dev': '/dev/ttyS5', 'gpio': [None, None, 1, None]},
    {'name': 'UART6', 'mode': MODE_232,     'dev': '/dev/ttyS5', 'gpio': [None, None, 0, None]},
    {'name': 'UART7', 'mode': MODE_232,     'dev': '/dev/ttyS6', 'gpio': [None, None, 0, None]},
    {'name': 'UART8', 'mode': MODE_485_422, 'dev': '/dev/ttyS7', 'gpio': [None, None, None, 1]},
    {'name': 'UART8', 'mode': MODE_232,     'dev': '/dev/ttyS7', 'gpio': [None, None, None, 0]},
    {'name': 'UART9', 'mode': MODE_232,     'dev': '/dev/ttyS8', 'gpio': [None, None, None, 0]},
]

if not DEBUG:
    CURRENT_UART = serial.Serial(UARTS[0]['dev'], baudrate=DEFAULT_BAUDRATE)
else:
    CURRENT_UART = serial.Serial()


def change_current_uart_gui(idx):
    global CURRENT_UART
    CURRENT_UART.close()

    if not DEBUG:
        CURRENT_UART = serial.Serial(UARTS[idx]['dev'], baudrate=DEFAULT_BAUDRATE)
    else:
        CURRENT_UART = serial.Serial()



def set_ctrl_pins(config):
    if not DEBUG:
        ctrl_pins = [
            UART_CTRL_1_485_232_1,
            UART_CTRL_1_485_232_2,
            UART_CTRL_2_485_232_1,
            UART_CTRL_2_485_232_2,
        ]
        idx, value = next(((i, elem) for i, elem in enumerate(config) if elem is not None))
        ctrl_pins[idx].set_value(value)


def write_gui(data):
    if not DEBUG:
        global CURRENT_UART
        CURRENT_UART.write(data)


def read_gui():
    if not DEBUG:
        global CURRENT_UART
        return CURRENT_UART.read_all()


def write_console(uart_idx, data):
    if not DEBUG:
        ser = serial.Serial(UARTS[int(uart_idx)]['dev'])
        ser.write(data)
        print(f"{RED('SENT:')} {format_packet(data)}")
        time.sleep(0.1)
        print(f"{RED('RECV:')} {format_packet(ser.read_all())}\n")
        ser.close()
    else:
        print(f"{RED('SENT:')} {format_packet(data)}\n")


        
def UART_Close():
    if not DEBUG:
        UART_CTRL_1_485_232_1.release()
        UART_CTRL_1_485_232_2.release()
        UART_CTRL_1_LB.release()
        UART_CTRL_1_RXEN1.release()
        UART_CTRL_1_DXEN1.release()
        UART_CTRL_1_DXEN2.release()
        UART_CTRL_1_RXEN2.release()
        UART_CTRL_2_485_232_1.release()
        UART_CTRL_2_485_232_2.release()
        UART_CTRL_2_LB.release()
        UART_CTRL_2_RXEN1.release()
        UART_CTRL_2_DXEN1.release()
        UART_CTRL_2_DXEN2.release()
        UART_CTRL_2_RXEN2.release()

        chip8.close()
        chip7.close()
        chip5.close()
        chip1.close()


def main():
    clear_screen()
    while True:
        print("Choose Your Port:\n")
        for i, uart in enumerate(UARTS):
            print(f"{RED(str(i)+')')} {uart['name']} {uart['mode']} {uart['dev']}")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        option = input("Option: ")
        print()
        if option.upper() == 'B':
            clear_screen()
            return
        elif option in [str(i) for i in range(len(UARTS))]:            
            set_ctrl_pins(UARTS[int(option)]['gpio'])
            while True:
                data_size = input('Enter data size in bytes: ')
                print()
                if data_size.isdigit():
                    data = os.urandom(int(data_size))
                    write_console(int(option), data)
                    break
                else:
                    clear_screen()
        else:
            clear_screen()
        





