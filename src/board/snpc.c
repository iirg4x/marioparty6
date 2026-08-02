#define _MATH_H
#include "dolphin/math.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"

#include "game/data.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/sprite.h"

#include "humath.h"

#define SNPC_MAGIC 'SNPC'
#define MBOBJ_FADE_WORK_MAGIC 'MBTV'
#define MBOBJ_METAL_WORK_MAGIC 'TV01'
#define MBOBJ_BIRIQ_WORK_MAGIC 'TV02'

#define SNPC_DATA_FADE_TEXTURE DATANUM(DATA_board, 103)
#define SNPC_DATA_METAL_TEXMAP4 DATANUM(DATA_board, 105)
#define SNPC_DATA_METAL_TEXMAP5 DATANUM(DATA_board, 104)

typedef struct MBSNPCSAVEWORK {
    u8 flags;
    u8 masuId;
    u8 effectMissCount;
} MBSNPCSAVEWORK;

typedef struct MBSNPCWORK MBSNPCWORK;

typedef struct MBOBJFADEWORK {
    u32 magic;
    ANIMDATA *anim;
    HuVecF pos;
    HuVecF rot;
    float alpha;
    GXColor color;
} MBOBJFADEWORK;

typedef struct MBOBJMETALWORK {
    u32 magic;
    ANIMDATA *anim[2];
    float tpLvl;
    GXColor shadowColor;
    GXColor hiliteColor;
} MBOBJMETALWORK;

typedef struct MBOBJBIRIQWORK {
    u32 magic;
    int mode;
    float level;
    GXColor color;
} MBOBJBIRIQWORK;

typedef struct MBOBJBIRIQTEV {
    u8 op;
    u8 outReg;
    u8 input[4];
} MBOBJBIRIQTEV;

static GXTevKColorSel kColorTbl[8] = {
    GX_TEV_KCSEL_8_8,
    GX_TEV_KCSEL_7_8,
    GX_TEV_KCSEL_6_8,
    GX_TEV_KCSEL_5_8,
    GX_TEV_KCSEL_4_8,
    GX_TEV_KCSEL_3_8,
    GX_TEV_KCSEL_2_8,
    GX_TEV_KCSEL_1_8,
};

static int biriQMatNumTbl[4] = {
    2,
    1,
    1,
    2,
};

static MBOBJBIRIQTEV biriQMatTbl[4][2][2] = {
    {
        { { GX_TEV_ADD, GX_TEVREG0, { GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST } },
            { GX_TEV_ADD, GX_TEVREG0, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST } } },
        { { GX_TEV_ADD, GX_TEVPREV, { GX_CC_CPREV, GX_CC_C0, GX_CC_A0, GX_CC_ZERO } },
            { GX_TEV_ADD, GX_TEVPREV, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV } } },
    },
    {
        { { GX_TEV_ADD, GX_TEVPREV, { GX_CC_KONST, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV } },
            { GX_TEV_ADD, GX_TEVPREV, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV } } },
        { { GX_TEV_ADD, GX_TEVPREV, { GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV } },
            { GX_TEV_ADD, GX_TEVPREV, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV } } },
    },
    {
        { { GX_TEV_SUB, GX_TEVPREV, { GX_CC_KONST, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV } },
            { GX_TEV_ADD, GX_TEVPREV, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV } } },
        { { GX_TEV_ADD, GX_TEVPREV, { GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV } },
            { GX_TEV_ADD, GX_TEVPREV, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV } } },
    },
    {
        { { GX_TEV_SUB, GX_TEVREG0, { GX_CC_CPREV, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE } },
            { GX_TEV_ADD, GX_TEVREG0, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST } } },
        { { GX_TEV_ADD, GX_TEVPREV, { GX_CC_CPREV, GX_CC_C0, GX_CC_A0, GX_CC_ZERO } },
            { GX_TEV_ADD, GX_TEVPREV, { GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV } } },
    },
};

static GXColor texCol[16];

static u32 snpcMagic;
static MBSNPCSAVEWORK *snpcSaveWork;
static MBSNPCWORK *snpcWork;

static void SNpcStarFunc(void);
static void GetStarTexTevStage(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material,
    int *tevStageNum, int *texGenNum);
static void GetStarNoTexTevStage(HU3D_DRAW_OBJ *drawObj,
    HSF_MATERIAL *material, int *tevStageNum, int *texGenNum);
static void FadeMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
static void MetalMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
static void BiriQMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);

extern void *mbMallocNum(s32 size, u32 num);
extern void mbMtxRot(Mtx mtx, float x, float y, float z);
extern const float lbl_802C3290;
extern const float lbl_802C32EC;

void mbSNpcInit(void)
{
    snpcMagic = 0;
    snpcSaveWork = NULL;
    snpcWork = NULL;
}

int mbSNpcMasuGet(void)
{
    if (snpcMagic != SNPC_MAGIC) {
        return 0;
    }
    return snpcSaveWork->masuId;
}

static void SNpcStarFunc(void)
{
}

void mbMasuChanceKill(void *work)
{
    HuMemDirectFree(work);
}

void mbMasuChanceTypeSet(u8 *chanceTbl, u8 value, int *typeTbl, BOOL inverseF)
{
    int masuNum;
    int masuType;
    BOOL inverseWork;
    int i;
    u8 *chanceTblP;
    int typeNo;

    masuNum = mbMasuNumGet();
    inverseWork = inverseF ? TRUE : FALSE;
    chanceTblP = chanceTbl + 1;
    for (i = 1; i < masuNum; i++, chanceTblP++) {
        if (*chanceTblP == 0) {
            masuType = mbMasuGet(i)->type;
            for (typeNo = 0; typeTbl[typeNo] >= 0; typeNo++) {
                if (masuType == typeTbl[typeNo]) {
                    break;
                }
            }
            if (inverseWork == (typeTbl[typeNo] < 0)) {
                *chanceTblP = (u8)value;
            }
        }
    }
}

void mbMasuChanceFlagSet(u8 *chanceTbl, u8 value, u32 flag, u32 mAttr,
    BOOL inverseF)
{
    u8 *chanceTblP;
    int masuNum;
    BOOL inverseWork;
    int i;
    MASU *masu;

    masuNum = mbMasuNumGet();
    inverseWork = inverseF ? TRUE : FALSE;
    chanceTblP = chanceTbl + 1;
    for (i = 1; i < masuNum; i++, chanceTblP++) {
        if (*chanceTblP == 0) {
            masu = mbMasuGet(i);
            if (inverseWork == (((masu->flag & flag) | (masu->mAttr & mAttr)) == 0)) {
                *chanceTblP = (u8)value;
            }
        }
    }
}

void mbMasuChancePlayerSet(u8 *chanceTbl, int value)
{
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        chanceTbl[GwPlayer[i].masuId] = value;
    }
}

static void GetStarTexTevStage(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material,
    int *tevStageNum, int *texGenNum)
{
    HSF_CONSTDATA *constData;
    HU3D_ATTR_ANIM *animWork;
    HSF_ATTRIBUTE *attribute;
    HSF_BITMAP *bitmap;
    HSF_OBJECT *object;
    HU3D_MODEL *model;
    u32 flags;
    u16 matHiliteF;
    u16 lightOnF;
    u16 projMask;
    u16 i;
    int specialAttrNo;
    int bumpAttrNo;
    BOOL texBlendF;
    BOOL shineF;
    int tevStage;
    int texGen;

    specialAttrNo = -1;
    object = drawObj->object;
    model = drawObj->model;
    for (i = 0; i < material->attrNum; i++) {
        attribute = &object->mesh.attribute[material->attr[i]];
        bitmap = attribute->bitmap;
        texCol[i].a = 0;
        if (attribute->animWorkP) {
            animWork = attribute->animWorkP;
            if ((animWork->attr & HU3D_ATTRANIM_ATTR_ANIM2D)
                && !(Hu3DTexAnimData[animWork->animId].attr & HU3D_ANIM_ATTR_NOUSE)) {
                continue;
            }
            if (animWork->attr & HU3D_ATTRANIM_ATTR_BMPANIM) {
                bitmap = animWork->bitMapPtr;
            }
        }
        switch (bitmap->dataFmt) {
            case HSF_BMPFMT_I4:
            case HSF_BMPFMT_I8:
            case HSF_BMPFMT_IA4:
            case HSF_BMPFMT_IA8:
                texCol[i].a = 1;
                break;
            case HSF_BMPFMT_CI_IA8:
                texCol[i].a = 2;
                break;
        }
    }
    flags = object->flags | material->flags;
    if (material->vtxMode == 2 || material->vtxMode == 3) {
        matHiliteF = TRUE;
    } else {
        matHiliteF = FALSE;
        if (material->vtxMode == 0 || material->vtxMode == 5) {
            lightOnF = FALSE;
        } else {
            lightOnF = TRUE;
        }
    }
    shineF = Hu3DShineF && lightOnF;
    constData = object->constData;
    if (material->attrNum == 1) {
        tevStage = 1;
        texGen = 1;
        attribute = &object->mesh.attribute[material->attr[0]];
        if (attribute->unk20 == 1.0f) {
            if (attribute->unk8[2] == 0) {
                tevStage++;
            } else if (!(model->attr & HU3D_ATTR_TOON_MAP)
                && (texCol[0].a == 1 || texCol[0].a == 2)) {
                tevStage++;
            }
        }
        if (model->attr & HU3D_ATTR_TOON_MAP) {
            texGen++;
            tevStage++;
        }
        if (material->refAlpha != 0.0f) {
            texGen++;
            tevStage++;
        }
        if (shineF) {
            tevStage++;
        }
        if (Hu3DShadowF && shadowNum
            && (constData->attr & HU3D_CONST_SHADOW_MAP)) {
            if (constData->attr & HU3D_CONST_SHADOW_MAP_TPLVL) {
                tevStage++;
            }
            texGen++;
            tevStage++;
        }
        if (matHiliteF) {
            if ((model->attr & HU3D_ATTR_HILITE)
                || (flags & HSF_MATERIAL_HILITE)) {
                texGen++;
                tevStage++;
                matHiliteF = FALSE;
            } else {
                if (attribute->unk20 != 1.0f) {
                    texGen++;
                }
                tevStage++;
            }
        } else if (material->invAlpha != 0.0f) {
            tevStage++;
        }
        for (i = 0, projMask = 1; i < 4; i++, projMask <<= 1) {
            if (model->projBit & projMask) {
                texGen++;
                tevStage += 2;
            }
        }
    } else {
        texBlendF = FALSE;
        texGen = 0;
        bumpAttrNo = -1;
        tevStage = 0;
        for (i = 0; i < material->attrNum; i++) {
            attribute = &object->mesh.attribute[material->attr[i]];
            if (attribute->nbtTpLvl != 0.0f) {
                tevStage++;
                bumpAttrNo = i;
                texGen++;
                texBlendF = TRUE;
                continue;
            }
            if (attribute->unk20 != 1.0f) {
                specialAttrNo = i;
                continue;
            }
            texGen++;
            if (i == 0) {
                if (texCol[i].a == 1 || texCol[i].a == 2) {
                    tevStage++;
                }
            } else if (texBlendF) {
                texBlendF = FALSE;
            } else if (attribute->unk8[2] == 0) {
                tevStage++;
            } else if (texCol[i].a == 1 || texCol[i].a == 2) {
                tevStage++;
            }
            tevStage++;
        }
        if (model->attr & HU3D_ATTR_TOON_MAP) {
            texGen++;
            tevStage++;
        }
        if (material->refAlpha != 0.0f) {
            if (specialAttrNo != -1) {
                texGen++;
                tevStage++;
            }
            texGen++;
            tevStage++;
        }
        if (shineF) {
            tevStage++;
        }
        if (Hu3DShadowF && shadowNum
            && (constData->attr & HU3D_CONST_SHADOW_MAP)) {
            if (constData->attr & HU3D_CONST_SHADOW_MAP_TPLVL) {
                tevStage++;
            }
            texGen++;
            tevStage++;
        }
        if (matHiliteF) {
            if ((model->attr & HU3D_ATTR_HILITE)
                || (flags & HSF_MATERIAL_HILITE)) {
                if (specialAttrNo != -1) {
                    texGen++;
                    tevStage++;
                }
                texGen++;
                tevStage++;
                matHiliteF = FALSE;
            } else {
                if (specialAttrNo != -1) {
                    texGen++;
                }
                tevStage++;
            }
        } else if (material->invAlpha != 0.0f) {
            tevStage++;
        }
        for (i = 0, projMask = 1; i < 4; i++, projMask <<= 1) {
            if (model->projBit & projMask) {
                texGen++;
                tevStage += 2;
            }
        }
        if (bumpAttrNo != -1) {
            texGen++;
        }
    }
    *tevStageNum = (u16)tevStage;
    *texGenNum = (u16)texGen;
}

static void GetStarNoTexTevStage(HU3D_DRAW_OBJ *drawObj,
    HSF_MATERIAL *material, int *tevStageNum, int *texGenNum)
{
    HSF_CONSTDATA *constData;
    HSF_OBJECT *object;
    HU3D_MODEL *model;
    u32 flags;
    s16 matHiliteF;
    s16 projMask;
    int tevStage;
    int texGen;
    int i;

    tevStage = 1;
    texGen = 0;
    object = drawObj->object;
    model = drawObj->model;
    flags = object->flags | material->flags;
    matHiliteF = material->vtxMode == 2 || material->vtxMode == 3;
    if (model->attr & HU3D_ATTR_TOON_MAP) {
        texGen++;
    }
    if (material->refAlpha != 0.0f) {
        tevStage++;
        texGen++;
    }
    constData = object->constData;
    if (Hu3DShadowF && shadowNum
        && (constData->attr & HU3D_CONST_SHADOW_MAP)) {
        if (constData->attr & HU3D_CONST_SHADOW_MAP_TPLVL) {
            tevStage++;
        }
        tevStage++;
        texGen++;
    }
    if (matHiliteF) {
        if ((model->attr & HU3D_ATTR_HILITE)
            || (flags & HSF_MATERIAL_HILITE)) {
            texGen++;
        }
        tevStage++;
    } else if (material->invAlpha != 0.0f) {
        tevStage++;
    }
    for (i = 0, projMask = 1; i < 4; i++, projMask <<= 1) {
        if (model->projBit & projMask) {
            texGen++;
            tevStage += 2;
        }
    }
    *tevStageNum = (s16)tevStage;
    *texGenNum = (s16)texGen;
}

void mbObjStarTevStageSet(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material,
    int *tevStageNum, int *texGenNum)
{
    if (material->attrNum == 0) {
        Hu3DTevStageNoTexSet(drawObj, material);
        GetStarNoTexTevStage(drawObj, material, tevStageNum, texGenNum);
    } else {
        Hu3DTevStageTexSet(drawObj, material);
        GetStarTexTevStage(drawObj, material, tevStageNum, texGenNum);
    }
}

void mbObjFadeCreate(MBMODELID modelId, HuVecF *pos)
{
    int hu3DModelId;
    MBOBJFADEWORK *work;
    HSF_DATA *hsf;
    HU3D_MODEL *model;
    int i;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    if (model->attr & HU3D_ATTR_LINK) {
        work = mbMallocNum(sizeof(*work), model->mallocNoLink);
        model->hookData = work;
    } else {
        work = mbMallocNum(sizeof(*work), model->mallocNo);
        model->hookData = work;
    }
    work->magic = MBOBJ_FADE_WORK_MAGIC;
    work->pos = *pos;
    work->alpha = lbl_802C32EC;
    work->color.r = work->color.g = work->color.b = 255;
    Hu3DModelMatHookSet(hu3DModelId, FadeMatHook);
    hsf = model->hsf;
    for (i = 0; i < hsf->materialNum; i++) {
        hsf->material[i].flags |= HSF_MATERIAL_MATHOOK;
    }
    work->anim = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(SNPC_DATA_FADE_TEXTURE), HU_MEMNUM_OVL, HEAP_MODEL));
}

void mbObjFadeKill(MBMODELID modelId)
{
    int hu3DModelId;
    MBOBJFADEWORK *work;
    HSF_DATA *hsf;
    HU3D_MODEL *model;
    int i;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    work = model->hookData;
    Hu3DModelMatHookSet(hu3DModelId, NULL);
    hsf = model->hsf;
    for (i = 0; i < hsf->materialNum; i++) {
        hsf->material[i].flags &= ~HSF_MATERIAL_MATHOOK;
    }
    HuSprAnimKill(work->anim);
    HuMemDirectFree(work);
    model->hookData = NULL;
}

static void FadeMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    MBOBJFADEWORK *work;
    Mtx texMtx;
    Mtx workMtx;
    float alpha;
    int tevStage;
    int texGen;

    work = drawObj->model->hookData;
    if (material->attrNum == 0) {
        Hu3DTevStageNoTexSet(drawObj, material);
        GetStarNoTexTevStage(drawObj, material, &tevStage, &texGen);
    } else {
        Hu3DTevStageTexSet(drawObj, material);
        GetStarTexTevStage(drawObj, material, &tevStage, &texGen);
    }
    HuSprTexLoad(work->anim, 0, GX_TEXMAP4, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    PSMTXInverse(Hu3DCameraMtx, texMtx);
    PSMTXConcat(texMtx, drawObj->matrix, texMtx);
    PSMTXTrans(workMtx, -work->pos.x, -work->pos.y, -work->pos.z);
    PSMTXConcat(workMtx, texMtx, texMtx);
    mbMtxRot(workMtx, work->rot.x, work->rot.y, work->rot.z);
    PSMTXInverse(workMtx, workMtx);
    PSMTXConcat(workMtx, texMtx, texMtx);
    alpha = work->alpha;
    if (alpha < 0.000001f) {
        alpha = 0.000001f;
    }
    PSMTXScale(workMtx, 0.001f, -0.01f / alpha, 1.0f);
    PSMTXConcat(workMtx, texMtx, texMtx);
    texMtx[1][3] += 0.96875f;
    GXLoadTexMtxImm(texMtx, GX_TEXMTX4, GX_MTX2x4);
    GXSetNumTexGens(texGen + 1);
    GXSetNumTevStages(tevStage + 1);
    GXSetTexCoordGen2(texGen, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX4,
        GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(tevStage, texGen, GX_TEXMAP4, GX_COLOR_NULL);
    GXSetTevKColor(GX_KCOLOR3, work->color);
    GXSetTevKColorSel(tevStage, GX_TEV_KCSEL_K3);
    GXSetTevColorIn(tevStage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST,
        GX_CC_ZERO);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV,
        GX_CA_ZERO);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    if ((drawObj->object->flags | material->flags)
        & (HSF_MATERIAL_NEAR | HSF_MATERIAL_DISABLE_ZWRITE)) {
        GXSetAlphaCompare(GX_GREATER, 128, GX_AOP_OR, GX_GREATER, 128);
    } else {
        GXSetAlphaCompare(GX_GREATER, 1, GX_AOP_AND, GX_GREATER, 1);
    }
}

void mbObjFadeTexRotSet(MBMODELID modelId, HuVecF *pos, HuVecF *rot)
{
    int hu3DModelId;
    MBOBJFADEWORK *work;
    HU3D_MODEL *model;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    work = model->hookData;
    work->pos = *pos;
    work->rot = *rot;
}

void mbObjFadeTexColorSet(MBMODELID modelId, u8 r, u8 g, u8 b, float alpha)
{
    int hu3DModelId;
    MBOBJFADEWORK *work;
    HU3D_MODEL *model;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    work = model->hookData;
    work->color.r = (int)r;
    work->color.g = (int)g;
    work->color.b = (int)b;
    work->alpha = alpha;
}

void mbObjMetalCreate(MBMODELID modelId)
{
    int hu3DModelId;
    MBOBJMETALWORK *work;
    HSF_DATA *hsf;
    HU3D_MODEL *model;
    int i;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    if (model->attr & HU3D_ATTR_LINK) {
        work = mbMallocNum(sizeof(*work), model->mallocNoLink);
        model->hookData = work;
    } else {
        work = mbMallocNum(sizeof(*work), model->mallocNo);
        model->hookData = work;
    }
    work->magic = MBOBJ_METAL_WORK_MAGIC;
    work->tpLvl = lbl_802C32EC;
    work->shadowColor.r = work->shadowColor.g = work->shadowColor.b = 255;
    work->hiliteColor.r = work->hiliteColor.g = work->hiliteColor.b = 255;
    work->shadowColor.r = 129;
    work->shadowColor.g = 255;
    work->shadowColor.b = 174;
    work->hiliteColor.r = 202;
    work->hiliteColor.g = 87;
    work->hiliteColor.b = 255;
    Hu3DModelMatHookSet(hu3DModelId, MetalMatHook);
    hsf = model->hsf;
    for (i = 0; i < hsf->materialNum; i++) {
        hsf->material[i].flags |= HSF_MATERIAL_MATHOOK;
    }
    work->anim[0] = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(SNPC_DATA_METAL_TEXMAP4), HU_MEMNUM_OVL, HEAP_MODEL));
    work->anim[1] = HuSprAnimRead(HuDataSelHeapReadNum(
        mbBoardDataNumGet(SNPC_DATA_METAL_TEXMAP5), HU_MEMNUM_OVL, HEAP_MODEL));
}

BOOL mbObjMetalKill(MBMODELID modelId)
{
    int hu3DModelId;
    MBOBJMETALWORK *work;
    HSF_DATA *hsf;
    HU3D_MODEL *model;
    int i;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    if (model->hookData == NULL) {
        return FALSE;
    }
    work = model->hookData;
    if (work->magic != MBOBJ_METAL_WORK_MAGIC) {
        return FALSE;
    }
    Hu3DModelMatHookSet(hu3DModelId, NULL);
    hsf = model->hsf;
    for (i = 0; i < hsf->materialNum; i++) {
        hsf->material[i].flags &= ~HSF_MATERIAL_MATHOOK;
    }
    HuSprAnimKill(work->anim[0]);
    HuSprAnimKill(work->anim[1]);
    HuMemDirectFree(work);
    model->hookData = NULL;
    return TRUE;
}

void mbObjMetalTPLvlSet(MBMODELID modelId, float tpLvl)
{
    int hu3DModelId;
    MBOBJMETALWORK *work;
    HU3D_MODEL *model;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    work = model->hookData;
    work->tpLvl = tpLvl;
}

void mbObjMetalColorSet(MBMODELID modelId, GXColor shadowColor,
    GXColor hiliteColor)
{
    int hu3DModelId;
    MBOBJMETALWORK *work;
    HU3D_MODEL *model;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    work = model->hookData;
    work->shadowColor = shadowColor;
    work->hiliteColor = hiliteColor;
}

static void MetalMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    MBOBJMETALWORK *work;
    HuVecF viewDir = { 0.0f, 0.0f, -1.0f };
    HuVecF lightDir;
    HuVecF axis;
    Mtx texMtx;
    Mtx workMtx;
    GXColor color;
    float angle;
    int tevStage;
    int texGen;

    work = drawObj->model->hookData;
    if (material->attrNum == 0) {
        Hu3DTevStageNoTexSet(drawObj, material);
        GetStarNoTexTevStage(drawObj, material, &tevStage, &texGen);
    } else {
        Hu3DTevStageTexSet(drawObj, material);
        GetStarTexTevStage(drawObj, material, &tevStage, &texGen);
    }
    if (work->tpLvl <= 0.05f) {
        return;
    }
    HuSprTexLoad(work->anim[0], 0, GX_TEXMAP4, GX_REPEAT, GX_REPEAT,
        GX_LINEAR);
    HuSprTexLoad(work->anim[1], 0, GX_TEXMAP5, GX_REPEAT, GX_REPEAT,
        GX_LINEAR);
    PSMTXCopy(drawObj->matrix, texMtx);
    PSMTXScale(workMtx, 0.5f / drawObj->scale.x,
        -0.5f / drawObj->scale.y, 0.5f / drawObj->scale.z);
    PSMTXConcat(workMtx, texMtx, texMtx);
    texMtx[0][3] = 0.5f;
    texMtx[1][3] = 0.5f;
    GXLoadTexMtxImm(texMtx, GX_TEXMTX4, GX_MTX2x4);
    PSMTXCopy(drawObj->matrix, texMtx);
    PSMTXMultVecSR(Hu3DCameraMtx, &Hu3DGlobalLight[0].dir, &lightDir);
    C_VECHalfAngle(&viewDir, &lightDir, &lightDir);
    if (fabsf(lightDir.z) < 0.999f) {
        angle = (float)acos(lightDir.z);
        PSVECCrossProduct(&viewDir, &lightDir, &axis);
        PSMTXRotAxisRad(workMtx, &axis, angle);
        PSMTXConcat(workMtx, texMtx, texMtx);
    }
    PSMTXScale(workMtx, 0.5f / drawObj->scale.x,
        -0.5f / drawObj->scale.y, 0.5f / drawObj->scale.z);
    PSMTXConcat(workMtx, texMtx, texMtx);
    texMtx[0][3] = 0.5f;
    texMtx[1][3] = 0.5f;
    GXLoadTexMtxImm(texMtx, GX_TEXMTX5, GX_MTX2x4);
    GXSetNumTexGens(texGen + 2);
    GXSetNumTevStages(tevStage + 3);
    GXSetTexCoordGen2(texGen, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX4,
        GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(texGen + 1, GX_TG_MTX2x4, GX_TG_NRM, GX_TEXMTX5,
        GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(tevStage, texGen, GX_TEXMAP4, GX_COLOR_NULL);
    GXSetTevKColorSel(tevStage,
        kColorTbl[(int)(7.9f * (1.0f - work->tpLvl))]);
    GXSetTevColorIn(tevStage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST,
        GX_CC_ZERO);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_APREV);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    tevStage++;
    GXSetTevOrder(tevStage, texGen + 1, GX_TEXMAP5, GX_COLOR_NULL);
    color.r = work->shadowColor.r * work->tpLvl;
    color.g = work->shadowColor.g * work->tpLvl;
    color.b = work->shadowColor.b * work->tpLvl;
    GXSetTevKColor(GX_KCOLOR2, color);
    GXSetTevKColorSel(tevStage, GX_TEV_KCSEL_K2);
    GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST,
        GX_CC_CPREV);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_TEXA);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVREG0);
    tevStage++;
    GXSetTevOrder(tevStage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
    color.r = work->hiliteColor.r * work->tpLvl;
    color.g = work->hiliteColor.g * work->tpLvl;
    color.b = work->hiliteColor.b * work->tpLvl;
    GXSetTevKColor(GX_KCOLOR3, color);
    GXSetTevKColorSel(tevStage, GX_TEV_KCSEL_K3);
    GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_A0, GX_CC_KONST,
        GX_CC_CPREV);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
        GX_CA_APREV);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
}

void mbObjBiriQCreate(MBMODELID modelId)
{
    int hu3DModelId;
    MBOBJBIRIQWORK *work;
    HSF_DATA *hsf;
    HU3D_MODEL *model;
    int i;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    if (model->attr & HU3D_ATTR_LINK) {
        work = mbMallocNum(sizeof(*work), model->mallocNoLink);
        model->hookData = work;
    } else {
        work = mbMallocNum(sizeof(*work), model->mallocNo);
        model->hookData = work;
    }
    work->magic = MBOBJ_BIRIQ_WORK_MAGIC;
    work->level = lbl_802C3290;
    work->color.r = work->color.g = work->color.b = work->color.a = 255;
    Hu3DModelMatHookSet(hu3DModelId, BiriQMatHook);
    hsf = model->hsf;
    for (i = 0; i < hsf->materialNum; i++) {
        hsf->material[i].flags |= HSF_MATERIAL_MATHOOK;
    }
}

BOOL mbObjBiriQKill(MBMODELID modelId)
{
    int hu3DModelId;
    MBOBJBIRIQWORK *work;
    HSF_DATA *hsf;
    HU3D_MODEL *model;
    int i;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    if (model->hookData == NULL) {
        return FALSE;
    }
    work = model->hookData;
    if (work->magic != MBOBJ_BIRIQ_WORK_MAGIC) {
        return FALSE;
    }
    Hu3DModelMatHookSet(hu3DModelId, NULL);
    hsf = model->hsf;
    for (i = 0; i < hsf->materialNum; i++) {
        hsf->material[i].flags &= ~HSF_MATERIAL_MATHOOK;
    }
    HuMemDirectFree(work);
    model->hookData = NULL;
    return TRUE;
}

void mbObjBiriQColorSet(MBMODELID modelId, BOOL mode, float level,
    GXColor color)
{
    int hu3DModelId;
    MBOBJBIRIQWORK *work;
    HU3D_MODEL *model;

    hu3DModelId = mbObjModelIDGet((int)modelId);
    model = &Hu3DData[hu3DModelId];
    if (model->hookData == NULL) {
        return;
    }
    work = model->hookData;
    if (work->magic != MBOBJ_BIRIQ_WORK_MAGIC) {
        return;
    }
    work->mode = mode;
    work->level = level;
    work->color = color;
}

static void BiriQMatHook(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    MBOBJBIRIQWORK *work;
    MBOBJBIRIQTEV *tevConfig;
    GXColor color;
    int tevStage;
    int texGen;
    int i;

    work = drawObj->model->hookData;
    if (material->attrNum == 0) {
        Hu3DTevStageNoTexSet(drawObj, material);
        GetStarNoTexTevStage(drawObj, material, &tevStage, &texGen);
    } else {
        Hu3DTevStageTexSet(drawObj, material);
        GetStarTexTevStage(drawObj, material, &tevStage, &texGen);
    }
    if (work->level <= 0.01f) {
        return;
    }
    GXSetNumTevStages(tevStage + biriQMatNumTbl[work->mode]);
    switch (work->mode) {
        case 0:
        case 3:
            color.r = work->color.r;
            color.g = work->color.g;
            color.b = work->color.b;
            break;
        case 1:
            color.r = work->color.r * work->level;
            color.g = work->color.g * work->level;
            color.b = work->color.b * work->level;
            break;
    }
    color.a = 255.0f * work->level;
    GXSetTevKColor(GX_KCOLOR3, color);
    GXSetTevKColorSel(tevStage, GX_TEV_KCSEL_K3);
    GXSetTevKAlphaSel(tevStage, GX_TEV_KASEL_K3_A);
    tevConfig = &biriQMatTbl[work->mode][0][0];
    for (i = 0; i < biriQMatNumTbl[work->mode]; i++, tevStage++) {
        GXSetTevOrder(tevStage, GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
            GX_COLOR_NULL);
        GXSetTevColorOp(tevStage, tevConfig->op, GX_TB_ZERO,
            GX_CS_SCALE_1, GX_TRUE, tevConfig->outReg);
        GXSetTevColorIn(tevStage, tevConfig->input[0], tevConfig->input[1],
            tevConfig->input[2], tevConfig->input[3]);
        tevConfig++;
        GXSetTevAlphaOp(tevStage, tevConfig->op, GX_TB_ZERO,
            GX_CS_SCALE_1, GX_TRUE, tevConfig->outReg);
        GXSetTevAlphaIn(tevStage, tevConfig->input[0], tevConfig->input[1],
            tevConfig->input[2], tevConfig->input[3]);
        tevConfig++;
    }
}
