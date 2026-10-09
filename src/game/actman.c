/* Manages minigame player and actor movement, collision, animation, and reactions. */
#include "game/mg/actman.h"
#include "game/mg/colman.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/frand.h"
#include "game/audio.h"

#include "datanum/charmot.h"

#include "math.h"
#include "string.h"

#define ACTOR_MAX COLBODY_MAX
#define PLAYER_MAX GW_PLAYER_MAX
#define COLBODY_ATTR_COL_RESULT_MASK 0x34000000
#define COLBODY_ATTR_PRESERVED_MASK 0xFEFF0000
#define MGPLAYER_VIBATTR_HIPDROP_LAND (1 << 9)

typedef struct ActManWork_s {
    /* Player state indexed by collision-body number. */
    MGPLAYER *player;
    /* Actor state indexed by collision-body number. */
    MGACTOR *actor;
    /* Temporary actors attached to each player's hit or squish effect. */
    MGACTOR *effPlayer[4];
    /* Selects cylindrical rather than spherical collision-body positioning. */
    BOOL colCylF;
    /* Object manager that stores player motion objects and runs vibration callbacks. */
    OMOBJMAN *objman;
} ACTMAN_WORK;

static ACTMAN_WORK actmanWork;

static void PlayerModeWalk(MGPLAYER *playerP);
static void PlayerModeJump(MGPLAYER *playerP);
static void PlayerModeFall(MGPLAYER *playerP);
static void PlayerModePunch(MGPLAYER *playerP);
static void PlayerModeKick(MGPLAYER *playerP);
static void PlayerModeHipDrop(MGPLAYER *playerP);
static void PlayerModeHit(MGPLAYER *playerP);
static void PlayerModeKnockback(MGPLAYER *playerP);
static void PlayerModeSquish(MGPLAYER *playerP);
static void PlayerModeSquishHard(MGPLAYER *playerP);
static void PlayerModeDefault(MGPLAYER *playerP);
static void PlayerModeJumpAlt(MGPLAYER *playerP);

s16 _CharFXPlay(s16 charNo, s16 seNo, u8 voiceFlag);

static MGPLAYER_MODE_FUNC playerModeFunc[MGPLAYER_MODE_MAX] = {
    PlayerModeWalk, PlayerModeJump, PlayerModeFall, PlayerModePunch,
    PlayerModeKick, PlayerModeHipDrop, PlayerModeHit, PlayerModeKnockback,
    PlayerModeSquish, PlayerModeSquishHard, PlayerModeDefault, PlayerModeDefault,
    PlayerModeDefault, PlayerModeDefault, PlayerModeDefault, PlayerModeDefault,
    PlayerModeDefault, PlayerModeDefault
};

/* Normalizes the direction, using random X/Z and a negative Y fallback near zero.
 * The direction-valid flag is assigned but is not consumed. */
static void SafeNormalize(HuVecF *inputDirection, HuVecF *normalizedDirection)
{
    BOOL inputHasDirection;
    if(VECSquareMag(inputDirection) < 1e-6) {
        (normalizedDirection)->x = 0.01f*((u32)frandmod(20)-10.0f);
        (normalizedDirection)->z = 0.01f*((u32)frandmod(20)-10.0f);
        (normalizedDirection)->y = ((u32)frandmod(1) != 0) ? 0.01f : -0.01f;
        VECNormalize(normalizedDirection, normalizedDirection);
        inputHasDirection = FALSE;
    } else {
        VECNormalize(inputDirection, normalizedDirection);
        inputHasDirection = TRUE;
    }
}

/* Runs the player's requested vibration effects from its object-manager callback. */
static void ExecVibrate(OMOBJ *obj)
{
    MGPLAYER *player = (MGPLAYER *)obj->work[0];
    if(MgPlayerVibAttrCheck(player, MGPLAYER_VIBATTR_SQUISH_FAST)) {
        omVibrate(player->playerNo, 20, 7, 3);
    } else if(MgPlayerVibAttrCheck(player, MGPLAYER_VIBATTR_SQUISH)) {
        omVibrate(player->playerNo, 20, 7, 3);
    } else if(MgPlayerVibAttrCheck(player, MGPLAYER_VIBATTR_HEADJUMP)) {
        omVibrate(player->playerNo, 20, 7, 3);
    } else if(MgPlayerVibAttrCheck(player, MGPLAYER_VIBATTR_HIT_SRC)) {
        omVibrate(player->playerNo, 20, 7, 3);
    } else if(MgPlayerVibAttrCheck(player, MGPLAYER_VIBATTR_KNOCKBACK_SRC)) {
        omVibrate(player->playerNo, 20, 7, 3);
    } else if(MgPlayerVibAttrCheck(player, MGPLAYER_VIBATTR_HIPDROP_LAND)) {
        omVibrate(player->playerNo, 20, 7, 3);
    }
    switch(player->stunType) {
        case MGPLAYER_STUN_HIT:
            omVibrate(player->playerNo, 20, 7, 3);
            break;

        case MGPLAYER_STUN_KNOCKBACK:
            omVibrate(player->playerNo, 20, 7, 3);
            break;

        case MGPLAYER_STUN_SQUISH_HARD:
            omVibrate(player->playerNo, 20, 7, 3);
            break;
    }
    if(MgPlayerAttrCheck(player, MGPLAYER_ATTR_VIBKILL)) {
        MgPlayerAttrReset(player, MGPLAYER_ATTR_VIBKILL);
        obj->objFunc = NULL;
        omDelObj(actmanWork.objman, obj);
    }
}

/* Reports whether the COM stick override was already enabled, then enables it. */
BOOL MgPlayerComStkOn(MGPLAYER *playerP)
{
    BOOL wasEnabled = MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK) ? TRUE : FALSE;
    MgPlayerAttrSet(playerP, MGPLAYER_ATTR_COMSTK);
    return wasEnabled;
}

void MgPlayerComStkOff(MGPLAYER *playerP)
{
    MgPlayerAttrReset(playerP, MGPLAYER_ATTR_COMSTK);
}

/* Steers a player unless movement is disabled and reports arrival by horizontal distance. */
BOOL MgPlayerVecChase(MGPLAYER *playerP, HuVecF *targetPos, float forwardSpeed, float arrivalRadius)
{
    float radiusSquared = 0.001+(arrivalRadius*arrivalRadius);
    float angle;
    HuVecF dir;
    if(MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_MOVEOFF)) {
        return FALSE;
    }
    VECSubtract(targetPos, &playerP->actor->pos, &dir);
    dir.y = 0;
    if(VECSquareMag(&dir) < radiusSquared) {
        return TRUE;
    }
    SafeNormalize(&dir, &dir);
    if(dir.z > 1) {
        dir.z = 1;
    } else if(dir.z < -1) {
        dir.z = -1;
    }
    angle = acosf(dir.z);
    if(dir.x < 0) {
        angle = -angle;
    }
    playerP->actor->rotY = (180*angle)/M_PI;
    playerP->actor->push.x = playerP->actor->push.y = 0;
    playerP->actor->push.z = forwardSpeed;
    return FALSE;
}

/* Turns a free actor toward a target and reports when it reaches the radius. */
BOOL MgActorVecChase(MGACTOR *actorP, HuVecF *targetPos, float forwardSpeed, float arrivalRadius)
{
    float radiusSquared = 0.001+(arrivalRadius*arrivalRadius);
    float angle;
    HuVecF dir;
    VECSubtract(targetPos, &actorP->pos, &dir);
    dir.y = 0;
    if(VECSquareMag(&dir) < radiusSquared) {
        return TRUE;
    }
    SafeNormalize(&dir, &dir);
    if(dir.z > 1) {
        dir.z = 1;
    } else if(dir.z < -1) {
        dir.z = -1;
    }
    angle = acosf(dir.z);
    if(dir.x < 0) {
        angle = -angle;
    }
    actorP->rotY = (180*angle)/M_PI;
    actorP->push.x = actorP->push.y = 0;
    actorP->push.z = forwardSpeed;
    return FALSE;
}

/* Adds a persistent callback that requests rumble for vibration flags and selected stuns. */
void MgPlayerVibrateCreate(MGPLAYER *playerP)
{
    OMOBJ *obj = omAddObj(actmanWork.objman, 3000, 0, 0, ExecVibrate);
    obj->work[0] = (u32)playerP;
}

/* Marks the callback for removal after its next update's vibration checks. */
void MgPlayerVibrateKill(MGPLAYER *playerP)
{
    MgPlayerAttrSet(playerP, MGPLAYER_ATTR_VIBKILL);
}

static void KillEffectPlayer(int playerNo);

/* Changes mode unless unchanged or frozen: kick/hip drop require jump/fall,
 * jump/fall/punch require walking, and other target modes have no source-mode restriction. */
BOOL MgPlayerModeLandSet(MGPLAYER *playerP, int mode)
{
    if(playerP->mode == mode) {
        return FALSE;
    }
    if(playerP->stunType == MGPLAYER_STUN_FREEZE) {
        return FALSE;
    }
    switch(mode) {
        case MGPLAYER_MODE_KICK:
        case MGPLAYER_MODE_HIPDROP:
            if(playerP->mode != MGPLAYER_MODE_JUMP && playerP->mode != MGPLAYER_MODE_FALL) {
                return FALSE;
            }
            break;

        case MGPLAYER_MODE_JUMP:
        case MGPLAYER_MODE_FALL:
        case MGPLAYER_MODE_PUNCH:
            if(playerP->mode != MGPLAYER_MODE_WALK) {
                return FALSE;
            }
            break;
    }
    KillEffectPlayer(playerP->actor->no);
    playerP->mode = mode;
    playerP->subMode = 0;
    return TRUE;
}

/* Changes the player's mode unless it is unchanged or frozen by a stun. */
BOOL MgPlayerModeSet(MGPLAYER *playerP, int mode)
{
    if(playerP->mode == mode) {
        return FALSE;
    }
    if(playerP->stunType == MGPLAYER_STUN_FREEZE) {
        return FALSE;
    }
    KillEffectPlayer(playerP->actor->no);
    playerP->mode = mode;
    playerP->subMode = 0;
    return TRUE;
}

/* Folds an angle into the range [-180, 180] degrees. */
static float WrapAngle(float angle)
{
    angle = fmod(angle, 360);
    if(angle < -180) {
        angle += 360;
    } else if(angle > 180) {
        angle -= 360;
    }
    return angle;
}

/* Clears an actor slot and assigns its collision-body index and default physics. */
static inline MGACTOR *SetupActor(int actorNo)
{
    MGACTOR *actor = &actmanWork.actor[actorNo];
    memset(actor, 0, sizeof(MGACTOR));
    memset(&actor->oldPos, 0, sizeof(HuVecF));
    memset(&actor->push, 0, sizeof(HuVecF));
    memset(&actor->pos, 0, sizeof(HuVecF));
    memset(&actor->colNorm, 0, sizeof(HuVecF));
    memset(&actor->forceA, 0, sizeof(HuVecF));
    memset(&actor->forceB, 0, sizeof(HuVecF));
    memset(&actor->colOfs, 0, sizeof(HuVecF));
    memset(&actor->vel, 0, sizeof(HuVecF));
    actor->no = actorNo;
    actor->mdlId = 0;
    actor->param = 0;
    actor->type = 0;
    actor->rotY = 0;
    actor->gravity = 150;
    actor->velY = 0;
    actor->terminalVelY = 0;
    actor->colMesh = 0;
    actor->colObj = NULL;
    actor->colFace = 0;
    actor->colGroundAttr = 0;
    actor->correctHookParam = 0;
    actor->correctHook = NULL;
    return actor;
}

/* Builds the horizontal camera-relative basis used to interpret stick movement. */
static void GetStickMtx(Mtx stickMatrix, int cameraBit)
{
    HuVecF cameraPosition, cameraTarget, cameraUp;
    HuVecF rightBasis, upBasis, forwardBasis;
    HuVecF originalLook;
    HuVecF lookDirection;
    HuVecF cameraWorldPos, cameraWorldTarget, cameraWorldUp;
    Hu3DCameraPosGet(cameraBit, &cameraWorldPos, &cameraWorldUp, &cameraWorldTarget);
    cameraPosition.x = cameraWorldPos.x;
    cameraPosition.y = cameraWorldPos.y;
    cameraPosition.z = cameraWorldPos.z;
    cameraTarget.x = cameraWorldTarget.x;
    cameraTarget.y = cameraWorldTarget.y;
    cameraTarget.z = cameraWorldTarget.z;
    cameraUp.x = cameraWorldUp.x;
    cameraUp.y = cameraWorldUp.y;
    cameraUp.z = cameraWorldUp.z;
    rightBasis.x = 1;
    rightBasis.y = 0;
    rightBasis.z = 0;
    upBasis.x = 0;
    upBasis.y = 1;
    upBasis.z = 0;
    forwardBasis.x = 0;
    forwardBasis.y = 0;
    forwardBasis.z = 1;
    VECSubtract(&cameraTarget, &cameraPosition, &lookDirection);
    originalLook = lookDirection;
    originalLook.y = 0;
    /* For a nearly vertical view, use camera up as the horizontal direction and
     * negative view direction to choose the vertical basis. */
    if(VECSquareMag(&originalLook) < 0.001) {
        originalLook = lookDirection;
        lookDirection = cameraUp;
        VECScale(&originalLook, &cameraUp, -1);
    }
    lookDirection.y = 0;
    SafeNormalize(&lookDirection, &lookDirection);
    if(VECDotProduct(&upBasis, &cameraUp) < 0) {
        upBasis.y *= -1;
        forwardBasis.z *= -1;
    }
    VECCrossProduct(&lookDirection, &upBasis, &rightBasis);
    forwardBasis = lookDirection;
    stickMatrix[0][0] = rightBasis.x;
    stickMatrix[0][1] = rightBasis.y;
    stickMatrix[0][2] = rightBasis.z;
    stickMatrix[0][3] = 0;
    stickMatrix[1][0] = upBasis.x;
    stickMatrix[1][1] = upBasis.y;
    stickMatrix[1][2] = upBasis.z;
    stickMatrix[1][3] = 0;
    stickMatrix[2][0] = forwardBasis.x;
    stickMatrix[2][1] = forwardBasis.y;
    stickMatrix[2][2] = forwardBasis.z;
    stickMatrix[2][3] = 0;
}

/* Claims the first inactive player collision body, or returns -1 when full. */
static inline int AllocPlayer(void)
{
    int bodyIndex;
    for(bodyIndex=0; bodyIndex<PLAYER_MAX; bodyIndex++) {
        COLBODY *body = ColBodyGet(bodyIndex);
        if(!(body->param.attr & COLBODY_ATTR_ACTIVE)) {
            body->param.attr |= COLBODY_ATTR_ACTIVE;
            body->param.attr |= COLBODY_ATTR_RESET;
            break;
        }
    }
    if(bodyIndex == PLAYER_MAX) {
        return -1;
    }
    SetupActor(bodyIndex);
    return bodyIndex;
}

/* Creates player state, a motion-storage object with no callback, the character model,
 * and its initial idle motion. */
static MGPLAYER *CreatePlayer(s16 playerNo, s16 model, u16 camBit, u32 actionFlag,
                              unsigned int *motDataNum)
{
    int mdlId;
    int actorNo;
    if(!ColMapInitCheck()) {
        return NULL;
    }
    actorNo = AllocPlayer();
    if(actorNo < 0) {
        return NULL;
    } else {
        MGPLAYER *playerP;
        OMOBJ *obj;
        playerP = &actmanWork.player[actorNo];
        memset(playerP, 0, sizeof(MGPLAYER));
        obj = omAddObj(actmanWork.objman, playerNo, 1, 32, NULL);
        playerP->actor = &actmanWork.actor[actorNo];
        playerP->playerNo = playerNo;
        playerP->charNo = GwPlayerConf[playerNo].charNo;
        playerP->padNo = GwPlayerConf[playerNo].padNo;
        playerP->camBit = camBit;
        playerP->omObj = obj;
        playerP->actionFlag = actionFlag;
        playerP->motNo = 0;
        playerP->stunType = MGPLAYER_STUN_NONE;
        playerP->stunAngle = 0;
        playerP->stunTime = 0;
        playerP->squishTime = 0;
        playerP->attr = 0;
        playerP->modeAttr = 0;
        playerP->vibAttr = 0;
        playerP->subMode = 0;
        playerP->mode = 0;
        playerP->timer = 0;
        playerP->squishOldHeight = 0;
        playerP->work[0] = 0;
        playerP->work[1] = 0;
        playerP->work[2] = 0;
        memcpy(&playerP->modeFunc[0], &playerModeFunc[0], sizeof(playerModeFunc));
        mdlId = CharModelMotListCreate(playerP->charNo, model, motDataNum, obj->mtnId);
        playerP->actor->mdlId = mdlId;
        obj->work[0] = obj->work[1] = obj->work[2] = obj->work[3] = 0;
        Hu3DModelAttrSet(mdlId, HU3D_MOTATTR_LOOP);
        CharMotionSet(playerP->charNo, playerP->omObj->mtnId[0]);
        playerP->motNo = 0;
        Hu3DModelCameraSet(mdlId, camBit);
        return playerP;
    }
}

/* Claims an inactive non-player collision body and initializes its actor state. */
static inline MGACTOR *CreateActor(int mdlId)
{
    MGACTOR *actor;
    int bodyIndex;
    if(!ColMapInitCheck()) {
        return NULL;
    }
    for(bodyIndex=PLAYER_MAX; bodyIndex<ACTOR_MAX; bodyIndex++) {
        COLBODY *body = ColBodyGet(bodyIndex);
        if(!(body->param.attr & COLBODY_ATTR_ACTIVE)) {
            body->param.attr |= COLBODY_ATTR_ACTIVE;
            body->param.attr |= COLBODY_ATTR_RESET;
            break;
        }
    }
    if(bodyIndex == ACTOR_MAX) {
        return NULL;
    }
    actor = SetupActor(bodyIndex);
    actor->mdlId = mdlId;
    return actor;
}

/* Removes and clears the temporary effect actor owned by a player slot. */
static void KillEffectPlayer(int playerNo)
{
    if(actmanWork.effPlayer[playerNo]) {
        MgActorKill(actmanWork.effPlayer[playerNo]);
        actmanWork.effPlayer[playerNo] = NULL;
    }
}

#define GET_ACTOR_VELY(actorP, speed)                                                              \
    ((actorP->gravity)                                                                             \
         ? (actorP->gravity +                                                                      \
            (actorP->gravity * ((-1 + sqrtf(1 + (8 * (speed / actorP->gravity)))) / 2)))           \
         : 0)

/* Player narrow hook: handles body contacts and attack stuns. Attack cases return FALSE
 * even after applying a stun, so this hook does not request physical collision response. */
static int PlayerColHook(COL_NARROW_PARAM *a, COL_NARROW_PARAM *b)
{
    MGPLAYER *playerP1;
    MGACTOR *otherActor;

    playerP1 = &actmanWork.player[a->paramA];
    otherActor = &actmanWork.actor[b->paramA];
    switch(b->type) {
        case 0:
        case 1:
        {
            HuVecF dir;
            HuVecF up;
            MGPLAYER *playerP2;
            BOOL result;
            float forceZ;
            float dot;

            playerP2 = &actmanWork.player[b->paramA];
            result = TRUE;
            VECSubtract(&b->point, &a->point, &dir);
            SafeNormalize(&dir, &dir);
            up.x = 0;
            up.y = 1;
            up.z = 0;
            dot = VECDotProduct(&up, &dir);
            if(dot > cos(M_PI/3)) {
                if(MgPlayerModeAttrCheck(playerP2, MGPLAYER_MODEATTR_AIR)) {
                    if(fabsf(playerP1->actor->velY) <= 1000.0f) {
                        if (playerP2->actor->velY < 0 ||
                            MgPlayerVibAttrCheck(playerP2, MGPLAYER_VIBATTR_HEADJUMP)) {
                            if(MgPlayerStunSet(playerP1, MGPLAYER_STUN_SQUISH, 0, -1)) {
                                MgPlayerVibAttrSet(playerP1, MGPLAYER_VIBATTR_SQUISH);
                            }
                        }
                    } else {
                        MgPlayerVibAttrSet(playerP1, MGPLAYER_VIBATTR_SQUISH_FAST);
                    }
                }
            } else if(dot < -cos(M_PI/3)) {
                if(!MgPlayerAttrCheck(playerP1, MGPLAYER_ATTR_HEADJUMP_OFF)) {
                    HuVecF norm = a->normPos;
                    float velY;
                    Mtx rotMtx;

                    if(playerP1->actor->velY > 0) {
                        break;
                    }
                    if(!MgPlayerModeAttrCheck(playerP1, MGPLAYER_MODEATTR_AIR)) {
                        break;
                    }
                    norm.y = 0;
                    playerP1->actor->velY = GET_ACTOR_VELY(playerP1->actor, 15000);
                    playerP1->actor->rotY += (u32)frandmod(90)-45.0f;
                    if(!MgPlayerModeAttrCheck(playerP1, MGPLAYER_MODEATTR_HEADJUMP)) {
                        forceZ = VECMag(&norm)/2;
                    } else {
                        forceZ = VECMag(&playerP1->actor->forceB);
                    }
                    if(forceZ < 4) {
                        forceZ = 4;
                    }
                    playerP1->actor->forceB.x = playerP1->actor->forceB.y = 0;
                    playerP1->actor->forceB.z = forceZ;
                    MTXRotRad(rotMtx, 'Y', 0.017453292f*playerP1->actor->rotY);
                    MTXMultVec(rotMtx, &playerP1->actor->forceB, &playerP1->actor->forceB);
                    MgPlayerAttrSet(playerP1, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                    MgPlayerModeAttrSet(playerP1, MGPLAYER_MODEATTR_HEADJUMP);
                    MgPlayerVibAttrSet(playerP1, MGPLAYER_VIBATTR_HEADJUMP);

                } else {
                    HuVecF norm = dir;
                    norm.y = 0;
                    if(fabsf(VECSquareMag(&norm)) < 0.001) {
                        norm.x = 1;
                    } else {
                        SafeNormalize(&norm, &norm);
                    }
                    VECScale(&norm, &norm, -50);
                    a->normPos = norm;
                    result = FALSE;
                }
            }
            (void)playerP2;
            if(!result) {
                break;
            }
            if(MgPlayerVibAttrCheck(playerP1, 0x7)) {
                break;
            }
            {
                HuVecF temp;
                VECAdd(&a->normPos, &b->normPos, &temp);
                VECScale(&temp, &temp, 0.3f);
                temp.y = a->normPos.y;
                a->normPos = temp;
            }
        }
        break;

        case 2:
        {
            MGPLAYER *playerP2;
            HuVecF dir;
            float angle;

            playerP2 = &actmanWork.player[b->paramB & 0xFF];
            if(a->paramA == (b->paramB & 0xFF)) {
               return FALSE;
            }
            VECSubtract(&a->point, &b->point, &dir);
            SafeNormalize(&dir, &dir);
            if(dir.z > 1) {
                dir.z = 1;
            } else if(dir.z < -1) {
                dir.z = -1;
            }
            angle = acosf(dir.z);
            if(dir.x < 0) {
                angle = -angle;
            }
            angle = (180*angle)/M_PI;
            if(!MgPlayerStunSet(playerP1, MGPLAYER_STUN_HIT, angle, -1)) {
                return FALSE;
            }
            MgPlayerVibAttrSet(playerP2, MGPLAYER_VIBATTR_HIT_SRC);
            MgPlayerVibAttrSet(playerP1, MGPLAYER_VIBATTR_HIT);
            (void)playerP2;
            return FALSE;
        }
        break;

        case 3:
        {
            MGPLAYER *playerP2;
            playerP2 = &actmanWork.player[b->paramB & 0xFF];
            if(a->paramA == (b->paramB & 0xFF)) {
               return FALSE;
            }
            if (!MgPlayerStunSet(playerP1, MGPLAYER_STUN_KNOCKBACK,
                                 actmanWork.actor[b->paramA].rotY, -1)) {
                return FALSE;
            }
            MgPlayerVibAttrSet(playerP2, MGPLAYER_VIBATTR_KNOCKBACK_SRC);
            MgPlayerVibAttrSet(playerP1, MGPLAYER_VIBATTR_KNOCKBACK);
            (void)playerP2;
            return FALSE;
        }
        break;

        case 4:
        {
            MGPLAYER *playerP2;
            HuVecF dir;
            float angle;

            playerP2 = &actmanWork.player[b->paramB & 0xFF];
            if(a->paramA == (b->paramB & 0xFF)) {
               return FALSE;
            }
            VECSubtract(&a->point, &b->point, &dir);
            dir.y = 0;
            SafeNormalize(&dir, &dir);
            if(dir.z > 1) {
                dir.z = 1;
            } else if(dir.z < -1) {
                dir.z = -1;
            }
            angle = acosf(dir.z);
            if(dir.x < 0) {
                angle = -angle;
            }
            angle = (180*angle)/M_PI;
            if(!MgPlayerStunSet(playerP1, MGPLAYER_STUN_SQUISH_HARD, angle, -1)) {
                return FALSE;
            }
            MgPlayerVibAttrSet(playerP2, MGPLAYER_VIBATTR_SQUISH_HARD_SRC);
            MgPlayerVibAttrSet(playerP1, MGPLAYER_VIBATTR_SQUISH_HARD);
            _CharFXPlay(playerP1->charNo, CHARVOICEID(15), 0);
            (void)playerP2;
            return FALSE;
        }
        break;

        default:
            return FALSE;
    }
    return TRUE;
}

/* Correction hook installed by both create functions; saves contact data and runs the actor
 * hook. */
static void ActorColCorrectHook(COLBODY *body, void *user)
{
    MGACTOR *actorP;
    COLBODY_POINT *point;

    point = &body->colPoint[body->colPointNum];
    actorP = user;

    actorP->colGroundAttr = body->groundAttr;
    actorP->colNorm = point->normal;
    actorP->colMesh = point->meshNo;
    actorP->colObj = point->obj;
    actorP->colFace = point->faceNo;
    actorP->colOfs = point->colOfs;
    if(actorP->correctHook) {
        actorP->correctHook(actorP, actorP->correctHookParam);
    }
}

/* Clears actor-manager pointers and resets the shared collision map. */
void MgActorInit(void)
{
    int effectSlot;
    actmanWork.objman = NULL;
    actmanWork.player = NULL;
    actmanWork.actor = NULL;
    actmanWork.colCylF = FALSE;
    for(effectSlot=4; effectSlot--; ) {
        actmanWork.effPlayer[effectSlot] = NULL;
    }
    ColMapClear();
}

/* Frees actor and player arrays and shuts down the shared collision map. */
void MgActorClose(void)
{
    int effectSlot;
    if(actmanWork.player) {
        HuMemDirectFree(actmanWork.player);
    }
    if(actmanWork.actor) {
        HuMemDirectFree(actmanWork.actor);
    }
    actmanWork.player = NULL;
    actmanWork.actor = NULL;
    actmanWork.objman = NULL;
    actmanWork.colCylF = FALSE;
    for(effectSlot=4; effectSlot--; ) {
        actmanWork.effPlayer[effectSlot] = NULL;
    }
    ColMapKill();
}

/* Creates the object manager and allocates zeroed player and actor state arrays. */
OMOBJMAN *MgActorObjectSetup(void)
{
    actmanWork.objman = omInitObjMan(256, 1000);
    omGameSysInit(actmanWork.objman);
    actmanWork.player =
        HuMemDirectMallocNum(HEAP_MODEL, PLAYER_MAX * sizeof(MGPLAYER), HU_MEMNUM_OVL);
    actmanWork.actor = HuMemDirectMallocNum(HEAP_MODEL, ACTOR_MAX*sizeof(MGACTOR), HU_MEMNUM_OVL);
    memset(actmanWork.player, 0, PLAYER_MAX*sizeof(MGPLAYER));
    memset(actmanWork.actor, 0, ACTOR_MAX*sizeof(MGACTOR));
    actmanWork.colCylF = FALSE;
    return actmanWork.objman;
}

/* Supplies the collision map models and body limit during minigame setup. */
void MgActorColMapInit(HU3D_MODELID *mdlId, s16 mdlNum, int bodyMax)
{
    ColMapInit(mdlId, mdlNum, bodyMax);
}

/* Replaces this player's stick and button input while preserving Start and Z bits. */
void MgPlayerPadSet(MGPLAYER *playerP, int stkX, int stkY, int btnDown, int btn)
{
    HuPadStkX[playerP->padNo] = stkX;
    HuPadStkY[playerP->padNo] = stkY;
    HuPadBtnDown[playerP->padNo] &= (PAD_BUTTON_START|PAD_TRIGGER_Z);
    HuPadBtn[playerP->padNo] &= (PAD_BUTTON_START|PAD_TRIGGER_Z);
    HuPadBtnDown[playerP->padNo] |= btnDown;
    HuPadBtn[playerP->padNo] |= btn;
}

/* Adds collision-body attributes, preserving the manager's active and reset rules. */
void MgActorColAttrSet(MGACTOR *actorP, u32 mask)
{
    COLBODY *body = ColBodyGet(actorP->no);
    body->param.attr |= mask&(COLBODY_ATTR_RESET|0xFFFE);
}

/* Clears selected collision-body attributes while retaining protected manager bits. */
void MgActorColAttrReset(MGACTOR *actorP, u32 mask)
{
    COLBODY *body = ColBodyGet(actorP->no);
    body->param.attr &= (~mask|COLBODY_ATTR_PRESERVED_MASK|COLBODY_ATTR_ACTIVE);
}

/* Returns the selected attributes from this actor's collision body. */
u32 MgActorColAttrGet(MGACTOR *actorP, u32 mask)
{
    COLBODY *body = ColBodyGet(actorP->no);
    return body->param.attr & mask;
}

/* Installs only the non-null mode callbacks supplied by a minigame. */
void MgPlayerModeFuncSet(MGPLAYER *playerP, MGPLAYER_MODE_FUNC *funcTbl)
{
    int modeIndex;
    for(modeIndex=MGPLAYER_MODE_MAX; modeIndex--;) {
        if(funcTbl[modeIndex]) {
            playerP->modeFunc[modeIndex] = funcTbl[modeIndex];
        }
    }
}

static unsigned int defMotDataNum[MGPLAYER_MOT_NUM+1] = {
    CHARMOT_HSF_c000m1_300, CHARMOT_HSF_c000m1_301, CHARMOT_HSF_c000m1_302, CHARMOT_HSF_c000m1_303,
    CHARMOT_HSF_c000m1_464, CHARMOT_HSF_c000m1_305, CHARMOT_HSF_c000m1_367, CHARMOT_HSF_c000m1_308,
    CHARMOT_HSF_c000m1_310, CHARMOT_HSF_c000m1_309, CHARMOT_HSF_c000m1_315, CHARMOT_HSF_c000m1_316,
    CHARMOT_HSF_c000m1_317, CHARMOT_HSF_c000m1_318, CHARMOT_HSF_c000m1_322, CHARMOT_HSF_c000m1_322,
    0
};

/* Builds a player and collision body; a supplied motion list is adjusted in place. */
MGPLAYER *MgPlayerCreate(s16 playerNo, MGACTOR_PARAM *param, s16 model, u16 camBit, u32 actionFlag,
                         unsigned int *motDataNum)
{
    MGPLAYER *player;
    COLBODY_PARAM colParam;
    if(motDataNum) {
        s16 motionIndex;
        for(motionIndex=0; motDataNum[motionIndex]; motionIndex++) {
            if(motDataNum[motionIndex] == CHARMOT_HSF_c000m1_304) {
                /* Replace this motion ID in the caller's list before building the player. */
                motDataNum[motionIndex] = CHARMOT_HSF_c000m1_464;
            }
        }
        player = CreatePlayer(playerNo, model, camBit, actionFlag, motDataNum);
    } else {
        player = CreatePlayer(playerNo, model, camBit, actionFlag, defMotDataNum);
    }
    /* The result is used without a null check; callers need an initialized collision map
     * and a free player slot. */
    colParam.height = param->height;
    colParam.radius = param->radius;
    colParam.paramA = player->actor->no;
    colParam.paramB = param->param;
    colParam.type = param->type;
    colParam.mask = -1;
    colParam.bounce = 0.2f;
    colParam.narrowHook = PlayerColHook;
    colParam.narrowHook2 = param->narrowHook;
    colParam.colCorrectHook = ActorColCorrectHook;
    colParam.user = player->actor;
    colParam.attr = param->attr|COLBODY_ATTR_ACTIVE|COLBODY_ATTR_RESET;
    ColBodyParamSet(&colParam, player->actor->no);
    player->actor->param = param->param;
    player->actor->type = param->type;
    player->actor->correctHookParam = param->correctHookParam;
    player->actor->correctHook = param->correctHook;
    return player;
}

inline MGPLAYER *MgPlayerCreate(s16 playerNo, MGACTOR_PARAM *param, s16 model, u16 camBit,
                                u32 actionFlag, unsigned int *motDataNum);

/* Creates a player, optionally routing jump mode through the alternate handler. */
MGPLAYER *MgPlayerCreateJumpAlt(s16 playerNo, MGACTOR_PARAM *param, s16 model, u16 camBit,
                                u32 actionFlag, unsigned int *motDataNum, unsigned int jumpAltFlags)
{
    MGPLAYER *player = MgPlayerCreate(playerNo, param, model, camBit, actionFlag, motDataNum);

    if(!player) {
        return NULL;
    }
    if(jumpAltFlags & 0x1) {
        MGPLAYER_MODE_FUNC modeFunc[MGPLAYER_MODE_MAX];
        int modeIndex;
        for(modeIndex=MGPLAYER_MODE_MAX; modeIndex--;) {
            modeFunc[modeIndex] = NULL;
            if(modeIndex == MGPLAYER_MODE_JUMP) {
                modeFunc[modeIndex] = PlayerModeJumpAlt;
            }
        }
        MgPlayerModeFuncSet(player, modeFunc);
    }
    return player;
}

/* Creates a non-player actor and registers its collision parameters and callback. */
MGACTOR *MgActorCreate(MGACTOR_PARAM *param, int mdlId)
{
    MGACTOR *actor = CreateActor(mdlId);
    COLBODY_PARAM colParam;

    /* The result is used without a null check; callers need an initialized collision map
     * and a free non-player slot. */
    colParam.height = param->height;
    colParam.radius = param->radius;
    colParam.paramA = actor->no;
    colParam.paramB = param->param;
    colParam.type = param->type;
    colParam.mask = -1;
    colParam.bounce = 0.2f;
    colParam.narrowHook = NULL;
    colParam.narrowHook2 = param->narrowHook;
    colParam.colCorrectHook = ActorColCorrectHook;
    colParam.user = actor;
    colParam.attr = param->attr|COLBODY_ATTR_ACTIVE|COLBODY_ATTR_RESET;
    ColBodyParamSet(&colParam, actor->no);
    actor->param = param->param;
    actor->type = param->type;
    actor->correctHookParam = param->correctHookParam;
    actor->correctHook = param->correctHook;
    return actor;
}

inline MGACTOR *MgActorCreate(MGACTOR_PARAM *param, int mdlId);

/* Sets the polygon mask used when the collision map tests this model. */
void MgActorColMapMaskSet(int mdlNo, u32 mask)
{
    ColMapMaskSet(mdlNo, mask);
}

/* Returns the current polygon mask for a collision-map model. */
u32 MgActorColMapMaskGet(int mdlNo)
{
    return ColMapMaskGet(mdlNo);
}

/* Updates the body's collision mask and reapplies its parameters when its index is in range. */
void MgActorColMaskSet(MGACTOR *actorP, u32 mask)
{
    COLBODY *body = ColBodyGetSafe(actorP->no);
    if(!body) {
        return;
    }
    body->param.mask = mask;
    ColBodyParamSet(&body->param, actorP->no);
}

/* Returns the body's collision mask, or zero when its index is outside the reserved range. */
u32 MgActorColMaskGet(MGACTOR *actorP)
{
    COLBODY *body = ColBodyGetSafe(actorP->no);
    if(!body) {
        return 0;
    }
    return body->param.mask;
}

/* Restores spherical body positioning for subsequent actor position operations. */
void MgActorColCylReset(void)
{
    actmanWork.colCylF = FALSE;
    ColCylReset();
}

/* Selects cylindrical body positioning for subsequent actor position operations. */
void MgActorColCylSet(void)
{
    actmanWork.colCylF = TRUE;
    ColCylSet();
}

/* Sets the collision body's bounce coefficient. */
void MgActorColBounceSet(MGACTOR *actorP, float bounce)
{
    COLBODY *body = ColBodyGet(actorP->no);
    body->param.bounce = bounce;
}

/* Marks the actor's collision body inactive so actor updates skip it. */
void MgActorKill(MGACTOR *actorP)
{
    COLBODY *body = ColBodyGet(actorP->no);
    body->param.attr &= ~COLBODY_ATTR_ACTIVE;
}

#define GET_STICK_SPEED(playerP, out) do { \
    HuVecF dir; \
    dir.x = HuPadStkX[playerP->padNo]/5.6f; \
    dir.y = 0; \
    dir.z = HuPadStkY[playerP->padNo]/5.6f; \
    out = dir.z*dir.z+(dir.x*dir.x+dir.y*dir.y); \
} while(0)

/* Changes the player's animation only when the requested motion differs. */
static inline void PlayerSetMotion(MGPLAYER *playerP, u16 motNo, float start, float end, u32 attr)
{
    if(playerP->motNo != motNo) {
        CharMotionShiftSet(playerP->charNo, playerP->omObj->mtnId[motNo], start, end, attr);
        playerP->motNo = motNo;
    }
}

/* Update handler for unassigned modes: requests walking mode and the idle animation. */
static void PlayerModeDefault(MGPLAYER *playerP)
{
    MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
    PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
}

/* Handles ground movement and starts jumps or attacks from the minigame update. */
static void PlayerModeWalk(MGPLAYER *playerP)
{
    if(playerP->actor->colGroundAttr & 0x407F) {
        if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
            && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_A)) {
            if(playerP->actionFlag & MGPLAYER_ACTFLAG_JUMP) {
                MgPlayerModeSet(playerP, MGPLAYER_MODE_JUMP);
            }
        } else if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
            && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_B)) {
            if(playerP->actionFlag & MGPLAYER_ACTFLAG_PUNCH) {
                MgPlayerModeSet(playerP, MGPLAYER_MODE_PUNCH);
            }
        } else {
            if(playerP->actionFlag & MGPLAYER_ACTFLAG_WALK) {
                float speed2 = playerP->actor->push.x * playerP->actor->push.x +
                               playerP->actor->push.z * playerP->actor->push.z;
                if(speed2 > 25) {
                    PlayerSetMotion(playerP, MGPLAYER_MOT_RUN, 0, 5, HU3D_MOTATTR_LOOP);
                } else if(speed2 > 0.001) {
                    PlayerSetMotion(playerP, MGPLAYER_MOT_WALK, 0, 5, HU3D_MOTATTR_LOOP);
                } else {
                    PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
                }
            }
        }
    } else {
        if(fabsf(playerP->actor->velY) > 2000) {
            MgPlayerModeSet(playerP, MGPLAYER_MODE_FALL);
        }
    }
}

/* Advances a normal jump, landing, and permitted midair attacks each player update. */
static void PlayerModeJump(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
            PlayerSetMotion(playerP, MGPLAYER_MOT_JUMP, 0, 1, HU3D_MOTATTR_NONE);
            playerP->actor->velY = GET_ACTOR_VELY(playerP->actor, 25000);
            MgPlayerModeAttrSet(playerP, MGPLAYER_MODEATTR_AIR);
            playerP->timer = 1;
            playerP->subMode = 1;
            break;

        case 1:
            /* During the early jump phase, overwrite vertical velocity from the timer;
             * releasing A under pad control switches to the shorter-jump velocity below. */
            playerP->actor->velY =
                GET_ACTOR_VELY(playerP->actor, 25000) - (playerP->timer * playerP->actor->gravity);
            if(playerP->timer >= 10) {
                playerP->timer = 0;
                playerP->subMode = 2;
            } else {
                if(((playerP->actor->colGroundAttr & 0x7F)
                    && playerP->actor->colNorm.y > 0
                    || (playerP->actor->colGroundAttr & 0x4000))
                    && playerP->actor->velY < 0.001) {
                    float speed;
                    GET_STICK_SPEED(playerP, speed);
                    if(speed < 25.0f) {
                        PlayerSetMotion(playerP, MGPLAYER_MOT_LAND, 0, 5, HU3D_MOTATTR_NONE);
                        MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                        playerP->timer = 10;
                        playerP->subMode = 3;
                    } else {
                        MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                        PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
                    }
                } else {
                    if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                        && !(HuPadBtn[playerP->padNo] & PAD_BUTTON_A)) {
                        playerP->actor->velY = GET_ACTOR_VELY(playerP->actor, 4000);
                        playerP->timer = 0;
                        playerP->subMode = 2;
                    }
                }
            }
            playerP->timer++;
            break;

        case 2:
            PlayerSetMotion(playerP, MGPLAYER_MOT_JUMP, 0, 1, HU3D_MOTATTR_NONE);
            if(((playerP->actor->colGroundAttr & 0x7F)
                && playerP->actor->colNorm.y > 0
                || (playerP->actor->colGroundAttr & 0x4000))
                && playerP->actor->velY < 0.001) {
                float speed;
                GET_STICK_SPEED(playerP, speed);
                if(speed < 25.0f) {
                    PlayerSetMotion(playerP, MGPLAYER_MOT_LAND, 0, 5, HU3D_MOTATTR_NONE);
                    MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                    playerP->timer = 10;
                    playerP->subMode = 3;
                } else {
                    MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                    PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
                    if(!(CharAttrGet(playerP->charNo) & 0x10)) {
                        HuVecF pos;
                        MgActorPosGet(playerP->actor, &pos);
                        Hu3DModelPosSet(playerP->actor->mdlId, pos.x, pos.y, pos.z);
                        CharModelLandDustCreateStep(playerP->charNo, &pos);
                    }
                }
            } else {
                if(playerP->motNo == MGPLAYER_MOT_JUMP) {
                    if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                        && !MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_HEADJUMP)
                        && ((HuPadBtnDown[playerP->padNo] & PAD_BUTTON_A)
                        || (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_TRIGGER_L))) {
                        if(playerP->actionFlag & MGPLAYER_ACTFLAG_HIPDROP) {
                            MgPlayerModeSet(playerP, MGPLAYER_MODE_HIPDROP);
                        }
                    } else if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                        && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_B)) {
                        if(playerP->actionFlag & MGPLAYER_ACTFLAG_KICK) {
                            MgPlayerModeSet(playerP, MGPLAYER_MODE_KICK);
                        }
                    }
                }
            }
            break;

        case 3:
            if (playerP->timer <= 9 &&
                !MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH_HARD)) {
                MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            }
            if(playerP->timer > 0) {
                if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                    && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_A)) {
                    playerP->subMode = 0;
                    if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH_HARD)) {
                        MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                    }
                    if(playerP->timer >= 6.0f) {
                        if(!(CharAttrGet(playerP->charNo) & 0x10)) {
                            HuVecF pos;
                            MgActorPosGet(playerP->actor, &pos);
                            Hu3DModelPosSet(playerP->actor->mdlId, pos.x, pos.y, pos.z);
                            CharModelLandDustCreateStep(playerP->charNo, &pos);
                        }
                    }
                }
                playerP->timer--;
            } else {
                MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
            }
            break;
    }
}

/* Handles falling, landing, and permitted midair attacks each player update. */
static void PlayerModeFall(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
            PlayerSetMotion(playerP, MGPLAYER_MOT_JUMP, 0, 1, HU3D_MOTATTR_NONE);
            MgPlayerModeAttrSet(playerP, MGPLAYER_MODEATTR_AIR);
            if(playerP->actor->colGroundAttr & 0x407F) {
                float speed;
                GET_STICK_SPEED(playerP, speed);
                if(speed < 25.0f) {
                    PlayerSetMotion(playerP, MGPLAYER_MOT_LAND, 0, 5, HU3D_MOTATTR_NONE);
                    MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                    playerP->timer = 10;
                    playerP->subMode = 1;
                } else {
                    MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                    PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
                    if(!(CharAttrGet(playerP->charNo) & 0x10)) {
                        HuVecF pos;
                        MgActorPosGet(playerP->actor, &pos);
                        Hu3DModelPosSet(playerP->actor->mdlId, pos.x, pos.y, pos.z);
                        CharModelLandDustCreateStep(playerP->charNo, &pos);
                    }
                }
            } else {
                if(playerP->motNo == MGPLAYER_MOT_JUMP) {
                    if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                        && ((HuPadBtnDown[playerP->padNo] & PAD_BUTTON_A)
                        || (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_TRIGGER_L))) {
                        if(playerP->actionFlag & MGPLAYER_ACTFLAG_HIPDROP) {
                            MgPlayerModeSet(playerP, MGPLAYER_MODE_HIPDROP);
                        }
                    } else if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                        && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_B)) {
                        if(playerP->actionFlag & MGPLAYER_ACTFLAG_KICK) {
                            MgPlayerModeSet(playerP, MGPLAYER_MODE_KICK);
                        }
                    }
                }
            }
            break;

        case 1:
            if (playerP->timer <= 9 &&
                !MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH_HARD)) {
                MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            }
            if(playerP->timer > 0) {
                if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                    && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_A)) {
                    if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH_HARD)) {
                        MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                    }
                    if(playerP->timer >= 6.0f) {
                        if(!(CharAttrGet(playerP->charNo) & 0x10)) {
                            HuVecF pos;
                            MgActorPosGet(playerP->actor, &pos);
                            Hu3DModelPosSet(playerP->actor->mdlId, pos.x, pos.y, pos.z);
                            CharModelLandDustCreateStep(playerP->charNo, &pos);
                        }
                    }
                    MgPlayerModeSet(playerP, MGPLAYER_MODE_JUMP);
                }
                playerP->timer--;
            } else {
                MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
            }
            break;
    }
}

/* Runs the punch animation and its short-lived collision actor during player updates. */
static void PlayerModePunch(MGPLAYER *playerP)
{
    MGACTOR_PARAM param;
    switch(playerP->subMode) {
        case 0:
        {
            int no;
            COLBODY *body;
            PlayerSetMotion(playerP, MGPLAYER_MOT_PUNCH, 0, 5, HU3D_MOTATTR_NONE);
            playerP->timer = 20;
            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            no = playerP->actor->no;
            body = ColBodyGetSafe(no);
            param.param = body->param.paramA+0x100;
            param.type = 2;
            param.height = 120;
            param.radius = 60;
            param.attr = COLBODY_ATTR_ACTIVE|COLBODY_ATTR_RESET|0xA;
            param.narrowHook = NULL;
            param.correctHook = NULL;
            actmanWork.effPlayer[no] = MgActorCreate(&param, HU3D_MODELID_NONE);
            MgActorColMaskSet(actmanWork.effPlayer[no], MgActorColMaskGet(playerP->actor));
            actmanWork.effPlayer[no]->rotY = playerP->actor->rotY;
            actmanWork.effPlayer[no]->pos = playerP->actor->pos;
            actmanWork.effPlayer[no]->push.x = 0;
            actmanWork.effPlayer[no]->push.y = 0;
            actmanWork.effPlayer[no]->push.z = 0;
            actmanWork.effPlayer[no]->gravity = 0;
            playerP->subMode = 1;
        }

            break;

        case 1:
        {
            int no;
            no = playerP->actor->no;
            if(playerP->timer > 0) {
                playerP->timer--;
                if(playerP->timer >= 10) {
                    actmanWork.effPlayer[no]->push.x = 0;
                    actmanWork.effPlayer[no]->push.y = 0;
                    actmanWork.effPlayer[no]->push.z = 10;
                } else {
                    actmanWork.effPlayer[no]->push.x = 0;
                    actmanWork.effPlayer[no]->push.y = 0;
                    actmanWork.effPlayer[no]->push.z = 0;
                }
            } else {
                KillEffectPlayer(no);
                MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                playerP->subMode = 2;
            }
        }
            break;

        case 2:
            MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
            PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
            break;
    }
}

/* Runs the kick animation and collision actor until landing during player updates. */
static void PlayerModeKick(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
        {
            int no;
            COLBODY *body;
            MGACTOR_PARAM param;
            PlayerSetMotion(playerP, MGPLAYER_MOT_KICK, 0, 5, HU3D_MOTATTR_NONE);
            playerP->timer = 20;
            no = playerP->actor->no;
            body = ColBodyGetSafe(no);
            param.param = body->param.paramA+0x100;
            param.type = 3;
            param.height = 120;
            param.radius = 60;
            param.attr = COLBODY_ATTR_ACTIVE|COLBODY_ATTR_RESET|0xA;
            param.narrowHook = NULL;
            param.correctHook = NULL;
            actmanWork.effPlayer[no] = MgActorCreate(&param, HU3D_MODELID_NONE);
            MgActorColMaskSet(actmanWork.effPlayer[no], MgActorColMaskGet(playerP->actor));
            actmanWork.effPlayer[no]->rotY = playerP->actor->rotY;
            actmanWork.effPlayer[no]->pos = playerP->actor->pos;
            actmanWork.effPlayer[no]->push.x = 0;
            actmanWork.effPlayer[no]->push.y = -20;
            actmanWork.effPlayer[no]->push.z = 60;
            actmanWork.effPlayer[no]->gravity = 0;
            playerP->timer = 20;
            playerP->subMode = 1;
        }

            break;

        case 1:
        {
            int no;
            no = playerP->actor->no;
            if(playerP->actor->colGroundAttr & 0x407F) {
                PlayerSetMotion(playerP, MGPLAYER_MOT_LAND, 0, 5, HU3D_MOTATTR_NONE);
                KillEffectPlayer(no);
                playerP->timer = 10;
                playerP->subMode = 2;
            } else {
                actmanWork.effPlayer[no]->rotY = playerP->actor->rotY;
                actmanWork.effPlayer[no]->pos = playerP->actor->pos;
                actmanWork.effPlayer[no]->push.x = 0;
                actmanWork.effPlayer[no]->push.y = -20;
                actmanWork.effPlayer[no]->push.z = 60;
            }
        }
            break;

        case 2:
            if(playerP->timer > 0) {
                /* Replace the landing timer with -1 on the first update after landing;
                 * the following update returns the player to walking. */
                playerP->timer = -1;
            } else {
                PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
                MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
            }

            break;
    }
}

/* Drives the airborne hip drop and landing impact during player updates. */
static void PlayerModeHipDrop(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
            PlayerSetMotion(playerP, MGPLAYER_MOT_HIPDROP, 0, 5, HU3D_MOTATTR_NONE);
            playerP->timer = 20;
            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            playerP->subMode = 1;
            break;

        case 1:
            if(playerP->timer > 0) {
                playerP->timer--;
                /* Cancel the later gravity subtraction to suspend vertical motion during
                 * the wind-up. */
                playerP->actor->velY = playerP->actor->gravity;
            } else {
                int no;
                COLBODY *body;
                MGACTOR_PARAM param;
                no = playerP->actor->no;
                body = ColBodyGetSafe(no);
                param.param = body->param.paramA+0x100;
                param.type = 4;
                param.height = 10;
                param.radius = 80;
                param.attr = COLBODY_ATTR_ACTIVE|COLBODY_ATTR_RESET|0xA;
                param.narrowHook = NULL;
                param.correctHook = NULL;
                actmanWork.effPlayer[no] = MgActorCreate(&param, HU3D_MODELID_NONE);
                MgActorColMaskSet(actmanWork.effPlayer[no], MgActorColMaskGet(playerP->actor));
                actmanWork.effPlayer[no]->rotY = playerP->actor->rotY;
                actmanWork.effPlayer[no]->pos = playerP->actor->pos;
                actmanWork.effPlayer[no]->pos.y -= 10;
                actmanWork.effPlayer[no]->push.x = 0;
                actmanWork.effPlayer[no]->push.y = 0;
                actmanWork.effPlayer[no]->push.z = 0;
                actmanWork.effPlayer[no]->gravity = 0;
                playerP->actor->velY = -1000;
                playerP->subMode = 2;
            }
            break;

        case 2:
        {
            int no;
            no = playerP->actor->no;
            actmanWork.effPlayer[no]->pos = playerP->actor->pos;
            actmanWork.effPlayer[no]->pos.y -= 10;
            if(playerP->actor->colGroundAttr & 0x407F) {
                playerP->timer = 20;
                playerP->subMode = 3;
                MgPlayerVibAttrSet(playerP, MGPLAYER_VIBATTR_HIPDROP_LAND);
                PlayerSetMotion(playerP, MGPLAYER_MOT_HIPDROP_LAND, 0, 5, HU3D_MOTATTR_NONE);
                KillEffectPlayer(playerP->actor->no);
            }
        }
            break;

        case 3:
            if(playerP->timer > 0) {
                playerP->timer--;
            } else {
                playerP->timer = 20;
                playerP->subMode = 4;
                PlayerSetMotion(playerP, MGPLAYER_MOT_HIPDROP_END, 0, 5, HU3D_MOTATTR_NONE);
            }
            break;

        case 4:
            if(playerP->timer > 0) {
                playerP->timer--;
            } else {
                if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH_HARD)) {
                    MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                }
                MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
            }
            break;
    }
}

/* Selects a front/back hit reaction, locks control and applies its push,
 * then resumes walking with the blink stun. */
static void PlayerModeHit(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
        {
            Mtx rotMtx;
            HuVecF forwardBasis;
            HuVecF rotDir;
            HuVecF stunDir;
            playerP->stunAngle = WrapAngle(playerP->stunAngle);
            playerP->actor->rotY = WrapAngle(playerP->actor->rotY);
            forwardBasis.x = 0;
            forwardBasis.y = 0;
            forwardBasis.z = 1;
            MTXRotRad(rotMtx, 'Y', 0.017453292f*playerP->actor->rotY);
            MTXMultVec(rotMtx, &forwardBasis, &rotDir);
            MTXRotRad(rotMtx, 'Y', 0.017453292f*playerP->stunAngle);
            MTXMultVec(rotMtx, &forwardBasis, &stunDir);
            if(VECDotProduct(&rotDir, &stunDir) < 0) {
                PlayerSetMotion(playerP, MGPLAYER_MOT_HIT_BACK, 0, 5, HU3D_MOTATTR_NONE);
                playerP->actor->rotY = WrapAngle(180+playerP->stunAngle);
            } else {
                PlayerSetMotion(playerP, MGPLAYER_MOT_HIT_FRONT, 0, 5, HU3D_MOTATTR_NONE);
                playerP->actor->rotY = playerP->stunAngle;
            }
            playerP->timer = 45;
            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            playerP->stunType = MGPLAYER_STUN_FREEZE;
            playerP->subMode = 1;
        }

            break;

        case 1:
            if(playerP->timer > 0) {
                playerP->actor->push.x = 0;
                playerP->actor->push.y = 0;
                if(playerP->motNo == MGPLAYER_MOT_HIT_BACK) {
                    if(playerP->timer > 25) {
                        playerP->actor->push.z = -(playerP->timer/1.7f)*((playerP->timer-25)/20);
                    }
                } else if(playerP->motNo == MGPLAYER_MOT_HIT_FRONT) {
                    if(playerP->timer > 0) {
                        playerP->actor->push.z = (playerP->timer/3.0f)*(playerP->timer/45);
                    }
                }
                playerP->timer--;
            } else {
                playerP->subMode = 2;
            }
            break;

        case 2:
            playerP->stunType = MGPLAYER_STUN_BLINK;
            MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
            PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
            break;

    }
}

/* Faces and pushes the player away from an impact, then finishes the reaction. */
static void PlayerModeKnockback(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
        {
            Mtx rotMtx;
            HuVecF forwardBasis;
            HuVecF rotDir;
            HuVecF stunDir;
            playerP->stunAngle = WrapAngle(playerP->stunAngle);
            playerP->actor->rotY = WrapAngle(playerP->actor->rotY);
            forwardBasis.x = 0;
            forwardBasis.y = 0;
            forwardBasis.z = 1;
            MTXRotRad(rotMtx, 'Y', 0.017453292f*playerP->actor->rotY);
            MTXMultVec(rotMtx, &forwardBasis, &rotDir);
            MTXRotRad(rotMtx, 'Y', 0.017453292f*playerP->stunAngle);
            MTXMultVec(rotMtx, &forwardBasis, &stunDir);
            if(VECDotProduct(&rotDir, &stunDir) < 0) {
                PlayerSetMotion(playerP, MGPLAYER_MOT_KNOCKBACK_BACK, 0, 5, HU3D_MOTATTR_NONE);
                playerP->actor->rotY = WrapAngle(180+playerP->stunAngle);
            } else {
                PlayerSetMotion(playerP, MGPLAYER_MOT_KNOCKBACK_FRONT, 0, 5, HU3D_MOTATTR_NONE);
                playerP->actor->rotY = playerP->stunAngle;
            }
            playerP->timer = 45;
            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            playerP->stunType = MGPLAYER_STUN_FREEZE;
            playerP->subMode = 1;
        }
            break;

        case 1:
            if(playerP->timer > 0) {
                playerP->actor->push.x = 0;
                playerP->actor->push.y = 0;
                if(playerP->timer > 25) {
                    if(playerP->motNo == MGPLAYER_MOT_KNOCKBACK_BACK) {
                        playerP->actor->push.z = -(playerP->timer/1.4f)*((playerP->timer-25)/20);
                    } else if(playerP->motNo == MGPLAYER_MOT_KNOCKBACK_FRONT) {
                        playerP->actor->push.z = (playerP->timer/1.4f)*((playerP->timer-25)/20);
                    }
                }
                playerP->timer--;
            } else {
                if(playerP->motNo == MGPLAYER_MOT_KNOCKBACK_BACK) {
                    PlayerSetMotion(playerP, MGPLAYER_MOT_KNOCKBACK_BACK_END, 0, 5,
                                    HU3D_MOTATTR_NONE);
                } else if(playerP->motNo == MGPLAYER_MOT_KNOCKBACK_FRONT) {
                    PlayerSetMotion(playerP, MGPLAYER_MOT_KNOCKBACK_FRONT_END, 0, 5,
                                    HU3D_MOTATTR_NONE);
                }
                playerP->timer = 30;
                playerP->subMode = 2;
            }
            break;

        case 2:
            if(playerP->timer > 0) {
                playerP->timer--;
            } else {
                playerP->stunType = MGPLAYER_STUN_BLINK;
                MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
            }

            break;
    }
}

/* Temporarily flattens the player and collision body after a light squish stun. */
static void PlayerModeSquish(MGPLAYER *playerP)
{
    COLBODY *body;
    float t, scaleXZ, scaleY;

    switch(playerP->subMode) {
        case 0:
            body = ColBodyGet(playerP->actor->no);
            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            playerP->timer = 8;
            if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH)) {
                MgPlayerModeAttrSet(playerP, MGPLAYER_MODEATTR_SQUISH);
                playerP->squishOldHeight = body->param.height;
                body->param.height = playerP->squishOldHeight*0.5f;
            }
            playerP->stunType = MGPLAYER_STUN_FREEZE;
            playerP->subMode = 1;
            break;

        case 1:
            t = playerP->timer/8;
            scaleXZ = 1.25f-(0.25f*t);
            scaleY = 0.5f+(0.5f*t);
            if(playerP->timer > 0) {
                playerP->timer--;
                Hu3DModelScaleSet(playerP->actor->mdlId, scaleXZ, scaleY, scaleXZ);
            } else {
                Hu3DModelScaleSet(playerP->actor->mdlId, 1.25f, 0.5f, 1.25f);
                playerP->subMode = 2;
            }
            break;

        case 2:
            MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            playerP->stunType = MGPLAYER_STUN_BLINK;
            playerP->squishTime = 90;
            MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
            PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
            break;
    }
}

/* Hides body collision during a hard squish, then restores the player's shape. */
static void PlayerModeSquishHard(MGPLAYER *playerP)
{
    COLBODY *body;

    switch(playerP->subMode) {
        case 0:
            MgActorColAttrSet(playerP->actor, COLBODY_ATTR_BODYCOL_OFF);
            MgPlayerModeAttrSet(playerP, MGPLAYER_MODEATTR_SQUISH_HARD);
            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            if(MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH)) {
                body = ColBodyGet(playerP->actor->no);
                MgPlayerModeAttrReset(playerP, MGPLAYER_MODEATTR_SQUISH);
                body->param.height = playerP->squishOldHeight;
            }
            playerP->timer = 5;
            PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
            playerP->stunType = MGPLAYER_STUN_FREEZE;
            playerP->subMode = 1;
            break;

        case 1:
            if(playerP->timer > 0) {
                float scale;
                playerP->timer--;
                scale = 1+((10*(5-playerP->timer))/100);
                Hu3DModelScaleSet(playerP->actor->mdlId, scale,
                                  0.05f + ((20 * playerP->timer) / 100), scale);
            } else {
                playerP->timer = 120;
                playerP->subMode = 2;
            }
            break;

        case 2:
            if(playerP->timer > 0) {
                playerP->timer--;
            } else {
                /* Restore body collision before the blinking reshape; movement and turning
                 * stay locked until that phase finishes. */
                MgActorColAttrReset(playerP->actor, COLBODY_ATTR_BODYCOL_OFF);
                playerP->timer = 100;
                playerP->subMode = 3;
            }
            break;

        case 3:
        {
            float t = playerP->timer/100;
            float angle = 270+((playerP->timer*2880)/100);
            float scale;
            t = (1+(0.5*(t*HuSin(angle))))-(t*0.45f);
            scale = 1+(0.08f*(playerP->timer-93));
            if(Hu3DModelAttrGet(playerP->actor->mdlId) & HU3D_ATTR_DISPOFF) {
                Hu3DModelDispOn(playerP->actor->mdlId);
            } else {
                Hu3DModelDispOff(playerP->actor->mdlId);
            }
            if(playerP->timer > 0) {
                if(playerP->timer > 93) {
                    Hu3DModelScaleSet(playerP->actor->mdlId, scale, t, scale);
                } else {
                    Hu3DModelScaleSet(playerP->actor->mdlId, 1, t, 1);
                }
                playerP->timer--;
            } else {
                Hu3DModelScaleSet(playerP->actor->mdlId, 1, 1, 1);
                playerP->subMode = 4;
            }
        }
            break;

        case 4:
            if(Hu3DModelAttrGet(playerP->actor->mdlId) & HU3D_ATTR_DISPOFF) {
                Hu3DModelDispOn(playerP->actor->mdlId);
            } else {
                Hu3DModelDispOff(playerP->actor->mdlId);
            }
            MgPlayerModeAttrReset(playerP, MGPLAYER_MODEATTR_SQUISH_HARD);
            MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            playerP->stunType = MGPLAYER_STUN_BLINK;
            MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
            PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
            break;
    }
}

/* Advances the alternate jump and landing when minigame setup enables it. */
static void PlayerModeJumpAlt(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
            MgPlayerModeAttrSet(playerP, MGPLAYER_MODEATTR_AIR);
            PlayerSetMotion(playerP, MGPLAYER_MOT_JUMP, 0, 1, HU3D_MOTATTR_NONE);
            playerP->actor->velY = GET_ACTOR_VELY(playerP->actor, 25000);
            playerP->subMode = 1;
            break;

        case 1:
        case 2:
            PlayerSetMotion(playerP, MGPLAYER_MOT_JUMP, 0, 1, HU3D_MOTATTR_NONE);
            if(((playerP->actor->colGroundAttr & 0x7F)
                    && playerP->actor->colNorm.y > 0
                    || (playerP->actor->colGroundAttr & 0x4000))
                    && playerP->actor->velY < 0.001) {
                        float speed = VECSquareMag(&playerP->actor->push);
                        if(speed < 25.0f) {
                            PlayerSetMotion(playerP, MGPLAYER_MOT_LAND, 0, 5, HU3D_MOTATTR_NONE);
                            MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                            playerP->timer = 10;
                            playerP->subMode = 3;
                        } else {
                            MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                            PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
                        }
                    } else {
                        /* This comparison's result is discarded; it does not change motion
                         * or start an attack. */
                        playerP->motNo == MGPLAYER_MOT_JUMP;
                    }
            break;

        case 3:
            if (playerP->timer <= 9 &&
                !MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH_HARD)) {
                MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
            }
            if(playerP->timer > 0) {
                if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)
                    && (HuPadBtnDown[playerP->padNo] & PAD_BUTTON_A)) {
                        playerP->subMode = 0;
                        if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH_HARD)) {
                            MgPlayerAttrReset(playerP,
                                              MGPLAYER_ATTR_ANGLELOCK | MGPLAYER_ATTR_MOVEOFF);
                        }
                    }
                    playerP->timer--;
            } else {
                MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
                PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
            }
            break;
    }
}

/* Copies the last contact and derives vertical correction velocity after collision.
 * With no contact, clears metadata and correction velocity but retains the previous normal. */
static inline void InitColPoint(MGACTOR *actorP, COLBODY *colBody)
{
    COLBODY_POINT *point;
    if(colBody->colPointNum) {
        point = &colBody->colPoint[colBody->colPointNum-1];
        actorP->colOfs = point->colOfs;
        actorP->colNorm = point->normal;
        actorP->colMesh = point->meshNo;
        actorP->colObj = point->obj;
        actorP->colFace = point->faceNo;
        actorP->terminalVelY = (point->colOfs.y < 0) ? (100*point->colOfs.y) : 0;
    } else {
        memset(&actorP->colOfs, 0, sizeof(HuVecF));
        actorP->colMesh = 0xFFFF;
        actorP->colObj = NULL;
        actorP->colFace = -1;
        actorP->terminalVelY = 0;
    }
}

/* Updates active minigame players and actors once per minigame frame. */
void MgActorExec(void)
{
    MGPLAYER *playerP;
    MGACTOR *actorP;
    COLBODY *colBody;
    int i;

    float stickSpeed;
    float t;
    float scaleXZ;
    float angle;
    float scaleY;
    float speedY;

    Mtx stickMtx;
    Mtx rotMtx;
    HuVecF stickVec;
    HuVecF stickDir;
    HuVecF vel;
    HuVecF pos;
    HuVecF gravityForce;
    HuVecF normForce;

    if(!ColMapInitCheck()) {
        return;
    }
    for(i=0; i<PLAYER_MAX; i++) {
        colBody = ColBodyGet(i);
        if(colBody->param.attr & COLBODY_ATTR_ACTIVE) {
            playerP = &actmanWork.player[i];
            if(MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_PAUSE)) {
                continue;
            }
            if ((playerP->actionFlag & MGPLAYER_ACTFLAG_WALK) &&
                !MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_COMSTK)) {
                /* With MOVEOFF, stickVec is not refreshed; speed and optional facing below
                 * reuse a previous loop iteration's vector, or an uninitialized value if none
                 * filled it. */
                if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_MOVEOFF)) {
                    stickVec.x = HuPadStkX[playerP->padNo]/5.6f;
                    stickVec.y = 0;
                    stickVec.z = HuPadStkY[playerP->padNo]/5.6f;
                    GetStickMtx(stickMtx, playerP->camBit);
                    MTXMultVec(stickMtx, &stickVec, &stickVec);
                }
                stickSpeed = stickVec.z*stickVec.z+(stickVec.x*stickVec.x+stickVec.y*stickVec.y);
                if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_ANGLELOCK)) {
                    if(stickSpeed >= 0.001) {
                        SafeNormalize(&stickVec, &stickDir);
                        if(stickDir.z > 1) {
                            stickDir.z = 1;
                        } else if(stickDir.z < -1) {
                            stickDir.z = -1;
                        }
                        angle = acosf(stickDir.z);
                        if(stickDir.x < 0) {
                            angle = -angle;
                        }
                        playerP->actor->rotY = (180*angle)/M_PI;
                    }
                }
                if(!MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_MOVEOFF)) {
                    playerP->actor->push.x = 0;
                    playerP->actor->push.y = 0;
                    playerP->actor->push.z = sqrtf(stickSpeed);
                }
            } else if (MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_MOVEOFF)) {
                playerP->actor->push.x = playerP->actor->push.y = playerP->actor->push.z = 0;
            }
            /* Clear vibration flags before this update's mode and collision reactions
             * generate requests. */
            playerP->vibAttr = 0;
            if(MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH)) {
                if (playerP->stunType == MGPLAYER_STUN_BLINK ||
                    playerP->stunType == MGPLAYER_STUN_NONE) {
                    playerP->squishTime--;
                    if(playerP->squishTime < 0) {
                        if(playerP->squishTime < -20) {
                            colBody = ColBodyGet(playerP->actor->no);
                            MgPlayerModeAttrReset(playerP, MGPLAYER_MODEATTR_SQUISH);
                            Hu3DModelScaleSet(playerP->actor->mdlId, 1, 1, 1);
                            colBody->param.height = playerP->squishOldHeight;
                            playerP->squishTime = 0;
                        } else {
                           if(playerP->squishTime > (-20.0f/3.0f)) {
                                t = playerP->squishTime/(-20.0f/3.0f);
                                scaleY = 0.5f+(0.3f*t);
                                scaleXZ = 1+(0.25f-(0.25f*t));
                            } else if(playerP->squishTime > (-40.0f/3.0f)) {
                                t = (playerP->squishTime-(-20.0f/3.0f))/(-20.0f/3.0f);
                                scaleY = 0.8f-(0.1f*t);
                                scaleXZ = 1;
                            } else {
                                t = (playerP->squishTime-(-40.0f/3.0f))/(-20.0f/3.0f);
                                scaleXZ = 1;
                                scaleY = 0.7f+(0.3f*t);
                            }
                            Hu3DModelScaleSet(playerP->actor->mdlId, scaleXZ, scaleY, scaleXZ);
                        }
                    }
                }
            }
            if(MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH)) {
                VECScale(&playerP->actor->push, &playerP->actor->push, 0.5f);
            }
            if(MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_HEADJUMP)) {
                if(MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_STKLOCK)) {
                    if(playerP->actor->velY <= 0) {
                        if(!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH_HARD)
                            && playerP->stunType == MGPLAYER_STUN_NONE
                            && playerP->mode != MGPLAYER_MODE_HIPDROP) {
                            MgPlayerModeAttrReset(playerP, MGPLAYER_MODEATTR_STKLOCK);
                            MgPlayerAttrReset(playerP,
                                              MGPLAYER_ATTR_ANGLELOCK | MGPLAYER_ATTR_MOVEOFF);
                        }
                    } else {
                        MgPlayerAttrSet(playerP, MGPLAYER_ATTR_ANGLELOCK|MGPLAYER_ATTR_MOVEOFF);
                    }
                }
                if(playerP->actor->colGroundAttr & 0x407F) {
                    MgPlayerModeAttrReset(playerP, MGPLAYER_MODEATTR_HEADJUMP);
                    if (!MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH_HARD) &&
                        playerP->mode != MGPLAYER_MODE_HIPDROP &&
                        (playerP->stunType == MGPLAYER_STUN_NONE ||
                         playerP->stunType == MGPLAYER_STUN_BLINK)) {
                        MgPlayerAttrReset(playerP, MGPLAYER_ATTR_ANGLELOCK | MGPLAYER_ATTR_MOVEOFF);
                    }
                    memset(&playerP->actor->forceB, 0, sizeof(HuVecF));
                }
            }
            if(playerP->actor->colGroundAttr & 0x407F) {
                MgPlayerModeAttrReset(playerP, MGPLAYER_MODEATTR_AIR);
            } else if (playerP->actor->velY <
                       -((5 * playerP->actor->gravity) + (5 * playerP->actor->gravity))) {
                MgPlayerModeAttrSet(playerP, MGPLAYER_MODEATTR_AIR);
            }
            if(playerP->actionFlag & MGPLAYER_ACTFLAG_STUN) {
                switch(playerP->stunType) {
                    case MGPLAYER_STUN_NONE:
                        break;

                    case MGPLAYER_STUN_HIT:
                        if(playerP->actor->velY > 0) {
                            playerP->actor->velY = 0;
                        }
                        KillEffectPlayer(playerP->actor->no);
                        MgPlayerModeSet(playerP, MGPLAYER_MODE_HIT);
                        break;

                    case MGPLAYER_STUN_KNOCKBACK:
                        if(playerP->actor->velY > 0) {
                            playerP->actor->velY = 0;
                        }
                        KillEffectPlayer(playerP->actor->no);
                        MgPlayerModeSet(playerP, MGPLAYER_MODE_KNOCKBACK);
                        break;

                    case MGPLAYER_STUN_SQUISH:
                        if(playerP->actor->velY > 0) {
                            playerP->actor->velY = 0;
                        }
                        KillEffectPlayer(playerP->actor->no);
                        MgPlayerModeSet(playerP, MGPLAYER_MODE_SQUISH);
                        break;

                    case MGPLAYER_STUN_SQUISH_HARD:
                        if(playerP->actor->velY > 0) {
                            playerP->actor->velY = 0;
                        }
                        KillEffectPlayer(playerP->actor->no);
                        MgPlayerModeSet(playerP, MGPLAYER_MODE_SQUISH_HARD);
                        break;

                    case MGPLAYER_STUN_BLINK:
                        if(Hu3DModelAttrGet(playerP->actor->mdlId) & HU3D_ATTR_DISPOFF) {
                            Hu3DModelDispOn(playerP->actor->mdlId);
                        } else {
                            Hu3DModelDispOff(playerP->actor->mdlId);
                        }
                        if(playerP->stunTime > 0) {
                            playerP->stunTime--;
                        } else {
                            Hu3DModelDispOn(playerP->actor->mdlId);
                            playerP->stunType = MGPLAYER_STUN_NONE;
                        }
                        break;
                }
            }
            playerP->modeFunc[playerP->mode](playerP);
        }
    }
    ColDirtyClear();
    for(i=0; i<ACTOR_MAX; i++) {
        colBody = ColBodyGet(i);
        if(colBody->param.attr & COLBODY_ATTR_ACTIVE) {
            actorP = &actmanWork.actor[i];
            if(actorP->attr & 0x1) {
                continue;
            }
            if(actorP->mdlId >= 0) {
                Hu3DModelRotSet(actorP->mdlId, 0, actorP->rotY, 0);
            }
            if(actorP->colGroundAttr & 0x407F) {
                if(actorP->velY < 0.001 && actorP->terminalVelY > -(5*actorP->gravity)) {
                    actorP->velY = -(5*actorP->gravity);
                }
            }
            actorP->velY -= actorP->gravity;
            speedY = actorP->velY/100;
            actorP->push.y += speedY;
            /* Rotate the one-update push into world space and add persistent velocity and
             * forces. Vertical velocity is divided by 100 above; push is cleared after
             * advancing position. */
            vel = actorP->push;
            MTXRotRad(rotMtx, 'Y', 0.017453292f*actorP->rotY);
            MTXMultVec(rotMtx, &vel, &vel);
            VECAdd(&vel, &actorP->vel, &vel);
            VECAdd(&vel, &actorP->forceB, &vel);
            VECAdd(&vel, &actorP->forceA, &vel);
            VECAdd(&actorP->pos, &vel, &actorP->pos);
            actorP->push.x = actorP->push.y =  actorP->push.z = 0;
            ColBodyPosSet(&actorP->pos, i);
            actorP->oldPos = actorP->pos;
        }
    }
    ColBodyExec();
    for(i=0; i<ACTOR_MAX; i++) {
        colBody = ColBodyGet(i);
        if(colBody->param.attr & COLBODY_ATTR_ACTIVE) {
            actorP = &actmanWork.actor[i];
            if(actorP->attr & 0x1) {
                continue;
            }
            /* If a correction callback changed the actor position, keep that override and
             * skip collision position/contact updates. This path uses height/2 and does not
             * check mdlId. */
            if (actorP->oldPos.x != actorP->pos.x || actorP->oldPos.y != actorP->pos.y ||
                actorP->oldPos.z != actorP->pos.z) {
                Hu3DModelPosSet(actorP->mdlId, actorP->pos.x,
                                actorP->pos.y - (colBody->param.height / 2), actorP->pos.z);
            } else {
                ColBodyPosGet(&actorP->pos, i);
                if(actorP->mdlId >= 0) {
                    MgActorPosGet(actorP, &pos);
                    Hu3DModelPosSet(actorP->mdlId, pos.x, pos.y, pos.z);
                }
                actorP->colGroundAttr = colBody->groundAttr;
                InitColPoint(actorP, colBody);
                if(actorP->colGroundAttr && (actorP->colGroundAttr & 0x407F)) {
                    /* Add 4% of the vertical motion's contact-plane tangent to persistent
                     * forceA; a negligible tangent clears that force instead. */
                    if(colBody->paramAttr & 0x2) {
                        gravityForce.x = gravityForce.z = 0;
                        gravityForce.y = actorP->velY/100;
                        VECScale(&actorP->colNorm, &normForce,
                                 VECDotProduct(&actorP->colNorm, &gravityForce));
                        VECSubtract(&gravityForce, &normForce, &gravityForce);
                        VECScale(&gravityForce, &gravityForce, 0.04f);
                        if(VECSquareMag(&gravityForce) > 0.001) {
                            VECAdd(&actorP->forceA, &gravityForce, &actorP->forceA);
                        } else {
                            memset(&actorP->forceA, 0, sizeof(HuVecF));
                        }
                    } else {
                        memset(&actorP->forceA, 0, sizeof(HuVecF));
                    }
                    if(actorP->velY < actorP->terminalVelY) {
                        actorP->velY = actorP->terminalVelY;
                    } else {
                        actorP->velY = 0;
                    }
                }
            }
        }
    }
}

/* Returns the first map polygon hit by a segment, when collision data is ready. */
BOOL MgActorColMapPolyGet(HuVecF *pos1, HuVecF *pos2, u32 mask, MGACTOR_COLMAP_POLY *outPoly)
{
    if(!ColMapInitCheck()) {
        return FALSE;
    }
    return ColMapPolyGet(pos1, pos2, mask, &outPoly->pos, &outPoly->code, &outPoly->mdlNo,
                         &outPoly->obj, &outPoly->triNo);
}

/* Attempts stun/shape and walking resets, ignoring failures, then requests idle motion
 * and moves the actor even if stun or mode remains unchanged. */
void MgPlayerPosSet(MGPLAYER *playerP, HuVecF *pos)
{
    MgPlayerModeIdleSet(playerP);
    MgPlayerModeSet(playerP, MGPLAYER_MODE_WALK);
    PlayerSetMotion(playerP, MGPLAYER_MOT_IDLE, 0, 5, HU3D_MOTATTR_LOOP);
    MgActorPosSet(playerP->actor, pos);
}

/* Moves an actor and marks its collision body for position synchronization. */
void MgActorPosSet(MGACTOR *actorP, HuVecF *pos)
{
    COLBODY *colBody = ColBodyGet(actorP->no);
    colBody->param.attr |= COLBODY_ATTR_RESET;
    MgActorPosSetRaw(actorP, pos);
}

/* Stores the actor's center from its base position; the collision buffers update during
 * MgActorExec. */
void MgActorPosSetRaw(MGACTOR *actorP, HuVecF *pos)
{
    COLBODY *colBody;
    if(actorP->no < 0) {
        return;
    }
    colBody = ColBodyGet(actorP->no);
    if(colBody->param.attr & COLBODY_ATTR_ACTIVE) {
        actorP->pos = *pos;
        if(actmanWork.colCylF) {
            actorP->pos.y += colBody->param.height/2;
        } else {
            actorP->pos.y += colBody->param.radius;
        }
    }
}

/* Gets an actor's base position from its collision body's stored center. */
void MgActorPosGet(MGACTOR *actorP, HuVecF *pos)
{
    COLBODY *colBody;
    if(actorP->no < 0) {
        return;
    }
    colBody = ColBodyGet(actorP->no);
    if(colBody->param.attr & COLBODY_ATTR_ACTIVE) {
        *pos = actorP->pos;
        if(actmanWork.colCylF) {
            pos->y -= colBody->param.height/2;
        } else {
            pos->y -= colBody->param.radius;
        }
    }
}

void MgActorPushSet(MGACTOR *actorP, HuVecF *push)
{
    actorP->push = *push;
}

void MgActorRotYSet(MGACTOR *actorP, float rotY)
{
    actorP->rotY = rotY;
}

void MgActorRotYGet(MGACTOR *actorP, float *rotY)
{
    *rotY = actorP->rotY;
}

void MgActorGravitySet(MGACTOR *actorP, float gravity)
{
    actorP->gravity = gravity;
}

void MgActorVelYSet(MGACTOR *actorP, float velY)
{
    actorP->velY = velY;
}

void MgActorVelSet(MGACTOR *actorP, HuVecF *vel)
{
    actorP->vel = *vel;
}

/* Returns the last contacted mesh when the body has a recorded collision result. */
BOOL MgActorColMeshGet(MGACTOR *actorP, int *mesh)
{
    COLBODY *body = ColBodyGet(actorP->no);
    u32 attr = body->param.attr & COLBODY_ATTR_COL_RESULT_MASK;
    u32 result = attr;
    if(!result) {
        return FALSE;
    }
    *mesh = actorP->colMesh;
    return TRUE;
}

/* Returns the last contact normal when the body has a recorded collision result. */
BOOL MgActorColNormalGet(MGACTOR *actorP, HuVecF *normal)
{
    COLBODY *body = ColBodyGet(actorP->no);
    u32 attr = body->param.attr & COLBODY_ATTR_COL_RESULT_MASK;
    u32 result = attr;
    if(!result) {
        return FALSE;
    }
    *normal = actorP->colNorm;
    return TRUE;
}

/* Returns the last ground collision code when the body has a recorded result. */
BOOL MgActorColCodeGet(MGACTOR *actorP, u32 *code)
{
    COLBODY *body = ColBodyGet(actorP->no);
    u32 attr = body->param.attr & COLBODY_ATTR_COL_RESULT_MASK;
    u32 result = attr;
    if(!result) {
        return FALSE;
    }
    *code = actorP->colGroundAttr;
    return TRUE;
}

/* Pauses a minigame player and disables its collision when it leaves play. */
void MgPlayerDespawn(MGPLAYER *playerP)
{
    KillEffectPlayer(playerP->actor->no);
    MgPlayerAttrSet(playerP, MGPLAYER_ATTR_PAUSE);
    MgPlayerColDisable(playerP->actor);
}

void MgPlayerSpawn(MGPLAYER *playerP, HuVecF *pos)
{
    MgPlayerAttrReset(playerP, MGPLAYER_ATTR_PAUSE);
    MgPlayerColEnable(playerP->actor, pos);
}

/* Disables a player's collision body while its actor is out of play. */
void MgPlayerColDisable(MGACTOR *actorP)
{
    COLBODY *colBody;
    actorP->attr |= 0x1;
    colBody = ColBodyGet(actorP->no);
    colBody->param.attr |= COLBODY_ATTR_COL_OFF;
}

/* Re-enables a player's collision body and places it at the spawn position. */
void MgPlayerColEnable(MGACTOR *actorP, HuVecF *pos)
{
    COLBODY *colBody;
    actorP->attr &= ~0x1;
    colBody = ColBodyGet(actorP->no);
    colBody->param.attr &= ~COLBODY_ATTR_COL_OFF;
    MgActorPosSet(actorP, pos);
}

void MgActorColAttrParamSet(COL_ATTRPARAM *param, u32 polyAttr)
{
    ColAttrParamSet(param, polyAttr);
}

void MgActorColAttrParamGet(COL_ATTRPARAM *param, u32 polyAttr)
{
    ColAttrParamGet(param, polyAttr);
}

void MgPlayerVibAttrSet(MGPLAYER *playerP, u16 attr)
{
    playerP->vibAttr |= attr;
}

u16 MgPlayerVibAttrCheck(MGPLAYER *playerP, u16 attr)
{
    return playerP->vibAttr & attr;
}

void MgPlayerModeAttrSet(MGPLAYER *playerP, u16 attr)
{
    playerP->modeAttr |= attr;
}

void MgPlayerModeAttrReset(MGPLAYER *playerP, u16 attr)
{
    playerP->modeAttr &= ~attr;
}

u16 MgPlayerModeAttrCheck(MGPLAYER *playerP, u16 attr)
{
    return playerP->modeAttr & attr;
}

void MgPlayerAttrSet(MGPLAYER *playerP, u16 attr)
{
    playerP->attr |= attr;
}

void MgPlayerAttrReset(MGPLAYER *playerP, u16 attr)
{
    playerP->attr &= ~attr;
}

u16 MgPlayerAttrCheck(MGPLAYER *playerP, u16 attr)
{
    return playerP->attr & attr;
}

/* Starts an allowed stun unless another stun is active; negative time uses 90 frames. */
BOOL MgPlayerStunSet(MGPLAYER *playerP, int stunType, float angle, int maxTime)
{
    if (!(playerP->actionFlag & MGPLAYER_ACTFLAG_STUN) ||
        MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_STUNOFF)) {
        return FALSE;
    }
    if(playerP->stunType != MGPLAYER_STUN_NONE) {
        return FALSE;
    }
    if(maxTime < 0) {
        maxTime = 90;
    }
    playerP->stunType = stunType;
    playerP->stunAngle = angle;
    playerP->stunTime = maxTime;
    return TRUE;
}

/* Clears active stun and squish presentation before a player is repositioned. */
BOOL MgPlayerModeIdleSet(MGPLAYER *playerP)
{
    if (!(playerP->actionFlag & MGPLAYER_ACTFLAG_STUN) ||
        MgPlayerAttrCheck(playerP, MGPLAYER_ATTR_STUNOFF)) {
        return FALSE;
    }
    if(playerP->stunType != MGPLAYER_STUN_NONE) {
        Hu3DModelDispOn(playerP->actor->mdlId);
        playerP->stunType = MGPLAYER_STUN_NONE;
        playerP->stunTime = 0;
    }
    if(MgPlayerModeAttrCheck(playerP, MGPLAYER_MODEATTR_SQUISH)) {
        Hu3DModelScaleSet(playerP->actor->mdlId, 1, 1, 1);
        MgPlayerModeAttrReset(playerP, MGPLAYER_MODEATTR_SQUISH);
    }
    return TRUE;
}
