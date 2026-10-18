/* Battle mode setup, round flow, and camera control. */
#include "dolphin/math.h"
#include "REL/mgmbattledll/module_types.h"
#include "game/mgdata.h"
#include "game/flag.h"
#include "game/saveload.h"
#include "dolphin/mtx.h"
#include "msm_stream.h"
typedef void (*MgBattleDestructor)(void);
extern const MgBattleDestructor _dtors[];

typedef void (*MGBATTLE_CONSTRUCTOR)(void);
extern MGBATTLE_CONSTRUCTOR _ctors[];

void fn_1_1A738(void);
void fn_1_1B3A4(OMOBJ *obj);
void fn_1_409C(void);

void fn_1_B414(void);
void fn_1_35A0(void);
void fn_1_125A0(void);
void fn_1_174FC(void);

s32 lbl_1_data_8D8 = -1;
s32 lbl_1_data_8DC = -1;
s32 lbl_1_data_8E0 = -1;
s32 lbl_1_data_8E4 = -1;
s32 lbl_1_data_8E8 = -1;
s32 lbl_1_data_8EC[4] = { -1, -1, -1, -1 };
s32 lbl_1_data_8FC[4] = { -1, -1, -1, -1 };

extern u8 MgModeWork[256];
extern int MgModeWorkInt[16];
extern s16 MgModeSubMode;
extern s16 MgGameNo;
extern u8 MgModePlayNum;

void fn_1_1B0AC(f32 rotX, f32 rotY, f32 rotZ, const Point3d *position, Point3d *out);
void fn_1_1408(s16 windowCount);
void fn_1_1528(s16 windowCount);
s32 fn_1_1648(s16 messageId);
void fn_1_4548(u16 enabled);

extern s16 MgModeScore[4];

s32 fn_1_1B324(void);
void fn_1_1B358(void);

s32 fn_1_1AF5C(Point3d *vector, Point3d *out);

void fn_1_12A8(void);
void fn_1_139BC(void);
void fn_1_13AAC(void);
void fn_1_148BC(void);
s16 fn_1_156CC(void);
void fn_1_169FC(void);
void fn_1_175A0(void);
void fn_1_17740(void);
void fn_1_1A498(void);
void fn_1_1B394(s16 value);
void fn_1_44E0(void);
void fn_1_7F0(void);
void fn_1_824(void);
void fn_1_BB7C(void);

void fn_1_1B470(void);
void fn_1_1B51C(void);
void fn_1_1B95C(void);
void fn_1_1BE28(void);
void fn_1_1C2B8(OMOBJ *obj);

f32 fn_1_B85C(void);
f32 fn_1_B8E0(void);
s16 fn_1_B964(void);
s32 fn_1_B9E8(void);

void fn_1_3B78(Point3d *position);

/* Initializes the Battle Bridge object manager, camera and round-flow process. */
void fn_1_1A088(void) {
    lbl_1_bss_674 = omInitObjMan(50, 8192);
    CRot.x = -16.0f;
    CRot.y = 0.0f;
    CRot.z = 0.0f;
    Center.x = 775.0f;
    Center.y = 130.0f;
    Center.z = 230.0f;
    CZoom = 2600.0f;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 20.0f, 1.0f, 9500.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    lbl_1_bss_678 = omAddObjEx(lbl_1_bss_674, 32730, 0U, 0U, -1, fn_1_1B3A4);
    HuPrcChildCreate(fn_1_1A738, 100U, 12288U, 0, lbl_1_bss_674);
    fn_1_409C();
}

/* Module entry point runs static constructors before starting battle setup. */
s32 _prolog(void) {
    MGBATTLE_CONSTRUCTOR *constructor;

    constructor = _ctors;
    while (*constructor) {
        (*constructor)();
        constructor++;
    }
    fn_1_1A088();
    return 0;
}

/* Module exit point runs registered static destructors during unload. */
void _epilog(void) {
    const MgBattleDestructor *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Loads battle scene state and initializes lighting, shadows, windows and mode data. */
void fn_1_1A498(void) {
    OMOVLHIS *history;
    HU3D_LIGHTID lightId;
    Point3d shadowPosition;
    Point3d shadowTarget;
    Point3d shadowUp;

    history = omOvlHisGet(0);
    if (history->evtno == 0) {
        *(s16 *)&MgModeSubMode = 0;
        memset(MgModeScore, 0, 8);
        MgGameNo = 3;
        memset(MgModeWork, 0, 256);
        MgModePlayNum = 0;
        memset(MgBattleOrder, 0, 4);
        memset(MgModeWorkInt, 0, 64);
        memset(MgModeWorkFloat, 0, 64);
        lbl_1_bss_672 = 0;
        lbl_1_bss_670 = 0;
    } else {
        lbl_1_bss_672 = (s16)history->evtno;
        lbl_1_bss_670 = 0;
        Center.x = 975.0f;
    }

    _SetFlag(FLAG_INST_MG_MODE);
    lightId = Hu3DGLightCreate(
        0.0f,
        1000.0f,
        1000.0f,
        0.0f,
        -1.0f,
        -0.5f,
        255,
        255,
        255);
    Hu3DGLightInfinitytSet(lightId);
    Hu3DShadowCreate(30.0f, 20.0f, 12000.0f);

    shadowPosition.x = 0.0f;
    shadowPosition.y = 3000.0f;
    shadowPosition.z = 250.0f;
    shadowUp.x = 0.0f;
    shadowUp.y = 1.0f;
    shadowUp.z = -0.25f;
    shadowTarget.x = shadowTarget.y = shadowTarget.z = 0.0f;
    Hu3DShadowPosSet(&shadowPosition, &shadowUp, &shadowTarget);
    Hu3DShadowTPLvlSet(0.6f);

    fn_1_E9C();
    fn_1_ADA8();
    fn_1_317C();
    if (history->evtno != 0) {
        fn_1_B414();
        fn_1_35A0();
    }
    fn_1_125A0();
    fn_1_174FC();
    if (history->evtno != 0) {
        fn_1_175A0();
    }
}

/* Battle mode process advances the round menu, preview, minigame launch and exit flow. */
void fn_1_1A738(void) {
    s32 result;
    OMOVLHIS *history;
    s32 mgNo;
    s16 stateActive = 1;
    history = omOvlHisGet(0);
    fn_1_1A498();
    fn_1_1B394(1);
    HuPrcVSleep();
    fn_1_1B394(0);
    fn_1_44E0();
    lbl_1_data_8D8 = HuAudSStreamPlay(MSM_STREAM_MGMUS_3);
    WipeCreate(1, 0, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }

    while (stateActive) {
        switch (lbl_1_bss_672) {
            case 0:
                fn_1_139BC();
                fn_1_1408(1);
                fn_1_1648(1);
                fn_1_1528(1);
                lbl_1_bss_672 = 1;
                break;
            case 1:
                fn_1_13AAC();
                if (MgGameNo == 3 || MgGameNo == 5 || MgGameNo == 7) {
                    lbl_1_bss_672 = 2;
                } else {
                    lbl_1_bss_670 = lbl_1_bss_672;
                    lbl_1_bss_672 = 10;
                }
                break;
            case 2:
                fn_1_148BC();
                if (MgModeSubMode >= 0 && MgModeSubMode < 3) {
                    lbl_1_bss_672 = 3;
                } else {
                    lbl_1_bss_672 = 1;
                }
                break;
            case 3:
                if (fn_1_156CC() != 0) {
                    lbl_1_bss_672 = 2;
                } else {
                    lbl_1_bss_672 = 4;
                    if (history->evtno == 0) {
                        fn_1_175A0();
                    }
                }
                break;
            case 4:
                fn_1_169FC();
                lbl_1_bss_672 = 5;
                break;
            case 5:
                fn_1_17740();
                lbl_1_bss_672 = 6;
                break;
            case 6:
                fn_1_1408(13);
                result = fn_1_1648(13);
                fn_1_1528(13);
                if ((s16)result == 0) {
                    fn_1_BB7C();
                    lbl_1_bss_672 = 7;
                    omOvlHisChg(0, history->ovl, lbl_1_bss_672,
                        history->stat);
                    fn_1_7F0();
                    HuAudSStreamFadeOut(lbl_1_data_8D8, 1000);
                    WipeCreate(2, 0, 30);
                    while (WipeCheck()) {
                        HuPrcVSleep();
                    }
                    fn_1_824();
                    GwSystem.mgInstDispF = 0;
                    mgNo = MgModeWork[MgModePlayNum] + 601;
                    GWMgUnlockSet(mgNo);
                    GwSystem.mgNo = mgNo - 601;
                    MgModePlayNum++;
                    if (MgModeWork[MgModePlayNum] == 255) {
                        MgModePlayNum = 0;
                    }
                    if (MgModeWorkInt[0] != 0) {
                        GwMgNightF = 1;
                    } else {
                        GwMgNightF = 0;
                    }
                    omOvlCallEx(DLL_instdll, 1, 0, 0);
                    stateActive = 0;
                    for (;;) {
                        HuPrcVSleep();
                    }
                }
                if ((s16)result == 2) {
                    fn_1_BB7C();
                    lbl_1_bss_672 = 7;
                    omOvlHisChg(0, history->ovl, lbl_1_bss_672,
                        history->stat);
                    fn_1_7F0();
                    HuAudSStreamFadeOut(lbl_1_data_8D8, 1000);
                    WipeCreate(2, 0, 30);
                    while (WipeCheck()) {
                        HuPrcVSleep();
                    }
                    fn_1_824();
                    GwSystem.mgInstDispF = 1;
                    mgNo = MgModeWork[MgModePlayNum] + 601;
                    GWMgUnlockSet(mgNo);
                    GwSystem.mgNo = mgNo - 601;
                    MgModePlayNum++;
                    if (MgModeWork[MgModePlayNum] == 255) {
                        MgModePlayNum = 0;
                    }
                    if (MgModeWorkInt[0] != 0) {
                        GwMgNightF = 1;
                    } else {
                        GwMgNightF = 0;
                    }
                    omOvlCallEx(DLL_instdll, 1, 0, 0);
                    stateActive = 0;
                    for (;;) {
                        HuPrcVSleep();
                    }
                }
                lbl_1_bss_670 = lbl_1_bss_672;
                lbl_1_bss_672 = 10;
                break;
            case 7:
                if ((s16)(result = fn_1_17A74()) != 0) {
                    lbl_1_bss_672 = 9;
                } else {
                    lbl_1_bss_672 = 8;
                }
                break;
            case 8:
                fn_1_1408(16);
                fn_1_1648(16);
                fn_1_1528(16);
                lbl_1_bss_672 = 5;
                mgNo = 1;
                while (mgNo != 0) {
                    mgNo = 0;
                    result = 0;
                    while ((s16)result < 4) {
                        if ((s16)fn_1_EE48(result) != 0) {
                            mgNo = 1;
                        }
                        result++;
                    }
                    HuPrcVSleep();
                }
                fn_1_4548(0);
                break;
            case 9:
                fn_1_17E40();
                fn_1_1408(19);
                result = fn_1_1648(19);
                fn_1_1528(19);
                if (lbl_1_data_8E0 >= 0) {
                    HuAudFXStop(lbl_1_data_8E0);
                    lbl_1_data_8E0 = -1;
                }
                if ((s16)result == 0) {
                    HuAudSStreamFadeOut(lbl_1_data_8DC, 1000);
                    WipeCreate(2, 0, 30);
                    while (WipeCheck()) {
                        HuPrcVSleep();
                    }
                    fn_1_7F0();
                    fn_1_824();
                    _ClearFlag(FLAG_INST_MG_MODE);
                    SLSaveModeExec(0);
                    omOvlHisChg(0, history->ovl, 0, 0);
                    omOvlReturnEx(0, 1);
                    stateActive = 0;
                    lbl_1_bss_672 = -1;
                } else if ((s16)result == 1) {
                    stateActive = 0;
                    lbl_1_bss_672 = -1;
                }
                break;
            case 10:
                fn_1_1408(6);
                result = fn_1_1648(6);
                fn_1_1528(6);
                if ((s16)result == 0) {
                    stateActive = 0;
                    HuAudSStreamFadeOut(lbl_1_data_8D8, 1000);
                } else {
                    lbl_1_bss_672 = lbl_1_bss_670;
                }
                break;
            default:
            stateActive = 0;
        }
        HuPrcVSleep();
    }

    HuAudSStreamFadeOut(lbl_1_data_8DC, 1000);
    WipeCreate(2, 0, 30);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    _ClearFlag(FLAG_INST_MG_MODE);
    fn_1_12A8();
    fn_1_7F0();
    fn_1_824();
    SLSaveModeExec(0);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Normalizes a vector, choosing a small random direction when its length is near zero. */
s32 fn_1_1AF5C(Point3d *inputVector, Point3d *normalizedVector) {
    f32 fallbackY;

    if (PSVECSquareMag(inputVector) <= 0.001) {
        normalizedVector->x = 0.01f * ((f32) (u32) frandmod(20) - 10.0f);
        normalizedVector->z = 0.01f * ((f32) (u32) frandmod(20) - 10.0f);
        if ((u32) frandmod(1) != 0) {
            fallbackY = 0.01f;
        } else {
            fallbackY = -0.01f;
        }
        normalizedVector->y = fallbackY;
        PSVECNormalize(normalizedVector, normalizedVector);
        return 0;
    }
    PSVECNormalize(inputVector, normalizedVector);
    return 1;
}

/* Rotates a vector by the supplied Euler angles, which are expressed in degrees. */
void fn_1_1B0AC(f32 angle_x, f32 angle_y, f32 angle_z,
               const Vec *source, Vec *destination)
{
    Mtx x_rotation;
    Mtx y_rotation;
    PSMTXRotRad(x_rotation, 'x', 0.017453292f * angle_x);
    PSMTXRotRad(y_rotation, 'y', 0.017453292f * angle_y);
    PSMTXConcat(y_rotation, x_rotation, x_rotation);
    PSMTXRotRad(y_rotation, 'z', 0.017453292f * angle_z);
    PSMTXConcat(y_rotation, x_rotation, x_rotation);
    PSMTXMultVecSR(x_rotation, source, destination);
}

/* Returns the angle in degrees between two vectors, or zero for coincident points. */
f64 fn_1_1B16C(Point3d *first, Point3d *second) {
    f32 work;
    f64 result;
    f64 lengthProduct;

    work = PSVECMag(first);
    lengthProduct = PSVECMag(second);
    lengthProduct = (f64)work * lengthProduct;
    work = PSVECDotProduct(first, second);
    result = 0.0;
    if (lengthProduct < -0.000001 || 0.000001 < lengthProduct) {
        result = (f64)work / lengthProduct;
    }
    result = acos(result);
    result = 180.0 * (result / 3.141592653589793);

    work = first->x - second->x;
    if (-0.000001 <= work && work <= 0.000001) {
        work = first->y - second->y;
        if (-0.000001 <= work && work <= 0.000001) {
            work = first->z - second->z;
            if (-0.000001 <= work && work <= 0.000001) {
                result = 0.0;
            }
        }
    }
    return result;
}

/* Returns true while the ending camera sequence is within its first 150 frames. */
s32 fn_1_1B324(void) {
    s32 canAdvance;

    canAdvance = 0;
    if (lbl_1_bss_668 <= 150) {
        canAdvance = 1;
    }
    return canAdvance;
}

/* Starts the camera ending sequence at its first frame if it is not already running. */
void fn_1_1B358(void) {
    if (lbl_1_bss_668 <= 0) {
        lbl_1_bss_668 = 1;
    }
}

/* Resets the camera ending-sequence frame counter. */
void fn_1_1B380(void) {
    lbl_1_bss_668 = 0;
}

/* Enables or disables immediate camera snapping for the active mode camera. */
void fn_1_1B394(s16 enabled) {
    lbl_1_bss_66A = enabled;
}

/* Object-manager callback updates the mode camera and ending sequence each frame. */
void fn_1_1B3A4(OMOBJ *obj) {
    if (lbl_1_bss_672 == 7 || lbl_1_bss_672 == 8) {
        if (MgGameNo == 3) {
            fn_1_1B470();
        } else if (MgGameNo == 5) {
            fn_1_1B51C();
        } else if (MgGameNo == 7) {
            fn_1_1B95C();
        }
    }
    if (lbl_1_bss_672 == 9 && lbl_1_bss_668 != 0) {
        fn_1_1BE28();
    }
    fn_1_1C2B8(obj);
}

/* Sets the fixed camera used by the three-player Battle Bridge minigame. */
void fn_1_1B470(void) {
    CRot.x = -29.0f;
    CRot.y = 0.0f;
    CRot.z = 0.0f;
    Center.x = 975.0f;
    Center.y = 230.0f;
    Center.z = 230.0f;
    CZoom = 2730.293f;
}

/* Smoothly tracks player positions and adjusts framing for the active four-player game. */
void fn_1_1B51C(void) {
    f32 centerDelta;
    f32 desiredCenterX;
    s32 playerCount;
    f32 leftPosition;
    f32 rightPosition;

    leftPosition = fn_1_B85C();
    rightPosition = fn_1_B8E0();
    CRot.x += 0.03f * (-29.0f - CRot.x);
    CRot.y += 0.03f * -CRot.y;
    CRot.z += 0.03f * -CRot.z;
    Center.y += 0.03f * (230.0f - Center.y);
    Center.z += 0.03f * (230.0f - Center.z);
    desiredCenterX = leftPosition + rightPosition;
    desiredCenterX *= 0.5f;
    if (desiredCenterX < 975.0f) desiredCenterX = 975.0f;
    if (desiredCenterX > 1475.0f) desiredCenterX = 1475.0f;
    if (fn_1_B964() >= 4) {
        playerCount = fn_1_B9E8();
        if ((s16)playerCount <= 3) desiredCenterX = 1325.0f;
        else desiredCenterX = 1225.0f;
    }
    centerDelta = desiredCenterX - Center.x;
    if (centerDelta > 0.0f) {
        if (centerDelta >= 5.0f) Center.x += 5.0f;
        else Center.x += centerDelta;
    } else if (centerDelta < 0.0f) {
        if (centerDelta <= -5.0f) Center.x += -5.0f;
        else Center.x += centerDelta;
    }
    if (lbl_1_bss_66A != 0) { Center.x = desiredCenterX; CRot.x = -29.0f; Center.y = 230.0f; }
    playerCount = fn_1_B9E8();
    if (fn_1_B964() == 4) playerCount++;
    if ((s16)playerCount <= 3) desiredCenterX = 2730.293f;
    else if ((s16)playerCount <= 4) desiredCenterX = 2910.0f;
    else desiredCenterX = 3650.0f;
    CZoom += 0.03f * (desiredCenterX - CZoom);
    if (lbl_1_bss_66A != 0) CZoom = desiredCenterX;
}

/* Updates the six-player camera framing from the current player positions. */
static inline void fn_1_1B95C_update(void) {
    f32 centerDelta;
    f32 desiredCenterX;
    s32 playerCount;
    f32 rightPosition;
    f32 leftPosition;

    leftPosition = fn_1_B85C();
    rightPosition = fn_1_B8E0();
    CRot.x += 0.03f * (-29.0f - CRot.x);
    CRot.y += 0.03f * -CRot.y;
    CRot.z += 0.03f * -CRot.z;
    Center.y += 0.03f * (230.0f - Center.y);
    Center.z += 0.03f * (230.0f - Center.z);
    desiredCenterX = leftPosition + rightPosition;
    desiredCenterX *= 0.5f;
    if (desiredCenterX < 975.0f) desiredCenterX = 975.0f;
    if (desiredCenterX > 1975.0f) desiredCenterX = 1975.0f;
    if (fn_1_B964() >= 6) {
        playerCount = fn_1_B9E8();
        if ((s16)playerCount <= 3) desiredCenterX = 1955.0f;
        else if ((s16)playerCount <= 4) desiredCenterX = 1785.0f;
        else if ((s16)playerCount <= 5) desiredCenterX = 1625.0f;
        else desiredCenterX = 1475.0f;
    }
    centerDelta = desiredCenterX - Center.x;
    if (centerDelta > 0.0f) {
        if (centerDelta >= 5.0f) Center.x += 5.0f;
        else Center.x += centerDelta;
    } else if (centerDelta < 0.0f) {
        if (centerDelta <= -5.0f) Center.x += -5.0f;
        else Center.x += centerDelta;
    }
    if (lbl_1_bss_66A != 0) { Center.x = desiredCenterX; CRot.x = -29.0f; Center.y = 230.0f; }
    playerCount = fn_1_B9E8();
    if (fn_1_B964() == 6) playerCount++;
    if ((s16)playerCount <= 2) desiredCenterX = 2730.293f;
    else if ((s16)playerCount <= 3) desiredCenterX = 3100.0f;
    else if ((s16)playerCount <= 4) desiredCenterX = 3500.0f;
    else if ((s16)playerCount <= 5) desiredCenterX = 4200.0f;
    else if ((s16)playerCount <= 6) desiredCenterX = 4500.0f;
    else desiredCenterX = 4810.0f;
    CZoom += 0.03f * (desiredCenterX - CZoom);
    if (lbl_1_bss_66A != 0) CZoom = desiredCenterX;
}

/* Smoothly tracks players and zooms out for the six-player Battle Bridge game. */
void fn_1_1B95C(void) {
    fn_1_1B95C_update();
}

/* Advances the 90-frame camera move used by the end-of-round scene. */
void fn_1_1BE28(void) {
    s16 frame;
    Point3d rotation;
    f32 zoomOffset;

    frame = lbl_1_bss_668 - 60;
    if (frame == 0) {
        lbl_1_data_938 = CRot;
        lbl_1_data_944 = Center;
        lbl_1_bss_66C = CZoom;
    }

    if (frame > 1 && frame < 91) {
        rotation.x = -19.0f;
        rotation.y = rotation.z = 0.0f;
        CRot.x = lbl_1_data_938.x +
            (f32)frame *
                ((rotation.x - lbl_1_data_938.x) / 90.0f);
        CRot.y = lbl_1_data_938.y +
            (f32)frame *
                ((rotation.y - lbl_1_data_938.y) / 90.0f);
        CRot.z = lbl_1_data_938.z +
            (f32)frame *
                ((rotation.z - lbl_1_data_938.z) / 90.0f);

        fn_1_3B78(&rotation);
        rotation.y += 203.0f;
        rotation.z += 243.0f;
        Center.x = lbl_1_data_944.x +
            (f32)frame *
                ((rotation.x - lbl_1_data_944.x) / 90.0f);
        Center.y = lbl_1_data_944.y +
            (f32)frame *
                ((rotation.y - lbl_1_data_944.y) / 90.0f);
        Center.z = lbl_1_data_944.z +
            (f32)frame *
                ((rotation.z - lbl_1_data_944.z) / 90.0f);

        zoomOffset = 2446.0f - lbl_1_bss_66C;
        if (zoomOffset < -0.000001 || 0.000001 < zoomOffset) {
            CZoom = zoomOffset / 90.0f * (f32)frame + lbl_1_bss_66C;
        }
    }

    lbl_1_bss_668++;
    if (lbl_1_bss_668 >= 512) {
        lbl_1_bss_668 = 512;
    }
}

/* Rebuilds the battle camera and shadow positions from the current Euler angles. */
void fn_1_1C2B8(OMOBJ *obj) {
    Point3d cameraPosition;
    Point3d center;
    Point3d cameraUp;
    Point3d cameraUpBasis;
    Point3d direction;
    f32 pitch;
    f32 yaw;
    f32 roll;

    pitch = CRot.x;
    yaw = CRot.y;
    roll = CRot.z;
    cameraPosition.x = (f32)((f64)Center.x +
        ((f64)CZoom * (sin((3.141592653589793 * (f64)yaw) / 180.0) *
            cos((3.141592653589793 * (f64)pitch) / 180.0))));
    cameraPosition.y = (f32)((f64)Center.y +
        ((f64)CZoom * -sin((3.141592653589793 * (f64)pitch) / 180.0)));
    cameraPosition.z = (f32)((f64)Center.z +
        ((f64)CZoom * (cos((3.141592653589793 * (f64)yaw) / 180.0) *
            cos((3.141592653589793 * (f64)pitch) / 180.0))));
    center.x = Center.x;
    center.y = Center.y;
    center.z = Center.z;
    cameraUpBasis.x = (f32)(sin((3.141592653589793 * (f64)yaw) / 180.0) *
        sin((3.141592653589793 * (f64)pitch) / 180.0));
    cameraUpBasis.y = (f32)cos((3.141592653589793 * (f64)pitch) / 180.0);
    cameraUpBasis.z = (f32)(cos((3.141592653589793 * (f64)yaw) / 180.0) *
        sin((3.141592653589793 * (f64)pitch) / 180.0));

    PSVECSubtract(&cameraPosition, &center, &direction);
    if (direction.x == 0.0 && direction.y == 0.0 && direction.z == 0.0) {
        direction.z = 1.0f;
    }
    fn_1_1AF5C(&direction, &direction);

    cameraUp.x = (f32)(((f64)cameraUpBasis.x *
        ((f64)(direction.x * direction.x) +
            ((f64)(1.0f - (direction.x * direction.x)) *
                cos((3.141592653589793 * (f64)roll) / 180.0)))) +
        ((f64)cameraUpBasis.y * ((f64)(direction.x * direction.y) *
            (1.0 - cos((3.141592653589793 * (f64)roll) / 180.0)) -
            ((f64)direction.z *
                sin((3.141592653589793 * (f64)roll) / 180.0)))) +
        ((f64)cameraUpBasis.z * ((f64)(direction.x * direction.z) *
            (1.0 - cos((3.141592653589793 * (f64)roll) / 180.0)) +
            ((f64)direction.y *
                sin((3.141592653589793 * (f64)roll) / 180.0)))));

    cameraUp.y = (f32)(((f64)cameraUpBasis.y *
        ((f64)(direction.y * direction.y) +
            ((f64)(1.0f - (direction.y * direction.y)) *
                cos((3.141592653589793 * (f64)roll) / 180.0)))) +
        ((f64)cameraUpBasis.x * ((f64)(direction.x * direction.y) *
            (1.0 - cos((3.141592653589793 * (f64)roll) / 180.0)) +
            ((f64)direction.z *
                sin((3.141592653589793 * (f64)roll) / 180.0)))) +
        ((f64)cameraUpBasis.z * ((f64)(direction.y * direction.z) *
            (1.0 - cos((3.141592653589793 * (f64)roll) / 180.0)) -
            ((f64)direction.x *
                sin((3.141592653589793 * (f64)roll) / 180.0)))));

    cameraUp.z = (f32)(((f64)cameraUpBasis.z *
        ((f64)(direction.z * direction.z) +
            ((f64)(1.0f - (direction.z * direction.z)) *
                cos((3.141592653589793 * (f64)roll) / 180.0)))) +
        (((f64)cameraUpBasis.x * ((f64)(direction.x * direction.z) *
            (1.0 - cos((3.141592653589793 * (f64)roll) / 180.0)) -
            ((f64)direction.y *
                sin((3.141592653589793 * (f64)roll) / 180.0)))) +
            ((f64)cameraUpBasis.y * ((f64)(direction.y * direction.z) *
                (1.0 - cos((3.141592653589793 * (f64)roll) / 180.0)) +
                ((f64)direction.x *
                    sin((3.141592653589793 * (f64)roll) / 180.0))))));

    fn_1_1AF5C(&cameraUp, &cameraUp);

    Hu3DCameraPosSet(1, cameraPosition.x, cameraPosition.y,
        cameraPosition.z, cameraUp.x, cameraUp.y, cameraUp.z,
        center.x, center.y, center.z);
    cameraPosition.x = center.x;
    cameraPosition.y = 2000.0f + center.y;
    cameraPosition.z = 250.0f + center.z;
    cameraUp.x = 0.0f;
    cameraUp.y = 1.0f;
    cameraUp.z = -0.25f;
    Hu3DShadowPosSet(&cameraPosition, &cameraUp, &center);
}

Point3d lbl_1_data_938 = { 0.0f, 0.0f, 0.0f };
Point3d lbl_1_data_944 = { 0.0f, 0.0f, 0.0f };

OMOBJ *lbl_1_bss_678;
HUPROCESS *lbl_1_bss_674;
s16 lbl_1_bss_672;
s16 lbl_1_bss_670;
f32 lbl_1_bss_66C;
s16 lbl_1_bss_66A;
s16 lbl_1_bss_668;
