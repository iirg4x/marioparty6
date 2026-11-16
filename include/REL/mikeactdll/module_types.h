#ifndef REL_MIKEACTDLL_MODULE_TYPES_H
#define REL_MIKEACTDLL_MODULE_TYPES_H

#include "math.h"
#include "REL/mikeactdll/sequence_context.h"
#include "REL/mikeactdll/mic_selection_context.h"
#include "REL/mikeactdll/effect_spawner.h"
#include "REL/mikeactdll/visibility_target.h"
#include "game/mic.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
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
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
typedef struct {
    HUSPR_GROUPID scene;
    HUSPR_GROUPID progress;
    HUSPR_GROUPID letters;
} MikeActSpriteGroups;

typedef struct {
    MGACTOR *actor;
    struct MikeActPlayerRecord_s *owner;
    Point3d position;
    s16 state;
    u8 unk16[6]; /* No code names this field. */
} MikeActWorldContext;

extern MikeActWorldContext *lbl_1_bss_F0;

typedef struct MikeActPlayerRecord_s {
    MGPLAYER *player;                 /* 0x00: MgPlayerCreate result */
    s16 player_no;                    /* 0x04 */
    s16 character_no;                 /* 0x06 */
    s16 flags;                        /* 0x08: fn_1_45B0/45C4/45D8 */
    s16 movementState;                     /* 0x0A: fn_1_5AD8 */
    s16 speedIndex;                     /* 0x0C: fn_1_5AD8 */
    f32 movement_speed;              /* 0x10: fn_1_52D8 */
    f32 movement_angle;              /* 0x14: fn_1_52D8 */
    s16 speedIncreaseCount;                     /* 0x18: fn_1_5AD8 */
    u8 unk1A[2]; /* No code names this field. */
    s32 motionFrameCount;                     /* 0x1C: fn_1_5AD8 */
    s16 timer;                     /* 0x20: fn_1_5AD8/fn_1_5BC8 */
    s16 duration;                  /* 0x22: fn_1_5BC8 */
    s16 lane;                     /* 0x24: existing fn_1_4A70 */
    s16 targetLane;                     /* 0x26: existing fn_1_4A70 */
    HuVecF previous_position;        /* 0x28 */
    s32 stationaryFrames;                    /* 0x34 */
    s16 actionState;                    /* 0x38 */
    u8 unk3A[2]; /* No code names this field. */
    void (*callback)(struct MikeActPlayerRecord_s *record); /* 0x3C */
    s32 transition_value;            /* 0x40 */
    s16 transition_counter;          /* 0x44 */
    u16 buttons_down;                       /* 0x46: fn_1_52D8 */
    u16 buttons;                       /* 0x48: fn_1_52D8 */
    u8 unk4A[2]; /* No code names this field. */
    u32 lastEffectTick;              /* 0x4C */
    u8 unk50[0x0C]; /* No code names this field. */
} MikeActPlayerRecord;

extern MikeActPlayerRecord *lbl_1_bss_C4[5];
extern MikeActPlayerRecord *lbl_1_bss_B8;
s16 fn_1_477C(MikeActPlayerRecord *record);
f32 fn_1_483C(MikeActPlayerRecord *record, HuVecF *position);
void fn_1_4D6C(MikeActPlayerRecord *record, s32 enabled);
void fn_1_45B0(MikeActPlayerRecord *record, s32 flags);
s32 fn_1_5C0C(MikeActPlayerRecord *record, s16 requestedState);

typedef struct {
    s16 kind;
    s16 flags;
    s16 model_id;
    u8 unk06[2]; /* No code names this field. */
    Point3d position;
    MGACTOR *actor;
    s16 collision_map_index;
    s16 linked_model_id;
    s32 collision_mask;
    s32 event_state;                 /* 0x20: fn_1_124FC and fn_1_1662C */
    s32 callback_state;              /* 0x24: fn_1_124FC, fn_1_2116C */
    u8 unk_28[8]; /* No code names this field. */
} MikeActStateRecord;

/* Shared layout: fn_1_16718 and stage setup read offsets 0, 8, 0x18 and 0x1C. */
typedef struct {
    MGACTOR *actor;
    s16 modelId;
    u8 unknown_06[2]; /* No code names this field. */
    Point3d position;
    u8 unknown_14[4]; /* No code names this field. */
    MikeActStateRecord *state;
    s16 linkedModelId;
    u8 unknown_1E[2]; /* No code names this field. */
} MikeActArenaObject;

extern MikeActStateRecord lbl_1_bss_1B8[0x100];

void MikeActPreActorCallbackSet(void (*callback)(void));
void MikeActPostActorCallbackSet(void (*callback)(void));
void fn_1_E0B8(s32 value);
void fn_1_3608(void);
void fn_1_6000(MikeActPlayerRecord *context);
void fn_1_20808(OMOBJ *object);
void fn_1_40A0(s16 playerIndex);
void fn_1_534(void);
void fn_1_598(void);
s16 fn_1_1C5C(void);
s32 fn_1_1F70(void);
void fn_1_21A0(void);
void fn_1_36E4(s16 value);
void fn_1_3CE8(void);
void fn_1_3D50(Mtx output);
void fn_1_3FC4(void);
void fn_1_4C84(Point3d *positions);
void fn_1_5B4C(MikeActPlayerRecord *record, s16 state);
void fn_1_5E98(MikeActPlayerRecord *turnState);
void fn_1_5F48(MikeActPlayerRecord *turnState);
void fn_1_60E4(MikeActPlayerRecord *record);
void fn_1_1277C(HuVecF *position);
void fn_1_16910(HuVecF *position);
void fn_1_14B68(void);
void fn_1_1DCF4(MGACTOR *actor);
void fn_1_49D0(MikeActPlayerRecord *record);
s32 fn_1_4A20(void);
void fn_1_4F3C(MikeActPlayerRecord *record);
void fn_1_5AD8(MikeActPlayerRecord *record, s16 state);
void fn_1_5BC8(MikeActPlayerRecord *record, s32 enabled);
void fn_1_69EC(MikeActPlayerRecord *record, s16 selectionNo);
void fn_1_1998(void);
void fn_1_20188(OMOBJ *object);
void fn_1_DCEC(MikeActPlayerRecord *record);
void fn_1_DDFC(void);
void fn_1_D5F4(s16 modelNo, HuVecF *position, s16 display);
void fn_1_55E8(MikeActPlayerRecord *record, s16 action);
void fn_1_4A70(MikeActPlayerRecord *record);
void fn_1_2116C(OMOBJ *object);
void fn_1_1FD4(void);
void fn_1_1E0C4(OMOBJ *object);
void fn_1_1E29C(void);
void ObjectSetup(void);
void fn_1_20E8(void);
void fn_1_2164(s32 pauseEnabled);
s32 fn_1_30E4(s32 soundId);
void fn_1_3128(s32 soundId);
void fn_1_317C(s16 streamId);
void fn_1_31C8(void);
void fn_1_3208(void);
void fn_1_34C0(void);
void fn_1_35A0(s16 duration);
void fn_1_45C4(MikeActPlayerRecord *record, s32 flags);
s32 fn_1_45D8(MikeActPlayerRecord *record, s32 flags);
void fn_1_45E4(MikeActPlayerRecord *record);
void fn_1_665C(MikeActPlayerRecord *record, s16 motionIndex);
void fn_1_45A0(void (*callback)(void *, s32));
void fn_1_E308(void);
void fn_1_E648(s32 state);
void fn_1_EA08(s32 mode);
void fn_1_EACC(void);
void fn_1_F0B8(void);
s16 fn_1_F138(void);
void fn_1_F148(void);
void fn_1_F188(void);
void fn_1_F1F8(void);
void fn_1_F2D8(s32 visible);
void fn_1_F370(u32 messageNo, u16 insertNo);
void fn_1_F420(u32 messageNo, u16 insertNo);
void fn_1_F4F4(u32 messageNo);
void fn_1_F598(void);
void fn_1_F5BC(void);
s16 fn_1_FC80(s32 selection);
void fn_1_FE94(s32 visible);
void fn_1_101C0(s16 mode);
void fn_1_10770(s16 selectionNo);
s16 fn_1_107EC(s16 modelId);
s16 fn_1_1092C(MGACTOR *actor, Point3d *position);
s16 fn_1_10AE0(s16 sourceModel, s16 targetModel, Point3d *position);
s16 fn_1_10CFC(s16 modelId, Point3d *position);
void fn_1_10F38(s16 playerNo, Point3d *out);
void fn_1_10F98(s16 index, s32 collisionMask);
void fn_1_10FB4(s16 index);
s16 fn_1_1106C(u32 value);
s16 fn_1_1110C(s16 state);
void fn_1_111A8(Point3d *positions, s32 count);
void fn_1_114A8(s16 collisionMap);
void fn_1_11690(Point3d *positions, s32 count);
void fn_1_11B44(void);
void fn_1_12690(OMOBJ *object);
s32 fn_1_15BA4(MGACTOR *actor);
void fn_1_1A234(void);
s32 fn_1_1B07C(void);
void fn_1_1B2B8(void);
s32 fn_1_1C880(void);
void fn_1_1DB7C(OMOBJ *object);
void fn_1_1E78(void);
void fn_1_2390(void);

void fn_1_17C4(MCResponse_s *update);

typedef struct {
    u16 motionIndex;
    u16 overlayIndex;
    f32 speed;
    u32 attributes;
} MikeActMotionEntry;

void fn_1_6124(MGPLAYER *context, s16 motionIndex);

extern f32 lbl_1_data_418[5];
extern MikeActMotionEntry lbl_1_data_310[0x16];

extern char *lbl_1_data_58[3];
extern Point3d lbl_1_data_64[4];
extern OM_CAMERA_VIEW lbl_1_data_94;
extern s16 lbl_1_bss_E;

void fn_1_1BF4(void);

void fn_1_1B40(s16 playerNo, MCResponseCallback callback);

void fn_1_1E1C(s32 messageNo, s16 itemNo);

s32 fn_1_120E8(MGACTOR *actor);

int fn_1_12380(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b);

int fn_1_12388(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b);

int fn_1_12390(COL_NARROW_PARAM *effectRequest, COL_NARROW_PARAM *playerRequest);

int fn_1_124FC(COL_NARROW_PARAM *event,
               COL_NARROW_PARAM *playerContext);

void fn_1_131A4(OMOBJ *object);

void fn_1_13A8C(void);

void fn_1_1381C(void);

void fn_1_12804(void);

void fn_1_12E48(void);

s16 fn_1_15818(Point3d *position);

void fn_1_10F68(s16 playerNo, const Point3d *in);

s32 fn_1_13C2C(void);

void fn_1_14970(MCResponse_s *context);

void fn_1_151D4(MCResponse_s *context);

void fn_1_1AE4(void);

void fn_1_3150(s16 streamId);

void fn_1_1CDEC(void);

void fn_1_21304(OMOBJ *object);

int fn_1_1DEE0(COL_NARROW_PARAM *eventSource, COL_NARROW_PARAM *eventContext);

void fn_1_1D594(void);

void fn_1_1ED4C(MCResponse_s *context);

void fn_1_1FFE8(MCResponse_s *callbackContext);

s32 fn_1_1FB7C(void);

void fn_1_205EC(MikeActEffectSpawner *spawner);

void fn_1_217BC(MikeActVisibilityTarget *target);

void fn_1_21954(MikeActVisibilityTarget *target);

/* Release debug hook; discarded comparisons still evaluate their operands. */
static inline void DebugValue(s32 value) { }

#endif
