// Creates particle bursts and draws the trails they leave.

#include "REL/m677DLL/game_constants.h"
#include "REL/framework/interpolation.h"
#include "REL/framework/world_units.h"

#include "REL/framework/random.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
#include "REL/m677DLL/trail_burst.h"

// Adds the supplied offset to a position.
inline Vec &operator+=(Vec &value, const Vec &other)
{
    PSVECAdd(&other, &value, &value);
    return value;
}

// Stores the scaled vector in the supplied destination.
inline Vec &scaleVector(const Vec &value, Vec &result, float scale)
{
    PSVECScale(&value, &result, scale);
    return result;
}

// Creates the burst trails with randomized directions.
TrailBurst::TrailBurst(const Vec *origin, float radius, float duration, int trailCount)
    : RenderObject(10)
{
    setLayer(5);
    trails.clear();

    for (int index = 0; index < trailCount; ++index) {
        double x;
        double y;
        double z;
        z = RandomFloat(-1.0f, 1.0f);
        y = RandomFloat(-0.01f, -1.0f);
        x = RandomFloat(-1.0f, 1.0f);
        Vec start;
        Vec direction;
        direction.x = (float) x;
        direction.y = (float) y;
        direction.z = (float) z;
        PSVECNormalize(&direction, &direction);
        scaleVector(direction, start, 0.1 * radius) += *origin;
        float rate = radius / secondsToFrames<f32>(duration);
        PSVECScale(&direction, &direction, rate);
        Trail record(start, direction, radius);
        trails.push_back(record);
    }
}

// Releases all trails in the burst.
TrailBurst::~TrailBurst()
{
}

// Fills the trail's point buffer with its starting position.
Trail::Trail(const Vec &start, const Vec &velocityValue, float lifeValue)
    : velocity(velocityValue), life(lifeValue), remaining(lifeValue), head(0)
{
    points.clear();
    points.assign(8, start);
}

// Advances a position by the current velocity.
inline void addPoint(const Vec &current, const Vec &velocity, Vec &next)
{
    PSVECAdd(&current, &velocity, &next);
}

// Initial vector for the randomized velocity offset.
const Vec zeroNoise = { 0.0f, 0.0f, 0.0f };

// Picks a random offset for the trail velocity.
struct NoiseVector : Vec {
    // Initializes a randomized velocity offset.
    NoiseVector() : Vec(zeroNoise)
    {
        x = RandomFloat(-4.0f, 4.0f);
        y = RandomFloat(0.0f, 4.0f);
        z = RandomFloat(-4.0f, 4.0f);
    }
};

// Advances the point buffer and reports whether the trail is still alive.
bool Trail::update()
{
    Vec *currentPoint = pointAt(head);
    if (++head >= points.size()) {
        head = 0;
    }

    Vec *nextPoint = pointAt(head);
    addPoint(*currentPoint, velocity, *nextPoint);
    remaining -= speed();

    const NoiseVector noise;
    velocity.x = interpolateValue(velocity.x, noise.x, 0.2);
    velocity.y = interpolateValue(velocity.y, noise.y, 0.2);
    velocity.z = interpolateValue(velocity.z, noise.z, 0.2);
    return remaining > 0.0f;
}

// Updates the trails and erases expired entries.
void TrailBurst::update()
{
    std::list<Trail>::iterator trail = trails.begin();
    std::list<Trail>::iterator end = trails.end();
    // The loop increments trail after erase invalidates it on the expired path.
    for (; trail != end; ++trail) {
        if (!(*trail).update()) {
            trails.erase(trail);
        }
    }

    if (trails.empty()) {
        delete this;
    }
}

// Draws the active trails with their vertex colors.
void TrailBurst::draw(Mtx *view)
{
    GXLoadPosMtxImm(*view, GX_PNMTX0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                  GX_AF_NONE);
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
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    GXSetAlphaCompare(GX_ALWAYS, 0xFF, GX_AOP_OR, GX_ALWAYS, 0xFF);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXClearVtxDesc();
    static const GXVtxDescList vertexDescriptions[3] = {
        { GX_VA_POS, GX_DIRECT },
        { GX_VA_CLR0, GX_DIRECT },
        { GX_VA_NULL, GX_NONE },
    };
    static const GXVtxAttrFmtList vertexFormats[3] = {
        { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
        { GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0 },
        { GX_VA_NULL, GX_POS_XY, GX_U8, 0 },
    };
    GXSetVtxDescv(vertexDescriptions);
    GXSetVtxAttrFmtv(GX_VTXFMT0, vertexFormats);
    GXSetLineWidth(6, GX_TO_ZERO);
    std::list<Trail>::iterator trail = trails.begin();
    std::list<Trail>::iterator end = trails.end();
    GXBegin(GX_LINES, GX_VTXFMT0, trails.size() * 2);
    for (; trail != end; ++trail) {
        (*trail).draw();
    }
}

// Draws the trail with opacity based on its remaining life.
void Trail::draw()
{
    int alpha;
    Vec *current;
    Vec *next;

    current = pointAt(head);
    next = pointAt((head + 1) % points.size());
    alpha = (int) ((255.0f * remaining) / life);

    GXPosition3f32(current->x, current->y, current->z);
    GXColor4u8(0xFF, 0xFF, 0, (u8) alpha);
    GXPosition3f32(next->x, next->y, next->z);
    GXColor4u8(255, 0, 0, 0);
}
