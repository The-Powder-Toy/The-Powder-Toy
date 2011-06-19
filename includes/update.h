/**
 * Powder Toy - Main source
 *
 * Copyright (c) 2008 - 2010 Stanislaw Skowronek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */
#ifndef UPDATE_H
#define UPDATE_H

#include <defines.h>

extern int version_check;

char *exe_name(void);
int update_start(char *data, int len);
int update_finish(void);
void update_cleanup(void);
void update_finalize(pixel *vid_buf);
void update_notify_ui_draw(pixel *vid_buf);
void update_notify_ui_process(pixel *vid_buf, int mb, int mbq, int mx, int my);

#endif
