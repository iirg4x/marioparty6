/* Scene model loading, board geometry, lighting, and transition callbacks. */
#include "dolphin/math.h"

#include "dolphin/types.h"

#include "REL/m617dll/shared.h"

#include "game/main.h"

#include "game/charman.h"

#include "game/mg/actman.h"

#include "game/mg/timer.h"

#include "game/data.h"

#include "game/hu3d.h"

#include "game/memory.h"

#include "game/object.h"

#include "datadir_enum.h"

/* Model resources shared by the scene and its player attachments. */
u32 lbl_1_data_118[51] = {
    DATANUM(DATA_m617, 0), DATANUM(DATA_m617, 1), DATANUM(DATA_m617, 2), DATANUM(DATA_m617, 3),
    DATANUM(DATA_m617, 4), DATANUM(DATA_m617, 5), DATANUM(DATA_m617, 6), DATANUM(DATA_m617, 7),
    DATANUM(DATA_m617, 8), DATANUM(DATA_m617, 9), DATANUM(DATA_m617, 10), DATANUM(DATA_m617, 11),
    DATANUM(DATA_m617, 12), DATANUM(DATA_m617, 13), DATANUM(DATA_m617, 14), DATANUM(DATA_m617, 15),
    DATANUM(DATA_m617, 16), DATANUM(DATA_m617, 17), DATANUM(DATA_m617, 18), DATANUM(DATA_m617, 19),
    DATANUM(DATA_m617, 20), DATANUM(DATA_m617, 21), DATANUM(DATA_m617, 22), DATANUM(DATA_m617, 23),
    DATANUM(DATA_m617, 24), DATANUM(DATA_m617, 25), DATANUM(DATA_m617, 26), DATANUM(DATA_m617, 27),
    DATANUM(DATA_m617, 28), DATANUM(DATA_m617, 29), DATANUM(DATA_m617, 30), DATANUM(DATA_m617, 31),
    DATANUM(DATA_m617, 32), DATANUM(DATA_m617, 33), DATANUM(DATA_m617, 34), DATANUM(DATA_m617, 35),
    DATANUM(DATA_m617, 36), DATANUM(DATA_m617, 37), DATANUM(DATA_m617, 38), DATANUM(DATA_m617, 39),
    DATANUM(DATA_m617, 40), DATANUM(DATA_m617, 41), DATANUM(DATA_m617, 42), DATANUM(DATA_m617, 43),
    DATANUM(DATA_m617, 44), DATANUM(DATA_m617, 45), DATANUM(DATA_m617, 46), DATANUM(DATA_m617, 47),
    DATANUM(DATA_m617, 48), DATANUM(DATA_m617, 49), DATANUM(DATA_m617, 50),
};

char lbl_1_data_1E4[] = "pattern %d rot%d\n";

char lbl_1_data_1F6[] = "%d ";

char lbl_1_data_1FA[] = "\n";

typedef struct M617PlayerSetupParam {
    f32 height;
    f32 radius;
    s32 param;
    s32 type;
    COL_NARROW_HOOK narrowHook;
    u32 attr;
    s32 correctHookParam;
    MGACTOR_CORRECT_HOOK correctHook;
    MGACTOR_CORRECT_HOOK correctionCallbacks[16];
} M617PlayerSetupParam;

s32 fn_1_780C(COL_NARROW_PARAM *first, COL_NARROW_PARAM *second);

void fn_1_788C(MGACTOR *task);

typedef struct M617BoxEdgeFlags {
    s8 flags[2][4];
} M617BoxEdgeFlags;

void fn_1_78C0(void);

void fn_1_2B48(void);

MGTIMER *fn_1_2FB4(int type, int duration, int end, f32 x, f32 y);

void fn_1_3DE0(void);

void fn_1_3E4(void);

void fn_1_A438(void);

void fn_1_AA84(void);

void fn_1_187C8(void);

void fn_1_BA64(void);

void fn_1_E1F0();

void fn_1_ECC(void);

/* Called from the sequence setup to load scene models and configure both players. */
s32 fn_1_93D0(void) {
    unsigned int motionData[17] = {
        DATANUM(DATA_mariomot, 95), DATANUM(DATA_mariomot, 96), DATANUM(DATA_mariomot, 97),
        DATANUM(DATA_mariomot, 3), DATANUM(DATA_mariomot, 4),
        DATANUM(DATA_mariomot, 5), DATANUM(DATA_mariomot, 56), DATANUM(DATA_mariomot, 8),
        DATANUM(DATA_mariomot, 10), DATANUM(DATA_mariomot, 9),
        DATANUM(DATA_mariomot, 15), DATANUM(DATA_mariomot, 16), DATANUM(DATA_mariomot, 17),
        DATANUM(DATA_mariomot, 18), DATANUM(DATA_mariomot, 22),
        DATANUM(DATA_mariomot, 22), 0
    };
    MGACTOR_PARAM actorParam;
    s32 hookIndex;
    s32 index;

    lbl_1_bss_398[0].links[0][0] = -1;
    lbl_1_bss_398[1].links[0][1] = -1;
    lbl_1_bss_398[4].links[0][2] = -1;
    lbl_1_bss_398[6].links[0][2] = -1;
    lbl_1_bss_398[7].links[0][2] = -1;
    lbl_1_bss_398[7].links[0][3] = -1;
    lbl_1_bss_398[9].links[0][2] = -1;
    lbl_1_bss_398[10].links[0][3] = -1;
    lbl_1_bss_398[12].links[0][3] = -1;
    lbl_1_bss_398[14].links[0][3] = -1;
    lbl_1_bss_398[15].links[0][0] = -1;
    lbl_1_bss_398[18].links[0][1] = -1;
    lbl_1_bss_398[19].links[0][2] = -1;
    lbl_1_bss_398[20].links[0][3] = -1;
    lbl_1_bss_398[0].links[1][0] = -1;
    lbl_1_bss_398[1].links[1][1] = -1;
    lbl_1_bss_398[3].links[1][0] = -1;
    lbl_1_bss_398[4].links[1][1] = -1;
    lbl_1_bss_398[5].links[1][2] = -1;
    lbl_1_bss_398[5].links[1][3] = -1;
    lbl_1_bss_398[6].links[1][3] = -1;
    lbl_1_bss_398[8].links[1][2] = -1;
    lbl_1_bss_398[9].links[1][3] = -1;
    lbl_1_bss_398[11].links[1][2] = -1;
    lbl_1_bss_398[12].links[1][3] = -1;
    lbl_1_bss_398[14].links[1][3] = -1;
    lbl_1_bss_398[15].links[1][1] = -1;
    lbl_1_bss_398[15].links[1][0] = -1;
    lbl_1_bss_398[16].links[1][2] = -1;
    lbl_1_bss_398[17].links[1][1] = -1;
    lbl_1_bss_398[18].links[1][2] = -1;
    lbl_1_bss_398[19].links[1][2] = -1;
    lbl_1_bss_398[19].links[1][3] = -1;
    lbl_1_bss_398[21].links[1][3] = -1;
    fn_1_3E4();
    fn_1_3DE0();
    fn_1_A438();
    fn_1_AA84();
    lbl_1_bss_E88[2].modelId =
        Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_118[0], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_E88[2].modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelCameraSet(lbl_1_bss_E88[2].modelId, 1U);
    Hu3DModelLayerSet(lbl_1_bss_E88[2].modelId, 0);
    lbl_1_bss_E88[2].position.x = 0.0f;
    lbl_1_bss_E88[2].position.y = 0.0f;
    lbl_1_bss_E88[2].position.z = 0.0f;
    lbl_1_bss_E88[2].rotation.x = 0.0f;
    lbl_1_bss_E88[2].rotation.y = 0.0f;
    lbl_1_bss_E88[2].rotation.z = 0.0f;
    index = 83;
    while (index <= 91) {
        lbl_1_bss_E88[index].modelId =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_118[47], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_E88[index].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_E88[index].modelId, 1U);
        Hu3DModelLayerSet(lbl_1_bss_E88[index].modelId, 7);
        lbl_1_bss_E88[index].position.x = 0.0f;
        lbl_1_bss_E88[index].position.y = 0.0f;
        lbl_1_bss_E88[index].position.z = 0.0f;
        lbl_1_bss_E88[index].rotation.x = 0.0f;
        lbl_1_bss_E88[index].rotation.y = 0.0f;
        lbl_1_bss_E88[index].rotation.z = 0.0f;
        Hu3DModelScaleSet(lbl_1_bss_E88[index].modelId, 0.0f, 0.0f, 0.0f);
        index += 1;
    }
    index = 81;
    while (index <= 82) {
        lbl_1_bss_E88[index].modelId =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_118[48], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_E88[index].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_E88[index].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_E88[index].modelId, 1U);
        Hu3DModelLayerSet(lbl_1_bss_E88[index].modelId, 6);
        Hu3DModelShadowMapSet(lbl_1_bss_E88[index].modelId);
        lbl_1_bss_E88[index].position.x = 0.0f;
        lbl_1_bss_E88[index].position.y = 0.0f;
        lbl_1_bss_E88[index].position.z = 0.0f;
        lbl_1_bss_E88[index].rotation.x = 0.0f;
        lbl_1_bss_E88[index].rotation.y = 0.0f;
        lbl_1_bss_E88[index].rotation.z = 0.0f;
        Hu3DModelScaleSet(lbl_1_bss_E88[index].modelId, 0.0f, 0.0f, 0.0f);
        lbl_1_bss_E88[index].motionState = 1;
        index += 1;
    }
    Hu3DModelLayerSet(lbl_1_bss_E88[81].modelId, 5);
    lbl_1_bss_E88[81].motionState = 0;
    index = 92;
    while (index <= 93) {
        lbl_1_bss_E88[index].modelId =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_118[49], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_E88[index].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_E88[index].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_E88[index].modelId, 1U);
        Hu3DModelLayerSet(lbl_1_bss_E88[index].modelId, 0);
        Hu3DModelShadowSet(lbl_1_bss_E88[index].modelId);
        Hu3DModelShadowDispOn(lbl_1_bss_E88[index].modelId);
        lbl_1_bss_E88[index].position.x = 0.0f;
        lbl_1_bss_E88[index].position.y = 0.0f;
        lbl_1_bss_E88[index].position.z = 0.0f;
        lbl_1_bss_E88[index].rotation.x = 0.0f;
        lbl_1_bss_E88[index].rotation.y = 0.0f;
        lbl_1_bss_E88[index].rotation.z = 0.0f;
        index += 1;
    }
    index = 94;
    while (index <= 95) {
        lbl_1_bss_E88[index].modelId =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_118[50], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_E88[index].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_E88[index].modelId, 1U);
        Hu3DModelLayerSet(lbl_1_bss_E88[index].modelId, 2);
        Hu3DModelAttrReset(lbl_1_bss_E88[index].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionTimeSet(lbl_1_bss_E88[index].modelId, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_E88[index].modelId, 0.0f);
        lbl_1_bss_E88[index].position.x = 0.0f;
        lbl_1_bss_E88[index].position.y = 0.0f;
        lbl_1_bss_E88[index].position.z = 0.0f;
        lbl_1_bss_E88[index].rotation.x = 0.0f;
        lbl_1_bss_E88[index].rotation.y = 0.0f;
        lbl_1_bss_E88[index].rotation.z = 0.0f;
        index += 1;
    }
    lbl_1_bss_9BC.collisionModel = lbl_1_bss_E88[2].modelId;
    MgActorColMapInit(&lbl_1_bss_9BC.collisionModel, 1, 10);
    actorParam.height = 150.0f;
    actorParam.radius = 50.0f;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.narrowHook = (int (*)(COL_NARROW_PARAM *, COL_NARROW_PARAM *))fn_1_780C;
    actorParam.correctHookParam = 0;
    actorParam.correctHook = (void (*)(MGACTOR *, int))fn_1_788C;
    index = 0;
    hookIndex = 92;
    while (index < 2) {
        actorParam.param = index;
        lbl_1_bss_9BC.players[index] = MgPlayerCreate((s16) lbl_1_bss_AB0[index].playerIndex,
                                                      &actorParam, 1, 1U, -63, motionData);
        lbl_1_bss_E88[index].modelId = (s16) lbl_1_bss_9BC.players[index]->actor->mdlId;
        lbl_1_bss_9BC.players[index]->actor->pos.x = lbl_1_bss_AB0[index].position.x;
        lbl_1_bss_9BC.players[index]->actor->pos.y = lbl_1_bss_AB0[index].position.y;
        lbl_1_bss_9BC.players[index]->actor->pos.z = lbl_1_bss_AB0[index].position.z;
        lbl_1_bss_9BC.players[index]->actor->rotY = lbl_1_bss_AB0[index].rotation.y;
        Hu3DModelShadowSet(lbl_1_bss_E88[index].modelId);
        Hu3DModelShadowDispOn((s16) lbl_1_bss_9BC.players[index]->actor->mdlId);
        Hu3DModelLayerSet((s16) lbl_1_bss_9BC.players[index]->actor->mdlId, 0);
        Hu3DMotionTimeSet(lbl_1_bss_E88[index].modelId, 20.0f);
        CharModelStepSet((s16) lbl_1_bss_AB0[index].character, 0);
        lbl_1_bss_AB0[index].transformState = 1;
        lbl_1_bss_AB0[index].characterMode = 1;
        lbl_1_bss_AB0[index].motionState = 1;
        lbl_1_bss_AB0[index].collisionModels[1] = lbl_1_bss_AB0[index].collisionModels[0] = -1;
        lbl_1_bss_AB0[index].collisionEdges[1] = lbl_1_bss_AB0[index].collisionEdges[0] = -1;
        lbl_1_bss_458[index].enabled = (s16) index;
        lbl_1_bss_AB0[index].itemHook =
            CharModelItemHookGet((s16) lbl_1_bss_AB0[index].character, 1, 0);
        Hu3DModelHookSet(lbl_1_bss_E88[index].modelId, lbl_1_bss_AB0[index].itemHook,
                         lbl_1_bss_E88[hookIndex].modelId);
        CharModelVoiceFlagSet((s16) lbl_1_bss_AB0[index].character, 1);
        index += 1;
        hookIndex += 1;
    }
    CharEffectLayerSet(7);
    index = 2;
    while (index < 96) {
        Hu3DModelAmbSet(lbl_1_bss_E88[index].modelId, 0.0f, 0.0f, 0.0f);
        index += 1;
    }
    lbl_1_bss_AB0->ambient = 0.1f;
    Hu3DModelAmbSet(lbl_1_bss_E88->modelId, lbl_1_bss_AB0->ambient, lbl_1_bss_AB0->ambient,
                    lbl_1_bss_AB0->ambient);
    Hu3DModelAmbSet(lbl_1_bss_E88[1].modelId, 0.5f, 0.5f, 0.5f);
    Hu3DModelAmbSet(lbl_1_bss_E88[92].modelId, 0.5f, 0.5f, 0.5f);
    Hu3DModelAmbSet(lbl_1_bss_E88[93].modelId, 0.5f, 0.5f, 0.5f);
    fn_1_2B48();
    fn_1_ECC();
    fn_1_BA64();
    fn_1_187C8();
    fn_1_E1F0();
    lbl_1_bss_9C8.timer =
        fn_1_2FB4(0, 30, 0, 288.0f, 64.0f);
    return 0;
}

s32 fn_1_CCB4(void);

void fn_1_5DB4(void);

void fn_1_24D8(void);

/* MgSeq calls this phase callback each frame and advances when it returns nonzero. */
s16 fn_1_A3BC() {
    s16 result = 0;
    result = fn_1_CCB4();
    fn_1_5DB4();
    fn_1_2B48();
    fn_1_24D8();
    return result;
}

/* MgSeq calls this phase callback each frame; it ignores transition completion. */
s16 fn_1_A3FC() {
    s16 result = 0;
    fn_1_CCB4();
    fn_1_5DB4();
    fn_1_2B48();
    fn_1_24D8();
    return result;
}

/* Board model placement and local light setup. */

typedef Vec M617ModelPosition;

typedef struct M617ShadowPositionTable {
    M617ModelPosition positions[16];
} M617ShadowPositionTable;

const f32 lbl_1_rodata_3A4[48] = {
    -650.0f, 0.0f, -650.0f, -225.0f,
    0.0f, -650.0f, 225.0f, 0.0f,
    -650.0f, 650.0f, 0.0f, -650.0f,
    -650.0f, 0.0f, -225.0f, -225.0f,
    0.0f, -225.0f, 225.0f, 0.0f,
    -225.0f, 650.0f, 0.0f, -225.0f,
    -650.0f, 0.0f, 225.0f, -225.0f,
    0.0f, 225.0f, 225.0f, 0.0f,
    225.0f, 650.0f, 0.0f, 225.0f,
    -650.0f, 0.0f, 650.0f, -225.0f,
    0.0f, 650.0f, 225.0f, 0.0f,
    650.0f, 650.0f, 0.0f, 650.0f
};

/* fn_1_93D0 calls this to load static board models and set their initial state. */
void fn_1_A438(void)
{
    M617ShadowPositionTable shadowPositions;
    s16 modelIndex;
    s16 fileIndex;
    s16 shadowPositionIndex;

    shadowPositions = *(M617ShadowPositionTable *)lbl_1_rodata_3A4;
    modelIndex = 3;
    fileIndex = 1;
    shadowPositionIndex = 0;
        while (modelIndex <= 36) {
        lbl_1_bss_E88[modelIndex].modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_118[fileIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_E88[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_E88[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_E88[modelIndex].modelId, 1);
        if (modelIndex >= 33 && modelIndex <= 36) {
            Hu3DModelLayerSet(lbl_1_bss_E88[modelIndex].modelId, 1);
            Hu3DModelTPLvlSet(lbl_1_bss_E88[modelIndex].modelId, 0.5f);
        } else {
            Hu3DModelLayerSet(lbl_1_bss_E88[modelIndex].modelId, 0);
        }
        if (modelIndex >= 3 && modelIndex <= 18) {
            Hu3DModelShadowMapSet(lbl_1_bss_E88[modelIndex].modelId);
            lbl_1_bss_E88[modelIndex].secondaryPosition =
                shadowPositions.positions[shadowPositionIndex];
        }
        Hu3DMotionTimeSet(lbl_1_bss_E88[modelIndex].modelId, 0.0f);
        if (modelIndex == 29 || modelIndex == 30 || modelIndex == 32) {
            Hu3DModelAttrSet(lbl_1_bss_E88[modelIndex].modelId, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(lbl_1_bss_E88[modelIndex].modelId, 1.0f);
            if (modelIndex == 32) {
                Hu3DMotionTimeSet(lbl_1_bss_E88[modelIndex].modelId, 1100.0f);
            }
        } else {
            Hu3DModelAttrReset(lbl_1_bss_E88[modelIndex].modelId, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(lbl_1_bss_E88[modelIndex].modelId, 0.0f);
        }
        lbl_1_bss_E88[modelIndex].position.x = 0.0f;
        lbl_1_bss_E88[modelIndex].position.y = 0.0f;
        lbl_1_bss_E88[modelIndex].position.z = 0.0f;
        lbl_1_bss_E88[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_E88[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_E88[modelIndex].rotation.z = 0.0f;
        modelIndex++;
        fileIndex++;
        shadowPositionIndex++;
    }

    Hu3DModelAttrSet(lbl_1_bss_E88[24].modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelTPLvlSet(lbl_1_bss_E88[24].modelId, 0.5f);
    {
        modelIndex = 37;
        fileIndex = 35;
        while (modelIndex <= 45) {
            lbl_1_bss_E88[modelIndex].modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_118[fileIndex], HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelAttrSet(lbl_1_bss_E88[modelIndex].modelId, HU3D_ATTR_DISPOFF);
            Hu3DModelCameraSet(lbl_1_bss_E88[modelIndex].modelId, 1);
            Hu3DModelLayerSet(lbl_1_bss_E88[modelIndex].modelId, 0);
            Hu3DMotionTimeSet(lbl_1_bss_E88[modelIndex].modelId, 0.0f);
            Hu3DModelAttrReset(lbl_1_bss_E88[modelIndex].modelId, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(lbl_1_bss_E88[modelIndex].modelId, 0.0f);
            lbl_1_bss_E88[modelIndex].position.x = 0.0f;
            lbl_1_bss_E88[modelIndex].position.y = 0.0f;
            lbl_1_bss_E88[modelIndex].position.z = 0.0f;
            lbl_1_bss_E88[modelIndex].rotation.x = 0.0f;
            lbl_1_bss_E88[modelIndex].rotation.y = 0.0f;
            lbl_1_bss_E88[modelIndex].rotation.z = 0.0f;
            modelIndex++;
            fileIndex++;
        }
    }
}

extern s32 lbl_1_rodata_46C;

extern s32 lbl_1_rodata_4B4;

extern s32 lbl_1_rodata_4D8;

#define setupTableEntryAtIndex0 setupTables[0]

#define setupTableEntryAtIndex2 setupTables[2]

#define setupTableEntryAtIndex70 setupTables[70]

#define setupTableEntryAtIndex74 setupTables[74]

#define setupTableEntryAtIndex382 setupTables[382]

#define setupTableEntryAtIndex386 setupTables[386]

s16 fn_1_3BB8(s16, s16);

void fn_1_7048(void);

extern s32 lbl_1_rodata_B58;

extern s32 lbl_1_rodata_B5C;

extern s32 lbl_1_rodata_B8C;

/* fn_1_93D0 calls this to load board pieces and place obstacles for the layout. */
void fn_1_AA84(void)
{
    s16 limits[9][4] = {
        {0, 4, 0, -1},
        {0, 4, 1, 2},
        {0, 4, 3, 4},
        {5, -1, 5, 6},
        {6, -1, 7, 8},
        {7, 8, 9, -1},
        {9, 10, 10, -1},
        {11, -1, 11, -1},
        {12, -1, 12, -1}
    };
    M617ModelPosition shapeParameters[3] = {
        {100.0f, 0.0f, 25.0f},
        {125.0f, 0.0f, 25.0f},
        {150.0f, 0.0f, 25.0f}
    };
    M617ModelPosition edgeDirections[4] = {
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, -1.0f},
        {1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f}
    };
    M617ModelPosition nodePositions[13][2] = {
        {
        {-520.0f, 0.0f, -315.0f},
        {-530.0f, 0.0f, -335.0f}
    },
        {
        {-80.0f, 0.0f, -775.0f},
        {-95.0f, 0.0f, -645.0f}
    },
        {
        {405.0f, 0.0f, -535.0f},
        {-110.0f, 0.0f, -500.0f}
    },
        {
        {-165.0f, 0.0f, -430.0f},
        {-670.0f, 0.0f, 140.0f}
    },
        {
        {-25.0f, 0.0f, -190.0f},
        {-520.0f, 0.0f, 140.0f}
    },
        {
        {630.0f, 0.0f, -260.0f},
        {-150.0f, 0.0f, -100.0f}
    },
        {
        {-775.0f, 0.0f, 220.0f},
        {0.0f, 0.0f, -95.0f}
    },
        {
        {-170.0f, 0.0f, 320.0f},
        {500.0f, 0.0f, 10.0f}
    },
        {
        {-10.0f, 0.0f, 225.0f},
        {780.0f, 0.0f, -140.0f}
    },
        {
        {510.0f, 0.0f, 195.0f},
        {200.0f, 0.0f, 405.0f}
    },
        {
        {665.0f, 0.0f, 140.0f},
        {-45.0f, 0.0f, 605.0f}
    },
        {
        {-340.0f, 0.0f, 665.0f},
        {-440.0f, 0.0f, 540.0f}
    },
        {
        {210.0f, 0.0f, 780.0f},
        {385.0f, 0.0f, 780.0f}
    }
    };
    M617ModelPosition obstacleBounds[26][4] = {
        {
        {-445.0f, 0.0f, -520.0f},
        {0.0f, 90.0f, 0.0f},
        {-455.0f, 0.0f, -535.0f},
        {0.0f, 90.0f, 0.0f}
    },
        {
        {-445.0f, 0.0f, -270.0f},
        {0.0f, 90.0f, 0.0f},
        {-605.0f, 0.0f, -285.0f},
        {0.0f, 90.0f, 0.0f}
    },
        {
        {-520.0f, 0.0f, -395.0f},
        {0.0f, 0.0f, 0.0f},
        {-530.0f, 0.0f, -410.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {-150.0f, 0.0f, -750.0f},
        {0.0f, 90.0f, 0.0f},
        {-175.0f, 0.0f, -750.0f},
        {0.0f, 90.0f, 0.0f}
    },
        {
        {360.0f, 0.0f, -460.0f},
        {0.0f, 0.0f, 0.0f},
        {-175.0f, 0.0f, -550.0f},
        {0.0f, 90.0f, 0.0f}
    },
        {
        {485.0f, 0.0f, -535.0f},
        {0.0f, 90.0f, 0.0f},
        {-50.0f, 0.0f, -575.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {-750.0f, 0.0f, 295.0f},
        {0.0f, 0.0f, 0.0f},
        {150.0f, 0.0f, -575.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {-550.0f, 0.0f, 295.0f},
        {0.0f, 0.0f, 0.0f},
        {500.0f, 0.0f, -445.0f},
        {0.0f, 90.0f, 0.0f}
    },
        {
        {-425.0f, 0.0f, 220.0f},
        {0.0f, 90.0f, 0.0f},
        {-200.0f, 0.0f, -175.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {-70.0f, 0.0f, -115.0f},
        {0.0f, 0.0f, 0.0f},
        {50.0f, 0.0f, -175.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {180.0f, 0.0f, -115.0f},
        {0.0f, 0.0f, 0.0f},
        {-75.0f, 0.0f, -100.0f},
        {0.0f, 90.0f, 0.0f}
    },
        {
        {55.0f, 0.0f, -190.0f},
        {0.0f, 90.0f, 0.0f},
        {-720.0f, 0.0f, 215.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {40.0f, 0.0f, 325.0f},
        {0.0f, 0.0f, 0.0f},
        {-470.0f, 0.0f, 215.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {-85.0f, 0.0f, 250.0f},
        {0.0f, 90.0f, 0.0f},
        {-595.0f, 0.0f, 140.0f},
        {0.0f, 90.0f, 0.0f}
    },
        {
        {710.0f, 0.0f, 65.0f},
        {0.0f, 0.0f, 0.0f},
        {705.0f, 0.0f, -65.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {585.0f, 0.0f, 140.0f},
        {0.0f, 90.0f, 0.0f},
        {580.0f, 0.0f, 60.0f},
        {0.0f, 90.0f, 0.0f}
    },
        {
        {510.0f, 0.0f, 265.0f},
        {0.0f, 0.0f, 0.0f},
        {505.0f, 0.0f, -65.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {285.0f, 0.0f, 750.0f},
        {0.0f, 90.0f, 0.0f},
        {580.0f, 0.0f, 260.0f},
        {0.0f, 90.0f, 0.0f}
    },
        {
        {-420.0f, 0.0f, 700.0f},
        {0.0f, 90.0f, 0.0f},
        {-50.0f, 0.0f, 330.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {-345.0f, 0.0f, 575.0f},
        {0.0f, 0.0f, 0.0f},
        {150.0f, 0.0f, 330.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {-145.0f, 0.0f, 575.0f},
        {0.0f, 0.0f, 0.0f},
        {275.0f, 0.0f, 405.0f},
        {0.0f, 90.0f, 0.0f}
    },
        {
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {-390.0f, 0.0f, 610.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {-515.0f, 0.0f, 535.0f},
        {0.0f, 90.0f, 0.0f}
    },
        {
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f},
        {460.0f, 0.0f, 750.0f},
        {0.0f, 90.0f, 0.0f}
    },
        {
        {835.0f, 0.0f, 65.0f},
        {0.0f, 0.0f, 0.0f},
        {830.0f, 0.0f, -65.0f},
        {0.0f, 0.0f, 0.0f}
    },
        {
        {-419.0f, 0.0f, 825.0f},
        {0.0f, 0.0f, 0.0f},
        {-455.0f, 0.0f, -535.0f},
        {0.0f, 0.0f, 0.0f}
    }
    };
    s16 selectedNodes[9];
    s16 outerIndex;
    s16 innerIndex;
    s16 layout;
    s32 candidate;
    s16 quarterTurns;
    s16 boundIndex;
    s16 savedLink;
    f32 rotation;
    f32 distanceSquared;
    f32 radius;
    f32 angle;

    layout = frandmod(2);
    lbl_1_bss_9C8.layout = layout;
    quarterTurns = frandmod(4);
    rotation = 90.0f * (f32)quarterTurns;
    lbl_1_bss_9C8.rotation = rotation;
    printf(lbl_1_data_1E4, layout, (s16)rotation);
    for (outerIndex = 0; outerIndex < quarterTurns; outerIndex++) {
    for (innerIndex = 0; innerIndex < 24; innerIndex++) {
            savedLink = lbl_1_bss_398[innerIndex].links[layout][2];
            lbl_1_bss_398[innerIndex].links[layout][2] = lbl_1_bss_398[innerIndex].links[layout][0];
            lbl_1_bss_398[innerIndex].links[layout][0] = lbl_1_bss_398[innerIndex].links[layout][3];
            lbl_1_bss_398[innerIndex].links[layout][3] = lbl_1_bss_398[innerIndex].links[layout][1];
            lbl_1_bss_398[innerIndex].links[layout][1] = savedLink;
        }
    }
    for (outerIndex = 0; outerIndex < 9; outerIndex++) selectedNodes[outerIndex] = -1;
    for (outerIndex = 0; outerIndex < 9; outerIndex++) {
        if (limits[outerIndex][layout * 2 + 1] != -1) {
            while (1) {
                candidate = limits[outerIndex][layout * 2] +
                            frandmod(candidate = limits[outerIndex][layout * 2 + 1] -
                                                 limits[outerIndex][layout * 2] + 1);
                for (innerIndex = 0; innerIndex < 9; innerIndex++) {
                    if ((s16)candidate == selectedNodes[innerIndex]) break;
                }
                if (innerIndex >= 9) break;
            }
        } else {
            candidate = limits[outerIndex][layout * 2];
        }
        lbl_1_bss_9C8.selected[outerIndex].nodeIndex = selectedNodes[outerIndex] = (s16)candidate;
    }
    for (outerIndex = 0; outerIndex < 9; outerIndex++)
        printf(lbl_1_data_1F6, selectedNodes[outerIndex]);
    printf(lbl_1_data_1FA);
    for (outerIndex = 46, innerIndex = 0; outerIndex <= 54; outerIndex++, innerIndex++) {
        lbl_1_bss_E88[outerIndex].modelId =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_118[44], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_E88[outerIndex].modelId, 1U);
        Hu3DModelAttrReset(lbl_1_bss_E88[outerIndex].modelId, 1U);
        Hu3DModelCameraSet(lbl_1_bss_E88[outerIndex].modelId, 1U);
        Hu3DModelLayerSet(lbl_1_bss_E88[outerIndex].modelId, 0);
        Hu3DModelShadowSet(lbl_1_bss_E88[outerIndex].modelId);
        Hu3DModelShadowDispOn(lbl_1_bss_E88[outerIndex].modelId);
        Hu3DModelAttrReset(lbl_1_bss_E88[outerIndex].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionTimeSet(lbl_1_bss_E88[outerIndex].modelId, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_E88[outerIndex].modelId, 0.0f);
        lbl_1_bss_E88[outerIndex].position = nodePositions[selectedNodes[innerIndex]][layout];
        lbl_1_bss_E88[outerIndex].rotation.x = 0.0f;
        lbl_1_bss_E88[outerIndex].rotation.y = 0.0f;
        lbl_1_bss_E88[outerIndex].rotation.z = 0.0f;
        lbl_1_bss_9C8.selected[innerIndex].modelIndex = outerIndex;
    }
    for (outerIndex = 55, boundIndex = 0; outerIndex <= 78; outerIndex++, boundIndex++) {
        lbl_1_bss_E88[outerIndex].modelId =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_118[45], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_E88[outerIndex].modelId, 1U);
        Hu3DModelCameraSet(lbl_1_bss_E88[outerIndex].modelId, 1U);
        Hu3DModelLayerSet(lbl_1_bss_E88[outerIndex].modelId, 1);
        Hu3DModelAttrReset(lbl_1_bss_E88[outerIndex].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionTimeSet(lbl_1_bss_E88[outerIndex].modelId, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_E88[outerIndex].modelId, 0.0f);
        lbl_1_bss_E88[outerIndex].position = obstacleBounds[boundIndex][layout * 2];
        *(M617ModelPosition *) &lbl_1_bss_E88[outerIndex].rotation =
            obstacleBounds[boundIndex][layout * 2 + 1];
        lbl_1_bss_E88[outerIndex].dimensions = shapeParameters[0];
        lbl_1_bss_E88[outerIndex].edgeDirections[0] = edgeDirections[0];
        lbl_1_bss_E88[outerIndex].edgeDirections[1] = edgeDirections[1];
        lbl_1_bss_E88[outerIndex].edgeDirections[2] = edgeDirections[2];
        lbl_1_bss_E88[outerIndex].edgeDirections[3] = edgeDirections[3];
    }
    for (outerIndex = 79; outerIndex <= 80; outerIndex++, boundIndex++) {
        lbl_1_bss_E88[outerIndex].modelId =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_118[46], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_E88[outerIndex].modelId, 1U);
        Hu3DModelCameraSet(lbl_1_bss_E88[outerIndex].modelId, 1U);
        Hu3DModelLayerSet(lbl_1_bss_E88[outerIndex].modelId, 1);
        Hu3DModelAttrReset(lbl_1_bss_E88[outerIndex].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionTimeSet(lbl_1_bss_E88[outerIndex].modelId, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_E88[outerIndex].modelId, 0.0f);
        lbl_1_bss_E88[outerIndex].position = obstacleBounds[boundIndex][layout * 2];
        *(M617ModelPosition *) &lbl_1_bss_E88[outerIndex].rotation =
            obstacleBounds[boundIndex][layout * 2 + 1];
    }
    for (outerIndex = 55; outerIndex <= 80; outerIndex++) {
        if (lbl_1_bss_E88[outerIndex].position.x != 0.0f)
            Hu3DModelAttrReset(lbl_1_bss_E88[outerIndex].modelId, 1U);
    }
    for (outerIndex = 46; outerIndex <= 80; outerIndex++) {
        angle = (f32) (180.0 * (atan2((f64) lbl_1_bss_E88[outerIndex].position.x,
                                      (f64) lbl_1_bss_E88[outerIndex].position.z) /
                                3.141592653589793));
        distanceSquared =
            lbl_1_bss_E88[outerIndex].position.x * lbl_1_bss_E88[outerIndex].position.x +
            lbl_1_bss_E88[outerIndex].position.z * lbl_1_bss_E88[outerIndex].position.z;
        radius = (f32)sqrt((f64)distanceSquared);
        lbl_1_bss_E88[outerIndex].position.x =
            (f32) ((f64) radius * sin((3.141592653589793 * (f64) (rotation + angle)) / 180.0));
        lbl_1_bss_E88[outerIndex].position.z =
            (f32) ((f64) radius * cos((3.141592653589793 * (f64) (rotation + angle)) / 180.0));
        if ((s16)rotation % 180 != 0) {
            lbl_1_bss_E88[outerIndex].rotation.y =
                lbl_1_bss_E88[outerIndex].rotation.y >= 90.0f ? 0.0f : 90.0f;
        }
    }
}

#undef setupTableEntryAtIndex386

#undef setupTableEntryAtIndex382

#undef setupTableEntryAtIndex74

#undef setupTableEntryAtIndex70

#undef setupTableEntryAtIndex2

#undef setupTableEntryAtIndex0

/* fn_1_93D0 calls this to create local lights and assign them to nearby models. */
void fn_1_BA64(void)
{
    GXColor color = {247, 148, 29, 255};
    Point3d lightPositions[4] = {
        {-740.0f, 0.0f, -740.0f},
        {740.0f, 0.0f, -740.0f},
        {-740.0f, 0.0f, 740.0f},
        {740.0f, 0.0f, 740.0f}
    };
    Point3d bounds[4][2] = {
        {
        {-850.0f, 0.0f, -425.0f},
        {-425.0f, 0.0f, -850.0f}
    },
        {
        {850.0f, 0.0f, -425.0f},
        {425.0f, 0.0f, -850.0f}
    },
        {
        {-850.0f, 0.0f, 425.0f},
        {-425.0f, 0.0f, 850.0f}
    },
        {
        {850.0f, 0.0f, 425.0f},
        {425.0f, 0.0f, 850.0f}
    }
    };
    Point3d upperBound;
    Point3d lowerBound;
    Point3d position;
    Point3d rotation;
    Point3d delta;
    Point3d direction;
    f32 intensity;
    f32 distance;
    s16 modelIndex;
    s16 innerIndex;
    s16 lightIndex;
    M617LightSource *light;
    M617PlayerStateData *player;
    M617ModelSetupRecord *model;
    HU3D_MODEL *modelData;

    for (modelIndex = 0; modelIndex < 15; modelIndex++) lbl_1_bss_458[modelIndex].modelId = -1;
    position.x = 0.0f;
    position.y = 120.0f;
    position.z = 100.0f;
    rotation.x = -60.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    direction.x = 0.0f;
    direction.y = -0.5f;
    direction.z = -0.5f;
    intensity = 0.6f;
    for (modelIndex = 0, lightIndex = 0; modelIndex < 2; modelIndex++, lightIndex++) {
        player = &lbl_1_bss_AB0[modelIndex];
        light = &lbl_1_bss_458[lightIndex];
        light->modelId = lbl_1_bss_E88[modelIndex].modelId;
        light->modelIndex = modelIndex;
        light->position = position;
        light->rotation = rotation;
        light->direction = direction;
        light->color = color;
        light->distance = position.y;
        if (modelIndex == 0) light->intensity = 0.001f;
        else light->intensity = intensity;
        light->intensityStep = 0.005f;
        modelData = &Hu3DData[light->modelId];
        light->lightId =
            Hu3DLLightCreateV(light->modelId, &light->position, &light->direction, &light->color);
        light->localLightPoolId = modelData->LLightId[light->lightId];
        Hu3DLLightStaticSet(light->modelId, light->lightId, 1);
        Hu3DLLightPointSet(light->modelId, light->lightId, light->distance, light->intensity,
                           GX_DA_STEEP);
        player->upperIntensityLimit[0] = 0.56f;
        player->lowerIntensityLimit[0] = 0.52f;
        player->upperIntensityLimit[1] = 0.56f;
        player->lowerIntensityLimit[1] = 0.52f;
        player->upperIntensityLimit[2] = 0.42f;
        player->lowerIntensityLimit[2] = 0.38f;
        player->upperScaleLimit[0] = 1.05f;
        player->lowerScaleLimit[0] = 0.95f;
        player->upperScaleLimit[1] = 1.05f;
        player->lowerScaleLimit[1] = 0.95f;
        player->upperScaleLimit[2] = 0.95f;
        player->lowerScaleLimit[2] = 0.85f;
        for (innerIndex = 0; innerIndex < 2; innerIndex++) {
            if (modelIndex != innerIndex)
                fn_1_3BB8(lbl_1_bss_E88[innerIndex].modelId, light->localLightPoolId);
        }
    for (innerIndex = 3; innerIndex <= 18; innerIndex++)
            fn_1_3BB8(lbl_1_bss_E88[innerIndex].modelId, light->localLightPoolId);
    for (innerIndex = 20; innerIndex <= 23; innerIndex++)
            fn_1_3BB8(lbl_1_bss_E88[innerIndex].modelId, light->localLightPoolId);
    for (innerIndex = 46; innerIndex <= 54; innerIndex++)
            fn_1_3BB8(lbl_1_bss_E88[innerIndex].modelId, light->localLightPoolId);
    for (innerIndex = 92; innerIndex <= 93; innerIndex++)
            fn_1_3BB8(lbl_1_bss_E88[innerIndex].modelId, light->localLightPoolId);
    }
    position.x = 0.0f;
    position.y = 100.0f;
    position.z = 0.0f;
    rotation.x = 90.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    direction.x = 0.0f;
    direction.y = -0.5f;
    direction.z = -0.5f;
    intensity = 0.5f;
    for (modelIndex = 46, lightIndex = 2; modelIndex <= 54; modelIndex++, lightIndex++) {
        light = &lbl_1_bss_458[lightIndex];
        light->modelId = lbl_1_bss_E88[modelIndex].modelId;
        light->modelIndex = modelIndex;
        light->position = position;
        light->referencePosition.z = 0.0f;
        light->rotation = rotation;
        light->direction = direction;
        light->color = color;
        light->distance = position.y;
        light->intensity = intensity;
        light->intensityStep = 0.005f;
        modelData = &Hu3DData[light->modelId];
        light->lightId =
            Hu3DLLightCreateV(light->modelId, &light->position, &light->direction, &light->color);
        light->localLightPoolId = modelData->LLightId[light->lightId];
        Hu3DLLightStaticSet(light->modelId, light->lightId, 1);
        Hu3DLLightPointSet(light->modelId, light->lightId, light->distance, light->intensity,
                           GX_DA_STEEP);
    }
    position.x = 0.0f;
    position.y = 100.0f;
    position.z = 0.0f;
    rotation.x = 90.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    direction.x = 0.0f;
    direction.y = -0.5f;
    direction.z = -0.5f;
    intensity = 0.4f;
    for (modelIndex = 20, innerIndex = 0, lightIndex = 11; modelIndex <= 23;
         modelIndex++, innerIndex++, lightIndex++) {
        light = &lbl_1_bss_458[lightIndex];
        light->modelId = lbl_1_bss_E88[modelIndex].modelId;
        light->modelIndex = modelIndex;
        light->position = position;
        light->referencePosition = lightPositions[innerIndex];
        light->rotation = rotation;
        light->direction = direction;
        light->color = color;
        light->distance = position.y;
        light->intensity = intensity;
        light->intensityStep = 0.005f;
        modelData = &Hu3DData[light->modelId];
        light->lightId =
            Hu3DLLightCreateV(light->modelId, &light->position, &light->direction, &light->color);
        light->localLightPoolId = modelData->LLightId[light->lightId];
        Hu3DLLightStaticSet(light->modelId, light->lightId, 1);
        Hu3DLLightPointSet(light->modelId, light->lightId, light->distance, light->intensity,
                           GX_DA_STEEP);
    }
    fn_1_7048();
    for (modelIndex = 3; modelIndex <= 18; modelIndex++) {
        model = &lbl_1_bss_E88[modelIndex];
        for (innerIndex = 46, lightIndex = 0; innerIndex <= 54; innerIndex++, lightIndex++) {
            delta.x = lbl_1_bss_E88[innerIndex].position.x - model->secondaryPosition.x;
            delta.z = lbl_1_bss_E88[innerIndex].position.z - model->secondaryPosition.z;
            distance = (f32)sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
            if (distance < 500.0f) {
                light = &lbl_1_bss_458[innerIndex - 44];
                fn_1_3BB8(model->modelId, light->localLightPoolId);
            }
        }
        for (innerIndex = 0; innerIndex < 4; innerIndex++) {
            delta.x = lightPositions[innerIndex].x - model->secondaryPosition.x;
            delta.z = lightPositions[innerIndex].z - model->secondaryPosition.z;
            distance = (f32)sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
            if (distance < 500.0f) {
                light = &lbl_1_bss_458[innerIndex + 11];
                fn_1_3BB8(model->modelId, light->localLightPoolId);
            }
        }
    }
    for (modelIndex = 55; modelIndex <= 80; modelIndex++) {
        model = &lbl_1_bss_E88[modelIndex];
        for (innerIndex = 0; innerIndex <= 1; innerIndex++) {
            light = &lbl_1_bss_458[innerIndex];
            fn_1_3BB8(model->modelId, light->localLightPoolId);
        }
        for (innerIndex = 46; innerIndex <= 54; innerIndex++) {
            delta.x = lbl_1_bss_E88[innerIndex].position.x - model->position.x;
            delta.z = lbl_1_bss_E88[innerIndex].position.z - model->position.z;
            distance = (f32)sqrt((f64)(delta.x * delta.x + delta.z * delta.z));
            if (distance < 300.0f) {
                light = &lbl_1_bss_458[innerIndex - 44];
                fn_1_3BB8(model->modelId, light->localLightPoolId);
            }
        }
    }
    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        for (innerIndex = 46, lightIndex = 0; innerIndex <= 54; innerIndex++, lightIndex++) {
            model = &lbl_1_bss_E88[innerIndex];
            light = &lbl_1_bss_458[lightIndex + 2];
            lowerBound.x = bounds[modelIndex][0].x - 300.0f;
            upperBound.x = 300.0f + bounds[modelIndex][0].x;
            lowerBound.z = bounds[modelIndex][0].z - 400.0f;
            upperBound.z = 400.0f + bounds[modelIndex][0].z;
            if (lowerBound.x < model->position.x && upperBound.x > model->position.x &&
                lowerBound.z < model->position.z && upperBound.z > model->position.z)
                fn_1_3BB8(lbl_1_bss_E88[modelIndex + 20].modelId, light->localLightPoolId);
            lowerBound.x = bounds[modelIndex][1].x - 400.0f;
            upperBound.x = 400.0f + bounds[modelIndex][1].x;
            lowerBound.z = bounds[modelIndex][1].z - 300.0f;
            upperBound.z = 300.0f + bounds[modelIndex][1].z;
            if (lowerBound.x < model->position.x && upperBound.x > model->position.x &&
                lowerBound.z < model->position.z && upperBound.z > model->position.z)
                fn_1_3BB8(lbl_1_bss_E88[modelIndex + 20].modelId, light->localLightPoolId);
        }
    }
}

void fn_1_535C(void);

void fn_1_54B8(s16, s16, Vec *);

s32 fn_1_16990(void *, s16);

s32 fn_1_2E94(f32 *, f32, f32, f32);

/* The sequence callbacks use this each frame; it returns nonzero when the intro ends. */
s32 fn_1_CCB4(void)
{
    M617PlayerStateData *players;
    M617ModelSetupRecord *model;
    M617ParticipantMotionState *transition;
    M617LightSource *modelConfiguration;
    s32 finished;
    s32 animationResult;
    f32 transitionDuration;

    players = lbl_1_bss_AB0;
    model = &lbl_1_bss_E88[81];
    transition = lbl_1_bss_DC0;
    modelConfiguration = lbl_1_bss_458;
    finished = 0;
    animationResult = 0;

    switch (lbl_1_bss_9C8.transitionPhase) {
    case 0:
        fn_1_535C();
        if (lbl_1_bss_9C8.transitionFrame++ > 100) {
            transition->previousMode = -1;
            transition->previousAttributes = -1;
            transition->mode = 6;
            transition->speed = 1.0f;
            transition->time = 0.0f;
            transition->attributes = 0;
            transition->attributesToReset = HU3D_MOTATTR_LOOP;
            lbl_1_bss_9C8.transitionFrame = 0;
            lbl_1_bss_9C8.transitionPhase++;
        }
        break;
    case 1:
        if (transition->time >= 14.0f) {
            transition->speed = 0.0f;
            lbl_1_bss_9C8.transitionPhase++;
        }
        break;
    case 2:
        if (lbl_1_bss_9C8.transitionFrame++ >= 20) {
            modelConfiguration->enabled = 1;
            model->motionState = 1;
            omVibrate(lbl_1_bss_AB0[0].playerIndex, 12, 7, 3);
            omVibrate(lbl_1_bss_AB0[1].playerIndex, 12, 7, 3);
            fn_1_54B8(0, 5, 0);
            lbl_1_bss_9C8.transitionFrame = 0;
            lbl_1_bss_9C8.transitionPhase++;
        }
        break;
    case 3:
        if ((players[0].ambient += 0.05f) >= 0.5f) {
            players[0].ambient = 0.5f;
        }
        Hu3DModelAmbSet(lbl_1_bss_E88[0].modelId,
            players[0].ambient, players[0].ambient, players[0].ambient);
        if (lbl_1_bss_9C8.transitionFrame++ >= 30) {
            transition->speed = 1.0f;
            lbl_1_bss_9C8.transitionFrame = 0;
            lbl_1_bss_9C8.transitionPhase++;
        }
        break;
    case 4:
        if (transition->time >= 32.9f) {
            transition->mode = transition->previousMode = 3;
            transition->shiftState = 1;
            transition->shiftMotion = 4;
            transition->shiftStart = 0.0f;
            transition->shiftEnd = 10.0f;
            transition->shiftFlags = HU3D_MOTATTR_LOOP;
            transition->attributesToReset = 0;
            lbl_1_bss_9C8.transitionPhase++;
        }
        break;
    case 5:
        transitionDuration = 60.0f;
        fn_1_2E94(&Center.x, transitionDuration, -737.0f,
            0.0f);
        fn_1_2E94(&Center.y, transitionDuration, 456.0f,
            181.0f);
        fn_1_2E94(&Center.z, transitionDuration, 55.0f,
            55.0f);
        fn_1_2E94(&CRot.x, transitionDuration, -24.0f,
            -35.0f);
        fn_1_2E94(&CRot.y, transitionDuration, 0.0f,
            0.0f);
        fn_1_2E94(&CRot.z, transitionDuration, 0.0f,
            0.0f);
        fn_1_2E94(&CZoom, transitionDuration, 197.0f,
            4590.0f);
        transitionDuration = 30.0f;
        animationResult = fn_1_2E94(&players[0].rotation.y,
            transitionDuration, -135.0f, 45.0f);
        lbl_1_bss_9BC.players[0]->actor->rotY = players[0].rotation.y;
        if ((s16)animationResult == 0) {
            players[0].targetPosition.x = -650.0f;
            players[0].targetPosition.y = 0.0f;
            players[0].targetPosition.z = -650.0f;
            players[0].chaseRadius = 7.1f;
            players[0].chaseSpeed = 5.0f;
            fn_1_16990(players, 0);
            lbl_1_bss_9C8.transitionPhase++;
        }
        break;
    case 6:
        transitionDuration = 60.0f;
        animationResult += fn_1_2E94(&Center.x, transitionDuration,
            -737.0f, 0.0f);
        animationResult += fn_1_2E94(&Center.y, transitionDuration,
            456.0f, 181.0f);
        animationResult += fn_1_2E94(&Center.z, transitionDuration,
            55.0f, 55.0f);
        animationResult += fn_1_2E94(&CRot.x, transitionDuration,
            -24.0f, -35.0f);
        animationResult += fn_1_2E94(&CRot.y, transitionDuration,
            0.0f, 0.0f);
        animationResult += fn_1_2E94(&CRot.z, transitionDuration,
            0.0f, 0.0f);
        animationResult += fn_1_2E94(&CZoom, transitionDuration,
            197.0f, 4590.0f);
        if ((s16)animationResult == 0) {
            lbl_1_bss_9C8.transitionPhase++;
        }
        fn_1_16990(players, 0);
        break;
    case 7:
        animationResult = fn_1_16990(players, 1);
        if ((s16)animationResult != 0) {
            lbl_1_bss_9C8.transitionPhase++;
            finished = 1;
        }
        break;
    case 8:
        finished = 1;
        break;
    }
    return finished;
}
