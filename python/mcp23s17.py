#!/usr/bin/python

import time
from utils import *


if not DEBUG:
    import spidev
    import gpiod


    chip7 = gpiod.Chip('gpiochip7')
    chip5 = gpiod.Chip('gpiochip5')
    chip4 = gpiod.Chip('gpiochip4')

    ioe1_cs = chip7.get_line(28)
    ioe1_cs.request(consumer='MCP23S17_DRIVER', type=gpiod.LINE_REQ_DIR_OUT)
    ioe1_cs.set_value(1)
    time.sleep(0.010)
    ioe1_cs.set_value(0)
    time.sleep(0.010)
    ioe1_cs.set_value(1)


    ioe1_reset = chip4.get_line(22)
    ioe1_reset.request(consumer='MCP23S17_DRIVER', type=gpiod.LINE_REQ_DIR_OUT)
    ioe1_reset.set_value(1)
    time.sleep(0.010)
    ioe1_reset.set_value(0)
    time.sleep(0.010)
    ioe1_reset.set_value(1)


    ioe2_cs = chip4.get_line(19)
    ioe2_cs.request(consumer='MCP23S17_DRIVER', type=gpiod.LINE_REQ_DIR_OUT)
    ioe2_cs.set_value(1)
    time.sleep(0.010)
    ioe2_cs.set_value(0)
    time.sleep(0.010)
    ioe2_cs.set_value(1)


    ioe2_reset = chip5.get_line(11)
    ioe2_reset.request(consumer='MCP23S17_DRIVER', type=gpiod.LINE_REQ_DIR_OUT)
    ioe2_reset.set_value(1)
    time.sleep(0.010)
    ioe2_reset.set_value(0)
    time.sleep(0.010)
    ioe2_reset.set_value(1)


    spi_ioe = spidev.SpiDev()
    spi_ioe.open(2, 0)
    spi_ioe.max_speed_hz = 10000000
    spi_ioe.mode = 0b00
    spi_ioe.bits_per_word = 8


# MPC23S17 ----------------------------------------------------------

class MCP23S17:

    PULLUP_ENABLED = 0
    PULLUP_DISABLED = 1
    PULLUP_STR = {PULLUP_ENABLED: 'ENABLED', PULLUP_DISABLED: 'DISABLED'}

    DIR_INPUT = 0
    DIR_OUTPUT = 1
    DIR_STR = {DIR_INPUT: 'INPUT', DIR_OUTPUT: 'OUTPUT'}

    LEVEL_LOW = 0
    LEVEL_HIGH = 1
    LEVEL_STR = {LEVEL_LOW: 'LOW', LEVEL_HIGH: 'HIGH'}

    # Register addresses -------------------------------------------------
    MCP23S17_IODIRA = 0x00
    MCP23S17_IODIRB = 0x01
    MCP23S17_IPOLA = 0x2
    MCP23S17_IPOLB = 0x3
    MCP23S17_GPIOA = 0x12
    MCP23S17_GPIOB = 0x13
    MCP23S17_OLATA = 0x14
    MCP23S17_OLATB = 0x15
    MCP23S17_IOCON = 0x0A
    MCP23S17_GPPUA = 0x0C
    MCP23S17_GPPUB = 0x0D

    # Bit field flags ----------------------------------------------------
    IOCON_UNUSED = 0x01
    IOCON_INTPOL = 0x02
    IOCON_ODR = 0x04
    IOCON_HAEN = 0x08
    IOCON_DISSLW = 0x10
    IOCON_SEQOP = 0x20
    IOCON_MIRROR = 0x40
    IOCON_BANK_MODE = 0x80
    IOCON_INIT = 0x28  # IOCON_SEQOP and IOCON_HAEN from above
    MCP23S17_CMD_WRITE = 0x40
    MCP23S17_CMD_READ = 0x41

    def __init__(self, spi, cs, reset, name, device_id=0x00):
        self.name = name
        self.device_id = device_id
        self._GPIOA = 0
        self._GPIOB = 0
        self._IODIRA = 0
        self._IODIRB = 0
        self._GPPUA = 0
        self._GPPUB = 0
        self._pin_reset = reset
        self._pin_cs = cs
        self._spimode = 0b00
        self._spi = spi
        print(f"[{PURPLE(self.name)}] Init")
        self._writeRegister(MCP23S17.MCP23S17_IOCON, MCP23S17.IOCON_INIT)


    def setPullupMode(self, pin, mode):
        assert pin < 16
        assert (mode == MCP23S17.PULLUP_ENABLED) or (mode == MCP23S17.PULLUP_DISABLED)

        print(f'Setting pullup mode to {GREEN(MCP23S17.PULLUP_STR[mode])} for pin {GREEN(pin)}.')

        if pin < 8:
            register = MCP23S17.MCP23S17_GPPUA
            data = self._GPPUA
            noshifts = pin
        else:
            register = MCP23S17.MCP23S17_GPPUB
            noshifts = pin & 0x07
            data = self._GPPUB

        if mode == MCP23S17.PULLUP_ENABLED:
            data |= (1 << noshifts)
        else:
            data &= (~(1 << noshifts))

        self._writeRegister(register, data)

        if pin < 8:
            self._GPPUA = data
        else:
            self._GPPUB = data

    def getPullupMode(self, pin):
        assert (pin < 16)

        print(f'Get pullup mode of pin {GREEN(pin)}.')

        if pin < 8:
            self._GPPUA = self._readRegister(MCP23S17.MCP23S17_GPPUA)
            if (self._GPPUA & (1 << pin)) != 0:
                result = MCP23S17.PULLUP_ENABLED
            else:
                result = MCP23S17.PULLUP_DISABLED
        else:
            self._GPPUB = self._readRegister(MCP23S17.MCP23S17_GPPUB)
            pin &= 0x07
            if (self._GPPUB & (1 << pin)) != 0:
                result = MCP23S17.PULLUP_ENABLED
            else:
                result = MCP23S17.PULLUP_DISABLED

        print('Result:', GREEN(self.PULLUP_STR[result]), '\n')
        return result

    def setDirection(self, pin, direction):
        assert (pin < 16)
        assert ((direction == MCP23S17.DIR_INPUT) or (direction == MCP23S17.DIR_OUTPUT))

        print(f'Setting direction of pin {GREEN(pin)} to {GREEN(MCP23S17.DIR_STR[direction])}.')

        if pin < 8:
            register = MCP23S17.MCP23S17_IODIRA
            data = self._IODIRA
            noshifts = pin
        else:
            register = MCP23S17.MCP23S17_IODIRB
            noshifts = pin & 0x07
            data = self._IODIRB

        if direction == MCP23S17.DIR_INPUT:
            data |= (1 << noshifts)
        else:
            data &= (~(1 << noshifts))

        self._writeRegister(register, data)

        if (pin < 8):
            self._IODIRA = data
        else:
            self._IODIRB = data

    def getDirection(self, pin):
        assert (pin < 16)

        print(f'Get direction of pin {GREEN(pin)}.')

        if pin < 8:
            self._IODIRA = self._readRegister(MCP23S17.MCP23S17_IODIRA)
            if (self._IODIRA & (1 << pin)) != 0:
                result = MCP23S17.DIR_INPUT
            else:
                result = MCP23S17.DIR_OUTPUT
        else:
            self._IODIRB = self._readRegister(MCP23S17.MCP23S17_IODIRB)
            pin &= 0x07
            if (self._IODIRB & (1 << pin)) != 0:
                result = MCP23S17.DIR_INPUT
            else:
                result = MCP23S17.DIR_OUTPUT

        print('Result:', GREEN(self.DIR_STR[result]), '\n')
        return result

    def getLevel(self, pin):
        assert (pin < 16)

        print(f'Get level of pin {GREEN(pin)}.')

        if pin < 8:
            self._GPIOA = self._readRegister(MCP23S17.MCP23S17_GPIOA)
            if (self._GPIOA & (1 << pin)) != 0:
                result = MCP23S17.LEVEL_HIGH
            else:
                result = MCP23S17.LEVEL_LOW
        else:
            self._GPIOB = self._readRegister(MCP23S17.MCP23S17_GPIOB)
            pin &= 0x07
            if (self._GPIOB & (1 << pin)) != 0:
                result = MCP23S17.LEVEL_HIGH
            else:
                result = MCP23S17.LEVEL_LOW

        print('Result:', GREEN(self.LEVEL_STR[result]), '\n')
        return result

    def setLevel(self, pin, level):
        assert (pin < 16)
        assert (level == MCP23S17.LEVEL_HIGH) or (level == MCP23S17.LEVEL_LOW)

        print(f'Setting level of pin {GREEN(pin)} to {GREEN(MCP23S17.LEVEL_STR[level])}.')

        if pin < 8:
            register = MCP23S17.MCP23S17_GPIOA
            data = self._GPIOA
            noshifts = pin
        else:
            register = MCP23S17.MCP23S17_GPIOB
            noshifts = pin & 0x07
            data = self._GPIOB

        if level == MCP23S17.LEVEL_HIGH:
            data |= (1 << noshifts)
        else:
            data &= (~(1 << noshifts))

        self._writeRegister(register, data)

        if (pin < 8):
            self._GPIOA = data
        else:
            self._GPIOB = data

    def setData(self, data):
        print(f'Setting data ({GREEN(data)}) on GPIOs.')
        self._GPIOA = (data & 0xFF)
        self._GPIOB = (data >> 8)
        self._writeRegisterWord(MCP23S17.MCP23S17_GPIOA, data)

    def getData(self):
        data = self._readRegisterWord(MCP23S17.MCP23S17_GPIOA)
        self._GPIOA = (data & 0xFF)
        self._GPIOB = (data >> 8)
        return data

    def _writeRegister(self, register, value):
        command = MCP23S17.MCP23S17_CMD_WRITE | (self.device_id << 1)
        recived = [0, 0, 0]
        if not DEBUG:
            self._pin_cs.set_value(0)
            recived = self._spi.xfer2([command, register, value])
            self._pin_cs.set_value(1)
        info = f"{RED('SENT:')} {format_packet(bytearray([command, register, value]))} --- "
        info += f"{RED('RECV:')} {format_packet(bytearray(recived))}\n"
        debug_print(info)

    def _readRegister(self, register):
        command = MCP23S17.MCP23S17_CMD_READ | (self.device_id << 1)
        data = [0, 0, 0]
        if not DEBUG:
            self._pin_cs.set_value(0)
            data = self._spi.xfer2([command, register, 0])
            self._pin_cs.set_value(1)
        info = f"{RED('SENT:')} {format_packet(bytearray([command, register, 0]))} --- "
        info += f"{RED('RECV:')} {format_packet(bytearray(data))}\n"
        debug_print(info)
        return data[2]

    def _readRegisterWord(self, register):
        buffer = [0, 0]
        buffer[0] = self._readRegister(register)
        buffer[1] = self._readRegister(register + 1)
        return (buffer[1] << 8) | buffer[0]

    def _writeRegisterWord(self, register, data):
        self._writeRegister(register, data & 0xFF)
        self._writeRegister(register + 1, data >> 8)




def select_ioe_loop():
    global ioe1, ioe2
    clear_screen()
    while True:
        print('*'*20 + PURPLE('Select IOE') + 20*'*' + '\n')
        print('Select IOE:\n')
        print(f"{RED('1)')} IOE 1")
        print(f"{RED('2)')} IOE 2\n")
        adc = input('Option: ')
        print()
        clear_screen()
        if adc == '1':
            return ioe1
        elif adc == '2':
            return ioe2


def set_pullup_mode_loop(ioe: MCP23S17):
    while True:
        print('*'*20 + PURPLE('Set Pullup Mode') + 20*'*' + '\n')
        print('Choose IOE Pin:\n')
        print(f"{RED('{0-15})')} Single Pin")
        print(f"{RED('A)')} All")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        option = input('Option: ')
        print()
        if option.upper() == 'B':
            clear_screen()
            return
        elif option.upper() in ['A'] + [str(i) for i in range(16)]:
            pin = option
            while True:
                print('Choose Pullup Mode:\n')
                print(f"{RED('E)')} Enabled")
                print(f"{RED('D)')} Disabled")
                print(f"{RED('B)')} {BLUE('Back')}\n")
                option = input('Option: ')
                print()
                if option.upper() == 'B':
                    clear_screen()
                    break
                elif option.upper() in ['E', 'D']:
                    mode = MCP23S17.PULLUP_ENABLED if option.upper() == 'E' else MCP23S17.PULLUP_DISABLED
                    if pin.upper() == 'A':
                        for i in range(16):
                            ioe.setPullupMode(i, mode)
                    else:
                        ioe.setPullupMode(int(pin), mode)

                else:
                    clear_screen()

        else:
            clear_screen()


def set_direction_loop(ioe: MCP23S17):
    while True:
        print('*'*20 + PURPLE('Set Direction') + 20*'*' + '\n')
        print('Choose IOE Pin:\n')
        print(f"{RED('{0-15})')} Single Pin")
        print(f"{RED('A)')} All")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        option = input('Option: ')
        print()
        if option.upper() == 'B':
            clear_screen()
            return
        elif option.upper() in ['A'] + [str(i) for i in range(16)]:
            pin = option
            while True:
                print('Choose Direction Mode:\n')
                print(f"{RED('I)')} Input")
                print(f"{RED('O)')} Output")
                print(f"{RED('B)')} {BLUE('Back')}\n")
                option = input('Option: ')
                print()
                if option.upper() == 'B':
                    clear_screen()
                    break
                elif option.upper() in ['I', 'O']:
                    direction = MCP23S17.DIR_INPUT if option.upper() == 'I' else MCP23S17.DIR_OUTPUT
                    if pin.upper() == 'A':
                        for i in range(16):
                            ioe.setDirection(i, direction)
                    else:
                        ioe.setDirection(int(pin), direction)

                else:
                    clear_screen()

        else:
            clear_screen()


def get_level_loop(ioe: MCP23S17):
    while True:
        print('*'*20 + PURPLE('Get Level') + 20*'*' + '\n')
        print('Choose IOE Pin:\n')
        print(f"{RED('{0-15})')} Single Pin")
        print(f"{RED('A)')} All")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        option = input('Option: ')
        print()
        if option.upper() == 'B':
            clear_screen()
            break
        elif option.upper() in ['A'] + [str(i) for i in range(16)]:
            if option.upper() == 'A':
                for i in range(16):
                    print('-'*60)
                    ioe.getLevel(i)
            else:
                ioe.getLevel(int(option))

        else:
            clear_screen()


def set_level_loop(ioe: MCP23S17):
    while True:
        print('*'*20 + PURPLE('Set Level') + 20*'*' + '\n')
        print('Choose IOE Pin:\n')
        print(f"{RED('{0-15})')} Single Pin")
        print(f"{RED('A)')} All")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        option = input('Option: ')
        print()
        if option.upper() == 'B':
            clear_screen()
            return
        elif option.upper() in ['A'] + [str(i) for i in range(16)]:
            pin = option
            while True:
                print('Choose Level Mode:\n')
                print(f"{RED('H)')} High")
                print(f"{RED('L)')} Low")
                print(f"{RED('B)')} {BLUE('Back')}\n")
                option = input('Option: ')
                print()
                if option.upper() == 'B':
                    clear_screen()
                    break
                elif option.upper() in ['H', 'L']:
                    level = MCP23S17.LEVEL_HIGH if option.upper() == 'H' else MCP23S17.LEVEL_LOW
                    if pin.upper() == 'A':
                        for i in range(16):
                            ioe.setLevel(i, level)
                    else:
                        ioe.setLevel(int(pin), level)

                else:
                    clear_screen()

        else:
            clear_screen()


def set_data_loop(ioe: MCP23S17):
    while True:
        print('*'*20 + PURPLE('Set Data') + 20*'*' + '\n')
        print('Enter the data to set on GPIOs:\n')
        print(f"{RED('{0-65535})')} Data")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        data = input('Option: ')
        print()
        if data.upper() == 'B':
            clear_screen()
            return
        elif data.isdecimal() and 0 <= int(data) <= 0xFFFF:
            ioe.setData(int(data))
        else:
            clear_screen()


def MCP23S17_Close():
    if not DEBUG:
        ioe1_cs.release()
        ioe2_cs.release()
        ioe1_reset.release()
        ioe2_reset.release()
        chip7.close()
        chip5.close()
        chip4.close()
        spi_ioe.close()


def main():
    ioe = ioe1

    while True:
        print(f"[{PURPLE(ioe.name)}] What Do You Want To Do?\n")
        print(f"{RED('1)')} Set Pullup Mode")
        print(f"{RED('2)')} Set Direction")
        print(f"{RED('3)')} Set Level")
        print(f"{RED('4)')} Get Level")
        print(f"{RED('5)')} Set Data")
        print(f"{RED('6)')} Get Data")
        print(f"{RED('7)')} Change IOE")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        option = input("Option: ")
        clear_screen()

        if option == '1':
            set_pullup_mode_loop(ioe)
        elif option == '2':
            set_direction_loop(ioe)
        elif option == '3':
            set_level_loop(ioe)
        elif option == '4':
            get_level_loop(ioe)
        elif option == '5':
            set_data_loop(ioe)
        elif option == '6':
            print('*'*20 + PURPLE('Get Data') + 20*'*' + '\n')
            print('Data:', GREEN(ioe.getData()), '\n')
        elif option == '7':
            ioe = select_ioe_loop()
        elif option.upper() == 'B':
            clear_screen()
            return
        else:
            clear_screen()



if not DEBUG:
    ioe1 = MCP23S17(spi_ioe, ioe1_cs, ioe1_reset, 'IOE 1')
    ioe2 = MCP23S17(spi_ioe, ioe2_cs, ioe2_reset, 'IOE 2')
else:
    ioe1 = MCP23S17(0, 0, 0, 'IOE 1')
    ioe2 = MCP23S17(0, 0, 0, 'IOE 2')

for my_ioe in [ioe1, ioe2]:
    for i in range(16):
        my_ioe.setDirection(i, MCP23S17.DIR_INPUT)

clear_screen()

