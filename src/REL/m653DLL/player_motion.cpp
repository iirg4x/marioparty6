#include "REL/framework/sound.h"
#include "REL/framework/p_info.h"
/* Player movement: motion state, collision response, turning and engine sound. */
#include "REL/m653DLL/motion_work.h"
#include "REL/m653DLL/boundary_plane.h"

extern "C" {
#include "game/audio.h"
#include "game/data.h"
#include "game/hu3d.h"
extern u8 lbl_1_bss_64;
void fn_1_3B04(M653MotionWork *work, const Sphere *target);
void fn_1_3BC4(M653MotionWork *first, M653MotionWork *second);
void fn_1_3EC4(M653MotionWork *work, const Vec *position);
void fn_1_3EE0(M653MotionWork *work);
void fn_1_4294(M653MotionWork *object, u32 value);
void fn_1_429C(M653MotionWork *work);
void fn_1_4300(M653MotionWork *work, f32 delta);
void fn_1_4310(M653MotionWork *work, f32 delta);
void fn_1_4320(void);
void fn_1_4324(M653MotionWork *work, Mtx output);
void fn_1_43FC(M653MotionWork *work, const Mtx matrix);

void fn_1_4458(M653MotionWork *work, int key);
void fn_1_44E4(M653MotionWork *work, float pitch);
void fn_1_4558(M653MotionWork *work);
}
f32 fn_1_9D30(f32, f32, Vec *);
extern "C" {

}

static inline void resetPosition(M653MotionWork *work)
{
    Vec center;
    center.x = 0.0f;
    center.y = 0.0f;
    center.z = 0.0f;
    Vec *centerPointer = &center;
    fn_1_3EC4(work, centerPointer);
}

M653MotionWork::M653MotionWork(void *target, PlayerInfo *kind)
    : resource_00(target), player_no_04(kind), sphere_10(0, this), registeredPosition(0)
{
    s32 modelData;
    f32 cameraDistance;
    f32 secondaryDistance;

    resetPosition(this);
    cameraDistance = 150.0f;
    sphere_10.radius = cameraDistance;
    vibration_request_08 = -1;
    vibration_previous_0C = -1;
    PSMTXIdentity(matrix_4C);
    scalar_7C = 90.0f;
    delta_x_8C = 0.0f;
    delta_z_90 = 0.0f;
    active_98 = 0;
    vector_9C.x = 0.0f;
    vector_9C.y = 0.0f;
    vector_9C.z = 0.0f;
    vector_A8.x = 0.0f;
    vector_A8.y = 0.0f;
    vector_A8.z = 0.0f;
    rotation_delta_C4.x = 0.0f;
    rotation_delta_C4.y = 0.0f;
    rotation_delta_C4.z = 0.0f;
    rotation_delta_C4.w = 1.0f;
    secondaryDistance = 200.0f;
    registeredPosition.radius = secondaryDistance;
    modelData = player_no_04->groupNo();
    sound_state_E4 = 0;
    sound_handle_E8 = soundPlay(modelData + 0x818);
    state_EC = -1;
}

void fn_1_3B04(M653MotionWork *work, const Sphere *target)
{
    Sphere target_copy = *target;
    Vec work_position = work->sphere_10.center;
    Vec difference;
    resolveSphereContact(&target_copy, &work->sphere_10, 0.0f, 1.0f);
    PSVECSubtract(&work->sphere_10.center, &work_position, &difference);
    work->position_80.x += difference.x;
    work->position_80.z += difference.z;
}

static inline f32 midpoint3BC4(f32 first, f32 second)
{
    return 0.5 * first + 0.5 * second;
}

static inline f32 separation3BC4(const Vec *offset)
{
    return PSVECMag(offset);
}

void fn_1_3BC4(M653MotionWork *first, M653MotionWork *second)
{
    Vec firstPosition = first->sphere_10.center;
    Vec secondPosition = second->sphere_10.center;

    if (resolveSphereContact(&first->sphere_10, &second->sphere_10, 0.5f, 1.0f)) {
        Vec modelPosition;
        s16 model;

        fn_1_4294(first, 1);
        fn_1_4294(second, 1);
        PSVECSubtract(&first->sphere_10.center, &firstPosition, &first->vector_A8);
        PSVECSubtract(&second->sphere_10.center, &secondPosition, &second->vector_A8);
        if (separation3BC4(&first->vector_A8) > 1.0f) {
            model = fn_80036DF8(fn_80008DD0(0x70000C, 0x10000000, 2));
            modelPosition.x = midpoint3BC4(firstPosition.x, secondPosition.x);
            modelPosition.y = midpoint3BC4(firstPosition.y, secondPosition.y);
            modelPosition.z = midpoint3BC4(firstPosition.z, secondPosition.z);
            modelPosition.y += 150.0;
            fn_80026790(model, &modelPosition);
        }
        first->vector_A8.y = 0.0f;
        second->vector_A8.y = 0.0f;
        PSVECScale(&first->vector_A8, &first->vector_A8, 5.2f);
        PSVECScale(&second->vector_A8, &second->vector_A8, 5.2f);
    }
}

void fn_1_3EC4(M653MotionWork *work, const Vec *position)
{
    work->position_80.x = position->x;
    work->position_80.y = position->y;
    work->position_80.z = position->z;
}

static inline Vec *nodePosition(CollisionSphere *node)
{
    return &node->center;
}

static inline void setNodePosition(CollisionSphere *node, const Vec *position)
{
    Vec *destination = nodePosition(node);
    destination->x = position->x;
    destination->y = position->y;
    destination->z = position->z;
}

static const f64 frameTime = 1.0 / 60.0;

static inline f32 vectorMagnitude(Vec *vector)
{
    return PSVECMag(vector);
}

static inline f64 square(const f64 &value)
{
    return value * value;
}

void fn_1_3EE0(M653MotionWork *work)
{
    Vec targetDirection;
    Vec rotationAxis;
    Vec translation;
    f32 deltaX;
    f32 deltaZ;
    f32 absoluteTotal;
    f32 absoluteX;
    f32 absoluteZ;
    f32 turnDelta;
    f32 scale;

    work->registeredPosition = work->position_80;
    if (work->active_98 != 0) {
        work->vector_9C.y = work->vector_9C.y - 300.0 * square(frameTime);
        PSVECAdd(&work->vector_9C, &work->position_80, &work->position_80);
        setNodePosition(&work->sphere_10, &work->position_80);
        PSQUATMultiply(&work->rotation_delta_C4, &work->orientation_B4, &work->orientation_B4);
    } else if (lbl_1_bss_64 != 0) {
        fn_1_429C(work);
        PSVECAdd(&work->vector_A8, &work->position_80, &work->position_80);
        PSVECScale(&work->vector_A8, &work->vector_A8, 0.9f);
        if (work->active_98 == 0) {
            if (fn_1_9568(work->position_80.x, work->position_80.z) < 0.0f) {
                work->active_98 = 1;
                fn_1_9D30(work->position_80.x, work->position_80.z, &targetDirection);
                rotationAxis.x = 0.0f;
                rotationAxis.y = -1.0f;
                rotationAxis.z = 0.0f;
                PSVECCrossProduct(&rotationAxis, &targetDirection, &targetDirection);
                C_QUATRotAxisRad(&work->rotation_delta_C4, &targetDirection, MTXDegToRad(1.0f));
                targetDirection.x = 0.0f;
                targetDirection.y = 0.0f;
                targetDirection.z = 243.14915f;
                PSVECSubtract(&work->position_80, &targetDirection, &targetDirection);
                scale = 2.0f / vectorMagnitude(&targetDirection);
                PSVECScale(&targetDirection, &targetDirection, scale);
                PSVECAdd(&targetDirection, &work->vector_9C, &work->vector_9C);
            }
        }
    deltaX = work->delta_x_8C;
    deltaZ = work->delta_z_90;
    work->delta_x_8C = 0.0f;
    work->delta_z_90 = 0.0f;
    if (deltaX < 0.0f) absoluteX = -deltaX;
    else absoluteX = deltaX;
    if (deltaZ < 0.0f) absoluteZ = -deltaZ;
    else absoluteZ = deltaZ;
    absoluteTotal = absoluteX + absoluteZ;
    if (absoluteTotal > 0.01f) {
        turnDelta = -deltaX + deltaZ;
        work->scalar_7C += turnDelta / absoluteTotal;
        translation.x = work->matrix_4C[0][2];
        translation.y = work->matrix_4C[1][2];
        translation.z = work->matrix_4C[2][2];
        PSVECScale(&translation, &translation, 0.7f * (deltaX + deltaZ));
        PSVECAdd(&translation, &work->position_80, &work->position_80);
    }
    setNodePosition(&work->sphere_10, &work->position_80);
    }
}

void fn_1_4294(M653MotionWork *object, u32 value)
{
    object->vibration_request_08 = value;
}

void fn_1_429C(M653MotionWork *work)
{
    if ((work->vibration_previous_0C < work->vibration_request_08) &&
        (work->vibration_request_08 >= 0)) {
        work->player_no_04->rumble(work->vibration_request_08, 0x14);
    }
    work->vibration_previous_0C = work->vibration_request_08;
    work->vibration_request_08 = -1;
}

void fn_1_4300(M653MotionWork *work, f32 delta)
{
    work->delta_x_8C += delta;
}

void fn_1_4310(M653MotionWork *work, f32 delta)
{
    work->delta_z_90 += delta;
}

void fn_1_4320(void) {}

void fn_1_4324(M653MotionWork *work, Mtx output)
{
    Mtx quaternionMatrix;
    Mtx rotationMatrix;
    f32 angle;
    f32 x;
    f32 y;
    f32 z;

    PSMTXQuat(quaternionMatrix, &work->orientation_B4);
    angle = work->scalar_7C;
    PSMTXRotRad(rotationMatrix, 0x59, MTXDegToRad(angle));
    PSMTXConcat(quaternionMatrix, rotationMatrix, work->matrix_4C);
    z = work->position_80.z;
    y = work->position_80.y;
    x = work->position_80.x;
    PSMTXTransApply(work->matrix_4C, work->matrix_4C, x, y, z);
    PSMTXCopy(work->matrix_4C, output);
}

void fn_1_43FC(M653MotionWork *work, const Mtx matrix)
{
    C_QUATMtx(&work->orientation_B4, matrix);
    work->position_80.x = matrix[0][3];
    work->position_80.y = matrix[1][3];
    work->position_80.z = matrix[2][3];
}

void M653MotionWork::fn_1_4454() {}

void fn_1_4458(M653MotionWork *work, int key)
{
    PlayerInfo *playerNumberAddress;
    s32 status;
    s32 playerNumber;
    s32 playerNumberForEffect;

    status = HuAudFXStatusGet(work->sounds_D4[key]);
    if (status != 2) {
        playerNumberAddress = work->player_no_04;
        playerNumber = playerNumberAddress->playerNo;
        playerNumberForEffect = playerNumber;
        work->sounds_D4[key] = soundPlayAt(1, key + playerNumberForEffect, &work->position_80);
    }
}

void fn_1_44E4(M653MotionWork *work, float pitch)
{
    if (work->sound_state_E4 != 0) work->sound_state_E4 = 0;
    HuAudFXVolSet(work->sound_handle_E8, 127);
    HuAudFXPitchSet(work->sound_handle_E8, (short)(8191.0f * pitch));
}

void fn_1_4558(M653MotionWork *work)
{
    if (work->sound_state_E4 != 1) {
        work->sound_state_E4 = 1;
        HuAudFXVolSet(work->sound_handle_E8, 0);
    }
}
