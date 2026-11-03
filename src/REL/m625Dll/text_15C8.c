/* Day and night stage models, water deformation, and the water material. */
#include <math.h>
#include "REL/m625Dll/wave.h"
#include "datadir_enum.h"
#include "game/memory.h"

extern s16 lbl_802C0278;
extern HUPROCESS *lbl_1_bss_810;
void fn_1_1398(Point3d *, const Point3d *, f32);
void fn_1_1FEC(const f32 *);
void fn_1_1E80(void);
void fn_1_20AC(void);
void fn_1_2528(HU3D_DRAW_OBJ *, HSF_MATERIAL *);

/* Called during stage setup to create the day or night lighting and water models. */
void fn_1_15C8(void)
{
    s16 effectMode;
    s16 lightMode;
    s16 index;

    effectMode = lbl_802C0278;
    lightMode = effectMode;
    if (lightMode == 0) {
        lbl_1_data_180[0].light = fn_800286D0(
            &lbl_1_data_180[0].position, &lbl_1_data_180[0].direction,
            &lbl_1_data_180[0].color);
        Hu3DGLightStaticSet(lbl_1_data_180[0].light, 1);
        Hu3DGLightInfinitytSet(lbl_1_data_180[0].light);
        lbl_1_data_180[1].light = lbl_1_data_180[0].light;
    } else {
        lbl_1_data_180[1].light = fn_800286D0(
            &lbl_1_data_180[1].position, &lbl_1_data_180[1].direction,
            &lbl_1_data_180[1].color);
        Hu3DGLightStaticSet(lbl_1_data_180[1].light, 1);
        Hu3DGLightInfinitytSet(lbl_1_data_180[1].light);
        lbl_1_data_180[0].light = lbl_1_data_180[1].light;
        lbl_1_data_1C0.r = 255;
        lbl_1_data_1C0.g = 255;
        lbl_1_data_1C0.b = 255;
        lbl_1_data_1C0.a = 0;
    }
    if (lightMode == 0) {
        lbl_1_bss_194.baseModel = fn_80025A04(
            fn_80008DD0(DATANUM(0x540000, 0), 0x10000000, 2));
        for (index = 0; index < 5; index++) {
            lbl_1_bss_194.effects[index] = fn_80025A04(
                fn_80008DD0(lbl_1_data_108[index].resourceNumber,
                                     0x10000000, 2));
            lbl_1_bss_194.positionsY[index] = lbl_1_data_108[index].positionY;
            fn_80026760(lbl_1_bss_194.effects[index],
                            lbl_1_bss_194.positionsY[index],
                            0.0f, 0.0f);
            lbl_1_bss_194.motionValues[index] = lbl_1_data_108[index].motionValue;
            fn_800269E8(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 1)));
            if (index == 2) {
                /* This middle effect is sampled at frame 16, then held paused. */
                fn_80026A44(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 1)));
                fn_800269E8(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 0)));
                fn_8002D414(lbl_1_bss_194.effects[index], 16.0f);
                fn_80026A44(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 0)));
                fn_800269E8(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 1)));
            }
        }
    } else {
        lbl_1_bss_194.baseModel = fn_80025A04(
            fn_80008DD0(DATANUM(0x540000, 4), 0x10000000, 2));
        for (index = 0; index < 5; index++) {
            lbl_1_bss_194.effects[index] = fn_80025A04(
                fn_80008DD0((&lbl_1_data_108[5])[index].resourceNumber,
                                     0x10000000, 2));
            lbl_1_bss_194.positionsY[index] = (&lbl_1_data_108[5])[index].positionY;
            fn_80026760(lbl_1_bss_194.effects[index],
                            lbl_1_bss_194.positionsY[index],
                            0.0f, 0.0f);
            lbl_1_bss_194.motionValues[index] = (&lbl_1_data_108[5])[index].motionValue;
            fn_800269E8(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 1)));
            if (index == 2) {
                /* This middle effect is sampled at frame 16, then held paused. */
                fn_80026A44(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 1)));
                fn_800269E8(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 0)));
                fn_8002D414(lbl_1_bss_194.effects[index], 16.0f);
                fn_80026A44(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 0)));
                fn_800269E8(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 1)));
            }
        }
    }
    fn_800269E8(lbl_1_bss_194.baseModel, ((1 << 30) | (1 << 0)));
    fn_80026B64(lbl_1_bss_194.baseModel, 1);
    fn_80026B8C(lbl_1_bss_194.baseModel, 0);
    fn_1_1E80();
}

void fn_1_1BAC(void)
{
}

/* Called when an object splashes to project its position and add a water ripple. */
void fn_1_1BB0(Point3d *position)
{
    Point3d transformed;
    Point3d input;
    Point3d finalValue;
    Point3d *inputPointer;
    Point3d *finalPointer;

    input = *position;
    inputPointer = &input;
    fn_1_1398(&transformed, inputPointer, 1200.0f);
    *position = transformed;
    finalValue = *position;
    finalPointer = &finalValue;
    fn_1_1FEC(&finalPointer->x);
}

/* Called during player updates to toggle nearby water effects; it always returns 0.0f. */
f32 fn_1_1C58(f32 position, f32 direction)
{
    f32 difference;
    f32 result;
    f32 motionTime;
    s16 index;

    result = 0.0f;
    for (index = 0; index < 5; index++) {
        difference = lbl_1_bss_194.positionsY[index] - (position + direction);
        if (difference < 0.0f) {
            difference *= -1.0f;
        }

        motionTime = fn_8002D4E0(lbl_1_bss_194.effects[index]);
        if (difference < lbl_1_bss_194.motionValues[index] &&
            0.0f != direction) {
            fn_80026A44(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 1)));
            fn_800269E8(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 0)));
        } else if (motionTime <= 1.0f) {
            fn_80026A44(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 0)));
            fn_800269E8(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 1)));
        } else {
            fn_80026A44(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 1)));
            fn_800269E8(lbl_1_bss_194.effects[index], ((1 << 30) | (1 << 0)));
        }
    }
    return result;
}

/* Called by fn_1_15C8 to create the water model, material hook, and ripple process. */
void fn_1_1E80(void)
{
    s16 night;
    lbl_1_bss_0.model =
        fn_80025A04(fn_80008DD0(DATANUM(0x540000, 8), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_0.model, 6);
    fn_80027884(lbl_1_bss_0.model, fn_1_2528);
    lbl_1_bss_0.textureBuffer = HuMemDirectMallocNum(
        2, GXGetTexBufferSize(640U, 480U, 4U, 0U, 0U), 0x10000000);
    lbl_1_bss_0.anim0 =
        HuSprAnimRead(fn_80008DD0(DATANUM(0x210000, 10), 0x10000000, 2));
    night = lbl_802C0278;
    if (night == 0) {
        lbl_1_bss_0.anim1 =
            HuSprAnimRead(fn_80008DD0(DATANUM(0x540000, 14), 0x10000000, 2));
    } else {
        lbl_1_bss_0.anim1 =
            HuSprAnimRead(fn_80008DD0(DATANUM(0x540000, 15), 0x10000000, 2));
    }
    lbl_1_bss_0.value = 0.0f;
    fn_8000DAC0(fn_1_20AC, 100U, 12288U, 0, lbl_1_bss_810);
}

/* Called by splash handling to add the next ripple to one of the 32 free slots. */
void fn_1_1FEC(const f32 *wavePositionX)
{
    s16 index;

    index = 0;
    while (index < 32) {
        if (-100000.0f == lbl_1_bss_0.rippleX[index]) {
            lbl_1_bss_0.rippleX[index] = *wavePositionX;
            lbl_1_bss_0.amplitude[index] = 30.0f;
            lbl_1_bss_0.phase[index] = 0.0f;
            return;
        }
        index++;
    }
}

M625LightAsset lbl_1_data_108[10] = {
    { DATANUM(0x540000, 2), -460.0f, 150.0f }, { DATANUM(0x540000, 3), -270.0f, 50.0f },
    { DATANUM(0x540000, 1), 0.0f, 200.0f }, { DATANUM(0x540000, 3), 270.0f, 50.0f },
    { DATANUM(0x540000, 2), 460.0f, 150.0f }, { DATANUM(0x540000, 6), -460.0f, 150.0f },
    { DATANUM(0x540000, 7), -270.0f, 50.0f }, { DATANUM(0x540000, 5), 0.0f, 200.0f },
    { DATANUM(0x540000, 7), 270.0f, 50.0f }, { DATANUM(0x540000, 6), 460.0f, 150.0f }
};

M625LightPreset lbl_1_data_180[2] = {
    { 0, { 0.0f, 20.0f, 2000.0f }, { 0.0f, -0.01f, -1.0f }, { 255, 255, 255, 255 } },
    { 0, { 0.0f, -1500.0f, 1000.0f }, { 0.0f, 15.0f, -10.0f }, { 128, 128, 128, 255 } }
};

GXColor lbl_1_data_1C0 = { 232, 232, 232, 0 };

f32 lbl_1_data_1C4[2][3] = { { 0.03f, 0.0f, 0.0f }, { 0.0f, 0.03f, 0.0f } };

/* The child process started by fn_1_1E80 deforms peak vertices and flushes them each frame. */
void fn_1_20AC(void)
{
    HSF_OBJECT *object;
    s32 initialZero = 0;
    Vec objectPosition;
    f32 peakY;
    f32 phaseTime;
    f32 distance;
    f32 distanceAbs;
    s32 index;
    s32 peakListIndex;
    s32 vertexIndex;
    s32 peakCount;
    Vec **peakVertices;
    Vec *vertices;

    object = fn_80026BF4(lbl_1_bss_0.model, "wave2");
    Hu3DModelObjPosGet(lbl_1_bss_0.model, "wave2", &objectPosition);
    for (index = 0; index < 32; index += 1) {
        lbl_1_bss_0.rippleX[index] = -100000.0f;
    }
    DCInvalidateRange(object->mesh.vertex->data, object->mesh.vertex->count * 12);
    peakY = -100000.0f;
    vertices = (Vec *)object->mesh.vertex->data;
    for (index = 0; index < object->mesh.vertex->count; index += 1, vertices += 1) {
        if (peakY < vertices->y) {
            peakY = vertices->y;
        }
    }
    vertices = (Vec *)object->mesh.vertex->data;
    for (index = peakCount = 0; index < object->mesh.vertex->count; index += 1, vertices += 1) {
        if (peakY == vertices->y) {
            peakCount += 1;
        }
    }
    peakVertices = (Vec **)HuMemDirectMallocNum(HEAP_HEAP, peakCount * 4, 0x10000000);
    vertices = (Vec *)object->mesh.vertex->data;
    for (index = peakListIndex = 0; index < object->mesh.vertex->count; index += 1, vertices += 1) {
        if (peakY == vertices->y) {
            peakVertices[peakListIndex] = vertices;
            peakListIndex += 1;
        }
    }
    phaseTime = 0.0f;
    for (;;) {
        HuPrcVSleep();
        for (index = 0; index < peakCount; index += 1) {
            vertices = peakVertices[index];
            vertices->y = peakY + 5.0 *
                sin(3.141592653589793 *
                    (f64)(2.0f * vertices->x + phaseTime) / 180.0);
        }
        for (index = 0; index < 32; index += 1) {
            if (-100000.0f != lbl_1_bss_0.rippleX[index]) {
                for (vertexIndex = 0; vertexIndex < peakCount; vertexIndex += 1) {
                    vertices = peakVertices[vertexIndex];
                    distance = lbl_1_bss_0.rippleX[index] - vertices->x;
                    if (distance < 0.0f) {
                        distanceAbs = -distance;
                    } else {
                        distanceAbs = distance;
                    }
                    distance = distanceAbs;
                    if (!(distance < (lbl_1_bss_0.phase[index] - 50.0f)) &&
                        !(distance > (50.0f + lbl_1_bss_0.phase[index]))) {
                        distance -= lbl_1_bss_0.phase[index];
                        vertices->y = (f32)((f64)vertices->y +
                            ((f64)lbl_1_bss_0.amplitude[index] *
                            cos((3.141592653589793 *
                            (f64)(90.0f *
                            (distance / 50.0f))) /
                            180.0)));
                    }
                }
                lbl_1_bss_0.phase[index] += 2.0f;
                lbl_1_bss_0.amplitude[index] *= 0.99;
                if (lbl_1_bss_0.phase[index] > 500.0f) {
                    lbl_1_bss_0.rippleX[index] = -100000.0f;
                }
            }
        }
        DCFlushRangeNoSync(object->mesh.vertex->data, object->mesh.vertex->count * 12);
        phaseTime += 2.0f;
        lbl_1_bss_0.value += 0.0002;
    }
}

/* Called as the water model's material hook to copy the framebuffer and set projective textures. */
void fn_1_2528(HU3D_DRAW_OBJ *waterDrawObject, HSF_MATERIAL *material) {
    Mtx objectCameraMatrix;
    Mtx textureMatrix;
    Mtx lightProjectionMatrix;
    Mtx cameraInverseMatrix;
    GXColor color;
    HU3D_CAMERA *camera = &Hu3DCamera[lbl_802C01A6];

    color = lbl_1_data_1C0;
    fn_8002BCDC(0, 0, 640, 480, GX_TF_RGB565, 0, lbl_1_bss_0.textureBuffer);
    GXPixModeSync();
    fn_800360D8(lbl_1_bss_0.textureBuffer, 640, 480, 4, GX_CLAMP, GX_CLAMP, 1, GX_TEXMAP0);
    HuSprTexLoad(lbl_1_bss_0.anim0, 0, 1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    HuSprTexLoad(lbl_1_bss_0.anim1, 0, 2, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    C_MTXLightPerspective(lightProjectionMatrix, camera->fov, camera->aspect,
                          0.5f, -0.5f,
                          0.5f, 0.5f);
    PSMTXInverse(lbl_802613F4, cameraInverseMatrix);
    PSMTXConcat(cameraInverseMatrix, waterDrawObject->matrix, objectCameraMatrix);
    PSMTXConcat(lightProjectionMatrix, lbl_802613F4, textureMatrix);
    PSMTXConcat(textureMatrix, objectCameraMatrix, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD2, GX_TEXMAP2, GX_COLOR0);
    GXSetTevColor(GX_TEVREG0, color);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_C0, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    PSMTXIdentity(textureMatrix);
    mtxTransCat(textureMatrix, lbl_1_bss_0.value, 0.0f, 0.0f);
    mtxScaleCat(textureMatrix, 0.5f, 0.5f, 0.5f);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, lbl_1_data_1C4, -1);
    GXSetNumIndStages(1);
    GXSetNumTexGens(3);
    GXSetNumTevStages(2);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
}

M625LightModelState lbl_1_bss_194;
M625WaveState lbl_1_bss_0;
