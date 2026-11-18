#ifndef REL_M610DLL_SCENE_CALLBACK_H
#define REL_M610DLL_SCENE_CALLBACK_H
#include "dolphin/types.h"
#ifdef __cplusplus
extern "C"
{
#endif
    /* Callback for contacts between the registered scene objects. */
    typedef void (*SceneContactCallback)(s32, void *, s32, void *);
    /* Camera transition request and end-of-game state shared by scene objects. */
    extern int lbl_1_bss_64;
    extern unsigned char lbl_1_bss_68;
    void fn_1_B914(SceneContactCallback callback);
    void fn_1_107E0(s32 mode, void *firstOwner, s32 kind, void *secondOwner);
#ifdef __cplusplus
}
#endif
#endif
