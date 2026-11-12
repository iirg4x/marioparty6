#ifndef REL_M653DLL_MOTION_WORK_H
#define REL_M653DLL_MOTION_WORK_H

#include <map>
#include "dolphin/mtx.h"
#include "REL/framework/p_info.h"
#include "REL/m653DLL/collision_sphere.h"
#include "REL/framework/category_sphere.h"
#include "REL/framework/vector3.h"

/* One player's movement: its collision sphere, transform, steering input and the sounds it has playing. */
struct M653MotionWork {
    void *asset; /* The car's asset pointer (always null); never read. */
    PlayerInfo *player;
    s32 rumbleRequest;
    s32 rumblePrevious;
    CollisionSphere sphere;
    RegisteredValue registeredPosition;
    Mtx matrix;
    f32 yaw;
    Vec position;
    f32 leftDrive;
    f32 rightDrive;
    unsigned char unknown94[4]; /* No code names this field. */
    u8 falling;
    Vec fallVelocity;
    Vector3 knockback;
    Quaternion orientation;
    Quaternion fallSpin;
    std::map<int, int> sounds;
    s32 engineMuted;
    s32 engineSound;
    s32 unkEC; /* Set to -1 by the constructor; never read. */
    M653MotionWork(void *target, PlayerInfo *kind);
    void fn_1_4454();
};

extern "C" {
void fn_1_3B04(M653MotionWork *work, const Sphere *target);
void fn_1_3BC4(M653MotionWork *first, M653MotionWork *second);
void fn_1_3EC4(M653MotionWork *work, const Vec *position);
void fn_1_3EE0(M653MotionWork *work);
void fn_1_4294(M653MotionWork *object, u32 value);
void fn_1_429C(M653MotionWork *work);
void fn_1_4300(M653MotionWork *work, f32 delta);
void fn_1_4310(M653MotionWork *work, f32 delta);
void fn_1_4320(M653MotionWork *work);
void fn_1_4324(M653MotionWork *work, Mtx output);
void fn_1_43FC(M653MotionWork *work, const Mtx matrix);
void fn_1_4458(M653MotionWork *work, int key);
void fn_1_44E4(M653MotionWork *work, float pitch);
void fn_1_4558(M653MotionWork *work);
}

#endif
