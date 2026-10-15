/* T Minus Five character selection, result, and board animation routines. */
#include "dolphin/math.h"
#include "game/main.h"
#include "game/object.h"
#include "game/charman.h"
#include "game/hu3d.h"
#include "game/gamework.h"
#include "game/wipe.h"
#include "game/mg/timer.h"
#include "game/mg/actman.h"
#include "datadir_enum.h"
#include "REL/m658dll/recovered.h"
#include "dolphin/gx.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "dolphin/types.h"
#include <string.h>
#include "include/game/mg/actman.h"
#include "game/process.h"
#include <dolphin/math.h>
#include <game/hu3d.h>
#include "dolphin/os.h"
#include "string.h"
#include "dolphin.h"
#include <game/object.h>
#include <dolphin/types.h>

typedef struct Position {
    HuVecF position;
    unsigned char unreadBytes[12];
    HuVecF offset;
} Position;
extern s32 lbl_1_data_F18;
extern float lbl_1_bss_1BC;
extern s8 lbl_1_bss_218, lbl_1_bss_219, lbl_1_data_2758;
s32 fn_1_367CC(s32 *player);
typedef struct M658CharacterSetting {
    s16 initialMotion;
    u8 unreadBytes[20];
    s16 idleMotion;
} M658CharacterSetting;
extern void fn_1_7048(s32 model, u8 *colors);
extern void fn_1_70F8(s32 model, const s8 *colors, f32 scale);
#define M658_UNREAD_VALUE_A_BIT_PATTERN 0x44FA0000
#define M658_UNREAD_VALUE_B_BIT_PATTERN 0xBF800000
#define M658_UNREAD_VALUE_C_BIT_PATTERN 0xFFFFFFFF
#define M658_PLAYER_FLAG_3_CLEAR_MASK 0xFFFFFFF7
#define M658_PLAYER_FLAG_2_CLEAR_MASK 0xFFFFFFFB
#define M658_PLAYER_FLAGS_4_AND_5_CLEAR_MASK 0xFFFFFFCF
#define M658_SOUND_ROW_ENTRY_PLAYER_1 2091
#define M658_SOUND_ROW_ENTRY_PLAYER_2 2092
#define M658_SOUND_ROW_RETURN_PLAYER_1 2093
#define M658_SOUND_ROW_RETURN_PLAYER_2 2094
#define M658_SOUND_CURSOR_MOVE_PLAYER_1 2095
#define M658_SOUND_CURSOR_MOVE_PLAYER_2 2096
#define M658_SOUND_SELECTION_PLAYER_1 2097
#define M658_SOUND_SELECTION_PLAYER_2 2098
#define M658_SOUND_ITEM_PLAYER_1 2099
#define M658_SOUND_ITEM_PLAYER_2 2100
#define M658_SOUND_RESULT_1 2101
#define M658_SOUND_RESULT_2 2102
#define M658_SOUND_RESULT_3 2103
#define M658_SOUND_RESULT_4 2104
void fn_1_404(MGPLAYER *);
void fn_1_2000(void *, s32);
void fn_1_2188(void *, s32);
void fn_1_2310(void *, s32);
void fn_1_24F0(void *, s32);
void fn_1_260C(void *, s32);
void fn_1_2728(void *, s32);
s32 lbl_1_data_30 = 2;
s32 lbl_1_data_34 = 4;
unsigned int lbl_1_data_38[17] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 5),
    DATANUM(DATA_mariomot, 56), DATANUM(DATA_mariomot, 8),
    DATANUM(DATA_mariomot, 10), DATANUM(DATA_mariomot, 9),
    DATANUM(DATA_mariomot, 85), DATANUM(DATA_mariomot, 30),
    DATANUM(DATA_mario, 32), DATANUM(DATA_mariomot, 40),
    DATANUM(DATA_mariomot, 22), DATANUM(DATA_mariomot, 22),
    0
};
u32 lbl_1_data_7C[4][3] = {
    { 5400, 7200, 60 },
    { 3600, 5400, 60 },
    { 3000, 4200, 60 },
    { 2100, 2700, 60 }
};
Vec lbl_1_data_AC[6] = {
    { 1400.0, 0.0, -1000.0 },
    { -1400.0, 700.0, -1000.0 },
    { 1400.0, 200.0, -1000.0 },
    { -1400.0, 700.0, -1000.0 },
    { -1400.0, -300.0, -1000.0 },
    { 1400.0, 200.0, -1000.0 }
};
s32 lbl_1_data_F4[5] = {60, 20, 10, 10, -1};
char *lbl_1_data_1A4[16] = {
    "658-1P-1", "658-1P-2", "658-1P-3", "658-1P-4",
    "658-1P-5", "658-2P-1", "658-2P-2", "658-2P-3",
    "658-2P-4", "658-2P-5", "658-gole", "658ENDNEW",
    "658-EFF2-1", "658-EFF2-2", "658-EFF03", "658-Rocket2"
};
s32 lbl_1_data_1E4[2][6] = {
    { 0, 1, 2, 3, 4, 10 },
    { 5, 6, 7, 8, 9, 10 }
};
/* These seven words are never read by the T Minus Five routines. */
u32 lbl_1_data_214[7] = {
    0, M658_UNREAD_VALUE_A_BIT_PATTERN, M658_UNREAD_VALUE_A_BIT_PATTERN,
    0, M658_UNREAD_VALUE_B_BIT_PATTERN, M658_UNREAD_VALUE_B_BIT_PATTERN,
    M658_UNREAD_VALUE_C_BIT_PATTERN
};
Vec lbl_1_data_230 = { 0.0, 200.0, 0.0 };
Vec lbl_1_data_23C = { 0.0, 0.0, 0.0 };
GXColor lbl_1_data_248 = {255, 255, 210, 255};
Vec lbl_1_data_24C = { 0.0, 0.0, 2000.0 };
Vec lbl_1_data_258 = { 0.0, 0.0, -1.0 };
GXColor lbl_1_data_264[2] = {{255,255,255,0},{242,234,255,0}};
s32 lbl_1_data_26C[6][20] = {
    { 1, 0, 2, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
MGPLAYER_MODE_FUNC lbl_1_data_44C[18] = {0,0,0,0,0,0,0,0,0,0,fn_1_404};
typedef struct M658RegistryDescriptor {
    void (*render)(void);
    void (*update)(void *, s32);
    s32 itemBytes, count;
} M658RegistryDescriptor;
/* Each descriptor gives effect callbacks, item work size and pool capacity; the last ends the
 * list. */
M658RegistryDescriptor lbl_1_data_494[7] = {
    {NULL, fn_1_2000, 64, 2},
    {NULL, fn_1_2188, 64, 2},
    {NULL, fn_1_2310, 64, 2},
    {NULL, fn_1_24F0, 64, 1},
    {NULL, fn_1_260C, 64, 1},
    {NULL, fn_1_2728, 64, 1},
    {NULL,NULL,0,0}
};

/* Player mode 10 flashes material RGB for 90 frames after a missed selection, then restores it. */
void fn_1_404(MGPLAYER *player)
{
    u16 idleMode;
    u16 initialMode;
    s32 timerFrame;
    f32 fade;

    switch (player->subMode) {
    case 0:
        MgPlayerComStkOn(player);
        if (player->motNo != 11) {
            CharMotionShiftSet(player->charNo,
                               ((M658CharacterSetting *)player->omObj->mtnId)->idleMotion,
                               0.0f, 5.0f, HU3D_MOTATTR_LOOP);
            idleMode = 11;
            player->motNo = idleMode;
        }
        player->timer = 9e+01f;
        player->subMode = 1;
        break;
    case 1:
        player->timer -= 1.0f;
        if (7e+01f == player->timer) {
            omVibrate(player->playerNo, 30, 4, 4);
        }
        timerFrame = (s32)player->timer;
        if (timerFrame % 4 == 0) {
            if (timerFrame % 8 == 0) {
                fade = 1.0f;
            } else {
                fade = 0.0f;
            }
            fn_1_70F8(player->actor->mdlId,
                      (const s8 *)player->omObj->work[0], fade);
        }
        if (!(player->timer > 0.0f)) {
            fn_1_7048(player->actor->mdlId,
                      (u8 *)player->omObj->work[0]);
            MgPlayerModeSet(player, 0);
            if (player->motNo != 0) {
                CharMotionShiftSet(player->charNo,
                                   ((M658CharacterSetting *)player->omObj->mtnId)->initialMotion,
                                   0.0f, 5.0f,
                                   HU3D_MOTATTR_LOOP);
                initialMode = 0;
                player->motNo = initialMode;
            }
        }
        break;
    }
}
/* Called during setup to cache each player's team and restore the default split if needed. */
void fn_1_5C0(M658Runtime *work)
{
    s32 counts[4];
    s32 player;
    s32 group;

    if (lbl_1_bss_38 != NULL) {
        player = 0;
        while (player < 4) {
            counts[player] = 0;
            player++;
        }
        player = 0;
        while (player < 4) {
            group = GwPlayerConf[player].grpNo;
            lbl_1_bss_38->group[player] = group;
            counts[group]++;
            player++;
        }
        if ((counts[0] != 1) || (counts[1] != 1)) {
            lbl_1_bss_38->group[0] = 0;
            lbl_1_bss_38->group[1] = 1;
            lbl_1_bss_38->group[2] = 2;
            lbl_1_bss_38->group[3] = 2;
        }
    }
}
/* Returns this player's cached team when the minigame runtime is available. */
s32 fn_1_730(M658PlayerWork *player)
{
    if (lbl_1_bss_38 == NULL) {
        return;
    }
    return lbl_1_bss_38->group[player->playerNo];
}
void fn_1_2828(OMOBJ *);
void *fn_1_A0(s32, u32, void (*)(OMOBJ *));
/* The sequence setup callback creates the shared T Minus Five runtime object. */
void fn_1_768(void) {
    lbl_1_bss_38 = fn_1_A0(100, 132, fn_1_2828);
}
extern void fn_1_1500(s32 player, s32 mode);
extern void fn_1_6414(void *resource);
extern s16 *fn_1_753C(s32 index);
extern OMOBJ *fn_1_6384(void *description);
extern u16 *fn_1_1F04(u32 index);
extern s32 fn_1_1D14(M658PlayerWork *, s32);
extern void Hu3DMotionTimeSet(s16 model, f32 time);
extern void Hu3DMotionSpeedSet(s16 model, f32 speed);
extern void Hu3DModelAttrReset(s16 model, u32 flags);
extern void Hu3DModelPosSetV(s16 model, Vec *position);
extern void MgPlayerColDisable(MGACTOR *actor);
extern void fn_1_1C98(M658PlayerWork *work, s32 mode, f32 time, s32 flags);
extern void fn_1_1C54(u16 mode);
/* After the winner reveal, replaces selection props with the closing scene and prepares both
 * characters. */
void fn_1_7A4(void)
{
    /* This zero-initialized local is never read. */
    s32 stackValue = 0;
    if (lbl_1_bss_38 == NULL) {
        return;
    }

    fn_1_1500(0, 0);
    fn_1_1500(1, 0);

    fn_1_6414(lbl_1_bss_38->objects[7]);
    fn_1_6414(lbl_1_bss_38->objects[6]);
    fn_1_6414(lbl_1_bss_38->objects[11]);
    fn_1_6414(lbl_1_bss_38->objects[10]);
    fn_1_6414(lbl_1_bss_38->objects[5]);
    fn_1_6414(lbl_1_bss_38->objects[1]);
    fn_1_6414(lbl_1_bss_38->objects[0]);
    fn_1_6414(lbl_1_bss_38->objects[2]);
    fn_1_6414(lbl_1_bss_38->objects[4]);

    lbl_1_bss_38->objects[5] = NULL;
    lbl_1_bss_38->objects[10] = NULL;
    lbl_1_bss_38->objects[11] = NULL;
    lbl_1_bss_38->objects[6] = NULL;
    lbl_1_bss_38->objects[7] = NULL;
    lbl_1_bss_38->objects[2] = NULL;
    lbl_1_bss_38->objects[0] = NULL;
    lbl_1_bss_38->objects[1] = NULL;
    lbl_1_bss_38->objects[4] = NULL;

    lbl_1_bss_38->objects[12] = fn_1_6384(fn_1_753C(12));
    lbl_1_bss_38->objects[13] = fn_1_6384(fn_1_753C(13));
    fn_1_1F04(5);
    fn_1_1C54(4);

    Hu3DMotionTimeSet(lbl_1_bss_38->objects[8]->mdlId[0], 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_38->objects[8]->mdlId[0], 0.0f);
    Hu3DMotionTimeSet(lbl_1_bss_38->objects[8]->mdlId[1], 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_38->objects[8]->mdlId[1], 0.0f);
    Hu3DMotionTimeSet(lbl_1_bss_38->objects[13]->mdlId[0], 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_38->objects[13]->mdlId[0], 1.0f);
    Hu3DMotionTimeSet(lbl_1_bss_38->objects[13]->mdlId[1], 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_38->objects[13]->mdlId[1], 1.0f);

    Hu3DModelAttrReset(lbl_1_bss_38->selected->player->actor->mdlId, 1);
    fn_1_1D14(lbl_1_bss_38->selected, 12);
    if (lbl_1_bss_38->other) {
        MgPlayerColDisable(lbl_1_bss_38->other->player->actor);
        lbl_1_bss_38->other->motionStart = lbl_1_data_AC[0];
        lbl_1_bss_38->other->motionEnd = lbl_1_data_AC[1];
        Hu3DModelPosSetV(lbl_1_bss_38->other->player->actor->mdlId,
                         &lbl_1_bss_38->other->motionStart);
        fn_1_1C98(lbl_1_bss_38->other, 0, 0.0f,
                  HU3D_MOTATTR_LOOP);
        fn_1_1D14(lbl_1_bss_38->other, 0);
    }
}
/* The fade-in music callback tests whether the runtime is still in its opening phase. */
s32 fn_1_C38(void)
{
    if (lbl_1_bss_38 == NULL) {
        return 0;
    }
    return lbl_1_bss_38->phase == 1;
}
/* The main sequence callback tests whether the runtime has entered the winner-reveal phase. */
s32 fn_1_C84(void)
{
    if (lbl_1_bss_38 == NULL) return 0;
    return lbl_1_bss_38->phase == 3;
}
/* Reports whether the sequence is in either of the two result phases. */
s32 fn_1_CD0(void) {
    s32 isResultPhase;

    if ((void *) lbl_1_bss_38 == NULL) {
        return 0;
    }
    isResultPhase = 1;
    if ((lbl_1_bss_38->phase != 5) && (lbl_1_bss_38->phase != 4)) {
        isResultPhase = 0;
    }
    return isResultPhase;
}
/* Reports whether the sequence has reached the winner presentation phase. */
s32 fn_1_D40(void) {
    if ((void *) lbl_1_bss_38 == NULL) {
        return 0;
    }
    return (s16) lbl_1_bss_38->phase == 6;
}
M658Runtime *lbl_1_bss_38;
M658PlayerWork *lbl_1_bss_28[4];
M658Rows *lbl_1_bss_24;
void *lbl_1_bss_20;
void *lbl_1_bss_1C;
M658RegistryWork *lbl_1_bss_18;
s32 lbl_1_bss_14;
s32 lbl_1_bss_10;
s32 lbl_1_bss_C;
s32 lbl_1_bss_8;
void fn_1_1040(s32 index, s32 adjust, Vec *position);
/* Picks one eligible player and records the other player for paired animations. */
void fn_1_D8C(void)
{
    s32 index;
    if (lbl_1_bss_38->count == 1) {
        lbl_1_bss_38->selected = lbl_1_bss_38->available[0];
    } else {
        index = frandmod(lbl_1_bss_38->count);
        if (index >= lbl_1_bss_38->count) {
            index = 0;
        }
        lbl_1_bss_38->selected = lbl_1_bss_38->available[index];
    }
    for (index = 0; index < 2; index++) {
        if (lbl_1_bss_38->selected != lbl_1_bss_28[index]) {
            lbl_1_bss_38->other = lbl_1_bss_28[index];
            break;
        }
    }
}
/* Adds a player record to the runtime's eligible-player list. */
s32 fn_1_EB0(void *player)
{
    lbl_1_bss_38->available[lbl_1_bss_38->count] = player;
    lbl_1_bss_38->count++;
}
/* Switches the board, row models, and character effects between selection layers. */
void fn_1_EF4(s32 mode)
{
    s32 player, model;
    switch (mode) {
    case 0:
        lbl_1_bss_C = 0;
        lbl_1_bss_10 = 0;
        lbl_1_bss_14 = 0;
        break;
    case 1:
        lbl_1_bss_C = 3;
        lbl_1_bss_10 = 4;
        lbl_1_bss_14 = 5;
        break;
    }
    Hu3DModelLayerSet(lbl_1_bss_38->objects[2]->mdlId[1], lbl_1_bss_C);
    for (player = 0; player < 2; player++) {
        for (model = 0; model < 5; model++) {
            Hu3DModelLayerSet(lbl_1_bss_24->player[player].models[model], lbl_1_bss_10);
        }
    }
    CharEffectLayerSet(lbl_1_bss_14);
}
/* Reads a named board marker and optionally uses its associated height marker. */
void fn_1_1040(s32 index, s32 adjust, Vec *position)
{
    HSF_OBJECT *node = NULL;
    if (lbl_1_bss_24 != NULL) {
        node = Hu3DModelObjPtrGet(lbl_1_bss_38->objects[2]->mdlId[1], lbl_1_data_1A4[index]);
        position->x = node->mesh.base.pos.x;
        position->y = node->mesh.base.pos.y;
        position->z = node->mesh.base.pos.z;
        if (adjust == 1) {
            if (index == 10) {
                node = Hu3DModelObjPtrGet(lbl_1_bss_38->objects[2]->mdlId[1], "658-gole-height");
            } else {
                node = Hu3DModelObjPtrGet(lbl_1_bss_38->objects[2]->mdlId[1], "658-1P-height");
            }
            position->y = node->mesh.base.pos.y;
        }
    }
}
extern void fn_1_1040(s32 index, s32 adjust, Vec *position);
extern void MgActorPosSet(MGACTOR *actor, Vec *position);
extern void MgActorRotYSet(MGACTOR *actor, f32 rotation);
/* Registers a player's character for the selection rows and places it at its marker. */
void fn_1_1158(M658PlayerWork *work)
{
    /* This zero-initialized local is never read. */
    s32 unusedInitialValue = 0;
    s32 registryIndex;
    s32 firstMarkerIndex;
    s32 index;
    s32 playerIndex;
    Vec position;
    Vec scale;

    if (work->playerIndex == 0) {
        firstMarkerIndex = 0;
    } else {
        firstMarkerIndex = 5;
    }
    index = firstMarkerIndex;
    if (lbl_1_bss_24 != NULL) {
        playerIndex = work->playerIndex;
        registryIndex = lbl_1_bss_24->registryIndex[playerIndex];
        lbl_1_bss_24->entries[playerIndex][registryIndex] = work;
        fn_1_1040(index, 1, &position);
        work->actorRotation = 0.0f;
        work->startPosition = position;
        work->registryIndex = registryIndex;
        scale.x = 0.0f;
        scale.y = 1.5f;
        scale.z = 0.0f;
        Hu3DModelScaleSetV((s16)work->player->actor->mdlId, &scale);
        MgActorPosSet(work->player->actor, &position);
        MgActorRotYSet(work->player->actor, work->actorRotation);
        lbl_1_bss_24->registryIndex[playerIndex]++;
    }
}
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_148(void);
extern HUPROCESS *lbl_1_bss_0;
extern OMOBJMAN *lbl_1_bss_0;
extern const HuVecF lbl_1_rodata_10, lbl_1_rodata_1C, lbl_1_rodata_28;
extern void fn_1_264(void);
/* The row constructor adds each board model to the collision map used during selection. */
void fn_1_1238(M658Rows *rows, s16 modelId)
{
    rows->collisionModels[rows->collisionModelCount] = modelId;
    rows->collisionModelCount++;
}
void fn_1_1040(s32 index, s32 adjust, Vec *position);
/* Builds a player's five row models and the final marker used by the selection sequence. */
void fn_1_12EC(s32 player)
{
    Vec position;
    M658PlayerRow *row;
    s32 index;
    if (lbl_1_bss_24 != NULL) {
        row = &lbl_1_bss_24->player[player];
        for (index = 0; index < 5; index++) {
            row->models[index] = lbl_1_bss_38->objects[0]->mdlId[lbl_1_bss_24->modelCount + 1];
            fn_1_1040(lbl_1_data_1E4[player][index], 0, &position);
            Hu3DModelPosSetV(row->models[index], &position);
            Hu3DMotionSet(row->models[index], 0);
            Hu3DModelLayerSet(row->models[index], lbl_1_bss_10);
            lbl_1_bss_24->modelCount++;
        }
        row->models[index] = lbl_1_bss_38->objects[1]->mdlId[1];
        fn_1_1040(lbl_1_data_1E4[player][index], 0, &position);
        Hu3DModelPosSetV(row->models[index], &position);
        Hu3DMotionSet(row->models[index], 0);
        Hu3DModelLayerSet(row->models[index], lbl_1_data_34);
        row->step = -1;
        row->timer = 0;
        row->position = 0;
        row->mode = 1;
        row->interval = 58;
        Hu3DMotionSet(row->models[row->position], 2);
    }
}
/* Changes the row mode unless locked in mode 7. Mode 3 primes an immediate cursor step;
 * others clear the timer. */
void fn_1_1500(s32 player, s32 mode)
{
    if (lbl_1_bss_24 != NULL) {
        M658PlayerRow *entry;

        entry = &lbl_1_bss_24->player[player];
        if (entry->mode != 7) {
            if (mode != 3) {
                entry->timer = 0;
            } else {
                entry->timer = entry->interval;
            }
            entry->mode = (s16)mode;
        }
    }
}
/* The row controller hides or shows the five models in a player's selection row. */
void fn_1_1574(s32 player, s32 resetAttributes)
{
    M658PlayerRow *entry;
    s32 index;
    s32 modelId;

    if (lbl_1_bss_24 != NULL) {
        entry = &lbl_1_bss_24->player[player];

        for (index = 0; index < 5; index++) {
            modelId = entry->models[index];
            if (resetAttributes != 0) {
                Hu3DModelAttrReset(modelId, 1);
            } else {
                Hu3DModelAttrSet(modelId, 1);
            }
        }
    }
}
extern void fn_1_736C(Vec *position, s32 soundId);
/* Called each row-update tick to move the highlight, play its sound, and finish choices. */
void fn_1_162C(s32 player)
{
    M658PlayerRow *work;
    s32 animationIndex;
    Vec position;

    if (lbl_1_bss_24 == NULL) {
        return;
    }

    work = &lbl_1_bss_24->player[player];
    switch (work->mode) {
    case 1:
        if (work->timer >= 58) {
            Hu3DMotionSet(work->models[work->position], 3);
            work->timer = 0;
            work->position++;
            if (work->position > 4) {
                work->mode = 0;
            } else {
                fn_1_1040(lbl_1_data_1E4[player][work->position], 0, &position);
                if (player == 0) {
                    fn_1_736C(&position, M658_SOUND_CURSOR_MOVE_PLAYER_1);
                } else {
                    fn_1_736C(&position, M658_SOUND_CURSOR_MOVE_PLAYER_2);
                }
                Hu3DMotionSet(work->models[work->position], 1);
            }
        }
        work->timer++;
        break;

    case 2:
        work->timer = 0;
        work->position = 4;
        work->mode = 3;
        Hu3DMotionSet(work->models[work->position], 1);
        fn_1_1040(lbl_1_data_1E4[player][work->position], 0, &position);
        if (player == 0) {
            fn_1_736C(&position, M658_SOUND_CURSOR_MOVE_PLAYER_1);
        } else {
            fn_1_736C(&position, M658_SOUND_CURSOR_MOVE_PLAYER_2);
        }
        break;

    case 3:
        work->timer++;
        if (work->timer >= work->interval) {
            Hu3DMotionSet(work->models[work->position], 3);
            work->timer = 0;
            work->position += work->step;
            Hu3DMotionSet(work->models[work->position], 1);
            fn_1_1040(lbl_1_data_1E4[player][work->position], 0, &position);
            if (player == 0) {
                fn_1_736C(&position, M658_SOUND_CURSOR_MOVE_PLAYER_1);
            } else {
                fn_1_736C(&position, M658_SOUND_CURSOR_MOVE_PLAYER_2);
            }
            animationIndex = work->position;
            if (animationIndex == 0 || animationIndex == 4) {
                work->step *= -1;
            }
        }
        break;

    case 4:
        if (work->timer == 0) {
            fn_1_1040(lbl_1_data_1E4[player][work->position], 0, &position);
            if (player == 0) {
                fn_1_736C(&position, M658_SOUND_SELECTION_PLAYER_1);
            } else {
                fn_1_736C(&position, M658_SOUND_SELECTION_PLAYER_2);
            }
        }
        if ((work->timer % 2) == 0) {
            if ((work->timer % 4) == 0) {
                Hu3DMotionSet(work->models[work->position], 0);
            } else {
                Hu3DMotionSet(work->models[work->position], 2);
            }
        }
        work->timer++;
        if (work->timer > 60) {
            fn_1_1500(player, 0);
            Hu3DMotionSet(work->models[work->position], 2);
            if (work->interval > 10) {
                work->interval -= 12;
            }
        }
        break;

    case 5:
        Hu3DMotionSet(work->models[work->position], 3);
        fn_1_1500(player, 7);
        break;

    case 6:
        work->timer++;
        if (work->timer >= 5) {
            Hu3DMotionSet(work->models[work->position], 3);
            work->timer = 0;
            work->position++;
            if (work->position > 5) {
                work->position = 0;
            }
            Hu3DMotionSet(work->models[work->position], 1);
        }
        break;

    case 0:
        break;
    }
}
/* Searches ten cursor steps for an arrival at least 45 frames away. Returns -1 during a step,
 * or the player index if none qualifies. */
s32 fn_1_1AA0(s32 index, s32 target)
{
    M658PlayerRow *row;
    s32 time;
    s32 i;
    s32 position;
    s32 step;
    s32 interval;
    s32 next;

    row = &lbl_1_bss_24->player[index];
    if (row->timer != 0) {
        return -1;
    }

    interval = row->interval;
    time = 0;
    position = row->position;
    step = row->step;
    i = 0;
    while (i < 10) {
        next = position + step;
        if (next < 0 || next > 4) {
            step *= -1;
        }
        next = position + step;
        if (position == target && time >= 45) {
            return time;
        }
        time += interval;
        position = next;
        i += 1;
    }
    return index;
}
void fn_1_1500(s32 player, s32 mode);
/* Collision correction records contact. During hip-drop submode 2, contact after a contact-free
 * update compares the player's action with the highlighted row. */
void fn_1_1B58(MGACTOR *actor, int workValue)
{
    M658PlayerWork *work = (M658PlayerWork *)workValue;

    work->flags |= 2;
    if ((work->flags & 1) != 0 &&
        (work->player)->mode == 5 &&
        (work->player)->subMode == 2 &&
        (work->actorFlags & 2) == 0) {
        MgPlayerComStkOn(work->player);
        omVibrate((work->player)->playerNo, 20, 4, 4);
        if (work->action ==
            lbl_1_bss_24->player[work->playerIndex].position) {
            fn_1_1500(work->playerIndex, 4);
            work->flags |= 4;
        } else {
            work->flags |= 8;
        }
    }
}
typedef struct M658CameraStorageView {
    s16 mode;
    s16 frame;
    u8 unreadBytesBeforePosition[12];
    Vec position;
    Vec target;
    u8 unreadBytesAfterTarget[56];
} M658CameraStorageView;
/* Selects the scene camera's mode and restarts its per-mode frame counter. */
void fn_1_1C54(u16 mode) {
    if ((void *) lbl_1_bss_1C != NULL) {
        ((M658CameraStorageView *)lbl_1_bss_1C)->frame = 0;
        ((M658CameraStorageView *)lbl_1_bss_1C)->mode = mode;
    }
}
/* Player-state callbacks shift to a new character motion only when its slot changes. */
void fn_1_1C98(M658PlayerWork *work, s32 motionSlot, f32 endTime, s32 attributes)
{
    if (work->motionSlot != motionSlot) {
        work->motionSlot = motionSlot;
        CharMotionShiftSet(work->characterNo,
                           (work->player)->omObj->mtnId[motionSlot],
                           0.0f, endTime, attributes);
    }
}
/* Called by character-state handlers to restart the frame counter on a state change. */
s32 fn_1_1D14(M658PlayerWork *work, s32 nextState)
{
    if (work->state != nextState) {
        work->frame = 0;
        work->state = (work->nextState = (s8)nextState);
    }
}
/* Immediately copies the queued player state into the current state and clears its frame
 * counter. */
void fn_1_1D3C(M658PlayerWork *work) {
    (work->frame) = 0;
    (work->state) = (work->nextState);
}
/* Disables motion-event processing and resets the selected event-table index. */
void fn_1_1D50(void) {
    if ((void *) lbl_1_bss_38 != NULL) {
        (lbl_1_bss_38->motionActive) = 0;
        (lbl_1_bss_38->motionEventSet) = 0;
    }
}
/* Sets the motion-event processing enable value without changing the selected event table. */
void fn_1_1D94(u16 enabled) {
    if ((void *) lbl_1_bss_38 != NULL) {
        (lbl_1_bss_38->motionActive) = enabled;
    }
}
/* Starts processing the selected motion-event table from its first entry. */
void fn_1_1DC4(s32 eventTable) {
    if ((void *) lbl_1_bss_38 != NULL) {
        (lbl_1_bss_38->motionActive) = 1;
        (lbl_1_bss_38->motionEventSet) = eventTable;
        (lbl_1_bss_38->motionEventIndex) = 0;
    }
}
/* With processing enabled, the timing hook clears event flags; only a true lag flag consumes
 * the next zero-terminated event group. */
void fn_1_1E18(HU3D_MODELID modelId, HU3D_MOTIONID motionId, BOOL lag)
{
    s32 eventId;
    s32 running;

    if (lbl_1_bss_38->motionActive == 0) {
        return;
    }

    lbl_1_bss_38->stateFlags = 0;
    if (lag == FALSE) {
        return;
    }

    running = TRUE;
    while (running != FALSE) {
        eventId = lbl_1_data_26C[
            lbl_1_bss_38->motionEventSet][
            lbl_1_bss_38->motionEventIndex];
        switch (eventId) {
        default:
            lbl_1_bss_38->stateFlags |= 1 << eventId;
            break;
        case 0:
            running = FALSE;
            break;
        }
        (lbl_1_bss_38->motionEventIndex)++;
    }
}
extern void *fn_1_6764(void *pool);
extern void *fn_1_67A0(void *pool, void *item);
/* Player and scene handlers claim an inactive pooled effect and run its start callback. */
u16 *fn_1_1F04(u32 index)
{
    M658RegistryEntry *entry;
    void *pool;
    u16 *item;

    if (lbl_1_bss_18 == NULL) {
        return NULL;
    }
    entry = &lbl_1_bss_18->entries[index];
    pool = entry->pool;
    item = (u16 *)fn_1_6764(pool);
    while (item) {
        if ((item[0] & 1) == 0) {
            item[0] |= 1;
            item[1] = 0;
            entry->update(item, 2);
            return item;
        }
        item = (u16 *)fn_1_67A0(pool, item);
    }
    return NULL;
}
/* Requests the effect's release callback and inactive state on the next registry update. */
void fn_1_1FEC(u16 *flags)
{
    *flags |= 2;
}
extern void fn_1_1FEC(u16 *);
/* The pooled attachment stores its effect model, host model, and attachment name. */
typedef struct M658HookActor {
    u8 unreadBytesBeforeModel[6];
    s16 model;
    u8 unreadBytesBeforeHost[48];
    s32 hostModel;
    char *hook;
} M658HookActor;
/* Manages the first pooled character effect through allocation, playback, and return. */
void fn_1_2000(void *callbackData, s32 action)
{

    switch (action) {
    case 0:
        ((M658HookActor *)callbackData)->model =
            lbl_1_bss_38->objects[5]->mdlId[lbl_1_bss_18->count + 1];
        Hu3DMotionTimeSet(((M658HookActor *)callbackData)->model, 0.0f);
        Hu3DMotionSpeedSet(((M658HookActor *)callbackData)->model, 0.0f);
        Hu3DModelAttrSet(((M658HookActor *)callbackData)->model, 1);
        break;
    case 2:
        Hu3DModelAttrReset(((M658HookActor *)callbackData)->model, 1);
        Hu3DModelScaleSet(((M658HookActor *)callbackData)->model, 0.88f, 1.0f, 0.88f);
        Hu3DMotionTimeSet(((M658HookActor *)callbackData)->model, 0.0f);
        Hu3DMotionSpeedSet(((M658HookActor *)callbackData)->model, 1.0f);
        Hu3DModelLayerSet(((M658HookActor *)callbackData)->model, (s16)lbl_1_bss_14);
        break;
    case 3:
        if (Hu3DMotionEndCheck(((M658HookActor *)callbackData)->model)) {
            Hu3DMotionTimeSet(((M658HookActor *)callbackData)->model, 0.0f);
            Hu3DMotionSpeedSet(((M658HookActor *)callbackData)->model, 0.0f);
            Hu3DModelAttrSet(((M658HookActor *)callbackData)->model, 1);
            fn_1_1FEC(callbackData);
        }
        break;
    default:
        break;
    }
}
/* Manages the second pooled character effect through allocation, playback, and return. */
void fn_1_2188(void *callbackData, s32 action)
{

    switch (action) {
    case 0:
        ((M658HookActor *)callbackData)->model =
            lbl_1_bss_38->objects[7]->mdlId[lbl_1_bss_18->count + 1];
        Hu3DMotionTimeSet(((M658HookActor *)callbackData)->model, 0.0f);
        Hu3DMotionSpeedSet(((M658HookActor *)callbackData)->model, 0.0f);
        Hu3DModelAttrSet(((M658HookActor *)callbackData)->model, 1);
        break;
    case 2:
        Hu3DModelAttrReset(((M658HookActor *)callbackData)->model, 1);
        Hu3DModelScaleSet(((M658HookActor *)callbackData)->model, 0.88f, 1.0f, 0.88f);
        Hu3DMotionTimeSet(((M658HookActor *)callbackData)->model, 0.0f);
        Hu3DMotionSpeedSet(((M658HookActor *)callbackData)->model, 1.0f);
        Hu3DModelLayerSet(((M658HookActor *)callbackData)->model, (s16)lbl_1_bss_14);
        break;
    case 3:
        if (Hu3DMotionEndCheck(((M658HookActor *)callbackData)->model)) {
            Hu3DMotionTimeSet(((M658HookActor *)callbackData)->model, 0.0f);
            Hu3DMotionSpeedSet(((M658HookActor *)callbackData)->model, 0.0f);
            Hu3DModelAttrSet(((M658HookActor *)callbackData)->model, 1);
            fn_1_1FEC(callbackData);
        }
        break;
    default:
        break;
    }
}
/* Positions a pooled floating model at its named character attachment. */
void fn_1_2310(void *actor, s32 phase)
{
    Vec position;
    switch (phase) {
    case 0:
        ((M658HookActor *)actor)->model = lbl_1_bss_38->objects[6]->mdlId[lbl_1_bss_18->count + 1];
        Hu3DMotionTimeSet(((M658HookActor *)actor)->model, 0.0f);
        Hu3DMotionSpeedSet(((M658HookActor *)actor)->model, 0.0f);
        Hu3DModelAttrSet(((M658HookActor *)actor)->model, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(((M658HookActor *)actor)->model, 1);
        Hu3DModelAmbSet(((M658HookActor *)actor)->model, 2.0f, 2.0f, 2.0f);
        Hu3DModelScaleSet(((M658HookActor *)actor)->model, 0.7f, 0.7f, 0.7f);
        break;
    case 2:
        Hu3DModelLayerSet(((M658HookActor *)actor)->model, lbl_1_bss_C);
        Hu3DModelAttrReset(((M658HookActor *)actor)->model, 1);
        Hu3DMotionTimeSet(((M658HookActor *)actor)->model, 0.0f);
        Hu3DMotionSpeedSet(((M658HookActor *)actor)->model, 2.0f);
        break;
    case 3:
        if (((M658HookActor *)actor)->hook) {
            Hu3DModelObjPosGet(((M658HookActor *) actor)->hostModel,
                               ((M658HookActor *) actor)->hook, &position);
            Hu3DModelPosSetV(((M658HookActor *)actor)->model, &position);
        }
        break;
    case 1:
        Hu3DMotionTimeSet(((M658HookActor *)actor)->model, 0.0f);
        Hu3DMotionSpeedSet(((M658HookActor *)actor)->model, 0.0f);
        Hu3DModelAttrSet(((M658HookActor *)actor)->model, 1);
        break;
    }
}
/* Places the result prop at its named attachment and stops it when its pooled item is released. */
void fn_1_24F0(void *actor, s32 phase)
{
    Vec position;
    switch (phase) {
    case 0:
        break;
    case 2:
        ((M658HookActor *)actor)->model = lbl_1_bss_38->objects[10]->mdlId[0];
        ((M658HookActor *)actor)->hostModel = lbl_1_bss_38->objects[2]->mdlId[0];
        ((M658HookActor *)actor)->hook = lbl_1_data_1A4[12];
        Hu3DModelLayerSet(((M658HookActor *)actor)->model, lbl_1_data_30);
        /* The creation phase also updates the attachment position. */
    case 3:
        if (((M658HookActor *)actor)->hook) {
            Hu3DModelObjPosGet(((M658HookActor *) actor)->hostModel,
                               ((M658HookActor *) actor)->hook, &position);
            Hu3DModelPosSetV(((M658HookActor *)actor)->model, &position);
        }
        break;
    case 1:
        Hu3DMotionTimeSet(((M658HookActor *)actor)->model, 0.0f);
        Hu3DMotionSpeedSet(((M658HookActor *)actor)->model, 0.0f);
        Hu3DModelAttrSet(((M658HookActor *)actor)->model, 1);
        break;
    }
}
/* Follows the second result prop's attachment and stops it when its pooled item is released. */
void fn_1_260C(void *work, s32 action)
{
    HuVecF position;

    switch (action) {
    case 2:
        ((M658HookActor *)work)->model =
            lbl_1_bss_38->objects[11]->mdlId[0];
        ((M658HookActor *)work)->hostModel =
            lbl_1_bss_38->objects[2]->mdlId[0];
        ((M658HookActor *)work)->hook = lbl_1_data_1A4[13];
        Hu3DModelLayerSet(((M658HookActor *)work)->model, (s16)lbl_1_data_30);
    case 3:
        if (((M658HookActor *)work)->hook) {
            Hu3DModelObjPosGet((s16)((M658HookActor *)work)->hostModel,
                               ((M658HookActor *)work)->hook, &position);
            Hu3DModelPosSetV(((M658HookActor *)work)->model, &position);
        }
        break;
    case 1:
        Hu3DMotionTimeSet(((M658HookActor *)work)->model, 0.0f);
        Hu3DMotionSpeedSet(((M658HookActor *)work)->model, 0.0f);
        Hu3DModelAttrSet(((M658HookActor *)work)->model, 1);
        break;
    case 0:
    default:
        break;
    }
}
/* Hooks the end effect to the closing scene's second model and stops it when the pooled item
 * is released. */
void fn_1_2728(void *work, s32 action)
{
    switch (action) {
    case 2:
        ((M658HookActor *)work)->model =
            lbl_1_bss_38->objects[12]->mdlId[0];
        ((M658HookActor *)work)->hostModel =
            lbl_1_bss_38->objects[13]->mdlId[1];
        ((M658HookActor *)work)->hook = lbl_1_data_1A4[14];
        Hu3DModelAttrSet(((M658HookActor *)work)->model, HU3D_MOTATTR_LOOP);
        Hu3DModelHookSet((HU3D_MODELID)((M658HookActor *)work)->hostModel,
                         ((M658HookActor *)work)->hook,
                         ((M658HookActor *)work)->model);
        break;
    case 3:
        return;
    case 1:
        Hu3DMotionTimeSet(((M658HookActor *)work)->model, 0.0f);
        Hu3DMotionSpeedSet(((M658HookActor *)work)->model, 0.0f);
        Hu3DModelAttrSet(((M658HookActor *)work)->model, 1);
        break;
    case 0:
    default:
        break;
    }
}
#define fn_1_358C fn_1_38E4
#define fn_1_61C fn_1_5C0
s16 *fn_1_753C(s32 index);
OMOBJ *fn_1_6384(void *description);
void *fn_1_A0(s32 priority, u32 bytes, OMOBJ_FUNC update);
void fn_1_140(OMOBJ *object, OMOBJ_FUNC update);
void fn_1_1E18(HU3D_MODELID modelId, HU3D_MOTIONID motionId, BOOL lag);
void fn_1_358C(OMOBJ *), fn_1_3940(OMOBJ *), fn_1_34E0(OMOBJ *), fn_1_3BC0(OMOBJ *),
    fn_1_5430(OMOBJ *), fn_1_2A58(OMOBJ *);
void fn_1_2AF4(OMOBJ *), fn_1_2BB4(OMOBJ *), fn_1_2DAC(OMOBJ *);
void fn_1_31E8(OMOBJ *), fn_1_3294(OMOBJ *), fn_1_33CC(OMOBJ *);
/* The runtime constructor loads board models, creates the scene camera for opening, main play and
 * results, and registers sequence callbacks. */
void fn_1_2828(OMOBJ *object)
{
    M658Runtime *state = object->data;
    s32 player, model, index;
    memset(state, 0, sizeof(*state));
    fn_1_61C(state);
    for (model = 0; model <= 9; model++) {
        state->objects[model] = fn_1_6384(fn_1_753C(model));
    }
    Hu3DMotionSpeedSet(lbl_1_bss_38->objects[8]->mdlId[0], 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_38->objects[8]->mdlId[1], 0.0f);
    Hu3DMotionTimingHookSet(lbl_1_bss_38->objects[8]->mdlId[0], fn_1_1E18);
    Hu3DMotionTimingHookSet(lbl_1_bss_38->objects[8]->mdlId[1], fn_1_1E18);
    /* This computes and discards a random value; it does not reseed the shared random stream. */
    frandom(OSGetTick());
    lbl_1_bss_1C = fn_1_A0(10, 96, fn_1_358C);
    lbl_1_bss_24 = fn_1_A0(10, 140, fn_1_3940);
    lbl_1_bss_20 = fn_1_A0(10, 16, fn_1_34E0);
    index = 0;
    for (player = 0; player < 4; player++) {
        if (lbl_1_bss_38->group[player] < 2) {
            lbl_1_bss_28[index] = fn_1_A0(20, 132, fn_1_3BC0);
            lbl_1_bss_28[index]->playerNo = player;
            index++;
        }
    }
    lbl_1_bss_18 = fn_1_A0(30, 92, fn_1_5430);
    state->phase = 1;
    fn_1_140(object, fn_1_2A58);
}
/* The runtime object dispatches its opening, main-play and result handlers each update. */
void fn_1_2A58(OMOBJ *object)
{
    M658Runtime *state = object->data;

    switch (state->phase) {
    case 0:
        break;
    case 1:
        fn_1_2AF4(object);
        break;
    case 2:
        fn_1_2BB4(object);
        break;
    case 3:
        fn_1_2DAC(object);
        break;
    case 4:
        fn_1_31E8(object);
        break;
    case 5:
        fn_1_3294(object);
        break;
    case 6:
        fn_1_33CC(object);
        break;
    }
}
extern void fn_1_EF4(s32);
/* Starts the opening camera, then enables both selection rows and players when main play begins. */
void fn_1_2AF4(OMOBJ *obj)
{
    s16 *work = (s16 *)obj->data;
    s32 index;

    if (work[0] == 0) {
        fn_1_1C54(1);
    }
    work[0] += 1;
    if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
        fn_1_EF4(1);
        fn_1_1500(0, 2);
        fn_1_1500(1, 2);
        for (index = 0; index < 2; index++) {
            fn_1_1D14(lbl_1_bss_28[index], 2);
        }
        work[1] = 2;
    }
}
typedef struct M658TransitionWork {

    s16 timer;

    s16 phase;

} M658TransitionWork;
typedef struct M658SequenceUpdateWork {

    s16 timer;

    s16 phase;

    s16 state;

} M658SequenceUpdateWork;
extern u32 MgSeqModeGet(void);
extern u16 MgSeqDrawSet(void);
extern void fn_1_D8C(void);
extern void Hu3DMotionSet(s16 model, s16 motion);
extern f32 Hu3DMotionTimeGet(s16 model);
extern void Hu3DModelAttrSet(s16 model, u32 flags);
extern void fn_1_7468(s32 soundId);
extern void fn_1_1DC4(s32 eventTable);
extern void fn_1_571C(s32 player, s32 amount);
extern void WipeCreate(s16 type, s16 direction, s16 duration);
extern u16 MgSeqWinnerSet(s16 firstCharacter, s16 secondCharacter, s16 thirdCharacter,
                          s16 fourthCharacter);
/* Minigame diagnostics discard the supplied value in the game build. */
static inline void DebugValue(s32 value) { }
/* During main play, waits for a selected player or a draw, then enters the corresponding result
 * phase. */
void fn_1_2BB4(OMOBJ *object)

{

    M658TransitionWork *work = object->data;

    M658PlayerWork *sequences[2];

    BOOL sequenceFinished[2];

    sequences[0] = lbl_1_bss_24->entries[0][0];

    sequences[1] = lbl_1_bss_24->entries[1][0];

    /* These finished-state tests are saved locally but never read. */
    sequenceFinished[0] = sequences[0]->state == 9;

    sequenceFinished[1] = sequences[1]->state == 9;

    if (MgSeqModeGet() == 6) {

        lbl_1_bss_24->entries[0][0]->flags &= ~1;

        lbl_1_bss_24->entries[1][0]->flags &= ~1;

        work->timer = 0;

        work->phase = 4;

        MgSeqDrawSet();

        fn_1_1500(0, 5);

        fn_1_1500(1, 5);

    } else {

        if (lbl_1_bss_38->count != 0) {

            fn_1_D8C();

        }

        if ((u32)lbl_1_bss_38->selected != 0) {

            fn_1_1500(lbl_1_bss_38->other->playerIndex, 5);

            lbl_1_bss_38->other->flags |= 0x40;

            lbl_1_bss_38->selected->flags &= ~1;

            lbl_1_bss_38->other->flags &= ~1;

            work->timer = 0;

            work->phase = 3;

        }

    }

}
/* Advances the winner reveal, result messages, wipe, and return to the minigame sequence. */
void fn_1_2DAC(OMOBJ *object)

{

    M658SequenceUpdateWork *work = object->data;

    s32 secondSequenceSnapshot;

    s32 firstSequenceSnapshot;

    s32 zeroSnapshot;

    /* These initialized local values are never read by this update. */
    zeroSnapshot = 0;

    firstSequenceSnapshot = (s32)lbl_1_bss_24->entries[0][0];

    secondSequenceSnapshot = (s32)lbl_1_bss_24->entries[1][0];

    switch (work->state) {

    case 0:

        if (((M658Runtime *)lbl_1_bss_38)->selected->state == 8) {

            Hu3DMotionSet(((OMOBJ *)((M658Runtime *)lbl_1_bss_38)->objects[2])->mdlId[0],

                          ((OMOBJ *)((M658Runtime *)lbl_1_bss_38)->objects[2])->mtnId[2]);

            Hu3DMotionSpeedSet(((OMOBJ *)((M658Runtime *)lbl_1_bss_38)->objects[8])->mdlId[0],

                               1.0f);

            Hu3DMotionTimeSet(((OMOBJ *)((M658Runtime *)lbl_1_bss_38)->objects[8])->mdlId[0],

                              0.0f);

            fn_1_7468(M658_SOUND_RESULT_1);

            fn_1_1DC4(0);

            work->timer = 0;

            work->state++;

        }

        break;

    case 1:

        work->timer++;

        if (work->timer == 220) {

            fn_1_7468(M658_SOUND_RESULT_2);

            break;

        }

        if (work->timer == 250) {

            Hu3DModelAttrSet(((M658Runtime *)lbl_1_bss_38)->selected->player->actor->mdlId, 1);

            ((M658Runtime *)lbl_1_bss_38)->selected->rotation = 1.8e+02f;

            MgActorRotYSet(((M658Runtime *)lbl_1_bss_38)->selected->player->actor,

                           ((M658Runtime *)lbl_1_bss_38)->selected->rotation);

            ((M658Runtime *)lbl_1_bss_38)->objects[10] = fn_1_6384(fn_1_753C(10));

            ((M658Runtime *)lbl_1_bss_38)->objects[11] = fn_1_6384(fn_1_753C(11));

            lbl_1_bss_24->transitionObjects[0] = fn_1_1F04(3);

            lbl_1_bss_24->transitionObjects[1] = fn_1_1F04(4);

            fn_1_7468(M658_SOUND_RESULT_3);

            work->state++;

            break;

        }

        if ((((M658Runtime *)lbl_1_bss_38)->stateFlags & 2) != 0) {

            fn_1_1D14(((M658Runtime *)lbl_1_bss_38)->selected, 10);

        } else {

            /* This event-bit test is discarded and does not change the reveal state. */
            DebugValue(((M658Runtime *)lbl_1_bss_38)->stateFlags & 4);

        }

        break;

    case 2:

        if (Hu3DMotionTimeGet(((OMOBJ *)((M658Runtime *)lbl_1_bss_38)->objects[2])->mdlId[0]) <

            4e+02f) break;

        WipeCreate(2, 0, 120);

        work->timer = 120;

        MgSeqWinnerSet(((M658Runtime *)lbl_1_bss_38)->selected->characterNo,

                       -1, -1, -1);

        fn_1_571C(((M658Runtime *)lbl_1_bss_38)->selected->playerNo, 10);

        work->state++;

        break;

    case 3:

        work->timer--;

        if (work->timer <= 0) {

            fn_1_1FEC(lbl_1_bss_24->transitionObjects[0]);

            fn_1_1FEC(lbl_1_bss_24->transitionObjects[1]);

            Hu3DModelAttrReset(((M658Runtime *)lbl_1_bss_38)->selected->player->actor->mdlId, 1);

            ((M658Runtime *)lbl_1_bss_38)->selected->rotation = 0.0f;

            MgActorRotYSet(((M658Runtime *)lbl_1_bss_38)->selected->player->actor,

                           ((M658Runtime *)lbl_1_bss_38)->selected->rotation);

            work->state = 0;

            work->timer = 0;

            work->phase = 5;

        }

        break;

    }

}
typedef struct M658ModeState {
    s16 timer;
    s16 phase;
    s16 substate;
} M658ModeState;
typedef struct M658ModeSnapshot {
    s32 initial;
    M658PlayerWork *firstPlayer;
    M658PlayerWork *secondPlayer;
} M658ModeSnapshot;
void fn_1_2BB4(OMOBJ *object);
void fn_1_2DAC(OMOBJ *object);
/* The runtime update callback holds a draw result before the closing message. */
void fn_1_31E8(OMOBJ *object)
{
    M658ModeState *state = object->data;
    M658ModeSnapshot snapshot;

    /* These snapshot fields are initialized but never read by this callback. */
    snapshot.initial = 0;
    snapshot.firstPlayer = lbl_1_bss_24->entries[0][0];
    snapshot.secondPlayer = lbl_1_bss_24->entries[1][0];
    switch (state->substate) {
    case 0:
        state->timer = 210;
        state->substate++;
        break;
    case 1:
        state->timer--;
        if (state->timer == 0) {
            state->substate = 0;
            state->timer = 0;
            state->phase = 6;
        }
        break;
    }
}

extern void fn_1_7A4(void);
extern s32 CharFXPlay(s16 characterNo, s16 effect);
extern void fn_1_7244(s32 modelId, char *objectName, s32 targetModelId);
/* Runs the winner's exit animation after the result sequence completes. */
void fn_1_3294(OMOBJ *object)
{
    /* This zero-initialized local is never read. */
    s32 unusedInitialValue;
    M658ModeState *state;

    state = object->data;
    unusedInitialValue = 0;

    switch (state->substate) {
    case 0:
        fn_1_7A4();
        fn_1_7468(M658_SOUND_RESULT_4);
        WipeCreate(1, 5, 120);
        state->timer = 90;
        state->substate++;
        return;
    case 1:
        fn_1_7244(lbl_1_bss_38->objects[13]->mdlId[1], lbl_1_data_1A4[15],
            lbl_1_bss_38->selected->player->actor->mdlId);
        state->timer--;
        if (state->timer == 0) {
            CharFXPlay(lbl_1_bss_38->selected->player->charNo, 577);
            fn_1_1D14(lbl_1_bss_38->other, 13);
            state->substate = 0;
            state->timer = 0;
            state->phase = 6;
        }
    }
}
/* Keeps the selected character aligned with the exit prop; a draw starts both losing motions
 * when the winner sequence begins. */
void fn_1_33CC(OMOBJ *object)
{
    struct {
        M658PlayerWork *firstEvent;
        M658PlayerWork *secondEvent;
    } events;
    M658ModeState *state = object->data;

    events.firstEvent = lbl_1_bss_24->entries[0][0];
    events.secondEvent = lbl_1_bss_24->entries[1][0];
    if (lbl_1_bss_38->selected != NULL) {
        fn_1_7244(lbl_1_bss_38->objects[13]->mdlId[1], lbl_1_data_1A4[15],
            lbl_1_bss_38->selected->player->actor->mdlId);
    }
    if (((u32)MgSeqModeGet() == 8) && (lbl_1_bss_38->selected == NULL) && (state->substate == 0)) {
        fn_1_1D14(events.firstEvent, 11);
        fn_1_1D14(events.secondEvent, 11);
        state->substate++;
    }
}
void fn_1_140(OMOBJ *, OMOBJ_FUNC);
void fn_1_3578(OMOBJ *object);
typedef struct M658LightWork {
    u8 unreadBytesBeforePhase[2];
    s16 phase;
    u8 unreadBytesBeforeLight[8];
    s32 light;
} M658LightWork;
/* Creates the board's point light. Initial light creation replaces the shared zero direction
 * with +Z. */
void fn_1_34E0(OMOBJ *object) {
    M658LightWork *lightWork;

    lightWork = object->data;
    lightWork->light =
        (s32) Hu3DGLightCreateV(&lbl_1_data_230, &lbl_1_data_23C, &lbl_1_data_248);
    Hu3DGLightPointSet((s16)lightWork->light, 4e+02f, 1.0f, GX_DA_GENTLE);
    lightWork->phase = 0;
    fn_1_140(object, fn_1_3578);
}
/* The point light's per-frame callback copies its work pointer into an unread local. */
void fn_1_3578(OMOBJ *object)
{
    void *work;

    work = object->data;
}
extern void *memset(void *destination, int value, unsigned long count);
void fn_1_3684(OMOBJ *object);
void fn_1_3714(OMOBJ *object);
void fn_1_380C(OMOBJ *object);
void fn_1_385C(OMOBJ *object);
void fn_1_3860(OMOBJ *object);
/* Creates the scene camera with its perspective and viewport, then installs its update callback. */
void fn_1_38E4(OMOBJ *object)
{
    M658CameraStorageView *work;
    s16 cameraMask;

    work = object->data;
    memset(work, 0, sizeof(*work));
    cameraMask = 1;
    Hu3DCameraCreate(cameraMask);
    Hu3DCameraPerspectiveSet(cameraMask, 45.0f, 2e+01f,
        1.5e+04f, 1.2f);
    Hu3DCameraViewportSet(cameraMask, 0.0f, 0.0f,
        6.4e+02f, 4.8e+02f, 0.0f, 1.0f);
    work->frame = 0;
    fn_1_140(object, fn_1_3684);
}
/* Called each camera-object tick to dispatch the active opening, main-play or closing camera
 * mode. */
void fn_1_3684(OMOBJ *object)
{
    s16 *state = object->data;

    switch (*state) {
    case 1:
        fn_1_3714(object);
        break;
    case 2:
        fn_1_380C(object);
        break;
    case 3:
        fn_1_385C(object);
        break;
    case 4:
        fn_1_3860(object);
        break;
    case 5:
        break;
    }
}
#define CAMERA_MOTION_MODEL \
    lbl_1_bss_38->objects[9]->mdlId[0]
/* Plays the opening camera motion, then switches to mode 2 to wait for main play. */
void fn_1_3714(OMOBJ *object)
{
    M658CameraStorageView *work;
    Vec position;
    Vec up;
    Vec target;

    work = object->data;
    if (work->frame == 0) {
        Hu3DCameraMotionStart(CAMERA_MOTION_MODEL, 1);
    }
    ++work->frame;
    Hu3DCameraPosGet(1, &position, &up, &target);
    work->target = target;
    work->position = position;
    Hu3DCameraPosSetV(1, &position, &up, &target);
    if (Hu3DMotionEndCheck(CAMERA_MOTION_MODEL)) {
        work->frame = 0;
        work->mode = 2;
    }
}
/* On entry to main play, selects the empty camera mode 3 and resets its frame counter. */
void fn_1_380C(OMOBJ *obj)
{
    s16 *work = obj->data;

    if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
        work[1] = 0;
        work[0] = 3;
    }
}
void fn_1_385C(OMOBJ *object)
{
}
#define CAMERA_MOTION_MODELS \
    lbl_1_bss_38->objects[9]->mdlId
/* Transfers camera animation to the second result motion and tracks its camera vectors. */
void fn_1_3860(OMOBJ *object)
{
    M658CameraStorageView *work;
    Vec position;
    Vec up;
    Vec target;

    work = object->data;
    if (work->frame == 0) {
        Hu3DCameraMotionOff(CAMERA_MOTION_MODELS[0]);
        Hu3DCameraMotionStart(CAMERA_MOTION_MODELS[1], 1);
    }
    ++work->frame;
    Hu3DCameraPosGet(1, &position, &up, &target);
    work->target = target;
    work->position = position;
    Hu3DCameraPosSetV(1, &position, &up, &target);
}
typedef struct M658SceneParameters {
    u32 value[8];
} M658SceneParameters;
extern void fn_1_12EC(s32 player);
extern void fn_1_1238(M658Rows *work, s16 modelId);
extern void fn_1_140(OMOBJ *object, void (*callback)(OMOBJ *));
extern void fn_1_3B94(OMOBJ *object);
/* Initializes both selection rows, their shared goal marker and collision models, then
 * installs row updates. */
void fn_1_3940(OMOBJ *object)
{
    M658Rows *work = object->data;
    s32 player;
    COL_ATTRPARAM collisionParameters;
    Vec position;
    s32 sceneState;

    /* The sceneState and sceneParameters locals are set to zero but never read. */
    sceneState = 0;
    {
    M658SceneParameters sceneParameters = {{0}};
    memset(work, 0, 140);
    player = 0;
    while (player < 2) {
        fn_1_12EC(player);
        player++;
    }
    Hu3DModelAttrSet(lbl_1_bss_38->objects[4]->mdlId[0],
                     HU3D_MOTATTR_LOOP);
    fn_1_1040(10, 0, &position);
    Hu3DModelPosSetV(lbl_1_bss_38->objects[1]->mdlId[1],
                     &position);
    fn_1_1238(work, lbl_1_bss_38->objects[3]->mdlId[0]);
    fn_1_1238(work, lbl_1_bss_38->objects[3]->mdlId[1]);
    fn_1_1238(work, lbl_1_bss_38->objects[3]->mdlId[2]);
    MgActorColMapInit(work->collisionModels, 3, 40);
    MgActorColAttrParamGet(&collisionParameters, 1);
    collisionParameters.yDeviate = 0.0f;
    MgActorColAttrParamSet(&collisionParameters, 1);
    Hu3DModelLayerSet(lbl_1_bss_38->objects[2]->mdlId[0],
                      lbl_1_bss_8);
    Hu3DModelLayerSet(lbl_1_bss_38->objects[2]->mdlId[1],
                      lbl_1_bss_C);
    Hu3DModelAttrSet(lbl_1_bss_38->objects[4]->mdlId[0],
                     HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_38->objects[4]->mdlId[1],
                     HU3D_MOTATTR_SHAPE_LOOP);
    fn_1_140(object, fn_1_3B94);

    }
}
void fn_1_162C(s32 index);
/* The row object's per-frame callback updates both players' selection highlights. */
void fn_1_3B94(OMOBJ *object)
{
    fn_1_162C(0);
    fn_1_162C(1);
}
extern void fn_1_1B58(MGACTOR *actor, int workValue);
extern s32 fn_1_730(M658PlayerWork *work);
extern void *fn_1_6F70(s32 value);
extern void fn_1_1158(M658PlayerWork *work);
extern s16 fn_1_5770(void);
extern void fn_1_3F14(OMOBJ *object);
/* Sequence setup registers this as each active player's object initializer; it creates the
* character actor and installs its selection update callback. Player creation also replaces
* the landing-motion entry in the shared motion list with its alternate resource. */
void fn_1_3BC0(OMOBJ *object)
{
    M658PlayerWork *work;
    s32 cameraBit;
    u32 actionFlag;
    s32 playerNo;
    HuVecF position;

    work = object->data;
    {
    MGACTOR_PARAM actorParam = {0};
    cameraBit = 1;
    actionFlag = 50;
    playerNo = work->playerNo;
    memset(work, 0, 132);
    work->playerNo = playerNo;
    work->flags |= 2;
    work->actorFlags |= 2;

    actorParam.height = 1.5e+02f;
    actorParam.radius = 4e+01f;
    actorParam.param = playerNo;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.correctHookParam = (int)work;
    actorParam.narrowHook = 0;
    actorParam.correctHook = fn_1_1B58;
    work->player = MgPlayerCreate(
        work->playerNo, &actorParam, 2, cameraBit, actionFlag,
        lbl_1_data_38);
    work->frame = 0;

    work->characterNo = GwPlayerConf[work->playerNo].charNo;
    work->playerIndex = (s8)fn_1_730(work);
    work->state = 1;
    work->unused = 0;
    work->playerType = GwPlayerConf[work->playerNo].type;
    work->difficulty = GwPlayerConf[work->playerNo].comDif;
    work->inputDelayFrames = 0;

    work->materialColors = fn_1_6F70(work->player->actor->mdlId);
    work->player->omObj->work[0] = (u32)work->materialColors;

    MgPlayerComStkOn(work->player);
    MgPlayerModeFuncSet(work->player, lbl_1_data_44C);
    MgActorColMaskSet(work->player->actor, 1);
    Hu3DModelCameraSet(work->player->actor->mdlId, cameraBit);
    Hu3DModelShadowSet(work->player->actor->mdlId);
    CharEffectLayerSet((s16)lbl_1_bss_14);
    fn_1_1158((M658PlayerWork *)work);

    position = work->startPosition;
    position.x += lbl_1_data_24C.x;
    position.y += lbl_1_data_24C.y;
    position.z += lbl_1_data_24C.z;
    Hu3DLLightCreateV(work->player->actor->mdlId,
                      &position, &lbl_1_data_258,
                      &lbl_1_data_264[fn_1_5770()]);

    if (work->playerType != 0) {
        work->choiceRange = (M658PlayerChoiceRange *)&lbl_1_data_7C[work->difficulty];
        work->interval = (u32)work->choiceRange->maximumElapsedFrames / 6;
        work->firstLimit = work->interval;
        work->secondLimit = work->firstLimit;
        work->attemptCount = 1;
    }
    work->frame = 0;
    fn_1_140(object, fn_1_3F14);
    }
}
extern void fn_1_402C(OMOBJ *object);
extern void fn_1_47A8(OMOBJ *object);
extern void fn_1_4268(OMOBJ *object);
extern void fn_1_4390(OMOBJ *object);
extern void fn_1_465C(OMOBJ *object);
extern void fn_1_46B4(OMOBJ *object);
extern void fn_1_4918(OMOBJ *object);
extern void fn_1_4A24(OMOBJ *object);
extern void fn_1_4A98(OMOBJ *object);
extern void fn_1_4B24(OMOBJ *object);
extern void fn_1_4D1C(OMOBJ *object);
extern void fn_1_4DAC(OMOBJ *object);
extern void fn_1_4E50(OMOBJ *object);
/* The player's update callback dispatches the handler for its current selection state. */
void fn_1_3F14(OMOBJ *object)
{
    M658PlayerWork *data = object->data;
    MGACTOR *actor;

    /* This copied actor pointer is never read by the dispatcher. */
    actor = data->player->actor;
    data->actorFlags = data->flags;
    data->flags &= ~2;

    switch (data->state) {
    case 1:
        fn_1_402C(object);
        break;
    case 2:
        fn_1_47A8(object);
        break;
    case 3:
        fn_1_4268(object);
        break;
    case 4:
        fn_1_4390(object);
        break;
    case 5:
        fn_1_465C(object);
        break;
    case 6:
        fn_1_46B4(object);
        break;
    case 7:
        fn_1_4918(object);
        break;
    case 8:
        fn_1_4A24(object);
        break;
    case 9:
        fn_1_4A98(object);
        break;
    case 10:
        fn_1_4B24(object);
        break;
    case 11:
        fn_1_4D1C(object);
        break;
    case 12:
        fn_1_4DAC(object);
        break;
    case 13:
        fn_1_4E50(object);
        break;
    case 0:
        break;
    }
}
extern f32 fn_1_69B8(f32 start, f32 end, f32 t);
/* State 1 places a row-return effect and restores normal character scale after a delay.
 * Both players use the first row's return sound. */
void fn_1_402C(OMOBJ *object)
{
    M658PlayerWork *work = object->data;
    Vec position;

    if (work->frame == 0) {
        u16 *animation;

        fn_1_1040(lbl_1_data_1E4[(s32)work->playerIndex][work->action],
                  0, &position);
        animation = fn_1_1F04(1);
        if ((u32)animation != 0) {
            Hu3DModelPosSetV(((s16 *)animation)[3], &position);
        }
        work->player->actor->gravity = 1.5e+02f;
        work->rotation = 0.0f;
        fn_1_736C(&position, M658_SOUND_ROW_RETURN_PLAYER_1);
    }
    work->frame++;
    if (work->frame >= 20) {
        f32 progress = (f32)(work->frame - 20) / 2e+01f;
        Vec scale;

        if (progress >= 1.0f) {
            progress = 1.0f;
        }
        scale.x = fn_1_69B8(0.0f, 1.0f, progress);
        scale.y = fn_1_69B8(1.5f, 1.0f, progress);
        scale.z = fn_1_69B8(0.0f, 1.0f, progress);
        Hu3DModelScaleSetV((s16)work->player->actor->mdlId, &scale);
    }
}
typedef struct MgPlayer_s MGPLAYER;
void MgPlayerComStkOff(MGPLAYER *);
/* State 2 enables normal input when main play begins. Pre-setting state 3 makes the following
 * state-setter call leave frame and nextState unchanged. */
void fn_1_47A8(OMOBJ *object)
{
    M658PlayerWork *playerWork;

    playerWork = object->data;
    MgPlayerComStkOff(playerWork->player);
    playerWork->flags = (u16)(playerWork->flags | 1);
    playerWork->state = 3;
    fn_1_1D14(playerWork, 3);
}
void fn_1_4FCC(OMOBJ *);
s32 fn_1_EB0(void *);

/* Called by fn_1_3F14 in state 3 each frame; it handles input and advances a pending selection. */
void fn_1_4268(OMOBJ *object) {
    M658PlayerWork *playerWork;

    playerWork = object->data;
    if ((s32)(playerWork->flags & 1) == 0) {
        if (playerWork->player->mode == 0) {
            MgPlayerComStkOn(playerWork->player);
            fn_1_1D14(playerWork, 8);
        }
        return;
    }
    if (playerWork->playerType != 0) {
        MgPlayerPadSet(playerWork->player, 0, 0, 0, 0);
        fn_1_4FCC(object);
    }
    if ((s32)(playerWork->flags & 8) != 0) {
        playerWork->flags = (u16)(playerWork->flags & M658_PLAYER_FLAG_3_CLEAR_MASK);
        playerWork->inputState = 0;
        fn_1_1D14(playerWork, 7);
        return;
    }
    if (((s32)(playerWork->flags & 4) != 0) && playerWork->player->mode != 5) {
        playerWork->flags = (u16)(playerWork->flags & M658_PLAYER_FLAG_2_CLEAR_MASK);
        playerWork->inputState = 0;
        if ((s32)playerWork->action >= 4) {
            fn_1_EB0(playerWork);
        }
        fn_1_1D14(playerWork, 4);
    }
}
typedef struct M658PlayerPhaseWork {
    u8 unreadBytesBeforeTimer[12];
    s32 timer;
    MGPLAYER *player;
    u8 unreadBytesBeforePlayerNumber[6];
    s8 playerIndex;
    u8 unreadByteBeforeModelIndex;
    s32 modelIndex;
    u8 unreadBytesBeforeRotation[20];
    f32 rotation;
} M658PlayerPhaseWork;
/* A position prop pairs its model ID with the stored callback data before it. */
typedef struct M658PositionModel {
    u8 unreadBytesBeforeModelId[6];
    s16 modelId;
} M658PositionModel;
/* A record with four model ID slots. */
typedef struct M658ModelList {
    u8 unreadBytesBeforeModels[64];
    s16 modelId[4];
} M658ModelList;
typedef struct M658ModelGroup {
    u8 unreadBytesBeforeModels[16];
    M658ModelList *models;
    u8 unreadBytesBeforeRotationModels[16];
    M658ModelList *rotationModels;
    u8 unreadBytesBeforeMotionModels[40];
    M658ModelList *motionModels;
} M658ModelGroup;
typedef struct M658Scene {
    u8 unreadBytesBeforeLayerModels[16];
    M658ModelList *layerModels;
    u8 unreadBytesBeforeSpeedModels[20];
    M658ModelList *speedModels;
    u8 unreadBytesBeforeObjects[4];
    OMOBJ *objects[2];
    u8 unreadBytesBeforeScenePlayer[32];
    MGPLAYER *scenePlayer;
    u8 unreadBytesBeforeFlags[28];
    s16 flags;
} M658Scene;
extern void Hu3DModelScaleSetV(s16, Vec *);
extern void MgPlayerComStkOff(MGPLAYER *);
/* State 4 collapses X/Z and stretches Y before changing markers. Flag 0x40 skips to state 8
 * on entry, or preserves the current marker if set during the transition. */
void fn_1_4390(OMOBJ *object)
{
    M658PlayerPhaseWork *work = object->data;
    Vec position;
    Vec scale;
    f32 progress;
    M658PositionModel *positionModel;

    if (work->timer == 0) {
        if ((*(u16 *)work & 0x40) != 0) {
            MgPlayerComStkOn(work->player);
        fn_1_1D14((M658PlayerWork *)work, 8);
            return;
        }
        fn_1_1040(lbl_1_data_1E4[(s32)work->playerIndex][work->modelIndex], 0,
                  &position);
        positionModel = (M658PositionModel *)fn_1_1F04(0);
        Hu3DModelPosSetV(positionModel->modelId, &position);
        work->player->actor->gravity = 0.0f;
        if (work->playerIndex == 0) {
            fn_1_736C(&position, M658_SOUND_ROW_ENTRY_PLAYER_1);
        } else {
            fn_1_736C(&position, M658_SOUND_ROW_ENTRY_PLAYER_2);
        }
    }
    work->timer++;
    if (work->timer >= 20) {
        progress = (work->timer - 20) / 2e+01f;
        if (progress >= 1.0f) {
            progress = 1.0f;
        }
        scale.x = fn_1_69B8(1.0f, 0.0f, progress);
        scale.y = fn_1_69B8(1.0f, 1.5f, progress);
        scale.z = fn_1_69B8(1.0f, 0.0f, progress);
        Hu3DModelScaleSetV(work->player->actor->mdlId, &scale);
        if (1.0f == progress) {
            if ((*(u16 *)work & 0x40) == 0) {
                work->modelIndex++;
            }
            fn_1_1040(lbl_1_data_1E4[(s32)work->playerIndex][work->modelIndex], 1,
                      &position);
            MgActorPosSet(work->player->actor, &position);
            fn_1_1D14((M658PlayerWork *)work, 5);
            if (work->modelIndex >= 5) {
                fn_1_1500(work->playerIndex, 6);
                work->timer = 120;
            } else if ((*(u16 *)work & 0x40) != 0) {
                work->timer = 120;
            }
        }
    }
}
/* Called in player state 5 to count down the pause before returning to the row animation. */
void fn_1_465C(OMOBJ *obj)
{
    M658PlayerWork *playerWork = obj->data;

    playerWork->frame--;
    if (playerWork->frame <= 0) {
        fn_1_1D14((M658PlayerWork *)playerWork, 6);
    }
}
/* The player-state dispatcher returns the character to its row after the choice resolves. */
void fn_1_46B4(OMOBJ *object)
{
    M658PlayerPhaseWork *work = object->data;
    Vec position;
    Vec scale;
    f32 progress;
    M658PositionModel *positionModel;

    if (work->timer == 0) {
        fn_1_1040(lbl_1_data_1E4[(s32)work->playerIndex][work->modelIndex], 0,
                  &position);
        positionModel = (M658PositionModel *)fn_1_1F04(1);
        Hu3DModelPosSetV(positionModel->modelId, &position);
        if (work->modelIndex < 5) {
            fn_1_1500(work->playerIndex, 3);
        }
        work->player->actor->gravity = 1.5e+02f;
        work->rotation = 0.0f;
        if (work->playerIndex == 0) {
            fn_1_736C(&position, M658_SOUND_ROW_RETURN_PLAYER_1);
        } else {
            fn_1_736C(&position, M658_SOUND_ROW_RETURN_PLAYER_2);
        }
    }
    work->timer++;
    if (work->timer >= 20) {
        progress = (work->timer - 20) / 2e+01f;
        if (progress >= 1.0f) {
            progress = 1.0f;
        }
        scale.x = fn_1_69B8(0.0f, 1.0f, progress);
        scale.y = fn_1_69B8(1.5f, 1.0f, progress);
        scale.z = fn_1_69B8(0.0f, 1.0f, progress);
        Hu3DModelScaleSetV(work->player->actor->mdlId, &scale);
        if (1.0f == progress) {
            if (work->modelIndex >= 5) {
                fn_1_1D14((M658PlayerWork *)work, 8);
            } else {
                if ((*(u16 *)work & 1) != 0) {
                    MgPlayerComStkOff(work->player);
                }
                fn_1_1D14((M658PlayerWork *)work, 3);
            }
        }
    }
}
/* After a missed selection, state 7 attaches the floating effect and requests mode 10's
 * material flash, then returns to state 3 when that mode ends. */
void fn_1_4918(OMOBJ *obj)
{
    M658PlayerWork *playerWork = obj->data;
    HuVecF effectPosition;

    if (playerWork->frame == 0) {
        playerWork->itemEffect = (M658HookActor *)fn_1_1F04(2);
        playerWork->itemEffect->hostModel = playerWork->player->actor->mdlId;
        playerWork->itemEffect->hook =
            CharModelItemHookGet(playerWork->player->charNo, 2, 4);
        MgPlayerModeSet(playerWork->player, 10);
        MgActorPosGet(playerWork->player->actor, &effectPosition);
        if (playerWork->playerIndex == 0) {
            fn_1_736C(&effectPosition, M658_SOUND_ITEM_PLAYER_1);
        } else {
            fn_1_736C(&effectPosition, M658_SOUND_ITEM_PLAYER_2);
        }
    }
    playerWork->frame++;
    if (playerWork->player->mode != 10) {
        fn_1_1FEC((u16 *)playerWork->itemEffect);
        if ((playerWork->flags & 1) != 0) {
            MgPlayerComStkOff(playerWork->player);
        }
        fn_1_1D14((M658PlayerWork *)playerWork, 3);
    }
}
/* On the first state-8 update, enables stick override and selects the looping idle motion. */
void fn_1_4A24(OMOBJ *owner)
{
    M658PlayerWork *work = owner->data;

    if (work->frame == 0) {
        MgPlayerComStkOn(work->player);
        fn_1_1C98(work, 0, 5.0f, HU3D_MOTATTR_LOOP);
        work->frame += 1;
    }
}
/* On the first state-9 update, disables gravity, enables stick override and selects the
 * looping idle motion. */
void fn_1_4A98(OMOBJ *owner)
{
    M658PlayerWork *work = owner->data;

    if (work->frame == 0) {
        (work->player)->actor->gravity = 0.0f;
        MgPlayerComStkOn(work->player);
        fn_1_1C98(work, 0, 5.0f, HU3D_MOTATTR_LOOP);
        work->frame += 1;
    }
}
extern f32 fn_1_6A78(f32 start, f32 end, f32 t);
/* The winner-state handler moves and turns the character into the result pose. */
void fn_1_4B24(OMOBJ *object)
{
    M658PlayerWork *work = object->data;
    Vec position;
    Vec rotationVector;
    f32 progress;

    if (work->frame == 0) {
        /* Pause normal player updates and collision while this state drives the visible model. */
        MgPlayerDespawn(work->player);
        fn_1_1C98(work, 1, 5.0f, HU3D_MOTATTR_LOOP);
    }
    Hu3DModelPosGet((s16)work->player->actor->mdlId, &work->startPosition);
    fn_1_7244(lbl_1_bss_38->objects[8]->mdlId[0],
              "658_NOW-END", work->player->actor->mdlId);
    Hu3DModelPosGet((s16)work->player->actor->mdlId, &position);
    work->frame++;
    progress = (f32)work->frame / 15.0f;
    if (progress > 1.0f) {
        progress = 1.0f;
    }
    work->rotation = fn_1_6A78(0.0f, 1.8e+02f, progress);
    rotationVector.x = 0.0f;
    rotationVector.y = work->rotation;
    rotationVector.z = 0.0f;
    Hu3DModelRotSetV((s16)work->player->actor->mdlId, &rotationVector);
    if (1.8e+02f == work->rotation &&
        position.x == work->startPosition.x &&
        position.z == work->startPosition.z) {
        fn_1_1C98(work, 0, 4.0f, HU3D_MOTATTR_LOOP);
    }
}
/* Called in player state 11 to start the losing character's closing motion. */
void fn_1_4D1C(OMOBJ *owner)
{
    M658PlayerWork *playerWork = owner->data;

    if (playerWork->frame == 0) {
        MgPlayerComStkOn(playerWork->player);
        playerWork->rotation = 0.0f;
        MgActorRotYSet(playerWork->player->actor, playerWork->rotation);
        fn_1_1C98(playerWork, 13, 5.0f, 0);
        playerWork->frame += 1;
    }
}
/* Called in player state 12 to start the final motion before the character exits. */
void fn_1_4DAC(OMOBJ *owner)
{
    M658PlayerWork *work = owner->data;

    if (work->frame == 0) {
        work->rotation = 0.0f;
        MgActorRotYSet((work->player)->actor,
                       work->rotation);
        fn_1_1C98(work, 10, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DMotionAttrSet((work->player)->omObj->mtnId[10], 1);
        (work->frame) += 1;
    }
}

extern f64 atan2(f64 y, f64 x);
extern void fn_1_69C8(Vec *start, Vec *end, f32 t, Vec *out);

/* Called in player state 13 each update to spin and move the character along its result path. */
void fn_1_4E50(OMOBJ *object)
{
    M658PlayerWork *work = object->data;
    Vec position;
    Vec direction;
    f32 progress;

    if (work->frame == 0) {
        PSVECSubtract(&work->motionEnd, &work->motionStart, &direction);
        work->spinZ = (f32)(180.0 *
            (atan2(-(f64)direction.x, (f64)direction.y) / 3.141592653589793));
        work->spinY = 0.0f;
    }
    Hu3DModelRotSet((s16)work->player->actor->mdlId,
        0.0f, work->spinY, work->spinZ);
    work->spinY += 5.0f;
    progress = (f32)work->frame / 240.0f;
    if (progress >= 1.0f) {
        progress = 1.0f;
    }
    fn_1_69C8(&work->motionStart, &work->motionEnd, progress, &position);
    Hu3DModelPosSetV((s16)work->player->actor->mdlId, &position);
    work->frame++;
}

extern s32 fn_1_1AA0(s32 playerIndex, s32 target);
extern s32 fn_1_72D0(s32 modulus, s32 count, s32 *weights);
extern s32 frandmod(s32 modulus);

void fn_1_506C(OMOBJ *obj);
extern void fn_1_533C(void *object);
extern void fn_1_5370(OMOBJ *obj);

/* During computer-controlled selection in player state 3, advances the input delay and timing
 * state. */
void fn_1_4FCC(OMOBJ *obj)
{
    s16 *state = obj->data;

    if (state[59] != 0) {
        state[59]--;
    }
    switch (state[58]) {
    case 0:
        fn_1_506C(obj);
        break;
    case 1:
        fn_1_533C(obj);
        break;
    case 2:
        fn_1_5370(obj);
        break;
    case 3:
        break;
    }
    state[61]++;
}

/* Computer input state 0 schedules a selection attempt relative to the moving highlight. */
void fn_1_506C(OMOBJ *obj)
{
    M658PlayerWork *work = obj->data;
    s32 move = fn_1_1AA0(work->playerIndex, work->action) - 45;
    s32 forceChoice;
    s32 rowInterval;
    s32 choice;
    s32 randomSum;
    s32 elapsedReachedSecondLimit;
    s32 selectionWeight;
    s32 cursorPositionSnapshot;
    s32 cursorStepSnapshot;
    /* This local is declared but never read. */
    s32 firstRandom;
    s32 oldLimit;

    if (move < 0) {
        return;
    }
    work->flags &= M658_PLAYER_FLAGS_4_AND_5_CLEAR_MASK;
    forceChoice = 0;
    /* This flag is set when elapsed input reaches the second limit, but is never read. */
    elapsedReachedSecondLimit = 0;
    choice = move + lbl_1_bss_24->player[work->playerIndex].interval;
    if (work->elapsedInputFrames >= work->secondLimit - choice) {
        work->attemptCount++;
        if (work->attemptCount > 1) {
            forceChoice = 1;
        }
        if (work->elapsedInputFrames >= work->secondLimit) {
            elapsedReachedSecondLimit = 1;
        }
    }
    if (work->action == 4 && work->elapsedInputFrames >= work->firstLimit - choice &&
        work->attemptCount != 0) {
        forceChoice = 1;
    }
    if (forceChoice == 0) {
        randomSum = frandmod(50) + frandmod(50);
        selectionWeight = work->choiceRange->selectionWeight;
        forceChoice = randomSum <= selectionWeight;
        if (work->attemptCount == 0) {
            forceChoice = 0;
        }
    }
    if (work->action == 4 && work->attemptCount == 1 &&
        (u32)work->elapsedInputFrames < (u32)(work->choiceRange->minimumElapsedFrames - choice)) {
        forceChoice = 0;
    }
    if (forceChoice == 1) {
        work->inputDelayFrames = move;
        work->inputState = 1;
        if (work->attemptCount != 0) {
            work->attemptCount--;
        }
        oldLimit = work->firstLimit;
        work->firstLimit = work->interval * (work->action + 2);
        work->secondLimit = oldLimit;
        return;
    }
    rowInterval = lbl_1_bss_24->player[work->playerIndex].interval;
    /* These cursor snapshots are assigned but never read. */
    cursorPositionSnapshot = lbl_1_bss_24->player[work->playerIndex].position;
    cursorStepSnapshot = lbl_1_bss_24->player[work->playerIndex].step;
    switch (fn_1_72D0(100, 1, lbl_1_data_F4)) {
    case 0:
        move += rowInterval;
        work->flags |= 0x10;
        break;
    case 1:
        move += frandmod(5);
        work->flags |= 0x20;
        break;
    case 2:
        move -= rowInterval;
        break;
    case 3:
        move += rowInterval;
        break;
    }
    work->inputDelayFrames = move;
    work->inputState = 1;
}

/* Computer input state 1 waits for the scheduled attempt, then starts its 13-frame input
 * sequence. */
void fn_1_533C(void *object) {
    M658PlayerWork *playerWork;

    playerWork = ((OMOBJ *)object)->data;
    if (playerWork->inputDelayFrames <= 0) {
        playerWork->inputDelayFrames = 13;
        playerWork->inputState = 2;
    }
}

/* Computer input state 2 issues A-button input with a neutral stick, then either retries or
 * finishes when its delay expires. */
void fn_1_5370(OMOBJ *obj)
{
    M658PlayerWork *work = obj->data;

    if (work->inputDelayFrames > 3 && (work->flags & 0x10) == 0) {
        MgPlayerPadSet(work->player, 0, 0, 256, 256);
    }
    if (work->inputDelayFrames <= 0) {
        if ((work->flags & 0x30) == 0) {
            MgPlayerPadSet(work->player, 0, 0, 256, 0);
        }
        if ((work->flags & 0x30) != 0) {
            work->inputState = 0;
        } else {
            work->inputState = 3;
        }
    }
}

typedef struct M658RegistryTemplate { u32 initialWords[16]; } M658RegistryTemplate;

extern void *fn_1_65E8(u8 *buffer, s32 bytes, s32 itemBytes);
struct M658Pool;
extern void *fn_1_6680(struct M658Pool *pool);
extern void fn_1_140(OMOBJ *object, OMOBJ_FUNC update);
extern void fn_1_55FC(OMOBJ *object);

/* Builds configured item pools on the first object update, then installs their tick. */
void fn_1_5430(OMOBJ *object)
{
    M658RegistryWork *work;
    const M658RegistryDescriptor *descriptor;
    void *item;
    void *buffer;
    s32 descriptorIndex;
    s32 itemIndex;
    s32 bufferBytes;

    work = (M658RegistryWork *)object->data;
    {
        /* This zero-initialized local template is never read. */
        M658RegistryTemplate templateCopy = {{0}};
    memset(work, 0, 92);
    descriptorIndex = 0;
    for (;;) {
        descriptor = &lbl_1_data_494[descriptorIndex];
        if (descriptor->update == NULL) {
            break;
        }
        bufferBytes = (descriptor->itemBytes + 8) * descriptor->count + 16;
        buffer = HuMemDirectMallocNum(0, bufferBytes, HU_MEMNUM_OVL);
        work->entries[descriptorIndex].pool =
            fn_1_65E8(buffer, bufferBytes, descriptor->itemBytes);
        work->entries[descriptorIndex].render = descriptor->render;
        work->entries[descriptorIndex].update = descriptor->update;
        work->count = 0;
        for (itemIndex = 0; itemIndex < descriptor->count; itemIndex++) {
            item = fn_1_6680(work->entries[descriptorIndex].pool);
            if (item != NULL) {
                descriptor->update(item, 0);
                work->count++;
            }
        }
        descriptorIndex++;
    }
    fn_1_140(object, fn_1_55FC);
    }
}
