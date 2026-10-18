/* Window, sprite, and player-input helpers for the Mic Quiz selection menu. */
#include "datadir_enum.h"
#include "game/memory.h"
#include "msm_se.h"
#include "messdir_enum.h"

#define MICQUIZ_WINDOW_PLAYER_MASK 0xFF
#define MICQUIZ_PAUSE_PROCESS_FLAGS 0xC

void fn_1_122C(s32 value)
{
    lbl_1_bss_220 = value;
}

void fn_1_123C(void)
{
    lbl_1_bss_208 = 1;
}

/* Requests selection sequence 2 and marks the shared menu transition state. */
void fn_1_1250(void) {
    fn_1_F820(2);
    lbl_1_bss_24C = 1;
    lbl_1_bss_214 = 1;
}

s32 fn_1_1294(void) {
    return lbl_1_bss_214;
}

/* Refreshes six stored window input masks; initial setup calls this before the two pause-window
 * handles are assigned. */
void fn_1_12A4(void)
{
    s32 playerIndex;
    u8 disabledPlayerMask;

    disabledPlayerMask = 0;
    playerIndex = 0;
    while (playerIndex < 4) {
        if (fn_1_9148(playerIndex) == 0) {
            disabledPlayerMask |= 1 << playerIndex;
        }
        playerIndex += 1;
    }
    HuWinDisablePlayerReset(lbl_1_bss_92[0], MICQUIZ_WINDOW_PLAYER_MASK);
    HuWinDisablePlayerReset(lbl_1_bss_90, MICQUIZ_WINDOW_PLAYER_MASK);
    HuWinDisablePlayerReset(lbl_1_bss_8E, MICQUIZ_WINDOW_PLAYER_MASK);
    HuWinDisablePlayerReset(lbl_1_bss_8C, MICQUIZ_WINDOW_PLAYER_MASK);
    HuWinDisablePlayerReset(lbl_1_bss_B6, MICQUIZ_WINDOW_PLAYER_MASK);
    HuWinDisablePlayerReset(lbl_1_bss_B4, MICQUIZ_WINDOW_PLAYER_MASK);
    HuWinDisablePlayerSet(lbl_1_bss_92[0], disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_90, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_8E, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_8C, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_B6, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_B4, disabledPlayerMask);
}

/* Creates the menu's prompt windows and assigns the answer-sound callback. */
void fn_1_13F8(void) {
    u8 colorA[4] = {0, 0, 0, 255};
    u8 colorB[4] = {0, 255, 0, 255};
    s32 playerIndex;

    lbl_1_bss_92[0] = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 6, 4);
    lbl_1_bss_90 = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 7, 3);
    lbl_1_bss_8E = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 23, 5);
    lbl_1_bss_8C = HuWinExCreateFrame(16.0f, 344.0f, 544, 96, 13, 0);

    fn_1_12A4();
    HuWinAttrSet(lbl_1_bss_92[0], HUWIN_ATTR_UPAUSE);
    HuWinAttrSet(lbl_1_bss_90, HUWIN_ATTR_UPAUSE);
    HuWinAttrSet(lbl_1_bss_8E, HUWIN_ATTR_UPAUSE);
    HuWinAttrSet(lbl_1_bss_8C, HUWIN_ATTR_UPAUSE);

    lbl_1_bss_8A = HuWinCreate(16.0f, 312.0f, 544, 32, 0);
    HuWinBGTPLvlSet(lbl_1_bss_8A, 0.0f);
    HuWinMesSpeedSet(lbl_1_bss_8A, 0);
    HuWinAttrSet(lbl_1_bss_8A, HUWIN_ATTR_ALIGN_CENTER);
    HuWinDispOff(lbl_1_bss_92[0]);
    HuWinDispOff(lbl_1_bss_90);
    HuWinDispOff(lbl_1_bss_8E);
    HuWinDispOff(lbl_1_bss_8C);
    HuWinPriSet(lbl_1_bss_92[0], 100);
    HuWinPriSet(lbl_1_bss_90, 100);
    HuWinPriSet(lbl_1_bss_8E, 100);
    HuWinPriSet(lbl_1_bss_8C, 100);
    HuWinPriSet(lbl_1_bss_8A, 100);
    HuWinCallbackSet(lbl_1_bss_92[0], fn_1_12E48);
    HuWinCallbackSet(lbl_1_bss_90, fn_1_12E48);
    HuWinCallbackSet(lbl_1_bss_8E, fn_1_12E48);
    HuWinCallbackSet(lbl_1_bss_8C, fn_1_12E48);
    lbl_1_bss_86 = -1;
}

/* Selects one of the four prompt windows for the message helpers below. */
s16 fn_1_1858(s32 windowNo) {
    s16 window;

    switch (windowNo) {
    case 0:
        window = lbl_1_bss_92[0];
        break;
    case 1:
        window = lbl_1_bss_90;
        break;
    case 2:
        window = lbl_1_bss_8E;
        break;
    case 3:
        window = lbl_1_bss_8C;
        break;
    }
    return window;
}

/* Slides the quiz heading into view when the answer flow requests it. */
void fn_1_18D4(void) {
    f32 slideDistance;
    f32 angleDegrees;
    f32 sineOffset;

    slideDistance = (362.0f);
    HuSprAttrReset(lbl_1_bss_178, 0, 4);
    angleDegrees = (0.0f);
    if ((s32) lbl_1_bss_1E8 == 0) {
        do {
            HuPrcVSleep();
            sineOffset = (f32) ((f64) slideDistance *
                                sin(((3.141592653589793) * (f64) angleDegrees) / (180.0)));
            HuSprPosSet(lbl_1_bss_178, 0, lbl_1_bss_1F0, lbl_1_bss_1EC + sineOffset);
            angleDegrees += (3.0f);
        } while (!(angleDegrees >= (90.0f)));
        lbl_1_bss_1EC = (210.0f);
        HuSprPosSet(lbl_1_bss_178, 0, lbl_1_bss_1F0, lbl_1_bss_1EC);
        lbl_1_bss_1E8 = 1;
    }
}

/* Slides the quiz heading out of view during answer transitions and feedback. */
void fn_1_1A40(void) {
    f32 slideDistance;
    f32 angleDegrees;
    f32 sineOffset;

    slideDistance = (-362.0f);
    angleDegrees = (0.0f);
    if ((s32) lbl_1_bss_1E8 != 0) {
        do {
            HuPrcVSleep();
            sineOffset = (f32) ((f64) slideDistance *
                                sin(((3.141592653589793) * (f64) angleDegrees) / (180.0)));
            HuSprPosSet(lbl_1_bss_178, 0, lbl_1_bss_1F0, lbl_1_bss_1EC + sineOffset);
            angleDegrees += (3.0f);
        } while (!(angleDegrees >= (90.0f)));
        lbl_1_bss_1EC = (-152.0f);
        HuSprPosSet(lbl_1_bss_178, 0, lbl_1_bss_1F0, lbl_1_bss_1EC);
        HuSprAttrSet(lbl_1_bss_178, 0, 4);
        lbl_1_bss_1E8 = 0;
    }
}

/* Starts text in the selected prompt window at the requested reveal speed. */
void fn_1_1BAC(u32 messageId, s16 messageSpeed, u8 windowIndex) {
    s16 window;

    window = fn_1_1858(windowIndex);
    HuWinAttrSet(window, HUWIN_ATTR_NOCANCEL);
    HuWinMesSet(window, messageId);
    HuWinMesSpeedSet(window, messageSpeed);
}

/* Closes the current prompt window when one is open. */
void fn_1_1C80(void) {
    if (lbl_1_bss_86 != -1) {
        HuWinExClose(lbl_1_bss_86);
        lbl_1_bss_86 = -1;
    }
}

/* Opens the requested prompt window, closing the previous one first. */
void fn_1_1CD0(s32 mode) {
    s16 window;

    window = fn_1_1858(mode);
    if (lbl_1_bss_86 != window) {
        if (lbl_1_bss_86 != -1) {
            fn_1_1C80();
        }
        HuWinExOpen(window);
        lbl_1_bss_86 = window;
    }
}

/* Waits for the active prompt window's message or choice interaction to finish. */
void fn_1_1DF0(void) {
    if (lbl_1_bss_86 != -1) {
        HuWinMesWait(lbl_1_bss_86);
    }
}

/* Returns the choice made in the active prompt window. */
s16 fn_1_1E30(s16 choice) {
    return HuWinChoiceGet(lbl_1_bss_86, choice);
}

/* Sets a message substitution slot in the selected prompt window. */
void fn_1_1E64(s32 windowNo, u32 messageId, s16 insertNo) {
    HuWinInsertMesSet(fn_1_1858(windowNo), messageId, insertNo);
}

/* Clears the message page in the selection menu's main window. */
void fn_1_1F18(void) {
    HuWinHomeClear(lbl_1_bss_88);
}

/* Clears the control-guide window's message page. */
void fn_1_1F44(void) {
    HuWinHomeClear(lbl_1_bss_8A);
}

/* Shows or hides the control-guide window. */
void fn_1_1F70(s32 displayOn) {
    if (displayOn) {
        HuWinDispOn(lbl_1_bss_8A);
        return;
    }
    HuWinDispOff(lbl_1_bss_8A);
}

/* Updates the control-guide message when its requested input state changes. */
void fn_1_1FC0(s32 index) {
    u32 messageId;

    if (lbl_1_data_1F4 != index) {
        switch ((u32)index) {
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
        HuWinMesSet(lbl_1_bss_8A, messageId);
        lbl_1_data_1F4 = index;
    }
}

/* Selects one of the two pause control-guide messages in the main status window. */
void fn_1_20AC(s32 messageType) {
    s32 messageId;

    switch (messageType) {
    case 5:
        messageId = MESSNUM(MESS_SYS_GUIDE, 28);
        break;
    case 6:
        messageId = MESSNUM(MESS_SYS_GUIDE, 29);
        break;
    }
    HuWinMesSet(lbl_1_bss_88, messageId);
}

/* Draws the quiz menu's scrolling texture during the overlay's GX draw pass. */
void fn_1_2118(s16 layer)
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

/* The overlay pause callback starts the pause/quit prompt process and records its camera mask. */
void fn_1_261C(HUPROCESS *parentProcess) {
    f32 perspective;
    f32 nearPlane;
    f32 farPlane;
    s32 cameraIndex;
    s32 cameraMask;

    lbl_1_bss_B0 = HuPrcChildCreate(fn_1_2700, -536, 8192, 0, HuPrcCurrentGet());
    HuPrcSetStat(lbl_1_bss_B0, MICQUIZ_PAUSE_PROCESS_FLAGS);
    fn_1_12A6C(0);
    cameraIndex = 0;
    while (cameraIndex < 16) {
        Hu3DCameraPerspectiveGet(1 << cameraIndex, &perspective, &nearPlane, &farPlane);
        if (-1.0f != perspective) {
            /* Keep the last camera with a configured perspective. */
            cameraMask = 1 << cameraIndex;
        }
        cameraIndex += 1;
    }
    /* With no configured perspective, cameraMask is left uninitialized. */
    lbl_1_bss_148 = cameraMask;
    lbl_1_bss_254 = parentProcess;
}

/* Waits for Start, opens the pause display, and handles resuming or confirming an overlay exit. */
void fn_1_2700(void) {
    u8 playerIndex;
    u8 selectedPlayer;
    s32 playerSelected;
    u8 disabledPlayers;
    s16 windowNo;
    u32 homeMessageId;
    u32 confirmMessageId;

    for (;;) {
        playerSelected = 0;
        for (;;) {
            HuPrcVSleep();
            if (WipeCheck() != 0) continue;
            if (omCurrentOvlGet() == DLL_NONE) continue;
            if (GameMesStatGet(-1) != 0) continue;
            if (HuMCSelModeGet() == 1) continue;
            if (omUPauseFlag + omPauseChk() != 0) continue;
            if (lbl_1_bss_1E4->startPromptEnabled == 0) continue;

            for (playerIndex = 0; playerIndex < 4U; playerIndex++) {
                if (omPadErrChk((u8) playerIndex) == 0 &&
                    (HuPadBtnDown[playerIndex] & 0x1000) != 0) {
                    selectedPlayer = playerIndex;
                    lbl_1_bss_1E4->drawMode = 1;
                    WipeCreate(2, 0, 10);
                    while (WipeCheck() != 0) {
                        HuPrcVSleep();
                    }
                    HuWinDispOff(lbl_1_bss_8A);
                    fn_1_2BF4();
                    HuWinHomeClear(lbl_1_bss_88);
                    homeMessageId = MESSNUM(MESS_SYS_GUIDE, 28);
                    HuWinMesSet(lbl_1_bss_88, homeMessageId);
                    playerSelected = 1;
                    WipeCreate(1, 0, 10);
                    while (WipeCheck() != 0) {
                        HuPrcVSleep();
                    }
                    break;
                }
            }
            if (playerSelected != 0) {
                break;
            }
        }

        for (;;) {
            HuPrcVSleep();
            if ((HuPadBtnDown[selectedPlayer] & 0x1000) != 0) {
        HuAudFXPlay(MSM_SE_CMN_06);
                WipeCreate(2, 0, 10);
                while (WipeCheck() != 0) {
                    HuPrcVSleep();
                }
                fn_1_2CDC();
                HuWinHomeClear(lbl_1_bss_88);
                HuWinDispOn(lbl_1_bss_8A);
                lbl_1_bss_1E4->drawMode = 0;
                WipeCreate(1, 0, 10);
                while (WipeCheck() != 0) {
                    HuPrcVSleep();
                }
                break;
            }
            if ((HuPadBtnDown[selectedPlayer] & 0x400) == 0) {
                continue;
            }

            HuAudFXPlay(MSM_SE_CMN_04);
            HuWinPosSet(lbl_1_bss_88, 16.0f, 312.0f);
            disabledPlayers = 0;
            for (playerIndex = 0; playerIndex < 4U; playerIndex++) {
                if (playerIndex != selectedPlayer) {
                    disabledPlayers |= 1 << playerIndex;
                }
            }
            if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
                windowNo = lbl_1_bss_B6;
            } else {
                windowNo = lbl_1_bss_B4;
            }
            HuWinDisablePlayerSet(windowNo, disabledPlayers);
            confirmMessageId = MESSNUM(MESS_SYS_GUIDE, 29);
            HuWinMesSet(lbl_1_bss_88, confirmMessageId);
            HuWinExOpen(windowNo);
            HuWinAttrSet(windowNo, HUWIN_ATTR_NOCANCEL);
            if (lbl_1_bss_1E4->useAlternatePrompt == 0) {
                HuWinMesSet(windowNo, MESSNUM(MESS_MICQUIZ, 47));
            } else {
                HuWinMesSet(windowNo, MESSNUM(MESS_MICQUIZ, 101));
            }
            HuWinMesSpeedSet(windowNo, 1);
            HuWinMesWait(windowNo);
            if (HuWinChoiceGet(windowNo, -1) == 0) {
                HuAudFXPlay(MSM_SE_CMN_06);
                WipeCreate(2, 0, 10);
                while (WipeCheck() != 0) {
                    HuPrcVSleep();
                }
                HuWinExClose(windowNo);
                HuWinHomeClear(lbl_1_bss_88);
                fn_1_2CDC();
                HuWinDispOn(lbl_1_bss_8A);
                lbl_1_bss_1E4->drawMode = 0;
                HuWinDisablePlayerReset(windowNo, HUWIN_PLAYER_ALL);
                WipeCreate(1, 0, 10);
                while (WipeCheck() != 0) {
                    HuPrcVSleep();
                }
                break;
            }

            HuAudSStreamFadeOut(lbl_1_bss_1F4, 600);
            WipeCreate(2, 0, 60);
            while (WipeCheck() != 0) {
                HuPrcVSleep();
            }
            fn_1_2CDC();
            HuMCClose();
            HuMCSessionClose();
            HuWinDisablePlayerReset(windowNo, HUWIN_PLAYER_ALL);
            if (lbl_1_bss_1E4->pauseOverlayMode != 0) {
                omOvlReturnEx(1, 1);
            } else {
                omOvlReturnEx(2, 1);
            }
        }
    }
}

/* Pauses gameplay systems and moves the menu UI to its pause draw layer. */
void fn_1_2BF4(void) {
    lbl_1_bss_A8 = (s32) omSysPauseEnableFlag;
    lbl_1_bss_AC = (s32) omPauseChk();
    omUPauseFlag = 1;
    omSysPauseEnable(0U);
    Hu3DCameraLayerHookSet((s16) lbl_1_bss_148, 15, fn_1_2118);
    if (lbl_1_bss_1E4->pauseOverlayMode != 0) {
        fn_1_7740();
        fn_1_6C90();
    } else {
        fn_1_7380();
    }
    fn_1_6DE4();
    omObjManPause(1);
    HuPrcAllPause(1);
    Hu3DPauseSet(1);
    HuSprPauseSet(1);
    HuPadRumbleAllStop();
    HuAudFXPauseAll(1);
    HuAudSStreamPauseAll(1);
    HuAudFXPlay(MSM_SE_CMN_06);
}

/* Restores the menu draw layer and resumes systems unless the system was already paused. */
void fn_1_2CDC(void) {
    omUPauseFlag = 0;
    omSysPauseEnable((u8) lbl_1_bss_A8);
    omObjManPause(0);
    Hu3DCameraLayerHookReset((s16) lbl_1_bss_148, 15);
    if (lbl_1_bss_1E4->pauseOverlayMode != 0) {
        fn_1_7684();
    } else {
        fn_1_707C();
    }
    fn_1_6F38();
    if ((s32) lbl_1_bss_AC == 0) {
        HuPrcAllPause(0);
        Hu3DPauseSet(0);
        HuSprPauseSet(0);
        HuAudFXPauseAll(0);
        HuAudSStreamPauseAll(0);
    }
}

/* Builds the selection screen's sprite groups and quiz-mode-specific indicators. */
void fn_1_2DA0(void) {
    u8 unusedLocalA;
    u8 unusedLocalB;
    u8 unusedLocalC;
    u8 unusedLocalD;
    u8 unusedLocalE;
    u8 unusedLocalF;
    u8 unusedLocalG;
    u8 unusedLocalH;
    s32 quizMode;
    s32 unusedLocalI;
    s32 playerIndex;
    s32 characterIndex;
    u8 unusedLocalJ;

    lbl_1_bss_1E4 = (MicquizWork *)MgModeWork;
lbl_1_bss_2D8[0] = HuSprGrpCreate(8);
    HuSprGrpPosSet(lbl_1_bss_2D8[0], 493.0f, 386.0f);
    playerIndex = 0;
    while (playerIndex < 4) {
        if (fn_1_9148(playerIndex) == 0) {
            lbl_1_bss_15C[playerIndex] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_mgconst, 0), HU_MEMNUM_OVL, HEAP_MODEL));
            HuSprGrpMemberSet(lbl_1_bss_2D8[0], (s16) (playerIndex + 4),
                              HuSprCreate(lbl_1_bss_15C[playerIndex], 48, 0));
            HuSprPosSet(lbl_1_bss_2D8[0], (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_2D8[0], (s16) (playerIndex + 4), 4);
        } else {
            characterIndex = GwPlayerConf[playerIndex].charNo;
            lbl_1_bss_15C[playerIndex] = HuSprAnimRead(
                HuDataSelHeapReadNum(lbl_1_data_1C[characterIndex], HU_MEMNUM_OVL, HEAP_MODEL));
            HuSprGrpMemberSet(lbl_1_bss_2D8[0], (s16) (playerIndex + 4),
                              HuSprCreate(lbl_1_bss_15C[playerIndex], 48, 0));
            HuSprPosSet(lbl_1_bss_2D8[0], (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_2D8[0], (s16) (playerIndex + 4), 4);
        }
        playerIndex += 1;
    }
    lbl_1_bss_158 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 28), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_2D8[0], 0, HuSprCreate(lbl_1_bss_158, 49, 0));
    HuSprPosSet(lbl_1_bss_2D8[0], 0, 0.0f, 0.0f);
    lbl_1_bss_154 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 29), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_2D8[0], 1, HuSprCreate(lbl_1_bss_154, 49, 0));
    HuSprPosSet(lbl_1_bss_2D8[0], 1, 8.0f, 12.0f);
    lbl_1_bss_150 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 30), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_2D8[0], 2, HuSprCreate(lbl_1_bss_150, 49, 0));
    HuSprPosSet(lbl_1_bss_2D8[0], 2, 13.0f, 9.0f);
    lbl_1_bss_14C =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 32), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_2D8[0], 3, HuSprCreate(lbl_1_bss_14C, 49, 0));
    HuSprPosSet(lbl_1_bss_2D8[0], 3, -23.0f, -25.0f);
    lbl_1_bss_19C =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 36), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_176 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_176, 0, HuSprCreate(lbl_1_bss_19C, 50, 0));
    HuSprPosSet(lbl_1_bss_176, 0, 288.0f, 68.0f);
    HuSprAttrSet(lbl_1_bss_176, 0, 4);
    lbl_1_bss_198 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 37), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_174 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_174, 0, HuSprCreate(lbl_1_bss_198, 50, 0));
    HuSprPosSet(lbl_1_bss_174, 0, 288.0f, 68.0f);
    HuSprBankSet(lbl_1_bss_174, 0, 1);
    lbl_1_bss_194 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 38), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_172 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_172, 0, HuSprCreate(lbl_1_bss_194, 50, 0));
    HuSprPosSet(lbl_1_bss_172, 0, 288.0f, 68.0f);
    HuSprTPLvlSet(lbl_1_bss_172, 0, 0.0f);
    lbl_1_bss_16C[0] = HuSprGrpCreate(2);
    lbl_1_bss_18C[0] =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 86), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_16C[0], 0, HuSprCreate(lbl_1_bss_18C[0], 50, 0));
    HuSprPosSet(lbl_1_bss_16C[0], 0, -16.0f, 0.0f);
    HuSprBankSet(lbl_1_bss_16C[0], 0, 0);
    lbl_1_bss_18C[1] =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 86), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_16C[0], 1, HuSprCreate(lbl_1_bss_18C[1], 50, 0));
    HuSprPosSet(lbl_1_bss_16C[0], 1, 16.0f, 0.0f);
    HuSprBankSet(lbl_1_bss_16C[0], 1, 0);
    HuSprGrpAttrSet(lbl_1_bss_16C[0], 4);
    HuSprGrpPosSet(lbl_1_bss_16C[0], 271.0f, 68.0f);
    quizMode = fn_1_9330();
    if (quizMode != 8) {
        lbl_1_bss_1E0 = HuSprAnimRead(
            HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 21), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_18A = HuSprGrpCreate(1);
        HuSprGrpMemberSet(lbl_1_bss_18A, 0, HuSprCreate(lbl_1_bss_1E0, 50, 0));
        HuSprPosSet(lbl_1_bss_18A, 0, 401.0f, 62.0f);
        HuSprBankSet(lbl_1_bss_18A, 0, (s16) (quizMode + 1));
        if (quizMode == 3) {
            lbl_1_bss_1D8[0] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 54), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_1D8[1] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 55), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_1B8 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 56), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_1B4 = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_bss_1B4, 0, HuSprCreate(lbl_1_bss_1B8, 49, 0));
            HuSprPosSet(lbl_1_bss_1B4, 0, 401.0f, 62.0f);
            HuSprAttrSet(lbl_1_bss_1B4, 0, 4);
        } else if (quizMode == 2) {
            lbl_1_bss_1D4 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 57), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_data_20A = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_20A, 0, HuSprCreate(lbl_1_bss_1D4, 51, 0));
            HuSprPosSet(lbl_1_data_20A, 0, 401.0f, 62.0f);
            lbl_1_bss_4C[0] = HuPrcChildCreate(fn_1_5600, 100U, 6144U, 0, HuPrcCurrentGet());
            lbl_1_bss_4C[0]->property = (void *) lbl_1_data_20A;
        } else if (quizMode == 4) {
            lbl_1_bss_1D0 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 58), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_data_20C = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_20C, 0, HuSprCreate(lbl_1_bss_1D0, 51, 0));
            HuSprPosSet(lbl_1_data_20C, 0, 401.0f, 62.0f);
            lbl_1_bss_4C[0] = HuPrcChildCreate(fn_1_5600, 100U, 6144U, 0, HuPrcCurrentGet());
            lbl_1_bss_4C[0]->property = (void *) lbl_1_data_20C;
            lbl_1_bss_1CC = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 59), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_data_20E = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_20E, 0, HuSprCreate(lbl_1_bss_1CC, 51, 0));
            HuSprPosSet(lbl_1_data_20E, 0, 288.0f, 68.0f);
            lbl_1_bss_1C8 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 60), HU_MEMNUM_OVL, HEAP_MODEL));
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                lbl_1_data_210[playerIndex] = HuSprGrpCreate(1);
                HuSprGrpMemberSet(lbl_1_data_210[playerIndex], 0,
                                  HuSprCreate(lbl_1_bss_1C8, 52, 0));
                HuSprPosSet(lbl_1_data_210[playerIndex], 0, lbl_1_data_21C[playerIndex].positionX,
                            lbl_1_data_21C[playerIndex].positionY);
                HuSprAttrSet(lbl_1_data_210[playerIndex], 0, 1);
                lbl_1_bss_4C[playerIndex + 2] =
                    HuPrcChildCreate(fn_1_5294, 100U, 6144U, 0, HuPrcCurrentGet());
                lbl_1_bss_4C[playerIndex + 2]->property = (void *) lbl_1_data_210[playerIndex];
            }
            lbl_1_bss_1C4 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 61), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_data_218 = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_218, 0, HuSprCreate(lbl_1_bss_1C4, 53, 0));
            HuSprPosSet(lbl_1_data_218, 0, 248.0f, 52.0f);
            /* This additional pulse process is created without assigning its sprite-group
             * property. */
            HuPrcChildCreate(fn_1_5600, 100U, 6144U, 0, HuPrcCurrentGet());
            lbl_1_bss_4C[0] = HuPrcChildCreate(fn_1_507C, 100U, 6144U, 0, HuPrcCurrentGet());
            lbl_1_bss_4C[0]->property = (void *) lbl_1_data_218;
            lbl_1_bss_1C0 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 62), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_data_21A = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_21A, 0, HuSprCreate(lbl_1_bss_1C0, 54, 0));
            HuSprPosSet(lbl_1_data_21A, 0, 328.0f, 84.0f);
            lbl_1_bss_4C[1] = HuPrcChildCreate(fn_1_507C, 100U, 6144U, 0, HuPrcCurrentGet());
            lbl_1_bss_4C[1]->property = (void *) lbl_1_data_21A;
        } else if (quizMode == 5 || quizMode == 7) {
            lbl_1_bss_1D0 = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 58), HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_data_20C = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_20C, 0, HuSprCreate(lbl_1_bss_1D0, 51, 0));
            HuSprPosSet(lbl_1_data_20C, 0, 401.0f, 62.0f);
            lbl_1_bss_4C[0] = HuPrcChildCreate(fn_1_5600, 100U, 6144U, 0, HuPrcCurrentGet());
            lbl_1_bss_4C[0]->property = (void *) lbl_1_data_20C;
        } else if (quizMode == 0 || quizMode == 1) {
            lbl_1_bss_1BC = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 53), HU_MEMNUM_OVL, HEAP_MODEL));
        } else if (quizMode == 6) {
            lbl_1_bss_1BC = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 63), HU_MEMNUM_OVL, HEAP_MODEL));
        }
    }
    lbl_1_bss_1B0 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 33), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_180[0] = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_180[0], 0, HuSprCreate(lbl_1_bss_1B0, 50, 0));
    HuSprPosSet(lbl_1_bss_180[0], 0, 506.0f, 65.0f);
    HuSprBankSet(lbl_1_bss_180[0], 0, 0);
    lbl_1_bss_1AC =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 45), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_17E = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_17E, 0, HuSprCreate(lbl_1_bss_1AC, 50, 0));
    HuSprPosSet(lbl_1_bss_17E, 0, 504.0f, 99.0f);
    HuSprBankSet(lbl_1_bss_17E, 0, 0);
    lbl_1_bss_1A8 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 46), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_17C = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_17C, 0, HuSprCreate(lbl_1_bss_1A8, 50, 0));
    HuSprPosSet(lbl_1_bss_17C, 0, 504.0f, 99.0f);
    HuSprBankSet(lbl_1_bss_17C, 0, 0);
    HuSprGrpAttrSet(lbl_1_bss_17C, 4);
    lbl_1_bss_1A4 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 44), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_17A = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_17A, 0, HuSprCreate(lbl_1_bss_1A4, 50, 0));
    HuSprPosSet(lbl_1_bss_17A, 0, 102.0f, 67.0f);
    HuSprBankSet(lbl_1_bss_17A, 0, (s16) lbl_1_bss_1E4->selectedQuizMode);
    lbl_1_bss_A4 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 42), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_9C = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_9C, 0, HuSprCreate(lbl_1_bss_A4, 50, 0));
    HuSprPosSet(lbl_1_bss_9C, 0, 292.0f, 208.0f);
    HuSprAttrSet(lbl_1_bss_9C, 0, 4);
    lbl_1_bss_A0 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 43), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_9A = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_9A, 0, HuSprCreate(lbl_1_bss_A0, 50, 0));
    HuSprPosSet(lbl_1_bss_9A, 0, 292.0f, 208.0f);
    HuSprAttrSet(lbl_1_bss_9A, 0, 4);
    lbl_1_bss_1A0 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 103), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_178 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_178, 0, HuSprCreate(lbl_1_bss_1A0, 1500, 0));
    lbl_1_bss_1F0 = 285.0f;
    lbl_1_bss_1EC = -152.0f;
    HuSprPosSet(lbl_1_bss_178, 0, lbl_1_bss_1F0, lbl_1_bss_1EC);
    HuSprAttrSet(lbl_1_bss_178, 0, 4);
    lbl_1_bss_1E8 = 0;
    fn_1_13F8();
    fn_1_638C();
    fn_1_E8A4(0);
}

/* Creates the player/input group; portrait-slot creation stops after the first human player. */
void fn_1_48A4(void) {
    s32 player;
    s32 characterIndex;

    lbl_1_bss_1E4 = (MicquizWork *)MgModeWork;
    lbl_1_bss_2D8[0] = HuSprGrpCreate(8);
    HuSprGrpPosSet(lbl_1_bss_2D8[0], 493.0f, 386.0f);

    for (player = 0; player < 4; player++) {
        if (fn_1_9148(player) == 0) {
            lbl_1_bss_15C[player] =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_mgconst, 0), HU_MEMNUM_OVL, HEAP_MODEL));
            HuSprGrpMemberSet(lbl_1_bss_2D8[0], player + 4,
                              HuSprCreate(lbl_1_bss_15C[player], 48, 0));
            HuSprPosSet(lbl_1_bss_2D8[0], player + 4, -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_2D8[0], player + 4, 4);
        } else {
            characterIndex = GwPlayerConf[player].charNo;
            lbl_1_bss_15C[player] = HuSprAnimRead(
        HuDataSelHeapReadNum(lbl_1_data_1C[characterIndex], HU_MEMNUM_OVL, HEAP_MODEL));
            HuSprGrpMemberSet(lbl_1_bss_2D8[0], player + 4,
                              HuSprCreate(lbl_1_bss_15C[player], 48, 0));
            HuSprPosSet(lbl_1_bss_2D8[0], player + 4, -23.0f, -25.0f);
            break;
        }
    }

    lbl_1_bss_158 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 28), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_2D8[0], 0, HuSprCreate(lbl_1_bss_158, 49, 1));
    HuSprPosSet(lbl_1_bss_2D8[0], 0, 0.0f, 0.0f);
    lbl_1_bss_154 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 29), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_2D8[0], 1, HuSprCreate(lbl_1_bss_154, 49, 1));
    HuSprPosSet(lbl_1_bss_2D8[0], 1, 8.0f, 12.0f);
    lbl_1_bss_150 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_micquiz, 30), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_2D8[0], 2, HuSprCreate(lbl_1_bss_150, 49, 0));
    HuSprPosSet(lbl_1_bss_2D8[0], 2, 13.0f, 9.0f);

    fn_1_13F8();
    fn_1_638C();
}

/* Animates the selected quiz mode's decorative sprite for each scheduled frame. */
void fn_1_507C(void) {
    HUPROCESS *process;
    s16 sprite;
    f32 angle;
    f32 unusedRandomAngleDegrees;
    f32 scale;
    f32 transparency;

    process = HuPrcCurrentGet();
    sprite = (s16)process->property;
    angle = (f32)(u32)frandmod(360);
    unusedRandomAngleDegrees = (f32)(u32)frandmod(360);
    HuSprTPLvlSet(sprite, 0,
        (f32)((0.5) +
        ((0.5) * sin(((3.141592653589793) * (f64)angle) /
        (180.0)))));
    HuSprScaleSet(sprite, 0,
        (f32)((1.0) +
        ((0.5) * sin(((3.141592653589793) * (f64)angle) /
        (180.0)))),
        (1.0f));
    for (;;) {
        HuPrcVSleep();
        scale = (f32)((1.0) +
            ((0.5) * sin(((3.141592653589793) * (f64)angle) /
            (180.0))));
        transparency = (f32)((0.5) +
            ((0.5) * sin(((3.141592653589793) * (f64)angle) /
            (180.0))));
        HuSprTPLvlSet(sprite, 0, transparency);
        HuSprScaleSet(sprite, 0, scale, (1.0f));
        angle += (0.75f);
    }
}

/* Animates a quiz-choice marker after a random delay, then repeats its pulse. */
void fn_1_5294(void) {
    f32 pulseScale;
    f32 angleDegrees;
    s32 initialDelayTicks;
    s32 repeatDelayTicks;
    HUPROCESS *currentProcess;
    s16 groupId;

    currentProcess = HuPrcCurrentGet();
    groupId = (s16)currentProcess->property;
    HuSprAttrSet(groupId, 0, 4);
    HuSprScaleSet(groupId, 0, (0.0f), (0.0f));
    initialDelayTicks = frandmod(60) + 60;
    repeatDelayTicks = frandmod(120) + 60;
    HuPrcSleep(initialDelayTicks);
    angleDegrees = (0.0f);
pulse_again:
    HuSprAttrReset(groupId, 0, 4);
    HuSprAttrReset(groupId, 0, 1);
    do {
        HuPrcVSleep();
        pulseScale = (f32)sin(((3.141592653589793) * (f64)angleDegrees) /
                           (180.0));
        HuSprScaleSet(groupId, 0, pulseScale, pulseScale);
        angleDegrees += (4.285714f);
    } while (!(angleDegrees >= (90.0f)));
    angleDegrees = (0.0f);
    HuSprAttrSet(groupId, 0, 4);
    HuSprAttrSet(groupId, 0, 1);
    HuPrcSleep(repeatDelayTicks);
    goto pulse_again;
}
