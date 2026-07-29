#include "dolphin.h"
#include "game/armem.h"
#include "game/board/audio.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"

typedef void (*VoidFunc)(void);

typedef struct EndingWindowPlayers {
    s16 player[4];
} EndingWindowPlayers;

typedef struct EndingSpritePositions {
    Vec position[10];
} EndingSpritePositions;

typedef struct EndingModelObjectNames {
    char *name[10];
} EndingModelObjectNames;

typedef struct EndingAudioState {
    s32 channel[32];
} EndingAudioState;

typedef struct EndingLightVectors {
    HuVecF vector[2];
} EndingLightVectors;

typedef struct EndingParticleCounts {
    s16 count[5];
} EndingParticleCounts;

typedef struct EndingScenePositions {
    HuVecF position[3];
} EndingScenePositions;

typedef struct EndingMotionWork {
    s16 state;
    float time;
    float duration;
    HuVecF unk_0C;
    HuVecF unk_18;
    HuVecF unk_24;
    float start;
    float end;
    float unk_38;
    float unk_3C;
} EndingMotionWork;

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern s16 lbl_1_bss_26;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_14;
extern OMOBJ *lbl_1_bss_18;
extern OMOBJ *lbl_1_bss_1C;
extern OMOBJ *lbl_1_bss_20;
extern s16 lbl_1_bss_24;
extern float lbl_1_bss_28;
extern s16 lbl_1_bss_2C;
extern EndingMotionWork lbl_1_bss_34[96];
extern EndingMotionWork lbl_1_bss_1834[7];
extern s16 lbl_1_bss_1A08[10];
extern s16 lbl_1_bss_1A1C[2];
extern ANIMDATA *lbl_1_bss_1A20[10];
extern s16 lbl_1_bss_19F4[10];
extern s16 lbl_1_bss_1A48[5];
extern s16 lbl_1_bss_1A52[2];
extern s16 lbl_1_bss_1A56[2];
extern EndingMotionWork lbl_1_bss_1A5C[2];
extern EndingMotionWork lbl_1_bss_1ADC[10];
extern EndingAudioState lbl_1_bss_1D5C;
extern s32 lbl_1_bss_1DDC;
extern s16 lbl_1_bss_1E1C;
extern s16 lbl_1_bss_1E1E[2];
extern s16 lbl_1_bss_1E22[2];
extern s16 lbl_1_bss_1E26[2];
extern s16 lbl_1_bss_1E2A;
extern s16 lbl_1_bss_1E2C;
extern s16 lbl_1_bss_1DE0[6][5];
extern ANIMDATA *lbl_1_bss_1E30[3];

extern float lbl_1_rodata_2F8;
extern float lbl_1_rodata_318;
extern float lbl_1_rodata_340;
extern float lbl_1_rodata_350;
extern float lbl_1_rodata_78;
extern float lbl_1_rodata_7C;
extern float lbl_1_rodata_118;
extern EndingSpritePositions lbl_1_rodata_11C;
extern EndingLightVectors lbl_1_rodata_88;
extern EndingLightVectors lbl_1_rodata_A0;
extern GXColor lbl_1_rodata_B8;
extern float lbl_1_rodata_BC;
extern float lbl_1_rodata_C0;
extern float lbl_1_rodata_C4;
extern float lbl_1_rodata_C8;
extern float lbl_1_rodata_CC;
extern float lbl_1_rodata_D0;
extern float lbl_1_rodata_110;
extern HuVecF lbl_1_rodata_D4;
extern HuVecF lbl_1_rodata_E0;
extern HuVecF lbl_1_rodata_EC;
extern float lbl_1_rodata_F8;
extern float lbl_1_rodata_FC;
extern EndingWindowPlayers lbl_1_rodata_100;
extern float lbl_1_rodata_108;
extern float lbl_1_rodata_10C;
extern float lbl_1_rodata_19C;
extern float lbl_1_rodata_1A0;
extern EndingModelObjectNames lbl_1_rodata_1B4;
extern float lbl_1_rodata_1B0;
extern float lbl_1_rodata_200;
extern float lbl_1_rodata_204;
extern float lbl_1_rodata_208;
extern float lbl_1_rodata_20C;
extern float lbl_1_rodata_228;
extern float lbl_1_rodata_230;
extern float lbl_1_rodata_1EC;
extern float lbl_1_rodata_210;
extern float lbl_1_rodata_194;
extern float lbl_1_rodata_198;
extern float lbl_1_rodata_1E0;
extern float lbl_1_rodata_1E4;
extern float lbl_1_rodata_1E8;
extern float lbl_1_rodata_114;
extern float lbl_1_rodata_214;
extern float lbl_1_rodata_218;
extern float lbl_1_rodata_21C;
extern float lbl_1_rodata_220;
extern float lbl_1_rodata_224;
extern float lbl_1_rodata_22C;
extern float lbl_1_rodata_244;
extern float lbl_1_rodata_274;
extern float lbl_1_rodata_2C8;
extern HuVecF lbl_1_rodata_294;
extern HuVecF lbl_1_rodata_2A0;
extern HuVecF lbl_1_rodata_2B8;
extern EndingScenePositions lbl_1_rodata_2CC;
extern float lbl_1_rodata_2F0;
extern float lbl_1_rodata_280;
extern float lbl_1_rodata_284;
extern HuVecF lbl_1_rodata_288;
extern EndingParticleCounts lbl_1_rodata_390;

extern s16 lbl_1_data_10E;
extern s16 lbl_1_data_38;
extern u32 lbl_1_data_0;
extern u32 lbl_1_data_128[3];
extern char lbl_1_data_25[];
extern char lbl_1_data_DD[];
extern char lbl_1_data_CB[];
extern char lbl_1_data_110[];

int rand8(void);
void fn_1_98(HUWINID window, u32 message, char character);
void fn_1_25C(OMOBJ *object);
void fn_1_1C0C(s16 index, s16 motion, float time);
void fn_1_10DEC(s16 index, HuVecF *pos, s16 mode);
void fn_1_DD14(void);
void fn_1_F1B8(s16 display, HuVecF *pos);
void fn_1_12838(void);
void fn_1_E1EC(s16 display, HuVecF *pos);
void fn_1_EAB8(s16 display, HuVecF *pos);
void fn_1_F068(s16 index, s16 display, HuVecF *pos);
void fn_1_F11C(s16 index, s16 count);
void fn_1_F23C(s16 count);
void fn_1_111B0(s16 display);
void fn_1_E0EC(s16 groupId, u32 attr);
void fn_1_EB54(s16 time);
void fn_1_1160(OMOBJ *object);
void fn_1_36E0(OMOBJ *object);
void fn_1_2AEC(OMOBJ *object);
void fn_1_E270(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_EBCC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_F2CC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_FDD4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_10628(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_112E0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_11714(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_702C(void);
void fn_1_8F80(OMOBJ *object);
void fn_1_A964(void);
void fn_1_B568(void);
void fn_1_BE30(void);
void fn_1_C7DC(void);
void fn_1_D434(void);
void fn_1_45C0(OMOBJ *object);
void fn_1_4BC8(OMOBJ *object);
void fn_1_11F34(void);
float fn_1_DDF8(float start, float end, float time, float duration);

int fn_1_0(int seId)
{
    if (lbl_1_bss_26 == 0) {
        return HuAudFXPlay(seId);
    }
    return -1;
}

int fn_1_44(int seId, s16 volume, s16 pan)
{
    if (lbl_1_bss_26 == 0) {
        return HuAudFXPlayVolPan(seId, volume, pan);
    }
    return -1;
}

inline int fn_1_44(int seId, s16 volume, s16 pan);

void fn_1_558(OMOBJ *object)
{
    OMOBJ *object0 = lbl_1_bss_C;
    OMOBJ *object1 = lbl_1_bss_10;
    EndingMotionWork *work0 = &lbl_1_bss_1A5C[0];
    EndingMotionWork *work1 = &lbl_1_bss_1A5C[1];

    Hu3DModelObjPosGet(object0->mdlId[0], lbl_1_data_25,
        &work0->unk_0C);
    Hu3DModelObjPosGet(object1->mdlId[0], lbl_1_data_25,
        &work1->unk_0C);
    fn_1_10DEC(0, &work0->unk_0C, 1);
    fn_1_10DEC(1, &work1->unk_0C, 1);
    if (++object->work[2] > 60) {
        fn_1_0(1407);
        fn_1_0(1406);
        object->work[2] = 0;
        object->objFunc = fn_1_25C;
    }
}

void fn_1_664(void)
{
    OMOBJ *object = lbl_1_bss_10;
    OMOBJ *object0 = lbl_1_bss_C;
    OMOBJ *object1 = lbl_1_bss_10;
    EndingMotionWork *work0 = &lbl_1_bss_1A5C[0];
    EndingMotionWork *work1 = &lbl_1_bss_1A5C[1];

    Hu3DModelObjPosGet(object0->mdlId[0], lbl_1_data_25,
        &work0->unk_0C);
    Hu3DModelObjPosGet(object1->mdlId[0], lbl_1_data_25,
        &work1->unk_0C);
    object->work[0] = 0;
    object->work[1] = 180;
    object->work[2] = 0;
    object->objFunc = fn_1_558;
}

void fn_1_71C(void)
{
    fn_1_10DEC(0, 0, 2);
    fn_1_10DEC(1, 0, 2);
    lbl_1_bss_10->objFunc = NULL;
}

void fn_1_76C(void)
{
    Hu3DCameraCreate(1);
}

inline void fn_1_76C(void);

void fn_1_790(void)
{
    Hu3DCameraKill(1);
}

void fn_1_904(void)
{
    Hu3DLightAllKill();
}

void fn_1_924(void)
{
    lbl_1_bss_1A52[0] = Hu3DLLightCreate(lbl_1_bss_C->mdlId[0],
        lbl_1_rodata_BC, lbl_1_rodata_C0, lbl_1_rodata_C4,
        lbl_1_rodata_C8, lbl_1_rodata_C8, lbl_1_rodata_CC,
        128, 128, 128);
    Hu3DLLightInfinitytSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_1A52[0]);
    Hu3DLLightStaticSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_1A52[0], 1);

    lbl_1_bss_1A52[1] = Hu3DLLightCreate(lbl_1_bss_10->mdlId[0],
        lbl_1_rodata_D0, lbl_1_rodata_C0, lbl_1_rodata_C4,
        lbl_1_rodata_C8, lbl_1_rodata_C8, lbl_1_rodata_CC,
        128, 128, 128);
    Hu3DLLightInfinitytSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_1A52[1]);
    Hu3DLLightStaticSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_1A52[1], 1);
}

void fn_1_AC8(void)
{
    Hu3DLLightKill(lbl_1_bss_C->mdlId[0], lbl_1_bss_1A52[0]);
    Hu3DLLightKill(lbl_1_bss_10->mdlId[0], lbl_1_bss_1A52[1]);
}

void fn_1_B2C(void)
{
    HuVecF position = lbl_1_rodata_D4;
    HuVecF up = lbl_1_rodata_E0;
    HuVecF target = lbl_1_rodata_EC;

    Hu3DShadowCreate(lbl_1_rodata_F8, lbl_1_rodata_78, lbl_1_rodata_FC);
    Hu3DShadowPosSet(&position, &up, &target);
}

inline void fn_1_B2C(void);

void fn_1_BE0(void)
{
    if (lbl_1_data_38 != -1) {
        HuWinExClose(lbl_1_bss_1A48[lbl_1_data_38]);
        lbl_1_data_38 = -1;
    }
}

void fn_1_C44(s16 window, u32 message)
{
    if (lbl_1_data_38 != -1 && lbl_1_data_38 != window) {
        fn_1_BE0();
    }
    if (lbl_1_data_38 == -1 || lbl_1_data_38 != window) {
        HuWinExOpen(lbl_1_bss_1A48[window]);
        lbl_1_data_38 = window;
    }
    HuWinAttrSet(lbl_1_bss_1A48[window], 2048);
    HuWinMesSet(lbl_1_bss_1A48[window], message);
    if (lbl_1_data_0 != message) {
        lbl_1_data_0 = -1;
    }
}

void fn_1_DAC(s16 window, u32 message, s16 delay)
{
    if (lbl_1_data_38 != -1 && lbl_1_data_38 != window) {
        fn_1_BE0();
    }
    if (lbl_1_data_38 == -1 || lbl_1_data_38 != window) {
        HuWinExOpen(lbl_1_bss_1A48[window]);
        lbl_1_data_38 = window;
    }
    HuWinAttrSet(lbl_1_bss_1A48[window], 2048);
    HuWinMesSet(lbl_1_bss_1A48[window], message);
    if (lbl_1_data_0 != message) {
        lbl_1_data_0 = -1;
    }
    if (delay > 0) {
        HuPrcSleep(delay);
    }
}

void fn_1_F34(void)
{
    EndingWindowPlayers players = lbl_1_rodata_100;
    s16 window;

    HuWinInit(1);
    for (window = 0; window < 4; window++) {
        lbl_1_bss_1A48[window] = HuWinExCreateFrame(
            lbl_1_rodata_108, lbl_1_rodata_10C, 544, 68, -1,
            players.player[window]);
        HuWinDispOff(lbl_1_bss_1A48[window]);
        HuWinBGTPLvlSet(lbl_1_bss_1A48[window], lbl_1_rodata_110);
        winData[lbl_1_bss_1A48[window]].padMask = 1;
    }
    for (window = 0; window < 4; window++) {
        HuWinCallbackSet(lbl_1_bss_1A48[window], fn_1_98);
    }
    lbl_1_bss_1A48[4] = HuWinExCreateFrame(lbl_1_rodata_108,
        lbl_1_rodata_114, 544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_1A48[4]);
    HuWinBGTPLvlSet(lbl_1_bss_1A48[4], lbl_1_rodata_C8);
}

inline void fn_1_F34(void);

void fn_1_1104(void)
{
    s16 window;

    for (window = 0; window < 4; window++) {
        HuWinExKill(lbl_1_bss_1A48[window]);
    }
    HuWinAllKill();
}

void fn_1_1160(OMOBJ *object)
{
    switch (object->work[0]) {
        case 0:
            lbl_1_bss_28 += lbl_1_rodata_118;
            HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_bss_28,
                lbl_1_rodata_C8);
            break;
        case 1:
            lbl_1_bss_28 -= lbl_1_rodata_118;
            HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_bss_28,
                lbl_1_rodata_C8);
            break;
        case 2:
            lbl_1_bss_28 -= lbl_1_rodata_118;
            HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
                lbl_1_bss_28);
            break;
    }
}

void fn_1_12A8(s16 state)
{
    if (state == 2) {
        HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
            lbl_1_rodata_C8);
    } else {
        HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
            lbl_1_rodata_C8);
    }
    fn_1_E0EC(lbl_1_bss_1A1C[0], HUSPR_ATTR_DISPOFF);
    lbl_1_bss_4->work[0] = state;
    lbl_1_bss_4->objFunc = fn_1_1160;

    switch (state) {
        case 0:
            lbl_1_bss_28 = lbl_1_rodata_C8;
            HuSprAttrReset(lbl_1_bss_1A1C[0], 4,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 6,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 3,
                HUSPR_ATTR_DISPOFF);
            break;
        case 1:
            lbl_1_bss_28 = lbl_1_rodata_C8;
            HuSprAttrReset(lbl_1_bss_1A1C[0], 1,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 5,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 8,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 9,
                HUSPR_ATTR_DISPOFF);
            break;
        case 2:
            lbl_1_bss_28 = lbl_1_rodata_C8;
            HuSprAttrReset(lbl_1_bss_1A1C[0], 0,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 2,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 7,
                HUSPR_ATTR_DISPOFF);
            break;
        case 3:
            lbl_1_bss_4->objFunc = NULL;
            break;
    }
}

void fn_1_14F4(void)
{
    EndingSpritePositions positions = lbl_1_rodata_11C;
    s16 index;

    for (index = 0; index < 10; index++) {
        lbl_1_bss_1A20[index] = HuSprAnimRead(HuDataSelHeapReadNum(
            2228244 + index, HU_MEMNUM_OVL, HEAP_MODEL));
    }
    lbl_1_bss_1A1C[0] = HuSprGrpCreate(10);
    for (index = 0; index < 10; index++) {
        lbl_1_bss_1A08[index] = HuSprCreate(lbl_1_bss_1A20[index],
            positions.position[index].z, 0);
        HuSprGrpMemberSet(lbl_1_bss_1A1C[0], index,
            lbl_1_bss_1A08[index]);
        HuSprPosSet(lbl_1_bss_1A1C[0], index,
            positions.position[index].x, positions.position[index].y);
    }
    HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
        lbl_1_rodata_C8);
    fn_1_E0EC(lbl_1_bss_1A1C[0], 4);
    HuSprExecLayerSet(64, 2);
    HuSprGrpDrawNoSet(lbl_1_bss_1A1C[0], 64);
}

inline void fn_1_14F4(void);

void fn_1_16D4(void)
{
    s16 sprite;
    ANIMDATA *animation;

    animation = HuSprAnimRead(HuDataSelHeapReadNum(
        2228243, HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_1A1C[1] = HuSprGrpCreate(1);
    sprite = HuSprCreate(animation, 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_1A1C[1], 0, sprite);
    HuSprGrpPosSet(lbl_1_bss_1A1C[1], lbl_1_rodata_194,
        lbl_1_rodata_198);
    fn_1_E0EC(lbl_1_bss_1A1C[1], 4);
}

void fn_1_189C(OMOBJ *object)
{
    omSetStatBit(object, 256);
    object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        2228224, HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[0] = Hu3DMotionIDGet(object->mdlId[0]);
    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 1073741825);
    object->objFunc = NULL;
}

void fn_1_1B20(void)
{
    Mtx matrix;
    EndingModelObjectNames names = lbl_1_rodata_1B4;
    s16 model;

    for (model = 0; model < 10; model++) {
        Hu3DModelObjMtxGet(lbl_1_bss_4->mdlId[0], names.name[model], matrix);
        Hu3DModelPosSet(lbl_1_bss_8->mdlId[model], matrix[0][3],
            matrix[1][3], matrix[2][3]);
    }
}

void fn_1_193C(OMOBJ *object)
{
    EndingMotionWork *work = lbl_1_bss_1ADC;
    s16 model;
    HuVecF rotation;

    for (model = 0; model < 10; model++, work++) {
        if (lbl_1_rodata_C8 == work->time) {
            Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
                lbl_1_bss_8->mtnId[3 + (model * 6)],
                lbl_1_rodata_C8, lbl_1_rodata_78, 1073741825);
        }
        Hu3DModelRotGet(object->mdlId[model], &rotation);
        rotation.y = fn_1_DDF8(work->start, work->end, work->time,
            work->duration);
        Hu3DModelRotSetV(object->mdlId[model], &rotation);
        if ((work->time += lbl_1_rodata_19C) > work->duration) {
            work->time = lbl_1_rodata_19C + work->duration;
            if (lbl_1_rodata_C8 == work->unk_38) {
                work->unk_38 = lbl_1_rodata_19C;
                Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
                    lbl_1_bss_8->mtnId[model * 6], lbl_1_rodata_C8,
                    lbl_1_rodata_1B0, 1073741825);
            }
        }
    }
}

void fn_1_1E3C(s16 motion, float time)
{
    s16 index;

    for (index = 0; index < 10; index++) {
        fn_1_1C0C(index, motion, time);
    }
}

void fn_1_1E90(s16 model, s16 motion, float blend, u32 attr)
{
    Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
        lbl_1_bss_8->mtnId[motion + (model * 6)], lbl_1_rodata_C8,
        blend, attr);
}

void fn_1_1F20(s16 motion, float blend, u32 attr)
{
    s16 model;

    for (model = 0; model < 10; model++) {
        Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
            lbl_1_bss_8->mtnId[motion + (model * 6)], lbl_1_rodata_C8,
            blend, attr);
    }
}

void fn_1_1FC4(void)
{
    s16 model;

    HuPrcSleep(2);
    for (model = 0; model < 10; model++) {
        Hu3DMotionSpeedSet(lbl_1_bss_8->mdlId[model], lbl_1_rodata_1E0);
    }
}

void fn_1_2034(s16 motion, float blend, u32 attr, s16 delay)
{
    s16 frame;
    s16 model;

    for (model = 0; model < 10; model++) {
        lbl_1_bss_19F4[model] = (rand8() % delay) + 1;
    }
    if (motion == 4) {
        fn_1_0(596);
    }
    for (frame = 0; frame < delay + 5; frame++) {
        HuPrcVSleep();
        for (model = 0; model < 10; model++) {
            if (lbl_1_bss_19F4[model] == 0) {
                Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
                    lbl_1_bss_8->mtnId[motion + (model * 6)],
                    lbl_1_rodata_C8, blend, attr);
            }
            lbl_1_bss_19F4[model]--;
            if (lbl_1_bss_19F4[model] <= -10) {
                lbl_1_bss_19F4[model] = -10;
            }
        }
    }
}

void fn_1_2208(s16 motion, float blend, u32 attr, s16 unused, s16 delay)
{
    s16 frame;
    s16 model;

    for (model = 0; model < 10; model++) {
        lbl_1_bss_19F4[model] = (rand8() % delay) + 1;
    }
    for (frame = 0; frame < delay + 5; frame++) {
        HuPrcVSleep();
        for (model = 0; model < 10; model++) {
            if (lbl_1_bss_19F4[model] == 0) {
                Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
                    lbl_1_bss_8->mtnId[motion + (model * 6)],
                    lbl_1_rodata_C8, blend, attr);
                Hu3DMotionSpeedSet(lbl_1_bss_8->mdlId[model],
                    lbl_1_rodata_1E0);
            }
            lbl_1_bss_19F4[model]--;
            if (lbl_1_bss_19F4[model] <= -10) {
                lbl_1_bss_19F4[model] = -10;
            }
        }
    }
}

void fn_1_23D8(OMOBJ *object)
{
    EndingMotionWork *work = lbl_1_bss_1ADC;
    s16 model;

    omSetStatBit(object, 256);
    for (model = 0; model < 10; model++, work++) {
        object->mdlId[model] = Hu3DModelCreate(HuDataSelHeapReadNum(
            2228277 + model, HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[model * 6] = Hu3DJointMotion(object->mdlId[model],
            HuDataSelHeapReadNum(2228298 + model, HU_MEMNUM_OVL,
                HEAP_MODEL));
        object->mtnId[(model * 6) + 1] = Hu3DJointMotion(
            object->mdlId[model], HuDataSelHeapReadNum(2228309 + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[(model * 6) + 2] = Hu3DJointMotion(
            object->mdlId[model], HuDataSelHeapReadNum(2228287 + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[(model * 6) + 3] = Hu3DJointMotion(
            object->mdlId[model], HuDataSelHeapReadNum(2228320 + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[(model * 6) + 4] = Hu3DJointMotion(
            object->mdlId[model], HuDataSelHeapReadNum(2228331 + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[(model * 6) + 5] = Hu3DJointMotion(
            object->mdlId[model], HuDataSelHeapReadNum(2228342 + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelScaleSet(object->mdlId[model], lbl_1_rodata_110,
            lbl_1_rodata_110, lbl_1_rodata_110);
        Hu3DMotionShiftSet(object->mdlId[model], object->mtnId[model * 6],
            lbl_1_rodata_C8, lbl_1_rodata_C8, 1073741825);
        work->time = lbl_1_rodata_1E4;
        work->duration = lbl_1_rodata_78;
        work->unk_38 = lbl_1_rodata_19C;
    }
    object->objFunc = fn_1_193C;
}

void fn_1_26D4(OMOBJ *object)
{
    s16 model;
    s16 motion;

    if (object) {
        for (model = 0; model < 10; model++) {
            for (motion = 0; motion < 6; motion++) {
                Hu3DMotionKill(object->mtnId[motion + (model * 6)]);
            }
            Hu3DModelKill(object->mdlId[model]);
        }
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

void fn_1_2790(HuVecF *dest, float x, float y, float z)
{
    dest->x = x;
    dest->y = y;
    dest->z = z;
}

float fn_1_27A0(float start, float middle, float end, float time)
{
    float inverse = lbl_1_rodata_19C - time;

    return end * (time * time)
        + (start * (inverse * inverse)
        + lbl_1_rodata_1E8 * (middle * (inverse * time)));
}

void fn_1_27FC(HuVecF *dest, HuVecF *start, HuVecF *middle,
    HuVecF *end, float time)
{
    dest->x = fn_1_27A0(start->x, middle->x, end->x, time);
    dest->y = fn_1_27A0(start->y, middle->y, end->y, time);
    dest->z = fn_1_27A0(start->z, middle->z, end->z, time);
}

void fn_1_31FC(void)
{
    OMOBJ *object = lbl_1_bss_C;
    EndingMotionWork *first = &lbl_1_bss_1A5C[0];
    EndingMotionWork *second = &lbl_1_bss_1A5C[1];

    first->state = 1;
    first->time = lbl_1_rodata_C8;
    first->duration = lbl_1_rodata_200;
    first->unk_0C.x = lbl_1_rodata_204;
    first->unk_0C.y = lbl_1_rodata_C0;
    first->unk_0C.z = lbl_1_rodata_208;
    first->unk_18.x = lbl_1_rodata_20C;
    first->unk_18.y = lbl_1_rodata_D0;
    first->unk_18.z = lbl_1_rodata_20C;
    first->unk_24.x = lbl_1_rodata_BC;
    first->unk_24.y = lbl_1_rodata_C0;
    first->unk_24.z = lbl_1_rodata_C8;

    second->state = 1;
    second->time = lbl_1_rodata_C8;
    second->duration = lbl_1_rodata_200;
    second->unk_0C.x = lbl_1_rodata_C4;
    second->unk_0C.y = lbl_1_rodata_C0;
    second->unk_0C.z = lbl_1_rodata_208;
    second->unk_18.x = lbl_1_rodata_210;
    second->unk_18.y = lbl_1_rodata_D0;
    second->unk_18.z = lbl_1_rodata_20C;
    second->unk_24.x = lbl_1_rodata_D0;
    second->unk_24.y = lbl_1_rodata_C0;
    second->unk_24.z = lbl_1_rodata_C8;

    Hu3DModelPosSet(lbl_1_bss_C->mdlId[0], lbl_1_rodata_204,
        lbl_1_rodata_C0, lbl_1_rodata_208);
    Hu3DModelPosSet(lbl_1_bss_10->mdlId[0], lbl_1_rodata_C4,
        lbl_1_rodata_C0, lbl_1_rodata_208);
    Hu3DModelRotSet(lbl_1_bss_C->mdlId[0], lbl_1_rodata_C8,
        lbl_1_rodata_214, lbl_1_rodata_C8);
    Hu3DModelRotSet(lbl_1_bss_10->mdlId[0], lbl_1_rodata_C8,
        lbl_1_rodata_218, lbl_1_rodata_C8);
    Hu3DModelAttrReset(lbl_1_bss_C->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_10->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[1],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 0);
    Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[1],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 0);
    lbl_1_bss_1D5C.channel[2] = fn_1_44(1150, 90, 64);
    lbl_1_bss_1D5C.channel[3] = fn_1_44(1148, 90, 64);
    object->objFunc = fn_1_2AEC;
}

void fn_1_35F4(s16 index, s16 motion, float blend, u32 attr)
{
    if (index == 0) {
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0],
            lbl_1_bss_C->mtnId[motion], lbl_1_rodata_C8, blend, attr);
    } else {
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0],
            lbl_1_bss_10->mtnId[motion], lbl_1_rodata_C8, blend, attr);
    }
}

void fn_1_39AC(s16 index, float end, float duration)
{
    EndingMotionWork *work = &lbl_1_bss_1A5C[index];
    OMOBJ *object;
    HuVecF rotation;

    work->time = lbl_1_rodata_C8;
    work->duration = duration;
    if (index == 0) {
        object = lbl_1_bss_C;
    } else {
        object = lbl_1_bss_10;
    }
    Hu3DModelRotGet(object->mdlId[0], &rotation);
    work->start = rotation.y;
    work->end = end;
    object->work[0] = index;
    object->work[1] = 1;
    object->objFunc = fn_1_36E0;
}

void fn_1_3A7C(s16 index, float end, float duration)
{
    EndingMotionWork *work = &lbl_1_bss_1A5C[index];
    OMOBJ *object;
    HuVecF position;

    work->time = lbl_1_rodata_C8;
    work->duration = duration;
    if (index == 0) {
        object = lbl_1_bss_C;
    } else {
        object = lbl_1_bss_10;
    }
    Hu3DModelPosGet(object->mdlId[0], &position);
    work->start = position.x;
    work->end = end;
    object->work[0] = index;
    object->work[1] = 0;
    object->objFunc = fn_1_36E0;
}

void fn_1_3B4C(OMOBJ *object)
{
    s16 motion;

    omSetStatBit(object, 256);
    object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        2228254, HU_MEMNUM_OVL, HEAP_MODEL));
    for (motion = 0; motion < 6; motion++) {
        object->mtnId[motion] = Hu3DJointMotion(object->mdlId[0],
            HuDataSelHeapReadNum(2228255 + motion, HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    Hu3DModelAttrSet(object->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 1073741825);
    Hu3DModelPosSet(object->mdlId[0], lbl_1_rodata_21C,
        lbl_1_rodata_C0, lbl_1_rodata_C8);
    Hu3DModelRotSet(object->mdlId[0], lbl_1_rodata_C8,
        lbl_1_rodata_214, lbl_1_rodata_C8);
    Hu3DModelScaleSet(object->mdlId[0], lbl_1_rodata_220,
        lbl_1_rodata_220, lbl_1_rodata_220);
    object->objFunc = NULL;
}

void fn_1_3CD4(OMOBJ *object)
{
    s16 motion;

    if (object) {
        for (motion = 0; motion < 6; motion++) {
            Hu3DMotionKill(object->mtnId[motion]);
        }
        Hu3DModelKill(object->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

void fn_1_3D5C(OMOBJ *object)
{
    s16 motion;

    omSetStatBit(object, 256);
    object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        2228261, HU_MEMNUM_OVL, HEAP_MODEL));
    object->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        2228262, HU_MEMNUM_OVL, HEAP_MODEL));
    for (motion = 0; motion < 6; motion++) {
        object->mtnId[motion] = Hu3DJointMotion(object->mdlId[0],
            HuDataSelHeapReadNum(2228263 + motion, HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    Hu3DModelHookSet(object->mdlId[0], lbl_1_data_CB,
        object->mdlId[1]);
    Hu3DModelAttrSet(object->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 1073741825);
    Hu3DModelPosSet(object->mdlId[0], lbl_1_rodata_C0,
        lbl_1_rodata_C0, lbl_1_rodata_C8);
    Hu3DModelRotSet(object->mdlId[0], lbl_1_rodata_C8,
        lbl_1_rodata_218, lbl_1_rodata_C8);
    Hu3DModelScaleSet(object->mdlId[0], lbl_1_rodata_220,
        lbl_1_rodata_220, lbl_1_rodata_220);
    object->objFunc = NULL;
}

void fn_1_3F20(OMOBJ *object)
{
    s16 motion;

    if (object) {
        Hu3DModelHookReset(object->mdlId[0]);
        for (motion = 0; motion < 6; motion++) {
            Hu3DMotionKill(object->mtnId[motion]);
        }
        Hu3DModelKill(object->mdlId[0]);
        Hu3DModelKill(object->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

void fn_1_3FC0(void)
{
    OMOBJ *object = lbl_1_bss_18;
    HU3D_MODEL *model = &Hu3DData[object->mdlId[0]];

    Hu3DMotionShapeSet(object->mdlId[0], object->mtnId[0]);
    Hu3DMotionShapeTimeSet(object->mdlId[0], lbl_1_rodata_C8);
    model->motShapeWork.speed = lbl_1_rodata_1E8;
    object->work[3] = 1;
}

void fn_1_4058(void)
{
    OMOBJ *object = lbl_1_bss_18;
    HU3D_MODEL *model = &Hu3DData[object->mdlId[0]];

    Hu3DMotionShapeSet(object->mdlId[0], object->mtnId[0]);
    Hu3DMotionShapeTimeSet(object->mdlId[0], lbl_1_rodata_C8);
    model->motShapeWork.speed = lbl_1_rodata_224;
    object->work[3] = 0;
}

void fn_1_40F0(OMOBJ *object)
{
    EndingMotionWork *work = lbl_1_bss_1834;
    s16 model;
    float alpha;

    for (model = 0; model < 7; model++, work++) {
        if (work->time < lbl_1_rodata_C8) {
            Hu3DModelAttrSet(object->mdlId[model + 2], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(object->mdlId[model + 9], HU3D_ATTR_DISPOFF);
        } else if (work->time >= lbl_1_rodata_C8) {
            if (lbl_1_rodata_C8 == work->time) {
                Hu3DModelAttrReset(object->mdlId[model + 2],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelTPLvlSet(object->mdlId[model + 2],
                    lbl_1_rodata_C8);
                Hu3DMotionSet(object->mdlId[model + 2],
                    object->mtnId[model + 2]);
                Hu3DModelAttrReset(object->mdlId[model + 9],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelTPLvlSet(object->mdlId[model + 9],
                    lbl_1_rodata_C8);
                Hu3DMotionSet(object->mdlId[model + 9],
                    object->mtnId[model + 2]);
            } else if (work->time < lbl_1_rodata_228) {
                Hu3DModelAttrReset(object->mdlId[model + 2],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(object->mdlId[model + 9],
                    HU3D_ATTR_DISPOFF);
                alpha = fn_1_DDF8(lbl_1_rodata_C8, lbl_1_rodata_22C,
                    work->time, lbl_1_rodata_228);
                Hu3DModelTPLvlSet(object->mdlId[model + 2], alpha);
                Hu3DModelTPLvlSet(object->mdlId[model + 9], alpha);
            } else if (work->time < lbl_1_rodata_1EC) {
                Hu3DModelAttrReset(object->mdlId[model + 2],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(object->mdlId[model + 9],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelTPLvlSet(object->mdlId[model + 2],
                    lbl_1_rodata_22C);
                Hu3DModelTPLvlSet(object->mdlId[model + 9],
                    lbl_1_rodata_22C);
            } else {
                Hu3DModelAttrReset(object->mdlId[model + 2],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(object->mdlId[model + 9],
                    HU3D_ATTR_DISPOFF);
                alpha = fn_1_DDF8(lbl_1_rodata_22C, lbl_1_rodata_C8,
                    work->time - lbl_1_rodata_1EC, lbl_1_rodata_F8);
                Hu3DModelTPLvlSet(object->mdlId[model + 2], alpha);
                Hu3DModelTPLvlSet(object->mdlId[model + 9], alpha);
            }
        }
        if ((work->time += lbl_1_rodata_19C) > lbl_1_rodata_200) {
            work->time = lbl_1_rodata_230;
            Hu3DModelAttrSet(object->mdlId[model + 2], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(object->mdlId[model + 9], HU3D_ATTR_DISPOFF);
        }
    }
    if (object->work[3] == 0) {
        for (model = 0; model < 7; model++) {
            Hu3DModelAttrSet(object->mdlId[model + 9], HU3D_ATTR_DISPOFF);
        }
    } else if (object->work[3] == 1) {
        for (model = 0; model < 7; model++) {
            Hu3DModelAttrSet(object->mdlId[model + 2], HU3D_ATTR_DISPOFF);
        }
    }
}

void fn_1_4A0C(OMOBJ *object)
{
    s16 model;

    if (object) {
        fn_1_E1EC(0, NULL);
        fn_1_EAB8(0, NULL);
        for (model = 0; model < 16; model++) {
            Hu3DMotionKill(object->mtnId[model]);
            Hu3DModelKill(object->mdlId[model]);
        }
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

void fn_1_4AB4(void)
{
    s16 window;

    fn_1_12838();
    for (window = 0; window < 4; window++) {
        HuWinExKill(lbl_1_bss_1A48[window]);
    }
    HuWinAllKill();
    Hu3DLightAllKill();
    Hu3DCameraKill(1);
}

void fn_1_4B20(OMOBJ *object)
{
    s16 window;

    if (WipeCheck() == 0) {
        HuAudFadeOut(1000);
        fn_1_12838();
        for (window = 0; window < 4; window++) {
            HuWinExKill(lbl_1_bss_1A48[window]);
        }
        HuWinAllKill();
        Hu3DLightAllKill();
        Hu3DCameraKill(1);
        HuAudAllStop();
        omOvlReturnEx(1, 1);
        object->objFunc = NULL;
    }
}

void fn_1_4DAC(void)
{
    s16 window;

    fn_1_DD14();
    HuAudSStreamFadeOut(lbl_1_bss_1DDC, 1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    fn_1_12838();
    for (window = 0; window < 4; window++) {
        HuWinExKill(lbl_1_bss_1A48[window]);
    }
    HuWinAllKill();
    Hu3DLightAllKill();
    Hu3DCameraKill(1);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

void fn_1_4E5C(void)
{
    lbl_1_bss_0 = omInitObjMan(11, 8192);
    fn_1_76C();
    fn_1_B2C();
    fn_1_F34();
    fn_1_11F34();

    lbl_1_bss_4 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1,
        fn_1_189C);
    lbl_1_bss_8 = omAddObjEx(lbl_1_bss_0, 4096, 16, 96, -1,
        fn_1_23D8);
    lbl_1_bss_14 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, NULL);
    lbl_1_bss_C = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1,
        fn_1_3B4C);
    lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1,
        fn_1_3D5C);
    lbl_1_bss_18 = omAddObjEx(lbl_1_bss_0, 4096, 32, 32, -1,
        fn_1_45C0);
    lbl_1_bss_1C = omAddObjEx(lbl_1_bss_0, 4096, 96, 16, -1, NULL);
    if (GwCommon.unkFlag4 != 0) {
        lbl_1_bss_20 = omAddObjEx(lbl_1_bss_0, 4096, 0, 0, -1,
            fn_1_4BC8);
    }
    GwCommon.unkFlag4 = 1;
    HuPrcChildCreate(fn_1_4DAC, 12288, 12288, 0, lbl_1_bss_0);
}

inline void fn_1_4E5C(void);

void fn_1_52D8(void)
{
    OSReport(lbl_1_data_DD);
    fn_1_4E5C();
}

inline void fn_1_52D8(void);

int _prolog(void)
{
    const VoidFunc *ctor;

    for (ctor = _ctors; *ctor != 0; ctor++) {
        (*ctor)();
    }
    fn_1_52D8();
    return 0;
}

void fn_1_6EC4(void)
{
    EndingModelObjectNames names = lbl_1_rodata_1B4;
    Mtx matrix;
    s16 model;
    s16 index;
    s16 motion;

    for (model = 0; model < 10; model++) {
        Hu3DModelObjMtxGet(lbl_1_bss_4->mdlId[0], names.name[model], matrix);
        Hu3DModelPosSet(lbl_1_bss_8->mdlId[model], matrix[0][3],
            matrix[1][3], matrix[2][3]);
    }
    motion = lbl_1_bss_18->mdlId[0];
    for (index = 0; index < 10; index++) {
        fn_1_1C0C(index, motion, lbl_1_rodata_19C);
    }
    HuPrcVSleep();
    Hu3DMotionSpeedSet(lbl_1_bss_4->mdlId[0], lbl_1_rodata_1E0);
}

void fn_1_8CFC(void)
{
    Hu3DLLightKill(lbl_1_bss_C->mdlId[0], lbl_1_bss_1A52[0]);
    Hu3DLLightKill(lbl_1_bss_10->mdlId[0], lbl_1_bss_1A52[1]);
    fn_1_10DEC(0, NULL, 2);
    fn_1_10DEC(1, NULL, 2);
    lbl_1_bss_10->objFunc = NULL;

    fn_1_26D4(lbl_1_bss_8);
    lbl_1_bss_8->objFunc = NULL;
    lbl_1_bss_8 = NULL;

    fn_1_3CD4(lbl_1_bss_C);
    lbl_1_bss_C->objFunc = NULL;
    lbl_1_bss_C = NULL;

    fn_1_3F20(lbl_1_bss_10);
    lbl_1_bss_10->objFunc = NULL;
    lbl_1_bss_10 = NULL;
}

void fn_1_93A0(OMOBJ *object)
{
    EndingMotionWork *work;
    s16 model;

    omSetStatBit(object, 256);
    for (model = 0, work = lbl_1_bss_34; model < 96; model++, work++) {
        work->state = 0;
    }
    for (model = 0; model < 96; model++) {
        if (model == 0) {
            object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
                2228276, HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            object->mdlId[model] = Hu3DModelLink(object->mdlId[0]);
        }
        Hu3DModelAttrSet(object->mdlId[model], HU3D_ATTR_DISPOFF);
    }
    object->objFunc = fn_1_8F80;
}

inline void fn_1_93A0(OMOBJ *object);

void fn_1_951C(void)
{
    Vec pos;
    Vec up;
    Vec target;

    Hu3DCameraPosGet(1, &pos, &up, &target);
    pos.x = lbl_1_rodata_C8;
    pos.y = lbl_1_rodata_274;
    pos.z = lbl_1_rodata_7C;
    target.x = lbl_1_rodata_C8;
    target.y = lbl_1_rodata_244;
    target.z = lbl_1_rodata_C8;
    Hu3DCameraPosSetV(1, &pos, &up, &target);
}

void fn_1_9498(OMOBJ *object)
{
    s16 model;

    if (object) {
        fn_1_111B0(0);
        for (model = 95; model >= 0; model--) {
            Hu3DModelKill(object->mdlId[model]);
        }
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

void fn_1_A58C(void)
{
    ANIMDATA *animation;
    void *fileData;
    s16 sprite;

    fileData = HuAR_ARAMtoMRAMFileRead(15859763, 805306368, HEAP_MODEL);
    animation = HuSprAnimRead(fileData);
    lbl_1_data_10E = HuSprGrpCreate(1);
    sprite = HuSprCreate(animation, 1, 0);
    HuSprGrpMemberSet(lbl_1_data_10E, 0, sprite);
    HuSprTPLvlSet(lbl_1_data_10E, 0, lbl_1_rodata_280);
    HuSprGrpPosSet(lbl_1_data_10E, lbl_1_rodata_194,
        lbl_1_rodata_198);
    HuSprScaleSet(lbl_1_data_10E, 0, lbl_1_rodata_284,
        lbl_1_rodata_284);
}

void fn_1_A6D8(void)
{
    HuVecF position = lbl_1_rodata_288;
    s16 light;

    fn_1_93A0(lbl_1_bss_1C);
    fn_1_F1B8(1, &position);
    HuPrcVSleep();
    fn_1_F23C(1);
    fn_1_F068(1, 1, &position);
    fn_1_F11C(1, 0);
    Hu3DLightAllKill();
    {
        EndingLightVectors lightDir;
        EndingLightVectors lightPos;
        GXColor color;

        lightPos = lbl_1_rodata_88;
        lightDir = lbl_1_rodata_A0;
        color = lbl_1_rodata_B8;

        for (light = 0; light < 2; light++) {
            lbl_1_bss_1A56[light] = Hu3DGLightCreateV(
                &lightPos.vector[light], &lightDir.vector[light], &color);
            Hu3DGLightInfinitytSet(lbl_1_bss_1A56[light]);
            Hu3DGLightStaticSet(lbl_1_bss_1A56[light], 1);
        }
    }
}

void fn_1_B244(void)
{
    OMOBJ *object;
    s16 index;
    HuVecF position0;
    HuVecF position1;

    omSetStatBit(lbl_1_bss_14, 256);
    object = lbl_1_bss_4;
    for (index = 0; index < 2; index++) {
        object->mdlId[index + 1] = Hu3DModelCreate(HuDataSelHeapReadNum(
            2228225 + index, HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[index + 1] = Hu3DMotionIDGet(
            object->mdlId[index + 1]);
        Hu3DModelAttrSet(object->mdlId[index + 1], HU3D_ATTR_DISPOFF);
        Hu3DMotionShiftSet(object->mdlId[index + 1],
            object->mtnId[index + 1], lbl_1_rodata_C8,
            lbl_1_rodata_C8, 1073741825);
    }

    position0 = lbl_1_rodata_294;
    position1 = lbl_1_rodata_2A0;
    fn_1_F1B8(1, &position0);
    fn_1_F23C(0);
    fn_1_F068(1, 1, NULL);
    fn_1_F11C(1, 2);

    object = lbl_1_bss_14;
    object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        2228272, HU_MEMNUM_OVL, HEAP_MODEL));
    object->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        2228275, HU_MEMNUM_OVL, HEAP_MODEL));
    for (index = 0; index < 2; index++) {
        object->mtnId[index] = Hu3DJointMotion(object->mdlId[0],
            HuDataSelHeapReadNum(2228273 + index, HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    Hu3DModelScaleSet(object->mdlId[1], lbl_1_rodata_22C,
        lbl_1_rodata_22C, lbl_1_rodata_22C);
    Hu3DModelHookSet(object->mdlId[0], lbl_1_data_110,
        object->mdlId[1]);
    Hu3DMotionSet(lbl_1_bss_14->mdlId[0], lbl_1_bss_14->mtnId[0]);
    Hu3DMotionSet(lbl_1_bss_4->mdlId[0], lbl_1_bss_4->mtnId[0]);
    HuPrcVSleep();
    Hu3DMotionTimeSet(lbl_1_bss_14->mdlId[0], lbl_1_rodata_1A0);
    Hu3DMotionTimeSet(lbl_1_bss_4->mdlId[1], lbl_1_rodata_1A0);
}

void fn_1_BB10(void)
{
    OMOBJ *object;
    s16 model;

    object = lbl_1_bss_4;
    for (model = 0; model < 2; model++) {
        Hu3DMotionKill(object->mtnId[model + 1]);
        Hu3DModelKill(object->mdlId[model + 1]);
    }
    object = lbl_1_bss_14;
    Hu3DModelHookReset(object->mdlId[0]);
    for (model = 0; model < 2; model++) {
        Hu3DMotionKill(object->mtnId[model]);
    }
    Hu3DModelKill(object->mdlId[0]);
    Hu3DModelKill(object->mdlId[1]);
}

void fn_1_BBEC(void)
{
    OMOBJ *object;
    s16 index;

    object = lbl_1_bss_4;
    for (index = 0; index < 2; index++) {
        object->mdlId[index + 3] = Hu3DModelCreate(HuDataSelHeapReadNum(
            2228227 + index, HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[index + 3] = Hu3DMotionIDGet(
            object->mdlId[index + 3]);
        Hu3DMotionShiftSet(object->mdlId[index + 3],
            object->mtnId[index + 3], lbl_1_rodata_C8,
            lbl_1_rodata_C8, 1073741825);
    }
    Hu3DModelShadowMapSet(object->mdlId[3]);
    {
        HuVecF position = lbl_1_rodata_2B8;

        fn_1_F1B8(1, &position);
    }

    object = lbl_1_bss_14;
    object->mdlId[2] = Hu3DModelCreate(HuDataSelHeapReadNum(
        2228269, HU_MEMNUM_OVL, HEAP_MODEL));
    for (index = 0; index < 2; index++) {
        object->mtnId[index + 2] = Hu3DJointMotion(object->mdlId[2],
            HuDataSelHeapReadNum(2228270 + index, HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    Hu3DModelPosSet(object->mdlId[2], lbl_1_rodata_C8,
        lbl_1_rodata_C8, lbl_1_rodata_C8);
    Hu3DMotionShiftSet(object->mdlId[2], object->mtnId[3],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 0);
    Hu3DModelShadowSet(object->mdlId[2]);
    HuPrcVSleep();
    Hu3DMotionSpeedSet(lbl_1_bss_4->mdlId[4], lbl_1_rodata_1E0);
}

void fn_1_C40C(void)
{
    OMOBJ *object;
    s16 model;

    object = lbl_1_bss_4;
    for (model = 0; model < 2; model++) {
        Hu3DMotionKill(object->mtnId[model + 3]);
        Hu3DModelKill(object->mdlId[model + 3]);
    }
    fn_1_F1B8(0, 0);
    fn_1_F068(1, 0, NULL);
    object = lbl_1_bss_14;
    for (model = 0; model < 2; model++) {
        Hu3DMotionKill(object->mtnId[model + 2]);
    }
    Hu3DModelKill(object->mdlId[2]);
}

void fn_1_C4F0(void)
{
    OMOBJ *object;
    Vec pos;
    Vec up;
    Vec target;

    fn_1_14F4();
    object = lbl_1_bss_4;
    object->mdlId[7] = Hu3DModelCreate(HuDataSelHeapReadNum(
        2228231, HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[7] = Hu3DMotionIDGet(object->mdlId[7]);
    Hu3DMotionShiftSet(object->mdlId[7], object->mtnId[7],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 1073741825);

    Hu3DCameraPosGet(1, &pos, &up, &target);
    pos.x = lbl_1_rodata_C8;
    pos.y = lbl_1_rodata_210;
    pos.z = lbl_1_rodata_2C8;
    target.x = lbl_1_rodata_C8;
    target.y = lbl_1_rodata_210;
    target.z = lbl_1_rodata_C8;
    Hu3DCameraPosSetV(1, &pos, &up, &target);
    fn_1_EB54(1);
}

inline void fn_1_12A8(s16 state);
inline void fn_1_C40C(void);
inline void fn_1_C4F0(void);

void fn_1_C7DC(void)
{
    float particleY = lbl_1_rodata_2F0;
    float phaseDuration;
    EndingScenePositions positions = lbl_1_rodata_2CC;

    HuAudFXStop(lbl_1_bss_1D5C.channel[6]);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 10);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    fn_1_C40C();
    fn_1_C4F0();

    fn_1_12A8(0);
    fn_1_EAB8(1, &positions.position[0]);
    positions.position[0].y = particleY;
    fn_1_F1B8(1, &positions.position[0]);
    fn_1_F23C(1);
    HuPrcSleep(60);
    lbl_1_bss_1D5C.channel[10] = fn_1_0(1402);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 10);
    lbl_1_bss_2C = 1;
    lbl_1_bss_24 = 1;
    phaseDuration = lbl_1_rodata_1A0;
    HuPrcSleep(60);

    lbl_1_bss_1D5C.channel[11] = fn_1_0(1403);
    fn_1_12A8(1);
    fn_1_EAB8(1, &positions.position[1]);
    positions.position[1].y = particleY;
    fn_1_F1B8(1, &positions.position[1]);
    phaseDuration = lbl_1_rodata_1A0;
    HuPrcSleep(48);

    lbl_1_bss_1D5C.channel[7] = fn_1_0(1404);
    fn_1_12A8(2);
    fn_1_EAB8(1, &positions.position[2]);
    positions.position[2].y = particleY;
    fn_1_F1B8(1, &positions.position[2]);
    phaseDuration = lbl_1_rodata_200;
    HuPrcSleep(132);
    lbl_1_bss_24 = 0;
    HuPrcSleep(5);
}

void fn_1_B0A4(void)
{
    OMOBJ *object;

    if (lbl_1_data_10E != -1) {
        HuSprGrpKill(lbl_1_data_10E);
    }
    object = lbl_1_bss_4;
    Hu3DMotionKill(object->mtnId[0]);
    Hu3DModelKill(object->mdlId[0]);

    fn_1_9498(lbl_1_bss_1C);
    lbl_1_bss_1C->objFunc = NULL;
    lbl_1_bss_1C = NULL;

    fn_1_4A0C(lbl_1_bss_18);
    lbl_1_bss_18->objFunc = NULL;
    lbl_1_bss_18 = NULL;
}

void fn_1_D028(void)
{
    OMOBJ *object = lbl_1_bss_4;

    Hu3DMotionKill(object->mtnId[7]);
    Hu3DModelKill(object->mdlId[7]);
    fn_1_EAB8(0, NULL);
    fn_1_F1B8(0, NULL);
    HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
        lbl_1_rodata_C8);
    fn_1_E0EC(lbl_1_bss_1A1C[0], 4);
    lbl_1_bss_4->work[0] = 3;
    lbl_1_bss_4->objFunc = fn_1_1160;
    lbl_1_bss_4->objFunc = NULL;
}

void fn_1_A698(void)
{
    if (lbl_1_data_10E != -1) {
        HuSprGrpKill(lbl_1_data_10E);
    }
}

void fn_1_DCF0(void)
{
    HuPrcSleep(10);
}

void fn_1_DD14(void)
{
    if (lbl_1_bss_26 == 0) {
        HuPrcSleep(10);
    }
    if (lbl_1_bss_26 == 0) {
        fn_1_702C();
    }
    if (lbl_1_bss_26 == 0) {
        fn_1_A964();
    }
    if (lbl_1_bss_26 == 0) {
        fn_1_B568();
    }
    if (lbl_1_bss_26 == 0) {
        fn_1_BE30();
    }
    if (lbl_1_bss_26 == 0) {
        fn_1_C7DC();
    }
    if (lbl_1_bss_26 == 0) {
        fn_1_D434();
    }
    if (lbl_1_bss_26 == 1) {
        while (TRUE) {
            HuPrcVSleep();
        }
    }
}

float fn_1_DDF8(float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_2F8) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + (time / duration) * (end - start);
}

float fn_1_DF18(float start, float end, float time)
{
    if (start == end || time <= lbl_1_rodata_318) {
        return end;
    }
    return (start * (time - lbl_1_rodata_318) + end) / time;
}

void fn_1_DF60(HuVecF *dest, HuVecF *target, float time)
{
    dest->x = fn_1_DF18(dest->x, target->x, time);
    dest->y = fn_1_DF18(dest->y, target->y, time);
    dest->z = fn_1_DF18(dest->z, target->z, time);
}

void fn_1_E0EC(s16 groupId, u32 attr)
{
    HUSPR_GROUP *group = &HuSprGrpData[groupId];
    s16 member;

    for (member = 0; member < group->sprNum; member++) {
        HuSprAttrSet(groupId, member, (u16)attr);
    }
}

void fn_1_E16C(s16 groupId, u32 attr)
{
    HUSPR_GROUP *group = &HuSprGrpData[groupId];
    s16 member;

    for (member = 0; member < group->sprNum; member++) {
        HuSprAttrReset(groupId, member, (u16)attr);
    }
}

void fn_1_E1EC(s16 display, HuVecF *pos)
{
    if (pos != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_1E2C, pos);
    }
    if (display == 0) {
        Hu3DModelAttrSet(lbl_1_bss_1E2C, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_1E2C, HU3D_ATTR_DISPOFF);
    }
}

void fn_1_E9A8(void)
{
    lbl_1_bss_1E2C = Hu3DParticleCreate(lbl_1_bss_1E30[0], 16);
    Hu3DModelPosSet(lbl_1_bss_1E2C, lbl_1_rodata_2F8,
        lbl_1_rodata_2F8, lbl_1_rodata_2F8);
    Hu3DModelScaleSet(lbl_1_bss_1E2C, lbl_1_rodata_318,
        lbl_1_rodata_318, lbl_1_rodata_318);
    Hu3DModelLayerSet(lbl_1_bss_1E2C, 6);
    Hu3DParticleHookSet(lbl_1_bss_1E2C, fn_1_E270);
    Hu3DParticleBlendModeSet(lbl_1_bss_1E2C, 1);
}

void fn_1_EAB8(s16 display, HuVecF *pos)
{
    if (pos != NULL) {
        Hu3DModelPosSet(lbl_1_bss_1E2A, pos->x, pos->y,
            lbl_1_rodata_340 + pos->z);
    }
    if (display == 0) {
        Hu3DModelAttrSet(lbl_1_bss_1E2A, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_1E2A, HU3D_ATTR_DISPOFF);
    }
}

void fn_1_EB54(s16 time)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E2A];
    HU3D_PARTICLE *particle = model->hookData;
    s16 i;
    HU3D_PARTICLE_DATA *data;

    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = time;
        i++;
        data++;
    }
}

void fn_1_EF58(void)
{
    lbl_1_bss_1E2A = Hu3DParticleCreate(lbl_1_bss_1E30[1], 4);
    Hu3DModelPosSet(lbl_1_bss_1E2A, lbl_1_rodata_2F8,
        lbl_1_rodata_340, lbl_1_rodata_350);
    Hu3DModelScaleSet(lbl_1_bss_1E2A, lbl_1_rodata_318,
        lbl_1_rodata_318, lbl_1_rodata_318);
    Hu3DModelLayerSet(lbl_1_bss_1E2A, 6);
    Hu3DParticleHookSet(lbl_1_bss_1E2A, fn_1_EBCC);
    Hu3DParticleBlendModeSet(lbl_1_bss_1E2A, 1);
}

void fn_1_EA8C(void)
{
    Hu3DModelKill(lbl_1_bss_1E2C);
}

void fn_1_F03C(void)
{
    Hu3DModelKill(lbl_1_bss_1E2A);
}

void fn_1_F068(s16 index, s16 display, HuVecF *pos)
{
    if (pos != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_1E26[index], pos);
    }
    if (display == 0) {
        Hu3DModelAttrSet(lbl_1_bss_1E26[index], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_1E26[index], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_F11C(s16 index, s16 count)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E26[index]];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    particle->dataCnt = count;
    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = 0;
        data->scale = lbl_1_rodata_2F8;
        i++;
        data++;
    }
}

void fn_1_F1B8(s16 display, HuVecF *pos)
{
    if (pos != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_1E26[0], pos);
    }
    if (display == 0) {
        Hu3DModelAttrSet(lbl_1_bss_1E26[0], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_1E26[0], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_F23C(s16 count)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E26[0]];
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle = model->hookData;
    s16 i;

    particle->dataCnt = count;
    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = 0;
        data->scale = lbl_1_rodata_2F8;
        i++;
        data++;
    }
}

void fn_1_F9B8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_1E26[i] = Hu3DParticleCreate(lbl_1_bss_1E30[0], 360);
        Hu3DModelPosSet(lbl_1_bss_1E26[i], lbl_1_rodata_2F8,
            lbl_1_rodata_2F8, lbl_1_rodata_2F8);
        Hu3DModelScaleSet(lbl_1_bss_1E26[i], lbl_1_rodata_318,
            lbl_1_rodata_318, lbl_1_rodata_318);
        Hu3DModelLayerSet(lbl_1_bss_1E26[i], 6);
        Hu3DParticleHookSet(lbl_1_bss_1E26[i], fn_1_F2CC);
        Hu3DParticleBlendModeSet(lbl_1_bss_1E26[i], 1);
    }
    fn_1_F068(0, 0, NULL);
    fn_1_F068(1, 0, NULL);
}

void fn_1_FB74(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_1E26[i]);
    }
}

void fn_1_FBCC(s16 index, s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_1E22[index], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_1E22[index], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_FC48(s16 index, HuVecF *pos, GXColor *color)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E22[index]];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = 1;
        if (color != NULL) {
            data->color.r = color->r;
            data->color.g = color->g;
            data->color.b = color->b;
        }
        i++;
        data++;
    }
    if (pos != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_1E22[index], pos);
    }
    Hu3DModelAttrReset(lbl_1_bss_1E22[index], HU3D_ATTR_DISPOFF);
}

void fn_1_FD4C(s16 index)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E22[index]];
    HU3D_PARTICLE *particle = model->hookData;
    s16 i;
    HU3D_PARTICLE_DATA *data;

    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = 2;
        i++;
        data++;
    }
}

void fn_1_1025C(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_1E22[i] = Hu3DParticleCreate(lbl_1_bss_1E30[1], 10);
        Hu3DModelPosSet(lbl_1_bss_1E22[i], lbl_1_rodata_2F8,
            lbl_1_rodata_2F8, lbl_1_rodata_2F8);
        Hu3DModelScaleSet(lbl_1_bss_1E22[i], lbl_1_rodata_318,
            lbl_1_rodata_318, lbl_1_rodata_318);
        Hu3DModelAttrSet(lbl_1_bss_1E22[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_1E22[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_1E22[i], fn_1_FDD4);
        Hu3DParticleBlendModeSet(lbl_1_bss_1E22[i], 1);
    }
}

void fn_1_103C8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_1E22[i]);
    }
}

void fn_1_10420(s16 index, s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_1E1E[index], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_1E1E[index], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_105D4(s16 index)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E1E[index]];
    HU3D_PARTICLE *particle = model->hookData;

    particle->dataCnt = 0;
}

void fn_1_10D94(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_1E1E[i]);
    }
}

void fn_1_10C28(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_1E1E[i] = Hu3DParticleCreate(lbl_1_bss_1E30[0], 128);
        Hu3DModelPosSet(lbl_1_bss_1E1E[i], lbl_1_rodata_2F8,
            lbl_1_rodata_2F8, lbl_1_rodata_2F8);
        Hu3DModelScaleSet(lbl_1_bss_1E1E[i], lbl_1_rodata_318,
            lbl_1_rodata_318, lbl_1_rodata_318);
        Hu3DModelAttrSet(lbl_1_bss_1E1E[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_1E1E[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_1E1E[i], fn_1_10628);
        Hu3DParticleBlendModeSet(lbl_1_bss_1E1E[i], 1);
    }
}

void fn_1_111B0(s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_1E1C, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_1E1C, HU3D_ATTR_DISPOFF);
    }
}

void fn_1_112E0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;
    s16 spawnCount = 0;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt;
            i++, data++) {
            data->time = 0;
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

void fn_1_11368(void)
{
    lbl_1_bss_1E1C = Hu3DParticleCreate(lbl_1_bss_1E30[1], 100);
    Hu3DModelPosSet(lbl_1_bss_1E1C, lbl_1_rodata_2F8,
        lbl_1_rodata_2F8, lbl_1_rodata_2F8);
    Hu3DModelScaleSet(lbl_1_bss_1E1C, lbl_1_rodata_318,
        lbl_1_rodata_318, lbl_1_rodata_318);
    Hu3DModelLayerSet(lbl_1_bss_1E1C, 6);
    Hu3DParticleHookSet(lbl_1_bss_1E1C, fn_1_112E0);
    Hu3DParticleBlendModeSet(lbl_1_bss_1E1C, 1);
}

void fn_1_1144C(void)
{
    Hu3DModelKill(lbl_1_bss_1E1C);
}

void fn_1_11478(s16 group, s16 display)
{
    s16 model;

    for (model = 0; model < 5; model++) {
        if (display != 0) {
            Hu3DModelAttrReset(lbl_1_bss_1DE0[group][model],
                HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_1DE0[group][model],
                HU3D_ATTR_DISPOFF);
        }
    }
}

void fn_1_11694(s16 group)
{
    HU3D_MODEL *modelData;
    HU3D_PARTICLE *particle;
    s16 model;

    for (model = 0; model < 5; model++) {
        modelData = &Hu3DData[lbl_1_bss_1DE0[group][model]];
        particle = modelData->hookData;
        particle->dataCnt = 0;
    }
}

void fn_1_11C94(void)
{
    EndingParticleCounts counts = lbl_1_rodata_390;
    s16 group;
    s16 model;

    for (group = 0; group < 6; group++) {
        for (model = 0; model < 5; model++) {
            lbl_1_bss_1DE0[group][model] = Hu3DParticleCreate(
                lbl_1_bss_1E30[2], counts.count[model]);
            Hu3DModelPosSet(lbl_1_bss_1DE0[group][model],
                lbl_1_rodata_2F8, lbl_1_rodata_2F8, lbl_1_rodata_2F8);
            Hu3DModelScaleSet(lbl_1_bss_1DE0[group][model],
                lbl_1_rodata_318, lbl_1_rodata_318, lbl_1_rodata_318);
            Hu3DModelAttrSet(lbl_1_bss_1DE0[group][model],
                HU3D_ATTR_DISPOFF);
            Hu3DModelLayerSet(lbl_1_bss_1DE0[group][model], 2);
            Hu3DParticleHookSet(lbl_1_bss_1DE0[group][model], fn_1_11714);
            Hu3DParticleBlendModeSet(lbl_1_bss_1DE0[group][model], 1);
        }
    }
}

void fn_1_11EB0(void)
{
    s16 group;
    s16 model;

    for (group = 0; group < 6; group++) {
        for (model = 0; model < 5; model++) {
            Hu3DModelKill(lbl_1_bss_1DE0[group][model]);
        }
    }
}

inline void fn_1_E9A8(void);
inline void fn_1_EF58(void);
inline void fn_1_F9B8(void);
inline void fn_1_1025C(void);
inline void fn_1_10C28(void);
inline void fn_1_11368(void);
inline void fn_1_11C94(void);

void fn_1_11F34(void)
{
    s16 index;

    for (index = 0; index < 3; index++) {
        lbl_1_bss_1E30[index] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_128[index], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    fn_1_E9A8();
    fn_1_EF58();
    fn_1_F9B8();
    fn_1_11368();
    fn_1_1025C();
    fn_1_10C28();
    fn_1_11C94();
}

void fn_1_12838(void)
{
    s16 firstIndex;
    s16 secondIndex;
    s16 thirdIndex;
    s16 model;
    s16 group;

    Hu3DModelKill(lbl_1_bss_1E2C);
    Hu3DModelKill(lbl_1_bss_1E2A);
    for (firstIndex = 0; firstIndex < 2; firstIndex++) {
        Hu3DModelKill(lbl_1_bss_1E26[firstIndex]);
    }
    Hu3DModelKill(lbl_1_bss_1E1C);
    for (secondIndex = 0; secondIndex < 2; secondIndex++) {
        Hu3DModelKill(lbl_1_bss_1E22[secondIndex]);
    }
    for (thirdIndex = 0; thirdIndex < 2; thirdIndex++) {
        Hu3DModelKill(lbl_1_bss_1E1E[thirdIndex]);
    }
    for (group = 0; group < 6; group++) {
        for (model = 0; model < 5; model++) {
            Hu3DModelKill(lbl_1_bss_1DE0[group][model]);
        }
    }
}

void _epilog(void)
{
    const VoidFunc *dtor;

    for (dtor = _dtors; *dtor != 0; dtor++) {
        (*dtor)();
    }
}
