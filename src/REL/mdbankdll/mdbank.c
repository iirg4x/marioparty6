#include <stdio.h>

#include "dolphin.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/gamework.h"
#include "game/main.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/saveload.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"
#include "messdir_enum.h"

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

enum MdbankUnlockMinigame {
    MDBANK_UNLOCK_MINIGAME_M678 = 678,
    MDBANK_UNLOCK_MINIGAME_M679 = 679,
};

enum MdbankItemMessage {
    MDBANK_ITEM_MESSAGE_INITIAL_UNLOCK_REQUIRED =
        MESSNUM(MESS_SBANK_ITEM, 71),
    MDBANK_ITEM_MESSAGE_M678_UNLOCK_REQUIRED =
        MESSNUM(MESS_SBANK_ITEM, 65),
    MDBANK_ITEM_MESSAGE_M679_UNLOCK_REQUIRED =
        MESSNUM(MESS_SBANK_ITEM, 114),
    MDBANK_ITEM_MESSAGE_BANK_FLAG_ENABLE_CONFIRM =
        MESSNUM(MESS_SBANK_ITEM, 66),
    MDBANK_ITEM_MESSAGE_BANK_FLAG_DISABLE_CONFIRM =
        MESSNUM(MESS_SBANK_ITEM, 67),
    MDBANK_ITEM_MESSAGE_STATE_ONE_NOTICE = MESSNUM(MESS_SBANK_ITEM, 56),
    MDBANK_ITEM_MESSAGE_STATE_TWO_NOTICE = MESSNUM(MESS_SBANK_ITEM, 61),
    MDBANK_ITEM_MESSAGE_STATE_THREE_NOTICE = MESSNUM(MESS_SBANK_ITEM, 68),
};

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

typedef struct MdbankModelData {
    u32 data[5];
} MDBANK_MODEL_DATA;

typedef struct MdbankEventRecord {
    s16 active;
    s16 unresolved_02;
    float time;
    float duration;
    HuVecF start;
    HuVecF control;
    HuVecF end;
    float rotation;
    float unresolved_34;
    float unresolved_38;
    float unresolved_3C;
} MDBANK_EVENT_RECORD;

typedef struct MdbankPostCallbackWork {
    u8 opaque_00[4];
    float time;
    float duration;
    u8 opaque_0C[0x34];
} MDBANK_POST_CALLBACK_WORK;

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
extern char lbl_1_data_9E8[52];
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
extern const double lbl_1_rodata_128;
extern const float lbl_1_rodata_1B8;
extern const float lbl_1_rodata_1BC;
extern const float lbl_1_rodata_1C0;
extern const float lbl_1_rodata_1C4;
extern const float lbl_1_rodata_130;
extern const float lbl_1_rodata_134;
extern const float lbl_1_rodata_138;
extern const float lbl_1_rodata_13C;
extern const float lbl_1_rodata_140;
extern const float lbl_1_rodata_144;
extern const float lbl_1_rodata_154;
extern const float lbl_1_rodata_158;
extern const float lbl_1_rodata_15C;
extern const float lbl_1_rodata_160;
extern const float lbl_1_rodata_164;
extern const float lbl_1_rodata_168;
extern const float lbl_1_rodata_16C;
extern const double lbl_1_rodata_170;
extern const float lbl_1_rodata_178;
extern const float lbl_1_rodata_17C;
extern const float lbl_1_rodata_180;
extern const float lbl_1_rodata_184;
extern const float lbl_1_rodata_188;
extern const MDBANK_NAME_TABLE lbl_1_rodata_190;
extern const float lbl_1_rodata_1B4;
extern const float lbl_1_rodata_1E0;
extern const HuVecF lbl_1_rodata_1E8;
extern const float lbl_1_rodata_1F4;
extern const float lbl_1_rodata_1F8;
extern const float lbl_1_rodata_1FC;
extern const float lbl_1_rodata_1E4;
extern const HuVecF lbl_1_rodata_148;
extern const HuVecF lbl_1_rodata_230;
extern const float lbl_1_rodata_23C;
extern const u8 lbl_1_rodata_2B8;
extern const float lbl_1_rodata_2BC;
extern const char lbl_1_data_A1C[];
extern const HuVecF lbl_1_rodata_200;
extern const MDBANK_MODEL_DATA lbl_1_rodata_20C;
extern const float lbl_1_rodata_220;
extern const float lbl_1_rodata_224;
extern const float lbl_1_rodata_228;
extern const float lbl_1_rodata_240;
extern const double lbl_1_rodata_248;
extern const double lbl_1_rodata_250;
extern const float lbl_1_rodata_258;
extern const double lbl_1_rodata_260;
extern const float lbl_1_rodata_268;
extern const float lbl_1_rodata_26C;
extern const float lbl_1_rodata_270;
extern const float lbl_1_rodata_274;
extern const float lbl_1_rodata_278;
extern const float lbl_1_rodata_27C;
extern const double lbl_1_rodata_280;
extern const double lbl_1_rodata_288;
extern const double lbl_1_rodata_290;
extern const float lbl_1_rodata_298;
extern const float lbl_1_rodata_29C;
extern const double lbl_1_rodata_2A0;
extern const float lbl_1_rodata_2A8;
extern const GXColor lbl_1_rodata_2AC;
extern const GXColor lbl_1_rodata_2B0;
extern const float lbl_1_rodata_2B4;
extern const HuVecF lbl_1_rodata_8C[2];
extern const HuVecF lbl_1_rodata_A4[2];
extern const u8 lbl_1_rodata_BC;
extern const HuVecF lbl_1_rodata_C0;
extern const HuVecF lbl_1_rodata_CC;
extern const HuVecF lbl_1_rodata_D8;
extern const float lbl_1_rodata_18C;
extern const char *const lbl_1_rodata_1C8[6];
extern const float lbl_1_rodata_22C;
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
extern s16 lbl_1_bss_34[6];
extern MDBANK_EVENT_RECORD lbl_1_bss_300[80];
extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
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
extern MDBANK_POST_CALLBACK_WORK lbl_1_bss_40[6];
extern HuVecF lbl_1_data_9D0;
extern char lbl_1_data_9DC[12];
extern const s32 lbl_1_data_19C[36];
extern const char lbl_1_data_99C[];
extern const char lbl_1_data_9A4[];
extern const char lbl_1_data_9AC[];
extern const char lbl_1_data_9B5[];
extern const char lbl_1_data_9BD[];
extern const char lbl_1_data_9C5[];
extern const MDBANK_VOID_FUNC _ctors[];
extern const MDBANK_VOID_FUNC _dtors[];

void fn_1_11880(HUSPR_GROUPID groupId, s32 attr);
void fn_1_0(HUWINID winId, u32 mess, s16 index);
void fn_1_12EA8(void);
void fn_1_9380(OMOBJ *obj, MDBANK_CAMERA_WORK *camera);
void fn_1_2468(OMOBJ *obj);
void fn_1_11900(HUSPR_GROUPID groupId, s32 attr);
void fn_1_4360(MDBANK_ANIM_SET *set, MDBANK_ITEM *item);
void fn_1_119C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_12B60(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_124F4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_780(void);
void fn_1_8F0(void);
void fn_1_47AC(s16 index);
float fn_1_11540(float start, float end, float time, float duration);
void fn_1_12230(HuVecF *pos, GXColor *color, float velocityY, float accelY);
void fn_1_12354(HuVecF *pos, GXColor *color, float velocityY, float velocityZ);
short fn_1_EA98(s32 arg0, s32 arg1);
float fn_1_1116C(float start, float end, float time, float duration);
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
void fn_1_A750(s16 index);
void fn_1_48CC(s16 index);
void fn_1_4C9C(void);
void fn_1_4DFC(s16 index);
void fn_1_520C(s16 skipIndex);
void fn_1_53AC(s16 skipIndex);
void fn_1_5988(OMOBJ *obj);
float fn_1_11064(float start, float end, float time, float duration);
void fn_1_11980(float x, float y, float z);
void fn_1_12A94(float x, float y, float z);
void fn_1_3640(void);
void fn_1_6810(OMOBJ *obj);
void fn_1_131C8(HuVecF *pos);
float fn_1_11458(float start, float end, float time, float duration);
void fn_1_11678(HuVecF *out, const HuVecF *start, const HuVecF *control, const HuVecF *end,
    float weight);
void fn_1_6290(HUSPR_GROUPID groupId, s16 baseIndex, s16 value);
void fn_1_70E4(OMOBJ *obj);
void fn_1_7594(OMOBJ *obj);
float fn_1_11384(float start, float end, float time, float duration);
void fn_1_9DE8(void);
void fn_1_3E30(OMOBJ *obj);
void fn_1_5D64(OMOBJ *obj);
void fn_1_7A84(OMOBJ *obj);
void fn_1_7F2C(OMOBJ *obj);
short fn_1_AA08(void);
void fn_1_13598(HuVecF *position);

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
                    HuAudFXPlayPan(effects.fx[index], 80);
                } else {
                    HuAudFXPlayPan(effects.fx[index], 48);
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
        GWMgUnlockSet(677);
    }
}

void fn_1_8B4(void)
{
    Hu3DGLightKill(lbl_1_bss_1994[0]);
    Hu3DGLightKill(lbl_1_bss_1994[1]);
}

void fn_1_8F0(void)
{
    HuVecF camPos = lbl_1_rodata_C0;
    HuVecF camUp = lbl_1_rodata_CC;
    HuVecF camTarget = lbl_1_rodata_D8;

    Hu3DShadowCreate(lbl_1_rodata_58, lbl_1_rodata_5C,
        lbl_1_rodata_60);
    Hu3DShadowPosSet(&camPos, &camUp, &camTarget);
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
        lbl_1_rodata_E8, 544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_198C[0]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[0], lbl_1_rodata_68);
    lbl_1_bss_198C[1] = HuWinExCreateFrame(lbl_1_rodata_E4,
        lbl_1_rodata_EC, 544, 68, -1, 5);
    HuWinDispOff(lbl_1_bss_198C[1]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[1], lbl_1_rodata_F0);
    lbl_1_bss_198C[2] = HuWinExCreateFrame(lbl_1_rodata_E4,
        lbl_1_rodata_EC, 544, 68, -1, 3);
    HuWinDispOff(lbl_1_bss_198C[2]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[2], lbl_1_rodata_F0);
    lbl_1_bss_198C[3] = HuWinExCreateFrame(lbl_1_rodata_E4,
        lbl_1_rodata_EC, 544, 68, -1, 4);
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
    obj->mdlId[0] = Hu3DModelCreateData(9699392);
    for (i = 0; i < 3; i++) {
        obj->mtnId[i] = Hu3DJointMotionData(obj->mdlId[0], 9699393 + i);
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
    HuSprPriSet(lbl_1_bss_194E[3], 0, 5500);
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
    HuSprExecLayerCameraSet(64, 1, 2);
    HuSprExecLayerCameraSet(65, 1, 4);
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
        GWMgUnlockSet(677);
    }
    if (result == 0) {
        s16 j;

        for (j = 0; j < 4; j++) {
            HuWinDispOff(lbl_1_bss_198C[j]);
        }
        HuSprPriSet(lbl_1_bss_194E[3], 0, 5500);
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
    obj->mdlId[0] = Hu3DModelCreateData(9699328);
    obj->mtnId[0] = Hu3DMotionIDGet(obj->mdlId[0]);
    obj->mdlId[1] = Hu3DModelCreateData(9699329);
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

void fn_1_6F00(void)
{
    HuVecF pos = lbl_1_rodata_200;
    MDBANK_MOVE_WORK *work = &lbl_1_bss_1C0;
    s16 i;

    if (work->active != 0) {
        fn_1_11678(&pos, &work->start, &work->control, &work->end,
            fn_1_11458(lbl_1_rodata_68, lbl_1_rodata_74,
                work->time, work->duration));
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
        if ((work->time += lbl_1_rodata_74) > work->duration) {
            work->active = 0;
        }
    }
}

void fn_1_6290(HUSPR_GROUPID groupId, s16 baseIndex, s16 value)
{
    s16 i;
    s16 thousands;
    s16 hundreds;
    s16 tens;
    s16 ones;

    if (value >= 1000) {
        for (i = 0; i < 4; i++) {
            HuSprPosSet(groupId, baseIndex + i,
                (float)(i * 22 - 33), lbl_1_rodata_68);
        }
    } else if (value >= 100) {
        for (i = 0; i < 3; i++) {
            HuSprPosSet(groupId, baseIndex + 1 + i,
                (float)(i * 22 - 22), lbl_1_rodata_68);
        }
    } else if (value >= 10) {
        for (i = 0; i < 2; i++) {
            HuSprPosSet(groupId, baseIndex + 2 + i,
                (float)(i * 22 - 11), lbl_1_rodata_68);
        }
    } else {
        for (i = 0; i < 1; i++) {
            HuSprPosSet(groupId, baseIndex + 3 + i,
                (float)(i << 5), lbl_1_rodata_68);
        }
    }

    fn_1_11900(groupId, HUSPR_ATTR_DISPOFF);

    thousands = value / 1000;
    HuSprBankSet(groupId, baseIndex, thousands);
    if (thousands == 0) {
        HuSprAttrSet(groupId, baseIndex, HUSPR_ATTR_DISPOFF);
    }

    value -= thousands * 1000;
    hundreds = value / 100;
    HuSprBankSet(groupId, baseIndex + 1, hundreds);
    if (hundreds == 0 && thousands == 0) {
        HuSprAttrSet(groupId, baseIndex + 1, HUSPR_ATTR_DISPOFF);
    }

    value -= hundreds * 100;
    tens = value / 10;
    HuSprBankSet(groupId, baseIndex + 2, tens);
    if (tens == 0 && hundreds == 0 && thousands == 0) {
        HuSprAttrSet(groupId, baseIndex + 2, HUSPR_ATTR_DISPOFF);
    }

    ones = value - tens * 10;
    HuSprBankSet(groupId, baseIndex + 3, ones);
}

void fn_1_70E4(OMOBJ *obj)
{
    HuVecF pos = lbl_1_rodata_200;
    MDBANK_MOVE_WORK *work = &lbl_1_bss_1C0;
    HuVecF modelPos;
    HuVecF screenPos;
    float verticalOffset;
    s16 i;

    if (work->active != 0) {
        fn_1_11678(&pos, &work->start, &work->control, &work->end,
            fn_1_11458(lbl_1_rodata_68, lbl_1_rodata_74,
                work->time, work->duration));
        Hu3DModelPosSet(lbl_1_bss_1C->mdlId[0], pos.x, pos.y, pos.z);
        for (i = 0; i < 12; i++) {
            Hu3DModelPosSet(lbl_1_bss_24[0]->mdlId[i],
                pos.x, pos.y, pos.z);
        }
        fn_1_11980(pos.x, pos.y, pos.z);
        fn_1_12A94(pos.x, pos.y, pos.z);
        for (i = 0; i < 20; i++) {
            Hu3DModelPosSet(lbl_1_bss_20->mdlId[i],
                pos.x + lbl_1_bss_210[i].x,
                pos.y + lbl_1_bss_210[i].y,
                pos.z + lbl_1_bss_210[i].z);
        }
        work->time += lbl_1_rodata_74;
        if (work->time > work->duration) {
            work->active = 0;
        }
    }

    if (obj->work[2] == 1) {
        lbl_1_bss_204.y = fn_1_111B0(lbl_1_bss_204.y,
            lbl_1_rodata_1E0, lbl_1_rodata_5C);

        Hu3DModelPosGet(obj->mdlId[1], &modelPos);
        verticalOffset = fn_1_11384(lbl_1_rodata_68,
            lbl_1_rodata_5C, (float)obj->work[1], lbl_1_rodata_160);
        modelPos.y = lbl_1_bss_204.y + verticalOffset;
        Hu3DModelPosSetV(obj->mdlId[1], &modelPos);

        Hu3D3Dto2D(&modelPos, 1, &screenPos);
        HuSprGrpPosSet(lbl_1_bss_194E[2], screenPos.x, screenPos.y);
        HuSprGrpScaleSet(lbl_1_bss_194E[2],
            lbl_1_rodata_188, lbl_1_rodata_74);

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
                Hu3DModelAttrReset(lbl_1_bss_20->mdlId[i],
                    HU3D_ATTR_DISPOFF);
            } else {
                Hu3DModelAttrSet(lbl_1_bss_20->mdlId[i],
                    HU3D_ATTR_DISPOFF);
            }
        }
        fn_1_6290(lbl_1_bss_194E[2], 0, lbl_1_bss_1930[0]);

        obj->work[1]++;
        if (obj->work[1] > 180) {
            obj->work[1] = 0;
        }
    }
}

void fn_1_7594(OMOBJ *obj)
{
    MDBANK_MODEL_DATA modelData = lbl_1_rodata_20C;
    s16 i;
    OMOBJ *shapeObj;
    HU3D_MODEL *shapeModel;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 1; i++) {
        obj->mdlId[i] = Hu3DModelCreate(
            HuDataSelHeapReadNum(modelData.data[i], 0x1000, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelPosSet(obj->mdlId[i], lbl_1_rodata_68,
            lbl_1_rodata_1E0, lbl_1_rodata_FC);
        Hu3DModelRotSet(obj->mdlId[i], lbl_1_rodata_100,
            lbl_1_rodata_68, lbl_1_rodata_68);
        Hu3DModelLayerSet(obj->mdlId[i], 3);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_68, lbl_1_rodata_68, HU3D_MOTATTR_LOOP);
        lbl_1_bss_200 = Hu3DAnimCreate(lbl_1_bss_1958[9],
            obj->mdlId[i], lbl_1_data_9DC);
    }

    obj->mdlId[1] = Hu3DModelCreate(
        HuDataSelHeapReadNum(0x00940017, 0x1000, HEAP_MODEL));
    obj->mtnId[1] = Hu3DMotionIDGet(obj->mdlId[1]);
    Hu3DModelPosSet(obj->mdlId[1], lbl_1_rodata_220,
        lbl_1_rodata_224, lbl_1_rodata_11C);
    Hu3DModelRotSet(obj->mdlId[1], lbl_1_rodata_68,
        lbl_1_rodata_160, lbl_1_rodata_68);
    Hu3DModelScaleSet(obj->mdlId[1], lbl_1_rodata_228,
        lbl_1_rodata_228, lbl_1_rodata_228);

    lbl_1_bss_204.x = lbl_1_rodata_220;
    if (lbl_1_bss_2C == 0) {
        lbl_1_bss_204.y = lbl_1_rodata_224;
    } else {
        lbl_1_bss_204.y = lbl_1_rodata_1E0;
    }
    lbl_1_bss_204.z = lbl_1_rodata_11C;
    Hu3DModelLayerSet(obj->mdlId[1], 1);
    Hu3DMotionShiftSet(obj->mdlId[1], obj->mtnId[1],
        lbl_1_rodata_68, lbl_1_rodata_68, HU3D_MOTATTR_LOOP);

    fn_1_11880(lbl_1_bss_194E[2], HUSPR_ATTR_DISPOFF);

    shapeObj = lbl_1_bss_1C;
    shapeModel = &Hu3DData[shapeObj->mdlId[0]];
    Hu3DMotionShapeSet(shapeObj->mdlId[0], shapeObj->mtnId[0]);
    shapeModel->motShapeWork.speed = lbl_1_rodata_1E4;
    Hu3DMotionShapeTimeSet(shapeObj->mdlId[0],
        Hu3DMotionShapeMaxTimeGet(shapeObj->mdlId[0]));
    Hu3DMotionShapeTimeSet(obj->mdlId[1], lbl_1_rodata_68);
    obj->objFunc = fn_1_70E4;
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

void fn_1_780(void)
{
    HuVecF lightPos[2];
    HuVecF lightDir[2];
    GXColor lightColor;
    const u8 *lightColorBytes = &lbl_1_rodata_BC;

    lightPos[0] = lbl_1_rodata_8C[0];
    lightPos[1] = lbl_1_rodata_8C[1];
    lightDir[0] = lbl_1_rodata_A4[0];
    lightDir[1] = lbl_1_rodata_A4[1];
    lightColor.r = lightColorBytes[0];
    lightColor.g = lightColorBytes[1];
    lightColor.b = lightColorBytes[2];
    lightColor.a = lightColorBytes[3];

    lbl_1_bss_1994[0] = Hu3DGLightCreateV(&lightPos[0],
        &lightDir[0], &lightColor);
    Hu3DGLightInfinitytSet(lbl_1_bss_1994[0]);
    Hu3DGLightStaticSet(lbl_1_bss_1994[0], 1);
    lbl_1_bss_1994[1] = Hu3DGLightCreateV(&lightPos[1],
        &lightDir[1], &lightColor);
    Hu3DGLightInfinitytSet(lbl_1_bss_1994[1]);
    Hu3DGLightStaticSet(lbl_1_bss_1994[1], 1);
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
    obj->mdlId[0] = Hu3DModelCreateData(9699330);
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

void fn_1_47AC(s16 index)
{
    MDBANK_ANIM_SET *set = &lbl_1_bss_179C[index];
    HuVecF rot;
    s16 i;

    Hu3DModelRotGet(set->model[0], &rot);
    rot.y += lbl_1_rodata_158;
    if (rot.y > lbl_1_rodata_140) {
        rot.y -= lbl_1_rodata_140;
    }
    Hu3DModelRotSetV(set->model[0], &rot);

    for (i = 0; i < 10; i++) {
        if (i != index) {
            set = &lbl_1_bss_179C[i];
            Hu3DModelRotSet(set->model[0], lbl_1_rodata_68,
                lbl_1_rodata_68, lbl_1_rodata_68);
        }
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
        lbl_1_data_0 = HuAudFXPlay(1423);
    } else if (lbl_1_bss_1930[0] > 60) {
        lbl_1_data_0 = HuAudFXPlay(1422);
    } else if (lbl_1_bss_1930[0] > 40) {
        lbl_1_data_0 = HuAudFXPlay(1421);
    } else if (lbl_1_bss_1930[0] > 20) {
        lbl_1_data_0 = HuAudFXPlay(1420);
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

void fn_1_226C(OMOBJ *obj)
{
    HuVecF pos;
    MDBANK_MOVE_WORK *work = &lbl_1_bss_171C;
    float rotation;

    fn_1_11678(&pos, &work->start, &work->control, &work->end,
        fn_1_11458(lbl_1_rodata_68, lbl_1_rodata_74,
            work->time, work->duration));
    Hu3DModelPosSetV(obj->mdlId[0], &pos);
    rotation = fn_1_11458(lbl_1_rodata_68, lbl_1_rodata_118,
        work->time, work->duration);
    Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_68,
        rotation, lbl_1_rodata_68);
    if ((work->time += lbl_1_rodata_74) > work->duration) {
        obj->objFunc = NULL;
    }
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

void fn_1_3E30(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 80; i++) {
        if (i == 0) {
            obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
                0x940017, HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            obj->mdlId[i] = Hu3DModelLink(obj->mdlId[0]);
        }
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelScaleSet(obj->mdlId[i], lbl_1_rodata_188,
            lbl_1_rodata_188, lbl_1_rodata_188);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_68, lbl_1_rodata_68, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }

    for (i = 0; i < 20; i++) {
        lbl_1_bss_210[i].x = (float)(frandmod(120) - 60);
        lbl_1_bss_210[i].y = (float)(frandmod(80) - 40);
        lbl_1_bss_210[i].z = (float)frandmod(20);
        Hu3DModelPosSet(obj->mdlId[i], lbl_1_bss_210[i].x,
            lbl_1_bss_210[i].y, lbl_1_bss_210[i].z);
        lbl_1_bss_210[i].y = (float)(frandmod(90) - 45);
        Hu3DModelRotSet(obj->mdlId[i], lbl_1_rodata_68,
            lbl_1_bss_210[i].y, lbl_1_rodata_68);
        Hu3DModelScaleSet(obj->mdlId[i], lbl_1_rodata_18C,
            lbl_1_rodata_18C, lbl_1_rodata_18C);
    }
    obj->objFunc = NULL;
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

void fn_1_5D64(OMOBJ *obj)
{
    MDBANK_ANIM_SET *set;
    char *name[6];
    s16 i;
    s16 j;
    s16 col;
    s16 row;

    set = lbl_1_bss_179C;
    name[0] = (char *)lbl_1_rodata_1C8[0];
    name[1] = (char *)lbl_1_rodata_1C8[1];
    name[2] = (char *)lbl_1_rodata_1C8[2];
    name[3] = (char *)lbl_1_rodata_1C8[3];
    name[4] = (char *)lbl_1_rodata_1C8[4];
    name[5] = (char *)lbl_1_rodata_1C8[5];

    for (i = 0; i < 36; i++) {
        lbl_1_bss_18A0[i] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_19C[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }

    for (i = 0; i < 10; i++, set++) {
        memset(set, 0, sizeof(*set));
        if (i == 0) {
            set->model[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
                0x94001B, HU_MEMNUM_OVL, HEAP_MODEL));
            set->model[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
                0x94001A, HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            set->model[0] = Hu3DModelLink(lbl_1_bss_179C[0].model[0]);
            set->model[1] = Hu3DModelLink(lbl_1_bss_179C[0].model[1]);
        }

        col = (s16)(i % 5);
        row = (s16)(i / 5);
        Hu3DModelPosSet(set->model[0],
            (float)(col * 100 - 200),
            (float)(235 - row * 100), lbl_1_rodata_11C);
        Hu3DModelPosSet(set->model[1],
            (float)(col * 100 - 200),
            (float)(235 - row * 100), lbl_1_rodata_11C);

        for (j = 0; j < 6; j++) {
            set->anim[j] = Hu3DAnimCreate(lbl_1_bss_18A0[0],
                set->model[0], name[j]);
        }
        set->spriteGroup = HuSprGrpCreate(3);
        for (j = 0; j < 3; j++) {
            set->state[j] = HuSprCreate(lbl_1_bss_1958[4],
                (s16)(j + 100), 0);
            HuSprGrpMemberSet(set->spriteGroup, j, set->state[j]);
            HuSprScaleSet(set->spriteGroup, j, lbl_1_rodata_188,
                lbl_1_rodata_188);
            HuSpr3DSet(set->state[j]);
            HuSpr3DFovSet(set->state[j], lbl_1_rodata_68);
        }
        HuSprGrpDrawNoSet(set->spriteGroup, 0x40);
        fn_1_11880(set->spriteGroup, HUSPR_ATTR_DISPOFF);
    }

    set = lbl_1_bss_179C;
    for (i = 0; i < 10; i++, set++) {
        Hu3DModelAttrSet(set->model[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(set->model[1], HU3D_ATTR_DISPOFF);
        fn_1_11880(set->spriteGroup, HUSPR_ATTR_DISPOFF);
    }
    obj->objFunc = NULL;
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

void fn_1_7A84(OMOBJ *obj)
{
    MDBANK_POST_CALLBACK_WORK *work;
    float opacity;
    s16 i;

    work = lbl_1_bss_40;
    for (i = 0; i < 6; i++, work++) {
        if (work->time < lbl_1_rodata_68) {
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(obj->mdlId[i + 6], HU3D_ATTR_DISPOFF);
        } else if (work->time == lbl_1_rodata_68) {
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(obj->mdlId[i + 6], HU3D_ATTR_DISPOFF);
            Hu3DModelTPLvlSet(obj->mdlId[i], lbl_1_rodata_68);
            Hu3DModelTPLvlSet(obj->mdlId[i + 6], lbl_1_rodata_68);
            Hu3DMotionSet(obj->mdlId[i], obj->mtnId[i]);
            Hu3DMotionSet(obj->mdlId[i + 6], obj->mtnId[i + 6]);
        } else if (work->time <= lbl_1_rodata_15C) {
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(obj->mdlId[i + 6], HU3D_ATTR_DISPOFF);
            opacity = fn_1_1116C(lbl_1_rodata_68, lbl_1_rodata_188,
                work->time, lbl_1_rodata_15C);
            Hu3DModelTPLvlSet(obj->mdlId[i], opacity);
            Hu3DModelTPLvlSet(obj->mdlId[i + 6], opacity);
        } else if (work->time <= lbl_1_rodata_178) {
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(obj->mdlId[i + 6], HU3D_ATTR_DISPOFF);
            Hu3DModelTPLvlSet(obj->mdlId[i], lbl_1_rodata_188);
            Hu3DModelTPLvlSet(obj->mdlId[i + 6], lbl_1_rodata_188);
        } else {
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(obj->mdlId[i + 6], HU3D_ATTR_DISPOFF);
            opacity = fn_1_1116C(lbl_1_rodata_188, lbl_1_rodata_68,
                work->time - lbl_1_rodata_178, lbl_1_rodata_58);
            Hu3DModelTPLvlSet(obj->mdlId[i], opacity);
            Hu3DModelTPLvlSet(obj->mdlId[i + 6], opacity);
        }

        work->time += lbl_1_rodata_74;
        if (work->time > work->duration) {
            work->time = lbl_1_rodata_22C;
            work->duration = lbl_1_rodata_134;
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(obj->mdlId[i + 6], HU3D_ATTR_DISPOFF);
        }
    }

    if (obj->work[3] == 0) {
        for (i = 0; i < 6; i++) {
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        }
    } else {
        for (i = 0; i < 6; i++) {
            Hu3DModelAttrSet(obj->mdlId[i + 6], HU3D_ATTR_DISPOFF);
        }
    }
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
    fn_1_12F8(1, 1572864, 1);
    fn_1_11A0();
    fn_1_1C64();
    fn_1_12F8(3, 1572865, 1);
    fn_1_11A0();
    obj = lbl_1_bss_4;
    obj->objFunc = fn_1_2750;
    return 1;
}

void fn_1_9DE8(void)
{
    OMOBJ *obj;
    MDBANK_ANIM_SET *set;
    MDBANK_MOVE_WORK *work;
    s16 previousWindow;
    s16 i;

    if (lbl_1_data_932[0] != -1 && lbl_1_data_932[0] != 2) {
        HuWinHomeClear(lbl_1_data_932[0]);
        previousWindow = lbl_1_data_932[0];
        if (previousWindow == 0) {
            HuWinDispOff(lbl_1_bss_198C[previousWindow]);
        } else {
            HuWinExClose(lbl_1_bss_198C[previousWindow]);
        }
    }

    if (lbl_1_data_932[0] != 2) {
        lbl_1_data_932[0] = 2;
        lbl_1_data_938[0] = -1;
        lbl_1_data_938[1] = -1;
        if (lbl_1_data_932[0] == 0) {
            HuWinDispOn(lbl_1_bss_198C[lbl_1_data_932[0]]);
        } else {
            HuWinExOpen(lbl_1_bss_198C[lbl_1_data_932[0]]);
        }
    }

    if (lbl_1_data_938[0] != 0x00180002) {
        lbl_1_data_938[0] = 0x00180002;
        lbl_1_data_938[1] = -1;
        HuWinAttrSet(lbl_1_bss_198C[lbl_1_data_932[0]], 0x800);
        HuWinMesSet(lbl_1_bss_198C[lbl_1_data_932[0]],
            lbl_1_data_938[0]);
        HuWinMesSpeedSet(lbl_1_bss_198C[lbl_1_data_932[0]], 1);
        if (lbl_1_data_90C != lbl_1_data_938[0]) {
            lbl_1_data_90C = -1;
        }
    }
    if (lbl_1_data_932[0] != -1) {
        HuWinMesWait(lbl_1_bss_198C[lbl_1_data_932[0]]);
    }

    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[2],
        lbl_1_rodata_68, lbl_1_rodata_58, 0);
    Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[2],
        lbl_1_rodata_68, lbl_1_rodata_58, 0);
    HuPrcSleep(0x2D);

    work = &lbl_1_bss_175C;
    fn_1_1115C(&work->start, lbl_1_rodata_FC, lbl_1_rodata_100,
        lbl_1_rodata_68);
    fn_1_1115C(&work->control, lbl_1_rodata_104, lbl_1_rodata_100,
        lbl_1_rodata_68);
    fn_1_1115C(&work->end, lbl_1_rodata_108, lbl_1_rodata_10C,
        lbl_1_rodata_68);
    work->time = lbl_1_rodata_68;
    work->duration = lbl_1_rodata_110;
    lbl_1_bss_C->objFunc = fn_1_1DDC;

    work = &lbl_1_bss_171C;
    fn_1_1115C(&work->start, lbl_1_rodata_11C, lbl_1_rodata_100,
        lbl_1_rodata_68);
    fn_1_1115C(&work->control, lbl_1_rodata_120, lbl_1_rodata_100,
        lbl_1_rodata_68);
    fn_1_1115C(&work->end, lbl_1_rodata_124, lbl_1_rodata_10C,
        lbl_1_rodata_68);
    work->time = lbl_1_rodata_68;
    work->duration = lbl_1_rodata_110;
    lbl_1_bss_10->objFunc = fn_1_226C;
    HuPrcSleep(0x1E);

    lbl_1_bss_1C->work[2] = 1;
    obj = lbl_1_bss_18;
    set = lbl_1_bss_179C;
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
    obj->work[0] = 0;
    obj->work[1] = 60;
    obj->objFunc = fn_1_5988;
    HuPrcSleep(0x1E);

    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[0],
        lbl_1_rodata_68, lbl_1_rodata_58, 0x40000001);
    Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[0],
        lbl_1_rodata_68, lbl_1_rodata_58, 0x40000001);
    HuPrcSleep(0x3C);
}

void fn_1_2750(OMOBJ *obj)
{
    float opacity;

    opacity = fn_1_1116C(lbl_1_rodata_74, lbl_1_rodata_68,
        (float)obj->work[0], lbl_1_rodata_5C);
    HuSprGrpTPLvlSet(lbl_1_bss_194E[0], opacity);
    obj->work[0]++;
    if (obj->work[0] > 10) {
        fn_1_11880(lbl_1_bss_194E[0], HUSPR_ATTR_DISPOFF);
        obj->objFunc = NULL;
    }
}

s32 fn_1_EEB8(s16 index)
{
    if (lbl_1_data_22C[index].value[0] == 0) {
        return 1;
    }
    if (lbl_1_data_22C[index].value[0] == 1) {
        if (lbl_1_data_22C[0].value[10] == 0) {
            fn_1_1C64();
            fn_1_12F8(3, MDBANK_ITEM_MESSAGE_INITIAL_UNLOCK_REQUIRED, 1);
            fn_1_11A0();
            return 0;
        }
        return 1;
    }
    if (lbl_1_data_22C[index].value[0] == 2) {
        if (GWMgUnlockGet(MDBANK_UNLOCK_MINIGAME_M678) == 0) {
            fn_1_1C64();
            fn_1_12F8(3, MDBANK_ITEM_MESSAGE_M678_UNLOCK_REQUIRED, 1);
            fn_1_11A0();
            return 0;
        }
        return 1;
    }
    if (lbl_1_data_22C[index].value[0] == 3) {
        if (GWMgUnlockGet(MDBANK_UNLOCK_MINIGAME_M679) == 0) {
            fn_1_1C64();
            fn_1_12F8(3, MDBANK_ITEM_MESSAGE_M679_UNLOCK_REQUIRED, 1);
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
            fn_1_12F8(2, MDBANK_ITEM_MESSAGE_BANK_FLAG_ENABLE_CONFIRM, 1);
            if (fn_1_1200(2) == 0) {
                GWBankFlagSet(60);
            }
        } else {
            fn_1_1D68();
            fn_1_12F8(2, MDBANK_ITEM_MESSAGE_BANK_FLAG_DISABLE_CONFIRM, 1);
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
        fn_1_12F8(2, MDBANK_ITEM_MESSAGE_STATE_ONE_NOTICE, 1);
        fn_1_11A0();
    } else if (lbl_1_data_22C[index].value[1] == 2) {
        fn_1_1D68();
        fn_1_12F8(2, MDBANK_ITEM_MESSAGE_STATE_TWO_NOTICE, 1);
        fn_1_11A0();
    } else if (lbl_1_data_22C[index].value[1] == 3) {
        fn_1_1D68();
        fn_1_12F8(2, MDBANK_ITEM_MESSAGE_STATE_THREE_NOTICE, 1);
        fn_1_11A0();
    } else {
        fn_1_1D68();
        fn_1_12F8(2, lbl_1_data_22C[index].choice.message, 1);
        fn_1_11A0();
    }
}

static s16 aa08_full_index(void)
{
    return (s16)((lbl_1_bss_1930[1] + lbl_1_bss_1930[3]) * 5
        + lbl_1_bss_1930[2]);
}

static s16 aa08_local_index(void)
{
    return (s16)(lbl_1_bss_1930[3] * 5 + lbl_1_bss_1930[2]);
}

static s16 aa08_table_index(void)
{
    s16 index = aa08_full_index();

    if (index >= 55) {
        index -= 55;
    }
    return index;
}

/* Window contracts are the exact F68/B98/C54/AC4 family inlined by AA08. */
static s32 aa08_status_message(const MDBANK_ITEM *item)
{
    return *((const s32 *)((const u8 *)item + 0x18));
}

static void aa08_win_open(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_198C[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_198C[winNo]);
    }
}

static void aa08_win_close(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_198C[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_198C[winNo]);
    }
}

static void aa08_win_wait(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_198C[winNo]);
}

static s16 aa08_win_choice(s16 winNo, s16 mode)
{
    s16 choice;

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

static void aa08_win_select(s16 winNo)
{
    if (lbl_1_data_932[0] != -1 && lbl_1_data_932[0] != winNo) {
        HuWinHomeClear(lbl_1_bss_198C[lbl_1_data_932[0]]);
        aa08_win_close(lbl_1_data_932[0]);
    }
    if (lbl_1_data_932[0] == -1 || lbl_1_data_932[0] != winNo) {
        lbl_1_data_932[0] = winNo;
        lbl_1_data_938[0] = -1;
        lbl_1_data_938[1] = -1;
        aa08_win_open(winNo);
    }
}

static void aa08_win_message_raw(s16 winNo, u32 message, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_198C[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_198C[winNo], message);
    HuWinMesSpeedSet(lbl_1_bss_198C[winNo], speed);
    if (lbl_1_data_90C != message) {
        lbl_1_data_90C = (u32)-1;
    }
}

static void aa08_win_insert_raw(s16 winNo, u32 message, s16 insert)
{
    HuWinAttrSet(lbl_1_bss_198C[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinHomeClear(lbl_1_bss_198C[winNo]);
    HuWinInsertMesSet(lbl_1_bss_198C[winNo], message, insert);
}

static void aa08_win_message(s16 winNo, u32 message, s16 speed)
{
    aa08_win_select(winNo);
    if (lbl_1_data_938[0] != (s32)message) {
        lbl_1_data_938[0] = (s32)message;
        lbl_1_data_938[1] = -1;
        aa08_win_message_raw(lbl_1_data_932[0], message, speed);
    }
}

static void aa08_win_insert(s16 winNo, u32 message, s16 insert)
{
    aa08_win_select(winNo);
    if (lbl_1_data_938[1] != (s32)message) {
        lbl_1_data_938[0] = -1;
        lbl_1_data_938[1] = (s32)message;
        aa08_win_insert_raw(lbl_1_data_932[0], message, insert);
    }
}

static void aa08_win_secondary_message(u32 message)
{
    if (lbl_1_data_932[1] == -1) {
        lbl_1_data_932[1] = 0;
        lbl_1_data_938[2] = -1;
        aa08_win_open(0);
    }
    if (lbl_1_data_938[2] != (s32)message) {
        lbl_1_data_938[2] = (s32)message;
        aa08_win_message_raw(0, message, 0);
    }
}

static void aa08_win_secondary_close(void)
{
    if (lbl_1_data_932[1] != -1) {
        aa08_win_close(lbl_1_data_932[1]);
    }
    lbl_1_data_932[1] = -1;
    lbl_1_data_938[2] = -1;
}

static void aa08_win_close_primary(void)
{
    if (lbl_1_data_932[0] != -1) {
        aa08_win_close(lbl_1_data_932[0]);
    }
    lbl_1_data_932[0] = -1;
    lbl_1_data_938[0] = -1;
    lbl_1_data_938[1] = -1;
}

static void aa08_reset_rotations(void)
{
    s16 i;

    for (i = 0; i < 10; i++) {
        Hu3DModelRotSet(lbl_1_bss_179C[i].model[0],
            lbl_1_rodata_68, lbl_1_rodata_68, lbl_1_rodata_68);
    }
}

static void aa08_project_digit(s16 member, float yOffset, s16 workIndex)
{
    HuVecF world = lbl_1_rodata_230;
    HuVecF screen = lbl_1_rodata_148;

    if (lbl_1_bss_14->work[workIndex] != 1) {
        /* Every four retail projections use the fixed rodata_230 anchor;
         * rodata_148 is the stack destination seed, not a second source. */
        Hu3D3Dto2D(&world, 1, &screen);
        HuSprPosSet(lbl_1_bss_194E[1], member,
            screen.x + lbl_1_rodata_68, screen.y + yOffset);
        HuSprScaleSet(lbl_1_bss_194E[1], member,
            lbl_1_rodata_154, lbl_1_rodata_154);
        HuSprAttrReset(lbl_1_bss_194E[1], member, HUSPR_ATTR_DISPOFF);
        lbl_1_bss_14->work[workIndex] = 1;
        lbl_1_data_964[member] = 1;
        lbl_1_bss_1700[member] = lbl_1_rodata_154;
    }
}

/* R0's 0xAA08-0xAFB4 bootstrap and selection work. */
static void aa08_region_R0(s16 *frameCounter)
{
    HuVecF position;
    s16 index;

    HuPrcVSleep();
    aa08_win_secondary_message(10);

    aa08_project_digit(2, lbl_1_rodata_178, 2);
    aa08_project_digit(3, lbl_1_rodata_23C, 3);
    if (lbl_1_bss_1930[2] == 0) {
        lbl_1_bss_14->work[3] = 0;
        lbl_1_data_964[3] = 0;
        HuSprAttrSet(lbl_1_bss_194E[1], 3, HUSPR_ATTR_DISPOFF);
    }
    if (lbl_1_bss_1930[2] == 9) {
        lbl_1_bss_14->work[2] = 0;
        lbl_1_data_964[2] = 0;
        HuSprAttrSet(lbl_1_bss_194E[1], 2, HUSPR_ATTR_DISPOFF);
    }

    index = aa08_local_index();
    Hu3DModelPosGet(lbl_1_bss_179C[index].model[0], &position);
    Hu3DModelPosSetV(lbl_1_bss_14->mdlId[0], &position);
    Hu3DModelRotSet(lbl_1_bss_14->mdlId[0],
        lbl_1_rodata_68, lbl_1_rodata_68, lbl_1_rodata_160);
    Hu3DModelScaleSet(lbl_1_bss_14->mdlId[0],
        lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_74);
    lbl_1_bss_1710.x = position.x - lbl_1_rodata_15C;
    lbl_1_bss_1710.y = lbl_1_rodata_15C + position.y;
    lbl_1_bss_1710.z = lbl_1_rodata_15C + position.z;
    Hu3DModelAttrReset(lbl_1_bss_14->mdlId[0], HU3D_ATTR_DISPOFF);
    *frameCounter = 0;
}

/* R1 0xAFB4-0xB890: status-specific message cache, then frame gate. */
static void aa08_region_R1(s16 *frameCounter)
{
    s16 index;
    MDBANK_ITEM *item;

    HuPrcVSleep();
    index = aa08_table_index();
    item = &lbl_1_data_22C[index];

    if (item->value[10] == 0) {
        aa08_win_insert(2, (u32)aa08_status_message(item), 0);
    } else {
        aa08_win_insert(2, (u32)aa08_status_message(item), 0);
    }
    aa08_win_message(2, (u32)0x18000004, 0);

    ++*frameCounter;
    if (*frameCounter > 60) {
        *frameCounter = 60;
    }
}

/* Helper used at C008: idle rotation, reset of non-selected slots, and bss1710. */
static void aa08_update_selected_work(s16 frame)
{
    HuVecF rotation;
    HuVecF position;
    s16 i;
    s16 index = aa08_local_index();

    if (frame >= 30) {
        Hu3DModelRotGet(lbl_1_bss_179C[index].model[0], &rotation);
        rotation.y += lbl_1_rodata_158;
        if (rotation.y > lbl_1_rodata_140) {
            rotation.y -= lbl_1_rodata_140;
        }
        Hu3DModelRotSetV(lbl_1_bss_179C[index].model[0], &rotation);
    }

    for (i = 0; i < 10; i++) {
        if (i != index) {
            Hu3DModelRotSet(lbl_1_bss_179C[i].model[0],
                lbl_1_rodata_68, lbl_1_rodata_68, lbl_1_rodata_68);
        }
    }

    Hu3DModelPosGet(lbl_1_bss_179C[index].model[0], &position);
    lbl_1_bss_1710.x = position.x - lbl_1_rodata_15C;
    lbl_1_bss_1710.y = lbl_1_rodata_15C + position.y;
    lbl_1_bss_1710.z = lbl_1_rodata_15C + position.z;
}

/* R2 0xB890-0xC1D8: exact D-pad masks, gating and button edges. */
static s16 aa08_region_R2(s16 *frameCounter)
{
    for (;;) {
        u8 dpad = HuPadDStkRep[0];
        u16 buttons = HuPadBtnDown[0];

        if (*frameCounter > 10) {
            if (dpad & 0x08) {
                if (lbl_1_bss_1930[3] == 1 || lbl_1_bss_1930[2] > 0) {
                    HuAudFXPlay(0);
                }
                if (lbl_1_bss_1930[3] == 1 || lbl_1_bss_1930[2] > 0) {
                    aa08_reset_rotations();
                }
                if (lbl_1_bss_1930[3] == 0) {
                    aa08_project_digit(3, lbl_1_rodata_23C, 3);
                    --lbl_1_bss_1930[2];
                    if (lbl_1_bss_1930[2] <= 0) {
                        lbl_1_bss_14->work[3] = 0;
                        lbl_1_data_964[3] = 0;
                        HuSprAttrSet(lbl_1_bss_194E[1], 3, HUSPR_ATTR_DISPOFF);
                        lbl_1_bss_1930[2] = 0;
                    }
                    lbl_1_bss_1700[3] = lbl_1_rodata_154;
                } else {
                    lbl_1_bss_1930[3] = 0;
                }
            } else if (dpad & 0x04) {
                if (lbl_1_bss_1930[3] == 0 || lbl_1_bss_1930[2] < 9) {
                    HuAudFXPlay(0);
                }
                if (lbl_1_bss_1930[3] == 0 || lbl_1_bss_1930[2] < 9) {
                    aa08_reset_rotations();
                }
                if (lbl_1_bss_1930[3] == 1) {
                    aa08_project_digit(3, lbl_1_rodata_23C, 3);
                    ++lbl_1_bss_1930[2];
                    if (lbl_1_bss_1930[2] >= 9) {
                        lbl_1_bss_14->work[2] = 0;
                        lbl_1_data_964[2] = 0;
                        HuSprAttrSet(lbl_1_bss_194E[1], 2, HUSPR_ATTR_DISPOFF);
                        lbl_1_bss_1930[2] = 9;
                    }
                    lbl_1_bss_1700[3] = lbl_1_rodata_154;
                } else {
                    lbl_1_bss_1930[3] = 1;
                }
            } else if (dpad & 0x01) {
                HuAudFXPlay(0);
                --lbl_1_bss_1930[1];
                if (lbl_1_bss_1930[1] < 0) {
                    lbl_1_bss_1930[1] += 5;
                }
                aa08_reset_rotations();
            } else if (dpad & 0x02) {
                HuAudFXPlay(0);
                ++lbl_1_bss_1930[1];
                if (lbl_1_bss_1930[1] > 4) {
                    lbl_1_bss_1930[1] -= 5;
                }
                aa08_reset_rotations();
            } else {
                if (buttons & 0x0200) {
                    HuAudFXPlay(3);
                    return -1;
                }
                if (buttons & 0x0400) {
                    HuAudFXPlay(3);
                    return -1;
                }
                if (buttons & 0x0100) {
                    HuAudFXPlay(2);
                    return 1;
                }
            }
        }

        aa08_update_selected_work(*frameCounter);
        aa08_region_R1(frameCounter);
    }
}

/* R3 0xC1D8-0xC6AC. Return 1=preview, 0=stay in the menu after the
 * confirmation dialog, and -1=leave the bank screen. */
static s16 aa08_region_R3(s16 navigation)
{
    s16 choice;

    aa08_reset_rotations();
    aa08_win_secondary_close();
    lbl_1_bss_14->work[2] = 0;
    lbl_1_data_964[2] = 0;
    HuSprAttrSet(lbl_1_bss_194E[1], 2, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_14->work[3] = 0;
    lbl_1_data_964[3] = 0;
    HuSprAttrSet(lbl_1_bss_194E[1], 3, HUSPR_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_14->mdlId[0], HU3D_ATTR_DISPOFF);

    if (navigation != -1) {
        return 1;
    }

    Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[1],
        lbl_1_rodata_68, lbl_1_rodata_F8, 0);
    lbl_1_bss_10->work[3] = 0;
    lbl_1_bss_10->objFunc = fn_1_1CD8;
    aa08_win_message(2, (u32)0x18000009, 1);
    choice = aa08_win_choice(2, 2);
    return choice != 0 ? 0 : -1;
}

/* R4 0xC6AC-0xCA88. Target loop is frame <= 30 (31 sleeps). */
static void aa08_region_R4(void)
{
    MDBANK_ANIM_SET *set = &lbl_1_bss_179C[aa08_local_index()];
    HuVecF position;
    HuVecF rotation;
    HuVecF value;
    s16 frame;

    Hu3DModelPosGet(set->model[0], &position);
    Hu3DModelRotGet(set->model[0], &rotation);
    Hu3DModelLayerSet(set->model[0], 3);
    HuSprGrpDrawNoSet(set->spriteGroup, 0x41);

    for (frame = 0; frame <= 30; ++frame) {
        HuPrcVSleep();
        value.x = fn_1_11458(position.x, lbl_1_rodata_68,
            (float)frame, 30.0f);
        value.y = fn_1_11458(position.y, lbl_1_rodata_1B8,
            (float)frame, 30.0f);
        value.z = fn_1_11458(position.z, lbl_1_rodata_120,
            (float)frame, 30.0f);
        Hu3DModelPosSetV(set->model[0], &value);
        value.x = lbl_1_rodata_68;
        value.y = fn_1_11458(rotation.y, lbl_1_rodata_1BC,
            (float)frame, 30.0f);
        value.z = lbl_1_rodata_68;
        Hu3DModelRotSetV(set->model[0], &value);
        value.x = fn_1_11458(lbl_1_rodata_74, lbl_1_rodata_1C0,
            (float)frame, 30.0f);
        value.y = value.x;
        value.z = lbl_1_rodata_74;
        Hu3DModelScaleSetV(set->model[0], &value);
    }
    Hu3DModelPosSet(set->model[0], lbl_1_rodata_68,
        lbl_1_rodata_1B8, lbl_1_rodata_120);
    Hu3DModelRotSet(set->model[0], lbl_1_rodata_68,
        lbl_1_rodata_68, lbl_1_rodata_68);
    Hu3DModelScaleSet(set->model[0], lbl_1_rodata_1C0,
        lbl_1_rodata_1C0, lbl_1_rodata_74);
}

static void aa08_wait_for_obj(OMOBJ *obj)
{
    while (obj->objFunc != NULL) {
        HuPrcVSleep();
    }
}

/* R5 0xCA88-0xD070. Return 0 only for a successful choice. */
static s16 aa08_region_R5(s16 index, char *starText)
{
    MDBANK_ITEM *item = &lbl_1_data_22C[index];
    s16 choice;

    lbl_1_bss_18->work[0] = 0;
    lbl_1_bss_18->work[1] = 1;
    lbl_1_bss_18->work[2] = aa08_local_index();

    if (lbl_1_bss_1930[0] < item->value[6]) {
        /* Target shared branch .L_E040: message 0x18000006, speed 1. */
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[1],
            lbl_1_rodata_68, lbl_1_rodata_F8, 0);
        lbl_1_bss_C->work[3] = 0;
        lbl_1_bss_C->objFunc = fn_1_1BD4;
        aa08_win_message(3, (u32)0x18000006, 1);
        if (lbl_1_data_932[0] != -1) {
            aa08_win_wait(lbl_1_data_932[0]);
        }
        return 1;
    }

    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[1],
        lbl_1_rodata_68, lbl_1_rodata_F8, 0);
    lbl_1_bss_C->work[3] = 0;
    lbl_1_bss_C->objFunc = fn_1_1BD4;
    sprintf(starText, lbl_1_data_A1C, item->value[6]);
    aa08_win_insert(3, (u32)starText, 0);
    choice = aa08_win_choice(3, 2);
    if (lbl_1_data_932[0] != -1) {
        aa08_win_wait(lbl_1_data_932[0]);
    }
    return choice == 0 ? 0 : 1;
}

/* R6 0xD070-0xD62C. */
static s16 aa08_region_R6(s16 index)
{
    MDBANK_ANIM_SET *set;
    s16 localIndex;
    s16 pass;
    s16 slot;
    float eased;

    if (fn_1_EEB8(index) != 1) {
        return 0;
    }
    HuAudFXPlay(0x35);
    localIndex = aa08_local_index();
    set = &lbl_1_bss_179C[localIndex];

    for (pass = 0; pass < 12; ++pass) {
        HuPrcVSleep();
        eased = fn_1_11064(lbl_1_rodata_74, lbl_1_rodata_68,
            (float)pass, lbl_1_rodata_5C);
        for (slot = 0; slot < 10; ++slot) {
            if (slot != localIndex) {
                set = &lbl_1_bss_179C[slot];
                set->state[3] = 1;
                Hu3DModelScaleSet(set->model[0], eased, eased, eased);
                Hu3DModelScaleSet(set->model[1], eased, eased, eased);
                Hu3DModelRotSet(set->model[0], lbl_1_rodata_68,
                    lbl_1_rodata_1C4 * eased, lbl_1_rodata_68);
                Hu3DModelRotSet(set->model[1], lbl_1_rodata_68,
                    lbl_1_rodata_1C4 * eased, lbl_1_rodata_68);
                fn_1_11880(set->spriteGroup, 4);
            }
        }
    }

    set = &lbl_1_bss_179C[localIndex];
    Hu3DModelRotSet(set->model[0], lbl_1_rodata_68,
        lbl_1_rodata_68, lbl_1_rodata_68);
    HuAudFXPlay(0x586);
    return 1;
}

/* R7 0xD62C-0xD89C. */
static void aa08_region_R7(s16 index)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1C->mdlId[0]];

    Hu3DMotionShapeSet(lbl_1_bss_1C->mdlId[0], lbl_1_bss_1C->mtnId[0]);
    model->motShapeWork.speed = lbl_1_rodata_154;
    HuPrcSleep(30);
    lbl_1_bss_24[0]->work[0] = 0;
    fn_1_13764(1);
    HuAudFXPlay(0x589);

    lbl_1_data_972 = index;
    lbl_1_data_970 = lbl_1_data_22C[index].value[3];
    lbl_1_data_974 = 5;
    lbl_1_bss_20->objFunc = fn_1_3788;
    aa08_wait_for_obj(lbl_1_bss_20);
    if (lbl_1_bss_1930[0] > 80) {
        lbl_1_data_0 = HuAudFXPlay(1423);
    } else if (lbl_1_bss_1930[0] > 60) {
        lbl_1_data_0 = HuAudFXPlay(1422);
    } else if (lbl_1_bss_1930[0] > 40) {
        lbl_1_data_0 = HuAudFXPlay(1421);
    } else if (lbl_1_bss_1930[0] > 20) {
        lbl_1_data_0 = HuAudFXPlay(1420);
    } else {
        HuAudFXStop(lbl_1_data_0);
    }
    lbl_1_bss_24[0]->work[0] = 1;
    fn_1_13764(0);
}

/* R8 0xD89C-0xDAC8. */
static void aa08_region_R8(s16 index)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1C->mdlId[0]];
    HuVecF position;

    lbl_1_bss_24[0]->work[0] = 1;
    fn_1_13764(0);
    HuPrcSleep(30);
    Hu3DMotionShapeSet(lbl_1_bss_1C->mdlId[0], lbl_1_bss_1C->mtnId[0]);
    model->motShapeWork.speed = lbl_1_rodata_1E4;
    Hu3DMotionShapeTimeSet(lbl_1_bss_1C->mdlId[0],
        Hu3DMotionShapeMaxTimeGet(lbl_1_bss_1C->mdlId[0]));
    HuAudFXPlay(0x585);
    HuAudFXPlay(0x58B);

    Hu3DModelPosGet(lbl_1_bss_179C[aa08_local_index()].model[0], &position);
    fn_1_13598(&position);
    HuPrcSleep(30);
    lbl_1_data_22C[index].value[10] = 1;
    HuPrcSleep(30);

    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[1],
        lbl_1_rodata_68, lbl_1_rodata_F8, 0);
    lbl_1_bss_C->work[3] = 0;
    lbl_1_bss_C->objFunc = fn_1_1BD4;
}

/* R9 0xDAC8-0xE2FC: insert choice message in window 3 then notice. */
static void aa08_region_R9(s16 index)
{
    s16 localIndex;
    float eased;
    s16 pass;
    s16 slot;

    aa08_win_insert(3, (u32)aa08_status_message(&lbl_1_data_22C[index]), 0);
    if (lbl_1_data_932[0] != -1) {
        aa08_win_wait(lbl_1_data_932[0]);
    }
    fn_1_1050C(index);
    lbl_1_bss_18->work[1] = 0;
    HuAudFXPlay(0x34);
    localIndex = aa08_local_index();

    for (pass = 0; pass < 12; ++pass) {
        HuPrcVSleep();
        eased = fn_1_11458(lbl_1_rodata_68, lbl_1_rodata_74,
            (float)pass, lbl_1_rodata_5C);
        for (slot = 0; slot < 10; ++slot) {
            MDBANK_ANIM_SET *set = &lbl_1_bss_179C[slot];

            if (slot != localIndex) {
                set->state[3] = 0;
                Hu3DModelScaleSet(set->model[0], eased, eased, eased);
                Hu3DModelScaleSet(set->model[1], eased, eased, eased);
                Hu3DModelRotSet(set->model[0], lbl_1_rodata_68,
                    lbl_1_rodata_1C4 * eased, lbl_1_rodata_68);
                Hu3DModelRotSet(set->model[1], lbl_1_rodata_68,
                    lbl_1_rodata_1C4 * eased, lbl_1_rodata_68);
            }
        }
    }
}

/* R10 0xE2FC-0xE640. */
static s32 aa08_region_R10(void)
{
    s16 index = aa08_table_index();
    s32 action = fn_1_F7C0(index);

    if (action == 1) {
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[1],
            lbl_1_rodata_68, lbl_1_rodata_F8, 0);
        lbl_1_bss_10->work[3] = 0;
        lbl_1_bss_10->objFunc = fn_1_1CD8;
        aa08_win_message(2, (u32)lbl_1_data_22C[index].choice.message, 1);
        if (lbl_1_data_932[0] != -1) {
            aa08_win_wait(lbl_1_data_932[0]);
        }
        return 0;
    }
    return action;
}

/* R11 0xE640-0xEA98. Target loop is frame <= 30 (31 sleeps). */
static void aa08_region_R11(void)
{
    MDBANK_ANIM_SET *set = &lbl_1_bss_179C[aa08_local_index()];
    HuVecF position;
    HuVecF rotation;
    HuVecF target;
    HuVecF value;
    s16 index = aa08_local_index();
    s16 page = index / 5;
    s16 item = index - page * 5;
    s16 frame;

    Hu3DModelPosGet(set->model[0], &position);
    Hu3DModelRotGet(set->model[0], &rotation);
    fn_1_1115C(&target, item * 100.0f - 200.0f,
        235.0f - page * 100.0f, lbl_1_rodata_11C);
    lbl_1_bss_18->work[1] = 0;

    for (frame = 0; frame <= 30; ++frame) {
        HuPrcVSleep();
        value.x = fn_1_11458(position.x, target.x,
            (float)frame, 30.0f);
        value.y = fn_1_11458(position.y, target.y,
            (float)frame, 30.0f);
        value.z = fn_1_11458(position.z, target.z,
            (float)frame, 30.0f);
        Hu3DModelPosSetV(set->model[0], &value);

        value.x = lbl_1_rodata_68;
        value.y = fn_1_11458(lbl_1_rodata_1BC, rotation.y,
            (float)frame, 30.0f);
        value.z = lbl_1_rodata_68;
        Hu3DModelRotSetV(set->model[0], &value);

        value.x = fn_1_11458(lbl_1_rodata_1C0, lbl_1_rodata_74,
            (float)frame, 30.0f);
        value.y = value.x;
        value.z = lbl_1_rodata_74;
        Hu3DModelScaleSetV(set->model[0], &value);
    }
    Hu3DModelPosSet(set->model[0], target.x, target.y, target.z);
    Hu3DModelRotSet(set->model[0], lbl_1_rodata_68,
        lbl_1_rodata_68, lbl_1_rodata_68);
    Hu3DModelScaleSet(set->model[0],
        lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_74);
    Hu3DModelLayerSet(set->model[0], 1);
    HuSprGrpDrawNoSet(set->spriteGroup, 0x40);
}

/* Main CFG preserves continue-menu, preview, unlock, and action returns. */
s16 fn_1_AA08(void)
{
    s16 navigation;
    s16 choice;
    s16 index;
    s32 action;
    char starText[16];
    s16 frameCounter = 0;

    for (;;) {
        aa08_region_R0(&frameCounter);
        aa08_region_R1(&frameCounter);
        navigation = aa08_region_R2(&frameCounter);
        choice = aa08_region_R3(navigation);

        if (choice == 0) {
            continue;
        }
        if (choice < 0) {
            aa08_win_secondary_close();
            return -1;
        }

        aa08_region_R4();
        index = aa08_table_index();

        if (lbl_1_data_22C[index].value[10] == 0) {
            choice = aa08_region_R5(index, starText);
            if (choice == 0) {
                if (aa08_region_R6(index) != 0) {
                    aa08_region_R7(index);
                    aa08_region_R8(index);
                    aa08_region_R9(index);
                } else {
                    aa08_region_R11();
                    continue;
                }
            } else {
                aa08_region_R11();
                continue;
            }
        }

        action = aa08_region_R10();
        if (action != 0) {
            aa08_win_secondary_close();
            return (s16)action;
        }
        aa08_region_R11();
    }
}

void fn_1_13598(HuVecF *position)
{
    const u8 *colorSource = &lbl_1_rodata_2B8;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    u8 color[4];
    s16 attempt;
    s16 slot;

    color[0] = colorSource[0];
    color[1] = colorSource[1];
    color[2] = colorSource[2];
    color[3] = colorSource[3];

    position->z += lbl_1_rodata_2B4;
    for (attempt = 0; attempt < 10; ++attempt) {
        float randomAccel = (float)(frandmod(500) + 500);

        particle = (HU3D_PARTICLE *)Hu3DData[lbl_1_bss_19EA].hookData;
        data = particle->data;
        for (slot = 0; slot < particle->maxCnt; ++slot, ++data) {
            if (data->time == 0) {
                data->time = 1;
                break;
            }
        }
        if (slot == particle->maxCnt) {
            continue;
        }

        data->color.r = color[0];
        data->color.g = color[1];
        data->color.b = color[2];
        data->color.a = 0;
        data->pos.x = position->x;
        data->pos.y = position->y;
        data->pos.z = position->z;
        data->scale = lbl_1_rodata_240;
        data->vel.x = lbl_1_rodata_240;
        data->vel.y = lbl_1_rodata_2BC;
        data->accel.x = randomAccel;
        data->accel.y = (float)color[3];
    }
}

short fn_1_EA98(s32 arg0, s32 arg1)
{
    OMOBJ *manager;
    OMOBJ *obj;
    MDBANK_ANIM_SET *set;
    s16 result;
    s16 i;

    (void)arg0;
    (void)arg1;
    HuPrcSleep(5);

    manager = lbl_1_bss_20;
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
            Hu3DModelAttrReset(manager->mdlId[i], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(manager->mdlId[i], HU3D_ATTR_DISPOFF);
        }
    }
    fn_1_6C6C();

    if (lbl_1_bss_2C == 1) {
        lbl_1_bss_1C->work[2] = 1;
        obj = lbl_1_bss_18;
        set = lbl_1_bss_179C;
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
        obj->work[0] = 0;
        obj->work[1] = 1;
        obj->objFunc = fn_1_5988;
    }

    lbl_1_bss_1988 = HuAudSStreamPlay(0x5C);
    HuAudFXPlay(0x585);
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

    WipeCreate(1, 0, 0x3C);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    if (lbl_1_bss_2C == 0) {
        fn_1_981C();
        fn_1_9DE8();
    }
    result = fn_1_AA08();
    switch (result) {
    case -1:
        return 0;
    case 10:
        return 1;
    case 20:
        return 2;
    default:
        return 0;
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

void fn_1_3640(void)
{
    OMOBJ *obj = lbl_1_bss_20;
    MDBANK_EVENT_RECORD *event;
    HuVecF modelPos;
    float verticalOffset;
    s16 i;

    for (i = 0; i < 20; i++) {
        event = &lbl_1_bss_300[i];
        verticalOffset = fn_1_11384(lbl_1_rodata_68, lbl_1_rodata_16C,
            event->time, event->duration);
        Hu3DModelPosGet(obj->mdlId[i], &modelPos);
        modelPos.y += verticalOffset;
        Hu3DModelPosSetV(obj->mdlId[i], &modelPos);
        event->time += lbl_1_rodata_74;
        if (event->time > event->duration) {
            event->time = lbl_1_rodata_68;
            event->duration = (float)(frandmod(0xB4) + 0xB4);
        }
    }
}

void fn_1_3468(OMOBJ *obj)
{
    OMOBJ *displayObj;
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(9699336);
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

void fn_1_6810(OMOBJ *obj)
{
    OMOBJ *manager;
    HuVecF modelPos;
    HuVecF screenPos;
    float verticalOffset;
    s16 i;

    lbl_1_bss_204.y = fn_1_111B0(lbl_1_bss_204.y,
        lbl_1_rodata_1E0, lbl_1_rodata_5C);

    Hu3DModelPosGet(obj->mdlId[5], &modelPos);
    verticalOffset = fn_1_11384(lbl_1_rodata_68, lbl_1_rodata_5C,
        (float)obj->work[1], lbl_1_rodata_160);
    modelPos.y = lbl_1_bss_204.y + verticalOffset;
    Hu3DModelPosSetV(obj->mdlId[5], &modelPos);

    Hu3D3Dto2D(&modelPos, 1, &screenPos);
    HuSprGrpPosSet(lbl_1_bss_194E[2], screenPos.x, screenPos.y);
    HuSprGrpScaleSet(lbl_1_bss_194E[2], lbl_1_rodata_188,
        lbl_1_rodata_74);

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

    manager = lbl_1_bss_20;
    for (i = 0; i < 20; i++) {
        if (i <= lbl_1_bss_1930[0] - 1) {
            Hu3DModelAttrReset(manager->mdlId[i], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(manager->mdlId[i], HU3D_ATTR_DISPOFF);
        }
    }
    fn_1_6290(lbl_1_bss_194E[2], 0, lbl_1_bss_1930[0]);

    obj->work[1]++;
    if (obj->work[1] > 180) {
        obj->work[1] = 0;
    }
}

void fn_1_3788(OMOBJ *obj)
{
    MDBANK_EVENT_RECORD *event;
    MDBANK_ITEM *item;
    HuVecF position;
    HuVecF rotation;
    float weight;
    float scale;
    float controlX;
    float controlY;
    s16 slot;
    s16 modelIndex;

    if (lbl_1_data_970 > 0) {
        for (slot = 20; slot < 80; slot++) {
            event = &lbl_1_bss_300[slot];
            --lbl_1_bss_34[0];
            if (lbl_1_bss_34[0] >= 0) {
                break;
            }
            if (event->active != 0) {
                continue;
            }

            HuAudFXPlay(1416);
            lbl_1_bss_34[0] = lbl_1_data_974;
            event->active = 1;
            event->time = lbl_1_rodata_68;
            event->duration = lbl_1_rodata_178;

            if (lbl_1_bss_1930[0] >= 20) {
                fn_1_1115C(&event->start, lbl_1_rodata_68,
                    (float)(frandmod(50) + 350), lbl_1_rodata_FC);
            } else {
                modelIndex = lbl_1_bss_1930[0] - 1;
                if (modelIndex < 0) {
                    modelIndex = 0;
                }
                Hu3DModelPosGet(obj->mdlId[modelIndex], &event->start);
            }

            controlY = (float)(frandmod(50) + 675);
            controlX = (float)(frandmod(300) - 150);
            fn_1_1115C(&event->control, controlX, controlY,
                lbl_1_rodata_120);
            fn_1_1115C(&event->end, lbl_1_rodata_68,
                lbl_1_rodata_17C, lbl_1_rodata_180);

            --lbl_1_bss_1930[0];
            if (lbl_1_bss_1930[0] < 0) {
                lbl_1_bss_1930[0] = 0;
            }
            --lbl_1_data_970;
            if (lbl_1_data_970 < 0) {
                lbl_1_data_970 = 0;
            }
            event->rotation = (float)(frandmod(5000) - 2500);

            Hu3DModelPosSet(obj->mdlId[slot], event->start.x,
                event->start.y, event->start.z);
            Hu3DModelRotSet(obj->mdlId[slot], lbl_1_rodata_68,
                lbl_1_rodata_68, lbl_1_rodata_68);
            Hu3DModelScaleSet(obj->mdlId[slot], lbl_1_rodata_16C,
                lbl_1_rodata_16C, lbl_1_rodata_16C);
            Hu3DModelLayerSet(obj->mdlId[slot], 1);
            Hu3DModelAttrReset(obj->mdlId[slot], HU3D_ATTR_DISPOFF);
            break;
        }
    }

    for (slot = 20; slot < 80; slot++) {
        event = &lbl_1_bss_300[slot];
        if (event->active == 0) {
            continue;
        }
        if (event->time > event->duration * lbl_1_rodata_184) {
            Hu3DModelLayerSet(obj->mdlId[slot], 2);
        }

        weight = fn_1_1116C(lbl_1_rodata_68, lbl_1_rodata_74,
            event->time, event->duration);
        fn_1_11678(&position, &event->start, &event->control,
            &event->end, weight);
        Hu3DModelPosSetV(obj->mdlId[slot], &position);

        Hu3DModelRotGet(obj->mdlId[slot], &rotation);
        rotation.y = fn_1_1116C(event->rotation, lbl_1_rodata_68,
            event->time, event->duration);
        Hu3DModelRotSetV(obj->mdlId[slot], &rotation);

        scale = fn_1_1116C(lbl_1_rodata_16C, lbl_1_rodata_188,
            event->time, event->duration);
        Hu3DModelScaleSet(obj->mdlId[slot], scale, scale, scale);

        event->time += lbl_1_rodata_74;
        if (event->time > event->duration + lbl_1_rodata_154) {
            Hu3DModelPosGet(obj->mdlId[slot], &position);
            fn_1_131C8(&position);
            HuAudFXPlay(1418);
            event->active = 0;
            Hu3DModelAttrSet(obj->mdlId[slot], HU3D_ATTR_DISPOFF);

            item = &lbl_1_data_22C[lbl_1_data_972];
            --item->value[3];
            if (item->value[3] < 0) {
                item->value[3] = 0;
            }
        }
    }

    item = &lbl_1_data_22C[lbl_1_data_972];
    if (item->value[3] <= 0) {
        obj->objFunc = NULL;
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

void fn_1_86C0(void)
{
    OMOBJ *obj;
    MDBANK_CAMERA_WORK *camera;
    MDBANK_SPRITE_CONFIG *config;
    HuVecF lightPos[2];
    HuVecF lightDir[2];
    GXColor lightColor;
    const u8 *lightColorBytes;
    HuVecF shadowPos;
    HuVecF shadowUp;
    HuVecF shadowTarget;
    s16 i;

    lbl_1_bss_0 = omInitObjMan(0x1B, 0x2000);
    omGameSysInit(lbl_1_bss_0);

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_58, lbl_1_rodata_5C,
        lbl_1_rodata_60, lbl_1_rodata_64);
    Hu3DCameraViewportSet(1, lbl_1_rodata_68, lbl_1_rodata_68,
        lbl_1_rodata_6C, lbl_1_rodata_70, lbl_1_rodata_68,
        lbl_1_rodata_74);

    camera = &lbl_1_bss_1998;
    memset(camera, 0, sizeof(*camera));
    camera->callback = fn_1_9380;
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
    camera->obj = omAddObjEx(lbl_1_bss_0, 0x100, 0, 0, -1,
        fn_1_4B8);

    lightPos[0] = lbl_1_rodata_8C[0];
    lightPos[1] = lbl_1_rodata_8C[1];
    lightDir[0] = lbl_1_rodata_A4[0];
    lightDir[1] = lbl_1_rodata_A4[1];
    lightColorBytes = &lbl_1_rodata_BC;
    lightColor.r = lightColorBytes[0];
    lightColor.g = lightColorBytes[1];
    lightColor.b = lightColorBytes[2];
    lightColor.a = lightColorBytes[3];

    lbl_1_bss_1994[0] = Hu3DGLightCreateV(&lightPos[0],
        &lightDir[0], &lightColor);
    Hu3DGLightInfinitytSet(lbl_1_bss_1994[0]);
    Hu3DGLightStaticSet(lbl_1_bss_1994[0], TRUE);
    lbl_1_bss_1994[1] = Hu3DGLightCreateV(&lightPos[1],
        &lightDir[1], &lightColor);
    Hu3DGLightInfinitytSet(lbl_1_bss_1994[1]);
    Hu3DGLightStaticSet(lbl_1_bss_1994[1], TRUE);

    HuWinInit(1);
    lbl_1_bss_198C[0] = HuWinExCreateFrame(
        lbl_1_rodata_E4, lbl_1_rodata_E8, 0x220, 0x2A, -1, 0);
    HuWinDispOff(lbl_1_bss_198C[0]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[0], lbl_1_rodata_68);
    lbl_1_bss_198C[1] = HuWinExCreateFrame(
        lbl_1_rodata_E4, lbl_1_rodata_EC, 0x220, 0x44, -1, 5);
    HuWinDispOff(lbl_1_bss_198C[1]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[1], lbl_1_rodata_F0);
    lbl_1_bss_198C[2] = HuWinExCreateFrame(
        lbl_1_rodata_E4, lbl_1_rodata_EC, 0x220, 0x44, -1, 3);
    HuWinDispOff(lbl_1_bss_198C[2]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[2], lbl_1_rodata_F0);
    lbl_1_bss_198C[3] = HuWinExCreateFrame(
        lbl_1_rodata_E4, lbl_1_rodata_EC, 0x220, 0x44, -1, 4);
    HuWinDispOff(lbl_1_bss_198C[3]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[3], lbl_1_rodata_F0);

    for (i = 0; i < 4; i++) {
        winData[lbl_1_bss_198C[i]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_198C[i], (HUWIN_CALLBACK)fn_1_0);
    }

    shadowPos = lbl_1_rodata_C0;
    shadowUp = lbl_1_rodata_CC;
    shadowTarget = lbl_1_rodata_D8;
    Hu3DShadowCreate(lbl_1_rodata_58, lbl_1_rodata_5C,
        lbl_1_rodata_60);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);

    config = lbl_1_data_3C;
    for (i = 0; i < 12; i++) {
        lbl_1_bss_1958[i] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_4[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 4; i++) {
        lbl_1_bss_194E[i] = HuSprGrpCreate(lbl_1_data_34[i]);
    }
    for (i = 0; i < 11; i++, config++) {
        lbl_1_bss_1938[i] = HuSprCreate(
            lbl_1_bss_1958[config->anim],
            (s16)(config->priority + 6000), config->bank);
        HuSprGrpMemberSet(lbl_1_bss_194E[config->group],
            config->member, lbl_1_bss_1938[i]);
        HuSprPosSet(lbl_1_bss_194E[config->group], config->member,
            config->x, config->y);
        HuSprScaleSet(lbl_1_bss_194E[config->group], config->member,
            config->scaleX, config->scaleY);
        HuSprZRotSet(lbl_1_bss_194E[config->group], config->member,
            config->rotation);
    }
    for (i = 0; i < 4; i++) {
        fn_1_11880(lbl_1_bss_194E[i], HUSPR_ATTR_DISPOFF);
    }
    HuSprExecLayerCameraSet(0x40, 1, 2);
    HuSprExecLayerCameraSet(0x41, 1, 4);

    lbl_1_bss_1930[0] = (s16)GWBankStarGet();
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

    fn_1_12EA8();
    lbl_1_bss_C = omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_1FD8);
    lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_2468);
    obj = omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_28B4);
    lbl_1_bss_4 = obj;
    lbl_1_bss_8 = omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_2B34);
    lbl_1_bss_14 = omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_3468);
    lbl_1_bss_18 = omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_5D64);
    lbl_1_bss_1C = omAddObjEx(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_7594);
    lbl_1_bss_20 = omAddObjEx(lbl_1_bss_0, 0x1000, 0x60, 0x1,
        -1, fn_1_3E30);
    lbl_1_bss_24[0] = omAddObjEx(lbl_1_bss_0, 0x1000, 0x0A, 0x0A,
        -1, fn_1_7F2C);
    HuPrcChildCreate(fn_1_83CC, 0x3000, 0x3000, 0,
        lbl_1_bss_0);
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

void fn_1_7F2C(OMOBJ *obj)
{
    OMOBJ *callbackObj;
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 6; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            0x9400000B + i, HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 4);
        Hu3DModelPosSet(obj->mdlId[i], lbl_1_rodata_68,
            lbl_1_rodata_1E0, lbl_1_rodata_FC);
        Hu3DModelRotSet(obj->mdlId[i], lbl_1_rodata_100,
            lbl_1_rodata_68, lbl_1_rodata_68);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_68, lbl_1_rodata_68, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        lbl_1_bss_40[i].time = 90.0f - (float)(i * 30);
        lbl_1_bss_40[i].duration = lbl_1_rodata_134;
    }
    for (i = 6; i < 12; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            0x94000012, HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 4);
        Hu3DModelPosSet(obj->mdlId[i], lbl_1_rodata_68,
            lbl_1_rodata_1E0, lbl_1_rodata_FC);
        Hu3DModelRotSet(obj->mdlId[i], lbl_1_rodata_100,
            lbl_1_rodata_68, lbl_1_rodata_68);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_68, lbl_1_rodata_68, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    callbackObj = lbl_1_bss_24[0];
    callbackObj->work[3] = 1;
    fn_1_13764(0);
    obj->objFunc = fn_1_7A84;
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

void fn_1_119C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;
    s16 spawnCount = 0;
    float angle;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 0 && spawnCount == 0) {
            spawnCount = 1;
            data->time = 1;
            data->parManId = 0;
            data->vel.x = (float)(frandmod(3) + 1);
            data->vel.y = (float)(frandmod(3) + 1);
            data->vel.z = (float)(frandmod(3) + 1);
            data->accel.x = lbl_1_rodata_240;
            data->accel.y = (float)(frandmod(720) + 360);
            data->accel.z = (float)(frandmod(80) + 100);
            data->accel.x = (float)frandmod(360);
            data->speedDecay = lbl_1_rodata_240;
            data->colorIdx = (float)(frandmod(180) + 60);
            data->speedDecay = (float)frandmod(60);
            data->scale = lbl_1_rodata_278;
            data->color.r = 255;
            data->color.g = 255;
            data->color.b = 255;
        } else if (data->time == 1) {
            if (data->accel.x <= lbl_1_rodata_240) {
                angle = lbl_1_rodata_240;
            } else if (data->accel.x >= data->accel.y) {
                angle = lbl_1_rodata_26C;
            } else {
                angle = lbl_1_rodata_26C
                    * (data->accel.x / data->accel.y);
            }
            data->pos.x = data->accel.z
                * sin(lbl_1_rodata_250
                    * (angle * data->vel.x) / lbl_1_rodata_260);
            data->pos.y = data->accel.z
                * cos(lbl_1_rodata_250
                    * (angle * data->vel.y) / lbl_1_rodata_260);
            data->pos.z = data->accel.z
                * sin(lbl_1_rodata_250
                    * (angle * data->vel.z) / lbl_1_rodata_260);
            data->scale = (float)(frandmod(30) + 10);
            if (rand8() % 10 == 0) {
                data->zRot = (float)frandmod(360) * lbl_1_rodata_27C;
            }
            if (data->speedDecay <= lbl_1_rodata_240
                || data->speedDecay >= data->colorIdx) {
                data->color.a = 0;
            } else {
                data->color.a = (u8)(lbl_1_rodata_288
                    * sin(lbl_1_rodata_250
                        * ((lbl_1_rodata_270 / data->colorIdx)
                            * data->speedDecay)
                        / lbl_1_rodata_260)
                    + lbl_1_rodata_280);
            }
            data->accel.x += lbl_1_rodata_268;
            if (data->accel.x > data->accel.y) {
                data->vel.x = (float)(frandmod(4) + 1);
                data->vel.y = (float)(frandmod(4) + 1);
                data->vel.z = (float)(frandmod(4) + 1);
                data->accel.x = lbl_1_rodata_240;
                data->accel.y = (float)(frandmod(720) + 360);
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
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

void fn_1_12230(HuVecF *pos, GXColor *color, float velocityY, float accelY)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_19EA];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 slot;

    for (slot = 0, data = particle->data;
         slot < particle->maxCnt; slot++, data++) {
        if (data->time == 0) {
            data->time = 1;
            break;
        }
    }
    if (slot < particle->maxCnt) {
        data->color.r = color->r;
        data->color.g = color->g;
        data->color.b = color->b;
        data->color.a = 0;
        data->pos = *pos;
        data->scale = lbl_1_rodata_240;
        data->vel.x = lbl_1_rodata_240;
        data->vel.y = velocityY;
        data->accel.y = accelY;
        data->colorIdx = (float)color->a;
    }
}

void fn_1_12354(HuVecF *pos, GXColor *color, float velocityY, float velocityZ)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_19EA];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 slot;

    for (slot = 0, data = particle->data;
         slot < particle->maxCnt; slot++, data++) {
        if (data->time == 0) {
            data->time = 2;
            break;
        }
    }
    if (slot < particle->maxCnt) {
        data->color = *color;
        data->pos = *pos;
        data->vel.x = lbl_1_rodata_240;
        data->vel.y = velocityY;
        data->vel.z = velocityZ;
        data->accel = *pos;
        data->speedDecay = (float)(frandmod(100) - 50);
        data->colorIdx = (float)(frandmod(100) - 50);
        data->scaleBase = lbl_1_rodata_240;
        PSVECNormalize((const Vec *)&data->speedDecay,
            (Vec *)&data->speedDecay);
    }
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

void fn_1_12B60(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 0) {
            data->pos.x = lbl_1_rodata_240;
            data->pos.y = lbl_1_rodata_240;
            data->pos.z = lbl_1_rodata_240;
            if (rand8() % 5 == 0) {
                data->color.r = 0x30;
                data->color.g = 0x30;
                data->color.b = 0x30;
                data->color.a = (u8)(frandmod(24) + 0x30);
                data->scale = (float)(frandmod(100) + 600);
            }
        } else if (data->time == 1) {
            data->pos.x = lbl_1_rodata_240;
            data->pos.y = lbl_1_rodata_240;
            data->pos.z = lbl_1_rodata_240;
            if (rand8() % 5 == 0) {
                data->color.r = 0x40;
                data->color.g = 0x40;
                data->color.b = 0x40;
                data->color.a = (u8)(frandmod(24) + 0x5C);
                data->scale = (float)(frandmod(100) + 600);
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
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

void fn_1_131C8(HuVecF *pos)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    GXColor baseColor = lbl_1_rodata_2AC;
    GXColor brightColor = lbl_1_rodata_2B0;
    float randomAccel;
    float randomVelocity;
    float randomDirection;
    s16 slot;

    pos->z += lbl_1_rodata_2B4;
    randomAccel = (float)(frandmod(300) + 300);
    randomVelocity = (float)(frandmod(6) + 6);

    model = &Hu3DData[lbl_1_bss_19EA];
    particle = model->hookData;
    for (slot = 0, data = particle->data;
         slot < particle->maxCnt; slot++, data++) {
        if (data->time == 0) {
            data->time = 1;
            break;
        }
    }
    if (slot < particle->maxCnt) {
        data->color.r = baseColor.r;
        data->color.g = baseColor.g;
        data->color.b = baseColor.b;
        data->color.a = baseColor.a;
        data->pos = *pos;
        data->scale = lbl_1_rodata_240;
        data->vel.x = lbl_1_rodata_240;
        data->vel.y = randomVelocity;
        data->accel.y = randomAccel;
        data->colorIdx = (float)baseColor.a;
    }

    randomDirection = (float)(frandmod(70) + 80);
    model = &Hu3DData[lbl_1_bss_19EA];
    particle = model->hookData;
    for (slot = 0, data = particle->data;
         slot < particle->maxCnt; slot++, data++) {
        if (data->time == 0) {
            data->time = 2;
            break;
        }
    }
    if (slot < particle->maxCnt) {
        data->color.r = brightColor.r;
        data->color.g = brightColor.g;
        data->color.b = brightColor.b;
        data->color.a = brightColor.a;
        data->pos = *pos;
        data->vel.x = lbl_1_rodata_240;
        data->vel.y = lbl_1_rodata_278;
        data->vel.z = randomDirection;
        data->accel.x = pos->x;
        data->accel.y = pos->y;
        data->accel.z = pos->z;
        data->speedDecay = (float)(frandmod(100) - 50);
        data->colorIdx = (float)(frandmod(100) - 50);
        data->scaleBase = lbl_1_rodata_240;
        PSVECNormalize((const Vec *)&data->speedDecay,
            (Vec *)&data->speedDecay);
    }
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

float fn_1_11384(float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_240) {
        return start;
    }
    if (time >= duration) {
        return start;
    }
    return start + (end - start)
        * sin(lbl_1_rodata_250
            * ((lbl_1_rodata_26C / duration) * time)
            / lbl_1_rodata_260);
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
    lbl_1_bss_1988 = HuAudSStreamPlay(92);
    HuAudFXPlay(1413);
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
    fn_1_12F8(2, 1572873, 1);
    return fn_1_1200(2);
}

void fn_1_A750(s16 index)
{
    OMOBJ *obj;
    HU3D_MODEL *model;
    MDBANK_ITEM *item;

    HuAudFXPlay(1414);
    obj = lbl_1_bss_1C;
    model = &Hu3DData[obj->mdlId[0]];
    Hu3DMotionShapeSet(obj->mdlId[0], obj->mtnId[0]);
    model->motShapeWork.speed = lbl_1_rodata_154;
    HuPrcSleep(30);

    obj = lbl_1_bss_24[0];
    obj->work[0] = 0;
    fn_1_13764(1);
    HuAudFXPlay(1417);

    item = &lbl_1_data_22C[index];
    lbl_1_data_972 = index;
    lbl_1_data_970 = item->value[3];
    lbl_1_data_974 = 5;
    obj = lbl_1_bss_20;
    obj->objFunc = fn_1_3788;
    while (obj->objFunc != NULL) {
        HuPrcVSleep();
    }

    if (lbl_1_bss_1930[0] > 80) {
        lbl_1_data_0 = HuAudFXPlay(1423);
    } else if (lbl_1_bss_1930[0] > 60) {
        lbl_1_data_0 = HuAudFXPlay(1422);
    } else if (lbl_1_bss_1930[0] > 40) {
        lbl_1_data_0 = HuAudFXPlay(1421);
    } else if (lbl_1_bss_1930[0] > 20) {
        lbl_1_data_0 = HuAudFXPlay(1420);
    } else {
        HuAudFXStop(lbl_1_data_0);
    }

    obj = lbl_1_bss_24[0];
    obj->work[0] = 1;
    fn_1_13764(0);

    obj = lbl_1_bss_1C;
    HuPrcSleep(30);

    model = &Hu3DData[obj->mdlId[0]];
    Hu3DMotionShapeSet(obj->mdlId[0], obj->mtnId[0]);
    model->motShapeWork.speed = lbl_1_rodata_1E4;
    Hu3DMotionShapeTimeSet(obj->mdlId[0],
        Hu3DMotionShapeMaxTimeGet(obj->mdlId[0]));
    HuAudFXPlay(1413);
}

void fn_1_124F4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;
    float q;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 1) {
            if (data->vel.x <= lbl_1_rodata_240) {
                q = lbl_1_rodata_240;
            } else if (data->vel.x >= data->vel.y) {
                q = data->accel.x;
            } else {
                q = data->accel.x
                    * sin(lbl_1_rodata_250
                        * ((lbl_1_rodata_258 / data->vel.y)
                            * data->vel.x)
                        / lbl_1_rodata_260)
                    + lbl_1_rodata_280;
            }
            data->scale = q;
            if (data->vel.x <= lbl_1_rodata_240
                || data->vel.x >= data->vel.y) {
                data->color.a = 0;
            } else {
                data->color.a = (u8)(data->accel.y
                    * sin(lbl_1_rodata_250
                        * ((lbl_1_rodata_270 / data->vel.y)
                            * data->vel.x)
                        / lbl_1_rodata_260)
                    + lbl_1_rodata_280);
            }
            data->vel.x += lbl_1_rodata_268;
            if (data->vel.x > data->vel.y) {
                data->scale = lbl_1_rodata_240;
                data->time = 0;
            }
        } else if (data->time == 2) {
            if (data->vel.x <= lbl_1_rodata_240) {
                q = lbl_1_rodata_240;
            } else if (data->vel.x >= data->vel.y) {
                q = data->vel.z;
            } else {
                q = data->vel.z
                    * sin(lbl_1_rodata_250
                        * ((lbl_1_rodata_258 / data->vel.y)
                            * data->vel.x)
                        / lbl_1_rodata_260)
                    + lbl_1_rodata_280;
            }
            data->scale = (float)(frandmod(6) + 1);
            data->pos.x = data->accel.x + data->speedDecay * q;
            data->pos.y = data->accel.y + data->colorIdx * q;
            data->pos.z = data->accel.z + data->scaleBase * q;
            data->vel.x += lbl_1_rodata_268;
            if (data->vel.x > data->vel.y) {
                data->scale = lbl_1_rodata_240;
                data->time = 0;
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
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
    camera->obj = omAddObjEx(lbl_1_bss_0, 256, 0, 0, -1, fn_1_4B8);
}

void fn_1_2468(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(9699396);
    obj->mdlId[1] = Hu3DModelCreateData(9699397);
    for (i = 0; i < 3; i++) {
        obj->mtnId[i] = Hu3DJointMotionData(obj->mdlId[0], 9699398 + i);
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

void fn_1_2CD8(s16 index, HuVecF *worldPos, float offsetX, float offsetY)
{
    HuVecF screenPos = lbl_1_rodata_148;

    if (worldPos) {
        Hu3D3Dto2D(worldPos, 1, &screenPos);
    }
    HuSprPosSet(lbl_1_bss_194E[1], index,
        screenPos.x + offsetX, screenPos.y + offsetY);
}

void fn_1_2D98(s16 index, HuVecF *worldPos, float offsetX, float offsetY)
{
    OMOBJ *obj = lbl_1_bss_14;
    HuVecF screenPos;

    if (obj->work[index] != 1) {
        lbl_1_bss_1700[index] = lbl_1_rodata_154;
        screenPos = lbl_1_rodata_148;
        if (worldPos) {
            Hu3D3Dto2D(worldPos, 1, &screenPos);
        }
        HuSprPosSet(lbl_1_bss_194E[1], index,
            screenPos.x + offsetX, screenPos.y + offsetY);
        HuSprScaleSet(lbl_1_bss_194E[1], index,
            lbl_1_rodata_154, lbl_1_rodata_154);
        HuSprAttrReset(lbl_1_bss_194E[1], index, HUSPR_ATTR_DISPOFF);
        obj->work[index] = 1;
        lbl_1_data_964[index] = 1;
    }
}

void fn_1_48CC(s16 index)
{
    MDBANK_ANIM_SET *set = &lbl_1_bss_179C[index];
    OMOBJ *obj = lbl_1_bss_18;
    HuVecF pos;
    HuVecF rot;
    HuVecF value;
    s16 i = 0;
    s16 end = 30;

    Hu3DModelPosGet(set->model[0], &pos);
    Hu3DModelRotGet(set->model[0], &rot);
    Hu3DModelLayerSet(set->model[0], 3);
    HuSprGrpDrawNoSet(set->spriteGroup, 65);
    for (; i <= end; i++) {
        HuPrcVSleep();
        value.x = fn_1_11458(pos.x, lbl_1_rodata_68, i, end);
        value.y = fn_1_11458(pos.y, lbl_1_rodata_1B8, i, end);
        value.z = fn_1_11458(pos.z, lbl_1_rodata_120, i, end);
        Hu3DModelPosSetV(set->model[0], &value);
        value.x = lbl_1_rodata_68;
        value.y = fn_1_11458(rot.y, lbl_1_rodata_1BC, i, end);
        value.z = lbl_1_rodata_68;
        Hu3DModelRotSetV(set->model[0], &value);
        value.x = fn_1_11458(lbl_1_rodata_74, lbl_1_rodata_1C0, i, end);
        value.y = value.x;
        value.z = lbl_1_rodata_74;
        Hu3DModelScaleSetV(set->model[0], &value);
    }
    Hu3DModelPosSet(set->model[0], lbl_1_rodata_68, lbl_1_rodata_1B8,
        lbl_1_rodata_120);
    Hu3DModelRotSet(set->model[0], lbl_1_rodata_68, lbl_1_rodata_68,
        lbl_1_rodata_68);
    Hu3DModelScaleSet(set->model[0], lbl_1_rodata_1C0, lbl_1_rodata_1C0,
        lbl_1_rodata_74);
}

void fn_1_4C9C(void)
{
    OMOBJ *obj = lbl_1_bss_18;
    MDBANK_ANIM_SET *set;
    HuVecF rot;
    s16 i = 0;

    obj->work[1] = 0;
    set = &lbl_1_bss_179C[obj->work[2]];
    Hu3DModelRotGet(set->model[0], &rot);
    if (rot.y > lbl_1_rodata_160) {
        rot.y -= lbl_1_rodata_140;
    }
    for (; i < 10; i++) {
        HuPrcVSleep();
        Hu3DModelRotSet(set->model[0], lbl_1_rodata_68,
            fn_1_11458(rot.y, lbl_1_rodata_68, i, lbl_1_rodata_5C),
            lbl_1_rodata_68);
    }
    Hu3DModelRotSet(set->model[0], lbl_1_rodata_68, lbl_1_rodata_68,
        lbl_1_rodata_68);
}

void fn_1_4DFC(s16 index)
{
    MDBANK_ANIM_SET *set = &lbl_1_bss_179C[index];
    OMOBJ *obj = lbl_1_bss_18;
    HuVecF pos;
    HuVecF rot;
    HuVecF target;
    HuVecF value;
    s16 i = 0;
    s16 end = 30;

    Hu3DModelPosGet(set->model[0], &pos);
    Hu3DModelRotGet(set->model[0], &rot);
    fn_1_1115C(&target, (index % 5) * 100 - 200,
        235 - (index / 5) * 100, lbl_1_rodata_11C);
    obj->work[1] = 0;
    for (; i <= end; i++) {
        HuPrcVSleep();
        value.x = fn_1_11458(pos.x, target.x, i, end);
        value.y = fn_1_11458(pos.y, target.y, i, end);
        value.z = fn_1_11458(pos.z, target.z, i, end);
        Hu3DModelPosSetV(set->model[0], &value);
        value.x = lbl_1_rodata_68;
        value.y = fn_1_11458(rot.y, lbl_1_rodata_1BC, i, end);
        value.z = lbl_1_rodata_68;
        Hu3DModelRotSetV(set->model[0], &value);
        value.x = fn_1_11458(lbl_1_rodata_1C0, lbl_1_rodata_74, i, end);
        value.y = value.x;
        value.z = lbl_1_rodata_74;
        Hu3DModelScaleSetV(set->model[0], &value);
    }
    Hu3DModelPosSet(set->model[0], target.x, target.y, target.z);
    Hu3DModelRotSet(set->model[0], lbl_1_rodata_68, lbl_1_rodata_68,
        lbl_1_rodata_68);
    Hu3DModelScaleSet(set->model[0], lbl_1_rodata_74, lbl_1_rodata_74,
        lbl_1_rodata_74);
    Hu3DModelLayerSet(set->model[0], 1);
    HuSprGrpDrawNoSet(set->spriteGroup, 64);
}

void fn_1_520C(s16 skipIndex)
{
    MDBANK_ANIM_SET *set = lbl_1_bss_179C;
    float scale;
    s16 i;
    s16 j = 0;

    HuAudFXPlay(53);
    for (; j < 12; j++) {
        HuPrcVSleep();
        scale = fn_1_11064(lbl_1_rodata_74, lbl_1_rodata_68, j,
            lbl_1_rodata_5C);
        for (i = 0; i < 10; i++) {
            set = &lbl_1_bss_179C[i];
            if (i != skipIndex) {
                set->state[3] = 1;
                Hu3DModelScaleSet(set->model[0], scale, scale, scale);
                Hu3DModelScaleSet(set->model[1], scale, scale, scale);
                Hu3DModelRotSet(set->model[0], lbl_1_rodata_68,
                    lbl_1_rodata_1C4 * scale, lbl_1_rodata_68);
                Hu3DModelRotSet(set->model[1], lbl_1_rodata_68,
                    lbl_1_rodata_1C4 * scale, lbl_1_rodata_68);
                fn_1_11880(set->spriteGroup, HUSPR_ATTR_DISPOFF);
            }
        }
    }
}

void fn_1_53AC(s16 skipIndex)
{
    MDBANK_ANIM_SET *set = lbl_1_bss_179C;
    OMOBJ *obj = lbl_1_bss_18;
    float scale;
    s16 i;
    s16 j = 0;

    HuAudFXPlay(52);
    for (; j < 12; j++) {
        HuPrcVSleep();
        scale = fn_1_11458(lbl_1_rodata_68, lbl_1_rodata_74, j,
            lbl_1_rodata_5C);
        for (i = 0; i < 10; i++) {
            set = &lbl_1_bss_179C[i];
            if (i != skipIndex) {
                set->state[3] = 0;
                Hu3DModelScaleSet(set->model[0], scale, scale, scale);
                Hu3DModelScaleSet(set->model[1], scale, scale, scale);
                Hu3DModelRotSet(set->model[0], lbl_1_rodata_68,
                    lbl_1_rodata_1C4 * scale, lbl_1_rodata_68);
                Hu3DModelRotSet(set->model[1], lbl_1_rodata_68,
                    lbl_1_rodata_1C4 * scale, lbl_1_rodata_68);
            }
        }
    }
}

void fn_1_5988(OMOBJ *obj)
{
    MDBANK_ANIM_SET *set = lbl_1_bss_179C;
    float scale;
    s16 i;

    scale = fn_1_11458(lbl_1_rodata_68, lbl_1_rodata_74,
        (float)obj->work[0], (float)obj->work[1]);
    for (i = 0; i < 10; i++, set++) {
        Hu3DModelAttrReset(set->model[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(set->model[1], HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(set->model[0], scale, scale, scale);
        Hu3DModelScaleSet(set->model[1], scale, scale, scale);
        Hu3DModelRotSet(set->model[0], lbl_1_rodata_68,
            lbl_1_rodata_1C4 * scale, lbl_1_rodata_68);
        Hu3DModelRotSet(set->model[1], lbl_1_rodata_68,
            lbl_1_rodata_1C4 * scale, lbl_1_rodata_68);
    }
    obj->work[0]++;
    if (obj->work[0] <= obj->work[1]) {
        return;
    }
    set = lbl_1_bss_179C;
    for (i = 0; i < 10; i++, set++) {
        Hu3DModelRotSet(set->model[0], lbl_1_rodata_68,
            lbl_1_rodata_68, lbl_1_rodata_68);
        Hu3DModelRotSet(set->model[1], lbl_1_rodata_68,
            lbl_1_rodata_68, lbl_1_rodata_68);
        Hu3DModelScaleSet(set->model[0], lbl_1_rodata_74,
            lbl_1_rodata_74, lbl_1_rodata_74);
        Hu3DModelScaleSet(set->model[1], lbl_1_rodata_74,
            lbl_1_rodata_74, lbl_1_rodata_74);
        set->state[3] = 0;
    }
    obj->work[0] = 0;
    obj->work[1] = 0;
    obj->work[2] = 0;
    obj->work[3] = 0;
    obj->objFunc = fn_1_58B8;
}

float fn_1_11064(float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_240) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + (end - start)
        * (lbl_1_rodata_248
            - cos(lbl_1_rodata_250
                * ((lbl_1_rodata_258 / duration) * time)
                / lbl_1_rodata_260));
}

float fn_1_11458(float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_240) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + (end - start)
        * sin(lbl_1_rodata_250
            * ((lbl_1_rodata_258 / duration) * time)
            / lbl_1_rodata_260);
}

float fn_1_11540(float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_240) {
        return start;
    }
    if (time >= duration) {
        return start;
    }
    return start + (end - start)
        * sin(lbl_1_rodata_250
            * ((lbl_1_rodata_270 / duration) * time)
            / lbl_1_rodata_260);
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
__declspec(section ".rodata") const float lbl_1_rodata_16C = 0.25f;
__declspec(section ".rodata") const float lbl_1_rodata_168 = 0.3f;
__declspec(section ".rodata") const float lbl_1_rodata_1E4 = -2.0f;
__declspec(section ".rodata") const float lbl_1_rodata_1B8 = 210.0f;
__declspec(section ".rodata") const float lbl_1_rodata_1BC = 720.0f;
__declspec(section ".rodata") const float lbl_1_rodata_1C0 = 1.75f;
__declspec(section ".rodata") const float lbl_1_rodata_1C4 = 1800.0f;
__declspec(section ".rodata") const HuVecF lbl_1_rodata_230 = {
    0.0f, 185.0f, 100.0f,
};
__declspec(section ".rodata") const float lbl_1_rodata_23C = -90.0f;
__declspec(section ".rodata") const float lbl_1_rodata_240 = 0.0f;
__declspec(section ".rodata") const double lbl_1_rodata_248 = 1.0;
__declspec(section ".rodata") const double lbl_1_rodata_250 = 3.141592653589793;
__declspec(section ".rodata") const float lbl_1_rodata_258 = 90.0f;
__declspec(section ".rodata") const double lbl_1_rodata_260 = 180.0;
__declspec(section ".rodata") const float lbl_1_rodata_268 = 1.0f;
__declspec(section ".rodata") const float lbl_1_rodata_26C = 360.0f;
__declspec(section ".rodata") const float lbl_1_rodata_270 = 180.0f;
__declspec(section ".rodata") const float lbl_1_rodata_274 = 2.0f;
__declspec(section ".rodata") const float lbl_1_rodata_278 = 10.0f;
__declspec(section ".rodata") const float lbl_1_rodata_27C = 0.01745329238474369f;
__declspec(section ".rodata") const double lbl_1_rodata_280 = 0.0;
__declspec(section ".rodata") const double lbl_1_rodata_288 = 255.0;
__declspec(section ".rodata") const double lbl_1_rodata_290 = 4503601774854144.0;
__declspec(section ".rodata") const float lbl_1_rodata_298 = 375.0f;
__declspec(section ".rodata") const float lbl_1_rodata_29C = -100.0f;
__declspec(section ".rodata") const double lbl_1_rodata_2A0 =
    4503599627370496.0;
__declspec(section ".rodata") const float lbl_1_rodata_2A8 = 100.0f;
__declspec(section ".rodata") const float lbl_1_rodata_2B4 = 20.0f;
__declspec(section ".rodata") const u8 lbl_1_rodata_2B8 = 0x40;
#pragma force_active on
__declspec(section ".rodata") static const u8 gap_03_000002B9_rodata[3] = {
    0x40, 0x40, 0x80,
};
#pragma force_active reset
__declspec(section ".rodata") const float lbl_1_rodata_2BC = 60.0f;

/* Callback-chain conversion, model, and sprite assets. */
__declspec(section ".rodata") const float lbl_1_rodata_5C = 10.0f;
__declspec(section ".rodata") const float lbl_1_rodata_11C = 100.0f;
__declspec(section ".rodata") const double lbl_1_rodata_128 =
    4503599627370496.0;
__declspec(section ".rodata") const double lbl_1_rodata_170 =
    4503601774854144.0;
__declspec(section ".rodata") const float lbl_1_rodata_188 = 0.8f;
__declspec(section ".rodata") const float lbl_1_rodata_1E0 = 375.0f;
__declspec(section ".rodata") const HuVecF lbl_1_rodata_200 = {
    0.0f, 375.0f, -100.0f,
};
__declspec(section ".rodata") const MDBANK_MODEL_DATA lbl_1_rodata_20C = {
    {0x0094000A, 0x0094000E, 0x0094000E, 0x0094000E, 0x0094000E},
};
__declspec(section ".rodata") const float lbl_1_rodata_220 = 200.0f;
__declspec(section ".rodata") const float lbl_1_rodata_224 = 1000.0f;
__declspec(section ".rodata") const float lbl_1_rodata_228 = 1.5f;

__declspec(section ".data") char lbl_1_data_9DC[12] = "bank_star04";
__declspec(section ".data") const char lbl_1_data_A1C[4] = {
    '%', 'd', '\0', '\0',
};

MDBANK_POST_CALLBACK_WORK lbl_1_bss_40[6];
MDBANK_EVENT_RECORD lbl_1_bss_300[80];

__declspec(section ".data") const s32 lbl_1_data_19C[36] = {
    0x0094001C, 0x0094001E, 0x0094001D, 0x00940036,
    0x0094001F, 0x00940020, 0x00940021, 0x00940022,
    0x00940023, 0x00940024, 0x00940029, 0x0094002A,
    0x0094002B, 0x0094002C, 0x0094002D, 0x0094002E,
    0x0094002F, 0x00940030, 0x00940031, 0x00940032,
    0x00940033, 0x00940025, 0x00940026, 0x00940027,
    0x00940028, 0x00940035, 0x00940034, 0x0094003B,
    0x0094003C, 0x0094003D, 0x0094003E, 0x00940037,
    0x0094003F, 0x00940038, 0x00940039, 0x0094003A,
};

__declspec(section ".rodata") const HuVecF lbl_1_rodata_8C[2] = {
    {0.0f, 1.0f, 1.0f},
    {-1.0f, 1.0f, -1.0f},
};
__declspec(section ".rodata") const HuVecF lbl_1_rodata_A4[2] = {
    {0.0f, -1.0f, -1.0f},
    {1.0f, -1.0f, -1.0f},
};
__declspec(section ".rodata") const u8 lbl_1_rodata_BC = 0x80;
#pragma force_active on
__declspec(section ".rodata") static const u8 gap_03_000000BD_rodata[3] = {
    0x80, 0xFF, 0x20,
};
#pragma force_active reset
__declspec(section ".rodata") const HuVecF lbl_1_rodata_C0 = {
    0.0f, 3000.0f, 600.0f,
};
__declspec(section ".rodata") const HuVecF lbl_1_rodata_CC = {
    0.0f, 1.0f, 0.0f,
};
__declspec(section ".rodata") const HuVecF lbl_1_rodata_D8 = {
    0.0f, 0.0f, 0.0f,
};
__declspec(section ".rodata") const float lbl_1_rodata_18C = 0.4f;
__declspec(section ".rodata") const char *const lbl_1_rodata_1C8[6] = {
    lbl_1_data_99C,
    lbl_1_data_9A4,
    lbl_1_data_9AC,
    lbl_1_data_9B5,
    lbl_1_data_9BD,
    lbl_1_data_9C5,
};
__declspec(section ".rodata") const float lbl_1_rodata_22C = -60.0f;
