// Owns the host model, its motions, and its voices.

#ifndef REL_SEER_TERROR_HOST_CHARACTER_H
#define REL_SEER_TERROR_HOST_CHARACTER_H

#include <memory>
#include "REL/framework/singleton.h"
#include "REL/framework/model_motion.h"
#include "REL/m677DLL/material_colors.h"

// Controls the host model's motions and material colors.
class HostCharacter : public MinigameSingleton<HostCharacter>, public ModelMotion {
public:
    explicit HostCharacter(Vec *position);
    ~HostCharacter();
    void fn_1_188BC(int sound);
    void fn_1_18928();
    void fn_1_18978();
    void fn_1_189CC();
    void fn_1_18A20();
    void spawnJawParticles();
    void playBeforeResultMessageMotion();
    void playBetweenMotionSound();
    void playAfterResultMessageMotion();
    void setMaterialColorBlend(f32 value);

    std::auto_ptr<MaterialColors> materialColors;
};

#endif
