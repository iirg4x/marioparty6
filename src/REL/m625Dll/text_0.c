/* Camera setup, scripted camera transitions, and world-point projection. */
#include "REL/m625Dll/camera.h"

extern s16 lbl_1_bss_814;
#include "game/main.h"
#include "math.h"

/* Interpolates a point between camera poses with the minigame's eased timing curve. */
void fn_1_0(Point3d *result, Point3d *start, Point3d *end, s16 duration, s16 elapsed)
{
    Point3d position;
    f32 interpolation;

    interpolation = (f32)cos((3.141592653589793 *
                              (f64)((180.0f / (f32)duration) *
                                    (f32)elapsed)) /
                             180.0);
    interpolation = 1.0f - (0.5f * (1.0f + interpolation));
    position.x = end->x - start->x;
    position.y = end->y - start->y;
    position.z = end->z - start->z;
    position.x *= interpolation;
    position.y *= interpolation;
    position.z *= interpolation;
    position.x += start->x;
    position.y += start->y;
    position.z += start->z;
    *result = position;
}

/* Applies a precomputed blend factor to one camera position or target. */
static inline void interpolateCameraPoint(Point3d *result, Point3d *start,
                                           Point3d *end, f32 factor)
{
    Point3d offset;

    offset.x = end->x - start->x;
    offset.y = end->y - start->y;
    offset.z = end->z - start->z;
    offset.x *= factor;
    offset.y *= factor;
    offset.z *= factor;
    offset.x += start->x;
    offset.y += start->y;
    offset.z += start->z;
    *result = offset;
}

/* Called by sequence setup to create and position the stage camera. */
void fn_1_1B0(void)
{
    s32 scissorY;
    s32 scissorWidth;
    s32 scissorHeight;

    lbl_1_data_0.position = lbl_1_data_50.position;
    lbl_1_data_0.up = lbl_1_data_50.up;
    lbl_1_data_0.target = lbl_1_data_50.target;
    Hu3DCameraCreate(lbl_1_data_0.cameraBit);
    Hu3DCameraViewportSet(lbl_1_data_0.cameraBit, lbl_1_data_0.viewportX,
                          lbl_1_data_0.viewportY, lbl_1_data_0.viewportWidth,
                          lbl_1_data_0.viewportHeight, lbl_1_data_0.minDepth,
                          lbl_1_data_0.maxDepth);
    Hu3DCameraPerspectiveSet(lbl_1_data_0.cameraBit, lbl_1_data_0.fieldOfView,
                             lbl_1_data_0.nearPlane, lbl_1_data_0.farPlane,
                             lbl_1_data_0.aspectRatio);
    scissorHeight = (u32) lbl_1_data_0.viewportHeight;
    scissorWidth = (u32) lbl_1_data_0.viewportWidth;
    scissorY = (u32) lbl_1_data_0.viewportY;
    Hu3DCameraScissorSet(lbl_1_data_0.cameraBit,
                         (u32) lbl_1_data_0.viewportX, scissorY, scissorWidth,
                         scissorHeight);
    lbl_1_data_0.position = lbl_1_data_74[0].position;
    lbl_1_data_0.up = lbl_1_data_74[0].up;
    lbl_1_data_0.target = lbl_1_data_74[0].target;
    Hu3DCameraPosSetV(lbl_1_data_0.cameraBit, &lbl_1_data_0.position,
                      &lbl_1_data_0.up, &lbl_1_data_0.target);
}

/* Runs the start, hold, and return camera shots; the return narrows the field of view by 5
 * degrees. */
s32 fn_1_41C(s16 frame)
{
    s16 elapsed;
    f32 positionFactorA;
    f32 targetFactorA;
    f32 positionFactorB;
    f32 targetFactorB;
    Point3d position;
    Point3d target;
    Point3d positionStartA;
    Point3d positionEndA;
    Point3d positionDeltaA;
    Point3d targetStartA;
    Point3d targetEndA;
    Point3d targetDeltaA;
    Point3d positionStartB;
    Point3d positionEndB;
    Point3d positionDeltaB;
    Point3d targetStartB;
    Point3d targetEndB;
    Point3d targetDeltaB;

    if (frame < 60) {
        elapsed = frame;
        goto returnZero;
    }
    if (frame < 120) {
        elapsed = frame - 60;
        positionEndA = lbl_1_data_74[1].position;
        positionStartA = lbl_1_data_74[0].position;
        positionFactorA = (f32)cos((3.141592653589793 *
                                     (f64)(3.0508475f * (f32)elapsed)) /
                                    180.0);
        positionFactorA = 1.0f -
                          (0.5f *
                           (1.0f + positionFactorA));
        interpolateCameraPoint(&position, &positionStartA, &positionEndA,
                               positionFactorA);
        targetEndA = lbl_1_data_74[1].target;
        targetStartA = lbl_1_data_74[0].target;
        targetFactorA = (f32)cos((3.141592653589793 *
                                   (f64)(3.0508475f * (f32)elapsed)) /
                                  180.0);
        targetFactorA = 1.0f -
                        (0.5f *
                         (1.0f + targetFactorA));
        interpolateCameraPoint(&target, &targetStartA, &targetEndA,
                               targetFactorA);
        lbl_1_data_0.position = position;
        lbl_1_data_0.target = target;
        Hu3DCameraPosSetV(lbl_1_data_0.cameraBit, &lbl_1_data_0.position,
                          &lbl_1_data_0.up, &lbl_1_data_0.target);
        goto returnZero;
    }
    if (frame < 180) {
        elapsed = frame - 120;
        goto returnZero;
    }
    if (frame < 240) {
        elapsed = frame - 180;
        positionEndB = lbl_1_data_50.position;
        positionStartB = lbl_1_data_74[1].position;
        positionFactorB = (f32)cos((3.141592653589793 *
                                     (f64)(3.0508475f * (f32)elapsed)) /
                                    180.0);
        positionFactorB = 1.0f -
                          (0.5f *
                           (1.0f + positionFactorB));
        interpolateCameraPoint(&position, &positionStartB, &positionEndB,
                               positionFactorB);
        targetEndB = lbl_1_data_50.target;
        targetStartB = lbl_1_data_74[1].target;
        targetFactorB = (f32)cos((3.141592653589793 *
                                   (f64)(3.0508475f * (f32)elapsed)) /
                                  180.0);
        targetFactorB = 1.0f -
                        (0.5f *
                         (1.0f + targetFactorB));
        interpolateCameraPoint(&target, &targetStartB, &targetEndB,
                               targetFactorB);
        lbl_1_data_0.position = position;
        lbl_1_data_0.target = target;
        Hu3DCameraPosSetV(lbl_1_data_0.cameraBit, &lbl_1_data_0.position,
                          &lbl_1_data_0.up, &lbl_1_data_0.target);
        lbl_1_data_0.fieldOfView -= 0.083333336f;
        Hu3DCameraPerspectiveSet(lbl_1_data_0.cameraBit,
                                 lbl_1_data_0.fieldOfView,
                                 lbl_1_data_0.nearPlane,
                                 lbl_1_data_0.farPlane,
                                 lbl_1_data_0.aspectRatio);
        goto returnZero;
    }
    if (frame < 300) {
        elapsed = frame - 240;
        goto returnZero;
    }
    lbl_1_data_0.position = lbl_1_data_50.position;
    lbl_1_data_0.up = lbl_1_data_50.up;
    lbl_1_data_0.target = lbl_1_data_50.target;
    Hu3DCameraPosSetV(lbl_1_data_0.cameraBit, &lbl_1_data_0.position,
                      &lbl_1_data_0.up, &lbl_1_data_0.target);
    return 1;

returnZero:
    return 0;
}

void fn_1_CB0(void)
{
}

/* Called by the pre-winner callback to move the camera into the selected view. */
s32 fn_1_CB4(s16 frame)
{
    s16 frameInShot;
    f32 factorA;
    f32 factorB;
    f32 factorC;
    f32 factorD;
    Point3d position;
    Point3d target;
    Point3d positionStartA;
    Point3d positionEndA;
    Point3d positionDeltaA;
    Point3d targetStartA;
    Point3d targetEndA;
    Point3d targetDeltaA;
    Point3d positionStartB;
    Point3d positionEndB;
    Point3d positionDeltaB;
    Point3d targetStartB;
    Point3d targetEndB;
    Point3d targetDeltaB;

    if (frame < 30) {
        frameInShot = frame;
        goto finished;
    }
    if (frame < 70) {
        frameInShot = frame - 30;
        if (lbl_1_bss_814 == 0) {
        positionEndA = lbl_1_data_BC[0].position;
        positionStartA = lbl_1_data_50.position;
        factorA = (f32)cos((3.141592653589793 *
                             (f64)(4.5f * (f32)frameInShot)) /
                            180.0);
        factorA = 1.0f -
                 (0.5f * (1.0f + factorA));
        interpolateCameraPoint(&position, &positionStartA, &positionEndA,
                               factorA);

        targetEndA = lbl_1_data_BC[0].target;
        targetStartA = lbl_1_data_50.target;
        factorB = (f32)cos((3.141592653589793 *
                             (f64)(4.5f * (f32)frameInShot)) /
                            180.0);
        factorB = 1.0f -
                 (0.5f * (1.0f + factorB));
        interpolateCameraPoint(&target, &targetStartA, &targetEndA, factorB);
        } else {
        positionEndB = lbl_1_data_BC[1].position;
        positionStartB = lbl_1_data_50.position;
        factorC = (f32)cos((3.141592653589793 *
                             (f64)(4.5f * (f32)frameInShot)) /
                            180.0);
        factorC = 1.0f -
                 (0.5f * (1.0f + factorC));
        interpolateCameraPoint(&position, &positionStartB, &positionEndB,
                               factorC);

        targetEndB = lbl_1_data_BC[1].target;
        targetStartB = lbl_1_data_50.target;
        factorD = (f32)cos((3.141592653589793 *
                             (f64)(4.5f * (f32)frameInShot)) /
                            180.0);
        factorD = 1.0f -
                 (0.5f * (1.0f + factorD));
        interpolateCameraPoint(&target, &targetStartB, &targetEndB, factorD);
        }

        lbl_1_data_0.position = position;
        lbl_1_data_0.target = target;
        Hu3DCameraPosSetV(lbl_1_data_0.cameraBit,
                          &lbl_1_data_0.position,
                          &lbl_1_data_0.up,
                          &lbl_1_data_0.target);
        goto finished;
    }
    return 1;

finished:
    return 0;
}

/* Called by splash handling to project an object position onto the water plane. */
void fn_1_1398(Point3d *result, const Point3d *point, f32 distance)
{
    Point3d fromPoint;
    Point3d direction = { 0.0f, 0.0f, 1.0f };
    Point3d normal = { 0.0f, 0.0f, 0.0f };
    Point3d fallback = { 0.0f, 0.0f, 0.0f };
    Point3d offset;
    f32 denominator;
    f32 numerator;
    f32 scale;

    normal.z = distance;
    offset.x = lbl_1_data_0.position.x - point->x;
    offset.y = lbl_1_data_0.position.y - point->y;
    offset.z = lbl_1_data_0.position.z - point->z;
    fromPoint.x = normal.x - point->x;
    fromPoint.y = normal.y - point->y;
    fromPoint.z = normal.z - point->z;
    numerator = (fromPoint.z * direction.z) +
                ((fromPoint.x * direction.x) + (fromPoint.y * direction.y));
    denominator = (offset.z * direction.z) +
                  ((offset.x * direction.x) + (offset.y * direction.y));
    if (0.0 == denominator) {
        *result = fallback;
        return;
    }
    if ((0.0f != numerator) && (0.0f != denominator)) {
        scale = numerator / denominator;
    } else {
        scale = 0.0f;
    }
    fallback.x = point->x + (scale * offset.x);
    fallback.y = point->y + (scale * offset.y);
    fallback.z = point->z + (scale * offset.z);
    *result = fallback;
}

M625CameraParams lbl_1_data_0 = {
    1, 25.0f, 10.0f, 10000.0f, 1.2f,
    0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f,
    { 0.0f, 0.0f, 1000.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }
};

M625CameraPose lbl_1_data_50 = {
    { 0.0f, 0.0f, 3300.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, -100.0f, -265.0f }
};

M625CameraPose lbl_1_data_74[2] = {
    { { 0.0f, 170.0f, 1500.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 170.0f, 0.0f } },
    { { 0.0f, -350.0f, 1500.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, -350.0f, 0.0f } }
};

M625CameraPose lbl_1_data_BC[2] = {
    { { 0.0f, 170.0f, 1500.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 170.0f, 0.0f } },
    { { 0.0f, 0.0f, 2000.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } }
};
