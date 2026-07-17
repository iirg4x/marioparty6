#include <string.h>

#include "datadir_enum.h"

#include "dolphin/os.h"

#include "REL/mdpartyDll.h"

#include "game/armem.h"
#include "game/board/main.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/printfunc.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"

#define sind(x) sin((M_PI * (x)) / 180.0)
#define cosd(x) cos((M_PI * (x)) / 180.0)

typedef void (*VoidFunc)(void);

typedef struct MdCameraWork_s MDCAMERA_WORK;
typedef void (*MDCAMERA_CALLBACK)(OMOBJ *obj, MDCAMERA_WORK *camera);

struct MdCameraWork_s {
    OMOBJ *obj;
    HuVecF center;
    HuVecF unk_10;
    HuVecF rot;
    HuVecF unk_28;
    float zoom;
    float unk_38;
    MDCAMERA_CALLBACK callback;
    float unk_40;
    u8 unk_44[0xC];
};

typedef struct MdSprInfo_s {
    s16 groupNo;
    s16 memberNo;
    s16 animNo;
    s16 priority;
    s16 bank;
    HuVec2f pos;
    HuVec2f scale;
    float zRot;
} MDSPR_INFO;

typedef struct Lbl1DataE12Entry {
    s16 unk_0[8];
    s16 unk_10;
} LBL_1_DATA_E12_ENTRY;

typedef struct Lbl1Bss288Entry {
    HU3D_MODELID modelId;
    HU3D_ANIMID animId[4];
    s16 unk_A;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
    s16 unk_30;
    u8 unk_32[6];
} LBL_1_BSS_288_ENTRY;

typedef struct Lbl1Bss1C {
    OMOBJ *obj;
    u8 unk_4[4];
} LBL_1_BSS_1C;

typedef struct Lbl1Bss228Entry {
    s16 state;
    s16 pad;
    HuVecF pos;
    HuVecF unk_10;
    HuVecF unk_1C;
    float time;
    float duration;
} LBL_1_BSS_228_ENTRY;

typedef struct Lbl1Bss1C8Entry {
    s16 state;
    s16 pad;
    HuVecF pos;
    HuVecF unk_10;
    HuVecF unk_1C;
    float time;
    float duration;
} LBL_1_BSS_1C8_ENTRY;

typedef struct Lbl1Bss9BCEntry {
    s16 unk_0;
    s16 unk_2;
    s16 unk_4;
    s16 unk_6;
    s16 chrSel;
    s16 unk_A;
    s16 unk_C;
} LBL_1_BSS_9BC_ENTRY;

typedef struct Lbl1Bss9F4 {
    s16 unk_0;
    s16 unk_2;
    s16 unk_4;
    s16 unk_6;
    s16 unk_8;
    s16 unk_A;
    s16 unk_C;
    s16 unk_E;
    s32 unk_10[2];
    s16 unk_18;
    s16 unk_1A;
} LBL_1_BSS_9F4;

#include "REL/mdpartyDll_globals.h"

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern int HuAudFXPlay(int seId);
extern int HuAudFXPlayPan(int seId, s16 pan);
extern void HuAudFXStop(int seNo);
extern int HuAudSStreamPlay(s16 streamId);


void fn_1_0(HUWINID winId, u32 mess, s16 index);
void fn_1_1B4(void);
void fn_1_2B0(void);
void fn_1_400(void);
void fn_1_64C(void);
void fn_1_718(void);
void fn_1_E30(void);
void fn_1_353C(void);
void fn_1_6284(OMOBJ *obj);
void fn_1_72C4(OMOBJ *obj);
void fn_1_109C(s16 arg0);
void fn_1_1248(void);
void fn_1_12F8(void);
void fn_1_2D1D4(void);
void fn_1_2DD38(void);
void fn_1_2DFD4(OMOBJ *obj, MDCAMERA_WORK *camera);
void fn_1_2E338(OMOBJ *obj, MDCAMERA_WORK *camera);
void fn_1_2E560(OMOBJ *obj, MDCAMERA_WORK *camera);
s32 fn_1_2E5D4(void);
s16 fn_1_2F22C(void);
s16 fn_1_30B4C(void);
s16 fn_1_31AC0(s16 arg0);
s16 fn_1_32904(void);
s16 fn_1_329A4(s16 arg0);
s16 fn_1_335F0(s16 *arg0, s16 *arg1, s16 *arg2);
s16 fn_1_33724(s16 arg0);
s16 fn_1_34FC4(s16 *arg0, s16 *arg1, s16 *arg2);
s16 fn_1_350F8(s16 arg0);
s16 fn_1_36518(s16 arg0);
void fn_1_37260(s16 *arg0, s16 *arg1, s16 *arg2);
s16 fn_1_3736C(s16 arg0);
s16 fn_1_38A3C(s16 arg0);
s16 fn_1_39C60(s16 arg0);
s16 fn_1_3AD84(s16 arg0);
s16 fn_1_3CFE4(s16 arg0);
s16 fn_1_3EAC8(s16 arg0);
void ObjectSetup(void);
void fn_1_32D0(OMOBJ *obj);
void fn_1_467C(void);
void fn_1_4A84(OMOBJ *obj);
void fn_1_4EDC(OMOBJ *obj);
void fn_1_50EC(OMOBJ *obj);
void fn_1_5208(OMOBJ *obj);
void fn_1_5324(OMOBJ *obj);
void fn_1_5A18(OMOBJ *obj);
void fn_1_6324(OMOBJ *obj);
void fn_1_6A20(OMOBJ *obj);
void fn_1_737C(OMOBJ *obj);
void fn_1_605C(OMOBJ *obj);
void fn_1_7064(OMOBJ *obj);
void fn_1_9658(s16 winNo, u32 messNum);
void fn_1_A8D8(s16 memberNo, HuVecF *pos, float xOffset, float yOffset);
void fn_1_A970(s16 memberNo, HuVecF *pos, float xOffset, float yOffset);
void fn_1_AA94(s16 memberNo);
void fn_1_AAF0(s16 memberNo);
void fn_1_AD14(s16 modelNo, HuVecF *pos);
void fn_1_7ED4(OMOBJ *obj);
void fn_1_861C(OMOBJ *obj);
void fn_1_9218(OMOBJ *obj);
void fn_1_9F50(OMOBJ *obj);
void fn_1_A5D8(OMOBJ *obj);
void fn_1_B008(OMOBJ *obj);
void fn_1_B4C8(OMOBJ *obj);
s16 fn_1_B748(s16 arg0, s16 arg1, s16 arg2, s16 arg3,
    LBL_1_DATA_E12_ENTRY *arg4, s16 arg5);
s16 fn_1_BBD8(
    s16 arg0, s16 *arg1, LBL_1_DATA_E12_ENTRY *arg2, s16 arg3);
void fn_1_C28C(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);
void fn_1_C698(s16 modelNo, s16 time, s16 duration);
void fn_1_CA68(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);
void fn_1_D058(s16 modelNo, s16 dataNo, s16 dataOffset, s16 time,
    s16 duration, s16 direction);
void fn_1_D748(void);
void fn_1_10518(OMOBJ *obj);
void fn_1_114C4(OMOBJ *obj);
void fn_1_114F4(OMOBJ *obj);
void fn_1_1158C(OMOBJ *obj);
void fn_1_11EAC(OMOBJ *obj);
void fn_1_12364(OMOBJ *obj);
void fn_1_12FD8(OMOBJ *obj);
void fn_1_12B18(OMOBJ *obj);
void fn_1_12D7C(OMOBJ *obj);
void fn_1_12EC8(OMOBJ *obj);
void fn_1_13EC0(OMOBJ *obj);
void fn_1_14344(OMOBJ *obj);
void fn_1_14510(OMOBJ *obj);
void fn_1_14CC0(OMOBJ *obj);
void fn_1_14F64(OMOBJ *obj);
void fn_1_150D0(OMOBJ *obj);
void fn_1_156B4(OMOBJ *obj);
void fn_1_160A0(OMOBJ *obj);
void fn_1_16A8C(OMOBJ *obj);
void fn_1_16E84(OMOBJ *obj);
void fn_1_1733C(s16 arg0);
void fn_1_17730(OMOBJ *obj);
void fn_1_17E10(OMOBJ *obj);
void fn_1_1807C(OMOBJ *obj);
void fn_1_18384(OMOBJ *obj);
void fn_1_18AA4(OMOBJ *obj);
void fn_1_1948C(OMOBJ *obj);
void fn_1_19860(OMOBJ *obj);
void fn_1_19D24(OMOBJ *obj);
void fn_1_1A20C(OMOBJ *obj);
void fn_1_1A8B0(OMOBJ *obj);
void fn_1_1AE60(OMOBJ *obj);
void fn_1_1B400(OMOBJ *obj);
void fn_1_1B9E8(OMOBJ *obj);
void fn_1_1D758(OMOBJ *obj);
void fn_1_1E7F8(OMOBJ *obj);
void fn_1_1F8B8(OMOBJ *obj);
void fn_1_21628(OMOBJ *obj);
void fn_1_219C8(OMOBJ *obj);
void fn_1_21C04(OMOBJ *obj);
void fn_1_21C9C(OMOBJ *obj);
void fn_1_22010(OMOBJ *obj);
void fn_1_221F4(OMOBJ *obj);
void fn_1_227E4(OMOBJ *obj);
void fn_1_228E8(OMOBJ *obj);
void fn_1_22EB8(OMOBJ *obj);
void fn_1_2318C(OMOBJ *obj);
void fn_1_23224(OMOBJ *obj);
void fn_1_232A0(OMOBJ *obj);
void fn_1_239DC(OMOBJ *obj);
void fn_1_23A00(OMOBJ *obj);
void fn_1_23AC0(OMOBJ *obj);
void fn_1_24090(OMOBJ *obj);
void fn_1_2457C(OMOBJ *obj);
void fn_1_26328(OMOBJ *obj);
void fn_1_2851C(OMOBJ *obj);
void fn_1_29598(OMOBJ *obj);
void fn_1_2A8C4(OMOBJ *obj);
void fn_1_2BBD8(OMOBJ *obj);
void fn_1_2C874(void);
void fn_1_2CE00(void);
s16 fn_1_2E638(void);
s16 fn_1_2EBEC(void);
s16 fn_1_2EF24(void);
s16 fn_1_31508(void);
s16 fn_1_38314(void);
s16 fn_1_3BEA8(void);
void fn_1_3CA70(void);
void fn_1_0(HUWINID winId, u32 mess, s16 index)
{
    s32 messNum[3] = { 0xD0000, 0xD0025, -1 };
    s32 fxNum[16] = {
        0x3B5, 0x3B6, 0x3B7, 0x3B8, 0x3B9, 0x3BA, 0x3BB, -1,
        0x3AD, 0x3AE, 0x3AF, 0x3B0, 0x3B1, 0x3B2, 0x3B3, -1,
    };
    s16 i;

    index--;
    OSReport(lbl_1_data_BCC, index);
    if (lbl_1_data_BC8 != mess) {
        lbl_1_data_BC8 = mess;
        for (i = 0;; i++) {
            if (messNum[i] == -1) {
                HuAudFXPlay(fxNum[index]);
                break;
            }
            if (mess == messNum[i]) {
                if (index >= 8) {
                    HuAudFXPlayPan(fxNum[index], 0x50);
                } else {
                    HuAudFXPlayPan(fxNum[index], 0x30);
                }
                break;
            }
        }
    }
}

void fn_1_1B4(void)
{
    lbl_1_bss_2A = FALSE;
    lbl_1_bss_2C = FALSE;
    if (GWBankFlagGet(2)) {
        lbl_1_bss_2A = TRUE;
    }
    if (GWBankFlagGet(3)) {
        lbl_1_bss_2C = TRUE;
    }
    lbl_1_data_BEE[0] = TRUE;
    lbl_1_data_BEE[1] = TRUE;
    lbl_1_data_BEE[2] = TRUE;
    lbl_1_data_BEE[3] = TRUE;
    lbl_1_data_BEE[4] = TRUE;
    lbl_1_data_BEE[5] = FALSE;
    if (GWBankFlagGet(0x33)) {
        lbl_1_data_BEE[5] = TRUE;
    }
}

inline void fn_1_1B4(void);

void fn_1_2B0(void)
{
    s16 i;
    s32 status;

    for (i = 0; i < 4; i++) {
        if ((void *)CharMotionAMemPGet(GwPlayer[i].charNo) == NULL) {
            break;
        }
    }
    if (i != 4) {
        CharDataClose(-1);
        for (i = 0; i < 4; i++) {
            status = HuDataDirReadAsync(
                CharDataDirTbl[GwPlayer[i].charNo][4]);
            if (status != -1) {
                while (!HuDataGetAsyncStat(status)) {
                    HuPrcVSleep();
                }
            }
            CharMotionInit(GwPlayer[i].charNo);
            HuDataDirClose(CharDataDirTbl[GwPlayer[i].charNo][4]);
        }
    }
}

inline void fn_1_2B0(void);

void fn_1_400(void)
{
    s32 dataDir[2] = { DATA_board_us, DATA_capsule };
    s16 i;
    s32 status;

    fn_1_2B0();
    for (i = 0; i < 2; i++) {
        status = HuDataDirReadAsync(dataDir[i]);
        if (status != -1) {
            while (!HuDataGetAsyncStat(status)) {
                HuPrcVSleep();
            }
        }
        HuAR_MRAMtoARAM(dataDir[i]);
        while (HuARDMACheck()) {
            HuPrcVSleep();
        }
        HuDataDirClose(dataDir[i]);
    }
    status = HuDataDirReadAsync(lbl_1_data_BFC[GwSystem.boardNo]);
    if (status != -1) {
        while (!HuDataGetAsyncStat(status)) {
            HuPrcVSleep();
        }
    }
    lbl_1_bss_2E = TRUE;
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

void fn_1_64C(void)
{
    lbl_1_bss_2E = FALSE;
    OSReport(lbl_1_data_C34);
    OSReport(lbl_1_data_C66, 0x21);
    OSReport(lbl_1_data_C77, 0x24);
    OSReport(lbl_1_data_C89, 0x9B);
    OSReport(lbl_1_data_C9B, 0xF2);
    HuAMemDump();
    OSReport(lbl_1_data_CA9);
    HuDataDirClose(DATA_mdparty);
    HuPrcChildCreate(fn_1_400, 0x100, 0x4000, 0, lbl_1_bss_0);
}

inline void fn_1_64C(void);

void fn_1_718(void)
{
    s16 i;

    lbl_1_bss_9F4.unk_A++;
    lbl_1_bss_9F4.unk_A %= 2;
    GwSystem.tagF = lbl_1_bss_9F4.unk_0;
    GwSystem.bonusStarF = lbl_1_bss_9F4.unk_A;
    GwSystem.mgPack = lbl_1_bss_9F4.unk_8;
    GwSystem.turnNo = TRUE;
    GwSystem.turnMax = (lbl_1_bss_9F4.unk_6 * 5) + 10;
    GwSystem.boardNo = lbl_1_bss_9F4.unk_4;
    GwCommon.partyMgPack = GwSystem.mgPack;
    GwCommon.confTag = lbl_1_bss_9F4.unk_0;
    GwCommon.confTurnNum = (lbl_1_bss_9F4.unk_6 * 5) + 10;
    GwCommon.partyMgPack = lbl_1_bss_9F4.unk_8;
    GwCommon.confBonusStar = lbl_1_bss_9F4.unk_A;
    GwCommon.lastBoard = lbl_1_bss_9F4.unk_4;

    for (i = 0; i < 4; i++) {
        GwPlayer[i].comF = lbl_1_bss_9BC[i].unk_4;
        GwPlayer[i].comDif = lbl_1_bss_9BC[i].unk_6;
        GwPlayer[i].charNo = lbl_1_bss_9BC[i].chrSel;
        GwPlayer[i].padNo = lbl_1_bss_9BC[i].unk_A;
        GwPlayer[i].handicap = lbl_1_bss_9BC[i].unk_C;
        GwPlayer[i].team = FALSE;
    }
    if (GwSystem.tagF) {
        for (i = 0; i < 4; i++) {
            GwPlayer[i].handicap = 0;
        }
        GwPlayer[lbl_1_data_4[lbl_1_bss_9F4.unk_E][0]].handicap =
            lbl_1_bss_9BC[0].unk_C;
        GwPlayer[lbl_1_data_4[lbl_1_bss_9F4.unk_E][2]].handicap =
            lbl_1_bss_9BC[1].unk_C;
        GwPlayer[lbl_1_data_4[lbl_1_bss_9F4.unk_E][0]].team = FALSE;
        GwPlayer[lbl_1_data_4[lbl_1_bss_9F4.unk_E][1]].team = FALSE;
        GwPlayer[lbl_1_data_4[lbl_1_bss_9F4.unk_E][2]].team = TRUE;
        GwPlayer[lbl_1_data_4[lbl_1_bss_9F4.unk_E][3]].team = TRUE;
    }
    for (i = 0; i < 4; i++) {
        GwPlayerConf[i].grpNo = FALSE;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].comDif = GwPlayer[i].comDif;
        GwPlayerConf[i].charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo;
        GwPlayerConf[i].grpNo = GwPlayer[i].team;
    }
    _ClearFlag(FLAG_BOARD_TUTORIAL);
    mbSavePartyInit(
        GwSystem.tagF, GwSystem.bonusStarF, GwSystem.mgPack,
        GwSystem.turnMax, GwPlayer[0].handicap, GwPlayer[1].handicap,
        GwPlayer[2].handicap, GwPlayer[3].handicap);
    mbSaveInit(GwSystem.boardNo);
}

void fn_1_E30(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        GwPlayer[i].comF = TRUE;
        GwPlayer[i].comDif = 0;
        GwPlayer[i].charNo = i;
        GwPlayer[i].padNo = i;
        GwPlayer[i].handicap = 0;
        GwPlayer[i].team = FALSE;
    }
    for (i = 0; i < 4; i++) {
        GwPlayerConf[i].grpNo = FALSE;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].comDif = GwPlayer[i].comDif;
        GwPlayerConf[i].charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo;
        GwPlayerConf[i].grpNo = GwPlayer[i].team;
    }
    mbSaveInit(9);
    GwSystem.boardNo = 6;
}

inline void fn_1_E30(void);

void fn_1_109C(s16 arg0)
{
    OMOVL boardDll[6] = {
        DLL_w01dll,
        DLL_w02dll,
        DLL_w03dll,
        DLL_w04dll,
        DLL_w05dll,
        DLL_w06dll,
    };

    do {
        HuPrcVSleep();
    } while (!lbl_1_bss_2E);
    OSReport(lbl_1_data_CAB);
    OSReport(lbl_1_data_C66, 0x21);
    OSReport(lbl_1_data_C77, 0x24);
    OSReport(lbl_1_data_C89, 0x9B);
    OSReport(lbl_1_data_C9B, 0xF2);
    HuAMemDump();
    OSReport(lbl_1_data_CA9);

    switch (arg0) {
        case 0: {
            OMOVLHIS *history = omOvlHisGet(0);

            omOvlHisChg(
                0, boardDll[GwSystem.boardNo], history->evtno,
                history->stat);
            omOvlGotoEx(boardDll[GwSystem.boardNo], TRUE, 0, 0);
            break;
        }
        case 1: {
            OMOVLHIS *history = omOvlHisGet(0);

            omOvlHisChg(0, history->ovl, 1, 0);
            omOvlCallEx(DLL_w10dll, TRUE, 0, 0);
            break;
        }
    }
}

inline void fn_1_109C(s16 arg0);

void fn_1_1248(void)
{
    CharDataClose(-1);
    HuARDirFree(DATA_board);
    HuARDirFree(DATA_board_us);
    HuARDirFree(DATA_capsule);
    OSReport(lbl_1_data_CDC);
    OSReport(lbl_1_data_C66, 0x21);
    OSReport(lbl_1_data_C77, 0x24);
    OSReport(lbl_1_data_C89, 0x9B);
    OSReport(lbl_1_data_C9B, 0xF2);
    HuAMemDump();
    OSReport(lbl_1_data_CA9);
}

void fn_1_12F8(void)
{
    CharDataClose(-1);
    HuARDirFree(DATA_board);
    HuARDirFree(DATA_board_us);
    HuARDirFree(DATA_capsule);
    OSReport(lbl_1_data_D0C);
    OSReport(lbl_1_data_C66, 0x21);
    OSReport(lbl_1_data_C77, 0x24);
    OSReport(lbl_1_data_C89, 0x9B);
    OSReport(lbl_1_data_C9B, 0xF2);
    HuAMemDump();
    OSReport(lbl_1_data_CA9);
}

inline void fn_1_12F8(void);

float fn_1_13A8(float arg0, float arg1, float arg2, float arg3)
{
    float temp = 1.0f - arg3;

    return (arg2 * (arg3 * arg3))
        + ((arg0 * (temp * temp)) + ((arg1 * (temp * arg3)) * 2.0f));
}

void fn_1_1404(
    HuVecF *dst, const HuVecF *a, const HuVecF *b, const HuVecF *c, float t)
{
    dst->x = fn_1_13A8(a->x, b->x, c->x, t);
    dst->y = fn_1_13A8(a->y, b->y, c->y, t);
    dst->z = fn_1_13A8(a->z, b->z, c->z, t);
}

inline void fn_1_1404(
    HuVecF *dst, const HuVecF *a, const HuVecF *b, const HuVecF *c, float t);

float fn_1_160C(float arg0, float arg1, float arg2)
{
    if (arg0 == arg1) {
        return arg1;
    }
    return (arg1 + (arg0 * (arg2 - 1.0f))) / arg2;
}

inline float fn_1_160C(float arg0, float arg1, float arg2);

void fn_1_163C(HuVecF *dst, const HuVecF *src, float weight)
{
    dst->x = fn_1_160C(dst->x, src->x, weight);
    dst->y = fn_1_160C(dst->y, src->y, weight);
    dst->z = fn_1_160C(dst->z, src->z, weight);
}

inline void fn_1_163C(HuVecF *dst, const HuVecF *src, float weight);

float fn_1_1780(float arg0, float arg1, float time, float duration)
{
    if (time <= 0.0f) {
        return arg0;
    }
    if (time >= duration) {
        return arg1;
    }
    return arg0 + ((arg1 - arg0) * sind((90.0f / duration) * time));
}

inline float fn_1_1780(
    float arg0, float arg1, float time, float duration);

float fn_1_1868(float arg0, float arg1, float arg2, float arg3)
{
    if (arg2 <= 0.0f) {
        return arg0;
    }
    if (arg2 >= arg3) {
        return arg1;
    }
    return arg0 + ((arg2 / arg3) * (arg1 - arg0));
}

inline float fn_1_1868(float arg0, float arg1, float arg2, float arg3);

void fn_1_18AC(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration)
{
    HuVecF modelPos;
    HuVecF modelRot;
    HuVecF pos;
    HuVecF rot;

    Hu3DModelPosGet(modelId, &modelPos);
    Hu3DModelRotGet(modelId, &modelRot);
    pos.x = fn_1_1868(start->x, end->x, time, duration);
    pos.y = fn_1_1868(start->y, end->y, time, duration);
    pos.z = fn_1_1868(start->z, end->z, time, duration);
    modelPos.x -= pos.x;
    modelPos.z -= pos.z;
    rot.y = -(180.0 * (atan2(modelPos.x, -modelPos.z) / M_PI));
    if (modelRot.y - rot.y > 180.0f) {
        modelRot.y -= 360.0f;
    } else if (modelRot.y - rot.y < -180.0f) {
        modelRot.y += 360.0f;
    }
    rot.x = modelRot.x;
    rot.y = fn_1_160C(modelRot.y, rot.y, 10.0f);
    rot.z = modelRot.z;
    Hu3DModelPosSetV(modelId, &pos);
    Hu3DModelRotSetV(modelId, &rot);
}

inline void fn_1_18AC(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration);

void fn_1_1C14(HU3D_MODELID modelId, float rotY)
{
    HuVecF modelRot;
    HuVecF rot;

    Hu3DModelRotGet(modelId, &modelRot);
    rot.y = rotY;
    if (modelRot.y - rot.y > 180.0f) {
        modelRot.y -= 360.0f;
    } else if (modelRot.y - rot.y < -180.0f) {
        modelRot.y += 360.0f;
    }
    rot.x = modelRot.x;
    rot.y = fn_1_160C(modelRot.y, rot.y, 10.0f);
    rot.z = modelRot.z;
    Hu3DModelRotSetV(modelId, &rot);
}

inline void fn_1_1C14(HU3D_MODELID modelId, float rotY);

void fn_1_1D54(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY)
{
    if (time <= duration) {
        fn_1_18AC(modelId, start, end, time, duration);
    } else {
        fn_1_1C14(modelId, rotY);
    }
}

inline void fn_1_1D54(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY);

float fn_1_21D4(float arg0, float arg1, float time, float duration)
{
    if (time <= 0.0f || time >= duration) {
        return arg0;
    }
    return arg0 + ((arg1 - arg0) * sind((180.0f / duration) * time));
}

inline float fn_1_21D4(
    float arg0, float arg1, float time, float duration);

void fn_1_22A8(
    HU3D_MODELID modelId, const HuVecF *a, const HuVecF *b, const HuVecF *c,
    float time, float duration)
{
    HuVecF pos;
    HuVecF modelPos;
    HuVecF modelRot;
    float value;

    value = fn_1_1868(0.0f, 1.0f, time, duration);
    fn_1_1404(&pos, a, b, c, value);
    Hu3DModelPosGet(modelId, &modelPos);
    Hu3DModelRotGet(modelId, &modelRot);
    modelPos.x -= pos.x;
    modelPos.z -= pos.z;
    modelPos.y = -(180.0 * (atan2(modelPos.x, -modelPos.z) / M_PI));
    if (modelRot.y - modelPos.y > 180.0f) {
        modelRot.y -= 360.0f;
    } else if (modelRot.y - modelPos.y < -180.0f) {
        modelRot.y += 360.0f;
    }
    modelRot.y = fn_1_160C(modelRot.y, modelPos.y, 10.0f);
    Hu3DModelPosSet(modelId, pos.x, pos.y, pos.z);
    Hu3DModelRotSet(modelId, 0.0f, modelRot.y, 0.0f);
}

inline void fn_1_22A8(
    HU3D_MODELID modelId, const HuVecF *a, const HuVecF *b, const HuVecF *c,
    float time, float duration);

void fn_1_26FC(void)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        entry->unk_0 = 0;
        entry->unk_2 = 0;
        entry->unk_6 = 0;
        entry->chrSel = i;
        entry->unk_A = i;
    }
}

void fn_1_275C(void)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 playerCount = lbl_1_bss_9F4.unk_2;
    s16 i;

    lbl_1_bss_9BC[0].unk_4 = 0;
    playerCount--;
    for (i = 1, entry = &lbl_1_bss_9BC[1]; i < 4; i++, entry++) {
        if (playerCount > 0 && HuPadStatGet(i) == 0) {
            entry->unk_4 = 0;
            playerCount--;
        } else {
            entry->unk_4 = 1;
        }
    }
}

inline void fn_1_275C(void);

void fn_1_2810(MDCAMERA_WORK *camera)
{
    memcpy(&camera->center, &camera->unk_10, sizeof(HuVecF));
    memcpy(&camera->rot, &camera->unk_28, sizeof(HuVecF));
    camera->zoom = camera->unk_38;
}

inline void fn_1_2810(MDCAMERA_WORK *camera);

void fn_1_2860(MDCAMERA_WORK *camera)
{
    memcpy(&camera->unk_10, &camera->center, sizeof(HuVecF));
    memcpy(&camera->unk_28, &camera->rot, sizeof(HuVecF));
    camera->unk_38 = camera->zoom;
}

inline void fn_1_2860(MDCAMERA_WORK *camera);

void fn_1_28B0(MDCAMERA_WORK *camera, float weight)
{
    fn_1_163C(&camera->center, &camera->unk_10, weight);
    fn_1_163C(&camera->rot, &camera->unk_28, weight);
    camera->zoom = fn_1_160C(camera->zoom, camera->unk_38, weight);
}

inline void fn_1_28B0(MDCAMERA_WORK *camera, float weight);

void fn_1_2B64(MDCAMERA_CALLBACK callback)
{
    lbl_1_bss_A10.callback = callback;
}

inline void fn_1_2B64(MDCAMERA_CALLBACK callback);

void fn_1_2B74(OMOBJ *obj, MDCAMERA_WORK *camera)
{
    float stickX = HuPadStkX[0];
    float stickY = -HuPadStkY[0];
    HuVec2f subStick;
    float moveX;
    float moveZ;

    subStick.y = -HuPadSubStkY[0];
    subStick.x = HuPadSubStkX[0];
    camera->unk_28.x += 0.02f * subStick.y;
    if (camera->unk_28.x > 85.0f) {
        camera->unk_28.x = 85.0f;
    }
    if (camera->unk_28.x < -85.0f) {
        camera->unk_28.x = -85.0f;
    }
    camera->unk_28.y += 0.02f * subStick.x;

    if (HuPadBtn[0] & PAD_TRIGGER_L) {
        camera->unk_10.y -= 0.25f * stickY;
    } else if (HuPadBtn[0] & PAD_TRIGGER_R) {
        camera->unk_38 += 0.25f * stickY;
        if (camera->unk_38 < 5.0f) {
            camera->unk_38 = 5.0f;
        }
    } else {
        moveX = stickX * sind(90.0f + camera->rot.y)
            + stickY * sind(camera->rot.y);
        moveZ = stickX * cosd(90.0f + camera->rot.y)
            + stickY * cosd(camera->rot.y);
        camera->unk_10.x += 0.25f * moveX;
        camera->unk_10.z += 0.25f * moveZ;
    }
    fn_1_28B0(camera, 15.0f);

    print8(16, 110, 1.0f, lbl_1_data_D3E);
    print8(16, 120, 1.0f, lbl_1_data_D60,
        camera->center.x, camera->center.y, camera->center.z);
    print8(16, 130, 1.0f, lbl_1_data_D7A,
        camera->rot.x, camera->rot.y, camera->rot.z);
    print8(16, 140, 1.0f, lbl_1_data_D94, camera->zoom);
}

void fn_1_3284(OMOBJ *obj, MDCAMERA_WORK *camera)
{
    if (camera->callback) {
        camera->callback(obj, camera);
    }
}

inline void fn_1_3284(OMOBJ *obj, MDCAMERA_WORK *camera);

void fn_1_32D0(OMOBJ *obj)
{
    MDCAMERA_WORK *camera = &lbl_1_bss_A10;

    fn_1_3284(obj, camera);
    Center.x = camera->center.x;
    Center.y = camera->center.y;
    Center.z = camera->center.z;
    CRot.x = camera->rot.x;
    CRot.y = camera->rot.y;
    CRot.z = camera->rot.z;
    CZoom = camera->zoom;
    omOutView(obj);
}

void fn_1_33A0(MDCAMERA_CALLBACK callback)
{
    MDCAMERA_WORK *camera = &lbl_1_bss_A10;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f, 10000.0f, 1.2f);
    Hu3DCameraViewportSet(
        1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    memset(camera, 0, sizeof(MDCAMERA_WORK));
    camera->callback = callback;
    camera->center.x = 0.0f;
    camera->center.y = 65.0f;
    camera->center.z = -800.0f;
    camera->rot.x = -7.25f;
    camera->rot.y = 0.0f;
    camera->rot.z = 0.0f;
    if (lbl_1_bss_28 == 0) {
        camera->zoom = 2650.0f;
    } else {
        camera->zoom = 2150.0f;
    }
    camera->obj =
        omAddObjEx(lbl_1_bss_0, 0x7FDA, 0, 0, -1, fn_1_32D0);
}

inline void fn_1_33A0(MDCAMERA_CALLBACK callback);

void fn_1_353C(void)
{
    MDCAMERA_WORK *camera = &lbl_1_bss_A10;

    Hu3DCameraKill(1);
    if (camera->obj) {
        omDelObjEx(lbl_1_bss_0, camera->obj);
    }
    camera->obj = NULL;
}

inline void fn_1_353C(void);

void fn_1_3598(void)
{
    lbl_1_bss_A68[0] = Hu3DGLightCreate(
        0.0f, 1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_A68[0]);
    Hu3DGLightStaticSet(lbl_1_bss_A68[0], TRUE);
    lbl_1_bss_A68[1] = Hu3DGLightCreate(
        -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_A68[1]);
    Hu3DGLightStaticSet(lbl_1_bss_A68[1], TRUE);
}

inline void fn_1_3598(void);

void fn_1_36C4(void)
{
    Hu3DGLightKill(lbl_1_bss_A68[0]);
    Hu3DGLightKill(lbl_1_bss_A68[1]);
}

inline void fn_1_36C4(void);

void fn_1_3700(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_A60[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_A60[winNo]);
    }
}

inline void fn_1_3700(s16 winNo);

void fn_1_3770(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_A60[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_A60[winNo]);
    }
}

inline void fn_1_3770(s16 winNo);

void fn_1_37E0(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_A60[winNo]);
}

inline void fn_1_37E0(s16 winNo);

s16 fn_1_381C(s16 winNo, s16 mode)
{
    s16 choice = 0;

    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_A60[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_A60[winNo], HUWIN_ATTR_NOCANCEL);
    }
    choice = HuWinChoiceGet(lbl_1_bss_A60[winNo], -1);
    if (mode == 2 && choice == -1) {
        choice = 1;
    }
    return choice;
}

inline s16 fn_1_381C(s16 winNo, s16 mode);

void fn_1_38F0(s16 winNo, s32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_A60[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_A60[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_A60[winNo], speed);
    if (lbl_1_data_BC8 != messNum) {
        lbl_1_data_BC8 = -1;
    }
}

inline void fn_1_38F0(s16 winNo, s32 messNum, s16 speed);

void fn_1_39AC(s16 winNo, s32 messNum, s16 insertPos)
{
    HuWinHomeClear(lbl_1_bss_A60[winNo]);
    HuWinInsertMesSet(lbl_1_bss_A60[winNo], messNum, insertPos);
}

inline void fn_1_39AC(s16 winNo, s32 messNum, s16 insertPos);

void fn_1_3A1C(void)
{
    s16 i;

    HuWinInit(1);
    lbl_1_bss_A60[0] =
        HuWinExCreateFrame(16.0f, 337.0f, 0x220, 0x2A, -1, 0);
    HuWinDispOff(lbl_1_bss_A60[0]);
    HuWinBGTPLvlSet(lbl_1_bss_A60[0], 0.0f);
    lbl_1_bss_A60[1] =
        HuWinExCreateFrame(16.0f, 372.0f, 0x220, 0x44, -1, 5);
    HuWinDispOff(lbl_1_bss_A60[1]);
    HuWinBGTPLvlSet(lbl_1_bss_A60[1], 0.9f);
    lbl_1_bss_A60[2] =
        HuWinExCreateFrame(16.0f, 372.0f, 0x220, 0x44, -1, 3);
    HuWinDispOff(lbl_1_bss_A60[2]);
    HuWinBGTPLvlSet(lbl_1_bss_A60[2], 0.9f);
    lbl_1_bss_A60[3] =
        HuWinExCreateFrame(16.0f, 372.0f, 0x220, 0x44, -1, 4);
    HuWinDispOff(lbl_1_bss_A60[3]);
    HuWinBGTPLvlSet(lbl_1_bss_A60[3], 0.9f);

    for (i = 0; i < 4; i++) {
        winData[lbl_1_bss_A60[i]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_A60[i], (HUWIN_CALLBACK)fn_1_0);
    }
}

inline void fn_1_3A1C(void);

void fn_1_3C44(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        HuWinExKill(lbl_1_bss_A60[i]);
    }
    HuWinAllKill();
}

inline void fn_1_3C44(void);

void fn_1_3CA0(s16 winNo)
{
    if (lbl_1_data_DA2[0] != -1 && lbl_1_data_DA2[0] != winNo) {
        fn_1_3770(lbl_1_data_DA2[0]);
    }
    if (lbl_1_data_DA2[0] == -1 || lbl_1_data_DA2[0] != winNo) {
        lbl_1_data_DA2[0] = winNo;
        lbl_1_data_DA8[0] = -1;
        lbl_1_data_DA8[1] = -1;
        fn_1_3700(lbl_1_data_DA2[0]);
    }
}

inline void fn_1_3CA0(s16 winNo);

void fn_1_3E0C(void)
{
    if (lbl_1_data_DA2[0] != -1) {
        fn_1_3770(lbl_1_data_DA2[0]);
    }
    lbl_1_data_DA2[0] = -1;
    lbl_1_data_DA8[0] = -1;
    lbl_1_data_DA8[1] = -1;
}

inline void fn_1_3E0C(void);

void fn_1_3EC8(void)
{
    if (lbl_1_data_DA2[0] != -1) {
        fn_1_37E0(lbl_1_data_DA2[0]);
    }
}

inline void fn_1_3EC8(void);

s16 fn_1_3F28(s16 mode)
{
    if (lbl_1_data_DA2[0] != -1) {
        return fn_1_381C(lbl_1_data_DA2[0], mode);
    }
    return 0;
}

inline s16 fn_1_3F28(s16 mode);

void fn_1_4020(s16 winNo, s32 messNum, s16 speed)
{
    fn_1_3CA0(winNo);
    if (lbl_1_data_DA8[0] != messNum) {
        lbl_1_data_DA8[0] = messNum;
        lbl_1_data_DA8[1] = -1;
        fn_1_38F0(lbl_1_data_DA2[0], lbl_1_data_DA8[0], speed);
    }
}

inline void fn_1_4020(s16 winNo, s32 messNum, s16 speed);

void fn_1_4258(s16 winNo, s32 messNum, s16 insertPos)
{
    fn_1_3CA0(winNo);
    if (lbl_1_data_DA8[1] != messNum) {
        lbl_1_data_DA8[0] = -1;
        lbl_1_data_DA8[1] = messNum;
        fn_1_39AC(lbl_1_data_DA2[0], lbl_1_data_DA8[1], insertPos);
    }
}

inline void fn_1_4258(s16 winNo, s32 messNum, s16 insertPos);

void fn_1_444C(s32 messNum)
{
    if (lbl_1_data_DA2[1] == -1) {
        lbl_1_data_DA2[1] = 0;
        lbl_1_data_DA8[2] = -1;
        fn_1_3700(lbl_1_data_DA2[1]);
    }
    if (lbl_1_data_DA8[2] != messNum) {
        lbl_1_data_DA8[2] = messNum;
        fn_1_38F0(lbl_1_data_DA2[1], lbl_1_data_DA8[2], 0);
    }
}

inline void fn_1_444C(s32 messNum);

void fn_1_45D0(void)
{
    if (lbl_1_data_DA2[1] != -1) {
        fn_1_3770(lbl_1_data_DA2[1]);
    }
    lbl_1_data_DA2[1] = -1;
    lbl_1_data_DA8[2] = -1;
}

inline void fn_1_45D0(void);

void fn_1_467C(void)
{
    HuVecF shadowPos = { 0.0f, 3000.0f, 600.0f };
    HuVecF shadowUp = { 0.0f, 1.0f, 0.0f };
    HuVecF shadowTarget = { 0.0f, 0.0f, 0.0f };

    Hu3DShadowCreate(30.0f, 10.0f, 10000.0f);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
}

inline void fn_1_467C(void);

void fn_1_4730(void)
{
}

void fn_1_4734(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

inline void fn_1_4734(HUSPR_GROUPID groupId, s32 attr);

void fn_1_47B4(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}

inline void fn_1_47B4(HUSPR_GROUPID groupId, s32 attr);

void fn_1_4834(void)
{
    MDSPR_INFO *desc;
    s16 i;

    for (i = 0; i < 32; i++) {
        lbl_1_bss_93C[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(
                lbl_1_data_1C[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 17; i++) {
        lbl_1_bss_918[i] = HuSprGrpCreate(lbl_1_data_9C[i]);
    }
    for (i = 0, desc = lbl_1_data_C0; i < 58; i++, desc++) {
        lbl_1_bss_8A4[i] = HuSprCreate(
            lbl_1_bss_93C[desc->animNo], desc->priority, desc->bank);
        HuSprGrpMemberSet(lbl_1_bss_918[desc->groupNo], desc->memberNo,
            lbl_1_bss_8A4[i]);
        HuSprPosSet(lbl_1_bss_918[desc->groupNo], desc->memberNo,
            desc->pos.x, desc->pos.y);
        HuSprScaleSet(lbl_1_bss_918[desc->groupNo], desc->memberNo,
            desc->scale.x, desc->scale.y);
        HuSprZRotSet(lbl_1_bss_918[desc->groupNo], desc->memberNo,
            desc->zRot);
    }
    for (i = 0; i < 17; i++) {
        fn_1_4734(lbl_1_bss_918[i], HUSPR_ATTR_DISPOFF);
    }
}

inline void fn_1_4834(void);

void fn_1_4A80(void)
{
}

void fn_1_4A84(OMOBJ *obj)
{
    s16 done = 0;
    float duration = 60.0f;
    OMOBJ *obj0 = lbl_1_bss_C;
    OMOBJ *obj1 = lbl_1_bss_10;
    LBL_1_BSS_228_ENTRY *work0 = &lbl_1_bss_228[0];
    LBL_1_BSS_228_ENTRY *work1 = &lbl_1_bss_228[1];
    HuVecF pos;

    if (obj0 != NULL) {
        pos.x = fn_1_1780(work0->pos.x, 500.0f, work0->time, duration);
        pos.y = fn_1_21D4(work0->pos.y, 400.0f, work0->time, duration);
        pos.z = work0->pos.z;
        if (++work0->time > duration) {
            done++;
            fn_1_43F3C(0, NULL, 0);
        } else {
            fn_1_43F3C(0, &pos, 1);
        }
    }
    if (obj1 != NULL) {
        pos.x = fn_1_1780(work1->pos.x, -500.0f, work1->time, duration);
        pos.y = fn_1_21D4(work1->pos.y, 400.0f, work1->time, duration);
        pos.z = work1->pos.z;
        if (++work1->time > duration) {
            done++;
            fn_1_43F3C(1, NULL, 0);
        } else {
            fn_1_43F3C(1, &pos, 1);
        }
    }
    if (done >= 2) {
        obj->objFunc = NULL;
    }
}

void fn_1_4EDC(OMOBJ *obj)
{
    OMOBJ *obj0 = lbl_1_bss_C;
    OMOBJ *obj1 = lbl_1_bss_10;
    LBL_1_BSS_228_ENTRY *work0 = &lbl_1_bss_228[0];
    LBL_1_BSS_228_ENTRY *work1 = &lbl_1_bss_228[1];

    if (obj0 != NULL) {
        Hu3DModelObjPosGet(obj0->mdlId[0], lbl_1_data_DB4, &work0->pos);
        fn_1_43F3C(0, &work0->pos, 1);
        work0->time = 0.0f;
    }
    if (obj1 != NULL) {
        Hu3DModelObjPosGet(obj1->mdlId[0], lbl_1_data_DB4, &work1->pos);
        fn_1_43F3C(1, &work1->pos, 1);
        work1->time = 0.0f;
    }
    if (++obj->work[0] > 30) {
        obj->work[0] = 0;
        obj->objFunc = fn_1_4A84;
    }
}

void fn_1_4FE8(void)
{
    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[4],
        0.0f, 5.0f, 0);
    Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[4],
        0.0f, 5.0f, 0);
    lbl_1_bss_30 =
        omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10, -1, fn_1_4EDC);
    lbl_1_bss_30->work[0] = 0;
    HuAudFXPlay(0x49F);
    HuAudFXPlay(0x4A0);
}

inline void fn_1_4FE8(void);

void fn_1_50EC(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(obj->mdlId[0], 2.0f);
    if (obj->work[3]++ > 30) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f, 15.0f,
            HU3D_MOTATTR_LOOP);
    }
}

void fn_1_5194(void)
{
    OMOBJ *obj = lbl_1_bss_C;

    Hu3DMotionShiftSet(
        obj->mdlId[0], obj->mtnId[3], 0.0f, 10.0f, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_50EC;
}

inline void fn_1_5194(void);

void fn_1_5208(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(obj->mdlId[0], 2.0f);
    if (obj->work[3]++ > 30) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f, 15.0f,
            HU3D_MOTATTR_LOOP);
    }
}

void fn_1_52B0(void)
{
    OMOBJ *obj = lbl_1_bss_10;

    Hu3DMotionShiftSet(
        obj->mdlId[0], obj->mtnId[3], 0.0f, 10.0f, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_5208;
}

inline void fn_1_52B0(void);

void fn_1_5324(OMOBJ *obj)
{
    LBL_1_BSS_1C8_ENTRY *work = &lbl_1_bss_1C8[0];
    HuVecF pos;

    if (work->state == 0) {
        Hu3DModelPosGet(obj->mdlId[0], &pos);
        pos.y += 5.0f;
        Hu3DModelPosSetV(obj->mdlId[0], &pos);
        if ((work->time += 1.0f) > work->duration) {
            Hu3DModelPosGet(obj->mdlId[0], &work->pos);
            work->state = 1;
            work->time = 0.0f;
            work->duration = 90.0f;
        }
    }
    if (work->state == 1) {
        fn_1_22A8(obj->mdlId[0], &work->pos, &work->unk_10,
            &work->unk_1C, work->time, work->duration);
        if ((work->time += 1.0f) > work->duration) {
            HuAudFXStop(lbl_1_bss_A6C[0]);
            work->state = 2;
        }
    }
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    fn_1_464A0(4, &pos, 1, 5);
    if (work->state == 2) {
        obj->objFunc = NULL;
    }
}

void fn_1_58A0(void)
{
    LBL_1_BSS_1C8_ENTRY *work = &lbl_1_bss_1C8[0];
    OMOBJ *obj = lbl_1_bss_C;

    work->state = 0;
    work->time = 0.0f;
    work->duration = 20.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->pos);
    work->pos.x = -325.0f;
    work->pos.y = 0.0f;
    work->pos.z = 100.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->unk_10);
    work->unk_10.x = 100.0f;
    work->unk_10.y = 600.0f;
    work->unk_10.z = 200.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->unk_1C);
    work->unk_1C.x = 100.0f;
    work->unk_1C.y = 225.0f;
    work->unk_1C.z = -1000.0f;
    Hu3DMotionShiftSet(
        obj->mdlId[0], obj->mtnId[2], 0.0f, 0.0f, 0);
    lbl_1_bss_A6C[0] = HuAudFXPlay(0x47E);
    obj->objFunc = fn_1_5324;
}

inline void fn_1_58A0(void);

void fn_1_5A18(OMOBJ *obj)
{
    HuVecF path[2] = {
        { -100.0f, 0.0f, 0.0f },
        { -325.0f, 0.0f, 100.0f },
    };
    float time = obj->work[0];

    fn_1_1D54(
        obj->mdlId[0], &path[0], &path[1], time, 30.0f, 15.0f);
    Hu3DMotionSpeedSet(obj->mdlId[0], 1.5f);
    if (++obj->work[0] > 60) {
        Hu3DMotionSpeedSet(obj->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            0.0f, 15.0f, HU3D_MOTATTR_LOOP);
        obj->objFunc = NULL;
    }
}

s32 fn_1_5FB0(void)
{
    OMOBJ *obj = lbl_1_bss_C;
    HuVecF pos;

    Hu3DModelPosGet(obj->mdlId[0], &pos);
    if (pos.x < -100.0f) {
        return FALSE;
    }
    obj->work[0] = 0;
    obj->objFunc = fn_1_5A18;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1], 0.0f, 0.0f,
        HU3D_MOTATTR_LOOP);
    return TRUE;
}

inline s32 fn_1_5FB0(void);

void fn_1_605C(OMOBJ *obj)
{
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreate(
        HuDataReadNum(DATANUM(DATA_mdparty, 0x4F), HU_MEMNUM_OVL));
    obj->mtnId[0] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 0x50), HU_MEMNUM_OVL));
    obj->mtnId[1] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 0x51), HU_MEMNUM_OVL));
    obj->mtnId[2] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 0x52), HU_MEMNUM_OVL));
    obj->mtnId[3] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 0x53), HU_MEMNUM_OVL));
    obj->mtnId[4] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 0x54), HU_MEMNUM_OVL));
    obj->mtnId[5] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 0x55), HU_MEMNUM_OVL));
    Hu3DModelPosSet(obj->mdlId[0], -100.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(obj->mdlId[0], 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 1.25f, 1.25f, 1.25f);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f, 0.0f,
        HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(obj->mdlId[0]);
    obj->objFunc = NULL;
}

void fn_1_6284(OMOBJ *obj)
{
    if (obj) {
        Hu3DMotionKill(obj->mtnId[0]);
        Hu3DMotionKill(obj->mtnId[1]);
        Hu3DMotionKill(obj->mtnId[2]);
        Hu3DMotionKill(obj->mtnId[3]);
        Hu3DMotionKill(obj->mtnId[4]);
        Hu3DMotionKill(obj->mtnId[5]);
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj->objFunc = NULL;
}

inline void fn_1_6284(OMOBJ *obj);

void fn_1_6324(OMOBJ *obj)
{
    LBL_1_BSS_1C8_ENTRY *work = &lbl_1_bss_1C8[1];
    HuVecF pos;

    if (work->state == 0) {
        Hu3DModelPosGet(obj->mdlId[0], &pos);
        pos.y += 5.0f;
        Hu3DModelPosSetV(obj->mdlId[0], &pos);
        if ((work->time += 1.0f) > work->duration) {
            Hu3DModelPosGet(obj->mdlId[0], &work->pos);
            work->state = 1;
            work->time = 0.0f;
            work->duration = 90.0f;
        }
    }
    if (work->state == 1) {
        fn_1_22A8(obj->mdlId[0], &work->pos, &work->unk_10,
            &work->unk_1C, work->time, work->duration);
        if ((work->time += 1.0f) > work->duration) {
            HuAudFXStop(lbl_1_bss_A6C[1]);
            work->state = 2;
        }
    }
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    fn_1_464A0(5, &pos, 1, 6);
    if (work->state == 2) {
        obj->objFunc = NULL;
    }
}

void fn_1_68A4(void)
{
    LBL_1_BSS_1C8_ENTRY *work = &lbl_1_bss_1C8[1];
    OMOBJ *obj = lbl_1_bss_10;

    work->state = 0;
    work->time = 0.0f;
    work->duration = 20.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->pos);
    work->pos.x = 325.0f;
    work->pos.y = 0.0f;
    work->pos.z = 100.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->unk_10);
    work->unk_10.x = -100.0f;
    work->unk_10.y = 400.0f;
    work->unk_10.z = 200.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->unk_1C);
    work->unk_1C.x = -100.0f;
    work->unk_1C.y = 125.0f;
    work->unk_1C.z = -1000.0f;
    Hu3DMotionShiftSet(
        obj->mdlId[0], obj->mtnId[2], 0.0f, 0.0f, 0);
    lbl_1_bss_A6C[1] = HuAudFXPlay(0x47C);
    obj->objFunc = fn_1_6324;
}

inline void fn_1_68A4(void);

void fn_1_6A20(OMOBJ *obj)
{
    HuVecF path[2] = {
        { 100.0f, 0.0f, 0.0f },
        { 325.0f, 0.0f, 100.0f },
    };
    float time = obj->work[0];

    fn_1_1D54(
        obj->mdlId[0], &path[0], &path[1], time, 30.0f, -15.0f);
    Hu3DMotionSpeedSet(obj->mdlId[0], 1.5f);
    if (++obj->work[0] > 60) {
        Hu3DMotionSpeedSet(obj->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            0.0f, 15.0f, HU3D_MOTATTR_LOOP);
        obj->objFunc = NULL;
    }
}

s32 fn_1_6FB8(void)
{
    OMOBJ *obj = lbl_1_bss_10;
    HuVecF pos;

    Hu3DModelPosGet(obj->mdlId[0], &pos);
    if (pos.x > 100.0f) {
        return FALSE;
    }
    obj->work[0] = 0;
    obj->objFunc = fn_1_6A20;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1], 0.0f, 0.0f,
        HU3D_MOTATTR_LOOP);
    return TRUE;
}

inline s32 fn_1_6FB8(void);

void fn_1_7064(OMOBJ *obj)
{
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreate(
        HuDataReadNum(DATANUM(DATA_mdparty, 0x56), HU_MEMNUM_OVL));
    obj->mdlId[1] = Hu3DModelCreate(
        HuDataReadNum(DATANUM(DATA_mdparty, 0x57), HU_MEMNUM_OVL));
    obj->mtnId[0] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 0x58), HU_MEMNUM_OVL));
    obj->mtnId[1] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 0x59), HU_MEMNUM_OVL));
    obj->mtnId[2] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 0x5A), HU_MEMNUM_OVL));
    obj->mtnId[3] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 0x5B), HU_MEMNUM_OVL));
    obj->mtnId[4] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 0x5C), HU_MEMNUM_OVL));
    obj->mtnId[5] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 0x5D), HU_MEMNUM_OVL));
    Hu3DModelHookSet(obj->mdlId[0], lbl_1_data_DC6, obj->mdlId[1]);
    Hu3DModelPosSet(obj->mdlId[0], 100.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(obj->mdlId[0], 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 1.25f, 1.25f, 1.25f);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f, 0.0f,
        HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(obj->mdlId[0]);
    obj->objFunc = NULL;
}

void fn_1_72C4(OMOBJ *obj)
{
    if (obj) {
        Hu3DModelHookReset(obj->mdlId[1]);
        Hu3DMotionKill(obj->mtnId[0]);
        Hu3DMotionKill(obj->mtnId[1]);
        Hu3DMotionKill(obj->mtnId[2]);
        Hu3DMotionKill(obj->mtnId[3]);
        Hu3DMotionKill(obj->mtnId[4]);
        Hu3DMotionKill(obj->mtnId[5]);
        Hu3DModelKill(obj->mdlId[0]);
        Hu3DModelKill(obj->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj->objFunc = NULL;
}

inline void fn_1_72C4(OMOBJ *obj);

void fn_1_737C(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *desc;
    LBL_1_BSS_1C8_ENTRY *work;
    HuVecF pos;
    s16 i;

    for (i = 0, desc = &lbl_1_bss_9BC[i]; i < 4; i++, desc++) {
        work = &lbl_1_bss_108[i];
        if (work->state == 1) {
            fn_1_22A8(obj->mdlId[desc->chrSel], &work->pos,
                &work->unk_10, &work->unk_1C, work->time,
                work->duration);
            if ((work->time += 1.0f) > work->duration) {
                work->state = 2;
                Hu3DModelAttrSet(
                    obj->mdlId[desc->chrSel], HU3D_ATTR_DISPOFF);
            }
            Hu3DModelPosGet(obj->mdlId[desc->chrSel], &pos);
            if (desc->unk_4 == 0) {
                fn_1_464A0(i, &pos, 1, desc->unk_A);
            } else {
                fn_1_464A0(i, &pos, 1, 4);
            }
        }
    }
    for (i = 0; i < 4; i++) {
        work = &lbl_1_bss_108[i];
        if (work->state != 2) {
            break;
        }
    }
    if (i == 4) {
        obj->objFunc = NULL;
    }
}

void fn_1_7900(void)
{
    LBL_1_BSS_9BC_ENTRY *desc;
    OMOBJ *obj = lbl_1_bss_14;
    LBL_1_BSS_1C8_ENTRY *work;
    s16 i;

    for (i = 0, desc = &lbl_1_bss_9BC[i]; i < 4; i++, desc++) {
        work = &lbl_1_bss_108[i];
        work->state = 1;
        work->time = 0.0f;
        work->duration = 90.0f;
        Hu3DModelPosGet(obj->mdlId[desc->chrSel], &work->pos);
        Hu3DModelPosGet(obj->mdlId[desc->chrSel], &work->unk_10);
        work->unk_10.x += 0.5f * work->unk_10.x;
        work->unk_10.y = 550.0f;
        work->unk_10.z = 100.0f;
        Hu3DModelPosGet(obj->mdlId[desc->chrSel], &work->unk_1C);
        work->unk_1C.y = 225.0f;
        work->unk_1C.z = -800.0f;
        Hu3DMotionShiftSet(obj->mdlId[desc->chrSel],
            obj->mtnId[(2 * desc->chrSel) + 1], 0.0f, 10.0f,
            HU3D_MOTATTR_LOOP);
    }
    HuAudFXPlay(0x4A2);
    obj->objFunc = fn_1_737C;
}

inline void fn_1_7900(void);

void fn_1_7ABC(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *desc;
    float scale;

    for (i = 0, desc = &lbl_1_bss_9BC[i]; i < 4; i++, desc++) {
        scale = fn_1_1868(0.0f, 1.0f, obj->work[0], obj->work[1]);
        Hu3DModelScaleSet(
            obj->mdlId[desc->chrSel], scale, scale, scale);
    }
    if ((obj->work[0] += 1.0f) > obj->work[1]) {
        obj->objFunc = NULL;
    }
}

void fn_1_7C88(void)
{
    OMOBJ *obj = lbl_1_bss_14;
    LBL_1_BSS_9BC_ENTRY *desc;
    HuVecF pos;
    s16 i;

    for (i = 0, desc = &lbl_1_bss_9BC[i]; i < 4; i++, desc++) {
        Hu3DModelPosGet(lbl_1_bss_288[desc->chrSel].modelId, &pos);
        if (desc->unk_4 != 0) {
            fn_1_45F3C(i, &pos, 4, 0);
        } else {
            fn_1_45F3C(i, &pos, desc->unk_A, 0);
        }
    }
    for (i = 0, desc = &lbl_1_bss_9BC[i]; i < 4; i++, desc++) {
        Hu3DModelPosGet(lbl_1_bss_288[desc->chrSel].modelId, &pos);
        pos.y -= 70.0f;
        Hu3DModelPosSetV(obj->mdlId[desc->chrSel], &pos);
        Hu3DModelScaleSet(obj->mdlId[desc->chrSel], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(obj->mdlId[desc->chrSel], HU3D_ATTR_DISPOFF);
    }
    obj->work[0] = 0;
    obj->work[1] = 15;
    obj->objFunc = fn_1_7ABC;
    HuPrcSleep(15);
    HuAudFXPlay(0x4A1);
    for (i = 0, desc = &lbl_1_bss_9BC[i]; i < 4; i++, desc++) {
        Hu3DModelPosGet(lbl_1_bss_288[desc->chrSel].modelId, &pos);
        if (desc->unk_4 != 0) {
            fn_1_45F3C(i, &pos, 4, 1);
        } else {
            fn_1_45F3C(i, &pos, desc->unk_A, 1);
        }
    }
}

void fn_1_7ED4(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 11; i++) {
        obj->mdlId[i] =
            Hu3DModelCreateData(DATANUM(DATA_mdparty, 0x5E) + i);
        obj->mtnId[2 * i] = Hu3DJointMotionData(
            obj->mdlId[i], DATANUM(DATA_mdparty, 0x69) + i);
        obj->mtnId[(2 * i) + 1] = Hu3DJointMotionData(
            obj->mdlId[i], DATANUM(DATA_mdparty, 0x74) + i);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[2 * i],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelShadowSet(obj->mdlId[i]);
    }
    obj->objFunc = NULL;
}

void fn_1_8074(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 11; i++) {
            Hu3DMotionKill(obj->mtnId[2 * i]);
            Hu3DMotionKill(obj->mtnId[(2 * i) + 1]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

inline void fn_1_8074(OMOBJ *obj);

void fn_1_8124(s16 arg0, s16 arg1)
{
    s16 digit = 0;

    if (arg0 <= arg1) {
        HuSprGrpPosSet(lbl_1_bss_918[12], 288.0f, 280.0f);
        fn_1_47B4(lbl_1_bss_918[12], HUSPR_ATTR_DISPOFF);

        digit = arg0 / 10;
        HuSprBankSet(lbl_1_bss_918[12], 0, digit);
        if (digit == 0) {
            HuSprAttrSet(lbl_1_bss_918[12], 0, HUSPR_ATTR_DISPOFF);
        }
        digit = arg0 % 10;
        HuSprBankSet(lbl_1_bss_918[12], 1, digit);

        digit = arg1 / 10;
        HuSprBankSet(lbl_1_bss_918[12], 2, digit);
        if (digit == 0) {
            HuSprAttrSet(lbl_1_bss_918[12], 2, HUSPR_ATTR_DISPOFF);
        }
        digit = arg1 % 10;
        HuSprBankSet(lbl_1_bss_918[12], 3, digit);
    }
}

void fn_1_8318(void)
{
    fn_1_4734(lbl_1_bss_918[12], HUSPR_ATTR_DISPOFF);
}

void fn_1_8398(OMOBJ *obj)
{
    float alpha = fn_1_1868(1.0f, 0.0f, obj->work[0], 10.0f);

    HuSprGrpTPLvlSet(lbl_1_bss_918[0], alpha);
    if (++obj->work[0] > 10) {
        fn_1_4734(lbl_1_bss_918[0], HUSPR_ATTR_DISPOFF);
        obj->objFunc = NULL;
    }
}

void fn_1_852C(void)
{
    if (lbl_1_bss_28 == 0) {
        HuSprGrpPosSet(lbl_1_bss_918[0], 288.0f, 120.0f);
        fn_1_47B4(lbl_1_bss_918[0], HUSPR_ATTR_DISPOFF);
    }
}

inline void fn_1_852C(void);

void fn_1_85E8(void)
{
    OMOBJ *obj = lbl_1_bss_4;

    obj->work[0] = 0;
    obj->objFunc = fn_1_8398;
}

void fn_1_861C(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 3; i++) {
        obj->mdlId[i] = Hu3DModelCreateData(DATA_mdparty + i);
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelShadowMapSet(obj->mdlId[i]);
    }
    fn_1_852C();
    obj->objFunc = NULL;
}

void fn_1_87BC(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 3; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

inline void fn_1_87BC(OMOBJ *obj);

void fn_1_884C(s16 memberNo)
{
    OMOBJ *obj = lbl_1_bss_8;

    HuSprGrpPosSet(lbl_1_bss_918[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_918[1], 0.0f);
    fn_1_4734(lbl_1_bss_918[1], HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(
        lbl_1_bss_918[1], memberNo, HUSPR_ATTR_DISPOFF);
    obj->work[2] = 1;
    obj->work[3] = 0;
}

inline void fn_1_884C(s16 memberNo);

void fn_1_8950(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[2] = 2;
    obj->work[3] = 0;
}

inline void fn_1_8950(void);

void fn_1_8980(void)
{
    HuSprGrpPosSet(lbl_1_bss_918[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_918[1], 0.0f);
}

inline void fn_1_8980(void);

void fn_1_89E0(OMOBJ *obj)
{
    float alpha;

    switch (obj->work[2]) {
        case 1:
            alpha = fn_1_1868(
                0.0f, 1.0f, obj->work[3], 10.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_918[1], alpha);
            if (++obj->work[3] > 10) {
                obj->work[2] = 0;
            }
            break;
        case 2:
            alpha = fn_1_1868(
                1.0f, 0.0f, obj->work[3], 10.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_918[1], alpha);
            if (++obj->work[3] > 10) {
                obj->work[2] = 0;
            }
            break;
    }
}

void fn_1_8C28(s16 animNo)
{
    OMOBJ *obj = lbl_1_bss_8;

    Hu3DAnimAnimSet(
        obj->mtnId[2], lbl_1_bss_93C[(2 * animNo) + 3]);
    Hu3DAnimAnimSet(
        obj->mtnId[3], lbl_1_bss_93C[(2 * animNo) + 4]);
    obj->work[0] = 1;
    obj->work[1] = 0;
}

inline void fn_1_8C28(s16 animNo);

void fn_1_8CCC(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[0] = 2;
    obj->work[1] = 0;
    fn_1_8950();
}

inline void fn_1_8CCC(void);

void fn_1_8D20(OMOBJ *obj)
{
    HuVecF rot;
    float modelAlpha;
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelRotGet(obj->mdlId[i], &rot);
        rot.z -= 0.1f;
        if (rot.z < 0.0f) {
            rot.z += 360.0f;
        }
        Hu3DModelRotSetV(obj->mdlId[i], &rot);
    }
    switch (obj->work[0]) {
        case 1:
            modelAlpha = fn_1_1868(
                1.0f, 0.0f, obj->work[1], 30.0f);
            Hu3DModelTPLvlSet(obj->mdlId[0], modelAlpha);
            if (++obj->work[1] > 30) {
                obj->work[0] = 0;
            }
            break;
        case 2:
            modelAlpha = fn_1_1868(
                0.0f, 1.0f, obj->work[1], 30.0f);
            Hu3DModelTPLvlSet(obj->mdlId[0], modelAlpha);
            if (++obj->work[1] > 30) {
                obj->work[0] = 0;
            }
            break;
    }
    fn_1_89E0(obj);
}

void fn_1_9218(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 2; i++) {
        if (i == 0) {
            obj->mdlId[i] = Hu3DModelCreate(
                HuDataSelHeapReadNum(
                    DATANUM(DATA_mdparty, 3), HU_MEMNUM_OVL,
                    HEAP_MODEL));
        } else {
            obj->mdlId[i] = Hu3DModelLink(obj->mdlId[0]);
        }
        Hu3DModelPosSet(
            obj->mdlId[i], 0.0f, -100.0f, -850.0f - i);
        Hu3DModelRotSet(obj->mdlId[i], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(obj->mdlId[i], 0);
        obj->mtnId[2 * i] = Hu3DAnimCreate(
            lbl_1_bss_93C[1], obj->mdlId[i], lbl_1_data_DD8);
        obj->mtnId[(2 * i) + 1] = Hu3DAnimCreate(
            lbl_1_bss_93C[2], obj->mdlId[i], lbl_1_data_DDF);
    }
    HuSprGrpPosSet(lbl_1_bss_918[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_918[1], 0.0f);
    obj->objFunc = fn_1_8D20;
}

void fn_1_945C(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 2; i++) {
            Hu3DAnimKill(obj->mtnId[2 * i]);
            Hu3DAnimKill(obj->mtnId[(2 * i) + 1]);
            obj->mtnId[2 * i] = -1;
            obj->mtnId[(2 * i) + 1] = -1;
        }
        for (i = 1; i >= 0; i--) {
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

inline void fn_1_945C(OMOBJ *obj);

u32 fn_1_9558(s16 charNo1, s16 charNo2)
{
    s8 pair[55][2] = {
        { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 0, 5 },
        { 0, 6 }, { 0, 7 }, { 0, 8 }, { 0, 9 }, { 0, 10 },
        { 1, 2 }, { 1, 3 }, { 1, 4 }, { 1, 5 }, { 1, 6 },
        { 1, 7 }, { 1, 8 }, { 1, 9 }, { 1, 10 }, { 2, 3 },
        { 2, 4 }, { 2, 5 }, { 2, 6 }, { 2, 7 }, { 2, 8 },
        { 2, 9 }, { 2, 10 }, { 3, 4 }, { 3, 5 }, { 3, 6 },
        { 3, 7 }, { 3, 8 }, { 3, 9 }, { 3, 10 }, { 4, 5 },
        { 4, 6 }, { 4, 7 }, { 4, 8 }, { 4, 9 }, { 4, 10 },
        { 5, 6 }, { 5, 7 }, { 5, 8 }, { 5, 9 }, { 5, 10 },
        { 6, 7 }, { 6, 8 }, { 6, 9 }, { 6, 10 }, { 7, 8 },
        { 7, 9 }, { 7, 10 }, { 8, 9 }, { 8, 10 }, { 9, 10 },
    };
    s8 i;

    for (i = 0; i < 55; i++) {
        if ((charNo1 == pair[i][0] && charNo2 == pair[i][1])
            || (charNo1 == pair[i][1] && charNo2 == pair[i][0])) {
            break;
        }
    }
    if (i == 55) {
        return 0x30037;
    }
    return 0x30000 + i;
}

inline u32 fn_1_9558(s16 charNo1, s16 charNo2);

void fn_1_9658(s16 winNo, u32 messNum)
{
    HuWinAttrSet(lbl_1_bss_104[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_104[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_104[winNo], 0);
}

inline void fn_1_9658(s16 winNo, u32 messNum);

void fn_1_96E4(u32 messNum1, u32 messNum2)
{
    HuWinDispOn(lbl_1_bss_104[0]);
    HuWinDispOn(lbl_1_bss_104[1]);
    fn_1_9658(0, messNum1);
    fn_1_9658(1, messNum2);
    fn_1_47B4(lbl_1_bss_918[13], HUSPR_ATTR_DISPOFF);
}

inline void fn_1_96E4(u32 messNum1, u32 messNum2);

void fn_1_9804(void)
{
    HuWinDispOff(lbl_1_bss_104[0]);
    HuWinDispOff(lbl_1_bss_104[1]);
    fn_1_4734(lbl_1_bss_918[13], HUSPR_ATTR_DISPOFF);
}

inline void fn_1_9804(void);

void fn_1_98A4(void)
{
    HuVecF pos[2] = {
        { 150.0f, 120.0f, 0.0f },
        { 426.0f, 120.0f, 0.0f },
    };
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_104[i] = HuWinExCreateFrame(
            pos[i].x - 124.0f, 6.0f + pos[i].y, 240, 42, -1, 0);
        HuWinDispOff(lbl_1_bss_104[i]);
        HuWinBGTPLvlSet(lbl_1_bss_104[i], 0.0f);
        HuWinPriSet(lbl_1_bss_104[i], 0);
        HuSprPosSet(lbl_1_bss_918[13], i, pos[i].x, pos[i].y);
    }
}

inline void fn_1_98A4(void);

void fn_1_9A24(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        HuWinExKill(lbl_1_bss_104[i]);
    }
}

inline void fn_1_9A24(void);

void fn_1_9A7C(s16 playerNo, s16 bank)
{
    lbl_1_bss_D4[playerNo].x = 0.85f;
    fn_1_47B4(
        lbl_1_bss_918[playerNo + 4], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_918[playerNo + 4], 0, bank);
}

inline void fn_1_9A7C(s16 playerNo, s16 bank);

void fn_1_9B68(s16 arg0)
{
    lbl_1_bss_D4[arg0].x = 0.0f;
}

void fn_1_9B90(s16 arg0, s16 arg1)
{
    HuSprBankSet(lbl_1_bss_918[arg0 + 4], 0, arg1);
}

inline void fn_1_9B90(s16 arg0, s16 arg1);

void fn_1_9BDC(s16 playerNo, s16 bank)
{
    lbl_1_bss_A4[playerNo].x = 0.85f;
    fn_1_47B4(
        lbl_1_bss_918[playerNo + 8], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_918[playerNo + 8], 2, bank);
}

inline void fn_1_9BDC(s16 playerNo, s16 bank);

void fn_1_9CC8(s16 arg0)
{
    lbl_1_bss_A4[arg0].x = 0.0f;
}

void fn_1_9CF0(s16 arg0, s16 arg1)
{
    HuSprBankSet(lbl_1_bss_918[arg0 + 8], 2, arg1);
}

void fn_1_9D3C(s16 playerNo, s16 bank)
{
    lbl_1_bss_8C[playerNo].x = 0.85f;
    fn_1_47B4(
        lbl_1_bss_918[playerNo + 14], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_918[playerNo + 14], 2, bank);
}

inline void fn_1_9D3C(s16 playerNo, s16 bank);

void fn_1_9E28(s16 arg0)
{
    lbl_1_bss_8C[arg0].x = 0.0f;
}

void fn_1_9E50(s16 arg0, s16 arg1)
{
    HuSprBankSet(lbl_1_bss_918[arg0 + 14], 2, arg1);
}

inline void fn_1_9E50(s16 arg0, s16 arg1);

void fn_1_9E9C(void)
{
    lbl_1_bss_80.x = 0.5f;
    fn_1_47B4(lbl_1_bss_918[16], HUSPR_ATTR_DISPOFF);
}

inline void fn_1_9E9C(void);

void fn_1_9F34(void)
{
    lbl_1_bss_80.x = 0.0f;
}

inline void fn_1_9F34(void);

void fn_1_9F50(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    HuVecF pos3D;
    HuVecF pos2D;
    s16 i;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        Hu3DModelPosGet(
            lbl_1_bss_288[lbl_1_bss_9BC[i].chrSel].modelId,
            &pos3D);
        Hu3D3Dto2D(&pos3D, 1, &pos2D);
        HuSprGrpPosSet(lbl_1_bss_918[i + 4],
            pos2D.x - 18.0f, (s16)(pos2D.y - 23.0f));
        HuSprAttrReset(
            lbl_1_bss_918[i + 4], 0, HUSPR_ATTR_LINEAR);
        lbl_1_bss_D4[i].y = fn_1_160C(
            lbl_1_bss_D4[i].y, lbl_1_bss_D4[i].x, 3.0f);
        HuSprGrpScaleSet(lbl_1_bss_918[i + 4],
            lbl_1_bss_D4[i].y, lbl_1_bss_D4[i].y);

        Hu3DModelPosGet(
            lbl_1_bss_288[lbl_1_bss_9BC[i].chrSel].modelId,
            &pos3D);
        Hu3D3Dto2D(&pos3D, 1, &pos2D);
        HuSprGrpPosSet(lbl_1_bss_918[i + 8],
            pos2D.x + 18.0f, pos2D.y + 23.0f);
        lbl_1_bss_A4[i].y = fn_1_160C(
            lbl_1_bss_A4[i].y, lbl_1_bss_A4[i].x, 3.0f);
        HuSprGrpScaleSet(lbl_1_bss_918[i + 8],
            lbl_1_bss_A4[i].y, lbl_1_bss_A4[i].y);
    }

    for (i = 0; i < 2; i++) {
        Hu3DModelPosGet(
            lbl_1_bss_288[lbl_1_bss_9BC[i].chrSel].modelId,
            &pos3D);
        pos3D.x = lbl_1_data_8A4[i + 61].x;
        Hu3D3Dto2D(&pos3D, 1, &pos2D);
        HuSprGrpPosSet(lbl_1_bss_918[i + 14],
            pos2D.x, pos2D.y + 23.0f);
        lbl_1_bss_8C[i].y = fn_1_160C(
            lbl_1_bss_8C[i].y, lbl_1_bss_8C[i].x, 3.0f);
        HuSprGrpScaleSet(lbl_1_bss_918[i + 14],
            lbl_1_bss_8C[i].y, lbl_1_bss_8C[i].y);
    }

    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9BC[i].chrSel].modelId,
        &pos3D);
    pos3D.x = 0.0f;
    Hu3D3Dto2D(&pos3D, 1, &pos2D);
    HuSprGrpPosSet(lbl_1_bss_918[16], pos2D.x, pos2D.y);
    lbl_1_bss_80.y = fn_1_160C(
        lbl_1_bss_80.y, lbl_1_bss_80.x, 3.0f);
    HuSprGrpScaleSet(
        lbl_1_bss_918[16], lbl_1_bss_80.y, lbl_1_bss_80.y);
}

void fn_1_A5D8(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        lbl_1_bss_D4[i].x = lbl_1_bss_D4[i].y = 0.0f;
        lbl_1_bss_A4[i].x = lbl_1_bss_A4[i].y = 0.0f;
    }
    for (i = 0; i < 2; i++) {
        lbl_1_bss_8C[i].x = lbl_1_bss_8C[i].y = 0.0f;
    }
    lbl_1_bss_80.x = lbl_1_bss_80.y = 0.0f;
    obj->objFunc = fn_1_9F50;
    fn_1_98A4();
}

void fn_1_A880(void)
{
    fn_1_9A24();
}

inline void fn_1_A880(void);

void fn_1_A8D8(
    s16 memberNo, HuVecF *pos3D, float xOffset, float yOffset)
{
    HuVecF pos2D = { 0.0f, 0.0f, 0.0f };

    if (pos3D) {
        Hu3D3Dto2D(pos3D, 1, &pos2D);
    }
    HuSprPosSet(lbl_1_bss_918[2], memberNo,
        pos2D.x + xOffset, pos2D.y + yOffset);
}

inline void fn_1_A8D8(
    s16 memberNo, HuVecF *pos3D, float xOffset, float yOffset);

void fn_1_A970(
    s16 memberNo, HuVecF *pos3D, float xOffset, float yOffset)
{
    fn_1_A8D8(memberNo, pos3D, xOffset, yOffset);
    HuSprScaleSet(lbl_1_bss_918[2], memberNo, 2.0f, 2.0f);
    HuSprAttrReset(lbl_1_bss_918[2], memberNo, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_38[memberNo] = 2.0f;
    lbl_1_data_DE6[memberNo] = 1;
}

inline void fn_1_A970(
    s16 memberNo, HuVecF *pos3D, float xOffset, float yOffset);

void fn_1_AA94(s16 memberNo)
{
    HuSprAttrSet(lbl_1_bss_918[2], memberNo, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[memberNo] = 0;
}

inline void fn_1_AA94(s16 memberNo);

void fn_1_AAF0(s16 memberNo)
{
    lbl_1_bss_38[memberNo] = 2.0f;
    if (lbl_1_data_DE6[memberNo] == 1) {
        HuAudFXPlay(0);
    }
}

inline void fn_1_AAF0(s16 memberNo);

void fn_1_AB64(void)
{
    s16 i;

    for (i = 0; i < 6; i++) {
        lbl_1_bss_38[i] =
            fn_1_160C(lbl_1_bss_38[i], 1.0f, 5.0f);
        HuSprScaleSet(lbl_1_bss_918[2], i,
            lbl_1_bss_38[i], lbl_1_bss_38[i]);
    }
}

inline void fn_1_AB64(void);

void fn_1_AC88(void)
{
    s16 i;

    for (i = 0; i < 6; i++) {
        fn_1_AA94(i);
        lbl_1_data_DE6[i] = 0;
    }
}

void fn_1_AD14(s16 modelNo, HuVecF *pos)
{
    lbl_1_bss_50[modelNo].x = pos->x - 50.0f;
    lbl_1_bss_50[modelNo].y = 50.0f + pos->y;
    lbl_1_bss_50[modelNo].z = 50.0f + pos->z;
}

inline void fn_1_AD14(s16 modelNo, HuVecF *pos);

void fn_1_AD9C(s32 modelNo, HuVecF *pos, s16 bank)
{
    OMOBJ *obj = lbl_1_bss_1C.obj;

    Hu3DModelPosSet(
        obj->mdlId[(s16)modelNo], pos->x, pos->y, pos->z);
    Hu3DModelRotSet(
        obj->mdlId[(s16)modelNo], 0.0f, 0.0f, 180.0f);
    Hu3DModelScaleSet(
        obj->mdlId[(s16)modelNo], 1.0f, 1.0f, 1.0f);
    fn_1_AD14(modelNo, pos);
    Hu3DModelAttrReset(
        obj->mdlId[(s16)modelNo], HU3D_ATTR_DISPOFF);
    HuSprScaleSet(lbl_1_bss_918[3], modelNo, 0.8f, 0.8f);
    HuSprAttrReset(lbl_1_bss_918[3], modelNo, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_918[3], modelNo, bank);
}

inline void fn_1_AD9C(s32 modelNo, HuVecF *pos, s16 bank);

void fn_1_AF70(s32 modelNo)
{
    OMOBJ *obj = lbl_1_bss_1C.obj;

    Hu3DModelAttrSet(obj->mdlId[(s16)modelNo], HU3D_ATTR_DISPOFF);
    HuSprScaleSet(lbl_1_bss_918[3], modelNo, 0.8f, 0.8f);
    HuSprAttrSet(lbl_1_bss_918[3], modelNo, HUSPR_ATTR_DISPOFF);
}

inline void fn_1_AF70(s32 modelNo);

void fn_1_B008(OMOBJ *obj)
{
    HuVecF pos3D;
    HuVecF pos2D;
    s16 i;

    for (i = 0; i < 4; i++) {
        Hu3DModelPosGet(obj->mdlId[i], &pos3D);
        fn_1_163C(&pos3D, &lbl_1_bss_50[i], 3.0f);
        Hu3DModelPosSetV(obj->mdlId[i], &pos3D);
        Hu3D3Dto2D(&pos3D, 1, &pos2D);
        HuSprPosSet(lbl_1_bss_918[3], (s16)i, pos2D.x, pos2D.y);
        HuSprScaleSet(lbl_1_bss_918[3], (s16)i, 0.8f, 0.8f);

        Hu3DModelRotGet(obj->mdlId[i], &pos3D);
        pos3D.z = fn_1_160C(pos3D.z, 0.0f, 3.0f);
        Hu3DModelRotSetV(obj->mdlId[i], &pos3D);

        Hu3DModelScaleGet(obj->mdlId[i], &pos3D);
        pos3D.x = pos3D.y = pos3D.z =
            fn_1_160C(pos3D.x, 0.4f, 3.0f);
        Hu3DModelScaleSetV(obj->mdlId[i], &pos3D);
    }
    fn_1_AB64();
}

void fn_1_B4C8(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 4; i++) {
        if (i == 0) {
            obj->mdlId[i] = Hu3DModelCreate(
                HuDataSelHeapReadNum(
                    DATANUM(DATA_mdparty, 0x1A), HU_MEMNUM_OVL,
                    HEAP_MODEL));
            obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[0]);
        } else {
            obj->mdlId[i] = Hu3DModelLink(obj->mdlId[0]);
        }
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[0],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        fn_1_AF70(i);
    }
    fn_1_AC88();
    obj->objFunc = fn_1_B008;
}

void fn_1_B6C0(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        Hu3DMotionKill(obj->mtnId[0]);
        for (i = 3; i >= 0; i--) {
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

inline void fn_1_B6C0(OMOBJ *obj);

s16 fn_1_B748(s16 arg0, s16 arg1, s16 arg2, s16 arg3,
    LBL_1_DATA_E12_ENTRY *arg4, s16 arg5)
{
    s16 result;
    s16 j;
    s16 i;
    s16 k;
    s16 l;

    if (arg1 == -1) {
        return -1;
    }
    result = arg4[arg0].unk_0[arg1];
    if (result == -1) {
        s16 routed;

        if (arg2 == -1) {
            routed = -1;
        } else {
            s16 next;

            next = arg4[arg0].unk_0[arg2];
            if (next == -1) {
                next = fn_1_B748(
                    arg0, arg3, -1, -1, arg4, arg5);
            }
            for (i = 0; i < arg5; i++) {
                if (arg0 != i && arg4[i].unk_10 != -1
                    && next == i) {
                    next = fn_1_B748(
                        next, arg2, arg3, -1, arg4, arg5);
                    if (next == -1) {
                        next = fn_1_B748(
                            arg0, arg3, -1, -1, arg4, arg5);
                    }
                }
            }
            routed = next;
        }
        result = routed;
    }

    for (j = 0; j < arg5; j++) {
        if (arg0 != j && arg4[j].unk_10 != -1 && result == j) {
            s16 routed;

            if (arg1 == -1) {
                routed = -1;
            } else {
                s16 next;

                next = arg4[result].unk_0[arg1];
                if (next == -1) {
                    next = fn_1_B748(
                        result, arg2, arg3, -1, arg4, arg5);
                }
                for (k = 0; k < arg5; k++) {
                    if (result != k && arg4[k].unk_10 != -1
                        && next == k) {
                        next = fn_1_B748(
                            next, arg1, arg2, arg3, arg4, arg5);
                        if (next == -1) {
                            next = fn_1_B748(
                                result, arg2, arg3, -1, arg4, arg5);
                        }
                    }
                }
                routed = next;
            }
            result = routed;
            if (result == -1) {
                s16 fallback;

                if (arg2 == -1) {
                    fallback = -1;
                } else {
                    s16 next;

                    next = arg4[arg0].unk_0[arg2];
                    if (next == -1) {
                        next = fn_1_B748(
                            arg0, arg3, -1, -1, arg4, arg5);
                    }
                    for (l = 0; l < arg5; l++) {
                        if (arg0 != l && arg4[l].unk_10 != -1
                            && next == l) {
                            next = fn_1_B748(
                                next, arg2, arg3, -1, arg4, arg5);
                            if (next == -1) {
                                next = fn_1_B748(
                                    arg0, arg3, -1, -1, arg4, arg5);
                            }
                        }
                    }
                    fallback = next;
                }
                result = fallback;
            }
        }
    }
    return result;
}

#pragma auto_inline off
s16 fn_1_BB30(s16 arg0, LBL_1_DATA_E12_ENTRY *arg1,
    s16 arg2, s16 arg3)
{
    s16 order[12][3] = {
        { 2, 1, 3 },
        { 6, 7, 5 },
        { 0, 7, 1 },
        { 4, 5, 3 },
        { 1, 2, 0 },
        { 3, 2, 4 },
        { 7, 6, 0 },
        { 5, 6, 4 },
        { 1, 0, 2 },
        { 7, 0, 6 },
        { 3, 4, 2 },
        { 5, 4, 6 },
    };

    return fn_1_B748(arg0, order[arg2][0], order[arg2][1],
        order[arg2][2], arg1, arg3);
}
#pragma auto_inline reset

s16 fn_1_BBD8(s16 padNo, s16 *current,
    LBL_1_DATA_E12_ENTRY *entries, s16 count)
{
    s16 next = 0;
    s16 directionNo = 0;
    HuVecF direction = { 0.0f, 0.0f, 0.0f };

    if (HuPadDStkRep[padNo] & PAD_BUTTON_UP) {
        direction.y = 1.0f;
    } else if (HuPadDStkRep[padNo] & PAD_BUTTON_DOWN) {
        direction.y = -1.0f;
    }
    if (HuPadDStkRep[padNo] & PAD_BUTTON_LEFT) {
        direction.x = -1.0f;
    } else if (HuPadDStkRep[padNo] & PAD_BUTTON_RIGHT) {
        direction.x = 1.0f;
    }
    if (HuAbs(direction.x) < 0.25f
        && HuAbs(direction.y) < 0.25f) {
        return FALSE;
    }

    direction.z = sqrtf(
        (direction.x * direction.x) + (direction.y * direction.y));
    direction.x /= direction.z;
    direction.y /= direction.z;
    if (HuAbs(direction.y) < 0.25f) {
        if (direction.x > 0.0f) {
            directionNo = 0;
        } else {
            directionNo = 1;
        }
    } else if (HuAbs(direction.x) < 0.25f) {
        if (direction.y > 0.0f) {
            directionNo = 2;
        } else {
            directionNo = 3;
        }
    } else if (HuAbs(direction.x) > HuAbs(direction.y)) {
        if (direction.x > 0.0f) {
            if (direction.y > 0.0f) {
                directionNo = 4;
            } else {
                directionNo = 5;
            }
        } else if (direction.y > 0.0f) {
            directionNo = 6;
        } else {
            directionNo = 7;
        }
    } else if (direction.y > 0.0f) {
        if (direction.x > 0.0f) {
            directionNo = 8;
        } else {
            directionNo = 9;
        }
    } else if (direction.x > 0.0f) {
        directionNo = 10;
    } else {
        directionNo = 11;
    }

    if ((next = fn_1_BB30(*current, entries, directionNo, count)) != -1) {
        entries[*current].unk_10 = -1;
        *current = next;
        entries[*current].unk_10 = 1;
        HuAudFXPlay(0);
        return TRUE;
    }
    return FALSE;
}

void fn_1_C158(s16 modelNo, s16 animNo, s16 dataNo, s16 bank)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];

    Hu3DAnimAnimSet(entry->animId[animNo], lbl_1_bss_800[dataNo]);
    if (bank != -1) {
        Hu3DAnimBankSet(entry->animId[animNo], bank);
    }
}

inline void fn_1_C158(s16 modelNo, s16 animNo, s16 dataNo, s16 bank);

void fn_1_C200(s16 modelNo, s16 dataNo)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        fn_1_C158(modelNo, i, dataNo, -1);
    }
}

void fn_1_C28C(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];
    float value;

    if (time == 0) {
        if (lbl_1_bss_34 == 0) {
            HuAudFXPlay(0x34);
            lbl_1_bss_34 = 1;
        }
        Hu3DModelPosSet(entry->modelId, pos->x, pos->y, pos->z);
        Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(entry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    } else {
        lbl_1_bss_34 = 0;
    }
    value = fn_1_1868(-540.0f, 0.0f, time, duration);
    Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
    value = fn_1_1868(0.0f, scale, time, duration);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
}

inline void fn_1_C28C(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);

void fn_1_C698(s16 modelNo, s16 time, s16 duration)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];
    float value;

    if (time == 0) {
        if (lbl_1_bss_34 == 0) {
            HuAudFXPlay(0x35);
            lbl_1_bss_34 = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_34 = 0;
    }
    value = fn_1_1868(
        entry->rot.y, entry->rot.y - 540.0f, time, duration);
    Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
    value = fn_1_1868(entry->scale.x, 0.0f, time, duration);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    if (time <= duration) {
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    }
}

inline void fn_1_C698(s16 modelNo, s16 time, s16 duration);

void fn_1_CA68(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];
    HuVecF modelPos;
    float value;

    if (time == 0) {
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    }
    modelPos.x = fn_1_1868(entry->pos.x, pos->x, time, duration);
    modelPos.y = fn_1_1868(entry->pos.y, pos->y, time, duration);
    modelPos.z = fn_1_1868(entry->pos.z, pos->z, time, duration);
    Hu3DModelPosSet(entry->modelId, modelPos.x, modelPos.y, modelPos.z);
    value = fn_1_1868(entry->scale.x, scale, time, duration);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
}

inline void fn_1_CA68(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);

void fn_1_D058(s16 modelNo, s16 dataNo, s16 dataOffset, s16 time,
    s16 duration, s16 direction)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];
    HuVecF rot;

    if (time == 0 || time >= duration) {
        Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
    }
    if (time >= duration) {
        fn_1_C200(modelNo, dataOffset + dataNo);
    } else {
        Hu3DModelRotGet(entry->modelId, &rot);
        rot.y = fn_1_1868(
            0.0f, 180.0f * direction, time, duration);
        if (((rot.y < 0.0f) ? -rot.y : rot.y) >= 90.0f) {
            fn_1_C200(modelNo, dataOffset + dataNo);
        }
        Hu3DModelRotSetV(entry->modelId, &rot);
    }
}

inline void fn_1_D058(s16 modelNo, s16 dataNo, s16 dataOffset, s16 time,
    s16 duration, s16 direction);

void fn_1_D3E0(s16 modelNo, float angle, s16 time, s16 duration)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];
    float rotY;

    if (time == 0) {
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        if (entry->rot.y - angle > 180.0f) {
            entry->rot.y -= 360.0f;
        } else if (entry->rot.y - angle < -180.0f) {
            entry->rot.y += 360.0f;
        }
    }
    rotY = fn_1_1868(entry->rot.y, 0.0f, time, duration);
    Hu3DModelRotSet(entry->modelId, 0.0f, rotY, 0.0f);
}

inline void fn_1_D3E0(
    s16 modelNo, float angle, s16 time, s16 duration);

void fn_1_D648(s16 arg0)
{
    lbl_1_bss_24->mtnId[0] = arg0;
}

s16 fn_1_D660(void)
{
    return lbl_1_bss_24->mtnId[0];
}

s16 fn_1_D678(void)
{
    OMOBJ *obj = lbl_1_bss_24;

    if (obj->work[0] >= 10) {
        return TRUE;
    }
    return FALSE;
}

inline s16 fn_1_D678(void);

void fn_1_D6B0(OMOBJ_FUNC callback)
{
    OMOBJ *obj = lbl_1_bss_24;

    lbl_1_bss_24->mtnId[0] = 0;
    obj->work[0] = 0;
    obj->work[1] = 0;
    obj->work[2] = 0;
    obj->work[3] = 0;
    obj->objFunc = callback;
    while (lbl_1_bss_24->mtnId[0] == 0) {
        HuPrcVSleep();
    }
}

inline void fn_1_D6B0(OMOBJ_FUNC callback);

void fn_1_D748(void)
{
    char *bitmapName[4] = {
        lbl_1_data_DF2,
        lbl_1_data_DFA,
        lbl_1_data_E02,
        lbl_1_data_E0A,
    };
    s16 i;
    s16 j;

    for (i = 0; i < 41; i++) {
        lbl_1_bss_800[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(
                lbl_1_data_800[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 25; i++) {
        memset(&lbl_1_bss_288[i], 0, sizeof(LBL_1_BSS_288_ENTRY));
        if (i == 0) {
            lbl_1_bss_288[i].modelId = Hu3DModelCreate(
                HuDataSelHeapReadNum(0x970024, HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            lbl_1_bss_288[i].modelId =
                Hu3DModelLink(lbl_1_bss_288[0].modelId);
        }
        for (j = 0; j < 4; j++) {
            lbl_1_bss_288[i].animId[j] = Hu3DAnimCreate(
                lbl_1_bss_800[0], lbl_1_bss_288[i].modelId,
                bitmapName[j]);
        }
        Hu3DModelLayerSet(lbl_1_bss_288[i].modelId, 1);
        Hu3DModelAttrSet(
            lbl_1_bss_288[i].modelId, HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_24 =
        omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10, -1, NULL);
}

inline void fn_1_D748(void);

void fn_1_D978(void)
{
    s16 i;
    s16 j;

    if (lbl_1_bss_24) {
        lbl_1_bss_24->mtnId[0] = -1;
        for (i = 24; i >= 0; i--) {
            for (j = 0; j < 4; j++) {
                Hu3DAnimKill(lbl_1_bss_288[i].animId[j]);
            }
            Hu3DModelKill(lbl_1_bss_288[i].modelId);
        }
    }
    lbl_1_bss_24 = NULL;
}

inline void fn_1_D978(void);

void fn_1_DA54(OMOBJ *obj)
{
    if (obj->work[0] == 0) {
        fn_1_884C(lbl_1_bss_9F4.unk_4);
        fn_1_C200(21, lbl_1_bss_9F4.unk_0 + 20);
        fn_1_C200(22, lbl_1_bss_9F4.unk_6 + 22);
        fn_1_C200(23, lbl_1_bss_9F4.unk_8 + 31);
        fn_1_C200(24, lbl_1_bss_9F4.unk_A + 36);
    }
    fn_1_C28C(21, obj->work[0], 10, &lbl_1_data_8A4[41], 1.5f);
    fn_1_C28C(22, obj->work[0], 10, &lbl_1_data_8A4[42], 1.5f);
    fn_1_C28C(23, obj->work[0], 10, &lbl_1_data_8A4[43], 1.5f);
    fn_1_C28C(24, obj->work[0], 10, &lbl_1_data_8A4[44], 1.5f);
    if (obj->work[0]++ > 10) {
        fn_1_8C28(lbl_1_bss_9F4.unk_4);
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_EB74(OMOBJ *obj)
{
    if (obj->work[0] == 0) {
        fn_1_8950();
    }
    fn_1_C698(21, obj->work[0], 10);
    fn_1_C698(22, obj->work[0], 10);
    fn_1_C698(23, obj->work[0], 10);
    fn_1_C698(24, obj->work[0], 10);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_F838(OMOBJ *obj)
{
    if (obj->work[0] == 0) {
        fn_1_8CCC();
    }
    fn_1_C698(21, obj->work[0], 10);
    fn_1_C698(22, obj->work[0], 10);
    fn_1_C698(23, obj->work[0], 10);
    fn_1_C698(24, obj->work[0], 10);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_10518(OMOBJ *obj)
{
    s16 minimum = 1;
    s16 count = 0;
    s16 i;

    for (i = 0; i < 4; i++) {
        if (HuPadStatGet(i) == 0) {
            count++;
        }
    }
    if (count < minimum) {
        count = minimum;
    }
    if (count >= lbl_1_bss_9F4.unk_2) {
        if (obj->work[0]++ > 10) {
            if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
                lbl_1_bss_9F4.unk_2--;
                if (lbl_1_bss_9F4.unk_2 < minimum) {
                    lbl_1_bss_9F4.unk_2 = count;
                }
                obj->work[0] = 0;
                obj->work[1] = 0;
                fn_1_AAF0(4);
            } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
                lbl_1_bss_9F4.unk_2++;
                if (lbl_1_bss_9F4.unk_2 >= count + 1) {
                    lbl_1_bss_9F4.unk_2 = minimum;
                }
                obj->work[0] = 0;
                obj->work[1] = 1;
                fn_1_AAF0(5);
            }
            if (count == minimum) {
                obj->work[0] = 12;
                if (obj->work[3] == 1) {
                    obj->work[3] = 0;
                    fn_1_AA94(4);
                    fn_1_AA94(5);
                }
            } else if (obj->work[3] == 0) {
                obj->work[3] = 1;
                fn_1_A970(4, &lbl_1_data_8A4[0], -180.0f, 0.0f);
                fn_1_A970(5, &lbl_1_data_8A4[0], 180.0f, 0.0f);
            }
        }
    } else {
        lbl_1_bss_9F4.unk_2 = count;
        obj->work[0] = 0;
        obj->work[1] = 0;
    }
    if (obj->work[0] <= 10) {
        for (i = 0; i < 4; i++) {
            if (obj->work[1]) {
                if (i < lbl_1_bss_9F4.unk_2) {
                    fn_1_D058(i + 17, 18, 1,
                        obj->work[0], 10, 1);
                } else {
                    fn_1_D058(i + 17, 19, -1,
                        obj->work[0], 10, 1);
                }
            } else if (i < lbl_1_bss_9F4.unk_2) {
                fn_1_D058(i + 17, 18, 1,
                    obj->work[0], 10, -1);
            } else {
                fn_1_D058(i + 17, 19, -1,
                    obj->work[0], 10, -1);
            }
        }
    }
}

void fn_1_114C4(OMOBJ *obj)
{
    obj->work[0] = 10;
    obj->objFunc = fn_1_10518;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_114F4(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_918[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[4] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_1158C(OMOBJ *obj)
{
    s16 minimum = 1;
    s16 i;

    for (i = 0; i < 4; i++) {
        fn_1_C28C(i + 17, obj->work[0], 10,
            &lbl_1_data_8A4[i + 33], 1.25f);
    }
    if (obj->work[0] == 0) {
        for (i = 0, lbl_1_bss_9F4.unk_2 = 0; i < 4; i++) {
            if (HuPadStatGet(i) == 0) {
                lbl_1_bss_9F4.unk_2++;
            }
        }
        if (lbl_1_bss_9F4.unk_2 < minimum) {
            lbl_1_bss_9F4.unk_2 = minimum;
        }
        for (i = 0; i < 4; i++) {
            if (i < lbl_1_bss_9F4.unk_2) {
                fn_1_C200(i + 17, 19);
            } else {
                fn_1_C200(i + 17, 18);
            }
        }
    }
    for (i = 0; i < 4; i++) {
        fn_1_C28C(i + 17, obj->work[0], 10,
            &lbl_1_data_8A4[i + 33], 1.25f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_11EAC(OMOBJ *obj)
{
    LBL_1_BSS_288_ENTRY *entry;
    float value;
    s16 i;
    s16 time;

    for (i = 0; i < 4; i++) {
        time = obj->work[0];
        entry = &lbl_1_bss_288[(s16)(i + 17)];
        if (time == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(0x35);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_34 = 0;
        }
        value = fn_1_1868(
            entry->rot.y, entry->rot.y - 540.0f, time, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_1868(entry->scale.x, 0.0f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
        if (time <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_12260(s16 arg0)
{
    HuVecF rot;
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[arg0];

    Hu3DModelRotGet(entry->modelId, &rot);
    rot.y += 6.0f;
    if (rot.y > 360.0f) {
        rot.y -= 360.0f;
    }
    Hu3DModelRotSetV(entry->modelId, &rot);
}

void fn_1_122FC(s16 arg0)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[arg0];

    Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
}

inline void fn_1_122FC(s16 arg0);

void fn_1_12364(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->unk_4 == 0) {
            if (entry->unk_0 == 0) {
                if (obj->work[i]++ > 10) {
                    if (fn_1_BBD8(entry->unk_A, &entry->chrSel,
                            lbl_1_data_E12, 11)) {
                        fn_1_AD14(
                            i, &lbl_1_data_8A4[entry->chrSel + 14]);
                        obj->work[i] = 0;
                    } else if (HuPadBtnDown[entry->unk_A]
                        & PAD_BUTTON_A) {
                        if (lbl_1_bss_2A == 0 && entry->chrSel == 10) {
                            HuAudFXPlay(4);
                            continue;
                        }
                        CharFXPlay(entry->chrSel, 0x24B);
                        entry->unk_0 = 1;
                        fn_1_AF70(i);
                        fn_1_9A7C(i, entry->unk_A);
                        fn_1_C158(
                            (s16)entry->chrSel, 0, (s16)entry->chrSel, 1);
                        fn_1_C158(
                            (s16)entry->chrSel, 2, (s16)entry->chrSel, 1);
                        fn_1_45C40(i,
                            &lbl_1_data_8A4[entry->chrSel + 14],
                            entry->unk_A);
                    }
                }
            } else if (HuPadBtnDown[entry->unk_A] & PAD_BUTTON_B) {
                HuAudFXPlay(3);
                entry->unk_0 = 0;
                fn_1_AD9C(i, &lbl_1_data_8A4[entry->chrSel + 14],
                    entry->unk_A);
                fn_1_9B68(i);
                fn_1_C158((s16)entry->chrSel, 0,
                    (s16)entry->chrSel, 0);
                fn_1_C158((s16)entry->chrSel, 2,
                    (s16)entry->chrSel, 0);
            }
        }
    }
    for (i = 0; i < 11; i++) {
        for (j = 0, entry = lbl_1_bss_9BC; j < 4;
             j++, entry++) {
            if (entry->unk_4 == 0 && entry->chrSel == i
                && entry->unk_0 == 0) {
                break;
            }
        }
        if (j == 4) {
            fn_1_122FC(i);
        } else {
            fn_1_12260(i);
        }
    }
}

void fn_1_12B18(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *entry;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->unk_4 == 0 && entry->unk_0 == 0) {
            lbl_1_data_E12[entry->chrSel].unk_10 = 1;
            fn_1_AD9C(
                i, &lbl_1_data_8A4[entry->chrSel + 14], entry->unk_A);
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_12364;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_12D7C(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *entry;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->unk_4 == 0 && entry->unk_0 == 0) {
            fn_1_AF70(i);
        }
    }
    for (i = 0; i < 11; i++) {
        fn_1_122FC(i);
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_12EC8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry = &lbl_1_bss_9BC[0];

    entry->unk_0 = 0;
    lbl_1_bss_D4[0].x = 0.0f;
    fn_1_C158((s16)entry->chrSel, 0, (s16)entry->chrSel, 0);
    fn_1_C158((s16)entry->chrSel, 2, (s16)entry->chrSel, 0);
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_12FD8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    HuVecF pos;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->unk_4 == 1 && entry->unk_0 != 2) {
            if (entry->unk_0 == 0) {
                if (obj->work[0]++ > 10) {
                    if (fn_1_BBD8(
                            0, &entry->chrSel, lbl_1_data_E12, 11)) {
                        fn_1_AD14(
                            i, &lbl_1_data_8A4[entry->chrSel + 14]);
                        obj->work[0] = 0;
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                        if (lbl_1_bss_2A == 0 && entry->chrSel == 10) {
                            HuAudFXPlay(4);
                            break;
                        }
                        CharFXPlay(entry->chrSel, 0x24B);
                        entry->unk_0 = 1;
                        entry->unk_6 = 1;
                        fn_1_AF70(i);
                        fn_1_9A7C(i, entry->unk_6 + 5);
                        fn_1_C158(
                            (s16)entry->chrSel, 0, (s16)entry->chrSel, 1);
                        fn_1_C158(
                            (s16)entry->chrSel, 2, (s16)entry->chrSel, 1);
                        Hu3DModelPosGet(
                            lbl_1_bss_288[entry->chrSel].modelId, &pos);
                        fn_1_A970(4, &pos, -40.0f, -45.0f);
                        fn_1_A970(5, &pos, 40.0f, -45.0f);
                        fn_1_45C40(i,
                            &lbl_1_data_8A4[entry->chrSel + 14], 4);
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                        HuAudFXPlay(3);
                        lbl_1_data_E12[entry->chrSel].unk_10 = -1;
                        fn_1_AF70(i);
                        for (j = 3; j >= 0; j--) {
                            if (lbl_1_bss_9BC[j].unk_4 == 1
                                && lbl_1_bss_9BC[j].unk_0 == 2) {
                                lbl_1_bss_9BC[j].unk_0 = 0;
                                lbl_1_data_E12[
                                    lbl_1_bss_9BC[j].chrSel]
                                    .unk_10 = -1;
                                fn_1_9B68(j);
                                fn_1_C158((s16)lbl_1_bss_9BC[j].chrSel, 0,
                                    (s16)lbl_1_bss_9BC[j].chrSel, 0);
                                fn_1_C158((s16)lbl_1_bss_9BC[j].chrSel, 2,
                                    (s16)lbl_1_bss_9BC[j].chrSel, 0);
                                break;
                            }
                        }
                        if (j != -1) {
                            obj->objFunc = fn_1_13EC0;
                        } else {
                            obj->objFunc = NULL;
                        }
                    }
                }
            } else if (entry->unk_0 == 1) {
                if (obj->work[0]++ > 10) {
                    if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
                        entry->unk_6--;
                        if (lbl_1_bss_2C == 0) {
                            if (entry->unk_6 < 0) {
                                entry->unk_6 += 3;
                            }
                        } else {
                            if (entry->unk_6 < 0) {
                                entry->unk_6 += 4;
                            }
                        }
                        obj->work[0] = 0;
                        fn_1_AAF0(4);
                    } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
                        entry->unk_6++;
                        if (lbl_1_bss_2C == 0) {
                            if (entry->unk_6 > 2) {
                                entry->unk_6 -= 3;
                            }
                        } else if (entry->unk_6 > 3) {
                            entry->unk_6 -= 4;
                        }
                        obj->work[0] = 0;
                        fn_1_AAF0(5);
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                        HuAudFXPlay(1);
                        entry->unk_0 = 2;
                        fn_1_AA94(4);
                        fn_1_AA94(5);
                        for (j = 0; j < 4; j++) {
                            if (lbl_1_bss_9BC[j].unk_4 == 1
                                && lbl_1_bss_9BC[j].unk_0 == 0) {
                                break;
                            }
                        }
                        if (j != 4) {
                            obj->objFunc = fn_1_13EC0;
                        } else {
                            obj->objFunc = NULL;
                        }
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                        HuAudFXPlay(3);
                        entry->unk_0 = 0;
                        fn_1_AA94(4);
                        fn_1_AA94(5);
                        fn_1_C158(
                            (s16)entry->chrSel, 0, (s16)entry->chrSel, 0);
                        fn_1_C158(
                            (s16)entry->chrSel, 2, (s16)entry->chrSel, 0);
                        fn_1_AD9C(
                            i, &lbl_1_data_8A4[entry->chrSel + 14], 4);
                        fn_1_9B68(i);
                    }
                }
                fn_1_9B90(i, entry->unk_6 + 5);
            }
            break;
        }
    }
    for (i = 0; i < 11; i++) {
        if (entry->chrSel == i && entry->unk_0 == 0) {
            fn_1_12260(i);
        } else {
            fn_1_122FC(i);
        }
    }
}

void fn_1_13EC0(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    HuVecF pos;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->unk_4 == 1) {
            if (entry->unk_0 == 0) {
                for (j = 0; j < 11; j++) {
                    if (lbl_1_data_E12[j].unk_10 == -1) {
                        entry->chrSel = j;
                        lbl_1_data_E12[j].unk_10 = 1;
                        break;
                    }
                }
                fn_1_AD9C(
                    i, &lbl_1_data_8A4[entry->chrSel + 14], 4);
                break;
            } else if (entry->unk_0 == 1) {
                Hu3DModelPosGet(
                    lbl_1_bss_288[entry->chrSel].modelId, &pos);
                fn_1_A970(4, &pos, -40.0f, -45.0f);
                fn_1_A970(5, &pos, 40.0f, -45.0f);
                break;
            }
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_12FD8;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_14344(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *entry;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->unk_4 == 1) {
            if (entry->unk_0 == 0) {
                lbl_1_data_E12[entry->chrSel].unk_10 = -1;
                fn_1_AF70(i);
                break;
            } else if (entry->unk_0 == 1) {
                fn_1_AA94(4);
                fn_1_AA94(5);
                break;
            }
        }
    }
    for (i = 0; i < 11; i++) {
        fn_1_122FC(i);
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_14510(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->unk_4 == 1 && entry->unk_0 != 1) {
            if (entry->unk_0 == 0) {
                if (obj->work[0]++ > 10) {
                    if (fn_1_BBD8(
                            0, &entry->chrSel, lbl_1_data_E12, 11)) {
                        fn_1_AD14(
                            i, &lbl_1_data_8A4[entry->chrSel + 14]);
                        obj->work[0] = 0;
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                        if (lbl_1_bss_2A == 0 && entry->chrSel == 10) {
                            HuAudFXPlay(4);
                            break;
                        }
                        CharFXPlay(entry->chrSel, 0x24B);
                        entry->unk_0 = 1;
                        entry->unk_6 = 1;
                        fn_1_AF70(i);
                        fn_1_9A7C(i, 4);
                        fn_1_C158(
                            (s16)entry->chrSel, 0, (s16)entry->chrSel, 1);
                        fn_1_C158(
                            (s16)entry->chrSel, 2, (s16)entry->chrSel, 1);
                        fn_1_45C40(i,
                            &lbl_1_data_8A4[entry->chrSel + 14], 4);
                        for (j = 0; j < 4; j++) {
                            if (lbl_1_bss_9BC[j].unk_4 == 1
                                && lbl_1_bss_9BC[j].unk_0 == 0) {
                                break;
                            }
                        }
                        if (j != 4) {
                            obj->objFunc = fn_1_14CC0;
                        } else {
                            obj->objFunc = NULL;
                        }
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                        HuAudFXPlay(3);
                        lbl_1_data_E12[entry->chrSel].unk_10 = -1;
                        fn_1_AF70(i);
                        for (j = 3; j >= 0; j--) {
                            if (lbl_1_bss_9BC[j].unk_4 == 1
                                && lbl_1_bss_9BC[j].unk_0 == 1) {
                                lbl_1_bss_9BC[j].unk_0 = 0;
                                lbl_1_data_E12[
                                    lbl_1_bss_9BC[j].chrSel]
                                    .unk_10 = -1;
                                fn_1_9B68(j);
                                fn_1_C158((s16)lbl_1_bss_9BC[j].chrSel, 0,
                                    (s16)lbl_1_bss_9BC[j].chrSel, 0);
                                fn_1_C158((s16)lbl_1_bss_9BC[j].chrSel, 2,
                                    (s16)lbl_1_bss_9BC[j].chrSel, 0);
                                break;
                            }
                        }
                        if (j != -1) {
                            obj->objFunc = fn_1_14CC0;
                        } else {
                            obj->objFunc = NULL;
                        }
                    }
                }
            }
            break;
        }
    }
    for (i = 0; i < 11; i++) {
        if (entry->chrSel == i && entry->unk_0 == 0) {
            fn_1_12260(i);
        } else {
            fn_1_122FC(i);
        }
    }
}

void fn_1_14CC0(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->unk_4 == 1 && entry->unk_0 == 0) {
            for (j = 0; j < 11; j++) {
                if (lbl_1_data_E12[j].unk_10 == -1) {
                    entry->chrSel = j;
                    lbl_1_data_E12[j].unk_10 = 1;
                    break;
                }
            }
            fn_1_AD9C(
                i, &lbl_1_data_8A4[entry->chrSel + 14], 4);
            break;
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_14510;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_14F64(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *entry;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->unk_4 == 1 && entry->unk_0 == 0) {
            lbl_1_data_E12[entry->chrSel].unk_10 = -1;
            fn_1_AF70(i);
            break;
        }
    }
    for (i = 0; i < 11; i++) {
        fn_1_122FC(i);
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_150D0(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    OMOBJ *workObj;
    float value;
    s16 i;
    s16 j;
    s16 time;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[2] = 2;
        workObj->work[3] = 0;
        for (i = 0; i < 11; i++) {
            fn_1_C200(i, i);
            fn_1_C158(i, 1, i, 2);
            fn_1_C158(i, 3, i, 2);
            lbl_1_data_E12[i].unk_10 = -1;
        }
        if (lbl_1_bss_2A == 0) {
            fn_1_C200(10, 11);
        }
        for (j = 0, player = lbl_1_bss_9BC; j < 4;
             j++, player++) {
            player->unk_0 = 0;
            player->unk_2 = 0;
            player->unk_6 = 0;
            player->chrSel = j;
            player->unk_A = j;
        }
    }
    for (i = 0; i < 11; i++) {
        time = obj->work[0];
        entry = &lbl_1_bss_288[i];
        if (time == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(0x34);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosSet(entry->modelId,
                lbl_1_data_8A4[i + 14].x,
                lbl_1_data_8A4[i + 14].y,
                lbl_1_data_8A4[i + 14].z);
            Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(entry->modelId, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        } else {
            lbl_1_bss_34 = 0;
        }
        value = fn_1_1868(-540.0f, 0.0f, time, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_1868(0.0f, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_156B4(OMOBJ *obj)
{
    s16 i;
    s16 j;

    for (i = 0; i < 11; i++) {
        for (j = 0; j < 4; j++) {
            if (i == lbl_1_bss_9BC[j].chrSel) {
                break;
            }
        }
        if (j == 4) {
            fn_1_C698(i, obj->work[0], 10);
        }
    }
    if (obj->work[0] >= 10) {
        for (i = 0; i < 4; i++) {
            fn_1_CA68((s16)lbl_1_bss_9BC[i].chrSel,
                obj->work[0] - 10, 10, &lbl_1_data_8A4[i + 45], 1.25f);
        }
    }
    if (obj->work[0]++ > 20) {
        fn_1_884C(lbl_1_bss_9F4.unk_4);
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_160A0(OMOBJ *obj)
{
    s16 i;
    s16 j;

    for (i = 0; i < 11; i++) {
        for (j = 0; j < 4; j++) {
            if (i == lbl_1_bss_9BC[j].chrSel) {
                break;
            }
        }
        if (j == 4) {
            fn_1_C698(i, obj->work[0], 10);
        }
    }
    if (obj->work[0] >= 10) {
        fn_1_884C(lbl_1_bss_9F4.unk_4);
        for (i = 0; i < 4; i++) {
            fn_1_CA68((s16)lbl_1_bss_9BC[i].chrSel,
                obj->work[0] - 10, 10, &lbl_1_data_8A4[i + 49], 1.25f);
        }
    }
    if (obj->work[0]++ > 20) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_16A8C(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    if (obj->work[0] == 0) {
        for (i = 0; i < 4; i++) {
            lbl_1_bss_D4[i].x = 0.0f;
        }
    }
    for (i = 0, player = lbl_1_bss_9BC; i < 4; i++, player++) {
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(0x35);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_34 = 0;
        }
        value = fn_1_1868(
            entry->rot.y, entry->rot.y - 540.0f, time, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_1868(entry->scale.x, 0.0f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
        if (time <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_16E84(OMOBJ *obj)
{
    LBL_1_BSS_288_ENTRY *entry;
    OMOBJ *workObj;
    float value;
    s16 memberNo;
    s16 i;
    s16 time;

    if (obj->work[0] == 0) {
        memberNo = lbl_1_bss_9F4.unk_4;
        workObj = lbl_1_bss_8;
        HuSprGrpPosSet(lbl_1_bss_918[1], 288.0f, 60.0f);
        HuSprGrpTPLvlSet(lbl_1_bss_918[1], 0.0f);
        fn_1_4734(lbl_1_bss_918[1], HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(
            lbl_1_bss_918[1], memberNo, HUSPR_ATTR_DISPOFF);
        workObj->work[2] = 1;
        workObj->work[3] = 0;
        for (i = 0; i < 4; i++) {
            lbl_1_bss_D4[i].x = 0.0f;
        }
    }
    for (i = 0; i < 11; i++) {
        time = obj->work[0];
        entry = &lbl_1_bss_288[i];
        if (time == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(0x35);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_34 = 0;
        }
        value = fn_1_1868(
            entry->rot.y, entry->rot.y - 540.0f, time, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_1868(entry->scale.x, 0.0f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
        if (time <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_1733C(s16 arg0)
{
    HuVecF value;
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[arg0 + 11];

    Hu3DModelPosGet(entry->modelId, &value);
    fn_1_163C(&value, &lbl_1_data_8A4[arg0 + 7], 5.0f);
    Hu3DModelPosSetV(entry->modelId, &value);
    if (entry->unk_30++ > 30) {
        entry->unk_30 = 35;
        Hu3DModelRotGet(entry->modelId, &value);
        value.y += 3.0f;
        if (value.y >= 360.0f) {
            value.y -= 360.0f;
        }
        Hu3DModelRotSetV(entry->modelId, &value);
    }
    Hu3DModelScaleGet(entry->modelId, &value);
    value.y = value.x = fn_1_160C(value.x, 2.25f, 5.0f);
    Hu3DModelScaleSetV(entry->modelId, &value);
    Hu3DModelLayerSet(entry->modelId, 2);
}

void fn_1_1765C(s16 arg0)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[arg0 + 11];

    Hu3DModelPosSetV(entry->modelId, &lbl_1_data_8A4[arg0 + 1]);
    Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(entry->modelId, 1.25f, 1.25f, 1.0f);
    entry->unk_30 = 0;
    Hu3DModelLayerSet(entry->modelId, 1);
}

void fn_1_17730(OMOBJ *obj)
{
    s16 i;

    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            OMOBJ *workObj;
            s16 memberNo;

            lbl_1_bss_9F4.unk_4--;
            if (lbl_1_bss_9F4.unk_4 < 0) {
                lbl_1_bss_9F4.unk_4 += 6;
            }
            obj->work[0] = 0;
            obj->work[1] = 0;
            memberNo = lbl_1_bss_9F4.unk_4;
            workObj = lbl_1_bss_8;
            HuSprGrpPosSet(lbl_1_bss_918[1], 288.0f, 60.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_918[1], 0.0f);
            fn_1_4734(lbl_1_bss_918[1], HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(
                lbl_1_bss_918[1], memberNo, HUSPR_ATTR_DISPOFF);
            workObj->work[2] = 1;
            workObj->work[3] = 0;
            lbl_1_bss_38[0] = 2.0f;
            if (lbl_1_data_DE6[0] == 1) {
                HuAudFXPlay(0);
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            OMOBJ *workObj;
            s16 memberNo;

            lbl_1_bss_9F4.unk_4++;
            if (lbl_1_bss_9F4.unk_4 >= 6) {
                lbl_1_bss_9F4.unk_4 -= 6;
            }
            obj->work[0] = 0;
            obj->work[1] = 1;
            memberNo = lbl_1_bss_9F4.unk_4;
            workObj = lbl_1_bss_8;
            HuSprGrpPosSet(lbl_1_bss_918[1], 288.0f, 60.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_918[1], 0.0f);
            fn_1_4734(lbl_1_bss_918[1], HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(
                lbl_1_bss_918[1], memberNo, HUSPR_ATTR_DISPOFF);
            workObj->work[2] = 1;
            workObj->work[3] = 0;
            lbl_1_bss_38[1] = 2.0f;
            if (lbl_1_data_DE6[1] == 1) {
                HuAudFXPlay(0);
            }
        }
    }
    for (i = 0; i < 6; i++) {
        if (i == lbl_1_bss_9F4.unk_4) {
            fn_1_1733C(i);
        } else {
            fn_1_1765C(i);
        }
    }
}

void fn_1_17E10(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 6; i++) {
        lbl_1_bss_288[i + 11].unk_30 = 0;
    }
    fn_1_A970(0, &lbl_1_data_8A4[0], -240.0f, 0.0f);
    fn_1_A970(1, &lbl_1_data_8A4[0], 240.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_17730;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_1807C(OMOBJ *obj)
{
    s16 i;

    if (obj->work[0] == 0) {
        fn_1_AA94(0);
        fn_1_AA94(1);
    }
    for (i = 0; i < 6; i++) {
        lbl_1_bss_288[i + 11].unk_30 = 0;
        fn_1_D3E0(i + 11, 0.0f, obj->work[0], 10);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_18384(OMOBJ *obj)
{
    s16 i;

    lbl_1_bss_9F4.unk_4 = 0;
    if (obj->work[0] == 0) {
        for (i = 0; i < 6; i++) {
            fn_1_C200(i + 11, i + 12);
            fn_1_C158(i + 11, 1, i + 12, 1);
            fn_1_C158(i + 11, 3, i + 12, 1);
            if (lbl_1_data_BEE[i] == 0) {
                fn_1_C158(i + 11, 2, 40, -1);
                fn_1_C158(i + 11, 3, 40, -1);
            }
        }
    }
    for (i = 0; i < 6; i++) {
        fn_1_C28C(
            i + 11, obj->work[0], 10, &lbl_1_data_8A4[i + 1], 1.25f);
    }
    if (obj->work[0]++ > 10) {
        Hu3DZClearLayerSet(2);
        fn_1_884C(lbl_1_bss_9F4.unk_4);
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_18A10(OMOBJ *obj)
{
    OMOBJ *workObj;
    OMOBJ *workObj2;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[0] = 2;
        workObj->work[1] = 0;
        workObj2 = lbl_1_bss_8;
        workObj2->work[2] = 2;
        workObj2->work[3] = 0;
    }
    if (obj->work[0]++ > 30) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_18AA4(OMOBJ *obj)
{
    HuVecF rot;
    s16 i;
    s16 time;

    for (i = 0; i < 6; i++) {
        if (i != lbl_1_bss_9F4.unk_4) {
            fn_1_C698(i + 11, obj->work[0], 10);
        }
    }
    if (obj->work[0] >= 10) {
        time = obj->work[0] - 10;
        fn_1_CA68(lbl_1_bss_9F4.unk_4 + 11, time, 10,
            &lbl_1_data_8A4[13], 1.5f);
        Hu3DModelRotGet(
            lbl_1_bss_288[lbl_1_bss_9F4.unk_4 + 11].modelId, &rot);
        rot.y += 54.0f;
        Hu3DModelRotSetV(
            lbl_1_bss_288[lbl_1_bss_9F4.unk_4 + 11].modelId, &rot);
    }
    if (obj->work[0] == 20) {
        Hu3DLayerHookReset(2);
        fn_1_45A48(&lbl_1_data_8A4[13]);
        HuAudFXPlay(0x49E);
        fn_1_8C28(lbl_1_bss_9F4.unk_4);
        Hu3DModelAttrSet(
            lbl_1_bss_288[lbl_1_bss_9F4.unk_4 + 11].modelId,
            HU3D_ATTR_DISPOFF);
    }
    if (obj->work[0]++ > 90) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_1948C(OMOBJ *obj)
{
    LBL_1_BSS_288_ENTRY *entry;
    s16 i;
    OMOBJ *workObj;
    float value;
    s16 time;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[2] = 2;
        workObj->work[3] = 0;
    }
    for (i = 0; i < 6; i++) {
        time = obj->work[0];
        entry = &lbl_1_bss_288[(s16)(i + 11)];
        if (time == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(0x35);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_34 = 0;
        }
        value = fn_1_1868(
            entry->rot.y, entry->rot.y - 540.0f, time, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_1868(entry->scale.x, 0.0f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
        if (time <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        Hu3DLayerHookReset(2);
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_19860(OMOBJ *obj)
{
    HuVecF pos;

    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            lbl_1_bss_9F4.unk_C--;
            if (lbl_1_bss_9F4.unk_C < 0) {
                lbl_1_bss_9F4.unk_C += 4;
            }
            obj->work[0] = 0;
            fn_1_AAF0(4);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            lbl_1_bss_9F4.unk_C++;
            if (lbl_1_bss_9F4.unk_C > 3) {
                lbl_1_bss_9F4.unk_C -= 4;
            }
            obj->work[0] = 0;
            fn_1_AAF0(5);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C--;
            if (lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C < 0) {
                lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C += 10;
            }
            obj->work[0] = 0;
            fn_1_AAF0(3);
            fn_1_9CF0(lbl_1_bss_9F4.unk_C,
                lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C++;
            if (lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C > 9) {
                lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C -= 10;
            }
            obj->work[0] = 0;
            fn_1_AAF0(2);
            fn_1_9CF0(lbl_1_bss_9F4.unk_C,
                lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C);
        }
    }
    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].chrSel]
            .modelId,
        &pos);
    fn_1_A8D8(2, &pos, 0.0f, -80.0f);
    fn_1_A8D8(3, &pos, 0.0f, 80.0f);
}

void fn_1_19D24(OMOBJ *obj)
{
    HuVecF pos;

    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].chrSel]
            .modelId,
        &pos);
    fn_1_A970(2, &pos, 0.0f, -80.0f);
    fn_1_A970(3, &pos, 0.0f, 80.0f);
    pos.x = 0.0f;
    fn_1_A970(4, &pos, -180.0f, 0.0f);
    fn_1_A970(5, &pos, 180.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_19860;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_1A124(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_918[2], 2, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[2] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 3, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[3] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[4] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_1A20C(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 bank;
    s16 i;
    s16 time;

    lbl_1_bss_9F4.unk_C = 0;
    for (i = 0, player = lbl_1_bss_9BC; i < 4; i++, player++) {
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 45].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 45].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 45].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        for (i = 0; i < 4; i++) {
            lbl_1_bss_9BC[i].unk_C = 0;
            bank = lbl_1_bss_9BC[i].unk_C;
            lbl_1_bss_A4[i].x = 0.85f;
            fn_1_47B4(
                lbl_1_bss_918[i + 8], HUSPR_ATTR_DISPOFF);
            HuSprBankSet(lbl_1_bss_918[i + 8], 2,
                bank);
        }
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_1A8B0(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    lbl_1_bss_9F4.unk_C = 0;
    for (i = 0, player = lbl_1_bss_9BC; i < 4; i++, player++) {
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 45].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 45].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 45].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_1AE60(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    for (i = 0, player = lbl_1_bss_9BC; i < 4; i++, player++) {
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 29].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 29].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 29].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_1B400(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float scale;
    s16 i;
    s16 time;
    s32 modelNo;

    if (obj->work[0] == 0) {
        for (i = 0; i < 4; i++) {
            lbl_1_bss_A4[i].x = 0.0f;
        }
    }
    for (i = 0, player = lbl_1_bss_9BC; i < 4; i++, player++) {
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 45].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 45].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 45].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        scale = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, scale, scale, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_1B9E8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;

    fn_1_CA68(21, obj->work[0], 10,
        &lbl_1_data_8A4[41], 1.5f);
    fn_1_CA68(22, obj->work[0], 10,
        &lbl_1_data_8A4[42], 1.5f);
    fn_1_CA68(23, obj->work[0], 10,
        &lbl_1_data_8A4[43], 1.5f);
    fn_1_CA68(24, obj->work[0], 10,
        &lbl_1_data_8A4[44], 1.5f);
    if (lbl_1_bss_9F4.unk_0 == 0) {
        for (i = 0, entry = lbl_1_bss_9BC;
             i < 4; i++, entry++) {
            fn_1_CA68((s16)entry->chrSel, obj->work[0], 10,
                &lbl_1_data_8A4[i + 29], 1.25f);
        }
    } else {
        for (i = 0; i < 4; i++) {
            entry = &lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
            fn_1_CA68((s16)entry->chrSel, obj->work[0], 10,
                &lbl_1_data_8A4[i + 57], 1.25f);
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_1D758(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    OMOBJ *workObj;
    s16 i;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[2] = 2;
        workObj->work[3] = 0;
        for (i = 0; i < 4; i++) {
            lbl_1_bss_D4[i].x = 0.0f;
            lbl_1_bss_A4[i].x = 0.0f;
        }
        lbl_1_bss_8C[0].x = 0.0f;
        lbl_1_bss_8C[1].x = 0.0f;
        lbl_1_bss_80.x = 0.0f;
    }
    fn_1_C698(21, obj->work[0], 10);
    fn_1_C698(22, obj->work[0], 10);
    fn_1_C698(23, obj->work[0], 10);
    fn_1_C698(24, obj->work[0], 10);
    for (i = 0, entry = lbl_1_bss_9BC;
         i < 4; i++, entry++) {
        fn_1_C698((s16)entry->chrSel, obj->work[0], 10);
    }
    if (obj->work[0]++ > 10) {
        fn_1_7C88();
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_1E7F8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    OMOBJ *workObj;
    OMOBJ *workObj2;
    s16 i;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[0] = 2;
        workObj->work[1] = 0;
        workObj2 = lbl_1_bss_8;
        workObj2->work[2] = 2;
        workObj2->work[3] = 0;
        for (i = 0; i < 4; i++) {
            lbl_1_bss_D4[i].x = 0.0f;
            lbl_1_bss_A4[i].x = 0.0f;
        }
        lbl_1_bss_8C[0].x = 0.0f;
        lbl_1_bss_8C[1].x = 0.0f;
        lbl_1_bss_80.x = 0.0f;
    }
    fn_1_C698(21, obj->work[0], 10);
    fn_1_C698(22, obj->work[0], 10);
    fn_1_C698(23, obj->work[0], 10);
    fn_1_C698(24, obj->work[0], 10);
    for (i = 0, entry = lbl_1_bss_9BC;
         i < 4; i++, entry++) {
        fn_1_C698((s16)entry->chrSel, obj->work[0], 10);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_1F8B8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;

    fn_1_CA68(21, obj->work[0], 10,
        &lbl_1_data_8A4[37], 1.0f);
    fn_1_CA68(22, obj->work[0], 10,
        &lbl_1_data_8A4[38], 1.0f);
    fn_1_CA68(23, obj->work[0], 10,
        &lbl_1_data_8A4[39], 1.0f);
    fn_1_CA68(24, obj->work[0], 10,
        &lbl_1_data_8A4[40], 1.0f);
    if (lbl_1_bss_9F4.unk_0 == 0) {
        for (i = 0, entry = lbl_1_bss_9BC;
             i < 4; i++, entry++) {
            fn_1_CA68((s16)entry->chrSel, obj->work[0], 10,
                &lbl_1_data_8A4[i + 45], 1.25f);
        }
    } else {
        for (i = 0; i < 4; i++) {
            entry = &lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
            fn_1_CA68((s16)entry->chrSel, obj->work[0], 10,
                &lbl_1_data_8A4[i + 57], 1.25f);
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_21628(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    HuVecF rot;
    s16 i;

    if (obj->work[0]++ > 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            lbl_1_bss_9F4.unk_E--;
            if (lbl_1_bss_9F4.unk_E < 0) {
                lbl_1_bss_9F4.unk_E += 3;
            }
            obj->work[0] = 0;
            obj->work[1] = 0;
            fn_1_AAF0(4);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            lbl_1_bss_9F4.unk_E++;
            if (lbl_1_bss_9F4.unk_E > 2) {
                lbl_1_bss_9F4.unk_E -= 3;
            }
            obj->work[0] = 0;
            obj->work[1] = 1;
            fn_1_AAF0(5);
        }
    }
    if (obj->work[0] < 10) {
        for (i = 0; i < 4; i++) {
            entry = &lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
            Hu3DModelRotGet(
                lbl_1_bss_288[entry->chrSel].modelId, &rot);
            if (obj->work[1]) {
                rot.y += 18.0f;
                if (rot.y > 90.0f) {
                    Hu3DModelPosSetV(
                        lbl_1_bss_288[entry->chrSel].modelId,
                        &lbl_1_data_8A4[i + 49]);
                }
            } else {
                rot.y -= 18.0f;
                if (rot.y < -90.0f) {
                    Hu3DModelPosSetV(
                        lbl_1_bss_288[entry->chrSel].modelId,
                        &lbl_1_data_8A4[i + 49]);
                }
            }
            Hu3DModelRotSetV(
                lbl_1_bss_288[entry->chrSel].modelId, &rot);
        }
    } else {
        for (i = 0; i < 4; i++) {
            entry = &lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
            Hu3DModelRotGet(
                lbl_1_bss_288[entry->chrSel].modelId, &rot);
            rot.y = 0.0f;
            Hu3DModelRotSetV(
                lbl_1_bss_288[entry->chrSel].modelId, &rot);
        }
    }
}

void fn_1_219C8(OMOBJ *obj)
{
    HuVecF pos;

    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9BC[0].chrSel].modelId, &pos);
    pos.x = 0.0f;
    fn_1_A970(4, &pos, -210.0f, 0.0f);
    fn_1_A970(5, &pos, 210.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_21628;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_21C04(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_918[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[4] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_21C9C(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;

    lbl_1_bss_9F4.unk_E = 0;
    lbl_1_bss_9F4.unk_10[0] = 0x30037;
    lbl_1_bss_9F4.unk_10[1] = 0x30037;
    if (obj->work[0] == 0) {
        for (i = 0; i < 4; i++) {
            entry = &lbl_1_bss_9BC[i];
            fn_1_C158(entry->chrSel, 0, entry->chrSel, 1);
            fn_1_C158(entry->chrSel, 1, entry->chrSel, 1);
            fn_1_C158(entry->chrSel, 2, entry->chrSel, 1);
            fn_1_C158(entry->chrSel, 3, entry->chrSel, 1);
        }
        fn_1_96E4(
            lbl_1_bss_9F4.unk_10[0], lbl_1_bss_9F4.unk_10[1]);
        fn_1_9E9C();
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_22010(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;

    lbl_1_bss_9F4.unk_10[0] = 0x30037;
    lbl_1_bss_9F4.unk_10[1] = 0x30037;
    if (obj->work[0] == 0) {
        for (i = 0; i < 4; i++) {
            entry = &lbl_1_bss_9BC[i];
            fn_1_C158(entry->chrSel, 0, entry->chrSel, 1);
            fn_1_C158(entry->chrSel, 2, entry->chrSel, 1);
        }
        fn_1_9658(0, lbl_1_bss_9F4.unk_10[0]);
        fn_1_9658(1, lbl_1_bss_9F4.unk_10[1]);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_221F4(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *entry;

    if (obj->work[0] == 0) {
        lbl_1_bss_9F4.unk_10[0] = fn_1_9558(
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][0]].chrSel,
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][1]].chrSel);
        lbl_1_bss_9F4.unk_10[1] = fn_1_9558(
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][2]].chrSel,
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][3]].chrSel);
        fn_1_9658(0, lbl_1_bss_9F4.unk_10[0]);
        fn_1_9658(1, lbl_1_bss_9F4.unk_10[1]);
        for (i = 0; i < 4; i++) {
            entry = &lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
            fn_1_C158(entry->chrSel, 0, entry->chrSel, 3);
        }
        fn_1_C158(
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][0]].chrSel,
            2, 38, -1);
        fn_1_C158(
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][1]].chrSel,
            2, 38, -1);
        fn_1_C158(
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][2]].chrSel,
            2, 39, -1);
        fn_1_C158(
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.unk_E][3]].chrSel,
            2, 39, -1);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_227E4(OMOBJ *obj)
{
    HUSPR_GROUP *group;
    s16 groupId;
    s16 i;

    if (obj->work[0] == 0) {
        HuWinDispOff(lbl_1_bss_104[0]);
        HuWinDispOff(lbl_1_bss_104[1]);
        groupId = lbl_1_bss_918[13];
        group = &HuSprGrpData[groupId];
        for (i = 0; i < group->sprNum; i++) {
            HuSprAttrSet(groupId, i, HUSPR_ATTR_DISPOFF);
        }
        lbl_1_bss_80.x = 0.0f;
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_228E8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;
    s16 j;

    for (i = 0; i < 4; i++) {
        entry = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
        if (entry->unk_4 == 1 && entry->unk_0 != 2) {
            if (entry->unk_0 == 1 && obj->work[0]++ > 10) {
                if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
                    entry->unk_6--;
                    if (lbl_1_bss_2C == 0) {
                        if (entry->unk_6 < 0) {
                            entry->unk_6 += 3;
                        }
                    } else if (entry->unk_6 < 0) {
                        entry->unk_6 += 4;
                    }
                    obj->work[0] = 0;
                    fn_1_AAF0(4);
                    fn_1_9B90(entry->unk_A, entry->unk_6 + 5);
                } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
                    entry->unk_6++;
                    if (lbl_1_bss_2C == 0) {
                        if (entry->unk_6 > 2) {
                            entry->unk_6 -= 3;
                        }
                    } else if (entry->unk_6 > 3) {
                        entry->unk_6 -= 4;
                    }
                    obj->work[0] = 0;
                    fn_1_AAF0(5);
                    fn_1_9B90(entry->unk_A, entry->unk_6 + 5);
                } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                    HuAudFXPlay(1);
                    entry->unk_0 = 2;
                    fn_1_AA94(4);
                    fn_1_AA94(5);
                    for (j = 0; j < 4; j++) {
                        if (lbl_1_bss_9BC[
                                lbl_1_data_4[lbl_1_bss_9F4.unk_E][j]]
                                    .unk_4 == 1
                            && lbl_1_bss_9BC[
                                lbl_1_data_4[lbl_1_bss_9F4.unk_E][j]]
                                    .unk_0 == 1) {
                            lbl_1_bss_9BC[
                                lbl_1_data_4[lbl_1_bss_9F4.unk_E][j]]
                                    .unk_6 = 1;
                            break;
                        }
                    }
                    if (j != 4) {
                        obj->objFunc = fn_1_22EB8;
                    } else {
                        obj->objFunc = NULL;
                    }
                } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                    HuAudFXPlay(3);
                    entry->unk_0 = 1;
                    fn_1_AA94(4);
                    fn_1_AA94(5);
                    fn_1_9B90(entry->unk_A, 4);
                    for (j = 3; j >= 0; j--) {
                        if (lbl_1_bss_9BC[
                                lbl_1_data_4[lbl_1_bss_9F4.unk_E][j]]
                                    .unk_4 == 1
                            && lbl_1_bss_9BC[
                                lbl_1_data_4[lbl_1_bss_9F4.unk_E][j]]
                                    .unk_0 == 2) {
                            lbl_1_bss_9BC[
                                lbl_1_data_4[lbl_1_bss_9F4.unk_E][j]]
                                    .unk_0 = 1;
                            break;
                        }
                    }
                    if (j != -1) {
                        obj->objFunc = fn_1_22EB8;
                    } else {
                        obj->objFunc = NULL;
                    }
                }
            }
            break;
        }
    }
}

void fn_1_22EB8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    HuVecF pos;
    s16 i;

    for (i = 0; i < 4; i++) {
        entry = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
        if (entry->unk_4 == 1 && entry->unk_0 == 1) {
            Hu3DModelPosGet(
                lbl_1_bss_288[entry->chrSel].modelId, &pos);
            fn_1_A970(4, &pos, -40.0f, -45.0f);
            fn_1_A970(5, &pos, 40.0f, -45.0f);
            fn_1_9B90(entry->unk_A, entry->unk_6 + 5);
            break;
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_228E8;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_2318C(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_918[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[4] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_23224(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *entry;

    for (i = 0; i < 4; i++) {
        entry = &lbl_1_bss_9BC[i];
        if (entry->unk_4 == 1) {
            entry->unk_6 = 1;
        }
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_232A0(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 49].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 49].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 49].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ >= 10) {
        fn_1_96E4(
            lbl_1_bss_9F4.unk_10[0], lbl_1_bss_9F4.unk_10[1]);
        for (i = 0; i < 4; i++) {
            player = &lbl_1_bss_9BC[i];
            if (player->unk_4 == 1) {
                player->unk_0 = 1;
                player->unk_6 = 1;
                HuSprBankSet(lbl_1_bss_918[i + 4], 0, 4);
            }
        }
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_239DC(OMOBJ *obj)
{
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_23A00(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;

    for (i = 0; i < 4; i++) {
        entry = &lbl_1_bss_9BC[i];
        if (entry->unk_4 == 1) {
            entry->unk_6 = 1;
            fn_1_9B90(i, 4);
        }
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_23AC0(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 49].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 49].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 49].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_24090(OMOBJ *obj)
{
    HuVecF pos;

    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            lbl_1_bss_9F4.unk_C--;
            if (lbl_1_bss_9F4.unk_C < 0) {
                lbl_1_bss_9F4.unk_C += 2;
            }
            obj->work[0] = 0;
            fn_1_AAF0(4);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            lbl_1_bss_9F4.unk_C++;
            if (lbl_1_bss_9F4.unk_C > 1) {
                lbl_1_bss_9F4.unk_C -= 2;
            }
            obj->work[0] = 0;
            fn_1_AAF0(5);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C--;
            if (lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C < 0) {
                lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C += 10;
            }
            obj->work[0] = 0;
            fn_1_AAF0(3);
            fn_1_9E50(lbl_1_bss_9F4.unk_C,
                lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C++;
            if (lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C > 9) {
                lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C -= 10;
            }
            obj->work[0] = 0;
            fn_1_AAF0(2);
            fn_1_9E50(lbl_1_bss_9F4.unk_C,
                lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].unk_C);
        }
    }
    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].chrSel].modelId,
        &pos);
    pos.x = lbl_1_data_8A4[lbl_1_bss_9F4.unk_C + 61].x;
    fn_1_A8D8(2, &pos, 0.0f, -80.0f);
    fn_1_A8D8(3, &pos, 0.0f, 80.0f);
}

void fn_1_2457C(OMOBJ *obj)
{
    HuVecF pos;

    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].chrSel].modelId,
        &pos);
    pos.x = lbl_1_data_8A4[lbl_1_bss_9F4.unk_C + 61].x;
    fn_1_A970(2, &pos, 0.0f, -80.0f);
    fn_1_A970(3, &pos, 0.0f, 80.0f);
    pos.x = 0.0f;
    fn_1_A970(4, &pos, -210.0f, 0.0f);
    fn_1_A970(5, &pos, 210.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_24090;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_249A4(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_918[2], 2, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[2] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 3, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[3] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[4] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_24A8C(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 bank;
    s16 i;
    s16 time;

    lbl_1_bss_9F4.unk_C = 0;
    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 49].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 49].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 49].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        for (i = 0; i < 2; i++) {
            lbl_1_bss_9BC[i].unk_C = 0;
            bank = lbl_1_bss_9BC[i].unk_C;
            lbl_1_bss_8C[i].x = 0.85f;
            fn_1_47B4(lbl_1_bss_918[i + 14], HUSPR_ATTR_DISPOFF);
            fn_1_9E50(i, bank);
        }
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_25160(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    lbl_1_bss_9F4.unk_C = 0;
    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 49].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 49].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 49].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_25740(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 57].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 57].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 57].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_25D10(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    if (obj->work[0] == 0) {
        for (i = 0; i < 2; i++) {
            lbl_1_bss_8C[i].x = 0.0f;
        }
    }
    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 49].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 49].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 49].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_26328(OMOBJ *obj)
{
    HuVecF pos;

    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            lbl_1_bss_9F4.unk_18--;
            if (lbl_1_bss_9F4.unk_18 < 0) {
                lbl_1_bss_9F4.unk_18 += 4;
            }
            obj->work[0] = 0;
            obj->work[1] = 2;
            fn_1_AAF0(4);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            lbl_1_bss_9F4.unk_18++;
            if (lbl_1_bss_9F4.unk_18 > 3) {
                lbl_1_bss_9F4.unk_18 -= 4;
            }
            obj->work[0] = 0;
            obj->work[1] = 2;
            fn_1_AAF0(5);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            switch (lbl_1_bss_9F4.unk_18) {
                case 0:
                    lbl_1_bss_9F4.unk_0--;
                    if (lbl_1_bss_9F4.unk_0 < 0) {
                        lbl_1_bss_9F4.unk_0 += 2;
                    }
                    break;
                case 1:
                    lbl_1_bss_9F4.unk_6--;
                    if (lbl_1_bss_9F4.unk_6 < 0) {
                        lbl_1_bss_9F4.unk_6 += 9;
                    }
                    break;
                case 2:
                    lbl_1_bss_9F4.unk_8--;
                    if (lbl_1_bss_9F4.unk_8 < 0) {
                        lbl_1_bss_9F4.unk_8 += 5;
                    }
                    break;
                case 3:
                    lbl_1_bss_9F4.unk_A--;
                    if (lbl_1_bss_9F4.unk_A < 0) {
                        lbl_1_bss_9F4.unk_A += 2;
                    }
                    break;
            }
            obj->work[0] = 0;
            obj->work[1] = 0;
            fn_1_AAF0(3);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            switch (lbl_1_bss_9F4.unk_18) {
                case 0:
                    lbl_1_bss_9F4.unk_0++;
                    if (lbl_1_bss_9F4.unk_0 >= 2) {
                        lbl_1_bss_9F4.unk_0 -= 2;
                    }
                    break;
                case 1:
                    lbl_1_bss_9F4.unk_6++;
                    if (lbl_1_bss_9F4.unk_6 >= 9) {
                        lbl_1_bss_9F4.unk_6 -= 9;
                    }
                    break;
                case 2:
                    lbl_1_bss_9F4.unk_8++;
                    if (lbl_1_bss_9F4.unk_8 >= 5) {
                        lbl_1_bss_9F4.unk_8 -= 5;
                    }
                    break;
                case 3:
                    lbl_1_bss_9F4.unk_A++;
                    if (lbl_1_bss_9F4.unk_A >= 2) {
                        lbl_1_bss_9F4.unk_A -= 2;
                    }
                    break;
            }
            obj->work[0] = 0;
            obj->work[1] = 1;
            fn_1_AAF0(2);
        }
    } else if (obj->work[1] != 2) {
        switch (lbl_1_bss_9F4.unk_18) {
            case 0:
                if (obj->work[1] != 0) {
                    fn_1_D058(21, lbl_1_bss_9F4.unk_0, 20,
                        obj->work[0], 10, 1);
                } else {
                    fn_1_D058(21, lbl_1_bss_9F4.unk_0, 20,
                        obj->work[0], 10, -1);
                }
                break;
            case 1:
                if (obj->work[1] != 0) {
                    fn_1_D058(22, lbl_1_bss_9F4.unk_6, 22,
                        obj->work[0], 10, 1);
                } else {
                    fn_1_D058(22, lbl_1_bss_9F4.unk_6, 22,
                        obj->work[0], 10, -1);
                }
                break;
            case 2:
                if (obj->work[1] != 0) {
                    fn_1_D058(23, lbl_1_bss_9F4.unk_8, 31,
                        obj->work[0], 10, 1);
                } else {
                    fn_1_D058(23, lbl_1_bss_9F4.unk_8, 31,
                        obj->work[0], 10, -1);
                }
                break;
            case 3:
                if (obj->work[1] != 0) {
                    fn_1_D058(24, lbl_1_bss_9F4.unk_A, 36,
                        obj->work[0], 10, 1);
                } else {
                    fn_1_D058(24, lbl_1_bss_9F4.unk_A, 36,
                        obj->work[0], 10, -1);
                }
                break;
        }
    }
    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9F4.unk_18 + 21].modelId, &pos);
    fn_1_A8D8(2, &pos, 0.0f, -65.0f);
    fn_1_A8D8(3, &pos, 0.0f, 65.0f);
}

void fn_1_28044(OMOBJ *obj)
{
    HuVecF pos;

    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9F4.unk_18 + 21].modelId, &pos);
    fn_1_A970(2, &pos, 0.0f, -65.0f);
    fn_1_A970(3, &pos, 0.0f, 65.0f);
    pos.x = 0.0f;
    fn_1_A970(4, &pos, -220.0f, 0.0f);
    fn_1_A970(5, &pos, 220.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_26328;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_28434(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_918[2], 2, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[2] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 3, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[3] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[4] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_2851C(OMOBJ *obj)
{
    if (obj->work[0] == 0) {
        lbl_1_bss_9F4.unk_18 = 0;
        lbl_1_bss_9F4.unk_0 = GwCommon.confTag;
        lbl_1_bss_9F4.unk_6 = (GwCommon.confTurnNum / 5) - 2;
        lbl_1_bss_9F4.unk_8 = GwCommon.partyMgPack;
        lbl_1_bss_9F4.unk_A = GwCommon.confBonusStar;
        lbl_1_bss_9F4.unk_A++;
        lbl_1_bss_9F4.unk_A %= 2;
        fn_1_C200(21, lbl_1_bss_9F4.unk_0 + 20);
        fn_1_C200(22, lbl_1_bss_9F4.unk_6 + 22);
        fn_1_C200(23, lbl_1_bss_9F4.unk_8 + 31);
        fn_1_C200(24, lbl_1_bss_9F4.unk_A + 36);
    }
    fn_1_C28C(21, obj->work[0], 10, &lbl_1_data_8A4[63], 1.5f);
    fn_1_C28C(22, obj->work[0], 10, &lbl_1_data_8A4[64], 1.5f);
    fn_1_C28C(23, obj->work[0], 10, &lbl_1_data_8A4[65], 1.5f);
    fn_1_C28C(24, obj->work[0], 10, &lbl_1_data_8A4[66], 1.5f);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_29598(OMOBJ *obj)
{
    if (obj->work[0] == 0) {
        lbl_1_bss_9F4.unk_18 = 0;
    }
    fn_1_CA68(21, obj->work[0], 10, &lbl_1_data_8A4[63], 1.5f);
    fn_1_CA68(22, obj->work[0], 10, &lbl_1_data_8A4[64], 1.5f);
    fn_1_CA68(23, obj->work[0], 10, &lbl_1_data_8A4[65], 1.5f);
    fn_1_CA68(24, obj->work[0], 10, &lbl_1_data_8A4[66], 1.5f);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_2A8C4(OMOBJ *obj)
{
    fn_1_CA68(21, obj->work[0], 10, &lbl_1_data_8A4[37], 1.0f);
    fn_1_CA68(22, obj->work[0], 10, &lbl_1_data_8A4[38], 1.0f);
    fn_1_CA68(23, obj->work[0], 10, &lbl_1_data_8A4[39], 1.0f);
    fn_1_CA68(24, obj->work[0], 10, &lbl_1_data_8A4[40], 1.0f);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_2BBD8(OMOBJ *obj)
{
    fn_1_C698(21, obj->work[0], 10);
    fn_1_C698(22, obj->work[0], 10);
    fn_1_C698(23, obj->work[0], 10);
    fn_1_C698(24, obj->work[0], 10);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_2C874(void)
{
    fn_1_87BC(lbl_1_bss_4);
    fn_1_945C(lbl_1_bss_8);
    fn_1_6284(lbl_1_bss_C);
    fn_1_72C4(lbl_1_bss_10);
    fn_1_8074(lbl_1_bss_14);
    fn_1_A880();
    fn_1_B6C0(lbl_1_bss_1C.obj);
    fn_1_4581C();
    fn_1_D978();
    fn_1_3C44();
    fn_1_36C4();
    fn_1_353C();
}

void fn_1_2CD6C(OMOBJ *obj)
{
    if (!WipeCheck()) {
        fn_1_2C874();
        omOvlReturnEx(1, 1);
    }
}

void fn_1_2CDA8(OMOBJ *obj)
{
    if (omSysExitReq != 0) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        obj->objFunc = fn_1_2CD6C;
    }
}

void fn_1_2CE00(void)
{
    s16 result = 0;

    result = fn_1_3EAC8(0);
    if (lbl_1_data_0 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_0, 1000);
        lbl_1_data_0 = -1;
    }
    switch (result) {
        case 0:
        case 1:
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            while (WipeCheck()) {
                HuPrcVSleep();
            }
            break;
        case 2:
        case 3:
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            while (WipeCheck()) {
                HuPrcVSleep();
            }
            break;
    }
    fn_1_2C874();
    switch (result) {
        case 0:
            fn_1_12F8();
            omOvlReturnEx(1, 1);
            break;
        case 1:
            fn_1_109C(1);
            break;
        case 2:
        case 3:
            fn_1_109C(0);
            break;
    }
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

void fn_1_2D1D4(void)
{
    lbl_1_bss_0 = omInitObjMan(27, 0x2000);
    omGameSysInit(lbl_1_bss_0);
    fn_1_33A0(fn_1_2DFD4);
    fn_1_3598();
    fn_1_3A1C();
    fn_1_467C();
    fn_1_4834();
    fn_1_D748();
    fn_1_44DCC(lbl_1_bss_0);
    lbl_1_bss_4 =
        omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10, -1, fn_1_861C);
    lbl_1_bss_8 =
        omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10, -1, fn_1_9218);
    lbl_1_bss_C =
        omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10, -1, fn_1_605C);
    lbl_1_bss_10 =
        omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10, -1, fn_1_7064);
    lbl_1_bss_14 =
        omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x20, -1, fn_1_7ED4);
    lbl_1_bss_18 =
        omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10, -1, fn_1_A5D8);
    lbl_1_bss_1C.obj =
        omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10, -1, fn_1_B4C8);
    HuPrcChildCreate(fn_1_2CE00, 0x3000, 0x3000, 0, lbl_1_bss_0);
}

void fn_1_2DD38(void)
{
    HuDataDirCloseAll();
}

void ObjectSetup(void)
{
    OSReport(lbl_1_data_ED8);
    fn_1_2DD38();
    fn_1_1248();
    _ClearFlag(FLAG_BOARD_INIT);
    _ClearFlag(FLAGNUM(FLAG_GROUP_COMMON, 0x23));
    lbl_1_bss_2A = FALSE;
    lbl_1_bss_2C = FALSE;
    if (GWBankFlagGet(2)) {
        lbl_1_bss_2A = TRUE;
    }
    if (GWBankFlagGet(3)) {
        lbl_1_bss_2C = TRUE;
    }
    lbl_1_data_BEE[0] = TRUE;
    lbl_1_data_BEE[1] = TRUE;
    lbl_1_data_BEE[2] = TRUE;
    lbl_1_data_BEE[3] = TRUE;
    lbl_1_data_BEE[4] = TRUE;
    lbl_1_data_BEE[5] = FALSE;
    if (GWBankFlagGet(0x33)) {
        lbl_1_data_BEE[5] = TRUE;
    }
    lbl_1_bss_28 = omovlevtno;
    fn_1_2D1D4();
}

int _prolog(void)
{
    const VoidFunc *ctors = _ctors;

    while (*ctors) {
        (**ctors)();
        ctors++;
    }
    ObjectSetup();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors) {
        (**dtors)();
        dtors++;
    }
}

void fn_1_2DFD4(OMOBJ *obj, MDCAMERA_WORK *camera)
{
    camera->unk_10.x = 0.0f;
    camera->unk_10.y = 65.0f;
    camera->unk_10.z = -800.0f;
    camera->unk_28.x = -7.25f;
    camera->unk_28.y = 0.0f;
    camera->unk_28.z = 0.0f;
    camera->unk_38 = 2150.0f;
    fn_1_28B0(camera, 15.0f);
}

void fn_1_2E338(OMOBJ *obj, MDCAMERA_WORK *camera)
{
    camera->center.y =
        fn_1_1868(camera->unk_10.y, 275.0f, camera->unk_40, 60.0f);
    camera->center.z =
        fn_1_1868(camera->unk_10.z, -2900.0f, camera->unk_40, 60.0f);
    camera->rot.x =
        fn_1_1868(camera->unk_28.x, 0.0f, camera->unk_40, 60.0f);
    camera->unk_40 += 1.0f;
}

void fn_1_2E560(OMOBJ *obj, MDCAMERA_WORK *camera)
{
    camera->unk_40 = 0.0f;
    fn_1_2810(camera);
    fn_1_2B64(fn_1_2E338);
}

s32 fn_1_2E5D4(void)
{
    HuPrcSleep(5);
    lbl_1_data_0 = HuAudSStreamPlay(4);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    return TRUE;
}

inline s32 fn_1_2E5D4(void);

s16 fn_1_2E638(void)
{
    OMOBJ *obj;

    fn_1_5194();
    fn_1_52B0();
    fn_1_4020(1, 0xD0000, 1);
    fn_1_3EC8();
    fn_1_5194();
    fn_1_4020(3, 0xD0001, 1);
    fn_1_3EC8();
    obj = lbl_1_bss_4;
    obj->work[0] = 0;
    obj->objFunc = fn_1_8398;
    return TRUE;
}

s16 fn_1_2EBEC(void)
{
    HuVecF pos;

    Hu3DModelPosGet(lbl_1_bss_C->mdlId[0], &pos);
    if (pos.x > -200) {
        fn_1_3E0C();
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0],
            lbl_1_bss_C->mtnId[5], 0.0f, 30.0f, 0);
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0],
            lbl_1_bss_10->mtnId[5], 0.0f, 30.0f, 0);
        HuPrcSleep(60);
        fn_1_5FB0();
        fn_1_6FB8();
        fn_1_3E0C();
        HuPrcSleep(90);
    }
}

inline s16 fn_1_2EBEC(void);

s16 fn_1_2EF24(void)
{
    fn_1_52B0();
    fn_1_4020(2, 0xD0008, 1);
    return fn_1_3F28(2);
}

inline s16 fn_1_2EF24(void);

s16 fn_1_2F22C(void)
{
    s16 result = 0;
    s16 turnNo = 0;

    if (GwCommon.saveEnableF == TRUE) {
        SLBoardLoad();
        lbl_1_bss_9F4.unk_0 = GwSystem.tagF;
        lbl_1_bss_9F4.unk_4 = GwSystem.boardNo;
        lbl_1_bss_9F4.unk_6 = (GwSystem.turnMax / 5) - 2;
        lbl_1_bss_9F4.unk_8 = GwSystem.mgPack;
        lbl_1_bss_9F4.unk_A = GwSystem.bonusStarF;
        lbl_1_bss_9F4.unk_A++;
        lbl_1_bss_9F4.unk_A %= 2;
        turnNo = GwSystem.turnNo;
        fn_1_2EBEC();
        fn_1_D6B0(fn_1_DA54);
        fn_1_8124(turnNo, (lbl_1_bss_9F4.unk_6 + 2) * 5);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_52B0();
            fn_1_4020(2, 0xD0002, 1);
            result = fn_1_3F28(0);
            if (result == 0) {
                _SetFlag(FLAGNUM(FLAG_GROUP_COMMON, 0x23));
                fn_1_8318();
                fn_1_D6B0(fn_1_EB74);
                fn_1_3E0C();
                fn_1_64C();
                HuAudFXPlay(0x4A2);
                fn_1_58A0();
                fn_1_68A4();
                HuPrcSleep(10);
                fn_1_463E0();
                if (lbl_1_data_0 != -1) {
                    HuAudSStreamFadeOut(lbl_1_data_0, 3000);
                    lbl_1_data_0 = -1;
                }
                HuAudFXPlay(0x4A3);
                HuPrcSleep(90);
                fn_1_2B64(fn_1_2E338);
                HuPrcSleep(60);
                return 0;
            }
            if (result == -1) {
                if (fn_1_2EF24()) {
                    continue;
                }
                return -1;
            }
            while (TRUE) {
                HuPrcVSleep();
                fn_1_4020(2, 0xD0026, 1);
                result = fn_1_3F28(0);
                if (result != -1) {
                    break;
                }
                if (fn_1_2EF24()) {
                    continue;
                }
                return -1;
            }
            if (result != 0) {
                continue;
            }
            fn_1_8318();
            fn_1_D6B0(fn_1_F838);
            break;
        }
    }
    return 1;
}

s16 fn_1_30B4C(void)
{
    s16 result = 0;
    while (TRUE) {
        HuPrcVSleep();
        fn_1_52B0();
        fn_1_4020(2, 0xD0004, 1);
        result = fn_1_3F28(0);
        if (result != -1) {
            if (result == 0) {
                fn_1_3E0C();
                fn_1_E30();
                fn_1_64C();
            }
            break;
        }
        if (!fn_1_2EF24()) {
            result = -1;
            break;
        }
    }
    return result;
}

s16 fn_1_31508(void)
{
    fn_1_2EBEC();
    fn_1_5194();
    fn_1_4020(3, 0xD0005, 1);
    fn_1_3EC8();
    return TRUE;
}

s16 fn_1_31AC0(s16 arg0)
{
    s16 result = 0;
    fn_1_D6B0(fn_1_1158C);
    while (TRUE) {
        HuPrcVSleep();
        fn_1_D6B0(fn_1_114C4);
        fn_1_444C(0x1000A);
        fn_1_52B0();
        fn_1_4020(2, 0xD0009 + lbl_1_bss_9F4.unk_2, 0);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_444C(0x1000A);
            fn_1_4020(2, 0xD0009 + lbl_1_bss_9F4.unk_2, 0);
            if (!fn_1_D678()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(2);
                fn_1_275C();
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(3);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(3);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_114F4);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result != -1) {
        fn_1_D6B0(fn_1_11EAC);
    }
    return result;
}

s16 fn_1_32904(void)
{
    s16 count;
    s16 i;

    for (i = 0, count = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].unk_0 == 1) {
            count++;
        }
    }
    if (lbl_1_bss_9F4.unk_2 <= count) {
        return 0;
    }
    if (lbl_1_bss_9BC[0].unk_0 == 0) {
        return 1;
    }
    return -1;
}

inline s16 fn_1_32904(void);

s16 fn_1_329A4(s16 arg0)
{
    s16 result = 0;

    if (arg0 != 0) {
        fn_1_D6B0(fn_1_12EC8);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_444C(0x1000A);
        fn_1_5194();
        fn_1_4020(3, 0xD000E, 0);
        fn_1_D6B0(fn_1_12B18);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_4020(3, 0xD000E, 0);
            result = fn_1_32904();
            if (!fn_1_D678()) {
                continue;
            }
            if (result == 0) {
                result = 0;
                break;
            }
            if (result == 1 && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                HuAudFXPlay(3);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(3);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_12D7C);
        if (result != -1) {
            break;
        }
        if (!fn_1_2EF24()) {
            result = -1;
            break;
        }
    }
    return result;
}

s16 fn_1_335F0(s16 *arg0, s16 *arg1, s16 *arg2)
{
    s16 i;
    s16 count = 0;

    *arg0 = 0;
    *arg1 = 0;
    *arg2 = 3;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].unk_4 == 1) {
            *arg1 = i;
            break;
        }
    }
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].unk_4 == 1
            && lbl_1_bss_9BC[i].unk_0 != 2) {
            break;
        }
    }
    *arg0 = i;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].unk_4 == 1
            && lbl_1_bss_9BC[i].unk_0 != 2) {
            count++;
        }
    }
    return count;
}

s16 fn_1_33724(s16 arg0)
{
    s16 result = 0;
    s16 activeNo = 0;
    s16 firstNo = 0;
    s16 maxNo = 0;
    s16 count = 0;

    count = fn_1_335F0(&activeNo, &firstNo, &maxNo);
    while (TRUE) {
        HuPrcVSleep();
        fn_1_444C(0x1000A);
        fn_1_5194();
        if (lbl_1_bss_9BC[activeNo].unk_0 == 0) {
            fn_1_4258(3, 0xD0013 - (count - 1), 0);
            fn_1_4020(3, 0xD000F, 0);
        } else {
            fn_1_4020(3, 0xD0014, 0);
        }
        fn_1_D6B0(fn_1_13EC0);
        while (TRUE) {
            HuPrcVSleep();
            if (lbl_1_bss_9BC[activeNo].unk_0 == 0) {
                fn_1_4258(3, 0xD0013 - (count - 1), 0);
                fn_1_4020(3, 0xD000F, 0);
            } else {
                fn_1_4020(3, 0xD0014, 0);
            }
            if (!fn_1_D678()) {
                continue;
            }
            count = fn_1_335F0(&activeNo, &firstNo, &maxNo);
            if (activeNo > maxNo) {
                result = 0;
                break;
            }
            if (firstNo == activeNo
                && lbl_1_bss_9BC[activeNo].unk_0 == 0
                && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                HuAudFXPlay(3);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(3);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_14344);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    return result;
}

s16 fn_1_34D10(s16 arg0)
{
    s16 result = 0;
    s16 flag = 0;

    if (arg0 != 0) {
        fn_1_D6B0(fn_1_16A8C);
    }
    fn_1_D6B0(fn_1_150D0);
    switch (lbl_1_bss_9F4.unk_2) {
        case 0:
            result = fn_1_33724(0);
            break;
        case 4:
            result = fn_1_329A4(0);
            break;
        default:
            while (TRUE) {
                HuPrcVSleep();
                result = fn_1_329A4(flag);
                if (result != 0) {
                    break;
                }
                result = fn_1_33724(0);
                if (result != 1) {
                    break;
                }
                flag = 1;
            }
            break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_156B4);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_16E84);
    }
    return result;
}

inline s16 fn_1_34D10(s16 arg0);

s16 fn_1_34FC4(s16 *arg0, s16 *arg1, s16 *arg2)
{
    s16 i;
    s16 count = 0;

    *arg0 = 0;
    *arg1 = 0;
    *arg2 = 3;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].unk_4 == 1) {
            *arg1 = i;
            break;
        }
    }
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].unk_4 == 1
            && lbl_1_bss_9BC[i].unk_0 != 1) {
            break;
        }
    }
    *arg0 = i;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].unk_4 == 1
            && lbl_1_bss_9BC[i].unk_0 != 1) {
            count++;
        }
    }
    return count;
}

s16 fn_1_350F8(s16 arg0)
{
    s16 result = 0;
    s16 activeNo = 0;
    s16 firstNo = 0;
    s16 maxNo = 0;
    s16 count = 0;

    count = fn_1_335F0(&activeNo, &firstNo, &maxNo);
    while (TRUE) {
        HuPrcVSleep();
        fn_1_444C(0x1000A);
        fn_1_5194();
        fn_1_4258(3, 0xD0013 - (count - 1), 0);
        fn_1_4020(3, 0xD000F, 0);
        fn_1_D6B0(fn_1_14CC0);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_4258(3, 0xD0013 - (count - 1), 0);
            fn_1_4020(3, 0xD000F, 0);
            if (!fn_1_D678()) {
                continue;
            }
            count = fn_1_34FC4(&activeNo, &firstNo, &maxNo);
            if (activeNo > maxNo) {
                result = 0;
                break;
            }
            if (firstNo == activeNo
                && lbl_1_bss_9BC[activeNo].unk_0 == 0
                && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                HuAudFXPlay(3);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(3);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_14F64);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    return result;
}

s16 fn_1_36264(s16 arg0)
{
    s16 result = 0;
    s16 flag = 0;

    if (arg0 != 0) {
        fn_1_D6B0(fn_1_16A8C);
    }
    fn_1_D6B0(fn_1_150D0);
    switch (lbl_1_bss_9F4.unk_2) {
        case 0:
            result = fn_1_350F8(0);
            break;
        case 4:
            result = fn_1_329A4(0);
            break;
        default:
            while (TRUE) {
                HuPrcVSleep();
                result = fn_1_329A4(flag);
                if (result != 0) {
                    break;
                }
                result = fn_1_350F8(0);
                if (result != 1) {
                    break;
                }
                flag = 1;
            }
            break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_160A0);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_16E84);
    }
    return result;
}

inline s16 fn_1_36264(s16 arg0);

s16 fn_1_36518(s16 arg0)
{
    s16 result = 0;

    if (arg0 == 0) {
        fn_1_D6B0(fn_1_21C9C);
    } else {
        fn_1_D6B0(fn_1_22010);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_D6B0(fn_1_219C8);
        fn_1_444C(0x1000A);
        fn_1_5194();
        fn_1_4020(3, 0xD0015, 0);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_4020(3, 0xD0015, 0);
            if (!fn_1_D678()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(2);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(3);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(3);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_21C04);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_221F4);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_227E4);
    }
    return result;
}

void fn_1_37260(s16 *arg0, s16 *arg1, s16 *arg2)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;

    *arg0 = 0;
    *arg1 = 0;
    *arg2 = 3;
    for (i = 0; i < 4; i++) {
        entry = &lbl_1_bss_9BC[lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
        if (entry->unk_4 == 1) {
            *arg1 = i;
            break;
        }
    }
    for (i = 0; i < 4; i++) {
        entry = &lbl_1_bss_9BC[lbl_1_data_4[lbl_1_bss_9F4.unk_E][i]];
        if (entry->unk_4 == 1 && entry->unk_0 != 2) {
            break;
        }
    }
    *arg0 = i;
}

inline void fn_1_37260(s16 *arg0, s16 *arg1, s16 *arg2);

s16 fn_1_3736C(s16 arg0)
{
    s16 activeNo;
    s16 firstNo;
    s16 maxNo;
    s16 result = 0;

    if (lbl_1_bss_9F4.unk_2 == 4) {
        if (arg0 == 0) {
            return 0;
        }
        fn_1_96E4(lbl_1_bss_9F4.unk_10[0], lbl_1_bss_9F4.unk_10[1]);
        return 1;
    }
    if (arg0 == 0) {
        fn_1_D6B0(fn_1_23224);
    } else {
        fn_1_D6B0(fn_1_232A0);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_444C(0x1000A);
        fn_1_5194();
        fn_1_4020(3, 0xD0014, 0);
        fn_1_D6B0(fn_1_22EB8);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_4020(3, 0xD0014, 0);
            if (!fn_1_D678()) {
                continue;
            }
            fn_1_37260(&activeNo, &firstNo, &maxNo);
            if (activeNo > maxNo) {
                result = 0;
                break;
            }
            if (activeNo == firstNo
                && lbl_1_bss_9BC[activeNo].unk_0 == 1
                && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                HuAudFXPlay(3);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(3);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_2318C);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_239DC);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_23A00);
    }
    return result;
}

s16 fn_1_38314(void)
{
    fn_1_5194();
    fn_1_4258(3, lbl_1_bss_9F4.unk_10[0], 0);
    fn_1_4258(3, lbl_1_bss_9F4.unk_10[1], 1);
    fn_1_4020(3, 0xD0016, 1);
    fn_1_3EC8();
    fn_1_9804();
    fn_1_D6B0(fn_1_23AC0);
}

s16 fn_1_38A3C(s16 arg0)
{
    s16 result = 0;

    if (arg0 != 0) {
        fn_1_D6B0(fn_1_18A10);
    }
    fn_1_D6B0(fn_1_18384);
    while (TRUE) {
        HuPrcVSleep();
        fn_1_D6B0(fn_1_17E10);
        fn_1_444C(0x1000A);
        fn_1_52B0();
        if (lbl_1_data_BEE[lbl_1_bss_9F4.unk_4] == 1) {
            fn_1_4020(2, lbl_1_bss_9F4.unk_4 + 0x20009, 0);
        } else {
            fn_1_4020(2, 0x110017, 0);
        }
        while (TRUE) {
            HuPrcVSleep();
            if (lbl_1_data_BEE[lbl_1_bss_9F4.unk_4] == 1) {
                fn_1_4020(2, lbl_1_bss_9F4.unk_4 + 0x20009, 0);
            } else {
                fn_1_4020(2, 0x110017, 0);
            }
            if (!fn_1_D678()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                if (lbl_1_data_BEE[lbl_1_bss_9F4.unk_4] == 0) {
                    HuAudFXPlay(4);
                    continue;
                }
                HuAudFXPlay(2);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(3);
                result = -1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(3);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_1807C);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_18AA4);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_1948C);
    }
    return result;
}

s16 fn_1_39C60(s16 arg0)
{
    s16 result = 0;

    if (arg0 == 0) {
        fn_1_D6B0(fn_1_1A20C);
    } else {
        fn_1_D6B0(fn_1_1A8B0);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_D6B0(fn_1_19D24);
        fn_1_444C(0x1000A);
        fn_1_52B0();
        fn_1_4258(2,
            lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].chrSel, 0);
        fn_1_4020(2, 0xD0022, 0);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_4258(2,
                lbl_1_bss_9BC[lbl_1_bss_9F4.unk_C].chrSel, 0);
            fn_1_4020(2, 0xD0022, 0);
            if (!fn_1_D678()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(2);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(3);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(3);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_1A124);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_1AE60);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_1B400);
    }
    return result;
}

s16 fn_1_3AD84(s16 arg0)
{
    s16 result = 0;

    if (arg0 == 0) {
        fn_1_D6B0(fn_1_24A8C);
    } else {
        fn_1_D6B0(fn_1_25160);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_D6B0(fn_1_2457C);
        fn_1_444C(0x1000A);
        fn_1_52B0();
        fn_1_4258(2,
            lbl_1_bss_9F4.unk_10[lbl_1_bss_9F4.unk_C], 0);
        fn_1_4020(2, 0xD0027, 0);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_4258(2,
                lbl_1_bss_9F4.unk_10[lbl_1_bss_9F4.unk_C], 0);
            fn_1_4020(2, 0xD0027, 0);
            if (!fn_1_D678()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(2);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(3);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(3);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_249A4);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_25740);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_25D10);
    }
    return result;
}

s16 fn_1_3BEA8(void)
{
    s16 result = 0;

    fn_1_D6B0(fn_1_1B9E8);
    HuPrcVSleep();
    fn_1_5194();
    fn_1_4020(3, 0xD0023, 1);
    if ((result = fn_1_3F28(2)) == 0) {
        fn_1_4020(1, 0xD0025, 1);
        HuPrcSleep(60);
        fn_1_4FE8();
        HuPrcSleep(60);
        fn_1_3E0C();
        fn_1_D6B0(fn_1_1D758);
        result = 0;
    } else {
        fn_1_5194();
        fn_1_4020(3, 0xD0024, 1);
        if ((result = fn_1_3F28(2)) == 0) {
            fn_1_D6B0(fn_1_1E7F8);
            result = 1;
        } else {
            fn_1_D6B0(fn_1_1F8B8);
            result = 2;
        }
    }
    return result;
}

void fn_1_3CA70(void)
{
    fn_1_718();
    fn_1_64C();
    HuPrcSleep(10);
    fn_1_7900();
    fn_1_58A0();
    fn_1_68A4();
    HuPrcSleep(10);
    fn_1_463E0();
    if (lbl_1_data_0 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_0, 3000);
        lbl_1_data_0 = -1;
    }
    HuAudFXPlay(0x4A3);
    HuPrcSleep(90);
    lbl_1_bss_A10.callback = fn_1_2E338;
    HuPrcSleep(60);
}

s16 fn_1_3CFE4(s16 arg0)
{
    s16 result = 0;

    if (arg0 == 0) {
        fn_1_D6B0(fn_1_2851C);
    } else {
        fn_1_D6B0(fn_1_29598);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_D6B0(fn_1_28044);
        fn_1_5194();
        fn_1_444C(0x1000A);
        switch (lbl_1_bss_9F4.unk_18) {
            case 0:
                fn_1_4020(3, lbl_1_bss_9F4.unk_0 + 0xD0006, 0);
                break;
            case 1:
                fn_1_4020(3, lbl_1_bss_9F4.unk_6 + 0xD0017, 0);
                break;
            case 2:
                fn_1_4020(3, lbl_1_bss_9F4.unk_8 + 0x40000, 0);
                break;
            case 3:
                fn_1_4020(3, lbl_1_bss_9F4.unk_A + 0xD0020, 0);
                break;
        }
        while (TRUE) {
            HuPrcVSleep();
            switch (lbl_1_bss_9F4.unk_18) {
                case 0:
                    fn_1_4020(3,
                        lbl_1_bss_9F4.unk_0 + 0xD0006, 0);
                    break;
                case 1:
                    fn_1_4020(3,
                        lbl_1_bss_9F4.unk_6 + 0xD0017, 0);
                    break;
                case 2:
                    fn_1_4020(3,
                        lbl_1_bss_9F4.unk_8 + 0x40000, 0);
                    break;
                case 3:
                    fn_1_4020(3,
                        lbl_1_bss_9F4.unk_A + 0xD0020, 0);
                    break;
            }
            if (!fn_1_D678()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(2);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(3);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(3);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_28434);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_2A8C4);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_2BBD8);
    }
    return result;
}

s16 fn_1_3EAC8(s16 arg0)
{
    s16 state = 0;
    s16 result = 0;
    s16 back = 0;

    fn_1_2E5D4();
    if (lbl_1_bss_28 == 0) {
        fn_1_2E638();
        result = fn_1_2F22C();
        if (result == 0) {
            return 3;
        }
        if (result == -1) {
            return 0;
        }
    } else {
        state = 1;
        back = 0;
    }
    while (TRUE) {
        HuPrcVSleep();
        switch (state) {
            case 0:
                result = fn_1_30B4C();
                if (result == 0) {
                    return 1;
                }
                if (result == 1) {
                    state = 1;
                    back = 0;
                }
                break;
            case 1:
                fn_1_31508();
                state = 2;
                back = 0;
                break;
            case 2:
                result = fn_1_38A3C(back);
                if (result == 0) {
                    state = 3;
                    back = 0;
                } else if (result == 1) {
                    state = 0;
                    back = 1;
                }
                break;
            case 3:
                result = fn_1_3CFE4(back);
                if (result == 0) {
                    state = 4;
                    back = 0;
                } else if (result == 1) {
                    state = 2;
                    back = 1;
                }
                break;
            case 4:
                result = fn_1_31AC0(back);
                if (result == 0) {
                    if (lbl_1_bss_9F4.unk_0 == 0) {
                        state = 5;
                        back = 0;
                    } else {
                        state = 100;
                        back = 0;
                    }
                } else if (result == 1) {
                    state = 3;
                    back = 1;
                }
                break;
            case 5:
                result = fn_1_34D10(back);
                if (result == 0) {
                    state = 6;
                    back = 0;
                } else if (result == 1) {
                    state = 4;
                    back = 1;
                }
                break;
            case 6:
                result = fn_1_39C60(back);
                if (result == 0) {
                    state = 7;
                    back = 0;
                } else if (result == 1) {
                    state = 5;
                    back = 1;
                }
                break;
            case 7:
                result = fn_1_3BEA8();
                if (result == 0) {
                    state = 8;
                    back = 0;
                } else if (result == 1) {
                    state = 1;
                    back = 0;
                } else if (result == 2) {
                    if (lbl_1_bss_9F4.unk_0 == 0) {
                        state = 6;
                        back = 1;
                    } else {
                        state = 104;
                        back = 1;
                    }
                }
                break;
            case 8:
                fn_1_3CA70();
                return 2;
            case 100:
                result = fn_1_36264(back);
                if (result == 0) {
                    state = 101;
                    back = 0;
                } else if (result == 1) {
                    state = 4;
                    back = 1;
                }
                break;
            case 101:
                result = fn_1_36518(back);
                if (result == 0) {
                    state = 102;
                    back = 0;
                } else if (result == 1) {
                    state = 100;
                    back = 1;
                }
                break;
            case 102:
                result = fn_1_3736C(back);
                if (result == 0) {
                    state = 103;
                    back = 0;
                } else if (result == 1) {
                    state = 101;
                    back = 1;
                }
                break;
            case 103:
                fn_1_38314();
                state = 104;
                back = 0;
                break;
            case 104:
                result = fn_1_3AD84(back);
                if (result == 0) {
                    state = 7;
                    back = 0;
                } else if (result == 1) {
                    state = 102;
                    back = 1;
                }
                break;
        }
        if (result == -1) {
            return 0;
        }
    }
}

s32 lbl_1_data_0 = -1;
s16 lbl_1_data_4[3][4] = {
    { 0, 1, 2, 3 },
    { 0, 2, 1, 3 },
    { 0, 3, 1, 2 },
};
u32 lbl_1_data_1C[32] = {
    DATANUM(DATA_mdparty, 0x1C),
    DATANUM(DATA_mdparty, 0x04),
    DATANUM(DATA_mdparty, 0x05),
    DATANUM(DATA_mdparty, 0x06),
    DATANUM(DATA_mdparty, 0x07),
    DATANUM(DATA_mdparty, 0x08),
    DATANUM(DATA_mdparty, 0x09),
    DATANUM(DATA_mdparty, 0x0A),
    DATANUM(DATA_mdparty, 0x0B),
    DATANUM(DATA_mdparty, 0x0C),
    DATANUM(DATA_mdparty, 0x0D),
    DATANUM(DATA_mdparty, 0x0E),
    DATANUM(DATA_mdparty, 0x0F),
    DATANUM(DATA_mdparty, 0x10),
    DATANUM(DATA_mdparty, 0x11),
    DATANUM(DATA_mdparty, 0x1E),
    DATANUM(DATA_mdparty, 0x1F),
    DATANUM(DATA_mdparty, 0x20),
    DATANUM(DATA_mdparty, 0x21),
    DATANUM(DATA_mdparty, 0x22),
    DATANUM(DATA_mdparty, 0x23),
    DATANUM(DATA_mdparty, 0x1B),
    DATANUM(DATA_mdparty, 0x15),
    DATANUM(DATA_mdparty, 0x14),
    DATANUM(DATA_mdparty, 0x16),
    DATANUM(DATA_mdparty, 0x17),
    DATANUM(DATA_mdparty, 0x12),
    DATANUM(DATA_mdparty, 0x13),
    DATANUM(DATA_mdparty, 0x18),
    DATANUM(DATA_mdparty, 0x19),
    DATANUM(DATA_mdparty, 0x4E),
    DATANUM(DATA_mdparty, 0x1D),
};
s16 lbl_1_data_9C[17] = {
    2, 6, 6, 4, 2, 2, 2, 2, 4, 4, 4, 4, 5, 2, 4, 4, 1,
};
MDSPR_INFO lbl_1_data_C0[58] = {
    { 0, 0, 0, 10, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 0, 15, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 1, 16, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 2, 17, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 3, 18, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 4, 19, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 5, 20, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 0, 21, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 1, 21, 0, 2, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 2, 21, 0, 5, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 3, 21, 0, 7, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 4, 21, 0, 4, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 5, 21, 0, 6, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 3, 0, 22, 0, 0, { 0.0f, 0.0f }, { 0.8f, 0.8f }, 0.0f },
    { 3, 1, 22, 0, 0, { 0.0f, 0.0f }, { 0.8f, 0.8f }, 0.0f },
    { 3, 2, 22, 0, 0, { 0.0f, 0.0f }, { 0.8f, 0.8f }, 0.0f },
    { 3, 3, 22, 0, 0, { 0.0f, 0.0f }, { 0.8f, 0.8f }, 0.0f },
    { 4, 0, 22, 9, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 4, 1, 23, 10, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 5, 0, 22, 9, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 5, 1, 23, 10, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 6, 0, 22, 9, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 6, 1, 23, 10, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 7, 0, 22, 9, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 7, 1, 23, 10, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 8, 0, 25, 9, 0, { -44.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 8, 1, 24, 9, 10, { -25.0f, 25.0f }, { 0.8f, 0.8f }, 0.0f },
    { 8, 2, 24, 9, 0, { -4.0f, 24.0f }, { 0.9f, 0.9f }, 0.0f },
    { 8, 3, 23, 10, 1, { -22.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 9, 0, 25, 9, 0, { -44.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 9, 1, 24, 9, 10, { -25.0f, 25.0f }, { 0.8f, 0.8f }, 0.0f },
    { 9, 2, 24, 9, 0, { -4.0f, 24.0f }, { 0.9f, 0.9f }, 0.0f },
    { 9, 3, 23, 10, 1, { -22.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 10, 0, 25, 9, 0, { -44.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 10, 1, 24, 9, 10, { -25.0f, 25.0f }, { 0.8f, 0.8f }, 0.0f },
    { 10, 2, 24, 9, 0, { -4.0f, 24.0f }, { 0.9f, 0.9f }, 0.0f },
    { 10, 3, 23, 10, 1, { -22.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 11, 0, 25, 9, 0, { -44.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 11, 1, 24, 9, 10, { -25.0f, 25.0f }, { 0.8f, 0.8f }, 0.0f },
    { 11, 2, 24, 9, 0, { -4.0f, 24.0f }, { 0.9f, 0.9f }, 0.0f },
    { 11, 3, 23, 10, 1, { -22.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 12, 0, 27, 9, 0, { -35.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 12, 1, 27, 9, 0, { 5.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 12, 2, 27, 9, 0, { 85.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 12, 3, 27, 9, 0, { 125.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 12, 4, 26, 10, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 13, 0, 28, 10, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 13, 1, 29, 10, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 14, 0, 25, 9, 0, { -22.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 14, 1, 24, 9, 10, { -3.0f, 25.0f }, { 0.8f, 0.8f }, 0.0f },
    { 14, 2, 24, 9, 0, { 18.0f, 24.0f }, { 0.9f, 0.9f }, 0.0f },
    { 14, 3, 23, 10, 2, { 0.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 15, 0, 25, 9, 0, { -22.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 15, 1, 24, 9, 10, { -3.0f, 25.0f }, { 0.8f, 0.8f }, 0.0f },
    { 15, 2, 24, 9, 0, { 18.0f, 24.0f }, { 0.9f, 0.9f }, 0.0f },
    { 15, 3, 23, 10, 2, { 0.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 16, 0, 30, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 0, 1, 31, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
};
u32 lbl_1_data_800[41] = {
    DATANUM(DATA_mdparty, 0x27),
    DATANUM(DATA_mdparty, 0x28),
    DATANUM(DATA_mdparty, 0x29),
    DATANUM(DATA_mdparty, 0x2A),
    DATANUM(DATA_mdparty, 0x2B),
    DATANUM(DATA_mdparty, 0x2C),
    DATANUM(DATA_mdparty, 0x2D),
    DATANUM(DATA_mdparty, 0x2E),
    DATANUM(DATA_mdparty, 0x2F),
    DATANUM(DATA_mdparty, 0x30),
    DATANUM(DATA_mdparty, 0x31),
    DATANUM(DATA_mdparty, 0x32),
    DATANUM(DATA_mdparty, 0x33),
    DATANUM(DATA_mdparty, 0x34),
    DATANUM(DATA_mdparty, 0x35),
    DATANUM(DATA_mdparty, 0x36),
    DATANUM(DATA_mdparty, 0x37),
    DATANUM(DATA_mdparty, 0x38),
    DATANUM(DATA_mdparty, 0x25),
    DATANUM(DATA_mdparty, 0x26),
    DATANUM(DATA_mdparty, 0x39),
    DATANUM(DATA_mdparty, 0x3A),
    DATANUM(DATA_mdparty, 0x3B),
    DATANUM(DATA_mdparty, 0x3C),
    DATANUM(DATA_mdparty, 0x3D),
    DATANUM(DATA_mdparty, 0x3E),
    DATANUM(DATA_mdparty, 0x3F),
    DATANUM(DATA_mdparty, 0x40),
    DATANUM(DATA_mdparty, 0x41),
    DATANUM(DATA_mdparty, 0x42),
    DATANUM(DATA_mdparty, 0x43),
    DATANUM(DATA_mdparty, 0x44),
    DATANUM(DATA_mdparty, 0x45),
    DATANUM(DATA_mdparty, 0x46),
    DATANUM(DATA_mdparty, 0x47),
    DATANUM(DATA_mdparty, 0x48),
    DATANUM(DATA_mdparty, 0x49),
    DATANUM(DATA_mdparty, 0x4A),
    DATANUM(DATA_mdparty, 0x4B),
    DATANUM(DATA_mdparty, 0x4C),
    DATANUM(DATA_mdparty, 0x4D),
};
HuVecF lbl_1_data_8A4[67] = {
    { 0.0f, 275.0f, 0.0f },
    { -200.0f, 325.0f, 0.0f },
    { -120.0f, 225.0f, 0.0f },
    { -40.0f, 325.0f, 0.0f },
    { 40.0f, 225.0f, 0.0f },
    { 120.0f, 325.0f, 0.0f },
    { 200.0f, 225.0f, 0.0f },
    { -200.0f, 275.0f, 0.0f },
    { -120.0f, 275.0f, 0.0f },
    { -40.0f, 275.0f, 0.0f },
    { 40.0f, 275.0f, 0.0f },
    { 120.0f, 275.0f, 0.0f },
    { 200.0f, 275.0f, 0.0f },
    { 0.0f, 275.0f, -800.0f },
    { -195.0f, 380.0f, 0.0f },
    { -65.0f, 380.0f, 0.0f },
    { 65.0f, 380.0f, 0.0f },
    { 195.0f, 380.0f, 0.0f },
    { -130.0f, 240.0f, 0.0f },
    { 0.0f, 240.0f, 0.0f },
    { 130.0f, 240.0f, 0.0f },
    { -195.0f, 100.0f, 0.0f },
    { -65.0f, 100.0f, 0.0f },
    { 65.0f, 100.0f, 0.0f },
    { 195.0f, 100.0f, 0.0f },
    { -180.0f, 65.0f, 0.0f },
    { -60.0f, 65.0f, 0.0f },
    { 60.0f, 65.0f, 0.0f },
    { 180.0f, 65.0f, 0.0f },
    { -180.0f, 125.0f, 0.0f },
    { -60.0f, 125.0f, 0.0f },
    { 60.0f, 125.0f, 0.0f },
    { 180.0f, 125.0f, 0.0f },
    { -180.0f, 275.0f, 0.0f },
    { -60.0f, 275.0f, 0.0f },
    { 60.0f, 275.0f, 0.0f },
    { 180.0f, 275.0f, 0.0f },
    { -340.0f, 375.0f, 0.0f },
    { -340.0f, 240.0f, 0.0f },
    { 340.0f, 375.0f, 0.0f },
    { 340.0f, 240.0f, 0.0f },
    { -225.0f, 300.0f, 0.0f },
    { -75.0f, 300.0f, 0.0f },
    { 75.0f, 300.0f, 0.0f },
    { 225.0f, 300.0f, 0.0f },
    { -180.0f, 175.0f, 0.0f },
    { -60.0f, 175.0f, 0.0f },
    { 60.0f, 175.0f, 0.0f },
    { 180.0f, 175.0f, 0.0f },
    { -220.0f, 175.0f, 0.0f },
    { -100.0f, 175.0f, 0.0f },
    { 100.0f, 175.0f, 0.0f },
    { 220.0f, 175.0f, 0.0f },
    { -220.0f, 65.0f, 0.0f },
    { -100.0f, 65.0f, 0.0f },
    { 100.0f, 65.0f, 0.0f },
    { 220.0f, 65.0f, 0.0f },
    { -220.0f, 125.0f, 0.0f },
    { -100.0f, 125.0f, 0.0f },
    { 100.0f, 125.0f, 0.0f },
    { 220.0f, 125.0f, 0.0f },
    { -160.0f, 0.0f, 0.0f },
    { 160.0f, 0.0f, 0.0f },
    { -225.0f, 275.0f, 0.0f },
    { -75.0f, 275.0f, 0.0f },
    { 75.0f, 275.0f, 0.0f },
    { 225.0f, 275.0f, 0.0f },
};
s32 lbl_1_data_BC8 = -1;
char lbl_1_data_BCC[] = "# ========== win callback :: %d\n";
s16 lbl_1_data_BEE[7] = { 0, 0, 0, 0, 0, 0, 0 };
u32 lbl_1_data_BFC[14] = {
    DATA_w01,
    DATA_w02,
    DATA_w03,
    DATA_w04,
    DATA_w05,
    DATA_w06,
    DATA_w10,
    DATA_w01n,
    DATA_w02n,
    DATA_w03n,
    DATA_w04n,
    DATA_w05n,
    DATA_w06n,
    DATA_w10n,
};
char lbl_1_data_C34[] =
    "\n>>>>>>>>>> mdpartydll :: READ INIT!! <<<<<<<<<<\n";
char lbl_1_data_C66[] = "0x%x :: _effect\n";
char lbl_1_data_C77[] = "0x%x :: _gamemes\n";
char lbl_1_data_C89[] = "0x%x :: _mgconst\n";
char lbl_1_data_C9B[] = "0x%x :: _win\n";
char lbl_1_data_CA9[] = "\n";
char lbl_1_data_CAB[] =
    "\n>>>>>>>>>> mdpartydll :: READ END!! <<<<<<<<<<\n";
char lbl_1_data_CDC[] = ">>>>>>>>>> mdpartydll :: objsetup!! <<<<<<<<<<\n";
char lbl_1_data_D0C[] = "\n>>>>>>>>>> mdpartydll :: ovlreturn!! <<<<<<<<<<\n";
char lbl_1_data_D3E[] = "-----===== CAMERA TEST =====-----";
char lbl_1_data_D60[] = "CENTER : %.2f, %.2f, %.2f";
char lbl_1_data_D7A[] = "ROT    : %.2f, %.2f, %.2f";
char lbl_1_data_D94[] = "ZOOM   : %.2f";
s16 lbl_1_data_DA2[3] = { -1, -1, 0 };
s32 lbl_1_data_DA8[3] = { -1, -1, -1 };
char lbl_1_data_DB4[] = "gN00m1-itemhook_R";
char lbl_1_data_DC6[] = "gN01m1-itemhook_R";
char lbl_1_data_DD8[] = "dpic00";
char lbl_1_data_DDF[] = "dpic01";
s16 lbl_1_data_DE6[6] = { 0, 0, 0, 0, 0, 0 };
char lbl_1_data_DF2[] = "ys77120";
char lbl_1_data_DFA[] = "ys77121";
char lbl_1_data_E02[] = "ys77060";
char lbl_1_data_E0A[] = "ys77061";
LBL_1_DATA_E12_ENTRY lbl_1_data_E12[11] = {
    { { -1, -1, 1, 5, 4, -1, -1, -1 }, -1 },
    { { -1, -1, 2, 6, 5, 4, 0, -1 }, -1 },
    { { -1, -1, 3, -1, 6, 5, 1, -1 }, -1 },
    { { -1, -1, -1, -1, -1, 6, 2, -1 }, -1 },
    { { 0, 1, 5, 9, 8, 7, -1, -1 }, -1 },
    { { 1, 2, 6, 10, 9, 8, 4, 0 }, -1 },
    { { 2, 3, 10, 10, 10, 9, 5, 1 }, -1 },
    { { -1, 4, 8, -1, -1, -1, -1, -1 }, -1 },
    { { 4, 5, 9, -1, -1, -1, 7, -1 }, -1 },
    { { 5, 6, 10, -1, -1, -1, 8, 4 }, -1 },
    { { 6, -1, -1, -1, -1, -1, 9, 5 }, -1 },
};
char lbl_1_data_ED8[] = "\n-----===== MARIO PARTY 6 :: PARTY MODE =====-----\n\n";

s32 lbl_1_bss_A6C[5];
HU3D_LIGHTID lbl_1_bss_A68[2];
HUWINID lbl_1_bss_A60[4];
MDCAMERA_WORK lbl_1_bss_A10;
LBL_1_BSS_9F4 lbl_1_bss_9F4;
LBL_1_BSS_9BC_ENTRY lbl_1_bss_9BC[4];
ANIMDATA *lbl_1_bss_93C[32];
HUSPR_GROUPID lbl_1_bss_918[17];
HUSPRID lbl_1_bss_8A4[58];
ANIMDATA *lbl_1_bss_800[41];
LBL_1_BSS_288_ENTRY lbl_1_bss_288[25];
LBL_1_BSS_228_ENTRY lbl_1_bss_228[2];
LBL_1_BSS_1C8_ENTRY lbl_1_bss_1C8[2];
LBL_1_BSS_1C8_ENTRY lbl_1_bss_108[4];
HUWINID lbl_1_bss_104[2];
HuVecF lbl_1_bss_D4[4];
HuVecF lbl_1_bss_A4[4];
HuVecF lbl_1_bss_8C[2];
HuVecF lbl_1_bss_80;
HuVecF lbl_1_bss_50[4];
float lbl_1_bss_38[6];
s16 lbl_1_bss_34;
OMOBJ *lbl_1_bss_30;
s16 lbl_1_bss_2E;
s16 lbl_1_bss_2C;
s16 lbl_1_bss_2A;
s16 lbl_1_bss_28;
OMOBJ *lbl_1_bss_24;
LBL_1_BSS_1C lbl_1_bss_1C;
OMOBJ *lbl_1_bss_18;
OMOBJ *lbl_1_bss_14;
OMOBJ *lbl_1_bss_10;
OMOBJ *lbl_1_bss_C;
OMOBJ *lbl_1_bss_8;
OMOBJ *lbl_1_bss_4;
OMOBJMAN *lbl_1_bss_0;
