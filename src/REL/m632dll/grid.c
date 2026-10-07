/* Arena grid generation and CPU movement helpers. */
#define _MATH_H
#include "REL/m632dll.h"

/* Model-hook callback registered by fn_1_4C90; it leaves the supplied model transform unchanged. */
void fn_1_4C8C(HU3D_MODEL *modelP, Mtx *mtx)
{

}

/* Called during fn_1_3934 scene setup; creates one camera-2 hook and initializes CPU timers and gridA weights. */
void fn_1_4C90(void)
{
    s32 value9;
    s16 model;

    model = Hu3DHookFuncCreate(fn_1_4C8C);
    Hu3DModelCameraSet(model, 2U);
    value9 = 0;
    while (value9 < 4) {
        lbl_1_bss_0.playerTimer[value9] = 0;
                lbl_1_bss_0.playerTimer[value9] = 0;
        value9 += 1;
    }
    value9 = 0;
    while (value9 < 64) {
        lbl_1_bss_0.gridA[value9] = 128;
        value9 += 1;
    }
}

/* Called by fn_1_535C during gameplay updates to build the 8-by-8 obstacle-weight map. */
void fn_1_4D48(void)
{
    Point3d localValue0;
    int value3;
    int temporary2;
    int temporary1;
    int value5;
    int value4;
    int value7;
    int value6;
    int value9;
    u8 *value8;
    int found; /* Set when a collision actor occupies the current cell. */

    value8 = lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    value9 = 0;
    while (value9 < 64) {
        lbl_1_bss_0.gridB[value9] = 10;
        /* Fill base weights before adding the obstacle map. */
        value8 += 1;
        value9 += 1;
    }
    value8 = lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    value9 = 0;
    while (value9 < 64) {
        temporary2 = value9 % 8;
        temporary1 = value9 / 8;
        if (((s8) value8[0] >= 2) && ((s8) value8[0] <= 3)) {
            lbl_1_bss_0.gridB[value9] += 255;
        } else {
            found = 0;
            value3 = 0;
            while (value3 < lbl_1_bss_0.collisionCount) {
                localValue0 = lbl_1_bss_0.collisionActors[value3]->pos;
                value5 = (s32) ((400.0f + localValue0.x) / 100.0f);
                value4 = (s32) ((400.0f + localValue0.z) / 100.0f);
                if (value5 < 0) {
                    value5 = 0;
                }
                if (value5 >= 8) {
                    value5 = 7;
                }
                if (value4 < 0) {
                    value4 = 0;
                }
                if (value4 >= 8) {
                    value4 = 7;
                }
                if ((temporary2 == value5) && (temporary1 == value4)) {
                    int localValue2[121] = {
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
                    value6 = -5;
                    while (value6 <= 5) {
                        value7 = -5;
                        while (value7 <= 5) {
                            if (((s32) (temporary2 + value7) >= 0) && ((s32) (temporary2 + value7) < 8) && ((s32) (temporary1 + value6) >= 0) && ((s32) (temporary1 + value6) < 8)) {
                                lbl_1_bss_0.gridB[temporary2 + value7 + (temporary1 + value6) * 8] += localValue2[(value7 + 5) + (value6 + 5) * 11];
                            }
                            value7 += 1;
                        }
                        value6 += 1;
                    }
                    found = 1;
                }
                value3 += 1;
            }
        }
        value8 += 1;
        value9 += 1;
    }
    value9 = 0;
    while (value9 < 64) {
        if (lbl_1_bss_0.gridB[value9] < 0) {
            lbl_1_bss_0.gridB[value9] = 0;
        } else if (lbl_1_bss_0.gridB[value9] > 255) {
            lbl_1_bss_0.gridB[value9] = 255;
        }
        value9 += 1;
    }
}

/* Called by fn_1_597C for a group-1 CPU player to add nearby group-1 players to gridA and clamp its weights. */
void fn_1_50A0(s32 parameter0)
{
    int localValue3[25] = {
        0,0,16,0,0, 0,16,32,16,0, 16,32,64,32,16,
        0,16,32,16,0, 0,0,16,0,0
    };
    Point3d localValue1;
    int value9;
    int value8;
    int value7;
    int value6;
    int value5;
    MGPLAYER *player;

    memcpy(lbl_1_bss_0.gridA, lbl_1_bss_0.gridB, sizeof(lbl_1_bss_0.gridA));
    value9 = 0;
    while (value9 < 4) {
        if ((value9 != parameter0) && (lbl_1_bss_0.group[value9] == 1) && (lbl_1_bss_0.playerState[value9] == 0)) {
            player = lbl_1_bss_0.players[value9];
            localValue1 = player->actor->pos;
            value6 = (s32) ((400.0f + localValue1.x) / 100.0f);
            value5 = (s32) ((400.0f + localValue1.z) / 100.0f);
            if (value6 < 0) {
                value6 = 0;
            }
            if (value6 >= 8) {
                value6 = 7;
            }
            if (value5 < 0) {
                value5 = 0;
            }
            if (value5 >= 8) {
                value5 = 7;
            }
            value7 = -2;
            while (value7 <= 2) {
                value8 = -2;
                while (value8 <= 2) {
                    if (((s32) (value6 + value8) >= 0) && ((s32) (value6 + value8) < 8) && ((s32) (value5 + value7) >= 0) && ((s32) (value5 + value7) < 8)) {
                        lbl_1_bss_0.gridA[value6 + value8 + (value5 + value7) * 8] += localValue3[(value8 + 2) + (value7 + 2) * 5];
                    }
                    value8 += 1;
                }
                value7 += 1;
            }
        }
        value9 += 1;
    }
    value9 = 0;
    while (value9 < 64) {
        if ((s32) lbl_1_bss_0.gridA[value9] < 0) {
            lbl_1_bss_0.gridA[value9] = 0;
        } else if ((s32) lbl_1_bss_0.gridA[value9] > 255) {
            lbl_1_bss_0.gridA[value9] = 255;
        }
        value9 += 1;
    }
}

/* Called by fn_1_17C0 during play to refresh the arena map and update active CPU movement decisions. */
void fn_1_535C(void)
{
    s32 value9;

    fn_1_4D48();
    value9 = 0;
    while (value9 < 4) {
        if ((GwPlayerConf[value9].type != 0) && ((s32) lbl_1_bss_0.playerState[value9] == 0)) {
            if ((s32) lbl_1_bss_0.group[value9] == 0) {
                fn_1_5400(value9);
            } else {
                fn_1_597C(value9);
            }
        }
        value9 += 1;
    }
}

/* Called by fn_1_535C for a group-0 CPU player to choose its free-movement direction. */
void fn_1_5400(s32 parameter0)
{
    Point3d localValue5;
    Point3d localValue4;
    Point3d localValue3;
    Point3d localValue1;
    MGPLAYER *localValue0;
    f32 temporaryFloat0;
    s32 temporary0;
    s32 value7;
    u32 value8;
    MGACTOR *temporary4;

    localValue0 = lbl_1_bss_0.players[parameter0];
    switch (GwPlayerConf[parameter0].comDif) {
    case 0:
        value8 = 30U;
        break;
    case 1:
        value8 = 15U;
        break;
    case 2:
        value8 = 5U;
        break;
    case 3:
        value8 = 0U;
        break;
    }
    if ((s32) lbl_1_bss_0.playerTimer[parameter0] < 0) {
        if (frandmod(100) < value8) {
            lbl_1_bss_0.playerAIState[parameter0] = 0;
            lbl_1_bss_0.playerTimer[parameter0] = (s32) (frandmod(60) + 30);
        } else {
            lbl_1_bss_0.playerAIState[parameter0] = 1;
            switch (GwPlayerConf[parameter0].comDif) {
            case 0:
                value8 = 20U;
                break;
            case 1:
                value8 = 30U;
                break;
            case 2:
                value8 = 55U;
                break;
            case 3:
                value8 = 80U;
                break;
            }
            if (frandmod(100) < value8) {
                localValue3.x = localValue3.y = localValue3.z = 0.0f;
                value7 = 0;
                while (value7 < (s32) lbl_1_bss_0.collisionCount) {
                    temporary4 = lbl_1_bss_0.collisionActors[value7];
                    localValue3.x += temporary4->pos.x;
                    localValue3.z += temporary4->pos.z;
                    value7 += 1;
                }
                localValue3.x /= (f32) lbl_1_bss_0.collisionCount;
                localValue3.z /= (f32) lbl_1_bss_0.collisionCount;
                do {
                    temporary0 = frandmod(4);
                } while (lbl_1_bss_0.group[temporary0] != 1 || lbl_1_bss_0.playerState[temporary0] != 0);
                {
                    MGPLAYER *selectedPlayer = lbl_1_bss_0.players[temporary0];
                    localValue4 = selectedPlayer->actor->pos;
                }
                PSVECSubtract(&localValue4, &localValue3, &localValue5);
                localValue5.z = -localValue5.z;
                if (PSVECMag(&localValue5) >= 0.01f) {
                    PSVECNormalize(&localValue5, &localValue5);
                }
                lbl_1_bss_0.playerDirection[parameter0] = localValue5;
                lbl_1_bss_0.playerTimer[parameter0] = (s32) (frandmod(20) + 20);
            } else {
                temporaryFloat0 = (f32) (u32) frandmod(360);
                localValue5.x = (f32) (cos((3.141592653589793 * (f64) temporaryFloat0) / 180.0) - sin((3.141592653589793 * (f64) temporaryFloat0) / 180.0));
                localValue5.y = 0.0f;
                localValue5.z = (f32) (sin((3.141592653589793 * (f64) temporaryFloat0) / 180.0) + cos((3.141592653589793 * (f64) temporaryFloat0) / 180.0));
                PSVECNormalize(&localValue5, &localValue5);
                lbl_1_bss_0.playerDirection[parameter0] = localValue5;
                lbl_1_bss_0.playerTimer[parameter0] = (s32) (frandmod(60) + 30);
            }
        }
    }
    switch (lbl_1_bss_0.playerAIState[parameter0]) {
    case 0:
        break;
    case 1:
        localValue1 = lbl_1_bss_0.playerDirection[parameter0];
        lbl_1_bss_0.cpuStickX = 28.0f * localValue1.x;
        lbl_1_bss_0.cpuStickZ = 28.0f * localValue1.z;
        break;
    }
    lbl_1_bss_0.playerTimer[parameter0]--;
}

/* Called by fn_1_535C for a group-1 CPU player to choose grid movement and write its pad input. */
void fn_1_597C(s32 parameter0)
{
    Point3d pos;
    Point3d direction;
    MGPLAYER *temporary3;
    int value4;
    int value3;
    int value6;
    int value9;
    int value8;

    temporary3 = lbl_1_bss_0.players[parameter0];
    pos = temporary3->actor->pos;
    value9 = (s32) ((400.0f + pos.x) / 100.0f);
    value8 = (s32) ((400.0f + pos.z) / 100.0f);
    if (value9 < 0) {
        value9 = 0;
    }
    if (value9 >= 8) {
        value9 = 7;
    }
    if (value8 < 0) {
        value8 = 0;
    }
    if (value8 >= 8) {
        value8 = 7;
    }
    fn_1_50A0(parameter0);
    switch (lbl_1_bss_0.playerAIState[parameter0]) {
    case 0:
        if ((s32) lbl_1_bss_0.gridA[value9 + (value8 * 8)] >= (s32) lbl_1_data_198[GwPlayerConf[parameter0].comDif]) {
            lbl_1_bss_0.playerAIState[parameter0] = 2;
            fn_1_5D34(parameter0);
        }
        break;
    case 2:
        value4 = 0;
        value3 = 0;
        if ((s32) lbl_1_bss_0.gridA[value9 + (value8 * 8)] >= (s32) lbl_1_data_198[GwPlayerConf[parameter0].comDif]) {
            fn_1_5D34(parameter0);
            goto block_28;
        }
        value6 = 0;
        if ((s32) (value9 - 1) >= 0) {
            value6 += lbl_1_bss_0.gridA[(value9 - 1) + (value8 * 8)];
        }
        if ((s32) (value9 + 1) < 8) {
            value6 += lbl_1_bss_0.gridA[(value9 + 1) + (value8 * 8)];
        }
        if ((s32) (value8 - 1) >= 0) {
            value6 += lbl_1_bss_0.gridA[value9 + ((value8 - 1) * 8)];
        }
        if ((s32) (value8 + 1) < 8) {
            value6 += lbl_1_bss_0.gridA[value9 + ((value8 + 1) * 8)];
        }
        if (value6 < (s32) lbl_1_data_198[GwPlayerConf[parameter0].comDif]) {
            lbl_1_bss_0.playerAIState[parameter0] = 0;
            return;
        }
block_28:
        direction = lbl_1_bss_0.playerDirection[parameter0];
        if ((s32) lbl_1_bss_0.gridA[value9 + (value8 * 8)] >= 144) {
            value4 = 256;
            value3 = 256;
        }
        MgPlayerPadSet(temporary3, (s32) (56.0f * direction.x), (s32) (56.0f * -direction.z), value4, value3);
        break;
    case 1:
        break;
    }
}

/* Called by fn_1_597C when the risk threshold is reached to update that player's grid direction. */
void fn_1_5D34(s32 parameter0)
{
    Point3d pos;
    Point3d direction;
    s32 localValue1;
    s32 localValue0;
    f32 floatValue0;
    s32 value9;
    s32 value8;
    MGPLAYER *player;

    player = lbl_1_bss_0.players[parameter0];
    pos = player->actor->pos;
    value9 = (s32) ((400.0f + pos.x) / 100.0f);
    value8 = (s32) ((400.0f + pos.z) / 100.0f);
    if (value9 < 0) {
        value9 = 0;
    }
    if (value9 >= 8) {
        value9 = 7;
    }
    if (value8 < 0) {
        value8 = 0;
    }
    if (value8 >= 8) {
        value8 = 7;
    }
    switch (GwPlayerConf[parameter0].comDif) {
    case 0:
        floatValue0 = 128.0f;
        break;
    case 1:
        floatValue0 = 80.0f;
        break;
    case 2:
        floatValue0 = 40.0f;
        break;
    case 3:
        floatValue0 = 16.0f;
        break;
    }
    if ((fn_1_604C(parameter0, value9, value8, &localValue1, &localValue0) != 0) && ((value9 != localValue1) || (value8 != localValue0)) && ((f32) lbl_1_bss_0.gridA[value9 + value8 * 8] >= floatValue0)) {
        direction.x = (f32) (localValue1 - value9);
        direction.y = 0.0f;
        direction.z = (f32) (localValue0 - value8);
        PSVECNormalize(&direction, &direction);
        lbl_1_bss_0.playerDirection[parameter0] = direction;
    }
}

/* Comparator passed to qsort in fn_1_604C; orders candidates by their distance-weighted cost. */
int fn_1_5FE4(const void *a, const void *b)
{
    const M632GridCandidate *parameter0 = a;
    const M632GridCandidate *parameter1 = b;
    if ((parameter0->distance == parameter1->distance) && (parameter0->cost == parameter1->cost)) {
        return 0;
    }
    if (parameter0->cost < parameter1->cost) {
        return -1;
    }
    return 1;
}

/* Called by fn_1_5D34 to rank the 8-by-8 cells, compare candidates with active group-1 player positions, and report whether selection changed. */
s32 fn_1_604C(s32 parameter0, s32 parameter1, s32 parameter2, s32 *parameter3, s32 *parameter4)
{
    M632GridCandidate localValue4[64];
    Point3d localValue3;
    Point3d localValue1;
    M632GridCandidate *value9;
    f32 floatValue0;
    s32 value0;
    s32 value2;
    s32 value1;
    s32 value5;
    s32 value7;
    s32 value6;
    s32 value8;

    value9 = &localValue4[0];
    if (parameter1 < 0) {
        parameter1 = 0;
    }
    if (parameter1 >= 8) {
        parameter1 = 7;
    }
    if (parameter2 < 0) {
        parameter2 = 0;
    }
    if (parameter2 >= 8) {
        parameter2 = 7;
    }
    value6 = 0;
    while (value6 < 8) {
        value7 = 0;
        while (value7 < 8) {
            localValue1.x = (f32) (value7 - parameter1);
            localValue1.y = 0.0f;
            localValue1.z = (f32) (value6 - parameter2);
            value9->x = value7;
            value9->z = value6;
            value9->distance = PSVECMag(&localValue1);
            floatValue0 = value9->distance;
            if (floatValue0 < 1.0f) {
                floatValue0 = 1.0f;
            }
            value9->cost = (s32) ((f32) lbl_1_bss_0.gridA[value7 + value6 * 8] * floatValue0);
            value9 += 1;
            value7 += 1;
        }
        value6 += 1;
    }
    fn_1_6574(&localValue4[0], 64U, sizeof(localValue4[0]), fn_1_5FE4);
    value9 = &localValue4[0];
    value1 = 0;
    value2 = 0;
    value8 = 0;
    while (value8 < 4) {
        if ((lbl_1_bss_0.group[value8] == 1) && (lbl_1_bss_0.playerState[value8] == 0)) {
            value2 += 1;
        }
        value8 += 1;
    }
    value8 = 0;
    while (value8 < 64) {
        if (value2 > 1) {
            value5 = 0;
            while (value5 < 4) {
                if ((lbl_1_bss_0.group[value5] == 1) && (lbl_1_bss_0.playerState[value5] == 0) && (value5 != parameter0)) {
                    localValue3 = lbl_1_bss_0.players[value5]->actor->pos;
                    value7 = (s32) ((400.0f + localValue3.x) / 100.0f);
                    value6 = (s32) ((400.0f + localValue3.z) / 100.0f);
                    if ((value7 != value9->x) || (value6 != value9->z)) {
                        value1 = 1;
                        break;
                    }
                }
                value5 += 1;
            }
        } else {
            value1 = 1;
        }
        if (value1 != 0) {
            *parameter3 = value9->x;
            *parameter4 = value9->z;
            break;
        }
        value9 += 1;
        value8 += 1;
    }
    value0 = 1;
    if ((*parameter3 == parameter1) && (*parameter4 == parameter2)) {
        value0 = 0;
    }
    return value0;
}
