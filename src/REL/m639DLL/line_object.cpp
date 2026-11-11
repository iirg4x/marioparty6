#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
// Lines that fall from the map's lights: each is a pair of points on a stretched spring.
#include "REL/m639DLL/line_object.h"
#include <math.h>

extern "C" {
#include "dolphin/gx.h"
}


#include "REL/framework/random.h"

extern "C" {


int lbl_1_bss_48 = minutesToFrames(5L);
int lbl_1_bss_4C = secondsToFrames(30L);
f32 lbl_1_bss_50 = (1.0 / 60.0) * ((1.0 / 60.0) * cmToWorld(980.665f));
void fn_1_25B0(float *position);
extern const GXVtxDescList lbl_1_rodata_F0[3];
extern const GXVtxAttrFmtList lbl_1_rodata_108[3];
}

static inline double lineConstant20()
{
    return 20.0;
}

static inline double lineConstant10()
{
    return 10.0;
}

static inline f32 pointDistance(const Vec *first, const Vec *second)
{
    Vec difference;
    PSVECSubtract(first, second, &difference);
    return PSVECMag(&difference);
}

Line::Line(const Vec *position, const Vec *target) : target(*target)
{
    accumulatedDistance = 500.0f;
    double angle = RandomFloat(-3.1415927f, 3.1415927f);
    double radius = RandomFloat(200.0f);
    points[0].x = position->x;
    points[0].y = position->y;
    points[0].z = position->z;
    points[0].x = (f32)((double)points[0].x + (radius * sin(angle)));
    points[0].z = (f32)((double)points[0].z + (radius * cos(angle)));

    points[1].x = points[0].x;
    points[1].y = points[0].y;
    points[1].z = points[0].z;
    points[1].x -= RandomFloat(-1.0f, 0.0f) * lineConstant20();
    points[1].y -= RandomFloat(0.0f, 1.0f) * lineConstant10();
    points[1].z -= RandomFloat(0.0f, 1.0f) * lineConstant20();

    velocities[0].x = target->x;
    velocities[0].y = target->y;
    velocities[0].z = target->z;
    velocities[1].x = target->x;
    velocities[1].y = target->y;
    velocities[1].z = target->z;
    velocities[1].x = (f32)((double)velocities[1].x * (1.0 + RandomFloat(-0.2f, 0.2f)));
    velocities[1].y = (f32)((double)velocities[1].y * (1.0 + RandomFloat(-0.2f, 0.2f)));
    velocities[1].z = (f32)((double)velocities[1].z * (1.0 + RandomFloat(-0.2f, 0.2f)));

    distanceLimit = pointDistance(&points[0], &points[1]);
}

/* Initializes an object and adds a set of randomized offset records. */
LineObject::LineObject(const Vec *position, const Vec *target) : RenderObject(5)
{
    setLayer(3);
    int count = (int)RandomFloat(2.0f, 4.0f);
    lines.clear();
    for (int index = 0; index < count; ++index) {
        lines.push_back(Line(position, target));
    }
}

LineObject::~LineObject() {}

inline LinePoint &LinePoint::operator=(const LinePoint &other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}

static inline void addPoint(const LinePoint &velocity, LinePoint &point)
{
    PSVECAdd(&velocity, &point, &point);
}

/* A debug check of a point; empty in this build. */
static inline void debugPoint(const LinePoint *point) {}

static inline LinePoint &differencePoint(const LinePoint &first, const LinePoint &second, LinePoint &destination)
{
    debugPoint(&first);
    PSVECSubtract(&first, &second, &destination);
    return destination;
}

inline LinePoint &LinePoint::operator*=(f32 scale)
{
    PSVECScale(this, this, scale);
    return *this;
}

bool Line::update()
{
    LinePoint initial[2];
    Vec difference;
    f32 scale;

    for (s32 index = 0; index < 2; ++index) {
        LinePoint *point = &points[index];
        initial[index] = *point;
        velocities[index].x *= 1.0 + RandomFloat(-0.2f, 0.2f);
        velocities[index].y *= 1.0 + RandomFloat(-0.1f, 0.1f);
        velocities[index].z *= 1.0 + RandomFloat(-0.2f, 0.2f);
        addPoint(velocities[index], *point);
        point->y -= lbl_1_bss_50;
    }

    PSVECSubtract(&points[1], &points[0], &difference);
    f32 measuredDistance = PSVECMag(&difference);
    f32 distanceForComparison = measuredDistance;
    f32 distanceForCorrection = distanceForComparison;
    if (distanceForCorrection > distanceLimit) {
        scale = ((distanceForCorrection - distanceLimit) / 2.0f) / distanceForCorrection;
        PSVECScale(&difference, &difference, scale);
        PSVECAdd(&points[0], &difference, &points[0]);
        PSVECSubtract(&points[1], &difference, &points[1]);
    }

    for (s32 adjustmentIndex = 0; adjustmentIndex < 2; ++adjustmentIndex) {
        differencePoint(points[adjustmentIndex], initial[adjustmentIndex], velocities[adjustmentIndex]) *= 0.99f;
    }

    f32 trailingDistance = PSVECMag(&velocities[1]);
    f32 leadingDistance = PSVECMag(&velocities[0]);
    accumulatedDistance -= (leadingDistance + trailingDistance) / 2.0f;

    return accumulatedDistance > 0.0f;
}

static inline bool updateLine(Line *line)
{
    f32 leadingDistance;
    f32 trailingDistance;
    f32 distanceForComparison;
    f32 measuredDistance;
    s32 adjustmentIndex;
    f32 scale;
    Vec difference;
    LinePoint initial[2];

    for (s32 index = 0; index < 2; ++index) {
        LinePoint *point = &line->points[index];
        initial[index] = *point;
        line->velocities[index].x *= 1.0 + RandomFloat(-0.2f, 0.2f);
        line->velocities[index].y *= 1.0 + RandomFloat(-0.1f, 0.1f);
        line->velocities[index].z *= 1.0 + RandomFloat(-0.2f, 0.2f);
        addPoint(line->velocities[index], *point);
        point->y -= lbl_1_bss_50;
    }

    PSVECSubtract(&line->points[1], &line->points[0], &difference);
    measuredDistance = PSVECMag(&difference);
    distanceForComparison = measuredDistance;
    f32 distanceForCorrection = distanceForComparison;
    if (distanceForCorrection > line->distanceLimit) {
        scale = ((distanceForCorrection - line->distanceLimit) / 2.0f) / distanceForCorrection;
        PSVECScale(&difference, &difference, scale);
        PSVECAdd(&line->points[0], &difference, &line->points[0]);
        PSVECSubtract(&line->points[1], &difference, &line->points[1]);
    }

    for (adjustmentIndex = 0; adjustmentIndex < 2; ++adjustmentIndex) {
        differencePoint(line->points[adjustmentIndex], initial[adjustmentIndex], line->velocities[adjustmentIndex]) *= 0.99f;
    }

    trailingDistance = PSVECMag(&line->velocities[1]);
    leadingDistance = PSVECMag(&line->velocities[0]);
    line->accumulatedDistance -= (leadingDistance + trailingDistance) / 2.0f;

    return line->accumulatedDistance > 0.0f;
}

void LineObject::update()
{
    std::list<Line>::iterator node = lines.begin();
    std::list<Line>::iterator end = lines.end();

    while (node != end) {
        if (!updateLine(&*node)) {
            lines.erase(node);
        }
        ++node;
    }

    if (lines.empty()) {
        delete this;
    }
}

extern const GXVtxDescList lbl_1_rodata_F0[3] = {
    { GX_VA_POS, GX_DIRECT },
    { GX_VA_CLR0, GX_DIRECT },
    { GX_VA_NULL, GX_NONE }
};

extern const GXVtxAttrFmtList lbl_1_rodata_108[3] = {
    { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
    { GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 },
    { GX_VA_NULL, (GXCompCnt)0, (GXCompType)0, 0 }
};

static inline unsigned long drawCount(const std::list<Line> &lines)
{
    return lines.size();
}

void LineObject::draw(Mtx *matrix)
{
    if (!lines.empty()) {
        GXLoadPosMtxImm(*matrix, GX_PNMTX0);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX,
                      GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXSetNumTexGens(0);
        GXSetNumTevStages(1);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
        GXSetCullMode(GX_CULL_NONE);
        GXSetColorUpdate(GX_TRUE);
        GXSetAlphaUpdate(GX_FALSE);
        GXSetZCompLoc(GX_TRUE);
        GXSetZMode(GX_TRUE, GX_LESS, GX_TRUE);
        GXSetAlphaCompare(GX_ALWAYS, 0xFF, GX_AOP_OR, GX_ALWAYS, 0xFF);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXClearVtxDesc();
        GXSetVtxDescv(lbl_1_rodata_F0);
        GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_108);
        GXSetLineWidth(0xC, GX_TO_ZERO);

        std::list<Line>::iterator node = lines.begin();
        std::list<Line>::iterator end = lines.end();
        GXBegin(GX_LINES, GX_VTXFMT0, (u16)(drawCount(lines) * 2));
        while (node != end) {
            fn_1_25B0(&node->points[0].x);
            ++node;
        }
    }
}

// Sends the two position vectors and command bytes to the graphics FIFO.
void fn_1_25B0(float *position)
{
    float *source = position;
    float *normal = position + 3;
    float x;
    float y;
    float z;
    z = source[2];
    y = source[1];
    x = source[0];
    GXPosition3f32(x, y, z);
    GXColor4u8(0, 0x94, 0, 0xFF);
    float normalX;
    float normalY;
    float normalZ;
    normalZ = normal[2];
    normalY = normal[1];
    normalX = normal[0];
    GXPosition3f32(normalX, normalY, normalZ);
    GXColor4u8(0, 0x40, 0, 0x20);
}
