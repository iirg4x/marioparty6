// Character model, motion, voice, lighting, and particle effects used throughout gameplay.
#define _MATH_H
#include "dolphin/math.h"

#include "game/charman.h"
#include "game/process.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/sprite.h"

#include "datanum/effect.h"

#define EFFECT_WARN 0
#define EFFECT_DUST 1
#define EFFECT_SMOKE 2
#define EFFECT_STAR 3
#define EFFECT_GLOW 4
#define EFFECT_HIT 5
#define EFFECT_BIRD 6
#define EFFECT_CRY 7
#define EFFECT_LANDDUST 8
#define EFFECT_MAX 9

#define CHAR_VOICE_OFF (1 << 0)
#define CHAR_MOTION_UPDATE_OFF (1 << 1)
#define CHAR_ATTR_BIRD_ACTIVE (1 << 0)
#define CHAR_ATTR_WIN_VOICE_PLAYED (1 << 1)
#define CHAR_ATTR_MOTION_FLAG (1 << 2)
#define CHAR_ATTR_VOICE_PAN_AUTO (1 << 3)
#define CHAR_ATTR_FX_OFF (1 << 4)
#define CHAR_EFFECT_SCALE_FLICKER (1 << 0)
#define CHAR_STEP_GLOBAL_SE (1 << 15)
#define CHAR_STEP_TYPE4_SE 1345
#define CHAR_STEP_TYPE5_SE 1353
#define CHAR_LIGHT_FLAGS_MASK (((1 << 8) - 1) << 8)

typedef struct CharWork_s {
    HU3D_MODELID modelId; // Active 3D model, or HU3D_MODELID_NONE when unloaded.
    s16 model; // Character model variant selected by CHAR_MODEL0 through CHAR_MODEL3.
    s16 motNoCurr; // Current character motion number.
    s16 motNoShiftCurr; // Current motion number on the blending track.
    s16 motNoPrev; // Previous motion number on the main track.
    s16 motNoShiftPrev; // Previous motion number on the blending track.
    HU3D_MOTIONID motId[32]; // Loaded engine motion IDs indexed by character motion slot.
    s16 motNoTbl[32]; // Character motion number for each loaded motion slot.
    s16 vol; // Character voice volume passed to the audio system.
    s16 pan; // Character voice pan passed to the audio system.
    u8 voiceFlag[32]; // Voice and update flags for each loaded motion.
    u32 attr; // Character behavior flags defined by CHAR_ATTR_*.
    s8 stepFx; // Footstep sound variation selected for this character.
    HuVecF pos; // Last sampled model position in world coordinates.
    AMEM_PTR motAMemP; // Handle for this character's motion archive in audio RAM.
    HUPROCESS *process; // Per-character animation update process.
    s16 timingHookNo; // Number of motion timing callbacks seen by the update process.
    s16 motNoPlay[2]; // Motion numbers currently tracked on the two animation tracks.
    s16 motPlayTime[2]; // Frames elapsed for each tracked motion.
} CHARWORK;

typedef struct EffectData_s {
    unsigned int dataNum; // Particle resource archive number.
    s16 maxCnt; // Maximum particle count for this effect.
    s16 blendMode; // GX particle blend mode.
    s32 motCnt; // Unused field; stored values do not count the motion-resource entries.
    unsigned int motDataNum[16]; // Particle motion resource numbers.
} EFFECTDATA;

typedef struct EffectParam_s {
    u32 attr; // Particle behavior flags.
    GXColor colorBegin; // Starting particle color and alpha.
    GXColor colorEnd; // Ending particle color and alpha.
    HuVecF vel; // Initial velocity in world units per frame.
    HuVecF velDecay; // Per-frame velocity multiplier.
    float gravity; // Vertical acceleration per frame.
    u32 zero; // Reserved particle parameter, initialized to zero.
    float scaleVel; // Particle scale change per frame.
    float alphaBase; // Alpha change per frame.
    float colorWeight; // Blend weight between the endpoint colors.
} EFFECTPARAM;

static EFFECTDATA effectDataTbl[] = {
    { EFFECT_ANM_danger, 10, 0, 2, { CHARMOT_HSF_c000m1_324, CHARMOT_HSF_c000m1_325 } },
    { EFFECT_ANM_dust, 150, 1, 0, { CHARMOT_HSF_c000m1_301, CHARMOT_HSF_c000m1_302,
        CHARMOT_HSF_c000m1_345, CHARMOT_HSF_c000m1_324, CHARMOT_HSF_c000m1_332 } },
    { EFFECT_ANM_smoke, 70, 0, 0, { CHARMOT_HSF_c000m1_320 } },
    { EFFECT_ANM_star, 300, 0, 0, { CHARMOT_HSF_c000m1_310 } },
    { EFFECT_ANM_glow, 300, 1, 0, {} },
    { EFFECT_ANM_circle, 200, 1, 0, { CHARMOT_HSF_c000m1_305 } },
    { EFFECT_ANM_bird, 12, 0, 2, { CHARMOT_HSF_c000m1_321, CHARMOT_HSF_c000m1_322,
        CHARMOT_HSF_c000m1_465 } },
    { EFFECT_ANM_cry, 100, 1, 0, { CHARMOT_HSF_c000m1_344, CHARMOT_HSF_c000m1_345 } },
    { EFFECT_ANM_land_dust, 100, 0, 0, { CHARMOT_HSF_c000m1_310, CHARMOT_HSF_c000m1_304 } },
};

char *CharHeadObjNameTbl[15] = {
    "root_head", "root_head", "root_hair1", "ske_head",  "eff_head",
    "hair1",     "eff_head",  "root_head",  "root_head", "root_head",
    "root_head", "root_head", "root_head",  "root_head", "root_head",
};

unsigned int lbl_8022E718[14][6] = {
    { 0x8f0000, 0x900000, 0x910000, 0x920000, 0x930000, 0x8e0000 },
    { 0x370000, 0x380000, 0x390000, 0x3a0000, 0x3b0000, 0x360000 },
    { 0xc10000, 0xc20000, 0xc30000, 0xc40000, 0xc50000, 0xc00000 },
    { 0xf40000, 0xf50000, 0xf60000, 0xf70000, 0xf80000, 0xf30000 },
    { 0xed0000, 0xee0000, 0xef0000, 0xf00000, 0xf10000, 0xec0000 },
    { 0x150000, 0x160000, 0x170000, 0x180000, 0x190000, 0x140000 },
    { 0xe70000, 0xe80000, 0xe90000, 0xea0000, 0xeb0000, 0xe60000 },
    { 0x310000, 0x320000, 0x330000, 0x340000, 0x350000, 0x300000 },
    { 0xd00000, 0xd10000, 0xd20000, 0xd30000, 0xd40000, 0xcf0000 },
    { 0xb80000, 0xb90000, 0xba0000, 0xbb0000, 0xbc0000, 0xa80000 },
    { 0x2b0000, 0x2c0000, 0x2d0000, 0x2e0000, 0x2f0000, 0x2a0000 },
    { 0xb40000, 0xb50000, 0xb60000, 0xb70000, 0xbc0000, 0xa80000 },
    { 0xaf0000, 0xb00000, 0xb10000, 0xb20000, 0xbc0000, 0xa80000 },
    { 0xaa0000, 0xab0000, 0xac0000, 0xad0000, 0xbc0000, 0xa80000 },
};

static u8 lbl_8026F7B0[232];
static u16 dustFlags[14];
static HUPROCESS *hookDustProc[32];
static EFFECTPARAM *effParamAll[EFFECT_MAX];
static HU3D_MODELID effModelId[EFFECT_MAX][16];
static CHARWORK CharWork[14];

static s16 effectLayer;
static BOOL motShiftF;
static AMEM_PTR effectAMemP;

// Initializes character work records and the shared particle resources before characters are
// created.
void fn_80056428(void)
{
    s16 i;
    s16 j;
    for(i=0; i<14; i++) {
        CHARWORK *workP = &CharWork[i];
        workP->motAMemP = 0;
        for(j=0; j<32; j++) {
            workP->motId[j] = -1;
        }
        workP->modelId = -1;
        workP->process = NULL;
        workP->vol = MSM_VOL_MAX;
        workP->pan = MSM_PAN_CENTER;
    }
    if(!effectAMemP) {
        effectAMemP = HuAR_DVDtoARAM(0x210000);
    }
    for(i=0; i<EFFECT_MAX; i++) {
        for(j=0; j<16; j++) {
            effModelId[i][j] = -1;
        }
        effParamAll[i] = NULL;
    }
    for(i=0; i<32; i++) {
        hookDustProc[i] = NULL;
    }
    effectLayer = 0;
}

// Returns this character's current motion archive handle in audio RAM.
AMEM_PTR fn_80056598(s16 charNo)
{
    return CharWork[charNo].motAMemP;
}

// Loads the selected character's motion archive into ARAM on first use.
void fn_800565B4(s16 charNo)
{
    if(charNo >= 14 || charNo < 0 || charNo == (u8)-1) {
        return;
    } else {
        CHARWORK *workP = &CharWork[charNo];
        if(!workP->motAMemP) {
            workP->motAMemP = HuAR_DVDtoARAM(lbl_8022E718[charNo][4]);
        }
    }
}

// Reads the motion archive asynchronously, waits for the read and DMA, then closes its MRAM
// directory.
void fn_8005664C(s16 charNo)
{
    if(charNo >= 14 || charNo < 0 || charNo == (u8)-1) {
        return;
    } else {
        CHARWORK *workP = &CharWork[charNo];
        if(!workP->motAMemP) {
            s16 statId = fn_80007F20(lbl_8022E718[charNo][4]);
            while(!fn_80008270(statId)) {
                HuPrcVSleep();
            }
            workP->motAMemP = HuAR_MRAMtoARAM(lbl_8022E718[charNo][4]);
            while(fn_80055FB4()) {
                HuPrcVSleep();
            }
            fn_800097F4(lbl_8022E718[charNo][4]);
        }
    }
}

// Frees the ARAM motion archive only if its source directory is present, then clears the stored
// handle either way.
void fn_80056754(s16 charNo)
{
    CHARWORK *workP = &CharWork[charNo];
    if(workP->motAMemP) {
        if(HuARDirCheck(lbl_8022E718[charNo][4])) {
            HuARFree(workP->motAMemP);
        }
        workP->motAMemP = 0;
    }
}

// Closes all model directories and motion storage for one character, or all characters for
// CHARNO_NONE.
void fn_800567D8(s16 charNo)
{
    s16 i;
    if(charNo == -1) {
        for(i=0; i<14; i++) {
            fn_800567D8(i);
        }
    } else {
        for(i=0; i<5; i++) {
            fn_800097F4(lbl_8022E718[charNo][i]);
        }
        fn_80056754(charNo);
    }
}

// Orders cached character motion archives according to the requested character sequence.
void fn_80056B7C(s16 *statList)
{
    CHARWORK *workP;
    CHARWORK *workList[4];
    s16 noList[4];
    s16 sortList[4];
    s16 i;
    s16 j;
    s16 cnt;
    s16 tmp;
    void *data1;
    void *data2;
    HUDATASTAT *statusP;
    OSTime start = OSGetTick();
    for(workP=CharWork, cnt=0, i=cnt; i<14; i++, workP++) {
        if(workP->motAMemP) {
            noList[cnt] = i;
            workList[cnt++] = workP;
        }
    }
    for(i=cnt-1; i>=0; i--) {
        for(j=0; j<i; j++) {
            if(workList[j]->motAMemP > workList[j+1]->motAMemP) {
                tmp = noList[j];
                noList[j] = noList[j+1];
                noList[j+1] = tmp;
                workP = workList[j];
                workList[j] = workList[j+1];
                workList[j+1] = workP;
            }
        }
    }
    for(tmp=0, i=tmp; i<cnt; i++) {
        for(j=0; j<cnt; j++) {
            if(statList[j] == noList[i]) {
                tmp++;
                sortList[i] = j;
            }
        }
    }
    if(tmp != cnt) {
        return;
    }
    for(i=cnt-1; i>=0; i--) {
        for(j=0; j<i; j++) {
            if(sortList[j] > sortList[j+1]) {
                data1 = fn_80055D40(workList[j]->motAMemP);
                while(fn_80055FB4()) {}
                data2 = fn_80055D40(workList[j+1]->motAMemP);
                while(fn_80055FB4()) {}
                HuARFree(workList[j]->motAMemP);
                HuARFree(workList[j+1]->motAMemP);
                workList[j+1]->motAMemP = HuAR_MRAMtoARAM2(data2);
                while(fn_80055FB4()) {}
                workList[j]->motAMemP = HuAR_MRAMtoARAM2(data1);
                while(fn_80055FB4()) {}
                statusP = fn_800077AC(data1);
                fn_800097F4(statusP->dirId << 16);
                statusP = fn_800077AC(data2);
                fn_800097F4(statusP->dirId << 16);
                tmp = sortList[j]; sortList[j] = sortList[j+1]; sortList[j+1] = tmp;
                tmp = noList[j]; noList[j] = noList[j+1]; noList[j+1] = tmp;
                workP = workList[j]; workList[j] = workList[j+1]; workList[j+1] = workP;
            }
        }
    }
    printf("Time %dmsec\n", OSTicksToMilliseconds(OSGetTick()-start));
}

static EFFECTPARAM dustEffParam = { 0,
                                    { 128, 128, 128, 255 },
                                    { 64, 32, 0, 255 },
                                    { 0, 2, 1 },
                                    { 0.95f, 0.95f, 0.95f },
                                    0,
                                    0,
                                    1,
                                    -5,
                                    0.02f };
static EFFECTPARAM hitEffParam = {
    0, { 255, 64, 64, 128 }, { 255, 64, 64, 128 }, { 0, 0, 0 }, { 0, 0, 0 }, 0, 0, -5, 0, 0
};
static EFFECTPARAM npcHitEffParam = { 0,
                                      { 255, 255, 0, 255 },
                                      { 255, 255, 0, 255 },
                                      { 0, 0, 0 },
                                      { 0.95f, 0.95f, 0.95f },
                                      0,
                                      0,
                                      -0.5,
                                      -10,
                                      0 };
static EFFECTPARAM warnEffParam = { 0,
                                    { 255, 255, 255, 255 },
                                    { 255, 255, 255, 255 },
                                    { 0, 20, 0 },
                                    { 0.95f, 0.85f, 0.95f },
                                    0,
                                    0,
                                    1,
                                    -5,
                                    0 };
static EFFECTPARAM smokeEffParam = {
    0,   { 128, 32, 32, 255 }, { 0, 0, 0, 255 }, { 0, 10, 0 }, { 1.0f, 0.95f, 1.0f }, 0, 0, 5, -13,
    0.1f
};

static s8 walkVoiceTimeTbl[56] = {
    10, 32, -1, -1,
    20, 40, -1, -1,
    12, 37, -1, -1,
    4, 25, -1, -1,
    5, 30, -1, -1,
    23, 50, -1, -1,
    19, 39, -1, -1,
    17, 38, -1, -1,
    10, 32, -1, -1,
    21, 44, -1, -1,
    17, 38, -1, -1,
    21, 44, -1, -1,
    21, 44, -1, -1,
    21, 44, -1, -1,
};

static s8 runVoiceTimeTbl[56] = {
    15, 29, -1, -1,
    5, 19, -1, -1,
    5, 22, -1, -1,
    12, 28, -1, -1,
    18, 35, -1, -1,
    0, 16, -1, -1,
    12, 28, -1, -1,
    14, 30, -1, -1,
    10, 32, -1, -1,
    16, 32, -1, -1,
    14, 30, -1, -1,
    16, 32, -1, -1,
    16, 32, -1, -1,
    16, 32, -1, -1,
};

static s8 jumpVoiceTimeTbl[28] = {
    2, 4,
    3, 5,
    2, 3,
    3, -1,
    2, 3,
    4, 5,
    2, 4,
    1, 2,
    4, -1,
    1, 3,
    2, 4,
    1, 3,
    1, 3,
    1, 3,
};

static u8 boardWinAnimLen[14] = { 14, 0, 46, 72, 20, 34, 41, 0, 0, 17, 0, 17, 17, 17 };
static u8 winAnimLen[14] = { 18, 0, 30, 106, 0, 0, 24, 24, 90, 60, 60, 60, 60, 60 };
static u8 handUpAnimLen[14] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static u8 okAnimLen[14] = { 0, 0, 24, 90, 30, 0, 18, 42, 96, 30, 42, 30, 30, 30 };
static u8 winAnimLen2[14] = { 18, 0, 30, 106, 0, 0, 24, 24, 90, 60, 24, 60, 60, 60 };
static u8 holdUpAnimLen[14] = { 0, 6, 0, 78, 0, 16, 50, 0, 54, 38, 0, 38, 38, 38 };
static u8 winJumpUpAnimLen[14] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static u8 loseAnimLen[14] = { 18, 0, 36, 106, 0, 0, 24, 24, 90, 60, 24, 60, 60, 60 };

static s8 jumpAnimLen[14] = { 42, 42, 43, 52, 42, 42, -1, 42, 42, 42, 42, 42, 42, 42 };

// Ignores charNo: releases all character models and data for shutdown, then frees the shared effect
// archive.
void fn_8005702C(s16 charNo)
{
    fn_8005BB8C(-1);
    fn_800567D8(-1);
    HuARFree(effectAMemP);
    effectAMemP = 0;
}

static void CharTimingHook(HU3D_MODELID modelId, HU3D_MOTIONID motId, BOOL lagF);
static void UpdateChar(void);
static s16 GetStepType(void);

// Creates a character model and its animation process, selecting the model archive from the variant
// flags.
HU3D_MODELID fn_80057068(s16 charNo, s16 model)
{
    s16 unused = 0; // This initialized local is not read during model setup.
    CHARWORK *workP = &CharWork[charNo];
    unsigned int dataNum;
    HU3D_MODELID modelId;
    s16 *characterNoPtr;
    void *modelData;

    if(workP->modelId != -1) {
        fn_800262D0(workP->modelId);
    }
    if(model & (1 << 0)) {
        dataNum = lbl_8022E718[charNo][0];
    } else if(model & (1 << 1)) {
        dataNum = lbl_8022E718[charNo][1];
    } else if(model & (1 << 2)) {
        dataNum = lbl_8022E718[charNo][2];
    } else {
        dataNum = lbl_8022E718[charNo][3];
    }
    modelData = fn_80008DD0(dataNum, 0x10000000, 2);
    workP->modelId = modelId = fn_80025A04(modelData);
    workP->process = fn_8000D818(UpdateChar, 99, 16384, 0);
    workP->process->property = characterNoPtr = HuMemDirectMalloc(HEAP_HEAP, sizeof(s16));
    workP->model = model;
    workP->attr = 0;
    *characterNoPtr = charNo;
    workP->stepFx = GetStepType();
    fn_8002DB98(modelId, CharTimingHook);
    workP->timingHookNo = 0;
    workP->motNoPlay[0] = workP->motNoPlay[1] = 32767;
    return modelId;
}

// Records each timing callback for this character model, regardless of active motion or lag state.
static void CharTimingHook(HU3D_MODELID modelId, HU3D_MOTIONID motId, BOOL lagF)
{
    CHARWORK *workP = &CharWork[0];
    s16 i;
    for(i=0; i<14; i++, workP++) {
        if(workP->modelId == modelId) {
            workP->timingHookNo++;
            break;
        }
    }
}

static void UpdateMotPlay(CHARWORK *workP, s16 motNo, s16 motNoShift);
void CharEffectHipDropCreate(s16 charNo, HuVecF *pos);
static void UpdateCharAnim(s16 charNo, HU3D_MODELID modelId, s16 motNo, u8 voiceFlag, s16 frameNo,
                           HuVecF *ofs);
static void EyeBmpUpdate(s16 charNo);

// Per-character process created by CharModelCreate; samples active motions and updates animation
// effects each frame.
static void UpdateChar(void)
{
    s16 *characterNoPtr = fn_8000DD9C()->property;
    CHARWORK *workP = &CharWork[*characterNoPtr];
    HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
    s16 updateBmpF = FALSE;
    HuVecF pos;
    s16 timingHookNo;
    pos.y = 100000.0f;
    workP->timingHookNo = timingHookNo = 0;
    
    while(1) {
        s16 motNo;
        s16 motNoShift;
        s16 motId;
        s16 motIdShift;
        s16 i;
        motNo = motNoShift = -1;
        if(modelP->motId != -1) {
            for(i=0; i<32; i++) {
                if(workP->motId[i] == modelP->motId) {
                    motNo = workP->motNoTbl[i];
                    motId = i;
                    break;
                }
            }
        }
        if(modelP->motIdShift != -1) {
            for(i=0; i<32; i++) {
                if(workP->motId[i] == modelP->motIdShift) {
                    motNoShift = workP->motNoTbl[i];
                    motIdShift = i;
                    break;
                }
                
            }
        }
        UpdateMotPlay(workP, motNo, motNoShift);
        motShiftF = FALSE;
        if(motNo != -1) {
            workP->motNoCurr = motNo;
            UpdateCharAnim(*characterNoPtr, workP->modelId, motNo, workP->voiceFlag[motId],
                           modelP->motWork.time, &pos);
            workP->motNoPrev = motNo;
        } else {
            workP->motNoCurr = -1;
        }
        if(modelP->motIdShift != -1) {
            motShiftF = TRUE;
            updateBmpF = TRUE;
            if(motNoShift != -1) {
                workP->motNoShiftCurr = motNoShift;
                UpdateCharAnim(*characterNoPtr, workP->modelId, motNoShift,
                               workP->voiceFlag[motIdShift], modelP->motShiftWork.time, &pos);
                workP->motNoShiftPrev = motNoShift;
            } else {
                workP->motNoShiftCurr = -1;
            }
        } else {
            if(updateBmpF) {
                EyeBmpUpdate(*characterNoPtr);
                updateBmpF = FALSE;
            }
        }
        workP->pos = modelP->pos;
        if(timingHookNo != workP->timingHookNo) {
            timingHookNo = workP->timingHookNo;
        } else {
            workP->timingHookNo = 0;
        }
        HuPrcVSleep();
    }
}

// Tracks elapsed frames for the active main and blending motions, clearing slots for motions that
// ended.
static void UpdateMotPlay(CHARWORK *workP, s16 motNo, s16 motNoShift)
{
    s16 i;
    if(motNo != -1) {
        if(workP->motNoPlay[0] != motNo && workP->motNoPlay[1] != motNo) {
            for(i=0; i<2; i++) {
                if(workP->motNoPlay[i] != motNoShift) {
                    workP->motNoPlay[i] = motNo;
                    workP->motPlayTime[i] = 0;
                    break;
                }
            }
        } else {
            for(i=0; i<2; i++) {
                if(workP->motNoPlay[i] == motNo) {
                    workP->motPlayTime[i]++;
                    break;
                }
            }
        }
    }
    if(motNoShift != -1) {
        if(workP->motNoPlay[0] != motNoShift && workP->motNoPlay[1] != motNoShift) {
            for(i=0; i<2; i++) {
                if(workP->motNoPlay[i] != motNo) {
                    workP->motNoPlay[i] = motNoShift;
                    workP->motPlayTime[i] = 0;
                    break;
                }
            }
        } else {
            for(i=0; i<2; i++) {
                if(workP->motNoPlay[i] == motNoShift) {
                    workP->motPlayTime[i]++;
                    break;
                }
            }
        }
    }
    if(workP->motNoPlay[0] != motNo && workP->motNoPlay[0] != motNoShift) {
        workP->motNoPlay[0] = 32767;
    }
    if(workP->motNoPlay[1] != motNo && workP->motNoPlay[1] != motNoShift) {
        workP->motNoPlay[1] = 32767;
    }
}

// Returns the elapsed frame count for a tracked motion, or -1 if neither animation track contains
// it.
static inline s32 GetMotNoPlayTime(CHARWORK *workP, s16 motNo)
{
    s16 i;
    for(i=0; i<2; i++) {
        if(workP->motNoPlay[i] == motNo) {
            return workP->motPlayTime[i];
        }
    }
    return -1;
}

s16 fn_800591C8(s16 charNo, s16 seNo, u8 voiceFlag);
static s16 CharWinVoicePlay(s16 charNo, u8 voiceFlag);
static s16 EffectDustCreate(HU3D_MODELID modelId, float posX, float posY, float posZ, float scale,
                            EFFECTPARAM *param);
static s16 EffectHitCreate(HU3D_MODELID modelId, float posX, float posY, float posZ, float scale,
                           EFFECTPARAM *param);
static s16 EffectSmokeCreate(HU3D_MODELID modelId, float posX, float posY, float posZ, float scale,
                             EFFECTPARAM *param);

static inline s16 EffectWarnCreate(HU3D_MODELID modelId, float posX, float posY, float posZ,
                                   float scale, float ofsY, EFFECTPARAM *param);
static s16 EffectBirdCreate(HU3D_MODELID modelId, float posX, float posY, float posZ, float scale,
                            s16 charNo, float ofsY, EFFECTPARAM *param);

static s16 PlayStepVoice(s16 charNo, s16 seId, u8 voiceFlag);

static void UpdateCharAnim(s16 charNo, HU3D_MODELID modelId, s16 motNo, u8 voiceFlag, s16 frameNo,
                           HuVecF *ofs)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    CHARWORK *workP = &CharWork[charNo];
    u32 attrOld = 0;
    s16 i;
    s16 dotMax;
    Mtx hitMtx;
    HuVecF pos;
    HuVecF hitPos;
    float warnSize;
    if(!motShiftF && (modelP->motAttr & ((1 << 30) | (1 << 1)))) {
        return;
    }
    if(voiceFlag & CHAR_MOTION_UPDATE_OFF) {
        return;
    }
    switch(motNo) {
        case FILENUM(CHARMOT_HSF_c000m1_315):
        case FILENUM(CHARMOT_HSF_c000m1_317):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                fn_800591C8(charNo, 582, voiceFlag);
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_301):
        case FILENUM(CHARMOT_HSF_c000m1_332):
        case FILENUM(CHARMOT_HSF_c000m1_433):
        case FILENUM(CHARMOT_HSF_c000m1_442):
        case FILENUM(CHARMOT_HSF_c000m1_448):
            if(!motShiftF && modelP->motWork.speed <= 0.5) {
                break;
            }
            if(motShiftF && modelP->motShiftWork.speed <= 0.5) {
                break;
            }
            if((frameNo & (16 - 1)) == 0 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                dustEffParam.vel.x = -sin(3.141592653589793*modelP->rot.y/180.0)*2.0;
                dustEffParam.vel.y = 1+(0.1*frandmod(10));
                dustEffParam.vel.z = -cos(3.141592653589793*modelP->rot.y/180.0)*2.0;
                pos.x = modelP->pos.x+(modelP->scale.x*(frandmod(50)-25));
                pos.y = modelP->pos.y;
                pos.z = modelP->pos.z+(modelP->scale.x*(frandmod(50)-25));
                if(charNo == 9) {
                    pos.x += -sin(3.141592653589793*modelP->rot.y/180.0)*40.0;
                    pos.z += -cos(3.141592653589793*modelP->rot.y/180.0)*40.0;
                }
                EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10)+30, &dustEffParam);
            }
            for(i=0; i<4; i++) {
                if(frameNo == walkVoiceTimeTbl[(charNo*4)+i]) {
                    PlayStepVoice(charNo, 177, voiceFlag);
                    break;
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_302):
        case FILENUM(CHARMOT_HSF_c000m1_345):
        case FILENUM(CHARMOT_HSF_c000m1_434):
        case FILENUM(CHARMOT_HSF_c000m1_443):
        case FILENUM(CHARMOT_HSF_c000m1_458):
            if(!motShiftF && modelP->motWork.speed <= 0.5) {
                break;
            }
            if(motShiftF && modelP->motShiftWork.speed <= 0.5) {
                break;
            }
            if((frameNo & (8 - 1)) == 0 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                dustEffParam.vel.x = -sin(3.141592653589793*modelP->rot.y/180.0)*4.0;
                dustEffParam.vel.y = 2+(0.1*frandmod(10));
                dustEffParam.vel.z = -cos(3.141592653589793*modelP->rot.y/180.0)*4.0;
                pos.x = modelP->pos.x+(modelP->scale.x*(frandmod(50)-25));
                pos.y = modelP->pos.y;
                pos.z = modelP->pos.z+(modelP->scale.x*(frandmod(50)-25));
                if(charNo == 9) {
                    pos.x += -sin(3.141592653589793*modelP->rot.y/180.0)*40.0;
                    pos.z += -cos(3.141592653589793*modelP->rot.y/180.0)*40.0;
                }
                EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10)+30, &dustEffParam);
            }
            if(motNo == FILENUM(CHARMOT_HSF_c000m1_345) && (frameNo & 1)) {
                fn_80060B10(charNo, 30, 0.8f);
            }
            for(i=0; i<4; i++) {
                if(frameNo == runVoiceTimeTbl[(charNo*4)+i]) {
                    PlayStepVoice(charNo, 181, voiceFlag);
                    break;
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_304):
            if(frameNo == 5 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                fn_8005F968(charNo, &modelP->pos);
            }
            for(i=0; i<2; i++) {
                if(frameNo == jumpVoiceTimeTbl[(charNo*2)+i]) {
                    PlayStepVoice(charNo, 185, voiceFlag);
                    break;
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_464):
            if(frameNo == 1 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                fn_8005F968(charNo, &modelP->pos);
                PlayStepVoice(charNo, 185, voiceFlag);
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_305):
            fn_80023398(modelId, fn_8005C0F0(charNo, workP->model, 0), hitMtx);
            if(frameNo == 0) {
                fn_800591C8(charNo, 199, voiceFlag);
            }
            dotMax = 10;
            if(frameNo <= dotMax && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                hitEffParam.vel.x = 0.0f;
                hitEffParam.vel.y = 0.0f;
                hitEffParam.vel.z = 0.0f;
                hitEffParam.scaleVel = -5.0f;
                pos.x = hitMtx[0][3];
                pos.y = hitMtx[1][3];
                pos.z = hitMtx[2][3];
                if(frameNo != 0 && ofs->y != 100000.0f) {
                    VECSubtract(&pos, ofs, &hitPos);
                    dotMax = 0.2*sqrtf(hitPos.z*hitPos.z+(hitPos.x*hitPos.x+hitPos.y*hitPos.y));
                    if(dotMax > 5) {
                        dotMax = 5;
                    }
                    if(dotMax < 1) {
                        dotMax = 1;
                    }
                    for(i=1; i<=dotMax; i++) {
                        hitPos.x = ofs->x+(pos.x-ofs->x)*((float)i/dotMax);
                        hitPos.y = ofs->y+(pos.y-ofs->y)*((float)i/dotMax);
                        hitPos.z = ofs->z+(pos.z-ofs->z)*((float)i/dotMax);
                        EffectHitCreate(modelId, hitPos.x, hitPos.y, hitPos.z, 50, &hitEffParam);
                    }
                } else {
                    EffectHitCreate(modelId, pos.x, pos.y, pos.z, 40, &hitEffParam);
                }
                *ofs = pos;
            } else {
                ofs->y = 100000.0f;
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_367):
            if(frameNo == 0) {
                fn_800591C8(charNo, 200, voiceFlag);
            }
            if(frameNo < 10) {
                fn_80023398(modelId, fn_8005C0F0(charNo, workP->model, 2), hitMtx);
                hitEffParam.vel.x = 0.0f;
                hitEffParam.vel.y = 0.0f;
                hitEffParam.vel.z = 0.0f;
                hitEffParam.scaleVel = -8.0f;

                pos.x = hitMtx[0][3];
                pos.y = hitMtx[1][3];
                pos.z = hitMtx[2][3];
                if(frameNo != 0) {
                    VECSubtract(&pos, ofs, &hitPos);
                    dotMax = 0.2*sqrtf(hitPos.z*hitPos.z+(hitPos.x*hitPos.x+hitPos.y*hitPos.y));
                    if(dotMax > 5) {
                        dotMax = 5;
                    }
                    if(dotMax < 1) {
                        dotMax = 1;
                    }
                    for(i=1; i<=dotMax; i++) {
                        hitPos.x = ofs->x+(pos.x-ofs->x)*((float)i/dotMax);
                        hitPos.y = ofs->y+(pos.y-ofs->y)*((float)i/dotMax);
                        hitPos.z = ofs->z+(pos.z-ofs->z)*((float)i/dotMax);
                        EffectHitCreate(modelId, hitPos.x, hitPos.y, hitPos.z, 80, &hitEffParam);
                    }
                } else {
                    EffectHitCreate(modelId, pos.x, pos.y, pos.z, 80, &hitEffParam);
                }
                *ofs = pos;
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_310):
            if(frameNo == 0) {
                if(!(workP->attr & CHAR_ATTR_FX_OFF)) {
                    CharEffectHipDropCreate(charNo, &modelP->pos);
                }
                fn_800591C8(charNo, 195, voiceFlag);
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_324):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                fn_800591C8(charNo, 581, voiceFlag);
            }
            if(modelP->motIdShift == -1 || motShiftF) {
                if(!(workP->attr & CHAR_ATTR_FX_OFF)) {
                    if(frameNo == 10) {
                        if(charNo == 6) {
                            warnSize = 190;
                        } else if(charNo == 2 || charNo == 5) {
                            warnSize = 140;
                        } else {
                            warnSize = 120;
                        }
                        EffectWarnCreate(modelId, modelP->pos.x, 100.0f + modelP->pos.y,
                                         modelP->pos.z, 20, warnSize, &warnEffParam);
                    }
                    if(frameNo == 30) {
                        fn_8005F968(charNo, &modelP->pos);
                        PlayStepVoice(charNo, 185, voiceFlag);
                    }
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_325):
            if(modelP->motIdShift == -1 || motShiftF) {
                if(!(workP->attr & CHAR_ATTR_FX_OFF)) {
                    if(frameNo == 0) {
                        if(charNo == 6) {
                            warnSize = 150;
                        } else if(charNo == 2 || charNo == 5) {
                            warnSize = 120;
                        } else {
                            warnSize = 100;
                        }
                        EffectWarnCreate(modelId, modelP->pos.x, 100.0f + modelP->pos.y,
                                         modelP->pos.z, 20, warnSize, &warnEffParam);
                    }
                    if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                        fn_800591C8(charNo, 582, voiceFlag);
                    }
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_321):
        case FILENUM(CHARMOT_HSF_c000m1_322):
        case FILENUM(CHARMOT_HSF_c000m1_465):
            if(!(workP->attr & CHAR_ATTR_BIRD_ACTIVE) && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                fn_800591C8(charNo, 196, voiceFlag);
                for(i=0; i<3; i++) {
                    EffectBirdCreate(modelId, modelP->pos.x,
                                     modelP->pos.y + (100.0f * modelP->scale.x), modelP->pos.z,
                                     1.0f, charNo, i * 120, &warnEffParam);
                }
                workP->attr |= CHAR_ATTR_BIRD_ACTIVE;
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_320):
            if(!(workP->attr & CHAR_ATTR_FX_OFF)) {
                if(frameNo & 1) {
                    fn_80023398(modelId, fn_8005C0F0(charNo, workP->model, 4),
                                       hitMtx);
                    pos.x = hitMtx[0][3];
                    pos.y = hitMtx[1][3];
                    pos.z = hitMtx[2][3];
                    EffectSmokeCreate(modelId, pos.x, pos.y, pos.z, 20, &smokeEffParam);
                }
                if(GetMotNoPlayTime(workP, motNo) == 0) {
                    fn_800591C8(charNo, 576, voiceFlag);
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_303):
            if(frameNo == 0) {
                fn_800591C8(charNo, 193, voiceFlag);
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_308):
            if(frameNo == 0) {
                fn_800591C8(charNo, 194, voiceFlag);
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_306):
            if(!(workP->attr & (CHAR_ATTR_WIN_VOICE_PLAYED | CHAR_ATTR_FX_OFF))) {
                if ((omcurovl < 123 || omcurovl > 130) &&
                    (omcurovl < 114 || omcurovl > 116)) {
                    if(frameNo == winAnimLen[charNo]) {
                        CharWinVoicePlay(charNo, voiceFlag);
                        workP->attr |= CHAR_ATTR_WIN_VOICE_PLAYED;
                        attrOld |= CHAR_ATTR_WIN_VOICE_PLAYED;
                    }
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_346):
            if(!(workP->attr & (CHAR_ATTR_WIN_VOICE_PLAYED | CHAR_ATTR_FX_OFF))) {
                if(frameNo == handUpAnimLen[charNo]) {
                    fn_800591C8(charNo, 579, voiceFlag);
                    workP->attr |= CHAR_ATTR_WIN_VOICE_PLAYED;
                    attrOld |= CHAR_ATTR_WIN_VOICE_PLAYED;
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_349):
            if(!(workP->attr & (CHAR_ATTR_WIN_VOICE_PLAYED | CHAR_ATTR_FX_OFF))) {
                if(frameNo == okAnimLen[charNo]) {
                    CharWinVoicePlay(charNo, voiceFlag);
                    workP->attr |= CHAR_ATTR_WIN_VOICE_PLAYED;
                    attrOld |= CHAR_ATTR_WIN_VOICE_PLAYED;
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_350):
            if(!(workP->attr & (CHAR_ATTR_WIN_VOICE_PLAYED | CHAR_ATTR_FX_OFF))) {
                if(frameNo == winAnimLen2[charNo]) {
                    CharWinVoicePlay(charNo, voiceFlag);
                    workP->attr |= CHAR_ATTR_WIN_VOICE_PLAYED;
                    attrOld |= CHAR_ATTR_WIN_VOICE_PLAYED;
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_363):
            if(!(workP->attr & (CHAR_ATTR_WIN_VOICE_PLAYED | CHAR_ATTR_FX_OFF))) {
                if(frameNo == holdUpAnimLen[charNo]) {
                    CharWinVoicePlay(charNo, voiceFlag);
                    workP->attr |= CHAR_ATTR_WIN_VOICE_PLAYED;
                    attrOld |= CHAR_ATTR_WIN_VOICE_PLAYED;
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_364):
            if(!(workP->attr & (CHAR_ATTR_WIN_VOICE_PLAYED | CHAR_ATTR_FX_OFF))) {
                if(frameNo == winJumpUpAnimLen[charNo]) {
                    fn_800591C8(charNo, 575, voiceFlag);
                    workP->attr |= CHAR_ATTR_WIN_VOICE_PLAYED;
                    attrOld |= CHAR_ATTR_WIN_VOICE_PLAYED;
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_352):
            if(!(workP->attr & (CHAR_ATTR_WIN_VOICE_PLAYED | CHAR_ATTR_FX_OFF))) {
                if(frameNo == loseAnimLen[charNo]) {
                    CharWinVoicePlay(charNo, voiceFlag);
                    workP->attr |= CHAR_ATTR_WIN_VOICE_PLAYED;
                    attrOld |= CHAR_ATTR_WIN_VOICE_PLAYED;
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_327):
            if(!(workP->attr & (CHAR_ATTR_WIN_VOICE_PLAYED | CHAR_ATTR_FX_OFF))) {
                if(GetMotNoPlayTime(workP, motNo) == 0) {
                    CharWinVoicePlay(charNo, voiceFlag);
                }
                workP->attr |= CHAR_ATTR_WIN_VOICE_PLAYED;
                attrOld |= CHAR_ATTR_WIN_VOICE_PLAYED;
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_307):
        case FILENUM(CHARMOT_HSF_c000m1_348):
        case FILENUM(CHARMOT_HSF_c000m1_351):
        case FILENUM(CHARMOT_HSF_c000m1_353):
        case FILENUM(CHARMOT_HSF_c000m1_354):
            if(frameNo == 0 && !(workP->attr & (CHAR_ATTR_MOTION_FLAG | CHAR_ATTR_FX_OFF))) {
                workP->attr |= CHAR_ATTR_MOTION_FLAG;
                attrOld |= CHAR_ATTR_MOTION_FLAG;
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_344):
            if(!(workP->attr & CHAR_ATTR_FX_OFF)) {
                if(frameNo & 1) {
                    fn_80060B10(charNo, 0, 0);
                }
                if(GetMotNoPlayTime(workP, motNo) == 0) {
                    fn_800591C8(charNo, 576, voiceFlag);
                }
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_370):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                fn_800591C8(charNo, 576, voiceFlag);
            }
            if(workP->timingHookNo != 0) {
                fn_80060B10(charNo, 30, 0.8f);
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_333):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                fn_800591C8(charNo, 584, voiceFlag);
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_311):
            if(frameNo == jumpAnimLen[charNo] && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                fn_8005F968(charNo, &modelP->pos);
                for(i=0; i<2; i++) {
                    if(frameNo == jumpVoiceTimeTbl[(charNo*2)+i]) {
                        PlayStepVoice(charNo, 185, voiceFlag);
                        break;
                    }
                }
            }

            break;

        case FILENUM(CHARMOT_HSF_c000m1_316):
        case FILENUM(CHARMOT_HSF_c000m1_318):
        case FILENUM(CHARMOT_HSF_c000m1_382):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                fn_800591C8(charNo, 582, voiceFlag);
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_357):
        case FILENUM(CHARMOT_HSF_c000m1_381):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                fn_800591C8(charNo, 588, voiceFlag);
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_361):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                fn_800591C8(charNo, 576, voiceFlag);
            }
            break;

        case FILENUM(CHARMOT_HSF_c000m1_377):
            if(GetMotNoPlayTime(workP, motNo) == 0 && !(workP->attr & CHAR_ATTR_FX_OFF)) {
                fn_800591C8(charNo, 581, voiceFlag);
            }
            break;
    }
    if(!motShiftF) {
        if(!(attrOld & CHAR_ATTR_MOTION_FLAG)) {
            workP->attr &= ~CHAR_ATTR_MOTION_FLAG;
        }
        if(!(attrOld & CHAR_ATTR_WIN_VOICE_PLAYED)) {
            workP->attr &= ~CHAR_ATTR_WIN_VOICE_PLAYED;
        }
    }
}

static s16 winVoiceTbl[][2] = {
    { 27, 579 },
    { 28, 579 },
    { 47, 579 },
    { 76, 579 },
    { 77, 579 },
    { 78, 579 },
    { 79, 579 },
    { 80, 579 },
    { 81, 579 },
    { -1, 0 },
};

static float CharYOfsTbl1[14] = { 110.0f, 160.0f, 110.0f, 160.0f, 150.0f, 180.0f, 130.0f,
                                          160.0f, 130.0f, 160.0f, 150.0f, 180.0f, 120.0f, 210.0f };
static float CharYOfsTbl2[14] = { 110.0f, 160.0f, 110.0f, 160.0f, 130.0f, 160.0f, 110.0f,
                                          160.0f, 130.0f, 160.0f, 130.0f, 160.0f, 130.0f, 160.0f };
static float birdYOfsTbl[14] = { 80.0f, 80.0f, 60.0f, 75.0f, 40.0f, 60.0f, 45.0f,
                                         90.0f, 70.0f, 75.0f, 90.0f, 75.0f, 75.0f, 75.0f };

static s16 stepVoiceTbl[] = {
    123, 0,
    124, 0,
    125, 0,
    126, 0,
    127, 0,
    128, 0,
    129, 0,
    130, 0,
    114, 0,
    115, 0,
    116, 0,
    93, 1,
    90, 1,
    89, 1,
    91, 1,
    94, 1,
    88, 1,
    108, 1,
    92, 1,
    99, 0,
    98, 0,
    96, 0,
    101, 0,
    97, 2,
    100, 0,
    102, 0,
    103, 0,
    104, 0,
    105, 0,
    107, 0,
    6, 0,
    7, 0,
    8, 0,
    9, 1,
    10, 0,
    11, 0,
    12, 0,
    13, 2,
    14, 1,
    15, 0,
    16, 0,
    17, 0,
    18, 2,
    19, 0,
    20, 0,
    21, 0,
    22, 1,
    23, 1,
    24, 0,
    25, 1,
    26, 2,
    27, 0,
    28, 0,
    29, 0,
    30, 1,
    31, 2,
    32, 1,
    33, 0,
    34, 1,
    35, 1,
    36, 1,
    37, 1,
    38, 0,
    39, 0,
    40, 2,
    41, 1,
    42, 1,
    43, 0,
    44, 0,
    45, 0,
    46, 0,
    47, 0,
    48, 0,
    49, 0,
    50, 0,
    51, 1,
    52, 1,
    53, 2,
    54, 1,
    55, 0,
    56, 0,
    57, 0,
    58, 0,
    59, 2,
    60, 0,
    61, 0,
    62, 1,
    63, 0,
    64, 0,
    65, 0,
    66, 0,
    67, 1,
    68, 2,
    69, 1,
    70, 0,
    71, 0,
    72, 0,
    73, 0,
    74, 0,
    75, 0,
    76, 0,
    77, 0,
    78, 0,
    79, 1,
    80, 0,
    81, 1,
    82, 0,
    83, 0,
    84, 0,
    85, 0,
    86, 1,
    87, 0,
    -1, -1,
};

// Plays a character sound at the model position or with its configured volume and pan; no value is
// assigned to this function's declared s16 return.
s16 fn_800591C8(s16 charNo, s16 seNo, u8 voiceFlag)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
    if(!(voiceFlag & CHAR_VOICE_OFF)) {
        if(workP->attr & CHAR_ATTR_VOICE_PAN_AUTO) {
            CharFXPlayPos(charNo, seNo, &modelP->pos);
        } else {
            CharFXPlayVolPan(charNo, seNo, workP->vol, workP->pan);
        }
    }
}

// Plays the win voice selected for the current minigame, falling back to the standard win voice.
// The playback is the useful result; ret is returned without being assigned in this function.
static s16 CharWinVoicePlay(s16 charNo, u8 voiceFlag)
{
    s16 seNo;
    s16 i;
    s16 ret;
    for(i=0; winVoiceTbl[i][0] != -1; i++) {
        if(omcurovl == winVoiceTbl[i][0]) {
            break;
        }
    }
    if(winVoiceTbl[i][0] != -1) {
        seNo = winVoiceTbl[i][1];
    } else {
        seNo = 577;
    }
    fn_800591C8(charNo, seNo, voiceFlag);
    return ret;
}

static void EffectParticleHook(HU3D_MODEL *modelP, HU3D_PARTICLE *particleP, Mtx mtx);

// Creates particle models for active cameras when character effects are first needed.
static void EffectInit(void)
{
    s16 effInitF = FALSE;
    s16 i;
    s16 cameraNo;
    ANIMDATA *anim[EFFECT_MAX];
    
    for(i=0; i<EFFECT_MAX; i++) {
        anim[i] = NULL;
    }
    for(cameraNo=0; cameraNo<16; cameraNo++) {
        if(-1.0f == Hu3DCamera[cameraNo].fov) {
            continue;
        }
        for(i=0; i<EFFECT_MAX; i++) {
            if(effModelId[i][cameraNo] != -1) {
                continue;
            }
            if(!anim[i]) {
                void *data =
                    fn_80008DD0(effectDataTbl[i].dataNum, 0x10000000, 2);
                anim[i] = HuSprAnimRead(data);
            }
            effModelId[i][cameraNo] = fn_8003216C(anim[i], effectDataTbl[i].maxCnt);
            if(i == EFFECT_BIRD) {
                fn_800327B8(effModelId[i][cameraNo], 0);
            }
            fn_800326D0(effModelId[i][cameraNo], EffectParticleHook);
            fn_80026B64(effModelId[i][cameraNo], (1 << cameraNo));
            if(!effParamAll[i]) {
                effParamAll[i] =
                    HuMemDirectMalloc(HEAP_HEAP, effectDataTbl[i].maxCnt * sizeof(EFFECTPARAM));
            }
            fn_8003269C(effModelId[i][cameraNo], effectDataTbl[i].blendMode);
            {
                HU3D_PARTICLE *particleP = lbl_802C01C8[effModelId[i][cameraNo]].hookData;
                HU3D_PARTICLE_DATA *particleDataP;
                s16 j;
                particleP->emitCnt = 0;
                particleP->work = effParamAll[i];
                particleP->count = 1;
                for (particleDataP = particleP->data, j = 0; j < particleP->maxCnt;
                     j++, particleDataP++) {
                    particleDataP->scale = 0;
                }
                effInitF = TRUE;
            }
        }
    }
    if(effInitF) {
        fn_800097F4(0x210000);
    }
}

// Ensures the requested particle effect has a model and particle storage for each active camera.
static void EffectParticleCreate(u8 type)
{
    ANIMDATA *anim = NULL;
    s16 cameraNo;
    HU3D_PARTICLE *particleP;
    
    for(cameraNo=0; cameraNo<16; cameraNo++) {
        if(-1.0f == Hu3DCamera[cameraNo].fov) {
            continue;
        }
        if(effModelId[type][cameraNo] != -1) {
            particleP = lbl_802C01C8[effModelId[type][cameraNo]].hookData;
            if(particleP->anim) {
                anim = particleP->anim;
                continue;
            }
        } else {
            if(!anim) {
                void *data =
                    fn_80008DD0(effectDataTbl[type].dataNum, 0x10000000, 2);
                anim = HuSprAnimRead(data);
            }
            effModelId[type][cameraNo] = fn_8003216C(anim, effectDataTbl[type].maxCnt);
            fn_80026B8C(effModelId[type][cameraNo], effectLayer);
            if(type == EFFECT_BIRD) {
                fn_800327B8(effModelId[type][cameraNo], 0);
            }
            fn_800326D0(effModelId[type][cameraNo], EffectParticleHook);
            fn_80026B64(effModelId[type][cameraNo], (1 << cameraNo));
            if(!effParamAll[type]) {
                effParamAll[type] =
                    HuMemDirectMalloc(HEAP_HEAP, effectDataTbl[type].maxCnt * sizeof(EFFECTPARAM));
            }
            fn_8003269C(effModelId[type][cameraNo], effectDataTbl[type].blendMode);
            {
                HU3D_PARTICLE_DATA *particleDataP;
                s16 j;
                particleP = lbl_802C01C8[effModelId[type][cameraNo]].hookData;
                particleP->emitCnt = 0;
                particleP->work = effParamAll[type];
                particleP->count = 1;
                for (particleDataP = particleP->data, j = 0; j < particleP->maxCnt;
                     j++, particleDataP++) {
                    particleDataP->scale = 0;
                }
            }
        }
    }
}

// Reserves a free particle slot on each requested active camera and initializes its motion data.
static s16 EffectCreate(s16 type, s16 cameraBit, float posX, float posY, float posZ, float scale,
                        EFFECTPARAM *param);

// Emits dust using the character model's camera and scales its motion with the model.
static s16 EffectDustCreate(s16 modelId, float posX, float posY, float posZ, float scale,
                            EFFECTPARAM *param)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    if(modelP->scale.x != 1.0) {
        EFFECTPARAM paramNew = *param;
        paramNew.vel.x *= modelP->scale.x;
        paramNew.vel.y *= modelP->scale.x;
        paramNew.vel.z *= modelP->scale.x;
        paramNew.scaleVel *= modelP->scale.x;
        scale *= modelP->scale.x;
        return EffectCreate(EFFECT_DUST, modelP->cameraBit, posX, posY, posZ, scale, &paramNew);
    } else {
        return EffectCreate(EFFECT_DUST, modelP->cameraBit, posX, posY, posZ, scale, param);
    }
}

// Emits smoke using the character model's camera and scales its motion with the model.
static s16 EffectSmokeCreate(HU3D_MODELID modelId, float posX, float posY, float posZ, float scale,
                             EFFECTPARAM *param)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    if(modelP->scale.x != 1.0) {
        EFFECTPARAM paramNew = *param;
        paramNew.vel.x *= modelP->scale.x;
        paramNew.vel.y *= modelP->scale.x;
        paramNew.vel.z *= modelP->scale.x;
        paramNew.scaleVel *= modelP->scale.x;
        scale *= modelP->scale.x;
        return EffectCreate(EFFECT_SMOKE, modelP->cameraBit, posX, posY, posZ, scale, &paramNew);
    } else {
        return EffectCreate(EFFECT_SMOKE, modelP->cameraBit, posX, posY, posZ, scale, param);
    }
}

// Emits a hit particle effect using the character model's camera and scale.
static s16 EffectHitCreate(HU3D_MODELID modelId, float posX, float posY, float posZ, float scale,
                           EFFECTPARAM *param)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    if(modelP->scale.x != 1.0) {
        EFFECTPARAM paramNew = *param;
        paramNew.vel.x *= modelP->scale.x;
        paramNew.vel.y *= modelP->scale.x;
        paramNew.vel.z *= modelP->scale.x;
        paramNew.scaleVel *= modelP->scale.x;
        scale *= modelP->scale.x;
        return EffectCreate(EFFECT_HIT, modelP->cameraBit, posX, posY, posZ, scale, &paramNew);
    } else {
        return EffectCreate(EFFECT_HIT, modelP->cameraBit, posX, posY, posZ, scale, param);
    }
}

// Emits star particles using the character model's camera and scale.
static inline s16 EffectStarCreate(HU3D_MODELID modelId, float posX, float posY, float posZ,
                                   float scale, EFFECTPARAM *param)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    if(modelP->scale.x != 1.0) {
        EFFECTPARAM paramNew = *param;
        paramNew.vel.x *= modelP->scale.x;
        paramNew.vel.y *= modelP->scale.x;
        paramNew.vel.z *= modelP->scale.x;
        paramNew.scaleVel *= modelP->scale.x;
        scale *= modelP->scale.x;
        return EffectCreate(EFFECT_STAR, modelP->cameraBit, posX, posY, posZ, scale, &paramNew);
    } else {
        return EffectCreate(EFFECT_STAR, modelP->cameraBit, posX, posY, posZ, scale, param);
    }
}

// Emits warning particles around the model and records model and height data for animation.
static inline s16 EffectWarnCreate(HU3D_MODELID modelId, float posX, float posY, float posZ,
                                   float scale, float ofsY, EFFECTPARAM *param)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    EFFECTPARAM paramNew = *param;
    s16 cameraNo, cameraBit;
    paramNew.vel.x *= modelP->scale.x;
    paramNew.vel.y *= modelP->scale.x;
    paramNew.vel.z *= modelP->scale.x;
    paramNew.scaleVel *= modelP->scale.x;
    scale *= modelP->scale.x;
    for(cameraNo=0, cameraBit=(1 << 0); cameraNo<16; cameraNo++, cameraBit <<= 1) {
        HU3D_MODEL *effModelP;
        HU3D_PARTICLE *particleP;
        HU3D_PARTICLE_DATA *particleDataP;
        s16 particleId;
        if(!(cameraBit & modelP->cameraBit)) {
            continue;
        }
        if(-1.0f == Hu3DCamera[cameraNo].fov) {
            continue;
        }
        particleId = EffectCreate(EFFECT_WARN, cameraBit, posX, posY, posZ, scale, &paramNew);
        if(particleId == -1) {
            return;
        } else {
            effModelP = &lbl_802C01C8[effModelId[EFFECT_WARN][cameraNo]];
            particleP = effModelP->hookData;
            particleDataP = &particleP->data[particleId];
            particleDataP->parManId = 0;
            particleDataP->vel.x = modelId;
            particleDataP->vel.y = ofsY;
            particleDataP->accel.x = particleDataP->accel.z = 0;
            particleDataP->accel.y = 100;
        }
    }
}

// Emits orbiting bird particles and records which character and head-height offset they follow.
static s16 EffectBirdCreate(HU3D_MODELID modelId, float posX, float posY, float posZ, float scale,
                            s16 charNo, float ofsY, EFFECTPARAM *param)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[modelId];
    EFFECTPARAM paramNew = *param;
    s16 cameraNo, cameraBit;
    paramNew.vel.x *= modelP->scale.x;
    paramNew.vel.y *= modelP->scale.x;
    paramNew.vel.z *= modelP->scale.x;
    paramNew.scaleVel *= modelP->scale.x;
    scale *= modelP->scale.x;
    for(cameraNo=0, cameraBit=(1 << 0); cameraNo<16; cameraNo++, cameraBit <<= 1) {
        HU3D_MODEL *effModelP;
        HU3D_PARTICLE *particleP;
        HU3D_PARTICLE_DATA *particleDataP;
        s16 particleId;
        if(!(cameraBit & modelP->cameraBit)) {
            continue;
        }
        if(-1.0f == Hu3DCamera[cameraNo].fov) {
            continue;
        }
        particleId = EffectCreate(EFFECT_BIRD, cameraBit, posX, posY, posZ, scale, &paramNew);
        if(particleId == -1) {
            return;
        } else {
            effModelP = &lbl_802C01C8[effModelId[EFFECT_BIRD][cameraNo]];
            particleP = effModelP->hookData;
            particleDataP = &particleP->data[particleId];
            particleDataP->parManId = 1;
            particleDataP->time = 0;
            particleDataP->vel.x = charNo;
            particleDataP->vel.y = ofsY;
        }
    }
}

static s16 EffectCreate(s16 type, s16 cameraBit, float posX, float posY, float posZ, float scale,
                        EFFECTPARAM *param)
{
    s16 cameraNo, bit;
    s16 id;
    EffectParticleCreate(type);
    id = -1;
    for(cameraNo=0, bit=(1 << 0); cameraNo<16; cameraNo++, bit <<= 1) {
        HU3D_PARTICLE_DATA *particleDataP;
        HU3D_PARTICLE *particleP;
        s16 nextId;
        HU3D_MODEL *modelP;
        EFFECTPARAM *effParam;
        s16 modelId;
        if(!(bit & cameraBit)) {
            continue;
        }
        if(-1.0f == Hu3DCamera[cameraNo].fov) {
            continue;
        }
        modelId = effModelId[type][cameraNo];
        if(modelId == -1) {
            return -1;
        }
        modelP = &lbl_802C01C8[modelId];
        particleP = modelP->hookData;
        effParam = particleP->work;
        for (particleDataP = &particleP->data[particleP->emitCnt], nextId = particleP->emitCnt;
             nextId < particleP->maxCnt; nextId++, particleDataP++) {
            if(!particleDataP->scale) {
                break;
            }
        }
        if(nextId >= particleP->maxCnt) {
            for (particleDataP = &particleP->data[0], nextId = 0; nextId < particleP->maxCnt;
                 nextId++, particleDataP++) {
                if(!particleDataP->scale) {
                    break;
                }
            }
        }
        if(nextId != particleP->maxCnt) {
            effParam[nextId] = *param;
            particleDataP->cameraBit = cameraBit;
            particleDataP->pos.x = posX;
            particleDataP->pos.y = posY;
            particleDataP->pos.z = posZ;
            particleDataP->vel = param->vel;
            particleDataP->color = param->colorBegin;
            particleDataP->scaleBase = scale;
            particleDataP->scale = scale;
            particleDataP->time = 0;
            particleDataP->parManId = -1;
            particleP->emitCnt = nextId;
            id = nextId;
        } else {
            id = -1;
        }
    }
    return id;
}

static void UpdateEffect(HU3D_PARTICLE_DATA *particleDataP);
static void UpdateBirdEffect(HU3D_PARTICLE_DATA *particleDataP);
static void UpdateModelEffect(HU3D_PARTICLE_DATA *particleDataP);

// Advances active particles each frame; Hu3DParticleHookSet installs this for effect models.
static void EffectParticleHook(HU3D_MODEL *modelP, HU3D_PARTICLE *particleP, Mtx mtx)
{
    EFFECTPARAM *param = particleP->work;
    HU3D_PARTICLE_DATA *particleDataP;
    s16 color;
    s16 i;
    if(particleP->count == 0) {
        for(particleDataP=&particleP->data[0], i=0; i<particleP->maxCnt; i++, particleDataP++) {
            particleDataP->scale = 0;
        }
    }
    for(particleDataP=&particleP->data[0], i=0; i<particleP->maxCnt; i++, particleDataP++) {
        if(!particleDataP->scale) {
            continue;
        }
        if(particleDataP->parManId == -1) {
            particleDataP->vel.x *= param[i].velDecay.x;
            particleDataP->vel.y *= param[i].velDecay.y;
            particleDataP->vel.z *= param[i].velDecay.z;
            VECAdd(&particleDataP->vel, &particleDataP->pos, &particleDataP->pos);
            particleDataP->vel.y += param[i].gravity;
            color = particleDataP->color.r +
                    (param[i].colorWeight * (param[i].colorEnd.r - param[i].colorBegin.r));
            if(color < 0) {
                color = 0;
            } else if(color > 255) {
                color = 255;
            }
            particleDataP->color.r = color;
            color = particleDataP->color.g +
                    (param[i].colorWeight * (param[i].colorEnd.g - param[i].colorBegin.g));
            if(color < 0) {
                color = 0;
            } else if(color > 255) {
                color = 255;
            }
            particleDataP->color.g = color;
            color = particleDataP->color.b +
                    (param[i].colorWeight * (param[i].colorEnd.b - param[i].colorBegin.b));
            if(color < 0) {
                color = 0;
            } else if(color > 255) {
                color = 255;
            }
            particleDataP->color.b = color;
            color = particleDataP->color.a+param[i].alphaBase;
            if(color < 1) {
                particleDataP->scale = 0;
            }
            particleDataP->color.a = color;
            if(particleDataP->scale) {
                if(param[i].attr & CHAR_EFFECT_SCALE_FLICKER) {
                    particleDataP->scale =
                        particleDataP->scaleBase * (((particleDataP->time + i) & 1) ? 1.0 : 0.5);
                } else {
                    particleDataP->scale = particleDataP->scaleBase;
                }
                particleDataP->scaleBase += param[i].scaleVel;
                if(particleDataP->scaleBase <= 0.01f) {
                    particleDataP->scale = 0;
                }
            }
            particleDataP->time++;
        } else {
            switch(particleDataP->parManId) {
                case 0:
                    UpdateEffect(particleDataP);
                    break;
                
                case 1:
                    UpdateBirdEffect(particleDataP);
                    break;
                    
                case 2:
                    UpdateModelEffect(particleDataP);
                    break;
                
            }
        }
    }
    DCStoreRangeNoSync(particleP->data, particleP->maxCnt*sizeof(HU3D_PARTICLE_DATA));
}

// Animates the warning ring around its source model and fades it after its hold period.
static void UpdateEffect(HU3D_PARTICLE_DATA *particleDataP)
{
    HU3D_MODEL *modelP = &lbl_802C01C8[(int)particleDataP->vel.x];
    float radius;
    if(particleDataP->time < 8) {
        radius = 0.3+sin(3.141592653589793*(40.0f+(10.0f*(particleDataP->time+1)))/180.0);
        particleDataP->scale = (50.0f*radius)*modelP->scale.x;
        particleDataP->color.a = 255;
        radius = 0.3+sin(3.141592653589793*(15.0f*(particleDataP->time+1))/180.0);
    } else {
        radius = 0.3+sin(3.141592653589793*135.0f/180.0);
    }
    radius *= modelP->scale.x;
    particleDataP->pos.x = modelP->pos.x+(particleDataP->accel.x*radius);
    particleDataP->pos.y = (modelP->pos.y + (particleDataP->vel.y * modelP->scale.x)) +
                           (particleDataP->accel.y * radius);
    particleDataP->pos.z = modelP->pos.z+(particleDataP->accel.z*radius);
    if(particleDataP->time > 20) {
        particleDataP->color.a -= 32;
        particleDataP->scale -= 8.0f*modelP->scale.x;
        if(particleDataP->scale < 0.0f) {
            particleDataP->scale = 0.0f;
        }
    }
    particleDataP->time++;
}

// Orbits bird particles around a character's head while a bird motion is playing, then fades them.
static void UpdateBirdEffect(HU3D_PARTICLE_DATA *particleDataP)
{
    s16 charNo = (s16)particleDataP->vel.x;
    s16 angle;
    CHARWORK *workP;
    HU3D_MODEL *modelP;
    s16 effectNo;
    BOOL birdMotPlayF;
    
    birdMotPlayF = TRUE;
    workP = &CharWork[charNo];
    modelP = &lbl_802C01C8[workP->modelId];
    
    for(effectNo=0; effectDataTbl[EFFECT_BIRD].motDataNum[effectNo]; effectNo++) {
        if(workP->motNoCurr == FILENUM(effectDataTbl[EFFECT_BIRD].motDataNum[effectNo])) {
            break;
        }
    }
    if(effectDataTbl[EFFECT_BIRD].motDataNum[effectNo] == 0) {
        birdMotPlayF = FALSE;
    }
    if(particleDataP->time < 20 && particleDataP->scale < 40.0f*modelP->scale.x) {
        particleDataP->scale += 4.0f*modelP->scale.x;
    }
    particleDataP->color.a = 255;
    angle = (particleDataP->time*5)%360;
    Hu3DModelObjPosGet(workP->modelId, CharHeadObjNameTbl[charNo], &particleDataP->pos);
    particleDataP->pos.x =
        particleDataP->pos.x
            + (modelP->scale.x * (40.0 * sin(3.141592653589793*(particleDataP->vel.y
            + angle)/180.0)));
    particleDataP->pos.y = particleDataP->pos.y+(modelP->scale.y*birdYOfsTbl[charNo]);
    particleDataP->pos.z =
        particleDataP->pos.z
            + (modelP->scale.z * (40.0 * cos(3.141592653589793*(particleDataP->vel.y
            + angle)/180.0)));
    particleDataP->time++;
    if(particleDataP->time >= 143) {
        particleDataP->time = 72;
    }
    if(!birdMotPlayF && particleDataP->time > 30) {
        particleDataP->scale -= 8.0f*modelP->scale.x;
        if(particleDataP->scale < 0.0f) {
            particleDataP->scale = 0.0f;
            workP->attr &= ~CHAR_ATTR_BIRD_ACTIVE;
            if(particleDataP->vel.y == 0.0) {
                fn_800591C8(charNo, 201, workP->attr);

            }
        }
    }
}

// Returns the footstep sound variation assigned to the currently running minigame.
static s16 GetStepType(void)
{
    s16 i;
    for(i=0; stepVoiceTbl[i] != -1; i++) {
        if(omcurovl == stepVoiceTbl[(i*2)+0]) {
            return stepVoiceTbl[(i*2)+1];
        }
    }
    return 0;
}

// Loads particle models for any effect motions referenced by the newly loaded motion resource.
static inline void MotionParticleInit(unsigned int dataNum)
{
    s16 i;
    s16 j;
    dataNum = FILENUM(dataNum);
    for(i=0; i<EFFECT_MAX; i++) {
        for(j=0; effectDataTbl[i].motDataNum[j]; j++) {
            if(dataNum == FILENUM(effectDataTbl[i].motDataNum[j])) {
                EffectParticleCreate(i);
                break;
            }
        }
    }
}

// Loads a character motion resource, creates its joint motion, and tracks its motion number.
HU3D_MOTIONID fn_8005B0B8(s16 charNo, unsigned int dataNum)
{
    CHARWORK *workP = &CharWork[charNo];
    s16 i;
    s16 motNo;
    void *data;
    unsigned int dir;
    if(workP->modelId == -1) {
        return -1;
    }
    for(motNo=0; motNo<32; motNo++) {
        if(workP->motId[motNo] == -1) {
            break;
        }
    }
    if(motNo == 32) {
        return -1;
    }
    dir = DIRNUM(dataNum);
    for(i=0; i<14; i++) {
        if(dir == lbl_8022E718[i][4]) {
            break;
        }
    }
    if(i != 14 || dir == 0) {
        dataNum = FILENUM(dataNum);
        data = fn_8005607C(DATANUM(lbl_8022E718[charNo][4], dataNum), 0x10000000,
                                       2);
        if(!data) {
            data = fn_80008DD0(DATANUM(lbl_8022E718[charNo][4], dataNum), 0x10000000,
                                        2);
        }
        workP->motNoTbl[motNo] = dataNum;
    } else {
        for(i=0; i<14; i++) {
            if(dir == lbl_8022E718[i][5]) {
                break;
            }
        }
        if(i != 14) {
            dataNum = DATANUM(lbl_8022E718[charNo][5], FILENUM(dataNum));
            data = fn_80009B04(dataNum, 0x10000000, 2);
        } else {
            data = fn_80008DD0(dataNum, 0x10000000, 2);
        }
        workP->motNoTbl[motNo] = -1;
    }
    workP->motId[motNo] = fn_80030458(workP->modelId, data);
    workP->voiceFlag[motNo] = 0;
    MotionParticleInit(dataNum);
    return workP->motId[motNo];
}

// Associates a loaded motion ID with its character motion number for motion tracking.
void fn_8005B398(s16 charNo, HU3D_MOTIONID motId, unsigned int motNo)
{
    CHARWORK *workP = &CharWork[charNo];
    s16 i;
    for(i=0; i<32; i++) {
        if(workP->motId[i] == motId) {
            break;
        }
    }
    if(i != 32) {
        workP->motNoTbl[i] = motNo;
    }
}

// Stops a character motion and removes its ID from the character's loaded-motion table.
void fn_8005B420(s16 charNo, unsigned int motId)
{
    CHARWORK *workP = &CharWork[charNo];
    s16 i;
    for(i=0; i<32; i++) {
        if(workP->motId[i] == motId) {
            break;
        }
    }
    workP->motId[i] = -1;
    fn_8002C624(motId);
    
}

// Closes the character motion and shared effect data directories for one character, or all
// characters.
void fn_8005B4BC(s16 charNo)
{
    s16 i;
    if(charNo == -1) {
        for(i=0; i<14; i++) {
            fn_8005B4BC(i);
        }
    } else {
        fn_800097F4(lbl_8022E718[charNo][4]);
        fn_800097F4(0x210000);
    }
}

// Closes the character model and motion directories plus shared effect data.
void fn_8005B7BC(s16 charNo)
{
    s16 i;
    if(charNo == -1) {
        for(i=0; i<14; i++) {
            fn_8005B7BC(i);
        }
    } else {
        for(i=0; i<5; i++) {
            fn_800097F4(lbl_8022E718[charNo][i]);
        }
        fn_800097F4(0x210000);
    }
}

// Stops a character's motions and removes its model; CHARNO_NONE applies cleanup to all characters.
void fn_8005BB8C(s16 charNo)
{
    s16 i;
    s16 cameraNo;
    if(charNo == -1) {
        for(i=0; i<14; i++) {
            fn_8005BB8C(i);
        }
        for(i=0; i<15; i++) {
            dustFlags[i] = 0;
        }
    } else {
        CHARWORK *workP;
        fn_8005B4BC(charNo);
        workP = &CharWork[charNo];
        for(i=0; i<32; i++) {
            if(workP->motId[i] != -1) {
                fn_8002C624(workP->motId[i]);
            }
            workP->motId[i] = -1;
            workP->motNoTbl[i] = -1;
        }
        if(workP->modelId != -1) {
            fn_800262D0(workP->modelId);
        }
        workP->modelId = -1;
        for(i=0; i<14; i++) {
            if(CharWork[i].modelId != -1) {
                break;
            }                
        }
        if(i == 14) {
            for(i=0; i<EFFECT_MAX; i++) {
                for(cameraNo=0; cameraNo<16; cameraNo++) {
                    if(effModelId[i][cameraNo] != -1) {
                        fn_800262D0(effModelId[i][cameraNo]);
                    }
                    effModelId[i][cameraNo] = -1;
                }
            }
            effectLayer = 0;
            for(i=0; i<EFFECT_MAX; i++) {
                if(effParamAll[i]) {
                    HuMemDirectFree(effParamAll[i]);
                }
                effParamAll[i] = NULL;
            }
        }
        if(workP->process) {
            HuMemDirectFree(workP->process->property);
            HuPrcKill(workP->process);
            for(i=0; i<32; i++) {
                if(hookDustProc[i]) {
                    HuPrcKill(hookDustProc[i]);
                }
                hookDustProc[i] = NULL;
            }
            workP->process = NULL;
        }
        workP->vol = MSM_VOL_MAX;
        workP->pan = MSM_PAN_CENTER;
    }
}

// Gameplay and menu animation controllers call this to switch a character model's motion.
void fn_8005BE80(s16 charNo, HU3D_MOTIONID motId)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MOTION *motP = &lbl_802624B8[motId];
    EyeBmpUpdate(charNo);
    fn_8002C95C(workP->modelId, motId);
}

// Called before a motion switch to clear translation and rotation animation transforms on the
// model's eye bitmaps.
static void EyeBmpUpdate(s16 charNo)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
    HSF_ATTRIBUTE *attrP = modelP->hsf->attribute;
    s16 i;
    s16 modelBit;
    char **eyeBmp;
    for(i=0, modelBit=1; i<4; i++, modelBit <<= 1) {
        if(modelBit & workP->model) {
            break;
        }
    }
    if(i >= 4) {
        return;
    }
    eyeBmp = fn_8005C074(charNo, workP->model);
    for(i=0; i<modelP->hsf->attributeNum; i++, attrP++) {
        if(attrP->bitmap->name[0] != eyeBmp[0][0] || strcmp(attrP->bitmap->name, eyeBmp[0])) {
            if(attrP->bitmap->name[0] != eyeBmp[1][0] || strcmp(attrP->bitmap->name, eyeBmp[1])) {
                continue;
            }
        }
        if(attrP->animWorkP) {
            HU3D_ATTR_ANIM *attrAnimP = attrP->animWorkP;
            attrAnimP->trans3D.x = attrAnimP->trans3D.y = attrAnimP->trans3D.z = 0;
            attrAnimP->rot.x = attrAnimP->rot.y = attrAnimP->rot.z = 0;
        }
    }
}

char *CharEyeBmpNameTbl[14 * 4 * 2] = {
    "s3c000m1_eyes", "s3c000m1_eyes", "s3c000m1_eyes", "s3c000m1_eyes",
    "s3c000m2_eyes", "s3c000m2_eyes", "mario_eyes", "mario_eyes",
    "S3c001m0_eye", "S3c001m0_eye", "S3c001m1_eye", "S3c001m1_eye",
    "c001m3_eye", "c001m3_eye", "c001m3_eye", "c001m3_eye",
    "s3c002m0_r_eye", "s3c002m0_l_eye", "s3c002m1_r_eye", "s3c002m1_l_eye",
    "s3c002m2_r_eye", "s3c002m2_l_eye", "", "",
    "", "", "", "",
    "", "", "S3c003m3", "S3c003m3",
    "s3c004m0_eye", "s3c004m0_eye", "s3c004m1_eye", "s3c004m1_eye",
    "s3c004m3_eye", "s3c004m3_eye", "s3c004m3_eye", "s3c004m3_eye",
    "s3c007m0_Eye_L", "s3c007m0_Eye_R", "s3c007m1_Eye_L", "s3c007m1_Eye_R",
    "s3c006m2_eye", "s3c006m2_eye_R", "", "",
    "s3c007_m0_eye", "s3c007_m0_eye", "s3c007_m1_eye", "s3c007_m1_eye",
    "s3c007_m2_eye", "s3c007_m2_eye", "s3c007_m3_eye", "s3c007_m3_eye",
    "c008m1_eyes1", "c008m1_eyes1", "c008m1_eyes1", "c008m1_eyes1",
    "c008m1_eyes1", "c008m1_eyes1", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "c011m1_eyes1", "c011m1_eyes1", "c011m1_eyes1", "c011m1_eyes1",
    "c008m1_eyes1", "c008m1_eyes1", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", "",
};

// Returns the two eye-bitmap names assigned to this character model variant.
char **fn_8005C074(s16 charNo, s16 model)
{
    s16 i;
    s16 bit;
    for(i=0, bit=(1 << 0); i<4-1; i++, bit <<= 1) {
        if(bit & model) {
            break;
        }
    }
    return &CharEyeBmpNameTbl[(charNo*(4*2))+(i*2)];
}

static char *CharHookNameTbl[14*5] = {
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
    "a-itemhook-r", "a-itemhook-l", "a-itemhook-fr", "a-itemhook-fl", "a-itemhook-body",
};

// Returns the named attachment point used for an item on a character model.
char *fn_8005C0F0(s16 charNo, s16 model, s16 hookNo)
{
    s16 i;
    s16 bit;
    for(i=0, bit=(1 << 0); i<4-1; i++, bit <<= 1) {
        if(bit & model) {
            break;
        }
    }
    return CharHookNameTbl[(charNo*5)+hookNo];
}

void fn_8005C16C(s16 charNo, float time)
{
    CHARWORK *workP = &CharWork[charNo];
    fn_8002D414(workP->modelId, time);
}

float fn_8005C1B8(s16 charNo)
{
    CHARWORK *workP = &CharWork[charNo];
    return fn_8002D4E0(workP->modelId);
}

float fn_8005C1FC(s16 charNo)
{
    CHARWORK *workP = &CharWork[charNo];
    return fn_8002D530(workP->modelId);
}

BOOL fn_8005C240(s16 charNo)
{
    CHARWORK *workP = &CharWork[charNo];
    return fn_8002D75C(workP->modelId);
}

HU3D_MOTIONID fn_8005C284(s16 charNo)
{
    CHARWORK *workP = &CharWork[charNo];
    return fn_8002D3EC(workP->modelId);
}

// Animation controllers call this to blend the character into another motion over a time range.
void fn_8005C2C8(s16 charNo, HU3D_MOTIONID motId, float start, float end, u32 attr)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MOTION *motP = &lbl_802624B8[motId];
    fn_8002CCC0(workP->modelId, motId, start, end, attr);
}

float fn_8005C34C(s16 charNo)
{
    CHARWORK *workP = &CharWork[charNo];
    return fn_8002D508(workP->modelId);
}

void fn_8005C390(s16 charNo, float speed)
{
    CHARWORK *workP = &CharWork[charNo];
    fn_8002D86C(workP->modelId, speed);
}

void fn_8005C3DC(s16 charNo, float start, float end)
{
    CHARWORK *workP = &CharWork[charNo];
    fn_8002D730(workP->modelId, start, end);
}

void fn_8005C430(s16 charNo, u32 attr)
{
    CHARWORK *workP = &CharWork[charNo];
    fn_800269E8(workP->modelId, attr);
}

void fn_8005C47C(s16 charNo, u32 attr)
{
    CHARWORK *workP = &CharWork[charNo];
    fn_80026A44(workP->modelId, attr);
}

static float modelHeightTbl[14] = {
    150.0f, 170.0f, 188.0f, 169.0f, 164.0f, 180.0f, 210.0f,
    120.0f, 150.0f, 150.0f, 120.0f, 150.0f, 150.0f, 150.0f
};

float fn_8005C4C8(s16 charNo)
{
    return modelHeightTbl[charNo];
}

HU3D_MODELID fn_8005C4E4(s16 charNo, s16 model, unsigned int *motDataNum,
                                    HU3D_MOTIONID *motId)
{
    HU3D_MODELID modelId = fn_80057068(charNo, model);
    s16 no = 0;
    do {
        if(motDataNum[no] == 0) {
            break;
        }
        motId[no] = fn_8005B0B8(charNo, motDataNum[no]);
        no++;
    } while(1);
    fn_8005B4BC(charNo);
    return modelId;
}

// Returns one of the character model's three timing-hook slots for animation callbacks.
s32 fn_8005C750(s16 charNo)
{
    CHARWORK *workP = &CharWork[charNo];
    if(workP->timingHookNo == 0) {
        return 0;
    } else if(workP->timingHookNo == 1) {
        return 1;
    } else {
        return 2;
    }
}

typedef struct HookDustWork_s {
    u16 cameraBit;
    HU3D_MODELID modelId;
} HOOKDUSTWORK;

static void CreateHookDust(void);

// Reserves a process slot for the short hook-dust animation started by CharModelHookDustCreate.
static inline HUPROCESS *CreateHookDustProc(void)
{
    s16 i;
    for(i=0; i<32; i++) {
        if(!hookDustProc[i]) {
            break;
        }
    }
    if(i == 32) {
        return NULL;
    }
    hookDustProc[i] = fn_8000D818(CreateHookDust, 100, 8192, 0);
    return hookDustProc[i];
}

// Stops a hook-dust process and releases its entry in the shared process table.
static inline void KillHookDustProc(HUPROCESS *process)
{
    s16 i;
    for(i=0; i<32; i++) {
        if(hookDustProc[i] == process) {
            HuPrcKill(process);
            hookDustProc[i] = NULL;
        }
    }
}

// Called when an attached object is removed to animate its model upward and burst it into dust.
void fn_8005C7A4(s16 charNo, char *objName)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
    HSF_OBJECT *objPtr = fn_80026BF4(workP->modelId, objName);
    HSF_CONSTDATA *constData;
    HOOKDUSTWORK *hookDustWork;
    HU3D_MODELID hookMdlId;
    HUPROCESS *process;
    Mtx hookMtx;
    HuVecF temp;
    
    fn_80023398(workP->modelId, objName, hookMtx);
    constData = objPtr->constData;
    hookMdlId = constData->hookMdlId;
    if(hookMdlId == -1) {
        return;
    }
    fn_800275B4(workP->modelId, objName);
    process = CreateHookDustProc();
    if(!process) {
        fn_800269E8(hookMdlId, (1 << 0));
        return;
    }
    process->property = hookDustWork =
        HuMemDirectMallocNum(HEAP_HEAP, sizeof(HOOKDUSTWORK), 0x10000000);
    modelP = &lbl_802C01C8[hookMdlId];
    Hu3DMtxTransGet(hookMtx, &temp);
    fn_80026790(hookMdlId, &temp);
    Hu3DMtxRotGet(hookMtx, &temp);
    fn_80022E38(modelP->mtx, temp.x, temp.y, temp.z);
    Hu3DMtxScaleGet(hookMtx, &temp);
    fn_800268E0(hookMdlId, &temp);
    hookDustWork->modelId = hookMdlId;
    hookDustWork->cameraBit = modelP->cameraBit;
}

// The hook-dust process shrinks and lifts the detached model, then emits dust particles.
static void CreateHookDust(void)
{
    HOOKDUSTWORK *hookDustWork = fn_8000DD9C()->property;
    HU3D_MODEL *modelP = &lbl_802C01C8[hookDustWork->modelId];
    Mtx rootMtx;
    HuVecF pos;
    s16 i;
    fn_80023398(hookDustWork->modelId, "", rootMtx);
    pos.x = lbl_8025FE04.x+((lbl_8025FDF8.x-lbl_8025FE04.x)/2);
    pos.y = lbl_8025FE04.y+((lbl_8025FDF8.y-lbl_8025FE04.y)/2);
    pos.z = lbl_8025FE04.z+((lbl_8025FDF8.z-lbl_8025FE04.z)/2);
    for(i=0; i<40; i++) {
        modelP->pos.y += 4.0f;
        mtxRotCat(modelP->mtx, 24, 0, 0);
        modelP->scale.x *= 0.95f;
        modelP->scale.y *= 0.95f;
        modelP->scale.z *= 0.95f;
        HuPrcVSleep();
    }
    fn_80023398(hookDustWork->modelId, "", rootMtx);
    pos.x = lbl_8025FE04.x+((lbl_8025FDF8.x-lbl_8025FE04.x)/2);
    pos.y = lbl_8025FE04.y+((lbl_8025FDF8.y-lbl_8025FE04.y)/2);
    pos.z = lbl_8025FE04.z+((lbl_8025FDF8.z-lbl_8025FE04.z)/2);
    fn_800269E8(hookDustWork->modelId, (1 << 0));
    dustEffParam.vel.x = 0;
    dustEffParam.vel.y = 0;
    dustEffParam.vel.z = 0;
    EffectDustCreate(hookDustWork->modelId, pos.x, pos.y, pos.z, 40.0f, &dustEffParam);
    for(i=0; i<8; i++) {
        dustEffParam.vel.x = frandmod(10)-5;
        dustEffParam.vel.y = frandmod(10)-5;
        dustEffParam.vel.z = frandmod(10)-5;
        EffectDustCreate(hookDustWork->modelId, pos.x, pos.y, pos.z, 20.0f, &dustEffParam);
    }
    MTXIdentity(modelP->mtx);
    KillHookDustProc(fn_8000DD9C());
    while(1) {
        HuPrcVSleep();
    }
}

static EFFECTPARAM modelSmokeEffParam = {
    0,
    { 255, 255, 255, 255 },
    { 64, 32, 0, 255 },
    { 0, 2, 1 },
    { 0.95f, 0.95f, 0.95f },
    0,
    0,
    1,
    -5,
    0.02f
};

// Creates the expanding smoke burst used by gameplay effects, scaled around the supplied position.
void fn_8005CF90(s16 cameraBit, HuVecF *pos, float scale)
{
    HU3D_PARTICLE_DATA *particleDataP;
    s16 i;
    s16 effectNo;
    HU3D_PARTICLE *particleP;
    HU3D_MODEL *modelP;
    s16 bit;
    s16 cameraNo;
    for(bit=(1 << 0), cameraNo=0; cameraNo<16; cameraNo++, bit <<= 1) {
        if(cameraBit & bit) {
            for(i=0; i<8; i++) {
                effectNo = EffectCreate(EFFECT_SMOKE, bit, pos->x, pos->y, pos->z, 20 * scale,
                                        &modelSmokeEffParam);
                if(effectNo == -1) {
                    break;
                }
                modelP = &lbl_802C01C8[effModelId[EFFECT_SMOKE][cameraNo]];
                particleP = modelP->hookData;
                particleDataP = &particleP->data[effectNo];
                particleDataP->parManId = 2;
                particleDataP->vel.x = scale*(30*sin(3.141592653589793*(i*45)/180.0));
                particleDataP->vel.y = scale*(30*cos(3.141592653589793*(i*45)/180.0));
                particleDataP->vel.z = 0;
                particleDataP->accel = *pos;
                particleDataP->speedDecay = 1.15f;
                particleDataP->colorIdx = scale*(0.1f*(frandmod(20)-10));
                particleDataP->color.a = 255-(frandmod(3)*16);
            }
            for(i=0; i<8; i++) {
                effectNo = EffectCreate(EFFECT_SMOKE, bit, pos->x, pos->y, pos->z, 10 * scale,
                                        &modelSmokeEffParam);
                if(effectNo == -1) {
                    break;
                }
                modelP = &lbl_802C01C8[effModelId[EFFECT_SMOKE][cameraNo]];
                particleP = modelP->hookData;
                particleDataP = &particleP->data[effectNo];
                particleDataP->parManId = 2;
                particleDataP->vel.x = scale*(frandmod(100)-50);
                particleDataP->vel.y = scale*(frandmod(100)-50);
                particleDataP->vel.z = scale*(frandmod(100)-50);
                particleDataP->accel = *pos;
                particleDataP->speedDecay = 1.15f;
                particleDataP->colorIdx = scale*(0.1f*(frandmod(20)-10));
                particleDataP->color.a = 255-(frandmod(3)*16);
            }
            effectNo = EffectCreate(EFFECT_SMOKE, bit, pos->x, pos->y, pos->z, 10 * scale,
                                    &modelSmokeEffParam);
            if(effectNo == -1) {
                break;
            }
            modelP = &lbl_802C01C8[effModelId[EFFECT_SMOKE][cameraNo]];
            particleP = modelP->hookData;
            particleDataP = &particleP->data[effectNo];
            particleDataP->parManId = 2;
            particleDataP->vel.x = 0;
            particleDataP->vel.y = 0;
            particleDataP->vel.z = 0;
            particleDataP->accel = *pos;
            particleDataP->colorIdx = 0;
            particleDataP->speedDecay = 1.15f;
            particleDataP->color.a = 255;
        }
    }
}

void fn_8005D404(s16 cameraBit, HuVecF *pos)
{
    fn_8005CF90(cameraBit, pos, 1.0f);
}

// Particle update callback moves a model-attached effect outward, grows it, then fades it away.
static void UpdateModelEffect(HU3D_PARTICLE_DATA *particleDataP)
{
    float speed;
    float angle;
    angle = 20+(3.75f*particleDataP->time);
    if(angle > 90.0f) {
        angle = 90.0f;
    }
    speed = sin(3.141592653589793*angle/180.0);
    particleDataP->pos.x = particleDataP->accel.x+(particleDataP->vel.x*speed);
    particleDataP->pos.y = particleDataP->accel.y+(particleDataP->vel.y*speed);
    particleDataP->pos.z = particleDataP->accel.z+(particleDataP->vel.z*speed);
    particleDataP->scale *= particleDataP->speedDecay;
    particleDataP->speedDecay -= 0.01;
    if(particleDataP->speedDecay < 1.0f) {
        particleDataP->speedDecay = 1.0f;
    }
    if(particleDataP->time > 8) {
        s16 alpha = particleDataP->color.a;
        alpha -= 8;
        if(alpha < 0) {
            particleDataP->color.a = 0;
            particleDataP->scale = 0;
        } else {
            particleDataP->color.a = alpha;
        }
    }
    particleDataP->time++;
}

static EFFECTPARAM coinEffParam = {
    1,
    { 255, 255, 0, 255 },
    { 255, 255, 0, 255 },
    { 0, 2, 1 },
    { 0.95f, 1.0f, 0.95f },
    -0.1f,
    0,
    -0.2f,
    -8,
    0
};

// Emits a ring of yellow glow particles when a coin-related character effect is shown.
void fn_8005D5A0(s16 cameraBit, HuVecF *pos)
{
    s16 i;
    for(i=0; i<16; i++) {
        s16 angle = i*(360.0f/16.0f);
        s16 effectNo;
        coinEffParam.vel.x = 5*sin(3.141592653589793*angle/180.0);
        coinEffParam.vel.y = 0.1f*(frandmod(100)-50);
        coinEffParam.vel.z = 5*cos(3.141592653589793*angle/180.0);
        effectNo = EffectCreate(EFFECT_GLOW, cameraBit, pos->x, pos->y, pos->z, 30, &coinEffParam);
        if(effectNo == -1) {
            break;
        }
    }
}

static EFFECTPARAM modelHitEffParam = {
    0,
    { 255, 255, 0, 255 },
    { 255, 128, 0, 255 },
    { 0, 2, 1 },
    { 0.95f, 0.95f, 0.95f },
    0,
    0,
    -0.2f,
    -16,
    0.05f
};

static EFFECTPARAM hitGlowEffParam = {
    0,
    { 224, 255, 32, 255 },
    { 224, 255, 32, 255 },
    { 0, 2, 1 },
    { 1.0f, 1.0f, 1.0f },
    0,
    0,
    -0.2f,
    -12,
    0.05f
};

// Called at the character's hit hook to place the impact burst just in front of the model.
void fn_8005D738(s16 charNo)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
    s16 motNo;
    Mtx mtx;
    HuVecF pos;
    HuVecF radius;
    
    for(motNo=0; motNo<32; motNo++) {
        if(workP->motId[motNo] == modelP->motId) {
            break;
        }
    }
    if(motNo == 32) {
        return;
    }
    fn_80023398(workP->modelId, "test11_tex_we-itemhook-r", mtx);
    pos.x = mtx[0][3];
    pos.y = mtx[1][3];
    pos.z = mtx[2][3];
    fn_80022E38(mtx, modelP->rot.x, modelP->rot.y, modelP->rot.z);
    radius.x = mtx[0][2];
    radius.y = mtx[1][2];
    radius.z = mtx[2][2]; 
    VECScale(&radius, &radius, 20);
    VECAdd(&pos, &radius, &pos);
    fn_8005D864(modelP->cameraBit, &pos, &modelP->rot);
}

// Creates the directional star and glow burst used for a character impact.
void fn_8005D864(s16 cameraBit, HuVecF *pos, HuVecF *rot)
{
    Mtx mtx;
    HuVecF radius;
    HuVecF dir;
    HuVecF vel;
    s16 i;
    s16 effectNo;
    float angle;
    fn_80022E38(mtx, rot->x, rot->y, rot->z);
    radius.x = mtx[0][2];
    radius.y = mtx[1][2];
    radius.z = mtx[2][2];
    for(i=0; i<8; i++) {
        angle = i*45;
        dir.x = sin(3.141592653589793*angle/180.0);
        dir.y = cos(3.141592653589793*angle/180.0);
        dir.z = 0.0f;
        VECNormalize(&dir, &dir);
        MTXMultVec(mtx, &dir, &dir);
        VECScale(&dir, &modelHitEffParam.vel, 10);
        effectNo =
            EffectCreate(EFFECT_STAR, cameraBit, pos->x, pos->y, pos->z, 30, &modelHitEffParam);
        if(effectNo == -1) {
            break;
        }
        VECScale(&radius, &vel, -3-(0.1*frandmod(20)));
        VECScale(&dir, &dir, 4);
        VECAdd(&dir, &vel, &hitGlowEffParam.vel);
        effectNo =
            EffectCreate(EFFECT_HIT, cameraBit, pos->x, pos->y, pos->z, 20, &hitGlowEffParam);
        if(effectNo == -1) {
            break;
        }
    }
}

static EFFECTPARAM shoeHitEffParam = {
    0,
    { 32, 32, 255, 255 },
    { 128, 255, 32, 255 },
    { 0, 2, 1 },
    { 0.95f, 0.95f, 0.95f },
    0,
    0,
    -0.2f,
    -16,
    0.06f
};

static EFFECTPARAM shoeHitGlowEffParam = {
    0,
    { 32, 224, 32, 255 },
    { 32, 224, 32, 255 },
    { 0, 2, 1 },
    { 1.0f, 1.0f, 1.0f },
    0,
    0,
    -0.2f,
    -12,
    0.05f
};

// Called at the right-shoe hook to place the shoe-impact burst in front of the character.
void fn_8005DA5C(s16 charNo)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
    Mtx mtx;
    HuVecF pos;
    HuVecF radius;

    fn_80023398(workP->modelId, "test11_tex_we-ske_R_shoe1", mtx);
    pos.x = mtx[0][3];
    pos.y = mtx[1][3];
    pos.z = mtx[2][3];
    fn_80022E38(mtx, modelP->rot.x, modelP->rot.y, modelP->rot.z);
    radius.x = mtx[0][2];
    radius.y = mtx[1][2];
    radius.z = mtx[2][2]; 
    VECScale(&radius, &radius, 30);
    VECAdd(&pos, &radius, &pos);
    fn_8005DB3C(modelP->cameraBit, &pos, &modelP->rot);
}

// Creates the star and glow particles for a shoe impact, aimed around the shoe's forward axis.
void fn_8005DB3C(s16 cameraBit, HuVecF *pos, HuVecF *rot)
{
    Mtx mtx;
    HuVecF radius;
    HuVecF dir;
    HuVecF vel;
    s16 i;
    s16 effectNo;
    fn_80022E38(mtx, rot->x, rot->y, rot->z);
    radius.x = mtx[0][2];
    radius.y = mtx[1][2];
    radius.z = mtx[2][2]; 
    for(i=0; i<8; i++) {
        float angle = i*45;
        VECNormalize(&radius, &radius);
        dir.x =
            ((radius.x*radius.y)*(1-cos(3.141592653589793*angle/180.0)))
            -(radius.z*sin(3.141592653589793*angle/180.0));
        dir.y = (radius.y*radius.y)+((1-(radius.y*radius.y))*cos(3.141592653589793*angle/180.0));
        dir.z =
            ((radius.y*radius.z)*(1-cos(3.141592653589793*angle/180.0)))
            +(radius.x*sin(3.141592653589793*angle/180.0));
        VECNormalize(&dir, &dir);
        VECScale(&dir, &shoeHitEffParam.vel, 10);
        effectNo =
            EffectCreate(EFFECT_STAR, cameraBit, pos->x, pos->y, pos->z, 20, &shoeHitEffParam);
        if(effectNo == -1) {
            break;
        }
        VECScale(&radius, &vel, -2-(0.1*frandmod(20)));
        VECScale(&dir, &dir, 2);
        VECAdd(&dir, &vel, &shoeHitGlowEffParam.vel);
        effectNo =
            EffectCreate(EFFECT_HIT, cameraBit, pos->x, pos->y, pos->z, 20, &shoeHitGlowEffParam);
        if(effectNo == -1) {
            break;
        }
    }
}

// Moves every active character-effect model to the requested render layer.
void fn_8005DDEC(s16 layerNo)
{
    s16 i, j;
    for(i=0; i<EFFECT_MAX; i++) {
        for(j=0; j<16; j++) {
            if(effModelId[i][j] != -1) {
                fn_80026B8C(effModelId[i][j], layerNo);
            }
        }
    }
    effectLayer = layerNo;
}

// Character setup and event scripts use this to enable or suppress voice playback for one motion.
void fn_8005DEB0(s16 charNo, s16 motNo, BOOL voiceOn)
{
    CHARWORK *workP = &CharWork[charNo];
    s16 i;
    if(workP->modelId == -1) {
        return;
    }
    motNo = FILENUM(motNo);
    for(i=0; i<32; i++) {
        if(workP->motNoTbl[i] == motNo) {
            break;
        }
    }
    if(i == 32) {
        return;
    }
    if(!voiceOn) {
        workP->voiceFlag[i] |= CHAR_VOICE_OFF;
    } else {
        workP->voiceFlag[i] &= ~CHAR_VOICE_OFF;
    }
}

// Enables or disables automatic stereo panning for voice lines emitted by this character.
void fn_8005DF74(s16 charNo, BOOL voicePanAuto)
{
    CHARWORK *workP = &CharWork[charNo];
    if(voicePanAuto) {
        workP->attr |= CHAR_ATTR_VOICE_PAN_AUTO;
    } else {
        workP->attr &= ~CHAR_ATTR_VOICE_PAN_AUTO;
    }
}

// Character and NPC controllers use this flag to suppress or restore their sound and effect cues.
void fn_8005DFC0(s16 charNo, BOOL fxFlag)
{
    CHARWORK *workP = &CharWork[charNo];
    if(charNo >= 14) {
        if(!fxFlag) {
            dustFlags[charNo] |= CHAR_ATTR_FX_OFF;
        } else {
            dustFlags[charNo] &= ~CHAR_ATTR_FX_OFF;
        }
        return;
    } else {
        if(!fxFlag) {
            workP->attr |= CHAR_ATTR_FX_OFF;
        } else {
            workP->attr &= ~CHAR_ATTR_FX_OFF;
        }
    }
}

// Sets whether the selected character motion updates its voice and motion events while playing.
void fn_8005E070(s16 charNo, unsigned int dataNum, BOOL updateF)
{
    CHARWORK *workP = &CharWork[charNo];
    s16 i;
    if(charNo >= 14) {
        return;
    }
    dataNum = FILENUM(dataNum);
    for(i=0; i<32; i++) {
        if(workP->motNoTbl[i] == dataNum) {
            break;
        }
    }
    if(i == 32) {
        return;
    }
    if(updateF) {
        workP->voiceFlag[i] &= ~CHAR_MOTION_UPDATE_OFF;
    } else {
        workP->voiceFlag[i] |= CHAR_MOTION_UPDATE_OFF;
    }
}

typedef struct NpcDustWork_s {
    HU3D_MODELID modelId; // Model whose motion drives the dust effect.
    HU3D_MOTIONID motId; // Motion that must be active for effects to run.
    s16 type; // Dust and sound pattern selected by UpdateNpcDust.
    s16 npcNo; // NPC identity, or CHAR_NPC_NONE to suppress NPC sounds.
} NPCDUSTWORK;

static void UpdateNpcDust(void);

// Starts motion-timed dust and sound effects for an NPC or character.
s32 fn_8005E128(HU3D_MODELID modelId, HU3D_MOTIONID motId, s16 type, s16 npcNo)
{
    HUPROCESS *parent = fn_8000DD9C();
    HUPROCESS *process = fn_8000DAC0(UpdateNpcDust, 100, 8192, 0, parent);
    if(!process) {
        return;
    } else {
        NPCDUSTWORK *work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(NPCDUSTWORK), 0x10000000);
        process->property = work;
        work->modelId = modelId;
        work->motId = motId;
        work->type = type;
        work->npcNo = npcNo;
        EffectInit();
    }
}

// Starts the same motion-timed dust effect without associating an NPC footstep sound.
s32 fn_8005E1D8(HU3D_MODELID modelId, HU3D_MOTIONID motId, s16 type)
{
    s32 ret;
    fn_8005E128(modelId, motId, type, -1);
    return ret;
}

static s8 npcSeTimeTbl[12] = { 7, 32, 1, 30, 1, 30, 1, 20, 19, 32, 2, 19 };

static u16 npcSeTbl[12] = { 177, 177, 177, 177, 177, 177, 288, 2590, 286, 273, 59367, 59367 };

static u16 npcSeTbl2[7] = { 181, 181, 181, 181, 181, 181, 0 };

static s8 npcSeTimeTbl2[8] = { 3, 55, -25, -25, -25, -25, -25, -25 };

static u16 npcSeTbl3[4] = { 177, 177, 177, 177 };

static s8 npcSeTimeTbl3[8] = { 4, 17, -25, -25, -25, -25, -25, -25 };

static u16 npcSeTbl4[4] = { 181, 181, 181, 181 };

static s8 npcSeTimeTbl4[8] = { 1, 23, -25, -25, -25, -25, -25, -25 };

static u16 npcSeTbl5[4] = { 177, 177, 177, 177 };

// Updates dust and NPC footstep sounds each frame while the selected motion is active.
static void UpdateNpcDust(void)
{
    NPCDUSTWORK *work = fn_8000DD9C()->property;
    HU3D_MODELID modelId = work->modelId;
    s16 time = 0;
    HU3D_MODEL *modelP = &lbl_802C01C8[work->modelId];
    s16 npcNo = work->npcNo-15;
    Vec pos;
    s16 i;
    while(1) {
        HuPrcVSleep();
        if(fn_8002D3C4(modelId) != work->motId) {
            continue;
        }
        if(modelP->attr & (1 << 0)) {
            continue;
        }
        time = fn_8002D4E0(modelId);
        switch(work->type) {
            case 0:
                if((time & (16 - 1)) == 0) {
                    if(!(dustFlags[npcNo] & CHAR_ATTR_FX_OFF)) {
                        dustEffParam.vel.x = -sin(3.141592653589793*modelP->rot.y/180.0)*2;
                        dustEffParam.vel.y = 1+(0.1*frandmod(10));
                        dustEffParam.vel.z = -cos(3.141592653589793*modelP->rot.y/180.0)*2;
                        pos.x = modelP->pos.x+(frandmod(50)-25);
                        pos.y = modelP->pos.y;
                        pos.z = modelP->pos.z+(frandmod(50)-25);
                        EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10) + 30,
                                         &dustEffParam);
                    }
                }
                if(work->npcNo != -1) {
                    for(i=0; i<2; i++) {
                        if(npcSeTimeTbl[(npcNo*2)+i] == (time*2)) {
                            HuAudFXPlay(npcSeTbl[npcNo]);
                            break;
                        }
                    }
                }
                break;
            
            case 1:
                if((time & (4 - 1)) == 0) {
                    if(!(dustFlags[npcNo] & CHAR_ATTR_FX_OFF)) {
                        dustEffParam.vel.x = -sin(3.141592653589793*modelP->rot.y/180.0)*4;
                        dustEffParam.vel.y = 2+(0.1*frandmod(10));
                        dustEffParam.vel.z = -cos(3.141592653589793*modelP->rot.y/180.0)*4;
                        pos.x = modelP->pos.x+(frandmod(50)-25);
                        pos.y = modelP->pos.y;
                        pos.z = modelP->pos.z+(frandmod(50)-25);
                        EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10) + 30,
                                         &dustEffParam);
                    }
                }
                if(work->npcNo != -1) {
                    for(i=0; i<2; i++) {
                        if(npcSeTimeTbl[(npcNo*2)+i] == (time*2)) {
                            HuAudFXPlay(npcSeTbl2[npcNo]);
                            break;
                        }
                    }
                }
                break;
            
            case 2:
                if(npcNo == -1) {
                    continue;
                }
                if((time & (32 - 1)) == 0) {
                    if(!(dustFlags[npcNo] & CHAR_ATTR_FX_OFF)) {
                        dustEffParam.vel.x = -sin(3.141592653589793*modelP->rot.y/180.0)*2;
                        dustEffParam.vel.y = 1+(0.1*frandmod(10));
                        dustEffParam.vel.z = -cos(3.141592653589793*modelP->rot.y/180.0)*2;
                        pos.x = modelP->pos.x+(frandmod(50)-25);
                        pos.y = modelP->pos.y;
                        pos.z = modelP->pos.z+(frandmod(50)-25);
                        EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10) + 30,
                                         &dustEffParam);
                    }
                }
                for(i=0; i<2; i++) {
                    if(npcSeTimeTbl2[(npcNo*2)+i] == (time*2)) {
                        HuAudFXPlay(npcSeTbl3[npcNo]);
                        break;
                    }
                }
                break;
             
            case 3:
                if(npcNo == -1) {
                    continue;
                }
                if((time & (4 - 1)) == 0) {
                    if(!(dustFlags[npcNo] & CHAR_ATTR_FX_OFF)) {
                        dustEffParam.vel.x = -sin(3.141592653589793*modelP->rot.y/180.0)*2;
                        dustEffParam.vel.y = 1+(0.1*frandmod(10));
                        dustEffParam.vel.z = -cos(3.141592653589793*modelP->rot.y/180.0)*2;
                        pos.x = modelP->pos.x+(frandmod(50)-25);
                        pos.y = modelP->pos.y;
                        pos.z = modelP->pos.z+(frandmod(50)-25);
                        EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10) + 30,
                                         &dustEffParam);
                    }
                }
                for(i=0; i<2; i++) {
                    if(npcSeTimeTbl3[(npcNo*2)+i] == (time*2)) {
                        HuAudFXPlay(npcSeTbl4[npcNo]);
                        break;
                    }
                }
                break;
             
            case 4:
                if(npcNo == -1) {
                    continue;
                }
                if((time & (8 - 1)) == 0) {
                    if(!(dustFlags[npcNo] & CHAR_ATTR_FX_OFF)) {
                        dustEffParam.vel.x = -sin(3.141592653589793*modelP->rot.y/180.0)*2;
                        dustEffParam.vel.y = 1+(0.1*frandmod(10));
                        dustEffParam.vel.z = -cos(3.141592653589793*modelP->rot.y/180.0)*2;
                        pos.x = modelP->pos.x+(frandmod(50)-25);
                        pos.y = modelP->pos.y;
                        pos.z = modelP->pos.z+(frandmod(50)-25);
                        EffectDustCreate(modelId, pos.x, pos.y, pos.z, frandmod(10) + 30,
                                         &dustEffParam);
                    }
                }
                for(i=0; i<2; i++) {
                    if(npcSeTimeTbl4[(npcNo*2)+i] == (time*2)) {
                        HuAudFXPlay(npcSeTbl5[npcNo]);
                        break;
                    }
                }
                break;
             
            case 5:
                if(time != 0) {
                    continue;
                }
                if(!(dustFlags[npcNo] & CHAR_ATTR_FX_OFF)) {
                    for(i=0; i<8; i++){ 
                        npcHitEffParam.vel.x =
                            10*sin(3.141592653589793*(45.0f*i)/180.0)*modelP->scale.x;
                        npcHitEffParam.vel.y = 0;
                        npcHitEffParam.vel.z =
                            10*cos(3.141592653589793*(45.0f*i)/180.0)*modelP->scale.x;
                        EffectStarCreate(modelId, modelP->pos.x,
                                         modelP->pos.y + (10 * modelP->scale.x), modelP->pos.z, 40,
                                         &npcHitEffParam);
                    }
                    for(i=0; i<8; i++){ 
                        dustEffParam.vel.x =
                            4*sin(3.141592653589793*((45.0f*i)+22.5)/180.0)*modelP->scale.x;
                        dustEffParam.vel.y = 0;
                        dustEffParam.vel.z =
                            4*cos(3.141592653589793*((45.0f*i)+22.5)/180.0)*modelP->scale.x;
                        EffectDustCreate(modelId, modelP->pos.x,
                                         modelP->pos.y + (10 * modelP->scale.x), modelP->pos.z, 20,
                                         &dustEffParam);
                    }
                    if(npcNo != -1) {
                        HuAudFXPlay(195);
                    }
                }
                break;
        }
    }
}

void fn_8005F77C(s16 charNo, s16 stepFx)
{
    CHARWORK *workP = &CharWork[charNo];
    workP->stepFx = stepFx;
}

// Selects the character's footstep sound and spatial playback when UpdateCharAnim reaches a step;
// its configured-volume/pan path returns an unassigned s16.
static s16 PlayStepVoice(s16 charNo, s16 seId, u8 voiceFlag)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
    s16 ret;
    if(voiceFlag & CHAR_VOICE_OFF) {
        return;
    }
    if(workP->stepFx == 4) {
        seId = (CHAR_STEP_GLOBAL_SE | CHAR_STEP_TYPE4_SE);
    } else if(workP->stepFx == 5) {
        seId = (CHAR_STEP_GLOBAL_SE | CHAR_STEP_TYPE5_SE);
    } else if(workP->stepFx == 6) {
        seId = 202;
    } else {
        seId += workP->stepFx;
    }
    if(!(seId & CHAR_STEP_GLOBAL_SE)) {
        if(workP->attr & CHAR_ATTR_VOICE_PAN_AUTO) {
            return CharFXPlayPos(charNo, seId, &modelP->pos);
        } else {
            fn_800591C8(charNo, seId, voiceFlag);
            return ret;
        }
    } else {
        seId &= (CHAR_STEP_GLOBAL_SE - 1);
        if(voiceFlag & CHAR_VOICE_OFF) {
            return -1;
        }
        if(workP->attr & CHAR_ATTR_VOICE_PAN_AUTO) {
            return HuAudFXEmiterPlay(seId, &modelP->pos);
        } else {
            return HuAudFXPlay(seId);
        }
    }
}

static EFFECTPARAM landEffParam = {
    0,
    { 255, 255, 255, 255 },
    { 255, 255, 255, 255 },
    { 0, 2, 1 },
    { 0.85f, 0.85f, 0.85f },
    0.2f,
    0,
    1,
    -8,
    0.02f
};

static EFFECTPARAM hipDropLandEffParam = {
    0,
    { 255, 255, 255, 255 },
    { 255, 255, 255, 255 },
    { 0, 2, 1 },
    { 0.9f, 0.9f, 0.9f },
    0.2f,
    0,
    1,
    -8,
    0.02f
};

static EFFECTPARAM hipDropStarEffParam = {
    0,
    { 255, 255, 0, 255 },
    { 255, 255, 0, 255 },
    { 0, 0, 0 },
    { 0.9f, 0.9f, 0.9f },
    -0.3f,
    0,
    -0.5f,
    -10,
    0
};

// Spreads a ring of landing dust around the character when UpdateCharAnim detects an impact.
void fn_8005F968(s16 charNo, HuVecF *pos)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
    s16 i;
    for(i=0; i<12; i++) {
        float speed = (6.0+frandmod(6))*modelP->scale.x;
        Vec effectPos;
        landEffParam.vel.x = speed*sin(3.141592653589793*(30.0f*i)/180.0);
        landEffParam.vel.y = 0;
        landEffParam.vel.z = speed*cos(3.141592653589793*(30.0f*i)/180.0);
        landEffParam.alphaBase = -(frandmod(4)+8);
        speed = (20.0+frandmod(20))*modelP->scale.x;
        effectPos.x = pos->x+(20*sin(3.141592653589793*(30.0f*i)/180.0)*modelP->scale.x);
        effectPos.y = pos->y+(10*modelP->scale.x);
        effectPos.z = pos->z+(20*cos(3.141592653589793*(30.0f*i)/180.0)*modelP->scale.x);
        landEffParam.gravity = frandmod(20)*0.01;
        EffectCreate(EFFECT_LANDDUST, modelP->cameraBit, effectPos.x, effectPos.y, effectPos.z,
                     speed, &landEffParam);
    }
}

// Creates landing dust and a footstep sound for step-triggered animation events.
void fn_8005FC84(s16 charNo, HuVecF *pos)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
    s16 i;
    for(i=0; i<12; i++) {
        float speed = modelP->scale.x*(6.0+frandmod(6));
        Vec effectPos;
        landEffParam.vel.x = speed*sin(3.141592653589793*(30.0f*i)/180.0);
        landEffParam.vel.y = 0;
        landEffParam.vel.z = speed*cos(3.141592653589793*(30.0f*i)/180.0);
        landEffParam.alphaBase = -(frandmod(4)+8);
        speed = modelP->scale.x*(20.0+frandmod(20));
        effectPos.x = pos->x+(modelP->scale.x*(20.0*sin(3.141592653589793*(30.0f*i)/180.0)));
        effectPos.y = pos->y+(10*modelP->scale.x);
        effectPos.z = pos->z+(modelP->scale.x*(20.0*cos(3.141592653589793*(30.0f*i)/180.0)));
        landEffParam.gravity = 0.01*frandmod(20);
        EffectCreate(EFFECT_LANDDUST, modelP->cameraBit, effectPos.x, effectPos.y, effectPos.z,
                     speed, &landEffParam);
    }
    PlayStepVoice(charNo, 185, 0);
}

// Creates a dust ring and outward stars when UpdateCharAnim detects a hip-drop landing.
void CharEffectHipDropCreate(s16 charNo, HuVecF *pos)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
    s16 i;
    for(i=0; i<12; i++) {
        float speed = (6.0+frandmod(6))*modelP->scale.x;
        Vec effectPos;
        hipDropLandEffParam.vel.x = speed*sin(3.141592653589793*(30.0f*i)/180.0);
        hipDropLandEffParam.vel.y = 0;
        hipDropLandEffParam.vel.z = speed*cos(3.141592653589793*(30.0f*i)/180.0);
        hipDropLandEffParam.alphaBase = -(frandmod(4)+8);
        speed = (30.0+frandmod(20))*modelP->scale.x;
        effectPos.x = pos->x+(20*sin(3.141592653589793*(30.0f*i)/180.0)*modelP->scale.x);
        effectPos.y = pos->y+(15*modelP->scale.x);
        effectPos.z = pos->z+(20*cos(3.141592653589793*(30.0f*i)/180.0)*modelP->scale.x);
        hipDropLandEffParam.gravity = frandmod(20)*0.01;
        EffectCreate(EFFECT_LANDDUST, modelP->cameraBit, effectPos.x, effectPos.y, effectPos.z,
                     speed, &hipDropLandEffParam);
    }
    for(i=0; i<8; i++) {
        hipDropStarEffParam.vel.x = (20*sin(3.141592653589793*(45.0f*i)/180.0)*modelP->scale.x);
        hipDropStarEffParam.vel.y = 5;
        hipDropStarEffParam.vel.z = (20*cos(3.141592653589793*(45.0f*i)/180.0)*modelP->scale.x);
        EffectStarCreate(workP->modelId, modelP->pos.x, modelP->pos.y + (10 * modelP->scale.x),
                         modelP->pos.z, 40, &hipDropStarEffParam);
    }
}

// Emits two rings of dust particles at the supplied position and scale.
void fn_8006065C(s16 cameraBit, float scale, HuVecF *pos)
{
    s16 i;
    for(i=0; i<8; i++) {
        dustEffParam.vel.x = scale*(4*sin(3.141592653589793*(i*45.0f)/180.0));
        dustEffParam.vel.y = 0;
        dustEffParam.vel.z = scale*(4*cos(3.141592653589793*(i*45.0f)/180.0));
        EffectCreate(EFFECT_DUST, cameraBit, pos->x, pos->y*scale, pos->z, 20*scale, &dustEffParam);
    }
    for(i=0; i<8; i++) {
        dustEffParam.vel.x = scale*(2*sin(3.141592653589793*((i*45.0f)+22.5)/180.0));
        dustEffParam.vel.y = 0;
        dustEffParam.vel.z = scale*(2*cos(3.141592653589793*((i*45.0f)+22.5)/180.0));
        EffectCreate(EFFECT_DUST, cameraBit, pos->x, pos->y*scale, pos->z, 20*scale, &dustEffParam);
    }
}

static EFFECTPARAM cryEffParam = {
    0,
    { 128, 255, 255, 255 },
    { 255, 255, 255, 255 },
    { 0, 2, 1 },
    { 0.85f, 0.85f, 0.85f },
    -0.5f,
    0,
    -0.1f,
    -16,
    0.02f
};

// Emits a drifting cry particle around a character's head during a cry animation.
void fn_800608C4(s16 cameraBit, HuVecF *pos, HuVecF *offset, float scale)
{
    float randZ;
    float randX;
    Mtx rotMtx;
    Vec effectPos;
    HuVecF dir;
    randX = frandmod(180)-90;
    randZ = frandmod(180)-90;
    VECNormalize(offset, &dir);
    fn_80022E38(rotMtx, frandmod(90)-45, frandmod(90)-45, 0);
    MTXMultVec(rotMtx, &dir, &dir);
    VECNormalize(&dir, &dir);
    cryEffParam.vel.x = 15*dir.x;
    cryEffParam.vel.y = 15*dir.y;
    cryEffParam.vel.z = 15*dir.z;
    effectPos.x = pos->x+(scale*(frandmod(30)-15));
    effectPos.y = pos->y+(scale*(frandmod(20)-10));
    effectPos.z = pos->z+(scale*(frandmod(30)-15));
    EffectCreate(EFFECT_CRY, cameraBit, effectPos.x, effectPos.y, effectPos.z, scale * 8,
                 &cryEffParam);
}

// Gets the character's head position and emits a cry particle during UpdateCharAnim.
void fn_80060B10(s16 charNo, float yOfs, float ofsY)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
    Vec pos;
    Vec offset;
    Hu3DModelObjPosGet(workP->modelId, CharHeadObjNameTbl[charNo], &pos);
    if(charNo == 4) {
        pos.y = pos.y-(40.0f*modelP->scale.x);
    }
    pos.y += yOfs;
    offset.x = sin(3.141592653589793*(modelP->rot.y+180.0f)/180.0);
    offset.y = ofsY;
    offset.z = cos(3.141592653589793*(modelP->rot.y+180.0f)/180.0);
    fn_800608C4(modelP->cameraBit, &pos, &offset, modelP->scale.x);
}

// Returns the selected character model archive number for the requested model variant.
unsigned int fn_80060E50(s16 charNo, s16 model)
{
    if(model & (1 << 0)) {
        return lbl_8022E718[charNo][0];
    } else if(model & (1 << 1)) {
        return lbl_8022E718[charNo][1];
    } else if(model & (1 << 2)) {
        return lbl_8022E718[charNo][2];
    } else {
        return lbl_8022E718[charNo][3];
    }
}

unsigned int fn_80060EF0(s16 charNo, u16 motId)
{
    return lbl_8022E718[charNo][4]|motId;
}

void fn_80060F14(s16 charNo, s16 pan)
{
    CHARWORK *workP = &CharWork[charNo];
    workP->pan = pan;
}

void fn_80060F40(s16 charNo, s16 vol)
{
    CHARWORK *workP = &CharWork[charNo];
    workP->vol = vol;
}

typedef struct WinLoseVoicePlay_s {
    s16 charNo; // Character whose result voice will play.
    s16 seId; // Sound effect ID to play after the animation delay.
    unsigned int motId; // Motion ID whose animation length determines the delay.
} WINLOSEVOICEPLAY;

static void PlayWinLoseVoice(void);

// Schedules a result voice after the character's selected win animation has played.
void CharWinLoseVoicePlay(s16 charNo, unsigned int motId, s16 seId)
{
    HUPROCESS *parent = fn_8000DD9C();
    HUPROCESS *process = fn_8000DAC0(PlayWinLoseVoice, 100, 6144, 0, parent);
    if(!process) {
        OSReport("Error: CharWinLoseVoicePlay Failure.\n");
        return;
    } else {
        WINLOSEVOICEPLAY *winLose =
            HuMemDirectMallocNum(HEAP_HEAP, sizeof(WINLOSEVOICEPLAY), 0x10000000);
        process->property = winLose;
        winLose->charNo = charNo;
        winLose->seId = seId;
        winLose->motId = motId;
    }
}

// Waits for the selected result animation, then plays its voice and exits the child task.
static void PlayWinLoseVoice(void)
{
    HUPROCESS *process = fn_8000DD9C();
    WINLOSEVOICEPLAY *winLose = process->property;
    CHARWORK *workP = &CharWork[winLose->charNo];
    s16 i;
    s16 motNo;
    
    if(workP->modelId == -1) {
        HuMemDirectFree(winLose);
        HuPrcEnd();
    }
    for(i=0; i<32; i++) {
        if(workP->motId[i] == winLose->motId) {
            motNo = workP->motNoTbl[i];
            break;
        }
    }
    switch(motNo) {
        case FILENUM(CHARMOT_HSF_c000m1_306):
            if(omcurovl >= 123 && omcurovl <= 123) {
                HuPrcSleep(boardWinAnimLen[winLose->charNo]);
            } else {
                HuPrcSleep(winAnimLen[winLose->charNo]);
            }
            break;
       
       case FILENUM(CHARMOT_HSF_c000m1_346):
            HuPrcSleep(handUpAnimLen[winLose->charNo]);
            break;
       
       case FILENUM(CHARMOT_HSF_c000m1_349):
            HuPrcSleep(okAnimLen[winLose->charNo]);
            break;
       
       case FILENUM(CHARMOT_HSF_c000m1_350):
            HuPrcSleep(winAnimLen2[winLose->charNo]);
            break;
    }
    fn_800591C8(winLose->charNo, winLose->seId, 0);
    HuMemDirectFree(process->property);
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}

// Plays the losing voice for each supplied character after the result sequence calls this helper.
void fn_800611E4(s16 charNo1, s16 charNo2, s16 charNo3, s16 charNo4)
{
    if(charNo1 != -1) {
        fn_800591C8(charNo1, 585, 0);
    }
    if(charNo2 != -1) {
        fn_800591C8(charNo2, 585, 0);
    }
    if(charNo3 != -1) {
        fn_800591C8(charNo3, 585, 0);
    }
    if(charNo4 != -1) {
        fn_800591C8(charNo4, 585, 0);
    }
}

// Returns the known result-animation duration for a character, or zero for other motions.
s16 fn_800613C0(s16 charNo, int motNo)
{
    switch(motNo) {
        case FILENUM(CHARMOT_HSF_c000m1_306):
            return winAnimLen[charNo];
        
        case FILENUM(CHARMOT_HSF_c000m1_346):
            return handUpAnimLen[charNo];
        
        case FILENUM(CHARMOT_HSF_c000m1_349):
            return okAnimLen[charNo];
        
        case FILENUM(CHARMOT_HSF_c000m1_350):
            return winAnimLen2[charNo];
        
        case FILENUM(CHARMOT_HSF_c000m1_363):
            return holdUpAnimLen[charNo];
        
        case FILENUM(CHARMOT_HSF_c000m1_364):
            return winJumpUpAnimLen[charNo];
        
        case FILENUM(CHARMOT_HSF_c000m1_352):
            return loseAnimLen[charNo];
        
        default:
            return 0;
    }
}

s16 fn_800614A4(s16 charNo, int motNo)
{
    return 0;
}

// Creates one shared light and attaches its ID to each loaded character model that has a free
// local-light slot.
HU3D_LIGHTID fn_800614AC(HuVecF *pos, HuVecF *dir, GXColor *color)
{
    CHARWORK *workP = CharWork;
    s16 i;
    s16 j;
    HU3D_LIGHTID lightId = -1;
    for(i=0; i<14; i++, workP++) {
        if(workP->modelId == -1) {
            continue;
        }
        if(lightId == -1) {
            lightId = fn_800288B4(workP->modelId, pos, dir, color);
            if(lightId == -1) {
                return -1;
            }
        } else {
            HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
            for(j=0; j<8; j++) {
                if(modelP->LLightId[j] == -1) {
                    modelP->LLightId[j] = lightId;
                    modelP->attr |= (1 << 12);
                    break;
                }
            }
        }
    }
    return lightId;
}

inline HU3D_LIGHTID fn_800614AC(HuVecF *pos, HuVecF *dir, GXColor *color);

// Builds a character light from position, direction, and RGB values, then attaches it to loaded
// character models.
HU3D_LIGHTID fn_800615B4(float posX, float posY, float posZ, float dirX, float dirY, float dirZ,
                             u8 r, u8 g, u8 b)
{
    HuVecF pos;
    HuVecF dir;
    GXColor color;
    pos.x = posX; pos.y = posY; pos.z = posZ;
    dir.x = dirX; dir.y = dirY; dir.z = dirZ;
    color.r = r; color.g = g; color.b = b; color.a = 255;
    return fn_800614AC(&pos, &dir, &color);
}

// Finds the first local light attached to a loaded character model.
static inline HU3D_LIGHT *GetCharLight(void)
{
    CHARWORK *workP = CharWork;
    s16 i;
    s16 j;
    for(i=0; i<14; i++, workP++) {
        if(workP->modelId == -1) {
            continue;
        }
        {
            HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
            for(j=0; j<8; j++) {
                if(modelP->LLightId[j] != -1) {
                    return &lbl_80261674[modelP->LLightId[j]];
                }
            }
        }
    }
    return NULL;
}

// Configures the shared character light as a spot light with the requested cutoff.
void fn_80061728(s32 func, float cutoff)
{
    HU3D_LIGHT *lightP = GetCharLight();
    if(lightP) {
        lightP->type &= CHAR_LIGHT_FLAGS_MASK;
        lightP->cutoff = cutoff;
        lightP->func = func;
    }
}

// Changes the shared character light to directional (infinite-distance) mode.
void fn_80061814(void)
{
    HU3D_LIGHT *lightP = GetCharLight();
    if(lightP) {
        lightP->type &= CHAR_LIGHT_FLAGS_MASK;
        lightP->type |= 1;
    }
}

// Configures the shared character light as a point light with its attenuation settings.
void fn_80061904(s32 func, float cutoff, float brightness)
{
    HU3D_LIGHT *lightP = GetCharLight();
    if(lightP) {
        lightP->type &= CHAR_LIGHT_FLAGS_MASK;
        lightP->type |= 2;
        lightP->cutoff = cutoff;
        lightP->brightness = brightness;
        lightP->func = func;
    }
}

// Sets the color and alpha used by the shared character light.
void fn_80061A04(u8 r, u8 g, u8 b, u8 a)
{
    HU3D_LIGHT *lightP = GetCharLight();
    if(lightP) {
        lightP->color.r = r;
        lightP->color.g = g;
        lightP->color.b = b;
        lightP->color.a = a;
    }
}

// Sets the shared light's position and normalizes its aim vector into the light direction.
void fn_80061AE4(HuVecF *pos, HuVecF *aim)
{
    HU3D_LIGHT *lightP = GetCharLight();
    if(lightP) {
        lightP->pos = *pos;
        PSVECNormalize(aim, &lightP->dir);
    }
}

// Sets the shared light position and normalizes the supplied aim direction.
void fn_80061BE0(float posX, float posY, float posZ, float aimX, float aimY, float aimZ)
{
    HU3D_LIGHT *lightP = GetCharLight();
    if(lightP) {
        lightP->pos.x = posX;
        lightP->pos.y = posY;
        lightP->pos.z = posZ;
        lightP->dir.x = aimX;
        lightP->dir.y = aimY;
        lightP->dir.z = aimZ;
        PSVECNormalize(&lightP->dir, &lightP->dir);
    }
}

// Enables or clears the static-light flag on the shared character light.
void fn_80061D04(BOOL staticF)
{
    HU3D_LIGHT *lightP = GetCharLight();
    if(lightP) {
        if(staticF) {
            lightP->type |= (1 << 15);
        } else {
            lightP->type &= ~(1 << 15);
        }
    }
}

// Places the warning marker above a character when a caller requests the danger effect.
void fn_80061E00(s16 charNo, float scale)
{
    CHARWORK *workP = &CharWork[charNo];
    HU3D_MODEL *modelP = &lbl_802C01C8[workP->modelId];
    EffectWarnCreate(workP->modelId, modelP->pos.x, 100.0f + modelP->pos.y, modelP->pos.z, 20,
                     scale, &warnEffParam);
}

u32 fn_80062094(s16 charNo)
{
    CHARWORK *workP = &CharWork[charNo];
    return workP->attr;
}
