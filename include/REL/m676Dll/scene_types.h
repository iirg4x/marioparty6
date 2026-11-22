/* Scene object registries, course grids and shared position tables. */
#ifndef REL_M676DLL_SCENE_TYPES_H
#define REL_M676DLL_SCENE_TYPES_H
#include "REL/m676Dll/prop_types.h"
#include "REL/m676Dll/particle_types.h"
typedef struct M676SceneAreaRecord {
    /* Area side (0 unused, 1 left, 2 right) and the index of its paired record. */
    s32 active, next;
    /* Mask suppressing repeated prop triggers at this area, and its world-space position. */
    u32 propMask;
    Vec position;
} M676SceneAreaRecord;
typedef struct M676SceneFlagEntry {
    /* Whether the flag is active and its world-space position. */
    s32 active;
    Vec position;
} M676SceneFlagEntry;

extern s32 lbl_1_bss_480[5];
extern s32 lbl_1_bss_494[5];
extern s32 lbl_1_bss_4A8[75];
extern s32 lbl_1_bss_5D4;
extern OMOBJ *lbl_1_bss_5D8[5];
extern M676SceneAreaRecord lbl_1_bss_5EC[70];
extern M676SceneFlagEntry lbl_1_bss_C7C[28];
extern s32 lbl_1_bss_E3C;
extern OMOBJ *lbl_1_bss_EA0[128];
extern const Vec lbl_1_rodata_46C[4];
extern const Vec lbl_1_rodata_4C4[5];
extern const Vec lbl_1_rodata_558[5];
extern Vec lbl_1_data_2D0[2];
extern OMOBJ *lbl_1_bss_10A0;
extern OMOBJ *lbl_1_bss_E9C;
extern OMOBJ *lbl_1_bss_E98, *lbl_1_bss_E94;
extern s32 lbl_1_bss_E90, lbl_1_bss_E8C, lbl_1_bss_E88;
extern M676SceneRegistration lbl_1_bss_E40;
extern M676PropParam lbl_1_data_36C[];
extern const M676CollisionBounds lbl_1_rodata_340;
extern Vec lbl_1_data_6E0[6];
extern f32 lbl_1_data_728[6];
typedef s32 (*M676SequenceStep)(M676PropWork *state);
extern M676SequenceStep lbl_1_data_75C[];
extern Vec lbl_1_data_768;
extern Vec lbl_1_data_790;
extern void (*lbl_1_data_6D0[4])(M676ParticleRecord *record);

void fn_1_88C0(HUPROCESS *process);
s32 fn_1_F7DC(Vec *value);
s32 fn_1_F940(s32 index, Vec *resultValues);
void fn_1_F9B0(s32 objectIndex, s32 result, s32 character);
s32 fn_1_FA68(Vec *position, f32 angle);
s32 fn_1_FBCC(s32 objectIndex);
s32 fn_1_FC30(s32 index, Vec *resultValues);
f32 fn_1_FD64(void);
s32 fn_1_FE28(s32 index);
void fn_1_10240(s32 index);
#endif
