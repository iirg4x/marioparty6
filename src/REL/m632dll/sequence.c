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

static char lbl_1_data_28[13] = { 69, 68, 112, 97, 116, 116, 101, 114, 110, 49, 80, 50, 0 };

static char lbl_1_data_35[13] = { 69, 68, 112, 97, 116, 116, 101, 114, 110, 50, 80, 50, 0 };

static char lbl_1_data_42[13] = { 69, 68, 112, 97, 116, 116, 101, 114, 110, 50, 80, 51, 0 };

static char lbl_1_data_4F[13] = { 69, 68, 112, 97, 116, 116, 101, 114, 110, 51, 80, 50, 0 };

static char lbl_1_data_5C[13] = { 69, 68, 112, 97, 116, 116, 101, 114, 110, 51, 80, 51, 0 };

static char lbl_1_data_69[15] = { 69, 68, 112, 97, 116, 116, 101, 114, 110, 51, 80, 52, 0, 0, 0 };

s32 fn_1_A0(s32 arg0, s32 arg1)
{
    s32 var_r31;

    var_r31 = arg0;
    if ((var_r31 == -1) && ((s32) (GameMesStatGet(MgSeqGameMesIdGet()) & 16) != 0)) {
        var_r31 = HuAudBGMPlay((s16) arg1);
    }
    return var_r31;
}

void fn_1_104(s32 arg0)
{
    if (arg0 != -1) {
        HuAudSStreamFadeOut(arg0, 100);
    }
}

void fn_1_140(s32 arg0, s32 arg1)
{
    Point3d sp18;
    Point3d spC;
    s32 var_r31;

    sp18 = lbl_1_bss_0.players[arg0]->actor->pos;
    Hu3D3Dto2D(&sp18, (s16) arg1, &spC);
    var_r31 = (s32) spC.x;
    var_r31 /= 5;
    if ((s32) var_r31 < 32) {
        var_r31 = 32;
    } else if ((s32) var_r31 > 96) {
        var_r31 = 96;
    }
    CharModelVoicePanSet((s16) lbl_1_bss_0.charNo[arg0], (s16) var_r31);
}

void fn_1_21C(s16 mode, s16 frameNo)
{
    MgActorExec();
    MgSeqModeNext();
}

void fn_1_240(OMOBJ *obj)
{
    f32 sp38[3][4];
    Point3d sp2C;
    Point3d sp20;
    Point3d sp14;
    Point3d sp8;
    MGPLAYER *player;
    s32 index;

    player = (MGPLAYER *) obj->work[0];
    index = obj->work[1];
    Hu3DModelObjMtxGet(lbl_1_bss_0.models024[index], lbl_1_data_108[index], sp38);
    Hu3DMtxTransGet(sp38, &sp2C);
    Hu3DMtxRotGet(sp38, &sp20);
    Hu3DMtxScaleGet(sp38, &sp14);
    Hu3DModelPosSetV(player->actor->mdlId, &sp2C);
    Hu3DModelRotSetV(player->actor->mdlId, &sp20);
    Hu3DModelScaleSetV(player->actor->mdlId, &sp14);
    Hu3DModelObjMtxGet(lbl_1_bss_0.models024[index], lbl_1_data_108[index], sp38);
    Hu3DMtxTransGet(sp38, &sp8);
    if (sp8.y >= 2.0f) {
        CharModelVoiceFlagSet(lbl_1_bss_0.players[index]->charNo, 0);
    }
    if (Hu3DMotionEndCheck(lbl_1_bss_0.models024[index]) != 0) {
        Hu3DModelAttrSet(player->actor->mdlId, 1U);
        lbl_1_bss_0.activeMask &= ~(1 << index);
        obj->objFunc = NULL;
    }
}

void fn_1_3E0(OMOBJ *obj)
{
    Point3d sp20;
    Point3d sp14;
    Point3d sp8;
    MGPLAYER *player;
    s16 model;
    s32 index;

    player = (MGPLAYER *) obj->work[0];
    index = obj->work[1];
    Hu3DModelObjPosGet(lbl_1_bss_0.models024[index], lbl_1_data_108[index], &sp20);
    Hu3DModelPosSetV(player->actor->mdlId, &sp20);
    sp14.x = 0.0f;
    sp14.y = 0.0f;
    sp14.z = 1.0f;
    sp8 = sp20;
    sp20.y = -2600.0f;
    sp8.y = 100.0f;
    Hu3DShadowMultiPosSet(&sp20, &sp14, &sp8, 1);
    if (Hu3DMotionEndCheck(lbl_1_bss_0.models024[index]) != 0) {
        model = player->actor->mdlId;
        Hu3DMotionOverlayReset(model);
        Hu3DMotionSet(model, player->omObj->mtnId[12]);
        Hu3DMotionTimeSet(model, 39.0f);
        Hu3DModelAttrSet(model, 1073741826U);
        Hu3DSubMotionSet(model, player->omObj->mtnId[14], 0.0f);
        Hu3DModelAttrSet(model, 1073741840U);
        lbl_1_bss_0.activeMask &= ~(1 << index);
        obj->objFunc = NULL;
    }
}

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
    f32 temp_f31, temp_f30, temp_f29, temp_f28, temp_f27;

    if (frameNo == 0) {
        MgSeqModeChangeOff();
        Hu3DCameraMotionStart(lbl_1_bss_0.cameraMotions[0], 1U);
        counter = 0;
        index = 0;
        while (index < 4) {
            player = lbl_1_bss_0.players[index];
            MgPlayerDespawn(player);
            Hu3DMotionTimeSet(lbl_1_bss_0.models024[index], 0.0f);
            if (lbl_1_bss_0.group[index] == 0) {
                CharMotionSet(player->charNo, player->omObj->mtnId[1]);
                Hu3DMotionOverlaySet(player->actor->mdlId, player->omObj->mtnId[13]);
                Hu3DModelAttrSet(player->actor->mdlId, 1073742336U);
                hookName = CharModelItemHookGet(player->charNo, 2, 0);
                Hu3DModelHookSet(player->actor->mdlId, hookName, lbl_1_bss_0.model0C0);
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
    switch (lbl_1_bss_0.phase) {                    /* irregular */
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
            Hu3DModelAttrReset(lbl_1_bss_0.models02E[1], 1U);
            Hu3DModelAttrReset(lbl_1_bss_0.models02E[2], 1U);
            counter = 0;
            index = 0;
            while (index < 4) {
                player = lbl_1_bss_0.players[index];
                if (lbl_1_bss_0.group[index] == 1) {
                    MgActorPosSet((MGACTOR *) lbl_1_bss_0.players[index], &lbl_1_bss_0.positions[counter]);
                    MgActorPosSetRaw((MGACTOR *) lbl_1_bss_0.players[index], &lbl_1_bss_0.positions[counter]);
                    Hu3DModelPosSetV(player->actor->mdlId, &lbl_1_bss_0.positions[counter]);
                    {
                    Point3d sp9C = {100000.0f, 0.0f, 0.0f};
                    Point3d sp90 = {0.0f, 0.0f, 0.0f};
                    Point3d sp84 = {1.0f, 1.0f, 1.0f};
                    Mtx matrix;
                    PSMTXIdentity(matrix);
                    Hu3DModelMtxSet(player->actor->mdlId, &matrix);
                    Hu3DModelRotSetV(player->actor->mdlId, &sp90);
                    Hu3DModelScaleSetV(player->actor->mdlId, &sp84);
                    Hu3DModelAttrReset(player->actor->mdlId, 1U);
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
        Point3d sp78, sp6C, sp60;
        temp_f30 = (f32) (u32) lbl_1_bss_0.frame / 60.0f;
        temp_f31 = 640.0f + (-430.0f * temp_f30);
        temp_f29 = 640.0f * temp_f30;
        temp_f28 = ((640.0f + (-414.0f * temp_f30)) / 2.0f) - 320.0f;
        temp_f27 = (640.0f - ((3.0f * temp_f29) / 4.0f)) - (8.0f * temp_f30);
        sp78.x = sp78.z = 0.0f;
        sp6C.x = sp6C.y = 0.0f;
        sp6C.z = 1.0f;
        sp60.x = sp60.z = 0.0f;
        sp60.y = 100.0f;
        sp78.y = -2600.0f + ((1600.0f * (f32) (u32) lbl_1_bss_0.frame) / 60.0f);
        Hu3DShadowMultiPosSet(&sp78, &sp6C, &sp60, 1);
        Hu3DCameraViewportSet(1, temp_f28, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        if (temp_f31 > 2.0f) {
            Hu3DCameraScissorSet(1, 0U, 0U, (u32) temp_f31 - 2, 480U);
        } else {
            Hu3DCameraScissorSet(1, 0U, 0U, 0U, 480U);
        }
        if ((u32) lbl_1_bss_0.frame >= 1U) {
            Hu3DCameraViewportSet(2, temp_f27, 0.0f, temp_f29, 480.0f, 0.0f, 1.0f);
            if (temp_f31 > 2.0f) {
                Hu3DCameraScissorSet(2, (u32) temp_f31 + 2, 0U, 638 - (u32) temp_f31, 480U);
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
                Hu3DModelHookReset(lbl_1_bss_0.models024[index]);
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
        Point3d sp54;
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
                Hu3DModelPosGet(player->actor->mdlId, &sp54);
                sp54.y -= 46.0f;
                if (sp54.y <= 0.0f) {
                    sp54.y = 0.0f;
                }
                MgActorPosSet((MGACTOR *) player, &sp54);
                Hu3DModelPosSetV(player->actor->mdlId, &sp54);
                if (sp54.y <= 0.0f) {
                    if ((s32) (lbl_1_bss_0.activeMask & (1 << index)) != 0) {
                        lbl_1_bss_0.activeMask &= ~(1 << index);
                        CharModelLandDustCreate(lbl_1_bss_0.charNo[index], &sp54);
                        CharMotionShiftSet(player->charNo, player->omObj->mtnId[11], 0.0f, 6.0f, 1073741825U);
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
                    CharMotionShiftSet(player->charNo, *player->omObj->mtnId, 0.0f, 6.0f, 1073741825U);
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

void fn_1_173C(s16 mode, s16 frameNo)
{
    lbl_1_bss_0.stream = fn_1_A0(lbl_1_bss_0.stream, 73);
    MgActorExec();
}

void fn_1_17C0(s16 mode, s16 frameNo)
{
    Point3d sp94;
    Point3d sp88;
    Point3d sp7C;
    Point3d sp70;
    Point3d sp64;
    Point3d sp58;
    Point3d sp4C;
    Point3d sp40;
    f32 spC;
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
            Hu3DModelPosGet(player->actor->mdlId, &sp94);
            player = lbl_1_bss_0.players[index];
            if (lbl_1_bss_0.group[index] == 1) {
                MgPlayerSpawn(player, &sp94);
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
                stickX = (s32) lbl_1_bss_0.scalar194;
                stickY = (s32) lbl_1_bss_0.scalar198;
            } else {
                stickX = HuPadStkX[lbl_1_bss_0.padNo[index]] / 2;
                stickY = HuPadStkY[lbl_1_bss_0.padNo[index]] / 2;
            }
            controller = lbl_1_bss_0.players[index];
            stickX -= stickX / 3;
            stickY -= stickY / 3;
            lbl_1_bss_0.scalar1D0 = (f32) -stickY;
            lbl_1_bss_0.scalar1D8 = (f32) -stickX;
        }
        index += 1;
    }
    lbl_1_bss_0.tilt.x += (lbl_1_bss_0.scalar1D0 - lbl_1_bss_0.tilt.x) / 30.0f;
    lbl_1_bss_0.tilt.z += (lbl_1_bss_0.scalar1D8 - lbl_1_bss_0.tilt.z) / 30.0f;
    Hu3DModelRotSet(lbl_1_bss_0.collisionModel, lbl_1_bss_0.tilt.x, 0.0f, lbl_1_bss_0.tilt.z);
    Hu3DModelRotSet(lbl_1_bss_0.models02E[1], lbl_1_bss_0.tilt.x, 0.0f, lbl_1_bss_0.tilt.z);
    angle = (f32) (360.0 - (180.0 + 180.0 * (atan2((f64) lbl_1_bss_0.tilt.z, -lbl_1_bss_0.tilt.x) / 3.141592653589793)));
    model = controller->actor->mdlId;
    sp88 = lbl_1_bss_0.tilt;
    motionLength = 80.0f;
    sp7C.x = 13.0f;
    sp7C.y = 0.0f;
    sp7C.z = 13.0f;
    maximum = PSVECMag(&sp7C);
    magnitude = PSVECMag(&sp88) / maximum;
    Hu3DMotionShiftTimeSet(model, (motionLength * angle) / 360.0f);
    Hu3DSubMotionTimeSet(model, magnitude);
    index = 0;
    while (index < lbl_1_bss_0.collisionCount) {
        actor = lbl_1_bss_0.collisionActors[index];
        rotY = lbl_1_bss_0.collisionRotY[index];
        MgActorRotYGet(actor, &spC);
        MgActorRotYSet(actor, spC + (rotY - spC) / 10.0f);
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
    if (lbl_1_bss_0.field3F8 == 0 && lbl_1_bss_0.field3FC == 1 && lbl_1_bss_0.field400 >= 30) {
        sp70.x = sp70.y = sp70.z = 0.0f;
        index = 0;
        while (index < lbl_1_bss_0.collisionCount) {
            sp64 = lbl_1_bss_0.collisionActors[index]->pos;
            sp70.x += sp64.x;
            sp70.y += sp64.y;
            sp70.z += sp64.z;
            index += 1;
        }
        sp70.x /= (f32) lbl_1_bss_0.collisionCount;
        sp70.y /= (f32) lbl_1_bss_0.collisionCount;
        sp70.z /= (f32) lbl_1_bss_0.collisionCount;
        soundA = HuAudFXPlay(1846);
        Hu3D3Dto2D(&sp70, 2, &sp58);
        panA = (s32) sp58.x;
        panA /= 5;
        if (panA < 32) {
            panA = 32;
        } else if (panA > 96) {
            panA = 96;
        }
        HuAudFXPanning(soundA, panA);
        lbl_1_bss_0.field400 = 0;
    }
    lbl_1_bss_0.field3F8 = lbl_1_bss_0.field3FC;
    lbl_1_bss_0.field3FC = 0;
    lbl_1_bss_0.field400 += 1;
    index = 0;
    while (index < lbl_1_bss_0.collisionCount) {
        if (lbl_1_bss_0.collisionPairs[index][0] == 0 && lbl_1_bss_0.collisionPairs[index][1] == 1 && lbl_1_bss_0.collisionTimers[index] >= 30) {
            soundB = HuAudFXPlay(1846);
            sp4C = lbl_1_bss_0.collisionActors[index]->pos;
            Hu3D3Dto2D(&sp4C, 2, &sp40);
            panB = (s32) sp40.x;
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

void fn_1_2368(s16 mode, s16 frameNo)
{
    Point3d sp14;
    Point3d sp8;
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
            sp8 = actor->pos;
            sp14.x = 100.0f;
            sp14.y = 1750.0f;
            sp14.z = 1100.0f;
            PSVECSubtract(&sp14, &sp8, &sp14);
            PSVECNormalize(&sp14, &sp14);
            sp8.x += 100.0f * sp14.x;
            sp8.y += 100.0f * sp14.y;
            sp8.z += 100.0f * sp14.z;
            CharEffectLayerSet(5);
            CharEffectSmokeCreateScale(2, &sp8, 2.5f);
            CharEffectLayerSet(5);
            index += 1;
        }
    }
    if (frameNo == 8) {
        index = 0;
        while (index < lbl_1_bss_0.collisionCount) {
            MGACTOR *actor = lbl_1_bss_0.collisionActors[index];
            Hu3DModelAttrSet(actor->mdlId, 1U);
            index += 1;
        }
    }
    MgActorExec();
    fn_1_2F90();
}

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
        Hu3DModelAttrSet(lbl_1_bss_0.model0C0, 1U);
        Hu3DSubMotionReset(player->actor->mdlId);
        CharMotionSet((s16) lbl_1_bss_0.charNo[count], player->omObj->mtnId[0]);
        Hu3DModelAttrSet(player->actor->mdlId, 1073741825U);
    } else {
        Hu3DCameraScissorSet(2, 0U, 0U, 640U, 480U);
        Hu3DCameraScissorSet(1, 0U, 0U, 0U, 0U);
        Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        Hu3DCameraMotionStart(lbl_1_bss_0.cameraMotions[4], 2U);
        Hu3DModelRotSet(lbl_1_bss_0.models02E[1], 0.0f, 0.0f, 0.0f);
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
                Hu3DModelObjPosGet(lbl_1_bss_0.model02C, hooks[count], &pos);
                Hu3DModelAttrSet(player->actor->mdlId, 1073741825U);
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
        Hu3DModelAttrSet(lbl_1_bss_0.linkedModels[index], 1U);
        index += 1;
    }
    index = 0;
    while (index < lbl_1_bss_0.collisionCount) {
        actor = lbl_1_bss_0.collisionActors[index];
        Hu3DModelAttrSet(actor->mdlId, 1U);
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

void fn_1_2F90(void)
{
    Point3d sp2C;
    Point3d sp20;
    Point3d sp14;
    Point3d sp8;
    MGPLAYER *temp_r31;
    f32 temp_f31;
    s32 var_r29;
    s32 var_r30;

    var_r30 = 0;
    while (var_r30 < 4) {
        if (lbl_1_bss_0.group[var_r30] != 0) {
            temp_r31 = lbl_1_bss_0.players[var_r30];
            switch (lbl_1_bss_0.playerState[var_r30]) {
            case 0:
                break;
            case 1:
                MgPlayerDespawn(temp_r31);
                omVibrate(temp_r31->playerNo, 20, 20, 0);
                CharMotionSet(temp_r31->charNo, temp_r31->omObj->mtnId[6]);
                lbl_1_bss_0.playerState[var_r30] = 2;
                break;
            case 2:
                sp20 = lbl_1_bss_0.playerMotionVec[var_r30];
                Hu3DModelPosGet((s16) temp_r31->actor->mdlId, &sp2C);
                if (sp2C.y >= 1200.0f) {
                    temp_f31 = (f32) (u32) frandmod(180);
                    var_r29 = 0;
                    while (var_r29 < 4) {
                        if (lbl_1_bss_0.group[var_r29] != 0) {
                            var_r29 += 1;
                            continue;
                        }
                        break;
                    }
                    sp2C = lbl_1_bss_0.players[var_r29]->actor->pos;
                    switch (lbl_1_bss_0.players[var_r29]->charNo) {
                    case 6:
                        sp2C.y += 95.0f;
                        sp2C.z += 60.0f;
                        break;
                    default:
                        sp2C.y += 60.0f;
                        sp2C.z += 20.0f;
                        break;
                    }
                    Hu3DModelCameraSet((s16) temp_r31->actor->mdlId, 1U);
                    Hu3DModelPosSet((s16) temp_r31->actor->mdlId, sp2C.x, sp2C.y, sp2C.z);
                    Hu3DModelScaleSet((s16) temp_r31->actor->mdlId, 0.2f, 0.2f, 0.2f);
                    sp20.x = (f32) (cos((3.141592653589793 * (f64) temp_f31) / 180.0) - sin((3.141592653589793 * (f64) temp_f31) / 180.0));
                    sp20.y = 5.0f;
                    sp20.z = (f32) (sin((3.141592653589793 * (f64) temp_f31) / 180.0) + cos((3.141592653589793 * (f64) temp_f31) / 180.0));
                    PSVECNormalize(&sp20, &sp20);
                    lbl_1_bss_0.playerMotionVec[var_r30] = sp20;
                    lbl_1_bss_0.playerState[var_r30] = 3;
                } else {
                    sp2C.x += 40.0f * sp20.x;
                    sp2C.y += 40.0f * sp20.y;
                    sp2C.z += 40.0f * sp20.z;
                    Hu3DModelPosSetV((s16) temp_r31->actor->mdlId, &sp2C);
                }
                break;
            case 3:
                sp8 = lbl_1_bss_0.playerMotionVec[var_r30];
                Hu3DModelPosGet((s16) temp_r31->actor->mdlId, &sp14);
                sp14.x += 20.0f * sp8.x;
                sp14.y += 10.0f * sp8.y;
                sp14.z += 20.0f * sp8.z;
                Hu3DModelPosSetV((s16) temp_r31->actor->mdlId, &sp14);
                if (PSVECMag(&sp14) >= 500.0f) {
                    lbl_1_bss_0.playerState[var_r30] = 4;
                    Hu3DModelShadowReset((s16) temp_r31->actor->mdlId);
                    Hu3DModelAttrSet((s16) temp_r31->actor->mdlId, 1U);
                }
                break;
            }
        }
        var_r30 += 1;
    }
}
