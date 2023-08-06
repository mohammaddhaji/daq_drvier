import os


DEBUG = True if os.name=='nt' else False

# Colorful text for terminal ------------------------------------------------
COLORIZE = True

class bcolors:
    PURPLE = '\033[95m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    WARNING = '\033[93m'
    RED = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def BLUE(txt):
    return f"{bcolors.BLUE}{txt}{bcolors.ENDC}" if COLORIZE else txt

def RED(txt):
    return f"{bcolors.RED}{txt}{bcolors.ENDC}" if COLORIZE else txt

def PURPLE(txt):
    return f"{bcolors.PURPLE}{txt}{bcolors.ENDC}" if COLORIZE else txt

def GREEN(txt):
    return f"{bcolors.GREEN}{txt}{bcolors.ENDC}" if COLORIZE else txt

def CYAN(txt):
    return f"{bcolors.CYAN}{txt}{bcolors.ENDC}" if COLORIZE else txt

#----------------------------------------------------------------------------

def debug_print(info):
    if DEBUG: print(info)

def format_packet(packet):
    return f"({BLUE(' '.join(packet.hex()[i:i+2].upper() for i in range(0, len(packet.hex()), 2)))})"


def clear_screen():
    os.system('cls' if os.name=='nt' else 'clear')



DRIVER_SELECTED = """
QPushButton{
    border-top-right-radius: 15px;
    border-bottom-right-radius: 15px;
    border:1px solid black;
    background-color: rgb(0, 170, 255);
    color:white;
}
"""

DRIVER_NOT_SELECTED = """
QPushButton{
    border-top-right-radius: 15px;
    border-bottom-right-radius: 15px;
    border:1px solid black;
}
"""

SELECTED_STYLE = """
QPushButton {
	background-color: rgb(0, 170, 255);
	color:white;
}
"""