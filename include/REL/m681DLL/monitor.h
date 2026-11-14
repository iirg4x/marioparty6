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

class Monitor : public MinigameSingleton<Monitor>, public GameObject, public ModelMotion, public EventListener {
public:
    typedef void (Monitor::*Callback)();
    Callback callback;
    f32 max_frames;
    s32 textureScale;
    u32 delay;
    std::vector<ANIMDATA *> animations;
    s32 remaining;
    s32 duration;
    s32 firstChoice;
    s32 secondChoice;
    u32 updateCount;
    u8 active;
    // Reports whether the countdown has reached zero.
    bool expired() const { return remaining <= 0; }
    // Reports whether both choices are set and the countdown has expired.
    bool finished() const { return (firstChoice != 0 && secondChoice != 0) && expired(); }
    // Returns the remaining countdown frames.
    s32 remainingFrames() const { return remaining; }
    Monitor();
    virtual ~Monitor();
    virtual void update();
    virtual void receive(const EventPacket *);
    void fn_1_3C24();
    void fn_1_3C28();
    s32 choicesExpired();
    void fn_1_3EAC();
};
#endif
