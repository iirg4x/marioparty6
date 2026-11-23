// Moves an object through a queue of waypoints.

#include "REL/m677DLL/route_runner.h"
#include "REL/framework/range.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include <list>
#include "math.h"
#include <vector>
#include "REL/framework/vector_math.h"

#include "REL/m677DLL/path_follower.h"

// Initializes the follower's position and speed.
PathFollower::PathFollower() : speed(0.0f), angle(0.0f)
{
    vectorSet(&position, 0.0f, 0.0f, 0.0f);
}

// Initializes the follower's position and speed.
PathFollower::PathFollower(const Vec &start, f32 initialSpeed) : position(start), angle(0.0f)
{
    fn_1_22150(this, initialSpeed);
}

// Sets the follower's travel distance per frame from its speed.
void fn_1_22150(PathFollower *self, float value)
{
    float rate = 100.0f * value;
    self->speed = fn_1_226FC(rate);
}

// Returns the direction of the latest path movement.
float fn_1_221AC(PathFollower *self)
{
    return self->angle;
}

// Returns true when there are no waypoints left.
bool fn_1_221B4(PathFollower *self)
{
    return self->path.empty();
}

// Appends one waypoint to the path.
void fn_1_221D8(PathFollower *self, const Vec *point)
{
    std::vector<Vec> route(1, *point);
    fn_1_22254(self, &route);
}

// Appends a route to the remaining waypoints.
void fn_1_22254(PathFollower *self, std::vector<Vec> *route)
{
    self->path.insert(self->path.end(), route->begin(), route->end());
}

// Returns the length of a displacement.
inline double vectorLength(const Vec *value)
{
    return vectorMagnitude(value);
}

// Scales the vector in place.
inline Vec &operator*=(Vec &value, float scale)
{
    PSVECScale(&value, &value, scale);
    return value;
}

// Adds a displacement to the vector.
inline Vec &operator+=(Vec &value, const Vec &other)
{
    PSVECAdd(&other, &value, &value);
    return value;
}

// Moves toward a point by at most the requested distance and returns the unused distance.
double fn_1_222F8(Vec *position, Vec *target, double requestedDistance)
{
    Vec difference;
    PSVECSubtract(target, position, &difference);
    const double distanceToTarget = vectorLength(&difference);
    const double scale = rangeMinimum(requestedDistance, distanceToTarget);
    *position += (difference *= (float) (scale / distanceToTarget));
    return rangeMaximum(0.0, requestedDistance - distanceToTarget);
}

// Spends a frame's travel distance on the remaining path and updates the heading.
bool fn_1_22454(PathFollower *self)
{
    if (self->path.empty())
        return true;

    Vec displacement = self->position;
    double remainingDistance = self->speed;
    while (remainingDistance > 0.0) {
        Vec *nextPoint = &self->path.front();
        remainingDistance = fn_1_222F8(&self->position, nextPoint, remainingDistance);
        if (remainingDistance > 0.0f) {
            self->path.pop_front();
        }
        if (self->path.empty())
            break;
    }

    PSVECSubtract(&self->position, &displacement, &displacement);
    self->angle = MTXRadToDeg(1.0f) * atan2f(displacement.x, displacement.z);
    return self->path.empty();
}

// Converts a per-second rate to its per-frame value.
float fn_1_226FC(float value)
{
    return (1.0 / 60.0) * value;
}
