#pragma once

#include "common/String.h"
#include "gui/game/GameController.h"

void EngineProcess(GameController* gameController);
void ClipboardPush(ByteString text);
ByteString ClipboardPull();
int GetModifiers();
unsigned int GetTicks();
