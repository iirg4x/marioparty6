/* Creates the course models and advances the opening camera and animation sequence. */
#include "dolphin/math.h"
#include "REL/m609dll/state.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/mg/seqman.h"
#include "game/mg/actman.h"
#include "game/frand.h"
#include "game/data.h"
#include "game/memory.h"

typedef struct M609SequencePlayerSlots {
    u8 beforePlayerSlots[106];
    s8 playerSlots[246];
} M609SequencePlayerSlots;

extern void fn_1_11B80(void);
extern void fn_1_52B8(void);
extern void fn_1_6974(void);
extern void fn_1_2DF4(void);
extern void fn_1_8EE4(void);
extern void fn_1_2780(void);
extern void fn_1_46F8(s16, s16, void *);
extern s32 fn_1_118BC(void);
extern s32 fn_1_30D8(f32 *value, f32 step, f32 start, f32 end);
extern f32 fn_1_31F8(f32 value);
extern void fn_1_670(void);
extern void fn_1_4F98(void);
extern void fn_1_7150(M609Model *record);
extern s32 fn_1_89E0(f32 startZ);
extern void fn_1_7E10(void);
extern void fn_1_8670(void);
extern void fn_1_8C54(void);
extern double sin(double);
extern double cos(double);
extern double sqrt(double);
extern void fn_1_3FF0(M609Player *player, s16 mode);
extern s32 fn_1_126F0(M609Player *player, s16 advanceAfterChase);

extern double atan2(double, double);

typedef struct M609CollisionEdge {
    f32 startX;
    f32 endX;
    f32 startZ;
    f32 endZ;
} M609CollisionEdge;

typedef struct M609Placement {
    s16 status;
    s16 pathIndex;
    Vec position;
    Vec movementValues;
    Vec positionStep;
    Vec movementSelectors;
    Vec bounds;
    s16 record;
    s16 linkedRecord;
} M609Placement;

#include "game/charman.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "datadir_enum.h"
void fn_1_3C0(void);
void fn_1_11E2C(void);
void fn_1_1661C(void);
void fn_1_10FC(void);
void fn_1_E4D4(void);
void fn_1_F8A8(void);
void fn_1_FDE0(void);
s32 fn_1_101AC(void);
int fn_1_C9A8(COL_NARROW_PARAM *, COL_NARROW_PARAM *);
void fn_1_CA6C(MGACTOR *,int);
void fn_1_5618(s16,s16);
void fn_1_4C68(s16,s16);
void fn_1_45EC(s32,s32);

/* Loads the day or night course models and actors from sequence initialization callback
 * fn_1_118. */
s32 fn_1_D1CC(void)
{
    u32 dataFiles[39][2] = {
        { DATANUM(DATA_m609, 0), DATANUM(DATA_m609, 1) },
        { DATANUM(DATA_m609, 2), DATANUM(DATA_m609, 2) },
        { DATANUM(DATA_m609, 3), DATANUM(DATA_m609, 3) },
        { DATANUM(DATA_m609, 4), DATANUM(DATA_m609, 5) },
        { DATANUM(DATA_m609, 6), DATANUM(DATA_m609, 7) },
        { DATANUM(DATA_m609, 8), DATANUM(DATA_m609, 8) },
        { DATANUM(DATA_m609, 9), DATANUM(DATA_m609, 9) },
        { DATANUM(DATA_m609, 10), DATANUM(DATA_m609, 10) },
        { DATANUM(DATA_m609, 11), DATANUM(DATA_m609, 11) },
        { DATANUM(DATA_m609, 12), DATANUM(DATA_m609, 12) },
        { DATANUM(DATA_m609, 13), DATANUM(DATA_m609, 13) },
        { DATANUM(DATA_m609, 14), DATANUM(DATA_m609, 14) },
        { DATANUM(DATA_m609, 15), DATANUM(DATA_m609, 15) },
        { DATANUM(DATA_m609, 16), DATANUM(DATA_m609, 16) },
        { DATANUM(DATA_m609, 17), DATANUM(DATA_m609, 17) },
        { DATANUM(DATA_m609, 18), DATANUM(DATA_m609, 18) },
        { DATANUM(DATA_m609, 19), DATANUM(DATA_m609, 19) },
        { DATANUM(DATA_m609, 20), DATANUM(DATA_m609, 20) },
        { DATANUM(DATA_m609, 21), DATANUM(DATA_m609, 21) },
        { DATANUM(DATA_m609, 22), DATANUM(DATA_m609, 22) },
        { DATANUM(DATA_m609, 23), DATANUM(DATA_m609, 23) },
        { DATANUM(DATA_m609, 24), DATANUM(DATA_m609, 24) },
        { DATANUM(DATA_m609, 25), DATANUM(DATA_m609, 25) },
        { DATANUM(DATA_m609, 26), DATANUM(DATA_m609, 26) },
        { DATANUM(DATA_m609, 27), DATANUM(DATA_m609, 27) },
        { DATANUM(DATA_m609, 28), DATANUM(DATA_m609, 28) },
        { DATANUM(DATA_m609, 29), DATANUM(DATA_m609, 29) },
        { DATANUM(DATA_m609, 30), DATANUM(DATA_m609, 30) },
        { DATANUM(DATA_m609, 31), DATANUM(DATA_m609, 31) },
        { DATANUM(DATA_m609, 32), DATANUM(DATA_m609, 32) },
        { DATANUM(DATA_m609, 33), DATANUM(DATA_m609, 33) },
        { DATANUM(DATA_m609, 34), DATANUM(DATA_m609, 34) },
        { DATANUM(DATA_m609, 35), DATANUM(DATA_m609, 35) },
        { DATANUM(DATA_m609, 36), DATANUM(DATA_m609, 36) },
        { DATANUM(DATA_m609, 37), DATANUM(DATA_m609, 37) },
        { DATANUM(DATA_m609, 38), DATANUM(DATA_m609, 38) },
        { DATANUM(DATA_m609, 39), DATANUM(DATA_m609, 39) },
        { DATANUM(DATA_m609, 41), DATANUM(DATA_m609, 40) },
        { DATANUM(DATA_m609, 42), DATANUM(DATA_m609, 42) }
    };
    MGACTOR_PARAM actorParam;
    s32 i;
    s32 setupIndexOrLineupX;
    if (lbl_1_bss_30.night == 0) {
        setupIndexOrLineupX = 0;
    } else if (lbl_1_bss_30.night == 1) {
        setupIndexOrLineupX = 1;
    }
    for (i = 0; i < 39; i++) {
        lbl_1_bss_6D88[i] = dataFiles[i][setupIndexOrLineupX];
    }
    fn_1_3C0();
    fn_1_11E2C();
    fn_1_1661C();
    lbl_1_bss_8BC[4].model =
        Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_bss_6D88[0], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_8BC[4].model, 1U);
    Hu3DModelAttrReset(lbl_1_bss_8BC[4].model, 1U);
    Hu3DModelCameraSet(lbl_1_bss_8BC[4].model, 1U);
    Hu3DModelLayerSet(lbl_1_bss_8BC[4].model, 0);
    Hu3DModelAttrSet(lbl_1_bss_8BC[4].model, HU3D_MOTATTR_SHAPE_LOOP);
    Hu3DMotionShapeTimeSet(lbl_1_bss_8BC[4].model, 0.0f);
    Hu3DMotionShapeSpeedSet(lbl_1_bss_8BC[4].model, 1.0f);
    lbl_1_bss_8BC[4].position.x = 0.0f;
    lbl_1_bss_8BC[4].position.y = 0.0f;
    lbl_1_bss_8BC[4].position.z = 0.0f;
    lbl_1_bss_8BC[4].rotation.x = 0.0f;
    lbl_1_bss_8BC[4].rotation.y = 0.0f;
    lbl_1_bss_8BC[4].rotation.z = 0.0f;
    lbl_1_bss_8BC[5].model =
        Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_bss_6D88[1], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_8BC[5].model, 1U);
    Hu3DModelAttrReset(lbl_1_bss_8BC[5].model, 1U);
    Hu3DModelCameraSet(lbl_1_bss_8BC[5].model, 1U);
    Hu3DModelLayerSet(lbl_1_bss_8BC[5].model, 7);
    Hu3DModelShadowMapSet(lbl_1_bss_8BC[5].model);
    Hu3DMotionTimeSet(lbl_1_bss_8BC[5].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_8BC[5].model, 0.0f);
    lbl_1_bss_8BC[5].position.x = 0.0f;
    lbl_1_bss_8BC[5].position.y = 0.0f;
    lbl_1_bss_8BC[5].position.z = 0.0f;
    lbl_1_bss_8BC[5].rotation.x = 0.0f;
    lbl_1_bss_8BC[5].rotation.y = 0.0f;
    lbl_1_bss_8BC[5].rotation.z = 0.0f;
    lbl_1_bss_8BC[7].model =
        Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_bss_6D88[2], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_8BC[7].model, 1U);
    if (lbl_1_bss_30.night == 0) {
        Hu3DModelAttrReset(lbl_1_bss_8BC[7].model, 1U);
    }
    Hu3DModelCameraSet(lbl_1_bss_8BC[7].model, 1U);
    Hu3DModelLayerSet(lbl_1_bss_8BC[7].model, 7);
    Hu3DMotionTimeSet(lbl_1_bss_8BC[7].model, 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_8BC[7].model, 0.0f);
    lbl_1_bss_8BC[7].position.x = 0.0f;
    lbl_1_bss_8BC[7].position.y = 0.0f;
    lbl_1_bss_8BC[7].position.z = 0.0f;
    lbl_1_bss_8BC[7].rotation.x = 0.0f;
    lbl_1_bss_8BC[7].rotation.y = 0.0f;
    lbl_1_bss_8BC[7].rotation.z = 0.0f;
    lbl_1_bss_8BC[8].model =
        Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_bss_6D88[3], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_8BC[8].model, 1U);
    Hu3DModelAttrReset(lbl_1_bss_8BC[8].model, 1U);
    Hu3DModelCameraSet(lbl_1_bss_8BC[8].model, 1U);
    Hu3DModelLayerSet(lbl_1_bss_8BC[8].model, 5);
    Hu3DModelAttrSet(lbl_1_bss_8BC[8].model, HU3D_MOTATTR_LOOP);
    Hu3DMotionSpeedSet(lbl_1_bss_8BC[8].model, 1.0f);
    Hu3DMotionTimeSet(lbl_1_bss_8BC[8].model, 0.0f);
    lbl_1_bss_8BC[8].position.x = 0.0f;
    lbl_1_bss_8BC[8].position.y = 0.0f;
    lbl_1_bss_8BC[8].position.z = 0.0f;
    lbl_1_bss_8BC[8].rotation.x = 0.0f;
    lbl_1_bss_8BC[8].rotation.y = 0.0f;
    lbl_1_bss_8BC[8].rotation.z = 0.0f;
    if (lbl_1_bss_30.night == 1) {
        lbl_1_bss_8BC[10].model =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_bss_6D88[8], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_8BC[10].model, 1U);
        Hu3DModelAttrReset(lbl_1_bss_8BC[10].model, 1U);
        Hu3DModelCameraSet(lbl_1_bss_8BC[10].model, 1U);
        Hu3DModelLayerSet(lbl_1_bss_8BC[10].model, 1);
        Hu3DMotionSpeedSet(lbl_1_bss_8BC[10].model, 0.0f);
        Hu3DMotionTimeSet(lbl_1_bss_8BC[10].model, 0.0f);
        lbl_1_bss_8BC[10].position.x = 0.0f;
        lbl_1_bss_8BC[10].position.y = 0.0f;
        lbl_1_bss_8BC[10].position.z = 0.0f;
        lbl_1_bss_8BC[10].rotation.x = 0.0f;
        lbl_1_bss_8BC[10].rotation.y = 0.0f;
        lbl_1_bss_8BC[10].rotation.z = 0.0f;
    }
    for (i = 28; i <= 31; i++) {
        lbl_1_bss_8BC[i].model =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_bss_6D88[10], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_8BC[i].model, 1U);
        Hu3DModelCameraSet(lbl_1_bss_8BC[i].model, 1U);
        Hu3DModelLayerSet(lbl_1_bss_8BC[i].model, 5);
        Hu3DMotionTimeSet(lbl_1_bss_8BC[i].model, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_8BC[i].model, 0.0f);
        lbl_1_bss_8BC[i].position.x = 0.0f;
        lbl_1_bss_8BC[i].position.y = 0.0f;
        lbl_1_bss_8BC[i].position.z = 0.0f;
        lbl_1_bss_8BC[i].rotation.x = 0.0f;
        lbl_1_bss_8BC[i].rotation.y = 0.0f;
        lbl_1_bss_8BC[i].rotation.z = 0.0f;
    }
    fn_1_E4D4();
    fn_1_F8A8();
    fn_1_FDE0();
    for (i = 32, setupIndexOrLineupX = 0; i <= 45; i++, setupIndexOrLineupX++) {
        lbl_1_bss_4.models[setupIndexOrLineupX] = lbl_1_bss_8BC[i].model;
    }
    MgActorColMapInit(lbl_1_bss_4.models, 14, 24);

    actorParam.height = 150.0f;
    actorParam.radius = 40.0f;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.narrowHook = fn_1_C9A8;
    actorParam.correctHookParam = 0;
    actorParam.correctHook = fn_1_CA6C;

    for (i = 0; i < 4; i++) {
        actorParam.param = i;
        lbl_1_bss_4.players[i] =
            MgPlayerCreate(lbl_1_bss_24C[i].playerNo, &actorParam, 4, 1U, -29, NULL);
        lbl_1_bss_8BC[i].model = lbl_1_bss_4.players[i]->actor->mdlId;
        lbl_1_bss_4.players[i]->actor->pos.x = lbl_1_bss_24C[i].position.x;
        lbl_1_bss_4.players[i]->actor->pos.y = lbl_1_bss_24C[i].position.y;
        lbl_1_bss_4.players[i]->actor->pos.z = lbl_1_bss_24C[i].position.z;
        lbl_1_bss_4.players[i]->actor->rotY = lbl_1_bss_24C[i].rotation.y;
        Hu3DModelShadowSet(lbl_1_bss_4.players[i]->actor->mdlId);
        Hu3DModelShadowDispOn(lbl_1_bss_4.players[i]->actor->mdlId);
        Hu3DModelLayerSet(lbl_1_bss_4.players[i]->actor->mdlId, 5);
        CharModelStepSet(lbl_1_bss_24C[i].characterNo, 1);
        MgPlayerVibrateCreate(lbl_1_bss_4.players[i]);
        lbl_1_bss_24C[i].movementFlag = 0;
    }
    CharEffectLayerSet(5);
    fn_1_2DF4();
    fn_1_10FC();
    fn_1_2780();
    lbl_1_bss_30.scroll = 7.0f;
    lbl_1_bss_30.rate = 1.3f;
    lbl_1_bss_30.update = 1;
    lbl_1_bss_30.startZ = -5500.0f;
    lbl_1_bss_30.winnerPositions[0][0].x = 0.0f;
    lbl_1_bss_30.winnerPositions[0][0].y = 0.0f;
    lbl_1_bss_30.winnerPositions[0][0].z = 100.0f;
    lbl_1_bss_30.winnerPositions[0][1].x = 0.0f;
    lbl_1_bss_30.winnerPositions[0][1].y = 0.0f;
    lbl_1_bss_30.winnerPositions[0][1].z = 0.0f;
    lbl_1_bss_30.winnerPositions[0][2].x = 0.0f;
    lbl_1_bss_30.winnerPositions[0][2].y = 0.0f;
    lbl_1_bss_30.winnerPositions[0][2].z = 0.0f;
    lbl_1_bss_30.winnerPositions[0][3].x = 0.0f;
    lbl_1_bss_30.winnerPositions[0][3].y = 0.0f;
    lbl_1_bss_30.winnerPositions[0][3].z = 0.0f;
    lbl_1_bss_30.winnerPositions[1][0].x = -100.0f;
    lbl_1_bss_30.winnerPositions[1][0].y = 0.0f;
    lbl_1_bss_30.winnerPositions[1][0].z = 100.0f;
    lbl_1_bss_30.winnerPositions[1][1].x = 100.0f;
    lbl_1_bss_30.winnerPositions[1][1].y = 0.0f;
    lbl_1_bss_30.winnerPositions[1][1].z = 100.0f;
    lbl_1_bss_30.winnerPositions[1][2].x = 0.0f;
    lbl_1_bss_30.winnerPositions[1][2].y = 0.0f;
    lbl_1_bss_30.winnerPositions[1][2].z = 0.0f;
    lbl_1_bss_30.winnerPositions[1][3].x = 0.0f;
    lbl_1_bss_30.winnerPositions[1][3].y = 0.0f;
    lbl_1_bss_30.winnerPositions[1][3].z = 0.0f;
    lbl_1_bss_30.winnerPositions[2][0].x = -200.0f;
    lbl_1_bss_30.winnerPositions[2][0].y = 0.0f;
    lbl_1_bss_30.winnerPositions[2][0].z = 100.0f;
    lbl_1_bss_30.winnerPositions[2][1].x = 0.0f;
    lbl_1_bss_30.winnerPositions[2][1].y = 0.0f;
    lbl_1_bss_30.winnerPositions[2][1].z = 100.0f;
    lbl_1_bss_30.winnerPositions[2][2].x = 200.0f;
    lbl_1_bss_30.winnerPositions[2][2].y = 0.0f;
    lbl_1_bss_30.winnerPositions[2][2].z = 100.0f;
    lbl_1_bss_30.winnerPositions[2][3].x = 0.0f;
    lbl_1_bss_30.winnerPositions[2][3].y = 0.0f;
    lbl_1_bss_30.winnerPositions[2][3].z = 0.0f;
    lbl_1_bss_30.winnerPositions[3][0].x = -300.0f;
    lbl_1_bss_30.winnerPositions[3][0].y = 0.0f;
    lbl_1_bss_30.winnerPositions[3][0].z = 100.0f;
    lbl_1_bss_30.winnerPositions[3][1].x = -100.0f;
    lbl_1_bss_30.winnerPositions[3][1].y = 0.0f;
    lbl_1_bss_30.winnerPositions[3][1].z = 100.0f;
    lbl_1_bss_30.winnerPositions[3][2].x = 100.0f;
    lbl_1_bss_30.winnerPositions[3][2].y = 0.0f;
    lbl_1_bss_30.winnerPositions[3][2].z = 100.0f;
    lbl_1_bss_30.winnerPositions[3][3].x = 300.0f;
    lbl_1_bss_30.winnerPositions[3][3].y = 0.0f;
    lbl_1_bss_30.winnerPositions[3][3].z = 100.0f;
    for (i = 0, setupIndexOrLineupX = 350; i < 8; i++, setupIndexOrLineupX -= 100) {
        lbl_1_bss_30.levels[i] = (f32) setupIndexOrLineupX;
    }
    lbl_1_bss_30.winners[0] = lbl_1_bss_30.winners[1] =
        lbl_1_bss_30.winners[2] = lbl_1_bss_30.winners[3] = -1;

    {
    GXColor lightColor = {255, 255, 255, 255};
    Vec lightPosition = {0.0f, 1000.0f, 1000.0f};
    Vec lightDirection = {0.0f, -0.5f, -0.5f};
    Hu3DModelLightBitReset(lbl_1_bss_8BC[0].model, lbl_1_bss_6BF8.id);
    Hu3DModelLightBitReset(lbl_1_bss_8BC[1].model, lbl_1_bss_6BF8.id);
    Hu3DModelLightBitReset(lbl_1_bss_8BC[2].model, lbl_1_bss_6BF8.id);
    Hu3DModelLightBitReset(lbl_1_bss_8BC[3].model, lbl_1_bss_6BF8.id);
    CharLightCreateV(&lightPosition, &lightDirection, &lightColor);
    CharLightInfinitytSet();
    }
    return 0;
}

/* Updates the opening scene each frame and tells fn_1_25C when its camera reaches step 6. */
s32 fn_1_E424(void) {
    s32 sequenceReady;

    sequenceReady = 0;
    fn_1_670();
    fn_1_101AC();
    if ((lbl_1_bss_30.step) >= 6) {
        sequenceReady = 1;
    }
    fn_1_2780();
    return sequenceReady;
}

/* Runs one opening-scene update for fn_1_288; its message check never sets the return value. */
s32 fn_1_E474(void) {
    s32 sequenceReady = 0;

    if (GameMesFXPlayCheck(MgSeqGameMesIdGet()) != 0) {
        fn_1_5618(8, 30);
        fn_1_459C();
    }
    fn_1_670();
    fn_1_101AC();
    fn_1_2780();
    return (s16) sequenceReady;
}

/* Creates and places course models during setup; fn_1_D1CC calls this after loading resources. */
void fn_1_E4D4(void)
{
    s16 stageFiles[14] = {
        11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 14, 14, 15, 16
    };
    s16 obstacleFiles[34] = {
        17, 17, 17, 17, 30, 18, 18, 18, 18, 18,
        19, 19, 19, 19, 20, 20, 21, 21, 22, 22,
        23, 23, 24, 24, 25, 25, 26, 26, 27, 27,
        28, 28, 29, 29
    };
    s16 playerFiles[64] = {
        31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
        32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33,
        33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 34,
        34, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 36, 36, 36, 36
    };
    M609Placement placements[34] = {
        { 0, 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 200.0f }, 46, 32 },
        { 1, 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 200.0f }, 47, 33 },
        { 2, 1, { 350.0f, 0.0f, 0.0f }, { 0.0f, -0.25f, 0.0f }, { 300.0f, 0.0f, 0.0f }, { 350.0f, 0.0f, 0.0f }, { 200.0f, 0.0f, 200.0f }, 51, 35 },
        { 3, 1, { -350.0f, 0.0f, 0.0f }, { 0.0f, 0.25f, 0.0f }, { -300.0f, 0.0f, 0.0f }, { -250.0f, 0.0f, 0.0f }, { 200.0f, 0.0f, 200.0f }, 52, 36 },
        { 4, 2, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 100.0f }, 56, 37 },
        { 5, 3, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 60, -1 },
        { 6, 3, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 64, -1 },
        { 7, 3, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 61, -1 },
        { 8, 3, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 200.0f }, 65, -1 },
        { 9, 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 200.0f }, 48, -1 },
        { 10, 8, { 100.0f, 0.0f, 0.0f }, { 0.0f, -0.3f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 100.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 76, -1 },
        { 11, 9, { 100.0f, 0.0f, 0.0f }, { 0.0f, -0.3f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 100.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 0.0f }, 78, -1 },
        { 12, 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 200.0f }, 49, -1 },
        { 13, 5, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 68, -1 },
        { 14, 5, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 72, -1 },
        { 15, 5, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 69, -1 },
        { 16, 5, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 200.0f }, 73, -1 },
        { 17, 1, { 100.0f, 0.0f, 0.0f }, { 0.0f, -0.3f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 100.0f, 0.0f, 0.0f }, { 200.0f, 0.0f, 200.0f }, 53, -1 },
        { 18, 1, { -100.0f, 0.0f, 0.0f }, { 0.0f, 0.3f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -100.0f, 0.0f, 0.0f }, { 200.0f, 0.0f, 200.0f }, 54, -1 },
        { 19, 8, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 77, -1 },
        { 20, 9, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 0.0f }, 79, -1 },
        { 21, 7, { 250.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 70, -1 },
        { 22, 7, { 250.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 74, -1 },
        { 23, 7, { 250.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 71, -1 },
        { 24, 7, { 250.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 200.0f, 0.0f, 200.0f }, 75, -1 },
        { 25, 6, { -250.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 62, -1 },
        { 26, 6, { -250.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 66, -1 },
        { 27, 6, { -250.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, 63, -1 },
        { 28, 6, { -250.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 200.0f, 0.0f, 200.0f }, 67, -1 },
        { 29, 2, { 100.0f, 0.0f, 0.0f }, { 0.0f, -0.3f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 100.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 100.0f }, 57, -1 },
        { 30, 2, { -100.0f, 0.0f, 0.0f }, { 0.0f, 0.3f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 100.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 100.0f }, 58, -1 },
        { 31, 2, { 100.0f, 0.0f, 0.0f }, { 0.0f, -0.3f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 100.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 100.0f }, 59, -1 },
        { 32, 1, { 0.0f, 0.0f, 0.0f }, { 0.0f, -0.5f, 0.0f }, { 0.0f, 100.0f, 100.0f }, { 0.0f, 0.0f, 200.0f }, { 200.0f, 0.0f, 200.0f }, 55, -1 },
        { 33, 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 400.0f, 0.0f, 200.0f }, 50, -1 }
    };
    s16 playerModels[18][4] = {
        { 80, 81, 82, 83 },
        { 84, 85, 86, 87 },
        { 88, 89, 90, 91 },
        { 96, 97, 98, 99 },
        { 100, 101, 102, 103 },
        { 92, 93, 94, 95 },
        { 80, 81, 82, 83 },
        { 84, 85, 86, 87 },
        { 104, 105, 106, 107 },
        { 108, 109, 110, 111 },
        { 112, 113, 114, 115 },
        { 116, 117, 118, 119 },
        { 136, 137, 138, 139 },
        { 140, 141, 142, 143 },
        { 120, 121, 122, 123 },
        { 124, 125, 126, 127 },
        { 128, 129, 130, 131 },
        { 132, 133, 134, 135 }
    };
    s16 recordIndex;
    s16 resourceOrPlayerIndex;
    s16 playerOrRouteIndex;

    placements[0].position.z = -2930.0f;
    placements[1].position.z = placements[0].position.z - 600.0f;
    placements[2].position.z = placements[1].position.z - 600.0f;
    placements[3].position.z = placements[1].position.z - 600.0f;
    placements[4].position.z = placements[3].position.z - 500.0f;
    placements[5].position.z = placements[4].position.z - 350.0f;
    placements[6].position.z = placements[5].position.z - 100.0f;
    placements[7].position.z = placements[6].position.z - 100.0f;
    placements[8].position.z = placements[7].position.z - 100.0f;
    placements[9].position.z = placements[8].position.z - 450.0f;
    placements[10].position.z = placements[9].position.z - 600.0f;
    placements[11].position.z = placements[10].position.z - 400.0f;
    placements[12].position.z = placements[11].position.z - 700.0f;
    placements[13].position.z = placements[12].position.z - 450.0f;
    placements[14].position.z = placements[13].position.z - 100.0f;
    placements[15].position.z = placements[14].position.z - 100.0f;
    placements[16].position.z = placements[15].position.z - 100.0f;
    placements[17].position.z = placements[16].position.z - 450.0f;
    placements[18].position.z = placements[17].position.z - 600.0f;
    placements[19].position.z = placements[18].position.z - 600.0f;
    placements[20].position.z = placements[19].position.z - 400.0f;
    placements[21].position.z = placements[20].position.z - 450.0f;
    placements[22].position.z = placements[21].position.z - 100.0f;
    placements[23].position.z = placements[22].position.z - 100.0f;
    placements[24].position.z = placements[23].position.z - 100.0f;
    placements[25].position.z = placements[21].position.z;
    placements[26].position.z = placements[22].position.z;
    placements[27].position.z = placements[23].position.z;
    placements[28].position.z = placements[24].position.z;
    placements[29].position.z = placements[24].position.z - 300.0f;
    placements[30].position.z = placements[29].position.z - 400.0f;
    placements[31].position.z = placements[30].position.z - 400.0f;
    placements[32].position.z = placements[31].position.z - 600.0f;
    placements[33].position.z = placements[32].position.z - 700.0f;

    for (recordIndex = 32, resourceOrPlayerIndex = 0; recordIndex <= 45;
         recordIndex++, resourceOrPlayerIndex++) {
        lbl_1_bss_8BC[recordIndex].model =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_bss_6D88[stageFiles[resourceOrPlayerIndex]],
                                               HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_8BC[recordIndex].model, 1);
        Hu3DModelCameraSet(lbl_1_bss_8BC[recordIndex].model, 1);
        Hu3DModelLayerSet(lbl_1_bss_8BC[recordIndex].model, 0);
        Hu3DMotionSpeedSet(lbl_1_bss_8BC[recordIndex].model, 0.0f);
        lbl_1_bss_8BC[recordIndex].position.x = 0.0f;
        lbl_1_bss_8BC[recordIndex].position.y = -10000.0f;
        lbl_1_bss_8BC[recordIndex].position.z = 0.0f;
        lbl_1_bss_8BC[recordIndex].rotation.x = 0.0f;
        lbl_1_bss_8BC[recordIndex].rotation.y = 0.0f;
        lbl_1_bss_8BC[recordIndex].rotation.z = 0.0f;
        lbl_1_bss_8BC[recordIndex].linkedRecord = -1;
        lbl_1_bss_8BC[recordIndex].resourceIndex = stageFiles[resourceOrPlayerIndex] - 11;
        for (playerOrRouteIndex = 0; playerOrRouteIndex < 4; playerOrRouteIndex++) {
            lbl_1_bss_8BC[recordIndex].playerModels[playerOrRouteIndex] =
                playerModels[resourceOrPlayerIndex][playerOrRouteIndex];
        }
    }
    recordIndex = resourceOrPlayerIndex;
    for (playerOrRouteIndex = 0; playerOrRouteIndex < 4; playerOrRouteIndex++) {
        for (resourceOrPlayerIndex = 0; resourceOrPlayerIndex < 4; resourceOrPlayerIndex++) {
            lbl_1_bss_30.models[playerOrRouteIndex][resourceOrPlayerIndex] =
                playerModels[recordIndex][resourceOrPlayerIndex];
        }
        recordIndex++;
    }
    for (recordIndex = 46, resourceOrPlayerIndex = 0; recordIndex <= 79;
         recordIndex++, resourceOrPlayerIndex++) {
        lbl_1_bss_8BC[recordIndex].model = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_bss_6D88[obstacleFiles[resourceOrPlayerIndex]], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_8BC[recordIndex].model, 1);
        Hu3DModelCameraSet(lbl_1_bss_8BC[recordIndex].model, 1);
        Hu3DModelLayerSet(lbl_1_bss_8BC[recordIndex].model, 1);
        Hu3DModelShadowMapSet(lbl_1_bss_8BC[recordIndex].model);
        Hu3DMotionSpeedSet(lbl_1_bss_8BC[recordIndex].model, 0.0f);
        Hu3DMotionTimeSet(lbl_1_bss_8BC[recordIndex].model, 0.0f);
        lbl_1_bss_8BC[recordIndex].position.x = 0.0f;
        lbl_1_bss_8BC[recordIndex].position.y = 0.0f;
        lbl_1_bss_8BC[recordIndex].position.z = 0.0f;
        lbl_1_bss_8BC[recordIndex].rotation.x = 0.0f;
        lbl_1_bss_8BC[recordIndex].rotation.y = 0.0f;
        lbl_1_bss_8BC[recordIndex].rotation.z = 0.0f;
        lbl_1_bss_8BC[recordIndex].resourceIndex = obstacleFiles[resourceOrPlayerIndex] - 17;
        lbl_1_bss_8BC[recordIndex].bobSpeed = 0.3f;
    }
    recordIndex = 9;
    lbl_1_bss_8BC[recordIndex].model =
        Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_bss_6D88[7], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_8BC[recordIndex].model, 1);
    Hu3DModelCameraSet(lbl_1_bss_8BC[recordIndex].model, 1);
    Hu3DModelLayerSet(lbl_1_bss_8BC[recordIndex].model, 5);
    Hu3DMotionSpeedSet(lbl_1_bss_8BC[recordIndex].model, 0.0f);
    Hu3DMotionTimeSet(lbl_1_bss_8BC[recordIndex].model, 0.0f);
    lbl_1_bss_8BC[recordIndex].position.x = 0.0f;
    lbl_1_bss_8BC[recordIndex].position.y = 0.0f;
    lbl_1_bss_8BC[recordIndex].position.z = 0.0f;
    lbl_1_bss_8BC[recordIndex].rotation.x = 0.0f;
    lbl_1_bss_8BC[recordIndex].rotation.y = 0.0f;
    lbl_1_bss_8BC[recordIndex].rotation.z = 0.0f;
    for (recordIndex = 11; recordIndex <= 16; recordIndex++) {
        lbl_1_bss_8BC[recordIndex].model =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_bss_6D88[9], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_8BC[recordIndex].model, 1);
        Hu3DModelCameraSet(lbl_1_bss_8BC[recordIndex].model, 1);
        Hu3DModelLayerSet(lbl_1_bss_8BC[recordIndex].model, 7);
        Hu3DMotionSpeedSet(lbl_1_bss_8BC[recordIndex].model, 0.0f);
        Hu3DMotionTimeSet(lbl_1_bss_8BC[recordIndex].model, 0.0f);
        lbl_1_bss_8BC[recordIndex].position.x = 0.0f;
        lbl_1_bss_8BC[recordIndex].position.y = 0.0f;
        lbl_1_bss_8BC[recordIndex].position.z = 0.0f;
        lbl_1_bss_8BC[recordIndex].rotation.x = 0.0f;
        lbl_1_bss_8BC[recordIndex].rotation.y = 0.0f;
        lbl_1_bss_8BC[recordIndex].rotation.z = 0.0f;
    }
    for (recordIndex = 80, resourceOrPlayerIndex = 0; recordIndex <= 143;
         recordIndex++, resourceOrPlayerIndex++) {
        lbl_1_bss_8BC[recordIndex].model =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_bss_6D88[playerFiles[resourceOrPlayerIndex]],
                                               HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_8BC[recordIndex].model, 1);
        Hu3DModelCameraSet(lbl_1_bss_8BC[recordIndex].model, 1);
        Hu3DModelLayerSet(lbl_1_bss_8BC[recordIndex].model, 5);
        Hu3DMotionSpeedSet(lbl_1_bss_8BC[recordIndex].model, 0.0f);
        Hu3DMotionTimeSet(lbl_1_bss_8BC[recordIndex].model, 0.0f);
        lbl_1_bss_8BC[recordIndex].position.x = 0.0f;
        lbl_1_bss_8BC[recordIndex].position.y = 0.0f;
        lbl_1_bss_8BC[recordIndex].position.z = 0.0f;
        lbl_1_bss_8BC[recordIndex].rotation.x = 0.0f;
        lbl_1_bss_8BC[recordIndex].rotation.y = 0.0f;
        lbl_1_bss_8BC[recordIndex].rotation.z = 0.0f;
        lbl_1_bss_8BC[recordIndex].linkedRecord = -1;
    }
    lbl_1_bss_8BC[50].resourceIndex = 13;
    lbl_1_bss_8BC[6].model =
        Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_bss_6D88[4], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_8BC[6].model, 1);
    Hu3DModelAttrSet(lbl_1_bss_8BC[6].model, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(lbl_1_bss_8BC[6].model, 1);
    Hu3DModelLayerSet(lbl_1_bss_8BC[6].model, 7);
    Hu3DMotionSpeedSet(lbl_1_bss_8BC[6].model, 1.0f);
    Hu3DMotionTimeSet(lbl_1_bss_8BC[6].model, 1.0f);
    lbl_1_bss_8BC[6].position.x = 0.0f;
    lbl_1_bss_8BC[6].position.y = 0.0f;
    lbl_1_bss_8BC[6].position.z = 0.0f;
    lbl_1_bss_8BC[6].rotation.x = 0.0f;
    lbl_1_bss_8BC[6].rotation.y = 0.0f;
    lbl_1_bss_8BC[6].rotation.z = 0.0f;
    for (recordIndex = 0; recordIndex < 34; recordIndex++) {
        lbl_1_bss_8BC[placements[recordIndex].record].status =
            placements[recordIndex].status;
        lbl_1_bss_8BC[placements[recordIndex].record].recordIndex =
            placements[recordIndex].record;
        lbl_1_bss_8BC[placements[recordIndex].record].pathIndex =
            placements[recordIndex].pathIndex;
        lbl_1_bss_8BC[placements[recordIndex].record].linkedRecord =
            placements[recordIndex].linkedRecord;
        lbl_1_bss_8BC[placements[recordIndex].record].position =
            placements[recordIndex].position;
        lbl_1_bss_8BC[placements[recordIndex].record].movementValues =
            placements[recordIndex].movementValues;
        lbl_1_bss_8BC[placements[recordIndex].record].positionStep =
            placements[recordIndex].positionStep;
        lbl_1_bss_8BC[placements[recordIndex].record].movementSelectors =
            placements[recordIndex].movementSelectors;
        lbl_1_bss_8BC[placements[recordIndex].record].bounds =
            placements[recordIndex].bounds;
        if (lbl_1_bss_8BC[placements[recordIndex].record].movementValues.y <= 0.0f) {
            lbl_1_bss_8BC[placements[recordIndex].record].movementPhase = 0;
        } else {
            lbl_1_bss_8BC[placements[recordIndex].record].movementPhase = 1;
        }
    }
}

/* Creates auxiliary hazard models and the final-platform attachment during setup;
 * fn_1_D1CC calls this after loading resources. */
void fn_1_F8A8(void) {
    s16 recordIndex;
    s16 initializedRecords;

    for (recordIndex = 144, initializedRecords = 0;
         recordIndex <= 152;
         recordIndex++, initializedRecords++) {
        if (recordIndex == 144) {
            lbl_1_bss_8BC[recordIndex].model =
                Hu3DModelCreate(HuDataSelHeapReadNum(
                ((s32 *)&lbl_1_bss_6D88)[148 / sizeof(s32)],
                HU_MEMNUM_OVL,
                HEAP_MODEL));
        } else {
            lbl_1_bss_8BC[recordIndex].model =
                Hu3DModelLink(
                    lbl_1_bss_8BC[144].model);
        }
        Hu3DModelAttrSet(
            lbl_1_bss_8BC[recordIndex].model, 1);
        Hu3DModelCameraSet(
            lbl_1_bss_8BC[recordIndex].model, 1);
        Hu3DModelLayerSet(
            lbl_1_bss_8BC[recordIndex].model, 7);
        lbl_1_bss_8BC[recordIndex].position.x =
            (0.0f);
        lbl_1_bss_8BC[recordIndex].position.y =
            (-10000.0f);
        lbl_1_bss_8BC[recordIndex].position.z =
            (0.0f);
        lbl_1_bss_8BC[recordIndex].rotation.x = (0.0f);
        lbl_1_bss_8BC[recordIndex].rotation.y =
            (0.0f);
        lbl_1_bss_8BC[recordIndex].rotation.z = (0.0f);
        lbl_1_bss_8BC[recordIndex].linkedRecord = -1;
        if (lbl_1_bss_30.night == 1) {
            Hu3DModelScaleSet(
                              lbl_1_bss_8BC[recordIndex].model,
                              (1.5f),
                              (1.5f),
                              (1.5f));
            if (recordIndex == 144) {
                lbl_1_bss_0[0] = Hu3DJointMotion(
                    lbl_1_bss_8BC[recordIndex].model,
                    HuDataSelHeapReadNum(DATANUM(DATA_m609, 43), HU_MEMNUM_OVL, HEAP_MODEL));
            }
            Hu3DMotionSet(
                lbl_1_bss_8BC[recordIndex].model,
                lbl_1_bss_0[0]);
            Hu3DModelAttrSet(
                lbl_1_bss_8BC[recordIndex].model,
                HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(
                lbl_1_bss_8BC[recordIndex].model,
                (1.0f));
            Hu3DMotionTimeSet(
                lbl_1_bss_8BC[recordIndex].model,
                              fn_1_31F8((100.0f)));
            lbl_1_bss_30.raftOffset.x = (0.0f);
            lbl_1_bss_30.raftOffset.y = (50.0f);
            lbl_1_bss_30.raftOffset.z = (0.0f);
        }
    }

    lbl_1_bss_8BC[153].model =
        Hu3DModelCreate(HuDataSelHeapReadNum(
            ((s32 *)&lbl_1_bss_6D88)[152 / sizeof(s32)],
            HU_MEMNUM_OVL,
            HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_8BC[153].model, 1);
    Hu3DModelCameraSet(lbl_1_bss_8BC[153].model, 1);
    Hu3DModelLayerSet(lbl_1_bss_8BC[153].model, 7);
    lbl_1_bss_8BC[153].position.x = (0.0f);
    lbl_1_bss_8BC[153].position.y = (-10000.0f);
    lbl_1_bss_8BC[153].position.z = (0.0f);
    lbl_1_bss_8BC[153].rotation.x = (0.0f);
    lbl_1_bss_8BC[153].rotation.y = (0.0f);
    lbl_1_bss_8BC[153].rotation.z = (0.0f);
    lbl_1_bss_8BC[153].linkedRecord = -1;
    lbl_1_bss_0[1] = Hu3DJointMotion(
        lbl_1_bss_8BC[153].model,
        HuDataSelHeapReadNum(DATANUM(DATA_m609, 44), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(lbl_1_bss_8BC[153].model,
                  lbl_1_bss_0[1]);
    Hu3DModelAttrSet(lbl_1_bss_8BC[153].model,
                      HU3D_MOTATTR_LOOP);
    Hu3DMotionSpeedSet(lbl_1_bss_8BC[153].model,
                       (1.0f));
    Hu3DMotionTimeSet(lbl_1_bss_8BC[153].model,
                      (0.0f));
}

/* Creates the two five-model drifting-obstacle pools during setup;
 * fn_1_D1CC calls this after loading resources. */
void fn_1_FDE0(void) {
    s16 modelIndex;
    s16 unusedModelIndex;
    modelIndex = 17;
    while (modelIndex <= 21)
    {
        lbl_1_bss_8BC[modelIndex].model =
            Hu3DModelCreate(HuDataSelHeapReadNum(((s32)lbl_1_bss_6D88[5]),
                                                 HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_8BC[modelIndex].model, 1U);
        Hu3DModelCameraSet(lbl_1_bss_8BC[modelIndex].model, 1U);
        Hu3DModelLayerSet(lbl_1_bss_8BC[modelIndex].model, 5);
        Hu3DMotionSpeedSet(lbl_1_bss_8BC[modelIndex].model, (0.0f));
        Hu3DMotionTimeSet(lbl_1_bss_8BC[modelIndex].model, (0.0f));
        (lbl_1_bss_8BC[modelIndex].position.x) =
            (f32) (0.0f);
        (lbl_1_bss_8BC[modelIndex].position.y) =
            (f32) (0.0f);
        (lbl_1_bss_8BC[modelIndex].position.z) =
            (f32) (0.0f);
        (lbl_1_bss_8BC[modelIndex].rotation.x) =
            (f32) (0.0f);
        (lbl_1_bss_8BC[modelIndex].rotation.y) =
            (f32) (0.0f);
        (lbl_1_bss_8BC[modelIndex].rotation.z) =
            (f32) (0.0f);
        modelIndex += 1;
    }
    modelIndex = 22;
    while (modelIndex <= 26)
    {
        lbl_1_bss_8BC[modelIndex].model =
            Hu3DModelCreate(HuDataSelHeapReadNum(((s32)lbl_1_bss_6D88[6]),
                                                 HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_8BC[modelIndex].model, 1U);
        Hu3DModelCameraSet(lbl_1_bss_8BC[modelIndex].model, 1U);
        Hu3DModelLayerSet(lbl_1_bss_8BC[modelIndex].model, 5);
        Hu3DMotionSpeedSet(lbl_1_bss_8BC[modelIndex].model, (0.0f));
        Hu3DMotionTimeSet(lbl_1_bss_8BC[modelIndex].model, (0.0f));
        (lbl_1_bss_8BC[modelIndex].position.x) =
            (f32) (0.0f);
        (lbl_1_bss_8BC[modelIndex].position.y) =
            (f32) (0.0f);
        (lbl_1_bss_8BC[modelIndex].position.z) =
            (f32) (0.0f);
        (lbl_1_bss_8BC[modelIndex].rotation.x) =
            (f32) (0.0f);
        (lbl_1_bss_8BC[modelIndex].rotation.y) =
            (f32) (0.0f);
        (lbl_1_bss_8BC[modelIndex].rotation.z) =
            (f32) (0.0f);
        modelIndex += 1;
    }
}

/* Advances the opening landing scene each frame; fn_1_E424 and fn_1_E474 call this update. */
s32 fn_1_101AC(void)
{
    s16 i;
    s16 modelIndex;
    s32 result = 0;
    s32 moving = 0;
    f32 motionStep;
    f32 cameraTravelOrVolume;
    f32 volumePerWorldUnit;
    s16 volume;

    switch (lbl_1_bss_30.step) {
    case 0:
        if (lbl_1_bss_30.waitTimer == 5) {
            fn_1_46F8(8, 5, 0);
        }
        if (lbl_1_bss_30.waitTimer++ > 60) {
            fn_1_46F8(7, 4, 0);
            lbl_1_bss_30.waitTimer = 0;
            lbl_1_bss_30.step++;
        }
        break;
    case 1:
        fn_1_46F8(0, 5, 0);
        Hu3DMotionSpeedSet(lbl_1_bss_8BC[5].model, 1.0f);
        for (i = 0; i < 4; i++) {
            lbl_1_bss_70C[i].mode = 6;
            lbl_1_bss_70C[i].speed = 0.0f;
            lbl_1_bss_70C[i].time = 1.0f;
            lbl_1_bss_70C[i].attributes = 0;
            lbl_1_bss_70C[i].attributesToReset = HU3D_MOTATTR_LOOP;
            omVibrate(lbl_1_bss_24C[i].playerNo, 20, 7, 3);
        }
        lbl_1_bss_30.step++;
        break;
    case 2:
        if (lbl_1_bss_70C[0].time >= 25.0f) {
            for (i = 0; i < 4; i++) {
                lbl_1_bss_70C[i].speed = 0.0f;
                lbl_1_bss_70C[i].previousSpeed = -1.0f;
            }
            fn_1_46F8(1, 5, 0);
            lbl_1_bss_30.step++;
        }
        break;
    case 3:
        if (lbl_1_bss_24C[0].position.y < 50.0f && lbl_1_bss_30.landed == 0) {
            for (i = 0; i < 4; i++) {
                lbl_1_bss_24C[i].position.y = 50.0f;
                lbl_1_bss_70C[i].shiftState = 1;
                lbl_1_bss_70C[i].shiftMotion = 4;
                lbl_1_bss_70C[i].shiftStart = 0.0f;
                lbl_1_bss_70C[i].blendDuration = 20.0f;
                lbl_1_bss_70C[i].shiftFlags = 0;
                lbl_1_bss_70C[i].attributesToReset = HU3D_MOTATTR_LOOP;
                CharModelLandDustCreate(lbl_1_bss_24C[i].characterNo, &lbl_1_bss_24C[i].position);
                lbl_1_bss_24C[i].collisionMesh = 0;
                lbl_1_bss_24C[i].previousCollisionMesh = 0;
                modelIndex = lbl_1_bss_8BC[32].playerModels[i];
                lbl_1_bss_8BC[46].motionVerticalSpeed = -1.0f;
                Hu3DModelAttrReset(lbl_1_bss_8BC[modelIndex].model, 1);
                Hu3DMotionTimeSet(lbl_1_bss_8BC[modelIndex].model, 0.0f);
                Hu3DMotionSpeedSet(lbl_1_bss_8BC[modelIndex].model, 1.0f);
                lbl_1_bss_8BC[modelIndex].stateOrTimer = 1;
                lbl_1_bss_8BC[modelIndex].linkedRecord = 46;
                lbl_1_bss_8BC[modelIndex].position = lbl_1_bss_8BC[32].position;
                omVibrate(lbl_1_bss_24C[i].playerNo, 20, 7, 3);
            }
            fn_1_46F8(2, 5, 0);
            lbl_1_bss_30.landed = 1;
        } else if (lbl_1_bss_30.landed == 0) {
            for (i = 0; i < 4; i++) {
                lbl_1_bss_24C[i].velocity.y -= 1.0f;
                lbl_1_bss_24C[i].position.y += lbl_1_bss_24C[i].velocity.y;
            }
        }
        moving = fn_1_30D8(&CRot.x, 60.0f, -44.0f, -81.0f);
        if (lbl_1_bss_24C[0].position.y <= 50.0f && (s16)moving == 0) {
            lbl_1_bss_30.step++;
        }
        break;
    case 4:
        lbl_1_bss_30.step++;
        break;
    case 5:
        motionStep = 60.0f;
        moving += fn_1_30D8(&Center.x, motionStep, 0.0f, 257.0f);
        moving += fn_1_30D8(&Center.y, motionStep, 1454.0f, 350.0f);
        moving += fn_1_30D8(&Center.z, motionStep, -1850.0f, 274.0f);
        moving += fn_1_30D8(&CRot.x, motionStep, -81.0f, -42.0f);
        moving += fn_1_30D8(&CRot.y, motionStep, 0.0f, 30.0f);
        moving += fn_1_30D8(&CRot.z, motionStep, 0.0f, 0.0f);
        moving += fn_1_30D8(&CZoom, motionStep, 812.0f, 1587.0f);
        if ((s16)moving == 0) {
            lbl_1_bss_30.startZ = -2050.0f;
            for (i = 46; i <= 152; i++) {
                lbl_1_bss_8BC[i].stateOrTimer = 0;
                Hu3DModelAttrSet(lbl_1_bss_8BC[i].model, 1);
                if (i >= 144 && i <= 152) {
                    lbl_1_bss_8BC[i].linkedRecord = -1;
                }
            }
            Hu3DModelAttrSet(lbl_1_bss_8BC[5].model, 1);
            lbl_1_bss_30.step++;
        }
        break;
    case 6:
        if (lbl_1_bss_30.waitTimer++ > 40) {
            for (i = 0; i < 4; i++) {
                lbl_1_bss_70C[i].mode = 5;
                lbl_1_bss_70C[i].speed = 0.0f;
                lbl_1_bss_70C[i].time = 1.0f;
                lbl_1_bss_70C[i].attributes = 0;
                lbl_1_bss_70C[i].attributesToReset = HU3D_MOTATTR_LOOP;
            }
            lbl_1_bss_30.waitTimer = 0;
            lbl_1_bss_30.step++;
        }
        break;
    case 7:
        if (lbl_1_bss_70C[0].time >= 28.9f) {
            for (i = 0; i < 4; i++) {
                lbl_1_bss_70C[i].mode = 0;
                lbl_1_bss_70C[i].speed = 0.0f;
                lbl_1_bss_70C[i].time = 1.0f;
                lbl_1_bss_70C[i].attributes = HU3D_MOTATTR_LOOP;
                lbl_1_bss_70C[i].attributesToReset = 0;
            }
            lbl_1_bss_30.step++;
        }
        break;
    case 8:
        if (lbl_1_bss_30.waitTimer++ > 1) {
            lbl_1_bss_30.waitTimer = 0;
            lbl_1_bss_30.step++;
        }
        break;
    case 9:
        lbl_1_bss_30.waitTimer++;
        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_30.waitTimer == i * 5 + 1) {
                lbl_1_bss_70C[i].shiftState = 1;
                lbl_1_bss_70C[i].shiftMotion = 1;
                lbl_1_bss_70C[i].shiftStart = 0.0f;
                lbl_1_bss_70C[i].blendDuration = 5.0f;
                lbl_1_bss_70C[i].shiftFlags = HU3D_MOTATTR_LOOP;
                lbl_1_bss_70C[i].attributesToReset = 0;
                lbl_1_bss_24C[i].turnYaw = 10.0f;
            }
            if ((lbl_1_bss_24C[i].rotation.y += lbl_1_bss_24C[i].turnYaw) >= 180.0f) {
                lbl_1_bss_24C[i].rotation.y = 180.0f;
                /* This remains the increment until the sequence step advances. */
                lbl_1_bss_24C[i].turnYaw = 180.0f;
                lbl_1_bss_70C[i].shiftState = 1;
                lbl_1_bss_70C[i].shiftMotion = 0;
                lbl_1_bss_70C[i].shiftStart = 0.0f;
                lbl_1_bss_70C[i].blendDuration = 5.0f;
                lbl_1_bss_70C[i].shiftFlags = HU3D_MOTATTR_LOOP;
                lbl_1_bss_70C[i].attributesToReset = 0;
                if (i == 3) {
                    lbl_1_bss_30.waitTimer = 0;
                    lbl_1_bss_30.step++;
                }
            }
        }
        break;
    case 10:
        result = 1;
        break;
    }
    volumePerWorldUnit = (f32)lbl_1_bss_30.volumeZHigh / 2124.0f;
    cameraTravelOrVolume = 1850.0f + Center.z;
    cameraTravelOrVolume *= volumePerWorldUnit;
    volume = cameraTravelOrVolume;
    fn_1_4C68(7, volume);
    if (lbl_1_bss_6B6C.target.z >= 0.0f) {
        lbl_1_bss_6B6C.target.z = 0.0f;
    } else {
        lbl_1_bss_6B6C.target.z = lbl_1_bss_24C[0].position.z;
    }
    if (lbl_1_bss_30.update) {
        fn_1_6974();
    }
    if (lbl_1_bss_30.landed) {
        for (i = 0; i < 4; i++) {
            lbl_1_bss_24C[i].position.y = 50.0f + lbl_1_bss_8BC[46].position.y;
            lbl_1_bss_24C[i].position.z += lbl_1_bss_30.scroll;
        }
    }
    return result;
}

u32 lbl_1_bss_6D88[40];
