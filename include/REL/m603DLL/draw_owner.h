#ifndef REL_M603DLL_DRAW_OWNER_H
#define REL_M603DLL_DRAW_OWNER_H

#include "dolphin/types.h"
extern "C" {
#include "game/hu3d.h"
}
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"

/* Configure GX for untextured, alpha-blended vertex colors. */
static inline void configureColorDrawing(GXBool depthTest = GX_FALSE,
                                         GXCompare depthFunction = GX_ALWAYS)
{
    GXSetNumTevStages(1U);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1U);
    GXSetNumTexGens(0U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetZMode(depthTest, depthFunction, GX_FALSE);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0U);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0U);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
}

/* Draw a camera shutter or flash and trigger its photo. */
class FadeWork : public ListNode<FadeWork> {
public:
    s32 progress;
    s32 context;
    s16 modelId;
    u8 active;
    FadeWork(s32 context);
    ~FadeWork();
    void fn_1_3B1C(Mtx *matrix);
    void fn_1_3B8C(Mtx *matrix);
    void fn_1_43A0(Mtx *matrix);
};

/* Own and update the shutter and frame-overlay lists. */
class DrawListOwner : public RequestObject,
                              public MinigameSingleton<DrawListOwner> {
public:
 DrawListOwner();
 virtual ~DrawListOwner();
 virtual void flagEnabled(int);
 virtual void update();
 void fn_1_3714();
};

struct ModelDrawOwner;
void fn_1_39C4();
void fn_1_48CC();
extern "C" void fn_1_3A94();
void fn_1_499C();
void fn_1_3AD4(HU3D_MODEL*, Mtx*);
void fn_1_3AC4(FadeWork *);
void fn_1_49DC(HU3D_MODEL *model, Mtx *matrix);
void fn_1_49CC(ModelDrawOwner *);

#endif
