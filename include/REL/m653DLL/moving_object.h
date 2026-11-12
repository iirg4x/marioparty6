#ifndef M653_MOVING_OBJECT_H
#define M653_MOVING_OBJECT_H
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/frame_interpolator.h"
struct MovingObject : GameObject, OwnedModel {
    typedef void (MovingObject::*Callback)();
    Callback callback;
    s16 m_light_id;
    Vec position;
    f32 velocityY;
    FrameInterpolator<float> transition;
    s32 countdown;
    MovingObject();
    virtual ~MovingObject();
    virtual void update();
    void fn_1_5840();
    void fn_1_5A54(); void fn_1_5AB4(); void fn_1_5C8C(); void fn_1_5E08();
    void setCallback(Callback function) { callback = function; }
    void setCountdown(s32 frames) { countdown = frames; }
    void restartWait();
};
void fn_1_5900(MovingObject *object);
void fn_1_5978(MovingObject *owner);
void fn_1_5BF0(MovingObject *object);
void fn_1_5D78(MovingObject *state);
void fn_1_5F4C(MovingObject *object);
void fn_1_5FD4(MovingObject *object);
extern "C" Vec lbl_1_data_120;
#endif
