/* Sequence manager example: player zero presses A to trigger ten jumps with the configured
 * character. */
#include "dolphin.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/gamework.h"
#include "game/audio.h"
#include "game/pad.h"
#include "game/printfunc.h"
#include "game/charman.h"
#include "game/mg/seqman.h"
#include "humath.h"

enum {
    SEQUENCE_JINGLE_WIN = MSM_STREAM_MGMUS_17,
    SEQUENCE_JINGLE_DRAW = MSM_STREAM_FILESEL,
};

#define SEQUENCE_OBJMAN_PRIORITY 8192
#define SEQUENCE_JUMP_TARGET 10

static void SeqModeInit(s16 mode, s16 frameNo);
static void SeqModeFadeIn(s16 mode, s16 frameNo);
static void SeqModeMain(s16 mode, s16 frameNo);
static void SeqModePreWin(s16 mode, s16 frameNo);
static void SeqModeWin(s16 mode, s16 frameNo);
static void PlayerObjExec(OMOBJ *playerObj);

/* Object manager that receives the scene and player update objects during ObjectSetup. */
static OMOBJMAN *objman;
/* Model and motion handles for the configured player; the motion table below assigns their
 * roles. */
static HU3D_MODELID charMdlId;
static HU3D_MOTIONID charMotId[5];
/* Character and controller selected from player slot zero when the sequence initializes. */
static s16 charNo;
static s16 padNo;
/* Number of accepted A-button jump starts during the main mode. */
static s16 jumpNum;

/* -1 means no jump is active; nonnegative values count frames since the current jump began. */
static s16 jumpTimer = -1;
/* The sequence manager calls these hooks for initialization, fade-in, play, pre-winner, and winner
 * stages. */
static MGSEQ_PARAM seqParam = {
    10,
    MGSEQ_TIMER_BOTTOM,
    SeqModeInit,
    SeqModeFadeIn,
    NULL,
    SeqModeMain,
    NULL,
    SeqModePreWin,
    SeqModeWin,
    NULL,
    NULL
};

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern void ObjectSetup(void);

/* The REL loader calls this entry point to run static constructors and create the sequence
 * objects. */
int _prolog(void) {
    const VoidFunc* ctors = _ctors;
    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    ObjectSetup();
    return 0;
}

/* The REL loader calls this entry point during unload to run static destructors. */
void _epilog(void) {
    const VoidFunc* dtors = _dtors;
    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}

/* Creates the 3D scene, white infinite light, player update object, and sequence manager after REL
 * initialization. */
void ObjectSetup(void)
{
    HU3D_LIGHTID lightId;
    OSReport("******* Sequence Manager Example ObjectSetup *********\n");
    objman = omInitObjMan(50, SEQUENCE_OBJMAN_PRIORITY);
    omGameSysInit(objman);
    CRot.x = -20;
    CRot.y = 0;
    CRot.z = 0;
    Center.x = 0;
    Center.y = 150;
    Center.z = 0;
    CZoom = 500;
    Hu3DCameraCreate(HU3D_CAM0);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 45, 20, 10000, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM0, 0, 0, 640, 480, 0, 1);
    omAddObj(objman, OM_OUTVIEW_PRIO, 0, 0, omOutView);
    lightId = Hu3DGLightCreate(0, 1000, 1000, 0, -1, -1, 255, 255, 255);
    Hu3DGLightInfinitytSet(lightId);
    omAddObj(objman, 32730, 0, 0, PlayerObjExec);
    MgSeqCreate(&seqParam);
}

/* Sequence-manager init hook: loads the configured player's character and five motions, then enters
 * fade-in. */
static void SeqModeInit(s16 mode, s16 frameNo)
{
    /* Idle, jump start, jump landing, victory, and draw motions used by the callbacks below. */
    static unsigned int motFileTbl[] = {
        CHARMOT_HSF_c000m1_300,
        CHARMOT_HSF_c000m1_303,
        CHARMOT_HSF_c000m1_304,
        CHARMOT_HSF_c000m1_306,
        CHARMOT_HSF_c000m1_307,
        0
    };

    padNo = GwPlayerConf[0].padNo;
    charNo = GwPlayerConf[0].charNo;
    charMdlId = CharModelMotListCreate(charNo, CHAR_MODEL0, motFileTbl, charMotId);
    Hu3DModelPosSet(charMdlId, 0, 0, 0);
    CharMotionSet(charNo, charMotId[0]);
    Hu3DModelAttrSet(charMdlId, HU3D_MOTATTR_LOOP);
    MgSeqModeNext();
}

/* Fade-in hook: lowers CZoom from 5500 to 500 over 60 frames; the manager shows its start message
 * before main mode. */
static void SeqModeFadeIn(s16 mode, s16 frameNo)
{
    CZoom = ((1-HuSin((frameNo/60.0)*90))*5000)+500;
    if(frameNo >= 60) {
        MgSeqModeNext();
    }
}

/* Main-mode hook: after ten jump starts, reports Mario as winner and requests MAIN-to-FINISH. */
static void SeqModeMain(s16 mode, s16 frameNo)
{
    if(jumpNum >= SEQUENCE_JUMP_TARGET) {
        /* The result is fixed to Mario even if player zero is configured with another character. */
        MgSeqWinnerSet1(CHARNO_MARIO);
        /* The sequence manager suppresses this record request in practice mode. */
        MgSeqRecordSet(SEQUENCE_JUMP_TARGET);
        MgSeqModeNext();
    }
}

/* Pre-winner hook: animates the camera through frame 60; afterward it requests an early advance
 * only below ten jump starts, while the manager advances automatically at its 240-frame timeout. */
static void SeqModePreWin(s16 mode, s16 frameNo)
{
    if(frameNo > 60) {
        if(jumpNum < SEQUENCE_JUMP_TARGET) {
            MgSeqModeNext();
        }
        return;
    }
    CRot.x = -20*HuCos((frameNo/60.0)*90);
    Center.y = 150-(50*HuSin((frameNo/60.0)*90));
    CZoom = 500-(HuSin((frameNo/60.0)*90)*200);
}

/* Winner-stage hook: at frame 0, plays victory after ten jumps or draw otherwise, without setting
 * the sequence manager's draw result. */
static void SeqModeWin(s16 mode, s16 frameNo)
{
    if(frameNo == 0) {
        if(jumpNum >= SEQUENCE_JUMP_TARGET) {
            CharMotionShiftSet(charNo, charMotId[3], 0, 8, HU3D_MOTATTR_NONE);
            HuAudJinglePlay(SEQUENCE_JINGLE_WIN);
        } else {
            CharMotionShiftSet(charNo, charMotId[4], 0, 8, HU3D_MOTATTR_NONE);
            HuAudJinglePlay(SEQUENCE_JINGLE_DRAW);
        }
    }
}

/* Per-frame player update: counts an A press only in MAIN while no jump is active; the count
 * increases when the jump starts. The model follows a 30-frame sine arc, switches to landing at
 * timer 28, then returns to idle and resets the timer after 30. */
static void PlayerObjExec(OMOBJ *playerObj)
{
    print8(8, 24, 2.0f, "%d", jumpNum);
    if (jumpTimer == -1 && MgSeqModeGet() == MGSEQ_MODE_MAIN &&
        (HuPadBtnDown[padNo] & PAD_BUTTON_A)) {
        jumpNum++;
        jumpTimer = 0;
        CharMotionShiftSet(charNo, charMotId[1], 0, 8, HU3D_MOTATTR_NONE);
    }
    if(jumpTimer >= 0) {
        Hu3DModelPosSet(charMdlId, 0, 150*HuSin((jumpTimer/30.0)*180.0), 0);
        jumpTimer++;
        if(jumpTimer == 28) {
            CharMotionShiftSet(charNo, charMotId[2], 0, 8, HU3D_MOTATTR_NONE);
        } else if(jumpTimer > 30) {
            CharMotionShiftSet(charNo, charMotId[0], 0, 8, HU3D_MOTATTR_NONE);
            jumpTimer = -1;
        }
    }
}
