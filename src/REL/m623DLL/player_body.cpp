// Player movement, ground collision and contact response.

extern "C" {
#include "dolphin/mtx.h"
}

#include "REL/framework/tri_colli.h"
#include "REL/framework/sphere.h"
#include "REL/framework/list_object.h"
#include "REL/m623DLL/scene_storage.h"
#include "REL/m623DLL/player_body.h"
#include "REL/framework/world_units.h"
#include "REL/framework/square.h"
#include <vector>

// Initializes the collision body and chooses its input integration callback.
PlayerBody::PlayerBody(bool primary) : GameObject(0), collisionObject(fn_1_1A2C())
{
    f32 bodyRadius;

    unk55 = primary;
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    velocity.z = 0.0f;
    positionDelta.x = 0.0f;
    positionDelta.y = 0.0f;
    positionDelta.z = 0.0f;
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    previousPosition.x = 0.0f;
    previousPosition.y = 0.0f;
    previousPosition.z = 0.0f;
    // The sphere used to sweep this body against the course.
    bodyRadius = 80.0f;
    radius = bodyRadius;
    stopOnDescent = 0;
    updateBody = primary ? &PlayerBody::fn_1_D064 : &PlayerBody::fn_1_D0D8;
    stickInput = 0.0f;
}

// Releases the collision query owned by the body.
PlayerBody::~PlayerBody()
{
}

// Copies the initial position into the body and its previous-position sample.
void fn_1_CE5C(PlayerBody *body, const Vec *position)
{
    Vec *destination = &body->position;
    Vec *currentPosition = destination;
    currentPosition->x = position->x;
    currentPosition->y = position->y;
    currentPosition->z = position->z;
    // The body center starts above the supplied ground position.
    body->position.y += 100.0f;
    body->previousPosition.x = body->position.x;
    body->previousPosition.y = body->position.y;
    body->previousPosition.z = body->position.z;
}

void fn_1_D13C(PlayerBody *body);

// A frame in seconds, in f64 precision.
static const f64 lbl_1_rodata_560 = 1.0 / 60.0;

// Applies gravity, integrates velocity and resolves terrain collision.
void PlayerBody::update()
{
    f32 startVelocity;
    // Converts the downward acceleration to the per-frame displacement step.
    velocity.y = (f32) ((f64) velocity.y - (2940.0 * square(lbl_1_rodata_560)));
    if ((stopOnDescent == 0) || !(velocity.y < 0.0f)) {
        PSVECAdd(&velocity, &position, &position);

        if (stopOnDescent == 0) {
            fn_1_D13C(this);
            // Advances the body along the course by a fixed step.
        startVelocity = cmToWorld(40.0f);
            if (currentRequest() >= 2) {
                velocity.x = startVelocity;
            }
            // Reduces lateral velocity each frame.
        velocity.z *= 0.95f;
        }

        PSVECSubtract(&position, &previousPosition, &positionDelta);
        previousPosition.x = position.x;
        previousPosition.y = position.y;
        previousPosition.z = position.z;
        stickInput = 0.0f;
    }
}

// Stores the stick input, invokes the body callback and resolves collision.
void fn_1_D000(PlayerBody *body, f32 horizontal, f32 vertical)
{
    body->stickInput = horizontal;
    (body->*body->updateBody)(horizontal, vertical);
    fn_1_D13C(body);
}

// Accelerates along Z and integrates the X input.
void PlayerBody::fn_1_D064(f32 zInput, f32 xInput)
{
    zInput = (f32) ((1.0 / 60.0) * (f64) (cmToWorld(500.0f) * zInput));
    // Applies one tenth of the scaled stick input to lateral velocity.
    velocity.z += 0.1f * zInput;
    position.x =
        (f32) ((f64) position.x + (1.0 / 60.0) * (f64) (cmToWorld(1600.0f) * xInput));
}

// Integrates the supplied Z and X inputs directly.
void PlayerBody::fn_1_D0D8(f32 zInput, f32 xInput)
{
    position.z =
        (f32) ((f64) position.z + ((1.0 / 60.0) * (f64) (cmToWorld(500.0f) * zInput)));
    position.x =
        (f32) ((f64) position.x + ((1.0 / 60.0) * (f64) (cmToWorld(1600.0f) * xInput)));
}

// Adds the plane offset to the point's dot product with its normal.
static inline f32 planeDistance(const PlaneRecord *plane, f32 dotProduct)
{
    return plane->distance + dotProduct;
}

// Sweeps the body against terrain planes and clamps its horizontal position.
void fn_1_D13C(PlayerBody *body)
{
    PlaneList result;
    s32 planeCount;
    f32 distance;
    f32 minimum;
    f32 maximum;
    s32 index;
    PlaneRecord *plane;
    f32 limited;
    f32 current;
    f32 bodyRadius;
    f32 dotProduct;
    Vector3 sweepStart = body->previousPosition;
    Vector3 sweepEnd = body->position;
    Vec correction;
    bodyRadius = body->radius;
    body->collisionObject.get()->sweepSphere(&sweepStart, &sweepEnd, bodyRadius, &result);
    index = 0;
    planeCount = result.size();
    while (index < planeCount) {
        plane = &result[index];
        dotProduct = PSVECDotProduct(&plane->normal, &body->position);
        distance = planeDistance(plane, dotProduct);
        if (!(distance >= 0.0f)) {
            PSVECScale(&plane->normal, &correction, -distance);
            PSVECAdd(&correction, &body->position, &body->position);
            PSVECAdd(&correction, &body->velocity, &body->velocity);
        }
        ++index;
    }
    if (body->position.y < 0.0f) {
        body->velocity.y += (-body->position.y);
        body->position.y = 0.0f;
    }
    maximum = fn_1_1D98();
    minimum = fn_1_1E5C();
    current = body->position.x;
    if (current < minimum) {
        limited = minimum;
    } else {
        if (current > maximum) {
            limited = maximum;
        } else {
            limited = current;
        }
    }
    body->position.x = limited;
}

// Separates two overlapping player bodies and clamps them to the scene bounds.
void fn_1_D370(PlayerBody *first, PlayerBody *second)
{
    f32 firstY;
    f32 secondY;
    f32 firstMinimum;
    f32 firstMaximum;
    f32 secondMinimum;
    f32 secondMaximum;
    f32 firstClamped;
    f32 firstCurrent;
    f32 secondClamped;
    f32 secondCurrent;

    firstY = first->position.y;
    secondY = second->position.y;
    // Splits the contact correction evenly and separates the spheres slightly farther.
    if (resolveSphereContact(reinterpret_cast<Sphere *>(&first->position),
                             reinterpret_cast<Sphere *>(&second->position), 0.5f, 1.001f) != 0) {
        first->position.y = firstY;
        second->position.y = secondY;
    }

    firstMaximum = fn_1_1D98();
    firstMinimum = fn_1_1E5C();
    firstCurrent = first->position.x;
    if (firstCurrent < firstMinimum) {
        firstClamped = firstMinimum;
    } else if (firstCurrent > firstMaximum) {
        firstClamped = firstMaximum;
    } else {
        firstClamped = firstCurrent;
    }
    first->position.x = firstClamped;

    secondMaximum = fn_1_1D98();
    secondMinimum = fn_1_1E5C();
    secondCurrent = second->position.x;
    if (secondCurrent < secondMinimum) {
        secondClamped = secondMinimum;
    } else if (secondCurrent > secondMaximum) {
        secondClamped = secondMaximum;
    } else {
        secondClamped = secondCurrent;
    }
    second->position.x = secondClamped;
}

// Clears X velocity, adds an upward impulse and enables stopping on descent.
void fn_1_D4FC(PlayerBody *body)
{
    body->velocity.x = 0.0f;
    body->velocity.y += (4000.0 / 60.0);
    body->stopOnDescent = 1;
}
