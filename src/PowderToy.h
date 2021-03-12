#pragma once

#include "common/String.h"

void EngineProcess();
void ClipboardPush(ByteString text);
ByteString ClipboardPull();
int GetModifiers();
unsigned int GetTicks();
void StartTextInput();
void StopTextInput();
void SetTextInputRect(int x, int y, int w, int h);
