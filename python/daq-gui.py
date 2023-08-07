#!/usr/bin/python

import os
import sys
sys.stdout = open(os.devnull, 'w')

from PyQt5.QtWidgets import (
    QDesktopWidget,
    QApplication, 
    QMainWindow, 
    QPushButton,
    QShortcut,
)
from PyQt5.QtGui import QKeySequence 
from PyQt5.QtCore import QTimer
from PyQt5 import uic

import utils
import ad7616
import ltc2668
import mcp23s17
import uart


CURRENT_FILE_DIR = os.path.dirname(os.path.abspath(__file__))
MAIN_UI = os.path.join(CURRENT_FILE_DIR, 'ui', 'main-2.ui')
os.chdir(CURRENT_FILE_DIR)


class MainWindow(QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        uic.loadUi(MAIN_UI, self)
        
        self.shortcut = QShortcut(QKeySequence("Shift+E"), self)
        self.shortcut.activated.connect(self.close)
        self.btn_close.clicked.connect(self.close)

        self.btn_adc.clicked.connect(lambda:self.set_page('adc'))
        self.btn_dac.clicked.connect(lambda:self.set_page('dac'))
        self.btn_ioe.clicked.connect(lambda:self.set_page('ioe'))
        self.btn_uart.clicked.connect(lambda:self.set_page('uart'))

        self.stackedWidget.setCurrentWidget(self.adc_page)

        self.timer = QTimer()
        self.timer.setInterval(100)
        self.timer.timeout.connect(self.read_adc)
        self.timer.timeout.connect(self.get_pin_value)
        self.timer.start()

        ##############################################################################
        ################################# ADC AD7616 #################################
        ##############################################################################

        self.btn_adc_single_channel.clicked.connect(lambda:self.set_adc_channel_mode('single'))
        self.btn_adc_all_channels.clicked.connect(lambda:self.set_adc_channel_mode('all'))
        self.btn_adc_all_channels.setStyleSheet(utils.SELECTED_STYLE)
        self.adc_channel_mode = 'all'

        self.btn_input_range_10.clicked.connect(lambda:self.set_channel_input_range(ad7616.AD7616_InputRange_10V_PN))
        self.btn_input_range_5.clicked.connect(lambda:self.set_channel_input_range(ad7616.AD7616_InputRange_5V0_PN))
        self.btn_input_range_2v5.clicked.connect(lambda:self.set_channel_input_range(ad7616.AD7616_InputRange_2V5_PN))

        self.btn_os_disabled.clicked.connect(lambda:self.set_oversampling_ratio(ad7616.AD7616_OverSampling_Disabled))
        self.btn_os_2.clicked.connect(lambda:self.set_oversampling_ratio(ad7616.AD7616_OverSampling_2))
        self.btn_os_4.clicked.connect(lambda:self.set_oversampling_ratio(ad7616.AD7616_OverSampling_4))
        self.btn_os_8.clicked.connect(lambda:self.set_oversampling_ratio(ad7616.AD7616_OverSampling_8))
        self.btn_os_16.clicked.connect(lambda:self.set_oversampling_ratio(ad7616.AD7616_OverSampling_16))
        self.btn_os_32.clicked.connect(lambda:self.set_oversampling_ratio(ad7616.AD7616_OverSampling_32))
        self.btn_os_64.clicked.connect(lambda:self.set_oversampling_ratio(ad7616.AD7616_OverSampling_64))
        self.btn_os_128.clicked.connect(lambda:self.set_oversampling_ratio(ad7616.AD7616_OverSampling_128))

        self.btn_adc1.clicked.connect(lambda:self.set_current_adc(1))
        self.btn_adc2.clicked.connect(lambda:self.set_current_adc(2))
        self.btn_adc1.setStyleSheet(utils.SELECTED_STYLE)
        self.current_adc = ad7616.adc1
        self.current_adc_channel = 'V0A'

        for btn in self.adc_channels_layout.findChildren(QPushButton):
            btn.clicked.connect(self.adc_channel_selected(btn))
        
        self.prev_adc_btn_channel = self.btn_V0A

        self.get_channel_input_range()
        self.get_oversampling_ratio()
        self.read_adc()

        ##############################################################################
        ################################# DAC LTC2668 ################################
        ##############################################################################

        for btn in self.dac_channels_layout.findChildren(QPushButton):
            btn.clicked.connect(self.dac_channel_selected(btn))

        self.prev_dac_btn_channel = self.btn_chl_0
        self.current_dac_channel = '0'

        self.btn_span_0_to_5.clicked.connect(lambda:self.set_channel_softspan(ltc2668.LTC2668_SPAN_0_TO_5V))
        self.btn_span_0_to_10.clicked.connect(lambda:self.set_channel_softspan(ltc2668.LTC2668_SPAN_0_TO_10V))
        self.btn_span_plus_minus_2v5.clicked.connect(lambda:self.set_channel_softspan(ltc2668.LTC2668_SPAN_PLUS_MINUS_2V5))
        self.btn_span_plus_minus_5.clicked.connect(lambda:self.set_channel_softspan(ltc2668.LTC2668_SPAN_PLUS_MINUS_5V))
        self.btn_span_plus_minus_10.clicked.connect(lambda:self.set_channel_softspan(ltc2668.LTC2668_SPAN_PLUS_MINUS_10V))

        self.btn_dac_single_channel.clicked.connect(lambda:self.set_dac_channel_mode('single'))
        self.btn_dac_all_channels.clicked.connect(lambda:self.set_dac_channel_mode('all'))
        self.btn_dac_all_channels.setStyleSheet(utils.SELECTED_STYLE)
        self.dac_channel_mode = 'all'

        self.btn_vref_internal.clicked.connect(lambda:self.set_reference_voltage(ltc2668.REF_INTERNAL))
        self.btn_vref_external.clicked.connect(lambda:self.set_reference_voltage(ltc2668.REF_EXTERNAL))
        self.btn_vref_internal.animateClick()

        self.btn_apply.clicked.connect(self.set_output_voltage)
        self.btn_dec.clicked.connect(lambda:self.set_voltage('dec'))
        self.btn_inc.clicked.connect(lambda:self.set_voltage('inc'))

        self.incTimer = QTimer()
        self.decTimer = QTimer()
        self.incTimer.timeout.connect(lambda:self.set_voltage('inc'))
        self.decTimer.timeout.connect(lambda:self.set_voltage('dec'))
        self.btn_inc.pressed.connect(lambda:self.incTimer.start(50))
        self.btn_inc.released.connect(lambda:self.incTimer.stop())
        self.btn_dec.pressed.connect(lambda:self.decTimer.start(50))
        self.btn_dec.released.connect(lambda:self.decTimer.stop())

        self.get_channel_softspan()
        self.btn_apply.animateClick()

        ##############################################################################
        ################################ IOE MCP26S17 ################################
        ##############################################################################

        for btn in self.ioe_pins_layout.findChildren(QPushButton):
            btn.clicked.connect(self.ioe_pin_selected(btn))

        self.btn_ioe1.clicked.connect(lambda:self.set_current_ioe(1))
        self.btn_ioe2.clicked.connect(lambda:self.set_current_ioe(2))
        self.btn_ioe1.setStyleSheet(utils.SELECTED_STYLE)
        self.current_ioe = mcp23s17.ioe1
        self.prev_ioe_btn_pin = self.btn_pin_0
        self.current_ioe_pin = 0

        self.btn_ioe_single_pin.clicked.connect(lambda:self.set_ioe_pin_mode('single'))
        self.btn_ioe_all_pins.clicked.connect(lambda:self.set_ioe_pin_mode('all'))
        self.btn_ioe_all_pins.setStyleSheet(utils.SELECTED_STYLE)
        self.ioe_pin_mode = 'all'

        self.btn_low.clicked.connect(lambda:self.set_pin_level(mcp23s17.MCP23S17.LEVEL_LOW))
        self.btn_high.clicked.connect(lambda:self.set_pin_level(mcp23s17.MCP23S17.LEVEL_HIGH))
        self.btn_input.clicked.connect(lambda:self.set_pin_direction(mcp23s17.MCP23S17.DIR_INPUT))
        self.btn_output.clicked.connect(lambda:self.set_pin_direction(mcp23s17.MCP23S17.DIR_OUTPUT))
        self.btn_pullup_disabled.clicked.connect(lambda:self.set_pin_pullup(mcp23s17.MCP23S17.PULLUP_DISABLED))
        self.btn_pullup_enabled.clicked.connect(lambda:self.set_pin_pullup(mcp23s17.MCP23S17.PULLUP_ENABLED))

        self.get_pin_value()
        self.get_pin_direction()
        self.get_pin_pullup()

        ##############################################################################
        #################################### UART ####################################
        ##############################################################################

        self.btn_uart_send.clicked.connect(self.uart_write)
        self.btn_uart_continuous.clicked.connect(self.on_uart_continuous_clicked)
        self.uart_write_timer = QTimer()
        self.uart_write_timer.setInterval(10)
        self.uart_write_timer.timeout.connect(self.uart_write)

        self.uart_read_timer = QTimer()
        self.uart_read_timer.setInterval(10)
        self.uart_read_timer.timeout.connect(self.uart_read)
        self.uart_read_timer.start()

        self.btn_clear_input.clicked.connect(lambda:self.clear_buffer('input'))
        self.btn_clear_output.clicked.connect(lambda:self.clear_buffer('output'))
        
        for port in uart.UARTS:
            self.cmb_uarts.addItem(f"{port['name']} {port['mode']}")

        self.cmb_uarts.currentIndexChanged.connect(self.on_uart_changed)

        
##########################################################################################
####################################### ADC AD7616 #######################################
##########################################################################################

    def set_adc_channel_mode(self, mode):

        self.adc_channel_mode = mode

        if mode == 'all':
            self.btn_adc_single_channel.setStyleSheet('')
            self.btn_adc_all_channels.setStyleSheet(utils.SELECTED_STYLE)

            for btn in self.adc_channels_layout.findChildren(QPushButton):
                btn.setStyleSheet('')

            x = self.current_adc.get_input_range('V0A')
            for btn in self.adc_channels_layout.findChildren(QPushButton):
                if x != self.current_adc.get_input_range(btn.text()):
                    self.set_channel_input_range(ad7616.AD7616_InputRange_10V_PN)
                    break            

        else:
            self.btn_adc_single_channel.setStyleSheet(utils.SELECTED_STYLE)
            self.btn_adc_all_channels.setStyleSheet('')
            self.prev_adc_btn_channel.animateClick()
       
    def adc_channel_selected(self, btn_channel):
        def wrapper():
            if self.adc_channel_mode == 'single':
                for btn in self.adc_channels_layout.findChildren(QPushButton):
                    btn.setStyleSheet('')

                btn_channel.setStyleSheet(utils.SELECTED_STYLE)
                self.current_adc_channel = btn_channel.text()
                self.prev_adc_btn_channel = btn_channel
                self.get_channel_input_range()
        return wrapper

    def set_channel_input_range(self, input_range):
        if self.adc_channel_mode == 'all':
            self.current_adc.set_input_range_all(input_range)
        else:
            self.current_adc.set_input_range(self.current_adc_channel, input_range)

        self.get_channel_input_range()
        
    def get_channel_input_range(self):
        self.btn_input_range_10.setStyleSheet('')
        self.btn_input_range_5.setStyleSheet('')
        self.btn_input_range_2v5.setStyleSheet('')

        input_range = self.current_adc.get_input_range(self.current_adc_channel)
        if input_range == ad7616.AD7616_InputRange_2V5_PN:
            self.btn_input_range_2v5.setStyleSheet(utils.SELECTED_STYLE)
        elif input_range == ad7616.AD7616_InputRange_5V0_PN:
            self.btn_input_range_5.setStyleSheet(utils.SELECTED_STYLE)
        elif input_range == ad7616.AD7616_InputRange_10V_PN:
            self.btn_input_range_10.setStyleSheet(utils.SELECTED_STYLE)

    def set_current_adc(self, adc):
        if adc == 1:
            self.current_adc = ad7616.adc1
            self.btn_adc1.setStyleSheet(utils.SELECTED_STYLE)
            self.btn_adc2.setStyleSheet('')
        else:
            self.current_adc = ad7616.adc2
            self.btn_adc1.setStyleSheet('')
            self.btn_adc2.setStyleSheet(utils.SELECTED_STYLE)

        self.get_channel_input_range()
        self.get_oversampling_ratio()

    def set_oversampling_ratio(self, ratio):
        self.current_adc.set_over_sampling(ratio)
        self.get_oversampling_ratio()
        
    def get_oversampling_ratio(self):
        for btn in self.over_sampling_ratio_layout.findChildren(QPushButton):
            btn.setStyleSheet('')

        oversamping = self.current_adc.get_over_sampling()
        if oversamping == ad7616.AD7616_OverSampling_Disabled:
            self.btn_os_disabled.setStyleSheet(utils.SELECTED_STYLE)
        elif oversamping == ad7616.AD7616_OverSampling_2:
            self.btn_os_2.setStyleSheet(utils.SELECTED_STYLE)
        elif oversamping == ad7616.AD7616_OverSampling_4:
            self.btn_os_4.setStyleSheet(utils.SELECTED_STYLE)
        elif oversamping == ad7616.AD7616_OverSampling_8:
            self.btn_os_8.setStyleSheet(utils.SELECTED_STYLE)
        elif oversamping == ad7616.AD7616_OverSampling_16:
            self.btn_os_16.setStyleSheet(utils.SELECTED_STYLE)
        elif oversamping == ad7616.AD7616_OverSampling_32:
            self.btn_os_32.setStyleSheet(utils.SELECTED_STYLE)
        elif oversamping == ad7616.AD7616_OverSampling_64:
            self.btn_os_64.setStyleSheet(utils.SELECTED_STYLE)
        elif oversamping == ad7616.AD7616_OverSampling_128:
            self.btn_os_128.setStyleSheet(utils.SELECTED_STYLE)

    def read_adc(self):
        for channel in range(8):
            voltage_A, voltage_B = self.current_adc.read_channel(channel, channel)
            txt_A = eval(f"self.txt_V{channel}A")
            txt_B = eval(f"self.txt_V{channel}B")
            txt_A.setText(str(voltage_A))
            txt_B.setText(str(voltage_B))

##########################################################################################
####################################### DAC LTC2668 ######################################
##########################################################################################

    def dac_channel_selected(self, btn_channel):
        def wrapper():
            if self.dac_channel_mode == 'single':
                for btn in self.dac_channels_layout.findChildren(QPushButton):
                    btn.setStyleSheet('')

                btn_channel.setStyleSheet(utils.SELECTED_STYLE)
                self.current_dac_channel = btn_channel.text().split()[1]
                self.prev_dac_btn_channel = btn_channel
                self.get_channel_softspan()
        return wrapper

    def get_channel_softspan(self):
        for btn in self.softspan_layout.findChildren(QPushButton):
            btn.setStyleSheet('')
        
        softspan = ltc2668.CHANNELS_SOFTSPAN[int(self.current_dac_channel)]

        if softspan == ltc2668.LTC2668_SPAN_0_TO_5V:
            self.btn_span_0_to_5.setStyleSheet(utils.SELECTED_STYLE)
        elif softspan == ltc2668.LTC2668_SPAN_0_TO_10V:
            self.btn_span_0_to_10.setStyleSheet(utils.SELECTED_STYLE)
        elif softspan == ltc2668.LTC2668_SPAN_PLUS_MINUS_2V5:
            self.btn_span_plus_minus_2v5.setStyleSheet(utils.SELECTED_STYLE)
        elif softspan == ltc2668.LTC2668_SPAN_PLUS_MINUS_5V:
            self.btn_span_plus_minus_5.setStyleSheet(utils.SELECTED_STYLE)
        elif softspan == ltc2668.LTC2668_SPAN_PLUS_MINUS_10V:
            self.btn_span_plus_minus_10.setStyleSheet(utils.SELECTED_STYLE)

        softspan_range = ltc2668.SOFTSPAN_RANGE[ltc2668.CHANNELS_SOFTSPAN[int(self.current_dac_channel)]]
        voltage = float(self.txt_dac_voltage.text())
        if voltage > softspan_range[1]:
            voltage = softspan_range[1]
        elif voltage < softspan_range[0]:
            voltage = softspan_range[0]
        
        self.txt_dac_voltage.setText(str(round(voltage, 2)))
         
    def set_channel_softspan(self, softspan):
        if self.dac_channel_mode == 'all':
            ltc2668.set_softspan_range(softspan, 'A')
        else:
            ltc2668.set_softspan_range(softspan, self.current_dac_channel)

        self.get_channel_softspan()

    def set_dac_channel_mode(self, mode):
        self.dac_channel_mode = mode

        if mode == 'all':
            self.btn_dac_single_channel.setStyleSheet('')
            self.btn_dac_all_channels.setStyleSheet(utils.SELECTED_STYLE)

            for btn in self.dac_channels_layout.findChildren(QPushButton):
                btn.setStyleSheet('')
            
            if not ltc2668.All_Channels_Same_Softspan():
                ltc2668.set_softspan_range(ltc2668.LTC2668_SPAN_PLUS_MINUS_10V, 'A')

        else:
            self.btn_dac_single_channel.setStyleSheet(utils.SELECTED_STYLE)
            self.btn_dac_all_channels.setStyleSheet('')
            self.prev_dac_btn_channel.animateClick()

        self.get_channel_softspan()

    def set_reference_voltage(self, ref):
        if ref == ltc2668.REF_EXTERNAL:
            self.btn_vref_external.setStyleSheet(utils.SELECTED_STYLE)
            self.btn_vref_internal.setStyleSheet('')
            ltc2668.set_reference_voltage(ltc2668.REF_EXTERNAL)
        else:
            self.btn_vref_external.setStyleSheet('')
            self.btn_vref_internal.setStyleSheet(utils.SELECTED_STYLE)
            ltc2668.set_reference_voltage(ltc2668.REF_INTERNAL)

    def set_voltage(self, operation):
        softspan = ltc2668.CHANNELS_SOFTSPAN[int(self.current_dac_channel)]
        voltage = float(self.txt_dac_voltage.text())
        if operation == 'inc' and voltage + 0.1 < ltc2668.SOFTSPAN_RANGE[softspan][1]:
            voltage += 0.1
        elif operation == 'dec' and voltage - 0.1 > ltc2668.SOFTSPAN_RANGE[softspan][0]:
            voltage -= 0.1
        
        self.txt_dac_voltage.setText(str(round(voltage, 2)))

    def set_output_voltage(self):
        softspan_range = ltc2668.SOFTSPAN_RANGE[ltc2668.CHANNELS_SOFTSPAN[int(self.current_dac_channel)]]
        voltage = float(self.txt_dac_voltage.text())
        if self.dac_channel_mode == 'all':
            adc_feedback = ltc2668.set_output_voltage(voltage, 'A', softspan_range)
        else:
            adc_feedback = ltc2668.set_output_voltage(voltage, self.current_dac_channel, softspan_range)

        for dac, feedback in adc_feedback.items():
            txt = eval(f'self.txt_chl_{dac}')
            txt.setText(str(feedback))

##########################################################################################
###################################### IOE MCP26S17 ######################################
##########################################################################################

    def ioe_pin_selected(self, btn_pin):
        def wrapper():
            if self.ioe_pin_mode == 'single':
                for btn in self.ioe_pins_layout.findChildren(QPushButton):
                    btn.setStyleSheet('')

                btn_pin.setStyleSheet(utils.SELECTED_STYLE)
                self.current_ioe_pin = int(btn_pin.text().split()[1])
                self.prev_ioe_btn_pin = btn_pin
                self.get_pin_value()
                self.get_pin_direction()
                self.get_pin_pullup()
        return wrapper

    def set_pin_level(self, level):
        if self.ioe_pin_mode == 'all':
            for i in range(16):
                self.current_ioe.setLevel(i, level)
                
        else:
            self.current_ioe.setLevel(self.current_ioe_pin, level)
        
        self.get_pin_value()

    def set_pin_direction(self, direction):
        if self.ioe_pin_mode == 'all':
            for i in range(16):
                self.current_ioe.setDirection(i, direction)
        else:
            self.current_ioe.setDirection(self.current_ioe_pin, direction)
            
        self.get_pin_direction()

    def set_pin_pullup(self, pullup):
        if self.ioe_pin_mode == 'all':
            for i in range(16):
                self.current_ioe.setPullupMode(i, pullup)
        else:
            self.current_ioe.setPullupMode(self.current_ioe_pin, pullup)

        self.get_pin_pullup()

    def get_pin_value(self):
        if self.ioe_pin_mode == 'all':
            for i in range(16):
                level = self.current_ioe.getLevel(i)
                txt = eval(f"self.txt_pin_{i}")
                txt.setText(mcp23s17.MCP23S17.LEVEL_STR[level])
        else:
            level = self.current_ioe.getLevel(self.current_ioe_pin)
            txt = eval(f"self.txt_pin_{self.current_ioe_pin}")
            txt.setText(mcp23s17.MCP23S17.LEVEL_STR[level])

        if level == mcp23s17.MCP23S17.LEVEL_HIGH:
            self.btn_high.setStyleSheet(utils.SELECTED_STYLE)
            self.btn_low.setStyleSheet('')
        elif level == mcp23s17.MCP23S17.LEVEL_LOW:
            self.btn_high.setStyleSheet('')
            self.btn_low.setStyleSheet(utils.SELECTED_STYLE)
        
    def get_pin_direction(self):
        direction = self.current_ioe.getDirection(self.current_ioe_pin)
        if direction == mcp23s17.MCP23S17.DIR_INPUT:
            self.btn_input.setStyleSheet(utils.SELECTED_STYLE)
            self.btn_output.setStyleSheet('')
        elif direction == mcp23s17.MCP23S17.DIR_OUTPUT:
            self.btn_input.setStyleSheet('')
            self.btn_output.setStyleSheet(utils.SELECTED_STYLE)

    def get_pin_pullup(self):
        pullup = self.current_ioe.getPullupMode(self.current_ioe_pin)
        if pullup == mcp23s17.MCP23S17.PULLUP_DISABLED:
            self.btn_pullup_disabled.setStyleSheet(utils.SELECTED_STYLE)
            self.btn_pullup_enabled.setStyleSheet('')
        elif pullup == mcp23s17.MCP23S17.PULLUP_ENABLED:
            self.btn_pullup_disabled.setStyleSheet('')
            self.btn_pullup_enabled.setStyleSheet(utils.SELECTED_STYLE)

    def set_current_ioe(self, ioe):
        if ioe == 1:
            self.current_ioe = mcp23s17.ioe1
            self.btn_ioe1.setStyleSheet(utils.SELECTED_STYLE)
            self.btn_ioe2.setStyleSheet('')
        else:
            self.current_ioe = mcp23s17.ioe2
            self.btn_ioe1.setStyleSheet('')
            self.btn_ioe2.setStyleSheet(utils.SELECTED_STYLE)

        self.get_pin_value()
        self.get_pin_direction()
        self.get_pin_pullup()

    def set_ioe_pin_mode(self, mode):
        self.ioe_pin_mode = mode

        if mode == 'all':
            self.btn_ioe_single_pin.setStyleSheet('')
            self.btn_ioe_all_pins.setStyleSheet(utils.SELECTED_STYLE)

            for btn in self.ioe_pins_layout.findChildren(QPushButton):
                btn.setStyleSheet('')

            x = self.current_ioe.getLevel(0)
            for btn in self.ioe_pins_layout.findChildren(QPushButton):
                if x != self.current_ioe.getLevel(int(btn.text().split()[1])):
                    self.set_pin_level(mcp23s17.MCP23S17.LEVEL_LOW)
                    break

            x = self.current_ioe.getDirection(0)
            for btn in self.ioe_pins_layout.findChildren(QPushButton):
                if x != self.current_ioe.getDirection(int(btn.text().split()[1])):
                    self.set_pin_direction(mcp23s17.MCP23S17.DIR_INPUT)
                    break 

            x = self.current_ioe.getPullupMode(0)
            for btn in self.ioe_pins_layout.findChildren(QPushButton):
                if x != self.current_ioe.getPullupMode(int(btn.text().split()[1])):
                    self.set_pin_pullup(mcp23s17.MCP23S17.PULLUP_DISABLED)
                    break 

        else:
            self.btn_ioe_single_pin.setStyleSheet(utils.SELECTED_STYLE)
            self.btn_ioe_all_pins.setStyleSheet('')
            self.prev_ioe_btn_pin.animateClick()


        self.get_pin_value()
        self.get_pin_direction()
        self.get_pin_pullup()

##########################################################################################
########################################## UART ##########################################
##########################################################################################

    def on_uart_continuous_clicked(self, checked):
        if checked:
            self.btn_uart_send.setStyleSheet(utils.SELECTED_STYLE)
            self.uart_write_timer.start()
        else:
            self.btn_uart_send.setStyleSheet('')
            self.uart_write_timer.stop()

    def uart_write(self):
        data = self.txt_output_buffer.toPlainText()
        uart.write_gui(data.encode())

    def uart_read(self):
        data = uart.read_gui()
        if data:
            try:
                self.txt_input_buffer.insertPlainText(data.decode())
            except Exception:
                self.txt_input_buffer.insertPlainText(data.hex())

            self.txt_input_buffer.verticalScrollBar().setValue(
                self.txt_input_buffer.verticalScrollBar().maximum()
            )

    def clear_buffer(self, buffer):
        if buffer == 'output':
            self.txt_output_buffer.clear()
        else:
            self.txt_input_buffer.clear()

    def on_uart_changed(self, idx):
        uart.set_ctrl_pins(uart.UARTS[idx]['gpio'])
        uart.change_current_uart_gui(idx)
        
##########################################################################################
##########################################################################################
##########################################################################################

    def set_page(self, page):

        for btn in [self.btn_adc, self.btn_dac, self.btn_ioe, self.btn_uart]:
            btn.setStyleSheet(utils.DRIVER_NOT_SELECTED)

        if page == 'adc':
            self.stackedWidget.setCurrentWidget(self.adc_page)
            self.btn_adc.setStyleSheet(utils.DRIVER_SELECTED)

        elif page == 'dac':
            self.stackedWidget.setCurrentWidget(self.dac_page)
            self.btn_dac.setStyleSheet(utils.DRIVER_SELECTED)

        elif page == 'ioe':
            self.stackedWidget.setCurrentWidget(self.ioe_page)
            self.btn_ioe.setStyleSheet(utils.DRIVER_SELECTED)

        elif page == 'uart':
            self.stackedWidget.setCurrentWidget(self.uart_page)
            self.btn_uart.setStyleSheet(utils.DRIVER_SELECTED)
    
    def closeEvent(self, event):
        ad7616.AD7616_Close()
        ltc2668.LTC2668_Close()
        mcp23s17.MCP23S17_Close()
        uart.UART_Close()
        return super().closeEvent(event)



if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = MainWindow()
    monitor = QDesktopWidget().screenGeometry(1)
    window.move(monitor.left(), monitor.top())
    window.showFullScreen()
    app.aboutToQuit.connect(lambda: os.system('reboot'))
    sys.exit(app.exec_())
