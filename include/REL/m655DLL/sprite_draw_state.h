#ifndef REL_M655DLL_SPRITE_DRAW_STATE_H
#define REL_M655DLL_SPRITE_DRAW_STATE_H

#include "dolphin/gx.h"

/* Set the cull, depth, alpha-compare and blend state shared by sprite drawing, and load the vertex
 * descriptors and formats. */
static inline void setSpriteDrawState(GXCullMode cullMode, GXBool zBeforeTexture,
                                      GXBool zCompare, GXCompare zFunction,
                                      GXBool zUpdate,
                                      const GXVtxDescList *descriptors,
                                      const GXVtxAttrFmtList *formats)
{
    GXSetCullMode(cullMode);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(zBeforeTexture);
    GXSetZMode(zCompare, zFunction, zUpdate);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(descriptors);
    GXSetVtxAttrFmtv(GX_VTXFMT0, formats);
}

#endif
