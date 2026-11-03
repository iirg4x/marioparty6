/* Player ships, character animations, lane input, and collision response for Asteroad Rage. */
#define M659_CHAR_FX_COURSE_CLEAR 577
#define M659_SFX_LANE_SIDE_0 2105
#define M659_SFX_LANE_SIDE_1 2106
#define M659_SFX_HIT_SIDE_0 2107
#define M659_SFX_HIT_SIDE_1 2108
#define M659_SFX_ENGINE_SIDE_0 2109
#define M659_SFX_ENGINE_SIDE_1 2110
#define M659_DATA_SHIP_MODEL DATANUM(0x760000, 6)
#define M659_DATA_SHIP_MOTION_0 DATANUM(0x760000, 7)
#define M659_DATA_SHIP_MOTION_1 DATANUM(0x760000, 8)
#define M659_DATA_SHIP_MOTION_2 DATANUM(0x760000, 9)
#define M659_DATA_BURNER_MODEL DATANUM(0x760000, 10)
#define M659_DATA_BURNER_MOTION_LEFT DATANUM(0x760000, 12)
#define M659_DATA_BURNER_MOTION_RIGHT DATANUM(0x760000, 11)
#define M659_DATA_ENDING_EFFECT_MODEL DATANUM(0x760000, 18)
#define M659_DATA_ENDING_EFFECT_MOTION DATANUM(0x760000, 19)
#define M659_DATA_CHARACTER_MOTION_0 DATANUM(0x8e0000, 32)
#define M659_DATA_CHARACTER_MOTION_1 DATANUM(0x8e0000, 127)
#define M659_DATA_CHARACTER_MOTION_2 DATANUM(0x8e0000, 11)
#define M659_DATA_CHARACTER_MOTION_3 DATANUM(0x8e0000, 4)
#include "REL/m659/player.h"
#include "game/charman.h"
#include "game/audio.h"

float lbl_1_data_70[8] = {1200, 800, 400, 0, -1200, -1200, 0, 0};
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
#include "game/pad.h"

extern float lbl_1_data_70[];
void fn_1_220C(OMOBJ *obj, s16 motion);
int fn_1_2384(s16 side);
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
    info->charNo = charNo = lbl_80265728[info->playerNo].charNo;
    info->initializationMarker = 1;
    model = obj->mdlId[0] = fn_80057068(charNo, 4);
    fn_80026B64(model, (u16)info->cameraBit);
    for (i = 0; i < 10; i++) {
        obj->mtnId[i] = fn_8005B0B8(charNo, lbl_1_data_D4[i]);
    }
    fn_800269E8(model, ((1 << 30) | (1 << 0)));
    fn_80026B8C(model, 7);
    fn_80026E44(model);
    fn_8005B4BC(charNo);
    fn_1_19DC(obj, 0);
    playerData->padNo = lbl_80265728[info->playerNo].padNo;
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
        attr = ((1 << 30) | (1 << 0));
    }
    if (playerData->info.motionId != motion) {
        fn_8005C2C8(charNo, obj->mtnId[motion], 0.0f, 8.0f, attr);
    } else {
        fn_8005BE80(charNo, obj->mtnId[motion]);
    }
    switch (motion) {
    case 0:
    case 3:
        fn_800269E8(model, ((1 << 30) | (1 << 0)));
        break;
    default:
        fn_80026A44(model, ((1 << 30) | (1 << 0)));
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
        fn_80025A04(fn_80008DD0(M659_DATA_SHIP_MODEL, 0x10000000, 2));
    for (i = 4; i < 7; i++) {
        motionId = obj->mtnId[i] = fn_80030458(
            model, fn_80008DD0(lbl_1_data_98[i - 4], 0x10000000, 2));
    }
    fn_8002C95C(model, motionId);
    fn_800269E8(model, ((1 << 30) | (1 << 0)));
    fn_8002D86C(model, 1.0f);
    fn_80026B8C(model, 7);
    fn_8002741C(model, lbl_1_data_130[lbl_1_data_FC[info->charNo]], charModel);
    fn_80026B64(model, (u16)info->cameraBit);
    {
        char *hooks[2] = {"ship-burnerhookL", "ship-burnerhookR"};
        s16 partMotion;
        HU3D_MODELID partModel;
        s16 part;
        HU3D_MOTIONID partMotionId;
        for (part = 2; part < 4; part++) {
            partModel = obj->mdlId[part] = fn_80025A04(
                fn_80008DD0(M659_DATA_BURNER_MODEL, 0x10000000, 2));
            for (partMotion = 7; partMotion < 9; partMotion++) {
                partMotionId = obj->mtnId[partMotion] =
                    fn_80030458(partModel, fn_80008DD0(lbl_1_data_A4[partMotion - 7],
                                                                    0x10000000, 2));
            }
            fn_8002C95C(partModel, partMotionId);
            fn_800269E8(partModel, ((1 << 30) | (1 << 0)));
            fn_8002D86C(partModel, 1.0f);
            fn_80026B8C(partModel, 7);
            fn_8002741C(obj->mdlId[1], hooks[part - 2], partModel);
            fn_80026B64(partModel, (u16)info->cameraBit);
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
                fn_80026790(model, &playerData->motion.start);
                fn_1_220C(obj, 6);
                motion->direction = -1;
                motion->substate = 0;
                motion->state = 1;
            }
            break;
        case 1:
            motion->position.x = lbl_1_data_70[motion->column];
            motion->start = motion->position;
            fn_80026790(model, &playerData->motion.start);
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
            u16 buttons = lbl_802C0008[playerData->padNo] &
                          (0x4000 | 0x2000);
            motion->direction = -1;
            /* Simultaneous presses of both lane buttons are discarded. */
            if (buttons != (0x4000 | 0x2000)) {
                if (buttons & 0x4000) {
                    motion->direction = 0;
                }
                if (buttons & 0x2000) {
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
        fn_8002C95C(model, obj->mtnId[motion]);
        fn_800269E8(model, ((1 << 30) | (1 << 0)));
        fn_8002C95C(left, obj->mtnId[7]);
        fn_800269E8(left, ((1 << 30) | (1 << 0)));
        fn_8002C95C(right, obj->mtnId[7]);
        fn_800269E8(right, ((1 << 30) | (1 << 0)));
    } else {
        fn_8002C95C(model, obj->mtnId[motion]);
        fn_80026A44(model, ((1 << 30) | (1 << 0)));
        fn_8002C95C(left, obj->mtnId[8]);
        fn_800269E8(left, ((1 << 30) | (1 << 0)));
        fn_8002C95C(right, obj->mtnId[8]);
        fn_800269E8(right, ((1 << 30) | (1 << 0)));
    }
    fn_8002D86C(model, 1.5f);
    fn_8002D86C(left, 1.0f);
    fn_8002D86C(right, 1.0f);
}

/* The pre-start movement callback calls this to detect the end of a side's ship animation. */
int fn_1_2384(s16 side)
{
    OMOBJ *obj = lbl_1_bss_2C[side];
    HU3D_MODELID model = obj->mdlId[1];
    if (fn_8002D75C(model)) {
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
    model = obj->mdlId[4] = fn_80025A04(
        fn_80008DD0(M659_DATA_ENDING_EFFECT_MODEL, 0x10000000, 2));
    motion = obj->mtnId[9] = fn_80030458(
        model, fn_80008DD0(M659_DATA_ENDING_EFFECT_MOTION, 0x10000000, 2));
    fn_8002C95C(model, motion);
    fn_800269E8(model, (1 << 0));
    fn_8002D86C(model, 0.0f);
    fn_80026B8C(model, 7);
    fn_80026B64(model, (u16)info->cameraBit);
}

/* The ship collision callback calls this to reveal and start the end effect. */
void fn_1_24C0(OMOBJ *obj)
{
    HU3D_MODELID model = obj->mdlId[4];
    fn_80026A44(model, (1 << 0));
    fn_8002D86C(model, 1.0f);
}

/* The per-frame player callback calls this to hide the end effect when its motion ends. */
int fn_1_2518(OMOBJ *obj)
{
    HU3D_MODELID model = obj->mdlId[4];
    if (fn_8002D75C(model)) {
        fn_800269E8(model, (1 << 0));
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
    fn_8006A030(playerData->info.playerNo, 20, 20, 0);
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
        if (fn_80071A8C() >= 5) {
            info->state++;
        }
        break;
    case 1:
        if (fn_80071A8C() > 5) {
            HU3D_MODELID left = obj->mdlId[2];
            HU3D_MODELID right = obj->mdlId[3];
            fn_8002C95C(left, obj->mtnId[7]);
            fn_800269E8(left, ((1 << 30) | (1 << 0)));
            fn_8002C95C(right, obj->mtnId[7]);
            fn_800269E8(right, ((1 << 30) | (1 << 0)));
            {
                M659Player *startWork = obj->data;
                M659Motion *startMotion = &startWork->motion;
                HU3D_MODELID model = obj->mdlId[1];
                /* Captured here but not used by this block. */
                HU3D_MODELID endingEffectModel = obj->mdlId[4];
                Hu3DModelObjPosGet(model, "ship-Ship", &startMotion->start);
                fn_80026790(model, &startWork->motion.start);
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
        fn_80026790(model, &playerData->motion.start);
    }
    if (motion->done) {
        HU3D_MODELID model = obj->mdlId[1];
        HU3D_MODELID effect = obj->mdlId[4];
        fn_1_2518(obj);
        playerData->motion.start.y += 8.0f;
        playerData->motion.start.z -= 8.0f;
        playerData->motion.rotation.x -= 20.0f;
        fn_80026790(model, &playerData->motion.start);
        fn_80026838(model, &playerData->motion.rotation);
        fn_80026790(effect, &playerData->motion.position);
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
        fn_80026760(model, 0.0f, 0.0f, 0.0f);
        fn_80026A44(lbl_1_bss_28, (1 << 0));
        fn_80026A44(model, (1 << 0));
        fn_80026A44(lbl_1_bss_28, ((1 << 30) | (1 << 1)));
        info->endingPosition.x = info->endingPosition.y = info->endingPosition.z = 0.0f;
        info->endingRotation.x = info->endingRotation.z = 0.0f;
        info->endingRotation.y = 180.0f;
        info->endingPosition.z = -100.0f;
        info->endingPosition.x = -50.0f;
        fn_1_220C(obj, 6);
        HuAudFXStop(playerData->info.engineSoundHandle);
        return;
    case 1:
        if (fn_8002D75C(charModel)) {
            fn_8002D414(charModel, 10.0f);
        }
        fn_80023398(lbl_1_bss_28, "ENDmot-grid1", mtx);
        Hu3DMtxTransGet(mtx, &pos);
        Hu3DMtxRotGet(mtx, &rot);
        fn_80026790(model, &pos);
        fn_80026838(model, &rot);
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
    fn_80026790(model, &playerData->motion.start);
    fn_80026838(model, &playerData->motion.rotation);
}
