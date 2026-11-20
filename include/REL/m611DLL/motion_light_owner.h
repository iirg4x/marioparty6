#ifndef REL_M611DLL_MOTION_LIGHT_OWNER_H
#define REL_M611DLL_MOTION_LIGHT_OWNER_H
#include "REL/framework/model.h"
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"

extern "C" {
#include "dolphin/gx.h"
}

// Owns the two scene models and their lighting callbacks.
struct MotionLightOwner : RequestObject, MinigameSingleton<MotionLightOwner> {
    MotionLightOwner(bool dayMode);
    virtual ~MotionLightOwner();
    virtual void flagEnabled(int);
    virtual void update();
    OwnedModel primaryModel;
    OwnedModel secondaryModel;
};
void fn_1_A928(MotionLightOwner *, const Mtx matrix);
void fn_1_A958();

// Configures direct position and color vertices for translucent draws.
inline void configurePositionColorDrawing(bool setDepthMode)
{
    GXSetNumTevStages(1U);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    GXSetNumChans(1U);
    GXSetNumTexGens(0U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
                  GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetCullMode(GX_CULL_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                   GX_LO_CLEAR);
    if (setDepthMode) {
        GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    }
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
}

#endif
