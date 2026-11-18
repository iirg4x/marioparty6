/* Store segment endpoints and measure the distance between them. */
#include "REL/m610DLL/rigid_box.h"

/* Set the segment endpoints and cache their distance. */
Segment::Segment(PositionCorrectionState *begin, PositionCorrectionState *end)
{
    this->begin = begin;
    this->end = end;
    length = fn_1_11F38();
}

/* Vector arithmetic used to correct segment length. */
struct PositionVector : Vec
{
    /* Leave the vector components uninitialized. */
    PositionVector()
    {
    }
    /* Copy the supplied vector. */
    PositionVector(const Vec *other) : Vec(*other)
    {
    }
    /* Copy the correction vector. */
    PositionVector(const PositionVector &other) : Vec(other)
    {
    }
    /* Return the vector length. */
    float magnitude() const
    {
        return PSVECMag(this);
    }
    /* Store the displacement from the second vector to the first. */
    PositionVector &setDifference(const Vec &a, const Vec &b)
    {
        PSVECSubtract(&a, &b, this);
        return *this;
    }
    /* Store a scaled copy of the supplied vector. */
    PositionVector &setScaled(const PositionVector &a, float scale)
    {
        PSVECScale(&a, this, scale);
        return *this;
    }
};
/* Return the displacement between the two vectors. */
static inline PositionVector operator-(const Vec &begin, const Vec &end)
{
    PositionVector difference;
    return difference.setDifference(begin, end);
}

/* Return the current endpoint separation. */
double Segment::fn_1_11F38()
{
    return (static_cast<const Vec &>(this->begin->position) -
            static_cast<const Vec &>(this->end->position))
        .magnitude();
}

/* Return a scaled copy of a constraint direction. */
static inline PositionVector scaleVector(const PositionVector &value, float scale)
{
    PositionVector result;
    return result.setScaled(value, scale);
}
/* Return the corner mass used to divide the constraint correction. */
static inline float getMass(const PositionCorrectionState *state)
{
    return state->mass;
}
/* Add a correction through the endpoint's actual vector type. */
inline ProjectionVector &ProjectionVector::operator+=(PositionVector correction)
{
    PSVECAdd(this, &correction, this);
    return *this;
}

/* Add a constraint correction to a corner position. */
static inline void addCorrection(PositionCorrectionState *state, const PositionVector &correction)
{
    ProjectionVector &position = state->position;
    position += scaleVector(correction, state->inverseMass);
}

/* Correct the endpoint distance using their relative masses. */
void Segment::fn_1_11FD0()
{
    PositionVector difference = static_cast<const Vec &>(this->begin->position) -
                                static_cast<const Vec &>(this->end->position);
    float distance = difference.magnitude();
    float beginMass = getMass(this->begin);
    float endMass = getMass(this->end);
    float inverse = 1.0f / (beginMass + endMass);
    beginMass *= inverse;
    endMass *= inverse;
    addCorrection(
        this->begin,
        scaleVector(scaleVector(scaleVector(difference, this->length - distance), 0.0005f),
                    endMass));
    addCorrection(
        this->end,
        scaleVector(scaleVector(scaleVector(difference, distance - this->length), 0.0005f),
                    beginMass));
}
