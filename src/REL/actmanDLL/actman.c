/* This REL creates a ground map and four players, then updates the scene camera through MG sequence
 * callbacks. */
#include <math.h>
#include "dolphin.h"
#include "game/gamework.h"
#include "game/mg/actman.h"
#include "game/charman.h"
#include "game/mg/seqman.h"
#include "datanum/actman.h"

static void fn_1_5BC(s16 mode, s16 frameNo);
static void fn_1_5E0(s16 mode, s16 frameNo);
static void fn_1_610(s16 mode, s16 frameNo);
static void fn_1_630(s16 mode, s16 frameNo);
static void fn_1_78C(s16 mode, s16 frameNo);
static void fn_1_7AC(s16 mode, s16 frameNo);
static void fn_1_7CC(s16 mode, s16 frameNo);
static void fn_1_91C(s16 mode, s16 frameNo);
static void fn_1_94C(s16 mode, s16 frameNo);

/* fn_80070788 copies this table and calls each hook during its corresponding sequence mode. */
static MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_5BC, fn_1_5E0,
    fn_1_610, fn_1_630, fn_1_78C, fn_1_7AC,
    fn_1_7CC, fn_1_91C, fn_1_94C
};

/* Player objects created for the scene; player zero drives the camera, and the winner hook passes
 * its model ID as fn_8005C2C8's character number. */
static MGPLAYER *lbl_1_bss_4[4];
/* Actor manager used to schedule the map and manage scene objects. */
static OMOBJMAN *lbl_1_bss_0;

static void fn_1_2BC(OMOBJ *obj);

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

    lbl_1_bss_0 = fn_80080418();
    lbl_80266A40.x = -20;
    lbl_80266A40.y = 0;
    lbl_80266A40.z = 0;
    lbl_80266A4C.x = 0;
    lbl_80266A4C.y = 150;
    lbl_80266A4C.z = 0;
    lbl_802C028C = 1000;
    Hu3DCameraCreate(1 << 0);
    Hu3DCameraPerspectiveSet(1 << 0, 45, 20, 15000, 1.2f);
    Hu3DCameraViewportSet(1 << 0, 0, 0, 640, 480, 0, 1);
    fn_8003AB38(lbl_1_bss_0, 32730, 0, 0, -1, fn_8003D4FC);

    lightId = Hu3DGLightCreate(0, 1000, 1000, 0, -1, -1, 255, 255, 255);
    fn_8003AB38(lbl_1_bss_0, 10, 3, 0, -1, fn_1_2BC);
    fn_80070788(&lbl_1_data_0);
}

/* Actor-manager callback that loads the ground collision map and creates and positions four
 * players. */
static void fn_1_2BC(OMOBJ *obj)
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
    lbl_1_bss_4[0] = fn_80080728(0, &param, 1 << 0, 1 << 0, ~(1 << 3), NULL);
    param.param++;
    lbl_1_bss_4[1] = fn_80080728(1, &param, 1 << 0, 1 << 0, ~(1 << 3), NULL);
    param.param++;
    lbl_1_bss_4[2] = fn_80080728(2, &param, 1 << 0, 1 << 0, ~(1 << 3), NULL);
    param.param++;
    lbl_1_bss_4[3] = fn_80080728(3, &param, 1 << 0, 1 << 0, ~(1 << 3), NULL);
    param.param++;
    lbl_1_bss_4[0]->actor->pos.x = -300.211f;
    lbl_1_bss_4[0]->actor->pos.y = 300;
    lbl_1_bss_4[0]->actor->pos.z = -85.369f;
    lbl_1_bss_4[1]->actor->pos.x = 200;
    lbl_1_bss_4[1]->actor->pos.y = 300;
    lbl_1_bss_4[1]->actor->pos.z = 0;
    lbl_1_bss_4[2]->actor->pos.x = 4300;
    lbl_1_bss_4[2]->actor->pos.y = 1000;
    lbl_1_bss_4[2]->actor->pos.z = 0;
    lbl_1_bss_4[3]->actor->pos.x = 400;
    lbl_1_bss_4[3]->actor->pos.y = 300;
    lbl_1_bss_4[3]->actor->pos.z = 110;
    obj->objFunc = NULL;
}

/* MG sequence init hook: request the next mode and run one actor update. */
static void fn_1_5BC(s16 mode, s16 frameNo)
{
    fn_80071A9C();
    fn_800856A8();
}

/* MG sequence fade-in hook: request the next mode after frame 90. */
static void fn_1_5E0(s16 mode, s16 frameNo)
{
    if(frameNo > 90) {
        fn_80071A9C();
    }
}

/* MG sequence start-message hook: advance as soon as the start message is active. */
static void fn_1_610(s16 mode, s16 frameNo)
{
    fn_80071A9C();
}

/* MG sequence main hook, called each gameplay frame to update actors, camera, and the exit
 * input. */
static void fn_1_630(s16 mode, s16 frameNo)
{
    fn_800856A8();
    lbl_802C028C += lbl_802BFFEC[0]/2;
    lbl_802C028C -= lbl_802BFFE8[0]/2;
    lbl_80266A4C.x = lbl_1_bss_4[0]->actor->pos.x;
    lbl_80266A4C.y = lbl_1_bss_4[0]->actor->pos.y+100;
    lbl_80266A4C.z = lbl_1_bss_4[0]->actor->pos.z;
    if(lbl_802C0008[0] & PAD_BUTTON_X) {
        fn_80071A9C();
    }
}

/* MG sequence finish-message hook: request the following sequence mode immediately. */
static void fn_1_78C(s16 mode, s16 frameNo)
{
    fn_80071A9C();
}

/* MG sequence pre-winner hook: request the winner mode immediately. */
static void fn_1_7AC(s16 mode, s16 frameNo)
{
    fn_80071A9C();
}

/* MG sequence winner hook: on frame one, create player zero's win motion, pass its model ID as
 * fn_8005C2C8's character number, and place its model at lbl_80266A4C; on other frames, set its
 * model Y position to lbl_80266A4C.y plus twice the frame number. */
static void fn_1_7CC(s16 mode, s16 frameNo)
{
    if(frameNo == 1) {
        int motId = fn_8005B0B8(lbl_1_bss_4[0]->charNo, CHARMOT_HSF_c000m1_306);
        fn_80026760(lbl_1_bss_4[0]->actor->mdlId, lbl_80266A4C.x, lbl_80266A4C.y, lbl_80266A4C.z);
        fn_8005C2C8(lbl_1_bss_4[0]->actor->mdlId, motId, 0, 5, (1 << 30) | (1 << 0));
    } else {
        fn_80026760(lbl_1_bss_4[0]->actor->mdlId, lbl_80266A4C.x, lbl_80266A4C.y+(frameNo*2),
            lbl_80266A4C.z);
    }
}

/* MG sequence fade-out hook: the request at frame 60 is not reached with the default 60-frame
 * phase, whose hook runs on frames 0 through 59. */
static void fn_1_91C(s16 mode, s16 frameNo)
{
    if(frameNo >= 60) {
        fn_80071A9C();
    }
}

/* Close hook: on the CLOSE and EXIT paths, sets the next-mode request, which is not consumed
 * before either path returns to the overlay. */
static void fn_1_94C(s16 mode, s16 frameNo)
{
    fn_80071A9C();
}
