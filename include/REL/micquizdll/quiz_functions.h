/* Quiz dialogue windows, microphone feedback, and animated display processes. */
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"

#define MSM_SE_MICQUIZ_COUNTDOWN_TICK 1269
#define MSM_SE_MICQUIZ_PLAYER_BUZZ_IN 1271
#define MSM_SE_MICQUIZ_EVENT_FRAGMENT 1301

/* Store the answer that the current microphone response must equal. */
void fn_1_200D0(s32 expectedAnswer)
{
    lbl_1_bss_514 = expectedAnswer;
}

/* Mark the current question as ready for its response process. */
void fn_1_200E0(void) {
    lbl_1_bss_4FC = 1;
}

/* Switch to the listening display and signal that the response can advance. */
void fn_1_200F4(void) {
    fn_1_2E6C4(2);
    lbl_1_bss_540 = 1;
    lbl_1_bss_508 = 1;
}

/* Return the current response process state to the round controller. */
s32 fn_1_20138(void) {
    return lbl_1_bss_508;
}

/* Apply the inactive-player mask to the quiz dialogue and hint windows. */
void fn_1_20148(void) {
    s32 playerIndex;
    u8 disabledPlayers;

    disabledPlayers = 0;
    playerIndex = 0;
    while (playerIndex < 4) {
        if (!fn_1_27FEC(playerIndex)) {
            disabledPlayers |= 1 << playerIndex;
        }
        playerIndex += 1;
    }
    HuWinDisablePlayerReset(lbl_1_bss_386, 255);
    HuWinDisablePlayerReset(lbl_1_bss_384, 255);
    HuWinDisablePlayerReset(lbl_1_bss_382, 255);
    HuWinDisablePlayerReset(lbl_1_bss_380, 255);
    HuWinDisablePlayerReset(lbl_1_bss_3AA, 255);
    HuWinDisablePlayerReset(lbl_1_bss_3A8, 255);
    HuWinDisablePlayerSet(lbl_1_bss_386, disabledPlayers);
    HuWinDisablePlayerSet(lbl_1_bss_384, disabledPlayers);
    HuWinDisablePlayerSet(lbl_1_bss_382, disabledPlayers);
    HuWinDisablePlayerSet(lbl_1_bss_380, disabledPlayers);
    HuWinDisablePlayerSet(lbl_1_bss_3AA, disabledPlayers);
    HuWinDisablePlayerSet(lbl_1_bss_3A8, disabledPlayers);
}

/* Create the four dialogue windows and the controller instruction line at quiz setup. */
void fn_1_2029C(void) {
    u8 colorA[4] = {0, 0, 0, 255};
    u8 colorB[4] = {0, 255, 0, 255};

    lbl_1_bss_386 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 6, 4);
    lbl_1_bss_384 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 7, 3);
    lbl_1_bss_382 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 23, 5);
    lbl_1_bss_380 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 13, 0);

    fn_1_20148();
    HuWinAttrSet(lbl_1_bss_386, HUWIN_ATTR_UPAUSE);
    HuWinAttrSet(lbl_1_bss_384, HUWIN_ATTR_UPAUSE);
    HuWinAttrSet(lbl_1_bss_382, HUWIN_ATTR_UPAUSE);
    HuWinAttrSet(lbl_1_bss_380, HUWIN_ATTR_UPAUSE);

    lbl_1_bss_37E = HuWinCreate(16.0f, 312.0f, 544, 32, 0);
    HuWinBGTPLvlSet(lbl_1_bss_37E, 0.0f);
    HuWinMesSpeedSet(lbl_1_bss_37E, 0);
    HuWinAttrSet(lbl_1_bss_37E, HUWIN_ATTR_ALIGN_CENTER);
    HuWinDispOff(lbl_1_bss_386);
    HuWinDispOff(lbl_1_bss_384);
    HuWinDispOff(lbl_1_bss_382);
    HuWinDispOff(lbl_1_bss_380);
    HuWinPriSet(lbl_1_bss_386, 100);
    HuWinPriSet(lbl_1_bss_384, 100);
    HuWinPriSet(lbl_1_bss_382, 100);
    HuWinPriSet(lbl_1_bss_380, 100);
    HuWinPriSet(lbl_1_bss_37E, 100);
    HuWinCallbackSet(lbl_1_bss_386, fn_1_31CEC);
    HuWinCallbackSet(lbl_1_bss_384, fn_1_31CEC);
    HuWinCallbackSet(lbl_1_bss_382, fn_1_31CEC);
    HuWinCallbackSet(lbl_1_bss_380, fn_1_31CEC);
    lbl_1_bss_37A = -1;
}

/* Select the dialogue window used by the requested speaker slot. */
s16 fn_1_206FC(s32 windowNo)
{
    s16 windowId;

    switch (windowNo) {
    case 0:
        windowId = lbl_1_bss_386;
        break;
    case 1:
        windowId = lbl_1_bss_384;
        break;
    case 2:
        windowId = lbl_1_bss_382;
        break;
    case 3:
        windowId = lbl_1_bss_380;
        break;
    }
    return windowId;
}

/* Slide the response banner into view when a participant starts answering. */
void fn_1_20778(void) {
    f32 travelDistance;
    f32 angle;
    f32 sineOffset;

    travelDistance = (362.0f);
    HuSprAttrReset(lbl_1_bss_46C, 0, HUSPR_ATTR_DISPOFF);
    angle = (0.0f);
    if ((s32) lbl_1_bss_4DC == 0) {
        do {
            HuPrcVSleep();
            sineOffset =
                (f32) ((f64) travelDistance * sin(((3.141592653589793) * (f64) angle) / (180.0)));
            HuSprPosSet(lbl_1_bss_46C, 0, lbl_1_bss_4E4, lbl_1_bss_4E0 + sineOffset);
            angle += (3.0f);
        } while (!(angle >= (90.0f)));
        lbl_1_bss_4E0 = (210.0f);
        HuSprPosSet(lbl_1_bss_46C, 0, lbl_1_bss_4E4, lbl_1_bss_4E0);
        lbl_1_bss_4DC = 1;
    }
}

/* Slide the response banner off the screen after an answer or timeout. */
void fn_1_208E4(void) {
    f32 travelDistance;
    f32 angle;
    f32 sineOffset;

    travelDistance = (-362.0f);
    angle = (0.0f);
    if ((s32) lbl_1_bss_4DC != 0) {
        do {
            HuPrcVSleep();
            sineOffset =
                (f32) ((f64) travelDistance * sin(((3.141592653589793) * (f64) angle) / (180.0)));
            HuSprPosSet(lbl_1_bss_46C, 0, lbl_1_bss_4E4, lbl_1_bss_4E0 + sineOffset);
            angle += (3.0f);
        } while (!(angle >= (90.0f)));
        lbl_1_bss_4E0 = (-152.0f);
        HuSprPosSet(lbl_1_bss_46C, 0, lbl_1_bss_4E4, lbl_1_bss_4E0);
        HuSprAttrSet(lbl_1_bss_46C, 0, HUSPR_ATTR_DISPOFF);
        lbl_1_bss_4DC = 0;
    }
}

/* Start a message at the requested speed in one of the dialogue windows. */
void fn_1_20A50(u32 messageId, s16 messageSpeed, u8 windowIndex) {
    s16 window = fn_1_206FC(windowIndex);

    HuWinAttrSet(window, HUWIN_ATTR_NOCANCEL);
    HuWinMesSet(window, messageId);
    HuWinMesSpeedSet(window, messageSpeed);
}

/* Close the active dialogue window between quiz exchanges. */
void fn_1_20B24(void)
{
    if (lbl_1_bss_37A != -1) {
        HuWinExClose(lbl_1_bss_37A);
        lbl_1_bss_37A = -1;
    }
}

/* Open the requested dialogue window, closing the previous speaker first. */
void fn_1_20B74(s32 windowNo) {
    s16 window = fn_1_206FC(windowNo);

    if (lbl_1_bss_37A != window) {
        if (lbl_1_bss_37A != -1) {
            fn_1_20B24();
        }
        HuWinExOpen(window);
        lbl_1_bss_37A = window;
    }
}

/* Wait for the active dialogue message before the round process continues. */
void fn_1_20C94(void)
{
    if (lbl_1_bss_37A != -1) {
        HuWinMesWait(lbl_1_bss_37A);
    }
}

/* Read a choice from the active dialogue window. */
short fn_1_20CD4(short choice) {
    return HuWinChoiceGet(lbl_1_bss_37A,choice);
}

/* Insert a message into a numbered slot in the selected dialogue window. */
void fn_1_20D08(s32 windowNo, u32 messageId, s16 insertNo) {
    HuWinInsertMesSet(fn_1_206FC(windowNo), messageId, insertNo);
}

/* Clear the short response hint line before its next message. */
void fn_1_20DBC(void) {
    HuWinHomeClear(lbl_1_bss_37C);
}

/* Clear the controller instruction line between response states. */
void fn_1_20DE8(void) {
    HuWinHomeClear(lbl_1_bss_37E);
}

/* Show or hide the controller instruction line during quiz transitions. */
void fn_1_20E14(s32 visible) {
    s32 unused;
    if (visible)
    {
        HuWinDispOn(lbl_1_bss_37E);
        return;
    }
    HuWinDispOff(lbl_1_bss_37E);
}

/* Change the controller instruction message only when its response state changes. */
void fn_1_20E64(u32 messageType) {
    u32 messageId;

    if (lbl_1_data_1F4 != (s32) messageType) {
        switch (messageType) {
        case 0:
            messageId = MESSNUM(MESS_SYS_GUIDE, 23);
            break;
        case 1:
            messageId = MESSNUM(MESS_SYS_GUIDE, 24);
            break;
        case 2:
            messageId = MESSNUM(MESS_SYS_GUIDE, 25);
            break;
        case 3:
            messageId = MESSNUM(MESS_SYS_GUIDE, 26);
            break;
        case 4:
            messageId = MESSNUM(MESS_SYS_GUIDE, 27);
            break;
        case 7:
            messageId = MESSNUM(MESS_SYS_GUIDE, 30);
            break;
        case 8:
            messageId = MESSNUM(MESS_SYS_GUIDE, 31);
            break;
        case 9:
            messageId = MESSNUM(MESS_SYS_GUIDE, 30);
            break;
        case 10:
            messageId = MESSNUM(MESS_SYS_GUIDE, 31);
            break;
        }
        HuWinMesSet(lbl_1_bss_37E, messageId);
        lbl_1_data_1F4 = messageType;
    }
}

/* Set the short response hint for the requested answer state. */
void fn_1_20F50(s32 messageType) {
    s32 unused;
    s32 messageId;
    switch (messageType)
    {
    case 5 :
        messageId = MESSNUM(MESS_SYS_GUIDE, 28);
        break;
    case 6 :
        messageId = MESSNUM(MESS_SYS_GUIDE, 29);
        break;
    }
    HuWinMesSet(lbl_1_bss_37C, messageId);
}

/* Draw and scroll the tiled background used while the quiz is paused. */
void fn_1_20FBC(s16 layerNo)
{
    Mtx44 projection;
    Mtx textureMatrix;

    if (lbl_1_bss_14) {
        GXInvalidateTexAll();
        C_MTXOrtho(projection, 0.0f, 480.0f,
                   0.0f, 576.0f, 0.0f, 10.0f);
        GXSetProjection(projection, GX_ORTHOGRAPHIC);

        if (RenderMode->field_rendering != 0) {
            GXSetViewportJitter(0.0f, 0.0f, 640.0f, 480.0f,
                                0.0f, 1.0f, VIGetNextField());
        } else {
            GXSetViewport(0.0f, 0.0f, 640.0f, 480.0f,
                          0.0f, 1.0f);
        }

        GXSetScissor(0, 0, 640, 480);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetCullMode(GX_CULL_NONE);
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                       GX_LO_NOOP);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0,
                      GX_DF_CLAMP, GX_AF_SPOT);
        GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0,
                      GX_DF_CLAMP, GX_AF_SPOT);

        PSMTXIdentity(textureMatrix);
        GXLoadPosMtxImm(textureMatrix, GX_PNMTX0);
        PSMTXTrans(textureMatrix, -lbl_1_bss_18, -lbl_1_bss_18, 0.0f);
        GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX2x4);
        HuSprTexLoad(lbl_1_bss_14, 0, GX_TEXMAP0,
                     GX_REPEAT, GX_REPEAT, GX_LINEAR);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                          GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0,
                      GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                        GX_CC_TEXC);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                        GX_CA_TEXA);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumTexGens(1);
        GXSetNumTevStages(1);

        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition2f32(0.0f, 0.0f);
        GXTexCoord2f32(0.0f, 0.0f);
        GXPosition2f32(576.0f, 0.0f);
        GXTexCoord2f32(36.0f, 0.0f);
        GXPosition2f32(576.0f, 480.0f);
        GXTexCoord2f32(36.0f, 30.0f);
        GXPosition2f32(0.0f, 480.0f);
        GXTexCoord2f32(0.0f, 30.0f);

        lbl_1_bss_18 += 0.02f;
        if (lbl_1_bss_18 >= 0.999f) {
            lbl_1_bss_18 = 0.0f;
        }
    }
}

/* Start pause handling and remember the active camera and quiz parent process. */
void fn_1_214C0(HUPROCESS *parentProcess) {
    f32 fieldOfView;
    f32 nearPlane;
    f32 farPlane;
    s32 cameraIndex;
    s32 activeCameraBit;

    lbl_1_bss_3A4 = HuPrcChildCreate(fn_1_215A4, -536, 8192, 0, HuPrcCurrentGet());
    HuPrcSetStat(lbl_1_bss_3A4, HU_PRC_STAT_PAUSE_ON | HU_PRC_STAT_UPAUSE_ON);
    fn_1_31910(0);
    cameraIndex = 0;
    /* When several cameras are active, the last one receives the pause background. */
    while (cameraIndex < 16) {
        Hu3DCameraPerspectiveGet(1 << cameraIndex, &fieldOfView, &nearPlane, &farPlane);
        if ((-1.0f) != fieldOfView) {
            activeCameraBit = 1 << cameraIndex;
        }
        cameraIndex += 1;
    }
    lbl_1_bss_43C = activeCameraBit;
    lbl_1_bss_548 = parentProcess;
}

/* Watch for Start during play, then run the pause menu and resume or leave the quiz. */
void fn_1_215A4(void)
{
    u8 playerIndex;
    u8 selectedPlayer;
    u8 disabledPlayers;
    s16 windowNo;
    u32 homeMessageId;
    u32 confirmMessageId;

    for (;;) {
        s32 playerSelected = 0;
        for (;;) {
            HuPrcVSleep();
            if (WipeCheck() != 0) continue;
            if (omCurrentOvlGet() == DLL_NONE) continue;
            if (GameMesStatGet(-1) != 0) continue;
            if ((s16)HuMCSelModeGet() == 1) continue;
            if (omUPauseFlag + omPauseChk() != 0) continue;
            if (lbl_1_bss_4D8->pauseAllowed == 0) continue;

            for (playerIndex = 0; playerIndex < 4U; playerIndex++) {
                if (omPadErrChk(playerIndex) == 0 &&
                    (HuPadBtnDown[playerIndex] & PAD_BUTTON_START) != 0) {
                    selectedPlayer = playerIndex;
                    lbl_1_bss_4D8->pauseDisplayActive = 1;
                    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 10);
                    while (WipeCheck() != 0) HuPrcVSleep();
                    HuWinDispOff(lbl_1_bss_37E);
                    fn_1_21A98();
                    HuWinHomeClear(lbl_1_bss_37C);
                    homeMessageId = MESSNUM(MESS_SYS_GUIDE, 28);
                    HuWinMesSet(lbl_1_bss_37C, homeMessageId);
                    playerSelected = 1;
                    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 10);
                    while (WipeCheck() != 0) HuPrcVSleep();
                    break;
                }
            }
            if (playerSelected != 0) break;
        }

        for (;;) {
            HuPrcVSleep();
            if ((HuPadBtnDown[selectedPlayer] & PAD_BUTTON_START) != 0) {
                HuAudFXPlay(MSM_SE_CMN_06);
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 10);
                while (WipeCheck() != 0) HuPrcVSleep();
                fn_1_21B80();
                HuWinHomeClear(lbl_1_bss_37C);
                HuWinDispOn(lbl_1_bss_37E);
                lbl_1_bss_4D8->pauseDisplayActive = 0;
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 10);
                while (WipeCheck() != 0) HuPrcVSleep();
                break;
            }
            if ((HuPadBtnDown[selectedPlayer] & PAD_BUTTON_X) == 0) continue;

            HuAudFXPlay(MSM_SE_CMN_04);
            HuWinPosSet(lbl_1_bss_37C, 16.0f, 312.0f);
            disabledPlayers = 0;
            for (playerIndex = 0; playerIndex < 4U; playerIndex++) {
                if (playerIndex != selectedPlayer) disabledPlayers |= 1 << playerIndex;
            }
            if (lbl_1_bss_4D8->presenterLayout == 0) windowNo = lbl_1_bss_3AA;
            else windowNo = lbl_1_bss_3A8;
            HuWinDisablePlayerSet(windowNo, disabledPlayers);
            confirmMessageId = MESSNUM(MESS_SYS_GUIDE, 29);
            HuWinMesSet(lbl_1_bss_37C, confirmMessageId);
            HuWinExOpen(windowNo);
            HuWinAttrSet(windowNo, HUWIN_ATTR_NOCANCEL);
            if (lbl_1_bss_4D8->presenterLayout == 0)
                HuWinMesSet(windowNo, MESSNUM(MESS_MICQUIZ, 47));
            else HuWinMesSet(windowNo, MESSNUM(MESS_MICQUIZ, 101));
            HuWinMesSpeedSet(windowNo, 1);
            HuWinMesWait(windowNo);
            if (HuWinChoiceGet(windowNo, -1) == 0) {
                HuAudFXPlay(MSM_SE_CMN_06);
                WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 10);
                while (WipeCheck() != 0) HuPrcVSleep();
                HuWinExClose(windowNo);
                HuWinHomeClear(lbl_1_bss_37C);
                fn_1_21B80();
                HuWinDispOn(lbl_1_bss_37E);
                lbl_1_bss_4D8->pauseDisplayActive = 0;
                HuWinDisablePlayerReset(windowNo, 15);
                WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 10);
                while (WipeCheck() != 0) HuPrcVSleep();
                break;
            }

            HuAudSStreamFadeOut(lbl_1_bss_4E8, 600);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            while (WipeCheck() != 0) HuPrcVSleep();
            fn_1_21B80();
            HuMCClose();
            HuMCSessionClose();
            HuWinDisablePlayerReset(windowNo, 15);
            if (lbl_1_bss_4D8->compactPauseDisplay != 0) omOvlReturnEx(1, 1);
            else omOvlReturnEx(2, 1);
        }
    }
}

/* Save pause state, show the pause display, and suspend quiz animation and audio. */
void fn_1_21A98(void) {
    lbl_1_bss_39C = (s32) omSysPauseEnableFlag;
    lbl_1_bss_3A0 = (s32) omPauseChk();
    omUPauseFlag = 1;
    omSysPauseEnable(0U);
    Hu3DCameraLayerHookSet((s16) lbl_1_bss_43C, 15, fn_1_20FBC);
    if (lbl_1_bss_4D8->compactPauseDisplay != 0) {
        fn_1_265E4();
        fn_1_25B34();
    } else {
        fn_1_26224();
    }
    fn_1_25C88();
    omObjManPause(1);
    HuPrcAllPause(1);
    Hu3DPauseSet(1);
    HuSprPauseSet(1);
    HuPadRumbleAllStop();
    HuAudFXPauseAll(1);
    HuAudSStreamPauseAll(1);
    HuAudFXPlay(MSM_SE_CMN_06);
}

/* Restore the quiz display and the pause state saved when the menu opened. */
void fn_1_21B80(void) {
    omUPauseFlag = 0;
    omSysPauseEnable((u8) lbl_1_bss_39C);
    omObjManPause(0);
    Hu3DCameraLayerHookReset((s16) lbl_1_bss_43C, 15);
    if (lbl_1_bss_4D8->compactPauseDisplay != 0) {
        fn_1_26528();
    } else {
        fn_1_25F20();
    }
    fn_1_25DDC();
    /* Keep animation and audio paused if they were already paused before the menu. */
    if ((s32) lbl_1_bss_3A0 == 0) {
        HuPrcAllPause(0);
        Hu3DPauseSet(0);
        HuSprPauseSet(0);
        HuAudFXPauseAll(0);
        HuAudSStreamPauseAll(0);
    }
}

/* Build the question HUD, event decorations, feedback icons, and dialogue windows. */
void fn_1_21C44(void) {
    u8 unusedByteA;
    u8 unusedByteB;
    u8 unusedByteC;
    u8 unusedByteD;
    u8 unusedByteE;
    u8 unusedByteF;
    u8 unusedByteG;
    u8 unusedByteH;
    s32 eventType;
    s32 unused;
    s32 playerIndex;
    s32 character;
    u8 unusedByte;

    lbl_1_bss_4D8 = (MicquizModeWorkView *)MgModeWork;
lbl_1_bss_5CC = HuSprGrpCreate(8);
    HuSprGrpPosSet(lbl_1_bss_5CC, 493.0f, 386.0f);
playerIndex = 0;
    while (playerIndex < 4) {
        if (fn_1_27FEC(playerIndex) == 0) {
            lbl_1_bss_450[playerIndex] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_mgconst, 0), HU_MEMNUM_OVL, HEAP_MODEL));
            HuSprGrpMemberSet(lbl_1_bss_5CC, (s16) (playerIndex + 4),
                              HuSprCreate(lbl_1_bss_450[playerIndex], 48, 0));
            HuSprPosSet(lbl_1_bss_5CC, (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_5CC, (s16) (playerIndex + 4), HUSPR_ATTR_DISPOFF);
        } else {
            character = GwPlayerConf[playerIndex].charNo;
            lbl_1_bss_450[playerIndex] = HuSprAnimRead(
                HuDataSelHeapReadNum(lbl_1_data_1C[character], HU_MEMNUM_OVL, HEAP_MODEL));
            HuSprGrpMemberSet(lbl_1_bss_5CC, (s16) (playerIndex + 4),
                              HuSprCreate(lbl_1_bss_450[playerIndex], 48, 0));
            HuSprPosSet(lbl_1_bss_5CC, (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_5CC, (s16) (playerIndex + 4), HUSPR_ATTR_DISPOFF);
        }
        playerIndex += 1;
    }
    lbl_1_bss_44C =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 28), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_5CC, 0, HuSprCreate(lbl_1_bss_44C, 49, 0));
    HuSprPosSet(lbl_1_bss_5CC, 0, 0.0f, 0.0f);
    lbl_1_bss_448 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 29), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_5CC, 1, HuSprCreate(lbl_1_bss_448, 49, 0));
    HuSprPosSet(lbl_1_bss_5CC, 1, 8.0f, 12.0f);
    lbl_1_bss_444 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 30), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_5CC, 2, HuSprCreate(lbl_1_bss_444, 49, 0));
    HuSprPosSet(lbl_1_bss_5CC, 2, 13.0f, 9.0f);
    lbl_1_bss_440 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 32), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_5CC, 3, HuSprCreate(lbl_1_bss_440, 49, 0));
    HuSprPosSet(lbl_1_bss_5CC, 3, -23.0f, -25.0f);
    lbl_1_bss_490 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 36), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_46A = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_46A, 0, HuSprCreate(lbl_1_bss_490, 50, 0));
    HuSprPosSet(lbl_1_bss_46A, 0, 288.0f, 68.0f);
    HuSprAttrSet(lbl_1_bss_46A, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_48C =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 37), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_468 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_468, 0, HuSprCreate(lbl_1_bss_48C, 50, 0));
    HuSprPosSet(lbl_1_bss_468, 0, 288.0f, 68.0f);
    HuSprBankSet(lbl_1_bss_468, 0, 1);
    lbl_1_bss_488 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 38), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_466 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_466, 0, HuSprCreate(lbl_1_bss_488, 50, 0));
    HuSprPosSet(lbl_1_bss_466, 0, 288.0f, 68.0f);
    HuSprTPLvlSet(lbl_1_bss_466, 0, 0.0f);
    lbl_1_bss_460[0] = HuSprGrpCreate(2);
    lbl_1_bss_480[0] =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 86), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_460[0], 0, HuSprCreate(lbl_1_bss_480[0], 50, 0));
    HuSprPosSet(lbl_1_bss_460[0], 0, -16.0f, 0.0f);
    HuSprBankSet(lbl_1_bss_460[0], 0, 0);
    lbl_1_bss_480[1] =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 86), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_460[0], 1, HuSprCreate(lbl_1_bss_480[1], 50, 0));
    HuSprPosSet(lbl_1_bss_460[0], 1, 16.0f, 0.0f);
    HuSprBankSet(lbl_1_bss_460[0], 1, 0);
    HuSprGrpAttrSet(lbl_1_bss_460[0], HUSPR_ATTR_DISPOFF);
    HuSprGrpPosSet(lbl_1_bss_460[0], 271.0f, 68.0f);
    eventType = fn_1_281D4();
    if (eventType != 8) {
        lbl_1_bss_4D4 = HuSprAnimRead(
            HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 21), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_47E = HuSprGrpCreate(1);
        HuSprGrpMemberSet(lbl_1_bss_47E, 0, HuSprCreate(lbl_1_bss_4D4, 50, 0));
        HuSprPosSet(lbl_1_bss_47E, 0, 401.0f, 62.0f);
        HuSprBankSet(lbl_1_bss_47E, 0, (s16) (eventType + 1));
        if (eventType == 3) {
            lbl_1_bss_4CC[0] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 54), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_4CC[1] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 55), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_4AC = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 56), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_4A8 = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_bss_4A8, 0, HuSprCreate(lbl_1_bss_4AC, 49, 0));
            HuSprPosSet(lbl_1_bss_4A8, 0, 401.0f, 62.0f);
            HuSprAttrSet(lbl_1_bss_4A8, 0, HUSPR_ATTR_DISPOFF);
        } else if (eventType == 2) {
            lbl_1_bss_4C8 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 57), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_data_20A = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_20A, 0, HuSprCreate(lbl_1_bss_4C8, 51, 0));
            HuSprPosSet(lbl_1_data_20A, 0, 401.0f, 62.0f);
            lbl_1_bss_340[0] = HuPrcChildCreate(fn_1_244A4, 100U, 6144U, 0, HuPrcCurrentGet());
            lbl_1_bss_340[0]->property = (void *) lbl_1_data_20A;
        } else if (eventType == 4) {
            lbl_1_bss_4C4 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 58), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_data_20C = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_20C, 0, HuSprCreate(lbl_1_bss_4C4, 51, 0));
            HuSprPosSet(lbl_1_data_20C, 0, 401.0f, 62.0f);
            lbl_1_bss_340[0] = HuPrcChildCreate(fn_1_244A4, 100U, 6144U, 0, HuPrcCurrentGet());
            lbl_1_bss_340[0]->property = (void *) lbl_1_data_20C;
            lbl_1_bss_4C0 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 59), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_data_20E = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_20E, 0, HuSprCreate(lbl_1_bss_4C0, 51, 0));
            HuSprPosSet(lbl_1_data_20E, 0, 288.0f, 68.0f);
            lbl_1_bss_4BC = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 60), HU_MEMNUM_OVL, HEAP_MODEL));
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                lbl_1_data_210[playerIndex] = HuSprGrpCreate(1);
                HuSprGrpMemberSet(lbl_1_data_210[playerIndex], 0,
                                  HuSprCreate(lbl_1_bss_4BC, 52, 0));
                HuSprPosSet(lbl_1_data_210[playerIndex], 0, lbl_1_data_21C[playerIndex].positionX,
                            lbl_1_data_21C[playerIndex].positionY);
                HuSprAttrSet(lbl_1_data_210[playerIndex], 0, HUSPR_ATTR_NOANIM);
                lbl_1_bss_340[playerIndex + 2] =
                    HuPrcChildCreate(fn_1_24138, 100U, 6144U, 0, HuPrcCurrentGet());
                lbl_1_bss_340[playerIndex + 2]->property = (void *) lbl_1_data_210[playerIndex];
            }
            lbl_1_bss_4B8 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 61), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_data_218 = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_218, 0, HuSprCreate(lbl_1_bss_4B8, 53, 0));
            HuSprPosSet(lbl_1_data_218, 0, 248.0f, 52.0f);
            HuPrcChildCreate(fn_1_244A4, 100U, 6144U, 0, HuPrcCurrentGet());
            lbl_1_bss_340[0] = HuPrcChildCreate(fn_1_23F20, 100U, 6144U, 0, HuPrcCurrentGet());
            lbl_1_bss_340[0]->property = (void *) lbl_1_data_218;
            lbl_1_bss_4B4 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 62), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_data_21A = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_21A, 0, HuSprCreate(lbl_1_bss_4B4, 54, 0));
            HuSprPosSet(lbl_1_data_21A, 0, 328.0f, 84.0f);
            lbl_1_bss_340[1] = HuPrcChildCreate(fn_1_23F20, 100U, 6144U, 0, HuPrcCurrentGet());
            lbl_1_bss_340[1]->property = (void *) lbl_1_data_21A;
        } else if (eventType == 5 || eventType == 7) {
            lbl_1_bss_4C4 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 58), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_data_20C = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_20C, 0, HuSprCreate(lbl_1_bss_4C4, 51, 0));
            HuSprPosSet(lbl_1_data_20C, 0, 401.0f, 62.0f);
            lbl_1_bss_340[0] = HuPrcChildCreate(fn_1_244A4, 100U, 6144U, 0, HuPrcCurrentGet());
            lbl_1_bss_340[0]->property = (void *) lbl_1_data_20C;
        } else if (eventType == 0 || eventType == 1) {
            lbl_1_bss_4B0 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 53), HU_MEMNUM_OVL, HEAP_MODEL));
        } else if (eventType == 6) {
            lbl_1_bss_4B0 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 63), HU_MEMNUM_OVL, HEAP_MODEL));
        }
    }
    lbl_1_bss_4A4 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 33), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_474 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_474, 0, HuSprCreate(lbl_1_bss_4A4, 50, 0));
    HuSprPosSet(lbl_1_bss_474, 0, 506.0f, 65.0f);
    HuSprBankSet(lbl_1_bss_474, 0, 0);
    lbl_1_bss_4A0 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 45), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_472 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_472, 0, HuSprCreate(lbl_1_bss_4A0, 50, 0));
    HuSprPosSet(lbl_1_bss_472, 0, 504.0f, 99.0f);
    HuSprBankSet(lbl_1_bss_472, 0, 0);
    lbl_1_bss_49C =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 46), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_470 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_470, 0, HuSprCreate(lbl_1_bss_49C, 50, 0));
    HuSprPosSet(lbl_1_bss_470, 0, 504.0f, 99.0f);
    HuSprBankSet(lbl_1_bss_470, 0, 0);
    HuSprGrpAttrSet(lbl_1_bss_470, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_498 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 44), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_46E = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_46E, 0, HuSprCreate(lbl_1_bss_498, 50, 0));
    HuSprPosSet(lbl_1_bss_46E, 0, 102.0f, 67.0f);
    HuSprBankSet(lbl_1_bss_46E, 0, (s16) lbl_1_bss_4D8->questionColumn);
    lbl_1_bss_398 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 42), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_390 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_390, 0, HuSprCreate(lbl_1_bss_398, 50, 0));
    HuSprPosSet(lbl_1_bss_390, 0, 292.0f, 208.0f);
    HuSprAttrSet(lbl_1_bss_390, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_394 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 43), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_38E = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_38E, 0, HuSprCreate(lbl_1_bss_394, 50, 0));
    HuSprPosSet(lbl_1_bss_38E, 0, 292.0f, 208.0f);
    HuSprAttrSet(lbl_1_bss_38E, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_494 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 103), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_46C = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_46C, 0, HuSprCreate(lbl_1_bss_494, 1500, 0));
    lbl_1_bss_4E4 = 285.0f;
    lbl_1_bss_4E0 = -152.0f;
    HuSprPosSet(lbl_1_bss_46C, 0, lbl_1_bss_4E4, lbl_1_bss_4E0);
    HuSprAttrSet(lbl_1_bss_46C, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_4DC = 0;
    fn_1_2029C();
    fn_1_25230();
    fn_1_2D748(0);
}

/* Build the compact microphone HUD, adding inactive placeholders until the first active
 * player's portrait, then stopping the portrait loop. */
void fn_1_23748(void) {
    s32 characterNo;
    s32 playerNo;

    lbl_1_bss_4D8 = (MicquizModeWorkView *)MgModeWork;
    lbl_1_bss_5CC = HuSprGrpCreate(8);
    HuSprGrpPosSet(lbl_1_bss_5CC, 493.0f, 386.0f);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (fn_1_27FEC(playerNo) == 0) {
            lbl_1_bss_450[playerNo] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_mgconst, 0), HU_MEMNUM_OVL, HEAP_MODEL));
            HuSprGrpMemberSet(lbl_1_bss_5CC, (s16) (playerNo + 4),
                              HuSprCreate(lbl_1_bss_450[playerNo], 48, 0));
            HuSprPosSet(lbl_1_bss_5CC, (s16) (playerNo + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_5CC, (s16) (playerNo + 4), HUSPR_ATTR_DISPOFF);
        } else {
        characterNo = GwPlayerConf[playerNo].charNo;
        lbl_1_bss_450[playerNo] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_1C[characterNo], HU_MEMNUM_OVL, HEAP_MODEL));
        HuSprGrpMemberSet(lbl_1_bss_5CC, (s16) (playerNo + 4),
                          HuSprCreate(lbl_1_bss_450[playerNo], 48, 0));
        HuSprPosSet(lbl_1_bss_5CC, (s16) (playerNo + 4), -23.0f, -25.0f);
            break;
        }
    }
    lbl_1_bss_44C =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 28), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_5CC, 0, HuSprCreate(lbl_1_bss_44C, 49, 1));
    HuSprPosSet(lbl_1_bss_5CC, 0, 0.0f, 0.0f);
    lbl_1_bss_448 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 29), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_5CC, 1, HuSprCreate(lbl_1_bss_448, 49, 1));
    HuSprPosSet(lbl_1_bss_5CC, 1, 8.0f, 12.0f);
    lbl_1_bss_444 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 30), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_5CC, 2, HuSprCreate(lbl_1_bss_444, 49, 0));
    HuSprPosSet(lbl_1_bss_5CC, 2, 13.0f, 9.0f);
    fn_1_2029C();
    fn_1_25230();
}

/* Continuously pulse the child process sprite with a randomized starting phase. */
void fn_1_23F20(void) {
    HUPROCESS *process;
    s16 sprite;
    f32 angle;
    f32 randomAngle;
    f32 scale;
    f32 transparency;

    process = HuPrcCurrentGet();
    sprite = (s16)process->property;
    angle = (f32)(u32)frandmod(360);
    /* Advance the random stream again; this second phase is unused. */
    randomAngle = (f32)(u32)frandmod(360);
    HuSprTPLvlSet(sprite, 0,
        (f32)(0.5 +
        (0.5 * sin((3.141592653589793 * (f64)angle) /
        180.0))));
    HuSprScaleSet(sprite, 0,
        (f32)(1.0 +
        (0.5 * sin((3.141592653589793 * (f64)angle) /
        180.0))),
        1.0f);
    for (;;) {
        HuPrcVSleep();
        scale = (f32)(1.0 +
            (0.5 * sin((3.141592653589793 * (f64)angle) /
            180.0)));
        transparency = (f32)(0.5 +
            (0.5 * sin((3.141592653589793 * (f64)angle) /
            180.0)));
        HuSprTPLvlSet(sprite, 0,
            transparency);
        HuSprScaleSet(sprite, 0, scale, 1.0f);
        angle += 0.75f;
    }
}

/* Show, scale, and hide a decorative sprite repeatedly after random delays. */
void fn_1_24138(void) {
    f32 scale;
    f32 angle;
    s32 initialDelay;
    s32 repeatDelay;
    HUPROCESS *currentProcess;
    s16 groupId;

    currentProcess = HuPrcCurrentGet();
    groupId = (s16) currentProcess->property;
    HuSprAttrSet(groupId, 0, HUSPR_ATTR_DISPOFF);
    HuSprScaleSet(groupId, 0, (0.0f), (0.0f));
    initialDelay = frandmod(60) + 60;
    repeatDelay = frandmod(120) + 60;
    HuPrcSleep(initialDelay);
    angle = (0.0f);
showAgain:
    HuSprAttrReset(groupId, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(groupId, 0, HUSPR_ATTR_NOANIM);
    do {
        HuPrcVSleep();
        scale = (f32) sin(((3.141592653589793) * (f64) angle) / (180.0));
        HuSprScaleSet(groupId, 0, scale, scale);
        angle += (4.285714f);
    } while (!(angle >= (90.0f)));
    angle = (0.0f);
    HuSprAttrSet(groupId, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(groupId, 0, HUSPR_ATTR_NOANIM);
    HuPrcSleep(repeatDelay);
    goto showAgain;
}

/* Animate one short spark around the question event icon, then remove it. */
void fn_1_24294(void) {
    f32 randomAngle;
    f32 randomAmplitude;
    s32 delay;
    s16 spriteGroup;
    f32 x;
    f32 y;
    f32 angle;
    s32 frame;
    f32 transparency;

    randomAngle = (2.0f) * frandf();
    randomAmplitude = frandf();
    delay = frandmod(10);
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(lbl_1_bss_4B0, 51, 0));
    HuSprAttrSet(spriteGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(spriteGroup, 0, HUSPR_ATTR_NOANIM);
    HuPrcSleep(delay);
    x = (f32)((401.0) +
        ((40.0) * ((f64)randomAmplitude *
        sin((3.141592653589793) * (f64)randomAngle))));
    y = (f32)((62.0) +
        ((40.0) * ((f64)randomAmplitude *
        cos((3.141592653589793) * (f64)randomAngle))));
    HuSprPosSet(spriteGroup, 0, x, y);
    HuSprAttrReset(spriteGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(spriteGroup, 0, HUSPR_ATTR_NOANIM);

    angle = (0.0f);
    for (frame = 0; frame < 60; frame++) {
        HuPrcVSleep();
        transparency = (f32)((1.0) -
            sin(((3.141592653589793) * (f64)angle) / (180.0)));
        angle += (1.5f);
        if (frame <= 50) {
            HuSprTPLvlSet(spriteGroup, 0, transparency);
        }
    }
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Continuously pulse the event sprite assigned to this child process. */
void fn_1_244A4(void) {
    f32 scale;
    f32 transparency;
    f32 angle;
    HUPROCESS *process;
    s16 spriteGroup;

    /* angle is not initialized on entry. */
    process = HuPrcCurrentGet();
    spriteGroup = (s16) process->property;
animateFrame:
    HuPrcVSleep();
    scale =
        (f32) ((1.0) + ((0.30000001192092896) * sin(((3.141592653589793) * angle) / (180.0))));
    transparency = (f32) ((1.0) - sin(((3.141592653589793) * angle) / (180.0)));
    HuSprTPLvlSet(spriteGroup, 0, transparency);
    HuSprScaleSet(spriteGroup, 0, scale, scale);
    angle += (1.5f);
    if (!(angle >= (90.0f))) {
        goto animateFrame;
    }
    angle = (0.0f);
    goto animateFrame;
}

/* Create the microphone HUD and player portraits beneath the current quiz process. */
void fn_1_245C0(void) {
    s32 characterNo;
    s32 playerIndex;

    lbl_1_bss_548 = HuPrcCurrentGet();
    lbl_1_bss_4D8 = (MicquizModeWorkView *)MgModeWork;
    lbl_1_bss_5CC = HuSprGrpCreate(8);
    HuSprGrpPosSet(lbl_1_bss_5CC, 493.0f, 386.0f);
    playerIndex = 0;
    while (playerIndex < 4) {
        if (fn_1_27FEC(playerIndex) == 0) {
            lbl_1_bss_450[playerIndex] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_mgconst, 0), HU_MEMNUM_OVL, HEAP_MODEL));
            HuSprGrpMemberSet(lbl_1_bss_5CC, (s16) (playerIndex + 4),
                              HuSprCreate(lbl_1_bss_450[playerIndex], 48, 0));
            HuSprPosSet(lbl_1_bss_5CC, (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_5CC, (s16) (playerIndex + 4), HUSPR_ATTR_DISPOFF);
        } else {
            characterNo = GwPlayerConf[playerIndex].charNo;
            lbl_1_bss_450[playerIndex] = HuSprAnimRead(
                HuDataSelHeapReadNum(lbl_1_data_1C[characterNo], HU_MEMNUM_OVL, HEAP_MODEL));
            HuSprGrpMemberSet(lbl_1_bss_5CC, (s16) (playerIndex + 4),
                              HuSprCreate(lbl_1_bss_450[playerIndex], 48, 0));
            HuSprPosSet(lbl_1_bss_5CC, (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_5CC, (s16) (playerIndex + 4), HUSPR_ATTR_DISPOFF);
        }
        playerIndex += 1;
    }
    lbl_1_bss_44C =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 28), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_5CC, 0, HuSprCreate(lbl_1_bss_44C, 49, 0));
    HuSprPosSet(lbl_1_bss_5CC, 0, 0.0f, 0.0f);
    lbl_1_bss_448 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 29), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_5CC, 1, HuSprCreate(lbl_1_bss_448, 49, 0));
    HuSprPosSet(lbl_1_bss_5CC, 1, 8.0f, 12.0f);
    lbl_1_bss_444 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 30), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_5CC, 2, HuSprCreate(lbl_1_bss_444, 49, 0));
    HuSprPosSet(lbl_1_bss_5CC, 2, 13.0f, 9.0f);
    lbl_1_bss_440 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 32), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_5CC, 3, HuSprCreate(lbl_1_bss_440, 49, 0));
    HuSprPosSet(lbl_1_bss_5CC, 3, -23.0f, -23.0f);
    HuSprAttrSet(lbl_1_bss_5CC, 2, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_5CC, 0, 0);
    HuSprBankSet(lbl_1_bss_5CC, 1, 0);
    HuSprGrpAttrSet(lbl_1_bss_5CC, HUSPR_ATTR_DISPOFF);
}

/* Reveal the selected player portrait and bounce the microphone HUD into view. */
void fn_1_24A54(s32 playerIndex) {
    f32 scale;
    f32 angle;
    s32 frame;

    scale = (0.0f);
    angle = (0.0f);
    HuSprGrpAttrReset(lbl_1_bss_5CC, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(lbl_1_bss_5CC, 2, HUSPR_ATTR_DISPOFF);
    for (frame = 0; frame < 4; frame++) {
        HuSprAttrSet(lbl_1_bss_5CC, (s16)(frame + 4), HUSPR_ATTR_DISPOFF);
    }
    HuSprAttrSet(lbl_1_bss_5CC, 3, HUSPR_ATTR_DISPOFF);
    HuSprGrpScaleSet(lbl_1_bss_5CC, (0.0f), (0.0f));
    fn_1_31A0C(playerIndex);
    do {
        HuPrcVSleep();
        scale = (f32)((1.2000000476837158) *
                      sin(((3.141592653589793) * (f64)angle) / (180.0)));
        HuSprGrpScaleSet(lbl_1_bss_5CC, scale, scale);
        angle += (4.5f);
    } while (!(angle >= (90.0f)));
    HuSprGrpScaleSet(lbl_1_bss_5CC, (1.2f), (1.2f));

    scale = (1.2f);
    angle = (0.0f);
    do {
        HuPrcVSleep();
        scale = (f32)((1.2000000476837158) -
                      ((0.4000000059604645) *
                       sin(((3.141592653589793) * (f64)angle) / (180.0))));
        HuSprGrpScaleSet(lbl_1_bss_5CC, scale, scale);
        angle += (18.0f);
    } while (!(angle >= (90.0f)));
    HuSprGrpScaleSet(lbl_1_bss_5CC, (0.8f), (0.8f));

    scale = (1.2f);
    angle = (0.0f);
    do {
        HuPrcVSleep();
        scale = (f32)((0.800000011920929) +
                      ((0.20000000298023224) *
                       sin(((3.141592653589793) * (f64)angle) / (180.0))));
        HuSprGrpScaleSet(lbl_1_bss_5CC, scale, scale);
        angle += (18.0f);
    } while (!(angle >= (90.0f)));
    HuSprGrpScaleSet(lbl_1_bss_5CC, (1.0f), (1.0f));
}

/* Shrink the microphone HUD away at the end of its display interval. */
void fn_1_24D94(void) {
    f32 angle;
    f32 scale;

    scale = (0.0f);
    angle = (0.0f);
    HuSprGrpScaleSet(lbl_1_bss_5CC, (1.0f), (1.0f));
    do {
        HuPrcVSleep();
        scale = (f32) (1.0 - sin((3.141592653589793 * (f64) angle) / 180.0));
        HuSprGrpScaleSet(lbl_1_bss_5CC, scale, scale);
        angle += 3.0f;
    } while (!(angle >= 90.0f));
    HuSprGrpAttrSet(lbl_1_bss_5CC, HUSPR_ATTR_DISPOFF);
    HuSprGrpScaleSet(lbl_1_bss_5CC, (0.0f), (0.0f));
}

/* Create three score digits for each active player before score updates begin. */
void fn_1_24EC8(void) {
    s32 playerIndex;
    s32 digitIndex;
    s32 score;
    s32 hundreds;
    s32 tens;
    s32 ones;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_27FEC(playerIndex) != 0) {
            for (digitIndex = 0; digitIndex < 3; digitIndex++) {
                lbl_1_bss_3EC[playerIndex][digitIndex] = HuSprAnimRead(
                    HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 86), HU_MEMNUM_OVL, HEAP_MODEL));
                lbl_1_bss_3D4[playerIndex][digitIndex] = HuSprGrpCreate(1);
                HuSprGrpMemberSet(lbl_1_bss_3D4[playerIndex][digitIndex], 0,
                                  HuSprCreate(lbl_1_bss_3EC[playerIndex][digitIndex], 1500, 0));
                HuSprPosSet(lbl_1_bss_3D4[playerIndex][digitIndex], 0,
                            (f32)(64 + (playerIndex << 7) + (digitIndex << 5)),
                            (275.0f));
                HuSprAttrSet(lbl_1_bss_3D4[playerIndex][digitIndex], 0, HUSPR_ATTR_DISPOFF);
            }

            score = lbl_1_bss_4D8->playerScore[playerIndex];
            hundreds = score / 100;
            tens = (score - (hundreds * 100)) / 10;
            ones = (score - (hundreds * 100)) - (tens * 10);
            HuSprBankSet(lbl_1_bss_3D4[playerIndex][0], 0, hundreds);
            HuSprBankSet(lbl_1_bss_3D4[playerIndex][1], 0, tens);
            HuSprBankSet(lbl_1_bss_3D4[playerIndex][2], 0, ones);
        }
    }
}

/* Refresh each active player score using its three digit sprites. */
void fn_1_25120(void) {
    s32 playerIndex;
    s32 score;
    s32 hundreds;
    s32 tens;
    s32 ones;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_27FEC(playerIndex) != 0) {
            score = lbl_1_bss_4D8->playerScore[playerIndex];
            hundreds = score / 100;
            tens = (score - hundreds * 100) / 10;
            ones = score - hundreds * 100 - tens * 10;
            HuSprBankSet(lbl_1_bss_3D4[playerIndex][0], 0, hundreds);
            HuSprBankSet(lbl_1_bss_3D4[playerIndex][1], 0, tens);
            HuSprBankSet(lbl_1_bss_3D4[playerIndex][2], 0, ones);
        }
    }
}

/* Create the pause display, player score rows, and question progress indicators. */
void fn_1_25230(void) {
    s32 animationIndex;
    s32 playerNo;
    s32 digitIndex;
    s32 score;
    s32 scoreRemainder;
    s32 hundreds;
    s32 characterIndex;

    lbl_1_bss_14 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 101), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_438 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 102), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_434 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_434, 0, HuSprCreate(lbl_1_bss_438, 1500, 0));
    HuSprPosSet(lbl_1_bss_434, 0, 288.0f, 64.0f);
    HuSprAttrSet(lbl_1_bss_434, 0, HUSPR_ATTR_DISPOFF);
    animationIndex = 0;
    while (animationIndex < 4) {
        lbl_1_bss_424[animationIndex] = HuSprAnimRead(HuDataSelHeapReadNum(
            animationIndex + DATANUM(DATA_micquiz, 104), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_41C[animationIndex] = HuSprGrpCreate(1);
        HuSprGrpMemberSet(lbl_1_bss_41C[animationIndex], 0,
                          HuSprCreate(lbl_1_bss_424[animationIndex], 1500, 0));
        HuSprPosSet(lbl_1_bss_41C[animationIndex], 0, (f32)((animationIndex << 7) + 96), 248.0f);
        HuSprAttrSet(lbl_1_bss_41C[animationIndex], 0, HUSPR_ATTR_DISPOFF);
        animationIndex++;
    }
    fn_1_24EC8();
    lbl_1_bss_3AC[0] = MgScoreInit(DATANUM(DATA_micquiz, 120));
    HuPrcSetStat(lbl_1_bss_3AC[0]->proc, HU_PRC_STAT_PAUSE_ON);
    MgScoreMaxDigitSet(lbl_1_bss_3AC[0], 2);
    MgScoreValueSet(lbl_1_bss_3AC[0], lbl_1_bss_4D8->questionCount);
    MgScorePosSet(lbl_1_bss_3AC[0], 186.0f, 135.0f);
    MgScoreDispOff(lbl_1_bss_3AC[0]);
    MgScorePriSet(lbl_1_bss_3AC[0], 50);
    lbl_1_bss_3D0 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 108), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_3CC = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_3CC, 0, HuSprCreate(lbl_1_bss_3D0, 1500, 0));
    HuSprPosSet(lbl_1_bss_3CC, 0, 288.0f, 140.0f);
    HuSprAttrSet(lbl_1_bss_3CC, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_3AC[1] = MgScoreInit(DATANUM(DATA_micquiz, 120));
    HuPrcSetStat(lbl_1_bss_3AC[1]->proc, HU_PRC_STAT_PAUSE_ON);
    MgScoreMaxDigitSet(lbl_1_bss_3AC[1], 2);
    MgScoreValueSet(lbl_1_bss_3AC[1], lbl_1_bss_4D8->playerCount * 5);
    MgScorePosSet(lbl_1_bss_3AC[1], 338.0f, 145.0f);
    MgScoreDispOff(lbl_1_bss_3AC[1]);
    MgScorePriSet(lbl_1_bss_3AC[1], 50);
    animationIndex = 0;
    while (animationIndex < 4) {
        if (fn_1_27FEC(animationIndex) != 0) {
            characterIndex = lbl_1_bss_4D8->playerCharacter[animationIndex];
            lbl_1_bss_3BC[animationIndex] = HuSprAnimRead(HuDataSelHeapReadNum(
                characterIndex + DATANUM(DATA_micquiz, 109), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_3B4[animationIndex] = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_bss_3B4[animationIndex], 0,
                              HuSprCreate(lbl_1_bss_3BC[animationIndex], 1500, 0));
            HuSprPosSet(lbl_1_bss_3B4[animationIndex], 0, (f32) ((animationIndex << 7) + 95),
                        214.0f);
            HuSprAttrSet(lbl_1_bss_3B4[animationIndex], 0, HUSPR_ATTR_DISPOFF);
        }
        animationIndex++;
    }
    lbl_1_bss_3AA = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 23, 5);
    lbl_1_bss_3A8 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 13, 0);
    HuWinDispOff(lbl_1_bss_3AA);
    HuWinPriSet(lbl_1_bss_3AA, 50);
    HuWinDispOff(lbl_1_bss_3A8);
    HuWinPriSet(lbl_1_bss_3A8, 50);
    lbl_1_bss_37C = HuWinCreate(16.0f, 352.0f, 544, 128, 0);
    HuWinPosSet(lbl_1_bss_37C, 16.0f, 352.0f);
    HuWinBGTPLvlSet(lbl_1_bss_37C, 0.0f);
    HuWinMesSpeedSet(lbl_1_bss_37C, 0);
    HuWinPriSet(lbl_1_bss_37C, 50);
    HuWinAttrSet(lbl_1_bss_37C, HUWIN_ATTR_ALIGN_CENTER);
}

/* Refresh pause-display progress values and player score digits before showing it. */
void fn_1_25B34(void) {
    s32 playerIndex;
    s32 hundreds;
    s32 score;
    s32 tens;
    s32 ones;

    MgScoreValueSet(lbl_1_bss_3AC[0], lbl_1_bss_4D8->questionCount);
    MgScoreValueSet(lbl_1_bss_3AC[1], lbl_1_bss_4D8->playerCount * 5);

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_27FEC(playerIndex) != 0) {
            score = lbl_1_bss_4D8->playerScore[playerIndex];
            hundreds = score / 100;
            tens = (score - hundreds * 100) / 10;
            ones = score - hundreds * 100 - tens * 10;
            HuSprBankSet(lbl_1_bss_3D4[playerIndex][0], 0, hundreds);
            HuSprBankSet(lbl_1_bss_3D4[playerIndex][1], 0, tens);
            HuSprBankSet(lbl_1_bss_3D4[playerIndex][2], 0, ones);
        }
    }
}

/* Show the pause display and its score widgets when the menu opens. */
void fn_1_25C88(void) {
    s32 playerIndex;
    s32 digitIndex;

    HuSprAttrReset(lbl_1_bss_434, 0, HUSPR_ATTR_DISPOFF);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        HuSprAttrReset(lbl_1_bss_41C[playerIndex], 0, HUSPR_ATTR_DISPOFF);
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_27FEC(playerIndex) != 0) {
            for (digitIndex = 0; digitIndex < 3; digitIndex++) {
                HuSprAttrReset(lbl_1_bss_3D4[playerIndex][digitIndex], 0, HUSPR_ATTR_DISPOFF);
            }
        }
    }
    HuSprAttrReset(lbl_1_bss_3CC, 0, HUSPR_ATTR_DISPOFF);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_27FEC(playerIndex) != 0) {
            HuSprAttrReset(lbl_1_bss_3B4[playerIndex], 0, HUSPR_ATTR_DISPOFF);
        }
    }
    MgScoreDispOn(lbl_1_bss_3AC[0]);
    MgScoreDispOn(lbl_1_bss_3AC[1]);
}

/* Hide the pause display and its score widgets when the menu closes. */
void fn_1_25DDC(void) {
    s32 playerIndex;
    s32 digitIndex;

    HuSprAttrSet(lbl_1_bss_434, 0, HUSPR_ATTR_DISPOFF);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        HuSprAttrSet(lbl_1_bss_41C[playerIndex], 0, HUSPR_ATTR_DISPOFF);
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        for (digitIndex = 0; digitIndex < 3; digitIndex++) {
            HuSprAttrSet(lbl_1_bss_3D4[playerIndex][digitIndex], 0, HUSPR_ATTR_DISPOFF);
        }
    }
    HuSprAttrSet(lbl_1_bss_3CC, 0, HUSPR_ATTR_DISPOFF);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_27FEC(playerIndex) != 0) {
            HuSprAttrSet(lbl_1_bss_3B4[playerIndex], 0, HUSPR_ATTR_DISPOFF);
        }
    }
    MgScoreDispOff(lbl_1_bss_3AC[0]);
    MgScoreDispOff(lbl_1_bss_3AC[1]);
}

/* Return the question HUD and dialogue windows to the normal sprite draw pass. */
void fn_1_25F20(void) {
    s32 decorationIndex;

    HuSprGrpDrawNoSet(lbl_1_bss_5CC, 0);
    HuSprDrawNoSet(lbl_1_bss_46A, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_468, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_466, 0, 0);
    HuSprGrpDrawNoSet(lbl_1_bss_460[0], 0);
    if (fn_1_281D4() != 8) {
        HuSprDrawNoSet(lbl_1_bss_47E, 0, 0);
    }
    if (lbl_1_data_208 != -1) {
        HuWinDrawNoSet(lbl_1_data_208, 0);
    }
    HuWinDrawNoSet(lbl_1_bss_386, 0);
    HuWinDrawNoSet(lbl_1_bss_384, 0);
    HuWinDrawNoSet(lbl_1_bss_382, 0);
    HuWinDrawNoSet(lbl_1_bss_380, 0);
    HuSprDrawNoSet(lbl_1_bss_474, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_472, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_470, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_46E, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_390, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_38E, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_46C, 0, 0);
    if (lbl_1_data_20A != -1) {
        HuSprDrawNoSet(lbl_1_data_20A, 0, 0);
    }
    if (lbl_1_data_20C != -1) {
        HuSprDrawNoSet(lbl_1_data_20C, 0, 0);
    }
    if (lbl_1_data_20E != -1) {
        HuSprDrawNoSet(lbl_1_data_20E, 0, 0);
    }
    for (decorationIndex = 0; decorationIndex < 4; decorationIndex++) {
        if (lbl_1_data_210[decorationIndex] != -1) {
            HuSprDrawNoSet(lbl_1_data_210[decorationIndex], 0, 0);
        }
    }
    if (lbl_1_data_218 != -1) {
        HuSprDrawNoSet(lbl_1_data_218, 0, 0);
    }
    if (lbl_1_data_21A != -1) {
        HuSprDrawNoSet(lbl_1_data_21A, 0, 0);
    }
}

/* Move the question HUD and dialogue windows to the pause sprite draw pass. */
void fn_1_26224(void) {
    s32 decorationIndex;

    HuSprGrpDrawNoSet(lbl_1_bss_5CC, 64);
    if (lbl_1_data_208 != -1) {
        HuWinDrawNoSet(lbl_1_data_208, 64);
    }
    HuWinDrawNoSet(lbl_1_bss_386, 64);
    HuWinDrawNoSet(lbl_1_bss_384, 64);
    HuWinDrawNoSet(lbl_1_bss_382, 64);
    HuWinDrawNoSet(lbl_1_bss_380, 64);
    HuSprDrawNoSet(lbl_1_bss_46A, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_468, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_466, 0, 64);
    HuSprGrpDrawNoSet(lbl_1_bss_460[0], 64);
    if (fn_1_281D4() != 8) {
        HuSprDrawNoSet(lbl_1_bss_47E, 0, 64);
    }
    HuSprDrawNoSet(lbl_1_bss_474, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_472, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_470, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_46E, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_390, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_38E, 0, 64);
    HuSprDrawNoSet(lbl_1_bss_46C, 0, 64);
    if (lbl_1_data_20A != -1) {
        HuSprDrawNoSet(lbl_1_data_20A, 0, 64);
    }
    if (lbl_1_data_20C != -1) {
        HuSprDrawNoSet(lbl_1_data_20C, 0, 64);
    }
    if (lbl_1_data_20E != -1) {
        HuSprDrawNoSet(lbl_1_data_20E, 0, 64);
    }
    for (decorationIndex = 0; decorationIndex < 4; decorationIndex++) {
        if (lbl_1_data_210[decorationIndex] != -1) {
            HuSprDrawNoSet(lbl_1_data_210[decorationIndex], 0, 64);
        }
    }
    if (lbl_1_data_218 != -1) {
        HuSprDrawNoSet(lbl_1_data_218, 0, 64);
    }
    if (lbl_1_data_21A != -1) {
        HuSprDrawNoSet(lbl_1_data_21A, 0, 64);
    }
}

/* Return the compact microphone HUD and dialogue windows to the normal draw pass. */
void fn_1_26528(void) {
    HuSprGrpDrawNoSet(lbl_1_bss_4EC, 0);
    HuSprGrpDrawNoSet(lbl_1_bss_5CC, 0);
    if (lbl_1_data_208 != -1) {
        HuWinDrawNoSet(lbl_1_data_208, 0);
    }
    HuWinDrawNoSet(lbl_1_bss_386, 0);
    HuWinDrawNoSet(lbl_1_bss_384, 0);
    HuWinDrawNoSet(lbl_1_bss_382, 0);
    HuWinDrawNoSet(lbl_1_bss_380, 0);
}

/* Move the compact microphone HUD and dialogue windows to the pause draw pass. */
void fn_1_265E4(void) {
    HuSprGrpDrawNoSet(lbl_1_bss_4EC, 64);
    HuSprGrpDrawNoSet(lbl_1_bss_5CC, 64);
    if (lbl_1_data_208 != -1) {
        HuWinDrawNoSet(lbl_1_data_208, 64);
    }
    HuWinDrawNoSet(lbl_1_bss_386, 64);
    HuWinDrawNoSet(lbl_1_bss_384, 64);
    HuWinDrawNoSet(lbl_1_bss_382, 64);
    HuWinDrawNoSet(lbl_1_bss_380, 64);
}

/* Change the response indicator animation as listening starts, completes, or stops. */
void fn_1_266A0(s32 action) {
    if (action == 0) {
        if ((u32)lbl_1_bss_C != 0) {
            HuPrcKill(lbl_1_bss_C);
            lbl_1_bss_C = NULL;
        }
        HuSprAttrSet(lbl_1_bss_474, 0, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(lbl_1_bss_472, 0, HUSPR_ATTR_DISPOFF);
    } else if (action == 1) {
        HuSprAttrReset(lbl_1_bss_474, 0, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(lbl_1_bss_472, 0, HUSPR_ATTR_DISPOFF);
        if ((u32)lbl_1_bss_C == 0) {
            lbl_1_bss_C = HuPrcChildCreate(fn_1_2D22C, 100, 6144, 0, lbl_1_bss_548);
        }
    } else if (action == 3) {
        fn_1_269C0();
        HuPrcSleep(20);
        HuSprAttrSet(lbl_1_bss_474, 0, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(lbl_1_bss_474, 0, (0.0f), (0.0f));
        HuSprGrpAttrSet(lbl_1_bss_472, HUSPR_ATTR_DISPOFF);
        HuSprGrpAttrReset(lbl_1_bss_470, HUSPR_ATTR_DISPOFF);
    } else if (action == 2) {
        if ((u32)lbl_1_bss_C != 0) {
            HuPrcKill(lbl_1_bss_C);
            lbl_1_bss_C = NULL;
        }
        fn_1_26A74();
        HuPrcSleep(30);
        HuSprAttrSet(lbl_1_bss_472, 0, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(lbl_1_bss_470, 0, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(lbl_1_bss_472, 0, (0.0f), (0.0f));
    } else if (action == 4) {
        if ((u32)lbl_1_bss_C != 0) {
            HuPrcKill(lbl_1_bss_C);
            lbl_1_bss_C = NULL;
        }
        HuAudFXPlay(MSM_SE_BRD00_45);
        fn_1_269C0();
        fn_1_26A1C();
        HuPrcSleep(30);
        HuSprAttrSet(lbl_1_bss_474, 0, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(lbl_1_bss_474, 0, (0.0f), (0.0f));
        HuSprAttrSet(lbl_1_bss_472, 0, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(lbl_1_bss_472, 0, (0.0f), (0.0f));
    }
}

/* Launch twenty expanding sparks around the microphone response indicator. */
void fn_1_269C0(void) {
    s32 emissionIndex;

    emissionIndex = 0;
    while (emissionIndex < 20) {
        HuPrcChildCreate(fn_1_26ACC, 100U, 6144U, 0, lbl_1_bss_548);
        emissionIndex += 1;
    }
}

/* Launch ten fading sparks around the microphone response indicator. */
void fn_1_26A1C(void) {
    s32 emissionIndex;

    emissionIndex = 0;
    while (emissionIndex < 10) {
        HuPrcChildCreate(fn_1_26DD8, 100U, 6144U, 0, HuPrcCurrentGet());
        emissionIndex += 1;
    }
}

/* Launch ten smaller sparks around the microphone response indicator. */
void fn_1_26A74(void) {
    s32 emissionIndex;

    emissionIndex = 0;
    while (emissionIndex < 10) {
        HuPrcChildCreate(fn_1_27070, 100U, 6144U, 0, HuPrcCurrentGet());
        emissionIndex += 1;
    }
}

/* Show one expanding response spark after a random delay, then remove it. */
void fn_1_26ACC(void) {
    s16 sleep;
    ANIMDATA *animation;
    HUSPR_GROUPID group;
    f32 transparency;
    f32 step;
    f32 rotation;
    f32 scale;
    f32 position[2];

    animation = HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 34), HU_MEMNUM_OVL, 2));
    group = HuSprGrpCreate(1);
    HuSprGrpMemberSet(group, 0, HuSprCreate(animation, 50, 0));
    step = (3.0f);
    position[0] =
        (f32) (sin((u8) frand() * (1.4117647f) * (3.141592653589793) / (180.0)) * (25.0) + (506.0));
    position[1] =
        (f32) (cos((u8) frand() * (1.4117647f) * (3.141592653589793) / (180.0)) * (25.0) + (65.0));
    HuSprGrpPosSet(group, position[0], position[1]);
    HuSprScaleSet(group, 0, (0.0f), (0.0f));
    rotation = (0.0f);
    scale = (0.0f);
    transparency = (1.0f);
    sleep = frandmod(20);
    HuPrcSleep(sleep);
    for (;;) {
        if (lbl_1_bss_4D8->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(group, 64);
        } else {
            HuSprGrpDrawNoSet(group, 0);
        }
        HuPrcVSleep();
        scale = (f32)(sin((3.141592653589793) * rotation / (180.0)) * (1.5));
        transparency = (f32)((1.0) - sin((3.141592653589793) * rotation / (180.0)));
        HuSprZRotSet(group, 0, scale);
        HuSprScaleSet(group, 0, scale, scale);
        HuSprTPLvlSet(group, 0, transparency);
        rotation += step;
        if (rotation >= (90.0f)) {
            break;
        }
    }
    HuSprGrpKill(group);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Show one fading response spark over a wide random radius, then remove it. */
void fn_1_26DD8(void) {
    s16 delay;
    ANIMDATA *animation;
    s16 spriteGroup;
    f32 step;
    f32 initialPosition[2];
    f32 angle;
    f32 rotation;
    f32 transparency;

    animation =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 35), HU_MEMNUM_OVL, HEAP_MODEL));
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(animation, 50, 0));
    step = (3.0f);
    initialPosition[0] = (f32)((504.0) +
                               ((45.0) *
                                sin(((3.141592653589793) *
                                     (f64)((1.4117647f) * (f32)(u8)frand())) /
                                    (180.0))));
    initialPosition[1] = (99.0f);
    HuSprGrpPosSet(spriteGroup, initialPosition[0], initialPosition[1]);
    HuSprScaleSet(spriteGroup, 0, (0.0f), (0.0f));
    angle = (0.0f);
    rotation = (0.0f);
    transparency = (1.0f);
    delay = frandmod(20);
    HuPrcSleep(delay);
    do {
        if (lbl_1_bss_4D8->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(spriteGroup, 64);
        } else {
            HuSprGrpDrawNoSet(spriteGroup, 0);
        }
        HuPrcVSleep();
        rotation = (f32)((1.5) *
                         sin(((3.141592653589793) * (f64)angle) / (180.0)));
        transparency = (f32)((1.0) -
                             sin(((3.141592653589793) * (f64)angle) / (180.0)));
        HuSprZRotSet(spriteGroup, 0, rotation);
        HuSprScaleSet(spriteGroup, 0, rotation, rotation);
        HuSprTPLvlSet(spriteGroup, 0, transparency);
        angle += step;
    } while (!(angle >= (90.0f)));
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Show one fading response spark over a small random radius, then remove it. */
void fn_1_27070(void) {
    s16 delay;
    ANIMDATA *animation;
    s16 spriteGroup;
    f32 step;
    f32 initialPosition[2];
    f32 angle;
    f32 rotation;
    f32 transparency;

    animation =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 35), HU_MEMNUM_OVL, HEAP_MODEL));
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(animation, 50, 0));
    step = (3.0f);
    initialPosition[0] = (f32)((504.0) +
                               ((25.0) *
                                sin(((3.141592653589793) *
                                     (f64)((1.4117647f) * (f32)(u8)frand())) /
                                    (180.0))));
    initialPosition[1] = (99.0f);
    HuSprGrpPosSet(spriteGroup, initialPosition[0], initialPosition[1]);
    HuSprScaleSet(spriteGroup, 0, (0.0f), (0.0f));
    angle = (0.0f);
    rotation = (0.0f);
    transparency = (1.0f);
    delay = frandmod(20);
    HuPrcSleep(delay);
    do {
        if (lbl_1_bss_4D8->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(spriteGroup, 64);
        } else {
            HuSprGrpDrawNoSet(spriteGroup, 0);
        }
        HuPrcVSleep();
        rotation = (f32)((1.5) *
                         sin(((3.141592653589793) * (f64)angle) / (180.0)));
        transparency = (f32)((1.0) -
                             sin(((3.141592653589793) * (f64)angle) / (180.0)));
        HuSprZRotSet(spriteGroup, 0, rotation);
        HuSprScaleSet(spriteGroup, 0, rotation, rotation);
        HuSprTPLvlSet(spriteGroup, 0, transparency);
        angle += step;
    } while (!(angle >= (90.0f)));
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Emit a microphone activity ring every twenty frames while this process runs. */
void fn_1_27308(void) {
    for (;;) {
        HuPrcChildCreate(fn_1_27344, 100U, 6144U, 0, lbl_1_bss_548);
        HuPrcSleep(20);
    }
}

/* Shrink one microphone activity ring while increasing its opacity, then remove its sprite
 * group. Select the draw pass from the current pause display state. */
void fn_1_27344(void) {
    ANIMDATA *animation;
    s16 spriteGroup;
    f32 scalePhase;
    f32 transparencyPhase;
    f32 scale;
    f32 transparency;

    animation =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 31), HU_MEMNUM_OVL, HEAP_MODEL));
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(animation, 47, 0));
    HuSprPosSet(spriteGroup, 0, (516.0f), (362.0f));
    HuSprTPLvlSet(spriteGroup, 0, (0.0f));

    scalePhase = (90.0f);
    transparencyPhase = (0.0f);
    do {
        if (lbl_1_bss_4D8->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(spriteGroup, 64);
        } else {
            HuSprGrpDrawNoSet(spriteGroup, 0);
        }
        HuPrcVSleep();
        scale = (f32)sin(((3.141592653589793) * (f64)scalePhase) / (180.0));
        transparency = (f32)sin(((3.141592653589793) * (f64)transparencyPhase) / (180.0));
        HuSprScaleSet(spriteGroup, 0, scale, scale);
        HuSprTPLvlSet(spriteGroup, 0, transparency);
        scalePhase += (2.0f);
        transparencyPhase += (2.0f);
        if (transparency >= (1.0f)) {
            transparency = (1.0f);
        }
    } while (!(scalePhase >= (180.0f)));
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Choose the accepted or rejected feedback icon and start its flashing process. */
void fn_1_27530(s32 accepted) {
    s32 unused;
    if (accepted)
    {
        HuAudFXPlay(MSM_SE_CMN_11);
        lbl_1_bss_378 = lbl_1_bss_390;
        fn_1_2E6C4(3);
    }
    else
    {
        HuAudFXPlay(MSM_SE_CMN_12);
        lbl_1_bss_378 = lbl_1_bss_38E;
    }
    HuPrcChildCreate(fn_1_275CC, 100U, 6144U, 0, lbl_1_bss_548);
}

/* Flash the selected answer feedback icon twice before ending this process. */
void fn_1_275CC(void) {
    HuSprAttrReset(lbl_1_bss_378, 0, HUSPR_ATTR_DISPOFF);
    HuPrcSleep(30);
    HuSprAttrSet(lbl_1_bss_378, 0, HUSPR_ATTR_DISPOFF);
    HuPrcSleep(30);
    HuSprAttrReset(lbl_1_bss_378, 0, HUSPR_ATTR_DISPOFF);
    HuPrcSleep(30);
    HuSprAttrSet(lbl_1_bss_378, 0, HUSPR_ATTR_DISPOFF);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Pulse the microphone HUD while a response is being requested. */
void fn_1_2765C(void) {
    s32 frame = 0;
    f32 angle = (0.0f);

    for (; ; frame++) {
        f32 scale;

        HuPrcVSleep();
        scale = (f32)((1.0) + ((0.30000001192092896) *
                 sin(((3.141592653589793) * (f64)angle) / (180.0))));
        HuSprScaleSet(lbl_1_bss_5CC, 1, scale, scale);
        angle += (5.0f);
        if (angle >= (180.0f)) {
            angle = (0.0f);
        }
    }
}

/* Play the event icon effect selected by the question type and answer result. */
void fn_1_27730(s32 accepted, s32 questionType) {
    s32 index;

    if (questionType == 0 || questionType == 1) {
        if (accepted != 0) {
            for (index = 0; index < 15; index++) {
                HuPrcChildCreate(fn_1_24294, 100U, 6144U, 0, lbl_1_bss_548);
            }
            fn_1_27CC0();
            return;
        }
        fn_1_27E18();
        return;
    }
    if (questionType == 3) {
        if (accepted != 0) {
            fn_1_27E18();
            return;
        }
        fn_1_27840();
        return;
    }
    if (questionType == 6) {
        if (accepted != 0) {
            for (index = 0; index < 15; index++) {
                HuPrcChildCreate(fn_1_24294, 100U, 6144U, 0, lbl_1_bss_548);
            }
            fn_1_27CC0();
            return;
        }
        fn_1_27E18();
    }
}

/* Break apart the event icon, fade its overlay, and hide it during a white wipe. */
void fn_1_27840(void) {
    f32 angle = (0.0f);
    f32 transparency;
    s32 index;

    HuAudFXPlay(MSM_SE_MICQUIZ_EVENT_FRAGMENT);
    for (index = 0; index < 15; index++) {
        HuPrcChildCreate(fn_1_27A50, 100U, 6144U, 0, lbl_1_bss_548);
    }
    HuPrcSleep(45);
    HuSprTPLvlSet(lbl_1_bss_4A8, 0, (0.0f));
    HuSprAttrReset(lbl_1_bss_4A8, 0, HUSPR_ATTR_DISPOFF);
    do {
        if (lbl_1_bss_4D8->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(lbl_1_bss_4A8, 64);
        } else {
            HuSprGrpDrawNoSet(lbl_1_bss_4A8, 0);
        }
        HuPrcVSleep();
        transparency = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_4A8, 0, transparency);
        angle += (18.0f);
    } while (!(angle >= (90.0f)));
    fn_1_31910(0);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_WHITE, 5);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    HuSprAttrSet(lbl_1_bss_47E, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(lbl_1_bss_4A8, 0, HUSPR_ATTR_DISPOFF);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_WHITE, 5);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    fn_1_31910(1);
}

/* Move one event fragment toward the icon center and change its animation in flight. */
void fn_1_27A50(void) {
    f32 velocityX;
    f32 velocityY;
    f32 originX;
    f32 originY;
    f32 randomAngle;
    f32 randomRadius;
    f32 positionX;
    f32 positionY;
    s16 spriteGroup;
    s32 delayFrames;
    s32 frame;
    randomAngle = (2.0f) * frandf();
    randomRadius = frandf();
    delayFrames = frandmod(10);
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(lbl_1_bss_4CC[0], 49, 0));
    HuSprAttrSet(spriteGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(spriteGroup, 0, HUSPR_ATTR_NOANIM);
    HuPrcSleep(delayFrames);
    originX = (401.0f);
    originY = (62.0f);
    positionX =
        (f32) ((f64) originX +
               ((35.0) * ((f64) randomRadius * sin((3.141592653589793) * (f64) randomAngle))));
    positionY =
        (f32) ((f64) originY +
               ((35.0) * ((f64) randomRadius * cos((3.141592653589793) * (f64) randomAngle))));
    velocityX = (originX - positionX) / (45.0f);
    velocityY = (originY - positionY) / (45.0f);
    HuSprAttrReset(spriteGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(spriteGroup, 0, HUSPR_ATTR_NOANIM);
    frame = 0;
    while (frame < 45)
    {
        if (lbl_1_bss_4D8->pauseDisplayActive != 0)
        {
            HuSprGrpDrawNoSet(spriteGroup, 64);
        }
        else
        {
            HuSprGrpDrawNoSet(spriteGroup, 0);
        }
        HuPrcVSleep();
        positionX += velocityX;
        positionY += velocityY;
        HuSprPosSet(spriteGroup, 0, positionX, positionY);
        if (frame == 10)
        {
            HuSprGrpKill(spriteGroup);
            spriteGroup = HuSprGrpCreate(1);
            HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(lbl_1_bss_4CC[1], 49, 0));
            HuSprPosSet(spriteGroup, 0, positionX, positionY);
        }
        frame += 1;
    }
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    waitAfterEnd : HuPrcVSleep();
    goto waitAfterEnd;
}

/* Expand and fade the event icon after an accepted response. */
void fn_1_27CC0(void) {
    f32 transparency;
    f32 scale;
    f32 angle;

    scale = (1.0f);
    transparency = (1.0f);
    angle = (0.0f);
    do {
        HuPrcVSleep();
        HuSprScaleSet(lbl_1_bss_47E, 0, scale, scale);
        HuSprTPLvlSet(lbl_1_bss_47E, 0, transparency);
        scale = (f32) ((1.0) + sin(((3.141592653589793) * (f64) angle) / (180.0)));
        transparency = (f32) ((1.0) - sin(((3.141592653589793) * (f64) angle) / (180.0)));
        angle += (1.5f);
    } while (!(angle >= (90.0f)));
    HuSprAttrSet(lbl_1_bss_47E, 0, HUSPR_ATTR_DISPOFF);
}

/* Shrink and fade the event icon when the response removes it. */
void fn_1_27E18(void) {
    f32 transparency;
    f32 scale;
    f32 angle;

    scale = (1.0f);
    transparency = (1.0f);
    angle = (0.0f);
    do {
        HuPrcVSleep();
        HuSprScaleSet(lbl_1_bss_47E, 0, scale, scale);
        HuSprTPLvlSet(lbl_1_bss_47E, 0, transparency);
        scale = (f32) ((1.0) - sin(((3.141592653589793) * (f64) angle) / (180.0)));
        transparency = (f32) ((1.0) - sin(((3.141592653589793) * (f64) angle) / (180.0)));
        angle += (3.0f);
    } while (!(angle >= (90.0f)));
    HuSprAttrSet(lbl_1_bss_47E, 0, HUSPR_ATTR_DISPOFF);
}

void fn_1_27F70(void)
{
}

void fn_1_27F74(void)
{
}

/* Return the selected question level to the round process. */
u8 fn_1_27F78(void) {
    return lbl_1_bss_4D8->questionLevel;
}

/* Return the selected question column to the round process. */
u8 fn_1_27F8C(void) {
    return lbl_1_bss_4D8->questionColumn;
}

/* Store the answering player, remaining points, and result for the round controller. */
void fn_1_27FA0(s8 playerNo, u16 points, s32 result) {
    lbl_1_bss_4D8->answeringPlayer = playerNo;
    lbl_1_bss_4D8->roundPoints = points;
    lbl_1_bss_4D8->roundResult = result;
}

/* Return the player whose turn selected the question. */
s32 fn_1_27FD8(void) {
    return lbl_1_bss_4D8->currentPlayer;
}

/* Return whether the requested player participates in this quiz. */
s32 fn_1_27FEC(s32 playerNo) {
    return lbl_1_bss_4D8->playerActive[playerNo];
}

/* Return the remaining answer time as a fraction of the selected question allowance. */
f32 fn_1_28004(void) {
    u8 nextIndex;
    u8 previousIndex;
    u8 currentIndex;
    s32 selectedIndex;

    if (fn_1_281D4() == 2) {
        nextIndex = lbl_1_bss_4D8->questionLevel;
        selectedIndex = nextIndex + 1;
    } else if (fn_1_281D4() == 5) {
        previousIndex = lbl_1_bss_4D8->questionLevel;
        selectedIndex = previousIndex - 1;
    } else {
        currentIndex = lbl_1_bss_4D8->questionLevel;
        selectedIndex = currentIndex;
    }
    return ((f32)lbl_1_bss_520 - ((f32)lbl_1_bss_518 / (f32)lbl_1_bss_51C)) /
           (f32)lbl_1_data_48[selectedIndex];
}

/* Wait two seconds between quiz presentation steps. */
void fn_1_28178(void) {
    HuPrcSleep(120);
}

/* Wait one second between quiz presentation steps. */
void fn_1_2819C(void) {
    HuPrcSleep(60);
}

/* Return which dialogue layout the current quiz mode uses. */
s32 fn_1_281C0(void) {
    return lbl_1_bss_4D8->presenterLayout;
}

/* Return the current question event to its HUD and timing helpers. */
s32 fn_1_281D4(void) {
    return lbl_1_bss_4D8->questionEvent;
}

/* Request the round presentation transition and mark it unfinished. */
void fn_1_281E8(void) {
    lbl_1_bss_4D8->presentationBusy = 1;
}

/* Mark the requested round presentation transition as finished. */
void fn_1_28200(void) {
    lbl_1_bss_4D8->presentationBusy = 0;
}

/* Present the current player name in the dialogue layout used by this mode. */
void fn_1_28218(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->currentPlayer]],
                   0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 13), 1, 0);
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[lbl_1_bss_4D8->currentPlayer]],
                   0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 76), 1, 3);
    }
    fn_1_20C94();
}

/* Insert a supplied message into the first response prompt for this dialogue layout. */
void fn_1_284C4(u32 message) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20D08(1, message, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 16), 1, 1);
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, message, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 79), 1, 3);
    }
    fn_1_20C94();
}

/* Insert a supplied message into the alternate response prompt for this layout. */
void fn_1_28704(u32 message) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20D08(1, message, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 27), 1, 1);
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, message, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 83), 1, 3);
    }
    fn_1_20C94();
}

/* Present the next response prompt unless the answer timer has expired. */
void fn_1_28944(void) {
    OSReport("SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSScoreMode:%d\n", lbl_1_bss_524);
    if (lbl_1_bss_524 != 2) {
        if (lbl_1_bss_4D8->presenterLayout == 0) {
            fn_1_20B74(1);
            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 48), 1, 1);
        } else {
            fn_1_20B74(3);
            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 102), 1, 3);
        }
        fn_1_20C94();
    }
}

/* Present the timeout follow-up using the current dialogue layout. */
void fn_1_28B70(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 17), 1, 1);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 80), 1, 3);
    }
    fn_1_20C94();
}

/* Present the response follow-up using the current dialogue layout. */
void fn_1_28D6C(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 25), 1, 0);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 81), 1, 3);
    }
    fn_1_20C94();
}

/* Present a named-player response message unless the answer timer has expired. */
void fn_1_28F68(s32 playerIndex)
{
    if (lbl_1_bss_524 != 2) {
        if (lbl_1_bss_4D8->presenterLayout == 0) {
            fn_1_20B74(0);
            fn_1_20D08(0, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 0);
            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 26), 1, 0);
        } else {
            fn_1_20B74(3);
            fn_1_20D08(3, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[playerIndex]], 0);
            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 82), 1, 3);
        }
        fn_1_20C94();
    }
}

/* Present the next quiz response message using the current dialogue layout. */
void fn_1_2920C(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 28), 1, 1);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 84), 1, 3);
    }
    fn_1_20C94();
}

/* Present an inserted response message, wait for it, and close the dialogue. */
void fn_1_29408(u32 message) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_20D08(0, message, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 29), 1, 0);
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, message, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    fn_1_20C94();
    fn_1_20B24();
}

/* Present a response follow-up using the current dialogue layout. */
void fn_1_2967C(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(2);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 40), 1, 2);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 96), 1, 3);
    }
    fn_1_20C94();
}

/* Present the alternate response follow-up using the current dialogue layout. */
void fn_1_29878(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 49), 1, 1);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 103), 1, 3);
    }
    fn_1_20C94();
}

/* Present the next round follow-up using the current dialogue layout. */
void fn_1_29A74(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 50), 1, 1);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 104), 1, 3);
    }
    fn_1_20C94();
}

/* Present the round follow-up unless the answer timer has expired. */
void fn_1_29C70(void) {
    if (lbl_1_bss_524 != 2) {
        if (lbl_1_bss_4D8->presenterLayout == 0) {
            fn_1_20B74(0);
            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 55), 1, 0);
        } else {
            fn_1_20B74(3);
            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 114), 1, 3);
        }
        fn_1_20C94();
    }
}

/* Present the question introduction using the current dialogue layout. */
void fn_1_29E80(void) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 2), 1, 0);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 42), 1, 3);
    }
    fn_1_20C94();
}

/* Present two dialogue lines with a delay before naming the selected player. */
void fn_1_2A07C(s32 messageIndex) {
    fn_1_20B74(0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 20), 1, 0);
    fn_1_20C94();
    HuPrcSleep(120);
    fn_1_20B74(1);
    fn_1_20D08(1, lbl_1_data_A0[lbl_1_bss_4D8->playerCharacter[messageIndex]], 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_I, 4), 1, 1);
    fn_1_20C94();
}

/* Initialize microphone recognition and load the requested response context. */
void fn_1_2A2D8(char *path, s32 questionType) {
    lbl_1_bss_4F8 = questionType;
    HuMCInit(0);
    HuMCSelWinCreate((-10000.0f), (88.0f));
    lbl_1_bss_50C = HuMCContextCreate(path);
}

/* Choose the microphone recognition context for the current question type. */
void fn_1_2A344(s32 questionType) {
    lbl_1_bss_4F8 = questionType;
    HuMCInit(0);
    HuMCSelWinCreate((-10000.0f), (88.0f));
    if (questionType == 0) {
        lbl_1_bss_50C = HuMCContextCreate("/mic/ctx/micquiz_num");
    } else if (questionType == 1) {
        lbl_1_bss_50C = HuMCContextCreate("/mic/ctx/micquiz_num2");
    } else if (questionType == 2) {
        lbl_1_bss_50C = HuMCContextCreate("/mic/ctx/micquiz_chara");
    } else if (questionType == 3) {
        lbl_1_bss_50C = HuMCContextCreate("/mic/ctx/micquiz_chara");
    } else if (questionType == 4) {
        lbl_1_bss_50C = HuMCContextCreate("/mic/ctx/micquiz_npc");
    } else if (questionType == 5) {
        lbl_1_bss_50C = HuMCContextCreate("/mic/ctx/micquiz_kurabe1");
    } else if (questionType == 6) {
        lbl_1_bss_50C = HuMCContextCreate("/mic/ctx/micquiz_fruit");
    } else {
        lbl_1_bss_50C = HuMCContextCreate("/mic/ctx/micquiz_chara");
    }
}

/* Remember the current process as the parent of quiz response effects. */
void fn_1_2A4B0(void) {
    lbl_1_bss_548 = HuPrcCurrentGet();
}

/* Remember the current process as the parent of quiz response effects. */
void fn_1_2A4DC(void) {
    lbl_1_bss_548 = HuPrcCurrentGet();
}

/* Run question responses, retries, and participant changes until the round result is ready. */
void fn_1_2A508(s32 quizMode) {
    extern s32 lbl_1_bss_4;
    extern s32 lbl_1_bss_514;
    extern s16 lbl_1_bss_378, lbl_1_bss_38E, lbl_1_bss_390, lbl_1_bss_47E;
    extern void fn_1_2DCEC(void);
    extern void fn_1_2F52C(u32 messageId);
    extern void fn_1_275CC(void);
    extern u32 lbl_1_data_1CC[2], lbl_1_data_1D4[8], lbl_1_data_5C[17], lbl_1_data_E8[17];
    f32 firstVerticalOffset;
    f32 outgoingAmplitude;
    f32 secondVerticalOffset;
    f32 incomingAmplitude;
    void *motionProcess;
    HUPROCESS *inputProcess;
    f32 verticalOffset;
    f32 alternateVerticalOffset;
    f64 motionAngle;
    f64 returnAngle;
    f64 messageMotionAngle;
    f64 initialMotionAngle;
    f64 closingAngle;
    f64 openingAngle;
    s32 hasSelection;
    s32 selectedAnswer;
    s32 currentAnswer;
    s32 currentPlayer;
    s32 playerIndex;
    s32 initialPlayer;
    u32 *messageEntry;
    u32 *alternateMessageEntry;

    hasSelection = 0;
    if (quizMode == 3 || quizMode == 2) {
        if ((s32) lbl_1_bss_514 >= 9) {
            lbl_1_bss_514 -= 1;
        }
    } else if (quizMode == 4) {
        if ((s32) lbl_1_bss_514 >= 15) {
            lbl_1_bss_514 -= 1;
        }
    }
    lbl_1_bss_4FC = 0;
    lbl_1_bss_528 = 0;
    lbl_1_bss_544 = quizMode;
    lbl_1_bss_540 = 0;
    lbl_1_bss_538 = 0;
    lbl_1_bss_534 = 1;
    lbl_1_bss_4 = 0;
    fn_1_28200();
    lbl_1_bss_4D8->responseRecord[2] = (s8)(lbl_1_bss_53C = fn_1_27FD8());
    fn_1_2DABC(lbl_1_bss_4F8, lbl_1_bss_53C);
    fn_1_31C1C();
    fn_1_2D748(0);
    fn_1_31910(0);
    lbl_1_bss_524 = 1;
    do {
        HuPrcVSleep();
    } while ((s32) lbl_1_bss_4FC == 0);
    fn_1_2E6C4(0);
    lbl_1_bss_508 = 0;
    HuPrcVSleep();
    lbl_1_bss_508 = 1;
    HuPrcSleep(30);
    lbl_1_bss_524 = 0;
    fn_1_266A0(1);
    fn_1_31C1C();
    fn_1_2D748(1);
    motionProcess = HuPrcChildCreate(fn_1_2DCEC, 100U, 6144U, 0, lbl_1_bss_548);
    inputProcess = HuPrcChildCreate(fn_1_2D34C, 100U, 6144U, 0, lbl_1_bss_548);
    fn_1_31910(1);
readInitialResponse:
    HuPrcVSleep();
    if (HuMCSelWinCheck()) {
        fn_1_20E64(1);
    }
    lbl_1_bss_528 = 1;
    lbl_1_bss_530 = fn_1_2CDCC(lbl_1_bss_53C, quizMode);
    fn_1_31910(1);
    lbl_1_bss_528 = 0;
    fn_1_31C1C();
    if (HuMCSelWinCheck() != 0) {
        HuWinHomeClear(lbl_1_bss_37E);
    }
    if ((s32) lbl_1_bss_524 == 2) {
waitAfterAbort:
        HuPrcVSleep();
        goto waitAfterAbort;
    }
    fn_1_2D748(0);
    if ((s32) lbl_1_bss_530 == 0) {
        if ((s32) lbl_1_bss_538 == 0) {
            fn_1_208E4();
            lbl_1_bss_508 = 2;
            fn_1_266A0(3);
            fn_1_29E80();
            HuPrcVSleep();
waitForResponseReady:
            HuPrcVSleep();
            if ((s32) lbl_1_bss_540 != 0) {
            fn_1_266A0(2);
            lbl_1_bss_538 = 1;
            lbl_1_bss_534 = 0;
            lbl_1_bss_524 = 0;
            fn_1_31C1C();
            fn_1_2D748(1);
    fn_1_2E6C4(2);
        } else {
                fn_1_208E4();
                goto waitForResponseReady;
            }
            fn_1_28F68(lbl_1_bss_53C);
            fn_1_2D748(1);
            } else {
            fn_1_29A74();
            HuPrcSleep(120);
    fn_1_28F68(lbl_1_bss_53C);
            fn_1_2D748(1);
        }
        goto readInitialResponse;
    } else if ((s32) lbl_1_bss_530 == 1) {
        lbl_1_bss_524 = 1;
        lbl_1_bss_534 = 0;
        if ((s32) lbl_1_bss_540 == 0) {
    fn_1_266A0(4);
        }
        HuAudFXPlay(MSM_SE_CMN_11);
        lbl_1_bss_378 = lbl_1_bss_390;
        fn_1_2E6C4(3);
        HuPrcChildCreate(fn_1_275CC, 100U, 6144U, 0, lbl_1_bss_548);
        lbl_1_bss_52C = 1;
        fn_1_27730(1, fn_1_281D4());
        lbl_1_bss_4D8->responseRecord[0] = 0;
        goto finishRound;
    } else if ((s32) lbl_1_bss_530 == 2) {
        if ((fn_1_281D4() == 3) && (hasSelection == 0)) {
    fn_1_27730(0, 3U);
            lbl_1_bss_524 = 1;
            hasSelection = 1;
            fn_1_2A07C(lbl_1_bss_53C);
            HuSprAttrSet(lbl_1_bss_47E, 0, HUSPR_ATTR_DISPOFF);
            fn_1_2D748(1);
            goto readInitialResponse;
        }
        if (hasSelection == 0) {
            lbl_1_bss_524 = 1;
            lbl_1_bss_534 = 0;
            HuAudFXPlay(MSM_SE_CMN_12);
            lbl_1_bss_378 = lbl_1_bss_38E;
            HuPrcChildCreate(fn_1_275CC, 100U, 6144U, 0, lbl_1_bss_548);
            fn_1_2EF84(quizMode, 0);
            HuPrcSleep(120);
            lbl_1_bss_52C = 0;
            lbl_1_bss_4D8->responseRecord[0] = 1;
            if ((s32) lbl_1_bss_540 == 0) {
    fn_1_266A0(4);
            }
        } else {
            lbl_1_bss_524 = 1;
            lbl_1_bss_534 = 0;
            HuAudFXPlay(MSM_SE_CMN_12);
            lbl_1_bss_378 = lbl_1_bss_38E;
            HuPrcChildCreate(fn_1_275CC, 100U, 6144U, 0, lbl_1_bss_548);
            fn_1_2EF84(quizMode, 1);
            HuPrcSleep(60);
            lbl_1_bss_52C = 0;
            lbl_1_bss_4D8->responseRecord[0] = 1;
            if ((s32) lbl_1_bss_540 == 0) {
    fn_1_266A0(4);
            }
            goto finishRound;
        }
    } else if ((s32) lbl_1_bss_530 == -2) {
        fn_1_28944();
        HuPrcSleep(120);
        fn_1_28F68(lbl_1_bss_53C);
        fn_1_2D748(1);
        goto readInitialResponse;
    } else if ((s32) lbl_1_bss_530 == -3) {
        fn_1_29C70();
        HuPrcSleep(120);
        fn_1_28F68(lbl_1_bss_53C);
        fn_1_2D748(1);
        goto readInitialResponse;
    } else if ((s32) lbl_1_bss_530 == -4) {
    fn_1_28F68(lbl_1_bss_53C);
        fn_1_2D748(1);
        goto readInitialResponse;
    } else {
        goto readInitialResponse;
    }
beginNextParticipant:
        lbl_1_bss_534 = 0;
        if (HuMCSelWinCheck() != 0) {
            fn_1_20E64(3);
        }
        HuPrcVSleep();
        fn_1_31B20();
        fn_1_2D748(2);
        fn_1_31924(lbl_1_bss_53C);
        fn_1_208E4();
        fn_1_2E6C4(2);
        lbl_1_bss_524 = 0;
        HuPrcKill(motionProcess);
        fn_1_28D6C();
waitForNextParticipant:
            HuPrcVSleep();
            if ((s32) lbl_1_bss_524 == 2) {
                fn_1_28B70();
                HuPrcSleep(60);
                lbl_1_bss_52C = 0;
                lbl_1_bss_4D8->responseRecord[1] = 2;
                goto finishRound;
            } else {
                for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                    if (lbl_1_bss_4D8->playerActive[playerIndex] != 0 &&
                        playerIndex != fn_1_27FD8() &&
                        (HuPadBtnDown[playerIndex] & PAD_BUTTON_A) != 0) {
                    HuAudFXPlay(MSM_SE_MICQUIZ_PLAYER_BUZZ_IN);
                    lbl_1_bss_4D8->responseRecord[3] = (s8)(lbl_1_bss_53C = playerIndex);
                    lbl_1_bss_524 = 1;
                    fn_1_31C1C();
                    fn_1_2D748(1);
                    fn_1_31924(fn_1_27FD8());
    fn_1_31A0C(lbl_1_bss_53C);
                    fn_1_20778();
    fn_1_28F68(lbl_1_bss_53C);
                    fn_1_2DABC(lbl_1_bss_4F8, lbl_1_bss_53C);
                    goto showRetryHint;
                }
                }
                goto waitForNextParticipant;
showRetryHint:
                    if (HuMCSelWinCheck() != 0) {
                        fn_1_20E64(4);
                    }
readRetryResponse:
                    lbl_1_bss_528 = 1;
                    lbl_1_bss_530 = fn_1_2CDCC(playerIndex, quizMode);
                    fn_1_31910(1);
                    lbl_1_bss_528 = 0;
                    if (HuMCSelWinCheck() != 0) {
                        HuWinHomeClear(lbl_1_bss_37E);
                    }
                    fn_1_2D748(0);
                    if (lbl_1_bss_530 == 0) {
                        fn_1_29A74();
                        HuPrcSleep(120);
                        if (HuMCSelWinCheck() != 0) {
                            fn_1_20E64(4);
                        }
    fn_1_28F68(lbl_1_bss_53C);
                        fn_1_2D748(1);
                        goto readRetryResponse;
                    }
                    else if (lbl_1_bss_530 == 1) {
                        lbl_1_bss_524 = 1;
                        lbl_1_bss_52C = 1;
                        HuAudFXPlay(MSM_SE_CMN_11);
                        lbl_1_bss_378 = lbl_1_bss_390;
        fn_1_2E6C4(3);
                        HuPrcChildCreate(fn_1_275CC, 100U, 6144U, 0, lbl_1_bss_548);
                        lbl_1_bss_4D8->responseRecord[1] = 0;
                        fn_1_27730(1, fn_1_281D4());
                        goto finishRound;
                    } else if (lbl_1_bss_530 == 2) {
                        lbl_1_bss_524 = 1;
                        HuAudFXPlay(MSM_SE_CMN_12);
                        lbl_1_bss_378 = lbl_1_bss_38E;
                        HuPrcChildCreate(fn_1_275CC, 100U, 6144U, 0, lbl_1_bss_548);
                        fn_1_2EF84(quizMode, 1);
                        HuPrcSleep(60);
                        lbl_1_bss_52C = 0;
                        lbl_1_bss_4D8->responseRecord[1] = 1;
                        fn_1_27730(1, fn_1_281D4());
                        goto finishRound;
                    } else {
                    if (lbl_1_bss_530 == -2) {
                            fn_1_28944();
                            HuPrcSleep(120);
                            fn_1_28F68(lbl_1_bss_53C);
                            fn_1_2D748(1);
                    } else if (lbl_1_bss_530 == -3) {
                            fn_1_29C70();
                            HuPrcSleep(120);
    fn_1_28F68(lbl_1_bss_53C);
                            fn_1_2D748(1);
                    } else if (lbl_1_bss_530 == -4) {
    fn_1_28F68(lbl_1_bss_53C);
                            fn_1_2D748(1);
                        }
                        goto readRetryResponse;
                    }
                }
                goto waitForNextParticipant;
    goto readInitialResponse;
finishRound:
        if (HuMCSelWinCheck() != 0) {
            HuWinHomeClear(lbl_1_bss_37E);
        }
        fn_1_27FA0((s8) lbl_1_bss_53C, (u16) lbl_1_bss_520, lbl_1_bss_52C);
    fn_1_31910(0);
        fn_1_2D748(0);
        if ((s32) lbl_1_bss_52C != 0) {
            fn_1_2967C();
        } else if ((s32) lbl_1_bss_4 == 0) {
            fn_1_208E4();
            fn_1_2D748(0);
            lbl_1_bss_4 = 1;
        if (quizMode == 0) {
                fn_1_2F76C(lbl_1_bss_514);
        } else if (quizMode == 1) {
                fn_1_2F9E4(lbl_1_bss_514);
        } else if (quizMode == 2) {
                fn_1_2F52C(lbl_1_data_E8[lbl_1_bss_514]);
        } else if (quizMode == 3) {
            fn_1_2F52C(lbl_1_data_E8[lbl_1_bss_514]);
        } else if (quizMode == 4) {
                fn_1_2F52C(lbl_1_data_5C[lbl_1_bss_514]);
        } else if (quizMode == 5) {
                fn_1_2F52C(lbl_1_data_1CC[lbl_1_bss_514]);
        } else if (quizMode == 6) {
                fn_1_2F52C(lbl_1_data_1D4[lbl_1_bss_514]);
            }
        }
        HuPrcSleep(100);
        fn_1_208E4();
        HuPrcSleep(30);
        if ((s32) lbl_1_bss_52C != 0) {
            lbl_1_bss_508 = 3;
            do {
                HuPrcVSleep();
            } while ((s32) lbl_1_bss_4D8->presentationBusy != 0);
        } else {
            lbl_1_bss_508 = 4;
            do {
                HuPrcVSleep();
            } while ((s32) lbl_1_bss_4D8->presentationBusy != 0);
        }
waitAfterRound:
        HuPrcVSleep();
        goto waitAfterRound;
}

/* Run the alternate question response sequence and return its result to the round controller. */
void fn_1_2BBF4(s32 questionType) {
    HUPROCESS *buttonProcess;
    HUPROCESS *selectionProcess;
    f64 angleFinal;
    f64 angleFourth;
    f64 angleThird;
    f64 angleSecond;
    f64 angleFirst;
    s32 playerIndex;

    lbl_1_bss_4FC = 0;
    lbl_1_bss_528 = 0;
    lbl_1_bss_544 = questionType;
    lbl_1_bss_540 = 0;
    lbl_1_bss_538 = 0;
    lbl_1_bss_534 = 1;
    fn_1_28200();
    lbl_1_bss_4D8->responseRecord[2] = (s8)(lbl_1_bss_53C = fn_1_27FD8());
    fn_1_2DABC(lbl_1_bss_4F8, lbl_1_bss_53C);
    fn_1_31C1C();
    fn_1_2D748(0);
    fn_1_31910(0);
    lbl_1_bss_548 = HuPrcCurrentGet();
    do {
        HuPrcVSleep();
    } while ((s32) lbl_1_bss_4FC == 0);
    fn_1_2E6C4(0);
    lbl_1_bss_508 = 0;
    HuPrcVSleep();
    lbl_1_bss_508 = 1;
    HuPrcSleep(30);
    lbl_1_bss_524 = 0;
    fn_1_266A0(1);
    fn_1_31C1C();
    fn_1_2D748(1);
    HuPrcChildCreate(fn_1_315C8, 100U, 6144U, 0, lbl_1_bss_548);
    buttonProcess = HuPrcChildCreate(fn_1_2E2B0, 100U, 6144U, 0, lbl_1_bss_548);
    selectionProcess = HuPrcChildCreate(fn_1_2D34C, 100U, 6144U, 0, lbl_1_bss_548);
    fn_1_31910(1);
readInitialResponse:
    HuPrcVSleep();
    if (HuMCSelWinCheck() != 0) {
        fn_1_20E64(1);
    }
    lbl_1_bss_528 = 1;
    lbl_1_bss_530 = fn_1_2CDCC(lbl_1_bss_53C, questionType);
    fn_1_31910(1);
    lbl_1_bss_528 = 0;
    fn_1_31C1C();
    if (HuMCSelWinCheck() != 0) {
        HuWinHomeClear(lbl_1_bss_37E);
    }
    if ((s32) lbl_1_bss_524 == 2) {
waitAfterAbort:
        HuPrcVSleep();
        goto waitAfterAbort;
    }
    fn_1_2D748(0);
    if ((s32) lbl_1_bss_530 == 0) {
        if ((s32) lbl_1_bss_538 == 0) {
            fn_1_31C1C();
            fn_1_2D748(0);
            fn_1_208E4();
            lbl_1_bss_508 = 2;
            fn_1_266A0(3);
            fn_1_29E80();
            HuPrcVSleep();
waitForResponseReady:
            HuPrcVSleep();
            if ((s32) lbl_1_bss_540 != 0) {
                fn_1_266A0(2);
                lbl_1_bss_538 = 1;
                lbl_1_bss_534 = 0;
                lbl_1_bss_524 = 0;
                fn_1_31C1C();
                fn_1_2D748(1);
                fn_1_2E6C4(2);
            } else {
                fn_1_208E4();
                goto waitForResponseReady;
            }
            fn_1_28F68(lbl_1_bss_53C);
            fn_1_2D748(1);
        } else {
            fn_1_29A74();
            HuPrcSleep(120);
            fn_1_28F68(lbl_1_bss_53C);
            fn_1_2D748(1);
        }
        goto readInitialResponse;
    }
    if ((s32) lbl_1_bss_530 == 1) {
        lbl_1_bss_524 = 1;
        lbl_1_bss_534 = 0;
        if ((s32) lbl_1_bss_540 == 0) {
    fn_1_266A0(4);
        }
        lbl_1_bss_52C = 1;
        fn_1_27730(1, fn_1_281D4());
        lbl_1_bss_4D8->responseRecord[0] = 0;
        fn_1_310F0(lbl_1_bss_52C, lbl_1_bss_4D8->responseRecord[2]);
        goto finishRound;
    }
    if ((s32) lbl_1_bss_530 == 2) {
        fn_1_27730(0, fn_1_281D4());
        lbl_1_bss_524 = 1;
        lbl_1_bss_534 = 0;
        lbl_1_bss_52C = 0;
        lbl_1_bss_4D8->responseRecord[0] = 1;
        if ((s32) lbl_1_bss_540 == 0) {
    fn_1_266A0(4);
        }
    fn_1_310F0(lbl_1_bss_52C, lbl_1_bss_4D8->responseRecord[2]);
        HuPrcSleep(120);
        goto beginNextParticipant;
    }
    if ((s32) lbl_1_bss_530 == -2) {
        fn_1_28944();
        HuPrcSleep(120);
    fn_1_28F68(lbl_1_bss_53C);
        fn_1_2D748(1);
    } else if ((s32) lbl_1_bss_530 == -3) {
        fn_1_29C70();
        HuPrcSleep(120);
    fn_1_28F68(lbl_1_bss_53C);
        fn_1_2D748(1);
    } else if ((s32) lbl_1_bss_530 == -4) {
    fn_1_28F68(lbl_1_bss_53C);
        fn_1_2D748(1);
    }
    goto readInitialResponse;
beginNextParticipant:
    fn_1_3186C(0, 8.0f, HU3D_MOTATTR_LOOP);
    lbl_1_bss_534 = 0;
    if (HuMCSelWinCheck() != 0) {
        fn_1_20E64(3);
    }
    HuPrcVSleep();
    fn_1_31B20();
    fn_1_2D748(2);
    fn_1_31924(lbl_1_bss_53C);
    fn_1_208E4();
    fn_1_3186C(1, 16.0f, HU3D_MOTATTR_LOOP);
    fn_1_2E6C4(2);
    lbl_1_bss_524 = 0;
    HuPrcKill(buttonProcess);
    fn_1_28D6C();
waitForNextParticipant:
    HuPrcVSleep();
    if ((s32) lbl_1_bss_524 == 2) {
        fn_1_28B70();
        HuPrcSleep(60);
        lbl_1_bss_52C = 0;
        lbl_1_bss_4D8->responseRecord[1] = 2;
    } else {
        for (playerIndex = 0; playerIndex < 4; playerIndex += 1) {
            if (lbl_1_bss_4D8->playerActive[playerIndex] != 0 &&
                playerIndex != fn_1_27FD8() &&
                (HuPadBtnDown[playerIndex] & PAD_BUTTON_A) != 0) {
            HuAudFXPlay(MSM_SE_MICQUIZ_PLAYER_BUZZ_IN);
            lbl_1_bss_4D8->responseRecord[3] = (s8)(lbl_1_bss_53C = playerIndex);
            lbl_1_bss_524 = 1;
            fn_1_31C1C();
            fn_1_2D748(1);
            fn_1_31924(fn_1_27FD8());
            fn_1_31A0C(lbl_1_bss_53C);
            fn_1_20778();
            fn_1_28F68(lbl_1_bss_53C);
            fn_1_2DABC(lbl_1_bss_4F8, lbl_1_bss_53C);
            goto showRetryHint;
            }
        }
        goto waitForNextParticipant;
showRetryHint:
    if (HuMCSelWinCheck() != 0) {
        fn_1_20E64(4);
    }
readRetryResponse:
    lbl_1_bss_528 = 1;
    lbl_1_bss_530 = fn_1_2CDCC(playerIndex, questionType);
    fn_1_31910(1);
    lbl_1_bss_528 = 0;
    fn_1_31C1C();
    if (HuMCSelWinCheck() != 0) {
        HuWinHomeClear(lbl_1_bss_37E);
    }
    fn_1_2D748(0);
    if ((s32) lbl_1_bss_530 == 0) {
        fn_1_29A74();
        HuPrcSleep(120);
        if (HuMCSelWinCheck()) {
            fn_1_20E64(4);
        }
        fn_1_28F68(lbl_1_bss_53C);
        fn_1_2D748(1);
        goto readRetryResponse;
    }
    if ((s32) lbl_1_bss_530 == 1) {
        lbl_1_bss_524 = 1;
        lbl_1_bss_52C = 1;
        lbl_1_bss_4D8->responseRecord[1] = 0;
        fn_1_310F0(lbl_1_bss_52C, lbl_1_bss_4D8->responseRecord[3]);
    } else if ((s32) lbl_1_bss_530 == 2) {
        lbl_1_bss_524 = 1;
        lbl_1_bss_52C = 0;
        lbl_1_bss_4D8->responseRecord[1] = 1;
        fn_1_310F0(lbl_1_bss_52C, lbl_1_bss_4D8->responseRecord[3]);
    } else {
        if ((s32) lbl_1_bss_530 == -2) {
            fn_1_28944();
            HuPrcSleep(120);
            fn_1_28F68(lbl_1_bss_53C);
            fn_1_2D748(1);
        } else if ((s32) lbl_1_bss_530 == -3) {
            fn_1_29C70();
            HuPrcSleep(120);
            fn_1_28F68(lbl_1_bss_53C);
            fn_1_2D748(1);
        } else if ((s32) lbl_1_bss_530 == -4) {
            fn_1_28F68(lbl_1_bss_53C);
            fn_1_2D748(1);
        }
        goto readRetryResponse;
    }
    }
finishRound:
    if (HuMCSelWinCheck() != 0) {
        HuWinHomeClear(lbl_1_bss_37E);
    }
    fn_1_27FA0((s8) lbl_1_bss_53C, (u16) lbl_1_bss_520, lbl_1_bss_52C);
    fn_1_31910(0);
    fn_1_208E4();
    fn_1_2D748(0);
    if ((s32) lbl_1_bss_52C != 0) {
        lbl_1_bss_508 = 3;
    } else {
        HuPrcSleep(120);
        lbl_1_bss_508 = 4;
    }
waitAfterRound:
    HuPrcVSleep();
    goto waitAfterRound;
}

/* Read and validate a microphone or selection-window response for the question type. */
s32 fn_1_2CDCC(s32 playerIndex, s32 responseMode) {
    s32 newResponseMessage;
    s32 selectedResponse;
    s32 responseData;

    if (responseMode == 0) {
        newResponseMessage = MESSNUM(MESS_MIC_RETYPING, 44);
    } else if (responseMode == 1) {
        newResponseMessage = MESSNUM(MESS_MIC_RETYPING, 65);
    } else if (responseMode == 2) {
        newResponseMessage = MESSNUM(MESS_MIC_RETYPING, 5);
    } else if (responseMode == 3) {
        newResponseMessage = MESSNUM(MESS_MIC_RETYPING, 5);
    } else if (responseMode == 4) {
        newResponseMessage = MESSNUM(MESS_MIC_RETYPING, 23);
    } else if (responseMode == 5) {
        newResponseMessage = MESSNUM(MESS_MIC_RETYPING, 41);
    } else if (responseMode == 6) {
        newResponseMessage = MESSNUM(MESS_QUIZ_CHARA, 24);
    }

    HuMCSelWinContextSet(lbl_1_bss_50C, 0, (u8) playerIndex);
    HuMCSelWinChoiceGet();
    if (lbl_1_bss_524 == 2) {
        return -1;
    }

    lbl_1_bss_8 = (MicquizResponse *) HuMCCurResponseGet();
    if (lbl_1_bss_8->result == 0) {
        lbl_1_bss_510 = lbl_1_bss_8->recognizedWord;
    } else if (lbl_1_bss_8->result == -1) {
        return -2;
    } else if (lbl_1_bss_8->result == -2) {
        return -2;
    } else if (lbl_1_bss_8->result == -4) {
        return -3;
    } else {
        return -2;
    }

    if (HuMCUnkResponseCheck() != 0) {
        fn_1_31910(0);
        fn_1_2D748(1);
        HuPrcVSleep();
        fn_1_2D748(1);
        fn_1_2FCA0(lbl_1_bss_548);
        selectedResponse = HuMCNewResponseGet(newResponseMessage);
        if (selectedResponse == -1) {
            fn_1_2D748(0);
            fn_1_2FC60();
            fn_1_31910(1);
            return -4;
        }
        lbl_1_bss_510 = selectedResponse;
        fn_1_31910(1);
        fn_1_2D748(0);
    }

    if (HuMCSelWinCheck() != 0) {
        lbl_1_bss_510 -= 1;
    }
    if (lbl_1_bss_510 == 0) {
        return 0;
    }

    if (responseMode == 0) {
        if (HuMCSelWinCheck() != 0) {
            responseData = fn_1_2D214(lbl_1_bss_510 + lbl_1_bss_500, lbl_1_bss_514);
        } else {
            responseData = fn_1_2D214(lbl_1_bss_510, lbl_1_bss_514);
        }
    } else if (responseMode == 1) {
        if (HuMCSelWinCheck() != 0) {
            responseData = fn_1_2D214(lbl_1_bss_510 + lbl_1_bss_500, lbl_1_bss_514);
        } else {
            responseData = fn_1_2D214(lbl_1_bss_510, lbl_1_bss_514);
        }
    } else if (responseMode == 2) {
        responseData = fn_1_2D214(lbl_1_bss_510 - 1, lbl_1_bss_514);
    } else if (responseMode == 3) {
        responseData = fn_1_2D214(lbl_1_bss_510 - 1, lbl_1_bss_514);
    } else if (responseMode == 4) {
        responseData = fn_1_2D214(lbl_1_bss_510 - 1, lbl_1_bss_514);
    } else if (responseMode == 5) {
        responseData = fn_1_2D214(lbl_1_bss_510 - 1, lbl_1_bss_514);
    } else if (responseMode == 6) {
        responseData = fn_1_2D214(lbl_1_bss_510 - 1, lbl_1_bss_514);
    }
    return responseData;
}

/* Classify the response as equal to or different from the expected answer. */
s32 fn_1_2D214(s32 response, s32 expectedAnswer) {
    if (response == expectedAnswer) {
        return 1;
    }
    return 2;
}

/* Swing the listening indicator back and forth while its child process runs. */
void fn_1_2D22C(void) {
    f32 baseRotation;
    f32 angle;
    f32 sineOffset;
    s16 spriteGroup;
    s32 reverseSwing;

    spriteGroup = lbl_1_bss_472;
    angle = (0.0f);
    reverseSwing = 0;
    baseRotation = (0.0f);
animateFrame:
    HuPrcVSleep();
    sineOffset = (f32) ((10.0) * sin(((3.141592653589793) * (f64) angle) / (180.0)));
    HuSprZRotSet(spriteGroup, 0, baseRotation + sineOffset);
    if (reverseSwing == 0) {
        angle += (3.0f);
        if (angle >= (90.0f)) {
            angle = (90.0f);
            reverseSwing = 1;
        }
        goto animateFrame;
    }
    angle -= (3.0f);
    if (!(angle <= (-90.0f))) {
        goto animateFrame;
    }
    angle = (-90.0f);
    reverseSwing = 0;
    goto animateFrame;
}

/* Update listening feedback as the microphone selection state changes during a response. */
void fn_1_2D34C(void)
{
    s32 selectionHandled = 0;

    for (;;) {
        HuPrcVSleep();
        if (lbl_1_bss_524 == 2) {
            fn_1_2D748(0);
            continue;
        }
        if (lbl_1_bss_528 == 0) {
            continue;
        }
        if (HuMCSelWinCheck() != 0) {
            if ((s16)HuMCSelModeGet() == 1) {
                if (selectionHandled == 0) {
                    fn_1_31910(0);
                    fn_1_2DA0C();
                    fn_1_2E6C4(1);
                    fn_1_20E64(2);
                    fn_1_2D748(3);
                    if (lbl_1_bss_524 != 2) {
                        lbl_1_bss_524 = 1;
                    }
                    fn_1_20778();
                    selectionHandled = 1;
                }
            } else {
                fn_1_2DA6C();
                selectionHandled = 0;
            }
            continue;
        }
        if (HuMCButtonGet() != 0) {
            fn_1_31910(0);
            fn_1_2DA0C();
            fn_1_2D748(3);
            fn_1_20778();
            fn_1_2E6C4(1);
            if (lbl_1_bss_524 != 2) {
                lbl_1_bss_524 = 1;
            }
        } else {
            fn_1_2DA6C();
        }
        if (HuMCButtonGet() != 0) {
            if (selectionHandled == 0) {
                fn_1_31910(0);
                fn_1_2DA0C();
                selectionHandled = 1;
            }
            continue;
        }
        selectionHandled = 0;
    }
}

/* Switch the microphone HUD between idle, listening, and completed response displays. */
void fn_1_2D748(s32 displayState) {
    if (displayState == 0) {
        if (lbl_1_data_1F8 != displayState) {
            lbl_1_data_1F8 = displayState;
            HuSprAttrReset(lbl_1_bss_5CC, 2, HUSPR_ATTR_DISPOFF);
            HuSprBankSet(lbl_1_bss_5CC, 0, 1);
            HuSprBankSet(lbl_1_bss_5CC, 1, 1);
            fn_1_2D9BC();
            HuSprScaleSet(lbl_1_bss_5CC, 1, (1.0f), (1.0f));
            fn_1_2DA6C();
        }
    } else if (displayState == 1) {
        if (lbl_1_data_1F8 != displayState) {
            lbl_1_data_1F8 = displayState;
            HuSprAttrSet(lbl_1_bss_5CC, 2, HUSPR_ATTR_DISPOFF);
            HuSprBankSet(lbl_1_bss_5CC, 0, 0);
            HuSprBankSet(lbl_1_bss_5CC, 1, 0);
            fn_1_2D95C();
        }
    } else if (displayState == 2) {
        if (lbl_1_data_1F8 != displayState) {
            lbl_1_data_1F8 = displayState;
            HuSprAttrReset(lbl_1_bss_5CC, 2, HUSPR_ATTR_DISPOFF);
            HuSprBankSet(lbl_1_bss_5CC, 0, 1);
            HuSprBankSet(lbl_1_bss_5CC, 1, 1);
        }
    } else if (displayState == 3 && lbl_1_data_1F8 != displayState) {
        lbl_1_data_1F8 = displayState;
        fn_1_2D9BC();
        HuSprScaleSet(lbl_1_bss_5CC, 1, (1.0f), (1.0f));
    }
}

/* Start the microphone pulse process if it is not already running. */
void fn_1_2D95C(void) {
    if(lbl_1_bss_10==0) {
        lbl_1_bss_10=HuPrcChildCreate(fn_1_2765C,100,8192,0,lbl_1_bss_548);
    }
}

/* Stop the microphone pulse process when listening ends. */
void fn_1_2D9BC(void) {
    if ((HUPROCESS *) lbl_1_bss_10) {
        HuPrcKill(lbl_1_bss_10);
        lbl_1_bss_10 = NULL;
    }
}

/* Start the repeating microphone activity rings if they are not already running. */
void fn_1_2DA0C(void) {
    if(lbl_1_bss_0==0) {
        lbl_1_bss_0=HuPrcChildCreate(fn_1_27308,100,8192,0,lbl_1_bss_548);
    }
}

/* Stop the repeating microphone activity rings when listening ends. */
void fn_1_2DA6C(void) {
    if ((HUPROCESS *) lbl_1_bss_0) {
        HuPrcKill(lbl_1_bss_0);
        lbl_1_bss_0 = NULL;
    }
}

/* Configure selection-window answer entries and bind them to the answering controller. */
void fn_1_2DABC(s32 select, s32 padNo) {
    if (select == 0 || select == 1) {
        fn_1_2DBFC(padNo, select);
    } else if (select == 2) {
        lbl_1_bss_504 = HuMCSelWinItemSet(MESSNUM(MESS_QUIZ_CHARA, 0), 11, (s16)padNo);
    } else if (select == 3) {
        lbl_1_bss_504 = HuMCSelWinItemSet(MESSNUM(MESS_QUIZ_CHARA, 0), 11, (s16)padNo);
    } else if (select == 4) {
        lbl_1_bss_504 = HuMCSelWinItemSet(MESSNUM(MESS_QUIZ_CHARA, 11), 7, (s16)padNo);
    } else if (select == 5) {
        lbl_1_bss_504 = HuMCSelWinItemSet(MESSNUM(MESS_QUIZ_CHARA, 54), 3, (s16)padNo);
    } else if (select == 6) {
        lbl_1_bss_504 = HuMCSelWinItemSet(MESSNUM(MESS_MIC_RETYPING, 118), 9, (s16)padNo);
    }
    HuMCSessionSet((s16)padNo);
}

/* Configure the numeric selection range and bind it to the answering controller. */
void fn_1_2DBFC(s32 padNo, s32 direction) {
    s32 messageBase;

    if (direction == 0) {
        /* Numeric answers use either the first or second ten-entry selection page. */
    if (lbl_1_bss_514 <= 10) {
            messageBase = MESSNUM(MESS_QUIZ_CHARA, 81);
            lbl_1_bss_500 = 0;
        } else {
            messageBase = MESSNUM(MESS_QUIZ_CHARA, 92);
            lbl_1_bss_500 = 10;
        }
    } else if (lbl_1_bss_514 <= 10) {
        messageBase = MESSNUM(MESS_QUIZ_CHARA, 103);
        lbl_1_bss_500 = 0;
    } else {
        messageBase = MESSNUM(MESS_QUIZ_CHARA, 114);
        lbl_1_bss_500 = 10;
    }
    lbl_1_bss_504 = HuMCSelWinItemSet(messageBase, 11, (s16) padNo);
}

/* Handle answer timeout, close recognition, and present the correct answer. */
void fn_1_2DCEC(void)
{
    s8 playerIndex;
    u16 selectedValue;
    s32 currentState;
    f32 verticalOffset;
    f32 angle;
    f32 amplitude;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_524 != 2);

    fn_1_2E6C4(1);
    fn_1_266A0(0);
    HuMCSelWinContextKill();

    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 17), 1, 1);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 80), 1, 3);
    }
    fn_1_20C94();
    HuPrcSleep(120);

    lbl_1_bss_52C = 0;
    lbl_1_bss_4D8->responseRecord[0] = 2;
    currentState = lbl_1_bss_52C;
    selectedValue = lbl_1_bss_520;
    playerIndex = (s8)lbl_1_bss_53C;
    lbl_1_bss_4D8->answeringPlayer = playerIndex;
    lbl_1_bss_4D8->roundPoints = (s16)selectedValue;
    lbl_1_bss_4D8->roundResult = currentState;
    lbl_1_bss_508 = 4;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_4D8->presentationBusy != 0);

    if (lbl_1_bss_4 == 0) {
        lbl_1_bss_4 = 1;
        amplitude = (-362.0f);
        angle = (0.0f);
        if (lbl_1_bss_4DC != 0) {
            do {
                HuPrcVSleep();
                verticalOffset = amplitude * sin(((3.141592653589793) * (f64)angle) / (180.0));
                HuSprPosSet(lbl_1_bss_46C, 0, lbl_1_bss_4E4, lbl_1_bss_4E0 + verticalOffset);
                angle += (3.0f);
            } while (!(angle >= (90.0f)));
            lbl_1_bss_4E0 = (-152.0f);
            HuSprPosSet(lbl_1_bss_46C, 0, lbl_1_bss_4E4, lbl_1_bss_4E0);
            HuSprAttrSet(lbl_1_bss_46C, 0, HUSPR_ATTR_DISPOFF);
            lbl_1_bss_4DC = 0;
        }

        if (lbl_1_bss_544 == 0) {
            fn_1_2F76C(lbl_1_bss_514);
        } else if (lbl_1_bss_544 == 1) {
            fn_1_2F9E4(lbl_1_bss_514);
        } else if (lbl_1_bss_544 == 2) {
            fn_1_2F52C(lbl_1_data_E8[lbl_1_bss_514]);
        } else if (lbl_1_bss_544 == 3) {
            fn_1_2F52C(lbl_1_data_E8[lbl_1_bss_514]);
        } else if (lbl_1_bss_544 == 4) {
            fn_1_2F52C(lbl_1_data_5C[lbl_1_bss_514]);
        } else if (lbl_1_bss_544 == 5) {
            fn_1_2F52C(lbl_1_data_1CC[lbl_1_bss_514]);
        } else if (lbl_1_bss_544 == 6) {
            fn_1_2F52C(lbl_1_data_1D4[lbl_1_bss_514]);
        }
    }

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_4D8->presentationBusy != 0);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Handle the alternate answer timeout and finish its presenter animation. */
void fn_1_2E2B0(void) {
    s8 playerIndex;
    u16 selectedValue;
    s32 currentState;
    do
    {
        HuPrcVSleep();
    }
    while (lbl_1_bss_524 != 2);
    fn_1_2E6C4(1);
    fn_1_266A0(0);
    HuMCSelWinContextKill();
    fn_1_208E4();
    fn_1_3186C(1, (16.0f), HU3D_MOTATTR_LOOP);
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 17), 1, 1);
    } else {
        fn_1_20B74(3);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 80), 1, 3);
    }
    fn_1_20C94();
    HuPrcSleep(120);
    lbl_1_bss_52C = 0;
    lbl_1_bss_4D8->responseRecord[0] = 2;
    currentState = lbl_1_bss_52C;
    selectedValue = lbl_1_bss_520;
    playerIndex = (s8) lbl_1_bss_53C;
    lbl_1_bss_4D8->answeringPlayer = playerIndex;
    lbl_1_bss_4D8->roundPoints = (s16) selectedValue;
    lbl_1_bss_4D8->roundResult = currentState;
    HuPrcSleep(120);
    lbl_1_bss_508 = 4;
    HuPrcEnd();
    for (;;)
    {
        HuPrcVSleep();
    }
}

/* Start the requested microphone HUD transition only when its state changes. */
void fn_1_2E6C4(s32 motionState) {
    if (motionState == 0) {
        if (lbl_1_data_1FC[0] != motionState) {
            lbl_1_data_1FC[0] = motionState;
            HuPrcChildCreate(fn_1_2E828, 100U, 6144U, 0, lbl_1_bss_548);
        }
    } else if (motionState == 1) {
        if (lbl_1_data_1FC[0] != motionState) {
            lbl_1_data_1FC[0] = motionState;
            HuPrcChildCreate(fn_1_2EAA0, 100U, 6144U, 0, lbl_1_bss_548);
        }
    } else if (motionState == 2) {
        if (lbl_1_data_1FC[0] != motionState) {
            lbl_1_data_1FC[0] = motionState;
            HuPrcChildCreate(fn_1_2EBB0, 100U, 6144U, 0, lbl_1_bss_548);
        }
    } else if (motionState == 3 && lbl_1_data_1FC[0] != motionState) {
        lbl_1_data_1FC[0] = motionState;
        HuPrcChildCreate(fn_1_2ECA8, 100U, 6144U, 0, lbl_1_bss_548);
    }
}

/* Reveal the answer timer and transition the microphone HUD into its idle display. */
void fn_1_2E828(void) {
    f32 angle;
    f32 transparency;
    f32 scale;
    s32 frame;

    HuSprGrpAttrReset(lbl_1_bss_460[0], HUSPR_ATTR_DISPOFF);
    scale = (0.0f);
    angle = (0.0f);
    for (frame = 0; frame < 20; frame++) {
        HuPrcVSleep();
        scale = (f32)((2.0) - sin(((3.141592653589793) * (f64)angle) / (180.0)));
        HuSprGrpScaleSet(lbl_1_bss_460[0], scale, scale);
        angle += (4.5f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    HuSprGrpScaleSet(lbl_1_bss_460[0], (1.0f), (1.0f));
    HuSprBankSet(lbl_1_bss_468, 0, 1);
    angle = (0.0f);
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparency = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_466, 0, transparency);
        angle += (9.0f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    HuSprAttrReset(lbl_1_bss_46A, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(lbl_1_bss_468, 0, HUSPR_ATTR_DISPOFF);
    angle = (90.0f);
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparency = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_466, 0, transparency);
        angle -= (9.0f);
        if (angle <= (45.0f)) {
            break;
        }
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Fade the microphone glow and restore its idle animation after listening. */
void fn_1_2EAA0(void) {
    f32 angle = (90.0f);
    f32 tpLevel;
    s32 count;

    for (count = 0; count < 10; count++) {
        HuPrcVSleep();
        tpLevel = (f32)((0.5) * sin(((3.141592653589793) * (f64)angle) / (180.0)));
        HuSprTPLvlSet(lbl_1_bss_466, 0, tpLevel);
        angle += (9.0f);
        if (angle <= (0.0f)) {
            break;
        }
    }

    HuSprAttrSet(lbl_1_bss_46A, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(lbl_1_bss_468, 0, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_468, 0, 1);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Fade in the microphone glow and show the listening animation. */
void fn_1_2EBB0(void) {
    f32 angle = (0.0f);
    s32 frame;

    for (frame = 0; frame < 10; frame++) {
        f32 glowAlpha;

        HuPrcVSleep();
        glowAlpha = (f32) ((0.5) * sin(((3.141592653589793) * (f64) angle) / (180.0)));
        HuSprTPLvlSet(lbl_1_bss_466, 0, glowAlpha);
        angle += (9.0f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    HuSprAttrReset(lbl_1_bss_46A, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(lbl_1_bss_468, 0, HUSPR_ATTR_DISPOFF);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Flash the microphone glow and switch to its fast completed-response animation. */
void fn_1_2ECA8(void) {
    f32 angle;
    f32 transparencyLevel;
    s32 frame;

    angle = (0.0f);
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_466, 0, transparencyLevel);
        angle += (9.0f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    HuSprBankSet(lbl_1_bss_468, 0, 1);
    angle = (90.0f);
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_466, 0, transparencyLevel);
        angle += (9.0f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    HuSprBankSet(lbl_1_bss_468, 0, 0);
    angle = (90.0f);
    for (frame = 0; frame < 3; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_466, 0, transparencyLevel);
        angle -= (30.0f);
        if (angle <= (0.0f)) {
            break;
        }
    }
    HuSprAttrReset(lbl_1_bss_46A, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(lbl_1_bss_468, 0, HUSPR_ATTR_DISPOFF);
    HuSprSpeedSet(lbl_1_bss_46A, 0, (3.0f));
    angle = (0.0f);
    for (frame = 0; frame < 10; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        HuSprTPLvlSet(lbl_1_bss_466, 0, transparencyLevel);
        angle += (9.0f);
        if (angle >= (90.0f)) {
            break;
        }
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Insert the selected answer into the normal or alternate response message. */
void fn_1_2EF84(s32 messageType, s32 alternate)
{
    extern s32 lbl_1_bss_500;
    extern s32 lbl_1_bss_510;
    extern u32 lbl_1_data_1CC[2];
    extern u32 lbl_1_data_1D4[8];
    extern u32 lbl_1_data_5C[17];
    extern u32 lbl_1_data_E8[17];
    s32 messageIndex;
    s32 insertMessage;

    if (HuMCSelWinCheck() != 0) {
        if (messageType == 0 || messageType == 1) {
            messageIndex = lbl_1_bss_510 + lbl_1_bss_500 - 1;
        } else {
            messageIndex = lbl_1_bss_510 - 1;
        }
    } else {
        messageIndex = lbl_1_bss_510 - 1;
    }

    switch (messageType) {
    case 0:
        insertMessage = messageIndex + MESSNUM(MESS_QUIZ_CHARA, 34);
        break;
    case 1:
        insertMessage = messageIndex + MESSNUM(MESS_QUIZ_CHARA, 58);
        break;
    case 2:
        insertMessage = lbl_1_data_E8[messageIndex];
        break;
    case 3:
        insertMessage = lbl_1_data_E8[messageIndex];
        break;
    case 4:
        insertMessage = lbl_1_data_5C[messageIndex];
        break;
    case 5:
        insertMessage = lbl_1_data_1CC[messageIndex];
        break;
    case 6:
        insertMessage = lbl_1_data_1D4[messageIndex];
        break;
    }

    if (alternate == 0) {

        if (lbl_1_bss_4D8->presenterLayout == 0) {

            fn_1_20B74(1);

            fn_1_20D08(1, insertMessage, 0);

            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 16), 1, 1);

        } else {

            fn_1_20B74(3);

            fn_1_20D08(3, insertMessage, 0);

            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 79), 1, 3);

        }

        fn_1_20C94();

    } else {

        if (lbl_1_bss_4D8->presenterLayout == 0) {

            fn_1_20B74(1);

            fn_1_20D08(1, insertMessage, 0);

            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 27), 1, 1);

        } else {

            fn_1_20B74(3);

            fn_1_20D08(3, insertMessage, 0);

            fn_1_20A50(MESSNUM(MESS_MICQUIZ, 83), 1, 3);

        }

        fn_1_20C94();

    }
}

/* Present a supplied answer message using the current dialogue layout. */
void fn_1_2F52C(u32 message) {
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(2);
        fn_1_20D08(2, message, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 29), 1, 2);
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, message, 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    fn_1_20C94();
}

/* Present the selected answer from the first answer-name table. The initial equality test
 * discards its result and leaves messageIndex unchanged for the index-minus-one lookup. */
void fn_1_2F76C(s32 messageIndex) {
    messageIndex == 0;
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(2);
        fn_1_20D08(2, lbl_1_data_12C[messageIndex - 1], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 29), 1, 2);
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, lbl_1_data_12C[messageIndex - 1], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    fn_1_20C94();
}

/* Present the selected answer from the alternate answer-name table. The initial equality test
 * discards its result and leaves messageIndex unchanged for the index-minus-one lookup. */
void fn_1_2F9E4(s32 messageIndex) {
    messageIndex == 0;
    if (lbl_1_bss_4D8->presenterLayout == 0) {
        fn_1_20B74(2);
        fn_1_20D08(2, lbl_1_data_17C[messageIndex - 1], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 29), 1, 2);
    } else {
        fn_1_20B74(3);
        fn_1_20D08(3, lbl_1_data_17C[messageIndex - 1], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    fn_1_20C94();
}

void fn_1_2FC5C(void)
{
}

/* Start the temporary quiz message beneath the active quiz process. */
void fn_1_2FC60(void) {
    HuPrcChildCreate(fn_1_2FD28, 100U, 6144U, 0, lbl_1_bss_548);
}

/* Schedule removal of the temporary quiz message if its window exists. */
void fn_1_2FCA0(HUPROCESS* parent) {
    if(lbl_1_data_208!=-1)
        HuPrcChildCreate(fn_1_2FCF0,100,6144,0,parent);
}

/* Remove the temporary quiz message window and clear its active handle. */
void fn_1_2FCF0(void) {
    HuWinKill(lbl_1_data_208);
    lbl_1_data_208 = -1;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Show the temporary quiz message for three seconds, then close and remove it. */
void fn_1_2FD28(void) {
    HuVec2f messageSize;

    HuWinMesMaxSizeGet(1, &messageSize, MESSNUM(MESS_MICQUIZ, 58));
    if (lbl_1_data_208 != -1) {
        HuPrcEnd();
        for (;;) {
            HuPrcVSleep();
        }
    }
    lbl_1_data_208 = HuWinExCreateFrame((-10000.0f), (200.0f),
        messageSize.x, messageSize.y, -1, 0);
    HuWinExOpen(lbl_1_data_208);
    HuWinMesSpeedSet(lbl_1_data_208, 0);
    HuWinAttrSet(lbl_1_data_208, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_data_208, MESSNUM(MESS_MICQUIZ, 58));
    HuWinMesWait(lbl_1_data_208);
    HuPrcSleep(180);
    if (lbl_1_data_208 != -1) {
        HuWinExClose(lbl_1_data_208);
        HuWinKill(lbl_1_data_208);
        lbl_1_data_208 = -1;
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Initialize the answer countdown from the selected level and question event. */
void fn_1_2FE74(void)
{
    u8 currentIndex;
    s32 firstMode;
    u8 nextIndex;
    s32 nextMode;
    u8 previousIndex;
    u8 fallbackIndex;
    s32 timingMode;

    lbl_1_bss_524 = 1;
    currentIndex = lbl_1_bss_4D8->questionLevel;
    lbl_1_bss_520 = lbl_1_data_48[currentIndex];
    firstMode = lbl_1_bss_4D8->questionEvent;

    if (firstMode == 2) {
        nextIndex = lbl_1_bss_4D8->questionLevel;
        lbl_1_bss_520 = lbl_1_data_48[nextIndex + 1];
    } else {
        nextMode = lbl_1_bss_4D8->questionEvent;
        if (nextMode == 5) {
            previousIndex = lbl_1_bss_4D8->questionLevel;
            lbl_1_bss_520 = lbl_1_data_48[previousIndex - 1];
        } else {
            fallbackIndex = lbl_1_bss_4D8->questionLevel;
            lbl_1_bss_520 = lbl_1_data_48[fallbackIndex];
        }
    }

    timingMode = lbl_1_bss_4D8->questionEvent;
    /* This event gives each countdown step half the usual number of frames. */
    if (timingMode == 4) {
        lbl_1_bss_51C = 900 / lbl_1_bss_520;
    } else {
        lbl_1_bss_51C = 1800 / lbl_1_bss_520;
    }
    lbl_1_bss_518 = 0;
    fn_1_3011C(lbl_1_bss_520);
}

/* Advance an active answer countdown and update its digits until time expires. */
s32 fn_1_3003C(void) {
    if ((s32) lbl_1_bss_524 == 0) {
        lbl_1_bss_518 += 1;
        if ((s32) lbl_1_bss_518 >= (s32) lbl_1_bss_51C) {
            HuAudFXPlay(MSM_SE_MICQUIZ_COUNTDOWN_TICK);
            lbl_1_bss_520 -= 1;
            lbl_1_bss_518 = 0;
            fn_1_3011C(lbl_1_bss_520);
            if ((s32) lbl_1_bss_520 <= 0) {
                lbl_1_bss_524 = 2;
            }
        }
    }
    return lbl_1_bss_524;
}

/* Set the two answer countdown digits when its value is nonnegative. */
void fn_1_3011C(s32 pointsRemaining) {
    if (pointsRemaining >= 0) {
        s32 tens = pointsRemaining / 10;
        s32 ones = pointsRemaining - tens * 10;
        HuSprBankSet(lbl_1_bss_460[0], 0, tens);
        HuSprBankSet(lbl_1_bss_460[0], 1, ones);
    }
}

/* Choose an integer from the requested minimum up to, but excluding, the maximum. */
s32 fn_1_301AC(s32 minimum, s32 maximum) {
    s32 unused;
    return minimum + frandmod(maximum - minimum);
}

/* Test a percentage chance using a random value from zero through ninety-nine. */
s32 fn_1_301E8(s32 percentChance) {
    s32 roll;

    roll = frandmod(100);
    if (percentChance == 0) {
        return 0;
    }
    if (roll < percentChance) {
        return 1;
    }
    return 0;
}

/* Present one of three accepted-answer exchanges with the animated presenter. */
void fn_1_30248(s32 primaryMessageIndex, s32 secondaryMessageIndex) {
    extern s32 lbl_1_bss_508;
    extern u32 lbl_1_data_1D4[8];
    extern u32 lbl_1_data_A0[18];
    s32 randomChoice;
    s32 previousSelection;

    randomChoice = frandmod(3);
    previousSelection = lbl_1_bss_508;
    fn_1_30FC4();
    lbl_1_bss_508 = 5;
    lbl_1_bss_4D8->presentationBusy = 1;
    do {
        HuPrcVSleep();
    } while (lbl_1_bss_4D8->presentationBusy != 0);
    lbl_1_bss_508 = previousSelection;
    HuPrcSleep(60);

    fn_1_20B74(3);
    fn_1_3186C(1, (16.0f), HU3D_MOTATTR_LOOP);

    if (randomChoice == 0) {
        fn_1_20D08(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
        fn_1_20D08(3, lbl_1_data_A0[primaryMessageIndex], 1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 63), 1, 3);
        fn_1_20C94();
    } else if (randomChoice == 1) {
        fn_1_20D08(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 64), 1, 3);
        fn_1_20C94();
        HuPrcSleep(120);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 65), 1, 3);
        fn_1_20C94();
    } else {
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 66), 1, 3);
        fn_1_20C94();
        HuPrcSleep(120);
        fn_1_20D08(3, lbl_1_data_A0[primaryMessageIndex], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 67), 1, 3);
        fn_1_20C94();
    }
}

/* Present one of four follow-up exchanges for a recognized answer choice. */
void fn_1_306A0(s32 primaryMessageIndex, s32 secondaryMessageIndex)
{
    s32 randomChoice = frandmod(4);

    if (randomChoice != 0) {
        s32 previousSelection = lbl_1_bss_508;

        fn_1_30FC4();
        lbl_1_bss_508 = 5;
        lbl_1_bss_4D8->presentationBusy = 1;
        do {
            HuPrcVSleep();
        } while (lbl_1_bss_4D8->presentationBusy != 0);
        lbl_1_bss_508 = previousSelection;
        HuPrcSleep(60);
    }

    fn_1_20B74(3);
    fn_1_3186C(1, (16.0f), HU3D_MOTATTR_LOOP);

    if (randomChoice == 0) {
        fn_1_20D08(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 68), 1, 3);
    } else if (randomChoice == 1) {
        fn_1_20D08(3, lbl_1_data_A0[primaryMessageIndex], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 69), 1, 3);
    } else if (randomChoice == 2) {
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 70), 1, 3);
        fn_1_20C94();
        HuPrcSleep(120);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 71), 1, 3);
        fn_1_20C94();
    } else {
        fn_1_20D08(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 72), 1, 3);
        fn_1_20C94();
        HuPrcSleep(120);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 73), 1, 3);
        fn_1_20C94();
    }
    fn_1_20C94();
}

/* Present one of two follow-up exchanges for a recognized flagged answer choice. */
void fn_1_30B0C(s32 primaryMessageIndex, s32 secondaryMessageIndex)
{
    s32 randomChoice = frandmod(2);

    if (randomChoice != 0) {
        s32 previousSelection = lbl_1_bss_508;

        fn_1_30FC4();
        lbl_1_bss_508 = 5;
        lbl_1_bss_4D8->presentationBusy = 1;
        do {
            HuPrcVSleep();
        } while (lbl_1_bss_4D8->presentationBusy != 0);
        lbl_1_bss_508 = previousSelection;
        HuPrcSleep(60);
    }

    fn_1_20B74(3);
    fn_1_3186C(1, (16.0f), HU3D_MOTATTR_LOOP);

    if (randomChoice == 0) {
        fn_1_20D08(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
        fn_1_20D08(3, lbl_1_data_A0[primaryMessageIndex], 1);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 74), 1, 3);
        fn_1_20C94();
        HuPrcSleep(120);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 75), 1, 3);
        fn_1_20C94();
    } else {
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 76), 1, 3);
        fn_1_20C94();
        HuPrcSleep(120);
        fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 77), 1, 3);
        HuPrcSleep(120);
    }
}

/* Present the follow-up for an answer absent from the active choice records. */
void fn_1_30E54(s32 messageIndex) {

    fn_1_20B74(3);
    fn_1_3186C(1, (16.0f), HU3D_MOTATTR_LOOP);
    fn_1_20D08(3, lbl_1_data_1D4[messageIndex], 0);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 79), 1, 3);
    fn_1_20C94();
}

/* Present and wait for the line preceding a presenter reaction. */
void fn_1_30FC4(void) {
    fn_1_20B74(3);
    fn_1_20A50(MESSNUM(MESS_MICQUIZ_Q, 62), 1, 3);
    if (lbl_1_bss_37A != -1) {
        HuWinMesWait(lbl_1_bss_37A);
    }
}

/* Return the zero-based index of the current recognized answer. */
s32 fn_1_310DC(void) {
    return lbl_1_bss_510 - 1;
}

/* Present accepted or rejected answer feedback and animate the presenter accordingly. */
void fn_1_310F0(s32 success, s32 choice) {
    s32 slot;

    fn_1_208E4();
    HuPrcSleep(30);

    if (success != 0) {
        fn_1_30248(lbl_1_bss_4D8->playerCharacter[choice], lbl_1_bss_510 - 1);
        HuAudFXPlay(MSM_SE_CMN_11);
        lbl_1_bss_378 = lbl_1_bss_390;
        if (lbl_1_data_1FC[0] != 3) {
            lbl_1_data_1FC[0] = 3;
            HuPrcChildCreate(fn_1_2ECA8, 100U, 6144U, 0, lbl_1_bss_548);
        }
        HuPrcChildCreate(fn_1_275CC, 100U, 6144U, 0, lbl_1_bss_548);
        HuAudFXPlay(MSM_SE_GUIDE_47);
        fn_1_3186C(3, 8.0f, HU3D_MOTATTR_LOOP);
        HuPrcSleep(120);
        return;
    }

    if (fn_1_316F4(lbl_1_bss_510 - 1) != 0) {
        slot = fn_1_316A4(lbl_1_bss_510 - 1);
        if (lbl_1_bss_54C[slot].alternateFollowup != 0) {
            fn_1_30B0C(lbl_1_bss_4D8->playerCharacter[choice], lbl_1_bss_510 - 1);
        } else {
            fn_1_306A0(lbl_1_bss_4D8->playerCharacter[choice], lbl_1_bss_510 - 1);
        }
    } else {
        fn_1_30E54(lbl_1_bss_510 - 1);
    }

    HuAudFXPlay(MSM_SE_CMN_12);
    lbl_1_bss_378 = lbl_1_bss_38E;
    HuPrcChildCreate(fn_1_275CC, 100U, 6144U, 0, lbl_1_bss_548);
    fn_1_3186C(6, 8.0f, HU3D_MOTATTR_LOOP);
    HuPrcSleep(120);
}

/* Play the presenter motion cue at the marked frames of its active motion. */
void fn_1_315C8(void) {
    f32 motionTime;
    s16 motionId;

    for (;;) {
        HuPrcVSleep();
        motionTime = Hu3DMotionTimeGet(lbl_1_bss_376);
        motionId = Hu3DMotionIDGet(lbl_1_bss_376);
        if (motionId == lbl_1_bss_368[6]) {
            if (((20.0f) == motionTime) || ((55.0f) == motionTime)) {
                HuAudFXPlay(MSM_SE_GUIDE_45);
            }
            continue;
        }
        if (motionId != lbl_1_bss_368[2]) {
            continue;
        }
        if (((10.0f) != motionTime) && ((60.0f) != motionTime)) {
            continue;
        }
        HuAudFXPlay(MSM_SE_GUIDE_45);
    }
}

/* Find an answer key in the eight choice records, returning minus one if absent. */
int fn_1_316A4(int key) {
    int index;
    for(index=0;index<8;index++) {
        if(key==lbl_1_bss_54C[index].answerIndex) return index;
    }
    return -1;
}

/* Test whether an answer key has an available choice record. */
int fn_1_316F4(int key) {
    int index;
    for (index = 0; index < 8; index++) {
        if (key == lbl_1_bss_54C[index].answerIndex &&
            lbl_1_bss_54C[index].availabilityState == 0) {
            return 1;
        }
    }
    return 0;
}

/* Create the presenter model and load its seven dialogue motions. */
void fn_1_31760(void) {
    s32 index;

    lbl_1_bss_376 = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_micquizishi, 121), HU_MEMNUM_OVL, HEAP_MODEL));
    for (index = 0; index < 7; index++) {
        lbl_1_bss_368[index] = Hu3DJointMotion(
            lbl_1_bss_376,
            HuDataSelHeapReadNum(lbl_1_data_0[index], HU_MEMNUM_OVL, HEAP_MODEL)
        );
    }
    Hu3DModelAttrSet(lbl_1_bss_376, HU3D_MOTATTR_LOOP);
    fn_1_3186C(0, (0.0f), HU3D_MOTATTR_LOOP);
    Hu3DModelPosSet(lbl_1_bss_376, (0.0f), (0.0f), -410.0f);
}

/* Change the presenter motion only when the requested motion is not already active. */
void fn_1_3186C(s32 motionNo, f32 blendFrames, u32 attr) {
    HU3D_MOTIONID currentMotion;

    currentMotion = Hu3DMotionIDGet(lbl_1_bss_376);
    if (currentMotion != lbl_1_bss_368[motionNo]) {
        Hu3DMotionShiftSet(lbl_1_bss_376, lbl_1_bss_368[motionNo],
                           (0.0f), blendFrames, attr);
    }
}

/* Enable or disable pausing as the response process enters a protected interval. */
void fn_1_31910(s32 pauseAllowed) {
    lbl_1_bss_4D8->pauseAllowed = pauseAllowed;
}

/* Shrink and hide the selected player portrait on the microphone HUD. */
void fn_1_31924(int playerIndex) {
    s16 sprite = (s16)(playerIndex + 4);
    f32 scale;
    f32 angle;

    angle = (90.0f);
    do {
        HuPrcVSleep();
        scale = (f32) sin(((3.141592653589793) * (f64) angle) / (180.0));
        HuSprScaleSet(lbl_1_bss_5CC, sprite, scale, scale);
        angle += (9.0f);
    } while (!(angle >= (180.0f)));
    HuSprAttrSet(lbl_1_bss_5CC, sprite, HUSPR_ATTR_DISPOFF);
}

/* Scale the selected player portrait into view on the microphone HUD. */
void fn_1_31A0C(s32 playerIndex) {
    f32 scale;
    f32 angle;
    s16 spriteIndex;

    spriteIndex = playerIndex + 4;
    angle = (0.0f);
    HuSprScaleSet(lbl_1_bss_5CC, spriteIndex, (0.0f), (0.0f));
    HuSprAttrReset(lbl_1_bss_5CC, spriteIndex, HUSPR_ATTR_DISPOFF);
    do {
        HuPrcVSleep();
        scale = (f32) sin(((3.141592653589793) * (f64) angle) / (180.0));
        HuSprScaleSet(lbl_1_bss_5CC, spriteIndex, scale, scale);
        angle += (9.0f);
    } while (!(angle >= (90.0f)));
}

/* Scale the microphone HUD response marker into view. */
void fn_1_31B20(void) {
    f32 scale;
    f32 angle;

    angle = (0.0f);
    HuSprScaleSet(lbl_1_bss_5CC, 3, (0.0f), (0.0f));
    HuSprAttrReset(lbl_1_bss_5CC, 3, HUSPR_ATTR_DISPOFF);
    do {
        HuPrcVSleep();
        scale = (f32) sin(((3.141592653589793) * (f64) angle) / (180.0));
        HuSprScaleSet(lbl_1_bss_5CC, 3, scale, scale);
        angle += (9.0f);
    } while (!(angle >= (90.0f)));
}

/* Shrink and hide the microphone HUD response marker. */
void fn_1_31C1C(void) {
    f32 scale;
    f32 angle;

    angle = (90.0f);
    do {
        HuPrcVSleep();
        scale = (f32) sin(((3.141592653589793) * (f64) angle) / (180.0));
        HuSprScaleSet(lbl_1_bss_5CC, 3, scale, scale);
        angle += (9.0f);
    } while (!(angle >= (180.0f)));
    HuSprAttrSet(lbl_1_bss_5CC, 3, HUSPR_ATTR_DISPOFF);
}

/* Play dialogue sound cues, panning selected speakers for the listed messages. */
void fn_1_31CEC(HUWINID windowId, u32 answerId, s16 soundIndex) {
    QuizAnswerIds answerIds = {
        { MESSNUM(MESS_MICQUIZ, 0),           MESSNUM(MESS_MICQUIZ, 6),
          MESSNUM(MESS_MICQUIZ, 12),          MESSNUM(MESS_MICQUIZ, 30),
          MESSNUM(MESS_MICQUIZ, 34),          MESSNUM(MESS_MICQUIZ, 35),
          MESSNUM(MESS_MICQUIZ, 36),          MESSNUM(MESS_MICQUIZ, 37),
          MESSNUM(MESS_MICQUIZ, 39),          MESSNUM(MESS_MICQUIZ, 40),
          MESSNUM(MESS_MICQUIZ, 54),          MESSNUM(MESS_MICQUIZ, 57),
          MESSNUM(MESS_MICQUIZ, 58),          MESSNUM(MESS_MICQUIZ_TUTORIAL, 40),
          MESSNUM(MESS_MICQUIZ_TUTORIAL, 41), MESSNUM(MESS_MICQUIZ_TUTORIAL, 42),
          MESSNUM(MESS_MICQUIZ_TUTORIAL, 44), MESSNUM(MESS_MICQUIZ_TUTORIAL, 45),
          MESSNUM(MESS_MICQUIZ_INTRO, 0),     -1 }
    };
    QuizSoundIds sounds = { { MSM_SE_GUIDE_26, MSM_SE_GUIDE_27, MSM_SE_GUIDE_28, MSM_SE_GUIDE_18,
                              MSM_SE_GUIDE_19, MSM_SE_GUIDE_20, MSM_SE_GUIDE_47, MSM_SE_GUIDE_48,
                              MSM_SE_GUIDE_49, MSM_SE_GUIDE_50, MSM_SE_GUIDE_51, MSM_SE_GUIDE_52,
                              MSM_SE_GUIDE_53, MSM_SE_GUIDE_54, MSM_SE_GUIDE_30, MSM_SE_GUIDE_22,
                              -MSM_SE_CMN_02 } };
    s16 answerIndex;

    soundIndex--;

    /* Only the listed dialogue messages separate these speaker cues by pan. */
    for (answerIndex = 0; ; answerIndex++) {
        if (answerIds.values[answerIndex] == -1) {
            HuAudFXPlay(sounds.values[soundIndex]);
            return;
        }
        if (answerId == (u32)answerIds.values[answerIndex]) {
            if (soundIndex == 0 || soundIndex == 1 ||
                soundIndex == 2 || soundIndex == 14) {
                HuAudFXPlayPan(sounds.values[soundIndex], 48);
                return;
            }
            if (soundIndex == 3 || soundIndex == 4 ||
                soundIndex == 5 || soundIndex == 15) {
                HuAudFXPlayPan(sounds.values[soundIndex], 80);
            }
            break;
        }
    }
}
