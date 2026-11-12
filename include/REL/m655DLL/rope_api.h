#ifndef REL_M655DLL_ROPE_API_H
#define REL_M655DLL_ROPE_API_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
struct GameObject;
struct Rope;
GameObject *fn_1_B140(s32 id);
s32 fn_1_B8E0(Rope *rope, const Vec *target, Vec *result, f32 amount);
#endif
