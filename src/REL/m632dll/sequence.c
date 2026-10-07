/* Sequence callbacks for M632 setup, play, and results. */
#include "REL/m632dll.h"

MGSEQ_PARAM lbl_1_data_0 = {
    0,
    0,
    fn_1_21C,
    fn_1_5A8,
    fn_1_173C,
    fn_1_17C0,
    fn_1_2368,
    fn_1_2574,
    fn_1_2E04,
    fn_1_2F88,
    fn_1_2F8C,
};

static char lbl_1_data_28[13] = "EDpattern1P2";

static char lbl_1_data_35[13] = "EDpattern2P2";

static char lbl_1_data_42[13] = "EDpattern2P3";

static char lbl_1_data_4F[13] = "EDpattern3P2";

static char lbl_1_data_5C[13] = "EDpattern3P3";

static char lbl_1_data_69[15] = "EDpattern3P4";

/* Called by fn_1_173C to start music when no stream is active and the message state allows it. */
s32 fn_1_A0(s32 parameter0, s32 parameter1)
{
    s32 value2;

    value2 = parameter0;
    if ((value2 == -1) && ((s32) (GameMesStatGet(MgSeqGameMesIdGet()) & 16) != 0)) {
        value2 = HuAudBGMPlay((s16) parameter1);
    }
    return value2;
}

/* Called by fn_1_2368 at frame 0 to fade out an active stream; -1 means none is playing. */
void fn_1_104(s32 parameter0)
{
    if (parameter0 != -1) {
        HuAudSStreamFadeOut(parameter0, 100);
    }
}

/* Called during the intro and play updates to pan a character's voice from the player's screen position. */
void fn_1_140(s32 parameter0, s32 parameter1)
{
    Point3d localValue3;
    Point3d localValue1;
    s32 value2;

    localValue3 = lbl_1_bss_0.players[parameter0]->actor->pos;
    Hu3D3Dto2D(&localValue3, (s16) parameter1, &localValue1);
    value2 = (s32) localValue1.x;
    value2 /= 5;
    if ((s32) value2 < 32) {
        value2 = 32;
    } else if ((s32) value2 > 96) {
        value2 = 96;
    }
    CharModelVoicePanSet((s16) lbl_1_bss_0.charNo[parameter0], (s16) value2);
}

/* Sequence callback registered in lbl_1_data_0; runs the actors and advances to the next mode. */
void fn_1_21C(s16 mode, s16 frameNo)
{
    MgActorExec();
    MgSeqModeNext();
}

/* Per-frame object callback added by fn_1_5A8 for a group-1 player; follows its model hook until motion ends. */
void fn_1_240(OMOBJ *obj)
{
    f32 localValue6[3][4];
    Point3d localValue5;
    Point3d localValue4;
    Point3d localValue2;
    Point3d localValue0;
    MGPLAYER *player;
    s32 index;

    player = (MGPLAYER *) obj->work[0];
    index = obj->work[1];
    Hu3DModelObjMtxGet(lbl_1_bss_0.playerModels[index], lbl_1_data_108[index], localValue6);
    Hu3DMtxTransGet(localValue6, &localValue5);
    Hu3DMtxRotGet(localValue6, &localValue4);
    Hu3DMtxScaleGet(localValue6, &localValue2);
    Hu3DModelPosSetV(player->actor->mdlId, &localValue5);
    Hu3DModelRotSetV(player->actor->mdlId, &localValue4);
    Hu3DModelScaleSetV(player->actor->mdlId, &localValue2);
    Hu3DModelObjMtxGet(lbl_1_bss_0.playerModels[index], lbl_1_data_108[index], localValue6);
    Hu3DMtxTransGet(localValue6, &localValue0);
    if (localValue0.y >= 2.0f) {
        CharModelVoiceFlagSet(lbl_1_bss_0.players[index]->charNo, 0);
    }
    if (Hu3DMotionEndCheck(lbl_1_bss_0.playerModels[index]) != 0) {
        Hu3DModelAttrSet(player->actor->mdlId, HU3D_ATTR_DISPOFF);
        lbl_1_bss_0.activeMask &= ~(1 << index);
        obj->objFunc = NULL;
    }
}

/* Per-frame object callback added by fn_1_5A8 for a group-0 player; follows its hook and shadow through the intro motion. */
void fn_1_3E0(OMOBJ *obj)
{
    Point3d localValue4;
    Point3d localValue2;
    Point3d localValue0;
    MGPLAYER *player;
    s16 model;
    s32 index;

    player = (MGPLAYER *) obj->work[0];
    index = obj->work[1];
    Hu3DModelObjPosGet(lbl_1_bss_0.playerModels[index], lbl_1_data_108[index], &localValue4);
    Hu3DModelPosSetV(player->actor->mdlId, &localValue4);
    localValue2.x = 0.0f;
    localValue2.y = 0.0f;
    localValue2.z = 1.0f;
    localValue0 = localValue4;
    localValue4.y = -2600.0f;
    localValue0.y = 100.0f;
    Hu3DShadowMultiPosSet(&localValue4, &localValue2, &localValue0, 1);
    if (Hu3DMotionEndCheck(lbl_1_bss_0.playerModels[index]) != 0) {
        model = player->actor->mdlId;
        Hu3DMotionOverlayReset(model);
        Hu3DMotionSet(model, player->omObj->mtnId[12]);
        Hu3DMotionTimeSet(model, 39.0f);
        Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
        Hu3DSubMotionSet(model, player->omObj->mtnId[14], 0.0f);
        Hu3DModelAttrSet(model, HU3D_MOTATTR_SHIFT_PAUSE);
        lbl_1_bss_0.activeMask &= ~(1 << index);
        obj->objFunc = NULL;
    }
}

/* Opening sequence callback registered in lbl_1_data_0; on frame 0 it sets up the intro, then drops players into the arena. */
void fn_1_5A8(s16 mode, s16 frameNo)
{
    int index;
    MGPLAYER *player;
    int counter;
    int soundId;
    int done;
    OMOBJ *soloObj;
    OMOBJ *otherObj;
    int charNo;
    char *hookName;
    f32 temporaryFloat4, temporaryFloat3, temporaryFloat2, temporaryFloat1, temporaryFloat0;

    if (frameNo == 0) {
        MgSeqModeChangeOff();
        Hu3DCameraMotionStart(lbl_1_bss_0.cameraMotions[0], 1U);
        counter = 0;
        index = 0;
        while (index < 4) {
            player = lbl_1_bss_0.players[index];
            MgPlayerDespawn(player);
            Hu3DMotionTimeSet(lbl_1_bss_0.playerModels[index], 0.0f);
            if (lbl_1_bss_0.group[index] == 0) {
                CharMotionSet(player->charNo, player->omObj->mtnId[1]);
                Hu3DMotionOverlaySet(player->actor->mdlId, player->omObj->mtnId[13]);
                Hu3DModelAttrSet(player->actor->mdlId, HU3D_MOTATTR_OVL_LOOP);
                hookName = CharModelItemHookGet(player->charNo, 2, 0);
                Hu3DModelHookSet(player->actor->mdlId, hookName, lbl_1_bss_0.hookModelId);
                soloObj = omAddObjEx(lbl_1_bss_0.objectManager, 32730, 0U, 0U, 0, fn_1_3E0);
                soloObj->work[0] = (u32) player;
                soloObj->work[1] = 0;
            } else {
                CharMotionSet(player->charNo, player->omObj->mtnId[5]);
                otherObj = omAddObjEx(lbl_1_bss_0.objectManager, 32730, 0U, 0U, 0, fn_1_240);
                otherObj->work[0] = (u32) player;
                otherObj->work[1] = counter + 1;
                counter += 1;
            }
            Hu3DModelCameraSet(player->actor->mdlId, 1U);
            index += 1;
        }
        lbl_1_bss_0.activeMask = 15;
        lbl_1_bss_0.phase = 1;
        lbl_1_bss_0.frame = 0;
    }
    switch (lbl_1_bss_0.phase) {
    case 1:
        done = 1;
        index = 0;
        while (index < 4) {
            done = lbl_1_bss_0.activeMask == 0;
            index += 1;
        }
        if ((u32) lbl_1_bss_0.frame == 98U) {
            HuAudFXPlay(1848);
        }
        if ((u32) lbl_1_bss_0.frame == 20U) {
            int voiceA[14] = {177,147,327,447,387,57,417,87,357,297,117,297,237,207};
            int voiceB[14] = {573,549,693,789,741,477,765,501,717,669,525,669,621,597};
            index = 0;
            while (index < 4) {
                if (lbl_1_bss_0.group[index] == 1) {
                    charNo = lbl_1_bss_0.charNo[index];
                    soundId = 581;
                    if (soundId < 573) {
                        soundId -= 177;
                        soundId += voiceA[charNo];
                    } else {
                        soundId -= 573;
                        soundId += voiceB[charNo];
                    }
                    HuAudFXPlay(soundId);
                }
                index += 1;
            }
        }
        if ((u32) lbl_1_bss_0.frame == 112U) {
            index = 0;
            while (index < 4) {
                if (lbl_1_bss_0.group[index] == 1) {
                    CharFXPlay(lbl_1_bss_0.charNo[index], 576);
                }
                index += 1;
            }
        }
        if (done != 0) {
            Hu3DCameraMotionStart(lbl_1_bss_0.cameraMotions[2], 2U);
            Hu3DModelAttrReset(lbl_1_bss_0.sceneModels[1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(lbl_1_bss_0.sceneModels[2], HU3D_ATTR_DISPOFF);
            counter = 0;
            index = 0;
            while (index < 4) {
                player = lbl_1_bss_0.players[index];
                if (lbl_1_bss_0.group[index] == 1) {
                    MgActorPosSet((MGACTOR *) lbl_1_bss_0.players[index], &lbl_1_bss_0.positions[counter]);
                    MgActorPosSetRaw((MGACTOR *) lbl_1_bss_0.players[index], &lbl_1_bss_0.positions[counter]);
                    Hu3DModelPosSetV(player->actor->mdlId, &lbl_1_bss_0.positions[counter]);
                    {
                    Point3d localValue21 = {100000.0f, 0.0f, 0.0f};
                    Point3d localValue19 = {0.0f, 0.0f, 0.0f};
                    Point3d localValue17 = {1.0f, 1.0f, 1.0f};
                    Mtx matrix;
                    PSMTXIdentity(matrix);
                    Hu3DModelMtxSet(player->actor->mdlId, &matrix);
                    Hu3DModelRotSetV(player->actor->mdlId, &localValue19);
                    Hu3DModelScaleSetV(player->actor->mdlId, &localValue17);
                    Hu3DModelAttrReset(player->actor->mdlId, HU3D_ATTR_DISPOFF);
                    Hu3DModelShadowReset(lbl_1_bss_0.players[index]->actor->mdlId);
                    Hu3DModelCameraSet(player->actor->mdlId, 2U);
                    player->camBit = 2;
                    }
                    counter += 1;
                }
                index += 1;
            }
            lbl_1_bss_0.phase = 2;
            lbl_1_bss_0.frame = 0;
        } else {
            lbl_1_bss_0.frame += 1;
        }
        break;
    case 2: {
        Point3d localValue15, localValue13, localValue11;
        temporaryFloat3 = (f32) (u32) lbl_1_bss_0.frame / 60.0f;
        temporaryFloat4 = 640.0f + (-430.0f * temporaryFloat3);
        temporaryFloat2 = 640.0f * temporaryFloat3;
        temporaryFloat1 = ((640.0f + (-414.0f * temporaryFloat3)) / 2.0f) - 320.0f;
        temporaryFloat0 = (640.0f - ((3.0f * temporaryFloat2) / 4.0f)) - (8.0f * temporaryFloat3);
        localValue15.x = localValue15.z = 0.0f;
        localValue13.x = localValue13.y = 0.0f;
        localValue13.z = 1.0f;
        localValue11.x = localValue11.z = 0.0f;
        localValue11.y = 100.0f;
        localValue15.y = -2600.0f + ((1600.0f * (f32) (u32) lbl_1_bss_0.frame) / 60.0f);
        Hu3DShadowMultiPosSet(&localValue15, &localValue13, &localValue11, 1);
        Hu3DCameraViewportSet(1, temporaryFloat1, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        if (temporaryFloat4 > 2.0f) {
            Hu3DCameraScissorSet(1, 0U, 0U, (u32) temporaryFloat4 - 2, 480U);
        } else {
            Hu3DCameraScissorSet(1, 0U, 0U, 0U, 480U);
        }
        if ((u32) lbl_1_bss_0.frame >= 1U) {
            Hu3DCameraViewportSet(2, temporaryFloat0, 0.0f, temporaryFloat2, 480.0f, 0.0f, 1.0f);
            if (temporaryFloat4 > 2.0f) {
                Hu3DCameraScissorSet(2, (u32) temporaryFloat4 + 2, 0U, 638 - (u32) temporaryFloat4, 480U);
            } else {
                Hu3DCameraScissorSet(2, 0U, 0U, 0U, 480U);
            }
        }
        lbl_1_bss_0.frame += 1;
        if ((u32) lbl_1_bss_0.frame >= 60U) {
            counter = 0;
            index = 0;
            while (index < 4) {
                player = lbl_1_bss_0.players[index];
                Hu3DModelHookReset(lbl_1_bss_0.playerModels[index]);
                if (lbl_1_bss_0.group[index] == 1) {
                    CharModelVoiceFlagSet(lbl_1_bss_0.players[index]->charNo, 1);
                    MgActorPosSet((MGACTOR *) lbl_1_bss_0.players[index], &lbl_1_bss_0.positions[counter]);
                    MgActorPosSetRaw((MGACTOR *) lbl_1_bss_0.players[index], &lbl_1_bss_0.positions[counter]);
                    Hu3DModelPosSetV(player->actor->mdlId, &lbl_1_bss_0.positions[counter]);
                    CharMotionShiftSet(player->charNo, player->omObj->mtnId[10], 0.0f, 0.0f, 0U);
                    Hu3DModelShadowSet(lbl_1_bss_0.players[index]->actor->mdlId);
                    counter += 1;
                }
                index += 1;
            }
            lbl_1_bss_0.activeMask = 15;
            lbl_1_bss_0.phase = 3;
            lbl_1_bss_0.frame = 0;
        }
        break;
    }
    case 3: {
        Point3d localValue9;
        done = 1;
        if ((u32) lbl_1_bss_0.frame == 30U) {
            index = 0;
            while (index < 4) {
                if (lbl_1_bss_0.group[index] == 1) {
                    CharModelVoiceFlagSet(lbl_1_bss_0.charNo[index], 0);
                }
                index += 1;
            }
        }
        index = 0;
        while (index < 4) {
            if (lbl_1_bss_0.group[index] == 1) {
                player = lbl_1_bss_0.players[index];
                Hu3DModelPosGet(player->actor->mdlId, &localValue9);
                localValue9.y -= 46.0f;
                if (localValue9.y <= 0.0f) {
                    localValue9.y = 0.0f;
                }
                MgActorPosSet((MGACTOR *) player, &localValue9);
                Hu3DModelPosSetV(player->actor->mdlId, &localValue9);
                if (localValue9.y <= 0.0f) {
                    if ((s32) (lbl_1_bss_0.activeMask & (1 << index)) != 0) {
                        lbl_1_bss_0.activeMask &= ~(1 << index);
                        CharModelLandDustCreate(lbl_1_bss_0.charNo[index], &localValue9);
                        CharMotionShiftSet(player->charNo, player->omObj->mtnId[11], 0.0f, 6.0f, HU3D_MOTATTR_LOOP);
                    }
                } else {
                    done = 0;
                }
            }
            index += 1;
        }
        if (done != 0) {
            lbl_1_bss_0.activeMask = 0;
            index = 0;
            while (index < 4) {
                omVibrate(index, 20, 4, 4);
                if (lbl_1_bss_0.group[index] == 1) {
                    lbl_1_bss_0.activeMask |= 1 << index;
                    CharModelVoiceFlagSet(lbl_1_bss_0.charNo[index], 1);
                }
                index += 1;
            }
            lbl_1_bss_0.phase = 4;
            lbl_1_bss_0.frame = 0;
        } else {
            lbl_1_bss_0.frame += 1;
        }
        break;
    }
    case 4:
        if ((u32) lbl_1_bss_0.frame == 60U) {
            index = 0;
            while (index < 4) {
                if ((s32) (lbl_1_bss_0.activeMask & (1 << index)) != 0) {
                    player = lbl_1_bss_0.players[index];
                    CharMotionShiftSet(player->charNo, *player->omObj->mtnId, 0.0f, 6.0f, HU3D_MOTATTR_LOOP);
                }
                index += 1;
            }
            MgSeqModeChangeOn();
            lbl_1_bss_0.phase = 5;
            lbl_1_bss_0.frame = 0;
        } else {
            lbl_1_bss_0.frame += 1;
        }
        break;
    }
    MgActorExec();
    for (index = 0; index < 4; index++) {
        switch (lbl_1_bss_0.phase) {
        case 2:
        case 3:
        case 4:
            if (lbl_1_bss_0.group[index] == 0) {
                fn_1_140(index, 1);
            } else {
                fn_1_140(index, 2);
            }
            break;
        default:
            fn_1_140(index, 1);
            break;
        }
    }
}

/* Sequence callback registered in lbl_1_data_0; starts background music when allowed and runs the actors. */
void fn_1_173C(s16 mode, s16 frameNo)
{
    lbl_1_bss_0.stream = fn_1_A0(lbl_1_bss_0.stream, 73);
    MgActorExec();
}

/* Play-mode callback registered in lbl_1_data_0; on frame 0 starts the round, then updates control, tilt, collisions, sounds, and timer. */
void fn_1_17C0(s16 mode, s16 frameNo)
{
    Point3d localValue20;
    Point3d localValue18;
    Point3d localValue16;
    Point3d localValue14;
    Point3d localValue12;
    Point3d localValue10;
    Point3d localValue8;
    Point3d localValue7;
    f32 localValue1;
    s32 index;
    s32 panA;
    s32 panB;
    MGPLAYER *player;
    s32 stickX;
    s32 stickY;
    MGPLAYER *controller;
    s16 model;
    MGACTOR *actor;
    s32 soundA;
    s32 soundB;
    f32 angle;
    f32 motionLength;
    f32 magnitude;
    f32 maximum;
    f32 rotY;

    if (frameNo == 0) {
        lbl_1_bss_0.activeMask = 15;
        index = 0;
        while (index < 4) {
            player = lbl_1_bss_0.players[index];
            Hu3DModelPosGet(player->actor->mdlId, &localValue20);
            player = lbl_1_bss_0.players[index];
            if (lbl_1_bss_0.group[index] == 1) {
                MgPlayerSpawn(player, &localValue20);
            } else {
                lbl_1_bss_0.activeMask &= ~(1 << index);
            }
            CharModelVoiceFlagSet(player->charNo, 1);
            index += 1;
        }
        lbl_1_bss_0.tilt.x = 0.0f;
        lbl_1_bss_0.tilt.y = 0.0f;
        lbl_1_bss_0.tilt.z = 0.0f;
        MgTimerParamSet(lbl_1_bss_0.timer, 1800, 0, 0);
        MgTimerModeOnSet(lbl_1_bss_0.timer, 1);
        MgTimerRecordDispOn(lbl_1_bss_0.timer);
    }
    index = 0;
    while (index < 4) {
        if (lbl_1_bss_0.group[index] == 0) {
            if (GwPlayerConf[index].type == 1) {
                stickX = (s32) lbl_1_bss_0.cpuStickX;
                stickY = (s32) lbl_1_bss_0.cpuStickZ;
            } else {
                stickX = HuPadStkX[lbl_1_bss_0.padNo[index]] / 2;
                stickY = HuPadStkY[lbl_1_bss_0.padNo[index]] / 2;
            }
            controller = lbl_1_bss_0.players[index];
            stickX -= stickX / 3;
            stickY -= stickY / 3;
            lbl_1_bss_0.targetTiltX = (f32) -stickY;
            lbl_1_bss_0.targetTiltZ = (f32) -stickX;
        }
        index += 1;
    }
    lbl_1_bss_0.tilt.x += (lbl_1_bss_0.targetTiltX - lbl_1_bss_0.tilt.x) / 30.0f;
    lbl_1_bss_0.tilt.z += (lbl_1_bss_0.targetTiltZ - lbl_1_bss_0.tilt.z) / 30.0f;
    Hu3DModelRotSet(lbl_1_bss_0.collisionModel, lbl_1_bss_0.tilt.x, 0.0f, lbl_1_bss_0.tilt.z);
    Hu3DModelRotSet(lbl_1_bss_0.sceneModels[1], lbl_1_bss_0.tilt.x, 0.0f, lbl_1_bss_0.tilt.z);
    angle = (f32) (360.0 - (180.0 + 180.0 * (atan2((f64) lbl_1_bss_0.tilt.z, -lbl_1_bss_0.tilt.x) / 3.141592653589793)));
    model = controller->actor->mdlId;
    localValue18 = lbl_1_bss_0.tilt;
    motionLength = 80.0f;
    localValue16.x = 13.0f;
    localValue16.y = 0.0f;
    localValue16.z = 13.0f;
    maximum = PSVECMag(&localValue16);
    magnitude = PSVECMag(&localValue18) / maximum;
    Hu3DMotionShiftTimeSet(model, (motionLength * angle) / 360.0f);
    Hu3DSubMotionTimeSet(model, magnitude);
    index = 0;
    while (index < lbl_1_bss_0.collisionCount) {
        actor = lbl_1_bss_0.collisionActors[index];
        rotY = lbl_1_bss_0.collisionRotY[index];
        MgActorRotYGet(actor, &localValue1);
        MgActorRotYSet(actor, localValue1 + (rotY - localValue1) / 10.0f);
        index += 1;
    }
    fn_1_535C();
    MgActorExec();
    index = 0;
    while (index < 4) {
        if (lbl_1_bss_0.group[index] == 0) {
            fn_1_140(index, 1);
        } else {
            fn_1_140(index, 2);
        }
        index += 1;
    }
    fn_1_2F90();
    if (lbl_1_bss_0.previousCollisionFlag == 0 && lbl_1_bss_0.collisionFlag == 1 && lbl_1_bss_0.soundCooldownFrames >= 30) {
        localValue14.x = localValue14.y = localValue14.z = 0.0f;
        index = 0;
        while (index < lbl_1_bss_0.collisionCount) {
            localValue12 = lbl_1_bss_0.collisionActors[index]->pos;
            localValue14.x += localValue12.x;
            localValue14.y += localValue12.y;
            localValue14.z += localValue12.z;
            index += 1;
        }
        localValue14.x /= (f32) lbl_1_bss_0.collisionCount;
        localValue14.y /= (f32) lbl_1_bss_0.collisionCount;
        localValue14.z /= (f32) lbl_1_bss_0.collisionCount;
        soundA = HuAudFXPlay(1846);
        Hu3D3Dto2D(&localValue14, 2, &localValue10);
        panA = (s32) localValue10.x;
        panA /= 5;
        if (panA < 32) {
            panA = 32;
        } else if (panA > 96) {
            panA = 96;
        }
        HuAudFXPanning(soundA, panA);
        lbl_1_bss_0.soundCooldownFrames = 0;
    }
    lbl_1_bss_0.previousCollisionFlag = lbl_1_bss_0.collisionFlag;
    lbl_1_bss_0.collisionFlag = 0;
    lbl_1_bss_0.soundCooldownFrames += 1;
    index = 0;
    while (index < lbl_1_bss_0.collisionCount) {
        if (lbl_1_bss_0.collisionPairs[index][0] == 0 && lbl_1_bss_0.collisionPairs[index][1] == 1 && lbl_1_bss_0.collisionTimers[index] >= 30) {
            soundB = HuAudFXPlay(1846);
            localValue8 = lbl_1_bss_0.collisionActors[index]->pos;
            Hu3D3Dto2D(&localValue8, 2, &localValue7);
            panB = (s32) localValue7.x;
            panB /= 5;
            if (panB < 32) {
                panB = 32;
            } else if (panB > 96) {
                panB = 96;
            }
            HuAudFXPanning(soundB, panB);
            lbl_1_bss_0.collisionTimers[index] = 0;
        }
        lbl_1_bss_0.collisionPairs[index][0] = lbl_1_bss_0.collisionPairs[index][1];
        lbl_1_bss_0.collisionPairs[index][1] = 0;
        lbl_1_bss_0.collisionTimers[index] += 1;
        index += 1;
    }
    if (lbl_1_bss_0.activeMask == 0 || MgTimerDoneCheck(lbl_1_bss_0.timer) != 0) {
        if (lbl_1_bss_0.activeMask == 0) {
            lbl_1_bss_0.completion = 0;
        } else {
            lbl_1_bss_0.completion = 1;
        }
        if (MgTimerDoneCheck(lbl_1_bss_0.timer) == 0) {
            MgTimerRecordDispOff(lbl_1_bss_0.timer);
        }
        MgSeqModeNext();
    }
}

/* Post-play callback registered in lbl_1_data_0; handles frame-0 effects, hides arena actors at frame 8, and advances defeat animations. */
void fn_1_2368(s16 mode, s16 frameNo)
{
    Point3d localValue2;
    Point3d localValue0;
    s32 index;

    if (frameNo == 0) {
        fn_1_104(lbl_1_bss_0.stream);
        index = 0;
        while (index < 4) {
            MgPlayerAttrSet(lbl_1_bss_0.players[index], 1U);
            index += 1;
        }
        CharEffectLayerSet(5);
        index = 0;
        while (index < lbl_1_bss_0.collisionCount) {
            MGACTOR *actor = lbl_1_bss_0.collisionActors[index];
            localValue0 = actor->pos;
            localValue2.x = 100.0f;
            localValue2.y = 1750.0f;
            localValue2.z = 1100.0f;
            PSVECSubtract(&localValue2, &localValue0, &localValue2);
            PSVECNormalize(&localValue2, &localValue2);
            localValue0.x += 100.0f * localValue2.x;
            localValue0.y += 100.0f * localValue2.y;
            localValue0.z += 100.0f * localValue2.z;
            CharEffectLayerSet(5);
            CharEffectSmokeCreateScale(2, &localValue0, 2.5f);
            CharEffectLayerSet(5);
            index += 1;
        }
    }
    if (frameNo == 8) {
        index = 0;
        while (index < lbl_1_bss_0.collisionCount) {
            MGACTOR *actor = lbl_1_bss_0.collisionActors[index];
            Hu3DModelAttrSet(actor->mdlId, HU3D_ATTR_DISPOFF);
            index += 1;
        }
    }
    MgActorExec();
    fn_1_2F90();
}

/* Results callback registered in lbl_1_data_0; awards the outcome, presents the winning group, and hides arena models. */
void fn_1_2574(s16 mode, s16 frameNo)
{
    s16 winners[4] = { -1, -1, -1, -1 };
    s16 coins[4] = { 0, 0, 0, 0 };
    s32 index;
    MGPLAYER *player;
    s32 count;
    char **hooks;
    s32 active;
    MGACTOR *actor;
    s16 bonus;

    MgActorExec();
    if (lbl_1_bss_0.completion == 0) {
        index = 0;
        while (index < 4) {
            if (lbl_1_bss_0.group[index] == 0) {
                winners[0] = (s16) lbl_1_bss_0.charNo[index];
                coins[index] = 10;
                break;
            }
            fn_1_140(index, 1);
            index += 1;
        }
    } else {
        count = 0;
        index = 0;
        while (index < 4) {
            if (lbl_1_bss_0.group[index] == 1) {
                winners[count++] = (s16) lbl_1_bss_0.charNo[index];
                coins[index] = 10;
            }
            fn_1_140(index, 2);
            index += 1;
        }
    }
    MgSeqWinnerSet(winners[0], winners[1], winners[2], winners[3]);
    index = 0;
    while (index < 4) {
        bonus = coins[index];
        if (_CheckFlag(65551U) == 0) {
            GwPlayer[index].mgCoinBonus = bonus;
        }
        index += 1;
    }
    WipeCreate(2, 0, 60);
    index = 0;
    while (index < 60) {
        HuPrcVSleep();
        index += 1;
    }
    index = 0;
    while (index < 4) {
        MgPlayerDespawn(lbl_1_bss_0.players[index]);
        Hu3DMotionOverlayReset(lbl_1_bss_0.players[index]->actor->mdlId);
        index += 1;
    }
    if (lbl_1_bss_0.completion == 0) {
        index = 0;
        while (index < 4) {
            if (lbl_1_bss_0.group[index] == 1) {
                player = lbl_1_bss_0.players[index];
                Hu3DModelShadowReset(player->actor->mdlId);
            }
            index += 1;
        }
        index = 0;
        while (index < 4) {
            if (lbl_1_bss_0.group[index] == 0) {
                count = index;
                player = lbl_1_bss_0.players[index];
                break;
            }
            index += 1;
        }
        Hu3DCameraScissorSet(1, 0U, 0U, 640U, 480U);
        Hu3DCameraScissorSet(2, 0U, 0U, 0U, 0U);
        Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        Hu3DCameraMotionStart(lbl_1_bss_0.cameraMotions[3], 1U);
        Hu3DModelAttrSet(lbl_1_bss_0.hookModelId, HU3D_ATTR_DISPOFF);
        Hu3DSubMotionReset(player->actor->mdlId);
        CharMotionSet((s16) lbl_1_bss_0.charNo[count], player->omObj->mtnId[0]);
        Hu3DModelAttrSet(player->actor->mdlId, HU3D_MOTATTR_LOOP);
    } else {
        Hu3DCameraScissorSet(2, 0U, 0U, 640U, 480U);
        Hu3DCameraScissorSet(1, 0U, 0U, 0U, 0U);
        Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        Hu3DCameraMotionStart(lbl_1_bss_0.cameraMotions[4], 2U);
        Hu3DModelRotSet(lbl_1_bss_0.sceneModels[1], 0.0f, 0.0f, 0.0f);
        active = 0;
        {
        char *one[1] = { lbl_1_data_28 };
        char *two[2] = { lbl_1_data_35, lbl_1_data_42 };
        char *three[3] = { lbl_1_data_4F, lbl_1_data_5C, lbl_1_data_69 };
        Point3d pos;
        index = 0;
        while (index < 4) {
            if (lbl_1_bss_0.group[index] == 1 && (lbl_1_bss_0.activeMask & (1 << index)) != 0) {
                active += 1;
            }
            index += 1;
        }
        switch (active) {
        case 1: hooks = one; break;
        case 2: hooks = two; break;
        case 3: hooks = three; break;
        }
        count = 0;
        index = 0;
        while (index < 4) {
            if (lbl_1_bss_0.group[index] != 0 && lbl_1_bss_0.playerState[index] == 0) {
                player = lbl_1_bss_0.players[index];
                Hu3DModelObjPosGet(lbl_1_bss_0.modelId, hooks[count], &pos);
                Hu3DModelAttrSet(player->actor->mdlId, HU3D_MOTATTR_LOOP);
                CharMotionSet(player->charNo, player->omObj->mtnId[0]);
                Hu3DModelPosSetV(player->actor->mdlId, &pos);
                Hu3DModelRotSet(player->actor->mdlId, 0.0f, 0.0f, 0.0f);
                count += 1;
            }
            index += 1;
        }
        }
    }
    index = 0;
    while (index < lbl_1_bss_0.linkedModelCount) {
        Hu3DModelAttrSet(lbl_1_bss_0.linkedModels[index], HU3D_ATTR_DISPOFF);
        index += 1;
    }
    index = 0;
    while (index < lbl_1_bss_0.collisionCount) {
        actor = lbl_1_bss_0.collisionActors[index];
        Hu3DModelAttrSet(actor->mdlId, HU3D_ATTR_DISPOFF);
        index += 1;
    }
    WipeCreate(1, 5, 60);
    index = 0;
    while (index < 60) {
        HuPrcVSleep();
        index += 1;
    }
    index = 0;
    while (index < 30) {
        HuPrcVSleep();
        index += 1;
    }
    MgSeqModeNext();
}

/* Exit callback registered in lbl_1_data_0; starts the losing players' exit motion on frame 0 and runs their actors. */
void fn_1_2E04(s16 mode, s16 frameNo)
{
    s32 index;
    MGPLAYER *player;

    if (frameNo == 0) {
        if (lbl_1_bss_0.completion == 0) {
            index = 0;
            while (index < 4) {
                if (lbl_1_bss_0.group[index] == 0) {
                    player = lbl_1_bss_0.players[index];
                    CharMotionShiftSet(lbl_1_bss_0.charNo[index], player->omObj->mtnId[8], 0.0f, 6.0f, 0U);
                }
                index += 1;
            }
        } else {
            index = 0;
            while (index < 4) {
                if (lbl_1_bss_0.group[index] == 1 && lbl_1_bss_0.playerState[index] == 0) {
                    player = lbl_1_bss_0.players[index];
                    CharMotionShiftSet(lbl_1_bss_0.charNo[index], player->omObj->mtnId[8], 0.0f, 6.0f, 0U);
                }
                index += 1;
            }
        }
    }
    MgActorExec();
}

void fn_1_2F88(s16 mode, s16 frameNo)
{

}

void fn_1_2F8C(s16 mode, s16 frameNo)
{

}

/* Called by fn_1_17C0 and fn_1_2368 to advance defeated players through launch, flight, and hide states. */
void fn_1_2F90(void)
{
    Point3d localValue5;
    Point3d localValue4;
    Point3d localValue2;
    Point3d localValue0;
    MGPLAYER *temporary0;
    f32 temporaryFloat4;
    s32 value0;
    s32 value1;

    value1 = 0;
    while (value1 < 4) {
        if (lbl_1_bss_0.group[value1] != 0) {
            temporary0 = lbl_1_bss_0.players[value1];
            switch (lbl_1_bss_0.playerState[value1]) {
            case 0:
                break;
            case 1:
                MgPlayerDespawn(temporary0);
                omVibrate(temporary0->playerNo, 20, 20, 0);
                CharMotionSet(temporary0->charNo, temporary0->omObj->mtnId[6]);
                lbl_1_bss_0.playerState[value1] = 2;
                break;
            case 2:
                localValue4 = lbl_1_bss_0.playerMotionVec[value1];
                Hu3DModelPosGet((s16) temporary0->actor->mdlId, &localValue5);
                if (localValue5.y >= 1200.0f) {
                    temporaryFloat4 = (f32) (u32) frandmod(180);
                    value0 = 0;
                    while (value0 < 4) {
                        if (lbl_1_bss_0.group[value0] != 0) {
                            value0 += 1;
                            continue;
                        }
                        break;
                    }
                    localValue5 = lbl_1_bss_0.players[value0]->actor->pos;
                    switch (lbl_1_bss_0.players[value0]->charNo) {
                    case 6:
                        localValue5.y += 95.0f;
                        localValue5.z += 60.0f;
                        break;
                    default:
                        localValue5.y += 60.0f;
                        localValue5.z += 20.0f;
                        break;
                    }
                    Hu3DModelCameraSet((s16) temporary0->actor->mdlId, 1U);
                    Hu3DModelPosSet((s16) temporary0->actor->mdlId, localValue5.x, localValue5.y, localValue5.z);
                    Hu3DModelScaleSet((s16) temporary0->actor->mdlId, 0.2f, 0.2f, 0.2f);
                    localValue4.x = (f32) (cos((3.141592653589793 * (f64) temporaryFloat4) / 180.0) - sin((3.141592653589793 * (f64) temporaryFloat4) / 180.0));
                    localValue4.y = 5.0f;
                    localValue4.z = (f32) (sin((3.141592653589793 * (f64) temporaryFloat4) / 180.0) + cos((3.141592653589793 * (f64) temporaryFloat4) / 180.0));
                    PSVECNormalize(&localValue4, &localValue4);
                    lbl_1_bss_0.playerMotionVec[value1] = localValue4;
                    lbl_1_bss_0.playerState[value1] = 3;
                } else {
                    localValue5.x += 40.0f * localValue4.x;
                    localValue5.y += 40.0f * localValue4.y;
                    localValue5.z += 40.0f * localValue4.z;
                    Hu3DModelPosSetV((s16) temporary0->actor->mdlId, &localValue5);
                }
                break;
            case 3:
                localValue0 = lbl_1_bss_0.playerMotionVec[value1];
                Hu3DModelPosGet((s16) temporary0->actor->mdlId, &localValue2);
                localValue2.x += 20.0f * localValue0.x;
                localValue2.y += 10.0f * localValue0.y;
                localValue2.z += 20.0f * localValue0.z;
                Hu3DModelPosSetV((s16) temporary0->actor->mdlId, &localValue2);
                if (PSVECMag(&localValue2) >= 500.0f) {
                    lbl_1_bss_0.playerState[value1] = 4;
                    Hu3DModelShadowReset((s16) temporary0->actor->mdlId);
                    Hu3DModelAttrSet((s16) temporary0->actor->mdlId, HU3D_ATTR_DISPOFF);
                }
                break;
            }
        }
        value1 += 1;
    }
}
