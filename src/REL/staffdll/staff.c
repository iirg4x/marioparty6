/* Staff-roll setup, character motions, scrolling names, and particle effects. */
#include "dolphin.h"
#include "datadir_enum.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "messdir_enum.h"
#include "msm_stream.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"
#include "REL/staffdll.h"

#define sind(x) sin((M_PI * (x)) / 180.0)
#define cosd(x) cos((M_PI * (x)) / 180.0)

typedef void (*VoidFunc)(void);

typedef struct StaffLightVectors {
    HuVecF entries[2]; /* Two light position or direction vectors; only positions use world
                        * units. */
} STAFF_LIGHT_VECTORS;

typedef struct StaffModelResource {
    s32 modelData; /* Archive entry for a staff model. */
    s32 motionData; /* Archive entry for that model's motion. */
} STAFF_MODEL_RESOURCE;

typedef struct StaffParticleTarget {
    u8 scale; /* Target particle scale. */
    u8 alpha; /* Target particle alpha. */
} STAFF_PARTICLE_TARGET;

typedef struct StaffParticleColorTable {
    GXColor color[2]; /* RGB particle tint for each of the two staff characters. */
} STAFF_PARTICLE_COLOR_TABLE;

STAFF_MODEL_RESOURCE lbl_1_data_0[13] = {
    { DATANUM(DATA_staff, 2), DATANUM(DATA_staff, 13) },
    { DATANUM(DATA_staff, 3), DATANUM(DATA_staff, 14) },
    { DATANUM(DATA_staff, 4), DATANUM(DATA_staff, 15) },
    { DATANUM(DATA_staff, 5), DATANUM(DATA_staff, 16) },
    { DATANUM(DATA_staff, 6), DATANUM(DATA_staff, 17) },
    { DATANUM(DATA_staff, 7), DATANUM(DATA_staff, 18) },
    { DATANUM(DATA_staff, 8), DATANUM(DATA_staff, 19) },
    { DATANUM(DATA_staff, 9), DATANUM(DATA_staff, 20) },
    { DATANUM(DATA_staff, 10), DATANUM(DATA_staff, 21) },
    { DATANUM(DATA_staff, 11), DATANUM(DATA_staff, 22) },
    { DATANUM(DATA_staff, 12), DATANUM(DATA_staff, 23) },
    { DATANUM(DATA_staff, 24), DATANUM(DATA_staff, 25) },
    { DATANUM(DATA_staff, 26), DATANUM(DATA_staff, 27) }
};
/* Message types and data IDs that drive the scrolling staff credits. */
typedef struct STAFF_TEXT_ENTRY {
    s16 kind; /* -1 ends new lines, 0 and 1 select line colors, 2 delays the next line while
               * scrolling continues. */
    u32 messageId; /* Message ID for a line, or the normal line-delay multiplier for kind 2. */
    s16 unknown08; /* This field is never read by the credits process. */
} STAFF_TEXT_ENTRY;

STAFF_TEXT_ENTRY lbl_1_data_68[196] = {
    { 0, MESSNUM(MESS_STAFF_NAME, 144), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 0), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 145), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 1), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 146), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 2), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 3), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 4), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 5), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 6), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 7), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 8), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 9), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 10), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 11), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 12), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 147), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 13), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 148), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 14), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 15), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 16), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 17), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 18), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 19), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 20), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 21), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 22), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 23), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 24), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 25), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 26), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 27), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 28), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 29), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 30), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 31), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 32), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 33), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 34), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 35), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 36), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 37), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 38), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 39), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 40), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 41), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 149), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 42), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 150), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 43), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 44), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 45), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 46), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 47), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 48), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 49), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 50), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 51), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 52), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 53), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 54), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 55), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 56), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 57), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 58), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 59), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 60), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 61), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 62), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 63), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 64), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 65), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 66), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 67), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 68), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 69), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 70), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 71), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 72), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 73), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 74), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 75), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 76), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 77), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 78), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 79), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 80), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 81), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 151), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 82), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 83), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 152), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 84), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 153), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 85), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 154), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 86), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 87), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 155), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 88), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 89), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 90), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 91), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 92), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 156), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 93), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 94), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 157), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 95), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 96), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 97), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 98), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 99), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 100), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 158), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 101), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 159), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 102), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 103), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 104), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 167), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 124), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 125), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 126), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 168), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 127), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 128), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 129), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 169), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 130), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 131), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 132), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 133), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 134), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 135), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 136), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 137), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 138), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 139), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 140), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 141), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 160), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 142), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 143), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 105), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 106), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 107), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 108), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 109), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 110), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 111), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 161), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 112), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 113), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 114), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 162), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 115), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 163), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 116), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 117), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 118), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 164), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 119), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 120), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 165), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 121), 0 },
    { 2, 1, 0 },
    { 0, MESSNUM(MESS_STAFF_NAME, 166), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 122), 0 },
    { 1, MESSNUM(MESS_STAFF_NAME, 123), 0 },
    { -1, -1, -1 },
};
char lbl_1_data_998[] = "gN00m1-itemhook_R";
char lbl_1_data_9AA[] = "bg03";
char lbl_1_data_9AF[] = "gN01m1-itemhook_R";
char lbl_1_data_9C1[55] = "\n-----===== MARIO PARTY 6 :: STAFF ROLL =====-----\n\n";
s32 lbl_1_data_9F8[2] = {DATANUM(DATA_staff, 41), DATANUM(DATA_staff, 42)};

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

void fn_1_884(OMOBJ *obj);
void fn_1_1110(OMOBJ *obj);
void fn_1_1758(OMOBJ *obj);
void fn_1_4668(OMOBJ *obj);
void fn_1_2BD4(OMOBJ *obj);
void fn_1_3140(OMOBJ *obj);
void fn_1_3568(OMOBJ *obj);
void fn_1_5148(OMOBJ *obj);
void fn_1_6840(void);
void fn_1_6E1C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_7670(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_7E34(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_8618(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_8FC4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_96A4(void);
void fn_1_9D14(void);
void fn_1_3E2C(OMOBJ *obj);
void fn_1_43D4(OMOBJ *obj);
void fn_1_A1F4(s16 modelNo, HuVecF *position, s16 state);
void fn_1_9E30(s16 modelNo, HuVecF *pos, s16 display);
void fn_1_A434(s16 modelNo, HuVecF *position, s16 state);

int rand8(void);
s32 frandmod(s32 modulus);
void HuAudFadeOut(s32 speed);
void HuAudSStreamFadeOut(int streamNo, s32 speed);

/* Interpolates a value over time; called by staff particle update callbacks. */
float fn_1_0(float start, float end, float time, float duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}

/* One whole sine wave around startValue, peakValue at its crest, over duration frames. */
float fn_1_44(float startValue, float peakValue, float time, float duration)
{
    if (time <= 0.0f || time >= duration) {
        return startValue;
    }
    return startValue + ((peakValue - startValue) * sind((360.0f / duration) * time));
}

/* Eases from startValue to endValue over duration frames (a quarter sine, fast start). */
float fn_1_118(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((endValue - startValue) * sind((90.0f / duration) * time));
}

/* Follows a half-sine that peaks at endValue halfway through and returns toward startValue; at
* duration it jumps to endValue. */
float fn_1_200(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((endValue - startValue) * sind((180.0f / duration) * time));
}

/* Moves current 1/steps of the remaining way to target (called every frame, an ease-out); returns
   target once current reaches it, or at once when steps is 1 or less. */
float fn_1_2E8(s16 steps, float current, float target)
{
    if ((current == target) || (steps <= 1)) {
        return target;
    }
    return (((current * (f32)(steps - 1)) + target) / (f32)steps);
}

/* Applies the frame-by-frame easing step to each staff-motion vector component. */
void fn_1_370(HuVecF *current, const HuVecF *target, float steps)
{
    current->x = fn_1_2E8((s16)steps, current->x, target->x);
    current->y = fn_1_2E8((s16)steps, current->y, target->y);
    current->z = fn_1_2E8((s16)steps, current->z, target->z);
}

/* Stores a position control point for the staff characters' entrance path. */
void fn_1_610(HuVecF *dst, float x, float y, float z)
{
    dst->x = x;
    dst->y = y;
    dst->z = z;
}

/* Evaluates one component of the staff pose's quadratic interpolation. */
float fn_1_620(float a, float b, float c, float t)
{
    float inv = 1.0f - t;

    return (c * (t * t))
        + ((a * (inv * inv)) + ((b * (inv * t)) * 2.0f));
}

/* Interpolates a staff model along three control points during its entrance. */
void fn_1_67C(
    HuVecF *dst, const HuVecF *a, const HuVecF *b, const HuVecF *c, float t)
{
    dst->x = fn_1_620(a->x, b->x, c->x, t);
    dst->y = fn_1_620(a->y, b->y, c->y, t);
    dst->z = fn_1_620(a->z, b->z, c->z, t);
}

/* Sets *value to base plus amplitude times the sine of speed * time degrees. */
static inline void StaffSineMove(f32 *value, f64 base, f64 amplitude, f32 speed, f32 time)
{
    *value = base + amplitude * sin(3.141592653589793 * (speed * time) / 180.0);
}

/* Moves the staff camera along its entrance path on each object-manager update. */
void fn_1_884(OMOBJ *obj)
{
    f32 positionX;
    f32 positionTimeX;
    f32 positionY;
    f32 positionTimeY;
    f32 positionZ;
    f32 positionTimeZ;
    f32 targetX;
    f32 targetTimeX;
    f32 targetY;
    f32 targetTimeY;
    f32 targetZ;
    f32 targetTimeZ;
    HuVecF position;
    HuVecF target;

    positionTimeX = lbl_1_bss_28;
    if (positionTimeX <= 0.0f) {
        positionX = 0.0f;
    } else if (positionTimeX >= 120.0f) {
        positionX = (-180.0f);
    } else {
        StaffSineMove(&positionX, 0.0, (-180.0), 0.75f, positionTimeX);
    }
    position.x = positionX;
    positionTimeY = lbl_1_bss_28;
    if (positionTimeY <= 0.0f) {
        positionY = 340.0f;
    } else if (positionTimeY >= 120.0f) {
        positionY = 280.0f;
    } else {
        StaffSineMove(&positionY, 340.0, (-60.0), 0.75f, positionTimeY);
    }
    position.y = positionY;
    positionTimeZ = lbl_1_bss_28;
    if (positionTimeZ <= 0.0f) {
        positionZ = 1400.0f;
    } else if (positionTimeZ >= 120.0f) {
        positionZ = 1300.0f;
    } else {
        StaffSineMove(&positionZ, 1400.0, (-100.0), 0.75f, positionTimeZ);
    }
    position.z = positionZ;
    targetTimeX = lbl_1_bss_28;
    if (targetTimeX <= 0.0f) {
        targetX = 0.0f;
    } else if (targetTimeX >= 120.0f) {
        targetX = (-180.0f);
    } else {
        StaffSineMove(&targetX, 0.0, (-180.0), 0.75f, targetTimeX);
    }
    target.x = targetX;
    targetTimeY = lbl_1_bss_28;
    if (targetTimeY <= 0.0f) {
        targetY = 340.0f;
    } else if (targetTimeY >= 120.0f) {
        targetY = 280.0f;
    } else {
        StaffSineMove(&targetY, 340.0, (-60.0), 0.75f, targetTimeY);
    }
    target.y = targetY;
    targetTimeZ = lbl_1_bss_28;
    if (targetTimeZ <= 0.0f) {
        targetZ = (-700.0f);
    } else if (targetTimeZ >= 120.0f) {
        targetZ = (-1100.0f);
    } else {
        StaffSineMove(&targetZ, (-700.0), (-400.0), 0.75f, targetTimeZ);
    }
    target.z = targetZ;
    Hu3DCameraPosSet(1, position.x, position.y, position.z,
        0.0f, 1.0f, 0.0f,
        target.x, target.y, target.z);
    lbl_1_bss_28 += 1.0f;
}

/* Creates the camera used while the staff roll is displayed. */
void fn_1_E20(void)
{
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f,
        10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
        640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPosSet(1, 0.0f, 340.0f,
        1400.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 340.0f,
        (-700.0f));
}

/* Disables the staff-roll camera during overlay cleanup. */
void fn_1_F44(void)
{
    Hu3DCameraKill(1);
}

/* Creates the two fixed lights used by staff-roll models. */
void fn_1_F68(void)
{
    STAFF_LIGHT_VECTORS positions = {{{0.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f}}};
    STAFF_LIGHT_VECTORS directions = {{{0.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}}};
    GXColor color = { 255, 255, 255, 255 };
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_82C[i] = Hu3DGLightCreateV(
            &positions.entries[i], &directions.entries[i], &color);
        Hu3DGLightInfinitytSet(lbl_1_bss_82C[i]);
        Hu3DGLightStaticSet(lbl_1_bss_82C[i], TRUE);
    }
}

/* Releases both staff-roll lights when the overlay is cleaned up. */
void fn_1_10B8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DGLightKill(lbl_1_bss_82C[i]);
    }
}

/* Called with each active credit line's vertical position; the debug hook does nothing. */
static inline void DebugValue(f32 value) { }

/* Scrolls staff credits upward and removes each message after it leaves view. */
void fn_1_1110(OMOBJ *obj)
{
    f32 movementStep = 0.5f;
    f32 messageDuration = 24.0f;
    s16 i;

    if (lbl_1_bss_12A == 0) {
        if (lbl_1_data_68[lbl_1_bss_128].kind == -1) {
            lbl_1_bss_12A = 2;
        } else {
            for (i = 0; i < 31; i++) {
                if (lbl_1_bss_2AC[i] == -1) {
                    break;
                }
            }
            if (i != 31) {
                if (lbl_1_data_68[lbl_1_bss_128].kind != 2) {
                    lbl_1_bss_12C[i].x = 0.0f;
                    lbl_1_bss_12C[i].y = 480.0f;
                    lbl_1_bss_2AC[i] = HuWinCreate(
                        lbl_1_bss_12C[i].x, lbl_1_bss_12C[i].y,
                        544, 40, 0);
                    HuWinDispOn(lbl_1_bss_2AC[i]);
                    HuWinBGTPLvlSet(lbl_1_bss_2AC[i],
                        0.0f);
                    lbl_1_bss_124 = messageDuration;
                    lbl_1_bss_12A = 1;

                    if (lbl_1_data_68[lbl_1_bss_128].kind == 0) {
                        HuWinMesColSet(lbl_1_bss_2AC[i],
                            HUWIN_MESCOL_GREEN);
                    } else {
                        HuWinMesColSet(lbl_1_bss_2AC[i],
                            HUWIN_MESCOL_WHITE);
                    }
                    HuWinAttrSet(lbl_1_bss_2AC[i],
                        HUWIN_ATTR_OUTLINE | HUWIN_ATTR_ALIGN_RIGHT);
                    HuWinMesSet(lbl_1_bss_2AC[i],
                        lbl_1_data_68[lbl_1_bss_128].messageId);
                    HuWinMesSpeedSet(lbl_1_bss_2AC[i], 0);
                } else {
                    lbl_1_bss_124 = messageDuration *
                        (f32)lbl_1_data_68[lbl_1_bss_128].messageId;
                    lbl_1_bss_12A = 1;
                }
            }
        }
    } else if (lbl_1_bss_12A == 1) {
        lbl_1_bss_124 -= movementStep;
        if (lbl_1_bss_124 < 0.0f) {
            lbl_1_bss_12A = 0;
            lbl_1_bss_128++;
        }
    } else if (lbl_1_bss_12A == 2) {
        for (i = 0; i < 31; i++) {
            if (lbl_1_bss_2AC[i] != -1) {
                break;
            }
        }
        if (i == 31) {
            obj->objFunc = NULL;
        }
    }

    for (i = 0; i < 31; i++) {
        if (lbl_1_bss_2AC[i] != -1) {
            DebugValue(lbl_1_bss_12C[i].y);
            lbl_1_bss_12C[i].y -= movementStep;
            HuWinPosSet(lbl_1_bss_2AC[i], lbl_1_bss_12C[i].x,
                lbl_1_bss_12C[i].y);
            if (lbl_1_bss_12C[i].y <= -60.0f) {
                HuWinKill(lbl_1_bss_2AC[i]);
                lbl_1_bss_2AC[i] = -1;
            }
        }
    }
}

/* Clears the credit-window slots and installs their per-frame update callback. */
void fn_1_1684(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 31; i++) {
        lbl_1_bss_2AC[i] = -1;
        lbl_1_bss_12C[i].x = 0.0f;
        lbl_1_bss_12C[i].y = 0.0f;
    }
    lbl_1_bss_12A = 0;
    lbl_1_bss_128 = 0;
    lbl_1_bss_124 = 0.0f;
    obj->objFunc = fn_1_1110;
}

/* Animates both staff models into place and updates their attached particles. */
void fn_1_1758(OMOBJ *obj)
{
    OMOBJ *first = lbl_1_bss_8;
    OMOBJ *second = lbl_1_bss_C;
    STAFF_MOTION_WORK *firstWork = &lbl_1_bss_5C[0];
    STAFF_MOTION_WORK *secondWork = &lbl_1_bss_5C[1];
    HuVecF initial[3] = {
        { -380.0f, 250.0f, 0.0f },
        { 20.0f, 250.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    };
    HuVecF position;

    if (firstWork->state == 0) {
        fn_1_370(&firstWork->orbitRadius, &firstWork->orbitRadiusTarget, 60.0f);
        position.x = firstWork->orbitRadius.x * sind(firstWork->angleOrFrame - 90.0f) - 180.0;
        position.y = -(firstWork->orbitRadius.y * cosd(firstWork->angleOrFrame)) + 350.0;
        position.z = firstWork->orbitRadius.z * sind(firstWork->angleOrFrame);
        Hu3DModelPosSetV(first->mdlId[0], &position);
        Hu3DModelRotSet(first->mdlId[0], -firstWork->angleOrFrame + 45.0f, 0.0f,
            -45.0f + 10.0f * ((firstWork->orbitRadius.y - 100.0f) / 200.0f));
        if ((firstWork->angleOrFrame += firstWork->angleStep) > 270.0f) {
            firstWork->angleOrFrame -= 360.0f;
            firstWork->orbitRadiusTarget.x = frandmod(100) + 150;
            firstWork->orbitRadiusTarget.y = frandmod(100) + 150;
            firstWork->orbitRadiusTarget.z = 200.0f;
        }
        if (obj->work[1] == 1) {
            firstWork->state = 1;
            firstWork->angleOrFrame = 0.0f;
            Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[1], 0.0f, 30.0f, HU3D_MOTATTR_LOOP);
            Hu3DModelPosGet(first->mdlId[0], &firstWork->position);
            fn_1_610(&firstWork->curveControl, (firstWork->position.x + initial[0].x) / 2,
                2.0f * ((firstWork->position.y + initial[0].y) / 2),
                2.0f * ((firstWork->position.z + initial[0].z) / 2));
            fn_1_610(&firstWork->curveEnd, initial[0].x, initial[0].y, initial[0].z + 200.0f);
            Hu3DModelRotGet(first->mdlId[0], &firstWork->rotation);
        }
    } else if (firstWork->state == 1) {
        fn_1_67C(&position, &firstWork->position, &firstWork->curveControl, &firstWork->curveEnd,
            fn_1_118(0.0f, 1.0f, firstWork->angleOrFrame, 120.0f));
        Hu3DModelPosSetV(first->mdlId[0], &position);
        fn_1_370(&firstWork->rotation, &initial[2], 10.0f);
        Hu3DModelRotSetV(first->mdlId[0], &firstWork->rotation);
        if ((firstWork->angleOrFrame += 1.0f) > 121.0f) {
            firstWork->state = 2;
        }
    }

    if (secondWork->state == 0) {
        fn_1_370(&secondWork->orbitRadius, &secondWork->orbitRadiusTarget, 60.0f);
        position.x = secondWork->orbitRadius.x * sind(90.0f + secondWork->angleOrFrame) - 180.0;
        position.y = -(secondWork->orbitRadius.y * cosd(secondWork->angleOrFrame)) + 350.0;
        position.z = secondWork->orbitRadius.z * sind(secondWork->angleOrFrame);
        Hu3DModelPosSetV(second->mdlId[0], &position);
        Hu3DModelRotSet(second->mdlId[0], -secondWork->angleOrFrame + 45.0f, 0.0f,
            45.0f - 10.0f * ((secondWork->orbitRadius.y - 100.0f) / 200.0f));
        if ((secondWork->angleOrFrame += secondWork->angleStep) > 270.0f) {
            secondWork->angleOrFrame -= 360.0f;
            secondWork->orbitRadiusTarget.x = frandmod(100) + 150;
            secondWork->orbitRadiusTarget.y = frandmod(100) + 150;
            secondWork->orbitRadiusTarget.z = 200.0f;
        }
        if (obj->work[1] == 1) {
            secondWork->state = 1;
            secondWork->angleOrFrame = 0.0f;
            Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[1], 0.0f, 30.0f, HU3D_MOTATTR_LOOP);
            Hu3DModelPosGet(second->mdlId[0], &secondWork->position);
            fn_1_610(&secondWork->curveControl, (secondWork->position.x + initial[1].x) / 2,
                2.0f * ((secondWork->position.y + initial[1].y) / 2),
                2.0f * ((secondWork->position.z + initial[1].z) / 2));
            fn_1_610(&secondWork->curveEnd, initial[1].x, initial[1].y, initial[1].z + 200.0f);
            Hu3DModelRotGet(second->mdlId[0], &secondWork->rotation);
        }
    } else if (secondWork->state == 1) {
        fn_1_67C(&position, &secondWork->position, &secondWork->curveControl, &secondWork->curveEnd,
            fn_1_118(0.0f, 1.0f, secondWork->angleOrFrame, 120.0f));
        Hu3DModelPosSetV(second->mdlId[0], &position);
        fn_1_370(&secondWork->rotation, &initial[2], 10.0f);
        Hu3DModelRotSetV(second->mdlId[0], &secondWork->rotation);
        if ((secondWork->angleOrFrame += 1.0f) > 121.0f) {
            secondWork->state = 2;
        }
    }

    Hu3DModelPosGet(first->mdlId[0], &position);
    fn_1_A434(0, &position, 1);
    Hu3DModelPosGet(second->mdlId[0], &position);
    fn_1_A434(1, &position, 1);
}

/* Resets both staff motion paths before their object callback begins. */
void fn_1_29D4(void)
{
    OMOBJ *obj = lbl_1_bss_8;
    STAFF_MOTION_WORK *firstWork = &lbl_1_bss_5C[0];
    STAFF_MOTION_WORK *secondWork = &lbl_1_bss_5C[1];

    firstWork->angleOrFrame = 0.0f;
    firstWork->angleStep = 0.5f;
    firstWork->state = 0;
    firstWork->orbitRadiusTarget.x = firstWork->orbitRadius.x = frandmod(100) + 100;
    firstWork->orbitRadiusTarget.y = firstWork->orbitRadius.y = frandmod(200) + 100;
    firstWork->orbitRadiusTarget.z = firstWork->orbitRadius.z = 200.0f;

    secondWork->angleOrFrame = 270.0f;
    secondWork->angleStep = 0.5f;
    secondWork->state = 0;
    secondWork->orbitRadiusTarget.x = secondWork->orbitRadius.x = frandmod(100) + 100;
    secondWork->orbitRadiusTarget.y = secondWork->orbitRadius.y = frandmod(200) + 100;
    secondWork->orbitRadiusTarget.z = secondWork->orbitRadius.z = 200.0f;
    obj->work[1] = 0;
    obj->objFunc = fn_1_1758;
}

/* Cues both staff models to leave their orbits and starts the camera entrance. */
void fn_1_2B94(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[1] = 1;
    lbl_1_bss_C->objFunc = fn_1_884;
}

/* Raises both characters' particle effects along circular paths. */
void fn_1_2BD4(OMOBJ *obj)
{
    STAFF_MOTION_WORK *firstWork;
    STAFF_MOTION_WORK *secondWork;
    HuVecF target;

    firstWork = &lbl_1_bss_5C[0];
    secondWork = &lbl_1_bss_5C[1];
    target.x = 200.0 *
        sin(3.141592653589793 * (f64)obj->work[0] / 180.0) -
        180.0;
    target.y = 50.0f + firstWork->position.y;
    if (target.y > 600.0f) {
        fn_1_A1F4(0, NULL, 1);
    }
    target.z = 200.0 *
        cos(3.141592653589793 * (f64)obj->work[0] / 180.0);
    fn_1_370(&firstWork->position, &target, 10.0f);

    target.x = 200.0 *
        sin(3.141592653589793 * (f64)obj->work[1] / 180.0) -
        180.0;
    target.y = 50.0f + secondWork->position.y;
    if (target.y > 600.0f) {
        fn_1_A1F4(1, NULL, 1);
    }
    target.z = 200.0 *
        cos(3.141592653589793 * (f64)obj->work[1] / 180.0);
    fn_1_370(&secondWork->position, &target, 10.0f);

    if ((obj->work[0] += 4) > 360) {
        obj->work[0] -= 360;
    }
    if ((obj->work[1] += 4) > 360) {
        obj->work[1] -= 360;
    }
    fn_1_9E30(0, &firstWork->position, 1);
    fn_1_9E30(1, &secondWork->position, 1);
}

void fn_1_9E30(s16 modelNo, HuVecF *pos, s16 display);

/* Advances staff motions through their entrance, pose, and looping sequence. */
void fn_1_3140(OMOBJ *obj)
{
    OMOBJ *first = lbl_1_bss_8;
    OMOBJ *second = lbl_1_bss_C;
    STAFF_MOTION_WORK *firstWork = &lbl_1_bss_5C[0];
    STAFF_MOTION_WORK *secondWork = &lbl_1_bss_5C[1];

    if (obj->work[0] < 30) {
        Hu3DModelObjPosGet(
            first->mdlId[0], lbl_1_data_998, &firstWork->position);
        Hu3DModelObjPosGet(
            second->mdlId[0], lbl_1_data_998, &secondWork->position);
        fn_1_9E30(0, &firstWork->position, 1);
        fn_1_9E30(1, &secondWork->position, 1);
    }
    if (obj->work[0] == 30) {
        Hu3DModelObjPosGet(
            first->mdlId[0], lbl_1_data_998, &firstWork->position);
        Hu3DModelObjPosGet(
            second->mdlId[0], lbl_1_data_998, &secondWork->position);
        lbl_1_bss_C->work[0] = 0;
        lbl_1_bss_C->work[1] = 180;
        lbl_1_bss_C->objFunc = fn_1_2BD4;
    }
    if (obj->work[0] == 60) {
        Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[3],
            0.0f, 30.0f, 0);
        Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[3],
            0.0f, 30.0f, 0);
    } else if (obj->work[0] == 180) {
        Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[4],
            0.0f, 30.0f, HU3D_MOTATTR_LOOP);
        Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[4],
            0.0f, 30.0f, HU3D_MOTATTR_LOOP);
        lbl_1_bss_8->objFunc = NULL;
    }
    obj->work[0]++;
}

/* Starts motion slot 2 for both staff models and schedules the timed pose sequence. */
void fn_1_3378(void)
{
    OMOBJ *obj = lbl_1_bss_8;
    OMOBJ *first = lbl_1_bss_8;
    OMOBJ *second = lbl_1_bss_C;
    STAFF_MOTION_WORK *firstWork = &lbl_1_bss_5C[0];
    STAFF_MOTION_WORK *secondWork = &lbl_1_bss_5C[1];

    Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[2],
        0.0f, 30.0f, 0);
    Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[2],
        0.0f, 30.0f, 0);
    obj->work[0] = 0;
    obj->objFunc = fn_1_3140;
}

/* Shows the staff name sprite and begins its fade-in. */
void fn_1_3460(void)
{
    HuSprAttrReset(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
    HuSprTPLvlSet(lbl_1_bss_82A, 0, 0.0f);
    lbl_1_bss_30 = 1.0f;
    lbl_1_bss_2C = 0.0f;
}

/* Hides the staff name sprite and resets its fade state. */
void fn_1_34E4(void)
{
    HuSprTPLvlSet(lbl_1_bss_82A, 0, 0.0f);
    HuSprAttrSet(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_30 = 0.0f;
    lbl_1_bss_2C = 0.0f;
}

/* Rotates the staff emblem and fades the name sprite in during its update. */
void fn_1_3568(OMOBJ *obj)
{
    HuVecF rotation;
    f32 positionOffset;
    f32 bobOffset;
    f32 frame = (f32)obj->work[1];

    if ((frame <= 0.0f) || (frame >= 360.0f)) {
        bobOffset = 0.0f;
    } else {
        StaffSineMove(&bobOffset, 0.0, 10.0, 1.0f, frame);
    }
    positionOffset = bobOffset;
    Hu3DModelPosSet(lbl_1_bss_4->mdlId[0], (-180.0f),
        325.0f + positionOffset, 0.0f);
    if (++obj->work[1] > 360) {
        obj->work[1] -= 360;
    }
    Hu3DModelRotGet(lbl_1_bss_4->mdlId[0], &rotation);
    rotation.x += 0.04f;
    if (rotation.x >= 360.0f) {
        rotation.x -= 360.0f;
    }
    rotation.y += 0.12f;
    if (rotation.y >= 360.0f) {
        rotation.y -= 360.0f;
    }
    rotation.z += 0.08f;
    if (rotation.z >= 360.0f) {
        rotation.z -= 360.0f;
    }
    Hu3DModelRotSetV(lbl_1_bss_4->mdlId[0], &rotation);
    if (1.0f == lbl_1_bss_30) {
        lbl_1_bss_2C += 0.02f;
        if (lbl_1_bss_2C > 1.0f) {
            lbl_1_bss_2C = 1.0f;
        }
        HuSprTPLvlSet(lbl_1_bss_82A, 0, lbl_1_bss_2C);
    }
}

/* Loads the rotating staff emblem and starts its scrolling background. */
void fn_1_389C(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 2; i++) {
        obj->mdlId[i] = Hu3DModelCreateData(DATANUM(DATA_staff, 0) + i);
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    }
    obj->work[0] = Hu3DTexScrollCreate(obj->mdlId[1], lbl_1_data_9AA);
    Hu3DTexScrollPosMoveSet(
        obj->work[0], 0.0f, (-0.001f), 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 2.0f, 2.0f,
        2.0f);
    obj->objFunc = fn_1_3568;
}

/* Releases the emblem's motions and models when its object is removed. */
void fn_1_3A08(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 2; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* Loads the first staff character and its five joint motions. */
void fn_1_3A98(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(DATANUM(DATA_staff, 28));
    for (i = 0; i < 5; i++) {
        obj->mtnId[i] = Hu3DJointMotionData(obj->mdlId[0], DATANUM(DATA_staff, 29) + i);
    }
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f,
        0.0f, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(obj->mdlId[0], 0.85f,
        0.85f, 0.85f);
    obj->objFunc = NULL;
}

/* Releases the first staff character's motions, model, and object. */
void fn_1_3BB0(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 5; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* Loads the second staff character and hooks its hand prop to the model. */
void fn_1_3C38(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(DATANUM(DATA_staff, 34));
    obj->mdlId[1] = Hu3DModelCreateData(DATANUM(DATA_staff, 35));
    for (i = 0; i < 5; i++) {
        obj->mtnId[i] = Hu3DJointMotionData(obj->mdlId[0], DATANUM(DATA_staff, 36) + i);
    }
    Hu3DModelHookSet(obj->mdlId[0], lbl_1_data_9AF, obj->mdlId[1]);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f,
        0.0f, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(obj->mdlId[0], 0.85f,
        0.85f, 0.85f);
    obj->objFunc = NULL;
}

/* Detaches the hand prop and releases the second staff character's resources. */
void fn_1_3D8C(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        Hu3DModelHookReset(obj->mdlId[0]);
        for (i = 0; i < 5; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        Hu3DModelKill(obj->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* Moves the thirteen background figures across the staff-roll camera each frame. */
void fn_1_3E2C(OMOBJ *obj)
{
    s16 i;
    f32 *work;

    /* Each figure uses 25 floats: a direction halfword at the start, a delay at [1],
    * XYZ position at [3..5], and speed at [6]. The negative delay counts up;
    * movement starts once it becomes positive. */
    work = lbl_1_bss_310;
    for (i = 0; i < 13; i++, work += 25) {
        if (work[1] > 0.0f) {
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            if (((s16 *)work)[0] == 0) {
                work[3] -= work[6];
                if (work[3] < (-1500.0f)) {
                    ((s16 *)work)[0] = rand8() % 2;
                    work[1] = -frandmod(600);
                    if (((s16 *)work)[0] == 0) {
                        work[3] = 1500.0f;
                        work[4] = frandmod(800) - 25;
                        work[5] = -200 - i * 100;
                        Hu3DModelRotSet(obj->mdlId[i], 0.0f, (-90.0f), 0.0f);
                    } else {
                        work[3] = (-1500.0f);
                        work[4] = frandmod(800) - 25;
                        work[5] = -200 - i * 100;
                        Hu3DModelRotSet(obj->mdlId[i], 0.0f, 90.0f, 0.0f);
                    }
                    work[6] = frandmod(8) + 2;
                    Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
                    Hu3DMotionSpeedSet(obj->mdlId[i], 1.0f + work[6] / 10.0f);
                }
            } else {
                work[3] += work[6];
                if (work[3] > 1500.0f) {
                    ((s16 *)work)[0] = rand8() % 2;
                    work[1] = -frandmod(600);
                    if (((s16 *)work)[0] == 0) {
                        work[3] = 1500.0f;
                        work[4] = frandmod(800) - 25;
                        work[5] = -200 - i * 100;
                        Hu3DModelRotSet(obj->mdlId[i], 0.0f, (-90.0f), 0.0f);
                    } else {
                        work[3] = (-1500.0f);
                        work[4] = frandmod(800) - 25;
                        work[5] = -200 - i * 100;
                        Hu3DModelRotSet(obj->mdlId[i], 0.0f, 90.0f, 0.0f);
                    }
                    work[6] = frandmod(8) + 2;
                    Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
                    Hu3DMotionSpeedSet(obj->mdlId[i], 1.0f + work[6] / 10.0f);
                }
            }
            Hu3DModelPosSetV(obj->mdlId[i], (HuVecF *)&work[3]);
        } else {
            work[1] += 1.0f;
        }
    }
}

/* Randomizes figure lanes and speeds before installing their update callback. */
void fn_1_43D4(OMOBJ *obj)
{
    s16 i;
    f32 *work;

    work = lbl_1_bss_310;
    for (i = 0; i < 13; i++, work += 25) {
        ((s16 *)work)[0] = rand8() % 2;
        work[1] = -frandmod(600);
        if (((s16 *)work)[0] == 0) {
            work[3] = 1500.0f;
            work[4] = frandmod(800) - 25;
            work[5] = -200 - (i * 100);
            Hu3DModelRotSet(obj->mdlId[i], 0.0f,
                (-90.0f), 0.0f);
        } else {
            work[3] = (-1500.0f);
            work[4] = frandmod(800) - 25;
            work[5] = -200 - (i * 100);
            Hu3DModelRotSet(obj->mdlId[i], 0.0f,
                90.0f, 0.0f);
        }
        work[6] = frandmod(8) + 2;
        Hu3DMotionSpeedSet(obj->mdlId[i],
            1.0f + (work[6] / 10.0f));
    }
    obj->objFunc = fn_1_3E2C;
}

/* Sends each background figure across the screen once, then disables its draw flag. */
void fn_1_4668(OMOBJ *obj)
{
    float horizontalStep = 2.6f;
    s16 modelIndex = (s16)obj->work[0];
    HuVecF position;

    if (obj->work[1] == 0) {
        obj->work[1] = 1;
        obj->work[2] = rand8() % 2;
        if (rand8() % 2 == 0) {
            Hu3DModelPosSet(obj->mdlId[modelIndex], 900.0f,
                (f32)(frandmod(200) + 525), (-500.0f));
        } else {
            Hu3DModelPosSet(obj->mdlId[modelIndex], 900.0f,
                (f32)(-frandmod(200) + 125), (-500.0f));
        }
        Hu3DModelPosGet(obj->mdlId[modelIndex], &position);
        Hu3DModelRotSet(obj->mdlId[modelIndex], 0.0f, (-90.0f),
            0.0f);
        if (obj->work[2] == 1) {
            position.x = (-900.0f);
            Hu3DModelPosSetV(obj->mdlId[modelIndex], &position);
            Hu3DModelRotSet(obj->mdlId[modelIndex], 0.0f, 90.0f,
                0.0f);
        }
        Hu3DModelAttrReset(obj->mdlId[modelIndex], HU3D_ATTR_DISPOFF);
        lbl_1_bss_50[0] = (f32)(frandmod(10) - 5) * 0.1f;
        lbl_1_bss_50[1] = (f32)(frandmod(10) - 5) * 0.1f;
        lbl_1_bss_50[2] = (f32)(frandmod(10) - 5) * 0.1f;
    }

    if (obj->work[1] == 1) {
        Hu3DModelPosGet(obj->mdlId[modelIndex], &position);
        if (obj->work[2] == 0) {
            position.x -= horizontalStep;
        } else {
            position.x += horizontalStep;
        }
        Hu3DModelPosSetV(obj->mdlId[modelIndex], &position);
        if (((position.x < (-900.0f)) && (obj->work[2] == 0)) ||
            ((position.x > 900.0f) && (obj->work[2] == 1))) {
            obj->work[1] = 0;
            obj->work[0]++;
            if (obj->work[0] >= 13) {
                obj->objFunc = NULL;
                obj->work[0] = 0;
            }
            Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_ATTR_DISPOFF);
        }

        /* The index is reset when it reaches 13 above, so this rotation block is never reached. */
        if (obj->work[0] >= 13) {
            Hu3DModelRotGet(obj->mdlId[modelIndex], &position);
            position.x += lbl_1_bss_50[0];
            position.y += lbl_1_bss_50[1];
            position.z += lbl_1_bss_50[2];
            Hu3DModelRotSetV(obj->mdlId[modelIndex], &position);
        }
    }
}

/* Loads and hides the thirteen background figures with looping motions. */
void fn_1_4AF4(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 13; i++) {
        obj->mdlId[i] = Hu3DModelCreateData(lbl_1_data_0[i].modelData);
        obj->mtnId[i] = Hu3DJointMotionData(
            obj->mdlId[i], lbl_1_data_0[i].motionData);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    }
    obj->work[0] = 0;
    obj->work[1] = 0;
    obj->objFunc = NULL;
}

/* Releases every background figure motion and model during overlay cleanup. */
void fn_1_4C40(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 13; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* Releases the emblem, both characters, background figures, particles, lights and camera. */
void fn_1_4CD0(void)
{
    OMOBJ *first;
    OMOBJ *second;
    OMOBJ *third;
    OMOBJ *fourth;
    s16 firstIndex;
    s16 secondIndex;
    s16 thirdIndex;
    s16 fourthIndex;
    s16 lightIndex;

    first = lbl_1_bss_4;
    if (first) {
        for (firstIndex = 0; firstIndex < 2; firstIndex++) {
            Hu3DMotionKill(first->mtnId[firstIndex]);
            Hu3DModelKill(first->mdlId[firstIndex]);
        }
        omDelObjEx(lbl_1_bss_0, first);
    }
    first = NULL;
    second = lbl_1_bss_8;
    if (second) {
        for (secondIndex = 0; secondIndex < 5; secondIndex++) {
            Hu3DMotionKill(second->mtnId[secondIndex]);
        }
        Hu3DModelKill(second->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, second);
    }
    second = NULL;
    third = lbl_1_bss_C;
    if (third) {
        Hu3DModelHookReset(third->mdlId[0]);
        for (thirdIndex = 0; thirdIndex < 5; thirdIndex++) {
            Hu3DMotionKill(third->mtnId[thirdIndex]);
        }
        Hu3DModelKill(third->mdlId[0]);
        Hu3DModelKill(third->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, third);
    }
    third = NULL;
    fourth = lbl_1_bss_10;
    if (fourth) {
        for (fourthIndex = 0; fourthIndex < 13; fourthIndex++) {
            Hu3DMotionKill(fourth->mtnId[fourthIndex]);
            Hu3DModelKill(fourth->mdlId[fourthIndex]);
        }
        omDelObjEx(lbl_1_bss_0, fourth);
    }
    fourth = NULL;
    fn_1_9D14();
    for (lightIndex = 0; lightIndex < 2; lightIndex++) {
        Hu3DGLightKill(lbl_1_bss_82C[lightIndex]);
    }
    Hu3DCameraKill(1);
}

/* Waits for the outgoing wipe, then removes staff resources and returns overlay. */
void fn_1_4EF4(OMOBJ *obj)
{
    OMOBJ *first;
    OMOBJ *second;
    OMOBJ *third;
    OMOBJ *fourth;
    s16 firstIndex;
    s16 secondIndex;
    s16 thirdIndex;
    s16 fourthIndex;
    s16 lightIndex;

    if (WipeCheck() == 0) {
        HuAudFadeOut(1000);
        first = lbl_1_bss_4;
        if (first) {
            for (firstIndex = 0; firstIndex < 2; firstIndex++) {
                Hu3DMotionKill(first->mtnId[firstIndex]);
                Hu3DModelKill(first->mdlId[firstIndex]);
            }
            omDelObjEx(lbl_1_bss_0, first);
        }
        first = NULL;
        second = lbl_1_bss_8;
        if (second) {
            for (secondIndex = 0; secondIndex < 5; secondIndex++) {
                Hu3DMotionKill(second->mtnId[secondIndex]);
            }
            Hu3DModelKill(second->mdlId[0]);
            omDelObjEx(lbl_1_bss_0, second);
        }
        second = NULL;
        third = lbl_1_bss_C;
        if (third) {
            Hu3DModelHookReset(third->mdlId[0]);
            for (thirdIndex = 0; thirdIndex < 5; thirdIndex++) {
                Hu3DMotionKill(third->mtnId[thirdIndex]);
            }
            Hu3DModelKill(third->mdlId[0]);
            Hu3DModelKill(third->mdlId[1]);
            omDelObjEx(lbl_1_bss_0, third);
        }
        third = NULL;
        fourth = lbl_1_bss_10;
        if (fourth) {
            for (fourthIndex = 0; fourthIndex < 13; fourthIndex++) {
                Hu3DMotionKill(fourth->mtnId[fourthIndex]);
                Hu3DModelKill(fourth->mdlId[fourthIndex]);
            }
            omDelObjEx(lbl_1_bss_0, fourth);
        }
        fourth = NULL;
        fn_1_9D14();
        for (lightIndex = 0; lightIndex < 2; lightIndex++) {
            Hu3DGLightKill(lbl_1_bss_82C[lightIndex]);
        }
        Hu3DCameraKill(1);
        omOvlReturnEx(1, 1);
        obj->objFunc = NULL;
    }
}

/* Handles the ending-view prompt and starts the exit wipe when Start is pressed. */
void fn_1_5148(OMOBJ *obj)
{
    HuWinDispOff(lbl_1_bss_304[4]);
    if (WipeCheck() != 0) {
        return;
    }
    if (lbl_1_bss_24 == 1) {
        obj->work[0]++;
        if (obj->work[0] < 60) {
            return;
        }
    } else {
        obj->work[0] = 0;
        return;
    }
    HuWinDispOn(lbl_1_bss_304[4]);
    HuWinMesSet(lbl_1_bss_304[4], MESSNUM(MESS_SYS_GUIDE, 5));
    HuWinMesSpeedSet(lbl_1_bss_304[4], 0);
    switch (lbl_1_bss_34) {
    case 0:
        if (lbl_1_bss_24 != 1) {
            return;
        }
        if ((HuPadBtnDown[0] & 0x1000) == 0) {
            return;
        }
        lbl_1_bss_34 = 1;
        HuAudSStreamFadeOut(lbl_1_bss_824, 1000);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        obj->objFunc = fn_1_4EF4;
        break;
    }
}

/* Plays the exit transition, releases staff resources, and returns the overlay. */
void fn_1_528C(void)
{
    OMOBJ *first;
    OMOBJ *second;
    OMOBJ *third;
    OMOBJ *fourth;
    s16 firstIndex;
    s16 secondIndex;
    s16 thirdIndex;
    s16 fourthIndex;
    s16 lightIndex;

    fn_1_6840();
    HuAudSStreamFadeOut(lbl_1_bss_824, 1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    first = lbl_1_bss_4;
    if (first) {
        for (firstIndex = 0; firstIndex < 2; firstIndex++) {
            Hu3DMotionKill(first->mtnId[firstIndex]);
            Hu3DModelKill(first->mdlId[firstIndex]);
        }
        omDelObjEx(lbl_1_bss_0, first);
    }
    first = NULL;
    second = lbl_1_bss_8;
    if (second) {
        for (secondIndex = 0; secondIndex < 5; secondIndex++) {
            Hu3DMotionKill(second->mtnId[secondIndex]);
        }
        Hu3DModelKill(second->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, second);
    }
    second = NULL;
    third = lbl_1_bss_C;
    if (third) {
        Hu3DModelHookReset(third->mdlId[0]);
        for (thirdIndex = 0; thirdIndex < 5; thirdIndex++) {
            Hu3DMotionKill(third->mtnId[thirdIndex]);
        }
        Hu3DModelKill(third->mdlId[0]);
        Hu3DModelKill(third->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, third);
    }
    third = NULL;
    fourth = lbl_1_bss_10;
    if (fourth) {
        for (fourthIndex = 0; fourthIndex < 13; fourthIndex++) {
            Hu3DMotionKill(fourth->mtnId[fourthIndex]);
            Hu3DModelKill(fourth->mdlId[fourthIndex]);
        }
        omDelObjEx(lbl_1_bss_0, fourth);
    }
    fourth = NULL;
    fn_1_9D14();
    for (lightIndex = 0; lightIndex < 2; lightIndex++) {
        Hu3DGLightKill(lbl_1_bss_82C[lightIndex]);
    }
    Hu3DCameraKill(1);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

/* Builds the staff-roll scene and starts its process. */
void fn_1_54F0(void)
{
    lbl_1_bss_0 = omInitObjMan(11, 8192);
    HuWinInit(1);
    fn_1_E20();
    fn_1_F68();
    fn_1_96A4();

    lbl_1_bss_4 = omAddObjEx(
        lbl_1_bss_0, 4096, 2, 2, OM_GRP_NONE, fn_1_389C);
    /* The character loaders store five motion IDs although these objects request four slots. */
    lbl_1_bss_8 = omAddObjEx(
        lbl_1_bss_0, 4096, 2, 4, OM_GRP_NONE, fn_1_3A98);
    lbl_1_bss_C = omAddObjEx(
        lbl_1_bss_0, 4096, 2, 4, OM_GRP_NONE, fn_1_3C38);
    lbl_1_bss_18 = omAddObjEx(
        lbl_1_bss_0, 4096, 0, 0, OM_GRP_NONE, fn_1_1684);

    lbl_1_bss_20 = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_staff, 43), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_82A = HuSprGrpCreate(1);
    lbl_1_bss_828 = HuSprCreate(lbl_1_bss_20, 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_82A, 0, lbl_1_bss_828);
    HuSprPosSet(
        lbl_1_bss_82A, 0, 288.0f, 400.0f);
    HuSprTPLvlSet(lbl_1_bss_82A, 0, 0.0f);
    HuSprAttrSet(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_30 = 0.0f;
    lbl_1_bss_2C = 0.0f;

    {
        s16 modelCount = 13;

        lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 4096,
            modelCount, modelCount, OM_GRP_NONE, fn_1_4AF4);
    }
    if (GwCommon.viewEnding) {
        lbl_1_bss_304[4] = HuWinExCreateFrame(16.0f,
            400.0f, 544, 42, -1, 0);
        HuWinDispOff(lbl_1_bss_304[4]);
        HuWinBGTPLvlSet(lbl_1_bss_304[4], 0.0f);
        lbl_1_bss_1C = omAddObjEx(
            lbl_1_bss_0, 4096, 0, 0, OM_GRP_NONE, fn_1_5148);
    }
    /* Mark the ending viewed even on visits that did not create the Start-to-exit prompt. */
    GwCommon.viewEnding = TRUE;
    HuPrcChildCreate(fn_1_528C, 12288, 12288, 0, lbl_1_bss_0);
}

/* Loads the ending staff-roll scene and starts its process from overlay entry. */
void fn_1_5A7C(void)
{
    OSReport(lbl_1_data_9C1);
    lbl_1_bss_0 = omInitObjMan(11, 8192);
    HuWinInit(1);
    fn_1_E20();
    fn_1_F68();
    fn_1_96A4();

    lbl_1_bss_4 = omAddObjEx(
        lbl_1_bss_0, 4096, 2, 2, OM_GRP_NONE, fn_1_389C);
    /* The character loaders store five motion IDs although these objects request four slots. */
    lbl_1_bss_8 = omAddObjEx(
        lbl_1_bss_0, 4096, 2, 4, OM_GRP_NONE, fn_1_3A98);
    lbl_1_bss_C = omAddObjEx(
        lbl_1_bss_0, 4096, 2, 4, OM_GRP_NONE, fn_1_3C38);
    lbl_1_bss_18 = omAddObjEx(
        lbl_1_bss_0, 4096, 0, 0, OM_GRP_NONE, fn_1_1684);

    lbl_1_bss_20 = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_staff, 43), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_82A = HuSprGrpCreate(1);
    lbl_1_bss_828 = HuSprCreate(lbl_1_bss_20, 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_82A, 0, lbl_1_bss_828);
    HuSprPosSet(
        lbl_1_bss_82A, 0, 288.0f, 400.0f);
    HuSprTPLvlSet(lbl_1_bss_82A, 0, 0.0f);
    HuSprAttrSet(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_30 = 0.0f;
    lbl_1_bss_2C = 0.0f;

    {
        s16 modelCount = 13;

        lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 4096,
            modelCount, modelCount, OM_GRP_NONE, fn_1_4AF4);
    }
    if (GwCommon.viewEnding) {
        lbl_1_bss_304[4] = HuWinExCreateFrame(16.0f,
            400.0f, 544, 42, -1, 0);
        HuWinDispOff(lbl_1_bss_304[4]);
        HuWinBGTPLvlSet(lbl_1_bss_304[4], 0.0f);
        lbl_1_bss_1C = omAddObjEx(
            lbl_1_bss_0, 4096, 0, 0, OM_GRP_NONE, fn_1_5148);
    }
    /* Mark the ending viewed even on visits that did not create the Start-to-exit prompt. */
    GwCommon.viewEnding = TRUE;
    HuPrcChildCreate(fn_1_528C, 12288, 12288, 0, lbl_1_bss_0);
}

/* Runs overlay constructors before starting the staff-roll scene. */
s32 _prolog(void)
{
    const VoidFunc *ctor = _ctors;

    while (*ctor) {
        (*ctor)();
        ctor++;
    }
    fn_1_5A7C();
    return 0;
}

/* Runs overlay destructors after the staff-roll scene ends. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;

    while (*dtor) {
        (*dtor)();
        dtor++;
    }
}

/* Initializes the staff entrance, then starts the music and incoming wipe after a delay. */
void fn_1_6634(void)
{
    OMOBJ *obj;
    STAFF_MOTION_WORK *secondWork;
    STAFF_MOTION_WORK *firstWork;

    HuPrcSleep(5);
    obj = lbl_1_bss_8;
    firstWork = &lbl_1_bss_5C[0];
    secondWork = &lbl_1_bss_5C[1];
    firstWork->angleOrFrame = 0.0f;
    firstWork->angleStep = 0.5f;
    firstWork->state = 0;
    firstWork->orbitRadiusTarget.x = firstWork->orbitRadius.x = frandmod(100) + 100;
    firstWork->orbitRadiusTarget.y = firstWork->orbitRadius.y = frandmod(200) + 100;
    firstWork->orbitRadiusTarget.z = firstWork->orbitRadius.z = 200.0f;

    secondWork->angleOrFrame = 270.0f;
    secondWork->angleStep = 0.5f;
    secondWork->state = 0;
    secondWork->orbitRadiusTarget.x = secondWork->orbitRadius.x = frandmod(100) + 100;
    secondWork->orbitRadiusTarget.y = secondWork->orbitRadius.y = frandmod(200) + 100;
    secondWork->orbitRadiusTarget.z = secondWork->orbitRadius.z = 200.0f;
    obj->work[1] = 0;
    obj->objFunc = fn_1_1758;
    HuPrcSleep(10);
    lbl_1_bss_824 = HuAudSStreamPlay(MSM_STREAM_W05);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
}

/* Resets both staff characters before the opening sequence begins. */
static inline void staff_initialize_work_pair_inline(void)
{
    STAFF_MOTION_WORK *firstWork;
    STAFF_MOTION_WORK *secondWork;
    OMOBJ *obj;

    obj = lbl_1_bss_8;
    firstWork = &lbl_1_bss_5C[0];
    secondWork = &lbl_1_bss_5C[1];
    firstWork->angleOrFrame = 0.0f;
    firstWork->angleStep = 0.5f;
    firstWork->state = 0;
    firstWork->orbitRadiusTarget.x = firstWork->orbitRadius.x = frandmod(100) + 100;
    firstWork->orbitRadiusTarget.y = firstWork->orbitRadius.y = frandmod(200) + 100;
    firstWork->orbitRadiusTarget.z = firstWork->orbitRadius.z = 200.0f;

    secondWork->angleOrFrame = 270.0f;
    secondWork->angleStep = 0.5f;
    secondWork->state = 0;
    secondWork->orbitRadiusTarget.x = secondWork->orbitRadius.x = frandmod(100) + 100;
    secondWork->orbitRadiusTarget.y = secondWork->orbitRadius.y = frandmod(200) + 100;
    secondWork->orbitRadiusTarget.z = secondWork->orbitRadius.z = 200.0f;
    obj->work[1] = 0;
    obj->objFunc = fn_1_1758;
}

/* Starts the staff music and incoming wipe after the entrance delay. */
static inline void staff_begin_sequence_inline(void)
{
    staff_initialize_work_pair_inline();
    HuPrcSleep(10);
    lbl_1_bss_824 = HuAudSStreamPlay(MSM_STREAM_W05);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
}

/* Starts both characters' next motion after the opening pose. */
static inline void staff_set_second_motion_inline(void)
{
    OMOBJ *obj = lbl_1_bss_8;
    OMOBJ *first = lbl_1_bss_8;
    OMOBJ *second = lbl_1_bss_C;
    STAFF_MOTION_WORK *firstWork = &lbl_1_bss_5C[0];
    STAFF_MOTION_WORK *secondWork = &lbl_1_bss_5C[1];

    Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[2],
        0.0f, 30.0f, 0);
    Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[2],
        0.0f, 30.0f, 0);
    obj->work[0] = 0;
    obj->objFunc = fn_1_3140;
}

/* Coordinates the staff entrance, credit timing, and transition to the next pose. */
void fn_1_6840(void)
{
    HuPrcSleep(5);
    staff_begin_sequence_inline();
    lbl_1_bss_24 = 1;
    HuPrcSleep(180);
    lbl_1_bss_10->objFunc = fn_1_4668;

    while (TRUE) {
        if (lbl_1_bss_18->objFunc == NULL) {
            lbl_1_bss_24 = 0;
            HuPrcSleep(60);
            HuSprAttrReset(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
            HuSprTPLvlSet(lbl_1_bss_82A, 0, 0.0f);
            lbl_1_bss_30 = 1.0f;
            lbl_1_bss_2C = 0.0f;
            {
                OMOBJ *obj = lbl_1_bss_8;

                obj->work[1] = 1;
            }
            lbl_1_bss_C->objFunc = fn_1_884;
            HuPrcSleep(180);

            staff_set_second_motion_inline();
            HuPrcSleep(1100);
            break;
        } else {
            HuPrcVSleep();
        }
    }
    HuPrcSleep(100);
}

/* Shows or hides one character's staff particle model. */
void fn_1_6C14(s16 modelNo, s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_44[modelNo], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_44[modelNo], HU3D_ATTR_DISPOFF);
    }
}

/* Starts every particle for a character and applies its color and position. */
void fn_1_6C90(s16 modelNo, HuVecF *pos, GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    s16 i;
    HU3D_PARTICLE_DATA *entry;

    model = &Hu3DData[lbl_1_bss_44[modelNo]];
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
        Hu3DModelPosSetV(lbl_1_bss_44[modelNo], pos);
    }
    Hu3DModelAttrReset(lbl_1_bss_44[modelNo], HU3D_ATTR_DISPOFF);
}

/* Marks every character particle for its fade-out update. */
void fn_1_6D94(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    s16 i;
    HU3D_PARTICLE_DATA *entry;

    model = &Hu3DData[lbl_1_bss_44[modelNo]];
    particle = model->hookData;
    for (i = 0, entry = particle->data; i < particle->maxCnt; i++, entry++) {
        entry->time = 2;
    }
}

/* Eases character particle scale and alpha; the first render update clears any pending
* start or fade request before processing the particles. */
void fn_1_6E1C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
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
            data->color.a = fn_1_2E8(10, data->color.a, data->vel.x);
            data->scale = fn_1_2E8(10, data->scale, data->vel.y);
        } else if (data->time == 2) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            data->vel.x = 0.0f;
            data->vel.y = 0.0f;
            data->color.a = fn_1_2E8(10, data->color.a, data->vel.x);
            data->scale = fn_1_2E8(10, data->scale, data->vel.y);
        } else {
            data->color.a = 0;
            data->scale = 0.0f;
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Creates two hidden particle models for the staff characters. */
void fn_1_72A4(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_44[i] = Hu3DParticleCreate(lbl_1_bss_48[0], 10);
        Hu3DModelPosSet(lbl_1_bss_44[i], 0.0f,
            0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_44[i], 1.0f,
            1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_44[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_44[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_44[i], fn_1_6E1C);
        Hu3DParticleBlendModeSet(lbl_1_bss_44[i], 1);
    }
}

/* Removes the two character particle models during scene cleanup. */
void fn_1_7410(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_44[i]);
    }
}

/* Shows or hides one of the two sparkle particle models. */
void fn_1_7468(s16 modelNo, s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_40[modelNo], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_40[modelNo], HU3D_ATTR_DISPOFF);
    }
}

/* Starts sparkles for a character using the supplied position and RGB color. */
void fn_1_74E4(s16 effectNo, HuVecF *position, GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_40[effectNo]];
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
    Hu3DModelAttrReset(lbl_1_bss_40[effectNo], HU3D_ATTR_DISPOFF);
}

/* Stops new particles from spawning for one character's sparkle effect. */
void fn_1_761C(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_40[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

/* Spawns and advances colored sparkles around a staff character each frame.
* The first render update forces spawning on and replaces any supplied RGB tint with white. */
void fn_1_7670(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
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
            random = fn_1_0(1.0f, 0.0f, data->vel.x, data->vel.y);
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

/* Creates two hidden sparkle models and installs their particle callback. */
void fn_1_7C70(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_40[i] = Hu3DParticleCreate(lbl_1_bss_48[1], 128);
        Hu3DModelPosSet(lbl_1_bss_40[i], 0.0f,
            0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_40[i], 1.0f,
            1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_40[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_40[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_40[i], fn_1_7670);
        Hu3DParticleBlendModeSet(lbl_1_bss_40[i], 1);
    }
}

/* Removes both sparkle particle models during staff effect cleanup. */
void fn_1_7DDC(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_40[i]);
    }
}

/* Updates descending white particles and their larger translucent copies. */
void fn_1_7E34(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE_DATA *copy;
    s16 i;
    s16 initialValue = 0;
    f32 sourceAlpha;
    f32 age;
    f32 duration;
    f32 copyAlpha;
    f32 copyAge;
    f32 copyDuration;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }

    for (i = 0, data = particle->data;
         i < particle->maxCnt / 2;
         i++, data++) {
        if (data->time == 0) {
            copy = &particle->data[(particle->maxCnt / 2) + i];
            data->time = 1;
            data->vel.x = 0.0f;
            data->vel.y = frandmod(60) + 60;
            data->pos.x = frandmod(2000) - 1000;
            data->pos.y = frandmod(1000);
            data->pos.z = frandmod(2000) - 1000;
            data->color.r = 255;
            data->color.g = 255;
            data->color.b = 255;
            data->color.a = 0;
            copy->color.a = 0;
            data->scale = 7.5f;
        } else if (data->time == 1) {
            copy = &particle->data[(particle->maxCnt / 2) + i];
            data->pos.y -= 1.0f;
            copy->pos.x = data->pos.x;
            copy->pos.y = data->pos.y;
            copy->pos.z = data->pos.z;
            copy->color.r = 255;
            copy->color.g = 255;
            copy->color.b = 255;
            copy->color.a = 16;
            copy->scale = 150.0f;
            duration = data->vel.y;
            age = data->vel.x;
            if (age <= 0.0f) {
                sourceAlpha = 0.0f;
            } else if (age >= duration) {
                sourceAlpha = 136.0f;
            } else {
                StaffSineMove(&sourceAlpha, 0.0, 136.0, 180.0f / duration, age);
            }
            data->color.a = sourceAlpha;

            copyDuration = data->vel.y;
            copyAge = data->vel.x;
            if (copyAge <= 0.0f) {
                copyAlpha = 0.0f;
            } else if (copyAge >= copyDuration) {
                copyAlpha = 16.0f;
            } else {
                StaffSineMove(&copyAlpha, 0.0, 16.0, 180.0f / copyDuration, copyAge);
            }
            copy->color.a = copyAlpha;

            if ((data->vel.x += 1.0f) > data->vel.y) {
                data->time = 0;
                data->color.a = 0;
                copy->color.a = 0;
                data->scale = 0.0f;
                copy->scale = 0.0f;
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Creates the shared white-particle model and installs its update callback. */
void fn_1_8300(void)
{
    lbl_1_bss_3E = Hu3DParticleCreate(lbl_1_bss_48[0], 256);
    Hu3DModelPosSet(lbl_1_bss_3E, 0.0f, 0.0f,
        0.0f);
    Hu3DModelScaleSet(lbl_1_bss_3E, 1.0f, 1.0f,
        1.0f);
    Hu3DModelLayerSet(lbl_1_bss_3E, 2);
    Hu3DParticleHookSet(lbl_1_bss_3E, fn_1_7E34);
    Hu3DParticleBlendModeSet(lbl_1_bss_3E, 1);
}

/* Removes the shared white-particle model during scene cleanup. */
void fn_1_83E4(void)
{
    Hu3DModelKill(lbl_1_bss_3E);
}

/* Shows or hides one staff particle model. */
void fn_1_8410(s16 modelNo, s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_3A[modelNo], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_3A[modelNo], HU3D_ATTR_DISPOFF);
    }
}

/* Starts one drifting-particle effect around the supplied position with the supplied RGB tint. */
void fn_1_848C(s16 effectNo, HuVecF *position, GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_3A[effectNo]];
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
    Hu3DModelAttrReset(lbl_1_bss_3A[effectNo], HU3D_ATTR_DISPOFF);
}

/* Stops the selected drifting-particle effect from spawning more particles. */
void fn_1_85C4(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_3A[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

/* Spawns at most one drifting particle per update and moves all active particles only along Y.
* The first render update forces spawning on and replaces any supplied RGB tint with white. */
void fn_1_8618(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
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
            data->accel.y *= 2.0f;
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
            data->pos.y += data->accel.y;
            if (rand8() % 5 == 0) {
                data->zRot = MTXDegToRad(frandmod(360));
                data->color.a = frandmod(127) + 128;
            }
            random = fn_1_0(1.0f, 0.0f, data->vel.x, data->vel.y);
            data->scale = frandmod(30) * random;
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

/* Creates two hidden drifting-particle models for staff characters. */
void fn_1_8BF8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_3A[i] = Hu3DParticleCreate(lbl_1_bss_48[1], 128);
        Hu3DModelPosSet(lbl_1_bss_3A[i], 0.0f,
            0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_3A[i], 1.0f,
            1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_3A[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_3A[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_3A[i], fn_1_8618);
        Hu3DParticleBlendModeSet(lbl_1_bss_3A[i], 1);
    }
}

/* Removes both drifting-particle models during scene cleanup. */
void fn_1_8D64(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_3A[i]);
    }
}

/* Shows or hides one of the two falling-particle models. */
void fn_1_8DBC(s16 modelNo, s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_36[modelNo], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_36[modelNo], HU3D_ATTR_DISPOFF);
    }
}

/* Starts a falling-particle effect and sets its RGB tint; its callback ignores the supplied
 * position. */
void fn_1_8E38(s16 effectNo, HuVecF *position, GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_36[effectNo]];
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
    Hu3DModelAttrReset(lbl_1_bss_36[effectNo], HU3D_ATTR_DISPOFF);
}

/* Stops new falling particles for the selected character. */
void fn_1_8F70(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_36[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

/* Spawns and advances falling particles for the staff scene.
* The first render update forces spawning on and replaces any supplied RGB tint with white. */
void fn_1_8FC4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
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
            data->accel.y = -frandmod(100);
            data->accel.z = frandmod(100) - 50;
            PSVECNormalize(&data->accel, &data->accel);
            data->accel.x *= 2.0f;
            data->accel.y *= 5.0f;
            data->accel.z *= 2.0f;
            data->pos.x = frandmod(2000) - 1000;
            data->pos.y = frandmod(100) + 1000;
            data->pos.z = -frandmod(1000);
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
            data->scale = frandmod(60) * 0.7f;
            if (data->pos.y < 0.0f) {
                data->time = 0;
                data->scale = 0.0f;
                data->color.a = 0;
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}
/* Creates two hidden falling-particle models and installs their callback. */
void fn_1_94E0(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_36[i] = Hu3DParticleCreate(lbl_1_bss_48[1], 128);
        Hu3DModelPosSet(lbl_1_bss_36[i], 0.0f,
            0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_36[i], 1.0f,
            1.0f, 1.0f);
        Hu3DModelAttrSet(lbl_1_bss_36[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_36[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_36[i], fn_1_8FC4);
        Hu3DParticleBlendModeSet(lbl_1_bss_36[i], 1);
    }
}

/* Removes both falling-particle models during scene cleanup. */
void fn_1_964C(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_36[i]);
    }
}

/* Loads both particle animations and creates each staff-roll particle system. */
void fn_1_96A4(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_48[i] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_9F8[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    fn_1_72A4();
    fn_1_7C70();
    fn_1_8300();
    fn_1_8BF8();
    fn_1_94E0();
}

/* Removes every staff-roll particle model during scene cleanup. */
void fn_1_9D14(void)
{
    s16 i;
    s16 j;
    s16 k;
    s16 l;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_44[i]);
    }
    for (j = 0; j < 2; j++) {
        Hu3DModelKill(lbl_1_bss_40[j]);
    }
    Hu3DModelKill(lbl_1_bss_3E);
    for (k = 0; k < 2; k++) {
        Hu3DModelKill(lbl_1_bss_3A[k]);
    }
    for (l = 0; l < 2; l++) {
        Hu3DModelKill(lbl_1_bss_36[l]);
    }
}

/* Starts one staff sparkle model with its color and the character's position. */
static inline void staff_spawn_effect(s16 modelNo, HuVecF *pos, GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_40[modelNo]];
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
    Hu3DModelAttrReset(lbl_1_bss_40[modelNo], HU3D_ATTR_DISPOFF);
}

/* Stops further particles from spawning for one staff sparkle model. */
static inline void staff_stop_effect(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_40[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

/* Starts, fades, or hides both particle models for a staff character. */
void fn_1_9E30(s16 modelNo, HuVecF *pos, s16 display)
{
    GXColor colors[2] = {
        { 255, 114, 46, 0 },
        { 109, 207, 246, 0 },
    };
    if (display == 1) {
        fn_1_6C90(modelNo, pos, &colors[modelNo]);

        staff_spawn_effect(modelNo, pos, &colors[modelNo]);
    } else if (display == 0) {
        fn_1_6D94(modelNo);

        staff_stop_effect(modelNo);
    } else if (display == 2) {
        Hu3DModelAttrSet(lbl_1_bss_44[modelNo], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_40[modelNo], HU3D_ATTR_DISPOFF);
    }
}

/* Stops new particles from one falling-particle model. */
static inline void staff_stop_falling_effect(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_36[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

/* Stops new particles from one drifting-particle model. */
static inline void staff_stop_drifting_effect(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_3A[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

/* Enables a falling-particle model with an optional RGB tint; its callback ignores the supplied
* position. */
static inline void staff_start_falling_effect(s16 modelNo, HuVecF *position, GXColor *color)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_36[modelNo]];
    HU3D_PARTICLE *particle = model->hookData;

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
    Hu3DModelAttrReset(lbl_1_bss_36[modelNo], HU3D_ATTR_DISPOFF);
}

/* Starts a drifting-particle model with optional color and position. */
static inline void staff_start_drifting_effect(s16 modelNo, HuVecF *position, GXColor *color)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_3A[modelNo]];
    HU3D_PARTICLE *particle = model->hookData;

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
    Hu3DModelAttrReset(lbl_1_bss_3A[modelNo], HU3D_ATTR_DISPOFF);
}

/* Initializes, stops, or hides one falling-particle effect; its callback ignores the supplied
 * position. */
void fn_1_A1F4(s16 modelNo, HuVecF *position, s16 state)
{
    STAFF_PARTICLE_COLOR_TABLE colorTable = {{{255, 114, 46, 0}, {109, 207, 246, 0}}};

    if (state == 1) {
        staff_start_falling_effect(modelNo, position, &colorTable.color[modelNo]);
    } else if (state == 0) {
        staff_stop_falling_effect(modelNo);
    } else if (state == 2) {
        Hu3DModelAttrSet(lbl_1_bss_36[modelNo], HU3D_ATTR_DISPOFF);
    }
}
/* Initializes, stops, or hides one staff character's drifting-particle model. */
void fn_1_A434(s16 modelNo, HuVecF *position, s16 state)
{
    STAFF_PARTICLE_COLOR_TABLE colorTable = {{{255, 114, 46, 0}, {109, 207, 246, 0}}};

    if (state == 1) {
        staff_start_drifting_effect(modelNo, position, &colorTable.color[modelNo]);
    } else if (state == 0) {
        staff_stop_drifting_effect(modelNo);
    } else if (state == 2) {
        Hu3DModelAttrSet(lbl_1_bss_3A[modelNo], HU3D_ATTR_DISPOFF);
    }
}

HU3D_LIGHTID lbl_1_bss_82C[2];
HUSPR_GROUPID lbl_1_bss_82A;
HUSPRID lbl_1_bss_828;
s32 lbl_1_bss_824;
f32 lbl_1_bss_310[325];
s16 lbl_1_bss_304[6];
s16 lbl_1_bss_2AC[44];
HuVecF lbl_1_bss_12C[32];
s16 lbl_1_bss_12A;
s16 lbl_1_bss_128;
f32 lbl_1_bss_124;
STAFF_MOTION_WORK lbl_1_bss_5C[2];
f32 lbl_1_bss_50[3];
ANIMDATA *lbl_1_bss_48[2];
HU3D_MODELID lbl_1_bss_44[2];
HU3D_MODELID lbl_1_bss_40[2];
HU3D_MODELID lbl_1_bss_3E;
HU3D_MODELID lbl_1_bss_3A[2];
HU3D_MODELID lbl_1_bss_36[2];
s16 lbl_1_bss_34;
f32 lbl_1_bss_30;
f32 lbl_1_bss_2C;
f32 lbl_1_bss_28;
s16 lbl_1_bss_24;
ANIMDATA *lbl_1_bss_20;
OMOBJ *lbl_1_bss_1C;
OMOBJ *lbl_1_bss_18;
OMOBJ *lbl_1_bss_14;
OMOBJ *lbl_1_bss_10;
OMOBJ *lbl_1_bss_C;
OMOBJ *lbl_1_bss_8;
OMOBJ *lbl_1_bss_4;
OMOBJMAN *lbl_1_bss_0;
