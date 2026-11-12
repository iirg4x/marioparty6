#include "REL/m653DLL/exhaust_smoke.h"
#include "REL/m653DLL/random.h"
#include "REL/m653DLL/particle_effects.h"
extern "C" {
#include "dolphin/types.h"
#include "dolphin/mtx.h"
}

extern "C" {
extern const GXColor lbl_1_rodata_3D0 = {0xEF, 0xFF, 0xFF, 0};
}

void fn_1_AEA8(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

int ExhaustSmoke::fn_1_A900(HU3D_PARTICLE_DATA *output)
{
    f32 factor;
    f32 velocityX;
    f32 velocityZ;

    output->time = 0;
    output->parManId = ((RandomUnsigned() & 7) + 3);
    factor = 3.0f * (0.5f + variation);
    if (factor > 3.0f) {
        factor = 3.0f;
    }
    velocityX = direction.x * factor;
    velocityZ = direction.z * factor;
    SetAccelerationXZ(output, velocityX, velocityZ);
    output->vel.y = 4.0f * lbl_1_bss_C0;
    output->pos.x = position.x;
    output->pos.y = position.y;
    output->pos.z = position.z;
    output->scale = 100.0f * (0.2f + RandomFloat(0.2f));
    output->zRot = MTXDegToRad(RandomFloat(360.0f));
    output->color = lbl_1_rodata_3D0;
    return 0;
}
ExhaustSmoke::ExhaustSmoke(ANIMDATA *animation) : GameObject(0x10)
{
    HU3D_PARTICLE_DATA *record;
    HU3D_PARTICLE_DATA *end;
    s32 index;
    s16 id;

    id = fn_8003216C(animation, 0x10);
    particles = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[id].hookData);
    particles->work = this;
    fn_800326D0((s16)id, fn_1_AEA8);
    fn_800325B0((s16)id, 0xC0, 0x50, 0x20);
    fn_8003269C((s16)id, 0);
    fn_80026B8C((s16)id, 2);
    fn_80026B64((s16)id, 1);
    fn_80026760((s16)id, 0.0f, 0.0f, 0.0f);
    particleId = (s16)id;
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 2000.0f;
    direction.x = 0.0f;
    direction.y = 0.0f;
    direction.z = 0.0f;
    variation = 0.0f;

    record = particles->data;
    end = particles->data + particles->maxCnt;
    for (index = 0; record != end; ++record, ++index) {
        fn_1_A900(record);
    }
}

float RandomFloat(float, float);

extern "C" float lbl_1_data_2D0=980.665f;

void fn_1_B4B4(HU3D_MODEL *,HU3D_PARTICLE *,Mtx);
void fn_1_BB3C(HU3D_MODEL *,HU3D_PARTICLE *,Mtx);
extern "C" {

}
ExhaustSmoke::~ExhaustSmoke()
{
    particles->work = 0;
    fn_800262D0(particleId);
}

void ExhaustSmoke::update() {}

void fn_1_AEA8(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    ExhaustSmoke *owner = static_cast<ExhaustSmoke *>(particle->work);
    owner->fn_1_AEF8(model, particle, matrix);
}

void ExhaustSmoke::fn_1_AEF8(HU3D_MODEL *, HU3D_PARTICLE *list, Mtx)
{
    HU3D_PARTICLE_DATA *entry;
    HU3D_PARTICLE_DATA *end;
    u8 targetIntensity;
    f32 speed;
    f32 minimumHeight;

    entry = list->data;
    end = list->data + list->maxCnt;
    targetIntensity = (u8)(200.0f * variation);
    if ((u8)targetIntensity < 100) {
        targetIntensity = 100;
    }

    for (; entry != end; entry++) {
        switch (entry->time) {
        case 0:
            entry->color.a += 8;
            if ((u8)entry->color.a < targetIntensity) break;
            entry->color.a = targetIntensity;
            entry->time++;
            /* fall through */
        case 1:
            entry->color.a -= entry->parManId;
            if (entry->color.a > 12) break;
            entry->color.a = 0;
            entry->time++;
            /* fall through */
        case 2:
            if (variation != 0.0f) {
                fn_1_A900(entry);
            }
            continue;
        }
        entry->zRot += MTXDegToRad(1.0f);
        entry->scale += 3.0f;
        if (entry->scale > 100.0f) {
            entry->scale = 100.0f;
        }
        speed = entry->vel.y;
        entry->vel.y += -lbl_1_bss_C0;
        speed = 0.5f * (speed + entry->vel.y);
        entry->pos.y += speed;
        minimumHeight = 0.2f * entry->scale + position.y;
        if (entry->pos.y < minimumHeight) {
            entry->pos.y = minimumHeight;
        }
        entry->pos.x += entry->accel.x;
        entry->pos.z += entry->accel.z;
    }
}

void ExhaustSmoke::fn_1_B2E8(Vec *value)
{
    position.x=value->x;
    position.y=value->y;
    position.z=value->z;
}

void ExhaustSmoke::fn_1_B304(Vec *value)
{
    direction.x=value->x;
    direction.y=value->y;
    direction.z=value->z;
}

void ExhaustSmoke::fn_1_B320(float value) { variation=value; }

RingBurst::RingBurst(ANIMDATA *animation,Vec *position)
    : GameObject(0x10),model(fn_1_B4B4,animation,0x20)
{
    model.setPosition(position);
    model.setLayer(1);
    model.SetScale(0.0f);
    model.SetTransparency(0.0f);
}
RingBurst::~RingBurst() {}
void RingBurst::update()
{
    HU3D_PARTICLE *particle=model.GetParticle();
    if(particle->dataCnt<0) delete this;
}

extern "C" {

extern float lbl_1_bss_C0,lbl_1_data_2D0;

}
void fn_1_B4B4(HU3D_MODEL *unused,HU3D_PARTICLE *work,Mtx unusedMatrix)
{
    HU3D_PARTICLE_DATA *end=work->data+work->maxCnt;
    HU3D_PARTICLE_DATA *first_entry;
    HU3D_PARTICLE_DATA *second_entry;
    int intensity;
    int active;
    if(work->count==0) {
        for(first_entry=work->data;first_entry!=end;++first_entry) {
            float angle=RandomFloat(-3.1415927f,3.1415927f);
            first_entry->vel.x=(float)cos(angle);
            first_entry->vel.y=0.0f;
            first_entry->vel.z=(float)sin(angle);
            first_entry->pos.x=100.0f*first_entry->vel.x;
            first_entry->pos.y=50.0f;
            first_entry->pos.z=100.0f*first_entry->vel.z;
            first_entry->vel.x*=20.0f;
            first_entry->vel.z*=20.0f;
            first_entry->color.a=0x80;
            first_entry->scale=300.0f;
        }
    }
    active=0;
    for(second_entry=work->data;second_entry!=end;++second_entry) {
        if(second_entry->scale<=0.0f) continue;
        PSVECAdd(&second_entry->vel,&second_entry->pos,&second_entry->pos);
        second_entry->vel.x*=0.95;
        second_entry->vel.y+=0.2;
        second_entry->vel.z*=0.95;
        intensity=second_entry->color.a;
        intensity-=4;
        if(intensity<0) {
            intensity=0;
            second_entry->scale=0.0f;
            work->dataCnt=-1;
        }
        second_entry->color.a=(u8)intensity;
        ++active;
    }
    work->emitCnt=active;
}

DustSpray::DustSpray(ANIMDATA *animation,Vec *position)
    : GameObject(0x10),model(fn_1_BB3C,animation,0x20)
{
    model.setPosition(position);
    model.setLayer(1);
    model.SetScale(0.0f);
    model.SetTransparency(0.0f);
    fn_800325B0(model,0xF0,0xE6,0x82);
    fn_1_B88C();
}
DustSpray::~DustSpray() {}
void DustSpray::update()
{
    HU3D_PARTICLE *particle=model.GetParticle();
    if(particle->dataCnt<=0) delete this;
}

void DustSpray::fn_1_B88C()
{
    HU3D_PARTICLE *state=model.GetParticle();
    HU3D_PARTICLE_DATA *element=state->data;
    s32 index=0;
    while(index<state->maxCnt) {
        element->vel.x=(float)(0.6f*sin(
            (M_PI*(double)((360.0f*(float)index)/
                (float)state->maxCnt))/180.0));
        element->vel.y=element->accel.y=1.5f*
            (lbl_1_bss_C0*(30.0f+RandomFloat(10.0f)));
        element->vel.z=(float)(0.6f*cos(
            (M_PI*(double)((360.0f*(float)index)/
                (float)state->maxCnt))/180.0));
        element->scale=80.0f;
        element->pos.x=RandomFloat(-100.0f,100.0f);
        element->pos.y=-100.0f;
        element->pos.z=RandomFloat(-100.0f,100.0f);
        element->color.a=0;
        element->time=0;
        element->parManId=(s16)((RandomUnsigned()&0xF)+0xF);
        element->zRot=(float)((M_PI*RandomFloat(360.0f))/180.0);
        state->dataCnt+=1;
        index+=1;
        element++;
    }
}
void fn_1_BB3C(HU3D_MODEL *owner,HU3D_PARTICLE *sequence,Mtx unusedMatrix)
{
    f32 previousHeight;
    f32 midpoint;
    s32 elementIndex;
    HU3D_PARTICLE_DATA *element;
    element=sequence->data;
    sequence->dataCnt=0;
    elementIndex=0;
    while(elementIndex<sequence->maxCnt) {
        if(element->time!=2) {
            if(element->time==0) {
                element->color.a=(u8)(element->color.a+element->parManId);
                if(element->color.a>=0xA0U) {
                    element->color.a=0xA0U;
                    ++element->time;
                }
            } else if(element->color.a>6U) {
                element->color.a-=4;
            } else {
                element->color.a=0;
                ++element->time;
            }
            element->scale += 100.0 / 60.0;
            previousHeight=element->vel.y;
            element->vel.y=(f32)(element->vel.y+-(1.5f*lbl_1_bss_C0));
            midpoint=0.5f*(previousHeight+element->vel.y);
            element->pos.x=(f32)((f64)element->pos.x+
                (0.016666666666666666*(f64)(100.0f*(2.0f*element->vel.x))));
            element->pos.y=(f32)(element->pos.y+midpoint);
            element->pos.z=(f32)((f64)element->pos.z+
                (0.016666666666666666*(f64)(100.0f*(2.0f*element->vel.z))));
            ++sequence->dataCnt;
        }
        elementIndex+=1;
        element++;
    }
}
extern "C" float lbl_1_bss_C0 = (1.0/60.0)*((1.0/60.0)*lbl_1_data_2D0);
