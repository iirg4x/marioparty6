// Queues colored debug geometry for drawing by a model hook.
extern "C"
{
#include "game/hu3d.h"
#include "types.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
}

#include "REL/framework/misc_rotarc.h"
#include "REL/framework/debug_geometry.h"
#include "REL/framework/model.h"
#include "REL/framework/singleton.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
#include "REL/framework/gxdl.h"

// Creates the debug model and reserves its point and color buffers.
GeometryOwner::GeometryOwner(HU3D_MODEL_HOOK hook)
    : OwnedModel(hook), displayLists(new DisplayListCache(0x3000))
{
    points.reserve(128);
    colors.reserve(128);
    setLayer(7);
}

// Appends a point and returns its buffer index.
inline s32 GeometryOwner::addPoint(const Vec &point)
{
    points.push_back(point);
    return points.size() - 1;
}

// Appends a packed color and returns its buffer index.
inline s32 GeometryOwner::addColor(u32 color)
{
    colors.push_back(*reinterpret_cast<const GXColor *>(&color));
    return colors.size() - 1;
}

// Queues a colored line between two points.
// Neither minigame calls this function.
void GeometryOwner::addLink(const Vec &first, const Vec &second, u32 color)
{
    Line line;
    line.first = addPoint(first);
    line.second = addPoint(second);
    line.color = addColor(color);
    lines.push_back(line);
}

// Emits the two indexed vertices of a queued line.
void GeometryOwner::fn_1_157DC(const Line &line)
{
    GXPosition1x16(line.first);
    GXColor1x16(line.color);
    GXPosition1x16(line.second);
    GXColor1x16(line.color);
}

// Queues a colored triangle from three points.
// Neither minigame calls this function.
void GeometryOwner::fn_1_15848(const Vec &first, const Vec &second, const Vec &third, u32 color)
{
    Triangle triangle;
    triangle.first = addPoint(first);
    triangle.second = addPoint(second);
    triangle.third = addPoint(third);
    triangle.color = addColor(color);
    triangles.push_back(triangle);
}

// Emits the three indexed vertices of a queued triangle.
void GeometryOwner::fn_1_159C8(const Triangle &triangle)
{
    GXPosition1x16(triangle.first);
    GXColor1x16(triangle.color);
    GXPosition1x16(triangle.second);
    GXColor1x16(triangle.color);
    GXPosition1x16(triangle.third);
    GXColor1x16(triangle.color);
}

// Queues a colored sphere with the requested radius.
// Neither minigame calls this function.
void GeometryOwner::fn_1_15A54(const Vec &position, f32 scale, u32 color)
{
    SphereShape sphere;
    sphere.position = addPoint(position);
    sphere.color = addColor(color);
    sphere.scale = scale;
    spheres.push_back(sphere);
}

// Transforms and draws a queued sphere.
void GeometryOwner::fn_1_15B38(const SphereShape &sphere)
{
    const Vec &position = points[sphere.position];
    f32 scale = sphere.scale;
    Mtx matrix;
    PSMTXScale(matrix, scale, scale, scale);
    f32 positionX;
    f32 positionY;
    f32 positionZ;
    positionZ = position.z;
    positionY = position.y;
    positionX = position.x;
    PSMTXTransApply(matrix, matrix, positionX, positionY, positionZ);
    PSMTXConcat(transform, matrix, matrix);
    GXSetChanMatColor(GX_COLOR0A0, colors[sphere.color]);
    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    if (displayLists->begin(1))
    {
        GXDrawSphere(8, 16);
        displayLists->end();
    }
}

// Queues a cylinder between the supplied endpoints.
// Neither minigame calls this function.
void GeometryOwner::fn_1_15C9C(const Vec &start, const Vec &end, f32 scale, u32 color)
{
    Cylinder cylinder;
    cylinder.start = addPoint(start);
    cylinder.end = addPoint(end);
    cylinder.color = addColor(color);
    cylinder.scale = scale;
    cylinders.push_back(cylinder);
}

// Tests whether a value lies strictly inside a centered interval.
bool fn_1_1AF58(f32 value, f32 center, f32 radius)
{
    value -= center;
    return value > -radius && value < radius;
}

// Reference direction for rotating a cylinder onto its segment.
static const Vec zAxis = { 0.0f, 0.0f, 1.0f };

// Builds the rotation from the positive Z axis to the endpoint direction and returns its length.
f32 fn_1_1AF90(Mtx matrix, const Vec *first, const Vec *second)
{
    Vec difference;
    PSVECSubtract(second, first, &difference);
    f32 magnitude = PSVECMag(&difference);
    f32 length = magnitude;
    if (length < 0.00001f)
        return length;
    Quaternion rotation;
    if (fn_1_1AF58(difference.z / length, -1.0f, 0.00001f))
    {
        rotation.x = 0.0f;
        rotation.y = 1.0f;
        rotation.z = 0.0f;
        rotation.w = 0.0f;
    }
    else
    {
        makeRotationArc(&rotation, &zAxis, &difference);
    }
    PSMTXQuat(matrix, &rotation);
    return length;
}

// Transforms and draws a queued cylinder.
void GeometryOwner::fn_1_15F5C(const Cylinder &cylinder)
{
    const Vec *end;
    const Vec *start = &points[cylinder.start];
    end = &points[cylinder.end];
    Mtx rotation;
    f32 length = fn_1_1AF90(rotation, start, end);
    if (length < 0.00001f)
        return;
    Mtx matrix;
    PSMTXTrans(matrix, 0.0f, 0.0f, 1.0f);
    f32 scaleX;
    f32 scaleY;
    scaleY = cylinder.scale;
    scaleX = cylinder.scale;
    PSMTXScaleApply(matrix, matrix, scaleX, scaleY, length / 2.0f);
    PSMTXConcat(rotation, matrix, matrix);
    f32 startX;
    f32 startY;
    f32 startZ;
    startZ = start->z;
    startY = start->y;
    startX = start->x;
    PSMTXTransApply(matrix, matrix, startX, startY, startZ);
    PSMTXConcat(transform, matrix, matrix);
    GXSetChanMatColor(GX_COLOR0A0, colors[cylinder.color]);
    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    if (displayLists->begin(2))
    {
        GXDrawCylinder(8);
        displayLists->end();
    }
}

// Clears all queued debug geometry.
void GeometryOwner::fn_1_162A8()
{
    points.clear();
    colors.clear();
    lines.clear();
    triangles.clear();
    spheres.clear();
    cylinders.clear();
}

// Draws the queued debug geometry with vertex colors, then clears it.
void fn_1_1B48C(HU3D_MODEL *model, Mtx *matrix)
{
    GeometryOwner *object = GeometrySingleton::getChecked().object.get();
    static const GXColor ambient = { 0xC0, 0xC0, 0xC0, 0xFF };
    PSMTXCopy(*matrix, object->transform);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetNumChans(1);
    GXSetChanAmbColor(GX_COLOR0A0, ambient);
    GXSetChanCtrl(GX_COLOR0A0, GX_TRUE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetNumTevStages(1);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetCullMode(GX_CULL_NONE);

    std::for_each(object->cylinders.begin(), object->cylinders.end(),
                  std::bind1st(std::mem_fun(&GeometryOwner::fn_1_15F5C), object));
    std::for_each(object->spheres.begin(), object->spheres.end(),
                  std::bind1st(std::mem_fun(&GeometryOwner::fn_1_15B38), object));

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    GXSetArray(GX_VA_POS, &object->points[0], sizeof(Vec));
    GXSetArray(GX_VA_CLR0, &object->colors[0], sizeof(GXColor));
    DCStoreRangeNoSync(&object->points[0], object->points.size() * sizeof(Vec));
    GXSetChanCtrl(GX_COLOR0A0, GX_TRUE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT0, GX_DF_CLAMP, GX_AF_NONE);
    GXLoadPosMtxImm(*matrix, GX_PNMTX0);

    if (object->triangles.size() != 0)
    {
        GXBegin(GX_TRIANGLES, GX_VTXFMT0, object->triangles.size() * 3);
        std::for_each(object->triangles.begin(), object->triangles.end(),
                      std::bind1st(std::mem_fun(&GeometryOwner::fn_1_159C8), object));
    }
    if (object->lines.size() != 0)
    {
        GXBegin(GX_LINES, GX_VTXFMT0, object->lines.size() * 2);
        std::for_each(object->lines.begin(), object->lines.end(),
                      std::bind1st(std::mem_fun(&GeometryOwner::fn_1_157DC), object));
    }

    object->fn_1_162A8();
}

// Creates the shared debug geometry model.
GeometrySingleton::GeometrySingleton() : object(new GeometryOwner(fn_1_1B48C))
{
}

// Releases the shared debug geometry model.
GeometrySingleton::~GeometrySingleton()
{
}

// Discards a debug line request.
void fn_1_1C318(const Vec *start, const Vec *end, u32 color)
{
}

// Neither minigame calls this function.
void fn_1_1C31C(void)
{
}

// Neither minigame calls this function.
void fn_1_1C320(void)
{
}

// Discards a debug triangle request.
void fn_1_1C324(const Vec *first, const Vec *second, const Vec *third, u32 color)
{
}

// Neither minigame calls this function.
void fn_1_1C328(void)
{
}

// Discards a debug cylinder request.
void fn_1_1C32C(const Vec *start, const Vec *end, f32 width, u32 color)
{
}
