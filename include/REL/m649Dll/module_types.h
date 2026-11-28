/* Contains the module's shared type declarations. */
#ifndef REL_M649DLL_MODULE_TYPES_H
#define REL_M649DLL_MODULE_TYPES_H
#include "REL/minigame_camera_work.h"
#include "REL/minigame_prop_params.h"
#include "REL/minigame_prop_motion.h"

#include "REL/minigame_light.h"
#include "REL/minigame_positional_audio.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/window.h"
#include "game/printfunc.h"
#include "string.h"
#include "stdio.h"

/* Stores per-object context and the optional frame callback used by the sound manager. */
typedef void (*M620ObjectCallback)(OMOBJ *obj);

typedef struct {
    void *userData; /* Client-owned data retained by the sound manager object. */
    M620ObjectCallback callback; /* Optional per-frame callback after sound updates. */
} M620ObjectState;

/* Character height and motion selections copied into every contestant's state at setup. */
typedef struct {
    float unk0; /* The game's code never reads this field. */
    float height; /* Replaced at setup with the selected character's model height. */
    /* Copied into player state at setup; no player update uses these bytes. */
    u8 unk8[4]; /* No code names this field. */
    /* Idle and movement motions used while turning toward the front after play. */
    s32 finishIdleMotion, moveMotion;
    /* Copied into player state at setup; no player update uses these bytes. */
    u8 unk14[12]; /* No code names this field. */
    s32 idleMotion; /* Motion selected while waiting for a turn. */
    /* Copied into player state at setup; no player update uses these bytes. */
    u8 unk24[16]; /* No code names this field. */
    s32 turnMotion, winMotion, loseMotion; /* Turn and results motion selections. */
} M649PlayerConfig;

/* A lane marker stores its world position and whether it has already scored a hit. */
typedef struct {
    Vec position;
    s16 active, hit;
} M649Point;

typedef struct {
    s32 id, timer; /* Prop identity and its startup delay in frames. */
    float position, speed, targetSpeed; /* Travel distance and current/desired speed. */
    s32 mode, group; /* Marker mode and color group. */
    s32 unk1C; /* The game's code never reads this field. */
    s32 nextPoint;
    M649Point points[64];
} M649Motion;

/* Called to update a camera view. */
typedef void (*M649CameraCallback)(OMOBJ *, u32, s32);

/* Setup and shared game state. */
s32 fn_1_1218(s32 cameraIndex);
s32 fn_1_1210(s32 resource);
void fn_1_1338(s32 player, u32 resultId, u32 character);
void fn_1_1368(s32 player);
BOOL fn_1_1388(void);
s32 fn_1_127C(u32 resultId);
void ObjectSetup(void);
s32 fn_1_1234(void);
s32 fn_1_1244(void);

/* Camera, lighting, audio, and resource support. */
void fn_1_1664(s16 modelId, char *lightName, s32 mode);
void fn_1_13D8(OMOBJMAN *manager);
void fn_1_14A0(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color);
void fn_1_1B88(OMOBJMAN *manager, u32 cameras, float fov, float nearPlane, float farPlane,
    float shadowRadius, OM_CAMERA_VIEW *view);
void fn_1_2BCC(u32 cameras, void *data);
void fn_1_2780(M649CameraCallback callback);
void fn_1_3250(HUPROCESS *objectManager, MinigameAudioRange *audioConfig);
BOOL fn_1_2C84(u32 cameras);
void fn_1_2590(s32 camera, Vec *direction, Vec *target, float fov, float radius);
void fn_1_281C(u32 cameras, float x, float y, float width, float height);
s32 fn_1_48DC(s32 dataNum);
float fn_1_4AF4(float start, float end, float factor);
float fn_1_4C10(float start, float end);
s32 fn_1_49E4(s32 model, s32 dataNum);
float fn_1_4CC4(HuVecF *currentPosition, HuVecF *nextPosition);
float fn_1_4D60(Vec *first, Vec *second);

/* Score and result updates. */
void fn_1_A220(OMOBJMAN *manager);
void fn_1_A600(void);
s32 fn_1_A644(s32 playerIndex);
void fn_1_A2A8(void);
void fn_1_A5A4(s32 playerIndex, s32 awardedPoints);

/* Player setup and updates. */
void fn_1_4F24(HUPROCESS *manager);
void fn_1_5020(void);
s32 fn_1_6958(s32 player);

/* Prop setup and updates. */
void fn_1_6990(HUPROCESS *manager);
void fn_1_6A84(void);
void fn_1_728C(OMOBJ *object);
void fn_1_7288(OMOBJ *object);
void fn_1_7300(OMOBJ *object);
void fn_1_7C94(OMOBJ *obj);
void fn_1_7DD8(OMOBJ *obj);
void fn_1_7FD4(OMOBJ *object);
void fn_1_80B4(OMOBJ *object);
void fn_1_8278(OMOBJ *object);
s32 fn_1_8D20(s32 group, s32 id, s32 frames);

/* Effect setup and particle updates. */
void fn_1_8F58(HUPROCESS *process);
void fn_1_A0BC(M649Motion *motion);
void fn_1_A168(s32 cameraIndex, Vec *position, float *speed, s32 effectType);

#endif
