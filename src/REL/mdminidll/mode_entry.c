#define _MATH_H
#include "dolphin/os.h"
#include "game/armem.h"
#include "game/gamework.h"
#include "game/object.h"
#include "game/mgdata.h"
#include "game/flag.h"
extern s16 lbl_1_bss_28;
extern s16 lbl_1_bss_2A;
extern s16 lbl_1_data_6B6[6];
s16 fn_1_1A4(s16 mode);

extern s16 lbl_1_bss_0;
extern char lbl_1_data_960[];
extern char lbl_1_data_738[];
extern char lbl_1_data_6F3[];
extern char lbl_1_data_704[];
extern char lbl_1_data_716[];
extern char lbl_1_data_728[];
extern char lbl_1_data_736[];
void fn_1_16400(void);

void fn_1_16F28(void)
{
    OSReport(lbl_1_data_960);
    HuARDirFree(10420224U);
    HuARDirFree(10223616U);
    HuARDirFree(10289152U);
    HuARDirFree(10551296U);
    HuARDirFree(10354688U);
    OSReport(lbl_1_data_738);
    OSReport(lbl_1_data_6F3, 33);
    OSReport(lbl_1_data_704, 36);
    OSReport(lbl_1_data_716, 155);
    OSReport(lbl_1_data_728, 242);
    HuAMemDump();
    OSReport(lbl_1_data_736);
    MgPauseExitF = 0;
    lbl_1_bss_28 = 0;
    lbl_1_bss_2A = 0;
    if (GWBankFlagGet(2) != 0) lbl_1_bss_28 = 1;
    if (GWBankFlagGet(3) != 0) lbl_1_bss_2A = 1;
    lbl_1_data_6B6[0] = 1;
    lbl_1_data_6B6[1] = 1;
    lbl_1_data_6B6[2] = 1;
    lbl_1_data_6B6[3] = 1;
    lbl_1_data_6B6[4] = 1;
    lbl_1_data_6B6[5] = 0;
    if (GWBankFlagGet(7) != 0) lbl_1_data_6B6[5] = 1;
    lbl_1_data_6B6[1] = fn_1_1A4(1);
    lbl_1_data_6B6[2] = fn_1_1A4(2);
    lbl_1_data_6B6[3] = fn_1_1A4(3);
    lbl_1_data_6B6[4] = fn_1_1A4(4);
    if (lbl_1_data_6B6[5] == 1) lbl_1_data_6B6[5] = fn_1_1A4(5);
    lbl_1_bss_0 = (s16)omovlevtno;
    if (lbl_1_bss_0 == 0) GWMgPlayNumSet(0U);
    MgInstExitF = 0;
    MgPauseExitF = 0;
    MgExitReq = 0;
    _SetFlag(196609U);
    _ClearFlag(196610U);
    _ClearFlag(196611U);
    _ClearFlag(196612U);
    fn_1_16400();
}
