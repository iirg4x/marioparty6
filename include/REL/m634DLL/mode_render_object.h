#ifndef REL_M634DLL_MODE_RENDER_OBJECT_H
#define REL_M634DLL_MODE_RENDER_OBJECT_H
#include "dolphin/gx.h"

#include "REL/framework/game_object2.h"
struct ModeRenderObject : RenderObject {
    // Initializes the object category.
    explicit ModeRenderObject(int category)
        : RenderObject(category)
    {
    }
    // Releases the object and its member-owned resources.
    virtual ~ModeRenderObject() { }
    // Does nothing.
    virtual void mode(int value) { }
};
/* Sets the color channel and vertex format for a textured quad. */
static inline void setTexturedQuadFormat(void)
{
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
}

#endif
