/* Collision tests and the minigame module entry and exit routines. */
#include "dolphin/math.h"
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

#define M625_OBJECT_SLOT_COUNT 300
#define M625_PROCESS_STACK_SIZE 4096
#define M625_SEQUENCE_PRIORITY 2000

/* Called by falling-object collision checks to test the distance between two centers. */
s32 fn_1_13318(Point3d point, f32 radius_a, Point3d center, f32 radius_b)
{
    radius_a += radius_b;
    radius_a *= radius_a;

    PSVECSubtract(&point, &center, &point);
    radius_b = PSVECSquareMag(&point);
    if (radius_b < radius_a) {
        return 1;
    }
    return 0;
}

/* Tests a swept position against a plane offset by distance; on contact it clips movement along the
 * plane normal. */
s32 fn_1_133A4(Point3d point, Point3d *direction, f32 distance, Point3d origin,
               Point3d normal, f32 amount)
{
    Point3d offset;
    Point3d scaledDirection;
    Point3d result;
    f32 directionProjection;
    f32 normalDistance;
    f32 normalComponent;
    f32 parameter;
    f32 amountAtLowerBound;
    f32 amountAtUpperBound;
    s32 lowerBounded;
    s32 upperBounded;

    origin.z = normal.z = 0.0f;
    PSVECSubtract(&point, &origin, &offset);
    directionProjection = PSVECDotProduct(&normal, &offset);
    if (directionProjection <= 0.0f) {
        return 0;
    }
    directionProjection = PSVECDotProduct(&normal, direction);
    if (directionProjection >= 0.0f) {
        return 0;
    }
    scaledDirection = normal;
    PSVECScale(&scaledDirection, &scaledDirection, distance);
    PSVECAdd(&scaledDirection, &origin, &scaledDirection);
    PSVECSubtract(&point, &scaledDirection, &offset);
    parameter = PSVECDotProduct(&normal, &offset);
    parameter = -parameter / directionProjection;
    lowerBounded = 0;
    if (-0.0001f <= parameter && parameter <= 0.0f) {
        lowerBounded = 1;
    }
    if (lowerBounded != 0) {
        parameter = 0.0f;
        normalDistance = parameter;
    } else {
        normalDistance = 0.0f;
    }
    upperBounded = 0;
    if (1.0001f >= parameter && parameter >= 1.0f) {
        upperBounded = 1;
    }
    if (upperBounded != 0) {
        parameter = 1.0f;
        normalComponent = parameter;
    } else {
        normalComponent = 0.0f;
    }
    if (0.0f <= parameter && parameter <= 1.0f) {
        result = *direction;
        PSVECScale(&result, &result, parameter);
        PSVECAdd(&result, &point, &result);
        scaledDirection = normal;
        PSVECScale(&scaledDirection, &scaledDirection, distance);
        PSVECSubtract(&result, &scaledDirection, &result);
        if (amount < 0.0f) {
            amountAtLowerBound = amount = 0.0f;
        } else {
            amountAtLowerBound = 0.0f;
        }
        if (amount > 1.0f) {
            amountAtUpperBound = amount = 1.0f;
        } else {
            amountAtUpperBound = 0.0f;
        }
        amount = 1.0f - amount;
        scaledDirection = *direction;
        PSVECScale(&scaledDirection, &scaledDirection,
                   1.0f - parameter);
        PSVECScale(direction, direction, parameter);
        directionProjection = PSVECDotProduct(&scaledDirection, &normal);
        offset = normal;
        PSVECScale(&offset, &offset, directionProjection);
        PSVECSubtract(&scaledDirection, &offset, &scaledDirection);
        PSVECScale(&scaledDirection, &scaledDirection, amount);
        PSVECAdd(direction, &scaledDirection, direction);
        return 1;
    }
    return 0;
}

/* Tests overlap using radii equal to 43% of each height, overwrites the supplied radii, ignores
* rotations and the second movement argument, and removes inward normal motion from the first. */
s32 fn_1_137DC(Point3d point, Point3d unusedPointA,
               Point3d *direction, f32 firstRadius, f32 firstDiameter,
               Point3d otherPoint, Point3d unusedPointB, Point3d *otherDirection,
               f32 secondRadius, f32 secondDiameter)
{
    Point3d directionSum;
    Point3d otherDirectionSum;

    firstRadius = 0.86f * (firstDiameter / 2.0f);
    secondRadius = 0.86f * (secondDiameter / 2.0f);

    PSVECAdd(&point, (Point3d *)&direction, &directionSum);
    PSVECAdd(&otherPoint, (Point3d *)&otherDirection,
             &otherDirectionSum);
    if (point.x == otherPoint.x) {
        point.x += 0.01f;
    }
    if (point.y == otherPoint.y) {
        point.y += 0.01f;
    }
    if (fn_1_13318(point, firstRadius, otherPoint, secondRadius) != 0) {
        PSVECSubtract(&point, &otherPoint, &directionSum);
        if (directionSum.x < -0.01f || 0.01f < directionSum.x ||
            directionSum.y < -0.01f || 0.01f < directionSum.y) {
            PSVECNormalize(&directionSum, &directionSum);
        } else {
            directionSum.x = directionSum.y = directionSum.z = 0.0f;
        }
        firstDiameter = PSVECDotProduct(&directionSum, direction);
        PSVECScale(&directionSum, &directionSum, firstDiameter);
        PSVECSubtract(direction, &directionSum, direction);
        if (firstDiameter >= 0.0f) {
            PSVECAdd(direction, &directionSum, direction);
        }
        return 1;
    }
    return 0;
}

/* Used by stage logic to test whether a position is strictly inside a rectangle. */
s32 fn_1_13AA8(f32 top, f32 bottom, f32 left, f32 right, Point3d position) {
    if (left < position.x && position.x < right && bottom < position.y && position.y < top) {
        return 1;
    }
    return 0;
}

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_13B88(void);

/* Called when the REL loads to run its static constructors and initialize the minigame. */
int _prolog(void)
{
    const VoidFunc *constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    fn_1_13B88();
    return 0;
}

/* Called when the REL unloads to run its static destructors. */
void _epilog(void)
{
    const VoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}

void fn_1_13E14(s16 mode, s16 frameNo);

/* static */
extern HUPROCESS *lbl_1_bss_810;

extern MGSEQ_PARAM lbl_1_data_448;

/* Called by the REL prolog to create the minigame object manager and sequence. */
void fn_1_13B88(void) {
    lbl_1_bss_810 = omInitObjMan(M625_OBJECT_SLOT_COUNT, M625_PROCESS_STACK_SIZE);
    omGameSysInit(lbl_1_bss_810);
    MgSeqCreatePrio(&lbl_1_data_448, M625_SEQUENCE_PRIORITY);
    MgSeqModeHookAdd(4, fn_1_13E14);
    MgSeqModeDelaySet(2, -1);
    MgSeqModeDelaySet(7, -1);
}
