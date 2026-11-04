/* Controls the tournament menu camera and its transitions. */
#include <math.h>
#include "dolphin.h"
#include "game/main.h"
#include "game/object.h"
#include "game/data.h"
#include "game/hu3d.h"
#include "game/process.h"
#include "game/memory.h"
#include "game/gamework.h"
#include "datadir_enum.h"
#include "REL/mgmtournamentdll/module_types.h"
#include "game/hsfex.h"

typedef struct TournamentCameraPair {
    Vec position; /* Camera location in world coordinates. */
    Vec target; /* Point viewed by the camera in world coordinates. */
} TournamentCameraPair;

extern OMOBJMAN *lbl_1_bss_12C;
extern s16 MgGameNo; /* Saved tournament camera preset, retained across minigame transitions. */
void fn_1_14C8(OMOBJ *object);
void fn_1_16F8(HU3D_CAMERA *camera, Vec *startPosition, Vec *startTarget, const Vec *endPosition,
               const Vec *endTarget, f32 durationFrames, f32 elapsedFrames);
void fn_1_7C78(Vec *center);

HU3D_MOTIONID lbl_1_bss_C6; /* Opening camera animation. */
HU3D_MODELID lbl_1_bss_C4; /* Model driving the opening camera animation. */
TournamentCameraPair lbl_1_bss_AC; /* View saved at the start of a preset transition. */
OMOBJ *lbl_1_bss_A8; /* work[] holds preset index, duration, and elapsed frames. */

u32 lbl_1_data_0 = DATANUM(0xa10000, 9);
TournamentCameraPair lbl_1_data_4[13] = {
    { {0.0f, -1120.0f, 3960.0f}, {0.0f, -1500.0f, 230.0f} },
    { {0.0f, 100.0f, 3960.0f}, {0.0f, -300.0f, 230.0f} },
    { {0.0f, 240.0f, 3900.0f}, {0.0f, 100.0f, 230.0f} },
    { {-400.0f, 350.0f, 2400.0f}, {-400.0f, 210.0f, 230.0f} },
    { {-400.0f, 500.0f, 3000.0f}, {-400.0f, 360.0f, 230.0f} },
    { {400.0f, 350.0f, 2400.0f}, {400.0f, 210.0f, 230.0f} },
    { {400.0f, 500.0f, 3000.0f}, {400.0f, 360.0f, 230.0f} },
    { {0.0f, 350.0f, 2400.0f}, {0.0f, 210.0f, 230.0f} },
    { {0.0f, 0.0f, 3200.0f}, {0.0f, -140.0f, 230.0f} },
    { {0.0f, 850.0f, 2000.0f}, {0.0f, 710.0f, -220.0f} },
    { {0.0f, 1000.0f, 2600.0f}, {0.0f, 860.0f, -220.0f} },
    { {0.0f, 1450.0f, 1535.0f}, {0.0f, 1030.0f, -1050.0f} },
    { {0.0f, 50.0f, 6390.0f}, {0.0f, 150.0f, 0.0f} },
};

/* Overlay startup creates the motion camera used for the tournament's opening view. */
void fn_1_E9C(void) {
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 20.0f, 20.0f, 10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    lbl_1_bss_C6 = fn_8002C4C0(fn_80008DD0(lbl_1_data_0, 0x10000000, 2));
    lbl_1_bss_C4 = fn_800283F4(lbl_1_bss_C6, 1U);
    fn_80028478(lbl_1_bss_C4, 1U);
    fn_800269E8(lbl_1_bss_C4, ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                     ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
    fn_8002D414(lbl_1_bss_C4, 0.0f);
    lbl_1_bss_A8 = 0;
}

/* Return from a minigame installs a camera object using the last saved tournament view. */
void tournamentCameraRestore(void)
{
    fn_8002858C(lbl_1_bss_C4);
    fn_800262D0(lbl_1_bss_C4);
    lbl_1_bss_C4 = 0;
    lbl_1_bss_A8 = fn_8003AB38(lbl_1_bss_12C, 3840, 0, 0, -1, fn_1_14C8);
    lbl_1_bss_A8->work[0] = MgGameNo;
    lbl_1_bss_A8->work[1] = 1;
    lbl_1_bss_A8->work[2] = 1;
    lbl_1_bss_AC = lbl_1_data_4[lbl_1_bss_A8->work[0]];
    Hu3DCamera[0].near = 20.0f;
    Hu3DCamera[0].far = 10000.0f;
}

void fn_1_1134(void)
{
}

/* State 1 plays the opening camera motion, then installs the tournament camera object. */
void tournamentCameraOpeningPlay(void)
{
    f32 motionTime;
    f32 maxTime;
    HU3D_CAMERA camera;

    fn_800284C8(lbl_1_bss_C4, 1);
    fn_80026A44(lbl_1_bss_C4, ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                       ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
    fn_8002D414(lbl_1_bss_C4, 0.0f);
    motionTime = 0.0f;
    maxTime = 1.0f;
    while (motionTime < maxTime) {
        HuPrcVSleep();
        motionTime = 1.0f + fn_8002D4E0(lbl_1_bss_C4);
        maxTime = fn_8002D530(lbl_1_bss_C4);
    }
    fn_800269E8(lbl_1_bss_C4, ((1 << 30) | (1 << 1)) | ((1 << 30) | (1 << 4)) |
                     ((1 << 30) | (1 << 7)) | ((1 << 30) | (1 << 10)));
    fn_8002858C(lbl_1_bss_C4);
    fn_800262D0(lbl_1_bss_C4);
    lbl_1_bss_C4 = 0;

    camera = Hu3DCamera[0];
    lbl_1_bss_A8 = fn_8003AB38(lbl_1_bss_12C, 3840, 0, 0, -1,
                              fn_1_14C8);
    lbl_1_bss_A8->work[0] = 0;
    lbl_1_bss_A8->work[1] = 1;
    lbl_1_bss_A8->work[2] = 1;
    lbl_1_bss_AC =
        lbl_1_data_4[lbl_1_bss_A8->work[0]];
    camera.near = 20.0f;
    camera.far = 10000.0f;
    Hu3DCamera[0] = camera;
    HuPrcVSleep();
}

/* The pre-round scene requests a timed camera move to the current pairing on the bracket. */
void tournamentCameraRoundViewSet(u32 durationFrames) {
    s16 roundLayout;

    roundLayout = tournamentLayoutGet();
    if (roundLayout == 0) {
        tournamentCameraViewSet(3, durationFrames);
        return;
    }
    if (roundLayout == 1) {
        tournamentCameraViewSet(5, durationFrames);
        return;
    }
    if (roundLayout == 2) {
        tournamentCameraViewSet(7, durationFrames);
        return;
    }
    tournamentCameraViewSet(9, durationFrames);
}

/* Scene transitions select a preset and save the current view for the timed camera move. */
void tournamentCameraViewSet(s16 viewIndex, u32 durationFrames) {
    lbl_1_bss_A8->work[0] = viewIndex;
    lbl_1_bss_A8->work[1] = durationFrames;
    lbl_1_bss_A8->work[2] = 0;
    lbl_1_bss_AC.position = Hu3DCamera[0].pos;
    lbl_1_bss_AC.target = Hu3DCamera[0].target;
}

/* The camera object advances its preset move and updates the scene shadow each frame. */
void fn_1_14C8(OMOBJ *object)
{
    Vec shadowPosition;
    Vec shadowUp;
    Vec shadowTarget;
    Vec center;

    if (object->work[0] < 13) {
        fn_1_16F8(&Hu3DCamera[0],
                  &lbl_1_bss_AC.position, &lbl_1_bss_AC.target,
                  &lbl_1_data_4[object->work[0]].position,
                  &lbl_1_data_4[object->work[0]].target,
                  (f32)object->work[1], (f32)object->work[2]);
        object->work[2]++;
        /* Completed moves use the destination as both endpoints on later updates. */
        if (object->work[2] > object->work[1]) {
            object->work[1] = object->work[2] = 1;
            lbl_1_bss_AC =
                lbl_1_data_4[lbl_1_bss_A8->work[0]];
        }
        MgGameNo = object->work[0];
    }

    /* Distant views extend the far plane; closer views do not reduce it again here. */
    if (Hu3DCamera[0].pos.z >= 5000.0f) {
        Hu3DCamera[0].far = 20000.0f;
    }
    fn_1_7C78(&center);
    shadowPosition.x = center.x;
    shadowPosition.y = 5000.0f + center.y;
    shadowPosition.z = 1000.0f + center.z;
    shadowUp.x = 0.0f;
    shadowUp.y = 0.2f;
    shadowUp.z = -1.0f;
    shadowTarget = center;
    Hu3DShadowPosSet(&shadowPosition,
                     &shadowUp, &shadowTarget);
}

/* The camera object eases its position and target from the saved view to the preset each frame. */
void fn_1_16F8(
    HU3D_CAMERA *camera,
    Vec *startPosition,
    Vec *startTarget,
    const Vec *endPosition,
    const Vec *endTarget,
    f32 durationFrames,
    f32 elapsedFrames)
{
    f32 axisDelta;
    f32 sampleTimes[4];
    f32 samples[4];

    sampleTimes[0] = -1.0f * durationFrames;
    sampleTimes[1] = 0.0f;
    sampleTimes[2] = durationFrames;
    sampleTimes[3] = 2.0f * durationFrames;
    /* A quarter sine wave slows the move as it reaches its destination. */
    elapsedFrames = (f32)((f64)durationFrames *
        sin((3.141592653589793 *
            (f64)(90.0f * (elapsedFrames / durationFrames))) /
            180.0));

    axisDelta = endPosition->x - startPosition->x;
    samples[0] = startPosition->x - axisDelta;
    samples[1] = startPosition->x;
    samples[2] = endPosition->x;
    samples[3] = endPosition->x + axisDelta;
    camera->pos.x = InterpolateBMLine(samples, sampleTimes, elapsedFrames);
    axisDelta = endPosition->y - startPosition->y;
    samples[0] = startPosition->y - axisDelta;
    samples[1] = startPosition->y;
    samples[2] = endPosition->y;
    samples[3] = endPosition->y + axisDelta;
    camera->pos.y = InterpolateBMLine(samples, sampleTimes, elapsedFrames);
    axisDelta = endPosition->z - startPosition->z;
    samples[0] = startPosition->z - axisDelta;
    samples[1] = startPosition->z;
    samples[2] = endPosition->z;
    samples[3] = endPosition->z + axisDelta;
    camera->pos.z = InterpolateBMLine(samples, sampleTimes, elapsedFrames);

    axisDelta = endTarget->x - startTarget->x;
    samples[0] = startTarget->x - axisDelta;
    samples[1] = startTarget->x;
    samples[2] = endTarget->x;
    samples[3] = endTarget->x + axisDelta;
    camera->target.x = InterpolateBMLine(samples, sampleTimes, elapsedFrames);
    axisDelta = endTarget->y - startTarget->y;
    samples[0] = startTarget->y - axisDelta;
    samples[1] = startTarget->y;
    samples[2] = endTarget->y;
    samples[3] = endTarget->y + axisDelta;
    camera->target.y = InterpolateBMLine(samples, sampleTimes, elapsedFrames);
    axisDelta = endTarget->z - startTarget->z;
    samples[0] = startTarget->z - axisDelta;
    samples[1] = startTarget->z;
    samples[2] = endTarget->z;
    samples[3] = endTarget->z + axisDelta;
    camera->target.z = InterpolateBMLine(samples, sampleTimes, elapsedFrames);
}
