/* Initializes and updates the particle motions used by score effects. */
#include "REL/m604DLL/particle_owner.h"
#include "dolphin/mtx.h"
extern "C" {
#include "math.h"
#include "game/frand.h"
}

/* Seeds each motion with a ring direction, rise speed, starting position and spin. */
void fn_1_15440(ModelParticles *unused, HU3D_PARTICLE *motionList)
{
    s32 index;
    HU3D_PARTICLE_DATA *motion;

    motion = motionList->data;
    for (index = 0; index < motionList->maxCnt; index++, motion++) {
        motion->vel.x = (f32)(0.6f * sin(
            MTXDegToRad(
            (360.0f * (f32)index /
             (f32)motionList->maxCnt))));
        motion->vel.y = motion->accel.y =
            (f32)(1.5 *
                  (0.27240694444444447 *
                   (30.0f +
                    10.0f *
                    (1.5258789e-05f *
                     (f32)(u16)frand()))));
        motion->vel.z = (f32)(0.6f * cos(
            MTXDegToRad(
            (360.0f * (f32)index /
             (f32)motionList->maxCnt))));
        motion->scale = 80.0f;
        motion->pos.x = 100.0f *
            (0.02f *
            ((f32)(frand() & 0x3F) - 31.0f));
        motion->pos.y = -100.0f;
        motion->pos.z = 100.0f *
            (0.02f *
            ((f32)(frand() & 0x3F) - 31.0f));
        motion->color.a = 0;
        motion->time = 0;
        motion->parManId = (frand() & 0xF) + 0xF;
        motion->zRot = MTXDegToRad((f32)(u16)frand() *
                             1.5258789e-05f * 360.0f
                             );
    }
}

/* Updates each active motion and initializes the effect after all motions
 * have finished their first pass. */
void fn_1_1578C(ModelParticles *effect, Mtx matrix)
{
    HU3D_MODEL *model;
    f32 previousY;
    f32 midpoint;
    s32 motionIndex;
    u8 allComplete;
    HU3D_PARTICLE_DATA *motion;
    HU3D_PARTICLE *motionList;

    if (effect->pending == 0) {
        model = &lbl_802C01C8[effect->modelId];
        motionList = effect->particle;
        allComplete = 1;
        motion = motionList->data;
        for (motionIndex = 0;
             motionIndex < motionList->maxCnt;
             motionIndex++, motion++) {
            if (motion->time != 2) {
                if (motion->time == 0) {
                    motion->color.a += motion->parManId;
                    if (motion->color.a >= 0xC8) {
                        motion->color.a = 0xC8;
                        motion->time++;
                    }
                } else if (motion->color.a > 6) {
                    motion->color.a -= 4;
                } else {
                    motion->color.a = 0;
                    motion->time++;
                }
                motion->scale += 1.6666666666666667;
                previousY = motion->vel.y;
                motion->vel.y += -0.4086104166666667;
                midpoint = 0.5f * (previousY + motion->vel.y);
                motion->pos.x = (f32)((f64)motion->pos.x +
                    (0.016666666666666666 *
                     (f64)(100.0f *
                           (2.0f * motion->vel.x))));
                motion->pos.y += midpoint;
                motion->pos.z = (f32)((f64)motion->pos.z +
                    (0.016666666666666666 *
                     (f64)(100.0f *
                           (2.0f * motion->vel.z))));
                allComplete = 0;
            }
        }
        effect->pending = allComplete;
        if (effect->pending != 0) {
            fn_1_15440(0, motionList);
            fn_800269E8(effect->modelId, 1);
        }
    }
}
