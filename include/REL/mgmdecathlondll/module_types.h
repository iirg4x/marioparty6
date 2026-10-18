/* Shared Decathlon Park state and data types used by its mode and scenery code. */
#ifndef REL_MGMDECATHLONDLL_MODULE_TYPES_H
#define REL_MGMDECATHLONDLL_MODULE_TYPES_H

#define DECATHLON_RESULT_TEXT_CAPACITY 20

#include <math.h>
#include "game/window.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/mgdata.h"
#include "game/memory.h"
#include "game/audio.h"

/* Text and display state used while building each player's result message. */
typedef struct MgmDecathlonPlayerRecord_s {
    u8 messageBytes[DECATHLON_RESULT_TEXT_CAPACITY]; /* inline message bytes, including control
                                                      * codes */
    s8 enabled; /* status byte cleared when result text is built or retrieved */
} MgmDecathlonPlayerRecord;

typedef struct DecathlonPlayerActorData_s {
    s16 playerNo; /* player slot, 0 through 3 */
    s16 characterNo; /* selected character */
    s16 padNo; /* controller slot */
    s16 groupNo; /* player grouping used by the minigame */
    s32 playerType; /* human or CPU player type */
    s16 cpuDifficulty; /* CPU difficulty setting */
    s16 state; /* actor update state */
    s16 currentMotion; /* selected motion-setting index; -1 before initialization */
    u8 reservedBytes[2]; /* storage unused by the player actor code */
    Vec position; /* world position */
    Vec rotation; /* rotation in degrees */
    f32 modelHeight; /* model height in world units */
    Vec destination; /* world position the actor turns and walks toward */
} DecathlonPlayerActorData;

typedef struct DecathlonRankPosition {
    Vec position; /* world position for a result rank */
} DecathlonRankPosition;

typedef struct DecathlonModeData {
    s16 modeId; /* zero-based minigame index; add 601 for the public number */
    MGDATA *data; /* minigame metadata and message IDs */
} DecathlonModeData;

typedef union DecathlonPadMaskTable {
    u16 unsignedValues[4]; /* one input bit mask per controller slot */
    s16 signedValues[4]; /* signed view used by the input helpers */
} DecathlonPadMaskTable;

typedef struct DecathlonMessageIds {
    u32 messageIds[10]; /* event-number message IDs for one through ten */
} DecathlonMessageIds;

typedef struct DecathlonScoreMessages {
    u32 messageIds[4]; /* result message selected by the number of leaders */
} DecathlonScoreMessages;

typedef struct DecathlonWindowState {
    HUWINID windowId; /* shared Decathlon result window */
    f32 width; /* window width in screen units */
    f32 height; /* window height in screen units */
} DecathlonWindowState;

typedef struct DecathlonMotionSetting {
    s16 motionIndex; /* index in the actor's motion array; -1 keeps the current motion */
    f32 duration; /* blend duration in frames; at most one selects the motion immediately */
    u32 modelAttributes; /* motion playback flags applied when selected */
    s32 voiceFlag; /* character voice setting passed to CharModelVoiceFlagSet */
} DecathlonMotionSetting;

typedef struct DecathlonMessageSound {
    u32 messageId; /* archive message that can trigger this cue */
    s16 characterCode; /* window callback code selecting the cue within the message */
    s32 soundEffectId; /* sound effect ID; a negative value suppresses playback */
} DecathlonMessageSound;

extern s16 lbl_1_bss_0;
extern s16 lbl_1_bss_2[4];
extern s16 lbl_1_bss_A[4];
extern s32 lbl_1_data_C70;
extern s32 lbl_1_data_C74;
extern s32 lbl_1_data_C94;
extern s16 lbl_1_data_2A8;
extern OMOBJ *lbl_1_bss_88;
extern OMOBJ *lbl_1_bss_78[4];
extern s32 lbl_1_data_C98[];
extern OMOBJ *lbl_1_bss_B8;
extern s32 lbl_1_data_5D8;
extern s32 lbl_1_data_60C[4];
extern s32 lbl_1_data_64C[14];
extern s32 lbl_1_data_6B4[10];
extern DecathlonMotionSetting lbl_1_data_4F8[9];
extern s32 lbl_1_data_C78[5];
extern HuVecF lbl_1_data_61C[];
extern f32 lbl_1_data_6DC[5];
extern f32 lbl_1_data_6F0[4];
extern f32 lbl_1_data_700;
extern f32 lbl_1_data_704[4];
extern f32 lbl_1_data_714[4];
extern f32 lbl_1_data_724;
extern HuVecF lbl_1_data_5DC[];
extern s16 lbl_1_bss_94;
extern s16 lbl_1_bss_96;
extern s16 lbl_1_bss_98;
extern u32 lbl_1_data_C8C;
extern s32 lbl_1_data_30[8];
extern u32 lbl_1_data_88[10];
extern u32 lbl_1_data_50[14];
extern DecathlonMessageSound lbl_1_data_B0[42];

extern DecathlonWindowState lbl_1_bss_18;
extern MgmDecathlonPlayerRecord lbl_1_bss_24[4];
extern OMOBJ *lbl_1_bss_8C;
extern OMOBJ *lbl_1_bss_90;
extern OMOBJ *lbl_1_bss_9C;
extern OMOBJ *lbl_1_bss_A0;
extern OMOBJ *lbl_1_bss_A8[4];
extern OMOBJMAN *lbl_1_bss_C0;
extern s32 lbl_1_data_C90;
extern HuVecF lbl_1_data_358;
extern char lbl_1_data_364[16];
extern void fn_1_6E80(OMOBJ *obj);
extern s16 lbl_1_bss_BC;
extern DecathlonRankPosition lbl_1_data_684[4];
extern DecathlonModeData lbl_1_data_CD0[10];

#endif /* REL_MGMDECATHLONDLL_MODULE_TYPES_H */
