#include "REL/m635dll.h"
#include "game/charman.h"
#include "game/gamework.h"

void fn_1_2954(void)
{
    int i;
    s16 charNo;
    int member;
    int motion;
    s16 player;
    s16 memberCountA;
    s16 memberCountB;
    s16 model;
    s16 lookupCountA;
    s16 lookupCountB;
    s16 team;
    s16 motionId;
    s16 motionTime;

    lookupCountA = 0;
    lookupCountB = 0;
    memberCountA = 0;
    memberCountB = 0;
    for (i = 0; i < 4; i++) {
        lbl_1_bss_BC[i].playerNo = i;
        charNo = GwPlayerConf[i].charNo;
        lbl_1_bss_BC[i].charNo = charNo;
        lbl_1_bss_BC[i].padNo = GwPlayerConf[i].padNo;
        team = GwPlayerConf[i].grpNo;
        lbl_1_bss_BC[i].teamNo = team;
        if (team == 0) {
            lbl_1_bss_BC[i].memberNo = memberCountA;
            memberCountA++;
        } else {
            lbl_1_bss_BC[i].memberNo = memberCountB;
            memberCountB++;
        }
        lbl_1_bss_BC[i].difficulty = GwPlayerConf[i].comDif;
        lbl_1_bss_BC[i].comF = GwPlayerConf[i].type;
        lbl_1_bss_BC[i].timer = fn_1_1308(GwPlayerConf[i].comDif);
        if (team == 0) {
            lbl_1_bss_B4[0][lookupCountA++] = i;
        } else {
            lbl_1_bss_B4[1][lookupCountB++] = i;
        }
    }
    for (i = 0; i < 2; i++) {
        for (member = 0; member < 2; member++) {
            player = lbl_1_bss_B4[i][member];
            charNo = lbl_1_bss_BC[player].charNo;
            model = CharModelCreate(charNo, 2);
            Hu3DModelPosSetV(model, &lbl_1_data_F8[member + i * 2]);
            lbl_1_bss_BC[player].z = lbl_1_data_F8[member + i * 2].z;
            lbl_1_bss_BC[player].model = model;
            for (motion = 0; motion < 6; motion++) {
                motionId = CharMotionCreate(charNo, lbl_1_data_158[motion].dataNum);
                lbl_1_bss_BC[player].motions[motion] = motionId;
            }
            Hu3DModelShadowSet(model);
            CharMotionDataClose(charNo);
            CharMotionSet(charNo, lbl_1_bss_BC[player].motions[0]);
            motionTime = CharMotionMaxTimeGet(charNo);
            CharMotionTimeSet(charNo, motionTime);
        }
    }
}

void fn_1_2CE8(void)
{
    int member;
    int team;
    s16 state;
    s16 player;
    s16 model;
    float z;

    for (team = 0; team < 2; team++) {
        for (member = 0; member < 2; member++) {
            player = lbl_1_bss_B4[team][member];
            state = lbl_1_bss_BC[player].state;
            if (state != 0) {
                model = lbl_1_bss_BC[player].model;
                z = lbl_1_bss_BC[player].z;
                z += (lbl_1_data_2E8[state].z[member] - lbl_1_data_2E8[state - 1].z[member])
                    / lbl_1_data_2E8[state].frames;
                if (z <= lbl_1_data_2E8[state].z[member]) {
                    z = lbl_1_data_2E8[state].z[member];
                } else {
                    lbl_1_bss_BC[player].z = z;
                    Hu3DModelPosSet(model,
                        lbl_1_data_F8[member + team * 2].x,
                        lbl_1_data_F8[member + team * 2].y,
                        lbl_1_bss_BC[player].z);
                }
            }
        }
    }
}
