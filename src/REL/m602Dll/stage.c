/* Maintains Odd Card Out card columns, round combinations, and stage effects. */
#define _MATH_H
#include "REL/m602Dll.h"

/* The signed words below contain unused positive and negative 1000.0 values. */
#define M602_POSITIVE_1000_BITS 0x447A0000
#define M602_NEGATIVE_1000_BITS (-998637568)
#define MSM_SE_M602_WINNER_STAGE 1576
#define M602_CARD_STOPPED_BACK_FLAG (1 << 0)
#define M602_CARD_STOPPED_FRONT_FLAG (1 << 1)
#define M602_CARD_TURNING_BACK_FLAG (1 << 2)
#define M602_CARD_TURNING_FRONT_FLAG (1 << 3)

/* Five card-image resources followed by unused words. */
s32 lbl_1_data_E8[13] = {
    DATANUM(DATA_m602, 15), DATANUM(DATA_m602, 16), DATANUM(DATA_m602, 17), DATANUM(DATA_m602, 18),
    DATANUM(DATA_m602, 19), 0, 0, 0,
    M602_POSITIVE_1000_BITS, 0, 0, M602_NEGATIVE_1000_BITS,
    -1,
};

s32 lbl_1_data_11C[2] = { DATANUM(DATA_m602, 6), DATANUM(DATA_m602, 7) };

M602CameraParams lbl_1_data_124[3] = {
    {
        1,
        20.0f,
        10.0f,
        5000.0f,
        0.6666667f,
        0.0f,
        0.0f,
        192.0f,
        288.0f,
        0.0f,
        1.0f,
        { 0.0f, 0.0f, 1800.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    },
    {
        2,
        20.0f,
        10.0f,
        5000.0f,
        0.6666667f,
        192.0f,
        0.0f,
        192.0f,
        288.0f,
        0.0f,
        1.0f,
        { 0.0f, 0.0f, 1800.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    },
    {
        4,
        20.0f,
        10.0f,
        5000.0f,
        0.6666667f,
        384.0f,
        0.0f,
        192.0f,
        288.0f,
        0.0f,
        1.0f,
        { 0.0f, 0.0f, 1800.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    },
};

M602CameraParams lbl_1_data_214 = {
    4,
    20.0f,
    10.0f,
    5000.0f,
    0.9791667f,
    0.0f,
    0.0f,
    576.0f,
    576.0f,
    0.0f,
    1.0f,
    { 0.0f, -19.0f, 1200.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, -19.0f, 0.0f },
};

char lbl_1_data_264[9] = "602_card";

char lbl_1_data_26D[11] = "test";

/* const */
static const Point3d lbl_1_rodata_60 = { 0.0f, 0.0f, -160.0f };

static const Point3d lbl_1_rodata_6C = { 1.1f, 1.1f, 1.1f };

static const Point3d lbl_1_rodata_78 = { 0.0f, 0.0f, 200.0f };

static const Point3d lbl_1_rodata_84 = { 0.0f, 0.0f, -140.0f };

static const Point3d lbl_1_rodata_90 = { 0.7f, 0.8f, 0.05f };

static const Point3d lbl_1_rodata_9C = { 0.9f, 1.15f, 0.05f };

/* Three columns hold card images, banks, transforms, and current/pending flip timing. */
M602BssA0Record lbl_1_bss_A0[3];

/* The five selectable card-face texture animations. */
ANIMDATA *lbl_1_bss_8C[5];

/* Each round has one image index and three animation-bank choices. */
struct _struct_lbl_1_bss_3C_0x8 lbl_1_bss_3C[10];

/* Model displaying the captured card-camera image. */
s16 lbl_1_bss_3A;

/* Texture animation binding the captured image to its display model. */
s16 lbl_1_bss_38;

/* Two animated models switched by the descending-model impact countdown. */
s16 lbl_1_bss_34[2];

/* Frames remaining for the alternate model shown during the impact reaction. */
s16 lbl_1_bss_32;

/* Animated card-view model whose position, scale, and speed depend on how many columns turn. */
s16 lbl_1_bss_30;

/* Stage animation shown after the winning cards turn sideways. */
s16 lbl_1_bss_2E;

/* Winner-stage phase: zero turns cards, positive waits, negative runs the final motion. */
s16 lbl_1_bss_2C;

/* Settling delay after all three cards finish turning sideways. */
s16 lbl_1_bss_2A;

/* Image index remembered during shuffling while a column is not turning with its back visible;
* random draws exclude it, and selecting the final round image leaves it unchanged. */
u8 lbl_1_bss_28;

/* 576 by 288 texture receiving the three card-camera views. */
ANIMDATA *lbl_1_bss_24;

/* Current round number, capped at ten. */
s16 lbl_1_bss_20;

void fn_1_1770(void)
{

}

void fn_1_1774(void)
{

}

/* The round sequence invokes this empty stage callback each frame. */
void fn_1_1778(s16 frameNo)
{

}

void fn_1_177C(void)
{

}

void fn_1_1780(void)
{

}

void fn_1_1784(void)
{

}

void fn_1_1788(void)
{

}

/* The round callback queries the round number to stop play after the tenth round. */
s16 fn_1_178C(void)
{
    return lbl_1_bss_20;
}

/* Called by intro setup to create the three card views, their display texture, and stage models. */
void fn_1_179C(void)
{
    s32 cardImageIndex;
    s32 stageIndex;
    u16 cameraMask;
    s16 cardLayer;
    u32 bufferSize;

    stageIndex = 0;
    while (stageIndex < 3) {
        Hu3DCameraCreate(lbl_1_data_124[stageIndex].cameraBit);
        Hu3DCameraViewportSet(
            lbl_1_data_124[stageIndex].cameraBit, lbl_1_data_124[stageIndex].viewportX,
            lbl_1_data_124[stageIndex].viewportY, lbl_1_data_124[stageIndex].viewportW,
            lbl_1_data_124[stageIndex].viewportH, lbl_1_data_124[stageIndex].minZ,
            lbl_1_data_124[stageIndex].maxZ);
        Hu3DCameraPerspectiveSet(
            lbl_1_data_124[stageIndex].cameraBit, lbl_1_data_124[stageIndex].fov,
            lbl_1_data_124[stageIndex].nearPlane, lbl_1_data_124[stageIndex].farPlane,
            lbl_1_data_124[stageIndex].aspect);
        Hu3DCameraScissorSet(
            lbl_1_data_124[stageIndex].cameraBit, (u32) lbl_1_data_124[stageIndex].viewportX,
            (u32) lbl_1_data_124[stageIndex].viewportY, (u32) lbl_1_data_124[stageIndex].viewportW,
            (u32) lbl_1_data_124[stageIndex].viewportH);
        Hu3DCameraPosSetV(lbl_1_data_124[stageIndex].cameraBit, &lbl_1_data_124[stageIndex].pos,
                          &lbl_1_data_124[stageIndex].up, &lbl_1_data_124[stageIndex].target);
        stageIndex += 1;
    }
    stageIndex = 0;
    while (stageIndex < 5) {
        lbl_1_bss_8C[stageIndex] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_E8[stageIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        stageIndex += 1;
    }
    for (stageIndex = 0; stageIndex < 3; stageIndex++) {
        lbl_1_bss_A0[stageIndex].cardImage = 0;
        lbl_1_bss_A0[stageIndex].animationBank = 0;
        lbl_1_bss_A0[stageIndex].turnFrames = 0;
        lbl_1_bss_A0[stageIndex].turnDegreesPerFrame = 0.0f;
        lbl_1_bss_A0[stageIndex].pendingImage = -1;
        lbl_1_bss_A0[stageIndex].pendingBank = -1;
        cardImageIndex = 0;
        while (cardImageIndex < 5) {
            lbl_1_bss_A0[stageIndex].model[cardImageIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m602, 0), HU_MEMNUM_OVL, HEAP_MODEL));
            if (stageIndex == 0) {
                cameraMask = 1;
            }
            if (stageIndex == 1) {
                cameraMask = 2;
            }
            if (stageIndex == 2) {
                cameraMask = 4;
            }
            cardLayer = 5;
            Hu3DModelCameraSet(lbl_1_bss_A0[stageIndex].model[cardImageIndex], cameraMask);
            Hu3DModelLayerSet(lbl_1_bss_A0[stageIndex].model[cardImageIndex], cardLayer);
            lbl_1_bss_A0[stageIndex].pos.x = lbl_1_bss_A0[stageIndex].pos.y =
                lbl_1_bss_A0[stageIndex].pos.z = 0.0f;
            lbl_1_bss_A0[stageIndex].rot.x = lbl_1_bss_A0[stageIndex].rot.z = 0.0f;
            lbl_1_bss_A0[stageIndex].rot.y = 180.0f;
            lbl_1_bss_A0[stageIndex].scale.x = lbl_1_bss_A0[stageIndex].scale.y =
                lbl_1_bss_A0[stageIndex].scale.z = 1.0f;
            Hu3DModelPosSetV(lbl_1_bss_A0[stageIndex].model[cardImageIndex],
                             &lbl_1_bss_A0[stageIndex].pos);
            Hu3DModelRotSetV(lbl_1_bss_A0[stageIndex].model[cardImageIndex],
                             &lbl_1_bss_A0[stageIndex].rot);
            Hu3DModelScaleSetV(lbl_1_bss_A0[stageIndex].model[cardImageIndex],
                               &lbl_1_bss_A0[stageIndex].scale);
            lbl_1_bss_A0[stageIndex].anim[cardImageIndex] =
                Hu3DAnimCreate(lbl_1_bss_8C[cardImageIndex],
                               lbl_1_bss_A0[stageIndex].model[cardImageIndex], lbl_1_data_264);
            Hu3DAnimBankSet(lbl_1_bss_A0[stageIndex].anim[cardImageIndex],
                            (u16) lbl_1_bss_A0[stageIndex].animationBank);
            if (cardImageIndex != lbl_1_bss_A0[stageIndex].cardImage) {
                Hu3DModelAttrSet(lbl_1_bss_A0[stageIndex].model[cardImageIndex], HU3D_ATTR_DISPOFF);
            }
            cardImageIndex += 1;
        }
    }
    Hu3DCameraLayerHookSet(8, 0, fn_1_2250);
    lbl_1_bss_3A =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m602, 4), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_3A, 8U);
    Hu3DModelLayerSet(lbl_1_bss_3A, 0);
    lbl_1_bss_24 = HuSprAnimMake(576, 288, 0);
    lbl_1_bss_24->bmp->palData = NULL;
    lbl_1_bss_24->bmp->palNum = 0;
    bufferSize = GXGetTexBufferSize(576U, 288U, GX_TF_RGBA8, 0U, 0U);
    lbl_1_bss_24->bmp->data = HuMemDirectMallocNum(HEAP_MODEL, bufferSize, HU_MEMNUM_OVL);
    lbl_1_bss_38 = Hu3DAnimCreate(lbl_1_bss_24, lbl_1_bss_3A, lbl_1_data_26D);
    lbl_1_bss_32 = 0;
    stageIndex = 0;
    while (stageIndex < 2) {
        lbl_1_bss_34[stageIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_11C[stageIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAmbSet(lbl_1_bss_34[stageIndex], 1.0f, 1.0f, 1.0f);
        if (stageIndex != 0) {
            Hu3DModelAttrSet(lbl_1_bss_34[stageIndex], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_34[stageIndex], HU3D_MOTATTR_LOOP);
        }
        Hu3DModelPosSet(lbl_1_bss_34[stageIndex], lbl_1_rodata_60.x, lbl_1_rodata_60.y,
                        lbl_1_rodata_60.z);
        Hu3DModelScaleSet(lbl_1_bss_34[stageIndex], lbl_1_rodata_6C.x, lbl_1_rodata_6C.y,
                          lbl_1_rodata_6C.z);
        Hu3DModelCameraSet(lbl_1_bss_34[stageIndex], 7U);
        Hu3DModelLayerSet(lbl_1_bss_34[stageIndex], 4);
        stageIndex += 1;
    }
    lbl_1_bss_30 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m602, 8), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_30, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_30, HU3D_MOTATTR_LOOP);
    Hu3DModelPosSet(lbl_1_bss_30, lbl_1_rodata_78.x, lbl_1_rodata_78.y, lbl_1_rodata_78.z);
    Hu3DModelScaleSet(lbl_1_bss_30, lbl_1_rodata_90.x, lbl_1_rodata_90.y, lbl_1_rodata_90.z);
    Hu3DModelCameraSet(lbl_1_bss_30, 7U);
    Hu3DModelLayerSet(lbl_1_bss_30, 6);
    Hu3DMotionSpeedSet(lbl_1_bss_30, 4.0f);
    fn_1_39D0();
    lbl_1_bss_20 = 0;
    Hu3DBGColorSet(0U, 178U, 235U);
    lbl_1_bss_2E =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m602, 9), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_2E, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_2E, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(lbl_1_bss_2E, 4U);
    Hu3DModelLayerSet(lbl_1_bss_2E, 5);
}

/* Card-display layer hook: capture the three card views, then clear depth for the stage draw. */
void fn_1_2250(s16 layerNo)
{
    Hu3DFbCopyExec(0, 0, 576, 288, GX_TF_RGBA8, 0, lbl_1_bss_24->bmp->data);
    Hu3DZClear();
}

/* Called by the exit sequence to release the card-view texture animation. */
void fn_1_22A0(void)
{
    HuSprAnimKill(lbl_1_bss_24);
    lbl_1_bss_24 = NULL;
}

/* Round and result callbacks call this each frame to shuffle, turn, and display the card
 * columns. */
void fn_1_22DC(void)
{
    s16 turningColumnCount;
    s32 cardImageIndex;
    s32 columnIndex;
    u8 shuffledImage;

    /* Change the shuffled face only while a column is waiting for its final card. */
    turningColumnCount = 0;
    if ((lbl_1_bss_A0->shuffleFrames > 0) || (lbl_1_bss_A0[1].shuffleFrames > 0) ||
        (lbl_1_bss_A0[2].shuffleFrames > 0)) {
        do {
            shuffledImage = (u8) frand() % 5;
        } while (shuffledImage == lbl_1_bss_28);
    }
    for (columnIndex = 0; columnIndex < 3; columnIndex++) {
        if ((lbl_1_bss_A0[columnIndex].pendingImage >= 0) &&
            (lbl_1_bss_A0[columnIndex].pendingBank >= 0) &&
            (lbl_1_bss_A0[columnIndex].shuffleFrames > 0)) {
            lbl_1_bss_A0[columnIndex].shuffleFrames -= 1;
            if (lbl_1_bss_A0[columnIndex].shuffleFrames <= 0) {
                fn_1_3FD0((s16) columnIndex, (s16) lbl_1_bss_A0[columnIndex].pendingImage,
                          (s16) lbl_1_bss_A0[columnIndex].pendingBank);
                lbl_1_bss_A0[columnIndex].pendingImage = -1;
                lbl_1_bss_A0[columnIndex].pendingBank = -1;
                lbl_1_bss_A0[columnIndex].shuffleFrames = 0;
            } else if ((s32) (fn_1_2E3C((s16) columnIndex) & M602_CARD_TURNING_BACK_FLAG) != 0) {
                /* All turning backs share this frame's shuffled image and its first bank. */
                fn_1_3FD0((s16) columnIndex, shuffledImage, 0);
            } else {
                lbl_1_bss_28 = (u8) lbl_1_bss_A0[columnIndex].cardImage;
            }
        }
        if (lbl_1_bss_A0[columnIndex].turnFrames > 0) {
            lbl_1_bss_A0[columnIndex].rot.y += lbl_1_bss_A0[columnIndex].turnDegreesPerFrame;
            if (lbl_1_bss_A0[columnIndex].rot.y >= 360.0f) {
                lbl_1_bss_A0[columnIndex].rot.y -= 360.0f;
            }
            if (lbl_1_bss_A0[columnIndex].rot.y <= -360.0f) {
                lbl_1_bss_A0[columnIndex].rot.y += 360.0f;
            }
            lbl_1_bss_A0[columnIndex].turnFrames -= 1;
            if (lbl_1_bss_A0[columnIndex].turnFrames <= 0) {
                lbl_1_bss_A0[columnIndex].turnFrames = 0;
                lbl_1_bss_A0[columnIndex].turnDegreesPerFrame = 0.0f;
                if (((-90.0f <= lbl_1_bss_A0[columnIndex].rot.y) &&
                     (lbl_1_bss_A0[columnIndex].rot.y <= 90.0f)) ||
                    (lbl_1_bss_A0[columnIndex].rot.y <= -270.0f) ||
                    (270.0f <= lbl_1_bss_A0[columnIndex].rot.y)) {
                    lbl_1_bss_A0[columnIndex].rot.y = 0.0f;
                } else {
                    lbl_1_bss_A0[columnIndex].rot.y = 180.0f;
                }
            }
            cardImageIndex = 0;
            while (cardImageIndex < 5) {
                Hu3DModelRotSetV(lbl_1_bss_A0[columnIndex].model[cardImageIndex],
                                 &lbl_1_bss_A0[columnIndex].rot);
                cardImageIndex += 1;
            }
            turningColumnCount += 1;
        }
    }
    if (turningColumnCount < 3) {
        Hu3DModelPosSet(lbl_1_bss_30, lbl_1_rodata_84.x, lbl_1_rodata_84.y, lbl_1_rodata_84.z);
        Hu3DModelScaleSet(lbl_1_bss_30, lbl_1_rodata_9C.x, lbl_1_rodata_9C.y, lbl_1_rodata_9C.z);
        Hu3DModelAttrReset(lbl_1_bss_30, HU3D_ATTR_DISPOFF);
        Hu3DMotionSpeedSet(lbl_1_bss_30, 2.0f);
    } else {
        Hu3DModelPosSet(lbl_1_bss_30, lbl_1_rodata_78.x, lbl_1_rodata_78.y, lbl_1_rodata_78.z);
        Hu3DModelScaleSet(lbl_1_bss_30, lbl_1_rodata_90.x, lbl_1_rodata_90.y, lbl_1_rodata_90.z);
        Hu3DModelAttrReset(lbl_1_bss_30, HU3D_ATTR_DISPOFF);
        Hu3DMotionSpeedSet(lbl_1_bss_30, 4.0f);
    }
    if (lbl_1_bss_32 != 0) {
        Hu3DModelAttrSet(lbl_1_bss_34[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_34[1], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_34[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_34[1], HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_32 -= 1;
    if (lbl_1_bss_32 < 0) {
        lbl_1_bss_32 = 0;
    }
}

/* Called at each round start to choose the flip duration and schedule the next card combination. */
void fn_1_2998(void)
{
    s16 flipFrames;
    s16 halfTurnsOrShuffleFrames;

    flipFrames = frand() % 60 + 90;
    halfTurnsOrShuffleFrames = flipFrames / 10;
    /* The first round starts on the back; later rounds start on the front. */
    halfTurnsOrShuffleFrames % 2 ? halfTurnsOrShuffleFrames-- : 0;
    if (lbl_1_bss_20 == 0) {
        halfTurnsOrShuffleFrames += 1;
    }
    lbl_1_bss_20 += 1;
    if (lbl_1_bss_20 > 10) {
        lbl_1_bss_20 = 10;
    }
    fn_1_2BB0(flipFrames, halfTurnsOrShuffleFrames);
    /* The final front-facing half-turn is excluded from the shuffle interval. */
    halfTurnsOrShuffleFrames = 0.5f + (360.0f / lbl_1_bss_A0[0].turnDegreesPerFrame) *
                                          ((halfTurnsOrShuffleFrames - 1) / 2.0f);
    fn_1_2CBC(lbl_1_bss_20, halfTurnsOrShuffleFrames);
}

/* Called during the reveal to spin the odd card once; returns column one if no pair matches. */
s16 fn_1_2B28(void)
{
    s16 oddColumn;

    oddColumn = fn_1_2D5C();
    if (oddColumn != 3) {
        fn_1_2C00(oddColumn, 90, 2);
        if (oddColumn == 0) { return 0; }
        if (oddColumn == 1) { return 1; }
        if (oddColumn == 2) { return 2; }
    }
    return 1;
}

/* Round setup and the no-winner sequence call this to start equal turns on all three columns. */
void fn_1_2BB0(s16 flipFrames, s16 halfTurns)
{
    s32 columnIndex;

    columnIndex = 0;
    while (columnIndex < 3) {
        fn_1_2C00((s16) columnIndex, flipFrames, halfTurns);
        columnIndex += 1;
    }
}

/* Called by card-turn setup to assign a column duration in frames and a count of half-turns. */
void fn_1_2C00(s16 columnIndex, s16 flipFrames, s16 halfTurns)
{
    if ((columnIndex < 0) || (columnIndex > 2)) {
        return;
    }
    lbl_1_bss_A0[columnIndex].turnFrames = flipFrames;
    lbl_1_bss_A0[columnIndex].turnDegreesPerFrame = (180.0f * (f32) halfTurns) / (f32) flipFrames;
}

/* Round setup schedules the prepared image and animation bank after the shuffle countdown;
 * the caller supplies a round number from one through ten. */
void fn_1_2CBC(s16 roundNumber, s32 shuffleFrames)
{
    s32 columnIndex;

    roundNumber -= 1;
    columnIndex = 0;
    while (columnIndex < 3) {
        lbl_1_bss_A0[columnIndex].pendingImage = (s32) lbl_1_bss_3C[roundNumber].cardImage;
        lbl_1_bss_A0[columnIndex].pendingBank =
            (s32) lbl_1_bss_3C[roundNumber].choices[columnIndex];
        lbl_1_bss_A0[columnIndex].shuffleFrames = shuffleFrames;
        columnIndex += 1;
    }
}

/* Reveal and scoring callbacks use the first equal pair to find the odd column; three means
 * no pair. Three equal cards return column two because the first pair takes precedence. */
s16 fn_1_2D5C(void)
{
    if ((lbl_1_bss_A0->cardImage == lbl_1_bss_A0[1].cardImage) &&
        (lbl_1_bss_A0->animationBank == lbl_1_bss_A0[1].animationBank)) {
        return 2;
    }
    if ((lbl_1_bss_A0->cardImage == lbl_1_bss_A0[2].cardImage) &&
        (lbl_1_bss_A0->animationBank == lbl_1_bss_A0[2].animationBank)) {
        return 1;
    }
    if ((lbl_1_bss_A0[1].cardImage == lbl_1_bss_A0[2].cardImage) &&
        (lbl_1_bss_A0[1].animationBank == lbl_1_bss_A0[2].animationBank)) {
        return 0;
    }
    return 3;
}

/* Round updates query a column: bits zero/one mark a stopped back/front, bits two/three a moving
 * back/front. Exact edge-on angles count as backs. */
s16 fn_1_2E3C(s16 columnIndex)
{
    if (lbl_1_bss_A0[columnIndex].turnFrames == 0) {
        if (((-90.0f < lbl_1_bss_A0[columnIndex].rot.y) &&
             (lbl_1_bss_A0[columnIndex].rot.y < 90.0f)) ||
            (lbl_1_bss_A0[columnIndex].rot.y < -270.0f) ||
            (270.0f < lbl_1_bss_A0[columnIndex].rot.y)) {
            return M602_CARD_STOPPED_FRONT_FLAG;
        }
        return M602_CARD_STOPPED_BACK_FLAG;
    }
    if (((-90.0f < lbl_1_bss_A0[columnIndex].rot.y) && (lbl_1_bss_A0[columnIndex].rot.y < 90.0f)) ||
        (lbl_1_bss_A0[columnIndex].rot.y < -270.0f) || (270.0f < lbl_1_bss_A0[columnIndex].rot.y)) {
        return M602_CARD_TURNING_FRONT_FLAG;
    }
    return M602_CARD_TURNING_BACK_FLAG;
}

/* The player-reaction update calls this when the descending model reaches the player's height;
 * an active stage-model effect keeps its existing countdown. */
void fn_1_2FDC(s16 effectFrames)
{
    if (lbl_1_bss_32 <= 0) {
        lbl_1_bss_32 = effectFrames;
    }
}

/* Winner-stage updates turn all cards sideways, hide their faces, and wait for a short settling
 * delay. */
s32 fn_1_3000(s32 startTurn)
{
    s16 finishedColumnCount;
    s16 cardImageIndex;
    s16 columnIndex;

    finishedColumnCount = 0;
    for (columnIndex = 0; columnIndex < 3; columnIndex++) {
        if (startTurn != 0) {
            /* A fresh winner sequence forces every column through a 270-degree turn. */
            lbl_1_bss_2A = 0;
            lbl_1_bss_A0[columnIndex].turnFrames = 45;
            lbl_1_bss_A0[columnIndex].turnDegreesPerFrame =
                (f32) (270 / lbl_1_bss_A0[columnIndex].turnFrames);
        }
        if (lbl_1_bss_A0[columnIndex].turnFrames > 0) {
            lbl_1_bss_A0[columnIndex].rot.y += lbl_1_bss_A0[columnIndex].turnDegreesPerFrame;
            if (lbl_1_bss_A0[columnIndex].rot.y >= 360.0f) {
                lbl_1_bss_A0[columnIndex].rot.y -= 360.0f;
            }
            if (lbl_1_bss_A0[columnIndex].rot.y <= -360.0f) {
                lbl_1_bss_A0[columnIndex].rot.y += 360.0f;
            }
            lbl_1_bss_A0[columnIndex].turnFrames -= 1;
            if (lbl_1_bss_A0[columnIndex].turnFrames <= 0) {
                lbl_1_bss_A0[columnIndex].turnFrames = 0;
                lbl_1_bss_A0[columnIndex].turnDegreesPerFrame = 0.0f;
                if (((0.0f <= lbl_1_bss_A0[columnIndex].rot.y) &&
                     (lbl_1_bss_A0[columnIndex].rot.y <= 180.0f)) ||
                    ((-360.0f <= lbl_1_bss_A0[columnIndex].rot.y) &&
                     (lbl_1_bss_A0[columnIndex].rot.y <= -180.0f))) {
                    lbl_1_bss_A0[columnIndex].rot.y = 90.0f;
                } else {
                    lbl_1_bss_A0[columnIndex].rot.y = 270.0f;
                }
                cardImageIndex = 0;
                while (cardImageIndex < 5) {
                    Hu3DModelAttrSet(lbl_1_bss_A0[columnIndex].model[cardImageIndex],
                                     HU3D_ATTR_DISPOFF);
                    cardImageIndex += 1;
                }
                finishedColumnCount += 1;
            }
            cardImageIndex = 0;
            while (cardImageIndex < 5) {
                Hu3DModelRotSetV(lbl_1_bss_A0[columnIndex].model[cardImageIndex],
                                 &lbl_1_bss_A0[columnIndex].rot);
                cardImageIndex += 1;
            }
        }
    }
    Hu3DModelAttrSet(lbl_1_bss_30, HU3D_ATTR_DISPOFF);
    if (finishedColumnCount >= 3) {
        lbl_1_bss_2A = 1;
    }
    if (lbl_1_bss_2A > 0) {
        if (lbl_1_bss_2A > 5) {
            return 1;
        }
        lbl_1_bss_2A += 1;
    }
    return 0;
}

/* Pre-winner and winner-display callbacks turn the cards sideways and play the final stage
 * motion; -1 begins the winner display and 1 marks the end of that motion. */
s16 fn_1_34A4(s16 frameNo)
{
    f32 motionEnd;
    s16 stageReady;
    s16 stageIndex;

    if (frameNo == 0) {
        fn_1_3000(1);
        lbl_1_bss_2C = 0;
    } else {
        if (lbl_1_bss_2C < 0) {
            if (lbl_1_bss_2C == -1) {
                stageIndex = 0;
                while (stageIndex < 2) {
                    Hu3DModelAttrSet(lbl_1_bss_34[stageIndex], HU3D_ATTR_DISPOFF);
                    stageIndex += 1;
                }
                Hu3DCameraViewportSet(lbl_1_data_214.cameraBit, lbl_1_data_214.viewportX,
                                      lbl_1_data_214.viewportY, lbl_1_data_214.viewportW,
                                      lbl_1_data_214.viewportH, lbl_1_data_214.minZ,
                                      lbl_1_data_214.maxZ);
                Hu3DCameraPerspectiveSet(lbl_1_data_214.cameraBit, lbl_1_data_214.fov,
                                         lbl_1_data_214.nearPlane, lbl_1_data_214.farPlane,
                                         lbl_1_data_214.aspect);
                Hu3DCameraScissorSet(lbl_1_data_214.cameraBit, (u32) lbl_1_data_214.viewportX,
                                     (u32) lbl_1_data_214.viewportY, (u32) lbl_1_data_214.viewportW,
                                     (u32) lbl_1_data_214.viewportH);
                Hu3DCameraPosSetV(lbl_1_data_214.cameraBit, &lbl_1_data_214.pos, &lbl_1_data_214.up,
                                  &lbl_1_data_214.target);
                Hu3DModelAttrReset(lbl_1_bss_2E, HU3D_ATTR_DISPOFF);
                Hu3DMotionTimeSet(lbl_1_bss_2E, 0.0f);
                Hu3DModelAttrSet(lbl_1_bss_2E, HU3D_MOTATTR_LOOP);
                HuAudFXPlay(MSM_SE_M602_WINNER_STAGE);
                fn_1_4C84();
                lbl_1_bss_2C -= 1;
            }
            if (lbl_1_bss_2C < -1) {
                motionEnd = Hu3DMotionMaxTimeGet(lbl_1_bss_2E);
                if ((1.0f + Hu3DMotionTimeGet(lbl_1_bss_2E)) >= motionEnd) {
                    lbl_1_bss_2C = -512;
                    Hu3DModelAttrSet(lbl_1_bss_2E, HU3D_MOTATTR_PAUSE);
                } else {
                    lbl_1_bss_2C = -20;
                }
            }
        }
        if (lbl_1_bss_2C > 0) {
            stageReady = 0;
            stageIndex = 0;
            while (stageIndex < 2) {
                if ((Hu3DMotionTimeGet(lbl_1_bss_34[stageIndex]) <= 50.0f) ||
                    (280.0f <= Hu3DMotionTimeGet(lbl_1_bss_34[stageIndex]))) {
                    Hu3DModelAttrSet(lbl_1_bss_34[stageIndex], HU3D_MOTATTR_PAUSE);
                } else {
                    stageReady += 1;
                }
                stageIndex += 1;
            }
            if (stageReady == 0) {
                lbl_1_bss_2C = -1;
            }
        }
        if (lbl_1_bss_2C == 0) {
            stageReady = fn_1_3000(0);
            if (stageReady != 0) { lbl_1_bss_2C += 1; }
        }
    }
    fn_1_3908();
    /* Minus one signals the transition; minus 512 signals the completed final motion. */
    if (lbl_1_bss_2C == -512) { return 1; }
    if (lbl_1_bss_2C == -1) { return -1; }
    return 0;
}

/* Result callbacks pause the two stage motions outside their central playback interval. */
void fn_1_3908(void)
{
    s16 stageIndex;

    stageIndex = 0;
    while (stageIndex < 2) {
        if ((Hu3DMotionTimeGet(lbl_1_bss_34[stageIndex]) <= 50.0f) ||
            (280.0f <= Hu3DMotionTimeGet(lbl_1_bss_34[stageIndex]))) {
            Hu3DModelAttrSet(lbl_1_bss_34[stageIndex], HU3D_MOTATTR_PAUSE);
        }
        stageIndex += 1;
    }
}

/* Intro setup prepares ten rounds, with one distinct animation bank among each group of three
 * cards. */
void fn_1_39D0(void)
{
    s16 previousOddColumn;
    s16 previousRoundOrSwapBank;
    s16 oddColumn;
    s16 randomChoiceOrSwapBank;
    s16 roundIndex;

    for (roundIndex = 0; roundIndex < 10; roundIndex++) {
        if (roundIndex == 0) {
            lbl_1_bss_3C[roundIndex].cardImage = (u8)frand() % 5;
        } else {
            previousRoundOrSwapBank = roundIndex - 1;
            /* Consecutive rounds use different card images even when their banks repeat. */
            do {
                lbl_1_bss_3C[roundIndex].cardImage = (u8)frand() % 5;
            } while (lbl_1_bss_3C[roundIndex].cardImage ==
                     lbl_1_bss_3C[previousRoundOrSwapBank].cardImage);
        }
        lbl_1_bss_3C[roundIndex].choices[0] = 0;
        lbl_1_bss_3C[roundIndex].choices[1] = (u8)frand() % 4 + 1;
        lbl_1_bss_3C[roundIndex].choices[2] = lbl_1_bss_3C[roundIndex].choices[(u8)frand() % 2];
        randomChoiceOrSwapBank = (u8)frand() % 10;
        if (randomChoiceOrSwapBank < 5) {
            randomChoiceOrSwapBank = lbl_1_bss_3C[roundIndex].choices[0];
            lbl_1_bss_3C[roundIndex].choices[0] = lbl_1_bss_3C[roundIndex].choices[1];
            lbl_1_bss_3C[roundIndex].choices[1] = randomChoiceOrSwapBank;
        }
        randomChoiceOrSwapBank = (u8)frand() % 10;
        if (randomChoiceOrSwapBank < 5) {
            randomChoiceOrSwapBank = lbl_1_bss_3C[roundIndex].choices[1];
            lbl_1_bss_3C[roundIndex].choices[1] = lbl_1_bss_3C[roundIndex].choices[2];
            lbl_1_bss_3C[roundIndex].choices[2] = randomChoiceOrSwapBank;
        }
        randomChoiceOrSwapBank = (u8)frand() % 10;
        if (randomChoiceOrSwapBank < 5) {
            randomChoiceOrSwapBank = lbl_1_bss_3C[roundIndex].choices[0];
            lbl_1_bss_3C[roundIndex].choices[0] = lbl_1_bss_3C[roundIndex].choices[2];
            lbl_1_bss_3C[roundIndex].choices[2] = randomChoiceOrSwapBank;
        }
    }
    oddColumn = previousOddColumn = -1;
    for (roundIndex = 0; roundIndex < 10; roundIndex++) {
        if (lbl_1_bss_3C[roundIndex].choices[0] == lbl_1_bss_3C[roundIndex].choices[1]) {
            oddColumn = 2;
        } else if (lbl_1_bss_3C[roundIndex].choices[0] == lbl_1_bss_3C[roundIndex].choices[2]) {
            oddColumn = 1;
        } else {
            oddColumn = 0;
        }
        /* Usually move a repeated odd-card position to an adjacent column. */
        if (previousOddColumn >= 0 && previousOddColumn == oddColumn) {
            randomChoiceOrSwapBank = (u8)frand() % 100;
            if (randomChoiceOrSwapBank >= 4) {
                if (randomChoiceOrSwapBank < 52) {
                    randomChoiceOrSwapBank = oddColumn + 1;
                    if (randomChoiceOrSwapBank > 2) { randomChoiceOrSwapBank = 0; }
                    if (randomChoiceOrSwapBank < 0) { randomChoiceOrSwapBank = 2; }
                    previousRoundOrSwapBank = lbl_1_bss_3C[roundIndex].choices[oddColumn];
                    lbl_1_bss_3C[roundIndex].choices[oddColumn] =
                        lbl_1_bss_3C[roundIndex].choices[randomChoiceOrSwapBank];
                    lbl_1_bss_3C[roundIndex].choices[randomChoiceOrSwapBank] =
                        previousRoundOrSwapBank;
                } else {
                    randomChoiceOrSwapBank = oddColumn - 1;
                    if (randomChoiceOrSwapBank > 2) { randomChoiceOrSwapBank = 0; }
                    if (randomChoiceOrSwapBank < 0) { randomChoiceOrSwapBank = 2; }
                    previousRoundOrSwapBank = lbl_1_bss_3C[roundIndex].choices[oddColumn];
                    lbl_1_bss_3C[roundIndex].choices[oddColumn] =
                        lbl_1_bss_3C[roundIndex].choices[randomChoiceOrSwapBank];
                    lbl_1_bss_3C[roundIndex].choices[randomChoiceOrSwapBank] =
                        previousRoundOrSwapBank;
                }
            }
        }
        /* Remember the position before any swap, including when the odd card moved. */
        previousOddColumn = oddColumn;
        oddColumn = -1;
    }
}

/* Shuffle updates select a column image and restart the chosen animation bank on all five
 * face models, including the four hidden images. */
void fn_1_3FD0(s16 columnIndex, s16 cardImageIndex, s16 animationBank)
{
    s16 modelIndex;

    lbl_1_bss_A0[columnIndex].cardImage = (s32) cardImageIndex;
    lbl_1_bss_A0[columnIndex].animationBank = (s32) animationBank;
    modelIndex = 0;
    while (modelIndex < 5) {
        if (modelIndex != cardImageIndex) {
            Hu3DModelAttrSet(lbl_1_bss_A0[columnIndex].model[modelIndex], HU3D_ATTR_DISPOFF);
        }
        if (modelIndex == cardImageIndex) {
            Hu3DModelAttrReset(lbl_1_bss_A0[columnIndex].model[modelIndex], HU3D_ATTR_DISPOFF);
        }
        /* The texture-animation IDs follow the five model IDs in each column. */
        Hu3DAnimBankSet((&lbl_1_bss_A0[columnIndex].model[modelIndex])[5],
                        (u16) lbl_1_bss_A0[columnIndex].animationBank);
        modelIndex += 1;
    }
}
