/* Updates pillar models and player actors during the microphone round. */
#include "REL/m670dll.h"
#include "game/charman.h"
#include "game/mg/seqman.h" /* Setup and round updates place a pillar and its collision model at the
                             * requested height. */
void fn_1_22F0(int pillarNo, float height)
{
    HuVecF pos = lbl_1_bss_10.pillarPos[pillarNo];
    pos.y = height;
    Hu3DModelPosSetV(lbl_1_bss_10.pillarModel[pillarNo], &pos);
    Hu3DModelPosSetV(lbl_1_bss_10.collisionModel[pillarNo], &pos);
}
/* MgPlayerCreate uses this hook in sequence mode 5 to add 8 times the ground-face normal's x and z
 * components to the actor position when the ground attribute has bit 0x80. */
void fn_1_2384(MGACTOR *actor, int playerNo)
{
    int unusedPlayerNo = playerNo; /* Copied from the hook argument but never used. */
    HuVecF normal;
    HuVecF pos;
    if ((actor->colGroundAttr & 0x80) && MgSeqModeGet() == 5) {
        normal = *(HuVecF *)((HSF_FACE *)actor->colObj->mesh.face->data)[actor->colFace].nbt;
        MgActorPosGet(actor, &pos);
        pos.x += 8.0f * normal.x;
        pos.z += 8.0f * normal.z;
        MgActorPosSetRaw(actor, &pos);
    }
}
/* Sequence, CPU, and actor updates use this to change a player's motion or retire that player. */
void fn_1_2460(int playerNo, int state)
{
    MGPLAYER *player;
    OMOBJ *obj;
    player = lbl_1_bss_10.players[playerNo];
    obj = lbl_1_bss_10.playerObjects[playerNo];
    switch (state) {
    case 1:
        CharMotionShiftSet(lbl_1_bss_10.characterNo[playerNo], player->omObj->mtnId[0],
            0.0f, 6.0f, HU3D_MOTATTR_LOOP | HU3D_ATTR_DISPOFF);
        break;
    case 2:
        CharMotionShiftSet(lbl_1_bss_10.characterNo[playerNo], player->omObj->mtnId[7],
            0.0f, 6.0f, 0);
        break;
    case 3:
        CharMotionShiftSet(lbl_1_bss_10.characterNo[playerNo], player->omObj->mtnId[8],
            0.0f, 6.0f, HU3D_MOTATTR_LOOP | HU3D_ATTR_DISPOFF);
        break;
    case 4:
        CharMotionShiftSet(lbl_1_bss_10.characterNo[playerNo], player->omObj->mtnId[9],
            0.0f, 6.0f, HU3D_MOTATTR_LOOP | HU3D_ATTR_DISPOFF);
        break;
    case 5:
        CharMotionShiftSet(lbl_1_bss_10.characterNo[playerNo], player->omObj->mtnId[0],
            0.0f, 6.0f, HU3D_MOTATTR_LOOP | HU3D_ATTR_DISPOFF);
        break;
    case 6:
        MgPlayerDespawn(player);
        Hu3DModelAttrSet(player->actor->mdlId, HU3D_ATTR_DISPOFF);
        break;
    }
    lbl_1_bss_10.playerState[playerNo] = state;
}
/* The solo player's per-frame object callback advances the call animation and moves the result
 * model. */
void fn_1_2690(OMOBJ *obj)
{
    int playerNo = obj->work[0];
    MGPLAYER *player = lbl_1_bss_10.players[playerNo];
    HuVecF pos;
    switch (lbl_1_bss_10.playerState[playerNo]) {
    case 2:
        /* Both shift checks pass (mdlId < 0), so Hu3DMotionShiftIDGet queries model 0 or 1 rather
         * than this actor's model. */
        if (Hu3DMotionShiftIDGet(player->actor->mdlId < 0)
            && Hu3DMotionEndCheck(player->actor->mdlId)
            && player->omObj->mtnId[7] == Hu3DMotionIDGet(player->actor->mdlId)) {
            fn_1_2460(playerNo, 3);
            fn_1_2A24(4);
        }
        break;
    case 3:
        if (Hu3DMotionShiftIDGet(player->actor->mdlId < 0)) {
            /* This state calls Hu3DMotionEndCheck when a shift motion exists but ignores its
             * completion result. */
            Hu3DMotionEndCheck(player->actor->mdlId);
        }
        break;
    case 4:
        Hu3DModelPosGet(lbl_1_bss_10.models[2], &pos);
        pos.y -= 4.0f;
        Hu3DModelPosSetV(lbl_1_bss_10.models[2], &pos);
        Hu3DMotionCalc(lbl_1_bss_10.models[2]);
        Hu3DModelObjPosGet(lbl_1_bss_10.models[2], "P00", &pos);
        Hu3DModelPosSetV(lbl_1_bss_10.players[lbl_1_bss_10.soloPlayer]->actor->mdlId, &pos);
        break;
    }
}
/* Each non-solo player's callback retires that player after falling below the playfield. */
void fn_1_28E8(OMOBJ *obj)
{
    int playerNo = obj->work[0];
    MGPLAYER *player = lbl_1_bss_10.players[playerNo];
    HuVecF pos;
    switch (lbl_1_bss_10.playerState[playerNo]) {
    case 5:
        pos = player->actor->pos;
        if (pos.y < -1000.0f) {
            fn_1_2460(playerNo, 6);
            lbl_1_bss_10.remainingPlayers &= ~(1 << playerNo);
            OSReport("pid ... %d : retire\n", playerNo);
        }
        break;
    }
}
