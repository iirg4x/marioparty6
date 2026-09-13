#include "REL/m635dll.h"

extern HuVecF lbl_1_data_128[4];

s16 fn_1_32E0(s16 team, s16 member)
{
    if (team < 0 || team >= 2 || member < 0 || member >= 2) {
        return 0;
    }
    return lbl_1_bss_B4[team][member];
}

void fn_1_3340(void)
{
    int team;
    int member;
    s16 player;
    s16 model;

    for (team = 0; team < 2; team++) {
        for (member = 0; member < 2; member++) {
            player = lbl_1_bss_B4[team][member];
            model = lbl_1_bss_BC[player].model;
            Hu3DModelPosSetV(model, &lbl_1_data_128[member + team * 2]);
        }
    }
}

void fn_1_33F8(void)
{
}
