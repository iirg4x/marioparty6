#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/m603DLL/model_motion.h"
#include "REL/framework/frame_time_inline.h"
#include "REL/m603DLL/zako.h"
#include "REL/m603DLL/random.h"
#include "REL/m603DLL/night.h"
#include "datadir_enum.h"

extern "C" {
#include "dolphin/types.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/memory.h"
#include "REL/m603DLL.h"
}

#include "REL/m603DLL/draw_node.h"
#include "REL/framework/assertion.h"

extern "C" {
const u32 lbl_1_rodata_398[17] = {
    DATANUM(0x3e0000, 20),
    DATANUM(0x3e0000, 21),
    DATANUM(0x3e0000, 22),
    DATANUM(0x3e0000, 23),
    DATANUM(0x3e0000, 24),
    DATANUM(0x3e0000, 25),
    DATANUM(0x3e0000, 26),
    DATANUM(0x3e0000, 27),
    DATANUM(0x3e0000, 28),
    DATANUM(0x3e0000, 29),
    DATANUM(0x3e0000, 30),
    DATANUM(0x3e0000, 31),
    DATANUM(0x3e0000, 32),
    DATANUM(0x3e0000, 33),
    DATANUM(0x3e0000, 34),
    DATANUM(0x3e0000, 35),
    DATANUM(0x3e0000, 36)
};
extern s16 lbl_802C0278;
}
DrawModelPool::DrawModelPool()
    : model(DATANUM(0x3e0000, 10))
{
    s32 outerIndex;
    s32 innerIndex;
    s32 nightOuterIndex;
    s32 nightInnerIndex;
    s16 modelId;
    modelId = this->model;
    fn_800269E8(modelId, 1);
    this->m_mot_id.clear();
    this->m_mot_id.reserve(NUMBEROF_PMOTION);
    fn_1_8EA0(DATANUM(0x3e0000, 11));
    fn_1_8EA0(DATANUM(0x3e0000, 12));
    assert(m_mot_id.size() == NUMBEROF_PMOTION);
    for (outerIndex = 0; outerIndex < 4; outerIndex++) {
        for (innerIndex = 0; innerIndex < 17; innerIndex++) {
            new DrawNode(&this->model, lbl_1_rodata_398[innerIndex], innerIndex, outerIndex);
        }
    }

    if (GWMgNightFGet() == 0) {
        for (nightOuterIndex = 0; nightOuterIndex < 4; nightOuterIndex++) {
            for (nightInnerIndex = 0; nightInnerIndex < 2; nightInnerIndex++) {
                new MotionModel(nightOuterIndex * 2 + nightInnerIndex,
                                   nightOuterIndex);
            }
        }
    }
}

HU3D_MOTIONID DrawModelPool::fn_1_8EA0(unsigned int dataNum)
{
    HU3D_MOTIONID motionId = fn_80030458(model, fn_80008DD0(dataNum, 0x10000000, 2));
    m_mot_id.push_back(motionId);
    return motionId;
}

DrawModelPool::~DrawModelPool()
{
}

void fn_1_98AC();
void fn_1_995C();
void fn_1_D698();
void fn_1_9794();

void DrawModelPool::update()
{
    if ((s32)(RandomUnsigned() & 0x3FUL) < 3) {
        fn_1_995C();
    }
    fn_1_98AC();
    fn_1_D698();
}

void DrawModelPool::flagEnabled(int bit)
{
    flags &= ~(1UL << bit);
    switch (bit) {
    case 0:
    case 1:
    case 2:
    case 3:
        break;
    case 4:
        fn_1_9794();
        fn_1_A12C();
        break;
    }
}
