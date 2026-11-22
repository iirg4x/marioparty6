// Declares the position and collision state shared by moving objects.

#ifndef REL_M673DLL_COLLISION_BODY_H
#define REL_M673DLL_COLLISION_BODY_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"

// A position stored as three single-precision coordinates.
struct Position3 {
    Vec value; // Coordinates stored in the position.

    // Sets all three position components.
    void set(f64 x, f64 y, f64 z)
    {
        value.x = x;
        value.y = y;
        value.z = z;
    }

    // Copies the supplied vector components.
    Position3 &operator=(const Position3 &other)
    {
        value.x = other.value.x;
        value.y = other.value.y;
        value.z = other.value.z;
        return *this;
    }
};

// Stores the position and contact state of a collision participant.
struct CollisionState {
    Position3 position; // Current position of the collision body.
    s32 unk0C; /* The game's code never reads this field. */
    s32 contactRelation; // Pending contact relation, or -1 when clear.
    u8 unk14[4]; /* No code names this field. */
    s32 relation; // Relation used by the overlap registry.
    u8 active; // Enables overlap tests for this body.

    // Clears the activity flag and initializes both collision relations to -1.
    CollisionState()
    {
        unk0C = 0;
        contactRelation = -1;
        relation = -1;
        active = 0;
    }

    // Destroys the collision state through its virtual interface.
    virtual ~CollisionState()
    {
    }

    // Returns this body's collision relation.
    s32 getRelation()
    {
        return relation;
    }

    // A kind-0 body searches only when this returns -1 and takes the first matching kind-1/2.
    // The partner receives relation 0 regardless of its pending contact.
    s32 getContactRelation()
    {
        return contactRelation;
    }

    // Returns the active flag.
    u8 isActive()
    {
        return active;
    }
};

// Adds the radius used by the collision registry.
struct CollisionBody : CollisionState {
    f32 radius; // Radius used by the overlap test.

    // Copies the supplied position into the stored position.
    void setPosition(const Position3 &p)
    {
        Position3 &destination = position;
        destination = p;
    }

    // Returns the collision radius.
    f32 getRadius() const
    {
        return radius;
    }

    // Creates a collision body with the supplied radius.
    explicit CollisionBody(f32 radius);

    // Destroys the collision body and its base state.
    virtual ~CollisionBody()
    {
    }
};

// Runs the collision checks for this frame.
void fn_1_368();

// Rejects identical X/Z origins. Otherwise the distance must be less than the sum of radii.
bool fn_1_0(CollisionBody *first, CollisionBody *second);

// Checks active collision bodies and records their contacts.
void fn_1_148();

#endif
