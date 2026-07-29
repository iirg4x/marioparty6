#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;

typedef struct omObj_s OMOBJ;
typedef void (*OMOBJ_FUNC)(OMOBJ *object);

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
    HuVecF trans;
    HuVecF rot;
    HuVecF scale;
    u16 mdlcnt;
    HU3D_MODELID *mdlId;
    u16 mtncnt;
    HU3D_MOTIONID *mtnId;
    u32 work[4];
    void *data;
};

extern s16 lbl_1_bss_1A48[5];

void fn_1_12838(void);
void HuWinExKill(s16 window);
void HuWinAllKill(void);
void Hu3DLightAllKill(void);
void Hu3DCameraKill(int cameraBit);
u8 WipeCheck(void);
void HuAudFadeOut(s32 speed);
void HuAudAllStop(void);
void omOvlReturnEx(s16 historyOffset, s16 unlink);

void fn_1_4AB4(void)
{
    s16 window;

    fn_1_12838();
    for (window = 0; window < 4; window++) {
        HuWinExKill(lbl_1_bss_1A48[window]);
    }
    HuWinAllKill();
    Hu3DLightAllKill();
    Hu3DCameraKill(1);
}

void fn_1_4B20(OMOBJ *object)
{
    s16 window;

    if (WipeCheck() == 0) {
        HuAudFadeOut(1000);
        fn_1_12838();
        for (window = 0; window < 4; window++) {
            HuWinExKill(lbl_1_bss_1A48[window]);
        }
        HuWinAllKill();
        Hu3DLightAllKill();
        Hu3DCameraKill(1);
        HuAudAllStop();
        omOvlReturnEx(1, 1);
        object->objFunc = NULL;
    }
}
