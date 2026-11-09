#ifndef REL_M681DLL_CHARACTER_MODEL_H
#define REL_M681DLL_CHARACTER_MODEL_H

#include <memory>
#include "REL/m681DLL/owned_model.h"
#include "REL/m681DLL/model_motion.h"
#include "REL/m681DLL/transition.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"

void operator delete(void *);
struct VirtualPrefix { unsigned long unknown00, unknown04; };
struct VirtualPointee : VirtualPrefix { virtual ~VirtualPointee(); };

class ModelOwnerComposite84 : public GameObject, public ModelMotion, public EventListener {
public:
    s32 character;
    char *motionData;
    s32 firstTimer;
    s32 secondTimer;
    std::auto_ptr<VirtualPointee> nestedResource;
    Transition transition;
    f32 *modelValue;
    ModelOwnerComposite84(int character);
    virtual ~ModelOwnerComposite84();
    virtual void update();
    virtual void receive(const EventPacket *request);
    void fn_1_83F8();
    void fn_1_8450();
};

Vec *fn_1_8184(ModelOwnerComposite84 *owner, Vec *value);

#endif
