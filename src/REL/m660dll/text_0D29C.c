/* Advances the result sequence and positions the winner scene. */
#include "REL/m660dll/particles.h"
#include "dolphin/math.h"
#include "game/hu3d.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "game/wipe.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "REL/m660dll/scene.h"

void fn_1_66C8(s16 mode, s16 duration);
s32 fn_1_D5C8(void);
void fn_1_7008(void);
void fn_1_2B34(void);
void fn_1_915C(void);
void fn_1_24C4(void);
void fn_1_71C(void);
void fn_1_6F38(void);
s32 fn_1_601C(M660Player *player, s32 mode);
extern HuVecF Center, CRot;
extern f32 CZoom;
void fn_1_4004(HuVecF *origin);
void fn_1_3068(MGTIMER *timer);
struct VoicePosition;
void fn_1_67D4(s16 slot, s16 mode, struct VoicePosition *position);
void fn_1_4B38(M660Model *origin);

/* The result-sequence callback updates the scene and returns zero, so its caller holds the
 * phase. */
int fn_1_D29C(void) {
    short result = 0;
    fn_1_66C8(1, 100);
    fn_1_D5C8();
    fn_1_7008();
    fn_1_2B34();
    fn_1_915C();
    fn_1_24C4();
    return result;
}

/* The result-sequence callback updates the scene and returns its phase controller's signal. */
s16 fn_1_D2E8(void)
{
    s16 result = 0;
    result = fn_1_D5C8();
    fn_1_7008();
    fn_1_915C();
    fn_1_24C4();
    return result;
}

/* The result-sequence callback starts winner motions once, then updates the scene each tick. */
s32 fn_1_D328(void) {
    s16 playerIndex;

    switch (lbl_1_bss_1F4.resultState) {
    case 1:
        break;
    case 0:
        playerIndex = 0;
        while (playerIndex < 2) {
            fn_1_601C(&lbl_1_bss_2DC[playerIndex], 6);
            if (lbl_1_bss_2DC[playerIndex].mode >= 0) {
                lbl_1_bss_544[playerIndex].shiftState = 1;
                lbl_1_bss_544[playerIndex].shiftMotion = 2;
                lbl_1_bss_544[playerIndex].shiftStart = 0.0f;
                lbl_1_bss_544[playerIndex].shiftEnd = 5.0f;
                lbl_1_bss_544[playerIndex].shiftFlags = 0;
                lbl_1_bss_544[playerIndex].attributesToReset = HU3D_MOTATTR_LOOP;
            } else if (lbl_1_bss_1F4.tie != 0) {
                lbl_1_bss_544[playerIndex].shiftState = 1;
                lbl_1_bss_544[playerIndex].shiftMotion = 3;
                lbl_1_bss_544[playerIndex].shiftStart = 0.0f;
                lbl_1_bss_544[playerIndex].shiftEnd = 5.0f;
                lbl_1_bss_544[playerIndex].shiftFlags = 0;
                lbl_1_bss_544[playerIndex].attributesToReset = HU3D_MOTATTR_LOOP;
            }
            playerIndex += 1;
        }
        lbl_1_bss_1F4.resultState += 1;
        break;
    }
    fn_1_71C();
    fn_1_D5C8();
    fn_1_24C4();
    return 0;
}

/* The result-sequence callback ticks the scene and shared stage updates while waiting. */
s32 fn_1_D57C(void) {
    fn_1_D5C8();
    fn_1_24C4();
    return 0;
}

/* Stops all audio and returns zero, keeping the result sequence in this phase. */
s32 fn_1_D5A4(void)
{
    fn_1_6F38();
    return 0;
}

/* Called by result-sequence callbacks to run phases; returns one once the winner scene is ready. */
s32 fn_1_D5C8(void)
{
    s32 result = 0;
    HuVecF origins[2] = {{-788.0f, 60.0f, -788.0f}, {788.0f, 60.0f, -788.0f}};
    M660Model *models[3];
    s16 index;
    s32 firstWinner;
    s32 secondWinner;

    fn_1_4004(&origins[0]);
    fn_1_4004(&origins[1]);
    switch (lbl_1_bss_1F4.phase) {
    case 0:
        if (MgSeqModeGet() == 7 && lbl_1_bss_1F4.timer++ > 30) {
            WipeCreate(2, 6, 60);
            lbl_1_bss_1F4.timer = 0;
            lbl_1_bss_1F4.phase++;
        }
        break;
    case 1:
        if (WipeCheck() == 0) {
            lbl_1_bss_2DC[0].state = 0;
            lbl_1_bss_2DC[1].state = 0;
            if (lbl_1_bss_1F4.tie != 0) {
                lbl_1_bss_2DC[0].position.x = -100.0f;
                lbl_1_bss_2DC[0].position.y = 0.0f;
                lbl_1_bss_2DC[0].position.z = -321.0f;
                lbl_1_bss_2DC[0].rotation.x = 0.0f;
                lbl_1_bss_2DC[0].rotation.y = 0.0f;
                lbl_1_bss_2DC[0].rotation.z = 0.0f;
                lbl_1_bss_2DC[1].position.x = 100.0f;
                lbl_1_bss_2DC[1].position.y = 0.0f;
                lbl_1_bss_2DC[1].position.z = -321.0f;
                lbl_1_bss_2DC[1].rotation.x = 0.0f;
                lbl_1_bss_2DC[1].rotation.y = 0.0f;
                lbl_1_bss_2DC[1].rotation.z = 0.0f;
                lbl_1_bss_544[0].previousAttributes = lbl_1_bss_544[0].previousMode = -1;
                lbl_1_bss_544[1].previousAttributes = lbl_1_bss_544[1].previousMode = -1;
                lbl_1_bss_544[0].mode = 0;
                lbl_1_bss_544[0].speed = 1.0f;
                lbl_1_bss_544[0].time = 0.0f;
                lbl_1_bss_544[0].attributes = HU3D_MOTATTR_LOOP;
                lbl_1_bss_544[0].attributesToReset = 0;
                lbl_1_bss_544[1].mode = 0;
                lbl_1_bss_544[1].speed = 1.0f;
                lbl_1_bss_544[1].time = 0.0f;
                lbl_1_bss_544[1].attributes = HU3D_MOTATTR_LOOP;
                lbl_1_bss_544[1].attributesToReset = 0;
                fn_1_3068(lbl_1_bss_1F4.gameTimer);
            } else {
                for (index = 0; index < 2; index++) {
                    if (lbl_1_bss_2DC[index].mode != -1) {
                        lbl_1_bss_2DC[index].position.x = 0.0f;
                        lbl_1_bss_2DC[index].position.y = 0.0f;
                        lbl_1_bss_2DC[index].position.z = -321.0f;
                        lbl_1_bss_2DC[index].rotation.x = 0.0f;
                        lbl_1_bss_2DC[index].rotation.y = 0.0f;
                        lbl_1_bss_2DC[index].rotation.z = 0.0f;
                        lbl_1_bss_544[index].previousAttributes =
                            lbl_1_bss_544[index].previousMode = -1;
                        lbl_1_bss_544[index].mode = 0;
                        lbl_1_bss_544[index].speed = 1.0f;
                        lbl_1_bss_544[index].time = 0.0f;
                        lbl_1_bss_544[index].attributes = HU3D_MOTATTR_LOOP;
                        lbl_1_bss_544[index].attributesToReset = 0;
                    } else {
                        lbl_1_bss_2DC[index].position.x = 1000.0f;
                        lbl_1_bss_2DC[index].position.y = 0.0f;
                        lbl_1_bss_2DC[index].position.z = -400.0f;
                        lbl_1_bss_2DC[index].rotation.x = 55.0f;
                        lbl_1_bss_2DC[index].rotation.y = 0.0f;
                        lbl_1_bss_2DC[index].rotation.z = 0.0f;
                        lbl_1_bss_544[index].previousAttributes =
                            lbl_1_bss_544[index].previousMode = -1;
                        lbl_1_bss_544[index].mode = 6;
                        lbl_1_bss_544[index].speed = 1.0f;
                        lbl_1_bss_544[index].time = 0.0f;
                        lbl_1_bss_544[index].attributes = HU3D_MOTATTR_LOOP;
                        lbl_1_bss_544[index].attributesToReset = 0;
                        lbl_1_bss_1F4.losingPlayerIndex = index;
                    }
                }
            }
            lbl_1_bss_2530.records[0].viewport[5] = 44.0f;
            Hu3DCameraPerspectiveSet(1, lbl_1_bss_2530.records[0].viewport[5], 20.0f, 40000.0f,
                                     1.33f);
            Center.x = 0.0f;
            Center.y = 176.656f;
            Center.z = -269.0f;
            CRot.x = -11.2f;
            CRot.y = 0.0f;
            CRot.z = 0.0f;
            CZoom = 508.0f;
            for (index = 45; index <= 53; index++)
                Hu3DModelAttrSet(lbl_1_bss_60C[index].modelId, HU3D_ATTR_DISPOFF);
            for (index = 54; index <= 62; index++)
                Hu3DModelAttrSet(lbl_1_bss_60C[index].modelId, HU3D_ATTR_DISPOFF);
            for (index = 63; index <= 71; index++)
                Hu3DModelAttrSet(lbl_1_bss_60C[index].modelId, HU3D_ATTR_DISPOFF);
            for (index = 31; index < 45; index++)
                Hu3DModelAttrSet(lbl_1_bss_60C[index].modelId, HU3D_ATTR_DISPOFF);
            for (index = 3; index <= 21; index++) {
                if (index != 5 && index != 6 && index != 7 && index != 8)
                    Hu3DModelAttrSet(lbl_1_bss_60C[index].modelId, HU3D_ATTR_DISPOFF);
            }
            for (index = 22; index <= 30; index++) {
                Hu3DModelAttrReset(lbl_1_bss_60C[index].modelId, HU3D_ATTR_DISPOFF);
                if (index == 25 || index == 26 || index == 28 || index == 30) {
                    Hu3DModelAttrSet(lbl_1_bss_60C[index].modelId, HU3D_MOTATTR_LOOP);
                    Hu3DMotionSpeedSet(lbl_1_bss_60C[index].modelId, 1.0f);
                }
            }
            if (lbl_1_bss_2DC[0].mode == -1 && lbl_1_bss_2DC[1].mode == -1) {
                MgSeqWinnerSet(-1, -1, -1, -1);
            } else if (lbl_1_bss_2DC[0].mode >= 0) {
                lbl_1_bss_2DC[0].mode = lbl_1_bss_2DC[0].character;
                firstWinner = lbl_1_bss_2DC[0].selectedPlayer;
                if (_CheckFlag(FLAG_MG_PRACTICE) == 0) GwPlayer[firstWinner].mgCoinBonus = 10;
                MgSeqWinnerSet(lbl_1_bss_2DC[0].mode, -1, -1, -1);
            } else if (lbl_1_bss_2DC[1].mode >= 0) {
                lbl_1_bss_2DC[1].mode = lbl_1_bss_2DC[1].character;
                secondWinner = lbl_1_bss_2DC[1].selectedPlayer;
                if (_CheckFlag(FLAG_MG_PRACTICE) == 0) GwPlayer[secondWinner].mgCoinBonus = 10;
                MgSeqWinnerSet(lbl_1_bss_2DC[1].mode, -1, -1, -1);
            }
            for (index = 72; index <= 74; index++) {
                Hu3DModelAttrReset(lbl_1_bss_60C[index].modelId, HU3D_ATTR_DISPOFF);
                lbl_1_bss_60C[index].position.x = 0.0f;
                lbl_1_bss_60C[index].position.y = 0.0f;
                lbl_1_bss_60C[index].position.z = 0.0f;
                lbl_1_bss_60C[index].rotation.x = 0.0f;
                lbl_1_bss_60C[index].rotation.y = 0.0f;
                lbl_1_bss_60C[index].rotation.z = 0.0f;
                Hu3DModelAttrSet(lbl_1_bss_60C[index].modelId, HU3D_MOTATTR_LOOP);
                Hu3DMotionSpeedSet(lbl_1_bss_60C[index].modelId, 1.0f);
                Hu3DMotionTimeSet(lbl_1_bss_60C[index].modelId, 0.0f);
            }
            for (index = 22; index <= 30; index++)
                Hu3DModelLayerSet(lbl_1_bss_60C[index].modelId, 0);
            Hu3DModelLayerSet(lbl_1_bss_2484.modelIds[1], 0);
            Hu3DModelLayerSet(lbl_1_bss_60C[72].modelId, 3);
            Hu3DModelLayerSet(lbl_1_bss_2484.modelIds[1], 4);
            Hu3DModelLayerSet(lbl_1_bss_2484.modelIds[3], 5);
            Hu3DModelLayerSet(lbl_1_bss_60C[74].modelId, 5);
            Hu3DModelLayerSet(lbl_1_bss_60C[73].modelId, 6);
            if (lbl_1_bss_1F4.tie != 0) {
                Hu3DModelLayerSet(lbl_1_bss_60C[0].modelId, 7);
                Hu3DModelLayerSet(lbl_1_bss_60C[1].modelId, 7);
            } else {
                Hu3DModelLayerSet(lbl_1_bss_60C[lbl_1_bss_1F4.losingPlayerIndex].modelId, 4);
                Hu3DModelLayerSet(lbl_1_bss_60C[lbl_1_bss_1F4.losingPlayerIndex ^ 1].modelId, 7);
            }
            lbl_1_bss_1F4.phase++;
        }
        break;
    case 2:
        if (lbl_1_bss_1F4.timer++ > 30) {
            WipeCreate(1, 6, 60);
            lbl_1_bss_1F4.timer = 0;
            lbl_1_bss_1F4.phase++;
        }
        break;
    case 3:
        models[0] = &lbl_1_bss_60C[72];
        Hu3DModelObjPosGet(models[0]->modelId, "end-cyl47_1", &models[0]->attachment);
        if (lbl_1_bss_1F4.tie == 0) {
            lbl_1_bss_2DC[lbl_1_bss_1F4.losingPlayerIndex].position = models[0]->attachment;
            lbl_1_bss_2DC[lbl_1_bss_1F4.losingPlayerIndex].position.y -= 20.0f;
            lbl_1_bss_2DC[lbl_1_bss_1F4.losingPlayerIndex].position.z += 40.0f;
        }
        if (WipeCheck() == 0) {
            if (lbl_1_bss_1F4.timer == 0) fn_1_67D4(0, 5, 0);
            if (lbl_1_bss_1F4.timer++ > 10) {
                lbl_1_bss_1F4.timer = 0;
                lbl_1_bss_1F4.phase++;
            }
        }
        break;
    case 4:
        models[0] = &lbl_1_bss_60C[72];
        Hu3DModelObjPosGet(models[0]->modelId, "end-cyl47_1", &models[0]->attachment);
        if (lbl_1_bss_1F4.tie == 0) {
            lbl_1_bss_2DC[lbl_1_bss_1F4.losingPlayerIndex].position = models[0]->attachment;
            lbl_1_bss_2DC[lbl_1_bss_1F4.losingPlayerIndex].position.y -= 20.0f;
            lbl_1_bss_2DC[lbl_1_bss_1F4.losingPlayerIndex].position.z += 40.0f;
        }
        fn_1_4B38(models[0]);
        result = 1;
        break;
    }
    return result;
}
