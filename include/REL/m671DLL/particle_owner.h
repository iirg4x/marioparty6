#ifndef REL_M671_PARTICLE_OWNER_H
#define REL_M671_PARTICLE_OWNER_H
#include "dolphin/types.h"
#include "game/hu3d.h"

class ParticleOwner {
public:
    s16 modelId;
    ANIMDATA *animation;
    HU3D_PARTICLE *particle;
    f32 positionX, positionY, positionZ;
    Vec vector;
    u8 completed, callbackReady;
    ParticleOwner();
    ~ParticleOwner();
    void setVector(const Vec *source);
};
#include "REL/framework/game_object2.h"
#include "REL/m671DLL/motion_position.h"

struct AttributeOwner : RenderObject {
    ANIMDATA *animation;
    f32 alphaScale;
    MotionPosition position;
    f32 scaleXZ;
    AttributeOwner();
    virtual ~AttributeOwner() {}
    virtual void update();
    virtual void draw(Mtx *);
};
void fn_1_41B4(AttributeOwner *owner, u8 reset);

#endif
