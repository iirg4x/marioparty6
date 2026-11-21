#ifndef REL_M623DLL_SCENE_STORAGE_H
#define REL_M623DLL_SCENE_STORAGE_H
#include "dolphin/types.h"

struct PlayerState;
struct SceneShadow;
struct TriangleCollision;

// Shared scene shadow.
extern SceneShadow *lbl_1_bss_24;
// The lone player.
extern PlayerState *lbl_1_bss_28;
// The opposing team.
extern PlayerState *lbl_1_bss_2C[3];
// Flag the game timer tests after clearing it.
extern unsigned char lbl_1_bss_38;
// Elapsed frames during play.
extern s32 lbl_1_bss_3C;
// Y coordinate of the lone player.
extern f32 lbl_1_bss_44;
// Z coordinate of the lone player.
extern f32 lbl_1_bss_48;
// Course collision query.
extern TriangleCollision *lbl_1_bss_4C;

// Returns the upper X bound.
f32 fn_1_1D98(void);

// Returns the lower X bound.
f32 fn_1_1E5C(void);

#endif
