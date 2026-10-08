/* Camera and character-motion helpers shared by M616's sequence callbacks. */
#include "REL/m616dll.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/pad.h"
#include "game/frand.h"
#include "datadir_enum.h"
#include <string.h>

typedef struct M616CpuParam {
    u32 unreadCpuSetting; /* CPU setting stored with the difficulty data but not read by this
                           * game. */
    u32 repeatPercent; /* Chance, from 0 to 100, of repeating the prior answer. */
} M616CpuParam;
extern MGSEQ_PARAM lbl_1_data_0;
extern s32 lbl_1_data_1F0[17];
extern s32 lbl_1_data_234[6];
extern s32 lbl_1_data_24C[13];
extern unsigned int lbl_1_data_280[12];
extern M616CpuParam lbl_1_data_2B0[4];
extern s32 lbl_1_data_2D0[3];

M616Work lbl_1_bss_10;
/* Called by sequence setup and camera changes to replace the active camera motion and reset its
 * elapsed time. */
void fn_1_1FF4(s32 cameraIndex)
{
    HU3D_MODELID cameraModel;
    HU3D_MOTIONID cameraMotion;

    cameraMotion = lbl_1_bss_10.cameraMotions[cameraIndex];
    cameraModel = Hu3DModelCameraCreate(cameraMotion, HU3D_CAM0);
    Hu3DCameraMotionStart(cameraModel, HU3D_CAM0);
    lbl_1_bss_10.cameraTime = 0.0f;
    lbl_1_bss_10.cameraMaxTime = Hu3DMotionMaxTimeGet(cameraModel);
    if (lbl_1_bss_10.activeCameraModel != HU3D_MODELID_NONE) {
        Hu3DModelKill(lbl_1_bss_10.activeCameraModel);
    }
    lbl_1_bss_10.activeCameraModel = cameraModel;
    OSReport("start camera motion ... idx:%d time:%f\n", cameraIndex, lbl_1_bss_10.cameraMaxTime);
}

/* Used by the opening callback to test whether the active camera motion has ended. */
BOOL fn_1_20D8(void)
{
    return Hu3DMotionEndCheck(lbl_1_bss_10.activeCameraModel);
}

/* Called by initialization and result callbacks to set a seat's shared player display motion. */
void fn_1_2104(s32 playerNo, s32 motionNo)
{
    Hu3DMotionSet(lbl_1_bss_10.playerModels[playerNo], lbl_1_bss_10.playerMotions[motionNo]);
}

/* Called by sequence callbacks to change a player's base pose; motion 12 is always restarted. */
void fn_1_215C(s32 playerNo, s32 motionNo)
{
    if (motionNo == 12 || lbl_1_bss_10.playerBaseMotions[playerNo][motionNo]
        != Hu3DMotionIDGet(lbl_1_bss_10.playerBaseModels[playerNo])) {
        Hu3DMotionSet(lbl_1_bss_10.playerBaseModels[playerNo],
            lbl_1_bss_10.playerBaseMotions[playerNo][motionNo]);
        Hu3DModelAttrSet(lbl_1_bss_10.playerBaseModels[playerNo], HU3D_MOTATTR_LOOP);
        OSReport("base motion time ... %f\n",
            Hu3DMotionMaxTimeGet(lbl_1_bss_10.playerBaseModels[playerNo]));
    }
}

/* Called when a round or final result is presented to set the center gear motion. */
void fn_1_2254(s32 motionNo)
{
    Hu3DMotionSet(lbl_1_bss_10.centerModel, lbl_1_bss_10.centerMotions[motionNo]);
    OSReport("gear motion time ... %f\n", Hu3DMotionMaxTimeGet(lbl_1_bss_10.centerModel));
}

/* Called by round callbacks to shift a character when its action changes; motion 10 always starts a
 * new shift. */
void fn_1_22BC(s32 playerNo, s32 motionNo, u32 motionAttributes)
{
    HU3D_MOTIONID currentMotion;
    float shiftStartFrame = 0.0f;

    currentMotion = Hu3DMotionIDGet(lbl_1_bss_10.characterModels[playerNo]);
    if (motionNo == 10 || currentMotion != lbl_1_bss_10.characterMotions[playerNo][motionNo]) {
        CharMotionShiftSet(lbl_1_bss_10.characterNos[playerNo],
                           lbl_1_bss_10.characterMotions[playerNo][motionNo], shiftStartFrame, 8.0f,
                           motionAttributes);
    }
}
