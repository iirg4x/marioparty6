/* Decathlon Park mode setup, menu sequence and exit handling. */
#include "REL/mgmdecathlondll/module_types.h"
#include "game/flag.h"
#include "game/wipe.h"
#include "dolphin/mtx.h"
#include "msm_stream.h"

#define DECATHLON_MODE_GAME_NUMBER 10000
#define DECATHLON_INITIAL_TIME_RECORD 5940
#define MINIGAME_PUBLIC_NUMBER_BASE 601

extern void fn_1_0(void);
extern void fn_1_C428(void);
extern void fn_1_F184(void);

/* Overlay startup creates the object manager and starts the Decathlon state process. */
void fn_1_EC20(void) {
    lbl_1_bss_C0 = omInitObjMan(100, 8192);
    *(s16 *)&MgModeSubMode = 0;
    fn_1_0();
    HuPrcChildCreate(fn_1_F184, 100U, 12288U, 0, lbl_1_bss_C0);
}

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];

/* The overlay loader calls this entry point before the mode begins. */
s32 _prolog(void) {
    const VoidFunc *constructor;

    constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    lbl_1_bss_C0 = omInitObjMan(100, 8192);
    *(s16 *)&MgModeSubMode = 0;
    fn_1_0();
    HuPrcChildCreate(fn_1_F184, 100U, 12288U, 0, lbl_1_bss_C0);
    return 0;
}

OMOBJMAN *lbl_1_bss_C0;

s32 lbl_1_data_C70 = -1;
s32 lbl_1_data_C74 = -1;
s32 lbl_1_data_C78[5] = {-1, -1, -1, -1, -1};
u32 lbl_1_data_C8C = -1;
s32 lbl_1_data_C90 = -1;
s32 lbl_1_data_C94 = -1;
s32 lbl_1_data_C98[14] = {
    1229, 1228, 1234, 1238, 1236, 1225, 1237, 1226, 1235, 1232, 1227, 1233, 1231, 1230
};
DecathlonModeData lbl_1_data_CD0[10] = {
    {0, &MgDataTbl[0]},
    {3, &MgDataTbl[3]},
    {5, &MgDataTbl[5]},
    {7, &MgDataTbl[7]},
    {14, &MgDataTbl[14]},
    {20, &MgDataTbl[20]},
    {47, &MgDataTbl[47]},
    {49, &MgDataTbl[49]},
    {45, &MgDataTbl[45]},
    {48, &MgDataTbl[48]},
};

extern const VoidFunc _dtors[];

/* The overlay loader calls this entry point while unloading the mode. */
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

extern void fn_1_7BF8(void);
extern void fn_1_92B8(void);
extern void fn_1_BC00(void);
extern void fn_1_E64C(s16 mode, s16 playerNo);
extern s32 fn_1_E23C(s16 mode, s16 playerNo);
extern void fn_1_9688(void);

/* The state process calls this on entry to initialize or restore Decathlon Park. */
void fn_1_ED78(void) {
    OMOVLHIS *overlayHistory;
    HuVecF shadowTop;
    HuVecF shadowOrigin;
    HuVecF shadowUp;
    s16 playerNo;

    overlayHistory = omOvlHisGet(0);
    if (overlayHistory->evtno == 0) {
        *(s16 *)&MgModeSubMode = 1;
        memset(MgModeScore, 0, 8);
        MgGameNo = DECATHLON_MODE_GAME_NUMBER;
        memset(MgModeWork, 0, 256);
        MgModePlayNum = 0;
        for (playerNo = 0; playerNo < 4; playerNo++) {
            MgBattleOrder[playerNo] = playerNo;
        }
        memset(MgModeWorkInt, 0, 64);
        /* Seed the two lower-is-better event records to 99 seconds at 60 frames per second. */
        MgModeWorkInt[6] = DECATHLON_INITIAL_TIME_RECORD;
        MgModeWorkInt[11] = DECATHLON_INITIAL_TIME_RECORD;
        memset(MgModeWorkFloat, 0, 64);
        memset(GwMgDecaScore, 0, 96);
    } else {
        /* The overlay history preserves the results state to resume after a minigame. */
        *(s16 *)&MgModeSubMode = (s16)overlayHistory->evtno;
    }
    _SetFlag(FLAG_INST_MG_MODE);
    _SetFlag(FLAG_INST_DECA);
    playerNo = Hu3DGLightCreate(0.0f, 1000.0f,
                                1000.0f, 0.0f,
                                -1.0f, -0.5f,
                                255, 255, 255);
    Hu3DGLightInfinitytSet(playerNo);
    Hu3DShadowCreate(30.0f, 20.0f, 12000.0f);
    shadowTop.x = 0.0f;
    shadowTop.y = 3000.0f;
    shadowTop.z = 250.0f;
    shadowUp.x = 0.0f;
    shadowUp.y = 1.0f;
    shadowUp.z = -0.25f;
    shadowOrigin.x = shadowOrigin.y = shadowOrigin.z = 0.0f;
    Hu3DShadowPosSet(&shadowTop, &shadowUp, &shadowOrigin);
    Hu3DShadowTPLvlSet(0.6f);
    fn_1_5900();
    fn_1_20D8();
    fn_1_6960();
    fn_1_75C4();
    fn_1_92B8();
    fn_1_BC00();
    fn_1_7BF8();
    if (overlayHistory->evtno != 0) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            MgModeWorkInt[playerNo] = fn_1_E23C(MgModePlayNum, playerNo);
            fn_1_E64C(MgModePlayNum, playerNo);
        }
        MgModePlayNum++;
        fn_1_9688();
        fn_1_C2FC();
        fn_1_C428();
    }
    lbl_1_data_C70 = HuAudSStreamPlay(MSM_STREAM_MGMUS_4);
    WipeCreate(1, 0, 50);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
}

extern s32 SLSaveModeExec(s32 mode);
extern s32 fn_1_184(void);
extern void fn_1_21DC(void);

/* An exit choice fades out, saves mode data, and returns to the preceding overlay. */
void fn_1_F0D8(void) {
    if (lbl_1_data_C90 >= 0) {
        HuAudFXFadeOut(lbl_1_data_C90, 200);
    }
    lbl_1_data_C90 = -1;
    WipeCreate(2, 0, 50);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    fn_1_21DC();
    fn_1_184();
    _ClearFlag(FLAG_INST_MG_MODE);
    _ClearFlag(FLAG_INST_DECA);
    SLSaveModeExec(0);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

extern s16 GwMgNightF;

extern void fn_1_ED78(void);
extern s32 fn_1_184(void);
extern s16 fn_1_28E8(void);
extern void fn_1_188(s16 playerNo);
extern void fn_1_2704(void);
extern void fn_1_6E50(void);
extern void fn_1_78DC(void);
extern void fn_1_5AFC(void);
extern s16 fn_1_7A6C(void);
extern void fn_1_8708(void);
extern void fn_1_87EC(void);
extern void fn_1_6E68(void);
extern void fn_1_97F0(void);
extern void fn_1_98F4(void);
extern void fn_1_9DA0(void);
extern void fn_1_337C(void);
extern void fn_1_9DF8(void);
extern void fn_1_C3DC(void);
extern void fn_1_C344(s16 enabled);
extern void fn_1_C46C(void);
extern s16 fn_1_C51C(void);
extern void fn_1_C428(void);
extern void fn_1_C4C4(void);
extern void fn_1_C540(void);
extern s32 fn_1_4130(void);
extern void fn_1_9CD0(void);
extern void fn_1_5130(void);
extern void fn_1_9AFC(void);
extern void fn_1_5DF8(void);
extern void fn_1_8778(u16 enabled);
extern void fn_1_6DC4(u16 mode);
extern s16 fn_1_6188(void);
extern s16 fn_1_567C(void);
extern void fn_1_E7A4(void);
extern s16 fn_1_22E0(void);
extern void fn_1_21DC(void);

/* Menu cancellation fades the mode music before starting the shared exit sequence. */
static inline void decathlonModeExit(void) {
    HuAudSStreamFadeOut(lbl_1_data_C70, 1000);
    fn_1_F0D8();
}

/* The child process created at startup advances menus, games, results, and exits by state. */
void fn_1_F184(void) {
    OMOVLHIS *history;
    s32 previousMode;
    s16 selection;
    s16 streamWait;
    s32 unlockMode;
    s32 instructionGameNumber;

    history = omOvlHisGet(0);
    switch (MgModeSubMode) {
    case 0:
        fn_1_ED78();
        return;
    case 1:
        MgModeSubMode = 2;
        return;
    case 2:
        selection = fn_1_22E0();
        if (selection != 0) {
            decathlonModeExit();
            return;
        }
        MgModeSubMode = 3;
        return;
    case 3:
        fn_1_188(0);
        fn_1_2704();
        MgModeSubMode = 4;
        return;
    case 4:
        if (lbl_1_data_C90 >= 0) {
            HuAudFXFadeOut(lbl_1_data_C90, 200);
        }
        lbl_1_data_C90 = -1;
        fn_1_188(1);
        fn_1_6E50();
        fn_1_78DC();
        fn_1_5AFC();
        while ((s16)fn_1_7A6C() == 0) {
            HuPrcVSleep();
        }
        MgModeSubMode = 5;
        HuPrcSleep(30);
        return;
    case 5:
        selection = fn_1_28E8();
        if (selection < 0) {
            decathlonModeExit();
            return;
        }
        if (selection > 0) {
            MgModeSubMode = 7;
            return;
        }
        MgModeSubMode = 6;
        return;
    case 6:
        fn_1_8708();
        fn_1_188(0);
        HuPrcVSleep();
        fn_1_87EC();
        fn_1_6E68();
        fn_1_97F0();
        MgModeSubMode = 8;
        return;
    case 7:
        fn_1_188(0);
        HuPrcVSleep();
        fn_1_87EC();
        fn_1_6E68();
        fn_1_97F0();
        MgModeSubMode = 9;
        return;
    case 8:
        /* Resume at the results entrance after launching the event without its instructions. */
        MgModeSubMode = 10;
        omOvlHisChg(0, history->ovl, MgModeSubMode, history->stat);
        HuAudSStreamFadeOut(lbl_1_data_C70, 1000);
        if (lbl_1_data_C90 >= 0) {
            HuAudFXFadeOut(lbl_1_data_C90, 200);
        }
        lbl_1_data_C90 = -1;
        WipeCreate(2, 0, 50);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        GwSystem.mgInstDispF = 0;
        unlockMode = lbl_1_data_CD0[MgModePlayNum].modeId + MINIGAME_PUBLIC_NUMBER_BASE;
        GWMgUnlockSet(unlockMode);
        GwSystem.mgNo = unlockMode - MINIGAME_PUBLIC_NUMBER_BASE;
        GwMgNightF = 0;
        omOvlCallEx(DLL_instdll, 1, 0, 0);
        for (;;) {
            HuPrcVSleep();
        }
    case 9:
        /* This launch path displays the event instructions before the minigame. */
        MgModeSubMode = 10;
        omOvlHisChg(0, history->ovl, MgModeSubMode, history->stat);
        HuAudSStreamFadeOut(lbl_1_data_C70, 1000);
        if (lbl_1_data_C90 >= 0) {
            HuAudFXFadeOut(lbl_1_data_C90, 200);
        }
        lbl_1_data_C90 = -1;
        WipeCreate(2, 0, 50);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        GwSystem.mgInstDispF = 1;
        instructionGameNumber = lbl_1_data_CD0[MgModePlayNum].modeId + MINIGAME_PUBLIC_NUMBER_BASE;
        GWMgUnlockSet(instructionGameNumber);
        GwSystem.mgNo = instructionGameNumber - MINIGAME_PUBLIC_NUMBER_BASE;
        GwMgNightF = 0;
        omOvlCallEx(DLL_instdll, 1, 0, 0);
        for (;;) {
            HuPrcVSleep();
        }
    case 10:
        /* Restore the player lineup after returning from the event overlay. */
        fn_1_98F4();
        fn_1_9DA0();
        MgModeSubMode = 11;
        return;
    case 11:
        fn_1_337C();
        fn_1_9DF8();
        MgModeSubMode = 12;
        return;
    case 12:
        /* Count event awards into the totals, then animate any ranking changes. */
        if (lbl_1_data_C90 >= 0) {
            HuAudFXFadeOut(lbl_1_data_C90, 200);
        }
        lbl_1_data_C90 = -1;
        fn_1_188(2);
        fn_1_C3DC();
        HuPrcSleep(120);
        fn_1_C344(1);
        HuPrcSleep(60);
        fn_1_C46C();
        selection = 1;
        while (selection != 0) {
            if (fn_1_C51C() != 0) {
                selection = 0;
            }
            HuPrcVSleep();
        }
        HuPrcSleep(30);
        fn_1_C344(0);
        fn_1_C428();
        HuPrcSleep(30);
        fn_1_C4C4();
        selection = 1;
        while (selection != 0) {
            if (fn_1_C51C() != 0) {
                selection = 0;
            }
            HuPrcVSleep();
        }
        HuPrcSleep(50);
        fn_1_8708();
        fn_1_188(0);
        if (MgModePlayNum >= 10) {
            MgModeSubMode = 14;
            return;
        }
        MgModeSubMode = 13;
        return;
    case 13:
        fn_1_C540();
        fn_1_8778(1);
        HuPrcSleep(60);
        fn_1_4130();
        fn_1_8778(0);
        fn_1_9CD0();
        MgModeSubMode = 3;
        fn_1_9CD0();
        return;
    case 14:
        /* All ten events are complete: celebrate the total-score leaders. */
        fn_1_5130();
        fn_1_9AFC();
        fn_1_6DC4(1);
        HuPrcSleep(150);
        fn_1_188(3);
        HuPrcSleep(25);
        if (lbl_1_data_C94 >= 0) {
            streamWait = 1;
            while (streamWait != 0) {
                if (HuAudSStreamStatGet(lbl_1_data_C94) == 0) {
                    streamWait = 0;
                }
                HuPrcVSleep();
            }
        }
        lbl_1_data_C74 = HuAudSStreamPlay(MSM_STREAM_MGMUS_KOOPA);
        MgModeSubMode = 15;
        fn_1_5DF8();
        fn_1_8778(1);
        HuPrcVSleep();
        return;
    case 15:
        selection = fn_1_6188();
        if ((s16)selection != 0) {
            MgModeSubMode = 16;
        }
        HuPrcVSleep();
        return;
    case 16:
        HuPrcSleep(45);
        fn_1_188(0);
        MgModeSubMode = -1;
        fn_1_E7A4();
        selection = fn_1_567C();
        HuAudSStreamFadeOut(lbl_1_data_C74, 1000);
        if (lbl_1_data_C90 >= 0) {
            HuAudFXFadeOut(lbl_1_data_C90, 200);
        }
        lbl_1_data_C90 = -1;
        fn_1_6DC4(0);
        if (selection == 0) {
            return;
        }
        WipeCreate(2, 0, 50);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        _ClearFlag(FLAG_INST_MG_MODE);
        _ClearFlag(FLAG_INST_DECA);
        SLSaveModeExec(0);
        omOvlHisChg(0, history->ovl, 0, 0);
        omOvlReturnEx(0, 1);
        while (1) {
            HuPrcVSleep();
        }
    }
    fn_1_F0D8();
}

/* Player and scenery updates normalize directions here, choosing a random fallback near zero
 * length. */
s32 fn_1_FB2C(Vec *direction, Vec *unitDirection) {
    f32 verticalComponent;

    if (PSVECSquareMag(direction) <= 0.001) {
        unitDirection->x = 0.01f * ((f32)frandmod(20) - 10.0f);
        unitDirection->z = 0.01f * ((f32)frandmod(20) - 10.0f);
        /* A modulus of one always returns zero, so the fallback's Y component is negative. */
        if (frandmod(1) != 0) {
            verticalComponent = 0.01f;
        } else {
            verticalComponent = -0.01f;
        }
        unitDirection->y = verticalComponent;
        PSVECNormalize(unitDirection, unitDirection);
        return 0;
    }
    PSVECNormalize(direction, unitDirection);
    return 1;
}

/* Player movement rotates a direction by X, then Y, then Z angles in degrees. */
void fn_1_FC84(f32 angleX, f32 angleY, f32 angleZ,
               Vec source, Vec *destination)
{
    Mtx combinedRotation;
    Mtx axisRotation;

    PSMTXRotRad(combinedRotation, 'x', 0.017453292f * angleX);
    PSMTXRotRad(axisRotation, 'y', 0.017453292f * angleY);
    PSMTXConcat(axisRotation, combinedRotation, combinedRotation);
    PSMTXRotRad(axisRotation, 'z', 0.017453292f * angleZ);
    PSMTXConcat(axisRotation, combinedRotation, combinedRotation);
    PSMTXMultVecSR(combinedRotation, &source, destination);
}

/* Player and scenery actor updates call this for the angle in degrees between directions. */
f64 fn_1_FD44(Vec first, Vec second) {
    f32 work;
    f64 result;
    f64 lengthProduct;

    work = PSVECMag(&first);
    lengthProduct = PSVECMag(&second);
    lengthProduct = (f64)work * lengthProduct;
    work = PSVECDotProduct(&first, &second);
    result = 0.0;
    if (lengthProduct < -1e-06 || 1e-06 < lengthProduct) {
        result = (f64)work / lengthProduct;
    }
    result = acos(result);
    result = 180.0 * (result / 3.141592653589793);

    /* Treat identical vectors as no turn even if floating-point math leaves residue. */
    work = first.x - second.x;
    if (-1e-06 <= work && work <= 1e-06) {
        work = first.y - second.y;
        if (-1e-06 <= work && work <= 1e-06) {
            work = first.z - second.z;
            if (-1e-06 <= work && work <= 1e-06) {
                result = 0.0;
            }
        }
    }
    return result;
}
