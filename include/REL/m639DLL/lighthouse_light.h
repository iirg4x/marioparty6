#ifndef M639_LIGHTHOUSE_LIGHT_H
#define M639_LIGHTHOUSE_LIGHT_H
#include <vector>
#include "REL/framework/game_object.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "REL/framework/frame_interpolator.h"

class LighthouseLight : public GameObject, public OwnedModel {
public:
    void (LighthouseLight::*callback)();
    int selector;
    float parameter;
    FrameInterpolator<float> transition;
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
    double sampledScalar() const { return transition.value; }
};
extern "C" {
void fn_1_CD58();
unsigned char fn_1_CE60();
void fn_1_D69C(LighthouseLight *owner);
}
#endif
