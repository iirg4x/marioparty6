/* Dunk Bros. module entry points, sequence control and shared state. */
#include "math.h"
#include "dolphin/types.h"
#include "game/audio.h"
#include "msm_stream.h"
#include "game/object.h"
#include "game/mg/seqman.h"
#include "REL/m699Dll/m699_natural_views.h"
#include "REL/m699Dll.h"
#include <math.h>
#include "dolphin/mtx.h"
#include "game/hu3d.h"
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

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

void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}


extern MGSEQ_PARAM lbl_1_data_B0;
extern u8 lbl_1_data_1C[];
extern u8 lbl_1_data_28[];
extern u8 lbl_1_data_34[];
extern u8 lbl_1_data_0[];
extern u8 lbl_1_data_68[];

extern void fn_1_10E0(s32);
extern void fn_1_3F8(s16, s16);
extern void fn_1_1380(OMOBJMAN *, void *, void *, void *);
extern void fn_1_1A68(OMOBJMAN *, s32, f32, f32, f32, f32, void *);
extern void fn_1_26B4(void (*)(OMOBJ *, u32, s32));
extern void fn_1_8EC(OMOBJ *, u32, s32);
extern void fn_1_3624(OMOBJMAN *, void *);
extern void fn_1_5760(OMOBJMAN *);
extern void fn_1_FD3C(OMOBJMAN *);

/* Initialize the object manager, scene services, players and effects. */
void fn_1_A0(void) {
    s32 index;

    lbl_1_bss_184 = 0;
    lbl_1_bss_17C = 0;
    lbl_1_bss_178 = -1;
    lbl_1_bss_16C = -1;
    lbl_1_bss_140 = 0;
    fn_1_10E0(0);
    for (index = 0; index < 4; index++) {
        lbl_1_bss_15C[index] = 0;
    }
    lbl_1_bss_188 = fn_8003A8BC(0x78, 0x2000);
    fn_80069F1C(lbl_1_bss_188);
    fn_80070788(&lbl_1_data_B0);
    fn_80071D20(4, fn_1_3F8);
    fn_1_12B8(lbl_1_bss_188);
    fn_1_1380(lbl_1_bss_188, lbl_1_data_1C, lbl_1_data_28, lbl_1_data_34);
    fn_1_14E0();
    fn_1_1A68(lbl_1_bss_188, 1, 45.0f, 20.0f, 13000.0f, 0.0f, lbl_1_data_0);
    fn_1_26B4(fn_1_8EC);
    fn_1_3624(lbl_1_bss_188, lbl_1_data_68);
    fn_1_191B0(lbl_1_bss_188);
    fn_1_1B6E4(lbl_1_bss_188);
    fn_1_5760(lbl_1_bss_188);
    fn_1_FD3C(lbl_1_bss_188);
    M699EffectObjectInit(lbl_1_bss_188);
}


s32 fn_1_71C(short,short);
void fn_1_288(short mode,short state) {
    if(!fn_1_71C(mode,state)) fn_80071A9C();
}
extern s32 fn_1_10C0(void);

void fn_1_2C4(s16 sequence, s16 mode)
{
    if (fn_1_71C(sequence, mode) == 0) {
        if (mode == 0) {
            lbl_1_bss_180 = 0;
        }
        if (lbl_1_bss_178 == -1 && mode >= 0x12) {
            lbl_1_bss_178 = HuAudBGMPlay(MSM_STREAM_SLAM_DUNK);
        }
        switch (lbl_1_bss_180) {
        case 0:
            if (fn_1_10C0() == 1) {
                lbl_1_bss_180 += 1;
                fn_80071A9C();
            }
            break;
        case 1:
        default:
            break;
        }
    }
}


extern void fn_1_1098(s32 value);
extern s32 fn_1_71C(s16 first, s16 second);

void fn_1_3A4(s16 first, s16 second) {
    if ((fn_1_71C(first, second) == 0) && (second == 0)) {
        fn_1_1098(0xB4);
    }
}

void fn_1_3F8(short mode, short state)
{
    if (fn_1_71C(mode, state) != 0) {
        return;
    }
}


s32 fn_1_71C(short mode, short state);

void fn_1_430(short mode, short state)
{
    if (!fn_1_71C(mode, state) && !fn_1_1B690()) {
        fn_80071A9C();
    }
}


extern void fn_1_10E0(s32 value);
extern s32 lbl_1_bss_178;

/* Fade out the music and schedule the transition when the sequence completes. */
void fn_1_478(s16 first, s16 second) {
    if ((fn_1_71C(first, second) == 0) && (second == 0)) {
        fn_1_10E0(7);
        if (lbl_1_bss_178 != -1) {
            HuAudSStreamFadeOut(lbl_1_bss_178, 0x64);
            lbl_1_bss_178 = -1;
        }
        fn_1_1098(0xB4);
    }
}


extern s16 lbl_1_bss_14C[4];
extern s16 lbl_1_bss_154[4];
extern s16 lbl_1_bss_144[4];
extern s32 lbl_1_bss_17C;

void fn_1_50C(s16 player, s16 elapsedFrames)
{
    u16 winner;
    s32 firstScore;
    s32 secondScore;
    s32 index;

    if (fn_1_71C(player, elapsedFrames) == 0) {
        if (elapsedFrames == 0) {
            firstScore = fn_1_1B650(0);
            secondScore = fn_1_1B650(1);
            index = 0;
            while (index < 4) {
                lbl_1_bss_154[index] = -1;
                if (((firstScore > secondScore) && (lbl_1_bss_14C[index] == 0)) ||
                    ((firstScore < secondScore) && (lbl_1_bss_14C[index] == 1))) {
                    lbl_1_bss_154[index] = lbl_1_bss_144[index];
                }
                index++;
            }
            winner = fn_80071C18(lbl_1_bss_154[0], lbl_1_bss_154[1],
                                    lbl_1_bss_154[2], lbl_1_bss_154[3]);
        }
        if (lbl_1_bss_17C == 0 && elapsedFrames >= 0x78) {
            fn_80071A9C();
        }
    }
}


void fn_1_680(short mode, short state)
{
    if (fn_1_71C(mode, state) != 0) {
        return;
    }
}


void fn_1_6B8(short mode, short state)
{
    if (fn_1_71C(mode, state) != 0) {
        return;
    }
}


void fn_1_FE6C(void);
void fn_1_6F0(s16 mode, s16 frameNo)
{
    fn_1_FE6C();
    fn_1_5958();
    fn_1_1B76C();
    fn_1_19238();
}


extern void fn_1_7C0(void);
extern s32 lbl_1_bss_170;

s32 fn_1_71C(s16, s16) {
    fn_1_7C0();
    lbl_1_bss_170 += 1;
    return 0;
}

extern s32 lbl_1_bss_140;
extern f32 lbl_1_bss_0[][4];

/* Append one four-component camera record while space remains. */
void fn_1_75C(f32 x, f32 y, f32 z, f32 w) {
    if (lbl_1_bss_140 < 20) {
        f32 *slot = lbl_1_bss_0[lbl_1_bss_140++];
        slot[0] = x;
        slot[1] = y;
        slot[2] = z;
        slot[3] = w;
    }
}


/* Reset the camera records and install the two initial entries. */
void fn_1_7C0(void) {
    f32 *record_first;
    f32 *record_second;

    lbl_1_bss_140 = 0;
    if (lbl_1_bss_140 < 0x14) {
        record_first = lbl_1_bss_0[lbl_1_bss_140++];
        record_first[0] = 0.0f;
        record_first[1] = 0.0f;
        record_first[2] = 600.0f;
        record_first[3] = 0.0f;
    }
    if (lbl_1_bss_140 < 0x14) {
        record_second = lbl_1_bss_0[lbl_1_bss_140++];
        record_second[0] = 0.0f;
        record_second[1] = 2000.0f;
        record_second[2] = -6000.0f;
        record_second[3] = 0.0f;
    }
}

typedef struct M699CameraMotionEntry_s {
    u8 unk_0[0x0C]; /* No code names this field. */
    Vec position;
    Vec rotationValues;
    f32 targetDistance;
    Vec center;
    u8 unk_34[0x34]; /* No code names this field. */
    f32 rotation;
    u8 unk_6C[0x2C]; /* No code names this field. */
    s32 state;
    s32 timer;
    f32 progress;
    u8 unk_A4[8]; /* No code names this field. */
    Vec targetPosition;
} M699CameraMotionEntry;

typedef struct M699CameraMotionWork_s {
    s32 cameraId;
    u32 unknown_004;
    f32 fov;
    u8 unk_C[0x14]; /* No code names this field. */
    M699CameraMotionEntry entries[16];
} M699CameraMotionWork;

typedef struct M699CameraMotionDefaults_s {
    Vec firstPoint;
    Vec secondPoint;
    f32 fitFovRadians;
    s32 unknown_01C;
    s32 unknown_020;
    s32 unknown_024;
    s32 unknown_028;
} M699CameraMotionDefaults;

typedef struct M699CameraDirectionData_s {
    Vec direction;
    u8 unk_C[4]; /* No code names this field. */
} M699CameraDirectionData;

extern M699CameraDirectionData lbl_1_data_D8;
extern s32 fn_1_14F80(void);
extern Vec *fn_1_14778(s32 player);
extern f64 fn_1_30DC(f32 *position, void *spheres, s32 count, Vec *result);

void fn_1_8EC(OMOBJ *object, u32 cameraMask, s32 index)
{
    M699CameraMotionWork *work = object->data;
    M699CameraMotionEntry *entry = &work->entries[index];
    Vec *position = &entry->position;
    Vec *entryTarget = &entry->center;
    Mtx rotation;
    Mtx rotationX;
    Mtx rotationY;
    Mtx rotationZ;
    M699CameraMotionDefaults defaults = {
        {0.0f, 0.0f, -1.0f},
        {0.0f, 1.0f, 0.0f},
        45.0f,
        0x240,
        0x1E0,
        0x10,
        0x28,
    };
    Vec result;
    Vec transformedPoint;
    f32 fraction;
    f32 distance;

    switch (entry->state) {
    case 0:
        entry->state++;
        entry->timer = 0;
        entry->progress = 0.02f;
        fn_1_2A48(&lbl_1_data_D8.direction);
        /* fall through */
    case 1:
        if (++entry->timer >= 0x5A) {
            entry->state++;
            entry->timer = 0;
        } else {
            break;
        }
        /* fall through */
    case 2:
        defaults.fitFovRadians = MTXDegToRad(entry->rotation);
        PSMTXRotRad(rotationX, 'x',
                    MTXDegToRad(position[1].x));
        PSMTXRotRad(rotationY, 'y',
                    MTXDegToRad(position[1].y));
        PSMTXRotRad(rotationZ, 'z',
                    MTXDegToRad(position[1].z));
        PSMTXConcat(rotationY, rotationX, rotationX);
        PSMTXConcat(rotationX, rotationZ, rotation);
        PSMTXMultVecSR(rotation, &defaults.firstPoint, &defaults.firstPoint);
        PSMTXMultVecSR(rotation, &defaults.secondPoint, &defaults.secondPoint);
        fraction = (f32)fn_1_30DC(&defaults.firstPoint.x, lbl_1_bss_0,
                                  lbl_1_bss_140, &result);
        PSVECScale(&defaults.firstPoint, &transformedPoint, fraction);
        PSVECAdd(&result, &transformedPoint, &result);
        position->x = (f32)((f64)position->x +
            (0.3 * (f64)entry->progress *
             (f64)(result.x - position->x)));
        position->y = (f32)((f64)position->y +
            (0.3 * (f64)entry->progress *
             (f64)(result.y - position->y)));
        position->z = (f32)((f64)position->z +
            (0.3 * (f64)entry->progress *
             (f64)(result.z - position->z)));
        position[2].x = (f32)((f64)position[2].x +
            (0.3 * (f64)entry->progress *
             (f64)(fraction - position[2].x)));
        if ((entry->progress += 0.02) > 1.0) {
            entry->progress = 1.0f;
        }
        if (fn_1_10C0() == 5) {
            entry->state++;
            entry->timer = 0;
            entry->progress = 0.5f;
            entry->targetPosition = *fn_1_14778(fn_1_14F80() ^ 1);
        }
        break;
    case 3:
        position->x = (f32)((f64)position->x +
            (0.3 * (f64)entry->progress *
             (f64)(entry->targetPosition.x - position->x)));
        position->y = (f32)((f64)position->y +
            (0.3 * (f64)entry->progress *
             (f64)(entry->targetPosition.y - position->y)));
        position->z = (f32)((f64)position->z +
            (0.3 * (f64)entry->progress *
             (f64)(entry->targetPosition.z - position->z)));
        position[2].x = (f32)((f64)position[2].x +
            ((1200.0 - (f64)position[2].x) *
             (0.3 * (f64)entry->progress)));
        if (entry->timer < 0x1E) {
            distance = (f32)((f64)(f32)entry->timer / 30.0);
            fn_1_190C8((f32)(0.5 - (0.5 * (f64)distance)),
                       (f32)(0.7 + (0.3 * (1.0 - (f64)distance))));
        } else {
            fn_1_190C8(0.0f, 0.7f);
        }
        if (++entry->timer >= 0x5A) {
            entry->state--;
            entry->timer = 0;
            entry->progress = 0.1f;
            fn_1_190C8(0.0f, 1.0f);
        }
        break;
    }

    result = *position;
    result.z = 0.0f;
    fn_1_24C4(work->cameraId, fn_1_2A98(), &result,
              work->fov, 1500.0f);
}


extern s32 lbl_1_bss_164;
extern s32 lbl_1_bss_168;
extern s32 lbl_1_bss_16C;

void fn_1_EF0(void)
{
    s32 volume;

    if (lbl_1_bss_16C == -1) {
        return;
    }
    if (lbl_1_bss_168 > 0) {
        lbl_1_bss_168--;
    }
    if (lbl_1_bss_164 > 0) {
        lbl_1_bss_164--;
    }
    if (lbl_1_bss_168 == 0) {
        HuAudFXVolSet(lbl_1_bss_16C, 0x54);
        return;
    }

    if (lbl_1_bss_164 != 0) {
        volume = 0x7F - (s32)(2.388888888888889 *
                              (f64)lbl_1_bss_164);
    } else {
        volume = (s32)(0.7166666666666667 *
                       (f64)lbl_1_bss_168) + 0x54;
    }
    if (volume < 0x54) {
        volume = 0x54;
    }
    if (volume > 0x7F) {
        volume = 0x7F;
    }
    HuAudFXVolSet(lbl_1_bss_16C, (s16)volume);
}


void fn_1_1098(s32 value) {
    lbl_1_bss_168 = value;
    lbl_1_bss_164 = 0x12;
}


u8 *fn_1_10B8(u8 *base) {
    return base + 0x8D0000;
}


extern s32 lbl_1_bss_174;

s32 fn_1_10C0(void) {
    return lbl_1_bss_174;
}


s32 fn_1_10D0(void) {
    return lbl_1_bss_170;
}

void fn_1_10E0(s32 value) {
    lbl_1_bss_174 = value;
    lbl_1_bss_170 = 0;
}

s32 fn_1_1100(void) {
    return fn_1_1B690() / 60;
}


s16 fn_1_113C(s32 resultPlayerIndex) {
    return lbl_1_bss_154[resultPlayerIndex];
}


u32 fn_1_1154(u32 id)
{
    s32 index;
    s32 result;

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_154[index] >= 0) {
            break;
        }
    }
    if (index >= 4) {
        return 0;
    }
    for (index = 0; index < 4; index++) {
        if (id == (u32)lbl_1_bss_14C[index]) {
            if (lbl_1_bss_154[index] >= 0) {
                result = 1;
            } else {
                result = -1;
            }
            return result;
        }
    }
    return id;
}


void fn_1_1210(s32 index, s32 value14C, s32 value144)
{
    index &= 3;
    lbl_1_bss_14C[index] = (s16) value14C;
    lbl_1_bss_144[index] = (s16) value144;
}

extern short lbl_1_bss_15C[4];
void fn_1_1248(unsigned int player) {
    player&=3;
    lbl_1_bss_15C[player]=1;
}

/* Return whether all four players have signaled completion. */
int fn_1_1268() {
    int player;
    for(player=0;player<4;player++) {
        if(lbl_1_bss_15C[player]==0) return 0;
    }
    return 1;
}

/* Camera, lighting and sequence parameters. */
u8 lbl_1_data_0[28] = { 0, 0, 0, 0, 194, 200, 0, 0, 196, 175, 0, 0, 193, 3, 51, 51, 0, 0, 0, 0, 0, 0, 0, 0, 68, 250, 0, 0 };
u8 lbl_1_data_1C[12] = { 0, 0, 0, 0, 68, 72, 0, 0, 0, 0, 0, 0 };
u8 lbl_1_data_28[12] = "";
u8 lbl_1_data_34[52] = { 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 65, 160, 0, 0, 70, 28, 64, 0, 192, 192, 192, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
u8 lbl_1_data_68[72] = { 0, 0, 255, 255, 0, 0, 0, 32, 0, 0, 0, 96, 0, 0, 0, 32, 0, 0, 0, 64, 0, 0, 0, 110, 69, 84, 128, 0, 69, 22, 0, 0, 68, 212, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* .bss: defined after the code; the last defined sits lowest */
OMOBJMAN *lbl_1_bss_188;
s32 lbl_1_bss_184;
s32 lbl_1_bss_180;
s32 lbl_1_bss_17C;
s32 lbl_1_bss_178;
s32 lbl_1_bss_174;
s32 lbl_1_bss_170;
s32 lbl_1_bss_16C;
s32 lbl_1_bss_168;
s32 lbl_1_bss_164;
s16 lbl_1_bss_15C[4];
s16 lbl_1_bss_154[4];
s16 lbl_1_bss_14C[4];
s16 lbl_1_bss_144[4];
s32 lbl_1_bss_140;
f32 lbl_1_bss_0[20][4];

MGSEQ_PARAM lbl_1_data_B0 = {
    0, 1,
    fn_1_288, fn_1_2C4, fn_1_3A4, fn_1_430, fn_1_478,
    fn_1_50C, fn_1_680, fn_1_6B8, fn_1_6F0
};
M699CameraDirectionData lbl_1_data_D8 = { { 0.3f, -1.0f, -0.2f }, { 0, 0, 0, 0 } };
