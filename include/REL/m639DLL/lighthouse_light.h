#ifndef M639_LIGHTHOUSE_LIGHT_H
#define M639_LIGHTHOUSE_LIGHT_H
#include <vector>
#include "REL/framework/game_object.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "REL/m639DLL/transition.h"

class LighthouseLight : public GameObject, public OwnedModel {
public:
    void (LighthouseLight::*callback)();
    int selector;
    float parameter;
    Transition<float> transition;
    float accumulatedAngle;
    float *angleCosine;
    HSF_TRANSFORM *cameraState;
    std::vector<HSF_TRANSFORM *> children; // The street lights' model nodes.
    float phase;
    LighthouseLight(unsigned int dataNum, int selector);
    virtual ~LighthouseLight();
    virtual void update();
    void fn_1_D23C();
    void fn_1_D38C();
    void fn_1_D44C();
    void fn_1_D670();
    bool fn_1_DA30() const;
    double sampledScalar() const { return transition.current; }
};
#endif
