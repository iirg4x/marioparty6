/* This REL creates a ground map and four players, then updates the scene camera through MG sequence
 * callbacks. */
#include <math.h>
#include "dolphin.h"
#include "game/gamework.h"
#include "game/mg/actman.h"
#include "game/charman.h"
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
    0, 0, SeqModeInit, SeqModeFadeIn,
    SeqModeStart, SeqModeMain, SeqModeFinish, SeqModePreWin,
    SeqModeWin, SeqModeFadeOut, SeqModeClose
};

/* Player objects created for the scene; player zero drives the camera, and the winner hook passes
 * its model ID as CharMotionShiftSet's character number. */
static MGPLAYER *player[4];
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

    objman = fn_80080418();
    lbl_80266A40.x = -20;
    lbl_80266A40.y = 0;
    lbl_80266A40.z = 0;
    lbl_80266A4C.x = 0;
    lbl_80266A4C.y = 150;
    lbl_80266A4C.z = 0;
    lbl_802C028C = 1000;
    Hu3DCameraCreate((1 << 0));
    Hu3DCameraPerspectiveSet((1 << 0), 45, 20, 15000, 1.2f);
    Hu3DCameraViewportSet((1 << 0), 0, 0, 640, 480, 0, 1);
    fn_8003AB38(objman, 32730, 0, 0, -1, fn_8003D4FC);

    lightId = Hu3DGLightCreate(0, 1000, 1000, 0, -1, -1, 255, 255, 255);
    fn_8003AB38(objman, 10, 3, 0, -1, CreateMap);
    fn_80070788(&seqParam);
}

/* Actor-manager callback that loads the ground collision map and creates and positions four
 * players. */
static void CreateMap(OMOBJ *obj)
{
    MGACTOR_PARAM param;
    HU3D_MODELID mdlId[4];

    obj->mdlId[0] = mdlId[0] = fn_80025A04(fn_80008DD0(ACTMAN_HSF_ground, 0x10000000, 2));
    param.height = 150;
    param.radius = 40;
    param.param = 0;
    param.type = 0;
    param.attr = 0;
    param.correctHookParam = 0;
    param.narrowHook = NULL;
    param.correctHook = NULL;
    fn_800804E8(mdlId, 1, 40);
    player[0] = fn_80080728(0, &param, (1 << 0), (1 << 0), ~(1 << 3), NULL);
    param.param++;
    player[1] = fn_80080728(1, &param, (1 << 0), (1 << 0), ~(1 << 3), NULL);
    param.param++;
    player[2] = fn_80080728(2, &param, (1 << 0), (1 << 0), ~(1 << 3), NULL);
    param.param++;
    player[3] = fn_80080728(3, &param, (1 << 0), (1 << 0), ~(1 << 3), NULL);
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
    fn_80071A9C();
    fn_800856A8();
}

/* MG sequence fade-in hook: request the next mode after frame 90. */
static void SeqModeFadeIn(s16 mode, s16 frameNo)
{
    if(frameNo > 90) {
        fn_80071A9C();
    }
}

/* MG sequence start-message hook: advance as soon as the start message is active. */
static void SeqModeStart(s16 mode, s16 frameNo)
{
    fn_80071A9C();
}

/* MG sequence main hook, called each gameplay frame to update actors, camera, and the exit
 * input. */
static void SeqModeMain(s16 mode, s16 frameNo)
{
    fn_800856A8();
    lbl_802C028C += lbl_802BFFEC[0]/2;
    lbl_802C028C -= lbl_802BFFE8[0]/2;
    lbl_80266A4C.x = player[0]->actor->pos.x;
    lbl_80266A4C.y = player[0]->actor->pos.y+100;
    lbl_80266A4C.z = player[0]->actor->pos.z;
    if(lbl_802C0008[0] & PAD_BUTTON_X) {
        fn_80071A9C();
    }
}

/* MG sequence finish-message hook: request the following sequence mode immediately. */
static void SeqModeFinish(s16 mode, s16 frameNo)
{
    fn_80071A9C();
}

/* MG sequence pre-winner hook: request the winner mode immediately. */
static void SeqModePreWin(s16 mode, s16 frameNo)
{
    fn_80071A9C();
}

/* MG sequence winner hook: on frame one, create player zero's win motion, pass its model ID as
 * CharMotionShiftSet's character number, and place its model at Center; on other frames, set its
 * model Y position to Center.y plus twice the frame number. */
static void SeqModeWin(s16 mode, s16 frameNo)
{
    if(frameNo == 1) {
        int motId = fn_8005B0B8(player[0]->charNo, CHARMOT_HSF_c000m1_306);
        fn_80026760(player[0]->actor->mdlId, lbl_80266A4C.x, lbl_80266A4C.y, lbl_80266A4C.z);
        fn_8005C2C8(player[0]->actor->mdlId, motId, 0, 5, ((1 << 30) | (1 << 0)));
    } else {
        fn_80026760(player[0]->actor->mdlId, lbl_80266A4C.x, lbl_80266A4C.y+(frameNo*2),
            lbl_80266A4C.z);
    }
}

/* MG sequence fade-out hook: the request at frame 60 is not reached with the default 60-frame
 * phase, whose hook runs on frames 0 through 59. */
static void SeqModeFadeOut(s16 mode, s16 frameNo)
{
    if(frameNo >= 60) {
        fn_80071A9C();
    }
}

/* Close hook: on the CLOSE and EXIT paths, sets the next-mode request, which is not consumed
 * before either path returns to the overlay. */
static void SeqModeClose(s16 mode, s16 frameNo)
{
    fn_80071A9C();
}
