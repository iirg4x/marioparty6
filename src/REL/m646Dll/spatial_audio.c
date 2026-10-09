/* Pans Hyper Sniper actor sounds by screen position and plays centered stage cues. */
#include "dolphin/math.h"
#include "REL/m646Dll/module_types.h"

s32 fn_1_B3C0(HuVecF *worldPosition, s32 soundId);
s32 fn_1_B4BC(s32 soundId);

/* Shot and impact callbacks pan their sounds by screen X while keeping full volume. */
s32 fn_1_B3C0(HuVecF *worldPosition, s32 soundId)
{
    s16 pan;
    s16 volume;
    s32 cameraMask = HU3D_CAM0;
    f32 horizontalFraction;
    HuVecF screenPosition;

    Hu3D3Dto2D(worldPosition, cameraMask, &screenPosition);
    /* Keep pan between 48 and 80 around center, even for positions beyond the screen edges. */
    if (screenPosition.x > (608.0f)) {
        pan = 80;
    } else if (screenPosition.x < (32.0f)) {
        pan = 48;
    } else {
        horizontalFraction = (screenPosition.x - (32.0f)) / (576.0f);
        pan = (s16)((s32)((32.0f) * horizontalFraction) + 48);
    }
    volume = MSM_VOL_MAX;
    return HuAudFXPlayVolPan(soundId, volume, pan);
}

/* Plays a stage cue at full volume and center pan when it has no world position. */
s32 fn_1_B4BC(s32 soundId)
{
    return HuAudFXPlayVolPan(soundId, MSM_VOL_MAX, MSM_PAN_CENTER);
}
