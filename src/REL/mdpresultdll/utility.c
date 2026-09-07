#include <string.h>

#include "datadir_enum.h"
#include "dolphin.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/saveload.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"
#include "messdir_enum.h"

#include "REL/mdpresultDll.h"

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

float fn_1_1F8BC(float current, float target, float weight);
void fn_1_1FB50(HuVecF *current, const HuVecF *target, float weight);
void fn_1_26164(s16 index, HuVecF *position);
void fn_1_26478(s16 index, HuVecF *position, const GXColor *color);
float fn_1_1F878(float start, float end, float time, float duration);
float fn_1_1FD7C(float start, float end, float time, float duration);
void fn_1_1F948(HuVecF *result, const HuVecF *start,
    const HuVecF *middle, const HuVecF *end, float time);
float fn_1_1FF48(float start, float end, float time, float duration);
float fn_1_1FE74(float start, float end, float time, float duration);
void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId,
    GXTexWrapMode wrapS, GXTexWrapMode wrapT, GXTexFilter filter);
void fn_1_26CF8(s16 index, HuVecF *position, float value);
float fn_1_1FC94(float start, float end, float time, float duration);
void fn_1_2001C(HU3D_MODELID modelId, const HuVecF *first,
    const HuVecF *second);
void fn_1_20108(HUSPR_GROUPID groupId, s32 attr);
void fn_1_20208(HUSPR_GROUPID groupId, s32 member, s16 value);
void fn_1_2035C(HUSPR_GROUPID groupId, s32 member, s16 value);
void fn_1_21714(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_217EC(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color, float accelY);
void fn_1_21904(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_25E6C(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_26070(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_2668C(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_26BE4(s16 index);
void fn_1_26EB0(HuVecF *position);
void fn_1_22F80(HU3D_MODEL *model, Mtx *matrix);
void fn_1_22E48(MDRESULT_TRAIL_WORK *work);
void fn_1_23AA8(void);
void fn_1_23DA0(s16 index, u8 *color, const HuVecF *position);
void fn_1_243DC(s16 index, const HuVecF *position, u8 *color,
    float velocityY, float velocityZ, float accelX, s16 mode);
void fn_1_20188(HUSPR_GROUPID groupId, s32 attr);
void fn_1_25DB0(s16 index, HuVecF *position, float alpha);
void fn_1_25D0C(float value);
void fn_1_25FF4(s16 index);
void fn_1_25B90(void);
void fn_1_26EAC(float value);
void fn_1_26F74(void);
void fn_1_23EF0(HuVecF *position);
void fn_1_2104C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_20554(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_21AD0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_22348(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_24554(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_24C58(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_252F8(void);

extern ANIMDATA *lbl_1_bss_131C;
extern HU3D_MODELID lbl_1_bss_1318;
extern HU3D_MODELID lbl_1_bss_131A;
extern HU3D_MODELID lbl_1_bss_14C2;
extern HU3D_MODELID lbl_1_bss_14C4;
extern HU3D_MODELID lbl_1_bss_14C6;
extern HU3D_MODELID lbl_1_bss_14B0[9];
extern HU3D_MODELID lbl_1_bss_1490[4][4];
extern MDRESULT_TRAIL_WORK lbl_1_bss_1320[8];
extern HU3D_MODELID lbl_1_bss_1480[8];
extern ANIMDATA *lbl_1_bss_14C8[7];
extern s32 lbl_1_data_788[7];



float fn_1_1F8EC(float start, float middle, float end, float time);
void fn_1_204B0(float value);
void fn_1_20BC8(void);
void fn_1_20CE0(void);
void fn_1_20D0C(s16 index, HuVecF *position, float alpha);
void fn_1_20DAC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_20E9C(void);
void fn_1_20F80(void);
void fn_1_20FAC(void);
void fn_1_21604(void);
void fn_1_216E8(void);
void fn_1_21A70(s16 index);
void fn_1_22080(void);
void fn_1_221EC(void);
void fn_1_22244(s16 index, HuVecF *position);
void fn_1_22A4C(void);
void fn_1_22C38(void);
void fn_1_22CBC(MDRESULT_TRAIL_WORK *work);
void fn_1_23C88(void);
void fn_1_23D38(s16 index, HuVecF *position, float value);
void fn_1_2429C(s16 index);
void fn_1_24308(s16 index, float value);
void fn_1_2436C(s16 index, HuVecF *position);
void fn_1_24AD0(void);
void fn_1_24BB4(void);
void fn_1_24BE0(HuVecF *pos);
void fn_1_24C28(void);
void fn_1_251D4(void);
void fn_1_252CC(void);

float fn_1_1F878(float start, float end, float time, float duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}

float fn_1_1F8BC(float current, float target, float weight)
{
    if (current == target) {
        return target;
    }
    return (target + (current * (weight - 1.0f))) / weight;
}

float fn_1_1F8EC(float start, float middle, float end, float time)
{
    float inverse = 1.0f - time;

    return (end * (time * time))
        + ((start * (inverse * inverse))
            + (2.0f * (middle * (inverse * time))));
}

void fn_1_1F948(HuVecF *result, const HuVecF *start,
    const HuVecF *middle, const HuVecF *end, float time)
{
    result->x = fn_1_1F8EC(start->x, middle->x, end->x, time);
    result->y = fn_1_1F8EC(start->y, middle->y, end->y, time);
    result->z = fn_1_1F8EC(start->z, middle->z, end->z, time);
}

void fn_1_1FB50(HuVecF *current, const HuVecF *target, float weight)
{
    current->x = fn_1_1F8BC(current->x, target->x, weight);
    current->y = fn_1_1F8BC(current->y, target->y, weight);
    current->z = fn_1_1F8BC(current->z, target->z, weight);
}

float fn_1_1FC94(float start, float end, float time, float duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return (float)(start + ((end - start) *
        sin((M_PI * ((90.0f / duration) * time)) /
            180.0)));
}

float fn_1_1FD7C(float start, float end, float time, float duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return (float)(start + ((end - start) *
        (1.0 - cos(
            (M_PI * ((90.0f / duration) * time)) /
                180.0))));
}

float fn_1_1FE74(float start, float end, float time, float duration)
{
    if (time <= 0.0f || time >= duration) {
        return start;
    }
    return (float)(start + ((end - start) *
        sin((M_PI * ((180.0f / duration) * time)) /
            180.0)));
}

float fn_1_1FF48(float start, float end, float time, float duration)
{
    if (time <= 0.0f || time >= duration) {
        return start;
    }
    return (float)(start + ((end - start) *
        sin((M_PI * ((360.0f / duration) * time)) /
            180.0)));
}

void fn_1_2001C(HU3D_MODELID modelId, const HuVecF *first,
    const HuVecF *second)
{
    HuVecF screen = { 0.0f, 0.0f, 0.0f };
    HuVecF world;

    if (first) {
        screen.x += first->x;
        screen.y += first->y;
        screen.z += first->z;
    }
    if (second) {
        screen.x += second->x;
        screen.y += second->y;
        screen.z += second->z;
    }
    Hu3D2Dto3D(&screen, 1, &world);
    Hu3DModelPosSet(modelId, world.x, world.y, world.z);
}

void fn_1_20108(HUSPR_GROUPID groupId, s32 attr)
{
    HUSPR_GROUP *group = &HuSprGrpData[groupId];
    s16 i;

    for (i = 0; i < group->sprNum; i++) {
        HuSprAttrSet(groupId, i, (u16)attr);
    }
}

void fn_1_20188(HUSPR_GROUPID groupId, s32 attr)
{
    HUSPR_GROUP *group = &HuSprGrpData[groupId];
    s16 i;

    for (i = 0; i < group->sprNum; i++) {
        HuSprAttrReset(groupId, i, (u16)attr);
    }
}

void fn_1_20208(HUSPR_GROUPID groupId, s32 member, s16 value)
{
    s16 digit;

    digit = value / 100;
    HuSprBankSet(groupId, member, digit);
    if (digit == 0) {
        HuSprBankSet(groupId, member, 10);
    }
    digit = (value - (digit * 100)) / 10;
    HuSprBankSet(groupId, member + 1, digit);
    if (digit == 0 && value / 100 == 0) {
        HuSprAttrSet(groupId, member + 1, HUSPR_ATTR_DISPOFF);
    }
    digit = value % 10;
    HuSprBankSet(groupId, member + 2, digit);
}

void fn_1_2035C(HUSPR_GROUPID groupId, s32 member, s16 value)
{
    s16 digit;

    digit = value / 100;
    HuSprBankSet(groupId, member, digit);
    if (digit == 0) {
        HuSprAttrSet(groupId, member, HUSPR_ATTR_DISPOFF);
    }
    digit = (value - (digit * 100)) / 10;
    HuSprBankSet(groupId, member + 1, digit);
    if (digit == 0 && value / 100 == 0) {
        HuSprAttrSet(groupId, member + 1, HUSPR_ATTR_DISPOFF);
    }
    digit = value % 10;
    HuSprBankSet(groupId, member + 2, digit);
}

void fn_1_204B0(float value)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14C6];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    Hu3DModelLayerSet(lbl_1_bss_14C6, 1);
    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        data->colorIdx = value;
    }
}

void fn_1_20554(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE_DATA *paired;
    s16 i;
    s32 alpha;

    if (particle->count == 0) {
        i = 0;
        data = particle->data;
        for (; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt / 2; i++, data++) {
        if (data->time == 0) {
            paired = &particle->data[(particle->maxCnt / 2) + i];
            paired->vel.x = 0.0f;
            paired->vel.y = frandmod(10) + 5;
            data->time = 1;
            data->scale = frandmod(5) + 5;
            data->pos.x = frandmod(2000) - 1000;
            data->pos.y = frandmod(1000);
            data->pos.z = -frandmod(2000) + 1000;
            data->color.r = MDRESULT_PARTICLE_COLOR_RED_GREEN;
            data->color.g = MDRESULT_PARTICLE_COLOR_RED_GREEN;
            data->color.b = MDRESULT_COLOR_MAX;
            data->color.a = 0;
            data->vel.x = 0.0f;
            data->vel.y = frandmod(120) + 120;
            data->vel.z = frandmod(80) + 80;
            data->accel.x = 1.0f;
            data->colorIdx = 0.0f;
        } else if (data->time == 1) {
            paired = &particle->data[(particle->maxCnt / 2) + i];
            if (i % 2 == 0) {
                data->pos.x += 0.01f * frandmod(10);
            } else {
                data->pos.x -= 0.01f * frandmod(10);
            }
            data->pos.y += 0.25f
                + (0.01f * frandmod(10));
            data->pos.y += data->colorIdx;
            if (data->pos.y < -400.0f) {
                data->pos.y = 1000.0f;
            }
            data->accel.x = fn_1_1FE74(0.0f, 1.0f,
                data->vel.x, data->vel.y);
            data->scale = 3.0f;
            paired->scale = fn_1_1F8BC(paired->scale,
                20.0f + data->scale, 10.0f);
            paired->pos.x = data->pos.x;
            paired->pos.y = data->pos.y;
            paired->pos.z = data->pos.z;
            paired->color.r = MDRESULT_PARTICLE_COLOR_RED_GREEN;
            paired->color.g = MDRESULT_PARTICLE_COLOR_RED_GREEN;
            paired->color.b = MDRESULT_COLOR_MAX;
            paired->color.a = MDRESULT_PARTICLE_PAIRED_ALPHA;
            data->color.r = MDRESULT_PARTICLE_COLOR_RED_GREEN;
            data->color.g = MDRESULT_PARTICLE_COLOR_RED_GREEN;
            data->color.b = MDRESULT_COLOR_MAX;
            data->color.a = MDRESULT_COLOR_MAX;
            alpha = (data->vel.z * data->accel.x);
            data->color.a = alpha;
            alpha = (0.8f * (data->color.a * data->accel.x));
            paired->color.a = alpha;
            data->vel.x += 1.0f;
            if (data->vel.x > data->vel.y) {
                data->time = 0;
            }
        }
    }
    DCFlushRangeNoSync(particle->data,
        particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

void fn_1_20BC8(void)
{
    lbl_1_bss_14C6 = Hu3DParticleCreate(lbl_1_bss_14C8[0], 600);
    Hu3DModelPosSet(lbl_1_bss_14C6, 0.0f,
        0.0f, 0.0f);
    Hu3DModelRotSet(lbl_1_bss_14C6, 0.0f,
        0.0f, 0.0f);
    Hu3DModelScaleSet(lbl_1_bss_14C6, 1.0f,
        1.0f, 1.0f);
    Hu3DModelLayerSet(lbl_1_bss_14C6, 2);
    Hu3DParticleHookSet(lbl_1_bss_14C6, fn_1_20554);
    Hu3DParticleBlendModeSet(lbl_1_bss_14C6, 1);
}

void fn_1_20CE0(void)
{
    Hu3DModelKill(lbl_1_bss_14C6);
}

void fn_1_20D0C(s16 index, HuVecF *position, float alpha)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14C4];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = &particle->data[index];
    float opacity;

    data->vel.x = position->x;
    data->vel.y = position->y;
    data->vel.z = position->z;
    opacity = 64.0f * alpha;
    data->color.a = opacity;
}

void fn_1_20DAC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    s16 state = 0;
    float preset[12] = {
        -270.0f, 140.0f, 0.0f, -90.0f, 140.0f, 0.0f,
        90.0f, 140.0f, 0.0f, 270.0f, 140.0f, 0.0f
    };
    HU3D_PARTICLE_DATA *data;
    s16 i;

    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        data->scale = 400.0f;
        data->color.r = 255;
        data->color.g = 255;
        data->color.b = 255;
        data->pos.x = data->vel.x;
        data->pos.y = data->vel.y;
        data->pos.z = data->vel.z;
    }
}

void fn_1_20E9C(void)
{
    lbl_1_bss_14C4 = Hu3DParticleCreate(lbl_1_bss_14C8[0], 4);
    Hu3DModelPosSet(lbl_1_bss_14C4, 0.0f,
        0.0f, 0.0f);
    Hu3DModelScaleSet(lbl_1_bss_14C4, 1.0f,
        1.0f, 1.0f);
    Hu3DModelLayerSet(lbl_1_bss_14C4, 2);
    Hu3DParticleHookSet(lbl_1_bss_14C4, fn_1_20DAC);
    Hu3DParticleBlendModeSet(lbl_1_bss_14C4, 1);
}

void fn_1_20F80(void)
{
    Hu3DModelKill(lbl_1_bss_14C4);
}

void fn_1_20FAC(void)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14C2];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    Hu3DModelAttrReset(lbl_1_bss_14C2, HU3D_ATTR_DISPOFF);
    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        data->time = 0;
    }
}

void fn_1_2104C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    GXColor colors[7] = {
        {255, 90, 90, 0},
        {255, 90, 255, 0},
        {100, 90, 255, 0},
        {90, 255, 255, 0},
        {90, 255, 90, 0},
        {255, 255, 90, 0},
        {255, 180, 90, 0}
    };
    HU3D_PARTICLE_DATA *data;
    float alpha;
    s16 colorNo;
    s16 i;

    if (particle->count == 0) {
        i = 0;
        data = particle->data;
        for (; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }

    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        if (data->time == 0) {
            data->time = 1;
            if (i % 2 == 0) {
                data->speedDecay = frandmod(50);
            } else {
                data->speedDecay = -frandmod(50);
            }
            data->colorIdx = frandmod(100) - 50;
            data->scaleBase = 0.0f;
            PSVECNormalize((HuVecF *)&data->speedDecay,
                (HuVecF *)&data->speedDecay);
            data->scale = frandmod(50) + 50;
            colorNo = rand8() % 7;
            data->color.r = colors[colorNo].r;
            data->color.g = colors[colorNo].g;
            data->color.b = colors[colorNo].b;
            data->color.a = 255;
            data->vel.x = 0.0f;
            data->vel.y = frandmod(90) + 60;
            data->vel.z = 0.005f * (frandmod(100) - 50);
            data->accel.y = frandmod(10) + 10;
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
        } else if (data->time == 1) {
            alpha = fn_1_1FC94(1.0f, 0.0f, data->vel.x, data->vel.y);
            data->zRot += data->vel.z;
            data->color.a = 255.0f * alpha;
            data->pos.x += data->speedDecay * data->accel.y;
            data->pos.y += data->colorIdx * data->accel.y;
            data->pos.z += data->scaleBase * data->accel.y;
            if (++data->vel.x > data->vel.y) {
                data->time = 0;
                data->color.a = 0;
                data->scale = 0.0f;
            }
        }
    }
}

void fn_1_21604(void)
{
    lbl_1_bss_14C2 = Hu3DParticleCreate(lbl_1_bss_14C8[5], 128);
    Hu3DModelPosSet(lbl_1_bss_14C2, 0.0f,
        0.0f, 0.0f);
    Hu3DModelScaleSet(lbl_1_bss_14C2, 1.0f,
        1.0f, 1.0f);
    Hu3DModelLayerSet(lbl_1_bss_14C2, 1);
    Hu3DModelAttrSet(lbl_1_bss_14C2, HU3D_ATTR_DISPOFF);
    Hu3DParticleHookSet(lbl_1_bss_14C2, fn_1_2104C);
}

void fn_1_216E8(void)
{
    Hu3DModelKill(lbl_1_bss_14C2);
}

void fn_1_21714(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14B0[index]];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = particle->data;

    if (parManId > 0) {
        data->parManId = parManId;
    }
    if (velocity) {
        data->vel.x = velocity->x;
        data->vel.y = velocity->y;
        data->vel.z = velocity->z;
    }
    if (accelX > 0.0f) {
        data->accel.x = accelX;
    }
    if (color) {
        data->color.r = color[0];
        data->color.g = color[1];
        data->color.b = color[2];
        data->color.a = 0;
    }
    data->accel.y = 0.0f;
}

void fn_1_217EC(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color, float accelY)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14B0[index]];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = particle->data;

    fn_1_21714(index, parManId, velocity, accelX, color);
    data->accel.y = accelY;
}

void fn_1_21904(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14B0[index]];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        data->time = 0;
        data->parManId = 0;
        data->scale = 0.0f;
    }
    data = particle->data;
    data->time = 1;
    fn_1_21714(index, parManId, velocity, accelX, color);
    model->attr &= ~HU3D_ATTR_DISPOFF;
}

void fn_1_21A70(s16 index)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14B0[index]];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = particle->data;

    data->time = 0;
}

void fn_1_21AD0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *first, *data;
    s16 spawnCount, i;
    u16 random, color;

    spawnCount = 0;
    first = particle->data;
    i = 1;
    data = &particle->data[1];
    for (; i < particle->maxCnt; i++, data++) {
        if (first->time == 1) {
            if (data->time == 0) {
                if (spawnCount < first->parManId) {
                    spawnCount++;
                    data->time = 1;
                    data->parManId = (rand8() % 30) + 30;
                    data->vel.x = first->vel.x +
                        frandmod((u32)first->accel.x) -
                        (first->accel.x / 2.0f);
                    data->vel.y = first->vel.y +
                        frandmod((u32)first->accel.x) -
                        (first->accel.x / 2.0f);
                    data->vel.z = first->vel.z +
                        frandmod((u32)first->accel.x) -
                        (first->accel.x / 2.0f);
                    data->accel.x = 0.0f;
                    data->accel.z = 0.1f * (frandmod(10) - 5);
                    data->speedDecay =
                        0.1f * (frandmod(10) - 5);
                    data->colorIdx =
                        -1.0f * (frandmod(5) + 1);
                    data->scaleBase =
                        0.1f * (frandmod(10) - 5);
                    random = rand8() % 128;
                    color = first->color.r + random;
                    if (color > 255) {
                        color = 255;
                    }
                    data->color.r = color;
                    color = first->color.g + random;
                    if (color > 255) {
                        color = 255;
                    }
                    data->color.g = color;
                    color = first->color.b + random;
                    if (color > 255) {
                        color = 255;
                    }
                    data->color.b = color;
                    data->color.a = 0;
                    data->zRot += data->accel.z;
                    data->scale = 0.0f;
                    data->pos.x = data->vel.x;
                    data->pos.y = data->vel.y;
                    data->pos.z = data->vel.z;
                }
            }
        }
        if (data->time >= 1) {
            data->pos.x += data->speedDecay;
            data->pos.y += data->colorIdx + first->accel.y;
            data->pos.z += data->scaleBase;
            data->scale = frandmod(13);
            data->color.a = fn_1_1F878(255.0f, 0.0f,
                data->time, data->parManId);
            if (++data->time > data->parManId) {
                data->time = 0;
                data->scale = 0.0f;
            }
        }
    }

    if (first->time == 0) {
        i = 1;
        data = &particle->data[1];
        for (; i < particle->maxCnt; i++, data++) {
            if (data->time != 0) {
                break;
            }
        }
        if (i == particle->maxCnt) {
            model->attr |= HU3D_ATTR_DISPOFF;
        }
    }
    DCFlushRangeNoSync(particle->data,
        particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

void fn_1_22080(void)
{
    s16 i;

    for (i = 0; i < 9; i++) {
        lbl_1_bss_14B0[i] = Hu3DParticleCreate(lbl_1_bss_14C8[0], 64);
        Hu3DModelPosSet(lbl_1_bss_14B0[i], 0.0f,
            0.0f, 0.0f);
        Hu3DModelScaleSet(lbl_1_bss_14B0[i], 1.0f,
            1.0f, 1.0f);
        Hu3DModelLayerSet(lbl_1_bss_14B0[i], 2);
        Hu3DModelAttrSet(lbl_1_bss_14B0[i], HU3D_ATTR_DISPOFF);
        Hu3DParticleHookSet(lbl_1_bss_14B0[i], fn_1_21AD0);
        Hu3DParticleBlendModeSet(lbl_1_bss_14B0[i], 1);
    }
}

void fn_1_221EC(void)
{
    s16 i;

    for (i = 0; i < 9; i++) {
        Hu3DModelKill(lbl_1_bss_14B0[i]);
    }
}

void fn_1_22244(s16 index, HuVecF *position)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    s16 i;
    s16 j;

    for (i = 0; i < 4; i++) {
        model = &Hu3DData[lbl_1_bss_1490[index][i]];
        particle = model->hookData;
        j = 0;
        data = particle->data;
        for (; j < particle->maxCnt; j++, data++) {
            data->time = 0;
            data->parManId = 0;
            data->scale = 0.0f;
        }
        model->attr &= ~1;
        Hu3DModelPosSetV(lbl_1_bss_1490[index][i], position);
    }
}

void fn_1_22348(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    u16 color;
    u16 i;

    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        if (data->time == 0) {
            data->time = 1;
            data->parManId = frandmod(30) + 20;
            color = rand8() + 128;
            color &= 255;
            data->color.r = color;
            color = rand8() + 128;
            color &= 255;
            data->color.g = color;
            color = rand8() % 204;
            color &= 255;
            data->color.b = color;
            data->color.a = 0;
            data->scale = (float)frandmod(20) + 25.0f;
            data->vel.x = frandmod(100) - 50;
            data->vel.y = frandmod(100) - 50;
            data->vel.z = frandmod(100) - 50;
            PSVECNormalize(&data->vel, &data->vel);
            data->accel.x = 0.0f;
            data->accel.y = frandmod(50) + 150;
            data->zRot = 0.1f * (frandmod(10) - 5);
            data->speedDecay =
                0.1f * (frandmod(10) - 5);
            data->colorIdx =
                -1.0f * (frandmod(5) + 1);
            data->scaleBase =
                0.1f * (frandmod(10) - 5);
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
        } else if (data->time < 100) {
            data->accel.x = fn_1_1FC94(0.0f, data->accel.y,
                data->time, data->parManId);
            data->pos.x = data->speedDecay +
                (data->vel.x * data->accel.x);
            data->pos.y = data->colorIdx +
                (data->vel.y * data->accel.x);
            data->pos.z = data->scaleBase +
                (data->vel.z * data->accel.x);
            data->speedDecay += data->speedDecay / 20.0f;
            data->colorIdx += data->colorIdx / 20.0f;
            data->scaleBase += data->scaleBase / 20.0f;
            data->color.a = fn_1_1FC94(255.0f, 0.0f,
                data->time, data->parManId);
            if (++data->time > data->parManId) {
                data->time = 100;
            }
        }
    }

    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        if (data->time != 100) {
            break;
        }
    }
    if (i == particle->maxCnt) {
        model->attr |= HU3D_ATTR_DISPOFF;
    }
    DCFlushRangeNoSync(particle->data,
        particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

void fn_1_22A4C(void)
{
    s16 i;
    s16 j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            lbl_1_bss_1490[i][j] = Hu3DParticleCreate(lbl_1_bss_14C8[j], 16);
            Hu3DModelPosSet(lbl_1_bss_1490[i][j], 0.0f,
                0.0f, 0.0f);
            Hu3DModelScaleSet(lbl_1_bss_1490[i][j], 1.0f,
                1.0f, 1.0f);
            Hu3DModelLayerSet(lbl_1_bss_1490[i][j], 2);
            Hu3DModelAttrSet(lbl_1_bss_1490[i][j], HU3D_ATTR_DISPOFF);
            Hu3DParticleHookSet(lbl_1_bss_1490[i][j], fn_1_22348);
            Hu3DParticleBlendModeSet(lbl_1_bss_1490[i][j], 1);
        }
    }
}

void fn_1_22C38(void)
{
    s16 i;
    s16 j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            Hu3DModelKill(lbl_1_bss_1490[i][j]);
        }
    }
}

void fn_1_22CBC(MDRESULT_TRAIL_WORK *work)
{
    s16 i;

    for (i = work->pointCount - 1; i >= 1; i--) {
        work->points[i - 1].y -= 0.001f;
        work->points[i].x = work->base.x + work->points[i - 1].x;
        work->points[i].y = work->base.y + work->points[i - 1].y;
        work->points[i].z = work->base.z + work->points[i - 1].z;
    }
    if (work->unk_28 == 0) {
        if (work->state == 1) {
            work->color.a += 5;
            if (work->color.a >= 255) {
                work->color.a = 255;
            }
        } else {
            work->color.a -= 5;
            if (work->color.a == 0) {
                work->color.a = 0;
                Hu3DModelAttrSet(lbl_1_bss_1480[work->modelIndex],
                    HU3D_ATTR_DISPOFF);
            }
        }
    }
}

void fn_1_22E48(MDRESULT_TRAIL_WORK *work)
{
    work->points->x += 25.0f * work->velocity.x;
    work->points->y += 25.0f * work->velocity.y;
    work->points->z += 25.0f * work->velocity.z;
    if (work->points->y < -2000.0f) {
        Hu3DModelAttrSet(lbl_1_bss_1480[work->modelIndex],
            HU3D_ATTR_DISPOFF);
    }
    work->velocity.x +=
        0.001f * (frandmod(100) - 50);
    work->velocity.y -= 0.025f;
}

void fn_1_22F80(HU3D_MODEL *model, Mtx *matrix)
{
    MDRESULT_TRAIL_WORK *work = model->hookData;
    MDRESULT_VECTOR_PAIR vertices[100];
    u8 alpha[100];
    HuVecF direction;
    float fade;
    s16 i;
    s16 j;

    GXLoadPosMtxImm(*matrix, GX_PNMTX0);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(
        GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
        GX_FALSE, GX_PTIDENTITY);
    GXSetNumChans(1);
    GXSetChanCtrl(
        GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0,
        GX_DF_CLAMP, GX_AF_NONE);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(
        GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevColorOp(
        GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(
        GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
    GXSetTevAlphaOp(
        GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);

    if (work->unk_28 == 0) {
        HuSprTexLoad(
            lbl_1_bss_131C, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
        GXSetBlendMode(
            GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    } else {
        HuSprTexLoad(
            lbl_1_bss_14C8[6], 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
        GXSetBlendMode(
            GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    }

    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetZMode(GX_TRUE, GX_ALWAYS, GX_FALSE);
    GXSetCullMode(GX_CULL_NONE);

    for (i = 0; i < work->pointCount; i++) {
        if (i < work->pointCount - 1) {
            direction.x = work->points[i + 1].x - work->points[i].x -
                0.001f;
            direction.y = work->points[i + 1].y - work->points[i].y;
            direction.z = 0.0f;
            PSVECNormalize(&direction, &direction);
        }

        vertices[i].values[0].x =
            work->points[i].x + (-direction.y * work->delay);
        vertices[i].values[0].y =
            work->points[i].y + (direction.x * work->delay);
        vertices[i].values[0].z = work->points[i].z;
        vertices[i].values[1].x =
            work->points[i].x + (direction.y * work->delay);
        vertices[i].values[1].y =
            work->points[i].y + (-direction.x * work->delay);
        vertices[i].values[1].z = work->points[i].z;

        if (work->unk_28 == 0) {
            fade = 255.0f -
                (255.0f / (work->pointCount - 2)) * i;
            if (fade < 0.0f) {
                fade = 0.0f;
            }
            if (fade > 255.0f) {
                fade = 255.0f;
            }
            alpha[i] = (u8)fade;
        } else {
            alpha[i] = 255;
        }
    }

    GXBegin(
        GX_TRIANGLESTRIP, GX_VTXFMT0,
        (u16)((work->pointCount - 2) * 2));
    for (i = 0; i < work->pointCount - 2; i++) {
        GXPosition3f32(
            vertices[i].values[0].x, vertices[i].values[0].y,
            vertices[i].values[0].z);
        GXColor4u8(
            work->color.r, work->color.g, work->color.b, alpha[i]);
        GXTexCoord2f32(
            (1.0f / work->pointCount) * i, 1.0f);

        GXPosition3f32(
            vertices[i].values[1].x, vertices[i].values[1].y,
            vertices[i].values[1].z);
        GXColor4u8(
            work->color.r, work->color.g, work->color.b, alpha[i]);
        GXTexCoord2f32(
            (1.0f / work->pointCount) * i, 0.0f);
    }

    for (j = work->pointCount - 1; j >= 1; j--) {
        work->points[j - 1].y -= 0.001f;
        work->points[j].x =
            work->base.x + work->points[j - 1].x;
        work->points[j].y =
            work->base.y + work->points[j - 1].y;
        work->points[j].z =
            work->base.z + work->points[j - 1].z;
    }

    if (work->unk_28 == 0) {
        if (work->state == 1) {
            work->color.a += 5;
            if (work->color.a >= 255) {
                work->color.a = 255;
            }
        } else {
            work->color.a -= 5;
            if (work->color.a == 0) {
                work->color.a = 0;
                Hu3DModelAttrSet(
                    lbl_1_bss_1480[work->modelIndex], HU3D_ATTR_DISPOFF);
            }
        }
    }

    if (work->unk_28 == 1) {
        work->points[0].x += 25.0f * work->velocity.x;
        work->points[0].y += 25.0f * work->velocity.y;
        work->points[0].z += 25.0f * work->velocity.z;
        if (work->points[0].y < -2000.0f) {
            Hu3DModelAttrSet(
                lbl_1_bss_1480[work->modelIndex], HU3D_ATTR_DISPOFF);
        }
        work->velocity.x += 0.001f * (frandmod(100) - 50);
        work->velocity.y -= 0.025f;
    }
}

void fn_1_23AA8(void)
{
    s16 i;

    lbl_1_bss_131C = HuSprAnimRead(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 54), HU_MEMNUM_OVL, HEAP_MODEL));
    for (i = 0; i < 8; i++) {
        MDRESULT_TRAIL_WORK *work = &lbl_1_bss_1320[i];
        HU3D_MODEL *model;
        s16 j;

        lbl_1_bss_1480[i] = Hu3DHookFuncCreate(fn_1_22F80);
        Hu3DModelLayerSet(lbl_1_bss_1480[i], 1);
        Hu3DModelAttrSet(lbl_1_bss_1480[i], HU3D_ATTR_DISPOFF);
        work->modelIndex = i;
        work->state = 0;
        work->delay = 30;
        work->pointCount = 50;
        work->points = HuMemDirectMallocNum(HEAP_MODEL,
            work->pointCount * sizeof(HuVecF), HU_MEMNUM_OVL);
        work->base.x = work->base.y = work->base.z = 0.0f;
        work->color.r = work->color.g = work->color.b = work->color.a = 0;
        for (j = 0; j < work->pointCount; j++) {
            work->points[j].x = work->points[j].y = work->points[j].z =
                0.0f;
        }
        model = &Hu3DData[lbl_1_bss_1480[i]];
        model->hookData = work;
    }
}

void fn_1_23C88(void)
{
    HU3D_MODEL *model;
    s16 i;

    for (i = 0; i < 8; i++) {
        HuMemDirectFree(lbl_1_bss_1320[i].points);
        model = &Hu3DData[lbl_1_bss_1480[i]];
        model->hookData = NULL;
        Hu3DModelKill(lbl_1_bss_1480[i]);
    }
}

void fn_1_23D38(s16 index, HuVecF *position, float value)
{
    MDRESULT_TRAIL_WORK *work = &lbl_1_bss_1320[index];

    work->points[0].x = position->x;
    work->points[0].y = position->y;
    work->points[0].z = position->z;
    work->base.x = work->base.y = work->base.z = 0.0f;
    work->base.y = value;
}

void fn_1_23DA0(s16 index, u8 *color, const HuVecF *position)
{
    MDRESULT_TRAIL_WORK *work = &lbl_1_bss_1320[index];
    s16 i;

    work->state = 1;
    work->color.r = color[0];
    work->color.g = color[1];
    work->color.b = color[2];
    work->color.a = 0;
    work->unk_28 = 0;
    for (i = 0; i < work->pointCount; i++) {
        work->points[i].x = position->x;
        work->points[i].y = position->y;
        work->points[i].z = position->z;
        work->points[i].y -= 0.001f * i;
    }
    Hu3DModelAttrReset(lbl_1_bss_1480[index], HU3D_ATTR_DISPOFF);
}

void fn_1_23EF0(HuVecF *position)
{
    MDRESULT_COLOR_TABLE_8 colors = {{
        { 255, 0, 0, 0 },
        { 255, 0, 255, 0 },
        { 50, 0, 255, 0 },
        { 0, 255, 255, 0 },
        { 0, 255, 0, 0 },
        { 255, 255, 0, 0 },
        { 255, 150, 0, 0 },
        { 255, 255, 255, 0 },
    }};
    MDRESULT_TRAIL_WORK *work;
    s16 i;
    s16 j;

    for (i = 0; i < 8; i++) {
        work = &lbl_1_bss_1320[i];
        work->state = 1;
        work->color.r = colors.values[i].r;
        work->color.g = colors.values[i].g;
        work->color.b = colors.values[i].b;
        work->color.a = 0;
        work->unk_28 = 1;
        work->base.x = work->base.y = work->base.z = 0.0f;
        work->base.y -= 5.0f;
        work->delay = 10;
        if (i % 2 == 0) {
            work->velocity.x = frandmod(50);
        } else {
            work->velocity.x = -frandmod(50);
        }
        work->velocity.y = frandmod(100);
        work->velocity.z = 0.0f;
        PSVECNormalize(&work->velocity, &work->velocity);
        for (j = 0; j < work->pointCount; j++) {
            work->points[j].x = position->x;
            work->points[j].y = position->y;
            work->points[j].z = position->z;
            work->points[j].y -= 0.001f * j;
        }
        Hu3DModelAttrReset(lbl_1_bss_1480[i], 1);
    }
}

void fn_1_2429C(s16 index)
{
    MDRESULT_TRAIL_WORK *work = &lbl_1_bss_1320[index];

    work->state = 0;
    Hu3DModelAttrSet(lbl_1_bss_1480[index], HU3D_ATTR_DISPOFF);
}

void fn_1_24308(s16 index, float value)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_131A];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = &particle->data[index];

    data->time = 2;
    data->vel.z = value;
}

void fn_1_2436C(s16 index, HuVecF *position)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_131A];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = &particle->data[index];

    data->pos.x = position->x;
    data->pos.y = position->y;
    data->pos.z = position->z;
}

void fn_1_243DC(s16 index, const HuVecF *position, u8 *color,
    float velocityY, float velocityZ, float accelX, s16 mode)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_131A];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = &particle->data[index];

    data->time = 1;
    if (mode == 1) {
        data->time = 3;
    }
    data->pos.x = position->x;
    data->pos.y = position->y;
    data->pos.z = position->z;
    data->scale = 0.0f;
    data->color.r = color[0];
    data->color.g = color[1];
    data->color.b = color[2];
    data->color.a = color[3];
    data->vel.x = 0.0f;
    data->vel.y = velocityY;
    data->vel.z = velocityZ;
    data->speedDecay = color[0];
    data->colorIdx = color[1];
    data->scaleBase = color[2];
    data->accel.x = accelX;
    data->accel.y = color[3];
}

void fn_1_24554(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    u16 color;
    s16 i;
    u16 random;

    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        if (data->time == 1) {
            if (data->vel.y > 0.0f) {
                data->scale = fn_1_1FC94(0.0f,
                    data->accel.x, data->vel.x, data->vel.y);
                data->color.a = fn_1_1FC94(0.0f,
                    data->accel.y, data->vel.x, data->vel.y);
                if ((data->vel.x += 1.0f) > data->vel.y) {
                    data->time = 2;
                    data->vel.x = 0.0f;
                }
            }
        } else if (data->time == 2) {
            if (data->vel.z > 0.0f) {
                data->color.a = fn_1_1FC94(data->accel.y,
                    0.0f, data->vel.x, data->vel.y);
                if ((data->vel.x += 1.0f) > data->vel.z) {
                    data->time = 0;
                    data->scale = 0.0f;
                }
            }
        } else if (data->time == 3) {
            data->scale = (float)(data->accel.x + (rand8() % 20));
            random = rand8() % 128;
            color = data->speedDecay + random;
            if (color > 255) {
                color = 255;
            }
            data->color.r = color;
            color = data->colorIdx + random;
            if (color > 255) {
                color = 255;
            }
            data->color.g = color;
            color = data->scaleBase + random;
            if (color > 255) {
                color = 255;
            }
            data->color.b = color;
            data->color.a = data->accel.y;
        }
    }
    DCFlushRangeNoSync(particle->data,
        particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

void fn_1_24AD0(void)
{
    lbl_1_bss_131A = Hu3DParticleCreate(lbl_1_bss_14C8[0], 100);
    Hu3DModelPosSet(lbl_1_bss_131A, 0.0f,
        0.0f, 0.0f);
    Hu3DModelScaleSet(lbl_1_bss_131A, 1.0f,
        1.0f, 1.0f);
    Hu3DModelLayerSet(lbl_1_bss_131A, 2);
    Hu3DParticleHookSet(lbl_1_bss_131A, fn_1_24554);
    Hu3DParticleBlendModeSet(lbl_1_bss_131A, 1);
}

void fn_1_24BB4(void)
{
    Hu3DModelKill(lbl_1_bss_131A);
}

void fn_1_24BE0(HuVecF *pos)
{
    Hu3DModelPosSetV(lbl_1_bss_1318, pos);
    Hu3DModelAttrReset(lbl_1_bss_1318, HU3D_ATTR_DISPOFF);
}

void fn_1_24C28(void)
{
    Hu3DModelAttrSet(lbl_1_bss_1318, HU3D_ATTR_DISPOFF);
}

void fn_1_24C58(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    float alpha;
    s16 i;

    if (particle->count == 0) {
        i = 0;
        data = particle->data;
        for (; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        if (data->time == 0) {
            data->attr |= HU3D_PARTICLE_ATTR_SCALEY;
            data->accel.x = 0.0f;
            data->accel.y = frandmod(60) + 60;
            data->accel.z = frandmod(360);
            data->scaleBase = -(frandmod(2) + 1);
            data->zRot = 0.017453292f * data->accel.z;
            if (i < 16) {
                data->vel.z = frandmod(25) + 350;
            } else {
                data->vel.z = frandmod(25) + 150;
            }
            data->vel.x = (float)((-sin((M_PI *
                (57.29578f * data->zRot)) / 180.0) *
                data->vel.z));
            data->vel.y = (float)(cos((M_PI *
                (57.29578f * data->zRot)) / 180.0) *
                data->vel.z);
            data->scale = 0.4f * data->vel.z;
            data->scaleY = 3.0f * data->vel.z;
            data->color.r = 255;
            data->color.g = 255;
            data->color.b = 255;
            data->color.a = 204;
            data->pos.x = data->vel.x;
            data->pos.y = data->vel.y;
            data->pos.z = 0.0f;
            data->time = 1;
        } else {
            alpha = fn_1_1FE74(0.0f, 1.0f, data->accel.x,
                data->accel.y);
            data->color.a = 255.0f * alpha;
            data->accel.z += data->scaleBase;
            data->zRot = 0.017453292f * data->accel.z;
            data->vel.x = (float)((-sin((M_PI *
                (57.29578f * data->zRot)) / 180.0) *
                data->vel.z));
            data->vel.y = (float)(cos((M_PI *
                (57.29578f * data->zRot)) / 180.0) *
                data->vel.z);
            data->pos.x = data->vel.x;
            data->pos.y = data->vel.y;
            data->pos.z = 0.0f;
            if ((data->accel.x += 1.0f) > data->accel.y) {
                data->accel.x = 0.0f;
                data->accel.y = frandmod(60) + 60;
                data->accel.z = frandmod(360);
            }
        }
    }
    DCFlushRangeNoSync(particle->data,
        particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

void fn_1_251D4(void)
{
    lbl_1_bss_1318 = Hu3DParticleCreate(lbl_1_bss_14C8[6], 32);
    Hu3DModelPosSet(lbl_1_bss_1318, 0.0f,
        0.0f, 0.0f);
    Hu3DModelScaleSet(lbl_1_bss_1318, 1.0f,
        1.0f, 1.0f);
    Hu3DModelLayerSet(lbl_1_bss_1318, 2);
    Hu3DModelAttrSet(lbl_1_bss_1318, HU3D_ATTR_DISPOFF);
    Hu3DParticleHookSet(lbl_1_bss_1318, fn_1_24C58);
    Hu3DParticleBlendModeSet(lbl_1_bss_1318, 1);
}

void fn_1_252CC(void)
{
    Hu3DModelKill(lbl_1_bss_1318);
}

void fn_1_252F8(void)
{
    s16 i;

    for (i = 0; i < 7; i++) {
        lbl_1_bss_14C8[i] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_788[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    fn_1_20BC8();
    fn_1_20E9C();
    fn_1_22080();
    fn_1_22A4C();

    lbl_1_bss_14C2 = Hu3DParticleCreate(lbl_1_bss_14C8[5], 128);
    Hu3DModelPosSet(lbl_1_bss_14C2, 0.0f,
        0.0f, 0.0f);
    Hu3DModelScaleSet(lbl_1_bss_14C2, 1.0f,
        1.0f, 1.0f);
    Hu3DModelLayerSet(lbl_1_bss_14C2, 1);
    Hu3DModelAttrSet(lbl_1_bss_14C2, HU3D_ATTR_DISPOFF);
    Hu3DParticleHookSet(lbl_1_bss_14C2, fn_1_2104C);

    fn_1_23AA8();
    fn_1_24AD0();
}

void fn_1_25B90(void)
{
    fn_1_20CE0();
    fn_1_20F80();
    fn_1_221EC();
    fn_1_22C38();
    fn_1_216E8();
    fn_1_23C88();
    fn_1_24BB4();
}

void fn_1_25D0C(float value)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14C6];
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle = model->hookData;
    s16 i;

    Hu3DModelLayerSet(lbl_1_bss_14C6, 1);
    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        data->colorIdx = value;
    }
}

void fn_1_25DB0(s16 index, HuVecF *position, float alpha)
{
    HU3D_PARTICLE *particle;
    HU3D_MODEL *model;
    HU3D_PARTICLE_DATA *data;
    float opacity;

    if (index < 0 || index > 3) {
        return;
    }
    model = &Hu3DData[lbl_1_bss_14C4];
    particle = model->hookData;
    data = &particle->data[index];
    data->vel.x = position->x;
    data->vel.y = position->y;
    data->vel.z = position->z;
    opacity = 64.0f * alpha;
    data->color.a = opacity;
}

void fn_1_25E6C(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color)
{
    if (index < 0 || index > 8) {
        return;
    }
    fn_1_21904(index, parManId, velocity, accelX, color);
}

void fn_1_25FF4(s16 index)
{
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle;
    HU3D_MODEL *model;

    if (index < 0 || index > 8) {
        return;
    }
    model = &Hu3DData[lbl_1_bss_14B0[index]];
    particle = model->hookData;
    data = particle->data;
    data->time = 0;
}

void fn_1_26070(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color)
{
    if (index < 0 || index > 8) {
        return;
    }
    fn_1_21714(index, parManId, velocity, accelX, color);
}

void fn_1_26164(s16 index, HuVecF *position)
{
    GXColor color = {255, 255, 255, 64};
    s16 k;
    HU3D_PARTICLE *particle;
    HU3D_MODEL *model;
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *sharedParticle;
    HU3D_MODEL *sharedModel;
    HU3D_PARTICLE_DATA *sharedData;
    float accelX;
    s16 i;
    s16 j;

    if (index < 0 || index > 3) {
        return;
    }
    for (i = 0; i < 4; i++) {
        model = &Hu3DData[lbl_1_bss_1490[index][i]];
        particle = model->hookData;
        j = 0;
        data = particle->data;
        for (; j < particle->maxCnt; j++, data++) {
            data->time = 0;
            data->parManId = 0;
            data->scale = 0.0f;
        }
        model->attr &= ~HU3D_ATTR_DISPOFF;
        Hu3DModelPosSetV(lbl_1_bss_1490[index][i], position);
    }
    position->z += 100.0f;
    for (k = 0; k < 3; k++) {
        accelX = (float)(rand8() + 200);
        sharedModel = &Hu3DData[lbl_1_bss_131A];
        sharedParticle = sharedModel->hookData;
        sharedData = &sharedParticle->data[(s16)(k + (index * 3))];
        sharedData->time = 1;
        sharedData->pos.x = position->x;
        sharedData->pos.y = position->y;
        sharedData->pos.z = position->z;
        sharedData->scale = 0.0f;
        sharedData->color.r = color.r;
        sharedData->color.g = color.g;
        sharedData->color.b = color.b;
        sharedData->color.a = color.a;
        sharedData->vel.x = 0.0f;
        sharedData->vel.y = 4.0f;
        sharedData->vel.z = 4.0f;
        sharedData->speedDecay = (float)color.r;
        sharedData->colorIdx = (float)color.g;
        sharedData->scaleBase = (float)color.b;
        sharedData->accel.x = accelX;
        sharedData->accel.y = (float)color.a;
    }
}

void fn_1_26478(s16 index, HuVecF *position, const GXColor *color)
{
    HU3D_PARTICLE *particle;
    HU3D_MODEL *model;
    HU3D_PARTICLE_DATA *data;
    float accelX;
    s16 i;

    position->z += 100.0f;
    for (i = 0; i < 3; i++) {
        accelX = (float)(rand8() + ((i + 1) * 200));
        model = &Hu3DData[lbl_1_bss_131A];
        particle = model->hookData;
        data = &particle->data[(s16)(i + (index * 3))];
        data->time = 1;
        data->pos.x = position->x;
        data->pos.y = position->y;
        data->pos.z = position->z;
        data->scale = 0.0f;
        data->color = *color;
        data->vel.x = 0.0f;
        data->vel.y = 4.0f;
        data->vel.z = 4.0f;
        data->speedDecay = (float)color->r;
        data->colorIdx = (float)color->g;
        data->scaleBase = (float)color->b;
        data->accel.x = accelX;
        data->accel.y = (float)color->a;
    }
}

void fn_1_2668C(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color)
{
    u8 attrs[5][3] = {
        { 10, 255, 1 }, { 10, 255, 1 }, { 40, 128, 0 },
        { 100, 64, 0 }, { 200, 32, 0 }
    };
    u8 unused[4] = { 255, 255, 255, 32 };
    s16 j;

    fn_1_21904(index, parManId, velocity, accelX, color);
    fn_1_23DA0(index, color, velocity);
    for (j = 0; j < 5; j++) {
        color[3] = attrs[j][1];
        fn_1_243DC(j + index * 5, velocity, color, 1.0f, -1.0f,
            attrs[j][0], attrs[j][2]);
    }
}

void fn_1_26BE4(s16 index)
{
    HU3D_PARTICLE_DATA *burstData;
    HU3D_PARTICLE *burstParticle;
    HU3D_MODEL *burstModel;
    MDRESULT_TRAIL_WORK *work;
    s16 i;
    HU3D_PARTICLE_DATA *trailData;
    HU3D_PARTICLE *trailParticle;
    HU3D_MODEL *trailModel;

    burstModel = &Hu3DData[lbl_1_bss_14B0[index]];
    burstParticle = burstModel->hookData;
    burstData = burstParticle->data;
    burstData->time = 0;
    work = &lbl_1_bss_1320[index];
    work->state = 0;
    Hu3DModelAttrSet(lbl_1_bss_1480[index], HU3D_ATTR_DISPOFF);
    for (i = 0; i < 5; i++) {
        trailModel = &Hu3DData[lbl_1_bss_131A];
        trailParticle = trailModel->hookData;
        trailData = &trailParticle->data[(s16)(i + (index * 5))];
        trailData->time = 2;
        trailData->vel.z = 1.0f;
    }
}

void fn_1_26CF8(s16 index, HuVecF *position, float value)
{
    HU3D_PARTICLE_DATA *burstData;
    HU3D_PARTICLE *burstParticle;
    HU3D_MODEL *burstModel;
    MDRESULT_TRAIL_WORK *work;
    s16 i;

    burstModel = &Hu3DData[lbl_1_bss_14B0[index]];
    burstParticle = burstModel->hookData;
    burstData = burstParticle->data;
    fn_1_21714(index, -1, position, -1.0f, NULL);
    burstData->accel.y = value;
    work = &lbl_1_bss_1320[index];
    work->points[0].x = position->x;
    work->points[0].y = position->y;
    work->points[0].z = position->z;
    work->base.x = work->base.y = work->base.z = 0.0f;
    work->base.y = value;
    for (i = 0; i < 5; i++) {
        HU3D_PARTICLE_DATA *data;
        HU3D_PARTICLE *loopParticle;
        HU3D_MODEL *loopModel;

        loopModel = &Hu3DData[lbl_1_bss_131A];
        loopParticle = loopModel->hookData;
        data = &loopParticle->data[(s16)((index * 5) + i)];

        data->pos.x = position->x;
        data->pos.y = position->y;
        data->pos.z = position->z;
    }
}

void fn_1_26EAC(float value)
{
}

void fn_1_26EB0(HuVecF *position)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle;
    s16 i;

    fn_1_23EF0(position);
    model = &Hu3DData[lbl_1_bss_14C2];
    particle = model->hookData;
    Hu3DModelAttrReset(lbl_1_bss_14C2, HU3D_ATTR_DISPOFF);
    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        data->time = 0;
    }
    Hu3DModelPosSetV(lbl_1_bss_14C2, position);
    Hu3DModelAttrReset(lbl_1_bss_14C2, HU3D_ATTR_DISPOFF);
}

void fn_1_26F74(void)
{
    Hu3DModelAttrSet(lbl_1_bss_14C2, HU3D_ATTR_DISPOFF);
}

s32 lbl_1_data_788[7] = {
    DATANUM(DATA_mdpresult, 54),
    DATANUM(DATA_mdpresult, 51),
    DATANUM(DATA_mdpresult, 52),
    DATANUM(DATA_mdpresult, 53),
    DATANUM(DATA_mdpresult, 55),
    DATANUM(DATA_mdpresult, 57),
    DATANUM(DATA_mdpresult, 56)
};

ANIMDATA *lbl_1_bss_14C8[7];
HU3D_MODELID lbl_1_bss_14C6;
HU3D_MODELID lbl_1_bss_14C4;
HU3D_MODELID lbl_1_bss_14C2;
HU3D_MODELID lbl_1_bss_14B0[9];
HU3D_MODELID lbl_1_bss_1490[4][4];
HU3D_MODELID lbl_1_bss_1480[8];
MDRESULT_TRAIL_WORK lbl_1_bss_1320[8];
ANIMDATA *lbl_1_bss_131C;
HU3D_MODELID lbl_1_bss_131A;
HU3D_MODELID lbl_1_bss_1318;
