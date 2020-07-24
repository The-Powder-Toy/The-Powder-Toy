#pragma once
#define FONT_H 12
#ifndef FONTEDITOR
extern const unsigned char font_data[];
extern const unsigned int font_ptrs[];
extern const unsigned int font_ranges[][2];
#else
extern unsigned char *font_data;
extern unsigned int *font_ptrs;
extern unsigned int (*font_ranges)[2];
#endif
