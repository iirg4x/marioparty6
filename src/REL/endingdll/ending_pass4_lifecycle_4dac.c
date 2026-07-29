#include <dolphin/types.h>

enum {
    WIPE_TYPE_NORMAL,
};

enum {
    WIPE_MODE_OUT = 2,
};

extern s16 lbl_1_bss_1A48[5];
extern s32 lbl_1_bss_1DDC;

void fn_1_DD14(void);
void fn_1_12838(void);
void HuAudSStreamFadeOut(int stream, s32 speed);
void WipeCreate(s16 mode, s16 type, s16 time);
u8 WipeCheck(void);
void HuPrcVSleep(void);
void HuPrcEnd(void);
void HuWinExKill(s16 window);
void HuWinAllKill(void);
void Hu3DLightAllKill(void);
void Hu3DCameraKill(int cameraBit);
void omOvlReturnEx(s16 historyOffset, s16 unlink);

void fn_1_4DAC(void)
{
    s16 window;

    fn_1_DD14();
    HuAudSStreamFadeOut(lbl_1_bss_1DDC, 1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    fn_1_12838();
    for (window = 0; window < 4; window++) {
        HuWinExKill(lbl_1_bss_1A48[window]);
    }
    HuWinAllKill();
    Hu3DLightAllKill();
    Hu3DCameraKill(1);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}
