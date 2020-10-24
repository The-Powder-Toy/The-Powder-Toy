#!/usr/bin/env python3

import math
import re
import argparse

CP_MAX = 0x10FFFF
FONT_CPP = "data/font.cpp"
FONT_HEIGHT = 12
PTRS_PER_LINE = 8


class ReadBDFError(RuntimeError):
    def __init__(self, line_number, message):
        super().__init__(self, 'line %i: %s' % (line_number, message))


class FontTool:
    def __init__(self):
        with open(FONT_CPP) as font_cpp:
            self.font_cpp_data = font_cpp.read()
        font_data = ([int(s, 16) for s in re.findall(r'\w+', re.search(r'font_data[^{]*{([^;]+);', self.font_cpp_data,
                                                                       re.MULTILINE | re.DOTALL)[1])])
        font_ptrs = ([int(s, 16) for s in re.findall(r'\w+', re.search(r'font_ptrs[^{]*{([^;]+);', self.font_cpp_data,
                                                                       re.MULTILINE | re.DOTALL)[1])])
        font_ranges = ([int(s, 16) for s in re.findall(r'\w+',
                                                       re.search(r'font_ranges[^{]*{([^;]+);', self.font_cpp_data,
                                                                 re.MULTILINE | re.DOTALL)[1])])
        self.code_points = [False for _ in range(CP_MAX + 2)]
        ptrs_ptr = 0
        for i in range(len(font_ranges) // 2 - 1):
            for cp in range(font_ranges[i * 2], font_ranges[i * 2 + 1] + 1):
                base = font_ptrs[ptrs_ptr]
                self.code_points[cp] = font_data[base: (base + math.ceil(font_data[base] * FONT_HEIGHT / 4) + 1)]
                ptrs_ptr += 1

    def commit(self):
        new_ranges = []
        in_range = False
        for i, data in enumerate(self.code_points):
            if in_range and not data:
                new_ranges[-1].append(i - 1)
                in_range = False
            elif not in_range and data:
                in_range = True
                new_ranges.append([i])
        font_data_lines_hex = [['0x%02X' % v for v in d] for d in filter(lambda x: x, self.code_points)]
        font_data_lines = [len(h) > 1 and h[0] + ',   ' + ', '.join(h[1:]) + ',' or '0x00,  ' for h in
                           font_data_lines_hex]
        font_cpp_data = re.sub(r'font_data[^{]*{([^;]+);',
                               'font_data[] = {\n    ' + '\n    '.join(font_data_lines) + '\n};', self.font_cpp_data)
        font_ptrs_blocks = []
        data_ptr = 0
        for ran in new_ranges:
            block = []
            for cp in range(ran[0], ran[1] + 1):
                block.append(data_ptr)
                data_ptr += math.ceil(self.code_points[cp][0] * FONT_HEIGHT / 4) + 1
            font_ptrs_wrapped = []
            for i in range(0, len(block), PTRS_PER_LINE):
                font_ptrs_wrapped.append(', '.join(['0x%08X' % v for v in block[i: (i + PTRS_PER_LINE)]]))
            font_ptrs_blocks.append(',\n    '.join(font_ptrs_wrapped))
        font_cpp_data = re.sub(r'font_ptrs[^{]*{([^;]+);',
                               'font_ptrs[] = {\n    ' + ',\n\n    '.join(font_ptrs_blocks) + ',\n};', font_cpp_data)
        font_ranges_lines = ['{ 0x%06X, 0x%06X },' % (r[0], r[1]) for r in new_ranges]
        font_cpp_data = re.sub(r'font_ranges[^{]*{([^;]+);',
                               'font_ranges[][2] = {\n    ' + '\n    '.join(font_ranges_lines) + '\n    { 0, 0 },\n};',
                               font_cpp_data)
        with open(FONT_CPP, 'w') as font_cpp:
            font_cpp.write(font_cpp_data)

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

    remove = command.add_parser("remove", help="Remove")
    remove.add_argument("first", metavar="FIRST", type=int)
    remove.add_argument("last", metavar="LAST", type=int, default=None, nargs="?", help="Defaults to FIRST")

    copy = command.add_parser("copy", help="Copy")
    copy.add_argument("dest", metavar="DSTFIRST", type=int)
    copy.add_argument("first", metavar="SRCFIRST", type=int)
    copy.add_argument("last", metavar="SRCLAST", type=int, default=None, nargs="?", help="Defaults to SRCFIRST")

    inspect = command.add_parser("inspect", help="Inspect")
    inspect.add_argument("first", metavar="FIRST", type=int)
    inspect.add_argument("last", metavar="LAST", type=int, default=None, nargs="?", help="Defaults to FIRST")

    args = parser.parse_args()

    cp_first = args.first
    cp_dest = args.dest
    if args.last is None:
        cp_last = cp_first
    else:
        cp_last = args.last
    if cp_first < 0 or cp_last > CP_MAX or cp_first > cp_last:
        print('invalid range')
        exit(1)

    ft = FontTool()

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
            ft.code_points[i + (cp_dest - cp_first)] = ft.code_points[i]
        ft.commit()
    elif args.command == 'inspect':
        lut = ['  ', '░░', '▒▒', '▓▓']
        for i in range(cp_first, cp_last + 1):
            if ft.code_points[i]:
                print('code point %i (%c)' % (i, i))
                print('')
                print('\n'.join([''.join([lut[ch] for ch in row]) for row in FontTool.unpack(ft.code_points[i])]))
                print('')
            else:
                print('code point %i (%c) is not available' % (i, i))
