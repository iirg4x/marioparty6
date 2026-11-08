#include <cmath>

extern "C" {
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "datadir_enum.h"
#include "game/hu3d.h"
#include "REL/m648DLL/singleton_32930.h"
}

#include "REL/framework/linknode.h"
#include "REL/m648DLL/light_shadow.h"
#include "REL/m648DLL/list_object.h"
#include "REL/framework/minigame_singleton.h"

extern "C" {
void *__dt__4NodeFv(void *object, s16 mode);
void __dl__FPv(void *object);
void Replace__5ModelFs(void *model, s32 mode);
}





extern "C" {
extern const u8 lbl_1_data_5C[];
extern u8 lbl_1_data_440[];


extern char lbl_1_data_428[];
extern char lbl_1_data_432[];
void *__ct__4NodeFv(void *);
void insert4__4NodeFP4Node(void *, const void *);
void fn_1_1D6D8(void *, s32);
void fn_1_1DE18(s16 *, s32);
void fn_1_1DAB8(s16 *, u32);
void configureShadowCamera__FsPC3VecP3Vecfff(s16, const Vec *, Vec *, f32, f32, f32);
}

CallbackModelOwner::CallbackModelOwner(bool shadow)
    : modelId(shadow ? DATANUM(0x6b0000, 0) : DATANUM(0x6b0000, 1)),
      shadowModelId(shadow ? DATANUM(0x6b0000, 2) : DATANUM(0x6b0000, 3))
{
    s16 currentModel;
    currentModel = this->modelId;
    fn_80029850(currentModel, 1);
    this->modelId.fn_1_33CB0(1);
    this->modelId.fn_1_33950(((1 << 30) | (1 << 0)));
    this->shadowModelId.fn_1_33CB0(1);
    this->shadowModelId.fn_1_33950(((1 << 30) | (1 << 0)));
    this->shadowModelId.fn_1_33950(((1 << 30) | (1 << 6)));
    Vec first = {0.0f, 0.0f, 0.0f};
    if (shadow) {
        Vec offset = {0.5f, -1.0f, -0.5f};
        configureShadowCamera__FsPC3VecP3Vecfff(1, &offset, &first, 1200.0f,
                   1000.0f, 5.0f);
        Hu3DShadowTPLvlSet(0.2f);
    } else {
        Vec offset = {0.0f, -1.0f, -0.1f};
        configureShadowCamera__FsPC3VecP3Vecfff(1, &offset, &first, 1200.0f,
                   1000.0f, 5.0f);
        Hu3DShadowTPLvlSet(0.2f);
    }
}

void CallbackModelOwner::update()
{
}

void CallbackModelOwner::clearRequest(int request)
{
    state &= ~(1UL << request);
    switch (request) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        break;
    }
}

CallbackModelOwner::~CallbackModelOwner() {}
