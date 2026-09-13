#include "REL/m670dll.h"
#include "game/frand.h"
#include "game/gamework.h"
#include "game/pad.h"

void fn_1_3B30(void)
{
    int i;
    M670CPU *cpu;

    for (i = 0; i < 4; i++) {
        if (GwPlayerConf[i].type == 1) {
            cpu = &lbl_1_bss_10.cpu[i];
            if (lbl_1_bss_10.group[i] == 0) {
                cpu->state = 1;
            } else {
                cpu->state = 4;
            }
            cpu->playerNo = i;
            cpu->player = lbl_1_bss_10.players[i];
        }
    }
}

void fn_1_3BDC(void)
{
    int i, j;
    M670CPU *cpu;
    int choice, randomValue, selected;
    s8 *pattern;

    for (i = 0; i < 4; i++) {
        if (GwPlayerConf[i].type == 1) {
            cpu = &lbl_1_bss_10.cpu[i];
            switch (cpu->state) {
            case 0:
            case 5:
            case 7:
                break;
            case 1:
                if ((lbl_1_bss_10.state == 0 || lbl_1_bss_10.state == 2)
                    && lbl_1_bss_10.playerState[lbl_1_bss_10.soloPlayer] == 1) {
                    switch (GwPlayerConf[cpu->playerNo].comDif) {
                    case 0: cpu->timer = frandmod(180) + 60; break;
                    case 1: cpu->timer = frandmod(120) + 60; break;
                    case 2: cpu->timer = frandmod(60) + 60; break;
                    case 3: cpu->timer = 0; break;
                    }
                    cpu->timer = frandmod(60) + 60;
                    cpu->state = 2;
                }
                break;
            case 2:
                if (--cpu->timer == 0) {
                    if (lbl_1_bss_10.group[cpu->playerNo] == 0) {
                        cpu->state = 3;
                    } else {
                        cpu->state = 4;
                    }
                }
                if (lbl_1_bss_10.state != 0 && lbl_1_bss_10.state != 2) {
                    cpu->state = 4;
                }
                break;
            case 3:
                lbl_1_bss_10.selectedWord = frandmod(6);
                fn_1_2460(lbl_1_bss_10.soloPlayer, 2);
                cpu->state = 1;
                break;
            case 4:
            {
                HuVecF positions[4];
                HuVecF pos;
                int indices[4];
                choice = 0;
                if (lbl_1_bss_10.state == 3) {
                    choice = 1;
                } else {
                    randomValue = frandmod(100);
                    pos = cpu->player->actor->pos;
                    pos.y = 0.0f;
                    if (lbl_1_bss_10.state != 0 && lbl_1_bss_10.state != 2) {
                        switch (GwPlayerConf[cpu->playerNo].comDif) {
                        case 0: choice = randomValue < 80 ? 0 : 2; break;
                        case 1: choice = randomValue < 70 ? 0 : 2; break;
                        case 2: choice = randomValue < 60 ? 0 : 2; break;
                        case 3: choice = randomValue < 30 ? 0 : 2; break;
                        }
                        if (fn_1_2F44(&pos)) {
                            choice = 1;
                        }
                    } else {
                        switch (GwPlayerConf[cpu->playerNo].comDif) {
                        case 0: choice = randomValue < 90 ? 1 : 0; break;
                        case 1: choice = randomValue < 90 ? 1 : 0; break;
                        case 2: choice = randomValue < 90 ? 1 : 0; break;
                        case 3: choice = randomValue < 90 ? 1 : 0; break;
                        }
                    }
                }
                if (choice == 0 || choice == 2) {
                    pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
                    if (choice == 0) {
                        selected = frandmod(24);
                    } else {
                        HuVecF dir, origin;
                        int count;
                        float minimum, distance;
                        origin = cpu->player->actor->pos;
                        origin.y = 0.0f;
                        count = 0;
                        for (j = 0; j < 24; j++) {
                            if (*pattern == lbl_1_bss_10.selectedWord) {
                                indices[count] = j;
                                positions[count] = lbl_1_bss_10.pillarPos[j];
                                count++;
                            }
                            pattern++;
                        }
                        minimum = 99999.0f;
                        count = -1;
                        for (j = 0; j < 4; j++) {
                            PSVECSubtract(&positions[j], &origin, &dir);
                            distance = PSVECMag(&dir);
                            if (distance < minimum) {
                                count = j;
                                minimum = distance;
                            }
                        }
                        selected = indices[count];
                    }
                    cpu->pillarNo = selected;
                    cpu->targetPos = lbl_1_bss_10.pillarPos[selected];
                    cpu->targetPos.y = 0.0f;
                    cpu->timer = 0;
                    cpu->buttonTimer = 0;
                    cpu->state = 6;
                } else {
                    cpu->timer = frandmod(60) + 30;
                    cpu->state = 2;
                }
                break;
            }
            case 6:
            {
                HuVecF pos, dir;
                int pillarNo;
                float distance, height;
                pos = cpu->player->actor->pos;
                pos.y = dir.y = 0.0f;
                PSVECSubtract(&cpu->targetPos, &pos, &dir);
                distance = PSVECMag(&dir);
                pillarNo = fn_1_2DE8(&pos);
                height = fn_1_2EF4(pillarNo);
                if (100.0f != height) {
                    cpu->buttonTimer = 30;
                }
                cpu->timer++;
                if (cpu->timer >= 120) {
                    cpu->state = 4;
                } else if (distance < 40.0f) {
                    cpu->state = 4;
                } else {
                    fn_1_1460(&dir, &dir);
                    MgPlayerPadSet(cpu->player, (int)(56.0f * dir.x),
                        (int)(-56.0f * dir.z),
                        cpu->buttonTimer == 30 ? PAD_BUTTON_A : 0,
                        cpu->buttonTimer != 0 ? PAD_BUTTON_A : 0);
                }
                if (cpu->buttonTimer != 0) {
                    cpu->buttonTimer = 0;
                }
                break;
            }
            }
        }
    }
}
