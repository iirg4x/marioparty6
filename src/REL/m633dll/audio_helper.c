/* Positions minigame sound effects in stereo from their world-space source. */
#include "REL/m633dll.h"

/* Plays a sound at a world position and clamps its screen-derived pan to the effect range 48–127. */
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
