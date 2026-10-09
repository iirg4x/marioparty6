/* Player ships, character animations, lane input, and collision response for Asteroad Rage. */
#define M659_CHAR_FX_COURSE_CLEAR 577
#define M659_SFX_LANE_SIDE_0 2105
#define M659_SFX_LANE_SIDE_1 2106
#define M659_SFX_HIT_SIDE_0 2107
#define M659_SFX_HIT_SIDE_1 2108
#define M659_SFX_ENGINE_SIDE_0 2109
#define M659_SFX_ENGINE_SIDE_1 2110
#define M659_DATA_SHIP_MODEL DATANUM(DATA_m659, 6)
#define M659_DATA_SHIP_MOTION_0 DATANUM(DATA_m659, 7)
#define M659_DATA_SHIP_MOTION_1 DATANUM(DATA_m659, 8)
#define M659_DATA_SHIP_MOTION_2 DATANUM(DATA_m659, 9)
#define M659_DATA_BURNER_MODEL DATANUM(DATA_m659, 10)
#define M659_DATA_BURNER_MOTION_LEFT DATANUM(DATA_m659, 12)
#define M659_DATA_BURNER_MOTION_RIGHT DATANUM(DATA_m659, 11)
#define M659_DATA_ENDING_EFFECT_MODEL DATANUM(DATA_m659, 18)
#define M659_DATA_ENDING_EFFECT_MOTION DATANUM(DATA_m659, 19)
#define M659_DATA_CHARACTER_MOTION_0 DATANUM(DATA_mario, 32)
#define M659_DATA_CHARACTER_MOTION_1 DATANUM(DATA_mario, 127)
#define M659_DATA_CHARACTER_MOTION_2 DATANUM(DATA_mario, 11)
#define M659_DATA_CHARACTER_MOTION_3 DATANUM(DATA_mario, 4)
#include "REL/m659/player.h"
#include "game/charman.h"
#include "game/audio.h"

float lbl_1_data_70[8] = {1200, 800, 400, 0, -1200, -1200, 0, 0};
#include "REL/m659/player.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/data.h"

int lbl_1_data_90[2] = {1, 2};
int lbl_1_data_98[3] = {M659_DATA_SHIP_MOTION_0, M659_DATA_SHIP_MOTION_1, M659_DATA_SHIP_MOTION_2};
int lbl_1_data_A4[2] = {M659_DATA_BURNER_MOTION_LEFT, M659_DATA_BURNER_MOTION_RIGHT};
int lbl_1_data_AC[10] = {1, 0, 0, 1, 0, 0, 0, 0, 0, 0};
int lbl_1_data_D4[10] = {
    M659_DATA_CHARACTER_MOTION_0, M659_DATA_CHARACTER_MOTION_1,
    M659_DATA_CHARACTER_MOTION_2, M659_DATA_CHARACTER_MOTION_3,
    0, 0,
    0, 0,
    0, 0
};
/* Selects the ship attachment point used for each character model. */
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

/* The result sequence calls this to store a side's clear result on its player object. */
void fn_1_16A8(s16 side, int result)
{
    OMOBJ *obj = lbl_1_bss_2C[side];
    M659Player *playerData;
    playerData = obj->data;
    playerData->info.result = result;
}

/* The result sequence calls this when a winner is announced to start winner animations and
 * effects. */
void fn_1_16E4(void)
{
    OMOBJ *obj = NULL;
    M659Player *playerData = NULL;
    int i = 0;
    for (i = 0; i < 2; i++) {
        HU3D_MODELID playerShipModel; /* Read from the player object below but not used by this
                                       * callback. */
        obj = lbl_1_bss_2C[i];
        playerData = obj->data;
        {
        int motion = -1;
        playerShipModel = obj->mdlId[1];
        if (playerData->info.result) {
            motion = 1;
            fn_1_19DC(obj, motion);
            CharFXPlay(playerData->info.charNo, M659_CHAR_FX_COURSE_CLEAR);
        }
        }
    }
}

/* The result transition calls this for the winner to install the course-clear ending callback. */
void fn_1_1790(s16 side)
{
    M659Player *playerData;
    M659PlayerInfo *info;
    OMOBJ *obj = lbl_1_bss_2C[side];
    playerData = obj->data;
    info = &playerData->info;
    info->state = 0;
    obj->objFunc = fn_1_2D0C;
}

/* The result transition calls this for the other side to install its losing ending callback. */
void fn_1_17E8(s16 side)
{
    M659Player *playerData;
    M659PlayerInfo *info;
    OMOBJ *obj = lbl_1_bss_2C[side];
    playerData = obj->data;
    info = &playerData->info;
    info->state = 0;
    obj->objFunc = fn_1_2FE0;
}

/* Sequence and course callbacks call this to check whether a side's ship has finished its run. */
int fn_1_1840(s16 side)
{
    OMOBJ *obj = lbl_1_bss_2C[side];
    M659Player *playerData;
    M659Motion *motion;
    playerData = obj->data;
    motion = &playerData->motion;
    return motion->done;
}

/* Creates the character model and its course animations when the player object starts. */
void fn_1_1888(OMOBJ *obj)
{
    M659Player *playerData = obj->data;
    M659PlayerInfo *info = &playerData->info;
    s16 i;
    HU3D_MODELID model;
    s16 charNo;
    info->cameraBit = lbl_1_data_90[info->side];
    info->charNo = charNo = GwPlayerConf[info->playerNo].charNo;
    info->initializationMarker = 1;
    model = obj->mdlId[0] = CharModelCreate(charNo, 4);
    Hu3DModelCameraSet(model, (u16)info->cameraBit);
    for (i = 0; i < 10; i++) {
        obj->mtnId[i] = CharMotionCreate(charNo, lbl_1_data_D4[i]);
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 7);
    Hu3DModelShadowSet(model);
    CharMotionDataClose(charNo);
    fn_1_19DC(obj, 0);
    playerData->padNo = GwPlayerConf[info->playerNo].padNo;
    playerData->stickX = playerData->stickY = 0.0f;
}

/* Character setup and result callbacks call this to change animation and set its looping state. */
void fn_1_19DC(OMOBJ *obj, s16 motion)
{
    M659Player *playerData = obj->data;
    s16 charNo = playerData->info.charNo;
    HU3D_MODELID model = obj->mdlId[0];
    u32 attr = 0;
    if (lbl_1_data_AC[motion]) {
        attr = HU3D_MOTATTR_LOOP;
    }
    if (playerData->info.motionId != motion) {
        CharMotionShiftSet(charNo, obj->mtnId[motion], 0.0f, 8.0f, attr);
    } else {
        CharMotionSet(charNo, obj->mtnId[motion]);
    }
    switch (motion) {
    case 0:
    case 3:
        Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
        break;
    default:
        Hu3DModelAttrReset(model, HU3D_MOTATTR_LOOP);
        break;
    }
    playerData->info.motionId = motion;
}

/* Creates the ship, burner parts, collision state, and their animations at player-object
 * startup. */
void fn_1_1AF4(OMOBJ *obj)
{
    M659Player *playerData = obj->data;
    M659PlayerInfo *info = &playerData->info;
    M659Motion *motion = &playerData->motion;
    HU3D_MODELID charModel = obj->mdlId[0];
    int zeroValue = 0; /* Initialized to zero but not read by this setup callback. */
    int collisionMasks[2] = {1, 2};
    HU3D_MODELID model;
    s16 i;
    HU3D_MOTIONID motionId;
    M659Collision *collision;
    model = obj->mdlId[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(M659_DATA_SHIP_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
    for (i = 4; i < 7; i++) {
        motionId = obj->mtnId[i] = Hu3DJointMotion(
            model, HuDataSelHeapReadNum(lbl_1_data_98[i - 4], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    Hu3DMotionSet(model, motionId);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
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
            partModel = obj->mdlId[part] = Hu3DModelCreate(
                HuDataSelHeapReadNum(M659_DATA_BURNER_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
            for (partMotion = 7; partMotion < 9; partMotion++) {
                partMotionId = obj->mtnId[partMotion] =
                    Hu3DJointMotion(partModel, HuDataSelHeapReadNum(lbl_1_data_A4[partMotion - 7],
                                                                    HU_MEMNUM_OVL, HEAP_MODEL));
            }
            Hu3DMotionSet(partModel, partMotionId);
            Hu3DModelAttrSet(partModel, HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(partModel, 1.0f);
            Hu3DModelLayerSet(partModel, 7);
            Hu3DModelHookSet(obj->mdlId[1], hooks[part - 2], partModel);
            Hu3DModelCameraSet(partModel, (u16)info->cameraBit);
        }
    }
    collision = &playerData->collision;
    collision->flags = 1 << 0;
    collision->mask = collisionMasks[info->side];
    collision->collisionImpulseEnabled = 0;
    collision->unusedCollisionWord = 0;
    collision->collisionDisabled = 0;
    collision->userData = obj;
    collision->update = NULL;
    collision->collide = fn_1_2578;
    collision->position = &motion->position;
    collision->velocity = &motion->velocity;
    collision->previousPosition = motion->position;
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

/* The player frame callback calls this before the start to read lane requests and move the ship. */
void fn_1_1E80(OMOBJ *obj)
{
    M659Player *playerData = obj->data;
    M659Motion *motion = &playerData->motion;
    HU3D_MODELID model = obj->mdlId[1];
    HU3D_MODELID endingEffectModel = obj->mdlId[4]; /* Captured here but not used by this block. */
    if (motion->state != 0) {
        switch (motion->substate) {
        case 0:
            if (!fn_1_1840(0) && !fn_1_1840(1)) {
                if (fn_1_2384(playerData->info.side)) {
                    motion->substate++;
                }
                Hu3DModelObjPosGet(model, "ship-Ship", &motion->position);
            } else {
                Hu3DModelObjPosGet(model, "ship-Ship", &motion->start);
                Hu3DModelPosSetV(model, &playerData->motion.start);
                fn_1_220C(obj, 6);
                motion->direction = -1;
                motion->substate = 0;
                motion->state = 1;
            }
            break;
        case 1:
            motion->position.x = lbl_1_data_70[motion->column];
            motion->start = motion->position;
            Hu3DModelPosSetV(model, &playerData->motion.start);
            fn_1_220C(obj, 6);
            motion->direction = -1;
            motion->substate = 0;
            motion->state = 0;
            if (playerData->com.isCom) {
                playerData->com.active = 0;
                playerData->com.state = 0;
            }
            break;
        }
    } else {
        if (playerData->com.isCom) {
            if (playerData->com.active) {
                motion->direction = playerData->com.direction;
                playerData->com.direction = -1;
            } else {
                playerData->com.direction = -1;
                motion->direction = -1;
            }
        } else {
            u16 buttons = HuPadBtnDown[playerData->padNo] &
                          (PAD_BUTTON_TRIGGER_L | PAD_BUTTON_TRIGGER_R);
            motion->direction = -1;
            /* Simultaneous presses of both lane buttons are discarded. */
            if (buttons != (PAD_BUTTON_TRIGGER_L | PAD_BUTTON_TRIGGER_R)) {
                if (buttons & PAD_BUTTON_TRIGGER_L) {
                    motion->direction = 0;
                }
                if (buttons & PAD_BUTTON_TRIGGER_R) {
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
                        int sideSoundIds[2] = {M659_SFX_LANE_SIDE_0, M659_SFX_LANE_SIDE_1};
                        HuAudFXPlayPan(sideSoundIds[playerData->info.side],
                                       playerData->info.side * 32 + 48);
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
                        int sideSoundIds[2] = {M659_SFX_LANE_SIDE_0, M659_SFX_LANE_SIDE_1};
                        HuAudFXPlayPan(sideSoundIds[playerData->info.side],
                                       playerData->info.side * 32 + 48);
                    }
                }
                break;
            }
        }
    }
    motion->previousDirection = motion->direction;
}

/* Player setup and movement callbacks call this to select ship and burner animations. */
void fn_1_220C(OMOBJ *obj, s16 motion)
{
    M659Player *playerData = obj->data;
    HU3D_MODELID model = obj->mdlId[1];
    HU3D_MODELID left = obj->mdlId[2];
    HU3D_MODELID right = obj->mdlId[3];
    int zeroValue = 0; /* Initialized to zero but not read by this animation helper. */
    if (motion == 6) {
        Hu3DMotionSet(model, obj->mtnId[motion]);
        Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
        Hu3DMotionSet(left, obj->mtnId[7]);
        Hu3DModelAttrSet(left, HU3D_MOTATTR_LOOP);
        Hu3DMotionSet(right, obj->mtnId[7]);
        Hu3DModelAttrSet(right, HU3D_MOTATTR_LOOP);
    } else {
        Hu3DMotionSet(model, obj->mtnId[motion]);
        Hu3DModelAttrReset(model, HU3D_MOTATTR_LOOP);
        Hu3DMotionSet(left, obj->mtnId[8]);
        Hu3DModelAttrSet(left, HU3D_MOTATTR_LOOP);
        Hu3DMotionSet(right, obj->mtnId[8]);
        Hu3DModelAttrSet(right, HU3D_MOTATTR_LOOP);
    }
    Hu3DMotionSpeedSet(model, 1.5f);
    Hu3DMotionSpeedSet(left, 1.0f);
    Hu3DMotionSpeedSet(right, 1.0f);
}

/* The pre-start movement callback calls this to detect the end of a side's ship animation. */
int fn_1_2384(s16 side)
{
    OMOBJ *obj = lbl_1_bss_2C[side];
    HU3D_MODELID model = obj->mdlId[1];
    if (Hu3DMotionEndCheck(model)) {
        return 1;
    }
    return 0;
}

/* Player object setup calls this to create the ship's end effect model and motion. */
void fn_1_23F0(OMOBJ *obj)
{
    M659Player *playerData = obj->data;
    M659PlayerInfo *info = &playerData->info;
    HU3D_MODELID model;
    HU3D_MOTIONID motion;
    model = obj->mdlId[4] = Hu3DModelCreate(
        HuDataSelHeapReadNum(M659_DATA_ENDING_EFFECT_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
    motion = obj->mtnId[9] = Hu3DJointMotion(
        model, HuDataSelHeapReadNum(M659_DATA_ENDING_EFFECT_MOTION, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.0f);
    Hu3DModelLayerSet(model, 7);
    Hu3DModelCameraSet(model, (u16)info->cameraBit);
}

/* The ship collision callback calls this to reveal and start the end effect. */
void fn_1_24C0(OMOBJ *obj)
{
    HU3D_MODELID model = obj->mdlId[4];
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 1.0f);
}

/* The per-frame player callback calls this to hide the end effect when its motion ends. */
int fn_1_2518(OMOBJ *obj)
{
    HU3D_MODELID model = obj->mdlId[4];
    if (Hu3DMotionEndCheck(model)) {
        Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
        return 1;
    }
    return 0;
}

/* Collision callback that plays the hit effect, stops the engine sound, and marks the ship
 * finished. */
int fn_1_2578(M659Collision *collision, M659Collision *other)
{
    OMOBJ *obj = collision->userData;
    M659Player *playerData = obj->data;
    M659Motion *motion = &playerData->motion;
    int sideSoundIds[2] = {M659_SFX_HIT_SIDE_0, M659_SFX_HIT_SIDE_1};
    HuAudFXPlayPan(sideSoundIds[playerData->info.side], playerData->info.side * 32 + 48);
    HuAudFXStop(playerData->info.engineSoundHandle);
    motion->done = 1;
    collision->flags = 0;
    fn_1_24C0(obj);
    omVibrate(playerData->info.playerNo, 20, 20, 0);
    return 1;
}

/* Player object setup callback; creates its models and installs the regular update callback. */
void fn_1_2648(OMOBJ *obj)
{
    M659Player *playerData = obj->data;
    fn_1_1888(obj);
    fn_1_1AF4(obj);
    fn_1_23F0(obj);
    fn_1_5CB0(obj);
    fn_1_220C(obj, 6);
    obj->objFunc = fn_1_29B0;
}

/* Per-frame player callback for the course start, lane input, ship bob, and collision aftermath. */
void fn_1_29B0(OMOBJ *obj)
{
    M659Player *playerData = obj->data;
    M659PlayerInfo *info = &playerData->info;
    M659Motion *motion = &playerData->motion;
    switch (info->state) {
    case 0:
        if (info->engineSoundHandle == -1) {
            int sideSoundIds[2] = {M659_SFX_ENGINE_SIDE_0, M659_SFX_ENGINE_SIDE_1};
            info->engineSoundHandle = HuAudFXPlayPan(sideSoundIds[playerData->info.side],
                                                     playerData->info.side * 32 + 48);
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
            Hu3DModelAttrSet(left, HU3D_MOTATTR_LOOP);
            Hu3DMotionSet(right, obj->mtnId[7]);
            Hu3DModelAttrSet(right, HU3D_MOTATTR_LOOP);
            {
                M659Player *startWork = obj->data;
                M659Motion *startMotion = &startWork->motion;
                HU3D_MODELID model = obj->mdlId[1];
                /* Captured here but not used by this block. */
                HU3D_MODELID endingEffectModel = obj->mdlId[4];
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
        Hu3DModelPosSetV(model, &playerData->motion.start);
    }
    if (motion->done) {
        HU3D_MODELID model = obj->mdlId[1];
        HU3D_MODELID effect = obj->mdlId[4];
        fn_1_2518(obj);
        playerData->motion.start.y += 8.0f;
        playerData->motion.start.z -= 8.0f;
        playerData->motion.rotation.x -= 20.0f;
        Hu3DModelPosSetV(model, &playerData->motion.start);
        Hu3DModelRotSetV(model, &playerData->motion.rotation);
        Hu3DModelPosSetV(effect, &playerData->motion.position);
    }
}

/* The result transition installs this callback for the winner to place its character on the finish
 * marker. */
void fn_1_2D0C(OMOBJ *obj)
{
    M659Player *playerData = obj->data;
    M659PlayerInfo *info = &playerData->info;
    M659Motion *motion = &playerData->motion;
    HU3D_MODELID charModel = obj->mdlId[0];
    HU3D_MODELID model = obj->mdlId[1];
    Mtx mtx;
    HuVecF pos, rot;
    switch (motion->substate) {
    case 0:
        motion->start.x = 0.0f;
        motion->start.y = 0.0f;
        motion->start.z = 0.0f;
        motion->rotation.x = 0.0f;
        motion->rotation.y = 90.0f;
        motion->rotation.z = 0.0f;
        motion->substate++;
        Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(lbl_1_bss_28, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_28, HU3D_MOTATTR_PAUSE);
        info->endingPosition.x = info->endingPosition.y = info->endingPosition.z = 0.0f;
        info->endingRotation.x = info->endingRotation.z = 0.0f;
        info->endingRotation.y = 180.0f;
        info->endingPosition.z = -100.0f;
        info->endingPosition.x = -50.0f;
        fn_1_220C(obj, 6);
        HuAudFXStop(playerData->info.engineSoundHandle);
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

/* The result transition installs this callback for the losing side to send its ship across the
 * scene. */
void fn_1_2FE0(OMOBJ *obj)
{
    M659Player *playerData = obj->data;
    M659Motion *motion = &playerData->motion;
    int verticalRange;
    HU3D_MODELID model = obj->mdlId[1];
    HuVecF end;
    switch (motion->substate) {
    case 0:
        if ((frand() & (1 << 0)) == 0) {
            motion->start.x = 3000.0f;
            end.x = -3000.0f;
        } else {
            motion->start.x = -3000.0f;
            end.x = 3000.0f;
        }
        end.z = motion->start.z = 4000.0f;
        verticalRange = 6000;
        motion->start.y = -3000.0f + (float)(frand() % verticalRange);
        end.y = -3000.0f + (float)(frand() % verticalRange);
        PSVECSubtract(&end, &playerData->motion.start, &motion->travelDirection);
        PSVECNormalize(&motion->travelDirection, &motion->travelDirection);
        fn_1_220C(obj, 6);
        motion->substate++;
        /* fall through */
    case 1:
        motion->start.x += 40.0f * motion->travelDirection.x;
        motion->start.y += 40.0f * motion->travelDirection.y;
        motion->rotation.x -= 20.0f;
        motion->rotation.y = 90.0f;
        break;
    }
    Hu3DModelPosSetV(model, &playerData->motion.start);
    Hu3DModelRotSetV(model, &playerData->motion.rotation);
}
