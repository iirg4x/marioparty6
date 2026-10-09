/* Defines Lift Leapers sequence callbacks and their shared setup state. */
#include "REL/m618dll.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/hsfex.h"

HUPROCESS *lbl_1_bss_4;
int lbl_1_bss_0;

/* Sequence timing and callbacks used to run Lift Leapers from setup through return. */
MGSEQ_PARAM lbl_1_data_0 = {
    300, 1, fn_1_DC, fn_1_1EC, fn_1_294, fn_1_2B4, fn_1_2F0,
    fn_1_310, fn_1_33C, fn_1_340, fn_1_344
};

/* Module entry point: creates the minigame actor manager and starts the sequence. */
void fn_1_A0(void)
{
    lbl_1_bss_4 = MgActorObjectSetup();
    MgSeqCreatePrio(&lbl_1_data_0, 8192);
}

/* Sequence init hook: creates four camera views and the course light, then prepares play. */
void fn_1_DC(s16 sequenceMode, s16 frameNo)
{
    HuVecF pos = { 0.0f, 1000.0f, 5000.0f };
    HuVecF dir = { 0.0f, -1.0f, -1.0f };
    GXColor color = { 255, 255, 255, 255 };
    OMOBJ *obj;
    HU3D_LIGHTID light;

    Hu3DCameraCreate(HU3D_CAM0 | HU3D_CAM1 | HU3D_CAM2 | HU3D_CAM3);
    obj = omAddObjEx(lbl_1_bss_4, OM_OUTVIEW_PRIO, 0, 0, -1, omOutViewMulti);
    obj->work[0] = 4;
    light = Hu3DGLightCreateV(&pos, &dir, &color);
    Hu3DGLightStaticSet(light, TRUE);
    Hu3DGLightInfinitytSet(light);
    fn_1_378();
    lbl_1_bss_0 = 0;
    MgSeqModeNext();
}

/* Sequence fade-in hook: starts player movement at callback 30 and begins the four-view split
 * transition at callback 180, updating it on later calls. */
void fn_1_1EC(s16 sequenceMode, s16 frameNo)
{
    lbl_1_bss_0++;
    if ((float)lbl_1_bss_0 >= 30.0f) {
        fn_1_644();
    }
    if (lbl_1_bss_0 >= 240 - fn_1_1CD8()) {
        fn_1_1B40(1);
    }
}

/* Sequence start hook: advances the opening wait, then updates actors and CPU input each frame. */
void fn_1_294(s16 sequenceMode, s16 frameNo)
{
    fn_1_84C();
}

/* Sequence main hook: updates gameplay and advances after the first player finishes. */
void fn_1_2B4(s16 sequenceMode, s16 frameNo)
{
    if (fn_1_570()) {
        lbl_1_bss_0 = 0;
        MgSeqModeNext();
    }
}

/* Sequence finish hook: records the winner or draw, then settles players before result poses. */
void fn_1_2F0(s16 sequenceMode, s16 frameNo)
{
    fn_1_910();
}

/* Sequence pre-winner hook: advances after winner or draw presentation is ready. */
void fn_1_310(s16 sequenceMode, s16 frameNo)
{
    if (fn_1_DA8() == -1) {
        MgSeqModeNext();
    }
}

/* Winner hook: no additional work is needed while the result pose is shown. */
void fn_1_33C(s16 sequenceMode, s16 frameNo)
{
}

/* Fade-out hook: no additional per-frame work is needed while the screen fades. */
void fn_1_340(s16 sequenceMode, s16 frameNo)
{
}

/* Close callback: stops audio, removes models and lights, then returns from the minigame. */
void fn_1_344(s16 sequenceMode, s16 frameNo)
{
    HuAudAllStop();
    Hu3DModelAllKill();
    Hu3DLightAllKill();
    omOvlReturnEx(1, 1);
}
