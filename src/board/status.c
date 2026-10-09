// Builds player status panels and the single-player counter of spaces remaining to the goal.
#define _MATH_H
#include "dolphin/math.h"

#include "game/board/status.h"
#include "game/board/player.h"
#include "game/board/main.h"
#include "game/board/pause.h"

#include "game/disp.h"
#include "game/flag.h"
#include "game/process.h"

#include <string.h>

extern int mbCapColorGet(int capsuleNo);
extern int mbSingleMgUnlockNumGet(void);
extern int mbSingleStepGet(void);
extern int mbMasuNumGet(void);
extern float mbSinDeg(float angle);
extern int mbLanguageGet(void);

#define STATUS_SPRNO_BACK 0
#define STATUS_SPRNO_BORDER 1
#define STATUS_SPRNO_RANK 2
#define STATUS_SPRNO_COIN_NUM 3
#define STATUS_SPRNO_COIN 6
#define STATUS_SPRNO_STAR_NUM 7
#define STATUS_SPRNO_STAR 10
#define STATUS_SPRNO_KAO 11
#define STATUS_SPRNO_COM 15
#define STATUS_SPRNO_CAPSULE 18
#define STATUS_SPR_MAX 23

typedef struct statusWork_s {
    unsigned offF : 1; // The panel sprites were created and must be released on close.
    unsigned dispF : 1; // Target visibility; a slide may still be bringing the panel on or off
                        // screen.
    unsigned moveF : 1; // The panel is still moving between positions.
    unsigned capsuleDispF : 1; // Capsule icons are allowed to be shown.
    unsigned layoutNo : 2; // Top or bottom capsule and computer-icon arrangement.
    unsigned layoutBackup : 2; // Saved layout slot; currently retained in the work record.
    s8 capsuleFocus; // Capsule slot enlarged by the pulse animation, or -1 for none.
    s16 capsuleTime; // Frame count used by the focused capsule pulse.
    s8 no; // Panel slot used to choose its screen position.
    s8 ownerIndex; // Individual player index or team index, retained from initialization.
    HUSPR_GROUPID gid; // Sprite group containing the panel and its icons.
    HuVec2f posBegin; // Starting screen position for a slide, in pixels.
    HuVec2f posEnd; // Destination screen position for a slide, in pixels.
    int moveType; // Interpolation curve selected by STATUS_MOVE_*.
    int time; // Current frame in the panel slide.
    int timeMax; // Total frames in the panel slide.
} STATUSWORK;

typedef struct statusMasuWork_s {
    unsigned offF : 1; // The counter sprite group has been created.
    unsigned dispF : 1; // The counter is enabled for display.
    unsigned : 6;
    u8 reservedPad01; // Unused alignment byte.
    s16 masuNoPrev; // Previous displayed number of spaces remaining to the single-player goal.
    s16 masuNo; // Current spaces remaining to the goal; invalid route lengths display zero.
    HUSPR_GROUPID gid; // Sprite group containing the counter.
    u8 initializationBytes[16]; // Cleared at startup; not accessed by the counter display code.
    int time; // Remaining frames in the counter pop animation.
    int timeMax; // Duration of the counter pop animation.
    u8 initializationTailBytes[8]; // Cleared at startup; not accessed by the counter display code.
} STATUSMASUWORK;

typedef struct statusDisp_s {
    unsigned dispF : 1; // Saved panel visibility for one player.
    int no; // Saved panel slot for one player.
} STATUSDISP;

static STATUSWORK statusWork[GW_PLAYER_MAX];
static STATUSMASUWORK statusMasuWork;
static STATUSDISP statusDispBackup[GW_PLAYER_MAX];

static HUPROCESS *statusProc;
static u8 statusForceFlag; // One bit per player whose panel had target visibility enabled at pause
                          // entry.

static HuVec2f statusPosOn[GW_PLAYER_MAX] = {
    { 114, 80 },
    { HU_DISP_WIDTH-114, 80 },
    { 114, HU_DISP_HEIGHT-80 },
    { HU_DISP_WIDTH-114, HU_DISP_HEIGHT-80 },
};

static HuVec2f statusPosOff[GW_PLAYER_MAX] = {
    { -98, 80 },
    { HU_DISP_WIDTH+98, 80 },
    { -98, HU_DISP_HEIGHT-80 },
    { HU_DISP_WIDTH+98, HU_DISP_HEIGHT-80 },
};

static HuVec2f statusPosOnTeam[GW_PLAYER_MAX/2] = {
    { 140, 80 },
    { HU_DISP_WIDTH-140, 80 },
};

static HuVec2f statusPosOffTeam[GW_PLAYER_MAX/2] = {
    { -124, 80 },
    { HU_DISP_WIDTH+124, 80 },
};

static GXColor statusColTbl[STATUS_COLOR_MAX] = {
    { 128, 128, 128, 0 },
    { 26, 132, 255, 0 },
    { 255, 26, 45, 0 },
    { 10, 180, 60, 0 },
    { 255, 64, 255, 0 },
};

static GXColor statusBorderColTbl[STATUS_COLOR_MAX] = {
    { 88, 88, 88, 0 },
    { 0, 0, 255, 0 },
    { 255, 0, 0, 0 },
    { 0, 255, 0, 0 },
    { 255, 0, 255, 0 },
};

static GXColor statusDefaultColTbl[GW_PLAYER_MAX] = {
    { 192, 65, 76, 0 },
    { 244, 113, 123, 0 },
    { 95, 100, 16, 0 },
    { 105, 164, 234, 0 },
};

static const int rainbowColorTbl[6] = {
    STATUS_COLOR_GREEN,
    STATUS_COLOR_BLUE,
    STATUS_COLOR_GREEN,
    STATUS_COLOR_RED,
    STATUS_COLOR_GREEN,
    STATUS_COLOR_GRAY,
};

static HuVec2f statusSprPosTbl[STATUS_SPR_MAX] = {
    { 0, 0 },
    { 0, 0 },
    { -68, 0 },
    { 80, 14 },
    { 64, 14 },
    { 48, 14 },
    { 26, 14 },
    { 80, -12 },
    { 64, -12 },
    { 48, -12 },
    { 26, -12 },
    { -16, 0 }
};

static HuVec2f statusCapsuleOfsTbl[2] = {
    { 24, 34 },
    { 24, -34 }
};

static HuVec2f statusComOfsTbl[2] = {
    { -16, -32 },
    { -16, 32 }
};

static HuVec2f statusTeamSprPosTbl[STATUS_SPR_MAX] = {
    { 0, 0 },
    { 0, 0 },
    { -100, 0 },
    { 112, 14 },
    { 96, 14 },
    { 80, 14 },
    { 58, 14 },
    { 112, -12 },
    { 96, -12 },
    { 80, -12 },
    { 58, -12 },
    { -48, 0 },
    { 16, 0 }
};

static HuVec2f statusTeamCapsuleOfsTbl[2] = {
    { -24, 34 },
    { -24, 34 }
};

static HuVec2f statusTeamComOfsTbl[2] = {
    { -48, -32 },
    { -48, -32 }
};

static HuVec2f statusCircuitCapsuleOfsTbl[3] = {
    { 38, 12 },
    { 54, -12 },
    { 70, 12 }
};

static void StatusMain(void);
static void StatusKill(void);
static BOOL StatusUpdate(void);
static BOOL StatusTeamUpdate(void);
static void StatusPosUpdate(STATUSWORK *status);

static void StatusPauseHook(BOOL dispF);

static void StatusSprCreate(int statusNo, STATUSWORK *status);
static void StatusTeamSprCreate(int statusNo, STATUSWORK *status);
static void StatusMasuSprCreate(STATUSMASUWORK *status);

// Board setup calls this to create the status panels for the current mode and start updates.
void mbStatusInit(void)
{
    int i;
    memset(&statusWork[0], 0, sizeof(statusWork));
    memset(&statusMasuWork, 0, sizeof(statusMasuWork));
    if(!GWTeamFGet()) {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            StatusSprCreate(i, &statusWork[i]);
            statusWork[i].offF = TRUE;
            statusWork[i].moveF = FALSE;
            statusWork[i].dispF = FALSE;
            statusWork[i].capsuleDispF = TRUE;
            statusWork[i].no = i;
            statusWork[i].capsuleFocus = -1;
            statusWork[i].capsuleTime = 0;
            statusWork[i].ownerIndex = i;
            if(i < GW_PLAYER_MAX/2) {
                statusWork[i].layoutNo = 0;
            } else {
                statusWork[i].layoutNo = 1;
            }
            statusWork[i].layoutBackup = statusWork[i].layoutNo;
            mbStatusColorSet(i, GwPlayer[i].statusColor);
            HuSprGrpDrawNoSet(statusWork[i].gid, 32);
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX/2; i++) {
            GXColor *colorP;
            StatusTeamSprCreate(i, &statusWork[i]);
            statusWork[i].offF = TRUE;
            statusWork[i].moveF = FALSE;
            statusWork[i].dispF = FALSE;
            statusWork[i].capsuleDispF = TRUE;
            statusWork[i].capsuleFocus = -1;
            statusWork[i].capsuleTime = 0;
            statusWork[i].no = i;
            statusWork[i].ownerIndex = i;
            statusWork[i].layoutNo = 0;
            statusWork[i].layoutBackup = statusWork[i].layoutNo;
            colorP = (i == 0) ? &statusDefaultColTbl[0] : &statusDefaultColTbl[2];
            // Initial team panels use the second palette color for both background and borders.
            colorP++;
            HuSprColorSet(statusWork[i].gid, STATUS_SPRNO_BACK, colorP->r, colorP->g, colorP->b);
            HuSprColorSet(statusWork[i].gid, STATUS_SPRNO_BORDER, colorP->r, colorP->g, colorP->b);
            HuSprColorSet(statusWork[i].gid, 13, colorP->r, colorP->g, colorP->b);
            HuSprColorSet(statusWork[i].gid, 14, colorP->r, colorP->g, colorP->b);
            HuSprGrpDrawNoSet(statusWork[i].gid, 32);
        }
    }
    if(GWPartyGet() == FALSE) {
        statusMasuWork.offF = TRUE;
        StatusMasuSprCreate(&statusMasuWork);
        HuSprGrpPosSet(statusMasuWork.gid, 472.0f, 72.0f);
        HuSprGrpDrawNoSet(statusMasuWork.gid, 32);
    }
    statusProc = HuPrcChildCreate(StatusMain, 8193, 16384, 0, mbMainProc);
    HuPrcDestructorSet2(statusProc, StatusKill);
    mbStatusDispForceSetAll(FALSE);
    mbStatusMasuDispSet(FALSE);
}

// Board teardown and reset call this to release active panel and space-counter sprite groups.
void mbStatusClose(void)
{
    STATUSWORK *status;
    int i;
    for(status=&statusWork[0], i=0; i<GW_PLAYER_MAX; i++, status++) {
        if(status->offF && status->gid != HUSPR_GRP_NONE) {
            HuSprGrpKill(status->gid);
            status->gid = HUSPR_GRP_NONE;
        }
    }
    if(statusMasuWork.offF) {
        HuSprGrpKill(statusMasuWork.gid);
        statusMasuWork.offF = FALSE;
    }
}

// Board transitions call this to close the current UI and recreate it for the active mode.
void mbStatusReset(void)
{
    mbStatusClose();
    HuPrcKill(statusProc);
    mbStatusInit();
}

// mbStatusInit starts this process to register the pause callback and refresh status once per
// frame.
static void StatusMain(void)
{
    STATUSWORK *status = &statusWork[0];
    mbPauseHookPush(StatusPauseHook);
    while(1) {
        if(!GWTeamFGet()) {
            if(StatusUpdate()) {
                break;
            }
        } else {
            if(StatusTeamUpdate()) {
                break;
            }
        }
        HuPrcVSleep();
    }
    HuPrcEnd();
}

// Clears the process handle when the status update process is destroyed.
static void StatusKill(void)
{
    statusProc = NULL;
}

// StatusMain refreshes individual panels each frame; single-player shows pending minigame unlocks
// instead of stars and also updates the spaces-to-goal counter.
static BOOL StatusUpdate(void)
{
    int i, j;
    float scale;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        STATUSWORK *status = &statusWork[i];
        int capsuleNo;
        if(GWPartyGet() == FALSE && i > 0) {
            for(j=0; j<STATUS_SPR_MAX; j++) {
                HuSprDispOff(status->gid, j);
            }
            statusWork[i].moveF = FALSE;
            continue;
        }
        GwPlayer[i].rank = mbPlayerRankGet(i);
        HuSprBankSet(status->gid, STATUS_SPRNO_RANK, GwPlayer[i].rank);
        mbSprNumSet(status->gid, STATUS_SPRNO_COIN_NUM, mbPlayerCoinGet(i));
        if(GWPartyGet() != FALSE) {
            mbSprNumSet(status->gid, STATUS_SPRNO_STAR_NUM, mbPlayerStarGet(i));
        } else {
            mbSprNumSet(status->gid, STATUS_SPRNO_STAR_NUM, mbSingleMgUnlockNumGet());
        }
        for(j=0; j<3; j++) {
            capsuleNo = mbPlayerCapsuleGet(i, j);
            if(capsuleNo != -1 && status->capsuleDispF) {
                HuSprBankSet(status->gid, STATUS_SPRNO_CAPSULE+j, mbCapColorGet(capsuleNo));
                HuSprPosSet(status->gid, STATUS_SPRNO_CAPSULE + j,
                            statusCapsuleOfsTbl[status->layoutNo].x + (j * 24),
                            statusCapsuleOfsTbl[status->layoutNo].y);
                HuSprDispOn(status->gid, STATUS_SPRNO_CAPSULE+j);
                if(status->capsuleFocus == j) {
                    scale = 1+fabs(HuCos(status->capsuleTime++ * 8.0f));
                    HuSprScaleSet(status->gid, STATUS_SPRNO_CAPSULE+j, scale, scale);
                } else {
                    // Each visible unfocused slot resets the pulse frame shared by this panel.
                    status->capsuleTime = 0;
                    HuSprScaleSet(status->gid, STATUS_SPRNO_CAPSULE+j, 1, 1);
                }
            } else {
                HuSprDispOff(status->gid, STATUS_SPRNO_CAPSULE+j);
            }
        }
        if(GwPlayer[i].comF) {
            HuSprDispOn(status->gid, STATUS_SPRNO_COM);
            HuSprPosSet(status->gid, STATUS_SPRNO_COM, statusComOfsTbl[status->layoutNo].x,
                        statusComOfsTbl[status->layoutNo].y);
        } else {
            HuSprDispOff(status->gid, STATUS_SPRNO_COM);
        }
        StatusPosUpdate(status);
    }
    if(statusMasuWork.offF) {
        if(statusMasuWork.dispF) {
            int onesDigit;
            HuSprGrpPosSet(statusMasuWork.gid, 472.0f, 72.0f);
            if(mbSingleStepGet() <= 1) {
                HuSprDispOff(statusMasuWork.gid, 1);
                HuSprDispOff(statusMasuWork.gid, 2);
                HuSprDispOn(statusMasuWork.gid, 3);
                HuSprDispOn(statusMasuWork.gid, 4);
            } else {
                HuSprDispOn(statusMasuWork.gid, 1);
                HuSprDispOn(statusMasuWork.gid, 2);
                HuSprDispOff(statusMasuWork.gid, 3);
                HuSprDispOff(statusMasuWork.gid, 4);
            }
            HuSprDispOff(statusMasuWork.gid, 5);
            i = mbSingleStepGet();
            if(i < 0) {
                i = 0;
            }
            if(i >= mbMasuNumGet()) {
                i = 0;
            }
            onesDigit = statusMasuWork.masuNo = i;
            j = onesDigit / 10;
            if(j > 0) {
                HuSprDispOn(statusMasuWork.gid, 5);
                HuSprBankSet(statusMasuWork.gid, 5, j);
            }
            onesDigit -= j * 10;
            if(i > 0 && i <= 5) {
                // The final five spaces use alternate digit images in the same animation.
                onesDigit += 9;
            }
            HuSprBankSet(statusMasuWork.gid, 6, onesDigit);
            if(statusMasuWork.masuNo != statusMasuWork.masuNoPrev) {
                statusMasuWork.masuNoPrev = statusMasuWork.masuNo;
                statusMasuWork.time = statusMasuWork.timeMax = 8;
            }
            if(statusMasuWork.time != 0) {
                statusMasuWork.time--;
                scale = (float)statusMasuWork.time / statusMasuWork.timeMax;
                scale = 1.0f + (0.5f * mbSinDeg(90.0f * scale));
                HuSprScaleSet(statusMasuWork.gid, 5, scale, scale);
                HuSprScaleSet(statusMasuWork.gid, 6, scale, scale);
            }
        } else {
            HuSprGrpPosSet(statusMasuWork.gid, 700.0f, 72.0f);
        }
    }
    return FALSE;
}

// StatusMain calls this each frame in team mode to refresh ranks, totals, capsules,
// computer icons, and panel positions.
static BOOL StatusTeamUpdate(void)
{
    int i, j;
    for(i=0; i<GW_PLAYER_MAX/2; i++) {
        STATUSWORK *status = &statusWork[i];
        int teamTotal;
        int capsuleNo;
        HuSprBankSet(status->gid, STATUS_SPRNO_RANK, mbPlayerTeamRankGet(i)+(i*2));
        teamTotal = mbPlayerTeamCoinGet(i);
        mbSprNumSet(status->gid, STATUS_SPRNO_COIN_NUM, teamTotal);
        teamTotal = mbPlayerGrpStarGet(i);
        mbSprNumSet(status->gid, STATUS_SPRNO_STAR_NUM, teamTotal);
        for(j=0; j<5; j++) {
            capsuleNo = mbPlayerTeamCapsuleGet(i, j);
            if(capsuleNo != -1 && status->capsuleDispF) {
                HuSprBankSet(status->gid, STATUS_SPRNO_CAPSULE+j, mbCapColorGet(capsuleNo));
                HuSprPosSet(status->gid, STATUS_SPRNO_CAPSULE + j,
                            statusTeamCapsuleOfsTbl[status->layoutNo].x + (j * 24),
                            statusTeamCapsuleOfsTbl[status->layoutNo].y);
                HuSprDispOn(status->gid, STATUS_SPRNO_CAPSULE+j);
            } else {
                HuSprDispOff(status->gid, STATUS_SPRNO_CAPSULE+j);
            }
        }
        for(j=0; j<2; j++) {
            if(GwPlayer[mbPlayerTeamFindPlayer(i, j)].comF) {
                HuSprDispOn(status->gid, STATUS_SPRNO_COM+j);
                HuSprPosSet(status->gid, STATUS_SPRNO_COM + j,
                            statusTeamComOfsTbl[status->layoutNo].x + (j * 64),
                            statusTeamComOfsTbl[status->layoutNo].y);
            } else {
                HuSprDispOff(status->gid, STATUS_SPRNO_COM+j);
            }
        }
        StatusPosUpdate(status);
    }
    return FALSE;
}

// StatusUpdate and StatusTeamUpdate call this to advance a panel slide and place its sprite group.
static void StatusPosUpdate(STATUSWORK *status)
{
    float weight;
    float time;
    HuVec2f pos;
    // Clear the movement flag one update after applying the final slide position.
    if(status->time >= status->timeMax) {
        status->moveF = FALSE;
        return;
    }
    time = (float)(++status->time)/status->timeMax;
    switch(status->moveType) {
        case STATUS_MOVE_LINEAR:
            weight = time;
            break;

        case STATUS_MOVE_SIN:
            weight = HuSin(time*90.0f);
            break;

        case STATUS_MOVE_REVCOS:
            weight = 1-HuCos(time*90.0f);
            break;
    }
    pos.x = status->posBegin.x+(weight*(status->posEnd.x-status->posBegin.x));
    pos.y = status->posBegin.y+(weight*(status->posEnd.y-status->posBegin.y));
    HuSprGrpPosSet(status->gid, pos.x, pos.y);
}

static STATUSWORK *StatusPlayerGet(int playerNo);

// Starts a 16-frame reverse-cosine slide between the supplied positions.
void mbStatusMoveTo(int playerNo, HuVecF *posBegin, HuVecF *posEnd)
{
    mbStatusMoveSet(playerNo, posBegin, posEnd, STATUS_MOVE_REVCOS, 16);
}

// The movement setters call this to store endpoints and timing for the per-frame slide update.
static inline void StatusMoveSet(STATUSWORK *status, HuVecF *posBegin, HuVecF *posEnd, int type,
                                 int time)
{
    status->posBegin.x = posBegin->x;
    status->posBegin.y = posBegin->y;
    status->posEnd.x = posEnd->x;
    status->posEnd.y = posEnd->y;
    status->moveType = type;
    status->time = 0;
    status->timeMax = time;
    status->moveF = TRUE;
}

// Starts a player panel slide; a null start position means its current position.
void mbStatusMoveSet(int playerNo, HuVecF *posBegin, HuVecF *posEnd, int type, int time)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    if(posBegin == NULL) {
        HuVecF pos;
        mbStatusPosGet(playerNo, &pos);
        StatusMoveSet(status, &pos, posEnd, type, time);
    } else {
        StatusMoveSet(status, posBegin, posEnd, type, time);
    }
}

// Starts a slide by panel slot rather than by player identity.
void mbStatusNoMoveSet(int statusNo, HuVecF *posBegin, HuVecF *posEnd, int type, int time)
{
    STATUSWORK *status = &statusWork[statusNo];
    if(posBegin == NULL) {
        HuVecF pos;
        mbStatusNoPosGet(statusNo, &pos);
        StatusMoveSet(status, &pos, posEnd, type, time);
    } else {
        StatusMoveSet(status, posBegin, posEnd, type, time);
    }
}

// Returns the shown position for a panel slot, using the current team layout.
void mbStatusPosOnGet(int statusNo, HuVecF *pos)
{
    if(!GWTeamFGet()) {
        pos->x = statusPosOn[statusNo].x;
        pos->y = statusPosOn[statusNo].y;
    } else {
        pos->x = statusPosOnTeam[statusNo].x;
        pos->y = statusPosOnTeam[statusNo].y;
    }
}

// Returns the hidden off-screen position for a panel slot, using the current team layout.
void mbStatusPosOffGet(int statusNo, HuVecF *pos)
{
    if(!GWTeamFGet()) {
        pos->x = statusPosOff[statusNo].x;
        pos->y = statusPosOff[statusNo].y;
    } else {
        pos->x = statusPosOffTeam[statusNo].x;
        pos->y = statusPosOffTeam[statusNo].y;
    }
}

// Reports whether the selected player's status panel has finished moving.
BOOL mbStatusMoveCheck(int playerNo)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    return status->moveF == FALSE;
}

// Reports whether every active status panel has finished moving.
BOOL mbStatusOffCheckAll(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(statusWork[i].offF && statusWork[i].moveF) {
            return FALSE;
        }
    }
    return TRUE;
}

// Changes target visibility and slides the resolved panel. Showing selects its slot;
// the layout write uses the player index rather than the resolved team slot.
static void StatusDispSet(int playerNo, int statusNo, BOOL dispF)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    HuVec2f posBegin;
    HuVec2f posEnd;

    if(GWPartyGet() == FALSE && playerNo > 0) {
        dispF = FALSE;
    }
    if(!dispF) {
        if(!status->dispF) {
            return;
        }
        // Hiding reverses the saved endpoints and starts at the previous destination,
        // even when the panel is still partway through its slide.
        posBegin = status->posEnd;
        posEnd = status->posBegin;
        mbStatusMoveSet(playerNo, (HuVecF *)&posBegin, (HuVecF *)&posEnd, STATUS_MOVE_REVCOS, 16);
        status->dispF = FALSE;
        status->moveF = TRUE;
    } else {
        if(status->dispF) {
            return;
        }
        mbStatusPosOffGet(statusNo, (HuVecF *)&posBegin);
        mbStatusPosOnGet(statusNo, (HuVecF *)&posEnd);
        mbStatusMoveSet(playerNo, (HuVecF *)&posBegin, (HuVecF *)&posEnd, STATUS_MOVE_SIN, 16);
        status->dispF = TRUE;
        status->moveF = TRUE;
    }
    if(dispF) {
        status->no = statusNo;
        if(status->no < 2) {
            mbStatusLayoutSet(playerNo, STATUS_LAYOUT_TOP);
        } else {
            mbStatusLayoutSet(playerNo, STATUS_LAYOUT_BOTTOM);
        }
    }
}

// Requests a player's panel visibility, resolving team players to their shared slot.
// Single-player mode forces players 1-3 hidden regardless of the request.
void mbStatusDispSet(int playerNo, BOOL dispF)
{
    if(GWPartyGet() == FALSE && playerNo > 0) {
        dispF = FALSE;
    }
    if(!GWTeamFGet()) {
        StatusDispSet(playerNo, playerNo, dispF);
    } else {
        StatusDispSet(playerNo, mbPlayerGrpGet(playerNo), dispF);
    }
}

// Requests slot-zero display for focused board events. An already-visible panel
// is not relocated; hiding reverses its stored slide endpoints.
void mbStatusDispFocusSet(int playerNo, BOOL dispF)
{
    StatusDispSet(playerNo, 0, dispF);
}

// Saves each player's panel visibility and panel slot before a temporary UI sequence.
void mbStatusDispBackup(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        statusDispBackup[i].dispF = mbStatusDispGet(i);
        statusDispBackup[i].no = mbStatusNoGet(i);
    }
}

// Board scroll exit restores saved visibility and positions immediately; existing slide timing
// remains in the work record.
void mbStatusDispRestoreForce(void)
{
    int i;
    STATUSWORK *status;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        status = StatusPlayerGet(i);
        status->dispF = statusDispBackup[i].dispF;
        status->no = statusDispBackup[i].no;
        if(status->dispF) {
            mbStatusPosOffGet(status->no, (HuVecF *)&status->posBegin);
            mbStatusPosOnGet(status->no, (HuVecF *)&status->posEnd);
        } else {
            mbStatusPosOnGet(status->no, (HuVecF *)&status->posBegin);
            mbStatusPosOffGet(status->no, (HuVecF *)&status->posEnd);
        }
        HuSprGrpPosSet(status->gid, status->posEnd.x, status->posEnd.y);
    }
}

// Restores saved visibility through the normal panel slide behavior.
void mbStatusDispRestore(void)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        StatusDispSet(i, statusDispBackup[i].no, statusDispBackup[i].dispF);
    }
}

// Returns the requested visibility of a player's panel, including while it slides into position.
BOOL mbStatusDispGet(int playerNo)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    return status->dispF;
}

// Returns the panel slot currently assigned to the player's status display.
int mbStatusNoGet(int playerNo)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    return status->no;
}

// Applies a visibility request to every player; single-player mode keeps players 1-3 hidden.
void mbStatusDispSetAll(BOOL dispF)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        mbStatusDispSet(i, dispF);
    }
}

// Board setup and events place a player's panel at a visibility endpoint and stop
// its slide; single-player mode forces players 1-3 to the hidden endpoint.
void mbStatusDispForceSet(int playerNo, BOOL dispF)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    if(GWPartyGet() == FALSE && playerNo > 0) {
        dispF = FALSE;
    }
    status->dispF = dispF;
    if(dispF) {
        mbStatusPosOffGet(status->no, (HuVecF *)&status->posBegin);
        mbStatusPosOnGet(status->no, (HuVecF *)&status->posEnd);
    } else {
        mbStatusPosOnGet(status->no, (HuVecF *)&status->posBegin);
        mbStatusPosOffGet(status->no, (HuVecF *)&status->posEnd);
    }
    HuSprGrpPosSet(status->gid, status->posEnd.x, status->posEnd.y);
    status->time = status->timeMax = 0;
    // Placement uses the saved slot above; the next visibility change uses the normal player slot.
    if(!GWTeamFGet()) {
        status->no = playerNo;
    } else {
        status->no = mbPlayerGrpGet(playerNo);
    }
    if(status->no < 2) {
        mbStatusLayoutSet(playerNo, STATUS_LAYOUT_TOP);
    } else {
        mbStatusLayoutSet(playerNo, STATUS_LAYOUT_BOTTOM);
    }
    status->moveF = FALSE;
}

// Places every player's panel at its requested endpoint; single-player mode keeps players 1-3
// hidden.
void mbStatusDispForceSetAll(BOOL dispF)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        mbStatusDispForceSet(i, dispF);
    }
}

// Board setup and scroll events request a counter position. Hiding does not clear dispF, so an
// enabled counter returns on the next StatusUpdate; showing during pause leaves dispF unchanged.
void mbStatusMasuDispSet(BOOL dispF)
{
    if(statusMasuWork.offF) {
        if(dispF) {
            if(!mbPauseProcCheck()) {
                statusMasuWork.dispF = TRUE;
            }
            HuSprGrpPosSet(statusMasuWork.gid, 472.0f, 72.0f);
        } else {
            HuSprGrpPosSet(statusMasuWork.gid, 700.0f, 72.0f);
        }
    }
}

// Enables or suppresses capsule icons for one status panel.
void mbStatusCapsuleDispSet(int statusNo, BOOL dispF)
{
    STATUSWORK *status = &statusWork[statusNo];
    if(dispF) {
        status->capsuleDispF = TRUE;
    } else {
        status->capsuleDispF = FALSE;
    }
}

// Stores the panel-slot layout index. Individual panels use top/bottom offsets;
// team panels use identical offsets for both layouts.
void mbStatusLayoutSet(int statusNo, int layoutNo)
{
    STATUSWORK *status = &statusWork[statusNo];
    status->layoutNo = layoutNo;
}

// Selects the capsule slot that receives the individual-panel pulse animation.
void mbStatusCapsuleFocusSet(int statusNo, int index)
{
    STATUSWORK *status = &statusWork[statusNo];
    status->capsuleFocus = index;
}

// Resolves a player to an individual or shared team status-panel work record.
static STATUSWORK *StatusPlayerGet(int playerNo)
{
    if(!GWTeamFGet()) {
        return &statusWork[playerNo];
    } else {
        return &statusWork[mbPlayerGrpGet(playerNo)];
    }
}

// Copies the current sprite-group position for a player's resolved panel.
void mbStatusPosGet(int playerNo, HuVecF *pos)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    HUSPR_GROUP *spriteGroup = &HuSprGrpData[status->gid];
    pos->x = spriteGroup->pos.x;
    pos->y = spriteGroup->pos.y;
    pos->z = 0;
}

// Copies the current sprite-group position for a panel slot.
void mbStatusNoPosGet(int statusNo, HuVecF *pos)
{
    STATUSWORK *status = &statusWork[statusNo];
    HUSPR_GROUP *spriteGroup = &HuSprGrpData[status->gid];
    pos->x = spriteGroup->pos.x;
    pos->y = spriteGroup->pos.y;
    pos->z = 0;
}

// Selects the face image bank shown by a status panel.
void mbStatusKaoNoSet(int statusNo, int kaoNo)
{
    STATUSWORK *status = &statusWork[statusNo];
    HuSprBankSet(status->gid, STATUS_SPRNO_KAO, kaoNo);
}

#define STATUS_SPR_INITNUM 11

static int statusKaoFileTbl[GW_CHARA_MAX] = {
    DATANUM(DATA_board, 54), DATANUM(DATA_board, 55),
    DATANUM(DATA_board, 56), DATANUM(DATA_board, 57),
    DATANUM(DATA_board, 58), DATANUM(DATA_board, 59),
    DATANUM(DATA_board, 60), DATANUM(DATA_board, 61),
    DATANUM(DATA_board, 62), DATANUM(DATA_board, 63),
    DATANUM(DATA_board, 64), DATANUM(DATA_board, 65),
    DATANUM(DATA_board, 66), DATANUM(DATA_board, 67),
};

// mbStatusInit calls this in individual mode to build a panel with its face, totals, and icons.
static void StatusSprCreate(int statusNo, STATUSWORK *status)
{
    static int fileTbl[STATUS_SPR_INITNUM] = {
        DATANUM(DATA_board, 40), DATANUM(DATA_board, 41),
        DATANUM(DATA_board, 39), DATANUM(DATA_board, 35),
        DATANUM(DATA_board, 35), DATANUM(DATA_board, 35),
        DATANUM(DATA_board, 38), DATANUM(DATA_board, 35),
        DATANUM(DATA_board, 35), DATANUM(DATA_board, 35),
        DATANUM(DATA_board, 37),
    };
    static int fileTblSingle[STATUS_SPR_INITNUM] = {
        DATANUM(DATA_board, 40), DATANUM(DATA_board, 41),
        DATANUM(DATA_board, 39), DATANUM(DATA_board, 35),
        DATANUM(DATA_board, 35), DATANUM(DATA_board, 35),
        DATANUM(DATA_board, 38), DATANUM(DATA_board, 35),
        DATANUM(DATA_board, 35), DATANUM(DATA_board, 35),
        DATANUM(DATA_board, 48),
    };
    static s16 prioTbl[STATUS_SPR_INITNUM] = {
        1520, 1510, 1500, 1500, 1500, 1500,
        1500, 1500, 1500, 1500, 1500,
    };
    int characterNo = GwPlayerConf[statusNo].charNo;
    int i;
    HUSPRID sprId;
    float posX, posY;
    status->gid = HuSprGrpCreate(STATUS_SPR_MAX);
    for(i=0; i<STATUS_SPR_INITNUM; i++) {
        int dataNum;
        s16 prio;
        if(GWPartyGet() != FALSE) {
            dataNum = fileTbl[i];
        } else {
            dataNum = fileTblSingle[i];
        }
        prio = prioTbl[i];
        mbSprCreate(mbBoardDataNumGet(dataNum), prio, NULL, &sprId);
        HuSprGrpMemberSet(status->gid, i, sprId);
        HuSprAttrSet(status->gid, i, HUSPR_ATTR_NOANIM);
        HuSprAttrSet(status->gid, i, HUSPR_ATTR_LINEAR);
        HuSprPosSet(status->gid, i, statusSprPosTbl[i].x, statusSprPosTbl[i].y);
    }
    mbSprCreate(mbBoardDataNumGet(statusKaoFileTbl[characterNo]), 1500, NULL, &sprId);
    HuSprGrpMemberSet(status->gid, STATUS_SPRNO_KAO, sprId);
    HuSprAttrSet(status->gid, STATUS_SPRNO_KAO, HUSPR_ATTR_LINEAR);
    posX = statusSprPosTbl[STATUS_SPRNO_KAO].x;
    posY = statusSprPosTbl[STATUS_SPRNO_KAO].y;
    HuSprPosSet(status->gid, STATUS_SPRNO_KAO, posX, posY);
    mbSprCreate(mbBoardDataNumGet(DATANUM(DATA_board, 53)), 1490, NULL, &sprId);
    HuSprGrpMemberSet(status->gid, 13, sprId);
    HuSprAttrSet(status->gid, 13, HUSPR_ATTR_LINEAR);
    posX = statusSprPosTbl[STATUS_SPRNO_KAO].x;
    posY = statusSprPosTbl[STATUS_SPRNO_KAO].y;
    HuSprPosSet(status->gid, 13, posX, posY);
    mbSprCreate(mbBoardDataNumGet(DATANUM(DATA_board, 36)), 1490, NULL, &sprId);
    HuSprGrpMemberSet(status->gid, STATUS_SPRNO_COM, sprId);
    HuSprAttrSet(status->gid, STATUS_SPRNO_COM, HUSPR_ATTR_LINEAR);
    for(i=0; i<3; i++) {
        mbSprCreate(mbBoardDataNumGet(DATANUM(DATA_board, 46)), 1500, NULL, &sprId);
        HuSprGrpMemberSet(status->gid, STATUS_SPRNO_CAPSULE+i, sprId);
        HuSprAttrSet(status->gid, STATUS_SPRNO_CAPSULE+i, HUSPR_ATTR_NOANIM);
        HuSprAttrSet(status->gid, STATUS_SPRNO_CAPSULE+i, HUSPR_ATTR_LINEAR);
    }
    HuSprTPLvlSet(status->gid, STATUS_SPRNO_BACK, 0.7f);
    if(GWPartyGet() == FALSE) {
        i = STATUS_SPRNO_RANK;
        HuSprDispOff(status->gid, i);
        for(i=STATUS_SPRNO_STAR_NUM; i<=STATUS_SPRNO_STAR; i++) {
            HuSprPosSet(status->gid, i, statusSprPosTbl[i].x-24, statusSprPosTbl[i].y);
        }
        for(i=STATUS_SPRNO_COIN_NUM; i<=STATUS_SPRNO_COIN; i++) {
            HuSprPosSet(status->gid, i, statusSprPosTbl[i].x-24, statusSprPosTbl[i].y);
        }
        i = STATUS_SPRNO_KAO;
        HuSprPosSet(status->gid, i, statusSprPosTbl[i].x-24, statusSprPosTbl[i].y);
        HuSprPosSet(status->gid, 13, statusSprPosTbl[i].x-24, statusSprPosTbl[i].y);
    }
}

// mbStatusInit calls this in team mode to build one panel with both faces and five capsule slots.
static void StatusTeamSprCreate(int statusNo, STATUSWORK *status)
{
    static int fileTbl[STATUS_SPR_INITNUM] = {
        DATANUM(DATA_board, 42), DATANUM(DATA_board, 43),
        DATANUM(DATA_board, 45), DATANUM(DATA_board, 35),
        DATANUM(DATA_board, 35), DATANUM(DATA_board, 35),
        DATANUM(DATA_board, 38), DATANUM(DATA_board, 35),
        DATANUM(DATA_board, 35), DATANUM(DATA_board, 35),
        DATANUM(DATA_board, 37),
    };
    static s16 prioTbl[STATUS_SPR_INITNUM] = {
        1520, 1510, 1500, 1500, 1500, 1500,
        1500, 1500, 1500, 1500, 1500,
    };
    int i;
    HUSPRID sprId;
    int faceSlot;
    float posX, posY;
    status->gid = HuSprGrpCreate(STATUS_SPR_MAX);
    for(i=0; i<STATUS_SPR_INITNUM; i++) {
        int dataNum = fileTbl[i];
        s16 prio = prioTbl[i];
        mbSprCreate(mbBoardDataNumGet(dataNum), prio, NULL, &sprId);
        HuSprGrpMemberSet(status->gid, i, sprId);
        HuSprAttrSet(status->gid, i, HUSPR_ATTR_NOANIM);
        HuSprAttrSet(status->gid, i, HUSPR_ATTR_LINEAR);
        HuSprPosSet(status->gid, i, statusTeamSprPosTbl[i].x, statusTeamSprPosTbl[i].y);
    }
    mbSprCreate(mbBoardDataNumGet(DATANUM(DATA_board, 44)), 1510, NULL, &sprId);
    HuSprGrpMemberSet(status->gid, 17, sprId);
    HuSprAttrSet(status->gid, 17, HUSPR_ATTR_NOANIM);
    HuSprAttrSet(status->gid, 17, HUSPR_ATTR_LINEAR);
    HuSprPosSet(status->gid, 17, statusTeamSprPosTbl[2].x, statusTeamSprPosTbl[2].y);
    HuSprBankSet(status->gid, 17, statusNo);
    faceSlot = 0;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        if(statusNo == mbPlayerGrpGet(i)) {
            mbSprCreate(mbBoardDataNumGet(statusKaoFileTbl[GwPlayerConf[i].charNo]), 1500, NULL,
                        &sprId);
            HuSprGrpMemberSet(status->gid, STATUS_SPRNO_KAO+faceSlot, sprId);
            HuSprAttrSet(status->gid, STATUS_SPRNO_KAO+faceSlot, HUSPR_ATTR_LINEAR);
            posX = statusTeamSprPosTbl[STATUS_SPRNO_KAO+faceSlot].x;
            posY = statusTeamSprPosTbl[STATUS_SPRNO_KAO+faceSlot].y;
            HuSprPosSet(status->gid, STATUS_SPRNO_KAO+faceSlot, posX, posY);
            mbSprCreate(mbBoardDataNumGet(DATANUM(DATA_board, 53)), 1490, NULL, &sprId);
            HuSprGrpMemberSet(status->gid, 13+faceSlot, sprId);
            HuSprAttrSet(status->gid, 13+faceSlot, HUSPR_ATTR_LINEAR);
            HuSprPosSet(status->gid, 13+faceSlot, posX, posY);
            faceSlot++;
        }
    }
    for(i=0; i<2; i++) {
        mbSprCreate(mbBoardDataNumGet(DATANUM(DATA_board, 36)), 1490, NULL, &sprId);
        HuSprGrpMemberSet(status->gid, STATUS_SPRNO_COM+i, sprId);
        HuSprAttrSet(status->gid, STATUS_SPRNO_COM+i, HUSPR_ATTR_LINEAR);
    }
    for(i=0; i<5; i++) {
        mbSprCreate(mbBoardDataNumGet(DATANUM(DATA_board, 46)), 1500, NULL, &sprId);
        HuSprGrpMemberSet(status->gid, STATUS_SPRNO_CAPSULE+i, sprId);
        HuSprAttrSet(status->gid, STATUS_SPRNO_CAPSULE+i, HUSPR_ATTR_NOANIM);
        HuSprAttrSet(status->gid, STATUS_SPRNO_CAPSULE+i, HUSPR_ATTR_LINEAR);
    }
    HuSprTPLvlSet(status->gid, STATUS_SPRNO_BACK, 0.7f);
}

// mbStatusInit calls this for single-player mode to build the localized board-space counter.
static void StatusMasuSprCreate(STATUSMASUWORK *status)
{
    static int fileTbl[7] = {
        DATANUM(DATA_board, 49), DATANUM(DATA_board, 50),
        DATANUM(DATA_board, 50), DATANUM(DATA_board, 52),
        DATANUM(DATA_board, 52), DATANUM(DATA_board, 51),
        DATANUM(DATA_board, 51),
    };
    static HuVec2f ofsTbl[7] = {
        { 0, 0 }, { -48, 0 }, { 48, 0 }, { -48, 0 },
        { 48, 0 }, { -12, 0 }, { 12, 0 },
    };
    static s16 sprOfsTbl[6][7] = {
        { 0, -48, 48, -48, 48, -12, 12 },
        { 0, -28, -28, -28, -28, -64, -40 },
        { 0, -28, -28, -28, -28, -64, -40 },
        { 0, -28, -28, -28, -28, -64, -40 },
        { 0, -28, -28, -28, -28, -64, -40 },
        { 0, -28, -28, -28, -28, -64, -40 },
    };
    int i;
    HUSPRID sprId;
    int languageNo;
    status->gid = HuSprGrpCreate(7);
    languageNo = mbLanguageGet();
    for(i=0; i<7; i++) {
        int dataNum = fileTbl[i];
        s16 prio = 1520;
        mbSprCreate(mbBoardDataNumGet(dataNum), prio, NULL, &sprId);
        HuSprGrpMemberSet(status->gid, i, sprId);
        HuSprAttrSet(status->gid, i, HUSPR_ATTR_NOANIM);
        HuSprAttrSet(status->gid, i, HUSPR_ATTR_LINEAR);
        HuSprPosSet(status->gid, i, sprOfsTbl[languageNo][i], 0.0f);
    }
    HuSprBankSet(status->gid, 2, 1);
    HuSprBankSet(status->gid, 4, 1);
    HuSprTPLvlSet(status->gid, 0, 0.7f);
}

#undef STATUS_SPR_INITNUM

// Board display setup loads an animation into the model heap, creates its sprite in bank zero,
// and optionally receives the animation and sprite handles.
void mbSprCreate(int dataNum, int prio, ANIMDATA **animationOut, HUSPRID *spriteIdOut)
{
    HUSPRID sprId;
    void *animationData;
    ANIMDATA *animation;
    animationData = HuDataSelHeapReadNum(dataNum, HU_MEMNUM_OVL, HEAP_MODEL);
    animation = HuSprAnimRead(animationData);
    sprId = HuSprCreate(animation, prio, 0);
    if(animationOut) {
        *animationOut = animation;
    }
    if(spriteIdOut) {
        *spriteIdOut = sprId;
    }
}

// Status totals and board counters use three digit sprites; single digits move into the middle
// slot and bank ten replaces leading hundreds. Values should be 0 to 999; this does not clamp them.
void mbSprNumSet(HUSPR_GROUPID gid, s16 memberNo, int num)
{
    int hundredsDigit = num/100;
    int tensDigit = (num-(hundredsDigit*100))/10;
    int onesDigit = (num-((hundredsDigit*100)+(tensDigit*10)))%10;
    if(hundredsDigit == 0) {
        HuSprBankSet(gid, memberNo+2, 10);
    } else {
        HuSprBankSet(gid, memberNo+2, hundredsDigit);
    }
    if(tensDigit == 0 && hundredsDigit == 0) {
        HuSprBankSet(gid, memberNo+1, onesDigit);
        HuSprDispOff(gid, memberNo);
    } else {
        HuSprBankSet(gid, memberNo+1, tensDigit);
        HuSprBankSet(gid, memberNo, onesDigit);
        HuSprDispOn(gid, memberNo);
    }
}

// The pause process passes FALSE on entry to place target-visible panels at their
// stored slide starts, then TRUE on exit to place them at their stored destinations.
// These endpoints need not be off/on screen. An existing space counter is hidden
// on entry and positioned on screen on exit.
static void StatusPauseHook(BOOL dispF)
{
    int i;
    STATUSWORK *status;
    if(!dispF) {
        statusForceFlag = 0;
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(mbStatusDispGet(i)) {
                status = StatusPlayerGet(i);
                HuSprGrpPosSet(status->gid, status->posBegin.x, status->posBegin.y);
                statusForceFlag |= (1 << i);
            }
        }
        if(statusMasuWork.offF) {
            HuSprGrpPosSet(statusMasuWork.gid, 700.0f, 72.0f);
        }
    } else {
        for(i=0; i<GW_PLAYER_MAX; i++) {
            if(statusForceFlag & (1 << i)) {
                status = StatusPlayerGet(i);
                HuSprGrpPosSet(status->gid, status->posEnd.x, status->posEnd.y);
            }
        }
        if(statusMasuWork.offF) {
            if(!mbPauseProcCheck()) {
                statusMasuWork.dispF = TRUE;
            }
            HuSprGrpPosSet(statusMasuWork.gid, 472.0f, 72.0f);
        }
    }
}

// Minigame selection calls this each animation frame with progress from zero to one. The panel
// cycles through five blends; team play then maps red to the first team palette and all other
// colors to the second.
void mbStatusRainbowSet(int statusNo, float progress, int colorEnd)
{
    STATUSWORK *status = &statusWork[statusNo];
    int colorTime;
    float weight;
    float weightRev;
    int colorNo;
    int colorNext;
    GXColor colorBorder;
    GXColor color;

    colorTime = 5.0f*progress;
    if(colorTime < 5) {
        colorNo = rainbowColorTbl[colorTime];
        if(colorTime < 4) {
            colorNext = rainbowColorTbl[colorTime+1];
        } else {
            colorNext = colorEnd;
        }
        weight = fmod(5.0f*progress, 1.0f);
        weightRev = 1.0f-weight;
        color.r = (weightRev*statusColTbl[colorNo].r)+(weight*statusColTbl[colorNext].r);
        color.g = (weightRev*statusColTbl[colorNo].g)+(weight*statusColTbl[colorNext].g);
        color.b = (weightRev*statusColTbl[colorNo].b)+(weight*statusColTbl[colorNext].b);
        colorBorder.r = (weightRev * statusBorderColTbl[colorNo].r) +
                        (weight * statusBorderColTbl[colorNext].r);
        colorBorder.g = (weightRev * statusBorderColTbl[colorNo].g) +
                        (weight * statusBorderColTbl[colorNext].g);
        colorBorder.b = (weightRev * statusBorderColTbl[colorNo].b) +
                        (weight * statusBorderColTbl[colorNext].b);
    } else {
        if(!GWTeamFGet()) {
            color = statusColTbl[colorEnd];
            colorBorder = statusBorderColTbl[colorEnd];
        } else {
            if(colorEnd == STATUS_COLOR_RED) {
                colorEnd = 0;
            } else {
                colorEnd = 2;
            }
            color = statusDefaultColTbl[colorEnd];
            colorBorder = statusDefaultColTbl[colorEnd+1];
        }
    }
    HuSprColorSet(status->gid, STATUS_SPRNO_BACK, color.r, color.g, color.b);
    HuSprColorSet(status->gid, STATUS_SPRNO_BORDER, colorBorder.r, colorBorder.g, colorBorder.b);
    HuSprColorSet(status->gid, 13, colorBorder.r, colorBorder.g, colorBorder.b);
    if(GWTeamFGet()) {
        HuSprColorSet(status->gid, 14, colorBorder.r, colorBorder.g, colorBorder.b);
    }
}

// Board and minigame setup store a player's status color; individual mode also tints the panel,
// while team mode only updates the stored color.
void mbStatusColorSet(int playerNo, int color)
{
    STATUSWORK *status = StatusPlayerGet(playerNo);
    if(!GWTeamFGet()) {
        HuSprColorSet(status->gid, STATUS_SPRNO_BACK, statusColTbl[color].r, statusColTbl[color].g,
                      statusColTbl[color].b);
        HuSprColorSet(status->gid, STATUS_SPRNO_BORDER, statusBorderColTbl[color].r,
                      statusBorderColTbl[color].g, statusBorderColTbl[color].b);
        HuSprColorSet(status->gid, 13, statusBorderColTbl[color].r, statusBorderColTbl[color].g,
                      statusBorderColTbl[color].b);
    }
    GwPlayer[playerNo].statusColor = color;
}

// Returns the status color stored for a player.
int mbStatusColorGet(int playerNo)
{
    return GwPlayer[playerNo].statusColor;
}

// Applies the same status color to every player.
void mbStatusColorAllSet(int color)
{
    int i;
    for(i=0; i<GW_PLAYER_MAX; i++) {
        mbStatusColorSet(i, color);
    }
}
