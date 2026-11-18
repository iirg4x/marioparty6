#ifndef REL_FRAMEWORK_DEBUG_GEOMETRY_H
#define REL_FRAMEWORK_DEBUG_GEOMETRY_H
#include "types.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "REL/framework/model.h"
#include "REL/framework/gxdl.h"
#include <memory>
#include <vector>

/* Indexed colored primitives queued for the debug drawing model. */
struct Line {
    u32 first;
    u32 second;
    u32 color;
};

struct Triangle {
    u32 first;
    u32 second;
    u32 third;
    u32 color;
};

struct SphereShape {
    s32 position;
    s32 color;
    f32 scale;
};

struct Cylinder {
    s32 start;
    s32 end;
    s32 color;
    f32 scale;
};

/* Own the queued primitives and their display-list cache. */
class GeometryOwner : public OwnedModel {
  public:
    Mtx transform;
    std::vector<Vec> points;
    std::vector<GXColor> colors;
    std::vector<Line> lines;
    std::vector<Triangle> triangles;
    std::vector<SphereShape> spheres;
    std::vector<Cylinder> cylinders;
    std::auto_ptr<DisplayListCache> displayLists;

    GeometryOwner(HU3D_MODEL_HOOK hook);
    s32 addPoint(const Vec &point);
    s32 addColor(u32 color);
    void addLink(const Vec &first, const Vec &second, s32 color);
    void fn_1_157DC(const Line &line);
    void fn_1_15848(const Vec &first, const Vec &second, const Vec &third, s32 color);
    void fn_1_159C8(const Triangle &triangle);
    void fn_1_15A54(const Vec &position, f32 scale, s32 color);
    void fn_1_15B38(const SphereShape &sphere);
    void fn_1_15C9C(const Vec &start, const Vec &end, f32 scale, s32 color);
    void fn_1_15F5C(const Cylinder &cylinder);
    void fn_1_162A8();
};
#endif
