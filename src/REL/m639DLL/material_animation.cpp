#include "game/sprput.h"
#include "REL/m639DLL/material_animation.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
extern "C" {
#include "dolphin/gx.h"
}

#include "dolphin/mtx.h"

extern "C" {
#include "game/data.h"
#include "game/sprite.h"
#include "game/hu3d.h"
}

#include "REL/m639DLL/game_constants.h"

void fn_1_BF94(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);

MaterialAnimation::MaterialAnimation(const Model *model) : GameObject(5)
{
    animationData = HuSprAnimRead(fn_80008DD0(DATANUM(0x620000, 34),
                                                              0x10000000, 2));
    textureFrame = 0.0f;
    HU3D_MODELID modelIndex = model->modelId;
    fn_80027884(modelIndex, fn_1_BF94);
}

MaterialAnimation::~MaterialAnimation() {}

void MaterialAnimation::update()
{
    textureFrame += framesToSeconds(0.5);
}

extern "C" const float lbl_1_rodata_3C8[2][3] = {
    {0.01f, 0.0f, 0.0f},
    {0.0f, 0.01f, 0.0f}
};

void fn_1_BF94(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    MaterialAnimation &animation = MaterialAnimation::getChecked();
    const float frame = animation.textureFrame;
    float matrix[3][4];
    PSMTXScale(matrix, 12.0f, 12.0f, 1.0f);
    PSMTXTransApply(matrix, matrix, frame / 10.0f,
                    -frame, 0.0f);
    GXLoadTexMtxImm(matrix, GX_TEXMTX0, GX_MTX2x4);
    GXSetNumTexGens(2);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0, 0, GX_PTIDENTITY);
    HuSprTexLoad(static_cast<ANIMDATA *>(animation.animationData), 0, GX_TEXMAP1,
                 GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetNumIndStages(1);
    GXSetIndTexMtx(GX_ITM_0, lbl_1_rodata_3C8, 0);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);

    GXColor color = {0x20, 0x20, 0x10, 0xBC};
    GXSetTevColor(GX_TEVREG0, color);
    GXSetNumTevStages(1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_A0, GX_CC_C0, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_COMP_BGR24_GT, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
}
