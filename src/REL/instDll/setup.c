/* Sets up and runs the instruction screen for the selected minigame. */
#include "dolphin.h"
#include "messdir_enum.h"
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

/* Limits, timing values, priorities, and input masks for the instruction screen. */
enum {
    INST_MINIGAME_COUNT = 82,
    INST_MINIGAME_NUMBER_BASE = 601,
    INST_AUX_OBJECT_PRIORITY = 32730,
    INST_WIPE_FRAMES = 40,
    INST_INPUT_WAIT_FRAMES = 20,
    INST_SUBSTICK_STEP_MASK = (u8)~7
};

typedef struct StageModel_s {
    u32 dataNum; /* Stage-model or joint-motion resource number. */
    s16 motMdl; /* -1 creates a model; otherwise indexes the model receiving the joint motion. */
    Vec pos; /* Preview position. */
    Vec rot; /* Rotation in degrees. */
    Vec scale; /* Per-axis scale. */
    s16 cameraBit; /* Camera-selection mask. */
} STAGE_MODEL;
typedef struct Inst_data_798_view {
    u8 stageModelCount;
    u32 stageModelDataIds[4];
    f32 orbitStartAngleDegrees;
    f32 previewModelScale;
    f32 characterFacingDegrees[5];
    u8 stagePlayerMasks[4];
    Vec characterPositions[5];
} Inst_data_798_view;
typedef struct Inst_bss_D8_view {
    HU3D_MODELID stageModel;
    u16 spinFramesRemaining;
    f32 spinAngleDegrees;
    u8 unusedStorage[12];
    s8 playerIndices[5];
    Vec playerPositions[5];
} Inst_bss_D8_view;
typedef struct InstDllMotionRecord {
    s16 motionIds[10];
} InstDllMotionRecord;

/* Shared instruction-screen positions, stage-preview data, and resource text. */
extern HuVecF lbl_1_data_70C;
extern HU3D_MOTIONID lbl_1_bss_4AA[];
extern HU3D_MODELID lbl_1_bss_6AA[];
extern u8 lbl_1_data_410[];
extern STAGE_MODEL lbl_1_data_520[];
extern HuVecF lbl_1_data_700;
extern HuVecF lbl_1_data_718;
extern HuVecF lbl_1_data_724;
extern Inst_data_798_view lbl_1_data_798[9];
extern u32 lbl_1_data_F8[];
extern GXRenderModeObj *RenderMode;

/* Initial stage-preview tint and camera views. */
GXColor lbl_1_data_0 = { 20, 0, 0, 0 };
BOOL lbl_1_data_4 = TRUE;
OM_CAMERA_VIEW lbl_1_data_8 = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 7e+03f };
OM_CAMERA_VIEW lbl_1_data_24 = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 3e+03f };
OM_CAMERA_VIEW lbl_1_data_40 = { { 1.7e+02f, 105.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 1e+02f };
extern HU3D_MODELID lbl_1_bss_49E[6];
extern InstDllMotionRecord lbl_1_bss_43A[5];
extern s16 lbl_1_bss_438;
extern HUSPR_GROUPID lbl_1_bss_238[256];
extern Inst_bss_D8_view lbl_1_bss_D8[4];
extern s16 lbl_1_bss_D4;
extern BOOL lbl_1_bss_84[20];
extern s16 lbl_1_bss_78[6];
extern ANIMDATA * lbl_1_bss_70[2];
extern s16 lbl_1_bss_6E;
extern HU3D_MODELID lbl_1_bss_6C;
extern HUSPR_GROUPID lbl_1_bss_66[3];
extern ANIMDATA * lbl_1_bss_5C[2];
extern ANIMDATA * lbl_1_bss_58;
extern ANIMDATA * lbl_1_bss_50[2];
extern void * lbl_1_bss_4C;
extern f32 lbl_1_bss_48;
OMOBJ * lbl_1_bss_40;
OMOBJMAN * lbl_1_bss_3C;
u16 lbl_1_bss_38;
u16 lbl_1_bss_36;
u8 lbl_1_bss_34;
s32 lbl_1_bss_30;
s16 lbl_1_bss_28[4];
s16 lbl_1_bss_26;
s16 lbl_1_bss_24;
MGDATA *lbl_1_bss_20;
s16 lbl_1_bss_1C;
u32 * lbl_1_bss_18;
s16 lbl_1_bss_14;
s32 lbl_1_bss_10;
s32 lbl_1_bss_C;
s16 lbl_1_bss_A;
s16 lbl_1_bss_8;
s16 lbl_1_bss_6;
u16 lbl_1_bss_4;
BOOL lbl_1_bss_0;
/* Instruction-screen setup routines. */

/* Initializes minigame state and launches either the instruction screen or the selected
 * minigame. */
void fn_1_A0(void)
{
    s32 i;
    s32 count;
        OMOVLHIS *history;
    int lightId;
    OMOBJ *obj;
    s32 languageNo;
    s16 initialNightMode;
    s32 selectedSubGameNo;
    s16 nightMode;

    OSReport("******* INST ObjectSetup *********\n");
    lbl_1_bss_3C = fn_8003A8BC(50, 8192);
    languageNo = lbl_80265E30.languageNo;
    lbl_1_bss_34 = languageNo;
    lbl_1_bss_36 = lbl_80265B70.mgNo;
    if (lbl_1_bss_36 >= INST_MINIGAME_COUNT) {
        lbl_1_bss_36 = 0;
    }
    fn_8003CCAC(lbl_1_bss_36 + INST_MINIGAME_NUMBER_BASE);
    lbl_1_bss_20 = &MgDataTbl[lbl_1_bss_36];
    /* Rare minigames use the last-player preview type. */
    if (lbl_1_bss_20->flag & MG_FLAG_RARE) {
        lbl_1_bss_20->type = MG_TYPE_LAST;
    }

    if (fn_8006A124(196610)) {
        lbl_1_bss_18 = &lbl_1_bss_20->instMes[2][0];
    } else {
        initialNightMode = lbl_802C0278;
        if (initialNightMode == 0) {
        lbl_1_bss_18 = &lbl_1_bss_20->instMes[0][0];
    } else {
        lbl_1_bss_18 = &lbl_1_bss_20->instMes[1][0];
        }
    }

    lbl_1_bss_38 = 0;
    lbl_1_bss_4 = 0;
    fn_8006A268(65547);
    i = 0;
    while (sndGrpTable[i].ovl != -1) {
        if (sndGrpTable[i].ovl == MgDataTbl[lbl_1_bss_36].ovl) {
            break;
        }
        i++;
    }
    if (sndGrpTable[i].ovl != -1) {
        HuAudSndGrpSetSet(sndGrpTable[i].grpSet);
    }

    if (!fn_8006A124(196609) && !fn_8006A124(65551)) {
        lbl_80265B70.subGameNo = -1;
        MgSubMode = 0;
    }
    selectedSubGameNo = lbl_80265B70.subGameNo;
    lbl_1_bss_14 = selectedSubGameNo;
    if (lbl_1_bss_14 < 0 ||
        lbl_1_bss_20->instPic[0][lbl_1_bss_14] == 0) {
        lbl_1_bss_14 = 0;
    }

    /* m678 selects 1-vs-3 or 2-vs-2 instructions from group-0 size; malformed group counts are
     * reset. */
    if (lbl_1_bss_20->ovl == 83) {
        for (i = count = 0; i < 4; i++) {
            if (lbl_80265728[i].grpNo == 0) {
                count++;
            }
        }
        if (count == 0 || count >= 3) {
            lbl_80265728[0].grpNo = 0;
            lbl_80265728[1].grpNo = lbl_80265728[2].grpNo = lbl_80265728[3].grpNo = 1;
            lbl_80265B70.subGameNo = 0;
            lbl_1_bss_14 = 0;
        } else if (count == 1) {
            lbl_80265B70.subGameNo = 0;
            lbl_1_bss_14 = 0;
        } else {
            lbl_80265B70.subGameNo = 1;
            lbl_1_bss_14 = 1;
            lbl_1_bss_18 = &lbl_1_bss_20->instMes[1][0];
        }
    }

    if (omprevovl >= 6 && omprevovl <= 87 &&
        !fn_8006A124(65551) && fn_8006A124(196609)) {
        goto prior_game_path;
    }
    if (lbl_802C0248 == 2) {
prior_game_path:
        if (lbl_1_bss_20->flag & MG_FLAG_GRPORDER) {
            for (i = 0; i < 4; i++) {
                lbl_80265728[i].grpNo = lbl_80265728[i].grpNo / 2;
            }
        }
        fn_800097F4(MgDataTbl[lbl_1_bss_36].dataDir);
        if (fn_8006A124(196612)) {
            fn_8000DAC0(fn_1_4158, 100, 12288, 0, fn_8000DD9C());
            return;
        } else {
            fn_8003A5E4(1, 1);
        }
        return;
    }

    fn_8006A268(65551);
    if (!fn_8006A124(196609) &&
        !fn_8006A124(196611)) {
        history = omOvlHisGet(0);
        if (lbl_1_bss_20->type != MG_TYPE_4P &&
            lbl_1_bss_20->type != MG_TYPE_1VS3 &&
            lbl_1_bss_20->type != MG_TYPE_2VS2 &&
            lbl_1_bss_20->type != MG_TYPE_BATTLE) {
            omOvlHisChg(0, 4, 2, history->stat);
        } else {
            omOvlHisChg(0, 113, history->evtno, history->stat);
        }
    }

    if (lbl_1_bss_20->type == MG_TYPE_4P) {
        for (i = 0; i < 4; i++) {
            lbl_80265728[i].grpNo = i;
        }
    }
    if (lbl_802C0248 == 0) {
        s32 groupPlayers[4][4];
        s32 groupCount[4];
    /* Randomize the display order within each two-player team. */
    if (lbl_1_bss_20->flag & MG_FLAG_GRPORDER) {
        groupCount[0] = groupCount[1] = 0;
        for (i = 0; i < 4; i++) {
            if (lbl_80265728[i].grpNo >= 2) {
                break;
            }
        }
        if (i != 4) {
            goto checked_player_groups;
        }
        for (i = 0; i < 4; i++) {
            groupPlayers[lbl_80265728[i].grpNo][groupCount[lbl_80265728[i].grpNo]++] = i;
        }
        if ((s32)frand() & 1) {
            lbl_80265728[groupPlayers[0][0]].grpNo = 0;
            lbl_80265728[groupPlayers[0][1]].grpNo = 1;
        } else {
            lbl_80265728[groupPlayers[0][0]].grpNo = 1;
            lbl_80265728[groupPlayers[0][1]].grpNo = 0;
        }
        if ((s32)frand() & 1) {
            lbl_80265728[groupPlayers[1][0]].grpNo = 2;
            lbl_80265728[groupPlayers[1][1]].grpNo = 3;
        } else {
            lbl_80265728[groupPlayers[1][0]].grpNo = 3;
            lbl_80265728[groupPlayers[1][1]].grpNo = 2;
        }
        goto checked_player_groups;
    }

    if (MgDataTbl[lbl_1_bss_36].type == MG_TYPE_1VS3) {
        groupCount[0] = groupCount[1] = 0;
        for (i = 0; i < 4; i++) {
            if (lbl_80265728[i].grpNo > 1) {
                break;
            }
            if (lbl_80265728[i].grpNo < 0) {
                break;
            }
            groupCount[lbl_80265728[i].grpNo]++;
        }
        if (i != 4 || groupCount[0] != 1 || groupCount[1] != 3) {
            OSReport("Error: Illegal Group pattern.Adjusted.\n");
            lbl_80265728[0].grpNo = 0;
            lbl_80265728[1].grpNo = lbl_80265728[2].grpNo = lbl_80265728[3].grpNo = 1;
        }
    } else if (MgDataTbl[lbl_1_bss_36].type == MG_TYPE_2VS2) {
        groupCount[0] = groupCount[1] = 0;
        for (i = 0; i < 4; i++) {
            if (lbl_80265728[i].grpNo > 1) {
                break;
            }
            if (lbl_80265728[i].grpNo < 0) {
                break;
            }
            groupCount[lbl_80265728[i].grpNo]++;
        }
        if (i != 4 || groupCount[0] != 2 || groupCount[1] != 2) {
            OSReport("Error: Illegal Group pattern.Adjusted.\n");
            lbl_80265728[0].grpNo = lbl_80265728[1].grpNo = 0;
            lbl_80265728[2].grpNo = lbl_80265728[3].grpNo = 1;
        }
    }

    }

checked_player_groups:
    if (!fn_8006A124(5) || !GWMgInstDispGet()) {
        fn_8003D3B0(1);
        fn_800097F4(0x260000);
        omSysPauseEnable(1);
        omOvlCallEx(MgDataTbl[lbl_1_bss_36].ovl, 1, 0, 0);
        return;
    }

    fn_800097F4(MgDataTbl[lbl_1_bss_36].dataDir);
    omSysPauseEnable(0);
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(1, 5.0f, 20.0f,
                             15000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
                          640.0f, 480.0f,
                          0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(2, 10.0f, 20.0f,
                             15000.0f, 1.2f);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f,
                          640.0f, 480.0f,
                          0.0f, 1.0f);
    fn_8003E488(1, &lbl_1_data_8);
    fn_8003E488(2, &lbl_1_data_24);
    lightId = Hu3DGLightCreate(0.0f, 100.0f,
                               1000.0f, 0.0f,
                               -0.5f, -1.0f,
                               255, 255, 255);
    Hu3DGLightInfinitytSet(lightId);
    lbl_1_bss_40 = fn_8003AB38(lbl_1_bss_3C, INST_AUX_OBJECT_PRIORITY, 0, 0, -1,
                              fn_8003DAA4);
    lbl_1_bss_40->work[0] = 2;
    fn_8003AB38(lbl_1_bss_3C, 0, 32, 32, -1, fn_1_35F8);
    HuAudSStreamStop(0);
    HuAudSStreamPlay(MSM_STREAM_WARMING_UP);
    Hu3DBGColorSet(0, 0, 0);
    HuWinInit(1);
    fn_8000DAC0(fn_1_1024, 1000, 12288, 0, lbl_1_bss_3C);
    for (i = 0; i < 4; i++) {
        lbl_1_bss_28[i] = lbl_80265728[i].charNo;
    }
    {
    HuVecF shadowPos;
    HuVecF shadowUp;
    HuVecF shadowDir;
    Hu3DShadowCreate(15.0f, 20.0f, 5000.0f);
    shadowPos = lbl_1_data_700;
    shadowPos.y += 1000.0f;
    shadowPos.z += 1000.0f;
    shadowUp.y = 1.0f;
    shadowUp.x = shadowUp.z = 0.0f;
    shadowDir = lbl_1_data_700;
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowDir);
    Hu3DShadowTPLvlSet(0.3f);
    }
    if (omprevovl >= 6 && omprevovl <= 87) {
        lbl_1_bss_6 = 2;
    } else {
        lbl_1_bss_6 = 0;
    }
    nightMode = lbl_802C0278;
    lbl_1_bss_1C = nightMode;
    if (((lbl_1_bss_20->flag & MG_FLAG_DISABLE_DAY) ||
         lbl_1_bss_20->type == MG_TYPE_KUPA) && MgInstExitF) {
        lbl_1_bss_1C = 1;
    }
    return;
}

u32 lbl_1_data_A8[] = { DATANUM(0x420000, 2), DATANUM(0x420000, 3), DATANUM(0x420000, 4),
                        DATANUM(0x420000, 5), DATANUM(0x420000, 6) };

/* Runs the instruction screen as the child process started by fn_1_A0. */
void fn_1_1024(void)
{
    s32 flag_1;
    s32 flag_2;
    s16 i;
    s32 value;
    OMOVLHIS *history;

    flag_1 = 0;
    fn_1_8FA0(lbl_1_bss_1C);
    fn_1_96F4(lbl_1_bss_1C);
    fn_1_4668(lbl_1_data_520);
    fn_1_539C();
    fn_1_5DE0();
    fn_1_47E4(lbl_1_bss_1C);
    fn_1_A1E8(fn_1_2A1C, lbl_1_bss_1C);
    fn_8000DAC0(fn_1_15C8, 100, 12288, 0, fn_8000DD9C());
    HuPrcVSleep();
    fn_8000DAC0(fn_1_4188, 2000, 12288, 0, fn_8000DD9C());
    fn_800097F4(0x260000);
    fn_8005B7BC(-1);
    WipeCreate(1, 5, INST_WIPE_FRAMES);
    while (WipeCheck()) {
        HuPrcVSleep();
    }

    if (lbl_1_bss_6 != 2) {
        lbl_1_bss_6 = 1;
        HuPrcSleep(30);
    }
    lbl_1_bss_6 = 2;

    flag_2 = 0;
    if (lbl_1_bss_20->type != MG_TYPE_KETTOU) {
        i = 0;
        while (i < 4) {
            if (lbl_80265728[i].type == 0) {
                break;
            }
            i++;
        }
        if (i == 4) {
            flag_2 = 1;
        }
    } else {
        i = 0;
        while (i < 4) {
            if (lbl_80265728[i].grpNo == 0 || lbl_80265728[i].grpNo == 1) {
                if (lbl_80265728[i].type == 0) {
                    break;
                }
            }
            i++;
        }
        if (i == 4) {
            flag_2 = 1;
        }
    }

    lbl_1_bss_30 = fn_8006A124(196612) ? 0 : 1;

    while (1) {
        fn_1_8DB0();
        if (lbl_1_bss_8 == 0) {
            if (lbl_1_bss_24 & PAD_BUTTON_START) {
                HuAudFXPlay(2);
                fn_8003D3B0(1);
                goto finish;
            }
            if (lbl_1_bss_24 & PAD_TRIGGER_Z) {
                HuAudFXPlay(2);
                history = omOvlHisGet(0);
                omOvlHisChg(0, 4, 1, history->stat);
                fn_8006A1C4(65551);
                goto finish;
            }
        }

        if (MgInstExitF) {
            if (lbl_1_bss_8 == 0 && (lbl_1_bss_24 & PAD_BUTTON_B)) {
                HuAudFXPlay(3);
                flag_1 = 1;
                goto finish;
            }
            if (lbl_1_bss_30 == 0 && (lbl_1_bss_20->flag & MG_FLAG_HAS_NIGHT) &&
                (lbl_1_bss_24 & PAD_BUTTON_X) && lbl_1_bss_8 == 0 && lbl_1_bss_C == 0) {
                fn_1_15B4();
                HuAudFXPlay(1);
            }
            if (lbl_1_bss_8 != 0) {
                lbl_1_bss_8--;
                if (lbl_1_bss_8 == 0) {
                    lbl_1_bss_1C = (lbl_1_bss_1C == 0) ? 1 : 0;
                    value = lbl_1_bss_1C;
                    lbl_802C0278 = value;
                }
            }
        }

        if (flag_2 != 0) {
            HuPrcSleep(60);
            break;
        }
        HuPrcVSleep();
    }

finish:
    if (flag_1 == 0) {
        lbl_1_bss_6 = 3;
        fn_8003E62C(2, &lbl_1_data_40, 60, 1);
        HuPrcSleep(20);
        HuAudFXPlay(56);
    }
    WipeCreate(2, 0, INST_WIPE_FRAMES);
    HuAudSStreamAllFadeOut(1000);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    HuTHPClose();
    while (HuTHPProcCheck()) {
        HuTHPClose();
        HuPrcVSleep();
    }
    if (flag_1 == 0) {
        while (lbl_1_data_4 == 0) {
            HuPrcVSleep();
        }
        omOvlCallEx(lbl_1_bss_20->ovl, 1, 0, 0);
    } else {
        if (fn_80008270(lbl_1_bss_A) == 0) {
            fn_80007DE4(lbl_1_bss_A);
        } else {
            fn_800097F4(MgDataTbl[lbl_1_bss_36].dataDir);
        }
        fn_8003A5E4(1, 1);
    }
    HuPrcEnd();
    while (1) {
        HuPrcVSleep();
    }
}

void fn_1_15B4(void) {
    lbl_1_bss_8 = INST_INPUT_WAIT_FRAMES;
}

/* Displays instruction pages, inserts player names by group, and switches between day/night message
 * sets. */
void fn_1_15C8(void)
{
    s16 i;
    s16 pageNo;
    s16 j;
    u32 ctrlMes;
    s16 k;
    u32 instMes;
    s16 insertMesNo;
    s16 night;
    s16 frame;
    s16 frameMax;
    float t;
    s16 charMes[4][4];
    HUWINID winTbl[2][4];
    HuVec2f instSize;
    HuVec2f ctrlSize;
    HuVec2f instPos;
    HuVec2f ctrlPos;
    s16 grpNum[4];
    HUWINID instWinId[2];
    HUWINID ctrlWinId[2];

    night = lbl_1_bss_1C;
    if (night == 0) {
        frame = 4;
    } else {
        frame = 3;
    }
    for (j=0; j<2; j++) {
        if (j == 0) {
            frame = 4;
        } else {
            frame = 3;
        }
        for (i=0; i<2; i++) {
            instSize.x = 400;
            instSize.y = 128;
            instPos.x = 560-instSize.x;
            instPos.y = 440-instSize.y;
            instWinId[i] = HuWinCreate(instPos.x, instPos.y, instSize.x, instSize.y, frame);
            HuWinMesSpeedSet(instWinId[i], 0);
            HuWinMesSet(instWinId[i], lbl_1_bss_18[0]);
            ctrlSize.x = 336;
            ctrlSize.y = 48;
            ctrlPos.x = 560-ctrlSize.x;
            ctrlPos.y = 440-instSize.y-ctrlSize.y;
            ctrlWinId[i] = HuWinCreate(ctrlPos.x, ctrlPos.y, ctrlSize.x, ctrlSize.y, frame);
            HuWinAttrSet(ctrlWinId[i], HUWIN_ATTR_ALIGN_CENTER);
            HuWinMesSpeedSet(ctrlWinId[i], 0);
            HuWinMesSet(ctrlWinId[i], lbl_1_data_A8[0]);
        }
        winTbl[j][0] = instWinId[0];
        winTbl[j][1] = ctrlWinId[0];
        winTbl[j][2] = instWinId[1];
        winTbl[j][3] = ctrlWinId[1];
        for (i=0; i<4; i++) {
            HuWinDispOff(winTbl[j][i]);
        }
    }
    if (night == 0) {
        instWinId[0] = winTbl[0][0];
        ctrlWinId[0] = winTbl[0][1];
        instWinId[1] = winTbl[0][2];
        ctrlWinId[1] = winTbl[0][3];
        for (i=0; i<4; i++) {
            HuWinDispOn(winTbl[0][i]);
        }
    } else {
        instWinId[0] = winTbl[1][0];
        ctrlWinId[0] = winTbl[1][1];
        instWinId[1] = winTbl[1][2];
        ctrlWinId[1] = winTbl[1][3];
        for (i=0; i<4; i++) {
            HuWinDispOn(winTbl[1][i]);
        }
    }
    for (i=0; i<4; i++) {
        grpNum[i] = 0;
    }
    for (i=0; i<4; i++) {
        charMes[lbl_80265728[i].grpNo][grpNum[lbl_80265728[i].grpNo]] = lbl_80265728[i].charNo;
        grpNum[lbl_80265728[i].grpNo]++;
    }
    for (i=insertMesNo=0; i<4; i++) {
        for (j=0; j<grpNum[i]; j++) {
            HuWinInsertMesSet(winTbl[0][0], charMes[i][j], (s32)insertMesNo);
            HuWinInsertMesSet(winTbl[0][2], charMes[i][j], (s32)insertMesNo);
            HuWinInsertMesSet(winTbl[1][0], charMes[i][j], (s32)insertMesNo);
            HuWinInsertMesSet(winTbl[1][2], charMes[i][j], (s32)insertMesNo);
            insertMesNo++;
        }
    }
    HuWinDispOff(instWinId[1]);
    HuWinDispOff(ctrlWinId[1]);
    lbl_1_bss_C = 1;
    if (lbl_1_bss_6 == 0 || lbl_1_bss_6 == 1) {
        HuWinPosSet(ctrlWinId[0], 1000, 0);
        HuWinPosSet(instWinId[0], 1000, 0);
        while (lbl_1_bss_6 != 1) {
            HuPrcVSleep();
        }
        HuPrcSleep(20);
        for (i=0; i<=20; i++) {
            HuWinPosSet(ctrlWinId[0],
                ((1-sin(3.141592653589793*((i/20.0)*90)/180.0))*400)+ctrlPos.x, ctrlPos.y);
            HuWinPosSet(instWinId[0], instPos.x,
                instPos.y+((1-sin(3.141592653589793*((i/20.0)*90)/180.0))*200));
            HuPrcVSleep();
        }
    }
    pageNo = 0;
    ctrlMes = lbl_1_data_A8[0];
    instMes = lbl_1_bss_18[0];
    lbl_1_bss_C = 0;
    while (1) {
        if (lbl_1_bss_8 == 0) {
            if (lbl_1_bss_24 & (0x2000 | PAD_TRIGGER_R)) {
                lbl_1_bss_C = 1;
                lbl_1_bss_10 = 1;
                HuWinPriSet(ctrlWinId[0], 100);
                HuWinPriSet(instWinId[0], 100);
                HuWinPriSet(ctrlWinId[1], 50);
                HuWinPriSet(instWinId[1], 50);
                HuWinMesSet(ctrlWinId[1], ctrlMes);
                HuWinMesSet(instWinId[1], instMes);
                HuWinDispOn(instWinId[1]);
                HuWinDispOn(ctrlWinId[1]);
                HuWinPosSet(ctrlWinId[1], 1000, 0);
                HuWinPosSet(instWinId[1], 1000, 0);
                HuPrcVSleep();
                pageNo++;
                if (pageNo > 4) {
                    pageNo = 0;
                }
                if (pageNo == 2 && lbl_1_bss_18[2] == 0) {
                    pageNo++;
                }
                if (pageNo == 4 && lbl_1_bss_18[4] == 0) {
                    pageNo = 0;
                }
                ctrlMes = lbl_1_data_A8[pageNo];
                if (pageNo == 1 && lbl_1_bss_18[2] == 0) {
                    ctrlMes = MESSNUM(MESS_MG_INST_SYS, 7);
                }
                if (pageNo == 3 && lbl_1_bss_18[4] == 0) {
                    ctrlMes = MESSNUM(MESS_MG_INST_SYS, 8);
                }
                HuWinMesSet(ctrlWinId[0], ctrlMes);
                instMes = lbl_1_bss_18[pageNo];
                HuWinMesSet(instWinId[0], instMes);
                HuAudFXPlay(55);
                for (i=0; i<15; i++) {
                    t = i/15.0;
                    HuWinPosSet(ctrlWinId[1],
                        ctrlPos.x + (30 * sin(3.141592653589793*(t * 90)/180.0)),
                                ctrlPos.y + (200 * (1 - cos(3.141592653589793*(t * 90)/180.0))));
                    HuWinPosSet(instWinId[1],
                        instPos.x + (30 * sin(3.141592653589793*(t * 90)/180.0)),
                                instPos.y + (200 * (1 - cos(3.141592653589793*(t * 90)/180.0))));
                    HuPrcVSleep();
                }
                HuWinDispOff(instWinId[1]);
                HuWinDispOff(ctrlWinId[1]);
                lbl_1_bss_C = 0;
            } else if (lbl_1_bss_24 & (0x4000 | PAD_TRIGGER_L)) {
                lbl_1_bss_C = 1;
                lbl_1_bss_10 = 1;
                HuWinPriSet(ctrlWinId[0], 50);
                HuWinPriSet(instWinId[0], 50);
                HuWinPriSet(ctrlWinId[1], 100);
                HuWinPriSet(instWinId[1], 100);
                HuWinMesSet(ctrlWinId[1], ctrlMes);
                HuWinMesSet(instWinId[1], instMes);
                HuWinDispOn(instWinId[1]);
                HuWinDispOn(ctrlWinId[1]);
                HuWinPosSet(ctrlWinId[1], ctrlPos.x, ctrlPos.y);
                HuWinPosSet(instWinId[1], instPos.x, instPos.y);
                HuPrcVSleep();
                pageNo--;
                if (pageNo < 0) {
                    pageNo = 4;
                }
                if (pageNo == 2 && lbl_1_bss_18[2] == 0) {
                    pageNo--;
                }
                if (pageNo == 4 && lbl_1_bss_18[4] == 0) {
                    pageNo--;
                }
                ctrlMes = lbl_1_data_A8[pageNo];
                if (pageNo == 1 && lbl_1_bss_18[2] == 0) {
                    ctrlMes = MESSNUM(MESS_MG_INST_SYS, 7);
                }
                if (pageNo == 3 && lbl_1_bss_18[4] == 0) {
                    ctrlMes = MESSNUM(MESS_MG_INST_SYS, 8);
                }
                HuWinMesSet(ctrlWinId[0], ctrlMes);
                instMes = lbl_1_bss_18[pageNo];
                HuWinMesSet(instWinId[0], instMes);
                HuAudFXPlay(54);
                for (i=1; i<=15; i++) {
                    t = i/15.0;
                    HuWinPosSet(ctrlWinId[0],
                        ctrlPos.x + (30 * cos(3.141592653589793*(t * 90)/180.0)),
                                ctrlPos.y + (200 * (1 - sin(3.141592653589793*(t * 90)/180.0))));
                    HuWinPosSet(instWinId[0],
                        instPos.x + (30 * cos(3.141592653589793*(t * 90)/180.0)),
                                instPos.y + (200 * (1 - sin(3.141592653589793*(t * 90)/180.0))));
                    HuPrcVSleep();
                }
                HuWinDispOff(instWinId[1]);
                HuWinDispOff(ctrlWinId[1]);
                lbl_1_bss_C = 0;
            }
        }
        if (lbl_1_bss_8 != 0) {
            frameMax = 9;
            for (k=1; k<frameMax; k++) {
                t = 1.0f-((float)k/(float)frameMax);
                for (i=0; i<2; i++) {
                    HuWinBGTPLvlSet(instWinId[i], t);
                    HuWinBGTPLvlSet(ctrlWinId[i], t);
                }
                HuPrcVSleep();
            }
            night = night == 0 ? 1 : 0;
            if (night == 0) {
                for (i=0; i<4; i++) {
                    HuWinDispOff(winTbl[1][i]);
                }
                instWinId[0] = winTbl[0][0];
                ctrlWinId[0] = winTbl[0][1];
                instWinId[1] = winTbl[0][2];
                ctrlWinId[1] = winTbl[0][3];
                for (i=0; i<2; i++) {
                    HuWinBGTPLvlSet(winTbl[0][i], 0);
                    HuWinDispOn(winTbl[0][i]);
                    HuWinMesSet(ctrlWinId[i], ctrlMes);
                }
                lbl_1_bss_18 = lbl_1_bss_20->instMes[0];
            } else {
                for (i=0; i<4; i++) {
                    HuWinDispOff(winTbl[0][i]);
                }
                instWinId[0] = winTbl[1][0];
                ctrlWinId[0] = winTbl[1][1];
                instWinId[1] = winTbl[1][2];
                ctrlWinId[1] = winTbl[1][3];
                for (i=0; i<2; i++) {
                    HuWinBGTPLvlSet(winTbl[1][i], 0);
                    HuWinDispOn(winTbl[1][i]);
                    HuWinMesSet(ctrlWinId[i], ctrlMes);
                }
                lbl_1_bss_18 = lbl_1_bss_20->instMes[1];
            }
            instMes = lbl_1_bss_18[pageNo];
            HuWinMesSet(instWinId[0], instMes);
            for (k=1; k<frameMax; k++) {
                t = (float)k/(float)frameMax;
                for (i=0; i<2; i++) {
                    HuWinBGTPLvlSet(instWinId[i], t);
                    HuWinBGTPLvlSet(ctrlWinId[i], t);
                }
                HuPrcVSleep();
            }
            while (lbl_1_bss_8 != 0) {
                HuPrcVSleep();
            }
        }
        if (lbl_1_bss_6 == 3) {
            break;
        }
        HuPrcVSleep();
    }
    for (i=0; i<=10; i++) {
        HuWinPosSet(ctrlWinId[0], ((1-cos(3.141592653589793*((i/10.0)*90)/180.0))*400)+ctrlPos.x,
            ctrlPos.y);
        HuWinPosSet(instWinId[0], instPos.x,
            instPos.y+((1-cos(3.141592653589793*((i/10.0)*90)/180.0))*200));
        HuPrcVSleep();
    }
    while (1) {
        HuPrcVSleep();
    }
}

/* Runs the paired model animation process started from fn_1_1024 via fn_1_A1E8. */
void fn_1_2A1C(void)
{
    HU3D_MODELID modelId;
    s16 i;
    s32 mode;
    s16 timer;
    HUPROCESS *process;
    float t;
    HuVecF posA;
    HuVecF posB;
    HU3D_MOTIONID motion[4];

    process = fn_8000DD9C();
    mode = (s32)process->property;
    timer = 0;
    fn_80026B8C(lbl_1_bss_6AA[0], 3);
    fn_8002C95C(lbl_1_bss_6AA[0], lbl_1_bss_4AA[1]);
    fn_800269E8(lbl_1_bss_6AA[0], ((1 << 30) | (1 << 0)));
    fn_80026B8C(lbl_1_bss_6AA[4], 3);
    fn_8002C95C(lbl_1_bss_6AA[4], lbl_1_bss_4AA[5]);
    fn_800269E8(lbl_1_bss_6AA[4], ((1 << 30) | (1 << 0)));
    fn_8002741C(lbl_1_bss_6AA[4], "gN01m1-itemhook_R", lbl_1_bss_6AA[8]);
    if (mode == 0) {
        fn_800269E8(lbl_1_bss_6AA[4], (1 << 0));
    } else {
        fn_800269E8(lbl_1_bss_6AA[0], (1 << 0));
    }
    lbl_1_bss_10 = 0;
    if (lbl_1_bss_6 == 0 || lbl_1_bss_6 == 1) {
        if (mode == 0) {
            modelId = lbl_1_bss_6AA[0];
            motion[0] = lbl_1_bss_4AA[1];
            motion[1] = lbl_1_bss_4AA[2];
        } else {
            modelId = lbl_1_bss_6AA[4];
            motion[0] = lbl_1_bss_4AA[5];
            motion[1] = lbl_1_bss_4AA[6];
        }
        fn_8002C95C(modelId, motion[1]);
        fn_8002D414(modelId, fn_8002D530(modelId));
        fn_80026760(modelId, -1000, 0, 0);
        fn_80026A44(modelId, ((1 << 30) | (1 << 0)));
        while (lbl_1_bss_6 != 1) {
            HuPrcVSleep();
        }
        for (i=1; i<=30; i++) {
            t = i/30.0f;
            fn_80026760(modelId,
                lbl_1_data_70C.x-(200*(1-sin(3.141592653589793*(t*90)/180.0))),
                lbl_1_data_70C.y+(200.0*(1.0-t)), lbl_1_data_70C.z);
            if (i == 15) {
                fn_8002CCC0(modelId, motion[0], 0, 15, ((1 << 30) | (1 << 0)));
            }
            if (i == 10) {
                if (mode == 0) {
                    HuAudFXPlay(950);
                } else {
                    HuAudFXPlay(942);
                }
            }
            fn_80026808(modelId, 0, 15.0+(30.0*(1.0-t)), 0);
            HuPrcVSleep();
        }
    }
    while (1) {
        if (lbl_1_bss_8 != 0) {
            t = 1.0-((lbl_1_bss_8-1)/19.0f);
            fn_80026A44(lbl_1_bss_6AA[0], (1 << 0));
            fn_80026A44(lbl_1_bss_6AA[4], (1 << 0));
            posA = lbl_1_data_70C;
            posB = lbl_1_data_70C;
            if (mode == 0) {
                posA.x = posA.x-100+(100*cos(3.141592653589793*(t*180)/180.0));
                posB.x = posB.x-100-(100*cos(3.141592653589793*(t*180)/180.0));
            } else {
                posA.x = posA.x-100-(100*cos(3.141592653589793*(t*180)/180.0));
                posB.x = posB.x-100+(100*cos(3.141592653589793*(t*180)/180.0));
            }
            posA.z += 100*sin(3.141592653589793*(t*180)/180.0);
            posB.z -= 100*sin(3.141592653589793*(t*180)/180.0);
            fn_80026790(lbl_1_bss_6AA[0], &posA);
            fn_80026790(lbl_1_bss_6AA[4], &posB);
            if (t == 1.0) {
                mode = mode == 0 ? 1 : 0;
                if (mode == 0) {
                    HuAudFXPlay(950);
                } else {
                    HuAudFXPlay(942);
                }
            }
            if (fn_8002D3C4(lbl_1_bss_6AA[0]) != lbl_1_bss_4AA[1] &&
                fn_8002D3EC(lbl_1_bss_6AA[0]) != lbl_1_bss_4AA[1]) {
                fn_8002CCC0(lbl_1_bss_6AA[0], lbl_1_bss_4AA[1], 0, 5,
                    ((1 << 30) | (1 << 0)));
                fn_8002CCC0(lbl_1_bss_6AA[4], lbl_1_bss_4AA[5], 0, 5,
                    ((1 << 30) | (1 << 0)));
                timer = 0;
            }
        } else if (lbl_1_bss_6 == 3) {
            break;
        }
        if (lbl_1_bss_10 != 0 && timer == 0) {
            timer = 80;
            lbl_1_bss_10 = 0;
            fn_8002CCC0(lbl_1_bss_6AA[0], lbl_1_bss_4AA[3], 0, 10, ((1 << 30) | (1 << 0)));
            fn_8002CCC0(lbl_1_bss_6AA[4], lbl_1_bss_4AA[7], 0, 10, ((1 << 30) | (1 << 0)));
        }
        if (timer != 0) {
            timer--;
            if (timer == 5) {
                fn_8002CCC0(lbl_1_bss_6AA[0], lbl_1_bss_4AA[1], 0, 5,
                    ((1 << 30) | (1 << 0)));
                fn_8002CCC0(lbl_1_bss_6AA[4], lbl_1_bss_4AA[5], 0, 5,
                    ((1 << 30) | (1 << 0)));
            }
            lbl_1_bss_10 = 0;
        }
        HuPrcVSleep();
    }
    if (mode == 0) {
        modelId = lbl_1_bss_6AA[0];
        motion[0] = lbl_1_bss_4AA[2];
    } else {
        modelId = lbl_1_bss_6AA[4];
        motion[0] = lbl_1_bss_4AA[6];
    }
    fn_8002CCC0(modelId, motion[0], 0, 15, 0);
    for (i=1; i<=30; i++) {
        t = i/30.0f;
        fn_80026760(modelId, lbl_1_data_70C.x-(200*(1-cos(3.141592653589793*(t*90)/180.0))),
            lbl_1_data_70C.y+(100*t),
                lbl_1_data_70C.z+(300*(1-cos(3.141592653589793*(t*90)/180.0))));
        fn_80026808(modelId, 0, 15+90*(-t), 0);
        HuPrcVSleep();
    }
    HuPrcEnd();
}

/* Updates the preview camera from player one's controls; fn_1_A0 registers this object callback. */
void fn_1_35F8(OMOBJ *obj)
{
    HuVecF pos;
    HuVecF offset;
    HuVecF dir;
    HuVecF yOfs;

    float rotZ;
    s8 stickPos;
    if(lbl_1_bss_0) {
        lbl_80266A58[1].y += 0.1f * lbl_802BFFFC[0];
        lbl_80266A58[1].x += 0.1f * lbl_802BFFF8[0];
        lbl_80266BD8[1] += lbl_802BFFEC[0] / 2;
        lbl_80266BD8[1] -= lbl_802BFFE8[0] / 2;
        if (lbl_80266BD8[1] < 100.0f) {
            lbl_80266BD8[1] = 100.0f;
        }
        pos.x = lbl_80266B18[1].x
            + (lbl_80266BD8[1] * (sin(3.141592653589793*lbl_80266A58[1].y/180.0)
                * cos(3.141592653589793*lbl_80266A58[1].x/180.0)));
        pos.y = (lbl_80266B18[1].y
            + (lbl_80266BD8[1] * -sin(3.141592653589793*lbl_80266A58[1].x/180.0)));
        pos.z = (lbl_80266B18[1].z
            + (lbl_80266BD8[1] * (cos(3.141592653589793*lbl_80266A58[1].y/180.0)
                * cos(3.141592653589793*lbl_80266A58[1].x/180.0))));
        offset.x = lbl_80266B18[1].x - pos.x;
        offset.y = lbl_80266B18[1].y - pos.y;
        offset.z = lbl_80266B18[1].z - pos.z;
        dir.x = (sin(3.141592653589793*lbl_80266A58[1].y/180.0)
            * sin(3.141592653589793*lbl_80266A58[1].x/180.0));
        dir.y = cos(3.141592653589793*lbl_80266A58[1].x/180.0);
        dir.z = (cos(3.141592653589793*lbl_80266A58[1].y/180.0)
            * sin(3.141592653589793*lbl_80266A58[1].x/180.0));
        rotZ = lbl_80266A58[1].z;
        yOfs.x = dir.x * (offset.x * offset.x
            + (1.0f - offset.x * offset.x) * cos(3.141592653589793*rotZ/180.0))
            + dir.y * (offset.x * offset.y * (1.0f - cos(3.141592653589793*rotZ/180.0))
                - offset.z * sin(3.141592653589793*rotZ/180.0))
            + dir.z * (offset.x * offset.z * (1.0f - cos(3.141592653589793*rotZ/180.0))
                + offset.y * sin(3.141592653589793*rotZ/180.0));

        yOfs.y = dir.y * (offset.y * offset.y
            + (1.0f - offset.y * offset.y) * cos(3.141592653589793*rotZ/180.0))
            + dir.x * (offset.x * offset.y * (1.0f - cos(3.141592653589793*rotZ/180.0))
                + offset.z * sin(3.141592653589793*rotZ/180.0))
            + dir.z * (offset.y * offset.z * (1.0f - cos(3.141592653589793*rotZ/180.0))
                - offset.x * sin(3.141592653589793*rotZ/180.0));

        yOfs.z = dir.z * (offset.z * offset.z
            + (1.0f - offset.z * offset.z) * cos(3.141592653589793*rotZ/180.0))
            + (dir.x * (offset.x * offset.z * (1.0 - cos(3.141592653589793*rotZ/180.0))
                - offset.y * sin(3.141592653589793*rotZ/180.0))
                + dir.y * (offset.y * offset.z * (1.0 - cos(3.141592653589793*rotZ/180.0))
                    + offset.x * sin(3.141592653589793*rotZ/180.0)));

        VECCrossProduct(&dir, &offset, &offset);
        VECNormalize(&offset, &offset);
        stickPos = (lbl_802BFFF4[0] & INST_SUBSTICK_STEP_MASK);
        if (stickPos != 0) {
            lbl_80266B18[1].x += 0.05f * (offset.x * stickPos);
            lbl_80266B18[1].y += 0.05f * (offset.y * stickPos);
            lbl_80266B18[1].z += 0.05f * (offset.z * stickPos);
        }
        VECNormalize(&yOfs, &offset);
        stickPos = -(lbl_802BFFF0[0] & INST_SUBSTICK_STEP_MASK);
        if (stickPos != 0) {
            lbl_80266B18[1].x += 0.05f * (offset.x * stickPos);
            lbl_80266B18[1].y += 0.05f * (offset.y * stickPos);
            lbl_80266B18[1].z += 0.05f * (offset.z * stickPos);
        }
    }
}

/* Runs the save/load mode when setup detects the group-system flag. */
void fn_1_4158(void)
{
    HuWinInit(1);
    SLSaveModeExec(0);
    fn_8003A5E4(1, 1);
    while (1) {
        HuPrcVSleep();
    }
}

/* Loads the minigame data directory asynchronously as a child of fn_1_1024. */
void fn_1_4188(void)
{
    u32 startTick;

    lbl_1_data_4 = 0;
    startTick = OSGetTick();
    lbl_1_bss_A = fn_80007F20(MgDataTbl[lbl_1_bss_36].dataDir);
    while (!fn_80008270(lbl_1_bss_A)) {
        HuPrcVSleep();
    }
    lbl_1_data_4 = 1;
    OSReport("Load Time:%d\n",
             (OSGetTick() - startTick) / (OS_TIMER_CLOCK / 1000));
    OSReport("Finished!\n");
    HuPrcEnd();
    while (1) {
        HuPrcVSleep();
    }
}

/* Tracks the second controller's stick and triggers, printing the position on B. */
void fn_1_4268(void)
{
    s16 initialModelId = lbl_1_bss_6C;
    HuVecF position = {0.0f, 0.0f, 0.0f};

    while (1) {
        position.x += lbl_802BFFFC[1] / 4;
        position.y += lbl_802BFFF8[1] / 4;
        position.z += lbl_802BFFEC[1] / 2;
        position.z -= lbl_802BFFE8[1] / 2;
        if (lbl_802C0008[1] & PAD_BUTTON_B) {
            OSReport("%f,%f,%f\n", position.x, position.y, position.z);
        }
        HuPrcVSleep();
    }
}
