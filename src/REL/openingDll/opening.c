/* Builds and runs the animated Mario Party 6 opening sequence. */
#include "dolphin.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"

#define OPENING_CHAR_COUNT 10
#define OPENING_WIN_COUNT 4
#define OPENING_DVD_RESERVE_BYTES 1048576
#define OPENING_GUIDE_ALTERNATE_FLAG (1 << 15)
#define OPENING_GUIDE_INDEX_MASK 0xFF
#define OPENING_PRIMARY_GUIDE_INDEX 10
#define OPENING_SECONDARY_GUIDE_INDEX 11
#define MSM_STREAM_OPENING 0

/* Opening dialogue and the skip prompt use the opening and guide message banks. */
#define MESS_OPENING 11
#define MESS_SYS_GUIDE 1
#define MESSNUM(dir, file) (u32)(((dir) << 16) | (file))

typedef struct OpeningEventWork {
    void (*callback)(s32, s32); /* Opening animation callback run by its child process. */
    s32 eventPrimaryParameter; /* Callback's primary value, such as a duration or character
                                * index. */
    s32 eventSecondaryParameter; /* Callback's second value, such as an angle or animation index. */
} OpeningEventWork;

typedef struct OpeningFocusObject {
    u32 unusedObjectDataA; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unusedObjectDataB; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unusedObjectDataC; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unusedObjectDataD; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unusedObjectDataE; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unusedObjectDataF; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unusedObjectDataG; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 positionXBits; /* Bit representation of the guide character's horizontal position. */
    u32 positionYBits; /* Bit representation of the guide character's vertical position. */
    u32 unusedObjectDataJ; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unusedObjectDataK; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unusedObjectDataL; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 rotationZBits; /* Bit representation of the guide character's Z rotation in degrees. */
    float primaryFocusScale; /* Guide-character focus scale: 1.0 normal, 1.2 enlarged. */
    float secondaryFocusScale; /* Guide-character focus scale: 1.0 normal, 1.2 enlarged. */
} OpeningFocusObject;

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern s16 lbl_1_bss_0;
extern s32 lbl_1_bss_4;
extern s32 lbl_1_bss_8;
extern s32 lbl_1_bss_C;
extern float lbl_1_bss_10;
extern HuVecF lbl_1_bss_14;
extern float lbl_1_bss_20;
extern HuVec2f lbl_1_bss_24;
extern s16 lbl_1_bss_2C;
extern float lbl_1_bss_38;
extern HuVecF lbl_1_bss_3C;
extern HuVecF lbl_1_bss_48;
extern HuVecF lbl_1_bss_54;
extern HUWINID lbl_1_bss_2E[OPENING_WIN_COUNT];
extern HU3D_ANIMID lbl_1_bss_60[12];
extern HU3D_ANIMID lbl_1_bss_78;
extern ANIMDATA *lbl_1_bss_7C[79];
extern ANIMDATA *lbl_1_bss_1B8[2];
extern void *lbl_1_bss_1C0;
extern void *lbl_1_bss_1C4[2];
extern s16 lbl_1_bss_1CC;
extern HU3D_MODELID lbl_1_bss_1CE[8];
extern s16 lbl_1_bss_1DE[3];
extern OMOBJMAN *lbl_1_bss_1E4;
extern s16 lbl_1_data_0[OPENING_WIN_COUNT];
extern u32 lbl_1_data_8[80];
extern char *lbl_1_data_1DC[12];
extern char *lbl_1_data_2F0[12];
extern HuVecF lbl_1_data_320;
extern HuVecF lbl_1_data_32C;
extern char lbl_1_data_338[14];
extern char lbl_1_data_346[15];
extern char lbl_1_data_355[20];
extern char lbl_1_data_369[39];

int _prolog(void);
void _epilog(void);

void fn_1_A0(void);

void fn_1_28C(void);
void fn_1_320(void);
void fn_1_A00(s32 enablePrompt);
void fn_1_9A4(void);
void fn_1_1128(void);
void fn_1_1828(s16 layerNo);
void fn_1_192C(s16 layerNo);
void fn_1_1B7C(void);
void fn_1_1BD8(void);
void fn_1_2104(s32 characterIndex, s32 targetAngle);
void fn_1_2238(u32 frameCount);
void fn_1_2304(u32 guideChar, s32 frameCount);
void fn_1_257C(u32 frameCount);
void fn_1_2708(u32 frameCount);
void fn_1_2B9C(u32 frameCount);
void fn_1_2D74(u32 frameCount);
void fn_1_34EC(u32 frameCount);
void fn_1_37F0(void);
void fn_1_392C(u32 frameCount);
void fn_1_3C28(void);
void fn_1_4068(u32 frameCount);
void fn_1_41B4(void);
void fn_1_420C(void);
void fn_1_4264(u32 frameCount);
void fn_1_4484(u32 frameCount);
void fn_1_45D4(s16 characterIndex, float angleDegrees);
void fn_1_46B4(s16 animIndex, s16 resourceIndex);
void fn_1_470C(s16 animIndex, s16 resourceIndex);
void fn_1_4744(s32 animIndex, s32 resourceIndex);
void fn_1_47AC(s16 winIndex, u32 message, s16 frameCount);
void fn_1_48AC(void);
void fn_1_4BD8(void);
void fn_1_4C30(void);
void fn_1_4DB0(void);
void fn_1_4E00(void);
void fn_1_4E34(void (*callback)(void), s32 eventPrimaryParameter, s32 eventSecondaryParameter);
void fn_1_4ECC(HuVecF *rotation, HuVecF *target, float distance);
void fn_1_517C(float alpha);

/* Runs static constructors, then initializes the opening overlay objects. */
int _prolog(void)
{
    const VoidFunc *ctors = _ctors;

    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    fn_1_A0();
    return 0;
}

/* Runs the overlay's static destructors when the REL unloads. */
void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}

#include "REL/openingDll/sequence.h"

/* Called by _prolog to initialize the camera, lights, windows, object manager, and opening
 * process. */
void fn_1_A0(void)
{
    OSReport(lbl_1_data_369);
    lbl_1_bss_1E4 = omInitObjMan(50, 8192);
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.f, 15000.f,
                             1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.f,
                          480.f, 0.0f, 1.0f);
    HuPrcCreate(fn_1_28C, 100, 12288, 0);

    lbl_1_bss_1DE[0] = Hu3DGLightCreate(0.0f, 10.f,
                                        1000.f, 0.0f,
                                        -0.01f, -1.0f,
                                        64, 64, 96);
    Hu3DGLightInfinitytSet(lbl_1_bss_1DE[0]);
    lbl_1_bss_1DE[1] = Hu3DGLightCreate(0.0f, 2000.f,
                                        100.f, 0.0f,
                                        -1.0f, -0.05f,
                                        160, 160, 160);
    Hu3DGLightInfinitytSet(lbl_1_bss_1DE[1]);
    Hu3DBGColorSet(0, 0, 0);
    HuWinInit(1);
}

s16 lbl_1_data_0[OPENING_WIN_COUNT] = { 0, 4, 3, 5 };

u32 lbl_1_data_8[80] = {
    DATANUM(DATA_title, 30), DATANUM(DATA_title, 32),
    DATANUM(DATA_title, 34), DATANUM(DATA_title, 33),
    DATANUM(DATA_title, 34), DATANUM(DATA_title, 35),
    DATANUM(DATA_title, 36), DATANUM(DATA_title, 38),
    DATANUM(DATA_title, 39), DATANUM(DATA_title, 40),
    DATANUM(DATA_title, 41), DATANUM(DATA_title, 42),
    DATANUM(DATA_title, 43), DATANUM(DATA_title, 44),
    DATANUM(DATA_title, 45), DATANUM(DATA_title, 46),
    DATANUM(DATA_title, 47), DATANUM(DATA_title, 48),
    DATANUM(DATA_title, 49), DATANUM(DATA_title, 50),
    DATANUM(DATA_title, 51), DATANUM(DATA_title, 52),
    DATANUM(DATA_title, 53), DATANUM(DATA_title, 54),
    DATANUM(DATA_title, 55), DATANUM(DATA_title, 56),
    DATANUM(DATA_title, 57), DATANUM(DATA_title, 58),
    DATANUM(DATA_title, 59), DATANUM(DATA_title, 60),
    DATANUM(DATA_title, 61), DATANUM(DATA_title, 62),
    DATANUM(DATA_title, 63), DATANUM(DATA_title, 64),
    DATANUM(DATA_title, 65), DATANUM(DATA_title, 66),
    DATANUM(DATA_title, 67), DATANUM(DATA_title, 68),
    DATANUM(DATA_title, 69), DATANUM(DATA_title, 70),
    DATANUM(DATA_title, 71), DATANUM(DATA_title, 72),
    DATANUM(DATA_title, 73), DATANUM(DATA_title, 74),
    DATANUM(DATA_title, 75), DATANUM(DATA_title, 76),
    DATANUM(DATA_title, 77), DATANUM(DATA_title, 78),
    DATANUM(DATA_title, 79), DATANUM(DATA_title, 80),
    DATANUM(DATA_title, 81), DATANUM(DATA_title, 82),
    DATANUM(DATA_title, 83), DATANUM(DATA_title, 84),
    DATANUM(DATA_title, 85), DATANUM(DATA_title, 86),
    DATANUM(DATA_title, 87), DATANUM(DATA_title, 88),
    DATANUM(DATA_title, 89), DATANUM(DATA_title, 90),
    DATANUM(DATA_title, 91), DATANUM(DATA_title, 92),
    DATANUM(DATA_title, 93), DATANUM(DATA_title, 94),
    DATANUM(DATA_title, 95), DATANUM(DATA_title, 96),
    DATANUM(DATA_title, 97), DATANUM(DATA_title, 98),
    DATANUM(DATA_title, 99), DATANUM(DATA_title, 100),
    DATANUM(DATA_title, 105), DATANUM(DATA_title, 106),
    DATANUM(DATA_title, 107), DATANUM(DATA_title, 108),
    DATANUM(DATA_title, 101), DATANUM(DATA_title, 102),
    DATANUM(DATA_title, 103), DATANUM(DATA_title, 104),
    DATANUM(DATA_title, 37), HU_DATANUM_NONE,
};

char lbl_1_data_148[12] = "Dummy_mario";
char lbl_1_data_154[12] = "Dummy_Luigi";
char lbl_1_data_160[12] = "Dummy_peach";
char lbl_1_data_16C[12] = "Dummy_yoshi";
char lbl_1_data_178[12] = "Dummy_wario";
char lbl_1_data_184[12] = "Dummy_daisy";
char lbl_1_data_190[14] = "Dummy_waluigi";
char lbl_1_data_19E[14] = "Dummy_kinopio";
char lbl_1_data_1AC[13] = "Dummy_teresa";
char lbl_1_data_1B9[16] = "Dummy_minikoopa";
char lbl_1_data_1C9[9] = "guide001";
char lbl_1_data_1D2[10] = "guide002";

char *lbl_1_data_1DC[12] = {
    lbl_1_data_148, lbl_1_data_154, lbl_1_data_160, lbl_1_data_16C,
    lbl_1_data_178, lbl_1_data_184, lbl_1_data_190, lbl_1_data_19E,
    lbl_1_data_1AC, lbl_1_data_1B9, lbl_1_data_1C9, lbl_1_data_1D2,
};

char lbl_1_data_20C[18] = "op_stage011-mario";
char lbl_1_data_21E[18] = "op_stage011-luigi";
char lbl_1_data_230[18] = "op_stage011-peach";
char lbl_1_data_242[18] = "op_stage011-yoshi";
char lbl_1_data_254[18] = "op_stage011-wario";
char lbl_1_data_266[18] = "op_stage011-daisy";
char lbl_1_data_278[20] = "op_stage011-waluigi";
char lbl_1_data_28C[20] = "op_stage011-kinopio";
char lbl_1_data_2A0[19] = "op_stage011-teresa";
char lbl_1_data_2B3[22] = "op_stage011-minikoopa";
char lbl_1_data_2C9[19] = "op_stage011-soruru";
char lbl_1_data_2DC[20] = "op_stage011-luluna";

char *lbl_1_data_2F0[12] = {
    lbl_1_data_20C, lbl_1_data_21E, lbl_1_data_230, lbl_1_data_242,
    lbl_1_data_254, lbl_1_data_266, lbl_1_data_278, lbl_1_data_28C,
    lbl_1_data_2A0, lbl_1_data_2B3, lbl_1_data_2C9, lbl_1_data_2DC,
};

HuVecF lbl_1_data_320 = { 0.0f, 0.0f, -2000.0f };
HuVecF lbl_1_data_32C = { -20.0f, -30.0f, 30.0f };
char lbl_1_data_338[14] = "op_book_dummy";
char lbl_1_data_346[15] = "op_book_dummy2";
char lbl_1_data_355[20] = "op_stage004-bg_root";
char lbl_1_data_369[39] = "******* Opening ObjectSetup *********\n";

OMOBJMAN *lbl_1_bss_1E4;
s16 lbl_1_bss_1DE[3];
HU3D_MODELID lbl_1_bss_1CE[8];
s16 lbl_1_bss_1CC;
void *lbl_1_bss_1C4[2];
void *lbl_1_bss_1C0;
ANIMDATA *lbl_1_bss_1B8[2];
ANIMDATA *lbl_1_bss_7C[79];
HU3D_ANIMID lbl_1_bss_78;
HU3D_ANIMID lbl_1_bss_60[12];
HuVecF lbl_1_bss_54;
HuVecF lbl_1_bss_48;
HuVecF lbl_1_bss_3C;
float lbl_1_bss_38;
HUWINID lbl_1_bss_2E[OPENING_WIN_COUNT];
s16 lbl_1_bss_2C;
HuVec2f lbl_1_bss_24;
float lbl_1_bss_20;
HuVecF lbl_1_bss_14;
float lbl_1_bss_10;
s32 lbl_1_bss_C;
s32 lbl_1_bss_8;
s32 lbl_1_bss_4;
s16 lbl_1_bss_0;
