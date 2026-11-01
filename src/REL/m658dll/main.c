/* Creates the minigame process and advances its start and finish sequence. */
#include "math.h"
#include "game/object.h"
#include "game/memory.h"
#include "game/mg/actman.h"
#include "game/charman.h"
#include "game/mg/seqman.h"
#include "game/audio.h"
#include "game/wipe.h"
#include "msm_stream.h"
#include <string.h>

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
OMOBJMAN *lbl_1_bss_0;
void fn_1_148(void);
void fn_1_264(void);
void fn_1_768(void);
s32 fn_1_C38(void);
s32 fn_1_C84(void);
s32 fn_1_CD0(void);
s32 fn_1_D40(void);
void fn_1_288(s16 mode, s16 frameNo);
void fn_1_2BC(s16 mode, s16 frameNo);
void fn_1_31C(s16 mode, s16 frameNo);
void fn_1_320(s16 mode, s16 frameNo);
void fn_1_34C(s16 mode, s16 frameNo);
void fn_1_3C4(s16 mode, s16 frameNo);
void fn_1_3F8(s16 mode, s16 frameNo);
void fn_1_3FC(s16 mode, s16 frameNo);
void fn_1_400(s16 mode, s16 frameNo);

MGSEQ_PARAM lbl_1_data_0 = {
    300, 1,
    fn_1_288, fn_1_2BC, fn_1_31C, fn_1_320, fn_1_34C, fn_1_3C4, fn_1_3F8, fn_1_3FC, fn_1_400
};
s32 lbl_1_data_28 = -1;

/* Runs when the minigame overlay loads, then starts its actor and sequence setup. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_148();
    return 0;
}

/* Runs when the minigame overlay unloads and calls its registered destructors. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Creates an actor object and returns its zeroed work block for minigame state. */
/* Minigame setup calls this to create an actor object and allocate its zeroed work block. */
void *fn_1_A0(s32 priority, u32 bytes, void (*update)(OMOBJ *))
{
    OMOBJ *object;

    object = fn_8003AB38(lbl_1_bss_0, priority, 0, 0, 0, update);
    if (bytes != 0) {
        object->data = HuMemDirectMallocNum(HEAP_HEAP, bytes, 0x10000000);
        memset(object->data, 0, bytes);
    } else {
        object->data = NULL;
    }
    return object->data;
}

void fn_1_140(OMOBJ *object, OMOBJ_FUNC callback)
{
    object->objFunc = callback;
}

/* Starts the minigame actor manager, shared shadows and sequence process at load. */
void fn_1_148(void)
{
    lbl_1_bss_0 = fn_80080418();
    fn_80080E8C();
    {
    HuVecF cameraPosition = {-100.0f, 8000.0f, 0.0f};
    HuVecF cameraUp = {0.0f, 1.0f, 0.0f};
    HuVecF cameraTarget = {0.0f, 0.0f, 0.0f};

    fn_8002A3B0(15.0f, 10.0f, 10000.0f, 3);
    fn_8002AA3C(16, 16, 16, 3);
    fn_8002A674(&cameraPosition, &cameraUp, &cameraTarget, 3);
    fn_8002A7D8(0.3f, 3);
    fn_8000DAC0(fn_1_264, 100, 12288, 0, lbl_1_bss_0);
    }
}

/* Runs as the child sequence process, advancing actors and yielding each frame. */
void fn_1_264(void)
{
    fn_80070788(&lbl_1_data_0);
    for (;;) {
        fn_800856A8();
        HuPrcVSleep();
    }
}

/* Starts the minigame sequence by clearing its stream handle and advancing phase. */
void fn_1_288(s16 mode, s16 frameNo)
{
    lbl_1_data_28 = -1;
    fn_1_768();
    fn_80071A9C();
}

/* Starts the sequence music once it is ready and no wipe is running. */
void fn_1_2BC(s16 mode, s16 frameNo) {
    if ((fn_1_C38() != 0) && ((s32) lbl_1_data_28 == -1) && (WipeCheck() == 0)) {
        lbl_1_data_28 = HuAudSStreamPlay(MSM_STREAM_STORY_BOWSER);
    }
}

void fn_1_31C(s16 mode, s16 frameNo)
{
}

void fn_1_320(s16 mode, s16 frameNo)
{
    if (fn_1_C84() != 0) fn_80071A9C();
}

/* Holds this sequence phase until the result transition is ready, then advances. */
void fn_1_34C(s16 mode, s16 frameNo) {
    if ((fn_80071DE4() == 0) && ((s32) lbl_1_data_28 != -1)) {
        HuAudSStreamFadeOut(lbl_1_data_28, 100);
        lbl_1_data_28 = -1;
    }
    fn_80071B08();
    if (fn_1_CD0() != 0) {
        fn_80071B30();
        fn_80071A9C();
    }
}
/* Holds this sequence phase until the minigame's result transition is ready. */
void fn_1_3C4(s16 mode, s16 frameNo) {
    fn_80071B08();
    if (fn_1_D40() != 0) {
        fn_80071B30();
        fn_80071A9C();
    }
}

void fn_1_3F8(s16 mode, s16 frameNo)
{
}

void fn_1_3FC(s16 mode, s16 frameNo)
{
}

void fn_1_400(s16 mode, s16 frameNo)
{
}
