/* Message system for timers, minigame notices, pause prompts, and score boxes. */
#define _MATH_H
#define M_PI 3.141592653589793
double sin(double);
double cos(double);
extern inline double fabs(double x)
{
   return __fabs(x);
}

#include "game/gamemes.h"
#include "game/process.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/armem.h"
#include "game/main.h"
#include "game/hu3d.h"
#include "game/audio.h"
#include "game/window.h"
#include "game/flag.h"
#include "game/wipe.h"
#include "game/pad.h"
#include "game/objdll.h"
#include "game/mgdata.h"
#include "messdir_enum.h"

#define GAMEMES_MESS_PAUSE_HEADER MESSNUM(MESS_MG_INST_SYS, 16)
#define GAMEMES_MESS_PAUSE_EXIT_PROMPT MESSNUM(MESS_MG_INST_SYS, 9)
#define GAMEMES_MG_UNSELECTED 65535

/* Border tile codes combine with row offsets in the score panel's indirect texture. */
#define SCOREBOX_TILE_LEFT_OUTER_BASE 160
#define SCOREBOX_TILE_LEFT_MIDDLE_BASE 176
#define SCOREBOX_TILE_LEFT_INNER_BASE 192
#define SCOREBOX_TILE_RIGHT_INNER_BASE 112
#define SCOREBOX_TILE_RIGHT_MIDDLE_BASE 128
#define SCOREBOX_TILE_RIGHT_OUTER_BASE 144
#define SCOREBOX_TILE_HORIZONTAL_EDGE_BASE 96
#define SCOREBOX_TILE_VERTICAL_LEFT 208
#define SCOREBOX_TILE_VERTICAL_RIGHT 241
#define SCOREBOX_TILE_INTERIOR 210
#define SCOREBOX_MIN_SIZE 48

#define GAMEMES_GLYPH_DATA_MASK 0x7FFFFFFF
#define GAMEMES_GLYPH_RAISED_FLAG 0x80000000

typedef struct GameMesEntry_s {
    GAMEMESCREATE create; /* Initializes one message slot from its arguments. */
    GAMEMESEXEC exec; /* Updates and draws the message once per frame. */
    HuVec2f pos; /* Default screen position in pixels. */
    HuVec2f scale; /* Default horizontal and vertical display scale. */
    int timeMax; /* Default lifetime used by the message implementation. */
} GAMEMES_ENTRY;

GAMEMES GameMesData[GAMEMES_MAX];
s16 GameMesTime;
u8 GameMesCloseF;
static u8 GameMesDebugF;
unsigned int GameMesVWait;
static ANIMDATA *ScoreBoxMaskAnim;
static ANIMDATA *ScoreBoxAnim;
static BOOL pauseWaitF;
static BOOL pauseCancelF;
static BOOL pauseEnableF;
static HUPROCESS *pauseProc;
int GameMesLanguageNo;

OMOVL GameMesOvlPrev = DLL_NONE;

GAMEMES_ENTRY GameMesTbl[] = {
    { NULL, NULL, { 292, 240 }, { 1, 1 }, 60 },
    { GameMesTimerInit, GameMesTimerExec, { 292, 64 }, { 1, 1 }, 60 },
    { GameMesMg4PInit, GameMesMg4PExec, { 292, 240 }, { 1, 1 }, 180 },
    { GameMesMg2Vs2Init, GameMesMg2Vs2Exec, { 292, 240 }, { 1, 1 }, 180 },
    { GameMesMgWinInit, GameMesMgWinExec, { 292, 240 }, { 1, 1 }, 180 },
    { GameMesMg1Vs3Init, GameMesMg1Vs3Exec, { 292, 240 }, { 1, 1 }, 180 },
    { GameMesMgBattleInit, GameMesMgBattleExec, { 292, 240 }, { 1, 1 }, 180 },
    { GameMesMg4PInit, GameMesMg4PExec, { 292, 240 }, { 1, 1 }, 180 },
    { GameMesMgKoopaInit, GameMesMgKoopaExec, { 292, 240 }, { 1, 1 }, 180 },
    { GameMesMgRareInit, GameMesMgRareExec, { 292, 240 }, { 1, 1 }, 180 },
    { GameMesMgKettouInit, GameMesMgKettouExec, { 292, 240 }, { 1, 1 }, 180 },
    { GameMesMgDonkeyInit, GameMesMgDonkeyExec, { 292, 240 }, { 1, 1 }, 180 },
    { GameMesMgSdInit, GameMesMgSdExec, { 292, 240 }, { 1, 1 }, 180 },
    { GameMesMgDrawInit, GameMesMgDrawExec, { 292, 240 }, { 1, 1 }, 60 },
    { GameMesMgRecordInit, GameMesMgRecordExec, { 292, 240 }, { 1, 1 }, 180 },
    { NULL, NULL, { 292, 240 }, { 1, 1 }, 60 },
};

static s16 startMesTbl[MG_TYPE_MAX] = {
    GAMEMES_MES_MG_4P, GAMEMES_MES_MG_1VS3, GAMEMES_MES_MG_2VS2, GAMEMES_MES_MG_BATTLE,
    GAMEMES_MES_MG_KUPA, GAMEMES_MES_MG_LAST, GAMEMES_MES_MG_KETTOU, GAMEMES_MES_MG_DONKEY,
    GAMEMES_MES_MG_SD
};

static char asciiTbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static char kanaTbl[] =
    "ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝｧｨｩｪｫｬｭｮｯｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾊﾋﾌﾍﾎﾊﾋﾌﾍﾎｰ";
static char numberTbl[] = "0123456789";
static char punctTbl[] = "!?.";
static int *fontBufs[5] = {};

static void CreateFontBuf(void);

/* Called during game startup to reset message slots and load message resources. */
void GameMesInit(void)
{
    GAMEMES *message = &GameMesData[0];
    int remainingSlots;
    for(remainingSlots=GAMEMES_MAX; remainingSlots; remainingSlots--, message++) {
        message->mesNo = GAMEMES_MES_NULL;
        message->buf = NULL;
    }
    GameMesCloseF = FALSE;
    GameMesDebugF = FALSE;
    GameMesTime = 30;
    HuAR_DVDtoARAM(DATA_gamemes);
    HuAR_DVDtoARAM(DATA_mgconst);
    while(HuARDMACheck());
    CreateFontBuf();
    GameMesOvlPrev = DLL_NONE;
    GameMesLanguageNo = GWLanguageGet();
    MgScoreBoxInit();
}

void *GameMesDataRead(unsigned int dataNum)
{
    return HuAR_ARAMtoMRAMFileRead(dataNum, HU_MEMNUM_OVL, HEAP_MODEL);
}

/* Called by the main game loop to update active messages and their shared timeout. */
void GameMesExec(void)
{
    GAMEMES *message;
    GAMEMES_ENTRY *messageType;
    int otherSlot;
    u8 combinedStatus;
    int slot;
    BOOL keepMessage;
    GameMesVWait = HuSysVWaitGet(GameMesVWait);
    if(!Hu3DPauseF) {
        keepMessage = FALSE;
        combinedStatus = GAMEMES_STAT_NONE;
        message = &GameMesData[0];
        for(slot=0; slot<GAMEMES_MAX; slot++, message++) {
            if(message->stat == GAMEMES_STAT_NONE) {
                continue;
            }
            if(message->exec) {
                keepMessage = message->exec(message);
            } else {
                messageType = &GameMesTbl[message->mesNo];
                if(message->mesNo != GAMEMES_MES_NULL && NULL != messageType->exec) {
                    keepMessage = messageType->exec(message);
                }
            }
            if(keepMessage == FALSE) {
                message->stat = GAMEMES_STAT_NONE;
                if(!GameMesDebugF) {
                    /* This loop stops at another active slot, but its index is not used
                     * afterward. */
                    for(otherSlot=0; otherSlot<GAMEMES_MAX; otherSlot++) {
                        if(GameMesData[otherSlot].stat != GAMEMES_STAT_NONE) {
                            break;
                        }
                    }
                }
                if(!message->buf) {
                    /* This guard allows only a NULL buffer through to the free call. */
                    HuMemDirectFree(message->buf);
                    message->buf = NULL;
                }
            }
            combinedStatus |= message->stat;
        }
        if(combinedStatus == GAMEMES_STAT_NONE || (combinedStatus & GAMEMES_STAT_TIMEEND)) {
            if(GameMesTime > 0) {
                GameMesTime -= GameMesVWait;
            }
        }
    }
}

/* Reserves a free message slot and invokes the selected type's setup callback. */
static GAMEMESID CreateGameMes(s16 mesNo, va_list args)
{
    GAMEMES *message = &GameMesData[0];
    GAMEMES_ENTRY *messageType = &GameMesTbl[(u8)mesNo];
    int spriteSlotOrCreateResult;
    int messageId;

    for(messageId=0; messageId<GAMEMES_MAX; messageId++, message++) {
        if(message->stat == GAMEMES_STAT_NONE) {
            break;
        }
    }
    if(messageId >= GAMEMES_MAX) {
        return GAMEMES_ID_NONE;
    }
    message->stat |= GAMEMES_STAT_EXIST;
    if(message->buf) {
        HuMemDirectFree(message->buf);
    }
    message->buf = NULL;
    message->mesNo = mesNo;
    message->time = 0;
    message->pos.x = messageType->pos.x;
    message->pos.y = messageType->pos.y;
    message->scale.x = messageType->scale.x;
    /* Copies the message type's default vertical display scale. */
    message->scale.y = messageType->scale.y;
    /* Initializes this message field to zero. */
    message->unk18 = 0;
    message->color.g = 255;
    message->timeMax = messageType->timeMax;
    message->timerVal = message->subMode = message->work = message->charNum = 0;
    message->angle = message->winScale = 0;
    message->dispMode = message->dispValue = 0;
    for (spriteSlotOrCreateResult = 0; spriteSlotOrCreateResult < GAMEMES_SPRMAX;
         spriteSlotOrCreateResult++) {
        message->sprId[spriteSlotOrCreateResult] = message->grpId[spriteSlotOrCreateResult] =
            HUSPR_NONE;
    }
    if(NULL != messageType->create) {
        spriteSlotOrCreateResult = messageType->create(message, args);
        if(spriteSlotOrCreateResult == FALSE) {
            message->stat = GAMEMES_STAT_NONE;
            return GAMEMES_ID_NONE;
        }
    }
    GameMesTime = 30;
    return messageId;
}

/* Called by game, board, and minigame sequences to create a message or timer. */
GAMEMESID GameMesCreate(s16 messageTypeId, ...)
{
    GAMEMESID messageId;
    va_list args;
    va_start(args, messageTypeId);
    GameMesLanguageNo = GWLanguageGet();
    if(messageTypeId == GAMEMES_MES_MG) {
        if(GwSystem.mgNo == GAMEMES_MG_UNSELECTED) {
            messageTypeId = GAMEMES_MES_MG_4P;
        } else {
            messageTypeId = startMesTbl[MgDataTbl[GwSystem.mgNo].type];
        }
    }
    messageId = CreateGameMes(messageTypeId, args);
    va_end(args);
    return messageId;
}

/* Returns the message slot for a valid ID; callers use it to inspect or adjust it. */
GAMEMES *GameMesGet(GAMEMESID messageId)
{
    if(messageId >= 0 && messageId < GAMEMES_MAX) {
        return &GameMesData[messageId];
    }
    return NULL;
}

/* Returns one slot's status, or the combined status when passed GAMEMES_ID_NONE. */
u8 GameMesStatGet(GAMEMESID messageId)
{
    GAMEMES *message;
    u8 combinedStatus;
    combinedStatus = GAMEMES_STAT_NONE;
    if(messageId < 0) {
        int remainingSlots;
        message = &GameMesData[0];
        for(remainingSlots=GAMEMES_MAX; remainingSlots; remainingSlots--, message++) {
            combinedStatus |= message->stat;
        }
    } else {
        if(messageId < GAMEMES_MAX) {
            combinedStatus = GameMesData[messageId].stat;
        }
    }
    return combinedStatus;
}

/* Callers set a live message's screen position here for its next GameMesExec update. */
void GameMesPosSet(GAMEMESID messageId, float posX, float posY)
{
    if(messageId < 0) {
        return;
    }
    if(messageId >= GAMEMES_MAX) {
        return;
    }
    GameMesData[messageId].pos.x = posX;
    GameMesData[messageId].pos.y = posY;
}

/* Callers queue a display command here for the message's next GameMesExec update. */
void GameMesDispSet(GAMEMESID messageId, s16 displayMode, s16 displayValue)
{
    if(messageId < 0) {
        return;
    }
    if(messageId >= GAMEMES_MAX) {
        return;
    }
    GameMesData[messageId].dispMode = displayMode;
    GameMesData[messageId].dispValue = displayValue;
}

/* Callers mark a live message here so GameMesExec removes it on the next update. */
void GameMesKill(GAMEMESID messageId)
{
    if(messageId < 0) {
        return;
    }
    if(messageId >= GAMEMES_MAX) {
        return;
    }
    if(GameMesData[messageId].stat == GAMEMES_STAT_NONE) {
        return;
    }
    GameMesData[messageId].stat = GAMEMES_STAT_KILL;
}

/* Runs one final message update while shutting down the message system. */
void GameMesClose(void)
{
    GameMesCloseF = TRUE;
    GameMesExec();
    GameMesCloseF = FALSE;
    GameMesStub();
    GameMesDebugF = FALSE;
}

/* Used by callers waiting for message completion or an explicit kill request. */
BOOL GameMesKillCheck(void)
{
    u8 combinedStatus = GameMesStatGet(GAMEMES_ID_NONE);
    if (combinedStatus == GAMEMES_STAT_NONE ||
        (combinedStatus & (GAMEMES_STAT_TIMEEND | GAMEMES_STAT_KILL))) {
        if(GameMesTime <= 0 || (combinedStatus & GAMEMES_STAT_KILL)) {
            return TRUE;
        }
    }
    return FALSE;
}

void GameMesStub(void)
{

}

/* Releases all sprite groups and individual sprites owned by a message slot. */
void GameMesSprKill(GAMEMES *message)
{
    int spriteSlot;
    for(spriteSlot=0; spriteSlot<GAMEMES_SPRMAX; spriteSlot++) {
        if(message->grpId[spriteSlot] >= 0) {
            HuSprGrpKill(message->grpId[spriteSlot]);
        }
        if(message->sprId[spriteSlot] >= 0) {
            HuSprKill(message->sprId[spriteSlot]);
        }
    }
}

#define TIMER_SUBMODE_OFF 1
#define TIMER_SUBMODE_SHOW 2
#define TIMER_SUBMODE_DIGITUP 3

/* GameMesTbl setup callback: creates the two-digit timer and its background sprite. */
BOOL GameMesTimerInit(GAMEMES *message, va_list args)
{
    HUSPR_GROUPID spriteGroup;
    ANIMDATA *animation;
    HUSPRID sprite;

    int initialSeconds, screenX, screenY;
    s16 digitSlot;
    initialSeconds = va_arg(args, int);
    screenX = va_arg(args, int);
    screenY = va_arg(args, int);
    /* This condition is impossible, so the supplied time value remains unclamped here. */
    if(initialSeconds <= 0 && initialSeconds > 99) {
        initialSeconds = 99;
    }
    message->timerVal = initialSeconds;
    if(screenX >= 0) {
        message->pos.x = screenX;
    }
    if(screenY >= 0) {
        message->pos.y = screenY;
    }
    message->work = 0;
    message->subMode = TIMER_SUBMODE_SHOW;
    message->angle = 0;
    spriteGroup = HuSprGrpCreate(3);
    message->grpId[0] = spriteGroup;
    HuSprGrpScaleSet(spriteGroup, message->scale.x, message->scale.y);
    animation = HuSprAnimRead(GameMesDataRead(GAMEMES_ANM_timerDigit));
    screenX = 11;
    for(digitSlot=0; digitSlot<2; screenX -= 22, digitSlot++) {
        sprite = HuSprCreate(animation, 5, 0);
        HuSprGrpMemberSet(spriteGroup, digitSlot, sprite);
        HuSprSpeedSet(spriteGroup, digitSlot, 0);
        HuSprPosSet(spriteGroup, digitSlot, screenX, 1);
        HuSprColorSet(spriteGroup, digitSlot, 112, 233, 255);
    }
    animation = HuSprAnimRead(GameMesDataRead(GAMEMES_ANM_timerBack));
    sprite = HuSprCreate(animation, 7, 0);
    HuSprGrpMemberSet(spriteGroup, 2, sprite);
    HuSprPosSet(spriteGroup, 2, 0, 0);
    HuSprTPLvlSet(spriteGroup, 2, 1.0f);
    /* The first update moves the group to the message position. */
    HuSprGrpPosSet(spriteGroup, -100, -100);
    return TRUE;
}

/* GameMesTbl update callback: applies timer commands, animates digits, and expires the timer. */
BOOL GameMesTimerExec(GAMEMES *message)
{
    HUSPR_GROUPID spriteGroup = message->grpId[0];
    s16 digitSlot;
    u8 digits[2];
    float digitScale, transparency;
    if(message->dispMode != 0 && message->subMode != GAMEMES_SUBMODE_NULL) {
        switch(message->dispMode) {
            case GAMEMES_DISP_SET:
                switch(message->dispValue) {
                    case -1:
                        message->stat |= GAMEMES_STAT_TIMEEND;
                        message->subMode = GAMEMES_SUBMODE_NULL;
                        message->angle = 0;
                        break;

                    case 0:
                        message->subMode = TIMER_SUBMODE_SHOW;
                        message->angle = 0;
                        break;

                    case 1:
                        message->subMode = TIMER_SUBMODE_DIGITUP;
                        message->angle = 0;
                        break;
                }
                message->dispMode = GAMEMES_DISP_NONE;
                break;

            case GAMEMES_DISP_UPDATE:
                if(message->dispValue < 0 && !(message->stat & GAMEMES_STAT_TIMEEND)) {
                    message->stat |= GAMEMES_STAT_TIMEEND;
                    message->subMode = GAMEMES_SUBMODE_NULL;
                    message->angle = 0;
                } else {
                    if(message->dispValue > 99) {
                        /* Updated values are capped here, unlike the initial value in setup. */
                        message->timerVal = 99;
                    } else {
                        if(message->timerVal != message->dispValue) {
                            message->timerVal = message->dispValue;
                            if(message->dispValue <= 5) {
                                HuAudFXPlay(MSM_SE_CMN_07);
                                message->subMode = TIMER_SUBMODE_DIGITUP;
                                message->angle = 0;
                                HuSprColorSet(spriteGroup, 0, 255, 112, 160);
                                HuSprColorSet(spriteGroup, 1, 255, 112, 160);
                            } else {
                                HuSprColorSet(spriteGroup, 0, 112, 233, 255);
                                HuSprColorSet(spriteGroup, 1, 112, 233, 255);
                            }
                        }
                    }
                }
                message->dispMode = GAMEMES_DISP_NONE;
                break;

            case GAMEMES_DISP_HIDE:
                /* The HIDE command calls the display-on routine for both timer digits. */
                for(digitSlot=0; digitSlot<2; digitSlot++) {
                    HuSprDispOn(spriteGroup, digitSlot);
                }
                message->subMode = TIMER_SUBMODE_DIGITUP;
                message->dispMode = GAMEMES_DISP_NONE;
                break;

            default:
                message->dispMode = GAMEMES_DISP_NONE;
                break;
        }
    }
    if(message->subMode == TIMER_SUBMODE_OFF) {
        return TRUE;
    }
    if(message->timerVal > 99) {
        digits[0] = digits[1] = 9;
    } else {
        int tensDigit = message->timerVal/10;
        digits[1] = tensDigit;
        digits[0] = message->timerVal-(tensDigit*10);
    }
    HuSprGrpPosSet(spriteGroup, message->pos.x, message->pos.y);
    HuSprGrpScaleSet(spriteGroup, message->scale.x, message->scale.y);
    for(digitSlot=0; digitSlot<2; digitSlot++) {
        HuSprBankSet(spriteGroup, digitSlot, digits[digitSlot]);
    }
    if(message->subMode != GAMEMES_SUBMODE_NONE) {
        switch(message->subMode) {
            case TIMER_SUBMODE_SHOW:
            {
                float scaleX, scaleY;
                digitScale = fabs(((5*HuSin(message->angle))+1)-(HuSin(130)*5));
                scaleX = message->scale.x*digitScale;
                scaleY = message->scale.y*digitScale;
                message->angle += 5.0f*GameMesVWait;
                if(message->angle > 130) {
                    message->subMode = GAMEMES_SUBMODE_NONE;
                } else {
                    HuSprGrpScaleSet(spriteGroup, scaleX, scaleY);
                }
            }
                break;

            case TIMER_SUBMODE_DIGITUP:
                digitScale = 1+HuSin(message->angle);
                transparency = 1-(0.5*HuSin(message->angle));
                message->angle += 18.0f*GameMesVWait;
                if(message->angle > 180) {
                    message->subMode = GAMEMES_SUBMODE_NONE;
                    digitScale = 1;
                    transparency = 1;
                }
                for(digitSlot=0; digitSlot<2; digitSlot++) {
                    HuSprScaleSet(spriteGroup, digitSlot, digitScale, digitScale);
                    HuSprTPLvlSet(spriteGroup, digitSlot, transparency);
                }
                break;

            case GAMEMES_SUBMODE_NULL:
                HuSprGrpScaleSet(spriteGroup, message->scale.x, message->scale.y);
                for(digitSlot=0; digitSlot<2; digitSlot++) {
                    HuSprScaleSet(spriteGroup, digitSlot, 1, 1);
                    HuSprTPLvlSet(spriteGroup, digitSlot, 1);
                }
                message->angle++;
                if(message->angle < 60) {
                    break;
                }
                transparency = 1.0-((message->angle-60)/20);
                if(transparency <= 0) {
                    transparency = 0;
                    message->subMode = GAMEMES_SUBMODE_NONE;
                    message->stat |= GAMEMES_STAT_KILL;
                }
                for(digitSlot=0; digitSlot<3; digitSlot++) {
                    HuSprTPLvlSet(spriteGroup, digitSlot, transparency);
                }
                break;
        }
    }
    if(GameMesCloseF || (message->stat & GAMEMES_STAT_KILL)) {
        GameMesSprKill(message);
        return FALSE;
    }
    return TRUE;
}

#undef TIMER_SUBMODE_OFF
#undef TIMER_SUBMODE_SHOW
#undef TIMER_SUBMODE_DIGITUP

/* Called by GameMesInit to allocate and clear lookup buffers for message fonts. */
static void CreateFontBuf(void)
{
    if(!fontBufs[0]) {
        fontBufs[0] = HuMemDirectMalloc(HEAP_HEAP, strlen(asciiTbl)*sizeof(int));
        memset(fontBufs[0], 0, strlen(asciiTbl)*sizeof(int));
    }
    if(!fontBufs[1]) {
        fontBufs[1] = HuMemDirectMalloc(HEAP_HEAP, strlen(kanaTbl)*sizeof(int));
        memset(fontBufs[1], 0, strlen(kanaTbl)*sizeof(int));
    }
    if(!fontBufs[2]) {
        fontBufs[2] = HuMemDirectMalloc(HEAP_HEAP, strlen(kanaTbl)*sizeof(int));
        memset(fontBufs[2], 0, strlen(kanaTbl)*sizeof(int));
    }
    if(!fontBufs[3]) {
        fontBufs[3] = HuMemDirectMalloc(HEAP_HEAP, strlen(numberTbl)*sizeof(int));
        memset(fontBufs[3], 0, strlen(numberTbl)*sizeof(int));
    }
    if(!fontBufs[4]) {
        fontBufs[4] = HuMemDirectMalloc(HEAP_HEAP, strlen(punctTbl)*sizeof(int));
        memset(fontBufs[4], 0, strlen(punctTbl)*sizeof(int));
    }
}

static ANIMDATA *CreateFontChar(char *character, s16 flags);

#define STR_CHAR_MAX 100

/* Builds a centered sprite group for a message's supported single-byte text. */
int GameMesStrCreate(GAMEMES *message, char *text, s16 flags)
{
    s16 glyphCount;
    s16 penX;
    s16 glyphIndex;
    s16 groupSlot;
    char *character;
    ANIMDATA **glyphAnimations;
    s16 *glyphPositions;
    s16 spaceCount;
    HUSPR_GROUPID spriteGroup;

    for(groupSlot=0; groupSlot<GAMEMES_SPRMAX; groupSlot++) {
        if(message->grpId[groupSlot] == HUSPR_GROUP_NONE) {
            break;
        }
    }
    if(groupSlot == GAMEMES_SPRMAX) {
        return GAMEMES_STR_NONE;
    }
    glyphAnimations = HuMemDirectMalloc(HEAP_HEAP, STR_CHAR_MAX*sizeof(ANIMDATA *));
    glyphPositions = HuMemDirectMalloc(HEAP_HEAP, STR_CHAR_MAX*sizeof(s16));

    for(character=text, penX=0, glyphCount=spaceCount=0; *character; character++) {
        if(*character == ' ') {
            penX += 56;
            spaceCount++;
        } else {
            glyphAnimations[glyphCount] = CreateFontChar(character, flags);
            if(glyphAnimations[glyphCount]) {
                glyphPositions[glyphCount] = penX;
                penX += 56;
                glyphCount++;
            }
        }
    }
    /* Spaces count toward group capacity and centering, but receive no sprite member. */
    spriteGroup = HuSprGrpCreate(glyphCount+spaceCount);
    message->grpId[groupSlot] = spriteGroup;
    penX = (penX/2)-28;
    for(glyphIndex=0; glyphIndex<glyphCount; glyphIndex++) {
        HUSPRID sprite = HuSprCreate(glyphAnimations[glyphIndex], 5, 0);
        HuSprGrpMemberSet(spriteGroup, glyphIndex, sprite);
        HuSprPosSet(spriteGroup, glyphIndex, glyphPositions[glyphIndex]-penX, 0);
    }
    message->charNum = glyphCount;
    HuMemDirectFree(glyphAnimations);
    HuMemDirectFree(glyphPositions);
    return groupSlot;
}

/* Used by minigame message updates to duplicate an existing text sprite group. */
int GameMesStrCopy(GAMEMES *message, s16 sourceGroupSlot)
{
    s16 groupSlot;
    for(groupSlot=0; groupSlot<GAMEMES_SPRMAX; groupSlot++) {
        if(message->grpId[groupSlot] == HUSPR_GROUP_NONE) {
            break;
        }
    }
    if(groupSlot == GAMEMES_SPRMAX) {
        return GAMEMES_STR_NONE;
    }
    message->grpId[groupSlot] = HuSprGrpCopy(message->grpId[sourceGroupSlot]);
    return groupSlot;
}

/* Resolves one supported character byte to its animation; bytes 222 and 223 are skipped. */
static ANIMDATA *CreateFontChar(char *character, s16 flags)
{
    char *characterTable;
    s16 characterIndex;
    unsigned int dataNum;
    char characterCode;
    characterCode = *character;
    if(characterCode == 222 || characterCode == 223) {
        return NULL;
    }
    /* flags is accepted by the caller but does not affect this character lookup. */
    for (characterIndex = 0, characterTable = asciiTbl; *characterTable;
         characterIndex++, characterTable++) {
        if(*characterTable == characterCode) {
            dataNum = GAMEMES_ANM_letterAUpper+characterIndex;
            return HuSprAnimRead(GameMesDataRead(dataNum));
        }
    }
    for (characterIndex = 0, characterTable = numberTbl; *characterTable;
         characterIndex++, characterTable++) {
        if(*characterTable == characterCode) {
            dataNum = GAMEMES_ANM_number0+characterIndex;
            return HuSprAnimRead(GameMesDataRead(dataNum));
        }
    }
    for (characterIndex = 0, characterTable = punctTbl; *characterTable;
         characterIndex++, characterTable++) {
        if(*characterTable == characterCode) {
            dataNum = GAMEMES_ANM_punctExcl+characterIndex;
            return HuSprAnimRead(GameMesDataRead(dataNum));
        }
    }
    return NULL;
}

static void PauseExec(void);

/* Called by the system pause handler to create the minigame pause prompt process. */
void GameMesPauseCreate(void)
{
    GameMesPauseEnable(FALSE);
    HuWinInit(1);
    pauseProc = HuPrcCreate(PauseExec, 100, 4096, 0);
    HuPrcSetStat(pauseProc, HU_PRC_STAT_PAUSE_ON|HU_PRC_STAT_UPAUSE_ON);
    pauseEnableF = TRUE;
    pauseCancelF = FALSE;
    pauseWaitF = FALSE;
}

/* Runs in a separate process to show minigame instructions and wait for resume. */
static void PauseExec(void)
{
    GAMEMES mes;
    s16 charNo[GW_PLAYER_MAX][GW_PLAYER_MAX];
    s16 charNum[GW_PLAYER_MAX];
    HUWINID winId[3] = { HUWIN_NONE, HUWIN_NONE, HUWIN_NONE };

    s16 i;
    u32 *instMes;
    s16 j;
    s16 mgNo;
    float time;

    HuAudFXPlay(MSM_SE_CMN_06);
    for(i=0; i<GAMEMES_SPRMAX; i++) {
        mes.sprId[i] = mes.grpId[i] = HUSPR_NONE;
    }
    GameMesStrWinCreate(&mes, GAMEMES_MESS_PAUSE_HEADER);
    for(i=0; i<mes.charNum; i++) {
        HuSprPriSet(mes.grpId[0], i, 0);
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        charNum[i] = 0;
    }
    for(i=0; i<GW_PLAYER_MAX; i++) {
        charNo[GwPlayerConf[i].grpNo][charNum[GwPlayerConf[i].grpNo]] = GwPlayerConf[i].charNo;
        charNum[GwPlayerConf[i].grpNo]++;
    }
    mgNo = MgNoGet(omcurovl);
    if(_CheckFlag(FLAG_INST_DECA)) {
        instMes = MgDataTbl[mgNo].instMes[2];
    } else {
        s16 night = GwMgNightF;
        if(night == 0) {
            instMes = MgDataTbl[mgNo].instMes[0];
        } else {
            instMes = MgDataTbl[mgNo].instMes[1];
        }
        if(MgDataTbl[mgNo].ovl == DLL_m678dll) {
            int subGame = GwSystem.subGameNo;
            if(subGame == 1) {
                instMes = MgDataTbl[mgNo].instMes[1];
            }
        }
    }
    if(!instMes[1] && !instMes[2]) {
        for(i=1; i<=20; i++) {
            time = HuSin(i*(90.0f/20.0f));
            HuSprGrpPosSet(mes.grpId[0], 288, (290*time)-50);
            HuPrcVSleep();
        }
    } else {
        if(MgPauseExitF && !_CheckFlag(FLAG_MG_PRACTICE)) {
            winId[2] = HuWinExCreateFrame(-10000, 400, 412, 42, HUWIN_SPEAKER_NULL, 0);
            HuWinPriSet(winId[2], 5);
            HuWinDispOn(winId[2]);
            HuWinMesSpeedSet(winId[2], 0);
            HuWinAttrSet(winId[2], HUWIN_ATTR_ALIGN_CENTER);
            HuWinMesSet(winId[2], GAMEMES_MESS_PAUSE_EXIT_PROMPT);
        }
        if(instMes[2]) {
            s16 insertMesNo;
            winId[0] = HuWinExCreateFrame(-10000, 140, 412, 120, HUWIN_SPEAKER_NULL, 0);
            HuWinPriSet(winId[0], 5);
            HuWinDispOn(winId[0]);
            HuWinMesSpeedSet(winId[0], 0);
            HuWinMesSet(winId[0], instMes[1]);
            winId[1] = HuWinExCreateFrame(-10000, 276, 412, 120, HUWIN_SPEAKER_NULL, 0);
            HuWinPriSet(winId[1], 5);
            HuWinDispOn(winId[1]);
            HuWinMesSpeedSet(winId[1], 0);
            HuWinMesSet(winId[1], instMes[2]);
            for(i=insertMesNo=0; i<GW_PLAYER_MAX; i++) {
                for(j=0; j<charNum[i]; j++) {
                    HuWinInsertMesSet(winId[0], charNo[i][j], (int)insertMesNo);
                    HuWinInsertMesSet(winId[1], charNo[i][j], (int)insertMesNo);
                    insertMesNo++;
                }
            }
            for(i=1; i<=20; i++) {
                time = HuSin(i*(90.0f/20.0f));
                HuSprGrpPosSet(mes.grpId[0], 288, (150*time)-50);
                HuWinPosSet(winId[0], (482*time)-400, 140);
                HuWinPosSet(winId[1], (-318*time)+400, 272);
                if(winId[2] != HUWIN_NONE) {
                    HuWinPosSet(winId[2], 82, (100*(1.0-time))+404);
                }
                HuPrcVSleep();
            }
        } else {
            winId[0] = HuWinExCreateFrame(-10000, 170, 412, 120, HUWIN_SPEAKER_NULL, 0);
            HuWinPriSet(winId[0], 5);
            HuWinDispOn(winId[0]);
            HuWinMesSpeedSet(winId[0], 0);
            HuWinMesSet(winId[0], instMes[1]);
            for(i=1; i<=20; i++) {
                time = HuSin(i*(90.0f/20.0f));
                HuSprGrpPosSet(mes.grpId[0], 288, (150*time)-50);
                HuWinPosSet(winId[0], (482*time)-400, 170);
                if(winId[2] != HUWIN_NONE) {
                    HuWinPosSet(winId[2], (-318*time)+400, 404);
                }
                HuPrcVSleep();
            }
        }
    }
    GameMesPauseEnable(TRUE);
    pauseWaitF = TRUE;
    while(!pauseCancelF) {
        HuPrcVSleep();
    }
    pauseWaitF = FALSE;
    if(winId[0] == HUWIN_NONE && winId[1] == HUWIN_NONE) {
        for(i=1; i<=10; i++) {
            time = HuCos(i*(90.0f/10.0f));
            HuSprGrpPosSet(mes.grpId[0], 288, (290*time)-50);
            HuPrcVSleep();
        }
    } else if(winId[1] != HUWIN_NONE) {
        for(i=1; i<=10; i++) {
            time = HuCos(i*(90.0f/10.0f));
            HuSprGrpPosSet(mes.grpId[0], 288, (150*time)-50);
            HuWinPosSet(winId[0], (482*time)-400, 140);
            HuWinPosSet(winId[1], (-318*time)+400, 272);
            if(winId[2] != HUWIN_NONE) {
                HuWinPosSet(winId[2], 82, (100*(1.0-time))+404);
            }
            HuPrcVSleep();
        }
    } else {
        for(i=1; i<=10; i++) {
            time = HuCos(i*(90.0f/10.0f));
            HuSprGrpPosSet(mes.grpId[0], 288, (150*time)-50);
            HuWinPosSet(winId[0], (482*time)-400, 170);
            if(winId[2] != HUWIN_NONE) {
                HuWinPosSet(winId[2], (-318*time)+400, 404);
            }
            HuPrcVSleep();
        }
    }
    omSysPauseCtrl(FALSE);
    if(winId[0] != HUWIN_NONE) {
        HuWinKill(winId[0]);
    }
    if(winId[1] != HUWIN_NONE) {
        HuWinKill(winId[1]);
    }
    if(winId[2] != HUWIN_NONE) {
        HuWinKill(winId[2]);
    }
    HuSprGrpKill(mes.grpId[0]);
    pauseProc = FALSE;
    pauseEnableF = FALSE;
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}

void GameMesPauseCancel(void)
{
    pauseCancelF = TRUE;
}

/* Enables or blocks the system pause controls unless the current mode disables pausing. */
void GameMesPauseEnable(BOOL enable)
{
    if(_CheckFlag(FLAG_MG_PAUSE_OFF) || _CheckFlag(FLAGNUM(FLAG_GROUP_SYSTEM, 5))) {
        return;
    }
    omSysPauseEnable(enable);
}

#define PRACTICE_POS_TOP 0
#define PRACTICE_POS_BOTTOM 1
#define PRACTICE_POS_CENTER 2
#define PRACTICE_POS_NUM 5

typedef struct Practice_s {
    s16 ovl;
    s16 posNo;
} PRACTICE;

static PRACTICE practiceTbl[] = {
    6, 1, 7, 1, 8, 2, 9, 1, 10, 0, 11, 2, 12, 1, 13, 3, 14, 1, 15, 1,
    16, 0, 17, 2, 18, 1, 19, 2, 20, 1, 21, 1, 22, 4, 23, 2, 24, 1, 25, 1,
    26, 0, 27, 4, 28, 3, 29, 4, 30, 1, 31, 1, 32, 1, 33, 1, 34, 1, 35, 1,
    36, 1, 37, 1, 38, 0, 39, 0, 40, 1, 41, 1, 42, 0, 43, 1, 44, 1, 45, 2,
    46, 2, 47, 1, 48, 2, 49, 1, 50, 1, 51, 1, 52, 0, 53, 1, 54, 2, 55, 2,
    56, 1, 57, 1, 58, 4, 59, 1, 60, 1, 61, 2, 62, 1, 63, 1, 64, 1, 65, 1,
    65, 1, 66, 4, 67, 1, 68, 1, 69, 1, 70, 0, 71, 0, 72, 1, 73, 1, 74, 1,
    75, 1, 76, 0, 77, 0, 78, 1, 79, 1, 80, 0, 81, 1, 82, 0, 83, 0, 84, 4,
    85, 1, 86, 1, 87, 4,
    DLL_NONE, 0
};

static void PracticeExec(void);

/* Called when a practice minigame overlay loads to start its exit-icon child process. */
void GameMesPracticeCreate(void)
{
    HUPROCESS *parent = HuPrcCurrentGet();
    s16 i;
    if(!_CheckFlag(FLAG_MG_PRACTICE)) {
        return;
    }
    wipeFadeInF = FALSE;
    for(i=0; practiceTbl[i].ovl != DLL_NONE; i++) {
        if(omcurovl == practiceTbl[i].ovl) {
            break;
        }
    }
    if(practiceTbl[i].ovl != DLL_NONE) {
        HuPrcSetStat(HuPrcChildCreate(PracticeExec, 10, 8192, 0, parent),
                     HU_PRC_STAT_PAUSE_ON | HU_PRC_STAT_UPAUSE_ON);
    }
}

/* Shows the practice exit icon and watches for the mode's human-player Z-button exit input. */
static void PracticeExec(void)
{
    float angle;
    s16 id;
    HUSPR_GROUPID practiceGroup;
    MGDATA *minigame;
    BOOL exitRequested;
    PRACTICE *practiceEntry;
    HUSPRID exitSprite;
    ANIMDATA *exitAnimation;

    static float posTbl[PRACTICE_POS_NUM] = {
        53,
        424,
        240,
        384,
        102
    };

    angle = 0;
    for(id=0; practiceTbl[id].ovl != DLL_NONE; id++) {
        if(omcurovl == practiceTbl[id].ovl) {
            break;
        }
    }
    practiceEntry = &practiceTbl[id];
    id = MgNoGet(omcurovl);
    if(id != -1) {
        minigame = &MgDataTbl[id];
    } else {
        minigame = &MgDataTbl[0];
    }
    practiceGroup = HuSprGrpCreate(1);
    HuSprGrpPosSet(practiceGroup, 0, 0);
    exitAnimation = HuSprAnimRead(GameMesDataRead(GAMEMES_ANM_practiceExit));
    exitSprite = HuSprCreate(exitAnimation, 0, 0);
    HuSprGrpMemberSet(practiceGroup, 0, exitSprite);
    HuSprPosSet(practiceGroup, 0, 288, posTbl[practiceEntry->posNo]);
    do {
        if(!wipeFadeInF || WipeCheckIn() || omPauseChk()) {
            HuSprAttrSet(practiceGroup, 0, 4);
            HuPrcVSleep();
            continue;
        }
        HuSprAttrReset(practiceGroup, 0, 4);
        exitRequested = FALSE;
        switch(minigame->type) {
            case MG_TYPE_4P:
            case MG_TYPE_1VS3:
            case MG_TYPE_2VS2:
            case MG_TYPE_BATTLE:
            case MG_TYPE_KUPA:
            case MG_TYPE_LAST:
            case MG_TYPE_DONKEY:
                for(id=0; id<GW_PLAYER_MAX; id++) {
                    if ((HuPadBtnDown[GwPlayerConf[id].padNo] & PAD_TRIGGER_Z) &&
                        GwPlayerConf[id].type == GW_PLAYER_TYPE_MAN) {
                        break;
                    }
                }
                if(id != GW_PLAYER_MAX) {
                    exitRequested = TRUE;
                }
                break;

            case MG_TYPE_KETTOU:
                for(id=0; id<GW_PLAYER_MAX; id++) {
                    if ((HuPadBtnDown[GwPlayerConf[id].padNo] & PAD_TRIGGER_Z) &&
                        GwPlayerConf[id].type == GW_PLAYER_TYPE_MAN && GwPlayerConf[id].grpNo < 2) {
                        break;
                    }
                }
                if(id != GW_PLAYER_MAX) {
                    exitRequested = TRUE;
                }
                break;

            default:
                break;
        }
        if(exitRequested) {
            break;
        }
        HuSprTPLvlSet(practiceGroup, 0, 0.7f);
        angle += 2;
        HuPrcVSleep();
    } while(1);
    HuSprAttrSet(practiceGroup, 0, 4);
    omSysExitReq = TRUE;
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}

void GameMesEnd(void)
{
}

/* Called by the minigame exit-check object to leave after Z during a pause prompt. */
void GameMesExitCheck(OMOBJ *exitCheckObject)
{
    MgExitReq = FALSE;
    if(exitCheckObject->work[0] == 0) {
        if(MgNoGet(omcurovl) == -1) {
            omDelObjEx(HuPrcCurrentGet(), exitCheckObject);
            return;
        } else {
            exitCheckObject->work[0]++;
        }
    }
    if(!omPauseChk() || _CheckFlag(FLAG_MG_PRACTICE) || !pauseWaitF) {
        return;
    }
    if(HuPadBtnDown[omDBGSysKeyObj->work[1]] & PAD_TRIGGER_Z) {
        HuAudFXPlay(MSM_SE_CMN_04);
        GameMesPauseCancel();
        omSysPauseCtrl(FALSE);
        omSysExitReq = TRUE;
        MgExitReq = TRUE;
        omDelObjEx(HuPrcCurrentGet(), exitCheckObject);
    }
}

static void ScoreBoxBGMake(ANIMDATA *animation, s16 characterCount);

/* Draws a score panel by combining its mask, generated background, and optional character art. */
static void ScoreBoxDraw(HUSPRITE *sprite)
{
    ANIMBMP *bitmap = sprite->bg->bmp;
    float sizeX = bitmap->sizeX*16;
    float sizeY = bitmap->sizeY*16;
    Mtx modelview;
    HuVec2f vtx[4];
    GXColor color;
    GXSetScissor(sprite->scissorX, sprite->scissorY, sprite->scissorW, sprite->scissorH);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    color.r = sprite->r;
    color.g = sprite->g;
    color.b = sprite->b;
    color.a = sprite->a;
    GXSetTevColor(GX_TEVREG0, color);
    GXSetNumTevStages(3);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_C0, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP2, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_TEXC, GX_CC_TEXA, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_APREV, GX_CA_TEXA, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_A0, GX_CA_APREV, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0, GX_DF_CLAMP,
                  GX_AF_SPOT);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetZCompLoc(GX_TRUE);
    PSMTXScale(modelview, sprite->scale.x, sprite->scale.y, 1.0f);
    mtxTransCat(modelview, sprite->pos.x, sprite->pos.y, 0);
    PSMTXConcat(*sprite->groupMtx, modelview, modelview);
    GXLoadPosMtxImm(modelview, GX_PNMTX0);
    HuSprTexLoad(ScoreBoxMaskAnim, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_NEAR);
    HuSprTexLoad(sprite->bg, 0, GX_TEXMAP1, GX_CLAMP, GX_CLAMP, GX_NEAR);
    HuSprTexLoad(ScoreBoxAnim, 0, GX_TEXMAP2, GX_CLAMP, GX_CLAMP, GX_NEAR);
    GXSetNumIndStages(2);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_TRUE, sizeX, sizeY);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD0, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_16, GX_ITS_16);
    GXSetTevIndTile(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 16, 16, 16, 16, GX_ITF_4, GX_ITM_0, GX_ITB_NONE,
                    GX_ITBA_OFF);
    GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD0, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_16, GX_ITS_16);
    GXSetTevIndTile(GX_TEVSTAGE1, GX_INDTEXSTAGE1, 16, 16, 16, 16, GX_ITF_4, GX_ITM_0, GX_ITB_NONE,
                    GX_ITBA_OFF);
    vtx[0].x = (-sizeX/2)/2;
    vtx[0].y = (-sizeY/2)/2;
    vtx[1].x = (sizeX/2)/2;
    vtx[1].y = (-sizeY/2)/2;
    vtx[2].x = (sizeX/2)/2;
    vtx[2].y = (sizeY/2)/2;
    vtx[3].x = (-sizeX/2)/2;
    vtx[3].y = (sizeY/2)/2;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(vtx[0].x, vtx[0].y, 0);
    GXTexCoord2f32(0, 0);
    GXPosition3f32(vtx[1].x, vtx[1].y, 0);
    GXTexCoord2f32(1, 0);
    GXPosition3f32(vtx[2].x, vtx[2].y, 0);
    GXTexCoord2f32(1, 1);
    GXPosition3f32(vtx[3].x, vtx[3].y, 0);
    GXTexCoord2f32(0, 1);
    GXEnd();
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_FALSE, 0, 0);
}

/* ScoreBoxCreate calls this to build the indirect tile map for a panel and its character art. */
static void ScoreBoxBGMake(ANIMDATA *animation, s16 characterCount)
{
    s16 row, column;
    u8 *bmpData;
    s16 blockW;
    s16 w;
    s16 blockH;
    s16 h;
    s16 k;

    w = animation->bmp->sizeX;
    h = animation->bmp->sizeY;
    blockW = (w+7) & 0xF8;
    blockH = (h+3) & 0xFC;
    bmpData = animation->bmp->data = HuMemDirectMallocNum(HEAP_HEAP, blockW*blockH, HU_MEMNUM_OVL);
    for(row=0; row<h; row++) {
        if(row < 3) {
            for(column=0; column<w; column++) {
                if(column == 0) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = row + SCOREBOX_TILE_LEFT_OUTER_BASE;
                } else if(column == 1) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = row + SCOREBOX_TILE_LEFT_MIDDLE_BASE;
                } else if(column == 2) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = row + SCOREBOX_TILE_LEFT_INNER_BASE;
                } else if(column == w-3) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = row + SCOREBOX_TILE_RIGHT_INNER_BASE;
                } else if(column == w-2) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = row + SCOREBOX_TILE_RIGHT_MIDDLE_BASE;
                } else if(column == w-1) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = row + SCOREBOX_TILE_RIGHT_OUTER_BASE;
                } else {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = row + SCOREBOX_TILE_HORIZONTAL_EDGE_BASE;
                }
            }
        } else if(row >= h-3) {
            for(column=0; column<w; column++) {
                s16 edgeRow = row-(h-6);
                if(column == 0) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = edgeRow + SCOREBOX_TILE_LEFT_OUTER_BASE;
                } else if(column == 1) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = edgeRow + SCOREBOX_TILE_LEFT_MIDDLE_BASE;
                } else if(column == 2) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = edgeRow + SCOREBOX_TILE_LEFT_INNER_BASE;
                } else if(column == w-3) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = edgeRow + SCOREBOX_TILE_RIGHT_INNER_BASE;
                } else if(column == w-2) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = edgeRow + SCOREBOX_TILE_RIGHT_MIDDLE_BASE;
                } else if(column == w-1) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = edgeRow + SCOREBOX_TILE_RIGHT_OUTER_BASE;
                } else {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = edgeRow + SCOREBOX_TILE_HORIZONTAL_EDGE_BASE;
                }
            }
        } else {
            for(column=0; column<w; column++) {
                if(column == 0) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = SCOREBOX_TILE_VERTICAL_LEFT;
                } else if(column == w-1) {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = SCOREBOX_TILE_VERTICAL_RIGHT;
                } else {
                    bmpData[(column & 7) + ((column >> 3) << 5) + (row >> 2) * (blockW * 4) +
                            (row & 3) * 8] = SCOREBOX_TILE_INTERIOR;
                }
            }
        }
    }
    if(characterCount) {
        for(k=0; k<characterCount; k++) {
            for(row=0; row<6; row++) {
                for(column=0; column<6; column++) {
                    bmpData[((column + k * 5) & 7) + (((column + k * 5) >> 3) << 5) +
                            (row >> 2) * (blockW * 4) + (row & 3) * 8] = row + (column * 16);
                }
            }
        }
    }
    DCStoreRangeNoSync(animation->bmp->data, blockW*blockH);
}

void MgScoreBoxInit(void)
{
    ScoreBoxMaskAnim = ScoreBoxAnim = NULL;
}

static HUSPR_GROUPID ScoreBoxCreate(s16 sizeX, s16 sizeY, s16 num, u8 *chars);

HUSPR_GROUPID MgScoreBoxCreate(s16 sizeX, s16 sizeY)
{
    return ScoreBoxCreate(sizeX, sizeY, 0, NULL);
}

/* Called by minigame score setup to create a panel containing one character tile. */
HUSPR_GROUPID MgScoreBoxCreateChar(s16 sizeX, s16 sizeY, s16 characterId)
{
    u8 characterTiles[1];
    characterTiles[0] = characterId;
    return ScoreBoxCreate(sizeX, sizeY, 1, characterTiles);
}

HUSPR_GROUPID MgScoreBoxCreateCharMulti(s16 sizeX, s16 sizeY, s16 num, u8 *chars)
{
    return ScoreBoxCreate(sizeX, sizeY, num, chars);
}

/* Allocates a score panel, generates its background, and adds any requested character tiles. */
static HUSPR_GROUPID ScoreBoxCreate(s16 sizeX, s16 sizeY, s16 characterCount, u8 *characterTiles)
{
    HUSPR_GROUPID gid;
    HUSPRID sprid;
    ANIMDATA *anim;
    s16 i;
    sizeX = (sizeX+7) & 0xFFF8;
    sizeY = (sizeY+7) & 0xFFF8;
    if(sizeX < SCOREBOX_MIN_SIZE) {
        sizeX = SCOREBOX_MIN_SIZE;
    }
    if(sizeY < SCOREBOX_MIN_SIZE) {
        sizeY = SCOREBOX_MIN_SIZE;
    }
    gid = HuSprGrpCreate(characterCount+1);
    if(!ScoreBoxMaskAnim) {
        ScoreBoxMaskAnim = HuSprAnimDataRead(DATANUM(DATA_mgconst, 47));
    }
    if(!ScoreBoxAnim) {
        ScoreBoxAnim = HuSprAnimDataRead(DATANUM(DATA_mgconst, 46));
    }
    sprid = HuSprFuncCreate(ScoreBoxDraw, 100);
    HuSprGrpMemberSet(gid, 0, sprid);
    HuSprData[sprid].bg = anim = HuSprAnimMake(sizeX/8, sizeY/8, ANIM_BMP_IA4);
    ScoreBoxBGMake(anim, characterCount);
    HuSprColorSet(gid, 0, 64, 64, 64);
    HuSprTPLvlSet(gid, 0, 1.0f);
    for(i=0; i<characterCount; i++) {
        anim = HuSprAnimRead(
            HuDataSelHeapReadNum(DATA_mgconst + characterTiles[i], HU_MEMNUM_OVL, HEAP_MODEL));
        sprid = HuSprCreate(anim, 100, 0);
        HuSprGrpMemberSet(gid, i+1, sprid);
        HuSprPosSet(gid, i+1, -(sizeX/2)+24+(i*40), 0);
        HuSprTPLvlSet(gid, i+1, 1.0f);
    }
    return gid;
}

void MgScoreBoxKill(HUSPR_GROUPID gid)
{
    HuSprGrpKill(gid);
}

void MgScoreBoxColorSet(HUSPR_GROUPID gid, u8 r, u8 g, u8 b)
{
    HuSprColorSet(gid, 0, r, g, b);
}

void MgScoreBoxTPLvlSet(HUSPR_GROUPID gid, float tpLvl)
{
    HuSprGrpTPLvlSet(gid, tpLvl);
}

void MgScoreBoxPosSet(HUSPR_GROUPID gid, float posX, float posY)
{
    HuSprGrpPosSet(gid, posX, posY);
}

/* Called by minigame HUD updates to show or hide every sprite in a score panel. */
void MgScoreBoxDispSet(HUSPR_GROUPID gid, BOOL display)
{
    if(display) {
        HuSprGrpAttrReset(gid, HUSPR_ATTR_DISPOFF);
    } else {
        HuSprGrpAttrSet(gid, HUSPR_ATTR_DISPOFF);
    }
}

#define FONT_NUMBER(c) (GAMEMES_ANM_number0+((c)-'0'))
#define FONT_LETTERUPPER(c) (GAMEMES_ANM_letterAUpper+((c)-'A'))
#define FONT_LETTERLOWER(c) (GAMEMES_ANM_letterALower+((c)-'a'))

static unsigned int fontFileTbl[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    FONT_NUMBER('0'),
    FONT_NUMBER('1'),
    FONT_NUMBER('2'),
    FONT_NUMBER('3'),
    FONT_NUMBER('4'),
    FONT_NUMBER('5'),
    FONT_NUMBER('6'),
    FONT_NUMBER('7'),
    FONT_NUMBER('8'),
    FONT_NUMBER('9'),
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_fontDash,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    FONT_LETTERUPPER('A'),
    FONT_LETTERUPPER('B'),
    FONT_LETTERUPPER('C'),
    FONT_LETTERUPPER('D'),
    FONT_LETTERUPPER('E'),
    FONT_LETTERUPPER('F'),
    FONT_LETTERUPPER('G'),
    FONT_LETTERUPPER('H'),
    FONT_LETTERUPPER('I'),
    FONT_LETTERUPPER('J'),
    FONT_LETTERUPPER('K'),
    FONT_LETTERUPPER('L'),
    FONT_LETTERUPPER('M'),
    FONT_LETTERUPPER('N'),
    FONT_LETTERUPPER('O'),
    FONT_LETTERUPPER('P'),
    FONT_LETTERUPPER('Q'),
    FONT_LETTERUPPER('R'),
    FONT_LETTERUPPER('S'),
    FONT_LETTERUPPER('T'),
    FONT_LETTERUPPER('U'),
    FONT_LETTERUPPER('V'),
    FONT_LETTERUPPER('W'),
    FONT_LETTERUPPER('X'),
    FONT_LETTERUPPER('Y'),
    FONT_LETTERUPPER('Z'),
    FONT_LETTERUPPER('O'),
    GAMEMES_ANM_fontApos,
    FONT_LETTERUPPER('O'),
    FONT_LETTERUPPER('O'),
    FONT_LETTERUPPER('O'),
    FONT_LETTERUPPER('O'),
    FONT_LETTERLOWER('a'),
    FONT_LETTERLOWER('b'),
    FONT_LETTERLOWER('c'),
    FONT_LETTERLOWER('d'),
    FONT_LETTERLOWER('e'),
    FONT_LETTERLOWER('f'),
    FONT_LETTERLOWER('g'),
    FONT_LETTERLOWER('h'),
    FONT_LETTERLOWER('i'),
    FONT_LETTERLOWER('j'),
    FONT_LETTERLOWER('k'),
    FONT_LETTERLOWER('l'),
    FONT_LETTERLOWER('m'),
    FONT_LETTERLOWER('n'),
    FONT_LETTERLOWER('o'),
    FONT_LETTERLOWER('p'),
    FONT_LETTERLOWER('q'),
    FONT_LETTERLOWER('r'),
    FONT_LETTERLOWER('s'),
    FONT_LETTERLOWER('t'),
    FONT_LETTERLOWER('u'),
    FONT_LETTERLOWER('v'),
    FONT_LETTERLOWER('w'),
    FONT_LETTERLOWER('x'),
    FONT_LETTERLOWER('y'),
    FONT_LETTERLOWER('z'),
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_fontTilde,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_fontDash,
    GAMEMES_ANM_punctDot,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_punctExcl,
    GAMEMES_ANM_punctHatena,
    GAMEMES_ANM_number0,
    GAMEMES_ANM_number0
};

/* Reads a window's current message and builds its centered glyph sprites in a message slot. */
GAMEMESID GameMesStrWinCreate(GAMEMES *message, u32 windowMessageId)
{
    char *messageText;
    s16 penX;
    s16 *glyphY;
    unsigned int *fontFile;
    s16 glyphCount;
    s16 glyphIndex;
    GAMEMESID groupSlot;
    unsigned int animationDataNum;
    s16 *glyphX;
    ANIMDATA **glyphAnimations;
    HUSPR_GROUPID spriteGroup;
    HUSPRID sprite;
    HuWinInit(0);
    for(groupSlot=0; groupSlot<GAMEMES_SPRMAX; groupSlot++) {
        if(message->grpId[groupSlot] == HUSPR_GROUP_NONE) {
            break;
        }
    }
    if(groupSlot == GAMEMES_SPRMAX) {
        return GAMEMES_STR_NONE;
    }
    fontFile = fontFileTbl;
    glyphAnimations = HuMemDirectMalloc(HEAP_HEAP, STR_CHAR_MAX*sizeof(ANIMDATA *));
    glyphX = HuMemDirectMalloc(HEAP_HEAP, STR_CHAR_MAX*sizeof(s16));
    glyphY = HuMemDirectMalloc(HEAP_HEAP, STR_CHAR_MAX*sizeof(s16));
    for (messageText = HuWinMesPtrGet(windowMessageId), penX = 0, glyphCount = 0; *messageText;
         messageText++) {
        if(messageText[0] < 48 && messageText[0] != ' ' && messageText[0] != 16) {
            continue;
        }
        animationDataNum = fontFile[messageText[0]];
        glyphAnimations[glyphCount] =
            HuSprAnimRead(GameMesDataRead(animationDataNum & GAMEMES_GLYPH_DATA_MASK));
        glyphX[glyphCount] = penX;
        if(messageText[0] == ' ' || messageText[0] == 16) {
            glyphY[glyphCount] = 0;
            penX += 28;
        } else if(messageText[0] >= 'a' && messageText[0] <= 'z') {
            glyphY[glyphCount] = 4;
            penX += 42;
        } else if(messageText[0] == 194 || messageText[0] == 195) {
            glyphY[glyphCount] = 0;
            penX += 56;
        } else if(messageText[0] == '\\') {
            glyphY[glyphCount] = 0;
            penX += 56;
        } else if(animationDataNum & GAMEMES_GLYPH_RAISED_FLAG) {
            glyphY[glyphCount] = -2;
            penX += 56;
        } else {
            glyphY[glyphCount] = 0;
            penX += 56;
        }
        glyphCount++;
    }
    spriteGroup = HuSprGrpCreate(glyphCount);
    message->grpId[groupSlot] = spriteGroup;
    penX = (penX/2)-28;
    for(glyphIndex=0; glyphIndex<glyphCount; glyphIndex++) {
        sprite = HuSprCreate(glyphAnimations[glyphIndex], 0, 0);
        HuSprGrpMemberSet(spriteGroup, glyphIndex, sprite);
        HuSprPosSet(spriteGroup, glyphIndex, glyphX[glyphIndex]-penX, glyphY[glyphIndex]);
    }
    message->charNum = glyphCount;
    HuMemDirectFree(glyphAnimations);
    HuMemDirectFree(glyphX);
    HuMemDirectFree(glyphY);
    return groupSlot;
}
