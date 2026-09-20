#include "REL/m618dll.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/gamework.h"
#include "game/hsfex.h"

HUPROCESS *lbl_1_bss_4;
int lbl_1_bss_0;

/* Shared readonly values also consumed by the unrecovered part of this TU. */

MGSEQ_PARAM lbl_1_data_0 = {
    300, 1, fn_1_DC, fn_1_1EC, fn_1_294, fn_1_2B4, fn_1_2F0,
    fn_1_310, fn_1_33C, fn_1_340, fn_1_344
};

/* Shared constants retain the same ownership as the unrecovered consumers. */

/* Existing shared camera literal immediately precedes the model table. */

void fn_1_A0(void)
{
    lbl_1_bss_4 = MgActorObjectSetup();
    MgSeqCreatePrio(&lbl_1_data_0, 8192);
}

void fn_1_DC(s16 mode, s16 frameNo)
{
    HuVecF pos = { 0.0f, 1000.0f, 5000.0f };
    HuVecF dir = { 0.0f, -1.0f, -1.0f };
    GXColor color = { 255, 255, 255, 255 };
    OMOBJ *obj;
    HU3D_LIGHTID light;

    Hu3DCameraCreate(15);
    obj = omAddObjEx(lbl_1_bss_4, OM_OUTVIEW_PRIO, 0, 0, -1, omOutViewMulti);
    obj->work[0] = 4;
    light = Hu3DGLightCreateV(&pos, &dir, &color);
    Hu3DGLightStaticSet(light, TRUE);
    Hu3DGLightInfinitytSet(light);
    fn_1_378();
    lbl_1_bss_0 = 0;
    MgSeqModeNext();
}

void fn_1_1EC(s16 mode, s16 frameNo)
{
    lbl_1_bss_0++;
    if ((float)lbl_1_bss_0 >= 30.0f) {
        fn_1_644();
    }
    if (lbl_1_bss_0 >= 240 - fn_1_1CD8()) {
        fn_1_1B40(1);
    }
}

void fn_1_294(s16 mode, s16 frameNo)
{
    fn_1_84C();
}

void fn_1_2B4(s16 mode, s16 frameNo)
{
    if (fn_1_570()) {
        lbl_1_bss_0 = 0;
        MgSeqModeNext();
    }
}

void fn_1_2F0(s16 mode, s16 frameNo)
{
    fn_1_910();
}

void fn_1_310(s16 mode, s16 frameNo)
{
    if (fn_1_DA8() == -1) {
        MgSeqModeNext();
    }
}

void fn_1_33C(s16 mode, s16 frameNo)
{
}

void fn_1_340(s16 mode, s16 frameNo)
{
}

void fn_1_344(s16 mode, s16 frameNo)
{
    HuAudAllStop();
    Hu3DModelAllKill();
    Hu3DLightAllKill();
    omOvlReturnEx(1, 1);
}
