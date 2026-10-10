// Board player-selection roulette used by capsule effects.
#define _MATH_H
#include "dolphin/math.h"

#include "game/board/roulette.h"
#include "game/board/audio.h"
#include "game/board/player.h"
#include "game/board/status.h"
#include "game/board/pause.h"

#include "game/sprite.h"
#include "game/pad.h"
#include "game/flag.h"
#include "game/frand.h"

#include "humath.h"
#include "math.h"

#define ROULETTE_SE_SPIN MSM_SE_BRD00_09
#define ROULETTE_SE_RESULT MSM_SE_BRD00_11

static s8 rouletteAngleNo = -1;
static HUSPR_GROUPID rouletteSprGrpId = HUSPR_GRP_NONE;
static int rouletteSeNo = MSM_SENO_NONE;
static float rouletteDecel = 0.92f;

static s8 rouletteChoiceTbl[GW_PLAYER_MAX];
static s16 rouletteComDelay; // CPU target pointer angle in degrees.
static s16 rouletteCounter;
static float rouletteMaxSpeed;
static s16 rouletteComValue;
static float rouletteOffset;
static s16 rouletteComValueF;
static u8 rouletteCapsuleF;
static OMOBJ *rouletteOMObj;

typedef struct RouletteWork_s {
    unsigned killF : 1; // Set when the object-manager callback should remove the roulette.
    unsigned stopF : 1; // Set after the player or timeout requests the spin to stop.
    unsigned mode : 4; // State: 0 initializes, 1 opens, 2 spins, 3 reveals, 4 closes.
    u8 maxPlayer; // Number of candidate player sectors; the creator currently sets this to three.
    s8 playerNo; // Player whose input or CPU behavior controls the spin.
    s8 time; // Animation phase in degrees; changes by 3 per update between 0 and 90.
    u8 hideTime; // Frames elapsed while blinking the selected player's sprite.
    // Spin-time raw angle divided by sector width; this can exceed the candidate range.
    u8 angleNo;
    s16 maxCounter; // Callback-frame cap before forced stop input: 180 for CPU, 3600 for a human.
    s16 angle; // Pointer angle in sixteenths of a degree during the spin, then whole degrees during
               // reveal.
    s16 speed; // Angular speed in sixteenths of a degree per frame.
} ROULETTE_WORK;

static void RouletteOMExec(OMOBJ *obj);
static void RouletteInit(ROULETTE_WORK *work);
static void RouletteWait(ROULETTE_WORK *work);
static void RouletteResultDisp(ROULETTE_WORK *work);
static void RouletteDispOn(ROULETTE_WORK *work);
static void RouletteDispOff(ROULETTE_WORK *work);
static void RouletteKill(void);
static void RouletteChoiceTblGet(int maxPlayer, int playerNo, s8 *result);
static int RoulettePadGet(ROULETTE_WORK *work);
static s16 RouletteComDelayGet(int playerNo, int maxPlayer);
static int RouletteComPadGet(ROULETTE_WORK *work);
static int RouletteAngleNoGet(ROULETTE_WORK *work);
static void RoulettePauseHook(BOOL pauseF);

// Called by the public roulette creators; builds candidates and starts the display when at least
// two are available.
static BOOL RouletteCreate(int playerNo, int maxPlayer, BOOL capsuleF)
{
    ROULETTE_WORK *rouletteWork;
    int playerIndex;
    int choiceCount;
    if(capsuleF) {
        maxPlayer = 3;
        rouletteCapsuleF = TRUE;
    } else {
        rouletteCapsuleF = FALSE;
    }
    if(rouletteCapsuleF) {
        rouletteDecel = ((frand() & 0x7F)*(1.0f/127.0f)*0.05f)+0.92f;
    } else {
        rouletteDecel = 0.92f;
    }
    // The requested maximum is overwritten; the candidate list always starts from three seats.
    maxPlayer = 3;
    for(playerIndex=0; playerIndex<GW_PLAYER_MAX; playerIndex++) {
        rouletteChoiceTbl[playerIndex] = -1;
    }
    if(!rouletteCapsuleF) {
        for(playerIndex=0, choiceCount=0; playerIndex<GW_PLAYER_MAX; playerIndex++) {
            if(maxPlayer != 3 || playerIndex != playerNo) {
                rouletteChoiceTbl[choiceCount++] = playerIndex;
            }
        }
    } else {
        for(playerIndex=0, choiceCount=0; playerIndex<3; playerIndex++) {
            rouletteChoiceTbl[choiceCount++] = playerIndex;
        }
    }
    maxPlayer = choiceCount;
    if(maxPlayer <= 1) {
        if(choiceCount == 0) {
            rouletteChoiceTbl[0] = playerNo;
        }
        rouletteAngleNo = 0;
        return FALSE;
    }
    rouletteOMObj = omAddObj(mbObjMan, 259, 0, 0, RouletteOMExec);
    rouletteWork = omObjGetWork(rouletteOMObj, ROULETTE_WORK);
    rouletteWork->killF = FALSE;
    rouletteWork->stopF = FALSE;
    rouletteWork->maxPlayer = maxPlayer;
    rouletteWork->playerNo = playerNo;
    rouletteWork->time = 0;
    rouletteWork->hideTime = 0;
    rouletteWork->angleNo = 0;
    rouletteWork->angle = 0;
    rouletteMaxSpeed = 18;
    if(GwPlayer[rouletteWork->playerNo].comF) {
        rouletteWork->maxCounter = 180;
    } else {
        rouletteWork->maxCounter = 3600;
    }
    return TRUE;
}

BOOL mbRouletteCreate(int playerNo, int maxPlayer)
{
    return RouletteCreate(playerNo, maxPlayer, FALSE);
}

BOOL mbRouletteKaneCreate(int playerNo)
{
    return RouletteCreate(playerNo, 3, TRUE);
}

BOOL mbRouletteCheck(void)
{
    return (rouletteOMObj != NULL) ? FALSE : TRUE;
}

// Sleeps the calling board process one frame at a time until the roulette object is deleted.
void mbRouletteWait(void)
{
    while(!mbRouletteCheck()) {
        HuPrcVSleep();
    }
}

int mbRouletteResultGet(void)
{
    return rouletteChoiceTbl[rouletteAngleNo];
}

// Sets the spin's acceleration cap and pitch reference; nonpositive requests use 18.
// Creation resets it to 18; the spin applies the cap only while accelerating below 18
// degrees/frame.
void mbRouletteMaxSpeedSet(float speed)
{
    if(speed <= 0) {
        speed = 18;
    }
    rouletteMaxSpeed = speed;
}

// Object-manager callback that advances the roulette states and deletes the object on exit.
static void RouletteOMExec(OMOBJ *obj)
{
    ROULETTE_WORK *work = omObjGetWork(obj, ROULETTE_WORK);
    if(work->killF || mbExitCheck()) {
        RouletteKill();
        rouletteOMObj = NULL;
        omDelObjEx(HuPrcCurrentGet(), obj);
        return;
    }
    if(rouletteCounter < work->maxCounter) {
        rouletteCounter++;
    }
    switch(work->mode) {
        case 0:
            RouletteInit(work);
            break;

        case 1:
            RouletteDispOn(work);
            break;

        case 2:
            RouletteWait(work);
            break;

        case 3:
            RouletteResultDisp(work);
            break;

        case 4:
            RouletteDispOff(work);
            break;
    }
}

static int rouletteFileTbl[6] = {
    DATANUM(DATA_board, 117),
    DATANUM(DATA_board, 120),
    DATANUM(DATA_board, 119),
    DATANUM(DATA_board, 120),
    DATANUM(DATA_board, 118),
    DATANUM(DATA_board, 118),
};

static s8 roulettePrioTbl[8+GW_PLAYER_MAX] = {
    60, 80, 80, 80, 20, 30, 70, 70, 70, 70,
};

static int charFileTbl[GW_CHARA_MAX] = {
    DATANUM(DATA_board, 121), DATANUM(DATA_board, 122),
    DATANUM(DATA_board, 123), DATANUM(DATA_board, 124),
    DATANUM(DATA_board, 125), DATANUM(DATA_board, 126),
    DATANUM(DATA_board, 127), DATANUM(DATA_board, 128),
    DATANUM(DATA_board, 129), DATANUM(DATA_board, 130),
    DATANUM(DATA_board, 131), DATANUM(DATA_board, 130),
    DATANUM(DATA_board, 130), DATANUM(DATA_board, 130),
};

static int rouletteCapsuleFileTbl[4] = {
    DATANUM(DATA_board, 121),
    DATANUM(DATA_board, 122),
    DATANUM(DATA_board, 123),
    DATANUM(DATA_board, 124),
};

// Called by RouletteOMExec in state 0; creates the roulette sprites and computes the CPU
// target angle.
static void RouletteInit(ROULETTE_WORK *work)
{
    // This unused color table is not applied; the roulette sprites use their file colors.
    GXColor playerColorTbl[13] = {
        { 227, 67, 67, 255 },
        { 68, 67, 227, 255 },
        { 241, 158, 220, 255 },
        { 67, 228, 68, 255 },
        { 138, 60, 180, 255 },
        { 227, 228, 68, 255 },
        { 40, 40, 40, 255 },
        { 227, 227, 227, 255 },
        { 40, 227, 227, 255 },
        { 227, 139, 40, 255 },
        { 180, 40, 40, 255 },
        { 40, 180, 40, 255 },
        { 40, 40, 180, 255 },
    };

    s16 spriteIndex;
    s16 choiceCount;
    HUSPRID spriteId;

    s16 cpuDelayOffset;

    float unusedRate;
    float sectorAngle;

    rouletteSeNo = MSM_SENO_NONE;
    rouletteCounter = 0;
    switch(work->maxPlayer) {
        case 3:
            choiceCount = 3;
            unusedRate = 60;
            break;

        case 2:
            choiceCount = 2;
            unusedRate = 60;
            break;
    }
    rouletteSprGrpId = HuSprGrpCreate(choiceCount+9);
    RouletteChoiceTblGet(work->maxPlayer, work->playerNo, rouletteChoiceTbl);
    if(GwPlayer[work->playerNo].comF) {
        rouletteComDelay = RouletteComDelayGet(work->playerNo, work->maxPlayer);
        // CPU difficulty sets the positive angle-offset range; a random 0–19 degrees is then
        // subtracted before the result is wrapped.
        switch(GwPlayer[work->playerNo].comDif) {
            case GW_PLAYER_COM_DIF_EASY:
                cpuDelayOffset = mbRandMod(90);
                break;

            case GW_PLAYER_COM_DIF_NORMAL:
                cpuDelayOffset = mbRandMod(60);
                break;

            case GW_PLAYER_COM_DIF_HARD:
                cpuDelayOffset = mbRandMod(30);
                break;

            case GW_PLAYER_COM_DIF_VERYHARD:
                cpuDelayOffset = mbRandMod(10);
                break;
        }
        cpuDelayOffset -= mbRandMod(20);
        rouletteComDelay += cpuDelayOffset;
        if(rouletteComDelay > 360.0f) {
            rouletteComDelay -= 360.0f;
        }
        if(rouletteComDelay < 0) {
            rouletteComDelay += 360.0f;
        }
    } else {
        rouletteComDelay = 0;
    }
    for(spriteIndex=0; spriteIndex<=4; spriteIndex++) {
        mbSprCreate(mbBoardDataNumGet(rouletteFileTbl[spriteIndex]), roulettePrioTbl[spriteIndex],
                    NULL, &spriteId);
        HuSprGrpMemberSet(rouletteSprGrpId, spriteIndex, spriteId);
        HuSprAttrSet(rouletteSprGrpId, spriteIndex, HUSPR_ATTR_LINEAR);
    }
    HuSprZRotSet(rouletteSprGrpId, 4, 180);
    for(spriteIndex=1; spriteIndex<4; spriteIndex++) {
        HuSprDispOff(rouletteSprGrpId, spriteIndex);
    }
    switch(work->maxPlayer) {
        case 4:
            HuSprDispOn(rouletteSprGrpId, 3);
            break;

        case 3:
            HuSprDispOn(rouletteSprGrpId, 2);
            break;

        case 2:
            HuSprDispOn(rouletteSprGrpId, 1);
            break;
    }
    for(spriteIndex=0; spriteIndex<choiceCount; spriteIndex++) {
        if(!rouletteCapsuleF) {
            s16 characterNo = GwPlayer[rouletteChoiceTbl[spriteIndex]].charNo;
            mbSprCreate(mbBoardDataNumGet(charFileTbl[characterNo]),
                        roulettePrioTbl[spriteIndex + 5], NULL, &spriteId);
        } else {
            mbSprCreate(mbBoardDataNumGet(rouletteCapsuleFileTbl[rouletteChoiceTbl[spriteIndex]]),
                        roulettePrioTbl[spriteIndex + 5], NULL, &spriteId);
        }
        HuSprGrpMemberSet(rouletteSprGrpId, spriteIndex+5, spriteId);
        HuSprAttrSet(rouletteSprGrpId, spriteIndex+5, HUSPR_ATTR_LINEAR);
    }
    HuSprGrpPosSet(rouletteSprGrpId, 288, 240);
    HuSprGrpScaleSet(rouletteSprGrpId, 0.01f, 0.01f);
    OSs16tof32(&choiceCount, &sectorAngle);
    // The sector angle is calculated but the sprite positions below are fixed pixel offsets.
    sectorAngle = 360/sectorAngle;
    for(spriteIndex=0; spriteIndex<choiceCount; spriteIndex++) {
        switch(work->maxPlayer) {
            case 3:
                switch(spriteIndex) {
                    case 0:
                        HuSprPosSet(rouletteSprGrpId, spriteIndex+5, 50, -35);
                        break;

                    case 1:
                        HuSprPosSet(rouletteSprGrpId, spriteIndex+5, 0, 57);
                        break;

                    case 2:
                        HuSprPosSet(rouletteSprGrpId, spriteIndex+5, -50, -35);
                        break;
                }
                break;

            case 2:
                switch(spriteIndex) {
                    case 0:
                        HuSprPosSet(rouletteSprGrpId, spriteIndex+5, 50, 0);
                        break;

                    case 1:
                        HuSprPosSet(rouletteSprGrpId, spriteIndex+5, -50, 0);
                        break;
                }
                break;
        }
    }
    mbPauseHookPush(RoulettePauseHook);
    for(spriteIndex=0; spriteIndex<3; spriteIndex++) {
        if(rouletteChoiceTbl[spriteIndex] == rouletteComValue) {
            break;
        }
    }
    rouletteOffset = spriteIndex*120.0f;
    if(rouletteOffset > 180) {
        rouletteOffset -= 360;
    }
    work->mode = 1;
}

// Advances the spin and reads stop input only at 18 degrees per frame or above. A cap below 18
// during acceleration prevents stop input. After a stop request, damps the speed but boosts
// near-stationary motion within two degrees of sector boundaries to keep the pointer moving.
static void RouletteWait(ROULETTE_WORK *work)
{
    float angularSpeed;
    float pointerAngle;

    float soundPitch;

    s16 pointerDegrees;
    s16 sectorWidth;
    u32 pressedButtons;
    s16 soundPitchValue;

    OSs16tof32(&work->angle, &pointerAngle);
    OSs16tof32(&work->speed, &angularSpeed);
    pointerAngle = 0.0625f*pointerAngle;
    angularSpeed = 0.0625f*angularSpeed;
    if(!work->stopF) {
        if(angularSpeed < 18) {
            angularSpeed += 0.7f;
            if(angularSpeed > rouletteMaxSpeed) {
                angularSpeed = rouletteMaxSpeed;
            }
        } else {
            pressedButtons = RoulettePadGet(work);
            if(pressedButtons & PAD_BUTTON_A) {
                work->stopF = TRUE;
            }
        }
    } else {
        OSf32tos16(&pointerAngle, &pointerDegrees);
        switch(work->maxPlayer) {
            case 3:
                sectorWidth = 120;
                break;

            case 2:
                sectorWidth = 180;
                break;
        }
        pointerDegrees %= sectorWidth;
        if(angularSpeed < 0.5f && (pointerDegrees < 2 || pointerDegrees >= sectorWidth-2)) {
            angularSpeed += angularSpeed/2;
        }
        angularSpeed *= rouletteDecel;
    }
    if(work->angleNo != RouletteAngleNoGet(work)) {
        work->angleNo = RouletteAngleNoGet(work);
    }
    soundPitch = 8191-(8191*(angularSpeed/rouletteMaxSpeed));
    OSf32tos16(&soundPitch, &soundPitchValue);
    HuAudFXPitchSet(rouletteSeNo, -soundPitchValue);
    if(angularSpeed > -0.0000001f && angularSpeed < 0.0000001f) {
        work->mode = 3;
        if(rouletteSeNo != MSM_SENO_NONE) {
            mbAudFXStop(rouletteSeNo);
            rouletteSeNo = MSM_SENO_NONE;
        }
        mbAudFXPlay(ROULETTE_SE_RESULT);
    }
    pointerAngle += angularSpeed;
    if(pointerAngle > 360) {
        pointerAngle -= 360;
    }
    HuSprZRotSet(rouletteSprGrpId, 4, fmod(180+pointerAngle, 360));
    pointerAngle *= 16;
    angularSpeed *= 16;
    OSf32tos16(&pointerAngle, &work->angle);
    OSf32tos16(&angularSpeed, &work->speed);
}

// Called by RouletteOMExec in state 3; blinks the selected player for 90 frames before closing.
static void RouletteResultDisp(ROULETTE_WORK *work)
{
    u8 hideTime;
    if(work->hideTime == 0) {
        work->angle >>= 4;
        rouletteAngleNo = RouletteAngleNoGet(work);
    }
    hideTime = work->hideTime%6;
    if(hideTime < 3) {
        HuSprDispOff(rouletteSprGrpId, rouletteAngleNo+5);
    } else {
        HuSprDispOn(rouletteSprGrpId, rouletteAngleNo+5);
    }
    if(work->hideTime < 90) {
        work->hideTime++;
    } else {
        HuSprDispOn(rouletteSprGrpId, rouletteAngleNo+5);
        work->mode = 4;
    }
}

// Called by RouletteOMExec in state 1; grows the roulette into view and starts its spin sound.
static void RouletteDispOn(ROULETTE_WORK *work)
{
    float angle;
    if(work->time < 90) {
        work->time += 3;
    } else {
        _CheckFlag(FLAG_BOARD_TUTORIAL); // The check result is intentionally unused here.
        work->time = 90;
        work->mode = 2;
        rouletteSeNo = mbAudFXPlay(ROULETTE_SE_SPIN);
    }
    OSs8tof32(&work->time, &angle);
    HuSprGrpScaleSet(rouletteSprGrpId, HuSin(angle), HuSin(angle));
}

// Called by RouletteOMExec in state 4; shrinks the roulette and requests object cleanup.
static void RouletteDispOff(ROULETTE_WORK *work)
{
    float angle;
    if(work->time > 0) {
        work->time -= 3;
    } else {
        work->time = 0;
        work->killF = TRUE;
        // The four-bit state stores 15; killF causes deletion on the next object update.
        work->mode = 255;
        rouletteComValueF = FALSE;
    }
    OSs8tof32(&work->time, &angle);
    HuSprGrpScaleSet(rouletteSprGrpId, HuSin(angle), HuSin(angle));
}

// Called by RouletteOMExec during deletion; removes the pause hook and destroys the sprite group.
static void RouletteKill(void)
{
    if(rouletteSprGrpId != HUSPR_GRP_NONE) {
        mbPauseHookPop(RoulettePauseHook);
        HuSprGrpKill(rouletteSprGrpId);
        rouletteSprGrpId = HUSPR_GRP_NONE;
    }
}

// Swaps random candidate entries before the roulette is drawn; playerNo is unused, and the trailing
// loop has no effect.
static void RouletteChoiceTblGet(int maxPlayer, int playerNo, s8 *result)
{
    int swapAttempt;
    for(swapAttempt=0; swapAttempt<255; swapAttempt++) {
        int firstChoiceIndex = (frand() & 0x7FFF)%maxPlayer;
        int secondChoiceIndex = (frand() & 0x7FFF)%maxPlayer;
        if(firstChoiceIndex != secondChoiceIndex) {
            s8 firstPlayer = result[firstChoiceIndex];
            result[firstChoiceIndex] = result[secondChoiceIndex];
            result[secondChoiceIndex] = firstPlayer;
        }
    }
    for(swapAttempt=0; swapAttempt<maxPlayer; swapAttempt++) {

    }
}

// Called during state 2; timeout forces A before any input handling. Otherwise reads human input
// or CPU timing. A requested CPU target replaces the normal timing result with A or zero using
// the pointer angle plus 46 times its current speed and a fixed 120-degree sector.
static int RoulettePadGet(ROULETTE_WORK *work)
{
    int pressedButtons;
    if(rouletteCounter >= work->maxCounter) {
        return PAD_BUTTON_A;
    }
    if(!GwPlayer[work->playerNo].comF) {
        pressedButtons = HuPadBtnDown[GwPlayer[work->playerNo].padNo];
        return pressedButtons;
    } else {
        pressedButtons = RouletteComPadGet(work);
        if(rouletteComValueF) {
            ROULETTE_WORK *rouletteWork = omObjGetWork(rouletteOMObj, ROULETTE_WORK);
            float angleStart;
            float angleEnd;
            float angle;
            float speed;
            OSs16tof32(&rouletteWork->angle, &angle);
            OSs16tof32(&rouletteWork->speed, &speed);
            angle = 0.0625f*angle;
            speed = 0.0625f*speed;
            angleStart = angle+(speed*46);
            angleStart = fmod(angleStart, 360);
            if(angleStart > 180) {
                angleStart -= 360;
            }
            angleEnd = rouletteOffset-angleStart;
            // This branch adds 360 to differences already at least 360; negative differences stay
            // unwrapped.
            if(angleEnd >= 360) {
                angleEnd += 360;
            }
            if(angleEnd >= 0 && angleEnd < (120-(2*speed))) {
                pressedButtons = PAD_BUTTON_A;
            } else {
                pressedButtons = 0;
            }
        }
        return pressedButtons;
    }
}

// Requests a CPU target player; set this before RouletteInit caches the sector offset.
// Changing the value after initialization does not retarget that spin.
void mbRouletteComValueSet(s16 value)
{
    rouletteComValueF = TRUE;
    rouletteComValue = value;
}

// Called by RouletteInit for a CPU player; computes a target pointer angle in degrees from the
// best-path player in party mode, or a random other player when unavailable or current. Outside
// party mode, bestPathPlayerNo is read uninitialized.
static s16 RouletteComDelayGet(int playerNo, int maxPlayer)
{
    int bestPathPlayerNo;
    int choiceCount;
    int choiceIndex;
    s16 targetAngleDeg;
    if(GWPartyGet() == TRUE) {
        bestPathPlayerNo = mbPlayerBestPathGet();
    }
    // Outside party mode this local is read without an assignment in this function.
    if(bestPathPlayerNo == -1 || bestPathPlayerNo == playerNo) {
        do {
            bestPathPlayerNo = mbRandMod(GW_PLAYER_MAX);
        } while(bestPathPlayerNo == playerNo);

    }
    choiceCount = maxPlayer;
    for(targetAngleDeg=choiceIndex=0; choiceIndex<choiceCount; choiceIndex++) {
        if(bestPathPlayerNo == rouletteChoiceTbl[choiceIndex]) {
            float targetAngle = choiceIndex*(360/choiceCount);
            targetAngle += 45;
            targetAngle -= 180;
            if(targetAngle < 0) {
                targetAngle += 360;
            }
            if(targetAngle >= 360) {
                targetAngle -= 360;
            }
            OSf32tos16(&targetAngle, &targetAngleDeg);
            break;
        }
    }
    return targetAngleDeg;
}

// Called by RoulettePadGet for a CPU player; returns A when the pointer angle is in the raw range
// from rouletteComDelay to rouletteComDelay plus sector width, without wrapping at 360.
static int RouletteComPadGet(ROULETTE_WORK *work)
{
    int angle;
    s16 origAngle;
    int delayMax;
    int delayMin;
    origAngle = work->angle >> 4;
    switch(work->maxPlayer) {
        case 3:
            angle = 120;
            break;

        case 2:
            angle = 180;
            break;
    }
    delayMin = rouletteComDelay;
    delayMax = rouletteComDelay+angle;
    if(origAngle >= delayMin && origAngle < delayMax) {
        return PAD_BUTTON_A;
    } else {
        return 0;
    }
}

// Divides the stored sixteenths-degree angle directly during the spin; the reveal shifts it to
// degrees before using this conversion.
static int RouletteAngleNoGet(ROULETTE_WORK *work)
{
    int sectorIndex;
    switch(work->maxPlayer) {
        case 3:
            sectorIndex = work->angle/120;
            break;

        case 2:
            sectorIndex = work->angle/180;
            break;
    }
    return sectorIndex;
}

// Moves the roulette sprite group offscreen when the pause overlay opens and restores it when
// the overlay closes.
static void RoulettePauseHook(BOOL displayF)
{
    if(displayF == 0) {
        HuSprGrpPosSet(rouletteSprGrpId, 1024, 1024);
    } else {
        HuSprGrpPosSet(rouletteSprGrpId, 288, 240);
    }
}
