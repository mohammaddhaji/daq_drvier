#!/usr/bin/python

import math
from utils import *
import time

if not DEBUG:
    import spidev
    import gpiod

    chip5 = gpiod.Chip('gpiochip5')
    chip4 = gpiod.Chip('gpiochip3')

    dac_cs = chip5.get_line(12)
    dac_cs.request(consumer='LTC2668_DRIVER', type=gpiod.LINE_REQ_DIR_OUT)
    dac_cs.set_value(1)

    adc_cs = chip5.get_line(7)
    adc_cs.request(consumer='LTC2668_DRIVER', type=gpiod.LINE_REQ_DIR_OUT)
    adc_cs.set_value(1)

    adc_convst = chip4.get_line(31)
    adc_convst.request(consumer='LTC2668_DRIVER', type=gpiod.LINE_REQ_DIR_OUT)
    adc_convst.set_value(0)

    adc_busy = chip5.get_line(10)
    adc_busy.request(consumer='LTC2668_DRIVER', type=gpiod.LINE_REQ_DIR_IN)


    spi = spidev.SpiDev()
    spi.open(1, 0)
    spi.max_speed_hz = 4000000
    spi.mode = 0
    spi.bits_per_word = 8



# DAC Commands ------------------------------------------------------------

LTC2668_CMD_WRITE_N              = 0x00   # Write to input register n
LTC2668_CMD_UPDATE_N             = 0x10   # Update (power up) DAC register n
LTC2668_CMD_WRITE_N_UPDATE_ALL   = 0x20   # Write to input register n, update (power-up) all
LTC2668_CMD_WRITE_N_UPDATE_N     = 0x30   # Write to input register n, update (power-up) 
LTC2668_CMD_POWER_DOWN_N         = 0x40   # Power down n
LTC2668_CMD_POWER_DOWN_ALL       = 0x50   # Power down chip (all DAC's, MUX 
LTC2668_CMD_SPAN                 = 0x60   # Write span to dac n
LTC2668_CMD_CONFIG               = 0x70   # Configure reference / toggle
LTC2668_CMD_WRITE_ALL            = 0x80   # Write to all input registers
LTC2668_CMD_UPDATE_ALL           = 0x90   # Update all DACs
LTC2668_CMD_WRITE_ALL_UPDATE_ALL = 0xA0   # Write to all input reg, update all DACs
LTC2668_CMD_MUX                  = 0xB0   # Select MUX channel (controlled by 5 LSbs in data word)
LTC2668_CMD_TOGGLE_SEL           = 0xC0   # Select which DACs can be toggled (via toggle pin or global toggle bit)
LTC2668_CMD_GLOBAL_TOGGLE        = 0xD0   # Software toggle control via global toggle bit
LTC2668_CMD_SPAN_ALL             = 0xE0   # Set span for all DACs
LTC2668_CMD_NO_OPERATION         = 0xF0   # No operation

LTC2668_SPAN_0_TO_5V             = 0x0
LTC2668_SPAN_0_TO_10V            = 0x1
LTC2668_SPAN_PLUS_MINUS_5V       = 0x2
LTC2668_SPAN_PLUS_MINUS_10V      = 0x3
LTC2668_SPAN_PLUS_MINUS_2V5      = 0x4

LTC2668_REF_DISABLE              = 0x1   # Disable internal reference to save power when using an ext. ref.
LTC2668_THERMAL_SHUTDOWN         = 0x2

LTC2668_MUX_DISABLE              = 0x00  # Disable MUX
LTC2668_MUX_ENABLE               = 0x10  # Enable MUX, OR with MUX channel to be monitored

LTC2668_TOGGLE_REG_A             = 0x00  # Update DAC with register A
LTC2668_TOGGLE_REG_B             = 0x10  # Update DAC with register B

REF_EXTERNAL                     = LTC2668_REF_DISABLE  # External mode 
REF_INTERNAL                     = 0                    # Internal mode

#----------------------------------------------------------------------------

SOFTSPAN_RANGE = {
    LTC2668_SPAN_0_TO_5V: [0 , 5],
    LTC2668_SPAN_0_TO_10V: [0 , 10],
    LTC2668_SPAN_PLUS_MINUS_5V: [-5 , 5],
    LTC2668_SPAN_PLUS_MINUS_10V: [-10 , 10],
    LTC2668_SPAN_PLUS_MINUS_2V5: [-2.5 , 2.5]
}

SOFTSPAN_STR = {
    LTC2668_SPAN_0_TO_5V: '0V to 5V',
    LTC2668_SPAN_0_TO_10V: '0V to 10V',
    LTC2668_SPAN_PLUS_MINUS_5V: '-5V to 5V',
    LTC2668_SPAN_PLUS_MINUS_10V: '-10V to 10V',
    LTC2668_SPAN_PLUS_MINUS_2V5: '-2.5V to 2.5V',
}

CHANNELS_SOFTSPAN = {}
for ch in range(16):
    CHANNELS_SOFTSPAN[ch] = LTC2668_SPAN_PLUS_MINUS_10V

last_data_array = bytearray(4)


def LTC2668_voltage_to_code(dac_voltage, min_output, max_output):
    float_code = 65535.0 * (dac_voltage - min_output) / (max_output - min_output)
    if float_code > (math.floor(float_code) + 0.5):
        float_code = math.ceil(float_code) 
    else:
        float_code = math.floor(float_code)
    if float_code < 0.0: float_code = 0.0
    if float_code > 65535.0: float_code = 65535.0
    return int(float_code)


def LTC2668_code_to_voltage(dac_code, min_output, max_output):
    dac_voltage = ((dac_code / 65535.0) * (max_output - min_output)) + min_output
    return dac_voltage


def All_Channels_Same_Softspan():
    return len(set(list(CHANNELS_SOFTSPAN.values()))) == 1


def LTC2668_write(dac_command, dac_address, dac_code):
    global last_data_array
    data_array = bytearray(4)
    rx_array = bytearray(4)

    data_array[0] = 0
    data_array[1] = dac_command | dac_address
    data_array[2] = dac_code.to_bytes(2, 'little')[1]
    data_array[3] = dac_code.to_bytes(2, 'little')[0]

    if not DEBUG: 
        dac_cs.set_value(0)
        rx_array = spi.xfer(list(data_array))
        dac_cs.set_value(1)

    info = f"{RED('SENT:')} {format_packet(data_array)} --- "
    info += f"{RED('RECV:')} {format_packet(bytearray(rx_array))}\n"
    debug_print(info)

    ret =  (rx_array[2] == last_data_array[2]) and \
            (rx_array[1] == last_data_array[1]) and \
            (rx_array[0] == last_data_array[0])


    last_data_array[0] = data_array[0]
    last_data_array[1] = data_array[1]
    last_data_array[2] = data_array[2]

    return ret


def LTC2328_read_mux(dac):
    data_array = bytearray(4)
    rx_array = bytearray(4)

    if not DEBUG: 
        adc_convst.set_value(1)
        adc_convst.set_value(0)    

        while(adc_busy.get_value()): pass

        adc_cs.set_value(0)
        rx_array = spi.xfer(list(data_array))
        adc_cs.set_value(1)

    info = f"{RED('SENT:')} {format_packet(data_array)} --- "
    info += f"{RED('RECV:')} {format_packet(bytearray(rx_array))}\n"
    debug_print(info)

    voltage = (int.from_bytes(rx_array, 'big',signed=True) >> 14) * 0.000078125
    print(f'DAC {GREEN(dac)} feedback value: {GREEN(round(voltage, 4))}\n')

    return round(voltage, 4)



def set_softspan_range(softspan, dac):
    global CHANNELS_SOFTSPAN
    if dac.upper() == 'A':
        print(f"Setting softspan range {GREEN(SOFTSPAN_STR[softspan])} for {GREEN('all')} DACs.")
        LTC2668_write(LTC2668_CMD_SPAN_ALL, 0, softspan)
        for ch in range(16):
            CHANNELS_SOFTSPAN[ch] = softspan
    else:
        print(f"Setting softspan range {GREEN(SOFTSPAN_STR[softspan])} for DAC {GREEN(dac)}.")
        LTC2668_write(LTC2668_CMD_SPAN, int(dac), int(softspan))
        CHANNELS_SOFTSPAN[int(dac)] = softspan


def set_output_voltage(voltage, dac, softspan_range):
    code = LTC2668_voltage_to_code(voltage, softspan_range[0], softspan_range[1])
    adc_feedback = {}
    if dac.upper() == 'A':
        print(f"Setting output voltage {GREEN(voltage)} for {GREEN('all')} DACs.")
        LTC2668_write(LTC2668_CMD_WRITE_ALL_UPDATE_ALL, 0, code)
        for mux_dac in range(16):
            print('-'*60)
            debug_print(f"Setting mux to DAC {GREEN(mux_dac)}.")
            LTC2668_write(LTC2668_CMD_MUX, 0, mux_dac+16)
            time.sleep(0.005)
            debug_print('Reading from ADC.')
            adc_feedback[mux_dac] = LTC2328_read_mux(mux_dac)

    else:
        print(f"Setting output voltage {GREEN(voltage)} for DAC {GREEN(dac)}.")
        LTC2668_write(LTC2668_CMD_WRITE_N_UPDATE_N, int(dac), code)
        debug_print(f"Setting mux to DAC {GREEN(dac)}.")
        LTC2668_write(LTC2668_CMD_MUX, 0, int(dac)+16)
        time.sleep(0.005)
        debug_print('Reading from ADC')
        adc_feedback[dac] = LTC2328_read_mux(dac)

    return adc_feedback


def set_reference_voltage(ref):
    if ref == REF_INTERNAL:
        print(f'Setting reference voltage to {GREEN("Internal")}.')
        LTC2668_write(LTC2668_CMD_CONFIG, 0, REF_INTERNAL)
    else:
        print(f'Setting reference voltage to {GREEN("External")}.')
        LTC2668_write(LTC2668_CMD_CONFIG, 0, REF_EXTERNAL)


def update_power_up_dac(dac):
    if dac.upper() == 'A':
        print(f'Update power up {GREEN("all")} DACs.')
        LTC2668_write(LTC2668_CMD_UPDATE_ALL, 0, 0)
    else:
        print(f'Update power up DAC {GREEN(dac)}.')
        LTC2668_write(LTC2668_CMD_UPDATE_N, int(dac), 0)


def power_down_dac(dac):
    if dac.upper() == 'A':
        print(f'Power down {GREEN("all")} DACs.')
        LTC2668_write(LTC2668_CMD_POWER_DOWN_ALL, 0, 0)
    else:
        print(f'Power down DAC {GREEN(dac)}.')
        LTC2668_write(LTC2668_CMD_POWER_DOWN_N, int(dac), 0)


def softspan_event_loop():
    while True:
        print('*'*20 + PURPLE('Set Softspan Range') + 20*'*' + '\n')
        print('Choose DAC Channel:\n')
        print(f"{RED('{0-15})')} Single Channel")
        print(f"{RED('A)')} All")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        dac = input('Option: ')
        print()
        if dac.upper() == 'B':
            clear_screen()
            return
        elif dac.upper() in ['A'] + [str(i) for i in list(range(16))]:
            print('Choose Softspan Range:\n')
            print(f"{RED('0)')} 0V to 5V")
            print(f"{RED('1)')} 0V to 10V")
            print(f"{RED('2)')} -5V to 5V")
            print(f"{RED('3)')} -10V to 10V")
            print(f"{RED('4)')} -2.5V to 2.5V\n")
            softspan_range = input('Option: ')
            print()
            if softspan_range in ['0', '1', '2', '3', '4']:
                set_softspan_range(int(softspan_range), dac)
            else:
                clear_screen()

        else:
            clear_screen()


def refrence_voltage_event_loop():
    while True:
        print('*'*20 + PURPLE('Set Reference Voltage') + 20*'*' + '\n')
        print(f"{RED('I)')} Internal")
        print(f"{RED('E)')} External")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        ref = input('Option: ')
        print()
        if ref.upper() == 'B':
            clear_screen()
            return
        elif ref.upper() == 'I':
            set_reference_voltage(REF_INTERNAL)
        elif ref.upper() == 'E':
            set_reference_voltage(REF_EXTERNAL)
        else:
            clear_screen()


def output_voltage_event_loop():
    while True:
        print('*'*20 + PURPLE('Set Output Voltage') + 20*'*' + '\n')
        print('Choose DAC Channel:\n')
        print(f"{RED('{0-15})')} Single Channel")
        print(f"{RED('A)')} All")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        dac = input('Option: ')
        print()
        if dac.upper() == 'B':
            clear_screen()
            return
        elif dac.upper() in ['A'] + [str(i) for i in list(range(16))]:            
            # Check if all channels have same softspan
            if dac.upper() == 'A' and not All_Channels_Same_Softspan():
                print('Some channels have different sotfspan. Setting voltage is applicable for a single channel only.\n')
            else:
                dac_idx = 0 if dac.upper() == 'A' else int(dac)
                softspan_range_str = SOFTSPAN_STR[CHANNELS_SOFTSPAN[dac_idx]]
                voltage = input(f'Enter Output Voltage ({softspan_range_str}): ')
                print()
                softspan_range = SOFTSPAN_RANGE[CHANNELS_SOFTSPAN[dac_idx]]
                try:
                    voltage = float(voltage)
                    if softspan_range[0] <= float(voltage) <= softspan_range[1]:
                        set_output_voltage(voltage, dac, softspan_range)
                    else:
                        print(f'{voltage} is out of range ({softspan_range_str})\n')
                    
                except ValueError:
                    clear_screen()


        else:
            clear_screen()

def LTC2668_Close():
    power_down_dac('A')
    
    if not DEBUG:
        dac_cs.release()
        adc_cs.release()
        chip5.close()
        spi.close()
    

def main():
    while True:
        print(f"[{PURPLE('DAC')}] What Do You Want To Do?\n")
        print(f"{RED('1)')} Set Softspan Range")
        print(f"{RED('2)')} Get Softspan Range")
        print(f"{RED('3)')} Set Reference Voltage")
        print(f"{RED('4)')} Set Output Voltage")
        print(f"{RED('B)')} {BLUE('Back')}\n")
        option = input("Option: ").upper()
        clear_screen()
        if option in ['1', '2', '3', '4', 'B']:
            if option == '1':
                softspan_event_loop()
            elif option == '2':
                print('*'*20 + PURPLE('Channels Softspan') + 20*'*' + '\n')
                for ch in range(16):
                    print(f'Channel {GREEN(ch)}: {BLUE(SOFTSPAN_STR[CHANNELS_SOFTSPAN[ch]])}')
                print()
            elif option == '3':
                refrence_voltage_event_loop()
            elif option == '4':
                output_voltage_event_loop()
            else:
                clear_screen()
                return
        else:
            clear_screen()


clear_screen()
update_power_up_dac('A')
set_reference_voltage(REF_INTERNAL)
set_softspan_range(LTC2668_SPAN_PLUS_MINUS_10V, 'A')
clear_screen()

