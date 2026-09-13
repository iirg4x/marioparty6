#include "REL/m621dll.h"
#include "game/audio.h"

s32 fn_1_420(HuVecF *pos, s16 soundId)
{
    HuVecF screenPos;
    s16 pan;
    s16 volume;

    Hu3D3Dto2D(pos, HU3D_CAM0, &screenPos);
    if (screenPos.x > 608.0f) {
        pan = 80;
    } else if (screenPos.x < 32.0f) {
        pan = 48;
    } else {
        pan = 48.0f + (32.0f * (screenPos.x - 32.0f)) / 576.0f;
    }
    if (screenPos.y > 400.0f) {
        volume = 127;
    } else if (screenPos.y < 40.0f) {
        volume = 96;
    } else {
        volume = 96.0f + (31.0f * (screenPos.y - 40.0f)) / 360.0f;
    }
    return HuAudFXPlayVolPan(soundId, volume, pan);
}

s32 fn_1_594(s16 pan, s16 soundId)
{
    return HuAudFXPlayVolPan(soundId, 127, pan);
}

void fn_1_5C8(s32 sound)
{
    HuAudFXStop(sound);
}
