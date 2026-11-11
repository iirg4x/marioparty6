#ifndef REL_M648_MANAGERS_H
#define REL_M648_MANAGERS_H

#include "REL/framework/minigame_core.h"
#include "REL/framework/list_object.h"
extern "C" {
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/sprite.h"
}

#include "REL/m648DLL/camera_manager.h"
typedef CameraManager RequestOwner8C40;
struct ParticleAnimationOwner : RequestObject, MinigameSingleton<ParticleAnimationOwner> {
    ParticleAnimationOwner();
    virtual ~ParticleAnimationOwner();
    virtual void flagEnabled(int);
    virtual void update();
    ANIMDATA *animations[2];
};
typedef ParticleAnimationOwner AnimationManager8FA4;
struct TimerManager : RequestObject, MinigameSingleton<TimerManager> {
    MGTIMER *timer;
    s32 parameter;
    u8 display;
    u8 active;
    TimerManager(s32 value, u8 active);
    virtual ~TimerManager();
    virtual void flagEnabled(int mode);
    void fn_1_D268();
    void fn_1_D330();
    void fn_1_D438();
    s32 fn_1_D464();
};
typedef struct ScoreCounterSlot {
    HUSPR_GROUPID groupId;
    MGSCORE *score;
    u32 unknown08;
    s32 characterNo;
    s32 scoreValue;
    f32 posX;
    f32 posY;
} ScoreCounterSlot;
struct ScoreManager : RequestObject, MinigameSingleton<ScoreManager> {
    u8 enabled;
    ScoreCounterSlot slots[4];
    ScoreManager();
    virtual ~ScoreManager();
    virtual void flagEnabled(int request);
    virtual void update();
    void fn_1_D77C(s32 playerNo, s32 characterNo);
    void fn_1_D8D8(int index, int value);
    void fn_1_D8E8(ScoreCounterSlot *slot);
};

#endif
