#include "REL/m655DLL/rope.h"
#include "REL/m655DLL/game_constants.h"
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
/* The ropes hung over the course: verlet points between two ends, drawn as a tube of triangle strips. */
#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include <vector>
#include <math.h>
#include "REL/framework/vector3.h"
#include "datadir_enum.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/m655DLL/rope_point.h"
#include "REL/m655DLL/rope_api.h"
extern "C" {
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/memory.h"
}
extern "C" {

#include "game/sprput.h"
}
void collectGameObjects(int category, std::vector<GameObject *> *list);
inline Vec &operator*=(Vec &value, float scale) {
    PSVECScale(&value, &value, scale);
    return value;
}
inline Vec &operator+=(Vec &value, const Vec &other) {
    PSVECAdd(&other, &value, &value);
    return value;
}
void fn_1_ACC8(RopePoint *self, const Vec *value) {
    self->position.x = value->x;
    self->position.y = value->y;
    self->position.z = value->z;
    self->previousPosition.x = value->x;
    self->previousPosition.y = value->y;
    self->previousPosition.z = value->z;
    self->unknown.x = 0.0f;
    self->unknown.y = 0.0f;
    self->unknown.z = 0.0f;
}
void fn_1_AD2C(RopePoint *state) {
    state->position.y -= gravity;
    PSVECAdd(&state->unknown, &state->position, &state->position);
}
void fn_1_AD7C(RopePoint *self) {
    PSVECSubtract(&self->position, &self->previousPosition, &self->unknown);
    PSVECScale(&self->unknown, &self->unknown, 0.99f);
    self->previousPosition.x = self->position.x;
    self->previousPosition.y = self->position.y;
    self->previousPosition.z = self->position.z;
}
void fn_1_ADE4(void) {
    f32 distance;
    f32 length;
    RopeStart *start;
    s32 index;
    static RopeStart starts[3] = {
        {{cmToWorld(-250.0f), cmToWorld(449.0f), cmToWorld(-579.0f)}, DATANUM(0x720000, 10)},
        {{cmToWorld(0.0f), cmToWorld(449.0f), cmToWorld(-579.0f)}, DATANUM(0x720000, 11)},
        {{cmToWorld(250.0f), cmToWorld(449.0f), cmToWorld(-579.0f)}, DATANUM(0x720000, 12)}};
    for (index = 0; index < 3; index++) {
        start = &starts[index];
        Vec top = start->position;
        top.y = 2000.0f;
        top.z = 600.0f;
        distance = PSVECSquareDistance(&top, &start->position);
        length = 1.03 * std::sqrt((f64)distance);
        new Rope(index, &top, &start->position, length, starts[index].dataNumber);
    }
}
GameObject *fn_1_B140(s32 id) {

/* Retain assertion source location.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
    assert((id >= 0) && (id < 3));
    std::vector<GameObject *> result;
    collectGameObjects(id + 7, &result);
    int n = result.size();
    assert(n == 1);
    return result[0];
}

void fn_1_C148(HU3D_MODEL *self, Mtx *matrix);
void fn_1_C1A8(HU3D_MODEL *self, Mtx *matrix);
void fn_1_BC18(Rope *rope, RopePoint *first, RopePoint *last);

void fn_1_BD08(Rope *rope);

void fn_1_BC84(Rope *rope, RopePoint *first, RopePoint *last);

static inline s32 stepsFor(f32 length) {
    s32 stepCount;
    if ((s32)((f64)length / 30.0) < 2) {
        stepCount = 2;
    } else {
        stepCount = (s32)((f64)length / 30.0);
    }
    return stepCount;
}

static inline f32 interpolatePosition(f64 from, f64 to, f64 amount) {
    return (f32)(((1.0 - amount) * from) + (amount * to));
}

Rope::Rope(s32 mode, const Vec *start, Vec *end, f32 length, s32 dataNumber)
    : GameObject(mode + 7), tube(fn_1_C148), ends(fn_1_C1A8),
      animation(HuSprAnimRead(fn_80008DD0(dataNumber, 0x10000000, 2))), targetLength(length),
      pinned(0) {
    s32 step;
    s32 animationCount;
    f64 ratio;
    s32 interpolationCount;
    RopePoint coordinates;
    Vec interpolatedPosition;
    Vec position;

    tube.setHookData(this);
    tube.setLayer(4);
    ends.setHookData(this);
    ends.setLayer(4);
    ends.setShadow();
    ends.setAttribute(8);
    interpolationCount = stepsFor(length);
    for (step = 0; step <= interpolationCount; ++step) {
        ratio = (f64)step / (f64)interpolationCount;
        interpolatedPosition.x = interpolatePosition(start->x, end->x, ratio);

        interpolatedPosition.y = interpolatePosition(start->y, end->y, ratio);

        interpolatedPosition.z = interpolatePosition(start->z, end->z, ratio);

        fn_1_ACC8(&coordinates, &interpolatedPosition);
        points.push_back(coordinates);
    }
    animationCount = 0;
    while (animationCount < secondsToFrames<long>(2)) {
        fn_1_B8E0(this, end, &position, 1.0f);
        update();
        ++animationCount;
    }
}

Rope::~Rope() {}

static inline f32 motionPointDistance(const Vec *difference) {
    return PSVECMag(difference);
}

s32 fn_1_B8E0(Rope *rope, const Vec *target, Vec *result, f32 amount) {
    Vec difference;
    f32 magnitude;
    f32 scale;
    RopePoint *startingRecords;
    RopePoint *point;

    rope->pinned = 1;
    startingRecords = rope->points.begin();
    PSVECSubtract(&startingRecords->position, target, &difference);
    magnitude = motionPointDistance(&difference);
    point = &rope->points.back();
    point->previousPosition.x = point->position.x;
    point->previousPosition.y = point->position.y;
    point->previousPosition.z = point->position.z;
    if (magnitude <= rope->targetLength) {
        point->position.x = interpolatePosition(point->position.x, target->x, amount);
        point->position.y = interpolatePosition(point->position.y, target->y, amount);
        point->position.z = interpolatePosition(point->position.z, target->z, amount);
        return 0;
    }
    scale = (magnitude - rope->targetLength) / magnitude;
    PSVECScale(&difference, &difference, scale);
    PSVECAdd(target, &difference, result);
    point->position.x = result->x;
    point->position.y = result->y;
    point->position.z = result->z;
    return 1;
}

void Rope::update() {
    RopePoint *first = points.begin() + 1;
    RopePoint *last = points.end() - (pinned ? 1 : 0);
    fn_1_BC18(this, first, last);
    fn_1_BD08(this);
    fn_1_BC84(this, first, last);
    pinned = 0;
}

void fn_1_BC18(Rope *, RopePoint *first, RopePoint *last) {
    for (RopePoint *point = first; point != last; ++point) {
        fn_1_AD2C(point);
    }
}

void fn_1_BC84(Rope *, RopePoint *first, RopePoint *last) {
    RopePoint *current = first;
    while (current != last) {
        fn_1_AD7C(current);
        ++current;
    }
}

void fn_1_BD08(Rope *rope) {
    Vec firstPoint = rope->points.front().position;
    Vec lastPoint = rope->points.back().position;
    s32 pointCount = rope->points.size();
    f32 spacing = rope->targetLength / (f32)(pointCount - 1);
    s32 passCount = (0x1770 / pointCount) / (rope->pinned ? 2 : 1);
    for (s32 pass = 0; pass < passCount; ++pass) {
        std::vector<RopePoint>::iterator point = rope->points.begin();
        std::vector<RopePoint>::iterator end = rope->points.end() - (rope->pinned ? 2 : 1);
        while (point != end) {
            Vector3 difference;
            PSVECSubtract(&(point + 1)->position, &point->position, &difference);
            f32 distance = difference.length();
            if (distance != spacing) {
                if (0.0f != distance) {
                    PSVECScale(&difference, &difference, ((distance - spacing) / distance) / 2.0f);
                    PSVECAdd(&point->position, &difference, &point->position);
                    PSVECSubtract(&(point + 1)->position, &difference, &(point + 1)->position);
                }
            }
            ++point;
        }
        if (rope->pinned) {
            std::vector<RopePoint>::reverse_iterator point = rope->points.rbegin();
            std::vector<RopePoint>::reverse_iterator end = rope->points.rend() - 2;
            while (point != end) {
                Vec *current;
                Vec *previous;
                previous = &point[1].position;
                current = &point[0].position;
                Vector3 difference;
                PSVECSubtract(previous, current, &difference);
                f32 distance = difference.length();
                if (distance != spacing) {
                    if (0.0f != distance) {
                        PSVECScale(&difference, &difference,
                                   ((distance - spacing) / distance) / 2.0f);
                        PSVECAdd(current, &difference, current);
                        PSVECSubtract(previous, &difference, previous);
                    }
                }
                ++point;
            }
        }
        rope->points.front().position = firstPoint;
        if (rope->pinned) {
            rope->points.back().position = lastPoint;
        }
    }
}

void fn_1_C208(Rope *, Mtx);
void fn_1_C304(Rope *);
void fn_1_C3A4(Rope *);

void fn_1_C6C8(Rope *, Mtx);

void fn_1_C148(HU3D_MODEL *self, Mtx *matrix) {
    Rope *consumer = (Rope *)self->hookData;
    fn_1_C208(consumer, *matrix);
    fn_1_C304(consumer);
    fn_1_C6C8(consumer, *matrix);
}

void fn_1_C1A8(HU3D_MODEL *self, Mtx *matrix) {
    Rope *consumer = (Rope *)self->hookData;
    fn_1_C208(consumer, *matrix);
    fn_1_C3A4(consumer);
    fn_1_C6C8(consumer, *matrix);
}

extern "C" {
const GXVtxDescList lbl_1_rodata_60C[3] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_TEX0, GX_DIRECT},
    {GX_VA_NULL, GX_NONE},
};
const GXVtxAttrFmtList lbl_1_rodata_624[3] = {
    {GX_VA_POS, GX_POS_XYZ, GX_F32, 0},
    {GX_VA_TEX0, GX_TEX_ST, GX_F32, 0},
    {GX_VA_NULL, GX_POS_XY, (GXCompType)0, 0},
};
}

void fn_1_C208(Rope *rope, Mtx matrix) {
    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    GXSetNumChans(0);
    HuSprTexLoad((ANIMDATA *)rope->animation, 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_FALSE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_TRUE);
    GXSetAlphaCompare(GX_GEQUAL, 0xC0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_60C);
    GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_624);
}

inline void setRopeTevColor(GXTevColorArg color) {
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, color);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 0, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 0, GX_TEVPREV);
}

void fn_1_C304(Rope *) {
    setRopeTevColor(GX_CC_TEXC);
}

void fn_1_C3A4(Rope *) {
    GXColor tevColor = {0, 0, 0, 0x66};
    GXSetTevColor(GX_TEVREG0, tevColor);
    setRopeTevColor(GX_CC_A0);
}

// Vector operations compose through the destination they update.
inline Vec &vectorDifference(Vec &result, const Vec &first, const Vec &second) {
    PSVECSubtract(&first, &second, &result);
    return result;
}

inline Vec &vectorAddTo(const Vec &source, Vec &destination) {
    PSVECAdd(&source, &destination, &destination);
    return destination;
}

inline Vec &vectorAdd(Vec &result, const Vec *first, const Vec *second) {
    PSVECAdd(first, second, &result);
    return result;
}

inline Vec &vectorSubtract(Vec &result, const Vec *first, const Vec *second) {
    PSVECSubtract(first, second, &result);
    return result;
}

inline Vec &vectorNormalize(Vec &value) {
    PSVECNormalize(&value, &value);
    return value;
}

inline Vec &vectorScale(Vec &value, float factor) {
    PSVECScale(&value, &value, factor);
    return value;
}

inline float vectorSquareLength(const Vec &value) {
    return PSVECSquareMag(&value);
}

inline float vectorLength(const Vec &value) {
    return PSVECMag(&value);
}

void ringAround(const Vec *triangle, Vec *points, const Vec *axisPoint, f32 scale,
                              s32 count) {
    Mtx rotation;
    Vec direction;
    Vec offset;
    Vec axis;
    Vec firstEdge;
    Vec secondEdge;
    s32 index;

    PSVECSubtract(&triangle[1], &triangle[0], &firstEdge);
    PSVECSubtract(&triangle[2], &triangle[1], &secondEdge);
    if (vectorSquareLength(firstEdge) <= 0.0f) {
        firstEdge = secondEdge;
    } else {
        if (vectorSquareLength(secondEdge) <= 0.0f) {
            secondEdge = firstEdge;
        }
    }
    vectorNormalize(vectorAdd(direction, &firstEdge, &secondEdge));
    vectorNormalize(vectorSubtract(offset, &triangle[1], axisPoint));
    PSVECCrossProduct(&direction, &offset, &axis);
    vectorScale(vectorNormalize(axis), scale);
    PSMTXRotAxisRad(rotation, &direction, 0.34906584f);
    PSMTXMultVecSR(rotation, &axis, &axis);
    PSMTXRotAxisRad(rotation, &direction, (f32)((M_PI * 140.0 / 180.0) / (f64)(f32)(count - 1)));
    for (index = 0; index < count; ++index) {
        vectorAdd(points[index], &triangle[1], &axis);
        PSMTXMultVecSR(rotation, &axis, &axis);
    }
}

void fn_1_C6C8(Rope *rope, Mtx matrix) {
    float length;
    Mtx inverse;
    int pointCount = rope->points.size();
    Vec view = {0.0f, 0.0f, 0.0f};
    float brightness;
    float texture;
    PSMTXInverse(matrix, inverse);
    PSMTXMultVec(inverse, &view, &view);
    std::vector<Vec> centerLine(pointCount + 2);
    {
        int centerSample;
        for (centerSample = 0; centerSample < pointCount; ++centerSample) {
            centerLine[centerSample + 1] = rope->points[centerSample].position;
        }
    }
    centerLine[0] = centerLine[1];
    Vec lastTangent;
    length = vectorLength(
        vectorDifference(lastTangent, centerLine[pointCount], centerLine[pointCount - 1]));
    if (length > 0.0f) {
        vectorAddTo(lastTangent *= 9.0f / length, centerLine[pointCount]);
    }
    centerLine[pointCount + 1] = centerLine[pointCount];
    std::vector<Vec> ringPoints(pointCount * 7);
    for (int ringSample = 0; ringSample < pointCount; ++ringSample) {
        ringAround(&centerLine[ringSample], &ringPoints[ringSample * 7], &view, 9.0f, 7);
    }
    for (int segment = 0; segment < pointCount - 1; ++segment) {
        brightness = (float)(segment == pointCount - 2 ? 0.6 : 0.2);
        float nextBrightness = 0.4 + brightness;
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 0xE);
        for (int ring = 0; ring < 7; ++ring) {
            int point = ring + segment * 7;
            Vec &left = ringPoints[point];
            Vec &right = ringPoints[point + 7];
            texture = (float)ring / 6.0f;
            GXPosition3f32(left.x, left.y, left.z);
            GXTexCoord2f32(brightness, texture);
            GXPosition3f32(right.x, right.y, right.z);
            GXTexCoord2f32(nextBrightness, texture);
        }
    }
}
