#include <dolphin/gx.h>
#include <dolphin/mtx.h>

#include "game/memory.h"
#include "game/process.h"

#define HU3D_CLUSTER_MAX 4
#define HU3D_MODEL_LLIGHT_MAX 8
#define HU3D_GLIGHT_MAX 8
#define HU3D_MOTATTR_PAUSE 0x40000002
#define HU3D_CLUSTER_ATTR_PAUSE ((s32)0xC0000002)
#define HU3D_ATTR_DISPOFF (1 << 0)

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_LIGHTID;

typedef struct HsfData_s HSF_DATA;
typedef struct HsfMaterial_s HSF_MATERIAL;
typedef struct HsfObject_s HSF_OBJECT;
typedef struct Hu3DDrawObj_s HU3D_DRAW_OBJ;
typedef struct Hu3DModel_s HU3D_MODEL;

typedef void (*HU3D_MODEL_HOOK)(HU3D_MODEL *model, Mtx *matrix);
typedef void (*HU3D_TIMING_HOOK)(
    HU3D_MODELID modelId, HU3D_MOTIONID motionId, BOOL lag);
typedef void (*HU3D_MAT_HOOK)(
    HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);

typedef struct Hu3DMotWork_s {
    float time;
    float speed;
    float start;
    float end;
} HU3D_MOTWORK;

struct Hu3DModel_s {
    u8 tick;
    u8 camInfoBit;
    u8 projBit;
    u8 hiliteIdx;
    s8 reflectType;
    u8 lightBit;
    s16 layerNo;
    HU3D_MOTIONID motId;
    HU3D_MOTIONID motIdOvl;
    HU3D_MOTIONID motIdShift;
    HU3D_MOTIONID motIdShape;
    HU3D_MOTIONID motIdCluster[HU3D_CLUSTER_MAX];
    s16 clusterAttr[HU3D_CLUSTER_MAX];
    HU3D_MOTIONID motIdSrc;
    u16 cameraBit;
    HU3D_MODELID linkMdlId;
    u16 lightNum;
    u16 lightId[HU3D_GLIGHT_MAX];
    HU3D_LIGHTID LLightId[HU3D_MODEL_LLIGHT_MAX];
    u32 mallocNo;
    u32 mallocNoLink;
    u32 attr;
    u32 motAttr;
    float ambR;
    float ambB;
    float ambG;
    HU3D_MOTWORK motWork;
    HU3D_MOTWORK motOvlWork;
    HU3D_MOTWORK motShiftWork;
    HU3D_MOTWORK motShapeWork;
    float clusterTime[HU3D_CLUSTER_MAX];
    float clusterSpeed[HU3D_CLUSTER_MAX];
    union {
        HSF_DATA *hsf;
        HU3D_MODEL_HOOK hookFunc;
    };
    HSF_DATA *hsfLink;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
    Mtx mtx;
    void *hookData;
    HU3D_TIMING_HOOK timingHook;
    HSF_OBJECT *timingHookObj;
    HU3D_MAT_HOOK matHook;
    u32 endCounter;
};

double sin(double value);
double cos(double value);

void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelClusterAttrSet(
    HU3D_MODELID modelId, s16 clusterNo, s32 clusterAttr);
void Hu3DCameraPosSet(int cameraBit, float posX, float posY, float posZ,
    float upX, float upY, float upZ, float targetX, float targetY,
    float targetZ);

extern HU3D_MODEL *Hu3DData;

typedef struct OpeningEventWork {
    void (*callback)(s32, s32);
    s32 arg0;
    s32 arg1;
} OpeningEventWork;

extern const float lbl_1_rodata_14;
extern const float lbl_1_rodata_34;
extern const float lbl_1_rodata_38;
extern const float lbl_1_rodata_74;
extern const float lbl_1_rodata_7C;
extern const float lbl_1_rodata_80;
extern const double lbl_1_rodata_98;
extern const double lbl_1_rodata_A8;
extern const double lbl_1_rodata_C8;
extern const double lbl_1_rodata_148;
extern const float lbl_1_rodata_150;
extern const float lbl_1_rodata_154;
extern const GXColor lbl_1_rodata_1C0;
extern const GXColor lbl_1_rodata_1C4;
extern const float lbl_1_rodata_1C8;
extern const float lbl_1_rodata_1CC;

extern float lbl_1_bss_38;
extern HuVecF lbl_1_bss_3C;
extern HuVecF lbl_1_bss_48;
extern HuVecF lbl_1_bss_54;
extern void *lbl_1_bss_1C0;
extern HU3D_MODELID lbl_1_bss_1CE[8];

void fn_1_4BD8(void)
{
    HuPrcSleep(120);
    Hu3DModelAttrSet(lbl_1_bss_1CE[1], HU3D_MOTATTR_PAUSE);
    Hu3DModelClusterAttrSet(lbl_1_bss_1CE[1], 0, HU3D_CLUSTER_ATTR_PAUSE);
}

void fn_1_4C30(void)
{
    HU3D_MODEL *model;
    float phase;

    model = &Hu3DData[lbl_1_bss_1CE[1]];
    phase = lbl_1_rodata_14;
    do {
        do {
            HuPrcVSleep();
        } while ((model->attr & HU3D_ATTR_DISPOFF) != 0);

        PSVECAdd(&lbl_1_bss_54, &lbl_1_bss_48, &model->pos);
        lbl_1_bss_48.x = lbl_1_bss_38 *
                         (lbl_1_rodata_148 * sin(lbl_1_rodata_98 * phase / lbl_1_rodata_A8));
        lbl_1_bss_48.y = lbl_1_bss_38 *
                         (lbl_1_rodata_C8 *
                          cos(lbl_1_rodata_98 * (lbl_1_rodata_34 * phase) / lbl_1_rodata_A8));
        model->rot = lbl_1_bss_3C;
        phase += lbl_1_rodata_38;
        if (phase > lbl_1_rodata_150) {
            phase -= lbl_1_rodata_150;
        }
    } while (TRUE);
}

void fn_1_4DB0(void)
{
    HUPROCESS *process;
    OpeningEventWork *work;

    process = HuPrcCurrentGet();
    work = process->property;
    work->callback(work->arg0, work->arg1);
    HuPrcEnd();
}

void fn_1_4E00(void)
{
    HUPROCESS *process;

    process = HuPrcCurrentGet();
    HuMemDirectFree(process->property);
}

void fn_1_4E34(void (*callback)(void), s32 arg0, s32 arg1)
{
    HUPROCESS *process;
    OpeningEventWork *work;

    process = HuPrcChildCreate(fn_1_4DB0, 0x100, 0x3000, 0, HuPrcCurrentGet());
    work = HuMemDirectMalloc(HEAP_HEAP, sizeof(OpeningEventWork));
    process->property = work;
    HuPrcDestructorSet2(process, fn_1_4E00);
    work->callback = (void (*)(s32, s32))callback;
    work->arg0 = arg0;
    work->arg1 = arg1;
}

void fn_1_4ECC(HuVecF *rotation, HuVecF *target, float distance)
{
    HuVecF position;
    HuVecF cameraTarget;
    HuVecF up;
    float x;
    float y;
    float z;

    x = rotation->x;
    y = rotation->y;
    z = rotation->z;

    position.x = target->x +
                 (distance *
                  (sin(lbl_1_rodata_98 * y / lbl_1_rodata_A8) *
                   cos(lbl_1_rodata_98 * x / lbl_1_rodata_A8)));
    position.y = target->y +
                 (distance * -sin(lbl_1_rodata_98 * x / lbl_1_rodata_A8));
    position.z = target->z +
                 (distance *
                  (cos(lbl_1_rodata_98 * y / lbl_1_rodata_A8) *
                   cos(lbl_1_rodata_98 * x / lbl_1_rodata_A8)));
    cameraTarget.x = target->x;
    cameraTarget.y = target->y;
    cameraTarget.z = target->z;
    up.x = sin(lbl_1_rodata_98 * y / lbl_1_rodata_A8) *
           sin(lbl_1_rodata_98 * x / lbl_1_rodata_A8);
    up.y = cos(lbl_1_rodata_98 * x / lbl_1_rodata_A8);
    up.z = cos(lbl_1_rodata_98 * y / lbl_1_rodata_A8) *
           sin(lbl_1_rodata_98 * x / lbl_1_rodata_A8);

    Hu3DCameraPosSet(1,
                     position.x,
                     position.y,
                     position.z,
                     up.x,
                     up.y,
                     up.z,
                     cameraTarget.x,
                     cameraTarget.y,
                     cameraTarget.z);
}

void fn_1_517C(float alpha)
{
    Mtx44 projection;
    Mtx modelview;
    GXTexObj texture;
    GXColor color1 = lbl_1_rodata_1C0;
    GXColor color2 = lbl_1_rodata_1C4;

    C_MTXOrtho(projection, lbl_1_rodata_14, lbl_1_rodata_80, lbl_1_rodata_14,
               lbl_1_rodata_7C, lbl_1_rodata_14, lbl_1_rodata_74);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    PSMTXIdentity(modelview);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    color1.a = lbl_1_rodata_1C8 * alpha;
    GXSetTevColor(GX_TEVREG0, color1);
    GXSetTevColor(GX_TEVREG1, color2);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_CPREV, GX_CC_C1, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_A0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetNumTevStages(2);
    GXInitTexObj(&texture, lbl_1_bss_1C0, 0x280, 0x140, GX_TF_I8, GX_CLAMP, GX_CLAMP,
                 GX_FALSE);
    GXInitTexObjLOD(&texture, GX_NEAR, GX_NEAR, lbl_1_rodata_14, lbl_1_rodata_14,
                    lbl_1_rodata_14, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texture, GX_TEXMAP0);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2f32(lbl_1_rodata_14, lbl_1_rodata_154);
    GXTexCoord2f32(lbl_1_rodata_14, lbl_1_rodata_14);
    GXPosition2f32(lbl_1_rodata_7C, lbl_1_rodata_154);
    GXTexCoord2f32(lbl_1_rodata_38, lbl_1_rodata_14);
    GXPosition2f32(lbl_1_rodata_7C, lbl_1_rodata_1CC);
    GXTexCoord2f32(lbl_1_rodata_38, lbl_1_rodata_38);
    GXPosition2f32(lbl_1_rodata_14, lbl_1_rodata_1CC);
    GXTexCoord2f32(lbl_1_rodata_14, lbl_1_rodata_38);
    GXEnd();
}
