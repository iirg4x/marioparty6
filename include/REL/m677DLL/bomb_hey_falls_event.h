// Runs the Bomb-hey falls event and its result.

#ifndef REL_SEER_TERROR_BOMB_HEY_FALLS_EVENT_H
#define REL_SEER_TERROR_BOMB_HEY_FALLS_EVENT_H

#include <memory>
#include "REL/m677DLL/event_base.h"
#include "REL/framework/model_motion.h"

// Drops a bomb, then launches the player after it lands.
class BombHeyFallsEvent : public EventBase {
public:
    BombHeyFallsEvent();
    virtual ~BombHeyFallsEvent();
    virtual void begin();
    virtual int step();

private:
    typedef void (BombHeyFallsEvent::*State)();
    // Tracks one falling model and the overlay at the same position.
    struct Entry {
        Entry();
        ~Entry();
        std::auto_ptr<ModelMotion> model;
        std::auto_ptr<OwnedModel> overlay;
        Vec position;
        float unk14; // The game's code never reads this field.
        float angle;
        float unk1C; // The game's code never reads this field.
        // Scales the descent applied during the fall.
        float fallScale;
        float unk24; // The game's code never reads this field.
        float elapsed;
        // Phase of the fall and the reaction that follows it.
        int state;
    };
    // Active handler for the event sequence.
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    Entry entries[1];
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames) { countdown = frames; }
    // Frames before the next reaction after impact.
    int transitionCountdown;

    // Sets the frames before the next reaction after impact.
    void setTransitionCountdown(int frames) { transitionCountdown = frames; }
    void initializeImpactEntry();
    void startFollowupCountdown();
    void startFollowupEffect();
    void disableEffectAfterCountdown();
    void updateImpactEntry();
};

#endif
