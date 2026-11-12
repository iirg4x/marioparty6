#ifndef M653_MOVING_OBJECT_H
#define M653_MOVING_OBJECT_H
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/frame_interpolator.h"
/* The position interval is copied as a whole by construction and consumed as
 * three vector components by the movement and light operations. */
struct MovingPosition { Vec vector; };
struct MovingObject : GameObject, OwnedModel {
    void (MovingObject::*callback)();
    s16 m_light_id;
    MovingPosition position;
    f32 velocityY;
    FrameInterpolator<float> transition;
    s32 countdown;
    MovingObject();
    virtual ~MovingObject();
    virtual void update();
    void fn_1_5840();
    void fn_1_5A54(); void fn_1_5AB4(); void fn_1_5C8C(); void fn_1_5E08();
    void resetCallback();
};
typedef void (MovingObject::*MovingObjectCallback)();
extern "C" MovingPosition lbl_1_data_120;
#endif
