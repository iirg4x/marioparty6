/* Sequence manager example: player zero presses A to trigger ten jumps with the configured
 * character. */
#include <math.h>
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
    1,
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
    objman = fn_8003A8BC(50, 8192);
    fn_80069F1C(objman);
    lbl_80266A40.x = -20;
    lbl_80266A40.y = 0;
    lbl_80266A40.z = 0;
    lbl_80266A4C.x = 0;
    lbl_80266A4C.y = 150;
    lbl_80266A4C.z = 0;
    lbl_802C028C = 500;
    Hu3DCameraCreate((1 << 0));
    Hu3DCameraPerspectiveSet((1 << 0), 45, 20, 10000, 1.2f);
    Hu3DCameraViewportSet((1 << 0), 0, 0, 640, 480, 0, 1);
    fn_8003AB38(objman, 32730, 0, 0, -1, fn_8003D4FC);
    lightId = Hu3DGLightCreate(0, 1000, 1000, 0, -1, -1, 255, 255, 255);
    Hu3DGLightInfinitytSet(lightId);
    fn_8003AB38(objman, 32730, 0, 0, -1, PlayerObjExec);
    fn_80070788(&seqParam);
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

    padNo = lbl_80265728[0].padNo;
    charNo = lbl_80265728[0].charNo;
    charMdlId = fn_8005C4E4(charNo, (1 << 0), motFileTbl, charMotId);
    fn_80026760(charMdlId, 0, 0, 0);
    fn_8005BE80(charNo, charMotId[0]);
    fn_800269E8(charMdlId, ((1 << 30) | (1 << 0)));
    fn_80071A9C();
}

/* Fade-in hook: lowers CZoom from 5500 to 500 over 60 frames; the manager shows its start message
 * before main mode. */
static void SeqModeFadeIn(s16 mode, s16 frameNo)
{
    lbl_802C028C = ((1-sin(3.141592653589793*((frameNo/60.0)*90)/180.0))*5000)+500;
    if(frameNo >= 60) {
        fn_80071A9C();
    }
}

/* Main-mode hook: after ten jump starts, reports Mario as winner and requests MAIN-to-FINISH. */
static void SeqModeMain(s16 mode, s16 frameNo)
{
    if(jumpNum >= 10) {
        /* The result is fixed to Mario even if player zero is configured with another character. */
        fn_80071C18(0, -1, -1, -1);
        /* The sequence manager suppresses this record request in practice mode. */
        fn_80071BAC(10);
        fn_80071A9C();
    }
}

/* Pre-winner hook: animates the camera through frame 60; afterward it requests advance if fewer
 * than ten jumps were started. Otherwise the manager uses its phase duration, normally 240 frames
 * but extended to 270 when a record is displayed. The frame count can carry over from FINISH. */
static void SeqModePreWin(s16 mode, s16 frameNo)
{
    if(frameNo > 60) {
        if(jumpNum < 10) {
            fn_80071A9C();
        }
        return;
    }
    lbl_80266A40.x = -20*cos(3.141592653589793*((frameNo/60.0)*90)/180.0);
    lbl_80266A4C.y = 150-(50*sin(3.141592653589793*((frameNo/60.0)*90)/180.0));
    lbl_802C028C = 500-(sin(3.141592653589793*((frameNo/60.0)*90)/180.0)*200);
}

/* Winner-stage hook: at frame 0, plays victory after ten jumps or draw otherwise, without setting
 * the sequence manager's draw result. */
static void SeqModeWin(s16 mode, s16 frameNo)
{
    if(frameNo == 0) {
        if(jumpNum >= 10) {
            fn_8005C2C8(charNo, charMotId[3], 0, 8, 0);
            HuAudJinglePlay(MSM_STREAM_MGMUS_17);
        } else {
            fn_8005C2C8(charNo, charMotId[4], 0, 8, 0);
            HuAudJinglePlay(MSM_STREAM_FILESEL);
        }
    }
}

/* Per-frame player update: counts an A press only in MAIN while no jump is active; the count
 * increases when the jump starts. The model follows a 30-frame sine arc, switches to landing at
 * timer 28, then returns to idle and resets the timer after 30. */
static void PlayerObjExec(OMOBJ *playerObj)
{
    print8(8, 24, 2.0f, "%d", jumpNum);
    if (jumpTimer == -1 && fn_80071A8C() == 5 &&
        (lbl_802C0008[padNo] & PAD_BUTTON_A)) {
        jumpNum++;
        jumpTimer = 0;
        fn_8005C2C8(charNo, charMotId[1], 0, 8, 0);
    }
    if(jumpTimer >= 0) {
        fn_80026760(charMdlId, 0, 150*sin(3.141592653589793*((jumpTimer/30.0)*180.0)/180.0), 0);
        jumpTimer++;
        if(jumpTimer == 28) {
            fn_8005C2C8(charNo, charMotId[2], 0, 8, 0);
        } else if(jumpTimer > 30) {
            fn_8005C2C8(charNo, charMotId[0], 0, 8, 0);
            jumpTimer = -1;
        }
    }
}
