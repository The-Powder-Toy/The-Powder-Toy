#!/usr/bin/env python3

import bz2
import math
import re
import argparse

CP_MAX = 0x10FFFF
FONT_CPP = "data/font.bz2"
FONT_HEIGHT = 12


class ReadBDFError(RuntimeError):
    def __init__(self, line_number, message):
        super().__init__(self, 'line %i: %s' % (line_number, message))


class FontTool:
    def __init__(self, file):
        self.file = file
        with open(self.file, 'rb') as font_cpp:
            font_cpp_data = bz2.decompress(font_cpp.read())
        i = 0
        self.code_points = [False for _ in range(CP_MAX + 2)]
        while i < len(font_cpp_data):
            cp = font_cpp_data[i] | (font_cpp_data[i + 1] << 8) | (font_cpp_data[i + 2] << 16)
            width = font_cpp_data[i + 3]
            n = i + 4 + 3 * width
            self.code_points[cp] = font_cpp_data[(i + 3): n]
            i = n

    def commit(self):
        l = []
        for i, data in enumerate(self.code_points):
            if data:
                l.append(i & 0xFF)
                l.append((i >> 8) & 0xFF)
                l.append((i >> 16) & 0xFF)
                l += data
        with open(self.file, 'wb') as font_cpp:
            font_cpp.write(bz2.compress(bytes(l)))

    def pack(cp_matrix):
        width = 0
        for row in cp_matrix:
            if width < len(row):
                width = len(row)
        cp_data = [width]
        bits = 8
        for row in cp_matrix:
            padded = row + [0] * (width - len(row))
            for cv in padded:
                if bits == 8:
                    cp_data.append(0)
                    bits = 0
                cp_data[-1] |= (cv & 3) << bits
                bits += 2
        return cp_data

    def unpack(cp_data):
        ptr = 1
        bits = 0
        buf = 0
        cp_matrix = []
        for y in range(FONT_HEIGHT):
            cp_matrix.append([])
            for x in range(cp_data[0]):
                if bits == 0:
                    buf = cp_data[ptr]
                    ptr += 1
                    bits = 8
                cp_matrix[-1].append(buf & 3)
                buf >>= 2
                bits -= 2
        return cp_matrix

    def dump(self, i, print):
        lut = ['  ', '░░', '▒▒', '▓▓']
        if self.code_points[i]:
            print('code point %i (%c)' % (i, i))
            print('')
            for l in [''.join([lut[ch] for ch in row]) for row in FontTool.unpack(self.code_points[i])]:
                print(l)
            print('')
        else:
            print('code point %i (%c) is not available' % (i, i))


class RawReader:
    def __init__(self, path):
        self.code_points = [False for _ in range(CP_MAX + 2)]
        with open(path) as raw:
            items = [int(v) for v in re.findall(r'[0-9]+', raw.read())]
        ptr = 0
        while ptr <= len(items) - 2:
            cp = items[ptr]
            width = items[ptr + 1]
            ptr += 2
            matrix = []
            for i in range(ptr, ptr + width * FONT_HEIGHT, width):
                matrix.append(items[i: (i + width)])
            ptr += width * FONT_HEIGHT
            self.code_points[cp] = FontTool.pack(matrix)


class BDFReader:

    def __init__(self, path, xoffs, yoffs):
        self.code_points = [False for _ in range(CP_MAX + 2)]
        item_re = re.compile(r'[^ \n\r]+')
        with open(path) as bdf:
            global_dw = False
            startchar = False
            bitmap = False
            char_dw = False
            char_cp = False
            char_bbx = False
            skip = 0
            for line_number, line in enumerate(bdf):
                if skip:
                    skip -= 1
                    continue
                items = re.findall(item_re, line)
                if startchar and items[0] == 'ENDCHAR':
                    if len(bitmap) != char_bbx[1]:
                        raise ReadBDFError(line_number, "invalid bitmap data")
                    cp_matrix = []
                    for y in range(FONT_HEIGHT):
                        cp_matrix.append([])
                        for x in range(char_dw):
                            cv = 0
                            xx = x + xoffs
                            yy = FONT_HEIGHT - 1 - y + yoffs
                            if char_bbx[2] <= xx < char_bbx[0] + char_bbx[2] and char_bbx[3] <= yy < char_bbx[1] + \
                                    char_bbx[3]:
                                cv = bitmap[char_bbx[1] - 1 - (yy - char_bbx[3])][xx - char_bbx[2]] * 3
                            cp_matrix[-1].append(cv)
                    self.code_points[char_cp] = FontTool.pack(cp_matrix)
                    startchar = False
                    bitmap = False
                    char_dw = False
                    char_cp = False
                    char_bbx = False
                elif bitmap != False:
                    if len(items) != 1:
                        raise ReadBDFError(line_number, "missing bitmap data")
                    bits = []
                    for ch in items[0]:
                        cv = int(ch, 16)
                        bits += [cv & 8 and 1 or 0, cv & 4 and 1 or 0, cv & 2 and 1 or 0, cv & 1 and 1 or 0]
                    bitmap.append(bits[: char_bbx[0]])
                elif items[0] == 'SIZE':
                    if len(items) != 4:
                        raise ReadBDFError(line_number, "invalid directive")
                elif items[0] == 'FONTBOUNDINGBOX':
                    if len(items) != 5:
                        raise ReadBDFError(line_number, "invalid directive")
                elif not startchar and items[0] == 'STARTCHAR':
                    startchar = True
                    char_dw = global_dw
                elif items[0] == 'STARTPROPERTIES':
                    if len(items) != 2:
                        raise ReadBDFError(line_number, "invalid directive")
                    skip = int(items[1]) + 1
                elif startchar and items[0] == 'BITMAP':
                    bitmap = []
                elif startchar and items[0] == 'BBX':
                    if len(items) != 5:
                        raise ReadBDFError(line_number, "invalid directive")
                    char_bbx = [int(items[1]), int(items[2]), int(items[3]), int(items[4])]
                elif startchar and items[0] == 'ENCODING':
                    if len(items) != 2:
                        raise ReadBDFError(line_number, "invalid directive")
                    char_cp = int(items[1])
                elif items[0] == 'METRICSSET':
                    if len(items) != 2:
                        raise ReadBDFError(line_number, "invalid directive")
                    if int(items[1]) == 1:
                        raise ReadBDFError(line_number, "font does not support writing direction 0")
                elif items[0] == 'DWIDTH':
                    if len(items) != 3:
                        raise ReadBDFError(line_number, "invalid directive")
                    if int(items[2]) != 0:
                        raise ReadBDFError(line_number, "vertical component of dwidth vector is non-zero")
                    char_dw = int(items[1])
                    if not startchar:
                        global_dw = char_dw

def pad_str(s, pad, n):
    return s + pad * (n - len(s))

if __name__ == "__main__":

    parser = argparse.ArgumentParser("fonttool.py", description="font tools for managing fonts, this script can be"
                                                                " imported as a module",
                                     fromfile_prefix_chars="@")
    command = parser.add_subparsers(dest="command", required=True)

    addbdf = command.add_parser("addbdf", help="Adds BDF Formated Font")
    addbdf.add_argument("first", metavar="FIRST", type=int)
    addbdf.add_argument("last", metavar="LAST", type=int)
    addbdf.add_argument("bdffile", metavar="BDFFILE", help="BDF is an archaic bitmap font format")
    addbdf.add_argument("xoffs", metavar="XOFFS", nargs="?", default=0, type=int, help="Defaults to 0")
    addbdf.add_argument("yoffs", metavar="YOFFS", nargs="?", default=0, type=int, help="Defaults to 0")

    addraw = command.add_parser("addraw", help="Adds a Raw Formated Font")
    addraw.add_argument("first", metavar="FIRST", type=int)
    addraw.add_argument("last", metavar="LAST", type=int)
    addraw.add_argument("rawfile", metavar="RAWFILE", help=""""Raw" files are simply ASCII-encoded white-space delimited \
lists
of decimal integer constants. These lists of integers encode
characters as any number of consecutive character description
structures laid out as follows:
  * the code point corresponding to the character being described;
  * the width in pixels of the character being described;
  * width times %i brightness levels between 0 and 3, a row-major matrix.""")

    remove = command.add_parser("remove", help="Remove a range of characters")
    remove.add_argument("first", metavar="FIRST", type=int)
    remove.add_argument("last", metavar="LAST", type=int, default=None, nargs="?", help="Defaults to FIRST")

    copy = command.add_parser("copy", help="Copy a range of characters to another range")
    copy.add_argument("dest", metavar="DSTFIRST", type=int)
    copy.add_argument("first", metavar="SRCFIRST", type=int)
    copy.add_argument("last", metavar="SRCLAST", type=int, default=None, nargs="?", help="Defaults to SRCFIRST")

    inspect = command.add_parser("inspect", help="Inspect a range of characters")
    inspect.add_argument("first", metavar="FIRST", type=int)
    inspect.add_argument("last", metavar="LAST", type=int, default=None, nargs="?", help="Defaults to FIRST")

    diff = command.add_parser("diff", help="Prints subranges that changed between the current and an external font file")
    diff.add_argument("external", metavar="EXTERNAL", help="External font.bz2")

    args = parser.parse_args()

    if 'first' in args:
        cp_first = args.first
        if args.last is None:
            cp_last = cp_first
        else:
            cp_last = args.last
        if cp_first < 0 or cp_last > CP_MAX or cp_first > cp_last:
            print('invalid range')
            exit(1)

    ft = FontTool(FONT_CPP)

    if args.command == "addbdf":
        xoffs = args.xoffs
        yoffs = args.yoffs
        bdfr = BDFReader(args.bdffile, xoffs, yoffs)
        for i in range(cp_first, cp_last + 1):
            if bdfr.code_points[i] and not ft.code_points[i]:
                ft.code_points[i] = bdfr.code_points[i]
        ft.commit()
    elif args.command == 'addraw':
        rr = RawReader(args.rawfile)
        for i in range(cp_first, cp_last + 1):
            if rr.code_points[i] and not ft.code_points[i]:
                ft.code_points[i] = rr.code_points[i]
        ft.commit()
    elif args.command == 'remove':
        for i in range(cp_first, cp_last + 1):
            ft.code_points[i] = False
        ft.commit()
    elif args.command == 'copy':
        for i in range(cp_first, cp_last + 1):
            ft.code_points[i + (args.dest - cp_first)] = ft.code_points[i]
        ft.commit()
    elif args.command == 'inspect':
        for i in range(cp_first, cp_last + 1):
            ft.dump(i, print)
    elif args.command == 'diff':
        pad_to = 50
        eft = FontTool(args.external)
        for i in range(0, CP_MAX + 1):
            if eft.code_points[i] != ft.code_points[i]:
                cur = []
                def add_cur(line):
                    global cur
                    cur.append(line)
                ft.dump(i, add_cur)
                ext = []
                def add_ext(line):
                    global ext
                    ext.append(line)
                eft.dump(i, add_ext)
                print('#' * (2 * pad_to + 7))
                for j in range(max(len(cur), len(ext))):
                    print('# ' + pad_str(j < len(cur) and cur[j] or '', ' ', pad_to) + ' # ' + pad_str(j < len(ext) and ext[j] or '', ' ', pad_to) + ' #')
