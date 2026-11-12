#include "REL/framework/sound.h"
#include "REL/framework/p_info.h"
/* Player movement: motion state, collision response, turning and engine sound. */
#include "REL/m653DLL/motion_work.h"
#include "REL/m653DLL/boundary_plane.h"
#include "REL/m653DLL/absolute.h"
#include "REL/m653DLL/translate_matrix.h"

extern "C" {
#include "game/audio.h"
#include "game/data.h"
#include "game/hu3d.h"
}
#include "REL/m653DLL.h"

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
    : asset(target), player(kind), sphere(0, this), registeredPosition(0)
{
    s32 groupNo;
    f32 sphereRadius;
    f32 registeredRadius;

    resetPosition(this);
    sphereRadius = 150.0f;
    sphere.radius = sphereRadius;
    rumbleRequest = -1;
    rumblePrevious = -1;
    PSMTXIdentity(matrix);
    yaw = 90.0f;
    leftDrive = 0.0f;
    rightDrive = 0.0f;
    falling = 0;
    fallVelocity.x = 0.0f;
    fallVelocity.y = 0.0f;
    fallVelocity.z = 0.0f;
    knockback.x = 0.0f;
    knockback.y = 0.0f;
    knockback.z = 0.0f;
    fallSpin.x = 0.0f;
    fallSpin.y = 0.0f;
    fallSpin.z = 0.0f;
    fallSpin.w = 1.0f;
    registeredRadius = 200.0f;
    registeredPosition.radius = registeredRadius;
    groupNo = player->groupNo();
    engineMuted = 0;
    engineSound = soundPlay(groupNo + 0x818);
    unkEC = -1;
}

void fn_1_3B04(M653MotionWork *work, const Sphere *target)
{
    Sphere target_copy = *target;
    Vec work_position = work->sphere.center;
    Vec difference;
    resolveSphereContact(&target_copy, &work->sphere, 0.0f, 1.0f);
    PSVECSubtract(&work->sphere.center, &work_position, &difference);
    work->position.x += difference.x;
    work->position.z += difference.z;
}

// Halfway between two values.
static inline f32 midpoint(f32 first, f32 second)
{
    return 0.5 * first + 0.5 * second;
}

void fn_1_3BC4(M653MotionWork *first, M653MotionWork *second)
{
    Vec firstPosition = first->sphere.center;
    Vec secondPosition = second->sphere.center;

    if (resolveSphereContact(&first->sphere, &second->sphere, 0.5f, 1.0f)) {
        Vec modelPosition;
        s16 model;

        fn_1_4294(first, 1);
        fn_1_4294(second, 1);
        PSVECSubtract(&first->sphere.center, &firstPosition, &first->knockback);
        PSVECSubtract(&second->sphere.center, &secondPosition, &second->knockback);
        if (first->knockback.length() > 1.0f) {
            model = fn_80036DF8(fn_80008DD0(0x70000C, 0x10000000, 2));
            modelPosition.x = midpoint(firstPosition.x, secondPosition.x);
            modelPosition.y = midpoint(firstPosition.y, secondPosition.y);
            modelPosition.z = midpoint(firstPosition.z, secondPosition.z);
            modelPosition.y += 150.0;
            fn_80026790(model, &modelPosition);
        }
        first->knockback.y = 0.0f;
        second->knockback.y = 0.0f;
        PSVECScale(&first->knockback, &first->knockback, 5.2f);
        PSVECScale(&second->knockback, &second->knockback, 5.2f);
    }
}

void fn_1_3EC4(M653MotionWork *work, const Vec *position)
{
    work->position.x = position->x;
    work->position.y = position->y;
    work->position.z = position->z;
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

static inline f64 square(const f64 &value)
{
    return value * value;
}

void fn_1_3EE0(M653MotionWork *work)
{
    Vector3 targetDirection;
    Vec rotationAxis;
    Vec translation;
    f32 deltaX;
    f32 deltaZ;
    f32 absoluteTotal;
    f32 turnDelta;
    f32 scale;

    work->registeredPosition = work->position;
    if (work->falling != 0) {
        work->fallVelocity.y = work->fallVelocity.y - 300.0 * square(frameTime);
        PSVECAdd(&work->fallVelocity, &work->position, &work->position);
        setNodePosition(&work->sphere, &work->position);
        PSQUATMultiply(&work->fallSpin, &work->orientation, &work->orientation);
    } else if (lbl_1_bss_64 != 0) {
        fn_1_429C(work);
        PSVECAdd(&work->knockback, &work->position, &work->position);
        PSVECScale(&work->knockback, &work->knockback, 0.9f);
        if (work->falling == 0) {
            if (fn_1_9568(work->position.x, work->position.z) < 0.0f) {
                work->falling = 1;
                fn_1_9D30(work->position.x, work->position.z, &targetDirection);
                rotationAxis.x = 0.0f;
                rotationAxis.y = -1.0f;
                rotationAxis.z = 0.0f;
                PSVECCrossProduct(&rotationAxis, &targetDirection, &targetDirection);
                C_QUATRotAxisRad(&work->fallSpin, &targetDirection, MTXDegToRad(1.0f));
                targetDirection.x = 0.0f;
                targetDirection.y = 0.0f;
                targetDirection.z = 243.14915f;
                PSVECSubtract(&work->position, &targetDirection, &targetDirection);
                scale = 2.0f / targetDirection.length();
                PSVECScale(&targetDirection, &targetDirection, scale);
                PSVECAdd(&targetDirection, &work->fallVelocity, &work->fallVelocity);
            }
        }
    deltaX = work->leftDrive;
    deltaZ = work->rightDrive;
    work->leftDrive = 0.0f;
    work->rightDrive = 0.0f;
    absoluteTotal = absolute(deltaX) + absolute(deltaZ);
    if (absoluteTotal > 0.01f) {
        turnDelta = -deltaX + deltaZ;
        work->yaw += turnDelta / absoluteTotal;
        translation.x = work->matrix[0][2];
        translation.y = work->matrix[1][2];
        translation.z = work->matrix[2][2];
        PSVECScale(&translation, &translation, 0.7f * (deltaX + deltaZ));
        PSVECAdd(&translation, &work->position, &work->position);
    }
    setNodePosition(&work->sphere, &work->position);
    }
}

void fn_1_4294(M653MotionWork *object, u32 value)
{
    object->rumbleRequest = value;
}

void fn_1_429C(M653MotionWork *work)
{
    if ((work->rumblePrevious < work->rumbleRequest) &&
        (work->rumbleRequest >= 0)) {
        work->player->rumble(work->rumbleRequest, 0x14);
    }
    work->rumblePrevious = work->rumbleRequest;
    work->rumbleRequest = -1;
}

void fn_1_4300(M653MotionWork *work, f32 delta)
{
    work->leftDrive += delta;
}

void fn_1_4310(M653MotionWork *work, f32 delta)
{
    work->rightDrive += delta;
}

void fn_1_4320(M653MotionWork *) {}

void fn_1_4324(M653MotionWork *work, Mtx output)
{
    Mtx quaternionMatrix;
    Mtx rotationMatrix;
    f32 yaw;

    PSMTXQuat(quaternionMatrix, &work->orientation);
    yaw = work->yaw;
    PSMTXRotRad(rotationMatrix, 0x59, MTXDegToRad(yaw));
    PSMTXConcat(quaternionMatrix, rotationMatrix, work->matrix);
    translateMatrix(work->matrix, work->matrix, work->position.x, work->position.y,
                    work->position.z);
    PSMTXCopy(work->matrix, output);
}

void fn_1_43FC(M653MotionWork *work, const Mtx matrix)
{
    C_QUATMtx(&work->orientation, matrix);
    work->position.x = matrix[0][3];
    work->position.y = matrix[1][3];
    work->position.z = matrix[2][3];
}

void M653MotionWork::fn_1_4454() {}

void fn_1_4458(M653MotionWork *work, int key)
{
    s32 status;

    status = HuAudFXStatusGet(work->sounds[key]);
    if (status != 2) {
        work->sounds[key] = soundPlayAt(1, key + work->player->number(), &work->position);
    }
}

void fn_1_44E4(M653MotionWork *work, float pitch)
{
    if (work->engineMuted != 0) work->engineMuted = 0;
    HuAudFXVolSet(work->engineSound, 127);
    HuAudFXPitchSet(work->engineSound, (short)(8191.0f * pitch));
}

void fn_1_4558(M653MotionWork *work)
{
    if (work->engineMuted != 1) {
        work->engineMuted = 1;
        HuAudFXVolSet(work->engineSound, 0);
    }
}
