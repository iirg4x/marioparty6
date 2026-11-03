// Camera view and system pause controls used by the object manager.
#define _MATH_H
#define M_PI 3.141592653589793
double sin(double x);
double cos(double x);
#include "game/object.h"
#include "game/pad.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/process.h"
#include "game/audio.h"

// Returns the MgDataTbl index associated with an overlay, or -1 if it is not listed.
extern s32 MgNoGet(s16 ovlNo);
// Opens the pause menu for the current minigame.
extern void GameMesPauseCreate(void);

float lbl_80266BD8[16];
HuVecF lbl_80266B18[16];
HuVecF lbl_80266A58[16];
float lbl_802C028C;
HuVecF lbl_80266A4C;
HuVecF lbl_80266A40;

s32 omDBGMenuButton;

// The object manager calls this callback to apply the active single camera each frame.
void fn_8003D4FC(OMOBJ *outViewObj)
{
    Vec cameraPos;
    Vec cameraTarget;
    Vec cameraUp;
    Vec rotatedUp;
    Vec viewDir;
    float rotX = lbl_80266A40.x;
    float rotY = lbl_80266A40.y;
    float rotZ = lbl_80266A40.z;
    cameraPos.x =
        lbl_80266A4C.x+(lbl_802C028C*(sin(3.141592653589793*rotY/180.0)
        *cos(3.141592653589793*rotX/180.0)));
    cameraPos.y = lbl_80266A4C.y+(lbl_802C028C*-sin(3.141592653589793*rotX/180.0));
    cameraPos.z =
        lbl_80266A4C.z+(lbl_802C028C*(cos(3.141592653589793*rotY/180.0)
        *cos(3.141592653589793*rotX/180.0)));
    cameraTarget.x = lbl_80266A4C.x;
    cameraTarget.y = lbl_80266A4C.y;
    cameraTarget.z = lbl_80266A4C.z;
    rotatedUp.x = sin(3.141592653589793*rotY/180.0)*sin(3.141592653589793*rotX/180.0);
    rotatedUp.y = cos(3.141592653589793*rotX/180.0);
    rotatedUp.z = cos(3.141592653589793*rotY/180.0)*sin(3.141592653589793*rotX/180.0);
    VECSubtract(&cameraPos, &cameraTarget, &viewDir);
    if(viewDir.x == 0.0 && viewDir.y == 0.0 && viewDir.z == 0.0) {
        viewDir.z = 1;
    }
    VECNormalize(&viewDir, &viewDir);
    // Apply Rodrigues' rotation to rotate the elevation-derived up vector around the normalized
    // view direction by rotZ; the result is normalized immediately afterward.
    cameraUp.x = (rotatedUp.x*((viewDir.x*viewDir.x)+((1.0f-(viewDir.x
        * viewDir.x))*cos(3.141592653589793*rotZ/180.0))))
        + (rotatedUp.y*(((viewDir.x*viewDir.y)*(1.0-cos(3.141592653589793*rotZ/180.0)))
            -(viewDir.z*sin(3.141592653589793*rotZ/180.0))))
        + (rotatedUp.z*(((viewDir.x*viewDir.z)*(1.0-cos(3.141592653589793*rotZ/180.0)))
            +(viewDir.y*sin(3.141592653589793*rotZ/180.0))));
    cameraUp.y = (rotatedUp.y*((viewDir.y*viewDir.y)+((1.0f-(viewDir.y
        * viewDir.y))*cos(3.141592653589793*rotZ/180.0))))
        + (rotatedUp.x*(((viewDir.x*viewDir.y)*(1.0-cos(3.141592653589793*rotZ/180.0)))
            +(viewDir.z*sin(3.141592653589793*rotZ/180.0))))
        + (rotatedUp.z*(((viewDir.y*viewDir.z)*(1.0-cos(3.141592653589793*rotZ/180.0)))
            -(viewDir.x*sin(3.141592653589793*rotZ/180.0))));
    cameraUp.z = (rotatedUp.z *
                  ((viewDir.z * viewDir.z)
                      + ((1.0f - (viewDir.z * viewDir.z)) * cos(3.141592653589793*rotZ/180.0)))) +
                 ((rotatedUp.x *
                   (((viewDir.x * viewDir.z) * (1.0 - cos(3.141592653589793*rotZ/180.0)))
                       - (viewDir.y * sin(3.141592653589793*rotZ/180.0)))) +
                  (rotatedUp.y *
                   (((viewDir.y * viewDir.z) * (1.0 - cos(3.141592653589793*rotZ/180.0)))
                       + (viewDir.x * sin(3.141592653589793*rotZ/180.0)))));
    VECNormalize(&cameraUp, &cameraUp);
    Hu3DCameraPosSet((1 << 0), cameraPos.x, cameraPos.y, cameraPos.z, cameraUp.x, cameraUp.y,
                     cameraUp.z, cameraTarget.x, cameraTarget.y, cameraTarget.z);
}

// The object manager calls this callback to apply each configured camera every frame.
void fn_8003DAA4(OMOBJ *outViewObj)
{
    u8 cameraIndex;
    for(cameraIndex=0; cameraIndex<outViewObj->work[0]; cameraIndex++) {
        Vec cameraPos;
        Vec cameraTarget;
        Vec cameraUp;
        Vec rotatedUp;
        Vec viewDir;
        float rotX = lbl_80266A58[cameraIndex].x;
        float rotY = lbl_80266A58[cameraIndex].y;
        float rotZ = lbl_80266A58[cameraIndex].z;
        cameraPos.x =
            lbl_80266B18[cameraIndex].x+(lbl_80266BD8[cameraIndex]
                *(sin(3.141592653589793*rotY/180.0)*cos(3.141592653589793*rotX/180.0)));
        cameraPos.y =
            lbl_80266B18[cameraIndex].y
            +(lbl_80266BD8[cameraIndex]*-sin(3.141592653589793*rotX/180.0));
        cameraPos.z =
            lbl_80266B18[cameraIndex].z+(lbl_80266BD8[cameraIndex]
                *(cos(3.141592653589793*rotY/180.0)*cos(3.141592653589793*rotX/180.0)));
        cameraTarget.x = lbl_80266B18[cameraIndex].x;
        cameraTarget.y = lbl_80266B18[cameraIndex].y;
        cameraTarget.z = lbl_80266B18[cameraIndex].z;
        // These values are copied to rotatedUp below; cameraUp is overwritten by the roll
        // rotation that follows.
        cameraUp.x = sin(3.141592653589793*rotY/180.0) * sin(3.141592653589793*rotX/180.0);
        cameraUp.y = cos(3.141592653589793*rotX/180.0);
        cameraUp.z = cos(3.141592653589793*rotY/180.0)*sin(3.141592653589793*rotX/180.0);
        rotatedUp.x = sin(3.141592653589793*rotY/180.0)*sin(3.141592653589793*rotX/180.0);
        rotatedUp.y = cos(3.141592653589793*rotX/180.0);
        rotatedUp.z = cos(3.141592653589793*rotY/180.0)*sin(3.141592653589793*rotX/180.0);
        VECSubtract(&cameraPos, &cameraTarget, &viewDir);
        VECNormalize(&viewDir, &viewDir);
        // Apply Rodrigues' rotation to rotate the elevation-derived up vector around the normalized
        // view direction by rotZ; the result is normalized immediately afterward.
        cameraUp.x =
            (rotatedUp.x *
             ((viewDir.x * viewDir.x)
                 + ((1.0f - (viewDir.x * viewDir.x)) * cos(3.141592653589793*rotZ/180.0)))) +
            (rotatedUp.y *
             (((viewDir.x * viewDir.y) * (1.0 - cos(3.141592653589793*rotZ/180.0)))
                 - (viewDir.z * sin(3.141592653589793*rotZ/180.0)))) +
            (rotatedUp.z *
             (((viewDir.x * viewDir.z) * (1.0 - cos(3.141592653589793*rotZ/180.0)))
                 + (viewDir.y * sin(3.141592653589793*rotZ/180.0))));
        cameraUp.y =
            (rotatedUp.y *
             ((viewDir.y * viewDir.y)
                 + ((1.0f - (viewDir.y * viewDir.y)) * cos(3.141592653589793*rotZ/180.0)))) +
            (rotatedUp.x *
             (((viewDir.x * viewDir.y) * (1.0 - cos(3.141592653589793*rotZ/180.0)))
                 + (viewDir.z * sin(3.141592653589793*rotZ/180.0)))) +
            (rotatedUp.z *
             (((viewDir.y * viewDir.z) * (1.0 - cos(3.141592653589793*rotZ/180.0)))
                 - (viewDir.x * sin(3.141592653589793*rotZ/180.0))));
        cameraUp.z =
            (rotatedUp.z *
             ((viewDir.z * viewDir.z)
                 + ((1.0f - (viewDir.z * viewDir.z)) * cos(3.141592653589793*rotZ/180.0)))) +
            ((rotatedUp.x *
              (((viewDir.x * viewDir.z) * (1.0 - cos(3.141592653589793*rotZ/180.0)))
                  - (viewDir.y * sin(3.141592653589793*rotZ/180.0)))) +
             (rotatedUp.y *
              (((viewDir.y * viewDir.z) * (1.0 - cos(3.141592653589793*rotZ/180.0)))
                  + (viewDir.x * sin(3.141592653589793*rotZ/180.0)))));
        VECNormalize(&cameraUp, &cameraUp);
        Hu3DCameraPosSetV(((1 << 0) << cameraIndex), &cameraPos, &cameraUp, &cameraTarget);
    }
}

#define SYSKEY_PAD_NONE 0xFFFF
#define SYSKEY_ATTR_PAUSEON (1 << 0)
#define SYSKEY_ATTR_PAUSE (1 << 7)
#define SYSKEY_ATTR_UPAUSE (1 << 8)
#define SYSKEY_ATTR_PAUSEKEY (1 << 9)

void fn_8003E12C(OMOBJMAN *objManager)
{
}

// Object-manager initialization calls this to install the per-frame system pause check.
void omSystemKeyCheckSetup(OMOBJMAN *objManager)
{
    OMOBJ *systemKeyObj = fn_8003AB38(objManager, 32731, 0, 0, -1, fn_8003E1A4);
    lbl_802C0260 = systemKeyObj;
    omSetStatBit(systemKeyObj, (1 << 5)|(1 << 7));
    systemKeyObj->work[0] = 0;
    systemKeyObj->work[1] = 0;
    systemKeyObj->work[2] = 0;
}

// Runs from the system-key object each frame to pause or resume game, process, 3D, sprite, and
// audio updates.
void fn_8003E1A4(OMOBJ *systemKeyObj)
{
    if(!omSysPauseEnableFlag) {
        return;
    }
    if(systemKeyObj->work[0] & SYSKEY_ATTR_PAUSEON) {
        u32 padNo = systemKeyObj->work[1];
        if(padNo != SYSKEY_PAD_NONE) {
            if(omPadErrChk(padNo) == PAD_ERR_NONE && (lbl_802C0008[padNo] & PAD_BUTTON_START)) {
                systemKeyObj->work[0] |= SYSKEY_ATTR_PAUSEKEY;
            }
        }
        if(systemKeyObj->work[0] & SYSKEY_ATTR_PAUSEKEY) {
            if(MgNoGet(omcurovl) != -1) {
                // A pause-menu minigame handles Start as a menu-close request.
                GameMesPauseCancel();
            } else {
                systemKeyObj->work[0] |= SYSKEY_ATTR_UPAUSE;
            }
        }
        if(systemKeyObj->work[0] & SYSKEY_ATTR_UPAUSE) {
            systemKeyObj->work[0] &= ~(SYSKEY_ATTR_UPAUSE|SYSKEY_ATTR_PAUSEKEY|SYSKEY_ATTR_PAUSEON);
            omAllPause(FALSE);
            HuPrcAllPause(FALSE);
            Hu3DPauseSet(FALSE);
            HuSprPauseSet(FALSE);
            HuAudFXPauseAll(FALSE);
            HuAudSeqPauseAll(FALSE);
            HuAudSStreamPauseAll(FALSE);
        }
    } else {
        s16 pauseRequested = FALSE;
        s32 padNo;
        // Suppress pause input during wipes, when no overlay is active, or while exit is pending;
        // Start polling is also skipped during wipe-in.
        if(WipeCheck() || fn_8003B774() == -1 || omSysExitReq) {
            return;
        }
        if(!WipeCheckIn()) {
            for(padNo=0; padNo<4; padNo++) {
                if(!omPadErrChk(padNo) && (lbl_802C0008[padNo] & PAD_BUTTON_START)) {
                    pauseRequested = TRUE;
                    break;
                }
            }
        }
        if(systemKeyObj->work[0] & SYSKEY_ATTR_PAUSE) {
            systemKeyObj->work[0] &= ~SYSKEY_ATTR_PAUSE;
            pauseRequested = TRUE;
            // Scripted pauses have no controller whose Start press can resume them.
            padNo = SYSKEY_PAD_NONE;
        }
        if(pauseRequested) {
            systemKeyObj->work[0] |= SYSKEY_ATTR_PAUSEON;
            systemKeyObj->work[1] = padNo;
            omAllPause(TRUE);
            HuPrcAllPause(TRUE);
            Hu3DPauseSet(TRUE);
            HuSprPauseSet(TRUE);
            HuAudFXPauseAll(TRUE);
            HuAudSeqPauseAll(TRUE);
            HuAudSStreamPauseAll(TRUE);
            if(MgNoGet(omcurovl) != -1) {
                GameMesPauseCreate();
            }
            fn_80006F3C();
        }
    }
}

// Global system setup enables or disables handling of the Start button and scripted pause requests.
void omSysPauseEnable(u8 enableFlag)
{
    omSysPauseEnableFlag = enableFlag;
}

// Queues a pause/resume request when the system-key object exists; the callback applies it on a
// later frame when system pause handling is enabled.
void fn_8003E3F0(s16 pauseFlag)
{
    if(!lbl_802C0260) {
        return;
    }
    lbl_802C0260->work[0] &= ~(SYSKEY_ATTR_PAUSE|SYSKEY_ATTR_UPAUSE);
    if(pauseFlag) {
        lbl_802C0260->work[0] |= SYSKEY_ATTR_PAUSE;
    } else {
        lbl_802C0260->work[0] |= SYSKEY_ATTR_UPAUSE;
    }
}

#define CAMERA_ATTR_NONE 0
#define CAMERA_ATTR_SINGLE (1 << 0)

typedef struct CameraViewWork_s {
    s16 cameraNo; // Camera index marked as transitioning; initialized to -1 and cleared when
                  // CameraMoveProc finishes. Immediate setters kill the process but leave this
                  // marker set.
    s16 attributes; // CAMERA_ATTR_SINGLE selects the single-camera globals instead of an indexed
                    // camera.
    s16 interpolationType; // Easing mode used to calculate each frame's interpolation weight.
    HuVecF centerTarget; // Target look-at point in world units.
    HuVecF rotationTarget; // Target camera rotation in degrees.
    float zoomTarget; // Target camera distance from the look-at point.
    u32 durationFrames; // Number of video frames in the transition.
    HUPROCESS *process; // Child process that updates this camera until the transition ends.
} CAMERA_VIEW_WORK;

static CAMERA_VIEW_WORK cameraViewWork[16];

static void CameraMoveProc(void);

// omMain calls this during object-manager creation to mark every camera transition slot idle.
void fn_8003E440(void)
{
    s16 cameraIndex;
    for(cameraIndex=0; cameraIndex<16; cameraIndex++) {
        cameraViewWork[cameraIndex].cameraNo = -1;
    }
}

// Sets target values immediately for each selected camera and cancels its running transition.
void fn_8003E488(s16 cameraBits, OM_CAMERA_VIEW *view)
{
    s16 cameraIndex;
    s16 cameraBit;
    for (cameraIndex = 0, cameraBit = (1 << 0); cameraIndex < 16;
         cameraIndex++, cameraBit <<= 1) {
        if(cameraBits & cameraBit) {
            lbl_80266A58[cameraIndex] = view->rot;
            lbl_80266B18[cameraIndex] = view->center;
            lbl_80266BD8[cameraIndex] = view->zoom;
            if(cameraViewWork[cameraIndex].cameraNo != -1) {
                HuPrcKill(cameraViewWork[cameraIndex].process);
            }
        }
    }
}

// Sets the single camera immediately and cancels its running transition, if any.
void fn_8003E5A4(OM_CAMERA_VIEW *view)
{
    lbl_80266A40 = view->rot;
    lbl_80266A4C = view->center;
    lbl_802C028C = view->zoom;
    if(cameraViewWork[0].cameraNo != -1) {
        HuPrcKill(cameraViewWork[0].process);
    }
}

// Starts or replaces a child-process transition for each selected camera, and returns the number
// started. OM_CAMERA_SINGLE selects the single-camera globals.
s16 fn_8003E62C(u32 cameraSelection, OM_CAMERA_VIEW *view, s32 durationFrames,
                          s16 interpolationType)
{
    s16 cameraMask;
    s16 cameraBit;
    s16 cameraCount;
    s16 cameraIndex;
    if(cameraSelection == 0x10000) {
        cameraMask = (1 << 0);
    } else {
        cameraMask = cameraSelection;
    }
    for (cameraIndex = cameraCount = 0, cameraBit = (1 << 0); cameraIndex < 16;
         cameraIndex++, cameraBit <<= 1) {
        if(cameraMask & cameraBit) {
            CAMERA_VIEW_WORK *viewWork = &cameraViewWork[cameraIndex];
            HUPROCESS *cameraProcess;
            if(cameraViewWork[cameraIndex].cameraNo != -1) {
                HuPrcKill(viewWork->process);
            }
            cameraProcess = fn_8000DAC0(CameraMoveProc, 100, 4096, 0, fn_8000DD9C());
            cameraProcess->property = (void *)cameraIndex;
            viewWork->process = cameraProcess;
            viewWork->cameraNo = cameraIndex;
            if(cameraSelection == 0x10000) {
                viewWork->attributes = CAMERA_ATTR_SINGLE;
            } else {
                viewWork->attributes = CAMERA_ATTR_NONE;
            }
            viewWork->centerTarget = view->center;
            viewWork->rotationTarget = view->rot;
            viewWork->zoomTarget = view->zoom;
            viewWork->durationFrames = durationFrames;
            viewWork->interpolationType = interpolationType;
            cameraCount++;
        }
    }
    return cameraCount;
}

// Starts a transition on the single camera through the shared multi-camera worker.
s16 fn_8003E7A0(OM_CAMERA_VIEW *view, s32 durationFrames, s16 interpolationType)
{
    return fn_8003E62C(0x10000, view, durationFrames, interpolationType);
}

// Starts the default smooth transition on each selected camera.
s16 fn_8003E8E4(u32 cameraSelection, OM_CAMERA_VIEW *view, s32 durationFrames)
{
    return fn_8003E62C(cameraSelection, view, durationFrames, 0);
}

// Starts the default smooth transition on the single camera.
s16 fn_8003EA54(OM_CAMERA_VIEW *view, s32 durationFrames)
{
    return fn_8003E7A0(view, durationFrames, 0);
}

// Returns FALSE if any selected camera slot is marked as transitioning (cameraNo != -1); the
// marker may remain set after an immediate setter cancels its process.
BOOL fn_8003EB94(u32 cameraBits)
{
    s16 cameraIndex;
    s16 cameraBit;
    s16 movingCameraCount;
    for (cameraIndex = movingCameraCount = 0, cameraBit = (1 << 0); cameraIndex < 16;
         cameraIndex++, cameraBit <<= 1) {
        if(cameraBits & cameraBit) {
            if(cameraViewWork[cameraIndex].cameraNo != -1) {
                movingCameraCount++;
            }
        }
    }
    if(movingCameraCount != 0) {
        return FALSE;
    } else {
        return TRUE;
    }
}

// Child process created by omCameraViewMoveMulti; interpolates one camera once per video frame.
static void CameraMoveProc(void)
{
    HUPROCESS *cameraProcess = fn_8000DD9C();
    s16 cameraIndex = (s16)cameraProcess->property;
    CAMERA_VIEW_WORK *viewWork = &cameraViewWork[cameraIndex];
    u32 frame;
    float angle;
    float weight;
    float zoomStart;
    float zoomFrame;
    Vec centerStart;
    Vec rotationStart;
    Vec centerFrame;
    Vec rotationFrame;
    
    Vec rotationDelta;
    
    if(viewWork->attributes & CAMERA_ATTR_SINGLE) {
        rotationStart = lbl_80266A40;
        centerStart = lbl_80266A4C;
        zoomStart = lbl_802C028C;
    } else {
        rotationStart = lbl_80266A58[cameraIndex];
        centerStart = lbl_80266B18[cameraIndex];
        zoomStart = lbl_80266BD8[cameraIndex];
    }
    // Wrap each rotation delta so the camera takes the shorter path around the circle.
    angle = viewWork->rotationTarget.x-rotationStart.x;
    if(((angle < 0) ? -angle : angle) > 180.0f) {
        if(viewWork->rotationTarget.x < rotationStart.x) {
            rotationDelta.x = 360+angle;
        } else {
            rotationDelta.x = angle-360;
        }
    } else {
        rotationDelta.x = angle;
    }
    angle = viewWork->rotationTarget.y-rotationStart.y;
    if(((angle < 0) ? -angle : angle) > 180.0f) {
        if(viewWork->rotationTarget.y < rotationStart.y) {
            rotationDelta.y = 360+angle;
        } else {
            rotationDelta.y = angle-360;
        }
    } else {
        rotationDelta.y = angle;
    }
    angle = viewWork->rotationTarget.z-rotationStart.z;
    if(((angle < 0) ? -angle : angle) > 180.0f) {
        if(viewWork->rotationTarget.z < rotationStart.z) {
            rotationDelta.z = 360+angle;
        } else {
            rotationDelta.z = angle-360;
        }
    } else {
        rotationDelta.z = angle;
    }
    for(frame=1; frame<=viewWork->durationFrames; frame++) {
        // SIMPLE uses a sine-of-sine curve, COS uses a cosine curve, and LINEAR (also the
        // COSSIN alias) uses a linear weight. Other values use a two-half curve: cosine easing
        // for the first half and sine-of-sine easing for the second half.
        if(viewWork->interpolationType == 0) {
            angle =
                sin(3.141592653589793*(90.0f*((float)frame/(float)viewWork->durationFrames))/180.0);
            weight = sin(3.141592653589793*(90.0f*angle)/180.0);
        } else if(viewWork->interpolationType == 1) {
            weight =
                1-cos(3.141592653589793*(90.0f*((float)frame/(float)viewWork->durationFrames))
                /180.0);
        } else if(viewWork->interpolationType == 2) {
            weight = (float)frame/(float)viewWork->durationFrames;
        } else if(viewWork->durationFrames/2 > frame) {
            angle = (float)frame/(float)(viewWork->durationFrames/2.0);
            weight = 0.5*(1-cos(3.141592653589793*(90.0f*angle)/180.0));
        } else {
            angle = (float) (frame - (viewWork->durationFrames / 2)) /
                    (float) (viewWork->durationFrames / 2.0);
            weight =
                0.5+(0.5*sin(3.141592653589793*(90.0*sin(3.141592653589793*(90.0f*angle)/180.0))
                /180.0));
        }
        zoomFrame = zoomStart+(weight*(viewWork->zoomTarget-zoomStart));
        rotationFrame.x = rotationStart.x+(weight*rotationDelta.x);
        rotationFrame.y = rotationStart.y+(weight*rotationDelta.y);
        rotationFrame.z = rotationStart.z+(weight*rotationDelta.z);
        centerFrame.x = centerStart.x+(weight*(viewWork->centerTarget.x-centerStart.x));
        centerFrame.y = centerStart.y+(weight*(viewWork->centerTarget.y-centerStart.y));
        centerFrame.z = centerStart.z+(weight*(viewWork->centerTarget.z-centerStart.z));
        if(viewWork->attributes & CAMERA_ATTR_SINGLE) {
            lbl_802C028C = zoomFrame;
            lbl_80266A40 = rotationFrame;
            lbl_80266A4C = centerFrame;
        } else {
            lbl_80266BD8[cameraIndex] = zoomFrame;
            lbl_80266A58[cameraIndex] = rotationFrame;
            lbl_80266B18[cameraIndex] = centerFrame;
        }
        HuPrcVSleep();
    }
    viewWork->cameraNo = -1;
    HuPrcEnd();
}
