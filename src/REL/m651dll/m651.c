/* Black Hole runs the duel sequence, audio, and winner or draw presentation. */
#include "REL/m651dll.h"
#include "game/audio.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "math.h"

#define M651_OPENING_START_EFFECT_ID 2052
#define M651_OPENING_STREAM_ID 87

/* Entry 0 counts cleared player selections; two cleared slots select the draw display. */
s16 lbl_1_bss_4[6];
/* Object manager for this module's player and scene callbacks. */
OMOBJMAN *lbl_1_bss_0;
/* Character selection by player slot; the sequence reads the first four entries for results. */
s16 lbl_1_data_0[6] = { -1, -1, -1, -1, 260, 0 };
/* Opening stream handle, faded when the result phase begins. */
s32 lbl_1_data_C = -1;
/* Opening sound-effect handle, faded at sequence frame 180. */
s32 lbl_1_data_10 = -1;
/* Callback table for the opening, duel, result, and closing sequence phases. */
MGSEQ_PARAM lbl_1_data_14 = {
    300, 0,
    fn_1_1B0, fn_1_1D0, fn_1_240, fn_1_244, fn_1_268,
    fn_1_3D4, fn_1_400, fn_1_404, fn_1_408
};

/* Returns the object manager created by the module initializer. */
OMOBJMAN *fn_1_A0(void)
{
    return lbl_1_bss_0;
}

/* Called when a player finishes; clears that player's stored character selection and increments the
 * cleared-selection count. */
void fn_1_B0(s16 playerNo)
{
    if (lbl_1_data_0[playerNo] != -1) {
        lbl_1_data_0[playerNo] = -1;
        lbl_1_bss_4[0]++;
    }
}

/* Called while player objects are created; stores and reports the configured character for a
 * selected player slot. */
void fn_1_10C(s16 playerNo)
{
    OSReport("player no : ( %d ) char no : ( %d )\n", playerNo, GwPlayerConf[playerNo].charNo);
    lbl_1_data_0[playerNo] = GwPlayerConf[playerNo].charNo;
}

/* Called by the scene update after its result animation; advances to the next sequence mode. */
void fn_1_190(void)
{
    MgSeqModeNext();
}

/* Immediate-transition callback in lbl_1_data_14; advances to the next sequence mode. */
void fn_1_1B0(s16 sequenceMode, s16 sequenceFrame)
{
    MgSeqModeNext();
}

/* Opening-phase callback in lbl_1_data_14; starts audio at frame 0 and fades the opening sound at
 * frame 180. */
void fn_1_1D0(s16 sequenceMode, s16 sequenceFrame)
{
    if (MgSeqFrameNoGet() == 0) {
        lbl_1_data_10 = HuAudFXPlay(M651_OPENING_START_EFFECT_ID);
        lbl_1_data_C = HuAudSStreamPlay(M651_OPENING_STREAM_ID);
    }
    if (MgSeqFrameNoGet() == 180) {
        HuAudFXFadeOut(lbl_1_data_10, 1000);
    }
}

/* Opening-phase callback in lbl_1_data_14; this phase has no additional per-frame update. */
void fn_1_240(s16 sequenceMode, s16 sequenceFrame)
{
}

/* Duel-phase callback in lbl_1_data_14; updates the scene and resolves any newly reached finish. */
void fn_1_244(s16 sequenceMode, s16 sequenceFrame)
{
    fn_1_4A0();
    fn_1_1E60();
}

/* Result-phase callback in lbl_1_data_14; at frame 0 hides meteors, reports the result, and awards
 * 10 coins to each still-selected player outside practice mode. Since a finisher's selection was
 * cleared, this can award the other player. At frame 90 it assigns post-result paths and advances
 * sequence mode without closing the scene. */
void fn_1_268(s16 sequenceMode, s16 sequenceFrame)
{
    int playerIndex;

    if (MgSeqFrameNoGet() == 0) {
        if (lbl_1_data_C != -1) {
            HuAudSStreamFadeOut(lbl_1_data_C, 100);
            lbl_1_data_C = -1;
        }
        fn_1_62C();
        if (lbl_1_bss_4[0] == 2) {
            MgSeqDrawSet();
        } else {
            MgSeqWinnerSet(lbl_1_data_0[0], lbl_1_data_0[1], lbl_1_data_0[2], lbl_1_data_0[3]);
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (lbl_1_data_0[playerIndex] != -1) {
                    OSReport("player no : ( %d ) char no : ( %d )\n", playerIndex,
                             GwPlayerConf[playerIndex].charNo);
                    if (!_CheckFlag(FLAG_MG_PRACTICE)) {
                        GwPlayer[playerIndex].mgCoinBonus = 10;
                    }
                }
            }
        }
    }
    OSReport("cnt : ( %d )\n", MgSeqFrameNoGet());
    if (MgSeqFrameNoGet() == 90) {
        fn_1_36F8();
        MgSeqModeNext();
    }
}

/* Result-transition callback in lbl_1_data_14; advances when this phase reaches frame 1. */
void fn_1_3D4(s16 sequenceMode, s16 sequenceFrame)
{
    if (MgSeqFrameNoGet() == 1) {
        MgSeqModeNext();
    }
}

/* Closing-phase callback in lbl_1_data_14; this phase has no per-frame update. */
void fn_1_400(s16 sequenceMode, s16 sequenceFrame)
{
}

/* Closing-phase callback in lbl_1_data_14; this phase has no per-frame update. */
void fn_1_404(s16 sequenceMode, s16 sequenceFrame)
{
}

/* Closing-phase callback in lbl_1_data_14; this phase has no per-frame update. */
void fn_1_408(s16 sequenceMode, s16 sequenceFrame)
{
}

/* Called by _prolog; creates the object manager and sequence, clears player slots, and builds the
 * scene. */
void fn_1_40C(void)
{
    lbl_1_bss_0 = omInitObjMan(30, 8192);
    omGameSysInit(lbl_1_bss_0);
    MgSeqCreate(&lbl_1_data_14);
    lbl_1_data_0[0] = -1;
    lbl_1_data_0[1] = -1;
    lbl_1_data_0[2] = -1;
    lbl_1_data_0[3] = -1;
    fn_1_3F30();
}
