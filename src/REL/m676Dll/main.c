/* Scene setup, phase callbacks and the minigame's tracking camera. */
#include "math.h"
#include "REL/m676Dll/camera_types.h"
#include "game/gamework.h"
#include "game/data.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/audio.h"
#include "game/frand.h"
#include "game/hu3d.h"
#include "msm_stream.h"

#define M676_SOUND_RESOURCE_ID_2230 2230
#define M676_DATA_NUMBER_OFFSET 8847360

typedef struct M676SequenceSettings {
    MGSEQ_PARAM sequence;
    s32 sequenceValues[3]; /* Trailing values stored after MGSEQ_PARAM. */
} M676SequenceSettings;
extern void fn_1_5E8(s16, s16);
extern void fn_1_16A0(OMOBJMAN *);
extern void fn_1_1768(OMOBJMAN *, Vec *, Vec *, GXColor *);
extern void fn_1_18C8(void);
extern void Hu3DBGColorSet(u8, u8, u8);
extern void fn_1_1E50(OMOBJMAN *, u32, f32, f32, f32, f32, OM_CAMERA_VIEW *);
extern s32 fn_1_1480(s32);
extern void fn_1_2EF8(u32, void *);
void fn_1_BA8(OMOBJ *, u32, s32);
extern void fn_1_2AAC(void (*)(OMOBJ *, u32, s32));
extern void fn_1_3A1C(OMOBJMAN *, M676AudioConfig *);
extern void fn_1_11F34(OMOBJMAN *);
extern void fn_1_126A4(OMOBJMAN *);
extern void fn_1_5A10(OMOBJMAN *);
extern void fn_1_88C0(OMOBJMAN *);
extern void fn_1_1033C(OMOBJMAN *);

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[], _dtors[];
void fn_1_A0(void);
s32 fn_1_A4C(s16 mode, s16 state);
void fn_1_AD4(void);
extern s32 fn_1_F7BC(void), fn_1_F828(void), fn_1_F8B4(void);
extern s32 fn_1_125E4(s32);
extern void fn_1_871C(void), fn_1_12590(void);
extern void fn_1_89F0(void), fn_1_5B2C(void), fn_1_11FBC(void);
extern void fn_1_28BC(s32, Vec *, Vec *, f32, f32);
extern Vec *fn_1_2E90(void);
extern void fn_1_2FB0(u32);
extern f64 fn_1_34D4(M676CameraFitParams *, M676CameraFitSphere *, s32, Vec *);
extern f32 fn_1_52C0(f32, f32, f32);
extern void fn_1_5698(Vec *, Vec *, f32, Vec *);
extern f32 fn_1_FD64(void);

void fn_1_348(s16, s16);
void fn_1_384(s16, s16);
void fn_1_5B0(s16, s16);
void fn_1_648(s16, s16);
void fn_1_748(s16, s16);
void fn_1_7DC(s16, s16);
void fn_1_8D0(s16, s16);
void fn_1_9E8(s16, s16);
void fn_1_A20(s16, s16);

M676CameraPose lbl_1_data_0[2] = {
    {{0.0f, -175.0f, 4800.0f}, {-43.0f, 0.0f, 0.0f}, 2345.0f},
    {{0.0f, -500.0f, 4800.0f}, {-43.0f, 0.0f, 0.0f}, 2500.0f}
};
Vec lbl_1_data_38 = {0.0f, 800.0f, 0.0f};
Vec lbl_1_data_44 = {0.0f, 0.0f, 0.0f};
M676LightSettings lbl_1_data_50 = {
    {255, 255, 255, 255},
    {{0, 1, 0, 20.0f, 10000.0f, {192, 192, 192, 0}}, {-1, 0, 0, 0, 0, {0}}}
};
M676AudioConfig lbl_1_data_84[2] = {
    {HU3D_CAM_ALL, 32, 96, 32, 64, 110, 2400.0f, 1900.0f, 1400.0f},
    {0}
};
M676SequenceSettings lbl_1_data_CC = {
    {0, 0, fn_1_348, fn_1_384, fn_1_5B0, fn_1_648, fn_1_748,
        fn_1_7DC, fn_1_8D0, fn_1_9E8, fn_1_A20},
    {0, 0, 0}
};

OMOBJMAN *lbl_1_bss_19C;
s32 lbl_1_bss_198;
s32 lbl_1_bss_194;
s32 lbl_1_bss_190;
s32 lbl_1_bss_18C;
s32 lbl_1_bss_188;
s32 lbl_1_bss_184;
s32 lbl_1_bss_180;
s32 lbl_1_bss_17C;
s32 lbl_1_bss_178;
s16 lbl_1_bss_170[4];
s16 lbl_1_bss_168[4];
s16 lbl_1_bss_160[4];
s16 lbl_1_bss_158[4];
s16 lbl_1_bss_150[4];
s32 lbl_1_bss_14C;
s32 lbl_1_bss_148;
s32 lbl_1_bss_144;
M676CameraFitSphere lbl_1_bss_4[20];
MGTIMER *lbl_1_bss_0;

/* Run module constructors, then initialize this minigame's board state. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

/* REL exit point: run the registered static destructors before unloading the minigame. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Module startup: initialize the sequence, timer, camera, audio, and scene objects. */
void fn_1_A0(void)
{
    s32 index;
    s32 resource;

    lbl_1_bss_190 = -1;
    lbl_1_bss_18C = -1;
    lbl_1_bss_188 = 0;
    lbl_1_bss_180 = 0;
    lbl_1_bss_184 = 0;
    lbl_1_bss_14C = 0;
    lbl_1_bss_148 = 0;
    lbl_1_bss_178 = 0;
    lbl_1_bss_144 = 0;
    for (index = 0; index < 4; index++) {
        lbl_1_bss_170[index] = 0;
        lbl_1_bss_150[index] = 0;
    }
    lbl_1_bss_19C = omInitObjMan(120, 8192);
    omGameSysInit(lbl_1_bss_19C);
    MgSeqCreate(&lbl_1_data_CC.sequence);
    MgSeqModeHookAdd(4, fn_1_5E8);
    lbl_1_bss_0 = MgTimerCreate(0);
    MgTimerParamSet(lbl_1_bss_0, 600, 0, 0);
    fn_1_16A0(lbl_1_bss_19C);
    fn_1_1768(lbl_1_bss_19C, &lbl_1_data_38, &lbl_1_data_44, &lbl_1_data_50.lightColor);
    fn_1_18C8();
    Hu3DBGColorSet(0, 0, 0);
    fn_1_1E50(lbl_1_bss_19C, 1, 45.0f, 20.0f, 4000.0f,
              2000.0f, lbl_1_data_0);
    resource = fn_1_1480(0);
    fn_1_2EF8(1, HuDataSelHeapReadNum(resource, HU_MEMNUM_OVL, 2));
    fn_1_2AAC(fn_1_BA8);
    fn_1_3A1C(lbl_1_bss_19C, lbl_1_data_84);
    fn_1_11F34(lbl_1_bss_19C);
    fn_1_126A4(lbl_1_bss_19C);
    fn_1_5A10(lbl_1_bss_19C);
    fn_1_88C0(lbl_1_bss_19C);
    fn_1_1033C(lbl_1_bss_19C);
}

void fn_1_348(short mode,short state) {
    if(!fn_1_A4C(mode,state)) MgSeqModeNext();
}

/* Fade-in callback starts music, waits at least 60 frames and for the opening-route flag to clear
 * after the first jump, then waits for the camera transition and a 30-frame hold before advancing.
 */
void fn_1_384(s16 sequenceMode, s16 frameNo)
{
    if (fn_1_A4C(sequenceMode, frameNo) != 0) {
        return;
    }
    if (frameNo == 0) {
        lbl_1_bss_198 = 0;
    }
    if (lbl_1_bss_190 == -1 && frameNo >= 18) {
        lbl_1_bss_190 = HuAudBGMPlay(MSM_STREAM_MGMUS_30);
        lbl_1_bss_18C = HuAudFXPlay(M676_SOUND_RESOURCE_ID_2230);
    }
    switch (lbl_1_bss_198) {
    case 0:
        lbl_1_bss_198++;
        lbl_1_bss_194 = 0;
        MgSeqModeChangeOff();
        break;
    case 1:
        if (++lbl_1_bss_194 < 60 || fn_1_F7BC() != 0) {
            break;
        }
        lbl_1_bss_198++;
        lbl_1_bss_194 = 0;
        lbl_1_bss_14C = 1;
        /* fall through */
    case 2:
        if (lbl_1_bss_188 != 0 && lbl_1_bss_18C != -1) {
            HuAudFXFadeOut(lbl_1_bss_18C, 1000);
            lbl_1_bss_18C = -1;
        }
        if (lbl_1_bss_148 == 0) {
            break;
        }
        lbl_1_bss_198++;
        lbl_1_bss_194 = 0;
        /* fall through */
    case 3:
        if (++lbl_1_bss_194 >= 30) {
            MgSeqModeNext();
        }
        break;
    }
}

/* Sequence callback for this phase; the shared entry also clears the camera-fit spheres. */
void fn_1_5B0(short mode, short state)
{
    if (fn_1_A4C(mode, state) != 0) {
        return;
    }
}

/* Sequence callback: begin the timer and mark the timed gameplay phase active. */
void fn_1_5E8(s16 mode, s16 state) {
    if (fn_1_A4C(mode, state) != 0) {
        return;
    }
    fn_1_12590();
    MgTimerModeOnSet(lbl_1_bss_0, 1);
    lbl_1_bss_178 = 1;
}

/* Gameplay forces unassigned players to select props on timer expiry, removes the timer after four
 * selections, and advances after four props leave the course or the phase exceeds 3600 frames. */
void fn_1_648(s16 firstValue, s16 secondValue)
{
    if (fn_1_A4C(firstValue, secondValue) == 0) {
        if (lbl_1_bss_0 != NULL) {
            if (MgTimerDoneCheck(lbl_1_bss_0) != 0) {
                fn_1_871C();
                lbl_1_bss_178 = 0;
            }
            if (fn_1_F828() != 0) {
                if (lbl_1_bss_0 != NULL) {
                    MgTimerKill(lbl_1_bss_0);
                    lbl_1_bss_0 = NULL;
                }
                lbl_1_bss_178 = 0;
            }
        }
        if (fn_1_F8B4() != 0) {
            MgSeqModeNext();
            return;
        }
        if (secondValue > 3600) {
            MgSeqModeNext();
            return;
        }
    }
}

/* On the first finish-phase frame, fades out the minigame music and sets lbl_1_bss_17C. */
void fn_1_748(s16 first, s16 second)
{
    if ((fn_1_A4C(first, second) == 0) && (second == 0)) {
        if (lbl_1_bss_190 != -1) {
            HuAudSStreamFadeOut(lbl_1_bss_190, 100);
            lbl_1_bss_190 = -1;
        }
        lbl_1_bss_17C = 1;
    }
}

/* Pre-winner callback snapshots award eligibility at frame zero and advances from frame 120 while
 * the shared wait flag is clear. */
void fn_1_7DC(s16 sequenceMode, s16 frameNo)
{
    s32 playerIndex;

    if (fn_1_A4C(sequenceMode, frameNo) == 0) {
        if (frameNo == 0) {

            lbl_1_bss_198 = 0;
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (fn_1_125E4(playerIndex) != 0) {
                    lbl_1_bss_168[playerIndex] = lbl_1_bss_158[playerIndex];
                } else {
                    lbl_1_bss_168[playerIndex] = -1;
                }
            }
        }
        if (lbl_1_bss_180 == 0 && frameNo >= 120) {
            MgSeqModeNext();
            return;
        }
    }
}

/* Results callback: add eligible bonuses outside practice mode, then play an award jingle if anyone
 * scored or the file-select jingle otherwise. */
void fn_1_8D0(s16 sequenceMode, s16 frameNo)
{
    s32 anyAward;
    s32 index;

    if (fn_1_A4C(sequenceMode, frameNo) != 0) {
        return;
    }
    if (frameNo != 0) {
        return;
    }

    anyAward = 0;
    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_168[index] >= 0) {
            s16 updatedBonus;
            s32 bonus;

            bonus = GwPlayer[index].mgCoinBonus;
            updatedBonus = bonus + fn_1_125E4(index);
            if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                GwPlayer[index].mgCoinBonus = updatedBonus;
            }
        }
    }

    index = 0;
    for (; index < 4; index++) {
        if (fn_1_125E4(index) != 0) {
            anyAward = 1;
        }
    }

    if (anyAward != 0) {
        HuAudJinglePlay(MSM_STREAM_MGMUS_17);
        return;
    }
    HuAudJinglePlay(MSM_STREAM_FILESEL);
}

/* Sequence callback for this phase; the shared entry also clears the camera-fit spheres. */
void fn_1_9E8(short mode, short state)
{
    if (fn_1_A4C(mode, state) != 0) {
        return;
    }
}

/* Final callback calls the empty prop and score shutdown stubs around player model cleanup; it
 * calls the score stub twice. */
void fn_1_A20(s16 mode, s16 state)
{
    fn_1_89F0();
    fn_1_5B2C();
    fn_1_11FBC();
    fn_1_11FBC();
}

s32 fn_1_A4C(s16 mode, s16 state) {
    /* Sequence callbacks reset the accumulated camera-fit spheres before their phase work. */
    fn_1_AD4();
    return 0;
}

/* Called by scene objects to add a world-space sphere to the camera-fit set. */
void fn_1_A70(f32 x, f32 y, f32 z, f32 w) {
    if (lbl_1_bss_144 < 20) {
        M676CameraFitSphere *slot = &lbl_1_bss_4[lbl_1_bss_144++];
        slot->center.x = x;
        slot->center.y = y;
        slot->center.z = z;
        slot->radius = w;
    }
}

void fn_1_AD4(void) {
    lbl_1_bss_144 = 0;
}

/* Interpolates the center and zoom linearly and each rotation along the shorter turn. */
void fn_1_AE8(M676CameraPose *start,
    M676CameraPose *end, M676CameraPose *result, f32 factor)
{
    fn_1_5698(&start->center, &end->center, factor, &result->center);
    result->rot.x = fn_1_52C0(start->rot.x, end->rot.x, factor);
    result->rot.y = fn_1_52C0(start->rot.y, end->rot.y, factor);
    result->rot.z = fn_1_52C0(start->rot.z, end->rot.z, factor);
    result->zoom = start->zoom + factor * (end->zoom - start->zoom);
}

/* Per-frame camera object callback: animate the view and fit it around registered scene spheres. */
void fn_1_BA8(OMOBJ *object, u32 slot, s32 index)
{
    M676CameraWork *data = object->data;
    M676CameraRecord *record = &data->entries[index];
    M676CameraPose *current = &record->currentPose;
    M676CameraPose *target = &record->targetPose;
    f32 combined[3][4];
    f32 rotationX[3][4];
    f32 rotationY[3][4];
    f32 rotationZ[3][4];
    M676CameraFitParams constants = {
        { 0.0f, 0.0f, -1.0f },
        { 0.0f, 1.0f, 0.0f },
        45.0f,
        576,
        480,
        16,
        40
    };
    Vec result;
    Vec movement;
    f32 delta;
    f32 blend;
    switch (record->state) {
    case 0:
        record->state++;
        record->timer = 0;
        record->blend = 0.05f;
        fn_1_2FB0(1);
        *current = lbl_1_data_0[1];
        /* fall through */
    case 1:
        if (lbl_1_bss_14C != 0) {
            record->state++;
            record->timer = 0;
        case 2:
            blend = (f32)(0.004166666666666667 *
                (f64)(f32)record->timer);
            blend = (f32)(0.5 + (-0.5 * cos((3.141592653589793 *
                (180.0 * (f64)blend)) / 180.0)));
            fn_1_AE8(
                &lbl_1_data_0[1], target,
                current, blend);
            if (record->timer == 210) {
                lbl_1_bss_188 = 1;
            }
            if (++record->timer >= 240) {
                record->state++;
                record->timer = 0;
                lbl_1_bss_148 = 1;
                *current = *target;
            case 3:
                if (fn_1_15E0() != 0) {
                    record->state++;
                    record->timer = 0;
                case 4:
                    if (++record->timer >= 90) {
                        record->state++;
                        record->timer = 0;
                        record->angle = 0.0f;
                        record->fade = 0.0f;
                        record->blend = 0.05f;
                    case 5:
                        if (MgSeqModeGet() == 7) {
                            record->state++;
                            record->timer = 0;
                            *target = *current;
                        break;
                        } else if (fn_1_FD64() < 4800.0) {
                            record->fade += 0.01 * (1.0 - record->fade);
                            current->rot.x = fn_1_52C0(
                                current->rot.x, -60.0f,
                                0.1 * record->blend);
                        } else {
                            record->fade += 0.02 * -record->fade;
                            current->rot.x = fn_1_52C0(
                                current->rot.x, -43.0f,
                                0.1 * record->blend);
                        }
                        record->angle = fmodf(0.5f + record->angle, 360.0f);
                        current->rot.y = record->fade *
                            (45.0f * sin((3.141592653589793 *
                                (f64)record->angle) / 180.0));
                        constants.fov = 0.017453292f *
                            record->fov;
                        PSMTXRotRad(rotationX, 'x',
                            0.017453292f * current->rot.x);
                        PSMTXRotRad(rotationY, 'y',
                            0.017453292f * current->rot.y);
                        PSMTXRotRad(rotationZ, 'z',
                            0.017453292f * current->rot.z);
                        PSMTXConcat(rotationY, rotationX, rotationX);
                        PSMTXConcat(rotationX, rotationZ, combined);
                        PSMTXMultVecSR(combined, &constants.direction,
                            &constants.direction);
                        PSMTXMultVecSR(combined, &constants.up,
                            &constants.up);
                        delta = (f32)fn_1_34D4(&constants,
                            lbl_1_bss_4,
                            lbl_1_bss_144, &result);
                        PSVECScale(&constants.direction, &movement, delta);
                        PSVECAdd(&result, &movement, &result);
                        current->center.x += record->blend *
                            (result.x - current->center.x);
                        current->center.y += record->blend *
                            (result.y - current->center.y);
                        current->center.z += record->blend *
                            (result.z - current->center.z);
                        current->zoom += record->blend *
                            (delta - current->zoom);
                    }
                }
            }
        }
        break;
    case 6:
        blend = (f32)(0.016666666666666666 * (f64)(f32)record->timer);
        blend = (f32)sin((3.141592653589793 *
            (90.0 * (f64)blend)) / 180.0);
        fn_1_AE8(target,
            &lbl_1_data_0[1], current,
            blend);
        if (++record->timer >= 60) {
            record->state++;
            record->timer = 0;
            *current = lbl_1_data_0[1];
        }
        break;
    }
    fn_1_28BC(data->cameras, fn_1_2E90(), &current->center,
        data->shadowFov, 1200.0f);
    Hu3DShadowColSet(0U, 0U, 64U);
    Hu3DShadowMultiTPLvlSet(0.6f, (s16)slot);
}

s32 fn_1_1480(s32 resource)
{
    return resource + M676_DATA_NUMBER_OFFSET;
}

s32 fn_1_1488(void) {
    return lbl_1_bss_184;
}

s32 fn_1_1498(void) {
    return lbl_1_bss_178;
}

/* Return the current minigame timer value, or zero after the timer has been removed. */
s32 fn_1_14A8(void) {
    if (lbl_1_bss_0 != NULL) {
        return MgTimerValueGet(lbl_1_bss_0);
    }
    return 0;
}

s16 fn_1_14F4(s32 playerIndex) {
    return lbl_1_bss_168[playerIndex];
}

/* Resolve a player ID to its stored award state, returning the original ID when it is absent. */
s32 fn_1_150C(u32 player)
{
    s32 index;
    s32 result;

    for (index = 0; index < 4; index++) {
        if (player == (u32) lbl_1_bss_160[index]) {
            if (lbl_1_bss_168[index] >= 0) {
                result = 1;
            } else {
                result = -1;
            }
            return result;
        }
    }
    return player;
}

/* Record a player's slot and character in the four-entry tables selected by playerIndex. */
void fn_1_1588(s32 playerIndex, s32 playerSlot, s32 character)
{
    playerIndex &= 3;
    lbl_1_bss_160[playerIndex] = (s16) playerSlot;
    lbl_1_bss_158[playerIndex] = (s16) character;
}

void fn_1_15C0(unsigned int player) {
    player&=3;
    lbl_1_bss_150[player]=1;
}

/* Camera callback readiness check: return nonzero after all four player flags are set. */
int fn_1_15E0() {
    int player;
    for(player=0;player<4;player++) {
        if(lbl_1_bss_150[player]==0) return 0;
    }
    return 1;
}

void fn_1_1630(unsigned int player) {
    player&=3;
    lbl_1_bss_170[player]=1;
}

/* Return nonzero once each of the four player slots has set its second readiness flag. */
int fn_1_1650() {
    int player;
    for(player=0;player<4;player++) {
        if(lbl_1_bss_170[player]==0) return 0;
    }
    return 1;
}
