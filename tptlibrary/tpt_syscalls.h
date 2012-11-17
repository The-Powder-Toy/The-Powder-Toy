#ifndef TPT_SYSCALLS_H
#define TPT_SYSCALLS_H

float sin(float value);
float cos(float value);
float atan2(float x, float y);
float sqrt(float value);
float floor(float value);
float ceil(float value);

void print(char * message);
void error(char * message);

int partCreate(int i, int x, int y, int type);
void partChangeType(int i, int x, int y, int type);
int pmapData(int x, int y);
void deletePart(int x, int y, int flags);
void killPart(int i);

#endif
