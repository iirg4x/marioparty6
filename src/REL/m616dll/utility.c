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
    u32 unk_00;
    u32 repeatPercent;
} M616CpuParam;
extern MGSEQ_PARAM lbl_1_data_0;
extern s32 lbl_1_data_1F0[17];
extern s32 lbl_1_data_234[6];
extern s32 lbl_1_data_24C[13];
extern unsigned int lbl_1_data_280[12];
extern M616CpuParam lbl_1_data_2B0[4];
extern s32 lbl_1_data_2D0[3];

M616Work lbl_1_bss_10;
void fn_1_1FF4(s32 index)
{
    HU3D_MODELID model;
    HU3D_MOTIONID motion;

    motion = lbl_1_bss_10.cameraMotions[index];
    model = Hu3DModelCameraCreate(motion, HU3D_CAM0);
    Hu3DCameraMotionStart(model, HU3D_CAM0);
    lbl_1_bss_10.cameraTime = 0.0f;
    lbl_1_bss_10.cameraMaxTime = Hu3DMotionMaxTimeGet(model);
    if (lbl_1_bss_10.activeCameraModel != HU3D_MODELID_NONE) {
        Hu3DModelKill(lbl_1_bss_10.activeCameraModel);
    }
    lbl_1_bss_10.activeCameraModel = model;
    OSReport("start camera motion ... idx:%d time:%f\n", index, lbl_1_bss_10.cameraMaxTime);
}

BOOL fn_1_20D8(void)
{
    return Hu3DMotionEndCheck(lbl_1_bss_10.activeCameraModel);
}

void fn_1_2104(s32 playerNo, s32 motionNo)
{
    Hu3DMotionSet(lbl_1_bss_10.playerModels[playerNo], lbl_1_bss_10.playerMotions[motionNo]);
}

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

void fn_1_2254(s32 motionNo)
{
    Hu3DMotionSet(lbl_1_bss_10.centerModel, lbl_1_bss_10.centerMotions[motionNo]);
    OSReport("gear motion time ... %f\n", Hu3DMotionMaxTimeGet(lbl_1_bss_10.centerModel));
}

void fn_1_22BC(s32 playerNo, s32 motionNo, u32 attr)
{
    HU3D_MOTIONID motion;
    float start = 0.0f;

    motion = Hu3DMotionIDGet(lbl_1_bss_10.characterModels[playerNo]);
    if (motionNo == 10 || motion != lbl_1_bss_10.characterMotions[playerNo][motionNo]) {
        CharMotionShiftSet(lbl_1_bss_10.characterNos[playerNo],
            lbl_1_bss_10.characterMotions[playerNo][motionNo], start, 8.0f, attr);
    }
}
