#pragma once
#include <string>

void EngineProcess();
void ClipboardPush(std::string text);
std::string ClipboardPull();
int GetModifiers();
bool LoadWindowPosition(int scale);
unsigned int GetTicks();
