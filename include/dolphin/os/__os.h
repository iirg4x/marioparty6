#ifndef _DOLPHIN_OS_INTERNAL_H_
#define _DOLPHIN_OS_INTERNAL_H_

#include <dolphin/os.h>

#ifdef __cplusplus
extern "C" {
#endif

void __OSSetExecParams(const OSExecParams* params, OSExecParams* addr);
OSSram* __OSLockSram(void);
OSSramEx* __OSLockSramEx(void);

#ifdef __cplusplus
}
#endif

#endif
