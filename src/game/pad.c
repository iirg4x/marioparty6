/* Controller input sampling, directional input conversion, and rumble control. */
/* This file does not use the inline sqrt helpers pulled in by dolphin.h. */
#define _MATH_H
#include "dolphin.h"
#include "humath.h"
#include "game/main.h"
#include "game/pad.h"

/* The mic subsystem uses this retrace call to maintain its periodic wake alarm. */
extern void HuMCPeriodicProc(void);

typedef struct PadRumble_s {
    s16 maxTime;      // Elapsed-retrace cutoff; zero disables the pattern, otherwise the motor is
                      // hard-stopped after the counter advances past this value.
    s16 offTime;      // Retrace phase at which the motor stops in each repeating cycle; this is the
                      // rumbling portion of the cycle.
    s16 onTime;       // Remaining retrace ticks in each repeating cycle after offTime; the motor
                      // stays stopped during this portion.
    s16 time;          // Elapsed retrace ticks for the current rumble pattern.
    s16 numRumble;     // Number of initial post-retrace callbacks remaining before the repeating
                       // rumble cycle.
} RUMBLEDATA;

static void PadReadVSync(u32 retraceCount);
static void PadADConv(s16 pad, PADStatus *status);

static u32 GlobalCounterOld = -1;

static int padStatErrOld[4];
static RUMBLEDATA rumbleData[4];
float lbl_8024C7D8[4];
float lbl_8024C7E8[4];
float lbl_8024C7F8[4];
float lbl_8024C808[4];
float lbl_8024C818[4];
float lbl_8024C828[4];

u16 lbl_802C0010[4];
u16 lbl_802C0008[4];
u16 lbl_802C0000[4];
s8 lbl_802BFFFC[4];
s8 lbl_802BFFF8[4];
s8 lbl_802BFFF4[4];
s8 lbl_802BFFF0[4];
u8 lbl_802BFFEC[4];
u8 lbl_802BFFE8[4];
u8 lbl_802BFFE4[4];
u8 lbl_802BFFE0[4];
u8 lbl_802BFFDC[4];
s8 lbl_802BFFD8[4];
u16 lbl_802BFFD0[4];
u16 lbl_802BFFC8[4];
static u16 _PadRepCnt[4];
static s8 _PadStkX[4];
static s8 _PadStkY[4];
static s8 _PadSubStkX[4];
static s8 _PadSubStkY[4];
static u8 _PadTrigL[4];
static u8 _PadTrigR[4];
static u8 _PadDStk[4];
static u8 _PadDStkRep[4];
static u8 _PadDStkDown[4];
static u8 _PadDStkRepCnt[4];
static u8 _PadDStkRepOld[4];
static s8 _PadErr[4];
static u32 RumbleBit;
static s16 RumbleCounter;
s32 lbl_802BFF84;
u16 lbl_802BFF80;

static u32 chanTbl[4] = { PAD_CHAN0_BIT, PAD_CHAN1_BIT, PAD_CHAN2_BIT, PAD_CHAN3_BIT };

extern int HuDvdErrWait;

/* Called during game startup to initialize controller sampling and disable scheduled rumble. */
void fn_80005C98(void)
{
    int padIndex;
    BOOL interruptsEnabled;
    PADSetSpec(PAD_SPEC_5);
    PADInit();
    SISetSamplingRate(0);
    interruptsEnabled = OSDisableInterrupts();
    VISetPostRetraceCallback(PadReadVSync);
    OSRestoreInterrupts(interruptsEnabled);
    for(padIndex=0; padIndex<4; padIndex++) {
        padStatErrOld[padIndex] = PAD_ERR_NOT_READY;
    }
    VIWaitForRetrace();
    VIWaitForRetrace();
    fn_80005D90();
    for(padIndex=0; padIndex<4; padIndex++) {
        if(_PadErr[padIndex] == PAD_ERR_NONE) {
            PADControlMotor(padIndex, PAD_MOTOR_STOP_HARD);
        }
        rumbleData[padIndex].maxTime = 0;
        _PadRepCnt[padIndex] = 0;
    }
    /* D-pad button bits are hidden from the public button arrays by default. */
    lbl_802BFF80 = (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT | PAD_BUTTON_UP | PAD_BUTTON_DOWN);
}

/* Copies the stored button, stick, trigger, D-pad, and error snapshots into their public arrays;
 * copies button-down edges once and then clears their accumulator. */
void fn_80005D90(void)
{
    s16 padIndex;
    for(padIndex=0; padIndex<4; padIndex++) {
        lbl_802C0010[padIndex] = lbl_802BFFD0[padIndex] & ~lbl_802BFF80;
        lbl_802C0008[padIndex] = lbl_802BFFC8[padIndex] & ~lbl_802BFF80;
        lbl_802BFFFC[padIndex] = _PadStkX[padIndex];
        lbl_802BFFF8[padIndex] = _PadStkY[padIndex];
        lbl_802BFFF4[padIndex] = _PadSubStkX[padIndex];
        lbl_802BFFF0[padIndex] = _PadSubStkY[padIndex];
        lbl_802BFFEC[padIndex] = _PadTrigL[padIndex];
        lbl_802BFFE8[padIndex] = _PadTrigR[padIndex];
        lbl_802BFFE4[padIndex] = _PadDStk[padIndex];
        lbl_802BFFE0[padIndex] = _PadDStkRep[padIndex];
        lbl_802BFFDC[padIndex] = _PadDStkDown[padIndex];
        lbl_802BFFD8[padIndex] = _PadErr[padIndex];
        
        /* Button-down edges are consumed when this public input snapshot is read. */
        lbl_802BFFC8[padIndex] = 0;
    }
}

/* VI post-retrace callback: when no disc error is active, samples controllers, updates input
 * repeats and rumble, and resets channels; always runs system callbacks and advances VCounter. */
static void PadReadVSync(u32 retraceCount)
{
    u32 channelsToReset;
    s16 padIndex;
    PADStatus padStatuses[4];
    if(!HuDvdErrWait) {
        RumbleBit = PADRead(padStatuses);
        PADClampCircle(padStatuses);
        channelsToReset = 0;
        if(GlobalCounterOld == GlobalCounter) {
             RumbleCounter++;
        } else {
            RumbleCounter = 0;
            GlobalCounterOld = GlobalCounter;
        }
        for(padIndex=0; padIndex<4; padIndex++) {
            PADStatus *padStatus = &padStatuses[padIndex];
            RUMBLEDATA *rumbleState = &rumbleData[padIndex];
            if(padStatErrOld[padIndex] && padStatus->err == PAD_ERR_NONE) {
                PADControlMotor(padIndex, PAD_MOTOR_STOP_HARD);
                rumbleState->maxTime = 0;
            }
            padStatErrOld[padIndex] = padStatus->err;
            if(padStatus->err != PAD_ERR_NONE) {
                _PadErr[padIndex] = padStatus->err;
                if(padStatus->err != PAD_ERR_TRANSFER && padStatus->err != PAD_ERR_NOT_READY) {
                    channelsToReset |= chanTbl[padIndex];
                }
                if(padStatus->err == PAD_ERR_TRANSFER) {
                    _PadErr[padIndex] = 0;
                    lbl_802BFFC8[padIndex] = _PadDStkDown[padIndex] = 0;
                    continue;
                }
                /* Treat this receiver signature as recoverable and request a channel reset. */
                if(padStatus->err == PAD_ERR_NO_CONTROLLER && SIProbe(padIndex) == SI_GC_RECEIVER) {
                    _PadErr[padIndex] = 0;
                    channelsToReset |= chanTbl[padIndex];
                }
                lbl_802BFFC8[padIndex] = lbl_802BFFD0[padIndex] = _PadStkX[padIndex] =
                    _PadStkY[padIndex] = _PadSubStkX[padIndex] = _PadSubStkY[padIndex] =
                        _PadTrigL[padIndex] = _PadTrigR[padIndex] = _PadDStkRep[padIndex] =
                            _PadDStk[padIndex] = _PadDStkDown[padIndex] = lbl_802C0000[padIndex] =
                                0;
            } else {
                u16 buttons = padStatus->button & ~lbl_802BFF80;
                if(padStatus->triggerLeft > 105.0) {
                    buttons |= 0x4000;
                }
                if(padStatus->triggerRight > 105.0) {
                    buttons |= 0x2000;
                }
                if(buttons && lbl_802BFFD0[padIndex] == buttons) {
                    if(_PadRepCnt[padIndex] > 20) {
                        lbl_802C0000[padIndex] = buttons;
                    } else {
                        /* Hold repeat visibility until the retrace sampling interval advances. */
                        if(RumbleCounter == 0) {
                            lbl_802C0000[padIndex] = 0;
                        }
                        _PadRepCnt[padIndex]++;
                    }
                } else {
                    _PadRepCnt[padIndex] = 0;
                    lbl_802C0000[padIndex] = buttons;
                }
                PadADConv(padIndex, padStatus);
                /* Record only bits newly present in this sample, using PADButtonDown's mask. */
                lbl_802BFFC8[padIndex] |= PADButtonDown(lbl_802BFFD0[padIndex], buttons);
                
                if(RumbleCounter == 0 || RumbleCounter > 3) {
                    lbl_802BFFD0[padIndex] = buttons;
                    _PadStkX[padIndex] = padStatus->stickX;
                    _PadStkY[padIndex] = padStatus->stickY;
                    _PadSubStkX[padIndex] = padStatus->substickX;
                    _PadSubStkY[padIndex] = padStatus->substickY;
                    _PadTrigL[padIndex] = padStatus->triggerLeft;
                    _PadTrigR[padIndex] = padStatus->triggerRight;
                    RumbleCounter = 0;
                } else {
                    /* For intermediate retraces, ORs button, stick, and trigger bytes into the
                     * stored values until the next sampling boundary. */
                    lbl_802BFFD0[padIndex] |= buttons;
                    _PadStkX[padIndex] |= padStatus->stickX;
                    _PadStkY[padIndex] |= padStatus->stickY;
                    _PadSubStkX[padIndex] |= padStatus->substickX;
                    _PadSubStkY[padIndex] |= padStatus->substickY;
                    _PadTrigL[padIndex] |= padStatus->triggerLeft;
                    _PadTrigR[padIndex] |= padStatus->triggerRight;
                }
                lbl_8024C7D8[padIndex] = (float)_PadStkX[padIndex]*(1.0/56.0);
                lbl_8024C7E8[padIndex] = (float)_PadStkY[padIndex]*(1.0/56.0);
                lbl_8024C7F8[padIndex] = (float)_PadSubStkX[padIndex]*(1.0/44.0);
                lbl_8024C808[padIndex] = (float)_PadSubStkY[padIndex]*(1.0/44.0);
                /* Rewrites the same normalized X sub-stick value after the Y assignment. */
                lbl_8024C7F8[padIndex] = (float)_PadSubStkX[padIndex]*(1.0/44.0);
                lbl_8024C818[padIndex] = (float)_PadTrigL[padIndex]*(1.0/150.0);
                lbl_8024C828[padIndex] = (float)_PadTrigR[padIndex]*(1.0/150.0);
                _PadErr[padIndex] = padStatus->err;
                if(rumbleState->maxTime) {
                    if(rumbleState->numRumble) {
                        if(rumbleState->time == 0) {
                            PADControlMotor(padIndex, PAD_MOTOR_RUMBLE);
                        }
                        rumbleState->numRumble--;
                    } else {
                        s16 rumblePhase =
                            rumbleState->time % (rumbleState->offTime + rumbleState->onTime);
                        if(rumblePhase == 0) {
                            PADControlMotor(padIndex, PAD_MOTOR_RUMBLE);
                        } else {
                            if(rumblePhase == rumbleState->offTime) {
                                PADControlMotor(padIndex, PAD_MOTOR_STOP);
                            }
                        }
                    }
                    rumbleState->time++;
                    if(rumbleState->time > rumbleState->maxTime) {
                        PADControlMotor(padIndex, PAD_MOTOR_STOP_HARD);
                        rumbleState->maxTime = 0;
                    }
                }
            }
        }
        if(channelsToReset) {
            PADReset(channelsToReset);
        }
    }
    msmSysRegularProc();
    HuMCPeriodicProc();
    lbl_802BFF84++;
}

/* Converts the two analog sticks into thresholded D-pad input and repeat state. */
static void PadADConv(s16 padIndex, PADStatus *padStatus)
{
    float combinedStickX, combinedStickY;
    float mainStickX, mainStickY;
    float subStickX, subStickY;
    float absStickX, absStickY;
    u8 previousDpad;
    s16 unusedZero, secondUnusedZero;
    /* These zeroed locals are retained even though the routine never reads them. */
    unusedZero = 0;
    secondUnusedZero = 0;
    mainStickX = padStatus->stickX/56.0f;
    mainStickY = padStatus->stickY/56.0f;
    subStickX = padStatus->substickX/44.0f;
    subStickY = padStatus->substickY/44.0f;
    /* For each axis, averages both values when both exceed 0.2, uses the main stick when only it
     * exceeds 0.2, and otherwise uses the sub-stick, even if neither exceeds 0.2. */
    if((mainStickX*mainStickX) > (0.2f*0.2f) && (subStickX*subStickX) > (0.2f*0.2f)) {
        combinedStickX = (mainStickX+subStickX)/2;
    } else if((mainStickX*mainStickX) > (0.2f*0.2f)) {
        combinedStickX = mainStickX;
    } else {
        combinedStickX = subStickX;
    }
    if((mainStickY*mainStickY) > (0.2f*0.2f) && (subStickY*subStickY) > (0.2f*0.2f)) {
        combinedStickY = (mainStickY+subStickY)/2;
    } else if((mainStickY*mainStickY) > (0.2f*0.2f)) {
        combinedStickY = mainStickY;
    } else {
        combinedStickY = subStickY;
    }
    absStickX = ((combinedStickX < 0) ? -combinedStickX : combinedStickX);
    absStickY = ((combinedStickY < 0) ? -combinedStickY : combinedStickY);
    previousDpad = _PadDStk[padIndex];
    _PadDStk[padIndex] = 0;
    if(absStickY > 0.3f) {
        if(combinedStickY > 0) {
            _PadDStk[padIndex] |= PAD_BUTTON_UP;
        } else {
            _PadDStk[padIndex] |= PAD_BUTTON_DOWN;
        }
    }
    if(absStickX > 0.4f) {
        if(combinedStickX < 0) {
            _PadDStk[padIndex] |= PAD_BUTTON_LEFT;
        } else {
            _PadDStk[padIndex] |= PAD_BUTTON_RIGHT;
        }
    }
    if(absStickX+absStickY < 0.3f) {
        _PadDStkRepOld[padIndex] = 0;
    }
    /* A changed direction sets a 20-callback delay and a continuing direction sets a two-callback
     * delay; during countdown, the repeat value is cleared when RumbleCounter is zero. */
    if(_PadDStkRepCnt[padIndex]) {
        _PadDStkRepCnt[padIndex]--;
        if(absStickX+absStickY < 0.3f) {
            _PadDStkRepCnt[padIndex] = 0;
        }
        if(RumbleCounter == 0) {
            _PadDStkRep[padIndex] = 0;
        }
    } else {
        _PadDStkRep[padIndex] = _PadDStk[padIndex];
        if(_PadDStkRep[padIndex]) {
            if(_PadDStkRepOld[padIndex] == _PadDStkRep[padIndex]) {
                _PadDStkRepCnt[padIndex] = 2;
            } else {
                _PadDStkRepCnt[padIndex] = 20;
            }
            _PadDStkRepOld[padIndex] = _PadDStkRep[padIndex];
        }
    }
    _PadDStkDown[padIndex] = _PadDStk[padIndex] & (_PadDStk[padIndex] ^ previousDpad);
}

/* Returns D-pad direction bits for the main stick copied by HuPadRead. */
u16 fn_80006B90(s16 padIndex)
{
    float stickX = lbl_802BFFFC[padIndex]/56.0f;
    float stickY = lbl_802BFFF8[padIndex]/56.0f;
    float absStickX = ((stickX < 0) ? -stickX : stickX);
    float absStickY = ((stickY < 0) ? -stickY : stickY);
    u16 directionButtons;
    
    if(absStickY > 0.3f) {
        if(stickY > 0) {
            directionButtons = PAD_BUTTON_UP;
        } else {
            directionButtons = PAD_BUTTON_DOWN;
        }
    } else {
        directionButtons = 0;
    }
    if(absStickX > 0.4f) {
        if(stickX < 0) {
            directionButtons |= PAD_BUTTON_LEFT;
        } else {
            directionButtons |= PAD_BUTTON_RIGHT;
        }
    }
    return directionButtons;
}

/* Returns D-pad direction bits for the sub-stick copied by HuPadRead. */
u16 fn_80006D00(s16 padIndex)
{
    float stickX = lbl_802BFFF4[padIndex]/44.0f;
    float stickY = lbl_802BFFF0[padIndex]/44.0f;
    float absStickX = ((stickX < 0) ? -stickX : stickX);
    float absStickY = ((stickY < 0) ? -stickY : stickY);
    u16 directionButtons;
    
    if(absStickY > 0.3f) {
        if(stickY > 0) {
            directionButtons = PAD_BUTTON_UP;
        } else {
            directionButtons = PAD_BUTTON_DOWN;
        }
    } else {
        directionButtons = 0;
    }
    if(absStickX > 0.4f) {
        if(stickX < 0) {
            directionButtons |= PAD_BUTTON_LEFT;
        } else {
            directionButtons |= PAD_BUTTON_RIGHT;
        }
    }
    return directionButtons;
}

/* Called by game effects to schedule a rumble pattern while the controller is connected. */
void fn_80006E70(s16 padIndex, s16 maxTime, s16 offTime, s16 onTime)
{
    RUMBLEDATA *rumbleState = &rumbleData[padIndex];
    if(_PadErr[padIndex] == PAD_ERR_NONE) {
        rumbleState->maxTime = maxTime;
        rumbleState->offTime = offTime;
        rumbleState->onTime = onTime;
        rumbleState->time = 0;
        rumbleState->numRumble = 3;
    }
}

/* If the controller has no recorded error, cancels its scheduled pattern and hard-stops its
 * motor. */
void fn_80006ECC(s16 padIndex)
{
    RUMBLEDATA *rumbleState = &rumbleData[padIndex];
    if(_PadErr[padIndex] == PAD_ERR_NONE) {
        rumbleState->maxTime = 0;
        PADControlMotor(padIndex, PAD_MOTOR_STOP_HARD);
    }
}

/* Called when game code needs to cancel every scheduled controller rumble pattern. */
void fn_80006F3C(void)
{
    int padIndex;
    for(padIndex=0; padIndex<4; padIndex++) {
        rumbleData[padIndex].maxTime = 0;
        if(_PadErr[padIndex] == PAD_ERR_NONE) {
            PADControlMotor(padIndex, PAD_MOTOR_STOP_HARD);
        }
    }
}

/* Returns the most recent controller error stored by the post-retrace reader. */
s16 HuPadStatGet(s16 padIndex)
{
    return _PadErr[padIndex];
}

/* Returns the value most recently returned by PADRead. */
u32 fn_80006FC0(void)
{
    return RumbleBit;
}
