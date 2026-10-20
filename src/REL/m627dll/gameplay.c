/* Runs the minigame's map, player, camera and moving-object updates. */
#include <math.h>
#include "REL/m627dll/gameplay.h"
#include "game/main.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/frand.h"
#include "game/flag.h"
#include "game/wipe.h"
#include "game/mg/seqman.h"
#include "game/charman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "datadir_enum.h"
#include "msm_se.h"
#include <string.h>

#define MSM_SE_M627_ACTOR_START 1807
#define MSM_SE_M627_ENTRY_FIRST_SCREEN 1808
#define MSM_SE_M627_ENTRY_SECOND_SCREEN 1809
#define MSM_SE_M627_LANDING_FIRST_SCREEN 1810
#define MSM_SE_M627_BOUNDARY_HIT 1812
#define MSM_SE_M627_TURN_FIRST_SCREEN 1813
#define MSM_SE_M627_LANDING_SECOND_SCREEN 1816
#define MSM_SE_M627_TURN_SECOND_SCREEN 1818

extern OMOBJMAN *lbl_1_bss_0;
void *fn_1_A0(s32 priority, u32 bytes, OMOBJ_FUNC update);
void fn_1_140(OMOBJ *object, OMOBJ_FUNC callback);
u32 __cvt_fp2unsigned(f64 value);

s32 lbl_1_data_30[7] = { DATANUM(DATA_m627, 1), DATANUM(DATA_m627, 2),
    DATANUM(DATA_m627, 3), DATANUM(DATA_m627, 4), DATANUM(DATA_m627, 5),
    DATANUM(DATA_m627, 6), DATANUM(DATA_m627, 0) };
s32 lbl_1_data_4C[2] = { DATANUM(DATA_m627, 7), DATANUM(DATA_m627, 8) };
s32 lbl_1_data_54[2] = { DATANUM(DATA_m627, 9), DATANUM(DATA_m627, 10) };

s16 lbl_1_bss_30;
M627MapWork *lbl_1_bss_2C;
s16 lbl_1_bss_28;
M627PlayerWork *lbl_1_bss_18[4];
s32 lbl_1_bss_14;
s16 lbl_1_bss_10;
M627ActorWork *lbl_1_bss_8[2];

s32 fn_1_430(s16 viewport, Point3d *position, s16 soundId);
void fn_1_5B8(s16 viewport, Point3d *position, s32 soundHandle);
void fn_1_73C(s32 soundHandle);
s32 fn_1_764(s16 pan, s16 soundId);
void fn_1_798(OMOBJ *object);
void fn_1_8DC(OMOBJ *obj);
void fn_1_92C(OMOBJ *obj);
void fn_1_A64(OMOBJ *object);
void fn_1_BE8(OMOBJ *obj);
void fn_1_C50(OMOBJ *object);
void fn_1_DD8(OMOBJ *object);
void fn_1_11F8(OMOBJ *obj);
s16 fn_1_12CC(M627CameraState *work, s16 row, f32 *result);
void fn_1_1508(OMOBJ *obj);
void fn_1_1C70(OMOBJ *obj);
void fn_1_21B8(OMOBJ *obj);
s16 fn_1_23F8(const u8 grid[30][11], s16 *singleCountOut, s16 *groupCountOut);
void fn_1_24F8(M627BoundaryLine *line, HuVecF *origin, HuVecF *end, s32 type);
void fn_1_25E4(const u8 grid[30][11], M627MapWork *setup);
void fn_1_2B58(M627MapWork *map);
void fn_1_2BF4(OMOBJ *object);
void fn_1_30B0(OMOBJ *obj);
void fn_1_3184(OMOBJ *obj);
void fn_1_32A4(s16 cameraGroup, M627Entry *work);
void fn_1_3590(MGPLAYER *player);
void fn_1_37C8(MGPLAYER *player);
void fn_1_3924(M627PlayerWork *work, s32 motionSlot, f32 blendFrames, s32 attr);
int fn_1_17188__12CallbackBaseFv(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b);
void fn_1_39A8(OMOBJ *object);
void fn_1_3D6C(OMOBJ *obj);
void fn_1_3DC8(OMOBJ *obj);
void fn_1_4070(OMOBJ *obj);
void *fn_1_40CC(M627PlayerWork *state);
void fn_1_42D4(OMOBJ *object);
void fn_1_492C(OMOBJ *obj);
void fn_1_49B8(OMOBJ *object);
void fn_1_4B04(OMOBJ *object);
void fn_1_4C80(OMOBJ *controller);
void fn_1_4D54(M627PlayerWork *state, M627Entry *entry);
s32 fn_1_4DB4(M627PlayerWork *work, M627Entry *entry);
void fn_1_4DDC(M627PlayerWork *work);
M627Entry *fn_1_4DEC(M627PlayerWork *state);
M627Entry *fn_1_4F24(M627PlayerWork *state, s16 reverseTest, f32 maxZ);
M627Entry *fn_1_5090(M627PlayerWork *state);
M627Entry *fn_1_5248(M627PlayerWork *state);
s16 fn_1_5718(M627PlayerWork *state);
void fn_1_58A8(M627PlayerWork *state, s16 action);
void fn_1_5E9C(OMOBJ *object);
void fn_1_6CE0(OMOBJ *object);
void fn_1_6F60(OMOBJ *object);
void fn_1_7020(OMOBJ *object);
s32 fn_1_7274(Point3d *sphereCenter, Point3d *pointStart, Point3d *direction, f32 radius,
              f32 *intersectionTime);
s16 fn_1_7580(Point3d *origin, Point3d *projectionAxis, Point3d *boundaryNormal,
               Point3d *sweepStart, Point3d *sweepDelta, f32 maxProjection, f32 *hitParameter);
s16 fn_1_7808(Point3d *queryStart, Point3d *movement,
              f32 boundaryOffset, M627CollisionHit *result);
s32 fn_1_7918(Point3d *circleCenter, Point3d *point, Point3d *direction, f32 radiusSquared,
              f32 *intersectionTime);
s16 fn_1_7B98(Point3d *start, Point3d *direction, f32 radius,
              Point3d *contactNormal, f32 *intersectionTime);
s16 fn_1_8098(s32 cameraGroup, Point3d *start, Point3d *direction,
              M627CollisionHit *hit);
void fn_1_8B40(OMOBJ *object);
void fn_1_9A2C(OMOBJ *object);
void fn_1_AB0C(OMOBJ *obj);

u8 lbl_1_data_5C[10][30][11] = {
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
        { 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0 },
        { 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0 },
        { 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1 },
        { 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0 },
        { 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1 },
        { 1, 1, 1, 0, 0, 1, 2, 2, 1, 1, 1 },
        { 0, 1, 0, 0, 0, 1, 2, 2, 1, 0, 0 },
        { 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0 },
        { 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0 },
        { 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1 },
        { 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0 },
        { 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1 },
        { 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1 },
        { 0, 1, 0, 1, 2, 2, 0, 1, 1, 0, 0 },
        { 0, 0, 1, 0, 2, 2, 0, 1, 0, 1, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0 },
        { 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1 },
        { 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
        { 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0 },
        { 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0 },
        { 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0 },
        { 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
        { 0, 1, 0, 2, 2, 0, 0, 1, 0, 0, 1 },
        { 1, 0, 1, 2, 2, 0, 0, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
        { 0, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0 },
        { 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 },
        { 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0 },
        { 0, 1, 1, 1, 0, 2, 2, 0, 0, 1, 0 },
        { 1, 0, 1, 1, 0, 2, 2, 0, 0, 0, 1 },
        { 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0 },
        { 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0 },
        { 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
        { 0, 1, 0, 1, 0, 0, 2, 2, 0, 0, 0 },
        { 0, 0, 1, 1, 0, 0, 2, 2, 1, 0, 0 },
        { 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0 },
        { 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0 },
        { 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1 },
        { 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0 },
        { 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
        { 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0 },
        { 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0 },
        { 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 2, 2, 0, 0, 1, 0, 1 },
        { 1, 0, 1, 0, 2, 2, 0, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
        { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1 },
        { 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0 },
        { 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0 },
        { 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1 },
        { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0 },
        { 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
        { 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1 },
        { 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1 },
        { 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0 },
        { 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1 },
        { 1, 0, 0, 0, 2, 2, 1, 0, 1, 0, 0 },
        { 0, 1, 0, 0, 2, 2, 1, 0, 0, 1, 0 },
        { 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1 },
        { 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
        { 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0 },
        { 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
        { 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0 },
        { 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 0, 0, 2, 2, 1, 0, 1 },
        { 1, 0, 1, 0, 1, 0, 2, 2, 0, 1, 0 },
        { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1 },
        { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0 },
        { 0, 1, 0, 1, 0, 2, 2, 0, 1, 0, 0 },
        { 1, 0, 0, 1, 0, 2, 2, 0, 1, 0, 1 },
        { 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0 },
        { 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1 },
        { 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1 },
        { 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0 },
        { 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1 },
        { 1, 0, 0, 2, 2, 1, 1, 0, 1, 0, 0 },
        { 0, 1, 0, 2, 2, 0, 1, 0, 0, 1, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1 },
        { 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
        { 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0 },
        { 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
        { 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0 },
        { 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0 },
        { 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1 },
        { 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0 },
        { 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1 },
        { 1, 1, 1, 0, 0, 0, 2, 2, 1, 1, 1 },
        { 0, 1, 0, 1, 0, 1, 2, 2, 1, 0, 0 },
        { 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0 },
        { 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0 },
        { 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1 },
        { 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0 },
        { 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1 },
        { 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1 },
        { 0, 1, 0, 0, 2, 2, 0, 1, 0, 0, 0 },
        { 0, 0, 1, 0, 2, 2, 0, 1, 0, 1, 1 },
        { 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1 },
        { 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
        { 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0 },
        { 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1 },
        { 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0 },
        { 1, 1, 1, 2, 2, 1, 0, 0, 1, 1, 1 },
        { 0, 0, 1, 2, 2, 1, 0, 0, 0, 1, 0 },
        { 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0 },
        { 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0 },
        { 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1 },
        { 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0 },
        { 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0 },
        { 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 1, 1, 0, 2, 2, 1, 0, 1, 0 },
        { 1, 1, 0, 1, 0, 2, 2, 0, 1, 0, 0 },
        { 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1 },
        { 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0 },
        { 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0 },
        { 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1 },
        { 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0 },
        { 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0 },
        { 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0 },
        { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 1, 0, 0, 1, 0, 0, 2, 2, 0, 1, 0 },
        { 0, 1, 0, 0, 0, 0, 2, 2, 1, 0, 1 },
        { 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0 },
        { 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0 },
        { 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0 },
        { 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0 },
        { 0, 1, 0, 0, 2, 2, 0, 1, 1, 1, 0 },
        { 1, 0, 0, 0, 2, 2, 0, 1, 1, 0, 1 },
        { 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0 },
        { 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0 },
        { 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0 },
        { 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 0, 0, 2, 2, 0, 0, 1, 0, 1, 0 },
        { 0, 0, 1, 2, 2, 0, 0, 1, 1, 0, 0 },
        { 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1 },
        { 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0 },
        { 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0 },
        { 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0 },
        { 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
        { 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0 },
        { 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0 },
        { 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0 },
        { 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
        { 1, 0, 1, 0, 0, 2, 2, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 0, 2, 2, 0, 1, 0, 1 },
        { 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0 },
        { 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 },
        { 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0 },
        { 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0 },
        { 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0 },
        { 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1 },
        { 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0 },
        { 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0 },
        { 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0 },
        { 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0 },
        { 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0 },
        { 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0 },
        { 0, 0, 1, 0, 1, 2, 2, 0, 0, 0, 1 },
        { 0, 1, 0, 0, 1, 2, 2, 0, 0, 1, 0 },
        { 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1 },
        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0 },
        { 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
        { 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0 },
        { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
        { 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0 },
        { 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
        { 1, 0, 1, 2, 2, 0, 0, 0, 0, 1, 0 },
        { 0, 1, 0, 2, 2, 0, 1, 0, 1, 0, 1 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
        { 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1 },
        { 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0 },
        { 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 0, 1, 0, 2, 2, 0, 1, 0, 1, 0 },
        { 1, 0, 1, 0, 2, 2, 0, 1, 0, 0, 1 },
        { 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0 },
        { 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0 },
        { 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0 },
        { 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0 },
        { 0, 0, 1, 0, 1, 1, 2, 2, 0, 0, 1 },
        { 0, 1, 0, 0, 1, 0, 2, 2, 0, 1, 0 },
        { 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0 },
        { 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    },
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0 },
        { 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1 },
        { 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0 },
        { 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0 },
        { 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0 },
        { 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0 },
        { 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0 },
        { 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1 },
        { 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0 },
        { 1, 1, 1, 2, 2, 0, 0, 0, 1, 1, 1 },
        { 0, 0, 1, 2, 2, 1, 0, 1, 0, 1, 0 },
        { 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0 },
        { 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0 },
        { 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1 },
        { 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0 },
        { 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0 },
        { 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0 },
        { 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0 },
        { 0, 0, 0, 1, 0, 2, 2, 0, 0, 1, 0 },
        { 1, 1, 0, 1, 0, 2, 2, 0, 1, 0, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
        { 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0 },
        { 1, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0 },
        { 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0 },
        { 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
    }
};

s16 lbl_1_data_D40 = 10;

Vec lbl_1_data_D44[10] = {
    { -550.0f, 0.0f, -60.0f },
    { 550.0f, 0.0f, -60.0f },
    { 550.0f, 0.0f, 2842.9f },
    { 250.0f, 0.0f, 3131.57f },
    { 250.0f, 0.0f, 3416.47f },
    { 200.2f, 0.0f, 3600.0f },
    { -200.2f, 0.0f, 3600.0f },
    { -250.0f, 0.0f, 3416.47f },
    { -250.0f, 0.0f, 3131.57f },
    { -550.0f, 0.0f, 2842.9f }
};

s16 lbl_1_data_DBC[18] = { 9, 8, 8, 7, 7, 6, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0, 9 };

M627TaperedCollisionVolume lbl_1_data_DE0 = {
    { 0.0f, -1942.1f, 3292.98f },
    { 0.0f, -2181.4f, 3292.98f },
    300.0f, 200.0f, 22.6793f
};

Vec lbl_1_data_E04 = { 0.0f, 2598.0762f, 1500.0f };
Vec lbl_1_data_E10 = { 0.0f, 1.0f, 0.0f };
Vec lbl_1_data_E1C = { 0.0f, 0.0f, 3000.0f };
Vec lbl_1_data_E28 = { 0.0f, 0.0f, 0.0f };
Vec lbl_1_data_E34 = { 0.0f, 0.0f, 3300.0f };
Vec lbl_1_data_E40 = { 0.0f, 0.0f, 1420.0f };
Vec lbl_1_data_E4C = { 0.0f, 766.044f, 642.7876f };

Vec lbl_1_data_E58[4] = {
    { -300.0f, 1.0f, 0.0f },
    { -450.0f, 1.0f, 0.0f },
    { -300.0f, 1.0f, 0.0f },
    { 450.0f, 1.0f, 0.0f }
};

Vec lbl_1_data_E88[4] = {
    { 0.0f, 1.0f, 1420.0f },
    { 0.0f, 1.0f, 1310.0f },
    { -135.0f, 1.0f, 1470.0f },
    { 135.0f, 1.0f, 1470.0f }
};

unsigned int lbl_1_data_EB8[7] = {
    DATANUM(DATA_mario, 90), DATANUM(DATA_mario, 91), DATANUM(DATA_mario, 160),
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 6), DATANUM(DATA_mariomot, 40), 0
};

MGPLAYER_MODE_FUNC lbl_1_data_ED4[18] = { fn_1_3590, NULL, NULL, fn_1_37C8 };

Vec lbl_1_data_F1C[2] = {
    { 0.0f, 0.0f, 200.0f },
    { 0.0f, 0.0f, 200.0f }
};

char *lbl_1_data_F68[4] = { "stnull-grid1", "stnull-grid4", "stnull-grid2", "stnull-grid3" };
char lbl_1_data_F78[] = "ball200cm";
char lbl_1_data_F82[] = "ball350cm";

/* Called by collision and entry updates to start an effect panned for the player's screen side. */
s32 fn_1_430(s16 viewport, Point3d *position, s16 soundId)
{
    Point3d screenPos;
    s16 pan;

    Hu3D3Dto2D(position, viewport, &screenPos);
    if (viewport == 1) {
        if (screenPos.x > 316.0f) {
            pan = 64;
        } else if (screenPos.x < 16.0f) {
            pan = 32;
        } else {
            pan = (s16)(32.0f + ((32.0f *
                (screenPos.x - 16.0f)) / 300.0f));
        }
    } else if (screenPos.x > 624.0f) {
        pan = 96;
    } else if (screenPos.x < 324.0f) {
        pan = 64;
    } else {
        pan = (s16)(64.0f + ((32.0f *
            (screenPos.x - 324.0f)) / 300.0f));
    }
    return HuAudFXPlayVolPan(soundId, 127, pan);
}

/* Called by moving-actor updates to keep an active effect panned as its actor moves onscreen. */
void fn_1_5B8(s16 viewport, Point3d *position, s32 soundHandle)
{
    Point3d screenPos;
    s16 pan;

    Hu3D3Dto2D(position, viewport, &screenPos);
    if (viewport == 1) {
        if (screenPos.x > 316.0f) {
            pan = 64;
        } else if (screenPos.x < 16.0f) {
            pan = 32;
        } else {
            pan = (s16)(32.0f + ((32.0f *
                (screenPos.x - 16.0f)) / 300.0f));
        }
    } else if (screenPos.x > 624.0f) {
        pan = 96;
    } else if (screenPos.x < 324.0f) {
        pan = 64;
    } else {
        pan = (s16)(64.0f + ((32.0f *
            (screenPos.x - 324.0f)) / 300.0f));
    }
    HuAudFXPanning(soundHandle, pan);
}

void fn_1_73C(s32 soundHandle)
{
    HuAudFXStop(soundHandle);
}

s32 fn_1_764(s16 pan, s16 soundId)
{
    return HuAudFXPlayVolPan(soundId, 127, pan);
}

/* Called at the sequence start to create the player, map and camera update objects. */
void fn_1_798(OMOBJ *sequenceObject)
{
    s32 playerOrCameraIndex;

    fn_1_A0(10, 108, fn_1_A64);
    lbl_1_bss_2C = fn_1_A0(10, 36, fn_1_2BF4);
    lbl_1_bss_14 = 1;

    for (playerOrCameraIndex = 0; playerOrCameraIndex < 4; playerOrCameraIndex++) {
        lbl_1_bss_18[playerOrCameraIndex] = fn_1_A0(20, 100, fn_1_39A8);
        lbl_1_bss_18[playerOrCameraIndex]->playerIndex = (s16)playerOrCameraIndex;
    }

    for (playerOrCameraIndex = 0; playerOrCameraIndex < 2; playerOrCameraIndex++) {
        lbl_1_bss_8[playerOrCameraIndex] = fn_1_A0(20, 52, fn_1_6CE0);
        lbl_1_bss_8[playerOrCameraIndex]->cameraGroup = (s16)playerOrCameraIndex;
    }

    Hu3DReflectNoSet(1);
    lbl_1_bss_10 = -1;
    fn_1_140(sequenceObject, fn_1_8DC);
}

/* Mark unfilled winner character slots with -1 before recording the winning group. */
static const M627WinnerList winnerDefaults = {{ -1, -1, -1, -1 }};

/* At opening phase 2, installs the callback that waits for result mode. */
void fn_1_8DC(OMOBJ *sequenceObject)
{
    M627CameraState *sequenceData = sequenceObject->data;

    if (MgSeqModeGet() == 2) {
        fn_1_140(sequenceObject, fn_1_92C);
        return;
    }
}

/* Installed by fn_1_8DC; records active winners and awards coins when result mode begins. */
void fn_1_92C(OMOBJ *sequenceObject)
{
    M627WinnerList winner;
    s32 winnerCount;
    s32 playerIndex;
    void *sequenceData;

    sequenceData = sequenceObject->data;
    if (MgSeqModeGet() == 6) {
        winner = winnerDefaults;
        winnerCount = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (lbl_1_bss_18[playerIndex]->active == 1) {
                winner.characterIds[winnerCount] = lbl_1_bss_18[playerIndex]->player->charNo;
                winnerCount++;
                if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                    /* Non-practice games receive the result coin bonus. */
                    GwPlayer[playerIndex].mgCoinBonus = 10;
                }
            }
        }
        MgSeqWinnerSet(winner.characterIds[0], winner.characterIds[1],
                       winner.characterIds[2], winner.characterIds[3]);
        fn_1_140(sequenceObject, NULL);
        return;
    }
}

/* Created by fn_1_798; initializes the opening camera before its timed entrance moves. */
void fn_1_A64(OMOBJ *cameraObject)
{
    M627CameraState *cameraState;
    cameraState = cameraObject->data;
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(3, 45.0f, 20.0f, 15000.0f, 1.2f);
    Hu3DCameraViewportSet(3, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    cameraState->cameraTarget[0].x = lbl_1_data_E1C.x;
    cameraState->cameraTarget[0].y = 0.0f;
    cameraState->cameraTarget[0].z = lbl_1_data_E1C.z;
    cameraState->cameraPosition[0].x = cameraState->cameraTarget[0].x + lbl_1_data_E04.x;
    cameraState->cameraPosition[0].y = cameraState->cameraTarget[0].y + lbl_1_data_E04.y;
    cameraState->cameraPosition[0].z = cameraState->cameraTarget[0].z + lbl_1_data_E04.z;
    Hu3DCameraPosSetV(1, &cameraState->cameraPosition[0], &lbl_1_data_E10,
                      &cameraState->cameraTarget[0]);
    Hu3DCameraScissorSet(2, 0U, 0U, 0U, 0U);
    cameraState->elapsedFrames = 0;
    fn_1_140(cameraObject, fn_1_BE8);
}

/* Called after fn_1_A64; holds the opening camera for 31 updates before its pan. */
void fn_1_BE8(OMOBJ *cameraObject)
{
    M627CameraState *cameraState = cameraObject->data;

    cameraState->elapsedFrames++;
    if (cameraState->elapsedFrames > 30) {
        cameraState->elapsedFrames = 0;
        fn_1_140(cameraObject, fn_1_C50);
        return;
    }
}

/* Called after fn_1_BE8; pans across the playfield before the split-screen entrance. */
void fn_1_C50(OMOBJ *cameraObject)
{
    M627CameraState *cameraState = cameraObject->data;
    f32 progress;

    progress = (f32)cameraState->elapsedFrames / 90.0f;
    cameraState->cameraTarget[0].x =
        lbl_1_data_E1C.x + progress * (lbl_1_data_E28.x - lbl_1_data_E1C.x);
    cameraState->cameraTarget[0].y = 0.0f;
    cameraState->cameraTarget[0].z =
        lbl_1_data_E1C.z + progress * (lbl_1_data_E28.z - lbl_1_data_E1C.z);
    cameraState->cameraPosition[0].x = cameraState->cameraTarget[0].x + lbl_1_data_E04.x;
    cameraState->cameraPosition[0].y = cameraState->cameraTarget[0].y + lbl_1_data_E04.y;
    cameraState->cameraPosition[0].z = cameraState->cameraTarget[0].z + lbl_1_data_E04.z;
    Hu3DCameraPosSetV(1, &cameraState->cameraPosition[0], &lbl_1_data_E10,
                      &cameraState->cameraTarget[0]);

    cameraState->elapsedFrames++;
    if (cameraState->elapsedFrames > 90) {
        cameraState->elapsedFrames = 0;
        fn_1_140(cameraObject, fn_1_DD8);
        return;
    }
}

/* Called after fn_1_C50; opens the second camera and reshapes both viewports into split-screen. */
void fn_1_DD8(OMOBJ *cameraObject)
{
    M627CameraState *cameraState = cameraObject->data;
    f32 progress;
    f32 splitBoundaryX;
    f32 cameraTwoWidth;
    f32 cameraOneViewportX;
    f32 cameraTwoViewportX;
    u32 cameraTwoScissorWidth;

    progress = (f32)cameraState->elapsedFrames / 60.0f;
    splitBoundaryX = 640.0f + (-320.0f * progress);
    cameraTwoWidth = 640.0f * progress;
    cameraOneViewportX =
        (640.0f + (-304.0f * progress)) / 2.0f - 320.0f;
    cameraTwoViewportX =
        (640.0f - ((3.0f * cameraTwoWidth) / 4.0f)) -
        (8.0f * progress);

    if (cameraState->elapsedFrames == 0) {
        cameraState->cameraTarget[0].x = lbl_1_data_E28.x;
        cameraState->cameraTarget[0].y = 0.0f;
        cameraState->cameraTarget[0].z = lbl_1_data_E28.z;
        cameraState->cameraPosition[0].x = cameraState->cameraTarget[0].x + lbl_1_data_E04.x;
        cameraState->cameraPosition[0].y = cameraState->cameraTarget[0].y + lbl_1_data_E04.y;
        cameraState->cameraPosition[0].z = cameraState->cameraTarget[0].z + lbl_1_data_E04.z;
        Hu3DCameraPosSetV(1, &cameraState->cameraPosition[0], &lbl_1_data_E10,
                          &cameraState->cameraTarget[0]);

        cameraState->cameraTarget[1].x = lbl_1_data_E28.x;
        cameraState->cameraTarget[1].y = 0.0f;
        cameraState->cameraTarget[1].z = lbl_1_data_E28.z;
        cameraState->cameraPosition[1].x = cameraState->cameraTarget[1].x + lbl_1_data_E04.x;
        cameraState->cameraPosition[1].y = cameraState->cameraTarget[1].y + lbl_1_data_E04.y;
        cameraState->cameraPosition[1].z = cameraState->cameraTarget[1].z + lbl_1_data_E04.z;
        Hu3DCameraPosSetV(2, &cameraState->cameraPosition[1], &lbl_1_data_E10,
                          &cameraState->cameraTarget[1]);
    }

    Hu3DCameraViewportSet(1, cameraOneViewportX, 0.0f,
                          640.0f, 480.0f,
                          0.0f, 1.0f);
    if (splitBoundaryX > 2.0f) {
        Hu3DCameraScissorSet(1, 0, 0, __cvt_fp2unsigned(splitBoundaryX) - 2, 480);
    } else {
        Hu3DCameraScissorSet(1, 0, 0, 0, 480);
    }

    if (cameraState->elapsedFrames >= 1) {
        Hu3DCameraViewportSet(2, cameraTwoViewportX, 0.0f,
                              cameraTwoWidth, 480.0f,
                              0.0f, 1.0f);
        if (splitBoundaryX > 2.0f) {
            /* Inset each scissor edge by two pixels, leaving a four-pixel gap. */
            cameraTwoScissorWidth = 638 - __cvt_fp2unsigned(splitBoundaryX);
            Hu3DCameraScissorSet(2, __cvt_fp2unsigned(splitBoundaryX) + 2, 0,
                                 cameraTwoScissorWidth, 480);
        } else {
            Hu3DCameraScissorSet(2, 0, 0, 0, 480);
        }
    }

    cameraState->elapsedFrames++;
    if (cameraState->elapsedFrames > 60) {
        cameraState->elapsedFrames = 0;
        fn_1_140(cameraObject, fn_1_11F8);
        return;
    }
}

/* Called after fn_1_DD8; waits for active play, clears camera history, then starts tracking. */
void fn_1_11F8(OMOBJ *cameraObject)
{
    M627CameraState *cameraState;
    s32 row;
    s32 column;

    if (MgSeqModeGet() == 5) {
        cameraState = cameraObject->data;
        cameraState->elapsedFrames = 0;
        for (row = 0; row < 2; row++) {
            for (column = 0; column < 5; column++) {
                cameraState->values[row][column] = 0.0f;
            }
            cameraState->rowValue[row] = 0.0f;
            cameraState->startingColumn = 0;
        }
        fn_1_140(cameraObject, fn_1_1508);
        return;
    }
}

/* Used by fn_1_1508 to reject five-sample camera trends whose deltas change direction. */
s16 fn_1_12CC(M627CameraState *cameraState, s16 row, f32 *extremeOut)
{
    s16 direction = 0;
    s16 sampleOffset;
    f32 extreme = 0.0f;

    for (sampleOffset = 0; sampleOffset < 5; sampleOffset++) {
        s16 sampleIndex = cameraState->startingColumn - sampleOffset;
        if (sampleIndex < 0) {
            sampleIndex += 5;
        }
        if (direction == 0) {
            if (cameraState->values[row][sampleIndex] < 0.0f) {
                direction = -1;
                extreme = cameraState->values[row][sampleIndex];
                continue;
            }
            if (cameraState->values[row][sampleIndex] > 0.0f) {
                direction = 1;
                extreme = cameraState->values[row][sampleIndex];
                continue;
            }
        } else if (direction < 0) {
            if (cameraState->values[row][sampleIndex] > 0.0f) {
                return 0;
            }
            if (cameraState->values[row][sampleIndex] < extreme) {
                extreme = cameraState->values[row][sampleIndex];
                continue;
            }
        } else if (direction > 0) {
            if (cameraState->values[row][sampleIndex] < 0.0f) {
                return 0;
            }
            if (cameraState->values[row][sampleIndex] > extreme) {
                extreme = cameraState->values[row][sampleIndex];
            }
        }
    }
    *extremeOut = extreme;
    return 1;
}

/* Installed by fn_1_11F8; updates both player-group cameras and shadows on each play tick. */
void fn_1_1508(OMOBJ *cameraObject)
{
    M627CameraState *cameraState = cameraObject->data;
    Point3d shadowEye;
    Point3d shadowUp;
    Point3d shadowTarget;
    Vec savedCameraTarget;
    Vec savedCameraPosition;
    Point3d playerPosition;
    Point3d screenPosition;
    f32 cameraTrend;
    f32 minimumPlayerZ;
    f32 playerGroupCenterZ;
    f32 maximumPlayerZ;
    s32 playerIndex;
    s16 activeIndex;

    if (MgSeqModeGet() == 6) {
        if (lbl_1_bss_10 == -1) {
            fn_1_140(cameraObject, NULL);
            return;
        }
        if (lbl_1_bss_10 == 0) {
            lbl_1_bss_28 = 1;
        } else {
            lbl_1_bss_28 = 2;
        }
        cameraState->elapsedFrames = 0;
        fn_1_140(cameraObject, fn_1_1C70);
        return;
    }

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_18[playerIndex]->cameraGroup == 0) {
            cameraState->values[0][cameraState->startingColumn] =
                lbl_1_bss_18[playerIndex]->player->actor->pos.z - cameraState->rowValue[0];
            cameraState->rowValue[0] =
                lbl_1_bss_18[playerIndex]->player->actor->pos.z;
            if (fn_1_12CC((M627CameraState *)cameraState, 0, &cameraTrend)) {
                if (cameraTrend > 2.0f) {
                    cameraState->cameraTarget[0].z +=
                        cameraTrend > 10.0f ? 10.0f : cameraTrend;
                } else if (cameraTrend < -2.0f) {
                    cameraState->cameraTarget[0].z +=
                        cameraTrend < -10.0f ? -10.0f : cameraTrend;
                }
                cameraState->cameraPosition[0].y = lbl_1_data_E04.y;
                cameraState->cameraPosition[0].z =
                    cameraState->cameraTarget[0].z + lbl_1_data_E04.z;
                Hu3DCameraPosSetV(1, &cameraState->cameraPosition[0], &lbl_1_data_E10,
                                  &cameraState->cameraTarget[0]);
                break;
            } else {
                break;
            }
        }
    }

    shadowUp.x = 0.0f;
    shadowUp.y = 1.0f;
    shadowUp.z = 0.0f;
    shadowTarget = cameraState->cameraTarget[0];
    shadowEye.x = shadowTarget.x - 100.0f;
    shadowEye.y = 8000.0f + shadowTarget.y;
    shadowEye.z = shadowTarget.z;
    Hu3DShadowMultiPosSet(&shadowEye, &shadowUp, &shadowTarget, 1);

    savedCameraTarget = cameraState->cameraTarget[1];
    savedCameraPosition = cameraState->cameraPosition[1];
    minimumPlayerZ = (1.0f / 0.0f);
    maximumPlayerZ = (-1.0f / 0.0f);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_18[playerIndex]->cameraGroup != 0) {
            if (minimumPlayerZ > lbl_1_bss_18[playerIndex]->player->actor->pos.z) {
                minimumPlayerZ = lbl_1_bss_18[playerIndex]->player->actor->pos.z;
            }
            if (maximumPlayerZ < lbl_1_bss_18[playerIndex]->player->actor->pos.z) {
                maximumPlayerZ = lbl_1_bss_18[playerIndex]->player->actor->pos.z;
            }
        }
    }
    cameraState->cameraTarget[1].y = 0.0f;
    playerGroupCenterZ = (minimumPlayerZ + maximumPlayerZ) / 2.0f;
    cameraState->values[1][cameraState->startingColumn] =
        playerGroupCenterZ - cameraState->rowValue[1];
    cameraState->rowValue[1] = playerGroupCenterZ;
    if (fn_1_12CC((M627CameraState *)cameraState, 1, &cameraTrend)) {
        if (cameraTrend > 2.0f) {
            cameraState->cameraTarget[1].z +=
                cameraTrend > 10.0f ? 10.0f : cameraTrend;
        } else if (cameraTrend < -2.0f) {
            cameraState->cameraTarget[1].z +=
                cameraTrend < -10.0f ? -10.0f : cameraTrend;
        }
        cameraState->cameraPosition[1].y = lbl_1_data_E04.y;
        cameraState->cameraPosition[1].z = cameraState->cameraTarget[1].z + lbl_1_data_E04.z;
        Hu3DCameraPosSetV(2, &cameraState->cameraPosition[1], &lbl_1_data_E10,
                          &cameraState->cameraTarget[1]);
        for (activeIndex = 0; activeIndex < 4; activeIndex++) {
            if (lbl_1_bss_18[activeIndex]->cameraGroup != 0) {
                MgActorPosGet(lbl_1_bss_18[activeIndex]->player->actor, &playerPosition);
                Hu3D3Dto2D(&playerPosition, 2, &screenPosition);
                if (cameraTrend < 0.0f &&
                    screenPosition.y > 420.0f) {
                    break;
                }
                if (cameraTrend > 0.0f &&
                    screenPosition.y < 70.0f) {
                    break;
                }
            }
        }
        /* Keep the previous view if this move would push a player past the screen edge. */
        if (activeIndex < 4) {
            cameraState->cameraTarget[1] = savedCameraTarget;
            cameraState->cameraPosition[1] = savedCameraPosition;
            Hu3DCameraPosSetV(2, &cameraState->cameraPosition[1], &lbl_1_data_E10,
                              &cameraState->cameraTarget[1]);
        }
    }
    /* Both camera rows share this ring-buffer slot for the next manager tick. */
    cameraState->startingColumn = (cameraState->startingColumn + 1) % 5;
    shadowTarget = cameraState->cameraTarget[1];
    shadowEye.x = shadowTarget.x - 100.0f;
    shadowEye.y = 8000.0f + shadowTarget.y;
    shadowEye.z = shadowTarget.z;
    Hu3DShadowMultiPosSet(&shadowEye, &shadowUp, &shadowTarget, 2);
}

/* Installed by fn_1_1508 at result start; closes split-screen and advances to the ending. */
void fn_1_1C70(OMOBJ *cameraObject)
{
    M627CameraState *cameraState = cameraObject->data;
    f32 progress = (f32)cameraState->elapsedFrames / 60.0f;

    if (MgSeqModeGet() == 7) {
        cameraState->elapsedFrames = 0;
        fn_1_140(cameraObject, fn_1_21B8);
        return;
    }

    if (progress <= 1.0f) {
        f32 cameraOneScissorWidth;
        f32 cameraOneViewportWidth;
        f32 cameraTwoViewportWidth;
        f32 cameraOneViewportX;
        f32 cameraTwoViewportX;

        if (lbl_1_bss_28 == 1) {
            progress = 1.0f - progress;
            cameraOneScissorWidth = 640.0f + -320.0f * progress;
            cameraOneViewportWidth = 640.0f;
            cameraTwoViewportWidth = 640.0f * progress;
            cameraOneViewportX = (640.0f + -304.0f * progress) / 2.0f -
                     320.0f;
            cameraTwoViewportX = 640.0f *
                          (1.0f - (3.0f * progress) / 4.0f) -
                      8.0f * progress;
        } else {
            cameraOneScissorWidth = 320.0f * (1.0f - progress);
            cameraOneViewportWidth = 640.0f * (1.0f - progress);
            cameraTwoViewportWidth = 640.0f;
            cameraOneViewportX = -640.0f *
                         ((1.0f - progress) / 4.0f) +
                     8.0f * (1.0f - progress);
            cameraTwoViewportX = 640.0f *
                           ((1.0f - progress) / 4.0f) -
                      8.0f * (1.0f - progress);
        }

        Hu3DCameraViewportSet(1, cameraOneViewportX, 0.0f, cameraOneViewportWidth, 480.0f,
                              0.0f, 1.0f);
        if (cameraOneScissorWidth > 2.0f) {
            Hu3DCameraScissorSet(1, 0, 0, __cvt_fp2unsigned(cameraOneScissorWidth) - 2, 480);
        } else {
            Hu3DCameraScissorSet(1, 0, 0, 0, 0);
        }
        Hu3DCameraViewportSet(2, cameraTwoViewportX, 0.0f, cameraTwoViewportWidth, 480.0f,
                              0.0f, 1.0f);
        /* Keep the two-pixel split insets. At width 640, the unsigned subtraction wraps,
        * passing camera 2 a scissor X of 642 and width 0xFFFFFFFE. */
        if (638 - __cvt_fp2unsigned(cameraOneScissorWidth) != 0) {
            Hu3DCameraScissorSet(2, __cvt_fp2unsigned(cameraOneScissorWidth) + 2, 0,
                                 638 - __cvt_fp2unsigned(cameraOneScissorWidth), 480);
        } else {
            Hu3DCameraScissorSet(2, 0, 0, 0, 0);
        }
    }

    cameraState->elapsedFrames++;
    if (cameraState->cameraTarget[lbl_1_bss_28 - 1].z < lbl_1_data_E34.z &&
        cameraState->elapsedFrames <= 30) {
        Vec interpolatedTarget;
        Vec interpolatedEye;
        Vec offset;

        PSVECSubtract(&lbl_1_data_E34, &cameraState->cameraTarget[lbl_1_bss_28 - 1], &offset);
        PSVECScale(&offset, &offset, (f32)cameraState->elapsedFrames / 30.0f);
        PSVECAdd(&cameraState->cameraTarget[lbl_1_bss_28 - 1], &offset, &interpolatedTarget);
        PSVECAdd(&interpolatedTarget, &lbl_1_data_E04, &interpolatedEye);
        Hu3DCameraPosSetV(lbl_1_bss_28, &interpolatedEye, &lbl_1_data_E10,
                          &interpolatedTarget);
    }
}

/* Installed by fn_1_1C70 in phase 7; runs the wipe and restores the full-screen result view. */
void fn_1_21B8(OMOBJ *cameraObject)
{
    M627CameraState *cameraState = cameraObject->data;

    cameraState->elapsedFrames++;
    if (cameraState->elapsedFrames == 30) {
        WipeCreate(2, 0, 30);
    } else if (cameraState->elapsedFrames == 140) {
        Vec cameraTarget;
        Vec cameraPosition;
        Vec cameraUp;

        Hu3DCameraKill(2);
        Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f,
                             480.0f, 0.0f, 1.0f);
        Hu3DCameraScissorSet(1, 0, 0, 640, 480);

        cameraState->cameraTarget[0].x = lbl_1_data_E40.x;
        cameraState->cameraTarget[0].y = lbl_1_data_E40.y;
        cameraState->cameraTarget[0].z = lbl_1_data_E40.z;
        cameraState->cameraPosition[0].x = cameraState->cameraTarget[0].x + lbl_1_data_E4C.x;
        cameraState->cameraPosition[0].y = cameraState->cameraTarget[0].y + lbl_1_data_E4C.y;
        cameraState->cameraPosition[0].z = cameraState->cameraTarget[0].z + lbl_1_data_E4C.z;
        Hu3DCameraPosSetV(1, &cameraState->cameraPosition[0], &lbl_1_data_E10,
                          &cameraState->cameraTarget[0]);

        cameraUp.x = 0.0f;
        cameraUp.y = 1.0f;
        cameraUp.z = 0.0f;
        cameraTarget = cameraState->cameraTarget[0];
        cameraPosition.x = cameraTarget.x - 100.0f;
        cameraPosition.y = 8000.0f + cameraTarget.y;
        cameraPosition.z = cameraTarget.z;
        Hu3DShadowMultiPosSet(&cameraPosition, &cameraUp, &cameraTarget, 1);

        Hu3DModelAttrSet(lbl_1_bss_8[0]->modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_8[1]->modelId, HU3D_ATTR_DISPOFF);
        WipeCreate(1, 5, 30);
    }
}

/* Called during map setup to count single blocks and connected two-by-two blocks in a board
 * layout. */
s16 fn_1_23F8(const u8 grid[30][11], s16 *singleCountOut, s16 *groupCountOut)
{
    const u8 (*cells)[11] = grid;
    s32 singleCount;
    s32 groupCount;
    s32 row;

    singleCount = groupCount = 0;

    for (row = 0; row < 30; row++) {
        s32 column;

        for (column = 0; column < 11; column++) {
            switch (cells[row][column]) {
            case 1:
                singleCount++;
                break;
            case 2:
                if (row != 0 && cells[row - 1][column] == 2) {
                    break;
                }
                if (column != 0 && cells[row][column - 1] == 2) {
                    break;
                }
                groupCount++;
                break;
            }
        }
    }

    if (singleCountOut != NULL) {
        *singleCountOut = (s16)singleCount;
    }
    if (groupCountOut != NULL) {
        *groupCountOut = (s16)groupCount;
    }
    return (s16)(singleCount + groupCount);
}

/* Called by map setup for each board edge; stores its direction, normal and two collision
 * offsets. */
void fn_1_24F8(M627BoundaryLine *line, HuVecF *origin, HuVecF *end, s32 type)
{
    HuVecF offset;

    line->type = type;
    line->origin = *origin;
    PSVECSubtract(end, origin, &line->direction);
    line->normal.x = line->direction.z;
    line->normal.y = 0.0f;
    line->normal.z = -line->direction.x;
    PSVECNormalize(&line->normal, &line->normal);
    PSVECScale(&line->normal, &offset, 75.0f);
    PSVECAdd(&line->origin, &offset, &line->nearPoint);
    PSVECScale(&line->normal, &offset, 175.0f);
    PSVECAdd(&line->origin, &offset, &line->farPoint);
}

/* Builds the single and connected block models from the board grid during map setup. */
void fn_1_25E4(const u8 grid[30][11], M627MapWork *setup)
{
    const u8 (*cells)[11];
    HU3D_MODELID singleDisplayModel;
    HU3D_MODELID groupDisplayModel;
    HU3D_MODELID singleCollisionModel;
    HU3D_MODELID groupCollisionModel;
    M627Entry *singlePiece;
    M627Entry *groupPiece;
    s16 totalPieceCount;
    s16 singleCount;
    s16 groupCount;
    s32 row;
    s32 column;
    s32 singleIndex;
    s32 groupIndex;

    cells = grid;
    singleDisplayModel = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m627, 13), HU_MEMNUM_OVL,
                                                       HEAP_MODEL));
    groupDisplayModel = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m627, 11), HU_MEMNUM_OVL,
                                                      HEAP_MODEL));
    singleCollisionModel =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m627, 14), HU_MEMNUM_OVL, HEAP_MODEL));
    groupCollisionModel =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m627, 12), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(singleDisplayModel, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(singleDisplayModel, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(groupDisplayModel, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(singleCollisionModel, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(groupCollisionModel, HU3D_ATTR_DISPOFF);

    totalPieceCount = fn_1_23F8(cells, &singleCount, &groupCount);
    setup->count = singleCount;
    setup->groupCount = groupCount;
    singlePiece = setup->entries =
        HuMemDirectMallocNum(HEAP_HEAP, singleCount * sizeof(M627Entry), HU_MEMNUM_OVL);
    groupPiece = setup->groups =
        HuMemDirectMallocNum(HEAP_HEAP, groupCount * sizeof(M627Entry), HU_MEMNUM_OVL);
    singleIndex = groupIndex = 0;

    for (row = 0; row < 30; row++) {
        for (column = 0; column < 11; column++) {
            switch (cells[row][column]) {
            case 1:
                singlePiece->flags = 3;
                singlePiece->position.x = 100.0f * (0.5f + column) -
                                          550.0f;
                singlePiece->position.y = 0.0f;
                singlePiece->position.z = (f32)(129.8 +
                    (f64)(300.0f +
                    100.0f * (0.5f + row)));
                singlePiece->modelId = Hu3DModelLink(singleDisplayModel);
                singlePiece->collisionModelId = Hu3DModelLink(singleCollisionModel);
                singlePiece->linkedModelIds[0] = singlePiece->linkedModelIds[1] = 0;
                Hu3DModelAttrSet(singlePiece->modelId, HU3D_MOTATTR_PAUSE);
                Hu3DModelAttrReset(singlePiece->modelId, HU3D_ATTR_DISPOFF);
                Hu3DModelPosSetV(singlePiece->modelId, &singlePiece->position);
                Hu3DModelPosSetV(singlePiece->collisionModelId, &singlePiece->position);
                Hu3DModelShadowMapSet(singlePiece->modelId);
                singlePiece->actorIndex = (s16)(singleIndex + 1);
                setup->modelsObject->mdlId[singlePiece->actorIndex + 6] =
                    singlePiece->collisionModelId;
                singleIndex++;
                singlePiece++;
                break;
            case 2:
                if ((row == 0 || cells[row - 1][column] != 2) &&
                    (column == 0 || cells[row][column - 1] != 2)) {
                    groupPiece->flags = 0;
                    groupPiece->position.x = 100.0f * (1.0f + column) -
                                             550.0f;
                    groupPiece->position.y = 0.0f;
                    groupPiece->position.z = (f32)(129.8 +
                        (f64)(300.0f +
                        100.0f * (1.0f + row)));
                    groupPiece->modelId = Hu3DModelLink(groupDisplayModel);
                    groupPiece->collisionModelId = Hu3DModelLink(groupCollisionModel);
                    Hu3DModelAttrReset(groupPiece->modelId, HU3D_ATTR_DISPOFF);
                    Hu3DModelPosSetV(groupPiece->modelId, &groupPiece->position);
                    Hu3DModelPosSetV(groupPiece->collisionModelId, &groupPiece->position);
                    Hu3DModelShadowMapSet(groupPiece->modelId);
                    groupPiece->actorIndex = (s16)(singleCount + 1 + groupIndex);
                    setup->modelsObject->mdlId[groupPiece->actorIndex + 6] =
                        groupPiece->collisionModelId;
                    groupIndex++;
                    groupPiece++;
                }
                break;
            }
        }
    }
}

/* After collision-map setup, enables collision masks for the course and every block. */
void fn_1_2B58(M627MapWork *map)
{
    s32 recordIndex;

    MgActorColMapMaskSet(0, 3);
    for (recordIndex = 0; recordIndex < map->count; recordIndex++) {
        MgActorColMapMaskSet(map->entries[recordIndex].actorIndex, 3);
    }
    for (recordIndex = 0; recordIndex < map->groupCount; recordIndex++) {
        MgActorColMapMaskSet(map->groups[recordIndex].actorIndex, 3);
    }
}

/* Chooses a board layout and creates its collision and display objects at game startup. */
void fn_1_2BF4(OMOBJ *object)
{
    M627MapWork *setup = object->data;
    COL_ATTRPARAM collisionAttributes;
    s16 totalCount;
    const u8 (*grid)[11];
    s16 singleCount;
    s16 groupCount;
    s16 gridIndex;
    s32 boundaryIndex;

    /* This result is discarded; frandom does not seed the shared stream used below. */
    frandom(OSGetTick());
    gridIndex = (s16)(frandf() * lbl_1_data_D40);
    if (gridIndex > lbl_1_data_D40 - 1) {
        gridIndex = lbl_1_data_D40 - 1;
    }
    grid = lbl_1_data_5C[gridIndex];
    totalCount = fn_1_23F8(grid, &singleCount, &groupCount);

    setup->modelsObject = omAddObjEx(lbl_1_bss_0, 32730,
                                     (u16)(totalCount + 7),
                                     0, -1, NULL);
    setup->blockClearModel = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m627, 15), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(setup->blockClearModel, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(setup->blockClearModel, HU3D_ATTR_DISPOFF);
    fn_1_25E4((const u8 (*)[11])grid, setup);

    for (boundaryIndex = 0; boundaryIndex < 7; boundaryIndex++) {
        setup->modelsObject->mdlId[boundaryIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(
                lbl_1_data_30[boundaryIndex],
                HU_MEMNUM_OVL,
                                 HEAP_MODEL));
    }
    for (boundaryIndex = 0; boundaryIndex < 6; boundaryIndex++) {
        Hu3DModelAttrSet(setup->modelsObject->mdlId[boundaryIndex], HU3D_MOTATTR_LOOP);
    }
    Hu3DModelShadowMapSet(setup->modelsObject->mdlId[0]);
    Hu3DModelAttrSet(setup->modelsObject->mdlId[6], HU3D_ATTR_DISPOFF);

    MgActorColMapInit(setup->modelsObject->mdlId + 6,
                      (s16)(totalCount + 1), 6);
    MgActorColAttrParamGet(&collisionAttributes, 1);
    collisionAttributes.yDeviate = 0.0f;
    MgActorColAttrParamSet(&collisionAttributes, 1);

    setup->boundaryCount = 9;
    setup->boundaries = HuMemDirectMallocNum(HEAP_HEAP,
                                             setup->boundaryCount << 6,
                                             HU_MEMNUM_OVL);
    for (boundaryIndex = 0; boundaryIndex < setup->boundaryCount;
         boundaryIndex++) {
        fn_1_24F8(
            &setup->boundaries[boundaryIndex],
            &lbl_1_data_D44[lbl_1_data_DBC[boundaryIndex * 2]],
            &lbl_1_data_D44[lbl_1_data_DBC[boundaryIndex * 2 + 1]], 0);
    }

    fn_1_2B58((M627MapWork *)setup);
    fn_1_140(object, fn_1_30B0);
}

/* Installed after map creation; kills finished block-clearing effect models until phase 7,
 * then starts the map cleanup callback. */
void fn_1_30B0(OMOBJ *obj)
{
    M627MapWork *work = obj->data;
    s32 recordIndex;
    s32 motionIndex;

    if (MgSeqModeGet() == 7) {
        work->timer = 0;
        fn_1_140(obj, fn_1_3184);
    } else {
        for (recordIndex = 0; recordIndex < work->count; recordIndex++) {
            for (motionIndex = 0; motionIndex < 2; motionIndex++) {
                M627Entry *record = &work->entries[recordIndex];
                HU3D_MOTIONID linkedModelId = record->linkedModelIds[motionIndex];

                if (linkedModelId != 0 && Hu3DMotionEndCheck(linkedModelId)) {
                    Hu3DModelKill(linkedModelId);
                    record->linkedModelIds[motionIndex] = 0;
                }
            }
        }
    }
}

/* After 140 result updates, hides blocks only if a group won, then stops the map callback. */
void fn_1_3184(OMOBJ *obj)
{
    M627MapWork *work = obj->data;
    s32 recordIndex;

    work->timer++;
    if (work->timer == 140) {
        if (lbl_1_bss_10 != -1) {
            for (recordIndex = 0; recordIndex < work->count; recordIndex++) {
                M627Entry *record = &work->entries[recordIndex];
                s32 attachedIndex;

                MgActorColMapMaskSet(record->actorIndex, 0);
                Hu3DModelAttrSet(record->modelId, HU3D_ATTR_DISPOFF);
                for (attachedIndex = 0; attachedIndex < 2; attachedIndex++) {
                    s16 model = record->linkedModelIds[attachedIndex];
                    if (model != 0) {
                        Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
                    }
                }
            }
            for (recordIndex = 0; recordIndex < work->groupCount; recordIndex++) {
                M627Entry *record = &work->groups[recordIndex];
                MgActorColMapMaskSet(record->actorIndex, 0);
                Hu3DModelAttrSet(record->modelId, HU3D_ATTR_DISPOFF);
            }
        }
        fn_1_140(obj, 0);
        return;
    }
}

/* When a player's block-clearing action finds a block, creates its per-camera effect model,
 * clears that view's block visibility and collision, and plays the entry sound. */
void fn_1_32A4(s16 cameraGroup, M627Entry *work)
{
    s16 cameraIndex = cameraGroup;
    u32 collisionMask;

    work->linkedModelIds[cameraIndex] = Hu3DModelLink(lbl_1_bss_2C->blockClearModel);
    Hu3DModelPosSetV(work->linkedModelIds[cameraIndex], &work->position);
    Hu3DModelAttrReset(work->linkedModelIds[cameraIndex], HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrReset(work->linkedModelIds[cameraIndex], HU3D_ATTR_DISPOFF);
    Hu3DMotionSet(work->linkedModelIds[cameraIndex], 0);
    Hu3DMotionCalc(work->linkedModelIds[cameraIndex]);

    Hu3DModelCameraSet(work->linkedModelIds[cameraIndex], cameraIndex == 0 ? 1 : 2);
    collisionMask = MgActorColMapMaskGet(work->actorIndex);
    if (cameraIndex == 0) {
        work->flags &= ~1;
        collisionMask &= ~1;
    } else {
        work->flags &= ~2;
        collisionMask &= ~2;
    }
    MgActorColMapMaskSet(work->actorIndex, collisionMask);
    if (work->flags != 0) {
        Hu3DModelCameraSet(work->modelId, work->flags);
    } else {
        Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
    }

    if (cameraGroup == 0) {
        fn_1_430(1, &work->position, MSM_SE_M627_ENTRY_FIRST_SCREEN);
        return;
    }
    fn_1_430(2, &work->position, MSM_SE_M627_ENTRY_SECOND_SCREEN);
}

/* Updates a grounded player's action and push animation from the per-player mode callback. */
void fn_1_3590(MGPLAYER *player) {
    f32 pushLengthSquared;
    s16 pushMotion;

    if ((u32) (player->actor->colGroundAttr & 0x407F) != 0) {
        if ((MgPlayerAttrCheck(player, 1U) == 0) &&
            ((s32) (HuPadBtnDown[(s8) player->padNo] & PAD_BUTTON_A) != 0)) {
            MgPlayerModeSet(player, 3);
        } else if ((u32) (player->actionFlag & 1) != 0) {
            pushLengthSquared = (player->actor->push.x * player->actor->push.x) +
                                (player->actor->push.z * player->actor->push.z);
            if (pushLengthSquared > 25.0f) {
                pushMotion = 1;
                lbl_1_bss_18[player->playerNo]->motionSpeed = (f32) 1.5f;
            } else if (pushLengthSquared > 0.001) {
                pushMotion = 1;
                lbl_1_bss_18[player->playerNo]->motionSpeed = (f32) 1.0f;
            } else {
                pushMotion = 0;
                lbl_1_bss_18[player->playerNo]->motionSpeed = (f32) 1.0f;
            }
            if ((s32) player->motNo != pushMotion) {
                CharMotionShiftSet(player->charNo, player->omObj->mtnId[pushMotion], 0.0f, 5.0f,
                                   HU3D_MOTATTR_LOOP);
                player->motNo = (u16) pushMotion;
            }
        }
    }
    CharMotionSpeedSet(player->charNo, lbl_1_bss_18[player->playerNo]->motionSpeed);
    Hu3DMotionSpeedSet((s16) player->actor->mdlId, lbl_1_bss_18[player->playerNo]->motionSpeed);
}

/* Mode-3 block-clearing action: locks movement and facing for its timed animation,
 * then restores them and returns to mode 0. */
void fn_1_37C8(MGPLAYER *player)
{
    switch (player->subMode) {
    case 0:
        if (player->motNo != 2) {
            CharMotionShiftSet(player->charNo, player->omObj->mtnId[2], 0.0f, 5.0f, 0);
            player->motNo = 2;
        }
        player->timer = 36.0f;
        MgPlayerAttrSet(player, MGPLAYER_ATTR_ANGLELOCK | MGPLAYER_ATTR_MOVEOFF);
        player->subMode = 1;
        break;
    case 1:
        if (player->timer > 0.0f) {
            player->timer -= 1.0f;
        } else {
            MgPlayerAttrReset(player, MGPLAYER_ATTR_ANGLELOCK | MGPLAYER_ATTR_MOVEOFF);
            player->subMode = 2;
        }
        break;
    case 2:
        if (player->motNo != 0) {
            CharMotionShiftSet(player->charNo, player->omObj->mtnId[0], 0.0f, 5.0f,
                               HU3D_MOTATTR_LOOP);
            player->motNo = 0;
        }
        MgPlayerModeSet(player, 0);
        break;
    default:
        break;
        break;
    }
}

/* Starts a character motion when its requested slot differs from the object's last tracked slot. */
void fn_1_3924(M627PlayerWork *work, s32 motionSlot, f32 blendFrames, s32 attr)
{
    if (work->motionSlot != motionSlot) {
        work->motionSlot = motionSlot;
        CharMotionShiftSet(work->characterId, work->player->omObj->mtnId[motionSlot],
                           0.0f, blendFrames, attr);
    }
}

int fn_1_17188__12CallbackBaseFv(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    return 1;
}

/* Player-object creation callback; builds the character actor and accessory before its entrance. */
void fn_1_39A8(OMOBJ *object)
{
    M627PlayerWork *work = omObjGetDataAs(object, M627PlayerWork);
    MGACTOR_PARAM actorParameters = {0};
    s32 playerCameraBit;
    s32 groupNumber;
    s32 cameraBit;
    u32 actorCollisionMask;
    char *itemHook;

    work->frame = 0;
    work->characterId = GwPlayerConf[work->playerIndex].charNo;
    if (GwPlayerConf[work->playerIndex].grpNo == 0) {
        groupNumber = 0;
    } else {
        groupNumber = 1;
    }
    work->cameraGroup = groupNumber;
    work->selectionMode = GwPlayerConf[work->playerIndex].comDif;
    work->blockClearState = 0;
    work->active = 0;
    if (work->cameraGroup == 0) {
        cameraBit = 1;
    } else {
        cameraBit = 2;
    }
    playerCameraBit = cameraBit;
    if (work->cameraGroup == 0) {
        work->playerSlot = 0;
    } else {
        work->playerSlot = lbl_1_bss_14++;
    }

    actorParameters.height = 150.0f;
    actorParameters.radius = 40.0f;
    actorParameters.param = work->playerIndex;
    actorParameters.type = 0;
    actorParameters.attr = 0;
    actorParameters.correctHookParam = 0;
    actorParameters.narrowHook = fn_1_17188__12CallbackBaseFv;
    actorParameters.correctHook = 0;
    work->player = MgPlayerCreate(work->playerIndex, &actorParameters, 4, playerCameraBit, 1,
                                  lbl_1_data_EB8);
    MgPlayerModeFuncSet(work->player, lbl_1_data_ED4);
    work->motionSpeed = 1.0f;
    work->playerType = GwPlayerConf[work->playerIndex].type;
    work->activeEntry = 0;
    work->clearTargetHistoryOnArrival = 0;
    work->sequenceType = 0;
    work->previousEntry = work->currentEntry = 0;
    MgPlayerComStkOn(work->player);

    if (work->cameraGroup == 0) {
        actorCollisionMask = 1;
    } else {
        actorCollisionMask = 2;
    }
    MgActorColMaskSet(work->player->actor, actorCollisionMask);
    Hu3DModelCameraSet(work->player->actor->mdlId, playerCameraBit);
    itemHook = CharModelItemHookGet(work->characterId, 4, 0);
    work->accessoryModelId =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m627, 16), HU_MEMNUM_OVL,
                                             HEAP_MODEL));
    Hu3DModelHookSet(work->player->actor->mdlId, itemHook, work->accessoryModelId);
    Hu3DModelShadowSet(work->player->actor->mdlId);
    Hu3DModelShadowSet(work->accessoryModelId);
    MgActorPosSet(work->player->actor, &lbl_1_data_E58[work->playerSlot]);
    work->position = lbl_1_data_E58[work->playerSlot];
    work->position.y = 0.0f;
    if (work->playerIndex == 0) {
        lbl_1_bss_30 = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_m627, 17), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_30, HU3D_MOTATTR_PAUSE);
    }
    work->player->actor->gravity = 0.0f;
        Hu3DModelAttrSet(work->player->actor->mdlId, HU3D_ATTR_DISPOFF);
    fn_1_140(object, fn_1_3D6C);
}

/* Waits for the opening sequence to enter its player-introduction phase, then starts the
 * entrance. */
void fn_1_3D6C(OMOBJ *obj)
{
    M627PlayerWork *work = obj->data;

    if (MgSeqModeGet() == 2) {
        work->frame = 0;
        fn_1_140(obj, fn_1_3DC8);
        return;
    }
}

/* Runs the player's entrance during the opening sequence, then places the player at the moving
 * surface. */
void fn_1_3DC8(OMOBJ *obj)
{
    M627PlayerWork *work = (M627PlayerWork *)obj->data;
    HuVecF position;
    HSF_OBJECT *modelObject;

    work->frame++;
    position.x = position.y = position.z = 0.0f;
    if (work->frame == 60) {
        MgActorColAttrSet(work->player->actor, 6);
        Hu3DModelAttrReset(lbl_1_bss_30, HU3D_MOTATTR_PAUSE);
        if (work->motionSlot != 1) {
            work->motionSlot = 1;
            CharMotionShiftSet(work->characterId, work->player->omObj->mtnId[1], 0.0f,
                               1.0f, HU3D_MOTATTR_LOOP);
        }
    } else if (work->frame > 60) {
        modelObject = Hu3DModelObjPtrGet(lbl_1_bss_30, lbl_1_data_F68[work->playerSlot]);
        position.x = modelObject->mesh.curr.pos.x;
        position.y = 10.0f + modelObject->mesh.curr.pos.y;
        position.z = modelObject->mesh.curr.pos.z;
        MgActorPosSetRaw(work->player->actor, &position);
        MgActorRotYSet(work->player->actor, modelObject->mesh.curr.rot.y);
        Hu3DModelPosSetV(work->player->actor->mdlId, &position);
        Hu3DModelRotSet(work->player->actor->mdlId, 0.0f, modelObject->mesh.curr.rot.y,
                        0.0f);
        Hu3DModelAttrReset(work->player->actor->mdlId, HU3D_ATTR_DISPOFF);
        if (work->frame == 260) {
            MgActorPosSetRaw(work->player->actor, &position);
            Hu3DModelPosSetV(work->player->actor->mdlId, &position);
            if (work->motionSlot != 0) {
                work->motionSlot = 0;
                CharMotionShiftSet(work->characterId, work->player->omObj->mtnId[0], 0.0f,
                                   5.0f, HU3D_MOTATTR_LOOP);
            }
        }
    }
    if (MgSeqModeGet() == 5) {
        Hu3DModelAttrSet(lbl_1_bss_30, HU3D_ATTR_DISPOFF);
        MgActorColAttrReset(work->player->actor, 6);
        work->player->actor->gravity = 150.0f;
        work->frame = 0;
        fn_1_140(obj, fn_1_4070);
        return;
    }
}

/* At active play, clears the scripted-stick flag and installs the normal player update. */
void fn_1_4070(OMOBJ *obj)
{
    MGPLAYER **playerSlot = (MGPLAYER **)obj->data;
    if (MgSeqModeGet() == 5) {
        MgPlayerComStkOff(*playerSlot);
        fn_1_140(obj, fn_1_42D4);
        return;
    }
}

/* During a block-clearing action, finds a visible block near the point 80 units ahead. */
void *fn_1_40CC(M627PlayerWork *state)
{
    Point3d searchPoint;
    Point3d *center = &state->player->actor->pos;
    f32 rotation = state->player->actor->rotY;
    s32 entryIndex;
    M627Entry *entry;
    f32 deltaX;
    f32 deltaZ;
    f32 distanceSquared;

    searchPoint.x = (f32)(80.0 *
                         sin((3.141592653589793 * (f64)rotation) / 180.0));
    searchPoint.y = 0.0f;
    searchPoint.z = (f32)(80.0 *
                         cos((3.141592653589793 * (f64)rotation) / 180.0));
    searchPoint.x += center->x;
    searchPoint.y += center->y;
    searchPoint.z += center->z;

    for (entryIndex = 0; entryIndex < lbl_1_bss_2C->count; entryIndex++) {
        entry = &lbl_1_bss_2C->entries[entryIndex];
        if (((state->cameraGroup == 0) && ((entry->flags & 1) != 0)) ||
            ((state->cameraGroup != 0) && ((entry->flags & 2) != 0))) {
            deltaX = searchPoint.x - entry->position.x;
            deltaZ = searchPoint.z - entry->position.z;
            distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
            if (distanceSquared < 5625.0f) {
                return entry;
            }
        }
    }

    return NULL;
}

/* Player update callback; checks block-clearing actions, drives computer movement,
 * clamps group-1 players to their view, and enters the result transition. */
void fn_1_42D4(OMOBJ *object)
{
    M627PlayerWork *state = object->data;
    M627Entry *entry;
    Point3d position;
    Point3d projectedPosition;
    f32 panValue;

    if (state->player->mode == 3) {
        if (state->blockClearState == 0) {
            state->blockClearState = 2;
            state->blockClearTimer = 0;
        } else if (state->blockClearState == 2) {
            if (state->blockClearTimer == 18) {
                entry = fn_1_40CC(state);
                if (entry != NULL) {
                    fn_1_32A4(state->cameraGroup, entry);
                    omVibrate(state->player->playerNo, 10, 7, 3);
                    state->blockClearState = 1;
                }
            }
            state->blockClearTimer++;
        }
    } else {
        state->blockClearState = 0;
        state->blockClearTimer = 0;
        if (state->playerType != 0) {
            fn_1_5E9C(object);
        }
        if (state->cameraGroup != 0) {
            MgActorPosGet(state->player->actor, &position);
            Hu3D3Dto2D(&position, 2, &projectedPosition);
            if ((state->position.z < position.z && projectedPosition.y > 420.0f) ||
                (state->position.z > position.z && projectedPosition.y < 70.0f)) {
                position.z = state->position.z;
                MgActorPosSetRaw(state->player->actor, &position);
            } else {
                MgActorPosGet(state->player->actor, &state->position);
            }
        } else {
            MgActorPosGet(state->player->actor, &state->position);
        }
    }
    if (MgSeqModeGet() == 6) {
        MgPlayerComStkOn(state->player);
        MgActorColAttrSet(state->player->actor, 4U);
        fn_1_140(object, fn_1_492C);
        return;
    }
}

/* Waits for phase 7, then selects the round-with-winner or draw presentation for every player. */
void fn_1_492C(OMOBJ *obj)
{
    M627PlayerWork *work = obj->data;
    if (MgSeqModeGet() == 7) {
        MgPlayerComStkOn(work->player);
        work->frame = 0;
        if (lbl_1_bss_10 != -1) {
            fn_1_140(obj, fn_1_49B8);
            return;
        }
        fn_1_140(obj, fn_1_4B04);
        return;
    }
}

/* Result callback for a round with a winner; moves winning players to their result positions
 * after 140 updates and switches to the final pose in phase 8. */
void fn_1_49B8(OMOBJ *object)
{
    M627PlayerWork *work = object->data;

    work->frame++;
    if (work->frame == 140) {
        if (work->active != 0) {
            Hu3DModelCameraSet(work->player->actor->mdlId, 1);
            MgActorPosSet(work->player->actor, &lbl_1_data_E88[work->playerSlot]);
        } else {
            Hu3DModelCameraSet(work->player->actor->mdlId, 2);
        }
        Hu3DModelHookReset(work->player->actor->mdlId);
        Hu3DModelAttrSet(work->accessoryModelId, HU3D_ATTR_DISPOFF);
        MgActorRotYSet(work->player->actor, 0.0f);
        if (work->motionSlot != 3) {
            work->motionSlot = 3;
            CharMotionShiftSet(work->characterId,
                               work->player->omObj->mtnId[3],
                               0.0f, 1.0f, 0);
        }
    }

    if (MgSeqModeGet() == 8) {
        fn_1_140(object, fn_1_4C80);
        return;
    }
}

/* Draw presentation: blinks the accessory, then detaches it and blends to motion slot 3. */
void fn_1_4B04(OMOBJ *object)
{
    M627PlayerWork *state = object->data;
    f32 blendFrames;
    s16 shouldReset;
    s32 resetValue;

    state->frame++;
    if (state->frame < 30) {
        if ((state->frame / 5) % 2 == 0) {
            shouldReset = 0;
        } else {
            shouldReset = 1;
        }
        resetValue = shouldReset;
        if (resetValue != 0) {
            Hu3DModelAttrReset(state->accessoryModelId, HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(state->accessoryModelId, HU3D_ATTR_DISPOFF);
        }
    } else if (MgSeqModeGet() == 8) {
        fn_1_140(object, fn_1_4C80);
    } else {
        Hu3DModelHookReset((s16)state->player->actor->mdlId);
        Hu3DModelAttrSet(state->accessoryModelId, HU3D_ATTR_DISPOFF);
        /* Draw a new blend duration even when slot 3 is already selected and no blend is
         * requested. */
        blendFrames = 20.0f + 20.0f * frandf();
        if (state->motionSlot != 3) {
            state->motionSlot = 3;
            CharMotionShiftSet(state->characterId,
                               state->player->omObj->mtnId[3],
                               0.0f, blendFrames, 0);
        }
    }
}

/* Result callback for the final pose; chooses the character motion from whether the player won. */
void fn_1_4C80(OMOBJ *controller)
{
    M627PlayerWork *playerWork = controller->data;
    if (playerWork->active != 0) {
        if (playerWork->motionSlot != 4) {
            playerWork->motionSlot = 4;
            CharMotionShiftSet(playerWork->characterId,
                               playerWork->player->omObj->mtnId[4],
                               0.0f, 4.0f, 0);
        }
    } else if (playerWork->motionSlot != 5) {
        playerWork->motionSlot = 5;
        CharMotionShiftSet(playerWork->characterId,
                           playerWork->player->omObj->mtnId[5],
                           0.0f, 4.0f, 0);
    }
}

/* Called when computer travel times out or stalls; records the failed target in a two-entry
 * history. */
void fn_1_4D54(M627PlayerWork *state, M627Entry *entry)
{
    if (state->previousEntry != NULL) {
        if (state->currentEntry != NULL) {
            state->previousEntry = state->currentEntry;
            state->currentEntry = entry;
            return;
        }
        state->currentEntry = entry;
        return;
    }
    if (state->currentEntry != NULL) {
        state->previousEntry = state->currentEntry;
        state->currentEntry = entry;
        return;
    }
    state->previousEntry = entry;
}

/* Used by target selection to reject either of the computer player's two recent entries. */
s32 fn_1_4DB4(M627PlayerWork *work, M627Entry *entry)
{
    if (work->previousEntry == entry || work->currentEntry == entry) {
        return 1;
    }
    return 0;
}

void fn_1_4DDC(M627PlayerWork *work)
{
    work->previousEntry = work->currentEntry = NULL;
}

/* Tries up to 32 random first-screen targets, excluding recent entries; exhaustion can return
 * the last visibility-rejected candidate instead of NULL. */
M627Entry *fn_1_4DEC(M627PlayerWork *state)
{
    s16 attempt;
    s16 entryIndex;
    M627Entry *entry = NULL;
    u16 filterMask;
    filterMask = state->cameraGroup == 0 ? 1 : 2;

    for (attempt = 0; attempt < 32; attempt++) {
        entryIndex = (s16)(lbl_1_bss_2C->count * frandf());
        if (entryIndex < lbl_1_bss_2C->count) {
            entry = &lbl_1_bss_2C->entries[entryIndex];
            if ((s16)fn_1_4DB4(state, entry) != 0) {
                entry = NULL;
                continue;
            }
            if ((entry->flags & filterMask) != 0) {
                break;
            }
        }
    }
    return entry;
}

/* Tries up to 32 targets on the requested X side and below the Z cutoff, excluding recent
 * entries; exhaustion can return the last side/Z-rejected candidate. */
M627Entry *fn_1_4F24(M627PlayerWork *state, s16 reverseTest, f32 maxZ)
{
    M627Entry *entry = NULL;
    s16 attempt = 0;
    s16 entryIndex;
    s16 duplicate;

    while (attempt < 32) {
        entryIndex = (s16)(lbl_1_bss_2C->count * frandf());
        if (entryIndex < lbl_1_bss_2C->count) {
            entry = &lbl_1_bss_2C->entries[entryIndex];
            if (state->previousEntry == entry || state->currentEntry == entry) {
                duplicate = 1;
            } else {
                duplicate = 0;
            }
            if (duplicate != 0) {
                entry = NULL;
            } else if ((((reverseTest != 0) || !(entry->position.x < 0.0f)) &&
                        ((reverseTest == 0) || !(entry->position.x >= 0.0f))) ||
                       !(entry->position.z < maxZ)) {
                /* Try another table entry. */
            } else {
                break;
            }
        }
        attempt++;
    }

    return entry;
}

/* Selects an eligible first-screen block nearest the group's ball; the hardest difficulty
 * discounts Z distance in its score. */
M627Entry *fn_1_5090(M627PlayerWork *state)
{
    Point3d displacement;
    f32 minimumDistanceSquared;
    f32 distanceSquared;
    M627Entry *nearestEntry;
    M627ActorWork *actorSlot;
    s16 entryIndex;
    M627Entry *entry;

    nearestEntry = NULL;
    actorSlot = lbl_1_bss_8[0];
    minimumDistanceSquared = 34028235000000000000000000000000000000.0f;
    entryIndex = 0;
    for (; entryIndex < lbl_1_bss_2C->count; entryIndex++) {
        entry = &lbl_1_bss_2C->entries[entryIndex];
        if ((entry->flags & 1) != 0 && state->activeEntry != entry &&
            (state->selectionMode < 2 ||
            !(entry->position.z < lbl_1_bss_8[0]->actor->pos.z - 75.0f))) {
            if ((s16)fn_1_4DB4((M627PlayerWork *)state, entry) == 0) {
                PSVECSubtract(&entry->position, &actorSlot->actor->pos, &displacement);
                displacement.y = 0.0f;
                if (state->selectionMode < 3) {
                    distanceSquared = PSVECSquareMag(&displacement);
                } else {
                    distanceSquared = displacement.x * displacement.x +
                                      displacement.z * displacement.z / 3.0f;
                }
                if (distanceSquared < minimumDistanceSquared) {
                    minimumDistanceSquared = distanceSquared;
                    nearestEntry = entry;
                }
            }
        }
    }
    return nearestEntry;
}

/* Scores second-screen blocks against teammates and prefers visible targets; only difficulties
 * 2 and 3 exclude teammates' selected blocks. */
M627Entry *fn_1_5248(M627PlayerWork *state)
{
    Point3d playerPositions[2];
    Point3d entryPosition;
    Point3d screenPosition;
    Point3d firstOffset;
    Point3d secondOffset;
    Point3d planeNormal;
    f32 bestVisibleScore;
    f32 bestFallbackScore;
    f32 score;
    s16 index;
    M627Entry *bestFallbackEntry;
    s16 ownedByOtherPlayer;
    s16 playerCursor;
    s16 currentEntryMatch;
    M627Entry *bestVisibleEntry;
    M627Entry *entry;

    bestVisibleEntry = NULL;
    bestFallbackEntry = NULL;
    playerCursor = 0;
    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_18[index]->cameraGroup != 0 &&
            lbl_1_bss_18[index]->playerIndex != state->playerIndex) {
            playerPositions[playerCursor] = lbl_1_bss_18[index]->player->actor->pos;
            playerPositions[playerCursor].y = 0.0f;
            playerCursor++;
        }
    }

    bestVisibleScore = bestFallbackScore = -34028235000000000000000000000000000000.0f;
    for (index = 0; index < lbl_1_bss_2C->count; index++) {
        entry = &lbl_1_bss_2C->entries[index];
        if ((entry->flags & 2) == 0) {
            continue;
        }

        entryPosition = entry->position;
        entryPosition.y = 0.0f;

        if (state->previousEntry == entry || state->currentEntry == entry) {
            currentEntryMatch = 1;
        } else {
            currentEntryMatch = 0;
        }
        if (currentEntryMatch == 0) {
            if (state->selectionMode >= 2) {
                ownedByOtherPlayer = 0;
                for (playerCursor = 0; playerCursor < 4; playerCursor++) {
                    if (lbl_1_bss_18[playerCursor]->cameraGroup != 0 &&
                        lbl_1_bss_18[playerCursor]->playerIndex != state->playerIndex &&
                        lbl_1_bss_18[playerCursor]->activeEntry == entry) {
                        ownedByOtherPlayer = 1;
                        break;
                    }
                }
                if (ownedByOtherPlayer != 0) {
                    continue;
                }
            }
            PSVECSubtract(&playerPositions[0], &entryPosition, &firstOffset);
            PSVECSubtract(&playerPositions[1], &entryPosition, &secondOffset);
            firstOffset.y = secondOffset.y = 0.0f;
            PSVECCrossProduct(&firstOffset, &secondOffset, &planeNormal);
        PSVECSubtract(&entryPosition, &lbl_1_bss_8[1]->actor->pos,
                          &firstOffset);
            firstOffset.y = 0.0f;

            switch (state->selectionMode) {
                case 0:
                    score = PSVECSquareMag(&planeNormal);
                    break;
                case 1:
                    score = PSVECSquareMag(&planeNormal) -
                            (3.0f * PSVECSquareMag(&firstOffset));
                    break;
                case 2:
                    score = PSVECSquareMag(&planeNormal) -
                            (3.0f * PSVECSquareMag(&firstOffset));
                    break;
                case 3:
                    score = PSVECSquareMag(&planeNormal) -
                            (3.0f *
                             ((firstOffset.x * firstOffset.x) +
                              (firstOffset.z * firstOffset.z / 2.0f)));
                    break;
            }

            Hu3D3Dto2D(&entryPosition, 2, &screenPosition);
            if ((state->selectionMode < 2 ||
                     entryPosition.z >= lbl_1_bss_8[1]->actor->pos.z - 175.0f) &&
                    screenPosition.y >= 50.0f &&
                    screenPosition.y <= 420.0f) {
                    if (score > bestVisibleScore) {
                        bestVisibleEntry = entry;
                        bestVisibleScore = score;
                    }
                } else if (bestVisibleEntry == NULL && score > bestFallbackScore) {
                    bestFallbackEntry = entry;
                    bestFallbackScore = score;
            }
        }
    }

    if (bestVisibleEntry == NULL) {
        bestVisibleEntry = bestFallbackEntry;
    }
    return bestVisibleEntry;
}

/* Called between computer target attempts to choose a difficulty-dependent pause in frames. */
s16 fn_1_5718(M627PlayerWork *state)
{
    if (state->cameraGroup == 0) {
        switch (state->selectionMode) {
        case 0:
            return (s16)((s32)(60.0f * frandf()) + 60);
        case 1:
            return (s16)((s32)(30.0f * frandf()) + 30);
        case 2:
            return (s16)((s32)(20.0f * frandf()) + 10);
        }
    } else {
        switch (state->selectionMode) {
        case 0:
            return (s16)((s32)(90.0f * frandf()) + 90);
        case 1:
            return (s16)((s32)(45.0f * frandf()) + 45);
        case 2:
            return (s16)((s32)(20.0f * frandf()) + 20);
        }
    }

    return 0;
}

/* Selects a block target or a follow position and initializes the computer's travel timer. */
void fn_1_58A8(M627PlayerWork *state, s16 action)
{
    Point3d displacement;
    Point3d playerPosition;
    f32 ballX;
    f32 adjustment;
    s32 cameraGroup;

    if (action != 0) {
        cameraGroup = state->cameraGroup;
        ballX = lbl_1_bss_8[cameraGroup]->actor->pos.x;
        adjustment = cameraGroup == 0 ? 150.0f : 350.0f;
        MgActorPosGet(state->player->actor, &state->followPosition);
        state->followPosition.y = 0.0f;
        switch (action) {
        case 1:
            if (state->followPosition.x < 0.0f) {
                if (ballX < state->followPosition.x) {
                    state->followPosition.x += 300.0f;
                } else {
                    state->followPosition.x += 300.0f + adjustment;
                }
            } else if (ballX > state->followPosition.x) {
                state->followPosition.x -= 300.0f;
            } else {
                state->followPosition.x -= 300.0f + adjustment;
            }
            if (state->followPosition.x < -500.0f) {
                state->followPosition.x = -500.0f;
            } else if (state->followPosition.x > 500.0f) {
                state->followPosition.x = 500.0f;
            }
            state->followPosition.z -= 150.0f;
            if (state->followPosition.z < 50.0f) {
                state->followPosition.z = 50.0f;
            }
            break;
        case 2:
            state->followPosition.z += 500.0f;
            break;
        }
        PSVECSubtract(&state->followPosition, &state->player->actor->pos, &displacement);
        state->travelTicks = (s32)((PSVECMag(&displacement) / 5.6f) *
                                     1.4f);
        state->positionState = 0;
    } else {
        if (state->cameraGroup == 0) {
            if (state->selectionMode == 0) {
                state->activeEntry = fn_1_4DEC((M627PlayerWork *)state);
            } else {
                state->activeEntry = fn_1_5090((M627PlayerWork *)state);
            }
        } else {
            state->activeEntry = fn_1_5248((M627PlayerWork *)state);
        }
        if (state->activeEntry != NULL) {
            PSVECSubtract(&state->activeEntry->position, &state->player->actor->pos,
                          &playerPosition);
            state->travelTicks = (s32)((PSVECMag(&playerPosition) / 5.6f) *
                                         1.4f);
            state->positionState = 0;
        }
    }
    state->sequenceType = action;
}

/* Computer-player update during ordinary movement; steers toward blocks, requests clearing
 * in range, and takes detours after timed-out or stalled travel. */
void fn_1_5E9C(OMOBJ *object)
{
    M627PlayerWork *state;
    Point3d entryPosition;
    Point3d actorPosition;
    Point3d offset;
    f32 distance;

    state = object->data;
    if (state->waitFrames > 0) {
        state->waitFrames--;
        return;
    }
    if (state->sequenceType != 0) {
        actorPosition = *(Point3d *)&state->player->actor->pos;
        actorPosition.y = 0.0f;
        PSVECSubtract(&actorPosition, &state->followPosition, &offset);
        if (PSVECSquareMag(&offset) < 1600.0f) {
            state->sequenceType++;
            if (state->sequenceType >= 3) {
                state->activeEntry = NULL;
                state->sequenceType = 0;
                return;
            }
            fn_1_58A8((M627PlayerWork *)state, state->sequenceType);
            return;
        }
        state->travelTicks--;
        if (state->travelTicks <= 0) {
            state->waitFrames = fn_1_5718((M627PlayerWork *)state);
            fn_1_58A8((M627PlayerWork *)state, 1);
            state->clearTargetHistoryOnArrival = 0;
            return;
        }
        if ((state->player->actor->colGroundAttr & 0x100) != 0) {
            u16 meshIndex = state->player->actor->colMesh;
            if (meshIndex >= 1 && meshIndex < lbl_1_bss_2C->count + 1) {
                state->activeEntry = &lbl_1_bss_2C->entries[meshIndex - 1];
                PSVECSubtract(&state->activeEntry->position,
                              &state->player->actor->pos, &offset);
                state->travelTicks = (s32)((PSVECMag(&offset) / 5.6f) * 1.4f);
                state->positionState = 0;
                state->clearTargetHistoryOnArrival = 0;
                state->sequenceType = 0;
                return;
            }
        }
        {
        Point3d movement;
        PSVECSubtract(&actorPosition, &state->lastPositionSample, &movement);
        movement.y = 0.0f;
        if (PSVECSquareMag(&movement) < 50.0f) {
            state->positionState++;
            if (state->positionState > 10) {
                state->waitFrames = fn_1_5718((M627PlayerWork *)state);
                fn_1_58A8((M627PlayerWork *)state, 1);
                state->clearTargetHistoryOnArrival = 0;
                return;
            }
        } else {
            state->lastPositionSample = actorPosition;
            state->positionState = 0;
        }
        }
        PSVECNormalize(&offset, &offset);
        PSVECScale(&offset, &offset, 80.0f);
        MgPlayerPadSet(state->player, (s32)-offset.x, (s32)offset.z, 0, 0);
        return;
    }
    if (state->activeEntry == NULL) {
        fn_1_58A8((M627PlayerWork *)state, 0);
        state->clearTargetHistoryOnArrival = 1;
        return;
    }
    if ((state->activeEntry->flags & (state->cameraGroup == 0 ? 1 : 2)) == 0) {
        state->activeEntry = NULL;
        return;
    }
    entryPosition = state->activeEntry->position;
    actorPosition = *(Point3d *)&state->player->actor->pos;
    entryPosition.y = actorPosition.y = 0.0f;
    PSVECSubtract(&actorPosition, &entryPosition, &offset);
    if (PSVECSquareMag(&offset) < 18225.0f) {
        PSVECNormalize(&offset, &offset);
        PSVECScale(&offset, &offset, 80.0f);
        MgPlayerPadSet(state->player, (s32)-offset.x, (s32)offset.z, 256, 256);
        state->activeEntry = NULL;
        state->waitFrames = fn_1_5718((M627PlayerWork *)state);
        if (state->clearTargetHistoryOnArrival != 0) {
            state->previousEntry = state->currentEntry = NULL;
        }
    } else {
        state->travelTicks--;
        if (state->travelTicks <= 0) {
            state->waitFrames = fn_1_5718((M627PlayerWork *)state);
            if (state->activeEntry != NULL) {
                fn_1_4D54(state, state->activeEntry);
            }
            fn_1_58A8((M627PlayerWork *)state, 1);
            state->clearTargetHistoryOnArrival = 0;
            return;
        }
        if ((state->player->actor->colGroundAttr & 0x100) != 0) {
            u16 meshIndex = state->player->actor->colMesh;
            if (meshIndex >= 1 && meshIndex < lbl_1_bss_2C->count + 1) {
                state->activeEntry = &lbl_1_bss_2C->entries[meshIndex - 1];
                PSVECSubtract(&state->activeEntry->position,
                              &state->player->actor->pos, &offset);
                state->travelTicks = (s32)((PSVECMag(&offset) / 5.6f) * 1.4f);
                state->positionState = 0;
                state->clearTargetHistoryOnArrival = 0;
                state->sequenceType = 0;
                return;
            }
        }
        {
        Point3d movement;
        PSVECSubtract(&actorPosition, &state->lastPositionSample, &movement);
        movement.y = 0.0f;
        if (PSVECSquareMag(&movement) < 50.0f) {
            state->positionState++;
            if (state->positionState > 10) {
                state->waitFrames = fn_1_5718((M627PlayerWork *)state);
                if (state->activeEntry != NULL) {
                    fn_1_4D54(state, state->activeEntry);
                }
                fn_1_58A8((M627PlayerWork *)state, 1);
                state->clearTargetHistoryOnArrival = 0;
                return;
            }
        } else {
            state->lastPositionSample = actorPosition;
            state->positionState = 0;
        }
        }
        PSVECNormalize(&offset, &offset);
        PSVECScale(&offset, &offset, 80.0f);
        MgPlayerPadSet(state->player, (s32)-offset.x, (s32)offset.z, 0, 0);
    }
}

/* Creates one camera group's moving actor when fn_1_798 sets up the board. */
void fn_1_6CE0(OMOBJ *object)
{
    M627ActorWork *state = object->data;
    MGACTOR_PARAM actorParameters = {0};
    s32 cameraBit;
    u32 modelCameraMask;
    u32 actorCollisionMask;
    f32 collisionRadius;
    f32 actorRadius;

    if (state->cameraGroup == 0) {
        cameraBit = 1;
    } else {
        cameraBit = 2;
    }
    modelCameraMask = cameraBit;
    if (state->cameraGroup == 0) {
        actorRadius = 75.0f;
    } else {
        actorRadius = 175.0f;
    }
    collisionRadius = actorRadius;
    state->unusedCollisionState = 0;
    state->modelId = Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_4C[state->cameraGroup],
                                                        HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(state->modelId, (u16)modelCameraMask);
    Hu3DModelShadowSet(state->modelId);
    state->motionId = Hu3DJointMotion(state->modelId,
                                      HuDataSelHeapReadNum(lbl_1_data_54[state->cameraGroup],
                                                          HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(state->modelId, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(state->modelId, HU3D_ATTR_DISPOFF);

    actorParameters.height = 2.0f * collisionRadius;
    actorParameters.radius = collisionRadius;
    actorParameters.param = state->cameraGroup;
    actorParameters.type = 5;
    actorParameters.attr = 2;
    actorParameters.correctHookParam = 0;
    actorParameters.narrowHook = NULL;
    actorParameters.correctHook = NULL;
    state->actor = MgActorCreate(&actorParameters, state->modelId);
    state->actor->gravity = 0.0f;
    if (state->cameraGroup == 0) {
        actorCollisionMask = 1;
    } else {
        actorCollisionMask = 2;
    }
    MgActorColMaskSet(state->actor, actorCollisionMask);
    MgPlayerColDisable(state->actor);
    MgActorPosSet(state->actor, &lbl_1_data_F1C[state->cameraGroup]);
    state->velocity.x = state->velocity.y = state->velocity.z = 0.0f;
    state->unusedPositionVector.x = state->unusedPositionVector.y = state->unusedPositionVector.z =
        0.0f;
    state->collisionMode = 0;
    state->soundMode = 1;
    fn_1_140(object, fn_1_6F60);
}

/* During the phase-3 start announcement, starts the ball's entrance motion and landing check. */
void fn_1_6F60(OMOBJ *object)
{
    s32 pan;
    M627ActorWork *work = object->data;

    if (MgSeqModeGet() == 3) {
        work->unusedCollisionState = 0;
        Hu3DModelAttrReset(work->modelId, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
        Hu3DMotionSet(work->modelId, work->motionId);
        if (work->cameraGroup == 0) {
            pan = 48;
        } else {
            pan = 80;
        }
        {
            s16 soundPan = pan;

        HuAudFXPlayVolPan(MSM_SE_M627_ACTOR_START, 127, soundPan);
        }
        fn_1_140((OMOBJ *)object, fn_1_7020);
        return;
    }
}

/* Waits for the actor to land, enables its collision, then pans the landing cue by screen
 * position. */
void fn_1_7020(OMOBJ *object)
{
    M627ActorWork *state = object->data;
    Point3d position;
    Point3d projectedCameraOne;
    Point3d projectedCameraTwo;
    char *objectName;
    s16 cameraOnePan;
    s16 cameraTwoPan;
    s32 cameraOneSoundHandle;
    s32 cameraTwoSoundHandle;

    if (state->cameraGroup == 0) {
        objectName = lbl_1_data_F78;
    } else {
        objectName = lbl_1_data_F82;
    }
    Hu3DModelObjPosGet(state->modelId, objectName, &position);
    position.y = 0.0f;
    if (Hu3DMotionEndCheck(state->modelId) != 0) {
        Hu3DMotionSet(state->modelId, -1);
        state->unusedCollisionState = 0;
        MgPlayerColEnable(state->actor, &position);
        Hu3DModelPosSetV(state->modelId, &position);
        state->soundActive = 1;
        state->boundarySoundPlayed = 0;
        if (state->cameraGroup == 0) {
            Hu3D3Dto2D(&position, 1, &projectedCameraOne);
            if (projectedCameraOne.x > 316.0f) {
                cameraOnePan = 64;
            } else if (projectedCameraOne.x < 16.0f) {
                cameraOnePan = 32;
            } else {
                cameraOnePan = (s16)(32.0f +
                                     (32.0f * (projectedCameraOne.x - 16.0f)) / 300.0f);
            }
            cameraOneSoundHandle = HuAudFXPlayVolPan(MSM_SE_M627_LANDING_FIRST_SCREEN, 127,
                                                     cameraOnePan);
            state->soundHandle = cameraOneSoundHandle;
        } else {
            Hu3D3Dto2D(&position, 2, &projectedCameraTwo);
            if (projectedCameraTwo.x > 624.0f) {
                cameraTwoPan = 96;
            } else if (projectedCameraTwo.x < 324.0f) {
                cameraTwoPan = 64;
            } else {
                cameraTwoPan = (s16)(64.0f +
                                     (32.0f * (projectedCameraTwo.x - 324.0f)) / 300.0f);
            }
            cameraTwoSoundHandle = HuAudFXPlayVolPan(MSM_SE_M627_LANDING_SECOND_SCREEN, 127,
                                                     cameraTwoPan);
            state->soundHandle = cameraTwoSoundHandle;
        }
        fn_1_140(object, fn_1_8B40);
        return;
    }
}

/* Finds a moving point's sphere intersection in (-0.1, 1.1); snaps times below 0.1 to zero
 * and above 0.9 to one. */
s32 fn_1_7274(Point3d *sphereCenter, Point3d *pointStart, Point3d *direction, f32 radius,
              f32 *intersectionTime)
{
    Point3d offset;
    f32 projectedLength;
    f32 directionLengthSquared;
    f32 discriminant;
    f32 separationSquared;
    f32 firstTime;
    f32 secondTime;
    f32 swapTime;
    f32 intersectionTimeValue;

    PSVECSubtract(pointStart, sphereCenter, &offset);
    projectedLength = PSVECDotProduct(direction, &offset);
    directionLengthSquared = PSVECSquareMag(direction);
    separationSquared = PSVECSquareMag(&offset);
    discriminant = projectedLength * projectedLength -
                   directionLengthSquared * (separationSquared - radius * radius);
    if (discriminant < 0.0f) {
        return 0;
    }
    discriminant = sqrtf(discriminant);
    firstTime = (discriminant - projectedLength) / directionLengthSquared;
    secondTime = (-discriminant - projectedLength) / directionLengthSquared;
    if (((firstTime <= -0.1f) || (firstTime >= 1.1f)) &&
        ((secondTime <= -0.1f) || (secondTime >= 1.1f))) {
        return 0;
    }
    if (firstTime > secondTime) {
        swapTime = firstTime;
        firstTime = secondTime;
        secondTime = swapTime;
    }
    if (firstTime <= -0.1f) {
        intersectionTimeValue = secondTime;
    } else {
        intersectionTimeValue = firstTime;
    }
    if (intersectionTimeValue < 0.1f) {
        intersectionTimeValue = 0.0f;
    }
    if (intersectionTimeValue > 0.9f) {
        intersectionTimeValue = 1.0f;
    }
    *intersectionTime = intersectionTimeValue;
    return 1;
}

/* Tests a center sweep against an offset boundary segment; returns fraction zero when already
 * inside the offset and moving farther inward. */
s16 fn_1_7580(Point3d *origin, Point3d *projectionAxis, Point3d *boundaryNormal,
               Point3d *sweepStart, Point3d *sweepDelta, f32 maxProjection, f32 *hitParameter)
{
    Point3d startOffset;
    Point3d endPoint;
    Point3d scaledAxis;
    Point3d separation;
    f32 startProjection;
    f32 endProjection;
    f32 candidateParameter;
    f32 axisProjection;

    PSVECSubtract(sweepStart, origin, &startOffset);
    startProjection = PSVECDotProduct(boundaryNormal, &startOffset);
    PSVECAdd(sweepStart, sweepDelta, &endPoint);
    PSVECSubtract(&endPoint, origin, &endPoint);
    endProjection = PSVECDotProduct(boundaryNormal, &endPoint);
    if (endProjection >= startProjection || endProjection > maxProjection) {
        return 0;
    }
    if (startProjection <= maxProjection) {
        PSVECScale(boundaryNormal, &scaledAxis, maxProjection);
        PSVECSubtract(&startOffset, &scaledAxis, &separation);
        axisProjection = PSVECDotProduct(&separation, projectionAxis);
        if (axisProjection >= 0.0f && axisProjection <= PSVECSquareMag(projectionAxis)) {
            *hitParameter = 0.0f;
            return 1;
        }
    }
    axisProjection = PSVECDotProduct(sweepDelta, boundaryNormal);
    if (0.0f == axisProjection) {
        return 0;
    }
    candidateParameter =
        (maxProjection - PSVECDotProduct(&startOffset, boundaryNormal)) / axisProjection;
    if (candidateParameter < 0.0f || candidateParameter > 1.0f) {
        return 0;
    }
    PSVECScale(sweepDelta, &endPoint, candidateParameter);
    PSVECAdd(sweepStart, &endPoint, &separation);
    PSVECSubtract(&separation, origin, &endPoint);
    PSVECScale(boundaryNormal, &scaledAxis, maxProjection);
    PSVECSubtract(&endPoint, &scaledAxis, &separation);
    axisProjection = PSVECDotProduct(&separation, projectionAxis);
    if (axisProjection < 0.0f || axisProjection > PSVECSquareMag(projectionAxis)) {
        return 0;
    }
    *hitParameter = candidateParameter;
    return 1;
}

/* Tests movement against every playfield boundary and stores the nearest hit. */
s16 fn_1_7808(Point3d *queryStart, Point3d *movement,
              f32 boundaryOffset, M627CollisionHit *result)
{
    s16 resultCode;
    s16 entryIndex;
    s16 hitResult;

    for (entryIndex = 0; entryIndex < lbl_1_bss_2C->boundaryCount; entryIndex++) {
        M627BoundaryLine *entry = &lbl_1_bss_2C->boundaries[entryIndex];
        f32 distance;

        hitResult = fn_1_7580(&entry->origin, &entry->direction, &entry->normal,
                              queryStart, movement, boundaryOffset, &distance);
        if (hitResult == 1 && distance < result->distance) {
            result->distance = distance;
            result->found = 1;
            result->normal = entry->normal;
        }
    }

    if (100.0f != result->distance) {
        resultCode = 1;
    } else {
        resultCode = 0;
    }
    return (s16)resultCode;
}

/* Tests a point sweep against a circle, rejecting discriminants at most 0.01; accepts times
 * in (-0.1, 1.1), snapping below 0.1 to zero and above one to one. */
s32 fn_1_7918(Point3d *circleCenter, Point3d *point, Point3d *direction, f32 radiusSquared,
              f32 *intersectionTime)
{
    Point3d offset;
    f32 projectedLength;
    f32 directionLengthSquared;
    f32 separationSquared;
    f32 discriminant;
    f32 time;
    PSVECSubtract(point, circleCenter, & offset);
    projectedLength = PSVECDotProduct(& offset, direction);
    directionLengthSquared = PSVECSquareMag(direction);
    if (0.0f == directionLengthSquared)
    {
        return 0;
    }
    separationSquared = PSVECSquareMag(& offset);
    discriminant =
        projectedLength * projectedLength -
        directionLengthSquared * (separationSquared - radiusSquared);
    if (discriminant <= 0.01f)
    {
        return 0;
    }
    time = (- projectedLength - sqrtf(discriminant)) / directionLengthSquared;
    if (time <= -0.1f)
    {
        return 0;
    }
    if (time >= 1.1f)
    {
        return 0;
    }
    if (time > 1.0f)
    {
        time = 1.0f;
    }
    if (time < 0.1f)
    {
        time = 0.0f;
    }
    * intersectionTime = time;
    return 1;
}

/* Tests a sphere sweep against the tapered wall in rotated collision space: 1 writes the
 * normal and hit fraction, 2 means below the lower end without outputs, and 0 means no hit. */
s16 fn_1_7B98(Point3d *start, Point3d *direction, f32 radius,
              Point3d *contactNormal, f32 *intersectionTime)
{
    Point3d offset;
    Point3d directionAtTime;
    f32 radiusAlongSlope;
    f32 adjustedOriginRadius;
    f32 adjustedLowerCenterRadius;
    f32 radiusAtStart;
    f32 radiusAtNextPoint;
    f32 radiusChange;
    f32 linearTerm;
    f32 quadraticTerm;
    f32 constantTerm;
    f32 discriminant;
    f32 time;

    if (start->y + radius < lbl_1_data_DE0.lowerCenter.y) {
        return 2;
    }

    radiusAlongSlope = (f32)((f64)radius /
                             cos((3.141592653589793 * (f64)lbl_1_data_DE0.angle) /
                                 180.0));
    adjustedOriginRadius = lbl_1_data_DE0.originRadius - radiusAlongSlope;
    adjustedLowerCenterRadius = lbl_1_data_DE0.lowerCenterRadius - radiusAlongSlope;
    radiusAtStart = adjustedOriginRadius +
                    (adjustedLowerCenterRadius - adjustedOriginRadius) *
                    ((start->y - lbl_1_data_DE0.origin.y) /
                     (lbl_1_data_DE0.lowerCenter.y - lbl_1_data_DE0.origin.y));
    radiusAtNextPoint = adjustedOriginRadius +
                        (adjustedLowerCenterRadius - adjustedOriginRadius) *
                        (((start->y + direction->y) - lbl_1_data_DE0.origin.y) /
                         (lbl_1_data_DE0.lowerCenter.y - lbl_1_data_DE0.origin.y));
    radiusChange = radiusAtNextPoint - radiusAtStart;

    PSVECSubtract(start, &lbl_1_data_DE0.origin, &offset);
    linearTerm = (offset.x * direction->x + offset.z * direction->z) -
                 radiusAtStart * radiusChange;
    quadraticTerm = (direction->x * direction->x + direction->z * direction->z) -
                    radiusChange * radiusChange;
    constantTerm = (offset.x * offset.x + offset.z * offset.z) -
                   (radiusAtStart * radiusAtStart);
    if (quadraticTerm < 0.1f) {
        return 0;
    }

    discriminant = (linearTerm * linearTerm) - (quadraticTerm * constantTerm);
    if (discriminant <= 0.0f) {
        return 0;
    }

    time = (-linearTerm + sqrtf(discriminant)) / quadraticTerm;
    if (time < 0.0f || time > 1.0f) {
        return 0;
    }
    *intersectionTime = time;

    PSVECScale(direction, &directionAtTime, time);
    PSVECAdd(&offset, &directionAtTime, contactNormal);
    contactNormal->y = 0.0f;
    if (PSVECSquareMag(contactNormal) > 0.01f) {
        PSVECNormalize(contactNormal, contactNormal);
        PSVECScale(contactNormal, contactNormal,
                   (f32)-cos((3.141592653589793 * (f64)lbl_1_data_DE0.angle) /
                             180.0));
        contactNormal->y =
            (f32)sin((3.141592653589793 * (f64)lbl_1_data_DE0.angle) /
                     180.0);
        PSVECNormalize(contactNormal, contactNormal);
    } else {
        contactNormal->x = contactNormal->z = 0.0f;
        contactNormal->y = 1.0f;
    }
    return 1;
}

/* Finds this camera group's nearest boundary or block hit; randomly perturbs XZ normals
 * whose X component is nearly zero. */
s16 fn_1_8098(s32 cameraGroup, Point3d *start, Point3d *direction,
              M627CollisionHit *hit)
{
    Point3d boundsEnd;
    Point3d boundsStart;
    Point3d contactPosition;
    Point3d offset;
    Point3d contact;
    s16 index;
    M627BoundaryLine *face;
    M627Entry *wall;
    M627Entry *circle;
    f32 faceRadius;
    f32 circleRadius;
    f32 intersectionDistance;
    f32 projectedLength;
    f32 expandedRadius;
    f32 expandedRadiusSquared;
    f32 root;
    f32 time;
    s16 faceIndex;
    u16 categoryMask;
    s16 faceResult;

    categoryMask = cameraGroup == 0 ? 1 : 2;
    faceRadius = cameraGroup == 0 ? 75.0f : 175.0f;
    circleRadius = cameraGroup == 0 ? 73.48f : 131.0f;
    expandedRadius = 90.0f + circleRadius;
    expandedRadiusSquared = expandedRadius * expandedRadius;
    hit->distance = 100.0f;
    hit->found = 0;

    for (faceIndex = 0; faceIndex < lbl_1_bss_2C->boundaryCount; faceIndex++) {
        face = &lbl_1_bss_2C->boundaries[faceIndex];
        faceResult = fn_1_7580(&face->origin, &face->direction, &face->normal,
                               start, direction, faceRadius, &intersectionDistance);
        if (faceResult == 1 && intersectionDistance < hit->distance) {
            hit->distance = intersectionDistance;
            hit->found = 1;
            hit->normal = face->normal;
        }
    }

    if (direction->x < 0.0f) {
        boundsStart.x = direction->x + (start->x - circleRadius) - 1.0f;
        boundsEnd.x = 1.0f + (start->x + circleRadius);
    } else {
        boundsStart.x = (start->x - circleRadius) - 1.0f;
        boundsEnd.x = 1.0f + (direction->x + (start->x + circleRadius));
    }
    if (direction->z < 0.0f) {
        boundsStart.z = direction->z + (start->z - circleRadius) - 1.0f;
        boundsEnd.z = 1.0f + (start->z + circleRadius);
    } else {
        boundsStart.z = (start->z - circleRadius) - 1.0f;
        boundsEnd.z = 1.0f + (direction->z + (start->z + circleRadius));
    }

    for (index = 0; index < lbl_1_bss_2C->groupCount; index++) {
        wall = &lbl_1_bss_2C->groups[index];
        if (boundsStart.x > 90.0f + wall->position.x ||
            boundsEnd.x < wall->position.x - 90.0f ||
            boundsStart.z > 90.0f + wall->position.z ||
            boundsEnd.z < wall->position.z - 90.0f) {
            continue;
        }
        {
            if ((s16)fn_1_7918(&wall->position, start, direction,
                               expandedRadiusSquared, &time) != 0 && time < hit->distance) {
                        hit->distance = time;
                        hit->found = 1;
                        PSVECScale(direction, &contact, time);
                        PSVECAdd(start, &contact, &contactPosition);
                        PSVECSubtract(&contactPosition, &wall->position, &hit->normal);
                        hit->normal.y = 0.0f;
                        PSVECNormalize(&hit->normal, &hit->normal);
            }
        }
    }

    expandedRadius = 50.0f + circleRadius;
    expandedRadiusSquared = expandedRadius * expandedRadius;
    for (index = 0; index < lbl_1_bss_2C->count; index++) {
        circle = &lbl_1_bss_2C->entries[index];
        if ((circle->flags & categoryMask) == 0 ||
            boundsStart.x > 50.0f + circle->position.x ||
            boundsEnd.x < circle->position.x - 50.0f ||
            boundsStart.z > 50.0f + circle->position.z ||
            boundsEnd.z < circle->position.z - 50.0f) {
            continue;
        }
        if ((s16) fn_1_7918(&circle->position, start, direction, expandedRadiusSquared, &time) ==
                1 &&
            time < hit->distance) {
            hit->distance = time;
            hit->found = 1;
            PSVECScale(direction, &contact, time);
            PSVECAdd(start, &contact, &contactPosition);
            PSVECSubtract(&contactPosition, &circle->position, &hit->normal);
            hit->normal.y = 0.0f;
            PSVECNormalize(&hit->normal, &hit->normal);
        }
    }
    if (hit->found > 0) {
        if (fabsf(hit->normal.x) < 0.01f) {
            hit->normal.x = frandf() < 0.5f ?
                                    (-0.06f) : 0.06f;
            PSVECNormalize(&hit->normal, &hit->normal);
        }
        return 1;
    }
    return 0;
}

/* Installed when a moving actor lands; advances it along the course and reflects it off blocks. */
void fn_1_8B40(OMOBJ *object)
{
    M627ActorWork *state = object->data;
    Mtx rotation;
    M627CollisionHit hit;
    Point3d probeDirection;
    Point3d position;
    Point3d direction;
    Point3d nextDirection;
    Point3d reflectionDirection;
    Point3d projectedNormal;
    Point3d normalizedDirection;
    Point3d step;
    f32 speed;
    f32 normalDot;
    f32 accelerationNormal;
    f32 previousAngle;
    f32 angleDelta;
    s16 playerIndex;
    s16 i;
    s16 didCollide = 0;
    s16 playTurnSound = 0;
    s32 losingCameraGroup;

    if (state->collisionMode == 0) {
        if (state->actor->pos.z >= 3131.57f) {
            for (i = 0; i < 4; i++) {
                if (state->cameraGroup == lbl_1_bss_18[i]->cameraGroup) {
                    lbl_1_bss_18[i]->active = 1;
                }
            }
            lbl_1_bss_10 = state->cameraGroup;
            losingCameraGroup = lbl_1_bss_10 == 0 ? 1 : 0;
            lbl_1_bss_8[0]->collisionMode = lbl_1_bss_8[1]->collisionMode = 1;
            lbl_1_bss_8[losingCameraGroup]->soundMode = 0;
            if (lbl_1_bss_8[losingCameraGroup]->soundActive != 0) {
                s32 soundHandle = lbl_1_bss_8[losingCameraGroup]->soundHandle;
                HuAudFXStop(soundHandle);
            }
            MgSeqModeNext();
        }
    } else if (state->actor->pos.z >= 3600.0f) {
        PSMTXRotRad(rotation, 88, 0.5235988f);
        PSMTXMultVecSR(rotation, &state->velocity, &state->velocity);
        fn_1_140(object, fn_1_9A2C);
        return;
    }

    probeDirection.x = probeDirection.y = 0.0f;
    probeDirection.z = 1.0f;
    position = *(Point3d *)&state->actor->pos;
    position.y = 0.0f;
    switch (fn_1_8098(state->cameraGroup, &position, &probeDirection, &hit)) {
    case 0:
        state->velocity.z += 0.6f;
        break;
    default:
        reflectionDirection.x = reflectionDirection.y = 0.0f;
        reflectionDirection.z = 0.6f;
        accelerationNormal = PSVECDotProduct(&reflectionDirection, &hit.normal);
        PSVECScale(&hit.normal, &projectedNormal, accelerationNormal);
        PSVECSubtract(&reflectionDirection, &projectedNormal, &reflectionDirection);
        PSVECAdd(&state->velocity, &reflectionDirection, &state->velocity);
    }

    if (0.0f == PSVECMag(&state->velocity)) {
        if (state->soundActive != 0) {
          if (state->soundMode != 0) {
            if (state->cameraGroup == 0) {
                fn_1_430(1, &position, MSM_SE_M627_TURN_FIRST_SCREEN);
            } else {
                fn_1_430(2, &position, MSM_SE_M627_TURN_SECOND_SCREEN);
            }
          }
            state->soundActive = 0;
        }
        return;
    }

    nextDirection = state->velocity;
    direction = nextDirection;
    while (fn_1_8098(state->cameraGroup, &position, &direction, &hit) != 0) {
        didCollide = 1;
        speed = PSVECMag(&direction);
        PSVECNormalize(&direction, &normalizedDirection);
        PSVECScale(&direction, &step, hit.distance);
        PSVECAdd(&position, &step, &position);
        normalDot = PSVECDotProduct(&normalizedDirection, &hit.normal);
        nextDirection.x = speed * (0.95f *
            (normalizedDirection.x - normalDot * hit.normal.x) -
            0.4f * (normalDot * hit.normal.x));
        nextDirection.z = speed * (0.95f *
            (normalizedDirection.z - normalDot * hit.normal.z) -
            0.4f * (normalDot * hit.normal.z));
        nextDirection.y = 0.0f;
        if (PSVECMag(&nextDirection) < 0.1f) {
            direction.x = direction.y = direction.z = 0.0f;
            nextDirection = direction;
            break;
        }
        PSVECScale(&nextDirection, &direction, 1.0f - hit.distance);
        if (PSVECMag(&direction) < 0.1f) {
            direction.x = direction.y = direction.z = 0.0f;
            break;
        }
    }

    if (didCollide != 0 && PSVECMag(&state->velocity) > 1.5f &&
        PSVECMag(&nextDirection) > 1.5f) {
        previousAngle = (f32)(180.0 *
            (atan2((f64)state->velocity.x, (f64)state->velocity.z) / 3.141592653589793));
        angleDelta = (f32)(180.0 *
            (atan2((f64)nextDirection.x, (f64)nextDirection.z) / 3.141592653589793));
        angleDelta -= previousAngle;
        while (angleDelta >= 180.0f) {
            angleDelta -= 360.0f;
        }
        while (angleDelta < -180.0f) {
            angleDelta += 360.0f;
        }
        if ((angleDelta < 0.0f ? -angleDelta : angleDelta) > 5.0f) {
            playTurnSound = 1;
        }
    }

    if (playTurnSound != 0) {
        if (state->soundMode != 0) {
            if (state->cameraGroup == 0) {
                fn_1_430(1, &position, MSM_SE_M627_TURN_FIRST_SCREEN);
            } else {
                fn_1_430(2, &position, MSM_SE_M627_TURN_SECOND_SCREEN);
            }
        }
        state->soundActive = 0;
    }

    position.x += direction.x;
    position.z += direction.z;
    position.y = 0.0f;
    if (didCollide == 0 && playTurnSound == 0) {
        if (state->soundActive != 0) {
            if (state->soundMode != 0) {
                fn_1_5B8((s16)(state->cameraGroup == 0 ? 1 : 2), &position, state->soundHandle);
            }
        } else if (PSVECMag(&state->velocity) > 1.5f) {
            if (state->soundMode != 0) {
                if (state->cameraGroup == 0) {
                    state->soundHandle = fn_1_430(1, &position,
                                                  MSM_SE_M627_LANDING_FIRST_SCREEN);
                } else {
                    state->soundHandle = fn_1_430(2, &position,
                                                  MSM_SE_M627_LANDING_SECOND_SCREEN);
                }
            }
            state->soundActive = 1;
        }
    } else if (state->soundActive != 0) {
      if (state->soundMode != 0) {
        if (state->cameraGroup == 0) {
            fn_1_430(1, &position, MSM_SE_M627_TURN_FIRST_SCREEN);
        } else {
            fn_1_430(2, &position, MSM_SE_M627_TURN_SECOND_SCREEN);
        }
      }
        state->soundActive = 0;
    }

    state->velocity = nextDirection;
    MgActorPosSetRaw(state->actor, &position);
}

/* Installed at the end of the course; advances the actor through the sloped boundary section. */
void fn_1_9A2C(OMOBJ *object)
{
    M627ActorWork *state = object->data;
    Mtx positiveRotation;
    Mtx negativeRotation;
    Point3d position;
    Point3d normal;
    Point3d sweepDirection;
    Point3d contactPosition;
    Point3d velocity;
    Point3d nextVelocity;
    Point3d bounce;
    Point3d projectedNormal;
    Point3d normalizedVelocity;
    Point3d step;
    f32 ballRadius;
    f32 distance;
    f32 speed;
    f32 normalDot;
    f32 accelerationNormal;
    f32 previousSpeed;
    f32 nextSpeed;
    f32 previousAngle;
    f32 angleDelta;
    s16 lastContactType;
    s16 didCollide;
    s16 playTurnSound;

    ballRadius = state->cameraGroup == 0 ? 75.0f : 175.0f;
    didCollide = 0;
    playTurnSound = 0;
    if (MgSeqModeGet() == 8) {
        fn_1_140(object, fn_1_AB0C);
        return;
    }
    PSMTXRotRad(positiveRotation, 88, 0.5235988f);
    PSMTXRotRad(negativeRotation, 88, -0.5235988f);
    position = *(Point3d *)&state->actor->pos;
    position.y -= ballRadius;
    PSMTXMultVec(positiveRotation, &position, &position);
    /* Sound cues project this rotated-space accumulator as world space. It starts with Y zero;
     * collision steps below can change its Y component. */
    contactPosition = position;
    contactPosition.y = 0.0f;
    /* This sweep sets X and Z only; its Y component is left uninitialized. */
    sweepDirection.x = sweepDirection.z = 0.0f;
    sweepDirection.x = -1.0f;
    switch (fn_1_7B98(&position, &sweepDirection, ballRadius, &normal, &distance)) {
    case 0:
        state->velocity.y -= 0.6f;
        break;
    case 2:
        state->velocity.x = state->velocity.z = 0.0f;
        state->velocity.y -= 0.6f;
        break;
    case 1:
    default:
        /* On a wall hit, the acceleration candidate points along negative X as well as negative
         * Y. */
        bounce.x = bounce.y = -0.6f;
        bounce.z = 0.0f;
        accelerationNormal = PSVECDotProduct(&bounce, &normal);
        PSVECScale(&normal, &projectedNormal, accelerationNormal);
        PSVECSubtract(&bounce, &projectedNormal, &bounce);
        PSVECAdd(&state->velocity, &bounce, &state->velocity);
        break;
    }

    if (PSVECMag(&state->velocity) < 0.1f) {
        if (state->soundActive != 0) {
            if (state->soundMode != 0) {
                if (state->cameraGroup == 0) {
                    fn_1_430(1, &contactPosition, MSM_SE_M627_TURN_FIRST_SCREEN);
                } else {
                    fn_1_430(2, &contactPosition, MSM_SE_M627_TURN_SECOND_SCREEN);
                }
            }
            state->soundActive = 0;
        }
        state->velocity.x = state->velocity.y = state->velocity.z = 0.0f;
        return;
    }

    nextVelocity = state->velocity;
    velocity = nextVelocity;
    while ((lastContactType = fn_1_7B98(&position, &velocity, ballRadius,
                                       &normal, &distance)) == 1) {
        didCollide = 1;
        speed = PSVECMag(&velocity);
        PSVECNormalize(&velocity, &normalizedVelocity);
        PSVECScale(&velocity, &step, distance);
        /* Advance the sound-position sample only; repeated wall queries still use position
         * unchanged. */
        PSVECAdd(&contactPosition, &step, &contactPosition);
        normalDot = PSVECDotProduct(&normalizedVelocity, &normal);
        nextVelocity.x = speed *
                         (0.95f *
                          (normalizedVelocity.x - normalDot * normal.x) -
                          0.4f * (normalDot * normal.x));
        nextVelocity.z = speed *
                         (0.95f *
                          (normalizedVelocity.z - normalDot * normal.z) -
                          0.4f * (normalDot * normal.z));
        nextVelocity.y = speed *
                         (0.95f *
                          (normalizedVelocity.y - normalDot * normal.y) -
                          0.4f * (normalDot * normal.y));
        if (PSVECMag(&nextVelocity) < 0.1f) {
            velocity.x = velocity.y = velocity.z = 0.0f;
            nextVelocity = velocity;
            break;
        }
        PSVECScale(&nextVelocity, &velocity, 1.0f - distance);
        if (PSVECMag(&velocity) < 0.1f) {
            velocity.x = velocity.y = velocity.z = 0.0f;
            break;
        }
    }

    if (didCollide != 0) {
        playTurnSound = 1;
        previousSpeed = PSVECMag(&state->velocity);
        if (previousSpeed > 1.5f) {
            nextSpeed = PSVECMag(&nextVelocity);
            if (nextSpeed > 1.5f) {
                previousAngle = (f32)(180.0 *
                    (atan2((f64)state->velocity.x, (f64)state->velocity.z) /
                     3.141592653589793));
                angleDelta = (f32)(180.0 *
                    (atan2((f64)nextVelocity.x, (f64)nextVelocity.z) /
                     3.141592653589793));
                angleDelta -= previousAngle;
                while (angleDelta >= 180.0f) {
                    angleDelta -= 360.0f;
                }
                while (angleDelta < -180.0f) {
                    angleDelta += 360.0f;
                }
                if ((angleDelta < 0.0f ? -angleDelta : angleDelta) < 5.0f) {
                    playTurnSound = 0;
                }
            }
        } else {
            playTurnSound = 0;
        }
    }
    if (playTurnSound != 0 && state->soundMode != 0) {
        if (state->cameraGroup == 0) {
            fn_1_430(1, &contactPosition, MSM_SE_M627_TURN_FIRST_SCREEN);
        } else {
            fn_1_430(2, &contactPosition, MSM_SE_M627_TURN_SECOND_SCREEN);
        }
    }

    if (lastContactType == 2) {
        if (state->boundarySoundPlayed == 0 && lbl_1_bss_10 == state->cameraGroup) {
            HuAudFXPlayVolPan(MSM_SE_M627_BOUNDARY_HIT, 127, 64);
            state->boundarySoundPlayed = 1;
        }
        if (state->soundActive != 0) {
            if (state->soundMode != 0) {
                if (state->cameraGroup == 0) {
                    fn_1_430(1, &contactPosition, MSM_SE_M627_TURN_FIRST_SCREEN);
                } else {
                    fn_1_430(2, &contactPosition, MSM_SE_M627_TURN_SECOND_SCREEN);
                }
            }
            state->soundActive = 0;
        }
        nextVelocity.x = nextVelocity.z = 0.0f;
        velocity.x = velocity.z = 0.0f;
    }

    position.x += velocity.x;
    position.y += velocity.y;
    position.z += velocity.z;
    if (lastContactType != 2) {
        if (didCollide == 0 && playTurnSound == 0) {
            if (state->soundActive != 0) {
                if (state->soundMode != 0) {
                    fn_1_5B8((s16) (state->cameraGroup == 0 ? 1 : 2), &contactPosition,
                             state->soundHandle);
                }
            } else {
                if (state->soundMode != 0) {
                    if (state->cameraGroup == 0) {
                        state->soundHandle = fn_1_430(
                            1, &contactPosition, MSM_SE_M627_LANDING_FIRST_SCREEN);
                    } else {
                        state->soundHandle = fn_1_430(
                            2, &contactPosition, MSM_SE_M627_LANDING_SECOND_SCREEN);
                    }
                }
                state->soundActive = 1;
            }
        } else if (state->soundActive != 0) {
            if (state->soundMode != 0) {
                if (state->cameraGroup == 0) {
                    fn_1_430(1, &contactPosition, MSM_SE_M627_TURN_FIRST_SCREEN);
                } else {
                    /* This second-screen cue still uses camera 1 for projection and panning. */
                    fn_1_430(1, &contactPosition, MSM_SE_M627_TURN_SECOND_SCREEN);
                }
            }
            state->soundActive = 0;
        }
    }

    state->velocity = nextVelocity;
    PSMTXMultVec(negativeRotation, &position, &position);
    MgActorPosSetRaw(state->actor, &position);
}

/* Installed after the last actor update to stop scheduling that object's callback. */
void fn_1_AB0C(OMOBJ *obj)
{
    void *work = obj->data;

    fn_1_140(obj, NULL);
}
