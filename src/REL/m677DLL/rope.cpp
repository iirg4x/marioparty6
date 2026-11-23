// Simulates and draws the ropes the player can pull.

#include "REL/framework/range.h"
#include "REL/framework/game_object_man.h"
#include "REL/m677DLL/rope.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/framework/world_units.h"
#include "dolphin/gx.h"
#include <vector>
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/rope_point.h"
#include "REL/framework/interpolation.h"
extern "C" {
#include "game/hu3d.h"
#include "game/sprite.h"
}
#include "game/sprput.h"
#include "REL/framework/vector_math.h"

#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"

// Scales the vector in place.
inline Vec &operator*=(Vec &value, float scale)
{
    PSVECScale(&value, &value, scale);
    return value;
}

// Initializes a simulation point from a single position.
// The previous position starts at the same coordinates, with no displacement.
void fn_1_1DD9C(RopePoint *self, const Vec *value)
{
    self->position.x = value->x;
    self->position.y = value->y;
    self->position.z = value->z;

    self->previousPosition.x = value->x;
    self->previousPosition.y = value->y;
    self->previousPosition.z = value->z;
    vectorSet(&self->displacement, 0.0f, 0.0f, 0.0f);
}

// Moves the point under gravity.
// The displacement from the preceding step carries the point's momentum.
void RopePoint::fn_1_1DE00()
{
    position.y -= gravity;
    PSVECAdd(&displacement, &position, &position);
}

// Recomputes the displacement from the change in position, applies damping,
// and saves the latest position for the next step.
void RopePoint::update()
{
    PSVECSubtract(&position, &previousPosition, &displacement);
    PSVECScale(&displacement, &displacement, 0.99f);
    previousPosition.x = position.x;
    previousPosition.y = position.y;
    previousPosition.z = position.z;
}

// Looks up the unique object for the selected rope index.
// Rope object IDs begin at three.
// Each slot has exactly one object.
Rope *fn_1_1DEB8(s32 id)
{
    assert((id >= 0) && (id < 4));
    std::vector<GameObject *> result;
    collectGameObjects(id + 3, &result);
    int n = result.size();
    assert(n == 1);
    return static_cast<Rope *>(result[0]);
}

// Deletes the four selectable ropes.
void fn_1_1DFA8()
{
    for (s32 id = 0; id < 4; ++id) {
        delete fn_1_1DEB8(id);
    }
}

// Builds a straight rope between the two endpoints and sets up rendering.
Rope::Rope(s32 ropeNumber, const Vec *start, const Vec *end, f32 length, s32 dataNumber)
    : GameObject(ropeNumber + 3), tube(fn_1_1EF94), ends(fn_1_1EFF4),
      // Load the rope sprite in the numbered allocation group used by model resources.
      animation(HuSprAnimRead(fn_80008DD0(dataNumber, 0x10000000, 2))), targetLength(length),
      pinned(0)
{
    s32 step;
    f64 ratio;
    s32 interpolationCount;
    RopePoint coordinates;
    Vec interpolatedPosition;

    tube.setHookData(this);
    tube.setLayer(4);
    ends.setHookData(this);
    ends.setLayer(4);
    ends.setShadow();
    ends.setAttribute(0x8);
    interpolationCount = rangeMaximum<s32>(2, (s32) ((f64) length / 20.0));
    for (step = 0; step <= interpolationCount; ++step) {
        ratio = (f64) step / (f64) interpolationCount;
        interpolatedPosition.x = interpolateValue(start->x, end->x, ratio);
        interpolatedPosition.y = interpolateValue(start->y, end->y, ratio);
        interpolatedPosition.z = interpolateValue(start->z, end->z, ratio);
        fn_1_1DD9C(&coordinates, &interpolatedPosition);
        points.push_back(coordinates);
    }
    const Vec *source = &points.begin()->position;
    target.x = source->x;
    target.y = source->y;
    target.z = source->z;
}

// Releases the two rope models and point storage.
Rope::~Rope()
{
}

// Pins the last rope point, saves its previous position, and blends it toward the goal.
void fn_1_1E638(Rope *rope, const Vec *goal, f64 blend)
{
    RopePoint *last;

    rope->pinned = 1;
    last = &rope->points.back();
    last->previousPosition = last->position;
    last->position.x = interpolateValue(last->position.x, goal->x, blend);
    last->position.y = interpolateValue(last->position.y, goal->y, blend);
    last->position.z = interpolateValue(last->position.z, goal->z, blend);
}

// Raises the rope target by the rope length.
void fn_1_1E7F0(Rope *rope)
{
    rope->target.y += rope->targetLength;
}

// Moves the rope toward its target and solves the point spacing.
void Rope::update()
{
    RopePoint *first = points.begin() + 1;
    RopePoint *last = points.end() - (pinned ? 1 : 0);
    fn_1_1EDEC(this);
    fn_1_1E8BC(this, first, last);
    fn_1_1E9AC(this);
    fn_1_1E928(this, first, last);
    pinned = 0;
}

// Integrates displacement and gravity for the movable points.
void fn_1_1E8BC(Rope *, RopePoint *first, RopePoint *last)
{
    for (RopePoint *point = first; point != last; ++point) {
        point->fn_1_1DE00();
    }
}

// Updates the velocities of the movable points after solving constraints.
void fn_1_1E928(Rope *, RopePoint *first, RopePoint *last)
{
    RopePoint *current = first;
    while (current != last) {
        current->update();
        ++current;
    }
}

// Adjusts adjacent points toward uniform spacing along the rope.
void fn_1_1E9AC(Rope *rope)
{
    Vec firstPoint = rope->points.front().position;
    Vec lastPoint = rope->points.back().position;
    s32 pointCount = rope->points.size();
    f32 spacing = rope->targetLength / (f32) (pointCount - 1);
    s32 passCount = (6000 / pointCount) / (rope->pinned ? 2 : 1);
    for (s32 pass = 0; pass < passCount; ++pass) {
        std::vector<RopePoint>::iterator point = rope->points.begin();
        std::vector<RopePoint>::iterator end = rope->points.end() - (rope->pinned ? 2 : 1);
        while (point != end) {
            Vec forwardDifference;
            PSVECSubtract(&(point + 1)->position, &point->position, &forwardDifference);
            f32 forwardDistance = vectorMagnitude(&forwardDifference);
            if (forwardDistance != spacing) {
                if (0.0f != forwardDistance) {
                    PSVECScale(&forwardDifference, &forwardDifference,
                               ((forwardDistance - spacing) / forwardDistance) / 2.0f);
                    PSVECAdd(&point->position, &forwardDifference, &point->position);
                    PSVECSubtract(&(point + 1)->position, &forwardDifference,
                                  &(point + 1)->position);
                }
            }
            ++point;
        }
        if (rope->pinned) {
            std::vector<RopePoint>::reverse_iterator point = rope->points.rbegin();
            std::vector<RopePoint>::reverse_iterator end = rope->points.rend() - 2;
            while (point != end) {
                RopePoint *current;
                RopePoint *previous;
                previous = &point[1];
                current = &point[0];
                Vec backwardDifference;
                PSVECSubtract(&previous->position, &current->position, &backwardDifference);
                f32 backwardDistance = vectorMagnitude(&backwardDifference);
                if (backwardDistance != spacing) {
                    if (0.0f != backwardDistance) {
                        PSVECScale(&backwardDifference, &backwardDifference,
                                   ((backwardDistance - spacing) / backwardDistance) / 2.0f);
                        PSVECAdd(&current->position, &backwardDifference, &current->position);
                        PSVECSubtract(&previous->position, &backwardDifference,
                                      &previous->position);
                    }
                }
                ++point;
            }
        }
        Vec &first = rope->points.front().position;
        first.x = firstPoint.x;
        first.y = firstPoint.y;
        first.z = firstPoint.z;
        if (rope->pinned) {
            Vec &last = rope->points.back().position;
            last.x = lastPoint.x;
            last.y = lastPoint.y;
            last.z = lastPoint.z;
        }
    }
}

// Moves the first point toward the rope target by a bounded step.
void fn_1_1EDEC(Rope *rope)
{
    Vec difference;
    f32 scale;
    f32 measuredDistance;
    f32 magnitude;
    f32 requested;
    f32 distance;
    Vec *goal;
    Vec *source;

    source = &rope->points.begin()->position;
    goal = &rope->target;
    PSVECSubtract(goal, source, &difference);
    measuredDistance = PSVECMag(&difference);
    magnitude = measuredDistance;
    distance = magnitude;
    if (!(distance <= 0.0f)) {
        requested = (f32) framesToSeconds<double>(800.0);
        scale = rangeMinimum(requested, distance);
        PSVECScale(&difference, &difference, scale / distance);
        PSVECAdd(source, &difference, &difference);
        fn_1_1DD9C(rope->points.begin(), &difference);
    }
}

// Sets up the textured pass and draws the rope.
void fn_1_1EF94(HU3D_MODEL *model, Mtx *matrix)
{
    Rope *rope = static_cast<Rope *>(model->hookData);
    fn_1_1F054(rope, *matrix);
    fn_1_1F150(rope);
    fn_1_1F514(rope, *matrix);
}

// Sets up the tinted pass and draws the rope.
void fn_1_1EFF4(HU3D_MODEL *model, Mtx *matrix)
{
    Rope *rope = static_cast<Rope *>(model->hookData);
    fn_1_1F054(rope, *matrix);
    fn_1_1F1F0(rope);
    fn_1_1F514(rope, *matrix);
}

extern "C" {
// GX vertex descriptors for the rope.
const GXVtxDescList ropeVertexDescriptors[3] = {
    { GX_VA_POS, GX_DIRECT },
    { GX_VA_TEX0, GX_DIRECT },
    { GX_VA_NULL, GX_NONE },
};
// GX vertex formats for the rope.
const GXVtxAttrFmtList ropeVertexFormats[3] = {
    { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
    { GX_VA_TEX0, GX_TEX_ST, GX_F32, 0 },
    { GX_VA_NULL, GX_POS_XY, (GXCompType) 0, 0 },
};
}

// Configures the matrix, texture, and GX state for the rope.
void fn_1_1F054(Rope *rope, Mtx matrix)
{
    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    GXSetNumChans(0);
    HuSprTexLoad(rope->animation, 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
    GXSetNumTexGens(1);
    GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_FALSE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_TRUE);
    GXSetAlphaCompare(GX_GEQUAL, 0xC0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(ropeVertexDescriptors);
    GXSetVtxAttrFmtv(GX_VTXFMT0, ropeVertexFormats);
}

// Configures the rope texture and the selected TEV color input.
static inline void configureRopeTev(GXTevColorArg color)
{
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, color);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_FALSE, GX_TEVPREV);
}

// Selects the rope texture color and alpha in the TEV stage.
void fn_1_1F150(Rope *)
{
    configureRopeTev(GX_CC_TEXC);
}

// Color applied in the tinted rope pass.
extern "C" const GXColor ropeShadowColor = { 0, 0, 0, 0x66 };

// Configures the tinted TEV stage.
void fn_1_1F1F0(Rope *)
{
    GXColor tevColor = ropeShadowColor;
    GXSetTevColor(GX_TEVREG0, tevColor);
    configureRopeTev(GX_CC_A0);
}

// Generates points around the rope axis for a cross section.
void fn_1_1F2F0(const Vec *triangle, Vec *points, const Vec *axisPoint, f32 scale, s32 count)
{
    Mtx rotation;
    Vec direction;
    Vec offset;
    Vec axis;
    Vec firstEdge;
    Vec secondEdge;
    Vec *directionAddress;
    Vec *normalizedDirection;
    Vec *offsetAddress;
    Vec *normalizedOffset;
    Vec *axisAddress;
    Vec *normalizedAxis;
    f32 firstLengthSquared;
    f32 secondLengthSquared;
    s32 index;

    PSVECSubtract(&triangle[1], &triangle[0], &firstEdge);
    PSVECSubtract(&triangle[2], &triangle[1], &secondEdge);
    firstLengthSquared = PSVECSquareMag(&firstEdge);
    if (firstLengthSquared <= 0.0f) {
        firstEdge = secondEdge;
    } else {
        secondLengthSquared = PSVECSquareMag(&secondEdge);
        if (secondLengthSquared <= 0.0f) {
            secondEdge = firstEdge;
        }
    }
    PSVECAdd(&firstEdge, &secondEdge, &direction);
    directionAddress = &direction;
    normalizedDirection = directionAddress;
    PSVECNormalize(normalizedDirection, normalizedDirection);
    PSVECSubtract(&triangle[1], axisPoint, &offset);
    offsetAddress = &offset;
    normalizedOffset = offsetAddress;
    PSVECNormalize(normalizedOffset, normalizedOffset);
    PSVECCrossProduct(&direction, &offset, &axis);
    PSVECNormalize(&axis, &axis);
    axisAddress = &axis;
    normalizedAxis = axisAddress;
    PSVECScale(normalizedAxis, normalizedAxis, scale);
    MTXRotAxisDeg(rotation, &direction, 20.0f);
    PSMTXMultVecSR(rotation, &axis, &axis);
    PSMTXRotAxisRad(rotation, &direction, (M_PI * 140.0 / 180.0) / (f32) (count - 1));
    for (index = 0; index < count; ++index) {
        PSVECAdd(&triangle[1], &axis, &points[index]);
        PSMTXMultVecSR(rotation, &axis, &axis);
    }
}

// Stores the difference between two points.
static inline Vec &subtractInto(Vec &out, const Vec &left, const Vec &right)
{
    PSVECSubtract(&left, &right, &out);
    return out;
}

// Stores the sum in the destination vector.
static inline Vec &addInto(Vec &out, const Vec &left, const Vec &right)
{
    PSVECAdd(&left, &right, &out);
    return out;
}

// Normalizes the vector in place.
static inline Vec &normalizeInPlace(Vec &value)
{
    PSVECNormalize(&value, &value);
    return value;
}

// Returns the squared length of a vector.
static inline f32 vecSquareMagnitude(const Vec *value)
{
    return PSVECSquareMag(value);
}

// Builds a cross section around the rope's local direction.
static inline void ringAround(const Vec *triangle, Vec *points, const Vec *axisPoint, f32 scale,
                              s32 count)
{
    Mtx rotation;
    Vec direction;
    Vec offset;
    Vec axis;
    Vec firstEdge;
    Vec secondEdge;
    s32 index;

    PSVECSubtract(&triangle[1], &triangle[0], &firstEdge);
    PSVECSubtract(&triangle[2], &triangle[1], &secondEdge);
    if (vecSquareMagnitude(&firstEdge) <= 0.0f) {
        firstEdge = secondEdge;
    } else {
        if (vecSquareMagnitude(&secondEdge) <= 0.0f) {
            secondEdge = firstEdge;
        }
    }
    normalizeInPlace(addInto(direction, firstEdge, secondEdge));
    normalizeInPlace(subtractInto(offset, triangle[1], *axisPoint));
    PSVECCrossProduct(&direction, &offset, &axis);
    normalizeInPlace(axis) *= scale;
    MTXRotAxisDeg(rotation, &direction, 20.0f);
    PSMTXMultVecSR(rotation, &axis, &axis);
    PSMTXRotAxisRad(rotation, &direction, (M_PI * 140.0 / 180.0) / (f32) (count - 1));
    for (index = 0; index < count; ++index) {
        addInto(points[index], triangle[1], axis);
        PSMTXMultVecSR(rotation, &axis, &axis);
    }
}

// Adds an offset to the rope point.
static inline Vec &addInPlace(const Vec &source, Vec &destination)
{
    PSVECAdd(&source, &destination, &destination);
    return destination;
}

// Returns the length of a displacement.
static inline f32 pointMagnitude(const Vec &value)
{
    return PSVECMag(&value);
}

// Builds the vertices from the rope points and draws the surface.
void fn_1_1F514(Rope *rope, Mtx matrix)
{
    float length;
    Mtx inverse;
    int pointCount = rope->points.size();
    Vec view = { 0.0f, 0.0f, 0.0f };
    float startS;

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
    length = pointMagnitude(
        subtractInto(lastTangent, centerLine[pointCount], centerLine[pointCount - 1]));
    if (length > 0.0f) {
        addInPlace(lastTangent *= 9.0f / length, centerLine[pointCount]);
    }
    centerLine[pointCount + 1] = centerLine[pointCount];
    std::vector<Vec> ringPoints(pointCount * 7);
    for (int ringSample = 0; ringSample < pointCount; ++ringSample) {
        ringAround(&centerLine[ringSample], &ringPoints[ringSample * 7], &view, 9.0f, 7);
    }
    for (int segment = 0; segment < pointCount - 1; ++segment) {
        startS = (float) (segment == pointCount - 2 ? 0.6 : 0.2);
        float endS = 0.4 + startS;
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 14);
        for (int ring = 0; ring < 7; ++ring) {
            int point = ring + segment * 7;
            Vec &left = ringPoints[point];
            Vec &right = ringPoints[point + 7];
            float t = (float) ring / 6.0f;
            GXPosition3f32(left.x, left.y, left.z);
            GXTexCoord2f32(startS, t);
            GXPosition3f32(right.x, right.y, right.z);
            GXTexCoord2f32(endS, t);
        }
    }
}
