#include "REL/m635dll.h"

void fn_1_2F40(s16 player)
{
    if (lbl_1_bss_BC[player].state > 8) {
        lbl_1_bss_BC[player].state++;
    } else if (lbl_1_bss_BC[player].state == 8) {
        lbl_1_bss_BC[player].state = 10;
    } else if (lbl_1_bss_BC[player].state == 0) {
        lbl_1_bss_BC[player].state = 1;
    } else if (lbl_1_bss_BC[player].state == 1) {
        if (lbl_1_bss_BC[player].memberNo == 0) {
            lbl_1_bss_BC[player].state = 3;
        } else {
            lbl_1_bss_BC[player].state = 2;
        }
    } else if (lbl_1_bss_BC[player].state < 8) {
        lbl_1_bss_BC[player].state += 2;
    }
}
