// Converts camera frames and poses and blends them during minigame camera motion.
static const char rcsid[] = "$Id: camera_ut1.cpp,v 1.5 2004/02/17 09:22:00 hanamasu Exp $";

extern "C" {
#include "dolphin/mtx.h"
}

// A camera frame stores the eye position, the direction above the eye, and the look target.
struct CameraFrame {
    Vec position; // World-space camera eye position.
    Vec up;        // Direction that defines which way is above the camera.
    Vec target;    // World-space point the camera looks toward.
};

// A camera pose stores the eye position and its orientation independently of the look target.
struct CameraPose {
    Vec position;         // World-space camera eye position.
    Quaternion rotation; // Camera orientation.
};

void cameraInverseLookAt(const CameraFrame *, Mtx);

// interpolate_camera_poses calls this to apply a pose's orientation and eye to a camera frame.
void set_camera_frame_from_pose(CameraFrame *frame, const CameraPose *pose)
{
    Mtx rotationMatrix;
    PSMTXQuat(rotationMatrix, &pose->rotation);
    frame->up.x = rotationMatrix[0][1];
    frame->up.y = rotationMatrix[1][1];
    frame->up.z = rotationMatrix[2][1];
    frame->position.x = pose->position.x;
    frame->position.y = pose->position.y;
    frame->position.z = pose->position.z;
    // The matrix's forward axis points from the eye toward the scene, opposite the look-at Z axis.
    frame->target.x = pose->position.x - rotationMatrix[0][2];
    frame->target.y = pose->position.y - rotationMatrix[1][2];
    frame->target.z = pose->position.z - rotationMatrix[2][2];
}

// Frame interpolation and debug camera inspection use this to read an eye and orientation pose.
void get_camera_pose_from_frame(const CameraFrame *frame, CameraPose *pose)
{
    Mtx cameraToWorldMatrix;
    cameraInverseLookAt(frame, cameraToWorldMatrix);
    pose->position.x = frame->position.x;
    pose->position.y = frame->position.y;
    pose->position.z = frame->position.z;
    C_QUATMtx(&pose->rotation, cameraToWorldMatrix);
}

// interpolate_camera_frames calls this to blend two poses and update the destination camera frame.
void interpolate_camera_poses(CameraFrame *frame, const CameraPose *startPose,
                              const CameraPose *endPose, float blendAmount)
{
    CameraPose interpolated;
    Vec startPositionPart, endPositionPart;
    C_QUATSlerp(&startPose->rotation, &endPose->rotation, &interpolated.rotation, blendAmount);
    PSVECScale(&startPose->position, &startPositionPart, 1.0f - blendAmount);
    PSVECScale(&endPose->position, &endPositionPart, blendAmount);
    PSVECAdd(&startPositionPart, &endPositionPart, &interpolated.position);
    set_camera_frame_from_pose(frame, &interpolated);
}

// Camera transition callers use this to blend endpoint frames at their chosen blend amount.
void interpolate_camera_frames(CameraFrame *frame, const CameraFrame *startFrame,
                                const CameraFrame *endFrame, float blendAmount)
{
    CameraPose startPose, endPose;
    get_camera_pose_from_frame(startFrame, &startPose);
    get_camera_pose_from_frame(endFrame, &endPose);
    interpolate_camera_poses(frame, &startPose, &endPose, blendAmount);
}
