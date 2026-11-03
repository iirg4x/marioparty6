#ifndef MSMSTREAM_H
#define MSMSTREAM_H

#include "msm.h"

void msmStreamPeriodicProc(void);
void msmStreamAmemFree(void);
s32 msmStreamAmemAlloc(void);
s32 msmStreamInit(char* pdtPath);

#endif
