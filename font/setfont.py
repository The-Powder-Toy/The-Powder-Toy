#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, codecs, os, re

reload(sys)
sys.setdefaultencoding('UTF-8')

fontchar = codecs.open('fontchar.txt', 'r', 'UTF-8')

charindex = 0
ptrcount = 256
ptrindex = 4615
lastchar = 255
ptrline = ""
fontfile = open('fontsample.h', 'r')
contents = fontfile.readlines()
fontfile.close()

for char in fontchar.read():
    charindex += 1
    cmd = "convert -size 10x10 xc:none +antialias -gravity center -pointsize 10 "
    cmd += "-font \"C:\\\\Windows\\\\Fonts\\\\ZpixEX2_EX.ttf\" label:\""
    cmd += char
    cmd += "\" result.xbm"
    print cmd
    if os.system(cmd.decode('UTF-8').encode('cp936')):
        print('IM process failed!')
    else:
        print('IM process success.')

    charmap = open('result-1.xbm', 'r').read()
    bit = []
    i = 0
    for hexstr in re.finditer('0x', charmap):
        i += 1
        byte = int(charmap[hexstr.start():hexstr.start() + 4], 16)
        if i % 2:
            rowcount = 8
        else:
            rowcount = 2
        for k in range(rowcount):
            bit.append(byte >> k & 1)

    i = 0
    line = "    0x0A, "
    while i < 100:
        bitslice = bit[i:i + 4]
        byte = 0
        for k in range(3, -1, -1):
            byte = byte << 2 | (bitslice[k] << 1) | bitslice[k]
        line += "0x" + format(byte, '02X') + ", "
        i += 4
    contents.insert(charindex + 260, line + "\n")

    currchar = int(repr(char)[4:8], 16)
    for i in range(lastchar + 1, currchar):
        if not ptrcount % 8:
            ptrline = "    "
        ptrline += "0x0000, "
        if ptrcount % 8 == 7:
            contents.insert(charindex + 262 + ((ptrcount + 1) / 8), ptrline + "\n")
        ptrcount += 1
    lastchar = currchar

    ptrindex += 26
    if not ptrcount % 8:
        ptrline = "    "
    ptrline += "0x" + format(ptrindex, '0002X') + ", "
    if ptrcount % 8 == 7:
        contents.insert(charindex + 262 + ((ptrcount + 1) / 8), ptrline + "\n")
    ptrcount += 1

fontfile = open('font.h', 'w')
fontfile.writelines(contents)
fontfile.close()

fontchar.close()
