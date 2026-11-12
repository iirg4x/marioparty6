#ifndef REL_M604DLL_STAGE_EFFECTS_H
#define REL_M604DLL_STAGE_EFFECTS_H

#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/sprite.h"
}

struct LensFlare : RequestObject {
    LensFlare();
    virtual ~LensFlare();
    virtual void flagEnabled(int);
    virtual void update();
    s16 modelId;
    ANIMDATA *animations[4];
};

struct WeatherParticles : RequestObject, MinigameSingleton<WeatherParticles> {
    WeatherParticles();
    virtual ~WeatherParticles();
    virtual void flagEnabled(int);
    virtual void update();
    ANIMDATA *getAnimation() { return animation; }
    ANIMDATA *animation;
    s32 weatherState;
    s32 interval;
    s32 frame;
    f32 elapsed;
};

#endif
