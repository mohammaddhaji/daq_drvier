# BeagleBone-X15 Linux Userspace Drivers for DAQ Board


## Overview

This repository contains Linux userspace drivers for the BeagleBone-X15 DAQ (Data Acquisition) board, which provides ADC (Analog-to-Digital Converter), DAC (Digital-to-Analog Converter), and GPIO (General Purpose Input/Output) functionalities. The drivers are implemented in C and Python, offering versatility and ease of use for interacting with the DAQ board's hardware components.

## Supported Features

1. **ADC Driver**: The ADC driver allows users to read analog signals from the external environment with precise accuracy. It provides reliable conversion from analog to digital data, making it suitable for various data acquisition applications.

2. **DAC Driver**: With the DAC driver, users can generate analog output signals to control external devices or systems. This feature is ideal for tasks that require precise analog voltage outputs.

3. **GPIO Driver**: The GPIO driver enables users to configure pins on the DAQ board for both input and output operations. It provides flexibility in controlling external devices or reading data from sensors.

4. **Console-Based Menu**: We've included a user-friendly console-based menu in Python to interact with the drivers conveniently. The menu allows users to select specific functionalities and provides real-time feedback on operations.

5. **Graphical App**: Additionally, we offer a graphical application built with Python that provides a visually appealing interface for interacting with the drivers. The graphical app makes it easy for users to monitor and control the DAQ board functionalities.
