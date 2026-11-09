#ifndef REL_M652DLL_RACER_CONTROL_H
#define REL_M652DLL_RACER_CONTROL_H
#include <vector>
#include "dolphin/types.h"
#include "REL/m652DLL/tail_types.h"
struct Racer;
struct RouteStep {
    s32 firstPoint;
    s32 secondPoint;
    unsigned int nextPoint;
    float endZ;
};

class RacerControl {
public:
    RacerControl(Racer *racer) : racer(racer) {}
    virtual ~RacerControl();
    virtual void reset();
    virtual void update() = 0;

    Racer *racer;
};

class HumanControl : public RacerControl {
public:
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

    s32 index;
    s32 unknown0C;
    s32 currentStep;
    s32 nextStep;
    f32 rotation;
    s32 collisionTimer;
    s32 timer;
    u8 moving : 1;
    u8 unknownFlags : 7;
    std::vector<RouteStep> steps;
    CameraArcMotion padControl;
};

#endif
