// Particle effects used by the ending sequence and its staff displays.
#include "dolphin/math.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm.h"
#include "msm_se.h"
#include "dolphin.h"
#include "game/armem.h"

#include "game/gamework.h"
#include "game/pad.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"

ANIMDATA *lbl_1_bss_1E30[3];
s16 lbl_1_bss_1E2C;
s16 lbl_1_bss_1E2A;
s16 lbl_1_bss_1E26[2];
s16 lbl_1_bss_1E22[2];
s16 lbl_1_bss_1E1E[2];
s16 lbl_1_bss_1E1C;
s16 lbl_1_bss_1DE0[6][5];
typedef void (*VoidFunc)(void);

extern double lbl_1_rodata_1F0;
extern double lbl_1_rodata_1F8;
extern Vec lbl_1_rodata_234;

typedef struct EndingWindowPlayers {
    s16 player[4];
} EndingWindowPlayers;

typedef struct EndingSpritePositions {
    Vec position[10];
} EndingSpritePositions;

typedef struct EndingModelObjectNames {
    char *name[10];
} EndingModelObjectNames;

typedef struct EndingAudioState {
    s32 channel[32];
} EndingAudioState;

typedef struct EndingLightVectors {
    HuVecF vector[2];
} EndingLightVectors;

typedef struct EndingParticleCounts {
    s16 count[5];
} EndingParticleCounts;

typedef struct EndingScenePositions {
    HuVecF position[3];
} EndingScenePositions;

typedef struct EndingMotionWork {
    s16 state;
    float time;
    float duration;
    HuVecF unk_0C;
    HuVecF unk_18;
    HuVecF unk_24;
    float start;
    float end;
    float unk_38;
    float unk_3C;
} EndingMotionWork;

#define sind(x) sin((M_PI * (x)) / 180.0)

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern s16 lbl_1_bss_26;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_14;
extern OMOBJ *lbl_1_bss_18;
extern OMOBJ *lbl_1_bss_1C;
extern OMOBJ *lbl_1_bss_20;
extern s16 lbl_1_bss_24;
extern s16 lbl_1_bss_2E;
extern float lbl_1_bss_28;
extern s16 lbl_1_bss_2C;
extern EndingMotionWork lbl_1_bss_34[96];
extern EndingMotionWork lbl_1_bss_1834[7];
extern s16 lbl_1_bss_1A08[10];
extern s16 lbl_1_bss_1A1C[2];
extern ANIMDATA *lbl_1_bss_1A20[10];
extern s16 lbl_1_bss_19F4[10];
extern s16 lbl_1_bss_1A48[5];
extern s16 lbl_1_bss_1A52[2];
extern s16 lbl_1_bss_1A56[2];
extern EndingMotionWork lbl_1_bss_1A5C[2];
extern EndingMotionWork lbl_1_bss_1ADC[10];
extern EndingAudioState lbl_1_bss_1D5C;
extern s32 lbl_1_bss_1DDC;

extern float lbl_1_rodata_C8;
extern EndingLightVectors lbl_1_rodata_88;
extern EndingLightVectors lbl_1_rodata_A0;
extern GXColor lbl_1_rodata_B8;

extern EndingModelObjectNames lbl_1_rodata_1B4;

extern HuVecF lbl_1_rodata_294;
extern HuVecF lbl_1_rodata_2A0;
extern HuVecF lbl_1_rodata_2B8;
extern EndingScenePositions lbl_1_rodata_2CC;

extern HuVecF lbl_1_rodata_288;
extern EndingParticleCounts lbl_1_rodata_390;

extern s16 lbl_1_data_10E;
extern s16 lbl_1_data_38;
extern u32 lbl_1_data_0;
extern s32 lbl_1_data_7C;
extern char lbl_1_data_80[];
u32 lbl_1_data_128[3] = {
    DATANUM(DATA_ending, 129), DATANUM(DATA_ending, 130),
    DATANUM(DATA_ending, 131)
};
extern char lbl_1_data_25[];
extern char lbl_1_data_DD[];
extern char lbl_1_data_CB[];

int rand8(void);
s32 frandmod(s32 modulus);
void fn_1_98(HUWINID window, u32 message, s16 character);
void fn_1_25C(OMOBJ *object);
void fn_1_1C0C(s16 index, s16 motion, float time);
void fn_1_10DEC(s16 index, HuVecF *pos, s16 mode);
void fn_1_DD14(void);
void fn_1_F1B8(s16 display, HuVecF *pos);
void fn_1_12838(void);
void fn_1_E1EC(s16 display, HuVecF *pos);
void fn_1_EAB8(s16 display, HuVecF *pos);
void fn_1_F068(s16 index, s16 display, HuVecF *pos);
void fn_1_F11C(s16 index, s16 count);
void fn_1_F23C(s16 count);
void fn_1_111B0(s16 display);
void fn_1_11208(s16 index, HuVecF *pos, s16 mode);
void fn_1_E0EC(s16 groupId, s32 attr);
void fn_1_EB54(s16 time);
void fn_1_1160(OMOBJ *object);
void fn_1_3B4C(OMOBJ *object);
void fn_1_3D5C(OMOBJ *object);
void fn_1_36E0(OMOBJ *object);
void fn_1_23D8(OMOBJ *object);
void fn_1_2AEC(OMOBJ *object);
void fn_1_E270(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_EBCC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_F2CC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_FDD4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_10628(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_112E0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_11714(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_702C(void);
void fn_1_8F80(OMOBJ *object);
void fn_1_A964(void);
void fn_1_B568(void);
void fn_1_BE30(void);
void fn_1_C7DC(void);
void fn_1_E16C(HUSPR_GROUPID groupId, s32 attr);
void fn_1_D434(void);
void fn_1_7B4(void);
void fn_1_45C0(OMOBJ *object);
void fn_1_4BC8(OMOBJ *object);
void fn_1_11F34(void);
void fn_1_DF60(HuVecF *dest, HuVecF *target, float time);
float fn_1_DDF8(float start, float end, float time, float duration);

typedef struct StaffParticleTarget {
    u8 scale;
    u8 alpha;
} STAFF_PARTICLE_TARGET;
enum {
    MDSEL_DLL_RETURN_REPORT_SIZE = 60,
    MDSEL_ITEM_HOOK_NAME_SIZE = 19,
    MDSEL_OBJECT_PRIORITY = 4096,
    MDSEL_OBJECT_CAPACITY = 16,
    MDSEL_LARGE_OBJECT_CAPACITY = 64,
    MDSEL_OBJECT_MANAGER_PRIORITY = 8192,
    MDSEL_MAIN_PROCESS_PRIORITY = 12288,
    MDSEL_MAIN_PROCESS_STACK_SIZE = 12288,
    MDSEL_WINDOW_WIDTH = 544,
    MDSEL_WINDOW_HEADER_HEIGHT = 42,
    MDSEL_WINDOW_MENU_HEIGHT = 68,
};
enum {
    MDPARTY_STAGE_OBJECT_PRIORITY = 4096,
    MDPARTY_STAGE_PARTICLE_TARGET_COUNT = 5,
    MDPARTY_COLOR_COMPONENT_MASK = 255,
};

enum {
    ENDING_COLOR_COMPONENT_MAX = 255,
    ENDING_SPARKLE_GRAY_LEVEL = 64,
    ENDING_SPARKLE_SMALL_SPEED_VARIANCE = 32,
    ENDING_SPARKLE_SMALL_SPEED_BASE = 64,
    ENDING_SPARKLE_SMALL_SCALE_VARIANCE = 100,
    ENDING_SPARKLE_SMALL_SCALE_BASE = 300,
    ENDING_SPARKLE_LARGE_SPEED_BASE = 160,
    ENDING_SPARKLE_LARGE_SCALE_BASE = 600,
    ENDING_PARTICLE_DATA_SIZE = sizeof(HU3D_PARTICLE_DATA),
};

/* Interpolate a winner-burst particle's scale factor as it ages. */
static inline float tail7_fn_1_3F50C(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((time / duration) * (endValue - startValue));
}
/* Move staff-particle alpha and scale toward their targets over ten hook passes. */
static inline float tail17_staff_approach_inlined(float current, float target, s32 steps)
{
    if (current == target || steps == 0) {
        return target;
    }
    return (target + current * (steps - 1)) / steps;
}
/* Fade a spawned ending particle as its particle hook advances its age. */
static inline float tail19_fn_1_0_inlined(float start, float end, float time, float duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}
/* Start the selected staff sparkle emitters when fn_1_10DEC enables them. */
static inline void tail20_fn_1_6C90_inlined(s16 modelNo, HuVecF *pos, GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    s16 i;
    HU3D_PARTICLE_DATA *entry;

    model = &Hu3DData[lbl_1_bss_1E22[modelNo]];
    particle = model->hookData;
    for (i = 0, entry = particle->data; i < particle->maxCnt; i++, entry++) {
        entry->time = 1;
        if (color != NULL) {
            entry->color.r = color->r;
            entry->color.g = color->g;
            entry->color.b = color->b;
        }
    }
    if (pos != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_1E22[modelNo], pos);
    }
    Hu3DModelAttrReset(lbl_1_bss_1E22[modelNo], HU3D_ATTR_DISPOFF);
}
/* Enable one staff burst emitter and save its color and particle spawn center. */
static inline void tail20_staff_spawn_effect_inlined(s16 modelNo, HuVecF *pos, GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_1E1E[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 1;
    if (color != NULL) {
        particle->pos.x = color->r;
        particle->pos.y = color->g;
        particle->pos.z = color->b;
    }
    if (pos != NULL) {
        particle->spawnCenter.x = pos->x;
        particle->spawnCenter.y = pos->y;
        particle->spawnCenter.z = pos->z;
    }
    Hu3DModelAttrReset(lbl_1_bss_1E1E[modelNo], HU3D_ATTR_DISPOFF);
}
/* Mark every selected staff sparkle particle for its fade-out hook state. */
static inline void tail20_fn_1_6D94_inlined(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    s16 i;
    HU3D_PARTICLE_DATA *entry;

    model = &Hu3DData[lbl_1_bss_1E22[modelNo]];
    particle = model->hookData;
    for (i = 0, entry = particle->data; i < particle->maxCnt; i++, entry++) {
        entry->time = 2;
    }
}
/* Stop spawning from the selected staff burst emitter when fn_1_10DEC hides it. */
static inline void tail20_staff_stop_effect_inlined(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_1E1E[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}
/* Start all five emitters for a winner group when fn_1_12988 selects display mode 1. */
static inline void tail22_fn_1_ED60_inlined(s16 groupNo, HuVecF *spawnPosition, GXColor *color)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *emitter;

    for (i = 0; i < 5; i++) {
        model = &Hu3DData[lbl_1_bss_1DE0[groupNo][i]];
        emitter = model->hookData;
        emitter->dataCnt = 1;
        if (color != NULL) {
            emitter->pos.x = color->r;
            emitter->pos.y = color->g;
            emitter->pos.z = color->b;
        }
        if (spawnPosition != NULL) {

            emitter->spawnCenter.x = spawnPosition->x;
            emitter->spawnCenter.y = spawnPosition->y;
            emitter->spawnCenter.z = spawnPosition->z;
        }
        Hu3DModelAttrReset(lbl_1_bss_1DE0[groupNo][i], HU3D_ATTR_DISPOFF);
    }
}
/* Stop new particles in all five emitters when fn_1_12988 selects mode 0. */
static inline void tail22_fn_1_EEC8_inlined(s16 groupNo)
{
    s16 i;
    HU3D_MODEL *model;
    s16 *work;

    for (i = 0; i < 5; i++) {
        model = &Hu3DData[lbl_1_bss_1DE0[groupNo][i]];
        work = model->hookData;
        *work = 0;
    }
}
/* Show or hide every emitter model in a winner group for fn_1_12988. */
static inline void tail22_fn_1_ECAC_inlined(s16 groupNo, s16 show)
{
    s16 i;

    for (i = 0; i < 5; i++) {
        if (show) {
            Hu3DModelAttrReset(lbl_1_bss_1DE0[groupNo][i], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_1DE0[groupNo][i], HU3D_ATTR_DISPOFF);
        }
    }
}
/* Approach an ending particle's target alpha or scale over the remaining frames. */
static inline float tail23_ending_lerp(float start, float end, float time)
{
    if (start == end || time <= 1.0f) {
        return end;
    }
    return (start * (time - 1.0f) + end) / time;
}
/* Create the four-particle pale sparkle model during ending effect setup. */
static inline void ending_inline_EF58(void)
{
    lbl_1_bss_1E2A = Hu3DParticleCreate(lbl_1_bss_1E30[1], 4);
    Hu3DModelPosSet(lbl_1_bss_1E2A, (0.0f),
        (100.0f), (200.0f));
    Hu3DModelScaleSet(lbl_1_bss_1E2A, (1.0f),
        (1.0f), (1.0f));
    Hu3DModelLayerSet(lbl_1_bss_1E2A, 6);
    Hu3DParticleHookSet(lbl_1_bss_1E2A, fn_1_EBCC);
    Hu3DParticleBlendModeSet(lbl_1_bss_1E2A, 1);
}

/* Create two hidden staff sparkle models during ending effect setup. */
static inline void ending_inline_1025C(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_1E22[i] = Hu3DParticleCreate(lbl_1_bss_1E30[1], 10);
        Hu3DModelPosSet(lbl_1_bss_1E22[i], (0.0f),
            (0.0f), (0.0f));
        Hu3DModelScaleSet(lbl_1_bss_1E22[i], (1.0f),
            (1.0f), (1.0f));
        Hu3DModelAttrSet(lbl_1_bss_1E22[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_1E22[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_1E22[i], fn_1_FDD4);
        Hu3DParticleBlendModeSet(lbl_1_bss_1E22[i], 1);
    }
}

/* Create two hidden staff burst models during ending effect setup. */
static inline void ending_inline_10C28(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_1E1E[i] = Hu3DParticleCreate(lbl_1_bss_1E30[0], 128);
        Hu3DModelPosSet(lbl_1_bss_1E1E[i], (0.0f),
            (0.0f), (0.0f));
        Hu3DModelScaleSet(lbl_1_bss_1E1E[i], (1.0f),
            (1.0f), (1.0f));
        Hu3DModelAttrSet(lbl_1_bss_1E1E[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_1E1E[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_1E1E[i], fn_1_10628);
        Hu3DParticleBlendModeSet(lbl_1_bss_1E1E[i], 1);
    }
}

/* Return the linear fade value used by particle hooks as their age advances. */
float fn_1_DDF8(float start, float end, float time, float duration)
{
    if (time <= (0.0f)) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + (time / duration) * (end - start);
}

/* Apply a sine-shaped alpha pulse that returns to its starting value at the end of each cycle. */
float fn_1_DE3C(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return startValue;
    }
    return startValue + ((endValue - startValue) * sind((180.0f / duration) * time));
}

/* Move one ending-scene value toward its target over the remaining frames. */
float fn_1_DF18(float start, float end, float time)
{
    if (start == end || time <= (1.0f)) {
        return end;
    }
    return (start * (time - (1.0f)) + end) / time;
}

/* Ease a scene position toward its target; called by ending scene updates. */
void fn_1_DF60(HuVecF *dest, HuVecF *target, float time)
{
    dest->x = fn_1_DF18(dest->x, target->x, time);
    dest->y = fn_1_DF18(dest->y, target->y, time);
    dest->z = fn_1_DF18(dest->z, target->z, time);
}

/* Add an attribute to every sprite in a group when the ending UI updates it. */
void fn_1_E0EC(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

/* Clear an attribute from every sprite in a group when the ending UI updates it. */
void fn_1_E16C(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}

/* Move and show or hide the main ending particle model when its scene effect changes. */
void fn_1_E1EC(s16 display, HuVecF *pos)
{
    if (pos != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_1E2C, pos);
    }
    if (display == 0) {
        Hu3DModelAttrSet(lbl_1_bss_1E2C, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_1E2C, HU3D_ATTR_DISPOFF);
    }
}

/* Update the large radial particle burst each time its registered particle hook runs. */
void fn_1_E270(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    HU3D_PARTICLE_DATA * data;
    s16 index;
    float amount;
    if (particle->count == 0)
    {
        for (index = 0, data = particle->data; index < particle->maxCnt; index++, data++)
        {
            data->time = 0;
        }
    }
    for (index = 0, data = particle->data; index < particle->maxCnt; index++, data++)
    {
        if (data->time == 0)
        {
            data->time = 1;
            data->parManId = 0;
            data->vel.x = frandmod(4) + 1;
            data->vel.y = frandmod(4) + 1;
            data->vel.z = frandmod(4) + 1;
            data->accel.x = frandmod(360);
            data->accel.y = frandmod(720) + 360;
            data->accel.z = frandmod(40) + 80;
            data->speedDecay = (0.0f);
            data->colorIdx = frandmod(180) + 60;
            data->speedDecay = frandmod(60);
            data->scale = (10.0f);
            data->color.r = ENDING_COLOR_COMPONENT_MAX;
            data->color.g = ENDING_COLOR_COMPONENT_MAX;
            data->color.b = ENDING_COLOR_COMPONENT_MAX;
            data->color.a = 0;
        }
        else
        {
            if (data->time == 1)
            {
                amount = fn_1_DDF8(0.0f, 360.0f, data->accel.x, data->accel.y);
                data->pos.x =
                    (float) (data->accel.z *
                             sin(((3.141592653589793) * (amount * data->vel.x)) / (180.0)));
                data->pos.y =
                    (float) (data->accel.z *
                             cos(((3.141592653589793) * (amount * data->vel.y)) / (180.0)));
                data->pos.z =
                    (float) (data->accel.z *
                             sin(((3.141592653589793) * (amount * data->vel.z)) / (180.0)));
                data->scale = frandmod(30) + 10;
                if (rand8() % 10 == 0)
                {
                    data->zRot = frandmod(360) * (0.01745329238474369f);
                }
                data->color.a = fn_1_DE3C(0.0f, 255.0f, data->speedDecay, data->colorIdx);
                if ((data->accel.x += (1.0f)) > data->accel.y)
                {
                    data->vel.x = frandmod(4) + 1;
                    data->vel.y = frandmod(4) + 1;
                    data->vel.z = frandmod(4) + 1;
                    data->accel.x = (0.0f);
                    data->accel.y = frandmod(720) + 360;
                    data->accel.z = frandmod(40) + 80;
                }
                if ((data->speedDecay += (1.0f)) > data->colorIdx)
                {
                    data->speedDecay = (0.0f);
                    data->colorIdx = frandmod(180) + 60;
                }
            }
        }
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * ENDING_PARTICLE_DATA_SIZE);
}

/* Create the main 16-particle burst model during ending effect setup. */
void fn_1_E9A8(void)
{
    lbl_1_bss_1E2C = Hu3DParticleCreate(lbl_1_bss_1E30[0], 16);
    Hu3DModelPosSet(lbl_1_bss_1E2C, (0.0f),
        (0.0f), (0.0f));
    Hu3DModelScaleSet(lbl_1_bss_1E2C, (1.0f),
        (1.0f), (1.0f));
    Hu3DModelLayerSet(lbl_1_bss_1E2C, 6);
    Hu3DParticleHookSet(lbl_1_bss_1E2C, fn_1_E270);
    Hu3DParticleBlendModeSet(lbl_1_bss_1E2C, 1);
}

void fn_1_EA8C(void)
{
    Hu3DModelKill(lbl_1_bss_1E2C);
}

/* Move and show or hide the pale sparkle model when the ending scene requests it. */
void fn_1_EAB8(s16 display, HuVecF *pos)
{
    if (pos != NULL) {
        Hu3DModelPosSet(lbl_1_bss_1E2A, pos->x, pos->y,
            (100.0f) + pos->z);
    }
    if (display == 0) {
        Hu3DModelAttrSet(lbl_1_bss_1E2A, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_1E2A, HU3D_ATTR_DISPOFF);
    }
}

/* Set every pale sparkle particle to the requested age before an effect change. */
void fn_1_EB54(s16 time)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E2A];
    HU3D_PARTICLE *particle = model->hookData;
    s16 i;
    HU3D_PARTICLE_DATA *data;

    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = time;
        i++;
        data++;
    }
}

/* Fade the pale sparkle particles toward their stored alpha and scale targets each hook pass. */
void fn_1_EBCC(HU3D_MODEL *model, HU3D_PARTICLE *work, Mtx mtx)
{
    HU3D_PARTICLE_DATA *entry;
    s16 index;
    f32 targetAlpha;
    f32 targetValue;

    if (work->count == 0) {
        index = 0;
        entry = work->data;
        while (index < work->maxCnt) {
            entry->time = 0;
            index++;
            entry++;
        }
    }

    index = 0;
    entry = work->data;
    while (index < work->maxCnt) {
        if (entry->time == 0) {
            entry->pos.x = 0.0f;
            entry->pos.y = 0.0f;
            entry->pos.z = 0.0f;
            if (rand8() % 5 == 0) {
                entry->color.r = ENDING_SPARKLE_GRAY_LEVEL;
                entry->color.g = ENDING_SPARKLE_GRAY_LEVEL;
                entry->color.b = ENDING_SPARKLE_GRAY_LEVEL;
                entry->vel.x = frandmod(ENDING_SPARKLE_SMALL_SPEED_VARIANCE) +
                    ENDING_SPARKLE_SMALL_SPEED_BASE;
                entry->vel.y = frandmod(ENDING_SPARKLE_SMALL_SCALE_VARIANCE) +
                    ENDING_SPARKLE_SMALL_SCALE_BASE;
            }
        } else if (entry->time == 1) {
            entry->pos.x = 0.0f;
            entry->pos.y = 0.0f;
            entry->pos.z = 0.0f;
            if (rand8() % 5 == 0) {
                entry->color.r = ENDING_SPARKLE_GRAY_LEVEL;
                entry->color.g = ENDING_SPARKLE_GRAY_LEVEL;
                entry->color.b = ENDING_SPARKLE_GRAY_LEVEL;
                entry->vel.x = frandmod(ENDING_SPARKLE_SMALL_SPEED_VARIANCE) +
                    ENDING_SPARKLE_LARGE_SPEED_BASE;
                entry->vel.y = frandmod(ENDING_SPARKLE_SMALL_SCALE_VARIANCE) +
                    ENDING_SPARKLE_LARGE_SCALE_BASE;
            }
        }

        targetAlpha = entry->vel.x;
        entry->color.a = tail23_ending_lerp(entry->color.a, targetAlpha, 10.0f);

        targetValue = entry->vel.y;
        entry->scale = tail23_ending_lerp(entry->scale, targetValue, 10.0f);
        index++;
        entry++;
    }

    DCFlushRangeNoSync(work->data, work->maxCnt * ENDING_PARTICLE_DATA_SIZE);
}

/* Create the four-particle pale sparkle model during ending effect setup. */
void fn_1_EF58(void)
{
    lbl_1_bss_1E2A = Hu3DParticleCreate(lbl_1_bss_1E30[1], 4);
    Hu3DModelPosSet(lbl_1_bss_1E2A, 0.0f,
        100.0f, 200.0f);
    Hu3DModelScaleSet(lbl_1_bss_1E2A, 1.0f,
        1.0f, 1.0f);
    Hu3DModelLayerSet(lbl_1_bss_1E2A, 6);
    Hu3DParticleHookSet(lbl_1_bss_1E2A, fn_1_EBCC);
    Hu3DParticleBlendModeSet(lbl_1_bss_1E2A, 1);
}

void fn_1_F03C(void)
{
    Hu3DModelKill(lbl_1_bss_1E2A);
}

/* Move and show or hide one of the two ending burst models when its scene effect changes. */
void fn_1_F068(s16 index, s16 display, HuVecF *pos)
{
    if (pos != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_1E26[index], pos);
    }
    if (display == 0) {
        Hu3DModelAttrSet(lbl_1_bss_1E26[index], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_1E26[index], HU3D_ATTR_DISPOFF);
    }
}

/* Select a burst mode and clear one particle model before the ending effect starts. */
void fn_1_F11C(s16 index, s16 count)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E26[index]];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    particle->dataCnt = count;
    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = 0;
        data->scale = (0.0f);
        i++;
        data++;
    }
}

/* Move and show or hide the first ending burst model when its scene effect changes. */
void fn_1_F1B8(s16 display, HuVecF *pos)
{
    if (pos != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_1E26[0], pos);
    }
    if (display == 0) {
        Hu3DModelAttrSet(lbl_1_bss_1E26[0], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_1E26[0], HU3D_ATTR_DISPOFF);
    }
}

/* Select a burst mode and clear the first particle model before the ending effect starts. */
void fn_1_F23C(s16 count)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E26[0]];
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle = model->hookData;
    s16 i;

    particle->dataCnt = count;
    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = 0;
        data->scale = (0.0f);
        i++;
        data++;
    }
}

/* Advance burst particles outward according to the selected ending effect each hook pass. */
void fn_1_F2CC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    HU3D_PARTICLE_DATA *data;
    s16 index;
    s16 initialized;

    initialized = 0;
    if (particle->count == 0) {
        for (index = 0, data = particle->data; index < particle->maxCnt; index++, data++) {
            data->time = 0;
            particle->dataCnt = 0;
        }
    }

    for (index = 0, data = particle->data; index < particle->maxCnt; index++, data++) {
        if (data->time == 0 && initialized == 0) {
            initialized = 1;
            data->time = 1;
            data->pos.x = (0.0f);
            data->pos.y = (0.0f);
            data->pos.z = (0.0f);
            data->color.r = ENDING_COLOR_COMPONENT_MAX;
            data->color.g = ENDING_COLOR_COMPONENT_MAX;
            data->color.b = ENDING_COLOR_COMPONENT_MAX;

            if (particle->dataCnt == 0) {
                data->vel.x = frandmod(100) - 50;
                data->vel.y = frandmod(50) + 25;
                data->vel.z = frandmod(100) - 50;
                PSVECNormalize(&data->vel, &data->vel);
                data->vel.x *= (3.0f);
                data->vel.y *= (2.0f);
                data->vel.z *= (3.0f);
            } else if (particle->dataCnt == 1) {
                data->vel.x = frandmod(100) - 50;
                data->vel.y = frandmod(100) - 50;
                data->vel.z = frandmod(100) - 50;
                PSVECNormalize(&data->vel, &data->vel);
                data->vel.x *= (1.5f);
                data->vel.y *= (1.5f);
                data->vel.z *= (1.5f);
            } else if (particle->dataCnt == 2) {
                data->pos.x = frandmod(2000) - 1000;
                data->pos.y = frandmod(1000);
                data->pos.z = frandmod(2000) - 1000;
                data->vel.x = frandmod(100) - 50;
                data->vel.y = -frandmod(50) - 25;
                data->vel.z = frandmod(100) - 50;
                PSVECNormalize(&data->vel, &data->vel);
            }

            data->speedDecay = (0.0f);
            data->colorIdx = frandmod(180) + 180;
            continue;
        }

        if (data->time == 1) {
            data->pos.x += data->vel.x;
            data->pos.y += data->vel.y;
            data->pos.z += data->vel.z;
            if (particle->dataCnt == 0 ||
                ((particle->dataCnt == 2 || particle->dataCnt == 1) && rand8() % 5 == 0)) {
                data->zRot = frandmod(360) * (0.01745329238474369f);
            }
            if (particle->dataCnt == 0) {
                data->scale = frandmod(10) + 5;
            } else {
                data->scale = frandmod(10) + 5;
            }

            data->color.a = fn_1_DDF8(255.0f, 0.0f, data->speedDecay, data->colorIdx);
            if ((data->speedDecay += (1.0f)) > data->colorIdx) {
                data->time = 0;
                data->scale = (0.0f);
            }
        }
    }
    DCFlushRangeNoSync(particle->data, particle->maxCnt * ENDING_PARTICLE_DATA_SIZE);
}

/* Create and hide the two large particle clouds during ending effect setup. */
void fn_1_F9B8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_1E26[i] = Hu3DParticleCreate(lbl_1_bss_1E30[0], 360);
        Hu3DModelPosSet(lbl_1_bss_1E26[i], (0.0f),
            (0.0f), (0.0f));
        Hu3DModelScaleSet(lbl_1_bss_1E26[i], (1.0f),
            (1.0f), (1.0f));
        Hu3DModelLayerSet(lbl_1_bss_1E26[i], 6);
        Hu3DParticleHookSet(lbl_1_bss_1E26[i], fn_1_F2CC);
        Hu3DParticleBlendModeSet(lbl_1_bss_1E26[i], 1);
    }
    fn_1_F068(0, 0, NULL);
    fn_1_F068(1, 0, NULL);
}

/* Destroy both large particle cloud models when the ending effect is torn down. */
void fn_1_FB74(void)
{
    s16 i;

    i = 0;
    while (i < 2) {
        Hu3DModelKill(lbl_1_bss_1E26[i]);
        i += 1;
    }
}

/* Show or hide one staff sparkle emitter when the ending scene changes its display state. */
void fn_1_FBCC(s16 index, s16 visible)
{
    if (visible != 0) {
        Hu3DModelAttrReset(lbl_1_bss_1E22[index], 1U);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_1E22[index], 1U);
    }
}

/* Activate one staff sparkle emitter with an optional position and color. */
void fn_1_FC48(s16 effectIndex, HuVecF *position, GXColor *color)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_1E22[effectIndex]];
    particle = model->hookData;
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = 1;
        if (color != NULL) {
            data->color.r = color->r;
            data->color.g = color->g;
            data->color.b = color->b;
        }
    }
    if (position != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_1E22[effectIndex], position);
    }
    Hu3DModelAttrReset(lbl_1_bss_1E22[effectIndex], HU3D_ATTR_DISPOFF);
}

/* Advance the selected staff sparkle particles to their fade-out state. */
void fn_1_FD4C(s16 effectIndex)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_1E22[effectIndex]];
    particle = model->hookData;
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = 2;
    }
}

/* Move staff sparkle particles toward their alpha and scale targets on each hook pass. */
void fn_1_FDD4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    STAFF_PARTICLE_TARGET particleTarget[5] = {
        { 16, 255 },
        { 16, 255 },
        { 48, 128 },
        { 128, 64 },
        { 255, 32 },
    };
    HU3D_PARTICLE_DATA *data;
    s16 i;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 1) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            data->vel.x = particleTarget[i % 5].alpha;
            data->vel.y = particleTarget[i % 5].scale;
            data->color.a = tail17_staff_approach_inlined(data->color.a, data->vel.x, 10);
            data->scale = tail17_staff_approach_inlined(data->scale, data->vel.y, 10);
        } else if (data->time == 2) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            data->vel.x = 0.0f;
            data->vel.y = 0.0f;
            data->color.a = tail17_staff_approach_inlined(data->color.a, data->vel.x, 10);
            data->scale = tail17_staff_approach_inlined(data->scale, data->vel.y, 10);
        } else {
            data->color.a = 0;
            data->scale = 0.0f;
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Create two hidden ten-particle sparkle emitters during ending effect setup. */
void fn_1_1025C(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_1E22[i] = Hu3DParticleCreate(lbl_1_bss_1E30[1], 10);
        Hu3DModelPosSet(lbl_1_bss_1E22[i], 0.0f,
            0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_1E22[i], 1.0f,
            1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_1E22[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_1E22[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_1E22[i], fn_1_FDD4);
        Hu3DParticleBlendModeSet(lbl_1_bss_1E22[i], 1);
    }
}

/* Destroy both staff sparkle emitter models when the ending effect is torn down. */
void fn_1_103C8(void)
{
    s16 i;

    i = 0;
    while (i < 2) {
        Hu3DModelKill(lbl_1_bss_1E22[i]);
        i += 1;
    }
}

/* Show or hide one staff burst emitter when the ending scene changes its display state. */
void fn_1_10420(s16 index, s16 visible)
{
    if (visible != 0) {
        Hu3DModelAttrReset(lbl_1_bss_1E1E[index], 1U);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_1E1E[index], 1U);
    }
}

/* Start one staff burst emitter with its color and spawn center when the scene requests it. */
void fn_1_1049C(s16 effectNo, HuVecF *position, GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_1E1E[effectNo]];
    particle = model->hookData;
    particle->dataCnt = 1;
    if (color != NULL) {
        particle->pos.x = color->r;
        particle->pos.y = color->g;
        particle->pos.z = color->b;
    }
    if (position != NULL) {
        particle->spawnCenter.x = position->x;
        particle->spawnCenter.y = position->y;
        particle->spawnCenter.z = position->z;
    }
    Hu3DModelAttrReset(lbl_1_bss_1E1E[effectNo], HU3D_ATTR_DISPOFF);
}

/* Stop spawning from one staff burst emitter. */
void fn_1_105D4(s16 effectIndex)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_1E1E[effectIndex]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

/* Spawn and animate one short staff burst particle at a time on each hook pass. */
void fn_1_10628(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;
    s16 spawnCount = 0;
    float color;
    float random;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
        particle->dataCnt = 1;
        particle->pos.x = 255.0f;
        particle->pos.y = 255.0f;
        particle->pos.z = 255.0f;
    }

    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 0 && particle->dataCnt == 1 && spawnCount == 0) {
            spawnCount = 1;
            data->time = 1;
            data->vel.x = 0.0f;
            data->vel.y = frandmod(30) + 30;
            data->accel.x = frandmod(100) - 50;
            data->accel.y = -frandmod(100) - 50;
            data->accel.z = frandmod(100) - 50;
            PSVECNormalize(&data->accel, &data->accel);
            data->accel.x *= 2.0f;
            data->accel.y *= 5.0f;
            data->accel.z *= 2.0f;
            data->pos.x = particle->spawnCenter.x + frandmod(100) - 50;
            data->pos.y = particle->spawnCenter.y + frandmod(100) - 50;
            data->pos.z = particle->spawnCenter.z + frandmod(100) - 50;
            random = frandmod(128);
            color = particle->pos.x + random;
            if (color > 255.0f) {
                color = 255.0f;
            }
            data->color.r = color;
            color = particle->pos.y + random;
            if (color > 255.0f) {
                color = 255.0f;
            }
            data->color.g = color;
            color = particle->pos.z + random;
            if (color > 255.0f) {
                color = 255.0f;
            }
            data->color.b = color;
            data->color.a = 0;
        } else if (data->time == 1) {
            data->pos.x += data->accel.x;
            data->pos.y += data->accel.y;
            data->pos.z += data->accel.z;
            if (rand8() % 5 == 0) {
                data->zRot = MTXDegToRad(frandmod(360));
                data->color.a = frandmod(127) + 128;
            }
            random = tail19_fn_1_0_inlined(1.0f, 0.0f, data->vel.x, data->vel.y);
            data->scale = frandmod(60) * random;
            if ((data->vel.x += 1.0f) > data->vel.y) {
                data->time = 0;
                data->scale = 0.0f;
                data->color.a = 0;
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Create two hidden 128-particle staff burst emitters during ending effect setup. */
void fn_1_10C28(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_1E1E[i] = Hu3DParticleCreate(lbl_1_bss_1E30[0], 128);
        Hu3DModelPosSet(lbl_1_bss_1E1E[i], 0.0f,
            0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_1E1E[i], 1.0f,
            1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_1E1E[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_1E1E[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_1E1E[i], fn_1_10628);
        Hu3DParticleBlendModeSet(lbl_1_bss_1E1E[i], 1);
    }
}

/* Destroy both staff burst emitter models when the ending effect is torn down. */
void fn_1_10D94(void)
{
    s16 i;

    i = 0;
    while (i < 2) {
        Hu3DModelKill(lbl_1_bss_1E1E[i]);
        i += 1;
    }
}

/* Start, fade, or hide a character's staff effects when its ending scene changes state. */
void fn_1_10DEC(s16 modelNo, HuVecF *pos, s16 display)
{
    GXColor colors[2] = {
        { 255, 114, 46, 0 },
        { 109, 207, 246, 0 },
    };
    if (display == 1) {
        tail20_fn_1_6C90_inlined(modelNo, pos, &colors[modelNo]);

        tail20_staff_spawn_effect_inlined(modelNo, pos, &colors[modelNo]);
    } else if (display == 0) {
        tail20_fn_1_6D94_inlined(modelNo);

        tail20_staff_stop_effect_inlined(modelNo);
    } else if (display == 2) {
        Hu3DModelAttrSet(lbl_1_bss_1E22[modelNo], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_1E1E[modelNo], HU3D_ATTR_DISPOFF);
    }
}

/* Show or hide the ending's central particle model when the scene changes its display state. */
void fn_1_111B0(s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_1E1C, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_1E1C, HU3D_ATTR_DISPOFF);
    }
}

/* Start or clear one central particle when the ending scene updates that effect slot. */
void fn_1_11208(s16 index, HuVecF *pos, s16 mode)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_1E1C];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = &particle->data[index];

    if (mode == 0) {
        data->time = 0;
        data->color.a = 0;
        data->scale = (0.0f);
    } else {
        data->time = 1;
        data->pos.x = pos->x;
        data->pos.y = pos->y;
        data->pos.z = pos->z;
        /* Red is written three times here, so green and blue keep their previous values. */
        data->color.r = 255;
        data->color.r = 255;
        data->color.r = 255;
        data->color.a = 32;
        data->scale = (100.0f);
    }
}

/* Clear the central particle model on first use and flush its data on each hook pass. */
void fn_1_112E0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;
    s16 spawnCount = 0;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt;
            i++, data++) {
            data->time = 0;
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Create the 100-particle central effect model during ending effect setup. */
void fn_1_11368(void)
{
    lbl_1_bss_1E1C = Hu3DParticleCreate(lbl_1_bss_1E30[1], 100);
    Hu3DModelPosSet(lbl_1_bss_1E1C, (0.0f),
        (0.0f), (0.0f));
    Hu3DModelScaleSet(lbl_1_bss_1E1C, (1.0f),
        (1.0f), (1.0f));
    Hu3DModelLayerSet(lbl_1_bss_1E1C, 6);
    Hu3DParticleHookSet(lbl_1_bss_1E1C, fn_1_112E0);
    Hu3DParticleBlendModeSet(lbl_1_bss_1E1C, 1);
}

void fn_1_1144C(void)
{
    Hu3DModelKill(lbl_1_bss_1E1C);
}

/* Show or hide all five particle emitters in a winner's ending effect group. */
void fn_1_11478(s16 groupNo, s16 show)
{
    s16 i;

    for (i = 0; i < 5; i++) {
        if (show) {
            Hu3DModelAttrReset(lbl_1_bss_1DE0[groupNo][i], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_1DE0[groupNo][i], HU3D_ATTR_DISPOFF);
        }
    }
}

/* Start all five winner emitters with a shared color and spawn center when requested. */
void fn_1_1152C(s16 groupNo, HuVecF *spawnPosition, GXColor *color)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *emitter;

    for (i = 0; i < 5; i++) {
        model = &Hu3DData[lbl_1_bss_1DE0[groupNo][i]];
        emitter = model->hookData;
        emitter->dataCnt = 1;
        if (color != NULL) {
            emitter->pos.x = color->r;
            emitter->pos.y = color->g;
            emitter->pos.z = color->b;
        }
        if (spawnPosition != NULL) {
            /* This emitter vector is used as the center for new particles' spawn positions. */
            emitter->spawnCenter.x = spawnPosition->x;
            emitter->spawnCenter.y = spawnPosition->y;
            emitter->spawnCenter.z = spawnPosition->z;
        }
        Hu3DModelAttrReset(lbl_1_bss_1DE0[groupNo][i], HU3D_ATTR_DISPOFF);
    }
}

/* Stop new particles in all five winner emitters when that group's burst ends. */
void fn_1_11694(s16 groupNo)
{
    s16 i;
    HU3D_MODEL *model;
    s16 *work;

    for (i = 0; i < 5; i++) {
        model = &Hu3DData[lbl_1_bss_1DE0[groupNo][i]];
        work = model->hookData;
        *work = 0;
    }
}

/* Spawn and animate a winner's burst particles on each registered particle hook pass. */
void fn_1_11714(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;
    s16 spawnCount = 0;
    float color;
    float random;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
        particle->dataCnt = 1;
        particle->pos.x = 255.0f;
        particle->pos.y = 255.0f;
        particle->pos.z = 255.0f;
    }

    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 0 && particle->dataCnt == 1 && spawnCount < 1) {
            spawnCount++;
            data->time = 1;
            data->vel.x = 0.0f;
            data->vel.y = frandmod(30) + 30;
            data->accel.x = frandmod(100) - 50;
            data->accel.y = -frandmod(100) - 50;
            data->accel.z = frandmod(100) - 50;
            PSVECNormalize(&data->accel, &data->accel);
            data->accel.x *= 2.0f;
            data->accel.y *= 4.0f;
            data->accel.z *= 2.0f;
            data->pos.x = particle->spawnCenter.x + frandmod(100) - 50;
            data->pos.y = particle->spawnCenter.y;
            data->pos.z = particle->spawnCenter.z + frandmod(100) - 50;
            random = frandmod(32);
            color = particle->pos.x + random;
            if (color > 255.0f) {
                color = 255.0f;
            }
            data->color.r = color;
            color = particle->pos.y + random;
            if (color > 255.0f) {
                color = 255.0f;
            }
            data->color.g = color;
            color = particle->pos.z + random;
            if (color > 255.0f) {
                color = 255.0f;
            }
            data->color.b = color;
            data->color.a = 0;
        } else if (data->time == 1) {
            data->pos.y += data->accel.y;
            if (rand8() % 5 == 0) {
                data->zRot = MTXDegToRad(frandmod(360));
                data->color.a = frandmod(127) + 128;
            }
            random = tail7_fn_1_3F50C(1.0f, 0.0f, data->vel.x, data->vel.y);
            data->scale = 40.0f * random;
            if (++data->vel.x > data->vel.y) {
                data->time = 0;
                data->scale = 0.0f;
                data->color.a = 0;
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Create six groups of five winner particle emitters during ending effect setup. */
void fn_1_11C94(void)
{
    EndingParticleCounts counts = { { 10, 10, 10, 10, 256 } };
    s16 group;
    s16 model;

    for (group = 0; group < 6; group++) {
        for (model = 0; model < 5; model++) {
            lbl_1_bss_1DE0[group][model] = Hu3DParticleCreate(
                lbl_1_bss_1E30[2], counts.count[model]);
            Hu3DModelPosSet(lbl_1_bss_1DE0[group][model],
                0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(lbl_1_bss_1DE0[group][model],
                1.0f, 1.0f, 1.0f);
            Hu3DModelAttrSet(lbl_1_bss_1DE0[group][model],
                HU3D_ATTR_DISPOFF);
            Hu3DModelLayerSet(lbl_1_bss_1DE0[group][model], 2);
            Hu3DParticleHookSet(lbl_1_bss_1DE0[group][model], fn_1_11714);
            Hu3DParticleBlendModeSet(lbl_1_bss_1DE0[group][model], 1);
        }
    }
}
inline void fn_1_11C94(void);

/* Destroy every winner particle emitter when the ending effect is torn down. */
void fn_1_11EB0(void)
{
    s16 row;
    s16 column;

    for (row = 0; row < 6; row++) {
        column = 0;
        while (column < 5) {
            Hu3DModelKill(lbl_1_bss_1DE0[row][column]);
            column += 1;
        }
    }
}

/* Load ending particle animations and create every effect model before the scene uses them. */
void fn_1_11F34(void)
{
    s16 index;

    for (index = 0; index < 3; index++) {
        lbl_1_bss_1E30[index] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_128[index], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    fn_1_E9A8();
    ending_inline_EF58();
    fn_1_F9B8();
    fn_1_11368();
    ending_inline_1025C();
    ending_inline_10C28();
    fn_1_11C94();
}

/* Destroy all ending particle models during scene teardown. */
void fn_1_12838(void)
{
    s16 firstIndex;
    s16 secondIndex;
    s16 thirdIndex;
    s16 model;
    s16 group;

    Hu3DModelKill(lbl_1_bss_1E2C);
    Hu3DModelKill(lbl_1_bss_1E2A);
    for (firstIndex = 0; firstIndex < 2; firstIndex++) {
        Hu3DModelKill(lbl_1_bss_1E26[firstIndex]);
    }
    Hu3DModelKill(lbl_1_bss_1E1C);
    for (secondIndex = 0; secondIndex < 2; secondIndex++) {
        Hu3DModelKill(lbl_1_bss_1E22[secondIndex]);
    }
    for (thirdIndex = 0; thirdIndex < 2; thirdIndex++) {
        Hu3DModelKill(lbl_1_bss_1E1E[thirdIndex]);
    }
    for (group = 0; group < 6; group++) {
        for (model = 0; model < 5; model++) {
            Hu3DModelKill(lbl_1_bss_1DE0[group][model]);
        }
    }
}

/* Start, stop, or hide a winner's grouped burst when the ending scene changes its mode. */
void fn_1_12988(s16 groupNo, HuVecF *pos, s16 mode, s16 colorNo)
{
    GXColor colors[7] = {
        { 254, 77, 75, 0 },
        { 50, 127, 200, 0 },
        { 199, 175, 0, 0 },
        { 52, 192, 63, 0 },
        { 159, 93, 200, 0 },
        { 255, 114, 46, 0 },
        { 109, 207, 246, 0 },
    };

    if (mode == 1) {
        tail22_fn_1_ED60_inlined(groupNo, pos, &colors[colorNo]);
    } else if (mode == 0) {
        tail22_fn_1_EEC8_inlined(groupNo);
    } else if (mode == 2) {
        tail22_fn_1_ECAC_inlined(groupNo, FALSE);
    }
}
