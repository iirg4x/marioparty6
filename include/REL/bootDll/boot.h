#ifndef REL_BOOTDLL_BOOT_H
#define REL_BOOTDLL_BOOT_H

#include "game/object.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Functions shared by the module's source files. */
void OpeningCreate(void);
void OpeningExec(s32 enablePrompt);
void OpeningKill(void);

#ifdef __cplusplus
}
#endif

#endif
