#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/printfunc.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "datadir_enum.h"
#include "string.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"
#ifndef M650_H
#define M650_H

/* Effect IDs observed at the audio consumers; original effect names unknown. */
enum M650EffectId {
    M650_EFFECT_2028 = 2028,
    M650_EFFECT_2029 = 2029,
    M650_EFFECT_2030 = 2030,
    M650_EFFECT_2031 = 2031,
    M650_EFFECT_2032 = 2032,
    M650_EFFECT_2033 = 2033,
    M650_EFFECT_2034 = 2034,
    M650_EFFECT_2035 = 2035,
    M650_EFFECT_2036 = 2036,
    M650_EFFECT_2037 = 2037,
    M650_EFFECT_2038 = 2038,
    M650_EFFECT_2039 = 2039,
    M650_EFFECT_2040 = 2040,
    M650_EFFECT_2041 = 2041,
    M650_EFFECT_2042 = 2042,
    M650_EFFECT_2043 = 2043,
    M650_EFFECT_2044 = 2044,
    M650_EFFECT_2045 = 2045,
    M650_EFFECT_2046 = 2046,
    M650_EFFECT_2047 = 2047,
    M650_EFFECT_2048 = 2048,
    M650_EFFECT_2049 = 2049,
    M650_EFFECT_2050 = 2050,
    M650_EFFECT_2051 = 2051
};

/* Consumer-derived layouts. Explicit unknown intervals do not imply original field types. */
typedef struct M650Scene {
    MGTIMER *timer;
    s32 record;
    s16 winner;
    s16 night;
    s16 state;
    s16 frame;
    s16 unk10;
    s16 unk12;
    s16 recordChanged;
    s16 unk16;
    int audio;
} M650Scene;

typedef struct M650Player {
    /* No direct accesses establish these first two bytes of the 160-byte record. */
    u8 unknown00[2];
    s16 model;
    s16 motionIDs[5];
    /* Five motion IDs end at byte 14; charNo is consumed at byte 16. */
    u8 unknown0E[2];
    s16 charNo;
    s16 unk12;
    s16 unk14;
    s16 unk16;
    s16 unk18;
    s16 unk1A;
    s16 unk1C;
    s16 model1E;
    s16 model20;
    s16 jointMotionIDs[4];
    HuVecF direction;
    HuVecF reflected;
    float unk44;
    float unk48;
    float unk4C;
    u8 flag50;
    u8 unk51;
    s16 model52;
    s16 model54;
    /* No recovered direct accesses between the model at byte 84 and vector at 100. */
    u8 unknown56[14];
    HuVecF pos;
    MGTIMER *timer;
    s16 unk74;
    s16 unk76;
    s16 model78;
    s16 model7A;
    s16 model7C;
    s16 candidateIDs[12];
    s16 candidateCount;
    s16 unk98;
    s16 unk9A;
    s16 unk9C;
} M650Player;

typedef struct M650Cell {
    s16 state;
    s16 timer;
} M650Cell;

typedef struct M650Point {
    HuVecF pos;
    s16 kind;
} M650Point;

typedef struct M650Motion {
    s32 file;
    u32 unk4;
} M650Motion;

extern M650Scene lbl_1_bss_0;
extern OMOBJMAN *lbl_1_bss_1C;
extern M650Player lbl_1_bss_28[4];
extern M650Cell lbl_1_bss_2AC[32][4];
extern MGSEQ_PARAM lbl_1_data_0;
extern M650Motion lbl_1_data_58[5];
extern u16 lbl_1_data_150[4];
extern M650Point lbl_1_data_1F8[5];
extern M650Point lbl_1_data_248[32];
extern s32 lbl_1_data_448[2];
extern s32 lbl_1_data_450[2];

void fn_1_F0(s16 mode, s16 frame);
void fn_1_168(s16 mode, s16 frame);
void fn_1_188(s16 mode, s16 frame);
void fn_1_1C8(s16 mode, s16 frame);
void fn_1_200(s16 mode, s16 frame);
void fn_1_6478(HU3D_MODEL *modelP, Mtx *mtx);
s16 fn_1_6D88(s16 player, HuVecF *pos);
void fn_1_23C4(s16 player, s16 motion, float blend);
void fn_1_5600(s16 unusedPlayer, float value, float *out0, float *out1);
s16 fn_1_6EE4(HuVecF *a, HuVecF *b, float radius);
s16 fn_1_7000(s16 unusedPlayer, HuVecF *pos, float angle, s16 candidate);
void fn_1_289C(s16 player);
void fn_1_14FC(void);
void fn_1_1830(void);
void fn_1_1AE4(void);
void fn_1_62E0(void);
void fn_1_5258(void);
void fn_1_5308(void);
void fn_1_54D4(void);
void fn_1_2558(void);
void fn_1_25B0(void);
void fn_1_550(void);
void fn_1_8EC(void);
void fn_1_F90(OMOBJ *obj);
void fn_1_6F54(void);
s16 fn_1_4D58(s16 frame);

#endif
