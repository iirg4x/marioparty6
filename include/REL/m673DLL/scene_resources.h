// Owns the models and animations used by scene objects.

#ifndef REL_M673DLL_SCENE_RESOURCES_H
#define REL_M673DLL_SCENE_RESOURCES_H
#include "REL/framework/singleton.h"
#include "REL/framework/event_hm.h"
#include <memory>
#include <vector>
#include "game/animdata.h"

struct OwnedModel;

// Owns the models and animations loaded for the scene.
struct SceneResources : MinigameSingleton<SceneResources>, EventListener {
    std::vector<ANIMDATA *> animations; // Loaded animations owned by the scene.
    std::vector<OwnedModel *> models; // Loaded models owned by the scene.
    // Returns the indexed scene animation.
    ANIMDATA *animationAt(unsigned long index)
    {
        return animations.at(index);
    }
    // Loads the scene models and animations and registers their event receivers.
    SceneResources();
    // Kills the scene animations and deletes the owned models.
    virtual ~SceneResources();
    // Dispatches events to the scene's registered channels.
    virtual void receive(const EventPacket *event);
};

// Creates the scene models, lighting, and background objects.
void fn_1_6104(SceneResources *resources);

// Creates a particle effect for a position event.
void fn_1_6800(SceneResources *resources, const EventPacket *event);
// Deletes the owned models and clears their list.
void fn_1_6898(std::vector<OwnedModel *> *models);

// Creates the four player actors at their starting positions.
void fn_1_6638(SceneResources *);
// Creates the moving actor at its starting position.
void fn_1_66AC(SceneResources *);
// Leaves the scene resources unchanged in this setup callback.
void fn_1_677C(SceneResources *);
// Leaves the scene resources unchanged in the final setup callback.
void fn_1_6780(SceneResources *);

// Ignores the event for this resource callback.
void fn_1_67FC(SceneResources *resources, const EventPacket *event);

// Reads and locks an animation resource.
ANIMDATA *fn_1_5894(unsigned long dataNumber);

#endif
