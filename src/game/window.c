// Manages the game's message windows, text rendering, choices, and speaker portraits.
#define _MATH_H
#define M_PI 3.141592653589793
double sin(double x);
double cos(double x);
#include "game/window.h"
#include "game/gamework.h"
#include "game/process.h"
#include "game/data.h"
#include "game/armem.h"
#include "game/audio.h"
#include "game/hu3d.h"
#include "game/dvd.h"
#include "game/pad.h"
#include "game/object.h"

#include "game/disp.h"
#include "datanum/win.h"
#include "humath.h"

#include "stdarg.h"

#define WIN_COMKEY_MAX 256
#define WIN_TILE_DIMENSION_MASK 65520
#define WIN_MESSAGE_INDEX_MASK 65535
#define WIN_DECIMAL_DIGIT_MASK 15
#define WIN_INLINE_MESSAGE_POINTER_BIT_PATTERN 0x80000000U

typedef struct spcFontTbl_s {
    ANIMDATA **animationSlot; // Animation resource slot used by this special glyph.
    s16 animBank; // Animation bank containing the glyph.
    s16 glyphWidth; // Glyph width in pixels.
    s16 glyphHeight; // Glyph height in pixels.
    s16 centerOffsetX; // Horizontal offset from the glyph placement point, in pixels.
    s16 centerOffsetY; // Vertical offset from the glyph placement point, in pixels.
} SPCFONTTBL;

typedef struct winComKey_s {
    s16 remainingPolls; // Scripted-input polls remaining; each unpaused HuWinComKeyGet call
                        // consumes one.
    u32 controllerKey[GW_PLAYER_MAX]; // Input mask supplied for each controller port during this
                                      // entry.
} WINCOMKEY;

static u16 mesHInsert[HUWIN_INSERTMES_MAX];
static u16 mesWInsert[HUWIN_INSERTMES_MAX];
static WINCOMKEY winComKeyBuf[WIN_COMKEY_MAX];
static u32 winKey[GW_PLAYER_MAX];
HUWIN winData[HUWIN_MAX];

static ANIMDATA *iconAnim;
static ANIMDATA *cursorAnim;
static ANIMDATA *cardAnimA;
static ANIMDATA *cardAnimB;
static HUPROCESS *winProc;
void *messDataPtr;
static s32 messDataNo;
static s16 winMaxWidth;
static s16 winMaxHeight;
static u8 winTabSize;
static u8 winInsertF;
static AMEM_PTR winAMemP;
static u8 *fontWidthP;
static s32 cancelCRF;
static s16 mesCharCnt;
static s16 comKeyIdxNow;
static s16 comKeyIdx;
static u8 LanguageNo;
static ANIMDATA *fontAnim;

s16 winPrio = 1000;
static int speakerSeTbl[1] = { 0 };

static SPCFONTTBL spcFontTbl[] = {
    {   &iconAnim,  0, 20, 24, 10, 12 },
    {   &iconAnim,  1, 20, 24, 10, 12 },
    {   &iconAnim,  2, 20, 24, 10, 12 },
    {   &iconAnim,  3, 20, 24, 10, 12 },
    {   &iconAnim,  4, 20, 24, 10, 12 },
    {   &iconAnim,  5, 20, 24, 10, 12 },
    {   &iconAnim,  6, 20, 24, 10, 12 },
    {   &iconAnim,  7, 20, 24, 10, 12 },
    {   &iconAnim,  8, 20, 24, 10, 12 },
    {   &iconAnim,  9, 20, 24, 10, 12 },
    {   &iconAnim, 10, 20, 24, 10, 12 },
    {   &iconAnim, 11, 30, 24, 15, 12 },
    {   &iconAnim, 12, 20, 24, 10, 12 },
    {   &iconAnim, 13, 20, 24, 10, 12 },
    {   &iconAnim, 14, 20, 24, 10, 12 },
    {   &iconAnim, 15, 20, 24, 10, 12 },
    {   &iconAnim, 16, 20, 24, 10, 12 },
    {   &iconAnim, 17, 20, 24, 10, 12 },
    {   &iconAnim, 18, 20, 24, 10, 12 },
    {   &iconAnim, 19, 24, 24, 12, 12 },
    {   &iconAnim, 20, 20, 24, 10, 12 },
    {   &iconAnim, 21, 20, 24, 10, 12 },
    {   &iconAnim, 22, 20, 24, 10, 12 },

    { &cursorAnim,  0, 40, 32, -15, 18 },
    {  &cardAnimA,  0, 32, 32, 16, 16 },
    {  &cardAnimB,  0, 32, 32, 16, 16 }
};

u8 charWETbl[] = {
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    20,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 18, 20, 12, 12, 11, 14,
    12, 13, 12, 12, 12, 12, 12, 12, 12,  9, 11, 12, 11, 15, 12, 13,
    12, 13, 12, 12, 11, 12, 11, 15, 12, 13, 11, 12,  6,  8,  8, 12,
    20, 12, 11, 12, 11, 11,  9, 12, 11,  4,  8, 11,  4, 14, 11, 12,
    11, 12,  9, 11,  9, 11, 11, 15, 11, 11, 11,  4, 13,  8, 14, 12,
     9,  8,  8,  8, 20,  4, 12, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 12, 12, 12, 12, 12, 12, 12, 12,  8,  8, 12, 12, 12, 12, 12,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 12, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 12,
    13, 13,  6, 12, 18, 14, 12, 8, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 10, 10, 10, 12, 12, 12, 12, 12, 10, 10, 12, 12, 10, 10, 10,
    10, 7, 7, 12, 12, 10, 12, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 16, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
};

u8 charWJTbl[] = {
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,  0, 20,  0,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 0, 20, 80,  0, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
     0,  0, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
};

u8 charWFixedTbl[] = {
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20
};

static u8 ATTRIBUTE_ALIGN(32) charColPal[2][3][HUWIN_MESCOL_MAX] = {
    0, 0, 0,
    0, 0, 255,
    255, 0, 0,
    255, 0, 255,
    0, 255, 0,
    0, 255, 255,
    255, 160, 0,
    0, 0, 0,
    96, 96, 96,
    144, 144, 144,
    0, 0, 0,
    96, 176, 255,
    255, 64, 128,
    255, 0, 255,
    0, 255, 0,
    0, 255, 255,
    255, 255, 0,
    255, 255, 255,
    96, 96, 96,
    144, 144, 144
};

static unsigned int frameFileTbl[HUWIN_FRAME_MAX*2] = {
    //HUWIN_FRAME_DEFAULT
    WIN_ANM_frame_fill,
    WIN_ANM_frame,
    //HUWIN_FRAME_DARK
    WIN_ANM_frame_fill,
    WIN_ANM_frame_dark,
    //HUWIN_FRAME_WARN
    WIN_ANM_frame_fill,
    WIN_ANM_frame_warn,
    //HUWIN_FRAME_NIGHT
    WIN_ANM_frame_fill,
    WIN_ANM_frame_night,
    //HUWIN_FRAME_DAY
    WIN_ANM_frame_fill,
    WIN_ANM_frame_day,
    //HUWIN_FRAME_DAYNIGHT
    WIN_ANM_frame_fill,
    WIN_ANM_frame_daynight,
};

static char *mesDataTbl[] = {
    "mess/messdata.bin",
    "mess/messdata_eng.bin",
    "mess/messdata_ger.bin",
    "mess/messdata_fra.bin",
    "mess/messdata_ita.bin",
    "mess/messdata_spa.bin",
};

static void MesDispFunc(HUSPRITE *sprP);
static u8 winBGMake(ANIMDATA *bgAnim);
static void HuWinProc(void);
static void HuWinDrawMes(HUWINID winId);
static BOOL HuWinCR(HUWIN *winP);
static void _HuWinHomeClear(HUWIN *winP);
static void HuWinKeyWait(HUWINID winId);
static s16 HuWinSpcFontEntry(HUWIN *winP, s16 entry, s16 x, s16 y);
static void HuWinSpcFontClear(HUWIN *winP);
static void HuWinChoice(HUWIN *winP);
static s32 GetMesMaxSizeSub(u32 messNum);
static s32 GetMesMaxSizeSub2(HUWIN *winP, void *data);
static u32 HuWinActivePadGet(HUWIN *winP);
static u32 HuWinActiveKeyGetX(HUWIN *winP);

// Reserves window assets in ARAM and marks every window slot unused during engine startup.
void HuWindowInit(void)
{
    s16 windowIndex;
    winAMemP = HuAR_DVDtoARAM(DATA_win);
    for(windowIndex=0; windowIndex<HUWIN_MAX; windowIndex++) {
        winData[windowIndex].grpId = HUSPR_GROUP_NONE;
    }
    winProc = NULL;
    winPrio = 1000;
}

// Loads the current language's message archive and shared art when windows are initialized.
// messageDataNo is stored without affecting archive selection.
void HuWinInit(s32 messageDataNo)
{
    void *assetData;
    s16 windowIndex;
    if(winProc) {
        return;
    }
    HuAR_ARAMtoMRAM(winAMemP);
    winProc = HuPrcCreate(HuWinProc, 100, 4096, 0);
    HuPrcSetStat(winProc, HU_PRC_STAT_PAUSE_ON|HU_PRC_STAT_UPAUSE_ON);
    LanguageNo = GWLanguageGet();
    messDataNo = messageDataNo;
    fontWidthP = (LanguageNo == HUWIN_LANG_JAPAN) ? charWJTbl : charWETbl;
    HuWinMesRead();
    for(windowIndex=0; windowIndex<HUWIN_MAX; windowIndex++) {
        winData[windowIndex].grpId = HUSPR_GROUP_NONE;
    }
    if(!fontAnim) {
        assetData = HuDataReadNum(WIN_ANM_font, HU_MEMNUM_OVL);
        fontAnim = HuSprAnimRead(assetData);
    }
    if(!iconAnim) {
        assetData = HuDataReadNum(WIN_ANM_icon, HU_MEMNUM_OVL);
        iconAnim = HuSprAnimRead(assetData);
        HuSprAnimLock(iconAnim);
    }
    if(!cursorAnim) {
        assetData = HuDataReadNum(WIN_ANM_cursor, HU_MEMNUM_OVL);
        cursorAnim = HuSprAnimRead(assetData);
        HuSprAnimLock(cursorAnim);
    }
    if(!cardAnimA) {
        assetData = HuDataReadNum(WIN_ANM_cardA, HU_MEMNUM_OVL);
        cardAnimA = HuSprAnimRead(assetData);
        HuSprAnimLock(cardAnimA);
    }
    if(!cardAnimB) {
        assetData = HuDataReadNum(WIN_ANM_cardB, HU_MEMNUM_OVL);
        cardAnimB = HuSprAnimRead(assetData);
        HuSprAnimLock(cardAnimB);
    }
    HuDataDirClose(DATA_win);
    HuWinComKeyReset();
    winPrio = 1000;
    winTabSize = 24;
}

// Allocates a window slot, sizes its text area, and creates its frame and text-render callback.
HUWINID HuWinCreate(float posX, float posY, s16 winW, s16 winH, s16 frame)
{
    HUWINID winId;
    s16 index;
    HUWIN *winP;
    HUSPR_GROUPID spriteGroupId;
    HUSPRID spriteId;
    ANIMDATA *backgroundAnim;
    void *assetData;
    for(winId=0; winId<HUWIN_MAX; winId++) {
        if(winData[winId].grpId == HUSPR_GROUP_NONE) {
            break;
        }
    }
    if(winId == HUWIN_MAX) {
        return HUWIN_NONE;
    }
    winP = &winData[winId];
    winP->grpId = spriteGroupId = HuSprGrpCreate(HUWIN_SPR_MAX);
    if(frame < 0 || frame >= HUWIN_FRAME_MAX) {
        frame = HUWIN_FRAME_DEFAULT;
    }
    winW = (winW+15)&WIN_TILE_DIMENSION_MASK; // Round dimensions up to whole 16-pixel tiles.
    winH = (winH+15)&WIN_TILE_DIMENSION_MASK;
    winP->winW = winW;
    winP->winH = winH;
    if(posX == HUWIN_POS_CENTER) {
        winP->pos.x = (HU_DISP_WIDTH-winW)/2;
    } else {
        winP->pos.x = posX;
    }
    if(posY == HUWIN_POS_CENTER) {
        winP->pos.y = (HU_DISP_HEIGHT-winH)/2;
    } else {
        winP->pos.y = posY;
    }
    HuSprGrpCenterSet(spriteGroupId, winW/2, winH/2);
    HuSprGrpPosSet(spriteGroupId, winP->pos.x, winP->pos.y);
    winP->prio = winPrio;
    HuWinFrameCreate(winP, frame, winP->prio);
    spriteId = winP->sprId[2] = HuSprFuncCreate(MesDispFunc, winP->prio);
    {
        HUSPRITE *sprite = &HuSprData[spriteId];
        sprite->work[0] = winId;
    }
    HuSprGrpMemberSet(spriteGroupId, 2, spriteId);
    winP->charEntryNum = 0;
    winP->charEntryMax = (winW/8)*(winH/24)*5;
    winP->charEntry = HuMemDirectMalloc(HEAP_HEAP, sizeof(WINCHARENTRY)*winP->charEntryMax);
    winP->attr = HUWIN_ATTR_NONE;
    winP->stat = HUWIN_STAT_NONE;
    /* unk94 is never read by the game's code, so it keeps its offset name. */
    winP->unk94 = 0;
    winP->mesTime = 0;
    winP->mesX = winP->mesY = 0;
    winP->mesRectX = 8;
    winP->mesRectY = 8;
    winP->mesRectW = winW-8;
    winP->mesRectH = winH-8;
    winP->mesSpeed = 1;
    winP->mesCol = HUWIN_MESCOL_WHITE;
    winP->mesColShadow = HUWIN_MESCOL_BLACK;
    winP->padMask = HUWIN_PLAYER_ALL;
    winP->disablePlayer = HUWIN_PLAYER_NONE;
    winP->messSp = 0;
    winP->messData = NULL;
    winP->choiceNum = 0;

    winP->charPadX = 1;
    winP->charPadY = 2;

    winP->scissorX = winP->scissorY = 0;
    winP->scissorW = HU_FB_WIDTH;
    winP->scissorH = HU_FB_HEIGHT;
    winP->tabW = 24;
    winP->pushKey = PAD_BUTTON_A|PAD_BUTTON_B;
    winP->choiceEndSe = MSM_SE_CMN_03;
    winP->drawNo = 0;
    winP->callback = NULL;
    if(frame != HUWIN_FRAME_DARK) {
        // The dark frame uses a separate palette; other frames keep the standard text colors.
        memcpy(&winP->mesPal[0][0], &charColPal[1][0][0], HUWIN_MESCOL_MAX*3);
    } else {
        memcpy(&winP->mesPal[0][0], &charColPal[0][0][0], HUWIN_MESCOL_MAX*3);
        winP->mesColShadow = HUWIN_MESCOL_LIGHTGRAY;
    }
    winP->mesCopy = NULL;
    for(index=0; index<HUWIN_INSERTMES_MAX; index++) {
        winP->messDataInsert[index] = NULL;
    }
    for(index=HUWIN_SPR_BEGIN; index<HUWIN_SPR_MAX; index++) {
        winP->sprId[index] = HUSPR_NONE;
    }
    winPrio -= 3; // Later-created windows draw above earlier ones, down to the priority floor.
    if(winPrio < 500) {
        winPrio = 500;
    }

    for(index=0; index<HUWIN_CHOICE_MAX; index++) {
        winP->choiceDisable[index] = FALSE;
        winP->choiceData[index].stat = 0;
    }
    return winId;
}

// Builds the selected window frame sprites and the tiled fill behind the message text.
void HuWinFrameCreate(HUWIN *winP, s16 frame, s16 prio)
{
    GXColor color[HUWIN_FRAME_MAX][2] = {
        {
            { 155, 129, 193, 255 },
            { 94, 59, 135, 255 },
        },
        {
            { 255, 232, 203, 255 },
            { 253, 199, 129, 255 },
        },
        {
            { 64, 64, 64, 255 },
            { 0, 0, 0, 255 },
        },
        {
            { 139, 137, 228, 255 },
            { 34, 34, 167, 255 },
        },
        {
            { 237, 152, 156, 255 },
            { 167, 34, 36, 255 },
        },
        {
            { 139, 184, 229, 255 },
            { 34, 103, 167, 255 },
        }
    };
    GXColor vtxColor[4];

    s16 colorIndex;
    void *frameFile;
    HUSPRID spriteId;
    ANIMDATA *backgroundAnim;
    frameFile = HuAR_ARAMtoMRAMFileRead(frameFileTbl[frame*2], HU_MEMNUM_OVL, HEAP_MODEL);
    winP->animFrame[0] = HuSprAnimRead(frameFile);
    spriteId = winP->sprId[0] = HuSprCreate(winP->animFrame[0], prio, 0);
    HuSprGrpMemberSet(winP->grpId, 0, spriteId);
    HuSprTPLvlSet(winP->grpId, 0, 0.9f);
    backgroundAnim = HuSprAnimMake(winP->winW/16, winP->winH/16, ANIM_BMP_IA4);
    HuSprBGSet(winP->grpId, 0, backgroundAnim, 0);
    winP->bgPalNum = winBGMake(backgroundAnim);
    if(frameFileTbl[(frame*2)+1] != HU_DATANUM_NONE) {
        frameFile = HuAR_ARAMtoMRAMFileRead(frameFileTbl[(frame*2)+1], HU_MEMNUM_OVL, HEAP_MODEL);
        winP->animFrame[1] = HuSprAnimRead(frameFile);
        spriteId = winP->sprId[1] = HuSprCreate(winP->animFrame[1], prio, 0);
        HuSprGrpMemberSet(winP->grpId, 1, spriteId);
        HuSprBGSet(winP->grpId, 1, backgroundAnim, 0);
        vtxColor[0] = vtxColor[1] = color[frame][0];
        vtxColor[2] = vtxColor[3] = color[frame][1];
        if(frame != HUWIN_FRAME_DARK) {
            for(colorIndex=0; colorIndex<4; colorIndex++) {
                vtxColor[colorIndex].r *= 0.8;
                vtxColor[colorIndex].g *= 0.8;
                vtxColor[colorIndex].b *= 0.8;
            }
        }
        HuSprVtxColorSet(winP->grpId, 0, vtxColor);
    } else {
        winP->animFrame[1] = NULL;
    }
}

// Replaces a window's frame art while preserving whether the window is currently hidden.
void HuWinFrameSet(HUWINID winId, s16 frame)
{
    HUWIN *winP = &winData[winId];
    HuSprGrpMemberKill(winP->grpId, 0);
    if(winP->animFrame[1]) {
        HuSprGrpMemberKill(winP->grpId, 1);
    }
    HuWinFrameCreate(winP, frame, winP->prio+1);
    if(winP->attr & HUWIN_ATTR_DISPOFF) {
        HuWinDispOff(winId);
    }
}

// Frees a window's text storage and sprite group when its owner closes the window.
void HuWinKill(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 spriteIndex;

    if(winP->grpId == HUSPR_GROUP_NONE) {
        return;
    }
    HuMemDirectFree(winP->charEntry);
    if(winP->mesCopy) {
        HuMemDirectFree(winP->mesCopy);
    }

    for(spriteIndex=HUWIN_SPR_BEGIN; spriteIndex<HUWIN_SPR_MAX; spriteIndex++) {
        if(winP->sprId[spriteIndex] != HUSPR_NONE) {
            HuSprGrpMemberKill(winP->grpId, spriteIndex);
        }
    }
    HuSprGrpKill(winP->grpId);
    winP->grpId = HUSPR_GROUP_NONE;
}

// Releases all live windows and shared window resources during window-system shutdown.
void HuWinAllKill(void)
{
    HUWINID windowIndex;
    for(windowIndex=0; windowIndex<HUWIN_MAX; windowIndex++) {
        if(winData[windowIndex].grpId != HUSPR_GROUP_NONE) {
            HuWinKill(windowIndex);
        }
    }
    if(fontAnim) {
        HuSprAnimKill(fontAnim);
        fontAnim = NULL;
    }
    if(iconAnim) {
        HuSprAnimKill(iconAnim);
        iconAnim = NULL;
    }
    if(cursorAnim) {
        HuSprAnimKill(cursorAnim);
        cursorAnim = NULL;
    }
    if(cardAnimA) {
        HuSprAnimKill(cardAnimA);
        cardAnimA = NULL;
    }
    if(cardAnimB) {
        HuSprAnimKill(cardAnimB);
        cardAnimB = NULL;
    }
    if(winProc) {
        HuPrcKill(winProc);
        winProc = NULL;
    }
    if(messDataPtr) {
        HuMemDirectFree(messDataPtr);
        messDataPtr = NULL;
    }
    HuDataDirClose(DATA_win);
}

// Returns the requested message string from the loaded message archive.
char *HuWinMesPtrGet(u32 messNum)
{
    if(!messDataPtr) {
        OSReport("Error: Not Read Mess Data.");
        return NULL;
    }
    return HuWinMesDataPtrGet(messDataPtr, messNum);
}

// Returns the number of message directories in the loaded archive.
u32 HuWinMesMaxDirGet(void)
{
    u32 *dirNum = messDataPtr;
    return *dirNum;
}

// Returns the message count in the directory encoded by the high half of dirNum.
// An out-of-range directory is reported but is still used to index the archive.
u32 HuWinMesMaxNumGet(u32 dirNum)
{
    u32 directoryIndex = dirNum >> 16;
    u32 *messData = messDataPtr;
    if(directoryIndex >= *messData) {
        OSReport("Error: Message Dir Over\n");
    }
    messData++;
    messData += messData[directoryIndex]/sizeof(u32);
    return *messData;
}

// Resolves a packed directory/message ID to the first byte of its encoded message.
// Out-of-range indices are reported but are still used to index the archive.
void *HuWinMesDataPtrGet(void *data, u32 messNum)
{
    u32 directoryIndex = messNum >> 16;
    u32 messageIndex = messNum & WIN_MESSAGE_INDEX_MASK;
    u32 *messageData = data;
    if(directoryIndex >= *messageData) {
        OSReport("Error: Message Dir Over\n");
    }
    messageData++;
    messageData += messageData[directoryIndex]/sizeof(u32);
    if(messageIndex >= *messageData) {
        OSReport("Error: Message Number Over\n");
    }
    messageData++;
    messageData += messageData[messageIndex]/sizeof(u32);
    messageData++;
    return messageData;
}

// Called by the sprite renderer to draw the window's visible message glyphs.
static void MesDispFunc(HUSPRITE *sprP)
{
    HUWIN *winP = &winData[sprP->work[0]];
    HUSPR_GROUP *sprGrpP;
    float uvMaxX;
    float uvMaxY;
    float uvMinX;
    float uvMinY;
    float charW;
    float charX;
    float charY;
    float charUvH;
    s16 i;
    u16 alpha;
    s16 color;
    Mtx scale;
    Mtx modelview;
    Mtx44 proj;

    if(winP->charEntryNum) {
        sprGrpP = &HuSprGrpData[winP->grpId];
        GXInvalidateTexAll();
        C_MTXOrtho(proj, 0.0f, HU_DISP_HEIGHT, 0.0f, HU_DISP_WIDTH, 0.0f, 10.0f);
        GXSetProjection(proj, GX_ORTHOGRAPHIC);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_RGBA6, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGB, GX_RGBA8, 0);
        GXSetCullMode(GX_CULL_NONE);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetNumTevStages(1);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_RASC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, 1, GX_DF_CLAMP, GX_AF_SPOT);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZCompLoc(0);
        MTXRotRad(modelview, 'z', MTXDegToRad(sprP->zRot));
        MTXScale(scale, sprP->scale.x, sprP->scale.y, 1.0f);
        MTXConcat(modelview, scale, modelview);
        mtxTransCat(modelview, sprP->pos.x, sprP->pos.y, 0.0f);
        MTXConcat(*sprP->groupMtx, modelview, modelview);
        GXLoadPosMtxImm(modelview, 0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        HuSprTexLoad(fontAnim, 0, 0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
        GXBegin(GX_QUADS, GX_VTXFMT0, winP->charEntryNum*4);
        charUvH = (LanguageNo == HUWIN_LANG_JAPAN) ? (24.0/432.0) : (24.0f/312.0f);
        for(i=0; i<winP->charEntryNum; i++) {
            charW = fontWidthP[winP->charEntry[i].charNo+48];
            uvMinX = (20.0f/320.0f) * (winP->charEntry[i].charNo%16);
            uvMinY = charUvH * (winP->charEntry[i].charNo/16);
            charW--;
            uvMaxX = uvMinX + (charW/320.0);
            uvMaxY = uvMinY + charUvH - (1.0/432.0);
            charX = winP->charEntry[i].x;
            charY = winP->charEntry[i].y;
            color = winP->charEntry[i].color;
            if(winP->mesSpeed != 0) {
                alpha = winP->charEntry[i].fade * 8;
            } else {
                alpha = 255;
            }
            // This window path currently draws glyphs fully opaque at every reveal step.
            alpha = 255;
            if(winP->charEntry[i].fade < 31) {
                winP->charEntry[i].fade++;
            }
            GXPosition3f32(charX + 1.0f, charY, 0.0f);
            GXColor4u8(winP->mesPal[color][0], winP->mesPal[color][1], winP->mesPal[color][2],
                       alpha);
            GXPosition2f32(uvMinX, uvMinY);
            GXPosition3f32(charX + charW, charY, 0.0f);
            GXColor4u8(winP->mesPal[color][0], winP->mesPal[color][1], winP->mesPal[color][2],
                       alpha);
            GXPosition2f32(uvMaxX, uvMinY);
            GXPosition3f32(charX + charW, charY + 23.0f, 0.0f);
            GXColor4u8(winP->mesPal[color][0], winP->mesPal[color][1], winP->mesPal[color][2],
                       alpha);
            GXPosition2f32(uvMaxX, uvMaxY);
            GXPosition3f32(charX + 1.0f, charY + 23.0f, 0.0f);
            GXColor4u8(winP->mesPal[color][0], winP->mesPal[color][1], winP->mesPal[color][2],
                       alpha);
            GXPosition2f32(uvMinX, uvMaxY);
        }
        GXEnd();
        mesCharCnt++;
    }
}

// Called while creating a window to build its tiled background border pixels.
static u8 winBGMake(ANIMDATA *bgAnim)
{
    ANIMBMP *bmp;
    s16 blockH;
    s16 h;
    s16 blockW;
    s16 w;
    s16 i;
    s16 j;
    u8 *bmpData;

    w = bgAnim->bmp->sizeX;
    h = bgAnim->bmp->sizeY;
    blockW = (w+7) & 248;
    blockH = (h+3) & 252;
    bmpData = bgAnim->bmp->data = HuMemDirectMallocNum(HEAP_HEAP, blockW*blockH, HU_MEMNUM_OVL);
    for(i=0; i<h; i++) {
        if(i == 0) {
            for(j=0; j<w; j++) {
                if(j == 0) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 0;
                } else if(j == w-1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 16;
                } else {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 112;
                }
            }
        } else if(i == h-1) {
            for(j = 0; j < w; j++) {
                if(j == 0) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 32;
                } else if(j == w - 1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 48;
                } else {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 96;
                }
            }
        } else {
            for(j = 0; j < w; j++) {
                if(j == 0) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 64;
                } else if(j == w-1) {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 80;
                } else {
                    bmpData[(j&7)+((j>>3)<<5)+(i>>2)*(blockW*4)+(i&3)*8] = 128;
                }
            }
        }
    }
    DCStoreRangeNoSync(bgAnim->bmp->data, blockW*blockH);
    return w;
}

// Window task: advances message, key-wait, and choice state once per frame.
static void HuWinProc(void)
{
    HUWIN *winP;
    HUWINID winId;

    while(1) {
        HuPrcVSleep();
        winP = winData;
        for(winId=0; winId<HUWIN_MAX; winId++, winP++) {
            if(winP->grpId != HUSPR_GROUP_NONE && !(winP->attr & HUWIN_ATTR_DISPOFF)) {
                if(omUPauseFlag+omPauseChk() == 0 || !(winP->attr & HUWIN_ATTR_UPAUSE)) {
                    switch(winP->stat) {
                        case HUWIN_STAT_NONE:
                            break;

                        case HUWIN_STAT_DRAWMES:
                            HuWinDrawMes(winId);
                            break;

                        case HUWIN_STAT_KEYWAIT:
                            HuWinComKeyGet(winId, winKey);
                            HuWinKeyWait(winId);
                            break;

                        case HUWIN_STAT_CHOICE:
                            if (!(winP->attr & HUWIN_ATTR_CHOICEON) ||
                                (HuWinActiveKeyGetX(winP) & PAD_BUTTON_DPAD) == 0) {
                                winP->attr &= ~HUWIN_ATTR_CHOICEON;
                                HuWinComKeyGet(winId, winKey);
                                HuWinChoice(winP);
                            }
                            break;
                    }
                }

            }
        }
    }
}

// Queues one message glyph at window-relative coordinates for the next draw.
static void charEntry(s16 window, s16 x, s16 y, s16 charNo, s16 color)
{
    HUWIN *winP = &winData[window];
    WINCHARENTRY *winCharP = winP->charEntry;

    winCharP = &winP->charEntry[winP->charEntryNum];
    winCharP->x = x - winP->winW / 2;
    winCharP->y = y - winP->winH / 2;
    winCharP->charNo = charNo-48;
    winCharP->color = color;
    winCharP->fade = 0;
    winP->charEntryNum++;
    if(winP->charEntryNum >= winP->charEntryMax) {
        // Keep the visible count below capacity; later glyphs overwrite the final, undrawn slot.
        winP->charEntryNum = winP->charEntryMax-1;
    }
}

// Called by HuWinProc while a window is revealing text; handles message controls,
// line wrapping, inline choices, and adding visible glyphs to the draw queue.
static void HuWinDrawMes(HUWINID winId) {
    HUWIN *winP = &winData[winId];
    HUSPR_GROUP *sprGrpP;
    BOOL extCtrlF;

    s16 i;
    s16 charW;
    s16 tabW;
    s16 insertMesNo;
    s16 endF;
    s16 c;
    s16 shadowColor;
    s16 color;
    s16 messW;

    extCtrlF = FALSE;
    sprGrpP = &HuSprGrpData[winP->grpId];
    winP->mesTime += 3;
    while(winP->mesTime >= winP->mesSpeed || (winP->attr & HUWIN_ATTR_CHOICE)) {
        winP->mesTime = 0;
        charW = winP->charPadX + fontWidthP[winP->messData[0]];
        endF = FALSE;
        extCtrlF = FALSE;
        if(winP->messData[0] != 0 && (winP->attr & HUWIN_ATTR_KEYWAIT_MESS)) {
            winP->attr &= ~HUWIN_ATTR_KEYWAIT_MESS;
            _HuWinHomeClear(winP);
            if(winP->attr & HUWIN_ATTR_ALIGN_CENTER) {
                messW = GetMesMaxSizeSub2(winP, winP->messData);
                winP->mesX = (winP->mesRectW - messW) / 2;
            } else if(winP->attr & HUWIN_ATTR_ALIGN_RIGHT) {
                messW = GetMesMaxSizeSub2(winP, winP->messData);
                winP->mesX = winP->mesRectW - messW;
            }
        }
        while(winP->messData[0] < 48 || winP->messData[0] == 255) {
            switch(winP->messData[0]) {
                case 0:
                    if(winP->messSp == 0) {
                        for(i=0; i<HUWIN_CHOICE_MAX; i++) {
                            winP->choiceDisable[i] = FALSE;
                        }
                        winP->stat = 0;
                        winP->attr &= ~HUWIN_ATTR_SETCOLOR;
                        return;
                    }
                    winP->messSp--;
                    winP->messData = winP->messDataStack[winP->messSp];
                    break;

                case 25:
                case 31:
                    winP->messData++;
                    insertMesNo = winP->messData[0]-1;
                    if(winP->messDataInsert[insertMesNo] != 0) {
                        winP->messDataStack[winP->messSp] = winP->messData;
                        winP->messData = winP->messDataInsert[insertMesNo];
                        winP->messSp++;
                        if(winP->messData[0] != 11) {
                            winP->messData--;
                        }
                    }
                    break;

                case 11:
                    winP->attr &= ~(HUWIN_ATTR_WHITESPACE|HUWIN_ATTR_CHOICE);
                    _HuWinHomeClear(winP);
                    if(winP->attr & HUWIN_ATTR_ALIGN_CENTER) {
                        messW = GetMesMaxSizeSub2(winP, winP->messData+1);
                        winP->mesX = (winP->mesRectW-messW)/2;
                    } else if(winP->attr & HUWIN_ATTR_ALIGN_RIGHT) {
                        messW = GetMesMaxSizeSub2(winP, winP->messData+1);
                        winP->mesX = winP->mesRectW-messW;
                    }
                    break;

                case 30:
                    winP->messData++;
                    if(!(winP->attr & HUWIN_ATTR_SETCOLOR)) {
                        winP->mesCol = winP->messData[0]-1;
                    }
                    break;

                case 29:
                    winP->attr ^= HUWIN_ATTR_OUTLINE;
                    break;

                case 10:
                    winP->attr &= ~(HUWIN_ATTR_TEXTDISABLE|HUWIN_ATTR_CHOICE);
                    if(winP->attr & HUWIN_ATTR_WHITESPACE) {
                        if(!(winP->attr & HUWIN_ATTR_NOCR)) {
                            if(HuWinCR(winP) != 0) {
                                winP->messData++;
                                HuWinKeyWaitEntry(winId);
                                winP->attr |= HUWIN_ATTR_KEYWAIT_CLEAR;
                                return;
                            }
                            break;
                        }
                        charW = fontWidthP[16]+winP->charPadX;
                        /* fallthrough */
                case 16:
                case 27:
                case 32:
                        winP->attr |= HUWIN_ATTR_WHITESPACE;
                        if(winP->mesX+charW > winP->mesRectW) {
                            if(HuWinCR(winP) != 0) {
                                winP->messData++;
                                HuWinKeyWaitEntry(winId);
                                winP->attr |= HUWIN_ATTR_KEYWAIT_CLEAR;
                                return;
                            }
                            break;
                        }
                        winP->mesX += charW;
                    }
                    break;

                case 14:
                    winP->attr |= HUWIN_ATTR_WHITESPACE;
                    winP->messData++;
                    tabW = winP->charPadX+spcFontTbl[winP->messData[0]-1].glyphWidth;
                    if(winP->mesX+tabW > winP->mesRectW && HuWinCR(winP)) {
                        winP->messData--;
                        HuWinKeyWaitEntry(winId);
                        winP->attr |= HUWIN_ATTR_KEYWAIT_CLEAR;
                        return;
                    }
                    HuWinSpcFontEntry(winP, winP->messData[0] - 1, winP->mesRectX + winP->mesX,
                                      winP->mesRectY + winP->mesY);
                    winP->mesX += tabW;
                    endF = TRUE;
                    break;

                case 28:
                    winP->messData++;
                    HuAudFXPlay(speakerSeTbl[winP->messData[0]-1]);
                    break;

                case 255:
                    winP->messData++;
                    HuWinKeyWaitEntry(winId);
                    winP->attr |= HUWIN_ATTR_KEYWAIT_MESS;
                    winP->attr &= ~HUWIN_ATTR_WHITESPACE;
                    return;

                case 13:
                    winP->choice = winP->choiceNum;

                case 15:
                    winP->attr |= HUWIN_ATTR_CHOICE;
                    if(winP->choiceDisable[winP->choiceNum]) {
                        winP->attr |= HUWIN_ATTR_TEXTDISABLE;
                        winP->choiceData[winP->choiceNum].stat |= 1;
                    } else {
                        winP->choiceData[winP->choiceNum].stat &= ~1;
                    }

                    winP->choiceData[winP->choiceNum].x = winP->mesX + winP->mesRectX;
                    winP->choiceData[winP->choiceNum].y = winP->mesY + winP->mesRectY;
                    winP->choiceNum++;
                    break;

                case 12:
                    winP->attr |= HUWIN_ATTR_WHITESPACE;
                    tabW = winP->tabW*((winP->mesX+winP->tabW)/winP->tabW)-winP->mesX;
                    if(winP->mesX+tabW > winP->mesRectW) {
                        if(HuWinCR(winP)) {
                            winP->messData++;
                            HuWinKeyWaitEntry(winId);
                            winP->attr |= HUWIN_ATTR_KEYWAIT_CLEAR;
                            return;
                        }
                    } else {
                        winP->mesX += tabW;
                    }
                    break;

                case 9:
                    extCtrlF = TRUE;
                    break;

                case 26:
                    winP->messData++;
                    if(winP->callback) {
                        winP->callback(winId, winP->origMes, winP->messData[0]);
                    }
                    break;
            }
            winP->messData++;
            if(!extCtrlF) {
                charW = winP->charPadX+fontWidthP[winP->messData[0]];
            } else {
                charW = charWFixedTbl[winP->messData[1]-1]+winP->charPadX;
                break;
            }

            if(endF) {
                break;
            }
        }
        if(endF == FALSE) {
            if(winP->mesX+charW > winP->mesRectW && HuWinCR(winP)) {
                HuWinKeyWaitEntry(winId);
                winP->attr |= HUWIN_ATTR_KEYWAIT_CLEAR;
                return;
            }
            c = winP->messData[0];
            winP->attr |= HUWIN_ATTR_WHITESPACE;
            color = (winP->attr & HUWIN_ATTR_TEXTDISABLE) ? HUWIN_MESCOL_DARKGRAY : winP->mesCol;
            if(winP->attr & HUWIN_ATTR_OUTLINE) {
                shadowColor = HUWIN_MESCOL_BLACK;
                charEntry(winId, winP->mesRectX + winP->mesX + 2, winP->mesRectY + winP->mesY, c,
                          shadowColor);
                charEntry(winId, winP->mesRectX + winP->mesX - 2, winP->mesRectY + winP->mesY, c,
                          shadowColor);
                charEntry(winId, winP->mesRectX + winP->mesX, winP->mesRectY + winP->mesY + 2, c,
                          shadowColor);
                charEntry(winId, winP->mesRectX + winP->mesX, winP->mesRectY + winP->mesY - 2, c,
                          shadowColor);
                charEntry(winId, winP->mesRectX+winP->mesX, winP->mesRectY+winP->mesY, c, color);
            } else {
                charEntry(winId, winP->mesRectX + winP->mesX + 2, winP->mesRectY + winP->mesY + 2,
                          c, winP->mesColShadow);
                charEntry(winId, winP->mesRectX+winP->mesX, winP->mesRectY+winP->mesY, c, color);
            }
            winP->mesX += charW;
            winP->messData++;
        }
    }
}

// Advances to the next message line and reports when the text area has wrapped.
static BOOL HuWinCR(HUWIN *winP)
{
    BOOL ret;
    u32 ofs;
    u32 messW;
    if(winP->mesY+48 > winP->mesRectH) {
        winP->mesY = 0;
        ret = TRUE;
    } else {
        winP->mesY += winP->charPadY+24;
        ret = FALSE;
    }

    if(winP->messData[0] == 10) {
        ofs = 1;
    } else {
        ofs = 0;
    }
    if(winP->attr & HUWIN_ATTR_ALIGN_CENTER) {
        messW = GetMesMaxSizeSub2(winP, winP->messData+ofs);
        winP->mesX = (winP->mesRectW - messW) / 2;
    } else if(winP->attr & HUWIN_ATTR_ALIGN_RIGHT) {
        messW = GetMesMaxSizeSub2(winP, winP->messData+ofs);
        winP->mesX = winP->mesRectW - messW;
    } else {
        winP->mesX = 0;
    }
    return ret;
}

// Clears the current page's glyphs, choice markers, and special-font sprites.
static void _HuWinHomeClear(HUWIN *winP)
{
    s16 i;

    winP->charEntryNum = 0;
    winP->mesX = winP->mesY = 0;
    winP->choiceNum = 0;
    HuWinSpcFontClear(winP);
    winP->attr &= ~(HUWIN_ATTR_TEXTDISABLE|HUWIN_ATTR_CHOICE);
    for(i=0; i<HUWIN_CHOICE_MAX; i++) {
        winP->choiceData[i].stat = 0;
    }
}

// Publicly resets a window to its empty page after message or choice handling.
void HuWinHomeClear(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    _HuWinHomeClear(winP);
    for(i=0; i<HUWIN_CHOICE_MAX; i++) {
        winP->choiceDisable[i] = FALSE;
    }
    winP->stat = HUWIN_STAT_NONE;
    winP->attr &= ~(HUWIN_ATTR_SETCOLOR|HUWIN_ATTR_OUTLINE);
    winP->messSp = 0;
}

// Called when message flow reaches a pause; starts the input prompt unless skipped.
void HuWinKeyWaitEntry(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    if(winP->attr & HUWIN_ATTR_SKIP_KEYWAIT) {
        // A skipped wait makes the window idle here; it does not resume the remaining message.
        winP->stat = HUWIN_STAT_NONE;
    } else {
        HUSPR_GROUP *gp;
        winP->stat = HUWIN_STAT_KEYWAIT;
        winP->keyWaitSprNo = HuWinSpcFontEntry(winP, 19, winP->mesRectW-24, winP->mesRectH-24);
        gp = &HuSprGrpData[winP->grpId];
        HuSprAttrSet(winP->grpId, winP->keyWaitSprNo, HUSPR_ATTR_NOPAUSE);
    }
}

// Called each frame during a message pause; resumes text when an accepted key lands.
static void HuWinKeyWait(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 messW;
    if(winP->pushKey & HuWinActivePadGet(winP)) {
        winP->activePadKey = HuWinActivePadGet(winP);
        winP->stat = HUWIN_STAT_DRAWMES;
        HuAudFXPlay(MSM_SE_CMN_25);
        HuWinSprKill(winId, winP->keyWaitSprNo);
        if(winP->attr & HUWIN_ATTR_KEYWAIT_CLEAR) {
            _HuWinHomeClear(winP);
            if(winP->attr & HUWIN_ATTR_ALIGN_CENTER) {
                messW = GetMesMaxSizeSub2(winP, winP->messData);
                winP->mesX = (winP->mesRectW - messW) / 2;
            } else if(winP->attr & HUWIN_ATTR_ALIGN_RIGHT) {
                messW = GetMesMaxSizeSub2(winP, winP->messData);
                winP->mesX = winP->mesRectW - messW;
            } else {
                winP->mesX = 0;
            }
        }

        winP->attr &= ~HUWIN_ATTR_KEYWAIT_CLEAR;
    }
}

// Adds a special glyph sprite such as the key-wait prompt or choice cursor.
static s16 HuWinSpcFontEntry(HUWIN *winP, s16 entry, s16 x, s16 y)
{
    HUSPR_GROUP *sprGrpP = &HuSprGrpData[winP->grpId];
    HUSPRID sprId;
    s16 i;
    ANIMDATA *anim;

    for(i=HUWIN_SPCFONT_BEGIN; i<HUWIN_SPCFONT_END; i++) {
        if(winP->sprId[i] == HUSPR_NONE) {
            anim = *spcFontTbl[entry].animationSlot;
            winP->sprId[i] = sprId = HuSprCreate(anim, winP->prio-1, spcFontTbl[entry].animBank);
            HuSprGrpMemberSet(winP->grpId, i, sprId);
            HuSprPosSet(winP->grpId, i, (x + spcFontTbl[entry].centerOffsetX) - (winP->winW / 2),
                        (y + spcFontTbl[entry].centerOffsetY) - (winP->winH / 2));
            break;
        }
    }
    HuSprGrpDrawNoSet(winP->grpId, winP->drawNo);
    return i;
}

// Moves an existing special glyph sprite to window-relative coordinates.
static void HuWinSpcFontPosSet(HUWIN *winP, s16 sprNo, s16 x, s16 y)
{
    HUSPR_GROUP *sprGrpP = &HuSprGrpData[winP->grpId];
    HuSprPosSet(winP->grpId, sprNo, x-(winP->winW/2), y-(winP->winH/2));
}

// Removes all special glyph sprites owned by this window.
static void HuWinSpcFontClear(HUWIN *winP)
{
    s16 i;

    for(i=HUWIN_SPCFONT_BEGIN; i<HUWIN_SPCFONT_END; i++) {
        if(winP->sprId[i] != HUSPR_NONE) {
            HuSprGrpMemberKill(winP->grpId, i);
            winP->sprId[i] = HUSPR_NONE;
        }
    }
}

#define WIN_CHOICEDIR_NONE -1
#define WIN_CHOICEDIR_LEFT 0
#define WIN_CHOICEDIR_UP 1
#define WIN_CHOICEDIR_RIGHT 2
#define WIN_CHOICEDIR_DOWN 3
#define WIN_CHOICE_DIST_INVALID 100000.0f

// Called by HuWinProc during choice mode to move, confirm, or cancel the cursor.
static void HuWinChoice(HUWIN *winP) {
    WINCHOICE *choiceP;
    float choiceDist;
    float minChoiceDist;
    s16 choiceDistY;
    s16 key;
    s16 choiceCurrX;
    s16 choiceCurrY;
    s16 choiceCurr;
    s16 choiceNum;
    s16 choiceDistX;
    s16 dir;
    s16 choiceY;
    s16 choice;
    s16 i;

    choiceCurr = winP->choice;
    choice = choiceCurr;
    dir = WIN_CHOICEDIR_NONE;
    key = HuWinActivePadGet(winP);
    // Simultaneous directions prefer down, then up, right, and left.
    if(key & PAD_BUTTON_LEFT) {
        dir = WIN_CHOICEDIR_LEFT;
    }
    if(key & PAD_BUTTON_RIGHT) {
        dir = WIN_CHOICEDIR_RIGHT;
    }
    if(key & PAD_BUTTON_UP) {
        dir = WIN_CHOICEDIR_UP;
    }
    if(key & PAD_BUTTON_DOWN) {
        dir = WIN_CHOICEDIR_DOWN;
    }
    choiceCurrX = winP->choiceData[choiceCurr].x;
    choiceCurrY = winP->choiceData[choiceCurr].y;
    minChoiceDist = WIN_CHOICE_DIST_INVALID;
    choiceDist = minChoiceDist;
    choiceNum = winP->choiceNum;
    switch(dir) {
        case WIN_CHOICEDIR_LEFT:
            for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                if (i != choiceCurr && !(choiceP->stat & 1) && choiceP->y == choiceCurrY &&
                    choiceP->x < choiceCurrX) {
                    break;
                }
            }
            if(i != choiceNum) {
                for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                    if (i != choiceCurr && !(choiceP->stat & 1) && choiceP->y == choiceCurrY &&
                        choiceP->x < choiceCurrX && choiceDist > choiceCurrX - choiceP->x) {
                        choiceDist = choiceCurrX - choiceP->x;
                        choice = i;
                    }
                }
            }
            break;
        case WIN_CHOICEDIR_UP:
            for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y < choiceCurrY) {
                    break;
                }
            }
            if(i != choiceNum) {
                for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                    if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y < choiceCurrY) {
                        choiceDistY = choiceCurrY - choiceP->y;
                        if(choiceDistY <= choiceDist) {
                            if(choiceDistY < choiceDist) {
                                minChoiceDist = WIN_CHOICE_DIST_INVALID;
                            }
                            choiceDistX = choiceCurrX - choiceP->x;
                            choiceDistX = choiceDistX * choiceDistX;
                            if(choiceDistX < minChoiceDist) {
                                choiceDist = choiceDistY;
                                minChoiceDist = choiceDistX;
                                choice = i;
                            }
                        }
                    }
                }
            } else {
                choiceY = -1000;
                for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                    if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y > choiceY) {
                        choiceY = choiceP->y;
                    }
                }
                if(choiceY != choiceCurrY) {
                    minChoiceDist = WIN_CHOICE_DIST_INVALID;
                    for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                        if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y == choiceY) {
                            choiceDistX = choiceCurrX - choiceP->x;
                            choiceDistX = choiceDistX * choiceDistX;
                            if(choiceDistX < minChoiceDist) {
                                minChoiceDist = choiceDistX;
                                choice = i;
                            }
                        }
                    }
                }
            }
            break;

        case WIN_CHOICEDIR_RIGHT:
            for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                if (i != choiceCurr && !(choiceP->stat & 1) && choiceP->y == choiceCurrY &&
                    choiceP->x > choiceCurrX) {
                    break;
                }
            }
            if(i != choiceNum) {
                for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                    if (i != choiceCurr && !(choiceP->stat & 1) && choiceP->y == choiceCurrY &&
                        choiceP->x > choiceCurrX && choiceDist > choiceP->x - choiceCurrX) {
                        choiceDist = choiceP->x - choiceCurrX;
                        choice = i;
                    }
                }
            }
            break;

        case WIN_CHOICEDIR_DOWN:
            for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y > choiceCurrY) {
                    break;
                }
            }
            if(i != choiceNum) {
                for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                    if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y > choiceCurrY) {
                        choiceDistY = choiceP->y - choiceCurrY;
                        if(choiceDistY <= choiceDist) {
                            if(choiceDistY < choiceDist) {
                                minChoiceDist = WIN_CHOICE_DIST_INVALID;
                            }
                            choiceDistX = choiceCurrX - choiceP->x;
                            choiceDistX = choiceDistX * choiceDistX;
                            if(choiceDistX < minChoiceDist) {
                                choiceDist = choiceDistY;
                                minChoiceDist = choiceDistX;
                                choice = i;
                            }
                        }
                    }
                }
            } else {
                choiceY = 1000;
                for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                    if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y < choiceY) {
                        choiceY = choiceP->y;
                    }
                }
                if(choiceY != choiceCurrY) {
                    minChoiceDist = WIN_CHOICE_DIST_INVALID;
                    for(i=0, choiceP=winP->choiceData; i<choiceNum; i++, choiceP++) {
                        if(i != choiceCurr && !(choiceP->stat & 1) && choiceP->y == choiceY) {
                            choiceDistX = choiceCurrX - choiceP->x;
                            choiceDistX = choiceDistX * choiceDistX;
                            if(choiceDistX < minChoiceDist) {
                                minChoiceDist = choiceDistX;
                                choice = i;
                            }
                        }
                    }
                }
            }
            break;
    }
    if(winP->choice != choice) {
        winP->choice = choice;
        HuAudFXPlay(MSM_SE_CMN_01);
    } else if(key & PAD_BUTTON_A) {
        if(winP->choiceEndSe >= 0) {
            HuAudFXPlay(winP->choiceEndSe);
        }
        winP->activePadKey = key;
        winP->stat = 0;
    } else if((key & PAD_BUTTON_B) && !(winP->attr & HUWIN_ATTR_NOCANCEL)) {
        HuAudFXPlay(MSM_SE_CMN_04);
        // Canceling keeps the previously stored activePadKey.
        (void)key;
        winP->choice = -1;
        winP->stat = 0;
    }
    HuWinSpcFontPosSet(winP, winP->cursorSprNo,
                       winP->choiceData[choice].x + spcFontTbl[23].centerOffsetX,
                       winP->choiceData[choice].y + spcFontTbl[23].centerOffsetY);
}

#undef WIN_CHOICEDIR_NONE
#undef WIN_CHOICEDIR_LEFT
#undef WIN_CHOICEDIR_UP
#undef WIN_CHOICEDIR_RIGHT
#undef WIN_CHOICEDIR_DOWN
#undef WIN_CHOICE_DIST_INVALID

// Combines scripted or live keys for the controller ports selected by the window's pad mask.
static u32 HuWinActivePadGet(HUWIN *winP)
{
    s32 key;
    u32 i;

    key = 0;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(winP->padMask & (1 << i)) {
            key |= winKey[i];
        }
    }
    return key;
}

// Returns button and directional input from enabled human players; choice-mode gating tests its
// directional bits.
static u32 HuWinActiveKeyGetX(HUWIN *winP)
{
    u32 btn;
    u32 i;
    u32 j;
    u8 padMask;

    btn = 0;
    padMask = (winP->padMask & ~winP->disablePlayer);
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(padMask & (1 << i)) {
            for(j=0; j<GW_PLAYER_MAX; j++) {
                if(i == GwPlayerConf[j].padNo) {
                    break;
                }
            }
            if(j != GW_PLAYER_MAX && GwPlayerConf[j].type == GW_PLAYER_TYPE_MAN) {
                btn |= HuPadBtn[GwPlayerConf[j].padNo]|HuPadDStk[GwPlayerConf[j].padNo];
            }
        }
    }
    return btn;
}

// Sets the window's screen position; HUWIN_POS_CENTER selects an axis midpoint.
void HuWinPosSet(HUWINID winId, float posX, float posY)
{
    HUWIN *winP = &winData[winId];
    if(posX == HUWIN_POS_CENTER) {
        winP->pos.x = (HU_DISP_WIDTH-winP->winW)/2;
    } else {
        winP->pos.x = posX;
    }
    if(posY == HUWIN_POS_CENTER) {
        winP->pos.y = (HU_DISP_HEIGHT-winP->winH)/2;
    } else {
        winP->pos.y = posY;
    }
    HuSprGrpPosSet(winP->grpId, winP->pos.x, winP->pos.y);
}

// Sets the window sprite group's horizontal and vertical scale.
void HuWinScaleSet(HUWINID winId, float scaleX, float scaleY)
{
    HUWIN *winP = &winData[winId];
    winP->scale.x = scaleX;
    winP->scale.y = scaleY;
    HuSprGrpScaleSet(winP->grpId, scaleX, scaleY);
}

// Sets the window sprite group's rotation around the screen's Z axis.
void HuWinZRotSet(HUWINID winId, float zRot)
{
    HUWIN *winP = &winData[winId];
    winP->zRot = zRot;
    HuSprGrpZRotSet(winP->grpId, zRot);
}

// Sets the point within the window used as the sprite group's transform center.
void HuWinCenterPosSet(HUWINID winId, float centerX, float centerY)
{
    HUWIN *winP = &winData[winId];
    HuSprGrpCenterSet(winP->grpId, (winP->winW/2.0f)-centerX, (winP->winH/2.0f)-centerY);
}

// Assigns the window's installed sprites to the draw queue selected by HuSprExec.
void HuWinDrawNoSet(HUWINID winId, s16 drawNo)
{
    HUWIN *winP = &winData[winId];
    winP->drawNo = drawNo;
    HuSprGrpDrawNoSet(winP->grpId, winP->drawNo);
}

// Restricts window drawing to the supplied screen-space rectangle.
void HuWinScissorSet(HUWINID winId, s16 x, s16 y, s16 w, s16 h)
{
    HUWIN *winP = &winData[winId];
    HuSprGrpScissorSet(winP->grpId, x, y, w, h);
}

// Updates the priority of the window frame and its active text sprites.
void HuWinPriSet(HUWINID winId, s16 prio)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    HuSprPriSet(winP->grpId, 0, prio);
    HuSprPriSet(winP->grpId, 1, prio);
    HuSprPriSet(winP->grpId, 2, prio);
    for(i=HUWIN_SPR_BEGIN; i<HUWIN_SPR_MAX; i++) {
        if(winP->sprId[i] != HUSPR_NONE) {
            HuSprPriSet(winP->grpId, i, prio-1);
        }
    }
    winP->prio = prio;
}

// Enables the supplied window behavior flags.
void HuWinAttrSet(HUWINID winId, u32 attr)
{
    HUWIN *winP = &winData[winId];
    winP->attr |= attr;
}

// Disables the supplied window behavior flags.
void HuWinAttrReset(HUWINID winId, u32 attr)
{
    HUWIN *winP = &winData[winId];
    winP->attr &= ~attr;
}

// Returns the current message, key-wait, or choice state of a window.
s16 HuWinStatGet(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    return winP->stat;
}

// Sets the current message palette entry and keeps later text from resetting it.
void HuWinMesColSet(HUWINID winId, u8 mesCol)
{
    HUWIN *winP = &winData[winId];
    winP->mesCol = mesCol;
    winP->attr |= HUWIN_ATTR_SETCOLOR;
}

// Changes one message palette entry's RGB color.
void HuWinMesPalSet(HUWINID winId, u8 mesCol, u8 r, u8 g, u8 b)
{
    HUWIN *winP = &winData[winId];
    winP->mesPal[mesCol][0] = r;
    winP->mesPal[mesCol][1] = g;
    winP->mesPal[mesCol][2] = b;
}

// Sets background transparency and hides or shows the two background sprites.
void HuWinBGTPLvlSet(HUWINID winId, float tpLvl)
{
    HUWIN *winP = &winData[winId];
    HuSprTPLvlSet(winP->grpId, 0, tpLvl);
    HuSprTPLvlSet(winP->grpId, 1, tpLvl);
    if(tpLvl == 0) {
        HuSprDispOff(winP->grpId, 0);
        HuSprDispOff(winP->grpId, 1);
    } else if(!(winP->attr & HUWIN_ATTR_DISPOFF)) {
        HuSprDispOn(winP->grpId, 0);
        HuSprDispOn(winP->grpId, 1);
    }
}

// Applies a tint to the window background, preserving full vertex alpha.
void HuWinBGColSet(HUWINID winId, GXColor *bgCol)
{
    HUWIN *winP = &winData[winId];
    if(!winP->animFrame[1]) {

        HuSprColorSet(winP->grpId, 0, bgCol->r, bgCol->g, bgCol->b);
    } else {

        GXColor vtxColor[4];
        vtxColor[0] = vtxColor[1] = vtxColor[2] = vtxColor[3] = *bgCol;
        vtxColor[0].a = vtxColor[1].a = vtxColor[2].a = vtxColor[3].a = 255;

        HuSprVtxColorSet(winP->grpId, 0, vtxColor);
    }

}

// Sets the reveal threshold, advanced by three timing units per frame; zero removes the glyph
// delay.
void HuWinMesSpeedSet(HUWINID winId, s16 mesSpeed)
{
    HUWIN *winP = &winData[winId];
    winP->mesSpeed = mesSpeed;
}

// Loads the message archive for the selected language into window message memory.
void HuWinMesRead(void)
{
    void *buf;
    char *path;

    if(messDataPtr) {
        HuMemDirectFree(messDataPtr);
    }
    path = mesDataTbl[LanguageNo];
    buf = HuDvdDataRead(path);
    messDataPtr = HuMemDirectMalloc(HEAP_HEAP, DirDataSize);
    memcpy(messDataPtr, buf, DirDataSize);
    HuMemDirectFree(buf);
}

// Selects which language archive HuWinMesRead loads.
void HuWinMesLanguageSet(HUWIN_LANG lang)
{
    LanguageNo = lang;
}

static BOOL HuWinMesCopyCheck(char *messP);
static char *HuWinMesCopy(HUWINID winId, char *messP);

// Starts a message by archive ID or inline pointer and resets the current page.
// With no loaded archive, an archive ID leaves the old page and pointer in draw-message state.
void HuWinMesSet(HUWINID winId, u32 messNum)
{
    HUWIN *winP = &winData[winId];
    s16 messW;
    winP->stat = HUWIN_STAT_DRAWMES;
    if(!(messNum & WIN_INLINE_MESSAGE_POINTER_BIT_PATTERN)) {
        if(!messDataPtr) {
            OSReport("Error: No Message\n");
            return;
        }
        winP->messData = HuWinMesPtrGet(messNum);
        if(!winP->messData) {
            OSReport("Error: No Message data\n");
        }
    } else {
        winP->messData = (void *)messNum;
    }
    if(HuWinMesCopyCheck(winP->messData)) {
        winP->messData = HuWinMesCopy(winId, winP->messData);
    }
    _HuWinHomeClear(winP);
    winP->origMes = messNum;
    if(winP->attr & HUWIN_ATTR_ALIGN_CENTER) {
        messW = GetMesMaxSizeSub2(winP, winP->messData);
        winP->mesX = (winP->mesRectW - messW) / 2;
    } else if(winP->attr & HUWIN_ATTR_ALIGN_RIGHT) {
        messW = GetMesMaxSizeSub2(winP, winP->messData);
        winP->mesX = winP->mesRectW - messW;
    }
    if(!(winP->attr & HUWIN_ATTR_SETCOLOR)) {
        winP->mesCol = HUWIN_MESCOL_WHITE;
    }
}

// Sets an insertion slot to an archive message ID or inline message pointer.
void HuWinInsertMesSet(HUWINID winId, u32 messNum, s16 insertMesNo)
{
    HUWIN *winP = &winData[winId];
    s16 messW;
    if(!(messNum & WIN_INLINE_MESSAGE_POINTER_BIT_PATTERN)) {
        if(!messDataPtr) {
            OSReport("Error: No Message\n");
            return;
        }
        winP->messDataInsert[insertMesNo] = HuWinMesPtrGet(messNum);
        if(!winP->messDataInsert[insertMesNo]) {
            OSReport("Error: No Message data\n");
        }
    } else {
        winP->messDataInsert[insertMesNo] = (void *)messNum;
    }
}

static char *mesWordTblEng[10] = {
    "star", "coin", "Star", "Coin", "orb", "Orb", "point", "Point", "space", "Space"
};

static char *mesWordTblEngPlural[10] = {
    "stars", "coins", "Stars", "Coins", "orbs", "Orbs", "points", "Points", "spaces", "Spaces"
};

// Concatenates decimal digits from the whole inserted message, ignoring nondigits and skipping
// the arguments of the handled message controls.
int HuWinAtoi(char *message)
{
    char *messageCursor;
    int digitIndex;
    int digitCount;
    int value;
    int placeValue;
    char digit[12];

    digitCount = 0;
    messageCursor = message;
    while(*messageCursor) {
        if(*messageCursor >= '0' && *messageCursor <= '9') {
            // ASCII decimal digits share their numeric value in the low four bits.
            digit[digitCount++] = *messageCursor & WIN_DECIMAL_DIGIT_MASK;
        }
        switch(*messageCursor) {
            case 14:
            case 25:
            case 26:
            case 28:
            case 30:
            case 31:
                messageCursor++;
                break;
        }
        messageCursor++;
    }
    value = 0;
    for(placeValue=1, digitIndex=digitCount-1; digitIndex>=0; digitIndex--, placeValue *= 10) {
        value += digit[digitIndex]*placeValue;
    }
    (void)messageCursor;
    (void)messageCursor;
    (void)messageCursor;
    (void)messageCursor;
    return value;
}

// After a count insertion, skips nonletters and matches a noun prefix at the first letter;
// stops at newline, page clear, input wait, or string end.
static char *HuWinPluralGet(char *message, char **singular, char **plural)
{
    s16 i;
    s16 j;
    while(*message) {
        if(*message == 10 || *message == 11 || *message == 255) {
            break;
        }
        for(i=0; i<10; i++) {
            for(j=0; j<strlen(mesWordTblEng[i]); j++) {
                if(message[j] != mesWordTblEng[i][j]) {
                    break;
                }
            }
            if(j == strlen(mesWordTblEng[i])) {
                *singular = mesWordTblEng[i];
                *plural = mesWordTblEngPlural[i];
                return message;
            }
        }
        if((*message >= 'A' && *message <= 'Z') || (*message >= 'a' && *message <= 'z')) {
            break;
        }
        message++;
    }

    return NULL;
}

// Copies the message, replacing matched English noun prefixes with plural text for counts
// other than one. Count one leaves the wording unchanged; existing suffix characters remain.
static char *HuWinMesCopy(HUWINID winId, char *messP)
{
    char *messageInput;
    char *messageOutput;
    HUWIN *winP;
    s16 messageLength;
    s16 insertMesNo;
    char *wordStart;
    char *plural;
    char *singular;

    winP = &winData[winId];
    for(messageLength=0; messP[messageLength]; messageLength++) {

    }
    if(winP->mesCopy) {
        HuMemDirectFree(winP->mesCopy);
    }
    winP->mesCopy = messageOutput = HuMemDirectMalloc(HEAP_HEAP, messageLength*2);
    for(messageInput=messP; *messageInput;) {
        if(*messageInput == 25) {
            *messageOutput = *messageInput;
            messageOutput++;
            messageInput++;
            insertMesNo = *messageInput-1;
            if(winP->messDataInsert[insertMesNo]) {
                int insertedNumber;
                insertedNumber = HuWinAtoi(winP->messDataInsert[insertMesNo]);
                if(insertedNumber != 1) {
                    wordStart = HuWinPluralGet(messageInput, &singular, &plural);
                    if(wordStart) {
                       while(wordStart > messageInput) {
                            *messageOutput++ = *messageInput++;
                       }
                       *messageOutput = 0;
                       strcat(messageOutput, plural);
                       messageInput += strlen(singular);
                       messageOutput += strlen(plural);
                    }
                }
            }

        }
        *messageOutput = *messageInput;
        messageOutput++;
        messageInput++;
    }
    *messageOutput = 0;
    return winP->mesCopy;
}

// Checks whether a message contains a count insertion that needs plural handling.
static BOOL HuWinMesCopyCheck(char *messP)
{
    char *s = messP;
    while(*s) {
        switch(*s) {
            case 14:
            case 26:
            case 28:
            case 30:
            case 31:
                s++;
                break;

            case 25:
                return TRUE;
        }
        s++;
    }
    return FALSE;
}

// Chooses the requested or next enabled choice when choiceNo is nonnegative; -1 reuses the current
// selection when it is below choiceNum, then enters choice mode.
s16 HuWinChoiceSet(HUWINID winId, s16 choiceNo)
{
    HUWIN *winP = &winData[winId];
    winP->attr |= HUWIN_ATTR_CHOICEON;
    while(winP->stat != HUWIN_STAT_NONE) {
        HuPrcVSleep();
    }
    if(choiceNo != -1) {
        while(choiceNo < winP->choiceNum) {
            if(!(winP->choiceData[choiceNo].stat & 1)) {
                break;
            }
            choiceNo++;
        }
        if(choiceNo == winP->choiceNum) {
            for(choiceNo=0; choiceNo<winP->choiceNum; choiceNo++) {
                if(!(winP->choiceData[choiceNo].stat & 1)) {
                    break;
                }
            }
            if(choiceNo == winP->choiceNum) {
                return -1;
            }
        }
        winP->choice = choiceNo;
    } else {
        if(winP->choice >= winP->choiceNum) {
            return -1;
        }
    }
    winP->cursorSprNo = HuWinSpcFontEntry(winP, 23, winP->choiceData[winP->choice].x,
                                          winP->choiceData[winP->choice].y);
    winP->stat = HUWIN_STAT_CHOICE;
    return 0;
}

// Called by menu and confirmation flows to show choices and wait for the player's selection.
s16 HuWinChoiceGet(HUWINID winId, s16 choiceNo)
{
    HUWIN *winP = &winData[winId];
    s16 result = HuWinChoiceSet(winId, choiceNo);
    if(result != 0) {
        return result;
    }
    while(winP->stat != HUWIN_STAT_NONE) {
        HuPrcVSleep();
    }
    return winP->choice;
}

// Returns the number of choices parsed for this window.
s16 HuWinChoiceNumGet(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    return winP->choiceNum;
}

// Marks a choice disabled for the next time its message choice marker is parsed.
void HuWinChoiceDisable(HUWINID winId, s16 choiceNo)
{
    HUWIN *winP = &winData[winId];
    winP->choiceDisable[choiceNo] = TRUE;
}

// Returns the current selection while a choice is active; callers poll it during menu updates.
s16 HuWinChoiceNowGet(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    if(winP->stat == HUWIN_STAT_CHOICE) {
        return winP->choice;
    } else {
        return -1;
    }
}

// Message scripts call this after starting text to wait until the window reports completion.
void HuWinMesWait(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    while(winP->stat != HUWIN_STAT_NONE) {
        HuPrcVSleep();
    }
}

// Adds an animated sprite to a window, used for speaker portraits and other window artwork.
s16 HuWinAnimSet(HUWINID winId, ANIMDATA *anim, s16 animBank, float posX, float posY)
{
    HUWIN *winP = &winData[winId];
    HUSPRID sprId = HuSprCreate(anim, winP->prio-1, animBank);
    return HuWinSprSet(winId, sprId, posX, posY);
}

// Adds a sprite to the first free window sprite slot at the requested window-relative point.
s16 HuWinSprSet(HUWINID winId, HUSPRID sprId, float posX, float posY)
{
    HUWIN *winP = &winData[winId];
    HUSPR_GROUP *sprGrpP = &HuSprGrpData[winP->grpId];
    s16 i;
    for(i=HUWIN_SPR_BEGIN; i<=HUWIN_SPR_END; i++){
        if(winP->sprId[i] == HUSPR_NONE) {
            winP->sprId[i] = sprId;
            HuSprGrpMemberSet(winP->grpId, i, sprId);
            HuSprPosSet(winP->grpId, i, posX-sprGrpP->center.x, posY-sprGrpP->center.y);
            break;
        }
    }
    return i;
}

// Repositions a window sprite at window-relative coordinates, offsetting them by the group center.
void HuWinSprPosSet(HUWINID winId, s16 sprNo, float posX, float posY)
{
    HUWIN *winP = &winData[winId];
    HUSPR_GROUP *sprGrpP = &HuSprGrpData[winP->grpId];
    HuSprPosSet(winP->grpId, sprNo, posX-sprGrpP->center.x, posY-sprGrpP->center.y);

}

// Changes the draw priority of one sprite in a window's sprite group.
void HuWinSprPriSet(HUWINID winId, s16 sprNo, s16 prio)
{
    HUWIN *winP = &winData[winId];
    HUSPR_GROUP *sprGrpP = &HuSprGrpData[winP->grpId];
    HuSprPriSet(winP->grpId, sprNo, prio);
}

// Returns the sprite ID stored in the requested window sprite slot.
HUSPRID HuWinSprIDGet(HUWINID winId, s16 sprNo)
{
    HUWIN *winP = &winData[winId];
    return winP->sprId[sprNo];
}

// Removes a sprite from the window group and marks its slot as free for later additions.
void HuWinSprKill(HUWINID winId, s16 sprNo)
{
    HUWIN *winP = &winData[winId];
    HuSprGrpMemberKill(winP->grpId, sprNo);
    winP->sprId[sprNo] = HUSPR_NONE;
}

// Hides every sprite in a window; menu and message flows use this while a window is closed.
void HuWinDispOff(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    for(i=0; i<HUWIN_SPR_MAX; i++) {
        if(winP->sprId[i] != HUSPR_NONE) {
            HuSprDispOff(winP->grpId, i);
        }
    }
    winP->attr |= HUWIN_ATTR_DISPOFF;
}

// Shows every installed sprite in a window when its owner opens or resumes the window.
void HuWinDispOn(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    for(i=0; i<HUWIN_SPR_MAX; i++) {
        if(winP->sprId[i] != HUSPR_NONE) {
            HuSprDispOn(winP->grpId, i);
        }
    }
    winP->attr &= ~HUWIN_ATTR_DISPOFF;
}

// Queues neutral keys (or HUWIN_COMKEY_NONE passthrough per controller) for time input polls,
// then the requested keys for one poll of the shared queue.
void HuWinComKeyWait(s32 keyP1, s32 keyP2, s32 keyP3, s32 keyP4, s16 time)
{
    _HuWinComKeySet((keyP1 == HUWIN_COMKEY_NONE) ? HUWIN_COMKEY_NONE : 0,
        (keyP2 == HUWIN_COMKEY_NONE) ? HUWIN_COMKEY_NONE : 0,
        (keyP3 == HUWIN_COMKEY_NONE) ? HUWIN_COMKEY_NONE : 0,
        (keyP4 == HUWIN_COMKEY_NONE) ? HUWIN_COMKEY_NONE : 0,
        time);
    _HuWinComKeySet(keyP1, keyP2, keyP3, keyP4, 1);
}

void HuWinComKeySet(s32 keyP1, s32 keyP2, s32 keyP3, s32 keyP4)
{
    _HuWinComKeySet(keyP1, keyP2, keyP3, keyP4, 1);
}

// Appends one scripted input entry, indexed by controller port, to the window input ring buffer.
void _HuWinComKeySet(s32 keyP1, s32 keyP2, s32 keyP3, s32 keyP4, s16 time)
{
    winComKeyBuf[comKeyIdx].controllerKey[0] = keyP1;
    winComKeyBuf[comKeyIdx].controllerKey[1] = keyP2;
    winComKeyBuf[comKeyIdx].controllerKey[2] = keyP3;
    winComKeyBuf[comKeyIdx].controllerKey[3] = keyP4;
    winComKeyBuf[comKeyIdx].remainingPolls = time;
    comKeyIdx++;
    comKeyIdx &= 255;
}

// Reads live input from enabled controller ports when paused or no script is queued.
// Otherwise returns scripted keys per port; HUWIN_COMKEY_NONE falls back to that port's enabled
// live input.
void HuWinComKeyGet(HUWINID winId, u32 *key)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    if(comKeyIdx == comKeyIdxNow || omUPauseFlag+omPauseChk() != 0 || Hu3DPauseF) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(!(winP->disablePlayer & (1 << i))) {
                key[i] = HuPadDStkRep[i]|HuPadBtnDown[i];
            } else {
                key[i] = 0;
            }
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            key[i] = winComKeyBuf[comKeyIdxNow].controllerKey[i];
            if(key[i] == HUWIN_COMKEY_NONE) {
                if(!(winP->disablePlayer & (1 << i))) {
                    key[i] = HuPadDStkRep[i]|HuPadBtnDown[i];
                } else {
                    key[i] = 0;
                }
            }
        }
        winComKeyBuf[comKeyIdxNow].remainingPolls--;
        if(winComKeyBuf[comKeyIdxNow].remainingPolls <= 0) {
            comKeyIdxNow++;
            comKeyIdxNow &= 255;
        }
    }
}

void HuWinComKeyReset(void)
{
    comKeyIdx = comKeyIdxNow = 0;
}

// Sets the controller-port mask used to select input for this window.
void HuWinPadMaskSet(HUWINID winId, s16 mask)
{
    HUWIN *winP = &winData[winId];
    winP->padMask = mask;
}

// Measures the largest message width and height, then clears cached insertion sizes and resets
// the measurement tab width to 24 pixels and newline handling to its default.
void HuWinMesMaxSizeGet(s16 messCnt, HuVec2f *maxSize, ...)
{
    s16 i;
    va_list vaList;
    winInsertF = FALSE;
    va_start(vaList, maxSize);
    winMaxWidth = winMaxHeight = 0;
    for(i=0; i<messCnt; i++) {
        u32 messNum = va_arg(vaList, u32);
        GetMesMaxSizeSub(messNum);
    }
    for(i=0; i<HUWIN_INSERTMES_MAX; i++) {
        mesWInsert[i] = mesHInsert[i] = 0;
    }
    winTabSize = 24;
    cancelCRF = FALSE;
    maxSize->x = (winMaxWidth+31)&WIN_TILE_DIMENSION_MASK;
    maxSize->y = winMaxHeight+16;
    va_end(vaList);
}

// Measures an inserted message and stores its width for later measurement of the containing
// message.
void HuWinInsertMesSizeGet(u32 messNum, s16 insertMesNo)
{
    winInsertF = TRUE;
    winMaxWidth = winMaxHeight = 0;
    GetMesMaxSizeSub(messNum);
    mesWInsert[insertMesNo] = winMaxWidth;
}

void HuWinMesSizeCancelCRSet(s32 cancelCR)
{
    cancelCRF = cancelCR;
}

// Measures an inclusive message-number range, then clears insertion sizes and resets tab width
// and newline handling. If messEnd precedes messStart, returns 100 by 100 without those resets.
void HuWinMesMaxSizeBetGet(HuVec2f *maxSize, u32 messStart, u32 messEnd)
{
    u32 i;
    if(messEnd < messStart) {
        maxSize->x = maxSize->y = 100;
        return;
    }
    winInsertF = FALSE;
    winMaxWidth = winMaxHeight = 0;
    for(i=messStart; i<=messEnd; i++) {
        GetMesMaxSizeSub(i);
    }
    for(i=0; i<HUWIN_INSERTMES_MAX; i++) {
        mesWInsert[i] = mesHInsert[i] = 0;
    }
    winTabSize = 24;
    cancelCRF = FALSE;
    maxSize->x = (winMaxWidth+31)&WIN_TILE_DIMENSION_MASK;
    maxSize->y = winMaxHeight+16;
}

// Measures a message archive entry or inline message for window sizing calls.
static s32 GetMesMaxSizeSub(u32 messNum)
{
    s16 winHeight;
    s16 winWidth;
    s16 charW;
    s16 charH;
    char *messData;
    char *messDataOrig = NULL;
    BOOL crF = FALSE;
    s16 messDataF;

    if(messNum > WIN_INLINE_MESSAGE_POINTER_BIT_PATTERN) {
        messDataF = FALSE;
        messData = (char *)messNum;
    } else {
        messDataF = TRUE;
        messData = messDataOrig = HuWinMesPtrGet(messNum);
    }
    winHeight = 26;
    winWidth = 0;
    while(messData[0] != 0) {
        charH = 0;
        charW = fontWidthP[messData[0]]+1;
        switch(messData[0]) {
            case 16:
            case 27:
            case 32:
                break;

            case 30:
                messData++;
                charW = 0;
                break;

            case 10:
                if(crF) {
                    if(cancelCRF == FALSE) {
                        if(winMaxWidth < winWidth) {
                            winMaxWidth = winWidth;
                        }
                        winWidth = charW = 0;
                        if(messData[1] != 0) {
                            charH = 26;
                        }
                    } else {
                        charW = fontWidthP[16] + 1;
                    }
                }
                break;
            case 255:
                winWidth += charW;
                /* fallthrough */
            case 11:
                crF = FALSE;
                if(winInsertF == FALSE) {
                    if(winMaxWidth < winWidth) {
                        winMaxWidth = winWidth;
                    }
                    if(winMaxHeight < winHeight) {
                        winMaxHeight = winHeight;
                    }
                    winHeight = 26;
                    charH = 0;
                    winWidth = charW = 0;
                }
                break;

            case 28:
                messData++;
                /* fallthrough */
            case 15:
            case 29:
                charW = 0;
                break;

            case 12:
                charW = winTabSize*((winWidth+winTabSize)/winTabSize)-winWidth;
                break;

            case 14:
                crF = TRUE;
                messData++;
                charW = spcFontTbl[messData[0]-1].glyphWidth+1;
                break;

            case 25:
                // Reserve an extra 's' width for every pluralizing insertion, even when the noun
                // stays singular.
                winWidth += fontWidthP['s'];

            case 31:
                crF = TRUE;
                messData++;
                charW = mesWInsert[messData[0]-1];
                break;

            case 26:
                charW = 0;
                messData++;
                break;
        }
        if((messData[0] != 255 && messData[0] >= 32) || messData[0] == 16) {
            crF = TRUE;
        }
        winWidth += charW;
        winHeight += charH;
        messData++;
    }
    if(winMaxWidth < winWidth) {
        winMaxWidth = winWidth;
    }
    if(winMaxHeight < winHeight) {
        winMaxHeight = winHeight;
    }
    return winWidth;
}

// Returns alignment width up to a newline (unless NOCR is set), input wait, or string end;
// page-clear controls discard the width accumulated so far.
static s32 GetMesMaxSizeSub2(HUWIN *winP, void *data)
{
    s16 i;
    char *messDataOrig = NULL;
    u8 messEnd;
    s16 winHeight;
    s16 winWidth;
    s16 charW;
    s16 charH;
    char *messData;

    for(i=0; i<HUWIN_INSERTMES_MAX; i++) {
        if(winP->messDataInsert[i]) {
            HuWinInsertMesSizeGet((u32)winP->messDataInsert[i], i);
        }
    }
    messData = data;
    winHeight = 26;
    winWidth = 0;
    messEnd = FALSE;
    while(messData[0] != 0 && !messEnd) {
        charH = 0;
        charW = fontWidthP[messData[0]]+1;
        switch(messData[0]) {
            case 16:
            case 32:
                break;

            case 10:
                if(!(winP->attr & 256)) {
                    charW = 0;
                    messEnd = TRUE;
                }
                break;

            case 255:
                charW = 0;
                messEnd = TRUE;
                break;

            case 11:
                winHeight = 26;
                charH = 0;
                winWidth = charW = 0;
                break;

            case 28:
            case 30:
                messData++;
                /* fallthrough */
            case 15:
            case 29:
                charW = 0;
                break;

            case 12:
                charW = winTabSize*((winWidth+winTabSize)/winTabSize)-winWidth;
                break;

            case 14:
                messData++;
                charW = spcFontTbl[messData[0]-1].glyphWidth+1;
                break;

            case 25:
            case 31:
                messData++;
                charW = mesWInsert[messData[0]-1];
                break;

            case 26:
                charW = 0;
                messData++;
                break;
        }
        winWidth += charW;
        winHeight += charH;
        messData++;
    }
    return winWidth;
}

// Counts bytes equal to the input-pause marker in an archive or inline message, including
// arguments.
s16 HuWinKeyWaitNumGet(u32 messNum)
{
    s16 waitNum;
    char *messData;
    if(messNum > WIN_INLINE_MESSAGE_POINTER_BIT_PATTERN) {
        messData = (char *)messNum;
    } else {
        messData = HuWinMesPtrGet(messNum);
    }
    for(waitNum=0; messData[0]; messData++) {
        if(*messData == 255) {
            waitNum++;
        }
    }
    return waitNum;
}

// Sets which button inputs can resume this window from a key-wait pause.
void HuWinPushKeySet(HUWINID winId, s16 pushKey)
{
    HUWIN *winP = &winData[winId];
    winP->pushKey = pushKey;
}

// Adds controller-port bits to the window's disabled live-input mask.
void HuWinDisablePlayerSet(HUWINID winId, u8 controllerBits)
{
    HUWIN *winP = &winData[winId];
    winP->disablePlayer |= controllerBits;
}

// Removes controller-port bits from the window's disabled live-input mask.
void HuWinDisablePlayerReset(HUWINID winId, u8 controllerBits)
{
    HUWIN *winP = &winData[winId];
    winP->disablePlayer &= ~controllerBits;
}

// No-op message callback that callers may explicitly install when no action is needed.
void HuWinCallbackStub(HUWINID winId, u32 mess, s16 c)
{
    (void)mess;
    &winData[winId];

}

void HuWinCallbackSet(HUWINID winId, HUWIN_CALLBACK cb)
{
    winData[winId].callback = cb;
}

static unsigned int speakerFileTbl[HUWIN_SPEAKER_MAX] = {
    WIN_ANM_face_hanachan, WIN_ANM_face_hanachan_star,
    WIN_ANM_face_kuribo, WIN_ANM_face_nokonoko_start,
    WIN_ANM_face_kokamekku, WIN_ANM_face_kamekku,
    WIN_ANM_face_sun, WIN_ANM_face_moon,
    WIN_ANM_face_nokonoko, WIN_ANM_face_heiho,
    WIN_ANM_face_teresa, WIN_ANM_face_battan,
    WIN_ANM_face_donkey, WIN_ANM_face_koopa,
    WIN_ANM_face_oyama, WIN_ANM_face_yariho,
    WIN_ANM_face_ukki, WIN_ANM_face_kinokio,
    WIN_ANM_face_warukio, WIN_ANM_face_minikoopa_r,
    WIN_ANM_face_minikoopa_g, WIN_ANM_face_minikoopa_b,
    WIN_ANM_face_wanwan, WIN_ANM_face_sunmoon
};

// Creates the file-selection message window; sun, moon, and sun/moon speakers use themed frames.
HUWINID HuWinExCreate(float x, float y, s16 w, s16 h, s16 speakerNo)
{
    s16 frame;
    if(speakerNo == HUWIN_SPEAKER_SUN) {
        frame = HUWIN_FRAME_DAY;
    } else if(speakerNo == HUWIN_SPEAKER_MOON) {
        frame = HUWIN_FRAME_NIGHT;
    } else if(speakerNo == HUWIN_SPEAKER_SUNMOON) {
        frame = HUWIN_FRAME_DAYNIGHT;
    } else {
        frame = HUWIN_FRAME_DEFAULT;
    }
    return HuWinExCreateFrame(x, y, w, h, speakerNo, frame);
}

// Creates a framed message window, raises its height to at least 84 pixels when a speaker is
// requested, and optionally installs the speaker portrait. Leaves the window hidden until opened.
HUWINID HuWinExCreateFrame(float x, float y, s16 w, s16 h, s16 speakerNo, s16 frame)
{

    HUWINID winId;
    HUWIN *winP;
    if(speakerNo >= 0) {
        h = (h < 84) ? 84 : h;
    }
    winId = HuWinCreate(x, y, w, h, frame);
    winP = &winData[winId];
    HuSpr3DSet(winP->sprId[0]);
    if(winP->animFrame[1]) {
        HuSpr3DSet(winP->sprId[1]);
    }
    if(speakerNo >= 0) {
        void *data = HuAR_ARAMtoMRAMFileRead(speakerFileTbl[speakerNo], HU_MEMNUM_OVL, HEAP_MODEL);
        ANIMDATA *anim = HuSprAnimRead(data);
        HuWinAnimSet(winId, anim, 0, 48, 48);
        HuSpr3DSet(winP->sprId[HUWIN_SPR_BEGIN]);
    }
    HuWinDispOff(winId);
    return winId;
}

// Clears the current page, then opens a framed dialog window with its turn animation before text is
// shown.
void HuWinExOpen(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    float t;
    _HuWinHomeClear(winP);
    if(winP->sprId[HUWIN_SPR_BEGIN] == HUSPR_NONE) {
        HuSpr3DRotSet(winP->sprId[0], 90, 0, 0);
        if(winP->animFrame[1]) {
            HuSpr3DRotSet(winP->sprId[1], 90, 0, 0);
        }
        HuWinDispOn(winId);
        for(i=1; i<=10; i++) {
            t = 90.0-(9.0*i);
            HuSpr3DRotSet(winP->sprId[0], t, 0, 0);
            if(winP->animFrame[1]) {
                HuSpr3DRotSet(winP->sprId[1], t, 0, 0);
            }
            HuPrcVSleep();
        }
        HuSpr3DRotSet(winP->sprId[0], 0, 0, 0);
        if(winP->animFrame[1]) {
            HuSpr3DRotSet(winP->sprId[1], 0, 0, 0);
        }
        return;
    } else {
        HuSpr3DRotSet(winP->sprId[0], 90, 0, 0);
        if(winP->animFrame[1]) {
            HuSpr3DRotSet(winP->sprId[1], 90, 0, 0);
        }
        HuSpr3DRotSet(winP->sprId[HUWIN_SPR_BEGIN], 90, 0, 0);
        HuWinDispOn(winId);
        for(i=1; i<=10; i++) {
            t = 90.0-(9.0*i);
            HuSpr3DRotSet(winP->sprId[0], t, 0, 0);
            if(winP->animFrame[1]) {
                HuSpr3DRotSet(winP->sprId[1], t, 0, 0);
            }
            HuPrcVSleep();
        }
        for(i=1; i<=10; i++) {
            t = 90.0-(9.0*i);
            HuSpr3DRotSet(winP->sprId[HUWIN_SPR_BEGIN], -t*3, 0, 0);
            t = ((1-HuSin(t))*0.5)+0.5;
            HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN, t, t);
            HuPrcVSleep();
        }
    }
}

// Plays the closing turn animation and hides a framed dialog window after its caller finishes using
// it.
void HuWinExClose(HUWINID winId)
{
    HUWIN *winP = &winData[winId];
    s16 i;
    float t;
    _HuWinHomeClear(winP);
    if(winP->sprId[HUWIN_SPR_BEGIN] == HUSPR_NONE) {
        for(i=1; i<=10; i++) {
            t = 9.0*i;
            HuSpr3DRotSet(winP->sprId[0], t, 0, 0);
            if(winP->animFrame[1]) {
                HuSpr3DRotSet(winP->sprId[1], t, 0, 0);
            }
            HuPrcVSleep();
        }
        HuSpr3DRotSet(winP->sprId[0], 90, 0, 0);
        if(winP->animFrame[1]) {
            HuSpr3DRotSet(winP->sprId[1], 90, 0, 0);
        }
        HuWinDispOff(winId);
        return;
    } else {
        for(i=1; i<=5; i++) {
            t = 18.0*i;
            t = 1-HuSin(t);
            HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN, t, t);
            HuPrcVSleep();
        }
        for(i=1; i<=10; i++) {
            t = 9.0*i;
            HuSpr3DRotSet(winP->sprId[0], t, 0, 0);
            if(winP->animFrame[1]) {
                HuSpr3DRotSet(winP->sprId[1], t, 0, 0);
            }
            HuPrcVSleep();
        }
        HuSpr3DRotSet(winP->sprId[0], 90, 0, 0);
        if(winP->animFrame[1]) {
            HuSpr3DRotSet(winP->sprId[1], 90, 0, 0);
        }
        HuWinDispOff(winId);
    }
}

void HuWinExKill(HUWINID winId)
{
    HuWinKill(winId);
}

// Swaps the portrait sprite in a framed dialog window while animating it out and back in.
void HuWinExSpeakerSet(HUWINID winId, s16 speakerNo)
{
    HUWIN *winP = &winData[winId];
    s16 i;

    for(i=0; i<=10; i++) {
        HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN, 1.0f, HuCos(9.0f*i));
        HuPrcVSleep();
    }
    HuWinSprKill(winId, HUWIN_SPR_BEGIN);
    {
        void *data = HuAR_ARAMtoMRAMFileRead(speakerFileTbl[speakerNo], HU_MEMNUM_OVL, HEAP_MODEL);
        ANIMDATA *anim = HuSprAnimRead(data);
        HuWinAnimSet(winId, anim, 0, 48, 48);
    }
    for(i=0; i<=10; i++) {
        HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN, 1.0f, HuSin(9.0f*i));
        HuPrcVSleep();
    }
    HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN, 1.0f, 1.0f);
}

static void WarningGlowProc(void);

// Creates a warning dialog for save, file-selection, or microphone-device messages, adding
// 44 pixels to the requested height. Uses instant text and controller-port 1 input, and leaves
// the window hidden until opened.
HUWINID HuWinWarningCreate(float posX, float posY, s16 winW, s16 winH)
{
    HUWIN *winP;
    HUWIN_WARNING *warning;
    s16 i;
    HUWINID winId;
    HUSPRID sprId;
    void *data;
    ANIMDATA *anim;

    winH += 44;
    winId = HuWinCreate(posX, posY, winW, winH, HUWIN_FRAME_WARN);
    HuWinMesSpeedSet(winId, 0);

    winP = &winData[winId];
    HuSprTPLvlSet(winP->grpId, 0, 0.9f);
    winW = winP->winW;
    winH = winP->winH;
    winP->warning = warning = HuMemDirectMalloc(HEAP_HEAP, sizeof(HUWIN_WARNING));
    warning->winId = winId;
    data = HuAR_ARAMtoMRAMFileRead(WIN_ANM_warning, HU_MEMNUM_OVL, HEAP_MODEL);
    anim = HuSprAnimRead(data);
    sprId = winP->sprId[HUWIN_SPR_BEGIN] = HuSprCreate(anim, winP->prio, 0);
    HuSprGrpMemberSet(winP->grpId, HUWIN_SPR_BEGIN, sprId);
    HuSprPosSet(winP->grpId, HUWIN_SPR_BEGIN, 0, (-winP->winH/2)+28);
    data = HuAR_ARAMtoMRAMFileRead(WIN_ANM_frame_glow, HU_MEMNUM_OVL, HEAP_MODEL);
    warning->glowAnim = HuSprAnimRead(data);
    for(i=0; i<2; i++) {
        warning->bgAnim[i] = HuSprAnimMake(winW/16, winH/16, ANIM_BMP_IA4);
        sprId = winP->sprId[HUWIN_SPR_BEGIN+1+i] = HuSprCreate(warning->glowAnim, winP->prio+2, 0);
        HuSprGrpMemberSet(winP->grpId, HUWIN_SPR_BEGIN+1+i, sprId);
        HuSprBGSet(winP->grpId, HUWIN_SPR_BEGIN+1+i, warning->bgAnim[i], 0);
        HuSprColorSet(winP->grpId, HUWIN_SPR_BEGIN+1+i, 192, 0, 0);
        winBGMake(warning->bgAnim[i]);
        warning->scaleTimer[i] = -1;
    }
    winP->mesRectY = 52;
    winP->mesRectX += 4;
    winP->mesRectW -= 4;
    warning->process = HuPrcCreate(WarningGlowProc, 100, 4096, 0);
    warning->process->property = warning;
    HuPrcSetStat(warning->process, HU_PRC_STAT_PAUSE_ON|HU_PRC_STAT_UPAUSE_ON);
    HuWinCenterPosSet(winId, winW/2, winH/2);
    HuWinPosSet(winId, winP->pos.x+(winW/2), winP->pos.y+(winH/2));
    HuWinDispOff(winId);
    HuWinPadMaskSet(winId, 1);
    return winId;

}

// Clears the page, then opens a hidden warning window and starts the two glow layers' scale cycle.
void HuWinWarningOpen(HUWINID winId)
{
    HUWIN *winP;
    HUWIN_WARNING *warning;
    s16 i;
    float scale;

    winP = &winData[winId];
    warning = winP->warning;

    _HuWinHomeClear(winP);
    if(winP->attr & HUWIN_ATTR_DISPOFF) {
        winP->stat = HUWIN_STAT_NONE;
        HuWinDispOn(warning->winId);
        for(i=1; i<=8; i++) {
            scale = i/8.0f;
            scale = HuSin(scale*120)*(1.0/HuSin(120));
            HuWinScaleSet(winId, scale, scale);
            HuPrcVSleep();
        }
        warning->scaleTimer[0] = 0;
        warning->scaleTimer[1] = 20;
    }
}

// Clears the page and stops message processing. If visible, stops the glow timers, shrinks the
// warning window, and hides it; an already hidden window keeps its existing glow timers.
void HuWinWarningClose(HUWINID winId)
{
    HUWIN *winP;
    s16 i;
    HUWIN_WARNING *warning;
    float scale;

    winP = &winData[winId];
    warning = winP->warning;

    _HuWinHomeClear(winP);
    winP->stat = HUWIN_STAT_NONE;
    if(!(winP->attr & HUWIN_ATTR_DISPOFF)) {
        for(i=0; i<2; i++) {
            warning->scaleTimer[i] = -1;
        }
        for(i=1; i<=8; i++) {
            scale = 1.0-(i/8.0f);
            scale = HuSin(scale*120)*(1.0/HuSin(120));
            HuWinScaleSet(winId, scale, scale);
            HuPrcVSleep();
        }
        HuWinDispOff(warning->winId);
    }

}

// Warning task: pulses each glow layer while its timer is nonnegative.
// The task remains alive with stopped timers; active timers also advance while the window is
// hidden.
static void WarningGlowProc(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    HUWIN_WARNING *warning = process->property;
    HUWIN *winP = &winData[warning->winId];
    s16 i;
    while(1) {
        for(i=0; i<2; i++) {
            if(warning->scaleTimer[i] >= 0) {
                float scaleX = 1.0 + (0.025f * warning->scaleTimer[i]) *
                                    (((float) (winP->winW + 50) / (winP->winW)) - 1.0);
                float scaleY = 1.0 + (0.025f * warning->scaleTimer[i]) *
                                    (((float) (winP->winH + 50) / (winP->winH)) - 1.0);
                HuSprScaleSet(winP->grpId, HUWIN_SPR_BEGIN+1+i, scaleX, scaleY);
                scaleX = 1.0-(0.025*warning->scaleTimer[i]);
                HuSprTPLvlSet(winP->grpId, HUWIN_SPR_BEGIN+1+i, 0.7*scaleX);
                warning->scaleTimer[i]++;
                if(warning->scaleTimer[i] > 40.0f) {
                    warning->scaleTimer[i] = 0;
                }
            }
        }
        HuPrcVSleep();
    }
}

// Stops the warning's glow process, frees its state, and destroys its window when the owner is
// done.
void HuWinWarningKill(HUWINID winId)
{
    HUWIN *winP;
    HUWIN_WARNING *warning;

    winP = &winData[winId];
    warning = winP->warning;
    HuPrcKill(warning->process);
    HuMemDirectFree(warning);
    HuWinKill(winId);

}
