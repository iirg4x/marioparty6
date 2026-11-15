#include <vector>
#include "REL/m634DLL/collision_shape.h"

extern "C" {
#include "dolphin/gx.h"
}
s32 fn_1_4BE0(CrossShape *first, CrossShape *second);

bool fn_1_4CD8(BoxShape *box, CrossShape *cross);

void fn_1_50C8(void);

std::vector<CollisionShape *> lbl_1_bss_34;

s32 fn_1_4BE0(CrossShape *first, CrossShape *second)
{
    Vector3 firstPosition = first->position;
    Vector3 secondPosition = second->position;
    Vec difference;
    f32 firstRadiusForComparison;
    f32 secondRadiusForComparison;
    f32 firstRadius;
    f32 secondRadius;
    f32 distance;

    firstRadius = first->radius;
    firstRadiusForComparison = firstRadius;
    secondRadius = second->radius;
    secondRadiusForComparison = secondRadius;
    PSVECSubtract(&firstPosition, &secondPosition, &difference);
    distance = PSVECMag(&difference);
    if (distance > firstRadiusForComparison + secondRadiusForComparison) {
        return 0;
    }
    return 1;
}

#define ABS(value) ((value) < 0.0f ? -(value) : (value))

static inline void toBoxSpace(BoxShape *object, Vec *outer, Vec *inner, Vec *result)
{
    Mtx transposed;

    MtxPtr matrix = (MtxPtr)static_cast<Model *>(object)->getMatrix();
    PSVECSubtract(outer, inner, result);
    PSMTXTranspose(matrix, transposed);
    PSMTXMultVec(transposed, result, result);
}

// Whether the cross's radius reaches a face of the box, in the box's own space.
bool fn_1_4CD8(BoxShape *box, CrossShape *cross)
{
    Vector3 boxPosition = box->position;
    Vector3 crossPosition = cross->position;
    Vec delta;
    f32 radius = cross->getRadius();
    f32 halfWidth = box->getWidth() / 2.0f;
    f32 halfHeight = box->getHeight() / 2.0f;
    f32 halfDepth = box->getDepth() / 2.0f;

    toBoxSpace(box, &crossPosition, &boxPosition, &delta);
    if (delta.z > 0.0f) {
        if (ABS(delta.z - radius) < halfDepth && halfWidth > ABS(delta.x)) {
            return 1;
        }
    } else {
        if (ABS(delta.z + radius) < halfDepth && halfWidth > ABS(delta.x)) {
            return 1;
        }
    }
    if (delta.x < 0.0f) {
        if (ABS(delta.x + radius) < halfWidth && halfDepth > ABS(delta.z)) {
            return 1;
        }
        if (ABS(delta.x - radius) < halfWidth && halfDepth > ABS(delta.z)) {
            return 1;
        }
    }
    return 0;
}

void fn_1_50C8(void)
{
    s32 count = lbl_1_bss_34.size();
    s32 outerIndex;
    for (outerIndex = 0; outerIndex < count; outerIndex += 1)
    {
        s32 outerType;
        CrossShape * outer = static_cast<CrossShape *>(lbl_1_bss_34.at(outerIndex));
        outerType = outer->getShapeKind();
        if (outer->getKind() == 0)
        {
            if (outer->isEnabled() != 0)
            {
                if (outer->getCollisionKind() == - 1)
                {
                    for (s32 innerIndex = 0; innerIndex < count; ++innerIndex)
                    {
                        CollisionShape * candidate = lbl_1_bss_34.at(innerIndex);
                        s32 innerType = candidate->getShapeKind();
                        if (outerType == 0)
                        {
                            if (innerType != 4)
                            {
                                continue;
                            }
                        }
                        else
                        {
                            if (innerType != 5)
                            {
                                continue;
                            }
                        }
                        if (candidate->isEnabled() == 0)
                        {
                            continue;
                        }
                        if (fn_1_4CD8(static_cast<BoxShape *>(candidate), outer))
                        {
                            outer->collisionKind = innerType;
                            candidate->collisionKind = outerType;
                            break;
                        }
                    }
                }
            }
        }
    }
}

CrossShape::CrossShape(float value) : CollisionShape(0), RenderObject(0x90), radius(value)
{
    lbl_1_bss_34.push_back(this);
    setCamera(2);
    setLayer(1);
    setAttribute(1);
}

void CrossShape::update() {}
void CrossShape::draw(Mtx *view) {
    Vec point = position;
    Mtx translated;
    PSMTXTrans(translated, point.x, point.y, point.z);
    PSMTXConcat(*view, translated, translated);
    GXLoadPosMtxImm(translated, GX_PNMTX0);
    GXSetCullMode(GX_CULL_NONE);
GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
GXSetNumChans(1);
GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
GXSetNumTexGens(0);
static const GXColor color = {255, 0, 255, 0};
GXSetTevColor(GX_TEVREG0, color);
GXSetNumTevStages(1);
GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
GXClearVtxDesc();
GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);

    GXSetLineWidth(12, GX_TO_ZERO);
    float extent = radius;
    GXBegin(GX_LINES, GX_VTXFMT0, 4);
    GXPosition3f32(0.0f, 0.0f, -extent);
    GXPosition3f32(0.0f, 0.0f, extent);
    GXPosition3f32(-extent, 0.0f, 0.0f);
    GXPosition3f32(extent, 0.0f, 0.0f);
}

BoxShape::BoxShape(float width, float height, float depth) : CollisionShape(1), RenderObject(0x90)
{
    dimensions.x = width;
    dimensions.y = height;
    dimensions.z = depth;
    lbl_1_bss_34.push_back(this);
    setCamera(2);
    setLayer(1);
    setAttribute(1);
}

void BoxShape::update() {}
void BoxShape::draw(Mtx *view) {
    Vec point = position;
    Mtx translated;
    Mtx basis;
    PSMTXCopy(CollisionShape::getMatrix(), basis);
    PSMTXTrans(translated, point.x, point.y, point.z);
    PSMTXConcat(*view, translated, translated);
    PSMTXConcat(translated, basis, basis);
    GXLoadPosMtxImm(basis, GX_PNMTX0);
    GXSetCullMode(GX_CULL_NONE);
GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
GXSetNumChans(1);
GXSetChanCtrl(GX_COLOR0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
GXSetNumTexGens(0);
static const GXColor color = {255, 0, 0, 0};
GXSetTevColor(GX_TEVREG0, color);
GXSetNumTevStages(1);
GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0);
GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_C0);
GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
GXClearVtxDesc();
GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);

    float x = dimensions.x / 2.0f;
    float y = dimensions.y / 2.0f;
    float z = dimensions.z / 2.0f;
    GXBegin(GX_QUADS, GX_VTXFMT0, 16);
    GXPosition3f32(-x,-y,z); GXPosition3f32(x,-y,z);
    GXPosition3f32(x,y,z); GXPosition3f32(-x,y,z);
    GXPosition3f32(-x,-y,-z); GXPosition3f32(x,-y,-z);
    GXPosition3f32(x,y,-z); GXPosition3f32(-x,y,-z);
    GXPosition3f32(x,-y,-z); GXPosition3f32(x,-y,z);
    GXPosition3f32(x,y,z); GXPosition3f32(x,y,-z);
    GXPosition3f32(-x,-y,-z); GXPosition3f32(-x,-y,z);
    GXPosition3f32(-x,y,z); GXPosition3f32(-x,y,-z);
}
