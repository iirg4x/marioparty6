/* Normalizes movement directions and rotates vectors for tournament animations. */
#include "dolphin/math.h"
#include "dolphin/mtx.h"
#include "game/frand.h"

/* Character movement updates normalize a direction, returning zero when a random fallback
 * replaces a nearly zero vector and one when the supplied direction is used. */
s32 fn_1_E378(Point3d *direction, Point3d *unitDirection) {
    f32 verticalSign;

    if (PSVECSquareMag(direction) <= 0.001) {
        unitDirection->x = 0.009999999776482582f * ((f32) frandmod(20) - 10.0f);
        unitDirection->z = 0.009999999776482582f * ((f32) frandmod(20) - 10.0f);
        /* A modulus of one always returns zero, so every fallback has a negative Y component. */
        if (frandmod(1) != 0) {
            verticalSign = 0.009999999776482582f;
        } else {
            verticalSign = -0.009999999776482582f;
        }
        unitDirection->y = verticalSign;
        PSVECNormalize(unitDirection, unitDirection);
        return 0;
    }
    PSVECNormalize(direction, unitDirection);
    return 1;
}

/* Character facing updates rotate a direction around X, Y and Z in that order; angles are
 * degrees. */
void fn_1_E4D0(f32 angleX, f32 angleY, f32 angleZ,
               const Vec *source, Vec *destination)
{
    Mtx combinedRotation;
    Mtx axisRotation;

    PSMTXRotRad(combinedRotation, 'x', 0.01745329238474369f * angleX);
    PSMTXRotRad(axisRotation, 'y', 0.01745329238474369f * angleY);
    PSMTXConcat(axisRotation, combinedRotation, combinedRotation);
    PSMTXRotRad(axisRotation, 'z', 0.01745329238474369f * angleZ);
    PSMTXConcat(axisRotation, combinedRotation, combinedRotation);
    PSMTXMultVecSR(combinedRotation, source, destination);
}
