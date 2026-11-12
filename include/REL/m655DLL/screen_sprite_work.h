#ifndef REL_M655DLL_SCREEN_SPRITE_WORK_H
#define REL_M655DLL_SCREEN_SPRITE_WORK_H
#include "REL/framework/frame_time.h"
#include "REL/m655DLL/screen_sprite_singleton.h"
#include <vector>
#include <list>
#include "dolphin/gx.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/game_object2.h"
float RandomFloat(float minimum, float maximum);
struct ScreenParticle {
    Vec origin;
    Vec position;
    f32 extent;
    s32 remainingFrames;
    ScreenParticle(const Vec &point)
        : origin(point), extent(RandomFloat(1.0f, 5.0f)),
          remainingFrames(secondsToFrames(RandomFloat(2.0f, 4.0f))) {}
};

/* The screen object: it copies the frame into a texture and draws it back through an indirect warp
 * made of the particles. */
struct ScreenSpriteWork : MinigameSingleton<ScreenSpriteWork>, RenderObject {
    std::vector<signed char> captureBuffer;
    GXTexObj texture;
    FrameInterpolator<float> transition;
    f32 perspective;
    f32 traveledDistance;
    Vec position;
    std::list<ScreenParticle> motions;
    void *spriteTexture;

    ScreenSpriteWork();
    ~ScreenSpriteWork();
    void update();
    void draw(Mtx *matrix);
};

#endif
