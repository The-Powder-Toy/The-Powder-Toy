#pragma once

#include "common/String.h"
#include <exception>

bool ValidateGOLName(const String &value);
int ParseGOLString(const String &value);
String SerialiseGOLRule(int rule);
bool AddCustomGol(String ruleString, String nameString, unsigned int highColor, unsigned int lowColor);
