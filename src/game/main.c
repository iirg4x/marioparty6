/* Starts the game systems and runs the input, update, and render loop at the configured retrace
 * pace. */
/* Prevent transitive inclusion of math.h; this file uses no math functions. */
#define _MATH_H

#include "game/main.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/charman.h"
#include "game/process.h"
#include "game/printfunc.h"
#include "game/perf.h"
#include "game/sprite.h"
#include "game/hu3d.h"
#include "game/init.h"
#include "game/sreset.h"
#include "game/saveload.h"
#include "game/gamemes.h"
#include "game/memory.h"
#include "game/wipe.h"
#include "game/frand.h"

#define RAND8_INITIAL_SEED 55789
#define RAND8_SEED_MULTIPLIER 1103515245
#define RAND8_SEED_INCREMENT 12345

extern OVLTBL _ovltbl[];

extern void HuMCSysInit(void);

static void LoadProcExec(void *unusedParam);
static void LoadProcWatch(void);

/* Incremented after each normal main-loop pass; game systems use it as a frame stamp. */
u32 GlobalCounter;
/* OS thread used for the asynchronous initialization hook. */
static OSThread *LoadThread;
/* Initialization function called by LoadProcExec. */
static void (*LoadProcHook)(void);
/* Heap block retained as the initialization thread's stack until completion. */
static u8 *LoadProcBuf;
/* The main loop waits while a disc error screen is active. */
BOOL HuDvdErrWait;
/* 0: normal game loop, 1: initialization hook pending or running, 2: hook finished and awaiting
 * cleanup. */
static s32 LoadProcMode;
/* Set while reset and disc-error handling must be suppressed, such as during save operations. */
BOOL HuSRDisableF;
/* Boot flow sets this when starting from an overlay or after opening setup; TRUE skips the initial
 * Nintendo-logo sequence. */
BOOL NintendoDispF;

/* Initializes the game once, then services input, processes, rendering, and callbacks while soft
 * reset and DVD-error handling are inactive; HuSysDoneRender applies retrace pacing. */
void main(void)
{
    s16 playerIndex;
    s32 retraceCount;
    s16 unusedValue = 0;

    HuSRDisableF = FALSE;
    HuDvdErrWait = 0;
    NintendoDispF = 0;
    HuSysInit(&GXNtsc480IntDf);
    HuPrcInit();
    HuPadInit();
    GWInit();
    pfInit();
    GlobalCounter = 0;
    HuDataInit();
    HuSprInit();
    Hu3DInit();
    SLSaveFlagSet(FALSE);
    HuPerfInit();
    HuPerfCreate("USR0", 0xFF, 0xFF, 0xFF, 0xFF);
    HuPerfCreate("USR1", 0, 0xFF, 0xFF, 0xFF);
    WipeInit(RenderMode);
    HuMCSysInit();

    for(playerIndex=0; playerIndex<GW_PLAYER_MAX; playerIndex++) {
        GwPlayerConf[playerIndex].charNo = CHARNO_NONE;
    }

    omMasterInit(0, _ovltbl, DLL_MAX, DLL_bootdll);
    VIWaitForRetrace();

    if(VIGetNextField() == 0) {
        OSReport("VI_FIELD_BELOW\n");
        VIWaitForRetrace();
    }
    OSReport("%s USA Mode\n", "Sep 25 2004");
    SLSaveFlagSet(FALSE);
    while(1) {
        retraceCount = VIGetRetraceCount();
        if (HuSoftResetButtonCheck() || HuDvdErrWait) {
            /* HuSoftResetButtonCheck can restart the system; otherwise, skip frame work while DVD
             * error handling owns the display and keep polling HuDvdErrWait. */
            continue;
        }
        HuPerfZero();
        HuPerfBegin(2);
        HuSysBeforeRender();
        HuPerfBegin(0);
        Hu3DPreProc();
        HuPadRead();
        pfClsScr();
        if(LoadProcMode == 0) {
            HuPrcCall(1); /* Run the game-process scheduler for one tick, advancing sleep timers and
                           * resuming runnable processes. */
            GameMesExec();
            HuPerfBegin(1);
            Hu3DExec();
        } else {
            HuPerfBegin(1);
            LoadProcWatch();
        }
        HuDvdErrorWatch();
        WipeExecAlways();
        HuPerfEnd(0);
        pfDrawFonts();
        HuPerfEnd(1);
        msmMusFdoutEnd();
        HuSysDoneRender(retraceCount);
        frand();
        rand8();
        HuPerfEnd(2);
        GlobalCounter++;
    }
}

/* Sets the retrace step used by frame-based animation and render pacing. */
void HuSysVWaitSet(s16 retraceStep)
{
    minimumVcount = retraceStep;
    minimumVcountf = retraceStep;
}

/* Returns the configured retrace step; the supplied previous wait value is not read. */
s16 HuSysVWaitGet(s16 previousVWait)
{
    return minimumVcount;
}

/* Seed advanced by rand8 for the game's shared byte-sized random value. */
s32 rnd_seed = RAND8_INITIAL_SEED;

/* Advances the shared seed and returns bits 16-23; the +1 offset is applied before selection. */
int rand8(void)
{
    rnd_seed = (rnd_seed * RAND8_SEED_MULTIPLIER) + RAND8_SEED_INCREMENT;
    return (u8)(((rnd_seed + 1) >> 16) & 0xFF);
}

/* Runs as the OS idle-function callback installed by HuLoadProcStart. */
static void LoadProcExec(void *unusedParam)
{
    void (*loadHook)(void) = LoadProcHook;

    loadHook();
    LoadProcMode = 2;
    OSReport("Init All Finished\n");
    OSCancelThread(LoadThread);
}

/* Reclaims the initialization stack and removes its loading wipe after the callback finishes. */
static void LoadProcWatch(void)
{
    if(LoadProcMode == 2) {
        HuMemDirectFree(LoadProcBuf);
        LoadProcMode = 0;
        WipeLoadKill();
    }
}

/* Starts an initialization callback on the OS idle thread while the main loop monitors
 * completion. */
void HuLoadProcStart(void (*initHook)(void))
{
    WipeLoadCreate();
    LoadProcBuf = HuMemDirectMalloc(HEAP_HEAP, 0x8000);
    LoadThread = OSSetIdleFunction(LoadProcExec, NULL, LoadProcBuf + 0x8000, 0x8000);
    LoadProcHook = initHook;
    LoadProcMode = 1;
}

/* Returns 0 when idle, 1 while initialization is pending or running, and 2 after the hook
 * finishes but before main-loop cleanup. */
s16 HuLoadProcModeGet(void)
{
    return LoadProcMode;
}
