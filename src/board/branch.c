/* Block math.h's extern inline sqrtf: its weak _half/_three statics would
 * prepend 16 bytes to .sdata2 that the original branch.o does not have. */
#define _MATH_H
#include "dolphin/math.h"

#include "game/board/branch.h"
#include "game/board/audio.h"
#include "game/board/guide.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/player.h"
#include "game/board/window.h"

#include "game/flag.h"
#include "game/gamework.h"
#include "game/pad.h"

#include "humath.h"
#include "game/board/tutorial.h"
#include "game/board/scroll.h"
#include "game/board/telop.h"
#include "game/board/math.h"

#define MB_BRANCH_ATTR_DEFAULT 0xA020

typedef struct BranchGuideWork_s {
    u8 killF : 1;
    u8 playerNo : 2;
    u8 linkNum : 2;
    u8 flag : 1;
    u8 unused : 2;
    s8 choice;
    s16 angle;
    s16 mdlTemp;
    MBMODELID modelId[4];
} BRANCHGUIDEWORK;

static OMOBJ *guideOMObj[4];
static MBBRANCHCOMSTARHOOK branchComStarHook;
static u32 branchMAttr;
static u16 branchAttr;

static void BranchGuideOMExec(OMOBJ *obj);
static int BranchComChoiceGet(int playerNo, int linkNum, s16 *linkTbl, BOOL debugF);

void mbBranchInit(void)
{
    int i;

    for (i = 0; i < 4; i++) {
        guideOMObj[i] = NULL;
    }
    branchComStarHook = NULL;
    mbBranchAttrInit();
    mbBranchMAttrInit();
}

static inline int MasuGetBranchLink(s16 masuId, s16 *linkTbl)
{
    int linkNum = 0;
    MASU *masuP = mbMasuGet(masuId);
    int i;

    for (i = 0; i < masuP->linkNum; i++) {
        MASU *masuLinkP = mbMasuGet(masuP->linkTbl[i]);
        if ((masuLinkP->flag & mbBranchAttrGet()) == 0
            && (masuLinkP->mAttr & mbBranchMAttrGet()) == 0) {
            linkTbl[linkNum++] = masuP->linkTbl[i];
        }
    }
    return linkNum;
}

static void BranchGuideCreate(int playerNo, int masuId, s16 *linkTbl, int linkNum, s16 choiceStart)
{
    BRANCHGUIDEWORK *work;
    OMOBJ *obj;
    HuVecF pos;
    HuVecF posLink;
    HuVecF dirLink;
    float angle;
    int i;

    if (guideOMObj[playerNo] || linkNum <= 1) {
        return;
    }
    guideOMObj[playerNo] = obj = fn_8003AB38(mbObjMan, 0x100, 0, 0, -1, BranchGuideOMExec);
    omSetStatBit(obj, (1 << 8));
    obj->scale.x = obj->scale.y = obj->scale.z = 3.0f;
    work = ((BRANCHGUIDEWORK *)(&(obj->work[0])));
    work->linkNum = linkNum;
    work->killF = FALSE;
    work->flag = FALSE;
    work->playerNo = playerNo;
    work->angle = 0;
    work->choice = choiceStart;
    if (!fn_8006A124(65550)) {
        if (GWPartyGet() != FALSE) {
            mbWinCreateHelp(0x00260007);
        }
    }
    mbMasuPosGet(masuId, &pos);
    pos.y += 350.0f;
    for (i = 0; i < linkNum; i++) {
        work->modelId[i] = mbObjCreate(mbBoardDataNumGet(0x50000), NULL, TRUE);
        mbObjLayerSet(work->modelId[i], 4);
        mbMasuPosGet(linkTbl[i], &posLink);
        PSVECSubtract(&posLink, &pos, &dirLink);
        angle = fmod((180.0*(atan2(dirLink.x, dirLink.z) / M_PI)), 360.0);
        mbObjRotSet(work->modelId[i], 0.0f, angle, 0.0f);
        mbObjLayerSet(work->modelId[i], 4);
        posLink.x = pos.x + (100.0 * sin(M_PI*angle/180.0));
        posLink.y = pos.y;
        posLink.z = pos.z + (100.0 * cos(M_PI*angle/180.0));
        mbObjPosSetV(work->modelId[i], &posLink);
        mbObjScaleSet(work->modelId[i], obj->scale.x, obj->scale.y, obj->scale.z);
    }
}

static void BranchGuideOMExec(OMOBJ *obj)
{
    BRANCHGUIDEWORK *work = ((BRANCHGUIDEWORK *)(&(obj->work[0])));

    if (work->killF || mbExitCheck()) {
        int i;

        for (i = 0; i < work->linkNum; i++) {
            mbObjKill(work->modelId[i]);
        }
        if (!fn_8006A124(65550)) {
            if (GWPartyGet() != FALSE) {
                mbWinTopKill();
            }
        }
        guideOMObj[work->playerNo] = NULL;
        omDelObjEx(fn_8000DD9C(), obj);
        return;
    }
    work->angle += 8;
    if (work->angle > 180) {
        work->angle -= 180;
    }
    obj->scale.x = work->angle;
    obj->scale.x = obj->scale.y = obj->scale.z =
        sin(M_PI*((float)work->angle)/180.0) + 3.0;
    mbObjScaleSet(work->modelId[work->choice], obj->scale.x, obj->scale.y, obj->scale.z);
}

static inline void BranchGuideChoiceSet(int playerNo, int choice)
{
    if (guideOMObj[playerNo]) {
        BRANCHGUIDEWORK *work = ((BRANCHGUIDEWORK *)(&(guideOMObj[playerNo]->work[0])));
        work->choice = choice;
    }
}

static inline void BranchGuideKill(int playerNo)
{
    if (guideOMObj[playerNo]) {
        BRANCHGUIDEWORK *work = ((BRANCHGUIDEWORK *)(&(guideOMObj[playerNo]->work[0])));
        work->killF = TRUE;
        HuPrcSleep(10);
    }
}

static BOOL ev_Branch(int playerNo, s16 *masuId, BOOL debugF)
{
    static s16 angleTbl[] = {
        135, 90, 45,
        180, 0, 0,
        225, 270, 315,
    };
    float anglePath[MASU_LINK_MAX * 2];
    s16 branchLinkTbl[MASU_LINK_MAX * 2];
    HuVecF dir;
    HuVecF dir2D;
    HuVecF pos;
    HuVecF pos2D;
    HuVecF posLink;
    HuVecF posLink2D;
    int i;
    int linkNum;
    int choice;
    int choiceTime;
    int padNo;
    s16 masuPlayer;
    int choiceAuto;

    choiceTime = -1;
    masuPlayer = lbl_80265750[playerNo].masuId;
    linkNum = MasuGetBranchLink(masuPlayer, branchLinkTbl);
    if (!debugF && linkNum <= 1) {
        *masuId = branchLinkTbl[0];
        return FALSE;
    }
    choice = 0;
    if (GWPartyGet() != FALSE) {
        choice = BranchComChoiceGet(playerNo, linkNum, branchLinkTbl, TRUE);
    }
    BranchGuideCreate(playerNo, masuPlayer, branchLinkTbl, linkNum, choice);
    if (debugF) {
        linkNum += mbMasuLinkParentGet(masuPlayer, &branchLinkTbl[linkNum]);
    }
    mbMasuPosGet(masuPlayer, &pos);
    mbPos3Dto2D(&pos, &pos2D);
    for (i = 0; i < linkNum; i++) {
        mbMasuPosGet(branchLinkTbl[i], &posLink);
        mbPos3Dto2D(&posLink, &posLink2D);
        PSVECSubtract(&posLink2D, &pos2D, &dir2D);
        anglePath[i] = fmod((180.0*(atan2((-dir2D.y), dir2D.x) / M_PI)), 360.0);
    }
    if (!debugF) {
        mbPlayerMotIdleSet(playerNo);
        HuPrcSleep(10);
    }
    mbConfigPadDisableSet(FALSE);
    padNo = lbl_80265750[playerNo].padNo;
    while (TRUE) {
        u16 btnDown;

        if (fn_8006A124(65550)) {
            if (choiceTime < 0) {
                i = mbTutorialCall(13);

                if (i < 0) {
                    i = 0;
                }
                choiceAuto = 4;
                if (i & 8) {
                    choiceAuto -= 3;
                }
                if (i & 4) {
                    choiceAuto += 3;
                }
                if (i & 1) {
                    choiceAuto--;
                }
                if (i & 2) {
                    choiceAuto++;
                }
                dir.x = 64.0f * mbCosDeg(angleTbl[choiceAuto]);
                dir.y = 64.0f * mbSinDeg(angleTbl[choiceAuto]);
                choiceTime = 30;
            } else {
                if (--choiceTime == 0) {
                    btnDown = PAD_BUTTON_A;
                }
            }
        } else if (lbl_80265750[playerNo].comF && !debugF) {
            if (choiceTime < 0) {
                static const s8 chanceTbl[] = { 30, 20, 10, 0 };

                if (GWPartyGet() != FALSE) {
                    choiceAuto = BranchComChoiceGet(playerNo, linkNum, branchLinkTbl, FALSE);
                } else if (mbRandMod(100) < chanceTbl[lbl_80265750[playerNo].comDif]) {
                    choiceAuto = mbRandMod(linkNum);
                } else {
                    choiceAuto = choice;
                }
                dir.x = 64.0 * cos(M_PI*anglePath[choiceAuto]/180.0);
                dir.y = 64.0 * sin(M_PI*anglePath[choiceAuto]/180.0);
                btnDown = 0;
                choiceTime = 6;
            } else {
                if (--choiceTime == 0) {
                    btnDown = PAD_BUTTON_A;
                }
            }
        } else {
            dir.x = mbPadStkXGet(padNo);
            dir.y = mbPadStkYGet(padNo);
            btnDown = lbl_802C0008[padNo];
            if (debugF) {
                btnDown = lbl_802C0010[padNo];
            }
        }
        if (btnDown == PAD_BUTTON_A) {
            *masuId = branchLinkTbl[choice];
            mbAudFXPlay(MSM_SE_CMN_MENU_ADVANCE);
            BranchGuideKill(playerNo);
            break;
        }
        if (btnDown == PAD_BUTTON_Y || btnDown == PAD_BUTTON_X) {
            mbAudFXPlay(MSM_SE_CMN_CHOICE_CONFIRM);
            mbMoveNumDispSet(playerNo, FALSE);
            BranchGuideKill(playerNo);
            mbPlayerMotIdleSet(playerNo);
            mbev_Scroll(playerNo, (btnDown == PAD_BUTTON_Y) ? TRUE : FALSE);
            mbMoveNumDispSet(playerNo, TRUE);
            BranchGuideCreate(playerNo, masuPlayer, branchLinkTbl, linkNum, choice);
            continue;
        }
        if (debugF && lbl_802C0008[padNo] == PAD_BUTTON_B) {
            BranchGuideKill(playerNo);
            mbConfigPadDisableSet(TRUE);
            return TRUE;
        }
        if (dir.x != 0.0f || dir.y != 0.0f) {
            float angle = (180.0*(atan2(dir.y, dir.x) / M_PI));
            int branchDir = -1;
            float bestDiff = 9999.0f;

            for (i = 0; i < linkNum; i++) {
                float angleDiff = fmod(anglePath[i] - angle, 360.0);

                if (angleDiff < -180.0f) {
                    angleDiff += 360.0f;
                } else if (angleDiff > 180.0f) {
                    angleDiff -= 360.0f;
                }
                if (fabs(angleDiff) < bestDiff) {
                    branchDir = i;
                    bestDiff = fabs(angleDiff);
                }
            }
            if (bestDiff < 45.0f) {
                if (choice != branchDir) {
                    mbAudFXPlay(MSM_SE_CMN_CURSOR_MOVE);
                    choice = branchDir;
                    BranchGuideChoiceSet(playerNo, choice);
                }
                if (debugF) {
                    *masuId = branchLinkTbl[choice];
                    BranchGuideKill(playerNo);
                    break;
                }
            }
        } else {
            mbPlayerMotIdleSet(playerNo);
        }
        HuPrcVSleep();
    }
    mbConfigPadDisableSet(TRUE);
    return FALSE;
}

BOOL mbev_Branch(int playerNo, s16 *masuId)
{
    return ev_Branch(playerNo, masuId, FALSE);
}

BOOL mbev_BranchDebug(int playerNo, s16 *masuId)
{
    return ev_Branch(playerNo, masuId, TRUE);
}

void mbBranchAttrSet(u16 attr)
{
    branchAttr |= attr;
}

void mbBranchMAttrSet(u32 attr)
{
    branchMAttr |= attr;
}

void mbBranchAttrReset(u16 attr)
{
    branchAttr &= ~attr;
}

void mbBranchMAttrReset(u32 attr)
{
    branchMAttr &= ~attr;
}

void mbBranchAttrInit(void)
{
    branchAttr = MB_BRANCH_ATTR_DEFAULT;
}

void mbBranchMAttrInit(void)
{
    branchMAttr = 0;
}

u16 mbBranchAttrGet(void)
{
    return branchAttr;
}

u32 mbBranchMAttrGet(void)
{
    return branchMAttr;
}

BOOL mbBranchAttrCheck(int masuId)
{
    MASU *masuP = mbMasuGet(masuId);

    if ((masuP->flag & mbBranchAttrGet()) != 0) {
        return FALSE;
    }
    if ((masuP->mAttr & mbBranchMAttrGet()) != 0) {
        return FALSE;
    }
    return TRUE;
}

static int BranchComChoiceGet(int playerNo, int linkNum, s16 *linkTbl, BOOL debugF)
{
    int choice;
    int len;
    int bestLen;
    int i;
    static const s8 chanceTbl[] = { 30, 20, 10, 0 };

    choice = -1;
    bestLen = 9999;

    if (!debugF && mbPlayerWorkGet(playerNo)->masuNext != 0) {
        for (i = 0; i < linkNum; i++) {
            int unk0E = mbPlayerWorkGet(playerNo)->masuNext;

            len = mbMasuFind_IdStepGet(linkTbl[i], unk0E);

            if (len < bestLen) {
                choice = i;
                bestLen = len;
            }
        }
        return choice;
    }
    if (branchComStarHook != NULL) {
        choice = branchComStarHook(playerNo, linkNum, linkTbl, debugF);
    } else {
        for (i = 0; i < linkNum; i++) {
            len = mbMasuFind_TypeStepGet2(linkTbl[i], 7, TRUE, TRUE);

            if (len < bestLen) {
                choice = i;
                bestLen = len;
            }
        }
        if (choice < 0
            || (bestLen > 20
                && mbRandMod(100) < chanceTbl[lbl_80265750[playerNo].comDif]
                && !debugF)) {
            i = 0;
            while (TRUE) {
                if (mbMasuGet(linkTbl[i])->linkNum != 0 && mbRandMod(1000) < 500) {
                    break;
                }
                i++;
                if (i >= linkNum) {
                    i = 0;
                }
            }
            choice = i;
        }
    }
    return choice;
}

void mbBranchComStarHookSet(MBBRANCHCOMSTARHOOK hook)
{
    branchComStarHook = hook;
}
