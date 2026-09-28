#define _MATH_H
#include "game/object.h"
#include "game/process.h"
#include "game/armem.h"
#include "dolphin/os.h"
#include "REL/mdminidll/player_config.h"

extern s16 lbl_1_bss_2C;
extern char lbl_1_data_798[];
extern char lbl_1_data_6F3[];
extern char lbl_1_data_704[];
extern char lbl_1_data_716[];
extern char lbl_1_data_728[];
extern char lbl_1_data_736;

void fn_1_F10(void)
{
    OMOVLHIS *var_r31;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_2C == 0);

    var_r31 = omOvlHisGet(0);
    omOvlHisChg(0, var_r31->ovl, 1, 0);
    OSReport(lbl_1_data_798);
    OSReport(lbl_1_data_6F3, 33);
    OSReport(lbl_1_data_704, 36);
    OSReport(lbl_1_data_716, 155);
    OSReport(lbl_1_data_728, 242);
    HuAMemDump();
    OSReport(&lbl_1_data_736);

    switch (lbl_1_bss_804[1]) {
    case 0:
        omOvlCallEx(DLL_mgmfreedll, 1, 0, 0);
        break;
    case 1:
        omOvlCallEx(DLL_mgmbattledll, 1, 0, 0);
        break;
    case 2:
        omOvlCallEx(DLL_mgmbingodll, 1, 0, 0);
        break;
    case 3:
        omOvlCallEx(DLL_mgmtournamentdll, 1, 0, 0);
        break;
    case 4:
        omOvlCallEx(DLL_mgmdecathlondll, 1, 0, 0);
        break;
    case 5:
        omOvlCallEx(DLL_mgmrenshodll, 1, 0, 0);
        break;
    }
}
