#include <dolphin/mtx/GeoTypes.h>

#define HUWIN_ATTR_NOCANCEL (1 << 4)
#define HUWIN_ATTR_ALIGN_CENTER (1 << 11)

typedef s16 HUWINID;

typedef struct MdbankObject {
    u16 stat;
    s16 objNext;
    s16 prio;
    s16 prev;
    s16 next;
    s16 nextNo;
    s16 grpNo;
    u16 memberNo;
    u32 mode;
    void (*objFunc)(struct MdbankObject *obj);
    Vec trans;
    Vec rot;
    Vec scale;
    u16 modelCount;
    s16 *mdlId;
    u16 motionCount;
    s16 *mtnId;
    u32 work[4];
} MDBANK_OBJECT;

typedef MDBANK_OBJECT OMOBJ;

typedef union MdbankItem {
    s16 value[16];
    struct {
        s16 value[14];
        s32 message;
    } choice;
} MDBANK_ITEM;

extern u32 lbl_1_data_90C;
extern MDBANK_ITEM lbl_1_data_22C[55];
extern s16 lbl_1_data_932[3];
extern s32 lbl_1_data_938[3];
extern s16 lbl_1_bss_198C[4];
extern MDBANK_OBJECT *lbl_1_bss_C;
extern MDBANK_OBJECT *lbl_1_bss_10;
extern const float lbl_1_rodata_68;
extern const float lbl_1_rodata_F8;

void Hu3DMotionShiftSet(s16 modelId, s16 motionId, float start, float end,
    u32 attr);
void HuWinHomeClear(HUWINID winId);
void HuWinAttrSet(HUWINID winId, u32 attr);
void HuWinAttrReset(HUWINID winId, u32 attr);
void HuWinMesSpeedSet(HUWINID winId, s16 mesSpeed);
void HuWinMesSet(HUWINID winId, u32 messNum);
s16 HuWinChoiceGet(HUWINID winId, s16 choiceNo);
void HuWinMesWait(HUWINID winId);
void HuWinDispOff(HUWINID winId);
void HuWinDispOn(HUWINID winId);
void HuWinExOpen(HUWINID winId);
void HuWinExClose(HUWINID winId);
BOOL GWMgUnlockGet(s32 minigameId);
void GWBankFlagSet(s32 flag);
void GWBankFlagReset(s32 flag);
BOOL GWBankFlagGet(s32 flag);

void fn_1_1BD4(MDBANK_OBJECT *obj);
void fn_1_1CD8(MDBANK_OBJECT *obj);

static inline void fn_1_9A8(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_198C[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_198C[winNo]);
    }
}

static inline void fn_1_A18(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_198C[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_198C[winNo]);
    }
}

static inline void fn_1_A88(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_198C[winNo]);
}

static inline s16 fn_1_AC4(s16 winNo, s16 mode)
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

static inline void fn_1_B98(s16 winNo, u32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_198C[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_198C[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_198C[winNo], speed);
    if (lbl_1_data_90C != messNum) {
        lbl_1_data_90C = -1;
    }
}

static inline void fn_1_F68(s16 winNo)
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

static inline void fn_1_11A0(void)
{
    if (lbl_1_data_932[0] != -1) {
        fn_1_A88(lbl_1_data_932[0]);
    }
}

static inline s16 fn_1_1200(s16 mode)
{
    if (lbl_1_data_932[0] != -1) {
        return fn_1_AC4(lbl_1_data_932[0], mode);
    }
    return 0;
}

static inline void fn_1_12F8(s16 winNo, s32 messNum, s16 speed)
{
    fn_1_F68(winNo);
    if (lbl_1_data_938[0] != messNum) {
        lbl_1_data_938[0] = messNum;
        lbl_1_data_938[1] = -1;
        fn_1_B98(lbl_1_data_932[0], lbl_1_data_938[0], speed);
    }
}

static inline void fn_1_1C64(void)
{
    OMOBJ *obj = lbl_1_bss_C;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1], lbl_1_rodata_68,
        lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_1BD4;
}

static inline void fn_1_1D68(void)
{
    OMOBJ *obj = lbl_1_bss_10;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1], lbl_1_rodata_68,
        lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_1CD8;
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
