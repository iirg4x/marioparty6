// Board window manager for message, choice, help, pause, and blank windows.
#define _MATH_H

#include "game/board/window.h"
#include "game/board/main.h"
#include "game/board/pause.h"

#include "game/disp.h"
#include "game/flag.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/pad.h"

#include "string.h"

#define MBWIN_PROCESS_PRIORITY 8207
#define MBWIN_PROCESS_STACK_SIZE 16384
#define MBWIN_PLAYER_DISABLE_RESET_MASK 0xFF

typedef struct MBWinSizeData_s {
    int posX; // Horizontal window origin in pixels.
    int posY; // Vertical window origin in pixels.
    int sizeX; // Window width in 24-pixel layout units.
    int sizeY; // Window height in 32-pixel layout units.
} MBWINSIZEDATA;

extern HUPROCESS *mbMainProc;
extern void *mbMalloc(s32 size);
extern void mbConfigPadDisableSet(BOOL disableF);
extern BOOL mbPlayerAllComCheck(void);

static BOOL mbWinOnF[MBWIN_MAX];
static s8 mbWinChoice[MBWIN_MAX];
static u8 mbWinStack[MBWIN_MAX];

static u8 mbWinTopNo;
static int mbWinNum;
static int mbWinNo;
static MBWIN *mbWinData;
static s8 mbWinLastChoice;

static const u8 speedTbl[4] = { 0, 1, 4, 0 };

static MBWINSIZEDATA winSizeTbl[MBWIN_TYPE_MAX] = {
    { 96, 328, 16, 2 },
    { 72, 128, 18, 6 },
    { 36, 344, 21, 3 },
    { 72, 328, 18, 3 },
    { 128, 312, 18, 4 },
    { 128, 312, 18, 3 },
    { 36, 352, 21, 3 },
    { 96, 416, 16, 2 },
    { 96, 328, 17, 3 },
    { 36, 344, 21, 3 },
};

static void KeyWaitInit(MBWIN *window);
static void KeyWaitSet(MBWIN *window, int keyWaitCount);
static void mbWinCenterSet(int windowNumber);

// Initializes the board window manager and registers its pause display hook during board setup.
void mbWinInit(void)
{
    MBWIN *firstWindow = &mbWinData[0];
    int windowIndex;

    HuWinInit(1);
    mbWinData = mbMalloc(sizeof(MBWIN) * MBWIN_MAX);
    for (windowIndex = 0; windowIndex < MBWIN_MAX; windowIndex++) {
        mbWinData[windowIndex].winId = HUWIN_NONE;
    }
    memset(mbWinOnF, 0, sizeof(mbWinOnF));
    mbWinNum = 0;
    mbWinNo = 0;
    mbWinTopNo = 0;
    mbPauseHookPush(mbWinPauseHook);
}

// Closes every board window and releases the manager storage during board teardown.
void mbWinClose(void)
{
    MBWIN *window;

    mbWinKillAll();
    HuWinAllKill();
    window = mbWinData;
    HuMemDirectFree(window);
}

// Creates one board window, waits for its message or choice, and runs as the child process from
// mbWinCreate. SUN and MOON speakers force frames 4 and 3, ignoring the frame stored by the caller.
// A paused window keeps this process asleep after display completion until the manager kills it.
static void mbWinProc(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    MBWIN *window = process->property;
    HuVec2f pos;
    HUWIN *windowData;
    int entryIndex;

    mbWinCenterSet(window->no);
    pos.x = window->pos.x + window->centerOfs.x;
    pos.y = window->pos.y + window->centerOfs.y;
    switch (window->type) {
        case MBWIN_TYPE_HELP:
            window->winId = HuWinCreate(pos.x, pos.y, window->size.x, window->size.y, 0);
            HuWinBGTPLvlSet(window->winId, 0.0f);
            HuWinMesSpeedSet(window->winId, 0);
            break;

        case MBWIN_TYPE_CAPSULE:
            window->winId =
                HuWinCreate(pos.x, pos.y, window->size.x, window->size.y, window->frame);
            HuWinMesSpeedSet(window->winId, 0);
            break;

        case MBWIN_TYPE_PAUSEGUIDE:
            window->winId =
                HuWinCreate(pos.x, pos.y, window->size.x, window->size.y, window->frame);
            HuWinMesSpeedSet(window->winId, 0);
            break;

        case MBWIN_TYPE_BLANK:
            window->winId = HuWinCreate(pos.x, pos.y, window->size.x, window->size.y, 0);
            break;

        default:
            if (window->speakerNo == HUWIN_SPEAKER_NULL) {
                window->winId = HuWinExCreateFrame(pos.x, pos.y, window->size.x, window->size.y,
                                                  HUWIN_SPEAKER_NULL, window->frame);
            } else if (window->speakerNo == HUWIN_SPEAKER_SUN) {
                window->winId = HuWinExCreateFrame(pos.x, pos.y, window->size.x, window->size.y,
                                                  window->speakerNo, 4);
            } else if (window->speakerNo == HUWIN_SPEAKER_MOON) {
                window->winId = HuWinExCreateFrame(pos.x, pos.y, window->size.x, window->size.y,
                                                  window->speakerNo, 3);
            } else {
                window->winId = HuWinExCreateFrame(pos.x, pos.y, window->size.x, window->size.y,
                                                  window->speakerNo, window->frame);
            }
            if (window->speakerNo == HUWIN_SPEAKER_NULL) {
                HuWinExOpen(window->winId);
            } else {
                HuWinExOpen(window->winId);
            }
            HuWinMesSpeedSet(window->winId, window->mesSpeed);
            break;
    }
    if (window->prio != -1) {
        HuWinPriSet(window->winId, window->prio);
    }
    HuWinAttrSet(window->winId, window->attr);
    for (entryIndex = 0; entryIndex < HUWIN_INSERTMES_MAX; entryIndex++) {
        if (window->insertMes[entryIndex] != MBWIN_MES_NONE) {
            HuWinInsertMesSet(window->winId, window->insertMes[entryIndex], entryIndex);
        }
    }
    if (window->type != MBWIN_TYPE_BLANK) {
        HuWinMesSet(window->winId, window->mess);
    }
    HuWinPushKeySet(window->winId, PAD_BUTTON_A);
    for (entryIndex = 0; entryIndex < HUWIN_CHOICE_MAX; entryIndex++) {
        if (window->choiceDisable[entryIndex]) {
            HuWinChoiceDisable(window->winId, entryIndex);
        }
    }
    mbWinPlayerDisable(window->no, window->playerNo);
    if (window->playerNo < 0 || GwPlayer[window->playerNo].comF) {
        KeyWaitInit(window);
        if (window->choiceF && window->comKeyHook) {
            window->comKeyHook();
        }
    }
    windowData = &winData[window->winId];
    while (windowData->stat != HUWIN_STAT_NONE) {
        // On the first message-wait clear, request one more automatic A press when this window's
        // computer-player rules allow it, then mark the clear handled.
        if ((windowData->attr & HUWIN_ATTR_KEYWAIT_CLEAR)
            && !(window->attr & HUWIN_ATTR_KEYWAIT_CLEAR)) {
            KeyWaitSet(window, 1);
            window->attr |= HUWIN_ATTR_KEYWAIT_CLEAR;
        }
        HuPrcVSleep();
    }
    if (window->pauseF) {
        HuPrcSleep(-1);
    }
    if (window->choiceNo != HUWIN_CHOICE_NONE) {
        mbWinChoice[window->no] = mbWinLastChoice = HuWinChoiceGet(window->winId, window->choiceNo);
    }
    if (window->speakerNo != HUWIN_SPEAKER_NULL) {
        HuWinExClose(window->winId);
    }
    HuPrcEnd();
}

// Destroys a window process and removes its record from the active and stacking lists as its
// destructor callback.
static void mbWinDestroy(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    MBWIN *window = process->property;
    int stackIndex;

    if (window->winId >= 0) {
        if (window->speakerNo == HUWIN_SPEAKER_NULL) {
            HuWinExKill(window->winId);
        } else {
            HuWinExKill(window->winId);
            window->speakerNo = HUWIN_SPEAKER_NULL;
        }
        window->winId = HUWIN_NONE;
    }
    if (window->choiceF) {
        mbConfigPadDisableSet(TRUE);
    }
    window->mess = 0;
    window->proc = NULL;
    if (mbWinOnF[window->no]) {
        mbWinOnF[window->no] = FALSE;
        for (stackIndex = 0; stackIndex < mbWinTopNo; stackIndex++) {
            if (mbWinStack[stackIndex] == window->no) {
                break;
            }
        }
        for (; stackIndex < mbWinTopNo - 1; stackIndex++) {
            mbWinStack[stackIndex] = mbWinStack[stackIndex + 1];
        }
        mbWinTopNo--;
    }
    mbWinNum--;
}

// Expands a window to fit its message and inserted text, then repositions the live window after
// size changes.
static void mbWinCenterSet(int winNo)
{
    MBWIN *window = &mbWinData[winNo];
    HuVec2f size;
    int insertIndex;

    for (insertIndex = 0; insertIndex < HUWIN_INSERTMES_MAX; insertIndex++) {
        if (window->insertMes[insertIndex] != MBWIN_MES_NONE) {
            HuWinInsertMesSizeGet(window->insertMes[insertIndex], insertIndex);
        }
    }
    HuWinMesMaxSizeGet(1, &size, window->mess);
    if (window->size.x < size.x) {
        window->centerOfs.x = -(size.x - window->size.x) * 0.5f;
        window->size.x = size.x;
    }
    if (window->size.y < size.y) {
        window->centerOfs.y = -(size.y - window->size.y) * 0.5f;
        window->size.y = size.y;
    }
    if (window->winId >= 0) {
        HuWinPosSet(window->winId, window->pos.x + window->centerOfs.x,
                    window->pos.y + window->centerOfs.y);
    }
}

// Allocates a window slot, fills its board defaults, and starts the child process used to display
// the window.
int mbWinCreate(int type, u32 mess, int speakerNo)
{
    MBWIN *window;
    HuVec2f size;
    int slotIndex;

    window = &mbWinData[1];
    for (slotIndex = 1; slotIndex < MBWIN_MAX; slotIndex++, window++) {
        if (window->proc == NULL) {
            break;
        }
    }
    if (slotIndex >= MBWIN_MAX) {
        return MBWIN_NONE;
    }
    memset(window, 0, sizeof(MBWIN));
    window->proc = HuPrcChildCreate(mbWinProc, MBWIN_PROCESS_PRIORITY,
                                    MBWIN_PROCESS_STACK_SIZE, 0, mbMainProc);
    HuPrcDestructorSet2(window->proc, mbWinDestroy);
    window->proc->property = window;
    window->winId = HUWIN_NONE;
    window->mess = mess;
    window->no = slotIndex;
    window->attr = HUWIN_ATTR_NONE;
    window->mesSpeed = GWMessSpeedGet();
    window->prio = -1;
    window->type = type;
    window->speakerNo = speakerNo;
    window->pauseF = window->choiceF = FALSE;
    window->choiceNo = HUWIN_CHOICE_NONE;
    window->scale.x = window->scale.y = 1.0f;
    window->comKeyHook = NULL;
    window->attr |= HUWIN_ATTR_NOCANCEL;
    window->frame = 0;
    window->playerNo = (_CheckFlag(FLAG_BOARD_TUTORIAL) == FALSE) ? GwSystem.turnPlayerNo : -1;
    for (slotIndex = 0; slotIndex < HUWIN_CHOICE_MAX; slotIndex++) {
        window->choiceDisable[slotIndex] = FALSE;
    }
    for (slotIndex = 0; slotIndex < HUWIN_INSERTMES_MAX; slotIndex++) {
        window->insertMes[slotIndex] = MBWIN_MES_NONE;
    }
    HuWinMesMaxSizeGet(1, &size, window->mess);
    if (window->type != MBWIN_TYPE_HELP) {
        window->pos.x = winSizeTbl[window->type].posX;
        window->pos.y = winSizeTbl[window->type].posY;
        window->size.x = winSizeTbl[window->type].sizeX * 24;
        window->size.y = winSizeTbl[window->type].sizeY * 32;
    } else {
        window->pos.x = HU_DISP_CENTERX - ((size.x / 2) - 16.0f);
        window->pos.y = 304.0f;
        window->size = size;
    }
    window->centerOfs.x = window->centerOfs.y = 0.0f;
    mbWinChoice[window->no] = 0;
    mbWinStack[mbWinTopNo++] = window->no;
    window->frame = 0;
    mbWinOnF[window->no] = TRUE;
    mbWinNum++;
    return window->no;
}

// Creates a choice window when a slot is available, records its choice, and enables pad input
// globally.
int mbWinCreateChoice(int type, u32 mess, int speakerNo, int choiceNo)
{
    int windowNumber = mbWinCreate(type, mess, speakerNo);

    if (windowNumber > 0) {
        MBWIN *window = &mbWinData[windowNumber];
        window->choiceF = TRUE;
        window->choiceNo = choiceNo;
        window->attr &= ~HUWIN_ATTR_NOCANCEL;
    }
    mbConfigPadDisableSet(FALSE);
    return windowNumber;
}

// Creates a help message window that pauses the board until the player dismisses it.
int mbWinCreateHelp(u32 mess)
{
    int windowNumber = mbWinCreate(MBWIN_TYPE_HELP, mess, HUWIN_SPEAKER_NULL);
    MBWIN *window = &mbWinData[windowNumber];

    window->pauseF = TRUE;
    return windowNumber;
}

// Stores a frame for window types that use a caller-selected frame; SUN and MOON speakers override
// it when the child creates the window.
int mbWinCreateFrame(int type, u32 mess, int speakerNo, int frame)
{
    int windowNumber = mbWinCreate(type, mess, speakerNo);

    if (windowNumber > 0) {
        MBWIN *window = &mbWinData[windowNumber];
        window->frame = frame;
    }
    return windowNumber;
}

// Creates a board window with frame 4 by day or 3 by night when its type and speaker use the
// caller-selected frame.
int mbWinCreateTime(int type, u32 mess, int speakerNo)
{
    if (!GwSystem.curTime) {
        return mbWinCreateFrame(type, mess, speakerNo, 4);
    } else {
        return mbWinCreateFrame(type, mess, speakerNo, 3);
    }
}

// Creates an empty board window that pauses the board while it is active.
int mbWinCreateBlank(void)
{
    int windowNumber = mbWinCreate(MBWIN_TYPE_BLANK, 0, HUWIN_SPEAKER_NULL);
    MBWIN *window = &mbWinData[windowNumber];

    window->pauseF = TRUE;
    return windowNumber;
}

// Creates an empty pausing window with an explicitly selected frame variant.
int mbWinCreateBlankFrame(int frame)
{
    int windowNumber = mbWinCreateFrame(MBWIN_TYPE_BLANK, 0, HUWIN_SPEAKER_NULL, frame);
    MBWIN *window = &mbWinData[windowNumber];

    window->pauseF = TRUE;
    return windowNumber;
}

// Returns the manager record for a board window number.
MBWIN *mbWinGet(int winNo)
{
    return &mbWinData[winNo];
}

// Stops one board window process and removes its number from the window stack.
void mbWinKill(s16 winNo)
{
    MBWIN *window = &mbWinData[winNo];
    int stackIndex;

    if (window->proc) {
        HuPrcKill(window->proc);
    }
    mbWinOnF[window->no] = FALSE;
    for (stackIndex = 0; stackIndex < mbWinTopNo; stackIndex++) {
        if (mbWinStack[stackIndex] == window->no) {
            break;
        }
    }
    for (; stackIndex < mbWinTopNo - 1; stackIndex++) {
        mbWinStack[stackIndex] = mbWinStack[stackIndex + 1];
    }
    mbWinTopNo--;
}

// Stops the board window currently at the top of the stack.
void mbWinTopKill(void)
{
    mbWinKill(mbWinStack[mbWinTopNo - 1]);
}

// Stops every active board window and clears the stack during reset or shutdown.
void mbWinKillAll(void)
{
    MBWIN *window = mbWinData;
    int windowIndex;

    for (windowIndex = 0; windowIndex < MBWIN_MAX; windowIndex++, window++) {
        if (mbWinOnF[windowIndex]) {
            if (window->proc) {
                HuPrcKill(window->proc);
            }
            mbWinOnF[windowIndex] = FALSE;
        }
    }
    mbWinTopNo = 0;
}

// Stores a window position and applies it immediately when the window has been created.
void mbWinPosSet(s16 winNo, s16 posX, s16 posY)
{
    MBWIN *window = &mbWinData[winNo];

    window->pos.x = posX;
    window->pos.y = posY;
    if (window->winId >= 0) {
        HuWinPosSet(window->winId, posX, posY);
    }
}

// Moves the board window currently at the top of the stack.
void mbWinTopPosSet(s16 posX, s16 posY)
{
    mbWinPosSet(mbWinStack[mbWinTopNo - 1], posX, posY);
}

// Copies a board window position into the caller's output vector.
void mbWinPosGet(s16 winNo, HuVec2f *pos)
{
    MBWIN *window = &mbWinData[winNo];

    pos->x = window->pos.x;
    pos->y = window->pos.y;
}

// Copies the position of the board window currently at the top of the stack.
void mbWinTopPosGet(HuVec2f *pos)
{
    mbWinPosGet(mbWinStack[mbWinTopNo - 1], pos);
}

// Stores the requested size, enforces the main message minimum, rounds live dimensions up to 16
// pixels, and updates the window geometry and sprites.
void mbWinSizeSet(s16 winNo, s16 sizeX, s16 sizeY)
{
    MBWIN *window = &mbWinData[winNo];
    HUWIN *windowData;
    HUSPRITE *spriteData;
    float backgroundAlpha0;
    float backgroundAlpha1;

    window->size.x = sizeX;
    window->size.y = sizeY;
    if (window->winId >= 0) {
        windowData = &winData[window->winId];
        mbWinCenterSet(winNo);
        sizeX = ((s16)window->size.x + 15) & 0xFFF0;
        sizeY = ((s16)window->size.y + 15) & 0xFFF0;
        windowData->winW = sizeX;
        windowData->winH = sizeY;
        windowData->mesRectX = 8;
        windowData->mesRectY = 8;
        windowData->mesRectW = sizeX - 8;
        windowData->mesRectH = sizeY - 8;
        spriteData = &HuSprData[HuSprGrpData[windowData->grpId].sprId[0]];
        backgroundAlpha0 = spriteData->a;
        spriteData = &HuSprData[HuSprGrpData[windowData->grpId].sprId[1]];
        backgroundAlpha1 = spriteData->a;
        HuSprGrpCenterSet(windowData->grpId, sizeX / 2, sizeY / 2);
        windowData->charEntryMax = (sizeX / 8) * (sizeY / 24) * 5;
        if (windowData->charEntry) {
            HuMemDirectFree(windowData->charEntry);
            windowData->charEntry = HuMemDirectMalloc(HEAP_HEAP,
                                                   sizeof(WINCHARENTRY) * windowData->charEntryMax);
        }
        HuWinFrameSet(window->winId, window->frame);
        HuSprTPLvlSet(windowData->grpId, 0, backgroundAlpha0 / 255.0f);
        HuSprTPLvlSet(windowData->grpId, 1, backgroundAlpha1 / 255.0f);
    }
}

// Resizes the board window currently at the top of the stack.
void mbWinTopSizeSet(s16 sizeX, s16 sizeY)
{
    mbWinSizeSet(mbWinStack[mbWinTopNo - 1], sizeX, sizeY);
}

// Gets the maximum rendered size of a window's main message, including the message data layout.
void mbWinMesMaxSizeGet(s16 winNo, HuVec2f *size)
{
    MBWIN *window = &mbWinData[winNo];

    HuWinMesMaxSizeGet(1, size, window->mess);
}

// Gets the maximum main message size for the board window at the top of the stack.
void mbWinTopMesMaxSizeGet(HuVec2f *size)
{
    mbWinMesMaxSizeGet(mbWinStack[mbWinTopNo - 1], size);
}

// Stores a window scale and applies it immediately to an existing window.
void mbWinScaleSet(s16 winNo, float scaleX, float scaleY)
{
    MBWIN *window = &mbWinData[winNo];

    window->scale.x = scaleX;
    window->scale.y = scaleY;
    if (window->winId >= 0) {
        HuWinScaleSet(window->winId, scaleX, scaleY);
    }
}

// Scales the board window currently at the top of the stack.
void mbWinTopScaleSet(float scaleX, float scaleY)
{
    mbWinScaleSet(mbWinStack[mbWinTopNo - 1], scaleX, scaleY);
}

// Copies a board window's stored scale into the caller's output vector.
void mbWinScaleGet(s16 winNo, HuVec2f *scale)
{
    MBWIN *window = &mbWinData[winNo];

    scale->x = window->scale.x;
    scale->y = window->scale.y;
}

// Copies the scale of the board window currently at the top of the stack.
void mbWinTopScaleGet(HuVec2f *scale)
{
    mbWinScaleGet(mbWinStack[mbWinTopNo - 1], scale);
}

// Returns the most recently completed choice from the window manager.
int mbWinTopChoiceGet(void)
{
    return mbWinLastChoice;
}

// Returns the completed choice stored for a board window number.
int mbWinChoiceGet(s16 winNo)
{
    return mbWinChoice[winNo];
}

// Marks a board window to hold the process after its message has finished.
void mbWinPause(s16 winNo)
{
    MBWIN *window = &mbWinData[winNo];

    window->pauseF = TRUE;
}

// Marks the board window currently at the top of the stack to pause.
void mbWinTopPause(void)
{
    mbWinPause(mbWinStack[mbWinTopNo - 1]);
}

// Stores an inserted message and updates the live window when that slot is already visible.
void mbWinInsertMesSet(s16 winNo, u32 insertMes, int insertMesNo)
{
    MBWIN *window = &mbWinData[winNo];

    window->insertMes[insertMesNo] = insertMes;
    if (window->winId >= 0 && window->insertMes[insertMesNo] != MBWIN_MES_NONE) {
        HuWinInsertMesSet(window->winId, window->insertMes[insertMesNo], insertMesNo);
    }
}

// Sets an inserted message in the board window currently at the top of the stack.
void mbWinTopInsertMesSet(u32 insertMes, int insertMesNo)
{
    mbWinInsertMesSet(mbWinStack[mbWinTopNo - 1], insertMes, insertMesNo);
}

// Reports whether a board window process has finished and cleared its active flag.
BOOL mbWinDoneCheck(s16 winNo)
{
    return mbWinOnF[winNo] == FALSE;
}

// Reports whether the stack is empty or its top board window has finished.
BOOL mbWinTopDoneCheck(void)
{
    if (mbWinTopNo == 0) {
        return TRUE;
    }
    return mbWinDoneCheck(mbWinStack[mbWinTopNo - 1]);
}

// Sleeps once per frame until the selected board window finishes.
void mbWinWait(s16 winNo)
{
    while (mbWinOnF[winNo]) {
        HuPrcVSleep();
    }
}

// Waits for the board window currently at the top of the stack when one exists.
void mbWinTopWait(void)
{
    if (mbWinTopNo == 0) {
        return;
    }
    mbWinWait(mbWinStack[mbWinTopNo - 1]);
}

// Adds window attributes to the stored state and applies them to a live window.
void mbWinAttrSet(s16 winNo, u32 attr)
{
    MBWIN *window = &mbWinData[winNo];

    window->attr |= attr;
    if (window->winId >= 0) {
        HuWinAttrSet(window->winId, window->attr);
    }
}

// Adds attributes to the board window currently at the top of the stack.
void mbWinTopAttrSet(u32 attr)
{
    mbWinAttrSet(mbWinStack[mbWinTopNo - 1], attr);
}

// Removes window attributes from the stored state and applies the result to a live window.
void mbWinAttrReset(s16 winNo, u32 attr)
{
    MBWIN *window = &mbWinData[winNo];

    window->attr &= ~attr;
    if (window->winId >= 0) {
        HuWinAttrSet(window->winId, window->attr);
    }
}

// Removes attributes from the board window currently at the top of the stack.
void mbWinTopAttrReset(u32 attr)
{
    mbWinAttrReset(mbWinStack[mbWinTopNo - 1], attr);
}

// Disables one choice entry in the stored window state before or during display.
void mbWinChoiceDisable(s16 winNo, int choiceNo)
{
    MBWIN *window = &mbWinData[winNo];

    window->choiceDisable[choiceNo] = TRUE;
}

// Disables one choice entry in the board window currently at the top of the stack.
void mbWinTopChoiceDisable(int choiceNo)
{
    mbWinChoiceDisable(mbWinStack[mbWinTopNo - 1], choiceNo);
}

// Selects a message speed and applies the corresponding engine value to a live window.
void mbWinMesSpeedSet(s16 winNo, int speed)
{
    MBWIN *window = &mbWinData[winNo];

    window->mesSpeed = speedTbl[speed];
    if (window->winId >= 0) {
        HuWinMesSpeedSet(window->winId, window->mesSpeed);
    }
}

// Sets message speed for the board window currently at the top of the stack.
void mbWinTopMesSpeedSet(int speed)
{
    mbWinMesSpeedSet(mbWinStack[mbWinTopNo - 1], speed);
}

// Sets the message color on a live board window.
void mbWinMesColSet(s16 winNo, int mesCol)
{
    MBWIN *window = &mbWinData[winNo];

    HuWinMesColSet(window->winId, mesCol);
}

// Sets the message color on the board window currently at the top of the stack.
void mbWinTopMesColSet(int mesCol)
{
    mbWinMesColSet(mbWinStack[mbWinTopNo - 1], mesCol);
}

// Returns the currently highlighted choice from a live board window.
s16 mbWinChoiceNowGet(s16 winNo)
{
    MBWIN *window = &mbWinData[winNo];

    return HuWinChoiceNowGet(window->winId);
}

// Returns the currently highlighted choice from the board window at the top of the stack.
s16 mbWinTopChoiceNowGet(void)
{
    return mbWinChoiceNowGet(mbWinStack[mbWinTopNo - 1]);
}

// Stores a window priority and applies it immediately when the window is live.
void mbWinPriSet(s16 winNo, s16 prio)
{
    MBWIN *window = &mbWinData[winNo];

    window->prio = prio;
    if (window->winId >= 0) {
        HuWinPriSet(window->winId, window->prio);
    }
}

// Sets priority on the board window currently at the top of the stack.
void mbWinTopPriSet(s16 prio)
{
    mbWinPriSet(mbWinStack[mbWinTopNo - 1], prio);
}

// Returns the highlighted choice, or -1 when the selected board window is not live.
s16 mbWinChoiceNowGet2(s16 winNo)
{
    MBWIN *window = &mbWinData[winNo];

    if (window->winId < 0) {
        return -1;
    }
    return HuWinChoiceNowGet(window->winId);
}

// Returns the guarded current choice from the board window at the top of the stack.
s16 mbWinTopChoiceNowGet2(void)
{
    return mbWinChoiceNowGet2(mbWinStack[mbWinTopNo - 1]);
}

// In tutorial mode, forces player 0; otherwise allows all human-player pads for -1, disables all
// pads for a computer player, or allows only the selected human player's pad, then applies the
// mask.
void mbWinPlayerDisable(s16 winNo, int playerNo)
{
    MBWIN *window = &mbWinData[winNo];
    u8 disabledPlayers;
    int playerIndex;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        window->playerNo = 0;
        disabledPlayers = ~HUWIN_PLAYER_1;
    } else {
        window->playerNo = playerNo;
        if (window->playerNo == -1) {
            disabledPlayers = HUWIN_PLAYER_ALL;
            for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
                if (GwPlayer[playerIndex].comF == FALSE) {
                    disabledPlayers &= ~(1 << GwPlayer[playerIndex].padNo);
                }
            }
        } else if (GwPlayer[window->playerNo].comF) {
            disabledPlayers = HUWIN_PLAYER_ALL;
        } else {
            disabledPlayers = ~(1 << GwPlayer[window->playerNo].padNo);
        }
    }
    if (window->winId >= 0) {
        HuWinDisablePlayerReset(window->winId, MBWIN_PLAYER_DISABLE_RESET_MASK);
        HuWinDisablePlayerSet(window->winId, disabledPlayers);
    }
}

// Sets controller access for the board window currently at the top of the stack.
void mbWinTopPlayerDisable(int playerNo)
{
    mbWinPlayerDisable(mbWinStack[mbWinTopNo - 1], playerNo);
}

// Reads the message's computer wait count and prepares automatic A presses outside the tutorial.
static void KeyWaitInit(MBWIN *window)
{
    int keyWaitCount;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        return;
    }
    keyWaitCount = HuWinKeyWaitNumGet(window->mess);
    if (keyWaitCount) {
        HuWinComKeyReset();
        KeyWaitSet(window, keyWaitCount);
    }
}

// Queues the requested message waits for all-computer games or a selected computer player; returns
// without queuing if the count is zero or no eligible computer player is selected.
static void KeyWaitSet(MBWIN *window, int keyWaitCount)
{
    s32 key[GW_PLAYER_MAX] = {};
    int playerIndex;
    int delayFrames;

    if (keyWaitCount == 0) {
        return;
    }
    if (mbPlayerAllComCheck()) {
        key[0] = key[1] = key[2] = key[3] = PAD_BUTTON_A;
    } else {
        if (window->playerNo == -1) {
            return;
        }
        if (!GwPlayer[window->playerNo].comF) {
            return;
        }
        for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
            int padNumber = GwPlayer[playerIndex].padNo;
            if (window->playerNo == playerIndex) {
                key[padNumber] |= PAD_BUTTON_A;
            }
        }
    }
    delayFrames = GWComKeyDelayGet() * 1.5f;
    for (playerIndex = 0; playerIndex < keyWaitCount; playerIndex++) {
        HuWinComKeyWait(key[0], key[1], key[2], key[3], delayFrames);
    }
}

// Installs the computer choice hook on the board window currently at the top of the stack.
void mbWinTopComKeyHookSet(MBWINCOMKEYHOOK comKeyHook)
{
    s16 windowNumber = mbWinStack[mbWinTopNo - 1];
    MBWIN *window = &mbWinData[windowNumber];

    window->comKeyHook = comKeyHook;
}

// Shows or hides a live board window while preserving the requested process state.
void mbWinDispSet(s16 winNo, BOOL dispF)
{
    MBWIN *window = &mbWinData[winNo];

    if (window->proc == NULL || window->winId < 0) {
        return;
    }
    if (dispF) {
        HuWinDispOn(window->winId);
    } else {
        HuWinDispOff(window->winId);
    }
}

// Shows or hides the board window currently at the top of the stack.
void mbWinTopDispSet(BOOL dispF)
{
    mbWinDispSet(mbWinStack[mbWinTopNo - 1], dispF);
}

// Applies pause display visibility to every active board window.
void mbWinPauseHook(BOOL dispF)
{
    int windowIndex;

    for (windowIndex = 1; windowIndex < MBWIN_MAX; windowIndex++) {
        if (mbWinOnF[windowIndex]) {
            mbWinDispSet(windowIndex, dispF);
        }
    }
}

// Returns the engine window ID stored for a board window number.
HUWINID mbWinIDGet(s16 winNo)
{
    MBWIN *window = &mbWinData[winNo];

    return window->winId;
}

// Returns the engine window ID for the board window at the top of the stack.
HUWINID mbWinTopIDGet(void)
{
    return mbWinIDGet(mbWinStack[mbWinTopNo - 1]);
}

// Computes the screen centered position needed for a board window's message and stored minimum
// size.
void mbWinCenterGet(s16 winNo, HuVec2f *pos)
{
    MBWIN *window = &mbWinData[winNo];
    HuVec2f size;

    mbWinMesMaxSizeGet(winNo, &size);
    if (window->size.x > size.x) {
        size.x = window->size.x;
    }
    if (window->size.y > size.y) {
        size.y = window->size.y;
    }
    pos->x = HU_DISP_CENTERX - (size.x / 2);
    pos->y = HU_DISP_CENTERY - (size.y / 2);
}

// Replaces a window's main message, reapplies inserted messages, and recenters the live window.
void mbWinCenterInsertGet(s16 winNo, u32 mess)
{
    MBWIN *window = &mbWinData[winNo];
    int insertIndex;

    window->mess = mess;
    if (window->winId >= 0) {
        for (insertIndex = 0; insertIndex < HUWIN_INSERTMES_MAX; insertIndex++) {
            if (window->insertMes[insertIndex] != MBWIN_MES_NONE) {
                HuWinInsertMesSet(window->winId, window->insertMes[insertIndex], insertIndex);
            }
        }
        HuWinMesSet(window->winId, window->mess);
    }
    mbWinCenterSet(winNo);
}

// Replaces the main message in the board window currently at the top of the stack and recenters it.
void mbWinTopCenterInsertGet(u32 mess)
{
    mbWinCenterInsertGet(mbWinStack[mbWinTopNo - 1], mess);
}
