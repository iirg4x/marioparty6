#ifndef REL_M639DLL_SCENE_OWNER_H
#define REL_M639DLL_SCENE_OWNER_H
// The map scene.
#include "REL/framework/singleton.h"
#include "REL/framework/game_object.h"
#include "REL/framework/list_object.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"

class SceneOwner : public RequestObject, public MinigameSingleton<SceneOwner> {
public:
    OwnedModel sceneModel0;
    OwnedModel sceneModel1;
    OwnedModel sceneModel2;
    OwnedModel animationModel;
    OwnedModel lightModel;
    SceneOwner();
    virtual ~SceneOwner();
    virtual void flagEnabled(int event);
    virtual void update();
    Vec *fn_1_5AAC(Vec *direction);
};

#endif
