/* Shared Battle Bridge scene, preview, window and stage types. */
#ifndef REL_MGMBATTLEDLL_MODULE_TYPES_H
#define REL_MGMBATTLEDLL_MODULE_TYPES_H

#include "dolphin/gx.h"
#include "game/main.h"
#include "game/mgdata.h"
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
#include "REL/mgmbattledll/include/game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

#include "REL/mgmbattledll/player.h"

/* Character motions available to Battle Bridge sequences. */
typedef struct MgBattleMotion_s {
    /* Index into the player's loaded character-motion array; -1 skips motion playback. */
    s16 motionIndex;
    /* Blend duration in frames; values at or below one switch immediately. */
    f32 transitionTime;
    /* Character-model attributes applied while the motion plays. */
    u32 modelAttr;
    /* Voice playback flags used when the motion is selected. */
    s32 voiceFlag;
} MgBattleMotion;

extern MgBattleMotion lbl_1_data_4F8[14];
extern Point3d lbl_1_data_428[3][4];

/* Battle sequence state and the camera's transition snapshot. */
extern s16 lbl_1_bss_668;
extern s16 lbl_1_bss_66A;
extern f32 lbl_1_bss_66C;
extern s16 lbl_1_bss_670;
extern s16 lbl_1_bss_672;
extern HUPROCESS *lbl_1_bss_674;
extern OMOBJ *lbl_1_bss_678;
extern Point3d lbl_1_data_938;
extern Point3d lbl_1_data_944;

/* Music, one-shot sounds, and per-player sound emitters. */
extern s32 lbl_1_data_8D8;
extern s32 lbl_1_data_8DC;
extern s32 lbl_1_data_8E0;
extern s32 lbl_1_data_8E4;
extern s32 lbl_1_data_8E8;
extern s32 lbl_1_data_8EC[4];
extern s32 lbl_1_data_8FC[4];

/* Results panel and player window positions for the three game counts. */
extern Point3d lbl_1_data_6D8;
extern Point3d lbl_1_data_6E4[3][4];

/* Preview images and movie resources. */
extern HU3D_MODELID lbl_1_bss_A4;
extern HUSPR_GROUPID lbl_1_bss_A2;
extern s16 lbl_1_bss_A0;
extern ANIMDATA *lbl_1_bss_9C;
extern ANIMDATA *lbl_1_bss_98;
extern void *lbl_1_bss_94;
extern MGDATA *lbl_1_bss_90;
extern ANIMDATA **lbl_1_bss_8C;
extern s16 lbl_1_bss_88;
extern ANIMDATA *lbl_1_bss_84;
extern u8 lbl_1_bss_2[128];
extern s16 lbl_1_bss_0;

/* Window handles and their pixel dimensions. */
typedef struct MgBattleWindow_s {
    /* Window handle returned by the message-window system. */
    s16 windowId;
    /* Window width in screen pixels. */
    f32 width;
    /* Window height in screen pixels. */
    f32 height;
} MgBattleWindow;
extern MgBattleWindow lbl_1_bss_A8[4];
extern MgBattleWindow lbl_1_bss_D8[3];
extern u32 lbl_1_data_0[14];

typedef struct MgBattleMessageSound_s {
    /* Message bank and message index for this dialogue line. */
    u32 messageId;
    /* Character code of the speaker associated with the line. */
    s16 characterCode;
    /* Sound effect played for the line, or -1 when silent. */
    s32 soundEffectId;
} MgBattleMessageSound;
extern MgBattleMessageSound lbl_1_data_38[38];
extern s16 lbl_1_data_200;

/* Stage objects, model resources, and their starting positions. */
extern OMOBJ *lbl_1_bss_3DC;
extern HU3D_MODELID lbl_1_bss_3E0;
extern s16 lbl_1_bss_3D8;
extern s32 lbl_1_data_208[7];
extern Point3d lbl_1_data_224[7];

#include "REL/mgmbattledll/effects.h"

#endif
