#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/m639DLL/player_control.h"
#include "REL/m639DLL/player.h"
#include "REL/framework/p_info.h"
#include "humath.h"
#include "REL/framework/random.h"
#include <math.h>

ScalarSourceOwner::~ScalarSourceOwner() {}
// Samples the controller triggers and applies the right-minus-left difference.
void ScalarSourceOwner::update()
{
    float leftTrigger = ((PlayerInfo *)owner)->triggerL();
    float rightTrigger = ((PlayerInfo *)owner)->triggerR();
    fn_1_75BC(static_cast<Player *>(owner), rightTrigger - leftTrigger);
}


MotionCycleOwner::MotionCycleOwner(void *owner)
 : PlayerControl(owner), valueIndex(((PlayerInfo *)owner)->difficulty()), phase(0.0f) {}
MotionCycleOwner::~MotionCycleOwner() {}
void MotionCycleOwner::clear() { phase = 0.0f; }
extern "C" const float lbl_1_rodata_24[5] = {5.4f, 6.5f, 10.0f, 12.0f, 0.0f};
// Advances the selected motion cycle and emits its phase and direction changes.
void MotionCycleOwner::update()
{
    const float *values = &lbl_1_rodata_24[valueIndex];
    float noise = (float)(1.0 + RandomFloat(-0.4f, 0.3f));
    float increment = (float)((double)noise *
        ((M_PI * (double)values[0]) /
         (double)secondsToFrames(1L)));
    float previousPhase = phase;

    if ((phase += increment) > (M_PI * 360.0 / 180.0)) {
        phase = (float)((double)phase - (M_PI * 360.0 / 180.0));
    }
    if (previousPhase < 1.5707964f && phase >= 1.5707964f) {
        fn_1_75BC(static_cast<Player *>(owner), 1.0f);
        return;
    }
    if (previousPhase < 4.712389f && phase >= 4.712389f) {
        fn_1_75BC(static_cast<Player *>(owner), -1.0f);
        return;
    }
    fn_1_75BC(static_cast<Player *>(owner), (float)sin((double)phase));
}
