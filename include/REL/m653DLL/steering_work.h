#ifndef REL_M653DLL_STEERING_WORK_H
#define REL_M653DLL_STEERING_WORK_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
struct Car;
struct M653SteeringValue {
    s32 value;
    M653SteeringValue() : value(0) {}
    s32 get() const { return value; }
    // Counts down one frame; true on the frame the count reaches zero.
    bool update()
    {
        bool expired = false;
        if (value > 0) {
            if (--value == 0) {
                expired = true;
            }
        }
        return expired;
    }
};

struct M653SteeringWork {
    s32 difficulty; /* The player's difficulty; never read (skill is worked out from it). */
    f32 skill;
    Car *car;
    Vec position;
    Vec opponentPosition;
    Vec forward;
    Vec side;
    Vec wallNormal;
    f32 wallDistance;
    M653SteeringValue stateTimer;
    M653SteeringValue chargeTimer;
    M653SteeringValue turnTimer; /* Set when the car turns away from a wall; never read. */
    u8 turnFlag; /* Cleared when the car turns away from a wall; never read. */
    s32 state;
    s32 carIndex;
    M653SteeringWork(s32 upperValue, s32 lowerValue, Car *owner);
};

extern "C" {
f32 fn_1_2300(const Vec *source, Vec *destination);
f32 fn_1_24C4(const Vec *left, const Vec *right);
void fn_1_268C(M653SteeringWork *work);
void fn_1_2A30(M653SteeringWork *work);
void fn_1_2E60(M653SteeringWork *work, s32 state);
void fn_1_2EF8(M653SteeringWork *object);
void fn_1_3040(M653SteeringWork *state);
void fn_1_333C(M653SteeringWork *state);
void fn_1_3444(M653SteeringWork *owner);
void fn_1_364C(M653SteeringWork *work);
void fn_1_36CC(M653SteeringWork *source, const Vec *direction);
}
f32 fn_1_37CC(f32 seconds);
#endif
