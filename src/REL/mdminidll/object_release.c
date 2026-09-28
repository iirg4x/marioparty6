#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "game/process.h"
#include "game/armem.h"
#include "game/audio.h"
#include "game/wipe.h"
#include "game/window.h"
#include "game/sprite.h"
#include "game/flag.h"
#include "game/gamework.h"
#include "game/saveload.h"
#include "game/charman.h"
#include "dolphin/os.h"
#include "REL/mdminidll/player_config.h"

extern OMOBJMAN *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern s16 lbl_1_bss_2C;
extern s16 lbl_1_bss_752[];
extern s16 lbl_1_bss_858[];
extern s32 lbl_1_data_94;
extern char lbl_1_data_798[];
extern char lbl_1_data_767[];
extern char lbl_1_data_6F3[];
extern char lbl_1_data_704[];
extern char lbl_1_data_716[];
extern char lbl_1_data_728[];
extern char lbl_1_data_736;

/* The callee ignores the word passed in r3; no semantic argument name is known. */
s16 fn_1_1E86C(s32);

void fn_1_5EC0(OMOBJ *obj)
{
    if (obj) {
        Hu3DModelHookReset(obj->mdlId[1]);
        Hu3DMotionKill(obj->mtnId[0]);
        Hu3DMotionKill(obj->mtnId[1]);
        Hu3DMotionKill(obj->mtnId[2]);
        Hu3DMotionKill(obj->mtnId[3]);
        Hu3DMotionKill(obj->mtnId[4]);
        Hu3DModelKill(obj->mdlId[0]);
        Hu3DModelKill(obj->mdlId[1]);
        omDelObjEx(lbl_1_bss_4, obj);
    }
    obj->objFunc = NULL;
}

#pragma section code_type ".text.secondary_object_release"
void fn_1_71E8(OMOBJ *obj)
{
    s16 index;
    if (obj) {
        obj->objFunc = NULL;
        index = 0;
        while (index < 3) {
            Hu3DMotionKill(obj->mtnId[index]);
            Hu3DModelKill(obj->mdlId[index]);
            index += 1;
        }
        omDelObjEx(lbl_1_bss_4, obj);
    }
    obj = NULL;
}

#pragma section code_type ".text.object_animation_pair_release"
void fn_1_7E90(OMOBJ *obj)
{
    s16 index;
    if (obj) {
        index = 0;
        while (index < 2) {
            Hu3DAnimKill(obj->mtnId[index * 2]);
            Hu3DAnimKill(obj->mtnId[index * 2 + 1]);
            obj->mtnId[index * 2] = -1;
            obj->mtnId[index * 2 + 1] = -1;
            index += 1;
        }
        index = 1;
        while (index >= 0) {
            Hu3DModelKill(obj->mdlId[index]);
            index -= 1;
        }
        omDelObjEx(lbl_1_bss_4, obj);
    }
    obj = NULL;
}

#pragma section code_type ".text.scene_exit"
void fn_1_15E0C(void)
{
    s16 var_r26;
    s16 var_r25;
    s16 var_r24;
    OMOVLHIS *var_r23;
    HUSPR_GROUP *var_r22;
    s16 var_r21;

    var_r24 = 0;
    var_r24 = fn_1_1E86C(0);
    if (lbl_1_data_94 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_94, 1000);
        lbl_1_data_94 = -1;
    }
    switch (var_r24) {
    case 0:
    case 1:
        WipeCreate(2, 0, 60);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        fn_1_71E8(lbl_1_bss_8);
        fn_1_7E90(lbl_1_bss_C);
        fn_1_5EC0(lbl_1_bss_10);
        HuPrcSleep(5);
        var_r26 = 0;
        while (var_r26 < 4) {
            HuWinDispOff(lbl_1_bss_858[var_r26]);
            var_r26 += 1;
        }
        HuSprPriSet(lbl_1_bss_752[8], 0, 5500);
        var_r21 = lbl_1_bss_752[8];
        var_r22 = &HuSprGrpData[var_r21];
        var_r25 = 0;
        while (var_r25 < var_r22->sprNum) {
            HuSprAttrReset(var_r21, var_r25, 4);
            var_r25 += 1;
        }
        HuPrcSleep(5);
        _ClearFlag(196609U);
        _ClearFlag(196610U);
        _ClearFlag(196611U);
        _ClearFlag(196612U);
        GWBankStarAdd((u16) (GWMgPlayNumGet() / 10));
        SLSaveModeExec(0);
        break;
    case 2:
        WipeCreate(2, 0, 60);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        break;
    }
    if (var_r24 == 2) {
        do {
            HuPrcVSleep();
        } while (lbl_1_bss_2C == 0);
        var_r23 = omOvlHisGet(0);
        omOvlHisChg(0, var_r23->ovl, 1, 0);
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
    } else {
        CharDataClose(-1);
        HuARDirFree(10420224U);
        HuARDirFree(10223616U);
        HuARDirFree(10289152U);
        HuARDirFree(10551296U);
        HuARDirFree(10354688U);
        OSReport(lbl_1_data_767);
        OSReport(lbl_1_data_6F3, 33);
        OSReport(lbl_1_data_704, 36);
        OSReport(lbl_1_data_716, 155);
        OSReport(lbl_1_data_728, 242);
        HuAMemDump();
        OSReport(&lbl_1_data_736);
        omOvlReturnEx(1, 1);
    }
    HuPrcEnd();
    while (1) {
        HuPrcVSleep();
    }
}
