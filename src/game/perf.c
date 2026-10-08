/* Tracks CPU and draw timings for the game's render loop. */
#include "game/perf.h"

#define PERF_TOKEN_BEGIN 0xFF00
#define PERF_TOKEN_END 0xFF01

typedef struct PerfWork_s {
    GXColor color; /* Display color assigned to this performance counter. */
    OSTime elapsedTicks; /* Counter duration captured before its stopwatch is reset, in OS ticks. */
    OSTime frameStartTicks; /* Frame elapsed ticks at counter start; this file does not read it
                             * back. */
    OSStopwatch stopwatch; /* Stopwatch that accumulates this counter's elapsed time. */
    s16 inUse; /* TRUE when this slot has been assigned to a named counter. */
} PERFWORK;

static void DSCallbackFunc(u16 token);

static OSStopwatch Ssw;
static PERFWORK perf[HUPERF_MAX];

static u32 met0;
static u32 met1;
static u32 vcheck;
static u32 vmiss;
static u32 vstall;
static u32 cp_req;
static u32 tc_req;
static u32 cpu_rd_req;
static u32 cpu_wr_req;
static u32 dsp_req;
static u32 io_req;
static u32 vi_req;
static u32 pe_req;
static u32 rf_req;
static u32 fi_req;
static u32 top_pixels_in;
static u32 top_pixels_out;
static u32 bot_pixels_in;
static u32 bot_pixels_out;
static u32 clr_pixels_in;
static u32 total_copy_clks;
static s16 tokenEndF;
/* Zero-initialized and never enabled here, so the guarded GX metric reads are skipped. */
static u8 metf;

/* Called once during game startup to reserve CPU and draw counters and install GX token timing. */
void HuPerfInit(void) {
    s32 perfIndex;

    for (perfIndex=0; perfIndex<HUPERF_MAX; perfIndex++) {
        perf[perfIndex].inUse = FALSE;
    }
    HuPerfCreate("CPU", 0, 255, 0, 255);
    HuPerfCreate("DRAW", 255, 0, 0, 255);
    GXSetDrawSyncCallback(DSCallbackFunc);
    total_copy_clks = 0;
}

/* Called during startup or setup to allocate a named counter and its display color. */
s32 HuPerfCreate(char *name, u8 red, u8 green, u8 blue, u8 alpha) {
    s32 perfIndex;

    for (perfIndex=0; perfIndex<HUPERF_MAX; perfIndex++) {
        if (perf[perfIndex].inUse == FALSE) {
            break;
        }
    }
    if (perfIndex == HUPERF_MAX) {
        return -1;
    }
    OSInitStopwatch(&perf[perfIndex].stopwatch, name);
    perf[perfIndex].elapsedTicks = 0;
    perf[perfIndex].inUse = TRUE;
    perf[perfIndex].color.r = red;
    perf[perfIndex].color.g = green;
    perf[perfIndex].color.b = blue;
    perf[perfIndex].color.a = alpha;
    return perfIndex;
}

/* Called once per frame from the game loop to restart the shared elapsed-time stopwatch. */
void HuPerfZero(void) {
    /* Stop and reinitialize so each frame starts with a fresh total. */
    OSStopStopwatch(&Ssw);
    OSResetStopwatch(&Ssw);
    OSStartStopwatch(&Ssw);
}

/* Called around CPU or user work in the game loop; draw timing begins when its GX token arrives. */
void HuPerfBegin(s32 perfId)
{
    if (perfId == HUPERF_DRAW) {
        /* The draw-sync callback starts the draw stopwatch after the GPU reaches this token. */
        GXSetDrawSync(PERF_TOKEN_BEGIN);
        return;
    }
    OSStartStopwatch(&perf[perfId].stopwatch);
    perf[perfId].frameStartTicks = OSCheckStopwatch(&Ssw);
}

/* Called after CPU, user, or draw work; the draw interval ends when its GX token arrives. */
void HuPerfEnd(s32 perfId)
{
    if (perfId == HUPERF_DRAW) {
        /* The draw-sync callback records the end after the GPU reaches this token. */
        GXSetDrawSync(PERF_TOKEN_END);
        return;
    }
    perf[perfId].elapsedTicks = OSCheckStopwatch(&perf[perfId].stopwatch);
    OSStopStopwatch(&perf[perfId].stopwatch);
    /* Reset clears the stopwatch total after elapsedTicks has saved the measurement. */
    OSResetStopwatch(&perf[perfId].stopwatch);
}

/* GX calls this from its draw-sync token interrupt to bracket GPU draw work and optionally sample
 * counters. */
static void DSCallbackFunc(u16 token) {
    switch (token) {
        case PERF_TOKEN_BEGIN:
            OSStartStopwatch(&perf[HUPERF_DRAW].stopwatch);
            perf[HUPERF_DRAW].frameStartTicks = OSCheckStopwatch(&Ssw);
            tokenEndF = 0;
            if (metf == 1) {
                GXClearGPMetric();
                GXClearVCacheMetric();
                GXClearMemMetric();
                GXClearPixMetric();
            }
            break;

        case PERF_TOKEN_END:
            if (tokenEndF == 0) {
                /* Ignore duplicate end tokens until a new draw-begin token resets this guard. */
                tokenEndF = 1;
                perf[HUPERF_DRAW].elapsedTicks = OSCheckStopwatch(&perf[HUPERF_DRAW].stopwatch);
                OSStopStopwatch(&perf[HUPERF_DRAW].stopwatch);
                OSResetStopwatch(&perf[HUPERF_DRAW].stopwatch);
                if (metf == 1) {
                    GXReadGPMetric(&met0, &met1);
                    GXReadVCacheMetric(&vcheck, &vmiss, &vstall);
                    GXReadMemMetric(&cp_req, &tc_req, &cpu_rd_req, &cpu_wr_req, &dsp_req, &io_req,
                                    &vi_req, &pe_req, &rf_req, &fi_req);
                    GXReadPixMetric(&top_pixels_in, &top_pixels_out, &bot_pixels_in,
                                    &bot_pixels_out, &clr_pixels_in, &total_copy_clks);
                }
            }
            break;
    }
}
