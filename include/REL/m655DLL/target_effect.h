/* TargetEffect is constructed by new-expressions in this module. */
#ifndef INCLUDE_REL_M655DLL_TARGET_EFFECT_H
#define INCLUDE_REL_M655DLL_TARGET_EFFECT_H

#include "REL/framework/model_motion.h"
#include "REL/framework/game_object.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/m655DLL/sequence_callback.h"
#include "REL/framework/category_sphere.h"

/* Vector operations on the 12-byte Vec base. */
struct EffectVector : Vec {
    using Vec::operator=;
    EffectVector &operator*=(f32 scale);
    EffectVector &normalize();
};

struct TargetEffect : GameObject, ModelMotion {
    virtual ~TargetEffect();
    virtual void update();
    TargetEffect(void);
    typedef void (TargetEffect::*State)(void);
    State targetState;
    s32 m_target_id;
    EffectVector startPosition;
    EffectVector currentPosition;
    EffectVector effectPosition;
    s32 timer;
    RegisteredValue firstRegisteredValue;
    RegisteredValue secondRegisteredValue;
    void fn_1_4A28(void);
    f32 lowerHeight() const { return firstRegisteredValue.radius; }
    void fn_1_4DA8();
    void fn_1_4E8C();
    void fn_1_5044();
    s32 soundId;
};

#endif
