/* Creates and resets the battle stage and its mode-dependent animations. */
#include <math.h>
#include "REL/mgmbattledll/module_types.h"
#include "datadir_enum.h"
#include <string.h>

#define MGBATTLE_SE_STAGE_OPEN 1221
#define MGBATTLE_SE_STAGE_CLOSE 1222
#define MGBATTLE_SE_STAGE_FINISH 1223
#define MGBATTLE_SE_STAGE_FINISH_CUE 1224
#define MGBATTLE_STAGE_LIGHT_MAX_AGE 131071
#define MGBATTLE_PARTICLE_MAX_AGE 2147483647

HU3D_MODELID lbl_1_bss_3E0;
OMOBJ *lbl_1_bss_3DC;
s16 lbl_1_bss_3D8;
OMOBJ *lbl_1_bss_3D4;
Point3d lbl_1_bss_3A4[4];
Point3d lbl_1_bss_374[4];
OMOBJ *lbl_1_bss_364[4];
OMOBJ *lbl_1_bss_354[4];
s16 lbl_1_bss_34C[4];
MgBattleWaveWork lbl_1_bss_2AC[10];
OMOBJ *lbl_1_bss_2A8;
s16 lbl_1_bss_2A4;
OMOBJ *lbl_1_bss_2A0;
Point3d lbl_1_bss_294;
OMOBJ *lbl_1_bss_290;
s16 lbl_1_bss_268[20];
s16 lbl_1_bss_240[20];
f32 lbl_1_bss_1F0[20];
Point3d lbl_1_bss_100[20];

s32 lbl_1_data_208[7] = {
    DATANUM(0x9c0000, 5), DATANUM(0x9c0000, 8),
    DATANUM(0x9c0000, 7), DATANUM(0x9c0000, 10),
    DATANUM(0x9c0000, 11), DATANUM(0x9c0000, 12),
    DATANUM(0x9c0000, 13)
};
Point3d lbl_1_data_224[7] = {
    { 0.0f, 0.0f, 0.0f },
    { 2000.0f, 0.0f, 0.0f },
    { -30.0f, -15.0f, 30.0f },
    { 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 0.0f },
    { 2000.0f, 0.0f, 0.0f }
};

Point3d lbl_1_data_278 = {0.0f, 0.0f, -600.0f};
Point3d lbl_1_data_284 = {425.0f, 0.0f, -600.0f};

void fn_1_3C34(OMOBJ *obj);
void fn_1_40A0(void);
void fn_1_5510(void);
void fn_1_6C48(void);
void fn_1_7870(void);
void fn_1_7F94(void);
void fn_1_35EC(void);
void fn_1_44E0(void);
void fn_1_45BC(void);
void fn_1_578C(void);
void fn_1_8244(void);

/* Plays the sound assigned to a battle message and recenters its window. */
void fn_1_3070(s16 winId, u32 messageId, s16 characterCode) {
    s16 entryIndex;

    for (entryIndex = 0; entryIndex < 38; entryIndex++) {
        if ((messageId == lbl_1_data_38[entryIndex].messageId) &&
            (characterCode == lbl_1_data_38[entryIndex].characterCode) &&
            (lbl_1_data_38[entryIndex].soundEffectId >= 0)) {
            HuAudFXPlayPan(lbl_1_data_38[entryIndex].soundEffectId, lbl_1_data_200);
            fn_1_45BC();
        }
    }
}

/* Sets the pan used by battle-message sound effects. */
void fn_1_314C(s16 pan) {
    if (pan < 32) pan = 32;
    if (pan > 96) pan = 96;
    lbl_1_data_200 = pan;
}

/* Creates the seven battle-stage models and the mode-dependent display. */
void fn_1_317C(void) {
    Point3d position;
    s32 modelIndex;
    HU3D_MODELID model;

    lbl_1_bss_3DC = fn_8003AB38(lbl_1_bss_674, 4608, 7, 0, -1, fn_1_3C34);
    lbl_1_bss_3DC->work[0] = lbl_1_bss_3DC->work[1] = lbl_1_bss_3DC->work[2] =
        lbl_1_bss_3DC->work[3] = 0;
    lbl_1_bss_3DC->stat |= (1 << 8);

    for (modelIndex = 0; modelIndex < 7; modelIndex++) {
        lbl_1_bss_3DC->mdlId[modelIndex] = fn_80025A04(
            fn_80008DD0(lbl_1_data_208[modelIndex], 0x10000000, 2));
        fn_80026B64(lbl_1_bss_3DC->mdlId[modelIndex], 1);
        fn_80026B8C(lbl_1_bss_3DC->mdlId[modelIndex], 0);
        fn_80026790(lbl_1_bss_3DC->mdlId[modelIndex],
            &lbl_1_data_224[modelIndex]);
        fn_800269E8(lbl_1_bss_3DC->mdlId[modelIndex], ((1 << 30) | (1 << 0)));

        if (modelIndex == 0) {
            model = fn_80025A04(
                fn_80008DD0(DATANUM(0x9c0000, 6), 0x10000000, 2));
            fn_80026FFC(model);
        }
        if (modelIndex == 1) {
            lbl_1_bss_3E0 = fn_80025A04(
                fn_80008DD0(DATANUM(0x9c0000, 9), 0x10000000, 2));
            fn_80026FFC(lbl_1_bss_3E0);
            position.y = position.z = 0.0f;
            if (MgGameNo == 3) {
                position.x = 1950.0f;
            } else if (MgGameNo == 5) {
                position.x = 2450.0f;
            } else {
                position.x = 2950.0f;
            }
            fn_80026790(lbl_1_bss_3E0, &position);
        }
        if (modelIndex == 3) {
            fn_8002D414(lbl_1_bss_3DC->mdlId[modelIndex], 120.0f);
            fn_80026FFC(lbl_1_bss_3DC->mdlId[modelIndex]);
            fn_80026A44(lbl_1_bss_3DC->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
            fn_800269E8(lbl_1_bss_3DC->mdlId[modelIndex], ((1 << 30) | (1 << 1)));
        }
        if (modelIndex == 4 || modelIndex == 5) {
            fn_8002D414(lbl_1_bss_3DC->mdlId[modelIndex], 120.0f);
            fn_80026FFC(lbl_1_bss_3DC->mdlId[modelIndex]);
            fn_800269E8(lbl_1_bss_3DC->mdlId[modelIndex], (1 << 0));
            fn_80026A44(lbl_1_bss_3DC->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
            fn_800269E8(lbl_1_bss_3DC->mdlId[modelIndex], ((1 << 30) | (1 << 1)));
        }
    }
    fn_8002D86C(lbl_1_bss_3DC->mdlId[0], 0.5f);
    fn_1_40A0();
    fn_1_5510();
    fn_1_6C48();
    fn_1_7870();
    fn_1_7F94();
}

/* Resets the battle-stage presentation and player displays. */
void fn_1_35A0(void)
{
    fn_1_35EC();
    fn_1_44E0();
    fn_1_578C();
    fn_1_8244();
}

/* Resets the stage models for the selected mode. */
void fn_1_35CC(s16 mode, s16 frameNo)
{
    fn_1_35EC();
}

/* Shows the stage model associated with the selected minigame mode. */
void fn_1_35EC(void) {
    s16 slot;
    Point3d position;
    u8 modelSlots[3] = { 3, 4, 5 };

    for (slot = 0; slot < 3; slot++) {
        fn_80026A44(
            lbl_1_bss_3DC->mdlId[modelSlots[slot]], ((1 << 30) | (1 << 0)));
        fn_800269E8(
            lbl_1_bss_3DC->mdlId[modelSlots[slot]], ((1 << 30) | (1 << 1)));
        fn_800269E8(lbl_1_bss_3DC->mdlId[modelSlots[slot]], (1 << 0));
    }

    position.x = position.y = position.z = 0.0f;
    if (MgGameNo == 3) {
        position.x = 1950.0f;
        fn_80026A44(lbl_1_bss_3DC->mdlId[3], (1 << 0));
    } else if (MgGameNo == 5) {
        position.x = 2450.0f;
        fn_80026A44(lbl_1_bss_3DC->mdlId[4], (1 << 0));
    } else if (MgGameNo == 7) {
        position.x = 2950.0f;
        fn_80026A44(lbl_1_bss_3DC->mdlId[5], (1 << 0));
    }

    fn_80026790(lbl_1_bss_3DC->mdlId[1], &position);
    fn_80026790(lbl_1_bss_3E0, &position);
    fn_80026790(lbl_1_bss_3DC->mdlId[6], &position);
    fn_80026A44(lbl_1_bss_3DC->mdlId[6], ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_3DC->mdlId[6], ((1 << 30) | (1 << 1)));
}

/* Starts the selected stage model opening animation. */
void fn_1_3840(void) {
    s16 motionMap[3] = { 3, 4, 5 };
    s16 gameIndex;

    if (MgGameNo == 3) {
        gameIndex = 0;
    } else if (MgGameNo == 5) {
        gameIndex = 1;
    } else {
        gameIndex = 2;
    }

    fn_8002D414(lbl_1_bss_3DC->mdlId[motionMap[gameIndex]], 0.0f);
    fn_8002CF98(lbl_1_bss_3DC->mdlId[motionMap[gameIndex]], 0.0f);
    lbl_1_bss_3DC->work[0] = 1;
    HuAudFXPlay(MGBATTLE_SE_STAGE_OPEN);
}

/* Reports whether the stage opening animation is active. */
s32 fn_1_394C(void) {
    if (lbl_1_bss_3DC->work[0] == 1U) {
        return 1;
    }
    return 0;
}

/* Starts the selected stage model closing animation. */
void fn_1_3974(void) {
    s16 motionMap[3] = { 3, 4, 5 };
    s16 gameIndex;

    if (MgGameNo == 3) {
        gameIndex = 0;
    } else if (MgGameNo == 5) {
        gameIndex = 1;
    } else {
        gameIndex = 2;
    }

    fn_8002D414(lbl_1_bss_3DC->mdlId[motionMap[gameIndex]], 120.0f);
    fn_8002CF98(lbl_1_bss_3DC->mdlId[motionMap[gameIndex]], 120.0f);
    lbl_1_bss_3DC->work[0] = 2;
    HuAudFXPlay(MGBATTLE_SE_STAGE_CLOSE);
}

/* Reports when the stage closing animation has reached its final frames. */
s32 fn_1_3A80(void) {
    s16 motionMap[3] = { 3, 4, 5 };
    s16 gameIndex;

    if (MgGameNo == 3) {
        gameIndex = 0;
    } else if (MgGameNo == 5) {
        gameIndex = 1;
    } else {
        gameIndex = 2;
    }

    if (lbl_1_bss_3DC->work[0] == 2 &&
        1.0f +
            fn_8002D4E0(lbl_1_bss_3DC->mdlId[motionMap[gameIndex]]) >=
            130.0f) {
        return 1;
    }
    return 0;
}

/* Returns the position of the stage display model. */
void fn_1_3B78(HuVecF *position)
{
    fn_800267CC(lbl_1_bss_3DC->mdlId[1], position);
}

extern s16 MgGameNo;

/* Starts the secondary stage animation at its ending frame for the finish sequence. */
void fn_1_3BB4(void) {
    lbl_1_bss_3DC->work[1] = 1;
    fn_8002D414(lbl_1_bss_3DC->mdlId[6], 241.0f);
    fn_8002CF98(lbl_1_bss_3DC->mdlId[6], 241.0f);
    HuAudFXPlay(MGBATTLE_SE_STAGE_FINISH);
}

/* Per-frame callback that advances the selected stage model and background motions. */
void fn_1_3C34(OMOBJ *obj) {
    s16 motionSelection[3] = {3, 4, 5};
    s16 gameIndex;
    f32 maxTime;

    if (MgGameNo == 3) {
        gameIndex = 0;
    } else if (MgGameNo == 5) {
        gameIndex = 1;
    } else {
        gameIndex = 2;
    }
    if (obj->work[0] == 0) {
        fn_80026A44(obj->mdlId[motionSelection[gameIndex]],
            ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        fn_800269E8(obj->mdlId[motionSelection[gameIndex]],
            ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
            ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_8002D414(obj->mdlId[motionSelection[gameIndex]], 120.0f);
        fn_8002CF98(obj->mdlId[motionSelection[gameIndex]], 120.0f);
    } else if (obj->work[0] == 1) {
        fn_80026A44(obj->mdlId[motionSelection[gameIndex]],
            ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
            ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_800269E8(obj->mdlId[motionSelection[gameIndex]],
            ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        if (1.0f +
                fn_8002D4E0(obj->mdlId[motionSelection[gameIndex]]) >=
            120.0f) {
            obj->work[0] = 0;
        }
    } else {
        fn_80026A44(obj->mdlId[motionSelection[gameIndex]],
            ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
            ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_800269E8(obj->mdlId[motionSelection[gameIndex]],
            ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        maxTime = fn_8002D530(obj->mdlId[motionSelection[gameIndex]]);
        if (1.0f +
                fn_8002D4E0(obj->mdlId[motionSelection[gameIndex]]) >= maxTime) {
            fn_80026A44(obj->mdlId[motionSelection[gameIndex]],
                ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
            fn_800269E8(obj->mdlId[motionSelection[gameIndex]],
                ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        }
    }

    if (obj->work[1] == 0) {
        fn_80026A44(obj->mdlId[6], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
            ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_800269E8(obj->mdlId[6], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        if (1.0f + fn_8002D4E0(obj->mdlId[6]) >= 240.0f) {
            fn_8002D414(obj->mdlId[6], 0.0f);
            fn_8002CF98(obj->mdlId[6], 0.0f);
        }
    } else {
        fn_80026A44(obj->mdlId[6], ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
            ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
        fn_800269E8(obj->mdlId[6], ((1 << 30) | (1 << 0)) | ((1 << 30) | (1 << 6)));
        maxTime = fn_8002D530(obj->mdlId[6]);
        if (1.0f + fn_8002D4E0(obj->mdlId[6]) >= maxTime) {
            fn_8002D414(obj->mdlId[6], 461.0f);
            fn_8002CF98(obj->mdlId[6], 461.0f);
        }
        if (380.0f == 1.0f + fn_8002D4E0(obj->mdlId[6])) {
            HuAudFXPlay(MGBATTLE_SE_STAGE_FINISH_CUE);
        }
    }
}

void fn_1_409C(void)
{
}

extern HUPROCESS *lbl_1_bss_674;
void fn_1_45E4(OMOBJ *obj);

/* Creates the foreground stage ornament, its attached model, and its motion set. */
void fn_1_40A0(void)
{
    Point3d *position;

    lbl_1_bss_3D4 = fn_8003AB38(lbl_1_bss_674, 4864, 2U, 3U, -1, fn_1_45E4);
    lbl_1_bss_3D4->data = HuMemDirectMallocNum(HEAP_HEAP, 24, 0x10000000);
    lbl_1_bss_3D4->work[0] = 0;
    lbl_1_bss_3D4->work[1] = 0;
    lbl_1_bss_3D4->work[2] = 0;
    memset(lbl_1_bss_3D4->data, 0, 24);
    position = ((Point3d *)(lbl_1_bss_3D4->data));
    PSVECAdd(&lbl_1_data_278, &lbl_80266A4C, position);
    position[1] = position[0];
    *lbl_1_bss_3D4->mdlId = fn_80025A04(
        fn_80008DD0(DATANUM(0x9c0000, 52), 0x10000000, 2));
    fn_80026B64(*lbl_1_bss_3D4->mdlId, 1);
    fn_80026B8C(*lbl_1_bss_3D4->mdlId, 0);
    fn_80026790(*lbl_1_bss_3D4->mdlId, position);
    fn_800269E8(*lbl_1_bss_3D4->mdlId, ((1 << 30) | (1 << 0)));
    fn_800268B0(*lbl_1_bss_3D4->mdlId, 1.375f, 1.375f, 1.375f);
    lbl_1_bss_3D4->stat |= (1 << 8);
    Hu3DLLightCreate(*lbl_1_bss_3D4->mdlId, 0.0f, 1500.0f, 1500.0f, 0.0f, -0.1f, -1.0f, 255U,
                     255U, 255U);
    lbl_1_bss_3D4->mdlId[1] = fn_80025A04(
        fn_80008DD0(DATANUM(0x9c0000, 56), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_3D4->mdlId[1], 1);
    fn_80026B8C(lbl_1_bss_3D4->mdlId[1], 0);
    fn_8002741C(*lbl_1_bss_3D4->mdlId, "f-itemhook_R", lbl_1_bss_3D4->mdlId[1]);
    fn_800269E8(lbl_1_bss_3D4->mdlId[1], ((1 << 30) | (1 << 0)));
    *lbl_1_bss_3D4->mtnId =
        fn_80030458(*lbl_1_bss_3D4->mdlId, fn_80008DD0(DATANUM(0x9c0000, 53),
                                                                    0x10000000, 2));
    fn_8002C95C(*lbl_1_bss_3D4->mdlId, *lbl_1_bss_3D4->mtnId);
    lbl_1_bss_3D4->mtnId[1] =
        fn_80030458(*lbl_1_bss_3D4->mdlId, fn_80008DD0(DATANUM(0x9c0000, 54),
                                                                    0x10000000, 2));
    lbl_1_bss_3D4->mtnId[2] =
        fn_80030458(*lbl_1_bss_3D4->mdlId, fn_80008DD0(DATANUM(0x9c0000, 55),
                                                                    0x10000000, 2));
    lbl_1_bss_3D8 = 0;
}

/* Repositions the foreground ornament relative to the current stage center. */
void fn_1_44E0(void) {
    Point3d *ornamentPosition;

    ornamentPosition = ((Point3d *)(lbl_1_bss_3D4->data));
    PSVECAdd(&lbl_1_data_278, &lbl_80266A4C, ornamentPosition);
    fn_80026790(*lbl_1_bss_3D4->mdlId, ornamentPosition);
}

/* Requests the ornament's open or close transition from its current state. */
void fn_1_4548(u16 requestedOpen) {
    if ((lbl_1_bss_3D4->work[1] == 0U) && (requestedOpen != 0U)) {
        lbl_1_bss_3D4->work[1] = 1U;
    }
    if ((lbl_1_bss_3D4->work[1] != 0U) && (requestedOpen == 0U)) {
        lbl_1_bss_3D4->work[1] = 2U;
    }
}

/* Marks the foreground ornament as ready to play its opening motion. */
void fn_1_45BC(void) {
    lbl_1_bss_3D4->work[0] = 1;
}

/* Returns whether the foreground ornament's approach movement is still active. */
s16 fn_1_45D4(void) {
    return lbl_1_bss_3D8;
}

extern s16 lbl_1_bss_672;
extern s32 fn_1_1AF5C(Point3d *, Point3d *);
extern void fn_1_1B0AC(f32, f32, f32, const Vec *, Vec *);
extern f64 fn_1_1B16C(Point3d *, Point3d *);

/* Updates the foreground ornament's orbit, position, and requested motion each frame. */
void fn_1_45E4(OMOBJ *work) {
    Vec *position;
    Vec separation;
    Vec axis;
    Vec rotation;
    Vec zAxisInput;
    Vec zAxisInput2;
    Vec xAxisInput;
    Vec zAxisInput3;
    Vec zAxisInput4;
    Vec angleSeparation;
    Vec angleDirection;
    Vec xAxisInput2;
    Vec zAxisInput5;
    Vec zAxisInput6;
    Vec shortAngleSeparation;
    Vec shortAngleDirection;
    Vec *zAxisInputPointer;
    Vec *zAxisInput2Pointer;
    Vec *xAxisInputPointer;
    Vec *zAxisInput3Pointer;
    Vec *zAxisInput4Pointer;
    Vec *angleDirectionPointer;
    Vec *angleSeparationPointer;
    Vec *xAxisInput2Pointer;
    Vec *zAxisInput5Pointer;
    Vec *zAxisInput6Pointer;
    Vec *shortAngleDirectionPointer;
    Vec *shortAngleSeparationPointer;

    f32 firstDot;
    f32 secondDot;
    f32 previousYaw;

    s16 adjustPosition;

    adjustPosition = 0;
    position = work->data;

    fn_80026874(*work->mdlId, &rotation);

    switch (lbl_1_bss_672) {
        case 0:
            position[1] = lbl_1_data_278;
            position[1].y += 25.0f;
            break;
        case 1:
        case 2:
        case 3:
            PSVECAdd(&lbl_1_data_284, &lbl_80266A4C, &position[1]);
            break;
        case 4:
            PSVECAdd(&lbl_1_data_278, &lbl_80266A4C, &position[1]);
            position[1].x = 975.0f;
            position[1].y -= 100.0f;
            break;
        case 5:
            PSVECAdd(&lbl_1_data_284, &lbl_80266A4C, &position[1]);
            if (lbl_802C028C <= 2800.0f) {
                goto camera_offset_done;
            }
            if (lbl_802C028C <= 3000.0f) {
                position[1].x += 100.0f;
            } else if (lbl_802C028C <= 3800.0f) {
                position[1].x += 150.0f;
            } else {
                position[1].x += 300.0f;
            }
        camera_offset_done:
            position[1].y -= 100.0f;
            break;
        case 6:
            work->objFunc = 0;
            break;
        case 7:
        case 8:
            PSVECAdd(&lbl_1_data_278, &lbl_80266A4C, &position[1]);
            position[1].y -= 100.0f;
            break;
        case 9: {
            separation.x = -50.0f;
            separation.y = -50.0f;
            separation.z = 0.0f;
            PSVECAdd(&lbl_1_data_284, &lbl_80266A4C, &position[1]);
            PSVECAdd(&position[1], &separation, &position[1]);
            break;
        }
        case 10:
        case 11:
            PSVECAdd(&lbl_1_data_278, &lbl_80266A4C, &position[1]);
            break;
    }

    PSVECSubtract(&position[1], position, &separation);
    if (PSVECMag(&separation) > 50.0f) {
        lbl_1_bss_3D8 = 1;
        separation.y = 0.0f;
        fn_1_1AF5C(&separation, &separation);
        if (separation.x < 0.0f) {
            separation.x = separation.y = 0.0f;
            separation.z = 1.0f;
            zAxisInput = separation;
            zAxisInputPointer = &zAxisInput;
            fn_1_1B0AC(0.0f, -40.0f, 0.0f, zAxisInputPointer, &separation);
        } else if (separation.x > 0.0f) {
            separation.x = separation.y = 0.0f;
            separation.z = 1.0f;
            zAxisInput2 = separation;
            zAxisInput2Pointer = &zAxisInput2;
            fn_1_1B0AC(0.0f, 40.0f, 0.0f, zAxisInput2Pointer, &separation);
        }

        adjustPosition = 0;
        previousYaw = rotation.y;
        axis.x = 1.0f;
        axis.y = 0.0f;
        axis.z = 0.0f;
        xAxisInput = axis;
        xAxisInputPointer = &xAxisInput;
        fn_1_1B0AC(0.0f, rotation.y, 0.0f, xAxisInputPointer, &axis);
        firstDot = PSVECDotProduct(&separation, &axis);
        axis.x = axis.y = 0.0f;
        axis.z = 1.0f;
        zAxisInput3 = axis;
        zAxisInput3Pointer = &zAxisInput3;
        fn_1_1B0AC(0.0f, rotation.y, 0.0f, zAxisInput3Pointer, &axis);
        secondDot = PSVECDotProduct(&separation, &axis);
        if (secondDot >= 0.0f) {
            if (firstDot > 0.0f) {
                rotation.y += 20.0f;
            } else if (firstDot < 0.0f) {
                rotation.y -= 20.0f;
            }
            axis.x = axis.y = 0.0f;
            axis.z = 1.0f;
            zAxisInput4 = axis;
            zAxisInput4Pointer = &zAxisInput4;
            fn_1_1B0AC(0.0f, rotation.y, 0.0f, zAxisInput4Pointer, &axis);
            angleDirection = axis;
            angleDirectionPointer = &angleDirection;
            angleSeparation = separation;
            angleSeparationPointer = &angleSeparation;
            secondDot = (f32)fn_1_1B16C(angleSeparationPointer, angleDirectionPointer);
            if (-20.0f < secondDot && secondDot < 20.0f) {
                rotation.y = previousYaw;
                adjustPosition = 1;
            }
        } else {
            if (firstDot >= 0.0f) {
                rotation.y += 20.0f;
            } else {
                rotation.y -= 20.0f;
            }
        }

        if (adjustPosition != 0) {
            f32 separationScale;

            PSVECSubtract(&position[1], position, &separation);
            if (PSVECMag(&separation) > 5.0f) {
                fn_1_1AF5C(&separation, &separation);
                firstDot = 5.0f;
                PSVECScale(&separation, &separation, firstDot);
                PSVECAdd(position, &separation, position);
            } else {
                *position = position[1];
                lbl_1_bss_3D8 = 0;
            }
        }
    } else {
        lbl_1_bss_3D8 = 0;
        position->x += 0.1f * separation.x;
        position->y += 0.1f * separation.y;
        position->z += 0.1f * separation.z;
        separation.x = 0.0f;
        separation.y = 0.0f;
        separation.z = 1.0f;
        axis.x = 1.0f;
        axis.y = 0.0f;
        axis.z = 0.0f;
        xAxisInput2 = axis;
        xAxisInput2Pointer = &xAxisInput2;
        fn_1_1B0AC(0.0f, rotation.y, 0.0f, xAxisInput2Pointer, &axis);
        firstDot = PSVECDotProduct(&separation, &axis);
        axis.x = axis.y = 0.0f;
        axis.z = 1.0f;
        zAxisInput5 = axis;
        zAxisInput5Pointer = &zAxisInput5;
        fn_1_1B0AC(0.0f, rotation.y, 0.0f, zAxisInput5Pointer, &axis);
        secondDot = PSVECDotProduct(&separation, &axis);
        if (secondDot >= 0.0f) {
            if (firstDot > 0.0f) {
                rotation.y += 20.0f;
            } else if (firstDot < 0.0f) {
                rotation.y -= 20.0f;
            }
            axis.x = axis.y = 0.0f;
            axis.z = 1.0f;
            zAxisInput6 = axis;
            zAxisInput6Pointer = &zAxisInput6;
            fn_1_1B0AC(0.0f, rotation.y, 0.0f, zAxisInput6Pointer, &axis);
            shortAngleDirection = axis;
            shortAngleDirectionPointer = &shortAngleDirection;
            shortAngleSeparation = separation;
            shortAngleSeparationPointer = &shortAngleSeparation;
            secondDot = (f32)fn_1_1B16C(shortAngleSeparationPointer, shortAngleDirectionPointer);
            if (-20.0f < secondDot && secondDot < 20.0f) {
                rotation.y = 0.0f;
            }
        } else if (firstDot >= 0.0f) {
            rotation.y += 20.0f;
        } else {
            rotation.y -= 20.0f;
        }
    }

    work->work[2] += 5;
    if (work->work[2] >= 360) {
        work->work[2] -= 360;
    }
    position->y = (f32)((f64)position->y +
        (2.0 * sin((3.141592653589793 * (f64)(f32)work->work[2]) / 180.0)));
    fn_80026838(*work->mdlId, &rotation);
    fn_80026790(*work->mdlId, position);

    if (work->work[1] != 0) {
        if (work->work[1] == 1) {
            if (fn_8002D3EC(*work->mdlId) < 0 &&
                work->mtnId[2] != fn_8002D3C4(*work->mdlId)) {
                fn_8002CCC0(*work->mdlId, work->mtnId[2], 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
                fn_8002D86C(*work->mdlId, 1.0f);
                work->work[0] = 0;
            }
            if (fn_8002D3EC(*work->mdlId) < 0 &&
                work->mtnId[2] == fn_8002D3C4(*work->mdlId) &&
                (fn_8002D4E0(*work->mdlId) == 6.0f ||
                 fn_8002D4E0(*work->mdlId) == 22.0f ||
                 fn_8002D4E0(*work->mdlId) == 38.0f ||
                 fn_8002D4E0(*work->mdlId) == 54.0f ||
                 fn_8002D4E0(*work->mdlId) == 70.0f)) {
                HuAudFXPlay(940);
            }
        } else if (fn_8002D3EC(*work->mdlId) < 0) {
            fn_800269E8(*work->mdlId, ((1 << 30) | (1 << 1)));
            fn_8002CCC0(*work->mdlId, work->mtnId[0], 0.0f, 8.0f,
                ((1 << 30) | (1 << 0)));
            fn_8002D86C(*work->mdlId, 1.0f);
            work->work[1] = 0;
            work->work[0] = 0;
        }
    } else if (work->work[0] != 0) {
        if (fn_8002D3EC(*work->mdlId) < 0 &&
            work->mtnId[1] != fn_8002D3C4(*work->mdlId)) {
            fn_8002CCC0(*work->mdlId, work->mtnId[1], 0.0f, 8.0f,
                ((1 << 30) | (1 << 0)));
            fn_8002D86C(*work->mdlId, 2.0f);
            return;
        }
        if (fn_8002D3EC(*work->mdlId) < 0 &&
            work->mtnId[1] == fn_8002D3C4(*work->mdlId)) {
            if ((lbl_802C01C8[*work->mdlId].motWork.speed + fn_8002D4E0(*work->mdlId)) >=
                fn_8002D530(*work->mdlId)) {
                fn_8002CCC0(*work->mdlId, work->mtnId[0], 0.0f, 8.0f,
                    ((1 << 30) | (1 << 0)));
                fn_8002D86C(*work->mdlId, 1.0f);
                work->work[0] = 0;
            }
        }
    }
}

extern s32 lbl_1_data_8FC[4];

void fn_1_5EAC(OMOBJ *obj);
f32 fn_1_AD8C(s16 player);
s16 fn_1_BAEC(s16 player);

Point3d lbl_1_data_2D0[4] = { { -600.0f, -600.0f, 200.0f },
                              { -600.0f, -600.0f, -300.0f },
                              { 600.0f, -600.0f, -200.0f },
                              { 600.0f, -600.0f, 200.0f } };

/* Creates one animated stage marker model for each player during stage setup. */
void fn_1_5510(void) {
    s32 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_364[playerIndex] =
            fn_8003AB38(lbl_1_bss_674, 5120, 1, 1, -1, fn_1_5EAC);
        lbl_1_bss_364[playerIndex]->work[0] = playerIndex;
        lbl_1_bss_364[playerIndex]->work[1] = 0;
        lbl_1_bss_364[playerIndex]->work[2] = 0;
        lbl_1_bss_364[playerIndex]->work[3] = 0;
        lbl_1_bss_364[playerIndex]->data = NULL;
        *lbl_1_bss_364[playerIndex]->mdlId = fn_80025A04(
        fn_80008DD0(DATANUM(0x9c0000, 57), 0x10000000, 2));
        fn_80026B64(*lbl_1_bss_364[playerIndex]->mdlId, 1);
        fn_80026B8C(*lbl_1_bss_364[playerIndex]->mdlId, 0);
        *lbl_1_bss_364[playerIndex]->mtnId = fn_80030458(
            *lbl_1_bss_364[playerIndex]->mdlId,
        fn_80008DD0(DATANUM(0x9c0000, 58), 0x10000000, 2));
        fn_8002C95C(*lbl_1_bss_364[playerIndex]->mdlId,
                      *lbl_1_bss_364[playerIndex]->mtnId);
        fn_800269E8(*lbl_1_bss_364[playerIndex]->mdlId, ((1 << 30) | (1 << 0)));
        lbl_1_bss_364[playerIndex]->stat |= (1 << 8);
        fn_800269E8(*lbl_1_bss_364[playerIndex]->mdlId, (1 << 0));
    }
}

/* Offsets the player marker positions by the current stage display position. */
void fn_1_578C(void) {
    Point3d position;
    s16 player;

    player = 0;
    while (player < 4) {
        fn_800267CC(lbl_1_bss_3DC->mdlId[1], &position);
        lbl_1_data_2D0[player].x += position.x;
        player += 1;
    }
}

/* Attaches a player's character model to its stage marker and lays out active markers. */
void fn_1_5808(s16 player, s16 modelId) {
    Point3d position;
    s16 playerPosition;
    s16 index;
    s16 playerCount;

    playerPosition = 0;
    playerCount = 0;
    fn_800267CC(lbl_1_bss_3DC->mdlId[1], &position);
    index = 0;
    while (index < 4) {
        if (fn_1_BAEC(index) < 0) {
            if (index == player) {
                playerPosition = playerCount;
            }
            playerCount++;
        }
        index++;
    }

    fn_80026A44(lbl_1_bss_364[player]->mdlId[0], (1 << 0));
    lbl_1_bss_364[player]->work[2] = modelId;
    fn_8002741C(lbl_1_bss_364[player]->mdlId[0], "itemhook_C",
        modelId);
    lbl_1_bss_374[player].x = 0.0f;
    if (lbl_80265728[player].charNo == 6) {
        lbl_1_bss_374[player].y = -(0.6f * fn_1_AD8C(player));
    } else if (lbl_80265728[player].charNo == 3) {
        lbl_1_bss_374[player].y = -(0.7f * fn_1_AD8C(player));
    } else {
        lbl_1_bss_374[player].y = -(0.8f * fn_1_AD8C(player));
    }
    lbl_1_bss_374[player].z = 70.0f;
    fn_80026790(modelId, &lbl_1_bss_374[player]);
    fn_80026808(modelId, 0.0f, 0.0f, 0.0f);
    fn_80026790(lbl_1_bss_364[player]->mdlId[0],
        &lbl_1_data_2D0[playerPosition]);

    if (playerCount == 1) {
        lbl_1_bss_3A4[player].x = position.x - 300.0f;
        lbl_1_bss_3A4[player].y = 0.0f;
        lbl_1_bss_3A4[player].z = 100.0f;
    } else if (playerCount == 2) {
        if (playerPosition == 0) {
            lbl_1_bss_3A4[player].x = position.x - 300.0f;
        } else {
            lbl_1_bss_3A4[player].x = 300.0f + position.x;
            fn_80026790(lbl_1_bss_364[player]->mdlId[0],
                &lbl_1_data_2D0[playerPosition + 1]);
        }
        lbl_1_bss_3A4[player].y = 0.0f;
        lbl_1_bss_3A4[player].z = 100.0f;
    } else {
        if (playerPosition == 0) {
            lbl_1_bss_3A4[player].x = position.x - 300.0f;
        } else if (playerPosition == 1) {
            lbl_1_bss_3A4[player].x = position.x - 110.0f;
        } else {
            lbl_1_bss_3A4[player].x = 150.0f + position.x;
        }
        lbl_1_bss_3A4[player].y = 0.0f;
        lbl_1_bss_3A4[player].z = 100.0f;
    }

    lbl_1_bss_364[player]->work[1] = 1;
    if (lbl_1_data_8FC[player] < 0) {
        lbl_1_data_8FC[player] = HuAudFXEmiterPlay(963,
            &lbl_1_bss_3A4[player]);
    }
}

/* Reports whether a player's stage-marker animation has reached a settled state. */
s32 fn_1_5DA8(s16 playerIndex) {
    if ((lbl_1_bss_364[playerIndex]->work[1] == 0U) ||
        (lbl_1_bss_364[playerIndex]->work[1] == 5U) ||
        (lbl_1_bss_364[playerIndex]->work[1] == 4U)) {
        return 1;
    }
    return 0;
}

/* Reports whether any player stage-marker animation is still running. */
s32 fn_1_5E24(void) {
    s16 playerIndex;
    s32 anyPlayerAnimating;

    anyPlayerAnimating = 0;
    playerIndex = 0;
    while (playerIndex < 4) {
        if (lbl_1_bss_364[playerIndex]->work[1] != 0U &&
            lbl_1_bss_364[playerIndex]->work[1] != 5U) {
            anyPlayerAnimating = 1;
        }
        playerIndex += 1;
    }
    return anyPlayerAnimating;
}

/* Per-frame callback that steers a player's marker and advances its motion states. */
void fn_1_5EAC(OMOBJ *obj) {
    s16 player;
    s16 modelId;
    Point3d position;
    Point3d direction;
    f32 limit;

    player = obj->work[0];
    modelId = obj->work[2];
    limit = fn_1_AD8C(player);

    if (obj->work[1] == 0) {
        return;
    }

    switch (obj->work[1]) {
    case 1:
        fn_800267CC(obj->mdlId[0], &direction);
        direction.y += 7.0f;
        fn_80026790(obj->mdlId[0], &direction);
        if (direction.y >= limit) {
            obj->work[1] = 2;
            obj->work[3] = 0;
        }
        if (lbl_1_data_8FC[player] >= 0) {
            HuAudFXEmiterUpDate(lbl_1_data_8FC[player], &direction);
        }
        break;
    case 2:
        obj->work[3]++;
        fn_800267CC(obj->mdlId[0], &position);
        PSVECSubtract(&lbl_1_bss_3A4[player], &position, &direction);
        direction.y = 0.0f;
        if (PSVECMag(&direction) > 7.0f) {
            fn_1_1AF5C(&direction, &direction);
            PSVECScale(&direction, &direction, 7.0f);
            position.x += direction.x *
                sin((3.141592653589793 * obj->work[3]) / 180.0);
            position.y += 4.0 *
                cos((3.141592653589793 * obj->work[3]) / 180.0);
            position.z += direction.z *
                sin((3.141592653589793 * obj->work[3]) / 180.0);
        } else {
            position.x = lbl_1_bss_3A4[player].x;
            position.z = lbl_1_bss_3A4[player].z;
            obj->work[1] = 3;
            obj->work[3] = 0;
        }
        fn_80026790(obj->mdlId[0], &position);
        if (lbl_1_data_8FC[player] >= 0) {
            HuAudFXEmiterUpDate(lbl_1_data_8FC[player], &position);
        }
        break;
    case 3:
        fn_80027508(obj->mdlId[0]);
        fn_800267CC(obj->mdlId[0], &direction);
        PSVECAdd(&direction, &lbl_1_bss_374[player], &direction);
        fn_80026790(modelId, &direction);
        fn_80026808(modelId, 0.0f,
                        0.0f, 0.0f);
        obj->work[1] = 4;
        if (lbl_1_data_8FC[player] >= 0) {
            HuAudFXFadeOut(lbl_1_data_8FC[player], 500);
        }
        lbl_1_data_8FC[player] = -1;
        break;
    case 4:
        fn_800267CC(obj->mdlId[0], &direction);
        direction.y += 7.0f;
        fn_80026790(obj->mdlId[0], &direction);
        if (direction.y >= 1000.0f) {
            fn_800269E8(obj->mdlId[0], (1 << 0));
            obj->work[1] = 5;
        }
        break;
    }
}

#define sind(x) sin((3.141592653589793 * (x)) / 180.0)

/* Interpolates between endpoints with a quarter-sine ease over the requested frames. */
float fn_1_6380(float startValue, float endValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f) {
        return startValue;
    }
    if (elapsedFrames >= durationFrames) {
        return endValue;
    }
    return startValue + ((endValue - startValue) * sind((90.0f / durationFrames) * elapsedFrames));
}

/* Applies a half-sine pulse that returns to the start value at either endpoint. */
float fn_1_6468(float startValue, float peakValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f || elapsedFrames >= durationFrames) {
        return startValue;
    }
    return startValue +
           ((peakValue - startValue) * sind((180.0f / durationFrames) * elapsedFrames));
}

/* Stores normalized height weights for each wave model's mesh vertices. */
void fn_1_653C(OMOBJ *obj) {
    HU3D_MODEL *modelRecord = NULL;
    HSF_DATA *model = NULL;
    HSF_OBJECT *object = NULL;
    s16 modelIndex;
    s16 objectIndex;
    s16 vertexIndex;
    s16 outputSize;
    f32 maximumY;
    f32 minimumY;
    f32 range;
    Vec *vertex;
    MgBattleWaveVertex *output;
    MgBattleWaveVertex *outputVertex;

    modelIndex = 0;
    while (modelIndex < 10) {
        modelRecord = &lbl_802C01C8[obj->mdlId[modelIndex]];
        model = modelRecord->hsf;
        object = model->object;
        maximumY = 9999.0f;
        minimumY = 0.0f;
        objectIndex = 0;
        while (objectIndex < model->objectNum) {
            if (object->type == 2) {
                outputSize = object->mesh.vertex->count * sizeof(MgBattleWaveVertex);
                vertex = object->mesh.vertex->data;
                vertexIndex = 0;
                while (vertexIndex < object->mesh.vertex->count) {
                    if (vertex->y > minimumY) {
                        minimumY = vertex->y;
                    }
                    if (vertex->y < maximumY) {
                        maximumY = vertex->y;
                    }
                    vertexIndex++;
                    vertex++;
                }
                range = minimumY - maximumY;
                output = HuMemDirectMallocNum(2, outputSize, 0x10000000);
                outputVertex = output;
                vertex = object->mesh.vertex->data;
                vertexIndex = 0;
                while (vertexIndex < object->mesh.vertex->count) {
                    outputVertex->position = *vertex;
                    outputVertex->weight = (vertex->y - maximumY) / range;
                    vertexIndex++;
                    vertex++;
                    outputVertex++;
                }
                lbl_1_bss_2AC[modelIndex].vertices = output;
                break;
            } else {
                objectIndex++;
                continue;
            }
        }
        modelIndex++;
    }
}

/* Deforms the active wave mesh each frame and animates its height and rotation. */
void fn_1_6718(OMOBJ *object) {
    HU3D_MODEL *model = NULL;
    HSF_DATA *modelData = NULL;
    HSF_OBJECT *modelObject = NULL;
    s16 modelIndex;
    s16 objectIndex;
    s16 vertexIndex;
    f32 weightDuration;
    f32 rotationAngle;
    f32 weight;
    f32 weightValue;
    f32 height;
    f32 heightDuration;
    f32 elapsedAtStart;
    f32 elapsedForHeight;
    Mtx endRotation;
    Mtx startRotation;
    Point3d startPosition;
    Point3d endPosition;
    Point3d modelPosition;

    for (modelIndex = 0; modelIndex < 10; modelIndex++) {
        if (lbl_1_bss_2AC[modelIndex].active == 1 &&
            (u32)modelIndex == (u32)object->work[2]) {
            model = &lbl_802C01C8[object->mdlId[modelIndex]];
            modelData = model->hsf;
            modelObject = modelData->object;

            fn_80026A44(object->mdlId[modelIndex], (1 << 0));
            weight = fn_1_6380(0.0f, 1.0f, lbl_1_bss_2AC[modelIndex].elapsed,
                lbl_1_bss_2AC[modelIndex].duration);

            if (++lbl_1_bss_2AC[modelIndex].elapsed >
                lbl_1_bss_2AC[modelIndex].duration) {
                lbl_1_bss_2AC[modelIndex].active = 0;
            }

            rotationAngle = 450.0f * weight;
            if (rotationAngle > 360.0f) {
                rotationAngle = 360.0f;
            }
            PSMTXRotRad(startRotation, 'y', 0.017453292f * rotationAngle);

            rotationAngle = (450.0f * weight) - 90.0f;
            if (rotationAngle < 0.0f) {
                rotationAngle = 0.0f;
            }
            PSMTXRotRad(endRotation, 'y', 0.017453292f * rotationAngle);

            weight = fn_1_6468(350.0f, 500.0f, lbl_1_bss_2AC[modelIndex].elapsed,
                lbl_1_bss_2AC[modelIndex].duration);

            fn_800267CC(object->mdlId[modelIndex], &modelPosition);
            modelPosition.y = weight;
            fn_80026790(object->mdlId[modelIndex], &modelPosition);

            objectIndex = 0;
            while (objectIndex < modelData->objectNum) {
                if (modelObject->type == 2) {
                    MgBattleWaveVertex *source =
                        lbl_1_bss_2AC[modelIndex].vertices;
                    Point3d *vertex = modelObject->mesh.vertex->data;

                    for (vertexIndex = 0;
                         vertexIndex < modelObject->mesh.vertex->count;
                         vertexIndex++,
                             vertex++,
                             source++) {
                        PSMTXMultVec(endRotation, &source->position, &startPosition);
                        PSMTXMultVec(startRotation, &source->position, &endPosition);
                        vertex->x = startPosition.x +
                            (source->weight *
                                (endPosition.x - startPosition.x));
                        vertex->y = startPosition.y +
                            (source->weight *
                                (endPosition.y - startPosition.y));
                        vertex->z = startPosition.z +
                            (source->weight *
                                (endPosition.z - startPosition.z));
                    }
                    DCStoreRangeNoSync(modelObject->mesh.vertex->data,
                        modelObject->mesh.vertex->count * 12);
                    break;
                } else {
                    objectIndex++;
                }
            }
        }
    }
}

void fn_1_73B8(OMOBJ *object);

s32 lbl_1_data_30C[10] = {
    DATANUM(0x9c0000, 64), DATANUM(0x9c0000, 65),
    DATANUM(0x9c0000, 66), DATANUM(0x9c0000, 67),
    DATANUM(0x9c0000, 68), DATANUM(0x9c0000, 69),
    DATANUM(0x9c0000, 70), DATANUM(0x9c0000, 71),
    DATANUM(0x9c0000, 72), DATANUM(0x9c0000, 73)
};

/* Creates the result models for all players and prepares their wave-deformation data. */
void fn_1_6C48(void) {
    s32 playerIndex;
    s32 modelIndex;
    s32 hasDuplicate;

    for (modelIndex = 0; modelIndex < 10; modelIndex++) {
        lbl_1_bss_2AC[modelIndex].active = 0;
        lbl_1_bss_2AC[modelIndex].elapsed = 0.0f;
        lbl_1_bss_2AC[modelIndex].duration = 30.0f;
        lbl_1_bss_2AC[modelIndex].vertices = NULL;
    }

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_34C[playerIndex] = 64;
        lbl_1_bss_354[playerIndex] = fn_8003AB38(lbl_1_bss_674, 5376,
            11, 0, -1,
            fn_1_73B8);
        lbl_1_bss_354[playerIndex]->work[0] = playerIndex;
        lbl_1_bss_354[playerIndex]->work[1] = 0;
        lbl_1_bss_354[playerIndex]->work[2] = frandmod(9) + 1;
        if (lbl_1_bss_354[playerIndex]->work[2] >= 10) {
            lbl_1_bss_354[playerIndex]->work[2] = 1;
        }
        lbl_1_bss_354[playerIndex]->data = NULL;
        lbl_1_bss_354[playerIndex]->stat |= (1 << 8);

        for (modelIndex = 0; modelIndex < 10; modelIndex++) {
            lbl_1_bss_354[playerIndex]->mdlId[modelIndex] =
                fn_80025A04(
                fn_80008DD0(lbl_1_data_30C[modelIndex], 0x10000000,
                    2));
            fn_80026B64(
                lbl_1_bss_354[playerIndex]->mdlId[modelIndex], 1);
            fn_80026B8C(
                lbl_1_bss_354[playerIndex]->mdlId[modelIndex], 0);
            fn_800269E8(
                lbl_1_bss_354[playerIndex]->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
            fn_800269E8(
                lbl_1_bss_354[playerIndex]->mdlId[modelIndex], (1 << 0));
        }

        lbl_1_bss_354[playerIndex]->mdlId[10] = fn_80025A04(
            fn_80008DD0(DATANUM(0x9c0000, 63), 0x10000000, 2));
        fn_80026B64(lbl_1_bss_354[playerIndex]->mdlId[10], 1);
        fn_80026B8C(lbl_1_bss_354[playerIndex]->mdlId[10], 0);
        fn_800269E8(lbl_1_bss_354[playerIndex]->mdlId[10], ((1 << 30) | (1 << 0)));
        fn_800269E8(lbl_1_bss_354[playerIndex]->mdlId[10], (1 << 0));
        Hu3DLLightCreate(lbl_1_bss_354[playerIndex]->mdlId[10], 0.0f, 10.0f, 5000.0f,
            0.0f, 0.0f, -1.0f, 255, 255, 255);
    }

    while (hasDuplicate != 0) {
        hasDuplicate = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            for (modelIndex = playerIndex + 1;
                 modelIndex < 4; modelIndex++) {
                if (lbl_1_bss_354[playerIndex]->work[2] ==
                    lbl_1_bss_354[modelIndex]->work[2]) {
                    lbl_1_bss_354[modelIndex]->work[2]++;
                    if (lbl_1_bss_354[modelIndex]->work[2] >= 10) {
                        lbl_1_bss_354[modelIndex]->work[2] =
                            lbl_1_bss_354[modelIndex]->work[2] - 10;
                    }
                    if (lbl_1_bss_354[modelIndex]->work[2] == 0) {
                        lbl_1_bss_354[modelIndex]->work[2] = 1;
                    }
                    hasDuplicate = 1;
                }
            }
        }
    }
    fn_1_653C(lbl_1_bss_354[0]);
}

/* Moves a player's display model to its result-board position and pans its cue by distance. */
void fn_1_71AC(s16 playerIndex, Point3d *targetPosition) {
    Point3d modelPosition;
    f32 normalizedDistance;

    lbl_1_bss_354[playerIndex]->work[1] = 1;
    fn_80026790(lbl_1_bss_354[playerIndex]->mdlId[10], targetPosition);
    fn_800267CC(lbl_1_bss_3DC->mdlId[1], &modelPosition);
    normalizedDistance =
        500.0f + (modelPosition.x - targetPosition->x);
    normalizedDistance /= 1000.0f;
    lbl_1_bss_34C[playerIndex] =
        (s16)(32.0f + 64.0f * normalizedDistance);
    HuAudFXPlayPan(1007, lbl_1_bss_34C[playerIndex]);
}

/* Returns the player's display-model position and the fixed offset used by its caller. */
void fn_1_72E4(s16 playerIndex, Point3d *position, f32 *verticalOffset) {
    fn_800267CC(lbl_1_bss_354[playerIndex]->mdlId[10], position);
    *verticalOffset = 40.0f;
}

/* Places a player's result model in animation state 3. */
void fn_1_734C(s16 playerIndex) {
    lbl_1_bss_354[playerIndex]->work[1] = 3;
}

/* Places a player's result model in animation state 7. */
void fn_1_7370(s16 playerIndex) {
    lbl_1_bss_354[playerIndex]->work[1] = 7;
}

/* Returns the saved ordering value for a player's result model. */
s16 fn_1_7394(s16 playerIndex) {
    return (s16) lbl_1_bss_354[playerIndex]->work[2];
}

extern s32 lbl_1_data_8EC[4];

/* Callback from fn_1_6C48 that advances each player's result marker and attached model per
 * frame. */
void fn_1_73B8(OMOBJ *obj) {
    f32 scaleX;
    f32 scaleY;
    Point3d position;

    switch (obj->work[1]) {
    case 1:
        fn_80026A44(obj->mdlId[10], (1 << 0));
        fn_800268B0(obj->mdlId[10], 1.0f,
                          1.0f, 1.0f);
        obj->work[1] = 2;
        break;
    case 2:
        obj->work[3] = 0;
        if (lbl_1_data_8EC[obj->work[0]] < 0) {
            lbl_1_data_8EC[obj->work[0]] = HuAudFXPlayPan(
                1005, lbl_1_bss_34C[obj->work[0]]);
        }
        break;
    case 3:
        if (lbl_1_data_8EC[obj->work[0]] >= 0) {
            HuAudFXStop(lbl_1_data_8EC[obj->work[0]]);
        }
        lbl_1_data_8EC[obj->work[0]] = -1;
        HuAudFXPlayPan(1008, lbl_1_bss_34C[obj->work[0]]);
        obj->work[3]++;
        scaleX = 1.0f -
            0.12f * (f32)obj->work[3];
        scaleY = 1.0f +
            0.1f * (f32)obj->work[3];
        fn_800268B0(obj->mdlId[10], scaleY, scaleX, scaleY);
        if (obj->work[3] > 5) {
            obj->work[3] = 0;
            obj->work[1] = 4;
        }
        break;
    case 4:
        fn_800269E8(obj->mdlId[10], (1 << 0));
        obj->work[3] = 0;
        obj->work[1] = 5;
        fn_800267CC(obj->mdlId[10], &position);
        fn_80026790(obj->mdlId[obj->work[2]], &position);
    case 5:
        obj->work[3]++;
        /* Fall through to the common scale update. */
        fn_80026A44(obj->mdlId[obj->work[2]], (1 << 0));
        lbl_1_bss_2AC[obj->work[2]].active = 1;
        scaleX = 0.2f * (f32)obj->work[3];
        if (scaleX > 1.0f) {
            scaleX = 1.0f;
        }
        fn_800268B0(obj->mdlId[obj->work[2]], scaleX, scaleX, scaleX);
        fn_1_6718(obj);
        if (obj->work[3] > 30) {
            obj->work[3] = 0;
            obj->work[1] = 6;
            fn_800268B0(obj->mdlId[obj->work[2]], 1.0f,
                              1.0f, 1.0f);
            obj->work[3] = 0;
        }
        break;
    case 6:
        fn_800267CC(obj->mdlId[10], &position);
        scaleX = position.y;
        obj->work[3] = 0;
        break;
    case 7:
        obj->work[3]++;
        scaleX = 1.0f -
            0.1f * (f32)obj->work[3];
        fn_800268B0(obj->mdlId[obj->work[2]], scaleX, scaleX, scaleX);
        if (obj->work[3] > 10) {
            obj->work[3] = 0;
            obj->work[1] = 0;
            fn_800269E8(obj->mdlId[10], (1 << 0));
            fn_800269E8(obj->mdlId[obj->work[2]], (1 << 0));
        }
        break;
    }

    fn_8002D414(obj->mdlId[obj->work[2]], 1.0f);
}

void fn_1_7C38(OMOBJ *obj);                         /* static */

HU3D_PARMAN_PARAM lbl_1_data_354 = {
    50, 0, 2.0f, 50.0f, 45.0f, {0.0f, -0.08f, 0.0f},
    2.0f, 1.02f, 20.0f, 0.99f, 2,
    {{255,255,192,255},{255,255,64,255},{0,0,0,0},{0,0,0,0}},
    {{255,128,128,16},{255,64,32,16},{0,0,0,0},{0,0,0,0}}
};

/* Creates the center-stage effect model and configures its particle emitter. */
void fn_1_7870(void) {
    lbl_1_bss_2A8 = fn_8003AB38(lbl_1_bss_674, 5632, 1U, 1U, -1, fn_1_7C38);
    lbl_1_bss_2A8->work[0] = 0;
    lbl_1_bss_2A8->work[1] = 0;
    lbl_1_bss_2A8->work[2] = 0;
    lbl_1_bss_2A8->data = NULL;
    *lbl_1_bss_2A8->mdlId = fn_80025A04(
        fn_80008DD0(DATANUM(0x9c0000, 59), 0x10000000, 2));
    fn_80026B64(*lbl_1_bss_2A8->mdlId, 1U);
    fn_80026B8C(*lbl_1_bss_2A8->mdlId, 0);
    *lbl_1_bss_2A8->mtnId =
        fn_80030458(*lbl_1_bss_2A8->mdlId, fn_80008DD0(DATANUM(0x9c0000, 60),
                                                                    0x10000000, 2));
    fn_8002C95C(*lbl_1_bss_2A8->mdlId, *lbl_1_bss_2A8->mtnId);
    fn_800269E8(*lbl_1_bss_2A8->mdlId, ((1 << 30) | (1 << 0)));
    lbl_1_bss_2A8->stat |= (1 << 8);
    fn_800269E8(*lbl_1_bss_2A8->mdlId, (1 << 0));
    lbl_1_bss_2A4 = fn_800334F4(
        HuSprAnimRead(fn_80008DD0(DATANUM(0x210000, 1), 0x10000000, 2)),
        100, &lbl_1_data_354);
    Hu3DParManAttrSet(lbl_1_bss_2A4, 100);
    fn_8003269C(Hu3DParManModelIDGet(lbl_1_bss_2A4), 1U);
    Hu3DParManRotSet(lbl_1_bss_2A4, 90.0f, 0.0f, 0.0f);
    Hu3DParManAttrSet(lbl_1_bss_2A4, 1);
}

extern s32 lbl_1_data_8E0;

/* Places and starts the center-stage effect after a player presentation begins. */
void fn_1_7AF0(void) {
    Point3d position = {800.0f,150.0f,-800.0f};
    Point3d rotation = {0.0f,270.0f,0.0f};

    PSVECAdd(&position, &lbl_80266A4C, &position);
    fn_80026790(*lbl_1_bss_2A8->mdlId, &position);
    fn_80026838(*lbl_1_bss_2A8->mdlId, &rotation);
    Hu3DParManPosSet(lbl_1_bss_2A4, position.x, position.y, position.z);
    fn_80026A44(*lbl_1_bss_2A8->mdlId, (1 << 0));
    Hu3DParManAttrReset(lbl_1_bss_2A4, 1);
    lbl_1_bss_2A8->work[0] = 1;
    lbl_1_bss_2A8->work[1] = 0;
    lbl_1_bss_2A8->work[2] = 0;
    lbl_1_data_8E0 = HuAudFXPlayVol(1052, 72);
}

/* Per-frame callback that moves the center-stage effect and pans its looping sound. */
void fn_1_7C38(OMOBJ *motion) {
    Point3d position;
    Point3d rotation;
    Point3d offset;
    Point3d rotatedOffset;
    Point3d *rotatedVector;
    s16 pan;

    fn_800267CC(motion->mdlId[0], &position);
    switch ((s32)motion->work[0]) {
    case 0:
        fn_800269E8(motion->mdlId[0], (1 << 0));
        break;
    case 1:
        motion->work[1]++;
        if (motion->work[1] >= MGBATTLE_STAGE_LIGHT_MAX_AGE) {
            motion->work[1] = MGBATTLE_STAGE_LIGHT_MAX_AGE;
        }
        fn_80026A44(motion->mdlId[0], (1 << 0));
        fn_80026874(motion->mdlId[0], &rotation);
        motion->work[2] += 4;
        if (motion->work[2] >= 360) {
            motion->work[2] -= 360;
        }
        position.y += 1.2f *
            sin((3.141592653589793 * (f64)motion->work[2]) / 180.0);
        if (motion->work[1] <= 80) {
            position.x -= 10.0f;
        } else {
            rotation.y += 0.5f;
            if (rotation.y >= 360.0f) {
                rotation.y -= 360.0f;
            }
            offset.x = offset.y = 0.0f;
            offset.z = 5.0f;
            rotatedOffset = offset;
            rotatedVector = &rotatedOffset;
            fn_1_1B0AC(rotation.x, rotation.y, rotation.z, rotatedVector, &offset);
            PSVECAdd(&position, &offset, &position);
        }
        fn_80026838(motion->mdlId[0], &rotation);
        if (rotation.y <= 180.0f) {
            pan = (s16)(48.0f +
                32.0f * (rotation.y / 180.0f));
        } else {
            pan = (s16)(48.0f +
                32.0f *
                    (1.0f -
                        ((rotation.y - 180.0f) / 180.0f)));
        }
        if (lbl_1_data_8E0 >= 0) {
            HuAudFXPanning(lbl_1_data_8E0, pan);
        }
        break;
    default:
        motion->work[0] = 0;
        break;
    }
    fn_80026790(motion->mdlId[0], &position);
    Hu3DParManPosSet(lbl_1_bss_2A4, position.x, position.y, position.z);
}

void fn_1_88D8(OMOBJ *obj);                         /* static */
/* Creates the two stage-light models and their motion controller. */
void fn_1_7F94(void) {
    lbl_1_bss_2A0 = fn_8003AB38(lbl_1_bss_674, 5888, 2U, 1U, -1, fn_1_88D8);
    lbl_1_bss_2A0->work[0] = 0;
    lbl_1_bss_2A0->work[1] = 0;
    lbl_1_bss_2A0->work[2] = 0;
    lbl_1_bss_2A0->data = NULL;
    *lbl_1_bss_2A0->mdlId = fn_80025A04(
        fn_80008DD0(DATANUM(0x9c0000, 0), 0x10000000, 2));
    fn_80026B64(*lbl_1_bss_2A0->mdlId, 1U);
    fn_80026B8C(*lbl_1_bss_2A0->mdlId, 0);
    lbl_1_bss_2A0->mdlId[1] = fn_80025A04(
        fn_80008DD0(DATANUM(0x9c0000, 61), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_2A0->mdlId[1], 1U);
    fn_80026B8C(lbl_1_bss_2A0->mdlId[1], 0);
    fn_800268B0(lbl_1_bss_2A0->mdlId[1], 0.7f, 0.7f, 0.7f);
    lbl_1_bss_2A0->mtnId[1] =
        fn_80030458(lbl_1_bss_2A0->mdlId[1], fn_80008DD0(DATANUM(0x9c0000, 62),
                                                                      0x10000000, 2));
    fn_8002C95C(lbl_1_bss_2A0->mdlId[1], lbl_1_bss_2A0->mtnId[1]);
    lbl_1_bss_2A0->stat |= (1 << 8);
    fn_800269E8(*lbl_1_bss_2A0->mdlId, ((1 << 30) | (1 << 0)));
    fn_800269E8(lbl_1_bss_2A0->mdlId[1], ((1 << 30) | (1 << 0)));
    fn_800269E8(*lbl_1_bss_2A0->mdlId, (1 << 0));
    fn_800269E8(lbl_1_bss_2A0->mdlId[1], (1 << 0));
}

void fn_1_8558(OMOBJ *obj);
void fn_1_9808(void);

/* Starts the stage-light arrival when at least one player has not finished all rounds. */
void fn_1_8244(void) {
    if (MgGameNo >= MgModeScore[0] + 1 ||
        MgGameNo >= MgModeScore[1] + 1 ||
        MgGameNo >= MgModeScore[2] + 1 ||
        MgGameNo >= MgModeScore[3] + 1) {
        lbl_1_bss_2A0->objFunc = fn_1_8558;
        lbl_1_bss_2A0->work[0] = 0;
        lbl_1_bss_2A0->work[1] = 0;
        lbl_1_bss_2A0->work[2] = 0;
        fn_800268B0(lbl_1_bss_2A0->mdlId[1],
                          0.7f,
                          0.7f,
                          0.7f);
        fn_8002C95C(lbl_1_bss_2A0->mdlId[1], lbl_1_bss_2A0->mtnId[1]);
        fn_800269E8(lbl_1_bss_2A0->mdlId[0], ((1 << 30) | (1 << 0)));
        fn_800269E8(lbl_1_bss_2A0->mdlId[1], ((1 << 30) | (1 << 0)));
        fn_800269E8(lbl_1_bss_2A0->mdlId[0], (1 << 0));
        fn_800269E8(lbl_1_bss_2A0->mdlId[1], (1 << 0));
        fn_1_9808();
    }
}

Point3d lbl_1_data_3A4 = {0.0f,25.0f,-100.0f};
Point3d lbl_1_data_3B0 = {-400.0f,100.0f,-700.0f};
Point3d lbl_1_data_3BC = {-400.0f,-1000.0f,-700.0f};

/* Positions the stage lights and starts their movement toward the result-board mark. */
void fn_1_8428(void) {
    Point3d stagePosition;

    fn_800267CC(lbl_1_bss_3DC->mdlId[1], &stagePosition);
    PSVECAdd(&stagePosition, &lbl_1_data_3B0, &lbl_1_data_3B0);
    PSVECAdd(&stagePosition, &lbl_1_data_3BC, &lbl_1_data_3BC);
    fn_80026A44(*lbl_1_bss_2A0->mdlId, (1 << 0));
    fn_80026A44(lbl_1_bss_2A0->mdlId[1], (1 << 0));
    fn_80026790(*lbl_1_bss_2A0->mdlId, &lbl_1_data_3BC);
    PSVECAdd(&lbl_1_data_3BC, &lbl_1_data_3A4, &stagePosition);
    fn_80026790(lbl_1_bss_2A0->mdlId[1], &stagePosition);
    lbl_1_bss_2A0->work[0] = 1;
    lbl_1_bss_2A0->work[1] = 0;
    lbl_1_bss_2A0->work[2] = 0;
}

void fn_1_9E10(Point3d position);
void fn_1_A0C0(Point3d position);

/* Per-frame callback that brings the lights to the board and then animates them in place. */
void fn_1_8558(OMOBJ *motion) {
    Point3d position;
    Point3d direction;

    fn_800267CC(motion->mdlId[0], &position);
    switch ((s32)motion->work[0]) {
    case 0:
        fn_800269E8(motion->mdlId[0], (1 << 0));
        fn_800269E8(motion->mdlId[1], (1 << 0));
        break;
    case 1:
        fn_80026A44(motion->mdlId[0], (1 << 0));
        fn_80026A44(motion->mdlId[1], (1 << 0));
        PSVECSubtract(&lbl_1_data_3B0, &position, &direction);
        if (fn_1_1AF5C(&direction, &direction) != 0) {
            PSVECScale(&direction, &direction, 5.0f);
        } else {
            direction.x = direction.y = direction.z = 0.0f;
        }
        PSVECAdd(&position, &direction, &position);
        PSVECSubtract(&lbl_1_data_3B0, &position, &direction);
        if (PSVECSquareMag(&direction) <= 25.0f) {
            motion->work[0] = 2;
            motion->work[1] = 0;
            motion->work[2] = 0;
            direction = position;
            direction.y += 100.0f;
            fn_1_9E10(direction);
        }
        break;
    case 2:
        fn_80026A44(motion->mdlId[0], (1 << 0));
        fn_80026A44(motion->mdlId[1], (1 << 0));
        motion->work[1] += 3;
        if (motion->work[1] >= 360) {
            motion->work[1] -= 360;
        }
        position.y += sin((3.141592653589793 * motion->work[1]) /
            180.0);
        motion->work[2]++;
        if (motion->work[2] >= 360) {
            motion->work[2] -= 360;
        }
        position.x += 7.0 *
            sin((3.141592653589793 * motion->work[2]) /
                180.0);
        direction = position;
        direction.y += 100.0f;
        fn_1_A0C0(direction);
        break;
    default:
        motion->work[0] = 0;
        break;
    }
    fn_80026790(motion->mdlId[0], &position);
    PSVECAdd(&position, &lbl_1_data_3A4, &position);
    fn_80026790(motion->mdlId[1], &position);
}

/* Per-frame callback that sends scaled stage effects across the background. */
void fn_1_88D8(OMOBJ *work) {
    s16 randomState;
    Point3d position;
    Point3d scale;
    Point3d offset;

    fn_8002691C(work->mdlId[0], &scale);
    fn_800267CC(work->mdlId[0], &position);

    switch ((s32)work->work[0]) {
    case 0:
        fn_800269E8(work->mdlId[0], (1 << 0));
        fn_800269E8(work->mdlId[1], (1 << 0));
        randomState = frandmod(8);
        randomState += 4;
        work->work[0] = randomState;
        work->work[1] = 0;
        work->work[2] = 0;
        break;
    case 4:
        fn_80026A44(work->mdlId[0], (1 << 0));
        fn_80026A44(work->mdlId[1], (1 << 0));
        scale.x = scale.y = scale.z = 0.4f;
        if (work->work[2] == 0) {
            position.x = -1500.0f;
            position.y = 0.0f;
            position.z = -1000.0f;
        }
        position.x += 5.0f * scale.x;
        if (position.x >= 4500.0f) {
            work->work[0] = 0;
        }
        work->work[1] += 3;
        if (work->work[1] >= 360) {
            work->work[1] -= 360;
        }
        position.y += scale.x *
            sin((3.141592653589793 * work->work[1]) / 180.0);
        work->work[2]++;
        if (work->work[2] >= MGBATTLE_PARTICLE_MAX_AGE) {
            work->work[2] = MGBATTLE_PARTICLE_MAX_AGE;
        }
        break;
    case 5:
        fn_80026A44(work->mdlId[0], (1 << 0));
        fn_80026A44(work->mdlId[1], (1 << 0));
        if (work->work[2] == 0) {
            scale.x = scale.y = scale.z = 0.6f;
            position.x = -1500.0f;
            position.y = 0.0f;
            position.z = -1000.0f;
        }
        scale.x -= 0.001f * scale.x;
        if (scale.x <= 0.1f) {
            scale.x = 0.1f;
        }
        scale.y = scale.z = scale.x;
        position.x = position.x + (5.0f * scale.x);
        if (position.x >= 4500.0f) {
            work->work[0] = 0;
        }
        work->work[1] += 3;
        if (work->work[1] >= 360) {
            work->work[1] -= 360;
        }
        position.y = (f32)((f64)position.y +
            ((f64)scale.x * sin((3.141592653589793 * work->work[1]) / 180.0)));
        work->work[2]++;
        if (work->work[2] >= MGBATTLE_PARTICLE_MAX_AGE) {
            work->work[2] = MGBATTLE_PARTICLE_MAX_AGE;
        }
        break;
    case 6:
        fn_80026A44(work->mdlId[0], (1 << 0));
        fn_80026A44(work->mdlId[1], (1 << 0));
        if (work->work[2] == 0) {
            scale.x = scale.y = scale.z = 0.1f;
            position.x = -1500.0f;
            position.y = 0.0f;
            position.z = -1000.0f;
        }
        scale.x += 0.001f * scale.x;
        if (scale.x >= 0.6f) {
            scale.x = 0.6f;
        }
        scale.y = scale.z = scale.x;
        position.x = position.x + (5.0f * scale.x);
        if (position.x >= 4500.0f) {
            work->work[0] = 0;
        }
        work->work[1] += 3;
        if (work->work[1] >= 360) {
            work->work[1] -= 360;
        }
        position.y = (f32)((f64)position.y +
            ((f64)scale.x * sin((3.141592653589793 * work->work[1]) / 180.0)));
        work->work[2]++;
        if (work->work[2] >= MGBATTLE_PARTICLE_MAX_AGE) {
            work->work[2] = MGBATTLE_PARTICLE_MAX_AGE;
        }
        break;
    case 7:
        fn_80026A44(work->mdlId[0], (1 << 0));
        fn_80026A44(work->mdlId[1], (1 << 0));
        if (work->work[2] == 0) {
            scale.x = scale.y = scale.z = 0.6f;
            position.x = -1500.0f;
            position.y = -1000.0f;
            position.z = -1000.0f;
        }
        scale.x -= 0.001f * scale.x;
        if (scale.x <= 0.2f) {
            scale.x = 0.2f;
        }
        scale.y = scale.z = scale.x;
        position.x = position.x + (5.0f * scale.x);
        position.y += scale.x;
        if (position.x >= 4500.0f || position.y >= 1000.0f) {
            work->work[0] = 0;
        }
        work->work[1] += 3;
        if (work->work[1] >= 360) {
            work->work[1] -= 360;
        }
        position.y += (f64)scale.x *
            sin((3.141592653589793 * work->work[1]) / 180.0);
        work->work[2]++;
        if (work->work[2] >= MGBATTLE_PARTICLE_MAX_AGE) {
            work->work[2] = MGBATTLE_PARTICLE_MAX_AGE;
        }
        break;
    case 8:
        fn_80026A44(work->mdlId[0], (1 << 0));
        fn_80026A44(work->mdlId[1], (1 << 0));
        if (work->work[2] == 0) {
            scale.x = scale.y = scale.z = 0.2f;
            position.x = -1500.0f;
            position.y = 1000.0f;
            position.z = -1000.0f;
        }
        scale.x += 0.001f * scale.x;
        if (scale.x >= 0.4f) {
            scale.x = 0.4f;
        }
        scale.y = scale.z = scale.x;
        position.x = position.x + (5.0f * scale.x);
        position.y -= scale.x;
        if (position.x >= 4500.0f || position.y <= -1500.0f) {
            work->work[0] = 0;
        }
        work->work[1] += 3;
        if (work->work[1] >= 360) {
            work->work[1] -= 360;
        }
        position.y += (f64)scale.x *
            sin((3.141592653589793 * work->work[1]) / 180.0);
        work->work[2]++;
        if (work->work[2] >= MGBATTLE_PARTICLE_MAX_AGE) {
            work->work[2] = MGBATTLE_PARTICLE_MAX_AGE;
        }
        break;
    case 9:
        fn_80026A44(work->mdlId[0], (1 << 0));
        fn_80026A44(work->mdlId[1], (1 << 0));
        scale.x = scale.y = scale.z = 0.4f;
        if (work->work[2] == 0) {
            position.x = 4500.0f;
            position.y = 0.0f;
            position.z = -1000.0f;
        }
        position.x = position.x - (5.0f * scale.x);
        if (position.x <= -1500.0f) {
            work->work[0] = 0;
        }
        work->work[1] += 3;
        if (work->work[1] >= 360) {
            work->work[1] -= 360;
        }
        position.y = (f32)((f64)position.y +
            ((f64)scale.x * sin((3.141592653589793 * work->work[1]) / 180.0)));
        work->work[2]++;
        if (work->work[2] >= MGBATTLE_PARTICLE_MAX_AGE) {
            work->work[2] = MGBATTLE_PARTICLE_MAX_AGE;
        }
        break;
    case 10:
        fn_80026A44(work->mdlId[0], (1 << 0));
        fn_80026A44(work->mdlId[1], (1 << 0));
        if (work->work[2] == 0) {
            scale.x = scale.y = scale.z = 0.6f;
            position.x = 4500.0f;
            position.y = 0.0f;
            position.z = -1000.0f;
        }
        scale.x -= 0.001f * scale.x;
        if (scale.x <= 0.1f) {
            scale.x = 0.1f;
        }
        scale.y = scale.z = scale.x;
        position.x = position.x - (5.0f * scale.x);
        if (position.x <= -1500.0f) {
            work->work[0] = 0;
        }
        work->work[1] += 3;
        if (work->work[1] >= 360) {
            work->work[1] -= 360;
        }
        position.y = (f32)((f64)position.y +
            ((f64)scale.x * sin((3.141592653589793 * work->work[1]) / 180.0)));
        work->work[2]++;
        if (work->work[2] >= MGBATTLE_PARTICLE_MAX_AGE) {
            work->work[2] = MGBATTLE_PARTICLE_MAX_AGE;
        }
        break;
    case 11:
        fn_80026A44(work->mdlId[0], (1 << 0));
        fn_80026A44(work->mdlId[1], (1 << 0));
        work->work[0] = 0;
        if (work->work[2] == 0) {
            scale.x = scale.y = scale.z = 0.1f;
            position.x = 4500.0f;
            position.y = 0.0f;
            position.z = -1000.0f;
        }
        scale.x += 0.001f * scale.x;
        if (scale.x >= 0.6f) {
            scale.x = 0.6f;
        }
        scale.y = scale.z = scale.x;
        position.x = position.x - (5.0f * scale.x);
        if (position.x <= -1500.0f) {
            work->work[0] = 0;
        }
        work->work[1] += 3;
        if (work->work[1] >= 360) {
            work->work[1] -= 360;
        }
        position.y = (f32)((f64)position.y +
            ((f64)scale.x * sin((3.141592653589793 * work->work[1]) / 180.0)));
        work->work[2]++;
        if (work->work[2] >= MGBATTLE_PARTICLE_MAX_AGE) {
            work->work[2] = MGBATTLE_PARTICLE_MAX_AGE;
        }
        break;
    default:
        work->work[0] = 0;
        break;
    }

    fn_800268E0(work->mdlId[0], &scale);
    fn_80026790(work->mdlId[0], &position);
    offset = lbl_1_data_3A4;
    PSVECScale(&offset, &offset, scale.x);
    PSVECAdd(&position, &offset, &position);
    PSVECScale(&scale, &scale, 0.7f);
    fn_80026790(work->mdlId[1], &position);
    fn_800268E0(work->mdlId[1], &scale);
}

void fn_1_A0E4(OMOBJ *object);

/* Allocates the reusable particle models and initializes the stage-effect pool. */
void fn_1_9808(void) {
    s16 modelIndex;
    s16 effectIndex;

    lbl_1_bss_290 = fn_8003AB38(lbl_1_bss_674, 6144, 32, 0, -1,
        fn_1_A0E4);
    lbl_1_bss_290->work[0] = 0;
    lbl_1_bss_290->data = HuMemDirectMallocNum(HEAP_HEAP, 640,
        0x10000000);
    memset(lbl_1_bss_290->data, 0, 640);

    lbl_1_bss_290->mdlId[0] =
        fn_80025A04(fn_80008DD0(DATANUM(0x9c0000, 1), 0x10000000,
            2));
    lbl_1_bss_290->mdlId[1] =
        fn_80025A04(fn_80008DD0(DATANUM(0x9c0000, 2), 0x10000000,
            2));
    lbl_1_bss_290->mdlId[2] =
        fn_80025A04(fn_80008DD0(DATANUM(0x9c0000, 3), 0x10000000,
            2));
    lbl_1_bss_290->mdlId[3] =
        fn_80025A04(fn_80008DD0(DATANUM(0x9c0000, 4), 0x10000000,
            2));

    fn_80026B64(lbl_1_bss_290->mdlId[0], 1);
    fn_80026B64(lbl_1_bss_290->mdlId[1], 1);
    fn_80026B64(lbl_1_bss_290->mdlId[2], 1);
    fn_80026B64(lbl_1_bss_290->mdlId[3], 1);
    fn_80026B8C(lbl_1_bss_290->mdlId[0], 0);
    fn_80026B8C(lbl_1_bss_290->mdlId[1], 0);
    fn_80026B8C(lbl_1_bss_290->mdlId[2], 0);
    fn_80026B8C(lbl_1_bss_290->mdlId[3], 0);
    fn_800268B0(lbl_1_bss_290->mdlId[0], 0.0f, 0.0f, 0.0f);
    fn_800268B0(lbl_1_bss_290->mdlId[1], 0.0f, 0.0f, 0.0f);
    fn_800268B0(lbl_1_bss_290->mdlId[2], 0.0f, 0.0f, 0.0f);
    fn_800268B0(lbl_1_bss_290->mdlId[3], 0.0f, 0.0f, 0.0f);

    for (modelIndex = 0; modelIndex < 28; modelIndex++) {
        effectIndex = modelIndex / 4;
        lbl_1_bss_290->mdlId[modelIndex + 4] =
            fn_80025D4C(lbl_1_bss_290->mdlId[effectIndex]);
        fn_80026B64(lbl_1_bss_290->mdlId[modelIndex + 4], 1);
        fn_80026B8C(lbl_1_bss_290->mdlId[modelIndex + 4], 0);
        fn_800268B0(lbl_1_bss_290->mdlId[modelIndex + 4],
            0.0f, 0.0f, 0.0f);
    }

    lbl_1_bss_2A0->stat |= (1 << 8);
    lbl_1_bss_294.x = lbl_1_bss_294.y = lbl_1_bss_294.z = 0.0f;

    for (modelIndex = 0; modelIndex < 20; modelIndex++) {
        effectIndex = modelIndex % 4;
        lbl_1_bss_240[modelIndex] = fn_80025D4C(
            lbl_1_bss_290->mdlId[effectIndex]);
        fn_80026B64(lbl_1_bss_240[modelIndex], 1);
        fn_80026B8C(lbl_1_bss_240[modelIndex], 0);
        fn_800268B0(lbl_1_bss_240[modelIndex], 0.0f, 0.0f, 0.0f);
        lbl_1_bss_268[modelIndex] = 0;
        lbl_1_bss_1F0[modelIndex] = 0.0f;
        lbl_1_bss_100[modelIndex].x = lbl_1_bss_100[modelIndex].y =
            lbl_1_bss_100[modelIndex].z = 0.0f;
    }
}

/* Motion state for one model in the stage particle pool. */
typedef struct {
    /* Negative values delay emission; nonnegative values count active frames. */
    s16 state;
    /* Velocity along each stage axis, in units per update. */
    f32 horizontalVelocity;
    f32 verticalVelocity;
    f32 depthVelocity;
    /* Rotation about the model's Z axis, in degrees. */
    f32 rotation;
} MgBattleParticle;

/* Starts a particle burst at the supplied stage position and resets its particles. */
void fn_1_9E10(Point3d position) {
    s16 index;
    Point3d scale;

    lbl_1_bss_290->work[0] = 1;
    lbl_1_bss_294 = position;

    for (index = 0; index < 32; index++) {
        s16 *particleScale;

        particleScale = &(((MgBattleParticle *)lbl_1_bss_290->data + index)->state);
        scale.x = scale.y = scale.z = 0.0f;
        *particleScale = frandmod(20) + 1;
        *particleScale = (s16)(-12.5f * (f32)*particleScale);
        fn_800268E0(lbl_1_bss_290->mdlId[index], &scale);
        fn_80026790(lbl_1_bss_290->mdlId[index], &position);
        fn_80026838(lbl_1_bss_290->mdlId[index], &lbl_80266A40);
    }

    for (index = 0; index < 20; index++) {
        scale.x = scale.y = scale.z = 0.0f;
        lbl_1_bss_268[index] = frandmod(20) + 1;
        lbl_1_bss_268[index] =
            (s16)(-12.5f * (f32)lbl_1_bss_268[index]);
        fn_800268E0(lbl_1_bss_240[index], &scale);
        fn_80026790(lbl_1_bss_240[index], &position);
        fn_80026838(lbl_1_bss_240[index], &lbl_80266A40);
    }
}

/* Sets the origin used when the stage particle effect next emits particles. */
void fn_1_A0C0(Point3d source) {
    lbl_1_bss_294 = source;
}

/* Per-frame callback that animates the stage particle bursts and their model transforms. */
void fn_1_A0E4(OMOBJ *object) {
    Mtx rotation;
    Point3d scale;
    Point3d position;
    s16 index;
    MgBattleParticle * particle;
    switch ((s32)object->work[0]) {
    case 0:
        break;
    case 1:
    index = 0;
    while (index < 32)
    {
        particle = (MgBattleParticle *)object->data + index;
        fn_8002691C(object->mdlId[index], & scale);
        fn_800267CC(object->mdlId[index], & position);
        particle->rotation += 12.0f;
        if (particle->rotation >= 360.0f)
        {
            particle->rotation -= 360.0f;
        }
        PSMTXRotRad(rotation, 'z', 0.017453292f * particle->rotation);
        fn_80026958(object->mdlId[index], & rotation);
        particle->state++;
        if (particle->state == 0)
        {
            scale.x = scale.y = scale.z = 0.1f;
            position = lbl_1_bss_294;
            particle->horizontalVelocity = 0.06f * ((f32)(u32) frandmod(100) - 50.0f);
            particle->verticalVelocity = 10.0f + (f32)(u32) frandmod(6);
            particle->depthVelocity = 1.0f;
        }
        else
        {
            if (particle->state > 0)
            {
                scale.x = scale.y = scale.z += 0.1f;
                if (scale.x >= 1.0f)
                {
                    scale.x = scale.y = scale.z = 1.0f;
                }
                particle->horizontalVelocity -= 0.015f * particle->horizontalVelocity;
                particle->verticalVelocity -= 1.0f;
                if (particle->verticalVelocity <= - 4.0f)
                {
                    particle->verticalVelocity = - 4.0f;
                }
                position.x += particle->horizontalVelocity;
                position.y += particle->verticalVelocity;
                position.z += particle->depthVelocity;
                if (particle->state >= 250)
                {
                    scale.x = scale.y = scale.z = 0.0f;
                    particle->state = frandmod(20) + 1;
                    particle->state = (s16)(- 12.5f * (f32) particle->state);
                    position = lbl_1_bss_294;
                }
            }
        }
        fn_800268E0(object->mdlId[index], & scale);
        fn_80026790(object->mdlId[index], & position);
        fn_80026808(object->mdlId[index], lbl_80266A40.x, lbl_80266A40.y, lbl_80266A40.z);
        index++;
    }
    index = 0;
    while (index < 20)
    {
        fn_8002691C(lbl_1_bss_240[index], & scale);
        fn_800267CC(lbl_1_bss_240[index], & position);
        lbl_1_bss_1F0[index] += 12.0f;
        if (lbl_1_bss_1F0[index] >= 360.0f)
        {
            lbl_1_bss_1F0[index] -= 360.0f;
        }
        PSMTXRotRad(rotation, 'z', 0.017453292f * lbl_1_bss_1F0[index]);
        fn_80026958(lbl_1_bss_240[index], & rotation);
        lbl_1_bss_268[index]++;
        if (lbl_1_bss_268[index] == 0)
        {
            scale.x = scale.y = scale.z = 0.1f;
            fn_800267CC(lbl_1_bss_3DC->mdlId[1], & position);
            position.x += 10.0f * ((f32)(u32) frandmod(100) - 50.0f);
            position.y += 900.0f;
            position.z += 800.0f;
            lbl_1_bss_100[index].x = 0.03f * ((f32)(u32) frandmod(100) - 50.0f);
            lbl_1_bss_100[index].y = 0.0f;
            lbl_1_bss_100[index].z = 0.0f;
        }
        else
        {
            if (lbl_1_bss_268[index] > 0)
            {
            scale.x = scale.y = scale.z += 0.1f;
                if (scale.x >= 1.0f)
                {
                    scale.x = scale.y = scale.z = 1.0f;
                }
                lbl_1_bss_100[index].x = lbl_1_bss_100[index].x - (0.015f * lbl_1_bss_100[index].x);
                lbl_1_bss_100[index].y -= 1.0f;
                if (lbl_1_bss_100[index].y <= - 4.5f)
                {
                    lbl_1_bss_100[index].y = - 4.5f;
                }
                position.x += lbl_1_bss_100[index].x;
                position.y += lbl_1_bss_100[index].y;
                position.z += lbl_1_bss_100[index].z;
                if (lbl_1_bss_268[index] >= 250)
                {
                    scale.x = scale.y = scale.z = 0.0f;
                    lbl_1_bss_268[index] = frandmod(20) + 1;
                    lbl_1_bss_268[index] = (s16)(- 12.5f * (f32) lbl_1_bss_268[index]);
                    position = lbl_1_bss_294;
                }
            }
        }
        fn_800268E0(lbl_1_bss_240[index], & scale);
        fn_80026790(lbl_1_bss_240[index], & position);
        fn_80026808(lbl_1_bss_240[index], lbl_80266A40.x, lbl_80266A40.y, lbl_80266A40.z);
        index++;
    }
        break;
    case 2:
        break;
    }
}
