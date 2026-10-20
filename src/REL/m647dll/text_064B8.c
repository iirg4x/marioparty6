/* Loads and updates the models and sprites for Insectiride's four activities. */
#include "REL/m647dll/module_types.h"
#include <string.h>

#define MSM_SE_M647_1999 1999
#define MSM_SE_M647_2000 2000
#define MSM_SE_M647_2001 2001
#define MSM_SE_M647_2002 2002
#define MSM_SE_M647_2003 2003
#define MSM_SE_M647_2004 2004
#define MSM_SE_M647_2005 2005
#define MSM_SE_M647_2006 2006
#define MSM_SE_M647_2007 2007
#define MSM_SE_M647_2008 2008
#define MSM_SE_M647_2009 2009
#define MSM_SE_M647_2010 2010

s16 fn_1_2B30(s16 index);
s16 fn_1_2B4C(s16 index);
void fn_1_32F4(s16 index);

typedef struct M647OffsetTable_s {
    f32 offsets[4]; /* World-unit offsets for each activity model. */
} M647OffsetTable;

static const M647OffsetTable lbl_1_rodata_1F8 = {{100.0f, 120.0f, 70.0f, 110.0f}};
static const M647OffsetTable lbl_1_rodata_208 = {{30.0f, 70.0f, 70.0f, 20.0f}};

typedef struct M647MotionAssets_s {
    u32 motionDataNumbers[3]; /* Motion archive entries; zero marks an unused slot. */
} M647MotionAssets;

Point3d lbl_1_data_348[4] = {
    -1130.0f, 200.0f, 475.0f, -1220.0f, 220.0f, 825.0f, -1260.0f, 190.0f,
    1175.0f, -1110.0f, 210.0f, 1525.0f
};
f32 lbl_1_data_378[3] = { 1160.0f, 150.0f, 1000.0f };
s32 lbl_1_data_384[4] = {
    DATANUM(DATA_m647, 16), DATANUM(DATA_m647, 22),
    DATANUM(DATA_m647, 31), DATANUM(DATA_m647, 37)
};
M647MotionAssets lbl_1_data_394[4] = {
    DATANUM(DATA_m647, 17), DATANUM(DATA_m647, 18), DATANUM(DATA_m647, 19),
    DATANUM(DATA_m647, 23), DATANUM(DATA_m647, 24), 0,
    DATANUM(DATA_m647, 32), DATANUM(DATA_m647, 33),
    DATANUM(DATA_m647, 34), DATANUM(DATA_m647, 38), DATANUM(DATA_m647, 39),
    DATANUM(DATA_m647, 40)
};
s32 lbl_1_data_3C4[2] = { DATANUM(DATA_m647, 20), DATANUM(DATA_m647, 21) };
s32 lbl_1_data_3CC[2] = { DATANUM(DATA_m647, 35), DATANUM(DATA_m647, 36) };
s32 lbl_1_data_3D4[4] = {
    DATANUM(DATA_m647, 27), DATANUM(DATA_m647, 29),
    DATANUM(DATA_m647, 28), DATANUM(DATA_m647, 30)
};
s32 lbl_1_data_3E4[2] = { DATANUM(DATA_m647, 41), DATANUM(DATA_m647, 42) };
s32 lbl_1_data_3EC[4] = {
    DATANUM(DATA_m647, 46), DATANUM(DATA_m647, 47),
    DATANUM(DATA_m647, 48), DATANUM(DATA_m647, 49)
};
s32 lbl_1_data_3FC[6] = {
    DATANUM(DATA_m647, 50), DATANUM(DATA_m647, 54),
    DATANUM(DATA_m647, 57), DATANUM(DATA_m647, 60),
    DATANUM(DATA_m647, 63), DATANUM(DATA_m647, 66)
};
s32 lbl_1_data_414[6] = {
    DATANUM(DATA_m647, 51), DATANUM(DATA_m647, 55),
    DATANUM(DATA_m647, 58), DATANUM(DATA_m647, 61),
    DATANUM(DATA_m647, 64), DATANUM(DATA_m647, 67)
};
s32 lbl_1_data_42C[6] = {
    DATANUM(DATA_m647, 52), DATANUM(DATA_m647, 56),
    DATANUM(DATA_m647, 59), DATANUM(DATA_m647, 62),
    DATANUM(DATA_m647, 65), DATANUM(DATA_m647, 68)
};
s32 lbl_1_data_444[3] = {
    DATANUM(DATA_m647, 43), DATANUM(DATA_m647, 45), DATANUM(DATA_m647, 44)
};
char lbl_1_data_450[6] = {'A', 'B', 'X', 'Y', 'L', 'R'};
u16 lbl_1_data_456[] = { (1 << 8), (1 << 9), (1 << 10), (1 << 11), (1 << 14), (1 << 13) };
f32 lbl_1_data_464[8] = { 40.0f, 120.0f, 40.0f, 195.0f, 40.0f, 270.0f, 40.0f, 345.0f };

extern s16 fn_1_2B68(s16 index);
extern s16 fn_1_34A0(s16 index);
u16 fn_1_84E4(s16 playerIndex);
s32 fn_1_8FF0(s16 playerIndex);
void fn_1_8D5C(void);
void fn_1_8E98(void);
void fn_1_8F2C(void);
void fn_1_90D4(s16 playerIndex);
s32 fn_1_9300(void);
void fn_1_A6C8(M647ActivityEntry *state, s16 mode);
u16 fn_1_9238(s16 playerIndex);
void fn_1_6E8C(s16 entryIndex, u32 attributes);
void fn_1_6ED0(s16 entryIndex, f32 speed);
void fn_1_86C4(void);
void fn_1_93E8(void);
void fn_1_6F88(void);
void fn_1_7A38(void);
void fn_1_6E20(s16 entryIndex, s16 motionIndex);
void fn_1_759C(void);
void fn_1_7618(void);
void fn_1_7660(void);
s16 fn_1_76E0(s16 playerIndex);
void fn_1_77B0(s16 playerIndex);
s16 fn_1_7940(void);
void *fn_1_8F58(s32 index);
void fn_1_80E4(void);
void fn_1_8174(void);
void fn_1_81BC(void);
void fn_1_823C(void);
void fn_1_83EC(s16 playerIndex);
s16 fn_1_831C(s16 playerIndex);
s16 fn_1_85CC(void);
void fn_1_A1E4(M647ActivityEntry *state);
s16 fn_1_A200(s16 playerIndex);
void fn_1_A2D4(s16 playerIndex);
s16 fn_1_A478(void);
void fn_1_64C8(void);
u32 fn_1_788C(s16 playerIndex);
void fn_1_79CC(void);
static inline void fn_1_7A38_setupGroupSprites(HUSPR_GROUPID groupId);
s16 fn_1_8648(void);
void fn_1_8658(void);
void fn_1_8A40(s16 playerIndex);
void fn_1_8E0C(void);
void fn_1_937C(void);
void fn_1_9820(void);
u32 fn_1_A3D8(s16 playerIndex);
s16 fn_1_A518(s16 entryIndex);
void fn_1_A534(void);
s32 fn_1_A740(void);
f32 fn_1_A790(void);
f32 fn_1_A854(void);
void fn_1_A8C4(void);
void fn_1_AA64(void);
void fn_1_AC40(s16 entryIndex);
void fn_1_ADFC(s16 entryIndex);
void fn_1_AE58(s16 mode);
void fn_1_AF18(s16 cameraMode);
void fn_1_66BC(void);
void fn_1_6A2C(void);
void fn_1_6F1C(void);
void fn_1_7230(s16 playerIndex);
void fn_1_7D68(s16 playerIndex);
void fn_1_975C(void);
void fn_1_97C0(void);
void fn_1_98CC(s16 argument);
M647AnimState *fn_1_64B8(void);
void fn_1_64C4(void);
s32 fn_1_6F14(void);

/* Sets up the row activity's fixed sprites when fn_1_7A38 creates its group. */
static inline void fn_1_7A38_setupGroupSprites(HUSPR_GROUPID groupId) {
    HuSprGrpPosSet(groupId, lbl_1_data_464[6],
                    lbl_1_data_464[7]);
    HuSprGrpMemberSet(groupId, 0, HuSprCreate(lbl_1_bss_214[3], 15, 0));
    HuSprPosSet(groupId, 0, 0.0f, 0.0f);
    HuSprAttrSet(groupId, 0, 4);
    /* Member 1 is still empty: the position call uses sprite -1, and the attribute call is
     * ignored. */
    HuSprPosSet(groupId, 1, 0.0f, 0.0f);
    HuSprAttrSet(groupId, 1, 4);
    HuSprGrpMemberSet(groupId, 2, HuSprCreate(lbl_1_bss_208[2], 15, 0));
    HuSprPosSet(groupId, 2, 50.0f, 0.0f);
    HuSprTPLvlSet(groupId, 2, 0.7f);
    HuSprAttrSet(groupId, 2, 4);
    HuSprGrpMemberSet(groupId, 3, HuSprCreate(lbl_1_bss_1A8, 8, 0));
    HuSprPosSet(groupId, 3, 53.0f, 0.0f);
    HuSprAttrSet(groupId, 3, 4);
}

/* Returns the shared animation state when another activity unit needs it. */
M647AnimState *fn_1_64B8(void) { return &lbl_1_bss_174; }

/* The sequence callbacks invoke this empty hook during their frame updates. */
void fn_1_64C4(void) { }

/* Loads the sprite animations used by the model activities. */
void fn_1_64C8(void) {
    s32 index;

    for (index = 0; index < 4; index++) {
        lbl_1_bss_214[index] = HuSprAnimRead(
        HuDataSelHeapReadNum(lbl_1_data_3EC[index], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_1AC[index] = HuSprAnimRead(HuDataSelHeapReadNum(
            fn_1_2B68((s16) index) + DATANUM(DATA_mgconst, 0), HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (index = 0; index < 6; index++) {
        lbl_1_bss_1F0[index] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_3FC[index], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_1D8[index] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_414[index], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_1C0[index] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_42C[index], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (index = 0; index < 3; index++) {
        lbl_1_bss_208[index] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_444[index], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    lbl_1_bss_1A8 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_m647, 69), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_1BC =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_m647, 53), HU_MEMNUM_OVL, HEAP_MODEL));
}

/* Creates shared activity models and attaches their item models during setup. */
void fn_1_66BC(void) {
    M647OffsetTable heightOffsets;
    M647OffsetTable xOffsets;
    M647AnimState *state;
    s32 modelIndex;
    HU3D_MODELID attachmentModelId;

    state = &lbl_1_bss_174;
    heightOffsets = lbl_1_rodata_1F8;
    xOffsets = lbl_1_rodata_208;
    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        state->modelIds[modelIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_384[modelIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelPosSet(state->modelIds[modelIndex],
                        7000.0f - xOffsets.offsets[modelIndex],
                        heightOffsets.offsets[modelIndex], 0.0f);
        Hu3DModelCameraSet(state->modelIds[modelIndex], 1);
        Hu3DModelAttrSet(state->modelIds[modelIndex], 1);

        switch (modelIndex) {
            case 0:
                attachmentModelId = Hu3DModelCreate(HuDataSelHeapReadNum(
                    lbl_1_data_3C4[lbl_1_bss_174.nightMode],
                    HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DModelHookSet(state->modelIds[modelIndex], "itemhook2",
                                 attachmentModelId);
                break;
            case 1:
                attachmentModelId = Hu3DModelCreate(
                    HuDataSelHeapReadNum(DATANUM(DATA_m647, 25), HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DModelHookSet(state->modelIds[modelIndex], "itemhook_rt",
                                 attachmentModelId);
                attachmentModelId = Hu3DModelCreate(
                    HuDataSelHeapReadNum(DATANUM(DATA_m647, 26), HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DModelHookSet(state->modelIds[modelIndex], "itemhook_ft",
                                 attachmentModelId);
                attachmentModelId = Hu3DModelCreate(HuDataSelHeapReadNum(
                    lbl_1_data_3D4[lbl_1_bss_174.nightMode],
                    HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DModelHookSet(state->modelIds[modelIndex], "itemhook_L1",
                                 attachmentModelId);
                attachmentModelId = Hu3DModelCreate(HuDataSelHeapReadNum(
                    (&lbl_1_data_3D4[lbl_1_bss_174.nightMode])[2],
                    HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DModelHookSet(state->modelIds[modelIndex], "itemhook_L2",
                                 attachmentModelId);
                break;
            case 2:
                attachmentModelId = Hu3DModelCreate(HuDataSelHeapReadNum(
                    lbl_1_data_3CC[lbl_1_bss_174.nightMode],
                    HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DModelHookSet(state->modelIds[modelIndex], "itemhook2",
                                 attachmentModelId);
                break;
            case 3:
                attachmentModelId = Hu3DModelCreate(HuDataSelHeapReadNum(
                    lbl_1_data_3E4[lbl_1_bss_174.nightMode],
                    HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DModelHookSet(state->modelIds[modelIndex], "itemhook2",
                                 attachmentModelId);
                break;
        }
    }
}

/* Loads activity sprites, creates the four activity models and initializes their motions. */
void fn_1_6A2C(void) {
    s16 modelId;
    s32 index;
    s32 modelIndex;
    s32 motionIndex;

    memset(&lbl_1_bss_224, 0, 304);
    for (index = 0; index < 4; index++) {
        lbl_1_bss_214[index] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_3EC[index], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_1AC[index] = HuSprAnimRead(HuDataSelHeapReadNum(
            fn_1_2B68((s16)index) + DATANUM(DATA_mgconst, 0),
            HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (index = 0; index < 6; index++) {
        lbl_1_bss_1F0[index] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_3FC[index], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_1D8[index] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_414[index], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_1C0[index] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_42C[index], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (index = 0; index < 3; index++) {
        lbl_1_bss_208[index] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_444[index], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    lbl_1_bss_1A8 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_m647, 69), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_1BC =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_m647, 53), HU_MEMNUM_OVL, HEAP_MODEL));

    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        modelId = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_384[modelIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(modelId, 1);
        Hu3DModelPosSet(modelId,
                        lbl_1_data_348[modelIndex].x,
                        lbl_1_data_348[modelIndex].y,
                        lbl_1_data_348[modelIndex].z);
        ((f32 *)(void *)&lbl_1_bss_224.entries[modelIndex].position.x)[0] =
            lbl_1_data_348[modelIndex].x;
        ((f32 *)(void *)&lbl_1_bss_224.entries[modelIndex].position.x)[1] =
            lbl_1_data_348[modelIndex].y;
        ((f32 *)(void *)&lbl_1_bss_224.entries[modelIndex].position.x)[2] =
            lbl_1_data_348[modelIndex].z;
        lbl_1_bss_224.entries[modelIndex].modelId = modelId;
        Hu3DModelShadowSet(modelId);

        for (motionIndex = 0; motionIndex < 3; motionIndex++) {
            if (lbl_1_data_394[modelIndex].motionDataNumbers[motionIndex] != 0) {
                HU3D_MOTIONID motionId;

                motionId = Hu3DMotionCreate(HuDataSelHeapReadNum(
                    lbl_1_data_394[modelIndex].motionDataNumbers[motionIndex],
                    HU_MEMNUM_OVL, HEAP_MODEL));
                lbl_1_bss_224.entries[modelIndex].motionIds[motionIndex] =
                    motionId;
            }
        }
        fn_1_6E20((s16)modelIndex, 0);
        fn_1_6ED0((s16)modelIndex, (0.0f));
        fn_1_6E8C((s16)modelIndex, HU3D_MOTATTR_LOOP);
    }
    fn_1_86C4();
    fn_1_93E8();
    fn_1_6F88();
    fn_1_7A38();
}

/* Selects the requested motion for one activity model. */
void fn_1_6E20(s16 entryIndex, s16 motionIndex) {
    Hu3DMotionSet(
        lbl_1_bss_224.entries[entryIndex].modelId,
        lbl_1_bss_224.entries[entryIndex].motionIds[motionIndex]);
}

/* Adds model flags or tagged motion flags to one activity model. */
void fn_1_6E8C(s16 entryIndex, u32 attributes) {
    Hu3DModelAttrSet(lbl_1_bss_224.entries[entryIndex].modelId, attributes);
}

/* Sets the playback speed for one activity model. */
void fn_1_6ED0(s16 entryIndex, f32 speed) {
    Hu3DMotionSpeedSet(lbl_1_bss_224.entries[entryIndex].modelId, speed);
}

/* Returns zero. */
s32 fn_1_6F14(void) { return 0; }

/* Installs the assigned player's character icon in the tap activity's group. */
void fn_1_6F1C(void) {
    HUSPR_GROUPID groupId;

    groupId = lbl_1_bss_224.entries[2].groupId;
    HuSprGrpMemberSet(groupId, 1, HuSprCreate(lbl_1_bss_1AC[fn_1_34A0(2)], 10, 0));
}

/* Builds the tap activity's item model and sprite group during setup. */
void fn_1_6F88(void) {
    M647ActivityEntry *entry;
    HUSPR_GROUPID groupId;
    HU3D_MODELID itemModelId;
    s32 buttonIndex;

    entry = &lbl_1_bss_224.entries[2];
    itemModelId = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_3CC[lbl_1_bss_174.nightMode],
                              HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelHookSet(entry->modelId, "itemhook2", itemModelId);
    entry->activity.tap.itemModelId = itemModelId;

    groupId = entry->groupId = HuSprGrpCreate(10);
    HuSprGrpPosSet(groupId, lbl_1_data_464[4],
                    lbl_1_data_464[5]);
    HuSprGrpMemberSet(groupId, 0, HuSprCreate(lbl_1_bss_214[2], 15, 0));
    HuSprPosSet(groupId, 0, 0.0f, 0.0f);
    HuSprAttrSet(groupId, 0, 4);
    /* Member 1 is still empty: the position call uses sprite -1, and the attribute call is
     * ignored. */
    HuSprPosSet(groupId, 1, 0.0f, 0.0f);
    HuSprAttrSet(groupId, 1, 4);
    HuSprGrpMemberSet(groupId, 2, HuSprCreate(lbl_1_bss_208[0], 15, 0));
    HuSprPosSet(groupId, 2, 50.0f, 0.0f);
    HuSprTPLvlSet(groupId, 2, 0.7f);
    HuSprAttrSet(groupId, 2, 4);
    HuSprGrpMemberSet(groupId, 3, HuSprCreate(lbl_1_bss_1A8, 8, 0));
    HuSprPosSet(groupId, 3, 56.0f, 0.0f);
    HuSprAttrSet(groupId, 3, 4);
    for (buttonIndex = 0; buttonIndex < 6; buttonIndex++) {
        HuSprGrpMemberSet(groupId, (s16)(buttonIndex + 4),
                          HuSprCreate(lbl_1_bss_1F0[buttonIndex], 10, 0));
        HuSprPosSet(groupId, (s16)(buttonIndex + 4), 56.0f, 0.0f);
        HuSprAttrSet(groupId, (s16)(buttonIndex + 4), 4);
    }
}

s16 lbl_1_data_4BE[4] = {30, 20, 5, 0};
s16 lbl_1_data_4C6[4] = {50, 40, 30, 23};
s16 lbl_1_data_4CE[4] = {50, 40, 30, 25};

/* Updates the tap activity each frame, handling input, motion and progress. */
void fn_1_7230(s16 playerIndex) {
    M647ActivityEntry *entry;
    Point3d movement;
    s16 inputResult;

    entry = &lbl_1_bss_224.entries[2];
    switch (entry->status) {
        case 0:
            fn_1_759C();
            fn_1_77B0(playerIndex);
            fn_1_A6C8(entry, 1);
            HuSprAttrReset(entry->groupId, 0, 4);
            HuSprAttrReset(entry->groupId, 2, 4);
            break;
        case 1:
            inputResult = fn_1_76E0(playerIndex);
            if (inputResult > 0) {
                fn_1_7660();
                HuAudFXPlay(MSM_SE_M647_2006);
                Hu3DMotionSet(lbl_1_bss_224.entries[2].modelId,
                              lbl_1_bss_224.entries[2].motionIds[1]);
                fn_1_A6C8(entry, 2);
                HuAudFXPlay(MSM_SE_M647_2007);
                entry->activity.tap.frame = 0;
            } else if (inputResult < 0) {
                fn_1_7618();
                Hu3DMotionSet(lbl_1_bss_224.entries[2].modelId,
                              lbl_1_bss_224.entries[2].motionIds[2]);
                fn_1_A6C8(entry, 3);
                fn_1_77B0(playerIndex);
                omVibrate(playerIndex, 20, 7, 3);
                entry->activity.tap.frame = 0;
            }
            break;
        case 2:
            movement.x = 3.3333333f;
            entry->position.x += movement.x;
            movement.y = movement.z = 0.0f;
            Hu3DModelPosSetV(entry->modelId,
                             &entry->position);
            if ((f32)++(entry->activity.tap.frame) >= 30.0f) {
                Hu3DMotionSet(lbl_1_bss_224.entries[2].modelId,
                              lbl_1_bss_224.entries[2].motionIds[0]);
                fn_1_759C();
                fn_1_A6C8(entry, 1);
                entry->activity.tap.frame = 0;
                fn_1_77B0(playerIndex);
            }
            break;
        case 3:
            if (++(entry->activity.tap.frame) >= 120) {
                fn_1_759C();
                entry->activity.tap.frame = 0;
                Hu3DMotionSet(lbl_1_bss_224.entries[2].modelId,
                              lbl_1_bss_224.entries[2].motionIds[0]);
                fn_1_A6C8(entry, 1);
            }
            if (entry->activity.tap.frame == 60) {
                fn_1_7660();
            }
            break;
        case 5:
            if (entry->position.x >= 530.0f + (2000.0f + lbl_1_data_348[2].x)) {
                Hu3DMotionSet(lbl_1_bss_224.entries[2].modelId,
                              lbl_1_bss_224.entries[2].motionIds[0]);
                fn_1_A6C8(entry, 6);
            } else {
                entry->position.x += 6.0f;
            }
            Hu3DModelPosSetV(entry->modelId,
                             &entry->position);
            break;
        case 4:
        case 6:
        default:
            break;
    }
    if (fn_1_7940() == 1 && entry->status < 5) {
        fn_1_7660();
        fn_1_A6C8(entry, 5);
        fn_1_32F4(playerIndex);
        Hu3DMotionSet(lbl_1_bss_224.entries[2].modelId,
                      lbl_1_bss_224.entries[2].motionIds[1]);
    }
}

/* Chooses the next tap prompt and shows it before the finish phase begins. */
void fn_1_759C(void) {
    M647ActivityEntry *entry;

    entry = &lbl_1_bss_224.entries[2];
    entry->activity.tap.buttonIndex = frandmod(6);
    if (lbl_1_bss_174.finishPhaseActive == 0) {
        HuSprAttrReset(entry->groupId, 2, 4);
        HuSprAttrReset(entry->groupId, entry->activity.tap.buttonIndex + 4, 4);
    }
}

/* Shows tap activity sprite slot 3 and plays the error sound. */
void fn_1_7618(void) {
    M647ActivityEntry *entry = &lbl_1_bss_224.entries[2];

    HuSprAttrReset(entry->groupId, 3, 4);
    HuAudFXPlay(MSM_SE_M647_2010);
}

/* Hides the tap activity's two markers and six button prompts. */
void fn_1_7660(void) {
    M647ActivityEntry *entry;
    s32 attributeIndex;

    entry = &lbl_1_bss_224.entries[2];
    HuSprAttrSet(entry->groupId, 2, 4);
    HuSprAttrSet(entry->groupId, 3, 4);
    for (attributeIndex = 0; attributeIndex < 6; attributeIndex++) {
        HuSprAttrSet(entry->groupId, attributeIndex + 4, 4);
    }
}

/* Reads the player's button press for the current tap prompt. */
s16 fn_1_76E0(s16 playerIndex) {
    M647ActivityEntry *entry;
    s16 padIndex;
    s16 cpuDifficulty;
    u32 buttons;

    padIndex = fn_1_2B4C(playerIndex);
    entry = &lbl_1_bss_224.entries[2];
    cpuDifficulty = fn_1_2B30(playerIndex);
    if ((s16)cpuDifficulty == -1) {
        buttons = HuPadBtnDown[(s16)padIndex];
    } else {
        buttons = fn_1_788C(playerIndex);
    }
    buttons = (u16)(buttons & 0x6F00);
    if (buttons != 0) {
        if ((u16)buttons == lbl_1_data_456[entry->activity.tap.buttonIndex]) {
            return 1;
        }
        return -1;
    }
    return 0;
}

/* Sets a computer player's tap delay, or clears the delay for a human player. */
void fn_1_77B0(s16 playerIndex) {
    s16 cpuDifficulty;
    M647ActivityEntry *entry;

    cpuDifficulty = fn_1_2B30(playerIndex);
    entry = &lbl_1_bss_224.entries[2];
    if ((s16)cpuDifficulty == -1) {
        entry->activity.tap.inputDelay = 0;
        return;
    }
    entry->activity.tap.inputDelay = lbl_1_data_4C6[(s16) cpuDifficulty] +
                                     (lbl_1_data_4C6[(s16) cpuDifficulty] *
                                       lbl_1_data_4CE[(s16) cpuDifficulty] * frandmod(100)) /
                                         10000;
}

/* Produces a computer player's tap input after its response delay. */
u32 fn_1_788C(s16 playerIndex) {
    s16 cpuDifficulty;
    M647ActivityEntry *entry;

    cpuDifficulty = fn_1_2B30(playerIndex);
    entry = &lbl_1_bss_224.entries[2];
    if (entry->activity.tap.inputDelay > 0) {
        entry->activity.tap.inputDelay -= 1;
        return 0;
    }
    if (frandmod(100) < lbl_1_data_4BE[(s16)cpuDifficulty]) {
        return 65535;
    }
    return lbl_1_data_456[entry->activity.tap.buttonIndex];
}

/* Checks the tap entry's finish threshold before the finish phase, while fewer than three
 * players have finished. */
s16 fn_1_7940(void) {
    if (lbl_1_bss_174.finishPhaseActive != 0) {
        return 0;
    }
    if (lbl_1_bss_18 >= 3) {
        return 0;
    }
    if (lbl_1_bss_224.entries[2].position.x >= (((2000.0f) + lbl_1_data_348[2].x) - (30.0f))) {
        return 1;
    }
    return 0;
}

/* Installs the assigned player's character icon in the row activity's group. */
void fn_1_79CC(void) {
    HUSPR_GROUPID groupId;

    groupId = lbl_1_bss_224.entries[3].groupId;
    HuSprGrpMemberSet(groupId, 1, HuSprCreate(lbl_1_bss_1AC[fn_1_34A0(3)], 10, 0));
}

/* Creates and positions the row activity's sprite group during setup. */
void fn_1_7A38(void) {
    M647ActivityEntry *entry;
    HUSPR_GROUPID groupId;
    HU3D_MODELID itemModelId;
    s32 buttonIndex;

    entry = &lbl_1_bss_224.entries[3];
    itemModelId = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_3E4[lbl_1_bss_174.nightMode],
                              HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelHookSet(entry->modelId, "itemhook2", itemModelId);
    entry->activity.row.itemModelId = itemModelId;

    groupId = entry->groupId = HuSprGrpCreate(12);
    fn_1_7A38_setupGroupSprites(groupId);
    for (buttonIndex = 0; buttonIndex < 4; buttonIndex++) {
        HuSprGrpMemberSet(groupId, (s16)(buttonIndex * 2 + 4),
                          HuSprCreate(lbl_1_bss_1F0[buttonIndex], 10, 0));
        HuSprGrpMemberSet(groupId, (s16)(buttonIndex * 2 + 5),
                          HuSprCreate(lbl_1_bss_1D8[buttonIndex], 10, 0));
        HuSprPosSet(groupId, (s16)(buttonIndex * 2 + 4), 53.0f, 0.0f);
        HuSprPosSet(groupId, (s16)(buttonIndex * 2 + 5), 53.0f, 0.0f);
        HuSprAttrSet(groupId, (s16)(buttonIndex * 2 + 4), 4);
        HuSprAttrSet(groupId, (s16)(buttonIndex * 2 + 5), 4);
    }
}

s16 lbl_1_data_4F4[4] = {30, 20, 5, 0};
s16 lbl_1_data_4FC[4] = {50, 45, 40, 38};
s16 lbl_1_data_504[4] = {3, 2, 0, 0};
s16 lbl_1_data_50C[4] = {20, 10, 8, 5};
s16 lbl_1_data_514[4] = {50, 45, 42, 42};

/* Updates one activity each frame, advancing its motion and player state. */
void fn_1_7D68(s16 playerIndex) {
    M647ActivityEntry *entry;
    Point3d movement;
    s16 inputResult;

    entry = &lbl_1_bss_224.entries[3];
    switch (entry->status) {
        case 0:
            fn_1_80E4();
            fn_1_A6C8(entry, 1);
            HuSprAttrReset(entry->groupId, 0, 4);
            HuSprAttrReset(entry->groupId, 2, 4);
            break;
        case 1:
            fn_1_823C();
            inputResult = fn_1_831C(playerIndex);
            if (inputResult > 0) {
                (entry->activity.row.successCount)++;
                if (entry->activity.row.successCount >= 10) {
                    fn_1_81BC();
                    fn_1_A6C8(entry, 2);
                    HuAudFXPlay(MSM_SE_M647_2008);
                    entry->activity.row.successCount = 0;
                    Hu3DMotionSet(lbl_1_bss_224.entries[3].modelId,
                                  lbl_1_bss_224.entries[3].motionIds[1]);
                    HuAudFXPlay(MSM_SE_M647_2009);
                }
                fn_1_83EC(playerIndex);
            } else if (inputResult < 0) {
                fn_1_8174();
                Hu3DMotionSet(lbl_1_bss_224.entries[3].modelId,
                              lbl_1_bss_224.entries[3].motionIds[2]);
                fn_1_A6C8(entry, 3);
                entry->activity.row.frame = 0;
                entry->activity.row.successCount = 0;
                fn_1_83EC(playerIndex);
                omVibrate(playerIndex, 20, 7, 3);
            }
            break;
        case 2:
            movement.x = 6.25f;
            entry->position.x += movement.x;
            movement.y = movement.z = 0.0f;
            Hu3DModelPosSetV(entry->modelId, &entry->position);
            if ((entry->activity.row.frame += 1) >= 40) {
                Hu3DMotionSet(lbl_1_bss_224.entries[3].modelId,
                              lbl_1_bss_224.entries[3].motionIds[0]);
                fn_1_80E4();
                fn_1_A6C8(entry, 1);
                entry->activity.row.frame = 0;
                fn_1_83EC(playerIndex);
            }
            break;
        case 3:
            if ((entry->activity.row.frame += 1) >= 120) {
                fn_1_80E4();
                entry->activity.row.frame = 0;
                Hu3DMotionSet(lbl_1_bss_224.entries[3].modelId,
                              lbl_1_bss_224.entries[3].motionIds[0]);
                fn_1_A6C8(entry, 1);
            }
            if (entry->activity.row.frame == 60) {
                fn_1_81BC();
            }
            break;
        case 4:
            break;
        case 5:
            entry->position.x += 6.0f;
            /* The row activity's final-travel threshold uses the tap activity's starting X. */
            if (entry->position.x >= (470.0f) + ((2000.0f) + lbl_1_data_348[2].x)) {
                Hu3DMotionSet(lbl_1_bss_224.entries[3].modelId,
                              lbl_1_bss_224.entries[3].motionIds[0]);
                fn_1_A6C8(entry, 6);
            }
            movement.x = (6.0f);
            movement.y = movement.z = (0.0f);
            Hu3DModelPosSetV(entry->modelId, &entry->position);
            break;
        case 6:
            break;
    }
    if (fn_1_85CC() == 1 && entry->status < 5) {
        fn_1_A6C8(entry, 5);
        fn_1_32F4(playerIndex);
        Hu3DMotionSet(lbl_1_bss_224.entries[3].modelId, lbl_1_bss_224.entries[3].motionIds[1]);
        fn_1_81BC();
    }
}

/* Chooses a row activity button and shows its prompt markers before the finish phase begins. */
void fn_1_80E4(void) {
    M647ActivityEntry *entry;

    entry = &lbl_1_bss_224.entries[3];
    entry->activity.row.buttonIndex = frandmod(4);
    if (lbl_1_bss_174.finishPhaseActive == 0) {
        HuSprAttrReset(entry->groupId, 2, 4);
        HuSprAttrReset(entry->groupId,
                       (s16)(entry->activity.row.buttonIndex * 2 + 4), 4);
        entry->activity.row.blinkVisible = 0;
        entry->activity.row.blinkFrame = 0;
    }
}

/* Shows a row activity marker and plays the error sound after a wrong input. */
void fn_1_8174(void) {
    M647ActivityEntry *entry = &lbl_1_bss_224.entries[3];

    HuSprAttrReset(entry->groupId, 3, 4);
    HuAudFXPlay(MSM_SE_M647_2010);
}

/* Hides the row activity's prompt sprites. */
void fn_1_81BC(void) {
    M647ActivityEntry *entry;
    s32 attributeIndex;

    entry = &lbl_1_bss_224.entries[3];
    HuSprAttrSet(entry->groupId, 2, 4);
    HuSprAttrSet(entry->groupId, 3, 4);
    for (attributeIndex = 0; attributeIndex < 8; attributeIndex++) {
        HuSprAttrSet(entry->groupId, (s16)(attributeIndex + 4), 4);
    }
}

/* Updates the row activity's blinking button indicators. */
void fn_1_823C(void) {
    M647ActivityEntry *entry;

    entry = &lbl_1_bss_224.entries[3];
    if ((entry->activity.row.blinkFrame)++ > 8) {
        if (entry->activity.row.blinkVisible != 0) {
            HuSprAttrReset(entry->groupId,
                           (s16)(entry->activity.row.buttonIndex * 2 + 4), 4);
            HuSprAttrSet(entry->groupId,
                         (s16)(entry->activity.row.buttonIndex * 2 + 5), 4);
            entry->activity.row.blinkVisible = 0;
        } else {
            HuSprAttrSet(entry->groupId,
                         (s16)(entry->activity.row.buttonIndex * 2 + 4), 4);
            HuSprAttrReset(entry->groupId,
                           (s16)(entry->activity.row.buttonIndex * 2 + 5), 4);
            entry->activity.row.blinkVisible = 1;
        }
        entry->activity.row.blinkFrame = 0;
    }
}

/* Reads and classifies the player's input for the row activity. */
s16 fn_1_831C(s16 playerIndex) {
    s16 padIndex;
    M647ActivityEntry *entry;
    s16 cpuDifficulty;
    u16 buttons;

    padIndex = fn_1_2B4C(playerIndex);
    entry = &lbl_1_bss_224.entries[3];
    cpuDifficulty = fn_1_2B30(playerIndex);
    if (cpuDifficulty == -1) {
        buttons = HuPadBtnDown[padIndex];
    } else {
        buttons = fn_1_84E4(playerIndex);
    }
    buttons &= 0x6F00;
    if (buttons != 0) {
        if (buttons == lbl_1_data_456[entry->activity.row.buttonIndex]) {
            return 1;
        }
        return -1;
    }
    return 0;
}

/* Sets a computer player's row input delay, or clears it for a human player. */
void fn_1_83EC(s16 playerIndex) {
    M647ActivityEntry *entry;
    s16 cpuDifficulty;
    s16 baseDelay;
    cpuDifficulty = fn_1_2B30(playerIndex);
    entry = &lbl_1_bss_224.entries[3];
    if (cpuDifficulty == - 1)
    {
        entry->activity.row.inputDelay = 0;
        return;
    }
    if (entry->activity.row.successCount == 0)
    {
        baseDelay = lbl_1_data_4FC[cpuDifficulty];
    }
    else
    {
        baseDelay = lbl_1_data_50C[cpuDifficulty];
    }
    entry->activity.row.inputDelay =
        (s32) baseDelay + (baseDelay * lbl_1_data_514[cpuDifficulty] * frandmod(100)) / 10000;
}

/* Produces a computer player's row activity input after its response delay. */
u16 fn_1_84E4(s16 playerIndex) {
    s16 cpuDifficulty;
    M647ActivityEntry *entry;
    s16 chance;

    cpuDifficulty = fn_1_2B30(playerIndex);
    entry = &lbl_1_bss_224.entries[3];
    if (entry->activity.row.inputDelay > 0) {
        entry->activity.row.inputDelay -= 1;
        return 0;
    }
    if (entry->activity.row.successCount == 0) {
        chance = lbl_1_data_4F4[(s16)cpuDifficulty];
    } else {
        chance = lbl_1_data_504[(s16)cpuDifficulty];
    }
    if (frandmod(100) < chance) {
        return 65535;
    }
    return lbl_1_data_456[entry->activity.row.buttonIndex];
}

/* Checks the row entry's finish threshold before the finish phase, while fewer than three
* players have finished. */
s16 fn_1_85CC(void) {
    if (lbl_1_bss_174.finishPhaseActive != 0) {
        return 0;
    }
    if (lbl_1_bss_18 >= 3) {
        return 0;
    }
    if (lbl_1_bss_224.entries[3].position.x >= ((2000.0f) + lbl_1_data_348[3].x)) {
        return 1;
    }
    return 0;
}

/* Returns the model ID for the grid activity entry. */
s16 fn_1_8648(void) {
    return lbl_1_bss_224.entries[0].modelId;
}

/* Installs the assigned player's character icon in the grid activity's group. */
void fn_1_8658(void) {
    HUSPR_GROUPID groupId;

    groupId = lbl_1_bss_224.entries[0].groupId;
    HuSprGrpMemberSet(groupId, 1, HuSprCreate(lbl_1_bss_1AC[fn_1_34A0(0)], 10, 0));
}

/* Creates the grid activity's item model and sprite group during setup. */
void fn_1_86C4(void) {
    M647ActivityEntry *entry;
    HU3D_MODELID itemModelId;
    HUSPR_GROUPID groupId;
    s32 row;
    s32 column;

    entry = &lbl_1_bss_224.entries[0];
    itemModelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_3C4[lbl_1_bss_174.nightMode],
        HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelHookSet(entry->modelId, "itemhook2", itemModelId);
    entry->activity.grid.itemModelId = itemModelId;

    groupId = entry->groupId = HuSprGrpCreate(100);
    HuSprGrpPosSet(groupId, lbl_1_data_464[0], lbl_1_data_464[1]);
    HuSprGrpMemberSet(groupId, 0, HuSprCreate(lbl_1_bss_214[0], 15, 0));
    HuSprPosSet(groupId, 0, 0.0f, 0.0f);
    HuSprAttrSet(groupId, 0, 4);
    /* Member 1 is still empty: the position call uses sprite -1, and the attribute call is
     * ignored. */
    HuSprPosSet(groupId, 1, 0.0f, 0.0f);
    HuSprAttrSet(groupId, 1, 4);
    HuSprGrpMemberSet(groupId, 2, HuSprCreate(lbl_1_bss_208[1], 15, 0));
    HuSprPosSet(groupId, 2, 164.0f, 0.0f);
    HuSprTPLvlSet(groupId, 2, 0.7f);
    HuSprAttrSet(groupId, 2, 4);
    HuSprGrpMemberSet(groupId, 3, HuSprCreate(lbl_1_bss_1A8, 8, 0));
    HuSprAttrSet(groupId, 3, 4);
    for (row = 0; row < 8; row++) {
        for (column = 0; column < 6; column++) {
            HuSprGrpMemberSet(groupId,
                               (s16)(column * 2 + 4 + row * 12),
                               HuSprCreate(lbl_1_bss_1F0[column], 10, 0));
            HuSprPosSet(groupId, (s16)(column * 2 + 4 + row * 12),
                        (f32)((row << 5) + 56), 0.0f);
            HuSprAttrSet(groupId, (s16)(column * 2 + 4 + row * 12), 4);
            HuSprGrpMemberSet(groupId,
                               (s16)(column * 2 + 5 + row * 12),
                               HuSprCreate(lbl_1_bss_1C0[column], 10, 0));
            HuSprPosSet(groupId, (s16)(column * 2 + 5 + row * 12),
                        (f32)((row << 5) + 56), 0.0f);
            HuSprAttrSet(groupId, (s16)(column * 2 + 5 + row * 12), 4);
        }
    }
}

s16 lbl_1_data_538[4] = {10, 8, 5, 0};
s16 lbl_1_data_540[4] = {40, 30, 23, 17};
s16 lbl_1_data_548[4] = {40, 35, 32, 29};
s16 lbl_1_data_550[4] = {30, 25, 25, 20};

/* Debug hook: empty in the release build; its argument is still evaluated. */
static inline void DebugValue(s32 inputResult) {
}

/* Updates one activity's displayed sprites and results during the round. */
void fn_1_8A40(s16 playerIndex) {
    M647ActivityEntry *state;
    Point3d movement;
    s32 inputResult;

    state = &lbl_1_bss_224.entries[0];
    switch (state->status) {
        case 0:
            fn_1_8D5C();
            fn_1_90D4(playerIndex);
            fn_1_A6C8(state, 1);
            break;
        case 1:
            inputResult = fn_1_8FF0(playerIndex);
            if ((s16)inputResult > 0) {
                fn_1_8E98();
                (state->activity.grid.row)++;
                if (state->activity.grid.row == 8) {
                    fn_1_8F2C();
                    Hu3DMotionSet(lbl_1_bss_224.entries[0].modelId,
                              lbl_1_bss_224.entries[0].motionIds[1]);
                    HuAudFXPlay(MSM_SE_M647_2000);
                    state->activity.grid.row = 0;
                    fn_1_A6C8(state, 2);
                    state->activity.grid.row = 0;
                }
                fn_1_90D4(playerIndex);
            } else if ((s16)inputResult < 0) {
                DebugValue(inputResult);
            }
            break;
        case 2:
            if (state->activity.grid.frame >= 10 &&
                 state->activity.grid.frame < 45) {
                movement.x = (11.428572f);
                state->position.x += movement.x;
                movement.y = movement.z = (0.0f);
                Hu3DModelPosSetV(state->modelId, &state->position);
            }
            if ((f32)state->activity.grid.frame == 45.0f) {
                HuAudFXPlay(MSM_SE_M647_2001);
            }
            if (++(state->activity.grid.frame) >= 60) {
                Hu3DMotionSet(lbl_1_bss_224.entries[0].modelId,
                              lbl_1_bss_224.entries[0].motionIds[0]);
                fn_1_8D5C();
                fn_1_A6C8(state, 1);
                state->activity.grid.frame = 0;
                fn_1_90D4(playerIndex);
            }
            break;
        case 3:
            if (++(state->activity.grid.frame) >= 120) {
                fn_1_8D5C();
                state->activity.grid.frame = 0;
                Hu3DMotionSet(lbl_1_bss_224.entries[0].modelId,
                              lbl_1_bss_224.entries[0].motionIds[0]);
                fn_1_A6C8(state, 1);
            }
            if (state->activity.grid.frame == 60) {
                fn_1_8F2C();
            }
            break;
        case 4:
            break;
        case 5:
            if (state->activity.grid.frame >= 30 &&
                state->activity.grid.frame < 60) {
                state->position.x += (12.0f);
            }
            Hu3DModelPosSetV(state->modelId, &state->position);
            if (++(state->activity.grid.frame) >= 60) {
                Hu3DMotionSet(lbl_1_bss_224.entries[0].modelId,
                              lbl_1_bss_224.entries[0].motionIds[0]);
                fn_1_A6C8(state, 6);
            }
            break;
        case 6:
            break;
        default:
            break;
    }
    if ((s16)fn_1_9300() == 1 && state->status < 5) {
        fn_1_8F2C();
        fn_1_A6C8(state, 5);
        fn_1_32F4(playerIndex);
        state->activity.grid.frame = 0;
    }
}

/* Before the finish phase begins, randomizes the grid activity's button order and shows its
* prompts. */
void fn_1_8D5C(void) {
    M647ActivityTable *table;
    s32 index;

    table = &lbl_1_bss_224;
    if (lbl_1_bss_174.finishPhaseActive == 0) {
        HuSprAttrReset(table->entries[0].groupId, 2, 4);
        for (index = 0; index < 8; index++) {
            table->entries[0].activity.grid.buttonOrder[index] = frandmod(6);
            HuSprAttrReset(table->entries[0].groupId,
                            (s16)(4 + index * 12 +
                                 table->entries[0].activity.grid.buttonOrder[index] * 2), 4);
        }
    }
}

/* Moves and shows the grid activity's row indicator, then plays its sound. */
void fn_1_8E0C(void) {
    M647ActivityEntry *entry;

    entry = &lbl_1_bss_224.entries[0];
    HuSprPosSet(entry->groupId, 3,
                 (f32)((entry->activity.grid.row << 5) + 56), (0.0f));
    HuSprAttrReset(entry->groupId, 3, 4);
    HuAudFXPlay(MSM_SE_M647_2010);
}

/* Updates visibility of the current grid prompt sprites. */
void fn_1_8E98(void) {
    M647ActivityTable *table;
    s16 animationRow;
    s16 cellIndex;

    table = &lbl_1_bss_224;
    animationRow = table->entries[0].activity.grid.row;
    cellIndex = table->entries[0].activity.grid.buttonOrder[animationRow];
    HuSprAttrSet(table->entries[0].groupId,
                  (s16)(animationRow * 12 + 4 + cellIndex * 2), 4);
    HuSprAttrReset(table->entries[0].groupId,
                    (s16)(animationRow * 12 + 5 + cellIndex * 2), 4);
}

/* Hides all grid activity indicator sprites. */
void fn_1_8F2C(void) {
    s32 row;
    s32 column;
    M647ActivityTable *table;

    table = &lbl_1_bss_224;
    HuSprAttrSet(table->entries[0].groupId, 2, 4);
    HuSprAttrSet(table->entries[0].groupId, 3, 4);
    row = 0;
    while (row < 8) {
        column = 0;
        while (column < 6) {
            HuSprAttrSet(table->entries[0].groupId,
                          (s16)(column * 2 + 4 + row * 12), 4);
            HuSprAttrSet(table->entries[0].groupId,
                          (s16)(column * 2 + 5 + row * 12), 4);
            column++;
        }
        row++;
    }
}

/* Returns the input result for one player during the activity round. */
s32 fn_1_8FF0(s16 playerIndex) {
    s16 padIndex;
    M647ActivityTable *table;
    s16 cpuDifficulty;
    s16 promptIndex;
    u16 buttons;

    padIndex = fn_1_2B4C(playerIndex);
    table = &lbl_1_bss_224;
    cpuDifficulty = fn_1_2B30(playerIndex);
    promptIndex = table->entries[0].activity.grid.row;
    if (cpuDifficulty == -1) {
        buttons = HuPadBtnDown[padIndex];
    } else {
        buttons = fn_1_9238(playerIndex);
    }
    buttons &= 0x6F00;
    if (buttons != 0) {
        if (buttons == lbl_1_data_456[table->entries[0].activity.grid.buttonOrder[promptIndex]]) {
            HuAudFXPlay(MSM_SE_M647_1999);
            return 1;
        }
        return -1;
    }
    return 0;
}

/* Sets the grid activity's input delay for this player, or clears it for a human player. */
void fn_1_90D4(s16 playerIndex) {
    M647ActivityEntry *gridEntry;
    s16 cpuDifficulty;

    cpuDifficulty = fn_1_2B30(playerIndex);
    gridEntry = &lbl_1_bss_224.entries[0];

    if ((s16)cpuDifficulty == -1) {
        gridEntry->activity.grid.inputDelay = 0;
        return;
    }

    if (gridEntry->activity.grid.row == 0) {
        gridEntry->activity.grid.inputDelay =
            (s16)(lbl_1_data_548[(s16)cpuDifficulty] +
                  ((lbl_1_data_548[(s16)cpuDifficulty] * lbl_1_data_550[(s16)cpuDifficulty] *
                    frandmod(100)) / 10000));
    } else {
        gridEntry->activity.grid.inputDelay =
            (s16)(lbl_1_data_540[(s16)cpuDifficulty] +
                  ((lbl_1_data_540[(s16)cpuDifficulty] * lbl_1_data_550[(s16)cpuDifficulty] *
                    frandmod(100)) / 10000));
    }
}

/* Reads a computer player's activity input after its configured delay. */
u16 fn_1_9238(s16 playerIndex) {
    s16 cpuDifficulty;
    M647ActivityTable *table;
    s16 promptIndex;

    cpuDifficulty = fn_1_2B30(playerIndex);
    table = &lbl_1_bss_224;
    promptIndex = table->entries[0].activity.grid.row;
    if (table->entries[0].activity.grid.inputDelay > 0) {
        table->entries[0].activity.grid.inputDelay -= 1;
        return 0;
    }
    if (frandmod(100) < lbl_1_data_538[(s16)cpuDifficulty]) {
        return 65535;
    }
    return lbl_1_data_456[table->entries[0].activity.grid.buttonOrder[promptIndex]];
}

/* Returns true before the finish phase begins, when fewer than three players have finished and
* the grid entry reaches its finish threshold. */
s32 fn_1_9300(void) {
    if (lbl_1_bss_174.finishPhaseActive != 0) {
        return 0;
    }
    if (lbl_1_bss_18 >= 3) {
        return 0;
    }
    if (lbl_1_bss_224.entries[0].position.x >= ((2000.0f) + lbl_1_data_348[0].x)) {
        return 1;
    }
    return 0;
}

/* Installs the assigned player's character icon in the lever activity's group. */
void fn_1_937C(void) {
    HUSPR_GROUPID groupId;

    groupId = lbl_1_bss_224.entries[1].groupId;
    HuSprGrpMemberSet(groupId, 1, HuSprCreate(lbl_1_bss_1AC[fn_1_34A0(1)], 10, 0));
}

/* Creates the lever activity's attached models and indicator sprite group. */
void fn_1_93E8(void) {
    HU3D_MODELID itemModelId;
    HUSPR_GROUPID groupId;
    M647ActivityEntry *entry;

    entry = &lbl_1_bss_224.entries[1];
    itemModelId = Hu3DModelCreate(
         HuDataSelHeapReadNum(DATANUM(DATA_m647, 25), HU_MEMNUM_OVL, HEAP_MODEL));
    entry->activity.lever.rightModelId = itemModelId;
    Hu3DModelHookSet(entry->modelId, "itemhook_rt", itemModelId);
    Hu3DMotionSpeedSet(itemModelId, 0.0f);
    Hu3DModelAttrSet(itemModelId, HU3D_MOTATTR_LOOP);

    itemModelId = Hu3DModelCreate(
         HuDataSelHeapReadNum(DATANUM(DATA_m647, 26), HU_MEMNUM_OVL, HEAP_MODEL));
    entry->activity.lever.frontModelId = itemModelId;
    Hu3DModelHookSet(entry->modelId, "itemhook_ft", itemModelId);
    Hu3DMotionSpeedSet(itemModelId, 0.0f);
    Hu3DModelAttrSet(itemModelId, HU3D_MOTATTR_LOOP);

    itemModelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_3D4[lbl_1_bss_174.nightMode],
        HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelHookSet(entry->modelId, "itemhook_L1", itemModelId);
    entry->activity.lever.leftModelIds[0] = itemModelId;

    itemModelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        (&lbl_1_data_3D4[lbl_1_bss_174.nightMode])[2],
        HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelHookSet(entry->modelId, "itemhook_L2", itemModelId);
    entry->activity.lever.leftModelIds[1] = itemModelId;

    groupId = entry->groupId = HuSprGrpCreate(5);
    HuSprGrpPosSet(groupId, lbl_1_data_464[2], lbl_1_data_464[3]);
    HuSprGrpMemberSet(groupId, 0, HuSprCreate(lbl_1_bss_214[1], 15, 0));
    HuSprPosSet(groupId, 0, 0.0f, 0.0f);
    HuSprAttrSet(groupId, 0, 4);
    /* Member 1 is still empty: the position call uses sprite -1, and the attribute call is
     * ignored. */
    HuSprPosSet(groupId, 1, 0.0f, 0.0f);
    HuSprAttrSet(groupId, 1, 4);
    HuSprGrpMemberSet(groupId, 2, HuSprCreate(lbl_1_bss_208[0], 15, 0));
    HuSprPosSet(groupId, 2, 50.0f, 0.0f);
    HuSprTPLvlSet(groupId, 2, 0.7f);
    HuSprAttrSet(groupId, 2, 4);
    HuSprGrpMemberSet(groupId, 3, HuSprCreate(lbl_1_bss_1F0[0], 10, 0));
    HuSprAttrSet(groupId, 3, 4);
    HuSprPosSet(groupId, 3, 56.0f, 0.0f);
    HuSprGrpMemberSet(groupId, 4, HuSprCreate(lbl_1_bss_1BC, 10, 0));
    HuSprAttrSet(groupId, 4, 4);
    HuSprPosSet(groupId, 4, 56.0f, 0.0f);
}

/* fn_1_98CC shows the lever's two indicator sprites while idle before the finish phase begins. */
/* Shows the lever indicators while the activity is idle before the finish phase begins. */
void fn_1_975C(void) {
    M647ActivityEntry *entry;

    entry = &lbl_1_bss_224.entries[1];
    if (lbl_1_bss_174.finishPhaseActive == 0) {
        HuSprAttrReset(entry->groupId, 2, 4);
        HuSprAttrReset(entry->groupId, 3, 4);
    }
}

/* fn_1_98CC hides the lever indicators during a pull or reset. */
/* Hides the lever indicators during a pull or return motion. */
void fn_1_97C0(void) {
    M647ActivityEntry *entry;

    entry = &lbl_1_bss_224.entries[1];
    HuSprAttrSet(entry->groupId, 2, 4);
    HuSprAttrSet(entry->groupId, 3, 4);
    HuSprAttrSet(entry->groupId, 4, 4);
}

/* fn_1_98CC switches the idle indicators here only before the finish phase begins. */
/* Switches the lever's idle indicator as its counter advances before the finish phase begins. */
void fn_1_9820(void) {
    M647ActivityEntry *entry;

    entry = &lbl_1_bss_224.entries[1];
    if (lbl_1_bss_174.finishPhaseActive == 0) {
        if (entry->activity.lever.counter++ > 140) {
            HuSprAttrReset(entry->groupId, 3, 4);
            HuSprAttrSet(entry->groupId, 4, 4);
            return;
        }
        if (entry->activity.lever.counter > 100) {
            HuSprAttrSet(entry->groupId, 3, 4);
            HuSprAttrReset(entry->groupId, 4, 4);
        }
    }
}

s16 lbl_1_data_574[4] = {40, 30, 18, 5};
s16 lbl_1_data_57C[4] = {50, 30, 0, 0};
s16 lbl_1_data_584[4] = {50, 40, 38, 35};

/* Called each frame for the lever activity; handles input, motion and hook travel. */
void fn_1_98CC(s16 playerIndex) {
    Point3d positionDelta;
    Point3d initialPosition;
    s16 motionTime;
    s16 result;
    f32 movementSpeed;
    M647ActivityEntry *leverEntry;

    leverEntry = &lbl_1_bss_224.entries[1];
    initialPosition.x = leverEntry->position.x;
    initialPosition.y = leverEntry->position.y;
    initialPosition.z = leverEntry->position.z;
    positionDelta.y = positionDelta.z = 0.0f;
    Hu3DModelObjPosGet(leverEntry->modelId, "itemhook_ft",
                       &leverEntry->activity.lever.hookPosition);
    motionTime = (s16)Hu3DMotionTimeGet(leverEntry->modelId);
    leverEntry->activity.lever.savedMotionTime =
        Hu3DMotionTimeGet(leverEntry->activity.lever.frontModelId);

    switch (leverEntry->status) {
        case 0:
            fn_1_A2D4(playerIndex);
            Hu3DMotionSet(
                (&lbl_1_bss_224.entries[1])->modelId,
                lbl_1_bss_224.entries[1].motionIds[1]);
            fn_1_A6C8(leverEntry, 1);
            HuAudFXPlay(MSM_SE_M647_2002);
            Hu3DMotionSpeedSet(leverEntry->activity.lever.frontModelId, 1.0f);
            leverEntry->activity.lever.speed = 1.0f;
            fn_1_975C();
            break;
        case 1:
            fn_1_9820();
            if (leverEntry->activity.lever.phase == 0) {
                if ((f32)motionTime >= (118.0f - leverEntry->activity.lever.speed)) {
                    leverEntry->activity.lever.phase = 1;
                    Hu3DModelAttrSet(leverEntry->modelId, HU3D_MOTATTR_REV);
                    Hu3DModelAttrSet(leverEntry->activity.lever.frontModelId, HU3D_MOTATTR_REV);
                    Hu3DMotionTimeSet(leverEntry->activity.lever.frontModelId,
                                      leverEntry->activity.lever.savedMotionTime);
                    HuAudFXPlay(MSM_SE_M647_2003);
                }
            } else if ((f32)motionTime <= leverEntry->activity.lever.speed) {
                leverEntry->activity.lever.counter = 0;
                leverEntry->activity.lever.phase = 0;
                Hu3DModelAttrReset(leverEntry->modelId, HU3D_MOTATTR_REV);
                Hu3DModelAttrReset(leverEntry->activity.lever.frontModelId, HU3D_MOTATTR_REV);
                Hu3DMotionTimeSet(leverEntry->activity.lever.frontModelId,
                                  leverEntry->activity.lever.savedMotionTime);
                fn_1_A6C8(leverEntry, 1);
                HuAudFXPlay(MSM_SE_M647_2002);
            }
            result = fn_1_A200(playerIndex);
            if (result == 1) {
                leverEntry->activity.lever.counter = 0;
                Hu3DMotionSet(
                    (&lbl_1_bss_224.entries[1])->modelId,
                    lbl_1_bss_224.entries[1].motionIds[1]);
                HuAudFXPlay(MSM_SE_M647_2004);
                fn_1_A6C8(leverEntry, 2);
                if (leverEntry->activity.lever.phase == 0) {
                    HuAudFXPlay(MSM_SE_M647_2003);
                }
                fn_1_A2D4(playerIndex);
                Hu3DModelAttrSet(leverEntry->modelId, HU3D_MOTATTR_REV);
                Hu3DMotionSpeedSet(leverEntry->activity.lever.frontModelId, 0.0f);
                Hu3DMotionTimeSet(leverEntry->modelId, (f32)motionTime);
                leverEntry->activity.lever.speed = (f32)(motionTime + 80) / 150.0f;
                Hu3DMotionSpeedSet(leverEntry->modelId, leverEntry->activity.lever.speed);
                fn_1_97C0();
            } else if (result == -1) {
                fn_1_A2D4(playerIndex);
            }
            break;
        case 2:
            positionDelta.x = leverEntry->activity.lever.previousHookPosition.x -
                              leverEntry->activity.lever.hookPosition.x;
            if (positionDelta.x > 0.0f) {
                leverEntry->position.x += positionDelta.x;
                leverEntry->activity.lever.hookPosition.x += positionDelta.x;
                leverEntry->activity.lever.previousHookPosition.x += positionDelta.x;
                Hu3DModelPosSetV(leverEntry->modelId, &leverEntry->position);
            }
            if ((f32)motionTime < leverEntry->activity.lever.speed) {
                Hu3DModelAttrReset(leverEntry->modelId, HU3D_MOTATTR_REV);
                leverEntry->activity.lever.phase = 0;
                Hu3DModelAttrReset(leverEntry->activity.lever.frontModelId, HU3D_MOTATTR_REV);
                Hu3DMotionTimeSet(leverEntry->activity.lever.frontModelId,
                                  leverEntry->activity.lever.savedMotionTime);
                Hu3DMotionSpeedSet(leverEntry->activity.lever.frontModelId, 0.0f);
                fn_1_A6C8(leverEntry, 4);
                Hu3DMotionSpeedSet(leverEntry->modelId, 0.0f);
                fn_1_975C();
                leverEntry->activity.lever.speed = 0.0f;
            }
            break;
        case 4:
            if (leverEntry->activity.lever.frame++ > 25) {
                Hu3DMotionSpeedSet(leverEntry->activity.lever.frontModelId, 1.0f);
                Hu3DMotionSpeedSet(leverEntry->modelId, 1.0f);
                leverEntry->activity.lever.speed = 1.0f;
                leverEntry->status = 1;
                HuAudFXPlay(MSM_SE_M647_2002);
                leverEntry->activity.lever.frame = 0;
            }
            break;
        case 5:
            if (leverEntry->activity.lever.phase == 0) {
                positionDelta.x = leverEntry->activity.lever.previousHookPosition.x -
                                  leverEntry->activity.lever.hookPosition.x;
                if (positionDelta.x > 0.0f) {
                    leverEntry->position.x += positionDelta.x;
                    leverEntry->activity.lever.hookPosition.x += positionDelta.x;
                    leverEntry->activity.lever.previousHookPosition.x += positionDelta.x;
                    Hu3DModelPosSetV(leverEntry->modelId,
                                     (Point3d *) (void *) &leverEntry->position.x);
                }
                if (motionTime < 3) {
                    HuAudFXPlay(MSM_SE_M647_2002);
                    Hu3DModelAttrReset(leverEntry->modelId, HU3D_MOTATTR_REV);
                    leverEntry->activity.lever.phase = 1;
                }
            } else if (leverEntry->activity.lever.phase == 1) {
                if (motionTime >= 114) {
                    HuAudFXPlay(MSM_SE_M647_2003);
                    Hu3DModelAttrSet(leverEntry->modelId, HU3D_MOTATTR_REV);
                    Hu3DMotionTimeSet(leverEntry->modelId, (f32)motionTime);
                    leverEntry->activity.lever.phase = 2;
                }
            } else {
                positionDelta.x = leverEntry->activity.lever.previousHookPosition.x -
                                  leverEntry->activity.lever.hookPosition.x;
                if (positionDelta.x > 0.0f) {
                    leverEntry->position.x += positionDelta.x;
                    leverEntry->activity.lever.hookPosition.x += positionDelta.x;
                    leverEntry->activity.lever.previousHookPosition.x += positionDelta.x;
                    Hu3DModelPosSetV(leverEntry->modelId,
                                     (Point3d *) (void *) &leverEntry->position.x);
                }
                if (motionTime < 3) {
                    Hu3DModelAttrReset(leverEntry->modelId, HU3D_MOTATTR_REV);
                    Hu3DMotionTimeSet(leverEntry->modelId, (f32)motionTime);
                    fn_1_A6C8(leverEntry, 6);
                    Hu3DMotionSet(
                        lbl_1_bss_224.entries[1].modelId,
                        lbl_1_bss_224.entries[1].motionIds[0]);
                }
            }
            /* Final travel falls through the invalid-state assertions below, which are inactive
             * outside debug builds. */
        case 6:
            ASSERT(FALSE);
        case 3:
            ASSERT(FALSE);
    }
    movementSpeed = 0.18 * (f64)(leverEntry->position.x - initialPosition.x);
    Hu3DMotionSpeedSet(leverEntry->activity.lever.rightModelId, movementSpeed);
    fn_1_A1E4(leverEntry);
    if (fn_1_A478() == 1 && leverEntry->status < 5) {
        fn_1_A6C8(leverEntry, 5);
        fn_1_32F4(playerIndex);
        Hu3DModelAttrSet(leverEntry->modelId, HU3D_MOTATTR_REV);
        Hu3DMotionTimeSet(leverEntry->modelId, (f32)motionTime);
        Hu3DMotionSpeedSet(leverEntry->modelId, 3.0f);
        fn_1_97C0();
        leverEntry->activity.lever.phase = 0;
    }
}

/* Saves the current lever hook position for the next frame's movement delta. */
void fn_1_A1E4(M647ActivityEntry *state) {
    state->activity.lever.previousHookPosition.x = state->activity.lever.hookPosition.x;
    state->activity.lever.previousHookPosition.y = state->activity.lever.hookPosition.y;
    state->activity.lever.previousHookPosition.z = state->activity.lever.hookPosition.z;
}

/* fn_1_98CC polls the lever input and distinguishes the target button from other presses. */
/* Reads the player's lever input and reports target or other button presses. */
s16 fn_1_A200(s16 playerIndex) {
    M647ActivityEntry *entry;
    s16 padIndex;
    s16 cpuDifficulty;
    u32 buttons;

    padIndex = fn_1_2B4C(playerIndex);
    entry = &lbl_1_bss_224.entries[1];
    cpuDifficulty = fn_1_2B30(playerIndex);
    if ((s16)cpuDifficulty == -1) {
        buttons = HuPadBtnDown[(s16)padIndex];
    } else {
        buttons = fn_1_A3D8(playerIndex);
    }
    buttons = (u16)(buttons & 0x6F00);
    if (buttons != 0) {
        if ((u16)buttons == (1 << 8)) {
            HuAudFXPlay(MSM_SE_M647_2005);
            return 1;
        }
        return -1;
    }
    return 0;
}

/* Sets a computer player's next lever input delay, or clears it for a human player. */
void fn_1_A2D4(s16 playerIndex) {
    s16 cpuDifficulty;
    M647ActivityEntry * entry;
    s16 adjustment;
    cpuDifficulty = fn_1_2B30(playerIndex);
    entry = &lbl_1_bss_224.entries[1];
    if ((s16) cpuDifficulty == - 1)
    {
        entry->activity.lever.inputDelay = 0;
        return;
    }
    adjustment = lbl_1_data_574[(s16) cpuDifficulty] +
                 (lbl_1_data_574[(s16) cpuDifficulty] * lbl_1_data_584[(s16) cpuDifficulty] *
                  frandmod(100)) /
                     10000;
    if (! (frandmod(2)))
    {
        adjustment *= - 1;
    }
    entry->activity.lever.inputDelay = (s16)(118 - adjustment);
}

/* fn_1_A200 uses this to delay and choose button presses for a computer player. */
/* Produces a computer player's lever input after its response delay. */
u32 fn_1_A3D8(s16 playerIndex) {
    s16 cpuDifficulty;
    M647ActivityEntry *entry;

    cpuDifficulty = fn_1_2B30(playerIndex);
    entry = &lbl_1_bss_224.entries[1];
    if (entry->activity.lever.inputDelay > 0) {
        entry->activity.lever.inputDelay -= 1;
        return 0;
    }
    if (frandmod(100) < lbl_1_data_57C[(s16)cpuDifficulty]) {
        return 65535;
    } else {
        return (1 << 8);
    }
}

/* fn_1_98CC tests the lever hook's finish threshold before the finish phase, while fewer than
* three players have finished. */
/* The lever hook's threshold uses the grid activity's starting X, plus 2130 world units. */
s16 fn_1_A478(void) {
    M647ActivityEntry *entry;

    entry = &lbl_1_bss_224.entries[1];
    if (lbl_1_bss_174.finishPhaseActive != 0) {
        return 0;
    }
    if (lbl_1_bss_18 >= 3) {
        return 0;
    }
    if (entry->activity.lever.hookPosition.x >=
        ((130.0f) + ((2000.0f) + lbl_1_data_348[0].x))) {
        return 1;
    }
    return 0;
}

/* Returns the model ID for the requested activity entry. */
s16 fn_1_A518(s16 entryIndex) {
    return lbl_1_bss_224.entries[entryIndex].modelId;
}

/* Prepares activity models for results, positioning the winner's model and hiding the others. */
void fn_1_A534(void) {
    s32 entryIndex;
    M647ActivityEntry *entry;
    s32 index;

    for (index = 0; index < 4; index++) {
        entryIndex = fn_1_3484((s16)index);
        entry = &lbl_1_bss_224.entries[(s16)entryIndex];

        if (lbl_1_bss_18 == 0) {
            Hu3DMotionSet(lbl_1_bss_224.entries[(s16)entryIndex].modelId,
                          lbl_1_bss_224.entries[(s16)entryIndex].motionIds[0]);
        } else if ((s16)fn_1_3468((s16)index) == 0) {
            Hu3DModelPosSet(entry->modelId, lbl_1_data_378[0],
                            lbl_1_data_348[(s16)entryIndex].y, lbl_1_data_378[2]);
            Hu3DModelRotSet(entry->modelId, (0.0f),
                            (-45.0f), (0.0f));
            Hu3DMotionSet(lbl_1_bss_224.entries[(s16)entryIndex].modelId,
                          lbl_1_bss_224.entries[(s16)entryIndex].motionIds[0]);
        } else {
            Hu3DModelAttrSet(entry->modelId, 1);
            if ((s16)entryIndex == 1) {
                Hu3DModelAttrSet(entry->activity.lever.rightModelId, 1);
                Hu3DModelAttrSet(entry->activity.lever.frontModelId, 1);
            }
        }
    }
}

/* Sets an activity state; after the finish phase begins, requests below 5 instead force status
* 6 and motion 0. */
void fn_1_A6C8(M647ActivityEntry *state, s16 mode) {
    if (lbl_1_bss_174.finishPhaseActive == 0) {
        state->status = mode;
        return;
    }
    if (mode < 5) {
        state->status = 6;
        Hu3DMotionSet(state->modelId, state->motionIds[0]);
        return;
    }
    state->status = mode;
}

/* Returns whether every activity entry has reached its completed state. */
s32 fn_1_A740(void) {
    s32 entryIndex;

    entryIndex = 0;
    while (entryIndex < 4) {
        if (lbl_1_bss_224.entries[entryIndex].status < 6) {
            return 0;
        }
        entryIndex++;
    }
    return 1;
}

/* Returns the greatest X among entries outside status 6, capped at 1100; three completed entries
 * force the return value to 1100. */
f32 fn_1_A790(void) {
    f32 maximumProgress;
    s32 completedCount;
    s32 index;
    M647ActivityEntry *entry;

    maximumProgress = (-10000.0f);
    index = 0;
    completedCount = 0;
    while (index < 4) {
        entry = &lbl_1_bss_224.entries[index];
        if (entry->status == 6) {
            completedCount++;
        } else if (maximumProgress < entry->position.x) {
            maximumProgress = entry->position.x;
        }
        index++;
    }
    if (completedCount >= 3) {
        maximumProgress = (1100.0f);
    }
    if (maximumProgress > (1100.0f)) {
        maximumProgress = (1100.0f);
    }
    return maximumProgress;
}

/* Returns the lowest X position among the four activity entries. */
/* Returns the lowest X position among the four activity entries. */
f32 fn_1_A854(void) {
    f32 minimumEntryX;
    s32 entryIndex;
    M647ActivityEntry *entry;

    minimumEntryX = (10000.0f);
    entryIndex = 0;
    while (entryIndex < 4) {
        entry = &lbl_1_bss_224.entries[entryIndex];
        if (minimumEntryX > entry->position.x) {
            minimumEntryX = entry->position.x;
        }
        entryIndex += 1;
    }
    return minimumEntryX;
}

/* Ends activity states below 5 at round end, except grid status 2, whose looping is disabled.
 * Status 5 finish animations continue unchanged. */
void fn_1_A8C4(void) {
    s32 index;

    for (index = 0; index < 4; index++) {
        if (index == 1) {
            if (lbl_1_bss_224.entries[index].status < 5) {
                Hu3DMotionSpeedSet(lbl_1_bss_224.entries[index].modelId, (0.0f));
                lbl_1_bss_224.entries[index].status = 6;
            }
        } else if (index == 0) {
            if (lbl_1_bss_224.entries[index].status < 5 &&
                lbl_1_bss_224.entries[index].status != 2) {
                lbl_1_bss_224.entries[index].status = 6;
            } else if (lbl_1_bss_224.entries[index].status == 2) {
                Hu3DModelAttrReset(lbl_1_bss_224.entries[index].modelId, HU3D_MOTATTR_LOOP);
            }
        } else if (lbl_1_bss_224.entries[index].status < 5) {
            lbl_1_bss_224.entries[index].status = 6;
            Hu3DMotionSet(lbl_1_bss_224.entries[(s16)index].modelId,
                          lbl_1_bss_224.entries[(s16)index].motionIds[0]);
        }
    }
}

/* Hides the activity indicator sprites when the result presentation begins. */
void fn_1_AA64(void) {
    M647ActivityEntry *entry1;
    M647ActivityEntry *entry0;
    M647ActivityEntry *entry2;
    M647ActivityEntry *entry3;
    s32 entryIndex;
    s32 column;
    s32 row;
    s32 secondCell;
    s32 thirdCell;

    entry1 = &lbl_1_bss_224.entries[1];
    HuSprAttrSet(entry1->groupId, 2, 4);
    HuSprAttrSet(entry1->groupId, 3, 4);
    HuSprAttrSet(entry1->groupId, 4, 4);
    entry0 = &lbl_1_bss_224.entries[0];
    HuSprAttrSet(entry0->groupId, 2, 4);
    HuSprAttrSet(entry0->groupId, 3, 4);
    for (row = 0; row < 8; row++) {
        for (column = 0; column < 6; column++) {
            HuSprAttrSet(entry0->groupId,
                          4 + column * 2 + row * 12, 4);
            HuSprAttrSet(entry0->groupId,
                          5 + column * 2 + row * 12, 4);
        }
    }
    entry2 = &lbl_1_bss_224.entries[2];
    HuSprAttrSet(entry2->groupId, 2, 4);
    HuSprAttrSet(entry2->groupId, 3, 4);
    for (secondCell = 0; secondCell < 6; secondCell++) {
        HuSprAttrSet(entry2->groupId, (s16)(secondCell + 4), 4);
    }
    entry3 = &lbl_1_bss_224.entries[3];
    HuSprAttrSet(entry3->groupId, 2, 4);
    HuSprAttrSet(entry3->groupId, 3, 4);
    for (thirdCell = 0; thirdCell < 8; thirdCell++) {
        HuSprAttrSet(entry3->groupId, (s16)(thirdCell + 4), 4);
    }
    for (entryIndex = 0; entryIndex < 4; entryIndex++) {
        fn_1_ADFC((s16)entryIndex);
        HuSprAttrSet(lbl_1_bss_224.entries[entryIndex].groupId, 2, 4);
    }
}

/* Installs the assigned player's character icon and shows the activity's first two group slots. */
void fn_1_AC40(s16 entryIndex) {
    s16 firstEntryGroupId;
    s16 secondEntryGroupId;
    s16 thirdEntryGroupId;
    s16 fourthEntryGroupId;
    M647ActivityEntry *entry;
    entry = &lbl_1_bss_224.entries[entryIndex];
    switch (entryIndex)
    {
    case 0 :
        firstEntryGroupId = lbl_1_bss_224.entries[0].groupId;
        HuSprGrpMemberSet(firstEntryGroupId, 1, HuSprCreate(lbl_1_bss_1AC[fn_1_34A0(0)], 10, 0));
        break;
    case 1 :
        secondEntryGroupId = lbl_1_bss_224.entries[1].groupId;
        HuSprGrpMemberSet(secondEntryGroupId, 1, HuSprCreate(lbl_1_bss_1AC[fn_1_34A0(1)], 10, 0));
        break;
    case 2 :
        thirdEntryGroupId = lbl_1_bss_224.entries[2].groupId;
        HuSprGrpMemberSet(thirdEntryGroupId, 1, HuSprCreate(lbl_1_bss_1AC[fn_1_34A0(2)], 10, 0));
        break;
    case 3 :
        fourthEntryGroupId = lbl_1_bss_224.entries[3].groupId;
        HuSprGrpMemberSet(fourthEntryGroupId, 1, HuSprCreate(lbl_1_bss_1AC[fn_1_34A0(3)], 10, 0));
        break;
    }
    HuSprAttrReset(entry->groupId, 0, 4);
    HuSprAttrReset(entry->groupId, 1, 4);
}

/* Hides the activity's first two group members, including its player character icon. */
void fn_1_ADFC(s16 entryIndex) {
    M647ActivityEntry *entry;

    entry = &lbl_1_bss_224.entries[entryIndex];
    HuSprAttrSet(entry->groupId, 0, 4);
    HuSprAttrSet(entry->groupId, 1, 4);
}

/* Selects whether shared stage models render in the activity camera. */
void fn_1_AE58(s16 mode) {
    s32 index;

    if (mode == 1) {
        Hu3DModelCameraSet(lbl_1_bss_174.shadowModelId, 1);
        for (index = 0; index < 4; index++) {
            Hu3DModelCameraSet(lbl_1_bss_174.modelIds[index], 1);
        }
    } else {
        Hu3DModelCameraSet(lbl_1_bss_174.shadowModelId, 0);
        for (index = 0; index < 4; index++) {
            Hu3DModelCameraSet(lbl_1_bss_174.modelIds[index], 0);
        }
    }
}

/* Selects whether player activity models render in the activity cameras. */
void fn_1_AF18(s16 cameraMode) {
    s32 entryIndex;

    if (cameraMode == 1) {
        for (entryIndex = 0; entryIndex < 4; entryIndex++) {
            Hu3DModelCameraSet(lbl_1_bss_224.entries[entryIndex].modelId, 3);
        }
        return;
    }
    for (entryIndex = 0; entryIndex < 4; entryIndex++) {
        Hu3DModelCameraSet(lbl_1_bss_224.entries[entryIndex].modelId, 0);
    }
}

M647ActivityTable lbl_1_bss_224;
ANIMDATA *lbl_1_bss_214[4];
ANIMDATA *lbl_1_bss_208[3];
ANIMDATA *lbl_1_bss_1F0[6];
ANIMDATA *lbl_1_bss_1D8[6];
ANIMDATA *lbl_1_bss_1C0[6];
ANIMDATA *lbl_1_bss_1BC;
ANIMDATA *lbl_1_bss_1AC[4];
ANIMDATA *lbl_1_bss_1A8;
