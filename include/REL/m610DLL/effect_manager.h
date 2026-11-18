#ifndef REL_M610DLL_EFFECT_MANAGER_H
#define REL_M610DLL_EFFECT_MANAGER_H
#include <vector>
#include "REL/framework/event_packet.h"
#include <memory>
#include "REL/framework/hu_archive.h"
#include "REL/framework/model.h"
#include "REL/framework/octtree_man.h"
#include "REL/framework/singleton.h"
extern "C"
{
#include "game/sprite.h"
}
struct PlayerPiece;
struct CaptureEffect;
struct GameObject;
struct ParticleEmitterState;
/* Owns the scene resources and creates its models and particle effects. */
class EffectManager : public MinigameSingleton<EffectManager>, public EventListener
{
  public:
    std::auto_ptr<Octree> collisionTree;
    std::vector<ANIMDATA *> animations;
    std::vector<OwnedModel *> models;
    PlayerPiece *fn_1_E9B8(int);
    void fn_1_EBAC(int);
    void fn_1_ECE8();
    ParticleEmitterState *fn_1_EDCC() const;
    void fn_1_EE3C();
    CaptureEffect *fn_1_EFC4();
    void fn_1_F0BC();
    EffectManager();
    virtual ~EffectManager();
    virtual void receive(const EventPacket *);
};

#endif
