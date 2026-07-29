#include <dolphin/mtx/GeoTypes.h>

#include "game/memory.h"

#define HUSPR_ATTR_DISPOFF 0x4

typedef Vec HuVecF;

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_LLIGHTID;
typedef s16 HUSPRID;
typedef s16 HUSPR_GROUPID;
typedef s16 HUWINID;

typedef struct AnimData_s ANIMDATA;
typedef struct Process_s HUPROCESS;
typedef struct HuWinWarning_s HUWIN_WARNING;

typedef struct HuVec2f_s {
    float x;
    float y;
} HuVec2f;

typedef struct WinCharEntry_s {
    u8 color;
    u8 fade;
    s16 x;
    s16 y;
    s16 charNo;
} WINCHARENTRY;

typedef struct WinChoice_s {
    u8 stat;
    s16 x;
    s16 y;
} WINCHOICE;

typedef void (*HUWIN_CALLBACK)(HUWINID window, u32 message, char character);

typedef struct HuWin_s {
    u8 stat;
    u8 padMask;
    u8 disablePlayer;
    u8 bgPalNum;
    HUSPR_GROUPID grpId;
    HUSPRID sprId[31];
    s16 mesSpeed;
    s16 mesTime;
    s16 keyWaitSprNo;
    s16 prio;
    s16 drawNo;
    u32 attr;
    ANIMDATA *animFrame[2];
    s16 mesRectX;
    s16 mesRectW;
    s16 mesRectY;
    s16 mesRectH;
    s16 mesX;
    s16 mesY;
    s16 mesCol;
    s16 mesColShadow;
    s16 charPadX;
    s16 charPadY;
    s16 winW;
    s16 winH;
    HuVec2f pos;
    HuVec2f scale;
    float zRot;
    s16 charEntryNum;
    s16 charEntryMax;
    WINCHARENTRY *charEntry;
    s16 messSp;
    s32 unk94;
    char *messData;
    char *messDataStack[16];
    char *messDataInsert[16];
    char *mesCopy;
    s16 choiceNum;
    s16 choice;
    s16 cursorSprNo;
    u8 choiceDisable[16];
    WINCHOICE choiceData[16];
    s16 scissorX;
    s16 scissorY;
    s16 scissorW;
    s16 scissorH;
    s16 tabW;
    s16 pushKey;
    s16 activePadKey;
    s16 choiceEndSe;
    u8 ATTRIBUTE_ALIGN(32) mesPal[10][3];
    HUWIN_WARNING *warning;
    HUWIN_CALLBACK callback;
    u32 origMes;
} HUWIN;

typedef struct omObj_s OMOBJ;
typedef void (*OMOBJ_FUNC)(OMOBJ *object);

struct omObj_s {
    u16 stat;
    s16 objNext;
    s16 prio;
    s16 prev;
    s16 next;
    s16 nextNo;
    s16 grpNo;
    u16 memberNo;
    u32 mode;
    OMOBJ_FUNC objFunc;
    HuVecF trans;
    HuVecF rot;
    HuVecF scale;
    u16 mdlcnt;
    HU3D_MODELID *mdlId;
    u16 mtncnt;
    HU3D_MOTIONID *mtnId;
    u32 work[4];
    void *data;
};

typedef struct EndingWindowPlayers {
    s16 player[4];
} EndingWindowPlayers;

typedef struct EndingSpritePositions {
    Vec position[10];
} EndingSpritePositions;

extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern float lbl_1_bss_28;
extern s16 lbl_1_bss_1A08[10];
extern s16 lbl_1_bss_1A1C[2];
extern ANIMDATA *lbl_1_bss_1A20[10];
extern s16 lbl_1_bss_1A48[5];
extern s16 lbl_1_bss_1A52[2];

extern s16 lbl_1_data_38;
extern u32 lbl_1_data_0;

extern float lbl_1_rodata_78;
extern float lbl_1_rodata_BC;
extern float lbl_1_rodata_C0;
extern float lbl_1_rodata_C4;
extern float lbl_1_rodata_C8;
extern float lbl_1_rodata_CC;
extern float lbl_1_rodata_D0;
extern HuVecF lbl_1_rodata_D4;
extern HuVecF lbl_1_rodata_E0;
extern HuVecF lbl_1_rodata_EC;
extern float lbl_1_rodata_F8;
extern float lbl_1_rodata_FC;
extern EndingWindowPlayers lbl_1_rodata_100;
extern float lbl_1_rodata_108;
extern float lbl_1_rodata_10C;
extern float lbl_1_rodata_110;
extern float lbl_1_rodata_114;
extern float lbl_1_rodata_118;
extern EndingSpritePositions lbl_1_rodata_11C;
extern float lbl_1_rodata_194;
extern float lbl_1_rodata_198;

extern HUWIN winData[32];

HU3D_LLIGHTID Hu3DLLightCreate(HU3D_MODELID modelId, float posX,
    float posY, float posZ, float dirX, float dirY, float dirZ,
    u8 colorR, u8 colorG, u8 colorB);
void Hu3DLLightInfinitytSet(HU3D_MODELID modelId,
    HU3D_LLIGHTID lightId);
void Hu3DLLightStaticSet(HU3D_MODELID modelId, HU3D_LLIGHTID lightId,
    BOOL staticF);
void Hu3DLLightKill(HU3D_MODELID modelId, HU3D_LLIGHTID lightId);
void Hu3DShadowCreate(float fov, float near, float far);
void Hu3DShadowPosSet(HuVecF *position, HuVecF *up, HuVecF *target);

void HuPrcSleep(s32 time);
void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);

ANIMDATA *HuSprAnimRead(void *data);
HUSPRID HuSprCreate(ANIMDATA *animation, s16 priority, s16 bank);
HUSPR_GROUPID HuSprGrpCreate(s16 spriteCount);
void HuSprGrpMemberSet(HUSPR_GROUPID group, s16 member, HUSPRID sprite);
void HuSprAttrReset(HUSPR_GROUPID group, s16 member, s32 attr);
void HuSprPosSet(HUSPR_GROUPID group, s16 member, float x, float y);
void HuSprGrpPosSet(HUSPR_GROUPID group, float x, float y);
void HuSprGrpDrawNoSet(HUSPR_GROUPID group, s32 drawNo);
void HuSprExecLayerSet(s16 drawNo, s16 layer);

void HuWinInit(s32 messageDataNo);
HUWINID HuWinExCreateFrame(float x, float y, s16 width, s16 height,
    s16 speaker, s16 frame);
void HuWinExOpen(HUWINID window);
void HuWinExClose(HUWINID window);
void HuWinExKill(HUWINID window);
void HuWinAllKill(void);
void HuWinDispOff(HUWINID window);
void HuWinBGTPLvlSet(HUWINID window, float level);
void HuWinAttrSet(HUWINID window, u32 attr);
void HuWinMesSet(HUWINID window, u32 message);
void HuWinCallbackSet(HUWINID window, HUWIN_CALLBACK callback);

void fn_1_98(HUWINID window, u32 message, char character);
void fn_1_E0EC(s16 group, u32 attr);

void fn_1_924(void)
{
    lbl_1_bss_1A52[0] = Hu3DLLightCreate(lbl_1_bss_C->mdlId[0],
        lbl_1_rodata_BC, lbl_1_rodata_C0, lbl_1_rodata_C4,
        lbl_1_rodata_C8, lbl_1_rodata_C8, lbl_1_rodata_CC,
        128, 128, 128);
    Hu3DLLightInfinitytSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_1A52[0]);
    Hu3DLLightStaticSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_1A52[0], 1);

    lbl_1_bss_1A52[1] = Hu3DLLightCreate(lbl_1_bss_10->mdlId[0],
        lbl_1_rodata_D0, lbl_1_rodata_C0, lbl_1_rodata_C4,
        lbl_1_rodata_C8, lbl_1_rodata_C8, lbl_1_rodata_CC,
        128, 128, 128);
    Hu3DLLightInfinitytSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_1A52[1]);
    Hu3DLLightStaticSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_1A52[1], 1);
}

void fn_1_AC8(void)
{
    Hu3DLLightKill(lbl_1_bss_C->mdlId[0], lbl_1_bss_1A52[0]);
    Hu3DLLightKill(lbl_1_bss_10->mdlId[0], lbl_1_bss_1A52[1]);
}

void fn_1_B2C(void)
{
    HuVecF position = lbl_1_rodata_D4;
    HuVecF up = lbl_1_rodata_E0;
    HuVecF target = lbl_1_rodata_EC;

    Hu3DShadowCreate(lbl_1_rodata_F8, lbl_1_rodata_78, lbl_1_rodata_FC);
    Hu3DShadowPosSet(&position, &up, &target);
}

inline void fn_1_B2C(void);

void fn_1_BE0(void)
{
    if (lbl_1_data_38 != -1) {
        HuWinExClose(lbl_1_bss_1A48[lbl_1_data_38]);
        lbl_1_data_38 = -1;
    }
}

void fn_1_C44(s16 window, u32 message)
{
    if (lbl_1_data_38 != -1 && lbl_1_data_38 != window) {
        fn_1_BE0();
    }
    if (lbl_1_data_38 == -1 || lbl_1_data_38 != window) {
        HuWinExOpen(lbl_1_bss_1A48[window]);
        lbl_1_data_38 = window;
    }
    HuWinAttrSet(lbl_1_bss_1A48[window], 0x800);
    HuWinMesSet(lbl_1_bss_1A48[window], message);
    if (lbl_1_data_0 != message) {
        lbl_1_data_0 = -1;
    }
}

void fn_1_DAC(s16 window, u32 message, s16 delay)
{
    if (lbl_1_data_38 != -1 && lbl_1_data_38 != window) {
        fn_1_BE0();
    }
    if (lbl_1_data_38 == -1 || lbl_1_data_38 != window) {
        HuWinExOpen(lbl_1_bss_1A48[window]);
        lbl_1_data_38 = window;
    }
    HuWinAttrSet(lbl_1_bss_1A48[window], 0x800);
    HuWinMesSet(lbl_1_bss_1A48[window], message);
    if (lbl_1_data_0 != message) {
        lbl_1_data_0 = -1;
    }
    if (delay > 0) {
        HuPrcSleep(delay);
    }
}

void fn_1_F34(void)
{
    EndingWindowPlayers players = lbl_1_rodata_100;
    s16 window;

    HuWinInit(1);
    for (window = 0; window < 4; window++) {
        lbl_1_bss_1A48[window] = HuWinExCreateFrame(
            lbl_1_rodata_108, lbl_1_rodata_10C, 0x220, 0x44, -1,
            players.player[window]);
        HuWinDispOff(lbl_1_bss_1A48[window]);
        HuWinBGTPLvlSet(lbl_1_bss_1A48[window], lbl_1_rodata_110);
        winData[lbl_1_bss_1A48[window]].padMask = 1;
    }
    for (window = 0; window < 4; window++) {
        HuWinCallbackSet(lbl_1_bss_1A48[window], fn_1_98);
    }
    lbl_1_bss_1A48[4] = HuWinExCreateFrame(lbl_1_rodata_108,
        lbl_1_rodata_114, 0x220, 0x2A, -1, 0);
    HuWinDispOff(lbl_1_bss_1A48[4]);
    HuWinBGTPLvlSet(lbl_1_bss_1A48[4], lbl_1_rodata_C8);
}

inline void fn_1_F34(void);

void fn_1_1104(void)
{
    s16 window;

    for (window = 0; window < 4; window++) {
        HuWinExKill(lbl_1_bss_1A48[window]);
    }
    HuWinAllKill();
}

void fn_1_1160(OMOBJ *object)
{
    switch (object->work[0]) {
        case 0:
            lbl_1_bss_28 += lbl_1_rodata_118;
            HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_bss_28,
                lbl_1_rodata_C8);
            break;
        case 1:
            lbl_1_bss_28 -= lbl_1_rodata_118;
            HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_bss_28,
                lbl_1_rodata_C8);
            break;
        case 2:
            lbl_1_bss_28 -= lbl_1_rodata_118;
            HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
                lbl_1_bss_28);
            break;
    }
}

void fn_1_12A8(s16 state)
{
    if (state == 2) {
        HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
            lbl_1_rodata_C8);
    } else {
        HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
            lbl_1_rodata_C8);
    }
    fn_1_E0EC(lbl_1_bss_1A1C[0], HUSPR_ATTR_DISPOFF);
    lbl_1_bss_4->work[0] = state;
    lbl_1_bss_4->objFunc = fn_1_1160;

    switch (state) {
        case 0:
            lbl_1_bss_28 = lbl_1_rodata_C8;
            HuSprAttrReset(lbl_1_bss_1A1C[0], 4,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 6,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 3,
                HUSPR_ATTR_DISPOFF);
            break;
        case 1:
            lbl_1_bss_28 = lbl_1_rodata_C8;
            HuSprAttrReset(lbl_1_bss_1A1C[0], 1,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 5,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 8,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 9,
                HUSPR_ATTR_DISPOFF);
            break;
        case 2:
            lbl_1_bss_28 = lbl_1_rodata_C8;
            HuSprAttrReset(lbl_1_bss_1A1C[0], 0,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 2,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 7,
                HUSPR_ATTR_DISPOFF);
            break;
        case 3:
            lbl_1_bss_4->objFunc = NULL;
            break;
    }
}

void fn_1_14F4(void)
{
    EndingSpritePositions positions = lbl_1_rodata_11C;
    s16 index;

    for (index = 0; index < 10; index++) {
        lbl_1_bss_1A20[index] = HuSprAnimRead(HuDataSelHeapReadNum(
            0x220014 + index, HU_MEMNUM_OVL, HEAP_MODEL));
    }
    lbl_1_bss_1A1C[0] = HuSprGrpCreate(10);
    for (index = 0; index < 10; index++) {
        lbl_1_bss_1A08[index] = HuSprCreate(lbl_1_bss_1A20[index],
            positions.position[index].z, 0);
        HuSprGrpMemberSet(lbl_1_bss_1A1C[0], index,
            lbl_1_bss_1A08[index]);
        HuSprPosSet(lbl_1_bss_1A1C[0], index,
            positions.position[index].x, positions.position[index].y);
    }
    HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_rodata_C8,
        lbl_1_rodata_C8);
    fn_1_E0EC(lbl_1_bss_1A1C[0], HUSPR_ATTR_DISPOFF);
    HuSprExecLayerSet(0x40, 2);
    HuSprGrpDrawNoSet(lbl_1_bss_1A1C[0], 0x40);
}

inline void fn_1_14F4(void);

void fn_1_16D4(void)
{
    s16 sprite;
    ANIMDATA *animation;

    animation = HuSprAnimRead(HuDataSelHeapReadNum(
        0x220013, HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_1A1C[1] = HuSprGrpCreate(1);
    sprite = HuSprCreate(animation, 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_1A1C[1], 0, sprite);
    HuSprGrpPosSet(lbl_1_bss_1A1C[1], lbl_1_rodata_194,
        lbl_1_rodata_198);
    fn_1_E0EC(lbl_1_bss_1A1C[1], HUSPR_ATTR_DISPOFF);
}
