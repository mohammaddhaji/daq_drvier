#!/usr/bin/python

from utils import *
import ad7616
import ltc2668
import mcp23s17
import uart


while True:
    print(f"Select Your Driver:\n")
    print(f"{RED('1)')} ADC")
    print(f"{RED('2)')} DAC")
    print(f"{RED('3)')} IOE")
    print(f"{RED('4)')} UART")
    print(f"{RED('Q)')} {BLUE('Quit')}\n")
    option = input("Option: ")
    clear_screen()
    if option == '1':
        ad7616.main()
    elif option == '2':
        ltc2668.main()
    elif option == '3':
        mcp23s17.main()
    elif option == '4':
        uart.main()
    elif option.upper() == 'Q':
        ad7616.AD7616_Close()
        ltc2668.LTC2668_Close()
        mcp23s17.MCP23S17_Close()
        uart.UART_Close()
        clear_screen()
        break
    else:
        clear_screen()