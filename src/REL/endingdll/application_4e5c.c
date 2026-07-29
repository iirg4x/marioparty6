#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HUWINID;
typedef s16 HUSPRID;
typedef s16 HUSPR_GROUPID;
typedef struct AnimData_s ANIMDATA;
typedef struct Process_s OMOBJMAN;
typedef struct Process_s HUPROCESS;
typedef struct omObj_s OMOBJ;
typedef void (*OMOBJ_FUNC)(OMOBJ *object);
typedef void (*HUWIN_CALLBACK)(HUWINID window, u32 message, char character);

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

typedef struct EndingVec2 {
    float x;
    float y;
} EndingVec2;

typedef struct EndingWinCharEntry {
    u8 color;
    u8 fade;
    s16 x;
    s16 y;
    s16 character;
} EndingWinCharEntry;

typedef struct EndingWinChoice {
    u8 stat;
    s16 x;
    s16 y;
} EndingWinChoice;

typedef struct EndingWinWarning {
    HUWINID window;
    u8 unk_02[4];
    s16 scaleTimer[2];
    ANIMDATA *glowAnim;
    ANIMDATA *bgAnim[2];
    HUPROCESS *process;
} EndingWinWarning;

typedef struct EndingWindow {
    u8 stat;
    u8 padMask;
    u8 disablePlayer;
    u8 bgPalNum;
    HUSPR_GROUPID group;
    HUSPRID sprite[31];
    s16 messageSpeed;
    s16 messageTime;
    s16 keyWaitSprite;
    s16 priority;
    s16 drawNo;
    u32 attr;
    ANIMDATA *frameAnim[2];
    s16 messageRectX;
    s16 messageRectW;
    s16 messageRectY;
    s16 messageRectH;
    s16 messageX;
    s16 messageY;
    s16 messageColor;
    s16 messageShadowColor;
    s16 charPadX;
    s16 charPadY;
    s16 width;
    s16 height;
    EndingVec2 position;
    EndingVec2 scale;
    float rotation;
    s16 charEntryNum;
    s16 charEntryMax;
    EndingWinCharEntry *charEntry;
    s16 messageSp;
    s32 unk_94;
    char *messageData;
    char *messageStack[16];
    char *messageInsert[16];
    char *messageCopy;
    s16 choiceNum;
    s16 choice;
    s16 cursorSprite;
    u8 choiceDisable[16];
    EndingWinChoice choiceData[16];
    s16 scissorX;
    s16 scissorY;
    s16 scissorW;
    s16 scissorH;
    s16 tabW;
    s16 pushKey;
    s16 activePadKey;
    s16 choiceEndSound;
    u8 ATTRIBUTE_ALIGN(32) messagePalette[10][3];
    EndingWinWarning *warning;
    HUWIN_CALLBACK callback;
    u32 originalMessage;
} EndingWindow;

typedef struct EndingGwCommonPrefix {
    char magic[4];
    u16 unk_04;
    u8 language : 3;
    u8 outputMode : 2;
    u8 vibrate : 1;
    u8 microphone : 2;
    s64 time;
    char name[17];
    u32 minigameUnlock[4];
    u32 record[8];
    u16 characterPlayCount[11][14];
    u16 boardPlayCount[11];
    u16 boardMaxStar[11][14];
    u16 boardMaxCoin[11][14];
    u8 singleMinigameWins[4];
    u8 singleBoardPlayCount[3];
    u64 singleBoardFlags[3];
    u8 saveEnable : 1;
    u8 map7Unlock : 1;
    u8 veryHardUnlock : 1;
    u8 m562VeryHardUnlock : 1;
    u8 : 1;
    u8 unkFlag4 : 1;
    u8 viewOpening : 1;
    u8 viewEnding : 1;
} EndingGwCommonPrefix;

typedef struct EndingWindowPlayers {
    s16 player[4];
} EndingWindowPlayers;

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_14;
extern OMOBJ *lbl_1_bss_18;
extern OMOBJ *lbl_1_bss_1C;
extern OMOBJ *lbl_1_bss_20;
extern HUWINID lbl_1_bss_1A48[5];

extern float lbl_1_rodata_78;
extern float lbl_1_rodata_C8;
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
extern char lbl_1_data_DD[];

extern EndingWindow winData[32];
extern EndingGwCommonPrefix GwCommon;

void OSReport(char *format, ...);
OMOBJMAN *omInitObjMan(s16 objectMax, s32 priority);
OMOBJ *omAddObjEx(OMOBJMAN *manager, s16 priority, u16 modelCount,
    u16 motionCount, s16 group, OMOBJ_FUNC function);
void Hu3DCameraCreate(s16 camera);
void Hu3DShadowCreate(float fov, float near, float far);
void Hu3DShadowPosSet(HuVecF *position, HuVecF *up, HuVecF *target);
void HuWinInit(s32 messageData);
HUWINID HuWinExCreateFrame(float x, float y, s16 width, s16 height,
    s16 speaker, s16 frame);
void HuWinDispOff(HUWINID window);
void HuWinBGTPLvlSet(HUWINID window, float level);
void HuWinCallbackSet(HUWINID window, HUWIN_CALLBACK callback);
HUPROCESS *HuPrcChildCreate(void (*function)(void), u16 priority,
    u32 stackSize, s32 extraSize, HUPROCESS *parent);
void fn_1_98(HUWINID window, u32 message, char character);
void fn_1_11F34(void);
void fn_1_189C(OMOBJ *object);
void fn_1_23D8(OMOBJ *object);
void fn_1_3B4C(OMOBJ *object);
void fn_1_3D5C(OMOBJ *object);
void fn_1_45C0(OMOBJ *object);
void fn_1_4BC8(OMOBJ *object);
void fn_1_4DAC(void);

static inline void fn_1_76C(void)
{
    Hu3DCameraCreate(1);
}

static inline void fn_1_B2C(void)
{
    HuVecF position = lbl_1_rodata_D4;
    HuVecF up = lbl_1_rodata_E0;
    HuVecF target = lbl_1_rodata_EC;

    Hu3DShadowCreate(lbl_1_rodata_F8, lbl_1_rodata_78, lbl_1_rodata_FC);
    Hu3DShadowPosSet(&position, &up, &target);
}

static inline void fn_1_F34(void)
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

void fn_1_4E5C(void)
{
    lbl_1_bss_0 = omInitObjMan(11, 0x2000);
    fn_1_76C();
    fn_1_B2C();
    fn_1_F34();
    fn_1_11F34();

    lbl_1_bss_4 = omAddObjEx(lbl_1_bss_0, 0x1000, 16, 16, -1,
        fn_1_189C);
    lbl_1_bss_8 = omAddObjEx(lbl_1_bss_0, 0x1000, 16, 96, -1,
        fn_1_23D8);
    lbl_1_bss_14 = omAddObjEx(lbl_1_bss_0, 0x1000, 16, 16, -1, NULL);
    lbl_1_bss_C = omAddObjEx(lbl_1_bss_0, 0x1000, 16, 16, -1,
        fn_1_3B4C);
    lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 0x1000, 16, 16, -1,
        fn_1_3D5C);
    lbl_1_bss_18 = omAddObjEx(lbl_1_bss_0, 0x1000, 32, 32, -1,
        fn_1_45C0);
    lbl_1_bss_1C = omAddObjEx(lbl_1_bss_0, 0x1000, 96, 16, -1, NULL);
    if (GwCommon.unkFlag4 != 0) {
        lbl_1_bss_20 = omAddObjEx(lbl_1_bss_0, 0x1000, 0, 0, -1,
            fn_1_4BC8);
    }
    GwCommon.unkFlag4 = 1;
    HuPrcChildCreate(fn_1_4DAC, 0x3000, 0x3000, 0, lbl_1_bss_0);
}

inline void fn_1_4E5C(void);

void fn_1_52D8(void)
{
    OSReport(lbl_1_data_DD);
    fn_1_4E5C();
}
