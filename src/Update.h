#pragma once
#include "Config.h"

//char *exe_name(void);
int update_start(char *data, unsigned int len);
int update_finish();
void update_cleanup();
