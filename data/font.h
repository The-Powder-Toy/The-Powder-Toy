#pragma once
#define FONT_H 12
#ifndef FONTEDITOR
extern unsigned char font_data[];
extern unsigned short font_ptrs[];
extern unsigned int font_ranges[][2];
#else
extern unsigned char *font_data;
extern unsigned short *font_ptrs;
extern unsigned int (*font_ranges)[2];
#endif
