/* Catch You Letter: the players and their movement, the computer players, the items and
 * particles, and the round flow and its results. */
#include "REL/m607DLL/game.h"
extern s32 lbl_1_data_28[2][4];
#include <string.h>

void fn_1_1180(OMOBJ *object);
void fn_1_1354(OMOBJ *object);
void fn_1_6318(OMOBJ *object);
void fn_1_90E0(OMOBJ *object);
void fn_1_CEA8(OMOBJ *object);
void fn_1_EA48(OMOBJ *object);
void fn_1_D7C(OMOBJ *object);
void fn_1_EB4(OMOBJ *object);
void fn_1_10C8(OMOBJ *object);
void fn_1_1144(OMOBJ *object);
void fn_1_12CC(OMOBJ *object);
void fn_1_144C(OMOBJ *object);
void fn_1_14A0(OMOBJ *object);
void fn_1_1720(OMOBJ *object);
void fn_1_1764(OMOBJ *object);
void fn_1_17DC(OMOBJ *object);
void fn_1_6728(OMOBJ *object);
void fn_1_6784(OMOBJ *object);
void fn_1_6828(OMOBJ *object);
void fn_1_69D4(OMOBJ *object);
void fn_1_6908(OMOBJ *object);
void fn_1_74DC(OMOBJ *object);
void fn_1_6A90(OMOBJ *object);
void fn_1_6FC4(OMOBJ *object);
void fn_1_761C(OMOBJ *object);
void fn_1_7740(OMOBJ *object);
void fn_1_82D4(OMOBJ *object);
void fn_1_8324(OMOBJ *object);
void fn_1_93D8(OMOBJ *object);
void fn_1_9590(OMOBJ *object);
void fn_1_964C(OMOBJ *object);
void fn_1_C004(OMOBJ *object);
void fn_1_9F24(OMOBJ *object);
void fn_1_B998(OMOBJ *object);
void fn_1_AD60(OMOBJ *object);
void fn_1_B248(OMOBJ *object);
void fn_1_C0A8(OMOBJ *object);
void fn_1_CACC(OMOBJ *object);
void fn_1_D83C(OMOBJ *object);
void fn_1_D16C(OMOBJ *object);
void fn_1_D1C4(OMOBJ *object);
void fn_1_D464(OMOBJ *object);
void fn_1_DCCC(OMOBJ *object);
void fn_1_E544(OMOBJ *object);
void fn_1_DE2C(OMOBJ *object);
void fn_1_E34C(OMOBJ *object);
void fn_1_E4D0(OMOBJ *object);
void fn_1_F08C(OMOBJ *object);
void fn_1_F1D4(OMOBJ *object);

#include "math.h"

#include "dolphin/gx.h"

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/mtx.h"

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

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "humath.h"

#include "game/hu3d.h"

#include "game/mg/colman.h"

#include "include/game/gamework.h"

#include "dolphin.h"

f32 fn_1_364(f32 value)
{
    return (f32)acos((f64)(value > 1.0f ? 1.0f :
                          (value < -1.0f ? -1.0f : value)));
}

float fn_1_220(float x);

void fn_1_410(HuVecF *out, f32 x, f32 y, f32 z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}

/* SDK sqrtf refinement, retaining its volatile single-precision result. */

f32 fn_1_420(const Vec *src)
{
    return sqrtf(src->z * src->z + (src->x * src->x + src->y * src->y));
}

f32 fn_1_540(const HuVecF *v)
{
    return (v->z * v->z) + ((v->x * v->x) + (v->y * v->y));
}

f32 fn_1_570(const HuVecF *a, const HuVecF *b)
{
    return PSVECDotProduct(a, b);
}

void fn_1_5A0(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECCrossProduct(a, b, dst);
}

void fn_1_5D8(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECAdd(a, b, dst);
}

void fn_1_610(HuVecF *dst, const HuVecF *a, const HuVecF *b)
{
    PSVECSubtract(a, b, dst);
}

f32 fn_1_648(HuVecF *dst, const HuVecF *src)
{
    f32 magnitude = fn_1_420(src);
    if (magnitude > 1.1920929e-07f) {
        f32 inverse = 1.0f / magnitude;
        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = 0.0f;
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
    return magnitude;
}

void fn_1_828(HuVecF *dst, const HuVecF *src, f32 scale)
{
    PSVECScale(src, dst, scale);
}

/* Swaps two halfwords. */
void fn_1_860(s16 *first, s16 *second) {
    s16 value;

    value = *first;
    *first = *second;
    *second = value;
}

f32 fn_1_884(Point3d *point, Point3d *start, Point3d *end)
{
    Point3d difference;
    Point3d segment;
    Point3d projectedSegment;
    f32 projection;
    f32 segmentLengthSquared;

    PSVECSubtract(end, start, &segment);
    PSVECSubtract(point, start, &difference);
    projection = fn_1_570(&difference, &segment);
    if (projection <= 0.0f) {
        projection = 0.0f;
    } else {
        segmentLengthSquared = fn_1_540(&segment);
        if (projection >= segmentLengthSquared) {
            projection = 1.0f;
            PSVECSubtract(&difference, &segment, &difference);
        } else {
            projection /= segmentLengthSquared;
            fn_1_828(&projectedSegment, &segment, projection);
            PSVECSubtract(&difference, &projectedSegment, &difference);
        }
    }
    return fn_1_540(&difference);
}

const f32 lbl_1_rodata_84[4] = { 1.0f, 2.2f, 1.0f, 1.0f };
const Vec lbl_1_rodata_94 = { -898.0f, 167.0f, -656.0f };

M607ItemWork *fn_1_9FC(s32 index) { return lbl_1_bss_24[index]; }

M607PlayerWork *fn_1_A14(s32 index) { return lbl_1_bss_4C[index]; }

M607RootWork *fn_1_A2C(void) {
    return lbl_1_bss_20;
}

void fn_1_A3C(s32 out[4])
{
    s32 teamPlayer[4] = { -1, -1, -1, -1 };
    s32 player;

    for (player = 0; player < 4; player++) {
        teamPlayer[lbl_80265728[player].grpNo] = player;
    }

    if ((teamPlayer[0] == -1) || (teamPlayer[1] == -1) ||
        (teamPlayer[2] == -1) || (teamPlayer[3] == -1)) {
        for (player = 0; player < 4; player++) {
            out[player] = player;
        }
        return;
    }

    for (player = 0; player < 4; player++) {
        out[player] = lbl_80265728[player].grpNo;
    }
}

typedef struct M607SetupWork {
    u8 unknown00[4]; /* No code names this field. */
    s32 winnerCount;
    s16 streamId;
    s32 effectId;
} M607SetupWork;

void fn_1_B2C(OMOBJ *obj)
{
    M607SetupWork *work = obj->data;
    s32 winners[4];
    s32 setupPlayer;

    lbl_1_bss_C = 0;
    work->streamId = -1;
    work->effectId = -1;
    fn_1_A3C(winners);
    fn_1_2BC(0xA, 8, fn_1_1180);
    fn_1_2BC(0xA, 0x10, fn_1_1354);
    for (setupPlayer = 0; setupPlayer < 4; setupPlayer++) {
        lbl_1_bss_4C[setupPlayer] = fn_1_2BC(0x14, 0x54, fn_1_6318);
        lbl_1_bss_4C[setupPlayer]->playerIndex = setupPlayer;
        lbl_1_bss_4C[setupPlayer]->positionIndex = winners[setupPlayer];
    }
    lbl_1_bss_20 = fn_1_2BC(0x14, 0x4C, fn_1_90E0);
    fn_1_2BC(0x1E, 0xC10, fn_1_CEA8);
    fn_1_2BC(0x1E, 0x1C28, fn_1_EA48);
    fn_8005DDEC(5);
    fn_1_35C(obj, fn_1_D7C);
}

void fn_1_D7C(OMOBJ *object)
{
    M607SetupWork *work;
    s16 nightFlag;
    s32 nightIndex;

    work = object->data;
    if ((work->effectId == -1) && (fn_80071A8C() == 2)) {
        s32 soundIds[2] = { MSM_SE_MG607_ARENA_AMBIENCE_DAY, MSM_SE_MG607_ARENA_AMBIENCE_NIGHT };
        nightFlag = lbl_802C0278;
        if (nightFlag == 0) {
            nightIndex = 0;
        } else {
            nightIndex = 1;
        }
        work->effectId = HuAudFXPlay(soundIds[nightIndex]);
    }
    if ((work->effectId != -1) && ((GameMesStatGet(fn_80071DF4()) & 1) != 0)) {
        HuAudFXFadeOut(work->effectId, 0x1F4);
        work->effectId = -1;
    }
    if ((work->streamId == -1) && ((GameMesStatGet(fn_80071DF4()) & 0x10) != 0)) {
        work->streamId = HuAudSStreamPlay(MSM_STREAM_JAZZY);
    }
    if (fn_80071A8C() == 5) {
        fn_1_35C(object, fn_1_EB4);
        return;
    }
}

void fn_1_EB4(OMOBJ *object)
{
    M607SetupWork *work = object->data;

    if (fn_80071A8C() == 6) {
        M607PlayerWork *firstPlayer = lbl_1_bss_4C[0];
        s32 highestScore = firstPlayer->scoreValue;
        s32 winnerSlots[4] = { -1, -1, -1, -1 };
        M607PlayerWork *candidate;
        M607PlayerWork *newLeader;
        M607PlayerWork *rankedPlayer;
        s32 winnerStatus;
        M607PlayerWork *winner;
        s32 playerIndex;

        for (playerIndex = 1; playerIndex < 4; playerIndex++) {
            candidate = lbl_1_bss_4C[playerIndex];
            if (highestScore < candidate->scoreValue) {
                newLeader = lbl_1_bss_4C[playerIndex];
                highestScore = newLeader->scoreValue;
            }
        }
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            rankedPlayer = lbl_1_bss_4C[playerIndex];
            if (highestScore == rankedPlayer->scoreValue) {
                winnerStatus = 1;
            } else {
                winnerStatus = -1;
            }
            lbl_1_bss_10[playerIndex] = winnerStatus;
            if (highestScore == 0) {
                lbl_1_bss_10[playerIndex] = 0;
            }
        }
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (lbl_1_bss_10[playerIndex] == 1) {
                winner = lbl_1_bss_4C[playerIndex];
                winnerSlots[work->winnerCount++] = winner->characterId;
                if (fn_8006A124(65551) == 0) {
                    lbl_80265750[playerIndex].mgCoinBonus = 10;
                }
            }
        }
        fn_80071C18((s16)winnerSlots[0],
                       (s16)winnerSlots[1],
                       (s16)winnerSlots[2],
                       (s16)winnerSlots[3]);
        if (work->streamId != -1) {
            HuAudSStreamFadeOut(work->streamId, 0x64);
        }
        fn_1_35C(object, fn_1_10C8);
        return;
    }
}

typedef struct M607ObjectManagerWork {
    OMOBJ *object;
    s32 frameCount;
} M607ObjectManagerWork;

void fn_1_10C8(OMOBJ *obj)
{
    void *work = obj->data;

    if (fn_80071A8C() == 8) {
        fn_1_35C(obj, fn_1_1144);
        return;
    }
    if (lbl_1_bss_8 == 6) {
        fn_80071A9C();
        fn_1_35C(obj, fn_1_1144);
        return;
    }
}

void fn_1_1144(OMOBJ *obj)
{
    void *work = obj->data;

    fn_1_35C(obj, NULL);
}

void fn_1_1180(OMOBJ *obj)
{
    M607ObjectManagerWork *work = obj->data;
    s16 night;
    s32 nightSet;
    s32 modelIndex;

    work->object = fn_8003AB38(lbl_1_bss_4, 0x7FDA, 4, 0, -1, NULL);

    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        night = lbl_802C0278;
        if (night == 0) {
            nightSet = 0;
        } else {
            nightSet = 1;
        }
        work->object->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(
            lbl_1_data_28[nightSet][modelIndex], 0x10000000, 2));
    }

    fn_80029850(work->object->mdlId[0], 1);
    fn_80026FFC(work->object->mdlId[1]);
    fn_800269E8(work->object->mdlId[2], (1 << 0));
    fn_800269E8(work->object->mdlId[3], (1 << 0));
    fn_800804E8(&work->object->mdlId[2], 2, 0x80);
    work->frameCount = 0;
    fn_1_35C(obj, fn_1_12CC);
}

void fn_1_12CC(OMOBJ *obj)
{
    M607ObjectManagerWork *work = obj->data;

    work->frameCount++;
    if (work->frameCount == 60) {
        HuAudFXPlay(MSM_SE_MG607_OPENING_BEAT_1);
    } else if (work->frameCount == 120) {
        HuAudFXPlay(MSM_SE_MG607_OPENING_BEAT_2);
    }

    if (fn_80071A8C() == 5) {
        fn_1_35C(obj, NULL);
        return;
    }
}

typedef struct M607CameraWork {
    HU3D_MOTIONID modelMotion;
    HU3D_MODELID camera;
    f32 maxTime;
    u8 unknown08[4]; /* No code names this field. */
    HU3D_MOTIONID cameraMotion;
} M607CameraWork;

void fn_1_1354(OMOBJ *object)
{
    Point3d cameraTarget;
    Point3d cameraPosition;
    Point3d cameraUp;
    M607CameraWork *work;

    work = object->data;
    work->cameraMotion = fn_8002C4C0(fn_80008DD0(DATANUM(0x420000, 12), 0x10000000, 2));
    work->modelMotion = fn_8002C4C0(fn_80008DD0(DATANUM(0x420000, 11), 0x10000000, 2));
    work->camera = fn_800283F4(work->modelMotion, 1U);
    work->maxTime = Hu3DMotionMotionMaxTimeGet(work->modelMotion);
    fn_80027CD0(1, &cameraPosition, &cameraUp, &cameraTarget);
    fn_800262D0(work->camera);
    Hu3DCameraPosSetV(1, &cameraPosition, &cameraUp, &cameraTarget);
    work->camera = fn_800283F4(work->modelMotion, 1U);
    fn_8002D86C(work->camera, 1.0f);
    fn_800284C8(work->camera, 1U);
    fn_1_35C(object, fn_1_144C);
}

void fn_1_144C(OMOBJ *obj)
{
    void *work = obj->data;
    if (fn_80071A8C() == 2) {
        fn_80071B08();
        fn_1_35C(obj, fn_1_14A0);
        return;
    }
}

void fn_1_14A0(OMOBJ *object)
{
    Point3d cameraPosition;
    Point3d cameraUp;
    Point3d cameraTarget;
    Point3d heading;
    M607CameraWork *work = object->data;

    if (fn_8002D75C(work->camera) != 0) {
        f32 magnitudeSquared;
        f32 sqrtResult;
        f32 inverseMagnitude;
        f32 magnitude;

        fn_80027CD0(1, &cameraPosition, &cameraUp, &cameraTarget);
        PSVECSubtract(&cameraTarget, &cameraPosition, &heading);
        magnitudeSquared = (heading.z * heading.z) + ((heading.x * heading.x) + (heading.y * heading.y));
        sqrtResult = sqrtf(magnitudeSquared);
        magnitude = sqrtResult;
        if (magnitude > 1.1920929e-07f) {
            inverseMagnitude = 1.0f / magnitude;
            heading.x = heading.x * inverseMagnitude;
            heading.y = heading.y * inverseMagnitude;
            heading.z = heading.z * inverseMagnitude;
        } else {
            magnitude = 0.0f;
            heading.x = 0.0f;
            heading.y = 0.0f;
            heading.z = 0.0f;
        }
        HuAudFXListnerSet(&cameraPosition, &heading, 3000.0f, 1000.0f);
        lbl_1_bss_C = 1;
        fn_1_35C(object, fn_1_1720);
        return;
    }
}

void fn_1_1720(OMOBJ *object)
{
    void *work;

    work = object->data;
    fn_80071B30();
    fn_1_35C(object, fn_1_1764);
}

void fn_1_1764(OMOBJ *object)
{
    M607CameraWork *work = object->data;

    if (fn_80071A8C() == 7) {
        fn_800262D0(work->camera);
        work->camera = fn_800283F4(work->cameraMotion, 1);
        fn_800284C8(work->camera, 1);
        fn_1_35C(object, fn_1_17DC);
        return;
    }
}

void fn_1_17DC(OMOBJ *object)
{
    M607CameraWork *work;

    work = object->data;
    if (fn_8002D75C(work->camera)) {
        lbl_1_bss_8++;
        fn_1_35C(object, 0);
        return;
    }
}

void fn_1_184C(s32 player, s32 entryIndex)
{
    M607PlayerWork *work;
    M607ItemWork *entry;
    M607PlayerWork *context;
    M607ItemWork *selectedEntry;

    context = lbl_1_bss_4C[player];
    work = context;
    selectedEntry = lbl_1_bss_24[entryIndex];
    entry = selectedEntry;
    work->collisionState = 1;
    work->selectedEntry = entry;
}

void fn_1_18B4(s32 player)
{
    s32 accepted;
    s32 scoreIncrement;
    s32 scoreAmount;
    M607PlayerWork *playerContext;
    M607PlayerWork *work;

    playerContext = lbl_1_bss_4C[player];
    work = playerContext;
    accepted = 1;
    if ((work->selectedEntry->mode != 0) && (work->selectedEntry->mode != 1)) {
        accepted = 0;
    }
    if (accepted != 0) {
        scoreIncrement = 3;
    } else {
        scoreIncrement = 1;
    }
    scoreAmount = scoreIncrement;
    work->collisionState = 2;
    work->scoreValue += scoreAmount;
    if (work->scoreValue > 99) {
        work->scoreValue = 99;
    }
    MgScoreValueSet(work->score, work->scoreValue);
}

s32 fn_1_1974(s32 player)
{
    M607PlayerWork *context;

    context = lbl_1_bss_4C[player];
    return context->collisionState;
}

M607ItemWork *fn_1_19A0(s32 player)
{
    M607PlayerWork *context;

    context = lbl_1_bss_4C[player];
    return context->selectedEntry;
}

extern s32 fn_1_D630(s32 itemIndex);

extern void fn_1_D468(s32 itemIndex, s32 playerIndex);

extern s32 fn_1_84C8(void);

extern void fn_1_8474(s32 playerIndex);

static inline M607PlayerWork *M607PlayerGet(s32 player)
{
    return lbl_1_bss_4C[player];
}

s32 fn_1_19CC(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    Point3d worldPosition;
    Point3d screenPosition;
    s32 itemIndex;
    s32 playerIndex;
    s32 soundPlayerIndex;
    f32 pan;
    s32 selectedMode;
    M607ItemWork *selectedEntry;
    s32 scoreAmount;

    if (b->paramB >= 200 && b->paramB < 210) {
        itemIndex = b->paramB - 200;
        playerIndex = a->paramB;
        if (fn_1_D630(itemIndex) == 2) {
            M607PlayerWork *collisionWork;
            s32 collisionState;
            collisionWork = lbl_1_bss_4C[playerIndex];
            collisionState = collisionWork->collisionState;
            if (!collisionState) {
                M607PlayerWork *selectionWork;
                selectionWork = lbl_1_bss_4C[playerIndex];
                if (!selectionWork->selectedEntry) {
                    M607PlayerWork *selectWork;
                    M607PlayerWork *selectSource;
                    M607ItemWork *entrySource;
                    selectSource = lbl_1_bss_4C[playerIndex];
                    selectWork = selectSource;
                    entrySource = lbl_1_bss_24[itemIndex];
                    selectedEntry = entrySource;
                    selectWork->collisionState = 1;
                    selectWork->selectedEntry = selectedEntry;
                    fn_1_D468(itemIndex, playerIndex);
                }
            }
        }
        return 0;
    }
    if (b->paramB == 100) {
        M607PlayerWork *selectionWork;

        soundPlayerIndex = a->paramB;
        selectionWork = lbl_1_bss_4C[soundPlayerIndex];
        if (selectionWork->selectedEntry && fn_1_84C8() != 0) {
            M607PlayerWork *collisionWork;
            s32 collisionState;
            collisionWork = lbl_1_bss_4C[soundPlayerIndex];
            collisionState = collisionWork->collisionState;
            if (!collisionState) {
                M607PlayerWork *stateWork;
                M607ItemWork *stateEntry;
                M607PlayerWork *scoreSource;
                M607PlayerWork *scoreWork;
                s32 accepted;
                s32 scoreIncrement;
                stateWork = lbl_1_bss_4C[soundPlayerIndex];
                stateEntry = stateWork->selectedEntry;
                /* This sample is retained even though the score path reads the entry again. */
                selectedMode = stateEntry->mode;
                scoreSource = lbl_1_bss_4C[soundPlayerIndex];
                scoreWork = scoreSource;
                accepted = 1;
                if ((scoreWork->selectedEntry->mode != 0) && (scoreWork->selectedEntry->mode != 1)) {
                    accepted = 0;
                }
                if (accepted != 0) {
                    scoreIncrement = 3;
                } else {
                    scoreIncrement = 1;
                }
                scoreAmount = scoreIncrement;
                scoreWork->collisionState = 2;
                scoreWork->scoreValue += scoreAmount;
                if (scoreWork->scoreValue > 99) {
                    scoreWork->scoreValue = 99;
                }
                MgScoreValueSet(scoreWork->score, scoreWork->scoreValue);
                fn_80086BC8(M607PlayerGet(soundPlayerIndex)->player->actor, &worldPosition);
                Hu3D3Dto2D(&worldPosition, 1, &screenPosition);
                pan = 64.0f * (screenPosition.x / 576.0f);
                pan = pan < 0.0f ? 0.0f : (pan > 64.0f ? 64.0f : pan);
                pan += 32.0f;
                if (M607PlayerGet(soundPlayerIndex)->selectedEntry->mode == 0 || M607PlayerGet(soundPlayerIndex)->selectedEntry->mode == 1) {
                    HuAudFXPlayPan(MSM_SE_MG607_LETTER_CATCH_BONUS, (s16)pan);
                } else {
                    HuAudFXPlayPan(MSM_SE_MG607_LETTER_CATCH, (s16)pan);
                }
                fn_1_8474(soundPlayerIndex);
            }
        }
        return 1;
    }
    return 1;
}

extern f32 lbl_1_data_150[];

void fn_1_1D80(M607PlayerWork *work, s32 motionIndex, f32 endTime, u32 attributes)
{
    if (work->motionChangePending != motionIndex) {
        work->motionChangePending = motionIndex;
        if (work->overlayActive != 0) {
            fn_8002CAF0((s16)work->player->actor->mdlId);
            work->overlayActive = 0;
        }
        fn_8005C2C8(work->characterId, work->player->omObj->mtnId[motionIndex],
                           0.0f, endTime, attributes);
        if ((work->selectedEntry != 0) &&
            ((motionIndex == 0) || (motionIndex == 1) || (motionIndex == 2))) {
            fn_8002CAA8((s16)work->player->actor->mdlId,
                                 work->player->omObj->mtnId[5]);
            fn_800269E8((s16)work->player->actor->mdlId, ((1 << 30) | (1 << 9)));
            work->overlayActive = 1;
        }
    }
}

void fn_1_1E90(M607PlayerWork *work, f32 angle, f32 pushDistance)
{
    Point3d targetDirection;
    Point3d actorDirection;
    HuVecF *push;
    f32 targetRotation;
    f32 targetSine;
    f32 targetCosine;
    f32 actorSine;
    f32 actorCosine;
    f32 dotProductResult;
    f32 dotProduct;
    f32 angleDifferenceResult;
    f32 angleDifferenceValue;
    f32 angleDifference;
    f32 crossProduct;
    f32 turnRate;
    f32 clampedDotProduct;
    f32 lowerClampedDotProduct;
    f32 newRotation;

    turnRate = lbl_1_data_150[lbl_80265728[work->playerIndex].comDif];
    targetRotation = (f32)(M_PI *
        (f64)(work->player->actor->rotY / 180.0f));
    targetCosine = (f32)cos((f64)angle);
    targetSine = (f32)sin((f64)angle);
    targetDirection.x = targetSine;
    targetDirection.y = 0.0f;
    targetDirection.z = targetCosine;
    actorCosine = (f32)cos((f64)targetRotation);
    actorSine = (f32)sin((f64)targetRotation);
    actorDirection.x = actorSine;
    actorDirection.y = 0.0f;
    actorDirection.z = actorCosine;
    dotProductResult = PSVECDotProduct(&targetDirection, &actorDirection);
    dotProduct = dotProductResult;
    if (dotProduct > 1.0f) {
        clampedDotProduct = 1.0f;
    } else {
        if (dotProduct < -1.0f) {
            lowerClampedDotProduct = -1.0f;
        } else {
            lowerClampedDotProduct = dotProduct;
        }
        clampedDotProduct = lowerClampedDotProduct;
    }
    angleDifferenceResult = (f32)acos((f64)clampedDotProduct);
    angleDifferenceValue = angleDifferenceResult;
    angleDifference = angleDifferenceValue;
    crossProduct = (targetDirection.x * actorDirection.z) -
                   (targetDirection.z * actorDirection.x);
    newRotation = targetRotation;
    if (crossProduct >= 0.0f) {
        newRotation += angleDifference * turnRate;
    } else if (crossProduct < 0.0f) {
        newRotation -= angleDifference * turnRate;
    }
    while (newRotation > (2.0 * M_PI)) {
        newRotation -= (2.0 * M_PI);
    }
    while (newRotation < -(2.0 * M_PI)) {
        newRotation += (2.0 * M_PI);
    }
    work->player->actor->rotY =
        (f32)((f64)(180.0f * newRotation) / M_PI);
    push = &work->player->actor->push;
    push->x = 0.0f;
    push->y = 0.0f;
    push->z = pushDistance;
}

static inline f32 M607SelectionNormalize(Vec *dst, const Vec *src)
{
    f32 magnitude = fn_1_420(src);
    if (magnitude > 1.1920929e-7f) {
        f32 inverse = 1.0f / magnitude;
        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = 0.0f;
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
    return magnitude;
}

/* Selects a nearby actor with a clear path, allowing flagged actors to take priority. */
s32 fn_1_21F8(M607PlayerWork *source, f32 angleDegrees, M607ItemWork **selectedOut)
{
    f32 candidateAngle;
    Vec sourcePosition;
    f32 facingAngle;
    f32 bestDistance = 10000.0f;
    s32 candidateIndex;
    u32 timerSection;
    s32 playerIndex;
    M607ItemWork *selected = NULL;

    fn_80086BC8(source->player->actor, &sourcePosition);
    facingAngle = (f32)(M_PI *
                        (f64)(source->player->actor->rotY / 180.0f));
    if (lbl_80265728[source->playerIndex].comDif == 3) {
        timerSection = (u32)fn_80071B58() / 60;
        if (((timerSection < 18 && timerSection > 15 &&
              source->selectedEntry == 0) ||
        (timerSection < 8 && timerSection > 5 &&
              source->selectedEntry == 0 && fn_1_9FC(0)->value != 2))) {
            return 0;
        }
    }

    for (candidateIndex = 0; candidateIndex < 10; candidateIndex++) {
        s32 forceCandidate = 0;
        M607ItemWork *candidate = fn_1_9FC(candidateIndex);
        Vec candidatePosition;
        Vec direction;
        Vec facing;
        f32 distance;

        s32 blocked;

        if (candidate == NULL || (candidate->value != 2 && candidate->value != 1)) {
            continue;
        }
        fn_80086BC8(candidate->actor, &candidatePosition);
        candidatePosition.y = 0.0f;
        PSVECSubtract(&candidatePosition, &sourcePosition, &direction);
        distance = M607SelectionNormalize(&direction, &direction);
        facing.x = (f32)sin((f64)facingAngle);
        facing.z = (f32)cos((f64)facingAngle);
        facing.y = 0.0f;
        M607SelectionNormalize(&facing, &facing);
        candidateAngle = fn_1_364(fn_1_570(&facing, &direction));
        if (candidate->flagged != 0 &&
                         (lbl_80265728[source->playerIndex].comDif == 2 ||
                          lbl_80265728[source->playerIndex].comDif == 3)) {
            forceCandidate = 1;
        }
        if (!((candidateAngle < (M_PI *
                                      (f64)angleDegrees / 180.0) &&
               bestDistance > distance) ||
              forceCandidate)) {
            continue;
        }
        blocked = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            M607PlayerWork *player = fn_1_A14(playerIndex);
            Vec otherPosition;
            if (player == source) {
                continue;
            }
            fn_80086BC8(player->player->actor, &otherPosition);
            if (fn_1_884(&otherPosition, &sourcePosition,
                         &candidatePosition) <= 3600.0f) {
                blocked = 1;
                break;
            }
        }
        {
            Vec otherPosition;
            M607RootWork *root = fn_1_A2C();
            fn_80086BC8(root->actor, &otherPosition);
            if (fn_1_884(&otherPosition, &sourcePosition,
                         &candidatePosition) <= 3600.0f) {
                blocked = 1;
            }
        }
        if (blocked) {
            continue;
        }
        bestDistance = distance;
        selected = candidate;
        if (forceCandidate) {
            break;
        }
    }
    if (selected == NULL) {
        return 0;
    }
    if (selectedOut != NULL) {
        *selectedOut = selected;
    }
    return 1;
}

f32 fn_1_2BEC(M607PlayerWork *owner, f32 angle)
{
    Point3d ownerPosition;
    Point3d direction;
    Point3d otherPosition;
    Point3d difference;
    f32 measuredDistance;
    f32 dotProduct;
    f32 distance;
    f32 reciprocalDistance;
    f32 dotProductResult;
    f32 angleDifference;
    f32 crossProduct;
    f32 targetAngle = angle;
    s32 playerIndex;
    M607PlayerWork * other;
    fn_80086BC8(owner->player->actor, & ownerPosition);
    direction.x = (f32) sin(angle);
    direction.z = (f32) cos(angle);
    direction.y = 0.0f;
    for (playerIndex = 0; playerIndex < 4; playerIndex++)
    {
        other = fn_1_A14(playerIndex);
        if (other != owner)
        {
            fn_80086BC8(other->player->actor, & otherPosition);
            PSVECSubtract(& otherPosition, & ownerPosition, & difference);
            distance = fn_1_648(&difference, &difference);
            if (distance < 100.0f)
            {
                dotProduct = fn_1_570(&difference, &direction);
                angleDifference = fn_1_364(dotProduct);
                crossProduct = (direction.x * difference.z) - (direction.z * difference.x);
                if (crossProduct >= 0.0f)
                {
                    targetAngle += 0.5f * angleDifference;
                }
                else
                {
                    targetAngle -= 0.5f * angleDifference;
                }
                if (targetAngle > (2.0 * M_PI) || targetAngle < -(2.0 * M_PI))
                {
                    targetAngle = 0.0f;
                }
                break;
            }
        }
    }
    return targetAngle;
}

extern f32 lbl_1_data_160[4];

extern unsigned int lbl_1_data_48[];

extern HuVecF lbl_1_data_A4[];

extern HuVecF lbl_1_data_D4[];

static inline f32 M607PlayerSetupNormalize(HuVecF *dst, const HuVecF *src)
{
    f32 magnitude2;
    f32 sqrt_result;
    f32 magnitude;

    magnitude2 = (src->z * src->z) + ((src->x * src->x) + (src->y * src->y));
    sqrt_result = sqrtf(magnitude2);
    magnitude = sqrt_result;
    if (magnitude > 1.1920929e-07f) {
        f32 inverse = 1.0f / magnitude;
        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = 0.0f;
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
    return magnitude;
}

static inline void M607PlayerSetupFaceDirection(MGACTOR *actor,
                                                 const HuVecF *direction,
                                                 f32 turnRate, f32 speed)
{
    HuVecF targetFacing;
    HuVecF actorFacing;
    HuVecF *push;
    f32 targetAngle = (f32)atan2((f64)direction->x, (f64)direction->z);
    f32 actorAngle = (f32)(M_PI *
                           (f64)(actor->rotY / 180.0f));
    f32 targetCosine = (f32)cos((f64)targetAngle);
    f32 targetSine = (f32)sin((f64)targetAngle);
    f32 actorCosine = (f32)cos((f64)actorAngle);
    f32 actorSine = (f32)sin((f64)actorAngle);
    f32 dot;
    f32 angleDifference;
    f32 cross;
    f32 nextAngle;

    targetFacing.x = targetSine;
    targetFacing.y = 0.0f;
    targetFacing.z = targetCosine;
    actorFacing.x = actorSine;
    actorFacing.y = 0.0f;
    actorFacing.z = actorCosine;
    dot = PSVECDotProduct(&targetFacing, &actorFacing);
    if (dot > 1.0f) {
        dot = 1.0f;
    } else if (dot < -1.0f) {
        dot = -1.0f;
    }
    angleDifference = (f32)acos((f64)dot);
    cross = targetFacing.x * actorFacing.z -
            targetFacing.z * actorFacing.x;
    nextAngle = actorAngle;
    if (cross >= 0.0f) {
        nextAngle += angleDifference * turnRate;
    } else if (cross < 0.0f) {
        nextAngle -= angleDifference * turnRate;
    }
    while (nextAngle > (2.0 * M_PI)) {
        nextAngle -= (2.0 * M_PI);
    }
    while (nextAngle < -(2.0 * M_PI)) {
        nextAngle += (2.0 * M_PI);
    }
    actor->rotY = (f32)((f64)(180.0f * nextAngle) / M_PI);
    push = &actor->push;
    push->x = 0.0f;
    push->y = 0.0f;
    push->z = speed;
}

/* Updates the player's heading and separates its push direction from nearby players. */
static inline f32 SpeedOf3048(M607PlayerWork *work)
{
    return work->selectionSpeed;
}

static inline f32 M607Player3048Normalize(HuVecF *dst, const HuVecF *src)
{
    f32 magnitude = fn_1_420(src);
    if (magnitude > 1.1920929e-07f) {
        f32 inverse = 1.0f / magnitude;
        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = 0.0f;
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
    return magnitude;
}

static inline f32 M607Player3048Acos(f32 dot)
{
    return (f32)acos((f64)(dot > 1.0f ? 1.0f :
                          (dot < -1.0f ? -1.0f : dot)));
}

static inline void M607Player3048Vector(HuVecF *direction, f32 x, f32 y, f32 z)
{
    direction->x = x;
    direction->y = y;
    direction->z = z;
}

static inline void M607Player3048Face(M607PlayerWork *work, f32 angle,
                                     f32 pushDistance)
{
    HuVecF targetDirection;
    HuVecF actorDirection;
    HuVecF *push;
    f32 targetRotation;
    f32 angleDifference;
    f32 crossProduct;
    f32 turnRate;
    f32 newRotation;

    turnRate = lbl_1_data_150[lbl_80265728[work->playerIndex].comDif];
    targetRotation = (f32)(M_PI *
        (f64)(work->player->actor->rotY / 180.0f));
    M607Player3048Vector(&targetDirection, (f32)sin((f64)angle), 0.0f,
                         (f32)cos((f64)angle));
    M607Player3048Vector(&actorDirection, (f32)sin((f64)targetRotation), 0.0f,
                         (f32)cos((f64)targetRotation));
    angleDifference = M607Player3048Acos(fn_1_570(&targetDirection, &actorDirection));
    crossProduct = targetDirection.x * actorDirection.z -
                   targetDirection.z * actorDirection.x;
    newRotation = targetRotation;
    if (crossProduct >= 0.0f) {
        newRotation += angleDifference * turnRate;
    } else if (crossProduct < 0.0f) {
        newRotation -= angleDifference * turnRate;
    }
    while (newRotation > (2.0 * M_PI)) {
        newRotation -= (2.0 * M_PI);
    }
    while (newRotation < -(2.0 * M_PI)) {
        newRotation += (2.0 * M_PI);
    }
    work->player->actor->rotY =
        (f32)((f64)(180.0f * newRotation) / M_PI);
    push = &work->player->actor->push;
    push->x = 0.0f;
    push->y = 0.0f;
    push->z = pushDistance;
}

void fn_1_3048(M607PlayerWork *work)
{
    HuVecF actorPosition;
    HuVecF direction;
    f32 directionLength;
    f32 targetAngleResult;
    f32 targetAngle;
    f32 actorAngle;
    f32 targetCosine;
    f32 targetSine;
    f32 actorCosine;
    f32 actorSine;
    HuVecF targetFacing;
    HuVecF actorFacing;
    f32 dot;
    f32 limitedDot;
    f32 angleDifferenceResult;
    f32 angleDifference;
    f32 cross;
    f32 nextAngle;
    f32 turnRate;
    s32 playerIndex;
    HuVecF *push;

    fn_80086BC8(work->player->actor, &actorPosition);
    PSVECSubtract(&work->push, &actorPosition, &direction);
    directionLength = M607Player3048Normalize(&direction, &direction);

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        M607PlayerWork *selectedWork = lbl_1_bss_4C[playerIndex];
        M607PlayerWork *otherWork = selectedWork;
        HuVecF otherPosition;
        HuVecF playerSeparation;
        HuVecF otherPushDirection;
        HuVecF normalizedPushDirection;
        f32 distance;

        if (otherWork == work) {
            continue;
        }
        fn_80086BC8(otherWork->player->actor, &otherPosition);
        PSVECSubtract(&otherWork->push, &otherPosition, &otherPushDirection);
        PSVECSubtract(&otherPosition, &actorPosition, &playerSeparation);
        distance = M607Player3048Normalize(&playerSeparation, &playerSeparation);
        if (distance < 100.0f) {
            PSVECScale(&direction, &direction, 10.0f);
            PSVECScale(&playerSeparation, &playerSeparation,
                       0.5f * -(100.0f - distance));
            PSVECAdd(&direction, &playerSeparation, &direction);
            M607Player3048Normalize(&direction, &direction);
        }
    }

    M607Player3048Face(work,
                       atan2f(direction.x, direction.z), work->selectionSpeed);
    if (directionLength < 200.0f) {
        f32 angle = 180.0f * frandf();
        f32 pushDistance = 600.0f * frandf();
        work->push.x = (f32)((f64)pushDistance *
                             sin(((2.0 * M_PI) * (f64)angle) / 180.0));
        work->push.y = 0.0f;
        work->push.z = (f32)((f64)pushDistance *
                             cos(((2.0 * M_PI) * (f64)angle) / 180.0));
    }
}

/* Steers the player toward the shared marker and away from nearby players. */
static inline void applyPushDistance(M607PlayerWork *work, f32 angle)
{
    f32 pushDistance = 600.0f * frandf();
    work->push.x = (f32)((f64)pushDistance *
                         sin(((2.0 * M_PI) * (f64)angle) / 180.0));
    work->push.y = 0.0f;
    work->push.z = (f32)((f64)pushDistance *
                         cos(((2.0 * M_PI) * (f64)angle) / 180.0));
}

void fn_1_3A8C(M607PlayerWork *work)
{
    HuVecF actorPosition;
    HuVecF pushDirection;
    HuVecF markerPosition;
    HuVecF markerDirection;
    f32 markerDistance;
    f32 directionLength;
    f32 targetAngleResult;
    f32 targetAngle;
    f32 actorAngle;
    f32 targetCosine;
    f32 targetSine;
    f32 actorCosine;
    f32 actorSine;
    HuVecF targetFacing;
    HuVecF actorFacing;
    f32 dotResult;
    f32 dot;
    f32 lowerLimitedDot;
    f32 limitedDot;
    f32 angleDifferenceResult;
    f32 angleDifference;
    f32 cross;
    f32 nextAngle;
    f32 turnRate;
    f32 speed;
    s32 playerIndex;
    HuVecF *push;
    M607RootWork *root = lbl_1_bss_20;

    fn_80086BC8(root->actor, &markerPosition);
    fn_80086BC8(work->player->actor, &actorPosition);
    PSVECSubtract(&markerPosition, &actorPosition, &markerDirection);
    M607Player3048Normalize(&markerDirection, &markerDirection);
    PSVECSubtract(&work->push, &actorPosition, &pushDirection);
    directionLength = M607Player3048Normalize(&pushDirection,
                                                &pushDirection);
    PSVECAdd(&pushDirection, &markerDirection, &pushDirection);
    M607Player3048Normalize(&pushDirection, &pushDirection);

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        M607PlayerWork *otherWork = fn_1_A14(playerIndex);
        HuVecF otherPosition;
        HuVecF playerSeparation;
        HuVecF otherPushDirection;
        HuVecF awayDirection;
        f32 otherDistance;

        if (otherWork == work) {
            continue;
        }
        fn_80086BC8(otherWork->player->actor, &otherPosition);
        PSVECSubtract(&otherWork->push, &otherPosition, &otherPushDirection);
        PSVECSubtract(&otherPosition, &actorPosition, &playerSeparation);
        otherDistance = M607Player3048Normalize(&playerSeparation,
                                              &playerSeparation);
        if (otherDistance < 100.0f) {
            PSVECScale(&pushDirection, &pushDirection, 10.0f);
            PSVECScale(&playerSeparation, &playerSeparation,
                       0.5f * -(100.0f - otherDistance));
            PSVECAdd(&pushDirection, &playerSeparation, &pushDirection);
            M607Player3048Normalize(&pushDirection, &pushDirection);
        }
    }

    M607Player3048Face(work,
              atan2f(pushDirection.x, pushDirection.z),
              work->selectionSpeed);
    if (directionLength < 200.0f) {
        f32 angle = 180.0f * frandf();
        f32 pushDistance = 600.0f * frandf();
        work->push.x = (f32)((f64)pushDistance *
                             sin(((2.0 * M_PI) * (f64)angle) / 180.0));
        work->push.y = 0.0f;
        work->push.z = (f32)((f64)pushDistance *
                             cos(((2.0 * M_PI) * (f64)angle) / 180.0));
    }
}

/* Updates the player heading for its computer difficulty and arena phase. */
void fn_1_4878(M607PlayerWork *work)
{
    HuVecF actorPosition;
    HuVecF direction;
    HuVecF targetPosition;
    HuVecF markerPosition;
    HuVecF markerDirection;
    HuVecF markerOffset;
    HuVecF pathPosition;
    HuVecF markerStep;
    HuVecF pathDirection;
    HuVecF fallbackPosition;
    HuVecF fallbackDirection;
    if (lbl_80265728[work->playerIndex].comDif == 0) {
        fn_1_3048(work);
        return;
    }
    fn_80086BC8(work->player->actor, &actorPosition);
    if (work->selectedEntry == 0) {
        M607ItemWork *selection;

        if (fn_1_21F8(work, work->selectionAngle, &selection) != 0) {
            fn_80086BC8(selection->actor, &targetPosition);
            PSVECSubtract(&targetPosition, &actorPosition, &direction);
            M607Player3048Normalize(&direction, &direction);
            {
                f32 targetAngle = atan2f(direction.x, direction.z);
                M607Player3048Face(work, targetAngle,
                          work->selectionSpeed);
            }
            return;
        }
        work->player->actor->rotY += 2.0f +
                       (2.0f * (f32)lbl_80265728[work->playerIndex].comDif);
        {
            M607Player3048Face(work,
                (f32)(M_PI *
                    (f64)(work->player->actor->rotY / 180.0f)),
                0.5f * lbl_1_data_160[lbl_80265728[work->playerIndex].comDif]);
        }
        return;
    }
    if (lbl_80265728[work->playerIndex].comDif == 1) {
        fn_1_3A8C(work);
        return;
    }
    if (lbl_80265728[work->playerIndex].comDif == 3) {
        M607RootWork *selectedRoot = lbl_1_bss_20;
        M607RootWork *root = selectedRoot;

        fn_80086BC8(root->actor, &markerPosition);
        if (root->phase == 1) {
            PSVECSubtract(&markerPosition, &actorPosition, &markerDirection);
            M607Player3048Normalize(&markerDirection, &markerDirection);
            {
                M607Player3048Face(work,
                    atan2f(markerDirection.x, markerDirection.z),
                    work->selectionSpeed);
            }
            return;
        }
        PSVECSubtract(&markerPosition, &actorPosition, &markerOffset);
        {
            f32 markerDistance = M607Player3048Normalize(&markerOffset,
                                                           &markerOffset);
            f32 markerAngle = (f32)(M_PI *
                                    (f64)(root->actor->rotY / 180.0f));
            markerStep.x = (f32)sin((f64)markerAngle);
            markerStep.y = 0.0f;
            markerStep.z = (f32)cos((f64)markerAngle);
            PSVECScale(&markerStep, &markerStep, markerDistance);
            PSVECAdd(&markerPosition, &markerStep, &pathPosition);
            PSVECSubtract(&pathPosition, &actorPosition, &pathDirection);
            M607Player3048Normalize(&pathDirection, &pathDirection);
        }
        {
            M607Player3048Face(work,
                atan2f(pathDirection.x, pathDirection.z), work->selectionSpeed);
        }
        return;
    }
    {
        M607RootWork *fallbackRoot = lbl_1_bss_20;
        fn_80086BC8(fallbackRoot->actor, &fallbackPosition);
    }
    PSVECSubtract(&fallbackPosition, &actorPosition, &fallbackDirection);
    M607Player3048Normalize(&fallbackDirection, &fallbackDirection);
    {
        M607Player3048Face(work,
            atan2f(fallbackDirection.x, fallbackDirection.z), work->selectionSpeed);
    }
}

const MGACTOR_PARAM lbl_1_rodata_138 = { 0.0f, 0.0f, 0, 0, NULL, 0, 0, NULL };

void fn_1_6318(OMOBJ *obj)
{
    M607PlayerWork *work = obj->data;
    MGACTOR_PARAM actorParam;
    f32 angle;
    f32 pushDistance;

    actorParam = lbl_1_rodata_138;
    work->initialized = 0;
    work->characterId = lbl_80265728[work->playerIndex].charNo;
    work->collisionState = 0;
    work->selectedEntry = NULL;

    actorParam.height = 150.0f;
    actorParam.radius = 40.0f;
    actorParam.param = work->playerIndex;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.correctHookParam = 0;
    actorParam.narrowHook = (COL_NARROW_HOOK)fn_1_19CC;
    actorParam.correctHook = NULL;
    work->player = fn_80080728(work->playerIndex, &actorParam, 2, 1, 1,
                                  lbl_1_data_48);
    fn_8007E818(work->player);
    memcpy(work->motionIds, work->player->omObj->mtnId, sizeof(work->motionIds));
    fn_80026E44(work->player->actor->mdlId);
    fn_80086AA8(work->player->actor, &lbl_1_data_A4[work->positionIndex]);

    if (work->motionChangePending != 0) {
        work->motionChangePending = 0;
        if (work->overlayActive != 0) {
            fn_8002CAF0(work->player->actor->mdlId);
            work->overlayActive = 0;
        }
        fn_8005C2C8(work->characterId, work->player->omObj->mtnId[0],
                           0.0f, 4.0f,
                           ((1 << 30) | (1 << 0)));
        if (work->selectedEntry) {
            fn_8002CAA8(work->player->actor->mdlId,
                                 work->player->omObj->mtnId[5]);
            fn_800269E8(work->player->actor->mdlId, ((1 << 30) | (1 << 9)));
            work->overlayActive = 1;
        }
    }
    fn_800269E8(work->player->actor->mdlId, (1 << 0));
    work->scoreBox = MgScoreBoxCreateChar(0x60, 0x30, work->characterId);
    MgScoreBoxPosSet(work->scoreBox, lbl_1_data_D4[work->positionIndex].x,
                     lbl_1_data_D4[work->positionIndex].y);
    MgScoreBoxDispSet(work->scoreBox, 0);
    work->score = MgScoreCreate(DATANUM(0x9b0000, 49), -1, 1);
    MgScoreMaxDigitSet(work->score, 2);
    MgScoreDispOff(work->score);
    MgScorePosSet(work->score,
                  7.0f + lbl_1_data_D4[work->positionIndex].x,
                  lbl_1_data_D4[work->positionIndex].y);
    MgScoreColorSet(work->score, 0xFF, 0xD8, 0x15);
    MgScoreValueSet(work->score, 0);
    angle = 180.0f * frandf();
    pushDistance = 300.0f * frandf();
    work->push.x = pushDistance * sin((M_PI * (f64)angle) /
                                             180.0);
    work->push.y = 0.0f;
    work->push.z = pushDistance * cos((M_PI * (f64)angle) /
                                             180.0);
    fn_1_35C(obj, fn_1_6728);
}

void fn_1_6728(OMOBJ *object)
{
    M607PlayerWork *work;

    work = object->data;
    if (fn_80071A8C() == 2) {
        work->initialized = 0;
        fn_1_35C(object, fn_1_6784);
        return;
    }
}

void fn_1_6784(OMOBJ *object)
{
    M607PlayerWork *work = object->data;

    work->initialized++;
    if (work->initialized == 0xF0) {
        fn_80026A44((s16)work->player->actor->mdlId, (1 << 0));
        return;
    }
    if (fn_80071A8C() == 5) {
        MgScoreBoxDispSet(work->scoreBox, 1);
        MgScoreDispOn(work->score);
        work->initialized = 0;
        fn_1_35C(object, fn_1_6828);
        return;
    }
}

extern f32 lbl_1_data_140[];

void fn_1_6828(OMOBJ *object)
{
    M607PlayerWork *work;

    work = object->data;
    if (lbl_80265728[work->playerIndex].type != 0) {
        work->selectionAngle = lbl_1_data_140[lbl_80265728[work->playerIndex].comDif];
        work->selectionSpeed = lbl_1_data_160[lbl_80265728[work->playerIndex].comDif];
        fn_1_35C(object, fn_1_69D4);
        return;
    }
    fn_8007E888(work->player);
    fn_1_35C(object, fn_1_6908);
}

/* static */

void fn_1_6908(OMOBJ *object)
{
    M607PlayerWork *work;

    work = object->data;
    if (fn_80071A8C() == 6) {
        fn_8007E818(work->player);
        fn_800805CC(work->player->actor, 4U);
        fn_1_35C(object, fn_1_74DC);
        return;
    }
    if (work->collisionState == 1) {
        fn_8007E818(work->player);
        work->initialized = 0;
        fn_1_35C(object, fn_1_6A90);
        return;
    }
    if (work->collisionState == 2) {
        fn_8007E818(work->player);
        work->initialized = 0;
        fn_1_35C(object, fn_1_6FC4);
        return;
    }
}

/* static */

void fn_1_69D4(OMOBJ *object)
{
    M607PlayerWork *work;

    work = object->data;
    if (fn_80071A8C() == 6) {
        fn_800805CC(work->player->actor, 4U);
        fn_1_35C(object, fn_1_74DC);
        return;
    }
    if (work->collisionState == 1) {
        work->initialized = 0;
        fn_1_35C(object, fn_1_6A90);
        return;
    }
    if (work->collisionState == 2) {
        work->initialized = 0;
        fn_1_35C(object, fn_1_6FC4);
        return;
    }
    fn_1_4878(work);
}

/* Faces the player toward the selectedEntry actor and advances the turn until the selectedEntry resumes. */
void fn_1_6A90(OMOBJ *obj)
{
    M607PlayerWork * work = obj->data;
    Vec facing;
    Vec playerPosition;
    Vec opponentPosition;
    Vec * push;
    f32 facingX;
    f32 facingZ;
    f32 vectorMagnitude;
    f32 angle;
    f32 rotationRadians;
    fn_1_1D80(work, 3, 4.0f, ((1 << 30) | (1 << 0)));
    if (work->initialized == 0)
    {
        fn_80086BC8(work->player->actor, & playerPosition);
        fn_80086BC8(work->selectedEntry->actor, & opponentPosition);
        PSVECSubtract(& opponentPosition, & playerPosition, & facing);
        {
            f32 inverseMagnitude;
            f32 magnitude;

            vectorMagnitude = sqrtf(facing.z * facing.z +
                                   (facing.x * facing.x + facing.y * facing.y));
            magnitude = vectorMagnitude;
            if (magnitude > 1.1920929e-07f)
            {
                inverseMagnitude = 1.0f / magnitude;
                facing.x *= inverseMagnitude;
                facing.y *= inverseMagnitude;
                facing.z *= inverseMagnitude;
            }
            else
            {
                magnitude = 0.0f;
                facing.x = 0.0f;
                facing.y = 0.0f;
                facing.z = 0.0f;
            }
        }
        facingZ = facing.z;
        facingX = facing.x;
        angle = (f32)atan2((f64)facingX, (f64)facingZ);
        rotationRadians = angle;
        work->player->actor->rotY = (f32)((f64)(180.0f * rotationRadians) /
                                          M_PI);
    }
    push = & work->player->actor->push;
    push->x = 0.0f;
    push->y = 0.0f;
    push->z = 0.0f;
    work->initialized++;
    if (work->initialized == 0xF)
    {
        work->selectedEntry->value = 3;
        fn_8006A030(work->playerIndex, 0xC, 7, 3);
        return;
    }
    if (Hu3DMotionMotionMaxTimeGet(work->player->omObj->mtnId[3]) <= (f32) work->initialized)
    {
        work->collisionState = 0;
        if (work->motionChangePending != 0)
        {
            work->motionChangePending = 0;
            if (work->overlayActive != 0)
            {
                fn_8002CAF0((s16) work->player->actor->mdlId);
                work->overlayActive = 0;
            }
            fn_8005C2C8(work->characterId, work->player->omObj->mtnId[0], 0.0f, 0.0f, ((1 << 30) | (1 << 0)));
            if (work->selectedEntry)
            {
                fn_8002CAA8((s16) work->player->actor->mdlId, work->player->omObj->mtnId[5]);
                fn_800269E8((s16) work->player->actor->mdlId, ((1 << 30) | (1 << 9)));
                work->overlayActive = 1;
            }
        }
        if (0 != lbl_80265728[work->playerIndex].type)
        {
            fn_1_35C(obj, fn_1_69D4);
            return;
        }
        fn_8007E888(work->player);
        fn_1_35C(obj, fn_1_6908);
        return;
    }
}

extern void fn_1_D4C4(s32 player);

void fn_1_6FC4(OMOBJ *object)
{
    M607PlayerWork *work = object->data;
    Point3d direction;
    Point3d playerPosition;
    Point3d targetPosition;
    f32 facingX;
    f32 facingZ;
    f32 vectorMagnitude;
    f32 angle;
    f32 rotationRadians;
    s32 mode;
    M607RootWork *target;
    M607RootWork *targetCopy;

    work->motionChangePending = -1;
    fn_1_1D80(work, 0, 4.0f, ((1 << 30) | (1 << 0)));

    target = lbl_1_bss_20;
    targetCopy = target;
    fn_80086BC8(work->player->actor, &playerPosition);
    fn_80086BC8(targetCopy->actor, &targetPosition);
    PSVECSubtract(&targetPosition, &playerPosition, &direction);
    {
        f32 magnitudeSquared;
        f32 inverseMagnitude;
        f32 magnitude;

        magnitudeSquared = (direction.z * direction.z) +
                           ((direction.x * direction.x) + (direction.y * direction.y));
        vectorMagnitude = sqrtf(magnitudeSquared);
        magnitude = vectorMagnitude;
        if (magnitude > 1.1920929e-07f) {
            inverseMagnitude = 1.0f / magnitude;
            direction.x = direction.x * inverseMagnitude;
            direction.y = direction.y * inverseMagnitude;
            direction.z = direction.z * inverseMagnitude;
        } else {
            magnitude = 0.0f;
            direction.x = 0.0f;
            direction.y = 0.0f;
            direction.z = 0.0f;
        }
    }
    facingZ = direction.z;
    facingX = direction.x;
    angle = (f32)atan2((f64)facingX, (f64)facingZ);
    rotationRadians = angle;
    work->player->actor->rotY = (f32)((f64)(180.0f * rotationRadians) /
                                      M_PI);

    work->initialized++;
    if (work->initialized == 0xF) {
        mode = work->selectedEntry->mode;
        fn_1_D4C4(mode);
        fn_8006A030(work->playerIndex, 0x14, 7, 3);
        return;
    }
    if (work->initialized >= 0x1E) {
        work->selectedEntry = NULL;
        work->collisionState = 0;
        work->motionChangePending = -1;
        fn_1_1D80(work, 0, 4.0f, ((1 << 30) | (1 << 0)));
        if (lbl_80265728[work->playerIndex].type != 0) {
            fn_1_35C(object, fn_1_69D4);
            return;
        }
        fn_8007E888(work->player);
        fn_1_35C(object, fn_1_6908);
        return;
    }
}

void fn_1_74DC(OMOBJ *object)
{
    M607PlayerWork *work;

    work = object->data;
    if (work->selectedEntry) {
        fn_800269E8(work->selectedEntry->model->mdlId[0], (1 << 0));
        work->selectedEntry = NULL;
        work->motionChangePending = -1;
        if (work->motionChangePending != 0) {
            work->motionChangePending = 0;
            if (work->overlayActive != 0) {
                fn_8002CAF0((s16)work->player->actor->mdlId);
                work->overlayActive = 0;
            }
            fn_8005C2C8(work->characterId, work->player->omObj->mtnId[0],
                               0.0f, 4.0f, ((1 << 30) | (1 << 0)));
            if (work->selectedEntry) {
                fn_8002CAA8((s16)work->player->actor->mdlId,
                                     work->player->omObj->mtnId[5]);
                fn_800269E8((s16)work->player->actor->mdlId, ((1 << 30) | (1 << 9)));
                work->overlayActive = 1;
            }
        }
    }
    fn_1_35C(object, fn_1_761C);
}

void fn_1_761C(OMOBJ *object)
{
    M607PlayerWork *work;

    work = object->data;
    if (fn_80071A8C() == 7) {
        fn_8007E818(work->player);
        if (work->motionChangePending != 0) {
            work->motionChangePending = 0;
            if (work->overlayActive != 0) {
                fn_8002CAF0(work->player->actor->mdlId);
                work->overlayActive = 0;
            }
            fn_8005C2C8(work->characterId, *work->player->omObj->mtnId,
                               0.0f, 4.0f, ((1 << 30) | (1 << 0)));
            if (work->selectedEntry != 0) {
                fn_8002CAA8(work->player->actor->mdlId,
                                     work->player->omObj->mtnId[5]);
                fn_800269E8(work->player->actor->mdlId, ((1 << 30) | (1 << 9)));
                work->overlayActive = 1;
            }
        }
        fn_1_35C(object, fn_1_7740);
        return;
    }
}

extern HuVecF lbl_1_data_110[4];

static inline void M607PlayerMotionVector(HuVecF *direction, f32 x, f32 y, f32 z)
{
    direction->x = x;
    direction->y = y;
    direction->z = z;
}

static inline f32 M607PlayerMotionAcos(f32 dot)
{
    return (f32)acos((f64)(dot > 1.0f ? 1.0f :
                          (dot < -1.0f ? -1.0f : dot)));
}

static inline void M607PlayerMotionFace(M607PlayerWork *work, f32 angle,
                             f32 pushDistance)
{
    Point3d targetDirection;
    Point3d actorDirection;
    HuVecF *push;
    f32 targetRotation;
    f32 targetSine;
    f32 targetCosine;
    f32 actorSine;
    f32 actorCosine;
    f32 dotProductResult;
    f32 dotProduct;
    f32 angleDifferenceResult;
    f32 angleDifferenceValue;
    f32 angleDifference;
    f32 crossProduct;
    f32 turnRate;
    f32 clampedDotProduct;
    f32 lowerClampedDotProduct;
    f32 newRotation;

    targetRotation = (f32)(M_PI *
        (f64)(work->player->actor->rotY / 180.0f));
    targetCosine = (f32)cos((f64)angle);
    targetSine = (f32)sin((f64)angle);
    targetDirection.x = targetSine;
    targetDirection.y = 0.0f;
    targetDirection.z = targetCosine;
    actorCosine = (f32)cos((f64)targetRotation);
    actorSine = (f32)sin((f64)targetRotation);
    actorDirection.x = actorSine;
    actorDirection.y = 0.0f;
    actorDirection.z = actorCosine;
    angleDifference = M607PlayerMotionAcos(fn_1_570(&targetDirection, &actorDirection));
    crossProduct = (targetDirection.x * actorDirection.z) -
                   (targetDirection.z * actorDirection.x);
    newRotation = targetRotation;
    if (crossProduct >= 0.0f) {
        newRotation += 0.2f * angleDifference;
    } else if (crossProduct < 0.0f) {
        newRotation -= 0.2f * angleDifference;
    }
    while (newRotation > (2.0 * M_PI)) {
        newRotation -= (2.0 * M_PI);
    }
    while (newRotation < -(2.0 * M_PI)) {
        newRotation += (2.0 * M_PI);
    }
    work->player->actor->rotY =
        (f32)((f64)(180.0f * newRotation) / M_PI);
    push = &work->player->actor->push;
    push->x = 0.0f;
    push->y = 0.0f;
    push->z = pushDistance;
}

static inline f32 M607PlayerMotionNormalize(HuVecF *dst, const HuVecF *src)
{
    f32 magnitude;

    magnitude = fn_1_420(src);
    if (magnitude > 1.1920929e-7f) {
        f32 inverse = 1.0f / magnitude;
        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = 0.0f;
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
    return magnitude;
}

/* Steers the player toward its assigned position while avoiding closer players. */
static inline void PSVECSubtractInline(const Vec*a0, const Vec*a1, Vec*a2)
{
    PSVECSubtract(a0, a1, a2);
}

void fn_1_7740(OMOBJ *obj)
{
    M607PlayerWork *work = obj->data;
    HuVecF targetPosition = lbl_1_data_110[work->positionIndex];
    HuVecF actorPosition;
    HuVecF direction;
    HuVecF otherPosition;
    HuVecF awayDirection;
    HuVecF targetToOther;
    HuVecF targetDirection;
    HuVecF actorDirection;
    f32 speed;
    f32 separationDistance;
    f32 otherDistance;
    f32 targetAngle;
    f32 angleDifference;
    f32 crossProduct;
    f32 directionLength;
    f32 targetRotation;
    f32 newRotation;
    f32 targetCosine;
    f32 targetSine;
    f32 actorCosine;
    f32 actorSine;
    HuVecF *push;
    s32 playerIndex;

    if (lbl_1_bss_10[work->playerIndex] == 1) {
        targetPosition.z = 200.0f;
    }
    fn_80086BC8(work->player->actor, &actorPosition);
    PSVECSubtract(&targetPosition, &actorPosition, &direction);
    directionLength = M607PlayerMotionNormalize(&direction, &direction);
    if (directionLength < 30.0f) {
        work->player->actor->rotY = 0.0f;
        lbl_1_bss_8++;
        fn_80086B04(work->player->actor, &targetPosition);
        fn_1_35C(obj, fn_1_82D4);
        return;
    }

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        M607PlayerWork *otherWork = fn_1_A14(playerIndex);

        if (otherWork == work) {
            continue;
        }
        fn_80086BC8(otherWork->player->actor, &otherPosition);
        PSVECSubtractInline(&lbl_1_data_110[work->positionIndex], &otherPosition,
                      &targetToOther);
        PSVECSubtract(&otherPosition, &actorPosition, &awayDirection);
        otherDistance = fn_1_420(&awayDirection);
        if (!(otherDistance > directionLength)) {
            separationDistance = M607PlayerMotionNormalize(&awayDirection, &awayDirection);
            if (separationDistance < 100.0f) {
                PSVECScale(&direction, &direction, 10.0f);
                PSVECScale(&awayDirection, &awayDirection,
                           0.5f * -(100.0f - separationDistance));
                PSVECAdd(&direction, &awayDirection, &direction);
                M607PlayerMotionNormalize(&direction, &direction);
            }
        }
    }

    targetAngle = atan2f(direction.x, direction.z);
    targetRotation = (f32)(M_PI *
        (f64)(work->player->actor->rotY / 180.0f));
    M607PlayerMotionVector(&targetDirection, (f32)sin((f64)targetAngle), 0.0f,
                          (f32)cos((f64)targetAngle));
    M607PlayerMotionVector(&actorDirection, (f32)sin((f64)targetRotation), 0.0f,
                          (f32)cos((f64)targetRotation));
    angleDifference = M607PlayerMotionAcos(fn_1_570(&targetDirection, &actorDirection));
    crossProduct = (targetDirection.x * actorDirection.z) -
                   (targetDirection.z * actorDirection.x);
    newRotation = targetRotation;
    if (crossProduct >= 0.0f) {
        newRotation += 0.2f * angleDifference;
    } else if (crossProduct < 0.0f) {
        newRotation -= 0.2f * angleDifference;
    }
    while (newRotation > (2.0 * M_PI)) {
        newRotation -= (2.0 * M_PI);
    }
    while (newRotation < -(2.0 * M_PI)) {
        newRotation += (2.0 * M_PI);
    }
    work->player->actor->rotY =
        (f32)((f64)(180.0f * newRotation) / M_PI);
    speed = 10.0f > directionLength ? directionLength : 10.0f;
    push = &work->player->actor->push;
    push->x = 0.0f;
    push->y = 0.0f;
    push->z = speed;
}

void fn_1_82D4(OMOBJ *obj)
{
    void *work = obj->data;
    if (fn_80071A8C() == 8) {
        fn_1_35C(obj, fn_1_8324);
        return;
    }
}

void fn_1_8324(OMOBJ *obj)
{
    M607PlayerWork *work = obj->data;

    if (lbl_1_bss_10[work->playerIndex] == 1) {
        if (work->motionChangePending != 6) {
            work->motionChangePending = 6;
            if (work->overlayActive != 0) {
                fn_8002CAF0((s16)work->player->actor->mdlId);
                work->overlayActive = 0;
            }
            fn_8005C2C8(work->characterId,
                               work->player->omObj->mtnId[6],
                               0.0f, 4.0f, 0);
            if (work->selectedEntry != 0U) {
                return;
            }
        }
    } else if (work->motionChangePending != 7) {
        work->motionChangePending = 7;
        if (work->overlayActive != 0) {
            fn_8002CAF0((s16)work->player->actor->mdlId);
            work->overlayActive = 0;
        }
        fn_8005C2C8(work->characterId,
                           work->player->omObj->mtnId[7],
                           0.0f, 4.0f, 0);
        if (work->selectedEntry == 0U) {
            return;
        }
    }
}

void fn_1_8474(s32 player)
{
    M607RootWork *state = lbl_1_bss_20;
    M607RootWork *work = state;
    void *playerData;

    work->phase = 1;
    playerData = lbl_1_bss_4C[player];
    work->playerData = playerData;
}

s32 fn_1_84C8(void) {
    return lbl_1_bss_20->phase == 0;
}

void fn_1_84E4(M607RootWork *state, s32 motionIndex, f32 endTime, u32 attributes)
{
    if (state->motionIndex != motionIndex) {
        state->motionIndex = motionIndex;
        fn_8002CCC0(state->model->mdlId[0], state->model->mtnId[motionIndex],
                           0.0f, endTime, attributes);
    }
}

int fn_1_8564(COL_NARROW_PARAM *first, COL_NARROW_PARAM *second) {
    if (second->paramB >= 200)
        return 0;
    return 1;
}

s32 fn_1_8580(M607RootWork *object, f32 radius, s32 *playerIndices)
{
    Point3d objectPosition;
    Point3d playerPosition;
    Point3d difference;
    s32 playerIndex;
    s32 foundCount = 0;

    fn_80086BC8(object->actor, &objectPosition);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        f32 distance;

        fn_80086BC8(lbl_1_bss_4C[playerIndex]->player->actor, &playerPosition);
        PSVECSubtract(&playerPosition, &objectPosition, &difference);
        distance = sqrtf(difference.z * difference.z +
                         (difference.x * difference.x + difference.y * difference.y));
        if (distance <= (40.0f + radius)) {
            playerIndices[foundCount] = playerIndex;
            foundCount++;
        }
    }
    return foundCount;
}

s32 fn_1_8774(M607RootWork *object, f32 radius, s32 *playerIndices)
{
    Point3d sourcePosition;
    Point3d facingDirection;
    s32 nearbyIndices[4];
    Point3d candidatePosition;
    Point3d candidateDifference;
    f32 sourceAngle;
    f32 facingSine;
    f32 facingCosine;
    s32 nearbyCount;
    s32 acceptedCount;
    s32 acceptedIndex;
    s32 nearbyIndex;

    sourceAngle = (f32)((M_PI *
                         (f64)object->actor->rotY) / 180.0);
    acceptedCount = 0;
    facingCosine = (f32)cos((f64)sourceAngle);
    facingSine = (f32)sin((f64)sourceAngle);
    facingDirection.x = facingSine;
    facingDirection.y = 0.0f;
    facingDirection.z = facingCosine;
    fn_80086BC8(object->actor, &sourcePosition);
    nearbyCount = fn_1_8580(object, radius, nearbyIndices);

    nearbyIndex = 0;
    while (nearbyIndex < nearbyCount) {
        acceptedIndex = nearbyIndices[nearbyIndex];
        fn_80086BC8(lbl_1_bss_4C[nearbyIndex]->player->actor,
                      &candidatePosition);
        PSVECSubtract(&candidatePosition, &sourcePosition, &candidateDifference);
        fn_1_648(&candidateDifference, &candidateDifference);

        if (fn_1_570(&candidateDifference, &facingDirection) > 0.75f) {
            playerIndices[acceptedCount] = acceptedIndex;
            acceptedCount++;
        }
        nearbyIndex++;
    }

    return acceptedCount;
}

void fn_1_8C64(M607RootWork *work, f32 angle,
                             f32 pushDistance, f32 turnRate)
{
    Point3d targetDirection;
    Point3d actorDirection;
    HuVecF *push;
    f32 targetRotation;
    f32 angleDifference;
    f32 crossProduct;
    f32 newRotation;

    targetRotation = (f32)(M_PI *
        (f64)(work->actor->rotY / 180.0f));
    fn_1_410(&targetDirection, (f32)sin((f64)angle), 0.0f, (f32)cos((f64)angle));
    fn_1_410(&actorDirection, (f32)sin((f64)targetRotation), 0.0f, (f32)cos((f64)targetRotation));
    angleDifference = fn_1_364(fn_1_570(&targetDirection, &actorDirection));
    crossProduct = (targetDirection.x * actorDirection.z) -
                   (targetDirection.z * actorDirection.x);
    newRotation = targetRotation;
    if (crossProduct >= 0.0f) {
        newRotation += angleDifference * turnRate;
    } else {
        if (crossProduct < 0.0f) {
            newRotation -= angleDifference * turnRate;
        }
    }
    while (newRotation > (2.0 * M_PI)) {
        newRotation -= (2.0 * M_PI);
    }
    while (newRotation < -(2.0 * M_PI)) {
        newRotation += (2.0 * M_PI);
    }
    work->actor->rotY = (f32)((f64)(180.0f * newRotation) /
                              M_PI);
    push = &work->actor->push;
    push->x = 0.0f;
    push->y = 0.0f;
    push->z = pushDistance;
    fn_8002D86C(work->model->mdlId[0],
                       lbl_1_rodata_84[work->motionIndex]);
}

void fn_1_8FC8(M607RootWork *object, const HuVecF *sourcePosition)
{
    HuVecF offset;
    HuVecF position = *sourcePosition;

    position.y += 150.0f;
    offset.x = (f32)sin((M_PI *
                         (f64)(180.0f + object->actor->rotY)) /
                        180.0);
    offset.y = 0.8f;
    offset.z = (f32)cos((M_PI *
                         (f64)(180.0f + object->actor->rotY)) /
                        180.0);
    fn_800608C4(1, &position, &offset, 1.0f);
}

extern s32 lbl_1_data_6C[4];

extern char lbl_1_data_170[];

const MGACTOR_PARAM lbl_1_rodata_17C = { 0.0f, 0.0f, 0, 0, NULL, 0, 0, NULL };

void fn_1_90E0(OMOBJ *object)
{
    M607RootWork *work = object->data;
    MGACTOR_PARAM actorParam = lbl_1_rodata_17C;
    s32 motionIndex;

    work->phase = 0;
    work->model = fn_8003AB38(lbl_1_bss_4, 0x28, 1, 4, -1, NULL);
    work->model->mdlId[0] = fn_80025A04(
        fn_80008DD0(DATANUM(0x420000, 0), 0x10000000, 2));
    omSetStatBit(work->model, 0x100);

    for (motionIndex = 0; motionIndex < 4; motionIndex++) {
        work->model->mtnId[motionIndex] = fn_80030458(
            work->model->mdlId[0],
            fn_80008DD0(lbl_1_data_6C[motionIndex],
                                 0x10000000, 2));
    }

    if (work->motionIndex != 1) {
        work->motionIndex = 1;
        fn_8002CCC0(work->model->mdlId[0],
                           work->model->mtnId[1],
                           0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }

    actorParam.height = 150.0f;
    actorParam.radius = 40.0f;
    actorParam.param = 0x64;
    actorParam.type = 6;
    actorParam.narrowHook = fn_1_8564;
    actorParam.attr = 0;
    actorParam.correctHookParam = 0;
    actorParam.correctHook = NULL;
    work->actor = fn_80080B04(&actorParam, work->model->mdlId[0]);
    work->actor->gravity = 100.0f;

    work->characterModel = fn_8003AB38(lbl_1_bss_4, 0x28, 1, 1, -1, NULL);
    work->characterModel->mdlId[0] = fn_80025A04(
        fn_80008DD0(DATANUM(0x420000, 25), 0x10000000, 2));
    work->characterModel->mtnId[0] = fn_80030458(
        work->characterModel->mdlId[0],
        fn_80008DD0(DATANUM(0x420000, 25), 0x10000000, 2));
    fn_8002D86C(work->characterModel->mdlId[0], 1.0f);
    fn_8002741C(work->characterModel->mdlId[0], lbl_1_data_170,
                     work->model->mdlId[0]);
    fn_80026E44(work->model->mdlId[0]);
    fn_80026E44(work->characterModel->mdlId[0]);
    fn_1_35C(object, fn_1_93D8);
}

static inline void M607TurnCryEffect(M607RootWork *work, const Point3d *sourcePosition)
{
    Point3d direction;
    Point3d position = *sourcePosition;

    position.y += 150.0f;
    direction.x = (f32)sin((M_PI *
                             (f64)(180.0f + work->actor->rotY)) /
                            180.0);
    direction.y = 0.8f;
    direction.z = (f32)cos((M_PI *
                             (f64)(180.0f + work->actor->rotY)) /
                            180.0);
    fn_800608C4(1, &position, &direction, 1.0f);
}

void fn_1_93D8(OMOBJ *object)
{
    M607RootWork *work = object->data;
    Point3d modelPosition;
    Point3d effectPosition;
    Point3d direction;

    Hu3DModelObjPosGet(*work->characterModel->mdlId, lbl_1_data_170,
                       &modelPosition);
    effectPosition = modelPosition;
    effectPosition.y += 150.0f;
    direction.x = (f32)sin((M_PI *
                            (180.0f + work->actor->rotY)) /
                           180.0);
    direction.y = 0.8f;
    direction.z = (f32)cos((M_PI *
                            (180.0f + work->actor->rotY)) /
                           180.0);
    fn_800608C4(1, &effectPosition, &direction, 1.0f);
    fn_8002D86C(*work->model->mdlId,
                       lbl_1_rodata_84[work->motionIndex]);
    work->frames++;
    if (fn_8002D75C(*work->characterModel->mdlId) != 0) {
        fn_80027508(*work->characterModel->mdlId);
        fn_800269E8(*work->model->mdlId, (1 << 0));
        fn_1_35C(object, fn_1_9590);
        return;
    }
}

const HuVecF lbl_1_rodata_19C = { 300.0f, 0.0f, 0.0f };

void fn_1_9590(OMOBJ *obj)
{
    M607RootWork *work = obj->data;
    HuVecF position;

    work->frames++;
    if (work->frames == 0xFA) {
        fn_80026A44(work->model->mdlId[0], (1 << 0));
        position = lbl_1_rodata_19C;
        fn_80086AA8(work->actor, &position);
        work->actor->rotY = 360.0f * frandf();
        fn_1_35C(obj, fn_1_964C);
        return;
    }
}

static inline f32 M607EnemyApproachNormalize(Vec *dst, const Vec *src)
{
    f32 sqrtResult;
    f32 magnitude;

    sqrtResult = sqrtf((src->z * src->z) +
                                       ((src->x * src->x) +
                                        (src->y * src->y)));
    magnitude = sqrtResult;
    if (magnitude > 1.1920929e-07f) {
        f32 inverse = 1.0f / magnitude;
        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = 0.0f;
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
    return magnitude;
}

static inline f32 M607EnemyReturnAngleDifference(f32 dot)
{
    f32 lowerClampedDot;
    f32 clampedDot;
    if (dot > 1.0f) {
        clampedDot = 1.0f;
    } else {
        if (dot < -1.0f) {
            lowerClampedDot = -1.0f;
        } else {
            lowerClampedDot = dot;
        }
        clampedDot = lowerClampedDot;
    }
    return (f32)acos((f64)clampedDot);
}

static inline void M607ReturnVector(HuVecF *dst, f32 x, f32 y, f32 z)
{
    dst->x = x; dst->y = y; dst->z = z;
}

static inline f32 M607ReturnAcos(f32 dot)
{
    return (f32)acos((f64)(dot > 1.0f ? 1.0f :
                          (dot < -1.0f ? -1.0f : dot)));
}

static inline void M607EnemyReturnFace(M607RootWork *work,
                                       f32 angle, f32 pushDistance,
                                       f32 turnRate)
{
    Point3d targetDirection;
    Point3d actorDirection;
    HuVecF *push;
    f32 targetRotation;
    f32 angleDifference;
    f32 crossProduct;
    f32 newRotation;

    targetRotation = (f32)(M_PI *
        (f64)(work->actor->rotY / 180.0f));
    M607ReturnVector(&targetDirection, (f32)sin((f64)angle), 0.0f, (f32)cos((f64)angle));
    M607ReturnVector(&actorDirection, (f32)sin((f64)targetRotation), 0.0f, (f32)cos((f64)targetRotation));
    angleDifference = M607ReturnAcos(fn_1_570(&targetDirection, &actorDirection));
    crossProduct = (targetDirection.x * actorDirection.z) -
                  (targetDirection.z * actorDirection.x);
    newRotation = targetRotation;
    if (crossProduct >= 0.0f) {
        newRotation += angleDifference * turnRate;
    } else {
        if (crossProduct < 0.0f) {
            newRotation -= angleDifference * turnRate;
        }
    }
    while (newRotation > (2.0 * M_PI)) {
        newRotation -= (2.0 * M_PI);
    }
    while (newRotation < -(2.0 * M_PI)) {
        newRotation += (2.0 * M_PI);
    }
    work->actor->rotY = (f32)((f64)(180.0f * newRotation) /
                              M_PI);
    push = &work->actor->push;
    push->x = 0.0f;
    push->y = 0.0f;
    push->z = pushDistance;
    fn_8002D86C(work->model->mdlId[0],
                       lbl_1_rodata_84[work->motionIndex]);
}

static inline f32 M607UnusedAngleDifference(f32 dot)
{
    f32 clampedDot;
    f32 lowerClampedDot;

    if (dot > 1.0f) {
        clampedDot = 1.0f;
    } else {
        if (dot < -1.0f) {
            lowerClampedDot = -1.0f;
        } else {
            lowerClampedDot = dot;
        }
        clampedDot = lowerClampedDot;
    }
    return (f32)acos((f64)clampedDot);
}

static inline void M607UnusedFace(M607RootWork *work, f32 angle,
                             f32 pushDistance, f32 turnRate)
{
    Point3d targetDirection;
    Point3d actorDirection;
    HuVecF *push;
    f32 targetRotation;
    f32 targetSine;
    f32 targetCosine;
    f32 actorSine;
    f32 actorCosine;
    f32 dotProductResult;
    f32 dotProduct;
    f32 angleDifferenceResult;
    f32 angleDifferenceValue;
    f32 angleDifference;
    f32 crossProduct;
    f32 newRotation;

    targetRotation = (f32)(M_PI *
        (f64)(work->actor->rotY / 180.0f));
    targetCosine = (f32)cos((f64)angle);
    targetSine = (f32)sin((f64)angle);
    targetDirection.x = targetSine;
    targetDirection.y = 0.0f;
    targetDirection.z = targetCosine;
    actorCosine = (f32)cos((f64)targetRotation);
    actorSine = (f32)sin((f64)targetRotation);
    actorDirection.x = actorSine;
    actorDirection.y = 0.0f;
    actorDirection.z = actorCosine;
    dotProductResult = PSVECDotProduct(&targetDirection, &actorDirection);
    dotProduct = dotProductResult;
    angleDifferenceResult = M607UnusedAngleDifference(dotProduct);
    angleDifferenceValue = angleDifferenceResult;
    angleDifference = angleDifferenceValue;
    crossProduct = (targetDirection.x * actorDirection.z) -
                   (targetDirection.z * actorDirection.x);
    newRotation = targetRotation;
    if (crossProduct >= 0.0f) {
        newRotation += angleDifference * turnRate;
    } else {
        if (crossProduct < 0.0f) {
            newRotation -= angleDifference * turnRate;
        }
    }
    while (newRotation > (2.0 * M_PI)) {
        newRotation -= (2.0 * M_PI);
    }
    while (newRotation < -(2.0 * M_PI)) {
        newRotation += (2.0 * M_PI);
    }
    work->actor->rotY = (f32)((f64)(180.0f * newRotation) /
                              M_PI);
    push = &work->actor->push;
    push->x = 0.0f;
    push->y = 0.0f;
    push->z = pushDistance;
    fn_8002D86C(work->model->mdlId[0],
                       lbl_1_rodata_84[work->motionIndex]);
}

/* Turns away from the arena marker and plays the return movement effect. */
static inline void M607EnemyReturnCry(M607RootWork *object,
                                      const HuVecF *sourcePosition)
{
    HuVecF offset;
    HuVecF position = *sourcePosition;

    position.y += 150.0f;
    offset.x = (f32)sin((M_PI *
                         (f64)(180.0f + object->actor->rotY)) /
                        180.0);
    offset.y = 0.8f;
    offset.z = (f32)cos((M_PI *
                         (f64)(180.0f + object->actor->rotY)) /
                        180.0);
    fn_800608C4(1, &position, &offset, 1.0f);
}

static inline f32 M607ReturnNormalize(HuVecF *dst, const HuVecF *src)
{
    f32 magnitude = fn_1_420(src);
    if (magnitude > 1.1920929e-07f) {
        f32 inverse = 1.0f / magnitude;
        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = 0.0f;
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
    return magnitude;
}

const Vec lbl_1_rodata_1AC = { 0.0f, 0.0f, 0.0f };

void fn_1_964C(OMOBJ *obj)
{
    M607RootWork *work = obj->data;
    Vec markerPosition = lbl_1_rodata_1AC;
    Vec scaledDirection;
    Vec actorPosition;
    Vec direction;
    Vec movementDirection;
    f32 movementDistance;
    if (fn_80071A8C() == 6) {
        fn_1_35C(obj, fn_1_C004);
        return;
    }
    fn_80086BC8(work->actor, &actorPosition);
    PSVECSubtract(&actorPosition, &markerPosition, &direction);
    M607ReturnNormalize(&direction, &direction);
    PSVECScale(&direction, &scaledDirection, 500.0f);
    PSVECSubtract(&scaledDirection, &actorPosition, &movementDirection);
    movementDistance = M607ReturnNormalize(&movementDirection, &movementDirection);

    M607EnemyReturnFace(work,
                        atan2f(movementDirection.x, movementDirection.z),
                        12.0f,
                        0.1f);

    M607EnemyReturnCry(work, &actorPosition);
    if (movementDistance <= 10.0f) {
        work->turnDirection = ((rand8() % 2) == 0) ? 1 : -1;
        work->frames = 0;
        fn_1_35C(obj, fn_1_9F24);
        return;
    }
    if (work->phase == 1) {
        work->frames = 0;
        fn_1_35C(obj, fn_1_B998);
        return;
    }
}

/* Turns the enemy around the arena marker and selects its next movement state. */
const Vec lbl_1_rodata_1C4 = { 0.0f, 0.0f, 0.0f };

void fn_1_9F24(OMOBJ *obj)
{
    M607RootWork *work = obj->data;
    Vec markerPosition = lbl_1_rodata_1C4;
    Vec actorPosition;
    Vec direction;
    Vec crossDirection;
    Vec originalCrossDirection;
    Vec turnDirection;
    f32 distance;
    f32 targetAngle;
    s32 nearbyPlayers[4];

    if (fn_80071A8C() == 6) {
        fn_1_35C(obj, fn_1_C004);
        return;
    }

    fn_80086BC8(work->actor, &actorPosition);
    PSVECSubtract(&markerPosition, &actorPosition, &direction);
    distance = M607ReturnNormalize(&direction, &direction);

    M607ReturnVector(&turnDirection, 0.0f,
                     (f32)work->turnDirection, 0.0f);
    PSVECCrossProduct(&turnDirection, &direction, &crossDirection);
    originalCrossDirection = crossDirection;
    PSVECScale(&direction, &direction, (500.0f - distance) / 12.0f);
    PSVECScale(&direction, &direction, -1.0f);
    PSVECAdd(&crossDirection, &direction, &crossDirection);
    M607ReturnNormalize(&crossDirection, &crossDirection);
    targetAngle = atan2f(crossDirection.x, crossDirection.z);
    fn_1_8C64(work, targetAngle,
                          12.0f, 0.1f);

    fn_1_8FC8(work, &actorPosition);

    if ((fn_1_8774(work, 200.0f, nearbyPlayers) != 0) &&
        ((rand8() % 20) == 0)) {
        s32 randomChoice = rand8() % 3;
        if (randomChoice == 0) {
            work->turnDirection = -work->turnDirection;
            work->frames = 0;
            work->currentAngle = atan2f(-originalCrossDirection.x,
                                        -originalCrossDirection.z);
            fn_1_35C(obj, fn_1_AD60);
            return;
        }
        if (randomChoice == 1) {
            PSVECSubtract(&markerPosition, &actorPosition, &work->direction);
            M607ReturnNormalize(&work->direction, &work->direction);
            work->frames = 0;
            fn_1_35C(obj, fn_1_B248);
            return;
        }
    } else {
        if (work->frames++ >= 0x3C) {
            s32 randomChoice = rand8() % 3;
            work->frames = 0;
            if (randomChoice == 0) {
                work->turnDirection = -work->turnDirection;
                work->currentAngle = atan2f(-originalCrossDirection.x,
                                            -originalCrossDirection.z);
                fn_1_35C(obj, fn_1_AD60);
                return;
            }
            if (randomChoice == 1) {
                PSVECSubtract(&markerPosition, &actorPosition,
                              &work->direction);
                M607ReturnNormalize(&work->direction,
                                            &work->direction);
                work->frames = 0;
                fn_1_35C(obj, fn_1_B248);
                return;
            }
        }
    }

    if (work->phase == 1) {
        work->frames = 0;
        fn_1_35C(obj, fn_1_B998);
        return;
    }
}

/* Turns the actor toward the arena position and advances its approach effect. */
void fn_1_AD60(OMOBJ *obj)
{
    M607RootWork *work = obj->data;
    Point3d actorPosition;

    if (fn_80071A8C() == 6) {
        fn_1_35C(obj, fn_1_C004);
        return;
    }

    fn_80086BC8(work->actor, &actorPosition);
    M607EnemyReturnFace(work, work->currentAngle, 0.0f, 0.1f);

    M607TurnCryEffect(work, &actorPosition);

    if (work->frames++ >= 0xA) {
        work->frames = 0;
        fn_1_35C(obj, fn_1_9F24);
        return;
    }
    if (work->phase == 1) {
        work->frames = 0;
        fn_1_35C(obj, fn_1_B998);
        return;
    }
}

const Vec lbl_1_rodata_1D0 = { 0.0f, 0.0f, 0.0f };

void fn_1_B248(OMOBJ *obj)
{
    M607RootWork *work = obj->data;
    Vec targetPosition = lbl_1_rodata_1D0;
    Vec actorPosition;
    Vec direction;
    s32 nearbyPlayers[4];
    f32 distance;
    f32 targetAngle;

    if (fn_80071A8C() == 6) {
        fn_1_35C(obj, fn_1_C004);
        return;
    }
    fn_80086BC8(work->actor, &actorPosition);
    PSVECSubtract(&targetPosition, &actorPosition, &direction);
    distance = M607ReturnNormalize(&direction, &direction);
    targetAngle = atan2f(direction.x, direction.z);
    fn_1_8C64(work, targetAngle,
              12.0f, 0.1f);

    fn_1_8FC8(work, &actorPosition);
    if (fn_1_8774(work, 200.0f, nearbyPlayers) != 0) {
        if ((rand8() % 0x14) == 0) {
            fn_1_35C(obj, fn_1_964C);
            return;
        }
    }
    if (fn_1_570(&work->direction, &direction) < 0.0f) {
        fn_1_35C(obj, fn_1_964C);
        return;
    }
    if (work->phase == 1) {
        work->frames = 0;
        fn_1_35C(obj, fn_1_B998);
        return;
    }
}

extern void fn_1_D59C(s32 managerIndex);

static inline f32 M607EnemyTurnLength(Vec *dst, const Vec *src)
{
    f32 magnitude;

    magnitude = fn_1_420(src);
    if (magnitude > 1.1920929e-07f) {
        f32 inverse = 1.0f / magnitude;
        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = 0.0f;
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
    return magnitude;
}

static inline f32 M607EnemyTurnAcos(f32 dot)
{
    return (f32)acos((f64)(dot > 1.0f ? 1.0f :
                          (dot < -1.0f ? -1.0f : dot)));
}

static inline void M607EnemyTurnDirection(HuVecF *direction, f32 x, f32 y, f32 z)
{
    direction->x = x;
    direction->y = y;
    direction->z = z;
}

static inline void M607EnemyTurnFace(M607RootWork *work, f32 angle,
                                    f32 pushDistance, f32 turnRate)
{
    Point3d targetDirection;
    Point3d actorDirection;
    HuVecF *push;
    f32 targetRotation;
    f32 angleDifference;
    f32 crossProduct;
    f32 newRotation;

    targetRotation = (f32)(M_PI *
        (f64)(work->actor->rotY / 180.0f));
    M607EnemyTurnDirection(&targetDirection, (f32)sin((f64)angle), 0.0f,
                           (f32)cos((f64)angle));
    M607EnemyTurnDirection(&actorDirection, (f32)sin((f64)targetRotation),
                           0.0f, (f32)cos((f64)targetRotation));
    angleDifference = M607EnemyTurnAcos(fn_1_570(&targetDirection, &actorDirection));
    crossProduct = (targetDirection.x * actorDirection.z) -
                   (targetDirection.z * actorDirection.x);
    newRotation = targetRotation;
    if (crossProduct >= 0.0f) {
        newRotation += angleDifference * turnRate;
    } else if (crossProduct < 0.0f) {
        newRotation -= angleDifference * turnRate;
    }
    while (newRotation > (2.0 * M_PI)) {
        newRotation -= (2.0 * M_PI);
    }
    while (newRotation < -(2.0 * M_PI)) {
        newRotation += (2.0 * M_PI);
    }
    work->actor->rotY = (f32)((f64)(180.0f * newRotation) / M_PI);
    push = &work->actor->push;
    push->x = 0.0f;
    push->y = 0.0f;
    push->z = pushDistance;
    fn_8002D86C(work->model->mdlId[0],
                       lbl_1_rodata_84[work->motionIndex]);
}

/* Turns the actor toward its target and selects the next motion after the turn. */

void fn_1_B998(OMOBJ *obj)
{
    M607RootWork * work = obj->data;
    Vec direction;
    Vec targetPosition;
    Vec actorPosition;
    if (fn_80071A8C() == 6)
    {
        fn_1_35C(obj, fn_1_C004);
        return;
    }
    if (2 != work->motionIndex)
    {
        work->motionIndex = 2;
        fn_8002CCC0(* work->model->mdlId, work->model->mtnId[2], 0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    fn_80086BC8(work->actor, & actorPosition);
    fn_80086BC8(work->playerData->player->actor, & targetPosition);
    PSVECSubtract(& targetPosition, & actorPosition, & direction);
    M607EnemyTurnLength(& direction, & direction);
    M607EnemyTurnFace(work, atan2f(direction.x, direction.z),
              0.0f, 0.2f);
    if (work->frames++ >= 0x1E)
    {
        work->phase = 0;
        if (work->motionIndex != 1)
        {
            work->motionIndex = 1;
            fn_8002CCC0(* work->model->mdlId, work->model->mtnId[1], 0.0f, 4.0f, ((1 << 30) | (1 << 0)));
        }
        fn_1_D59C(work->activeEntry->mode);
        fn_1_35C(obj, fn_1_964C);
        return;
    }
}

void fn_1_C004(OMOBJ *obj)
{
    M607RootWork *work = obj->data;

    if (work->motionIndex != 0) {
        work->motionIndex = 0;
        fn_8002CCC0(work->model->mdlId[0], work->model->mtnId[0],
                           0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    if (fn_80071A8C() == 7) {
        fn_1_35C(obj, fn_1_C0A8);
        return;
    }
}

extern Point3d lbl_1_data_104;

static inline f32 M607C0A8Normalize(Point3d *dst, const Point3d *src)
{
    f32 sqrtResult;
    f32 magnitude;

    magnitude = fn_1_420(src);
    if (magnitude > 1.1920929e-07f) {
        f32 inverse = 1.0f / magnitude;
        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = 0.0f;
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
    return magnitude;
}

static inline void M607C0A8PlayerPosGet(M607PlayerWork *player, Point3d *position)
{
    fn_80086BC8(player->player->actor, position);
}

static inline f32 M607C0A8Acos(f32 dot)
{
    return (f32)acos((f64)(dot > 1.0f ? 1.0f :
                          (dot < -1.0f ? -1.0f : dot)));
}

void fn_1_C0A8(OMOBJ *obj)
{
    M607RootWork *work = obj->data;
    Point3d actorPosition;
    Point3d markerDirection;
    Point3d playerPosition;
    Point3d playerDirection;
    Point3d targetDirection;
    Point3d actorDirection;
    f32 squaredLength;
    f32 distance;
    f32 speed;
    f32 reciprocalDistance;
    f32 playerSquaredLength;
    f32 playerDistance;
    f32 playerReciprocalDistance;
    f32 combinedSquaredLength;
    f32 combinedDistance;
    f32 combinedReciprocalDistance;
    f32 targetAngleResult;
    f32 targetAngle;
    f32 actorAngle;
    f32 targetCosine;
    f32 targetSine;
    f32 actorCosine;
    f32 actorSine;
    f32 dotProductResult;
    f32 dotProduct;
    f32 clampedDot;
    f32 lowerClampedDot;
    f32 upperClampedDot;
    f32 angleDifferenceResult;
    f32 angleDifferenceValue;
    f32 angleDifference;
    f32 crossProduct;
    f32 newAngle;
    s32 playerIndex;
    HuVecF *push;

    fn_80086BC8(work->actor, &actorPosition);
    PSVECSubtract(&lbl_1_data_104, &actorPosition, &markerDirection);
    distance = M607C0A8Normalize(&markerDirection, &markerDirection);
    if (distance < 30.0f) {
        lbl_1_bss_8++;
        fn_80086B04(work->actor, &lbl_1_data_104);
        fn_1_35C(obj, fn_1_CACC);
        return;
    }

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        M607C0A8PlayerPosGet(fn_1_A14(playerIndex), &playerPosition);
        PSVECSubtract(&playerPosition, &actorPosition, &playerDirection);
        playerDistance = M607C0A8Normalize(&playerDirection, &playerDirection);
        if (playerDistance < 100.0f) {
            PSVECScale(&markerDirection, &markerDirection, 10.0f);
            PSVECScale(&playerDirection, &playerDirection,
                       -(100.0f - playerDistance));
            PSVECAdd(&markerDirection, &playerDirection, &markerDirection);
            M607C0A8Normalize(&markerDirection, &markerDirection);
        }
    }

    speed = 9.0f > distance ? distance : 9.0f;
    if (work->motionIndex != 1) {
        work->motionIndex = 1;
        fn_8002CCC0(work->model->mdlId[0],
                           work->model->mtnId[1], 0.0f, 4.0f,
                           ((1 << 30) | (1 << 0)));
    }

    fn_1_8C64(work,
              atan2f(markerDirection.x, markerDirection.z), speed, 0.3f);
}

void fn_1_CACC(OMOBJ *obj)
{
    M607RootWork *work = obj->data;

    if (work->motionIndex != 3) {
        work->motionIndex = 3;
        fn_8002CCC0(work->model->mdlId[0],
                           work->model->mtnId[3], 0.0f,
                           4.0f, ((1 << 30) | (1 << 0)));
    }
    M607EnemyReturnFace(work, 0.0f, 0.0f, 0.1f);
}

typedef struct M607SceneSetupWork {
    Vec positions[256];
    s32 positionCount;
    s32 positionASelected;
    s32 positionBSelected;
} M607SceneSetupWork;

const Vec lbl_1_rodata_1F0 = { -500.0f, 0.0f, -500.0f };

void fn_1_CEA8(OMOBJ *obj)
{
    M607SceneSetupWork *work = obj->data;
    Vec point;
    s32 outerIndex;
    s32 innerIndex;
    void *resource;

    point = lbl_1_rodata_1F0;
    work->positionASelected = work->positionBSelected = 0;
    work->positionCount = 0;
    for (outerIndex = 0; outerIndex < 16; outerIndex++) {
        point.x = -500.0f;
        for (innerIndex = 0; innerIndex < 16; innerIndex++) {
            f32 magnitudeSquared = (point.z * point.z) + ((point.x * point.x) + (point.y * point.y));
            f32 magnitude = sqrtf(magnitudeSquared);

            if (magnitude < 500.0f) {
                work->positions[work->positionCount] = point;
                work->positionCount++;
            }
            point.x += 62.5f;
        }
        point.z += 62.5f;
    }
    resource = fn_80008634(DATANUM(0x210000, 0), 0x10000000);
    for (outerIndex = 0; outerIndex < 10; outerIndex++) {
        lbl_1_bss_24[outerIndex] = fn_1_2BC(0x14, 0x3C, fn_1_D83C);
        lbl_1_bss_24[outerIndex]->mode = outerIndex;
        lbl_1_bss_24[outerIndex]->resource = resource;
    }
    fn_1_35C(obj, fn_1_D16C);
}

void fn_1_D16C(OMOBJ *obj)
{
    void *work = obj->data;
    if (lbl_1_bss_C != 0) {
        fn_1_35C(obj, fn_1_D1C4);
        return;
    }
}

void fn_1_D1C4(OMOBJ *obj)
{
    M607SceneSetupWork *work = obj->data;
    u32 timerSection = (u32)fn_80071B58() / 60;
    s32 mode;
    s32 randomPosition;
    s32 positionAIndex;
    s32 positionBIndex;

    mode = 2;
    while (mode < 10) {
        if (lbl_1_bss_24[mode]->value == 0) {
            randomPosition = rand8() % work->positionCount;
            lbl_1_bss_24[mode]->value = 1;
            lbl_1_bss_24[mode]->position.x = work->positions[randomPosition].x;
            lbl_1_bss_24[mode]->position.y = 1000.0f;
            lbl_1_bss_24[mode]->position.z = work->positions[randomPosition].z;
            break;
        }
        mode++;
    }
    if ((work->positionASelected == 0) && (timerSection == 15)) {
        positionAIndex = rand8() % work->positionCount;
        work->positionASelected = 1;
        lbl_1_bss_24[0]->value = 1;
        lbl_1_bss_24[0]->position.x = work->positions[positionAIndex].x;
        lbl_1_bss_24[0]->position.y = 1000.0f;
        lbl_1_bss_24[0]->position.z = work->positions[positionAIndex].z;
    } else if ((work->positionBSelected == 0) && (timerSection == 5)) {
        positionBIndex = rand8() % work->positionCount;
        work->positionBSelected = 1;
        lbl_1_bss_24[1]->value = 1;
        lbl_1_bss_24[1]->position.x = work->positions[positionBIndex].x;
        lbl_1_bss_24[1]->position.y = 1000.0f;
        lbl_1_bss_24[1]->position.z = work->positions[positionBIndex].z;
    }
    if ((u32)fn_80071A8C() == 6U) {
        for (mode = 0; mode < 10; mode++) {
            fn_800269E8(lbl_1_bss_24[mode]->model->mdlId[0], (1 << 0));
        }
        fn_1_35C(obj, fn_1_D464);
        return;
    }
}

extern s32 lbl_1_data_F18;

extern float lbl_1_bss_1BC;

extern s8 lbl_1_bss_218;

extern s8 lbl_1_bss_219;

extern s8 lbl_1_data_2758;

s32 fn_1_367CC(s32 *player);

void fn_1_D464(OMOBJ *object)
{
}

extern char lbl_1_data_183[];

void fn_1_D468(s32 managerIndex, s32 playerIndex)
{
    M607ItemWork *entry = lbl_1_bss_24[managerIndex];
    M607ItemWork *work = entry;
    void *playerContext;

    playerContext = lbl_1_bss_4C[playerIndex];
    work->playerContext = playerContext;
    work->value = 4;
}

void fn_1_D4C4(s32 player)
{
    M607ItemWork *work;
    M607PlayerWork *hookRoot;
    M607RootWork *state;
    M607RootWork *activeRoot;

    work = fn_1_9FC(player);
    hookRoot = work->playerContext;
    fn_80027508((s16)hookRoot->player->actor->mdlId);
    fn_80026760(work->model->mdlId[0], 0.0f, 0.0f,
                    0.0f);
    state = lbl_1_bss_20;
    fn_8002741C(state->model->mdlId[0], lbl_1_data_183, work->model->mdlId[0]);
    activeRoot = lbl_1_bss_20;
    activeRoot->activeEntry = work;
}

void fn_1_D59C(s32 managerIndex)
{
    M607ItemWork *manager;
    M607ItemWork *work;
    M607RootWork *root;

    manager = lbl_1_bss_24[managerIndex];
    work = manager;
    work->playerContext = 0;
    work->value = 0;
    root = lbl_1_bss_20;
    fn_80027508(root->model->mdlId[0]);
    fn_800269E8(work->model->mdlId[0], (1 << 0));
}

s32 fn_1_D630(s32 player)
{
    M607ItemWork *entry;

    entry = lbl_1_bss_24[player];
    return entry->value;
}

int fn_1_D65C(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    return 0;
}

void fn_1_D664(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    HU3D_PARTICLE_DATA *data = particle->data;
    s32 i;
    s32 alpha;

    for (i = 0; i < particle->maxCnt; i++, data++) {
        if (data->time == 0) {
            data->time++;
            data->vel.x = (8.0f * frandf()) - 4.0f;
            data->vel.y = 2.0f * frandf();
            data->vel.z = (8.0f * frandf()) - 4.0f;
            data->pos.x += (8.0f * frandf()) - 4.0f;
            data->pos.z += (8.0f * frandf()) - 4.0f;
            data->color.r = 0xFF;
            data->color.g = 0x80;
            data->color.b = 0x80;
            data->color.a = 0x80;
            data->scale = 5.0f;
        }
        if (data->time == 1) {
            PSVECAdd(&data->pos, &data->vel, &data->pos);
            data->scale *= 1.02f;
            alpha = data->color.a - 2;
            if (alpha < 0) {
                alpha = 0;
                data->scale = 0.0f;
                data->time = -1;
            }
            data->color.a = alpha;
        }
    }
}

extern s32 lbl_1_data_7C[5];

extern s32 lbl_1_data_90[5];

extern int fn_1_D65C(COL_NARROW_PARAM *, COL_NARROW_PARAM *);

extern void fn_1_D664(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

const MGACTOR_PARAM lbl_1_rodata_210 = { 0.0f, 0.0f, 0, 0, NULL, 0, 0, NULL };

void fn_1_D83C(OMOBJ *object)
{
    M607ItemWork *work = object->data;
    MGACTOR_PARAM actorParam = lbl_1_rodata_210;
    HU3D_PARTICLE *hookData;
    s32 motionIndex;
    HU3D_PARTICLE_DATA *particle;
    s32 isStandardState;
    s32 hasStandardState;
    s32 modelResource;
    s32 motionResource;
    s32 particleIndex;

    work->model = fn_8003AB38(lbl_1_bss_4, 0x28, 2, 5, -1, NULL);
    omSetStatBit(work->model, 0x100U);

    isStandardState = 1;
    if (work->mode != 0 && work->mode != 1) {
        isStandardState = 0;
    }
    if (isStandardState != 0) {
        hasStandardState = 1;
    } else {
        hasStandardState = 0;
    }
    work->flagged = hasStandardState;
    if (work->flagged != 0) {
        modelResource = DATANUM(0x420000, 14);
    } else {
        modelResource = DATANUM(0x420000, 13);
    }

    work->model->mdlId[0] = fn_80025A04(
        fn_80008DD0(modelResource, 0x10000000, 2));
    work->model->mdlId[1] = fn_8003216C(
        HuSprAnimRead(work->resource), 0x100);
    hookData = lbl_802C01C8[work->model->mdlId[1]].hookData;
    fn_8003269C(work->model->mdlId[1], 1U);
    fn_800326D0(work->model->mdlId[1], fn_1_D664);
    fn_80026B8C(work->model->mdlId[1], 5);
    fn_80026760(work->model->mdlId[1], 0.0f,
                    0.0f, 0.0f);

    particle = hookData->data;
    particleIndex = 0;
    while (particleIndex < hookData->maxCnt) {
        particle->time = -1;
        particle->scale = 0.0f;
        particleIndex++;
        particle++;
    }

    actorParam.height = 10.0f;
    actorParam.radius = sqrtf(92.0f);
    actorParam.param = work->mode + 0xC8;
    actorParam.type = 7;
    actorParam.narrowHook = fn_1_D65C;
    actorParam.attr = 0;
    actorParam.correctHookParam = 0;
    actorParam.correctHook = NULL;
    work->actor = fn_80080B04(&actorParam, work->model->mdlId[0]);
    work->actor->gravity = 1.0f;

    for (motionIndex = 0; motionIndex < 5; motionIndex++) {
        if (work->flagged != 0) {
            motionResource = lbl_1_data_90[motionIndex];
        } else {
            motionResource = lbl_1_data_7C[motionIndex];
        }
        work->model->mtnId[motionIndex] = fn_80030458(
            work->model->mdlId[0],
            fn_80008DD0(motionResource, 0x10000000, 2));
    }

    fn_8002CCC0(work->model->mdlId[0], work->model->mtnId[1],
                       0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    fn_80026E44((s16)work->actor->mdlId);
    work->value = 0;
    fn_80086F44(work->actor);
    fn_800269E8(work->model->mdlId[0], (1 << 0));
    work->frame = 0;
    work->emitterId = -1;
    fn_1_35C(object, fn_1_DCCC);
}

void fn_1_DCCC(OMOBJ *obj)
{
    M607ItemWork *work = obj->data;

    if (fn_80071A8C() == 6) {
        fn_1_35C(obj, fn_1_E544);
        return;
    }
    if (work->value == 1) {
        work->actor->rotY = 360.0f * frandf();
        work->actor->velY = 0.0f;
        fn_80086FA0(work->actor, &work->position);
        fn_80026A44(*work->model->mdlId, (1 << 0));
        work->rotation = ((rand8() % 2) == 0) ? 1.0f : -1.0f;
        work->horizontalSpeed = frandf();
        work->verticalSpeed = (8.0f * frandf()) - 4.0f;
        if (work->flagged != 0) {
            work->emitterId =
                HuAudFXEmiterPlay(MSM_SE_MG607_LETTER_CHARACTER_KNOCKED, &work->position);
        }
        fn_1_35C(obj, fn_1_DE2C);
        return;
    }
}

/* Advances the falling character's bounce motion and updates its emitter and particles. */
static inline void M607FallPush(M607ItemWork *work)
{
    Vec *push;
    f32 pushY;
    f32 pushX;

    pushY = 2.0f +
            (15.0f * -fabsf(work->horizontalSpeed));
    pushX = 10.0f * work->horizontalSpeed;
    push = &work->actor->push;
    push->x = pushX;
    push->y = pushY;
    push->z = 0.0f;
}

void fn_1_DE2C(OMOBJ *obj)
{
    M607ItemWork *work = obj->data;
    Vec actorPosition;
    HU3D_PARTICLE *particleModel;
    HU3D_PARTICLE_DATA *particle;
    s16 mdlId;
    s32 entryIndex;
    s32 particleCount;

    if (fn_80071A8C() == 6) {
        if ((work->frame % 2) == 0) {
            fn_800269E8(*work->model->mdlId, (1 << 0));
        } else {
            fn_80026A44(*work->model->mdlId, (1 << 0));
        }
        if (work->frame++ >= 0x5A) {
            fn_800269E8(*work->model->mdlId, (1 << 0));
            fn_1_35C(obj, 0);
            return;
        }
    }

    work->horizontalSpeed += (work->rotation - work->horizontalSpeed) / 15.0f;
    if (fabsf(work->horizontalSpeed - work->rotation) < 0.01f) {
        work->rotation = -work->rotation;
    }
    M607FallPush(work);
    work->actor->rotY += work->verticalSpeed;

    if (work->horizontalSpeed > 0.0f) {
        f32 maxMotionTime;
        f32 productTime;

        fn_8002C95C(*work->model->mdlId, work->model->mtnId[2]);
        maxMotionTime = fn_8002D530(*work->model->mdlId);
        productTime = maxMotionTime * fabsf(work->horizontalSpeed);
        productTime = (productTime > maxMotionTime) ? productTime
                      : ((productTime < 0.0f) ? 0.0f : productTime);
        fn_8002D414(*work->model->mdlId, productTime);
    } else {
        f32 maxMotionTime;
        f32 productTime;

        fn_8002C95C(*work->model->mdlId, work->model->mtnId[3]);
        maxMotionTime = fn_8002D530(*work->model->mdlId);
        productTime = maxMotionTime * fabsf(work->horizontalSpeed);
        productTime = (productTime > maxMotionTime) ? productTime
                      : ((productTime < 0.0f) ? 0.0f : productTime);
        fn_8002D414(*work->model->mdlId, productTime);
    }

    fn_80086BC8(work->actor, &actorPosition);
    if (actorPosition.y <= 10.0f) {
        work->value = 2;
    fn_8002C95C(*work->model->mdlId,
                      work->model->mtnId[0]);
        fn_1_35C(obj, fn_1_E34C);
        return;
    }
    if (work->flagged != 0) {
        /* The emitter uses the actor pointer and following state words as coordinates. */
        HuAudFXEmiterUpDate(work->emitterId, (Vec *)&work->actor);
    }
    if (work->flagged != 0) {
        mdlId = work->model->mdlId[1];
        particleModel = lbl_802C01C8[mdlId].hookData;
        particle = particleModel->data;
        particleCount = 0;
        for (entryIndex = 0; entryIndex < particleModel->maxCnt; entryIndex++, particle++) {
            if (particle->time < 0) {
                particle->time = 0;
                particle->pos = actorPosition;
                if (particleCount++ >= 2) {
                    return;
                }
            }
        }
    }
}

void fn_1_E34C(OMOBJ *object)
{
    M607ItemWork *work = object->data;

    if (fn_80071A8C() == 6) {
        fn_1_35C(object, fn_1_E544);
        return;
    }
    if (work->value == 3) {
        fn_80086F44(work->actor);
        fn_8002741C((s16)work->playerContext->player->actor->mdlId,
                         fn_8005C0F0(work->playerContext->characterId, 2, 0),
                         *work->model->mdlId);
        if (work->playerContext->characterId == 8) {
            fn_80026760(*work->model->mdlId, 0.0f, 0.0f,
                            -18.0f);
        } else {
            fn_80026760(*work->model->mdlId, 0.0f, 0.0f,
                            0.0f);
        }
        fn_80026808(*work->model->mdlId, 0.0f, 0.0f,
                        0.0f);
        fn_8002C95C(*work->model->mdlId, work->model->mtnId[4]);
        fn_1_35C(object, fn_1_E4D0);
        return;
    }
}

void fn_1_E4D0(OMOBJ *object)
{
    M607ItemWork *work = object->data;
    if (fn_80071A8C() == 6) {
        fn_1_35C(object, fn_1_E544);
        return;
    }
    if (work->value == 0) {
        fn_1_35C(object, fn_1_DCCC);
        return;
    }
}

void fn_1_E544(OMOBJ *object)
{
    M607ItemWork *work = object->data;

    if (((s32)work->frame % 2) == 0) {
        fn_800269E8(work->model->mdlId[0], (1 << 0));
    } else {
        fn_80026A44(work->model->mdlId[0], (1 << 0));
    }
    work->frame++;
    if ((work->frame >= 0x5A) || (work->playerContext != 0)) {
        fn_800269E8(work->model->mdlId[0], (1 << 0));
        fn_1_35C(object, 0);
        return;
    }
}

static inline f32 m607ParticleVectorLength(const HuVecF *src)
{
    return sqrtf((src->z * src->z) +
                ((src->x * src->x) + (src->y * src->y)));
}

static inline f32 m607ParticleNormalizeVector(HuVecF *dst, const HuVecF *src)
{
    f32 magnitude;

    magnitude = m607ParticleVectorLength(src);
    if (magnitude > 1.1920929e-07f) {
        f32 inverse = 1.0f / magnitude;

        dst->x = src->x * inverse;
        dst->y = src->y * inverse;
        dst->z = src->z * inverse;
    } else {
        magnitude = 0.0f;
        dst->x = 0.0f;
        dst->y = 0.0f;
        dst->z = 0.0f;
    }
    return magnitude;
}

void fn_1_E608(HU3D_MODEL *model, HU3D_PARTICLE *particle, f32 (*mtx)[4])
{
    HU3D_PARTICLE_DATA *data = particle->data;
    s32 index;
    s32 alpha;
    f32 magnitude;

    for (index = 0; index < particle->maxCnt; index++, data++) {
        if (data->time == 0) {
            data->time++;
            data->vel.x = (16.0f * frandf()) - 8.0f;
            data->vel.y = 1.0f + 2.0f * frandf();
            data->vel.z = (16.0f * frandf()) - 8.0f;
            PSVECScale(&data->vel, &data->vel, 2.0f);
            data->pos.x += (600.0f * frandf()) -
                           300.0f - 100.0f;
            data->pos.z += 100.0f +
                           ((600.0f * frandf()) - 300.0f);
            data->color.r = 0xE0;
            data->color.g = 0xE0;
            data->color.b = 0xE0;
            data->color.a = 0xFF;
            data->scale = 200.0f;
        }
        if (data->time == 1) {
            PSVECAdd(&data->pos, &data->vel, &data->pos);
            magnitude = m607ParticleNormalizeVector(&data->vel, &data->vel);
            magnitude += -magnitude / 20.0f;
            PSVECScale(&data->vel, &data->vel, magnitude);
            data->scale *= 1.01f;
            alpha = data->color.a - 2;
            if (alpha < 0) {
                alpha = 0;
                data->scale = 0.0f;
                data->time = -1;
            }
            data->color.a = alpha;
        }
    }
}

typedef struct M607FireworkModel {
    s16 mdlId;
    Vec position;
    f32 horizontalSpeed;
    f32 verticalSpeed;
    f32 rotation;
    f32 scale;
    f32 positionX;
    f32 orbitRadius;
    f32 positionY;
    s32 timer;
} M607FireworkModel;

typedef struct M607FireworkWork {
    M607FireworkModel models[150];
    s32 timer;
    s16 particleId;
} M607FireworkWork;

static inline void M607FireworkModelInit(M607FireworkModel *model, s32 modelData)
{
    model->mdlId = fn_80025A04(fn_80008DD0(modelData, 0x10000000, 2));
    model->position = lbl_1_rodata_94;
    model->horizontalSpeed = (6.0f * frandf()) - 3.0f;
    model->verticalSpeed = 12.0f;
    model->rotation = 0.0f;
    model->scale = 60.0f * frandf() * (M_PI * 6.0 / 180.0);
    model->timer = -(rand8() % 0x1E);
    model->positionX = 1.0f + ((12.0f * frandf()) - 6.0f);
    model->positionY = 35.0f + (20.0f * frandf());
    fn_80026808(model->mdlId, 45.0f, 0.0f, 0.0f);
    fn_80026790(model->mdlId, &model->position);
    fn_800268B0(model->mdlId, 1.5f, 1.5f, 1.5f);
}

/* Builds the layered firework models and particle system used for the celebration. */
void fn_1_EA48(OMOBJ *obj)
{
    M607FireworkWork *work = obj->data;
    HU3D_PARTICLE *particleState;
    HU3D_PARTICLE_DATA *particle;
    M607FireworkModel *model;
    s32 modelIndex;
    s32 particleIndex;

    model = &work->models[0];
    M607FireworkModelInit(model, DATANUM(0x420000, 13));
    model++;
    M607FireworkModelInit(model, DATANUM(0x420000, 14));
    model++;
    for (modelIndex = 2; modelIndex < 150; modelIndex++) {
        if (modelIndex - ((modelIndex / 10) * 10) == 0) {
            model->mdlId = fn_80025D4C(work->models[1].mdlId);
        } else {
            model->mdlId = fn_80025D4C(work->models[0].mdlId);
        }
        model->position = work->models[0].position;
        model->horizontalSpeed = (6.0f * frandf()) - 3.0f;
        model->verticalSpeed = 12.0f;
        model->rotation = 0.0f;
        model->scale = (f32)((M_PI * 6.0 / 180.0) * (60.0f * frandf()));
        model->timer = -(rand8() % 0x1E);
        model->positionX = 1.0f + ((12.0f * frandf()) - 6.0f);
        model->positionY = 35.0f + (20.0f * frandf());
        fn_80026808(model->mdlId, 45.0f, 0.0f, 0.0f);
        fn_80026790(model->mdlId, &model->position);
        fn_800268B0(model->mdlId, 1.5f, 1.5f, 1.5f);
        model++;
    }

    work->particleId = fn_8003216C(HuSprAnimRead(fn_80008634(DATANUM(0x210000, 2), 0x10000000)), (s16)0x100);
    particleState = lbl_802C01C8[work->particleId].hookData;
    fn_8003269C(work->particleId, 0);
    fn_800326D0(work->particleId, fn_1_E608);
    fn_80026B8C(work->particleId, 3);
    fn_80026760(work->particleId, 0.0f, 0.0f, 0.0f);
    particle = particleState->data;
    for (particleIndex = 0; particleIndex < particleState->maxCnt; particleIndex++, particle++) {
        particle->time = -1;
        particle->scale = 0.0f;
    }
    work->timer = 0;
    fn_1_35C(obj, fn_1_F08C);
}

void fn_1_F08C(OMOBJ *object)
{
    M607FireworkWork *work;
    HU3D_PARTICLE *hookData;
    HU3D_PARTICLE_DATA *record;
    s16 mdlId;
    s32 initializedCount;
    s32 recordIndex;

    work = object->data;
    work->timer++;
    if (work->timer == 0xD2) {
        HuAudFXPlay(MSM_SE_MG607_FIREWORK_BURST);
    } else if (work->timer >= 0xD2) {
        mdlId = work->particleId;
        hookData = lbl_802C01C8[mdlId].hookData;
        record = hookData->data;
        initializedCount = 0;
        recordIndex = 0;
        for (; recordIndex < hookData->maxCnt; recordIndex++, record++) {
            if (record->time < 0) {
                record->time = 0;
                record->pos = lbl_1_rodata_94;
                record->pos.y += 400.0f;
                if (initializedCount++ >= 0xC) {
                    break;
                }
            }
        }
    }
    if (work->timer >= 0xD7) {
        s32 player;

        for (player = 0; player < 4; player++) {
            fn_8006A030((s16)player, 0x1E, 0x1E, 0);
        }
        fn_1_35C(object, fn_1_F1D4);
        return;
    }
}

typedef struct M607FlashParticle {
    s16 mdlId;
    Point3d position;
    f32 horizontalSpeed;
    f32 verticalSpeed;
    f32 depthSpeed;
    f32 phase;
    f32 angularSpeed;
    f32 orbitRadius;
    f32 rotation;
    s32 lifetime;
} M607FlashParticle;

typedef struct M607FlashParticleWork {
    M607FlashParticle particles[0x96];
    s32 frame;
    s16 mdlId;
} M607FlashParticleWork;

void fn_1_F1D4(OMOBJ *object)
{
    M607FlashParticleWork *work = object->data;
    M607FlashParticle *particle = work->particles;
    s32 particleIndex;

    for (particleIndex = 0; particleIndex < 0x96; particleIndex++, particle++) {
        if (particle->lifetime == 0) {
            particle->position = lbl_1_rodata_94;
            particle->horizontalSpeed =
                (6.0f * frandf()) - 3.0f;
            particle->verticalSpeed = 128.0f;
            particle->depthSpeed = 0.0f;
            particle->phase = (M_PI * 6.0 / 180.0) *
                              (60.0f * frandf());
            particle->orbitRadius = 0.0f;
            particle->angularSpeed = 1.0f +
                ((12.0f * frandf()) - 6.0f);
            particle->angularSpeed *= 2.0f;
            particle->orbitRadius = 150.0f * frandf();
        } else if (particle->lifetime > 0) {
            f32 phase = (f32)((f64)particle->phase +
                ((M_PI * 3.0 / 180.0) * (f64)(0.5f * particle->lifetime)));

            particle->orbitRadius += 4.0f * particle->angularSpeed;
            particle->position.x = (f32)((f64)lbl_1_rodata_94.x +
                ((f64)particle->orbitRadius * sin((f64)phase)));
            particle->position.y += particle->verticalSpeed;
            particle->position.z = (f32)((f64)lbl_1_rodata_94.z +
                ((f64)particle->orbitRadius * cos((f64)phase)));
            fn_80026808(particle->mdlId, particle->rotation,
                (f32)((f64)(180.0f * phase) / M_PI),
                0.0f);
        }

        fn_80026790(particle->mdlId, &particle->position);
        particle->lifetime = (particle->lifetime + 1) % 100;
    }

    if (work->frame++ >= 0x122) {
        for (particleIndex = 0; particleIndex < 0x96; particleIndex++) {
            fn_800269E8(work->particles[particleIndex].mdlId, (1 << 0));
        }
        fn_1_35C(object, NULL);
        return;
    }
}

s32 lbl_1_data_28[2][4] = { { 0x420005, 0x420006, 0x420009, 0x42000A }, { 0x420007, 0x420008, 0x420009, 0x42000A } };
unsigned int lbl_1_data_48[9] = { 0x930000, 0x930001, 0x930002, 0x930020, 0x8E002C, 0x8E002B, 0x930006, 0x930007, 0 };
s32 lbl_1_data_6C[4] = { 0x420001, 0x420002, 0x420003, 0x420004 };
s32 lbl_1_data_7C[5] = { 0x42000F, 0x420010, 0x420011, 0x420012, 0x420013 };
s32 lbl_1_data_90[5] = { 0x420014, 0x420015, 0x420016, 0x420017, 0x420018 };
HuVecF lbl_1_data_A4[4] = { { -300.0f, 0.0f, -300.0f }, { 300.0f, 0.0f, -300.0f }, { -300.0f, 0.0f, 300.0f }, { 300.0f, 0.0f, 300.0f } };
HuVecF lbl_1_data_D4[4] = { { 55.0f, 59.0f, 0.0f }, { 175.0f, 59.0f, 0.0f }, { 401.0f, 59.0f, 0.0f }, { 521.0f, 59.0f, 0.0f } };
Point3d lbl_1_data_104 = { 0.0f, 0.0f, -300.0f };
HuVecF lbl_1_data_110[4] = { { -300.0f, 0.0f, 0.0f }, { -100.0f, 0.0f, 0.0f }, { 100.0f, 0.0f, 0.0f }, { 300.0f, 0.0f, 0.0f } };
f32 lbl_1_data_140[4] = { 10.0f, 30.0f, 60.0f, 90.0f };
f32 lbl_1_data_150[4] = { 0.04f, 0.06f, 0.09f, 0.12f };
f32 lbl_1_data_160[4] = { 8.0f, 9.0f, 10.0f, 10.0f };
char lbl_1_data_170[19] = "607dotabata-npcMOV";
char lbl_1_data_183[11] = "itemhook_R";

M607PlayerWork *lbl_1_bss_4C[4];
M607ItemWork *lbl_1_bss_24[10];
M607RootWork *lbl_1_bss_20;
/* Each player's winner status. */
s32 lbl_1_bss_10[4];
s32 lbl_1_bss_C;
s32 lbl_1_bss_8;
