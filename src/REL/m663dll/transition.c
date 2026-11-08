/* Finish, pre-winner, winner and close phase updates, and the ending sequence that returns
 * the players, fades the background models and moves the camera. */
#include "REL/m663dll/transition.h"
#include "dolphin/types.h"
#include "REL/m663dll/models.h"
#include "REL/m663dll/motion.h"
#include "REL/m663dll/player.h"
#include "REL/m663dll/camera.h"

#include "math.h"

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

#include "game/hu3d.h"
#include "REL/m663dll/input.h"
#include "REL/m663dll/projection.h"
#include "REL/m663dll/actors.h"
#include "REL/m663dll.h"

s32 fn_1_C05C(void);

/* Finish phase: fades out the BGM over 100 frames and runs the ending sequence with the
 * gameplay, actor, collision and model updates; always returns 0. */
int fn_1_BCC8(void) {
    short result = 0;
    fn_1_5F0C(1, 100);
    fn_1_C05C();
    fn_1_69E8();
    fn_1_3588();
    fn_1_987C();
    fn_1_2A00();
    return result;
}

/* Pre-winner phase: runs the ending sequence with the gameplay, actor, collision and model
 * updates; returns 1 once the ending sequence is complete. */
s16 fn_1_BD14(void) {
    s16 result = 0;
    result = fn_1_C05C();
    fn_1_69E8();
    fn_1_3588();
    fn_1_987C();
    fn_1_2A00();
    return result;
}

/* Winner phase: on the first frame sets each player's sound position and blends it into
 * motion 2 (mode >= 0) or 3; every frame it reads input and runs the ending sequence,
 * gameplay and model updates. Always returns 0. */
s32 fn_1_BD58(void) {
    s16 player;

        switch (lbl_1_bss_424.transitionState) {
    case 1:
        break;
    case 0:
        player = 0;
        while (player < 2) {
            fn_8005DFC0(lbl_1_bss_4F0[player].characterNo, 1);
            fn_1_584C(&lbl_1_bss_4F0[player], 6);
            if (lbl_1_bss_4F0[player].mode >= 0) {
                lbl_1_bss_688[player].shiftState = 1;
                lbl_1_bss_688[player].shiftMotionIndex = 2;
                lbl_1_bss_688[player].shiftStartTime = 0.0f;
                lbl_1_bss_688[player].shiftDuration = 5.0f;
                lbl_1_bss_688[player].shiftAttributes = 0;
                lbl_1_bss_688[player].attributesToClear = ((1 << 30) | (1 << 0));
            } else {
                lbl_1_bss_688[player].shiftState = 1;
                lbl_1_bss_688[player].shiftMotionIndex = 3;
                lbl_1_bss_688[player].shiftStartTime = 0.0f;
                lbl_1_bss_688[player].shiftDuration = 5.0f;
                lbl_1_bss_688[player].shiftAttributes = 0;
                lbl_1_bss_688[player].attributesToClear = ((1 << 30) | (1 << 0));
            }
            player += 1;
        }
        lbl_1_bss_424.transitionState += 1;
        break;
    }
    fn_1_754();
    fn_1_C05C();
    fn_1_69E8();
    fn_1_2A00();
    return 0;
}

/* static */

s32 fn_1_BFC0(void) {
    fn_1_C05C();
    fn_1_2A00();
    return 0;
}

/* Close phase: removes the timer when both players' modes are nonnegative, releases the
 * draw-hook model (79) and stops all audio. */
s32 fn_1_BFE8(void) {
    if (lbl_1_bss_4F0[0].mode >= 0 && lbl_1_bss_4F0[1].mode >= 0) {
        fn_1_3ABC(lbl_1_bss_424.timer);
    }
    fn_800262D0(lbl_1_bss_750[0x4F].modelId);
    fn_1_67E4();
    return 0;
}

extern f32 lbl_802C028C;

s32 fn_1_C05C(void)
{
    s32 result = 0;
    s32 moving = 0;
    s16 index;

    switch (lbl_1_bss_424.transitionMode) {
    case 0:
        if (lbl_1_bss_4F0[0].score == 6 && lbl_1_bss_4F0[1].score == 6) {
            lbl_1_bss_4F0[0].score = lbl_1_bss_4F0[1].score = 7;
            fn_800269E8(lbl_1_bss_750[0x17].modelId, 1);
            lbl_1_bss_424.transitionTimer = 0;
            lbl_1_bss_424.transitionMode++;
        }
        break;
    case 1:
        if (lbl_1_bss_4F0[0].score == 10 && lbl_1_bss_4F0[1].score == 10) {
            if (lbl_1_bss_424.transitionTimer++ > 60) {
                for (index = 0; index <= 3; index++) {
                    lbl_1_bss_0[index].state = 1;
                }
                lbl_1_bss_424.transitionTimer = 0;
                lbl_1_bss_424.transitionMode++;
            }
        }
        break;
    case 2: {
        f32 transitionSpeed = 60.0f;
        s16 movingCount;

        moving = fn_1_38D4(&lbl_80266A4C.x, transitionSpeed,
                           0.0f, 0.0f);
        moving += fn_1_38D4(&lbl_80266A4C.y, transitionSpeed,
                            166.0f, 228.0f);
        moving += fn_1_38D4(&lbl_80266A4C.z, transitionSpeed,
                            25.0f, 25.0f);
        moving += fn_1_38D4(&lbl_80266A40.x, transitionSpeed,
                            -31.7f, -19.7f);
        moving += fn_1_38D4(&lbl_80266A40.y, transitionSpeed,
                            0.0f, 0.0f);
        moving += fn_1_38D4(&lbl_80266A40.z, transitionSpeed,
                            0.0f, 0.0f);
        moving += fn_1_38D4(&lbl_802C028C, transitionSpeed,
                            2483.3f, 1181.0f);

        index = 0;
        movingCount = 0;
        for (; index < 6; index++) {
            if (lbl_1_bss_0[index].mode >= 2) {
                movingCount++;
            }
        }
        if (movingCount >= 4) {
            if (lbl_1_bss_424.transitionTimer == 0) {
                lbl_1_bss_0[4].state = 1;
                lbl_1_bss_0[5].state = 1;
                fn_80026F9C((s16)lbl_1_bss_418.players[0]->actor->mdlId);
                fn_80026F9C((s16)lbl_1_bss_418.players[1]->actor->mdlId);
                fn_80026A44(lbl_1_bss_750[0xC].modelId, 1);
            }
            if ((lbl_1_bss_1378.transparency -= 0.05f) < 0.0f) {
                lbl_1_bss_1378.transparency = 0.0f;
            } else {
                moving++;
            }
        } else {
            moving++;
        }
        if ((s16)moving == 0) {
            lbl_1_bss_424.transitionTimer = 0;
            lbl_1_bss_424.transitionMode++;
        }
        break;
    }
    case 3: {
        if (lbl_1_bss_424.transitionTimer++ > 5) {
            lbl_1_bss_424.transitionMode++;
        }
        break;
    }
    case 4:
        result = 1;
        break;
    }
    return result;
}
