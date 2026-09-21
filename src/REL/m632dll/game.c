#define _MATH_H
#include "REL/m632dll.h"

static int lbl_1_data_78[5] = { 5963776, 5963778, 5963778, 5963779, 5963780 };

static int lbl_1_data_8C[5] = { 0, 0, 1, 0, 1 };

static int lbl_1_data_A0[4] = { 5963797, 5963798, 5963799, 5963800 };

static int lbl_1_data_B0[3] = { 5963781, 5963783, 5963784 };

static char lbl_1_data_BC[19] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    45,
    80,
    67,
    48,
    49,
    115,
    116,
    0,
};

static char lbl_1_data_CF[19] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    45,
    80,
    67,
    48,
    50,
    115,
    116,
    0,
};

static char lbl_1_data_E2[19] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    45,
    80,
    67,
    48,
    51,
    115,
    116,
    0,
};

static char lbl_1_data_F5[19] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    45,
    80,
    67,
    48,
    52,
    115,
    116,
    0,
};

char *lbl_1_data_108[4] = { lbl_1_data_BC, lbl_1_data_CF, lbl_1_data_E2, lbl_1_data_F5 };

unsigned int lbl_1_data_118[4] = { 5963786, 5963787, 5963788, 5963789 };

/* Retail .data[0x128,0x158): unreferenced initialized storage.
 * The words resemble coordinates, but no consumer authenticates a record type. */
u8 lbl_1_data_128[48] = {
    195, 200, 0, 0,
    68, 150, 0, 0,
    0, 0, 0, 0,
    67, 200, 0, 0,
    68, 150, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    68, 150, 0, 0,
    195, 200, 0, 0,
    0, 0, 0, 0,
    68, 150, 0, 0,
    67, 200, 0, 0,
};

unsigned int lbl_1_data_158[16] = {
    9633792,
    9633793,
    9633794,
    9633795,
    9633796,
    9633827,
    9633826,
    9633816,
    9633798,
    9633832,
    9633816,
    9633813,
    9633824,
    9306156,
    9306312,
    0,
};

s32 lbl_1_data_198[4] = { 128, 80, 40, 32 };

static u8 lbl_1_data_1A8[64] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    6,
    0,
    0,
    0,
    2,
    0,
    0,
    3,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    3,
    0,
    0,
    2,
    0,
    0,
    0,
    4,
    0,
    0,
    0,
    0,
    5,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static u8 lbl_1_data_1E8[64] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    3,
    0,
    0,
    6,
    0,
    2,
    0,
    0,
    0,
    5,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    4,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    0,
    1,
    0,
    0,
    3,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static u8 lbl_1_data_228[64] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    3,
    0,
    0,
    2,
    0,
    0,
    0,
    4,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    6,
    0,
    0,
    0,
    2,
    0,
    0,
    3,
    0,
    0,
    0,
    1,
    0,
    0,
    5,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static u8 lbl_1_data_268[64] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,
    0,
    5,
    0,
    0,
    3,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    4,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    6,
    0,
    0,
    0,
    3,
    1,
    0,
    0,
    0,
    2,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

u8 *lbl_1_data_2A8[4] = { lbl_1_data_1A8, lbl_1_data_1E8, lbl_1_data_228, lbl_1_data_268 };

static char lbl_1_data_2B8[21] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    72,
    79,
    79,
    75,
    49,
    0,
};

static char lbl_1_data_2CD[21] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    72,
    79,
    79,
    75,
    50,
    0,
};

static char lbl_1_data_2E2[21] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    72,
    79,
    79,
    75,
    51,
    0,
};

static char lbl_1_data_2F7[21] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    72,
    79,
    79,
    75,
    52,
    0,
};

static char lbl_1_data_30C[21] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    72,
    79,
    79,
    75,
    53,
    0,
};

static char lbl_1_data_321[21] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    72,
    79,
    79,
    75,
    54,
    0,
};

static char lbl_1_data_336[21] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    72,
    79,
    79,
    75,
    55,
    0,
};

static char lbl_1_data_34B[21] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    72,
    79,
    79,
    75,
    56,
    0,
};

static char lbl_1_data_360[24] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    67,
    79,
    76,
    72,
    79,
    79,
    75,
    49,
    0,
};

static char lbl_1_data_378[24] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    67,
    79,
    76,
    72,
    79,
    79,
    75,
    50,
    0,
};

static char lbl_1_data_390[24] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    67,
    79,
    76,
    72,
    79,
    79,
    75,
    51,
    0,
};

static char lbl_1_data_3A8[24] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    67,
    79,
    76,
    72,
    79,
    79,
    75,
    52,
    0,
};

static char lbl_1_data_3C0[24] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    67,
    79,
    76,
    72,
    79,
    79,
    75,
    53,
    0,
};

static char lbl_1_data_3D8[24] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    67,
    79,
    76,
    72,
    79,
    79,
    75,
    54,
    0,
};

static char lbl_1_data_3F0[24] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    67,
    79,
    76,
    72,
    79,
    79,
    75,
    55,
    0,
};

static char lbl_1_data_408[24] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    66,
    97,
    67,
    79,
    76,
    72,
    79,
    79,
    75,
    56,
    0,
};

static char lbl_1_data_420[23] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    65,
    45,
    115,
    116,
    97,
    103,
    101,
    65,
    115,
    104,
    97,
    0,
};

static char lbl_1_data_437[26] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    65,
    45,
    115,
    116,
    97,
    103,
    101,
    65,
    115,
    104,
    97,
    65,
    68,
    68,
    0,
};

static char lbl_1_data_451[23] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    65,
    45,
    115,
    116,
    97,
    103,
    101,
    67,
    115,
    104,
    97,
    0,
};

static char lbl_1_data_468[24] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    115,
    116,
    97,
    103,
    101,
    66,
    97,
    83,
    104,
    97,
    0,
};

static char lbl_1_data_480[21] = {
    54,
    51,
    50,
    104,
    97,
    107,
    111,
    110,
    105,
    119,
    97,
    66,
    45,
    115,
    116,
    97,
    103,
    101,
    66,
    98,
    0,
};

M632State lbl_1_bss_0;

void fn_1_3510(MGACTOR *actorP, int param)
{
    if ((u32) (actorP->colGroundAttr & 128) != 0) {
        lbl_1_bss_0.collisionPairs[param][1] = 1;
    }
}

int fn_1_353C(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    MGACTOR_COLMAP_POLY sp68;
    Point3d sp5C;
    Point3d sp50;
    Point3d sp44;
    Point3d sp38;
    Point3d sp2C;
    Point3d sp20;
    Point3d sp14;
    Point3d sp8;
    f32 temp_f31;
    s32 temp_r26;
    s32 pan;
    MGPLAYER *player;
    MGACTOR *actor;

    if ((a->type == 1) && (b->type == 1)) {
        PSVECSubtract(&b->point, &a->point, &sp5C);
        PSVECNormalize(&sp5C, &sp5C);
        PSVECNormalize(&a->normPos, &sp50);
        if (PSVECDotProduct(&sp5C, &sp50) > 0.1f) {
            temp_f31 = a->normPos.y;
            PSVECScale(&a->normPos, &a->normPos, 0.01f);
            a->normPos.y = 0.05f * temp_f31;
        }
        lbl_1_bss_0.field3FC = 1;
        goto block_20;
    }
    if ((a->type == 1) && (b->type == 0)) {
        if (MgSeqModeGet() != 5) {
            return 1;
        }
        actor = lbl_1_bss_0.collisionActors[a->paramB];
        player = lbl_1_bss_0.players[b->paramB];
        if ((lbl_1_bss_0.activeMask & (1 << b->paramB)) != 0) {
            sp44 = actor->pos;
            sp38 = player->actor->pos;
            sp44.y = sp38.y = 0.0f;
            PSVECSubtract(&sp38, &sp44, &sp2C);
            PSVECNormalize(&sp2C, &sp2C);
            sp44.x = sp38.x + (2000.0f * sp2C.x);
            sp44.z = sp38.z + (2000.0f * sp2C.z);
            sp44.y = sp38.y = 600.0f;
            if (MgActorColMapPolyGet(&sp38, &sp44, 1U, &sp68) != 0) {
                sp20 = sp68.pos;
                if (((HSF_FACE *) sp68.obj->mesh.face->data)[sp68.triNo].nbt[2] < 0.0f) {
                    sp20.y = 600.0f;
                } else {
                    sp20.y = 1200.0f;
                }
            } else {
                sp20.x = 0.0f;
                sp20.y = 400.0f;
                sp20.z = 400.0f;
            }
            sp38.y = player->actor->pos.y;
            PSVECSubtract(&sp20, &sp38, &sp2C);
            PSVECNormalize(&sp2C, &sp2C);
            lbl_1_bss_0.playerMotionVec[b->paramB] = sp2C;
            lbl_1_bss_0.playerState[b->paramB] = 1;
            lbl_1_bss_0.activeMask &= ~(1 << b->paramB);
            temp_r26 = HuAudFXPlay(1847);
            sp14 = player->actor->pos;
            Hu3D3Dto2D(&sp14, 1, &sp8);
            pan = (s32) sp8.x;
            pan /= 5;
            if (pan < 32) {
                pan = 32;
            } else if (pan > 96) {
                pan = 96;
            }
            HuAudFXPanning(temp_r26, pan);
        }
        goto block_20;
    }
block_20:
    return 1;
}

void fn_1_3934(void)
{
    Point3d sp44;
    Point3d sp38;
    Point3d sp2C;
    Point3d sp20;
    Point3d sp14;
    int index;
    int counter;
    s8 *var_r29;
    s8 *var_r28;
    s16 temp_r3;
    int mapCount = 0;
    s8 *var_r25;
    int var_r24;
    s16 temp_r3_3;
    int var_r22;
    s16 jointMotion;
    int x1, z1;
    int x2, z2;
    int sp10, spC;
    s16 spA;
    s16 sp8;
    f32 temp_f31;
    f32 var_f30;

    memset(&lbl_1_bss_0, 0, sizeof(lbl_1_bss_0));
    lbl_1_bss_0.scalar194 = 0.0f;
    lbl_1_bss_0.scalar198 = 0.0f;
    lbl_1_bss_0.objectManager = MgActorObjectSetup();
    lbl_1_bss_0.patternIndex = frand() & 3;
    lbl_1_bss_0.timer = MgTimerCreate(0);
    CRot.x = -35.0f;
    CRot.y = 0.0f;
    CRot.z = 0.0f;
    Center.x = 0.0f;
    Center.y = 700.0f;
    Center.z = 400.0f;
    CZoom = 1000.0f;
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(3, 45.0f, 20.0f, 8000.0f, 1.2f);
    Hu3DCameraViewportSet(3, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(2, 640U, 480U, 0U, 0U);
    index = 0;
    while (index < 5) {
        lbl_1_bss_0.cameraMotions[index] = Hu3DMotionCreate(HuDataSelHeapReadNum(lbl_1_data_78[index], 268435456, HEAP_MODEL));
        if (lbl_1_data_8C[index] == 0) {
            lbl_1_bss_0.cameraModels[index] = Hu3DModelCameraCreate(lbl_1_bss_0.cameraMotions[index], 1U);
        } else {
            lbl_1_bss_0.cameraModels[index] = Hu3DModelCameraCreate(lbl_1_bss_0.cameraMotions[index], 2U);
        }
        Hu3DCameraMotionOff(lbl_1_bss_0.cameraModels[index]);
        index += 1;
    }
    index = 0;
    while (index < 2) {
        lbl_1_bss_0.motions020[index] = -1;
        index += 1;
    }
    Hu3DCameraMotionStart(lbl_1_bss_0.cameraModels[0], 1U);
    sp44.x = 0.0f;
    sp44.y = 4000.0f;
    sp44.z = 0.0f;
    sp38.x = 0.0f;
    sp38.y = 0.0f;
    sp38.z = 1.0f;
    sp2C.x = 0.0f;
    sp2C.y = -10.0f;
    sp2C.z = 0.0f;
    Hu3DShadowMultiCreate(30.0f, 1.0f, 13000.0f, 3);
    Hu3DShadowMultiTPLvlSet(0.7f, 3);
    sp44.y = -1800.0f;
    sp2C.y = 100.0f;
    Hu3DShadowMultiPosSet(&sp44, &sp38, &sp2C, 1);
    Hu3DShadowMultiColSet(25U, 25U, 25U, 1);
    Hu3DShadowMultiSizeSet(240U, 1);
    sp44.y = -2000.0f;
    sp2C.y = 100.0f;
    Hu3DShadowMultiPosSet(&sp44, &sp38, &sp2C, 2);
    Hu3DShadowMultiColSet(50U, 50U, 50U, 2);
    Hu3DShadowMultiSizeSet(192U, 2);
    index = 0;
    while (index < 4) {
        lbl_1_bss_0.models024[index] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_A0[index], 268435456, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_0.models024[index], 1U);
        index += 1;
    }
    lbl_1_bss_0.model02C = Hu3DModelCreate(HuDataSelHeapReadNum(5963796, 268435456, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_0.model02C, 1U);
    index = 0;
    while (index < 3) {
        lbl_1_bss_0.models02E[index] = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_B0[index], 268435456, HEAP_MODEL));
        index += 1;
    }
    jointMotion = Hu3DJointMotion(lbl_1_bss_0.models02E[0], HuDataSelHeapReadNum(5963782, 268435456, HEAP_MODEL));
    Hu3DMotionSet(lbl_1_bss_0.models02E[0], jointMotion);
    (&lbl_1_bss_0.collisionModel)[mapCount] = Hu3DModelCreate(HuDataSelHeapReadNum((int) lbl_1_data_118[lbl_1_bss_0.patternIndex], 268435456, HEAP_MODEL));
    Hu3DModelAttrSet((&lbl_1_bss_0.collisionModel)[mapCount], 1U);
    mapCount += 1;
    Hu3DModelCameraSet(lbl_1_bss_0.models02E[0], 1U);
    Hu3DModelCameraSet(lbl_1_bss_0.models02E[1], 2U);
    Hu3DModelCameraSet(lbl_1_bss_0.models02E[2], 2U);
    Hu3DModelLightInfoSet(lbl_1_bss_0.models02E[0], 1);
    Hu3DModelLightInfoSet(lbl_1_bss_0.models02E[1], 1);
    Hu3DModelAttrSet(lbl_1_bss_0.models02E[1], 1U);
    Hu3DModelAttrSet(lbl_1_bss_0.models02E[2], 1U);
    Hu3DModelShadowMapObjSet(lbl_1_bss_0.models02E[0], lbl_1_data_420);
    Hu3DModelShadowMapObjSet(lbl_1_bss_0.models02E[0], lbl_1_data_437);
    Hu3DModelShadowMapObjSet(lbl_1_bss_0.models02E[0], lbl_1_data_451);
    Hu3DModelShadowMapObjSet(lbl_1_bss_0.models02E[1], lbl_1_data_468);
    lbl_1_bss_0.models034[0] = Hu3DModelCreate(HuDataSelHeapReadNum(5963790, 268435456, HEAP_MODEL));
    lbl_1_bss_0.models034[1] = Hu3DModelCreate(HuDataSelHeapReadNum(5963792, 268435456, HEAP_MODEL));
    lbl_1_bss_0.models034[2] = Hu3DModelCreate(HuDataSelHeapReadNum(5963793, 268435456, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_0.models034[0], 1U);
    Hu3DModelAttrSet(lbl_1_bss_0.models034[1], 1U);
    Hu3DModelAttrSet(lbl_1_bss_0.models034[2], 1U);
    lbl_1_bss_0.model0C0 = Hu3DModelCreate(HuDataSelHeapReadNum(5963795, 268435456, HEAP_MODEL));
    var_r29 = (s8 *) lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    {
    char *spB0[8] = { lbl_1_data_2B8,lbl_1_data_2CD,lbl_1_data_2E2,lbl_1_data_2F7,lbl_1_data_30C,lbl_1_data_321,lbl_1_data_336,lbl_1_data_34B };
    char *sp90[8] = { lbl_1_data_360,lbl_1_data_378,lbl_1_data_390,lbl_1_data_3A8,lbl_1_data_3C0,lbl_1_data_3D8,lbl_1_data_3F0,lbl_1_data_408 };
    MGACTOR_PARAM sp70;
    MGACTOR_PARAM sp50;
    lbl_1_bss_0.linkedModelCount = 0;
    counter = 1;
    index = 0;
    while (index < 64) {
        if ((var_r29[0] >= 2) && (var_r29[0] <= 3)) {
            x1 = index % 8;
            z1 = index / 8;
            switch (var_r29[0]) {                 /* irregular */
            case 2:
                var_f30 = -45.0f;
                break;
            case 3:
                var_f30 = 45.0f;
                break;
            }
            temp_r3 = Hu3DModelLink(lbl_1_bss_0.models034[0]);
            Hu3DModelCameraSet(temp_r3, 2U);
            Hu3DModelAttrReset(temp_r3, 1U);
            Hu3DModelShadowMapObjSet(temp_r3, lbl_1_data_480);
            Hu3DModelHookSet(lbl_1_bss_0.models02E[1], spB0[counter], temp_r3);
            Hu3DModelPosSet(temp_r3, (f32) ((x1 * 100) - 350), 0.0f, (f32) ((z1 * 100) - 350));
            Hu3DModelRotSet(temp_r3, 0.0f, var_f30, 0.0f);
            lbl_1_bss_0.linkedModels[lbl_1_bss_0.linkedModelCount] = temp_r3;
            counter += 1;
            lbl_1_bss_0.linkedModelCount += 1;
        }
        var_r29 += 1;
        index += 1;
    }
    MgActorColMapInit(&lbl_1_bss_0.collisionModel, mapCount, 30);
    var_r25 = (s8 *) lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    lbl_1_bss_0.collisionCount = 0;
    sp70.height = 50.0f;
    sp70.radius = 55.0f;
    sp70.param = 0;
    sp70.type = 1;
    sp70.attr = 0;
    sp70.narrowHook = fn_1_353C;
    sp70.correctHook = fn_1_3510;
    counter = 0;
    index = 0;
    while (index < 64) {
        if (var_r25[0] == 1) {
            x2 = index % 8;
            z2 = index / 8;
            temp_r3_3 = Hu3DModelLink(lbl_1_bss_0.models034[2]);
            Hu3DModelCameraSet(temp_r3_3, 2U);
            Hu3DModelAttrReset(temp_r3_3, 1U);
            sp70.correctHookParam = counter;
            lbl_1_bss_0.collisionActors[counter] = MgActorCreate(&sp70, temp_r3_3);
            MgActorColBounceSet(lbl_1_bss_0.collisionActors[counter], 0.0f);
            sp20.x = (f32) ((x2 * 100) - 350);
            sp20.y = 0.0f;
            sp20.z = (f32) ((z2 * 100) - 350);
            MgActorPosSet(lbl_1_bss_0.collisionActors[counter], &sp20);
            MgActorColAttrSet(lbl_1_bss_0.collisionActors[counter], 96U);
            MgActorGravitySet(lbl_1_bss_0.collisionActors[counter], 700.0f);
            Hu3DModelShadowSet(lbl_1_bss_0.collisionActors[counter]->mdlId);
            temp_f31 = (f32) (u32) frandmod(360);
            lbl_1_bss_0.collisionRotY[counter] = temp_f31;
            lbl_1_bss_0.collisionActors[counter]->rotY = temp_f31;
            MgActorRotYSet(lbl_1_bss_0.collisionActors[counter], temp_f31);
            spA = Hu3DLLightCreate(lbl_1_bss_0.collisionActors[counter]->mdlId, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 160U, 160U, 160U);
            Hu3DLLightInfinitytSet(lbl_1_bss_0.collisionActors[counter]->mdlId, spA);
            sp70.param += 1;
            counter += 1;
            lbl_1_bss_0.collisionCount += 1;
        }
        var_r25 += 1;
        index += 1;
    }
    sp50.height = 150.0f;
    sp50.radius = 55.0f;
    sp50.param = 0;
    sp50.type = 0;
    sp50.attr = 0;
    sp50.narrowHook = 0;
    sp50.correctHook = 0;
    index = 0;
    while (index < 4) {
        lbl_1_bss_0.charNo[index] = GwPlayerConf[index].charNo;
        lbl_1_bss_0.padNo[index] = GwPlayerConf[index].padNo;
        if (GwPlayerConf[index].grpNo == 0) {
            var_r22 = 1;
        } else {
            var_r22 = 2;
        }
        lbl_1_bss_0.players[index] = MgPlayerCreate(index, &sp50, 2, (u16) var_r22, -31U, lbl_1_data_158);
        if (GwPlayerConf[index].grpNo == 0) {
            lbl_1_bss_0.group[index] = 0;
        } else {
            lbl_1_bss_0.group[index] = 1;
        }
        sp14.x = 0.0f;
        sp14.y = -1000.0f;
        sp14.z = 0.0f;
        MgActorPosSet((MGACTOR *) lbl_1_bss_0.players[index], &sp14);
        MgActorPosSetRaw((MGACTOR *) lbl_1_bss_0.players[index], &sp14);
        Hu3DModelPosSetV(lbl_1_bss_0.players[index]->actor->mdlId, &sp14);
        sp8 = Hu3DLLightCreate(lbl_1_bss_0.players[index]->actor->mdlId, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 160U, 160U, 160U);
        Hu3DLLightInfinitytSet(lbl_1_bss_0.players[index]->actor->mdlId, sp8);
        Hu3DModelShadowSet(lbl_1_bss_0.players[index]->actor->mdlId);
        MgPlayerVibrateCreate(lbl_1_bss_0.players[index]);
        MgPlayerDespawn(lbl_1_bss_0.players[index]);
        sp50.param += 1;
        index += 1;
    }
    var_r28 = (s8 *) lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    index = 0;
    while (index < 64) {
        if ((var_r28[0] >= 4) && (var_r28[0] <= 6)) {
            counter = var_r28[0] - 4;
            sp10 = index % 8;
            spC = index / 8;
            lbl_1_bss_0.positions[counter].x = (f32) ((sp10 * 100) - 350);
            lbl_1_bss_0.positions[counter].y = 1500.0f;
            lbl_1_bss_0.positions[counter].z = (f32) ((spC * 100) - 350);
        }
        var_r28 += 1;
        index += 1;
    }
    lbl_1_bss_0.playerState[0] = 0;
    lbl_1_bss_0.playerState[1] = 0;
    lbl_1_bss_0.playerState[2] = 0;
    lbl_1_bss_0.playerState[3] = 0;
    lbl_1_bss_0.field3F8 = 0;
    lbl_1_bss_0.field3FC = 0;
    lbl_1_bss_0.field400 = 0;
    index = 0;
    while (index < lbl_1_bss_0.collisionCount) {
        lbl_1_bss_0.collisionPairs[index][0] = 0;
        lbl_1_bss_0.collisionPairs[index][1] = 0;
        lbl_1_bss_0.collisionTimers[index] = 0;
        index += 1;
    }
    fn_1_4C90();
    lbl_1_bss_0.stream = -1;
    var_r24 = 0;
    while (var_r24 < 4) {
        CharFXPlay(lbl_1_bss_0.charNo[var_r24], 581);
        var_r24 += 1;
    }
    MgSeqCreate(&lbl_1_data_0);
    }
}
