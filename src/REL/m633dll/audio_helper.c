#include "REL/m633dll.h"

void fn_1_24EC(s32 soundId, Point3d *pos)
{
    Point3d projected;
    s32 pan;
    s32 sound;

    Hu3D3Dto2D(pos, 1, &projected);
    pan = (s32)projected.x;
    pan /= 5;
    if (pan < 48) {
        pan = 48;
    } else if (pan > 127) {
        pan = 127;
    }
    sound = HuAudFXPlay(soundId);
    HuAudFXPanning(sound, (s16)pan);
}
