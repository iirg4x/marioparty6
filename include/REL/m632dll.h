#ifndef M632DLL_H
#define M632DLL_H

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
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "datadir_enum.h"
#include "string.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"
#include <stddef.h>

#define fn_1_7364 __cvt_fp2unsigned
#define fn_1_75B8 __div2u
#define fn_1_76A4 __div2i
#define fn_1_77DC __mod2u
#define fn_1_78C0 __mod2i
#define fn_1_79CC __shl2i
#define fn_1_79F0 __shr2u
#define fn_1_7A14 __shr2i
#define fn_1_7A3C __cvt_sll_dbl
#define fn_1_7AEC __cvt_ull_dbl
#define fn_1_7B88 __cvt_sll_flt
#define fn_1_7C3C __cvt_ull_flt
#define fn_1_7CDC __cvt_dbl_usll
/* Consumed record layout. Names are descriptive, not original symbols.
 * Table extents come from the retail object and bounded copy/consumer loops. */
typedef struct M632State {
    HUPROCESS *objectManager;
    s16 phase;
    s32 frame;
    s16 cameraModels[5]; /* 0x00C */
    s16 cameraMotions[5]; /* 0x016 */
    s16 motions020[2];
    s16 models024[4];
    s16 model02C;
    s16 models02E[3];
    s16 models034[3];
    u8 unknown03A[2];
    s16 collisionModel;
    u8 unknown03E[22];
    MGACTOR *collisionActors[10]; /* 0x054; consumed slot extent, not recovered capacity. */
    s32 collisionCount;
    s16 linkedModels[10]; /* 0x080 */
    s32 linkedModelCount;
    f32 collisionRotY[10]; /* 0x098 */
    s16 model0C0;
    MGPLAYER *players[4]; /* 0xC4: MgPlayerCreate stores; actor at+0x3C. */
    s32 charNo[4]; /* 0xD4: CharModelVoicePanSet consumers. */
    s32 padNo[4];
    s32 group[4];
    s32 playerState[4];
    Point3d playerMotionVec[4]; /* 0x114 */
    s32 playerTimer[4];
    s32 playerAIState[4];
    Point3d playerDirection[4]; /* 0x164 */
    f32 scalar194, scalar198;
    s32 patternIndex;
    Point3d positions[4]; /* 0x1A0, observed indexed 12-byte copies. */
    f32 scalar1D0;
    u8 unknown1D4[4];
    f32 scalar1D8;
    Point3d tilt; /* 0x1DC, whole-vector copy and magnitude consumers. */
    MGTIMER *timer;
    s32 completion;
    int activeMask;
    s32 gridA[64]; /* 0x1F4 */
    s32 gridB[64]; /* 0x2F4 */
    s32 stream, field3F8, field3FC, field400;
    s32 collisionPairs[10][2]; /* 0x404 */
    s32 collisionTimers[10]; /* 0x454 */
} M632State; /* Retail memset and BSS extent: 0x47C. */

typedef struct M632GridCandidate {
    s32 x, z;
    f32 distance;
    s32 cost;
} M632GridCandidate;

/* Anonymous SDK qsort and float conversion; not application recovery credit. */
void fn_1_6574(void *, size_t, size_t, int (*)(const void *, const void *));
u32 fn_1_7364(f64);

extern MGSEQ_PARAM lbl_1_data_0;
extern char *lbl_1_data_108[4];
extern unsigned int lbl_1_data_118[4];
/* Retail .data[0x128,0x158): unreferenced initialized storage.
 * The words resemble coordinates, but no consumer authenticates a record type. */
extern u8 lbl_1_data_128[48];
extern unsigned int lbl_1_data_158[16];
extern s32 lbl_1_data_198[4];
extern u8 *lbl_1_data_2A8[4];
extern M632State lbl_1_bss_0;
s32 fn_1_A0(s32 arg0, s32 arg1);
void fn_1_104(s32 arg0);
void fn_1_140(s32 arg0, s32 arg1);
void fn_1_21C(s16 mode, s16 frameNo);
void fn_1_240(OMOBJ *obj);
void fn_1_3E0(OMOBJ *obj);
void fn_1_5A8(s16 mode, s16 frameNo);
void fn_1_173C(s16 mode, s16 frameNo);
void fn_1_17C0(s16 mode, s16 frameNo);
void fn_1_2368(s16 mode, s16 frameNo);
void fn_1_2574(s16 mode, s16 frameNo);
void fn_1_2E04(s16 mode, s16 frameNo);
void fn_1_2F88(s16 mode, s16 frameNo);
void fn_1_2F8C(s16 mode, s16 frameNo);
void fn_1_2F90(void);
void fn_1_3510(MGACTOR *actorP, int param);
int fn_1_353C(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b);
void fn_1_3934(void);
void fn_1_4C8C(HU3D_MODEL *modelP, Mtx *mtx);
void fn_1_4C90(void);
void fn_1_4D48(void);
void fn_1_50A0(s32 arg0);
void fn_1_535C(void);
void fn_1_5400(s32 arg0);
void fn_1_597C(s32 arg0);
void fn_1_5D34(s32 arg0);
int fn_1_5FE4(const void *a, const void *b);
s32 fn_1_604C(s32 arg0, s32 arg1, s32 arg2, s32 *arg3, s32 *arg4);

/* Existing consumed layout; unknown intervals are not padding. */
typedef char m632_bss_size_check[(sizeof(M632State) == 1148) ? 1 : -1]; /* extent 0x47C */
typedef char m632_pointer_size_check[(sizeof(void *) == 4) ? 1 : -1];
typedef char m632_offset_0[(offsetof(M632State, activeMask) == 496) ? 1 : -1]; /* offset 0x1F0 */
typedef char m632_width_0[(sizeof(((M632State *)0)->activeMask) == 4) ? 1 : -1];
typedef char m632_offset_1[(offsetof(M632State, cameraModels) == 12) ? 1 : -1]; /* offset 0xC */
typedef char m632_width_1[(sizeof(((M632State *)0)->cameraModels[0]) == 2) ? 1 : -1];
typedef char m632_offset_2[(offsetof(M632State, cameraMotions) == 22) ? 1 : -1]; /* offset 0x16 */
typedef char m632_width_2[(sizeof(((M632State *)0)->cameraMotions[0]) == 2) ? 1 : -1];
typedef char m632_offset_3[(offsetof(M632State, charNo) == 212) ? 1 : -1]; /* offset 0xD4 */
typedef char m632_width_3[(sizeof(((M632State *)0)->charNo[0]) == 4) ? 1 : -1];
typedef char m632_offset_4[(offsetof(M632State, collisionActors) == 84) ? 1 : -1]; /* offset 0x54 */
typedef char m632_width_4[(sizeof(((M632State *)0)->collisionActors[0]) == 4) ? 1 : -1];
typedef char m632_offset_5[(offsetof(M632State, collisionCount) == 124) ? 1 : -1]; /* offset 0x7C */
typedef char m632_width_5[(sizeof(((M632State *)0)->collisionCount) == 4) ? 1 : -1];
typedef char m632_offset_6[(offsetof(M632State, collisionModel) == 60) ? 1 : -1]; /* offset 0x3C */
typedef char m632_width_6[(sizeof(((M632State *)0)->collisionModel) == 2) ? 1 : -1];
typedef char m632_offset_7[(offsetof(M632State, collisionPairs) == 1028) ? 1 : -1]; /* offset 0x404 */
typedef char m632_width_7[(sizeof(((M632State *)0)->collisionPairs[0][0]) == 4) ? 1 : -1];
typedef char m632_offset_8[(offsetof(M632State, collisionRotY) == 152) ? 1 : -1]; /* offset 0x98 */
typedef char m632_width_8[(sizeof(((M632State *)0)->collisionRotY[0]) == 4) ? 1 : -1];
typedef char m632_offset_9[(offsetof(M632State, collisionTimers) == 1108) ? 1 : -1]; /* offset 0x454 */
typedef char m632_width_9[(sizeof(((M632State *)0)->collisionTimers[0]) == 4) ? 1 : -1];
typedef char m632_offset_10[(offsetof(M632State, completion) == 492) ? 1 : -1]; /* offset 0x1EC */
typedef char m632_width_10[(sizeof(((M632State *)0)->completion) == 4) ? 1 : -1];
typedef char m632_offset_11[(offsetof(M632State, field3F8) == 1016) ? 1 : -1]; /* offset 0x3F8 */
typedef char m632_width_11[(sizeof(((M632State *)0)->field3F8) == 4) ? 1 : -1];
typedef char m632_offset_12[(offsetof(M632State, field3FC) == 1020) ? 1 : -1]; /* offset 0x3FC */
typedef char m632_width_12[(sizeof(((M632State *)0)->field3FC) == 4) ? 1 : -1];
typedef char m632_offset_13[(offsetof(M632State, field400) == 1024) ? 1 : -1]; /* offset 0x400 */
typedef char m632_width_13[(sizeof(((M632State *)0)->field400) == 4) ? 1 : -1];
typedef char m632_offset_14[(offsetof(M632State, frame) == 8) ? 1 : -1]; /* offset 0x8 */
typedef char m632_width_14[(sizeof(((M632State *)0)->frame) == 4) ? 1 : -1];
typedef char m632_offset_15[(offsetof(M632State, gridA) == 500) ? 1 : -1]; /* offset 0x1F4 */
typedef char m632_width_15[(sizeof(((M632State *)0)->gridA[0]) == 4) ? 1 : -1];
typedef char m632_offset_16[(offsetof(M632State, gridB) == 756) ? 1 : -1]; /* offset 0x2F4 */
typedef char m632_width_16[(sizeof(((M632State *)0)->gridB[0]) == 4) ? 1 : -1];
typedef char m632_offset_17[(offsetof(M632State, group) == 244) ? 1 : -1]; /* offset 0xF4 */
typedef char m632_width_17[(sizeof(((M632State *)0)->group[0]) == 4) ? 1 : -1];
typedef char m632_offset_18[(offsetof(M632State, linkedModelCount) == 148) ? 1 : -1]; /* offset 0x94 */
typedef char m632_width_18[(sizeof(((M632State *)0)->linkedModelCount) == 4) ? 1 : -1];
typedef char m632_offset_19[(offsetof(M632State, linkedModels) == 128) ? 1 : -1]; /* offset 0x80 */
typedef char m632_width_19[(sizeof(((M632State *)0)->linkedModels[0]) == 2) ? 1 : -1];
typedef char m632_offset_20[(offsetof(M632State, model02C) == 44) ? 1 : -1]; /* offset 0x2C */
typedef char m632_width_20[(sizeof(((M632State *)0)->model02C) == 2) ? 1 : -1];
typedef char m632_offset_21[(offsetof(M632State, model0C0) == 192) ? 1 : -1]; /* offset 0xC0 */
typedef char m632_width_21[(sizeof(((M632State *)0)->model0C0) == 2) ? 1 : -1];
typedef char m632_offset_22[(offsetof(M632State, models024) == 36) ? 1 : -1]; /* offset 0x24 */
typedef char m632_width_22[(sizeof(((M632State *)0)->models024[0]) == 2) ? 1 : -1];
typedef char m632_offset_23[(offsetof(M632State, models02E) == 46) ? 1 : -1]; /* offset 0x2E */
typedef char m632_width_23[(sizeof(((M632State *)0)->models02E[0]) == 2) ? 1 : -1];
typedef char m632_offset_24[(offsetof(M632State, models034) == 52) ? 1 : -1]; /* offset 0x34 */
typedef char m632_width_24[(sizeof(((M632State *)0)->models034[0]) == 2) ? 1 : -1];
typedef char m632_offset_25[(offsetof(M632State, motions020) == 32) ? 1 : -1]; /* offset 0x20 */
typedef char m632_width_25[(sizeof(((M632State *)0)->motions020[0]) == 2) ? 1 : -1];
typedef char m632_offset_26[(offsetof(M632State, objectManager) == 0) ? 1 : -1]; /* offset 0x0 */
typedef char m632_width_26[(sizeof(((M632State *)0)->objectManager) == 4) ? 1 : -1];
typedef char m632_offset_27[(offsetof(M632State, padNo) == 228) ? 1 : -1]; /* offset 0xE4 */
typedef char m632_width_27[(sizeof(((M632State *)0)->padNo[0]) == 4) ? 1 : -1];
typedef char m632_offset_28[(offsetof(M632State, patternIndex) == 412) ? 1 : -1]; /* offset 0x19C */
typedef char m632_width_28[(sizeof(((M632State *)0)->patternIndex) == 4) ? 1 : -1];
typedef char m632_offset_29[(offsetof(M632State, phase) == 4) ? 1 : -1]; /* offset 0x4 */
typedef char m632_width_29[(sizeof(((M632State *)0)->phase) == 2) ? 1 : -1];
typedef char m632_offset_30[(offsetof(M632State, playerAIState) == 340) ? 1 : -1]; /* offset 0x154 */
typedef char m632_width_30[(sizeof(((M632State *)0)->playerAIState[0]) == 4) ? 1 : -1];
typedef char m632_offset_31[(offsetof(M632State, playerDirection) == 356) ? 1 : -1]; /* offset 0x164 */
typedef char m632_width_31[(sizeof(((M632State *)0)->playerDirection[0]) == 12) ? 1 : -1];
typedef char m632_offset_32[(offsetof(M632State, playerMotionVec) == 276) ? 1 : -1]; /* offset 0x114 */
typedef char m632_width_32[(sizeof(((M632State *)0)->playerMotionVec[0]) == 12) ? 1 : -1];
typedef char m632_offset_33[(offsetof(M632State, playerState) == 260) ? 1 : -1]; /* offset 0x104 */
typedef char m632_width_33[(sizeof(((M632State *)0)->playerState[0]) == 4) ? 1 : -1];
typedef char m632_offset_34[(offsetof(M632State, playerTimer) == 324) ? 1 : -1]; /* offset 0x144 */
typedef char m632_width_34[(sizeof(((M632State *)0)->playerTimer[0]) == 4) ? 1 : -1];
typedef char m632_offset_35[(offsetof(M632State, players) == 196) ? 1 : -1]; /* offset 0xC4 */
typedef char m632_width_35[(sizeof(((M632State *)0)->players[0]) == 4) ? 1 : -1];
typedef char m632_offset_36[(offsetof(M632State, positions) == 416) ? 1 : -1]; /* offset 0x1A0 */
typedef char m632_width_36[(sizeof(((M632State *)0)->positions[0]) == 12) ? 1 : -1];
typedef char m632_offset_37[(offsetof(M632State, scalar194) == 404) ? 1 : -1]; /* offset 0x194 */
typedef char m632_width_37[(sizeof(((M632State *)0)->scalar194) == 4) ? 1 : -1];
typedef char m632_offset_38[(offsetof(M632State, scalar198) == 408) ? 1 : -1]; /* offset 0x198 */
typedef char m632_width_38[(sizeof(((M632State *)0)->scalar198) == 4) ? 1 : -1];
typedef char m632_offset_39[(offsetof(M632State, scalar1D0) == 464) ? 1 : -1]; /* offset 0x1D0 */
typedef char m632_width_39[(sizeof(((M632State *)0)->scalar1D0) == 4) ? 1 : -1];
typedef char m632_offset_40[(offsetof(M632State, scalar1D8) == 472) ? 1 : -1]; /* offset 0x1D8 */
typedef char m632_width_40[(sizeof(((M632State *)0)->scalar1D8) == 4) ? 1 : -1];
typedef char m632_offset_41[(offsetof(M632State, stream) == 1012) ? 1 : -1]; /* offset 0x3F4 */
typedef char m632_width_41[(sizeof(((M632State *)0)->stream) == 4) ? 1 : -1];
typedef char m632_offset_42[(offsetof(M632State, tilt) == 476) ? 1 : -1]; /* offset 0x1DC */
typedef char m632_width_42[(sizeof(((M632State *)0)->tilt) == 12) ? 1 : -1];
typedef char m632_offset_43[(offsetof(M632State, timer) == 488) ? 1 : -1]; /* offset 0x1E8 */
typedef char m632_width_43[(sizeof(((M632State *)0)->timer) == 4) ? 1 : -1];

#endif
