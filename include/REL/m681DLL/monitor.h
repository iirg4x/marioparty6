#ifndef M681_MONITOR_H
#define M681_MONITOR_H
#include <vector>
#include "dolphin/types.h"
#include "REL/m681DLL/monitor_interface.h"
#include "REL/framework/model.h"
#include "REL/framework/model_motion.h"
#include "REL/framework/singleton.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"
extern "C" {
#include "game/sprite.h"
}
struct Monitor;

typedef void (Monitor::*CallbackData329C)();

class Monitor : public MinigameSingleton<Monitor>, public GameObject, public ModelMotion, public EventListener {
public:
    CallbackData329C callback;
    f32 max_frames;
    s32 textureScale;
    u32 delay;
    std::vector<ANIMDATA *> animations;
    s32 remaining;
    s32 duration;
    s32 firstTexture;
    s32 secondTexture;
    u32 updateCount;
    u8 active;
    bool expired() const { return remaining <= 0; }
    bool finished() const { return (firstTexture != 0 && secondTexture != 0) && expired(); }
    s32 remainingFrames() const { return remaining; }
    Monitor();
    virtual ~Monitor();
    virtual void update();
    virtual void receive(const EventPacket *);
    void fn_1_3C24();
    void fn_1_3C28();
    s32 fn_1_3BAC();
    void fn_1_3EAC();
};
#endif
