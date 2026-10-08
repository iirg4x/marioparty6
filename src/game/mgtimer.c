/* Creates and updates the minigame countdown and score timers. */
#define _MATH_H

#include "game/gamemes.h"
#include "game/memory.h"
#include "game/process.h"
#include "game/main.h"
#include "game/mg/timer.h"
#include "datanum/mgconst.h"

/* Timer values are frames at 60 frames per second; positions are screen pixels. */

const static s16 MinuteLenTbl[4] = {
    0, 60, 100, 60
};

const static s16 boxSizeTbl[MGTIMER_TYPE_MAX][2] = {
    { 40, 32 },
    { 152, 64 },
    { 104, 32 },
    { 134, 32 }
};

const static s16 timerPosTbl[MGTIMER_TYPE_MAX][2] = {
    { 288, 64 },
    { 292, 70 },
    { 292, 64 },
    { 292, 64 }
};

typedef void (*TIMERFUNC)(MGTIMER *timer);

static void TimerExec();
static void TimerExecOn(MGTIMER* timer);
static void TimerExecOff(MGTIMER* timer);
static void CreateSprite(MGTIMER* timer);
static void KillSprite(MGTIMER* timer);
static void UpdateSprite(MGTIMER* timer);

static const HuVec2f digitOfsTbl[MGTIMER_TYPE_MAX][15] = {
    {
        {   0.0f,   0.0f}, {   0.0f,   0.0f}, {   0.0f,   0.0f}, {   0.0f,   0.0f},
        {   0.0f,   0.0f}, {   0.0f,   0.0f}, {   0.0f,   0.0f}, {   0.0f,   0.0f},
        {   0.0f,   0.0f}, {   0.0f,   0.0f}, {   0.0f,   0.0f}, {   0.0f,   0.0f},
        {   0.0f,   0.0f}, {   0.0f,   0.0f}, {   0.0f,   0.0f}
    },
    {
        { -46.0f,   0.0f}, { -30.0f,   0.0f}, { -14.0f,   0.0f}, {   2.0f,   0.0f},
        {  18.0f,   0.0f}, {  34.0f,   0.0f}, {  50.0f,   0.0f}, { -46.0f, -16.0f},
        { -30.0f, -16.0f}, { -14.0f, -16.0f}, {   2.0f, -16.0f}, {  18.0f, -16.0f},
        {  34.0f, -16.0f}, {  50.0f, -16.0f}, { -64.0f, -16.0f},
    },
    {
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, { -39.0f,  -8.0f}, { -23.0f,  -8.0f},
        {  -7.0f,  -8.0f}, {   9.0f,  -8.0f}, {  25.0f,  -8.0f}, {-900.0f, -900.0f},
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f},
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f}
    },
    {
        { -55.0f,  -8.0f}, { -39.0f,  -8.0f}, { -23.0f,  -8.0f}, {  -7.0f,  -8.0f},
        {   9.0f,  -8.0f}, {  25.0f,  -8.0f}, {  41.0f,  -8.0f}, {-900.0f, -900.0f},
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f},
        {-900.0f, -900.0f}, {-900.0f, -900.0f}, {-900.0f, -900.0f}
    },
};

static TIMERFUNC modeTbl[MGTIMER_MODE_MAX] = { TimerExecOff, TimerExecOn };

/* Called by minigame setup to allocate a timer, create its display, and start its update
 * process. */
MGTIMER* MgTimerCreate(int type) {
    MGTIMER* timer;
    int spriteIndex;
    timer = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MGTIMER), HU_MEMNUM_OVL);
    if (timer == NULL) {
        return NULL;
    }
    timer->type = type;
    timer->mode = MGTIMER_MODE_OFF;
    timer->prevMode = MGTIMER_MODE_OFF;
    timer->dispOnF = FALSE;
    timer->boxDispF = TRUE;
    timer->stopF = FALSE;
    timer->scaleDir = 0;
    timer->flashFlag = 0;
    timer->time = 0;
    timer->maxTime = 0;
    timer->recordTime = 0;
    timer->endTime = 0;
    timer->speed = 0;
    timer->pos.x = timerPosTbl[type][0];
    timer->pos.y = timerPosTbl[type][1];
    timer->gameMesId = GAMEMES_ID_NONE;
    timer->boxId = HUSPR_GROUP_NONE;
    timer->digitR = 255;
    timer->digitG = 255;
    timer->digitB = 255;
    timer->digitScale = 1.0f;
    timer->timeUnit = MinuteLenTbl[type];
    timer->fadeOutTime = 0;
    for (spriteIndex = 0; spriteIndex < 15; spriteIndex++) {
        timer->espId[spriteIndex] = -1;
    }
    CreateSprite(timer);
    switch (type) {
        case MGTIMER_TYPE_NORMAL:
            break;

        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_WIDESCORE:
            timer->boxId = MgScoreBoxCreate(boxSizeTbl[timer->type][0], boxSizeTbl[timer->type][1]);
            MgScoreBoxDispSet(timer->boxId, FALSE);
            break;

    }
    MgTimerColorSet(timer, 255, 255, 255);
    UpdateSprite(timer);
    timer->proc = HuPrcChildCreate(TimerExec, 0x1000, 0x1200, 0, HuPrcCurrentGet());
    timer->proc->property = timer;
    return timer;
}

/* Called by the owning minigame during cleanup to stop the timer and release its display
 * resources. */
void MgTimerKill(MGTIMER* timer) {
    HuPrcKill(timer->proc);
    KillSprite(timer);
    if (timer->boxId != HUSPR_GROUP_NONE) {
        MgScoreBoxKill(timer->boxId);
    }
    if (timer->gameMesId != GAMEMES_ID_NONE) {
        GameMesKill(timer->gameMesId);
    }
    HuMemDirectFree(timer);
}

int MgTimerModeGet(MGTIMER* timer) {
    return timer->mode;
}

/* Minigame setup sets the starting, ending, and record frame counts before starting the timer. */
void MgTimerParamSet(MGTIMER* timer, int maxTime, int endTime, int recordTime) {
    timer->maxTime = maxTime;
    timer->time = timer->maxTime;
    timer->endTime = endTime;
    timer->recordTime = recordTime;
    timer->speed = (maxTime == endTime) ? 0 : ((maxTime > endTime) ? -1 : 1);
    UpdateSprite(timer);
}

int MgTimerValueGet(MGTIMER* timer) {
    return timer->time;
}

/* Minigame UI code moves the timer display in screen-pixel coordinates. */
void MgTimerPosSet(MGTIMER *timer, float posX, float posY) {
    timer->pos.x = posX;
    timer->pos.y = posY;
    UpdateSprite(timer);
}

/* Minigame UI code reads the timer display position in screen pixels. */
void MgTimerPosGet(MGTIMER* timer, float *posX, float *posY) {
    *posX = timer->pos.x;
    *posY = timer->pos.y;
}

/* Minigame control code starts counting and selects what the display does at the end. */
void MgTimerModeOnSet(MGTIMER* timer, int offType) {
    timer->offType = offType;
    timer->stopF = FALSE;
    timer->mode = MGTIMER_MODE_ON;
}

/* Minigame control code requests the timer process to enter its idle mode. */
void MgTimerModeOffSet(MGTIMER* timer) {
    timer->mode = MGTIMER_MODE_OFF;
}

BOOL MgTimerDoneCheck(MGTIMER* timer) {
    return timer->endTime == timer->time;
}

/* Record-mode callers update the best time; -1 asks this function to compare the live time. */
void MgTimerRecordSet(MGTIMER* timer, int recordTime) {
    if (recordTime == -1) {
        if (((timer->speed > 0) && (timer->time < timer->recordTime)) ||
            ((timer->speed < 0) && (timer->time > timer->recordTime))) {
            timer->recordTime = timer->time;
            timer->flashFlag = timer->flashFlag | MGTIMER_FLASH_RECORD;
        }
    } else {
        timer->recordTime = recordTime;
        timer->flashFlag = timer->flashFlag | MGTIMER_FLASH_RECORD;
    }
}

/* Minigame UI code changes the RGB color used by the seven live timer digits. */
void MgTimerColorSet(MGTIMER* timer, u8 red, u8 green, u8 blue) {
    int spriteIndex;
    timer->digitR = red;
    timer->digitG = green;
    timer->digitB = blue;
    for (spriteIndex = 0; spriteIndex <= 6; spriteIndex++) {
        if (timer->espId[spriteIndex] != -1) {
            espColorSet(timer->espId[spriteIndex], red, green, blue);
        }
    }
}

/* Minigame UI code changes the score-box background color when the timer has a box. */
void MgTimerBackColorSet(MGTIMER* timer, u8 red, u8 green, u8 blue) {
    if (timer->boxId != HUSPR_GROUP_NONE) {
        MgScoreBoxColorSet(timer->boxId, red, green, blue);
    }
}

/* Child-process entry point created by MgTimerCreate; dispatches the selected timer mode. */
static void TimerExec(void) {
    MGTIMER* timer;

    timer = HuPrcCurrentGet()->property;
    while (1) {
        modeTbl[timer->mode](timer);
    }
}

/* modeTbl idle callback: refreshes the display and checks for a mode change once per frame. */
static void TimerExecOff(MGTIMER* timer) {
    int pollIndex;
    s16 currentMode;
    int pollCount;
    s16 previousMode;

    while (1) {
        UpdateSprite(timer);
        for(pollCount=1, pollIndex=0; pollIndex<pollCount; pollIndex++) {
            HuPrcVSleep();
            currentMode = ((MGTIMER *)HuPrcCurrentGet()->property)->mode;
            previousMode = ((MGTIMER *)HuPrcCurrentGet()->property)->prevMode;
            ((MGTIMER *)HuPrcCurrentGet()->property)->prevMode = currentMode;
            if (currentMode != previousMode) {
                return;
            }
        }
    }
}

/* modeTbl active callback: advances one frame per tick and applies the selected end behavior. */
static void TimerExecOn(MGTIMER* timer) {
    int pollIndex;
    s16 currentMode;
    int pollCount;
    s16 previousMode;

    if (timer->dispOnF == 0) {
        MgTimerDispOn(timer);
    }
    while ((timer->time != timer->endTime) && (timer->stopF == 0)) {
        timer->time = timer->time + timer->speed;
        UpdateSprite(timer);
        for(pollCount=1, pollIndex=0; pollIndex<pollCount; pollIndex++) {
            HuPrcVSleep();
            currentMode = ((MGTIMER*)HuPrcCurrentGet()->property)->mode;
            previousMode = ((MGTIMER*)HuPrcCurrentGet()->property)->prevMode;
            ((MGTIMER*)HuPrcCurrentGet()->property)->prevMode = currentMode;
            if (currentMode != previousMode) {
                return;
            }
        }
    }
    timer->stopF = TRUE;
    UpdateSprite(timer);
    timer->fadeOutTime = 0;
    switch (timer->offType) {
        case MGTIMER_OFFTYPE_NORMAL:
            timer->mode = MGTIMER_MODE_OFF;
            return;

        case MGTIMER_OFFTYPE_FADEOUT:
            if (timer->type == MGTIMER_TYPE_NORMAL) {
                MgTimerDispOff(timer);
            } else {
                timer->flashFlag = timer->flashFlag | MGTIMER_FLASH_FADEOUT;
            }
            timer->mode = MGTIMER_MODE_OFF;
            return;

        case MGTIMER_OFFTYPE_FLASH:
            timer->flashFlag = timer->flashFlag | MGTIMER_FLASH_COLOR;
            timer->mode = MGTIMER_MODE_OFF;
            break;
    }
}

/* MgTimerCreate builds the digit and crown sprites for score-style timer displays. */
static void CreateSprite(MGTIMER* timer) {
    int spriteIndex;

    switch (timer->type) {
        case MGTIMER_TYPE_NORMAL:
            break;

        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_WIDESCORE:
            for (spriteIndex = 0; spriteIndex < 7; spriteIndex++) {
                timer->espId[spriteIndex] = espEntry(MGCONST_ANM_scoreSmall, 0, 0);
                espColorSet(timer->espId[spriteIndex], 255, 255, 255);
            }
            for (spriteIndex = 0; spriteIndex < 7; spriteIndex++) {
                timer->espId[spriteIndex + 7] = espEntry(MGCONST_ANM_scoreSmall, 0, 0);
                espColorSet(timer->espId[spriteIndex + 7], 66, 255, 122);
            }
            timer->espId[14] = espEntry(MGCONST_ANM_crown, 0, 0);
            espBankSet(timer->espId[1], 10);
            espBankSet(timer->espId[4], 11);
            espBankSet(timer->espId[8], 10);
            espBankSet(timer->espId[11], 11);
            for (spriteIndex = 0; spriteIndex < 15; spriteIndex++) {
                if (timer->espId[spriteIndex] != -1) {
                    espDispOff(timer->espId[spriteIndex]);
                    espPriSet(timer->espId[spriteIndex], 9);
                }
            }
            break;
    }
}

/* MgTimerKill releases every sprite created for this timer. */
static void KillSprite(MGTIMER* timer) {
    int spriteIndex;
    for (spriteIndex = 0; spriteIndex < 15; spriteIndex++) {
        if (timer->espId[spriteIndex] != -1) {
            espKill(timer->espId[spriteIndex]);
            timer->espId[spriteIndex] = -1;
        }
    }
}

/* Called at setup and each timer tick to position digits and display the current and record
 * times. */
static void UpdateSprite(MGTIMER* timer) {
    int spriteIndex;
    s32 seconds;
    s32 centiseconds;
    s32 minutes;
    s32 elapsedFrames;
    u8 red;
    u8 green;
    u8 blue;
    float transparencyLevel;

    switch (timer->type) {
        case MGTIMER_TYPE_NORMAL:
            /* The message timer displays whole seconds, so update its value only at 60-frame
             * boundaries. */
            if ((timer->time % 60) == 0) {
                GameMesTimerValueSet(timer->gameMesId, timer->time / 60);
            }
            GameMesPosSet(timer->gameMesId, timer->pos.x, timer->pos.y);
            break;

        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_WIDESCORE:
            for (spriteIndex = 0; spriteIndex < 15; spriteIndex++) {
                if (timer->espId[spriteIndex] != -1) {
                    espPosSet(timer->espId[spriteIndex],
                              timer->pos.x + digitOfsTbl[timer->type][spriteIndex].x,
                              timer->pos.y + digitOfsTbl[timer->type][spriteIndex].y);
                }
            }
            if (timer->boxId != -1) {
                MgScoreBoxPosSet(timer->boxId, timer->pos.x - 8.0f, timer->pos.y - 8.0f);
            }
            /* The live timer digits cap at 9:59.99; record-time digits are formatted separately
             * below. */
            if (timer->time >= 35999) {
                minutes = 9;
                seconds = 59;
                centiseconds = 99;
            } else {
                /* The enclosing branch already guarantees this value is below the display cap. */
                elapsedFrames = (timer->time >= 35999) ? 35999 : timer->time;
                minutes = elapsedFrames / (timer->timeUnit * 60);
                seconds = (elapsedFrames % (timer->timeUnit * 60)) / 60;
                /* Convert the remaining sub-second frames to truncated centiseconds using 100/60;
                 * the record-time display uses the same conversion below. */
                centiseconds = 1.6666666f * ((elapsedFrames % 3600) % 60);
            }
            espBankSet(timer->espId[0], minutes % 10);
            espBankSet(timer->espId[2], seconds / 10);
            espBankSet(timer->espId[3], seconds % 10);
            espBankSet(timer->espId[5], centiseconds / 10);
            espBankSet(timer->espId[6], centiseconds % 10);

            minutes = timer->recordTime / (timer->timeUnit * 60);
            seconds = (timer->recordTime % (timer->timeUnit * 60)) / 60;
            centiseconds = 1.6666666f * ((timer->recordTime % 3600) % 60);
            espBankSet(timer->espId[7], minutes % 10);
            espBankSet(timer->espId[9], seconds / 10);
            espBankSet(timer->espId[10], seconds % 10);
            espBankSet(timer->espId[12], centiseconds / 10);
            espBankSet(timer->espId[13],  centiseconds - (centiseconds / 10 * 10));
            if (timer->flashFlag & MGTIMER_FLASH_COLOR) {
                if (((GlobalCounter / 10) & 1) == 0) {
                    red = timer->digitR;
                    green = timer->digitG;
                    blue = timer->digitB;
                } else {
                    red = 255;
                    green = 216;
                    blue = 0;
                }
                for (spriteIndex = 0; spriteIndex < 7; spriteIndex++) {
                    if (timer->espId[spriteIndex] != -1) {
                        espColorSet(timer->espId[spriteIndex], red, green, blue);
                    }
                }
            }
            if (timer->flashFlag & MGTIMER_FLASH_RECORD) {
                if (timer->scaleDir) {
                    if ((timer->digitScale += 0.02f) >= 1.2f) {
                        timer->digitScale = 1.2f;
                        timer->scaleDir = 0;
                    }
                } else {
                    if ((timer->digitScale -= 0.04f) <= 1.0f) {
                        timer->digitScale = 1.0f;
                        timer->scaleDir = 1;
                    }
                }
                for (spriteIndex = 0; spriteIndex < 8; spriteIndex++) {
                    if (timer->espId[spriteIndex+7] != -1) {
                        espScaleSet(timer->espId[spriteIndex + 7], timer->digitScale,
                                    timer->digitScale);
                    }
                }
            }
            if (timer->flashFlag & MGTIMER_FLASH_FADEOUT) {
                transparencyLevel = 1.0f - (0.016666668f * timer->fadeOutTime);
                for (spriteIndex = 0; spriteIndex < 15; spriteIndex++) {
                    if (timer->espId[spriteIndex] != -1) {
                        espTPLvlSet(timer->espId[spriteIndex], transparencyLevel);
                    }
                    if (timer->boxId != -1) {
                        MgScoreBoxTPLvlSet(timer->boxId, transparencyLevel);
                    }
                }
                /* Opacity falls by 1/60 per update; when the counter is already 60, the next update
                 * reaches zero, clears the fade flag, and hides the display. */
                if (timer->fadeOutTime++ >= 60) {
                    timer->flashFlag = timer->flashFlag & ~MGTIMER_FLASH_FADEOUT;
                    MgTimerDispOff(timer);
                }
            }
    }
    (void)seconds;
}

/* Minigame callers show the timer message or all timer sprites and its score box. */
void MgTimerDispOn(MGTIMER* timer) {
    int spriteIndex;

    switch (timer->type) {
        case MGTIMER_TYPE_NORMAL:
            if (timer->gameMesId == GAMEMES_ID_NONE) {
                timer->gameMesId = GameMesTimerCreate(timer->time / 60);
                GameMesPosSet(timer->gameMesId, timer->pos.x, timer->pos.y);
            }
            break;

        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_WIDESCORE:
            for (spriteIndex = 0; spriteIndex < 15; spriteIndex++) {
                if (timer->espId[spriteIndex] != -1) {
                    espDispOn(timer->espId[spriteIndex]);
                }
            }
            if (timer->boxId != -1) {
                MgScoreBoxDispSet(timer->boxId, timer->boxDispF);
            }
            break;

    }
    timer->dispOnF = TRUE;
}

/* Minigame callers hide the timer message or all timer sprites and its score box. */
void MgTimerDispOff(MGTIMER* timer) {
    int spriteIndex;

    switch (timer->type) {
        case MGTIMER_TYPE_NORMAL:
            if (timer->gameMesId != GAMEMES_ID_NONE) {
                GameMesTimerEnd(timer->gameMesId);
                timer->gameMesId = GAMEMES_ID_NONE;
            }
            break;

        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_WIDESCORE:
            for (spriteIndex = 0; spriteIndex < 15; spriteIndex++) {
                if (timer->espId[spriteIndex] != -1) {
                    espDispOff(timer->espId[spriteIndex]);
                }
            }
            if (timer->boxId != -1) {
                MgScoreBoxDispSet(timer->boxId, FALSE);
            }
            break;
    }
    timer->dispOnF = FALSE;
}

/* Record-screen callers show the timer message or record-style digits and score box. */
void MgTimerRecordDispOn(MGTIMER* timer) {
    int spriteIndex;

    switch (timer->type) {
        case MGTIMER_TYPE_NORMAL:
            if (timer->gameMesId == GAMEMES_ID_NONE) {
                timer->gameMesId = GameMesTimerCreate(timer->time / 60);
                GameMesPosSet(timer->gameMesId, timer->pos.x, timer->pos.y);
            }
            break;

        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_WIDESCORE:
            for (spriteIndex = 0; spriteIndex < 15; spriteIndex++) {
                if (timer->espId[spriteIndex] != -1) {
                    espDispOn(timer->espId[spriteIndex]);
                }
            }
            if (timer->boxId != HUSPR_GROUP_NONE) {
                MgScoreBoxDispSet(timer->boxId, timer->boxDispF);
            }
            break;
    }
    timer->dispOnF = TRUE;
}

/* Record-screen callers hide the timer message or record-style digits and score box. */
void MgTimerRecordDispOff(MGTIMER* timer) {
    int spriteIndex;

    switch (timer->type) {
        case MGTIMER_TYPE_NORMAL:
            if(timer->gameMesId != GAMEMES_ID_NONE) {
                GameMesTimerEnd(timer->gameMesId);
                timer->gameMesId = GAMEMES_ID_NONE;
            }
            break;

        case MGTIMER_TYPE_RECORD:
        case MGTIMER_TYPE_SCORE:
        case MGTIMER_TYPE_WIDESCORE:
            for(spriteIndex = 0; spriteIndex < 15; spriteIndex++) {
                if (timer->espId[spriteIndex] != -1) {
                    espDispOff(timer->espId[spriteIndex]);
                }
            }
            if(timer->boxId != -1) {
                MgScoreBoxDispSet(timer->boxId, FALSE);
            }
    }

    timer->dispOnF = FALSE;
}
