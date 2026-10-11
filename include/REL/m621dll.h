/* Shared state and callback declarations for Pokey Punch-out. */
#ifndef _REL_M621DLL_H
#define _REL_M621DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/mg/seqman.h"
#include "game/mg/actman.h"
#include "game/mg/score.h"

typedef struct M621Target_s M621Target;
typedef struct M621Effect_s M621Effect;
typedef struct M621Environment_s M621Environment;

typedef struct M621TargetPart_s {
    s16 state; /* 0: removed, 1: rising, 2: standing, 3: knocked away, 4: delayed removal, 5:
                * removal countdown. */
    s16 timer; /* Updates remaining or elapsed for the current segment state. */
    u32 collisionMask; /* Collision bits reserved for this segment. */
    HuVecF velocity; /* Per-update movement in world units. */
    HU3D_MODELID model; /* Visible model for this segment. */
    MGACTOR *actor; /* Collision and movement actor for this segment. */
    /* Unused bytes retained in the segment record; their game meaning is unknown. */
    u8 reservedBytes[2];
    HU3D_MODELID effectModel; /* Linked disappearance effect for this segment. */
    s16 effectPlayerNo; /* Player index selecting the segment-hit effect, 0 through 3. */
} M621TargetPart;

struct M621Target_s {
    s16 targetNo; /* Target slot, 0 through 2. */
    s16 state; /* 0: waiting or assembling, 1: active stack, 2: collapsing or cleared stack. */
    s32 timer; /* Updates in the current spawn, assembly, or cleanup phase. */
    s16 firstStackF; /* Nonzero until this slot has shown its first stack. */
    s16 partCount; /* Segment count; later stacks cap at seven, but the unclamped first choice can
                    * reach eight. */
    s16 turnTimer; /* Updates until the next random turn choice. */
    float headRotation; /* Top segment facing angle in degrees. */
    float rotation; /* Stack facing angle in degrees. */
    float turnRandom; /* Random turn value: below 0.4 turns left, above 0.6 turns right, and 0.4
                       * through 0.6 pauses. */
    HuVecF position; /* Target anchor in world units. */
    float stackHeight; /* Vertical rise distance used during stack assembly, in world units. */
    s16 spawnPointNo; /* Scene spawn-point index; -1 means none selected. */
    HU3D_MODELID model; /* Target appearance model. */
    MGACTOR *actor; /* Stack collision actor. */
    M621TargetPart part[7]; /* Seven allocated segments, top first; the unclamped first stack can
                             * set partCount to 8 and write past this array. */
};

typedef struct {
    s16 unusedState; /* Unused field; its game meaning is unknown. */
    s16 timer; /* Updates elapsed during fade-in or the post-round facing-alignment wait. */
    s32 sound; /* Round-start sound handle, retained after the sound is stopped. */
} M621Round;

typedef struct {
    s32 unusedState; /* Written during camera setup but not otherwise read. */
    HU3D_MOTIONID motion[2]; /* Loaded camera motion data; only motion[0] is started, and motion[1]
                              * is unused here. */
    HU3D_MODELID cameraModel; /* Camera motion model bound to camera 0. */
} M621Camera;

typedef struct {
    s32 timer; /* Updates in the current player phase. */
    MGPLAYER *player; /* Engine player and actor state. */
    float baseRotation; /* Starting facing angle in degrees. */
    HuVecF basePos; /* Starting position in world units, saved during setup. */
    s16 score; /* Segments hit, capped at 99. */
    s16 winnerF; /* Nonzero when this player is selected as a winner. */
    s16 playerNo; /* Player slot, 0 through 3. */
    s16 charNo; /* Character assigned to this player. */
    s32 motionNo; /* Cached motion index for this overlay's explicit animation requests. */
    s16 attackF; /* Attack contact latch; value 2 prevents duplicate hit handling. */
    s16 unusedState; /* Cleared when leaving an attack; not otherwise read here. */
    s16 vibrationDelay; /* Updates before hit feedback may vibrate this player. */
    s16 scoreBox; /* Character portrait and score box handle. */
    MGSCORE *scoreDisplay; /* Numeric score display handle. */
    s16 computerF; /* Nonzero when computer-controlled. */
    s16 difficulty; /* Computer difficulty setting. */
    s16 cpuState; /* Computer state: 0 approaches, 1 jumps and attacks while descending, 2 steers a
                   * jump without attacking. */
    s16 cpuTimer; /* Updates spent approaching a target. */
    s16 cpuDelay; /* Updates remaining before the computer's next action. */
    M621Target *target; /* Target currently selected by computer control. */
} M621Player;

extern OMOBJMAN *lbl_1_bss_0;
extern MGSEQ_PARAM lbl_1_data_0;
extern s32 lbl_1_data_28;
extern s16 lbl_1_bss_8;
extern s32 lbl_1_bss_C;
extern M621Player *lbl_1_bss_10[4];
extern M621Effect *lbl_1_bss_20;
extern HU3D_MOTIONID lbl_1_bss_24[4];
extern HU3D_MODELID lbl_1_bss_2C[4];
extern s32 lbl_1_data_9C[4][2];
extern HU3D_MOTIONID lbl_1_bss_34;
extern HU3D_MODELID lbl_1_bss_36;
extern HU3D_MODELID lbl_1_bss_38;
extern HU3D_MODELID lbl_1_bss_3A;
extern HU3D_MODELID lbl_1_bss_3C;
extern HU3D_MODELID lbl_1_bss_3E;
extern M621Target *lbl_1_bss_40[3];
extern s32 lbl_1_data_74[2][3];
extern s32 lbl_1_data_8C[4];
extern unsigned int lbl_1_data_2B0[17];
extern char *lbl_1_data_334[4];
extern HuVecF lbl_1_data_344[4];
extern HuVecF lbl_1_data_30;
extern s32 lbl_1_data_3C[2][5];
extern s32 lbl_1_data_64[2][2];
extern u32 lbl_1_data_2AC;
extern char *lbl_1_data_258[21];
extern s16 lbl_1_data_37E;
extern s16 lbl_1_data_380;
extern M621Environment *lbl_1_bss_4C;

void *fn_1_A0(s32 priority, u32 dataSize, OMOBJ_FUNC callback);
void fn_1_140(OMOBJ *obj, OMOBJ_FUNC callback);
void fn_1_148(void);
void fn_1_2BC(void);
void fn_1_2E0(s16 mode, s16 frameNo);
void fn_1_358(s16 mode, s16 frameNo);
void fn_1_35C(s16 mode, s16 frameNo);
void fn_1_3B8(s16 mode, s16 frameNo);
void fn_1_3BC(s16 mode, s16 frameNo);
void fn_1_410(s16 mode, s16 frameNo);
void fn_1_414(s16 mode, s16 frameNo);
void fn_1_418(s16 mode, s16 frameNo);
void fn_1_41C(s16 mode, s16 frameNo);
s32 fn_1_420(HuVecF *worldPosition, s16 soundId);
s32 fn_1_594(s16 pan, s16 soundId);
void fn_1_5C8(s32 soundHandle);
void fn_1_5F0(OMOBJ *obj);
void fn_1_70C(OMOBJ *obj);
void fn_1_778(OMOBJ *obj);
void fn_1_A5C(OMOBJ *obj);
void fn_1_B4C(OMOBJ *obj);
void fn_1_C90(OMOBJ *obj);
void fn_1_CEC(OMOBJ *obj);
void fn_1_D28(OMOBJ *obj);
void fn_1_106C(M621Player *player, s32 motionNo, float blendTime, u32 motionAttr);
int fn_1_10E8(HuVecF *sourceDirection, HuVecF *normalizedDirection);
int fn_1_1240(COL_NARROW_PARAM *firstCollider, COL_NARROW_PARAM *secondCollider);
void fn_1_13EC(OMOBJ *obj);
void fn_1_175C(OMOBJ *obj);
void fn_1_1844(OMOBJ *obj);
void fn_1_19C4(OMOBJ *obj);
void fn_1_1A3C(OMOBJ *obj);
void fn_1_1B5C(OMOBJ *obj);
void fn_1_1BF4(OMOBJ *obj);
M621Target *fn_1_1C7C(void);
M621Target *fn_1_1D08(M621Player *player);
s16 fn_1_1F9C(M621Player *player);
s16 fn_1_2074(M621Player *player);
void fn_1_215C(OMOBJ *obj);
u32 fn_1_26A4(void);
void fn_1_270C(u32 collisionMask);
s16 fn_1_272C(void);
void fn_1_27A0(void);
void fn_1_29E8(OMOBJ *obj);
void fn_1_2A54(M621Player *player, M621Target *target);
void fn_1_2C6C(M621Player *player, M621Target *target, s16 partIndex);
void fn_1_2E10(M621TargetPart *part);
int fn_1_2E68(COL_NARROW_PARAM *firstCollider, COL_NARROW_PARAM *secondCollider);
int fn_1_2FFC(COL_NARROW_PARAM *firstCollider, COL_NARROW_PARAM *secondCollider);
void fn_1_3024(MGACTOR *actor, int param);
void fn_1_3084(M621Target *target);
void fn_1_3678(OMOBJ *obj);
void fn_1_390C(OMOBJ *obj);
void fn_1_3A78(OMOBJ *obj);
void fn_1_3E18(OMOBJ *obj);
void fn_1_47B4(OMOBJ *obj);
void fn_1_4CB0(OMOBJ *obj);
void fn_1_5054(OMOBJ *obj);
void fn_1_51F4(s16 effectPlayerNo, HuVecF *effectPosition);
void fn_1_52D4(s32 playerNo, s16 coins);
void fn_1_5328(s32 playerNo, s32 score);
s16 fn_1_5340(void);
float fn_1_5350(float value);
double fn_1_537C(double value);

#endif
