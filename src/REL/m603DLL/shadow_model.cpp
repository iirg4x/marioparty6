/* Model handles and camera copies used by the shared shadow model. */

#include "REL/m603DLL/shadow_owner.h"
#include "REL/m603DLL/shadow_camera.h"
#include "datadir_enum.h"
#include "dolphin/types.h"

void operator delete(void *);

extern "C" {
#include "game/data.h"
#include "game/hu3d.h"
#include "game/memory.h"





extern const u8 lbl_1_data_EC[], lbl_1_data_210[];
const Vec lbl_1_rodata_210 = { 0.0f, -1.0f, 0.01f };
const Vec lbl_1_rodata_21C = { 0.0f, 0.0f, -400.0f };
const u32 lbl_1_rodata_228[4] = { 1, 2, 4, 8 };


void insert4__4NodeFP4Node(void *, void *);
void *__ct__4NodeFv(void *);



}



static inline void fn_1_D7D4(s16 model)
{
    fn_80026A44(model, ((1 << 30) | (1 << 1)));
    fn_800269E8(model, ((1 << 30) | (1 << 0)));
    fn_80026A44(model, 1);
}

ShadowModelOwner::ShadowModelOwner(bool alternate)
    : firstModel(alternate ? DATANUM(0x3e0000, 0) : DATANUM(0x3e0000, 2)),
      shadowModel(alternate ? DATANUM(0x3e0000, 1) : DATANUM(0x3e0000, 3)),
      animatedModel(alternate ? DATANUM(0x3e0000, 4) : DATANUM(0x3e0000, 5))
{
    s16 model;
    int cameraIndex;
    model = firstModel;
    fn_80026B8C(model, 1);
    fn_80029850(model, 1);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_1_D7D4(model);
    fn_80026B64(model, 0xF);
    model = shadowModel;
    fn_80026B8C(model, 1);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_1_D7D4(model);
    fn_80026B64(model, 0xF);
    fn_80026FFC(model);
    {
        Vec direction = lbl_1_rodata_210;
        Vec target = lbl_1_rodata_21C;
        configureShadowCamera(1, & direction, & target, 650.0f, 650.0f, 5.0f);
    }
    model = animatedModel;
    motion = fn_80030458(animatedModel, fn_80008DD0(DATANUM(0x3e0000, 6), 0x10000000, 2));
    fn_8002C95C(model, motion);
    cameraModels[0] = model;
    cameraModels[1] = fn_80025D4C(animatedModel);
    cameraModels[2] = fn_80025D4C(animatedModel);
    cameraModels[3] = fn_80025D4C(animatedModel);
    for (cameraIndex = 0; 4 > cameraIndex; ++cameraIndex)
    {
        model = cameraModels[cameraIndex];
        fn_80026B8C(model, 1);
        fn_80026760(model, 0.0f, 0.0f, 0.0f);
        fn_1_D7D4(model);
        fn_80026B64(model, lbl_1_rodata_228[cameraIndex]);
    }
}
ShadowModelOwner::~ShadowModelOwner()
{
 fn_800262D0(cameraModels[1]);
 fn_800262D0(cameraModels[2]);
 fn_800262D0(cameraModels[3]);
}
void ShadowModelOwner::update() {}

/* Pause the selected model's motion. */
void ShadowModelOwner::fn_1_5914(int modelIndex)
{
    short modelId = cameraModels[modelIndex];
    fn_800269E8(modelId, ((1 << 30) | (1 << 1)));
}

void fn_1_5968(void) {}

void ShadowModelOwner::flagEnabled(int request)
{
    flags &= ~(1UL << request);
    switch (request) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        break;
    }
}
