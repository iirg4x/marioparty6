#define _MATH_H
#include "REL/m632dll.h"

void fn_1_4C8C(HU3D_MODEL *modelP, Mtx *mtx)
{

}

void fn_1_4C90(void)
{
    s32 var_r31;
    s16 model;

    model = Hu3DHookFuncCreate(fn_1_4C8C);
    Hu3DModelCameraSet(model, 2U);
    var_r31 = 0;
    while (var_r31 < 4) {
        lbl_1_bss_0.playerTimer[var_r31] = 0;
        /* The duplicate initialization is present in the shipped code. */
        lbl_1_bss_0.playerTimer[var_r31] = 0;
        var_r31 += 1;
    }
    var_r31 = 0;
    while (var_r31 < 64) {
        lbl_1_bss_0.gridA[var_r31] = 128;
        var_r31 += 1;
    }
}

void fn_1_4D48(void)
{
    Point3d sp8;
    int var_r25;
    int temp_r24;
    int temp_r23;
    int var_r27;
    int var_r26;
    int var_r29;
    int var_r28;
    int var_r31;
    u8 *var_r30;
    int found; /* Set but never read in the shipped routine; original purpose unknown. */

    var_r30 = lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    var_r31 = 0;
    while (var_r31 < 64) {
        lbl_1_bss_0.gridB[var_r31] = 10;
        /* The first pass advances this iterator without reading it. */
        var_r30 += 1;
        var_r31 += 1;
    }
    var_r30 = lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    var_r31 = 0;
    while (var_r31 < 64) {
        temp_r24 = var_r31 % 8;
        temp_r23 = var_r31 / 8;
        if (((s8) var_r30[0] >= 2) && ((s8) var_r30[0] <= 3)) {
            lbl_1_bss_0.gridB[var_r31] += 255;
        } else {
            found = 0;
            var_r25 = 0;
            while (var_r25 < lbl_1_bss_0.collisionCount) {
                sp8 = lbl_1_bss_0.collisionActors[var_r25]->pos;
                var_r27 = (s32) ((400.0f + sp8.x) / 100.0f);
                var_r26 = (s32) ((400.0f + sp8.z) / 100.0f);
                if (var_r27 < 0) {
                    var_r27 = 0;
                }
                if (var_r27 >= 8) {
                    var_r27 = 7;
                }
                if (var_r26 < 0) {
                    var_r26 = 0;
                }
                if (var_r26 >= 8) {
                    var_r26 = 7;
                }
                if ((temp_r24 == var_r27) && (temp_r23 == var_r26)) {
                    int sp14[121] = {
                        4,4,4,4,8,8,8,8,4,4,4,
                        4,4,4,8,16,16,16,16,8,4,4,
                        4,4,8,16,32,32,32,32,16,4,4,
                        4,8,16,32,64,64,64,32,16,8,4,
                        8,16,32,64,128,128,128,64,32,16,8,
                        8,16,32,64,128,255,128,64,32,16,8,
                        8,16,32,64,128,128,128,64,32,16,8,
                        4,8,16,32,64,64,64,32,16,8,4,
                        4,4,8,16,32,32,32,16,8,4,4,
                        4,4,4,8,16,16,16,8,4,4,4,
                        4,4,4,4,8,8,8,4,4,4,4
                    };
                    var_r28 = -5;
                    while (var_r28 <= 5) {
                        var_r29 = -5;
                        while (var_r29 <= 5) {
                            if (((s32) (temp_r24 + var_r29) >= 0) && ((s32) (temp_r24 + var_r29) < 8) && ((s32) (temp_r23 + var_r28) >= 0) && ((s32) (temp_r23 + var_r28) < 8)) {
                                lbl_1_bss_0.gridB[temp_r24 + var_r29 + (temp_r23 + var_r28) * 8] += sp14[(var_r29 + 5) + (var_r28 + 5) * 11];
                            }
                            var_r29 += 1;
                        }
                        var_r28 += 1;
                    }
                    found = 1;
                }
                var_r25 += 1;
            }
        }
        var_r30 += 1;
        var_r31 += 1;
    }
    var_r31 = 0;
    while (var_r31 < 64) {
        if (lbl_1_bss_0.gridB[var_r31] < 0) {
            lbl_1_bss_0.gridB[var_r31] = 0;
        } else if (lbl_1_bss_0.gridB[var_r31] > 255) {
            lbl_1_bss_0.gridB[var_r31] = 255;
        }
        var_r31 += 1;
    }
}

void fn_1_50A0(s32 arg0)
{
    int sp18[25] = {
        0,0,16,0,0, 0,16,32,16,0, 16,32,64,32,16,
        0,16,32,16,0, 0,0,16,0,0
    };
    Point3d spC;
    int var_r31;
    int var_r30;
    int var_r29;
    int var_r28;
    int var_r27;
    MGPLAYER *player;

    memcpy(lbl_1_bss_0.gridA, lbl_1_bss_0.gridB, sizeof(lbl_1_bss_0.gridA));
    var_r31 = 0;
    while (var_r31 < 4) {
        if ((var_r31 != arg0) && (lbl_1_bss_0.group[var_r31] == 1) && (lbl_1_bss_0.playerState[var_r31] == 0)) {
            player = lbl_1_bss_0.players[var_r31];
            spC = player->actor->pos;
            var_r28 = (s32) ((400.0f + spC.x) / 100.0f);
            var_r27 = (s32) ((400.0f + spC.z) / 100.0f);
            if (var_r28 < 0) {
                var_r28 = 0;
            }
            if (var_r28 >= 8) {
                var_r28 = 7;
            }
            if (var_r27 < 0) {
                var_r27 = 0;
            }
            if (var_r27 >= 8) {
                var_r27 = 7;
            }
            var_r29 = -2;
            while (var_r29 <= 2) {
                var_r30 = -2;
                while (var_r30 <= 2) {
                    if (((s32) (var_r28 + var_r30) >= 0) && ((s32) (var_r28 + var_r30) < 8) && ((s32) (var_r27 + var_r29) >= 0) && ((s32) (var_r27 + var_r29) < 8)) {
                        lbl_1_bss_0.gridA[var_r28 + var_r30 + (var_r27 + var_r29) * 8] += sp18[(var_r30 + 2) + (var_r29 + 2) * 5];
                    }
                    var_r30 += 1;
                }
                var_r29 += 1;
            }
        }
        var_r31 += 1;
    }
    var_r31 = 0;
    while (var_r31 < 64) {
        if ((s32) lbl_1_bss_0.gridA[var_r31] < 0) {
            lbl_1_bss_0.gridA[var_r31] = 0;
        } else if ((s32) lbl_1_bss_0.gridA[var_r31] > 255) {
            lbl_1_bss_0.gridA[var_r31] = 255;
        }
        var_r31 += 1;
    }
}

void fn_1_535C(void)
{
    s32 var_r31;

    fn_1_4D48();
    var_r31 = 0;
    while (var_r31 < 4) {
        if ((GwPlayerConf[var_r31].type != 0) && ((s32) lbl_1_bss_0.playerState[var_r31] == 0)) {
            if ((s32) lbl_1_bss_0.group[var_r31] == 0) {
                fn_1_5400(var_r31);
            } else {
                fn_1_597C(var_r31);
            }
        }
        var_r31 += 1;
    }
}

void fn_1_5400(s32 arg0)
{
    Point3d sp30;
    Point3d sp24;
    Point3d sp18;
    Point3d spC;
    MGPLAYER *sp8;
    f32 temp_f31;
    s32 temp_r3;
    s32 var_r29;
    u32 var_r30;
    MGACTOR *temp_r28;

    sp8 = lbl_1_bss_0.players[arg0];
    switch (GwPlayerConf[arg0].comDif) {
    case 0:                                         /* switch 1 */
        var_r30 = 30U;
        break;
    case 1:                                         /* switch 1 */
        var_r30 = 15U;
        break;
    case 2:                                         /* switch 1 */
        var_r30 = 5U;
        break;
    case 3:                                         /* switch 1 */
        var_r30 = 0U;
        break;
    }
    if ((s32) lbl_1_bss_0.playerTimer[arg0] < 0) {
        if (frandmod(100) < var_r30) {
            lbl_1_bss_0.playerAIState[arg0] = 0;
            lbl_1_bss_0.playerTimer[arg0] = (s32) (frandmod(60) + 30);
        } else {
            lbl_1_bss_0.playerAIState[arg0] = 1;
            switch (GwPlayerConf[arg0].comDif) {
            case 0:                                 /* switch 2 */
                var_r30 = 20U;
                break;
            case 1:                                 /* switch 2 */
                var_r30 = 30U;
                break;
            case 2:                                 /* switch 2 */
                var_r30 = 55U;
                break;
            case 3:                                 /* switch 2 */
                var_r30 = 80U;
                break;
            }
            if (frandmod(100) < var_r30) {
                sp18.x = sp18.y = sp18.z = 0.0f;
                var_r29 = 0;
                while (var_r29 < (s32) lbl_1_bss_0.collisionCount) {
                    temp_r28 = lbl_1_bss_0.collisionActors[var_r29];
                    sp18.x += temp_r28->pos.x;
                    sp18.z += temp_r28->pos.z;
                    var_r29 += 1;
                }
                sp18.x /= (f32) lbl_1_bss_0.collisionCount;
                sp18.z /= (f32) lbl_1_bss_0.collisionCount;
                do {
                    temp_r3 = frandmod(4);
                } while (lbl_1_bss_0.group[temp_r3] != 1 || lbl_1_bss_0.playerState[temp_r3] != 0);
                {
                    MGPLAYER *selectedPlayer = lbl_1_bss_0.players[temp_r3];
                    sp24 = selectedPlayer->actor->pos;
                }
                PSVECSubtract(&sp24, &sp18, &sp30);
                sp30.z = -sp30.z;
                if (PSVECMag(&sp30) >= 0.01f) {
                    PSVECNormalize(&sp30, &sp30);
                }
                lbl_1_bss_0.playerDirection[arg0] = sp30;
                lbl_1_bss_0.playerTimer[arg0] = (s32) (frandmod(20) + 20);
            } else {
                temp_f31 = (f32) (u32) frandmod(360);
                sp30.x = (f32) (cos((3.141592653589793 * (f64) temp_f31) / 180.0) - sin((3.141592653589793 * (f64) temp_f31) / 180.0));
                sp30.y = 0.0f;
                sp30.z = (f32) (sin((3.141592653589793 * (f64) temp_f31) / 180.0) + cos((3.141592653589793 * (f64) temp_f31) / 180.0));
                PSVECNormalize(&sp30, &sp30);
                lbl_1_bss_0.playerDirection[arg0] = sp30;
                lbl_1_bss_0.playerTimer[arg0] = (s32) (frandmod(60) + 30);
            }
        }
    }
    switch (lbl_1_bss_0.playerAIState[arg0]) {
    case 0:
        break;
    case 1:
        spC = lbl_1_bss_0.playerDirection[arg0];
        lbl_1_bss_0.scalar194 = 28.0f * spC.x;
        lbl_1_bss_0.scalar198 = 28.0f * spC.z;
        break;
    }
    lbl_1_bss_0.playerTimer[arg0]--;
}

void fn_1_597C(s32 arg0)
{
    Point3d pos;
    Point3d direction;
    MGPLAYER *temp_r27;
    int var_r26;
    int var_r25;
    int var_r28;
    int var_r31;
    int var_r30;

    temp_r27 = lbl_1_bss_0.players[arg0];
    pos = temp_r27->actor->pos;
    var_r31 = (s32) ((400.0f + pos.x) / 100.0f);
    var_r30 = (s32) ((400.0f + pos.z) / 100.0f);
    if (var_r31 < 0) {
        var_r31 = 0;
    }
    if (var_r31 >= 8) {
        var_r31 = 7;
    }
    if (var_r30 < 0) {
        var_r30 = 0;
    }
    if (var_r30 >= 8) {
        var_r30 = 7;
    }
    fn_1_50A0(arg0);
    switch (lbl_1_bss_0.playerAIState[arg0]) {
    case 0:
        if ((s32) lbl_1_bss_0.gridA[var_r31 + (var_r30 * 8)] >= (s32) lbl_1_data_198[GwPlayerConf[arg0].comDif]) {
            lbl_1_bss_0.playerAIState[arg0] = 2;
            fn_1_5D34(arg0);
        }
        break;
    case 2:
        var_r26 = 0;
        var_r25 = 0;
        if ((s32) lbl_1_bss_0.gridA[var_r31 + (var_r30 * 8)] >= (s32) lbl_1_data_198[GwPlayerConf[arg0].comDif]) {
            fn_1_5D34(arg0);
            goto block_28;
        }
        var_r28 = 0;
        if ((s32) (var_r31 - 1) >= 0) {
            var_r28 += lbl_1_bss_0.gridA[(var_r31 - 1) + (var_r30 * 8)];
        }
        if ((s32) (var_r31 + 1) < 8) {
            var_r28 += lbl_1_bss_0.gridA[(var_r31 + 1) + (var_r30 * 8)];
        }
        if ((s32) (var_r30 - 1) >= 0) {
            var_r28 += lbl_1_bss_0.gridA[var_r31 + ((var_r30 - 1) * 8)];
        }
        if ((s32) (var_r30 + 1) < 8) {
            var_r28 += lbl_1_bss_0.gridA[var_r31 + ((var_r30 + 1) * 8)];
        }
        if (var_r28 < (s32) lbl_1_data_198[GwPlayerConf[arg0].comDif]) {
            lbl_1_bss_0.playerAIState[arg0] = 0;
            return;
        }
block_28:
        direction = lbl_1_bss_0.playerDirection[arg0];
        if ((s32) lbl_1_bss_0.gridA[var_r31 + (var_r30 * 8)] >= 144) {
            var_r26 = 256;
            var_r25 = 256;
        }
        MgPlayerPadSet(temp_r27, (s32) (56.0f * direction.x), (s32) (56.0f * -direction.z), var_r26, var_r25);
        break;
    case 1:
        break;
    }
}

void fn_1_5D34(s32 arg0)
{
    Point3d pos;
    Point3d direction;
    s32 spC;
    s32 sp8;
    f32 var_f31;
    s32 var_r31;
    s32 var_r30;
    MGPLAYER *player;

    player = lbl_1_bss_0.players[arg0];
    pos = player->actor->pos;
    var_r31 = (s32) ((400.0f + pos.x) / 100.0f);
    var_r30 = (s32) ((400.0f + pos.z) / 100.0f);
    if (var_r31 < 0) {
        var_r31 = 0;
    }
    if (var_r31 >= 8) {
        var_r31 = 7;
    }
    if (var_r30 < 0) {
        var_r30 = 0;
    }
    if (var_r30 >= 8) {
        var_r30 = 7;
    }
    switch (GwPlayerConf[arg0].comDif) {
    case 0:
        var_f31 = 128.0f;
        break;
    case 1:
        var_f31 = 80.0f;
        break;
    case 2:
        var_f31 = 40.0f;
        break;
    case 3:
        var_f31 = 16.0f;
        break;
    }
    if ((fn_1_604C(arg0, var_r31, var_r30, &spC, &sp8) != 0) && ((var_r31 != spC) || (var_r30 != sp8)) && ((f32) lbl_1_bss_0.gridA[var_r31 + var_r30 * 8] >= var_f31)) {
        direction.x = (f32) (spC - var_r31);
        direction.y = 0.0f;
        direction.z = (f32) (sp8 - var_r30);
        PSVECNormalize(&direction, &direction);
        lbl_1_bss_0.playerDirection[arg0] = direction;
    }
}

int fn_1_5FE4(const void *a, const void *b)
{
    const M632GridCandidate *arg0 = a;
    const M632GridCandidate *arg1 = b;
    if ((arg0->distance == arg1->distance) && (arg0->cost == arg1->cost)) {
        return 0;
    }
    if (arg0->cost < arg1->cost) {
        return -1;
    }
    return 1;
}

s32 fn_1_604C(s32 arg0, s32 arg1, s32 arg2, s32 *arg3, s32 *arg4)
{
    M632GridCandidate sp24[64];
    Point3d sp18;
    Point3d spC;
    M632GridCandidate *var_r31;
    f32 var_f31;
    s32 var_r20;
    s32 var_r24;
    s32 var_r23;
    s32 var_r27;
    s32 var_r29;
    s32 var_r28;
    s32 var_r30;

    var_r31 = &sp24[0];
    if (arg1 < 0) {
        arg1 = 0;
    }
    if (arg1 >= 8) {
        arg1 = 7;
    }
    if (arg2 < 0) {
        arg2 = 0;
    }
    if (arg2 >= 8) {
        arg2 = 7;
    }
    var_r28 = 0;
    while (var_r28 < 8) {
        var_r29 = 0;
        while (var_r29 < 8) {
            spC.x = (f32) (var_r29 - arg1);
            spC.y = 0.0f;
            spC.z = (f32) (var_r28 - arg2);
            var_r31->x = var_r29;
            var_r31->z = var_r28;
            var_r31->distance = PSVECMag(&spC);
            var_f31 = var_r31->distance;
            if (var_f31 < 1.0f) {
                var_f31 = 1.0f;
            }
            var_r31->cost = (s32) ((f32) lbl_1_bss_0.gridA[var_r29 + var_r28 * 8] * var_f31);
            var_r31 += 1;
            var_r29 += 1;
        }
        var_r28 += 1;
    }
    fn_1_6574(&sp24[0], 64U, sizeof(sp24[0]), fn_1_5FE4);
    var_r31 = &sp24[0];
    var_r23 = 0;
    var_r24 = 0;
    var_r30 = 0;
    while (var_r30 < 4) {
        if ((lbl_1_bss_0.group[var_r30] == 1) && (lbl_1_bss_0.playerState[var_r30] == 0)) {
            var_r24 += 1;
        }
        var_r30 += 1;
    }
    var_r30 = 0;
    while (var_r30 < 64) {
        if (var_r24 > 1) {
            var_r27 = 0;
            while (var_r27 < 4) {
                if ((lbl_1_bss_0.group[var_r27] == 1) && (lbl_1_bss_0.playerState[var_r27] == 0) && (var_r27 != arg0)) {
                    sp18 = lbl_1_bss_0.players[var_r27]->actor->pos;
                    var_r29 = (s32) ((400.0f + sp18.x) / 100.0f);
                    var_r28 = (s32) ((400.0f + sp18.z) / 100.0f);
                    if ((var_r29 != var_r31->x) || (var_r28 != var_r31->z)) {
                        var_r23 = 1;
                        break;
                    }
                }
                var_r27 += 1;
            }
        } else {
            var_r23 = 1;
        }
        if (var_r23 != 0) {
            *arg3 = var_r31->x;
            *arg4 = var_r31->z;
            break;
        }
        var_r31 += 1;
        var_r30 += 1;
    }
    var_r20 = 1;
    if ((*arg3 == arg1) && (*arg4 == arg2)) {
        var_r20 = 0;
    }
    return var_r20;
}
