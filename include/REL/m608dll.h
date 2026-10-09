/* Shared declarations for Snow Whirled course sequences and display state. */
#ifndef M608DLL_H
#define M608DLL_H
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
#include "datadir_enum.h"
#include "string.h"
#ifndef M608_SCENE_CONSUMED_CONTEXT_H
#define M608_SCENE_CONSUMED_CONTEXT_H

/* Stores the sampled course path; the remaining bytes are not read by this module. */
typedef struct M608PathSamples { Point3d points[660]; u8 unusedBytes[7920]; } M608PathSamples;
typedef struct M608SceneConsumedContext {
    HUPROCESS *objectManager; /* Manager for course and player processes. */
    u32 state; /* Active course-sequence state. */
    s32 frame; /* Frames elapsed in the active sequence state. */
    s32 activePlayer; /* Current rider index, 0 through 3. */

    HU3D_MOTIONID cameraMotions[10]; /* Loaded camera motions. */
    HU3D_MODELID cameraModels[10]; /* Camera models that play the motions. */
    HU3D_MODELID activeCameraMotion; /* Camera model currently driving the view. */
    /* Bytes between the camera model and process pointer; not read here. */
    u8 unusedCameraBytes[2];
    OMOBJ *cameraObject; /* Process that updates camera playback and position. */
    f32 cameraTime; /* Elapsed camera-motion time in frames. */
    f32 cameraMaxTime; /* Duration of the active camera motion in frames. */
    /* Bytes before the scene model list; not read here. */
    u8 unusedSceneBytes[2];

    HU3D_MODELID sceneModels[5]; /* Background models selected for day or night. */
    HU3D_MODELID playerModelsA[4]; /* Primary rider models used during the course. */
    HU3D_MODELID playerModelsB[4]; /* Rider models used to read attachment hooks. */
    HU3D_MODELID playerModelsC[4]; /* Alternate rider models used for attachments. */
    HU3D_MODELID mainModel; /* Main moving course model. */
    HU3D_MOTIONID mainMotion; /* Motion assigned to the main course model. */
    /* Hook positions read from player model variant B. */
    Point3d playerBoardHookPositions[4];
    /* Bytes between model-hook position arrays; not read here. */
    u8 unusedPlayerModelBytes[48];
    /* Attachment positions read from player model variants B and C. */
    Point3d playerAttachmentPositions[4];

    s16 characterNos[4]; /* Character number assigned to each rider. */
    s16 padNos[4]; /* Controller port assigned to each rider. */
    HU3D_MODELID characterModels[4]; /* Loaded character models. */
    HU3D_MOTIONID characterMotions[4][8]; /* Motion IDs loaded for each character. */
    OMOBJ *playerObjects[4]; /* Process objects that update rider rotation. */
    HU3D_MODELID secondaryModels[4]; /* Board models attached to each character. */
    /* Course props used by scene setup, indexed by their setup code. */
    HU3D_MODELID courseModels[16];
    /* Joint motions used by the course props. */
    HU3D_MOTIONID courseMotions[9];
    /* Bytes before the two timed course models; not read here. */
    u8 unusedTimingModelBytes[6];
    /* Models whose motion callbacks advance the run. */
    HU3D_MODELID timingModels[2];
    s32 timingSubstate; /* Turn-animation step advanced by a motion callback. */

    /* Current, starting, and target rider yaw, in degrees. */
    f32 riderTurnAngle;
    f32 riderTurnStartAngle;
    f32 riderTurnTargetAngle;
    /* Frames elapsed by the active rider-turn callback. */
    s32 riderTurnFrame;
    /* Models, motions, sampled positions, and updater processes for course props. */
    HU3D_MODELID movingCourseModels[27];
    HU3D_MOTIONID movingCourseMotions[27];
    Point3d movingCoursePositions[27];
    OMOBJ *movingCourseObjects[27];

    /* Number of recorded course path samples. */
    s32 coursePathSampleCount;
    s32 scores[4]; /* Direction-match score for each rider. */
    u32 record; /* Course record value; saved points are divided by 90 for comparisons, and new
                 * match counts are multiplied by 90 for display and saving. */
    s32 winners[4]; /* Whether each rider tied for the highest score: 1 yes, 0 no. */
    /* Set when the results sequence detects a new record. */
    s32 recordBeaten;
    s16 difficulty[4]; /* Computer-player difficulty for each rider. */
    s16 base[4]; /* Base rate used to choose computer button timing. */
    s16 interval[4]; /* Frame threshold for computer direction presses; a 1% check can trigger
                      * earlier. */
    s16 aiRollFrameCounter[4]; /* Frames since each computer rider last pressed. */
    /* Shadow center, target point, and up direction in world units. */
    Point3d shadowCenter;
    Point3d shadowTarget;
    Point3d shadowUpDirection;
    /* Randomly selected rider index used by the special course effect. */
    s32 featuredRiderIndex;
    /* Handles for the jump effect and streamed course music. */
    s32 jumpSoundHandle;
    s32 courseMusicStreamHandle;
} M608SceneConsumedContext;

#endif

/* Updates the course shadow position from the current camera-hook position. */
void fn_1_5DA4(Point3d position);
BOOL fn_1_5F64(void);
void fn_1_5F90(s16 coursePropIndex);
void fn_1_5FDC(s16 coursePropIndex, s16 courseMotionIndex);
void fn_1_6534(s16 scoreIndex, s16 scoreValue);
void fn_1_6958(s32 spriteMemberIndex);
void fn_1_69F4(void);
u32 MgSeqModeNext(void);

struct _struct_lbl_1_bss_3DF0_0x18 {
    s16 scoreBoxId; /* Score-box resource ID. */
    MGSCORE *scoreDisplay; /* Score digits and formatting state. */
    ANIMDATA *recordSpriteAnimation; /* Record emblem animation, used by panel 4. */
    s16 recordSpriteGroup; /* Group containing the record emblem. */
    s16 recordSpriteId; /* Record emblem member within its sprite group. */
    u8 unusedBytes[8]; /* Bytes not read by this module. */
};

/* Position, dimensions, and colors used to build a score panel. */
struct _struct_lbl_1_data_44C_0x14 {
    s16 screenX; /* Panel horizontal position, in screen pixels. */
    s16 screenY; /* Panel vertical position, in screen pixels. */
    s16 boxWidth; /* Score-box width, in screen pixels. */
    s16 boxHeight; /* Score-box height, in screen pixels. */
    u8 boxRed; /* Score-box red color channel. */
    u8 boxGreen; /* Score-box green color channel. */
    u8 boxBlue; /* Score-box blue color channel. */
    u8 boxUnusedByte; /* Stored but not passed to the score-box color setter. */
    u8 unusedColorBytes[4]; /* Stored between box and digit colors; not read here. */
    u8 digitRed; /* Score-digit red color channel. */
    u8 digitGreen; /* Score-digit green color channel. */
    u8 digitBlue; /* Score-digit blue color channel. */
    u8 digitUnusedByte; /* Stored but not passed to the digit color setter. */
};

/* Camera-motion preset data; only randomizedHeight is read or changed here. */
typedef struct M608Unknown80 {
    u32 motionPresetWords[2]; /* Motion preset values not interpreted by this code. */
    f32 randomizedHeight; /* Randomized vertical camera offset, in world units. */
    u32 motionPresetData[17]; /* Remaining preset values not interpreted here. */
} M608Unknown80;
s32 fn_1_A0(s32 currentTrackHandle, s32 fallbackBgmId);
void fn_1_104(s32 streamHandle);
void fn_1_140(s16 sequenceMode, s16 frameNo);
void fn_1_190(s16 sequenceMode, s16 frameNo);
void fn_1_57C(s16 sequenceMode, s16 frameNo);
void fn_1_580(s16 sequenceMode, s16 frameNo);
void fn_1_1A2C(s16 sequenceMode, s16 frameNo);
void fn_1_1DD8(s16 sequenceMode, s16 frameNo);
void fn_1_2374(s16 sequenceMode, s16 frameNo);
void fn_1_2538(s16 sequenceMode, s16 frameNo);
void fn_1_253C(s16 sequenceMode, s16 frameNo);
void fn_1_2540(OMOBJ *cameraObject);
void fn_1_2FD8(s16 timingModelId, s16 motionId, BOOL timingCallbackF);
void fn_1_33A4(OMOBJ *playerObject);
void fn_1_34D0(OMOBJ *playerObject);
void fn_1_3718(OMOBJ *courseObject);
void fn_1_388C(OMOBJ *courseObject);
void fn_1_38E8(HU3D_MODEL *cameraModel, f32 (*cameraMatrix)[3][4]);
void fn_1_38EC(Point3d position);
void fn_1_38F0(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4]);
void fn_1_39B4(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4]);
void fn_1_3A78(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4]);
void fn_1_3B3C(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4]);
void fn_1_3C00(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4]);
void fn_1_3C84(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4]);
void fn_1_3D7C(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4]);
void fn_1_3E00(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4]);
void fn_1_3F18(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4]);
void fn_1_400C(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4]);
void fn_1_40F0(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4]);
void fn_1_41D4(HSF_OBJECT *hookObject, HSF_TRANSFORM *transform, f32 (*previousMatrix)[3][4],
               f32 (*currentMatrix)[3][4]);
void fn_1_42B8(void);
void fn_1_437C(s16 layerNo);
void fn_1_43C8(void);
void fn_1_5DA4(Point3d position);
void fn_1_5E6C(s32 cameraIndex);
BOOL fn_1_5F64(void);
void fn_1_5F90(s16 coursePropIndex);
void fn_1_5FDC(s16 coursePropIndex, s16 courseMotionIndex);
void fn_1_605C(s16 coursePropIndex);
void fn_1_60CC(void);
void fn_1_6148(void);
void fn_1_614C(void);
void fn_1_6150(void);
void fn_1_6534(s16 scoreIndex, s16 scoreValue);
void fn_1_6578(void);
void fn_1_6604(void);
void fn_1_66C0(OMOBJ *recordObject);
void fn_1_6828(void);
void fn_1_6958(s32 spriteMemberIndex);
void fn_1_69F4(void);
u32 fn_1_6B90(s32 playerIndex);
extern /* button mask */
MGSEQ_PARAM lbl_1_data_0;
extern f32 lbl_1_data_28[14];
extern f32 lbl_1_data_60[14];
extern f32 lbl_1_data_98[14];
extern char lbl_1_data_D0[9];
extern char lbl_1_data_D9[16];
extern char lbl_1_data_E9[16];
extern char lbl_1_data_F9[16];
extern char lbl_1_data_109[16];
extern char lbl_1_data_119[14];
extern char lbl_1_data_127[5];
extern char lbl_1_data_12C[22];
extern char lbl_1_data_142[5];
extern char lbl_1_data_147[26];
extern char lbl_1_data_161[24];
extern char lbl_1_data_179[16];
extern char lbl_1_data_189[7];
extern char lbl_1_data_1B4[29];
extern char lbl_1_data_1D1[5];
extern char lbl_1_data_1D6[65];
extern char lbl_1_data_217[16];
extern char lbl_1_data_227[14];
extern char lbl_1_data_235[13];
extern char lbl_1_data_242[12];
extern char lbl_1_data_24E[18];
extern char lbl_1_data_295[11];
extern char lbl_1_data_2A0[16];
extern char lbl_1_data_2B0[5];
extern char lbl_1_data_2B5[5];
extern char lbl_1_data_2BA[11];
extern char lbl_1_data_2C5[11];
extern char lbl_1_data_2D0[11];
extern char lbl_1_data_2DB[29];
extern char lbl_1_data_2F8[40];
extern M608Unknown80 lbl_1_data_320;
extern M608Unknown80 lbl_1_data_370;
extern M608Unknown80 lbl_1_data_3C0;
extern s32 lbl_1_data_410[15];
extern struct _struct_lbl_1_data_44C_0x14 lbl_1_data_44C[5];
extern s32 lbl_1_data_4B0[6];
extern char lbl_1_data_4C8[40];
extern s32 lbl_1_data_4F0[10];
extern s32 lbl_1_data_518[4];
extern s32 lbl_1_data_528[4];
extern s32 lbl_1_data_538[4];
extern unsigned int lbl_1_data_548[9];
extern u32 lbl_1_data_56C[8];
extern char lbl_1_data_58C[17];
extern char lbl_1_data_59D[17];
extern char lbl_1_data_5AE[17];
extern char lbl_1_data_5BF[17];
extern char *lbl_1_data_5D0[4];
extern char lbl_1_data_5E0[17];
extern char lbl_1_data_5F1[17];
extern char lbl_1_data_602[17];
extern char lbl_1_data_613[17];
extern char *lbl_1_data_624[4];
extern char lbl_1_data_634[6];
extern char lbl_1_data_63A[6];
extern char lbl_1_data_640[6];
extern char lbl_1_data_646[6];
extern char *lbl_1_data_64C[4];
extern char lbl_1_data_65C[10];
extern char lbl_1_data_666[10];
extern char lbl_1_data_670[10];
extern char lbl_1_data_67A[10];
extern char *lbl_1_data_684[4];
extern s32 lbl_1_data_694[27];
extern struct _struct_lbl_1_bss_3DF0_0x18 lbl_1_bss_3DF0[5];
typedef struct M608SpriteGroupWork {
    s16 currentMember; /* Visible prompt member, or -1 when all are hidden. */
    HUSPR_GROUPID group; /* Group holding the course prompt sprites. */
    ANIMDATA *anim[5]; /* Animation resource for each prompt member. */
} M608SpriteGroupWork;
extern M608SpriteGroupWork lbl_1_bss_3E68;
extern M608SceneConsumedContext lbl_1_bss_3E80;
extern const Point3d lbl_1_rodata_88;
extern const Point3d lbl_1_rodata_94;
extern const Point3d lbl_1_rodata_A0;
extern const GXColor lbl_1_rodata_AC;
extern const Point3d lbl_1_rodata_B0;
extern const Point3d lbl_1_rodata_BC;
extern const Point3d lbl_1_rodata_C8;
extern const GXColor lbl_1_rodata_D4;

typedef char m608_scene_extent[(sizeof(M608SceneConsumedContext)==1112)?1:-1];
typedef char m608_path_extent[(sizeof(M608PathSamples)==15840)?1:-1];
typedef char m608_sprite_extent[(sizeof(M608SpriteGroupWork)==24)?1:-1];
typedef char m608_score_extent[(sizeof(struct _struct_lbl_1_bss_3DF0_0x18)==24)?1:-1];
#endif
