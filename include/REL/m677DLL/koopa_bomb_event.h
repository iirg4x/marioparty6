// Runs the Koopa-bomb event and animates its models.

#ifndef REL_SEER_TERROR_KOOPA_BOMB_EVENT_H
#define REL_SEER_TERROR_KOOPA_BOMB_EVENT_H

#include "REL/framework/model.h"
#include "REL/m677DLL/event_base.h"

#include <memory>
#include <vector>

// Coordinates the model motions and the character's response.
class KoopaBombEvent : public EventBase {
  public:
    KoopaBombEvent();
    virtual ~KoopaBombEvent();
    virtual void begin();
    virtual int step();
    virtual void fn_1_17104();

  private:
    typedef void (KoopaBombEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next)
    {
        state = next;
    }
    std::auto_ptr<OwnedModel> firstModel;
    std::auto_ptr<OwnedModel> secondModel;
    Vec position;
    Vec scale;
    // The game's code never reads this field.
    int unk30;
    // Frames elapsed since the vertical movement began.
    int verticalFrames;
    // Factor applied to the vertical descent.
    float verticalSpeed;
    int countdown;

    // Sets the remaining frames in the countdown.
    void setCountdown(int frames)
    {
        countdown = frames;
    }
    // Motion frames at which the remaining sound cues play.
    std::vector<int> pendingFrames;

    void playQueuedMotionCue();
    void startMotionAndRevealModel();
    void lowerModels();
    void releaseMotionAfterLanding();
    void sendCharacterToDestination();
    void startSecondModel();
    void hideFirstModel();
    void startFinalMotionAndCamera();
    void waitForFinalMotionShift();
};

#endif
