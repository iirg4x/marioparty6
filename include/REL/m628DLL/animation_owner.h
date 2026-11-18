#ifndef REL_M628DLL_ANIMATION_OWNER_H
#define REL_M628DLL_ANIMATION_OWNER_H
#include "REL/framework/event_packet.h"
#include "game/sprite.h"
#include <vector>
class OwnedModel;

#include "REL/framework/singleton.h"
// Loads and owns the scene models, animations and textures.
class SceneAnimations : public MinigameSingleton<SceneAnimations>, public EventListener
{
  public:
    std::vector<ANIMDATA *> animations;
    std::vector<OwnedModel *> models;

    SceneAnimations();
    virtual ~SceneAnimations();
    virtual void receive(const EventPacket *event);
};

void fn_1_10760(SceneAnimations *owner);

void fn_1_10BD8(SceneAnimations *);
void fn_1_10D34(SceneAnimations *);

#endif
