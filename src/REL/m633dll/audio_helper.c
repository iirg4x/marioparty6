/* Positions minigame sound effects in stereo from their world-space source. */
#include "REL/m633dll.h"

/* Called by the segment update after a player is hit; plays the effect and pans it from projected
 * screen x, clamped to 48-127. */
void fn_1_24EC(s32 soundId, Point3d *worldPosition)
{
    Point3d screenPosition;
    s32 screenPan;
    s32 soundHandle;

    Hu3D3Dto2D(worldPosition, 1, &screenPosition);
    screenPan = (s32)screenPosition.x;
    screenPan /= 5;
    if (screenPan < 48) {
        screenPan = 48;
    } else if (screenPan > 127) {
        screenPan = 127;
    }
    soundHandle = HuAudFXPlay(soundId);
    HuAudFXPanning(soundHandle, (s16)screenPan);
}
