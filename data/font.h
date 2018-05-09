#pragma once
#define FONT_H 12
#ifndef FONTEDITOR
extern const unsigned char font_data[];
extern const unsigned short font_ptrs[];
extern const unsigned int font_ranges[][2];
#else
extern const unsigned char *font_data;
extern const unsigned short *font_ptrs;
extern const unsigned int (*font_ranges)[2];
#endif
