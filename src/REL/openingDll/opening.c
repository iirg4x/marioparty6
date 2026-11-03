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
    u32 unk_00; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unk_04; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unk_08; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unk_0C; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unk_10; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unk_14; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unk_18; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 positionXBits; /* Bit representation of the guide character's horizontal position. */
    u32 positionYBits; /* Bit representation of the guide character's vertical position. */
    u32 unk_24; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unk_28; /* Object word not accessed by this overlay; its meaning is unknown. */
    u32 unk_2C; /* Object word not accessed by this overlay; its meaning is unknown. */
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
    lbl_1_bss_1E4 = fn_8003A8BC(50, 8192);
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.f, 15000.f,
                             1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.f,
                          480.f, 0.0f, 1.0f);
    fn_8000D818(fn_1_28C, 100, 12288, 0);

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
    DATANUM(0xd50000, 30), DATANUM(0xd50000, 32),
    DATANUM(0xd50000, 34), DATANUM(0xd50000, 33),
    DATANUM(0xd50000, 34), DATANUM(0xd50000, 35),
    DATANUM(0xd50000, 36), DATANUM(0xd50000, 38),
    DATANUM(0xd50000, 39), DATANUM(0xd50000, 40),
    DATANUM(0xd50000, 41), DATANUM(0xd50000, 42),
    DATANUM(0xd50000, 43), DATANUM(0xd50000, 44),
    DATANUM(0xd50000, 45), DATANUM(0xd50000, 46),
    DATANUM(0xd50000, 47), DATANUM(0xd50000, 48),
    DATANUM(0xd50000, 49), DATANUM(0xd50000, 50),
    DATANUM(0xd50000, 51), DATANUM(0xd50000, 52),
    DATANUM(0xd50000, 53), DATANUM(0xd50000, 54),
    DATANUM(0xd50000, 55), DATANUM(0xd50000, 56),
    DATANUM(0xd50000, 57), DATANUM(0xd50000, 58),
    DATANUM(0xd50000, 59), DATANUM(0xd50000, 60),
    DATANUM(0xd50000, 61), DATANUM(0xd50000, 62),
    DATANUM(0xd50000, 63), DATANUM(0xd50000, 64),
    DATANUM(0xd50000, 65), DATANUM(0xd50000, 66),
    DATANUM(0xd50000, 67), DATANUM(0xd50000, 68),
    DATANUM(0xd50000, 69), DATANUM(0xd50000, 70),
    DATANUM(0xd50000, 71), DATANUM(0xd50000, 72),
    DATANUM(0xd50000, 73), DATANUM(0xd50000, 74),
    DATANUM(0xd50000, 75), DATANUM(0xd50000, 76),
    DATANUM(0xd50000, 77), DATANUM(0xd50000, 78),
    DATANUM(0xd50000, 79), DATANUM(0xd50000, 80),
    DATANUM(0xd50000, 81), DATANUM(0xd50000, 82),
    DATANUM(0xd50000, 83), DATANUM(0xd50000, 84),
    DATANUM(0xd50000, 85), DATANUM(0xd50000, 86),
    DATANUM(0xd50000, 87), DATANUM(0xd50000, 88),
    DATANUM(0xd50000, 89), DATANUM(0xd50000, 90),
    DATANUM(0xd50000, 91), DATANUM(0xd50000, 92),
    DATANUM(0xd50000, 93), DATANUM(0xd50000, 94),
    DATANUM(0xd50000, 95), DATANUM(0xd50000, 96),
    DATANUM(0xd50000, 97), DATANUM(0xd50000, 98),
    DATANUM(0xd50000, 99), DATANUM(0xd50000, 100),
    DATANUM(0xd50000, 105), DATANUM(0xd50000, 106),
    DATANUM(0xd50000, 107), DATANUM(0xd50000, 108),
    DATANUM(0xd50000, 101), DATANUM(0xd50000, 102),
    DATANUM(0xd50000, 103), DATANUM(0xd50000, 104),
    DATANUM(0xd50000, 37), -1,
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
