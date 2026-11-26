/* Loads the instruction screen's stage preview models and motions. */
#include <math.h>
#include "dolphin.h"
#include "dolphin/gx.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/frand.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/main.h"
#include "game/mgdata.h"
#include "game/object.h"
#include "game/process.h"
#include "game/window.h"
#include "datadir_enum.h"
#include "game/charman.h"
#include "game/thpmain.h"
#include "game/wipe.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "dolphin/os.h"
#include "humath.h"
#include "REL/instDll.h"

typedef struct {
    s16 characterMotionIds[10]; /* Loaded character motions used by the preview. */
} InstDllMotionRecord;

/* Shared instruction-screen state and resources. */

/* Stage preview models and motions, in the order used by the instruction screen. */
STAGE_MODEL lbl_1_data_520[] = {
    { DATANUM(0x260000, 26), -1, { -206.0f, -2.1e+02f, 302.0f }, { 0.0f, 15.0f, 0.0f }, { 0.9f,
        0.9f, 0.9f }, 2 },
    { DATANUM(0x260000, 27), 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f },
        2 },
    { DATANUM(0x260000, 28), 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f },
        2 },
    { DATANUM(0x260000, 29), 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f },
        2 },
    { DATANUM(0x260000, 30), -1, { -206.0f, -2.1e+02f, 302.0f }, { 0.0f, 15.0f, 0.0f }, { 0.9f,
        0.9f, 0.9f }, 2 },
    { DATANUM(0x260000, 31), 4, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f },
        2 },
    { DATANUM(0x260000, 32), 4, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f },
        2 },
    { DATANUM(0x260000, 33), 4, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f },
        2 },
    { DATANUM(0x260000, 34), -1, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f },
        2 },
    { -1, -1, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 2 }
};

extern HU3D_MODELID lbl_1_bss_6AA[256];
extern HU3D_MOTIONID lbl_1_bss_4AA[256];
/* Called by fn_1_1024 during setup to load the stage preview models and motions. */
void fn_1_4668(STAGE_MODEL *stageModels)
{
    STAGE_MODEL *stageModel;
    s16 stageModelIndex;

    stageModel = stageModels;
    stageModelIndex = 0;
    while (stageModel->modelDataId != -1) {
        if (stageModel->sourceModelIndex == -1) {
            lbl_1_bss_6AA[stageModelIndex] =
                fn_80025A04( fn_80008DD0(stageModel->modelDataId, 0x10000000, 2));
            fn_80026790(lbl_1_bss_6AA[stageModelIndex], &stageModel->position);
            fn_80026838(lbl_1_bss_6AA[stageModelIndex], &stageModel->rotation);
            fn_800268E0(lbl_1_bss_6AA[stageModelIndex], &stageModel->scale);
            fn_80026B64(lbl_1_bss_6AA[stageModelIndex], stageModel->cameraMask);
            fn_80026B8C(lbl_1_bss_6AA[stageModelIndex], 1);
        } else {
            /* Motion records refer back to a model loaded earlier in the table. */
            lbl_1_bss_4AA[stageModelIndex] = fn_80030458(
                lbl_1_bss_6AA[stageModel->sourceModelIndex],
                fn_80008DD0(stageModel->modelDataId, 0x10000000, 2));
        }
        stageModel++;
        stageModelIndex++;
    }
}
