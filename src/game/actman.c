/* Manages minigame player and actor movement, collision, animation, and reactions. */
#include "game/mg/actman.h"
#include "game/charman.h"
#include "game/mg/colman.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/frand.h"
#include "game/audio.h"

#include "datanum/charmot.h"

#include "math.h"
#include "string.h"
#include "humath.h"

#define ACTOR_MAX 256
#define PLAYER_MAX 4
#define MGACTOR_ATTR_COL_OFF (1 << 0)
#define MGACTOR_COLMESH_NONE 65535
#define COLBODY_ATTR_COL_RESULT_MASK 0x34000000
#define COLBODY_ATTR_PRESERVED_MASK 0xFEFF0000
#define MGPLAYER_VIBATTR_HIPDROP_LAND (1 << 9)

typedef struct ActManWork_s {
    /* Player state indexed by collision-body number. */
    MGPLAYER *player;
    /* Actor state indexed by collision-body number. */
    MGACTOR *actor;
    /* Temporary collision actors for each player's punch, kick, or hip drop. */
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


static MGPLAYER_MODE_FUNC playerModeFunc[18] = {
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
    if(fn_80087114(player, (1 << 2))) {
        fn_8006A030(player->playerNo, 20, 7, 3);
    } else if(fn_80087114(player, (1 << 0))) {
        fn_8006A030(player->playerNo, 20, 7, 3);
    } else if(fn_80087114(player, (1 << 1))) {
        fn_8006A030(player->playerNo, 20, 7, 3);
    } else if(fn_80087114(player, (1 << 3))) {
        fn_8006A030(player->playerNo, 20, 7, 3);
    } else if(fn_80087114(player, (1 << 4))) {
        fn_8006A030(player->playerNo, 20, 7, 3);
    } else if(fn_80087114(player, MGPLAYER_VIBATTR_HIPDROP_LAND)) {
        fn_8006A030(player->playerNo, 20, 7, 3);
    }
    switch(player->stunType) {
        case 1:
            fn_8006A030(player->playerNo, 20, 7, 3);
            break;

        case 2:
            fn_8006A030(player->playerNo, 20, 7, 3);
            break;

        case 4:
            fn_8006A030(player->playerNo, 20, 7, 3);
            break;
    }
    if(fn_8008717C(player, (1 << 11))) {
        fn_80087168(player, (1 << 11));
        obj->objFunc = NULL;
        omDelObjEx(actmanWork.objman, obj);
    }
}

/* Reports whether the COM stick override was already enabled, then enables it. */
BOOL fn_8007E818(MGPLAYER *playerP)
{
    BOOL wasEnabled = fn_8008717C(playerP, (1 << 0)) ? TRUE : FALSE;
    fn_80087158(playerP, (1 << 0));
    return wasEnabled;
}

void fn_8007E888(MGPLAYER *playerP)
{
    fn_80087168(playerP, (1 << 0));
}

/* Steers a player unless movement is disabled and reports arrival by horizontal distance. */
BOOL fn_8007E8B4(MGPLAYER *playerP, HuVecF *targetPos, float forwardSpeed, float arrivalRadius)
{
    float radiusSquared = 0.001+(arrivalRadius*arrivalRadius);
    float angle;
    HuVecF dir;
    if(fn_8008717C(playerP, (1 << 9))) {
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
BOOL fn_8007EB24(MGACTOR *actorP, HuVecF *targetPos, float forwardSpeed, float arrivalRadius)
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
void fn_8007ED60(MGPLAYER *playerP)
{
    OMOBJ *obj = fn_8003AB38(actmanWork.objman, 3000, 0, 0, -1, ExecVibrate);
    obj->work[0] = (u32)playerP;
}

/* Marks the callback for removal after its next update's vibration checks. */
void fn_8007EDBC(MGPLAYER *playerP)
{
    fn_80087158(playerP, (1 << 11));
}

static void KillEffectPlayer(int playerNo);

/* Changes mode unless unchanged or frozen: kick/hip drop require jump/fall,
 * jump/fall/punch require walking, and other target modes have no source-mode restriction. */
BOOL fn_8007EDE8(MGPLAYER *playerP, int mode)
{
    if(playerP->mode == mode) {
        return FALSE;
    }
    if(playerP->stunType == 6) {
        return FALSE;
    }
    switch(mode) {
        case 4:
        case 5:
            if(playerP->mode != 1 && playerP->mode != 2) {
                return FALSE;
            }
            break;

        case 1:
        case 2:
        case 3:
            if(playerP->mode != 0) {
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
BOOL fn_8007EEBC(MGPLAYER *playerP, int mode)
{
    if(playerP->mode == mode) {
        return FALSE;
    }
    if(playerP->stunType == 6) {
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
    fn_80027CD0(cameraBit, &cameraWorldPos, &cameraWorldUp, &cameraWorldTarget);
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
        COLBODY *body = fn_8007BAB8(bodyIndex);
        if(!(body->param.attr & (1 << 0))) {
            body->param.attr |= (1 << 0);
            body->param.attr |= (1 << 24);
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
    if(!fn_8007D52C()) {
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
        obj = fn_8003AB38(actmanWork.objman, playerNo, 1, 32, -1, NULL);
        playerP->actor = &actmanWork.actor[actorNo];
        playerP->playerNo = playerNo;
        playerP->charNo = lbl_80265728[playerNo].charNo;
        playerP->padNo = lbl_80265728[playerNo].padNo;
        playerP->camBit = camBit;
        playerP->omObj = obj;
        playerP->actionFlag = actionFlag;
        playerP->motNo = 0;
        playerP->stunType = 0;
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
        mdlId = fn_8005C4E4(playerP->charNo, model, motDataNum, obj->mtnId);
        playerP->actor->mdlId = mdlId;
        obj->work[0] = obj->work[1] = obj->work[2] = obj->work[3] = 0;
        fn_800269E8(mdlId, ((1 << 30) | (1 << 0)));
        fn_8005BE80(playerP->charNo, playerP->omObj->mtnId[0]);
        playerP->motNo = 0;
        fn_80026B64(mdlId, camBit);
        return playerP;
    }
}

/* Claims an inactive non-player collision body and initializes its actor state. */
static inline MGACTOR *CreateActor(int mdlId)
{
    MGACTOR *actor;
    int bodyIndex;
    if(!fn_8007D52C()) {
        return NULL;
    }
    for(bodyIndex=PLAYER_MAX; bodyIndex<ACTOR_MAX; bodyIndex++) {
        COLBODY *body = fn_8007BAB8(bodyIndex);
        if(!(body->param.attr & (1 << 0))) {
            body->param.attr |= (1 << 0);
            body->param.attr |= (1 << 24);
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
        fn_80080F38(actmanWork.effPlayer[playerNo]);
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
                if(fn_80087148(playerP2, (1 << 4))) {
                    if(fabsf(playerP1->actor->velY) <= 1000.0f) {
                        if (playerP2->actor->velY < 0 ||
                            fn_80087114(playerP2, (1 << 1))) {
                            if(fn_8008718C(playerP1, 3, 0, -1)) {
                                fn_80087104(playerP1, (1 << 0));
                            }
                        }
                    } else {
                        fn_80087104(playerP1, (1 << 2));
                    }
                }
            } else if(dot < -cos(M_PI/3)) {
                if(!fn_8008717C(playerP1, (1 << 1))) {
                    HuVecF norm = a->normPos;
                    float velY;
                    Mtx rotMtx;

                    if(playerP1->actor->velY > 0) {
                        break;
                    }
                    if(!fn_80087148(playerP1, (1 << 4))) {
                        break;
                    }
                    norm.y = 0;
                    playerP1->actor->velY = GET_ACTOR_VELY(playerP1->actor, 15000);
                    playerP1->actor->rotY += (u32)frandmod(90)-45.0f;
                    if(!fn_80087148(playerP1, ((1 << 2)|(1 << 3)))) {
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
                    fn_80087158(playerP1, (1 << 8)|(1 << 9));
                    fn_80087124(playerP1, ((1 << 2)|(1 << 3)));
                    fn_80087104(playerP1, (1 << 1));

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
            if(!result) {
                break;
            }
            if (fn_80087114(playerP1, (1 << 0) | (1 << 1) |
                                                   (1 << 2))) {
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
            if(!fn_8008718C(playerP1, 1, angle, -1)) {
                return FALSE;
            }
            fn_80087104(playerP2, (1 << 3));
            fn_80087104(playerP1, (1 << 6));
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
            if (!fn_8008718C(playerP1, 2,
                                 actmanWork.actor[b->paramA].rotY, -1)) {
                return FALSE;
            }
            fn_80087104(playerP2, (1 << 4));
            fn_80087104(playerP1, (1 << 7));
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
            if(!fn_8008718C(playerP1, 4, angle, -1)) {
                return FALSE;
            }
            fn_80087104(playerP2, (1 << 5));
            fn_80087104(playerP1, (1 << 8));
            fn_800591C8(playerP1->charNo, MSM_SE_CHARVOICE_BUMP_GRUNT, 0);
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
void fn_800802A8(void)
{
    int effectSlot;
    actmanWork.objman = NULL;
    actmanWork.player = NULL;
    actmanWork.actor = NULL;
    actmanWork.colCylF = FALSE;
    for(effectSlot=4; effectSlot--; ) {
        actmanWork.effPlayer[effectSlot] = NULL;
    }
    fn_8007BAD0();
}

/* Frees actor and player arrays and shuts down the shared collision map. */
void fn_8008033C(void)
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
    fn_8007D2C0();
}

/* Creates the object manager and allocates zeroed player and actor state arrays. */
OMOBJMAN *fn_80080418(void)
{
    actmanWork.objman = fn_8003A8BC(256, 1000);
    fn_80069F1C(actmanWork.objman);
    actmanWork.player =
        HuMemDirectMallocNum(2, PLAYER_MAX * sizeof(MGPLAYER), 0x10000000);
    actmanWork.actor = HuMemDirectMallocNum(2, ACTOR_MAX*sizeof(MGACTOR), 0x10000000);
    memset(actmanWork.player, 0, PLAYER_MAX*sizeof(MGPLAYER));
    memset(actmanWork.actor, 0, ACTOR_MAX*sizeof(MGACTOR));
    actmanWork.colCylF = FALSE;
    return actmanWork.objman;
}

/* Supplies the collision map models and body limit during minigame setup. */
void fn_800804E8(HU3D_MODELID *mdlId, s16 mdlNum, int bodyMax)
{
    ColMapInit(mdlId, mdlNum, bodyMax);
}

/* Replaces this player's stick and button input while preserving Start and Z bits. */
void fn_80080520(MGPLAYER *playerP, int stkX, int stkY, int btnDown, int btn)
{
    lbl_802BFFFC[playerP->padNo] = stkX;
    lbl_802BFFF8[playerP->padNo] = stkY;
    lbl_802C0008[playerP->padNo] &= (PAD_BUTTON_START|PAD_TRIGGER_Z);
    lbl_802C0010[playerP->padNo] &= (PAD_BUTTON_START|PAD_TRIGGER_Z);
    lbl_802C0008[playerP->padNo] |= btnDown;
    lbl_802C0010[playerP->padNo] |= btn;
}

/* Adds collision-body attributes, preserving the manager's active and reset rules. */
void fn_800805CC(MGACTOR *actorP, u32 mask)
{
    COLBODY *body = fn_8007BAB8(actorP->no);
    body->param.attr |= mask&((1 << 24)|0xFFFE);
}

/* Clears selected collision-body attributes while retaining protected manager bits. */
void fn_8008062C(MGACTOR *actorP, u32 mask)
{
    COLBODY *body = fn_8007BAB8(actorP->no);
    body->param.attr &= (~mask|COLBODY_ATTR_PRESERVED_MASK|(1 << 0));
}

/* Returns the selected attributes from this actor's collision body. */
u32 fn_8008068C(MGACTOR *actorP, u32 mask)
{
    COLBODY *body = fn_8007BAB8(actorP->no);
    return body->param.attr & mask;
}

/* Installs only the non-null mode callbacks supplied by a minigame. */
void fn_800806DC(MGPLAYER *playerP, MGPLAYER_MODE_FUNC *funcTbl)
{
    int modeIndex;
    for(modeIndex=18; modeIndex--;) {
        if(funcTbl[modeIndex]) {
            playerP->modeFunc[modeIndex] = funcTbl[modeIndex];
        }
    }
}

static unsigned int defMotDataNum[16+1] = {
    CHARMOT_HSF_c000m1_300, CHARMOT_HSF_c000m1_301, CHARMOT_HSF_c000m1_302, CHARMOT_HSF_c000m1_303,
    CHARMOT_HSF_c000m1_464, CHARMOT_HSF_c000m1_305, CHARMOT_HSF_c000m1_367, CHARMOT_HSF_c000m1_308,
    CHARMOT_HSF_c000m1_310, CHARMOT_HSF_c000m1_309, CHARMOT_HSF_c000m1_315, CHARMOT_HSF_c000m1_316,
    CHARMOT_HSF_c000m1_317, CHARMOT_HSF_c000m1_318, CHARMOT_HSF_c000m1_322, CHARMOT_HSF_c000m1_322,
    0
};

/* Builds a player and collision body; a supplied motion list is adjusted in place. */
MGPLAYER *fn_80080728(s16 playerNo, MGACTOR_PARAM *param, s16 model, u16 camBit, u32 actionFlag,
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
    colParam.attr = param->attr|(1 << 0)|(1 << 24);
    fn_8007D84C(&colParam, player->actor->no);
    player->actor->param = param->param;
    player->actor->type = param->type;
    player->actor->correctHookParam = param->correctHookParam;
    player->actor->correctHook = param->correctHook;
    return player;
}

inline MGPLAYER *fn_80080728(s16 playerNo, MGACTOR_PARAM *param, s16 model, u16 camBit,
                                u32 actionFlag, unsigned int *motDataNum);

/* Creates a player, optionally routing jump mode through the alternate handler. */
MGPLAYER *fn_800808C0(s16 playerNo, MGACTOR_PARAM *param, s16 model, u16 camBit,
                                u32 actionFlag, unsigned int *motDataNum, unsigned int jumpAltFlags)
{
    MGPLAYER *player = fn_80080728(playerNo, param, model, camBit, actionFlag, motDataNum);

    if(!player) {
        return NULL;
    }
    if(jumpAltFlags & 0x1) {
        MGPLAYER_MODE_FUNC modeFunc[18];
        int modeIndex;
        for(modeIndex=18; modeIndex--;) {
            modeFunc[modeIndex] = NULL;
            if(modeIndex == 1) {
                modeFunc[modeIndex] = PlayerModeJumpAlt;
            }
        }
        fn_800806DC(player, modeFunc);
    }
    return player;
}

/* Creates a non-player actor and registers its collision parameters and callback. */
MGACTOR *fn_80080B04(MGACTOR_PARAM *param, int mdlId)
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
    colParam.attr = param->attr|(1 << 0)|(1 << 24);
    fn_8007D84C(&colParam, actor->no);
    actor->param = param->param;
    actor->type = param->type;
    actor->correctHookParam = param->correctHookParam;
    actor->correctHook = param->correctHook;
    return actor;
}

inline MGACTOR *fn_80080B04(MGACTOR_PARAM *param, int mdlId);

/* Sets the polygon mask used when the collision map tests this model. */
void fn_80080D7C(int mdlNo, u32 mask)
{
    fn_8007D118(mdlNo, mask);
}

/* Returns the collision-map model mask; the result is undefined if the map is uninitialized. */
u32 fn_80080DAC(int mdlNo)
{
    return fn_8007D18C(mdlNo);
}

/* Updates the body's collision mask and reapplies its parameters when its index is in range. */
void fn_80080DD4(MGACTOR *actorP, u32 mask)
{
    COLBODY *body = fn_8007D8E0(actorP->no);
    if(!body) {
        return;
    }
    body->param.mask = mask;
    fn_8007D84C(&body->param, actorP->no);
}

/* Returns the body's collision mask, or zero when its index is outside the reserved range. */
u32 fn_80080E38(MGACTOR *actorP)
{
    COLBODY *body = fn_8007D8E0(actorP->no);
    if(!body) {
        return 0;
    }
    return body->param.mask;
}

/* Restores spherical body positioning for subsequent actor position operations. */
void fn_80080E8C(void)
{
    actmanWork.colCylF = FALSE;
    fn_8007D200();
}

/* Selects cylindrical body positioning for subsequent actor position operations. */
void fn_80080EBC(void)
{
    actmanWork.colCylF = TRUE;
    fn_8007D260();
}

/* Sets the collision body's bounce coefficient. */
void fn_80080EEC(MGACTOR *actorP, float bounce)
{
    COLBODY *body = fn_8007BAB8(actorP->no);
    body->param.bounce = bounce;
}

/* Marks the actor's collision body inactive so actor updates skip it. */
void fn_80080F38(MGACTOR *actorP)
{
    COLBODY *body = fn_8007BAB8(actorP->no);
    body->param.attr &= ~(1 << 0);
}

#define GET_STICK_SPEED(playerP, out) do { \
    HuVecF dir; \
    dir.x = lbl_802BFFFC[playerP->padNo]/5.6f; \
    dir.y = 0; \
    dir.z = lbl_802BFFF8[playerP->padNo]/5.6f; \
    out = dir.z*dir.z+(dir.x*dir.x+dir.y*dir.y); \
} while(0)

/* Changes the player's animation only when the requested motion differs. */
static inline void PlayerSetMotion(MGPLAYER *playerP, u16 motNo, float start, float end, u32 attr)
{
    if(playerP->motNo != motNo) {
        fn_8005C2C8(playerP->charNo, playerP->omObj->mtnId[motNo], start, end, attr);
        playerP->motNo = motNo;
    }
}

/* Update handler for unassigned modes: requests walking mode and the idle animation. */
static void PlayerModeDefault(MGPLAYER *playerP)
{
    fn_8007EEBC(playerP, 0);
    PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
}

/* Handles ground movement and starts jumps or attacks from the minigame update. */
static void PlayerModeWalk(MGPLAYER *playerP)
{
    if(playerP->actor->colGroundAttr & 0x407F) {
        if(!fn_8008717C(playerP, (1 << 0))
            && (lbl_802C0008[playerP->padNo] & PAD_BUTTON_A)) {
            if(playerP->actionFlag & (1 << 1)) {
                fn_8007EEBC(playerP, 1);
            }
        } else if(!fn_8008717C(playerP, (1 << 0))
            && (lbl_802C0008[playerP->padNo] & PAD_BUTTON_B)) {
            if(playerP->actionFlag & (1 << 2)) {
                fn_8007EEBC(playerP, 3);
            }
        } else {
            if(playerP->actionFlag & (1 << 0)) {
                float speed2 = playerP->actor->push.x * playerP->actor->push.x +
                               playerP->actor->push.z * playerP->actor->push.z;
                if(speed2 > 25) {
                    PlayerSetMotion(playerP, 2, 0, 5, ((1 << 30) | (1 << 0)));
                } else if(speed2 > 0.001) {
                    PlayerSetMotion(playerP, 1, 0, 5, ((1 << 30) | (1 << 0)));
                } else {
                    PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
                }
            }
        }
    } else {
        if(fabsf(playerP->actor->velY) > 2000) {
            fn_8007EEBC(playerP, 2);
        }
    }
}

/* Advances a normal jump, landing, and permitted midair attacks each player update. */
static void PlayerModeJump(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
            PlayerSetMotion(playerP, 3, 0, 1, 0);
            playerP->actor->velY = GET_ACTOR_VELY(playerP->actor, 25000);
            fn_80087124(playerP, (1 << 4));
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
                        PlayerSetMotion(playerP, 4, 0, 5, 0);
                        fn_80087158(playerP, (1 << 8)|(1 << 9));
                        playerP->timer = 10;
                        playerP->subMode = 3;
                    } else {
                        fn_8007EEBC(playerP, 0);
                        PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
                    }
                } else {
                    if(!fn_8008717C(playerP, (1 << 0))
                        && !(lbl_802C0010[playerP->padNo] & PAD_BUTTON_A)) {
                        playerP->actor->velY = GET_ACTOR_VELY(playerP->actor, 4000);
                        playerP->timer = 0;
                        playerP->subMode = 2;
                    }
                }
            }
            playerP->timer++;
            break;

        case 2:
            PlayerSetMotion(playerP, 3, 0, 1, 0);
            if(((playerP->actor->colGroundAttr & 0x7F)
                && playerP->actor->colNorm.y > 0
                || (playerP->actor->colGroundAttr & 0x4000))
                && playerP->actor->velY < 0.001) {
                float speed;
                GET_STICK_SPEED(playerP, speed);
                if(speed < 25.0f) {
                    PlayerSetMotion(playerP, 4, 0, 5, 0);
                    fn_80087158(playerP, (1 << 8)|(1 << 9));
                    playerP->timer = 10;
                    playerP->subMode = 3;
                } else {
                    fn_8007EEBC(playerP, 0);
                    PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
                    if(!(fn_80062094(playerP->charNo) & 0x10)) {
                        HuVecF pos;
                        fn_80086BC8(playerP->actor, &pos);
                        fn_80026760(playerP->actor->mdlId, pos.x, pos.y, pos.z);
                        fn_8005FC84(playerP->charNo, &pos);
                    }
                }
            } else {
                if(playerP->motNo == 3) {
                    if(!fn_8008717C(playerP, (1 << 0))
                        && !fn_80087148(playerP, ((1 << 2)|(1 << 3)))
                        && ((lbl_802C0008[playerP->padNo] & PAD_BUTTON_A)
                        || (lbl_802C0008[playerP->padNo] & 0x4000))) {
                        if(playerP->actionFlag & (1 << 4)) {
                            fn_8007EEBC(playerP, 5);
                        }
                    } else if(!fn_8008717C(playerP, (1 << 0))
                        && (lbl_802C0008[playerP->padNo] & PAD_BUTTON_B)) {
                        if(playerP->actionFlag & (1 << 3)) {
                            fn_8007EEBC(playerP, 4);
                        }
                    }
                }
            }
            break;

        case 3:
            if (playerP->timer <= 9 &&
                !fn_80087148(playerP, (1 << 1))) {
                fn_80087168(playerP, (1 << 8)|(1 << 9));
            }
            if(playerP->timer > 0) {
                if(!fn_8008717C(playerP, (1 << 0))
                    && (lbl_802C0008[playerP->padNo] & PAD_BUTTON_A)) {
                    playerP->subMode = 0;
                    if(!fn_80087148(playerP, (1 << 1))) {
                        fn_80087168(playerP, (1 << 8)|(1 << 9));
                    }
                    if(playerP->timer >= 6.0f) {
                        if(!(fn_80062094(playerP->charNo) & 0x10)) {
                            HuVecF pos;
                            fn_80086BC8(playerP->actor, &pos);
                            fn_80026760(playerP->actor->mdlId, pos.x, pos.y, pos.z);
                            fn_8005FC84(playerP->charNo, &pos);
                        }
                    }
                }
                playerP->timer--;
            } else {
                fn_8007EEBC(playerP, 0);
                PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
            }
            break;
    }
}

/* Handles falling, landing, and permitted midair attacks each player update. */
static void PlayerModeFall(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
            PlayerSetMotion(playerP, 3, 0, 1, 0);
            fn_80087124(playerP, (1 << 4));
            if(playerP->actor->colGroundAttr & 0x407F) {
                float speed;
                GET_STICK_SPEED(playerP, speed);
                if(speed < 25.0f) {
                    PlayerSetMotion(playerP, 4, 0, 5, 0);
                    fn_80087158(playerP, (1 << 8)|(1 << 9));
                    playerP->timer = 10;
                    playerP->subMode = 1;
                } else {
                    fn_8007EEBC(playerP, 0);
                    PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
                    if(!(fn_80062094(playerP->charNo) & 0x10)) {
                        HuVecF pos;
                        fn_80086BC8(playerP->actor, &pos);
                        fn_80026760(playerP->actor->mdlId, pos.x, pos.y, pos.z);
                        fn_8005FC84(playerP->charNo, &pos);
                    }
                }
            } else {
                if(playerP->motNo == 3) {
                    if(!fn_8008717C(playerP, (1 << 0))
                        && ((lbl_802C0008[playerP->padNo] & PAD_BUTTON_A)
                        || (lbl_802C0008[playerP->padNo] & 0x4000))) {
                        if(playerP->actionFlag & (1 << 4)) {
                            fn_8007EEBC(playerP, 5);
                        }
                    } else if(!fn_8008717C(playerP, (1 << 0))
                        && (lbl_802C0008[playerP->padNo] & PAD_BUTTON_B)) {
                        if(playerP->actionFlag & (1 << 3)) {
                            fn_8007EEBC(playerP, 4);
                        }
                    }
                }
            }
            break;

        case 1:
            if (playerP->timer <= 9 &&
                !fn_80087148(playerP, (1 << 1))) {
                fn_80087168(playerP, (1 << 8)|(1 << 9));
            }
            if(playerP->timer > 0) {
                if(!fn_8008717C(playerP, (1 << 0))
                    && (lbl_802C0008[playerP->padNo] & PAD_BUTTON_A)) {
                    if(!fn_80087148(playerP, (1 << 1))) {
                        fn_80087168(playerP, (1 << 8)|(1 << 9));
                    }
                    if(playerP->timer >= 6.0f) {
                        if(!(fn_80062094(playerP->charNo) & 0x10)) {
                            HuVecF pos;
                            fn_80086BC8(playerP->actor, &pos);
                            fn_80026760(playerP->actor->mdlId, pos.x, pos.y, pos.z);
                            fn_8005FC84(playerP->charNo, &pos);
                        }
                    }
                    fn_8007EEBC(playerP, 1);
                }
                playerP->timer--;
            } else {
                fn_8007EEBC(playerP, 0);
                PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
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
            PlayerSetMotion(playerP, 5, 0, 5, 0);
            playerP->timer = 20;
            fn_80087158(playerP, (1 << 8)|(1 << 9));
            no = playerP->actor->no;
            body = fn_8007D8E0(no);
            param.param = body->param.paramA+256;
            param.type = 2;
            param.height = 120;
            param.radius = 60;
            param.attr = (1 << 0)|(1 << 24)|0xA;
            param.narrowHook = NULL;
            param.correctHook = NULL;
            actmanWork.effPlayer[no] = fn_80080B04(&param, -1);
            fn_80080DD4(actmanWork.effPlayer[no], fn_80080E38(playerP->actor));
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
                fn_80087168(playerP, (1 << 8)|(1 << 9));
                playerP->subMode = 2;
            }
        }
            break;

        case 2:
            fn_8007EEBC(playerP, 0);
            PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
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
            PlayerSetMotion(playerP, 6, 0, 5, 0);
            playerP->timer = 20;
            no = playerP->actor->no;
            body = fn_8007D8E0(no);
            param.param = body->param.paramA+256;
            param.type = 3;
            param.height = 120;
            param.radius = 60;
            param.attr = (1 << 0)|(1 << 24)|0xA;
            param.narrowHook = NULL;
            param.correctHook = NULL;
            actmanWork.effPlayer[no] = fn_80080B04(&param, -1);
            fn_80080DD4(actmanWork.effPlayer[no], fn_80080E38(playerP->actor));
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
                PlayerSetMotion(playerP, 4, 0, 5, 0);
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
                PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
                fn_8007EEBC(playerP, 0);
            }

            break;
    }
}

/* Drives the airborne hip drop and landing impact during player updates. */
static void PlayerModeHipDrop(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
            PlayerSetMotion(playerP, 7, 0, 5, 0);
            playerP->timer = 20;
            fn_80087158(playerP, (1 << 8)|(1 << 9));
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
                body = fn_8007D8E0(no);
                param.param = body->param.paramA+256;
                param.type = 4;
                param.height = 10;
                param.radius = 80;
                param.attr = (1 << 0)|(1 << 24)|0xA;
                param.narrowHook = NULL;
                param.correctHook = NULL;
                actmanWork.effPlayer[no] = fn_80080B04(&param, -1);
                fn_80080DD4(actmanWork.effPlayer[no], fn_80080E38(playerP->actor));
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
                fn_80087104(playerP, MGPLAYER_VIBATTR_HIPDROP_LAND);
                PlayerSetMotion(playerP, 8, 0, 5, 0);
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
                PlayerSetMotion(playerP, 9, 0, 5, 0);
            }
            break;

        case 4:
            if(playerP->timer > 0) {
                playerP->timer--;
            } else {
                if(!fn_80087148(playerP, (1 << 1))) {
                    fn_80087168(playerP, (1 << 8)|(1 << 9));
                }
                fn_8007EEBC(playerP, 0);
                PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
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
                PlayerSetMotion(playerP, 10, 0, 5, 0);
                playerP->actor->rotY = WrapAngle(180+playerP->stunAngle);
            } else {
                PlayerSetMotion(playerP, 11, 0, 5, 0);
                playerP->actor->rotY = playerP->stunAngle;
            }
            playerP->timer = 45;
            fn_80087158(playerP, (1 << 8)|(1 << 9));
            playerP->stunType = 6;
            playerP->subMode = 1;
        }

            break;

        case 1:
            if(playerP->timer > 0) {
                playerP->actor->push.x = 0;
                playerP->actor->push.y = 0;
                if(playerP->motNo == 10) {
                    if(playerP->timer > 25) {
                        playerP->actor->push.z = -(playerP->timer/1.7f)*((playerP->timer-25)/20);
                    }
                } else if(playerP->motNo == 11) {
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
            playerP->stunType = 5;
            fn_80087168(playerP, (1 << 8)|(1 << 9));
            fn_8007EEBC(playerP, 0);
            PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
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
                PlayerSetMotion(playerP, 12, 0, 5, 0);
                playerP->actor->rotY = WrapAngle(180+playerP->stunAngle);
            } else {
                PlayerSetMotion(playerP, 13, 0, 5, 0);
                playerP->actor->rotY = playerP->stunAngle;
            }
            playerP->timer = 45;
            fn_80087158(playerP, (1 << 8)|(1 << 9));
            playerP->stunType = 6;
            playerP->subMode = 1;
        }
            break;

        case 1:
            if(playerP->timer > 0) {
                playerP->actor->push.x = 0;
                playerP->actor->push.y = 0;
                if(playerP->timer > 25) {
                    if(playerP->motNo == 12) {
                        playerP->actor->push.z = -(playerP->timer/1.4f)*((playerP->timer-25)/20);
                    } else if(playerP->motNo == 13) {
                        playerP->actor->push.z = (playerP->timer/1.4f)*((playerP->timer-25)/20);
                    }
                }
                playerP->timer--;
            } else {
                if(playerP->motNo == 12) {
                    PlayerSetMotion(playerP, 14, 0, 5,
                                    0);
                } else if(playerP->motNo == 13) {
                    PlayerSetMotion(playerP, 15, 0, 5,
                                    0);
                }
                playerP->timer = 30;
                playerP->subMode = 2;
            }
            break;

        case 2:
            if(playerP->timer > 0) {
                playerP->timer--;
            } else {
                playerP->stunType = 5;
                fn_80087168(playerP, (1 << 8)|(1 << 9));
                fn_8007EEBC(playerP, 0);
                PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
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
            body = fn_8007BAB8(playerP->actor->no);
            fn_80087158(playerP, (1 << 8)|(1 << 9));
            playerP->timer = 8;
            if(!fn_80087148(playerP, (1 << 0))) {
                fn_80087124(playerP, (1 << 0));
                playerP->squishOldHeight = body->param.height;
                body->param.height = playerP->squishOldHeight*0.5f;
            }
            playerP->stunType = 6;
            playerP->subMode = 1;
            break;

        case 1:
            t = playerP->timer/8;
            scaleXZ = 1.25f-(0.25f*t);
            scaleY = 0.5f+(0.5f*t);
            if(playerP->timer > 0) {
                playerP->timer--;
                fn_800268B0(playerP->actor->mdlId, scaleXZ, scaleY, scaleXZ);
            } else {
                fn_800268B0(playerP->actor->mdlId, 1.25f, 0.5f, 1.25f);
                playerP->subMode = 2;
            }
            break;

        case 2:
            fn_80087168(playerP, (1 << 8)|(1 << 9));
            playerP->stunType = 5;
            playerP->squishTime = 90;
            fn_8007EEBC(playerP, 0);
            PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
            break;
    }
}

/* Hides body collision during a hard squish, then restores the player's shape. */
static void PlayerModeSquishHard(MGPLAYER *playerP)
{
    COLBODY *body;

    switch(playerP->subMode) {
        case 0:
            fn_800805CC(playerP->actor, (1 << 2));
            fn_80087124(playerP, (1 << 1));
            fn_80087158(playerP, (1 << 8)|(1 << 9));
            if(fn_80087148(playerP, (1 << 0))) {
                body = fn_8007BAB8(playerP->actor->no);
                fn_80087134(playerP, (1 << 0));
                body->param.height = playerP->squishOldHeight;
            }
            playerP->timer = 5;
            PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
            playerP->stunType = 6;
            playerP->subMode = 1;
            break;

        case 1:
            if(playerP->timer > 0) {
                float scale;
                playerP->timer--;
                scale = 1+((10*(5-playerP->timer))/100);
                fn_800268B0(playerP->actor->mdlId, scale,
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
                fn_8008062C(playerP->actor, (1 << 2));
                playerP->timer = 100;
                playerP->subMode = 3;
            }
            break;

        case 3:
        {
            float t = playerP->timer/100;
            float angle = 270+((playerP->timer*2880)/100);
            float scale;
            t = (1+(0.5*(t*sin(M_PI*angle/180.0))))-(t*0.45f);
            scale = 1+(0.08f*(playerP->timer-93));
            if(fn_80026A90(playerP->actor->mdlId) & (1 << 0)) {
                fn_80026A44(playerP->actor->mdlId, (1 << 0));
            } else {
                fn_800269E8(playerP->actor->mdlId, (1 << 0));
            }
            if(playerP->timer > 0) {
                if(playerP->timer > 93) {
                    fn_800268B0(playerP->actor->mdlId, scale, t, scale);
                } else {
                    fn_800268B0(playerP->actor->mdlId, 1, t, 1);
                }
                playerP->timer--;
            } else {
                fn_800268B0(playerP->actor->mdlId, 1, 1, 1);
                playerP->subMode = 4;
            }
        }
            break;

        case 4:
            if(fn_80026A90(playerP->actor->mdlId) & (1 << 0)) {
                fn_80026A44(playerP->actor->mdlId, (1 << 0));
            } else {
                fn_800269E8(playerP->actor->mdlId, (1 << 0));
            }
            fn_80087134(playerP, (1 << 1));
            fn_80087168(playerP, (1 << 8)|(1 << 9));
            playerP->stunType = 5;
            fn_8007EEBC(playerP, 0);
            PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
            break;
    }
}

/* Advances the alternate jump and landing when minigame setup enables it. */
static void PlayerModeJumpAlt(MGPLAYER *playerP)
{
    switch(playerP->subMode) {
        case 0:
            fn_80087124(playerP, (1 << 4));
            PlayerSetMotion(playerP, 3, 0, 1, 0);
            playerP->actor->velY = GET_ACTOR_VELY(playerP->actor, 25000);
            playerP->subMode = 1;
            break;

        case 1:
        case 2:
            PlayerSetMotion(playerP, 3, 0, 1, 0);
            if(((playerP->actor->colGroundAttr & 0x7F)
                    && playerP->actor->colNorm.y > 0
                    || (playerP->actor->colGroundAttr & 0x4000))
                    && playerP->actor->velY < 0.001) {
                        float speed = VECSquareMag(&playerP->actor->push);
                        if(speed < 25.0f) {
                            PlayerSetMotion(playerP, 4, 0, 5, 0);
                            fn_80087158(playerP, (1 << 8)|(1 << 9));
                            playerP->timer = 10;
                            playerP->subMode = 3;
                        } else {
                            fn_8007EEBC(playerP, 0);
                            PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
                        }
                    } else {
                        /* This comparison's result is discarded; it does not change motion
                         * or start an attack. */
                        playerP->motNo == 3;
                    }
            break;

        case 3:
            if (playerP->timer <= 9 &&
                !fn_80087148(playerP, (1 << 1))) {
                fn_80087168(playerP, (1 << 8)|(1 << 9));
            }
            if(playerP->timer > 0) {
                if(!fn_8008717C(playerP, (1 << 0))
                    && (lbl_802C0008[playerP->padNo] & PAD_BUTTON_A)) {
                        playerP->subMode = 0;
                        if(!fn_80087148(playerP, (1 << 1))) {
                            fn_80087168(playerP,
                                              (1 << 8) | (1 << 9));
                        }
                    }
                    playerP->timer--;
            } else {
                fn_8007EEBC(playerP, 0);
                PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
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
        actorP->colMesh = MGACTOR_COLMESH_NONE;
        actorP->colObj = NULL;
        actorP->colFace = -1;
        actorP->terminalVelY = 0;
    }
}

/* Updates active minigame players and actors once per minigame frame. */
void fn_800856A8(void)
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

    if(!fn_8007D52C()) {
        return;
    }
    for(i=0; i<PLAYER_MAX; i++) {
        colBody = fn_8007BAB8(i);
        if(colBody->param.attr & (1 << 0)) {
            playerP = &actmanWork.player[i];
            if(fn_8008717C(playerP, (1 << 10))) {
                continue;
            }
            if ((playerP->actionFlag & (1 << 0)) &&
                !fn_8008717C(playerP, (1 << 0))) {
                /* With MOVEOFF, stickVec is not refreshed; speed and optional facing below
                 * reuse a previous loop iteration's vector, or an uninitialized value if none
                 * filled it. */
                if(!fn_8008717C(playerP, (1 << 9))) {
                    stickVec.x = lbl_802BFFFC[playerP->padNo]/5.6f;
                    stickVec.y = 0;
                    stickVec.z = lbl_802BFFF8[playerP->padNo]/5.6f;
                    GetStickMtx(stickMtx, playerP->camBit);
                    MTXMultVec(stickMtx, &stickVec, &stickVec);
                }
                stickSpeed = stickVec.z*stickVec.z+(stickVec.x*stickVec.x+stickVec.y*stickVec.y);
                if(!fn_8008717C(playerP, (1 << 8))) {
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
                if(!fn_8008717C(playerP, (1 << 9))) {
                    playerP->actor->push.x = 0;
                    playerP->actor->push.y = 0;
                    playerP->actor->push.z = sqrtf(stickSpeed);
                }
            } else if (fn_8008717C(playerP, (1 << 9))) {
                playerP->actor->push.x = playerP->actor->push.y = playerP->actor->push.z = 0;
            }
            /* Clear vibration flags before this update's mode and collision reactions
             * generate requests. */
            playerP->vibAttr = 0;
            if(fn_80087148(playerP, (1 << 0))) {
                if (playerP->stunType == 5 ||
                    playerP->stunType == 0) {
                    playerP->squishTime--;
                    if(playerP->squishTime < 0) {
                        if(playerP->squishTime < -20) {
                            colBody = fn_8007BAB8(playerP->actor->no);
                            fn_80087134(playerP, (1 << 0));
                            fn_800268B0(playerP->actor->mdlId, 1, 1, 1);
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
                            fn_800268B0(playerP->actor->mdlId, scaleXZ, scaleY, scaleXZ);
                        }
                    }
                }
            }
            if(fn_80087148(playerP, (1 << 0))) {
                VECScale(&playerP->actor->push, &playerP->actor->push, 0.5f);
            }
            if(fn_80087148(playerP, ((1 << 2)|(1 << 3)))) {
                if(fn_80087148(playerP, (1 << 3))) {
                    if(playerP->actor->velY <= 0) {
                        if(!fn_80087148(playerP, (1 << 1))
                            && playerP->stunType == 0
                            && playerP->mode != 5) {
                            fn_80087134(playerP, (1 << 3));
                            fn_80087168(playerP,
                                              (1 << 8) | (1 << 9));
                        }
                    } else {
                        fn_80087158(playerP, (1 << 8)|(1 << 9));
                    }
                }
                if(playerP->actor->colGroundAttr & 0x407F) {
                    fn_80087134(playerP, ((1 << 2)|(1 << 3)));
                    if (!fn_80087148(playerP, (1 << 1)) &&
                        playerP->mode != 5 &&
                        (playerP->stunType == 0 ||
                         playerP->stunType == 5)) {
                        fn_80087168(playerP, (1 << 8) | (1 << 9));
                    }
                    memset(&playerP->actor->forceB, 0, sizeof(HuVecF));
                }
            }
            if(playerP->actor->colGroundAttr & 0x407F) {
                fn_80087134(playerP, (1 << 4));
            } else if (playerP->actor->velY <
                       -((5 * playerP->actor->gravity) + (5 * playerP->actor->gravity))) {
                fn_80087124(playerP, (1 << 4));
            }
            if(playerP->actionFlag & (1 << 5)) {
                switch(playerP->stunType) {
                    case 0:
                        break;

                    case 1:
                        if(playerP->actor->velY > 0) {
                            playerP->actor->velY = 0;
                        }
                        KillEffectPlayer(playerP->actor->no);
                        fn_8007EEBC(playerP, 6);
                        break;

                    case 2:
                        if(playerP->actor->velY > 0) {
                            playerP->actor->velY = 0;
                        }
                        KillEffectPlayer(playerP->actor->no);
                        fn_8007EEBC(playerP, 7);
                        break;

                    case 3:
                        if(playerP->actor->velY > 0) {
                            playerP->actor->velY = 0;
                        }
                        KillEffectPlayer(playerP->actor->no);
                        fn_8007EEBC(playerP, 8);
                        break;

                    case 4:
                        if(playerP->actor->velY > 0) {
                            playerP->actor->velY = 0;
                        }
                        KillEffectPlayer(playerP->actor->no);
                        fn_8007EEBC(playerP, 9);
                        break;

                    case 5:
                        if(fn_80026A90(playerP->actor->mdlId) & (1 << 0)) {
                            fn_80026A44(playerP->actor->mdlId, (1 << 0));
                        } else {
                            fn_800269E8(playerP->actor->mdlId, (1 << 0));
                        }
                        if(playerP->stunTime > 0) {
                            playerP->stunTime--;
                        } else {
                            fn_80026A44(playerP->actor->mdlId, (1 << 0));
                            playerP->stunType = 0;
                        }
                        break;
                }
            }
            playerP->modeFunc[playerP->mode](playerP);
        }
    }
    fn_8007D534();
    for(i=0; i<ACTOR_MAX; i++) {
        colBody = fn_8007BAB8(i);
        if(colBody->param.attr & (1 << 0)) {
            actorP = &actmanWork.actor[i];
            if(actorP->attr & MGACTOR_ATTR_COL_OFF) {
                continue;
            }
            if(actorP->mdlId >= 0) {
                fn_80026808(actorP->mdlId, 0, actorP->rotY, 0);
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
            fn_8007D6E0(&actorP->pos, i);
            actorP->oldPos = actorP->pos;
        }
    }
    fn_8007DDB8();
    for(i=0; i<ACTOR_MAX; i++) {
        colBody = fn_8007BAB8(i);
        if(colBody->param.attr & (1 << 0)) {
            actorP = &actmanWork.actor[i];
            if(actorP->attr & MGACTOR_ATTR_COL_OFF) {
                continue;
            }
            /* If a correction callback changed the actor position, keep that override and
             * skip collision position/contact updates. This path uses height/2 and does not
             * check mdlId. */
            if (actorP->oldPos.x != actorP->pos.x || actorP->oldPos.y != actorP->pos.y ||
                actorP->oldPos.z != actorP->pos.z) {
                fn_80026760(actorP->mdlId, actorP->pos.x,
                                actorP->pos.y - (colBody->param.height / 2), actorP->pos.z);
            } else {
                fn_8007D7B4(&actorP->pos, i);
                if(actorP->mdlId >= 0) {
                    fn_80086BC8(actorP, &pos);
                    fn_80026760(actorP->mdlId, pos.x, pos.y, pos.z);
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

/* Queries a map segment when collision data is ready. A hit up to 0.001 later in segment
* fraction than the current best can replace it, including beyond the segment endpoint. */
BOOL fn_80086910(HuVecF *pos1, HuVecF *pos2, u32 mask, MGACTOR_COLMAP_POLY *outPoly)
{
    if(!fn_8007D52C()) {
        return FALSE;
    }
    return fn_8007D91C(pos1, pos2, mask, &outPoly->pos, &outPoly->code, &outPoly->mdlNo,
                         &outPoly->obj, &outPoly->triNo);
}

/* Attempts stun/shape and walking resets, ignoring failures, then requests idle motion
 * and moves the actor even if stun or mode remains unchanged. */
void fn_8008697C(MGPLAYER *playerP, HuVecF *pos)
{
    fn_80087218(playerP);
    fn_8007EEBC(playerP, 0);
    PlayerSetMotion(playerP, 0, 0, 5, ((1 << 30) | (1 << 0)));
    fn_80086AA8(playerP->actor, pos);
}

/* Moves an actor and marks its collision body for position synchronization. */
void fn_80086AA8(MGACTOR *actorP, HuVecF *pos)
{
    COLBODY *colBody = fn_8007BAB8(actorP->no);
    colBody->param.attr |= (1 << 24);
    fn_80086B04(actorP, pos);
}

/* Stores the actor's center from its base position; the collision buffers update during
 * MgActorExec. */
void fn_80086B04(MGACTOR *actorP, HuVecF *pos)
{
    COLBODY *colBody;
    if(actorP->no < 0) {
        return;
    }
    colBody = fn_8007BAB8(actorP->no);
    if(colBody->param.attr & (1 << 0)) {
        actorP->pos = *pos;
        if(actmanWork.colCylF) {
            actorP->pos.y += colBody->param.height/2;
        } else {
            actorP->pos.y += colBody->param.radius;
        }
    }
}

/* Gets the actor's base position from its stored center using the collision body's shape. */
void fn_80086BC8(MGACTOR *actorP, HuVecF *pos)
{
    COLBODY *colBody;
    if(actorP->no < 0) {
        return;
    }
    colBody = fn_8007BAB8(actorP->no);
    if(colBody->param.attr & (1 << 0)) {
        *pos = actorP->pos;
        if(actmanWork.colCylF) {
            pos->y -= colBody->param.height/2;
        } else {
            pos->y -= colBody->param.radius;
        }
    }
}

void fn_80086C8C(MGACTOR *actorP, HuVecF *push)
{
    actorP->push = *push;
}

void fn_80086CA8(MGACTOR *actorP, float rotY)
{
    actorP->rotY = rotY;
}

void fn_80086CB0(MGACTOR *actorP, float *rotY)
{
    *rotY = actorP->rotY;
}

void fn_80086CBC(MGACTOR *actorP, float gravity)
{
    actorP->gravity = gravity;
}

void fn_80086CC4(MGACTOR *actorP, float velY)
{
    actorP->velY = velY;
}

void fn_80086CCC(MGACTOR *actorP, HuVecF *vel)
{
    actorP->vel = *vel;
}

/* Returns the last contacted mesh when the body has a recorded collision result. */
BOOL fn_80086CE8(MGACTOR *actorP, int *mesh)
{
    COLBODY *body = fn_8007BAB8(actorP->no);
    u32 attr = body->param.attr & COLBODY_ATTR_COL_RESULT_MASK;
    u32 result = attr;
    if(!result) {
        return FALSE;
    }
    *mesh = actorP->colMesh;
    return TRUE;
}

/* Returns the last contact normal when the body has a recorded collision result. */
BOOL fn_80086D54(MGACTOR *actorP, HuVecF *normal)
{
    COLBODY *body = fn_8007BAB8(actorP->no);
    u32 attr = body->param.attr & COLBODY_ATTR_COL_RESULT_MASK;
    u32 result = attr;
    if(!result) {
        return FALSE;
    }
    *normal = actorP->colNorm;
    return TRUE;
}

/* Returns the last ground collision code when the body has a recorded result. */
BOOL fn_80086DD0(MGACTOR *actorP, u32 *code)
{
    COLBODY *body = fn_8007BAB8(actorP->no);
    u32 attr = body->param.attr & COLBODY_ATTR_COL_RESULT_MASK;
    u32 result = attr;
    if(!result) {
        return FALSE;
    }
    *code = actorP->colGroundAttr;
    return TRUE;
}

/* Pauses a minigame player and disables its collision when it leaves play. */
void fn_80086E3C(MGPLAYER *playerP)
{
    KillEffectPlayer(playerP->actor->no);
    fn_80087158(playerP, (1 << 10));
    fn_80086F44(playerP->actor);
}

void fn_80086F00(MGPLAYER *playerP, HuVecF *pos)
{
    fn_80087168(playerP, (1 << 10));
    fn_80086FA0(playerP->actor, pos);
}

/* Disables a player's collision body while its actor is out of play. */
void fn_80086F44(MGACTOR *actorP)
{
    COLBODY *colBody;
    actorP->attr |= MGACTOR_ATTR_COL_OFF;
    colBody = fn_8007BAB8(actorP->no);
    colBody->param.attr |= (1 << 4);
}

/* Re-enables a player's collision body and places it at the spawn position. */
void fn_80086FA0(MGACTOR *actorP, HuVecF *pos)
{
    COLBODY *colBody;
    actorP->attr &= ~MGACTOR_ATTR_COL_OFF;
    colBody = fn_8007BAB8(actorP->no);
    colBody->param.attr &= ~(1 << 4);
    fn_80086AA8(actorP, pos);
}

void fn_800870A4(COL_ATTRPARAM *param, u32 polyAttr)
{
    fn_8007D550(param, polyAttr);
}

void fn_800870D4(COL_ATTRPARAM *param, u32 polyAttr)
{
    fn_8007D618(param, polyAttr);
}

void fn_80087104(MGPLAYER *playerP, u16 attr)
{
    playerP->vibAttr |= attr;
}

u16 fn_80087114(MGPLAYER *playerP, u16 attr)
{
    return playerP->vibAttr & attr;
}

void fn_80087124(MGPLAYER *playerP, u16 attr)
{
    playerP->modeAttr |= attr;
}

void fn_80087134(MGPLAYER *playerP, u16 attr)
{
    playerP->modeAttr &= ~attr;
}

u16 fn_80087148(MGPLAYER *playerP, u16 attr)
{
    return playerP->modeAttr & attr;
}

void fn_80087158(MGPLAYER *playerP, u16 attr)
{
    playerP->attr |= attr;
}

void fn_80087168(MGPLAYER *playerP, u16 attr)
{
    playerP->attr &= ~attr;
}

u16 fn_8008717C(MGPLAYER *playerP, u16 attr)
{
    return playerP->attr & attr;
}

/* Starts an allowed stun unless another stun is active; negative time uses 90 frames. */
BOOL fn_8008718C(MGPLAYER *playerP, int stunType, float angle, int maxTime)
{
    if (!(playerP->actionFlag & (1 << 5)) ||
        fn_8008717C(playerP, (1 << 2))) {
        return FALSE;
    }
    if(playerP->stunType != 0) {
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

/* Clears stun state and light-squish presentation unless stun actions are disabled.
* Clearing the squish flag restores model scale but leaves collision-body height unchanged. */
BOOL fn_80087218(MGPLAYER *playerP)
{
    if (!(playerP->actionFlag & (1 << 5)) ||
        fn_8008717C(playerP, (1 << 2))) {
        return FALSE;
    }
    if(playerP->stunType != 0) {
        fn_80026A44(playerP->actor->mdlId, (1 << 0));
        playerP->stunType = 0;
        playerP->stunTime = 0;
    }
    if(fn_80087148(playerP, (1 << 0))) {
        fn_800268B0(playerP->actor->mdlId, 1, 1, 1);
        fn_80087134(playerP, (1 << 0));
    }
    return TRUE;
}
