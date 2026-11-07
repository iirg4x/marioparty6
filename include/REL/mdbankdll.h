#ifndef MDBANKDLL_H
#define MDBANKDLL_H

void fn_1_0(HUWINID winId, u32 mess, s16 index);
void fn_1_9A8(s16 winNo);
void fn_1_A18(s16 winNo);
s16 fn_1_AC4(s16 winNo, s16 mode);
void fn_1_12F8(s16 winNo, s32 messNum, s16 speed);
void fn_1_1BD4(OMOBJ *obj);
void fn_1_1CD8(OMOBJ *obj);
void fn_1_1DDC(OMOBJ *obj);
void fn_1_226C(OMOBJ *obj);
void fn_1_2AA4(OMOBJ *obj);
void fn_1_2EFC(s16 index);
void fn_1_35E0(OMOBJ *obj);
void fn_1_420C(HUSPR_GROUPID groupId, s32 memberNo, s16 value);
void fn_1_6290(HUSPR_GROUPID groupId, s16 baseIndex, s16 value);
float fn_1_11064(float start, float end, float time, float duration);
void fn_1_1115C(HuVecF *out, f32 x, f32 y, f32 z);
float fn_1_1116C(float start, float end, float time, float duration);
float fn_1_111B0(float current, float target, float weight);
void fn_1_111F8(HuVecF *current, const HuVecF *target, float weight);
float fn_1_11384(float startValue, float peakValue, float elapsedFrames, float durationFrames);
float fn_1_11458(float startValue, float endValue, float time, float duration);
float fn_1_1161C(float start, float control, float end, float weight);
void fn_1_11678(HuVecF *out, const HuVecF *start, const HuVecF *control, const HuVecF *end,
                float weight);
void fn_1_11880(HUSPR_GROUPID groupId, s32 attr);
void fn_1_11900(HUSPR_GROUPID groupId, s32 attr);
void fn_1_11980(float posX, float posY, float posZ);
void fn_1_119C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_12230(HuVecF *pos, GXColor *color, float velocityY, float accelY);
void fn_1_12354(float velocityY, float velocityZ, HuVecF *pos, GXColor *color);
void fn_1_124F4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_12984(void);
void fn_1_12A94(float x, float y, float z);
void fn_1_12AE8(s16 time);
void fn_1_12B60(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_12D98(void);
void fn_1_12EA8(void);
void fn_1_1317C(void);
void fn_1_131C8(HuVecF *pos);
void fn_1_13598(HuVecF *position);
void fn_1_13764(s16 time);

#endif
