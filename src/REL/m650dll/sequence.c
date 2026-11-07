/* Defines Throw Me a Bone's minigame sequence and end-of-game flow. */
#include "REL/m650/m650.h"
#include "msm_stream.h"

void fn_1_A0(void);
void fn_1_F0(s16 sequenceMode, s16 sequenceFrame);
void fn_1_168(s16 sequenceMode, s16 sequenceFrame);
void fn_1_188(s16 sequenceMode, s16 sequenceFrame);
void fn_1_1C8(s16 sequenceMode, s16 sequenceFrame);
void fn_1_200(s16 sequenceMode, s16 sequenceFrame);
void fn_1_304(s16 sequenceMode, s16 sequenceFrame);
void fn_1_3AC(s16 sequenceMode, s16 sequenceFrame);
void fn_1_3B0(s16 sequenceMode, s16 sequenceFrame);
void fn_1_3B4(s16 sequenceMode, s16 sequenceFrame);
void fn_1_3B8(void);
void fn_1_484(void);
void fn_1_550(void);
void fn_1_8EC(void);
void fn_1_F90(OMOBJ *callbackObject);
void fn_1_1228(void);

extern GXColor lbl_1_data_40[2];
extern Point3d lbl_1_data_28;
extern Point3d lbl_1_data_34;
extern s16 lbl_1_bss_4AC[4];
extern s16 lbl_1_bss_4B4;
extern s16 lbl_1_bss_4B6;
extern s32 lbl_1_bss_2A8;
extern s32 lbl_1_data_48[4];
extern s32 lbl_1_data_80[2];
extern s32 lbl_1_data_88[2];
extern s32 lbl_1_data_90[2];
extern s32 lbl_1_data_98[2];
extern s32 lbl_1_data_A0[2];

MGSEQ_PARAM lbl_1_data_0 = {
    300, 1, fn_1_F0, fn_1_168, fn_1_188, fn_1_1C8, fn_1_200, fn_1_304, fn_1_3AC, fn_1_3B0, fn_1_3B4,
};

/* Reserved sequence storage. */
u8 lbl_1_bss_20[8];
OMOBJMAN *lbl_1_bss_1C;
M650Scene lbl_1_bss_0;

/* Called by the REL startup hook to create the object manager and start the minigame sequence. */
void fn_1_A0(void)
{
    lbl_1_bss_1C = fn_8003A8BC(256, 8192);
    fn_80069F1C(lbl_1_bss_1C);
    fn_80070788(&lbl_1_data_0);
}

/* First-stage callback in lbl_1_data_0 resets round state, creates the scene, and advances
 * setup. */
void fn_1_F0(s16 sequenceMode, s16 sequenceFrame)
{
    s16 isNightCourse;

    memset(&lbl_1_bss_0, 0, sizeof(lbl_1_bss_0));
    isNightCourse = lbl_802C0278;
    lbl_1_bss_0.night = isNightCourse;
    lbl_1_bss_0.winner = -1;
    fn_1_14FC();
    fn_1_1830();
    fn_1_1AE4();
    fn_1_62E0();
    fn_1_5258();
    fn_80071A9C();
}

/* Opening-stage callback in lbl_1_data_0 advances the minigame's intro each frame. */
void fn_1_168(s16 sequenceMode, s16 sequenceFrame)
{
    fn_1_550();
}

/* Start-announcement callback in lbl_1_data_0 starts the music on frame zero. */
void fn_1_188(s16 sequenceMode, s16 sequenceFrame)
{
    if (sequenceFrame == 0) {
        lbl_1_bss_0.audio = HuAudBGMPlay(MSM_STREAM_LAID_BACK);
    }
}

/* lbl_1_data_0 gameplay-entry callback starts the timer UI and player states. */
void fn_1_1C8(s16 sequenceMode, s16 sequenceFrame)
{
    if (sequenceFrame == 0) {
        fn_1_5308();
        fn_1_54D4();
        fn_1_2558();
    }
}

/* lbl_1_data_0 results callback stops the stream and awards the winner when applicable. */
void fn_1_200(s16 sequenceMode, s16 sequenceFrame)
{
    int winnerPlayer;

    if (sequenceFrame == 0) {
        HuAudSStreamFadeOut(lbl_1_bss_0.audio, 100);
        fn_1_25B0();
        if (fn_8006A124(196610) == 0) {
            if (lbl_1_bss_0.winner == -1) {
                fn_80071C70();
            } else {
                fn_80071C18(lbl_1_bss_28[lbl_1_bss_0.winner].charNo, -1, -1, -1);
                winnerPlayer = lbl_1_bss_0.winner;
                if (fn_8006A124(65551) == 0) {
                    lbl_80265750[winnerPlayer].mgCoinBonus = 10;
                }
            }
            lbl_1_bss_0.state = 0;
            lbl_1_bss_0.frame = 0;
        }
    }
}

/* Before winner display, Decathlon copies elapsed times to scores and skips to fade-out;
 * other play advances a draw or prepares the winner scene. */
void fn_1_304(s16 sequenceMode, s16 sequenceFrame)
{
    int playerNo;
    int timerScore;

    if (fn_8006A124(196610) != 0) {
        fn_80071AC4(9);
        for (playerNo = 0; playerNo < 4; playerNo++) {
            timerScore = lbl_1_bss_28[playerNo].timerValue;
            lbl_80265750[playerNo].mgScore = timerScore;
        }
    } else if (lbl_1_bss_0.winner == -1) {
        fn_80071A9C();
    } else {
        fn_1_8EC();
    }
}

void fn_1_3AC(s16 sequenceMode, s16 sequenceFrame) { }

void fn_1_3B0(s16 sequenceMode, s16 sequenceFrame) { }

void fn_1_3B4(s16 sequenceMode, s16 sequenceFrame) { }

/* Called by fn_1_550 when the opening settles; moves all cameras to the wide view. */
void fn_1_3B8(void)
{
    OM_CAMERA_VIEW cameraView;
    int cameraIndex;

    cameraView.center.x = 0.0f;
    cameraView.center.y = 150.0f;
    cameraView.center.z = 0.0f;
    cameraView.rot.x = -15.0f;
    cameraView.rot.y = -180.0f;
    cameraView.rot.z = 0.0f;
    cameraView.zoom = 1000.0f;
    for (cameraIndex = 0; cameraIndex < 4; cameraIndex++) {
        fn_8003E62C(lbl_1_data_150[cameraIndex], &cameraView, 120, 2);
    }
}

/* Called by fn_1_550 after the viewport split; moves all cameras to the final opening pose. */
void fn_1_484(void)
{
    OM_CAMERA_VIEW cameraView;
    int cameraIndex;

    cameraView.center.x = 0.0f;
    cameraView.center.y = 300.0f;
    cameraView.center.z = 0.0f;
    cameraView.rot.x = -5.0f;
    cameraView.rot.y = -180.0f;
    cameraView.rot.z = 0.0f;
    cameraView.zoom = 1300.0f;
    for (cameraIndex = 0; cameraIndex < 4; cameraIndex++) {
        fn_8003E62C(lbl_1_data_150[cameraIndex], &cameraView, 60, 0);
    }
}

/* Called each opening frame by fn_1_168; holds the opening phase while advancing camera and
 * viewport steps. */
void fn_1_550(void)
{
    HuVecF shadowPosition;
    HuVecF shadowTarget;
    HuVecF shadowUp;

    switch (lbl_1_bss_0.state) {
    case 0:
        fn_80071B08();
        lbl_1_bss_0.state++;
        break;
    case 1:
        if (lbl_1_bss_0.frame++ >= 60) {
            lbl_1_bss_0.frame = 0;
            lbl_1_bss_0.state++;
            fn_1_3B8();
        } else if (lbl_1_bss_0.frame == 30) {
            HuAudFXPlay(M650_EFFECT_1001);
        }
        break;
    case 2:
        if (lbl_1_bss_0.frame++ >= 60) {
            lbl_1_bss_0.frame = 0;
            lbl_1_bss_0.state++;
        }
        break;
    case 3:
        if (fn_1_4D58(lbl_1_bss_0.frame++)) {
            fn_1_484();
            lbl_1_bss_0.frame = 0;
            lbl_1_bss_0.state++;
            shadowPosition.x = -1500.0f;
            shadowPosition.y = 9000.0f;
            shadowPosition.z = -2000.0f;
            shadowUp.x = 0.0f;
            shadowUp.y = 1.0f;
            shadowUp.z = 0.0f;
            shadowTarget.x = shadowTarget.y = 0.0f;
            shadowTarget.z = 2500.0f;
            fn_8002A674(&shadowPosition, &shadowUp, &shadowTarget, 15);
        }
        break;
    case 4:
        if (lbl_1_bss_0.frame++ >= 60) {
            fn_1_5394();
            lbl_1_bss_0.frame = 0;
            fn_80071A9C();
        }
        break;
    }
}

/* Called by fn_1_304 after a win; reveals the winner and runs the record-break sequence. */
void fn_1_8EC(void)
{
    HuVecF shadowPos;
    HuVecF shadowTarget;
    HuVecF shadowUp;
    M650Player *playerWork;
    s32 playerNo;
    s32 motionIndex;

    switch (lbl_1_bss_0.state) {
    case 0:
        fn_80071B08();
        WipeCreate(2, 0, 60);
        lbl_1_bss_0.frame = 0;
        lbl_1_bss_0.state += 1;
        return;
    case 1:
        if (WipeCheck() != 0) {
            break;
        }
            lbl_1_bss_0.resultsScene = 1;
            fn_1_6F54();
            fn_1_26D0();
            if (lbl_1_bss_0.recordChanged != 0) {
                fn_1_1228();
            }
            shadowPos.x = -1000.0f;
            shadowPos.y = 5000.0f;
            shadowPos.z = -1000.0f;
            shadowUp.x = 0.0f;
            shadowUp.y = 1.0f;
            shadowUp.z = 0.0f;
            shadowTarget.x = shadowTarget.y = 0.0f;
            shadowTarget.z = 2500.0f;
            fn_8002A674(&shadowPos, &shadowUp, &shadowTarget, 15);
            for (playerNo = 0; playerNo < 4; playerNo++) {
                playerWork = &lbl_1_bss_28[playerNo];
                if (playerNo == lbl_1_bss_0.winner) {
                    fn_8005BB8C(playerWork->charNo);
                    playerWork->model = fn_80057068(playerWork->charNo, 2);
                    motionIndex = 0;
                    while (motionIndex < 5) {
                        playerWork->motionIDs[motionIndex] = fn_8005B0B8(
                            playerWork->charNo, (u32) lbl_1_data_58[motionIndex].file);
                        motionIndex += 1;
                    }
                }
                fn_80026E44(playerWork->model);
                if (lbl_1_bss_0.recordChanged == 0) {
                    fn_80026760(playerWork->model20, -200.0f, 0.0f, 1200.0f);
                    fn_80026808(playerWork->model20, 0.0f, 170.0f, 0.0f);
                    fn_80026A44(playerWork->model52, (1 << 0));
                    fn_80026760(playerWork->model52, -180.0f, 5.0f, 1100.0f);
                    fn_80026A44(playerWork->model54, (1 << 0));
                    fn_80026760(playerWork->model54, -180.0f, 0.2f, 1100.0f);
                    fn_8002C95C(playerWork->model20, playerWork->jointMotionIDs[3]);
                }
                fn_1_23C4((s16) playerNo, 3, 1.0f);
                lbl_80266B18[playerNo].x = 0.0f;
                lbl_80266B18[playerNo].y = 150.0f;
                lbl_80266B18[playerNo].z = 900.0f;
                lbl_80266A58[playerNo].x = -5.0f;
                lbl_80266A58[playerNo].y = 180.0f;
                lbl_80266A58[playerNo].z = 0.0f;
                lbl_80266BD8[playerNo] = 800.0f;
                if (lbl_1_bss_0.winner == playerNo) {
                    Hu3DCameraScissorSet((s32) lbl_1_data_150[playerNo], 0U, 0U, 640U, 480U);
                    Hu3DCameraViewportSet((s32) lbl_1_data_150[playerNo], 0.0f, 0.0f, 640.0f,
                                          480.0f, 0.0f, 1.0f);
                } else {
                    Hu3DCameraScissorSet((s32) lbl_1_data_150[playerNo], 0U, 0U, 0U, 0U);
                    Hu3DCameraViewportSet((s32) lbl_1_data_150[playerNo], 0.0f, 0.0f, 0.0f, 0.0f,
                                          0.0f, 1.0f);
                }
            }
            lbl_1_bss_0.state += 1;
        break;
    case 2:
        WipeCreate(1, 5, 60);
        lbl_1_bss_0.state += 1;
        return;
    case 3:
        if (WipeCheck() == 0) {
            if (lbl_1_bss_0.recordChanged == 0) {
                fn_1_2320(lbl_1_bss_0.winner, 4);
                fn_80071A9C();
                return;
            }
            MgTimerRecordSet(lbl_1_bss_0.timer, -1);
            lbl_1_bss_0.state += 1;
            return;
        }
        break;
    case 4:
        if ((f32) lbl_1_bss_0.frame++ > 150.0f) {
            fn_1_2320(lbl_1_bss_0.winner, 4);
            fn_80071A9C();
            lbl_1_bss_0.frame = 0;
            lbl_1_bss_0.state = 11;
        }
        if (lbl_1_bss_0.frame == 110) {
            lbl_1_bss_0.winnerAnimationState = 1;
            if (lbl_1_bss_0.night != 0) {
                HuAudFXPlayPan(M650_EFFECT_2036, 96);
                return;
            }
            HuAudFXPlayPan(M650_EFFECT_2036, 48);
        }
        break;
    }
}

/* Object callback installed by fn_1_1228; bounces the winner's bone, then moves the body model
 * sideways each frame. */
void fn_1_F90(OMOBJ *object)
{
    Point3d modelPosition;
    M650Player *winnerWork;
    s16 winner;

    winner = lbl_1_bss_0.winner;
    winnerWork = &lbl_1_bss_28[winner];
    switch (lbl_1_bss_0.winnerAnimationState) {
    case 1:
        fn_800267CC(winnerWork->model52, &modelPosition);
        modelPosition.x += winnerWork->boneVelocity.x;
        modelPosition.y += winnerWork->boneVelocity.y;
        modelPosition.z += winnerWork->boneVelocity.z;
        if (modelPosition.y <= 0.5f) {
            /* The landing sound pans with the bouncing bone's horizontal position. */
            HuAudFXPlayPan(M650_EFFECT_2036, (s16) (64.0f + (modelPosition.x / 50.0f)));
            winnerWork->boneVelocity.x *= 0.5;
            winnerWork->boneVelocity.y *= -0.5;
            winnerWork->boneVelocity.z *= 0.5;
            modelPosition.y = 0.5f;
            if (winnerWork->boneVelocity.y < 1.0f) {
                lbl_1_bss_0.winnerAnimationState += 1;
                if (lbl_1_bss_0.night == 0) {
                    HuAudFXPlayPan(M650_EFFECT_1001, 96);
                } else {
                    HuAudFXPlayPan(M650_EFFECT_1001, 32);
                }
            }
        }
        winnerWork->boneVelocity.y -= 1.6333333f;
        fn_80026790(winnerWork->model52, &modelPosition);
        fn_80026760(winnerWork->model54, modelPosition.x, 0.2f, modelPosition.z);
        break;
    case 2:
        fn_800267CC(winnerWork->model20, &modelPosition);
        if (winnerWork->movementSpeed < 10.0f) {
            winnerWork->movementSpeed += 0.2f;
        }
        modelPosition.x += winnerWork->movementSpeed * (f32) (1 - (lbl_1_bss_0.night * 2));
        fn_80026790(winnerWork->model20, &modelPosition);
        break;
    }
}

/* Called during the results reveal after a new record; prepares the winner's bone bounce and
 * installs the callback that later moves the body model sideways. */
void fn_1_1228(void)
{
    OMOBJ *bounceObject;
    M650Player *winnerWork;
    s16 winner;

    winner = lbl_1_bss_0.winner;
    winnerWork = &lbl_1_bss_28[winner];
    /* The returned object-manager handle is ignored after registering the callback. */
    bounceObject = fn_8003AB38(lbl_1_bss_1C, 100, 0U, 0U, -1, fn_1_F90);
    fn_80026760(winnerWork->model20, -700.0f + (f32) (lbl_1_bss_0.night * 1400), 0.0f, 1300.0f);
    fn_80026808(winnerWork->model20, 0.0f, 90.0f + (180.0f * (f32) lbl_1_bss_0.night), 0.0f);
    fn_8002C95C(winnerWork->model20, winnerWork->jointMotionIDs[1]);
            fn_80026A44(winnerWork->model52, (1 << 0));
    fn_80026760(winnerWork->model52, -550.0f + (1100.0f * (f32) lbl_1_bss_0.night), 200.0f,
                    1300.0f);
            fn_80026A44(winnerWork->model54, (1 << 0));
    fn_80026760(winnerWork->model54, -550.0f + (1100.0f * (f32) lbl_1_bss_0.night), 0.2f,
                    1300.0f);
    winnerWork->boneVelocity.x = 30.0f - (60.0f * (f32) lbl_1_bss_0.night);
    winnerWork->boneVelocity.y = 15.0f;
    winnerWork->boneVelocity.z = 0.0f;
    winnerWork->movementSpeed = 0.0f;
}
