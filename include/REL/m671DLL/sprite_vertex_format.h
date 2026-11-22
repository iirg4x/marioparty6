#ifndef REL_M671DLL_SPRITE_VERTEX_FORMAT_H
#define REL_M671DLL_SPRITE_VERTEX_FORMAT_H

#include "dolphin/gx.h"

// Selects direct XYZ positions and ST texture coordinates for sprite vertices.
static inline void setSpriteVertexFormat()
{
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
}

#endif
