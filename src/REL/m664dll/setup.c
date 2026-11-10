/* Wrasslin' Rapids overlay startup, scene setup and sequence callbacks. */
#include "math.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/mg/seqman.h"
#include "game/charman.h"
#include "game/mg/actman.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "REL/m664dll/m664dll.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
HUPROCESS *lbl_1_bss_4; /* Object manager created for the minigame's actors. */
int lbl_1_bss_0; /* Cleared at scene setup and at the end of active play. */
void fn_1_D8(s16 mode, s16 frameNo);
void fn_1_27C(s16 mode, s16 frameNo);
void fn_1_29C(s16 mode, s16 frameNo);
void fn_1_2BC(s16 mode, s16 frameNo);
/* No sequence timer is requested; the main callback decides when play ends. */
MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_D8, fn_1_27C, fn_1_29C, fn_1_2BC, fn_1_2F8, fn_1_318, fn_1_34C, fn_1_36C, fn_1_38C
};

void ObjectSetup(void);

/* The overlay loader calls this to run constructors and start the minigame sequence. */
int _prolog(void)
{
    const VoidFunc *constructor = _ctors;

    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    ObjectSetup();
    return 0;
}

/* The overlay loader calls this to run destructors when Wrasslin' Rapids is unloaded. */
void _epilog(void)
{
    const VoidFunc *destructor = _dtors;

    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}

/* Called at overlay startup to create the actor manager and register the phase callbacks. */
void ObjectSetup(void)
{
    lbl_1_bss_4 = fn_80080418();
    fn_80070788(&lbl_1_data_0);
}

/* The sequence's init callback prepares the camera, shadows and course before fade-in. */
void fn_1_D8(s16 mode, s16 frameNo)
{
    Point3d shadowCameraPosition = { 0.0f, 5000.0f, 1.0f };
    Point3d shadowCameraUp = { 0.0f, 1.0f, 0.0f };
    Point3d shadowCameraTarget = { 0.0f, 0.0f, 0.0f };

    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f,
                          480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(1, 50.0f, 20.0f,
                             30000.0f, 1.2f);
    /* Apply the shared camera view during each object-manager update. */
    fn_8003AB38(lbl_1_bss_4, 32730, 0, 0, -1, fn_8003D4FC);
    Hu3DShadowCreate(30.0f, 20.0f, 10000.0f);
    fn_8002AAB4(0, 0, 0);
    Hu3DShadowTPLvlSet(0.4f);
    Hu3DShadowPosSet(&shadowCameraPosition, &shadowCameraUp, &shadowCameraTarget);
    fn_1_3C0();
    lbl_1_bss_0 = 0;
    fn_80071A9C();
}

/* The fade-in callback advances the opening animation and guides players onto the raft. */
void fn_1_27C(s16 mode, s16 frameNo)
{
    fn_1_560();
}

/* While the Start message runs, settle the player lineup and start the gameplay music. */
void fn_1_29C(s16 mode, s16 frameNo)
{
    fn_1_8DC();
}

/* The main phase updates play each frame and advances when fewer than two players remain
 * or the course sequence reaches its end. */
void fn_1_2BC(s16 mode, s16 frameNo)
{
    if (fn_1_4C0()) {
        lbl_1_bss_0 = 0;
        fn_80071A9C();
    }
}
