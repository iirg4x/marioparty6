// Runs the Tarai falls event and animates the player's reaction.

#ifndef REL_SEER_TERROR_TARAI_FALLS_EVENT_H
#define REL_SEER_TERROR_TARAI_FALLS_EVENT_H

#include "REL/m677DLL/event_base.h"
#include <memory>

class TaraiFallsModel;

// Runs the callback that lowers the model onto the character.
class TaraiFallsEvent : public EventBase {
public:
    TaraiFallsEvent();
    virtual ~TaraiFallsEvent();
    virtual void begin();
    virtual int step();

private:
    typedef void (TaraiFallsEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    std::auto_ptr<TaraiFallsModel> model;
    int countdown;

    void startCharacterMotion();
    void revealModelAfterMotion();
    void playImpactEffects();
    void scaleCharacterUnderModel();
    void startHeightInterpolation();
    void playCueAndWaitForMotion();
};

#endif
