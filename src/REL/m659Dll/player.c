#include "REL/m659/player.h"
#include "game/charman.h"
#include "game/audio.h"

float lbl_1_data_70[8] = {1200, 800, 400, 0, -1200, -1200, 0, 0};
#include "REL/m659/player.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/data.h"

int lbl_1_data_90[2] = {1, 2};
int lbl_1_data_98[3] = {7733255, 7733256, 7733257};
int lbl_1_data_A4[2] = {7733260, 7733259};
int lbl_1_data_AC[10] = {1, 0, 0, 1, 0, 0, 0, 0, 0, 0};
int lbl_1_data_D4[10] = {9306144, 9306239, 9306123, 9306116, 0, 0, 0, 0, 0, 0};
/* Character-indexed hook selectors. The full stored extent is preserved;
 * the original declaration capacity is unknown. */
s16 lbl_1_data_FC[14] = {0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
char *lbl_1_data_130[2] = {"ship-stbox", "ship-stbox1"};
int fn_1_2578(M659Collision *collision, M659Collision *other);
int fn_1_4CA4(M659Collision *collision);
#include "game/audio.h"
#include "game/pad.h"

extern float lbl_1_data_70[];
void fn_1_220C(OMOBJ *obj, s16 motion);
int fn_1_2384(s16 side);
#include "REL/m659/player.h"
#include "game/data.h"
#include "game/audio.h"
#include "game/frand.h"
#include "game/hsfex.h"

extern HU3D_MODELID lbl_1_bss_28;


#include "game/mg/seqman.h"
#include "math.h"

void fn_1_29B0(OMOBJ *obj);
void fn_1_1E80(OMOBJ *obj);

void fn_1_16A8(s16 player, int result)
{
    OMOBJ *obj = lbl_1_bss_2C[player];
    M659Player *work;
    work = obj->data;
    work->info.result = result;
}

void fn_1_16E4(void)
{
    OMOBJ *obj = NULL;
    M659Player *work = NULL;
    int i = 0;
    for (i = 0; i < 2; i++) {
        HU3D_MODELID model;
        obj = lbl_1_bss_2C[i];
        work = obj->data;
        {
        int motion = -1;
        model = obj->mdlId[1];
        if (work->info.result) {
            motion = 1;
            fn_1_19DC(obj, motion);
            CharFXPlay(work->info.charNo, 577);
        }
        }
    }
}

void fn_1_1790(s16 player)
{
    M659Player *work;
    M659PlayerInfo *info;
    OMOBJ *obj = lbl_1_bss_2C[player];
    work = obj->data;
    info = &work->info;
    info->state = 0;
    obj->objFunc = fn_1_2D0C;
}

void fn_1_17E8(s16 player)
{
    M659Player *work;
    M659PlayerInfo *info;
    OMOBJ *obj = lbl_1_bss_2C[player];
    work = obj->data;
    info = &work->info;
    info->state = 0;
    obj->objFunc = fn_1_2FE0;
}

int fn_1_1840(s16 player)
{
    OMOBJ *obj = lbl_1_bss_2C[player];
    M659Player *work;
    M659Motion *motion;
    work = obj->data;
    motion = &work->motion;
    return motion->done;
}

void fn_1_1888(OMOBJ *obj)
{
    M659Player *work = obj->data;
    M659PlayerInfo *info = &work->info;
    s16 i;
    HU3D_MODELID model;
    s16 charNo;
    info->cameraBit = lbl_1_data_90[info->side];
    info->charNo = charNo = GwPlayerConf[info->playerNo].charNo;
    info->unk_0C = 1;
    model = obj->mdlId[0] = CharModelCreate(charNo, 4);
    Hu3DModelCameraSet(model, (u16)info->cameraBit);
    for (i = 0; i < 10; i++) {
        obj->mtnId[i] = CharMotionCreate(charNo, lbl_1_data_D4[i]);
    }
    Hu3DModelAttrSet(model, 1073741825);
    Hu3DModelLayerSet(model, 7);
    Hu3DModelShadowSet(model);
    CharMotionDataClose(charNo);
    fn_1_19DC(obj, 0);
    work->padNo = GwPlayerConf[info->playerNo].padNo;
    work->stickX = work->stickY = 0.0f;
}

void fn_1_19DC(OMOBJ *obj, s16 motion)
{
    M659Player *work = obj->data;
    s16 charNo = work->info.charNo;
    HU3D_MODELID model = obj->mdlId[0];
    u32 attr = 0;
    if (lbl_1_data_AC[motion]) {
        attr = 1073741825;
    }
    if (work->info.motionId != motion) {
        CharMotionShiftSet(charNo, obj->mtnId[motion], 0.0f, 8.0f, attr);
    } else {
        CharMotionSet(charNo, obj->mtnId[motion]);
    }
    switch (motion) {
    case 0:
    case 3:
        Hu3DModelAttrSet(model, 1073741825);
        break;
    default:
        Hu3DModelAttrReset(model, 1073741825);
        break;
    }
    work->info.motionId = motion;
}

void fn_1_1AF4(OMOBJ *obj)
{
    M659Player *work = obj->data;
    M659PlayerInfo *info = &work->info;
    M659Motion *motion = &work->motion;
    HU3D_MODELID charModel = obj->mdlId[0];
    int unused = 0;
    int masks[2] = {1, 2};
    HU3D_MODELID model;
    s16 i;
    HU3D_MOTIONID motionId;
    M659Collision *collision;
    model = obj->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(7733254, 268435456, HEAP_MODEL));
    for (i = 4; i < 7; i++) {
        motionId = obj->mtnId[i] = Hu3DJointMotion(model, HuDataSelHeapReadNum(lbl_1_data_98[i - 4], 268435456, HEAP_MODEL));
    }
    Hu3DMotionSet(model, motionId);
    Hu3DModelAttrSet(model, 1073741825);
    Hu3DMotionSpeedSet(model, 1.0f);
    Hu3DModelLayerSet(model, 7);
    Hu3DModelHookSet(model, lbl_1_data_130[lbl_1_data_FC[info->charNo]], charModel);
    Hu3DModelCameraSet(model, (u16)info->cameraBit);
    {
        char *hooks[2] = {"ship-burnerhookL", "ship-burnerhookR"};
        s16 partMotion;
        HU3D_MODELID partModel;
        s16 part;
        HU3D_MOTIONID partMotionId;
        for (part = 2; part < 4; part++) {
            partModel = obj->mdlId[part] = Hu3DModelCreate(HuDataSelHeapReadNum(7733258, 268435456, HEAP_MODEL));
            for (partMotion = 7; partMotion < 9; partMotion++) {
                partMotionId = obj->mtnId[partMotion] = Hu3DJointMotion(partModel, HuDataSelHeapReadNum(lbl_1_data_A4[partMotion - 7], 268435456, HEAP_MODEL));
            }
            Hu3DMotionSet(partModel, partMotionId);
            Hu3DModelAttrSet(partModel, 1073741825);
            Hu3DMotionSpeedSet(partModel, 1.0f);
            Hu3DModelLayerSet(partModel, 7);
            Hu3DModelHookSet(obj->mdlId[1], hooks[part - 2], partModel);
            Hu3DModelCameraSet(partModel, (u16)info->cameraBit);
        }
    }
    collision = &work->collision;
    collision->flags = 1;
    collision->mask = masks[info->side];
    collision->unk_08 = 0;
    collision->unk_0C = 0;
    collision->unk_10 = 0;
    collision->userData = obj;
    collision->update = NULL;
    collision->collide = fn_1_2578;
    collision->pos = &motion->pos;
    collision->velocity = &motion->velocity;
    collision->previous = motion->pos;
    collision->radius = 150.0f;
    collision->factor = 1.0f;
    fn_1_4CA4(collision);
    motion->state = 0;
    motion->column = 3;
    motion->start.x = motion->start.y = motion->start.z = 0.0f;
    motion->velocity.x = motion->velocity.y = motion->velocity.z = 0.0f;
    motion->done = 0;
    motion->direction = motion->previousDirection = -1;
}

void fn_1_1E80(OMOBJ *obj)
{
    M659Player *work = obj->data;
    M659Motion *motion = &work->motion;
    HU3D_MODELID model = obj->mdlId[1];
    HU3D_MODELID unused = obj->mdlId[4];
    if (motion->state != 0) {
        switch (motion->substate) {
        case 0:
            if (!fn_1_1840(0) && !fn_1_1840(1)) {
                if (fn_1_2384(work->info.side)) {
                    motion->substate++;
                }
                Hu3DModelObjPosGet(model, "ship-Ship", &motion->pos);
            } else {
                Hu3DModelObjPosGet(model, "ship-Ship", &motion->start);
                Hu3DModelPosSetV(model, &work->motion.start);
                fn_1_220C(obj, 6);
                motion->direction = -1;
                motion->substate = 0;
                motion->state = 1;
            }
            break;
        case 1:
            motion->pos.x = lbl_1_data_70[motion->column];
            motion->start = motion->pos;
            Hu3DModelPosSetV(model, &work->motion.start);
            fn_1_220C(obj, 6);
            motion->direction = -1;
            motion->substate = 0;
            motion->state = 0;
            if (work->com.isCom) {
                work->com.active = 0;
                work->com.state = 0;
            }
            break;
        }
    } else {
        if (work->com.isCom) {
            if (work->com.active) {
                motion->direction = work->com.direction;
                work->com.direction = -1;
            } else {
                work->com.direction = -1;
                motion->direction = -1;
            }
        } else {
            u16 buttons = HuPadBtnDown[work->padNo] & 24576;
            motion->direction = -1;
            if (buttons != 24576) {
                if (buttons & 16384) {
                    motion->direction = 0;
                }
                if (buttons & 8192) {
                    motion->direction = 1;
                }
            }
        }
        if (motion->direction != motion->previousDirection) {
            switch (motion->direction) {
            case 0:
                motion->column--;
                if (motion->column < 0) {
                    motion->column = 0;
                } else {
                    fn_1_220C(obj, 4);
                    motion->state = 1;
                    motion->substate = 0;
                    {
                        int fx[2] = {2105, 2106};
                        HuAudFXPlayPan(fx[work->info.side], work->info.side * 32 + 48);
                    }
                }
                break;
            case 1:
                motion->column++;
                if (motion->column > 6) {
                    motion->column = 6;
                } else {
                    fn_1_220C(obj, 5);
                    motion->state = 1;
                    motion->substate = 0;
                    {
                        int fx[2] = {2105, 2106};
                        HuAudFXPlayPan(fx[work->info.side], work->info.side * 32 + 48);
                    }
                }
                break;
            }
        }
    }
    motion->previousDirection = motion->direction;
}

void fn_1_220C(OMOBJ *obj, s16 motion)
{
    M659Player *work = obj->data;
    HU3D_MODELID model = obj->mdlId[1];
    HU3D_MODELID left = obj->mdlId[2];
    HU3D_MODELID right = obj->mdlId[3];
    int unused = 0;
    if (motion == 6) {
        Hu3DMotionSet(model, obj->mtnId[motion]);
        Hu3DModelAttrSet(model, 1073741825);
        Hu3DMotionSet(left, obj->mtnId[7]);
        Hu3DModelAttrSet(left, 1073741825);
        Hu3DMotionSet(right, obj->mtnId[7]);
        Hu3DModelAttrSet(right, 1073741825);
    } else {
        Hu3DMotionSet(model, obj->mtnId[motion]);
        Hu3DModelAttrReset(model, 1073741825);
        Hu3DMotionSet(left, obj->mtnId[8]);
        Hu3DModelAttrSet(left, 1073741825);
        Hu3DMotionSet(right, obj->mtnId[8]);
        Hu3DModelAttrSet(right, 1073741825);
    }
    Hu3DMotionSpeedSet(model, 1.5f);
    Hu3DMotionSpeedSet(left, 1.0f);
    Hu3DMotionSpeedSet(right, 1.0f);
}

int fn_1_2384(s16 side)
{
    OMOBJ *obj = lbl_1_bss_2C[side];
    HU3D_MODELID model = obj->mdlId[1];
    if (Hu3DMotionEndCheck(model)) {
        return 1;
    }
    return 0;
}

void fn_1_23F0(OMOBJ *obj)
{
    M659Player *work = obj->data;
    M659PlayerInfo *info = &work->info;
    HU3D_MODELID model;
    HU3D_MOTIONID motion;
    model = obj->mdlId[4] = Hu3DModelCreate(HuDataSelHeapReadNum(7733266, 268435456, HEAP_MODEL));
    motion = obj->mtnId[9] = Hu3DJointMotion(model, HuDataSelHeapReadNum(7733267, 268435456, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DModelAttrSet(model, 1);
    Hu3DMotionSpeedSet(model, 0.0f);
    Hu3DModelLayerSet(model, 7);
    Hu3DModelCameraSet(model, (u16)info->cameraBit);
}

void fn_1_24C0(OMOBJ *obj)
{
    HU3D_MODELID model = obj->mdlId[4];
    Hu3DModelAttrReset(model, 1);
    Hu3DMotionSpeedSet(model, 1.0f);
}

int fn_1_2518(OMOBJ *obj)
{
    HU3D_MODELID model = obj->mdlId[4];
    if (Hu3DMotionEndCheck(model)) {
        Hu3DModelAttrSet(model, 1);
        return 1;
    }
    return 0;
}

int fn_1_2578(M659Collision *collision, M659Collision *other)
{
    OMOBJ *obj = collision->userData;
    M659Player *work = obj->data;
    M659Motion *motion = &work->motion;
    int fx[2] = {2107, 2108};
    HuAudFXPlayPan(fx[work->info.side], work->info.side * 32 + 48);
    HuAudFXStop(work->info.unk_34);
    motion->done = 1;
    collision->flags = 0;
    fn_1_24C0(obj);
    omVibrate(work->info.playerNo, 20, 20, 0);
    return 1;
}

void fn_1_2648(OMOBJ *obj)
{
    M659Player *work = obj->data;
    fn_1_1888(obj);
    fn_1_1AF4(obj);
    fn_1_23F0(obj);
    fn_1_5CB0(obj);
    fn_1_220C(obj, 6);
    obj->objFunc = fn_1_29B0;
}

void fn_1_29B0(OMOBJ *obj)
{
    M659Player *work = obj->data;
    M659PlayerInfo *info = &work->info;
    M659Motion *motion = &work->motion;
    switch (info->state) {
    case 0:
        if (info->unk_34 == -1) {
            int fx[2] = {2109, 2110};
            info->unk_34 = HuAudFXPlayPan(fx[work->info.side], work->info.side * 32 + 48);
        }
        if (MgSeqModeGet() >= 5) {
            info->state++;
        }
        break;
    case 1:
        if (MgSeqModeGet() > 5) {
            HU3D_MODELID left = obj->mdlId[2];
            HU3D_MODELID right = obj->mdlId[3];
            Hu3DMotionSet(left, obj->mtnId[7]);
            Hu3DModelAttrSet(left, 1073741825);
            Hu3DMotionSet(right, obj->mtnId[7]);
            Hu3DModelAttrSet(right, 1073741825);
            {
                M659Player *startWork = obj->data;
                M659Motion *startMotion = &startWork->motion;
                HU3D_MODELID model = obj->mdlId[1];
                HU3D_MODELID unused = obj->mdlId[4];
                Hu3DModelObjPosGet(model, "ship-Ship", &startMotion->start);
                Hu3DModelPosSetV(model, &startWork->motion.start);
                fn_1_220C(obj, 6);
                startMotion->direction = -1;
                startMotion->substate = 0;
                startMotion->state = 1;
            }
            info->state++;
        } else {
            fn_1_1E80(obj);
        }
        break;
    case 2:
        break;
    }
    {
        HU3D_MODELID model = obj->mdlId[1];
        motion->start.y = 20.0 * sin(motion->phase);
        if (motion->state == 0) {
            motion->phase += 0.1f;
        }
        Hu3DModelPosSetV(model, &work->motion.start);
    }
    if (motion->done) {
        HU3D_MODELID model = obj->mdlId[1];
        HU3D_MODELID effect = obj->mdlId[4];
        fn_1_2518(obj);
        work->motion.start.y += 8.0f;
        work->motion.start.z -= 8.0f;
        work->motion.unk_18.x -= 20.0f;
        Hu3DModelPosSetV(model, &work->motion.start);
        Hu3DModelRotSetV(model, &work->motion.unk_18);
        Hu3DModelPosSetV(effect, &work->motion.pos);
    }
}

void fn_1_2D0C(OMOBJ *obj)
{
    M659Player *work = obj->data;
    M659PlayerInfo *info = &work->info;
    M659Motion *motion = &work->motion;
    HU3D_MODELID charModel = obj->mdlId[0];
    HU3D_MODELID model = obj->mdlId[1];
    Mtx mtx;
    HuVecF pos, rot;
    switch (motion->substate) {
    case 0:
        motion->start.x = 0.0f;
        motion->start.y = 0.0f;
        motion->start.z = 0.0f;
        motion->unk_18.x = 0.0f;
        motion->unk_18.y = 90.0f;
        motion->unk_18.z = 0.0f;
        motion->substate++;
        Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(lbl_1_bss_28, 1);
        Hu3DModelAttrReset(model, 1);
        Hu3DModelAttrReset(lbl_1_bss_28, 1073741826);
        info->unk_1C.x = info->unk_1C.y = info->unk_1C.z = 0.0f;
        info->unk_28.x = info->unk_28.z = 0.0f;
        info->unk_28.y = 180.0f;
        info->unk_1C.z = -100.0f;
        info->unk_1C.x = -50.0f;
        fn_1_220C(obj, 6);
        HuAudFXStop(work->info.unk_34);
        return;
    case 1:
        if (Hu3DMotionEndCheck(charModel)) {
            Hu3DMotionTimeSet(charModel, 10.0f);
        }
        Hu3DModelObjMtxGet(lbl_1_bss_28, "ENDmot-grid1", mtx);
        Hu3DMtxTransGet(mtx, &pos);
        Hu3DMtxRotGet(mtx, &rot);
        Hu3DModelPosSetV(model, &pos);
        Hu3DModelRotSetV(model, &rot);
        break;
    }
}

void fn_1_2FE0(OMOBJ *obj)
{
    M659Player *work = obj->data;
    M659Motion *motion = &work->motion;
    int height;
    HU3D_MODELID model = obj->mdlId[1];
    HuVecF end;
    switch (motion->substate) {
    case 0:
        if ((frand() & 1) == 0) {
            motion->start.x = 3000.0f;
            end.x = -3000.0f;
        } else {
            motion->start.x = -3000.0f;
            end.x = 3000.0f;
        }
        end.z = motion->start.z = 4000.0f;
        height = 6000;
        motion->start.y = -3000.0f + (float)(frand() % height);
        end.y = -3000.0f + (float)(frand() % height);
        PSVECSubtract(&end, &work->motion.start, &motion->unk_24);
        PSVECNormalize(&motion->unk_24, &motion->unk_24);
        fn_1_220C(obj, 6);
        motion->substate++;
        /* fall through */
    case 1:
        motion->start.x += 40.0f * motion->unk_24.x;
        motion->start.y += 40.0f * motion->unk_24.y;
        motion->unk_18.x -= 20.0f;
        motion->unk_18.y = 90.0f;
        break;
    }
    Hu3DModelPosSetV(model, &work->motion.start);
    Hu3DModelRotSetV(model, &work->motion.unk_18);
}
