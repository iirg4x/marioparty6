#include "dolphin.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/process.h"
#include "game/saveload.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"

typedef struct MdbankAnimSet {
    s16 model[2];
    s16 anim[6];
    s16 spriteGroup;
    s16 state[4];
} MDBANK_ANIM_SET;

typedef union MdbankItem {
    s16 value[16];
    struct {
        s16 value[14];
        s32 message;
    } choice;
} MDBANK_ITEM;

typedef struct MdbankSpriteConfig {
    s16 group;
    s16 member;
    s16 anim;
    s16 priority;
    s16 bank;
    float x;
    float y;
    float scaleX;
    float scaleY;
    float rotation;
} MDBANK_SPRITE_CONFIG;

typedef void (*MDBANK_VOID_FUNC)(void);

typedef struct MdbankMoveWork {
    s16 active;
    s16 unk_02;
    float time;
    float duration;
    HuVecF start;
    HuVecF control;
    HuVecF end;
} MDBANK_MOVE_WORK;

typedef struct MdbankNameTable {
    char *name[3][3];
} MDBANK_NAME_TABLE;

typedef struct MdbankMessageTable {
    s32 message[2];
} MDBANK_MESSAGE_TABLE;

typedef struct MdbankFxTable {
    s32 fx[16];
} MDBANK_FX_TABLE;

typedef struct MdbankCameraWork MDBANK_CAMERA_WORK;
typedef void (*MDBANK_CAMERA_CALLBACK)(OMOBJ *obj, MDBANK_CAMERA_WORK *work);

struct MdbankCameraWork {
    OMOBJ *obj;
    HuVecF center;
    HuVecF centerTarget;
    HuVecF rot;
    HuVecF rotTarget;
    float zoom;
    float zoomTarget;
    MDBANK_CAMERA_CALLBACK callback;
    s32 state[4];
};

extern u32 lbl_1_data_90C;
extern s32 lbl_1_data_0;
extern s32 lbl_1_data_4[12];
extern s16 lbl_1_data_34[4];
extern MDBANK_SPRITE_CONFIG lbl_1_data_3C[11];
extern MDBANK_ITEM lbl_1_data_22C[55];
extern s16 lbl_1_data_964[4];
extern s16 lbl_1_data_932[3];
extern s32 lbl_1_data_938[3];
extern s16 lbl_1_data_970;
extern s16 lbl_1_data_972;
extern s16 lbl_1_data_974;
extern char lbl_1_data_956[7];
extern char lbl_1_data_95D[7];
extern char lbl_1_data_944[];
extern char lbl_1_data_9E8[0x34];
extern char lbl_1_data_910[];
extern s32 lbl_1_data_A20[2];
extern const float lbl_1_rodata_68;
extern const MDBANK_MESSAGE_TABLE lbl_1_rodata_10;
extern const MDBANK_FX_TABLE lbl_1_rodata_18;
extern const float lbl_1_rodata_58;
extern const float lbl_1_rodata_5C;
extern const float lbl_1_rodata_60;
extern const float lbl_1_rodata_64;
extern const float lbl_1_rodata_6C;
extern const float lbl_1_rodata_70;
extern const float lbl_1_rodata_74;
extern const float lbl_1_rodata_78;
extern const float lbl_1_rodata_7C;
extern const float lbl_1_rodata_80;
extern const float lbl_1_rodata_84;
extern const float lbl_1_rodata_88;
extern const float lbl_1_rodata_E4;
extern const float lbl_1_rodata_E8;
extern const float lbl_1_rodata_EC;
extern const float lbl_1_rodata_F0;
extern const float lbl_1_rodata_F4;
extern const float lbl_1_rodata_F8;
extern const float lbl_1_rodata_FC;
extern const float lbl_1_rodata_100;
extern const float lbl_1_rodata_104;
extern const float lbl_1_rodata_108;
extern const float lbl_1_rodata_10C;
extern const float lbl_1_rodata_110;
extern const float lbl_1_rodata_114;
extern const float lbl_1_rodata_118;
extern const float lbl_1_rodata_11C;
extern const float lbl_1_rodata_120;
extern const float lbl_1_rodata_124;
extern const float lbl_1_rodata_130;
extern const float lbl_1_rodata_134;
extern const float lbl_1_rodata_138;
extern const float lbl_1_rodata_13C;
extern const float lbl_1_rodata_140;
extern const float lbl_1_rodata_144;
extern const HuVecF lbl_1_rodata_148;
extern const float lbl_1_rodata_154;
extern const float lbl_1_rodata_158;
extern const float lbl_1_rodata_15C;
extern const float lbl_1_rodata_160;
extern const float lbl_1_rodata_164;
extern const float lbl_1_rodata_168;
extern const float lbl_1_rodata_178;
extern const MDBANK_NAME_TABLE lbl_1_rodata_190;
extern const float lbl_1_rodata_1B4;
extern const float lbl_1_rodata_1E0;
extern const HuVecF lbl_1_rodata_1E8;
extern const float lbl_1_rodata_1F4;
extern const float lbl_1_rodata_1F8;
extern const float lbl_1_rodata_1FC;
extern const float lbl_1_rodata_1E4;
extern const float lbl_1_rodata_240;
extern const float lbl_1_rodata_268;
extern const float lbl_1_rodata_274;
extern const float lbl_1_rodata_298;
extern const float lbl_1_rodata_29C;
extern const float lbl_1_rodata_2A8;
extern s16 lbl_1_bss_198C[4];
extern s16 lbl_1_bss_1994[2];
extern MDBANK_CAMERA_WORK lbl_1_bss_1998;
extern HUSPRID lbl_1_bss_1938[11];
extern HUSPR_GROUPID lbl_1_bss_194E[5];
extern ANIMDATA *lbl_1_bss_1958[12];
extern s16 lbl_1_bss_19E8;
extern s16 lbl_1_bss_19EA;
extern s16 lbl_1_bss_19EC;
extern s16 lbl_1_bss_1930[4];
extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_14;
extern OMOBJ *lbl_1_bss_18;
extern OMOBJ *lbl_1_bss_1C;
extern OMOBJ *lbl_1_bss_20;
extern OMOBJ *lbl_1_bss_24[2];
extern s32 lbl_1_bss_2C;
extern u32 lbl_1_bss_30;
extern s16 lbl_1_bss_200;
extern float lbl_1_bss_1700[4];
extern HuVecF lbl_1_bss_1710;
extern MDBANK_MOVE_WORK lbl_1_bss_171C;
extern MDBANK_MOVE_WORK lbl_1_bss_175C;
extern MDBANK_ANIM_SET lbl_1_bss_179C[10];
extern ANIMDATA *lbl_1_bss_18A0[36];
extern ANIMDATA *lbl_1_bss_19F0[2];
extern s32 lbl_1_bss_1988;
extern MDBANK_MOVE_WORK lbl_1_bss_1C0;
extern HuVecF lbl_1_bss_204;
extern HuVecF lbl_1_bss_210[20];
extern HuVecF lbl_1_data_9D0;
extern const MDBANK_VOID_FUNC _ctors[];
extern const MDBANK_VOID_FUNC _dtors[];

void fn_1_11880(HUSPR_GROUPID groupId, s32 attr);
void fn_1_0(HUWINID winId, u32 mess, s16 index);
void fn_1_11900(HUSPR_GROUPID groupId, s32 attr);
void fn_1_4360(MDBANK_ANIM_SET *set, MDBANK_ITEM *item);
void fn_1_119C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_12B60(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_124F4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
short fn_1_EA98(s32 arg0, s32 arg1);
void fn_1_3788(OMOBJ *obj);
void fn_1_86C0(void);
void fn_1_91A4(void);
float fn_1_111B0(float current, float target, float weight);
void fn_1_111F8(HuVecF *current, const HuVecF *target, float weight);
float fn_1_1161C(float start, float control, float end, float weight);
void fn_1_2750(OMOBJ *obj);
void fn_1_13764(s16 time);
void fn_1_1115C(HuVecF *out, float x, float y, float z);
void fn_1_226C(OMOBJ *obj);
void fn_1_5988(OMOBJ *obj);
void fn_1_11980(float x, float y, float z);
void fn_1_12A94(float x, float y, float z);
void fn_1_3640(void);
float fn_1_11458(float start, float end, float time, float duration);
void fn_1_11678(HuVecF *out, const HuVecF *start, const HuVecF *control, const HuVecF *end,
    float weight);

void fn_1_0(HUWINID winId, u32 mess, s16 index)
{
    MDBANK_MESSAGE_TABLE messages = lbl_1_rodata_10;
    MDBANK_FX_TABLE effects = lbl_1_rodata_18;
    s16 i;

    index--;
    OSReport(lbl_1_data_910, index);
    if (lbl_1_data_90C != mess) {
        lbl_1_data_90C = mess;
        for (i = 0;; i++) {
            if (messages.message[i] == -1) {
                HuAudFXPlay(effects.fx[index]);
                break;
            }
            if (mess == messages.message[i]) {
                if (index >= 8) {
                    HuAudFXPlayPan(effects.fx[index], 0x50);
                } else {
                    HuAudFXPlayPan(effects.fx[index], 0x30);
                }
                break;
            }
        }
    }
}

void fn_1_1AC(void)
{
    s16 i;

    lbl_1_bss_1930[0] = GWBankStarGet();
    lbl_1_bss_1930[1] = 0;
    if (lbl_1_bss_2C == 0) {
        lbl_1_bss_1930[2] = 0;
        lbl_1_bss_1930[3] = 0;
    } else if (lbl_1_bss_30 == 1) {
        lbl_1_bss_1930[2] = 0;
        lbl_1_bss_1930[3] = 0;
    } else {
        lbl_1_bss_1930[1] = 9;
        lbl_1_bss_1930[2] = 4;
        lbl_1_bss_1930[3] = 1;
    }
    for (i = 0; i < 55; i++) {
        lbl_1_data_22C[i].value[10] = 0;
        if (GWBankFlagGet(lbl_1_data_22C[i].value[2])) {
            lbl_1_data_22C[i].value[10] = 1;
        }
    }
}

void fn_1_308(void)
{
    s16 i;

    if (lbl_1_bss_1930[0] < 0) {
        lbl_1_bss_1930[0] = 0;
    }
    if (lbl_1_bss_1930[0] > 9999) {
        lbl_1_bss_1930[0] = 9999;
    }
    GwCommon.bankStar = lbl_1_bss_1930[0];
    for (i = 0; i < 55; i++) {
        if (lbl_1_data_22C[i].value[10] == 1) {
            GWBankFlagSet(lbl_1_data_22C[i].value[2]);
        }
    }
    if (GWBankFlagGet(6)) {
        GWMgUnlockSet(0x2A5);
    }
}

void fn_1_8B4(void)
{
    Hu3DGLightKill(lbl_1_bss_1994[0]);
    Hu3DGLightKill(lbl_1_bss_1994[1]);
}

void fn_1_9A4(void)
{
}

void fn_1_9A8(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_198C[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_198C[winNo]);
    }
}

inline void fn_1_9A8(s16 winNo);

void fn_1_A18(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_198C[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_198C[winNo]);
    }
}

inline void fn_1_A18(s16 winNo);

void fn_1_A88(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_198C[winNo]);
}

inline void fn_1_A88(s16 winNo);

s16 fn_1_AC4(s16 winNo, s16 mode)
{
    s16 choice = 0;

    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_198C[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_198C[winNo], HUWIN_ATTR_NOCANCEL);
    }
    choice = HuWinChoiceGet(lbl_1_bss_198C[winNo], -1);
    if (mode == 2 && choice == -1) {
        choice = 1;
    }
    return choice;
}

inline s16 fn_1_AC4(s16 winNo, s16 mode);

void fn_1_B98(s16 winNo, u32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_198C[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_198C[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_198C[winNo], speed);
    if (lbl_1_data_90C != messNum) {
        lbl_1_data_90C = -1;
    }
}

inline void fn_1_B98(s16 winNo, u32 messNum, s16 speed);

void fn_1_C54(s16 winNo, u32 messNum, s16 insertMesNo)
{
    HuWinAttrSet(lbl_1_bss_198C[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinHomeClear(lbl_1_bss_198C[winNo]);
    HuWinInsertMesSet(lbl_1_bss_198C[winNo], messNum, insertMesNo);
}

void fn_1_CE4(void)
{
    s16 i;

    HuWinInit(1);
    lbl_1_bss_198C[0] = HuWinExCreateFrame(lbl_1_rodata_E4,
        lbl_1_rodata_E8, 0x220, 0x2A, -1, 0);
    HuWinDispOff(lbl_1_bss_198C[0]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[0], lbl_1_rodata_68);
    lbl_1_bss_198C[1] = HuWinExCreateFrame(lbl_1_rodata_E4,
        lbl_1_rodata_EC, 0x220, 0x44, -1, 5);
    HuWinDispOff(lbl_1_bss_198C[1]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[1], lbl_1_rodata_F0);
    lbl_1_bss_198C[2] = HuWinExCreateFrame(lbl_1_rodata_E4,
        lbl_1_rodata_EC, 0x220, 0x44, -1, 3);
    HuWinDispOff(lbl_1_bss_198C[2]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[2], lbl_1_rodata_F0);
    lbl_1_bss_198C[3] = HuWinExCreateFrame(lbl_1_rodata_E4,
        lbl_1_rodata_EC, 0x220, 0x44, -1, 4);
    HuWinDispOff(lbl_1_bss_198C[3]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[3], lbl_1_rodata_F0);
    for (i = 0; i < 4; i++) {
        winData[lbl_1_bss_198C[i]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_198C[i], (HUWIN_CALLBACK)fn_1_0);
    }
}

void fn_1_F0C(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        HuWinExKill(lbl_1_bss_198C[i]);
    }
    HuWinAllKill();
}

void fn_1_F68(s16 winNo)
{
    if (lbl_1_data_932[0] != -1 && lbl_1_data_932[0] != winNo) {
        HuWinHomeClear(lbl_1_data_932[0]);
        fn_1_A18(lbl_1_data_932[0]);
    }
    if (lbl_1_data_932[0] == -1 || lbl_1_data_932[0] != winNo) {
        lbl_1_data_932[0] = winNo;
        lbl_1_data_938[0] = -1;
        lbl_1_data_938[1] = -1;
        fn_1_9A8(lbl_1_data_932[0]);
    }
}

void fn_1_10E4(void)
{
    if (lbl_1_data_932[0] != -1) {
        fn_1_A18(lbl_1_data_932[0]);
    }
    lbl_1_data_932[0] = -1;
    lbl_1_data_938[0] = -1;
    lbl_1_data_938[1] = -1;
}

void fn_1_11A0(void)
{
    if (lbl_1_data_932[0] != -1) {
        fn_1_A88(lbl_1_data_932[0]);
    }
}

s16 fn_1_1200(s16 mode)
{
    if (lbl_1_data_932[0] != -1) {
        return fn_1_AC4(lbl_1_data_932[0], mode);
    }
    return 0;
}

void fn_1_12F8(s16 winNo, s32 messNum, s16 speed)
{
    fn_1_F68(winNo);
    if (lbl_1_data_938[0] != messNum) {
        lbl_1_data_938[0] = messNum;
        lbl_1_data_938[1] = -1;
        fn_1_B98(lbl_1_data_932[0], lbl_1_data_938[0], speed);
    }
}

void fn_1_1540(s16 winNo, s32 messNum, s16 insertMesNo)
{
    fn_1_F68(winNo);
    if (lbl_1_data_938[1] != messNum) {
        lbl_1_data_938[0] = -1;
        lbl_1_data_938[1] = messNum;
        fn_1_C54(lbl_1_data_932[0], lbl_1_data_938[1], insertMesNo);
    }
}

void fn_1_1764(s32 messNum)
{
    if (lbl_1_data_932[1] == -1) {
        lbl_1_data_932[1] = 0;
        lbl_1_data_938[2] = -1;
        fn_1_9A8(lbl_1_data_932[1]);
    }
    if (lbl_1_data_938[2] != messNum) {
        lbl_1_data_938[2] = messNum;
        fn_1_B98(lbl_1_data_932[1], lbl_1_data_938[2], 0);
    }
}

void fn_1_18E8(void)
{
    if (lbl_1_data_932[1] != -1) {
        fn_1_A18(lbl_1_data_932[1]);
    }
    lbl_1_data_932[1] = -1;
    lbl_1_data_938[2] = -1;
}

void fn_1_1BD0(void)
{
}

void fn_1_1BD4(OMOBJ *obj)
{
    if (obj->work[3]++ > 45) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_74);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_rodata_68,
            lbl_1_rodata_F4, HU3D_MOTATTR_LOOP);
    }
}

void fn_1_1C64(void)
{
    OMOBJ *obj = lbl_1_bss_C;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1], lbl_1_rodata_68,
        lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_1BD4;
}

void fn_1_1CD8(OMOBJ *obj)
{
    if (obj->work[3]++ > 45) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_74);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_rodata_68,
            lbl_1_rodata_F4, HU3D_MOTATTR_LOOP);
    }
}

void fn_1_1D68(void)
{
    OMOBJ *obj = lbl_1_bss_10;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1], lbl_1_rodata_68,
        lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_1CD8;
}

void fn_1_1DDC(OMOBJ *obj)
{
    MDBANK_MOVE_WORK *work = &lbl_1_bss_175C;
    HuVecF position;
    float rotation;

    fn_1_11678(&position, &work->start, &work->control, &work->end,
        fn_1_11458(lbl_1_rodata_68, lbl_1_rodata_74, work->time, work->duration));
    Hu3DModelPosSetV(obj->mdlId[0], &position);
    rotation = fn_1_11458(lbl_1_rodata_68, lbl_1_rodata_F4, work->time, work->duration);
    Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_68, rotation, lbl_1_rodata_68);
    if ((work->time += lbl_1_rodata_74) > work->duration) {
        obj->objFunc = NULL;
    }
}

void fn_1_1EE8(void)
{
    OMOBJ *obj = lbl_1_bss_C;
    MDBANK_MOVE_WORK *work = &lbl_1_bss_175C;

    fn_1_1115C(&work->start, lbl_1_rodata_FC, lbl_1_rodata_100, lbl_1_rodata_68);
    fn_1_1115C(&work->control, lbl_1_rodata_104, lbl_1_rodata_100, lbl_1_rodata_68);
    fn_1_1115C(&work->end, lbl_1_rodata_108, lbl_1_rodata_10C, lbl_1_rodata_68);
    work->time = lbl_1_rodata_68;
    work->duration = lbl_1_rodata_110;
    obj->objFunc = fn_1_1DDC;
}

void fn_1_1FD8(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(0x940040);
    for (i = 0; i < 3; i++) {
        obj->mtnId[i] = Hu3DJointMotionData(obj->mdlId[0], 0x940041 + i);
    }
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_rodata_68,
        lbl_1_rodata_68, HU3D_MOTATTR_LOOP);
    if (lbl_1_bss_2C == 0) {
        Hu3DModelPosSet(obj->mdlId[0], lbl_1_rodata_FC, lbl_1_rodata_100,
            lbl_1_rodata_68);
        Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_68, lbl_1_rodata_68,
            lbl_1_rodata_68);
    } else {
        Hu3DModelPosSet(obj->mdlId[0], lbl_1_rodata_108, lbl_1_rodata_10C,
            lbl_1_rodata_68);
        Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_68, lbl_1_rodata_F4,
            lbl_1_rodata_68);
    }
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_114, lbl_1_rodata_114,
        lbl_1_rodata_114);
    Hu3DModelShadowSet(obj->mdlId[0]);
    obj->objFunc = NULL;
}

void fn_1_8280(void)
{
}

void fn_1_5834(void)
{
    OMOBJ *obj = lbl_1_bss_18;
    MDBANK_ANIM_SET *set = lbl_1_bss_179C;
    s16 i;

    for (i = 0; i < 10; i++, set++) {
        Hu3DModelAttrSet(set->model[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(set->model[1], HU3D_ATTR_DISPOFF);
        fn_1_11880(set->spriteGroup, HUSPR_ATTR_DISPOFF);
    }
}

void fn_1_6B34(void)
{
    fn_1_11880(lbl_1_bss_194E[2], HUSPR_ATTR_DISPOFF);
}

void fn_1_8340(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        HuWinDispOff(lbl_1_bss_198C[i]);
    }
    HuSprPriSet(lbl_1_bss_194E[3], 0, 0x157C);
    fn_1_11900(lbl_1_bss_194E[3], HUSPR_ATTR_DISPOFF);
    HuPrcSleep(5);
}

void fn_1_1994(void)
{
    MDBANK_SPRITE_CONFIG *config = lbl_1_data_3C;
    s16 i;

    for (i = 0; i < 12; i++) {
        lbl_1_bss_1958[i] = HuSprAnimDataRead(lbl_1_data_4[i]);
    }
    for (i = 0; i < 4; i++) {
        lbl_1_bss_194E[i] = HuSprGrpCreate(lbl_1_data_34[i]);
    }
    for (i = 0; i < 11; i++, config++) {
        lbl_1_bss_1938[i] = HuSprCreate(lbl_1_bss_1958[config->anim], config->priority + 6000,
            config->bank);
        HuSprGrpMemberSet(lbl_1_bss_194E[config->group], config->member, lbl_1_bss_1938[i]);
        HuSprPosSet(lbl_1_bss_194E[config->group], config->member, config->x, config->y);
        HuSprScaleSet(lbl_1_bss_194E[config->group], config->member, config->scaleX, config->scaleY);
        HuSprZRotSet(lbl_1_bss_194E[config->group], config->member, config->rotation);
    }
    for (i = 0; i < 4; i++) {
        fn_1_11880(lbl_1_bss_194E[i], HUSPR_ATTR_DISPOFF);
    }
    HuSprExecLayerCameraSet(0x40, 1, 2);
    HuSprExecLayerCameraSet(0x41, 1, 4);
}

void fn_1_5550(void)
{
    s16 itemIndex = 0;
    MDBANK_ANIM_SET *set = lbl_1_bss_179C;
    s16 i;
    MDBANK_ITEM *item;

    for (i = 0; i < 10; i++, set++) {
        itemIndex = i + (lbl_1_bss_1930[1] * 5);
        if (itemIndex >= 55) {
            itemIndex -= 55;
        }
        item = &lbl_1_data_22C[itemIndex];
        Hu3DAnimAnimSet(set->anim[0], lbl_1_bss_18A0[31]);
        Hu3DAnimAnimSet(set->anim[1], lbl_1_bss_18A0[31]);
        Hu3DAnimAnimSet(set->anim[2], lbl_1_bss_18A0[31]);
        Hu3DAnimAnimSet(set->anim[3], lbl_1_bss_18A0[item->value[4]]);
        if (item->value[7] != -1) {
            Hu3DAnimBankSet(set->anim[3], item->value[7]);
        }
        if (item->value[6] != -1) {
            Hu3DAnimAnimSet(set->anim[4], lbl_1_bss_18A0[item->value[6]]);
            if (item->value[9] != -1) {
                Hu3DAnimBankSet(set->anim[4], item->value[9]);
            }
        } else {
            Hu3DAnimAnimSet(set->anim[4], lbl_1_bss_18A0[0]);
        }
        Hu3DAnimAnimSet(set->anim[5], lbl_1_bss_18A0[0]);
        if (set->state[3] == 0) {
            fn_1_4360(set, item);
        } else {
            fn_1_11880(set->spriteGroup, HUSPR_ATTR_DISPOFF);
        }
        if (item->value[10] != 0) {
            if (item->value[5] != -1) {
                Hu3DAnimAnimSet(set->anim[0], lbl_1_bss_18A0[item->value[5]]);
            } else {
                Hu3DAnimAnimSet(set->anim[0], lbl_1_bss_18A0[item->value[4]]);
            }
            if (item->value[8] != -1) {
                Hu3DAnimBankSet(set->anim[0], item->value[8]);
            }
            if (item->value[6] != -1) {
                Hu3DAnimAnimSet(set->anim[1], lbl_1_bss_18A0[item->value[6]]);
                if (item->value[9] != -1) {
                    Hu3DAnimBankSet(set->anim[1], item->value[9]);
                }
            } else {
                Hu3DAnimAnimSet(set->anim[1], lbl_1_bss_18A0[0]);
            }
            Hu3DAnimAnimSet(set->anim[2], lbl_1_bss_18A0[32]);
            Hu3DAnimAnimSet(set->anim[3], lbl_1_bss_18A0[1]);
            Hu3DAnimAnimSet(set->anim[4], lbl_1_bss_18A0[1]);
            Hu3DAnimAnimSet(set->anim[5], lbl_1_bss_18A0[32]);
            fn_1_11880(set->spriteGroup, HUSPR_ATTR_DISPOFF);
        }
    }
}

void fn_1_83CC(void)
{
    s16 result = fn_1_EA98(0, 0);
    s16 i;

    HuAudSStreamFadeOut(lbl_1_bss_1988, 1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    HuAudFadeOut(1000);
    if (lbl_1_bss_1930[0] < 0) {
        lbl_1_bss_1930[0] = 0;
    }
    if (lbl_1_bss_1930[0] > 9999) {
        lbl_1_bss_1930[0] = 9999;
    }
    GwCommon.bankStar = lbl_1_bss_1930[0];
    for (i = 0; i < 55; i++) {
        if (lbl_1_data_22C[i].value[10] == 1) {
            GWBankFlagSet(lbl_1_data_22C[i].value[2]);
        }
    }
    if (GWBankFlagGet(6)) {
        GWMgUnlockSet(0x2A5);
    }
    if (result == 0) {
        s16 j;

        for (j = 0; j < 4; j++) {
            HuWinDispOff(lbl_1_bss_198C[j]);
        }
        HuSprPriSet(lbl_1_bss_194E[3], 0, 0x157C);
        fn_1_11900(lbl_1_bss_194E[3], HUSPR_ATTR_DISPOFF);
        HuPrcSleep(5);
        SLSaveModeExec(0);
    }
    {
        MDBANK_CAMERA_WORK *camera;
        s16 j;

        for (j = 0; j < 4; j++) {
            HuWinExKill(lbl_1_bss_198C[j]);
        }
        HuWinAllKill();
        Hu3DGLightKill(lbl_1_bss_1994[0]);
        Hu3DGLightKill(lbl_1_bss_1994[1]);
        camera = &lbl_1_bss_1998;
        Hu3DCameraKill(1);
        if (camera->obj) {
            omDelObjEx(lbl_1_bss_0, camera->obj);
        }
        camera->obj = NULL;
    }
    switch (result) {
        case 0:
            omOvlReturn(1);
            break;
        case 1: {
            OMOVLHIS *history = omOvlHisGet(0);

            omOvlHisChg(0, history->ovl, 1, 1);
            omOvlCall(108, 0, 0);
            break;
        }
        case 2: {
            OMOVLHIS *history = omOvlHisGet(0);

            omOvlHisChg(0, history->ovl, 1, 2);
            omOvlCall(120, 0, 0);
            break;
        }
    }
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

void fn_1_281C(void)
{
    if (lbl_1_bss_2C == 0) {
        HuSprGrpPosSet(lbl_1_bss_194E[0], lbl_1_rodata_130,
            lbl_1_rodata_134);
        fn_1_11900(lbl_1_bss_194E[0], HUSPR_ATTR_DISPOFF);
    }
}

inline void fn_1_281C(void);

void fn_1_28B4(OMOBJ *obj)
{
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(0x940000);
    obj->mtnId[0] = Hu3DMotionIDGet(obj->mdlId[0]);
    obj->mdlId[1] = Hu3DModelCreateData(0x940001);
    obj->mtnId[1] = Hu3DMotionIDGet(obj->mdlId[0]);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_rodata_68,
        lbl_1_rodata_68, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(obj->mdlId[1], 1);
    Hu3DMotionShiftSet(obj->mdlId[1], obj->mtnId[1], lbl_1_rodata_68,
        lbl_1_rodata_68, HU3D_MOTATTR_LOOP);
    Hu3DModelTPLvlSet(obj->mdlId[1], lbl_1_rodata_138);
    Hu3DModelShadowMapSet(obj->mdlId[0]);
    fn_1_281C();
    obj->objFunc = NULL;
}

void fn_1_11880(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

void fn_1_11900(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}

void fn_1_6C6C(void)
{
    HuVecF pos = lbl_1_rodata_1E8;
    MDBANK_MOVE_WORK *work;
    s16 i;

    lbl_1_data_9D0.x = pos.x;
    lbl_1_data_9D0.y = pos.y;
    lbl_1_data_9D0.z = pos.z;
    work = &lbl_1_bss_1C0;
    work->active = 1;
    if (lbl_1_bss_2C == 0) {
        work->time = lbl_1_rodata_68;
        work->duration = lbl_1_rodata_134;
    } else {
        work->time = lbl_1_rodata_68;
        work->duration = lbl_1_rodata_74;
    }
    work->start.x = lbl_1_rodata_68;
    work->start.y = lbl_1_rodata_1F4;
    work->start.z = lbl_1_rodata_1F8;
    work->control.x = lbl_1_rodata_68;
    work->control.y = lbl_1_rodata_11C;
    work->control.z = lbl_1_rodata_1FC;
    work->end.x = lbl_1_rodata_68;
    work->end.y = lbl_1_rodata_1E0;
    work->end.z = lbl_1_rodata_FC;
    Hu3DModelPosSet(lbl_1_bss_1C->mdlId[0], pos.x, pos.y, pos.z);
    for (i = 0; i < 12; i++) {
        Hu3DModelPosSet(lbl_1_bss_24[0]->mdlId[i], pos.x, pos.y, pos.z);
    }
    fn_1_11980(pos.x, pos.y, pos.z);
    fn_1_12A94(pos.x, pos.y, pos.z);
    for (i = 0; i < 20; i++) {
        Hu3DModelPosSet(lbl_1_bss_20->mdlId[i],
            pos.x + lbl_1_bss_210[i].x,
            pos.y + lbl_1_bss_210[i].y,
            pos.z + lbl_1_bss_210[i].z);
    }
}

void fn_1_11980(float x, float y, float z)
{
    Hu3DModelPosSet(lbl_1_bss_19EC, x, y, z);
}

void fn_1_724(void)
{
    MDBANK_CAMERA_WORK *camera = &lbl_1_bss_1998;

    Hu3DCameraKill(1);
    if (camera->obj) {
        omDelObjEx(lbl_1_bss_0, camera->obj);
    }
    camera->obj = NULL;
}

void fn_1_2A50(OMOBJ *obj)
{
    if (obj) {
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_2AA4(OMOBJ *obj)
{
    HuVecF rot;

    Hu3DModelRotGet(obj->mdlId[0], &rot);
    rot.z -= lbl_1_rodata_13C;
    if (rot.z < lbl_1_rodata_68) {
        rot.z += lbl_1_rodata_140;
    }
    Hu3DModelRotSetV(obj->mdlId[0], &rot);
}

void fn_1_2B34(OMOBJ *obj)
{
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(0x940002);
    Hu3DModelPosSet(obj->mdlId[0], lbl_1_rodata_68, lbl_1_rodata_FC,
        lbl_1_rodata_144);
    Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_68, lbl_1_rodata_68,
        lbl_1_rodata_68);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    obj->mtnId[0] = Hu3DAnimCreate(lbl_1_bss_1958[0], obj->mdlId[0],
        lbl_1_data_956);
    obj->mtnId[1] = Hu3DAnimCreate(lbl_1_bss_1958[1], obj->mdlId[0],
        lbl_1_data_95D);
    obj->objFunc = fn_1_2AA4;
}

void fn_1_2C54(OMOBJ *obj)
{
    if (obj) {
        Hu3DAnimKill(obj->mtnId[0]);
        Hu3DAnimKill(obj->mtnId[1]);
        obj->mtnId[0] = -1;
        obj->mtnId[1] = -1;
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_2CD8(s16 index, HuVecF *worldPos, float offsetX, float offsetY)
{
    HuVecF screenPos = lbl_1_rodata_148;

    if (worldPos) {
        Hu3D3Dto2D(worldPos, 1, &screenPos);
    }
    HuSprPosSet(lbl_1_bss_194E[1], index,
        screenPos.x + offsetX, screenPos.y + offsetY);
}

void fn_1_2D70(s16 index)
{
    lbl_1_bss_1700[index] = lbl_1_rodata_154;
}

void fn_1_35E0(OMOBJ *obj)
{
    if (obj) {
        Hu3DMotionKill(obj->mdlId[0]);
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_8284(void)
{
    MDBANK_CAMERA_WORK *camera;
    s16 i;

    for (i = 0; i < 4; i++) {
        HuWinExKill(lbl_1_bss_198C[i]);
    }
    HuWinAllKill();
    Hu3DGLightKill(lbl_1_bss_1994[0]);
    Hu3DGLightKill(lbl_1_bss_1994[1]);
    camera = &lbl_1_bss_1998;
    Hu3DCameraKill(1);
    if (camera->obj) {
        omDelObjEx(lbl_1_bss_0, camera->obj);
    }
    camera->obj = NULL;
}

void fn_1_46C(OMOBJ *obj, MDBANK_CAMERA_WORK *camera)
{
    if (camera->callback) {
        camera->callback(obj, camera);
    }
}

void fn_1_4B8(OMOBJ *obj)
{
    MDBANK_CAMERA_WORK *camera = &lbl_1_bss_1998;

    if (camera->callback) {
        camera->callback(obj, camera);
    }
    Center.x = camera->center.x;
    Center.y = camera->center.y;
    Center.z = camera->center.z;
    CRot.x = camera->rot.x;
    CRot.y = camera->rot.y;
    CRot.z = camera->rot.z;
    CZoom = camera->zoom;
    omOutView(obj);
}

void fn_1_4730(void)
{
    s16 i;

    for (i = 0; i < 10; i++) {
        Hu3DModelRotSet(lbl_1_bss_179C[i].model[0], lbl_1_rodata_68,
            lbl_1_rodata_68, lbl_1_rodata_68);
    }
}

void fn_1_3280(void)
{
    OMOBJ *obj = lbl_1_bss_14;

    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
}

void fn_1_4C64(s16 index)
{
    OMOBJ *obj = lbl_1_bss_18;

    obj->work[0] = 0;
    obj->work[1] = 1;
    obj->work[2] = index;
}

void fn_1_6B0C(void)
{
    OMOBJ *obj = lbl_1_bss_1C;

    obj->work[2] = 1;
}

void fn_1_6B64(void)
{
    OMOBJ *obj = lbl_1_bss_1C;
    HU3D_MODEL *model = &Hu3DData[obj->mdlId[0]];

    Hu3DMotionShapeSet(obj->mdlId[0], obj->mtnId[0]);
    model->motShapeWork.speed = lbl_1_rodata_154;
}

void fn_1_6BDC(void)
{
    OMOBJ *obj = lbl_1_bss_1C;
    HU3D_MODEL *model = &Hu3DData[obj->mdlId[0]];

    Hu3DMotionShapeSet(obj->mdlId[0], obj->mtnId[0]);
    model->motShapeWork.speed = lbl_1_rodata_1E4;
    Hu3DMotionShapeTimeSet(obj->mdlId[0],
        Hu3DMotionShapeMaxTimeGet(obj->mdlId[0]));
}

void fn_1_92A4(void)
{
    if (lbl_1_bss_1930[0] > 80) {
        lbl_1_data_0 = HuAudFXPlay(0x58F);
    } else if (lbl_1_bss_1930[0] > 60) {
        lbl_1_data_0 = HuAudFXPlay(0x58E);
    } else if (lbl_1_bss_1930[0] > 40) {
        lbl_1_data_0 = HuAudFXPlay(0x58D);
    } else if (lbl_1_bss_1930[0] > 20) {
        lbl_1_data_0 = HuAudFXPlay(0x58C);
    } else {
        HuAudFXStop(lbl_1_data_0);
    }
}

void fn_1_21E4(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 3; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_26B0(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        Hu3DModelHookReset(obj->mdlId[1]);
        for (i = 0; i < 3; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        Hu3DModelKill(obj->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_4184(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 49; i >= 0; i--) {
            Hu3DModelKill(obj->mdlId[i]);
        }
        Hu3DMotionKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_61A8(OMOBJ *obj)
{
    s16 i;
    s16 j;

    if (obj) {
        for (i = 9; i >= 0; i--) {
            for (j = 0; j < 6; j++) {
                Hu3DAnimKill(lbl_1_bss_179C[i].anim[j]);
            }
            Hu3DModelKill(lbl_1_bss_179C[i].model[0]);
            Hu3DModelKill(lbl_1_bss_179C[i].model[1]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_7990(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 6; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

s32 fn_1_981C(void)
{
    OMOBJ *obj;

    fn_1_1C64();
    fn_1_1D68();
    fn_1_12F8(1, 0x180000, 1);
    fn_1_11A0();
    fn_1_1C64();
    fn_1_12F8(3, 0x180001, 1);
    fn_1_11A0();
    obj = lbl_1_bss_4;
    obj->objFunc = fn_1_2750;
    return 1;
}

s32 fn_1_EEB8(s16 index)
{
    if (lbl_1_data_22C[index].value[0] == 0) {
        return 1;
    }
    if (lbl_1_data_22C[index].value[0] == 1) {
        if (lbl_1_data_22C[0].value[10] == 0) {
            fn_1_1C64();
            fn_1_12F8(3, 0x70047, 1);
            fn_1_11A0();
            return 0;
        }
        return 1;
    }
    if (lbl_1_data_22C[index].value[0] == 2) {
        if (GWMgUnlockGet(0x2A6) == 0) {
            fn_1_1C64();
            fn_1_12F8(3, 0x70041, 1);
            fn_1_11A0();
            return 0;
        }
        return 1;
    }
    if (lbl_1_data_22C[index].value[0] == 3) {
        if (GWMgUnlockGet(0x2A7) == 0) {
            fn_1_1C64();
            fn_1_12F8(3, 0x70072, 1);
            fn_1_11A0();
            return 0;
        }
        return 1;
    }
    return 1;
}

s32 fn_1_F7C0(s16 index)
{
    if (lbl_1_data_22C[index].value[1] == 1) {
        fn_1_1D68();
        fn_1_12F8(2, lbl_1_data_22C[index].choice.message, 1);
        if (fn_1_1200(2) == 0) {
            return 10;
        }
        return 0;
    }
    if (lbl_1_data_22C[index].value[1] == 2) {
        fn_1_1D68();
        fn_1_12F8(2, lbl_1_data_22C[index].choice.message, 1);
        if (fn_1_1200(2) == 0) {
            return 20;
        }
        return 0;
    }
    if (lbl_1_data_22C[index].value[1] == 3) {
        if (GWBankFlagGet(60) == 0) {
            fn_1_1D68();
            fn_1_12F8(2, 0x70042, 1);
            if (fn_1_1200(2) == 0) {
                GWBankFlagSet(60);
            }
        } else {
            fn_1_1D68();
            fn_1_12F8(2, 0x70043, 1);
            if (fn_1_1200(2) == 0) {
                GWBankFlagReset(60);
            }
        }
        return 0;
    }
    return 1;
}

void fn_1_1050C(s16 index)
{
    if (lbl_1_data_22C[index].value[1] == 1) {
        fn_1_1D68();
        fn_1_12F8(2, 0x70038, 1);
        fn_1_11A0();
    } else if (lbl_1_data_22C[index].value[1] == 2) {
        fn_1_1D68();
        fn_1_12F8(2, 0x7003D, 1);
        fn_1_11A0();
    } else if (lbl_1_data_22C[index].value[1] == 3) {
        fn_1_1D68();
        fn_1_12F8(2, 0x70044, 1);
        fn_1_11A0();
    } else {
        fn_1_1D68();
        fn_1_12F8(2, lbl_1_data_22C[index].choice.message, 1);
        fn_1_11A0();
    }
}

void fn_1_2EFC(s16 index)
{
    OMOBJ *obj = lbl_1_bss_14;

    obj->work[index] = 0;
    lbl_1_data_964[index] = 0;
    HuSprAttrSet(lbl_1_bss_194E[1], index, HUSPR_ATTR_DISPOFF);
}

inline void fn_1_2EFC(s16 index);

void fn_1_2F80(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        lbl_1_bss_1700[i] = fn_1_111B0(lbl_1_bss_1700[i],
            lbl_1_rodata_74, lbl_1_rodata_158);
        HuSprScaleSet(lbl_1_bss_194E[1], i, lbl_1_bss_1700[i],
            lbl_1_bss_1700[i]);
    }
}

void fn_1_304C(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        fn_1_2EFC(i);
        lbl_1_data_964[i] = 0;
    }
}

void fn_1_3100(HuVecF *pos)
{
    lbl_1_bss_1710.x = pos->x - lbl_1_rodata_15C;
    lbl_1_bss_1710.y = lbl_1_rodata_15C + pos->y;
    lbl_1_bss_1710.z = lbl_1_rodata_15C + pos->z;
}

inline void fn_1_3100(HuVecF *pos);

void fn_1_3164(HuVecF *pos)
{
    OMOBJ *obj = lbl_1_bss_14;

    Hu3DModelPosSetV(obj->mdlId[0], pos);
    Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_68, lbl_1_rodata_68,
        lbl_1_rodata_160);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_74, lbl_1_rodata_74,
        lbl_1_rodata_74);
    fn_1_3100(pos);
    Hu3DModelAttrReset(obj->mdlId[0], HU3D_ATTR_DISPOFF);
}

void fn_1_32C0(OMOBJ *obj)
{
    HuVecF transform;
    s16 i;

    Hu3DModelPosGet(obj->mdlId[0], &transform);
    fn_1_111F8(&transform, &lbl_1_bss_1710, lbl_1_rodata_164);
    Hu3DModelPosSetV(obj->mdlId[0], &transform);
    Hu3DModelRotGet(obj->mdlId[0], &transform);
    transform.z = fn_1_111B0(transform.z, lbl_1_rodata_68, lbl_1_rodata_164);
    Hu3DModelRotSetV(obj->mdlId[0], &transform);
    Hu3DModelScaleGet(obj->mdlId[0], &transform);
    transform.x = transform.y = transform.z =
        fn_1_111B0(transform.x, lbl_1_rodata_168, lbl_1_rodata_164);
    Hu3DModelScaleSetV(obj->mdlId[0], &transform);
    for (i = 0; i < 4; i++) {
        lbl_1_bss_1700[i] = fn_1_111B0(lbl_1_bss_1700[i], lbl_1_rodata_74,
            lbl_1_rodata_158);
        HuSprScaleSet(lbl_1_bss_194E[1], i, lbl_1_bss_1700[i],
            lbl_1_bss_1700[i]);
    }
    fn_1_3640();
}

void fn_1_3468(OMOBJ *obj)
{
    OMOBJ *displayObj;
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(0x940008);
    obj->mtnId[0] = Hu3DMotionIDGet(obj->mdlId[0]);
    Hu3DModelLayerSet(obj->mdlId[0], 5);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_rodata_68,
        lbl_1_rodata_68, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
    displayObj = lbl_1_bss_14;
    Hu3DModelAttrSet(displayObj->mdlId[0], HU3D_ATTR_DISPOFF);
    for (i = 0; i < 4; i++) {
        s16 index = i;
        OMOBJ *workObj = lbl_1_bss_14;

        workObj->work[index] = 0;
        lbl_1_data_964[index] = 0;
        HuSprAttrSet(lbl_1_bss_194E[1], index, HUSPR_ATTR_DISPOFF);
        lbl_1_data_964[i] = 0;
    }
    obj->objFunc = fn_1_32C0;
}

void fn_1_420C(HUSPR_GROUPID groupId, s32 memberNo, s16 value)
{
    s16 bank;

    bank = value / 100;
    HuSprBankSet(groupId, memberNo, bank);
    if (bank == 0) {
        HuSprAttrSet(groupId, memberNo, HUSPR_ATTR_DISPOFF);
    }
    bank = (value - (bank * 100)) / 10;
    HuSprBankSet(groupId, memberNo + 1, bank);
    if (bank == 0 && value / 100 == 0) {
        HuSprAttrSet(groupId, memberNo + 1, HUSPR_ATTR_DISPOFF);
    }
    bank = value % 10;
    HuSprBankSet(groupId, memberNo + 2, bank);
}

void fn_1_6600(void)
{
    OMOBJ *obj = lbl_1_bss_1C;
    s16 i;

    for (i = 1; i < 5; i++) {
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_6668(void)
{
    OMOBJ *obj = lbl_1_bss_1C;
}

void fn_1_6684(void)
{
    OMOBJ *obj = lbl_1_bss_20;
    s16 i;

    if (lbl_1_bss_1930[0] > 80) {
        Hu3DAnimAnimSet(lbl_1_bss_200, lbl_1_bss_1958[10]);
    } else if (lbl_1_bss_1930[0] > 60) {
        Hu3DAnimAnimSet(lbl_1_bss_200, lbl_1_bss_1958[9]);
    } else if (lbl_1_bss_1930[0] > 40) {
        Hu3DAnimAnimSet(lbl_1_bss_200, lbl_1_bss_1958[8]);
    } else if (lbl_1_bss_1930[0] > 20) {
        Hu3DAnimAnimSet(lbl_1_bss_200, lbl_1_bss_1958[7]);
    } else {
        Hu3DAnimAnimSet(lbl_1_bss_200, lbl_1_bss_1958[6]);
    }
    for (i = 0; i < 20; i++) {
        if (i <= lbl_1_bss_1930[0] - 1) {
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        }
    }
}

void fn_1_3DBC(s16 state, s16 time)
{
    OMOBJ *obj = lbl_1_bss_20;

    lbl_1_data_972 = state;
    lbl_1_data_970 = time;
    if (time >= 60) {
        lbl_1_data_974 = 5;
    } else {
        lbl_1_data_974 = 5;
    }
    obj->objFunc = fn_1_3788;
}

int _prolog(void)
{
    const MDBANK_VOID_FUNC *ctor = _ctors;

    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_91A4();
    return 0;
}

void _epilog(void)
{
    const MDBANK_VOID_FUNC *dtor = _dtors;

    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

void fn_1_91A4(void)
{
    OSReport(lbl_1_data_9E8);
    lbl_1_bss_2C = omovlevtno;
    lbl_1_bss_30 = omovlstat;
    fn_1_86C0();
}

void fn_1_2378(void)
{
    OMOBJ *obj = lbl_1_bss_10;
    MDBANK_MOVE_WORK *work = &lbl_1_bss_171C;

    fn_1_1115C(&work->start, lbl_1_rodata_11C,
        lbl_1_rodata_100, lbl_1_rodata_68);
    fn_1_1115C(&work->control, lbl_1_rodata_120,
        lbl_1_rodata_100, lbl_1_rodata_68);
    fn_1_1115C(&work->end, lbl_1_rodata_124,
        lbl_1_rodata_10C, lbl_1_rodata_68);
    work->time = lbl_1_rodata_68;
    work->duration = lbl_1_rodata_110;
    obj->objFunc = fn_1_226C;
}

void fn_1_1115C(HuVecF *out, float x, float y, float z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}

void fn_1_7A20(s16 state)
{
    OMOBJ *obj = lbl_1_bss_24[0];

    obj->work[0] = state;
    if (state == 0) {
        fn_1_13764(1);
    } else {
        fn_1_13764(0);
    }
}

void fn_1_3FC(MDBANK_CAMERA_WORK *camera, float weight)
{
    fn_1_111F8(&camera->center, &camera->centerTarget, weight);
    fn_1_111F8(&camera->rot, &camera->rotTarget, weight);
    camera->zoom = fn_1_111B0(camera->zoom, camera->zoomTarget, weight);
}

void fn_1_2888(void)
{
    OMOBJ *obj = lbl_1_bss_4;

    obj->objFunc = fn_1_2750;
}

void fn_1_12120(void)
{
    lbl_1_bss_19EC = Hu3DParticleCreate(lbl_1_bss_19F0[0], 32);
    Hu3DModelPosSet(lbl_1_bss_19EC, lbl_1_rodata_240,
        lbl_1_rodata_298, lbl_1_rodata_29C);
    Hu3DModelScaleSet(lbl_1_bss_19EC, lbl_1_rodata_268,
        lbl_1_rodata_268, lbl_1_rodata_268);
    Hu3DModelLayerSet(lbl_1_bss_19EC, 6);
    Hu3DParticleHookSet(lbl_1_bss_19EC, fn_1_119C4);
    Hu3DParticleBlendModeSet(lbl_1_bss_19EC, HU3D_PARTICLE_BLEND_ADDCOL);
}

void fn_1_12204(void)
{
    Hu3DModelKill(lbl_1_bss_19EC);
}

void fn_1_12A68(void)
{
    Hu3DModelKill(lbl_1_bss_19EA);
}

void fn_1_12A94(float x, float y, float z)
{
    Hu3DModelPosSet(lbl_1_bss_19E8, x, y, lbl_1_rodata_2A8 + z);
}

void fn_1_12AE8(s16 time)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_19E8];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = time;
    }
}

void fn_1_12D98(void)
{
    lbl_1_bss_19E8 = Hu3DParticleCreate(lbl_1_bss_19F0[1], 4);
    Hu3DModelPosSet(lbl_1_bss_19E8, lbl_1_rodata_240,
        lbl_1_rodata_298, lbl_1_rodata_2A8);
    Hu3DModelScaleSet(lbl_1_bss_19E8, lbl_1_rodata_268,
        lbl_1_rodata_268, lbl_1_rodata_268);
    Hu3DModelLayerSet(lbl_1_bss_19E8, 6);
    Hu3DParticleHookSet(lbl_1_bss_19E8, fn_1_12B60);
    Hu3DParticleBlendModeSet(lbl_1_bss_19E8, HU3D_PARTICLE_BLEND_ADDCOL);
}

void fn_1_12E7C(void)
{
    Hu3DModelKill(lbl_1_bss_19E8);
}

void fn_1_12EA8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_19F0[i] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_A20[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }

    lbl_1_bss_19EC = Hu3DParticleCreate(lbl_1_bss_19F0[0], 32);
    Hu3DModelPosSet(lbl_1_bss_19EC, lbl_1_rodata_240,
        lbl_1_rodata_298, lbl_1_rodata_29C);
    Hu3DModelScaleSet(lbl_1_bss_19EC, lbl_1_rodata_268,
        lbl_1_rodata_268, lbl_1_rodata_268);
    Hu3DModelLayerSet(lbl_1_bss_19EC, 6);
    Hu3DParticleHookSet(lbl_1_bss_19EC, fn_1_119C4);
    Hu3DParticleBlendModeSet(lbl_1_bss_19EC, HU3D_PARTICLE_BLEND_ADDCOL);

    lbl_1_bss_19EA = Hu3DParticleCreate(lbl_1_bss_19F0[1], 1000);
    Hu3DModelPosSet(lbl_1_bss_19EA, lbl_1_rodata_240,
        lbl_1_rodata_240, lbl_1_rodata_240);
    Hu3DModelScaleSet(lbl_1_bss_19EA, lbl_1_rodata_268,
        lbl_1_rodata_268, lbl_1_rodata_268);
    Hu3DModelLayerSet(lbl_1_bss_19EA, 6);
    Hu3DParticleHookSet(lbl_1_bss_19EA, fn_1_124F4);
    Hu3DParticleBlendModeSet(lbl_1_bss_19EA, HU3D_PARTICLE_BLEND_ADDCOL);

    lbl_1_bss_19E8 = Hu3DParticleCreate(lbl_1_bss_19F0[1], 4);
    Hu3DModelPosSet(lbl_1_bss_19E8, lbl_1_rodata_240,
        lbl_1_rodata_298, lbl_1_rodata_2A8);
    Hu3DModelScaleSet(lbl_1_bss_19E8, lbl_1_rodata_268,
        lbl_1_rodata_268, lbl_1_rodata_268);
    Hu3DModelLayerSet(lbl_1_bss_19E8, 6);
    Hu3DParticleHookSet(lbl_1_bss_19E8, fn_1_12B60);
    Hu3DParticleBlendModeSet(lbl_1_bss_19E8, HU3D_PARTICLE_BLEND_ADDCOL);
}

void fn_1_1317C(void)
{
    Hu3DModelKill(lbl_1_bss_19EC);
    Hu3DModelKill(lbl_1_bss_19EA);
    Hu3DModelKill(lbl_1_bss_19E8);
}

void fn_1_13764(s16 time)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_19E8];
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle = model->hookData;
    s16 i;

    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = time;
    }
}

void fn_1_9380(OMOBJ *obj, MDBANK_CAMERA_WORK *camera)
{
    camera->centerTarget.x = lbl_1_rodata_68;
    camera->centerTarget.y = lbl_1_rodata_78;
    camera->centerTarget.z = lbl_1_rodata_7C;
    camera->rotTarget.x = lbl_1_rodata_80;
    camera->rotTarget.y = lbl_1_rodata_68;
    camera->rotTarget.z = lbl_1_rodata_68;
    camera->zoomTarget = lbl_1_rodata_88;
    fn_1_111F8(&camera->center, &camera->centerTarget, lbl_1_rodata_F4);
    fn_1_111F8(&camera->rot, &camera->rotTarget, lbl_1_rodata_F4);
    camera->zoom = fn_1_111B0(camera->zoom, camera->zoomTarget,
        lbl_1_rodata_F4);
}

float fn_1_1116C(float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_240) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}

float fn_1_111B0(float current, float target, float weight)
{
    if (current == target || weight <= lbl_1_rodata_268) {
        return target;
    }
    return (target + (current * (weight - lbl_1_rodata_268))) / weight;
}

void fn_1_111F8(HuVecF *current, const HuVecF *target, float weight)
{
    current->x = fn_1_111B0(current->x, target->x, weight);
    current->y = fn_1_111B0(current->y, target->y, weight);
    current->z = fn_1_111B0(current->z, target->z, weight);
}

float fn_1_1161C(float start, float control, float end, float weight)
{
    float inverse = lbl_1_rodata_268 - weight;

    return (weight * weight * end)
        + ((inverse * inverse * start)
            + (lbl_1_rodata_274 * (control * (inverse * weight))));
}

void fn_1_11678(HuVecF *out, const HuVecF *start, const HuVecF *control, const HuVecF *end,
    float weight)
{
    out->x = fn_1_1161C(start->x, control->x, end->x, weight);
    out->y = fn_1_1161C(start->y, control->y, end->y, weight);
    out->z = fn_1_1161C(start->z, control->z, end->z, weight);
}

void fn_1_4360(MDBANK_ANIM_SET *set, MDBANK_ITEM *item)
{
    Mtx matrix;
    HuVecF modelPos;
    HuVecF screenPos;
    s16 digitSet = 0;
    MDBANK_NAME_TABLE names = lbl_1_rodata_190;
    s16 i;

    if (item->value[3] >= 100) {
        digitSet = 0;
    } else if (item->value[3] >= 10) {
        digitSet = 1;
    } else {
        digitSet = 2;
    }
    Hu3DModelPosGet(set->model[0], &modelPos);
    Hu3DModelPosSetV(set->model[1], &modelPos);
    Hu3DModelScaleGet(set->model[0], &modelPos);
    Hu3DModelScaleSet(set->model[1], lbl_1_rodata_74,
        lbl_1_rodata_74, modelPos.z);
    if (set->state[3] != 1) {
        Hu3DModelRotGet(set->model[0], &modelPos);
        while (TRUE) {
            if (modelPos.y > lbl_1_rodata_140) {
                modelPos.y -= lbl_1_rodata_140;
                continue;
            }
            break;
        }
        while (TRUE) {
            if (modelPos.y < lbl_1_rodata_68) {
                modelPos.y += lbl_1_rodata_140;
                continue;
            }
            break;
        }
        Hu3DModelRotSetV(set->model[1], &modelPos);
        for (i = 0; i < 3; i++) {
            HuSpr3DRotSet(set->state[i], lbl_1_rodata_68,
                modelPos.y, lbl_1_rodata_68);
        }
        for (i = 0; i < 3; i++) {
            Hu3DModelObjMtxGet(set->model[1], names.name[digitSet][i], matrix);
            modelPos.x = matrix[0][3];
            modelPos.y = matrix[1][3];
            modelPos.z = matrix[2][3];
            Hu3D3Dto2D(&modelPos, 1, &screenPos);
            HuSprPosSet(set->spriteGroup, i, screenPos.x, screenPos.y);
            HuSprAttrReset(set->spriteGroup, i, HUSPR_ATTR_DISPOFF);
        }
        {
            s16 value = item->value[3];
            HUSPR_GROUPID group = set->spriteGroup;
            s16 bank;

            bank = value / 100;
            HuSprBankSet(group, 0, bank);
            if (bank == 0) {
                HuSprAttrSet(group, 0, HUSPR_ATTR_DISPOFF);
            }
            bank = (value - (bank * 100)) / 10;
            HuSprBankSet(group, 1, bank);
            if (bank == 0 && value / 100 == 0) {
                HuSprAttrSet(group, 1, HUSPR_ATTR_DISPOFF);
            }
            bank = value % 10;
            HuSprBankSet(group, 2, bank);
        }
        Hu3DModelRotGet(set->model[1], &modelPos);
        for (i = 0; i < 3; i++) {
            if (modelPos.y >= lbl_1_rodata_178
                && modelPos.y <= lbl_1_rodata_1B4) {
                HuSprAttrSet(set->spriteGroup, i, HUSPR_ATTR_DISPOFF);
            }
        }
    }
}

void fn_1_5C1C(s16 mode)
{
    OMOBJ *obj = lbl_1_bss_18;
    MDBANK_ANIM_SET *set = lbl_1_bss_179C;
    s16 i;

    for (i = 0; i < 10; i++, set++) {
        Hu3DAnimAnimSet(set->anim[0], lbl_1_bss_18A0[31]);
        Hu3DAnimAnimSet(set->anim[1], lbl_1_bss_18A0[31]);
        Hu3DAnimAnimSet(set->anim[2], lbl_1_bss_18A0[31]);
        Hu3DAnimAnimSet(set->anim[3], lbl_1_bss_18A0[31]);
        Hu3DAnimAnimSet(set->anim[4], lbl_1_bss_18A0[31]);
        Hu3DAnimAnimSet(set->anim[5], lbl_1_bss_18A0[31]);
        Hu3DModelScaleSet(set->model[0], lbl_1_rodata_68,
            lbl_1_rodata_68, lbl_1_rodata_68);
    }
    if (mode == 0) {
        obj->work[0] = 0;
        obj->work[1] = 60;
    } else {
        obj->work[0] = 0;
        obj->work[1] = 1;
    }
    obj->objFunc = fn_1_5988;
}

s32 fn_1_9464(void)
{
    OMOBJ *obj;
    s16 i;

    HuPrcSleep(5);
    obj = lbl_1_bss_20;
    if (lbl_1_bss_1930[0] > 80) {
        Hu3DAnimAnimSet(lbl_1_bss_200, lbl_1_bss_1958[10]);
    } else if (lbl_1_bss_1930[0] > 60) {
        Hu3DAnimAnimSet(lbl_1_bss_200, lbl_1_bss_1958[9]);
    } else if (lbl_1_bss_1930[0] > 40) {
        Hu3DAnimAnimSet(lbl_1_bss_200, lbl_1_bss_1958[8]);
    } else if (lbl_1_bss_1930[0] > 20) {
        Hu3DAnimAnimSet(lbl_1_bss_200, lbl_1_bss_1958[7]);
    } else {
        Hu3DAnimAnimSet(lbl_1_bss_200, lbl_1_bss_1958[6]);
    }
    for (i = 0; i < 20; i++) {
        if (i <= lbl_1_bss_1930[0] - 1) {
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        }
    }
    fn_1_6C6C();
    if (lbl_1_bss_2C == 1) {
        OMOBJ *shapeObj = lbl_1_bss_1C;

        shapeObj->work[2] = 1;
        fn_1_5C1C(1);
    }
    lbl_1_bss_1988 = HuAudSStreamPlay(0x5C);
    HuAudFXPlay(0x585);
    fn_1_92A4();
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    return 1;
}

s16 fn_1_A438(void)
{
    fn_1_1D68();
    fn_1_12F8(2, 0x180009, 1);
    return fn_1_1200(2);
}

void fn_1_12984(void)
{
    lbl_1_bss_19EA = Hu3DParticleCreate(lbl_1_bss_19F0[1], 1000);
    Hu3DModelPosSet(lbl_1_bss_19EA, lbl_1_rodata_240,
        lbl_1_rodata_240, lbl_1_rodata_240);
    Hu3DModelScaleSet(lbl_1_bss_19EA, lbl_1_rodata_268,
        lbl_1_rodata_268, lbl_1_rodata_268);
    Hu3DModelLayerSet(lbl_1_bss_19EA, 6);
    Hu3DParticleHookSet(lbl_1_bss_19EA, fn_1_124F4);
    Hu3DParticleBlendModeSet(lbl_1_bss_19EA,
        HU3D_PARTICLE_BLEND_ADDCOL);
}

void fn_1_588(MDBANK_CAMERA_CALLBACK callback)
{
    MDBANK_CAMERA_WORK *camera = &lbl_1_bss_1998;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_58, lbl_1_rodata_5C,
        lbl_1_rodata_60, lbl_1_rodata_64);
    Hu3DCameraViewportSet(1, lbl_1_rodata_68, lbl_1_rodata_68,
        lbl_1_rodata_6C, lbl_1_rodata_70, lbl_1_rodata_68,
        lbl_1_rodata_74);
    memset(camera, 0, sizeof(*camera));
    camera->callback = callback;
    camera->center.x = lbl_1_rodata_68;
    camera->center.y = lbl_1_rodata_78;
    camera->center.z = lbl_1_rodata_7C;
    camera->rot.x = lbl_1_rodata_80;
    camera->rot.y = lbl_1_rodata_68;
    camera->rot.z = lbl_1_rodata_68;
    if (lbl_1_bss_2C == 0) {
        camera->zoom = lbl_1_rodata_84;
    } else {
        camera->zoom = lbl_1_rodata_88;
    }
    camera->obj = omAddObjEx(lbl_1_bss_0, 0x100, 0, 0, -1, fn_1_4B8);
}

void fn_1_2468(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(0x940044);
    obj->mdlId[1] = Hu3DModelCreateData(0x940045);
    for (i = 0; i < 3; i++) {
        obj->mtnId[i] = Hu3DJointMotionData(obj->mdlId[0], 0x940046 + i);
    }
    Hu3DModelHookSet(obj->mdlId[0], lbl_1_data_944, obj->mdlId[1]);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_rodata_68,
        lbl_1_rodata_68, HU3D_MOTATTR_LOOP);
    if (lbl_1_bss_2C == 0) {
        Hu3DModelPosSet(obj->mdlId[0], lbl_1_rodata_11C,
            lbl_1_rodata_100, lbl_1_rodata_68);
        Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_68, lbl_1_rodata_68,
            lbl_1_rodata_68);
    } else {
        Hu3DModelPosSet(obj->mdlId[0], lbl_1_rodata_124,
            lbl_1_rodata_10C, lbl_1_rodata_68);
        Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_68,
            lbl_1_rodata_118, lbl_1_rodata_68);
    }
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_114, lbl_1_rodata_114,
        lbl_1_rodata_114);
    Hu3DModelShadowSet(obj->mdlId[0]);
    obj->objFunc = NULL;
}

void fn_1_58B8(OMOBJ *obj)
{
    MDBANK_ANIM_SET *set;
    HuVecF rot;

    if (obj->work[1] == 1 && ++obj->work[0] > 60) {
        obj->work[0] = 70;
        set = &lbl_1_bss_179C[obj->work[2]];
        Hu3DModelRotGet(set->model[0], &rot);
        rot.y += lbl_1_rodata_164;
        if (rot.y > lbl_1_rodata_140) {
            rot.y -= lbl_1_rodata_140;
        }
        Hu3DModelRotSetV(set->model[0], &rot);
    }
    fn_1_5550();
}

__declspec(section ".rodata") const float lbl_1_rodata_68 = 0.0f;
__declspec(section ".rodata") const float lbl_1_rodata_6C = 640.0f;
__declspec(section ".rodata") const float lbl_1_rodata_74 = 1.0f;
__declspec(section ".rodata") const float lbl_1_rodata_F4 = 15.0f;
__declspec(section ".rodata") const float lbl_1_rodata_F8 = 20.0f;
__declspec(section ".rodata") const float lbl_1_rodata_FC = -100.0f;
__declspec(section ".rodata") const float lbl_1_rodata_100 = 25.0f;
__declspec(section ".rodata") const float lbl_1_rodata_108 = -350.0f;
__declspec(section ".rodata") const float lbl_1_rodata_10C = 150.0f;
__declspec(section ".rodata") const float lbl_1_rodata_114 = 1.25f;
__declspec(section ".rodata") const float lbl_1_rodata_130 = 288.0f;
__declspec(section ".rodata") const float lbl_1_rodata_134 = 120.0f;
__declspec(section ".rodata") const float lbl_1_rodata_138 = 0.6f;
__declspec(section ".rodata") const float lbl_1_rodata_13C = 0.1f;
__declspec(section ".rodata") const float lbl_1_rodata_140 = 360.0f;
__declspec(section ".rodata") const float lbl_1_rodata_144 = -850.0f;
__declspec(section ".rodata") const float lbl_1_rodata_154 = 2.0f;
__declspec(section ".rodata") const float lbl_1_rodata_158 = 5.0f;
__declspec(section ".rodata") const float lbl_1_rodata_15C = 50.0f;
__declspec(section ".rodata") const float lbl_1_rodata_160 = 180.0f;
__declspec(section ".rodata") const float lbl_1_rodata_168 = 0.3f;
__declspec(section ".rodata") const float lbl_1_rodata_1E4 = -2.0f;
__declspec(section ".rodata") const float lbl_1_rodata_240 = 0.0f;
__declspec(section ".rodata") const double lbl_1_rodata_248 = 1.0;
__declspec(section ".rodata") const double lbl_1_rodata_250 = 3.141592653589793;
__declspec(section ".rodata") const float lbl_1_rodata_258 = 90.0f;
__declspec(section ".rodata") const double lbl_1_rodata_260 = 180.0;
__declspec(section ".rodata") const float lbl_1_rodata_268 = 1.0f;
__declspec(section ".rodata") const float lbl_1_rodata_26C = 360.0f;
__declspec(section ".rodata") const float lbl_1_rodata_270 = 180.0f;
__declspec(section ".rodata") const float lbl_1_rodata_274 = 2.0f;
__declspec(section ".rodata") const float lbl_1_rodata_298 = 375.0f;
__declspec(section ".rodata") const float lbl_1_rodata_29C = -100.0f;
__declspec(section ".rodata") const float lbl_1_rodata_2A8 = 100.0f;
