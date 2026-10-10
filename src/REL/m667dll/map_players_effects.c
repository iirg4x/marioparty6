/* Talkie Walkie scene progression, map and player control, and entry effects. */
#include "game/frand.h"
#include "dolphin/types.h"

/* Talkie Walkie scene setup, sequence phases, and player motion callbacks. */
#include <math.h>
#include "REL/m667dll/recovered.h"
#include "game/memory.h"
#include <string.h>
#include "game/mg/actman.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/mic.h"
#include "game/object.h"
#include "game/hsfex.h"
#include "datadir_enum.h"

#define MSM_STREAM_M667_TALKIE_WALKIE 73
#define M667_PLAYER_RED_RGBA_BIT_PATTERN 0xFF0000FF
#define M667_PLAYER_BLUE_RGBA_BIT_PATTERN 0x0000FFFF
#define M667_PLAYER_YELLOW_RGBA_BIT_PATTERN 0xFFFF00FF
#define M667_PLAYER_GREEN_RGBA_BIT_PATTERN 0x00FF00FF

/* Scene setup creates object-manager work records with this helper. */
void *fn_1_A0(s32 priority, u32 bytes, void (*update)(OMOBJ *))
{
    OMOBJ *object;

    object = omAddObjEx(lbl_1_bss_0, priority, 0, 0, 0, update);
    if (bytes != 0) {
        object->data = HuMemDirectMallocNum(HEAP_HEAP, bytes, HU_MEMNUM_OVL);
        memset(object->data, 0, bytes);
    } else {
        object->data = NULL;
    }
    return object->data;
}

/* Object setup uses this helper to install each object's update callback. */
void fn_1_140(OMOBJ *object, OMOBJ_FUNC callback)
{
    object->objFunc = callback;
}

/* Minigame startup creates the actor manager, selects spherical collision positioning, and
 * starts its sequence process. */
void fn_1_148(void)
{
    lbl_1_bss_0 = MgActorObjectSetup();
    MgActorColCylReset();
    HuPrcChildCreate(fn_1_19C, 100, 12288, 0, lbl_1_bss_0);
}

/* The child creates the sequence process, then advances actor simulation once per frame. */
void fn_1_19C(void)
{
    MgSeqCreate(&lbl_1_data_0);
    while (1) {
        MgActorExec();
        HuPrcVSleep();
    }
}

/* The sequence transition callback resets the scene and advances to the next mode. */
void fn_1_1C0(s16 mode, s16 frameNo)
{
    lbl_1_data_28 = -1;
    fn_1_768();
    MgSeqModeNext();
}

void fn_1_1F4(s16 mode, s16 frameNo) {}

/* The sequence starts its minigame stream when the start message reaches FX-play. */
void fn_1_1F8(s16 mode, s16 frameNo)
{
    if (lbl_1_data_28 == -1 &&
        (GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY) != 0) {
        lbl_1_data_28 = HuAudSStreamPlay(MSM_STREAM_M667_TALKIE_WALKIE);
    }
}

/* The main sequence advances after the scene determines the outcome and enters result mode 3. */
void fn_1_254(s16 mode, s16 frameNo)
{
    if (fn_1_B90() != 0) MgSeqModeNext();
}

/* The finish sequence fades the minigame stream at frame zero, then waits for scene mode 4. */
void fn_1_280(s16 mode, s16 frameNo)
{
    if (MgSeqFrameNoGet() == 0 && lbl_1_data_28 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_28, 100);
        lbl_1_data_28 = -1;
    }
    if (fn_1_BDC() != 0) MgSeqModeNext();
}

/* The pre-winner sequence advances after the ending scene reaches mode 5. */
void fn_1_2F0(s16 mode, s16 frameNo)
{
    if (fn_1_C28() != 0) MgSeqModeNext();
}

void fn_1_31C(s16 mode, s16 frameNo) {}
void fn_1_320(s16 mode, s16 frameNo) {}

/* Sequence shutdown attempts response retrieval if microphone input setup was requested,
 * discards the result, and closes microphone input. */
void fn_1_324(s16 mode, s16 frameNo)
{
    fn_1_7A4();
}

/* AI choices sum the requested random draws, then scan cumulative weights for a choice.
 * The scan
 * is unbounded; negative weights are added to the unsigned total, not treated as terminators. */
s32 fn_1_344(s32 modulus, s32 count, s32 *weights)
{
    s32 index;
    u32 random;
    u32 cumulative;

    random = 0;
    for (index = 0; index < count; index++) {
        random += frandmod(modulus);
    }
    cumulative = 0;
    for (index = 0; ; index++) {
        cumulative += weights[index];
        /* The inclusive test can select a leading zero-weight choice when the random sum is
         * zero. */
        if (cumulative >= random) {
            return index;
        }
    }
}

/* The actor manager calls this in player mode 10 after impact; its first submode starts
* looping motion 14 and enables the COM stick override. */
void fn_1_3E0(MGPLAYER *player)
{
    u16 motion;

    switch (player->subMode) {
    case 0:
        MgPlayerComStkOn(player);
        if (player->motNo != 14) {
            CharMotionShiftSet(player->charNo, player->omObj->mtnId[14],
                0.0f, 5.0f, HU3D_MOTATTR_LOOP);
            motion = 14;
            player->motNo = motion;
        }
        Hu3DMotionAttrSet(player->omObj->mtnId[14], HU3D_MOTATTR_LOOP);
        player->timer = 0.0f;
        player->subMode = 1;
    case 1:
        break;
    }
}

/* The actor manager calls this in player mode 11 after contact is lost; its first submode
* starts looping motion 15 and enables the COM stick override. */
void fn_1_4A8(MGPLAYER *player)
{
    u16 motion;

    switch (player->subMode) {
    case 0:
        MgPlayerComStkOn(player);
        if (player->motNo != 15) {
            CharMotionShiftSet(player->charNo, player->omObj->mtnId[15],
                0.0f, 4.0f, HU3D_MOTATTR_LOOP);
            motion = 15;
            player->motNo = motion;
        }
        player->timer = 0.0f;
        player->subMode = 1;
    case 1:
        break;
    }
}

/* Player mode 12 enables the COM stick override on its first submode, during the entrance. */
void fn_1_55C(MGPLAYER *player)
{
    switch (player->subMode) {
    case 0:
        MgPlayerComStkOn(player);
        player->timer = 0.0f;
        player->subMode = 1;
        /* fallthrough */
    case 1:
        break;
    }
}

/* Scene setup reads four player groups, forces a 3-to-1 split if needed, and selects the larger
 * team. */
void fn_1_5C0(M667SceneView *work)
{
    s32 counts[4];
    s32 group;
    s32 majority;
    s32 player;

    if (lbl_1_bss_2C != NULL) {
        player = 0;
        while (player < 4) {
            counts[player] = 0;
            player++;
        }
        player = 0;
        while (player < 4) {
            group = GwPlayerConf[player].grpNo;
            lbl_1_bss_2C->groups[player] = group;
            counts[group]++;
            player++;
        }
        if (((counts[0] != 3) || (counts[1] != 1)) &&
            ((counts[0] != 1) || (counts[1] != 3))) {
            counts[0] = 3;
            lbl_1_bss_2C->groups[0] = 1;
            lbl_1_bss_2C->groups[1] = 0;
            lbl_1_bss_2C->groups[2] = 0;
            counts[1] = 1;
            lbl_1_bss_2C->groups[3] = 0;
        }
        if (counts[0] > counts[1]) majority = 0;
        else majority = 1;
        lbl_1_bss_2C->majorityGroup = majority;
    }
}

/* Player actor setup reads the assigned group after scene setup; absent scene state returns
 * without supplying a group value. */
s32 fn_1_730(M667PlayerView *player)
{
    if (lbl_1_bss_2C == NULL) {
        return;
    }
    return lbl_1_bss_2C->groups[player->index];
}

/* Scene setup allocates and clears the shared scene state block. */
void fn_1_768(void)
{
    extern void *fn_1_A0(s32 priority, u32 bytes, void (*update)(OMOBJ *));
    lbl_1_bss_2C = fn_1_A0(100, 104, fn_1_2B68);
}

/* At sequence shutdown, attempt response retrieval if input setup was requested and discard
 * the result; clear the setup flag, then close microphone input in either case. */
void fn_1_7A4(void)
{
    if (lbl_1_bss_2C->micInputRequested != 0) {
        HuMCResponseGet();
        lbl_1_bss_2C->micInputRequested = 0;
    }
    HuMCClose();
}

void fn_1_25E8(void);
void fn_1_1B98(void);
void fn_1_5F74(void);
void fn_1_78F8(OMOBJ *object);
void fn_1_1C1C(s32 mode);
void *fn_1_8F58(s32 index);

/* Scene teardown stops map and effect objects, then prepares the ending model layout. */
void fn_1_7F4(void)
{
    HuVecF position;
    s32 unusedWord;
    s16 model;
    s16 playerIndex;
    s32 index;
    M667PlayerView *player;

    unusedWord = 0;
    if (lbl_1_bss_2C == NULL) {
        return;
    }

    fn_1_25E8();
    fn_1_1B98();
    fn_1_5F74();
    fn_1_78F8(lbl_1_bss_2C->mapObject);
    fn_1_78F8(lbl_1_bss_2C->entryObject);

    if (lbl_1_bss_2C->entryCount == 1) {
        fn_1_78F8(lbl_1_bss_2C->playerObject);
        lbl_1_bss_2C->singlePlayerResultObject = fn_1_7868(fn_1_8F58(5));
        model = lbl_1_bss_2C->singlePlayerResultObject->mdlId[0];
        Hu3DMotionSpeedSet(model, 0.0f);
        Hu3DMotionTimeSet(model, 0.0f);
        model = lbl_1_bss_2C->singlePlayerResultObject->mdlId[1];
        Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
        model = lbl_1_bss_2C->singlePlayerResultObject->mdlId[6];
        Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
        fn_1_1C1C(2);
        Hu3DModelShadowMapSet(lbl_1_bss_2C->singlePlayerResultObject->mdlId[3]);

        player = lbl_1_bss_2C->entries[0];
        player->rotationCurrentY = 0.0f;
        MgActorRotYSet(player->player->actor, player->rotationCurrentY);
        fn_1_D6C(8, &position);
        MgActorPosSet(player->player->actor, &position);

        for (index = 0; index < 3; index++) {
            player = lbl_1_bss_18->players[lbl_1_bss_2C->majorityGroup][index];
            Hu3DModelAttrSet(player->player->actor->mdlId, HU3D_ATTR_DISPOFF);
        }
        for (index = 0; index < 1; index++) {
            lbl_1_bss_2C->entries[index]->nextActionMode = 10;
        }
        return;
    }

    lbl_1_bss_2C->multiplayerResultObject = fn_1_7868(fn_1_8F58(6));
    /* Model zero is selected first, then immediately replaced by model two. */
    model = lbl_1_bss_2C->multiplayerResultObject->mdlId[0];
    model = lbl_1_bss_2C->multiplayerResultObject->mdlId[2];
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    fn_1_1C1C(3);
    Hu3DModelAttrSet(lbl_1_bss_18->currentPlayer->player->actor->mdlId,
        HU3D_ATTR_DISPOFF);
    for (index = 0; index < 3; index++) {
        player = lbl_1_bss_2C->entries[index];
        model = lbl_1_bss_2C->playerObject->mdlId[player->index];
        fn_1_D6C(index + 9, &position);
        Hu3DModelPosSetV(model, &position);
        playerIndex = player->index;
        fn_1_D6C(playerIndex, &position);
        MgActorPosSet(player->player->actor, &position);
        MgActorRotYSet(player->player->actor, 0.0f);
        player->nextActionMode = 10;
    }
}

/* The main sequence polls whether the scene has reached result mode 3. */
s32 fn_1_B90(void)
{
    if (lbl_1_bss_2C == NULL) return 0;
    return lbl_1_bss_2C->mode == 3;
}

/* The finish sequence polls whether the scene has reached ending mode 4. */
s32 fn_1_BDC(void)
{
    if (lbl_1_bss_2C == NULL) return 0;
    return lbl_1_bss_2C->mode == 4;
}

/* The result sequence polls whether the ending phase has completed in scene mode 5. */
s32 fn_1_C28(void)
{
    if (lbl_1_bss_2C == NULL) return 0;
    return lbl_1_bss_2C->mode == 5;
}

/* Result setup records the survivor or majority-team players before mode 3. */
s32 fn_1_C74(void *entry)
{
    if (lbl_1_bss_2C == NULL) return 0;
    if (lbl_1_bss_2C->entryCount < 3) {
        lbl_1_bss_2C->entries[lbl_1_bss_2C->entryCount] = entry;
        lbl_1_bss_2C->entryCount++;
        return 1;
    }
    return 0;
}

/* Scene transitions select a mode and reset its phase and timer. */
void fn_1_CFC(s32 mode)
{
    if (lbl_1_bss_2C != NULL && lbl_1_bss_2C->mode != mode) {
        lbl_1_bss_2C->mode = mode;
        lbl_1_bss_2C->modeTimer = 0;
        lbl_1_bss_2C->modePhase = 0;
    }
}

/* The scene object table supplies model groups used to position players and map props. */
typedef struct SceneObjectsView {
    s16 modeTimer;
    s16 mode;
    s16 modePhase;
    s16 outcome;
    OMOBJ *objects[7];
} SceneObjectsView;

/* Twelve four-byte rows; only the first three signed bytes are consumed. */
extern s8 lbl_1_data_364[12][4];
extern char *lbl_1_data_334[12];

/* Scene setup reads a named node's world translation for kind 0 or its local base position
 * for kind 1. A missing kind-0 node returns the model translation; a missing kind-1 node
 * leaves the supplied position unchanged. */
void fn_1_D6C(s32 index, HuVecF *position)
{
    Mtx matrix;
    HSF_OBJECT *object = NULL;
    s32 model;
    s32 kind;

    if (lbl_1_bss_18 != NULL) {
        (void)lbl_1_data_364[index];
        kind = lbl_1_data_364[index][0];
        model = ((SceneObjectsView *) lbl_1_bss_2C)
                    ->objects[lbl_1_data_364[index][1]]
                    ->mdlId[lbl_1_data_364[index][2]];
        switch (kind) {
        case 0:
            Hu3DModelObjMtxGet(model, lbl_1_data_334[index], matrix);
            Hu3DMtxTransGet(matrix, position);
            break;
        case 1:
            object = Hu3DModelObjPtrGet(model, lbl_1_data_334[index]);
            if (object) {
                position->x = object->mesh.base.pos.x;
                position->y = object->mesh.base.pos.y;
                position->z = object->mesh.base.pos.z;
            }
            break;
        }
    }
}
#include <math.h>
#include "REL/m667dll/recovered.h"
#include "dolphin/types.h"
#include "REL/m667dll/recovered.h"
#include "dolphin/gx/GXStruct.h"
#include "dolphin/mtx/GeoTypes.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "humath.h"

/* Each model-effect type supplies draw and update callbacks plus its pool sizing. */
typedef struct M667RegistryDescriptorView {
    void (*render)(void);
    void (*update)(M667ModelEffectView *item, s32 event);
    s32 itemBytes;
    s32 count;
} M667RegistryDescriptorView;

void fn_1_1C0(s16 mode, s16 frameNo);
void fn_1_1F4(s16 mode, s16 frameNo);
void fn_1_1F8(s16 mode, s16 frameNo);
void fn_1_254(s16 mode, s16 frameNo);
void fn_1_280(s16 mode, s16 frameNo);
void fn_1_2F0(s16 mode, s16 frameNo);
void fn_1_31C(s16 mode, s16 frameNo);
void fn_1_320(s16 mode, s16 frameNo);
void fn_1_324(s16 mode, s16 frameNo);
void fn_1_3E0(MGPLAYER *player);
void fn_1_4A8(MGPLAYER *player);
void fn_1_55C(MGPLAYER *player);
void fn_1_5D78(void);
void fn_1_6060(M667ModelEffectView *state, s32 event);
void fn_1_6064(M667ModelEffectView *effect, s32 event);
void fn_1_61A8(M667ModelEffectView *item, s32 event);
void fn_1_67D8(M667ModelEffectView *item, s32 event);
void fn_1_6844(M667ModelEffectView *item, s32 event);
void fn_1_68B0(M667ModelEffectView *item, s32 event);
void fn_1_691C(M667ModelEffectView *item, s32 event);

MGSEQ_PARAM lbl_1_data_0 = {
    60, 2, fn_1_1C0, fn_1_1F4, fn_1_1F8, fn_1_254, fn_1_280, fn_1_2F0, fn_1_31C, fn_1_320, fn_1_324
};
s32 lbl_1_data_28 = -1;
/* Player creation replaces c000m1_304 with c000m1_464 in this list, then loads the motions
* through the zero terminator. */
unsigned int lbl_1_data_30[17] ATTRIBUTE_ALIGN(8) = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 5),
    DATANUM(DATA_mariomot, 56), DATANUM(DATA_mariomot, 8),
    DATANUM(DATA_mariomot, 10), DATANUM(DATA_mariomot, 9),
    DATANUM(DATA_mariomot, 6), DATANUM(DATA_mariomot, 32),
    DATANUM(DATA_mariomot, 90), DATANUM(DATA_mariomot, 89),
    DATANUM(DATA_mariomot, 20), DATANUM(DATA_mariomot, 34)
};
f32 lbl_1_data_74[14] = {
    50.0f, 50.0f, 50.0f, 50.0f, 50.0f, 50.0f, 50.0f, 30.0f, 75.0f, 30.0f, 30.0f, 30.0f, 30.0f, 30.0f
};
f32 lbl_1_data_AC[14] = {
    12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f
};
f32 lbl_1_data_E4[14] = {
    12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f, 12.0f
};
M667AIView lbl_1_data_11C[4] = {
    {
        30,
        30,
        {20, 80, -1},
        {30, 70, -1},
        {0, 70, 30, -1, 0}
    },
    {
        40,
        40,
        {40, 60, -1},
        {45, 55, -1},
        {0, 55, 45, -1, 0}
    },
    {
        50,
        50,
        {60, 40, -1},
        {60, 40, -1},
        {0, 40, 60, -1, 0}
    },
    {
        60,
        80,
        {100, 0, -1},
        {75, 25, -1},
        {0, 25, 75, -1, 0}
    }
};
s32 lbl_1_data_1EC[5] = {8, 4, 1, 2};
HuVecF lbl_1_data_200[4] = {
    {-600.0f, 100.0f, -300.0f},
    {-600.0f, 100.0f, 300.0f},
    {600.0f, 100.0f, 300.0f},
    {600.0f, 100.0f, -300.0f}
};
HuVecF lbl_1_data_230[4][2] = {
    {{-1800.0f, 50.0f, -1300.0f}, {1800.0f, 50.0f, -1300.0f}},
    {{1800.0f, 50.0f, -1300.0f}, {-1800.0f, 50.0f, -1300.0f}},
    {{-1400.0f, -400.0f, 400.0f}, {1200.0f, -400.0f, 400.0f}},
    {{1200.0f, -400.0f, 400.0f}, {-1400.0f, -400.0f, 400.0f}}
};
s32 lbl_1_data_290[2] = {600, 540};
char lbl_1_data_298[] = "667kumo-kumo1P";
char lbl_1_data_2A7[] = "667kumo-kumo2P";
char lbl_1_data_2B6[] = "667kumo-kumo3P";
char lbl_1_data_2C5[] = "667kumo-kumo4P";
char lbl_1_data_2D4[] = "667start-1p01";
char lbl_1_data_2E2[] = "667start-3p01";
char lbl_1_data_2F0[] = "667start-1p02";
char lbl_1_data_2FE[] = "667start-3p02";
char lbl_1_data_30C[] = "667-1PEND";
char lbl_1_data_316[] = "667-2PEND";
char lbl_1_data_320[] = "667-3PEND";
char lbl_1_data_32A[] = "667-4PEND";
char *lbl_1_data_334[12] = {
    lbl_1_data_298, lbl_1_data_2A7, lbl_1_data_2B6, lbl_1_data_2C5, lbl_1_data_2D4,
    lbl_1_data_2E2, lbl_1_data_2F0, lbl_1_data_2FE, lbl_1_data_30C, lbl_1_data_316,
    lbl_1_data_320, lbl_1_data_32A
};
s8 lbl_1_data_364[12][4] = {
    {0, 2, 0, 0}, {0, 2, 1, 0}, {0, 2, 2, 0}, {0, 2, 3, 0}, {1, 4, 3, 0}, {1, 4, 3, 0},
    {1, 4, 3, 0}, {1, 4, 3, 0}, {1, 5, 2, 0}, {1, 6, 0, 0}, {1, 6, 0, 0}, {1, 6, 0, 0}
};
HuVecF lbl_1_data_394[4] = {
    { 0.0f, 2.0f, 0.0f }, { 0.0f, 5.0f, 0.0f }, { 3.0f, 5.0f, 0.0f }, { 3.0f, 2.0f, 0.0f }
};
s32 lbl_1_data_3C4[4] = {
    M667_PLAYER_RED_RGBA_BIT_PATTERN, M667_PLAYER_BLUE_RGBA_BIT_PATTERN,
    M667_PLAYER_YELLOW_RGBA_BIT_PATTERN, M667_PLAYER_GREEN_RGBA_BIT_PATTERN
};
HuVecF lbl_1_data_3D4 = {0.0f, 2000.0f, 2000.0f};
HuVecF lbl_1_data_3E0 = {-0.0f, -1.0f, -1.0f};
/* Light setup reads the initial RGBA color from the first four bytes. */
u8 lbl_1_data_3EC[32] = {
    255, 255, 255, 255, 0, 0, 0, 0, 67, 72, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 255, 255, 210, 255
};
HuVecF lbl_1_data_40C = {0.0f, 0.0f, 2000.0f};
HuVecF lbl_1_data_418 = {0.0f, 0.0f, -1.0f};
GXColor lbl_1_data_424[2] = {{255, 255, 255, 0}, {242, 234, 255, 0}};
f32 lbl_1_data_42C[4] = {120.0f, 120.0f, 120.0f, 120.0f};
f32 lbl_1_data_43C[4] = {20.0f, 20.0f, 20.0f, 100.0f};
MGPLAYER_MODE_FUNC lbl_1_data_44C[18] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, fn_1_3E0, fn_1_4A8, fn_1_55C
};
M667RegistryDescriptorView lbl_1_data_494[8] = {
    {0, fn_1_67D8, 80, 15},
    {0, fn_1_6844, 80, 15},
    {0, fn_1_68B0, 80, 15},
    {0, fn_1_691C, 80, 15},
    {0, fn_1_6060, 80, 1},
    {0, fn_1_6064, 80, 10},
    {fn_1_5D78, fn_1_61A8, 80, 64},
    {0, 0, 0, 0}
};
char lbl_1_data_514[] = "/mic/ctx/m667_words";

/* Map, player state, and effects. */
#include "REL/m667dll/completion.h"
#include "messdir_enum.h"
#include "game/mg/colman.h"
#include "dolphin/pad.h"

#define M667_ACTOR_COL_ATTR_FLAGS 0x40000010U
#define M667_DIRECTION_INPUT_MASK                                                                  \
    (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT | PAD_BUTTON_DOWN | PAD_BUTTON_UP)
#define MSM_SE_M667_ENTRY_THROW_STEP_1 2183
#define MSM_SE_M667_ENTRY_THROW_STEP_2 2184
#define MSM_SE_M667_ENTRY_THROW_STEP_3 2185
#define MSM_SE_M667_ENTRY_LAND 2186
#define M667_AI_ROUTE_GRID_FLAG (1 << 0)
#define M667_AI_ROUTE_ENTRY_FLAG (1 << 1)
#define M667_AI_STAY_FLAG (1 << 4)
#define M667_AI_Z_NEGATIVE_FLAG (1 << 0)
#define M667_AI_Z_POSITIVE_FLAG (1 << 1)
#define M667_AI_X_NEGATIVE_FLAG (1 << 2)
#define M667_AI_X_POSITIVE_FLAG (1 << 3)

extern f32 fn_1_7FA0(f32 start, f32 end, f32 t);
extern f64 fn_1_6C54(f64 value);
void fn_1_8CBC(HuVecF *position, s32 sound);
extern void fn_1_2878(s16 index);
extern f32 fn_1_7F48(HuVecF *direction);
/* The trail effect uses this bounds check before reflecting a particle back into the board. */
s32 fn_1_EC0(HuVecF *position)
{
    s32 minX;
    s32 maxX;
    s32 minY;
    s32 xUpperLimit2;

    minX = -400;
    maxX = 400;
    minY = -800;
    xUpperLimit2 = 800;
    if (position->x < minX) {
        return 0;
    }
    if (position->x > maxX) {
        return 0;
    }
    /* The upper X bound is checked again here; Y is checked against the floor below. */
    if (position->y < minY) {
        return 0;
    }
    if (position->x > xUpperLimit2) {
        return 0;
    }
    if (position->y > 0.0f) {
        return 0;
    }
    return 1;
}

extern f32 lbl_1_data_74[14];
extern HuVecF lbl_1_data_394[4];
extern s32 lbl_1_data_3C4[4];

/* Player setup groups each player; only majority-group players receive an active map slot. */
void fn_1_1000(M667PlayerView *player)
{
    HuVecF position;
    s32 unusedWord;
    s32 playerIndex;
    s32 group;
    s32 slot;

    unusedWord = 0;
    if (lbl_1_bss_18 != NULL) {
        playerIndex = player->index;
        group = player->group;
        slot = lbl_1_bss_18->groupCount[group];
        lbl_1_bss_18->players[group][slot] = player;
        if (lbl_1_bss_2C->majorityGroup == group) {
            fn_1_D6C(playerIndex, &position);
            position.y -= lbl_1_data_74[player->charNo];
            MgActorPosSet(player->player->actor, &position);
            player->rotationStartY = player->rotationCurrentY = player->rotationTargetY =
                0.0f;
            MgActorRotYSet(player->player->actor, player->rotationCurrentY);
            player->mapSlotIndex = player->index;
            fn_1_1554(player->mapSlotIndex);
            fn_1_16B8(player->mapSlotIndex, &lbl_1_data_3C4[player->index]);
            fn_1_1810(player->mapSlotIndex, (s32)lbl_1_data_394[player->index].x,
                      (s32)lbl_1_data_394[player->index].y);
        } else {
            fn_1_D6C(player->index / 2 + 4, &position);
            MgActorPosSet(player->player->actor, &position);
            if (playerIndex <= 1) {
                player->mapX = 1;
                player->mapZ = 7;
            } else {
                player->mapX = 2;
                player->mapZ = 7;
            }
            lbl_1_bss_18->currentPlayer = player;
            player->mapSlotIndex = -1;
        }
        player->slot = (s8)slot;
        lbl_1_bss_18->groupCount[group]++;
    }
}

/* Map setup records each model ID for collision-map initialization. */
void fn_1_1238(M667MapView *work, s16 modelId)
{
    work->modelIds[work->modelCount] = modelId;
    work->modelCount++;
}

extern s32 lbl_1_data_290[2];

/* Map setup initializes the two moving map models before the object starts updating them. */
void fn_1_1258(s32 index)
{
    M667MapModel *record;

    if (lbl_1_bss_18 != NULL) {
        record = &lbl_1_bss_18->records[index];
        record->model = lbl_1_bss_2C->mapObject->mdlId[index + 5];
        record->elapsedFrames = 0;
        record->index = index;
        record->durationFrames = lbl_1_data_290[index];
        Hu3DMotionSet(record->model, lbl_1_bss_2C->mapObject->mtnId[2]);
        Hu3DModelAttrSet(record->model, HU3D_MOTATTR_LOOP);
    }
}

/* The map object update advances each moving map model along its two-point path. */
void fn_1_1338(s32 index)
{
    Point3d position;
    HuVecF direction;
    M667MapModel *record;
    f32 angle;
    f32 t;
    s16 dataIndex;

    if (lbl_1_bss_18 != NULL) {
        record = &lbl_1_bss_18->records[index];
        dataIndex = record->index + (index * 2);
        if (record->elapsedFrames == 0) {
            PSVECSubtract(
                (Point3d *)((u8 *)lbl_1_data_230 + (dataIndex * 24)),
                (Point3d *)((u8 *)lbl_1_data_230 + (dataIndex * 24) + 12),
                &direction);
            angle = fn_1_7F48(&direction);
            Hu3DModelRotSet(record->model, 0.0f,
                            180.0f + angle,
                            0.0f);
        }
        record->elapsedFrames += 1;
        t = (f32)record->elapsedFrames / (f32)record->durationFrames;
        if (t >= 1.0f) {
            t = 1.0f;
        }
        fn_1_7EF0(
            (Point3d *)((u8 *)lbl_1_data_230 + (dataIndex * 24)),
            (Point3d *)((u8 *)lbl_1_data_230 + (dataIndex * 24) + 12),
            t, &position);
        Hu3DModelPosSetV(record->model, &position);
        if (t >= 1.0f) {
            record->index = (record->index + 1) % 2;
            record->elapsedFrames = 0;
        }
    }
}

/* Player setup enables the tile model assigned to a player and saves its original material
 * colors. */
void fn_1_1554(s32 index)
{
    M667MapSlot *slot;

    if (lbl_1_bss_18 != NULL) {
        slot = &lbl_1_bss_18->slots[index];
        slot->enabled = 1;
        slot->model = lbl_1_bss_2C->entryObject->mdlId[index + 16];
        Hu3DMotionSet(slot->model, lbl_1_bss_2C->entryObject->mtnId[4]);
        Hu3DModelAttrReset(slot->model, 1U);
        Hu3DModelAttrSet(slot->model, HU3D_MOTATTR_LOOP);
        slot->data = fn_1_895C((s32)slot->model);
    }
}

/* Map teardown hides a tile model, frees its saved colors, and releases the slot. */
void fn_1_1628(s32 index)
{
    M667MapSlot *slot = NULL;
    if (lbl_1_bss_18 != NULL) {
        slot = &lbl_1_bss_18->slots[index];
        if (slot->enabled != 0) {
            Hu3DModelAttrSet(slot->model, HU3D_ATTR_DISPOFF);
            HuMemDirectFree(slot->data);
            slot->data = NULL;
            slot->enabled = 0;
        }
    }
}

void fn_1_8AE4(s32 model, u8 *data, f32 red, f32 green, f32 blue);

/* Player setup tints the tile model from that player's RGB color. */
void fn_1_16B8(s32 index, void *color)
{
    M667MapSlot *slot;
    f32 red;
    f32 green;
    f32 blue;

    if (lbl_1_bss_18 != NULL) {
        slot = &lbl_1_bss_18->slots[index];
        if (slot->enabled != 0) {
            red = (f32)((GXColor *)color)->r / 255.0f;
            green = (f32)((GXColor *)color)->g / 255.0f;
            blue = (f32)((GXColor *)color)->b / 255.0f;
            fn_1_8AE4(slot->model, (u8 *)slot->data, red, green, blue);
        }
    }
}

/* Map movement places an enabled tile at its grid coordinates. */
void fn_1_1810(s32 index, s32 x, s32 z)
{
    HuVecF position;
    M667MapSlot *slot;

    if (lbl_1_bss_18 != NULL) {
        slot = &lbl_1_bss_18->slots[index];
        if (slot->enabled != 0) {
            slot->x = (s16)x;
            slot->z = (s16)z;
            position.x = (-300.0f) + (f32)(slot->x * 200);
            position.y = 2.0f;
            position.z = (-700.0f) + (f32)(slot->z * 200);
            Hu3DModelPosSetV(slot->model, &position);
        }
    }
}

#include "REL/m667dll/recovered.h"

/* AI movement reads the indexed map slot without checking whether it is enabled; absent map
 * state returns without supplying a pointer value. */
M667MapSlot *fn_1_1934(s32 index)
{
    if (lbl_1_bss_18 == NULL) {
        return;
    }
    return &lbl_1_bss_18->slots[index];
}

#include "REL/m667dll/recovered.h"

/* Player input moves a tile by one grid step when the destination is unoccupied. */
void fn_1_1968(s32 index, s32 dx, s32 dz)
{
    extern void fn_1_1810(s32 index, s32 x, s32 z);
    M667MapSlot *slot;
    s32 x;
    s32 z;
    if (lbl_1_bss_18 != NULL) {
        slot = &lbl_1_bss_18->slots[index];
        if (slot->enabled != 0) {
            x = slot->x + dx;
            z = slot->z + dz;
            if (x < 0) {
                x = 0;
            }
            if (x >= 4) {
                x = 3;
            }
            if (z < 0) {
                z = 0;
            }
            if (z >= 8) {
                z = 7;
            }
            if (fn_1_1AD4(index, x, z) == 0) {
                fn_1_1810(index, x, z);
            }
        }
    }
}

#include "REL/m667dll/recovered.h"

/* Movement handlers read the world position of an enabled tile model. */
void fn_1_1A5C(s32 index, HuVecF *position)
{
    M667MapSlot *slot;
    if (lbl_1_bss_18 != NULL) {
        slot = &lbl_1_bss_18->slots[index];
        if (slot->enabled != 0) {
            Hu3DModelPosGet(slot->model, position);
        }
    }
}

#include "REL/m667dll/recovered.h"

/* Check another enabled tile for this grid cell; absent map state or a disabled current tile
 * returns without supplying an occupancy value. */
s32 fn_1_1AD4(s32 index, s32 x, s32 z)
{
    M667MapSlot *other;
    s32 slot;
    M667MapSlot *current;

/* Only an enabled current tile is compared with the other slots. */
    if (lbl_1_bss_18 == NULL) {
        return;
    }
    current = &lbl_1_bss_18->slots[index];
    if (current->enabled == 0) {
        return;
    }
    for (slot = 0; slot < 4; slot++) {
        other = &lbl_1_bss_18->slots[slot];
        if (current != other && other->enabled != 0 &&
            other->x == x && other->z == z) {
            return 1;
        }
    }
    return 0;
}

#include "REL/m667dll/recovered.h"

/* Map teardown calls this helper to release all four player tile slots. */
void fn_1_1B98(void)
{
    extern void fn_1_1628(s32 index);
    s32 index;
    for (index = 0; index < 4; index++) {
        fn_1_1628(index);
    }
}

#include "REL/m667dll/recovered.h"
#include "game/hu3d.h"
#include "game/charman.h"

/* Set the camera callback's mode and clear its unused reset word. */
void fn_1_1BD8(s32 mode)
{
    if (lbl_1_bss_10 != NULL) {
        lbl_1_bss_10->timer = 0;
        lbl_1_bss_10->mode = mode;
    }
}

/* Scene phases start the requested camera motion after disabling the previous motion's
 * camera output. The previous model's motion clock is not paused by this switch. */
void fn_1_1C1C(s32 index)
{
    if (lbl_1_bss_10 != NULL && lbl_1_bss_10->motionIndex != index) {
        if (lbl_1_bss_10->motionIndex >= 0) {
            Hu3DCameraMotionOff(lbl_1_bss_2C->cameraObject->mdlId[lbl_1_bss_10->motionIndex]);
        }
        Hu3DCameraMotionStart(lbl_1_bss_2C->cameraObject->mdlId[index], 1);
        lbl_1_bss_10->motionIndex = index;
        lbl_1_bss_10->timer = 0;
        lbl_1_bss_10->mode = 1;
    }
}

/* Scene phases poll this helper until the selected camera motion ends. */
s32 fn_1_1D18(void)
{
    if (lbl_1_bss_10 == NULL) return 1;
    if (lbl_1_bss_10->motionIndex < 0) return 1;
    if (Hu3DMotionEndCheck(lbl_1_bss_2C->cameraObject->mdlId[lbl_1_bss_10->motionIndex]) != 0)
        return 1;
    return 0;
}

/* The actor collision hook sets bit 1 in the player's work flags when contact is corrected. */
void fn_1_1DB8(MGACTOR *actor, int param)
{
    u16 *flags = (u16 *)param;

    *flags |= 2;
}

/* Player action handlers start a new character motion only when its index changes. */
void fn_1_1DE0(M667PlayerView *work, s32 motion, f32 blend, u32 attr)
{
    if (work->motion != motion) {
        work->motion = motion;
        CharMotionShiftSet(work->charNo, work->player->omObj->mtnId[motion],
                          0.0f, blend, attr);
    }
}

#include "REL/m667dll/recovered.h"

/* Scene phases reset a player's action timer and step when selecting a different action mode. */
void fn_1_1E5C(M667PlayerView *state, s32 mode)
{
    if (state->actionMode != mode) {
        state->actionTimer = 0;
        state->actionStep = 0;
        state->actionMode = state->nextActionMode = mode;
    }
}

/* The result transition resets the action timer and restores the queued player mode. */
void fn_1_1E8C(M667PlayerView *state)
{
    state->actionTimer = 0;
    state->actionMode = state->nextActionMode;
}

/* Movement callbacks begin a turn toward the requested yaw angle. */
void fn_1_1EA0(M667PlayerView *state, f32 angle)
{
    state->rotationStartY = state->rotationCurrentY;
    state->rotationTargetY = angle;
    state->rotationTimer = 0;
}

/* Player movement callbacks advance the turn interpolation and report when it reaches its
 * target. */
s32 fn_1_1EB8(M667PlayerView *work, s16 frames)
{
    f32 t;

    work->rotationTimer += 1;
    t = (f32)work->rotationTimer / (f32)frames;
    if (t > (1.0f)) {
        t = (1.0f);
    }
    work->rotationCurrentY = fn_1_7FA0(work->rotationStartY, work->rotationTargetY, t);
    MgActorRotYSet(work->player->actor, work->rotationCurrentY);
    return t >= (1.0f);
}

#include "REL/m667dll/recovered.h"
#include "game/mic.h"

/* The microphone response stores its status, confidence, recognized-value count, and values. */
typedef struct M667MicResponse {
    s16 status;
    u16 confidence;
    s16 count;
    s16 *values;
} M667MicResponse;

/* The microphone input callback maps a confident recognized direction to player controls. */
void fn_1_1FB4(u16 *response)
{
    if (((M667MicResponse *)response)->status != 0 ||
        ((M667MicResponse *)response)->count == 0) {
        return;
    }
    if (((M667MicResponse *)response)->confidence >= 3000) {
        lbl_1_bss_2C->controlBits = 0;
        switch (*((M667MicResponse *)response)->values) {
        case 0:
            lbl_1_bss_2C->controlBits = 8;
            break;
        case 1:
            lbl_1_bss_2C->controlBits = 4;
            break;
        case 2:
            lbl_1_bss_2C->controlBits = 2;
            break;
        case 3:
            lbl_1_bss_2C->controlBits = 1;
            break;
        default:
            lbl_1_bss_2C->controlBits = 0;
            break;
        }
    }
}

#include "REL/m667dll/recovered.h"

/* Collisions and effect callbacks request a free pooled effect item at a world position. */
u16 *fn_1_2098(u32 index, HuVecF *position)
{
    M667RegistryEntryView *entry;
    u16 *item;
    void *pool;

    if (lbl_1_bss_C == NULL) {
        return NULL;
    }
    entry = &lbl_1_bss_C->entries[index];
    pool = entry->pool;
    item = fn_1_7C8C(pool);
    while (item) {
        if ((item[0] & 1) == 0) {
            item[0] |= 1;
            item[1] = 0;
            item[4] = 0;
            if (position != NULL) {
                /* A supplied world position becomes the new effect's starting point. */
                ((M667ModelEffectView *)item)->position = *position;
            }
            entry->update((M667ModelEffectView *)item, 2);
            return item;
        }
        item = fn_1_7CC8(pool, item);
    }
    return NULL;
}

#include "dolphin/types.h"

void fn_1_21B8(u16 *flags)
{
    *flags |= 2;
}

#include "REL/m667dll/recovered.h"

/* Entry collision handlers switch an entry mode and reset its timer on change. */
void fn_1_21CC(s16 index, s32 mode)
{
    M667Entry *entry;

    entry = fn_1_2230(index);
    if (entry && entry->mode != mode) {
        entry->timer = 0;
        entry->mode = mode;
    }
}

/* Entry handlers look up the indexed entry, or return null when shared state is absent.
 * The index is used without a range check. */
M667Entry *fn_1_2230(s16 index)
{
    if (lbl_1_bss_8 == NULL) return NULL;
    return &lbl_1_bss_8->entries[index];
}

#include "REL/m667dll/recovered.h"

const HuVecF lbl_1_rodata_4C = {0.0f, 0.0f, 0.0f};

/* Entry placement reserves an inactive entry owned by this player and moves only its model
 * to the origin; its actor and stored launch position are unchanged. */
s16 fn_1_2270(s32 ownerPlayerIndex)
{
    HuVecF position = lbl_1_rodata_4C;
    s16 index;
    M667Entry *entry;
    M667Entries *entries;

    entries = lbl_1_bss_8;
    for (index = 0; index < 12; index++) {
        entry = &entries->entries[index];
        if (entry->mode == 0 && entry->ownerPlayerIndex == ownerPlayerIndex) {
            entry->mode = 1;
            Hu3DModelPosSetV(entry->modelId, &position);
            return index;
        }
    }
    return -1;
}

#include "REL/m667dll/recovered.h"

/* Entry setup builds the jump curve and initializes its actor for the selected landing point. */
void fn_1_2338(s16 index, HuVecF *start, HuVecF *end, f32 height)
{
    extern M667Entry *fn_1_2230(s16 index);
    HuVecF peak;
    HuVecF delta;
    HuVecF direction;
    HuVecF startFlat;
    HuVecF endFlat;
    f32 peakY;
    f32 distance;
    f32 factor;
    M667Entry *entry;

    entry = fn_1_2230(index);
    if (entry) {
        entry->x = (end->x - (-300.0f)) / 200.0f;
        entry->z = (end->z - (-700.0f)) / 200.0f;
        factor = fn_1_80AC(start, end);
        startFlat = *start;
        endFlat = *end;
        endFlat.y = startFlat.y;
        PSVECSubtract(&endFlat, &startFlat, &delta);
        distance = PSVECMag(&delta);
        peakY = start->y + height;
        fn_1_8544(&delta, &direction);
        peak.x = start->x + direction.x * factor;
        peak.y = peakY;
        peak.z = start->z + direction.z * factor;
        fn_1_8254(&entry->curve, start, &peak, end);
        entry->position = *start;
        entry->travelDistance = 0.0f;
        entry->distancePerFrame = distance / 105.0f;
        PSVECSubtract(end, start, &entry->direction);
        entry->direction.y = 0.0f;
        fn_1_8544(&entry->direction, &entry->direction);
        entry->actor->pos = *start;
        entry->actor->push.x = 0.0f;
        entry->actor->push.y = 0.0f;
        entry->actor->push.z = 0.0f;
        entry->actor->vel.x = 0.0f;
        entry->actor->vel.y = 0.0f;
        entry->actor->vel.z = 0.0f;
        entry->mode = 2;
    }
}

#include "REL/m667dll/recovered.h"

/* Scene teardown kills the actor for every entry before its object is released. */
void fn_1_25E8(void)
{
    extern M667Entry *fn_1_2230(s16 index);
    s32 index;
    M667Entry *entry;
    for (index = 0; index < 12; index++) {
        entry = fn_1_2230(index);
        if (entry) {
            MgActorKill(entry->actor);
        }
    }
}

/* The entry update callback advances a thrown item, handles landing, and shrinks reserved
 * items released when play ends. */
void fn_1_2644(s16 entryIndex)
{
    extern M667Entry *fn_1_2230(s16 index);
    extern u16 *fn_1_2098(u32 index, HuVecF *position);
    extern void fn_1_21CC(s16 index, s32 mode);
    HuVecF position;
    M667Entry *entry;
    f32 height;
    f32 scale;
    f32 t;

    entry = fn_1_2230(entryIndex);
    if (entry) {
        switch (entry->mode) {
        case 0:
        case 1:
            break;
        case 2:
            entry->timer += 1;
            height = fn_1_84E8(&entry->curve, entry->travelDistance);
            position.x = entry->position.x + (entry->direction.x * entry->travelDistance);
            position.y = entry->position.y - height;
            position.z = entry->position.z + (entry->direction.z * entry->travelDistance);
            MgActorPosSet(entry->actor, &position);
            if (position.y < 18.0f) {
                fn_1_2098((u32)entry->ownerPlayerIndex, &position);
                position.y = 0.0f;
                fn_1_2098(5U, &position);
                fn_1_21CC(entryIndex, 3);
                fn_1_8CBC(&position, MSM_SE_M667_ENTRY_LAND);
                return;
            }
            entry->travelDistance += entry->distancePerFrame;
            return;
        case 3:
            fn_1_2878(entryIndex);
            return;
        case 4:
            entry->timer += 1;
            t = (f32)entry->timer / 60.0f;
            if (t > (1.0f)) {
                t = (1.0f);
            }
            scale = (1.0f) - t;
            Hu3DModelScaleSet(entry->modelId, scale, scale, scale);
            if (t >= (1.0f)) {
                Hu3DModelAttrSet(entry->modelId, 1U);
                entry->mode = 0;
            }
            break;
        }
    }
}

#include "REL/m667dll/recovered.h"

/* The entry update after landing or contact selects inactive mode, disables body collisions,
 * and hides the entry model. */
void fn_1_2878(s16 index)
{
    extern M667Entry *fn_1_2230(s16 index);
    extern void fn_1_21CC(s16 index, s32 mode);
    M667Entry *entry;

    entry = fn_1_2230(index);
    fn_1_21CC(index, 0);
    MgActorColAttrSet(entry->actor, 4U);
    Hu3DModelAttrSet(entry->modelId, 1U);
}

#include "REL/m667dll/recovered.h"

extern M667PlayerView *lbl_1_bss_1C[4];
void fn_1_8CBC(HuVecF *position, s32 sound);

/* Entry collision handling emits impact effects and switches the hit entry to its landing state. */
void fn_1_28D8(COLBODY *body)
{
    extern M667Entry *fn_1_2230(s16 index);
    extern u16 *fn_1_2098(u32 index, HuVecF *position);
    extern void fn_1_21CC(s16 index, s32 mode);
    HuVecF position;
    s16 index;
    M667Entry *entry;
    u16 *effect;

    index = body->param.paramB & 0xFF;
    entry = fn_1_2230(index);
    MgActorPosGet(entry->actor, &position);
    /* Both returned handles are captured, but not subsequently consumed. */
    effect = fn_1_2098(index / 3, &position);
    effect = fn_1_2098(5U, &position);
    MgActorColAttrSet(entry->actor, M667_ACTOR_COL_ATTR_FLAGS);
    MgActorColAttrReset(entry->actor, 1U);
    fn_1_21CC(index, 3);
}

/* Reject same-type or same-team pairs; apply actor and player effects only when a is an entry. */
int fn_1_2998(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    extern M667Entry *fn_1_2230(s16 index);
    extern u16 *fn_1_2098(u32 index, HuVecF *position);
    extern void fn_1_1E5C(M667PlayerView *state, s32 mode);
    extern void fn_1_21CC(s16 index, s32 mode);
    HuVecF unusedEntryOrigin;
    HuVecF playerPosition;
    HuVecF direction;
    HuVecF effectPosition;
    M667PlayerView *player;
    M667Entry *entry;
    s16 entryIndex;
    s16 playerIndex;

    if (a->type == b->type) {
        return 0;
    }
    if (((a->paramB >> 8) & 0xFF) == ((b->paramB >> 8) & 0xFF)) {
        return 0;
    }
    if (a->type == 5) {
        entryIndex = a->paramB & 0xFF;
        entry = fn_1_2230(entryIndex);
        fn_1_21CC(entryIndex, 3);
        playerIndex = b->paramB & 0xFF;
        player = lbl_1_bss_1C[playerIndex];
        MgActorColAttrSet(player->player->actor, M667_ACTOR_COL_ATTR_FLAGS);
        MgActorColAttrReset(player->player->actor, 1);
        MgActorColAttrSet(entry->actor, M667_ACTOR_COL_ATTR_FLAGS);
        MgActorColAttrReset(entry->actor, 1);
        player->player->actor->gravity = 0.0f;
        unusedEntryOrigin = entry->position;
        MgActorPosGet(player->player->actor, &playerPosition);
        direction.x = 0.0f;
        direction.y = (1.0f);
        direction.z = 0.0f;
        fn_1_8544(&direction, &player->velocity);
        fn_1_1E5C(player, 8);
        omVibrate(player->index, 20, 20, 0);
        MgActorPosGet(entry->actor, &effectPosition);
        fn_1_2098(entry->ownerPlayerIndex, &effectPosition);
        fn_1_8CBC(&effectPosition, 2186);
        CharFXPlay(player->player->charNo, 576);
    }
    return 1;
}

#include "REL/m667dll/recovered.h"
#include "game/mic.h"

extern char lbl_1_data_514[];
extern void *lbl_1_bss_14;
extern M667PlayerView *lbl_1_bss_1C[4];
void *fn_1_8F58(s32 index);
void fn_1_38E4(OMOBJ *object);
void fn_1_378C(OMOBJ *object);
void fn_1_3ACC(OMOBJ *object);
void fn_1_3D74(OMOBJ *object);
void fn_1_5AF8(OMOBJ *object);
void fn_1_6988(OMOBJ *object);
void fn_1_2D3C(OMOBJ *object);

/* The object manager loads the scene models and creates its camera, map, player, and effect
 * objects. */
void fn_1_2B68(OMOBJ *object)
{
    M667SceneView *work;
    s32 index;

    work = object->data;
    fn_1_5C0(work);
    /* This tick-derived random value is discarded; the shared random stream is not reseeded. */
    frandom(OSGetTick());
    HuMCInit(0);
    lbl_1_bss_2C->micContext = HuMCContextCreate(lbl_1_data_514);
    work->cameraObject = fn_1_7868(fn_1_8F58(0));
    work->mapObject = fn_1_7868(fn_1_8F58(4));
    work->entryObject = fn_1_7868(fn_1_8F58(3));
    work->collisionObject = fn_1_7868(fn_1_8F58(1));
    work->playerObject = fn_1_7868(fn_1_8F58(2));
    lbl_1_bss_10 = fn_1_A0(10, 100, fn_1_38E4);
    lbl_1_bss_14 = fn_1_A0(10, 16, fn_1_378C);
    lbl_1_bss_18 = fn_1_A0(10, 196, fn_1_3ACC);
    for (index = 0; index < 4; index++) {
        lbl_1_bss_1C[index] = fn_1_A0(20, 160, fn_1_3D74);
        lbl_1_bss_1C[index]->index = index;
    }
    lbl_1_bss_C = fn_1_A0(30, 108, fn_1_5AF8);
    lbl_1_bss_8 = fn_1_A0(30, 820, fn_1_6988);
    work->mode = 1;
    fn_1_140(object, fn_1_2D3C);
}

#include "REL/m667dll/recovered.h"

void fn_1_2DCC(OMOBJ *object);
void fn_1_30C4(OMOBJ *object);
void fn_1_3300(OMOBJ *object);
void fn_1_34EC(OMOBJ *object);

/* The scene object's update callback dispatches the active minigame phase. */
void fn_1_2D3C(OMOBJ *object)
{
    M667SceneView *work;

    work = object->data;
    switch (work->mode) {
    case 0:
        break;
    case 1:
        fn_1_2DCC(object);
        break;
    case 2:
        fn_1_30C4(object);
        break;
    case 3:
        fn_1_3300(object);
        break;
    case 4:
        fn_1_34EC(object);
        break;
    }
}

#include "REL/m667dll/recovered.h"
#include "game/audio.h"
#include "game/hu3d.h"
#include "game/mic.h"

extern M667PlayerView *lbl_1_bss_1C[4];

extern s32 fn_1_8E5C(s32 soundId);
extern void fn_1_1C1C(s32 mode);
extern s32 fn_1_1D18(void);
extern void fn_1_1FB4(u16 *response);

/* The opening phase starts the camera motion, player entrance, and microphone prompt. */
void fn_1_2DCC(OMOBJ *object)
{
    extern s32 fn_1_1D18(void);
    extern void fn_1_1C1C(s32 index);
    extern void fn_1_1E5C(M667PlayerView *state, s32 mode);
    M667SceneView *work;
    s32 index;
    s32 modelId;

    work = object->data;
    if (MgSeqModeGet() == 2) {
        work->soundAge += 1;
        switch (lbl_1_bss_2C->soundState) {
        case 0:
            lbl_1_bss_2C->soundHandle = fn_1_8E5C(2187);
            work->soundState = 1;
            break;
        case 1:
            if (work->soundAge >= 210) {
                HuAudFXFadeOut(work->soundHandle, 500);
                work->soundState = 2;
            }
            break;
        }
    }

    switch (work->modePhase) {
    case 0:
        fn_1_1C1C(0);
        work->modePhase += 1;
        return;
    case 1:
        if (fn_1_1D18() == 0) {
            return;
        }
        modelId = lbl_1_bss_2C->mapObject->mdlId[3];
        Hu3DMotionTimeSet(modelId, 0.0f);
        Hu3DMotionSpeedSet(modelId, (1.0f));
        fn_1_8E5C(2182);
        for (index = 0; index < 4; index++) {
            fn_1_1E5C(lbl_1_bss_1C[index], 1);
        }
        work->modePhase += 1;
        /* fallthrough */
    case 2:
        for (index = 0; index < 4; index++) {
            if ((lbl_1_bss_1C[index]->flags & 4) == 0) {
                return;
            }
        }
        fn_1_1C1C(1);
        work->modePhase += 1;
        /* fallthrough */
    case 3:
        if (MgSeqModeGet() == 5) {
            if (lbl_1_bss_18->currentPlayer->controlMode == 0) {
                HuMCListenerCreate(
                    (s16) lbl_1_bss_2C->micContext, fn_1_1FB4,
                    (u8) lbl_1_bss_18->currentPlayer->player->padNo);
                HuMCSelWinCreate((-10000.0f), (-10002.0f));
                HuMCSelWinItemSet(
                    MESSNUM(MESS_MIC_MIN_665, 13), 4,
                    lbl_1_bss_18->currentPlayer->player->padNo);
                lbl_1_bss_2C->micInputRequested = 1;
            }
            for (index = 0; index < 4; index++) {
                fn_1_1E5C(lbl_1_bss_1C[index], 2);
            }
            work->soundHandle = 0;
            work->soundState = 0;
            work->soundAge = 0;
            work->mode = 2;
        }
        break;
    }
}

#include "game/object.h"
#include "game/mg/seqman.h"
#include "REL/m667dll/recovered.h"

extern M667PlayerView *lbl_1_bss_1C[4];
s32 fn_1_C74(void *player);
void fn_1_CFC(s32 mode);

/* Scene mode 2 determines the outcome from the lone player's action or the sequence timeout.
 * Record the lone player as winner after reaching the far edge; otherwise record the majority
 * team, then enter result mode 3. */
void fn_1_30C4(OMOBJ *object)
{
    extern void fn_1_1E5C(M667PlayerView *state, s32 mode);
    s32 index;
    M667SceneView *work;

    work = object->data;
    if ((lbl_1_bss_18->currentPlayer->actionMode == 3) &&
        (lbl_1_bss_18->currentPlayer->mapZ < 0)) {
        lbl_1_bss_2C->outcome = 4;
    } else if (lbl_1_bss_18->currentPlayer->actionMode == 8) {
        lbl_1_bss_2C->outcome = 1;
    } else if (lbl_1_bss_18->currentPlayer->actionMode == 9) {
        lbl_1_bss_2C->outcome = 2;
    } else if (MgSeqModeGet() == 6) {
        lbl_1_bss_2C->outcome = 3;
    }

    if (lbl_1_bss_2C->outcome != 0) {
        if (lbl_1_bss_2C->outcome == 4) {
            fn_1_C74(lbl_1_bss_18->currentPlayer);
        } else {
            for (index = 0; index < 3; index++) {
                fn_1_C74(lbl_1_bss_18->players[lbl_1_bss_2C->majorityGroup][index]);
            }
        }
        fn_1_CFC(3);
    }

    if (work->mode == 3) {
        for (index = 0; index < 4; index++) {
            if ((lbl_1_bss_1C[index]->actionMode != 8) &&
                (lbl_1_bss_1C[index]->actionMode != 9)) {
                fn_1_1E5C(lbl_1_bss_1C[index], 7);
            }
        }
    }
}

#include "REL/m667dll/recovered.h"
#include "game/mic.h"
#include "game/wipe.h"

typedef struct ResultPhaseView {
    s16 timer;
    s16 sceneMode;
    s16 phase;
} ResultPhaseView;

extern f32 lbl_1_data_42C[4];
void fn_1_6BF0(s32 player, s16 effect);
void fn_1_CFC(s32 mode);

/* The result phase waits its outcome delay, requests a wipe, records winners and bonuses,
 * then waits a fixed 60 updates before entering the ending scene. */
void fn_1_3300(OMOBJ *object)
{
    ResultPhaseView *work;
    s32 index;
    s32 winnerIds[3];
    s32 unusedWord;

    work = object->data;
    unusedWord = 0;
    MgSeqModeChangeOff();
    switch (work->phase) {
    case 0:
        if (lbl_1_bss_2C->micInputRequested != 0) {
            HuMCListenerKill();
        }
        work->timer = lbl_1_data_42C[lbl_1_bss_2C->outcome - 1];
        work->phase++;
        break;
    case 1:
        work->timer--;
        if (work->timer <= 0) {
            WipeCreate(2, 0, 60);
            work->timer = 60;
            for (index = 0; index < 3; index++) {
                if (lbl_1_bss_2C->entries[index]) {
                    fn_1_6BF0(lbl_1_bss_2C->entries[index]->index, 10);
                    winnerIds[index] = lbl_1_bss_2C->entries[index]->charNo;
                } else {
                    winnerIds[index] = -1;
                }
            }
            MgSeqWinnerSet(winnerIds[0], winnerIds[1], winnerIds[2], -1);
            work->phase++;
        }
        break;
    case 2:
        work->timer--;
        if (work->timer <= 0) {
            MgSeqModeChangeOn();
            fn_1_CFC(4);
        }
        break;
    }
}

#include "REL/m667dll/recovered.h"
#include "game/audio.h"
#include "game/wipe.h"

extern f32 lbl_1_data_43C[4];
s32 fn_1_8E5C(s32 soundId);
void fn_1_7F4(void);
void fn_1_CFC(s32 mode);
void fn_1_1E8C(M667PlayerView *entry);

/* The ending phase waits for result timing or camera motion, then restores queued player modes. */
void fn_1_34EC(OMOBJ *object)
{
    extern void fn_1_1E8C(M667PlayerView *state);
    M667SceneView *work;
    s32 index;
    s16 model;
    s32 unusedWord;

    work = object->data;
    unusedWord = 0;
    if (lbl_1_bss_2C->singlePlayerResultObject) {
        work->soundAge++;
        switch (lbl_1_bss_2C->soundState) {
        case 0:
            lbl_1_bss_2C->soundHandle = fn_1_8E5C(2187);
            work->soundState = 1;
            break;
        case 1:
            if (work->soundAge >= 170) {
                HuAudFXFadeOut(work->soundHandle, 500);
                work->soundState = 2;
            }
            break;
        }
    }
    MgSeqModeChangeOff();
    switch (work->modePhase) {
    case 0:
        fn_1_7F4();
        WipeCreate(1, 5, 60);
        work->modeTimer = 60;
        work->modePhase++;
        break;
    case 1:
        work->modeTimer--;
        if (work->modeTimer <= 0) {
            if (lbl_1_bss_2C->singlePlayerResultObject) {
                model = lbl_1_bss_2C->singlePlayerResultObject->mdlId[0];
                Hu3DMotionTimeSet(model, 0.0f);
                Hu3DMotionSpeedSet(model, (1.0f));
            } else {
                model = lbl_1_bss_2C->multiplayerResultObject->mdlId[0];
            }
            work->modeTimer = lbl_1_data_43C[lbl_1_bss_2C->outcome - 1];
            work->modePhase++;
        }
        break;
    case 2:
        if (lbl_1_bss_2C->entryCount > 1) {
            work->modeTimer--;
            if (work->modeTimer > 0) break;
            } else if (lbl_1_bss_10->mode != 0) {
            break;
        }
        for (index = 0; index < lbl_1_bss_2C->entryCount; index++) {
            fn_1_1E8C(lbl_1_bss_2C->entries[index]);
        }
        MgSeqModeChangeOn();
        fn_1_CFC(5);
        break;
    }
}

#include <math.h>
#include "REL/m667dll/recovered.h"

typedef struct LightSetupView {
    s16 unknown_00; /* No light-object callback reads or writes this word. */
    s16 phase;
    u8 unknown_04[4]; /* No light-object callback accesses these bytes. */
    s32 lightId;
} LightSetupView;

const HuVecF lbl_1_rodata_70 = {100.0f, 8000.0f, 0.0f};
const HuVecF lbl_1_rodata_7C = {0.0f, 1.0f, 0.0f};
const HuVecF lbl_1_rodata_88 = {0.0f, 0.0f, 0.0f};
extern HuVecF lbl_1_data_3D4, lbl_1_data_3E0;
/* Light setup reads its initial RGBA color from this data block. */
extern u8 lbl_1_data_3EC[32];
void fn_1_38D0(OMOBJ *object);

/* Scene object creation configures the directional light and shadow used by the board. */
void fn_1_378C(OMOBJ *object)
{
    HuVecF lightPosition;
    HuVecF shadowUp;
    HuVecF shadowTarget;
    LightSetupView *work;

    work = object->data;
    work->lightId = Hu3DGLightCreateV(&lbl_1_data_3D4, &lbl_1_data_3E0,
        (GXColor *)lbl_1_data_3EC);
    Hu3DGLightInfinitytSet(work->lightId);
    lightPosition = lbl_1_rodata_70;
    shadowUp = lbl_1_rodata_7C;
    shadowTarget = lbl_1_rodata_88;
    Hu3DShadowMultiCreate(15.0f, 10.0f,
        10000.0f, 1);
    Hu3DShadowMultiColSet(16, 16, 16, 1);
    Hu3DShadowMultiPosSet(&lightPosition, &shadowUp, &shadowTarget, 1);
    Hu3DShadowMultiTPLvlSet(0.3f, 1);
    work->phase = 0;
    fn_1_140(object, fn_1_38D0);
}

#include "game/object.h"

/* The light object's update callback is intentionally idle after setup. */
void fn_1_38D0(OMOBJ *object)
{
    void *objectWork;
    /* Lighting and shadow setup is complete, so this object's update has no visible work. */
    objectWork = object->data;
}

#include "REL/m667dll/recovered.h"

void fn_1_39D4(OMOBJ *object);

/* Scene object creation configures the camera before its per-frame callback is installed. */
void fn_1_38E4(OMOBJ *object)
{
    M667CameraView *work;
    s16 cameraMask;

    work = object->data;
    cameraMask = 1;
    Hu3DCameraCreate(cameraMask);
    Hu3DCameraPerspectiveSet(cameraMask, 45.0f, 20.0f,
        15000.0f, 1.2f);
    Hu3DCameraViewportSet(cameraMask, 0.0f, 0.0f,
        640.0f, 480.0f, 0.0f, (1.0f));
    work->motionIndex = -1;
    work->timer = 0;
    fn_1_140(object, fn_1_39D4);
}

#include "REL/m667dll/recovered.h"

/* The camera object's update callback tracks its active motion until the motion ends. */
void fn_1_39D4(OMOBJ *object)
{
    M667CameraView *work;
    HuVecF position;
    HuVecF up;
    HuVecF target;

    work = object->data;
    switch (work->mode) {
    case 0:
        break;
    case 1:
        if (work->motionIndex < 0) {
            work->motionIndex = -1;
            work->mode = 0;
            break;
        }
        Hu3DCameraPosGet(1, &position, &up, &target);
        work->target = target;
        work->position = position;
        Hu3DCameraPosSetV(1, &position, &up, &target);
        if (Hu3DMotionEndCheck(lbl_1_bss_2C->cameraObject->mdlId[work->motionIndex])) {
            work->mode = 0;
        }
        break;
    }
}

#include "REL/m667dll/recovered.h"

/* The map initializer copies this zero-filled template into an unused local. */
typedef struct M667MapUnknownTemplate { u32 words[8]; } M667MapUnknownTemplate;
const M667MapUnknownTemplate lbl_1_rodata_BC = {{0, 0, 0, 0, 0, 0, 0, 0}};
extern HuVecF lbl_1_data_200[4];
void fn_1_1238(M667MapView *work, s16 model);
void fn_1_3D20(OMOBJ *object);

/* The map object initializer sets collision geometry, team models, and moving-map callbacks. */
void fn_1_3ACC(OMOBJ *object)
{
    extern void fn_1_1238(M667MapView *work, s16 modelId);
    extern void fn_1_1258(s32 index);
    M667MapView *work;
    s32 index;
    s32 model;
    COL_ATTRPARAM attr;
    M667MapUnknownTemplate unusedMapTemplate;
    s32 unusedWord;

    work = object->data;
    unusedWord = 0;
    unusedMapTemplate = lbl_1_rodata_BC;
    fn_1_1238(work, lbl_1_bss_2C->collisionObject->mdlId[0]);
    MgActorColMapInit(work->modelIds, lbl_1_bss_18->modelCount, 40);
    for (index = 0; index < 4; index++) {
        model = lbl_1_bss_2C->playerObject->mdlId[index];
        if (lbl_1_bss_2C->groups[index] == lbl_1_bss_2C->majorityGroup) {
            Hu3DModelPosSetV(model, &lbl_1_data_200[index]);
            Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
        } else {
            Hu3DModelAttrSet(model, 1);
        }
    }
    model = lbl_1_bss_2C->mapObject->mdlId[0];
    Hu3DModelShadowMapSet(model);
    model = lbl_1_bss_2C->mapObject->mdlId[1];
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    model = lbl_1_bss_2C->mapObject->mdlId[3];
    Hu3DMotionTimeSet(model, 0.0f);
    Hu3DMotionSpeedSet(model, 0.0f);
    Hu3DModelShadowMapSet(model);
    if (fn_1_6C44() == 0) {
        fn_1_1258(0);
        fn_1_1258(1);
    }
    MgActorColAttrParamGet(&attr, 1);
    attr.yDeviate = 0.0f;
    MgActorColAttrParamSet(&attr, 1);
    fn_1_140(object, fn_1_3D20);
}

#include "REL/m667dll/recovered.h"

/* In day mode, the map update advances its two animations before scene mode 4. */
void fn_1_3D20(OMOBJ *object)
{
    extern void fn_1_1338(s32 index);
    if (fn_1_6C44() == 0 && lbl_1_bss_2C->mode < 4) {
        fn_1_1338(0);
        fn_1_1338(1);
    }
}

#include "REL/m667dll/recovered.h"
#include "game/gamework.h"

const MGACTOR_PARAM lbl_1_rodata_DC = {0.0f, 0.0f, 0, 0, 0, 0, 0, 0};
extern unsigned int lbl_1_data_30[];
extern M667AIView lbl_1_data_11C[4];
extern MGPLAYER_MODE_FUNC lbl_1_data_44C[];
extern HuVecF lbl_1_data_40C, lbl_1_data_418;
extern GXColor lbl_1_data_424[];
s32 fn_1_730(M667PlayerView *work);
void fn_1_1DB8(MGACTOR *actor, int context);
void fn_1_1000(M667PlayerView *work);
void fn_1_4170(OMOBJ *object);

/* The player object initializer creates its actor, applies player settings, and places it on the
 * map. */
void fn_1_3D74(OMOBJ *object)
{
    MGACTOR_PARAM param;
    HuVecF position;
    M667PlayerView *work;
    s16 quarter;
    u32 camera;
    s32 group;
    s16 duration;
    u32 actionFlags;

    work = object->data;
    param = lbl_1_rodata_DC;
    group = fn_1_730(work);
    camera = 1;
    actionFlags = 32;
    work->flags |= 2;
    work->flags2 |= 2;
    param.height = 150.0f;
    param.radius = 40.0f;
    param.param = (group << 8) | work->index;
    param.type = 0;
    param.attr = 0;
    param.correctHookParam = (int)work;
    param.narrowHook = NULL;
    param.correctHook = fn_1_1DB8;
    work->player = MgPlayerCreate(work->index, &param, 2, camera, actionFlags, lbl_1_data_30);
    work->actionTimer = 0;
    work->charNo = GwPlayerConf[work->index].charNo;
    work->group = group;
    work->difficulty = GwPlayerConf[work->index].comDif;
    work->actionMode = 0;
    work->reservedWord = 0;
    work->controlMode = GwPlayerConf[work->index].type;
    work->difficulty = GwPlayerConf[work->index].comDif;
    work->aiMoveInterval = 0;
    MgPlayerComStkOn(work->player);
    MgPlayerModeFuncSet(work->player, lbl_1_data_44C);
    MgActorColMaskSet(work->player->actor, 1);
    Hu3DModelCameraSet(work->player->actor->mdlId, camera);
    Hu3DModelShadowSet(work->player->actor->mdlId);
    fn_1_1000(work);
    /* Both team branches set zero gravity, so every player receives the same value. */
    if (lbl_1_bss_2C->majorityGroup == work->group) {
        work->player->actor->gravity = 0.0f;
    } else {
        work->player->actor->gravity = 0.0f;
    }
    position = work->position;
    position.x += lbl_1_data_40C.x;
    position.y += lbl_1_data_40C.y;
    position.z += lbl_1_data_40C.z;
    Hu3DLLightCreateV(work->player->actor->mdlId, &position, &lbl_1_data_418, lbl_1_data_424);
    if (work->controlMode != 0) {
        work->ai = &lbl_1_data_11C[work->difficulty];
        if (lbl_1_bss_2C->majorityGroup == work->group) {
            duration = 1800 / (work->ai->actionCadence + 1);
            quarter = duration / 4;
            work->aiMoveInterval = quarter + quarter * lbl_1_bss_2C->aiPlayerIndex;
            work->aiActionInterval = duration;
            work->aiIntervalVariance = quarter;
            work->aiMoveTimer = frandmod(5) + 5;
            work->aiAttackTimer = work->aiIntervalVariance * frandmod(4);
            lbl_1_bss_2C->aiPlayerIndex++;
        } else {
            work->aiActionInterval = 0;
            work->aiIntervalVariance = 0;
        }
    }
    work->actionTimer = 0;
    fn_1_140(object, fn_1_4170);
}

#include "REL/m667dll/recovered.h"

void fn_1_8C30(s32 model, char *name, s32 target);
void fn_1_42CC(OMOBJ *object);
void fn_1_47A8(OMOBJ *object);
void fn_1_481C(OMOBJ *object);
void fn_1_4990(OMOBJ *object);
void fn_1_4C44(OMOBJ *object);
void fn_1_4E6C(OMOBJ *object);
void fn_1_5110(OMOBJ *object);
void fn_1_51DC(OMOBJ *object);
void fn_1_5330(OMOBJ *object);
void fn_1_5458(OMOBJ *object);

/* The player object update callback dispatches the state handler for its current action mode. */
void fn_1_4170(OMOBJ *object)
{
    extern M667Entry *fn_1_2230(s16 index);
    HuVecF position;
    MGACTOR *actor;
    M667PlayerView *work;
    M667Entry *entry;

    work = object->data;
    actor = work->player->actor;
    if ((work->flags & 8) != 0 && work->entryId >= 0) {
        /* Keep a reserved entry model attached to the player's character model. */
        entry = fn_1_2230(work->entryId);
        fn_1_8C30(work->player->actor->mdlId, work->itemHook, entry->modelId);
        Hu3DModelPosGet(entry->modelId, &position);
        MgActorPosSet(entry->actor, &position);
    }
    switch (work->actionMode) {
    case 0: break;
    case 1: fn_1_42CC(object); break;
    case 2: fn_1_47A8(object); break;
    case 4: fn_1_481C(object); break;
    case 3: fn_1_4990(object); break;
    case 5: fn_1_4C44(object); break;
    case 6: fn_1_4E6C(object); break;
    case 7: fn_1_5110(object); break;
    case 8: fn_1_51DC(object); break;
    case 9: fn_1_5330(object); break;
    case 10: fn_1_5458(object); break;
    }
    if ((work->flags & 1) != 0) {
        work->flags &= ~2;
    }
}

#include "REL/m667dll/completion.h"

extern void fn_1_1DE0(M667PlayerView *work, s32 motion, f32 blend, u32 attr);
extern void fn_1_1EA0(M667PlayerView *work, f32 angle);
extern f32 fn_1_7F48(HuVecF *direction);

/* Action mode 1 moves the lone player into position; tile players turn toward that player,
 * then select their side's fixed yaw after the entrance finishes. */
void fn_1_42CC(OMOBJ *object)
{
    extern s32 fn_1_1EB8(M667PlayerView *work, s16 frames);
    extern void fn_1_1DE0(M667PlayerView *work, s32 motion, f32 blend, u32 attr);
    extern void fn_1_1EA0(M667PlayerView *state, f32 angle);
    HuVecF position;
    HuVecF delta;
    HuVecF currentPosition;
    HuVecF playerPosition;
    HuVecF direction;
    f32 angle;
    s32 result;
    M667PlayerView *work;

    work = object->data;
    if ((work->flags & 4) != 0) {
        return;
    }
    if (work->mapSlotIndex < 0) {
        switch (work->actionStep) {
        case 0:
            MgPlayerModeSet(work->player, 12);
            MgActorColAttrSet(work->player->actor, COLBODY_ATTR_COL_OFF);
            fn_1_D6C(work->index / 2 + 4,
                     &work->startPos);
            fn_1_D6C(work->index / 2 + 6,
                     &work->targetPos);
            work->player->actor->gravity = 0.0f;
            work->targetPos.y = 0.0f;
            MgActorPosSet(work->player->actor, &work->startPos);
            work->rotationCurrentY = (180.0f);
            MgActorRotYSet(work->player->actor, work->rotationCurrentY);
            fn_1_1DE0(work, 2, 4.0f, HU3D_MOTATTR_LOOP);
            work->actionStep += 1;
            return;
        case 1:
            MgActorPosGet(work->player->actor, &position);
            PSVECSubtract(&work->targetPos, &position, &delta);
            if (PSVECMag(&delta) > 10.0f) {
                fn_1_8544(&delta, &delta);
                position.x += 10.0f * delta.x;
                position.y += 10.0f * delta.y;
                position.z += 10.0f * delta.z;
                MgActorPosSet(work->player->actor, &position);
                return;
            }
            MgActorPosSet(work->player->actor, &work->targetPos);
            work->startPos.x = work->targetPos.x;
            work->startPos.y = 0.0f;
            work->startPos.z = work->targetPos.z;
            work->targetPos.x = (-300.0f) + (f32)(work->mapX * 200);
            work->targetPos.y = 0.0f;
            work->targetPos.z = (-700.0f) + (f32)(work->mapZ * 200);
            work->player->actor->gravity = 150.0f;
            MgActorColAttrReset(work->player->actor, COLBODY_ATTR_COL_OFF);
            work->actionStep += 1;
            return;
        case 2:
            if (MgPlayerVecChase(work->player, &work->targetPos,
                                 10.0f, 5.0f) != 0) {
                MgActorPosSet(work->player->actor, &work->targetPos);
                MgPlayerModeSet(work->player, 0);
                fn_1_1DE0(work, 0, 4.0f, HU3D_MOTATTR_LOOP);
                work->flags |= 4;
                return;
            }
            break;
        }
    } else {
        switch (work->actionStep) {
        case 0:
            MgActorPosGet(lbl_1_bss_18->currentPlayer->player->actor, &currentPosition);
            MgActorPosGet(work->player->actor, &playerPosition);
            PSVECSubtract(&currentPosition, &playerPosition, &direction);
            fn_1_8544(&direction, &direction);
            angle = fn_1_7F48(&direction);
            if (work->rotationTargetY != angle) {
                fn_1_1EA0(work, angle);
            }
            result = fn_1_1EB8(work, 10);
            if ((lbl_1_bss_18->currentPlayer->flags & 4) != 0) {
                work->rotationStartY = work->rotationCurrentY;
                if (work->index <= 1) {
                    angle = 90.0f;
                } else {
                    angle = 270.0f;
                }
                fn_1_1EA0(work, angle);
                work->actionStep += 1;
                return;
            }
            break;
        case 1:
            result = fn_1_1EB8(work, 10);
            if (result != 0) {
                work->rotationStartY = work->rotationCurrentY;
                work->flags |= 4;
            }
            break;
        }
    }
}

#include "REL/m667dll/recovered.h"

/* Action mode 2 marks play active and preassigns mode 3. Tile players then switch to mode 4;
 * the lone player's mode-3 selection leaves its existing action timer and step unchanged. */
void fn_1_47A8(OMOBJ *object)
{
    extern void fn_1_1E5C(M667PlayerView *state, s32 mode);
    M667PlayerView *work = object->data;
    work->flags |= 1;
    work->actionMode = 3;
    if (work->mapSlotIndex >= 0) {
        fn_1_1E5C(work, 4);
    } else {
        fn_1_1E5C(work, 3);
    }
}

/* The player update dispatch starts the entry animation and reserves an entry for the throw. */
void fn_1_481C(OMOBJ *object)
{
    extern M667Entry *fn_1_2230(s16 index);
    extern s16 fn_1_2270(s32 ownerPlayerIndex);
    extern void fn_1_1DE0(M667PlayerView *work, s32 motion, f32 blend, u32 attr);
    extern void fn_1_1E5C(M667PlayerView *state, s32 mode);
    M667PlayerView *work;
    char *itemHook;
    M667Entry *entry;

    work = object->data;
    if (work->actionTimer == 0) {
        fn_1_1DE0(work, 11, 5.0f, 0);
        work->entryId = fn_1_2270(work->index);
        /* The lookup also runs when reservation returns -1; the later reveal assumes a valid
         * entry. */
        entry = fn_1_2230(work->entryId);
        if (work->entryId >= 0) {
            itemHook = CharModelItemHookGet(work->charNo, 2, 0);
            work->itemHook = itemHook;
            work->flags |= 8;
        }
        goto increment;
    }
    if ((f32)work->actionTimer == ((f32 *)lbl_1_data_AC)[work->charNo]) {
        entry = fn_1_2230(work->entryId);
        Hu3DModelAttrReset(entry->modelId, 1U);
    }
    if (Hu3DMotionEndCheck((s16)work->player->actor->mdlId) != 0) {
        fn_1_1DE0(work, 13, 5.0f, HU3D_MOTATTR_LOOP);
        fn_1_1E5C(work, 3);
        return;
    }
increment:
    work->actionTimer += 1;
}

#include "REL/m667dll/recovered.h"
#include "game/pad.h"

void fn_1_54E4(OMOBJ *object);
void fn_1_1968(s32 index, s32 x, s32 z);
void fn_1_1A5C(s32 index, HuVecF *position);
void fn_1_1EA0(M667PlayerView *work, f32 angle);
f32 fn_1_7F48(HuVecF *vector);

/* The player update dispatch reads controls and moves the player or its map tile. */
void fn_1_4990(OMOBJ *object)
{
    extern s32 fn_1_1EB8(M667PlayerView *work, s16 frames);
    extern void fn_1_1968(s32 index, s32 dx, s32 dz);
    extern void fn_1_1A5C(s32 index, HuVecF *position);
    extern void fn_1_1E5C(M667PlayerView *state, s32 mode);
    extern void fn_1_1EA0(M667PlayerView *state, f32 angle);
    M667PlayerView *work;
    u32 input;
    s32 moveX;
    s32 moveZ;
    HuVecF position;
    HuVecF direction;
    HuVecF target;

    work = object->data;
    input = 0;
    if (work->controlMode != 0) {
        fn_1_54E4(object);
        input = work->controlBits;
    } else if (work->mapSlotIndex >= 0) {
        input = HuPadDStkRep[work->player->padNo];
        input |= HuPadBtn[work->player->padNo] & ~M667_DIRECTION_INPUT_MASK;
    } else {
        /* Accept only newly asserted response bits, remember this response, and clear the shared
        * controls after consuming them. */
        input = lbl_1_bss_2C->controlBits & (work->controlBits ^ lbl_1_bss_2C->controlBits);
        work->controlBits = lbl_1_bss_2C->controlBits;
        lbl_1_bss_2C->controlBits = 0;
    }
    if (work->mapSlotIndex >= 0) {
        moveX = moveZ = 0;
        if (input & 2) {
            moveX = 1;
        } else if (input & 1) {
            moveX = -1;
        }
        if (input & 4) {
            moveZ = 1;
        } else if (input & 8) {
            moveZ = -1;
        }
        fn_1_1968(work->mapSlotIndex, moveX, moveZ);
        if (input & M667_DIRECTION_INPUT_MASK) {
            MgActorPosGet(work->player->actor, &position);
            fn_1_1A5C(work->mapSlotIndex, &target);
            PSVECSubtract(&target, &position, &direction);
            fn_1_8544(&direction, &direction);
            fn_1_1EA0(work, fn_1_7F48(&direction));
        }
        fn_1_1EB8(work, 10);
        if (work->entryId >= 0 && (input & PAD_BUTTON_A)) {
            fn_1_1E5C(work, 5);
            return;
        }
    } else {
        if (input & 8) {
            work->rotationCurrentY = (180.0f);
            work->mapZ--;
        } else if (input & 4) {
            work->rotationCurrentY = 0.0f;
            work->mapZ++;
        } else if (input & 1) {
            work->rotationCurrentY = 270.0f;
            work->mapX--;
        } else if (input & 2) {
            work->rotationCurrentY = 90.0f;
            work->mapX++;
        }
        if (input & M667_DIRECTION_INPUT_MASK) {
            fn_1_1E5C(work, 6);
        }
    }
}

/* The player update dispatch launches the reserved entry toward the player's tile. */
void fn_1_4C44(OMOBJ *object)
{
    extern M667Entry *fn_1_2230(s16 index);
    extern void fn_1_1A5C(s32 index, HuVecF *position);
    extern void fn_1_1DE0(M667PlayerView *work, s32 motion, f32 blend, u32 attr);
    extern void fn_1_1E5C(M667PlayerView *state, s32 mode);
    HuVecF target;
    HuVecF position;
    M667Entry *entry;
    M667PlayerView *work;

    work = object->data;
    if (work->entryId >= 0) {
        if (work->actionTimer == 0) {
            fn_1_1DE0(work, 12, 5.0f, 0);
        } else if ((f32)work->actionTimer == ((f32 *)lbl_1_data_E4)[work->charNo]) {
            work->flags &= ~8;
            entry = fn_1_2230(work->entryId);
            Hu3DModelPosGet(entry->modelId, &position);
            switch (work->slot) {
            case 0:
                fn_1_8CBC(&position, MSM_SE_M667_ENTRY_THROW_STEP_1);
                /* fall through */
            case 1:
                fn_1_8CBC(&position, MSM_SE_M667_ENTRY_THROW_STEP_2);
                /* fall through */
            case 2:
                fn_1_8CBC(&position, MSM_SE_M667_ENTRY_THROW_STEP_3);
                break;
            }
            MgActorColAttrReset(entry->actor, 4U);
            fn_1_1A5C(work->mapSlotIndex, &target);
            target.y = 0.0f;
            target.y += 18.0f;
            fn_1_2338(work->entryId, &position, &target, 350.0f);
            work->entryId = -1;
        }
    }
    if (Hu3DMotionEndCheck((s16)work->player->actor->mdlId) != 0) {
        fn_1_1DE0(work, 0, 5.0f, HU3D_MOTATTR_LOOP);
    }
    work->actionTimer += 1;
    if ((f32)work->actionTimer > 100.0f) {
        fn_1_1E5C(work, 4);
    }
}

/* Move the lone player toward the requested grid cell, then turn to face forward; loss of
 * corrected contact switches to horizontal knockback. */
void fn_1_4E6C(OMOBJ *object)
{
    extern s32 fn_1_1EB8(M667PlayerView *work, s16 frames);
    extern void fn_1_1DE0(M667PlayerView *work, s32 motion, f32 blend, u32 attr);
    extern void fn_1_1E5C(M667PlayerView *state, s32 mode);
    extern void fn_1_1EA0(M667PlayerView *state, f32 angle);
    M667PlayerView *work;
    s16 frames;
    s32 result;

    work = object->data;
    switch (work->actionStep) {
    case 0:
        work->flags |= 2;
        fn_1_1DE0(work, 2, 5.0f, HU3D_MOTATTR_LOOP);
        MgActorPosGet(work->player->actor, &work->startPos);
        work->targetPos.x = (-300.0f) + (f32)(work->mapX * 200);
        work->targetPos.y = 0.0f;
        work->targetPos.z = (-700.0f) + (f32)(work->mapZ * 200);
        work->actionStep += 1;
        /* fall through */
    case 1:
        if ((work->flags & 2) == 0) {
            fn_1_1E5C(work, 9);
            return;
        }
        if (MgPlayerVecChase(work->player, &work->targetPos,
                             10.0f, 0.0f) == 0) {
            return;
        }
        if ((180.0f) != work->rotationCurrentY) {
            fn_1_1DE0(work, 1, 4.0f, HU3D_MOTATTR_LOOP);
            fn_1_1EA0(work, (180.0f));
            work->actionStep += 1;
            return;
        }
        fn_1_1DE0(work, 0, 4.0f, HU3D_MOTATTR_LOOP);
        fn_1_1E5C(work, 3);
        return;
    case 2:
        frames = (s16)((s32)(fn_1_6C54((f64)(work->rotationStartY - (180.0f))) /
                            90.0) * 10);
        result = fn_1_1EB8(work, frames);
        if (result != 0) {
            fn_1_1DE0(work, 0, 4.0f, HU3D_MOTATTR_LOOP);
            fn_1_1E5C(work, 3);
        }
        break;
    }
}

#include "REL/m667dll/recovered.h"

void fn_1_1DE0(M667PlayerView *work, s32 motion, f32 blend, u32 attr);

/* When play ends, freeze gravity and clear throw attachment state on the first update,
 * shrinking any reserved entry still attached to the player. */
void fn_1_5110(OMOBJ *object)
{
    extern void fn_1_1DE0(M667PlayerView *work, s32 motion, f32 blend, u32 attr);
    extern void fn_1_21CC(s16 index, s32 mode);
    M667PlayerView *work;

    work = object->data;
    if (work->actionTimer == 0) {
        work->player->actor->gravity = 0.0f;
        MgPlayerComStkOn(work->player);
        fn_1_1DE0(work, 0, 5.0f, HU3D_MOTATTR_LOOP);
        if ((work->entryId >= 0) && ((work->flags & 8) != 0)) {
            fn_1_21CC(work->entryId, 4);
            work->entryId = -1;
        }
        work->flags &= ~(1 << 3);
    }
    work->actionTimer += 1;
}

#include "REL/m667dll/recovered.h"
#include "humath.h"

/* A struck player moves by its knockback velocity, then is hidden and has gravity cleared below
 * screen Y -100. */
void fn_1_51DC(OMOBJ *object)
{
    M667PlayerView *work;
    HuVecF position;
    HuVecF screenPosition;

    work = object->data;
    switch (work->actionStep) {
    case 0:
        MgPlayerModeSet(work->player, 10);
        work->actionStep += 1;
        /* fallthrough */
    case 1:
        MgActorPosGet(work->player->actor, &position);
        position.x += 40.0f * work->velocity.x;
        position.y += 40.0f * work->velocity.y;
        position.z += 40.0f * work->velocity.z;
        MgActorPosSet(work->player->actor, &position);
        Hu3D3Dto2D(&position, 1, &screenPosition);
        if (!(screenPosition.y >= (-100.0f))) {
            Hu3DModelAttrSet((s16) work->player->actor->mdlId, 1U);
            work->player->actor->gravity = 0.0f;
            work->actionStep += 1;
        }
    }
}

/* After grid movement loses corrected contact, push the lone player along the requested
 * horizontal movement direction. */
void fn_1_5330(OMOBJ *object)
{
    M667PlayerView *work;
    HuVecF direction;
    HuVecF position;

    work = object->data;
    if (work->actionTimer == 0) {
        omVibrate(work->index, 20, 20, 0);
        MgPlayerModeSet(work->player, 11);
        MgPlayerComStkOn(work->player);
        work->actionTimer += 1;
        PSVECSubtract(&work->targetPos, &work->startPos, &direction);
        fn_1_8544(&direction, &direction);
        work->velocity = direction;
    }
    MgActorPosGet(work->player->actor, &position);
    position.x += 1.5f * (10.0f * work->velocity.x);
    position.z += 1.5f * (10.0f * work->velocity.z);
    MgActorPosSet(work->player->actor, &position);
}

#include "REL/m667dll/recovered.h"

void fn_1_1DE0(M667PlayerView *work, s32 motion, f32 blend, u32 attr);

/* Action mode 10 resets yaw every update; it requests motion 10 only when the action timer is
 * zero. */
void fn_1_5458(OMOBJ *object)
{
    extern void fn_1_1DE0(M667PlayerView *work, s32 motion, f32 blend, u32 attr);
    M667PlayerView *work;

    work = object->data;
    work->rotationCurrentY = 0.0f;
    MgActorRotYSet(work->player->actor, work->rotationCurrentY);
    switch (work->actionTimer) {
    case 0:
        fn_1_1DE0(work, 10, 5.0f, 0U);
        work->actionTimer += 1;
        break;
    }
}

#include "REL/m667dll/recovered.h"

/* The AI player update routes input generation through the shared AI dispatcher. */
void fn_1_54E4(OMOBJ *object)
{
    void *objectWork;
    objectWork = object->data;
    fn_1_551C(object);
}

#include "REL/m667dll/recovered.h"

/* During action mode 3, the AI dispatcher selects tile controls for players with a map slot
 * and grid movement for the lone player without one. */
void fn_1_551C(OMOBJ *object)
{
    M667PlayerView *work = object->data;

    if (work->actionMode == 3) {
        if (work->mapSlotIndex >= 0) {
            fn_1_5898(object);
        } else {
            fn_1_5580(object);
        }
    }
}

#include "game/main.h"
#include "game/object.h"
#include "REL/m667dll/recovered.h"

s32 fn_1_344(s32 modulus, s32 count, s32 *weights);
extern s32 lbl_1_data_1EC[5];

/* fn_1_551C calls this during action mode 3 for an AI player without a map slot;
 * it builds grid movement choices and checks nearby thrown entries. */
void fn_1_5580(OMOBJ *object)
{
    M667PlayerView *player;
    s32 routeChoice;
    u32 routeMask;
    s32 routeCount;
    u32 routeCriteria;
    s32 routeIndex;
    s32 routeWeights[6];

    player = object->data;
    if (player->aiMoveInterval != 0) {
        player->aiMoveInterval -= 1;
        return;
    }
    if (player->aiActionInterval == 0) {
        routeChoice = fn_1_344(100, 1, player->ai->directionWeights);
        /* Both choices check grid bounds and airborne entries. Choice 0 removes routes when an
         * entry differs by one in the corresponding coordinate; choice 1 adds them even across
         * a grid boundary. These tests do not compare the other coordinate. */
        switch (routeChoice) {
        case 0:
            routeCriteria = M667_AI_ROUTE_GRID_FLAG | M667_AI_ROUTE_ENTRY_FLAG;
            break;
        case 1:
            routeCriteria = M667_AI_ROUTE_GRID_FLAG | M667_AI_ROUTE_ENTRY_FLAG;
            break;
        }

        routeMask = M667_AI_Z_NEGATIVE_FLAG;
        if (routeCriteria & M667_AI_ROUTE_GRID_FLAG) {
            routeMask = M667_AI_STAY_FLAG;
            if (player->mapX > 0) {
                routeMask |= M667_AI_X_NEGATIVE_FLAG;
            }
            if (player->mapX < 3) {
                routeMask |= M667_AI_X_POSITIVE_FLAG;
            }
            if (player->mapZ < 7) {
                routeMask |= M667_AI_Z_POSITIVE_FLAG;
            }
            if (player->mapZ >= 0) {
                routeMask |= M667_AI_Z_NEGATIVE_FLAG;
            }
        }
        if (routeCriteria & M667_AI_ROUTE_ENTRY_FLAG) {
            routeMask |= M667_AI_STAY_FLAG;
            routeIndex = 0;
            while (routeIndex < 12) {
                M667Entry *entry = &lbl_1_bss_8->entries[routeIndex];
                if (entry->mode == 2) {
                    if ((entry->x == player->mapX) && (entry->z == player->mapZ)) {
                        routeMask &= ~M667_AI_STAY_FLAG;
                    }
                    if (routeChoice == 0) {
                        if (entry->x == player->mapX - 1) {
                            routeMask &= ~M667_AI_X_NEGATIVE_FLAG;
                        }
                        if (entry->x == player->mapX + 1) {
                            routeMask &= ~M667_AI_X_POSITIVE_FLAG;
                        }
                        if (entry->z == player->mapZ - 1) {
                            routeMask &= ~M667_AI_Z_NEGATIVE_FLAG;
                        }
                        if (entry->z == player->mapZ + 1) {
                            routeMask &= ~M667_AI_Z_POSITIVE_FLAG;
                        }
                    } else {
                        if (entry->x == player->mapX - 1) {
                            routeMask |= M667_AI_X_NEGATIVE_FLAG;
                        }
                        if (entry->x == player->mapX + 1) {
                            routeMask |= M667_AI_X_POSITIVE_FLAG;
                        }
                        if (entry->z == player->mapZ - 1) {
                            routeMask |= M667_AI_Z_NEGATIVE_FLAG;
                        }
                        if (entry->z == player->mapZ + 1) {
                            routeMask |= M667_AI_Z_POSITIVE_FLAG;
                        }
                    }
                }
                routeIndex += 1;
            }
            if (((routeMask & M667_AI_Z_NEGATIVE_FLAG) != 0) &&
                (frandmod(100) < player->ai->chance)) {
                routeMask = M667_AI_Z_NEGATIVE_FLAG;
            }
        }

        routeCount = 0;
        routeIndex = 0;
        while (routeIndex < 5) {
            if ((routeMask & (1 << routeIndex)) != 0) {
                routeWeights[routeIndex] = 20;
                routeCount += 1;
            } else {
                routeWeights[routeIndex] = 0;
            }
            routeIndex += 1;
        }
        routeWeights[5] = -1;
        routeChoice = fn_1_344(routeCount * 20, 1, routeWeights);
        player->controlBits |= lbl_1_data_1EC[routeChoice];
        player->aiActionInterval = 1;
        return;
    }
    player->controlBits = 0;
    if (player->actionMode == 3) {
        player->aiActionInterval = 0;
        player->aiMoveInterval = 20;
    }
}

#include <math.h>
#include "REL/m667dll/completion.h"

s32 abs(s32);                                       /* extern */
M667MapSlot *fn_1_1934(s32);
s32 fn_1_344(s32, s32, s32 *);

/* fn_1_551C calls this in action mode 3 for an AI player controlling a map tile.
 * It usually targets an offset near the lone player's cell (5% direct) and checks attack
 * choices. */
void fn_1_5898(OMOBJ *object) {
    extern M667MapSlot *fn_1_1934(s32 index);
    s32 routeWeights[6];
    s32 routeResult;
    s32 distanceX;
    M667MapSlot *currentSlot;
    s32 distanceZ;
    s32 deltaX;
    s32 deltaZ;
    s32 attackChoiceCount;
    s32 weightIndex;
    M667PlayerView *player;
    player = object->data;
    player->controlBits = 0;
    player->aiMoveTimer -= 1;
    if (player->aiMoveTimer <= 0)
    {
        player->aiMoveTimer = 10;
        currentSlot = fn_1_1934(player->mapSlotIndex);
        if (5U > frandmod(100))
        {
            deltaX = lbl_1_bss_18->currentPlayer->mapX - currentSlot->x;
            deltaZ = lbl_1_bss_18->currentPlayer->mapZ - currentSlot->z;
        }
        else
        {
            deltaX = frandmod(3) + (lbl_1_bss_18->currentPlayer->mapX - 1) - currentSlot->x;
            deltaZ = ((lbl_1_bss_18->currentPlayer->mapZ - 1) - frandmod(2)) - currentSlot->z;
        }
        /* Z movement takes priority when both map coordinates differ. */
        if (deltaZ < 0)
        {
            player->controlBits = (s32)(player->controlBits | (1 << 3));
        }
        else
        {
            if (deltaZ > 0)
            {
                player->controlBits = (s32)(player->controlBits | (1 << 2));
            }
            else
            {
                if (deltaX < 0)
                {
                    player->controlBits = (s32)(player->controlBits | (1 << 0));
                }
                else
                {
                    if (deltaX > 0)
                    {
                        player->controlBits = (s32)((1 << 1) | player->controlBits);
                    }
                }
            }
        }
    }
    player->aiAttackTimer -= 1;
    if (player->aiAttackTimer <= 0)
    {
        player->aiAttackTimer =
            (s16) (frandmod(player->aiIntervalVariance / 2) +
                   (player->aiActionInterval - (player->aiIntervalVariance / 4)));
        /* When only the attack timer expires, these local deltas have not been assigned this
         * update. */
        distanceX = abs(deltaX);
        distanceZ = abs(deltaZ);
        attackChoiceCount = 0;
        if ((s32)(distanceX + distanceZ) <= 3)
        {
            attackChoiceCount += 1;
        }
        if ((s32)(distanceX + distanceZ) == 1)
        {
            attackChoiceCount += 1;
        }
        if (attackChoiceCount > 0)
        {
            weightIndex = 0;
            while (weightIndex < attackChoiceCount)
            {
                routeWeights[weightIndex] = player->ai->attackWeights[weightIndex];
                weightIndex += 1;
            }
            (&routeWeights[0])[weightIndex] = -1;
            routeResult = fn_1_344(100, 1, &routeWeights[0]);
            /* Any returned choice requests an attack; the weighted choice's identity is ignored. */
            if (routeResult >= 0)
            {
                player->controlBits = (s32)(player->controlBits | (1 << 8));
            }
        }
    }
}

#include <math.h>
#include "REL/m667dll/recovered.h"
#include "game/hu3d.h"
#include "game/memory.h"

typedef struct M667RegistryTemplate80 {
    u32 unknownWords[20]; /* The setup callback copies this template but never reads its
                           * contents. */
} M667RegistryTemplate80;

extern M667RegistryDescriptorView lbl_1_data_494[];
const M667RegistryTemplate80 lbl_1_rodata_128 = { { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };
extern void fn_1_5E64(HU3D_MODEL *model, Mtx *matrix);
extern void fn_1_5C6C(OMOBJ *object);

/* The effect object initializer creates the render hook and initializes each effect pool. */
void fn_1_5AF8(OMOBJ *object)
{
    M667RegistryTemplate80 unusedEffectTemplate;
    M667RegistryView *work;
    const M667RegistryDescriptorView *descriptor;
    M667ModelEffectView *item;
    void *buffer;
    s32 descriptorIndex;
    s32 itemIndex;
    s32 bufferBytes;

    work = object->data;
    unusedEffectTemplate = lbl_1_rodata_128;
    work->model = Hu3DHookFuncCreate(fn_1_5E64);
    descriptorIndex = 0;
    for (;;) {
        descriptor = &lbl_1_data_494[descriptorIndex];
        if (descriptor->update == NULL) {
            break;
        }
        bufferBytes = (descriptor->itemBytes + 8) * descriptor->count + 16;
        buffer = HuMemDirectMallocNum(HEAP_HEAP, bufferBytes, HU_MEMNUM_OVL);
        work->entries[descriptorIndex].pool =
            fn_1_7B10(buffer, bufferBytes, descriptor->itemBytes);
        work->entries[descriptorIndex].render = descriptor->render;
        work->entries[descriptorIndex].update = descriptor->update;
        /* Reset for each effect type, so the stored count ends with only the final pool's item
         * count. */
        work->count = 0;
        for (itemIndex = 0; itemIndex < descriptor->count; itemIndex++) {
            item = fn_1_7BA8(work->entries[descriptorIndex].pool);
            if (item != NULL) {
                descriptor->update(item, 0);
                work->count++;
            }
        }
        descriptorIndex++;
    }
    fn_1_140(object, fn_1_5C6C);
}

#include "REL/m667dll/recovered.h"

/* The effect object's update callback advances and retires active pooled effects. */
void fn_1_5C6C(OMOBJ *object)
{
    void *objectWork;
    s32 unusedWord;
    s32 index;
    u16 *item;
    void *pool;
    M667RegistryEntryView *entry;

    objectWork = object->data;
    unusedWord = 15;
    if (lbl_1_bss_2C->mode < 4) {
        index = 0;
nextEntry:
        entry = &lbl_1_bss_C->entries[index];
        if (entry->update) {
            pool = entry->pool;
            item = fn_1_7C8C(pool);
            while (item) {
                if ((*item & 2) != 0) {
                    entry->update((M667ModelEffectView *)item, 1);
                    *item &= ~3;
                }
                if ((*item & 1) != 0) {
                    entry->update((M667ModelEffectView *)item, 3);
                }
                item = fn_1_7CC8(pool, item);
            }
            index++;
            goto nextEntry;
        }
    }
}

#include "dolphin/gx.h"
#include "REL/m667dll/recovered.h"
#include "game/hu3d.h"

/* The line-effect renderer installs the GX state used to draw colored trail segments. */
void fn_1_5D78(void)
{
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(TRUE, GX_LESS, FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetChanCtrl(GX_COLOR0A0, FALSE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
}

/* The render hook checks the active flag and increments age after dispatching event 4. */
typedef struct M667RenderItemView {
    u16 flags;
    s16 age;
} M667RenderItemView;

/* The shared model hook runs each registry renderer, dispatches event 4 to active items,
* and increments their age even when their callback performs no drawing. */
void fn_1_5E64(HU3D_MODEL *model, Mtx *matrix)
{
    Mtx local;
    s32 unusedWord;
    s32 index;
    void *pool;
    M667RegistryEntryView *entry;
    M667RenderItemView *item;

    unusedWord = 15;
    PSMTXIdentity(local);
    PSMTXConcat(*matrix, local, local);
    GXLoadPosMtxImm(local, 0);
    index = 0;
nextEntry:
    entry = &lbl_1_bss_C->entries[index];
    if (entry->update) {
        if (entry->render) {
            entry->render();
        }
        pool = entry->pool;
        item = fn_1_7C8C(pool);
        while (item) {
            if ((item->flags & 1) != 0) {
                entry->update((M667ModelEffectView *)item, 4);
                item->age++;
            }
            item = fn_1_7CC8(pool, item);
        }
        index++;
        goto nextEntry;
    }
}

#include "REL/m667dll/recovered.h"
#include "game/hu3d.h"

/* Scene teardown clears remaining effect items and destroys their shared render hook. */
void fn_1_5F74(void)
{
    s32 unusedWord;
    s32 index;
    u16 *item;
    void *pool;
    M667RegistryEntryView *entry;

    unusedWord = 15;
    index = 0;
nextEntry:
    entry = &lbl_1_bss_C->entries[index];
    if (entry->update) {
        pool = entry->pool;
        item = fn_1_7C8C(pool);
        while (item) {
            if ((*item & 1) != 0) {
                entry->update((M667ModelEffectView *)item, 1);
                *item &= ~3;
            }
            item = fn_1_7CC8(pool, item);
        }
        index++;
        goto nextEntry;
    }
    Hu3DModelKill(lbl_1_bss_C->model);
}

#include "REL/m667dll/recovered.h"

/* This registered effect has no per-event work. */
void fn_1_6060(M667ModelEffectView *state, s32 event) {}

#include "REL/m667dll/recovered.h"
#include "game/memory.h"

typedef struct M667ParticlePointView {
    HuVecF position;
    GXColor color;
} M667ParticlePointView;

void fn_1_21B8(u16 *flags);

/* The particle controller callback allocates, resets, and fills its child trail list. */
void fn_1_6064(M667ModelEffectView *effect, s32 event)
{
    extern u16 *fn_1_2098(u32 index, HuVecF *position);
    extern void fn_1_21B8(u16 *flags);
    s32 capacity = 10;
    s32 i;
    M667ModelEffectView *child;

    switch (event) {
    case 0:
        effect->children =
            (M667ModelEffectView **) HuMemDirectMallocNum(
                HEAP_HEAP, capacity * sizeof(*effect->children),
                HU_MEMNUM_OVL);
        break;
    case 2:
        effect->counts.childCount = 0;
        break;
    case 3:
        if (effect->counts.childCount >= capacity) {
            for (i = 0; i < capacity; i++) {
                if (effect->children[i]->stopped == 0) {
                    break;
                }
            }
            if (i == capacity) {
                fn_1_21B8(&effect->flags);
                for (i = 0; i < capacity; i++) {
                    fn_1_21B8((u16 *) effect->children[i]);
                }
                break;
            }
        }
        /* The controller creates another child trail while capacity remains. */
        if (effect->age % 1 == 0 && effect->counts.childCount < capacity) {
            child = (M667ModelEffectView *) fn_1_2098(6U, &effect->position);
            if (child) {
                effect
                    ->children[effect->counts.childCount] = child;
                effect->counts.childCount++;
            }
        }
        break;
    }
}

#include <math.h>
#include "dolphin.h"
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
#include "game/memory.h"
#include "REL/m667dll/recovered.h"
#include <string.h>

extern s32 fn_1_EC0(HuVecF *position);
extern void fn_1_6C6C(f32 x, f32 y, f32 z);
extern void fn_1_6C60(u32 color);
extern void fn_1_6C5C(void);

/* The effect-pool callback advances and draws a bouncing, fading line trail. */
void fn_1_61A8(M667ModelEffectView *item, s32 event)
{
    extern s32 fn_1_EC0(HuVecF *position);
    s32 unusedWord = 15;
    s32 i;
    s32 count;
    u8 lineWidth;
    s32 lineOffset;
    f32 scale;
    f32 fadeFactor;

    /* A stopped trail ignores every event, including later reset and release requests. */
    if (item->stopped == 1) {
        return;
    }

    switch (event) {
    case 0:
        item->model = 0;
        item->pointCount = 3;
        item->points = (M667ParticlePointView *) HuMemDirectMallocNum(
            HEAP_HEAP, item->pointCount * 16, HU_MEMNUM_OVL);
        break;

    case 2:
        item->velocity.x =
            (-50.0f) + (f32) (u32) frandmod(100);
        item->velocity.y =
            20.0f + (f32) (u32) frandmod(25);
        item->velocity.z =
            (-50.0f) + (f32) (u32) frandmod(100);
        fn_1_8544(&item->velocity,
            &item->velocity);
        scale = 25.0f;
        item->velocity.x *= scale;
        item->velocity.y *= scale;
        item->velocity.z *= scale;

        for (i = 0; i < item->pointCount; i++) {
            item->points[i].position = item->position;
            item->points[i].color.a = 200;
            item->points[i].color.r = 200;
            item->points[i].color.g = 40;
            item->points[i].color.b = 40;
        }
        item->counts.bounceCount = 0;
        item->alpha = 200.0f;
        item->stopped = 0;
        break;

    case 3:
        memmove(item->points + 2,
            item->points + 1,
            (item->pointCount - 2) * 16);
        item->points[1] =
            item->points[0];
        PSVECAdd(&item->points[0].position,
            &item->velocity,
            &item->points[0].position);

        item->velocity.y -= 0.49f;
        item->alpha -= 7.0f;
        if (item->alpha < 0.0f) {
            item->alpha = 0.0f;
        }
        for (i = 0; i < item->pointCount; i++) {
            fadeFactor = (1.0f) - ((f32) i /
                (f32) (item->pointCount - 1));
            item->points[i].color.a =
                item->alpha * fadeFactor;
        }

        if (item->velocity.y < 0.0f) {
          if (fn_1_EC0(&item->points[0].position) != 0) {
            if (item->counts.bounceCount >= 3) {
                item->stopped = 1;
                break;
            }
            item->velocity.x *= 0.7f;
            item->velocity.y *= (-0.7f);
            item->velocity.z *= 0.7f;
            item->points[0].position.y = 0.0f;
            item->counts.bounceCount += 1;
        }
        if (item->points[0].position.y < (-500.0f)) {
            item->stopped = 1;
        }
        }
        break;

    case 4:
        lineWidth = 24;
        lineOffset = 0;
        GXSetLineWidth(lineWidth, lineOffset);
        count = item->pointCount;
        GXBegin(GX_LINESTRIP, GX_VTXFMT0, count);
        for (i = 0; i < count; i++) {
            fn_1_6C6C(
                item->points[i].position.x,
                item->points[i].position.y,
                item->points[i].position.z);
            fn_1_6C60(*(u32 *) &item->points[i].color);
        }
        fn_1_6C5C();
        break;
    }
}

#include "REL/m667dll/recovered.h"

void fn_1_21B8(u16 *flags);

/* The effect-pool callback positions, animates, and releases a temporary model effect. */
void fn_1_66AC(M667ModelEffectView *item, s32 event)
{
    extern void fn_1_21B8(u16 *flags);
    switch (event) {
    case 0:
        item->model = -1;
        break;
    case 2:
        if (item->model >= 0) {
            Hu3DModelPosSetV(item->model,
                            &item->position);
            Hu3DModelAttrReset(item->model, 1);
            Hu3DMotionTimeSet(item->model, 0.0f);
            Hu3DMotionSpeedSet(item->model, (1.0f));
        }
        break;
    case 3:
        if (item->model >= 0 &&
            Hu3DMotionEndCheck(item->model)) {
            Hu3DMotionTimeSet(item->model, 0.0f);
            Hu3DMotionSpeedSet(item->model, 0.0f);
            Hu3DModelAttrSet(item->model, 1);
            fn_1_21B8(&item->flags);
        }
        break;
    case 1:
        if (item->model >= 0) {
            Hu3DModelKill(item->model);
            item->model = -1;
        }
        break;
    }
}

/* Entry-object setup copies these zeroed actor and entry templates into local variables. */
const MGACTOR_PARAM lbl_1_rodata_194 = {0.0f, 0.0f, 0, 0, 0, 0, 0, 0};
const M667Entry lbl_1_rodata_1B4 = {
    0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 0,
    {0.0f, 0.0f, 0.0f}, {0, 0, 0, 0}
};

/* Talkie Walkie model callbacks, entry setup, and small game-state helpers. */
#include <math.h>
#include "REL/m667dll/recovered.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "dolphin/gx/GXVert.h"

M667SceneView *lbl_1_bss_2C;
M667PlayerView *lbl_1_bss_1C[4];
M667MapView *lbl_1_bss_18;
void *lbl_1_bss_14;
M667CameraView *lbl_1_bss_10;
M667RegistryView *lbl_1_bss_C;
M667Entries *lbl_1_bss_8;

/* The model-effect system calls this hook to link entry model 12 on event 2. */
void fn_1_67D8(M667ModelEffectView *effect, s32 event)
{
    switch (event) {
    case 2:
        effect->model = Hu3DModelLink(lbl_1_bss_2C->entryObject->mdlId[12]);
        break;
    }
    fn_1_66AC(effect, event);
}

/* The model-effect system calls this hook to link entry model 13 on event 2. */
void fn_1_6844(M667ModelEffectView *effect, s32 event)
{
    switch (event) {
    case 2:
        effect->model = Hu3DModelLink(lbl_1_bss_2C->entryObject->mdlId[13]);
        break;
    }
    fn_1_66AC(effect, event);
}

/* The model-effect system calls this hook to link entry model 14 on event 2. */
void fn_1_68B0(M667ModelEffectView *effect, s32 event)
{
    switch (event) {
    case 2:
        effect->model = Hu3DModelLink(lbl_1_bss_2C->entryObject->mdlId[14]);
        break;
    }
    fn_1_66AC(effect, event);
}

/* The model-effect system calls this hook to link entry model 15 on event 2. */
void fn_1_691C(M667ModelEffectView *effect, s32 event)
{
    switch (event) {
    case 2:
        effect->model = Hu3DModelLink(lbl_1_bss_2C->entryObject->mdlId[15]);
        break;
    }
    fn_1_66AC(effect, event);
}

/* Entry setup uses these actor parameters and copies the unused entry template. */
extern const MGACTOR_PARAM lbl_1_rodata_194;
extern const M667Entry lbl_1_rodata_1B4;
int fn_1_2998(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b);
void fn_1_6B84(OMOBJ *object);

/* The object manager calls this initializer to create the twelve entry actors. */
void fn_1_6988(OMOBJ *object)
{
    MGACTOR_PARAM param;
    M667Entry entryTemplate;
    M667Entry *entry;
    s32 ownerPlayerIndex;
    s32 column;
    s32 modelIndex;
    M667Entries *work;
    s32 entryIndex;

    work = object->data;
    param = lbl_1_rodata_194;
    /* This local template is copied but is not applied to the live entries. */
    entryTemplate = lbl_1_rodata_1B4;
    work->reservedWord = 0;
    modelIndex = 0;
    for (ownerPlayerIndex = 0; ownerPlayerIndex < 4; ownerPlayerIndex++) {
        for (column = 0; column < 3; column++) {
            entryIndex = column + ownerPlayerIndex * 3;
            entry = &lbl_1_bss_8->entries[entryIndex];
            entry->ownerPlayerIndex = (s16)ownerPlayerIndex;
            entry->modelId = lbl_1_bss_2C->entryObject->mdlId[modelIndex];
            entry->radius = 36.0f;
            entry->travelDistance = 0.0f;
            entry->distancePerFrame = 0.0f;
            modelIndex++;
            param.height = entry->radius;
            param.radius = entry->radius;
            param.param = (lbl_1_bss_2C->majorityGroup << 8) | entryIndex;
            param.type = 5;
            param.attr = 4;
            param.correctHookParam = (int)entry;
            param.narrowHook = fn_1_2998;
            param.correctHook = NULL;
            entry->actor = MgActorCreate(&param, entry->modelId);
            entry->actor->gravity = 0.0f;
            Hu3DModelAttrSet(entry->modelId, HU3D_ATTR_DISPOFF);
            Hu3DModelShadowSet(entry->modelId);
        }
    }
    fn_1_140(object, fn_1_6B84);
}

/* While the scene mode is below 4, the entry callback advances all twelve entries. */
void fn_1_6B84(OMOBJ *object)
{
    void *objectWork;
    s32 entryIndex;

    objectWork = object->data;
    if (lbl_1_bss_2C->mode < 4) {
        entryIndex = 0;
        while (entryIndex < 12) {
            fn_1_2644((s16)entryIndex);
            entryIndex++;
        }
    }
}

/* Result setup assigns the winner's coin bonus, replacing any previous amount unless practice mode
 * is active. */
void fn_1_6BF0(s32 player, s16 amount)
{
    if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
        GwPlayer[player].mgCoinBonus = amount;
    }
}

/* Map setup and updates read the night-mode state to enable moving map models only by day. */
s16 fn_1_6C44(void) { return GwMgNightF; }

/* Return an absolute magnitude for rotation and sound-volume calculations. */
f64 fn_1_6C54(f64 value) { return fabs(value); }

/* The trail renderer calls this after emitting its vertices; the helper performs no work. */
void fn_1_6C5C(void) {}

/* Send a packed RGBA vertex color through the FIFO during trail drawing. */
void fn_1_6C60(u32 word) { GXWGFifo.u32 = word; }
/* Send three vertex coordinates through the FIFO during immediate drawing. */
void fn_1_6C6C(f32 x, f32 y, f32 z)
{
    GXWGFifo.f32 = x;
    GXWGFifo.f32 = y;
    GXWGFifo.f32 = z;
}
