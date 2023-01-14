#pragma once

#include "common/String.h"
#include <exception>

bool ValidateGOLName(const String &value);
int ParseGOLString(const String &value);
String SerialiseGOLRule(int rule);
