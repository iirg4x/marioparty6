/* Starts and stops the module, dispatches its minigame phases, and supplies vector math. */
#include "REL/m613Dll/types.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/hu3d.h"
#include "game/gamework.h"
#include "game/frand.h"
#include "game/wipe.h"
#include "game/mg/seqman.h"
#include "game/board/object.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

s32 lbl_1_bss_1260;
s16 lbl_1_bss_125E;
s16 lbl_1_bss_125C;
HUPROCESS *lbl_1_bss_1258;

extern s16 GwMgNightF;
extern void fn_1_F048(void);
extern void fn_1_11F8(void);
extern void fn_1_4208(void);
extern void fn_1_F3C4(void);

extern u8 fn_1_F23C(s16 value);
extern u8 fn_1_4D38(s16 value);
extern void fn_1_F2A0(void);
extern void fn_1_5038(void);
extern void fn_1_18FC(void);
extern void fn_1_F850(void);

extern void fn_1_F568(void);
extern void fn_1_5058(void);
extern void fn_1_C2AC(void);

extern s32 fn_1_C850(s16 value);
extern void fn_1_D0C8(void);

int _prolog(void);
void _epilog(void);
void fn_1_10814(void);
void fn_1_10890(s16 mode, s16 frameNo);
void fn_1_109BC(s16 unusedMode, s16 value);
void fn_1_10A40(s16 mode, s16 frameNo);
void fn_1_10AB0(s16 mode, s16 frameNo);
void fn_1_10AF0(s16 mode, s16 frameNo);
void fn_1_10B2C(s16 unusedMode, s16 frameNo);
void fn_1_10B88(s16 unusedMode, s16 value);
void fn_1_10BE0(s16 mode, s16 frameNo);
void fn_1_10C14(s16 mode, s16 frameNo);
void fn_1_10C48(s16 mode, s16 frameNo);
s32 fn_1_10C5C(Point3d *direction, Point3d *unitDirection);
s32 fn_1_10DB4(f32 numerator, f32 denominator, f32 *ratio);
f64 fn_1_10E3C(f64 x, f64 y);

static const Point3d lbl_1_rodata_1B8 = {0.0f,5000.0f,1.0f};
static const Point3d lbl_1_rodata_1C4 = {0.0f,1.0f,0.0f};
static const Point3d lbl_1_rodata_1D0 = {0.0f,0.0f,0.0f};

MGSEQ_PARAM lbl_1_data_270 = {
    30, 0, fn_1_10890, fn_1_109BC, fn_1_10A40, fn_1_10AF0, fn_1_10B2C, fn_1_10B88,
    fn_1_10BE0, fn_1_10C14, fn_1_10C48
};

/* Called by the module loader to run constructors and start this minigame. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_10814();
    return 0;
}

/* Called by the module loader to run constructors' paired destructors on unload. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Called by the module prolog to start the object manager and configure sequence phases. */
void fn_1_10814(void) {
    lbl_1_bss_1258 = omInitObjMan(300, 4096);
    omGameSysInit(lbl_1_bss_1258);
    MgSeqCreatePrio(&lbl_1_data_270, 2000);
    MgSeqModeHookAdd(4, fn_1_10AB0);
    MgSeqModeDelaySet(2, -1);
    MgSeqModeDelaySet(7, -1);
}

/* Initialization callback: set up the shadow and board, use a black background in both day and
 * night modes, then request the fade-in phase. */
void fn_1_10890(s16 mode, s16 frameNo)
{
    Point3d shadowPosition;
    Point3d shadowUp;
    Point3d shadowTarget;
    s16 nightFlag;

    lbl_1_bss_125E = 0;
    fn_1_F048();
    shadowPosition = lbl_1_rodata_1B8;
    shadowUp = lbl_1_rodata_1C4;
    shadowTarget = lbl_1_rodata_1D0;
    Hu3DShadowCreate(30.0f, 20.0f, 5000.0f);
    Hu3DShadowTPLvlSet(0.5f);
    Hu3DShadowPosSet(&shadowPosition, &shadowUp, &shadowTarget);
    fn_1_11F8();
    fn_1_4208();
    fn_1_F3C4();
    nightFlag = GwMgNightF;
    if (nightFlag == 0) {
        Hu3DBGColorSet(0, 0, 0);
    } else {
        Hu3DBGColorSet(0, 0, 0);
    }
    MgSeqModeNext();
}

/* Fade-in callback: update the opening scene and advance when frame 230 is reached and every
 * player has completed its intro states. */
void fn_1_109BC(s16 unusedMode, s16 value) {
    u8 passedRange;
    u8 playersReady;

    lbl_1_bss_125E = 1;
    passedRange = fn_1_F23C(value);
    playersReady = fn_1_4D38(value);
    fn_1_18FC();
    fn_1_F850();
    if (passedRange && playersReady) {
        MgSeqModeNext();
    }
}

/* Start-message callback: start the music on its first frame and update the player scene. */
void fn_1_10A40(s16 mode, s16 frameNo) {
    lbl_1_bss_125E = 2;
    lbl_1_bss_125C = 0;
    if (frameNo == 0) {
        lbl_1_bss_1260 = HuAudBGMPlay(MSM_STREAM_CAPSULE_WANWAN);
    }
    fn_1_F2A0();
    fn_1_5038();
    fn_1_18FC();
    fn_1_F850();
}

/* Before gameplay begins, set the module update state and copy leading progress to every
 * baseline. */
void fn_1_10AB0(s16 mode, s16 frameNo) {
    lbl_1_bss_125E = 3;
    lbl_1_bss_125C = 0;
    fn_1_F568();
}

/* Main gameplay callback: read controls and update contacts, glass, and progress each frame. */
void fn_1_10AF0(s16 mode, s16 frameNo) {
    lbl_1_bss_125E = 3;
    fn_1_F2A0();
    fn_1_5058();
    fn_1_18FC();
    fn_1_F850();
}

/* Finish-message callback: switch players to idle, stop movement sounds, fade the background
 * stream on entry, and update progress animation. */
void fn_1_10B2C(s16 unusedMode, s16 frameNo) {
    lbl_1_bss_125E = 4;
    fn_1_F2A0();
    fn_1_C2AC();
    if (frameNo == 0) {
        HuAudSStreamFadeOut(lbl_1_bss_1260, 100);
    }
    fn_1_F850();
}

/* Result callback: advance when the result check succeeds and keep progress motion current. */
void fn_1_10B88(s16 unusedMode, s16 value) {
    s32 result;

    lbl_1_bss_125E = 5;
    result = fn_1_C850(value);
    if ((u8)result != 0) {
        MgSeqModeNext();
    }
    fn_1_F850();
}

/* Winner-phase callback: update the final player scene and progress animation. */
void fn_1_10BE0(s16 mode, s16 frameNo)
{
    lbl_1_bss_125E = 6;
    fn_1_D0C8();
    fn_1_F850();
}

/* Fade-out callback: update the final player scene and progress animation. */
void fn_1_10C14(s16 mode, s16 frameNo) {
    lbl_1_bss_125E = 7;
    fn_1_D0C8();
    fn_1_F850();
}

/* Terminal-phase callback: mark the final sequence state. */
void fn_1_10C48(s16 mode, s16 frameNo) {
    lbl_1_bss_125E = 8;
}

/* Normalizes a direction for bitmap and particle motion. Near-zero inputs get a random unit
 * fallback and return zero; frandmod(1) consumes a draw but forces fallback Y to -0.01 before
 * normalization. */
s32 fn_1_10C5C(Point3d *direction, Point3d *unitDirection) {
    f32 fallbackY;

    if (PSVECSquareMag(direction) <= 0.001) {
        unitDirection->x = 0.01f * ((f32) frandmod(20) - 10.0f);
        unitDirection->z = 0.01f * ((f32) frandmod(20) - 10.0f);
        if (frandmod(1) != 0) {
            fallbackY = 0.01f;
        } else {
            fallbackY = -0.01f;
        }
        unitDirection->y = fallbackY;
        PSVECNormalize(unitDirection, unitDirection);
        return 0;
    }
    PSVECNormalize(direction, unitDirection);
    return 1;
}

/* Called by particle motion to divide safely, returning zero when either operand is near zero. */
s32 fn_1_10DB4(f32 numerator, f32 denominator, f32 *ratio) {
    if (((-0.000001 <= numerator) && (numerator <= 0.000001)) ||
        ((-0.000001 <= denominator) && (denominator <= 0.000001))) {
        *ratio = 0.0f;
        return 0;
    }
    *ratio = numerator / denominator;
    return 1;
}

/* Called by CPU steering to map two direction components to a heading in degrees. */
f64 fn_1_10E3C(f64 x, f64 y) {
    f64 angle;

    angle = acos(x / sqrtf((x * x) + (y * y)));
    angle = 180.0 * (angle / 3.141592653589793);
    if (y < 0.0) {
        angle = 360.0 - angle;
    }
    return angle;
}
