#ifndef REL_M605DLL_SCENE_STAGE_H
#define REL_M605DLL_SCENE_STAGE_H

#include <memory>
#include "REL/framework/list_object.h"
#include "REL/framework/model.h"
#include "REL/framework/sphere.h"
struct SceneShadow;

// Owns the stage models and their attachments.
struct SceneStage : RequestObject {
    // Model names come from the stage assets.
    OwnedModel iseki;
    OwnedModel taimatu;
    OwnedModel i8akari;
    OwnedModel hasi;
    OwnedModel prapara;
    OwnedModel ita;
    Sphere hooks[4];
    OwnedModel *attachments[9];
    OwnedModel attachmentModel;
    std::auto_ptr<SceneShadow> shadow;

    SceneStage();
    virtual ~SceneStage();
    virtual void flagEnabled(int step);
    virtual void update();
    void fn_1_4AE8();
};

bool fn_1_49D0(f32 height, f32 depth);
f32 fn_1_4A0C(f32 value);
s32 fn_1_4AA4(f32 depth);

#endif
