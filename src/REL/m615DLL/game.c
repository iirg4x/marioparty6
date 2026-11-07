/* Note to Self gameplay: the players and their movement, the computer players, scoring,
 * stage effects and the stage music. */
#include "dolphin/types.h"

#include "math.h"

#include "humath.h"

#include "game/memory.h"

#include "include/dolphin/types.h"

#include "game/object.h"

#include "game/gamework.h"

#include "game/mg/actman.h"

#include "game/mg/seqman.h"

#include "game/charman.h"

#include "game/audio.h"

#include "game/gamemes.h"

#include "game/flag.h"

#include "include/game/audio.h"

#include "include/game/data.h"

#include "include/game/memory.h"

#include "include/datadir_enum.h"

#include "include/game/frand.h"

#include "include/string.h"

#include "include/game/gamework.h"

#include "include/game/gamemes.h"
#include "dolphin/gx.h"
#include "string.h"
#include "msm_stream.h"
#include "game/hu3d.h"
#include "game/main.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include <string.h>
#include "REL/m615DLL.h"

typedef struct M615EffectWork {
    OMOBJ *modelObject;
    u8 unknown_04[4]; /* No code names this field. */
    s16 particles[4];
    HuVecF position;
    f32 rotation;
    f32 scale[4];
    s32 curveIndex;
    HuVecF direction;
    HuVecF velocity;
    HuVecF updatedDirection;
} M615EffectWork;

typedef struct M615Camera M615Camera;

extern M615EffectWork *lbl_1_bss_2C[3];
extern s32 lbl_1_bss_8;
extern s32 lbl_1_bss_C[4];
extern u32 lbl_1_data_28[2][3];

typedef struct M615Bounds {
    HuVecF min;
    HuVecF max;
} M615Bounds;

typedef struct M615Segment {
    HuVecF start;
    HuVecF delta;
} M615Segment;

typedef struct M615Capsule {
    M615Segment segment;
    f32 radius;
} M615Capsule;

typedef struct M615Sphere {
    HuVecF center;
    f32 radius;
} M615Sphere;

typedef struct M615Triangle {
    HuVecF start;
    HuVecF edge1;
    HuVecF edge2;
} M615Triangle;

typedef struct M615CollisionData {
    s32 type;
    M615Triangle triangle;
    HuVecF normal;
    M615Bounds bounds;
} M615CollisionData;

typedef struct M615CollisionPool {
    s32 count;
    M615CollisionData **entries;
} M615CollisionPool;

extern s32 fn_1_1478(M615Bounds *a, M615Bounds *b);

extern f32 fn_1_14E8(HuVecF *queryPoint, M615Triangle *triangle, f32 *barycentricS, f32 *barycentricT);

extern f32 fn_1_2544(M615Segment *segment, M615Triangle *triangle, f32 *segmentTOut, f32 *triangleSOut, f32 *triangleTOut);

extern f32 fn_1_42CC(HuVecF *point, M615Triangle *triangle, f32 *barycentricS, f32 *barycentricT);

extern f32 fn_1_4C9C(M615Segment *segment, M615Triangle *triangle, f32 *segmentTOut, f32 *triangleSOut, f32 *triangleTOut);

extern void fn_1_A0C(M615Bounds *bounds, M615Capsule *capsule);

extern double fmod(double value, double divisor);

typedef struct M615MovementWork {
    MGPLAYER *player;
    s16 playerIndex;
    s16 character;
    s16 pathIndex;
    HuVecF previousPosition;
    HuVecF position;
    s32 reached[3];
    f32 approach[3];
    s32 started;
    s32 state;
    f32 targetX;
} M615MovementWork;

extern double sin(double angle);

extern double cos(double angle);

extern s32 lbl_1_bss_1C[4];

extern f32 fn_1_643C(HuVecF *queryPoint, M615Segment *lineSegment);

typedef struct M615RoundPlayerWork {
    s16 playerIndex;
    u16 unknown02;
    s16 teamIndex;
} M615RoundPlayerWork;

typedef struct M615SequenceWork {
    s32 frame;
} M615SequenceWork;

extern u32 lbl_1_bss_CC;

void fn_1_248(OMOBJ *object, OMOBJ_FUNC callback);

typedef struct M615StageAudioWork {
    s32 stageState;
    s16 streamHandle;
} M615StageAudioWork;

void fn_1_56A4(OMOBJ *object);

extern void fn_1_BC4(M615Bounds *bounds, M615Triangle *triangle);

extern void fn_1_E38(M615Bounds *bounds, M615Triangle *triangle);

extern double fmod(double value, double divisor);

extern double sin(double angle);

extern double cos(double angle);

static inline HuVecF targetPositionGet(M615EffectWork *target)
{
    return target->position;
}

extern f32 fn_1_643C(HuVecF *queryPoint, M615Segment *lineSegment);

extern HUPROCESS *lbl_1_bss_4;

extern s16 lbl_802C0278;

extern char lbl_1_data_354[16];

extern char lbl_1_data_364[16];

extern void fn_1_BC4(M615Bounds *bounds, M615Triangle *triangle);

extern void fn_1_E38(M615Bounds *bounds, M615Triangle *triangle);

typedef struct M615PositionAdjustment {
    Point3d position;
    Point3d adjustment;
} M615PositionAdjustment;

u32 lbl_1_data_28[2][3] = {
    { DATANUM(0x4a0000, 3), DATANUM(0x4a0000, 0), DATANUM(0x4a0000, 2) },
    { DATANUM(0x4a0000, 4), DATANUM(0x4a0000, 0), DATANUM(0x4a0000, 2) }
};

unsigned int lbl_1_data_40[0x11] = {
    DATANUM(0x930000, 0), DATANUM(0x930000, 1), DATANUM(0x930000, 2), DATANUM(0x930000, 3),
    DATANUM(0x930000, 4), DATANUM(0x930000, 5), DATANUM(0x930000, 56), DATANUM(0x930000, 8),
    DATANUM(0x930000, 10), DATANUM(0x930000, 9), DATANUM(0x930000, 15), DATANUM(0x930000, 16),
    DATANUM(0x930000, 17), DATANUM(0x930000, 18), DATANUM(0x930000, 6), DATANUM(0x930000, 7),
    0
};

HuVecF lbl_1_data_84[4] = {
    { -480.0f, 0.0f, 0.0f },
    { -215.0f, 0.0f, 0.0f },
    { 215.0f, 0.0f, 0.0f },
    { 480.0f, 0.0f, 0.0f }
};

HuVecF lbl_1_data_B4[4] = {
    { 55.0f, 59.0f, 0.0f },
    { 175.0f, 59.0f, 0.0f },
    { 401.0f, 59.0f, 0.0f },
    { 521.0f, 59.0f, 0.0f }
};

M615PositionAdjustment lbl_1_data_E4[0x1A] = {
    {{-936.65f, 85.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
    {{-936.65f, 255.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
    {{-936.65f, 425.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
    {{-936.65f, 595.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
    {{-936.65f, 765.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
    {{-936.65f, 935.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
    {{-936.65f, 1105.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}},
    {{-936.65f, 1275.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{-766.65f, 1275.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{-596.65f, 1275.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{-426.65f, 1275.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{-256.65f, 1275.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{-86.65f, 1275.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{83.35f, 1275.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{253.35f, 1275.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{423.35f, 1275.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{593.35f, 1275.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{763.35f, 1275.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{936.65f, 1275.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{936.65f, 1105.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{936.65f, 935.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{936.65f, 765.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{936.65f, 595.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{936.65f, 425.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{936.65f, 255.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{936.65f, 85.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
};

char lbl_1_data_354[16] = "6xx_tatake-colA";

char lbl_1_data_364[16] = "6xx_tatake-colC";

typedef struct M615Box {
    HuVecF center;
    HuVecF axis[3];
    f32 halfExtents[3];
} M615Box;

typedef struct M615ModelSetup {
    s16 modelId;
    M615Box box;
    M615Bounds bounds;
    f32 blend;
} M615ModelSetup;

extern void fn_1_109C(M615Bounds *bounds, const M615Box *box);

extern s32 fn_1_1478(M615Bounds *a, M615Bounds *b);

extern void fn_1_A0C(M615Bounds *bounds, M615Capsule *capsule);

extern f32 fn_1_8D08(const M615Segment *line, const M615Box *box, f32 *time,
                     f32 *closestX, f32 *closestY, f32 *closestZ);

typedef struct M615CollisionPoint {
    Point3d start;
    f32 radius;
} M615CollisionPoint;

extern f32 fn_1_181C(M615Segment *firstSegment, M615Segment *secondSegment, f32 *firstParameter, f32 *secondParameter);

extern f32 fn_1_181C(M615Segment *firstSegment,
                      M615Segment *secondSegment,
                      f32 *firstParameter, f32 *secondParameter);

typedef struct M615CameraWork {
    HU3D_MODELID cameraModelId;
    u16 unknown02;
    Vec position;
    Vec up;
    Vec target;
} M615CameraWork;

typedef struct M615ScoreWork {
    s16 playerIndex;
    s16 character;
    s16 positionIndex;
    s16 scoreBoxGroup;
    MGSCORE *score;
} M615ScoreWork;

extern M615CollisionPool *lbl_1_bss_48;

extern void fn_1_9140(M615CollisionPool **poolResult, s32 count);
extern M615CollisionData *fn_1_9228(M615CollisionPool *pool, s32 index);
extern M615CollisionData *fn_1_9238(M615CollisionPool *pool, M615Capsule *capsule);

extern void fn_1_444(HuVecF *out, f32 x, f32 y, f32 z);
extern f32 fn_1_574(const HuVecF *v);
extern f32 fn_1_5A4(const HuVecF *a, const HuVecF *b);
extern void fn_1_60C(HuVecF *dst, const HuVecF *a, const HuVecF *b);
extern void fn_1_644(HuVecF *dst, const HuVecF *a, const HuVecF *b);
extern f32 fn_1_67C(HuVecF *dst, const HuVecF *src);
extern void fn_1_85C(HuVecF *dst, const HuVecF *src, f32 scale);

extern M615MovementWork *lbl_1_bss_38[4];

M615MovementWork *fn_1_9500(s32 playerIndex);
M615EffectWork *fn_1_9518(s32 index);
void fn_1_9530(s32 *groups);
void fn_1_9620(OMOBJ *obj);
void fn_1_9884(OMOBJ *obj);
void fn_1_98E4(OMOBJ *object);
void fn_1_9948(OMOBJ *object);
void fn_1_99D0(OMOBJ *object);
void fn_1_9C28(OMOBJ *obj);
void fn_1_9CDC(OMOBJ *obj);
s32 fn_1_9D18(HSF_OBJECT *object);
void fn_1_9D3C(HSF_OBJECT *object, s32 faceIndex, HuVecF *triangle);
void fn_1_9E04(M615CollisionPool *pool, HSF_OBJECT *object, s32 faceIndex, s32 slot);
void fn_1_9FCC(HSF_OBJECT *object, s32 faceIndex, HuVecF *normal);
void fn_1_A010(OMOBJ *object);
void fn_1_A454(OMOBJ *object);
void fn_1_A468(OMOBJ *object);
void fn_1_A6A4(OMOBJ *object);
void fn_1_A9C8(OMOBJ *object);
void fn_1_AB58(OMOBJ *object);
void fn_1_ABF4(OMOBJ *object);
void fn_1_AF3C(void *callback, HU3D_PARTICLE *work);
void fn_1_AFB0(void *particle, HU3D_PARTICLE *work);
void fn_1_B094(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_B4F8(HU3D_MODEL *model, HU3D_PARTICLE *particle, void *matrix);
void fn_1_B71C(OMOBJ *object);
void fn_1_BCE8(OMOBJ *object);
void fn_1_C9DC(OMOBJ *obj);
void fn_1_CAB0(OMOBJ *object);
void fn_1_CAC4(OMOBJ *obj);
void fn_1_CB3C(OMOBJ *obj);
void fn_1_CBD0(OMOBJ *obj);
void fn_1_D118(OMOBJ *object);
void fn_1_D270(OMOBJ *object);
void fn_1_D2D8(OMOBJ *object);
void fn_1_D32C(f32 separation);
void fn_1_D708(OMOBJ *object);
void fn_1_D748(OMOBJ *object);
void fn_1_D7A8(OMOBJ *object);
void fn_1_D7E8(M615MovementWork *work, s32 motionSlot, f32 endTime, s32 attr);
void fn_1_D864(M615MovementWork *work);
void fn_1_DA90(OMOBJ *object);
void fn_1_DD08(OMOBJ *object);
void fn_1_DDA0(OMOBJ *object);
void fn_1_E02C(OMOBJ *object);
void fn_1_E358(OMOBJ *object);
void fn_1_E6BC(OMOBJ *object);
void fn_1_E8EC(OMOBJ *object);
void fn_1_EC50(OMOBJ *object);
void fn_1_EFD4(OMOBJ *object);
void fn_1_F058(OMOBJ *object);
s32 fn_1_F170(M615MovementWork *work, s32 *direction);
s32 fn_1_F2D4(M615MovementWork *work, s32 *direction);
s32 fn_1_F554(M615MovementWork *work, s32 *direction);
s32 fn_1_F938(M615MovementWork *work, s32 *direction);

const GXColor lbl_1_rodata_B8[14] = {
    { 254, 0, 0, 255 },
    { 1, 0, 254, 255 },
    { 237, 0, 140, 255 },
    { 0, 255, 1, 255 },
    { 125, 0, 190, 255 },
    { 255, 255, 0, 255 },
    { 80, 80, 80, 255 },
    { 255, 255, 255, 255 },
    { 1, 255, 255, 255 },
    { 0, 166, 82, 255 },
    { 255, 81, 160, 255 },
    { 255, 96, 22, 255 },
    { 255, 96, 22, 255 },
    { 255, 96, 22, 255 }
};

const HuVecF lbl_1_rodata_F0[3] = {
    { 0.0f, 420.0f, 0.0f },
    { 0.0f, 700.0f, 0.0f },
    { 0.0f, 980.0f, 0.0f }
};

s32 (*const lbl_1_rodata_114[4])(M615MovementWork *work, s32 *button) = {
    fn_1_F170, fn_1_F2D4, fn_1_F554, fn_1_F938
};

M615MovementWork *fn_1_9500(s32 playerIndex)
{
    return lbl_1_bss_38[playerIndex];
}

M615EffectWork *fn_1_9518(s32 index)
{
    return lbl_1_bss_2C[index];
}

void fn_1_9530(s32 *groups)
{
    s32 teamOrder[4] = { -1, -1, -1, -1 };
    s32 player;

    for (player = 0; player < 4; player++) {
        teamOrder[lbl_80265728[player].grpNo] = player;
    }
    if ((teamOrder[0] == -1) || (teamOrder[1] == -1) ||
        (teamOrder[2] == -1) || (teamOrder[3] == -1)) {
        for (player = 0; player < 4; player++) {
            groups[player] = player;
        }
    } else {
        for (player = 0; player < 4; player++) {
            groups[player] = lbl_80265728[player].grpNo;
        }
    }
}

void fn_1_9620(OMOBJ *obj)
{
    M615StageAudioWork *work = obj->data;
    s32 winners[4];
    s32 setupPlayer;

    work->streamHandle = -1;
    fn_1_9530(winners);
    fn_1_2F0(0xA, 4, fn_1_A010);
    fn_1_2F0(0xC8, 0x28, fn_1_CAC4);
    fn_1_2F0(0x14, 4, fn_1_AB58);
    fn_1_2F0(0x1E, 0x958, fn_1_A468);
    for (setupPlayer = 0; setupPlayer < 4; setupPlayer++) {
        lbl_1_bss_38[setupPlayer] = fn_1_2F0(0x1E, 0x48, fn_1_DA90);
        lbl_1_bss_38[setupPlayer]->playerIndex = setupPlayer;
        lbl_1_bss_38[setupPlayer]->pathIndex = winners[setupPlayer];
    }
    fn_1_2F0(0x28, 4, fn_1_D708);
    for (setupPlayer = 0; setupPlayer < 4; setupPlayer++) {
        M615RoundPlayerWork *roundWork = fn_1_2F0(0x14, 0x10, fn_1_D118);
        roundWork->playerIndex = setupPlayer;
        roundWork->teamIndex = winners[setupPlayer];
    }
    fn_8005DDEC(5);
    fn_1_390(obj, fn_1_9884);
}

void fn_1_9884(OMOBJ *obj)
{
    s32 *timer = obj->data;

    if (fn_80071A8C() == 2) {
        fn_80071B08();
        *timer = 0;
        fn_1_390(obj, fn_1_98E4);
        return;
    }
}

void fn_1_98E4(OMOBJ *object)
{
    M615SequenceWork *work = object->data;

    if (work->frame++ >= 0x78) {
        fn_80071A9C();
        fn_80071B30();
        fn_1_390(object, fn_1_9948);
        return;
    }
}

void fn_1_9948(OMOBJ *object)
{
    M615StageAudioWork *work = object->data;

    if (work->streamHandle == -1 &&
        (GameMesStatGet(fn_80071DF4()) & 0x10) != 0) {
        work->streamHandle = HuAudSStreamPlay(MSM_STREAM_JAZZY);
    }
    if (fn_80071A8C() == 5) {
        fn_1_390(object, fn_1_99D0);
        return;
    }
}

void fn_1_99D0(OMOBJ *object)
{
    M615StageAudioWork *work = object->data;

    if (fn_80071A8C() == 6) {
        s32 highestScore = lbl_1_bss_1C[0];
        s32 winnerValues[4] = { -1, -1, -1, -1 };
        s32 winnerCount;
        s32 winnerStatus;
        M615MovementWork *winner;
        s32 playerScore;
        s32 playerIndex;

        winnerCount = 0;
        for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            if (highestScore < lbl_1_bss_1C[playerIndex]) {
                highestScore = lbl_1_bss_1C[playerIndex];
            }
        }
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (highestScore == lbl_1_bss_1C[playerIndex]) {
                winnerStatus = 1;
            } else {
                winnerStatus = -1;
            }
            lbl_1_bss_C[playerIndex] = winnerStatus;
            if (highestScore == 0) {
                lbl_1_bss_C[playerIndex] = 0;
            }
        }
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (lbl_1_bss_C[playerIndex] == 1) {
                winner = lbl_1_bss_38[playerIndex];
                winnerValues[winnerCount++] = winner->character;
        if (fn_8006A124(65551) == 0) {
                    lbl_80265750[playerIndex].mgCoinBonus = 10;
                }
            }
        }
        if (fn_8006A124(0x30002U)) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                playerScore = lbl_1_bss_1C[playerIndex];
                lbl_80265750[playerIndex].mgScore = playerScore;
            }
        } else {
            fn_80071C18((s16)winnerValues[0],
                           (s16)winnerValues[1],
                           (s16)winnerValues[2],
                           (s16)winnerValues[3]);
        }
        if (work->streamHandle != -1) {
            HuAudSStreamFadeOut(work->streamHandle, 0x64);
        }
        fn_1_390(object, fn_1_9C28);
        return;
    }
}

void fn_1_9C28(OMOBJ *obj)
{
    M615StageAudioWork *work = obj->data;
    if (fn_8006A124(0x30002U) != 0 && fn_80071A8C() != 6) {
        fn_80071AC4(9);
        fn_1_390(obj, NULL);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_390(obj, fn_1_9CDC);
        return;
    }
    if (lbl_1_bss_8 == 5) {
        fn_80071A9C();
        fn_1_390(obj, fn_1_9CDC);
        return;
    }
}

void fn_1_9CDC(OMOBJ *obj)
{
    M615Camera *work = obj->data;
    fn_1_390(obj, NULL);
}

s32 fn_1_9D18(HSF_OBJECT *object)
{
    HSF_BUFFER *vertexBuffer = object->mesh.vertex;
    HSF_BUFFER *faceBuffer = object->mesh.face;

    return faceBuffer->count;
}

void fn_1_9D3C(HSF_OBJECT *object, s32 faceIndex, HuVecF *triangle)
{
    HSF_BUFFER *vertexBuffer;
    HSF_BUFFER *faceBuffer;
    HSF_FACE *face;

    vertexBuffer = object->mesh.vertex;
    faceBuffer = object->mesh.face;
    face = &faceBuffer->faces[faceIndex];
    *triangle = vertexBuffer->positions[face->index[0].vertex];
    fn_1_644(&triangle[1], &vertexBuffer->positions[face->index[1].vertex],
             &vertexBuffer->positions[face->index[0].vertex]);
    fn_1_644(&triangle[2], &vertexBuffer->positions[face->index[2].vertex],
             &vertexBuffer->positions[face->index[0].vertex]);
}

void fn_1_9E04(M615CollisionPool *pool, HSF_OBJECT *object, s32 faceIndex, s32 slot)
{
    HSF_BUFFER *vertexBuffer;
    HSF_BUFFER *faceBuffer;
    HSF_FACE *face;
    M615CollisionData *result;

    vertexBuffer = object->mesh.vertex;
    faceBuffer = object->mesh.face;
    face = &faceBuffer->faces[faceIndex];
    result = fn_1_9228(pool, slot);

    if (face->type == 3) {
        result->type = 0;
        result->triangle.start = vertexBuffer->positions[face->index[0].vertex];
        fn_1_644(&result->triangle.edge1,
                 &vertexBuffer->positions[face->index[1].vertex],
                 &vertexBuffer->positions[face->index[0].vertex]);
        fn_1_644(&result->triangle.edge2,
                 &vertexBuffer->positions[face->index[2].vertex],
                 &vertexBuffer->positions[face->index[0].vertex]);
        result->normal = face->normal;
        fn_1_BC4(&result->bounds, &result->triangle);
        return;
    }
    if (face->type == 2) {
        result->type = 1;
        result->triangle.start = vertexBuffer->positions[face->index[0].vertex];
        fn_1_644(&result->triangle.edge1,
                 &vertexBuffer->positions[face->index[1].vertex],
                 &vertexBuffer->positions[face->index[0].vertex]);
        fn_1_644(&result->triangle.edge2,
                 &vertexBuffer->positions[face->index[2].vertex],
                 &vertexBuffer->positions[face->index[0].vertex]);
        result->normal = face->normal;
        fn_1_E38(&result->bounds, &result->triangle);
    }
}

void fn_1_9FCC(HSF_OBJECT *object, s32 faceIndex, HuVecF *normal)
{
    HSF_BUFFER *faceBuffer = object->mesh.face;
    HSF_FACE *face = &faceBuffer->faces[faceIndex];

    *normal = face->normal;
}

void fn_1_A010(OMOBJ *object)
{
    s32 triangleIndex;
    s32 collisionSlot;
    s32 collisionObjectIndex;
    s16 nightMode;
    s32 totalFaceCount;
    s32 collisionIndex;
    OMOBJ * * childObjectSlot;
    s32 modelIndex;
    childObjectSlot = object->data;
    * childObjectSlot = fn_8003AB38(lbl_1_bss_4, 0x7FDA, 3, 0, -1, NULL);
    for (modelIndex = 0; modelIndex < 3; modelIndex++)
    {
        nightMode = lbl_802C0278;
        (* childObjectSlot)->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_data_28[nightMode == 0 ? 0 : 1][modelIndex], 0x10000000, 2));
    }
    fn_80026B8C((* childObjectSlot)->mdlId[0], 1);
    collisionSlot = fn_80029850((* childObjectSlot)->mdlId[0], 1);
    fn_80026FFC((* childObjectSlot)->mdlId[0]);
    fn_800269E8((* childObjectSlot)->mdlId[1], 1);
    fn_800269E8((* childObjectSlot)->mdlId[2], 1);
    fn_800804E8(& (* childObjectSlot)->mdlId[1], 2, 0x80);
    collisionIndex = 0;
    {
        HSF_OBJECT * collisionObjects[3] = { 0 };
        collisionObjects[0] = fn_80026BF4((* childObjectSlot)->mdlId[1], lbl_1_data_354);
        collisionObjects[2] = fn_80026BF4((* childObjectSlot)->mdlId[2], lbl_1_data_364);
        totalFaceCount = fn_1_9D18(collisionObjects[0]) + fn_1_9D18(collisionObjects[2]);
        fn_1_9140(&lbl_1_bss_48, totalFaceCount);
        for (collisionObjectIndex = 0; 3 > collisionObjectIndex; collisionObjectIndex += 1)
        {
            if (collisionObjects[collisionObjectIndex])
            {
                for (triangleIndex = 0; triangleIndex < fn_1_9D18(collisionObjects[collisionObjectIndex]); triangleIndex += 1)
                {
                    fn_1_9E04(lbl_1_bss_48, collisionObjects[collisionObjectIndex], triangleIndex, collisionIndex);
                    collisionIndex++;
                }
            }
        }
    }
    fn_1_390(object, fn_1_A454);
}

void fn_1_A454(OMOBJ *object)
{
    void *work = object->data;
}

void fn_1_A468(OMOBJ *object)
{
    M615ModelSetup *models = object->data;
    s16 nightMode = lbl_802C0278;
    s16 modelId;
    u32 modelData;
    s32 modelIndex;

    if (nightMode == 0) {
        modelData = DATANUM(0x4a0000, 6);
    } else {
        modelData = DATANUM(0x4a0000, 7);
    }
    modelId = fn_80025A04(fn_80008DD0(modelData, 0x10000000, 2));
    models[0].modelId = modelId;

    for (modelIndex = 1; modelIndex < 0x1A; modelIndex++) {
        models[modelIndex].modelId = fn_80025D4C(modelId);
    }

    for (modelIndex = 0; modelIndex < 0x1A; modelIndex++) {
        fn_80026B8C(models[modelIndex].modelId, 5);
        fn_80026790(models[modelIndex].modelId, &lbl_1_data_E4[modelIndex].position);
        models[modelIndex].box.center = lbl_1_data_E4[modelIndex].position;
        fn_1_444(&models[modelIndex].box.axis[0], 1.0f, 0.0f, 0.0f);
        fn_1_444(&models[modelIndex].box.axis[1], 0.0f, 1.0f, 0.0f);
        fn_1_444(&models[modelIndex].box.axis[2], 0.0f, 0.0f, 1.0f);
        models[modelIndex].box.halfExtents[0] = 87.0f;
        models[modelIndex].box.halfExtents[1] = 84.0f;
        models[modelIndex].box.halfExtents[2] = 190.0f;
        fn_1_109C(&models[modelIndex].bounds, &models[modelIndex].box);
        models[modelIndex].blend = 1.0f;
    }

    fn_1_390(object, fn_1_A6A4);
}

void fn_1_A6A4(OMOBJ *object)
{
    M615ModelSetup *models = object->data;
    M615EffectWork *collisionObject;
    M615Capsule capsule;
    M615Bounds collisionBounds;
    Point3d position;
    f32 angleValue;
    f32 cosine;
    s32 objectIndex;
    s32 modelIndex;

    for (objectIndex = 0; objectIndex < 3; objectIndex++) {
        collisionObject = fn_1_9518(objectIndex);
        if (fn_1_574(&collisionObject->direction) == 0.0f) {
            cosine = (f32)cos((f64)collisionObject->rotation);
            fn_1_444(&capsule.segment.delta,
                     (f32)sin((f64)collisionObject->rotation), cosine, 0.0f);
        } else {
            capsule.segment.delta = collisionObject->direction;
        }
        fn_1_85C(&capsule.segment.delta, &capsule.segment.delta, collisionObject->scale[0]);
        fn_1_60C(&capsule.segment.delta, &capsule.segment.delta, &collisionObject->velocity);
        capsule.segment.start = collisionObject->position;
        capsule.radius = 100.0f;
        fn_1_A0C(&collisionBounds, &capsule);

        for (modelIndex = 0; modelIndex < 0x1A; modelIndex++) {
            if (fn_1_1478(&collisionBounds,
                          &models[modelIndex].bounds) != 0 &&
                fn_1_8D08(&capsule.segment,
                          &models[modelIndex].box,
                          NULL, NULL, NULL, NULL) <= capsule.radius * capsule.radius) {
                models[modelIndex].blend = 0.0f;
            }
        }
    }

    for (modelIndex = 0; modelIndex < 0x1A; modelIndex++) {
        models[modelIndex].blend +=
            (1.0f - models[modelIndex].blend) / 5.0f;
        angleValue = 1.0f - models[modelIndex].blend;
        position = lbl_1_data_E4[modelIndex].position;
        position.x += angleValue * (100.0f * lbl_1_data_E4[modelIndex].adjustment.x);
        position.y += angleValue * (100.0f * lbl_1_data_E4[modelIndex].adjustment.y);
        position.z += angleValue * (100.0f * lbl_1_data_E4[modelIndex].adjustment.z);
        fn_80026790(models[modelIndex].modelId, &position);
    }

    if ((u32)fn_80071A8C() == 6) {
        fn_1_390(object, fn_1_A9C8);
        return;
    }
}

void fn_1_A9C8(OMOBJ *object)
{
    M615ModelSetup *models = object->data;
    Point3d position;
    f32 remaining;
    s32 modelIndex;

    for (modelIndex = 0; modelIndex < 0x1A; modelIndex++) {
        models[modelIndex].blend +=
            (1.0f - models[modelIndex].blend) / 10.0f;
        remaining = 1.0f - models[modelIndex].blend;
        position = lbl_1_data_E4[modelIndex].position;
        position.x += remaining * (50.0f * lbl_1_data_E4[modelIndex].adjustment.x);
        position.y += remaining * (50.0f * lbl_1_data_E4[modelIndex].adjustment.y);
        position.z += remaining * (50.0f * lbl_1_data_E4[modelIndex].adjustment.z);
        fn_80026790(models[modelIndex].modelId, &position);
    }
}

/* Creates the three effect objects, numbering their curves, then continues in fn_1_ABF4. */
void fn_1_AB58(OMOBJ *object) {
    void *data;
    s32 index;

    data = object->data;
    index = 0;
    while (index < 3) {
        lbl_1_bss_2C[index] = fn_1_2F0(0xA, 0x58, fn_1_B71C);
        lbl_1_bss_2C[index]->curveIndex = index;
        index += 1;
    }
    fn_1_390(object, fn_1_ABF4);
}

void fn_1_ABF4(OMOBJ *object)
{
    void *workData = object->data;
    Point3d primaryDirection;
    M615CollisionPoint primaryPoint;
    M615CollisionPoint secondaryPoint;
    Point3d separation;
    Point3d normalizedSeparation;
    Point3d secondaryDirection;
    Point3d displacement;
    f32 overlap;
    f32 distance;
    f32 strength;
    M615EffectWork *primary;
    M615EffectWork *secondary;
    s32 primaryIndex;
    s32 secondaryIndex;

    primaryIndex = 0;
    while (primaryIndex < 3) {
        primary = fn_1_9518(primaryIndex);
        primaryPoint.start = primary->position;
        primaryPoint.radius = 100.0f;
        fn_1_444(&primary->velocity, 0.0f,
                 0.0f, 0.0f);
        fn_1_444(&primary->direction, 0.0f,
                 0.0f, 0.0f);
        fn_1_444(&primaryDirection,
                 (f32)sin((f64)primary->rotation),
                 (f32)cos((f64)primary->rotation), 0.0f);

        secondaryIndex = 0;
        while (secondaryIndex < 3) {
            if (primaryIndex != secondaryIndex) {
                secondary = fn_1_9518(secondaryIndex);
                secondaryPoint.start = secondary->position;
                secondaryPoint.radius = 100.0f;
                fn_1_644(&separation, &primaryPoint.start,
                         &secondaryPoint.start);
                distance = fn_1_67C(&normalizedSeparation, &separation);
                if (distance < primaryPoint.radius + secondaryPoint.radius) {
                    overlap = (primaryPoint.radius + secondaryPoint.radius) - distance;
                    fn_1_444(&secondaryDirection,
                             (f32)sin((f64)secondary->rotation),
                             (f32)cos((f64)secondary->rotation),
                             0.0f);
                    fn_1_85C(&displacement, &normalizedSeparation, overlap);
                    fn_1_60C(&primary->velocity, &primary->velocity,
                             &displacement);
                    fn_1_85C(&primary->velocity, &primary->velocity,
                             0.5f);
                    strength = fabsf(fn_1_5A4(
                        &primaryDirection, &normalizedSeparation) *
                        2.0f);
                    primary->direction.x += primaryDirection.x +
                                           (normalizedSeparation.x * strength);
                    primary->direction.y += primaryDirection.y +
                                           (normalizedSeparation.y * strength);
                    primary->direction.z += primaryDirection.z +
                                           (normalizedSeparation.z * strength);
                    fn_1_60C(&primary->direction, &primary->direction,
                             &secondaryDirection);
                    fn_1_67C(&primary->direction, &primary->direction);
                    primary->scale[1] = 0.8f;
                }
            }
            secondaryIndex++;
        }
        primaryIndex++;
    }
}

void fn_1_AF3C(void *callback, HU3D_PARTICLE *work)
{
    HU3D_PARTICLE_DATA *entry = work->data;
    s32 entryIndex = 0;

    while (entryIndex < work->maxCnt) {
        if (entry->time == 0) {
            entry->time++;
            entry->color.r = 0xFF;
            entry->color.g = 0;
            entry->color.b = 0xFF;
            entry->color.a = 0xC0;
        }
        entryIndex++;
        entry++;
    }
}

void fn_1_AFB0(void *particle, HU3D_PARTICLE *work)
{
    HU3D_PARTICLE_DATA *record = work->data;
    s32 index;
    s32 fade;

    for (index = 0; index < work->maxCnt; index++, record++) {
        if (record->time == 0) {
            record->time++;
            record->color.r = 0x40;
            record->color.g = 0x40;
            record->color.b = 0x40;
            record->color.a = 0x40;
            record->scale = 240.00002f;
        }
        if (record->time == 1) {
            fade = record->color.a - 2;
            if (fade <= 0) {
                fade = 0;
                record->scale = 0.0f;
                record->time = -1;
            }
            record->color.a = fade;
            record->color.r = fade;
            record->color.g = fade;
            record->color.b = fade;
        }
    }
}

void fn_1_B094(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    f32 angle;
    f32 radius;
    s32 particleIndex;
    s32 colorIndex;
    s32 remainingAlpha;

    data = particle->data;
    particleIndex = 0;
    while (particleIndex < particle->maxCnt) {
        if (data->time == 0) {
            angle = frandf() * 2.0f;
            radius = frandf();
            colorIndex = rand8() % 14;
            data->time++;
            data->parManId = rand8() % 32;
            data->vel.x = (f32)(radius * sin((f64)(M_PI * angle)) * 8.0);
            data->vel.y = (f32)(radius * cos((f64)(M_PI * angle)) * 8.0);
            data->vel.z = 0.0f;
            data->pos.x = (f32)((f64)data->pos.x +
                100.0 *
                (radius * sin((f64)(M_PI * angle))));
            data->pos.y = (f32)((f64)data->pos.y +
                100.0 *
                (radius * cos((f64)(M_PI * angle))));
            data->color.r = lbl_1_rodata_B8[colorIndex].r;
            data->color.g = lbl_1_rodata_B8[colorIndex].g;
            data->color.b = lbl_1_rodata_B8[colorIndex].b;
            data->color.a = 0x80;
            data->scale = (f32)(64.0 + (f64)(rand8() % 64));
        }
        if (data->time == 1) {
            remainingAlpha = data->color.a - 1;
            data->pos.y += data->vel.y;
            data->vel.y -= 0.1f;
            data->parManId++;
            if (data->pos.y < 0.5f * data->scale) {
                data->pos.y = 0.5f * data->scale;
                data->vel.y = 0.5f * -data->vel.y;
            } else if (data->pos.y > 0.0f) {
                data->pos.x += data->vel.x;
            }
            if (data->pos.y > 1200.0f) {
                data->pos.y = 1200.0f;
            }
            if (data->pos.x < (-850.0f) + 0.5f * data->scale) {
                data->pos.x = -850.0f + 0.5f * data->scale;
                data->vel.x = -data->vel.x;
            } else if (data->pos.x > 850.0f - 0.5f * data->scale) {
                data->pos.x = 850.0f - 0.5f * data->scale;
                data->vel.x = -data->vel.x;
            }
            if (remainingAlpha <= 0) {
                remainingAlpha = 0;
                data->scale = 0.0f;
                data->time = -1;
            }
            data->color.a = remainingAlpha;
        }
        particleIndex++;
        data++;
    }
}

void fn_1_B4F8(HU3D_MODEL *model, HU3D_PARTICLE *particle, void *matrix)
{
    HU3D_PARTICLE_DATA *data = particle->data;
    f32 speed;
    s32 alpha;
    s32 particleIndex;

    particleIndex = 0;
    while (particleIndex < particle->maxCnt) {
        if (data->time == 0) {
            data->time += 1;
            data->color.a = 0xFF;
            data->scale = 160.0f;
            data->pos.z = 150.0f;
        }
        if (data->time == 1) {
            alpha = data->color.a;
            fn_1_60C(&data->pos, &data->pos, &data->vel);
            speed = fn_1_67C(&data->vel, &data->vel);
            speed += -speed / 10.0f;
            fn_1_85C(&data->vel, &data->vel, speed);

            if (data->pos.x < -800.0f) {
                data->pos.x = -800.0f;
                data->vel.x = -data->vel.x;
            } else if (data->pos.x > 800.0f) {
                data->pos.x = 800.0f;
                data->vel.x = -data->vel.x;
            }
            if (data->pos.y < 50.0f) {
                data->pos.y = 50.0f;
                data->vel.y = -data->vel.y;
            } else if (data->pos.y > 1150.0f) {
                data->pos.y = 1150.0f;
                data->vel.y = -data->vel.y;
            }

            if (speed < 0.1f) {
                alpha -= 0x10;
            }
            if (alpha <= 0) {
                alpha = 0;
                data->scale = 0.0f;
                data->time = -1;
            }
            data->color.a = alpha;
        }
        particleIndex++;
        data++;
    }
}

void fn_1_B71C(OMOBJ *object)
{
    M615EffectWork *work = object->data;
    HU3D_PARTICLE *firstEmitter;
    HU3D_PARTICLE *secondEmitter;
    HU3D_PARTICLE *thirdEmitter;
    HU3D_PARTICLE *fourthEmitter;
    HU3D_PARTICLE_DATA *particle;
    s16 firstParticleId;
    s16 secondParticleId;
    s16 thirdParticleId;
    s16 fourthParticleId;
    s32 firstParticleIndex;
    s32 secondParticleIndex;
    s32 thirdParticleIndex;
    s32 fourthParticleIndex;

    work->modelObject = fn_8003AB38(lbl_1_bss_4, 0x28, 2, 0, -1, NULL);
    work->modelObject->mdlId[0] = fn_80025A04(
        fn_80008DD0(DATANUM(0x4a0000, 5), 0x10000000, 2));
    omSetStatBit(work->modelObject, 0x100);
    fn_80026B8C(work->modelObject->mdlId[0], 5);
    work->modelObject->mdlId[1] = fn_80036DF8(
        fn_80008DD0(DATANUM(0x4a0000, 8), 0x10000000, 2));
    fn_80026B8C(work->modelObject->mdlId[1], 5);
    fn_800269E8(work->modelObject->mdlId[1], 1);
    fn_800269E8(work->modelObject->mdlId[1], 0x40000002);
    fn_80026E44(work->modelObject->mdlId[0]);

    work->position = lbl_1_rodata_F0[work->curveIndex % 3];
    work->position.x = (1500.0f * frandf()) - 750.0f;
    fn_80026790(work->modelObject->mdlId[0], &work->position);
    work->rotation = (f32)(M_PI * 90.0 / 180.0 +
        (M_PI * (f64)(rand8() % 2)));
    work->scale[0] = 12.0f;
    work->scale[1] = 1.0f;
    work->scale[2] = 1.0f;
    work->scale[3] = 1.0f;

    work->particles[3] = fn_8003216C(
        HuSprAnimRead(fn_80008634(DATANUM(0x4a0000, 12), 0x10000000)), 1);
    firstParticleId = work->particles[3];
    firstEmitter = lbl_802C01C8[firstParticleId].hookData;
    fn_8003269C(firstParticleId, 0);
    fn_800326D0(firstParticleId, (HU3D_PARTICLE_HOOK)fn_1_AF3C);
    fn_80026B8C(firstParticleId, 6);
    fn_80026760(firstParticleId, 0.0f, 0.0f, 0.0f);
    particle = firstEmitter->data;
    for (firstParticleIndex = 0; firstParticleIndex < firstEmitter->maxCnt;
         firstParticleIndex++, particle++) {
        particle->time = -1;
        particle->scale = 0.0f;
    }

    work->particles[0] = fn_8003216C(
        HuSprAnimRead(fn_80008634(DATANUM(0x4a0000, 10), 0x10000000)), 0x40);
    secondParticleId = work->particles[0];
    secondEmitter = lbl_802C01C8[secondParticleId].hookData;
    fn_8003269C(secondParticleId, 1);
    fn_800326D0(secondParticleId, (HU3D_PARTICLE_HOOK)fn_1_AFB0);
    fn_80026B8C(secondParticleId, 6);
    fn_80026760(secondParticleId, 0.0f, 0.0f, 0.0f);
    for (secondParticleIndex = 0; secondParticleIndex < secondEmitter->maxCnt;
         secondParticleIndex++) {
        memset(&secondEmitter->data[secondParticleIndex], 0, 0x48);
        secondEmitter->data[secondParticleIndex].time = -1;
        secondEmitter->data[secondParticleIndex].cameraBit = 1;
    }

    work->particles[1] = fn_8003216C(
        HuSprAnimRead(fn_80008634(DATANUM(0x4a0000, 11), 0x10000000)), 0xFF);
    thirdParticleId = work->particles[1];
    thirdEmitter = lbl_802C01C8[thirdParticleId].hookData;
    fn_8003269C(thirdParticleId, 1);
    fn_800326D0(thirdParticleId, (HU3D_PARTICLE_HOOK)fn_1_B094);
    fn_80026B8C(thirdParticleId, 6);
    fn_80026760(thirdParticleId, 0.0f, 0.0f, 0.0f);
    for (thirdParticleIndex = 0; thirdParticleIndex < thirdEmitter->maxCnt;
         thirdParticleIndex++) {
        memset(&thirdEmitter->data[thirdParticleIndex], 0, 0x48);
        thirdEmitter->data[thirdParticleIndex].time = -1;
        thirdEmitter->data[thirdParticleIndex].cameraBit = 1;
    }

    work->particles[2] = fn_8003216C(
        HuSprAnimRead(fn_80008634(DATANUM(0x4a0000, 11), 0x10000000)), 0x40);
    fourthParticleId = work->particles[2];
    fourthEmitter = lbl_802C01C8[fourthParticleId].hookData;
    fn_8003269C(fourthParticleId, 0);
    fn_800326D0(fourthParticleId, (HU3D_PARTICLE_HOOK)fn_1_B4F8);
    fn_80026B8C(fourthParticleId, 6);
    fn_80026760(fourthParticleId, 0.0f, 0.0f, 0.0f);
    for (fourthParticleIndex = 0; fourthParticleIndex < fourthEmitter->maxCnt;
         fourthParticleIndex++) {
        memset(&fourthEmitter->data[fourthParticleIndex], 0, 0x48);
        fourthEmitter->data[fourthParticleIndex].time = -1;
        fourthEmitter->data[fourthParticleIndex].cameraBit = 1;
    }
    fn_1_390(object, fn_1_BCE8);
}

const HuVecF lbl_1_rodata_1EC = { 0.0f, 1.0f, 0.0f };

void fn_1_BCE8(OMOBJ *object)
{
    s32 particleIndex;
    M615EffectWork *work = object->data;
    HuVecF facing;
    HuVecF beamDirection;
    HuVecF particleDirection;
    M615Capsule effectCapsule;
    M615Capsule playerCapsule;
    HuVecF playerDirection;
    HuVecF closestPoint;
    HuVecF separation;
    HuVecF responseDirection;
    HuVecF correction;
    HuVecF oppositeAxis;
    HuVecF inwardDirection;
    HuVecF targetAxis;
    M615Capsule capsule;
    M615CollisionData *collision;
    f32 alignmentX;
    f32 alignmentY;
    f32 absoluteDot;
    f32 pan;
    f32 scaleX;
    f32 scaleY;
    f32 closestDistance;
    f32 playerCollisionRadius;
    f32 effectCollisionRadius;
    s32 playerIndex;
    s16 hitParticleId;
    s16 trailParticleId;
    f32 collisionPan;

    work->scale[1] = work->scale[1] +
        ((1.0f - work->scale[1]) / 8.0f);
    work->scale[2] = work->scale[2] +
        ((1.0f - work->scale[2]) / 5.0f);
    work->scale[3] = work->scale[3] +
        ((1.0f - work->scale[3]) / 5.0f);

    if (fn_1_574(&work->direction) == 0.0f) {
        facing.x = sin(work->rotation);
        facing.y = cos(work->rotation);
        facing.z = 0.0f;
    } else {
        facing = work->direction;
    }
    fn_1_85C(&facing, &facing, work->scale[0]);
    beamDirection = facing;
    particleDirection = facing;
    fn_1_60C(&facing, &facing, &work->velocity);

    effectCapsule.segment.start = work->position;
    effectCapsule.segment.delta = facing;
    effectCapsule.radius = 100.0f;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        M615MovementWork *target = fn_1_9500(playerIndex);

        if (target->started != 0) {
            fn_1_644(&playerDirection, &target->position, &target->previousPosition);
            playerCapsule.segment.start = target->previousPosition;
            playerCapsule.segment.delta = playerDirection;
            playerCapsule.radius = 100.0f;
            if (fn_1_181C(&effectCapsule.segment, &playerCapsule.segment, NULL, NULL) <=
                (effectCapsule.radius * effectCapsule.radius) +
                (playerCapsule.radius * playerCapsule.radius)) {
                fn_1_60C(&facing, &facing, &playerDirection);
                fn_1_67C(&facing, &facing);
                fn_1_85C(&facing, &facing, work->scale[0]);
                fn_1_60C(&closestPoint, &work->position, &facing);
                fn_1_644(&separation, &closestPoint, &target->position);
                closestDistance = fn_1_67C(&separation, &separation);
                if (closestDistance <= effectCapsule.radius + playerCapsule.radius) {
                    correction = separation;
                    fn_1_85C(&correction, &correction,
                             (effectCapsule.radius + playerCapsule.radius) - closestDistance);
                    fn_1_60C(&facing, &facing, &correction);
                }
                fn_1_67C(&inwardDirection, &beamDirection);
                fn_1_67C(&responseDirection, &separation);
                fn_1_85C(&responseDirection, &responseDirection, -1.0f);
                fn_1_60C(&oppositeAxis, &responseDirection, &inwardDirection);
                fn_1_85C(&oppositeAxis, &oppositeAxis,
                         2.0f * work->scale[0]);
                target->player->actor->vel = oppositeAxis;
                targetAxis = lbl_1_rodata_1EC;
                if ((fn_1_5A4(&responseDirection, &targetAxis) < 1.0f) &&
                    (target->reached[work->curveIndex] != 0)) {
                    HU3D_PARTICLE *emitter;
                    HU3D_PARTICLE_DATA *particles;

                    lbl_1_bss_1C[playerIndex] += 1;
                    if (lbl_1_bss_1C[playerIndex] > 0x63) {
                        lbl_1_bss_1C[playerIndex] = 0x63;
                    }
                    work->scale[1] = 1.35f;
                    work->scale[2] = work->scale[3] = 1.0f;
                    hitParticleId = work->particles[2];
                    emitter = lbl_802C01C8[hitParticleId].hookData;
                    particles = emitter->data;
                    particleIndex = 0;
                    while (particleIndex < emitter->maxCnt) {
                        if (particles->time < 0) {
                            GXColor color = lbl_1_rodata_B8[target->character];
                            particles->time = 0;
                            particles->color.r = color.r;
                            particles->color.g = color.g;
                            particles->color.b = color.b;
                            particles->vel = separation;
                            fn_1_85C(&particles->vel, &particles->vel, 50.0f);
                            particles->pos = work->position;
                            break;
                        }
                        particleIndex++;
                        particles++;
                    }
                    fn_8006A030((s16)playerIndex, 0xC, 7, 3);
                    {
                        s32 sounds[4] = { MSM_SE_MG615_BEAM_PLAYER_HIT_P1,
                                          MSM_SE_MG615_BEAM_PLAYER_HIT_P2,
                                          MSM_SE_MG615_BEAM_PLAYER_HIT_P3,
                                          MSM_SE_MG615_BEAM_PLAYER_HIT_P4 };
                        HuVecF playerScreenPosition;
                        Hu3D3Dto2D(&target->position, 1, &playerScreenPosition);
                        pan = 64.0f * (playerScreenPosition.x / 576.0f);
                        pan = pan < 0.0f ? 0.0f : (pan > 64.0f ? 64.0f : pan);
                        pan += 32.0f;
                        HuAudFXPlayPan(sounds[playerIndex], (s16) pan);
                    }
                    target->approach[work->curveIndex] = 2.5f * target->approach[work->curveIndex];
                }
                target->reached[work->curveIndex] = 0;
                beamDirection = facing;
                particleDirection = facing;
            }
        }
    }

    capsule.segment.start = work->position;
    capsule.segment.delta = facing;
    capsule.radius = 100.0f;
    collision = fn_1_9238(lbl_1_bss_48,
                          &capsule);
    if (collision) {
        HuVecF collisionDirection;
        HuVecF collisionNormal;
        HuVecF collisionOffset;

        collisionNormal = collision->normal;
        absoluteDot = fabsf(2.0f * fn_1_5A4(&beamDirection, &collisionNormal));
        particleDirection.x = beamDirection.x + collisionNormal.x * absoluteDot;
        particleDirection.y = beamDirection.y + collisionNormal.y * absoluteDot;
        particleDirection.z = beamDirection.z + collisionNormal.z * absoluteDot;
        fn_1_67C(&particleDirection, &particleDirection);
        fn_1_85C(&particleDirection, &particleDirection, work->scale[0]);
        facing = particleDirection;

        fn_1_67C(&collisionDirection, &beamDirection);
        if (fabsf(collisionNormal.x) > 0.0f) {
            alignmentX = fabsf(fn_1_5A4(&collisionDirection, &collisionNormal));
            work->scale[2] = 1.0f - 0.75f * alignmentX;
        }
        if (fabsf(collisionNormal.y) > 0.0f) {
            alignmentY = fabsf(fn_1_5A4(&collisionDirection, &collisionNormal));
            work->scale[3] = 1.0f - 0.75f * alignmentY;
        }
        Hu3D3Dto2D(&work->position, 1, &collisionOffset);
        collisionPan = 64.0f * (collisionOffset.x / 576.0f);
        collisionPan = collisionPan < 0.0f ? 0.0f :
            (collisionPan > 64.0f ? 64.0f : collisionPan);
        collisionPan += 32.0f;
        HuAudFXPlayPan(MSM_SE_MG615_BEAM_WALL_HIT, (s16)collisionPan);
    }

    facing.z = 0.0f;
    work->rotation = atan2f(particleDirection.x, particleDirection.y);
    work->updatedDirection = facing;
    fn_1_60C(&work->position, &work->position, &facing);
    fn_80026790(work->modelObject->mdlId[0], &work->position);

    scaleX = work->scale[1] * work->scale[2];
    scaleY = work->scale[1] * work->scale[3];
    if (scaleX > 1.25f) {
        scaleX = 1.25f;
    }
    if (scaleX < 0.75f) {
        scaleX = 0.75f;
    }
    if (scaleY > 1.25f) {
        scaleY = 1.25f;
    }
    if (scaleY < 0.75f) {
        scaleY = 0.75f;
    }
    fn_800268B0(work->modelObject->mdlId[0], scaleX, scaleY,
                      1.0f);

    {
        HU3D_PARTICLE *emitter;
        HU3D_PARTICLE_DATA *particle;
        s32 particleIndex;

        trailParticleId = work->particles[0];
        emitter = lbl_802C01C8[trailParticleId].hookData;
        particle = emitter->data;
        particleIndex = 0;

        while (particleIndex < emitter->maxCnt) {
            if (particle->time < 0) {
                particle->time = 0;
                particle->pos = work->position;
                break;
            }
            particleIndex++;
            particle++;
        }
    }

    if ((u32)fn_80071A8C() == 6) {
        fn_8006A030((s16)playerIndex, 0x1E, 0x1E, 0);
        fn_80026790(work->modelObject->mdlId[1], &work->position);
        fn_80026A44(work->modelObject->mdlId[1], 1);
        fn_80026A44(work->modelObject->mdlId[1], 0x40000002);
        if (work->curveIndex == 0) {
            HuAudFXPlay(MSM_SE_MG615_RESULT_BEAM_OFF_1);
            HuAudFXPlay(MSM_SE_MG615_RESULT_BEAM_OFF_2);
        }
        fn_1_390(object, fn_1_C9DC);
        return;
    }
}

void fn_1_C9DC(OMOBJ *obj)
{
    M615EffectWork *scene = obj->data;
    s16 modelId;
    HU3D_PARTICLE *hook;
    HU3D_PARTICLE_DATA *record;
    s32 index;

    fn_800269E8(*scene->modelObject->mdlId, 1);
    modelId = scene->particles[1];
    hook = lbl_802C01C8[modelId].hookData;
    record = hook->data;
    for (index = 0; index < hook->maxCnt; index++, record++) {
        if (record->time < 0) {
            record->time = 0;
            record->pos = scene->position;
        }
    }
    if (fn_80071A8C() == 7) {
        fn_1_390(obj, fn_1_CAB0);
        return;
    }
}

void fn_1_CAB0(OMOBJ *object)
{
    void *work = object->data;
}

void fn_1_CAC4(OMOBJ *obj)
{
    HU3D_MODELID *modelId = obj->data;
    HU3D_MOTIONID cameraMotion;

    cameraMotion = fn_8002C4C0(fn_80008DD0(DATANUM(0x4a0000, 9), 0x10000000, 2));
    *modelId = fn_800283F4(cameraMotion, 1);
    fn_1_390(obj, fn_1_CB3C);
}

void fn_1_CB3C(OMOBJ *obj)
{
    M615CameraWork *work = obj->data;

    if (fn_80071A8C() == 7) {
        fn_80027CD0(1, &work->position, &work->up, &work->target);
        fn_8002858C(work->cameraModelId);
        if (fn_8006A124(0x30002U)) {
            fn_1_390(obj, 0);
            return;
        } else {
            fn_1_390(obj, fn_1_CBD0);
            return;
        }
    }
}

void fn_1_CBD0(OMOBJ *obj)
{
    M615CameraWork *camera = obj->data;
    HuVecF desiredTarget;
    HuVecF firstPlayerPosition;
    HuVecF position;
    HuVecF direction;
    f32 cameraMinimumX;
    f32 cameraMaximumX;
    f32 halfRange;
    f32 rangeAdjustment;
    f32 lowerAdjustment;
    f32 upperAdjustment;
    f32 cameraDistance;
    f32 desiredCameraDistance;
    f32 halfFieldOfView;
    f32 viewHeight;
    s32 foundPlayer;
    s32 playerIndex;
    M615MovementWork *firstPlayerSlot;
    M615MovementWork *secondPlayerSlot;

    foundPlayer = 0;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_C[playerIndex] == 1) {
            firstPlayerSlot = fn_1_9500(playerIndex);
            fn_80086BC8(firstPlayerSlot->player->actor, &firstPlayerPosition);
            cameraMinimumX = firstPlayerPosition.x;
            cameraMaximumX = firstPlayerPosition.x;
            desiredTarget = firstPlayerPosition;
            foundPlayer++;
            break;
        }
    }

    for (; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_C[playerIndex] == 1) {
            secondPlayerSlot = fn_1_9500(playerIndex);
            fn_80086BC8(secondPlayerSlot->player->actor, &position);
            if (cameraMinimumX > position.x) {
                cameraMinimumX = position.x;
            }
            if (cameraMaximumX < position.x) {
                cameraMaximumX = position.x;
            }
            fn_1_60C(&desiredTarget, &desiredTarget, &position);
            foundPlayer++;
        }
    }

    if (foundPlayer == 0) {
        lbl_1_bss_8++;
        fn_1_390(obj, 0);
        return;
    }

    cameraMinimumX -= 80.0f;
    cameraMaximumX += 80.0f;
    if (fabs(cameraMaximumX - cameraMinimumX) < 800.0) {
        rangeAdjustment = (f32)(800.0 -
                           fabs(cameraMaximumX - cameraMinimumX));
        cameraMinimumX -= 0.5f * rangeAdjustment;
        cameraMaximumX = cameraMaximumX + 0.5f * rangeAdjustment;
    }
    if (cameraMinimumX < -850.0f) {
        lowerAdjustment = (f32)fabs(-850.0f - cameraMinimumX);
        cameraMinimumX += lowerAdjustment;
        cameraMaximumX += lowerAdjustment;
    }
    if (cameraMaximumX > 850.0f) {
        upperAdjustment = (f32)fabs(850.0f - cameraMaximumX);
        cameraMinimumX -= upperAdjustment;
        cameraMaximumX -= upperAdjustment;
    }

    desiredTarget.x = 0.5f * (cameraMinimumX + cameraMaximumX);
    desiredTarget.y = 75.0f;
    desiredTarget.z = 0.0f;
    halfRange = (f32)(0.5 * fabs(cameraMaximumX - cameraMinimumX));
    halfFieldOfView = 0.5235988f;
    viewHeight = 0.75f * halfRange;
    desiredCameraDistance = viewHeight / tanf(0.5f * halfFieldOfView);
    desiredTarget.y = 0.5f * halfRange;
    camera->target.x += (desiredTarget.x - camera->target.x) / 15.0f;
    camera->target.y += (desiredTarget.y - camera->target.y) / 15.0f;
    fn_1_644(&direction, &camera->target, &camera->position);
    cameraDistance = fn_1_67C(&direction, &direction);
    fn_1_444(&direction, 0.0f, 0.0f, -1.0f);
    cameraDistance += (desiredCameraDistance - cameraDistance) / 15.0f;
    camera->position = camera->target;
    fn_1_85C(&direction, &direction, -cameraDistance);
    fn_1_60C(&camera->position, &camera->position, &direction);
    Hu3DCameraPosSetV(1, &camera->position, &camera->up, &camera->target);
    if (fabsf(desiredCameraDistance - cameraDistance) < 10.0f) {
        lbl_1_bss_8++;
        fn_1_390(obj, 0);
        return;
    }
}

void fn_1_D118(OMOBJ *object)
{
    M615ScoreWork *work = object->data;

    work->character = lbl_80265728[work->playerIndex].charNo;
    work->scoreBoxGroup = MgScoreBoxCreateChar(0x60, 0x30, work->character);
    MgScoreBoxPosSet(work->scoreBoxGroup,
                     lbl_1_data_B4[work->positionIndex].x,
                     lbl_1_data_B4[work->positionIndex].y);
    MgScoreBoxDispSet(work->scoreBoxGroup, 0);
    work->score = MgScoreCreate(0x9B0031, -1, 1);
    MgScoreUnitBankSet(work->score, 0x11);
    MgScoreMaxDigitSet(work->score, 2);
    MgScoreDispOff(work->score);
    MgScorePosSet(work->score,
                  7.0f + lbl_1_data_B4[work->positionIndex].x,
                  lbl_1_data_B4[work->positionIndex].y);
    MgScoreColorSet(work->score, 0xFFU, 0xD8U, 0x15U);
    MgScoreValueSet(work->score, 0);
    fn_1_390(object, fn_1_D270);
}

void fn_1_D270(OMOBJ *object)
{
    M615ScoreWork *work = object->data;

    if (fn_80071A8C() == 5) {
        MgScoreBoxDispSet(work->scoreBoxGroup, TRUE);
        MgScoreDispOn(work->score);
        fn_1_390(object, fn_1_D2D8);
        return;
    }
}

void fn_1_D2D8(OMOBJ *object)
{
    M615ScoreWork *scoreState = object->data;

    MgScoreValueSet(scoreState->score, lbl_1_bss_1C[scoreState->playerIndex]);
}

void fn_1_D32C(f32 separation)
{
    HuVecF collisionPush[4];
    HuVecF clampedPosition;
    HuVecF firstPosition;
    HuVecF difference;
    HuVecF normalizedDifference;
    HuVecF secondPosition;
    HuVecF push;
    HuVecF finalPosition;
    f32 distance;
    f32 pushScale;
    s32 playerIndex;
    s32 otherIndex;
    M615MovementWork *clampWork;
    M615MovementWork *playerWork;
    M615MovementWork *otherWork;
    M615MovementWork *finalWork;

    memset(collisionPush, 0, sizeof(collisionPush));
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        clampWork = fn_1_9500(playerIndex);

        fn_80086BC8(clampWork->player->actor, &clampedPosition);
        if (clampedPosition.x < (-750.0f)) {
            clampedPosition.x = -750.0f;
        }
        if (clampedPosition.x > 750.0f) {
            clampedPosition.x = 750.0f;
        }
        fn_80086B04(clampWork->player->actor, &clampedPosition);
    }

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        playerWork = fn_1_9500(playerIndex);

        fn_80086BC8(playerWork->player->actor, &firstPosition);
        firstPosition.y += separation;
        firstPosition.z = 0.0f;
        for (otherIndex = 0; otherIndex < 4; otherIndex++) {
            if (playerIndex != otherIndex) {
                otherWork = fn_1_9500(otherIndex);

                fn_80086BC8(otherWork->player->actor, &secondPosition);
                secondPosition.y += separation;
                secondPosition.z = 0.0f;
                fn_1_644(&difference, &firstPosition, &secondPosition);
                distance = fn_1_67C(&normalizedDifference, &difference);
                if (0.0f == normalizedDifference.x) {
                    normalizedDifference.x =
                        (f32)(firstPosition.x >= 0.0f ? -1 : 1);
                    fn_1_67C(&normalizedDifference, &normalizedDifference);
                }
                if (distance <= separation + separation) {
                    pushScale = 0.5f * ((separation + separation) - distance);
                    fn_1_85C(&push, &normalizedDifference, pushScale);
                    fn_1_60C(&collisionPush[playerIndex], &collisionPush[playerIndex], &push);
                    lbl_802BFFFC[playerWork->player->padNo] *= 0.55f;
                    lbl_802BFFF8[playerWork->player->padNo] *= 0.55f;
                }
            }
        }
    }

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        finalWork = fn_1_9500(playerIndex);

        fn_80086BC8(finalWork->player->actor, &finalPosition);
        fn_1_60C(&finalPosition, &finalPosition, &collisionPush[playerIndex]);
        fn_80086B04(finalWork->player->actor, &finalPosition);
    }
}

void fn_1_D708(OMOBJ *object)
{
    void *work;

    work = object->data;
    fn_1_390(object, fn_1_D748);
}

void fn_1_D748(OMOBJ *object)
{
    void *workData[1];
    workData[0] = object->data;
    fn_1_D32C(60.0f);
    if (fn_80071A8C() == 6) {
        fn_1_390(object, fn_1_D7A8);
        return;
    }
}

void fn_1_D7A8(OMOBJ *object)
{
    void *workData[1];

    workData[0] = object->data;
    fn_1_D32C(100.0f);
}

void fn_1_D7E8(M615MovementWork *work, s32 motionSlot, f32 endTime, s32 attr)
{
    if (work->state != motionSlot) {
        work->state = motionSlot;
        fn_8005C2C8(work->character,
                           work->player->omObj->mtnId[motionSlot],
                           0.0f, endTime, attr);
    }
}

void fn_1_D864(M615MovementWork *work)
{
    HuVecF position;
    f32 speed;
    s32 axis;

    fn_80086BC8(work->player->actor, &position);
    position.z = 0.0f;
    fn_80086B04(work->player->actor, &position);
    work->previousPosition = work->position;
    work->position = position;
    work->position.y += 100.0f;

    speed = fn_1_67C(&work->player->actor->vel, &work->player->actor->vel);
    speed += -speed / 6.0f;
    fn_1_85C(&work->player->actor->vel, &work->player->actor->vel, speed);

    for (axis = 0; axis < 3; axis++) {
        work->approach[axis] += (1.0f - work->approach[axis]) / 15.0f;
    }

    for (axis = 0; axis < 3; axis++) {
        if (work->reached[axis] == 0) {
            HuVecF difference;
            M615EffectWork *targetPath;

            targetPath = fn_1_9518(axis);
            fn_1_644(&difference, &targetPath->position, &work->position);
            if (fn_1_454(&difference) > 100.0f * work->approach[axis]) {
                work->reached[axis] = 1;
            }
        }
    }

    if (fn_80087114(work->player, 1) != 0) {
        fn_8006A030(work->playerIndex, 0x14, 7, 3);
    }
}

const MGACTOR_PARAM lbl_1_rodata_264 = { 0 };

void fn_1_DA90(OMOBJ *object)
{
    M615MovementWork *work = object->data;
    MGACTOR_PARAM actorParam;
    HuVecF spawnPosition;
    s32 axis;

    work->character = lbl_80265728[work->playerIndex].charNo;

    actorParam = lbl_1_rodata_264;
    actorParam.height = 150.0f;
    actorParam.radius = 40.0f;
    actorParam.param = work->playerIndex;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.correctHookParam = 0;
    actorParam.narrowHook = NULL;
    actorParam.correctHook = NULL;

    work->player = fn_80080728(work->playerIndex, &actorParam, 2, 1, 0x23,
                                  lbl_1_data_40);
    fn_8007E818(work->player);
    fn_800805CC(work->player->actor, 0x80);
    fn_80026E44(work->player->actor->mdlId);
    fn_80026B8C(work->player->actor->mdlId, 5);

    spawnPosition = lbl_1_data_84[work->pathIndex];
    fn_80086AA8(work->player->actor, &spawnPosition);
    work->position = spawnPosition;
    work->previousPosition = work->position;
    work->previousPosition.y += 100.0f;
    work->position.y += 100.0f;

    for (axis = 0; axis < 3; axis++) {
        work->reached[axis] = 0;
        work->approach[axis] = 1.0f;
    }
    work->started = 1;
    work->state = -1;

    work->targetX = 1700.0f * frandf() - 850.0f;
    fn_1_390(object, fn_1_DD08);
}

void fn_1_DD08(OMOBJ *object)
{
    M615MovementWork *work = object->data;

    if (fn_80071A8C() == 5) {
        if (lbl_80265728[work->playerIndex].type != 0) {
            fn_8007E888(work->player);
            fn_1_390(object, fn_1_E02C);
            return;
        }
        fn_8007E888(work->player);
        fn_1_390(object, fn_1_DDA0);
        return;
    }
}

void fn_1_DDA0(OMOBJ *object)
{
    M615MovementWork *work = object->data;
    HuVecF difference;
    HuVecF position;
    f32 speed;
    s32 axis;

    if (fn_80071A8C() == 6) {
        fn_8007E818(work->player);
        fn_800805CC(work->player->actor, 4);
        fn_1_390(object, fn_1_E6BC);
        return;
    }

    fn_80086BC8(work->player->actor, &position);
    position.z = 0.0f;
    fn_80086B04(work->player->actor, &position);
    work->previousPosition = work->position;
    work->position = position;
    work->position.y += 100.0f;

    speed = fn_1_67C(&work->player->actor->vel, &work->player->actor->vel);
    speed += -speed / 6.0f;
    fn_1_85C(&work->player->actor->vel, &work->player->actor->vel, speed);

    for (axis = 0; axis < 3; axis++) {
        work->approach[axis] +=
            (1.0f - work->approach[axis]) / 15.0f;
    }

    for (axis = 0; axis < 3; axis++) {
        if (work->reached[axis] == 0) {
            M615EffectWork *targetPath;
            s32 pathIndex;
            M615EffectWork *path;
            M615EffectWork *pathPosition;

            pathIndex = axis;
            path = lbl_1_bss_2C[pathIndex];
            pathPosition = path;
            targetPath = pathPosition;
            fn_1_644(&difference, &targetPath->position, &work->position);
            if (fn_1_454(&difference) > 100.0f * work->approach[axis]) {
                work->reached[axis] = 1;
            }
        }
    }

    if (fn_80087114(work->player, 1) != 0) {
        fn_8006A030(work->playerIndex, 0x14, 7, 3);
    }
    if (fn_80087148(work->player, 0x10) != 0) {
        fn_1_390(object, fn_1_E358);
        return;
    }
}

void fn_1_E02C(OMOBJ *object)
{
    M615MovementWork *work = object->data;
    s32 button;

    if (fn_80071A8C() == 6) {
        fn_8007E818(work->player);
        fn_800805CC(work->player->actor, 4);
        fn_1_390(object, fn_1_E6BC);
        return;
    }

    fn_1_D864(work);
    if (fn_80087148(work->player, 0x10) != 0) {
        fn_80080520(work->player, 0, 0, 0x100, 0x100);
        fn_1_390(object, fn_1_E358);
        return;
    }

    if (lbl_1_rodata_114[lbl_80265728[work->playerIndex].comDif](work, &button) != 0) {
        fn_80080520(work->player, button, 0, 0x100, 0x100);
        fn_1_390(object, fn_1_E358);
        return;
    }
    fn_80080520(work->player, button, 0, 0, 0);
}

void fn_1_E358(OMOBJ *object)
{
    M615MovementWork *work = object->data;
    s32 button;

    if (fn_80071A8C() == 6) {
        fn_8007E818(work->player);
        fn_800805CC(work->player->actor, 4);
        fn_1_390(object, fn_1_E6BC);
        return;
    }

    fn_1_D864(work);
    if (work->reached == 0) {
        fn_80087114(work->player, 2);
    }
    if (fn_80087148(work->player, 0x10) == 0) {
        if (lbl_80265728[work->playerIndex].type != 0) {
            fn_1_390(object, fn_1_E02C);
        } else {
            fn_1_390(object, fn_1_DDA0);
        }
    } else if (lbl_80265728[work->playerIndex].type != 0 &&
               fn_80087148(work->player, 1) == 0) {
        lbl_1_rodata_114[lbl_80265728[work->playerIndex].comDif](work, &button);
        fn_80080520(work->player, button, 0, 0, 0x100);
    }
}

void fn_1_E6BC(OMOBJ *object)
{
    M615MovementWork *work = object->data;
    HuVecF position;
    f32 flagAngle;
    f32 modeAngle;
    f32 flagResult;
    f32 flagStoredAngle;
    f32 modeResult;
    f32 modeStoredAngle;

    fn_80086BC8(work->player->actor, &position);
    position.z = 0.0f;
    fn_80086B04(work->player->actor, &position);
    work->started = 0;
    fn_1_444(&work->player->actor->push, 0.0f, 0.0f, 0.0f);
    fn_1_444(&work->player->actor->vel, 0.0f, 0.0f, 0.0f);

    if (fn_8006A124(0x30002U)) {
        flagAngle = work->player->actor->rotY;
        flagResult = (f32)fmod(flagAngle, 180.0);
        flagStoredAngle = flagResult;
        work->player->actor->rotY = flagStoredAngle;
        fn_1_390(object, fn_1_E8EC);
    } else if (fn_80071A8C() == 7) {
        if (work->state != 1) {
            work->state = 1;
            fn_8005C2C8(work->character, work->player->omObj->mtnId[1], 0.0f,
                               4.0f, 0x40000001);
        }
        modeAngle = work->player->actor->rotY;
        modeResult = (f32)fmod(modeAngle, 180.0);
        modeStoredAngle = modeResult;
        work->player->actor->rotY = modeStoredAngle;
        fn_1_390(object, fn_1_EC50);
        return;
    }
}

void fn_1_E8EC(OMOBJ *object)
{
    M615MovementWork *work = object->data;
    HuVecF targetDirection;
    HuVecF currentDirection;
    f32 currentAngle;
    f32 zero;
    f32 cross;
    f32 angleDifference;
    f32 adjustedAngle;

    zero = 0.0f;
    currentAngle = (f32)((M_PI * work->player->actor->rotY) /
                         180.0);
    fn_1_444(&targetDirection, (f32)sin(zero), 0.0f,
             (f32)cos(zero));
    fn_1_444(&currentDirection, (f32)sin(currentAngle), 0.0f,
             (f32)cos(currentAngle));
    angleDifference = fn_1_398(fn_1_5A4(&targetDirection, &currentDirection));
    cross = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    adjustedAngle = currentAngle;
    if (cross >= 0.0f) {
        adjustedAngle += 0.1f * angleDifference;
    } else if (cross < 0.0f) {
        adjustedAngle -= 0.1f * angleDifference;
    }
    while (adjustedAngle > (2.0 * M_PI)) {
        adjustedAngle = (f32)((f64)adjustedAngle - (2.0 * M_PI));
    }
    while (adjustedAngle < (-2.0 * M_PI)) {
        adjustedAngle += (2.0 * M_PI);
    }
    work->player->actor->rotY = (f32)((f64)(180.0f * adjustedAngle) /
        M_PI);
    fn_1_444(&work->player->actor->push, 0.0f, 0.0f,
             0.0f);
    fn_1_444(&work->player->actor->vel, 0.0f, 0.0f,
             0.0f);
    if (fabsf(adjustedAngle - currentAngle) <= 0.01f) {
        work->player->actor->rotY = 0.0f;
        if (work->state != 0) {
            work->state = 0;
            fn_8005C2C8(work->character, work->player->omObj->mtnId[0],
                               0.0f, 4.0f, 0x40000001);
        }
        fn_1_390(object, NULL);
        return;
    }
}

void fn_1_EC50(OMOBJ *object)
{
    M615MovementWork *work = object->data;
    HuVecF targetDirection;
    HuVecF currentDirection;
    f32 currentAngle;
    f32 zero;
    f32 cross;
    f32 angleDifference;
    f32 adjustedAngle;

    zero = 0.0f;
    currentAngle = (f32)((M_PI * work->player->actor->rotY) /
                         180.0);
    fn_1_444(&targetDirection, (f32)sin(zero), 0.0f,
             (f32)cos(zero));
    fn_1_444(&currentDirection, (f32)sin(currentAngle), 0.0f,
             (f32)cos(currentAngle));
    angleDifference = fn_1_398(fn_1_5A4(&targetDirection, &currentDirection));
    cross = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    adjustedAngle = currentAngle;
    if (cross >= 0.0f) {
        adjustedAngle += 0.1f * angleDifference;
    } else if (cross < 0.0f) {
        adjustedAngle -= 0.1f * angleDifference;
    }
    while (adjustedAngle > (2.0 * M_PI)) {
        adjustedAngle = (f32)((f64)adjustedAngle - (2.0 * M_PI));
    }
    while (adjustedAngle < (-2.0 * M_PI)) {
        adjustedAngle += (2.0 * M_PI);
    }
    work->player->actor->rotY = (f32)((f64)(180.0f * adjustedAngle) /
        M_PI);
    fn_1_444(&work->player->actor->push, 0.0f, 0.0f,
             0.0f);
    fn_1_444(&work->player->actor->vel, 0.0f, 0.0f,
             0.0f);
    if (fabsf(adjustedAngle - currentAngle) <= 0.01f) {
        work->player->actor->rotY = 0.0f;
        lbl_1_bss_8++;
        if (work->state != 0) {
            work->state = 0;
            fn_8005C2C8(work->character, work->player->omObj->mtnId[0],
                               0.0f, 4.0f, 0x40000001);
        }
        fn_1_390(object, fn_1_EFD4);
        return;
    }
}

void fn_1_EFD4(OMOBJ *object)
{
    M615MovementWork *work = object->data;
    HuVecF position;

    fn_80086BC8(work->player->actor, &position);
    position.z = 0.0f;
    fn_80086B04(work->player->actor, &position);
    if (fn_80071A8C() == 8) {
        fn_1_390(object, fn_1_F058);
        return;
    }
}

void fn_1_F058(OMOBJ *object)
{
    M615MovementWork *work = object->data;
    HuVecF position;

    fn_80086BC8(work->player->actor, &position);
    position.z = 0.0f;
    fn_80086B04(work->player->actor, &position);
    if (lbl_1_bss_C[work->playerIndex] == 1) {
        if (work->state != 0xE) {
            work->state = 0xE;
            fn_8005C2C8(work->character,
                               work->player->omObj->mtnId[14],
                               0.0f, 4.0f, 0);
        }
    } else if (work->state != 0xF) {
        work->state = 0xF;
        fn_8005C2C8(work->character,
                           work->player->omObj->mtnId[15],
                           0.0f, 4.0f, 0);
    }
}

s32 fn_1_F170(M615MovementWork *work, s32 *direction)
{
    *direction = 0;
    if (fn_80087148(work->player, 1) != 0) {
        return 0;
    }
    if (fabsf(work->position.x - work->targetX) <= 80.0f) {
        work->targetX = (1500.0f * frandf()) - 750.0f;
    }
    if (work->position.x < work->targetX) {
        *direction = 0x38;
    } else if (work->position.x > work->targetX) {
        *direction = -0x38;
    }
    if ((rand8() % 180) == 0) {
        return 1;
    }
    return 0;
}

s32 fn_1_F2D4(M615MovementWork *work, s32 *direction)
{
    HuVecF playerPosition;
    HuVecF otherPlayerPosition;
    M615Segment movementSegment;
    s32 playerIndex;

    *direction = 0;
    fn_80086BC8(work->player->actor, &playerPosition);
    if (fn_80087148(work->player, 1) != 0) {
        return 0;
    }

    if (fabsf(work->position.x - work->targetX) <= 80.0f) {
        work->targetX = (1500.0f * frandf()) - 750.0f;
    }

    if (work->position.x < work->targetX) {
        *direction = 0x38;
    } else if (work->position.x > work->targetX) {
        *direction = -0x38;
    }

    if (fn_80087148(work->player, 1) == 0) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (playerIndex != work->playerIndex) {
                M615MovementWork *otherPlayerSlot;

                otherPlayerSlot = fn_1_9500(playerIndex);
                fn_80086BC8(otherPlayerSlot->player->actor, &otherPlayerPosition);
                movementSegment.start = playerPosition;
                if (*direction > 0) {
                    fn_1_444(&movementSegment.delta, 100.0f,
                             0.0f, 0.0f);
                } else {
                    fn_1_444(&movementSegment.delta, -100.0f,
                             0.0f, 0.0f);
                }
                if (fn_1_643C(&otherPlayerPosition, &movementSegment) <=
                    1600.0f) {
                    return 1;
                }
            }
        }
    }

    if ((rand8() % 120) == 0) {
        return 1;
    }
    return 0;
}

M615CollisionPool *lbl_1_bss_48;
M615MovementWork *lbl_1_bss_38[4];
M615EffectWork *lbl_1_bss_2C[3];
s32 lbl_1_bss_1C[4];
s32 lbl_1_bss_C[4];
s32 lbl_1_bss_8;

s32 fn_1_F554(M615MovementWork *work, s32 *direction)
{
    M615Segment movementSegment;
    HuVecF playerPosition;
    HuVecF storedPointA = {0.0f, -1.0f, 0.0f};
    HuVecF storedPointB = {0.0f, 1.0f, 0.0f};
    HuVecF difference;
    HuVecF targetPosition;
    HuVecF otherPlayerPosition;
    f32 nearestDistance;
    f32 distance;
    f32 speed;
    s32 nearestIndex;
    s32 index;
    M615EffectWork *target;
    *direction = 0;
    fn_80086BC8(work->player->actor, &playerPosition);

    nearestIndex = 0;
    target = fn_1_9518(nearestIndex);
    targetPosition = target->position;
    fn_1_644(&difference, &targetPosition, &playerPosition);
    nearestDistance = fn_1_454(&difference);
    for (index = 1; index < 3; index++) {
        target = fn_1_9518(index);
        targetPosition = target->position;
        fn_1_644(&difference, &targetPosition, &playerPosition);
        if (fn_1_454(&difference) < nearestDistance) {
            nearestIndex = index;
            nearestDistance = fn_1_454(&difference);
        }
    }

    targetPosition = targetPositionGet(lbl_1_bss_2C[nearestIndex]);
    distance = fabsf(targetPosition.x - playerPosition.x);
    if (distance > 10.0f) {
        distance = 10.0f;
    }
    if (playerPosition.x > targetPosition.x) {
        speed = -5.6f;
    } else {
        speed = 5.6f;
    }
    *direction = (s32)speed;
    *direction = (s32)((f32)*direction * distance);

    if (nearestDistance <= 450.0f && nearestDistance > 80.0f) {
        return 1;
    }

    if (fn_80087148(work->player, 1) == 0) {
        for (index = 0; index < 4; index++) {
            if (index != work->playerIndex) {
                M615MovementWork *otherPlayerSlot = fn_1_9500(index);
                fn_80086BC8(otherPlayerSlot->player->actor, &otherPlayerPosition);
                movementSegment.start = playerPosition;
                if (*direction > 0) {
                    fn_1_444(&movementSegment.delta, 100.0f,
                             0.0f, 0.0f);
                } else {
                    fn_1_444(&movementSegment.delta, -100.0f,
                             0.0f, 0.0f);
                }
                if (fn_1_643C(&otherPlayerPosition, &movementSegment) <=
                    1600.0f) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

s32 fn_1_F938(M615MovementWork *work, s32 *direction)
{
    M615Segment movementSegment;
    HuVecF playerPosition;
    HuVecF directionAxis = {0.0f, -1.0f, 0.0f};
    HuVecF stageAxis = {0.0f, 1.0f, 0.0f};
    M615MovementWork *selectedPlayer;
    f32 projectedX;
    f32 bestFacing;
    f32 scanDistance;
    u32 elapsedSeconds;
    s32 highestScore;
    s32 selectedPlayerIndex;
    s32 isLeader;
    s32 playerIndex;
    s32 nearestStageIndex;
    s32 predictedStageIndex;
    M615MovementWork *otherPlayerSlot;

    elapsedSeconds = (u32)fn_80071B58() / 60;
    isLeader = 1;
    *direction = 0;
    fn_80086BC8(work->player->actor, &playerPosition);
    projectedX = playerPosition.x / 850.0f;
    projectedX *= (-100.0f);

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if ((work->playerIndex != playerIndex) &&
            (lbl_1_bss_1C[playerIndex] >= lbl_1_bss_1C[work->playerIndex])) {
            isLeader = 0;
            break;
        }
    }

    if (isLeader != 0) {
        highestScore = -1;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if ((work->playerIndex != playerIndex) &&
                (highestScore < lbl_1_bss_1C[playerIndex])) {
                highestScore = lbl_1_bss_1C[playerIndex];
            }
        }
    }

    if ((isLeader != 0) && (elapsedSeconds <= 10)) {
        selectedPlayer = NULL;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (playerIndex != work->playerIndex && highestScore == lbl_1_bss_1C[playerIndex]) {
                M615MovementWork *candidate = fn_1_9500(playerIndex);
                HuVecF position;
                HuVecF offset;
                f32 alignment;
                fn_80086BC8(candidate->player->actor, &position);
                fn_1_644(&offset, &position, &playerPosition);
                scanDistance = fn_1_67C(&offset, &offset);
                alignment = fn_1_5A4(&directionAxis, &offset);
                if (alignment >= 0.5f && scanDistance <= 500.0f) {
                    selectedPlayer = candidate;
                    selectedPlayerIndex = playerIndex;
                    bestFacing = alignment;
                    break;
                }
            }
        }
        for (; playerIndex < 4; playerIndex++) {
            if (playerIndex != work->playerIndex && highestScore == lbl_1_bss_1C[playerIndex]) {
                M615MovementWork *candidate = fn_1_9500(playerIndex);
                HuVecF position;
                HuVecF offset;
                f32 alignment;
                fn_80086BC8(candidate->player->actor, &position);
                fn_1_644(&offset, &position, &playerPosition);
                scanDistance = fn_1_67C(&offset, &offset);
                alignment = fn_1_5A4(&directionAxis, &offset);
                if (alignment >= 0.5f && bestFacing < alignment && scanDistance <= 500.0f) {
                    selectedPlayer = candidate;
                    selectedPlayerIndex = playerIndex;
                    bestFacing = alignment;
                }
            }
        }

        if (selectedPlayer) {
            HuVecF otherPlayerPosition;
            f32 distance;
            fn_80086BC8(selectedPlayer->player->actor, &otherPlayerPosition);
            distance = fabsf(otherPlayerPosition.x - playerPosition.x);
            if (distance > 10.0f) {
                distance = 10.0f;
            }
            *direction = (s32)(playerPosition.x > otherPlayerPosition.x ?
                (-5.6f) : 5.6f);
            *direction = (s32)((f32)*direction * distance);
            return 1;
        }
    }

    {
        M615EffectWork *stage;
        HuVecF stagePosition;
        HuVecF stageDirection;
        HuVecF difference;
        f32 nearestDistance;
        f32 distance;
        nearestStageIndex = 0;
        stage = fn_1_9518(0);
        stagePosition = stage->position;
        fn_1_644(&difference, &stagePosition, &playerPosition);
        nearestDistance = fn_1_454(&difference);

        for (playerIndex = 1; playerIndex < 3; playerIndex++) {
            stage = fn_1_9518(playerIndex);
            stagePosition = stage->position;
            stagePosition.x += projectedX;
            stagePosition.y += fabsf(projectedX);
            fn_1_644(&difference, &stagePosition, &playerPosition);
            if (fn_1_454(&difference) < nearestDistance) {
                nearestStageIndex = playerIndex;
                nearestDistance = fn_1_454(&difference);
            }
        }

        stage = fn_1_9518(nearestStageIndex);
        fn_1_444(&stageDirection, (f32)sin((f64)stage->rotation),
                 (f32)cos((f64)stage->rotation), 0.0f);
        stagePosition = stage->position;
        stagePosition.x += projectedX;
        stagePosition.y += fabsf(projectedX);
        fn_1_67C(&stageDirection, &stageDirection);
        fn_1_644(&difference, &stagePosition, &playerPosition);
        distance = fn_1_67C(&difference, &difference);

        if (nearestDistance <= 300.0f) {
            f32 horizontalDistance;
            horizontalDistance = fabsf(stagePosition.x - playerPosition.x);
            if (horizontalDistance > 10.0f) {
                horizontalDistance = 10.0f;
            }
            *direction = (s32)(playerPosition.x > stagePosition.x ?
                (-5.6f) : 5.6f);
            *direction = (s32)((f32)*direction * horizontalDistance);
            return 1;
        }

        if ((fn_1_5A4(&directionAxis, &stageDirection) >= 0.0f) &&
            (fn_1_5A4(&stageAxis, &difference) >= 0.25f) &&
            (distance < 600.0f)) {
            f32 horizontalDistance;
            horizontalDistance = fabsf(stagePosition.x - playerPosition.x);
            if (horizontalDistance > 10.0f) {
                horizontalDistance = 10.0f;
            }
            *direction = (s32)(playerPosition.x > stagePosition.x ?
                (-5.6f) : 5.6f);
            *direction = (s32)((f32)*direction * horizontalDistance);
            return 1;
        }

    }

    if ((fn_80087148(work->player, 0x10) == 0) &&
        (fn_80087148(work->player, 1) == 0)) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (playerIndex != work->playerIndex) {
                M615MovementWork *otherWork;
                HuVecF otherPlayerPosition;
                HuVecF playerDirection;
                HuVecF otherDirection;
                f32 facing;
                f32 distance;

                otherWork = fn_1_9500(playerIndex);
                fn_80086BC8(otherWork->player->actor, &otherPlayerPosition);
                fn_1_644(&otherDirection,
                         &otherWork->position,
                         &otherWork->previousPosition);
                if (!(otherDirection.y >= 0.0f)) {
                    fn_1_644(&playerDirection, &otherPlayerPosition,
                             &playerPosition);
                    distance = fn_1_67C(&playerDirection, &playerDirection);
                    facing = fn_1_5A4(&stageAxis, &playerDirection);
                    if ((facing >= 0.6f) &&
                        (distance <= 400.0f)) {
                        *direction = (s32)(playerPosition.x > otherPlayerPosition.x ?
                            5.6f : (-5.6f));
                        *direction *= 10;
                        if ((facing >= 0.75f) &&
                            (distance <= 100.0f)) {
                            return 1;
                        }
                        return 0;
                    }
                }
            }
        }
    }

    {
        M615EffectWork *stage;
        HuVecF stagePosition;
        HuVecF stageDirection;
        HuVecF difference;
        f32 nearestDistance;
        predictedStageIndex = 0;
        stage = fn_1_9518(0);
        stagePosition = stage->position;
        stagePosition.x += projectedX;
        stagePosition.y += fabsf(projectedX);
        fn_1_444(&stageDirection, (f32)sin((f64)stage->rotation),
                 (f32)cos((f64)stage->rotation), 0.0f);
        fn_1_85C(&stageDirection, &stageDirection,
                 120.0f * stage->scale[0]);
        fn_1_60C(&stagePosition, &stagePosition, &stageDirection);
        fn_1_644(&difference, &stagePosition, &playerPosition);
        nearestDistance = fn_1_454(&difference);
        for (playerIndex = 1; playerIndex < 3; playerIndex++) {
            stage = fn_1_9518(playerIndex);
            stagePosition = stage->position;
            stagePosition.x += projectedX;
            stagePosition.y += fabsf(projectedX);
            fn_1_444(&stageDirection, (f32)sin((f64)stage->rotation),
                     (f32)cos((f64)stage->rotation), 0.0f);
            fn_1_85C(&stageDirection, &stageDirection,
                     60.0f * stage->scale[0]);
            fn_1_60C(&stagePosition, &stagePosition, &stageDirection);
            fn_1_644(&difference, &stagePosition, &playerPosition);
            if (fn_1_454(&difference) < nearestDistance) {
                predictedStageIndex = playerIndex;
                nearestDistance = fn_1_454(&difference);
            }
        }

        {
            M615EffectWork *selectedStage = lbl_1_bss_2C[predictedStageIndex];
        stagePosition = selectedStage->position;
        stagePosition.x += projectedX;
        stagePosition.y += fabsf(projectedX);
        {
            f32 horizontalDistance;
            horizontalDistance = fabsf(stagePosition.x - playerPosition.x);
            if (horizontalDistance > 10.0f) {
                horizontalDistance = 10.0f;
            }
            *direction = (s32)(playerPosition.x > stagePosition.x ?
                (-5.6f) : 5.6f);
            *direction = (s32)((f32)*direction * horizontalDistance);
        }

        }
    }

    if (fn_80087148(work->player, 1) == 0) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (playerIndex != work->playerIndex) {
                HuVecF otherPlayerPosition;
                otherPlayerSlot = fn_1_9500(playerIndex);
                fn_80086BC8(otherPlayerSlot->player->actor, &otherPlayerPosition);
                movementSegment.start = playerPosition;
                if (*direction > 0) {
                    fn_1_444(&movementSegment.delta, 100.0f,
                             0.0f, 0.0f);
                } else {
                    fn_1_444(&movementSegment.delta, -100.0f,
                             0.0f, 0.0f);
                }
                if (fn_1_643C(&otherPlayerPosition, &movementSegment) <=
                    3600.0f) {
                    return 1;
                }
            }
        }
    }

    return 0;
}
