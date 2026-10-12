// Runs the Miracle Book menu, including its page turns and reward effects.
#include "dolphin/math.h"

#include "dolphin.h"

#include "messdir_enum.h"

#include "msm_se.h"

#include "msm_stream.h"

#include "REL/miraclebookdll/include/game/audio.h"

#include "game/data.h"

#include "game/esprite.h"

#include "game/gamework.h"

#include "game/hu3d.h"

#include "game/hsfex.h"

#include "game/mgdata.h"

#include "game/memory.h"

#include "game/object.h"

#include "game/pad.h"

#include "game/process.h"

#include "game/window.h"

#include "game/wipe.h"

#define MSM_STREAM_MBOOK 94
#define MSM_SE_MBOOK_OPEN 1408
#define MSM_SE_MBOOK_PAGE_TURN 1409
#define MSM_SE_MBOOK_REVERSE_PAGE_TURN 1410
#define MSM_SE_MBOOK_PAGE_SCROLL_START 1411
#define MSM_SE_MBOOK_PAGE_SCROLL_REPEAT 1412
#define MIRACLEBOOK_ENTRY_EMPTY 255 /* 0xFF marks an empty entry-state slot. */
#define MIRACLEBOOK_BOOK_MOTION_END_FRAME 139
#define MIRACLEBOOK_PAGE_TURN_LAST_FRAME 119
#define MIRACLEBOOK_PAGE_SCROLL_REPEAT_LIMIT 59
#define MIRACLEBOOK_PAGE_TURN_COVER_SWAP_FRAME 109
#define MIRACLEBOOK_PAGE_TURN_STACK_LAYER_FRAME 90
#define MIRACLEBOOK_PAGE_TURN_SPREAD_LAYER_FRAME 70
#define MIRACLEBOOK_PAGE_TURN_DETAIL_LAYER_FRAME 47
#define MIRACLEBOOK_PAGE_TURN_MULTI_ENTRY_LAYER_FRAME 87
#define MIRACLEBOOK_ENTRY_GROUP_2_START_INDEX 5
#define MIRACLEBOOK_ENTRY_GROUP_3_START_INDEX 15

typedef void (*VoidFunc)(void);

typedef struct UnkMiracleBookHook_s {
    f32 textureMatrix00;
    f32 textureMatrix01;
    f32 textureMatrix02;
    f32 textureMatrix10;
    f32 textureMatrix11;
    f32 textureMatrix12;
    GXColor color;
    f32 textureOffsetY;
    f32 textureOffsetSpeed;
    f32 effectFramesRemaining;
} UNK_MIRACLEBOOK_HOOK;

typedef struct MiracleBookVecTable_s {
    HuVecF entries[5];
} MIRACLEBOOK_VEC_TABLE;

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

s32 lbl_1_data_0[20] = {
    4, 4, 7, 1, 5, 1, 4, 1, 2, 4,
    1, 3, 3, 5, 5, 3, 1, 5, 6, 7,
};

s32 lbl_1_data_50[20][7] = {
    {1, 2, 1, 2, 0, 0, 0},
    {1, 1, 2, 2, 0, 0, 0},
    {1, 1, 2, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 0, 0},
    {1, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 1, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0},
    {1, 1, 0, 0, 0, 0, 0},
    {1, 1, 2, 1, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0},
    {1, 2, 2, 0, 0, 0, 0},
    {1, 2, 1, 0, 0, 0, 0},
    {1, 2, 2, 1, 1, 0, 0},
    {1, 1, 2, 2, 1, 0, 0},
    {1, 1, 2, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0},
    {1, 1, 2, 2, 1, 0, 0},
    {1, 1, 2, 2, 1, 1, 0},
    {1, 2, 2, 2, 1, 1, 1},
};

extern s16 lbl_1_data_280[][7];

extern HU3D_PARMAN_PARAM lbl_1_data_5C8;

extern u32 lbl_1_data_750[];

extern char lbl_1_data_758[];

extern char lbl_1_data_778[];

extern s32 lbl_1_bss_0;

extern OMOBJMAN *lbl_1_bss_4;

extern OMOBJ *lbl_1_bss_8;

extern void *lbl_1_bss_C;

extern ANIMDATA *lbl_1_bss_10;

extern ANIMDATA *lbl_1_bss_14;

extern UNK_MIRACLEBOOK_HOOK lbl_1_bss_18[2];

extern s16 lbl_1_bss_68[5];

extern OMOBJMAN *lbl_1_bss_74;

extern HuVecF lbl_1_bss_78;

extern f32 lbl_1_bss_84;

extern f32 lbl_1_bss_88;

extern s32 lbl_1_bss_8C;

extern s16 lbl_1_bss_90[3];

extern s32 lbl_1_bss_98[2];

extern s32 lbl_1_bss_A0;

extern s32 lbl_1_bss_A4[2];

extern s32 lbl_1_bss_AC;

extern s32 lbl_1_bss_B0;

extern s32 lbl_1_bss_B4[22];

extern s32 lbl_1_bss_10C;

extern s32 lbl_1_bss_110;

extern s32 lbl_1_bss_114;

extern f32 lbl_1_bss_118;

extern f32 lbl_1_bss_11C;

extern f32 lbl_1_bss_120[4];

extern s32 lbl_1_bss_130;

extern s32 lbl_1_bss_134;

extern s32 lbl_1_bss_138;

extern s32 lbl_1_bss_13C;

extern s32 lbl_1_bss_140;

extern s16 lbl_1_bss_144[20];

extern s16 lbl_1_bss_16C[2][2];

extern s16 lbl_1_bss_174[2];

extern s16 lbl_1_bss_178;

extern s16 lbl_1_bss_17A;

extern s32 lbl_1_bss_17C;

extern s32 lbl_1_bss_180;

extern f32 lbl_1_bss_184[6];

extern f32 lbl_1_bss_19C;

extern s32 lbl_1_bss_1A0;

extern OM_CAMERA_VIEW lbl_1_bss_1A4;

extern s16 lbl_1_bss_1C0[2];

extern s32 lbl_1_bss_1D4;

extern s32 lbl_1_bss_1D8;

extern s32 lbl_1_bss_1C4;

extern f32 lbl_1_bss_1C8[3];

extern s32 lbl_1_bss_1DC;

extern s32 lbl_1_bss_1E0;

extern s32 lbl_1_bss_1E4;

extern u8 lbl_1_bss_1E8[];

extern s16 lbl_1_bss_1FC[2][7];

extern s16 lbl_1_bss_218;

extern s16 lbl_1_bss_21A;

extern s16 lbl_1_bss_21C;

extern s32 lbl_1_bss_220;

extern s32 lbl_1_bss_224;

extern u32 lbl_1_data_6F4[];

extern u32 lbl_1_data_748[];

extern s32 lbl_1_data_398[];

extern f32 lbl_1_data_3E8[];

void fn_1_A0(void);

void fn_1_1CC(void);

void fn_1_2AC(OMOBJMAN *objMan);

s32 fn_1_2FCC(s32 dir);

void fn_1_3154(s32 dir);

void fn_1_3734(s32 cameraMoveDirection);

s32 fn_1_394(void);

void fn_1_434(void);

void fn_1_4CC(void);

void fn_1_A30(void);

void fn_1_3418(void);

void fn_1_34C4(void);

void fn_1_3E40(void);

s32 fn_1_427C(void);

void fn_1_479C(void);

void fn_1_4E94(void);

void fn_1_4EFC(void);

void fn_1_4FD4(void);

void fn_1_5904(void);

void fn_1_5ED0(s32 displaySprites);

void fn_1_60A4(void);

void fn_1_69A8(void);

void fn_1_69F8(void);

void fn_1_6B8C(void);

void fn_1_7158(void);

void fn_1_71E0(s32 messNo);

void fn_1_7324(void);

s32 fn_1_7354(void);

void fn_1_7560(void);

void fn_1_7998(void);

void fn_1_8A0C(s16 layerNo);

void fn_1_8A78(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);

void fn_1_8F10(OMOBJ *obj);

void fn_1_9130(s16 effectIndex, HuVecF *position);

void fn_1_92B4(OMOBJ *obj);

void fn_1_95B8(void);

void fn_1_9718(void);

s16 lbl_1_data_280[][7] = {
    1, 4, 5, 5, 0, 0, 0, 1, 4, 2, 3, 0, 0, 0,
    1, 7, 2, 5, 4, 3, 6, 1, 0, 0, 0, 0, 0, 0,
    1, 4, 3, 6, 5, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    1, 4, 2, 3, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    1, 5, 0, 0, 0, 0, 0, 1, 4, 2, 3, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 1, 4, 2, 0, 0, 0, 0,
    1, 4, 5, 0, 0, 0, 0, 1, 4, 2, 5, 3, 0, 0,
    2, 5, 4, 3, 1, 0, 0, 1, 4, 2, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 1, 4, 2, 3, 5, 0, 0,
    1, 4, 2, 3, 3, 3, 0, 1, 4, 2, 3, 5, 5, 5,
};

s32 lbl_1_data_398[20] = {
    14, 0, 2, 4, 11, 6, 9, 13, 3, 10,
    16, 15, 18, 8, 1, 7, 19, 17, 12, 5,
};

f32 lbl_1_data_3E8[120] = {
    0.0, 0.01889999955892563, 0.07509999722242355, 0.1678999960422516,
    0.2969000041484833, 0.46129998564720154, 0.6606000065803528, 0.8939999938011169,
    1.1611000299453735, 1.4610999822616577, 1.7934999465942383, 2.1577000617980957,
    2.5529000759124756, 2.9786999225616455, 3.434299945831299, 3.9191999435424805,
    4.432700157165527, 4.974299907684326, 5.5432000160217285, 6.138899803161621,
    6.760799884796143, 7.408199787139893, 8.080599784851074, 8.777199745178223,
    9.497599601745605, 10.241000175476074, 11.006799697875977, 11.794400215148926,
    12.603300094604492, 13.42959976196289, 14.275300025939941, 15.139599800109863,
    16.0221004486084, 16.922000885009766, 17.83880043029785, 18.771900177001953,
    19.720699310302734, 20.684600830078125, 21.662900924682617, 22.65519905090332,
    23.66069984436035, 24.67889976501465, 25.709199905395508, 26.750999450683594,
    27.803600311279297, 28.866600036621094, 29.939199447631836, 31.02079963684082,
    32.111000061035156, 33.20899963378906, 34.314300537109375, 35.422698974609375,
    36.5364990234375, 37.65489959716797, 38.77750015258789, 39.90340042114258,
    41.03219985961914, 42.16320037841797, 43.29570007324219, 44.429100036621094,
    45.56290054321289, 46.69620132446289, 47.8286018371582, 48.95940017700195,
    50.0880012512207, 51.21369934082031, 52.33599853515625, 53.45399856567383,
    54.56740188598633, 55.67530059814453, 56.777198791503906, 57.872501373291016,
    58.96049880981445, 60.04059982299805, 61.11220169067383, 62.174598693847656,
    63.22719955444336, 64.26930236816406, 65.30039978027344, 66.3198013305664,
    67.32689666748047, 68.32099914550781, 69.30159759521484, 70.26789855957031,
    71.21939849853516, 72.15540313720703, 73.07530212402344, 73.9761962890625,
    74.85980224609375, 75.72530364990234, 76.57209777832031, 77.39949798583984,
    78.20670318603516, 78.99320220947266, 79.75830078125, 80.5011978149414,
    81.22139739990234, 81.91799926757812, 82.59049987792969, 83.23819732666016,
    83.86039733886719, 84.456298828125, 85.02539825439453, 85.56700134277344,
    86.08039855957031, 86.5647964477539, 87.01969909667969, 87.44439697265625,
    87.83809661865234, 88.20020294189453, 88.53389739990234, 88.83599853515625,
    89.10559844970703, 89.34200286865234, 89.54440307617188, 89.71199798583984,
    89.84400177001953, 89.9395980834961, 89.99800109863281, 90.0,
};

HU3D_PARMAN_PARAM lbl_1_data_5C8 = {
    50, 0, 1.0f, 10.0f, 0.0f, {0.0f, -0.02f, 0.0f},
    2.0f, 1.0f, 20.0f, 0.98f, 2,
    {{240, 224, 32, 255}, {255, 255, 64, 255}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{192, 128, 16, 0}, {192, 64, 16, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
};

// Called when the Miracle Book overlay starts to load its menu and display objects.
void fn_1_2AC(OMOBJMAN *objectManager)
{
    lbl_1_bss_74 = objectManager;
    fn_1_34C4();
    HuWinInit(1);
    fn_1_4CC();
    fn_1_479C();
    fn_1_69A8();
    fn_1_7560();
    HuPrcChildCreate(fn_1_60A4, 100, 8192, 0, HuPrcCurrentGet());
    if (MiracleBookEvtNo != 0) {
        lbl_1_bss_224 = 1;
    }
    lbl_1_bss_AC = -1;
    lbl_1_bss_A4[0] = lbl_1_bss_A4[1] = -1;
    lbl_1_bss_A0 = 0;
    lbl_1_bss_98[0] = lbl_1_bss_98[1] = 0;
}

// Called by the overlay update to advance the book, then report when it can exit.
s32 fn_1_394(void)
{
    s32 result = 0;

    if (lbl_1_bss_AC == -1) {
        lbl_1_bss_AC = HuAudBGMPlay(MSM_STREAM_MBOOK);
    }
    fn_1_A30();
    fn_1_4E94();
    if (fn_1_7354() == 0) {
        result = 1;
    }
    if (lbl_1_bss_17C != 0) {
        result = 1;
    }
    if (result == 1) {
        HuAudSStreamFadeOut(lbl_1_bss_AC, 100);
    }
    return result;
}

// Called during overlay teardown to return to the event that opened the book.
void fn_1_434(void)
{
    s32 overlayId;

    if (lbl_1_bss_134 > 0) {
        omOvlHisChg(0, 108, 0, 0);
        if (lbl_1_bss_134 == 1) {
            overlayId = 110;
        } else {
            overlayId = 2;
        }
        omOvlCallEx(overlayId, 1, 0, 0);
        MiracleBookEvtNo = lbl_1_bss_134;
    }
}

// Called during overlay setup to create the book models and load saved entry states.
void fn_1_4CC(void)
{
    s32 i;
    s32 isMarked;
    s32 entryState;
    s32 isEntryMarked;

    lbl_1_bss_21C = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 0), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_21C, 1);
    Hu3DModelPosSet(lbl_1_bss_21C, (0.0f),
        (f32)((2000.0) * sin((-0.4363323129985824))), (-2000.0f));
    Hu3DModelRotSet(lbl_1_bss_21C, (-25.0f),
        (0.0f), (0.0f));
    Hu3DModelLayerSet(lbl_1_bss_21C, 0);
    Hu3DModelAttrSet(lbl_1_bss_21C, HU3D_MOTATTR_LOOP);
    Hu3DMotionSpeedSet(lbl_1_bss_21C, (0.04f));
    lbl_1_bss_21A = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 1), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_21A, 1);
    Hu3DModelPosSet(lbl_1_bss_21A, (100.0f),
        (-229.0f), (-500.0f));
    Hu3DModelRotSet(lbl_1_bss_21A, (125.0f),
        (-130.0f), (-45.0f));
    Hu3DModelLayerSet(lbl_1_bss_21A, 0);
    Hu3DModelAttrSet(lbl_1_bss_21A, HU3D_MOTATTR_PAUSE);
    Hu3DModelClusterAttrSet(lbl_1_bss_21A, 0, HU3D_CLUSTER_ATTR_PAUSE);
    lbl_1_bss_218 = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 2), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_218, 1);
    Hu3DModelPosSet(lbl_1_bss_218, (100.0f),
        (-229.0f), (-500.0f));
    Hu3DModelRotSet(lbl_1_bss_218, (125.0f),
        (-130.0f), (-45.0f));
    Hu3DModelLayerSet(lbl_1_bss_218, 5);
    Hu3DModelAttrSet(lbl_1_bss_218, HU3D_MOTATTR_PAUSE);
    for (i = 0; i < 2; i++) {
        lbl_1_bss_1C0[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATA_miraclebook + i + 6, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_1C0[i], 1);
        Hu3DModelPosSet(lbl_1_bss_1C0[i], (0.0f),
            (-65.0f), (0.0f));
        Hu3DModelRotSet(lbl_1_bss_1C0[i], (0.0f),
            (0.0f), (0.0f));
        Hu3DModelLayerSet(lbl_1_bss_1C0[i], 0);
        Hu3DModelAttrSet(lbl_1_bss_1C0[i], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < 7; i++) {
        lbl_1_bss_1FC[0][i] = -1;
        lbl_1_bss_1FC[1][i] = -1;
    }
    lbl_1_bss_1D8 = 0;
    lbl_1_bss_1D4 = 19;
    lbl_1_bss_224 = 0;
    lbl_1_bss_220 = -1;
    lbl_1_bss_1E0 = 0;
    if (GWMiracleBookFlagGet(0)) {
        isMarked = 1;
    } else {
        isMarked = 0;
    }
    lbl_1_bss_1E8[0] = isMarked;
    lbl_1_bss_130 = lbl_1_bss_1E8[0];
    for (i = 1; i < 20; i++) {
        if (!GWBankFlagGet(i + 10)) {
            entryState = MIRACLEBOOK_ENTRY_EMPTY;
        } else {
            if (GWMiracleBookFlagGet(i)) {
                isEntryMarked = 1;
            } else {
                isEntryMarked = 0;
            }
            entryState = isEntryMarked;
        }
        lbl_1_bss_1E8[i] = entryState;
        if (lbl_1_bss_1E8[i] == 1) {
            lbl_1_bss_130++;
        }
    }
}

// Called once per overlay update to handle the opening, page-turn and reward states.
void fn_1_A30(void)
{
    f32 motionTime;
    f32 rotationMultiplier;
    s32 soundId;
    s32 rotationDirection;
    s32 i;

    switch (lbl_1_bss_224) {
    case 0:
        fn_1_69F8();
        lbl_1_bss_224 = 1;
        return;
    case 1:
        if (lbl_1_bss_19C == 0.0f) {
            HuAudFXPlay(MSM_SE_MBOOK_OPEN);
        }
        Hu3DModelAttrReset(lbl_1_bss_21A, HU3D_MOTATTR_PAUSE);
        Hu3DModelClusterAttrReset(lbl_1_bss_21A, 0, HU3D_CLUSTER_ATTR_PAUSE);
        Hu3DModelAttrReset(lbl_1_bss_218, HU3D_MOTATTR_PAUSE);
        fn_1_3E40();
        if (Hu3DMotionEndCheck(lbl_1_bss_218) != 0) {
            lbl_1_bss_224 = 2;
            fn_1_71E0(0);
            fn_1_6B8C();
            lbl_1_bss_19C = 0.0f;
            lbl_1_bss_11C = 0.0f;
            lbl_1_bss_120[2] = lbl_1_bss_120[3] = 0.0f;
            if (MiracleBookEvtNo != 0) {
                if (MiracleBookEvtNo == 1) {
                    espDispOff(lbl_1_bss_178);
                    Hu3DModelPosSet(lbl_1_bss_90[2], 0.0f, 86.0f, 5.0f);
                    Hu3DModelAttrReset(lbl_1_bss_90[2], 1U);
                    lbl_1_bss_180 = 0;
                } else if (MiracleBookEvtNo == 2) {
                    espDispOff(lbl_1_bss_178);
                    Hu3DModelPosSet(lbl_1_bss_90[2], 0.0f, -54.0f, 60.0f);
                    Hu3DModelAttrReset(lbl_1_bss_90[2], 1U);
                    lbl_1_bss_180 = 2;
                }
                lbl_1_bss_134 = MiracleBookEvtNo * -1;
                lbl_1_bss_138 = -1;
                MiracleBookEvtNo = 0;
                return;
            }
        }
        break;
    case 2:
        if (lbl_1_bss_1A0 == 0) {
            if ((lbl_1_bss_13C == 0) && ((HuPadBtnDown[0] & 0x100) != 0)) {
                if (lbl_1_bss_134 != 0) {
                    lbl_1_bss_134 *= -1;
                    lbl_1_bss_224 = 9;
                    lbl_1_bss_19C = 0.0f;
                } else {
                    if (lbl_1_bss_1E8[lbl_1_bss_1E4] == MIRACLEBOOK_ENTRY_EMPTY) {
                        HuAudFXPlay(MSM_SE_CMN_05);
                        return;
                    }
                    fn_1_7324();
                    fn_1_3734(1);
                }
                HuAudFXPlay(MSM_SE_CMN_03);
                return;
            }
        } else {
            lbl_1_bss_1A0 = 0;
            if (lbl_1_bss_1E8[lbl_1_bss_1E4] == 0) {
                lbl_1_bss_1E8[lbl_1_bss_1E4] = 1;
                GWMiracleBookFlagSet(lbl_1_bss_1E4);
                lbl_1_bss_130 += 1;
            }
            fn_1_3154(0);
            Hu3DModelAttrSet(lbl_1_bss_218, HU3D_MOTATTR_REV);
            for (i = 0; i < lbl_1_data_0[lbl_1_bss_1E4]; i++) {
                if (lbl_1_data_50[lbl_1_bss_1E4][i] == 1) {
                    Hu3DMotionStartEndSet(lbl_1_bss_1FC[0][i], 0.0f,
                        (f32)MIRACLEBOOK_BOOK_MOTION_END_FRAME);
                    Hu3DModelAttrReset(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_PAUSE);
                } else if (lbl_1_data_50[lbl_1_bss_1E4][i] == 2) {
                    Hu3DMotionShapeStartEndSet(lbl_1_bss_1FC[0][i], 0.0f,
                                               (f32) MIRACLEBOOK_BOOK_MOTION_END_FRAME);
                    Hu3DModelAttrReset(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_SHAPE_PAUSE);
                }
                Hu3DModelAttrReset(lbl_1_bss_1C0[0], 1U);
            }
            lbl_1_bss_224 = 3;
            lbl_1_bss_1E0 = 0;
            lbl_1_bss_1DC = 0;
            return;
        }
        break;
    case 3:
        if (lbl_1_bss_1E0 == 0) {
            HuAudFXPlay(MSM_SE_MBOOK_PAGE_TURN);
        }
        if (++lbl_1_bss_1E0 <= MIRACLEBOOK_PAGE_TURN_LAST_FRAME) {
                    Hu3DModelRotSet(lbl_1_bss_218, 0.0f, 0.0f, lbl_1_data_3E8[lbl_1_bss_1E0]);
            if (lbl_1_bss_1E0 == MIRACLEBOOK_PAGE_TURN_COVER_SWAP_FRAME) {
                Hu3DModelAttrSet(lbl_1_bss_218, 1U);
                Hu3DModelAttrReset(lbl_1_bss_1C0[1], 1U);
            }
            for (i = 0; i < lbl_1_data_0[lbl_1_bss_1E4]; i++) {
                Hu3DModelRotSet(lbl_1_bss_1FC[0][i], 0.0f, 0.0f,
                                -90.0f + lbl_1_data_3E8[lbl_1_bss_1E0]);
                if (lbl_1_bss_1E0 == 10) {
                    Hu3DModelAttrReset(lbl_1_bss_1FC[0][i], 1U);
                    Hu3DModelAttrSet(lbl_1_bss_1C0[0], 1U);
                }
                switch (lbl_1_data_398[lbl_1_bss_1E4]) {
                case 3:
                    if (i == 1) {
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], 1U);
                    }
                    break;
                case 5:
                    if (i > 3) {
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], 1U);
                    }
                    break;
                case 8:
                case 11:
                    if (i == 3) {
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], 1U);
                    }
                    break;
                case 12:
                    if (i > 3) {
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], 1U);
                    }
                    break;
                case 17:
                    if (i == 4) {
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], 1U);
                    }
                    break;
                }
            }
            if (lbl_1_data_398[lbl_1_bss_1E4] == 8) {
                if (lbl_1_bss_1E0 < 80) {
                    Hu3DModelLayerSet(lbl_1_bss_1FC[0][1], 3);
                    Hu3DModelLayerSet(lbl_1_bss_1FC[0][4], 4);
                } else {
                    Hu3DModelLayerSet(lbl_1_bss_1FC[0][1], 4);
                    Hu3DModelLayerSet(lbl_1_bss_1FC[0][4], 3);
                }
            }
        }
        if (Hu3DMotionEndCheck(lbl_1_bss_1FC[0][0]) != 0) {
            lbl_1_bss_224 = 4;
            fn_1_71E0(1);
            Hu3DModelAttrSet(lbl_1_bss_1C0[1], 1U);
            return;
        }
        break;
    case 4:
        if (lbl_1_bss_B0 != 0) {
            if ((HuPadBtn[0] & 0x100) == 0) {
                lbl_1_bss_B0 = 0;
                return;
            }
        } else {
            if (lbl_1_bss_1E8[lbl_1_bss_1E4] == 0) {
                lbl_1_bss_1E8[lbl_1_bss_1E4] = 1;
                GWMiracleBookFlagSet(lbl_1_bss_1E4);
                lbl_1_bss_130 += 1;
            }
            if ((s32) (HuPadBtn[0] & 0x100) != 0) {
                if (lbl_1_bss_A0 == 0) {
                    lbl_1_bss_A4[0] = HuAudFXPlay(MSM_SE_MBOOK_PAGE_SCROLL_START);
                    if (lbl_1_bss_A4[1] != -1) {
                        HuAudFXStop(lbl_1_bss_A4[1]);
                        lbl_1_bss_A4[1] = -1;
                    }
                    lbl_1_bss_A0 = -1;
                }
                if (++lbl_1_bss_98[0] >= 25) {
                    lbl_1_bss_A0 = 1;
                }
                if (++lbl_1_bss_98[1] > MIRACLEBOOK_PAGE_SCROLL_REPEAT_LIMIT) {
                    lbl_1_bss_98[1] = MIRACLEBOOK_PAGE_SCROLL_REPEAT_LIMIT;
                }
                if (lbl_1_bss_1DC <= 0) {
                    for (i = 0; i < lbl_1_data_0[lbl_1_bss_1E4]; i++) {
                        if (lbl_1_data_50[lbl_1_bss_1E4][i] == 1) {
                            Hu3DMotionStartEndSet(lbl_1_bss_1FC[0][i], 149.0f, 209.0f);
                            Hu3DModelAttrReset(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_REV);
                            Hu3DMotionTimeSet(lbl_1_bss_1FC[0][i], 149.0f);
                        } else if (lbl_1_data_50[lbl_1_bss_1E4][i] == 2) {
                            Hu3DMotionShapeStartEndSet(lbl_1_bss_1FC[0][i], 149.0f, 209.0f);
                            Hu3DModelAttrReset(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_SHAPE_REV);
                            Hu3DMotionShapeTimeSet(lbl_1_bss_1FC[0][i], 149.0f);
                        }
                    }
                    lbl_1_bss_1DC = 1;
                    fn_1_7324();
                    switch (lbl_1_data_398[lbl_1_bss_1E4]) {
                    case 3:
                        Hu3DModelAttrReset(lbl_1_bss_1FC[0][1], 1U);
                        break;
                    case 5:
                        lbl_1_bss_1C8[0] = Hu3DMotionMaxTimeGet(lbl_1_bss_1FC[0][4]);
                        Hu3DMotionStartEndSet(lbl_1_bss_1FC[0][4], 0.0f, lbl_1_bss_1C8[0]);
                        Hu3DModelAttrReset(lbl_1_bss_1FC[0][4], 1U);
                        Hu3DMotionTimeSet(lbl_1_bss_1FC[0][4], 0.0f);
                        lbl_1_bss_1C8[1] = Hu3DMotionMaxTimeGet(lbl_1_bss_1FC[0][5]);
                        lbl_1_bss_1C8[2] = Hu3DMotionMaxTimeGet(lbl_1_bss_1FC[0][6]);
                        Hu3DMotionStartEndSet(lbl_1_bss_1FC[0][6], 0.0f, lbl_1_bss_1C8[2]);
                        Hu3DModelAttrReset(lbl_1_bss_1FC[0][6], 1U);
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][6], HU3D_MOTATTR_LOOP);
                        Hu3DMotionTimeSet(lbl_1_bss_1FC[0][6], 0.0f);
                        break;
                    case 8:
                    case 11:
                        Hu3DModelAttrReset(lbl_1_bss_1FC[0][3], 1U);
                        break;
                    case 12:
                        lbl_1_bss_1C8[0] = Hu3DMotionMaxTimeGet(lbl_1_bss_1FC[0][4]);
                        Hu3DMotionStartEndSet(lbl_1_bss_1FC[0][4], 0.0f, lbl_1_bss_1C8[0]);
                        Hu3DModelAttrReset(lbl_1_bss_1FC[0][4], 1U);
                        Hu3DMotionTimeSet(lbl_1_bss_1FC[0][4], 0.0f);
                        lbl_1_bss_1C8[1] = Hu3DMotionMaxTimeGet(lbl_1_bss_1FC[0][5]);
                        break;
                    case 17:
                        Hu3DModelAttrReset(lbl_1_bss_1FC[0][4], 1U);
                        break;
                    }
                }
                switch (lbl_1_data_398[lbl_1_bss_1E4]) {
                case 5:
                    if (((s32) lbl_1_bss_1C8[0] != 0) &&
                        (Hu3DMotionEndCheck(lbl_1_bss_1FC[0][4]) != 0)) {
                        lbl_1_bss_1C8[0] = 0.0f;
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][4], 1U);
                        Hu3DMotionStartEndSet(lbl_1_bss_1FC[0][5], 0.0f, lbl_1_bss_1C8[1]);
                        Hu3DModelAttrReset(lbl_1_bss_1FC[0][5], 1U);
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][5], HU3D_MOTATTR_LOOP);
                        Hu3DMotionTimeSet(lbl_1_bss_1FC[0][5], 0.0f);
                        return;
                    }
                    break;
                case 12:
                    if (((s32) lbl_1_bss_1C8[0] != 0) &&
                        (Hu3DMotionEndCheck(lbl_1_bss_1FC[0][4]) != 0)) {
                        lbl_1_bss_1C8[0] = 0.0f;
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][4], 1U);
                        Hu3DMotionStartEndSet(lbl_1_bss_1FC[0][5], 0.0f, lbl_1_bss_1C8[1]);
                        Hu3DModelAttrReset(lbl_1_bss_1FC[0][5], 1U);
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][5], HU3D_MOTATTR_LOOP);
                        Hu3DMotionTimeSet(lbl_1_bss_1FC[0][5], 0.0f);
                        return;
                    }
                    break;
                }
            } else {
                if (lbl_1_bss_A0 != 0) {
                    if ((lbl_1_bss_A0 > 0) ||
                        (lbl_1_bss_98[1] == MIRACLEBOOK_PAGE_SCROLL_REPEAT_LIMIT)) {
                        lbl_1_bss_A4[1] = HuAudFXPlay(MSM_SE_MBOOK_PAGE_SCROLL_REPEAT);
                        if (lbl_1_bss_A4[0] != -1) {
                            HuAudFXStop(lbl_1_bss_A4[0]);
                            lbl_1_bss_A4[0] = -1;
                        }
                    }
                    lbl_1_bss_A0 = 0;
                    lbl_1_bss_98[0] = 0;
                }
                if (--lbl_1_bss_98[1] < 0) {
                    lbl_1_bss_98[1] = 0;
                }
                if (lbl_1_bss_1DC == 1) {
                    for (i = 0; i < lbl_1_data_0[lbl_1_bss_1E4]; i++) {
                        if (lbl_1_data_50[lbl_1_bss_1E4][i] == 1) {
                            motionTime = Hu3DMotionTimeGet(lbl_1_bss_1FC[0][i]);
                            Hu3DMotionStartEndSet(lbl_1_bss_1FC[0][i], 149.0f, motionTime);
                            Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_REV);
                            Hu3DMotionTimeSet(lbl_1_bss_1FC[0][i], motionTime);
                        } else if (lbl_1_data_50[lbl_1_bss_1E4][i] == 2) {
                            motionTime = Hu3DMotionShapeTimeGet(lbl_1_bss_1FC[0][i]);
                            Hu3DMotionShapeStartEndSet(lbl_1_bss_1FC[0][i], 149.0f, motionTime);
                            Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_SHAPE_REV);
                            Hu3DMotionShapeTimeSet(lbl_1_bss_1FC[0][i], motionTime);
                        }
                    }
                    lbl_1_bss_1DC = -1;
                    switch (lbl_1_data_398[lbl_1_bss_1E4]) {
                    case 3:
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][1], 1U);
                        return;
                    case 5:
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][4], 1U);
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][5], 1U);
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][6], 1U);
                        return;
                    case 8:
                    case 11:
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][3], 1U);
                        return;
                    case 12:
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][4], 1U);
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][5], 1U);
                        return;
                    case 17:
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][4], 1U);
                        return;
                    }
                } else if (lbl_1_bss_1DC == -1) {
                    if (Hu3DMotionEndCheck(lbl_1_bss_1FC[0][0]) != 0) {
                        lbl_1_bss_1DC = 0;
                        fn_1_71E0(1);
                        return;
                    }
                } else {
                    if ((HuPadBtnDown[0] & 0x200) != 0) {
                        for (i = 0; i < lbl_1_data_0[lbl_1_bss_1E4]; i++) {
                            if (lbl_1_data_50[lbl_1_bss_1E4][i] == 1) {
                                Hu3DMotionStartEndSet(lbl_1_bss_1FC[0][i], 0.0f,
                                                      (f32) MIRACLEBOOK_BOOK_MOTION_END_FRAME);
                                Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_REV);
                                Hu3DMotionTimeSet(lbl_1_bss_1FC[0][i],
                                                  (f32) MIRACLEBOOK_BOOK_MOTION_END_FRAME);
                            } else if (lbl_1_data_50[lbl_1_bss_1E4][i] == 2) {
                                Hu3DMotionShapeStartEndSet(lbl_1_bss_1FC[0][i], 0.0f,
                                                           (f32) MIRACLEBOOK_BOOK_MOTION_END_FRAME);
                                Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_SHAPE_REV);
                                Hu3DMotionShapeTimeSet(lbl_1_bss_1FC[0][i],
                                                       (f32) MIRACLEBOOK_BOOK_MOTION_END_FRAME);
                            }
                        }
                        lbl_1_bss_224 = 5;
                        lbl_1_bss_1E0 = MIRACLEBOOK_BOOK_MOTION_END_FRAME;
                        fn_1_7324();
            HuAudFXPlay(MSM_SE_FILESEL_12);
                        return;
                    }
                    if (lbl_1_bss_220 == -1) {
                        if ((HuPadBtnDown[0] & 0x20) != 0) {
                            if (lbl_1_bss_1E4 != lbl_1_bss_1D4) {
                                lbl_1_bss_1C4 = fn_1_2FCC(1);
                                if (lbl_1_bss_1C4 != 0) {
                                    lbl_1_bss_220 = 6;
                                    fn_1_7324();
                                    fn_1_3154(lbl_1_bss_1C4);
                                }
                            }
                        } else if (((HuPadBtnDown[0] & 0x40) != 0) &&
                                   (lbl_1_bss_1E4 != lbl_1_bss_1D8)) {
                            lbl_1_bss_1C4 = fn_1_2FCC(-1);
                            if (lbl_1_bss_1C4 != 0) {
                                lbl_1_bss_220 = 7;
                                fn_1_7324();
                                fn_1_3154(lbl_1_bss_1C4);
                            }
                        }
                    }
                    if (lbl_1_bss_220 != -1) {
                        for (i = 0; i < lbl_1_data_0[lbl_1_bss_1E4]; i++) {
                            if (lbl_1_data_50[lbl_1_bss_1E4][i] == 1) {
                                Hu3DMotionStartEndSet(lbl_1_bss_1FC[0][i], 0.0f,
                                                      (f32) MIRACLEBOOK_BOOK_MOTION_END_FRAME);
                                Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_REV);
                                Hu3DMotionTimeSet(lbl_1_bss_1FC[0][i],
                                                  (f32) MIRACLEBOOK_BOOK_MOTION_END_FRAME);
                            } else if (lbl_1_data_50[lbl_1_bss_1E4][i] == 2) {
                                Hu3DMotionShapeStartEndSet(lbl_1_bss_1FC[0][i], 0.0f,
                                                           (f32) MIRACLEBOOK_BOOK_MOTION_END_FRAME);
                                Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_SHAPE_REV);
                                Hu3DMotionShapeTimeSet(lbl_1_bss_1FC[0][i],
                                                       (f32) MIRACLEBOOK_BOOK_MOTION_END_FRAME);
                            }
                        }
                        for (i = 0; i < lbl_1_data_0[lbl_1_bss_1E4 + lbl_1_bss_1C4]; i++) {
                            if (lbl_1_data_50[lbl_1_bss_1E4 + lbl_1_bss_1C4][i] == 1) {
                                Hu3DMotionStartEndSet(lbl_1_bss_1FC[1][i], 0.0f,
                                                      (f32) MIRACLEBOOK_BOOK_MOTION_END_FRAME);
                                Hu3DMotionTimeSet(lbl_1_bss_1FC[1][i], 0.0f);
                            } else if (lbl_1_data_50[lbl_1_bss_1E4 + lbl_1_bss_1C4][i] == 2) {
                                Hu3DMotionShapeStartEndSet(lbl_1_bss_1FC[1][i], 0.0f,
                                                           (f32) MIRACLEBOOK_BOOK_MOTION_END_FRAME);
                                Hu3DMotionShapeTimeSet(lbl_1_bss_1FC[1][i], 0.0f);
                            }
                        }
                        lbl_1_bss_224 = lbl_1_bss_220;
                        lbl_1_bss_220 = -1;
                        lbl_1_bss_1E0 = MIRACLEBOOK_BOOK_MOTION_END_FRAME;
            HuAudFXPlay(MSM_SE_FILESEL_12);
                        return;
                    }
                }
            }
        }
        break;
    case 5:
    case 6:
    case 7:
        if (--lbl_1_bss_1E0 < 0) {
            lbl_1_bss_1E0 = 0;
        }
        if (lbl_1_bss_1E0 <= MIRACLEBOOK_PAGE_TURN_LAST_FRAME) {
            if (lbl_1_bss_1E0 == MIRACLEBOOK_PAGE_TURN_LAST_FRAME) {
                if (lbl_1_bss_224 == 5) {
                    soundId = MSM_SE_MBOOK_REVERSE_PAGE_TURN;
                } else {
                    soundId = MSM_SE_MBOOK_PAGE_TURN;
                }
                HuAudFXPlay(soundId);
            }
            if (lbl_1_bss_224 == 5) {
                if (lbl_1_bss_1E0 == MIRACLEBOOK_PAGE_TURN_LAST_FRAME) {
                    Hu3DModelAttrReset(lbl_1_bss_218, HU3D_MOTATTR_REV);
                    Hu3DModelAttrReset(lbl_1_bss_1C0[1], 1U);
                }
                if (lbl_1_bss_1E0 == MIRACLEBOOK_PAGE_TURN_COVER_SWAP_FRAME) {
                    Hu3DModelAttrReset(lbl_1_bss_218, 1U);
                    Hu3DModelAttrSet(lbl_1_bss_1C0[1], 1U);
                }
                Hu3DModelRotSet(lbl_1_bss_218, 0.0f, 0.0f, lbl_1_data_3E8[lbl_1_bss_1E0]);
            } else {
                for (i = 0; i < lbl_1_data_0[lbl_1_bss_1E4 + lbl_1_bss_1C4]; i++) {
                    if (lbl_1_bss_1E0 == MIRACLEBOOK_PAGE_TURN_LAST_FRAME) {
                        if (lbl_1_data_50[lbl_1_bss_1E4 + lbl_1_bss_1C4][i] == 1) {
                            Hu3DModelAttrReset(lbl_1_bss_1FC[1][i], HU3D_MOTATTR_PAUSE);
                        } else if (lbl_1_data_50[lbl_1_bss_1E4 + lbl_1_bss_1C4][i] == 2) {
                            Hu3DModelAttrReset(lbl_1_bss_1FC[1][i], HU3D_MOTATTR_SHAPE_PAUSE);
                        }
                        Hu3DModelAttrReset(lbl_1_bss_1C0[0], 1U);
                    }
                    if (lbl_1_bss_1E0 == MIRACLEBOOK_PAGE_TURN_COVER_SWAP_FRAME) {
                        Hu3DModelAttrReset(lbl_1_bss_1FC[1][i], 1U);
                        Hu3DModelAttrSet(lbl_1_bss_1C0[0], 1U);
                        switch (lbl_1_data_398[lbl_1_bss_1E4 + lbl_1_bss_1C4]) {
                        case 5:
                            if (i > 3) {
                                Hu3DModelAttrSet(lbl_1_bss_1FC[1][i], 1U);
                            }
                            break;
                        case 12:
                            if (i > 3) {
                                Hu3DModelAttrSet(lbl_1_bss_1FC[1][i], 1U);
                            }
                            break;
                        }
                    }
                    if (lbl_1_bss_224 == 6) {
                        Hu3DModelRotSet(lbl_1_bss_1FC[1][i], 0.0f, 0.0f,
                                        -lbl_1_data_3E8[lbl_1_bss_1E0]);
                    } else if (lbl_1_bss_224 == 7) {
                        Hu3DModelRotSet(lbl_1_bss_1FC[1][i], 0.0f, 0.0f,
                                        lbl_1_data_3E8[lbl_1_bss_1E0]);
                    }
                }
                if (lbl_1_data_398[lbl_1_bss_1E4 + lbl_1_bss_1C4] == 2) {
                    if (lbl_1_bss_1E0 <= 33) {
                        Hu3DModelLayerSet(lbl_1_bss_1FC[1][1], 7);
                    } else {
                        Hu3DModelLayerSet(lbl_1_bss_1FC[1][1], 2);
                    }
                    if (lbl_1_bss_1E0 < 45) {
                        Hu3DModelLayerSet(lbl_1_bss_1FC[1][3], 5);
                        Hu3DModelLayerSet(lbl_1_bss_1FC[1][4], 4);
                    } else {
                        Hu3DModelLayerSet(lbl_1_bss_1FC[1][3], 4);
                        Hu3DModelLayerSet(lbl_1_bss_1FC[1][4], 5);
                    }
                    if (lbl_1_bss_1E0 < 70) {
                        Hu3DModelLayerSet(lbl_1_bss_1FC[1][5], 6);
                    } else {
                        Hu3DModelLayerSet(lbl_1_bss_1FC[1][5], 3);
                    }
                }
                if (lbl_1_data_398[lbl_1_bss_1E4 + lbl_1_bss_1C4] == 8) {
                    if (lbl_1_bss_1E0 < 35) {
                        Hu3DModelLayerSet(lbl_1_bss_1FC[1][1], 4);
                        Hu3DModelLayerSet(lbl_1_bss_1FC[1][4], 3);
                    } else {
                        Hu3DModelLayerSet(lbl_1_bss_1FC[1][1], 3);
                        Hu3DModelLayerSet(lbl_1_bss_1FC[1][4], 4);
                    }
                }
            }
            if (lbl_1_bss_224 == 6) {
                rotationDirection = 1;
            } else {
                rotationDirection = -1;
            }
            rotationMultiplier = (f32) rotationDirection;
            for (i = 0; i < lbl_1_data_0[lbl_1_bss_1E4]; i++) {
                Hu3DModelRotSet(lbl_1_bss_1FC[0][i], 0.0f, 0.0f,
                                rotationMultiplier * (90.0f - lbl_1_data_3E8[lbl_1_bss_1E0]));
                if (lbl_1_bss_1E0 == 10) {
                    Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], 1U);
                    Hu3DModelAttrReset(lbl_1_bss_1C0[0], 1U);
                }
                if (lbl_1_bss_1E0 == 0) {
                    Hu3DModelAttrSet(lbl_1_bss_1C0[0], 1U);
                    if (lbl_1_data_50[lbl_1_bss_1E4][i] == 1) {
                        Hu3DModelAttrReset(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_REV);
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_PAUSE);
                    } else if (lbl_1_data_50[lbl_1_bss_1E4][i] == 2) {
                        Hu3DModelAttrReset(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_SHAPE_REV);
                        Hu3DModelAttrSet(lbl_1_bss_1FC[0][i], HU3D_MOTATTR_SHAPE_PAUSE);
                    }
                }
            }
            if (lbl_1_data_398[lbl_1_bss_1E4] == 2) {
                if (lbl_1_bss_1E0 == MIRACLEBOOK_PAGE_TURN_STACK_LAYER_FRAME) {
                    Hu3DModelLayerSet(lbl_1_bss_1FC[0][1], 2);
                }
                if (lbl_1_bss_1E0 == MIRACLEBOOK_PAGE_TURN_SPREAD_LAYER_FRAME) {
                    Hu3DModelLayerSet(lbl_1_bss_1FC[0][3], 4);
                    Hu3DModelLayerSet(lbl_1_bss_1FC[0][4], 5);
                }
                if (lbl_1_bss_1E0 == MIRACLEBOOK_PAGE_TURN_DETAIL_LAYER_FRAME) {
                    Hu3DModelLayerSet(lbl_1_bss_1FC[0][5], 3);
                }
            }
            if ((lbl_1_data_398[lbl_1_bss_1E4] == 8) &&
                (lbl_1_bss_1E0 == MIRACLEBOOK_PAGE_TURN_MULTI_ENTRY_LAYER_FRAME)) {
                Hu3DModelLayerSet(lbl_1_bss_1FC[0][1], 3);
                Hu3DModelLayerSet(lbl_1_bss_1FC[0][4], 4);
            }
            if (lbl_1_bss_1E0 == 0) {
                if (lbl_1_bss_224 == 5) {
                    if (lbl_1_bss_1E4 < MIRACLEBOOK_ENTRY_GROUP_2_START_INDEX) {
                        lbl_1_bss_180 = 0;
                        lbl_1_bss_138 = lbl_1_bss_1E4;
                    } else if (lbl_1_bss_1E4 < MIRACLEBOOK_ENTRY_GROUP_3_START_INDEX) {
                        lbl_1_bss_180 = 1;
                        lbl_1_bss_138 = lbl_1_bss_1E4 - MIRACLEBOOK_ENTRY_GROUP_2_START_INDEX;
                    } else {
                        lbl_1_bss_180 = 2;
                        lbl_1_bss_138 = lbl_1_bss_1E4 - MIRACLEBOOK_ENTRY_GROUP_3_START_INDEX;
                    }
                    lbl_1_bss_1E4 = 0;
                    fn_1_3418();
                    fn_1_3734(-1);
                    return;
                }
                if (Hu3DMotionEndCheck(lbl_1_bss_1FC[1][0]) != 0) {
                    lbl_1_bss_224 = 4;
                    lbl_1_bss_1E4 += lbl_1_bss_1C4;
                    fn_1_71E0(1);
                    fn_1_3418();
                    return;
                }
            }
        }
        break;
    case 8:
        fn_1_3734(0);
        return;
    case 9:
        fn_1_7998();
        break;
    }
}

// Called by page navigation to find the next unlocked entry in the requested direction.
s32 fn_1_2FCC(s32 dir)
{
    s32 result = 0;
    s32 i;

    if (dir > 0) {
        if (lbl_1_bss_1E4 + 1 == lbl_1_bss_1D4) {
            if (lbl_1_bss_1E8[lbl_1_bss_1E4 + 1] != MIRACLEBOOK_ENTRY_EMPTY) {
                result = 1;
            }
        } else {
            for (i = lbl_1_bss_1E4 + 1; i <= lbl_1_bss_1D4; i++) {
                if (lbl_1_bss_1E8[i] != MIRACLEBOOK_ENTRY_EMPTY) {
                    result = i - lbl_1_bss_1E4;
                    break;
                }
            }
        }
    } else if (dir < 0) {
        if (lbl_1_bss_1E4 - 1 == lbl_1_bss_1D8) {
            if (lbl_1_bss_1E8[lbl_1_bss_1E4 - 1] != MIRACLEBOOK_ENTRY_EMPTY) {
                result = -1;
            }
        } else {
            for (i = lbl_1_bss_1E4 - 1; i >= lbl_1_bss_1D8; i--) {
                if (lbl_1_bss_1E8[i] != MIRACLEBOOK_ENTRY_EMPTY) {
                    result = i - lbl_1_bss_1E4;
                    break;
                }
            }
        }
    }
    return result;
}

// Loads item models for the current page when dir is zero, or for the target page at current entry
// + dir otherwise.
void fn_1_3154(s32 dir)
{
    s32 bankFlag;
    s32 bank;
    s32 page;
    s32 total = 0;
    s32 sumIndex;
    s32 itemIndex;

    if (dir == 0) {
        bankFlag = 0;
    } else {
        bankFlag = 1;
    }
    bank = bankFlag;
    page = lbl_1_bss_1E4 + dir;
    if (page > 0) {
        for (sumIndex = 0; sumIndex < page; sumIndex++) {
            total += lbl_1_data_0[sumIndex];
        }
    }
    for (itemIndex = 0; itemIndex < 7; itemIndex++) {
        if (itemIndex >= lbl_1_data_0[page]) {
            lbl_1_bss_1FC[bank][itemIndex] = -1;
        } else {
            lbl_1_bss_1FC[bank][itemIndex] = Hu3DModelCreate(HuDataSelHeapReadNum(
                DATA_miraclebook + total + itemIndex + 8, HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelCameraSet(lbl_1_bss_1FC[bank][itemIndex], 1);
            Hu3DModelPosSet(lbl_1_bss_1FC[bank][itemIndex], (0.0f),
                (-65.0f), (0.0f));
            Hu3DModelRotSet(lbl_1_bss_1FC[bank][itemIndex], (0.0f),
                (0.0f), (-90.0f));
            if (lbl_1_data_50[page][itemIndex] == 1) {
                Hu3DModelAttrSet(lbl_1_bss_1FC[bank][itemIndex], HU3D_MOTATTR_PAUSE);
            } else if (lbl_1_data_50[page][itemIndex] == 2) {
                Hu3DModelAttrSet(lbl_1_bss_1FC[bank][itemIndex],
                    HU3D_MOTATTR_SHAPE_PAUSE);
            }
            Hu3DModelLayerSet(lbl_1_bss_1FC[bank][itemIndex],
                lbl_1_data_280[page][itemIndex]);
            Hu3DModelAttrSet(lbl_1_bss_1FC[bank][itemIndex], HU3D_ATTR_DISPOFF);
        }
    }
}

// Called after loading the next set of page models to replace the previous set.
void fn_1_3418(void)
{
    s32 i;

    for (i = 0; i < 7; i++) {
        if (lbl_1_bss_1FC[0][i] != -1) {
            Hu3DModelKill(lbl_1_bss_1FC[0][i]);
        }
        lbl_1_bss_1FC[0][i] = lbl_1_bss_1FC[1][i];
        lbl_1_bss_1FC[1][i] = -1;
    }
}

// Called during overlay setup to create the two cameras used by the book display.
void fn_1_34C4(void)
{
    OMOBJ *obj;
    HuVecF center;
    HuVecF rot;
    HuVecF *centerP;
    HuVecF *rotP;

    obj = omAddObjEx(lbl_1_bss_74, 32730, 0, 0, -1, omOutViewMulti);
    obj->work[0] = 2;
    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, (0.0f), (0.0f),
        (640.0f), (480.0f), (0.0f), (1.0f));
    Hu3DCameraPerspectiveSet(1, (35.259f), (20.0f),
        (30000.0f), (1.2f));
    Hu3DCameraCreate(2);
    Hu3DCameraViewportSet(2, (0.0f), (0.0f),
        (640.0f), (480.0f), (0.0f), (1.0f));
    Hu3DCameraPerspectiveSet(2, (35.259f), (20.0f),
        (30000.0f), (1.2f));
    lbl_1_bss_1A4.center = (HuVecF){0.0f, 0.0f, 0.0f};
    lbl_1_bss_1A4.rot = (HuVecF){-25.0f, 0.0f, 0.0f};
    lbl_1_bss_1A4.zoom = (701.8f);
    omCameraViewSetMulti(1, &lbl_1_bss_1A4);
    omCameraViewSetMulti(2, &lbl_1_bss_1A4);
    lbl_1_bss_1A0 = 0;
    lbl_1_bss_19C = (0.0f);
}

// Called after a page choice or cancellation to move the book camera in that direction.
void fn_1_3734(s32 cameraMoveDirection)
{
    f64 zeroForward;
    f64 zeroReverse;

    if (lbl_1_bss_1A0 == 0) {
        lbl_1_bss_1A0 = cameraMoveDirection;
        lbl_1_bss_224 = 8;
        return;
    }

    if ((0.0f) == lbl_1_bss_19C) {
        if (lbl_1_bss_1A0 == 1) {
            lbl_1_bss_1A4.zoom = (745.9f);
            omCameraViewMoveSimpleMulti(3, &lbl_1_bss_1A4, 50);
        } else if (lbl_1_bss_1A0 == -1) {
            lbl_1_bss_1A4.zoom = (701.8f);
            omCameraViewMoveSimpleMulti(3, &lbl_1_bss_1A4, 50);
        }
    }

    lbl_1_bss_19C += (1.8f);
    if (lbl_1_bss_19C > (90.0f)) {
        lbl_1_bss_19C = (90.0f);
    }

    if (lbl_1_bss_1A0 == 1) {
        lbl_1_bss_184[1] = (f32)(((86.0f) + lbl_1_bss_11C) *
            sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));
        sin((3.141592653589793) * lbl_1_bss_19C / (180.0));
        zeroForward = (0.0);
        lbl_1_bss_184[2] = (f32)zeroForward;
        lbl_1_bss_184[3] = (f32)((55.0) *
            sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));
        lbl_1_bss_184[5] = (f32)(lbl_1_bss_118 *
            sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));

        Hu3DModelPosSet(lbl_1_bss_21A, (0.0f),
            (21.0f) + lbl_1_bss_11C - lbl_1_bss_184[1],
            -lbl_1_bss_184[2]);
        Hu3DModelRotSet(lbl_1_bss_21A,
            (55.0f) - lbl_1_bss_184[3], (0.0f),
            lbl_1_bss_118 - lbl_1_bss_184[5]);
        Hu3DModelPosSet(lbl_1_bss_218, (0.0f),
            (21.0f) + lbl_1_bss_11C - lbl_1_bss_184[1],
            -lbl_1_bss_184[2]);
        Hu3DModelRotSet(lbl_1_bss_218,
            (55.0f) - lbl_1_bss_184[3], (0.0f),
            lbl_1_bss_118 - lbl_1_bss_184[5]);

        if ((90.0f) == lbl_1_bss_19C) {
            lbl_1_bss_224 = 2;
            lbl_1_bss_19C = (0.0f);
        }
    } else if (lbl_1_bss_1A0 == -1) {
        lbl_1_bss_184[1] = (f32)((86.0) *
            sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));
        sin((3.141592653589793) * lbl_1_bss_19C / (180.0));
        zeroReverse = (0.0);
        lbl_1_bss_184[2] = (f32)zeroReverse;
        lbl_1_bss_184[3] = (f32)((55.0) *
            sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));

        Hu3DModelPosSet(lbl_1_bss_21A, (0.0f),
            (-65.0f) + lbl_1_bss_184[1], lbl_1_bss_184[2]);
        Hu3DModelRotSet(lbl_1_bss_21A, lbl_1_bss_184[3],
            (0.0f), (0.0f));
        Hu3DModelPosSet(lbl_1_bss_218, (0.0f),
            (-65.0f) + lbl_1_bss_184[1], lbl_1_bss_184[2]);
        Hu3DModelRotSet(lbl_1_bss_218, lbl_1_bss_184[3],
            (0.0f), (0.0f));

        if ((90.0f) == lbl_1_bss_19C) {
            lbl_1_bss_224 = 2;
            lbl_1_bss_1A0 = 0;
            lbl_1_bss_19C = (0.0f);
            fn_1_71E0(0);
            lbl_1_bss_11C = (0.0f);
            lbl_1_bss_120[2] = lbl_1_bss_120[3] = (0.0f);
        }
    }
}

// Called during the opening sequence to move the book and its cover through the opening arc.
void fn_1_3E40(void)
{
    lbl_1_bss_19C += (1.5f);
    if (lbl_1_bss_19C > (90.0f)) {
        lbl_1_bss_19C = (90.0f);
    }

    lbl_1_bss_184[0] = (f32) ((100.0) *
        sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));
    lbl_1_bss_184[1] = (f32) (((-250.0f) + lbl_1_bss_11C) *
        sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));
    lbl_1_bss_184[2] = (f32) ((-500.0) *
        sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));
    lbl_1_bss_184[3] = (f32) ((70.0) *
        sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));
    lbl_1_bss_184[4] = (f32) ((-130.0) *
        sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));
    lbl_1_bss_184[5] = (f32) ((-45.0) *
        sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));

    Hu3DModelPosSet(lbl_1_bss_21A,
        (100.0f) - lbl_1_bss_184[0],
        (21.0f) + ((-250.0f) + lbl_1_bss_11C) - lbl_1_bss_184[1],
        (-500.0f) - lbl_1_bss_184[2]);
    Hu3DModelRotSet(lbl_1_bss_21A,
        (125.0f) - lbl_1_bss_184[3],
        (-130.0f) - lbl_1_bss_184[4],
        (-45.0f) - lbl_1_bss_184[5]);
    Hu3DModelPosSet(lbl_1_bss_218,
        (100.0f) - lbl_1_bss_184[0],
        (21.0f) + ((-250.0f) + lbl_1_bss_11C) - lbl_1_bss_184[1],
        (-500.0f) - lbl_1_bss_184[2]);
    Hu3DModelRotSet(lbl_1_bss_218,
        (125.0f) - lbl_1_bss_184[3],
        (-130.0f) - lbl_1_bss_184[4],
        (-45.0f) - lbl_1_bss_184[5]);
}

// Called by the page-selection transition to settle the book and reward models into place.
s32 fn_1_427C(void)
{
    s32 result = 0;

    lbl_1_bss_19C += (1.5f);
    if (lbl_1_bss_19C > (90.0f)) {
        lbl_1_bss_19C = (90.0f);
    }

    lbl_1_bss_184[1] = (f32) (lbl_1_bss_11C *
        ((1.0) - sin((3.141592653589793) * lbl_1_bss_19C / (180.0))));
    lbl_1_bss_184[2] = (f32) (lbl_1_bss_118 *
        ((1.0) - sin((3.141592653589793) * lbl_1_bss_19C / (180.0))));
    lbl_1_bss_184[5] = (f32) ((100.0) *
        sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));

    Hu3DModelPosSet(lbl_1_bss_21A, (0.0f),
        (21.0f) + lbl_1_bss_184[1], (0.0f));
    Hu3DModelRotSet(lbl_1_bss_21A, (55.0f),
        (0.0f), lbl_1_bss_184[2]);
    Hu3DModelPosSet(lbl_1_bss_218, (0.0f),
        (21.0f) + lbl_1_bss_184[1], (0.0f));
    Hu3DModelRotSet(lbl_1_bss_218, (55.0f),
        (0.0f), lbl_1_bss_184[2]);

    lbl_1_bss_184[0] = (f32) ((35.0) *
        sin((3.141592653589793) * lbl_1_bss_19C / (180.0)));
    if (lbl_1_bss_134 == 1) {
        Hu3DModelPosSet(lbl_1_bss_90[0], (0.0f),
            (86.0f), (5.0f) + lbl_1_bss_184[5]);
        Hu3DModelPosSet(lbl_1_bss_90[2], (0.0f),
            (86.0f), (5.0f) + lbl_1_bss_184[5]);
    } else if (lbl_1_bss_134 == 2) {
        Hu3DModelPosSet(lbl_1_bss_90[1], (0.0f),
            (-54.0f), (60.0f) + lbl_1_bss_184[5]);
        Hu3DModelPosSet(lbl_1_bss_90[2], (0.0f),
            (-54.0f), (60.0f) + lbl_1_bss_184[5]);
    }
    Hu3DModelRotSet(lbl_1_bss_90[0], (-35.0f) + lbl_1_bss_184[0],
        (0.0f), (0.0f));
    Hu3DModelRotSet(lbl_1_bss_90[1], (-35.0f) + lbl_1_bss_184[0],
        (0.0f), (0.0f));
    Hu3DModelRotSet(lbl_1_bss_90[2], (-35.0f) + lbl_1_bss_184[0],
        (0.0f), (0.0f));

    if ((90.0f) == lbl_1_bss_19C) {
        lbl_1_bss_19C = (0.0f);
        result = 1;
    }
    return result;
}

// Called during overlay setup to create the page selector, row markers and entry status icons.
void fn_1_479C(void)
{
    s32 i;
    s32 j;
    f32 bankPos;
    f32 itemPos;
    f32 posY;
    f32 posYWork;
    f32 posYForCall;

    lbl_1_bss_17A = espEntry(DATANUM(DATA_miraclebook, 83), 0, 0);
    espDrawNoSet(lbl_1_bss_17A, 0);
    espPosSet(lbl_1_bss_17A, (288.0f), (220.0f));
    espDispOff(lbl_1_bss_17A);
    espPriSet(lbl_1_bss_17A, 512);
    espScaleSet(lbl_1_bss_17A, (60.0f), (37.0f));
    espTPLvlSet(lbl_1_bss_17A, (0.2f));

    lbl_1_bss_178 = espEntry(DATANUM(DATA_miraclebook, 84), 0, 0);
    espDrawNoSet(lbl_1_bss_178, 0);
    espPosSet(lbl_1_bss_178, (306.2f), (246.0f));
    espScaleSet(lbl_1_bss_178, (12.4f), (1.0f));
    espTPLvlSet(lbl_1_bss_178, (0.4f));
    espDispOff(lbl_1_bss_178);
    espPriSet(lbl_1_bss_178, 256);

    for (i = 0; i < 2; i++) {
        lbl_1_bss_174[i] = espEntry(DATANUM(DATA_miraclebook, 81), 0, 0);
        espDrawNoSet(lbl_1_bss_174[i], 0);
        espBankSet(lbl_1_bss_174[i], i);
        if (i == 0) {
            bankPos = (74.0f);
        } else {
            bankPos = (422.0f);
        }
        espPosSet(lbl_1_bss_174[i],
            (20.0f) + ((20.0f) + bankPos),
            (95.0f));
        espDispOff(lbl_1_bss_174[i]);
        espPriSet(lbl_1_bss_174[i], 256);

        for (j = 0; j < 2; j++) {
            lbl_1_bss_16C[i][j] =
                espEntry(DATANUM(DATA_miraclebook, 82), 0, 0);
            espDrawNoSet(lbl_1_bss_16C[i][j], 0);
            espBankSet(lbl_1_bss_16C[i][j], j + i * 2);
            if (i == 0) {
                itemPos = (30.0f);
            } else {
                itemPos = (462.0f);
            }
            espPosSet(lbl_1_bss_16C[i][j],
                (22.0f) + ((20.0f) + itemPos),
                (92.0f));
            espDispOff(lbl_1_bss_16C[i][j]);
            espPriSet(lbl_1_bss_16C[i][j], 256);
            if (j == 1) {
                espTPLvlSet(lbl_1_bss_16C[i][j], (0.0f));
            }
        }
    }

    for (i = 0; i < 20; i++) {
        lbl_1_bss_144[i] = espEntry(DATANUM(DATA_miraclebook, 80), 0, 0);
        espDrawNoSet(lbl_1_bss_144[i], 0);
        if (i < 5) {
            posY = (266.0f) + (24.0f) * (f32)i;
        } else {
            if (i < 15) {
                posYWork = (146.0f) +
                    (24.0f) * (f32)(i - 5);
            } else {
                posYWork = (146.0f) +
                    (24.0f) * (f32)(i - 15);
            }
            posY = posYWork;
        }
        posYForCall = posY;
        espPosSet(lbl_1_bss_144[i], (118.0f),
            posYForCall - (20.0f));
        espDispOff(lbl_1_bss_144[i]);
        espPriSet(lbl_1_bss_144[i], 256);
    }

    lbl_1_bss_180 = 0;
    lbl_1_bss_140 = 0;
    lbl_1_bss_13C = 0;
    lbl_1_bss_138 = 0;
    lbl_1_bss_134 = 0;
    lbl_1_bss_120[0] = lbl_1_bss_120[1] = lbl_1_bss_120[2] =
        lbl_1_bss_120[3] = (0.0f);
    lbl_1_bss_11C = (0.0f);
    lbl_1_bss_17C = 0;
}

// Called by the overlay update to refresh page text and selector sprites for the current state.
void fn_1_4E94(void)
{
    if (lbl_1_bss_224 == 2) {
        if (lbl_1_bss_1A0 == 0) {
            fn_1_5ED0(1);
            fn_1_4FD4();
            fn_1_5904();
            fn_1_6B8C();
        }
    } else {
        fn_1_5ED0(0);
        fn_1_7158();
    }
}

// Called during overlay teardown to destroy the page selector and entry status sprites.
void fn_1_4EFC(void)
{
    s32 i;
    s32 j;

    espKill(lbl_1_bss_17A);
    espKill(lbl_1_bss_178);
    for (i = 0; i < 2; i++) {
        espKill(lbl_1_bss_174[i]);
        for (j = 0; j < 2; j++) {
            espKill(lbl_1_bss_16C[i][j]);
        }
    }
    for (i = 0; i < 20; i++) {
        espKill(lbl_1_bss_144[i]);
    }
}

// Called each menu update to handle vertical selection and keep the highlighted row visible.
void fn_1_4FD4(void)
{
    s32 i;
    s32 start;
    s32 end;
    s32 limit;
    s32 upLimit;
    s32 downLimit;
    f32 posY;

    if (lbl_1_bss_180 == 0) {
        Hu3DModelAttrReset(lbl_1_bss_90[0], HU3D_ATTR_DISPOFF);
        if (lbl_1_bss_138 == -1 || lbl_1_bss_138 == 5) {
            espDispOff(lbl_1_bss_178);
        }
        lbl_1_bss_1E4 = 0;
        if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            HuAudFXPlay(MSM_SE_FILESEL_05);
            lbl_1_bss_138--;
            if (lbl_1_bss_138 < -1) {
                lbl_1_bss_138 = 4;
            }
            if (lbl_1_bss_138 == -1) {
                espDispOff(lbl_1_bss_178);
                Hu3DModelPosSet(lbl_1_bss_90[2], (0.0f),
                    (86.0f), (5.0f));
                Hu3DModelAttrReset(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = -1;
            } else {
                espDispOn(lbl_1_bss_178);
                Hu3DModelAttrSet(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = 0;
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            HuAudFXPlay(MSM_SE_FILESEL_05);
            lbl_1_bss_138++;
            if (lbl_1_bss_138 > 5) {
                lbl_1_bss_138 = 0;
            }
            if (lbl_1_bss_138 == 5) {
                espDispOff(lbl_1_bss_178);
                Hu3DModelPosSet(lbl_1_bss_90[2], (0.0f),
                    (86.0f), (5.0f));
                Hu3DModelAttrReset(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = -1;
            } else {
                espDispOn(lbl_1_bss_178);
                Hu3DModelAttrSet(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = 0;
            }
        }
        posY = (267.0f) + (24.0f) *
            (f32)lbl_1_bss_138 - (1.0f);
        start = 0;
        end = 5;
        lbl_1_bss_1E4 += lbl_1_bss_138;
    } else if (lbl_1_bss_180 == 1) {
        lbl_1_bss_1E4 = 5;
        if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            HuAudFXPlay(MSM_SE_FILESEL_05);
            lbl_1_bss_138--;
            if (lbl_1_bss_138 < 0) {
                lbl_1_bss_138 = 9;
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            HuAudFXPlay(MSM_SE_FILESEL_05);
            lbl_1_bss_138++;
            if (lbl_1_bss_138 > 9) {
                lbl_1_bss_138 = 0;
            }
        }
        posY = (147.0f) + (24.0f) *
            (f32)lbl_1_bss_138 - (1.0f);
        start = 5;
        end = 15;
        lbl_1_bss_1E4 += lbl_1_bss_138;
        Hu3DModelAttrSet(lbl_1_bss_90[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_90[1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
        lbl_1_bss_134 = 0;
    } else {
        Hu3DModelAttrReset(lbl_1_bss_90[1], HU3D_ATTR_DISPOFF);
        if (lbl_1_bss_138 == -1 || lbl_1_bss_138 == 5) {
            espDispOff(lbl_1_bss_178);
        }
        lbl_1_bss_1E4 = 15;
        if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            HuAudFXPlay(MSM_SE_FILESEL_05);
            if (lbl_1_bss_130 == 20) {
                upLimit = -1;
            } else {
                upLimit = 0;
            }
            limit = upLimit;
            lbl_1_bss_138--;
            if (lbl_1_bss_138 < limit) {
                lbl_1_bss_138 = 4;
            }
            if (lbl_1_bss_138 == -1) {
                espDispOff(lbl_1_bss_178);
                Hu3DModelPosSet(lbl_1_bss_90[2], (0.0f),
                    (-54.0f), (60.0f));
                Hu3DModelAttrReset(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = -2;
            } else {
                espDispOn(lbl_1_bss_178);
                Hu3DModelAttrSet(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = 0;
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            HuAudFXPlay(MSM_SE_FILESEL_05);
            if (lbl_1_bss_130 == 20) {
                downLimit = 5;
            } else {
                downLimit = 4;
            }
            limit = downLimit;
            lbl_1_bss_138++;
            if (lbl_1_bss_138 > limit) {
                lbl_1_bss_138 = 0;
            }
            if (lbl_1_bss_138 == 5) {
                espDispOff(lbl_1_bss_178);
                Hu3DModelPosSet(lbl_1_bss_90[2], (0.0f),
                    (-54.0f), (60.0f));
                Hu3DModelAttrReset(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = -2;
            } else {
                espDispOn(lbl_1_bss_178);
                Hu3DModelAttrSet(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
                lbl_1_bss_134 = 0;
            }
        }
        posY = (147.0f) + (24.0f) *
            (f32)lbl_1_bss_138 - (1.0f);
        start = 15;
        end = 20;
        lbl_1_bss_1E4 += lbl_1_bss_138;
    }

    espPosSet(lbl_1_bss_178, (306.2f),
        posY - (20.0f));
    for (i = 0; i < 20; i++) {
        if (i >= start && i < end && lbl_1_bss_1E8[i] == 0) {
            espDispOn(lbl_1_bss_144[i]);
        } else {
            espDispOff(lbl_1_bss_144[i]);
        }
    }
}

// Called each menu update to animate switching between the three groups of book entries.
void fn_1_5904(void)
{
    s32 timerMax = 30;
    f32 posY;

    if (lbl_1_bss_13C == 0) {
        if ((HuPadBtnDown[0] & PAD_TRIGGER_R) && lbl_1_bss_180 != 2) {
            lbl_1_bss_140 = timerMax;
            lbl_1_bss_13C = 1;
        } else if ((HuPadBtnDown[0] & PAD_TRIGGER_L) && lbl_1_bss_180 != 0) {
            lbl_1_bss_140 = timerMax;
            lbl_1_bss_13C = -1;
        }
        if (lbl_1_bss_13C == 1) {
            espTPLvlSet(lbl_1_bss_16C[1][0], (0.0f));
            espTPLvlSet(lbl_1_bss_16C[1][1], (1.0f));
        } else if (lbl_1_bss_13C == -1) {
            espTPLvlSet(lbl_1_bss_16C[0][0], (0.0f));
            espTPLvlSet(lbl_1_bss_16C[0][1], (1.0f));
        }
        if (lbl_1_bss_13C != 0) {
            HuAudFXPlay(MSM_SE_FILESEL_12);
        }
    } else {
        if (lbl_1_bss_140 == timerMax / 2) {
            if (lbl_1_bss_13C == 1) {
                espTPLvlSet(lbl_1_bss_16C[1][0], (1.0f));
                espTPLvlSet(lbl_1_bss_16C[1][1], (0.0f));
            } else if (lbl_1_bss_13C == -1) {
                espTPLvlSet(lbl_1_bss_16C[0][0], (1.0f));
                espTPLvlSet(lbl_1_bss_16C[0][1], (0.0f));
            }
        }
        if (--lbl_1_bss_140 == 0) {
            if (lbl_1_bss_13C == 1) {
                lbl_1_bss_180++;
                if (lbl_1_bss_180 > 2) {
                    lbl_1_bss_180 = 2;
                }
            } else if (lbl_1_bss_13C == -1) {
                lbl_1_bss_180--;
                if (lbl_1_bss_180 < 0) {
                    lbl_1_bss_180 = 0;
                }
            }
            lbl_1_bss_13C = 0;
            if (lbl_1_bss_180 == 0) {
                posY = (266.0f);
            } else {
                posY = (146.0f);
            }
            espPosSet(lbl_1_bss_178, (306.2f), posY - (20.0f));
            lbl_1_bss_138 = 0;
        }
    }
    if (lbl_1_bss_180 == 0) {
        espColorSet(lbl_1_bss_174[0], 128, 128, 128);
        espColorSet(lbl_1_bss_174[1], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[0][0], 128, 128, 128);
        espColorSet(lbl_1_bss_16C[0][1], 128, 128, 128);
        espColorSet(lbl_1_bss_16C[1][0], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[1][1], 255, 255, 255);
    } else if (lbl_1_bss_180 == 1) {
        espColorSet(lbl_1_bss_174[0], 255, 255, 255);
        espColorSet(lbl_1_bss_174[1], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[0][0], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[0][1], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[1][0], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[1][1], 255, 255, 255);
    } else {
        espColorSet(lbl_1_bss_174[0], 255, 255, 255);
        espColorSet(lbl_1_bss_174[1], 128, 128, 128);
        espColorSet(lbl_1_bss_16C[0][0], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[0][1], 255, 255, 255);
        espColorSet(lbl_1_bss_16C[1][0], 128, 128, 128);
        espColorSet(lbl_1_bss_16C[1][1], 128, 128, 128);
    }
}

// Called by the menu state machine to show or hide the page-selection sprites.
void fn_1_5ED0(s32 displaySprites)
{
    s32 i;
    s32 j;

    if (displaySprites != 0) {
        espDispOn(lbl_1_bss_17A);
        espDispOn(lbl_1_bss_178);
        for (i = 0; i < 2; i++) {
            espDispOn(lbl_1_bss_174[i]);
            for (j = 0; j < 2; j++) {
                espDispOn(lbl_1_bss_16C[i][j]);
            }
        }
        for (i = 0; i < 20; i++) {
            espDispOn(lbl_1_bss_144[i]);
        }
    } else {
        espDispOff(lbl_1_bss_17A);
        espDispOff(lbl_1_bss_178);
        for (i = 0; i < 2; i++) {
            espDispOff(lbl_1_bss_174[i]);
            for (j = 0; j < 2; j++) {
                espDispOff(lbl_1_bss_16C[i][j]);
            }
        }
        for (i = 0; i < 20; i++) {
            espDispOff(lbl_1_bss_144[i]);
        }
        if (lbl_1_bss_224 != 9) {
            Hu3DModelAttrSet(lbl_1_bss_90[0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(lbl_1_bss_90[1], HU3D_ATTR_DISPOFF);
        }
    }
}

// Runs as a child process to bob the book and animate page and reward models while the menu is
// open.
void fn_1_60A4(void)
{
    s32 i;

    if (lbl_1_bss_224 == 0) {
        lbl_1_bss_120[2] += (2.0f);
        if (lbl_1_bss_120[2] > (360.0f)) {
            lbl_1_bss_120[2] -= (360.0f);
        }
        lbl_1_bss_11C = (f32) ((10.0) *
            sin((3.141592653589793) * lbl_1_bss_120[2] / (180.0)));
        Hu3DModelPosSet(lbl_1_bss_21A, (100.0f),
            (21.0f) + ((-250.0f) + lbl_1_bss_11C),
            (-500.0f));
        Hu3DModelPosSet(lbl_1_bss_218, (100.0f),
            (21.0f) + ((-250.0f) + lbl_1_bss_11C),
            (-500.0f));
    }

    if (lbl_1_bss_224 == 2 && lbl_1_bss_1A0 == 0) {
        lbl_1_bss_120[0] += (5.0f);
        if (lbl_1_bss_120[0] > (360.0f)) {
            lbl_1_bss_120[0] -= (360.0f);
        }
        for (i = 0; i < 20; i++) {
            espZRotSet(lbl_1_bss_144[i], (f32) ((10.0) *
                sin((3.141592653589793) * lbl_1_bss_120[0] / (180.0))));
        }

        if (lbl_1_bss_134 == 0) {
            lbl_1_bss_120[1] += (2.0f);
            if (lbl_1_bss_120[1] > (360.0f)) {
                lbl_1_bss_120[1] -= (360.0f);
            }
            if (lbl_1_bss_180 == 2 && lbl_1_bss_130 < 20) {
                lbl_1_bss_120[1] = (0.0f);
            }
            Hu3DModelRotSet(lbl_1_bss_90[0], (-35.0f),
                (0.0f), (f32) (3.0 *
                    sin((3.141592653589793) * lbl_1_bss_120[1] / (180.0))));
            Hu3DModelRotSet(lbl_1_bss_90[1], (-35.0f),
                (0.0f), (f32) (3.0 *
                    sin((3.141592653589793) * lbl_1_bss_120[1] / (180.0))));
            Hu3DModelRotSet(lbl_1_bss_90[2], (-35.0f),
                (0.0f), (f32) (3.0 *
                    sin((3.141592653589793) * lbl_1_bss_120[1] / (180.0))));
            lbl_1_bss_84 = (1.0f);
            Hu3DModelScaleSet(lbl_1_bss_90[0], lbl_1_bss_84,
                lbl_1_bss_84, (1.0f));
            Hu3DModelScaleSet(lbl_1_bss_90[1], lbl_1_bss_84,
                lbl_1_bss_84, (1.0f));
            Hu3DModelScaleSet(lbl_1_bss_90[2], lbl_1_bss_84,
                lbl_1_bss_84, (1.0f));
        } else {
            lbl_1_bss_120[1] = (0.0f);
            Hu3DModelRotSet(lbl_1_bss_90[0], (-35.0f),
                (0.0f), (0.0f));
            Hu3DModelRotSet(lbl_1_bss_90[1], (-35.0f),
                (0.0f), (0.0f));
            Hu3DModelRotSet(lbl_1_bss_90[2], (-35.0f),
                (0.0f), (0.0f));
            lbl_1_bss_84 = (1.2f);
            Hu3DModelScaleSet(lbl_1_bss_90[0], lbl_1_bss_84,
                lbl_1_bss_84, (1.0f));
            Hu3DModelScaleSet(lbl_1_bss_90[1], lbl_1_bss_84,
                lbl_1_bss_84, (1.0f));
            Hu3DModelScaleSet(lbl_1_bss_90[2], lbl_1_bss_84,
                lbl_1_bss_84, (1.0f));
        }

        lbl_1_bss_120[2] += (2.0f);
        if (lbl_1_bss_120[2] > (360.0f)) {
            lbl_1_bss_120[2] -= (360.0f);
        }
        lbl_1_bss_11C = (f32) ((2.0) *
            sin((3.141592653589793) * lbl_1_bss_120[2] / (180.0)));
        Hu3DModelPosSet(lbl_1_bss_21A, (0.0f),
            (21.0f) + lbl_1_bss_11C, (0.0f));
        Hu3DModelPosSet(lbl_1_bss_218, (0.0f),
            (21.0f) + lbl_1_bss_11C, (0.0f));

        lbl_1_bss_120[3] += (1.0f);
        if (lbl_1_bss_120[3] > (360.0f)) {
            lbl_1_bss_120[3] -= (360.0f);
        }
        lbl_1_bss_118 = (f32) ((1.5) *
            sin((3.141592653589793) * lbl_1_bss_120[3] / (180.0)));
        Hu3DModelRotSet(lbl_1_bss_21A, (55.0f),
            (0.0f), lbl_1_bss_118);
        Hu3DModelRotSet(lbl_1_bss_218, (55.0f),
            (0.0f), lbl_1_bss_118);
    }
    HuPrcVSleep();
}

// Called during overlay setup to mark all page text windows as not yet allocated.
void fn_1_69A8(void)
{
    s32 i;

    for (i = 0; i < 20; i++) {
        lbl_1_bss_B4[i] = -1;
    }
    lbl_1_bss_B0 = 0;
}

// Called when the overlay first opens to show and wait for the Miracle Book introduction.
void fn_1_69F8(void)
{
    HuVec2f size;
    f32 posY;
    HUWIN *win;

    HuWinMesMaxSizeGet(1, &size, MESSNUM(MESS_MBOOK_PAGE, 23));
    size.x = (544.0f);
    posY = (360.0f);
    lbl_1_bss_114 = HuWinExCreateFrame((-10000.0f), posY, (s16)size.x,
        (s16)size.y, -1, 0);
    HuWinAttrSet((s16)lbl_1_bss_114, HUWIN_ATTR_ALIGN_CENTER);
    HuWinExOpen((s16)lbl_1_bss_114);
    HuWinPriSet((s16)lbl_1_bss_114, 0);
    HuWinMesSpeedSet((s16)lbl_1_bss_114, 1);
    HuWinMesSet((s16)lbl_1_bss_114, MESSNUM(MESS_MBOOK_PAGE, 23));
    win = &winData[lbl_1_bss_114];
    win->padMask = 1;
    win->disablePlayer = 14;
    HuWinMesWait((s16)lbl_1_bss_114);
    HuWinExClose((s16)lbl_1_bss_114);
    HuWinExKill((s16)lbl_1_bss_114);
}

// Called after a page or entry-group change to create the visible entry names and page caption.
void fn_1_6B8C(void)
{
    HuVec2f size;
    f32 posX;
    f32 posY;
    s32 start;
    s32 end;
    s32 i;
    u32 messNo;

    fn_1_7158();
    if (lbl_1_bss_180 == 0) {
        start = 0;
        end = 5;
        for (i = start; i < end; i++) {
            messNo = (lbl_1_bss_1E8[i] == MIRACLEBOOK_ENTRY_EMPTY) ?
                lbl_1_data_6F4[0] : lbl_1_data_6F4[i + 1];
            HuWinMesMaxSizeGet(1, &size, messNo);
            posX = (140.0f);
            posY = (254.0f) +
                (24.0f) * (f32)(i - start) -
                (8.0f) - (20.0f);
            lbl_1_bss_B4[i] = HuWinCreate(posX, posY,
                (s16)size.x, (s16)size.y, 0);
            HuWinPriSet((s16)lbl_1_bss_B4[i], 0);
            HuWinBGTPLvlSet((s16)lbl_1_bss_B4[i], (0.0f));
            HuWinMesSpeedSet((s16)lbl_1_bss_B4[i], 0);
            HuWinMesSet((s16)lbl_1_bss_B4[i], messNo);
        }
        messNo = lbl_1_data_748[0];
        HuWinMesMaxSizeGet(1, &size, messNo);
        posX = (240.0f);
        posY = (127.0f);
        lbl_1_bss_B4[20] = HuWinCreate(posX, posY,
            (s16)size.x, (s16)size.y, 0);
        HuWinPriSet((s16)lbl_1_bss_B4[20], 0);
        HuWinBGTPLvlSet((s16)lbl_1_bss_B4[20], (0.0f));
        HuWinMesSpeedSet((s16)lbl_1_bss_B4[20], 0);
        HuWinMesSet((s16)lbl_1_bss_B4[20], messNo);
    } else {
        if (lbl_1_bss_180 == 1) {
            start = 5;
            end = 15;
        } else {
            start = 15;
            end = 20;
        }
        for (i = start; i < end; i++) {
            messNo = (lbl_1_bss_1E8[i] == MIRACLEBOOK_ENTRY_EMPTY) ?
                lbl_1_data_6F4[0] : lbl_1_data_6F4[i + 1];
            HuWinMesMaxSizeGet(1, &size, messNo);
            posX = (140.0f);
            posY = (134.0f) +
                (24.0f) * (f32)(i - start) -
                (8.0f) - (20.0f);
            lbl_1_bss_B4[i] = HuWinCreate(posX, posY,
                (s16)size.x, (s16)size.y, 0);
            HuWinPriSet((s16)lbl_1_bss_B4[i], 0);
            HuWinBGTPLvlSet((s16)lbl_1_bss_B4[i], (0.0f));
            HuWinMesSpeedSet((s16)lbl_1_bss_B4[i], 0);
            HuWinMesSet((s16)lbl_1_bss_B4[i], messNo);
        }
        if (lbl_1_bss_180 == 2) {
            messNo = lbl_1_data_748[1];
            HuWinMesMaxSizeGet(1, &size, messNo);
            posX = (240.0f);
            posY = (297.0f);
            lbl_1_bss_B4[21] = HuWinCreate(posX, posY,
                (s16)size.x, (s16)size.y, 0);
            HuWinPriSet((s16)lbl_1_bss_B4[21], 0);
            HuWinBGTPLvlSet((s16)lbl_1_bss_B4[21], (0.0f));
            HuWinMesSpeedSet((s16)lbl_1_bss_B4[21], 0);
            HuWinMesSet((s16)lbl_1_bss_B4[21], messNo);
        }
    }
}

// Called when the page changes or the overlay closes to release all entry text windows.
void fn_1_7158(void)
{
    s32 i;

    for (i = 0; i < 22; i++) {
        if (lbl_1_bss_B4[i] != -1) {
            HuWinKill((s16)lbl_1_bss_B4[i]);
            lbl_1_bss_B4[i] = -1;
        }
    }
}

// Called when a page animation changes phase to display its short instruction message.
void fn_1_71E0(s32 messNo)
{
    HuVec2f size;
    f32 posX;
    f32 posY;

    HuWinMesMaxSizeGet(1, &size, lbl_1_data_750[messNo]);
    posX = (-10000.0f);
    posY = (440.0f) - size.y;
    lbl_1_bss_110 = HuWinCreate(posX, posY, (s16)size.x, (s16)size.y, 0);
    HuWinPriSet((s16)lbl_1_bss_110, 0);
    HuWinBGTPLvlSet((s16)lbl_1_bss_110, (0.0f));
    HuWinMesSpeedSet((s16)lbl_1_bss_110, 0);
    HuWinMesSet((s16)lbl_1_bss_110, lbl_1_data_750[messNo]);
}

// Called before a replacement instruction message is created to remove the old one.
void fn_1_7324(void)
{
    HuWinKill((s16)lbl_1_bss_110);
}

// Called by the overlay update to process the selected entry's optional detail prompt.
s32 fn_1_7354(void)
{
    HuVec2f size;
    f32 posX;
    f32 posY;
    HUWIN *win;
    s32 result = -1;

    if (((lbl_1_bss_224 == 2 && lbl_1_bss_13C == 0) ||
            (lbl_1_bss_224 == 4 && lbl_1_bss_1DC == 0)) &&
        (HuPadBtnDown[0] & PAD_BUTTON_X)) {
            HuAudFXPlay(MSM_SE_CMN_04);
        HuWinMesMaxSizeGet(1, &size, MESSNUM(MESS_MBOOK_PAGE, 24));
        posX = (-10000.0f);
        posY = (240.0f) - size.y;
        lbl_1_bss_10C = HuWinCreate(posX, posY, (s16)size.x, (s16)size.y, 0);
        HuWinAttrSet((s16)lbl_1_bss_10C, HUWIN_ATTR_ALIGN_CENTER);
        HuWinPriSet((s16)lbl_1_bss_10C, 0);
        HuWinMesSpeedSet((s16)lbl_1_bss_10C, 0);
        HuWinMesSet((s16)lbl_1_bss_10C, MESSNUM(MESS_MBOOK_PAGE, 24));
        win = &winData[lbl_1_bss_10C];
        win->padMask = 1;
        win->disablePlayer = 14;
        result = (s16)HuWinChoiceGet((s16)lbl_1_bss_10C, -1);
        HuWinKill((s16)lbl_1_bss_10C);
        lbl_1_bss_B0 = 1;
    }
    return result;
}

// Called during overlay setup to create the reward and page-effect models and the sparkle particle
// managers.
void fn_1_7560(void)
{
    ANIMDATA *anim;
    s16 i;

    omAddObjEx(lbl_1_bss_74, 4096, 16, 16, -1, fn_1_92B4);
    Hu3DParManInit();
    lbl_1_bss_90[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 3), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_90[0], 1);
    Hu3DModelPosSet(lbl_1_bss_90[0], (0.0f),
        (86.0f), (5.0f));
    Hu3DModelRotSet(lbl_1_bss_90[0], (-35.0f),
        (0.0f), (0.0f));
    Hu3DModelLayerSet(lbl_1_bss_90[0], 5);
    Hu3DModelAttrSet(lbl_1_bss_90[0], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_90[0], HU3D_ATTR_DISPOFF);
    lbl_1_bss_90[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 4), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_90[1], 1);
    Hu3DModelPosSet(lbl_1_bss_90[1], (0.0f),
        (-54.0f), (60.0f));
    Hu3DModelRotSet(lbl_1_bss_90[1], (-35.0f),
        (0.0f), (0.0f));
    Hu3DModelLayerSet(lbl_1_bss_90[1], 5);
    Hu3DModelAttrSet(lbl_1_bss_90[1], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(lbl_1_bss_90[1], HU3D_ATTR_DISPOFF);
    lbl_1_bss_90[2] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 5), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_90[2], 1);
    Hu3DModelPosSet(lbl_1_bss_90[2], (0.0f),
        (86.0f), (5.0f));
    Hu3DModelRotSet(lbl_1_bss_90[2], (-35.0f),
        (0.0f), (0.0f));
    Hu3DModelLayerSet(lbl_1_bss_90[2], 5);
    Hu3DModelAttrSet(lbl_1_bss_90[2], HU3D_ATTR_DISPOFF);
    lbl_1_bss_8C = 0;
    anim = HuSprAnimRead(HuDataSelHeapReadNum(
        DATANUM(DATA_effect, 0), HU_MEMNUM_OVL, HEAP_MODEL));
    for (i = 0; i < 5; i++) {
        lbl_1_bss_68[i] = Hu3DParManCreate(anim, 100, &lbl_1_data_5C8);
        Hu3DParManAttrSet(lbl_1_bss_68[i], 100);
        Hu3DParticleBlendModeSet(Hu3DParManModelIDGet(lbl_1_bss_68[i]), 1);
        Hu3DParManRotSet(lbl_1_bss_68[i], (90.0f),
            (0.0f), (0.0f));
        Hu3DModelCameraSet(Hu3DParManModelIDGet(lbl_1_bss_68[i]), 2);
        Hu3DModelLayerSet(Hu3DParManModelIDGet(lbl_1_bss_68[i]), 4);
        Hu3DParManAttrSet(lbl_1_bss_68[i], HU3D_PARMAN_ATTR_TIMEUP);
    }
}

// Called by the overlay update to animate the selected Miracle Book reward effect.
void fn_1_7998(void)
{

    f32 amplitude;

    switch (lbl_1_bss_8C) {
        case 0:
            fn_1_5ED0(0);
            fn_1_7158();
            HuWinKill((s16) lbl_1_bss_110);
            if (fn_1_427C() == 0) {
                return;
            }
            lbl_1_bss_8C++;
            lbl_1_bss_78.x = lbl_1_bss_78.y = lbl_1_bss_78.z =
                (0.0f);
            lbl_1_bss_88 = (1.0f);
            HuAudFXPlay(MSM_SE_FILESEL_13);
            break;

        case 1:
            lbl_1_bss_78.x += (2.0f);
            if (lbl_1_bss_78.x > (180.1f)) {
                lbl_1_bss_78.x = (180.1f);
            }
            lbl_1_bss_78.y += (1.0f);
            if (lbl_1_bss_78.y > (90.0f)) {
                lbl_1_bss_78.y = (90.0f);
            }
            if (lbl_1_bss_78.x >= (90.0f)) {
                lbl_1_bss_78.z += (1.5f);
                if (lbl_1_bss_78.z > (90.0f)) {
                    lbl_1_bss_78.z = (90.0f);
                }
            }

            if (lbl_1_bss_134 == 1) {
                amplitude = (35.0f);
                Hu3DModelPosSet(lbl_1_bss_90[0], (0.0f),
                    (f32) ((86.0) - amplitude *
                        sin((3.141592653589793) * lbl_1_bss_78.y / (180.0))),
                    (f32) ((105.0) + (100.0) *
                        sin((3.141592653589793) * lbl_1_bss_78.x / (180.0))));
                Hu3DModelRotSet(lbl_1_bss_90[0], (0.0f),
                    (0.0f),
                    (f32) ((360.0) *
                        sin((3.141592653589793) * lbl_1_bss_78.y / (180.0)) +
                        (360.0) * ((1.0) -
                            sin((3.141592653589793) *
                                ((90.0f) - lbl_1_bss_78.z) /
                                (180.0)))));

                Hu3DModelPosSet(lbl_1_bss_90[2], (0.0f),
                    (f32) ((86.0) - amplitude *
                        sin((3.141592653589793) * lbl_1_bss_78.y / (180.0))),
                    (f32) ((105.0) + (100.0) *
                        sin((3.141592653589793) * lbl_1_bss_78.x / (180.0))));
                Hu3DModelRotSet(lbl_1_bss_90[2], (0.0f),
                    (0.0f),
                    (f32) ((360.0) *
                        sin((3.141592653589793) * lbl_1_bss_78.y / (180.0)) +
                        (360.0) * ((1.0) -
                            sin((3.141592653589793) *
                                ((90.0f) - lbl_1_bss_78.z) /
                                (180.0)))));
            } else if (lbl_1_bss_134 == 2) {
                amplitude = (105.0f);
                Hu3DModelPosSet(lbl_1_bss_90[1], (0.0f),
                    (f32) ((-54.0) + amplitude *
                        sin((3.141592653589793) * lbl_1_bss_78.y / (180.0))),
                    (f32) ((160.0) + (40.0) *
                        sin((3.141592653589793) * lbl_1_bss_78.x / (180.0))));
                Hu3DModelRotSet(lbl_1_bss_90[1], (0.0f),
                    (0.0f),
                    (f32) ((360.0) *
                        sin((3.141592653589793) * lbl_1_bss_78.y / (180.0)) +
                        (360.0) * ((1.0) -
                            sin((3.141592653589793) *
                                ((90.0f) - lbl_1_bss_78.z) /
                                (180.0)))));

                Hu3DModelPosSet(lbl_1_bss_90[2], (0.0f),
                    (f32) ((-54.0) + amplitude *
                        sin((3.141592653589793) * lbl_1_bss_78.y / (180.0))),
                    (f32) ((160.0) + (40.0) *
                        sin((3.141592653589793) * lbl_1_bss_78.x / (180.0))));
                Hu3DModelRotSet(lbl_1_bss_90[2], (0.0f),
                    (0.0f),
                    (f32) ((360.0) *
                        sin((3.141592653589793) * lbl_1_bss_78.y / (180.0)) +
                        (360.0) * ((1.0) -
                            sin((3.141592653589793) *
                                ((90.0f) - lbl_1_bss_78.z) /
                                (180.0)))));
            }

            if (lbl_1_bss_78.x >= (160.0f)) {
                lbl_1_bss_88 -= (0.03f);
                if (lbl_1_bss_88 < (0.0f)) {
                    lbl_1_bss_88 = (0.0f);
                }
                if (lbl_1_bss_134 == 1) {
                    Hu3DModelTPLvlSet(lbl_1_bss_90[0], lbl_1_bss_88);
                } else if (lbl_1_bss_134 == 2) {
                    Hu3DModelTPLvlSet(lbl_1_bss_90[1], lbl_1_bss_88);
                }
                Hu3DModelTPLvlSet(lbl_1_bss_90[2], lbl_1_bss_88);
            }
            fn_1_9718();
            if (lbl_1_bss_78.x > (180.0f)) {
                Hu3DParManAttrSet(lbl_1_bss_68[0], HU3D_PARMAN_ATTR_TIMEUP);
                Hu3DParManAttrSet(lbl_1_bss_68[1], HU3D_PARMAN_ATTR_TIMEUP);
                Hu3DParManAttrSet(lbl_1_bss_68[2], HU3D_PARMAN_ATTR_TIMEUP);
                Hu3DParManAttrSet(lbl_1_bss_68[3], HU3D_PARMAN_ATTR_TIMEUP);
                Hu3DParManAttrSet(lbl_1_bss_68[4], HU3D_PARMAN_ATTR_TIMEUP);
            }
            if ((180.0f) == lbl_1_bss_78.x) {
                HuVecF pos = {0.0f, 0.0f, 0.0f};
                fn_1_9130(0, &pos);
                HuAudFXPlay(MSM_SE_FILESEL_14);
            }
            if ((90.0f) == lbl_1_bss_78.x) {
                lbl_1_bss_1A4.zoom = (100.0f);
                omCameraViewMoveMulti(3, &lbl_1_bss_1A4, 90, 1);
            }
            break;
    }
    if ((0.0f) == lbl_1_bss_88) {
        lbl_1_bss_17C = 1;
    }
}

// Copies the current frame into the page-effect texture when its buffer exists; layerNo is ignored.
void fn_1_8A0C(s16 layerNo)
{
    if (lbl_1_bss_C) {
        GXSetTexCopySrc(0, 0, 640, 480);
        GXSetTexCopyDst(640, 480, GX_TF_RGB565, GX_FALSE);
        GXCopyTex(lbl_1_bss_C, GX_FALSE);
    }
}

// While either page-effect model is drawn, advances its texture offset and sets up screen-texture
// mapping and TEV color; material is unused.
void fn_1_8A78(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    HU3D_CAMERA *camera = &Hu3DCamera[Hu3DCameraNo];
    UNK_MIRACLEBOOK_HOOK *effect;
    Mtx modelMatrix;
    Mtx scaleMatrix;
    Mtx textureMatrix;
    Mtx lightMatrix;
    Mtx inverseCameraMatrix;
    GXTexObj screenTexture;
    f32 cameraCenterS;

    effect = drawObj->model->hookData;
    effect->textureOffsetY = effect->textureOffsetY + effect->textureOffsetSpeed;

    GXInitTexObj(&screenTexture, lbl_1_bss_C, 640, 480, GX_TF_RGB565, 0, 0, 0);
    GXInitTexObjLOD(&screenTexture, 1, 1, (0.0f), (0.0f),
        (0.0f), 0, 0, 0);
    GXLoadTexObj(&screenTexture, 1);
    HuSprTexLoad(lbl_1_bss_10, 0, 2, 1, 1, 1);
    HuSprTexLoad(lbl_1_bss_14, 0, 3, 1, 1, 1);
    GXSetNumTexGens(3);
    GXSetNumTevStages(2);

    cameraCenterS = ((f32)camera->scissorX +
        (f32)camera->scissorW / 2.0f) / (640.0f);
    C_MTXLightPerspective(lightMatrix, camera->fov, (1.2f),
        (0.5f), (-0.5f), cameraCenterS, (0.5f));
    PSMTXInverse(Hu3DCameraMtx, inverseCameraMatrix);
    PSMTXConcat(inverseCameraMatrix, drawObj->matrix, modelMatrix);
    PSMTXConcat(lightMatrix, Hu3DCameraMtx, textureMatrix);
    PSMTXConcat(textureMatrix, modelMatrix, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX1, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS,
        GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);

    PSMTXTrans(modelMatrix, (0.0f), effect->textureOffsetY, (0.0f));
    PSMTXScale(scaleMatrix, (0.8f), (0.8f),
        (1.0f));
    PSMTXConcat(scaleMatrix, modelMatrix, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCoordGen2(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);

    GXSetTevColor(1, effect->color);
    GXSetTevOrder(0, 0, 1, 0);
    GXSetTevColorIn(0, 15, 15, 15, 8);
    GXSetTevColorOp(0, 0, 0, 0, 1, 0);
    GXSetTevAlphaIn(0, 7, 7, 7, 6);
    GXSetTevAlphaOp(0, 0, 0, 0, 1, 0);
    GXSetTevOrder(1, 1, 0, 0);
    GXSetTevColorIn(1, 15, 8, 2, 0);
    GXSetTevColorOp(1, 0, 0, 0, 1, 0);
    GXSetTevAlphaIn(1, 7, 7, 7, 6);
    GXSetTevAlphaOp(1, 0, 0, 0, 1, 0);
    GXSetNumIndStages(1);
    GXSetIndTexOrder(0, 1, 2);
    GXSetIndTexCoordScale(0, 0, 0);
    GXSetTevIndWarp(0, 0, 1, 0, 1);
    GXSetIndTexMtx(1, (f32 (*)[3])effect, -1);
}

// Called by the effect object each update to fade and finish its two page overlays.
void fn_1_8F10(OMOBJ *obj)
{
    UNK_MIRACLEBOOK_HOOK *effect;
    s16 i = 0;
    f32 colorFadeProgress;

    effect = lbl_1_bss_18;
    for (; i < 2; i++, effect++) {
        if (effect->effectFramesRemaining <= (0.0f)) {
            continue;
        } else {
            effect->textureMatrix00 = (0.0f);
            effect->textureMatrix01 = (0.0f);
            effect->textureMatrix02 = (0.09f) * effect->effectFramesRemaining / (120.0f);
            effect->textureMatrix10 = (0.0f);
            effect->textureMatrix11 = (0.0f);
            effect->textureMatrix12 = (0.09f) * effect->effectFramesRemaining / (120.0f);
            effect->textureOffsetSpeed = (-0.02f) * effect->effectFramesRemaining / (120.0f);
            colorFadeProgress = (effect->effectFramesRemaining - (80.0f)) / (40.0f);
            if (colorFadeProgress >= (0.0f)) {
                effect->color.r = 0;
                effect->color.g = (u8)((16.0f) * colorFadeProgress);
                effect->color.b = (u8)((64.0f) * colorFadeProgress);
            }
            effect->effectFramesRemaining -= (1.0f);
            if (effect->effectFramesRemaining <= (0.0f)) {
                Hu3DLayerHookReset(10);
                Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
                obj->objFunc = NULL;
            }
        }
    }
}

// Called at 180 degrees in the reward animation to start its timed effect; position is ignored.
void fn_1_9130(s16 effectIndex, HuVecF *position)
{
    OMOBJ *obj = lbl_1_bss_8;

    if (!obj) {
        return;
    }
    if (effectIndex >= 2 || effectIndex < 0) {
        return;
    }
    lbl_1_bss_18[effectIndex].textureOffsetY = (0.0f);
    lbl_1_bss_18[effectIndex].effectFramesRemaining = (120.0f);
    Hu3DModelAttrReset(obj->mdlId[effectIndex], HU3D_ATTR_DISPOFF);
    Hu3DModelPosSet(obj->mdlId[effectIndex], (0.0f),
        (21.0f), (10.0f));
    Hu3DModelRotSet(obj->mdlId[effectIndex], (55.0f),
        (0.0f), (0.0f));
    Hu3DModelScaleSet(obj->mdlId[effectIndex], (1.0f),
        (1.0f), (1.0f));
    Hu3DLayerHookSet(10, fn_1_8A0C);
    obj->objFunc = fn_1_8F10;
}

// Called when the effect object is created to load and prepare both overlay models.
void fn_1_92B4(OMOBJ *obj)
{
    HU3D_MODEL *model = NULL;
    GXColor color;
    GXColor *colorP;
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    lbl_1_bss_10 = HuSprAnimRead(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 86), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_14 = HuSprAnimRead(HuDataSelHeapReadNum(
        DATANUM(DATA_miraclebook, 86), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_C = HuMemDirectMallocNum(2,
        GXGetTexBufferSize(640, 480, GX_TF_RGB565, GX_FALSE, 0), HU_MEMNUM_OVL);
    for (i = 0; i < 2; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_miraclebook, 85), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(obj->mdlId[i], 2);
        Hu3DModelLayerSet(obj->mdlId[i], 3);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DModelMatHookSet(obj->mdlId[i], fn_1_8A78);
        lbl_1_bss_18[i].textureMatrix00 = lbl_1_bss_18[i].textureMatrix01 =
            lbl_1_bss_18[i].textureMatrix02 = lbl_1_bss_18[i].textureMatrix10 =
            lbl_1_bss_18[i].textureMatrix11 = lbl_1_bss_18[i].textureMatrix12 =
                (0.0f);
        lbl_1_bss_18[i].color = (GXColor){0, 16, 64, 16};
        lbl_1_bss_18[i].textureOffsetY = (0.0f);
        lbl_1_bss_18[i].textureOffsetSpeed = (0.0f);
        model = &Hu3DData[obj->mdlId[i]];
        model->hookData = &lbl_1_bss_18[i];
    }
    obj->objFunc = NULL;
    lbl_1_bss_8 = obj;
}

// Called when the Miracle Book closes to release its effect models, images and buffer.
void fn_1_95B8(void)
{
    OMOBJ *obj = lbl_1_bss_8;
    HU3D_MODEL *model = NULL;
    s16 i;

    if (obj) {
        Hu3DLayerHookReset(10);
        for (i = 0; i < 2; i++) {
            model = &Hu3DData[obj->mdlId[i]];
            model->hookData = NULL;
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_74, obj);
    }
    obj = NULL;
    if (lbl_1_bss_10) {
        HuSprAnimKill(lbl_1_bss_10);
    }
    lbl_1_bss_10 = NULL;
    if (lbl_1_bss_14) {
        HuSprAnimKill(lbl_1_bss_14);
    }
    lbl_1_bss_14 = NULL;
    if (lbl_1_bss_C) {
        HuMemDirectFree(lbl_1_bss_C);
    }
    lbl_1_bss_C = NULL;
}

// Called during the page effect to place sparkle particles around the active page.
void fn_1_9718(void)
{
    Mtx objMtx;
    Mtx transMtx;
    MIRACLEBOOK_VEC_TABLE offsets = {{
    {0.0f, 0.0f, 0.0f},
    {40.0f, 10.0f, 0.0f},
    {-40.0f, 10.0f, 0.0f},
    {20.0f, -40.0f, 0.0f},
    {-20.0f, -40.0f, 0.0f},
}};
    HuVecF pos;
    HuVecF base;
    s32 i;

    for (i = 0; i < 5; i++) {
        Hu3DParManAttrReset(lbl_1_bss_68[i], HU3D_PARMAN_ATTR_TIMEUP);
    }
    if (lbl_1_bss_134 == 1) {
        Hu3DModelObjMtxGet(lbl_1_bss_90[0], lbl_1_data_758, objMtx);
    } else if (lbl_1_bss_134 == 2) {
        Hu3DModelObjMtxGet(lbl_1_bss_90[1], lbl_1_data_778, objMtx);
    }
    Hu3DMtxTransGet(objMtx, &base);
    if (base.z > (0.0f)) {
        for (i = 0; i < 5; i++) {
            PSMTXTrans(transMtx, offsets.entries[i].x, offsets.entries[i].y,
                offsets.entries[i].z);
            PSMTXConcat(objMtx, transMtx, transMtx);
            Hu3DMtxTransGet(transMtx, &pos);
            Hu3DParManPosSet(lbl_1_bss_68[i], pos.x, pos.y, pos.z);
            PSVECSubtract(&pos, &base, &pos);
            if (HuMag2Point3D(pos.x, pos.y, pos.z) <= (0.0)) {
                pos.x = pos.y = pos.z = (0.0f);
            } else {
                PSVECNormalize(&pos, &pos);
            }
            Hu3DParManVecSet(lbl_1_bss_68[i], pos.x, (-0.5f), pos.z);
        }
    } else {
        for (i = 0; i < 5; i++) {
            Hu3DParManAttrSet(lbl_1_bss_68[i], HU3D_PARMAN_ATTR_TIMEUP);
        }
    }
}

u32 lbl_1_data_6F4[21] = {
    MESSNUM(MESS_MBOOK_PAGE, 20), MESSNUM(MESS_MBOOK_PAGE, 0),
    MESSNUM(MESS_MBOOK_PAGE, 1), MESSNUM(MESS_MBOOK_PAGE, 2),
    MESSNUM(MESS_MBOOK_PAGE, 3), MESSNUM(MESS_MBOOK_PAGE, 4),
    MESSNUM(MESS_MBOOK_PAGE, 5), MESSNUM(MESS_MBOOK_PAGE, 6),
    MESSNUM(MESS_MBOOK_PAGE, 7), MESSNUM(MESS_MBOOK_PAGE, 8),
    MESSNUM(MESS_MBOOK_PAGE, 9), MESSNUM(MESS_MBOOK_PAGE, 10),
    MESSNUM(MESS_MBOOK_PAGE, 11), MESSNUM(MESS_MBOOK_PAGE, 12),
    MESSNUM(MESS_MBOOK_PAGE, 13), MESSNUM(MESS_MBOOK_PAGE, 14),
    MESSNUM(MESS_MBOOK_PAGE, 15), MESSNUM(MESS_MBOOK_PAGE, 16),
    MESSNUM(MESS_MBOOK_PAGE, 17), MESSNUM(MESS_MBOOK_PAGE, 18),
    MESSNUM(MESS_MBOOK_PAGE, 19),
};

u32 lbl_1_data_748[2] = {
    MESSNUM(MESS_MBOOK_PAGE, 21), MESSNUM(MESS_MBOOK_PAGE, 22),
};

u32 lbl_1_data_750[2] = {
    MESSNUM(MESS_SYS_GUIDE, 14), MESSNUM(MESS_SYS_GUIDE, 15),
};

char lbl_1_data_758[32] = "mgbook_siori005-end_shiori_root";

char lbl_1_data_778[33] = "mgbook_siori6-end_siori_hirahira";

s32 lbl_1_bss_224;

s32 lbl_1_bss_220;

s16 lbl_1_bss_21C;

s16 lbl_1_bss_21A;

s16 lbl_1_bss_218;

s16 lbl_1_bss_1FC[2][7];

u8 lbl_1_bss_1E8[20];

s32 lbl_1_bss_1E4;

s32 lbl_1_bss_1E0;

s32 lbl_1_bss_1DC;

s32 lbl_1_bss_1D8;

s32 lbl_1_bss_1D4;

f32 lbl_1_bss_1C8[3];

s32 lbl_1_bss_1C4;

s16 lbl_1_bss_1C0[2];

OM_CAMERA_VIEW lbl_1_bss_1A4;

s32 lbl_1_bss_1A0;

f32 lbl_1_bss_19C;

f32 lbl_1_bss_184[6];

s32 lbl_1_bss_180;

s32 lbl_1_bss_17C;

s16 lbl_1_bss_17A;

s16 lbl_1_bss_178;

s16 lbl_1_bss_174[2];

s16 lbl_1_bss_16C[2][2];

s16 lbl_1_bss_144[20];

s32 lbl_1_bss_140;

s32 lbl_1_bss_13C;

s32 lbl_1_bss_138;

s32 lbl_1_bss_134;

s32 lbl_1_bss_130;

f32 lbl_1_bss_120[4];

f32 lbl_1_bss_11C;

f32 lbl_1_bss_118;

s32 lbl_1_bss_114;

s32 lbl_1_bss_110;

s32 lbl_1_bss_10C;

s32 lbl_1_bss_B4[22];

s32 lbl_1_bss_B0;

s32 lbl_1_bss_AC;

s32 lbl_1_bss_A4[2];

s32 lbl_1_bss_A0;

s32 lbl_1_bss_98[2];

s16 lbl_1_bss_90[3];

s32 lbl_1_bss_8C;

f32 lbl_1_bss_88;

f32 lbl_1_bss_84;

HuVecF lbl_1_bss_78;

OMOBJMAN *lbl_1_bss_74;

s16 lbl_1_bss_68[5];

UNK_MIRACLEBOOK_HOOK lbl_1_bss_18[2];

ANIMDATA *lbl_1_bss_14;

ANIMDATA *lbl_1_bss_10;

void *lbl_1_bss_C;

OMOBJ *lbl_1_bss_8;
