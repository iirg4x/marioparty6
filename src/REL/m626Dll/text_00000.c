/* Overlay startup, camera setup, and the first minigame sequence callbacks. */
#include "dolphin/types.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/mg/seqman.h"
#include "game/charman.h"
#include "REL/m626Dll/setup.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

/* Runs the overlay constructors and starts Snow Brawl after the module loads. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

/* Runs the overlay destructors when the module is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

OMOBJMAN *lbl_1_bss_34;

extern void fn_1_3AC(s16 mode, s16 frameNo);
extern void fn_1_A08(OMOBJMAN *manager);
extern void fn_1_AD0(OMOBJMAN *manager, void *lightPosition, void *lightTarget, void *lightColor);
extern void fn_1_EEC(OMOBJMAN *manager, void *fogParameters);
extern void fn_1_11B8(OMOBJMAN *manager, s32 cameras, f32 fieldOfView, f32 nearPlane,
                       f32 farPlane, f32 shadowRadius, void *cameraView);
extern void fn_1_1F4C(s32 red, s32 green, s32 blue);
extern void fn_1_1F84(s32 cameras, void *motionData);
extern s32 fn_1_830(s32 index);
extern void fn_1_2608(OMOBJMAN *manager, void *audioConfig);
extern void fn_1_E268(OMOBJMAN *manager);
extern void fn_1_42DC(OMOBJMAN *manager);
extern void fn_1_8C50(OMOBJMAN *manager);
extern void fn_1_BA34(OMOBJMAN *manager);

void fn_1_2C4(short mode, short frameNo);
void fn_1_300(s16 mode, s16 frameNo);
void fn_1_374(short mode, short state);
void fn_1_3E4(s16 mode, s16 frameNo);
void fn_1_4A8(s16 mode, s16 frameNo);
void fn_1_52C(s16 player, s16 frameNo);
void fn_1_6EC(s16 player, s16 frameNo);
void fn_1_7C4(s16 mode, s16 frameNo);
void fn_1_7FC(s16 mode, s16 frameNo);

OM_CAMERA_VIEW lbl_1_data_0 = {{0.0f, -300.0f, -1200.0f}, {-30.0f, 0.0f, 0.0f}, 2000.0f};

Vec lbl_1_data_1C = {-400.0f, 800.0f, 400.0f};

Vec lbl_1_data_28 = {0.0f, 0.0f, 0.0f};

GXColor lbl_1_data_34 = {255, 255, 255, 255};

M626FogParameters lbl_1_data_38[3] = {
    {0, 1, 2, 20.0f, 6000.0f, {192, 192, 192, 0}},
    {1, 1, 0, 20.0f, 10000.0f, {192, 192, 192, 0}},
    {-1, 0, 0, 0.0f, 0.0f, {0, 0, 0, 0}}
};

M626FogParameters lbl_1_data_80[3] = {
    {0, 1, 2, 20.0f, 6000.0f, {64, 64, 64, 0}},
    {1, 1, 0, 20.0f, 10000.0f, {192, 192, 192, 0}},
    {-1, 0, 0, 0.0f, 0.0f, {0, 0, 0, 0}}
};

M626AudioParameters lbl_1_data_C8[2] = {
    {65535, 32, 96, 32, 64, 110, 2300.0f, 2000.0f, 1700.0f},
    {0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f}
};

MGSEQ_PARAM lbl_1_data_110 = {
    60, 0, fn_1_2C4, fn_1_300, fn_1_374, fn_1_3E4, fn_1_4A8,
    fn_1_52C, fn_1_6EC, fn_1_7C4, fn_1_7FC
};

s32 lbl_1_bss_30;

s32 lbl_1_bss_24[3];

s32 lbl_1_bss_20;

s16 lbl_1_bss_18[4];

s16 lbl_1_bss_10[4];

s16 lbl_1_bss_8[4];

s16 lbl_1_bss_0[4];

/* Starts Snow Brawl's object manager, sequence, camera, fog, audio and actors. */
void fn_1_A0(void)
{
    s32 index;
    s16 nightMode;

    lbl_1_bss_30 = 0;
    lbl_1_bss_24[0] = 0;
    lbl_1_bss_20 = -1;
    for (index = 0; index < 4; index++) {
        lbl_1_bss_18[index] = 0;
    }
    lbl_1_bss_34 = omInitObjMan(80, 8192);
    omGameSysInit(lbl_1_bss_34);
    MgSeqCreate(&lbl_1_data_110);
    MgSeqModeHookAdd(4, fn_1_3AC);
    fn_1_A08(lbl_1_bss_34);
    fn_1_AD0(lbl_1_bss_34, &lbl_1_data_1C, &lbl_1_data_28, &lbl_1_data_34);
    nightMode = GwMgNightF;
    if (nightMode != 1) {
        fn_1_EEC(lbl_1_bss_34, &lbl_1_data_38);
    } else {
        fn_1_EEC(lbl_1_bss_34, &lbl_1_data_80);
    }
    fn_1_11B8(lbl_1_bss_34, 1, 45.0f, 20.0f, 13000.0f,
              1200.0f, &lbl_1_data_0);
    fn_1_1F4C(0, 0, 128);
    fn_1_1F84(1, HuDataSelHeapReadNum(fn_1_830(0), HU_MEMNUM_OVL, HEAP_MODEL));
    fn_1_2608(lbl_1_bss_34, &lbl_1_data_C8);
    fn_1_E268(lbl_1_bss_34);
    fn_1_42DC(lbl_1_bss_34);
    fn_1_8C50(lbl_1_bss_34);
    fn_1_BA34(lbl_1_bss_34);
}

s32 fn_1_824(s16, s16);
/* Advances the sequence when the mode's completion check accepts its callback. */
void fn_1_2C4(short mode, short frameNo)
{
    if (!fn_1_824(mode, frameNo)) {
        MgSeqModeNext();
    }
}

/* Starts the minigame stream once the accepted sequence callback reaches frame 18. */
void fn_1_300(s16 mode, s16 frameNo) {
    if (fn_1_824(mode, frameNo) != 0) {
        return;
    }
    if (lbl_1_bss_20 != -1) {
        return;
    }
    if (frameNo < 18) {
        return;
    }
    lbl_1_bss_20 = HuAudBGMPlay(MSM_STREAM_MGTOUR_END);
}

/* Sequence callback that idles while its supplied mode and frame are not accepted. */
void fn_1_374(short mode, short state)
{
    if (fn_1_824(mode, state) != 0) {
        return;
    }
}

/* Mode-4 sequence hook; returns while its supplied mode and frame are not accepted. */
void fn_1_3AC(short mode, short state)
{
    if (fn_1_824(mode, state) != 0) {
        return;
    }
}

s32 fn_1_8B58(s32 index);

typedef struct M626CategoryCountPair {
    s32 values[2];
} M626CategoryCountPair;

static const M626CategoryCountPair lbl_1_rodata_20 = {{0, 0}};

/* Advances the sequence when either team has no active players remaining. */
void fn_1_3E4(s16 mode, s16 frameNo)
{
    M626CategoryCountPair categoryCounts;
    categoryCounts = lbl_1_rodata_20;

    if (fn_1_824(mode, frameNo) == 0) {
        s32 index;

        for (index = 0; index < 4; index++) {
            if (fn_1_8B58(index) == 0) {
                categoryCounts.values[lbl_1_bss_8[index]]++;
            }
        }
        if (categoryCounts.values[0] != 0 && categoryCounts.values[1] != 0) {
            return;
        }
        (void)MgSeqModeNext();
        return;
    }
}

/* Stops the minigame stream at the result callback's first frame. */
void fn_1_4A8(s16 mode, s16 frameNo)
{
    if (fn_1_824(mode, frameNo) == 0 && frameNo == 0) {
        if (lbl_1_bss_20 != -1) {
            HuAudSStreamFadeOut(lbl_1_bss_20, 100);
            lbl_1_bss_20 = -1;
        }
    }
}
