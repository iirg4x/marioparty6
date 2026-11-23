// Runs Coins stolen and moves the models used in the event.

#ifndef REL_SEER_TERROR_COINS_STOLEN_EVENT_H
#define REL_SEER_TERROR_COINS_STOLEN_EVENT_H

#include "REL/m677DLL/event_base.h"
#include "REL/m677DLL/route_runner.h"

// Keeps the model and animation state of one effect entry.
struct CoinsStolenEffectModel {
    std::auto_ptr<OwnedModel> model;
    Vec position;
    Vec rotation;
    Vec velocity;
    int type;
    float elapsedTime;
    float startDelay;
    bool active;
    CoinsStolenEffectModel();
    ~CoinsStolenEffectModel();
};

// Advances the event's effects and their countdowns.
class CoinsStolenEvent : public EventBase {
public:
    CoinsStolenEvent();
    virtual ~CoinsStolenEvent();
    virtual void begin();
    virtual int step();

    typedef void (CoinsStolenEvent::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    CoinsStolenEffectModel entries[20];
    RouteRunner runners[6];
    int frame;
    bool effectEnabled;
    int nextRunner;
    int stateCountdown;

    // Sets the frames before the state advances.
    void setStateCountdown(int frames) { stateCountdown = frames; }
    int effectCountdown;

    // Sets the frames before the next effect update.
    void setEffectCountdown(int frames) { effectCountdown = frames; }

    void scatterFallingEffects();
    void updateEffectTransforms();
    void spawnTimedParticleBursts();
    void waitForOpeningCountdown();
    void waitForFinalLanding();
    void startRunnerPaths();
    void waitForLeadRunnerToStop();
    void enableParticleBursts();
    void waitForLeadRunnerPath();
    void launchEffectsAtRunners();
    void sendRunnersToDestinations();
    void waitForFinalRunnerPath();
    void configureSceneMode(int);
    void updateFlyingEffect(int);
    void queueRunnerApproach(RouteRunner &, int);
    void queueRunnerDestination(RouteRunner &, int);
};

#endif
