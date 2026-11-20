#ifndef REL_M611DLL_TIMED_FLASH_H
#define REL_M611DLL_TIMED_FLASH_H

extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "REL/framework/list.h"

// Owns a model and tracks its position and display countdown.
struct TimedFlash : ListNode<TimedFlash> {
    OwnedModel model;
    s32 countdown;
    s32 frame;
    s32 alpha;
    Vec position;
    TimedFlash();
    ~TimedFlash();
};

void fn_1_91DC(HU3D_MODEL *model, Mtx *matrix);
void fn_1_93E0(TimedFlash *effect, Mtx parentMatrix);

void fn_1_9370(TimedFlash *effect);

void fn_1_9348(TimedFlash *effect);
void fn_1_9390(TimedFlash *effect);

#endif
