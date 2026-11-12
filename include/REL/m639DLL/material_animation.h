#ifndef REL_M639_MATERIAL_ANIMATION_H
#define REL_M639_MATERIAL_ANIMATION_H
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/singleton.h"
struct MaterialAnimation : MinigameSingleton<MaterialAnimation>, GameObject {
    void *animationData; // ANIMDATA used by HuSprTexLoad.
    float textureFrame;
    MaterialAnimation(const Model *model);
    virtual ~MaterialAnimation();
    virtual void update();
};
#endif
