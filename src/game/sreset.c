/* Soft-reset handling and disc-error screens, serviced alongside the game loop. */
/* This file uses no math functions from the transitive dolphin.h includes. */
#define _MATH_H

#include "dolphin.h"
#include "game/flag.h"
#include "game/dvd.h"
#include "game/pad.h"
#include "game/audio.h"
#include "game/thpmain.h"
#include "game/init.h"
#include "game/main.h"

BOOL HuSoftResetButtonCheck(void);
void HuDvdErrDispInit(GXRenderModeObj *rmode, void *xfb1, void *xfb2);
void HuRestartSystem(void);
BOOL HuSoftResetCountCheck(void);
BOOL HuSoftResetCheck(void);

extern BOOL SR_ExecReset;
extern BOOL SR_ExecResetMenu;

#define SR_DVD_LOADING 0
#define SR_DVD_COVER_OPEN 1
#define SR_DVD_NO_DISK 2
#define SR_DVD_WRONG_DISK 3
#define SR_DVD_RETRY_ERROR 4
#define SR_DVD_FATAL_ERROR 5

#define PAD_BTN_SRESET (PAD_BUTTON_START|PAD_BUTTON_X|PAD_BUTTON_B)
#define SR_XFB_WIDTH_ALIGN_MASK 0xF
#define SR_XFB_BLACK_BIT_PATTERN 0x800080
#define SR_XFB_WHITE_LUMA 235
#define SR_XFB_BLACK_LUMA 16
#define SR_RETRACE_WAIT_SPIN_LIMIT 1349800

/* Normal reset polling ignores startup-held chords until the stored button set differs from
 * Start + X + B, including an extra button. */
static s32 SR_PreRstChk[4] = {};

#include "coveropen_en.inc"
#include "fatalerror_en.inc"
#include "loading_en.inc"
#include "nodisc_en.inc"
#include "retryerror_en.inc"
#include "wrongdisc_en.inc"

/* Consecutive reset-count checks with the chord held, one counter per controller. */
static s16 SR_PushTime[4] = {};
/* Controller that completed the hold; -1 means no controller reset has been selected. */
static s8 SR_ResetPad = -1;

/* Framebuffer width in pixels, rounded up to a multiple of 16. */
static s16 XfbW;
/* Framebuffer height in rows. */
static s16 XfbH;
/* TRUE for a framebuffer mode other than VI_XFBMODE_SF; the drawing code does not read it. */
static BOOL XfbProg;
/* Both video buffers receive the same disc-error bitmap. */
static void *Xfb[2] = {};
/* Keep showing loading during busy/cover-closed states after a disc problem was displayed. */
static BOOL trychkBusyWait;
/* Set after reset preparation; the monitor and main loop both act on it. */
BOOL SR_ExecReset;
/* Request a hot reset that opens the console menu instead of restarting the game. */
BOOL SR_ExecResetMenu;
/* Prevent a second entry into the console restart sequence. */
static BOOL SR_RestartChk;
/* Latched after the console reset button is pressed; its release requests a reset. */
static BOOL H_ResetReady;

static void HuSoftResetPostProc(void);

/* Called by the main loop before game updates; executes a prepared reset and returns its flag. */
BOOL HuSoftResetButtonCheck(void)
{
	if(SR_ExecReset) {
		HuRestartSystem();
	}
	return (SR_ExecReset) ? TRUE : FALSE;
}

/* Its sender wait queue wakes the monitor at retrace; no messages are sent or received. */
static OSMessageQueue ToeMessageQueue;

/* VI pre-retrace callback installed at startup; wakes the reset and disc-error monitor thread. */
void HuDvdErrDispIntFunc(u32 unusedRetraceCount)
{
	OSWakeupThread(&ToeMessageQueue.queueSend);
}

/* Called during disc-monitor initialization to record reset chords already held at startup. */
static inline void HuPreRstChk(void)
{
	/* Startup samples are kept separate from the regular pad retrace snapshots. */
	static PADStatus padStat[4];
	int padIndex;
	PADRead(padStat);
	for(padIndex=0; padIndex<4; padIndex++) {
		PADStatus *padStatus = &padStat[padIndex];
		if(padStatus->err != 0) {
			continue;
		}
		if(padStatus->button == PAD_BTN_SRESET) {
			SR_PreRstChk[padIndex] = 1;
		} else {
			SR_PreRstChk[padIndex] = 0;
		}
	}
}

static OSMessage ToeMessageArray[16];
static OSThread ToeThread;
static u8 ToeThreadStack[4096];

static void *ToeThreadFunc(void *param);
static void ToeDispCheck(void);

/* Called by HuSysInit to bind both video buffers and start retrace-driven reset/disc monitoring.
 * The caller supplies a render mode: its framebuffer mode is read even after the size fallback. */
void HuDvdErrDispInit(GXRenderModeObj *renderMode, void *firstFramebuffer, void *secondFramebuffer)
{
	BOOL interruptsWereEnabled;
	HuSRDisableF = FALSE;
	SR_ResetPad = -1;
	SR_ExecReset = H_ResetReady = 0;
	SR_RestartChk = 0;
	SR_PushTime[0] = SR_PushTime[1] = SR_PushTime[2] = SR_PushTime[3] = 0;
	VIWaitForRetrace();
	VIWaitForRetrace();
	VIWaitForRetrace();
	/* Suppress startup-held chords in normal polling until a stored button sample differs from
	 * the exact chord. */
	HuPreRstChk();
	HuDvdErrWait = FALSE;
	Xfb[0] = firstFramebuffer;
	Xfb[1] = secondFramebuffer;
	if(renderMode) {
		XfbW = (u16)(((u16)renderMode->fbWidth+15) & ~SR_XFB_WIDTH_ALIGN_MASK);
		XfbH = renderMode->xfbHeight;
	} else {
		XfbW = 640;
		XfbH = 480;
	}
	if((u16)renderMode->xFBmode == VI_XFBMODE_SF) {
		XfbProg = FALSE;
	} else {
		XfbProg = TRUE;
	}
	trychkBusyWait = FALSE;
	/* Only queueSend is used as a sleep/wakeup queue for the retrace callback. */
	OSInitMessageQueue(&ToeMessageQueue, ToeMessageArray, 16);
        OSCreateThread(&ToeThread, ToeThreadFunc, NULL, &ToeThreadStack[4096], 4096, 8,
                       OS_THREAD_ATTR_DETACH);
        OSResumeThread(&ToeThread);
	interruptsWereEnabled = OSDisableInterrupts();
	VISetPreRetraceCallback(HuDvdErrDispIntFunc);
	OSRestoreInterrupts(interruptsWereEnabled);
}

/* Monitor thread started by HuDvdErrDispInit; each retrace checks reset requests and disc state.
 * A request remains pending while HuSRDisableF postpones reset preparation. */
static void *ToeThreadFunc(void *unusedParam)
{
    BOOL resetPending = FALSE;
	while(1) {
		BOOL resetExecuting;
        BOOL restartRequested;
		OSSleepThread(&ToeMessageQueue.queueSend);
		if(!HuSoftResetCheck()) {
            if(SR_ExecReset) {
                restartRequested = TRUE;
            } else {
                /* Console-button reset is triggered after a press followed by release. */
                if(H_ResetReady == TRUE && OSGetResetButtonState() != TRUE) {
                    restartRequested = TRUE;
                } else {
                    if(H_ResetReady != TRUE && OSGetResetButtonState() == TRUE) {
                        H_ResetReady = TRUE;
                    }
                    restartRequested = FALSE;
                }
            }
            if(restartRequested || SR_ExecResetMenu) {
                proc_reset:
                resetPending = TRUE;
            }
        }  else {
            goto proc_reset;
        }
        if(resetPending && HuSRDisableF == FALSE) {
            HuSoftResetPostProc();
        }
		if(SR_ExecReset) {
			HuRestartSystem();
		}
		if(SR_ExecReset) {
			resetExecuting = TRUE;
		} else {
			resetExecuting = FALSE;
		}
		if(!resetExecuting) {
			ToeDispCheck();
		}
	}
}

static void _HuDvdErrDispXFB(s32 error);

/* Called by the monitor when no reset is executing; maps drive problems to their error screens.
 * A fatal drive error disables subsequent reset handling. */
static void ToeDispCheck(void)
{
	s32 errorStatus;
	if(SR_ResetPad != -1 || SR_ExecReset != FALSE || SR_RestartChk != 0) {
		return;
	}
	if(HuSRDisableF) {
		return;
	}
	errorStatus = DVDGetDriveStatus();
	switch(errorStatus) {
		case DVD_STATE_FATAL_ERROR:
			errorStatus = SR_DVD_FATAL_ERROR;
			trychkBusyWait = TRUE;
            HuSRDisableF = TRUE;
			break;

		case DVD_STATE_END:
			HuDvdErrWait = FALSE;
			trychkBusyWait = FALSE;
			return;

		case DVD_STATE_COVER_OPEN:
			errorStatus = SR_DVD_COVER_OPEN;
			trychkBusyWait = TRUE;
			break;

		/* Ordinary reads stay invisible; loading follows only a displayed disc problem. */
		case DVD_STATE_BUSY:
		case DVD_STATE_COVER_CLOSED:
			if(!trychkBusyWait) {
				return;
			}
			errorStatus = SR_DVD_LOADING;
			break;

		case DVD_STATE_NO_DISK:
			errorStatus = SR_DVD_NO_DISK;
			trychkBusyWait = TRUE;
			break;

		case DVD_STATE_WRONG_DISK:
			errorStatus = SR_DVD_WRONG_DISK;
			trychkBusyWait = TRUE;
			break;

		case DVD_STATE_RETRY:
			errorStatus = SR_DVD_RETRY_ERROR;
			trychkBusyWait = TRUE;
			break;

		default:
			return;
	}
	HuDvdErrWait = TRUE;
	HuPadRumbleAllStop();
	VISetBlack(FALSE);
	VIFlush();

	_HuDvdErrDispXFB(errorStatus);
}

static void DvdErrDispXFB(void *data);

/* Called for a drive problem to display its bitmap until the drive status changes.
 * Polls controller and console reset input directly while the normal game loop is waiting. */
static void _HuDvdErrDispXFB(s32 errorImage)
{
	/* This build always selects the English row. Columns follow the SR_DVD_* values. */
	static void *bmpMes[][6] = {
		loading_en, coveropen_en, nodisc_en, wrongdisc_en, retryerror_en, fatalerror_en
	};
    s16 padIndex;
    s32 stableDriveStatus;
    PADStatus padStatuses[4];
    OSTick resetHoldStartTicks[4]; /* Tick timestamp at the start of each held reset chord. */
    u8 resetHeld[4]; /* TRUE once that controller has started holding the exact chord. */
    u32 *firstFramebuffer;
    u32 *secondFramebuffer;
	BOOL padResetRequested = FALSE;
    s8 languageIndex = 0;
    DvdErrDispXFB(bmpMes[languageIndex][errorImage]);
    for(padIndex=0; padIndex<4; padIndex++) {
        resetHeld[padIndex] = FALSE;
    }
    padResetRequested = FALSE;
    stableDriveStatus = DVDGetDriveStatus();
    /* Re-evaluate the selected screen in ToeDispCheck as soon as the drive state changes. */
    while(stableDriveStatus != 0) {

        if(stableDriveStatus != DVDGetDriveStatus()) {
            break;
        }
        /* Normal pad sampling stops during a disc error, so read controllers here. */
        PADRead(padStatuses);
        for(padIndex=0; padIndex<4; padIndex++) {
            PADStatus *padStatus = &padStatuses[padIndex];
            if(padStatus->err != 0) {
                /* Read errors preserve the hold state and timestamp; a later valid chord sample
                 * includes this time in its hold duration. */
                continue;
            }
            if(padStatus->button != PAD_BTN_SRESET) {
                resetHeld[padIndex] = FALSE;
                continue;
            } else if(resetHeld[padIndex] == FALSE) {
                resetHoldStartTicks[padIndex] = OSGetTick();
            } else {
                if(OSTicksToMilliseconds(OSGetTick()-resetHoldStartTicks[padIndex]) > 500) {
                    padResetRequested = TRUE;
                }
            }
            resetHeld[padIndex] = TRUE;
        }
        if(HuSRDisableF == FALSE) {
            BOOL restartRequested;
            if(SR_ExecReset) {
                restartRequested = TRUE;
            } else {
                if(H_ResetReady == TRUE && OSGetResetButtonState() != TRUE) {
                    restartRequested = TRUE;
                } else {
                    if(H_ResetReady != TRUE && OSGetResetButtonState() == TRUE) {
                        H_ResetReady = TRUE;
                    }
                    restartRequested = FALSE;
                }
            }
            if(restartRequested || padResetRequested) {
                if(msmSysCheckInit()) {
                    msmStreamSetMasterVolume(0);
                    msmSeSetMasterVolume(0);
                    msmMusSetMasterVolume(0);
                }
                HuRestartSystem();
            }
        }
        firstFramebuffer = Xfb[0];
        secondFramebuffer = Xfb[1];
        /* Redraw if another renderer overwrote either buffer at the sampled pixel. */
        DCInvalidateRange(&firstFramebuffer[(640/2)*200], sizeof(u16)*640);
        DCInvalidateRange(&secondFramebuffer[(640/2)*200], sizeof(u16)*640);
        if (firstFramebuffer[((640 / 2) * 200) + 32] != SR_XFB_BLACK_BIT_PATTERN ||
            secondFramebuffer[((640 / 2) * 200) + 32] != SR_XFB_BLACK_BIT_PATTERN) {
            DvdErrDispXFB(bmpMes[languageIndex][errorImage]);
        }
        VISetNextFrameBuffer(DemoCurrentBuffer);
        VIFlush();
        OSYieldThread();
    }
}

/* Called by the disc-error loop to clear both buffers and draw a centered monochrome bitmap.
 * The first two signed halfwords are width and height; following words hold low-bit-first pixels.
 */
static void DvdErrDispXFB(void *bitmap)
{
    s16 *bitmapHeader;
    u8 *firstPixelPair;
    u8 *secondPixelPair;
    u32 drawIndex;
    u32 bitIndex;
    u32 bitmapBits;
    u32 bitmapRow;

    u32 *firstFramebuffer;
    u32 *secondFramebuffer;
    u32 *bitmapWords;
    s32 rowOffsetBytes;
    s32 rowStrideBytes;
    u8 chromaCb;
    u8 chromaCr;
    firstFramebuffer = Xfb[0];
    secondFramebuffer = Xfb[1];
    /* YUYV pairs use zero luma and neutral chroma for the cleared background. */
    for (drawIndex = 0; drawIndex < XfbW * XfbH * 2 / 4;
         drawIndex++, firstFramebuffer++, secondFramebuffer++) {
        *firstFramebuffer = *secondFramebuffer = SR_XFB_BLACK_BIT_PATTERN;
    }
    DCFlushRangeNoSync(Xfb[0], XfbW*XfbH*2);
	DCFlushRangeNoSync(Xfb[1], XfbW*XfbH*2);
    bitmapHeader = bitmap;
    bitmapWords = (u32 *)(&bitmapHeader[2]);
    rowOffsetBytes = ((XfbW/2)-(bitmapHeader[0]/2))*2;
	rowStrideBytes = XfbW*2;
	chromaCb = chromaCr = 128;
    /* Error text is horizontally centered and begins at framebuffer row 200. */
    for(bitmapRow=0; bitmapRow<bitmapHeader[1]; bitmapRow++) {
        void *rowStartAddresses[2];
        firstPixelPair = ((u8 *)(Xfb[0])+((bitmapRow+200)*rowStrideBytes)+rowOffsetBytes);
		rowStartAddresses[1] = firstPixelPair;
		secondPixelPair = ((u8 *)(Xfb[1])+((bitmapRow+200)*rowStrideBytes)+rowOffsetBytes);
		rowStartAddresses[0] = secondPixelPair;
        for(drawIndex=0; drawIndex<bitmapHeader[0]; drawIndex += 32) {
            bitmapBits = *bitmapWords++;
            for (bitIndex = 0; bitIndex < 32;
                 bitIndex += 2, bitmapBits >>= 2, firstPixelPair += 4, secondPixelPair += 4) {
                /* A zero pair leaves the cleared background untouched. */
                if(bitmapBits & 0x3){
                    u8 firstLuma;
                    u8 secondLuma;
					if(bitmapBits & 0x1) {
						firstLuma = SR_XFB_WHITE_LUMA;
					} else {
						firstLuma = SR_XFB_BLACK_LUMA;
					}
					if(bitmapBits & 0x2) {
						secondLuma = SR_XFB_WHITE_LUMA;
					} else {
						secondLuma = SR_XFB_BLACK_LUMA;
					}
					firstPixelPair[0] = firstLuma;
					firstPixelPair[1] = chromaCb;
					firstPixelPair[2] = secondLuma;
					firstPixelPair[3] = chromaCr;
					secondPixelPair[0] = firstLuma;
					secondPixelPair[1] = chromaCb;
					secondPixelPair[2] = secondLuma;
					secondPixelPair[3] = chromaCr;
				}
            }
        }
        DCFlushRangeNoSync(rowStartAddresses[1], bitmapHeader[0]*2);
        DCFlushRangeNoSync(rowStartAddresses[0], bitmapHeader[0]*2);
    }
    PPCSync();
}

/* Called by the main-loop reset poll or monitor to blank video and reset the console once.
 * Menu requests and wrong discs use a hot reset; other requests restart the game. */
void HuRestartSystem(void)
{
	u32 retraceWait[2]; /* [0]: spin iterations; [1]: retrace count before waiting. */
	BOOL interruptsWereEnabled;
	if(SR_RestartChk) {
		return;
	}
	SR_RestartChk = TRUE;
	PADRecalibrate(PAD_CHAN0_BIT|PAD_CHAN1_BIT|PAD_CHAN2_BIT|PAD_CHAN3_BIT);
	/* The audio-system check result is ignored here. */
	msmSysCheckInit();
	VISetBlack(TRUE);
	VIFlush();
	interruptsWereEnabled = OSDisableInterrupts();
	if(!interruptsWereEnabled) {
		OSReport("PrevInt=DISABLE!!\n");
	}
	/* Retrace needs interrupts; enable them even if this caller arrived with them disabled. */
	OSEnableInterrupts();
	retraceWait[1] = VIGetRetraceCount();
	retraceWait[0] = 0;
	/* Bound the busy wait if video interrupts fail to advance the retrace count. */
	while(retraceWait[1] == VIGetRetraceCount()) {
		if(retraceWait[0]++ >= SR_RETRACE_WAIT_SPIN_LIMIT) {
			break;
		}
	}
	OSReport("Timeout Count=%d\n", retraceWait[0]);
	GXAbortFrame();
    if(SR_ExecResetMenu != 0) {
        OSResetSystem(TRUE, 0, TRUE);
    } else if(DVDGetDriveStatus() == DVD_STATE_WRONG_DISK) {
        OSResetSystem(TRUE, 0, FALSE);
    } else {
        OSResetSystem(FALSE, 0, FALSE);
    }

}

/* Called by the monitor each retrace to check prepared resets and controller hold/release state.
 * Controller checks wait until the pad retrace callback has advanced VCounter. */
BOOL HuSoftResetCheck(void)
{
	int padIndex;
	if(VCounter == 0) {
		return FALSE;
	}
	if(SR_ExecReset) {
		return TRUE;
	}
	/* Once selected, any stored button set differing from the exact chord requests reset,
	 * including an extra held button. */
	if(SR_ResetPad != -1) {
		if(_PadBtn[SR_ResetPad] != PAD_BTN_SRESET) {
			return TRUE;
		}
	} else {
		for(padIndex=0; padIndex<4; padIndex++) {
			if(SR_PreRstChk[padIndex] && _PadBtn[padIndex] != PAD_BTN_SRESET) {
				SR_PreRstChk[padIndex] = FALSE;
			}
		}
	}

	if(HuSoftResetCountCheck()) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/* Called by HuSoftResetCheck to count consecutive checks with exactly Start + X + B held.
 * Selects a controller when its prior count is at least 30, excluding startup-held chords. */
BOOL HuSoftResetCountCheck(void)
{
	int padIndex;
	for(padIndex=0; padIndex<4; padIndex++) {
		if(_PadBtn[padIndex] != PAD_BTN_SRESET) {
			SR_PushTime[padIndex] = 0;
		} else {
			if(!SR_PreRstChk[padIndex]) {
				if(_PadBtn[padIndex] & PAD_BUTTON_START) {
					if(SR_PushTime[padIndex]++ >= 30) {
						SR_ResetPad = padIndex;
						return TRUE;
					}
				} else {
					SR_PushTime[padIndex] = 0;
				}
			}
		}
	}
	return FALSE;
}

/* Called by the monitor once a reset is pending and resets are enabled; blanks video, requests
 * movie shutdown, mutes audio when its check succeeds, stops rumble, and marks reset prepared. */
static void HuSoftResetPostProc(void)
{
	if(!SR_ExecReset) {
		VISetBlack(TRUE);
		VIFlush();
		if(THPProc) {
			/* Both calls set the playback state; close immediately supersedes stop. */
			HuTHPStop();
			HuTHPClose();
		}
		if(msmSysCheckInit()) {
			msmStreamSetMasterVolume(0);
			msmSeSetMasterVolume(0);
			msmMusSetMasterVolume(0);
		}
		HuPadRumbleAllStop();
		SR_ExecReset = TRUE;
	}
}
