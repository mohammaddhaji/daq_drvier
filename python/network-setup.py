#!/usr/bin/python

import subprocess
import time
import os


IP = '192.168.1.2'


timeout_1 = 7
timeout_2 = 2

start_time_1 = time.perf_counter()


def has_numbers(inputString):
    return any(char.isdigit() for char in inputString)

def GREEN(txt):
    return f"\033[92m{txt}\033[0m"

def RED(txt):
    return f"\033[91m{txt}\033[0m"


def get_speed_and_duplex():
    output = subprocess.check_output('ethtool eth0', shell=True).decode()
    lines = [l.strip() for l in output.split('\n')]
    speed, duplex = '', ''
    for l in lines:
        if l.startswith('Speed'):
            speed = l.split(':')[1].strip()

        if l.startswith('Duplex'):
            duplex = l.split(':')[1].strip()

    return speed, duplex



os.system('ifconfig -a &')
time.sleep(0.2)
os.system('ifconfig -a &')
time.sleep(0.2)
os.system('ifconfig eth0 up')
time.sleep(0.3)



while True:

    t1 = time.perf_counter()
    if t1 - start_time_1 > timeout_1:
        print(f"\n{RED('Failed...')} Could not setup network.\n")
        exit(1)

    speed, duplex = get_speed_and_duplex()  

    if has_numbers(speed) and duplex in ['Full', 'Half']:

        print(f"{GREEN('Link detected')}. Speed: {speed}, Duplex: {duplex}\n")
        stable = True
        start_time_2 = time.perf_counter()
        os.system('sudo ethtool -s eth0 speed 100 duplex full autoneg off')
        

        while stable:
            t2 = time.perf_counter()
            if t2 - start_time_2 > timeout_2:
                print(f"\n{GREEN('Success...')}  Setting IP: {IP}\n")
                os.system(f"ifconfig eth0 {IP}")
                exit(0)

            
            speed, duplex = get_speed_and_duplex()
            info = f"Speed: {speed}, Duplex: {duplex}"

            if has_numbers(speed) and duplex in ['Full', 'Half']:
                print(f"Checking if network is stable...  {GREEN('OK')}   {info}")
                stable = True
            else:
                print(f"Checking if network is stable...  {RED('Failed')}   {info}")
                stable = False

            time.sleep(0.1)
    

    time.sleep(0.1)


