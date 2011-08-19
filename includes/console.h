#ifndef CONSOLE_H
#define CONSOLE_H

#include <defines.h>
extern char console_more;
extern char console_error[];
extern int file_script;

int console_parse_coords(char *txt, int *x, int *y, char *err);
int console_parse_type(char *txt, int *element, char *err);
int console_parse_partref(char *txt, int *which, char *err);

int process_command_old(pixel *vid_buf, char *console, char *console_error);

#endif
