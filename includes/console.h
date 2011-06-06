#ifndef CONSOLE_H
#define CONSOLE_H

#include <defines.h>
#ifdef PYCONSOLE
#include "Python.h"
//#include "pystdlib.h"
#include <marshal.h>
#endif

extern char pyready, pygood, console_more;
extern char console_error[];
extern int file_script;
#ifdef PYCONSOLE
extern PyMethodDef EmbMethods[];
#endif

int console_parse_coords(char *txt, int *x, int *y, char *err);
int console_parse_type(char *txt, int *element, char *err);
int console_parse_partref(char *txt, int *which, char *err);

#ifdef PYCONSOLE
int process_command(pixel *vid_buf, char *console, char *console_error, PyObject *pfunc);
#endif
int process_command_old(pixel *vid_buf, char *console, char *console_error);

#endif
