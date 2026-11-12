#include "REL/m604DLL/camera_masks.h"
#include "REL/m604DLL/particle_owner.h"
#include "types.h"
#include "math.h"
#include "REL/m604DLL/animation_menu.h"

extern "C" {
#include "game/mg/score.h"
#include "game/gamework.h"
#include "game/gamemes.h"
}

#include "REL/m604DLL/score_owner.h"

void M604ScoreOwner::fn_1_14FF8()
{
    s32 index;

    for (index = 0; index < 4; index++) {
        MgScoreBoxDispSet(scoreBoxIds[index], 1);
        MgScoreDispOn(scoreDisplays[index]);
    }
}

void M604ScoreOwner::fn_1_15060()
{
    s32 index;

    for (index = 0; index < 4; index++) {
        MgScoreBoxDispSet(scoreBoxIds[index], 0);
        MgScoreDispOff(scoreDisplays[index]);
    }
}

void M604ScoreOwner::fn_1_150C8(s32 index, s32 value)
{
    MgScoreValueSet(scoreDisplays[index], value);
}

void M604ScoreOwner::flagEnabled(int bitIndex) {
    flags &= ~(1 << bitIndex);
}

extern "C" {
#include "game/hu3d.h"
}

void fn_1_15128(HU3D_MODEL *, HU3D_PARTICLE *particle, Mtx matrix)
{
    ModelParticles *timingObject = static_cast<ModelParticles *>(particle->work);
    fn_1_1578C(timingObject, matrix);
}

extern "C" void fn_800262D0(short modelId);

struct ParticleColor {
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
};

const ParticleColor lbl_1_rodata_920[2] = {
    { 248, 250, 175, 255 },
    { 209, 209, 151, 255 }
};

ModelParticles::ModelParticles(int mode)
{
    ParticleAnimations *menu = &MinigameSingleton<ParticleAnimations>::getChecked();
    s16 id;
    s32 nightModeIndex;
    id = fn_8003216C(menu->getAnimation(), 0xC);
    particle = (HU3D_PARTICLE *) lbl_802C01C8[(s16)id].hookData;
    particle->work = this;
    nightModeIndex = GWMgNightFGet();
    fn_800326D0((s16)id, fn_1_15128);
    fn_80026B64((s16)id, cameraMasks[mode]);
    fn_800325B0((s16)id, lbl_1_rodata_920[nightModeIndex].red, lbl_1_rodata_920[nightModeIndex].green, lbl_1_rodata_920[nightModeIndex].blue);
    fn_80032620((s16)id, 0.0f);
    fn_8003269C((s16)id, 0);
    fn_80026B8C((s16)id, 5);
    fn_80026760((s16)id, 0.0f, 0.0f, 0.0f);
    fn_80026808((s16)id, 0.0f, 0.0f, 0.0f);
    modelId = (s16)id;
    pending = 1;
    fn_1_15440(this, particle);
    fn_800269E8((s16)id, (1 << 0));
}

ModelParticles::~ModelParticles()
{
    particle->work = 0;
    fn_800262D0(modelId);
}

void fn_1_153C4(ModelParticles *record, const Vec *position) {
    if (record->pending != 0) {
        fn_80026760(record->modelId, position->x, position->y, position->z);
        fn_80032620(record->modelId, 0.0f);
        record->pending = 0;
        fn_80026A44(record->modelId, (1 << 0));
    }
}
