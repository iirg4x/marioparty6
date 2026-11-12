#ifndef REL_M653DLL_MOTION_WORK_H
#define REL_M653DLL_MOTION_WORK_H

#include <map>
#include "dolphin/mtx.h"
#include "REL/framework/p_info.h"
#include "REL/m653DLL/collision_sphere.h"
#include "REL/framework/category_sphere.h"

/* One player's movement: its collision sphere, transform, steering input and the sounds it has playing. */
struct M653MotionWork {
    void *resource_00;
    PlayerInfo *player_no_04;
    s32 vibration_request_08;
    s32 vibration_previous_0C;
    CollisionSphere sphere_10;
    RegisteredValue registeredPosition;
    Mtx matrix_4C;
    f32 scalar_7C;
    Vec position_80;
    f32 delta_x_8C;
    f32 delta_z_90;
    u8 storage_94[4];
    u8 active_98;
    u8 storage_99[3];
    Vec vector_9C;
    Vec vector_A8;
    Quaternion orientation_B4;
    Quaternion rotation_delta_C4;
    std::map<int, int> sounds_D4;
    s32 sound_state_E4;
    s32 sound_handle_E8;
    s32 state_EC;
    M653MotionWork(void *target, PlayerInfo *kind);
    void fn_1_4454();
};

#endif
