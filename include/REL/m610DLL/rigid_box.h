#ifndef REL_M610DLL_RIGID_BOX_H
#define REL_M610DLL_RIGID_BOX_H

#include "REL/framework/tri_colli.h"
#include <map>
#include "REL/m610DLL/collision_node.h"

#include "REL/m610DLL/debug_geometry_calls.h"

#include "REL/framework/p_info.h"

#include "REL/m610DLL/position_state.h"

// A distance constraint between two corners.
struct Segment
{
    PositionCorrectionState *begin;
    PositionCorrectionState *end;
    float length;

    /* Leave the constraint endpoints uninitialized. */
    Segment()
    {
    }
    Segment(PositionCorrectionState *, PositionCorrectionState *);
    double fn_1_11F38();
    void fn_1_11FD0();
    /* Copy the endpoints and stored constraint length. */
    Segment(const Segment &other) : begin(other.begin), end(other.end), length(other.length)
    {
    }
    /* Copy the endpoints and stored constraint length. */
    Segment &operator=(const Segment &other)
    {
        begin = other.begin;
        end = other.end;
        length = other.length;
        return *this;
    }
    /* Return the first constraint endpoint coordinates. */
    const Vec *firstPoint() const
    {
        return &begin->position;
    }
    /* Return the second constraint endpoint coordinates. */
    const Vec *lastPoint() const
    {
        return &end->position;
    }
    /* Submit the constraint endpoints to the debug cylinder hook. */
    void draw(u32 color, float width) const
    {
        fn_1_1C32C(firstPoint(), lastPoint(), width, color);
    }
};

#include "REL/framework/parameter_interpolation.h"
#include "REL/framework/vector3.h"

struct MovingModelOwner;
/* Solves the playing piece corner constraints and collision response. */
struct RigidBox
{
    PositionCorrectionState corners[8];
    /* Contact flags for the four lower corners. */
    bool resting[4];
    /* Whether the box is in the squash response state. */
    bool squashing;
    /* Distance constraints connecting the box corners. */
    std::vector<Segment> segments;
    /* Scene collision tree queried by the box contacts. */
    TriangleCollision *collision;
    PlayerInfo *player;
    /* Requested rumble type and the previously applied type. */
    s32 pendingRumbleType;
    s32 lastRumbleType;
    /* Collision registration used for the box bounds. */
    CollisionNode sphere;
    /* Axes derived from the solved box corners. */
    ProjectionVector normalX;
    ProjectionVector normalY;
    ProjectionVector normalZ;
    /* Current progress of the squash response. */
    float squash;
    /* Interpolates the timed initial phase of the squash response. */
    ParameterInterpolation squashTime;
    /* Positioned sound handles indexed by the requested sound id. */
    std::map<int, int> sounds;
    /* Tracks whether the sliding sound is muted. */
    s32 slideMode;
    /* Handle of the looping sliding sound. */
    s32 slideEffect;
    s32 unk1F8; /* The game's code never reads this field. */

    /* Return the first lower corner position. */
    ProjectionVector &positionValue()
    {
        return corners[0].position;
    }
    /* Return the first lower corner position. */
    const ProjectionVector &positionValue() const
    {
        return corners[0].position;
    }
    /* Return the opposite lower corner position. */
    const ProjectionVector &oppositeLowerCorner() const
    {
        return corners[3].position;
    }
    /* Return the other lower corner used to build the basis. */
    ProjectionVector &otherLowerCorner()
    {
        return corners[2].position;
    }
    /* Return the upper corner used to build the basis. */
    ProjectionVector &upperCorner()
    {
        return corners[4].position;
    }

    void fn_1_77E0(const Sphere *request);
    void fn_1_7CBC(const Vec *first, const Vec *second, float otherRadius);
    void fn_1_7DB8(RigidBox *secondObject);
    void fn_1_7ED0();
    void fn_1_8150();
    void fn_1_81D4(const Vec *center);
    void fn_1_83FC();
    void fn_1_89A4();
    void fn_1_89A8();
    void fn_1_8A10();
    void fn_1_8A98(s32 rumbleType);
    void fn_1_8AA0();
    void fn_1_8B04();
    void fn_1_8FD0();
    void fn_1_998C(f32 distance);
    void fn_1_9C38(f32 distance);
    void fn_1_9EE4();
    void fn_1_9F30(f32 progress);
    void fn_1_9F84();
    void fn_1_9FB0(int soundId);
    void fn_1_A048(float pitch);
    void fn_1_A0BC();
    void fn_1_A284(Mtx matrix);
    void fn_1_A5EC(Vector3 *result) const;
    bool fn_1_10544(const MovingModelOwner *);

    RigidBox(TriangleCollision *tree, PlayerInfo *owner);
    bool isSquashing() const
    {
        return squashing;
    }
    void place(const Vec *center);
    s32 collisionCorrection(const ProjectionVector *query, const ProjectionVector *position,
                            ProjectionVector *correction);
};

unsigned char fn_1_799C(const Vec *, const Vec *, float, const Vec *, float, Vec *);
void fn_1_A104(float, float *, float *);
#endif
