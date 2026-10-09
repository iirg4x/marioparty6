/* Sets computer-player timing and button choices for the four riders. */
#define _MATH_H
#include "dolphin/math.h"
#include "REL/m608dll.h"

char lbl_1_data_4C8[40] = "com(%d) ... level : %d, interval : %d\012\000";

/* Called during setup to load difficulty and choose a button interval for each CPU rider. */
void fn_1_69F4(void)
{
    u32 randomOffset;
    s32 playerIndex;
    u32 randomDivisor;

    playerIndex = 0;
    while (playerIndex < 4) {
        if (GwPlayerConf[playerIndex].type == 1) {
            lbl_1_bss_3E80.difficulty[playerIndex] = GwPlayerConf[playerIndex].comDif;
            randomDivisor = lbl_1_data_56C[(lbl_1_bss_3E80.difficulty[playerIndex] * 2) + 1];
            randomOffset = frand() % randomDivisor;
            lbl_1_bss_3E80.base[playerIndex] =
                (s16) lbl_1_data_56C[lbl_1_bss_3E80.difficulty[playerIndex] * 2];
            lbl_1_bss_3E80.interval[playerIndex] =
                (s16) (300U / (lbl_1_bss_3E80.base[playerIndex] + randomOffset));
            lbl_1_bss_3E80.aiRollFrameCounter[playerIndex] = 0;
            OSReport(lbl_1_data_4C8, playerIndex, lbl_1_bss_3E80.difficulty[playerIndex],
                     lbl_1_bss_3E80.interval[playerIndex]);
        }
        playerIndex += 1;
    }
}

/* Called each CPU turn during the rolling state; returns a face-button press when due. */
/* A 1% check can trigger early, and score modulo four selects A, B, Y, then X. */
u32 fn_1_6B90(s32 playerIndex)
{
    s16 interval;
    s16 framesSincePress;
    unsigned int buttons;
    int trigger;

    interval = lbl_1_bss_3E80.interval[playerIndex];
    framesSincePress = lbl_1_bss_3E80.aiRollFrameCounter[playerIndex];
    buttons = 0;
    trigger = 0;
    framesSincePress++;
    if (interval <= framesSincePress) {
        trigger = 1;
        framesSincePress = 0;
    } else if (frandmod(1000) < 10U) {
        trigger = 1;
        framesSincePress = 0;
    }
    if (trigger != 0) {
        switch (lbl_1_bss_3E80.scores[playerIndex] % 4) {
        case 0: buttons |= PAD_BUTTON_A; break;
        case 1: buttons |= PAD_BUTTON_B; break;
        case 2: buttons |= PAD_BUTTON_Y; break;
        case 3: buttons |= PAD_BUTTON_X; break;
        }
    }
    lbl_1_bss_3E80.aiRollFrameCounter[playerIndex] = framesSincePress;
    return buttons;
}
