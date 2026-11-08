/* Sound-effect slot controls (pan, pitch, volume, stop) and the main play phase: its
 * start, per-frame update order and end check. */
#include <math.h>
#include "game/audio.h"
#include "game/charman.h"
#include "game/hu3d.h"
#include "game/gamemes.h"
#include "REL/m661dll/system_state.h"
#include "REL/m661dll/player_state.h"
#include "REL/m661dll/score_display.h"
#include "REL/m661dll/particle.h"

extern s32 lbl_1_bss_210;

s16 lbl_1_bss_31C8[4];

extern f32 lbl_1_bss_2FEC[];

void fn_1_3038(MGTIMER *timer);

void fn_1_58DC(void);

void fn_1_695C(void);

void fn_1_24E8(void);

s32 fn_1_5954(void);

extern void fn_1_71C(void);

extern void fn_1_20D74(void);

extern void fn_1_59F0(void);

extern void fn_1_5A7C(void);

extern void fn_1_67DC(void);

#include "dolphin/types.h"

extern s16 lbl_1_data_F6[];

/* Sets the pan of the sound effect in slot index while the slot is active and the
 * effect's status is 2. */
void fn_1_5444(s16 index, s16 panning) {
    if (lbl_1_bss_196C.soundActive[index] == 1) {
        if (HuAudFXStatusGet(lbl_1_bss_196C.soundHandles[index]) == 2) {
            HuAudFXPanning(lbl_1_bss_196C.soundHandles[index], panning);
        }
    }
}

/* Sets the pitch of the sound effect in slot index while the slot is active and the
 * effect's status is 2. */
void fn_1_54D4(s16 index, s16 pitch) {
    if (lbl_1_bss_196C.soundActive[index] == 1) {
        if (HuAudFXStatusGet(lbl_1_bss_196C.soundHandles[index]) == 2) {
            HuAudFXPitchSet(lbl_1_bss_196C.soundHandles[index], pitch);
        }
    }
}

/* Sets the volume of the sound effect in slot index while the slot is active and the
 * effect's status is 2. */
void fn_1_5564(s16 index, s16 volume) {
    if (lbl_1_bss_196C.soundActive[index] == 1) {
        if (HuAudFXStatusGet(lbl_1_bss_196C.soundHandles[index]) == 2) {
            HuAudFXVolSet(lbl_1_bss_196C.soundHandles[index], volume);
        }
    }
}

/* Stops the sound effect in slot index if the slot is active and the effect's status
 * is 2 or 3, and marks the slot idle. */
void fn_1_55F4(s16 index) {
    if (lbl_1_bss_196C.soundActive[index] == 1 && (HuAudFXStatusGet(lbl_1_bss_196C.soundHandles[index]) == 2 || HuAudFXStatusGet(lbl_1_bss_196C.soundHandles[index]) == 3)) {
        lbl_1_bss_196C.soundActive[index] = -1;
        HuAudFXStop(lbl_1_bss_196C.soundHandles[index]);
    }
}

/* Stops the sound effect in slot index if the slot is active and the effect's status
 * is 2 or 3, and marks the slot idle. */
void fn_1_56BC(s16 index) {
    if (lbl_1_bss_196C.soundActive[index] == 1 && (HuAudFXStatusGet(lbl_1_bss_196C.soundHandles[index]) == 2 || HuAudFXStatusGet(lbl_1_bss_196C.soundHandles[index]) == 3)) {
        lbl_1_bss_196C.soundActive[index] = -1;
        HuAudFXStop(lbl_1_bss_196C.soundHandles[index]);
    }
}

void fn_1_5784(void) {
    HuAudAllStop();
}

s32 fn_1_57A4(void)
{
    s32 result = 0;
    s16 playerIndex;

    if (lbl_1_bss_31C8[0] == 0) {
        fn_1_3038(lbl_1_bss_196C.action);
        for (playerIndex = 0; playerIndex < 2; playerIndex++) {
            lbl_1_bss_1C04[playerIndex].active = 0;
            MgScoreBoxTPLvlSet(lbl_1_bss_0[playerIndex].characterBox,
                               0.8f);
            MgScoreDispOn(lbl_1_bss_0[playerIndex].score);
        }
        lbl_1_bss_2FEC[0x48 / sizeof(f32)] = 25300.0f;
        lbl_1_bss_31C8[0] = 1;
    }
    if (lbl_1_bss_31C8[0] != 0) {
        lbl_1_bss_196C.frameCounter++;
    }
    fn_1_58DC();
    fn_1_2B58();
    fn_1_695C();
    fn_1_24E8();
    result = fn_1_5954();
    return (s16)result;
}

void fn_1_58DC(void)
{
    HuVecF values[2] = { { -788.0f, 60.0f, -788.0f }, { 788.0f, 60.0f, -788.0f } };

    fn_1_71C();
    fn_1_20D74();
    fn_1_59F0();
    fn_1_5A7C();
    fn_1_67DC();
    fn_1_400C(&values[0]);
    fn_1_400C(&values[1]);
}
