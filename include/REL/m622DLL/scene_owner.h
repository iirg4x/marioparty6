// The moving stage: twelve models, the scene groups it spawns and the shuffled order they come in.
#ifndef REL_M622DLL_SCENE_OWNER_H
#define REL_M622DLL_SCENE_OWNER_H

#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
#include "REL/m622DLL/scene_models.h"

class SceneOwner : public RequestObject, public MinigameSingleton<SceneOwner> {
public:
    OwnedModel model0, model1, model2, model3, model4, model5;
    OwnedModel model6, model7, model8, model9, model10, model11;
    s32 updateCount;
    u8 phaseStarted;
    s32 sceneState;
    s32 delayFrames;
    s32 completedSegments;
    f32 segmentPosition;
    s32 sceneIndex;
    struct ScenePhase {
        s32 selectedAction[50];
    } phases[4];
    f32 pathPosition;
    s32 unusedValue;
    u8 soundActive;

    SceneOwner();
    virtual ~SceneOwner();
    virtual void flagEnabled(int bit);
    virtual void update();
    void fn_1_E00();
    void fn_1_1058();
};

#endif
