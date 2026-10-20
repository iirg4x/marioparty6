/* Sets up stage models, collision layout, lighting and the opening sequence. */
#include "REL/m660dll/camera_lighting.h"
#include "dolphin/math.h"
#include "game/data.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/mg/actman.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/wipe.h"
#include "game/mg/seqman.h"
#include "REL/m660dll/scene.h"
#include "REL/m660dll/player_slots.h"
#include "datanum/charmot.h"
#include <string.h>

typedef struct ShadowPositionTable_AA2C {
    Vec positions[16];
} ShadowPositionTable_AA2C;

typedef struct StageLayout_B050 {
    Vec position;
    Vec rotation;
} StageLayout_B050;

typedef struct StageLayouts_B050 {
    StageLayout_B050 values[14];
} StageLayouts_B050;

typedef struct BoundsPresets_B050 {
    Vec values[3];
} BoundsPresets_B050;

typedef struct BoundaryPlanes_B050 {
    Vec values[4];
} BoundaryPlanes_B050;

typedef struct ModelScales_B9D4 {
    Vec values[9];
} ModelScales_B9D4;

typedef struct ModelRadii_B9D4 {
    f32 values[9];
} ModelRadii_B9D4;

u32 lbl_1_data_480[36] = {
    DATANUM(DATA_m660, 0), DATANUM(DATA_m660, 1), DATANUM(DATA_m660, 2), DATANUM(DATA_m660, 3),
    DATANUM(DATA_m660, 4), DATANUM(DATA_m660, 5), DATANUM(DATA_m660, 6), DATANUM(DATA_m660, 7),
    DATANUM(DATA_m660, 8), DATANUM(DATA_m660, 9), DATANUM(DATA_m660, 10), DATANUM(DATA_m660, 11),
    DATANUM(DATA_m660, 12), DATANUM(DATA_m660, 13), DATANUM(DATA_m660, 14), DATANUM(DATA_m660, 15),
    DATANUM(DATA_m660, 16), DATANUM(DATA_m660, 17), DATANUM(DATA_m660, 18), DATANUM(DATA_m660, 19),
    DATANUM(DATA_m660, 20), DATANUM(DATA_m660, 21), DATANUM(DATA_m660, 22), DATANUM(DATA_m660, 23),
    DATANUM(DATA_m660, 24), DATANUM(DATA_m660, 25), DATANUM(DATA_m660, 26), DATANUM(DATA_m660, 27),
    DATANUM(DATA_m660, 28), DATANUM(DATA_m660, 29), DATANUM(DATA_m660, 30), DATANUM(DATA_m660, 31),
    DATANUM(DATA_m660, 32), DATANUM(DATA_m660, 33), DATANUM(DATA_m660, 34), DATANUM(DATA_m660, 35)
};
extern HuVecF Center, CRot;
extern f32 CZoom;
void fn_1_3D0(void);
void fn_1_3CCC(void);
void fn_1_AA2C(void);
void fn_1_B050(void);
void fn_1_B9D4(void);
void fn_1_2B34(void);
void fn_1_71C(void);
void fn_1_24C4(void);
void fn_1_EB8(void);
void fn_1_C594(void);
s32 fn_1_C754(void);
void fn_1_E6F4(void);
int fn_1_90A8(struct ColNarrowParam_s *first, struct ColNarrowParam_s *second);
void fn_1_9128(struct MgActor_s *actor, int value);
struct MgTimer_s *fn_1_2FA0(s32, s32, s32, f32, f32);
s32 frandmod(s32 range);
s16 fn_1_3AA4(s16 modelId, s16 lightId);
f32 fn_1_3C54(f32 range);
void fn_1_4004(HuVecF *origin);
void fn_1_42E0(M660Model *origin, s16 effectIndex);
void fn_1_6678(void);
struct VoicePosition;
void fn_1_67D4(s16 slot, s16 mode, struct VoicePosition *position);
s16 fn_1_12030(M660Player *player, s16 advanceState);
s16 fn_1_2E80(f32 *value, f32 step, f32 start, f32 end);

/* Initializes the stage, players and shared model state before the sequence starts. */
s32 fn_1_A3E4(void)
{
    s32 player;
    MGACTOR_PARAM spawn;

    for (player = 0; player < 10; player++) {
        lbl_1_bss_1F4.slotHistory[player] = -1;
    }
    lbl_1_bss_1F4.selectedSlot = -1;
    lbl_1_bss_1F4.interval = 300.0f;
    lbl_1_bss_1F4.speedRandomRange = 1.0f;
    lbl_1_bss_0[0].directions[0] = -1;
    lbl_1_bss_0[1].directions[1] = -1;
    lbl_1_bss_0[2].directions[0] = -1;
    lbl_1_bss_0[4].directions[2] = -1;
    lbl_1_bss_0[6].directions[3] = -1;
    lbl_1_bss_0[7].directions[0] = -1;
    lbl_1_bss_0[8].directions[1] = -1;
    lbl_1_bss_0[9].directions[2] = -1;
    lbl_1_bss_0[12].directions[1] = -1;

    fn_1_3D0();
    fn_1_3CCC();
    fn_1_AA2C();
    fn_1_B050();
    fn_1_B9D4();

    lbl_1_bss_60C[2].modelId =
        Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_480[0], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_60C[2].modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelCameraSet(lbl_1_bss_60C[2].modelId, 1);
    Hu3DModelLayerSet(lbl_1_bss_60C[2].modelId, 0);
    lbl_1_bss_60C[2].position.x = 0.0f;
    lbl_1_bss_60C[2].position.y = 0.0f;
    lbl_1_bss_60C[2].position.z = 0.0f;
    lbl_1_bss_60C[2].rotation.x = 0.0f;
    lbl_1_bss_60C[2].rotation.y = 0.0f;
    lbl_1_bss_60C[2].rotation.z = 0.0f;
    lbl_1_bss_1E8.modelId = lbl_1_bss_60C[2].modelId;
    MgActorColMapInit(&lbl_1_bss_1E8.modelId, 1, 10);

    spawn.height = 150.0f;
    spawn.radius = 50.0f;
    spawn.type = 0;
    spawn.attr = 0;
    spawn.narrowHook = fn_1_90A8;
    spawn.correctHookParam = 0;
    spawn.correctHook = fn_1_9128;

    for (player = 0; player < 2; player++) {
        spawn.param = player;
        lbl_1_bss_1E8.players[player] =
            MgPlayerCreate(lbl_1_bss_2DC[player].selectedPlayer, &spawn,
                           2, 1, -63, 0);
        lbl_1_bss_60C[player].modelId =
            lbl_1_bss_1E8.players[player]->actor->mdlId;
        lbl_1_bss_1E8.players[player]->actor->pos.x =
            lbl_1_bss_2DC[player].position.x;
        lbl_1_bss_1E8.players[player]->actor->pos.y =
            lbl_1_bss_2DC[player].position.y;
        lbl_1_bss_1E8.players[player]->actor->pos.z =
            lbl_1_bss_2DC[player].position.z;
        lbl_1_bss_1E8.players[player]->actor->rotY =
            lbl_1_bss_2DC[player].rotation.y;
        Hu3DModelShadowSet(lbl_1_bss_60C[player].modelId);
        Hu3DModelShadowDispOn(lbl_1_bss_1E8.players[player]->actor->mdlId);
        Hu3DModelLayerSet(lbl_1_bss_1E8.players[player]->actor->mdlId, 1);
        Hu3DMotionTimeSet(lbl_1_bss_60C[player].modelId, 20.0f);
        CharModelStepSet(lbl_1_bss_2DC[player].character, 0);
        lbl_1_bss_2DC[player].state = 1;
        lbl_1_bss_2DC[player].collisionActive = 1;
        lbl_1_bss_2DC[player].inputDisabled = 1;
        lbl_1_bss_2DC[player].contacts[1] =
            lbl_1_bss_2DC[player].contacts[0] = -1;
        lbl_1_bss_2DC[player].contactEdges[1] =
            lbl_1_bss_2DC[player].contactEdges[0] = -1;
    }
    CharEffectLayerSet(2);
    fn_1_2B34();
    fn_1_EB8();
    CharMotionUpdateSet(lbl_1_bss_2DC[0].character, DATANUM(DATA_mariomot, 34), 0);
    CharMotionUpdateSet(lbl_1_bss_2DC[1].character, DATANUM(DATA_mariomot, 34), 0);
    fn_1_C594();
    fn_1_E6F4();
    lbl_1_bss_1F4.gameTimer =
        fn_1_2FA0(0, 30, 0, 288.0f, 64.0f);
    return 0;
}

/* Advances the opening scene each sequence tick and reports when it is complete. */
s16 fn_1_A9B0() {
    s16 result = 0;
    result = fn_1_C754();
    fn_1_71C();
    fn_1_2B34();
    fn_1_24C4();
    return result;
}

/* Continues opening-scene updates; this callback always returns zero to hold the phase. */
s16 fn_1_A9F0() {
    s16 result = 0;
    fn_1_C754();
    fn_1_71C();
    fn_1_2B34();
    fn_1_24C4();
    return result;
}

/* Called by fn_1_A3E4 to load stage models and apply their initial motion and shadow settings. */
void fn_1_AA2C(void)
{
    ShadowPositionTable_AA2C shadowPositions = {{
        {-650.0f, 0.0f, -650.0f},
        {-225.0f, 0.0f, -650.0f},
        {225.0f, 0.0f, -650.0f},
        {650.0f, 0.0f, -650.0f},
        {-650.0f, 0.0f, -225.0f},
        {-225.0f, 0.0f, -225.0f},
        {225.0f, 0.0f, -225.0f},
        {650.0f, 0.0f, -225.0f},
        {-650.0f, 0.0f, 225.0f},
        {-225.0f, 0.0f, 225.0f},
        {225.0f, 0.0f, 225.0f},
        {650.0f, 0.0f, 225.0f},
        {-650.0f, 0.0f, 650.0f},
        {-225.0f, 0.0f, 650.0f},
        {225.0f, 0.0f, 650.0f},
        {650.0f, 0.0f, 650.0f}
    }};
    s16 modelIndex;
    s16 fileIndex;
    s16 shadowPositionIndex;

    modelIndex = 3;
    fileIndex = 1;
    shadowPositionIndex = 0;
    while (modelIndex <= 21) {
        lbl_1_bss_60C[modelIndex].modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_480[fileIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_60C[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_60C[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_60C[modelIndex].modelId, 1);
        Hu3DModelLayerSet(lbl_1_bss_60C[modelIndex].modelId, 0);
        if (modelIndex == 14 || modelIndex == 15 || modelIndex == 16 ||
            modelIndex == 17) {
            Hu3DModelLayerSet(lbl_1_bss_60C[modelIndex].modelId, 7);
        }
        if (modelIndex == 3) {
            Hu3DModelShadowMapSet(lbl_1_bss_60C[modelIndex].modelId);
            lbl_1_bss_60C[modelIndex].attachment =
                shadowPositions.positions[shadowPositionIndex];
        }
        Hu3DMotionTimeSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        if (modelIndex == 18 || modelIndex == 19 || modelIndex == 21) {
            Hu3DModelAttrSet(lbl_1_bss_60C[modelIndex].modelId, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(lbl_1_bss_60C[modelIndex].modelId,
                               1.0f);
            if (modelIndex == 21) {
                Hu3DMotionTimeSet(lbl_1_bss_60C[modelIndex].modelId,
                                  1100.0f);
            }
        } else {
            Hu3DModelAttrReset(lbl_1_bss_60C[modelIndex].modelId, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(lbl_1_bss_60C[modelIndex].modelId,
                               0.0f);
        }
        lbl_1_bss_60C[modelIndex].position.x = 0.0f;
        lbl_1_bss_60C[modelIndex].position.y = 0.0f;
        lbl_1_bss_60C[modelIndex].position.z = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.z = 0.0f;
        modelIndex++;
        fileIndex++;
        shadowPositionIndex++;
    }

    modelIndex = 22;
    fileIndex = 20;
    while (modelIndex <= 30) {
        lbl_1_bss_60C[modelIndex].modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_480[fileIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_60C[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_60C[modelIndex].modelId, 1);
        Hu3DModelLayerSet(lbl_1_bss_60C[modelIndex].modelId, 0);
        Hu3DMotionTimeSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        Hu3DModelAttrReset(lbl_1_bss_60C[modelIndex].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        if (modelIndex == 22) {
            Hu3DModelShadowMapSet(lbl_1_bss_60C[modelIndex].modelId);
        }
        lbl_1_bss_60C[modelIndex].position.x = 0.0f;
        lbl_1_bss_60C[modelIndex].position.y = 0.0f;
        lbl_1_bss_60C[modelIndex].position.z = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.z = 0.0f;
        modelIndex++;
        fileIndex++;
    }
}

const BoundsPresets_B050 lbl_1_rodata_59C = {
    { { 100.0f, 0.0f, 25.0f }, { 125.0f, 0.0f, 25.0f }, { 150.0f, 0.0f, 25.0f } }
};
const BoundaryPlanes_B050 lbl_1_rodata_5C0 = {
    { { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } }
};
const StageLayouts_B050 lbl_1_rodata_5F0 = {{
    {{-625.0f, 0.0f, -350.0f}, {0.0f, 90.0f, 0.0f}},
    {{-625.0f, 0.0f, -150.0f}, {0.0f, 90.0f, 0.0f}},
    {{-125.0f, 0.0f, -750.0f}, {0.0f, 90.0f, 0.0f}},
    {{-50.0f, 0.0f, -625.0f}, {0.0f, 0.0f, 0.0f}},
    {{200.0f, 0.0f, -325.0f}, {0.0f, 0.0f, 0.0f}},
    {{325.0f, 0.0f, -250.0f}, {0.0f, 90.0f, 0.0f}},
    {{750.0f, 0.0f, -25.0f}, {0.0f, 0.0f, 0.0f}},
    {{625.0f, 0.0f, 50.0f}, {0.0f, 90.0f, 0.0f}},
    {{625.0f, 0.0f, 250.0f}, {0.0f, 90.0f, 0.0f}},
    {{-450.0f, 0.0f, 325.0f}, {0.0f, 0.0f, 0.0f}},
    {{-325.0f, 0.0f, 250.0f}, {0.0f, 90.0f, 0.0f}},
    {{-425.0f, 0.0f, 750.0f}, {0.0f, 90.0f, 0.0f}},
    {{375.0f, 0.0f, 750.0f}, {0.0f, 90.0f, 0.0f}},
    {{300.0f, 0.0f, 625.0f}, {0.0f, 0.0f, 0.0f}}
}};

/* Called by fn_1_A3E4 to randomize the stage turn and place its collision bounds. */
void fn_1_B050(void)
{
    StageLayouts_B050 layouts;
    BoundsPresets_B050 boundsPresets;
    BoundaryPlanes_B050 boundaryPlanes;
    s16 modelIndex;
    s16 entryIndex;
    s16 layoutIndex;
    s16 quarterTurns;
    s16 savedDirection;
    f32 rotation;
    f32 distance;
    f32 angle;
    f32 distanceSquared;

    boundsPresets = lbl_1_rodata_59C;
    boundaryPlanes = lbl_1_rodata_5C0;
    layouts = lbl_1_rodata_5F0;
    quarterTurns = frandmod(4);
    lbl_1_bss_1F4.angle = rotation = 90.0f * quarterTurns;
    for (modelIndex = 0; modelIndex < quarterTurns; modelIndex++) {
        for (entryIndex = 0; entryIndex < 14; entryIndex++) {
            savedDirection = lbl_1_bss_0[entryIndex].directions[2];
            lbl_1_bss_0[entryIndex].directions[2] = lbl_1_bss_0[entryIndex].directions[0];
            lbl_1_bss_0[entryIndex].directions[0] = lbl_1_bss_0[entryIndex].directions[3];
            lbl_1_bss_0[entryIndex].directions[3] = lbl_1_bss_0[entryIndex].directions[1];
            lbl_1_bss_0[entryIndex].directions[1] = savedDirection;
        }
    }
    modelIndex = 31;
    layoutIndex = 0;
    while (modelIndex < 45) {
        lbl_1_bss_60C[modelIndex].modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_480[30], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_60C[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_60C[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_60C[modelIndex].modelId, 1);
        Hu3DModelLayerSet(lbl_1_bss_60C[modelIndex].modelId, 0);
        Hu3DModelAttrReset(lbl_1_bss_60C[modelIndex].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionTimeSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        Hu3DModelShadowMapSet(lbl_1_bss_60C[modelIndex].modelId);
        lbl_1_bss_60C[modelIndex].position = layouts.values[layoutIndex].position;
        lbl_1_bss_60C[modelIndex].rotation = layouts.values[layoutIndex].rotation;
        lbl_1_bss_60C[modelIndex].bounds = boundsPresets.values[0];
        lbl_1_bss_60C[modelIndex].normals[0] = boundaryPlanes.values[0];
        lbl_1_bss_60C[modelIndex].normals[1] = boundaryPlanes.values[1];
        lbl_1_bss_60C[modelIndex].normals[2] = boundaryPlanes.values[2];
        lbl_1_bss_60C[modelIndex].normals[3] = boundaryPlanes.values[3];
        modelIndex++;
        layoutIndex++;
    }
    for (modelIndex = 31; modelIndex < 45; modelIndex++) {
        angle = 180.0 * (atan2(lbl_1_bss_60C[modelIndex].position.x,
                             lbl_1_bss_60C[modelIndex].position.z) / 3.141592653589793);
        distanceSquared =
            (lbl_1_bss_60C[modelIndex].position.x * lbl_1_bss_60C[modelIndex].position.x) +
            (lbl_1_bss_60C[modelIndex].position.z * lbl_1_bss_60C[modelIndex].position.z);
        distance = sqrt(distanceSquared);
        lbl_1_bss_60C[modelIndex].position.x =
            distance * sin((3.141592653589793 * (rotation + angle)) / 180.0);
        lbl_1_bss_60C[modelIndex].position.z =
            distance * cos((3.141592653589793 * (rotation + angle)) / 180.0);
        if ((s16)rotation % 180 != 0) {
            lbl_1_bss_60C[modelIndex].rotation.y =
                lbl_1_bss_60C[modelIndex].rotation.y >= 90.0f ? 0.0f : 90.0f;
        }
        lbl_1_bss_60C[modelIndex].bounds.x = lbl_1_bss_60C[modelIndex].rotation.y >= 89.0f ?
            boundsPresets.values[0].z : boundsPresets.values[0].x;
        lbl_1_bss_60C[modelIndex].bounds.z = lbl_1_bss_60C[modelIndex].rotation.y >= 89.0f ?
            boundsPresets.values[0].x : boundsPresets.values[0].z;
    }
}

/* Called by fn_1_A3E4 to load stage props and assign their scales, bounds, layers and shadows. */
void fn_1_B9D4(void)
{
    ModelScales_B9D4 scales = { { { 0.300000012f, 0.300000012f, 0.300000012f },
                                  { 0.300000012f, 0.300000012f, 0.300000012f },
                                  { 0.300000012f, 0.300000012f, 0.300000012f },
                                  { 0.300000012f, 0.300000012f, 0.300000012f },
                                  { 0.300000012f, 0.300000012f, 0.300000012f },
                                  { 0.600000024f, 0.600000024f, 0.600000024f },
                                  { 0.600000024f, 0.600000024f, 0.600000024f },
                                  { 0.600000024f, 0.600000024f, 0.600000024f },
                                  { 1.0f, 1.0f, 1.0f } } };
    ModelRadii_B9D4 radii = { { 150.0f, 150.0f, 150.0f, 150.0f, 150.0f, 280.0f, 280.0f, 280.0f,
                                460.0f } };
    s16 modelIndex;
    s16 entryIndex;

    modelIndex = 45;
    entryIndex = 0;
    while (modelIndex <= 53) {
        lbl_1_bss_60C[modelIndex].modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_480[31], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_60C[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_60C[modelIndex].modelId, 1);
        Hu3DModelLayerSet(lbl_1_bss_60C[modelIndex].modelId, 0);
        Hu3DModelScaleSet(lbl_1_bss_60C[modelIndex].modelId, scales.values[entryIndex].x,
            scales.values[entryIndex].y, scales.values[entryIndex].z);
        Hu3DModelAttrReset(lbl_1_bss_60C[modelIndex].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionTimeSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        lbl_1_bss_60C[modelIndex].position.x = 0.0f;
        lbl_1_bss_60C[modelIndex].position.y = 0.0f;
        lbl_1_bss_60C[modelIndex].position.z = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.z = 0.0f;
        lbl_1_bss_60C[modelIndex].bounds.x = radii.values[entryIndex];
        lbl_1_bss_80[entryIndex].modelA = entryIndex + 54;
        lbl_1_bss_80[entryIndex].modelB = entryIndex + 63;
        modelIndex++;
        entryIndex++;
    }
    modelIndex = 54;
    entryIndex = 0;
    while (modelIndex <= 62) {
        lbl_1_bss_60C[modelIndex].modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_480[32], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_60C[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_60C[modelIndex].modelId, 1);
        Hu3DModelLayerSet(lbl_1_bss_60C[modelIndex].modelId, 7);
        Hu3DModelScaleSet(lbl_1_bss_60C[modelIndex].modelId, scales.values[entryIndex].x,
            scales.values[entryIndex].y, scales.values[entryIndex].z);
        Hu3DModelAttrReset(lbl_1_bss_60C[modelIndex].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionTimeSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        lbl_1_bss_60C[modelIndex].position.x = 0.0f;
        lbl_1_bss_60C[modelIndex].position.y = (f32)entryIndex / 10.0f;
        lbl_1_bss_60C[modelIndex].position.z = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.z = 0.0f;
        modelIndex++;
        entryIndex++;
    }
    modelIndex = 63;
    entryIndex = 0;
    while (modelIndex <= 71) {
        lbl_1_bss_60C[modelIndex].modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_480[31], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_60C[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_60C[modelIndex].modelId, 1);
        Hu3DModelLayerSet(lbl_1_bss_60C[modelIndex].modelId, 0);
        Hu3DModelScaleSet(lbl_1_bss_60C[modelIndex].modelId, scales.values[entryIndex].x,
            1.0f, scales.values[entryIndex].z);
        Hu3DModelAttrReset(lbl_1_bss_60C[modelIndex].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionTimeSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        Hu3DModelAttrSet(lbl_1_bss_60C[modelIndex].modelId, HU3D_ATTR_MOTION_OFF);
        Hu3DModelShadowSet(lbl_1_bss_60C[modelIndex].modelId);
        Hu3DModelShadowDispOn(lbl_1_bss_60C[modelIndex].modelId);
        lbl_1_bss_60C[modelIndex].bounds.x = radii.values[entryIndex];
        lbl_1_bss_60C[modelIndex].position.x = 0.0f;
        lbl_1_bss_60C[modelIndex].position.y = -440.0f;
        lbl_1_bss_60C[modelIndex].position.z = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.z = 0.0f;
        modelIndex++;
        entryIndex++;
    }
    Hu3DModelLayerSet(lbl_1_bss_60C[45].modelId, 3);
    Hu3DModelLayerSet(lbl_1_bss_60C[46].modelId, 4);
    Hu3DModelLayerSet(lbl_1_bss_60C[47].modelId, 5);
    Hu3DModelLayerSet(lbl_1_bss_60C[48].modelId, 6);
    Hu3DModelLayerSet(lbl_1_bss_60C[49].modelId, 7);
    Hu3DModelLayerSet(lbl_1_bss_60C[50].modelId, 3);
    Hu3DModelLayerSet(lbl_1_bss_60C[51].modelId, 4);
    Hu3DModelLayerSet(lbl_1_bss_60C[52].modelId, 5);
    Hu3DModelLayerSet(lbl_1_bss_60C[53].modelId, 3);
    modelIndex = 72;
    entryIndex = 33;
    while (modelIndex <= 74) {
        lbl_1_bss_60C[modelIndex].modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_480[entryIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_60C[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_60C[modelIndex].modelId, 1);
        Hu3DModelLayerSet(lbl_1_bss_60C[modelIndex].modelId, 1);
        Hu3DModelAttrReset(lbl_1_bss_60C[modelIndex].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionTimeSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_60C[modelIndex].modelId, 0.0f);
        Hu3DModelShadowSet(lbl_1_bss_60C[modelIndex].modelId);
        Hu3DModelShadowDispOn(lbl_1_bss_60C[modelIndex].modelId);
        lbl_1_bss_60C[modelIndex].position.x = 0.0f;
        lbl_1_bss_60C[modelIndex].position.y = 100.0f;
        lbl_1_bss_60C[modelIndex].position.z = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_60C[modelIndex].rotation.z = 0.0f;
        modelIndex++;
        entryIndex++;
    }
}

/* Called by fn_1_A3E4 to create the stage point light and attempt to add it to all 75 model light
 * lists. */
void fn_1_C594(void)
{
    M660SceneLight *light;
    HU3D_MODEL *model;
    s16 modelIndex;

    light = &lbl_1_bss_38;
    light->modelId = lbl_1_bss_60C[3].modelId;
    light->unused08 = 0;
    light->position.x = 0.0f;
    light->position.y = 573.0f;
    light->position.z = 0.0f;
    light->unused24 = -60.0f;
    light->unused28 = 0.0f;
    light->unused2C = 0.0f;
    light->direction.x = 0.0f;
    light->direction.y = -0.5f;
    light->direction.z = -0.5f;
    light->color.r = 153;
    light->color.g = 153;
    light->color.b = 112;
    light->color.a = 255;
    light->referenceDistance = light->position.y;
    light->referenceBrightness = 1.0f;

    model = &Hu3DData[light->modelId];
    light->lightSlot = Hu3DLLightCreateV(
        light->modelId, &light->position, &light->direction, &light->color);
    light->lightId = model->LLightId[light->lightSlot];
    Hu3DLLightStaticSet(light->modelId, light->lightSlot, 1);
    Hu3DLLightPointSet(light->modelId, light->lightSlot,
                       light->referenceDistance, light->referenceBrightness,
                       GX_DA_STEEP);

    modelIndex = 0;
    while (modelIndex < 75) {
        fn_1_3AA4(lbl_1_bss_60C[modelIndex].modelId, light->lightId);
        modelIndex++;
    }
}

/* Called by opening callbacks to advance the camera and character animation until finished. */
s32 fn_1_C754(void)
{
    s32 result = 0;
    s16 completed = 0;
    HuVecF origins[2] = {{-788.0f, 60.0f, -788.0f}, {788.0f, 60.0f, -788.0f}};
    M660Model *models[3];
    f32 movementStep;
    s16 index;

    models[0] = &lbl_1_bss_60C[45];
    models[1] = &lbl_1_bss_60C[54];
    models[2] = &lbl_1_bss_60C[63];
    movementStep = 15.0f;
    fn_1_4004(&origins[0]);
    fn_1_4004(&origins[1]);
    switch (lbl_1_bss_1F4.openingState) {
    case 0:
        for (index = 0; index < 2; index++) {
            lbl_1_bss_544[index].previousMode = lbl_1_bss_544[index].previousAttributes = -1;
            lbl_1_bss_544[index].mode = 4;
            lbl_1_bss_544[index].speed = 1.0f;
            lbl_1_bss_544[index].time = fn_1_3C54(110.0f);
            lbl_1_bss_544[index].attributes = HU3D_MOTATTR_LOOP;
            lbl_1_bss_544[index].attributesToReset = 0;
        }
        for (index = 0; index < 3; index++) {
            Hu3DModelAttrReset(models[index]->modelId, HU3D_ATTR_DISPOFF);
            models[index]->position.x = 0.0f;
            models[index]->position.y = 0.0f;
            models[index]->position.z = -850.0f;
            models[index]->rotation.x = 0.0f;
            models[index]->rotation.y = 0.0f;
            models[index]->rotation.z = 0.0f;
        }
        fn_1_6678();
        lbl_1_bss_1F4.openingState++;
    case 1:
        if (lbl_1_bss_1F4.openingTimer++ > 140) {
            for (index = 0; index < 2; index++) {
                lbl_1_bss_544[index].shiftState = 1;
                lbl_1_bss_544[index].shiftMotion = 5;
                lbl_1_bss_544[index].shiftStart = 0.0f;
                lbl_1_bss_544[index].shiftEnd = 5.0f;
                lbl_1_bss_544[index].shiftFlags = 0;
                lbl_1_bss_544[index].attributesToReset = HU3D_MOTATTR_LOOP;
            }
            models[0]->position.y = 250.0f;
            models[1]->position.y = 250.0f;
            models[2]->position.y = -440.0f;
            lbl_1_bss_1F4.openingTimer = 0;
            lbl_1_bss_1F4.openingState++;
        }
        break;
    case 2:
        models[0]->position.z += movementStep;
        models[1]->position.z += movementStep;
        models[2]->position.z = models[1]->position.z + models[1]->position.z / 10.0f;
        fn_1_42E0(models[0], 0);
        if (lbl_1_bss_544[0].motionEnded != 0) {
            lbl_1_bss_2DC[0].currentPosition.x = -350.0f;
            lbl_1_bss_2DC[1].currentPosition.x = 350.0f;
            for (index = 0; index < 2; index++) {
                lbl_1_bss_2DC[index].currentPosition.z = 0.0f;
                lbl_1_bss_2DC[index].forwardSpeed = 10.0f;
                lbl_1_bss_2DC[index].arrivalRadius = 10.1f;
            }
            fn_1_67D4(0, 5, 0);
            lbl_1_bss_1F4.openingState++;
        }
        break;
    case 3:
        models[0]->position.z += movementStep;
        models[1]->position.z += movementStep;
        models[2]->position.z = models[1]->position.z + models[1]->position.z / 10.0f;
        fn_1_42E0(models[0], 0);
        /* Both players are updated, but only player 1's completion result controls the
         * transition. */
        for (index = 0; index < 2; index++) {
            completed = fn_1_12030(&lbl_1_bss_2DC[index], 0);
        }
        if ((s16)completed != 0) {
            for (index = 0; index < 2; index++) {
                MgPlayerComStkOff(lbl_1_bss_1E8.players[index]);
                lbl_1_bss_2DC[index].chaseState = 0;
                lbl_1_bss_544[index].mode = 1;
                lbl_1_bss_544[index].previousMode = -1;
                lbl_1_bss_544[index].time = Hu3DMotionTimeGet(lbl_1_bss_60C[index].modelId);
                lbl_1_bss_544[index].previousTime = -1.0f;
            }
            lbl_1_bss_1F4.openingState++;
        }
        break;
    case 4:
    {
        f32 step;
        models[0]->position.z += movementStep;
        models[1]->position.z += movementStep;
        models[2]->position.z = models[1]->position.z + models[1]->position.z / 10.0f;
        fn_1_42E0(models[0], 0);
        step = 20.0f;
        completed = fn_1_2E80(&lbl_1_bss_1E8.players[0]->actor->rotY, step, -90.0f, 0.0f);
        completed += fn_1_2E80(&lbl_1_bss_1E8.players[1]->actor->rotY, step, 90.0f, 0.0f);
        if ((s16)completed == 0) {
            if (lbl_1_bss_1F4.openingTimer == 0) {
                lbl_1_bss_544[0].shiftState = 1;
                lbl_1_bss_544[0].shiftMotion = 0;
                lbl_1_bss_544[0].shiftStart = 0.0f;
                lbl_1_bss_544[0].shiftEnd = 5.0f;
                lbl_1_bss_544[0].shiftFlags = HU3D_MOTATTR_LOOP;
                lbl_1_bss_544[0].attributesToReset = 0;
                lbl_1_bss_544[1].shiftState = 1;
                lbl_1_bss_544[1].shiftMotion = 0;
                lbl_1_bss_544[1].shiftStart = 0.0f;
                lbl_1_bss_544[1].shiftEnd = 5.0f;
                lbl_1_bss_544[1].shiftFlags = HU3D_MOTATTR_LOOP;
                lbl_1_bss_544[1].attributesToReset = 0;
            }
            lbl_1_bss_1F4.openingTimer = 1;
        }
        step = 40.0f;
        completed += fn_1_2E80(&Center.x, step, 0.0f, 0.0f);
        completed += fn_1_2E80(&Center.y, step, 101.0f, 132.0f);
        completed += fn_1_2E80(&Center.z, step, 61.0f, 1.6f);
        completed += fn_1_2E80(&CRot.x, step, -78.0f, -89.9f);
        completed += fn_1_2E80(&CRot.y, step, 0.0f, 0.0f);
        completed += fn_1_2E80(&CRot.z, step, 0.0f, 0.0f);
        completed += fn_1_2E80(&CZoom, step, 1039.1f, 2342.1f);
        if ((s16)completed == 0) {
            lbl_1_bss_1F4.openingTimer = 0;
            lbl_1_bss_1F4.openingState++;
        }
        break;
    }
    case 5:
        models[0]->position.z += movementStep;
        models[1]->position.z += movementStep;
        models[2]->position.z = models[1]->position.z + models[1]->position.z / 10.0f;
        fn_1_42E0(models[0], 0);
        if (models[0]->position.z > 1400.0f) {
            lbl_1_bss_1F4.openingState++;
        }
        break;
    case 6:
        result = 1;
        break;
    }
    return result;
}
