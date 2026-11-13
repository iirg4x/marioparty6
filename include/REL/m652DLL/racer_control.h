#ifndef REL_M652DLL_RACER_CONTROL_H
#define REL_M652DLL_RACER_CONTROL_H
#include <vector>
#include "dolphin/types.h"
#include "REL/m652DLL/camera_arc_motion.h"
struct Racer;
struct RouteStep {
    s32 firstPoint;
    s32 secondPoint;
    unsigned int nextPoint;
    float endZ;
};

class RacerControl {
public:
    /* Stores the racer controlled by this object. */
    RacerControl(Racer *racer) : racer(racer) {}
    virtual ~RacerControl();
    virtual void reset();
    virtual void update() = 0;

    Racer *racer;
};

class HumanControl : public RacerControl {
public:
    /* Initializes the controller with its racer. */
    HumanControl(Racer *racer) : RacerControl(racer) {}
    virtual ~HumanControl();
    virtual void update();
};

class ComputerControl : public RacerControl {
public:
    ComputerControl(Racer *racer);
    virtual ~ComputerControl();
    virtual void reset();
    virtual void update();

    s32 difficulty;
    s32 resetCount;
    s32 currentStep;
    s32 nextStep;
    f32 stepDistanceFactor;
    s32 collisionTimer;
    s32 timer;
    u8 moving : 1;
    std::vector<RouteStep> steps;
    PlayerPadControl padControl;
};

#endif
