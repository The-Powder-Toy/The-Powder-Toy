/**
 * Powder Toy - simple console (header)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
