#ifndef REL_M610DLL_POSITION_STATE_H
#define REL_M610DLL_POSITION_STATE_H

#include "REL/framework/vector3.h"

struct PositionVector;

/* Apply position corrections through three vector coordinates. */
struct ProjectionVector : Vector3
{
    /* Add a segment correction by value. */
    ProjectionVector &operator+=(PositionVector);
    /* Leave the vector coordinates uninitialized. */
    ProjectionVector()
    {
    }
    /* Expose the coordinates through the shared vector type. */
    const Vector3 &vector() const
    {
        return *this;
    }

    /* Copy the supplied vector coordinates. */
    ProjectionVector(const Vector3 &other) : Vector3(other)
    {
    }
    /* Copy the supplied projection vector. */
    ProjectionVector(const ProjectionVector &other) : Vector3(other)
    {
    }
    /* Scale the supplied vector into this vector. */
    ProjectionVector &setScaled(const ProjectionVector &vector, float scale)
    {
        PSVECScale(&vector, this, scale);
        return *this;
    }
    /* Scale the supplied vector into this vector. */
    ProjectionVector &setScaled(const Vector3 &vector, float scale)
    {
        PSVECScale(&vector, this, scale);
        return *this;
    }
    /* Return the dot product with the supplied vector. */
    float dot(const ProjectionVector &other) const
    {
        return PSVECDotProduct(this, &other);
    }
    /* Return the dot product with the supplied vector. */
    float dot(const Vec &other) const
    {
        return PSVECDotProduct(this, &other);
    }

    /* Return the difference of two vectors. */
    ProjectionVector operator-(const ProjectionVector &other) const
    {
        ProjectionVector difference;
        return difference.setDifference(*this, other);
    }
    /* Subtract the offset from the supplied position. */
    ProjectionVector &setDifference(const ProjectionVector &position,
                                    const ProjectionVector &offset)
    {
        PSVECSubtract(&position, &offset, this);
        return *this;
    }
    /* Return the displacement from the other vector. */
    ProjectionVector difference(const ProjectionVector &other) const
    {
        ProjectionVector difference;
        return difference.setDifference(*this, other);
    }
    /* Subtract the supplied vector in place. */
    ProjectionVector &operator-=(const ProjectionVector &other)
    {
        PSVECSubtract(this, &other, this);
        return *this;
    }
    /* Assign the three coordinates. */
    ProjectionVector &operator=(const ProjectionVector &other)
    {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }
    /* Add the supplied vector in place. */
    ProjectionVector &operator+=(const Vec &other)
    {
        PSVECAdd(this, &other, this);
        return *this;
    }
    /* Add the supplied vector in place. */
    ProjectionVector &operator+=(ProjectionVector other)
    {
        PSVECAdd(this, &other, this);
        return *this;
    }
};

/* Return a scaled copy of the vector. */
static inline ProjectionVector operator*(const ProjectionVector &vector, float scale)
{
    ProjectionVector scaled;
    return scaled.setScaled(vector, scale);
}

/* Return the sum of two vectors. */
static inline ProjectionVector operator+(const ProjectionVector &left,
                                         const ProjectionVector &right)
{
    ProjectionVector sum;
    return (PSVECAdd(&left, &right, &sum), sum.vector());
}

/* Stores one constrained endpoint and its motion history. */
struct PositionCorrectionState
{
    ProjectionVector position;
    ProjectionVector previous;
    ProjectionVector displacement;
    float mass;
    float inverseMass;

    /* Initialize the endpoint mass and its reciprocal. */
    PositionCorrectionState()
    {
        mass = 1.0f;
        inverseMass = 1.0f;
    }
    /* Apply a correction weighted by the inverse mass. */
    void addScaled(const ProjectionVector &correction)
    {
        ProjectionVector &destination = position;
        destination += correction * inverseMass;
    }
    /* Return the address of the previous integration position. */
    const ProjectionVector *previousPosition() const
    {
        return &previous;
    }
    void fn_1_12460(const ProjectionVector *);
    void fn_1_124F0(float);
    ProjectionVector fn_1_12584() const;
    ProjectionVector fn_1_125F8(float) const;
    void fn_1_126F0();
    void fn_1_12750();
    void fn_1_12794();
    void applyCorrection(const ProjectionVector *normal, f32 distance);
};

#endif
