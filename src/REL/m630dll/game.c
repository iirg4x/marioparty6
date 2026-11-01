/* Runs the four-character minigame: player movement, three object rows, and the result sequence. */
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "REL/m630/game.h"
#include "game/pad.h"

void fn_1_4010(void);
void fn_1_43E8(int player);
#include "game/charman.h"
#include "game/gamework.h"
#include "game/mg/seqman.h"
#include "math.h"
#include "dolphin/os.h"

extern int lbl_1_bss_18[4];
extern HUPROCESS *lbl_1_bss_834;
extern M630Bss744 lbl_1_bss_744;

extern u32 lbl_1_data_28[9];
#include "game/frand.h"

extern int lbl_1_bss_750;
void fn_1_4F10(void);

void fn_1_52F4(int player);
void fn_1_3464(int player);
int fn_1_4CD4(int player);

extern float lbl_1_bss_168;
extern int lbl_1_bss_16C;
void fn_1_6360(int player);
void fn_1_6D70(int player);
void fn_1_7204(int player);
#include "game/hu3d.h"

/* Audio IDs and data numbers used for Stage Fright's sounds, models, and motions. */
#define M630_BGM_GAMEPLAY 81
#define M630_SFX_OBJECT_IMPACT 1833
#define M630_SFX_ROW_1_MOVE 1834
#define M630_SFX_ROW_2_MOVE 1835
#define M630_SFX_ROW_3_MOVE 1836
#define M630_SFX_ROW_1_LAUNCH 1832
#define M630_SFX_ROW_2_LAUNCH 1837
#define M630_SFX_ROW_3_LAUNCH 1838
#define M630_DATA_STAGE_MODEL_FIRST DATANUM(0x590000, 0)
#define M630_DATA_STAGE_MODEL DATANUM(0x590000, 2)
#define M630_DATA_STAGE_SHADOW_MODEL DATANUM(0x590000, 3)
#define M630_DATA_MOVING_ROW_MODEL_FIRST DATANUM(0x590000, 4)
#define M630_DATA_ROW_COMPANION_MODEL DATANUM(0x590000, 7)
#define M630_DATA_SPINNER_MODEL_FIRST DATANUM(0x590000, 8)
#define M630_DATA_MOVING_OBJECT_FIRST DATANUM(0x590000, 11)
#define M630_DATA_SCENE_PROP_FIRST DATANUM(0x590000, 15)
#define M630_DATA_SCENE_MOTION_MODEL DATANUM(0x590000, 20)
#define M630_DATA_SCENE_MOTION DATANUM(0x590000, 21)
#define M630_DATA_DECORATION_MODEL DATANUM(0x590000, 22)
#define M630_DATA_DECORATION_BASE_MOTION DATANUM(0x590000, 23)
#define M630_DATA_DECORATION_ALTERNATE_MOTION DATANUM(0x590000, 25)
#define M630_DATA_OBJECT_COMPANION_MODEL DATANUM(0x590000, 26)

/* Motion resource indices and course position tables used by setup and result movement. */
u32 lbl_1_data_28[9] = {
    DATANUM(0x930000, 0), DATANUM(0x930000, 1),
    DATANUM(0x930000, 2), DATANUM(0x930000, 21),
    DATANUM(0x930000, 39), DATANUM(0x930000, 40),
    DATANUM(0x8e0000, 154), DATANUM(0x8e0000, 155),
    DATANUM(0x930000, 34)
};
HuVecF lbl_1_data_4C[4] = {
    { 300.0f, 0.0f, -2360.0f },
    { 300.0f, 0.0f, -763.58f },
    { 100.0f, 0.0f, -444.296f },
    { -100.0f, 0.0f, -123.149f }
};
float lbl_1_data_7C[4][2] = {
    { 0.0f, 300.0f }, { 300.0f, 100.0f },
    { 0.0f, -100.0f }, { -300.0f, -300.0f }
};

HUPROCESS *lbl_1_bss_834;
M630State lbl_1_bss_828;
int lbl_1_bss_824;
M630Player lbl_1_bss_754[4];
int lbl_1_bss_750;
M630Bss744 lbl_1_bss_744;
M630MovingModel lbl_1_bss_6FC[3];
M630RotatingModel lbl_1_bss_6E4[3];
M630ModelRow lbl_1_bss_1D4[3];
M630Com lbl_1_bss_174[4];
int lbl_1_bss_170;
int lbl_1_bss_16C;
float lbl_1_bss_168;
M630MotionRecord lbl_1_bss_28[10];
int lbl_1_bss_18[4];
int lbl_1_bss_C[3];
int lbl_1_bss_8;

/* The opening callback calls this after camera setup to create players and reset play state. */

void fn_1_4E8(void)
{
    fn_1_1EB0();
    fn_1_25E0();
    lbl_1_bss_828.frameCount = 0;
    lbl_1_bss_828.actionTimer = -1;
    lbl_1_bss_828.resultState = 0;
    lbl_1_bss_170 = 0;
    lbl_1_bss_824 = 0;
    lbl_1_bss_C[0] = lbl_1_bss_C[1] = lbl_1_bss_C[2] = -1;
    lbl_1_bss_8 = -1;
}

/* The main-play callback checks for player contact and advances decorative motions. */

int fn_1_594(void)
{
    lbl_1_bss_824 = fn_1_24F0();
    fn_1_75C8();
    return lbl_1_bss_824;
}

/* The fade-in callback calls this each frame to steer player 0 and advance opponent launch and
 * moving-object state. */

void fn_1_5D0(void)
{
    int i;

    for (i = 0; i < 4; i++) {
        lbl_802BFFFC[lbl_1_bss_754[i].padNo] = 0;
        lbl_802BFFF8[lbl_1_bss_754[i].padNo] = 0;
        lbl_802C0008[lbl_1_bss_754[i].padNo] &= ~PAD_BUTTON_A;
    }
    lbl_1_bss_828.frameCount++;
    if (lbl_1_bss_828.actionTimer == -1) {
        if (lbl_1_bss_1D4[0].pos[0].z < -1900.0f
            && lbl_1_bss_754[0].pos.x > lbl_1_data_4C[2].x) {
            lbl_802BFFFC[lbl_1_bss_754[0].padNo] = -56;
            if (lbl_1_bss_754[0].pos.x <= 20.0f + lbl_1_data_4C[2].x) {
                lbl_1_bss_828.actionTimer = 12;
            }
        }
        if (lbl_1_bss_1D4[1].pos[0].z < -1900.0f
            && lbl_1_bss_754[0].pos.x > lbl_1_data_4C[3].x) {
            lbl_802BFFFC[lbl_1_bss_754[0].padNo] = -56;
            if (lbl_1_bss_754[0].pos.x <= 20.0f + lbl_1_data_4C[3].x) {
                lbl_1_bss_828.actionTimer = 12;
            }
        }
        if (lbl_1_bss_1D4[2].pos[0].z < -1900.0f) {
            lbl_802BFFFC[lbl_1_bss_754[0].padNo] = -56;
            if (lbl_1_bss_754[0].pos.x <= 20.0f + -lbl_1_data_4C[0].x) {
                lbl_1_bss_828.actionTimer = 12;
            }
        }
    } else {
        if (--lbl_1_bss_828.actionTimer >= 0) {
            lbl_802BFFFC[lbl_1_bss_754[0].padNo] = -lbl_1_bss_828.actionTimer;
            lbl_802BFFF8[lbl_1_bss_754[0].padNo] = lbl_1_bss_828.actionTimer - 12;
        }
        if (lbl_1_bss_828.actionTimer == 0) {
            if (lbl_1_bss_754[0].pos.x <= 20.0f + -lbl_1_data_4C[0].x) {
                lbl_1_bss_828.actionTimer = -2;
            } else {
                lbl_1_bss_828.actionTimer = -1;
            }
            lbl_802BFFFC[lbl_1_bss_754[0].padNo] = 0;
            lbl_802BFFF8[lbl_1_bss_754[0].padNo] = 0;
            lbl_1_bss_754[0].rotationY = 0.0f;
        }
    }
    fn_1_78A4(&lbl_1_bss_754[0]);
    fn_1_4010();
    /* Simulate an A-button press for each computer player at staggered times. */
    if (lbl_1_bss_828.frameCount == 30) {
        lbl_802C0008[lbl_1_bss_754[1].padNo] = PAD_BUTTON_A;
    } else if (lbl_1_bss_828.frameCount == 60) {
        lbl_802C0008[lbl_1_bss_754[2].padNo] = PAD_BUTTON_A;
    } else if (lbl_1_bss_828.frameCount == 90) {
        lbl_802C0008[lbl_1_bss_754[3].padNo] = PAD_BUTTON_A;
    }
    for (i = 1; i < 4; i++) {
        fn_1_43E8(i);
    }
    for (i = 0; i < 3; i++) {
        fn_80026790(lbl_1_bss_1D4[i].model[0], &lbl_1_bss_1D4[i].pos[0]);
    }
}

/* The sequence start callback starts the game music and resets the result counters. */

void fn_1_AF0(void)
{
    lbl_1_bss_828.frameCount = lbl_1_bss_828.actionTimer = 0;
    if (lbl_1_bss_8 == -1) {
        lbl_1_bss_8 = HuAudBGMPlay(M630_BGM_GAMEPLAY);
    }
}

/* The finish callback records the result and coin bonuses, slides player 0 on a loss, and fades out
 * moving objects. */

void fn_1_B50(void)
{
    int i;
    int j;
    HuVecF pos;

    for (i = 0; i < 4; i++) {
        lbl_802BFFFC[lbl_1_bss_754[i].padNo] = 0;
        lbl_802BFFF8[lbl_1_bss_754[i].padNo] = 0;
        lbl_802C0008[lbl_1_bss_754[i].padNo] &= ~PAD_BUTTON_A;
    }
    if (lbl_1_bss_828.actionTimer == 0) {
        if (lbl_1_bss_824 == 0) {
            lbl_1_bss_754[0].currentMotion = lbl_1_bss_754[0].motionId[0];
            fn_8005C2C8(lbl_1_bss_754[0].charNo, lbl_1_bss_754[0].motionId[0], 0.0f, 8.0f,
                               ((1 << 30) | (1 << 0)));
        }
        for (i = 1; i < 4; i++) {
            lbl_1_bss_754[i].hitState = 0;
            fn_8005C2C8(lbl_1_bss_754[i].charNo, lbl_1_bss_754[i].motionId[0], 0.0f, 8.0f,
                               ((1 << 30) | (1 << 0)));
            fn_8002D86C(lbl_1_bss_754[i].charNo, 1.0f);
            fn_80026A44(lbl_1_bss_754[i].modelId, 1);
            HuAudFXStop(lbl_1_bss_C[i - 1]);
            lbl_1_bss_C[i - 1] = -1;
        }
        HuPrcKill(lbl_1_bss_834);
        if (lbl_1_bss_824 == 0) {
            fn_80071C18(lbl_1_bss_754[0].charNo, -1, -1, -1);
            GWMgCoinBonusSet(lbl_1_bss_18[0], 10);
        } else {
            fn_80071C18(lbl_1_bss_754[1].charNo, lbl_1_bss_754[2].charNo,
                           lbl_1_bss_754[3].charNo, -1);
            GWMgCoinBonusSet(lbl_1_bss_18[1], 10);
            GWMgCoinBonusSet(lbl_1_bss_18[2], 10);
            GWMgCoinBonusSet(lbl_1_bss_18[3], 10);
        }
        HuAudSStreamFadeOut(lbl_1_bss_8, 100);
    }
    if (lbl_1_bss_824 != 0) {
        if (lbl_1_bss_828.actionTimer == 0) {
            fn_80026EF0(lbl_1_bss_754[0].modelId);
            lbl_1_bss_744.losingPlayerSlideStep = lbl_1_bss_754[0].pos.x > 0.0f ? 15.0f : -15.0f;
        }
        if (lbl_1_bss_828.frameCount % 30 == 0) {
            lbl_1_bss_828.actionTimer++;
        }
        if (lbl_1_bss_828.actionTimer <= 5) {
            lbl_1_bss_828.frameCount++;
            lbl_1_bss_828.frameCount %= 30;
            lbl_1_bss_754[0].pos.y =
                lbl_1_data_4C[0].y +
                (300.0f / lbl_1_bss_828.actionTimer) *
                    sin((3.141592653589793 * (6.0f * lbl_1_bss_828.frameCount)) / 180.0);
            lbl_1_bss_754[0].pos.x += lbl_1_bss_744.losingPlayerSlideStep;
            fn_80026790(lbl_1_bss_754[0].modelId, &lbl_1_bss_754[0].pos);
            lbl_1_bss_754[0].rotationY += lbl_1_bss_744.losingPlayerSlideStep > 0.0f ? -3.0f : 3.0f;
            fn_80026808(lbl_1_bss_754[0].modelId, 0.0f, lbl_1_bss_754[0].rotationY, 0.0f);
        }
    } else {
        lbl_1_bss_828.actionTimer = 1;
    }
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 15; j++) {
            if (lbl_1_bss_1D4[i].itemState[j] != 0) {
                if (lbl_1_bss_1D4[i].itemState[j] == -1 || lbl_1_bss_1D4[i].itemState[j] == 1) {
                    if (lbl_1_bss_1D4[i].transparency[j] == 1.0f) {
                        pos = lbl_1_bss_754[lbl_1_bss_1D4[i].playerIndex[j]].pos;
                        pos.y += 70.0f;
                        pos.z += 65.2f;
                        /* The computed hit position is unused; the companion remains at the row
                         * slot. */
                        fn_80026790(lbl_1_bss_1D4[i].secondaryModel[j],
                                         &lbl_1_bss_1D4[i].pos[j]);
                        fn_80026A44(lbl_1_bss_1D4[i].secondaryModel[j], 1);
                        fn_8002D86C(lbl_1_bss_1D4[i].secondaryModel[j], 1.0f);
                        fn_8002D414(lbl_1_bss_1D4[i].secondaryModel[j], 0.0f);
                        fn_800268B0(lbl_1_bss_1D4[i].secondaryModel[j], 1.8000001f,
                                          1.8000001f, 1.8000001f);
                    }
                    fn_800268B0(lbl_1_bss_1D4[i].model[j],
                                      1.8000001f + (5.0f - 5.0f * lbl_1_bss_1D4[i].transparency[j]),
                                      1.8000001f + (5.0f - 5.0f * lbl_1_bss_1D4[i].transparency[j]),
                                      1.8000001f +
                                          (5.0f - 5.0f * lbl_1_bss_1D4[i].transparency[j]));
                }
                fn_80026CBC(lbl_1_bss_1D4[i].model[j], lbl_1_bss_1D4[i].transparency[j]);
                lbl_1_bss_1D4[i].transparency[j] -= 0.1f;
                if (lbl_1_bss_1D4[i].transparency[j] < 0.0f) {
                    lbl_1_bss_1D4[i].transparency[j] = 0.0f;
                    fn_800269E8(lbl_1_bss_1D4[i].model[j], 1);
                    fn_800268B0(lbl_1_bss_1D4[i].model[j], 1.8000001f, 1.8000001f,
                                      1.8000001f);
                    lbl_1_bss_1D4[i].itemState[j] = 0;
                }
            }
        }
    }
    fn_1_75C8();
}

/* The pre-winner callback moves the human and opponent rows to result positions, then signals when
 * the winner stage can begin. */

int fn_1_1670(void)
{
    int i;
    int finished;
    int result;
    float targetX;

    result = 0;
    finished = 0;
    for (i = 0; i < 4; i++) {
        lbl_802BFFFC[lbl_1_bss_754[i].padNo] = 0;
        lbl_802BFFF8[lbl_1_bss_754[i].padNo] = 0;
        lbl_802C0008[lbl_1_bss_754[i].padNo] &= ~PAD_BUTTON_A;
    }
    if (lbl_1_bss_824 == 0) {
        lbl_1_bss_824 = -1;
        lbl_1_bss_828.frameCount = 0;
    }
    if (lbl_1_bss_824 == -1) {
        if (lbl_1_bss_828.frameCount == 0) {
            if (lbl_1_bss_754[0].pos.x < lbl_1_data_7C[0][1]) {
                lbl_802BFFFC[lbl_1_bss_754[0].padNo] = 56;
                if (lbl_1_bss_754[0].pos.x >= lbl_1_data_7C[0][1] - 20.0f) {
                    lbl_1_bss_828.frameCount = 12;
                }
                lbl_1_bss_828.actionTimer = 0;
            } else {
                lbl_802BFFFC[lbl_1_bss_754[0].padNo] = -56;
                if (lbl_1_bss_754[0].pos.x <= 20.0f + lbl_1_data_7C[0][1]) {
                    lbl_1_bss_828.frameCount = 12;
                }
                lbl_1_bss_828.actionTimer = 1;
            }
        } else {
            if (--lbl_1_bss_828.frameCount >= 0) {
                if (lbl_1_bss_828.actionTimer == 0) {
                    lbl_802BFFFC[lbl_1_bss_754[0].padNo] = lbl_1_bss_828.frameCount;
                } else {
                    lbl_802BFFFC[lbl_1_bss_754[0].padNo] = -lbl_1_bss_828.frameCount;
                }
                lbl_802BFFF8[lbl_1_bss_754[0].padNo] = lbl_1_bss_828.frameCount - 12;
            }
            if (lbl_1_bss_828.frameCount == 0) {
                lbl_1_bss_824 = 2;
                lbl_802BFFFC[lbl_1_bss_754[0].padNo] = 0;
                lbl_802BFFF8[lbl_1_bss_754[0].padNo] = 0;
                lbl_1_bss_754[0].rotationY = 0.0f;
            }
        }
        fn_1_78A4(&lbl_1_bss_754[0]);
    } else if (lbl_1_bss_824 == 1) {
        fn_800269E8(lbl_1_bss_754[0].modelId, 1);
        fn_8005E070(lbl_1_bss_754[0].charNo, lbl_1_data_28[8], 0);
    }
    for (i = 0; i < 3; i++) {
        if (lbl_1_bss_754[i + 1].rotationY > 0.0f) {
            if (lbl_1_bss_754[i + 1].rotationY == 180.0f) {
                fn_8005C2C8(lbl_1_bss_754[i + 1].charNo, lbl_1_bss_754[i + 1].motionId[1],
                                   0.0f, 8.0f, ((1 << 30) | (1 << 0)));
            }
            lbl_1_bss_754[i + 1].rotationY -= 6.0f;
            if (lbl_1_bss_754[i + 1].rotationY <= 0.0f) {
                lbl_1_bss_754[i + 1].rotationY = 0.0f;
                fn_8005C2C8(lbl_1_bss_754[i + 1].charNo, lbl_1_bss_754[i + 1].motionId[0],
                                   0.0f, 8.0f, ((1 << 30) | (1 << 0)));
            }
        }
        targetX = lbl_1_data_7C[i + 1][lbl_1_bss_824 == 1 ? 0 : 1];
        lbl_1_bss_6FC[i].direction = lbl_1_bss_6FC[i].pos.x < targetX ? 0 : 1;
        if (fn_1_3B2C(i + 1, targetX)) {
            finished++;
        }
    }
    fn_1_4010();
    if (lbl_1_bss_824 > 0 && finished == 3) {
        if (++lbl_1_bss_828.resultState >= 15) {
            if (lbl_1_bss_824 == 1) {
                fn_8005C2C8(lbl_1_bss_754[1].charNo, lbl_1_bss_754[1].motionId[4], 0.0f,
                                   8.0f, 0);
                fn_8005C2C8(lbl_1_bss_754[2].charNo, lbl_1_bss_754[2].motionId[4], 0.0f,
                                   8.0f, 0);
                fn_8005C2C8(lbl_1_bss_754[3].charNo, lbl_1_bss_754[3].motionId[4], 0.0f,
                                   8.0f, 0);
            } else {
                fn_8005C2C8(lbl_1_bss_754[0].charNo, lbl_1_bss_754[0].motionId[4], 0.0f,
                                   8.0f, 0);
                fn_8005C2C8(lbl_1_bss_754[1].charNo, lbl_1_bss_754[1].motionId[5], 0.0f,
                                   8.0f, 0);
                fn_8005C2C8(lbl_1_bss_754[2].charNo, lbl_1_bss_754[2].motionId[5], 0.0f,
                                   8.0f, 0);
                fn_8005C2C8(lbl_1_bss_754[3].charNo, lbl_1_bss_754[3].motionId[5], 0.0f,
                                   8.0f, 0);
            }
            result = 1;
            fn_80071B30();
        }
    } else {
        fn_80071B08();
    }
    fn_1_75C8();
    return result;
}

/* The winner callback advances alternate motions on the decorative models. */

void fn_1_1E70(void)
{
    fn_1_75C8();
}

/* The fade-out callback advances alternate motions on the decorative models. */

void fn_1_1E90(void)
{
    fn_1_75C8();
}

/* Player setup calls this to assign the human slot and create all four characters and motions. */

void fn_1_1EB0(void)
{
    int slot;
    int i;
    int j;
    int motion;
    int nextSlot;
    int shuffled[3];
    int players[3];

    for (i = 0; i < 4; i++) {
        if (lbl_80265728[i].grpNo == 0) {
            lbl_1_bss_750 = i;
            break;
        }
    }
    for (i = 0, j = 0; i < 4; i++) {
        if (i != lbl_1_bss_750) {
            players[j++] = i;
        }
    }
    switch (frandmod(6)) {
    case 0:
        shuffled[0] = players[0]; shuffled[1] = players[1]; shuffled[2] = players[2];
        break;
    case 1:
        shuffled[0] = players[0]; shuffled[1] = players[2]; shuffled[2] = players[1];
        break;
    case 2:
        shuffled[0] = players[1]; shuffled[1] = players[0]; shuffled[2] = players[2];
        break;
    case 3:
        shuffled[0] = players[1]; shuffled[1] = players[2]; shuffled[2] = players[0];
        break;
    case 4:
        shuffled[0] = players[2]; shuffled[1] = players[0]; shuffled[2] = players[1];
        break;
    case 5:
        shuffled[0] = players[2]; shuffled[1] = players[1]; shuffled[2] = players[0];
        break;
    }
    for (i = 0, nextSlot = 1, j = 0; i < 4; i++) {
        slot = i == lbl_1_bss_750 ? 0 : nextSlot++;
        lbl_1_bss_754[slot].charNo = lbl_80265728[slot == 0 ? i : shuffled[j]].charNo;
        lbl_1_bss_754[slot].modelId = fn_80057068(lbl_1_bss_754[slot].charNo, 2);
        lbl_1_bss_754[slot].pos = lbl_1_data_4C[slot];
        fn_80026790(lbl_1_bss_754[slot].modelId, &lbl_1_bss_754[slot].pos);
        lbl_1_bss_754[slot].rotationY = slot == 0 ? 0.0f : 180.0f;
        fn_80026808(lbl_1_bss_754[slot].modelId, 0.0f, lbl_1_bss_754[slot].rotationY, 0.0f);
        fn_80026B64(lbl_1_bss_754[slot].modelId, 1);
        fn_800269E8(lbl_1_bss_754[slot].modelId, ((1 << 30) | (1 << 0)));
        if (slot == 0) {
            fn_80026E44(lbl_1_bss_754[slot].modelId);
        }
        fn_80026B8C(lbl_1_bss_754[slot].modelId, 1);
        fn_8005DDEC(3);
        for (motion = 0; motion < 9; motion++) {
            lbl_1_bss_754[slot].motionId[motion] =
                fn_8005B0B8(lbl_1_bss_754[slot].charNo, lbl_1_data_28[motion]);
        }
        lbl_1_bss_754[slot].currentMotion = lbl_1_bss_754[slot].motionId[slot == 0 ? 0 : 6];
        fn_8005BE80(lbl_1_bss_754[slot].charNo, lbl_1_bss_754[slot].currentMotion);
        fn_8005B4BC(lbl_1_bss_754[slot].charNo);
        lbl_1_bss_754[slot].padNo = lbl_80265728[slot == 0 ? i : shuffled[j]].padNo;
        lbl_1_bss_754[slot].hitState = 0;
        lbl_1_bss_754[slot].hitTimerFrames = 0;
        lbl_1_bss_174[slot].difficulty = lbl_80265728[slot == 0 ? i : shuffled[j]].comDif;
        lbl_1_bss_174[slot].type = lbl_80265728[slot == 0 ? i : shuffled[j]].type;
        lbl_1_bss_174[slot].unusedWord = 0;
        lbl_1_bss_174[slot].unusedWord2 = 0;
        lbl_1_bss_174[slot].moveDirection = 1;
        lbl_1_bss_174[slot].moveTimerFrames = 0;
        if (slot != 0) {
            j++;
        }
        lbl_1_bss_18[slot] = i;
    }
    lbl_1_bss_834 = fn_8000DAC0(fn_1_4F10, 100, 8192, 0, fn_8000DD9C());
}

/* The play-contact callback checks all characters against objects and reports human contact. */

int fn_1_24F0(void)
{
    int player;
    int result = 0;

    for (player = 0; player < 4; player++) {
        if (lbl_1_bss_174[player].type != 0) {
            fn_1_52F4(player);
        }
        if (player == 0) {
            if (lbl_1_bss_754[player].hitState >= 0) {
                fn_1_78A4(&lbl_1_bss_754[player]);
            }
        } else {
            fn_1_3464(player);
        }
    }
    for (player = 0; player < 4; player++) {
        if (player > 0) {
            fn_1_43E8(player);
        }
        if (fn_1_4CD4(player) != 0) {
            return 1;
        }
    }
    fn_1_4010();
    return result;
}

/* Opening setup calls this to create the course, object rows, side movers, and decorations. */

void fn_1_25E0(void)
{
    int i;
    s16 model;
    int j;
    s16 motion;
    float motionLength;
    HuVecF pos;
    HuVecF rotatingPos[3] = {
        { 0.0f, -52.4391f, -800.944f },
        { 0.0f, -52.4391f, -471.727f },
        { 0.0f, -52.4391f, -152.798f },
    };
    HuVecF motionPos[10] = {
        { 630.0f, -172.866f, -1900.0f },
        { -816.0f, -172.866f, -500.0f },
        { 699.0f, -172.866f, -937.8f },
        { 715.0f, -172.866f, -826.0f },
        { 710.6762f, -172.866f, -714.227f },
        { 703.0f, -172.866f, -610.0f },
        { 700.0f, -172.866f, -483.0f },
        { 710.0f, -172.866f, -381.0f },
        { 690.0f, -172.866f, -250.0f },
        { 700.0f, -172.866f, -107.0f },
    };
    float motionRot[10] = {
        -72.0f, -13.0f, -95.0f, -137.0f, -145.0f,
        -76.0f, -88.0f, -110.0f, -100.0f, -100.0f,
    };

    for (i = 0; i < 2; i++) {
        model = fn_80025A04(
            fn_80008DD0(M630_DATA_STAGE_MODEL_FIRST + i, 0x10000000, 2));
        fn_80026B64(model, 1);
        fn_80026760(model, 0.0f, 0.0f, 0.0f);
        fn_80026808(model, 0.0f, 0.0f, 0.0f);
        fn_80026FFC(model);
        fn_80026B8C(model, 0);
    }
    model = fn_80025A04(fn_80008DD0(M630_DATA_STAGE_MODEL, 0x10000000, 2));
    fn_80026B64(model, 1);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026808(model, 0.0f, 0.0f, 0.0f);
    model = fn_80025A04(
        fn_80008DD0(M630_DATA_STAGE_SHADOW_MODEL, 0x10000000, 2));
    fn_80026B64(model, 1);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026808(model, 0.0f, 0.0f, 0.0f);
    fn_80026FFC(model);
    for (i = 0; i < 5; i++) {
        model = fn_80025A04(
            fn_80008DD0(M630_DATA_SCENE_PROP_FIRST + i, 0x10000000, 2));
        fn_80026B64(model, 1);
        fn_80026760(model, 0.0f, 0.0f, 0.0f);
        fn_80026808(model, 0.0f, 0.0f, 0.0f);
        if (i == 4) {
            fn_800269E8(model, ((1 << 30) | (1 << 0)));
        }
    }
    for (i = 0; i < 3; i++) {
        lbl_1_bss_6FC[i].model = fn_80025A04(
            fn_80008DD0(M630_DATA_MOVING_ROW_MODEL_FIRST + i, 0x10000000, 2));
        fn_80026B64(lbl_1_bss_6FC[i].model, 1);
        lbl_1_bss_6FC[i].pos = lbl_1_bss_754[i + 1].pos;
        fn_80026790(lbl_1_bss_6FC[i].model, &lbl_1_bss_6FC[i].pos);
        fn_80026808(lbl_1_bss_6FC[i].model, 0.0f, 0.0f, 0.0f);
        lbl_1_bss_6FC[i].direction = 1;
        lbl_1_bss_6FC[i].secondaryModel = fn_80025A04(
            fn_80008DD0(M630_DATA_ROW_COMPANION_MODEL, 0x10000000, 2));
        fn_80026B64(lbl_1_bss_6FC[i].secondaryModel, 1);
        fn_80026760(lbl_1_bss_6FC[i].secondaryModel, lbl_1_bss_6FC[i].pos.x, 0.0f,
                        lbl_1_bss_6FC[i].pos.z);
        fn_80026808(lbl_1_bss_6FC[i].secondaryModel, 0.0f, 0.0f, 0.0f);
        fn_80026B8C(lbl_1_bss_6FC[i].secondaryModel, 2);
        lbl_1_bss_6E4[i].model = fn_80025A04(
            fn_80008DD0(M630_DATA_SPINNER_MODEL_FIRST + i, 0x10000000, 2));
        fn_80026B64(lbl_1_bss_6E4[i].model, 1);
        fn_80026790(lbl_1_bss_6E4[i].model, &rotatingPos[i]);
        lbl_1_bss_6E4[i].rotation = 0.0f;
        fn_80026808(lbl_1_bss_6E4[i].model, lbl_1_bss_6E4[i].rotation, 0.0f, 0.0f);
        for (j = 0; j < 15; j++) {
            lbl_1_bss_1D4[i].model[j] = fn_80025A04(fn_80008DD0(
                frandmod(4) + M630_DATA_MOVING_OBJECT_FIRST, 0x10000000, 2));
            fn_80026B64(lbl_1_bss_1D4[i].model[j], 1);
            lbl_1_bss_1D4[i].pos[j] = lbl_1_bss_754[i + 1].pos;
            lbl_1_bss_1D4[i].pos[j].y += 70.0f;
            lbl_1_bss_1D4[i].pos[j].z -= 65.2f;
            fn_80026790(lbl_1_bss_1D4[i].model[j], &lbl_1_bss_1D4[i].pos[j]);
            fn_80026808(lbl_1_bss_1D4[i].model[j], 0.0f, 0.0f, 0.0f);
            lbl_1_bss_1D4[i].transparency[j] = 0.0f;
            fn_80026CBC(lbl_1_bss_1D4[i].model[j], lbl_1_bss_1D4[i].transparency[j]);
            fn_800269E8(lbl_1_bss_1D4[i].model[j], 1);
            lbl_1_bss_1D4[i].itemState[j] = 0;
            fn_80026E44(lbl_1_bss_1D4[i].model[j]);
            fn_800268B0(lbl_1_bss_1D4[i].model[j], 1.8000001f, 1.8000001f, 1.8000001f);
            lbl_1_bss_1D4[i].secondaryModel[j] = fn_80025A04(
                fn_80008DD0(M630_DATA_OBJECT_COMPANION_MODEL, 0x10000000, 2));
            fn_80026B64(lbl_1_bss_1D4[i].secondaryModel[j], 1);
            pos = lbl_1_bss_754[i + 1].pos;
            pos.y += 70.0f;
            pos.z += 65.2f;
            fn_80026790(lbl_1_bss_1D4[i].secondaryModel[j], &pos);
            fn_80026808(lbl_1_bss_1D4[i].secondaryModel[j], 0.0f, 0.0f, 0.0f);
            fn_800269E8(lbl_1_bss_1D4[i].secondaryModel[j], 1);
            fn_80026B8C(lbl_1_bss_1D4[i].secondaryModel[j], 3);
            fn_8002D86C(lbl_1_bss_1D4[i].secondaryModel[j], 0.0f);
            fn_8002D414(lbl_1_bss_1D4[i].secondaryModel[j], 0.0f);
        }
        lbl_1_bss_1D4[i].phaseState = 0;
        lbl_1_bss_1D4[i].currentIndex = 0;
    }
    for (i = 0; i < 2; i++) {
        model = fn_80025A04(
            fn_80008DD0(M630_DATA_SCENE_MOTION_MODEL, 0x10000000, 2));
        fn_80026B64(model, 1);
        if (i == 0) {
            fn_80026760(model, 759.0895f, -172.928f, -1294.64f);
            fn_80026808(model, 0.0f, -111.728f, 0.0f);
        } else {
            fn_80026760(model, -743.932f, -172.928f, -697.841f);
            fn_80026808(model, 0.0f, -183.92f, 0.0f);
        }
        fn_800269E8(model, ((1 << 30) | (1 << 0)));
        motion = fn_80030458(
            model, fn_80008DD0(M630_DATA_SCENE_MOTION, 0x10000000, 2));
        fn_8002C95C(model, motion);
        fn_80026E44(model);
    }
    for (i = 0; i < 10; i++) {
        lbl_1_bss_28[i].model = fn_80025A04(
            fn_80008DD0(M630_DATA_DECORATION_MODEL, 0x10000000, 2));
        fn_80026B64(lbl_1_bss_28[i].model, 1);
        fn_80026790(lbl_1_bss_28[i].model, &motionPos[i]);
        fn_80026808(lbl_1_bss_28[i].model, 0.0f, motionRot[i], 0.0f);
        fn_800269E8(lbl_1_bss_28[i].model, ((1 << 30) | (1 << 0)));
        lbl_1_bss_28[i].motionA = fn_80030458(
            lbl_1_bss_28[i].model,
            fn_80008DD0(M630_DATA_DECORATION_BASE_MOTION, 0x10000000, 2));
        lbl_1_bss_28[i].motionB = fn_80030458(
            lbl_1_bss_28[i].model,
            fn_80008DD0(M630_DATA_DECORATION_ALTERNATE_MOTION, 0x10000000, 2));
        fn_8002C95C(lbl_1_bss_28[i].model, lbl_1_bss_28[i].motionA);
        motionLength = Hu3DMotionMotionMaxTimeGet(lbl_1_bss_28[i].motionA);
        fn_8002D414(lbl_1_bss_28[i].model, (float) (u32) frandmod((s32) motionLength));
        fn_80026E44(lbl_1_bss_28[i].model);
        lbl_1_bss_28[i].motionState = 0;
    }
}

/* The per-frame play update calls this for each opponent row and its inactive object positions. */

void fn_1_3464(int player)
{
    int row;
    int i;

    row = player - 1;
    if (lbl_1_bss_1D4[row].phaseState > 0 && lbl_1_bss_1D4[row].phaseState < 16) {
        if (lbl_1_bss_C[row] != -1) {
            HuAudFXStop(lbl_1_bss_C[row]);
            lbl_1_bss_C[row] = -1;
        }
        return;
    }
    if (lbl_1_bss_C[row] == -1) {
        lbl_1_bss_C[row] =
            HuAudFXPlay(row == 0 ? M630_SFX_ROW_1_MOVE
                                 : (row == 1 ? M630_SFX_ROW_2_MOVE : M630_SFX_ROW_3_MOVE));
    }
    if (row == 0) {
        if (lbl_1_bss_6FC[row].direction == 0) {
            lbl_1_bss_6FC[row].pos.x += 3.3333333f;
            if (lbl_1_bss_6FC[row].pos.x > 400.0f) {
                lbl_1_bss_6FC[row].pos.x = 400.0f;
                lbl_1_bss_6FC[row].direction = 1;
            }
            lbl_1_bss_6E4[row].rotation -= 3.3333333f;
        } else {
            lbl_1_bss_6FC[row].pos.x -= 3.3333333f;
            if (lbl_1_bss_6FC[row].pos.x < -400.0f) {
                lbl_1_bss_6FC[row].pos.x = -400.0f;
                lbl_1_bss_6FC[row].direction = 0;
            }
            lbl_1_bss_6E4[row].rotation += 3.3333333f;
        }
    } else if (row == 1) {
        if (lbl_1_bss_6FC[row].direction == 0) {
            lbl_1_bss_6FC[row].pos.x += 5.0f;
            if (lbl_1_bss_6FC[row].pos.x > 400.0f) {
                lbl_1_bss_6FC[row].pos.x = 400.0f;
                lbl_1_bss_6FC[row].direction = 1;
            }
            lbl_1_bss_6E4[row].rotation -= 5.0f;
        } else {
            lbl_1_bss_6FC[row].pos.x -= 5.0f;
            if (lbl_1_bss_6FC[row].pos.x < -400.0f) {
                lbl_1_bss_6FC[row].pos.x = -400.0f;
                lbl_1_bss_6FC[row].direction = 0;
            }
            lbl_1_bss_6E4[row].rotation += 5.0f;
        }
    } else if (lbl_1_bss_6FC[row].direction == 0) {
        lbl_1_bss_6FC[row].pos.x += 10.0f;
        if (lbl_1_bss_6FC[row].pos.x > 400.0f) {
            lbl_1_bss_6FC[row].pos.x = 400.0f;
            lbl_1_bss_6FC[row].direction = 1;
        }
        lbl_1_bss_6E4[row].rotation -= 10.0f;
    } else {
        lbl_1_bss_6FC[row].pos.x -= 10.0f;
        if (lbl_1_bss_6FC[row].pos.x < -400.0f) {
            lbl_1_bss_6FC[row].pos.x = -400.0f;
            lbl_1_bss_6FC[row].direction = 0;
        }
        lbl_1_bss_6E4[row].rotation += 10.0f;
    }
    lbl_1_bss_754[player].pos = lbl_1_bss_6FC[row].pos;
    for (i = 0; i < 15; i++) {
        if (lbl_1_bss_1D4[row].itemState[i] == 0) {
            lbl_1_bss_1D4[row].pos[i] = lbl_1_bss_754[row + 1].pos;
            lbl_1_bss_1D4[row].pos[i].y += 70.0f;
            lbl_1_bss_1D4[row].pos[i].z -= 65.2f;
        }
    }
}

/* The result callback calls this each frame to move a side model toward its end mark. */

int fn_1_3B2C(int player, float targetX)
{
    int row;
    int result;

    result = 0;
    row = player - 1;
    if (lbl_1_bss_C[row] == -1) {
        lbl_1_bss_C[row] =
            HuAudFXPlay(row == 0 ? M630_SFX_ROW_1_MOVE
                                 : (row == 1 ? M630_SFX_ROW_2_MOVE : M630_SFX_ROW_3_MOVE));
    }
    if (row == 0) {
        if (lbl_1_bss_6FC[row].direction == 0) {
            lbl_1_bss_6FC[row].pos.x += 3.3333333f;
            if (lbl_1_bss_6FC[row].pos.x > targetX) {
                lbl_1_bss_6FC[row].pos.x = targetX;
                result = 1;
            } else {
                lbl_1_bss_6E4[row].rotation -= 3.3333333f;
            }
        } else {
            lbl_1_bss_6FC[row].pos.x -= 3.3333333f;
            if (lbl_1_bss_6FC[row].pos.x < targetX) {
                lbl_1_bss_6FC[row].pos.x = targetX;
                result = 1;
            } else {
                lbl_1_bss_6E4[row].rotation += 3.3333333f;
            }
        }
    } else if (row == 1) {
        if (lbl_1_bss_6FC[row].direction == 0) {
            lbl_1_bss_6FC[row].pos.x += 5.0f;
            if (lbl_1_bss_6FC[row].pos.x > targetX) {
                lbl_1_bss_6FC[row].pos.x = targetX;
                result = 1;
            } else {
                lbl_1_bss_6E4[row].rotation -= 5.0f;
            }
        } else {
            lbl_1_bss_6FC[row].pos.x -= 5.0f;
            if (lbl_1_bss_6FC[row].pos.x < targetX) {
                lbl_1_bss_6FC[row].pos.x = targetX;
                result = 1;
            } else {
                lbl_1_bss_6E4[row].rotation += 5.0f;
            }
        }
    } else if (lbl_1_bss_6FC[row].direction == 0) {
        lbl_1_bss_6FC[row].pos.x += 10.0f;
        if (lbl_1_bss_6FC[row].pos.x > targetX) {
            lbl_1_bss_6FC[row].pos.x = targetX;
            result = 1;
        } else {
            lbl_1_bss_6E4[row].rotation -= 10.0f;
        }
    } else {
        lbl_1_bss_6FC[row].pos.x -= 10.0f;
        if (lbl_1_bss_6FC[row].pos.x < targetX) {
            lbl_1_bss_6FC[row].pos.x = targetX;
            result = 1;
        } else {
            lbl_1_bss_6E4[row].rotation += 10.0f;
        }
    }
    lbl_1_bss_754[player].pos = lbl_1_bss_6FC[row].pos;
    if (result != 0 && lbl_1_bss_C[row] > -1) {
        HuAudFXStop(lbl_1_bss_C[row]);
        lbl_1_bss_C[row] = -2;
    }
    return result;
}

/* Play and result callbacks call this after movement to clamp and place models and effects. */

void fn_1_4010(void)
{
    int i;
    int j;
    HuVecF pos;
    float bound;

    for (i = 0; i < 4; i++) {
        bound = i == 0 ? 368.0f : 400.0f;
        if (lbl_1_bss_754[i].pos.x > bound) {
            lbl_1_bss_754[i].pos.x = bound;
        } else if (lbl_1_bss_754[i].pos.x < -bound) {
            lbl_1_bss_754[i].pos.x = -bound;
        }
        lbl_1_bss_754[i].pos.z = lbl_1_data_4C[i].z;
        fn_80026790(lbl_1_bss_754[i].modelId, &lbl_1_bss_754[i].pos);
        fn_80026808(lbl_1_bss_754[i].modelId, 0.0f, lbl_1_bss_754[i].rotationY, 0.0f);
    }
    for (i = 0; i < 3; i++) {
        fn_80026790(lbl_1_bss_6FC[i].model, &lbl_1_bss_6FC[i].pos);
        fn_80026808(lbl_1_bss_6E4[i].model, lbl_1_bss_6E4[i].rotation, 0.0f, 0.0f);
        fn_80026760(lbl_1_bss_6FC[i].secondaryModel, lbl_1_bss_6FC[i].pos.x, 0.0f,
                        lbl_1_bss_6FC[i].pos.z);
        for (j = 0; j < 15; j++) {
            fn_80026790(lbl_1_bss_1D4[i].model[j], &lbl_1_bss_1D4[i].pos[j]);
            if (lbl_1_bss_1D4[i].itemState[j] != 0) {
                if (lbl_1_bss_1D4[i].itemState[j] == -2) {
                    pos = lbl_1_bss_1D4[i].pos[j];
                    pos.z += 25.2f;
                } else {
                    pos = lbl_1_bss_754[lbl_1_bss_1D4[i].playerIndex[j]].pos;
                    pos.y += 70.0f;
                    pos.z += 65.2f;
                }
                fn_80026790(lbl_1_bss_1D4[i].secondaryModel[j], &pos);
            }
        }
    }
}

/* The play and contact callbacks call this to animate one opponent and its launched objects. */

void fn_1_43E8(int player)
{
    int row;
    int i;

    row = player - 1;
    if (lbl_1_bss_1D4[row].phaseState == 0 && lbl_1_bss_754[player].hitState >= 0
        && (lbl_802C0008[lbl_1_bss_754[player].padNo] & PAD_BUTTON_A) != 0) {
        fn_8006A030(lbl_1_bss_754[player].padNo, 20, 4, 4);
        lbl_1_bss_1D4[row].transparency[lbl_1_bss_1D4[row].currentIndex] = 1.0f;
        lbl_1_bss_1D4[row].itemState[lbl_1_bss_1D4[row].currentIndex] = 2;
        fn_80026CBC(lbl_1_bss_1D4[row].model[lbl_1_bss_1D4[row].currentIndex],
            lbl_1_bss_1D4[row].transparency[lbl_1_bss_1D4[row].currentIndex]);
        if (++lbl_1_bss_1D4[row].currentIndex == 15) {
            lbl_1_bss_1D4[row].currentIndex = 0;
        }
        lbl_1_bss_1D4[row].phaseState = 1;
        fn_8005C2C8(lbl_1_bss_754[player].charNo, lbl_1_bss_754[player].motionId[7], 0.0f,
                           8.0f, 0);
        fn_8002D86C(lbl_1_bss_754[player].charNo, 2.0f);
    }
    for (i = 0; i < 15; i++) {
        if (lbl_1_bss_1D4[row].itemState[i] == 2 && lbl_1_bss_1D4[row].phaseState == 4) {
            lbl_1_bss_1D4[row].itemState[i] = 1;
            fn_80026A44(lbl_1_bss_1D4[row].model[i], 1);
            HuAudFXPlay(row == 0 ? M630_SFX_ROW_1_LAUNCH
                                 : (row == 1 ? M630_SFX_ROW_2_LAUNCH : M630_SFX_ROW_3_LAUNCH));
        }
        if (lbl_1_bss_1D4[row].itemState[i] == 1) {
            lbl_1_bss_1D4[row].pos[i].z -= 40.0f;
            if (lbl_1_bss_1D4[row].pos[i].z < lbl_1_bss_754[0].pos.z - 210.0f) {
                lbl_1_bss_1D4[row].itemState[i] = -2;
                fn_1_7724(M630_SFX_OBJECT_IMPACT, lbl_1_bss_1D4[row].model[i], 1);
            }
        } else if (lbl_1_bss_1D4[row].itemState[i] < 0) {
            if (lbl_1_bss_1D4[row].transparency[i] == 1.0f) {
                fn_80026A44(lbl_1_bss_1D4[row].secondaryModel[i], 1);
                fn_8002D86C(lbl_1_bss_1D4[row].secondaryModel[i], 1.0f);
                fn_8002D414(lbl_1_bss_1D4[row].secondaryModel[i], 0.0f);
                if (lbl_1_bss_1D4[row].itemState[i] == -1) {
                    fn_800268B0(lbl_1_bss_1D4[row].secondaryModel[i], 1.8000001f, 1.8000001f,
                                      1.8000001f);
                } else {
                    fn_800268B0(lbl_1_bss_1D4[row].secondaryModel[i], 0.90000004f,
                                      0.90000004f, 0.90000004f);
                }
            }
            fn_800268B0(lbl_1_bss_1D4[row].model[i],
                1.8000001f + (5.0f - 5.0f * lbl_1_bss_1D4[row].transparency[i]),
                1.8000001f + (5.0f - 5.0f * lbl_1_bss_1D4[row].transparency[i]),
                1.8000001f + (5.0f - 5.0f * lbl_1_bss_1D4[row].transparency[i]));
            fn_80026CBC(lbl_1_bss_1D4[row].model[i], lbl_1_bss_1D4[row].transparency[i]);
            lbl_1_bss_1D4[row].transparency[i] -= 0.1f;
            if (lbl_1_bss_1D4[row].transparency[i] < 0.0f) {
                lbl_1_bss_1D4[row].transparency[i] = 0.0f;
                fn_800269E8(lbl_1_bss_1D4[row].model[i], 1);
                fn_800268B0(lbl_1_bss_1D4[row].model[i], 1.8000001f, 1.8000001f, 1.8000001f);
                lbl_1_bss_1D4[row].itemState[i] = 0;
            }
        }
    }
    if (lbl_1_bss_1D4[row].phaseState != 0) {
        if (lbl_1_bss_1D4[row].phaseState == 7 && lbl_1_bss_754[player].hitState >= 0) {
            fn_8005C2C8(lbl_1_bss_754[player].charNo, lbl_1_bss_754[player].motionId[6],
                               0.0f, 8.0f, ((1 << 30) | (1 << 0)));
            fn_8002D86C(lbl_1_bss_754[player].charNo, 1.0f);
        }
        if (lbl_1_bss_1D4[row].phaseState == 30) {
            lbl_1_bss_1D4[row].phaseState = 0;
        } else {
            lbl_1_bss_1D4[row].phaseState++;
        }
    }
}

/* The play-contact callback calls this for each character to detect object hits. */

int fn_1_4CD4(int player)
{
    int i;
    int j;
    int result = 0;
    HuVecF playerPos;
    HuVecF itemPos;
    HuVecF delta;
    float distance;

    playerPos = lbl_1_bss_754[player].pos;
    playerPos.y = 0.0f;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 15; j++) {
            if (lbl_1_bss_1D4[i].itemState[j] == 1) {
                itemPos = lbl_1_bss_1D4[i].pos[j];
                itemPos.y = 0.0f;
                PSVECSubtract(&itemPos, &playerPos, &delta);
                distance = PSVECMag(&delta);
                if (distance < 65.2f) {
                    fn_1_7724(M630_SFX_OBJECT_IMPACT, lbl_1_bss_754[player].modelId, 1);
                    lbl_1_bss_1D4[i].itemState[j] = -1;
                    lbl_1_bss_1D4[i].playerIndex[j] = player;
                    if (lbl_1_bss_754[player].hitState == 0) {
                        lbl_1_bss_754[player].hitState = -1;
                        if (player == 0) {
                            fn_8006A030(lbl_1_bss_754[player].padNo, 20, 20, 0);
                            return 1;
                        }
                        fn_8006A030(lbl_1_bss_754[player].padNo, 20, 7, 3);
                    }
                }
            }
        }
    }
    return result;
}

/* A child process created during player setup advances hit reactions and row effects each frame. */

void fn_1_4F10(void)
{
    int i;
    int j;

        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_754[i].hitState == -1) {
                if (i == 0) {
                    lbl_1_bss_754[i].hitState = -3;
                    fn_8005C2C8(lbl_1_bss_754[i].charNo, lbl_1_bss_754[i].motionId[8], 0.0f,
                                       8.0f, ((1 << 30) | (1 << 0)));
                } else {
                    lbl_1_bss_754[i].hitState = -2;
                    fn_8005C2C8(lbl_1_bss_754[i].charNo, lbl_1_bss_754[i].motionId[3], 0.0f,
                                       8.0f, ((1 << 30) | (1 << 0)));
                    fn_8002D86C(lbl_1_bss_754[i].charNo, 1.0f);
                }
            } else if (lbl_1_bss_754[i].hitState == -2) {
                if (++lbl_1_bss_754[i].hitTimerFrames == 90) {
                    lbl_1_bss_754[i].hitState = 1;
                    fn_8005C2C8(lbl_1_bss_754[i].charNo,
                                       lbl_1_bss_754[i].motionId[i == 0 ? 0 : 6], 0.0f, 8.0f,
                                       ((1 << 30) | (1 << 0)));
                    fn_8002D86C(lbl_1_bss_754[i].charNo, 1.0f);
                    lbl_1_bss_754[i].hitTimerFrames = 0;
                }
            } else if (lbl_1_bss_754[i].hitState == 1) {
                if ((lbl_1_bss_754[i].hitTimerFrames / 3) % 2) {
                    fn_80026A44(lbl_1_bss_754[i].modelId, 1);
                } else {
                    fn_800269E8(lbl_1_bss_754[i].modelId, 1);
                }
                if (++lbl_1_bss_754[i].hitTimerFrames == 60) {
                    lbl_1_bss_754[i].hitState = 0;
                    lbl_1_bss_754[i].hitTimerFrames = 0;
                }
            } else {
                fn_80026A44(lbl_1_bss_754[i].modelId, 1);
            }
        }
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 15; j++) {
                if (fn_8002D75C(lbl_1_bss_1D4[i].secondaryModel[j])) {
                    fn_800269E8(lbl_1_bss_1D4[i].secondaryModel[j], 1);
                }
            }
        }
        HuPrcVSleep();
}

/* The play-contact update calls this for CPU slots: slot 0 steers around approaching objects, while
 * opponents choose A presses by difficulty and nearby player positions. */

void fn_1_52F4(int player)
{
    int i;
    int count;
    int k;
    int j;
    int total;
    int roll;
    float margin;
    float lookAhead;
    HuVecF candidates[3];
    HuVecF base;
    HuVecF current;
    HuVecF delta;
    int limits[6] = { 1000, 800, 600, 400, 250, 100 };
    int weights[4][5] = {
        { 0, 5, 30, 50, 0 }, { 0, 10, 40, 40, 0 },
        { 5, 15, 50, 30, 0 }, { 20, 30, 30, 20, 0 },
    };
    int stick[4] = { 56, 56, 56, 56 };
    /* Initialized threshold values retained by the game but not read here. */
    int unusedThresholdTableA[4] = { 70, 80, 90, 98 };
    /* Initialized threshold values retained by the game but not read here. */
    int unusedThresholdTableB[4] = { 50, 75, 90, 98 };

    lbl_802BFFFC[lbl_1_bss_754[player].padNo] = 0;
    lbl_802BFFF8[lbl_1_bss_754[player].padNo] = 0;
    lbl_802C0008[lbl_1_bss_754[player].padNo] = 0;
    margin = lbl_1_bss_174[player].difficulty >= 2   ? 50.0f
             : lbl_1_bss_174[player].difficulty == 1 ? 60.0f + (float) (u32) frandmod(100)
                                                     : 80.0f + (float) (u32) frandmod(150);
    if (player == 0) {
        base = lbl_1_bss_754[player].pos;
        total = 0;
        roll = frandmod(100);
        for (k = 0; k < 5; k++) {
            total += weights[lbl_1_bss_174[player].difficulty][k];
            if (roll < total) {
                lookAhead = (float) (u32) frandmod(limits[k]);
                if (lookAhead < limits[k + 1]) {
                    lookAhead = limits[k + 1];
                }
                break;
            }
        }
        /* There is no fallback when the probability table selects no entry. */
        lookAhead += base.z;
        count = 0;
        for (i = 0; i < 3; i++) {
            for (j = 0; j < 15; j++) {
                if (lbl_1_bss_1D4[i].itemState[j] == 1) {
                    current = lbl_1_bss_1D4[i].pos[j];
                    if (current.z < lookAhead) {
                        if (count == 0) {
                            candidates[0] = current;
                            count = 1;
                        } else if (count == 1) {
                            if (current.z > candidates[0].z) {
                                candidates[1] = current;
                            } else {
                                candidates[1] = candidates[0];
                                candidates[0] = current;
                            }
                            count = 2;
                        } else if (count == 2) {
                            if (current.z > candidates[0].z) {
                                if (current.z > candidates[1].z) {
                                    candidates[2] = current;
                                } else {
                                    candidates[2] = candidates[1];
                                    candidates[1] = current;
                                }
                            } else {
                                candidates[2] = candidates[1];
                                candidates[1] = candidates[0];
                                candidates[0] = current;
                            }
                            count = 3;
                        } else {
                            if (current.z > candidates[0].z) {
                                if (current.z > candidates[1].z) {
                                    if (!(current.z > candidates[2].z)) {
                                        candidates[2] = current;
                                    }
                                } else {
                                    candidates[2] = candidates[1];
                                    candidates[1] = current;
                                }
                            } else {
                                candidates[2] = candidates[1];
                                candidates[1] = candidates[0];
                                candidates[0] = current;
                            }
                        }
                    }
                }
            }
        }
        base.y = base.z = 0.0f;
        for (i = 0; i < count; i++) {
            float distance;
            candidates[i].y = candidates[i].z = 0.0f;
            PSVECSubtract(&candidates[i], &base, &delta);
            distance = PSVECMag(&delta);
            if (distance < 65.2f) {
                if (lbl_1_bss_16C == 0) {
                    if (candidates[i].x < base.x) {
                        lbl_1_bss_168 = 65.2f + margin + candidates[i].x;
                        lbl_1_bss_16C = 1;
                        if (lbl_1_bss_168 > 368.0f) {
                            lbl_1_bss_168 = candidates[i].x - (65.2f + margin);
                            lbl_1_bss_16C = -1;
                        }
                    } else {
                        lbl_1_bss_168 = candidates[i].x - (65.2f + margin);
                        lbl_1_bss_16C = -1;
                        if (lbl_1_bss_168 < -368.0f) {
                            lbl_1_bss_168 = 65.2f + margin + candidates[i].x;
                            lbl_1_bss_16C = 1;
                        }
                    }
                } else if (lbl_1_bss_16C == 1) {
                    lbl_1_bss_168 = 65.2f + margin + candidates[i].x;
                    if (lbl_1_bss_168 > 368.0f) {
                        lbl_1_bss_168 = candidates[i].x - (65.2f + margin);
                        lbl_1_bss_16C = -1;
                    }
                } else if (lbl_1_bss_16C == -1) {
                    lbl_1_bss_168 = candidates[i].x - (65.2f + margin);
                    if (lbl_1_bss_168 < -368.0f) {
                        lbl_1_bss_168 = 65.2f + margin + candidates[i].x;
                        lbl_1_bss_16C = 1;
                    }
                }
            }
        }
        if (lbl_1_bss_16C == 1) {
            if (base.x >= lbl_1_bss_168) {
                lbl_1_bss_16C = 0;
            } else {
                lbl_802BFFFC[lbl_1_bss_754[player].padNo] = stick[lbl_1_bss_174[player].difficulty];
            }
        } else if (lbl_1_bss_16C == -1) {
            if (base.x <= lbl_1_bss_168) {
                lbl_1_bss_16C = 0;
            } else {
                lbl_802BFFFC[lbl_1_bss_754[player].padNo] =
                    -stick[lbl_1_bss_174[player].difficulty];
            }
        } else if (lbl_1_bss_174[player].difficulty <= 1) {
            if (lbl_1_bss_174[player].moveTimerFrames == 0) {
                if ((u32) frandmod(30) == 15U) {
                    lbl_1_bss_174[player].moveTimerFrames = frandmod(10) + 15;
                }
            } else {
                if (--lbl_1_bss_174[player].moveTimerFrames <= 0) {
                    lbl_1_bss_174[player].moveTimerFrames = 0;
                    lbl_1_bss_174[player].moveDirection = frandmod(2);
                } else if (lbl_1_bss_174[player].moveDirection == 1) {
                    lbl_802BFFFC[lbl_1_bss_754[player].padNo] =
                        stick[lbl_1_bss_174[player].difficulty] / 2;
                    if (base.x > 318.0f) {
                        lbl_1_bss_174[player].moveDirection = 0;
                    }
                } else {
                    lbl_802BFFFC[lbl_1_bss_754[player].padNo] =
                        -stick[lbl_1_bss_174[player].difficulty] / 2;
                    if (base.x < -318.0f) {
                        lbl_1_bss_174[player].moveDirection = 1;
                    }
                }
            }
        }
        if (lbl_1_bss_16C == 0) {
            if (lbl_1_bss_754[player].rotationY < 0.0f) {
                lbl_1_bss_754[player].rotationY += 18.0f;
                if (lbl_1_bss_754[player].rotationY > 0.0f) {
                    lbl_1_bss_754[player].rotationY = 0.0f;
                }
                if (lbl_1_bss_754[player].currentMotion != 1) {
                    fn_8005C2C8(lbl_1_bss_754[player].charNo,
                                       lbl_1_bss_754[player].motionId[1], 0.0f, 5.0f,
                                       ((1 << 30) | (1 << 0)));
                    lbl_1_bss_754[player].currentMotion = 1;
                    return;
                }
            } else if (lbl_1_bss_754[player].rotationY > 0.0f) {
                lbl_1_bss_754[player].rotationY -= 18.0f;
                if (lbl_1_bss_754[player].rotationY < 0.0f) {
                    lbl_1_bss_754[player].rotationY = 0.0f;
                }
                if (lbl_1_bss_754[player].currentMotion != 1) {
                    fn_8005C2C8(lbl_1_bss_754[player].charNo,
                                       lbl_1_bss_754[player].motionId[1], 0.0f, 5.0f,
                                       ((1 << 30) | (1 << 0)));
                    lbl_1_bss_754[player].currentMotion = 1;
                    return;
                }
            }
        }
    } else if ((lbl_1_bss_174[player].difficulty == 0 && (u32) frandmod(100) < 5U) ||
               (lbl_1_bss_174[player].difficulty == 1 && (u32) frandmod(100) < 5U) ||
               (lbl_1_bss_174[player].difficulty == 2 && (u32) frandmod(100) < 20U) ||
               (lbl_1_bss_174[player].difficulty == 3 && (u32) frandmod(100) < 50U)) {
        count = 0;
        if ((u32) frandmod(2) != 0U) {
            fn_1_6360(player);
            count++;
        }
        if ((u32) frandmod(2) != 0U) {
            fn_1_6D70(player);
            count++;
        }
        if ((u32) frandmod(2) != 0U) {
            fn_1_7204(player);
            count++;
        }
        if (count == 0) {
            count = frandmod(3);
            if (count == 0) {
                fn_1_6360(player);
            } else if (count == 1) {
                fn_1_6D70(player);
            } else {
                fn_1_7204(player);
            }
        }
    }
}

/* The opponent action selector calls this strategy when choosing an action around nearby
 * players. */

void fn_1_6360(int player)
{
    HuVecF pos;
    HuVecF other;
    int chance[4] = { 20, 30, 70, 99 };
    int avoidChance[4] = { 20, 30, 80, 100 };
    int assistChance[4] = { 2, 5, 20, 40 };
    /* This initialized chance table is not read by this opponent strategy. */
    int unusedChanceTable[4] = { 70, 30, 2, 1 };

    pos = lbl_1_bss_754[player].pos;
    other = lbl_1_bss_754[0].pos;
    if (player == 1) {
        if ((u32) (frandmod(100) + 1) <= (u32) chance[lbl_1_bss_174[player].difficulty]) {
            if (pos.x >= other.x - 40.0f && pos.x <= 40.0f + other.x) {
                lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
                return;
            }
        } else if ((u32) (frandmod(100) + 1) <= (u32) chance[lbl_1_bss_174[player].difficulty]) {
            lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
            return;
        }
    } else if (player == 2) {
        if ((u32) (frandmod(100) + 1) <= (u32) chance[lbl_1_bss_174[player].difficulty]) {
            if (pos.x >= other.x - 40.0f && pos.x <= 40.0f + other.x) {
                if ((u32) (frandmod(100) + 1) <=
                    (u32) avoidChance[lbl_1_bss_174[player].difficulty]) {
                    other = lbl_1_bss_754[1].pos;
                    if (pos.x < (other.x - 40.0f) - 80.0f || pos.x > 80.0f + (40.0f + other.x)) {
                        lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
                        if (lbl_1_bss_174[1].type != 0 &&
                            (u32) (frandmod(100) + 1) <=
                                (u32) assistChance[lbl_1_bss_174[player].difficulty]) {
                            lbl_802C0008[lbl_1_bss_754[1].padNo] = PAD_BUTTON_A;
                            return;
                        }
                    }
                } else if ((u32) (frandmod(100) + 1) <=
                           (u32) chance[lbl_1_bss_174[player].difficulty]) {
                    lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
                    return;
                }
            }
        } else if ((u32) (frandmod(100) + 1) <= (u32) chance[lbl_1_bss_174[player].difficulty]) {
            lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
            return;
        }
    } else if ((u32) (frandmod(100) + 1) <= (u32) chance[lbl_1_bss_174[player].difficulty]) {
        if (pos.x >= other.x - 40.0f && pos.x <= 40.0f + other.x) {
            if ((u32) (frandmod(100) + 1) <= (u32) avoidChance[lbl_1_bss_174[player].difficulty]) {
                other = lbl_1_bss_754[1].pos;
                if (pos.x < (other.x - 40.0f) - 150.0f || pos.x > 150.0f + (40.0f + other.x)) {
                    if ((u32) (frandmod(100) + 1) <=
                        (u32) avoidChance[lbl_1_bss_174[player].difficulty]) {
                        other = lbl_1_bss_754[2].pos;
                        if (pos.x < (other.x - 40.0f) - 100.0f ||
                            pos.x > 100.0f + (40.0f + other.x)) {
                            lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
                            if (lbl_1_bss_174[1].type != 0 &&
                                (u32) (frandmod(100) + 1) <=
                                    (u32) assistChance[lbl_1_bss_174[player].difficulty]) {
                                lbl_802C0008[lbl_1_bss_754[1].padNo] = PAD_BUTTON_A;
                            }
                            if ((u32) (frandmod(100) + 1) <=
                                (u32) avoidChance[lbl_1_bss_174[player].difficulty]) {
                                pos = lbl_1_bss_754[2].pos;
                                other = lbl_1_bss_754[1].pos;
                                if ((pos.x < (other.x - 40.0f) - 80.0f ||
                                     pos.x > 80.0f + (40.0f + other.x)) &&
                                    lbl_1_bss_174[2].type != 0 &&
                                    (u32) (frandmod(100) + 1) <=
                                        (u32) assistChance[lbl_1_bss_174[player].difficulty]) {
                                    lbl_802C0008[lbl_1_bss_754[2].padNo] = PAD_BUTTON_A;
                                    return;
                                }
                            } else if ((u32) (frandmod(100) + 1) <=
                                       (u32) chance[lbl_1_bss_174[player].difficulty]) {
                                lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
                                return;
                            }
                        }
                    } else if ((u32) (frandmod(100) + 1) <=
                               (u32) chance[lbl_1_bss_174[player].difficulty]) {
                        lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
                        return;
                    }
                }
            } else if ((u32) (frandmod(100) + 1) <=
                       (u32) chance[lbl_1_bss_174[player].difficulty]) {
                lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
                return;
            }
        }
    } else if ((u32) (frandmod(100) + 1) <= (u32) chance[lbl_1_bss_174[player].difficulty]) {
        lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
    }
}

/* The opponent action selector calls this strategy when characters match its spacing patterns. */

void fn_1_6D70(int player)
{
    HuVecF pos[4];
    int spacingPatternFound = 0;

    pos[0] = lbl_1_bss_754[0].pos;
    pos[1] = lbl_1_bss_754[1].pos;
    pos[2] = lbl_1_bss_754[2].pos;
    pos[3] = lbl_1_bss_754[3].pos;
    if ((pos[1].x > (80.0f + pos[3].x)) && (pos[2].x > (80.0f + pos[3].x)) &&
        (pos[3].x >= (pos[0].x - 60.0f)) && (pos[3].x <= (pos[0].x - 20.0f))) {
        spacingPatternFound = 1;
    }
    if ((pos[1].x < (pos[3].x - 80.0f)) && (pos[2].x < (pos[3].x - 80.0f)) &&
        (pos[3].x >= (20.0f + pos[0].x)) && (pos[3].x <= (60.0f + pos[0].x))) {
        spacingPatternFound = 1;
    }
    if ((pos[1].x > (80.0f + pos[3].x)) && (pos[2].x < (pos[3].x - 80.0f)) &&
        (pos[3].x >= (pos[0].x - 40.0f)) && (pos[3].x <= (40.0f + pos[0].x))) {
        spacingPatternFound = 1;
    }
    if ((pos[1].x < (pos[3].x - 80.0f)) && (pos[2].x > (80.0f + pos[3].x)) &&
        (pos[3].x >= (pos[0].x - 40.0f)) && (pos[3].x <= (40.0f + pos[0].x))) {
        spacingPatternFound = 1;
    }
    if (spacingPatternFound) {
        if (player == 1) {
            lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
        } else if (player == 2) {
            if ((pos[2].x < ((pos[1].x - 40.0f) - 80.0f)) ||
                (pos[2].x > (80.0f + (40.0f + pos[1].x)))) {
                lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
            }
        } else if (player == 3) {
            if (((pos[0].x < ((pos[1].x - 40.0f) - 150.0f)) ||
                 (pos[0].x > (150.0f + (40.0f + pos[1].x)))) &&
                ((pos[0].x < ((pos[2].x - 40.0f) - 100.0f)) ||
                 (pos[0].x > (100.0f + (40.0f + pos[2].x))))) {
                lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
            }
        }
    }
}

/* The opponent action selector calls this strategy when opponent positions are evenly spaced. */

void fn_1_7204(int player)
{
    HuVecF pos[3];
    int spacingPatternFound = 0;

    pos[0] = lbl_1_bss_754[1].pos;
    pos[1] = lbl_1_bss_754[2].pos;
    pos[2] = lbl_1_bss_754[3].pos;
    if (((pos[1].x - pos[0].x) > 80.0f) && ((pos[1].x - pos[0].x) < 200.0f) &&
        ((pos[2].x - pos[1].x) > 80.0f) && ((pos[2].x - pos[1].x) < 200.0f)) {
        spacingPatternFound = 1;
    }
    if (((pos[2].x - pos[0].x) > 80.0f) && ((pos[2].x - pos[0].x) < 200.0f) &&
        ((pos[1].x - pos[2].x) > 80.0f) && ((pos[1].x - pos[2].x) < 200.0f)) {
        spacingPatternFound = 1;
    }
    if (((pos[0].x - pos[1].x) > 80.0f) && ((pos[0].x - pos[1].x) < 200.0f) &&
        ((pos[2].x - pos[0].x) > 80.0f) && ((pos[2].x - pos[0].x) < 200.0f)) {
        spacingPatternFound = 1;
    }
    if (((pos[1].x - pos[0].x) > 80.0f) && ((pos[1].x - pos[0].x) < 200.0f) &&
        ((pos[0].x - pos[2].x) > 80.0f) && ((pos[0].x - pos[2].x) < 200.0f)) {
        spacingPatternFound = 1;
    }
    if (((pos[0].x - pos[1].x) > 80.0f) && ((pos[0].x - pos[1].x) < 200.0f) &&
        ((pos[1].x - pos[2].x) > 80.0f) && ((pos[1].x - pos[2].x) < 200.0f)) {
        spacingPatternFound = 1;
    }
    if (((pos[0].x - pos[2].x) > 80.0f) && ((pos[0].x - pos[2].x) < 200.0f) &&
        ((pos[2].x - pos[1].x) > 80.0f) && ((pos[2].x - pos[1].x) < 200.0f)) {
        spacingPatternFound = 1;
    }
    if (spacingPatternFound) {
        lbl_802C0008[lbl_1_bss_754[player].padNo] = PAD_BUTTON_A;
    }
}

/* Play and sequence callbacks call this to occasionally play alternate decorative motions. */

void fn_1_75C8(void)
{
    int i;

    for (i = 0; i < 10; i++) {
        switch (lbl_1_bss_28[i].motionState) {
        case 0:
            if ((u32)frandmod(120) == 0U) {
                fn_8002CCC0(lbl_1_bss_28[i].model,
                    lbl_1_bss_28[i].motionB, 0.0f, 8.0f, 0);
                lbl_1_bss_28[i].motionState = 1;
            }
            break;
        case 1:
            if (fn_8002D3EC(lbl_1_bss_28[i].model) == -1) {
                fn_8002CCC0(lbl_1_bss_28[i].model,
                    lbl_1_bss_28[i].motionA, 0.0f, 8.0f, 0);
                lbl_1_bss_28[i].motionState = 0;
            }
            break;
        }
    }
}

/* Object-hit handling calls this to play an effect panned to the model's screen position. */

s16 fn_1_7724(s16 effectNo, s16 modelId, s16 cameraId)
{
    s16 handle;
    int panMin;
    int pan;
    int panMax;
    float scale;
    HuVecF world;
    HuVecF screen;

    panMin = 48;
    panMax = 80;
    scale = 576.0f / (panMax - panMin);
    world = lbl_802C01C8[modelId].pos;
    Hu3D3Dto2D(&world, cameraId, &screen);
    if (screen.x < 0.0f || screen.x > 576.0f) {
        return -1;
    }
    if (screen.y < 0.0f || screen.y > 480.0f) {
        return -1;
    }
    pan = panMin + (int)(screen.x / scale);
    handle = HuAudFXPlay(effectNo);
    HuAudFXPanning(handle, pan);
    return handle;
}
