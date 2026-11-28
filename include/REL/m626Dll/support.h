/* Shared camera, lighting, fog, and positional-audio state. */
#ifndef M626_SUPPORT_H
#define M626_SUPPORT_H
#include "REL/minigame_fog_work.h"
#include "REL/minigame_positional_audio.h"
#include "REL/minigame_camera_fit.h"
#include "REL/m626Dll/setup.h"
#include "game/hu3d.h"

typedef void (*M626CameraCallback)(OMOBJ *, u32, s32);

typedef struct M626CameraEntry_s {
    s32 active;
    s32 unk04; /* The game's code never reads this field. */
    s32 model;
    OM_CAMERA_VIEW view, previousView;
    Vec position, up, target;
    f32 fov, nearPlane, farPlane, aspect;
    f32 viewportX, viewportY, viewportWidth, viewportHeight;
    u8 unk88[32]; /* No code names this field. */
} M626CameraEntry;

typedef struct M626CameraWork_s {
    u32 cameras;
    f32 shadowRadius, shadowFov;
    Vec shadowDirection;
    GXColor shadowColor;
    M626CameraCallback callback;
    M626CameraEntry entries[16];
} M626CameraWork;

typedef struct M626CameraFitSphere_s {
    Vec center;
    f32 radius;
} M626CameraFitSphere;

typedef struct M626AudioWork_s {
    MinigameAudioRange *config;
    OMOBJ_FUNC callback;
    MinigameAudioRecord entries[64];
} M626AudioWork;

extern s32 lbl_1_bss_38[80];
extern s32 lbl_1_bss_178[80];
extern OMOBJ *lbl_1_bss_2B8, *lbl_1_bss_2BC, *lbl_1_bss_2C0, *lbl_1_bss_2C4, *lbl_1_bss_2C8;
extern Vec lbl_1_data_138;
extern GXColor lbl_1_data_144;

void fn_1_A08(OMOBJMAN *manager);
void fn_1_AD0(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color);
void fn_1_C94(s32 modelId, char *lightName, s32 mode);
void fn_1_DD8(HuVecF *direction);
void fn_1_EEC(OMOBJMAN *manager, MinigameFogParams *parameters);
void fn_1_11B8(OMOBJMAN *manager, u32 cameras, f32 fov, f32 nearPlane, f32 farPlane,
               f32 shadowRadius, OM_CAMERA_VIEW *view);
void fn_1_1ECC(HuVecF *direction);
void fn_1_1F4C(u8 r, u8 g, u8 b);
void fn_1_1F84(u32 cameras, void *data);
void fn_1_2608(HUPROCESS *process, MinigameAudioRange *audioConfig);
s32 fn_1_29BC(Vec *object);
s32 fn_1_2BDC(Vec *object);
s32 fn_1_3340(s32 left, Vec *right);
void fn_1_3AF4(int effect, int handle, Vec *position, int flag, int value);
int fn_1_3C94(int dataNumber);
s32 fn_1_3D9C(s32 model, s32 dataNum);
float fn_1_3EAC(float start, float end, float factor);
float fn_1_407C(HuVecF *origin, HuVecF *target);
float fn_1_4118(Vec *first, Vec *second);

#endif
