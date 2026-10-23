/* Vector and angle helpers shared by Mole-it! board logic. */
#include <math.h>

#include "dolphin/gx.h"

#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include <dolphin/mtx.h>

/* Fade-in, finish, main, pre-winner, and start callbacks normalize collision directions. */
s32 fn_1_146B8(Point3d *sourceVector, Point3d *unitVector) {
    f32 fallbackY;

    /* Tiny vectors get a normalized random fallback. Before normalization, frandmod(1) advances
     * the stream and fixes Y at -0.01. */
    if (PSVECSquareMag(sourceVector) <= 0.001) {
        unitVector->x = 0.01f * ((f32) frandmod(20) - 10.0f);
        unitVector->z = 0.01f * ((f32) frandmod(20) - 10.0f);
        if (frandmod(1) != 0) {
            fallbackY = 0.01f;
        } else {
            fallbackY = -0.01f;
        }
        unitVector->y = fallbackY;
        PSVECNormalize(unitVector, unitVector);
        return 0;
    }
    PSVECNormalize(sourceVector, unitVector);
    return 1;
}

/* Rotates a vector about X, Y, then Z in degrees; no callback calls this helper in this module. */
void fn_1_14810(f32 angle_x, f32 angle_y, f32 angle_z, const Vec *source, Vec *destination) {
    Mtx x_rotation;
    Mtx y_rotation;

    PSMTXRotRad(x_rotation, 'x', 0.017453292f * angle_x);
    PSMTXRotRad(y_rotation, 'y', 0.017453292f * angle_y);
    PSMTXConcat(y_rotation, x_rotation, x_rotation);
    PSMTXRotRad(y_rotation, 'z', 0.017453292f * angle_z);
    PSMTXConcat(y_rotation, x_rotation, x_rotation);
    PSMTXMultVecSR(x_rotation, source, destination);
}

/* Converts planar x/y components to a heading in degrees; no callback calls it in this module. */
f64 fn_1_148D0(f64 x, f64 y) {
    f64 angle;

    angle = acos(x / sqrtf((x * x) + (y * y)));
    angle = 180.0 * (angle / 3.141592653589793);
    if (y < 0.0) {
        angle = 360.0 - angle;
    }
    return angle;
}
