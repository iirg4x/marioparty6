#ifndef REL_M653DLL_ALLOCATION_H
#define REL_M653DLL_ALLOCATION_H

void *operator new(unsigned long byteCount);
void operator delete(void *allocation);
#endif
