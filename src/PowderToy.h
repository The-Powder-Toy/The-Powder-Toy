#pragma once
#include <string>

extern int depth3d;
void EngineProcess();
void ClipboardPush(std::string text);
std::string ClipboardPull();
int GetModifiers();
bool LoadWindowPosition(int scale);
