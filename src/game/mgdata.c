// Stores minigame metadata and converts minigame results to decathlon scores.
#include "game/mgdata.h"

#include "game/omovl.h"
#include "messnum/mg_name.h"
#include "messnum/mg_inst.h"

#include "datanum/instpic.h"

#define DECA_SCORE_TIME 0 // Lower normalized results earn more points.
#define DECA_SCORE_POINT 1 // Higher normalized results earn more points.
#define DECA_SCORE_NUM 11
#define MGDATA_COLOR_GOLD 4292351487u
#define MGDATA_COLOR_GREEN 1124039423u

int MgModeSubMode;
s16 MgModeScore[GW_PLAYER_MAX];
s16 MgGameNo;
u8 MgModePlayNum;
u8 MgBattleOrder[GW_PLAYER_MAX];
int lbl_802C0438;
int MgSubMode;
int MiracleBookEvtNo;
BOOL MgExitReq;
BOOL MgPauseExitF;
BOOL MgInstExitF;
BOOL MgBoard2Force;

u8 ATTRIBUTE_ALIGN(4) MgModeWork[256];
GW_COMMON GwCommonBackup;
int MgModeWorkInt[16];
float MgModeWorkFloat[16];

#include "mgdata.inc"

// Game, board, and object-system callers use this to find the table row for an overlay ID.
s32 MgNoGet(s16 overlayNo)
{
    MGDATA *minigameData;
    s16 minigameIndex;
    for (minigameData = &MgDataTbl[0], minigameIndex = 0; minigameData->ovl != (u16) DLL_NONE;
         minigameData++, minigameIndex++) {
        if(minigameData->ovl == overlayNo) {
            return minigameIndex;
        }
    }
    return -1;
}

// Returns the current minigame submode when a caller queries it.
int MgSubModeGet(void)
{
    return MgSubMode;
}

typedef struct MgDecaPoint_s {
    float decathlonScore; // Decathlon score at this point on the result curve.
    float normalizedValue; // Normalized minigame result at this point on the curve.
} MGDECAPOINT;

typedef struct MgDecaScore_s {
    s32 overlayNo; // Minigame overlay ID whose result uses this curve.
    s16 scoreType; // Whether better results increase or decrease the curve value.
    s16 pointCount; // Number of initialized points in the curve.
    MGDECAPOINT points[10]; // Result-to-score breakpoints, in curve order.
} MGDECASCORE;

// Curves map each decathlon minigame's normalized result to its per-game score.
static MGDECASCORE MgDecaScoreTbl[DECA_SCORE_NUM] = {
    {
        502,
        DECA_SCORE_TIME,
        5,
        {
            { 0, 90 },
            { 100, 66 },
            { 550, 42 },
            { 800, 31 },
            { 1000, 27 }
        }
    },
    {
        504,
        DECA_SCORE_TIME,
        5,
        {
            { 0, 90 },
            { 100, 72 },
            { 450, 42 },
            { 800, 23 },
            { 1000, 15 }
        }
    },
    {
        563,
        DECA_SCORE_POINT,
        7,
        {
            { 0, 0 },
            { 100, 8 },
            { 350, 18 },
            { 700, 28 },
            { 800, 31 },
            { 950, 39 },
            { 1000, 44 }
        }
    },
    {
        506,
        DECA_SCORE_POINT,
        6,
        {
            { 0, 0 },
            { 100, 60 },
            { 400, 100 },
            { 650, 123 },
            { 850, 140 },
            { 1000, 150 },
        }
    },
    {
        507,
        DECA_SCORE_TIME,
        5,
        {
            { 0, 30 },
            { 150, 20 },
            { 400, 10 },
            { 700, 3 },
            { 1000, 0.1 },
        }
    },
    {
        510,
        DECA_SCORE_POINT,
        7,
        {
            { 0, 0 },
            { 50, 5 },
            { 150, 10 },
            { 300, 15 },
            { 500, 20 },
            { 700, 25 },
            { 1000, 30 },
        }
    },
    {
        511,
        DECA_SCORE_TIME,
        7,
        {
            { 0, 90 },
            { 100, 75 },
            { 250, 65 },
            { 500, 55 },
            { 700, 50 },
            { 850, 45 },
            { 1000, 37 },
        }
    },
    {
        513,
        DECA_SCORE_TIME,
        7,
        {
            { 0, 60 },
            { 50, 25 },
            { 300, 17 },
            { 550, 13 },
            { 750, 11 },
            { 950, 8 },
            { 1000, 6 },
        }
    },
    {
        512,
        DECA_SCORE_TIME,
        7,
        {
            { 0, 90 },
            { 200, 48 },
            { 400, 38 },
            { 500, 33 },
            { 650, 30 },
            { 900, 28 },
            { 1000, 27 },
        }
    },
    {
        514,
        DECA_SCORE_POINT,
        7,
        {
            { 0, 0 },
            { 0, 12.32 },
            { 150, 20 },
            { 600, 30 },
            { 900, 40 },
            { 980, 45 },
            { 1000, 48 },
        }
    },
    {
        // MgDecaScoreCalc selects overlay 514, not this curve; these scores are 3.0009 times
        // overlay 514's.
        5141,
        DECA_SCORE_POINT,
        7,
        {
            { 0, 0 },
            { 0, 36.97109 },
            { 150, 60.018 },
            { 600, 90.027 },
            { 900, 120.036 },
            { 980, 135.0405 },
            { 1000, 144.0432 },
        }
    },
};

// The kernel export lets decathlon result flow convert a minigame result to its decathlon score.
int MgDecaScoreCalc(int decathlonGameIndex, int gameResult)
{
    MGDECASCORE *scoreEntry;
    MGDECAPOINT *previousPoint;
    MGDECAPOINT *currentPoint;
    int pointIndex;
    s32 overlayNo;
    float normalizedResult;
    int decathlonScore;
    decathlonScore = 1000;
    // Keep the raw result visible in the debug log; this output does not affect scoring.
    OSReport("%d\n", gameResult);
    overlayNo = -1;
    // Map each decathlon slot to its overlay curve and normalize its raw result using that slot's
    // scale.
    // A zero result on these time-based slots bypasses interpolation and returns zero.
    switch(decathlonGameIndex) {
        case 0:
            if(gameResult == 0) {
                decathlonScore = 0;
            } else {
                normalizedResult = gameResult/50.0f;
                overlayNo = 502;
            }
            break;
        
        case 1:
            if(gameResult == 0) {
                decathlonScore = 0;
            } else {
                normalizedResult = gameResult/50.0f;
                overlayNo = 504;
            }
            break;
       
       case 2:
            normalizedResult = gameResult;
            overlayNo = 563;
            break;
       
       case 3:
            normalizedResult = gameResult;
            overlayNo = 506;
            break;
       
       case 4:
            if(gameResult == 0) {
                decathlonScore = 0;
            } else {
                normalizedResult = gameResult/1000.0f;
                overlayNo = 507;
            }
            break;
       
       case 5:
            normalizedResult = gameResult;
            overlayNo = 510;
            break;
       
       case 6:
            if(gameResult == 0) {
                decathlonScore = 0;
            } else {
                normalizedResult = gameResult/50.0f;
                overlayNo = 511;
            }
            break;
       
       case 8:
            if(gameResult == 0) {
                decathlonScore = 0;
            } else {
                normalizedResult = gameResult/50.0f;
                overlayNo = 512;
            }
            break;
       
       case 7:
            if(gameResult == 0) {
                decathlonScore = 0;
            } else {
                normalizedResult = gameResult/50.0f;
                overlayNo = 513;
            }
            break;
       
       case 9:
            normalizedResult = gameResult/100.0f;
            overlayNo = 514;
            break;
    }
    // Unmapped slots keep the initial score of 1000; zero-result slots above keep their explicit
    // zero.
    if(overlayNo > 0) {
        
        int scoreEntryIndex;
        for (scoreEntry = &MgDecaScoreTbl[0], scoreEntryIndex = DECA_SCORE_NUM; scoreEntryIndex--;
             scoreEntry++) {
            if(scoreEntry->overlayNo == overlayNo) {
                break;
            }
        }
        previousPoint = NULL;
        currentPoint = &scoreEntry->points[0];
        decathlonScore = 1000;
        if(scoreEntry->scoreType != DECA_SCORE_TIME) {
            for (pointIndex = 0; pointIndex < scoreEntry->pointCount;
                 previousPoint = currentPoint, currentPoint++, pointIndex++) {
                if(normalizedResult <= currentPoint->normalizedValue) {
                    if(!previousPoint) {
                        decathlonScore = 0;
                        break;
                    } else {
                        // Linear interpolation is stored in int decathlonScore, truncating any
                        // fractional score.
                        decathlonScore = previousPoint->decathlonScore;
                        decathlonScore +=
                            (currentPoint->decathlonScore - previousPoint->decathlonScore) *
                            ((normalizedResult - previousPoint->normalizedValue) /
                             (currentPoint->normalizedValue - previousPoint->normalizedValue));
                        break;
                    }
                }
            }
        } else {
            for (pointIndex = 0; pointIndex < scoreEntry->pointCount;
                 previousPoint = currentPoint, currentPoint++, pointIndex++) {
                if(normalizedResult >= currentPoint->normalizedValue) {
                    if(!previousPoint) {
                        decathlonScore = 0;
                        break;
                    } else {
                        decathlonScore = previousPoint->decathlonScore;
                        decathlonScore +=
                            (currentPoint->decathlonScore - previousPoint->decathlonScore) *
                            ((previousPoint->normalizedValue - normalizedResult) /
                             (previousPoint->normalizedValue - currentPoint->normalizedValue));
                        break;
                    }
                }
            }
        }
    }
    if(decathlonScore < 0) {
        decathlonScore = 0;
    } else if(decathlonScore > 1000) {
        decathlonScore = 1000;
    }
    return decathlonScore;
}

int lbl_802BF860 = MGDATA_COLOR_GOLD;
int lbl_802BF864 = 0xFFFFFFFF;
int lbl_802BF868 = MGDATA_COLOR_GREEN;
