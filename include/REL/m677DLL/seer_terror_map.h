// Owns the board models and updates the map lighting.

#ifndef REL_SEER_TERROR_MAP_H
#define REL_SEER_TERROR_MAP_H

#include <memory>

#include "REL/framework/game_object.h"
#include "REL/framework/model_motion.h"
#include "REL/framework/singleton.h"
#include "REL/m677DLL/camera_facing_scene_models.h"

// Loads the jointed board model and its map objects.
class SeerTerrorMap : public MinigameSingleton<SeerTerrorMap>, public GameObject {
public:
    SeerTerrorMap();
    virtual ~SeerTerrorMap();
    virtual void update();

    OwnedModel firstModel;
    OwnedModel secondModel;
    ModelMotion joint;
    // Returns the joint model of the active board map.
    static ModelMotion &getJoint()
    {
        SeerTerrorMap &view = getChecked();
        return view.joint;
    }

private:
    void fn_1_191FC();

    std::auto_ptr<CameraFacingSceneModels> scene;
};

#endif
