#define _MATH_H
#include "REL/mdminidll/camera.h"
#include "REL/mdminidll/model_animation.h"
#include "game/window.h"

extern OMOBJMAN *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_24;
extern HUWINID lbl_1_bss_858[4];
extern HU3D_LIGHTID lbl_1_bss_860[2];
extern void fn_1_257D0(void);

void fn_1_159BC(void)
{
    MDMinidllCameraState *cameraState;
    s16 modelRow;
    s16 animation;
    s16 winIndex;

    fn_1_257D0();
    if (lbl_1_bss_24) {
        *lbl_1_bss_24->mtnId = -1;
        modelRow = 20;
        while (modelRow >= 0) {
            animation = 0;
            while (animation < 4) {
                Hu3DAnimKill(lbl_1_bss_230[modelRow].animation[animation]);
                animation++;
            }
            Hu3DModelKill(lbl_1_bss_230[modelRow].model);
            modelRow--;
        }
    }
    lbl_1_bss_24 = NULL;
    winIndex = 0;
    while (winIndex < 4) {
        HuWinExKill(lbl_1_bss_858[winIndex]);
        winIndex++;
    }
    HuWinAllKill();
    Hu3DGLightKill(lbl_1_bss_860[0]);
    Hu3DGLightKill(lbl_1_bss_860[1]);
    cameraState = &lbl_1_bss_808;
    Hu3DCameraKill(1);
    if (cameraState->outViewObj) {
        omDelObjEx(lbl_1_bss_4, cameraState->outViewObj);
    }
    cameraState->outViewObj = NULL;
}
