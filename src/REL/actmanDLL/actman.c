/* This REL creates a ground map and four players, then updates the scene camera through MG sequence
 * callbacks. */
#include "dolphin.h"
#include "game/gamework.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "datanum/actman.h"

static void SeqModeInit(s16 mode, s16 frameNo);
static void SeqModeFadeIn(s16 mode, s16 frameNo);
static void SeqModeStart(s16 mode, s16 frameNo);
static void SeqModeMain(s16 mode, s16 frameNo);
static void SeqModeFinish(s16 mode, s16 frameNo);
static void SeqModePreWin(s16 mode, s16 frameNo);
static void SeqModeWin(s16 mode, s16 frameNo);
static void SeqModeFadeOut(s16 mode, s16 frameNo);
static void SeqModeClose(s16 mode, s16 frameNo);

/* MgSeqCreate copies this table and calls each hook during its corresponding sequence mode. */
static MGSEQ_PARAM seqParam = {
    0,
    MGSEQ_TIMER_TOP,
    SeqModeInit,
    SeqModeFadeIn,
    SeqModeStart,
    SeqModeMain,
    SeqModeFinish,
    SeqModePreWin,
    SeqModeWin,
    SeqModeFadeOut,
    SeqModeClose
};

/* Player objects created for the scene; player zero drives the camera and winner animation. */
static MGPLAYER *player[GW_PLAYER_MAX];
/* Actor manager used to schedule the map and manage scene objects. */
static OMOBJMAN *objman;

static void CreateMap(OMOBJ *obj);

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern void ObjectSetup(void);

/* REL loader entry point: run static constructors, then initialize this scene. */
int _prolog(void) {
    const VoidFunc* ctors = _ctors;
    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    ObjectSetup();
    return 0;
}

/* REL loader exit point: run the registered static destructors before unloading. */
void _epilog(void) {
    const VoidFunc* dtors = _dtors;
    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}

/* Called by the REL prolog to create the actor manager, camera, light, map callback, and
 * sequence. */
void ObjectSetup(void)
{
    HU3D_LIGHTID lightId;

    objman = MgActorObjectSetup();
    CRot.x = -20;
    CRot.y = 0;
    CRot.z = 0;
    Center.x = 0;
    Center.y = 150;
    Center.z = 0;
    CZoom = 1000;
    Hu3DCameraCreate(HU3D_CAM0);
    Hu3DCameraPerspectiveSet(HU3D_CAM0, 45, 20, 15000, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM0, 0, 0, 640, 480, 0, 1);
    omAddObj(objman, 32730, 0, 0, omOutView);

    lightId = Hu3DGLightCreate(0, 1000, 1000, 0, -1, -1, 255, 255, 255);
    omAddObj(objman, 10, 3, 0, CreateMap);
    MgSeqCreate(&seqParam);
}

/* Actor-manager callback that loads the ground collision map and creates and positions four
 * players. */
static void CreateMap(OMOBJ *obj)
{
    MGACTOR_PARAM param;
    HU3D_MODELID mdlId[4];

    obj->mdlId[0] = mdlId[0] = Hu3DModelCreateData(ACTMAN_HSF_ground);
    param.height = 150;
    param.radius = 40;
    param.param = 0;
    param.type = 0;
    param.attr = 0;
    param.correctHookParam = 0;
    param.narrowHook = NULL;
    param.correctHook = NULL;
    MgActorColMapInit(mdlId, 1, 40);
    player[0] = MgPlayerCreate(0, &param, CHAR_MODEL0, HU3D_CAM0, ~MGPLAYER_ACTFLAG_KICK, NULL);
    param.param++;
    player[1] = MgPlayerCreate(1, &param, CHAR_MODEL0, HU3D_CAM0, ~MGPLAYER_ACTFLAG_KICK, NULL);
    param.param++;
    player[2] = MgPlayerCreate(2, &param, CHAR_MODEL0, HU3D_CAM0, ~MGPLAYER_ACTFLAG_KICK, NULL);
    param.param++;
    player[3] = MgPlayerCreate(3, &param, CHAR_MODEL0, HU3D_CAM0, ~MGPLAYER_ACTFLAG_KICK, NULL);
    param.param++;
    player[0]->actor->pos.x = -300.211f;
    player[0]->actor->pos.y = 300;
    player[0]->actor->pos.z = -85.369f;
    player[1]->actor->pos.x = 200;
    player[1]->actor->pos.y = 300;
    player[1]->actor->pos.z = 0;
    player[2]->actor->pos.x = 4300;
    player[2]->actor->pos.y = 1000;
    player[2]->actor->pos.z = 0;
    player[3]->actor->pos.x = 400;
    player[3]->actor->pos.y = 300;
    player[3]->actor->pos.z = 110;
    obj->objFunc = NULL;
}

/* MG sequence init hook: request the next mode and run one actor update. */
static void SeqModeInit(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
    MgActorExec();
}

/* MG sequence fade-in hook: request the next mode after frame 90. */
static void SeqModeFadeIn(s16 mode, s16 frameNo)
{
    if(frameNo > 90) {
        MgSeqModeNext();
    }
}

/* MG sequence start-message hook: advance as soon as the start message is active. */
static void SeqModeStart(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}

/* MG sequence main hook, called each gameplay frame to update actors, camera, and the exit
 * input. */
static void SeqModeMain(s16 mode, s16 frameNo)
{
    MgActorExec();
    CZoom += HuPadTrigL[0]/2;
    CZoom -= HuPadTrigR[0]/2;
    Center.x = player[0]->actor->pos.x;
    Center.y = player[0]->actor->pos.y+100;
    Center.z = player[0]->actor->pos.z;
    if(HuPadBtnDown[0] & PAD_BUTTON_X) {
        MgSeqModeNext();
    }
}

/* MG sequence finish-message hook: request the following sequence mode immediately. */
static void SeqModeFinish(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}

/* MG sequence pre-winner hook: request the winner mode immediately. */
static void SeqModePreWin(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}

/* MG sequence winner hook: start player zero's motion on frame one, then raise its model each
 * frame. */
static void SeqModeWin(s16 mode, s16 frameNo)
{
    if(frameNo == 1) {
        int motId = CharMotionCreate(player[0]->charNo, CHARMOT_HSF_c000m1_306);
        Hu3DModelPosSet(player[0]->actor->mdlId, Center.x, Center.y, Center.z);
        CharMotionShiftSet(player[0]->actor->mdlId, motId, 0, 5, HU3D_MOTATTR_LOOP);
    } else {
        Hu3DModelPosSet(player[0]->actor->mdlId, Center.x, Center.y+(frameNo*2), Center.z);
    }
}

/* MG sequence fade-out hook: request the close mode at frame 60. */
static void SeqModeFadeOut(s16 mode, s16 frameNo)
{
    if(frameNo >= 60) {
        MgSeqModeNext();
    }
}

/* Called once in close mode before overlay return; sets the sequence manager's next-mode
 * request. */
static void SeqModeClose(s16 mode, s16 frameNo)
{
    MgSeqModeNext();
}
