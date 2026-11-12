#ifndef REL_M655DLL_HEIHO_H
#define REL_M655DLL_HEIHO_H
#include "REL/m655DLL/player_list.h"
void sendEvent(int channel, unsigned long value, float delay);

struct Goal;

struct SequenceEventObject;

void fn_1_4538(SequenceEventObject *);

s32 fn_1_7CB8(void);

void fn_1_50C8(TargetEffect *owner);

struct RaisedPosition : Vec {
    RaisedPosition(f64 x, f64 y, f64 z)
    {
        this->x = (f32)x;
        this->y = (f32)y;
        this->z = (f32)z;
    }
};

#endif
