#!/usr/bin/python

import serial
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

    # 1 ---> uart1 (485/422), 0 ---> uart2 (232)
    UART_CTRL_1_485_232_1 = REQ_GPIO(chip=chip5, line=13, value=1)

    # 1 ---> uart4 (485/422), 0 ---> uart5 (232)
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

    # 1 ---> uart6 (485/422), 0 ---> uart7 (232)
    UART_CTRL_2_485_232_1 = REQ_GPIO(chip=chip5, line=31, value=1)

    # 1 ---> uart8 (485/422), 0 ---> uart9 (232)
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


UART_DEVICE = {
    'UART1 (485/422)': '/dev/ttyS0',
    'UART2 (232)': '/dev/ttyS1',
    'UART4 (485/422)': '/dev/ttyS3',
    'UART5 (232)': '/dev/ttyS4',
    'UART6 (485/422)': '/dev/ttyS5',
    'UART7 (232)': '/dev/ttyS6',
    'UART8 (485/422)': '/dev/ttyS7',
    'UART9 (232)': '/dev/ttyS8',
}


if not DEBUG:
    UART_STATUS = {
        1: [{'UART1 (485/422)': True, 'UART2 (232)': False}, UART_CTRL_1_485_232_1],
        2: [{'UART4 (485/422)': True, 'UART5 (232)': False}, UART_CTRL_1_485_232_2],
        3: [{'UART6 (485/422)': True, 'UART7 (232)': False}, UART_CTRL_2_485_232_1],
        4: [{'UART8 (485/422)': True, 'UART9 (232)': False}, UART_CTRL_2_485_232_2], 
    }
else:
    UART_STATUS = {
        1: [{'UART1 (485/422)': True, 'UART2 (232)': False}, None],
        2: [{'UART4 (485/422)': True, 'UART5 (232)': False}, None],
        3: [{'UART6 (485/422)': True, 'UART7 (232)': False}, None],
        4: [{'UART8 (485/422)': True, 'UART9 (232)': False}, None], 
    }


if not DEBUG:
    UART_LOOPBACK = {
        1: [['UART1 (485/422)', 'UART2 (232)', 
             'UART4 (485/422)', 'UART5 (232)'], UART_CTRL_1_LB],

        2: [['UART6 (485/422)', 'UART7 (232)', 
             'UART8 (485/422)', 'UART9 (232)'], UART_CTRL_2_LB],   
    }
else:
    UART_LOOPBACK = {
        1: [['UART1 (485/422)', 'UART2 (232)', 
             'UART4 (485/422)', 'UART5 (232)'], False],

        2: [['UART6 (485/422)', 'UART7 (232)', 
             'UART8 (485/422)', 'UART9 (232)'], False],   
    }

UART_PORTS = {}

def update_ports():
    global UART_PORTS
    UART_PORTS = {
        1: ['UART1 (485/422) ttyS0', UART_STATUS[1][0]['UART1 (485/422)'], '/dev/ttyS0'],
        2: ['UART2 (232) ttyS1', UART_STATUS[1][0]['UART2 (232)'], '/dev/ttyS1'],
        3: ['UART4 (485/422) ttyS3', UART_STATUS[2][0]['UART4 (485/422)'], '/dev/ttyS3'],
        4: ['UART5 (232) ttyS4', UART_STATUS[2][0]['UART5 (232)'], '/dev/ttyS4'],
        5: ['UART6 (485/422) ttyS5', UART_STATUS[3][0]['UART6 (485/422)'], '/dev/ttyS5'],
        6: ['UART7 (232) ttyS6', UART_STATUS[3][0]['UART7 (232)'], '/dev/ttyS6'],
        7: ['UART8 (485/422) ttyS7', UART_STATUS[4][0]['UART8 (485/422)'], '/dev/ttyS7'],
        8: ['UART9 (232) ttyS8', UART_STATUS[4][0]['UART9 (232)'], '/dev/ttyS8'],
    }

update_ports()


def enabled_disable_uarts_loop():
    global UART_STATUS
    while True:
        print('*'*20 + PURPLE('Enable/Disable UART ports') + 20*'*' + '\n')

        print("Choose from options below to Enable/Disable uart ports:\n")
        for i, ports in UART_STATUS.items():
            print(f"{RED(str(i) + ')')}", end=' ')
            for key, enblaed in ports[0].items():
                space = '' if '422' in key else '   '
                if enblaed:
                    print(f"{space}{key}: {GREEN('Enabled')}")
                else:
                    print(f"{space}{key}: {RED('Disabled')}")

            print()

        print(f"{RED('B)')} {BLUE('Back')}\n")

        option = input('Option: ')
        clear_screen()

        if option.upper() == 'B':
            update_ports()
            return
        elif option in ['1', '2', '3', '4']:
            for key in UART_STATUS[int(option)][0]:
                UART_STATUS[int(option)][0][key] = not UART_STATUS[int(option)][0][key]
            
            if not DEBUG:
                val = 0 if UART_STATUS[int(option)][1].get_value() == 1 else 1
                UART_STATUS[int(option)][1].set_value(val)
            

def enabled_disable_loopback_loop():
    global UART_LOOPBACK

    while True:
        print('*'*20 + PURPLE('Enable/Disable Loopback') + 20*'*' + '\n')
        print("Choose from options below to Enable/Disable loopback:\n")
        for i, port_list in UART_LOOPBACK.items():
            print(f"{RED(str(i) + ')')}", end='')
            for j, port in enumerate(port_list[0]):
                if j == 0:
                    print(f' {port}')
                else:
                    print(f'   {port}')

            if not DEBUG:
                print('-'*20 + f"> Loopback: {GREEN('Enabled') if UART_LOOPBACK[i][1].get_value() else RED('Disabled')}")
            else:
                print('-'*20 + f"> Loopback: {GREEN('Enabled') if UART_LOOPBACK[i][1] else RED('Disabled')}")

            print()

        print(f"{RED('B)')} {BLUE('Back')}\n")

        option = input('Option: ')
        clear_screen()

        if option.upper() == 'B':
            return
        elif option in ['1', '2']:
            if not DEBUG:
                val = 0 if UART_LOOPBACK[int(option)][1].get_value() == 1 else 1
                UART_LOOPBACK[int(option)][1].set_value(val)
            else:
                UART_LOOPBACK[int(option)][1] = not UART_LOOPBACK[int(option)][1]


def write_to_buffer_loop():
    global UART_PORTS
    while True:
        print('*'*20 + PURPLE('Write To Buffer') + 20*'*' + '\n')
        print('Choose UART port:\n')
        available_ports = []
        for i, port in UART_PORTS.items():
            if port[1]:
                print(f"{RED(str(i) + ')')} {port[0]}")
                available_ports.append(str(i))
        
        print(f"{RED('B)')} {BLUE('Back')}\n")

        option = input('Option: ')
        print()
        if option.upper() == 'B':
            clear_screen()
            return
        elif option in available_ports:
            while True:
                data_size = input('Enter Data Size in Bytes: ')
                print()
                if data_size.isdigit():
                    ser = serial.Serial(UART_PORTS[int(option)][2])
                    data = os.urandom(int(data_size))
                    ser.write(data)
                    print(f"{RED('SENT:')} {format_packet(data)}")
                    print(f"{RED('RECV:')} {format_packet(ser.read_all())}\n")
                    ser.close()
                    break
        else:
            clear_screen()

        
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
        print("What Do You Want To Do?\n")
        print(f"{RED('1)')} Enable/Disable UART ports")
        print(f"{RED('2)')} Enable/Disable Loopback")
        print(f"{RED('3)')} Write To Buffer")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        option = input("Option: ").upper()
        clear_screen()
        
        if option == '1':
            enabled_disable_uarts_loop()
        elif option == '2':
            enabled_disable_loopback_loop()
        elif option == '3':
            write_to_buffer_loop()
        elif option.upper() == 'B':
            clear_screen()
            return
        else:
            clear_screen()





