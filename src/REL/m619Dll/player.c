/* Sets up character objects and updates their steering, movement and collision reactions. */
#include "REL/m619Dll/module_types.h"
#include "REL/m619Dll/player.h"
#include "msm_se.h"

#define M619_IMPACT_MOTION_START 0.0f
#define M619_IMPACT_MOTION_END 5.0f

s32 lbl_1_data_118[8] = {
    DATANUM(DATA_mario, 33), DATANUM(DATA_mariomot, 26),
    DATANUM(DATA_mariomot, 34), DATANUM(DATA_mario, 159),
    DATANUM(DATA_mariomot, 27), DATANUM(DATA_mariomot, 0),
    DATANUM(DATA_mariomot, 34), DATANUM(DATA_mariomot, 23)
};

s32 fn_1_A1F4(void *motionData);
void fn_1_A23C(OMOBJ *object);
void fn_1_A614(OMOBJ *object);
void fn_1_A674(M619Player *player);
void fn_1_A894(M619Player *player);
void fn_1_AEE4(M619Player *state);
void fn_1_B200(M619Player *state);
void fn_1_B3C0(M619Sway *setup, M619Player *player);
void fn_1_B3FC(M619Sway *motion, s32 expand);
void fn_1_B678(void *playerData);
s32 fn_1_BADC(void *player);
void fn_1_BB4C(M619MotionNode *object, M619MotionNode *event,
               f32 impactStrength);
void fn_1_C194(M619Player *player);
void fn_1_C1E8(M619Player *player, f32 y);
s32 fn_1_C324(void *playerData);
s32 fn_1_C3B4(M619Player *player);
s32 fn_1_C420(M619Player *playerState);
void fn_1_C628(M619Player *player);
void fn_1_C690(s32 *responseState);

extern void fn_1_5780(M619ModelQueueNode *node);
extern void fn_1_51EC(M619ModelQueueNode *node, const HuVecF *position);
extern void fn_1_55BC(M619ModelQueueNode *node, f32 scale, s32 mode);
extern void fn_1_792C(M619PlayerMotionPayload *recipient, M619Player *source);
extern void fn_1_9EEC(M619ModelMotion *modelMotion, s32 variant);
extern void fn_1_C69C(M619ParticleEffect *effect, const HuVecF *position, const HuVecF *rotation);
extern void fn_1_5794(HuVecF *position);

static inline f32 M619ImpactMagnitude(f32 angle) {
    return (f32)fabs(angle);
}

static inline f32 M619ImpactAtan2F(f32 y, f32 x) {
    return (f32)atan2(y, x);
}

static inline f32 M619SteeringAngleMagnitude(f32 difference) {
    f64 magnitude = __fabs(difference);
    return (f32)magnitude;
}

/* Queued by fn_1_A124 to report when the model motion has finished. */
s32 fn_1_A1F4(void *motionData) {
    M619ModelMotion *motionState;

    motionState = motionData;
    if (Hu3DMotionEndCheck(motionState->model) != 0) {
        return 1;
    }
    return 0;
}

/* Runs when a player object is created, loading its character and initializing its effects. */
void fn_1_A23C(OMOBJ *object) {
    M619Player *player;
    M619Player **playerSlots;
    s32 index;
    HU3D_MOTIONID motion;

    player = (M619Player *)object->data;
    playerSlots = (M619Player **)lbl_1_bss_0->data;
    index = 0;
    while (index < 4) {
        if (playerSlots[index] == NULL) {
            playerSlots[index] = player;
            break;
        }
        index++;
    }
    player->object = object;
    player->playerNumber = object->work[0];
    player->character = GwPlayerConf[player->playerNumber].charNo;
    player->group = GwPlayerConf[player->playerNumber].grpNo;
    player->pad = GwPlayerConf[player->playerNumber].padNo;
    player->playerType = GwPlayerConf[player->playerNumber].type;
    player->difficulty =
        GwPlayerConf[player->playerNumber].comDif;
    *object->mdlId = CharModelCreate(player->character, 4);
    player->characterModel = *object->mdlId;
    for (index = 0; index < 8; index++) {
        motion = CharMotionCreate(player->character,
                                  lbl_1_data_118[index]);
        player->motions[index] = motion;
        object->mtnId[index] = motion;
    }
    CharMotionSet(player->character, object->mtnId[5]);
    CharModelAttrReset(player->character, HU3D_MOTATTR_PAUSE);
    CharModelAttrSet(player->character, HU3D_MOTATTR_LOOP);
    CharModelAttrReset(player->character, HU3D_ATTR_DISPOFF);
    CharMotionDataClose(player->character);
    CharModelVoiceFlagSet(player->character, 0);
    Hu3DModelLayerSet(player->characterModel, 3);
    fn_1_B3C0((M619Sway *)&player->sway,
              player);
    player->phase = 4;
    player->enabled = 1;
    player->inputEnabled = 0;
    player->position.x = 0.0f;
    player->position.y = 0.0f;
    player->position.z = 0.0f;
    player->rotation.x = 0.0f;
    player->rotation.y = 0.0f;
    player->rotation.z = 0.0f;
    player->turnFrames = 0;
    player->movementActive = 0;
    player->phaseFrame = 0;
    player->flashCounter = 0;
    player->excluded = 0;
    player->movementFinished = 0;
    Hu3DModelPosSetV(player->characterModel, &player->position);
    omSetTra(player->object, player->position.x,
             player->position.y, player->position.z);
    Hu3DModelRotSetV(player->characterModel, &player->rotation);
    omSetRot(player->object, player->rotation.x,
             player->rotation.y, player->rotation.z);
    player->hookNameA =
        CharModelItemHookGet(player->character, 4, 0);
    player->hookNameB =
        CharModelItemHookGet(player->character, 4, 1);
    player->positionA.x = 0.0f;
    player->positionA.y = 0.0f;
    player->positionA.z = 0.0f;
    player->positionB.x = 0.0f;
    player->positionB.y = 0.0f;
    player->positionB.z = 0.0f;
    player->crossingStateA = 0;
    player->crossingStateB = 0;
    fn_1_C69C(&player->flashA, &player->positionA,
              &player->rotation);
    fn_1_C69C(&player->flashB, &player->positionB,
              &player->rotation);
    fn_1_55BC(&player->modelQueue, 1.5f, 0);
    fn_1_51EC(&player->modelQueue, &player->position);
    fn_1_792C(&player->motionPayload, player);
    fn_1_C690(&player->aimResponse.state);
    fn_1_9EEC(&player->modelMotion, 0);
    object->objFunc = fn_1_A614;
}

/* Runs each player-object update and dispatches input for enabled human or CPU players. */
void fn_1_A614(OMOBJ *object) {
    M619Player *player;

    player = (M619Player *)object->data;
    if (player->inputEnabled != 0) {
        if (player->playerType != 0) {
            fn_1_A894(player);
            return;
        }
        fn_1_A674((M619Player *)player);
    }
}

/* Reads human pad input while the player can steer, updating heading and turn timing. */
void fn_1_A674(M619Player *player) {
    f32 stickX;
    f32 stickY;
    f32 absoluteStickY;
    f32 absoluteStickX;
    f32 magnitude;
    f32 angle;
    f32 computedAngle;

    stickX = (s8)HuPadStkX[player->pad];
    stickY = -(s8)HuPadStkY[player->pad];
    if (stickY < 0.0f) {
        absoluteStickY = -stickY;
    } else {
        absoluteStickY = stickY;
    }
    if (stickX < 0.0f) {
        absoluteStickX = -stickX;
    } else {
        absoluteStickX = stickX;
    }
    magnitude = absoluteStickX + absoluteStickY;
    if (magnitude > 32.0f) {
        if (0.0f == stickY) {
            if (stickX >= 0.0f) {
                angle = 1.5707964f;
            } else {
                angle = -1.5707964f;
            }
        } else {
            computedAngle = (f32)atan2((f64)stickX, (f64)stickY);
            angle = computedAngle;
        }
        player->targetAngle =
            (f32)((f64)(180.0f * angle) / 3.141592653589793);
    }
    if ((HuPadBtnDown[player->pad] & 0x100) != 0) {
        player->turnFrames = 60;
    }
}

/* Updates a CPU player's chosen heading and turn timing during enabled input frames. */
void fn_1_A894(M619Player *player) {
    M619PlayerMotionPayload * aimCache;
    f32 directionX;
    f32 directionZ;
    f32 headingResult;
    f32 angle;
    f32 responseAngle;
    f32 refreshAngle;
    f32 angleResult;
    f32 targetAngleRadians;
    M619AimResponse * response;
    /* Chance per update of requesting a turn, indexed by CPU difficulty. */
    s32 difficultyLimits[4] = { 5, 8, 60, 100 };
    f32 refreshAngleResult;
    aimCache = &player->motionPayload;
    response = &player->aimResponse;
    switch (response->state)
    {
    case 0 :
        directionX = player->position.x;
        directionZ = player->position.z;
        if (directionZ == 0.0f)
        {
            if (directionX >= 0.0f)
            {
                angle = 1.5707964f;
            }
            else
            {
                angle = - 1.5707964f;
            }
        }
        else
        {
            angleResult = (f32) atan2((f64) directionX, (f64) directionZ);
            angle = angleResult;
        }
        headingResult =
            (f32)((((f64)(180.0f * angle) / 3.141592653589793) +
                    (f64)(160.0f * ((f32)frandmod(10001) / 10000.0f))) -
                  80.0);
        response->randomHeading = headingResult;
        response->updateLimit = frandmod(60) + 60;
        response->updates = 0;
        response->frame = 0;
        response->frameLimit = frandmod(120) + 600;
        response->state++;
        /* Apply the new random heading in this same update. */
    case 1 :
        aimCache->position.x = player->position.x;
        aimCache->position.z = player->position.z;
        if (aimCache->usesPlayerHeading != 0)
        {
            f32 unusedDirectionAngle;
            directionX = player->position.x;
            directionZ = player->position.z;
            if (0.0f == directionZ)
            {
                if (directionX >= 0.0f)
                {
                    responseAngle = 1.5707964f;
                }
                else
                {
                    responseAngle = - 1.5707964f;
                }
            }
            else
            {
                targetAngleRadians = (f32) atan2((f64) directionX, (f64) directionZ);
                responseAngle = targetAngleRadians;
            }
            player->targetAngle =
                (f32)(((f64)(180.0f * responseAngle) / 3.141592653589793) - 10.0);
            response->state++;
        }
        else
        {
            player->targetAngle = response->randomHeading;
            response->frame++;
            if (response->frame >= response->frameLimit)
            {
                response->state++;
            }
            else
            {
                response->updates++;
                if (response->updates > response->updateLimit)
                {
                    directionX = player->position.x;
                    directionZ = player->position.z;
                    if (directionZ == 0.0f)
                    {
                        if (directionX >= 0.0f)
                        {
                            refreshAngle = 1.5707964f;
                        }
                        else
                        {
                            refreshAngle = - 1.5707964f;
                        }
                    }
                    else
                    {
                        f32 refreshedHeadingRadians =
                            (f32) atan2((f64) directionX, (f64) directionZ);
                        refreshAngle = refreshedHeadingRadians;
                    }
                    headingResult = (f32) ((((f64) (180.0f * refreshAngle) / 3.141592653589793) +
                                             (f64) (160.0f * ((f32) frandmod(10001) / 10000.0f))) -
                                           80.0);
                    response->randomHeading = headingResult;
                     response->updateLimit = frandmod(60) + 60;
                    response->updates = 0;
                }
            }
        }
        if (frandmod(100) < difficultyLimits[player->difficulty])
        {
            player->turnFrames = 60;
        }
        break;
    case 2 :
        if (0 != aimCache->usesPlayerHeading)
        {
            f32 responseDirectionX;
            directionX = player->position.x;
            directionZ = player->position.z;
            if (directionZ == 0.0f)
            {
                if (directionX >= 0.0f)
                {
                    responseDirectionX = 1.5707964f;
                }
                else
                {
                    responseDirectionX = - 1.5707964f;
                }
            }
            else
            {
                f32 currentHeadingRadians;
                currentHeadingRadians = (f32) atan2((f64) directionX, (f64) directionZ);
                responseDirectionX = currentHeadingRadians;
            }
            headingResult = (f32)(((f64)(180.0f * responseDirectionX) / 3.141592653589793) - 10.0);
        }
        else
        {
            f32 responseDirectionX;
            directionX = aimCache->position.x - player->position.x;
            directionZ = aimCache->position.z - player->position.z;
            if (directionZ == 0.0f)
            {
                if (directionX >= 0.0f)
                {
                    responseDirectionX = 1.5707964f;
                }
                else
                {
                    responseDirectionX = - 1.5707964f;
                }
            }
            else
            {
                f32 relativeHeadingRadians = (f32) atan2((f64) directionX, (f64) directionZ);
                responseDirectionX = relativeHeadingRadians;
            }
            headingResult = (f32)((f64)(180.0f * responseDirectionX) / 3.141592653589793);
        }
        player->targetAngle = headingResult;
        if (frandmod(100) < difficultyLimits[player->difficulty])
        {
            player->turnFrames = 60;
        }
        break;
    }
}

/* Called by fn_1_B678 before collision checks to advance phases and track player movement. */
void fn_1_AEE4(M619Player *state) {
    f32 pulse;

    switch (state->phase) {
        case 0:
        case 1:
        if (state->turnFrames != 0) {
            state->turnFrames--;
            pulse = (f32)state->turnFrames / 60.0f;
            state->position.x = (f32)((f64)state->position.x +
                ((f64)(5.0f * pulse * pulse) *
                 sin((3.141592653589793 * (f64)state->rotation.y) /
                     180.0)));
            state->position.z = (f32)((f64)state->position.z +
                ((f64)(5.0f * pulse * pulse) *
                 cos((3.141592653589793 * (f64)state->rotation.y) /
                     180.0)));
            if (state->motions[1] != Hu3DMotionIDGet(state->characterModel) &&
                state->motions[1] != Hu3DMotionShiftIDGet(state->characterModel)) {
                CharMotionShiftSet(state->character, state->motions[1],
                                   0.0f, 15.0f,
                                   HU3D_MOTATTR_LOOP);
                CharMotionSpeedSet(state->character, 1.0f);
                state->phase = 1;
            }
        } else {
            if (state->motions[0] != Hu3DMotionIDGet(state->characterModel) &&
                state->motions[0] != Hu3DMotionShiftIDGet(state->characterModel)) {
                CharMotionShiftSet(state->character, state->motions[0],
                                   0.0f, 15.0f,
                                   HU3D_MOTATTR_LOOP);
                CharMotionSpeedSet(state->character, 1.0f);
                state->phase = 0;
            }
            if (state->movementActive != 0) {
                state->targetAngle =
                    state->targetAngle - 0.6f;
            }
        }
        break;
        case 2:
        CharMotionSpeedSet(state->character, 0.3f);
        state->phaseFrame++;
        if (state->phaseFrame >= 110) {
            fn_1_E5B8(fn_1_C324, state);
            CharMotionShiftSet(state->character, state->motions[0],
                               0.0f, 15.0f,
                               HU3D_MOTATTR_LOOP);
            CharMotionSpeedSet(state->character, 1.0f);
            state->phase = 0;
            if (state->movementActive != 0) {
                state->inputEnabled = 1;
            }
        }
        break;
        case 4:
        default:
            break;
    }
    if (state->movementActive != 0) {
        fn_1_5794(&state->position);
    }
}

/* fn_1_B678 calls this before collision checks to turn up to three degrees per frame. */
void fn_1_B200(M619Player *state) {
    f32 angleDelta;

    while (state->targetAngle > 180.0f) {
        state->targetAngle -= 360.0f;
    }
    while (state->targetAngle < -180.0f) {
        state->targetAngle += 360.0f;
    }
    while (state->rotation.y > 180.0f) {
        state->rotation.y -= 360.0f;
    }
    while (state->rotation.y < -180.0f) {
        state->rotation.y += 360.0f;
    }
    angleDelta = state->targetAngle - state->rotation.y;
    while (angleDelta > 180.0f) {
        angleDelta -= 360.0f;
    }
    while (angleDelta < -180.0f) {
        angleDelta += 360.0f;
    }
    if (angleDelta > 0.0f) {
        if (angleDelta > 3.0f) {
            angleDelta = 3.0f;
        }
    } else if (angleDelta < 0.0f &&
               angleDelta < -3.0f) {
        angleDelta = -3.0f;
    }
    state->rotation.y += angleDelta;
}

/* fn_1_A23C links the sway state to player transforms during object setup. */
void fn_1_B3C0(M619Sway *setup, M619Player *player) {
    setup->position = &player->position;
    setup->rotation = &player->rotation;
    setup->rotationY = 0.0f;
    setup->amount = 0.0f;
    setup->model = -1;
}

/* fn_1_B678 calls this each enabled update to animate the linked sway model. */
void fn_1_B3FC(M619Sway *motion, s32 expand) {
    f32 scale;
    f32 strength;
    f32 horizontalOffset;
    f32 depthOffset;

    motion->rotationY -= 0.6f;
    if (motion->rotationY < -180.0f) {
        motion->rotationY += 360.0f;
    }
    Hu3DModelRotSet(motion->model, 0.0f, motion->rotationY,
                    0.0f);
    if (expand != 0) {
        motion->amount += 1.0f;
        if (motion->amount > 15.0f) {
            motion->amount = 15.0f;
        }
    } else {
        motion->amount -= 1.0f;
        if (motion->amount < 0.0f) {
            motion->amount = 0.0f;
        }
    }
    strength = motion->amount / 15.0f;
    scale = 0.85f + (0.15f * strength);
    Hu3DModelScaleSet(motion->model, scale, scale, scale);
    scale = 0.5f + (0.5f * strength);
    Hu3DMotionSpeedSet(motion->model, scale);
    scale = 15.0f * strength;
    horizontalOffset = (f32)((f64)scale *
                             sin((3.141592653589793 *
                                  (f64)motion->rotation->y) /
                                 180.0));
    depthOffset = (f32)((f64)scale *
                        cos((3.141592653589793 *
                             (f64)motion->rotation->y) /
                            180.0));
    Hu3DModelPosSet(motion->model, motion->position->x + horizontalOffset,
                    motion->position->y,
                    motion->position->z + depthOffset);
}

/* Registered as the player's pre-collision update to steer it and animate its sway. */
void fn_1_B678(void *playerData) {
    M619Player *state;
    s32 isMoving;

    state = playerData;
    if (state->enabled != 0) {
        fn_1_B200(state);
        fn_1_AEE4((M619Player *)state);
        isMoving = state->phase == 1;
        fn_1_B3FC(&state->sway, isMoving);
    }
}

/* fn_1_E070 calls this after collision resolution to sync model and object transforms. */
s32 fn_1_BADC(void *player) {
    M619Player *playerState;

    playerState = player;
    Hu3DModelPosSetV(playerState->characterModel, &playerState->position);
    omSetTra(playerState->object, playerState->position.x, playerState->position.y,
             playerState->position.z);
    Hu3DModelRotSetV(playerState->characterModel, &playerState->rotation);
    omSetRot(playerState->object, playerState->rotation.x, playerState->rotation.y,
             playerState->rotation.z);
    return 0;
}

/* fn_1_E070 calls this for overlapping nodes to adjust motion and start hit reactions. */
void fn_1_BB4C(M619MotionNode *object, M619MotionNode *event,
               f32 impactStrength) {
    M619Player *objectPlayer;
    M619Player *eventPlayer;
    f32 eventKindOneDeltaX;
    f32 eventKindOneDeltaZ;
    f32 eventKindOneAngle;
    f32 angleDifference;
    f32 angleForTrig;
    f32 turnDeltaX;
    f32 turnDeltaZ;
    f32 turnAngle;
    f32 driftDeltaX;
    f32 driftDeltaZ;
    f32 driftAngle;
    f32 eventKindThreeDeltaX;
    f32 eventKindThreeDeltaZ;
    f32 eventKindThreeAngle;

    objectPlayer = object->owner;
    switch (event->kind) {
    case 2:
    case 4:
        break;
    case 1:
        omVibrate(objectPlayer->playerNumber, 20, 7, 3);
        eventKindOneDeltaZ = event->position->z - object->position->z;
        eventKindOneDeltaX = event->position->x - object->position->x;
        if (eventKindOneDeltaZ == 0.0f) {
            if (eventKindOneDeltaX >= 0.0f) {
                eventKindOneAngle = 1.5707964f;
            } else {
                eventKindOneAngle = -1.5707964f;
            }
        } else {
            eventKindOneAngle = M619ImpactAtan2F(eventKindOneDeltaX, eventKindOneDeltaZ);
        }
        angleForTrig = eventKindOneAngle;
        object->deltaX = (f32)(object->deltaX -
            (impactStrength * sin((f64)angleForTrig)));
        object->deltaZ = (f32)(object->deltaZ -
            (impactStrength * cos((f64)angleForTrig)));
        break;
    case 0:
        eventPlayer = event->owner;
        angleDifference = objectPlayer->rotation.y - eventPlayer->rotation.y;
        while (angleDifference > 180.0f) {
            angleDifference -= 360.0f;
        }
        while (angleDifference < -180.0f) {
            angleDifference += 360.0f;
        }
        angleDifference = M619ImpactMagnitude(angleDifference);
        if (angleDifference >= 180.0f) {
            turnDeltaZ = event->position->z - object->position->z;
            turnDeltaX = event->position->x - object->position->x;
            if (turnDeltaZ == 0.0f) {
                if (turnDeltaX >= 0.0f) {
                    turnAngle = 1.5707964f;
                } else {
                    turnAngle = -1.5707964f;
                }
            } else {
                turnAngle = M619ImpactAtan2F(turnDeltaX, turnDeltaZ);
            }
            angleForTrig = turnAngle;
            object->deltaX = (f32)(object->deltaX -
                ((f64)(0.5f * impactStrength) *
                 sin(0.017453292519943295 + (f64)angleForTrig)));
            object->deltaZ = (f32)(object->deltaZ -
                ((f64)(0.5f * impactStrength) *
                 cos(0.017453292519943295 + (f64)angleForTrig)));
        } else {
            driftDeltaZ = event->position->z - object->position->z;
            driftDeltaX = event->position->x - object->position->x;
            if (driftDeltaZ == 0.0f) {
                if (driftDeltaX >= 0.0f) {
                    driftAngle = 1.5707964f;
                } else {
                    driftAngle = -1.5707964f;
                }
            } else {
                driftAngle = M619ImpactAtan2F(driftDeltaX, driftDeltaZ);
            }
            angleForTrig = driftAngle;
            object->deltaX = (f32)(object->deltaX -
                ((f64)(0.5f * (f32)impactStrength) * sin((f64)angleForTrig)));
            object->deltaZ = (f32)(object->deltaZ -
                ((f64)(0.5f * (f32)impactStrength) * cos((f64)angleForTrig)));
        }
        break;
    case 3:
        omVibrate(objectPlayer->playerNumber, 20, 7, 3);
        eventKindThreeDeltaZ = event->position->z - object->position->z;
        eventKindThreeDeltaX = event->position->x - object->position->x;
        if (eventKindThreeDeltaZ == 0.0f) {
            if (eventKindThreeDeltaX >= 0.0f) {
                eventKindThreeAngle = 1.5707964f;
            } else {
                eventKindThreeAngle = -1.5707964f;
            }
        } else {
            eventKindThreeAngle = M619ImpactAtan2F(eventKindThreeDeltaX, eventKindThreeDeltaZ);
        }
        angleForTrig = eventKindThreeAngle;
        object->deltaX = (f32)(object->deltaX -
            ((f64)(0.5f * impactStrength) * sin((f64)angleForTrig)));
        object->deltaZ = (f32)(object->deltaZ -
            ((f64)(0.5f * impactStrength) * cos((f64)angleForTrig)));
        break;
    }

    if (((event->kind == 3) || (event->kind == 1)) &&
        (objectPlayer->phase != 2) && (objectPlayer->flashCounter == 0)) {
        CharFXPlay(objectPlayer->character, MSM_SE_CHARVOICE_MARIO + 9);
        CharMotionShiftSet(objectPlayer->character, objectPlayer->motions[3],
                           M619_IMPACT_MOTION_START, M619_IMPACT_MOTION_END,
                           HU3D_MOTATTR_LOOP);
        objectPlayer->phase = 2;
        objectPlayer->inputEnabled = 0;
        objectPlayer->turnFrames = 0;
        objectPlayer->phaseFrame = 0;
    }
}

/* Scene bounds handling excludes the player, disables input, and requests rumble. */
void fn_1_C194(M619Player *player) {
    player->inputEnabled = 0;
    player->excluded = 1;
    player->turnFrames = 0;
    omVibrate(player->playerNumber, 60, 20, 0);
}

/* Scene transition phase 4 starts the impact sequence at its animation heading. */
void fn_1_C1E8(M619Player *player, f32 y) {
    player->enabled = 0;
    player->flashCounter = 30;
    fn_1_E4A4(&player->collision);
        Hu3DModelAttrSet(player->sway.model, HU3D_ATTR_DISPOFF);
    fn_1_5780(&player->modelQueue);
    CharMotionShiftSet(player->character, player->motions[2], 0.0f, 10.0f, HU3D_MOTATTR_LOOP);
    player->position.x = 0.0f;
    player->position.y = 0.0f;
    player->position.z = 0.0f;
    player->rotation.x = 0.0f;
    player->rotation.y = y;
    player->rotation.z = 0.0f;
    player->targetAngle = y;
    player->movementActive = 0;
    player->phase = 3;
    Hu3DModelPosSetV(player->characterModel, &player->position);
    omSetTra(player->object, player->position.x, player->position.y, player->position.z);
    Hu3DModelRotSetV(player->characterModel, &player->rotation);
    omSetRot(player->object, player->rotation.x, player->rotation.y, player->rotation.z);
}

/* fn_1_AEE4 queues this after the hit animation to blink the character for 30 updates. */
s32 fn_1_C324(void *playerData) {
    M619Player *playerState;

    playerState = playerData;
    playerState->flashCounter = (s32) (playerState->flashCounter + 1);
    if ((s32) (playerState->flashCounter & 1) != 0) {
        Hu3DModelAttrSet(playerState->characterModel, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(playerState->characterModel, HU3D_ATTR_DISPOFF);
    }
    if ((s32) playerState->flashCounter >= 30) {
        playerState->flashCounter = 0;
        Hu3DModelAttrReset(playerState->characterModel, HU3D_ATTR_DISPOFF);
        return 1;
    }
    return 0;
}

/* Queued during scene exit to seek the finished final motion back to time 35. */
s32 fn_1_C3B4(M619Player *player) {
    M619Player *state;

    state = player;
    if ((Hu3DMotionEndCheck(state->characterModel) != 0) &&
        (state->motions[4] == Hu3DMotionIDGet(state->characterModel))) {
        Hu3DMotionTimeSet(state->characterModel, 35.0f);
    }
    return 0;
}

/* Scene movement callbacks use this to steer a player and finish within ten XZ units. */
s32 fn_1_C420(M619Player *playerState) {
    M619Player *player;
    s32 result;
    f32 deltaX;
    f32 deltaZ;
    f32 angle;
    f32 directionAngle;
    f32 distanceSquared;
    f32 angleDifference;
    f32 computedAngle;

    result = 0;
    player = playerState;
    deltaX = player->targetPosition.x - player->position.x;
    deltaZ = player->targetPosition.z - player->position.z;
    if (0.0f == deltaZ) {
        if (deltaX >= 0.0f) {
            angle = 1.5707964f;
        } else {
            angle = -1.5707964f;
        }
    } else {
        computedAngle = (f32)atan2(deltaX, deltaZ);
        angle = computedAngle;
    }
    directionAngle = angle;
    distanceSquared = (deltaX * deltaX) + (deltaZ * deltaZ);
    if (distanceSquared > 100.0f) {
        player->targetAngle =
            (f32)((f64)(180.0f * directionAngle) /
                  3.141592653589793);
        angleDifference = player->rotation.y - player->targetAngle;
        if (M619SteeringAngleMagnitude(angleDifference) > 3.0f) {
            player->turnFrames = 0;
        } else {
            player->turnFrames = 60;
        }
    } else {
        player->targetAngle = 0.0f;
        player->movementFinished = 1;
        player->turnFrames = 0;
        result = 1;
    }
    return result;
}

/* Called by scene placement and drop updates to sync the character model and object transforms. */
void fn_1_C628(M619Player *player) {
    Hu3DModelPosSetV(player->characterModel, &player->position);
    omSetTra(player->object, player->position.x, player->position.y, player->position.z);
    Hu3DModelRotSetV(player->characterModel, &player->rotation);
    omSetRot(player->object, player->rotation.x, player->rotation.y, player->rotation.z);
}

void fn_1_C690(s32 *responseState)
{
    *responseState = 0;
}
