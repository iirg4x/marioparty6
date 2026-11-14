#ifndef M609_COURSE_H
#define M609_COURSE_H

#include "REL/m609dll/state.h"
#include "game/mg/actman.h"

void fn_1_7150(M609Model *segment);
void fn_1_7E10(void);
void fn_1_8670(void);
s32 fn_1_89E0(f32 startZ);
void fn_1_8C54(void);
int fn_1_C9A8(COL_NARROW_PARAM *contact, COL_NARROW_PARAM *otherContact);
void fn_1_CA6C(MGACTOR *actor, int hookParameter);

#endif
