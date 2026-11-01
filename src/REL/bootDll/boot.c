// Starts the system warning, Nintendo startup screen, and title menu.
#include "dolphin.h"
#include "dolphin/os/OSReset.h"
#include "game/audio.h"
#include "game/data.h"
#include "datanum/title.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/init.h"
#include "game/main.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"
#include "math.h"

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern u8 nintendoData[];
extern u32 warningData[];
extern u32 warningPressStartData[];
extern s16 BootUnusedBss;
extern s16 TitleMdlId[4];
extern s16 BootLightId[3];
extern float BootCameraZoom[2];
extern HuVecF BootCameraCenter[2];
extern HuVecF BootCameraRot[2];
extern OMOBJMAN *BootObjMan;
extern OMOBJ *OutViewObj;

enum {
    WARNING_SPRITE_MEMBER = 0,
    PRESS_START_SPRITE_MEMBER = 1,
    WARNING_SPRITE_MEMBER_COUNT = 2,

    WARNING_SPRITE_PRIORITY = 0,
    WARNING_SPRITE_BANK = 0,
    TITLE_WIPE_FRAMES = 30,
    TITLE_STREAM_DELAY_FRAMES = 10,
    TITLE_INPUT_WAIT_FRAMES = 2100,
    TITLE_CONFIRM_WAIT_FRAMES = 20,
    TITLE_CONFIRM_SE = 1180,
    INITIAL_WAIT_FRAMES = 90,
    MAX_INPUT_WAIT_FRAMES = 3510,
    BLINK_PERIOD_FRAMES = 30,
    BOOT_PAD_SKIP_MASK = PAD_BUTTON_A | PAD_BUTTON_START,
    CAMERA_OUT_VIEW_PRIORITY = 32730,
    SUBSTICK_INPUT_MASK = 0xF8,
};

void BootExec(void);
void BootProgExec(void);
void ObjectSetup(void);
void CameraOutView(OMOBJ *object);
void CameraMove(OMOBJ *object);
void BootTitleCreate(void);
s32 BootTitleExec(s32 firstRun);
void BootWarningExec(void);
void *NintendoDataDecode(void);
void fn_80056428(void);
void GameMesInit(void);
void OpeningCreate(void);
void OpeningExec(s32 enablePrompt);
void OpeningKill(void);

// The overlay loader calls this entry point before BootExec starts.
int _prolog(void)
{
    const VoidFunc *ctor;

    for (ctor = _ctors; *ctor != 0; ctor++) {
        (*ctor)();
    }

    ObjectSetup();
    return 0;
}

// The overlay loader calls this entry point while unloading the boot overlay.
void _epilog(void)
{
    const VoidFunc *dtor;

    for (dtor = _dtors; *dtor != 0; dtor++) {
        (*dtor)();
    }
}

// Sets up the boot scene, camera callbacks, lighting, and player defaults before BootExec.
void ObjectSetup(void)
{
    s32 i;
    OMOVLHIS *history;

    OSReport("******* Boot ObjectSetup *********\n");
    BootObjMan = fn_8003A8BC(50, 8192);
    BootCameraRot[0].x = -5.0f;
    BootCameraRot[0].y = 0.0f;
    BootCameraRot[0].z = 0.0f;
    BootCameraCenter[0].x = 0.0f;
    BootCameraCenter[0].y = 105.0f;
    BootCameraCenter[0].z = -54.0f;
    BootCameraZoom[0] = 636.0f;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.0f, 15000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    fn_8000D818(BootExec, 100, 12288, 0);
    OutViewObj = fn_8003AB38(BootObjMan, CAMERA_OUT_VIEW_PRIORITY, 0, 0, -1, CameraOutView);
    fn_8003AB38(BootObjMan, 0, 32, 32, -1, CameraMove);
    BootLightId[0] = Hu3DGLightCreate(0.0f, 10.0f, 1000.0f,
                                      0.0f, -0.01f, -1.0f,
                                      64, 64, 96);
    Hu3DGLightInfinitytSet(BootLightId[0]);
    BootLightId[1] = Hu3DGLightCreate(0.0f, 2000.0f, 100.0f,
                                      0.0f, -1.0f, -0.05f,
                                      160, 160, 160);
    Hu3DGLightInfinitytSet(BootLightId[1]);
    Hu3DBGColorSet(0, 0, 0);
    history = omOvlHisGet(0);
    omOvlHisChg(0, history->ovl, 1, history->stat);
    for (i = 0; i < 4; i++) {
        lbl_80265728[i].charNo = lbl_80265728[i].padNo = i;
    }
}

// Runs from the boot process created by ObjectSetup; sequences startup screens and title input.
void BootExec(void)
{
    HUPROCESS *process;
    HUSPR_GROUPID group;
    HUSPRID nintendoSprite;
    HUSPRID titleSprite;
    HUSPRID waitSprite;
    ANIMDATA *anim;
    s32 firstRun;
    s16 asyncStat;
    s16 frame;
    OSTick start;
    void *buf;

    // The current-process pointer is fetched here but is not used by the startup sequence.
    process = fn_8000DD9C();
    firstRun = FALSE;
    asyncStat = -1;

    if (lbl_802C0248 == 0 && VIGetTvFormat() != VI_MPAL) {
        BootProgExec();
    }
    if (lbl_802C0248 == 0) {
        BootWarningExec();
    }

    group = HuSprGrpCreate(3);
    anim = HuSprAnimRead(NintendoDataDecode());
    nintendoSprite = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(group, 0, nintendoSprite);
    HuSprPosSet(group, 0, 288.0f, 240.0f);
    HuSprAttrSet(group, 0, HUSPR_ATTR_DISPOFF);

    if (lbl_802C0248 != 0) {
        HuAudSndGrpSetSet(MSM_GRP_MENU);

        anim = HuSprAnimRead(fn_80008DD0(NINTENDO_SPRITE_DATA, 0x10000000, 2));
        titleSprite = HuSprCreate(anim, 0, 0);
        HuSprGrpMemberSet(group, 1, titleSprite);
        HuSprPosSet(group, 1, 288.0f, 240.0f);
        HuSprAttrSet(group, 1, HUSPR_ATTR_DISPOFF);

        waitSprite = HuSprCreate(HuSprAnimRead(fn_80008DD0(OPENING_WAIT_SPRITE_DATA,
                                                                    0x10000000, 2)),
                                 0, 0);
        HuSprGrpMemberSet(group, 2, waitSprite);
        HuSprPosSet(group, 2, 288.0f, 206.0f);
        HuSprAttrSet(group, 2, HUSPR_ATTR_DISPOFF | HUSPR_ATTR_LINEAR);

        HuWinInit(1);
        BootTitleCreate();
        OpeningCreate();
        NintendoDispF = TRUE;
        firstRun = TRUE;
        goto title_exec;
    } else {
        for (frame = 0; frame < 4; frame++) {
            lbl_80265728[frame].padNo = frame;
        }

        HuSprAttrReset(group, 0, HUSPR_ATTR_DISPOFF);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
        while (WipeCheck()) {
            HuPrcVSleep();
        }
        WipeColorSet(255, 255, 255);

        if (NintendoDispF == FALSE) {
            start = OSGetTick();
            fn_80056428();
            HuWindowInit();
            GameMesInit();
            HuWinInit(1);
            asyncStat = fn_80007F20(0xd50000);
            while (OSTicksToMilliseconds(OSGetTick() - start) < 1500) {
                HuPrcVSleep();
            }
            OSReport("%dms\n", OSTicksToMilliseconds(OSGetTick() - start));
        } else {
            for (frame = 0; frame < 90; frame++) {
                if (lbl_802C0008[0] & BOOT_PAD_SKIP_MASK) {
                    break;
                }
                HuPrcVSleep();
            }
        }

        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
        while (WipeCheck()) {
            HuPrcVSleep();
        }

        if (NintendoDispF == FALSE) {
            start = OSGetTick();
            while (!fn_80008270(asyncStat)) {
                HuPrcVSleep();
            }
            BootTitleCreate();

            anim = HuSprAnimRead(
                fn_80008DD0(NINTENDO_SPRITE_DATA, 0x10000000, 2));
            titleSprite = HuSprCreate(anim, 0, 0);
            HuSprGrpMemberSet(group, 1, titleSprite);
            HuSprPosSet(group, 1, 288.0f, 240.0f);
            HuSprAttrSet(group, 1, HUSPR_ATTR_DISPOFF);

            waitSprite = HuSprCreate(HuSprAnimRead(fn_80008DD0(OPENING_WAIT_SPRITE_DATA,
                                                                        0x10000000, 2)),
                                     0, 0);
            HuSprGrpMemberSet(group, 2, waitSprite);
            HuSprPosSet(group, 2, 288.0f, 206.0f);
            HuSprAttrSet(group, 2, HUSPR_ATTR_DISPOFF);

            while (OSTicksToMilliseconds(OSGetTick() - start) < 1000) {
                HuPrcVSleep();
            }
            OSReport("%dms\n", OSTicksToMilliseconds(OSGetTick() - start));
        } else {
            HuPrcSleep(60);
        }

        HuSprAttrSet(group, 0, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(group, 1, HUSPR_ATTR_DISPOFF);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
        while (WipeCheck()) {
            HuPrcVSleep();
        }

        if (NintendoDispF == FALSE) {
            start = OSGetTick();
            OpeningCreate();
            while (OSTicksToMilliseconds(OSGetTick() - start) < 1500) {
                HuPrcVSleep();
            }
            OSReport("%dms\n", OSTicksToMilliseconds(OSGetTick() - start));
        } else {
            for (frame = 0; frame < 90; frame++) {
                if (lbl_802C0008[0] & BOOT_PAD_SKIP_MASK) {
                    break;
                }
                HuPrcVSleep();
            }
        }

        WipeColorSet(255, 255, 255);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
        while (WipeCheck()) {
            HuPrcVSleep();
        }
        HuPrcSleep(60);

        HuSprAttrSet(group, 0, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group, 1, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(group, 2, HUSPR_ATTR_DISPOFF);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
        while (WipeCheck()) {
            HuPrcVSleep();
        }

        if (NintendoDispF == FALSE) {
            HuPrcSleep(60);
        } else {
            for (frame = 0; frame < 60; frame++) {
                if (lbl_802C0008[0] & BOOT_PAD_SKIP_MASK) {
                    break;
                }
                HuPrcVSleep();
            }
        }

        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
        while (WipeCheck()) {
            HuPrcVSleep();
        }
        HuSprAttrSet(group, 2, HUSPR_ATTR_DISPOFF);

        if (NintendoDispF == FALSE) {
            fn_800097F4(0xd50000);
            buf = HuMemDirectMallocNum(3, msmSysGetSampSize(FALSE), 0x10000000);
            msmSysLoadGroup(0, buf, FALSE);
            HuMemDirectFree(buf);
            msmSysSetGroupLoadMode(TRUE);
            HuAudSndGrpSetSet(MSM_GRP_MENU);
        } else {
            HuPrcSleep(90);
        }
    }

opening_exec:
    OpeningExec(1);
    NintendoDispF = TRUE;
    HuSprAttrSet(group, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(group, 1, HUSPR_ATTR_DISPOFF);
    HuSprAttrSet(group, 2, HUSPR_ATTR_DISPOFF);

title_exec:
    if (BootTitleExec(firstRun) == 0) {
        firstRun = FALSE;
        HuPrcSleep(30);
        goto opening_exec;
    }

    HuPrcSleep(60);
    OpeningKill();
    fn_800097F4(0xd50000);
    HuAudSStreamAllFadeOut(1000);
    asyncStat = fn_80007F20(0x230000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    HuPrcSleep(30);
    while (!fn_80008270(asyncStat)) {
        HuPrcVSleep();
    }

    if (omSysExitReq != 0) {
        fn_8003A5E4(1, TRUE);
    } else {
        omOvlCallEx(3, TRUE, 0, 0);
    }

    for (frame = 0; frame < 4; frame++) {
        lbl_80265728[frame].charNo = -1;
    }

    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

// BootExec offers progressive scan on eligible startup; when progressive mode is off, B must
// be held. The 600-frame timeout accepts the current selection, which initially is progressive.
void BootProgExec(void)
{
    static u16 posTbl[] = { 221, 311, 357, 311 };
    HUSPR_GROUPID group;
    HUSPRID sprite;
    s16 frame;
    ANIMDATA *anim;
    s16 selection;

    selection = 0;
    if (OSGetResetCode() != 0) {
        return;
    }

    if (VIGetDTVStatus() != 1) {
        RenderMode = &GXNtsc480IntDf;
        VIConfigure(&GXNtsc480IntDf);
        VIFlush();
        return;
    }

    if (!OSGetProgressiveMode()) {
        if (!((lbl_802C0010[0] | lbl_802C0010[1] | lbl_802C0010[2]
            | lbl_802C0010[3]) & PAD_BUTTON_B)) {
            return;
        }
    }

    if (OSGetProgressiveMode() == 1) {
        RenderMode = &GXNtsc480IntDf;
        VIConfigure(&GXNtsc480IntDf);
        VIFlush();
    }

    group = HuSprGrpCreate(3);

    anim =
        HuSprAnimRead(fn_80008DD0(PROGRESSIVE_MSG_SPRITE_DATA, 0x10000000, 2));
    sprite = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(group, 0, sprite);
    HuSprPosSet(group, 0, 288.0f, 240.0f);

    anim = HuSprAnimRead(
        fn_80008DD0(PROGRESSIVE_CURSOR_SPRITE_DATA, 0x10000000, 2));
    sprite = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(group, 1, sprite);
    HuSprPosSet(group, 1, posTbl[selection * 2], posTbl[selection * 2 + 1]);

    anim = HuSprAnimRead(
        fn_80008DD0(PROGRESSIVE_CURSOR_SEL_SPRITE_DATA, 0x10000000, 2));
    sprite = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(group, 2, sprite);
    HuSprPosSet(group, 2, posTbl[selection * 2], posTbl[selection * 2 + 1]);
    HuSprAttrSet(group, 2, HUSPR_ATTR_DISPOFF);

    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
    while (WipeCheck()) {
        HuPrcVSleep();
    }

    for (frame = 0; frame < 600; frame++) {
        if ((lbl_802BFFE0[0] | lbl_802BFFE0[1] | lbl_802BFFE0[2] | lbl_802BFFE0[3]) & 3) {
            selection ^= 1;
            HuSprPosSet(group, 1, posTbl[selection * 2], posTbl[selection * 2 + 1]);
            frame = 0;
        }

        if ((lbl_802C0008[0] | lbl_802C0008[1] | lbl_802C0008[2] | lbl_802C0008[3]) &
            PAD_BUTTON_A) {
            HuSprAttrSet(group, 1, HUSPR_ATTR_DISPOFF);
            HuSprPosSet(group, 2, posTbl[selection * 2], posTbl[selection * 2 + 1]);
            HuSprAttrReset(group, 2, HUSPR_ATTR_DISPOFF);
            break;
        }
        HuPrcVSleep();
    }

    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 20);
    while (WipeCheck()) {
        HuPrcVSleep();
    }

    if (selection == 0) {
        OSSetProgressiveMode(1);
        RenderMode = &GXNtsc480Prog;
        VIConfigure(&GXNtsc480Prog);
        VIFlush();
        HuPrcSleep(100);
    } else {
        OSSetProgressiveMode(0);
        GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, GX_TRUE, RenderMode->vfilter);
    }

    HuSprGrpKill(group);

    group = HuSprGrpCreate(1);
    if (selection == 0) {
        anim = HuSprAnimRead(
            fn_80008DD0(PROGRESSIVE_ON_SPRITE_DATA, 0x10000000, 2));
    } else {
        anim = HuSprAnimRead(
            fn_80008DD0(PROGRESSIVE_OFF_SPRITE_DATA, 0x10000000, 2));
    }
    sprite = HuSprCreate(anim, 0, 0);
    HuSprGrpMemberSet(group, 0, sprite);
    HuSprPosSet(group, 0, 288.0f, 240.0f);

    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 20);
    while (WipeCheck()) {
        HuPrcVSleep();
    }

    HuPrcSleep(60);
    for (frame = 0; frame < 180; frame++) {
        if ((lbl_802C0008[0] | lbl_802C0008[1] | lbl_802C0008[2] | lbl_802C0008[3]) &
            PAD_BUTTON_A) {
            break;
        }
        HuPrcVSleep();
    }

    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    HuSprGrpKill(group);
    HuPrcSleep(30);
}

// The object manager positions the title view from the camera's x/y rotation, target, and zoom;
// it reads the z rotation but does not apply it.
void CameraOutView(OMOBJ *object)
{
    static u16 cameraMask[] = { 1, 2 };
    s16 i;
    float rotX;
    float rotY;
    float rotZ;
    HuVecF pos;
    HuVecF target;
    HuVecF up;

    for (i = 0; i < 1; i++) {
        rotX = BootCameraRot[i].x;
        rotY = BootCameraRot[i].y;
        rotZ = BootCameraRot[i].z;

        pos.x = BootCameraCenter[i].x + (BootCameraZoom[i] *
            (sin(3.141592653589793 * rotY / 180.0) *
             cos(3.141592653589793 * rotX / 180.0)));
        pos.y = BootCameraCenter[i].y + (BootCameraZoom[i] *
            -sin(3.141592653589793 * rotX / 180.0));
        pos.z = BootCameraCenter[i].z + (BootCameraZoom[i] *
            (cos(3.141592653589793 * rotY / 180.0) *
             cos(3.141592653589793 * rotX / 180.0)));

        target.x = BootCameraCenter[i].x;
        target.y = BootCameraCenter[i].y;
        target.z = BootCameraCenter[i].z;

        up.x = sin(3.141592653589793 * rotY / 180.0) *
               sin(3.141592653589793 * rotX / 180.0);
        up.y = cos(3.141592653589793 * rotX / 180.0);
        up.z = cos(3.141592653589793 * rotY / 180.0) *
               sin(3.141592653589793 * rotX / 180.0);

        Hu3DCameraPosSet(cameraMask[i],
                         pos.x, pos.y, pos.z,
                         up.x, up.y, up.z,
                         target.x, target.y, target.z);
    }
}

// The object manager calls this callback, but its private move flag is never set here, so it
// returns immediately.
void CameraMove(OMOBJ *object)
{
    static s32 moveF;
    HuVecF eye;
    HuVecF delta;
    HuVecF up;
    HuVecF roll;
    s8 subStick;
    float rotZ;

    if (!moveF) {
        return;
    }

    BootCameraRot[0].y += 0.1f * (float)lbl_802BFFFC[0];
    BootCameraRot[0].x += 0.1f * (float)lbl_802BFFF8[0];
    BootCameraZoom[0] += (float)(lbl_802BFFEC[0] / 2);
    BootCameraZoom[0] -= (float)(lbl_802BFFE8[0] / 2);
    if (BootCameraZoom[0] < 100.0f) {
        BootCameraZoom[0] = 100.0f;
    }

    eye.x = BootCameraCenter[0].x + (BootCameraZoom[0] *
        (sin(3.141592653589793 * BootCameraRot[0].y / 180.0) *
         cos(3.141592653589793 * BootCameraRot[0].x / 180.0)));
    eye.y = BootCameraCenter[0].y + (BootCameraZoom[0] *
        -sin(3.141592653589793 * BootCameraRot[0].x / 180.0));
    eye.z = BootCameraCenter[0].z + (BootCameraZoom[0] *
        (cos(3.141592653589793 * BootCameraRot[0].y / 180.0) *
         cos(3.141592653589793 * BootCameraRot[0].x / 180.0)));

    delta.x = BootCameraCenter[0].x - eye.x;
    delta.y = BootCameraCenter[0].y - eye.y;
    delta.z = BootCameraCenter[0].z - eye.z;

    up.x = sin(3.141592653589793 * BootCameraRot[0].y / 180.0) *
           sin(3.141592653589793 * BootCameraRot[0].x / 180.0);
    up.y = cos(3.141592653589793 * BootCameraRot[0].x / 180.0);
    up.z = cos(3.141592653589793 * BootCameraRot[0].y / 180.0) *
           sin(3.141592653589793 * BootCameraRot[0].x / 180.0);

    rotZ = BootCameraRot[0].z;

    roll.x = (up.x * ((delta.x * delta.x) +
               ((1.0f - (delta.x * delta.x)) *
                cos(3.141592653589793 * rotZ / 180.0)))) +
             (up.y * (((delta.x * delta.y) *
               (1.0 - cos(3.141592653589793 * rotZ / 180.0))) -
               (delta.z * sin(3.141592653589793 * rotZ / 180.0)))) +
             (up.z * (((delta.x * delta.z) *
               (1.0 - cos(3.141592653589793 * rotZ / 180.0))) +
               (delta.y * sin(3.141592653589793 * rotZ / 180.0))));

    roll.y = (up.x * (((delta.x * delta.y) *
               (1.0 - cos(3.141592653589793 * rotZ / 180.0))) +
               (delta.z * sin(3.141592653589793 * rotZ / 180.0)))) +
             (up.y * ((delta.y * delta.y) +
               ((1.0f - (delta.y * delta.y)) *
                cos(3.141592653589793 * rotZ / 180.0)))) +
             (up.z * (((delta.y * delta.z) *
               (1.0 - cos(3.141592653589793 * rotZ / 180.0))) -
               (delta.x * sin(3.141592653589793 * rotZ / 180.0))));

    roll.z = (up.x * (((delta.x * delta.z) *
               (1.0 - cos(3.141592653589793 * rotZ / 180.0))) -
               (delta.y * sin(3.141592653589793 * rotZ / 180.0)))) +
             (up.y * (((delta.y * delta.z) *
               (1.0 - cos(3.141592653589793 * rotZ / 180.0))) +
               (delta.x * sin(3.141592653589793 * rotZ / 180.0)))) +
             (up.z * ((delta.z * delta.z) +
               ((1.0f - (delta.z * delta.z)) *
                cos(3.141592653589793 * rotZ / 180.0))));

    PSVECCrossProduct(&up, &delta, &delta);
    PSVECNormalize(&delta, &delta);

    subStick = lbl_802BFFF4[0] & SUBSTICK_INPUT_MASK;
    if (subStick != 0) {
        BootCameraCenter[0].x += (delta.x * (float)subStick) * 0.05f;
        BootCameraCenter[0].y += (delta.y * (float)subStick) * 0.05f;
        BootCameraCenter[0].z += (delta.z * (float)subStick) * 0.05f;
    }

    PSVECNormalize(&roll, &delta);

    subStick = -(lbl_802BFFF0[0] & SUBSTICK_INPUT_MASK);
    if (subStick != 0) {
        BootCameraCenter[0].x += (delta.x * (float)subStick) * 0.05f;
        BootCameraCenter[0].y += (delta.y * (float)subStick) * 0.05f;
        BootCameraCenter[0].z += (delta.z * (float)subStick) * 0.05f;
    }
}

// BootExec calls this after title data is available to create and initially hide the title logo
// models.
void BootTitleCreate(void)
{
    TitleMdlId[0] = fn_80025A04(fn_80008DD0(TITLE_HSF_PARTY, 0x10000000, 2));
    TitleMdlId[1] = fn_80025A04(fn_80008DD0(TITLE_HSF_SIX, 0x10000000, 2));
    TitleMdlId[2] = fn_80025A04(fn_80008DD0(TITLE_HSF_STAR, 0x10000000, 2));

    fn_800269E8(TitleMdlId[0], (1 << 0));
    fn_800269E8(TitleMdlId[1], (1 << 0));
    fn_800269E8(TitleMdlId[2], (1 << 0));
    fn_80026B8C(TitleMdlId[0], 1);
}

// BootExec calls this while the title screen is active; returns 1 on Start or 0 after the
// 2100-frame input timeout.
s32 BootTitleExec(s32 firstRun)
{
    s16 frame;

    Hu3DCameraPerspectiveSet(1, 20.0f, 20.0f, 5000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
                          640.0f, 480.0f, 0.0f, 1.0f);

    BootCameraRot[0].x = 0.0f;
    BootCameraRot[0].y = 0.0f;
    BootCameraRot[0].z = 0.0f;
    BootCameraCenter[0].x = 0.0f;
    BootCameraCenter[0].y = 0.0f;
    BootCameraCenter[0].z = 0.0f;
    BootCameraZoom[0] = 1000.0f;

    fn_80026A44(TitleMdlId[2], (1 << 0));
    fn_8002D414(TitleMdlId[2], 0.0f);
    fn_8002D730(TitleMdlId[2], 0.0f, 71.0f);
    fn_80026A44(TitleMdlId[2], ((1 << 30) | (1 << 0)));
    fn_80026A44(TitleMdlId[1], (1 << 0));
    fn_8002D414(TitleMdlId[1], 0.0f);
    fn_800269E8(TitleMdlId[0], ((1 << 30) | (1 << 0)));
    fn_8002D730(TitleMdlId[0], 0.0f, 59.0f);

    if (!firstRun) {
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, TITLE_WIPE_FRAMES);
        HuPrcSleep(TITLE_STREAM_DELAY_FRAMES);
        HuAudSStreamPlay(MSM_STREAM_TEST);
        WipeWait();
        while (fn_8002D4E0(TitleMdlId[2]) < 70.0f) {
            HuPrcVSleep();
        }
        fn_8002D730(TitleMdlId[2], 70.0f, 500.0f);
    } else {
        fn_8002D414(TitleMdlId[2], 71.0f);
        fn_8002D730(TitleMdlId[2], 70.0f, 500.0f);
        fn_8002D414(TitleMdlId[1], 70.0f);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, TITLE_WIPE_FRAMES);
        HuPrcSleep(TITLE_STREAM_DELAY_FRAMES);
        HuAudSStreamPlay(MSM_STREAM_TEST);
        WipeWait();
    }

    fn_800269E8(TitleMdlId[2], ((1 << 30) | (1 << 0)));
    fn_80026A44(TitleMdlId[0], (1 << 0));
    for (frame = 0; frame < TITLE_INPUT_WAIT_FRAMES; frame++) {
        if (lbl_802C0008[0] & PAD_BUTTON_START) {
            HuAudFXPlay(TITLE_CONFIRM_SE);
            fn_8002D730(TitleMdlId[0], 0.0f, 90.0f);
            fn_80026A44(TitleMdlId[0], ((1 << 30) | (1 << 0)));
            fn_8002D414(TitleMdlId[0], 60.0f);
            HuPrcSleep(TITLE_CONFIRM_WAIT_FRAMES);
            return 1;
        }
        HuPrcVSleep();
    }

    WipeColorSet(255, 255, 255);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, TITLE_WIPE_FRAMES);
    WipeWait();
    fn_800269E8(TitleMdlId[0], (1 << 0));
    fn_800269E8(TitleMdlId[2], (1 << 0));
    return 0;
}

// BootExec decodes the embedded Nintendo startup image before creating its sprite.
void *NintendoDataDecode(void)
{
    u32 *src;
    u32 size;
    void *dst;
    s32 decodeType;

    src = (u32 *)nintendoData;
    size = *src++;
    dst = HuMemDirectMalloc(2, size);
    decodeType = *src++;
    if (dst) {
        fn_8000A384(src, dst, size, decodeType);
    }

    return dst;
}

// Decodes a packed image buffer whose first two words give its size and decode type.
void *WarningDataDecode(u32 *src)
{
    u32 size;
    void *dst;
    s32 decodeType;

    size = *src++;
    dst = HuMemDirectMalloc(2, size);
    decodeType = *src++;
    if (dst) {
        fn_8000A384(src, dst, size, decodeType);
    }

    return dst;
}

// BootExec calls this on first boot to show the safety warning; after 90 frames, any button
// press on any controller dismisses it; otherwise the prompt times out after 3510 frames.
void BootWarningExec(void)
{
    HUSPR_GROUPID group;
    HUSPRID sprite;
    s16 phaseSign;
    ANIMDATA *anim;
    u32 *warningPacked;
    u32 warningSize;
    void *warningRawData;
    void *warningAnimData;
    s32 warningDecodeType;
    u32 *pressStartPacked;
    u32 pressStartSize;
    void *pressStartRawData;
    void *pressStartAnimData;
    s32 pressStartDecodeType;
    s16 frame;
    float scaleX;
    float scaleY;
    float tpLevel;
    float tpLevelStack[1];

    scaleX = 0.9473684430122375f;
    scaleY = 1.0714285373687744f;

    group = HuSprGrpCreate(WARNING_SPRITE_MEMBER_COUNT);

    warningPacked = warningData;
    warningSize = *warningPacked++;
    warningRawData = HuMemDirectMalloc(2, warningSize);
    warningDecodeType = *warningPacked++;
    if (warningRawData) {
        fn_8000A384(warningPacked, warningRawData, warningSize, warningDecodeType);
    }
    warningAnimData = warningRawData;
    anim = HuSprAnimRead(warningAnimData);
    sprite = HuSprCreate(anim, WARNING_SPRITE_PRIORITY, WARNING_SPRITE_BANK);
    HuSprGrpMemberSet(group, WARNING_SPRITE_MEMBER, sprite);
    HuSprPosSet(group, WARNING_SPRITE_MEMBER, 288.0f, 224.0f * scaleY);
    HuSprScaleSet(group, WARNING_SPRITE_MEMBER, scaleX, scaleY);

    pressStartPacked = warningPressStartData;
    pressStartSize = *pressStartPacked++;
    pressStartRawData = HuMemDirectMalloc(2, pressStartSize);
    pressStartDecodeType = *pressStartPacked++;
    if (pressStartRawData) {
        fn_8000A384(pressStartPacked, pressStartRawData, pressStartSize, pressStartDecodeType);
    }
    pressStartAnimData = pressStartRawData;
    anim = HuSprAnimRead(pressStartAnimData);
    sprite = HuSprCreate(anim, WARNING_SPRITE_PRIORITY, WARNING_SPRITE_BANK);
    HuSprGrpMemberSet(group, PRESS_START_SPRITE_MEMBER, sprite);
    HuSprPosSet(group, PRESS_START_SPRITE_MEMBER, 288.0f, 383.0f * scaleY);
    HuSprDispOff(group, PRESS_START_SPRITE_MEMBER);
    HuSprScaleSet(group, PRESS_START_SPRITE_MEMBER, scaleX, scaleY);

    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
    WipeWait();

    HuPrcSleep(INITIAL_WAIT_FRAMES);
    // This initialization is unused; the blink loop computes the prompt's opacity in tpLevel.
    tpLevelStack[0] = 0.0f;
    phaseSign = 1;
    HuSprDispOn(group, PRESS_START_SPRITE_MEMBER);
    frame = 0;
    while (frame < MAX_INPUT_WAIT_FRAMES &&
           !(lbl_802C0008[0] | lbl_802C0008[1] | lbl_802C0008[2] | lbl_802C0008[3])) {
        if (frame != 0 && (frame % BLINK_PERIOD_FRAMES) == 0) {
            phaseSign = -phaseSign;
        }

        tpLevel = (float)(frame % BLINK_PERIOD_FRAMES) / 30.0f;
        if (phaseSign < 0) {
            tpLevel = 1.0f - tpLevel;
        }

        HuSprTPLvlSet(group, PRESS_START_SPRITE_MEMBER, tpLevel);
        HuPrcVSleep();
        frame++;
    }

    HuSprDispOff(group, PRESS_START_SPRITE_MEMBER);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
    WipeWait();

    HuSprGrpKill(group);
    HuPrcSleep(30);
}

OMOBJ *OutViewObj;
OMOBJMAN *BootObjMan;
HuVecF BootCameraRot[2];
HuVecF BootCameraCenter[2];
float BootCameraZoom[2];
s16 BootLightId[3];
s16 TitleMdlId[4];
s16 BootUnusedBss;
