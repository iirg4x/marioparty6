// Runs Coins get! and animates the coin models.

#ifndef REL_SEER_TERROR_COINS_GET_EVENT_H
#define REL_SEER_TERROR_COINS_GET_EVENT_H

#include "REL/m677DLL/event_base.h"
#include "REL/framework/model.h"

#include <memory>

// Stores the model and timing for one animated coin.
struct CoinsGetDropModel {
    CoinsGetDropModel();
    ~CoinsGetDropModel();

    std::auto_ptr<OwnedModel> model;
    Vec position;
    Vec rotation;
    float unk1C; // The game's code never reads this field.
    float unk20; // The game's code never reads this field.
    float unk24; // The game's code never reads this field.
    int unk28; // The game's code never reads this field.
    // Time advanced by the coin animation.
    float elapsedTime;
    // Time before this coin starts to fall.
    float startDelay;
    // The coin remains active until it reaches character height.
    unsigned char active;
};

// Staggers the coin animations and waits for them to finish.
class CoinsGetEvent : public EventBase {
public:
    CoinsGetEvent();
    virtual ~CoinsGetEvent();
    virtual void begin();
    virtual int step();

    typedef void (CoinsGetEvent::*State)();
    // Active handler for the event sequence.
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    CoinsGetDropModel entries[20];
    int frame;
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames) { countdown = frames; }

    void updateDropTransforms();
    void initializeDrops();
    void shiftCharacterAfterCountdown();
    void advanceActiveDrops();
    void waitForFinalCharacterMotion();
};

#endif
