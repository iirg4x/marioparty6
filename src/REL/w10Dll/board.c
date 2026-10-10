/* Implements the W10 board setup and its board tutorial scenes. */
#include "REL/w10Dll/module_context.h"

s8 lbl_1_data_0[5] = { 1, 12, 21, 7, -1 };
#include "humath.h"
#include "game/esprite.h"
#include "game/board/effect.h"
extern void mbNormPosto3D(HuVecF *src, s16 cameraMask, HuVecF *dst);
extern void fn_1_2EF8(void);
#include "REL/w10Dll/module_context.h"

extern void fn_1_1F4(void);
extern void fn_1_418(void);
void mbObjectSetup(s32 boardNo, void (*init)(void), void (*close)(void));

#include "REL/w10Dll/module_context.h"
#include "game/board/masu.h"
#include "game/board/opening.h"
#include "game/board/main.h"
#include "msm_grp.h"

extern void mbScrollInit(int dataNum);
extern int mbCapThrowColCreate(int dataNum);
extern void mbLightFuncSet(void (*setHook)(void), void (*resetHook)(void));
extern void mbev_ShopInit(int dataNum);
extern void mbStarMasuNextSet(int masuId);
extern void fn_1_458(OMOBJ *obj);
extern void fn_1_45C(void);
extern void fn_1_490(void);
extern void fn_1_494(void);
extern MBMODELID lbl_1_bss_10;

/* Returns whether the current board time uses W10's day assets. */
static inline BOOL MBTimeDayGet(void)
{
    return GwSystem.curTime == 0;
}

#include "REL/w10Dll/module_context.h"

extern MBMODELID lbl_1_bss_10;

#include "REL/w10Dll/module_context.h"

extern MBMODELID lbl_1_bss_10;

void fn_1_4BC(void);

#include "REL/w10Dll/module_context.h"

extern s8 lbl_1_data_0[5];
extern void fn_1_2CB8(void);
extern void fn_1_554(void);
extern void fn_1_A68(void);
extern void fn_1_C3C(void);
extern void fn_1_1924(void);
extern void fn_1_2484(void);
extern void fn_1_2B34(void);

#include "REL/w10Dll/module_context.h"

extern void mbCameraMoveMasu(s16 masuId, HuVecF *rot, HuVecF *offset,
    float zoom, float fov, s16 maxTime);
extern void mbCameraMoveWait(void);
extern void mbCameraPlayerViewSetFast(int playerNo, int viewNo);
extern void mbCameraStackPop(int maxTime);
extern int mbCameraStackPush(void);
extern void mbCapMasuPlayerTypeSet(s16 masuId, s16 capsuleNo, s16 playerNo);
extern s16 mbMasuFind_MAttrIdGet(s16 id, u32 attr);
extern void mbMoveNumDispSet(int playerNo, BOOL dispF);
extern int mbPlayerCapsuleAdd(int playerNo, int capsuleNo);
extern void mbPlayerMotIdleSet(int playerNo);
extern u32 mbPlayerNameMesGet(int playerNo);
extern void mbWinInsertMesSet(s16 winNo, u32 insertMes, int insertMesNo);
extern void mbWipeDissolveFadeIn(void);
extern void mbWipeDissolveFadeOut(void);
extern void fn_1_306C(float x, float y, float z);
extern void fn_1_30EC(void);

#include "REL/w10Dll/module_context.h"
#include "game/board/status.h"
#include "game/board/window.h"

extern s16 mbTelopTimeSprCreate(void);
extern void mbTelopTimeStarSet(s16 group, int stars);
extern void mbTelopTimeTPLvlSet(s16 group, float level);
extern void mbTelopTimeSprRotSet(s16 group, float time);
extern void mbWipeDissolveFadeIn(void);
extern void mbWipeFadeOut(void);
extern void fn_1_30EC(void);

#include "REL/w10Dll/module_context.h"

extern void mbPlayerDispSet(int playerNo, BOOL dispF);
extern float mbCameraPlayerViewZoomGet(int viewNo);
extern void mbCameraZoomSet(float zoom);
extern void mbGuideFadeIn(OMOBJ *obj);
extern s16 mbMasuFind_TypeSearch(s16 id, s16 type);
extern void mbMasuTypeSet(s16 id, int type);
extern void mbStarDispSetAll(BOOL dispF);
extern void mbWipeDissolveFadeIn(void);
extern void mbWipeDissolveFadeOut(void);
extern void fn_1_306C(float x, float y, float z);

extern u8 lbl_1_data_58[9];
extern u8 lbl_1_data_61[9];
extern u8 lbl_1_data_6A[4];
extern u8 lbl_1_data_6E[4];
extern u8 lbl_1_data_72[4];
extern u8 lbl_1_data_76[4];
extern u8 lbl_1_data_7A[4];
extern u8 lbl_1_data_7E[4];
extern u8 lbl_1_data_82[4];
extern u8 lbl_1_data_86[4];
extern u8 lbl_1_data_8A[4];
extern float lbl_1_data_8[10][2];
extern int mbBoardDataNumGet(int dataNum);
extern BOOL mbStatusOffCheckAll(void);
extern float mbSinDeg(float angle);
extern void mbMgCallVsEffCreate(void);
extern void mbTutorialMgCallExec(int mode);
extern void mbGuideMotionShiftSet(OMOBJ *guide, s16 motion, BOOL shift);
extern BOOL mbGuideMotionCheck(OMOBJ *guide);
extern void mbPlayerPosReset(int player);
extern void mbWipeFadeIn(void);

#include "REL/w10Dll/module_context.h"

typedef struct W10SPRPAIR_s {
    s16 spriteIds[2]; /* Banner ID at 0, label ID at 1; -1 means that sprite is unused. */
} W10SPRPAIR;

#include "REL/w10Dll/module_context.h"

extern void mbCameraOffsetSet(float x, float y, float z);
extern void mbCameraRotSet(float x, float y, float z);
extern void mbPlayerDispSet(int playerNo, BOOL dispF);
extern void mbPlayerPosSetV(int playerNo, const HuVecF *pos);
extern void mbPlayerWinLoseVoicePlay(int playerNo, int motNo, int seId);
extern void mbPlayerMotionShiftSet(int playerNo, int motNo, float start, float end, u32 attr);
extern void mbPlayerMotionEndWait(int playerNo);
extern void mbWipeDissolveFadeIn(void);
extern void mbWipeFadeOut(void);

#include "REL/w10Dll/module_context.h"

#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"

typedef u16 (*W10DICEPADHOOK)(int playerNo);

extern void mbDiceExec(int playerNo, int diceType, void *valueTbl, int tutorialVal,
    BOOL padWinF, BOOL waitF, void *pos, int color);
extern BOOL mbDiceKillCheckAll(int playerCount);
extern void mbDiceNumShrinkSet(int playerNo);
extern void mbDicePadBtnHookSet(int playerNo, W10DICEPADHOOK hook);
extern void mbPlayerMotIdleSet(int playerNo);
extern void mbPlayerMotionEndWait(int playerNo);
extern void mbPlayerMotionShiftSet(int playerNo, int motNo, float start, float end,
    u32 attr);
extern u32 mbPlayerNameMesGet(int playerNo);
extern void mbPlayerWinLoseVoicePlay(int playerNo, int voiceNo, int seNo);
extern void mbStatusDispSet(int playerNo, BOOL enable);
extern void mbWinInsertMesSet(s16 winNo, u32 insertMes, int insertMesNo);
extern u16 fn_1_2EA8(int playerNo);
extern int lbl_1_bss_0[4];
extern int lbl_1_data_9C[4];

#include "REL/w10Dll/module_context.h"

extern int mbGuideModelGet(OMOBJ *obj);
extern void mbNormPosto3D(HuVecF *src, s16 cameraMask, HuVecF *dst);

#include "REL/w10Dll/module_context.h"
#include "game/board/camera.h"

extern HuVecF lbl_1_data_AC;

/* Called from the module prolog to set W10's 20-turn rules and install board setup callbacks. */
void fn_1_A0(void)
{
    GWPartySet(TRUE);
    _SetFlag(FLAG_BOARD_TUTORIAL);
    GwSystem.turnMax = 20;
    GwSystem.tagF = FALSE;
    GWBonusStarSet(FALSE);
    GWMgInstDispSet(TRUE);
    GWMgComDispSet(TRUE);
    GwSystem.mgPack = 0;
    GWVibrateSet(GwCommon.vibrateF);
    GWMessSpeedSet(GW_MESS_SPEED_NORMAL);
    mbObjectSetup(9, fn_1_1F4, fn_1_418);
}

/* Called by board setup to load day or night assets and install W10's board objects and hooks. */
void fn_1_1F4(void)
{
    int boardNo = MBBoardNoGet();
    HuAudSndGrpSetSet(MSM_GRP_BRDTT);
    mbObjDirSet(DATA_w10, DATA_w10n);
    mbMasuInit(MBTimeDayGet() ? DATA_w10 : DATA_w10n);
    lbl_1_bss_10 = mbObjCreate((MBTimeDayGet() ? DATA_w10 : DATA_w10n) | 1, NULL, FALSE);
    mbObjAttrSet(lbl_1_bss_10, HU3D_MOTATTR_LOOP);
    mbObjPosSet(lbl_1_bss_10, 0.0f, 0.0f, 0.0f);
    mbObjCullRadiusSet(lbl_1_bss_10, -1.0f);
    mbScrollInit(0);
    mbCapThrowColCreate(-1);
    mbLightFuncSet(fn_1_45C, fn_1_490);
    mbOpeningInstHookSet(NULL);
    mbOpeningStarInstHookSet(NULL);
    mbev_ShopInit((MBTimeDayGet() ? DATA_w10 : DATA_w10n) | 2);
    HuDataDirClose(MBTimeDayGet() ? DATA_w10 : DATA_w10n);
    omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_458);
    mbStarMasuNextSet(mbMasuFind_TypeSearch(-1, 7));
    fn_1_494();
}

/* Releases the W10 board model when board setup is torn down. */
void fn_1_418(void) {
    if (lbl_1_bss_10 >= 0) {
        mbObjKill(lbl_1_bss_10);
    }
}

/* Per-frame board object callback; this object has no frame behavior. */
void fn_1_458(OMOBJ *obj) {
    (void)obj;
}

/* Imports W10's HSF lights as static global lights during the board lighting pass. */
void fn_1_45C(void) {
    Hu3DModelLightInfoSet(mbObjModelIDGet(lbl_1_bss_10), 1);
}

/* Reset hook for W10's board lighting pass; no reset adjustment is needed. */
void fn_1_490(void) {
}

/* Registers the tutorial scene callback after board initialization. */
void fn_1_494(void) {
    mbTutorialMainFuncSet(fn_1_4BC);
}

/* Tutorial child-process callback that runs W10's opening and follow-up tutorial scenes. */
void fn_1_4BC(void)
{
    mbTutorialGuideCreate(lbl_1_data_0, TRUE);
    switch (mbTutorialModeGet()) {
    case 0:
        fn_1_2CB8();
        fn_1_554();
        fn_1_A68();
        mbTutorialModeSet(1);
        break;
    case 1:
        mbTutorialCallWait(1);
        fn_1_C3C();
        fn_1_1924();
        fn_1_2484();
        fn_1_2B34();
        mbTutorialModeSet(2);
        mbTutorialVSleep();
        mbTutorialExitOnSet(FALSE);
        mbTutorialExitSet();
        break;
    }
    mbTutorialCallWait(24);
}

/* Called by fn_1_4BC to present W10's turn, space, capsule, and movement lessons in order. */
void fn_1_554(void) {
    s32 winNo;

    fn_1_306C(-0.75f, -0.7f, -800.0f);
    mbTutorialTurnCall(1);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 8));
    mbTutorialCallWait(6);
    winNo = mbTutorialWinCreate(MESSNUM(MESS_BOARD_TUTORIAL, 9));
    mbWinInsertMesSet((s16) winNo, mbPlayerNameMesGet(0), 0);
    mbWinInsertMesSet((s16) winNo, (u32)"1", 1);
    mbTutorialWinKeyWait(winNo);
    mbTutorialCallWait(10);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 10));
    mbTutorialWinMesMasuExec(MESSNUM(MESS_BOARD_TUTORIAL, 11), DATANUM(DATA_btutorial, 0));
    mbTutorialTurnCall(2);
    mbTutorialMesMasuCall(10, MESSNUM(MESS_BOARD_TUTORIAL, 12), DATANUM(DATA_btutorial, 1));
    mbTutorialTurnCall(3);
    mbTutorialMesMasuCall(4, MESSNUM(MESS_BOARD_TUTORIAL, 13), DATANUM(DATA_btutorial, 2));
    mbTutorialTurnCall(4);
    mbTutorialMesMasuCall(4, MESSNUM(MESS_BOARD_TUTORIAL, 14), DATANUM(DATA_btutorial, 3));
    mbTutorialCallWait(2);
    fn_1_30EC();
    mbWipeDissolveFadeIn();
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 15));
    mbWipeDissolveFadeOut();
    mbTutorialTurnCall(5);
    mbTutorialMesMasuCall(4, MESSNUM(MESS_BOARD_TUTORIAL, 16), DATANUM(DATA_btutorial, 4));
    mbTutorialTurnCall(5);
    mbTutorialMesMasuCall(4, MESSNUM(MESS_BOARD_TUTORIAL, 17), DATANUM(DATA_btutorial, 5));
    mbTutorialCallWait(3);
    mbCapMasuPlayerTypeSet(mbMasuFind_MAttrIdGet(-1, 1), 10, 3);
    mbCapMasuPlayerTypeSet(mbMasuFind_MAttrIdGet(-1, 2), 20, 0);
    mbPlayerCapsuleAdd(0, 12);
    mbPlayerCapsuleAdd(1, 11);
    mbPlayerCapsuleAdd(3, 0);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 20));
    mbTutorialTurnCall(8);
    mbTutorialCallResult(21, 11);
    mbTutorialMesMasuCall(22, MESSNUM(MESS_BOARD_TUTORIAL, 21), DATANUM(DATA_btutorial, 8));
    mbTutorialCallResult(19, 1);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 22));
    mbTutorialCallResult(20, 23);
    mbTutorialMesCall(3, MESSNUM(MESS_BOARD_TUTORIAL, 23));
    mbTutorialCapsuleUseCall(0);
    mbTutorialMesCall(16, MESSNUM(MESS_BOARD_TUTORIAL, 24));
    mbTutorialTurnCall(4);
    mbTutorialTurnCall(7);
    mbTutorialCallResult(21, 31);
    mbTutorialGuideCall(2);
    mbTutorialCapsuleUseCall(12);
    mbTutorialMesCall(16, MESSNUM(MESS_BOARD_TUTORIAL, 25));
    mbTutorialCallResult(17, mbMasuFind_MAttrIdGet(-1, 4));
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 26));
    mbTutorialMesCall(18, MESSNUM(MESS_BOARD_TUTORIAL, 27));
    mbTutorialTurnCall(3);
    mbTutorialCallWait(4);
    mbWipeDissolveFadeOut();
    mbCameraPlayerViewSetFast(GwSystem.turnPlayerNo, 0);
    mbWipeDissolveFadeIn();
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 28));
    mbTutorialCapsuleUseCall(11);
    mbTutorialCallResult(17, mbMasuFind_MAttrIdGet(-1, 4));
    mbTutorialMesCall(18, MESSNUM(MESS_BOARD_TUTORIAL, 29));
    mbTutorialTurnCall(1);
    mbTutorialMesCall(4, MESSNUM(MESS_BOARD_TUTORIAL, 30));
    mbTutorialCapsuleUseCall(23);
    mbTutorialMesCall(16, MESSNUM(MESS_BOARD_TUTORIAL, 31));
    mbTutorialCallResult(17, mbMasuFind_MAttrIdGet(-1, 8));
    mbTutorialMesCall(18, MESSNUM(MESS_BOARD_TUTORIAL, 32));
    mbTutorialTurnCall(5);
    mbTutorialMultiCall(9, 1);
    mbPlayerMotIdleSet(GwSystem.turnPlayerNo);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 33));
    mbTutorialGuideCall(4);
    mbCameraStackPush();
    mbCameraMoveMasu(mbMasuFind_MAttrIdGet(-1, 2), NULL, NULL,
        -1.0f, -1.0f, 60);
    mbCameraMoveWait();
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 37));
    mbCameraStackPop(40);
    mbCameraMoveWait();
    mbTutorialMultiCall(9, 3);
    mbPlayerMotIdleSet(GwSystem.turnPlayerNo);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 34));
    mbMoveNumDispSet(GwSystem.turnPlayerNo, 0);
    mbCameraStackPush();
    mbCameraMoveMasu(mbMasuFind_MAttrIdGet(-1, 2), NULL, NULL,
        -1.0f, -1.0f, 30);
    mbCameraMoveWait();
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 38));
    mbCameraStackPop(20);
    mbCameraMoveWait();
    mbMoveNumDispSet(GwSystem.turnPlayerNo, 1);
    mbTutorialCapsuleUseCall(31);
    mbTutorialMesCall(16, MESSNUM(MESS_BOARD_TUTORIAL, 35));
    mbTutorialTurnCall(1);
    mbTutorialMesCall(4, MESSNUM(MESS_BOARD_TUTORIAL, 36));
}

/* Called by fn_1_4BC after the first lesson to show the three-star result and rotate it away. */
void fn_1_A68(void)
{
    int window;
    int fadeFrame;
    int stars;
    int group;
    int turnFrame;

    mbTutorialCallWait(2);
    fn_1_30EC();
    mbStatusDispForceSetAll(FALSE);
    mbWipeDissolveFadeIn();
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 39));
    stars = 3;
    group = mbTelopTimeSprCreate();
    mbTutorialSprGrpSet(group);
    mbTelopTimeStarSet(group, stars);
    HuSprGrpPosSet(group, 288.0f, 176.0f);
    window = mbTutorialWinCreate(MESSNUM(MESS_BOARD_TUTORIAL, 40));
    for (fadeFrame = 0; fadeFrame <= 30; fadeFrame++) {
        mbTelopTimeTPLvlSet(group, (1.0f / 30.0f) * fadeFrame);
        HuPrcVSleep();
    }
    turnFrame = 0;
    while (!mbWinDoneCheck(window)) {
        turnFrame++;
        if (turnFrame >= 60) {
            turnFrame = 0;
        }
        mbTelopTimeSprRotSet(group, (1.0f / 60.0f) * turnFrame);
        HuPrcVSleep();
    }
    while (stars != 0) {
        turnFrame++;
        if (turnFrame >= 60) {
            turnFrame = 0;
            stars--;
            mbTelopTimeStarSet(group, stars);
        }
        mbTelopTimeSprRotSet(group, (1.0f / 60.0f) * turnFrame);
        HuPrcVSleep();
    }
    mbWipeFadeOut();
    mbTutorialSprGrpKill(group);
}

/* Called by fn_1_4BC to hide players and move the tutorial view from their spaces to the star. */
void fn_1_C3C(void)
{
    int playerNo;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbPlayerDispSet(playerNo, FALSE);
    }
    fn_1_306C(-0.75f, -0.7f, -800.0f);
    mbWipeDissolveFadeIn();
    mbGuideFadeIn(mbTutorialGuideGet());
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 41));
    mbWipeDissolveFadeOut();
    mbTutorialViewMasuSet(mbMasuFind_TypeSearch(-1, 6));
    mbWipeDissolveFadeIn();
    mbTutorialWinMesMasuExec(MESSNUM(MESS_BOARD_TUTORIAL, 42), DATANUM(DATA_btutorial, 5));
    mbTutorialWinMesMasuExec(MESSNUM(MESS_BOARD_TUTORIAL, 43), DATANUM(DATA_btutorial, 6));
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 44));
    mbWipeDissolveFadeOut();
    mbTutorialViewMasuSet(mbMasuFind_TypeSearch(-1, 7));
    mbTutorialVSleep();
    mbCameraZoomSet(mbCameraPlayerViewZoomGet(1));
    mbWipeDissolveFadeIn();
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 62));
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 63));
    mbStarDispSetAll(FALSE);
    mbMasuTypeSet(mbMasuFind_TypeSearch(-1, 7), 1);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 64));
    mbWipeDissolveFadeOut();
}

/* Called by fn_1_1924 to cycle all four status colors while a tutorial message is open. */
void fn_1_D78(int message, u8 *colors)
{
    int frame;
    int window;
    int player;

    window = mbTutorialWinCreate(message);
    for (frame = 0; frame <= 36U; frame++) {
        for (player = 0; player < 4; player++) {
            mbStatusRainbowSet(player, (float)frame / 36.0f, colors[player]);
        }
        mbTutorialVSleep();
    }
    mbTutorialWinKeyWait(window);
}

/* Returns the sine magnitude used to scale the tutorial banner. */
static inline float fabsf2(register float x)
{
    asm {
        fabs x, x
    }
    return x;
}

/* Called by fn_1_1924 to reposition panels and show an optional banner; it returns its sprites for
 * the reveal or fades and kills them. */
void fn_1_E3C(int message, u8 *layout, int bannerNo, s16 *sprites)
{
    HuVecF position;
    int window;
    int banner;
    int label;
    int player;
    float progress;
    float fadeProgress;
    float xScale;
    float yScale;
    float slideProgress;

    banner = -1;
    label = -1;
    for (player = 0; player < 4; player++) {
        mbStatusPosGet(player, (HuVecF *)&position);
        mbStatusMoveTo(player, (HuVecF *)&position, (HuVecF *)lbl_1_data_8[layout[player]]);
    }
    while (!mbStatusOffCheckAll()) {
        mbTutorialVSleep();
    }
    window = mbTutorialWinCreate(message);
    if (bannerNo >= 0) {
    banner = mbTutorialSprCreate(mbBoardDataNumGet(DATANUM(DATA_board, 138)));
        espPriSet(banner, 90);
        espPosSet(banner, 288.0f, 0.7f * 288.0f);
        espScaleSet(banner, 0.5f, 0.5f);
    label = mbTutorialSprCreate(mbBoardDataNumGet(DATANUM(DATA_board, 132)));
        espPriSet(label, 100);
        espBankSet(label, lbl_1_data_58[bannerNo]);
        espPosSet(label, 288.0f, -0.7f * 288.0f);
        espScaleSet(label, 0.75f, 0.75f);
        mbAudFXPlay(MSM_SE_BRD00_108);
        for (player = 1; player <= 30; player++) {
            progress = (float)player / 30.0f;
            xScale = 0.5f * mbSinDeg(90.0f * progress) +
                1.5f * mbSinDeg(180.0f * progress);
            yScale = 0.5f * mbSinDeg(90.0f * progress) + 1.5f * fabsf2(mbSinDeg(360.0f * progress));
            espPosSet(banner, 288.0f - 32.0f * (1.0f - progress),
                0.7f * 288.0f + 64.0f * mbSinDeg(180.0f * progress));
            espScaleSet(banner, xScale, yScale);
            espZRotSet(banner, 360.0f * -progress);
            mbTutorialVSleep();
        }
        for (player = 1; player <= 20; player++) {
            slideProgress = (float)player / 20.0f;
            espPosSet(label, 288.0f, 100.0f * slideProgress - 30.0f);
            mbTutorialVSleep();
        }
    }
    mbTutorialWinKeyWait(window);
    if (bannerNo >= 0) {
        if (sprites == NULL) {
            for (player = 0; player < 12U; player++) {
                fadeProgress = (float)player / 12.0f;
                espScaleSet(banner, 0.5f * (1.0f - fadeProgress),
                    0.5f * (1.0f - fadeProgress));
                espTPLvlSet(banner, 1.0f - fadeProgress);
                espTPLvlSet(label, 1.0f - fadeProgress);
                mbTutorialVSleep();
            }
            mbTutorialSprKill(banner);
            mbTutorialSprKill(label);
            banner = label = -1;
        } else {
            sprites[0] = banner;
            sprites[1] = label;
        }
    }
}

/* Called by fn_1_1924 after its banner message to play the status-panel reveal effect. */
void fn_1_135C(s16 *sprites)
{
    HuVecF position;
    int primarySprite = -1;
    int secondarySprite = -1;
    int frame;
    float progress;
    float scale;
    float labelScale;

    if (sprites != NULL) {
        for (frame = 1; frame <= 12U; frame++) {
            progress = (float)frame / 12.0f;
            scale = 0.1f + 0.4f * HuCos(90.0f * progress);
            espScaleSet(sprites[0], scale, scale);
            mbTutorialVSleep();
        }
        for (frame = 0; frame < 4; frame++) {
            mbStatusPosGet(frame, (HuVecF *)&position);
            if (position.x >= 288.0f) {
                position.x = 704.0f;
            } else {
                position.x = -128.0f;
            }
            if (position.y > 0.7f * 288.0f + 16.0f) {
                position.y += 128.0f;
            } else if (position.y < 0.7f * 288.0f - 16.0f) {
                position.y -= 128.0f;
            }
            mbStatusMoveSet(frame, NULL, (HuVecF *)&position, 1, 15);
        }
        mbAudFXPlay(MSM_SE_BRD00_109);
        mbMgCallVsEffCreate();
        for (frame = 1; frame <= 30U; frame++) {
            progress = (float)frame / 30.0f;
            scale = 0.1f + 3.0f * mbSinDeg(90.0f * progress);
            espScaleSet(sprites[0], scale, scale);
            espTPLvlSet(sprites[0], 1.0f - progress);
            espZRotSet(sprites[0], 360.0f * mbSinDeg(90.0f * progress));
            espPosSet(sprites[1], 288.0f, 70.0f + 8.0f * progress);
            labelScale = 0.75f + 0.25f * progress;
            espScaleSet(sprites[1], labelScale, labelScale);
            mbTutorialVSleep();
        }
        espDispOff(sprites[0]);
    }
}

/* Called by fn_1_1924 at scene exit to kill both tutorial sprites and mark their IDs unused. */
void fn_1_16B4(W10SPRPAIR *spritePair)
{
    if (spritePair->spriteIds[1] >= 0) mbTutorialSprKill(spritePair->spriteIds[1]);
    if (spritePair->spriteIds[0] >= 0) mbTutorialSprKill(spritePair->spriteIds[0]);
    spritePair->spriteIds[1] = spritePair->spriteIds[0] = -1;
}

/* Called by fn_1_1924 between banner lessons to slide in the next label and remove the old one. */
void fn_1_1710(int bannerNo, W10SPRPAIR *sprites)
{
    int exitFrame;
    int previousSprite = -1;
    int sprite = -1;
    int frame;
    float progress;

    sprite = mbTutorialSprCreate(mbBoardDataNumGet(DATANUM(DATA_board, 132)));
    espPriSet(sprite, 100);
    espBankSet(sprite, lbl_1_data_61[bannerNo]);
    espPosSet(sprite, 288.0f, -0.7f * 288.0f);
    espScaleSet(sprite, 1.0f, 1.0f);
    for (frame = 1; frame <= 50; frame++) {
        if (frame <= 20) {
            progress = (float)frame / 20.0f;
            espPosSet(sprite, 288.0f, 108.0f * progress - 30.0f);
        }
        exitFrame = frame - 6;
        if (exitFrame >= 0 && exitFrame <= 30) {
            progress = (float)exitFrame / 30.0f;
            espPosSet(sprites->spriteIds[1], 288.0f, 78.0f + 250.0f * progress);
            espZRotSet(sprites->spriteIds[1], 40.0f * progress);
            espTPLvlSet(sprites->spriteIds[1], 1.0f - progress);
        }
        mbTutorialVSleep();
    }
    espDispOff(sprites->spriteIds[1]);
    mbTutorialSprKill(sprites->spriteIds[1]);
    sprites->spriteIds[1] = sprite;
}

/* Called by fn_1_4BC to run status-panel lessons and reveal effects, then hide the panels and close
 * the tutorial call. */
void fn_1_1924(void)
{
    int window;
    W10SPRPAIR sprites;
    int player;

    fn_1_30EC();
    mbWipeDissolveFadeIn();
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 45));
    mbEffFadeCreate(16, 96);
    mbEffFadeCameraSet(2);
    mbStatusColorAllSet(0);
    for (player = 0; player < 4; player++) {
        mbStatusLayoutSet(player, 0);
    }
    mbStatusDispSetAll(TRUE);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 46));
    fn_1_E3C(MESSNUM(MESS_BOARD_TUTORIAL, 47), lbl_1_data_6A, -1, NULL);
    fn_1_D78(MESSNUM(MESS_BOARD_TUTORIAL, 48), lbl_1_data_7A);
    fn_1_D78(MESSNUM(MESS_BOARD_TUTORIAL, 49), lbl_1_data_7E);
    fn_1_E3C(MESSNUM(MESS_BOARD_TUTORIAL, 50), lbl_1_data_6E, 0, NULL);
    fn_1_D78(MESSNUM(MESS_BOARD_TUTORIAL, 51), lbl_1_data_86);
    fn_1_D78(MESSNUM(MESS_BOARD_TUTORIAL, 52), lbl_1_data_82);
    fn_1_E3C(MESSNUM(MESS_BOARD_TUTORIAL, 53), lbl_1_data_72, 2, NULL);
    fn_1_D78(MESSNUM(MESS_BOARD_TUTORIAL, 54), lbl_1_data_86);
    fn_1_D78(MESSNUM(MESS_BOARD_TUTORIAL, 55), lbl_1_data_8A);
    fn_1_E3C(MESSNUM(MESS_BOARD_TUTORIAL, 56), lbl_1_data_76, 1, sprites.spriteIds);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 57));
    fn_1_135C(sprites.spriteIds);
    mbTutorialMgCallExec(1);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 58));
    window = mbTutorialWinCreate(MESSNUM(MESS_BOARD_TUTORIAL, 59));
    fn_1_1710(3, &sprites);
    mbTutorialWinWait(window);
    mbGuideMotionShiftSet(mbTutorialGuideGet(), 12, TRUE);
    fn_1_1710(6, &sprites);
    mbTutorialWinKeyWait(window);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 60));
    mbWipeDissolveFadeOut();
    mbEffFadeOutSet(1);
    mbStatusDispForceSetAll(FALSE);
    mbTutorialMgCallClose();
    fn_1_16B4(&sprites);
}

/* Called by fn_1_2484 for each capsule example to show, spin, and shrink its board model. */
void fn_1_2234(int message, int dataNum)
{
    HuVecF worldPosition;
    HuVecF screenPosition = { 0.0f, 0.2f, -650.0f };
    int window;
    int model;
    int frame;
    float progress;

    window = mbTutorialWinCreate(message);
    model = mbTutorialModelCreate(dataNum, FALSE);
    mbObjCameraSet(model, 4);
    mbNormPosto3D(&screenPosition, 4, &worldPosition);
    mbObjPosSetV(model, &worldPosition);
    for (frame = 0; frame <= 30U; frame++) {
        progress = (float)frame / 30.0f;
        mbObjScaleSet(model, 2.0f * progress, 2.0f, 2.0f * progress);
        mbObjRotSet(model, 0.0f, -720.0f * (1.0f - progress), 0.0f);
        mbTutorialVSleep();
    }
    mbTutorialWinKeyWait(window);
    for (frame = 0; frame <= 12U; frame++) {
        progress = (float)frame / 12.0f;
        mbObjScaleSet(model, 2.0f * (1.0f - progress), 2.0f, 2.0f * (1.0f - progress));
        mbObjRotSet(model, 0.0f, 720.0f * progress, 0.0f);
        mbTutorialVSleep();
    }
    mbTutorialModelKill((MBMODELID)model);
}

/* Called by fn_1_4BC to present the tutorial's three capsule illustrations. */
void fn_1_2484(void)
{
    fn_1_30EC();
    mbWipeDissolveFadeIn();
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 65));
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 66));
    fn_1_2234(MESSNUM(MESS_BOARD_TUTORIAL, 67), DATANUM(DATA_btutorial, 14));
    fn_1_2234(MESSNUM(MESS_BOARD_TUTORIAL, 68), DATANUM(DATA_btutorial, 15));
    fn_1_2234(MESSNUM(MESS_BOARD_TUTORIAL, 69), DATANUM(DATA_btutorial, 16));
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 70));
    mbWipeDissolveFadeOut();
}

/* Called by fn_1_4BC at tutorial close to gather all players and play their ending animation. */
void fn_1_2B34(void)
{
    HuVecF pos;
    int playerNo;

    {
        int view;
        view = (s16)(int)mbTutorialViewSet();
        mbTutorialVSleep();
        mbCameraOffsetSet(0.0f, 130.0f, 0.0f);
        mbCameraRotSet(-20.0f, 0.0f, 0.0f);
        mbMasuPosGet((s16)view, &pos);
    }
    pos.x -= 300.0f;
    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbPlayerDispSet(playerNo, TRUE);
        mbPlayerPosSetV(playerNo, &pos);
        pos.x += 200.0f;
    }
    mbWipeDissolveFadeIn();
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 71));
    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbPlayerWinLoseVoicePlay(playerNo, 7, 579);
        mbPlayerMotionShiftSet(playerNo, 7, 0.0f, 8.0f, 0);
    }
    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbPlayerMotionEndWait(playerNo);
    }
    for (playerNo = 0; (unsigned int)playerNo < 60; playerNo++) {
        mbTutorialVSleep();
    }
    mbWipeFadeOut();
}

/* Called by fn_1_4BC to position the players and guide for the opening tutorial scene. */
void fn_1_2CB8(void)
{
    HuVecF position;
    int playerOrder[4] = { 3, 0, 2, 1 };
    int window;
    int space;
    int player;
    int resetPlayer;
    OMOBJ *guide;

    mbTutorialCallWait(0);
    space = mbTutorialViewSet();
    mbTutorialVSleep();
    mbCameraOffsetSet(0.0f, 130.0f, 0.0f);
    mbCameraRotSet(-20.0f, 0.0f, 0.0f);
    fn_1_306C(-0.75f, -0.7f, -800.0f);
    mbMasuPosGet(space, &position);
    position.x -= 300.0f;
    for (player = 0; player < 4; player++) {
        mbPlayerPosSetV(playerOrder[player], &position);
        position.x += 200.0f;
    }
    mbWipeFadeIn();
    guide = mbTutorialGuideGet();
    mbGuideFadeIn(guide);
    mbGuideMotionShiftSet(guide, 12, TRUE);
    window = mbTutorialWinCreate(MESSNUM(MESS_BOARD_TUTORIAL, 0));
    mbTutorialWinKeyWait(window);
    mbGuideMotionShiftSet(guide, 12, TRUE);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 1));
    fn_1_2EF8();
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 7));
    mbGuideMotionShiftSet(guide, 21, TRUE);
    while (!mbGuideMotionCheck(guide)) {
        mbTutorialVSleep();
    }
    mbWipeFadeOut();
    for (resetPlayer = 0; resetPlayer < 4; resetPlayer++) {
        mbPlayerPosReset(resetPlayer);
    }
}

/* Called by the dice input hook each frame to count down a player's tutorial roll delay. */
u16 fn_1_2EA8(int playerNo)
{
    if (lbl_1_bss_0[playerNo] != 0) {
        if (--lbl_1_bss_0[playerNo] == 0) {
            return 256;
        }
    }
    return 0;
}

/* Called by fn_1_2CB8 to roll tutorial dice for every player and present each result in turn. */
void fn_1_2EF8(void) {
    int i;
    int winNo;

    for (i = 0; i < 4; i++) {
        mbDiceExec(i, 0, NULL, lbl_1_data_9C[i] - 1, FALSE, FALSE, NULL, 0);
        lbl_1_bss_0[i] = mbRandMod(30) + 30;
        mbDicePadBtnHookSet(i, fn_1_2EA8);
    }
    while (!mbDiceKillCheckAll(i)) {
        mbTutorialVSleep();
    }
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 2));
    for (i = 0; i < 4; i++) {
        winNo = mbTutorialWinCreate(i + MESSNUM(MESS_BOARD_TUTORIAL, 3));
        mbWinInsertMesSet((s16)winNo, mbPlayerNameMesGet(i), 0);
        mbTutorialWinKeyWait(winNo);
        mbDiceNumShrinkSet(i);
        mbStatusDispSet(i, TRUE);
        mbPlayerWinLoseVoicePlay(i, 12, 579);
        mbPlayerMotionShiftSet(i, 12, 0.0f, 8.0f, 0);
        mbPlayerMotionEndWait(i);
        mbPlayerMotIdleSet(i);
    }
    HuPrcSleep(30);
}

/* Called by tutorial scenes to place the guide at the supplied overlay position in the scene. */
void fn_1_306C(float x, float y, float z)
{
    HuVecF pos;
    OMOBJ *guide;
    MBMODELID modelId;

    pos.x = x;
    pos.y = y;
    pos.z = z;
    guide = mbTutorialGuideGet();
    modelId = mbGuideModelGet(guide);
    mbNormPosto3D(&pos, 4, &pos);
    mbObjPosSetV((s16)modelId, &pos);
}

/* Called by tutorial scenes before overlay lessons to restore W10's fixed camera framing. */
void fn_1_30EC(void)
{
    mbCameraFocusReset();
    mbCameraMoveOnSet(FALSE);
    mbCameraCenterSetV(&lbl_1_data_AC);
    mbCameraZoomSet(10000.0f);
    mbCameraRotSet(-40.0f, 0.0f, 0.0f);
}

float lbl_1_data_8[10][2] = {
    { 150.8f, 0.7f * 288.0f - 72.0f },
    { 425.2f, 0.7f * 288.0f - 72.0f },
    { 150.8f, 0.7f * 288.0f - 36.0f },
    { 425.2f, 0.7f * 288.0f - 36.0f },
    { 150.8f, 0.7f * 288.0f },
    { 425.2f, 0.7f * 288.0f },
    { 150.8f, 0.7f * 288.0f + 36.0f },
    { 425.2f, 0.7f * 288.0f + 36.0f },
    { 150.8f, 0.7f * 288.0f + 72.0f },
    { 425.2f, 0.7f * 288.0f + 72.0f },
};
u8 lbl_1_data_58[9] = { 0, 1, 2, 3, 0, 0, 4, 0, 0 };
u8 lbl_1_data_61[9] = { 0, 1, 2, 3, 0, 0, 4, 0, 0 };
u8 lbl_1_data_6A[4] = { 0, 1, 8, 9 };
u8 lbl_1_data_6E[4] = { 2, 3, 6, 7 };
u8 lbl_1_data_72[4] = { 2, 6, 3, 7 };
u8 lbl_1_data_76[4] = { 0, 4, 8, 5 };
u8 lbl_1_data_7A[4] = { 1, 2, 3, 1 };
u8 lbl_1_data_7E[4] = { 1, 1, 1, 1 };
u8 lbl_1_data_82[4] = { 1, 1, 2, 2 };
u8 lbl_1_data_86[4] = { 2, 2, 1, 2 };
u8 lbl_1_data_8A[4] = { 1, 1, 1, 2 };
HuVecF lbl_1_data_90 = { -0.75f, -0.7f, -750.0f };
int lbl_1_data_9C[4] = { 9, 7, 5, 2 };
HuVecF lbl_1_data_AC = { 0.0f, 0.0f, 0.0f };

MBMODELID lbl_1_bss_10;
int lbl_1_bss_0[4];
