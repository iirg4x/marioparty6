#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "game/animdata.h"
#include "game/sprite.h"

#include <math.h>

#include "REL/framework/game_object2.h"

extern "C" {
#include "game/sprput.h"
}

struct Sprite : RenderObject {
    ANIMDATA *animation;
    f32 transformValues[6]; // The game never reads these values.
    u8 unknown_030[4]; /* No code names this field. */
    f32 textureS[4];
    f32 textureT[4];
    explicit Sprite(ANIMDATA *);
    virtual ~Sprite();
    virtual void update();
    virtual void draw(Mtx *);
};

void fn_1_C2EC(Sprite *);
void fn_1_C370(Sprite *);

Sprite::Sprite(ANIMDATA *animation) : RenderObject(3)
{
    this->animation = animation;
    setLayer(3);
    transformValues[0] = 0.0f;
    transformValues[1] = 0.0f;
    transformValues[2] = 0.0f;
    transformValues[3] = 0.0f;
    transformValues[4] = 0.0f;
    transformValues[5] = 0.0f;
    textureS[0] = 0.0f;
    textureT[0] = 0.0f;
    textureS[1] = 0.0f;
    textureT[1] = 1.0f;
    textureS[2] = 1.0f;
    textureT[2] = 1.0f;
    textureS[3] = 1.0f;
    textureT[3] = 0.0f;
}

void Sprite::update()
{
    fn_1_C370(this);
}

void Sprite::draw(Mtx *matrix)
{
    GXLoadPosMtxImm(*matrix, GX_PNMTX0);
    HuSprTexLoad(animation, 0, 0, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);

    {
        GXColor color = { 0, 0, 0, 0x20 };
        GXSetTevColor(GX_TEVREG0, color);
    }

    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
                    GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_A0, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_4, GX_TRUE,
                    GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                      GX_PTIDENTITY);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-200.0f, 400.0f, 100.0f);
    GXTexCoord2f32(textureS[0], textureT[0]);
    GXPosition3f32(200.0f, 400.0f, 100.0f);
    GXTexCoord2f32(textureS[1], textureT[1]);
    GXPosition3f32(200.0f, 0.0f, 100.0f);
    GXTexCoord2f32(textureS[2], textureT[2]);
    GXPosition3f32(-200.0f, 0.0f, 100.0f);
    GXTexCoord2f32(textureS[3], textureT[3]);
}

void fn_1_C2EC(Sprite *owner) {
    owner->textureS[0] = 0.0f;
    owner->textureT[0] = 0.0f;
    owner->textureS[1] = 0.0f;
    owner->textureT[1] = 1.0f;
    owner->textureS[2] = 1.0f;
    owner->textureT[2] = 1.0f;
    owner->textureS[3] = 1.0f;
    owner->textureT[3] = 0.0f;
}

void fn_1_C370(Sprite *owner)
{
    int index;

    for (index = 0; index < 4; ++index) {
        owner->textureS[index] -= 0.025f;
        if (owner->textureS[index] > 100.0f || owner->textureT[index] > 100.0f) {
            fn_1_C2EC(owner);
        }
    }
}

Sprite::~Sprite()
{
}
