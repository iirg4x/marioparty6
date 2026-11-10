/* Minigame process startup and sequence callbacks for Clean Team. */
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

void fn_1_F0(void);

/* static */
HUPROCESS *lbl_1_bss_0;

void ObjectSetup(void) {
    lbl_1_bss_0 = fn_80080418();
    fn_8000DAC0(fn_1_F0, 0x64U, 0x6000U, 0, lbl_1_bss_0);
}

void fn_1_110(s16 mode, s16 frameNo);
void fn_1_144(s16 mode, s16 frameNo);
void fn_1_148(s16 mode, s16 frameNo);
void fn_1_14C(s16 mode, s16 frameNo);
void fn_1_150(s16 mode, s16 frameNo);
void fn_1_154(s16 mode, s16 frameNo);
void fn_1_158(s16 mode, s16 frameNo);
void fn_1_15C(s16 mode, s16 frameNo);
void fn_1_160(s16 mode, s16 frameNo);

MGSEQ_PARAM lbl_1_data_0 = { 0x12C, 0, fn_1_110, fn_1_144, fn_1_148, fn_1_14C, fn_1_150, fn_1_154, fn_1_158, fn_1_15C, fn_1_160 };

/* Start the minigame sequence, then yield on each video frame. */
void fn_1_F0(void) {
    fn_80070788(&lbl_1_data_0);
    while (1) {
        HuPrcVSleep();
    }
}

void *fn_1_164(s32 priority, u32 bytes, void (*update)(OMOBJ *));

/* static */
void fn_1_2ABC(OMOBJ *object);

/* static */

void fn_1_110(s16 mode, s16 frameNo) {
    fn_1_164(0x64, 8, fn_1_2ABC);
    fn_80071A9C();
}
