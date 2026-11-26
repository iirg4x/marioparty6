#include "dolphin/ai.h"
#include "dolphin/hw_regs.h"
#include "dolphin/os.h"

const char *__AIVersion = "<< Dolphin SDK - AI\trelease build: Apr  5 2004 04:15:02 (0x2301) >>";

static AISCallback __AIS_Callback = NULL;
static AIDCallback __AID_Callback = NULL;
static u8 *__CallbackStack;
static u8 *__OldStack;
static volatile s32 __AI_init_flag = FALSE;
static volatile s32 __AID_Active = FALSE;

static OSTime bound_32KHz;
static OSTime bound_48KHz;
static OSTime min_wait;
static OSTime max_wait;
static OSTime buffer;

static void __AISHandler(s16 interrupt, OSContext *context);
static void __AIDHandler(s16 interrupt, OSContext *context);
static void __AICallbackStackSwitch(AIDCallback cb);
static void __AI_SRC_INIT(void);

AIDCallback AIRegisterDMACallback(AIDCallback callback)
{
    s32 old;
    AIDCallback old_callback;

    old_callback = __AID_Callback;
    old = OSDisableInterrupts();
    __AID_Callback = callback;
    OSRestoreInterrupts(old);
    return old_callback;
}

void AIInitDMA(u32 start_addr, u32 length)
{
    s32 old;
    old = OSDisableInterrupts();
    __DSPRegs[24] = (u16)((__DSPRegs[24] & ~0x3FF) | (start_addr >> 16));
    __DSPRegs[25] = (u16)((__DSPRegs[25] & ~0xFFE0) | (0xffff & start_addr));
    __DSPRegs[27] = (u16)((__DSPRegs[27] & ~0x7FFF) | (u16)((length >> 5) & 0xFFFF));
    OSRestoreInterrupts(old);
}

void AIStartDMA()
{
    __DSPRegs[27] |= 0x8000;
}

void AIStopDMA(void)
{
    __DSPRegs[27] &= ~0x8000;
}

u32 AIGetDMAStartAddr(void)
{
    return (u32)((__DSPRegs[24] & 0x03ff) << 16) | (__DSPRegs[25] & 0xffe0);
}

AISCallback AIRegisterStreamCallback(AISCallback callback)
{
    AISCallback ret;
    s32 oldInts;

    ret = __AIS_Callback;
    oldInts = OSDisableInterrupts();
    __AIS_Callback = callback;
    OSRestoreInterrupts(oldInts);
    return ret;
}

void AIResetStreamSampleCount(void)
{
    __AIRegs[0] = (__AIRegs[0] & ~0x20) | 0x20;
}

void AISetStreamTrigger(u32 trigger)
{
    __AIRegs[3] = trigger;
}

void AISetStreamPlayState(u32 state)
{
    s32 old;
    u8 vol_right;
    u8 vol_left;

    if (state == AIGetStreamPlayState()) {
        return;
    }
    if ((AIGetStreamSampleRate() == 0U) && (state == 1)) {
        vol_right = AIGetStreamVolRight();
        vol_left = AIGetStreamVolLeft();
        AISetStreamVolRight(0);
        AISetStreamVolLeft(0);
        old = OSDisableInterrupts();
        __AI_SRC_INIT();
        __AIRegs[0] = (__AIRegs[0] & ~0x20) | 0x20;
        __AIRegs[0] = (__AIRegs[0] & ~1) | 1;
        OSRestoreInterrupts(old);
        AISetStreamVolLeft(vol_right);
        AISetStreamVolRight(vol_left);
    }
    else {
        __AIRegs[0] = (__AIRegs[0] & ~1) | state;
    }
}

u32 AIGetStreamPlayState()
{
    return __AIRegs[0] & 1;
}

void AISetDSPSampleRate(u32 rate)
{
    u32 play_state;
    s32 old;
    u8 vol_left;
    u8 vol_right;
    u32 afr_state;

    if (rate == AIGetDSPSampleRate()) {
        return;
    }

    __AIRegs[0] &= ~0x40;
    if (rate == 0) {
        vol_left = AIGetStreamVolLeft();
        vol_right = AIGetStreamVolRight();
        play_state = AIGetStreamPlayState();
        afr_state = AIGetStreamSampleRate();
        AISetStreamVolLeft(0);
        AISetStreamVolRight(0);
        old = OSDisableInterrupts();
        __AI_SRC_INIT();
        __AIRegs[0] = (__AIRegs[0] & ~0x20) | 0x20;
        __AIRegs[0] = (__AIRegs[0] & ~2) | (afr_state * 2);
        __AIRegs[0] = (__AIRegs[0] & ~1) | play_state;
        __AIRegs[0] |= 0x40;
        OSRestoreInterrupts(old);
        AISetStreamVolLeft(vol_left);
        AISetStreamVolRight(vol_right);
    }
}

u32 AIGetDSPSampleRate()
{
    return ((__AIRegs[0] >> 6) & 1) ^ 1;
}

static void __AI_set_stream_sample_rate(u32 rate)
{
    s32 old;
    s32 play_state;
    u8 vol_left;
    u8 vol_right;
    s32 dsp_src_state;

    if (rate == AIGetStreamSampleRate()) {
        return;
    }
    play_state = AIGetStreamPlayState();
    vol_left = AIGetStreamVolLeft();
    vol_right = AIGetStreamVolRight();
    AISetStreamVolRight(0);
    AISetStreamVolLeft(0);
    dsp_src_state = __AIRegs[0] & 0x40;
    __AIRegs[0] &= ~0x40;
    old = OSDisableInterrupts();
    __AI_SRC_INIT();
    __AIRegs[0] |= dsp_src_state;
    __AIRegs[0] = (__AIRegs[0] & ~0x20) | 0x20;
    __AIRegs[0] = (__AIRegs[0] & ~2) | (rate * 2);
    OSRestoreInterrupts(old);
    AISetStreamPlayState(play_state);
    AISetStreamVolLeft(vol_left);
    AISetStreamVolRight(vol_right);
}

u32 AIGetStreamSampleRate()
{
    return (__AIRegs[0] >> 1) & 1;
}

void AISetStreamVolLeft(u8 vol)
{
    __AIRegs[1] = (__AIRegs[1] & ~0xFF) | (vol & 0xFF);
}

u8 AIGetStreamVolLeft()
{
    return __AIRegs[1];
}

void AISetStreamVolRight(u8 vol)
{
    __AIRegs[1] = (__AIRegs[1] & ~0xFF00) | ((vol & 0xFF) << 8);
}

u8 AIGetStreamVolRight()
{
    return __AIRegs[1] >> 8;
}

void AIInit(u8 *stack)
{
    if (__AI_init_flag == TRUE) {
        return;
    }

    OSRegisterVersion(__AIVersion);

    bound_32KHz = OSNanosecondsToTicks(31524);
    bound_48KHz = OSNanosecondsToTicks(42024);
    min_wait = OSNanosecondsToTicks(42000);
    max_wait = OSNanosecondsToTicks(63000);
    buffer = OSNanosecondsToTicks(3000);

    AISetStreamVolRight(0);
    AISetStreamVolLeft(0);
    AISetStreamTrigger(0);
    AIResetStreamSampleCount();
    __AI_set_stream_sample_rate(1);
    AISetDSPSampleRate(0);
    __AIS_Callback = 0;
    __AID_Callback = 0;
    __CallbackStack = stack;
    __OSSetInterruptHandler(5, __AIDHandler);
    __OSUnmaskInterrupts(0x04000000);
    __OSSetInterruptHandler(8, __AISHandler);
    __OSUnmaskInterrupts(0x800000);
    __AI_init_flag = TRUE;
}

static void __AISHandler(s16 interrupt, OSContext *context)
{
    OSContext exceptionContext;
    __AIRegs[0] |= 8;
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(&exceptionContext);
    if (__AIS_Callback != NULL) {
        __AIS_Callback(__AIRegs[2]);
    }
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(context);
}

static void __AIDHandler(s16 interrupt, OSContext *context)
{
    OSContext exceptionContext;
    u16 tmp = __DSPRegs[5];
    __DSPRegs[5] = (tmp & ~0xA0) | 8;
    OSClearContext(&exceptionContext);
    OSSetCurrentContext(&exceptionContext);
    if (__AID_Callback && !__AID_Active) {
        __AID_Active = TRUE;
        if (__CallbackStack) {
            __AICallbackStackSwitch(__AID_Callback);
        }
        else {
            __AID_Callback();
        }
        __AID_Active = FALSE;
    }

    OSClearContext(&exceptionContext);
    OSSetCurrentContext(context);
}

// clang-format off
asm void __AICallbackStackSwitch(register AIDCallback cb) {
  // Allocate stack frame
  fralloc

  // Store current stack
  lis r5, __OldStack@ha
  addi r5, r5, __OldStack@l
  stw r1, 0(r5)

  // Load stack for callback
  lis r5, __CallbackStack@ha
  addi r5, r5, __CallbackStack@l
  lwz r1,0(r5)

  // Move stack down 8 bytes
  subi r1, r1, 8
  // Call callback
  mtlr cb
  blrl

  // Restore old stack
  lis r5, __OldStack @ha
  addi r5, r5, __OldStack@l
  lwz r1,0(r5)

  // Free stack frame
  frfree

  blr
}
// clang-format on

static void __AI_SRC_INIT(void)
{
    OSTime rising_32khz = 0;
    OSTime rising_48khz = 0;
    OSTime diff = 0;
    OSTime t1 = 0;
    OSTime temp = 0;
    u32 temp0 = 0;
    u32 temp1 = 0;
    u32 done = 0;
    u32 walking = 0;
    u32 volume = 0;
    u32 Init_Cnt = 0;

    walking = 0;
    Init_Cnt = 0;
    temp = 0;

    while (!done) {
        __AIRegs[0] = (__AIRegs[0] & ~0x20) | 0x20;
        __AIRegs[0] &= ~2;
        __AIRegs[0] = (__AIRegs[0] & ~1) | 1;

        temp0 = __AIRegs[2];

        while (temp0 == __AIRegs[2])
            ;
        rising_32khz = OSGetTime();

        __AIRegs[0] = (__AIRegs[0] & ~2) | 2;
        __AIRegs[0] = (__AIRegs[0] & ~1) | 1;

        temp1 = __AIRegs[2];
        while (temp1 == __AIRegs[2])
            ;

        rising_48khz = OSGetTime();

        diff = rising_48khz - rising_32khz;
        __AIRegs[0] &= ~2;
        __AIRegs[0] &= ~1;

        if (diff < (bound_32KHz - buffer)) {
            temp = min_wait;
            done = 1;
            ++Init_Cnt;
        }
        else if (diff >= (bound_32KHz + buffer) && diff < (bound_48KHz - buffer)) {
            temp = max_wait;
            done = 1;
            ++Init_Cnt;
        }
        else {
            done = 0;
            walking = 1;
            ++Init_Cnt;
        }
    }

    while ((rising_48khz + temp) > OSGetTime())
        ;
}
