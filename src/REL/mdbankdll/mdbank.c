/* The Star Bank: its menus and windows, the item shop, purchases and the bank's save data. */
#include <math.h>
#include "dolphin.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/process.h"
#include "game/saveload.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"
#include "REL/mdbankdll.h"

typedef struct MdbankAnimSet {
    s16 model[2];
    s16 anim[6];
    s16 spriteGroup;
    s16 state[4];
} MDBANK_ANIM_SET;

typedef union MdbankItem {
    s16 value[16];
    struct {
        s16 value[12];
        s32 nameMessage;
        s32 message;
    } choice;
} MDBANK_ITEM;

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
    float unk30;
    float unk34;
    float unk38;
    float unk3C;
} MDBANK_MOVE_WORK;

typedef struct MdbankEventRecord {
    s16 active;
    float time;
    float duration;
    HuVecF start;
    HuVecF control;
    HuVecF end;
    float rotation;
    float value34;
    float value38;
    float value3C;
} MDBANK_EVENT_RECORD;

typedef struct MdbankPostCallbackWork {
    /* Each record is 0x40 bytes; the contents outside time and duration are unknown. */
    u8 unk_00[4]; /* No code names this field. */
    float time;
    float duration;
    u8 unk_0C[0x34]; /* No code names this field. */
} MDBANK_POST_CALLBACK_WORK;

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
extern s16 lbl_1_data_964[6];
extern s16 lbl_1_data_932[3];
extern s32 lbl_1_data_938[3];
extern s16 lbl_1_data_970;
extern s16 lbl_1_data_972;
extern s16 lbl_1_data_974;
extern char lbl_1_data_956[7];
extern char lbl_1_data_95D[7];
extern char lbl_1_data_944[];
extern s32 lbl_1_data_A20[2];
extern char lbl_1_data_9DC[12];
extern s32 lbl_1_data_19C[36];
typedef struct MDBANK_LIGHT_VECTOR_PAIR_s {
    HuVecF vector[2];
} MDBANK_LIGHT_VECTOR_PAIR;

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
extern OMOBJ *lbl_1_bss_8;
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
extern s16 lbl_1_bss_34[6];
extern MDBANK_POST_CALLBACK_WORK lbl_1_bss_40[6];
extern MDBANK_EVENT_RECORD lbl_1_bss_300[80];
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

s32 lbl_1_data_0 = -1;
s32 lbl_1_data_4[12] = { DATANUM(0x940000, 3), DATANUM(0x940000, 4), DATANUM(0x940000, 6), DATANUM(0x940000, 7), DATANUM(0x940000, 24), DATANUM(0x940000, 9), DATANUM(0x940000, 28), DATANUM(0x940000, 19), DATANUM(0x940000, 20), DATANUM(0x940000, 21), DATANUM(0x940000, 22), DATANUM(0x940000, 75) };
s16 lbl_1_data_34[4] = { 2, 4, 4, 1 };
MDBANK_SPRITE_CONFIG lbl_1_data_3C[11] = { { 0, 0, 2, 10, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f }, { 0, 1, 3, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f }, { 1, 0, 5, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f }, { 1, 1, 5, 0, 2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f }, { 1, 2, 5, 0, 3, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f }, { 1, 3, 5, 0, 1, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f }, { 2, 0, 4, 20, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f }, { 2, 1, 4, 21, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f }, { 2, 2, 4, 22, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f }, { 2, 3, 4, 23, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f }, { 3, 0, 11, 0, 0, 288.0f, 240.0f, 10.0f, 10.0f, 0.0f } };
s32 lbl_1_data_19C[36] = { DATANUM(0x940000, 28), DATANUM(0x940000, 30), DATANUM(0x940000, 29), DATANUM(0x940000, 54), DATANUM(0x940000, 31), DATANUM(0x940000, 32), DATANUM(0x940000, 33), DATANUM(0x940000, 34), DATANUM(0x940000, 35), DATANUM(0x940000, 36), DATANUM(0x940000, 41), DATANUM(0x940000, 42), DATANUM(0x940000, 43), DATANUM(0x940000, 44), DATANUM(0x940000, 45), DATANUM(0x940000, 46), DATANUM(0x940000, 47), DATANUM(0x940000, 48), DATANUM(0x940000, 49), DATANUM(0x940000, 50), DATANUM(0x940000, 51), DATANUM(0x940000, 37), DATANUM(0x940000, 38), DATANUM(0x940000, 39), DATANUM(0x940000, 40), DATANUM(0x940000, 53), DATANUM(0x940000, 52), DATANUM(0x940000, 59), DATANUM(0x940000, 60), DATANUM(0x940000, 61), DATANUM(0x940000, 62), DATANUM(0x940000, 55), DATANUM(0x940000, 63), DATANUM(0x940000, 56), DATANUM(0x940000, 57), DATANUM(0x940000, 58) };
MDBANK_ITEM lbl_1_data_22C[55] = { { { 0, 1, 1, 10, 4, -1, -1, -1, -1, -1, 0, 0, 7, 0, 7, 55 } }, { { 0, 0, 2, 30, 5, 5, -1, 0, 1, -1, 0, 0, 7, 1, 7, 57 } }, { { 0, 0, 3, 30, 6, -1, -1, -1, -1, -1, 0, 0, 7, 2, 7, 58 } }, { { 0, 0, 4, 30, 7, -1, -1, -1, -1, -1, 0, 0, 7, 3, 7, 59 } }, { { 0, 0, 51, 100, 33, -1, -1, -1, -1, -1, 0, 0, 7, 50, 7, 111 } }, { { 0, 0, 52, 50, 34, -1, -1, -1, -1, -1, 0, 0, 7, 51, 7, 112 } }, { { 0, 0, 6, 50, 9, -1, -1, -1, -1, -1, 0, 0, 7, 5, 7, 62 } }, { { 0, 0, 7, 50, 21, -1, -1, -1, -1, -1, 0, 0, 7, 6, 7, 63 } }, { { 2, 0, 8, 50, 22, -1, -1, -1, -1, -1, 0, 0, 7, 7, 7, 64 } }, { { 0, 3, 9, 30, 23, -1, -1, -1, -1, -1, 0, 0, 7, 8, 7, 66 } }, { { 1, 0, 11, 10, 10, -1, 27, -1, -1, 1, 0, 0, 7, 10, 7, 70 } }, { { 1, 0, 12, 10, 10, -1, 27, -1, -1, 2, 0, 0, 7, 11, 7, 72 } }, { { 1, 0, 13, 10, 10, -1, 27, -1, -1, 3, 0, 0, 7, 12, 7, 73 } }, { { 1, 0, 14, 10, 10, -1, 27, -1, -1, 4, 0, 0, 7, 13, 7, 74 } }, { { 1, 0, 15, 10, 10, -1, 28, -1, -1, 0, 0, 0, 7, 14, 7, 75 } }, { { 1, 0, 16, 20, 10, -1, 28, -1, -1, 1, 0, 0, 7, 15, 7, 76 } }, { { 1, 0, 17, 20, 10, -1, 28, -1, -1, 2, 0, 0, 7, 16, 7, 77 } }, { { 1, 0, 18, 20, 10, -1, 28, -1, -1, 3, 0, 0, 7, 17, 7, 78 } }, { { 1, 0, 19, 20, 10, -1, 28, -1, -1, 4, 0, 0, 7, 18, 7, 79 } }, { { 1, 0, 20, 20, 10, -1, 29, -1, -1, 0, 0, 0, 7, 19, 7, 80 } }, { { 1, 0, 21, 30, 10, -1, 29, -1, -1, 1, 0, 0, 7, 20, 7, 81 } }, { { 1, 0, 22, 30, 10, -1, 29, -1, -1, 2, 0, 0, 7, 21, 7, 82 } }, { { 1, 0, 23, 30, 10, -1, 29, -1, -1, 3, 0, 0, 7, 22, 7, 83 } }, { { 1, 0, 24, 30, 10, -1, 29, -1, -1, 4, 0, 0, 7, 23, 7, 84 } }, { { 1, 0, 25, 30, 10, -1, 30, -1, -1, 0, 0, 0, 7, 24, 7, 85 } }, { { 1, 0, 26, 40, 10, -1, 30, -1, -1, 1, 0, 0, 7, 25, 7, 86 } }, { { 1, 0, 27, 40, 10, -1, 30, -1, -1, 2, 0, 0, 7, 26, 7, 87 } }, { { 1, 0, 28, 40, 10, -1, 30, -1, -1, 3, 0, 0, 7, 27, 7, 88 } }, { { 1, 0, 29, 40, 10, -1, 30, -1, -1, 4, 0, 0, 7, 28, 7, 89 } }, { { 0, 0, 30, 10, 25, -1, -1, -1, -1, -1, 0, 0, 7, 29, 7, 90 } }, { { 0, 0, 31, 30, 11, -1, -1, -1, -1, -1, 0, 0, 7, 30, 7, 91 } }, { { 0, 0, 32, 30, 11, -1, -1, -1, -1, -1, 0, 0, 7, 31, 7, 92 } }, { { 0, 0, 33, 30, 11, -1, -1, -1, -1, -1, 0, 0, 7, 32, 7, 93 } }, { { 0, 0, 34, 30, 11, -1, -1, -1, -1, -1, 0, 0, 7, 33, 7, 94 } }, { { 0, 0, 35, 30, 11, -1, -1, -1, -1, -1, 0, 0, 7, 34, 7, 95 } }, { { 0, 0, 36, 10, 12, -1, -1, -1, -1, -1, 0, 0, 7, 35, 7, 96 } }, { { 0, 0, 37, 10, 13, -1, -1, -1, -1, -1, 0, 0, 7, 36, 7, 97 } }, { { 0, 0, 38, 10, 14, -1, -1, -1, -1, -1, 0, 0, 7, 37, 7, 98 } }, { { 0, 0, 39, 10, 15, -1, -1, -1, -1, -1, 0, 0, 7, 38, 7, 99 } }, { { 0, 0, 40, 10, 16, -1, -1, -1, -1, -1, 0, 0, 7, 39, 7, 100 } }, { { 0, 0, 41, 10, 17, -1, -1, -1, -1, -1, 0, 0, 7, 40, 7, 101 } }, { { 0, 0, 42, 10, 18, -1, -1, -1, -1, -1, 0, 0, 7, 41, 7, 102 } }, { { 0, 0, 43, 10, 19, -1, -1, -1, -1, -1, 0, 0, 7, 42, 7, 103 } }, { { 0, 0, 44, 10, 20, -1, -1, -1, -1, -1, 0, 0, 7, 43, 7, 104 } }, { { 0, 0, 45, 10, 26, -1, -1, -1, -1, -1, 0, 0, 7, 44, 7, 105 } }, { { 0, 0, 46, 10, 3, -1, -1, -1, -1, -1, 0, 0, 7, 45, 7, 106 } }, { { 0, 0, 47, 10, 3, -1, -1, -1, -1, -1, 0, 0, 7, 46, 7, 107 } }, { { 0, 0, 48, 10, 3, -1, -1, -1, -1, -1, 0, 0, 7, 47, 7, 110 } }, { { 0, 0, 49, 10, 3, -1, -1, -1, -1, -1, 0, 0, 7, 48, 7, 116 } }, { { 0, 0, 50, 10, 3, -1, -1, -1, -1, -1, 0, 0, 7, 49, 7, 109 } }, { { 0, 0, 54, 10, 3, -1, -1, -1, -1, -1, 0, 0, 7, 53, 7, 108 } }, { { 0, 0, 55, 10, 3, -1, -1, -1, -1, -1, 0, 0, 7, 54, 7, 115 } }, { { 0, 0, 10, 10, 24, -1, -1, -1, -1, -1, 0, 0, 7, 9, 7, 69 } }, { { 3, 0, 53, 10, 24, -1, -1, -1, -1, -1, 0, 0, 7, 52, 7, 113 } }, { { 0, 2, 5, 100, 8, -1, -1, -1, -1, -1, 0, 0, 7, 4, 7, 60 } } };
u32 lbl_1_data_90C = 0xFFFFFFFF;

extern u8 lbl_802BFFE0[4];
extern u16 lbl_802C0008[4];

void fn_1_0(HUWINID winId, u32 mess, s16 index);
void fn_1_1AC(void);
void fn_1_308(void);
void fn_1_3FC(MDBANK_CAMERA_WORK *camera, float weight);
void fn_1_46C(OMOBJ *obj, MDBANK_CAMERA_WORK *camera);
void fn_1_4B8(OMOBJ *obj);
void fn_1_588(MDBANK_CAMERA_CALLBACK callback);
void fn_1_724(void);
void fn_1_780(void);
void fn_1_8B4(void);
void fn_1_8F0(void);
void fn_1_9A4(void);
void fn_1_9A8(s16 winNo);
void fn_1_A18(s16 winNo);
void fn_1_A88(s16 winNo);
s16 fn_1_AC4(s16 winNo, s16 mode);
void fn_1_B98(s16 winNo, u32 messNum, s16 speed);
void fn_1_C54(s16 winNo, u32 messNum, s16 insertMesNo);
void fn_1_CE4(void);
void fn_1_F0C(void);
void fn_1_F68(s16 winNo);
void fn_1_10E4(void);
void fn_1_11A0(void);
s16 fn_1_1200(s16 mode);
void fn_1_12F8(s16 winNo, s32 messNum, s16 speed);
void fn_1_1540(s16 winNo, s32 messNum, s16 insertMesNo);
void fn_1_1764(s32 messNum);
void fn_1_18E8(void);
void fn_1_1994(void);
void fn_1_1BD0(void);
void fn_1_1BD4(OMOBJ *obj);
void fn_1_1C64(void);
void fn_1_1CD8(OMOBJ *obj);
void fn_1_1D68(void);
void fn_1_1DDC(OMOBJ *obj);
void fn_1_1EE8(void);
void fn_1_1FD8(OMOBJ *obj);
void fn_1_21E4(OMOBJ *obj);
void fn_1_226C(OMOBJ *obj);
void fn_1_2378(void);
void fn_1_2468(OMOBJ *obj);
void fn_1_26B0(OMOBJ *obj);
void fn_1_2750(OMOBJ *obj);
void fn_1_281C(void);
void fn_1_2888(void);
void fn_1_28B4(OMOBJ *obj);
void fn_1_2A50(OMOBJ *obj);
void fn_1_2AA4(OMOBJ *obj);
void fn_1_2B34(OMOBJ *obj);
void fn_1_2C54(OMOBJ *obj);
void fn_1_2CD8(s16 index, HuVecF *worldPos, float offsetX, float offsetY);
void fn_1_2D70(s16 index);
void fn_1_2D98(s16 index, HuVecF *worldPos, float offsetX, float offsetY);
void fn_1_2EFC(s16 index);
void fn_1_2F80(void);
void fn_1_304C(void);
void fn_1_3100(HuVecF *pos);
void fn_1_3164(HuVecF *pos);
void fn_1_3280(void);
void fn_1_32C0(OMOBJ *obj);
void fn_1_3468(OMOBJ *obj);
void fn_1_35E0(OMOBJ *obj);
void fn_1_3640(void);
void fn_1_3788(OMOBJ *obj);
void fn_1_3DBC(s16 state, s16 time);
void fn_1_3E30(OMOBJ *obj);
void fn_1_4184(OMOBJ *obj);
void fn_1_420C(HUSPR_GROUPID groupId, s32 memberNo, s16 value);
void fn_1_4360(MDBANK_ANIM_SET *set, MDBANK_ITEM *item);
void fn_1_4730(void);
s32 fn_1_47AC(s16 index);
void fn_1_48CC(s16 index);
void fn_1_4C64(s16 index);
void fn_1_4C9C(void);
void fn_1_4DFC(s16 index);
void fn_1_520C(s16 skipIndex);
void fn_1_53AC(s16 skipIndex);
void fn_1_5550(void);
void fn_1_5834(void);
void fn_1_58B8(OMOBJ *obj);
void fn_1_5988(OMOBJ *obj);
void fn_1_5C1C(s16 mode);
void fn_1_5D64(OMOBJ *obj);
void fn_1_61A8(OMOBJ *obj);
void fn_1_6290(HUSPR_GROUPID groupId, s16 baseIndex, s16 value);
void fn_1_6600(void);
void fn_1_6668(void);
void fn_1_6684(void);
void fn_1_6810(OMOBJ *obj);
void fn_1_6B0C(void);
void fn_1_6B34(void);
void fn_1_6B64(void);
void fn_1_6BDC(void);
void fn_1_6C6C(void);
void fn_1_6F00(void);
void fn_1_70E4(OMOBJ *obj);
void fn_1_7594(OMOBJ *obj);
void fn_1_7990(OMOBJ *obj);
void fn_1_7A20(s16 state);
void fn_1_7A84(OMOBJ *obj);
void fn_1_7F2C(OMOBJ *obj);
void fn_1_8280(void);
void fn_1_8284(void);
void fn_1_8340(void);
void fn_1_83CC(void);
void fn_1_86C0(void);
void ObjectSetup(void);
int _prolog(void);
void _epilog(void);
void fn_1_92A4(void);
void fn_1_9380(OMOBJ *obj, MDBANK_CAMERA_WORK *camera);
s32 fn_1_9464(void);
s32 fn_1_981C(void);
s32 fn_1_9DE8(void);
s16 fn_1_A438(void);
void fn_1_A750(s16 index);
s16 fn_1_AA08(void);
short fn_1_EA98(s32 unused0, s32 unused1);
s32 fn_1_EEB8(s16 index);
s32 fn_1_F7C0(s16 index);
void fn_1_1050C(s16 index);

void fn_1_0(HUWINID winId, u32 mess, s16 index)
{
    s32 messages[2] = { MESSNUM(MESS_SBANK_MAIN_MENU, 0), -1 };
    s32 effects[16] = { MSM_SE_GUIDE_DAY_HAPPY,      MSM_SE_GUIDE_DAY_INTRO,
                        MSM_SE_GUIDE_DAY_APOLOGY,    MSM_SE_GUIDE_DAY_EXPLAIN,
                        MSM_SE_GUIDE_DAY_SURPRISE,   MSM_SE_GUIDE_DAY_DISAPPOINTED,
                        MSM_SE_GUIDE_DAY_LAUGH,      -1,
                        MSM_SE_GUIDE_NIGHT_HAPPY,    MSM_SE_GUIDE_NIGHT_INTRO,
                        MSM_SE_GUIDE_NIGHT_APOLOGY,  MSM_SE_GUIDE_NIGHT_EXPLAIN,
                        MSM_SE_GUIDE_NIGHT_SURPRISE, MSM_SE_GUIDE_NIGHT_DISAPPOINTED,
                        MSM_SE_GUIDE_NIGHT_LAUGH,    -1 };
    s16 i;

    index--;
    OSReport("# ========== win callback :: %d\n", index);
    if (lbl_1_data_90C != mess) {
        lbl_1_data_90C = mess;
        for (i = 0;; i++) {
            if (messages[i] == -1) {
                HuAudFXPlay(effects[index]);
                break;
            }
            if (mess == messages[i]) {
                if (index >= 8) {
                    HuAudFXPlayPan(effects[index], 80);
                } else {
                    HuAudFXPlayPan(effects[index], 48);
                }
                break;
            }
        }
    }
}

s16 lbl_1_data_932[3] = { -1, -1, 0 };
s32 lbl_1_data_938[3] = { -1, -1, -1 };
char lbl_1_data_944[18] = "gN01m1-itemhook_R";
char lbl_1_data_956[7] = "dpic00";
char lbl_1_data_95D[7] = "dpic01";
s16 lbl_1_data_964[6] = { 0, 0, 0, 0, 0, 0 };
s16 lbl_1_data_970 = 100;
s16 lbl_1_data_972 = 100;
s16 lbl_1_data_974 = 60;
char lbl_1_data_976[8] = "null000";
char lbl_1_data_97E[7] = "null00";
char lbl_1_data_985[6] = "null0";
char lbl_1_data_98B[9] = "null00_b";
char lbl_1_data_994[8] = "null0_b";
char lbl_1_data_99C[8] = "ys77120";
char lbl_1_data_9A4[8] = "ys77060";
char lbl_1_data_9AC[9] = "hatena00";
char lbl_1_data_9B5[8] = "ys77121";
char lbl_1_data_9BD[8] = "ys77061";
char lbl_1_data_9C5[11] = "hatena01";
HuVecF lbl_1_data_9D0 = { 0.0f, 280.0f, 800.0f };
char lbl_1_data_9DC[12] = "bank_star04";

void fn_1_1AC(void)
{
    s16 i;

    lbl_1_bss_1930[0] = fn_8003D248();
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
        if (fn_8003D2F0(lbl_1_data_22C[i].value[2])) {
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
    lbl_80265E30.bankStar = lbl_1_bss_1930[0];
    for (i = 0; i < 55; i++) {
        if (lbl_1_data_22C[i].value[10] == 1) {
            fn_8003D27C(lbl_1_data_22C[i].value[2]);
        }
    }
    if (fn_8003D2F0(6)) {
        fn_8003CCAC(677);
    }
}

void fn_1_3FC(MDBANK_CAMERA_WORK *camera, float weight)
{
    fn_1_111F8(&camera->center, &camera->centerTarget, weight);
    fn_1_111F8(&camera->rot, &camera->rotTarget, weight);
    camera->zoom = fn_1_111B0(camera->zoom, camera->zoomTarget, weight);
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
    lbl_80266A4C.x = camera->center.x;
    lbl_80266A4C.y = camera->center.y;
    lbl_80266A4C.z = camera->center.z;
    lbl_80266A40.x = camera->rot.x;
    lbl_80266A40.y = camera->rot.y;
    lbl_80266A40.z = camera->rot.z;
    lbl_802C028C = camera->zoom;
    fn_8003D4FC(obj);
}

void fn_1_588(MDBANK_CAMERA_CALLBACK callback)
{
    MDBANK_CAMERA_WORK *camera = &lbl_1_bss_1998;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f,
        10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
        640.0f, 480.0f, 0.0f,
        1.0f);
    memset(camera, 0, sizeof(*camera));
    camera->callback = callback;
    camera->center.x = 0.0f;
    camera->center.y = 65.0f;
    camera->center.z = -800.0f;
    camera->rot.x = -7.25f;
    camera->rot.y = 0.0f;
    camera->rot.z = 0.0f;
    if (lbl_1_bss_2C == 0) {
        camera->zoom = 2650.0f;
    } else {
        camera->zoom = 2150.0f;
    }
    camera->obj = fn_8003AB38(lbl_1_bss_0, 256, 0, 0, -1, fn_1_4B8);
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
    HuVecF pos[2] = {
        { 0.0f, 1.0f, 1.0f },
        { -1.0f, 1.0f, -1.0f }
    };
    HuVecF dir[2] = {
        { 0.0f, -1.0f, -1.0f },
        { 1.0f, -1.0f, -1.0f }
    };
    GXColor color = { 128, 128, 255, 32 };

    lbl_1_bss_1994[0] =
        fn_800286D0(&pos[0], &dir[0], &color);
    Hu3DGLightInfinitytSet(lbl_1_bss_1994[0]);
    Hu3DGLightStaticSet(lbl_1_bss_1994[0], TRUE);
    lbl_1_bss_1994[1] =
        fn_800286D0(&pos[1], &dir[1], &color);
    Hu3DGLightInfinitytSet(lbl_1_bss_1994[1]);
    Hu3DGLightStaticSet(lbl_1_bss_1994[1], TRUE);
}

inline void fn_1_780(void);

void fn_1_8B4(void)
{
    Hu3DGLightKill(lbl_1_bss_1994[0]);
    Hu3DGLightKill(lbl_1_bss_1994[1]);
}

void fn_1_8F0(void)
{
    Vec shadowPos = { 0.0f, 3000.0f, 600.0f };
    Vec shadowUp = { 0.0f, 1.0f, 0.0f };
    Vec shadowTarget = { 0.0f, 0.0f, 0.0f };

    Hu3DShadowCreate(
        30.0f, 10.0f, 10000.0f);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
}

inline void fn_1_8F0(void);

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
    lbl_1_bss_198C[0] = HuWinExCreateFrame(16.0f,
        337.0f, 544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_198C[0]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[0], 0.0f);
    lbl_1_bss_198C[1] = HuWinExCreateFrame(16.0f,
        372.0f, 544, 68, -1, 5);
    HuWinDispOff(lbl_1_bss_198C[1]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[1], 0.9f);
    lbl_1_bss_198C[2] = HuWinExCreateFrame(16.0f,
        372.0f, 544, 68, -1, 3);
    HuWinDispOff(lbl_1_bss_198C[2]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[2], 0.9f);
    lbl_1_bss_198C[3] = HuWinExCreateFrame(16.0f,
        372.0f, 544, 68, -1, 4);
    HuWinDispOff(lbl_1_bss_198C[3]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[3], 0.9f);
    for (i = 0; i < 4; i++) {
        winData[lbl_1_bss_198C[i]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_198C[i], fn_1_0);
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

void fn_1_1BD0(void)
{
}

void fn_1_1BD4(OMOBJ *obj)
{
    if (obj->work[3]++ > 45) {
        obj->objFunc = NULL;
        fn_8002D86C(obj->mdlId[0], 1.0f);
        fn_8002CCC0(obj->mdlId[0], obj->mtnId[0], 0.0f,
            15.0f, ((1 << 30) | (1 << 0)));
    }
}

void fn_1_1C64(void)
{
    OMOBJ *obj = lbl_1_bss_C;

    fn_8002CCC0(obj->mdlId[0], obj->mtnId[1], 0.0f,
        20.0f, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_1BD4;
}

void fn_1_1CD8(OMOBJ *obj)
{
    if (obj->work[3]++ > 45) {
        obj->objFunc = NULL;
        fn_8002D86C(obj->mdlId[0], 1.0f);
        fn_8002CCC0(obj->mdlId[0], obj->mtnId[0], 0.0f,
            15.0f, ((1 << 30) | (1 << 0)));
    }
}

void fn_1_1D68(void)
{
    OMOBJ *obj = lbl_1_bss_10;

    fn_8002CCC0(obj->mdlId[0], obj->mtnId[1], 0.0f,
        20.0f, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_1CD8;
}

void fn_1_1DDC(OMOBJ *obj)
{
    MDBANK_MOVE_WORK *work = &lbl_1_bss_175C;
    HuVecF position;
    float rotation;

    fn_1_11678(&position, &work->start, &work->control, &work->end,
        fn_1_11458(0.0f, 1.0f, work->time, work->duration));
    fn_80026790(obj->mdlId[0], &position);
    rotation = fn_1_11458(0.0f, 15.0f, work->time, work->duration);
    fn_80026808(obj->mdlId[0], 0.0f, rotation, 0.0f);
    if ((work->time += 1.0f) > work->duration) {
        obj->objFunc = NULL;
    }
}

void fn_1_1EE8(void)
{
    OMOBJ *obj = lbl_1_bss_C;
    MDBANK_MOVE_WORK *work = &lbl_1_bss_175C;

    fn_1_1115C(&work->start, -100.0f, 25.0f, 0.0f);
    fn_1_1115C(&work->control, -300.0f, 25.0f, 0.0f);
    fn_1_1115C(&work->end, -350.0f, 150.0f, 0.0f);
    work->time = 0.0f;
    work->duration = 60.0f;
    obj->objFunc = fn_1_1DDC;
}

void fn_1_1FD8(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, (1 << 8));
    obj->mdlId[0] = fn_80025A04(fn_80008DD0((DATANUM(0x940000, 64)), 0x10000000, 2));
    for (i = 0; i < 3; i++) {
        obj->mtnId[i] = fn_80030458(obj->mdlId[0], fn_80008DD0((DATANUM(0x940000, 65) + i), 0x10000000, 2));
    }
    fn_80026B8C(obj->mdlId[0], 1);
    fn_8002CCC0(obj->mdlId[0], obj->mtnId[0], 0.0f,
        0.0f, ((1 << 30) | (1 << 0)));
    if (lbl_1_bss_2C == 0) {
        fn_80026760(obj->mdlId[0], -100.0f, 25.0f,
            0.0f);
        fn_80026808(obj->mdlId[0], 0.0f, 0.0f,
            0.0f);
    } else {
        fn_80026760(obj->mdlId[0], -350.0f, 150.0f,
            0.0f);
        fn_80026808(obj->mdlId[0], 0.0f, 15.0f,
            0.0f);
    }
    fn_800268B0(obj->mdlId[0], 1.25f, 1.25f,
        1.25f);
    fn_80026E44(obj->mdlId[0]);
    obj->objFunc = NULL;
}

void fn_1_21E4(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 3; i++) {
            fn_8002C624(obj->mtnId[i]);
        }
        fn_800262D0(obj->mdlId[0]);
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
        fn_1_11458(0.0f, 1.0f,
            work->time, work->duration));
    fn_80026790(obj->mdlId[0], &pos);
    rotation = fn_1_11458(0.0f, -15.0f,
        work->time, work->duration);
    fn_80026808(obj->mdlId[0], 0.0f,
        rotation, 0.0f);
    if ((work->time += 1.0f) > work->duration) {
        obj->objFunc = NULL;
    }
}

void fn_1_2378(void)
{
    OMOBJ *obj = lbl_1_bss_10;
    MDBANK_MOVE_WORK *work = &lbl_1_bss_171C;

    fn_1_1115C(&work->start, 100.0f,
        25.0f, 0.0f);
    fn_1_1115C(&work->control, 300.0f,
        25.0f, 0.0f);
    fn_1_1115C(&work->end, 350.0f,
        150.0f, 0.0f);
    work->time = 0.0f;
    work->duration = 60.0f;
    obj->objFunc = fn_1_226C;
}

inline void fn_1_2378(void);

void fn_1_2468(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, (1 << 8));
    obj->mdlId[0] = fn_80025A04(fn_80008DD0((DATANUM(0x940000, 68)), 0x10000000, 2));
    obj->mdlId[1] = fn_80025A04(fn_80008DD0((DATANUM(0x940000, 69)), 0x10000000, 2));
    for (i = 0; i < 3; i++) {
        obj->mtnId[i] = fn_80030458(obj->mdlId[0], fn_80008DD0((DATANUM(0x940000, 70) + i), 0x10000000, 2));
    }
    fn_8002741C(obj->mdlId[0], lbl_1_data_944, obj->mdlId[1]);
    fn_80026B8C(obj->mdlId[0], 1);
    fn_8002CCC0(obj->mdlId[0], obj->mtnId[0], 0.0f,
        0.0f, ((1 << 30) | (1 << 0)));
    if (lbl_1_bss_2C == 0) {
        fn_80026760(obj->mdlId[0], 100.0f,
            25.0f, 0.0f);
        fn_80026808(obj->mdlId[0], 0.0f, 0.0f,
            0.0f);
    } else {
        fn_80026760(obj->mdlId[0], 350.0f,
            150.0f, 0.0f);
        fn_80026808(obj->mdlId[0], 0.0f,
            -15.0f, 0.0f);
    }
    fn_800268B0(obj->mdlId[0], 1.25f, 1.25f,
        1.25f);
    fn_80026E44(obj->mdlId[0]);
    obj->objFunc = NULL;
}

void fn_1_26B0(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        fn_80027508(obj->mdlId[1]);
        for (i = 0; i < 3; i++) {
            fn_8002C624(obj->mtnId[i]);
        }
        fn_800262D0(obj->mdlId[0]);
        fn_800262D0(obj->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_2750(OMOBJ *obj)
{
    float opacity;

    opacity = fn_1_1116C(1.0f, 0.0f,
        (float)obj->work[0], 10.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_194E[0], opacity);
    if (++obj->work[0] > 10) {
        fn_1_11880(lbl_1_bss_194E[0], HUSPR_ATTR_DISPOFF);
        obj->objFunc = NULL;
    }
}

void fn_1_281C(void)
{
    if (lbl_1_bss_2C == 0) {
        HuSprGrpPosSet(lbl_1_bss_194E[0], 288.0f,
            120.0f);
        fn_1_11900(lbl_1_bss_194E[0], HUSPR_ATTR_DISPOFF);
    }
}

inline void fn_1_281C(void);

void fn_1_2888(void)
{
    OMOBJ *obj = lbl_1_bss_4;

    obj->objFunc = fn_1_2750;
}

void fn_1_28B4(OMOBJ *obj)
{
    omSetStatBit(obj, (1 << 8));
    obj->mdlId[0] = fn_80025A04(fn_80008DD0((DATANUM(0x940000, 0)), 0x10000000, 2));
    obj->mtnId[0] = fn_8002D3C4(obj->mdlId[0]);
    obj->mdlId[1] = fn_80025A04(fn_80008DD0((DATANUM(0x940000, 1)), 0x10000000, 2));
    obj->mtnId[1] = fn_8002D3C4(obj->mdlId[0]);
    fn_80026B8C(obj->mdlId[0], 1);
    fn_8002CCC0(obj->mdlId[0], obj->mtnId[0], 0.0f,
        0.0f, ((1 << 30) | (1 << 0)));
    fn_80026B8C(obj->mdlId[1], 1);
    fn_8002CCC0(obj->mdlId[1], obj->mtnId[1], 0.0f,
        0.0f, ((1 << 30) | (1 << 0)));
    fn_80026CBC(obj->mdlId[1], 0.6f);
    fn_80026FFC(obj->mdlId[0]);
    fn_1_281C();
    obj->objFunc = NULL;
}

void fn_1_2A50(OMOBJ *obj)
{
    if (obj) {
        fn_800262D0(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_2AA4(OMOBJ *obj)
{
    HuVecF rot;

    fn_80026874(obj->mdlId[0], &rot);
    rot.z -= 0.1f;
    if (rot.z < 0.0f) {
        rot.z += 360.0f;
    }
    fn_80026838(obj->mdlId[0], &rot);
}

void fn_1_2B34(OMOBJ *obj)
{
    omSetStatBit(obj, (1 << 8));
    obj->mdlId[0] = fn_80025A04(fn_80008DD0((DATANUM(0x940000, 2)), 0x10000000, 2));
    fn_80026760(obj->mdlId[0], 0.0f, -100.0f,
        -850.0f);
    fn_80026808(obj->mdlId[0], 0.0f, 0.0f,
        0.0f);
    fn_80026B8C(obj->mdlId[0], 1);
    obj->mtnId[0] = fn_80030B2C(lbl_1_bss_1958[0], obj->mdlId[0],
        lbl_1_data_956);
    obj->mtnId[1] = fn_80030B2C(lbl_1_bss_1958[1], obj->mdlId[0],
        lbl_1_data_95D);
    obj->objFunc = fn_1_2AA4;
}

void fn_1_2C54(OMOBJ *obj)
{
    if (obj) {
        fn_80031010(obj->mtnId[0]);
        fn_80031010(obj->mtnId[1]);
        obj->mtnId[0] = -1;
        obj->mtnId[1] = -1;
        fn_800262D0(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_2CD8(s16 index, HuVecF *worldPos, float offsetX, float offsetY)
{
    HuVecF screenPos = { 0.0f, 0.0f, 0.0f };

    if (worldPos) {
        Hu3D3Dto2D(worldPos, 1, &screenPos);
    }
    HuSprPosSet(lbl_1_bss_194E[1], index,
        screenPos.x + offsetX, screenPos.y + offsetY);
}

void fn_1_2D70(s16 index)
{
    lbl_1_bss_1700[index] = 2.0f;
}

void fn_1_2D98(s16 index, HuVecF *worldPos, float offsetX, float offsetY)
{
    OMOBJ *obj = lbl_1_bss_14;

    if (obj->work[index] != 1) {
        fn_1_2D70(index);
        fn_1_2CD8(index, worldPos, offsetX, offsetY);
        HuSprScaleSet(lbl_1_bss_194E[1], index,
            2.0f, 2.0f);
        HuSprAttrReset(lbl_1_bss_194E[1], index, HUSPR_ATTR_DISPOFF);
        obj->work[index] = 1;
        lbl_1_data_964[index] = 1;
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
            1.0f, 5.0f);
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
    lbl_1_bss_1710.x = pos->x - 50.0f;
    lbl_1_bss_1710.y = 50.0f + pos->y;
    lbl_1_bss_1710.z = 50.0f + pos->z;
}

inline void fn_1_3100(HuVecF *pos);

void fn_1_3164(HuVecF *pos)
{
    OMOBJ *obj = lbl_1_bss_14;

    fn_80026790(obj->mdlId[0], pos);
    fn_80026808(obj->mdlId[0], 0.0f, 0.0f,
        180.0f);
    fn_800268B0(obj->mdlId[0], 1.0f, 1.0f,
        1.0f);
    fn_1_3100(pos);
    fn_80026A44(obj->mdlId[0], (1 << 0));
}

void fn_1_3280(void)
{
    OMOBJ *obj = lbl_1_bss_14;

    fn_800269E8(obj->mdlId[0], (1 << 0));
}

void fn_1_32C0(OMOBJ *obj)
{
    HuVecF transform;
    s16 i;

    fn_800267CC(obj->mdlId[0], &transform);
    fn_1_111F8(&transform, &lbl_1_bss_1710, 3.0f);
    fn_80026790(obj->mdlId[0], &transform);
    fn_80026874(obj->mdlId[0], &transform);
    transform.z = fn_1_111B0(transform.z, 0.0f, 3.0f);
    fn_80026838(obj->mdlId[0], &transform);
    fn_8002691C(obj->mdlId[0], &transform);
    transform.x = transform.y = transform.z =
        fn_1_111B0(transform.x, 0.3f, 3.0f);
    fn_800268E0(obj->mdlId[0], &transform);
    for (i = 0; i < 4; i++) {
        lbl_1_bss_1700[i] = fn_1_111B0(lbl_1_bss_1700[i], 1.0f,
            5.0f);
        HuSprScaleSet(lbl_1_bss_194E[1], i, lbl_1_bss_1700[i],
            lbl_1_bss_1700[i]);
    }
    fn_1_3640();
}

void fn_1_3468(OMOBJ *obj)
{
    OMOBJ *displayObj;
    s16 i;

    omSetStatBit(obj, (1 << 8));
    obj->mdlId[0] = fn_80025A04(fn_80008DD0((DATANUM(0x940000, 8)), 0x10000000, 2));
    obj->mtnId[0] = fn_8002D3C4(obj->mdlId[0]);
    fn_80026B8C(obj->mdlId[0], 5);
    fn_8002CCC0(obj->mdlId[0], obj->mtnId[0], 0.0f,
        0.0f, ((1 << 30) | (1 << 0)));
    fn_800269E8(obj->mdlId[0], (1 << 0));
    displayObj = lbl_1_bss_14;
    fn_800269E8(displayObj->mdlId[0], (1 << 0));
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

void fn_1_35E0(OMOBJ *obj)
{
    if (obj) {
        fn_8002C624(obj->mdlId[0]);
        fn_800262D0(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
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
        verticalOffset = fn_1_11384(0.0f, 0.25f,
            event->time, event->duration);
        fn_800267CC(obj->mdlId[i], &modelPos);
        modelPos.y += verticalOffset;
        fn_80026790(obj->mdlId[i], &modelPos);
        if ((event->time += 1.0f) > event->duration) {
            event->time = 0.0f;
            event->duration = (float)(frandmod(0xB4) + 0xB4);
        }
    }
}

void fn_1_3788(OMOBJ *obj)
{
    MDBANK_EVENT_RECORD *event;
    HuVecF position;
    float controlY;
    s16 slot;
    s16 modelIndex;
    s16 firstSlot;
    s16 lastSlot;

    if (lbl_1_data_970 > 0) {
        firstSlot = 20;
        lastSlot = 80;
        for (slot = firstSlot; slot < lastSlot; slot++) {
            event = &lbl_1_bss_300[slot];
            if (--lbl_1_bss_34[0] >= 0) {
                break;
            }
            if (event->active != 0) {
                continue;
            }

            HuAudFXPlay(MSM_SE_BANK_ITEM_EVENT_START);
            lbl_1_bss_34[0] = lbl_1_data_974;
            event->active = 1;
            event->time = 0.0f;
            event->duration = 90.0f;

            if (lbl_1_bss_1930[0] >= 20) {
                fn_1_1115C(&event->start, 0.0f,
                    (float)(frandmod(50) + 350), -100.0f);
            } else {
                modelIndex = lbl_1_bss_1930[0] - 1;
                if (modelIndex < 0) {
                    modelIndex = 0;
                }
                fn_800267CC(obj->mdlId[modelIndex], &event->start);
            }

            controlY = (float)(frandmod(50) + 675);
            fn_1_1115C(&event->control, (float)(frandmod(300) - 150), controlY,
                300.0f);
            fn_1_1115C(&event->end, 0.0f,
                244.0f, 600.0f);

            --lbl_1_bss_1930[0];
            if (lbl_1_bss_1930[0] < 0) {
                lbl_1_bss_1930[0] = 0;
            }
            --lbl_1_data_970;
            if (lbl_1_data_970 < 0) {
                lbl_1_data_970 = 0;
            }
            event->rotation = (float)(frandmod(5000) - 2500);

            fn_80026790(obj->mdlId[slot], &event->start);
            fn_80026808(obj->mdlId[slot], 0.0f,
                0.0f, 0.0f);
            fn_800268B0(obj->mdlId[slot], 0.25f,
                0.25f, 0.25f);
            fn_80026B8C(obj->mdlId[slot], 1);
            fn_80026A44(obj->mdlId[slot], (1 << 0));
            break;
        }
    }

    for (slot = 20; slot < 80; slot++) {
        event = &lbl_1_bss_300[slot];
        if (event->active == 0) {
            continue;
        }
        if (event->time > event->duration / 2.0f) {
            fn_80026B8C(obj->mdlId[slot], 2);
        }

        fn_1_11678(&position, &event->start, &event->control,
            &event->end, fn_1_1116C(0.0f, 1.0f,
                event->time, event->duration));
        fn_80026790(obj->mdlId[slot], &position);

        fn_80026874(obj->mdlId[slot], &position);
        position.y = fn_1_1116C(event->rotation, 0.0f,
            event->time, event->duration);
        fn_80026838(obj->mdlId[slot], &position);

        position.x = position.y = position.z = fn_1_1116C(
            0.25f, 0.8f, event->time,
            event->duration);
        fn_800268E0(obj->mdlId[slot], &position);

        if ((event->time += 1.0f)
            > event->duration + 2.0f) {
            fn_800267CC(obj->mdlId[slot], &position);
            fn_1_131C8(&position);
            HuAudFXPlay(MSM_SE_BANK_ITEM_EVENT_BURST);
            event->active = 0;
            fn_800269E8(obj->mdlId[slot], (1 << 0));

            --lbl_1_data_22C[lbl_1_data_972].value[3];
            if (lbl_1_data_22C[lbl_1_data_972].value[3] < 0) {
                lbl_1_data_22C[lbl_1_data_972].value[3] = 0;
            }
        }
    }

    if (lbl_1_data_22C[lbl_1_data_972].value[3] <= 0) {
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

void fn_1_3E30(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, (1 << 8));
    for (i = 0; i < 80; i++) {
        if (i == 0) {
            obj->mdlId[i] = fn_80025A04(fn_80008DD0(
                DATANUM(0x940000, 23), 0x10000000, 2));
            obj->mtnId[i] = fn_8002D3C4(obj->mdlId[i]);
        } else {
            obj->mdlId[i] = fn_80025D4C(obj->mdlId[0]);
        }
        fn_800268B0(obj->mdlId[i], 0.8f,
            0.8f, 0.8f);
        fn_80026B8C(obj->mdlId[i], 1);
        fn_8002CCC0(obj->mdlId[i], obj->mtnId[0],
            0.0f, 0.0f, ((1 << 30) | (1 << 0)));
        fn_800269E8(obj->mdlId[i], (1 << 0));
    }

    for (i = 0; i < 20; i++) {
        lbl_1_bss_210[i].x = (float)(frandmod(120) - 60);
        lbl_1_bss_210[i].y = (float)(frandmod(80) - 40);
        lbl_1_bss_210[i].z = (float)frandmod(20);
        fn_80026760(obj->mdlId[i], lbl_1_bss_210[i].x,
            lbl_1_bss_210[i].y, lbl_1_bss_210[i].z);
        fn_80026808(obj->mdlId[i], 0.0f,
            (float)(frandmod(90) - 45), 0.0f);
        fn_800268B0(obj->mdlId[i], 0.4f,
            0.4f, 0.4f);
    }
    obj->objFunc = NULL;
}

void fn_1_4184(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 49; i >= 0; i--) {
            fn_800262D0(obj->mdlId[i]);
        }
        fn_8002C624(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
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

void fn_1_4360(MDBANK_ANIM_SET *set, MDBANK_ITEM *item)
{
    Mtx matrix;
    HuVecF modelPos;
    HuVecF screenPos;
    s16 digitSet = 0;
    char *names[3][3] = { { lbl_1_data_976, lbl_1_data_97E, lbl_1_data_985 }, { lbl_1_data_98B, lbl_1_data_98B, lbl_1_data_994 }, { lbl_1_data_97E, lbl_1_data_97E, lbl_1_data_97E } };
    s16 i;

    if (item->value[3] >= 100) {
        digitSet = 0;
    } else if (item->value[3] >= 10) {
        digitSet = 1;
    } else {
        digitSet = 2;
    }
    fn_800267CC(set->model[0], &modelPos);
    fn_80026790(set->model[1], &modelPos);
    fn_8002691C(set->model[0], &modelPos);
    fn_800268B0(set->model[1], 1.0f,
        1.0f, modelPos.z);
    if (set->state[3] != 1) {
        fn_80026874(set->model[0], &modelPos);
        while (TRUE) {
            if (modelPos.y > 360.0f) {
                modelPos.y -= 360.0f;
                continue;
            }
            break;
        }
        while (TRUE) {
            if (modelPos.y < 0.0f) {
                modelPos.y += 360.0f;
                continue;
            }
            break;
        }
        fn_80026838(set->model[1], &modelPos);
        for (i = 0; i < 3; i++) {
            HuSpr3DRotSet(set->state[i], 0.0f,
                modelPos.y, 0.0f);
        }
        for (i = 0; i < 3; i++) {
            fn_80023398(set->model[1], names[digitSet][i], matrix);
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
        fn_80026874(set->model[1], &modelPos);
        for (i = 0; i < 3; i++) {
            if (modelPos.y >= 90.0f
                && modelPos.y <= 270.0f) {
                HuSprAttrSet(set->spriteGroup, i, HUSPR_ATTR_DISPOFF);
            }
        }
    }
}

void fn_1_4730(void)
{
    s16 i;

    for (i = 0; i < 10; i++) {
        fn_80026808(lbl_1_bss_179C[i].model[0], 0.0f,
            0.0f, 0.0f);
    }
}

s32 fn_1_47AC(s16 index)
{
    s16 i;
    HuVecF rotation;
    fn_80026874(lbl_1_bss_179C[index].model[0], & rotation);
    rotation.y += 5.0f;
    if (rotation.y > 360.0f)
    {
        rotation.y -= 360.0f;
    }
    fn_80026838(lbl_1_bss_179C[index].model[0], & rotation);
    for (i = 0; i < 10; i++)
    {
        if (i != index) {
            fn_80026808(lbl_1_bss_179C[i].model[0], 0.0f, 0.0f, 0.0f);
        }
    }
}

void fn_1_48CC(s16 index)
{
    MDBANK_ANIM_SET *set;
    OMOBJ *obj;
    HuVecF pos;
    HuVecF rotation;
    HuVecF value;
    s16 i = 0;
    s16 end = 30;

    set = &lbl_1_bss_179C[index];
    obj = lbl_1_bss_18;
    fn_800267CC(set->model[0], &pos);
    fn_80026874(set->model[0], &rotation);
    fn_80026B8C(set->model[0], 3);
    HuSprGrpDrawNoSet(set->spriteGroup, 65);
    do {
        HuPrcVSleep();
        value.x = fn_1_11458(pos.x, 0.0f, i, end);
        value.y = fn_1_11458(pos.y, 210.0f, i, end);
        value.z = fn_1_11458(pos.z, 300.0f, i, end);
        fn_80026790(set->model[0], &value);
        value.x = 0.0f;
        value.y = fn_1_11458(rotation.y, 720.0f, i, end);
        value.z = 0.0f;
        fn_80026838(set->model[0], &value);
        value.x = value.y = fn_1_11458(1.0f, 1.75f,
            i, end);
        value.z = 1.0f;
        fn_800268E0(set->model[0], &value);
    } while (++i <= end);
    fn_80026760(set->model[0], 0.0f, 210.0f,
        300.0f);
    fn_80026808(set->model[0], 0.0f, 0.0f,
        0.0f);
    fn_800268B0(set->model[0], 1.75f, 1.75f,
        1.0f);
}

void fn_1_4C64(s16 index)
{
    OMOBJ *obj = lbl_1_bss_18;

    obj->work[0] = 0;
    obj->work[1] = 1;
    obj->work[2] = index;
}

void fn_1_4C9C(void)
{
    s16 i;
    OMOBJ *obj = lbl_1_bss_18;
    MDBANK_ANIM_SET *set;
    HuVecF rotation;
    float angle;

    obj->work[1] = 0;
    set = &lbl_1_bss_179C[obj->work[2]];
    fn_80026874(set->model[0], &rotation);
    if (rotation.y > 180.0f) {
        rotation.y -= 360.0f;
    }
    i = 0;
    for (; i < 10; i++) {
        HuPrcVSleep();
        angle = fn_1_11458(rotation.y, 0.0f, i, 10.0f);
        fn_80026808(set->model[0], 0.0f,
            angle, 0.0f);
    }
    fn_80026808(set->model[0], 0.0f, 0.0f,
        0.0f);
}

void fn_1_4DFC(s16 index)
{
    MDBANK_ANIM_SET *set;
    OMOBJ *obj;
    HuVecF pos;
    HuVecF rotation;
    HuVecF value;
    HuVecF target;
    s16 i;
    s16 end;

    i = 0;
    end = 30;
    set = &lbl_1_bss_179C[index];
    obj = lbl_1_bss_18;

    fn_800267CC(set->model[0], &pos);
    fn_80026874(set->model[0], &rotation);
    fn_1_1115C(&target, (index % 5) * 100 - 200,
        235 - (index / 5) * 100, 100.0f);
    obj->work[1] = 0;
    do {
        HuPrcVSleep();
        value.x = fn_1_11458(pos.x, target.x, i, end);
        value.y = fn_1_11458(pos.y, target.y, i, end);
        value.z = fn_1_11458(pos.z, target.z, i, end);
        fn_80026790(set->model[0], &value);
        value.x = 0.0f;
        value.y = fn_1_11458(720.0f, rotation.y, i, end);
        value.z = 0.0f;
        fn_80026838(set->model[0], &value);
        value.x = value.y = fn_1_11458(1.75f, 1.0f,
            i, end);
        value.z = 1.0f;
        fn_800268E0(set->model[0], &value);
    } while (++i <= end);
    fn_80026760(set->model[0], target.x, target.y, target.z);
    fn_80026808(set->model[0], 0.0f, 0.0f,
        0.0f);
    fn_800268B0(set->model[0], 1.0f, 1.0f,
        1.0f);
    fn_80026B8C(set->model[0], 1);
    HuSprGrpDrawNoSet(set->spriteGroup, 64);
}

void fn_1_520C(s16 skipIndex)
{
    MDBANK_ANIM_SET *set = lbl_1_bss_179C;
    float scale;
    s16 j;
    s16 i;

    HuAudFXPlay(MSM_SE_CMN_MODEL_POP_OUT);
    j = 0;
    for (; j < 12; j++) {
        HuPrcVSleep();
        scale = fn_1_11064(1.0f, 0.0f, j,
            10.0f);
        for (i = 0; i < 10; i++) {
            set = &lbl_1_bss_179C[i];
            if (i != skipIndex) {
                set->state[3] = 1;
                fn_800268B0(set->model[0], scale, scale, scale);
                fn_800268B0(set->model[1], scale, scale, scale);
                fn_80026808(set->model[0], 0.0f,
                    1800.0f * scale, 0.0f);
                fn_80026808(set->model[1], 0.0f,
                    1800.0f * scale, 0.0f);
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
    s16 j;
    s16 i;

    HuAudFXPlay(MSM_SE_CMN_MODEL_POP_IN);
    for (j = 0; j < 12; j++) {
        HuPrcVSleep();
        scale = fn_1_11458(0.0f, 1.0f, j,
            10.0f);
        for (i = 0; i < 10; i++) {
            set = &lbl_1_bss_179C[i];
            if (i != skipIndex) {
                set->state[3] = 0;
                fn_800268B0(set->model[0], scale, scale, scale);
                fn_800268B0(set->model[1], scale, scale, scale);
                fn_80026808(set->model[0], 0.0f,
                    1800.0f * scale, 0.0f);
                fn_80026808(set->model[1], 0.0f,
                    1800.0f * scale, 0.0f);
            }
        }
    }
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
        fn_80030D50(set->anim[0], lbl_1_bss_18A0[31]);
        fn_80030D50(set->anim[1], lbl_1_bss_18A0[31]);
        fn_80030D50(set->anim[2], lbl_1_bss_18A0[31]);
        fn_80030D50(set->anim[3], lbl_1_bss_18A0[item->value[4]]);
        if (item->value[7] != -1) {
            Hu3DAnimBankSet(set->anim[3], item->value[7]);
        }
        if (item->value[6] != -1) {
            fn_80030D50(set->anim[4], lbl_1_bss_18A0[item->value[6]]);
            if (item->value[9] != -1) {
                Hu3DAnimBankSet(set->anim[4], item->value[9]);
            }
        } else {
            fn_80030D50(set->anim[4], lbl_1_bss_18A0[0]);
        }
        fn_80030D50(set->anim[5], lbl_1_bss_18A0[0]);
        if (set->state[3] == 0) {
            fn_1_4360(set, item);
        } else {
            fn_1_11880(set->spriteGroup, HUSPR_ATTR_DISPOFF);
        }
        if (item->value[10] != 0) {
            if (item->value[5] != -1) {
                fn_80030D50(set->anim[0], lbl_1_bss_18A0[item->value[5]]);
            } else {
                fn_80030D50(set->anim[0], lbl_1_bss_18A0[item->value[4]]);
            }
            if (item->value[8] != -1) {
                Hu3DAnimBankSet(set->anim[0], item->value[8]);
            }
            if (item->value[6] != -1) {
                fn_80030D50(set->anim[1], lbl_1_bss_18A0[item->value[6]]);
                if (item->value[9] != -1) {
                    Hu3DAnimBankSet(set->anim[1], item->value[9]);
                }
            } else {
                fn_80030D50(set->anim[1], lbl_1_bss_18A0[0]);
            }
            fn_80030D50(set->anim[2], lbl_1_bss_18A0[32]);
            fn_80030D50(set->anim[3], lbl_1_bss_18A0[1]);
            fn_80030D50(set->anim[4], lbl_1_bss_18A0[1]);
            fn_80030D50(set->anim[5], lbl_1_bss_18A0[32]);
            fn_1_11880(set->spriteGroup, HUSPR_ATTR_DISPOFF);
        }
    }
}

void fn_1_5834(void)
{
    OMOBJ *obj = lbl_1_bss_18;
    MDBANK_ANIM_SET *set = lbl_1_bss_179C;
    s16 i;

    for (i = 0; i < 10; i++, set++) {
        fn_800269E8(set->model[0], (1 << 0));
        fn_800269E8(set->model[1], (1 << 0));
        fn_1_11880(set->spriteGroup, HUSPR_ATTR_DISPOFF);
    }
}

void fn_1_58B8(OMOBJ *obj)
{
    MDBANK_ANIM_SET *set;
    HuVecF rot;

    if (obj->work[1] == 1 && ++obj->work[0] > 60) {
        obj->work[0] = 70;
        set = &lbl_1_bss_179C[obj->work[2]];
        fn_80026874(set->model[0], &rot);
        rot.y += 3.0f;
        if (rot.y > 360.0f) {
            rot.y -= 360.0f;
        }
        fn_80026838(set->model[0], &rot);
    }
    fn_1_5550();
}

void fn_1_5988(OMOBJ *obj)
{
    MDBANK_ANIM_SET *set = lbl_1_bss_179C;
    float scale;
    s16 i;

    scale = fn_1_11458(0.0f, 1.0f,
        (float)obj->work[0], (float)obj->work[1]);
    for (i = 0; i < 10; i++, set++) {
        fn_80026A44(set->model[0], (1 << 0));
        fn_80026A44(set->model[1], (1 << 0));
        fn_800268B0(set->model[0], scale, scale, scale);
        fn_800268B0(set->model[1], scale, scale, scale);
        fn_80026808(set->model[0], 0.0f,
            1800.0f * scale, 0.0f);
        fn_80026808(set->model[1], 0.0f,
            1800.0f * scale, 0.0f);
    }
    if (++obj->work[0] <= obj->work[1]) {
        return;
    }
    set = lbl_1_bss_179C;
    for (i = 0; i < 10; i++, set++) {
        fn_80026808(set->model[0], 0.0f,
            0.0f, 0.0f);
        fn_80026808(set->model[1], 0.0f,
            0.0f, 0.0f);
        fn_800268B0(set->model[0], 1.0f,
            1.0f, 1.0f);
        fn_800268B0(set->model[1], 1.0f,
            1.0f, 1.0f);
        set->state[3] = 0;
    }
    obj->work[0] = 0;
    obj->work[1] = 0;
    obj->work[2] = 0;
    obj->work[3] = 0;
    obj->objFunc = fn_1_58B8;
}

void fn_1_5C1C(s16 mode)
{
    OMOBJ *obj = lbl_1_bss_18;
    MDBANK_ANIM_SET *set = lbl_1_bss_179C;
    s16 i;

    for (i = 0; i < 10; i++, set++) {
        fn_80030D50(set->anim[0], lbl_1_bss_18A0[31]);
        fn_80030D50(set->anim[1], lbl_1_bss_18A0[31]);
        fn_80030D50(set->anim[2], lbl_1_bss_18A0[31]);
        fn_80030D50(set->anim[3], lbl_1_bss_18A0[31]);
        fn_80030D50(set->anim[4], lbl_1_bss_18A0[31]);
        fn_80030D50(set->anim[5], lbl_1_bss_18A0[31]);
        fn_800268B0(set->model[0], 0.0f,
            0.0f, 0.0f);
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

inline void fn_1_5C1C(s16 mode);

void fn_1_5D64(OMOBJ *obj)
{
    MDBANK_ANIM_SET *set = lbl_1_bss_179C;
    MDBANK_ANIM_SET *cleanupSet;
    char *name[6] = {
        lbl_1_data_99C,
        lbl_1_data_9A4,
        lbl_1_data_9AC,
        lbl_1_data_9B5,
        lbl_1_data_9BD,
        lbl_1_data_9C5
    };
    OMOBJ *manager;
    s16 i;
    s16 j;
    s16 cleanupIndex;

    for (i = 0; i < 36; i++) {
        lbl_1_bss_18A0[i] = HuSprAnimRead(fn_80008DD0(
            lbl_1_data_19C[i], 0x10000000, 2));
    }

    for (i = 0; i < 10; i++, set++) {
        memset(set, 0, sizeof(*set));
        if (i == 0) {
            set->model[0] = fn_80025A04(fn_80008DD0(
                DATANUM(0x940000, 27), 0x10000000, 2));
            set->model[1] = fn_80025A04(fn_80008DD0(
                DATANUM(0x940000, 26), 0x10000000, 2));
        } else {
            set->model[0] = fn_80025D4C(lbl_1_bss_179C[0].model[0]);
            set->model[1] = fn_80025D4C(lbl_1_bss_179C[0].model[1]);
        }

        fn_80026760(set->model[0],
            (float)((i % 5) * 100 - 200),
            (float)(235 - (i / 5) * 100), 100.0f);
        fn_80026760(set->model[1],
            (float)((i % 5) * 100 - 200),
            (float)(235 - (i / 5) * 100), 100.0f);

        for (j = 0; j < 6; j++) {
            set->anim[j] = fn_80030B2C(lbl_1_bss_18A0[0],
                set->model[0], name[j]);
        }
        set->spriteGroup = HuSprGrpCreate(3);
        for (j = 0; j < 3; j++) {
            set->state[j] = HuSprCreate(lbl_1_bss_1958[4],
                (s16)(j + 100), 0);
            HuSprGrpMemberSet(set->spriteGroup, j, set->state[j]);
            HuSprScaleSet(set->spriteGroup, j, 0.8f,
                0.8f);
            HuSpr3DSet(set->state[j]);
            HuSpr3DFovSet(set->state[j], 0.0f);
        }
        HuSprGrpDrawNoSet(set->spriteGroup, 0x40);
        fn_1_11880(set->spriteGroup, HUSPR_ATTR_DISPOFF);
    }

    manager = lbl_1_bss_18;
    cleanupSet = lbl_1_bss_179C;
    for (cleanupIndex = 0; cleanupIndex < 10; cleanupIndex++, cleanupSet++) {
        fn_800269E8(cleanupSet->model[0], (1 << 0));
        fn_800269E8(cleanupSet->model[1], (1 << 0));
        fn_1_11880(cleanupSet->spriteGroup, HUSPR_ATTR_DISPOFF);
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
                fn_80031010(lbl_1_bss_179C[i].anim[j]);
            }
            fn_800262D0(lbl_1_bss_179C[i].model[0]);
            fn_800262D0(lbl_1_bss_179C[i].model[1]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_6290(HUSPR_GROUPID groupId, s16 baseIndex, s16 value)
{
    s16 i;
    s16 thousands;
    s16 hundreds;
    s16 ones;
    s16 tens;

    if (value >= 1000) {
        for (i = 0; i < 4; i++) {
            HuSprPosSet(groupId, baseIndex + i,
                (float)(i * 22 - 33), 0.0f);
        }
    } else if (value >= 100) {
        for (i = 0; i < 3; i++) {
            HuSprPosSet(groupId, baseIndex + i + 1,
                (float)(i * 22 - 22), 0.0f);
        }
    } else if (value >= 10) {
        for (i = 0; i < 2; i++) {
            HuSprPosSet(groupId, baseIndex + i + 2,
                (float)(i * 22 - 11), 0.0f);
        }
    } else {
        for (i = 0; i < 1; i++) {
            HuSprPosSet(groupId, baseIndex + i + 3,
                (float)(i << 5), 0.0f);
        }
    }

    fn_1_11900(groupId, HUSPR_ATTR_DISPOFF);

    thousands = value / 1000;
    HuSprBankSet(groupId, (s16)baseIndex, thousands);
    if (thousands == 0) {
        HuSprAttrSet(groupId, (s16)baseIndex, HUSPR_ATTR_DISPOFF);
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
    if (tens == 0 && thousands == 0 && hundreds == 0) {
        HuSprAttrSet(groupId, baseIndex + 2, HUSPR_ATTR_DISPOFF);
    }

    ones = value % 10;
    HuSprBankSet(groupId, baseIndex + 3, ones);
}

void fn_1_6600(void)
{
    OMOBJ *obj = lbl_1_bss_1C;
    s16 i;

    for (i = 1; i < 5; i++) {
        fn_800269E8(obj->mdlId[i], (1 << 0));
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
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[10]);
    } else if (lbl_1_bss_1930[0] > 60) {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[9]);
    } else if (lbl_1_bss_1930[0] > 40) {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[8]);
    } else if (lbl_1_bss_1930[0] > 20) {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[7]);
    } else {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[6]);
    }
    for (i = 0; i < 20; i++) {
        if (i <= lbl_1_bss_1930[0] - 1) {
            fn_80026A44(obj->mdlId[i], (1 << 0));
        } else {
            fn_800269E8(obj->mdlId[i], (1 << 0));
        }
    }
}

inline void fn_1_6684(void);

void fn_1_6810(OMOBJ *obj)
{
    HuVecF modelPos;
    HuVecF screenPos;
    HuVecF projectionPos;
    HuVecF *projection;

    lbl_1_bss_204.y = fn_1_111B0(lbl_1_bss_204.y,
        375.0f, 10.0f);

    fn_800267CC(obj->mdlId[5], &modelPos);
    modelPos.y = lbl_1_bss_204.y + fn_1_11384(0.0f,
        10.0f, (float)obj->work[1], 180.0f);
    fn_80026790(obj->mdlId[5], &modelPos);

    projectionPos = modelPos;
    projection = &projectionPos;
    Hu3D3Dto2D(projection, 1, &screenPos);
    HuSprGrpPosSet(lbl_1_bss_194E[2], screenPos.x, screenPos.y);
    HuSprGrpScaleSet(lbl_1_bss_194E[2], 0.8f,
        1.0f);

    fn_1_6684();
    fn_1_6290(lbl_1_bss_194E[2], 0, lbl_1_bss_1930[0]);

    if (++obj->work[1] > 180) {
        obj->work[1] = 0;
    }
}

inline void fn_1_6810(OMOBJ *obj);

void fn_1_6B0C(void)
{
    OMOBJ *obj = lbl_1_bss_1C;

    obj->work[2] = 1;
}

void fn_1_6B34(void)
{
    fn_1_11880(lbl_1_bss_194E[2], HUSPR_ATTR_DISPOFF);
}

void fn_1_6B64(void)
{
    OMOBJ *obj = lbl_1_bss_1C;
    HU3D_MODEL *model = &lbl_802C01C8[obj->mdlId[0]];

    fn_8002CEB0(obj->mdlId[0], obj->mtnId[0]);
    model->motShapeWork.speed = 2.0f;
}

void fn_1_6BDC(void)
{
    OMOBJ *obj = lbl_1_bss_1C;
    HU3D_MODEL *model = &lbl_802C01C8[obj->mdlId[0]];

    fn_8002CEB0(obj->mdlId[0], obj->mtnId[0]);
    model->motShapeWork.speed = -2.0f;
    fn_8002CF98(obj->mdlId[0],
        fn_8002CFE4(obj->mdlId[0]));
}

void fn_1_6C6C(void)
{
    HuVecF pos = { 0.0f, 280.0f, 800.0f };
    MDBANK_MOVE_WORK *work;
    s16 i;

    lbl_1_data_9D0.x = pos.x;
    lbl_1_data_9D0.y = pos.y;
    lbl_1_data_9D0.z = pos.z;
    work = &lbl_1_bss_1C0;
    work->active = 1;
    if (lbl_1_bss_2C == 0) {
        work->time = 0.0f;
        work->duration = 120.0f;
    } else {
        work->time = 0.0f;
        work->duration = 1.0f;
    }
    work->start.x = 0.0f;
    work->start.y = 280.0f;
    work->start.z = 800.0f;
    work->control.x = 0.0f;
    work->control.y = 100.0f;
    work->control.z = 400.0f;
    work->end.x = 0.0f;
    work->end.y = 375.0f;
    work->end.z = -100.0f;
    fn_80026760(lbl_1_bss_1C->mdlId[0], pos.x, pos.y, pos.z);
    for (i = 0; i < 12; i++) {
        fn_80026760(lbl_1_bss_24[0]->mdlId[i], pos.x, pos.y, pos.z);
    }
    fn_1_11980(pos.x, pos.y, pos.z);
    fn_1_12A94(pos.x, pos.y, pos.z);
    for (i = 0; i < 20; i++) {
        fn_80026760(lbl_1_bss_20->mdlId[i],
            pos.x + lbl_1_bss_210[i].x,
            pos.y + lbl_1_bss_210[i].y,
            pos.z + lbl_1_bss_210[i].z);
    }
}

void fn_1_6F00(void)
{
    HuVecF pos = { 0.0f, 375.0f, -100.0f };
    MDBANK_MOVE_WORK *work = &lbl_1_bss_1C0;
    s16 i;

    if (work->active != 0) {
        fn_1_11678(&pos, &work->start, &work->control, &work->end,
            fn_1_11458(0.0f, 1.0f,
                work->time, work->duration));
        fn_80026760(lbl_1_bss_1C->mdlId[0], pos.x, pos.y, pos.z);
        for (i = 0; i < 12; i++) {
            fn_80026760(lbl_1_bss_24[0]->mdlId[i], pos.x, pos.y, pos.z);
        }
        fn_1_11980(pos.x, pos.y, pos.z);
        fn_1_12A94(pos.x, pos.y, pos.z);
        for (i = 0; i < 20; i++) {
            fn_80026760(lbl_1_bss_20->mdlId[i],
                pos.x + lbl_1_bss_210[i].x,
                pos.y + lbl_1_bss_210[i].y,
                pos.z + lbl_1_bss_210[i].z);
        }
        if ((work->time += 1.0f) > work->duration) {
            work->active = 0;
        }
    }
}

inline void fn_1_6F00(void);

void fn_1_70E4(OMOBJ *obj)
{

    fn_1_6F00();

    if (obj->work[2] == 1) {
        fn_1_6810(obj);
    }
}

void fn_1_7594(OMOBJ *obj)
{
    u32 modelData[5] = { DATANUM(0x940000, 10), DATANUM(0x940000, 14), DATANUM(0x940000, 14), DATANUM(0x940000, 14), DATANUM(0x940000, 14) };
    s16 i;
    OMOBJ *shapeObj;
    HU3D_MODEL *shapeModel;
    OMOBJ *spriteOwner;

    omSetStatBit(obj, (1 << 8));
    for (i = 0; i < 1; i++) {
        obj->mdlId[i] = fn_80025A04(
            fn_80008DD0(modelData[i], 0x10000000, 2));
        obj->mtnId[i] = fn_8002D3C4(obj->mdlId[i]);
        fn_80026760(obj->mdlId[i], 0.0f,
            375.0f, -100.0f);
        fn_80026808(obj->mdlId[i], 25.0f,
            0.0f, 0.0f);
        fn_80026B8C(obj->mdlId[0], 3);
        fn_8002CCC0(obj->mdlId[0], obj->mtnId[i],
            0.0f, 0.0f, ((1 << 30) | (1 << 0)));
        lbl_1_bss_200 = fn_80030B2C(lbl_1_bss_1958[9],
            obj->mdlId[0], lbl_1_data_9DC);
    }

    obj->mdlId[5] = fn_80025A04(
        fn_80008DD0(DATANUM(0x940000, 23), 0x10000000, 2));
    obj->mtnId[5] = fn_8002D3C4(obj->mdlId[5]);
    fn_80026760(obj->mdlId[5], 200.0f,
        1000.0f, 100.0f);
    fn_80026808(obj->mdlId[5], 0.0f,
        180.0f, 0.0f);
    fn_800268B0(obj->mdlId[5], 1.5f,
        1.5f, 1.5f);

    lbl_1_bss_204.x = 200.0f;
    if (lbl_1_bss_2C == 0) {
        lbl_1_bss_204.y = 1000.0f;
    } else {
        lbl_1_bss_204.y = 375.0f;
    }
    lbl_1_bss_204.z = 100.0f;
    fn_80026B8C(obj->mdlId[5], 1);
    fn_8002CCC0(obj->mdlId[5], obj->mtnId[5],
        0.0f, 0.0f, ((1 << 30) | (1 << 0)));

    spriteOwner = lbl_1_bss_1C;
    fn_1_11880(lbl_1_bss_194E[2], HUSPR_ATTR_DISPOFF);
    shapeObj = lbl_1_bss_1C;
    shapeModel = &lbl_802C01C8[shapeObj->mdlId[0]];
    fn_8002CEB0(shapeObj->mdlId[0], shapeObj->mtnId[0]);
    shapeModel->motShapeWork.speed = -2.0f;
    fn_8002CF98(shapeObj->mdlId[0],
        fn_8002CFE4(shapeObj->mdlId[0]));
    fn_8002CF98(obj->mdlId[0], 0.0f);
    obj->objFunc = fn_1_70E4;
}

void fn_1_7990(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 6; i++) {
            fn_8002C624(obj->mtnId[i]);
            fn_800262D0(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
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

void fn_1_7A84(OMOBJ *obj)
{
    MDBANK_POST_CALLBACK_WORK *work;
    float opacity;
    s16 i;
    s16 callbackState = obj->work[0];

    work = lbl_1_bss_40;
    for (i = 0; i < 6; i++, work++) {
        if (work->time < 0.0f) {
            fn_800269E8(obj->mdlId[i], (1 << 0));
            fn_800269E8(obj->mdlId[i + 6], (1 << 0));
        } else if (work->time >= 0.0f) {
            if (0.0f == work->time) {
            fn_80026A44(obj->mdlId[i], (1 << 0));
            fn_80026A44(obj->mdlId[i + 6], (1 << 0));
            fn_80026CBC(obj->mdlId[i], 0.0f);
            fn_80026CBC(obj->mdlId[i + 6], 0.0f);
            fn_8002C95C(obj->mdlId[i], obj->mtnId[i]);
            fn_8002C95C(obj->mdlId[i + 6], obj->mtnId[i]);
        } else if (work->time <= 50.0f) {
            fn_80026A44(obj->mdlId[i], (1 << 0));
            fn_80026A44(obj->mdlId[i + 6], (1 << 0));
            opacity = fn_1_1116C(0.0f, 0.8f,
                work->time, 50.0f);
            fn_80026CBC(obj->mdlId[i], opacity);
            fn_80026CBC(obj->mdlId[i + 6], opacity);
        } else if (work->time <= 90.0f) {
            fn_80026A44(obj->mdlId[i], (1 << 0));
            fn_80026A44(obj->mdlId[i + 6], (1 << 0));
            fn_80026CBC(obj->mdlId[i], 0.8f);
            fn_80026CBC(obj->mdlId[i + 6], 0.8f);
        } else {
            fn_80026A44(obj->mdlId[i], (1 << 0));
            fn_80026A44(obj->mdlId[i + 6], (1 << 0));
            opacity = fn_1_1116C(0.8f, 0.0f,
                work->time - 90.0f, 30.0f);
            fn_80026CBC(obj->mdlId[i], opacity);
            fn_80026CBC(obj->mdlId[i + 6], opacity);
        }
        }

        if ((work->time += 1.0f) > work->duration) {
            work->time = -60.0f;
            work->duration = 120.0f;
            fn_800269E8(obj->mdlId[i], (1 << 0));
            fn_800269E8(obj->mdlId[i + 6], (1 << 0));
        }
    }

    if (obj->work[0] == 0) {
        for (i = 0; i < 6; i++) {
            fn_800269E8(obj->mdlId[i], (1 << 0));
        }
    } else {
        for (i = 0; i < 6; i++) {
            fn_800269E8(obj->mdlId[i + 6], (1 << 0));
        }
    }
}

void fn_1_7F2C(OMOBJ *obj)
{
    OMOBJ *callbackObj;
    s16 i;

    omSetStatBit(obj, (1 << 8));
    for (i = 0; i < 6; i++) {
        obj->mdlId[i] = fn_80025A04(fn_80008DD0(
            DATANUM(0x940000, 11) + i, 0x10000000, 2));
        obj->mtnId[i] = fn_8002D3C4(obj->mdlId[i]);
        fn_80026B8C(obj->mdlId[i], 4);
        fn_80026760(obj->mdlId[i], 0.0f,
            375.0f, -100.0f);
        fn_80026808(obj->mdlId[i], 25.0f,
            0.0f, 0.0f);
        fn_8002CCC0(obj->mdlId[i], obj->mtnId[i],
            0.0f, 0.0f, ((1 << 30) | (1 << 0)));
        lbl_1_bss_40[i].time = (float)(90 - i * 30);
        lbl_1_bss_40[i].duration = 120.0f;
    }
    for (i = 6; i < 12; i++) {
        obj->mdlId[i] = fn_80025A04(fn_80008DD0(
            DATANUM(0x940000, 18), 0x10000000, 2));
        obj->mtnId[i] = fn_8002D3C4(obj->mdlId[i]);
        fn_80026B8C(obj->mdlId[i], 4);
        fn_80026760(obj->mdlId[i], 0.0f,
            375.0f, -100.0f);
        fn_80026808(obj->mdlId[i], 25.0f,
            0.0f, 0.0f);
        fn_8002CCC0(obj->mdlId[i], obj->mtnId[i],
            0.0f, 0.0f, ((1 << 30) | (1 << 0)));
        fn_800269E8(obj->mdlId[i], (1 << 0));
    }
    callbackObj = lbl_1_bss_24[0];
    callbackObj->work[0] = 1;
    fn_1_13764(0);
    obj->objFunc = fn_1_7A84;
}

void fn_1_8280(void)
{
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

void fn_1_8340(void)
{
    s16 i;
    for (i = 0; i < 4; i++) {
        HuWinDispOff(lbl_1_bss_198C[i]);
    }
    HuSprPriSet(lbl_1_bss_194E[3], 0, 5500);
    fn_1_11900(lbl_1_bss_194E[3], 4);
    HuPrcSleep(5);
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
    lbl_80265E30.bankStar = lbl_1_bss_1930[0];
    for (i = 0; i < 55; i++) {
        if (lbl_1_data_22C[i].value[10] == 1) {
            fn_8003D27C(lbl_1_data_22C[i].value[2]);
        }
    }
    if (fn_8003D2F0(6)) {
        fn_8003CCAC(677);
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
            fn_8003A5E4(1, TRUE);
            break;
        case 1: {
            OMOVLHIS *history = omOvlHisGet(0);

            omOvlHisChg(0, history->ovl, 1, 1);
            omOvlCall(DLL_miraclebookdll, 0, 0);
            break;
        }
        case 2: {
            OMOVLHIS *history = omOvlHisGet(0);

            omOvlHisChg(0, history->ovl, 1, 2);
            omOvlCall(DLL_staffdll, 0, 0);
            break;
        }
    }
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

void fn_1_86C0(void)
{
    MDBANK_CAMERA_WORK *camera;
    MDBANK_SPRITE_CONFIG *config;
    s16 i;
    s16 flagIndex;
    s16 windowIndex;

    lbl_1_bss_0 = fn_8003A8BC(0x1B, 0x2000);
    fn_80069F1C(lbl_1_bss_0);

    camera = &lbl_1_bss_1998;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f,
        10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
        640.0f, 480.0f, 0.0f,
        1.0f);

    memset(camera, 0, sizeof(*camera));
    camera->callback = fn_1_9380;
    camera->center.x = 0.0f;
    camera->center.y = 65.0f;
    camera->center.z = -800.0f;
    camera->rot.x = -7.25f;
    camera->rot.y = 0.0f;
    camera->rot.z = 0.0f;
    if (lbl_1_bss_2C == 0) {
        camera->zoom = 2650.0f;
    } else {
        camera->zoom = 2150.0f;
    }
    camera->obj = fn_8003AB38(lbl_1_bss_0, 0x100, 0, 0, -1,
        fn_1_4B8);

    fn_1_780();

    HuWinInit(1);
    lbl_1_bss_198C[0] = HuWinExCreateFrame(
        16.0f, 337.0f, 0x220, 0x2A, -1, 0);
    HuWinDispOff(lbl_1_bss_198C[0]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[0], 0.0f);
    lbl_1_bss_198C[1] = HuWinExCreateFrame(
        16.0f, 372.0f, 0x220, 0x44, -1, 5);
    HuWinDispOff(lbl_1_bss_198C[1]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[1], 0.9f);
    lbl_1_bss_198C[2] = HuWinExCreateFrame(
        16.0f, 372.0f, 0x220, 0x44, -1, 3);
    HuWinDispOff(lbl_1_bss_198C[2]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[2], 0.9f);
    lbl_1_bss_198C[3] = HuWinExCreateFrame(
        16.0f, 372.0f, 0x220, 0x44, -1, 4);
    HuWinDispOff(lbl_1_bss_198C[3]);
    HuWinBGTPLvlSet(lbl_1_bss_198C[3], 0.9f);

    for (windowIndex = 0; windowIndex < 4; windowIndex++) {
        winData[lbl_1_bss_198C[windowIndex]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_198C[windowIndex], (HUWIN_CALLBACK)fn_1_0);
    }

    fn_1_8F0();

    config = lbl_1_data_3C;
    for (i = 0; i < 12; i++) {
        lbl_1_bss_1958[i] = HuSprAnimRead(fn_80008DD0(
            lbl_1_data_4[i], 0x10000000, 2));
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

    lbl_1_bss_1930[0] = (s16)fn_8003D248();
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

    for (flagIndex = 0; flagIndex < 55; flagIndex++) {
        lbl_1_data_22C[flagIndex].value[10] = 0;
        if (fn_8003D2F0(lbl_1_data_22C[flagIndex].value[2])) {
            lbl_1_data_22C[flagIndex].value[10] = 1;
        }
    }

    fn_1_12EA8();
    lbl_1_bss_C = fn_8003AB38(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_1FD8);
    lbl_1_bss_10 = fn_8003AB38(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_2468);
    lbl_1_bss_4 = fn_8003AB38(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_28B4);
    lbl_1_bss_8 = fn_8003AB38(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_2B34);
    lbl_1_bss_14 = fn_8003AB38(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_3468);
    lbl_1_bss_18 = fn_8003AB38(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_5D64);
    lbl_1_bss_1C = fn_8003AB38(lbl_1_bss_0, 0x1000, 0x10, 0x10,
        -1, fn_1_7594);
    lbl_1_bss_20 = fn_8003AB38(lbl_1_bss_0, 0x1000, 0x60, 0x1,
        -1, fn_1_3E30);
    lbl_1_bss_24[0] = fn_8003AB38(lbl_1_bss_0, 0x1000, 0x0A, 0x0A,
        -1, fn_1_7F2C);
    fn_8000DAC0(fn_1_83CC, 0x3000, 0x3000, 0,
        lbl_1_bss_0);
}

void ObjectSetup(void)
{
    OSReport("\n-----===== MARIO PARTY 6 :: STAR BANK =====-----\n\n");
    lbl_1_bss_2C = lbl_802C0248;
    lbl_1_bss_30 = lbl_802C0240;
    fn_1_86C0();
}

int _prolog(void)
{
    extern void ObjectSetup(void);

    const MDBANK_VOID_FUNC *ctor = _ctors;

    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
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

void fn_1_92A4(void)
{
    if (lbl_1_bss_1930[0] > 80) {
        lbl_1_data_0 = HuAudFXPlay(MSM_SE_BANK_BALANCE_LOOP_4);
    } else if (lbl_1_bss_1930[0] > 60) {
        lbl_1_data_0 = HuAudFXPlay(MSM_SE_BANK_BALANCE_LOOP_3);
    } else if (lbl_1_bss_1930[0] > 40) {
        lbl_1_data_0 = HuAudFXPlay(MSM_SE_BANK_BALANCE_LOOP_2);
    } else if (lbl_1_bss_1930[0] > 20) {
        lbl_1_data_0 = HuAudFXPlay(MSM_SE_BANK_BALANCE_LOOP_1);
    } else {
        HuAudFXStop(lbl_1_data_0);
    }
}

inline void fn_1_92A4(void);

void fn_1_9380(OMOBJ *obj, MDBANK_CAMERA_WORK *camera)
{
    camera->centerTarget.x = 0.0f;
    camera->centerTarget.y = 65.0f;
    camera->centerTarget.z = -800.0f;
    camera->rotTarget.x = -7.25f;
    camera->rotTarget.y = 0.0f;
    camera->rotTarget.z = 0.0f;
    camera->zoomTarget = 2150.0f;
    fn_1_111F8(&camera->center, &camera->centerTarget, 15.0f);
    fn_1_111F8(&camera->rot, &camera->rotTarget, 15.0f);
    camera->zoom = fn_1_111B0(camera->zoom, camera->zoomTarget,
        15.0f);
}

s32 fn_1_9464(void)
{
    OMOBJ *obj;
    s16 i;

    HuPrcSleep(5);
    obj = lbl_1_bss_20;
    if (lbl_1_bss_1930[0] > 80) {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[10]);
    } else if (lbl_1_bss_1930[0] > 60) {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[9]);
    } else if (lbl_1_bss_1930[0] > 40) {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[8]);
    } else if (lbl_1_bss_1930[0] > 20) {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[7]);
    } else {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[6]);
    }
    for (i = 0; i < 20; i++) {
        if (i <= lbl_1_bss_1930[0] - 1) {
            fn_80026A44(obj->mdlId[i], (1 << 0));
        } else {
            fn_800269E8(obj->mdlId[i], (1 << 0));
        }
    }
    fn_1_6C6C();
    if (lbl_1_bss_2C == 1) {
        OMOBJ *shapeObj = lbl_1_bss_1C;

        shapeObj->work[2] = 1;
        fn_1_5C1C(1);
    }
    lbl_1_bss_1988 = HuAudSStreamPlay(MSM_STREAM_STAR_GATHERING);
    HuAudFXPlay(MSM_SE_BANK_DEPOSIT_DONE);
    fn_1_92A4();
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    return 1;
}

s32 fn_1_981C(void)
{
    OMOBJ *obj;

    fn_1_1C64();
    fn_1_1D68();
    fn_1_12F8(1, MESSNUM(MESS_SBANK_MAIN_MENU, 0), 1);
    fn_1_11A0();
    fn_1_1C64();
    fn_1_12F8(3, MESSNUM(MESS_SBANK_MAIN_MENU, 1), 1);
    fn_1_11A0();
    obj = lbl_1_bss_4;
    obj->objFunc = fn_1_2750;
    return 1;
}

s32 fn_1_9DE8(void)
{
    fn_1_12F8(2, MESSNUM(MESS_SBANK_MAIN_MENU, 2), 1);
    fn_1_11A0();
    fn_8002CCC0(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[2], 0.0f, 30.0f, 0);
    fn_8002CCC0(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[2], 0.0f, 30.0f, 0);
    HuPrcSleep(0x2D);
    fn_1_1EE8();
    fn_1_2378();
    HuPrcSleep(0x1E);
    fn_1_6B0C();
    fn_1_5C1C(0);
    HuPrcSleep(0x1E);
    fn_8002CCC0(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[0], 0.0f, 30.0f, ((1 << 30) | (1 << 0)));
    fn_8002CCC0(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[0], 0.0f, 30.0f, ((1 << 30) | (1 << 0)));
    HuPrcSleep(0x3C);
    return 1;
}

s16 fn_1_A438(void)
{
    fn_1_1D68();
    fn_1_12F8(2, MESSNUM(MESS_SBANK_MAIN_MENU, 9), 1);
    return fn_1_1200(2);
}

void fn_1_A750(s16 index)
{
    HuAudFXPlay(MSM_SE_BANK_DEPOSIT_START);
    fn_1_6B64();
    HuPrcSleep(30);
    fn_1_7A20(0);
    HuAudFXPlay(MSM_SE_BANK_DEPOSIT_DROP);
    fn_1_6668();
    fn_1_3DBC(index, lbl_1_data_22C[index].value[3]);
    do {
        HuPrcVSleep();
    } while (lbl_1_bss_20->objFunc != NULL);
    fn_1_92A4();
    fn_1_7A20(1);
    fn_1_6668();
    HuPrcSleep(30);
    fn_1_6BDC();
    HuAudFXPlay(MSM_SE_BANK_DEPOSIT_DONE);
}

s16 fn_1_AA08(void)
{
    s16 result = 0;
    s16 index = 0;
    s16 time = 0;
    HuVecF pos;
    HuVecF cursorPos = { 0.0f, 185.0f, 100.0f };
    char text[12];
    HuVecF effectPos;

    while (TRUE) {
        HuPrcVSleep();
        fn_1_1764(MESSNUM(MESS_SYS_GUIDE, 10));
        fn_1_2D98(2, &cursorPos, 0.0f, 90.0f);
        fn_1_2D98(3, &cursorPos, 0.0f, -90.0f);
        if (lbl_1_bss_1930[1] == 0) {
            fn_1_2EFC(3);
        }
        if (lbl_1_bss_1930[1] == 9) {
            fn_1_2EFC(2);
        }
        index = lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5;
        fn_800267CC(lbl_1_bss_179C[index].model[0], &pos);
        fn_1_3164(&pos);
        time = 0;
        while (TRUE) {
            HuPrcVSleep();
            index = lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5 + lbl_1_bss_1930[1] * 5;
            if (index >= 55) {
                index -= 55;
            }
            if (lbl_1_data_22C[index].value[10] == 0) {
                fn_1_1540(2, lbl_1_data_22C[index].choice.nameMessage, 0);
                fn_1_12F8(2, MESSNUM(MESS_SBANK_MAIN_MENU, 4), 0);
            } else {
                fn_1_1540(2, lbl_1_data_22C[index].choice.nameMessage, 0);
                fn_1_12F8(2, MESSNUM(MESS_SBANK_MAIN_MENU, 4), 0);
            }
            time++;
            if (time > 60) {
                time = 60;
            }
            if (time > 10) {
                if (lbl_802BFFE0[0] & 8) {
                    if (lbl_1_bss_1930[3] == 1 || lbl_1_bss_1930[1] > 0) {
                        HuAudFXPlay(MSM_SE_CMN_CURSOR_MOVE);
                    }
                    if (lbl_1_bss_1930[3] == 1 || lbl_1_bss_1930[1] > 0) {
                        time = 0;
                        fn_1_4730();
                    }
                    if (lbl_1_bss_1930[3] == 0) {
                        fn_1_2D98(2, &cursorPos, 0.0f, 90.0f);
                        lbl_1_bss_1930[1]--;
                        if (lbl_1_bss_1930[1] <= 0) {
                            fn_1_2EFC(3);
                            lbl_1_bss_1930[1] = 0;
                        }
                        fn_1_2D70(3);
                    } else {
                        lbl_1_bss_1930[3] = 0;
                    }
                } else if (lbl_802BFFE0[0] & 4) {
                    if (lbl_1_bss_1930[3] == 0 || lbl_1_bss_1930[1] < 9) {
                        HuAudFXPlay(MSM_SE_CMN_CURSOR_MOVE);
                    }
                    if (lbl_1_bss_1930[3] == 0 || lbl_1_bss_1930[1] < 9) {
                        time = 0;
                        fn_1_4730();
                    }
                    if (lbl_1_bss_1930[3] == 1) {
                        fn_1_2D98(3, &cursorPos, 0.0f, -90.0f);
                        lbl_1_bss_1930[1]++;
                        if (lbl_1_bss_1930[1] >= 9) {
                            fn_1_2EFC(2);
                            lbl_1_bss_1930[1] = 9;
                        }
                        fn_1_2D70(2);
                    } else {
                        lbl_1_bss_1930[3] = 1;
                    }
                } else if (lbl_802BFFE0[0] & 1) {
                    HuAudFXPlay(MSM_SE_CMN_CURSOR_MOVE);
                    lbl_1_bss_1930[2]--;
                    if (lbl_1_bss_1930[2] < 0) {
                        lbl_1_bss_1930[2] += 5;
                    }
                    time = 0;
                    fn_1_4730();
                } else if (lbl_802BFFE0[0] & 2) {
                    HuAudFXPlay(MSM_SE_CMN_CURSOR_MOVE);
                    lbl_1_bss_1930[2]++;
                    if (lbl_1_bss_1930[2] > 4) {
                        lbl_1_bss_1930[2] -= 5;
                    }
                    time = 0;
                    fn_1_4730();
                } else if (lbl_802C0008[0] & 0x200) {
                    HuAudFXPlay(MSM_SE_CMN_CHOICE_CANCEL);
                    result = -1;
                    break;
                } else if (lbl_802C0008[0] & 0x400) {
                    HuAudFXPlay(MSM_SE_CMN_CHOICE_CANCEL);
                    result = -1;
                    break;
                } else if (lbl_802C0008[0] & 0x100) {
                    HuAudFXPlay(MSM_SE_CMN_MENU_ADVANCE);
                    result = 1;
                    break;
                }
            }
            if (time >= 30) {
                index = lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5;
                fn_1_47AC(index);
            }
            index = lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5;
            fn_800267CC(lbl_1_bss_179C[index].model[0], &pos);
            fn_1_3100(&pos);
        }
        fn_1_4730();
        fn_1_18E8();
        fn_1_2EFC(2);
        fn_1_2EFC(3);
        fn_1_3280();
        if (result == -1) {
            if (!fn_1_A438()) {
                result = -1;
                break;
            }
            continue;
        }
        fn_1_48CC(lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5);
        index = lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5 + lbl_1_bss_1930[1] * 5;
        if (index >= 55) {
            index -= 55;
        }
        if (lbl_1_data_22C[index].value[10] == 0) {
            fn_1_4C64(lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5);
            if (lbl_1_bss_1930[0] >= lbl_1_data_22C[index].value[3]) {
                fn_1_1C64();
                sprintf(text, "%d", lbl_1_data_22C[index].value[3]);
                fn_1_1540(3, (s32)text, 0);
                fn_1_12F8(3, MESSNUM(MESS_SBANK_MAIN_MENU, 5), 1);
                if (fn_1_1200(2) == 0) {
                    if (fn_1_EEB8(lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5 + lbl_1_bss_1930[1] * 5) == 1) {
                        fn_1_520C(lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5);
                        fn_1_1C64();
                        fn_1_12F8(3, MESSNUM(MESS_SBANK_MAIN_MENU, 7), 1);
                        fn_1_11A0();
                        fn_1_4C9C();
                        fn_1_A750(index);
                        HuAudFXPlay(MSM_SE_BANK_PURCHASE_SPARKLE);
                        fn_800267CC(lbl_1_bss_179C[lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5].model[0],
                            &effectPos);
                        fn_1_13598(&effectPos);
                        HuPrcSleep(30);
                        lbl_1_data_22C[index].value[10] = 1;
                        HuPrcSleep(30);
                        fn_1_1C64();
                        fn_1_1540(3, lbl_1_data_22C[index].choice.nameMessage, 0);
                        fn_1_12F8(3, MESSNUM(MESS_SBANK_MAIN_MENU, 8), 1);
                        fn_1_11A0();
                        fn_1_1050C(index);
                        fn_1_53AC(lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5);
                    }
                }
            } else {
                fn_1_1C64();
                fn_1_12F8(3, MESSNUM(MESS_SBANK_MAIN_MENU, 6), 1);
                fn_1_11A0();
            }
        } else {
            index = lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5 + lbl_1_bss_1930[1] * 5;
            if (index >= 55) {
                index -= 55;
            }
            result = fn_1_F7C0(index);
            if (result == 1) {
                fn_1_1D68();
                fn_1_12F8(2, lbl_1_data_22C[index].choice.message, 1);
                fn_1_11A0();
            } else if (result) {
                return result;
            }
        }
        fn_1_4DFC(lbl_1_bss_1930[2] + lbl_1_bss_1930[3] * 5);
    }
    return result;
}

s32 lbl_1_data_A20[2] = { DATANUM(0x940000, 73), DATANUM(0x940000, 74) };

short fn_1_EA98(s32 unused0, s32 unused1)
{
    OMOBJ *manager;
    OMOBJ *obj;
    MDBANK_ANIM_SET *set;
    s16 result = 0;
    OMOBJ *shapeObj;
    s16 i;
    s16 j;

    HuPrcSleep(5);

    manager = lbl_1_bss_20;
    if (lbl_1_bss_1930[0] > 80) {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[10]);
    } else if (lbl_1_bss_1930[0] > 60) {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[9]);
    } else if (lbl_1_bss_1930[0] > 40) {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[8]);
    } else if (lbl_1_bss_1930[0] > 20) {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[7]);
    } else {
        fn_80030D50(lbl_1_bss_200, lbl_1_bss_1958[6]);
    }
    for (i = 0; i < 20; i++) {
        if (i <= lbl_1_bss_1930[0] - 1) {
            fn_80026A44(manager->mdlId[i], (1 << 0));
        } else {
            fn_800269E8(manager->mdlId[i], (1 << 0));
        }
    }
    fn_1_6C6C();

    if (lbl_1_bss_2C == 1) {
        shapeObj = lbl_1_bss_1C;
        shapeObj->work[2] = 1;
        obj = lbl_1_bss_18;
        set = lbl_1_bss_179C;
        for (j = 0; j < 10; j++, set++) {
            fn_80030D50(set->anim[0], lbl_1_bss_18A0[31]);
            fn_80030D50(set->anim[1], lbl_1_bss_18A0[31]);
            fn_80030D50(set->anim[2], lbl_1_bss_18A0[31]);
            fn_80030D50(set->anim[3], lbl_1_bss_18A0[31]);
            fn_80030D50(set->anim[4], lbl_1_bss_18A0[31]);
            fn_80030D50(set->anim[5], lbl_1_bss_18A0[31]);
            fn_800268B0(set->model[0], 0.0f,
                0.0f, 0.0f);
        }
        obj->work[0] = 0;
        obj->work[1] = 1;
        obj->objFunc = fn_1_5988;
    }

    lbl_1_bss_1988 = HuAudSStreamPlay(MSM_STREAM_STAR_GATHERING);
    HuAudFXPlay(MSM_SE_BANK_DEPOSIT_DONE);
    if (lbl_1_bss_1930[0] > 80) {
        lbl_1_data_0 = HuAudFXPlay(MSM_SE_BANK_BALANCE_LOOP_4);
    } else if (lbl_1_bss_1930[0] > 60) {
        lbl_1_data_0 = HuAudFXPlay(MSM_SE_BANK_BALANCE_LOOP_3);
    } else if (lbl_1_bss_1930[0] > 40) {
        lbl_1_data_0 = HuAudFXPlay(MSM_SE_BANK_BALANCE_LOOP_2);
    } else if (lbl_1_bss_1930[0] > 20) {
        lbl_1_data_0 = HuAudFXPlay(MSM_SE_BANK_BALANCE_LOOP_1);
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
        if (fn_8003CC40(678) == 0) {
            fn_1_1C64();
            fn_1_12F8(3, MDBANK_ITEM_MESSAGE_M678_UNLOCK_REQUIRED, 1);
            fn_1_11A0();
            return 0;
        }
        return 1;
    }
    if (lbl_1_data_22C[index].value[0] == 3) {
        if (fn_8003CC40(679) == 0) {
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
        if (fn_8003D2F0(60) == 0) {
            fn_1_1D68();
            fn_1_12F8(2, MDBANK_ITEM_MESSAGE_BANK_FLAG_ENABLE_CONFIRM, 1);
            if (fn_1_1200(2) == 0) {
                fn_8003D27C(60);
            }
        } else {
            fn_1_1D68();
            fn_1_12F8(2, MDBANK_ITEM_MESSAGE_BANK_FLAG_DISABLE_CONFIRM, 1);
            if (fn_1_1200(2) == 0) {
                fn_8003D2B4(60);
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

ANIMDATA *lbl_1_bss_19F0[2];
s16 lbl_1_bss_19EC;
s16 lbl_1_bss_19EA;
s16 lbl_1_bss_19E8;
MDBANK_CAMERA_WORK lbl_1_bss_1998;
s16 lbl_1_bss_1994[2];
s16 lbl_1_bss_198C[4];
s32 lbl_1_bss_1988;
ANIMDATA *lbl_1_bss_1958[12];
HUSPR_GROUPID lbl_1_bss_194E[5];
HUSPRID lbl_1_bss_1938[11];
s16 lbl_1_bss_1930[4];
ANIMDATA *lbl_1_bss_18A0[36];
MDBANK_ANIM_SET lbl_1_bss_179C[10];
MDBANK_MOVE_WORK lbl_1_bss_175C;
MDBANK_MOVE_WORK lbl_1_bss_171C;
HuVecF lbl_1_bss_1710;
float lbl_1_bss_1700[4];
MDBANK_EVENT_RECORD lbl_1_bss_300[80];
HuVecF lbl_1_bss_210[20];
HuVecF lbl_1_bss_204;
s16 lbl_1_bss_200;
MDBANK_MOVE_WORK lbl_1_bss_1C0;
MDBANK_POST_CALLBACK_WORK lbl_1_bss_40[6];
s16 lbl_1_bss_34[6];
u32 lbl_1_bss_30;
s32 lbl_1_bss_2C;
OMOBJ *lbl_1_bss_24[2];
OMOBJ *lbl_1_bss_20;
OMOBJ *lbl_1_bss_1C;
OMOBJ *lbl_1_bss_18;
OMOBJ *lbl_1_bss_14;
OMOBJ *lbl_1_bss_10;
OMOBJ *lbl_1_bss_C;
OMOBJ *lbl_1_bss_8;
OMOBJ *lbl_1_bss_4;
OMOBJMAN *lbl_1_bss_0;
