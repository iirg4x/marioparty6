#include <dolphin/mtx/GeoTypes.h>

#define WIPE_TYPE_NORMAL 0
#define WIPE_MODE_OUT 2

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_LIGHTID;

typedef struct Process OMOBJMAN;
typedef struct omObj_s OMOBJ;
typedef void (*OMOBJ_FUNC)(OMOBJ *obj);

struct omObj_s {
    u16 stat;
    s16 objNext;
    s16 prio;
    s16 prev;
    s16 next;
    s16 nextNo;
    s16 grpNo;
    u16 memberNo;
    u32 mode;
    OMOBJ_FUNC objFunc;
    Vec trans;
    Vec rot;
    Vec scale;
    u16 mdlcnt;
    HU3D_MODELID *mdlId;
    u16 mtncnt;
    HU3D_MOTIONID *mtnId;
    u32 work[4];
    void *data;
};

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern s32 lbl_1_bss_824;
extern HU3D_LIGHTID lbl_1_bss_82C[2];

void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelHookReset(HU3D_MODELID modelId);
BOOL Hu3DMotionKill(HU3D_MOTIONID motId);
void Hu3DGLightKill(HU3D_LIGHTID lightId);
void Hu3DCameraKill(u32 cameraBit);
void omDelObjEx(OMOBJMAN *objMan, OMOBJ *obj);
void omOvlReturnEx(s16 hisOfs, s16 unlinkF);
void HuPrcVSleep(void);
void HuPrcEnd(void);
void WipeCreate(s16 mode, s16 type, s16 maxTime);
u8 WipeCheck(void);
void HuAudSStreamFadeOut(int streamNo, s32 speed);
void fn_1_6840(void);
void fn_1_9D14(void);

void fn_1_528C(void)
{
    OMOBJ *first;
    OMOBJ *second;
    OMOBJ *third;
    OMOBJ *fourth;
    s16 firstIndex;
    s16 secondIndex;
    s16 thirdIndex;
    s16 fourthIndex;
    s16 lightIndex;

    fn_1_6840();
    HuAudSStreamFadeOut(lbl_1_bss_824, 1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    first = lbl_1_bss_4;
    if (first) {
        for (firstIndex = 0; firstIndex < 2; firstIndex++) {
            Hu3DMotionKill(first->mtnId[firstIndex]);
            Hu3DModelKill(first->mdlId[firstIndex]);
        }
        omDelObjEx(lbl_1_bss_0, first);
    }
    first = NULL;
    second = lbl_1_bss_8;
    if (second) {
        for (secondIndex = 0; secondIndex < 5; secondIndex++) {
            Hu3DMotionKill(second->mtnId[secondIndex]);
        }
        Hu3DModelKill(second->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, second);
    }
    second = NULL;
    third = lbl_1_bss_C;
    if (third) {
        Hu3DModelHookReset(third->mdlId[0]);
        for (thirdIndex = 0; thirdIndex < 5; thirdIndex++) {
            Hu3DMotionKill(third->mtnId[thirdIndex]);
        }
        Hu3DModelKill(third->mdlId[0]);
        Hu3DModelKill(third->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, third);
    }
    third = NULL;
    fourth = lbl_1_bss_10;
    if (fourth) {
        for (fourthIndex = 0; fourthIndex < 13; fourthIndex++) {
            Hu3DMotionKill(fourth->mtnId[fourthIndex]);
            Hu3DModelKill(fourth->mdlId[fourthIndex]);
        }
        omDelObjEx(lbl_1_bss_0, fourth);
    }
    fourth = NULL;
    fn_1_9D14();
    for (lightIndex = 0; lightIndex < 2; lightIndex++) {
        Hu3DGLightKill(lbl_1_bss_82C[lightIndex]);
    }
    Hu3DCameraKill(1);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}
