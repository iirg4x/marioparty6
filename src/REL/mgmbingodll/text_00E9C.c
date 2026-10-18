/* Sets the Treetop Bingo camera and interpolates its view between board paths. */
#include "REL/mgmbingodll/module_types.h"
#include "game/thpmain.h"

void fn_1_898(HU3D_MODEL *modelP, Mtx *mtx);
void fn_1_854(s16 layerNo);

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
extern void fn_1_10CC(OMOBJ *object);

#include "game/hu3d.h"
void fn_1_1214(
    HU3D_CAMERA *camera,
    HuVecF *cameraPosition,
    HuVecF *cameraTarget,
    const HuVecF *pathPosition,
    const HuVecF *pathTarget,
    f32 duration,
    f32 frame);
extern f32 InterpolateBMLine(f32 *samples, f32 *sampleTimes, f32 frame);

/* Creates the camera used to view the bingo board when gameplay begins. */
void fn_1_E9C(void) {
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(
        1, (20.0f), (10.0f), (5000.0f),
        (1.2f));
    Hu3DCameraViewportSet(
        1, (0.0f), (0.0f), (640.0f),
        (480.0f), (0.0f), (1.0f));
    Hu3DCameraPosSet(
        1, (0.0f), (150.0f), (5000.0f),
        (0.0f), (1.0f), (0.0f),
        (0.0f), (0.0f), (0.0f));
    lbl_1_bss_A8 = omAddObjEx(
        lbl_1_bss_B00, 3840, 0, 0, -1, fn_1_10CC);
    lbl_1_bss_A8->work[0] = 1;
    lbl_1_bss_A8->work[1] = 1;
    lbl_1_bss_A8->work[2] = 1;
    lbl_1_bss_AC.path =
        lbl_1_data_0[lbl_1_bss_A8->work[0]];
}

void fn_1_1088(void)
{
}

/* Selects the next camera path and its transition duration when gameplay changes the view. */
void fn_1_108C(s16 pathIndex, s16 durationFrames) {
    lbl_1_bss_A8->work[0] = (u32)pathIndex;
    lbl_1_bss_A8->work[1] = (u32)durationFrames;
    lbl_1_bss_A8->work[2] = 0;
}

/* Advances the camera path object once per frame until its selected transition ends. */
void fn_1_10CC(OMOBJ *cameraPath) {
    if (cameraPath->work[0] < 6) {
        fn_1_1214(
            Hu3DCamera,
            &lbl_1_bss_AC.path.position,
            &lbl_1_bss_AC.path.target,
            &lbl_1_data_0[cameraPath->work[0]].position,
            &lbl_1_data_0[cameraPath->work[0]].target,
            (f32)cameraPath->work[1],
            (f32)cameraPath->work[2]);
        cameraPath->work[2]++;
        if (cameraPath->work[2] > cameraPath->work[1]) {
            cameraPath->work[1] = cameraPath->work[2] = 1;
            lbl_1_bss_AC.path =
                lbl_1_data_0[lbl_1_bss_A8->work[0]];
        }
    }
}

/* Updates the camera position and target for each frame of a board-view transition. */
void fn_1_1214(
    HU3D_CAMERA *camera,
    HuVecF *cameraPosition,
    HuVecF *cameraTarget,
    const HuVecF *pathPosition,
    const HuVecF *pathTarget,
    f32 duration,
    f32 frame)
{
    f32 distance;
    f32 sampleTimes[4];
    f32 samples[4];

    sampleTimes[0] = (-1.0f) * duration;
    sampleTimes[1] = (0.0f);
    sampleTimes[2] = duration;
    sampleTimes[3] = (2.0f) * duration;
    // A quarter sine wave slows the camera as it approaches the selected view.
    frame = (f32)((f64)duration *
        sin(((3.141592653589793) *
            (f64)((90.0f) * (frame / duration))) /
            (180.0)));

    distance = pathPosition->x - cameraPosition->x;
    samples[0] = cameraPosition->x - distance;
    samples[1] = cameraPosition->x;
    samples[2] = pathPosition->x;
    samples[3] = pathPosition->x + distance;
    camera->pos.x = InterpolateBMLine(samples, sampleTimes, frame);
    distance = pathPosition->y - cameraPosition->y;
    samples[0] = cameraPosition->y - distance;
    samples[1] = cameraPosition->y;
    samples[2] = pathPosition->y;
    samples[3] = pathPosition->y + distance;
    camera->pos.y = InterpolateBMLine(samples, sampleTimes, frame);
    distance = pathPosition->z - cameraPosition->z;
    samples[0] = cameraPosition->z - distance;
    samples[1] = cameraPosition->z;
    samples[2] = pathPosition->z;
    samples[3] = pathPosition->z + distance;
    camera->pos.z = InterpolateBMLine(samples, sampleTimes, frame);

    distance = pathTarget->x - cameraTarget->x;
    samples[0] = cameraTarget->x - distance;
    samples[1] = cameraTarget->x;
    samples[2] = pathTarget->x;
    samples[3] = pathTarget->x + distance;
    camera->target.x = InterpolateBMLine(samples, sampleTimes, frame);
    distance = pathTarget->y - cameraTarget->y;
    samples[0] = cameraTarget->y - distance;
    samples[1] = cameraTarget->y;
    samples[2] = pathTarget->y;
    samples[3] = pathTarget->y + distance;
    camera->target.y = InterpolateBMLine(samples, sampleTimes, frame);
    distance = pathTarget->z - cameraTarget->z;
    samples[0] = cameraTarget->z - distance;
    samples[1] = cameraTarget->z;
    samples[2] = pathTarget->z;
    samples[3] = pathTarget->z + distance;
    camera->target.z = InterpolateBMLine(samples, sampleTimes, frame);
}

MgmbingoCameraState lbl_1_bss_AC;
OMOBJ *lbl_1_bss_A8;

MgmbingoCameraPath lbl_1_data_0[6] = {
    {{0.0f, 2800.0f, 1600.0f}, {0.0f, 2400.0f, 220.0f}},
    {{0.0f, 870.0f, 2500.0f}, {0.0f, 870.0f, 220.0f}},
    {{-200.0f, 1070.0f, 1500.0f}, {-200.0f, 1070.0f, 220.0f}},
    {{200.0f, 1070.0f, 1500.0f}, {200.0f, 1070.0f, 220.0f}},
    {{-200.0f, 770.0f, 1500.0f}, {-200.0f, 770.0f, 220.0f}},
    {{200.0f, 770.0f, 1500.0f}, {200.0f, 770.0f, 220.0f}}
};
