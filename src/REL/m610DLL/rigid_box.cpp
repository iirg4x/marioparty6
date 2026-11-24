/* Integrates eight constrained corners and computes their contact and model transforms. */
#include "humath.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/debug_geometry.h"
#include "REL/framework/sound.h"
#include <cmath>
#include "REL/m610DLL/rigid_box.h"

extern "C"
{
#include "game/audio.h"
}

/* Set the corner positions and rebuild the box basis. */
inline void RigidBox::place(const Vec *center)
{
    const Vec *position = center;
    this->fn_1_81D4(position);
    this->fn_1_83FC();
}

/* Create the box corners, constraints, collision registration and sliding sound. */
RigidBox::RigidBox(TriangleCollision *tree, PlayerInfo *owner)
    : collision(tree), player(owner), sphere(0, this)
{
    Vec center;
    int number;
    center.x = 0.0f;
    center.y = 0.0f;
    center.z = 0.0f;
    place(&center);

    int currentIndex;
    int nextIndex;
    int index;
    for (currentIndex = 0; currentIndex < 7; ++currentIndex)
    {
        for (nextIndex = currentIndex + 1; nextIndex < 8; ++nextIndex)
        {
            Segment segment(&corners[currentIndex], &corners[nextIndex]);
            segments.push_back(segment);
        }
    }

    for (index = 0; index < 8; ++index)
    {
        corners[index].mass = (float) (index < 4 ? 10 : 1);
        corners[index].inverseMass = 1.0f / (float) (index < 4 ? 10 : 1);
    }

    resting[0] = 0;
    resting[1] = 0;
    resting[2] = 0;
    resting[3] = 0;
    squashing = 0;
    float maxDistance = 150.0f;
    sphere.radius = maxDistance;
    squash = 1.0f;
    pendingRumbleType = -1;
    lastRumbleType = -1;

    number = player->number();
    slideMode = 0;
    slideEffect = soundPlay(number + 0x67A);
    unk1F8 = -1;
}

/* Resolve a sphere contact and apply its bounded horizontal correction to every corner. */
void RigidBox::fn_1_77E0(const Sphere *request)
{
    Sphere requestCopy(*request);
    Vec previousReference = this->sphere.center;
    Vector3 shift;
    resolveSphereContact(&requestCopy, &this->sphere, 0.0f, 1.0f);
    PSVECSubtract(&this->sphere.center, &previousReference, &shift);
    shift.y = 0.0f;
    float currentMagnitude;
    float measuredMagnitude = PSVECMag(&shift);
    currentMagnitude = measuredMagnitude;
    if (currentMagnitude > 10.0f)
    {
        float measuredNormalizationMagnitude = PSVECMag(&shift);
        float normalizationMagnitude = measuredNormalizationMagnitude;
        shift /= normalizationMagnitude;
        shift *= 10.0f;
    }

    for (int index = 0; index < 8; ++index)
        PSVECAdd(&this->corners[index].position, &shift, &this->corners[index].position);
}

/* Find the correction between a point sphere and a finite capsule. */
unsigned char fn_1_799C(const Vec *start, const Vec *end, float radius, const Vec *point,
                        float tolerance, Vec *correction)
{
    Vec direction;
    Vec nearestPoint;
    Vector3 relative;
    Vector3 endpointOffset;
    float startProjection;
    float endProjection;
    float pointProjection;
    float distance;
    float endpointDistance;
    const Vec *endpoint;

    PSVECSubtract(end, start, &direction);
    PSVECNormalize(&direction, &direction);
    startProjection = vectorDot(&direction, start);
    endProjection = vectorDot(&direction, end);
    pointProjection = vectorDot(&direction, point);
    if ((pointProjection + tolerance) < startProjection)
        return 0;
    if ((pointProjection - tolerance) > endProjection)
        return 0;

    PSVECScale(&direction, &nearestPoint, pointProjection - startProjection);
    PSVECAdd(start, &nearestPoint, &nearestPoint);
    PSVECSubtract(point, &nearestPoint, &relative);
    distance = relative.length();
    if (distance > (radius + tolerance))
        return 0;

    if ((pointProjection >= startProjection) && (pointProjection <= endProjection) &&
        (distance != 0.0f))
    {
        PSVECScale(&relative, correction, ((radius + tolerance) - distance) / distance);
        return 1;
    }

    if (distance < radius)
    {
        float boundary;
        float segmentMidpoint = 0.5f * (startProjection + endProjection);
        if (pointProjection < segmentMidpoint)
            boundary = startProjection - tolerance;
        else
            boundary = endProjection + tolerance;
        float selectedBoundary = boundary;
        PSVECScale(&direction, correction, selectedBoundary - pointProjection);
        return 1;
    }

    PSVECScale(&relative, &relative, radius / distance);
    if (pointProjection < startProjection)
        endpoint = start;
    else
        endpoint = end;
    PSVECAdd(endpoint, &relative, &nearestPoint);
    PSVECSubtract(point, &nearestPoint, &endpointOffset);
    endpointDistance = endpointOffset.length();
    if (endpointDistance > tolerance)
        return 0;
    if (endpointDistance != 0.0f)
        PSVECScale(&endpointOffset, correction, (tolerance - endpointDistance) / endpointDistance);
    return 1;
}

/* Resolve a capsule contact and move all box corners by its horizontal correction. */
void RigidBox::fn_1_7CBC(const Vec *first, const Vec *second, float otherRadius)
{
    Vector3 shift;
    shift.x = 0.0f;
    shift.y = 0.0f;
    shift.z = 0.0f;
    float radius = this->sphere.radius;
    if (fn_1_799C(first, second, otherRadius, &this->sphere.center, radius, &shift) != 0)
    {
        shift.y = 0.0f;
        for (int index = 0; index < 8; index++)
        {
            PSVECAdd(&this->corners[index].position, &shift, &this->corners[index].position);
        }
        this->fn_1_8A98(1);
        this->fn_1_9FB0(0x66E);
    }
}

/* Resolve contact between two boxes and shift their corners with their spheres. */
void RigidBox::fn_1_7DB8(RigidBox *secondObject)
{
    Vec firstReference = this->sphere.center;
    Vec secondReference = secondObject->sphere.center;
    if (resolveSphereContact(&this->sphere, &secondObject->sphere, 0.5f, 1.0f))
    {
        this->fn_1_8A98(1);
        secondObject->fn_1_8A98(1);
        Vec firstShift;
        Vec secondShift;
        PSVECSubtract(&this->sphere.center, &firstReference, &firstShift);
        PSVECSubtract(&secondObject->sphere.center, &secondReference, &secondShift);
        for (int index = 0; index < 8; index++)
        {
            PSVECAdd(&this->corners[index].position, &firstShift, &this->corners[index].position);
            PSVECAdd(&secondObject->corners[index].position, &secondShift,
                     &secondObject->corners[index].position);
        }
    }
}

/* Return the signed distance of a point from a collision plane. */
static inline float signedDistance(const PlaneRecord &plane, const Vec &point)
{
    return plane.distance + vectorDot(&plane.normal, &point);
}

/* Push the box out of whatever its sphere touches. */
void RigidBox::fn_1_7ED0()
{
    PlaneList contacts;
    Vec correction;
    ProjectionVector scaledHit;
    int hitLimit;
    int hitIndex;
    PlaneRecord *hit;
    float penetration;
    correction.x = 0.0f;
    correction.y = 0.0f;
    correction.z = 0.0f;
    this->collision->sweepSphere(reinterpret_cast<const Vector3 *>(&this->sphere.center),
                                 reinterpret_cast<const Vector3 *>(&this->sphere.center),
                                 0.8f * this->sphere.getRadius(), &contacts);
    hitLimit = contacts.size();
    hitIndex = 0;
    while (hitIndex < hitLimit)
    {
        hit = &contacts[hitIndex];
        penetration = signedDistance(*hit, this->sphere.center);
        if (!(penetration >= 0.0f))
        {
            scaledHit.setScaled(hit->normal, -penetration);
            PSVECAdd(&scaledHit, &correction, &correction);
        }
        ++hitIndex;
    }
    correction.y = 0.0f;
    PSVECScale(&correction, &correction, 0.98f);
    for (int index = 0; index < 8; index++)
    {
        this->corners[index].position += correction;
    }
}

/* Set the lower and upper corner heights. */
void RigidBox::fn_1_8150()
{
    this->corners[0].position.y = 0.0f;
    this->corners[1].position.y = 0.0f;
    this->corners[2].position.y = 0.0f;
    this->corners[3].position.y = 0.0f;
    this->corners[4].position.y = 130.0f;
    this->corners[5].position.y = 130.0f;
    this->corners[6].position.y = 130.0f;
    this->corners[7].position.y = 130.0f;
}

/* Place the eight corners around the supplied center and reset their motion. */
void RigidBox::fn_1_81D4(const Vec *center)
{
    float x = center->x;
    float y = center->y;
    float z = center->z;

    y += 40.0f;

    this->corners[0].position.x = -100.0f + x;
    this->corners[0].position.y = y;
    this->corners[0].position.z = -100.0f + z;
    this->corners[1].position.x = 100.0f + x;
    this->corners[1].position.y = y;
    this->corners[1].position.z = -100.0f + z;
    this->corners[2].position.x = -100.0f + x;
    this->corners[2].position.y = y;
    this->corners[2].position.z = 100.0f + z;
    this->corners[3].position.x = 100.0f + x;
    this->corners[3].position.y = y;
    this->corners[3].position.z = 100.0f + z;
    this->corners[4].position.x = -100.0f + x;
    this->corners[4].position.y = 130.0f + y;
    this->corners[4].position.z = -100.0f + z;
    this->corners[5].position.x = 100.0f + x;
    this->corners[5].position.y = 130.0f + y;
    this->corners[5].position.z = -100.0f + z;
    this->corners[6].position.x = -100.0f + x;
    this->corners[6].position.y = 130.0f + y;
    this->corners[6].position.z = 100.0f + z;
    this->corners[7].position.x = 100.0f + x;
    this->corners[7].position.y = 130.0f + y;
    this->corners[7].position.z = 100.0f + z;
    this->fn_1_9EE4();
}

/* Vector arithmetic for the box basis directions. */
struct BoxVector
{
    Vec value;
    /* Leave the vector components uninitialized. */
    BoxVector()
    {
    }
    /* Copy the three vector components. */
    BoxVector(const BoxVector &other) : value(other.value)
    {
    }

    /* Return the vector length. */
    float length() const
    {
        return PSVECMag(&value);
    }
    /* Return a unit direction in the same direction. */
    BoxVector normalized() const
    {
        BoxVector result;
        Vector3::divide(value, result.value, length());
        return result;
    }
};

/* Return the difference between two corner vectors. */
static inline BoxVector boxSubtract(const ProjectionVector &left, const ProjectionVector &right)
{
    BoxVector result;
    PSVECSubtract(&left, &right, &result.value);
    return result;
}

/* Return the vector multiplied by a scalar. */
static inline BoxVector operator*(BoxVector vector, float scale)
{
    BoxVector result;
    PSVECScale(&vector.value, &result.value, scale);
    return result;
}

/* Return the sum of two box vectors. */
static inline BoxVector operator+(BoxVector left, const BoxVector &right)
{
    BoxVector result;
    PSVECAdd(&left.value, &right.value, &result.value);
    return result;
}

/* Copy a normalized box direction into the stored basis. */
static inline void boxSetNormal(ProjectionVector &normal, const BoxVector &vector)
{
    normal.x = vector.value.x;
    normal.y = vector.value.y;
    normal.z = vector.value.z;
}

/* Rebuild the box basis and sphere center from the corner positions. */
void RigidBox::fn_1_83FC()
{
    boxSetNormal(this->normalX,
                 (boxSubtract(this->corners[1].position, this->corners[0].position) +
                  boxSubtract(this->corners[3].position, this->corners[2].position) * 0.5f)
                     .normalized());
    boxSetNormal(this->normalY,
                 (boxSubtract(this->corners[4].position, this->corners[0].position) +
                  boxSubtract(this->corners[5].position, this->corners[1].position) * 0.5f)
                     .normalized());
    boxSetNormal(this->normalZ,
                 (boxSubtract(this->corners[0].position, this->corners[2].position) +
                  boxSubtract(this->corners[1].position, this->corners[3].position) * 0.5f)
                     .normalized());

    this->sphere.center.x = (this->corners[0].position.x + this->corners[3].position.x) / 2;
    this->sphere.center.y = (this->corners[0].position.y + this->corners[3].position.y) / 2;
    this->sphere.center.z = (this->corners[0].position.z + this->corners[3].position.z) / 2;
}

/* Leave this box update hook empty. */
void RigidBox::fn_1_89A4()
{
}

/* Apply the saved corner displacements, resetting them while squashed. */
void RigidBox::fn_1_89A8()
{
    s32 index;

    if (this->isSquashing())
    {
        this->fn_1_9EE4();
    }
    index = 0;
    while (index < 8)
    {
        this->corners[index].fn_1_12750();
        index += 1;
    }
}

// Submits the constraint endpoints to the debug cylinder hook.
inline void Segment::draw(u32 color, float width) const
{
    fn_1_1C32C(firstPoint(), lastPoint(), width, color);
}

/* Draw the box's constraints. */
void RigidBox::fn_1_8A10()
{
    s32 count = this->segments.size();
    for (s32 index = 0; index < count; ++index)
    {
        this->segments[index].draw(0xCCCCFFFF, 5.0f);
    }
}

/* Record the pending rumble type. */
void RigidBox::fn_1_8A98(s32 rumbleType)
{
    this->pendingRumbleType = rumbleType;
}

/* Apply a newly increased rumble request and clear the pending type. */
void RigidBox::fn_1_8AA0()
{
    if ((this->lastRumbleType < this->pendingRumbleType) && (this->pendingRumbleType >= 0))
    {
        this->player->rumble(this->pendingRumbleType, 20);
    }
    this->lastRumbleType = this->pendingRumbleType;
    this->pendingRumbleType = -1;
}

/* Solve the box constraints, gravity, contact response and squash transition. */
void RigidBox::fn_1_8B04()
{
    this->fn_1_8AA0();
    for (s32 repeat = 10; repeat != 0; repeat--)
    {
        s32 itemCount = this->segments.size();
        for (s32 index = 0; index < itemCount; index++)
        {
            this->segments[index].fn_1_11FD0();
        }
    }
    for (s32 index = 0; index < 8; index++)
    {
        this->corners[index].position.y += -((1.0 / 60.0) * (1.0 / 60.0) * 980.665);
    }
    for (s32 index = 0; index < 4; index++)
    {
        if (!(this->resting[index]))
        {
            ProjectionVector normal = this->corners[index].fn_1_12584();
            float normalProjection = normal.dot(this->normalX);
            normal = (this->normalX * -normalProjection) * 1.0f;
            normal.y = 0.0f;
            this->corners[index].addScaled(normal);
        }
    }
    this->fn_1_83FC();
    s32 itemCount = 8;
    for (s32 index = 0; index < itemCount; index++)
    {
        this->corners[index].fn_1_12794();
    }
    this->fn_1_8FD0();
    if (this->isSquashing())
    {
        if (this->squash <= 0.1f)
        {
            this->pendingRumbleType = 2;
        }
        if (this->squashTime.elapsed > this->squashTime.duration)
        {
            this->squash += 1.0f / secondsToFrames(0.9f);
            if (this->squash >= 1.0f)
            {
                this->squash = 1.0f;
                this->squashing = 0;
            }
        }
        else
        {
            this->squash = this->squashTime.update();
        }
    }
}

/* Collect a corner's contacts and sum how far they push it out; returns how many there were. */
s32 RigidBox::collisionCorrection(const ProjectionVector *query, const ProjectionVector *position,
                                  ProjectionVector *correction)
{
    PlaneList contacts;
    ProjectionVector scaledDirection;
    s32 count;
    s32 index;
    f32 penetration;
    correction->x = 0.0f;
    correction->y = 0.0f;
    correction->z = 0.0f;
    collision->sweepSphere(query, position, 40.0f, &contacts);
    count = contacts.size();
    index = 0;
    while (index < count)
    {
        PlaneRecord &hit = contacts[index];
        penetration = signedDistance(hit, *position);
        if (!(penetration >= 0.0f))
        {
            scaledDirection.setScaled(hit.normal, -penetration);
            PSVECAdd(&scaledDirection, correction, correction);
        }
        ++index;
    }
    return count;
}

/* Correct the lower corners and edge midpoints against the collision tree. */
void RigidBox::fn_1_8FD0()
{
    s32 hitCount;
    s32 index;
    ProjectionVector correction;

    this->fn_1_7ED0();

    index = 0;
    while (index < 4)
    {
        hitCount = this->collisionCorrection(this->corners[index].previousPosition(),
                                             &this->corners[index].position, &correction);
        PSVECAdd(&correction, &this->corners[index].position, &this->corners[index].position);
        this->corners[index].position.y = 40.0f;
        if (hitCount != 0)
            this->fn_1_9FB0(0x66A);
        ++index;
    }

    ProjectionVector pairMidpoint = (this->corners[0].position + this->corners[1].position) * 0.5f;
    this->collisionCorrection(&pairMidpoint, &pairMidpoint, &correction);
    PSVECAdd(&this->corners[0].position, &correction, &this->corners[0].position);
    PSVECAdd(&this->corners[1].position, &correction, &this->corners[1].position);

    ProjectionVector oppositePairMidpoint =
        (this->corners[2].position + this->corners[3].position) * 0.5f;
    this->collisionCorrection(&oppositePairMidpoint, &oppositePairMidpoint, &correction);
    PSVECAdd(&this->corners[2].position, &correction, &this->corners[2].position);
    PSVECAdd(&this->corners[3].position, &correction, &this->corners[3].position);
}

/* Adjust horizontal displacement along the supplied normal. */
void PositionCorrectionState::applyCorrection(const ProjectionVector *normal, f32 distance)
{
    ProjectionVector displacement = fn_1_12584();
    f32 projection = normal->dot(displacement);
    ProjectionVector correction = *normal * (distance - projection);
    correction.y = 0.0f;
    addScaled(correction);
}

/* Apply the forward displacement to the first lower corner pair. */
void RigidBox::fn_1_998C(f32 distance)
{
    if (!this->isSquashing())
    {
        if (!this->resting[0])
        {
            this->corners[0].applyCorrection(&this->normalZ, distance);
        }

        if (!this->resting[2])
        {
            this->corners[2].applyCorrection(&this->normalZ, distance);
        }
    }
}

/* Apply the forward displacement to the opposite lower corner pair. */
void RigidBox::fn_1_9C38(f32 distance)
{
    if (!this->isSquashing())
    {
        if (!this->resting[1])
            this->corners[1].applyCorrection(&this->normalZ, distance);
        if (!this->resting[3])
            this->corners[3].applyCorrection(&this->normalZ, distance);
    }
}

/* Reset the motion state of all eight corners. */
void RigidBox::fn_1_9EE4()
{
    s32 index = 0;
    while (index < 8)
    {
        this->corners[index].fn_1_126F0();
        ++index;
    }
}

/* Reduce the displacement of the four lower corners. */
void RigidBox::fn_1_9F30(f32 progress)
{
    s32 index = 0;
    while (index < 4)
    {
        this->corners[index].fn_1_124F0(progress);
        ++index;
    }
}

/* Play the squash sound at the box position. */
void RigidBox::fn_1_9F84()
{
    this->fn_1_9FB0(0x676);
}

/* Play the positioned sound when its previous handle is no longer playing. */
void RigidBox::fn_1_9FB0(int soundId)
{
    Vector3 position;
    const PlayerInfo *player;
    int status;
    int number;
    int base;

    status = HuAudFXStatusGet(this->sounds[soundId]);
    if (status != 2)
    {
        this->fn_1_A5EC(&position);
        player = this->player;
        number = player->playerNo;
        base = number;
        this->sounds[soundId] = soundPlayAt(1, soundId + base, &position);
    }
}

/* Restore the active effect's volume and scale its pitch from the supplied amount. */
void RigidBox::fn_1_A048(float pitch)
{
    if (this->slideMode != 0)
        this->slideMode = 0;
    HuAudFXVolSet(this->slideEffect, 127);
    HuAudFXPitchSet(this->slideEffect, 8191.0f * pitch);
}

/* Mute the sliding effect when entering its stopped mode. */
void RigidBox::fn_1_A0BC()
{
    if (this->slideMode != 1)
    {
        this->slideMode = 1;
        HuAudFXVolSet(this->slideEffect, 0);
    }
}

/* Evaluate the oscillating squash scales for the supplied progress. */
void fn_1_A104(float progress, float *current, float *target)
{
    float phase;
    float factor;

    phase = 9.0f * ((float) M_PI * (progress * progress));
    factor = (float) ((double) (1.0f - progress) * std::sin((double) phase));
    if (phase < (float) M_PI)
    {
        factor *= 2.0f;
    }
    *target = progress + (1.4f * factor);
    *current = 1.0f + (0.3f * (1.0f - *target));
    if (*target > 1.0f)
    {
        *target -= 1.0f;
        *target *= (2.0f / 3.0f);
        *target += 1.0f;
    }
}

/* Apply a translation to the supplied matrix in place. */
static inline void translateBoxMatrix(Mtx matrix, float x, float y, float z)
{
    PSMTXTransApply(matrix, matrix, x, y, z);
}

/* Build the box orientation, translation and animated squash matrix. */
void RigidBox::fn_1_A284(Mtx matrix)
{
    Mtx scaleMatrix;
    float currentScale;
    float targetScale;

    const ProjectionVector &position = this->positionValue();
    const ProjectionVector &upperCornerPosition = this->upperCorner();
    const ProjectionVector &lowerCornerPosition = this->otherLowerCorner();
    ProjectionVector upperEdge = upperCornerPosition - position;
    ProjectionVector lowerEdge = lowerCornerPosition - position;
    Vector3 boxPosition;
    boxPosition.x = 0.0f;
    boxPosition.y = 0.0f;
    boxPosition.z = 0.0f;
    C_MTXLookAt(matrix, &boxPosition, &upperEdge, &lowerEdge);
    PSMTXTranspose(matrix, matrix);
    this->fn_1_A5EC(&boxPosition);
    translateBoxMatrix(matrix, boxPosition.x, boxPosition.y, boxPosition.z);

    if ((u8) this->squashing != 0)
    {
        if (this->squashTime.elapsed > this->squashTime.duration)
        {
            fn_1_A104(this->squash, &currentScale, &targetScale);
        }
        else
        {
            currentScale = 1.0f;
            targetScale = this->squash;
        }
        if (currentScale < 0.05f)
            currentScale = 0.05f;
        if (targetScale < 0.05f)
            targetScale = 0.05f;
        PSMTXScale(scaleMatrix, currentScale, targetScale, currentScale);
        PSMTXConcat(matrix, scaleMatrix, matrix);
    }
}

/* Return the lower face midpoint adjusted down to the model origin. */
void RigidBox::fn_1_A5EC(Vector3 *result) const
{
    *result = (this->positionValue() + this->oppositeLowerCorner()) * 0.5f;
    result->y -= 40.0f;
}

/* Advance and return the linear transition value. */
float ParameterInterpolation::update()
{
    float value = this->value;
    if (this->elapsed <= this->duration)
    {
        double elapsed = (double) ++this->elapsed;
        double fraction = elapsed / (double) (this->duration + 1);
        value = (float) (this->from * (1.0 - fraction) + this->to * fraction);
        this->value = value;
    }
    return value;
}

/* Start a transition from the current value. */
void ParameterInterpolation::set(float end, int frames)
{
    set(value, end, frames);
}

/* Initialize a transition, completing it immediately when its duration is zero. */
void ParameterInterpolation::set(float start, float end, int frames)
{
    this->duration = frames;
    this->elapsed = 0;
    if (frames == 0)
    {
        this->value = end;
        this->from = end;
        this->to = end;
    }
    else
    {
        this->value = start;
        this->from = start;
        this->to = end;
    }
}
