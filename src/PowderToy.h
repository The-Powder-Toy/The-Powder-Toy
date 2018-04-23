#pragma once

void EngineProcess();
void ClipboardPush(ByteString text);
ByteString ClipboardPull();
int GetModifiers();
unsigned int GetTicks();
