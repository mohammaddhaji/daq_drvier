#!/usr/bin/python

import ctypes
from utils import *

if not DEBUG:
    import spidev
    import gpiod

    chip6 = gpiod.Chip('gpiochip6')

    adc1_convst = chip6.get_line(7)
    adc1_convst.request(consumer='AD7616_DRIVER', type=gpiod.LINE_REQ_DIR_OUT)
    adc1_convst.set_value(0)

    adc1_reset = chip6.get_line(9)
    adc1_reset.request(consumer='AD7616_DRIVER', type=gpiod.LINE_REQ_DIR_OUT)
    adc1_reset.set_value(1)

    adc1_busy = chip6.get_line(8)
    adc1_busy.request(consumer='AD7616_DRIVER', type=gpiod.LINE_REQ_DIR_IN)


    chip4 = gpiod.Chip('gpiochip4')

    adc2_convst = chip4.get_line(17)
    adc2_convst.request(consumer='AD7616_DRIVER', type=gpiod.LINE_REQ_DIR_OUT)
    adc2_convst.set_value(0)

    adc2_reset = chip4.get_line(21)
    adc2_reset.request(consumer='AD7616_DRIVER', type=gpiod.LINE_REQ_DIR_OUT)
    adc2_reset.set_value(1)

    adc2_busy = chip4.get_line(25)
    adc2_busy.request(consumer='AD7616_DRIVER', type=gpiod.LINE_REQ_DIR_IN)


    spi_adc1 = spidev.SpiDev()
    spi_adc1.open(4, 0)
    spi_adc1.max_speed_hz = 4000000
    spi_adc1.mode = 3
    spi_adc1.bits_per_word = 8

    spi_adc2 = spidev.SpiDev()
    spi_adc2.open(3, 0)
    spi_adc2.max_speed_hz = 4000000
    spi_adc2.mode = 3
    spi_adc2.bits_per_word = 8



# Registers Adress------------------------------------------------------

AD7616_Addr_Config__Register = 0x2
AD7616_Addr_Channel_Register = 0x3
AD7616_Addr_RangeA1_Register = 0x4
AD7616_Addr_RangeA2_Register = 0x5
AD7616_Addr_RangeB1_Register = 0x6
AD7616_Addr_RangeB2_Register = 0x7


# Configuration Register ------------------------------------------------------

class Configuration(ctypes.LittleEndianStructure):
    _fields_ = [
        ("CRCEN", ctypes.c_uint16, 1),     # Enable CRC
        ("STATUSEN", ctypes.c_uint16, 1),  # Enable STATUS
        ("OverSamp", ctypes.c_uint16, 3),  # Over Sampling Ratio
        ("SEQEN", ctypes.c_uint16, 1),     # Enable Sequencer
        ("BURSTEN", ctypes.c_uint16, 1),   # Enable Burst
        ("SDEF", ctypes.c_uint16, 1),      # Self Detector Error Flag
        ("Rsvrd", ctypes.c_uint16, 1),     # Reserved = 0
        ("Address", ctypes.c_uint16, 6),   # Address of register => AD7616_Add_Config__Register
        ("W_nR", ctypes.c_uint16, 1)       # 1 for write and 0 for read
    ]

class Configuration_Reg(ctypes.Union):
    _fields_ = [
        ("all", ctypes.c_uint16),
        ("bit", Configuration)
    ]


# OverSampling Value ------------------------------------------------------

AD7616_OverSampling_Disabled  = 0x0
AD7616_OverSampling_2         = 0x1
AD7616_OverSampling_4         = 0x2
AD7616_OverSampling_8         = 0x3
AD7616_OverSampling_16        = 0x4
AD7616_OverSampling_32        = 0x5
AD7616_OverSampling_64        = 0x6
AD7616_OverSampling_128       = 0x7

OVER_SAMPLING_STR = {
    AD7616_OverSampling_Disabled: 'Disabled',
    AD7616_OverSampling_2:        '2',
    AD7616_OverSampling_4:        '4',
    AD7616_OverSampling_8:        '8',
    AD7616_OverSampling_16:       '16',
    AD7616_OverSampling_32:       '32',
    AD7616_OverSampling_64:       '64',
    AD7616_OverSampling_128:      '128'
}


# Channels Register ------------------------------------------------------

class Channels(ctypes.LittleEndianStructure):
    _fields_ = [
        ("ADC_A_SEL", ctypes.c_uint16, 4),
        ("ADC_B_SEL", ctypes.c_uint16, 4),
        ("Rsvrd", ctypes.c_uint16, 1),
        ("Address", ctypes.c_uint16, 6),
        ("W_nR", ctypes.c_uint16, 1),
    ]

class Channels_Reg(ctypes.Union):
    _fields_ = [
        ("all", ctypes.c_uint16),
        ("bit", Channels)
    ]


class ADC_Channels(ctypes.Structure):
    _fields_ = [
        ('RawAB', ctypes.c_int16 * 16),
        ('VA', ctypes.c_float * 8),
        ('VB', ctypes.c_float * 8)
    ]

# Channel Selection ------------------------------------------------------

AD7616_Channel_V0 = 0x0
AD7616_Channel_V1 = 0x1
AD7616_Channel_V2 = 0x2
AD7616_Channel_V3 = 0x3
AD7616_Channel_V4 = 0x4
AD7616_Channel_V5 = 0x5
AD7616_Channel_V6 = 0x6
AD7616_Channel_V7 = 0x7
AD7616_Channel_AVCC = 0x8
AD7616_Channel_ALDO = 0x9

CHANNEL_SEL_STR = {
    AD7616_Channel_V0: 'V0',
    AD7616_Channel_V1: 'V1',
    AD7616_Channel_V2: 'V2',
    AD7616_Channel_V3: 'V3',
    AD7616_Channel_V4: 'V4',
    AD7616_Channel_V5: 'V5',
    AD7616_Channel_V6: 'V6',
    AD7616_Channel_V7: 'V7',
    AD7616_Channel_AVCC: 'AVCC',
    AD7616_Channel_ALDO: 'ALDO',
}


# InputRange Register ------------------------------------------------------

class InputRangeA1(ctypes.LittleEndianStructure):
    _fields_ = [
        ("V0A_Range", ctypes.c_uint16, 2),
        ("V1A_Range", ctypes.c_uint16, 2),
        ("V2A_Range", ctypes.c_uint16, 2),
        ("V3A_Range", ctypes.c_uint16, 2),
        ("Rsvrd", ctypes.c_uint16, 1),
        ("Address", ctypes.c_uint16, 6),
        ("W_nR", ctypes.c_uint16, 1),
    ]

class InputRangeA1_Reg(ctypes.Union):
    _fields_ = [
        ("all", ctypes.c_uint16),
        ("bit", InputRangeA1)
    ]


class InputRangeA2(ctypes.LittleEndianStructure):
    _fields_ = [
        ("V4A_Range", ctypes.c_uint16, 2),
        ("V5A_Range", ctypes.c_uint16, 2),
        ("V6A_Range", ctypes.c_uint16, 2),
        ("V7A_Range", ctypes.c_uint16, 2),
        ("Rsvrd", ctypes.c_uint16, 1),
        ("Address", ctypes.c_uint16, 6),
        ("W_nR", ctypes.c_uint16, 1),
    ]

class InputRangeA2_Reg(ctypes.Union):
    _fields_ = [
        ("all", ctypes.c_uint16),
        ("bit", InputRangeA2)
    ]


class InputRangeB1(ctypes.LittleEndianStructure):
    _fields_ = [
        ("V0B_Range", ctypes.c_uint16, 2),
        ("V1B_Range", ctypes.c_uint16, 2),
        ("V2B_Range", ctypes.c_uint16, 2),
        ("V3B_Range", ctypes.c_uint16, 2),
        ("Rsvrd", ctypes.c_uint16, 1),
        ("Address", ctypes.c_uint16, 6),
        ("W_nR", ctypes.c_uint16, 1),
    ]

class InputRangeB1_Reg(ctypes.Union):
    _fields_ = [
        ("all", ctypes.c_uint16),
        ("bit", InputRangeB1)
    ]


class InputRangeB2(ctypes.LittleEndianStructure):
    _fields_ = [
        ("V4B_Range", ctypes.c_uint16, 2),
        ("V5B_Range", ctypes.c_uint16, 2),
        ("V6B_Range", ctypes.c_uint16, 2),
        ("V7B_Range", ctypes.c_uint16, 2),
        ("Rsvrd", ctypes.c_uint16, 1),
        ("Address", ctypes.c_uint16, 6),
        ("W_nR", ctypes.c_uint16, 1),
    ]

class InputRangeB2_Reg(ctypes.Union):
    _fields_ = [
        ("all", ctypes.c_uint16),
        ("bit", InputRangeB2)
    ]


AD7616_InputRange_2V5_PN = 0x1
AD7616_InputRange_5V0_PN = 0x2
AD7616_InputRange_10V_PN = 0x3

InputRange_STR = {
    AD7616_InputRange_2V5_PN: "+/- 2.5V",
    AD7616_InputRange_5V0_PN: "+/- 5V",
    AD7616_InputRange_10V_PN: "+/- 10V",
}

InputRange_VAL = {
    AD7616_InputRange_2V5_PN: 5,
    AD7616_InputRange_5V0_PN: 10,
    AD7616_InputRange_10V_PN: 20,
}


# ADC ---------------------------------------------------------------

class ADC:
    def __init__(self, spi, convst, busy, name):
        self.spi = spi
        self.convst = convst
        self.busy = busy
        self.byte_order = 'big'
        self.name = name

        self.config = Configuration_Reg()
        self.config.bit.CRCEN    = 0
        self.config.bit.STATUSEN = 0
        self.config.bit.SEQEN    = 0    
        self.config.bit.BURSTEN  = 0
        self.config.bit.SDEF     = 0
        self.config.bit.Rsvrd    = 0
        self.config.bit.Address  = AD7616_Addr_Config__Register

        self.channels = Channels_Reg()
        self.InRangeA1 = InputRangeA1_Reg()
        self.InRangeA2 = InputRangeA2_Reg()
        self.InRangeB1 = InputRangeB1_Reg()
        self.InRangeB2 = InputRangeB2_Reg()

        self.InRangeA1.bit.Rsvrd   = 0
        self.InRangeA1.bit.Address = AD7616_Addr_RangeA1_Register
        self.InRangeA2.bit.Rsvrd   = 0
        self.InRangeA2.bit.Address = AD7616_Addr_RangeA2_Register
        self.InRangeB1.bit.Rsvrd   = 0
        self.InRangeB1.bit.Address = AD7616_Addr_RangeB1_Register
        self.InRangeB2.bit.Rsvrd   = 0
        self.InRangeB2.bit.Address = AD7616_Addr_RangeB2_Register

    def set_over_sampling(self, over_sampling):
        self.config.bit.OverSamp = over_sampling
        self.config.bit.W_nR     = 1
        print(f'Setting oversampling ratio to {GREEN(OVER_SAMPLING_STR[over_sampling])}.')
        self._spi_write(list(self.config.all.to_bytes(2, self.byte_order)))

    def get_over_sampling(self):
        return self.config.bit.OverSamp

    def set_input_range_all(self, in_range):
        self.InRangeA1.bit.V0A_Range = in_range
        self.InRangeA1.bit.V1A_Range = in_range
        self.InRangeA1.bit.V2A_Range = in_range
        self.InRangeA1.bit.V3A_Range = in_range
        self.InRangeA1.bit.W_nR      = 1

        self.InRangeA2.bit.V4A_Range = in_range
        self.InRangeA2.bit.V5A_Range = in_range
        self.InRangeA2.bit.V6A_Range = in_range
        self.InRangeA2.bit.V7A_Range = in_range
        self.InRangeA2.bit.W_nR      = 1

        self.InRangeB1.bit.V0B_Range = in_range
        self.InRangeB1.bit.V1B_Range = in_range
        self.InRangeB1.bit.V2B_Range = in_range
        self.InRangeB1.bit.V3B_Range = in_range
        self.InRangeB1.bit.W_nR      = 1

        self.InRangeB2.bit.V4B_Range = in_range
        self.InRangeB2.bit.V5B_Range = in_range
        self.InRangeB2.bit.V6B_Range = in_range
        self.InRangeB2.bit.V7B_Range = in_range
        self.InRangeB2.bit.W_nR      = 1

        print(f"Setting input range {GREEN(InputRange_STR[in_range])} for {GREEN('all')}.")
        self._spi_write(list(self.InRangeA1.all.to_bytes(2, self.byte_order)))
        self._spi_write(list(self.InRangeA2.all.to_bytes(2, self.byte_order)))
        self._spi_write(list(self.InRangeB1.all.to_bytes(2, self.byte_order)))
        self._spi_write(list(self.InRangeB2.all.to_bytes(2, self.byte_order)))

    def set_input_range_AV0_7(self, in_range):
        self.InRangeA1.bit.V0A_Range = in_range
        self.InRangeA1.bit.V1A_Range = in_range
        self.InRangeA1.bit.V2A_Range = in_range
        self.InRangeA1.bit.V3A_Range = in_range
        self.InRangeA1.bit.W_nR      = 1

        self.InRangeA2.bit.V4A_Range = in_range
        self.InRangeA2.bit.V5A_Range = in_range
        self.InRangeA2.bit.V6A_Range = in_range
        self.InRangeA2.bit.V7A_Range = in_range
        self.InRangeA2.bit.W_nR      = 1

        print(f"Setting input range {GREEN(InputRange_STR[in_range])} for {GREEN('AV0_7')}.")
        self._spi_write(list(self.InRangeA1.all.to_bytes(2, self.byte_order)))
        self._spi_write(list(self.InRangeA2.all.to_bytes(2, self.byte_order)))

    def set_input_range_BV0_7(self, in_range):
        self.InRangeB1.bit.V0B_Range = in_range
        self.InRangeB1.bit.V1B_Range = in_range
        self.InRangeB1.bit.V2B_Range = in_range
        self.InRangeB1.bit.V3B_Range = in_range
        self.InRangeB1.bit.W_nR      = 1

        self.InRangeB2.bit.V4B_Range = in_range
        self.InRangeB2.bit.V5B_Range = in_range
        self.InRangeB2.bit.V6B_Range = in_range
        self.InRangeB2.bit.V7B_Range = in_range
        self.InRangeB2.bit.W_nR      = 1

        print(f"Setting input range {GREEN(InputRange_STR[in_range])} for {GREEN('BV0_7')}.")
        self._spi_write(list(self.InRangeB1.all.to_bytes(2, self.byte_order)))
        self._spi_write(list(self.InRangeB2.all.to_bytes(2, self.byte_order)))

    def set_input_range(self, channel, in_range):
        print(f"Setting input range {GREEN(InputRange_STR[in_range])} for {GREEN(channel)}")
        if channel == 'V0A':
            self.InRangeA1.bit.V0A_Range = in_range
            self.InRangeA1.bit.W_nR      = 1
            self._spi_write(list(self.InRangeA1.all.to_bytes(2, self.byte_order)))
        elif channel == 'V1A':
            self.InRangeA1.bit.V1A_Range = in_range
            self.InRangeA1.bit.W_nR      = 1
            self._spi_write(list(self.InRangeA1.all.to_bytes(2, self.byte_order)))
        elif channel == 'V2A':
            self.InRangeA1.bit.V2A_Range = in_range
            self.InRangeA1.bit.W_nR      = 1
            self._spi_write(list(self.InRangeA1.all.to_bytes(2, self.byte_order)))
        elif channel == 'V3A':
            self.InRangeA1.bit.V3A_Range = in_range
            self.InRangeA1.bit.W_nR      = 1
            self._spi_write(list(self.InRangeA1.all.to_bytes(2, self.byte_order)))
        elif channel == 'V4A':
            self.InRangeA2.bit.V4A_Range = in_range
            self.InRangeA2.bit.W_nR      = 1
            self._spi_write(list(self.InRangeA2.all.to_bytes(2, self.byte_order)))
        elif channel == 'V5A':
            self.InRangeA2.bit.V5A_Range = in_range
            self.InRangeA2.bit.W_nR      = 1
            self._spi_write(list(self.InRangeA2.all.to_bytes(2, self.byte_order)))
        elif channel == 'V6A':
            self.InRangeA2.bit.V6A_Range = in_range
            self.InRangeA2.bit.W_nR      = 1
            self._spi_write(list(self.InRangeA2.all.to_bytes(2, self.byte_order)))
        elif channel == 'V7A':
            self.InRangeA2.bit.V7A_Range = in_range
            self.InRangeA2.bit.W_nR      = 1
            self._spi_write(list(self.InRangeA2.all.to_bytes(2, self.byte_order)))
        elif channel == 'V0B':
            self.InRangeB1.bit.V0B_Range = in_range
            self.InRangeB1.bit.W_nR      = 1
            self._spi_write(list(self.InRangeB1.all.to_bytes(2, self.byte_order)))
        elif channel == 'V1B':
            self.InRangeB1.bit.V1B_Range = in_range
            self.InRangeB1.bit.W_nR      = 1
            self._spi_write(list(self.InRangeB1.all.to_bytes(2, self.byte_order)))
        elif channel == 'V2B':
            self.InRangeB1.bit.V2B_Range = in_range
            self.InRangeB1.bit.W_nR      = 1
            self._spi_write(list(self.InRangeB1.all.to_bytes(2, self.byte_order)))
        elif channel == 'V3B':
            self.InRangeB1.bit.V3B_Range = in_range
            self.InRangeB1.bit.W_nR      = 1
            self._spi_write(list(self.InRangeB1.all.to_bytes(2, self.byte_order)))
        elif channel == 'V4B':
            self.InRangeB2.bit.V4B_Range = in_range
            self.InRangeB2.bit.W_nR      = 1
            self._spi_write(list(self.InRangeB2.all.to_bytes(2, self.byte_order)))
        elif channel == 'V5B':
            self.InRangeB2.bit.V5B_Range = in_range
            self.InRangeB2.bit.W_nR      = 1
            self._spi_write(list(self.InRangeB2.all.to_bytes(2, self.byte_order)))
        elif channel == 'V6B':
            self.InRangeB2.bit.V6B_Range = in_range
            self.InRangeB2.bit.W_nR      = 1
            self._spi_write(list(self.InRangeB2.all.to_bytes(2, self.byte_order)))
        elif channel == 'V7B':
            self.InRangeB2.bit.V7B_Range = in_range
            self.InRangeB2.bit.W_nR      = 1
            self._spi_write(list(self.InRangeB2.all.to_bytes(2, self.byte_order)))

    def get_input_range(self, channel):
        if channel == 'V0A': return self.InRangeA1.bit.V0A_Range
        elif channel == 'V1A': return self.InRangeA1.bit.V1A_Range
        elif channel == 'V2A': return self.InRangeA1.bit.V2A_Range
        elif channel == 'V3A': return self.InRangeA1.bit.V3A_Range
        elif channel == 'V4A': return self.InRangeA2.bit.V4A_Range
        elif channel == 'V5A': return self.InRangeA2.bit.V5A_Range
        elif channel == 'V6A': return self.InRangeA2.bit.V6A_Range
        elif channel == 'V7A': return self.InRangeA2.bit.V7A_Range
        elif channel == 'V0B': return self.InRangeB1.bit.V0B_Range
        elif channel == 'V1B': return self.InRangeB1.bit.V1B_Range
        elif channel == 'V2B': return self.InRangeB1.bit.V2B_Range
        elif channel == 'V3B': return self.InRangeB1.bit.V3B_Range
        elif channel == 'V4B': return self.InRangeB2.bit.V4B_Range
        elif channel == 'V5B': return self.InRangeB2.bit.V5B_Range
        elif channel == 'V6B': return self.InRangeB2.bit.V6B_Range
        elif channel == 'V7B': return self.InRangeB2.bit.V7B_Range

    def get_all_input_range(self):
        channels = [
            'V0A', 'V1A', 'V2A', 'V3A',
            'V4A', 'V5A', 'V6A', 'V7A',
            'V0B', 'V1B', 'V2B', 'V3B',
            'V4B', 'V5B', 'V6B', 'V7B',
        ]
        info = ''
        for ch in channels:
            info += f"{GREEN(ch)}: {BLUE(InputRange_STR[self.get_input_range(ch)])}\n"
            
        return info + '\n'

    def _set_channel(self, A_channel, B_channel):
        self.channels.bit.ADC_A_SEL = A_channel & 0xF
        self.channels.bit.ADC_B_SEL = B_channel & 0xF
        self.channels.bit.Rsvrd     = 0
        self.channels.bit.Address   = AD7616_Addr_Channel_Register
        self.channels.bit.W_nR      = 1

        info = f'Setting channel A: {GREEN(CHANNEL_SEL_STR[A_channel])}, '
        info += f'channel B: {GREEN(CHANNEL_SEL_STR[B_channel])}'
        debug_print(info)
        self._spi_write(list(self.channels.all.to_bytes(2, self.byte_order)))

    def read_channel(self, A_channel, B_channel):
        print('-'*60)
        self._set_channel(A_channel, B_channel)

        for i in range(2):
            if not DEBUG:
                self.convst.set_value(1)
                self.convst.set_value(0)
                while(self.busy.get_value()): pass

            if i == 0:
                info = f'Reading channel A: {GREEN(CHANNEL_SEL_STR[A_channel])}, '
                info += f'channel B: {GREEN(CHANNEL_SEL_STR[B_channel])}'
                debug_print(info)

            result = self._spi_write(bytearray(6))

        voltage_A = int.from_bytes(bytearray(result)[:2], self.byte_order, signed=True)
        input_range = InputRange_VAL[self.get_input_range(f"V{A_channel}A")]
        voltage_A *= input_range / 2**16
        voltage_B = int.from_bytes(bytearray(result)[2:4], self.byte_order, signed=True)
        input_range = InputRange_VAL[self.get_input_range(f"V{B_channel}B")]
        voltage_B *= input_range / 2**16
        print(f"Voltage of {GREEN(CHANNEL_SEL_STR[A_channel] + 'A')} is {BLUE(round(voltage_A, 3))}")
        print(f"Voltage of {GREEN(CHANNEL_SEL_STR[B_channel] + 'B')} is {BLUE(round(voltage_B, 3))}\n")
        return round(voltage_A, 3), round(voltage_B, 3)

    def _spi_write(self, packet):
        packet_copy = packet.copy()

        if not DEBUG:
            received = self.spi.xfer(packet)
        else:
            received = [0x00 for _ in range(len(packet_copy))]

        info = f"{RED('SENT:')} {format_packet(bytearray(packet_copy))} --- "
        info += f"{RED('RECV:')} {format_packet(bytearray(received))}\n"
        debug_print(info)
        return received



def select_adc_loop():
    global adc1, adc2
    while True:
        print('*'*20 + PURPLE('Select ADC') + 20*'*' + '\n')
        print('Select ADC:\n')
        print(f"{RED('1)')} ADC 1")
        print(f"{RED('2)')} ADC 2\n")
        adc = input('Option: ')
        print()
        clear_screen()
        if adc == '1':
            return adc1
        elif adc == '2':
            return adc2
    

def set_input_range_loop(adc: ADC):
    def print_ranges():
        print(f"{RED('1)')} +/- 2.5V")
        print(f"{RED('2)')} +/- 5V")
        print(f"{RED('3)')} +/- 10V")
        print(f"{RED('B)')} {BLUE('Back')}\n")

    while True:
        print('*'*20 + PURPLE('Set Input Range') + 20*'*' + '\n')
        print('Choose ADC Channel:\n')
        print(f"{RED('1)')} All")
        print(f"{RED('2)')} ADC A V0_7")
        print(f"{RED('3)')} ADC B V0_7")
        print(f"{RED('V{0-7}{A,B})')} Single Channel (e.g. V3B)")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        option = input('Option: ')
        print()
        clear_screen()
        if option.upper() == 'B':
            return
        elif option == '1':
            while True:
                print('Set Input Range For All:\n')
                print_ranges()
                in_range = input('Option: ')
                print()
                if in_range.upper() == 'B':
                    clear_screen()
                    break
                elif in_range == '1':
                    adc.set_input_range_all(AD7616_InputRange_2V5_PN)
                    break
                elif in_range == '2':
                    adc.set_input_range_all(AD7616_InputRange_5V0_PN)
                    break
                elif in_range == '3':
                    adc.set_input_range_all(AD7616_InputRange_10V_PN)
                    break
                else:
                    clear_screen()


        elif option == '2':
            while True:
                print('Set Input Range For ADC A V0_7:\n')
                print_ranges()
                in_range = input('Option: ')
                print()
                if in_range.upper() == 'B':
                    clear_screen()
                    break
                elif in_range == '1':
                    adc.set_input_range_AV0_7(AD7616_InputRange_2V5_PN)
                    break
                elif in_range == '2':
                    adc.set_input_range_AV0_7(AD7616_InputRange_5V0_PN)
                    break
                elif in_range == '3':
                    adc.set_input_range_AV0_7(AD7616_InputRange_10V_PN)
                    break
                else:
                    clear_screen()


        elif option == '3':
            while True:
                print('Set Input Range For ADC B V0_7:\n')
                print_ranges()
                in_range = input('Option: ')
                print()
                if in_range.upper() == 'B':
                    clear_screen()
                    break
                elif in_range == '1':
                    adc.set_input_range_BV0_7(AD7616_InputRange_2V5_PN)
                    break
                elif in_range == '2':
                    adc.set_input_range_BV0_7(AD7616_InputRange_5V0_PN)
                    break
                elif in_range == '3':
                    adc.set_input_range_BV0_7(AD7616_InputRange_10V_PN)
                    break
                else:
                    clear_screen()


        elif option.upper() in [f'V{i}A' for i in range(8)] + [f'V{i}B' for i in range(8)]:
            option = option.upper()
            while True:
                print(f'Set Input Range For {option}:\n')
                print_ranges()
                in_range = input('Option: ')
                print()
                if in_range.upper() == 'B':
                    clear_screen()
                    break
                elif in_range == '1':
                    adc.set_input_range(option, AD7616_InputRange_2V5_PN)
                    break
                elif in_range == '2':
                    adc.set_input_range(option, AD7616_InputRange_5V0_PN)
                    break
                elif in_range == '3':
                    adc.set_input_range(option, AD7616_InputRange_10V_PN)
                    break
                else:
                    clear_screen()


def set_oversampling_ration_loop(adc: ADC):
    while True:
        print('*'*20 + PURPLE('Set Oversampling Ratio') + 20*'*' + '\n')
        print('Select Overampling Ratio:\n')
        print(f"{RED('1)')} Disable        {RED('5)')} 16")
        print(f"{RED('2)')} 2              {RED('6)')} 32")
        print(f"{RED('3)')} 4              {RED('7)')} 64")
        print(f"{RED('4)')} 8              {RED('8)')} 128")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        option = input('Option: ')
        print()
        if option.upper() == 'B':
            clear_screen()
            return

        elif option == '1':
            adc.set_over_sampling(AD7616_OverSampling_Disabled)
        elif option == '2':
            adc.set_over_sampling(AD7616_OverSampling_2)
        elif option == '3':
            adc.set_over_sampling(AD7616_OverSampling_4)
        elif option == '4':
            adc.set_over_sampling(AD7616_OverSampling_8)
        elif option == '5':
            adc.set_over_sampling(AD7616_OverSampling_16)
        elif option == '6':
            adc.set_over_sampling(AD7616_OverSampling_32)
        elif option == '7':
            adc.set_over_sampling(AD7616_OverSampling_64)
        elif option == '8':
            adc.set_over_sampling(AD7616_OverSampling_128)
        else:
            clear_screen()


def read_channel_loop(adc: ADC):
    def print_option(ch):
        print(f"{RED('0)')} V0{ch}        {RED('4)')} V4{ch}")
        print(f"{RED('1)')} V1{ch}        {RED('5)')} V5{ch}")
        print(f"{RED('2)')} V2{ch}        {RED('6)')} V6{ch}")
        print(f"{RED('3)')} V3{ch}        {RED('7)')} V7{ch}")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        

    while True:
        print('*'*20 + PURPLE('Read Channels') + 20*'*' + '\n')
        print('Choose Channels To Read:\n')
        print(f"{RED('1)')} All")
        print(f"{RED('2)')} Set Channels A & B")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        option = input('Option: ')
        print()
        if option.upper() == 'B':
            clear_screen()
            return
        elif option.upper() == '1':
            for chl in range(8):
                adc.read_channel(chl, chl)
        elif option.upper() == '2':
            clear_screen()
            while True:
                print('Set Channel A:\n')
                print_option('A')
                option = input('Option: ')
                print()
                if option.upper() == 'B':
                    clear_screen()
                    break
                elif option in [str(i) for i in range(8)]:
                    A_channel = int(option)
                    print('Set Channel B:\n')
                    print_option('B')
                    option = input('Option: ')
                    print()
                    if option.upper() == 'B':
                        clear_screen()
                        break
                    elif option in [str(i) for i in range(8)]:
                        B_channel = int(option)
                        adc.read_channel(A_channel, B_channel)
                    else:
                        clear_screen()
                else:
                    clear_screen()
        else:
            clear_screen()
            
            
def AD7616_Close():
    if not DEBUG:
        adc1_convst.release()
        adc1_reset.release()
        adc1_busy.release()
        adc2_convst.release()
        adc2_reset.release()
        adc2_busy.release()
        chip6.close()
        chip4.close()
        spi_adc1.close()
        spi_adc2.close()


def main():
    adc = adc1

    while True:
        print(f"[{PURPLE(adc.name)}] What Do You Want To Do?\n")
        print(f"{RED('1)')} Set Inputs Range")
        print(f"{RED('2)')} Get Inputs Range")
        print(f"{RED('3)')} Set Oversampling Ratio")
        print(f"{RED('4)')} Get Oversampling Ratio")
        print(f"{RED('5)')} Read Channels")
        print(f"{RED('6)')} Change ADC")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        option = input("Option: ")
        clear_screen()
        if option == '1':
            set_input_range_loop(adc)
        elif option == '2':
            print('*'*20 + PURPLE('Get Input Range') + 20*'*' + '\n')
            print(adc.get_all_input_range())
        elif option == '3':
            set_oversampling_ration_loop(adc)
        elif option == '4':
            print('*'*20 + PURPLE('Get Oversampling Ratio') + 20*'*' + '\n')
            print(f"Oversampling ratio is {GREEN(adc.get_over_sampling())}.\n")
        elif option == '5':
            read_channel_loop(adc)
        elif option == '6':
            adc = select_adc_loop()
        elif option.upper() == 'B':
            clear_screen()
            return
        else:
            clear_screen()



if not DEBUG:
    adc1 = ADC(spi_adc1, adc1_convst, adc1_busy, 'ADC 1')
    adc2 = ADC(spi_adc2, adc2_convst, adc2_busy, 'ADC 2')
else:
    adc1 = ADC(None, None, None, 'ADC 1')
    adc2 = ADC(None, None, None, 'ADC 2')

print(f'[{PURPLE(adc1.name)}]')
adc1.set_input_range_all(AD7616_InputRange_10V_PN)
adc1.set_over_sampling(AD7616_OverSampling_Disabled)
print(f'[{PURPLE(adc2.name)}]')
adc2.set_input_range_all(AD7616_InputRange_10V_PN)
adc2.set_over_sampling(AD7616_OverSampling_Disabled)

clear_screen()

    
