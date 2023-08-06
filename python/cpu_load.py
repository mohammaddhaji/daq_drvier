#!/usr/bin/python

import os

os.chdir('/root')

size = 10000000

print('create a file with size:', size, 'bytes.')
f = open('newfile', 'wb')
f.write(b'0' * size)
f.close()


for i in range(100):
    print('STEP:', i)
    os.system('cp newfile newfile-copy')
    os.system('rm newfile-copy')

os.system('rm newfile')
