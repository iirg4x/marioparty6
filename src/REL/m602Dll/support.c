/* Moves the Odd Card Out intro camera and shakes it during the impact reaction. */
#define _MATH_H
#include "REL/m602Dll.h"

/* Remaining camera-shake frames; zero restores the centered camera pose. */
s16 lbl_1_bss_18;

/* The fade-in callback pans between the side views, then returns to the centered play view;
 * a nonzero return lets the sequence show the start announcement. */
u8 fn_1_D54(s16 frameNo)
{
    Point3d cameraPos;
    Point3d cameraTarget;
    f32 segmentFrame;

    if (frameNo < 60) {
        segmentFrame = frameNo;
        if (frameNo == 1) {
            fn_1_48B4();
        }
        fn_1_4E48();
    } else if (frameNo < 240) {
        segmentFrame = (f32) (frameNo - 60);
        cameraPos.x = lbl_1_data_9C[1].pos.x - lbl_1_data_9C->pos.x;
        cameraPos.y = lbl_1_data_9C[1].pos.y - lbl_1_data_9C->pos.y;
        cameraPos.z = lbl_1_data_9C[1].pos.z - lbl_1_data_9C->pos.z;
        cameraPos.x = (cameraPos.x / 180.0f) * segmentFrame;
        cameraPos.y = (cameraPos.y / 180.0f) * segmentFrame;
        cameraPos.z = (cameraPos.z / 180.0f) * segmentFrame;
        cameraPos.x += lbl_1_data_9C->pos.x;
        cameraPos.y += lbl_1_data_9C->pos.y;
        cameraPos.z += lbl_1_data_9C->pos.z;
        cameraTarget.x = lbl_1_data_9C[1].target.x - lbl_1_data_9C->target.x;
        cameraTarget.y = lbl_1_data_9C[1].target.y - lbl_1_data_9C->target.y;
        cameraTarget.z = lbl_1_data_9C[1].target.z - lbl_1_data_9C->target.z;
        cameraTarget.x = (cameraTarget.x / 180.0f) * segmentFrame;
        cameraTarget.y = (cameraTarget.y / 180.0f) * segmentFrame;
        cameraTarget.z = (cameraTarget.z / 180.0f) * segmentFrame;
        cameraTarget.x += lbl_1_data_9C->target.x;
        cameraTarget.y += lbl_1_data_9C->target.y;
        cameraTarget.z += lbl_1_data_9C->target.z;
        lbl_1_data_28.pos = cameraPos;
        lbl_1_data_28.target = cameraTarget;
        Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &lbl_1_data_28.pos, &lbl_1_data_28.up,
                          &lbl_1_data_28.target);
        if (segmentFrame > 90.0f) {
            fn_1_4EEC();
        }
    } else if (frameNo < 300) {
        segmentFrame = (f32) (frameNo - 240);
        cameraPos.x = lbl_1_data_78.pos.x - lbl_1_data_9C[1].pos.x;
        cameraPos.y = lbl_1_data_78.pos.y - lbl_1_data_9C[1].pos.y;
        cameraPos.z = lbl_1_data_78.pos.z - lbl_1_data_9C[1].pos.z;
        cameraPos.x = (cameraPos.x / 60.0f) * segmentFrame;
        cameraPos.y = (cameraPos.y / 60.0f) * segmentFrame;
        cameraPos.z = (cameraPos.z / 60.0f) * segmentFrame;
        cameraPos.x += lbl_1_data_9C[1].pos.x;
        cameraPos.y += lbl_1_data_9C[1].pos.y;
        cameraPos.z += lbl_1_data_9C[1].pos.z;
        cameraTarget.x = lbl_1_data_78.target.x - lbl_1_data_9C[1].target.x;
        cameraTarget.y = lbl_1_data_78.target.y - lbl_1_data_9C[1].target.y;
        cameraTarget.z = lbl_1_data_78.target.z - lbl_1_data_9C[1].target.z;
        cameraTarget.x = (cameraTarget.x / 60.0f) * segmentFrame;
        cameraTarget.y = (cameraTarget.y / 60.0f) * segmentFrame;
        cameraTarget.z = (cameraTarget.z / 60.0f) * segmentFrame;
        cameraTarget.x += lbl_1_data_9C[1].target.x;
        cameraTarget.y += lbl_1_data_9C[1].target.y;
        cameraTarget.z += lbl_1_data_9C[1].target.z;
        lbl_1_data_28.pos = cameraPos;
        lbl_1_data_28.target = cameraTarget;
        Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &lbl_1_data_28.pos, &lbl_1_data_28.up,
                          &lbl_1_data_28.target);
        if ((s32) segmentFrame == 53) {
            fn_1_4D68();
            fn_1_4998();
        }
    } else {
    lbl_1_data_28.pos = lbl_1_data_78.pos;
    lbl_1_data_28.up = lbl_1_data_78.up;
    lbl_1_data_28.target = lbl_1_data_78.target;
    Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &lbl_1_data_28.pos, &lbl_1_data_28.up,
                      &lbl_1_data_28.target);
    fn_1_4D68();
    return 1U;
    }
    return 0U;
}

/* The player-reaction update sets this frame countdown when the descending model finishes
 * compressing the player's displayed height. */
void fn_1_1418(s16 shakeFrames)
{
    lbl_1_bss_18 = shakeFrames;
}

/* Called by the gameplay sequence callback each frame to apply camera shake. */
void fn_1_1428(void)
{
    Point3d cameraPos;
    Point3d cameraTarget;
    Point3d jitter;
    s16 shakeParity;

    if (lbl_1_bss_18 > 0) {
        /* Odd countdown frames retain the previous camera pose instead of choosing new jitter. */
        shakeParity = lbl_1_bss_18 % 2;
        if (shakeParity == 0) {
            cameraPos = lbl_1_data_78.pos;
            cameraTarget = lbl_1_data_78.target;
            jitter.x = ((u8)frand() % 20) - 10;
            jitter.y = ((u8)frand() % 40) - 20;
            cameraPos.x += jitter.x;
            cameraPos.y += jitter.y;
            cameraTarget.x += jitter.x;
            cameraTarget.y += jitter.y;
            Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &cameraPos, &lbl_1_data_28.up,
                              &cameraTarget);
        }
    } else {
        lbl_1_data_28.pos = lbl_1_data_78.pos;
        lbl_1_data_28.up = lbl_1_data_78.up;
        lbl_1_data_28.target = lbl_1_data_78.target;
        Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &lbl_1_data_28.pos, &lbl_1_data_28.up,
                          &lbl_1_data_28.target);
    }
    lbl_1_bss_18 -= 1;
    if (lbl_1_bss_18 < 0) {
        lbl_1_bss_18 = 0;
        lbl_1_data_28.pos = lbl_1_data_78.pos;
        lbl_1_data_28.up = lbl_1_data_78.up;
        lbl_1_data_28.target = lbl_1_data_78.target;
        Hu3DCameraPosSetV(lbl_1_data_28.cameraBit, &lbl_1_data_28.pos, &lbl_1_data_28.up,
                          &lbl_1_data_28.target);
    }
}
