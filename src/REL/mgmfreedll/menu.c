/* MGMfree menu setup, selection screens, model motion, and event handling. */
#include <math.h>
#include <dolphin/gx.h>
#include <game/hu3d.h>
#include <game/charman.h>
#include <game/memory.h>
#include <game/sprite.h>
#include <game/window.h>
#include <string.h>

typedef struct MgmfreeSpriteInit {
    s16 groupNo;
    s16 memberNo;
    s16 animNo;
    s16 priority;
    s16 bank;
    f32 posX;
    f32 posY;
    f32 scaleX;
    f32 scaleY;
    f32 zRot;
} MGMFREE_SPRITE_INIT;

extern HU3D_LIGHTID lbl_1_bss_9E0[2];
extern HUWINID lbl_1_bss_9DC[2];
extern u32 lbl_1_data_388;
extern ANIMDATA *lbl_1_bss_554[11];
extern HUSPR_GROUPID lbl_1_bss_53E[10];
extern HUSPRID lbl_1_bss_518[19];
extern u32 lbl_1_data_10[11];
extern s16 lbl_1_data_3C[10];
extern MGMFREE_SPRITE_INIT lbl_1_data_50[19];
extern ANIMDATA *lbl_1_bss_4DC[7];
extern u32 lbl_1_bss_4C0[7];

void fn_1_0(HUWINID winId, u32 mess, s16 index);
void fn_1_E5AC(HUSPR_GROUPID groupId, s32 attr);

#include <game/mgdata.h>
#include <game/object.h>
#include <game/thpmain.h>

extern ANIMDATA *lbl_1_bss_40;
extern OMOBJMAN *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_20;
extern OMOBJ *lbl_1_bss_44;
extern u8 lbl_1_bss_3A[6];
extern s16 lbl_1_bss_4A8;
extern HUSPRID lbl_1_bss_4AA[2];
extern HUSPR_GROUPID lbl_1_bss_4AE;
extern HU3D_ANIMID lbl_1_bss_4B0[7];
extern s16 lbl_1_bss_580[8][32][2];
extern s16 lbl_1_bss_9D0[4];
extern s16 lbl_1_data_3AE;
extern s16 lbl_1_data_3CC;
extern s16 lbl_1_data_3CE;

#include <REL/mgmfreedll/include/game/audio.h>
#include <game/data.h>
#include <game/frand.h>
#include <game/gamework.h>
#include <game/main.h>
#include <messdir_enum.h>

enum { MGMFREE_SE_MODE_MENU_ENTRY_FX = 1217 };
#define MGMFREE_SE_RANDOM_MODEL_DISPLAY_START 1220

extern OMOBJ *lbl_1_bss_30;
/* These records track menu model animation state and transition timing. */
typedef struct MgmfreeModelState {
    s16 state;
    u8 unusedBeforeTiming[38];
    f32 interpolationTime;
    f32 interpolationDuration;
    union {
        f32 countdown;
        f32 transitionStart;
    } value;
    f32 transitionTarget;
    f32 transitionActive;
    u8 unusedAfterTransition[4];
} MGMFREE_MODEL_STATE;

extern MGMFREE_MODEL_STATE lbl_1_bss_A8[16];
void fn_1_6848(s16 mode);
f32 fn_1_E35C(f32 start, f32 end, f32 time, f32 duration);
f32 fn_1_E3E8(f32 start, f32 end, f32 time, f32 duration);

#include <dolphin/os.h>
extern char lbl_1_data_38C[];
extern s32 lbl_1_bss_0;
extern s16 lbl_1_bss_980[8];
extern s16 lbl_1_bss_990[8];
extern s16 lbl_1_bss_9A0[4][6];
extern s16 lbl_1_data_0[8];

/* Category display order used by the menu list; -1 ends the sequence. */
s16 lbl_1_data_0[8] = {0, 1, 2, 3, 6, 7, 4, -1};
u32 lbl_1_data_10[11] = {
    DATANUM(DATA_mgmfree, 19), DATANUM(DATA_mgmfree, 22), DATANUM(DATA_mgmfree, 26),
    DATANUM(DATA_mgmfree, 23), DATANUM(DATA_mgmfree, 24), DATANUM(DATA_mgmfree, 25),
    DATANUM(DATA_mgmfree, 16), DATANUM(DATA_mgmfree, 17), DATANUM(DATA_mgmfree, 18),
    DATANUM(DATA_mgmfree, 20), DATANUM(DATA_mgmfree, 21)
};
s16 lbl_1_data_3C[10] = {1, 1, 4, 2, 2, 2, 2, 1, 2, 2};
/* Sprite initializers loaded by fn_1_DA4 for the menu's animation groups. */
MGMFREE_SPRITE_INIT lbl_1_data_50[19] = {
    {0, 0, 0, 10, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {1, 0, 1, 6, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {2, 0, 2, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {2, 1, 2, 0, 2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {2, 2, 2, 0, 3, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {2, 3, 2, 0, 1, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {3, 0, 3, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {3, 1, 4, 10, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {4, 0, 3, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {4, 1, 4, 10, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {5, 0, 3, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {5, 1, 4, 10, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {6, 0, 3, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {6, 1, 4, 10, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {7, 0, 5, 0, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {8, 0, 9, 5, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {8, 1, 1, 3, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {9, 0, 10, 5, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
    {9, 1, 1, 3, 0, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f}
};
s16 lbl_1_data_2B0[4] = {4, 3, 6, 3};
/* Selection remapping entries consumed as signed halfwords by fn_1_CAD4. */
u8 lbl_1_data_2B8[200] = { 0, 0, 0, 1, 0, 2, 0, 3, 0, 1, 0, 0, 0, 2, 0, 3, 0, 2, 0, 0, 0, 1, 0,
                           3, 0, 3, 0, 0, 0, 1, 0, 2, 0, 0, 0, 1, 0, 2, 0, 3, 0, 0, 0, 1, 0, 2,
                           0, 3, 0, 0, 0, 1, 0, 2, 0, 3, 0, 0, 0, 2, 0, 1, 0, 3, 0, 0, 0, 3, 0,
                           1, 0, 2, 0, 0, 0, 1, 0, 2, 0, 3, 0, 0, 0, 1, 0, 2, 0, 3, 0, 0, 0, 1,
                           0, 2, 0, 3, 0, 0, 0, 1, 0, 2, 0, 3, 0, 0, 0, 2, 0, 1, 0, 3, 0, 0, 0,
                           3, 0, 1, 0, 2, 0, 1, 0, 2, 0, 0, 0, 3, 0, 1, 0, 3, 0, 0, 0, 2, 0, 2,
                           0, 3, 0, 0, 0, 1, 0, 0, 0, 1, 0, 2, 0, 3, 0, 0, 0, 2, 0, 1, 0, 3, 0,
                           0, 0, 3, 0, 1, 0, 2, 0, 1, 0, 2, 0, 0, 0, 3, 0, 1, 0, 3, 0, 0, 0, 2,
                           0, 2, 0, 3, 0, 0, 0, 1, 0, 0, 0, 1, 0, 2, 0, 3 };
s16 lbl_1_data_380[4] = {0, 1, 2, 3};
/* Last text-message ID used by the window callback's sound de-duplication. */
u32 lbl_1_data_388 = -1;
char lbl_1_data_38C[] = "# ========== win callback :: %d\n";
/* Movie state: 0–6 advance playback; 100 is ready and 999 requests close. */
s16 lbl_1_data_3AE = 100;

HU3D_LIGHTID lbl_1_bss_9E0[2];
HUWINID lbl_1_bss_9DC[2];
s32 lbl_1_bss_9D8;
/* Current category, page, and selection state shared by the menu flow. */
s16 lbl_1_bss_9D0[4];
s16 lbl_1_bss_9A0[4][6];
s16 lbl_1_bss_990[8];
s16 lbl_1_bss_980[8];
/* Each list entry stores a minigame data index or -1, plus its unlock flag. */
s16 lbl_1_bss_580[8][32][2];
ANIMDATA *lbl_1_bss_554[11];
HUSPR_GROUPID lbl_1_bss_53E[10];
HUSPRID lbl_1_bss_518[19];
f32 lbl_1_bss_508[4];
f32 lbl_1_bss_4F8[4];
ANIMDATA *lbl_1_bss_4DC[7];
u32 lbl_1_bss_4C0[7];
HU3D_ANIMID lbl_1_bss_4B0[7];

HUSPR_GROUPID lbl_1_bss_4AE;
HUSPRID lbl_1_bss_4AA[2];
s16 lbl_1_bss_4A8;
MGMFREE_MODEL_STATE lbl_1_bss_A8[16];
/* Scale values used by the four option markers and the cursor. */
f32 lbl_1_bss_98[4];
f32 lbl_1_bss_94;
HUWINID lbl_1_bss_84[8];
HUWINID lbl_1_bss_80[2];
HUWINID lbl_1_bss_7A[3];
HUWINID lbl_1_bss_78;
/* These ten-character buffers hold formatted unlocked and total minigame counts. */
char lbl_1_bss_6E[10];
char lbl_1_bss_64[10];
s16 lbl_1_bss_4C[12];
f32 lbl_1_bss_48;
OMOBJ *lbl_1_bss_44;
ANIMDATA *lbl_1_bss_40;
/* This array is only zeroed at its first halfword in this TU; its role is unknown. */
u8 lbl_1_bss_3A[6];
s16 lbl_1_bss_38;
OMOBJ *lbl_1_bss_34;
OMOBJ *lbl_1_bss_30;
/* The first word points to the active item-window object; the remaining bytes have no known use. */
u8 lbl_1_bss_28[8];
OMOBJ *lbl_1_bss_24;
OMOBJ *lbl_1_bss_20;
OMOBJ *lbl_1_bss_1C;
/* The first word points to the model-display object; the remaining bytes have no known use. */
u8 lbl_1_bss_C[16];
OMOBJMAN *lbl_1_bss_8;
u32 lbl_1_bss_4;
s32 lbl_1_bss_0;

/* Play the effect selected by the text-window callback index once per distinct message. */
void fn_1_0(HUWINID winId, u32 mess, s16 index)
{
    s32 sentinel[1] = {-1};
    s32 sounds[16] = { MSM_SE_GUIDE_25, MSM_SE_GUIDE_26, MSM_SE_GUIDE_27, MSM_SE_GUIDE_28,
                       MSM_SE_GUIDE_29, MSM_SE_GUIDE_30, MSM_SE_GUIDE_31, -1,
                       MSM_SE_GUIDE_17, MSM_SE_GUIDE_18, MSM_SE_GUIDE_19, MSM_SE_GUIDE_20,
                       MSM_SE_GUIDE_21, MSM_SE_GUIDE_22, MSM_SE_GUIDE_23, -1 };
    s16 i;

    index--;
    OSReport(lbl_1_data_38C, (s16)index);
    if (lbl_1_data_388 != mess) {
        lbl_1_data_388 = mess;
        for (i = 0; ; i++) {
            if (sentinel[i] == -1) {
                HuAudFXPlay(sounds[index]);
                return;
            }
            if (mess == (u32)sentinel[i]) {
                if (index >= 8) {
                    HuAudFXPlayPan(sounds[index], 80);
                    return;
                }
                HuAudFXPlayPan(sounds[index], 48);
                return;
            }
        }
    }
}

/* Initialize the four menu counters from the current mode and its available item counts. */
void fn_1_1A4(void)
{
    s16 i;
    s16 *p;
    s16 j;
    s16 *q;

    q = lbl_1_bss_9D0;
    q[0] = 0;
    if (lbl_1_bss_0 >= 1) {
        lbl_1_bss_9D0[0] = lbl_1_bss_0 - 1;
        lbl_1_bss_9D0[1] = 0;
        lbl_1_bss_9D0[2] = 0;
        lbl_1_bss_9D0[3] = 0;
    }

    for (i = 0, p = lbl_1_bss_9A0[0]; i < 4; i++, p += 6) {
        p[0] = i;
        p[1] = 0;
        p[2] = GwPlayerConf[p[0]].type;
        p[3] = GwPlayerConf[p[0]].comDif;
        p[4] = GwPlayerConf[p[0]].charNo;
        p[5] = GwPlayerConf[p[0]].padNo;
    }

    for (i = 0; i < 8; i++) {
        lbl_1_bss_990[i] = 0;
        lbl_1_bss_980[i] = 0;
        for (j = 0; j < 32; j++) {
            lbl_1_bss_580[i][j][0] = -1;
            lbl_1_bss_580[i][j][1] = 0;
        }
    }

    for (i = 0; i < 100; i++) {
        if (MgDataTbl[i].ovl == (u16)-1) {
            break;
        }
        for (j = 0; j < 7; j++) {
            if (MgDataTbl[i].flag & MG_FLAG_MIC) {
                break;
            }
            if (MgDataTbl[i].flag & MG_FLAG_RARE) {
                lbl_1_bss_580[7][lbl_1_bss_990[7]][0] = i;
                lbl_1_bss_580[7][lbl_1_bss_990[7]][1] = GWMgUnlockGet(i + 601);
                if (lbl_1_bss_580[7][lbl_1_bss_990[7]][1] == 1) {
                    lbl_1_bss_980[7]++;
                }
                lbl_1_bss_990[7]++;
                break;
            }
            if (lbl_1_data_0[j] == MgDataTbl[i].type) {
                lbl_1_bss_580[j][lbl_1_bss_990[j]][0] = i;
                lbl_1_bss_580[j][lbl_1_bss_990[j]][1] = GWMgUnlockGet(i + 601);
                if (lbl_1_bss_580[j][lbl_1_bss_990[j]][1] == 1) {
                    lbl_1_bss_980[j]++;
                }
                lbl_1_bss_990[j]++;
                break;
            }
        }
    }
}

/* Create and place the shared MGMfree camera during scene setup. */
void fn_1_624(void)
{
    Hu3DCameraCreate(HU3D_CAM_ALL);
    Hu3DCameraPerspectiveSet(HU3D_CAM_ALL, 20.0f, 10.0f, 10000.0f, 1.2f);
    Hu3DCameraViewportSet(HU3D_CAM_ALL, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPosSet(HU3D_CAM_ALL, 0.0f, 250.0f, 2150.0f, 0.0f, 1.0f,
                    0.0f, 0.0f, 260.0f, 0.0f);
}

void fn_1_758(void)
{
    Hu3DCameraKill(HU3D_CAM_ALL);
}

/* Create the two directional scene lights used by the MGMfree models. */
void fn_1_780(void)
{
    {
        Vec position[2] = {{0.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f}};
        Vec direction[2] = {{0.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}};
        GXColor color = {255, 255, 255, 255};
        s16 lightNo;

        for (lightNo = 0; lightNo < 2; lightNo++) {
            lbl_1_bss_9E0[lightNo] =
                Hu3DGLightCreateV(&position[lightNo], &direction[lightNo], &color);
            Hu3DGLightInfinitytSet(lbl_1_bss_9E0[lightNo]);
            Hu3DGLightStaticSet(lbl_1_bss_9E0[lightNo], 1);
        }
    }
}

/* Release both scene lights during module teardown. */
void fn_1_8D0(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DGLightKill(lbl_1_bss_9E0[i]);
    }
}

/* Show a standard or framed message window before its message is read. */
void fn_1_928(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_9DC[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_9DC[winNo]);
    }
}

/* Hide or close the selected message window after its message is handled. */
void fn_1_998(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_9DC[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_9DC[winNo]);
    }
}

/* Set centered message text and its reveal speed, resetting duplicate-sound tracking. */
void fn_1_A08(s16 winNo, u32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_9DC[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_9DC[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_9DC[winNo], speed);
    if (lbl_1_data_388 != messNum) {
        lbl_1_data_388 = -1;
    }
}

void fn_1_AC4(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_9DC[winNo]);
}

/* Read a window choice, using choice 1 when mode 2 receives no selection. */
s16 fn_1_B00(s16 winNo, s16 mode)
{
    s16 choice = 0;

    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_9DC[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_9DC[winNo], HUWIN_ATTR_NOCANCEL);
    }
    choice = HuWinChoiceGet(lbl_1_bss_9DC[winNo], -1);
    if (mode == 2 && choice == -1) {
        choice = 1;
    }
    return choice;
}

/* Create the two framed message windows used for prompts and status text. */
void fn_1_BD4(void)
{
    s16 winNo;

    HuWinInit(1);
    lbl_1_bss_9DC[0] = HuWinExCreateFrame(16.0f, 398.0f, 544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_9DC[0]);
    HuWinBGTPLvlSet(lbl_1_bss_9DC[0], 0.0f);
    lbl_1_bss_9DC[1] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 3);
    HuWinDispOff(lbl_1_bss_9DC[1]);
    HuWinBGTPLvlSet(lbl_1_bss_9DC[1], 0.8999999761581421f);
    HuWinPriSet(lbl_1_bss_9DC[1], 0);
    for (winNo = 0; winNo < 2; winNo++) {
        winData[lbl_1_bss_9DC[winNo]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_9DC[winNo], fn_1_0);
    }
}

/* Destroy the module's message windows and clear remaining window state. */
void fn_1_D48(void)
{
    s16 winNo;

    for (winNo = 0; winNo < 2; winNo++) {
        HuWinExKill(lbl_1_bss_9DC[winNo]);
    }
    HuWinAllKill();
}

/* Load the menu sprite animations and create their groups from the initialized sprite table. */
void fn_1_DA4(void)
{
    MGMFREE_SPRITE_INIT *entry;
    s16 i;

    entry = (MGMFREE_SPRITE_INIT *)lbl_1_data_50;
    for (i = 0; i < 11; i++) {
        lbl_1_bss_554[i] =
            HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_10[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 10; i++) {
        lbl_1_bss_53E[i] = HuSprGrpCreate(lbl_1_data_3C[i]);
    }
    for (i = 0; i < 19; i++, entry++) {
        lbl_1_bss_518[i] = HuSprCreate(lbl_1_bss_554[entry->animNo], entry->priority, entry->bank);
        HuSprGrpMemberSet(lbl_1_bss_53E[entry->groupNo], entry->memberNo, lbl_1_bss_518[i]);
        HuSprPosSet(lbl_1_bss_53E[entry->groupNo], entry->memberNo, entry->posX, entry->posY);
        HuSprScaleSet(lbl_1_bss_53E[entry->groupNo], entry->memberNo, entry->scaleX, entry->scaleY);
        HuSprZRotSet(lbl_1_bss_53E[entry->groupNo], entry->memberNo, entry->zRot);
    }
    for (i = 0; i < 10; i++) {
        fn_1_E5AC(lbl_1_bss_53E[i], HUSPR_ATTR_DISPOFF);
    }
}

void fn_1_FB8(void)
{
}

/* Create seven sprite animations and allocate and clear their texture buffers. */
void fn_1_FBC(void)
{
    u16 image[7][3] = {
        {16, 16, 0},
        {16, 16, 0},
        {16, 16, 0},
        {16, 16, 0},
        {16, 16, 0},
        {128, 128, 0},
        {16, 16, 0}
    };
    s16 i;
    ANIMBMP *bitmap;

    for (i = 0; i < 7; i++) {
        lbl_1_bss_4DC[i] = HuSprAnimMake((s16)image[i][0], (s16)image[i][1], 0);
        lbl_1_bss_4C0[i] = GXGetTexBufferSize(image[i][0], image[i][1], GX_TF_RGBA8, 0, 0);
        bitmap = lbl_1_bss_4DC[i]->bmp;
        bitmap->data = HuMemDirectMallocNum(HEAP_HEAP, lbl_1_bss_4C0[i], HU_MEMNUM_OVL);
        memset(bitmap->data, image[i][2], lbl_1_bss_4C0[i]);
    }
}

/* Release the seven sprite animations loaded for the menu scene. */
void fn_1_11C0(void)
{
    s16 i;

    for (i = 0; i < 7; i++) {
        HuSprAnimKill(lbl_1_bss_4DC[i]);
    }
}

/* Advance the title movie's close, start-check, playback-wait, and sprite-ready states each object
 * tick. */
void fn_1_1218(OMOBJ *obj)
{
    switch (lbl_1_data_3AE) {
    case 0:
        HuTHPClose();
        lbl_1_data_3AE = 1;
        break;
    case 1:
        HuTHPClose();
        if (HuTHPProcCheck() == 0) {
            lbl_1_data_3AE = 2;
        }
        break;
    case 2:
        lbl_1_bss_4AA[1] = HuTHPSprCreateVol(MgDataTbl[lbl_1_bss_4A8].movie[0][0], 1, 200, 0.0f);
        HuSprGrpMemberSet(lbl_1_bss_4AE, 1, lbl_1_bss_4AA[1]);
        HuSprGrpDrawNoSet(lbl_1_bss_4AE, 127);
        lbl_1_data_3AE = 5;
        break;
    case 3:
        if (HuTHPStartCheck() == 1) {
            lbl_1_data_3AE = 4;
            obj->work[0] = 0;
            obj->work[1] = 10;
        }
        break;
    case 4:
        if (++obj->work[0] > obj->work[1]) {
            lbl_1_data_3AE = 5;
        }
        break;
    case 5:
        if (HuTHPStartCheck() == 1) {
            lbl_1_data_3AE = 6;
            HuSprAttrSet(lbl_1_bss_4AE, 0, 4);
            obj->work[0] = 0;
            obj->work[1] = 10;
        }
        break;
    case 6:
        if (++obj->work[0] > obj->work[1]) {
            lbl_1_data_3AE = 100;
        }
        break;
    }
}

/* Select the sprite set for the current menu category and prepare its playback state. */
void fn_1_1400(s16 mode)
{
    s16 category;
    s16 index;

    if (lbl_1_data_3AE != 999) {
        if (lbl_1_bss_40) {
            HuSprKill(lbl_1_bss_4AA[0]);
            HuSprAnimKill(lbl_1_bss_40);
            lbl_1_bss_40 = NULL;
        }
        category = lbl_1_bss_9D0[0];
        index = (lbl_1_bss_9D0[2] + 2 * lbl_1_bss_9D0[1]) + 2 * lbl_1_bss_9D0[3];
        lbl_1_bss_40 = HuSprAnimRead(HuDataSelHeapReadNum(
            MgDataTbl[lbl_1_bss_580[category][index][0]].instPic[0][0], HU_MEMNUM_OVL, HEAP_MODEL));
        HuSprAnimLock(lbl_1_bss_40);
        lbl_1_bss_4AA[0] = HuSprCreate(lbl_1_bss_40, 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_4AE, 0, lbl_1_bss_4AA[0]);
        HuSprAttrReset(lbl_1_bss_4AE, 0, 4);
        HuSprGrpDrawNoSet(lbl_1_bss_4AE, 127);
        if (mode == 2) {
            Hu3DAnimAnimSet(lbl_1_bss_4B0[5], lbl_1_bss_554[7]);
            return;
        }
        if (lbl_1_bss_580[category][index][0] == -1 || lbl_1_bss_580[category][index][1] == 0 ||
            mode == 0) {
            Hu3DAnimAnimSet(lbl_1_bss_4B0[5], lbl_1_bss_554[6]);
            return;
        }
        lbl_1_bss_4A8 = lbl_1_bss_580[category][index][0];
        lbl_1_data_3AE = 0;
        Hu3DAnimAnimSet(lbl_1_bss_4B0[5], lbl_1_bss_4DC[5]);
    }
}

/* Copy the menu's captured texture during the registered 3D layer hook. */
void fn_1_16B4(s16 layerNo)
{
    ANIMBMP *bitmap;

    bitmap = lbl_1_bss_4DC[5]->bmp;
    GXSetTexCopySrc(0, 320, 128, 128);
    GXSetTexCopyDst(128, 128, GX_TF_RGBA8, 0);
    GXCopyTex(bitmap->data, 0);
}

/* Wait for the title movie to finish before closing its player and marking playback complete. */
void fn_1_171C(void)
{
    do {
        HuPrcVSleep();
    } while (lbl_1_data_3AE != 100 && lbl_1_data_3AE != 999);
    lbl_1_data_3AE = 999;
    HuTHPClose();
    while (HuTHPProcCheck() != 0) {
        HuTHPClose();
        HuPrcVSleep();
    }
}

s16 lbl_1_data_3CC = -1;
s16 lbl_1_data_3CE = -1;

/* Initialize the option cursor and title-movie playback state and callback. */
void fn_1_1790(void)
{
    lbl_1_bss_4AE = HuSprGrpCreate(2);
    HuSprGrpPosSet(lbl_1_bss_4AE, 58.0f, 384.0f);
    HuSprGrpScaleSet(lbl_1_bss_4AE, 0.5f, 0.5f);
    HuSprExecLayerSet(64, 0);
    lbl_1_data_3CC = -1;
    lbl_1_data_3CE = -1;
    lbl_1_bss_4A8 = -1;
    lbl_1_data_3AE = 100;
    ((s16 *)lbl_1_bss_3A)[0] = 0;
    fn_1_FBC();
    lbl_1_bss_44 = omAddObjEx(lbl_1_bss_8, 4096, 16, 16, -1, fn_1_1218);
    Hu3DLayerHookSet(8, fn_1_16B4);
}

/* Choose which model animations run for the current menu display mode. */
void fn_1_1A98(s16 mode)
{
    if (mode == 1) {
        Hu3DAnimAnimSet(lbl_1_bss_4B0[2], lbl_1_bss_554[8]);
        Hu3DAnimAnimSet(lbl_1_bss_4B0[3], lbl_1_bss_554[8]);
        return;
    }
    if (mode == 2) {
        Hu3DAnimAnimSet(lbl_1_bss_4B0[1], lbl_1_bss_554[8]);
        Hu3DAnimAnimSet(lbl_1_bss_4B0[2], lbl_1_bss_554[8]);
        Hu3DAnimAnimSet(lbl_1_bss_4B0[3], lbl_1_bss_554[8]);
        Hu3DModelRotSet(lbl_1_bss_20->mdlId[1], 0.0f, -210.0f, 0.0f);
        Hu3DModelRotSet(lbl_1_bss_20->mdlId[2], 0.0f, -210.0f, 0.0f);
        Hu3DModelRotSet(lbl_1_bss_20->mdlId[3], 0.0f, -210.0f, 0.0f);
        return;
    }
    Hu3DAnimAnimSet(lbl_1_bss_4B0[1], lbl_1_bss_4DC[0]);
    Hu3DAnimAnimSet(lbl_1_bss_4B0[2], lbl_1_bss_4DC[0]);
    Hu3DAnimAnimSet(lbl_1_bss_4B0[3], lbl_1_bss_4DC[0]);
}

/* Update animated menu models each frame and advance their timed transitions. */
void fn_1_1C74(OMOBJ *obj)
{
    s16 table[4][6] = {
        {4, 1080, 2, -750, 1500, 240},
        {7, 1080, 3, 0, 2167, 5400},
        {8, 1080, 3, 0, 2167, 5400},
        {9, 1080, 5, 0, 2167, 5400}
    };
    Vec position;
    MGMFREE_MODEL_STATE *record;
    f32 value;
    s16 i;
    s16 j;

    Hu3DMotionSpeedSet(obj->mdlId[0], 0.5f);
    record = &lbl_1_bss_A8[2];
    value = fn_1_E35C(0.0f, 2167.0f, record->interpolationTime, record->interpolationDuration);
    if ((record->interpolationTime += 1.0f) > record->interpolationDuration) {
        record->interpolationTime = 0.0f;
        record->interpolationDuration = 10800.0f;
    }
    Hu3DModelPosSet(obj->mdlId[2], value, 0.0f, 0.0f);
    record = &lbl_1_bss_A8[3];
    value = fn_1_E35C(0.0f, 2167.0f, record->interpolationTime, record->interpolationDuration);
    if ((record->interpolationTime += 1.0f) > record->interpolationDuration) {
        record->interpolationTime = 0.0f;
        record->interpolationDuration = 5400.0f;
    }
    Hu3DModelPosSet(obj->mdlId[3], value, 0.0f, 0.0f);
    for (i = 0; i < 4; i++) {
        record = &lbl_1_bss_A8[table[i][0]];
        if (record->state == 0) {
            if ((record->value.countdown -= 1.0f) < 0.0f) {
                record->value.countdown = table[i][1];
                if (rand8() % table[i][2] == 0) {
                    record->state = 1;
                    record->interpolationTime = 0.0f;
                    record->interpolationDuration = table[i][5];
                    if (i == 0) {
                        HuAudFXPlay(MGMFREE_SE_RANDOM_MODEL_DISPLAY_START);
                    }
                    if (i == 0) {
                        for (j = 0; j < 3; j++) {
                            Hu3DModelAttrSet(lbl_1_bss_30->mdlId[j], HU3D_ATTR_DISPOFF);
                        }
                        Hu3DModelAttrReset(lbl_1_bss_30->mdlId[rand8() % 3], HU3D_ATTR_DISPOFF);
                        fn_1_6848(0);
                    }
                }
            }
        } else {
            value = fn_1_E35C(table[i][3], table[i][4], record->interpolationTime,
                              record->interpolationDuration);
            if ((record->interpolationTime += 1.0f) > record->interpolationDuration) {
                record->state = 0;
            }
            Hu3DModelPosSet(obj->mdlId[table[i][0]], value, 0.0f, 0.0f);
        }
    }
    record = &lbl_1_bss_A8[5];
    if ((record->value.countdown -= 1.0f) < 0.0f) {
        fn_1_6848(1);
        Hu3DMotionSet(obj->mdlId[5], obj->mtnId[5]);
        record->value.countdown = 1080.0f + Hu3DMotionTimeGet(obj->mdlId[5]);
    }
    record = &lbl_1_bss_A8[6];
    if ((record->value.countdown -= 1.0f) < 0.0f) {
        if (rand8() % 2 == 0) {
            Hu3DMotionSet(obj->mdlId[6], obj->mtnId[6]);
        }
        record->value.countdown = 1080.0f + Hu3DMotionTimeGet(obj->mdlId[6]);
    }
    Hu3DModelPosGet(obj->mdlId[10], &position);
    Hu3DModelPosSetV(obj->mdlId[11], &position);
    Hu3DModelPosSetV(obj->mdlId[12], &position);
    Hu3DModelPosSetV(obj->mdlId[13], &position);
    Hu3DModelPosSetV(obj->mdlId[14], &position);
    record = &lbl_1_bss_A8[12];
    if ((record->interpolationTime += 1.0f) > record->interpolationDuration) {
        record->interpolationTime = 0.0f;
        record->interpolationDuration = frandmod(600) + 600;
        Hu3DMotionSet(obj->mdlId[12], obj->mtnId[12]);
    }
    record = &lbl_1_bss_A8[11];
    if (record->transitionTarget != (f32)lbl_1_bss_9D0[0] && 0.0f == record->transitionActive) {
        if (7.0f == record->transitionTarget && lbl_1_bss_9D0[0] == 0) {
            record->value.transitionStart = 7.0f;
            record->transitionTarget = 8.0f;
        } else if (0.0f == record->transitionTarget && lbl_1_bss_9D0[0] == 7) {
            record->value.transitionStart = 8.0f;
            record->transitionTarget = 7.0f;
        } else {
            record->value.transitionStart = record->transitionTarget;
            record->transitionTarget = lbl_1_bss_9D0[0];
        }
        record->interpolationTime = 0.0f;
        record->interpolationDuration = 5.0f;
        record->transitionActive = 1.0f;
    }
    value = fn_1_E3E8(9.8f * record->value.transitionStart, 9.8f * record->transitionTarget,
                      record->interpolationTime, record->interpolationDuration);
    if ((record->interpolationTime += 1.0f) > record->interpolationDuration) {
        record->transitionActive = 0.0f;
        record->transitionTarget = lbl_1_bss_9D0[0];
    }
    Hu3DMotionTimeSet(obj->mdlId[11], value);
}

/* Create the menu model set and attach the configured motions and texture animations. */
void fn_1_2590(OMOBJ *obj)
{
    s32 table[19][4] = {
        {DATANUM(DATA_mgmfree, 0), 1, 2, 64},
        {DATANUM(DATA_mgmfree, 1), 1, 4, 64},
        {DATANUM(DATA_mgmfree, 2), 1, 2, 64},
        {DATANUM(DATA_mgmfree, 3), 1, 2, 64},
        {DATANUM(DATA_mgmfree, 4), 1, 6, 64},
        {DATANUM(DATA_mgmfree, 5), 0, 6, 64},
        {DATANUM(DATA_mgmfree, 6), 0, 2, 64},
        {DATANUM(DATA_mgmfree, 7), 1, 2, 64},
        {DATANUM(DATA_mgmfree, 8), 1, 2, 64},
        {DATANUM(DATA_mgmfree, 9), 1, 2, 64},
        {DATANUM(DATA_mgmfree, 13), 1, 4, 64},
        {DATANUM(DATA_mgmfree, 14), 0, 4, 64},
        {DATANUM(DATA_mgmfree, 15), 0, 4, 64},
        {DATANUM(DATA_mgmfree, 41), 0, 4, 64},
        {DATANUM(DATA_mgmfree, 42), 0, 4, 64},
        {DATANUM(DATA_mgmfree, 37), 1, 4, 64},
        {DATANUM(DATA_mgmfree, 38), 0, 4, 64},
        {DATANUM(DATA_mgmfree, 39), 1, 4, 64},
        {DATANUM(DATA_mgmfree, 40), 0, 4, 64}
    };
    char *texture[7] = {
        "chara_renda1", "chara_renda2", "chara_renda3", "chara_renda4",
        "chara_renda0", "mg_shot", "chara_renda5"
    };
    s16 flag;
    s16 i;

    omSetStatBit(obj, 256);
    if (GWBankFlagGet(60) != 0) {
        flag = 1;
    } else {
        flag = 0;
    }
    for (i = 0; i < 15; i++) {
        if (i == 10) {
            if (flag == 0) {
                obj->mdlId[i] = Hu3DModelCreateData(table[10][0]);
            } else if (flag == 1) {
                obj->mdlId[i] = Hu3DModelCreateData(table[15][0]);
            } else {
                obj->mdlId[i] = Hu3DModelCreateData(table[17][0]);
            }
            obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        } else if (i == 11) {
            if (flag == 0) {
                obj->mdlId[i] = Hu3DModelCreateData(table[11][0]);
            } else if (flag == 1) {
                obj->mdlId[i] = Hu3DModelCreateData(table[16][0]);
            } else {
                obj->mdlId[i] = Hu3DModelCreateData(table[18][0]);
            }
            obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        } else {
            obj->mdlId[i] = Hu3DModelCreateData(table[i][0]);
            obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        }
        Hu3DModelLayerSet(obj->mdlId[i], table[i][2]);
        Hu3DModelCameraSet(obj->mdlId[i], table[i][3]);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i], 0.0f, 0.0f,
                           table[i][1] * HU3D_MOTATTR_LOOP);
    }
    for (i = 0; i < 6; i++) {
        lbl_1_bss_4B0[i] = Hu3DAnimCreate(lbl_1_bss_4DC[i], obj->mdlId[10], texture[i]);
    }
    lbl_1_bss_4B0[6] = Hu3DAnimCreate(lbl_1_bss_4DC[6], obj->mdlId[4], texture[6]);
    lbl_1_bss_A8[11].interpolationTime = lbl_1_bss_9D0[0] * 10;
    lbl_1_bss_A8[11].interpolationDuration = lbl_1_bss_9D0[0] * 10;
    Hu3DModelAttrSet(obj->mdlId[11], HU3D_MOTATTR_PAUSE);
    Hu3DMotionTimeSet(obj->mdlId[11], lbl_1_bss_A8[11].interpolationTime);
    Hu3DModelPosSet(obj->mdlId[4], -750.0f, 0.0f, 0.0f);
    fn_1_1400(2);
    obj->objFunc = fn_1_1C74;
}

extern s16 lbl_1_data_380[4];
/* The target SDK declares this conversion helper elsewhere; host builds need its prototype here. */
#if defined(__MWERKS__)

#else
void Hu3D3Dto2D(HuVecF *src, s16 cameraBit, HuVecF *dst);
#endif

void fn_1_2BF8(void)
{
}

/* Create four category character models and four linked markers, then set their camera scissors. */
void fn_1_2BFC(OMOBJ *obj)
{
    s16 *row = lbl_1_bss_9A0[0];
    f32 yOffsets[11] = { 0, 0, -15, 0, 0, -5, -30, 30, 10, 30, 30 };
    HuVecF pos;
    HuVecF screen;
    s16 modelNo;
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 4; i++, row += 6) {
        obj->mdlId[i] = CharModelCreate(row[4], CHAR_MODEL3);
        obj->mtnId[i] = CharMotionCreate(row[4], DATANUM(DATA_mariomot, 0));
        Hu3DModelPosSet(obj->mdlId[i], i * 125 - 336, 360 + yOffsets[row[4]], -100);
        Hu3DModelRotSet(obj->mdlId[i], 0, -30, 0);
        Hu3DModelLayerSet(obj->mdlId[i], 3);
        Hu3DModelCameraSet(obj->mdlId[i], 1 << (i + 1));
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i], 0, 0, HU3D_MOTATTR_LOOP);
    }
    for (i = 0, modelNo = 4; i < 4; i++, modelNo++) {
        if (i == 0) {
            obj->mdlId[modelNo] = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_mgmfree, 10), HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            obj->mdlId[modelNo] = Hu3DModelLink(obj->mdlId[4]);
        }
        Hu3DModelPosSet(obj->mdlId[modelNo], i * 9 - 24, 20, 0);
        Hu3DModelLayerSet(obj->mdlId[modelNo], 3);
        Hu3DModelCameraSet(obj->mdlId[modelNo], 1 << (i + 1));
    }
    for (i = 0; i < 4; i++) {
        Hu3DModelPosGet(obj->mdlId[i], &pos);
        pos.y = 360;
        Hu3D3Dto2D(&pos, 1 << (i + 1), &screen);
        screen.x *= 1.1111112f;
        Hu3DCameraScissorSet(1 << (i + 1), (u32)(screen.x - 32),
                             (u32)(screen.y - 96), 64, 64);
    }
    obj->objFunc = NULL;
}

/* Update each category camera scissor from its projected model position. */
void fn_1_3004(void)
{
    HuVecF pos;
    HuVecF screen;
    s16 i;
    OMOBJ *obj = lbl_1_bss_20;

    for (i = 0; i < 4; i++) {
        Hu3DModelPosGet(obj->mdlId[lbl_1_data_380[i]], &pos);
        pos.y = 360;
        Hu3D3Dto2D(&pos, 1 << (i + 1), &screen);
        screen.x *= 1.1111112f;
        Hu3DCameraScissorSet(1 << (i + 1), (u32)(screen.x - 32),
                             (u32)(screen.y - 96), 64, 64);
    }
}

/* Create the model and joint motion that display the minigame selection scene. */
void fn_1_3124(OMOBJ *obj)
{
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_mgmfree, 27), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[0] = Hu3DJointMotion(
        obj->mdlId[0], HuDataSelHeapReadNum(DATANUM(DATA_mgmfree, 28), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(obj->mdlId[0], -310.0f, 220.0f, -100.0f);
    Hu3DModelRotSet(obj->mdlId[0], 0.0f, -90.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 1.25f, 1.25f, 1.25f);
    Hu3DModelLayerSet(obj->mdlId[0], 3);
    Hu3DModelCameraSet(obj->mdlId[0], HU3D_CAM6);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    obj->mdlId[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_mgmfree, 11), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(obj->mdlId[1], 3);
    Hu3DModelCameraSet(obj->mdlId[1], HU3D_CAM6);
    obj->objFunc = NULL;
}

/* Release the selection-scene model and its motion when the object is removed. */
void fn_1_32D0(OMOBJ *obj)
{
    if (obj) {
        Hu3DMotionKill(obj->mtnId[0]);
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_8, obj);
    }
    obj = NULL;
}

extern u8 lbl_1_bss_C[16];

/* As the registered selection-display callback, position its models and update the selected-model
 * scissor. */
void fn_1_3330(OMOBJ *obj)
{
    Vec position;
    Vec screen;
    f32 x;
    f32 width;
    f32 y;
    f32 height;
    s16 i;

    Hu3DModelPosGet((*(OMOBJ **)lbl_1_bss_C)->mdlId[4], &position);
    for (i = 0; i < 3; i++) {
        if (i == 2) {
            Hu3DModelPosSet(obj->mdlId[i], 60.0f + position.x, 170.0f, 86.0f);
        } else {
            Hu3DModelPosSet(obj->mdlId[i], 40.0f + position.x, 170.0f, 86.0f);
        }
    }
    Hu3DModelPosSet(obj->mdlId[3], position.x, 0.0f, 0.0f);
    Hu3DModelPosGet((*(OMOBJ **)lbl_1_bss_C)->mdlId[4], &position);
    position.x = 1.1111112f * (20.0f + position.x);
    position.y = 300.0f;
    position.z = 150.0f;
    Hu3D3Dto2D(&position, 32, &screen);
    x = screen.x;
    width = 90.0f;
    if (x <= 0.0f) {
        x = 0.0f;
        width = 90.0f + screen.x;
        if (width <= 0.0f) {
            width = 1.0f;
        }
    }
    y = screen.y;
    height = 128.0f;
    if (y <= 0.0f) {
        y = 0.0f;
        height = 128.0f + screen.y;
        if (y <= 0.0f) {
            y = 1.0f;
        }
    }
    Hu3DCameraScissorSet(32, (u32)x, (u32)y, (u32)width, (u32)height);
}

void fn_1_3330(OMOBJ *obj);

/* Create the three paired model groups used by the menu's animated selection display. */
void fn_1_361C(OMOBJ *obj)
{
    s32 filePairs[3][2] = {
        {DATANUM(DATA_mgmfree, 31), DATANUM(DATA_mgmfree, 33)},
        {DATANUM(DATA_mgmfree, 31), DATANUM(DATA_mgmfree, 33)},
        {DATANUM(DATA_mgmfree, 34), DATANUM(DATA_mgmfree, 36)}
    };
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 3; i++) {
        obj->mdlId[i] =
            Hu3DModelCreate(HuDataSelHeapReadNum(filePairs[i][0], HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DJointMotion(
            obj->mdlId[i], HuDataSelHeapReadNum(filePairs[i][1], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelPosSet(obj->mdlId[i], 30.0f, 170.0f, 86.0f);
        Hu3DModelRotSet(obj->mdlId[i], 0.0f, 60.0f, 0.0f);
        Hu3DModelScaleSet(obj->mdlId[i], 0.8f, 0.8f, 0.8f);
        Hu3DModelLayerSet(obj->mdlId[i], 5);
        Hu3DModelCameraSet(obj->mdlId[i], HU3D_CAM5);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < 3; i++) {
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    Hu3DModelAttrReset(obj->mdlId[rand8() % 3], HU3D_ATTR_DISPOFF);
    obj->mdlId[3] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_mgmfree, 12), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(obj->mdlId[3], 4);
    Hu3DModelCameraSet(obj->mdlId[3], HU3D_CAM5);
    obj->objFunc = fn_1_3330;
}

extern OMOBJ *lbl_1_bss_24;
extern s16 lbl_1_bss_38;
extern f32 lbl_1_bss_48;
extern f32 lbl_1_bss_94;
extern f32 lbl_1_bss_98[4];
f32 fn_1_E3A0(f32 start, f32 end, f32 time);
f32 fn_1_E4D0(f32 start, f32 end, f32 time, f32 duration);
void fn_1_E62C(HUSPR_GROUPID groupId, s32 attr);

/* Release the paired models, motions, and object created for the selection display. */
void fn_1_3904(OMOBJ *obj)
{
    s16 i;
    if (obj) {
        for (i = 0; i < 3; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        Hu3DModelKill(obj->mdlId[3]);
        omDelObjEx(lbl_1_bss_8, obj);
    }
    obj = NULL;
}

void fn_1_39A0(void)
{
    lbl_1_bss_94 = 2.0f;
}

/* Pulse the option cursor scale on each callback while easing its active size toward the target. */
void fn_1_39BC(void)
{
    f32 scale;

    lbl_1_bss_94 = fn_1_E3A0(lbl_1_bss_94, 0.5f, 5.0f);
    scale = fn_1_E4D0(0.5f, 1.0f, lbl_1_bss_48, 30.0f);
    if ((lbl_1_bss_48 += 1.0f) > 30.0f) {
        lbl_1_bss_48 = 0.0f;
    }
    scale = fn_1_E3A0(lbl_1_bss_94, scale, 5.0f);
    HuSprScaleSet(lbl_1_bss_53E[7], 0, scale, scale);
}

/* Place the option cursor beside the current category when that category has a visible selector. */
void fn_1_3AE8(void)
{
    if (lbl_1_bss_9D0[0] == 1 || lbl_1_bss_9D0[0] == 4) {
        HuSprPosSet(lbl_1_bss_53E[7], 0, 125.0f, 110.0f);
    } else if (lbl_1_bss_9D0[0] == 2 || lbl_1_bss_38 == 1) {
        HuSprPosSet(lbl_1_bss_53E[7], 0, 200.0f, 110.0f);
    } else {
        return;
    }
    lbl_1_bss_94 = 2.0f;
     fn_1_E62C(lbl_1_bss_53E[7], HUSPR_ATTR_DISPOFF);
}

void fn_1_3BE4(void)
{
     fn_1_E5AC(lbl_1_bss_53E[7], HUSPR_ATTR_DISPOFF);
}

void fn_1_3C14(s16 index)
{
    lbl_1_bss_98[index] = 2.0f;
}

/* Ease the four option-marker scales toward their resting size during the selection update. */
void fn_1_3C3C(void)
{
    s16 i;
    for (i = 0; i < 4; i++) {
        lbl_1_bss_98[i] = fn_1_E3A0(lbl_1_bss_98[i], 1.0f, 5.0f);
        HuSprScaleSet(lbl_1_bss_53E[2], i, lbl_1_bss_98[i], lbl_1_bss_98[i]);
    }
}

/* Show one option marker at its category-specific position and expand it from its hidden state. */
void fn_1_3D08(s16 index)
{
    s16 position[4][2] = {{36, 110}, {364, 110}, {288, 394}, {288, 229}};
    OMOBJ *obj = lbl_1_bss_24;

    if (obj->work[index] == 0) {
        lbl_1_bss_98[index] = 2.0f;
        HuSprPosSet(lbl_1_bss_53E[2], index, position[index][0], position[index][1]);
        HuSprScaleSet(lbl_1_bss_53E[2], index, 4.0f, 4.0f);
        HuSprAttrReset(lbl_1_bss_53E[2], index, 4);
        obj->work[index] = 1;
    }
}

/* Hide one option marker and clear its active flag after a selection transition. */
void fn_1_3EA0(s16 index)
{
    OMOBJ *obj = lbl_1_bss_24;

    if (obj->work[index] == 1) {
        HuSprAttrSet(lbl_1_bss_53E[2], index, 4);
        obj->work[index] = 0;
    }
}

/* Set the model animation for the current category and its selected menu state. */
void fn_1_3F20(s16 mode)
{
    Vec position;
    s16 *row = lbl_1_bss_9A0[0];
    s16 i;

    if ((lbl_1_bss_9D0[0] == 4 || lbl_1_bss_38 == 2) && mode == 1) {
        Hu3DAnimAnimSet(lbl_1_bss_4B0[2], lbl_1_bss_554[8]);
        Hu3DAnimAnimSet(lbl_1_bss_4B0[3], lbl_1_bss_554[8]);
    } else {
        Hu3DAnimAnimSet(lbl_1_bss_4B0[1], lbl_1_bss_4DC[0]);
        Hu3DAnimAnimSet(lbl_1_bss_4B0[2], lbl_1_bss_4DC[0]);
        Hu3DAnimAnimSet(lbl_1_bss_4B0[3], lbl_1_bss_4DC[0]);
    }
    for (i = 0; i < 4; i++, row += 6) {
        Hu3DModelPosGet(lbl_1_bss_20->mdlId[i], &position);
        Hu3D3Dto2D(&position, 1, &position);
        HuSprGrpPosSet(lbl_1_bss_53E[i + 3], position.x, 65.0f);
        HuSprGrpScaleSet(lbl_1_bss_53E[i + 3], 0.8f, 0.8f);
         fn_1_E62C(lbl_1_bss_53E[i + 3], HUSPR_ATTR_DISPOFF);
        if ((lbl_1_bss_9D0[0] == 4 || lbl_1_bss_38 == 2) && mode == 1 && position.x > 200.0f) {
             fn_1_E5AC(lbl_1_bss_53E[i + 3], HUSPR_ATTR_DISPOFF);
        }
        if (row[2] != 0) {
            HuSprBankSet(lbl_1_bss_53E[i + 3], 0, (s16)(row[3] + 5));
        } else {
            HuSprBankSet(lbl_1_bss_53E[i + 3], 0, row[5]);
        }
    }
}

/* Hide the four category option groups during menu transitions. */
void fn_1_41D8(void)
{
    s16 i = 0;
    while (i < 4) {
         fn_1_E5AC(lbl_1_bss_53E[i + 3], HUSPR_ATTR_DISPOFF);
        i++;
    }
}

/* Advance the cursor and option-marker animations from the registered object callback. */
void fn_1_4238(OMOBJ *obj)
{
    fn_1_3C3C();
    fn_1_39BC();
}

/* Initialize the existing option-marker object and install its update callback. */
void fn_1_4414(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        fn_1_3EA0(i);
    }
    fn_1_3BE4();
    obj->objFunc = fn_1_4238;
}

/* Remove a menu object from the module's object manager when it is no longer active. */
void fn_1_44D4(OMOBJ *obj)
{
    if (obj) {
        omDelObjEx(lbl_1_bss_8, obj);
    }
    obj = NULL;
}

#if defined(__MWERKS__)
/* The target runtime provides sprintf; host builds use its standard-library declaration. */
int sprintf();
#else
#include <stdio.h>
#endif

extern HUWINID lbl_1_bss_84[8];
extern HUWINID lbl_1_bss_78;
extern char lbl_1_bss_64[10];
extern char lbl_1_bss_6E[10];
extern char lbl_1_data_426[];

/* Fill the eight minigame windows with names and put unlocked/total counts in the side info
 * window. */
void fn_1_451C(void)
{
    s16 i;
    s16 index = lbl_1_bss_9D0[0];
    s16 base = lbl_1_bss_9D0[1] * 2;

    for (i = 0; i < 8; i++) {
        if (lbl_1_bss_580[index][base + i][0] == -1) {
            HuWinHomeClear(lbl_1_bss_84[i]);
        } else if (lbl_1_bss_580[index][base + i][1] != 0) {
            if (lbl_1_bss_580[index][base + i][0] == 21 ||
                lbl_1_bss_580[index][base + i][0] == 22 ||
                lbl_1_bss_580[index][base + i][0] == 41) {
                HuWinMesColSet(lbl_1_bss_84[i], HUWIN_MESCOL_ORANGE);
            } else {
                HuWinMesColSet(lbl_1_bss_84[i], HUWIN_MESCOL_WHITE);
            }
            HuWinMesSet(lbl_1_bss_84[i], MgDataTbl[lbl_1_bss_580[index][base + i][0]].nameMes);
        } else {
            HuWinMesSet(lbl_1_bss_84[i], MESSNUM(MESS_MGM_FREE, 2));
        }
        HuWinMesSpeedSet(lbl_1_bss_84[i], 0);
    }
    sprintf(lbl_1_bss_6E, lbl_1_data_426, lbl_1_bss_980[lbl_1_bss_9D0[0]]);
    HuWinInsertMesSet(lbl_1_bss_78, MESSNUM_PTR(lbl_1_bss_6E), 0);
    sprintf(lbl_1_bss_64, lbl_1_data_426, lbl_1_bss_990[lbl_1_bss_9D0[0]]);
    HuWinInsertMesSet(lbl_1_bss_78, MESSNUM_PTR(lbl_1_bss_64), 1);
    HuWinMesSet(lbl_1_bss_78, MESSNUM(MESS_MGM_FREE, 3));
    HuWinMesSpeedSet(lbl_1_bss_78, 0);
}

extern u8 lbl_1_bss_28[8];
extern HUWINID lbl_1_bss_78;
extern HUWINID lbl_1_bss_84[8];
void fn_1_451C(void);
f32 fn_1_E264(f32 start, f32 end, f32 time, f32 duration);

/* Slide the minigame windows upward as the registered object callback advances its frame count. */
void fn_1_4860(OMOBJ *obj)
{
    f32 position;
    s16 i;

    position = fn_1_E3E8(1000.0f, 314.0f, (f32)obj->work[0], (f32)obj->work[1]);
    for (i = 0; i < 8; i++) {
        HuWinPosSet(lbl_1_bss_84[i], (i % 2) * 264 + 20,
                    ((position - 64.0f + (i / 2) * 32) - 4.0f) - 10.0f);
        HuWinDispOn(lbl_1_bss_84[i]);
    }
    HuWinPosSet(lbl_1_bss_78, 448.0f, (position - 64.0f + (i / 2) * 32) - 12.0f);
    HuWinDispOn(lbl_1_bss_78);
    HuSprGrpPosSet(lbl_1_bss_53E[0], 288.0f, position);
     fn_1_E62C(lbl_1_bss_53E[0], HUSPR_ATTR_DISPOFF);
    if (++obj->work[0] > obj->work[1]) {
        HuWinDispOn(lbl_1_bss_9DC[0]);
        HuWinAttrSet(lbl_1_bss_9DC[0], HUWIN_ATTR_ALIGN_CENTER);
        HuWinMesSet(lbl_1_bss_9DC[0], MESSNUM(MESS_SYS_GUIDE, 11));
        HuWinMesSpeedSet(lbl_1_bss_9DC[0], 0);
        if (lbl_1_data_388 != MESSNUM(MESS_SYS_GUIDE, 11)) {
            lbl_1_data_388 = -1;
        }
        obj->objFunc = NULL;
    }
}

/* Prepare the item-window transition and install its upward-sliding callback. */
void fn_1_4B58(void)
{
    OMOBJ *obj = *(OMOBJ **)&lbl_1_bss_28[0];

    fn_1_451C();
    obj->work[0] = 0;
    obj->work[1] = 30;
    obj->objFunc = fn_1_4860;
}

/* Slide the minigame and side windows down offscreen during the registered object callback. */
void fn_1_4ECC(OMOBJ *obj)
{
    f32 position;
    s16 i;

    position = fn_1_E264(314.0f, 1000.0f, (f32)obj->work[0], (f32)obj->work[1]);
    for (i = 0; i < 8; i++) {
        HuWinPosSet(lbl_1_bss_84[i], (i % 2) * 264 + 20,
                    ((position - 64.0f + (i / 2) * 32) - 4.0f) - 10.0f);
        HuWinDispOn(lbl_1_bss_84[i]);
    }
    HuWinPosSet(lbl_1_bss_78, 448.0f, (position - 64.0f + (i / 2) * 32) - 12.0f);
    HuWinDispOn(lbl_1_bss_78);
    HuSprGrpPosSet(lbl_1_bss_53E[0], 288.0f, position);
     fn_1_E62C(lbl_1_bss_53E[0], HUSPR_ATTR_DISPOFF);
    if (++obj->work[0] > obj->work[1]) {
        obj->objFunc = NULL;
    }
}

/* Hide the category cursor and install the callback that slides the item windows away. */
void fn_1_514C(void)
{
    OMOBJ *obj = *(OMOBJ **)&lbl_1_bss_28[0];

    obj->work[0] = 0;
    obj->work[1] = 30;
     fn_1_E5AC(lbl_1_bss_53E[1], HUSPR_ATTR_DISPOFF);
    HuWinDispOff(lbl_1_bss_9DC[0]);
    obj->objFunc = fn_1_4ECC;
}

/* Show the minigame list and either open or close its side information windows. */
void fn_1_51BC(s16 mode)
{
    s16 i;

    if (mode == 1) {
        fn_1_451C();
        for (i = 0; i < 8; i++) {
            HuWinPosSet(lbl_1_bss_84[i], (i % 2) * 264 + 20, (i / 2) * 32 + 236);
            HuWinDispOn(lbl_1_bss_84[i]);
        }
        HuWinPosSet(lbl_1_bss_78, 448.0f, (i / 2) * 32 + 238);
        HuWinDispOn(lbl_1_bss_78);
        HuSprGrpPosSet(lbl_1_bss_53E[0], 288.0f, 314.0f);
         fn_1_E62C(lbl_1_bss_53E[0], HUSPR_ATTR_DISPOFF);
        return;
    }
    for (i = 0; i < 8; i++) {
        HuWinDispOff(lbl_1_bss_84[i]);
    }
    HuWinDispOff(lbl_1_bss_78);
         fn_1_E5AC(lbl_1_bss_53E[0], HUSPR_ATTR_DISPOFF);
}

extern HUWINID lbl_1_bss_7A[3];
extern HUWINID lbl_1_bss_80[2];

/* Show the two item-detail windows and their sprite group before drawing item details. */
void fn_1_56E4(void)
{
    s16 index;

     fn_1_E62C(lbl_1_bss_53E[8], HUSPR_ATTR_DISPOFF);
    index = 0;
    while (index < 2) {
        HuWinDispOn(lbl_1_bss_80[index]);
        index++;
    }
}

/* Hide the two item-detail windows and their sprite group after the item prompt. */
void fn_1_5750(void)
{
    s16 index;

         fn_1_E5AC(lbl_1_bss_53E[8], HUSPR_ATTR_DISPOFF);
    index = 0;
    while (index < 2) {
        HuWinDispOff(lbl_1_bss_80[index]);
        index++;
    }
}

/* Show the three choice windows and their sprite group before reading a choice. */
void fn_1_57BC(void)
{
    s16 index;

     fn_1_E62C(lbl_1_bss_53E[9], HUSPR_ATTR_DISPOFF);
    index = 0;
    while (index < 3) {
        HuWinDispOn(lbl_1_bss_7A[index]);
        index++;
    }
}

/* Hide the three choice windows and their sprite group when choice input ends. */
void fn_1_5828(void)
{
    s16 index;

         fn_1_E5AC(lbl_1_bss_53E[9], HUSPR_ATTR_DISPOFF);
    index = 0;
    while (index < 3) {
        HuWinDispOff(lbl_1_bss_7A[index]);
        index++;
    }
}

extern OMOBJ *lbl_1_bss_1C;

extern HUWINID lbl_1_bss_7A[3];
extern HUWINID lbl_1_bss_80[2];

/* Create eight list windows, the side count/info window, two detail windows, and three choice
 * windows. */
void fn_1_5894(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 8; i++) {
        lbl_1_bss_84[i] = HuWinExCreateFrame((i % 2) * 264 + 20, (i / 2) * 32 + 246,
                                           300, 42, -1, 0);
        HuWinPriSet(lbl_1_bss_84[i], 8);
        HuWinBGTPLvlSet(lbl_1_bss_84[i], 0.0f);
    }
    lbl_1_bss_78 = HuWinExCreateFrame(448.0f, (i / 2) * 32 + 238, 300, 42, -1, 0);
    HuWinPriSet(lbl_1_bss_78, 8);
    HuWinBGTPLvlSet(lbl_1_bss_78, 0.0f);
    HuWinScaleSet(lbl_1_bss_78, 0.8f, 0.8f);
    fn_1_51BC(0);
    HuSprGrpPosSet(lbl_1_bss_53E[8], 288.0f, 240.0f);
    HuSprPosSet(lbl_1_bss_53E[8], 1, 0.0f, 18.0f);
    HuSprPosSet(lbl_1_bss_53E[8], 1, 0.0f, -18.0f);
    for (i = 0; i < 2; i++) {
        lbl_1_bss_80[i] = HuWinExCreateFrame(138.0f, i * 42 + 198, 300, 42, -1, 0);
        HuWinPriSet(lbl_1_bss_80[i], 4);
        HuWinBGTPLvlSet(lbl_1_bss_80[i], 0.0f);
        HuWinAttrSet(lbl_1_bss_80[i], HUWIN_ATTR_ALIGN_CENTER);
        HuWinMesSpeedSet(lbl_1_bss_80[i], 0);
        HuWinMesSet(lbl_1_bss_80[i], i + MESSNUM(MESS_MGM_FREE, 4));
    }
    HuSprGrpPosSet(lbl_1_bss_53E[9], 288.0f, 240.0f);
    HuSprPosSet(lbl_1_bss_53E[9], 1, 0.0f, 18.0f);
    HuSprPosSet(lbl_1_bss_53E[9], 1, 0.0f, -42.0f);
    for (i = 0; i < 3; i++) {
        lbl_1_bss_7A[i] = HuWinExCreateFrame(138.0f, i * 42 + 178, 300, 42, -1, 0);
        HuWinPriSet(lbl_1_bss_7A[i], 4);
        HuWinBGTPLvlSet(lbl_1_bss_7A[i], 0.0f);
        HuWinAttrSet(lbl_1_bss_7A[i], HUWIN_ATTR_ALIGN_CENTER);
        HuWinMesSpeedSet(lbl_1_bss_7A[i], 0);
        HuWinMesSet(lbl_1_bss_7A[i], i + MESSNUM(MESS_MGM_FREE, 6));
    }
    obj->objFunc = NULL;
}

/* Destroy the minigame-list and item-detail windows when their object is removed. */
void fn_1_5E4C(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 8; i++) {
            HuWinKill(lbl_1_bss_84[i]);
        }
        for (i = 0; i < 2; i++) {
            HuWinKill(lbl_1_bss_80[i]);
        }
        for (i = 0; i < 3; i++) {
            HuWinKill(lbl_1_bss_7A[i]);
        }
        HuWinKill(lbl_1_bss_78);
        omDelObjEx(lbl_1_bss_8, obj);
    }
    obj = NULL;
}

/* Animate the selection models and their motion from the current menu state each frame. */
void fn_1_5F48(OMOBJ *obj)
{
    Vec position;
    f32 step = 5.0f;
    s16 i;

    if (obj->work[2] == 1) {
        Hu3DModelPosGet(lbl_1_bss_1C->mdlId[0], &position);
        position.x -= step;
        Hu3DModelPosSetV(lbl_1_bss_1C->mdlId[0], &position);
        Hu3DModelPosGet(lbl_1_bss_1C->mdlId[1], &position);
        position.x -= step;
        Hu3DModelPosSetV(lbl_1_bss_1C->mdlId[1], &position);
        for (i = 0; i < 4; i++) {
            Hu3DModelPosGet(lbl_1_bss_20->mdlId[i], &position);
            position.x -= step;
            Hu3DModelPosSetV(lbl_1_bss_20->mdlId[i], &position);
            Hu3DModelPosGet(lbl_1_bss_20->mdlId[i + 4], &position);
            position.x -= step;
            Hu3DModelPosSetV(lbl_1_bss_20->mdlId[i + 4], &position);
        }
        Hu3DModelPosGet((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[10], &position);
        position.x -= step;
        if (position.x < 0.0f) {
            for (i = 0; i < 4; i++) {
                Hu3DModelPosGet(lbl_1_bss_20->mdlId[i], &position);
                position.x = i * 125 - 336;
                Hu3DModelPosSetV(lbl_1_bss_20->mdlId[i], &position);
            }
            obj->objFunc = NULL;
            Hu3DModelPosGet((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[10], &position);
            position.x = 0.0f;
        }
        Hu3DModelPosSetV((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[10], &position);
        Hu3DModelPosGet((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[10], &position);
        Hu3DModelPosSetV((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[11], &position);
        Hu3DModelPosSetV((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[12], &position);
        Hu3DModelPosSetV((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[13], &position);
        Hu3DModelPosSetV((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[14], &position);
    } else {
        Hu3DModelPosGet((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[10], &position);
        position.x -= step;
        if (position.x < -500.0f) {
            position.x = -500.0f;
            obj->objFunc = NULL;
        }
        Hu3DModelPosSetV((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[10], &position);
        Hu3DModelPosGet(lbl_1_bss_1C->mdlId[0], &position);
        position.x -= step;
        Hu3DModelPosSetV(lbl_1_bss_1C->mdlId[0], &position);
        Hu3DModelPosGet(lbl_1_bss_1C->mdlId[1], &position);
        position.x -= step;
        Hu3DModelPosSetV(lbl_1_bss_1C->mdlId[1], &position);
        for (i = 0; i < 4; i++) {
            Hu3DModelPosGet(lbl_1_bss_20->mdlId[i], &position);
            position.x -= step;
            Hu3DModelPosSetV(lbl_1_bss_20->mdlId[i], &position);
            Hu3DModelPosGet(lbl_1_bss_20->mdlId[i + 4], &position);
            position.x -= step;
            Hu3DModelPosSetV(lbl_1_bss_20->mdlId[i + 4], &position);
        }
        Hu3DModelPosGet((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[10], &position);
        Hu3DModelPosSetV((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[11], &position);
        Hu3DModelPosSetV((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[12], &position);
        Hu3DModelPosSetV((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[13], &position);
        Hu3DModelPosSetV((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[14], &position);
    }
    fn_1_3004();
}

extern OMOBJ *lbl_1_bss_34;
extern s16 lbl_1_bss_4C[12];
extern char lbl_1_data_42A[];

/* Start or reset the selection-scene model motion for the requested transition mode. */
void fn_1_65E0(s16 mode)
{
    Vec position;
    s16 i;
    OMOBJ *obj = lbl_1_bss_1C;

    obj->work[0] = 0;
    obj->work[2] = mode;
    if (mode != 0) {
        Hu3DModelPosSet((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[10], 200.0f, 0.0f, 0.0f);
        Hu3DModelPosGet(lbl_1_bss_1C->mdlId[0], &position);
        position.x += 200.0f;
        Hu3DModelPosSetV(lbl_1_bss_1C->mdlId[0], &position);
        Hu3DModelPosGet(lbl_1_bss_1C->mdlId[1], &position);
        position.x += 200.0f;
        Hu3DModelPosSetV(lbl_1_bss_1C->mdlId[1], &position);
        for (i = 0; i < 4; i++) {
            Hu3DModelPosGet(lbl_1_bss_20->mdlId[i], &position);
            position.x += 200.0f;
            Hu3DModelPosSetV(lbl_1_bss_20->mdlId[i], &position);
            Hu3DModelPosGet(lbl_1_bss_20->mdlId[i + 4], &position);
            position.x += 200.0f;
            Hu3DModelPosSetV(lbl_1_bss_20->mdlId[i + 4], &position);
        }
    } else {
        Hu3DModelPosSet((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[10], 0.0f, 0.0f, 0.0f);
    }
    obj->objFunc = fn_1_5F48;
}

/* Randomize visibility and rotation for one of the two six-model groups selected by mode. */
void fn_1_6848(s16 mode)
{
    OMOBJ *obj = lbl_1_bss_34;
    s16 i;

    if (mode == 0) {
        for (i = 0; i < 6; i++) {
            if (rand8() % 2 == 0) {
                Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
                Hu3DModelRotSet(obj->mdlId[i], 0.0f, rand8() - 128, 0.0f);
                lbl_1_bss_4C[i] = 1;
            } else {
                Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
                lbl_1_bss_4C[i] = 0;
            }
        }
        return;
    }
    for (i = 6; i < 12; i++) {
        if (rand8() % 2 == 0) {
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            Hu3DModelRotSet(obj->mdlId[i], 0.0f, rand8() + 52, 0.0f);
            lbl_1_bss_4C[i] = 1;
        } else {
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            lbl_1_bss_4C[i] = 0;
        }
    }
}

/* Place the six revealed models around the active selection model during its update callback. */
void fn_1_6A84(OMOBJ *obj)
{
    Vec position;
    Mtx matrix;
    s16 i;

    for (i = 0; i < 6; i++) {
        if (lbl_1_bss_4C[i] != 0) {
            Hu3DModelPosGet((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[4], &position);
            Hu3DModelPosSet(obj->mdlId[i], (position.x - 80.0f) - i * 20,
                             150.0f, position.z + (i % 3) * 40);
        }
    }
    for (i = 6; i < 12; i++) {
        Hu3DModelObjMtxGet((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[5], lbl_1_data_42A, matrix);
        position.x = matrix[0][3];
        position.y = matrix[1][3];
        position.z = matrix[2][3];
        Hu3DModelPosSet(obj->mdlId[i], position.x + (i - 6) * 40,
                         position.y, position.z + (i % 3) * 40);
    }
}

/* Create and animate the twelve models used by the menu's selection scene. */
void fn_1_6CB0(OMOBJ *obj)
{
    s32 filePairs[3][2] = {
        {DATANUM(DATA_mgmfree, 29), DATANUM(DATA_mgmfree, 30)},
        {DATANUM(DATA_mgmfree, 31), DATANUM(DATA_mgmfree, 32)},
        {DATANUM(DATA_mgmfree, 34), DATANUM(DATA_mgmfree, 35)}
    };
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 12; i++) {
        if (i <= 2) {
            obj->mdlId[i] = Hu3DModelCreate(
                HuDataSelHeapReadNum(filePairs[i % 3][0], HU_MEMNUM_OVL, HEAP_MODEL));
            obj->mtnId[i] =
                Hu3DJointMotion(obj->mdlId[i], HuDataSelHeapReadNum(filePairs[i % 3][1],
                                                                    HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            obj->mdlId[i] = Hu3DModelLink(obj->mdlId[i % 3]);
        }
        Hu3DModelScaleSet(obj->mdlId[i], 0.8f, 0.8f, 0.8f);
        Hu3DModelLayerSet(obj->mdlId[i], 5);
        Hu3DModelCameraSet(obj->mdlId[i], HU3D_CAM6);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i % 3], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        lbl_1_bss_4C[i] = 0;
    }
    obj->objFunc = fn_1_6A84;
}

void fn_1_6F44(void)
{
}

/* Release menu objects, sprites, windows, movie playback, and scene resources on exit. */
void fn_1_6F48(void)
{
    fn_1_44D4(lbl_1_bss_24);
    fn_1_5E4C(*(OMOBJ **)&lbl_1_bss_28[0]);
    fn_1_171C();
    fn_1_11C0();
    fn_1_D48();
    fn_1_8D0();
    fn_1_758();
}

/* Initialized strings and halfwords following the menu data. */
char lbl_1_data_426[] = "%2d";
char lbl_1_data_42A[] = "free_play-null_npc1";
char lbl_1_data_43E[] = "minigame index -1\n";
char lbl_1_data_451[] = "minigame number error!!\n";
char lbl_1_data_46A[] = "\n\nIDX ======= %d,%d,%d,%d\n";
char lbl_1_data_485[] = "GRP ======= %d,%d,%d,%d\n\n";
char lbl_1_data_49F[] = "\n-----===== MARIO PARTY 6 :: MINIGAME FREE PLAY =====-----\n\n";
s16 lbl_1_data_4DC = -1;
s16 lbl_1_data_4DE = -1;

#include <game/pad.h>
#include <game/wipe.h>

extern u32 lbl_1_bss_4;
extern s32 lbl_1_bss_9D8;
extern f32 lbl_1_bss_4F8[4];
extern f32 lbl_1_bss_508[4];

extern BOOL MgPauseExitF;
extern s32 omovlevtno;
extern s32 omovlstat;
void OSReport(const char *message, ...);
s32 fn_1_E0C0(s16 mode);

typedef void (*MGMFREE_VOID_FUNC)(void);
extern const MGMFREE_VOID_FUNC _ctors[];
extern const MGMFREE_VOID_FUNC _dtors[];

/* Unlock the selected minigame and update the system's current minigame number. */
static inline void inline_0(int id)
{
    GWMgUnlockSet(id);
    GwSystem.mgNo = id - GW_MGNO_BASE;
}

/* Run the free-play mode process, dispatching its category and minigame selection flow. */
void fn_1_717C(void)
{
    s32 result;
    s16 category;
    s16 index;
    s16 mgNo;
    OMOVLHIS *history;

    result = 0;
    result = fn_1_E0C0((s16)lbl_1_bss_0);
    if ((s16)result == 0) {
        fn_1_65E0(0);
    }
    HuAudSStreamFadeOut(lbl_1_bss_9D8, 1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    WipeWait();
    category = lbl_1_bss_9D0[0];
    index = (lbl_1_bss_9D0[2] + 2 * lbl_1_bss_9D0[1]) + 2 * lbl_1_bss_9D0[3];
    fn_1_6F48();
    switch ((s16)result) {
    case 0:
        if (lbl_1_bss_580[category][index][0] == -1) {
            OSReport(lbl_1_data_43E);
            omOvlReturnEx(1, 1);
        } else {
            mgNo = MgNoGet(MgDataTbl[lbl_1_bss_580[category][index][0]].ovl);
            if (mgNo == -1) {
                OSReport(lbl_1_data_451);
                omOvlReturnEx(1, 1);
            } else {
                history = omOvlHisGet(0);
                omOvlHisChg(0, history->ovl, category + 1, index);
                OSReport(lbl_1_data_46A, lbl_1_data_380[0], lbl_1_data_380[1],
                         lbl_1_data_380[2], lbl_1_data_380[3]);
                if (lbl_1_bss_9D0[0] == 1) {
                    GwPlayerConf[lbl_1_data_380[0]].grpNo = 0;
                    GwPlayerConf[lbl_1_data_380[1]].grpNo = 1;
                    GwPlayerConf[lbl_1_data_380[2]].grpNo = 1;
                    GwPlayerConf[lbl_1_data_380[3]].grpNo = 1;
                    OSReport(lbl_1_data_485, 0, 1, 1, 1);
                } else if (lbl_1_bss_9D0[0] == 2 || lbl_1_bss_38 == 1) {
                    GwPlayerConf[lbl_1_data_380[0]].grpNo = 0;
                    GwPlayerConf[lbl_1_data_380[1]].grpNo = 0;
                    GwPlayerConf[lbl_1_data_380[2]].grpNo = 1;
                    GwPlayerConf[lbl_1_data_380[3]].grpNo = 1;
                    OSReport(lbl_1_data_485, 0, 0, 1, 1);
                } else if (lbl_1_bss_9D0[0] == 4) {
                    GwPlayerConf[lbl_1_data_380[0]].grpNo = 0;
                    GwPlayerConf[lbl_1_data_380[1]].grpNo = 1;
                    GwPlayerConf[lbl_1_data_380[2]].grpNo = 2;
                    GwPlayerConf[lbl_1_data_380[3]].grpNo = 2;
                    OSReport(lbl_1_data_485, 0, 1, 2, 2);
                } else if (lbl_1_bss_9D0[0] == 7 && index == 1) {
                    if (lbl_1_bss_38 == 2) {
                        GwPlayerConf[lbl_1_data_380[0]].grpNo = 0;
                        GwPlayerConf[lbl_1_data_380[1]].grpNo = 0;
                        GwPlayerConf[lbl_1_data_380[2]].grpNo = 1;
                        GwPlayerConf[lbl_1_data_380[3]].grpNo = 1;
                        OSReport(lbl_1_data_485, 0, 0, 1, 1);
                    } else {
                        GwPlayerConf[lbl_1_data_380[0]].grpNo = 0;
                        GwPlayerConf[lbl_1_data_380[1]].grpNo = 1;
                        GwPlayerConf[lbl_1_data_380[2]].grpNo = 1;
                        GwPlayerConf[lbl_1_data_380[3]].grpNo = 1;
                        OSReport(lbl_1_data_485, 0, 1, 1, 1);
                    }
                } else {
                    GwPlayerConf[0].grpNo = 0;
                    GwPlayerConf[1].grpNo = 1;
                    GwPlayerConf[2].grpNo = 2;
                    GwPlayerConf[3].grpNo = 3;
                    OSReport(lbl_1_data_485, 0, 1, 2, 3);
                }
                MgInstExitF = TRUE;
                _SetFlag(FLAGNUM(FLAG_GROUP_SYSTEM, 4));
                _SetFlag(FLAG_INST_MG_MODE);
                GwMgNightF = 0;
                GWMgInstDispSet(TRUE);
                inline_0(mgNo + GW_MGNO_BASE);
                omOvlCallEx(DLL_instdll, 1, 0, 0);
            }
        }
        break;
    case 1:
        omOvlReturnEx(1, 1);
        break;
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Initialize MGMfree's object manager, scene, windows, sprites, and input callbacks. */
void fn_1_7AE8(void)
{
    lbl_1_bss_8 = omInitObjMan(27, 8192);
    omGameSysInit(lbl_1_bss_8);
    fn_1_624();
    fn_1_780();
    fn_1_BD4();
    fn_1_DA4();
    fn_1_1790();
    fn_1_1A4();
    *(OMOBJ **)lbl_1_bss_C = omAddObjEx(lbl_1_bss_8, 4096, 16, 16, -1, fn_1_2590);
    lbl_1_bss_20 = omAddObjEx(lbl_1_bss_8, 4096, 16, 16, -1, fn_1_2BFC);
    lbl_1_bss_1C = omAddObjEx(lbl_1_bss_8, 4096, 16, 16, -1, fn_1_3124);
    lbl_1_bss_30 = omAddObjEx(lbl_1_bss_8, 4096, 4, 4, -1, fn_1_361C);
    lbl_1_bss_24 = omAddObjEx(lbl_1_bss_8, 4096, 0, 0, -1, fn_1_4414);
    *(OMOBJ **)lbl_1_bss_28 = omAddObjEx(lbl_1_bss_8, 4096, 0, 0, -1, fn_1_5894);
    lbl_1_bss_34 = omAddObjEx(lbl_1_bss_8, 4096, 16, 16, -1, fn_1_6CB0);
    HuPrcChildCreate(fn_1_717C, 12288, 12288, 0, lbl_1_bss_8);
}

/* Enter MGMfree from the minigame event and save the event state for the return path. */
void fn_1_8548(void)
{
    OSReport(lbl_1_data_49F);
    MgPauseExitF = 1;
    lbl_1_bss_0 = omovlevtno;
    lbl_1_bss_4 = omovlstat;
    fn_1_7AE8();
}

/* Run the module startup hook before free-play objects are created. */
int _prolog(void)
{
    const MGMFREE_VOID_FUNC *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_8548();
    return 0;
}

/* Run the module shutdown hook after free-play objects have been removed. */
void _epilog(void)
{
    const MGMFREE_VOID_FUNC *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Show the option markers available for the current category and remaining item count. */
void fn_1_8658(void)
{
    s16 count = lbl_1_bss_990[lbl_1_bss_9D0[0]];

    if (count % 2 == 1) {
        count++;
    }
    count /= 2;
    fn_1_3D08(2);
    fn_1_3D08(3);
    if (count <= 4) {
        fn_1_3EA0(2);
        fn_1_3EA0(3);
    }
    if (lbl_1_bss_9D0[1] == 0) {
        fn_1_3EA0(3);
    }
    if (lbl_1_bss_9D0[1] >= count - 4) {
        fn_1_3EA0(2);
    }
     fn_1_E62C(lbl_1_bss_53E[1], HUSPR_ATTR_DISPOFF);
    HuSprPosSet(lbl_1_bss_53E[1], 0,
        lbl_1_bss_9D0[2] * 264 + 153, (lbl_1_bss_9D0[3] << 5) + 256);
}

void fn_1_8ADC(void)
{
    fn_1_3EA0(2);
    fn_1_3EA0(3);
}

/* Handle category, page, and item navigation, returning the resulting change code. */
s16 fn_1_8B78(void)
{
    s16 change = 0;
    s16 index = 0;
    s16 count = lbl_1_bss_990[lbl_1_bss_9D0[0]];

    if (count % 2 == 1) {
        count++;
    }
    count /= 2;
    lbl_1_data_4DC = lbl_1_bss_9D0[0];
    lbl_1_data_4DE = (lbl_1_bss_9D0[2] + 2 * lbl_1_bss_9D0[1]) + 2 * lbl_1_bss_9D0[3];
    if (HuPadBtn[0] & PAD_TRIGGER_L) {
        Hu3DMotionSet((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[13],
                      (*(OMOBJ **)&lbl_1_bss_C[0])->mtnId[13]);
        lbl_1_bss_9D0[0]--;
        if (lbl_1_bss_9D0[0] < 0) {
            lbl_1_bss_9D0[0] = 7;
        }
        lbl_1_bss_9D0[1] = 0;
        lbl_1_bss_9D0[2] = 0;
        lbl_1_bss_9D0[3] = 0;
        change = 1;
        fn_1_8658();
    } else if (HuPadBtn[0] & PAD_TRIGGER_R) {
        Hu3DMotionSet((*(OMOBJ **)&lbl_1_bss_C[0])->mdlId[14],
                      (*(OMOBJ **)&lbl_1_bss_C[0])->mtnId[14]);
        lbl_1_bss_9D0[0]++;
        if (lbl_1_bss_9D0[0] >= 8) {
            lbl_1_bss_9D0[0] = 0;
        }
        lbl_1_bss_9D0[1] = 0;
        lbl_1_bss_9D0[2] = 0;
        lbl_1_bss_9D0[3] = 0;
        change = 1;
        fn_1_8658();
    } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
        if (lbl_1_bss_9D0[2] == 0) {
            lbl_1_bss_9D0[2] = 1;
            change = 2;
        }
    } else if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
        if (lbl_1_bss_9D0[2] == 1) {
            lbl_1_bss_9D0[2] = 0;
            change = 2;
        }
    } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
        lbl_1_bss_9D0[3]++;
        change = 3;
        if (lbl_1_bss_9D0[3] > 3) {
            lbl_1_bss_9D0[3] = 3;
            if (count - 4 > 0) {
                lbl_1_bss_9D0[1]++;
                fn_1_3D08(3);
                fn_1_3C14(2);
                if (lbl_1_bss_9D0[1] >= count - 4) {
                    lbl_1_bss_9D0[1] = count - 4;
                    fn_1_3EA0(2);
                }
            }
        }
    } else if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
        lbl_1_bss_9D0[3]--;
        change = 3;
        if (lbl_1_bss_9D0[3] < 0) {
            lbl_1_bss_9D0[3] = 0;
            if (count - 4 > 0) {
                lbl_1_bss_9D0[1]--;
                fn_1_3D08(2);
                fn_1_3C14(3);
                if (lbl_1_bss_9D0[1] <= 0) {
                    lbl_1_bss_9D0[1] = 0;
                    fn_1_3EA0(3);
                }
            }
        }
    }
    if (change == 2) {
        index = (lbl_1_bss_9D0[2] + 2 * lbl_1_bss_9D0[1]) + 2 * lbl_1_bss_9D0[3];
        if (lbl_1_bss_580[lbl_1_bss_9D0[0]][index][0] == -1) {
            lbl_1_bss_9D0[3]--;
        }
        change = 1;
    } else if (change == 3) {
        index = (lbl_1_bss_9D0[2] + 2 * lbl_1_bss_9D0[1]) + 2 * lbl_1_bss_9D0[3];
        if (lbl_1_bss_580[lbl_1_bss_9D0[0]][index][0] == -1) {
            index = lbl_1_bss_9D0[1] * 2 + lbl_1_bss_9D0[3] * 2;
            if (lbl_1_bss_580[lbl_1_bss_9D0[0]][index][0] == -1) {
                lbl_1_bss_9D0[3]--;
            } else {
                lbl_1_bss_9D0[2] = 0;
                change = 1;
            }
        } else {
            change = 1;
        }
    }
    if (change == 1) {
        fn_1_451C();
    }
    HuSprPosSet(lbl_1_bss_53E[1], 0,
                lbl_1_bss_9D0[2] * 264 + 153, (lbl_1_bss_9D0[3] << 5) + 256);
    if (lbl_1_data_4DC != lbl_1_bss_9D0[0] ||
        lbl_1_data_4DE != (lbl_1_bss_9D0[2] + 2 * lbl_1_bss_9D0[1]) + 2 * lbl_1_bss_9D0[3]) {
        HuAudFXPlay(MSM_SE_CMN_01);
        fn_1_1400(1);
        change = 1;
    } else {
        change = 0;
    }
    return change;
}

/* Prepare the mode-specific scene and selection state before the free-play loop starts. */
s32 fn_1_A3FC(void)
{
    s16 count;

    HuPrcSleep(5);
    fn_1_65E0(1);
    if (lbl_1_bss_0 != 0) {
        lbl_1_bss_9D0[0] = lbl_1_bss_0 - 1;
        count = lbl_1_bss_990[lbl_1_bss_9D0[0]];
        if (count % 2 == 1) {
            count++;
        }
        count /= 2;
        lbl_1_bss_9D0[1] = lbl_1_bss_4 >> 1;
        if (lbl_1_bss_9D0[1] >= count - 4) {
            lbl_1_bss_9D0[1] = count - 4;
        }
        if (lbl_1_bss_9D0[1] <= 0) {
            lbl_1_bss_9D0[1] = 0;
        }
        lbl_1_bss_4 -= lbl_1_bss_9D0[1] * 2;
        lbl_1_bss_9D0[2] = lbl_1_bss_4 & 1;
        lbl_1_bss_9D0[3] = lbl_1_bss_4 >> 1;
        fn_1_4B58();
    }
    lbl_1_bss_9D8 = HuAudSStreamPlay(MSM_STREAM_MGMUS_2);
    HuAudFXPlay(MGMFREE_SE_MODE_MENU_ENTRY_FX);
    WipeCreate(1, 0, 60);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    return 1;
}

/* Display the introductory free-play prompt when entering the main menu mode. */
s32 fn_1_AB00(void)
{
    if (lbl_1_bss_0 == 0) {
        fn_1_928(1);
         fn_1_A08(1, MESSNUM(MESS_MGM_FREE, 0), 1);
        fn_1_AC4(1);
        fn_1_998(1);
        fn_1_4B58();
        HuPrcSleep(60);
    }
    fn_1_1400(1);
    return 1;
}

/* Display the confirmation prompt and return the selected action to the caller. */
s16 fn_1_B160(void)
{
    s16 result = 0;

    HuWinExOpen(lbl_1_bss_9DC[1]);
    HuWinAttrSet(lbl_1_bss_9DC[1], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_9DC[1], MESSNUM(MESS_MGM_FREE, 1));
    HuWinMesSpeedSet(lbl_1_bss_9DC[1], 1);
    if ((u32)(lbl_1_data_388 + -MESSNUM(MESS_MGM_FREE, 0)) != 1U) {
        lbl_1_data_388 = -1;
    }
    result = fn_1_B00(1, 2);
    if (result != 0) {
        HuWinExClose(lbl_1_bss_9DC[1]);
    }
    return result;
}

/* Wait for input on the two-choice prompt and return its accepted result. */
s16 fn_1_B278(void)
{
    s16 result = 0;
    s16 selection = 0;

    HuSprPosSet(lbl_1_bss_53E[8], 1, 0.0f, selection * 42 - 22);
    fn_1_56E4();
    for (;;) {
        HuPrcVSleep();
        if (HuPadBtnDown[0] & PAD_BUTTON_A) {
            HuAudFXPlay(MSM_SE_CMN_03);
            result = 1;
            break;
        } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
            HuAudFXPlay(MSM_SE_CMN_04);
            result = 0;
            break;
        } else {
            if (HuPadDStkRep[0] & 4) {
                if (selection == 0) {
                    HuAudFXPlay(MSM_SE_CMN_01);
                }
                if (selection == 0) {
                    selection = 1;
                }
            } else if (HuPadDStkRep[0] & 8) {
                if (selection == 1) {
                    HuAudFXPlay(MSM_SE_CMN_01);
                }
                if (selection == 1) {
                    selection = 0;
                }
            }
            HuSprPosSet(lbl_1_bss_53E[8], 1, 0.0f,
                        selection * 42 - 22);
        }
    }
    fn_1_5750();
    if (result == 1) {
        if (selection == 0) {
            result = 1;
        } else {
            result = 2;
        }
    }
    return result;
}

/* Wait for input on the three-choice prompt and return its accepted result. */
s16 fn_1_B4E8(void)
{
    s16 result = 0;
    s16 selection = 1;

    HuSprPosSet(lbl_1_bss_53E[9], 1, 0.0f, selection * 42 - 42);
    fn_1_57BC();
    for (;;) {
        HuPrcVSleep();
        if (HuPadBtnDown[0] & PAD_BUTTON_A) {
            HuAudFXPlay(MSM_SE_CMN_03);
            result = 1;
            break;
        } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
            HuAudFXPlay(MSM_SE_CMN_04);
            result = 0;
            break;
        } else {
            if (HuPadDStkRep[0] & 4) {
                if (selection != 2) {
                    HuAudFXPlay(MSM_SE_CMN_01);
                }
                selection++;
                if (selection >= 2) {
                    selection = 2;
                }
            } else if (HuPadDStkRep[0] & 8) {
                if (selection != 0) {
                    HuAudFXPlay(MSM_SE_CMN_01);
                }
                selection--;
                if (selection <= 0) {
                    selection = 0;
                }
            }
            HuSprPosSet(lbl_1_bss_53E[9], 1, 0.0f,
                        selection * 42 - 42);
        }
    }
    fn_1_5828();
    if (result == 1) {
        MgGameNo = selection + 1;
    }
    return result;
}

/* Run the main menu selection loop until the player confirms or cancels a choice. */
s16 fn_1_B75C(void)
{
    s16 result = 0;
    s16 index = 0;

    fn_1_3F20(0);
    fn_1_51BC(1);
    for (;;) {
        HuPrcVSleep();
        fn_1_8658();
        for (;;) {
            HuPrcVSleep();
            if (fn_1_8B78() != 0) {
                HuPrcSleep(10);
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                index = (lbl_1_bss_9D0[2] + 2 * lbl_1_bss_9D0[1]) + 2 * lbl_1_bss_9D0[3];
                if (lbl_1_bss_580[lbl_1_bss_9D0[0]][index][1] == 1) {
                    HuAudFXPlay(MSM_SE_CMN_03);
                    result = 1;
                    break;
                } else {
                    HuAudFXPlay(MSM_SE_CMN_05);
                }
            } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_8ADC();
        if (result == -1) {
            if (!fn_1_B160()) {
                result = -1;
                break;
            }
        } else {
            lbl_1_bss_38 = 0;
            if (lbl_1_bss_9D0[0] == 1 || lbl_1_bss_9D0[0] == 2 || lbl_1_bss_9D0[0] == 4) {
                result = 2;
            }
            index = (lbl_1_bss_9D0[2] + 2 * lbl_1_bss_9D0[1]) + 2 * lbl_1_bss_9D0[3];
            if (MgDataTbl[lbl_1_bss_580[lbl_1_bss_9D0[0]][index][0]].ovl == 87) {
                if ((result = fn_1_B4E8()) != 0) {
                    lbl_1_bss_38 = 1;
                    result = 2;
                } else {
                    continue;
                }
            }
            if (lbl_1_bss_9D0[0] == 7 && index == 1) {
                if (GWBankFlagGet(8) != 0) {
                    result = fn_1_B278();
                    if (result == 0) {
                        continue;
                    }
                    if (result == 2) {
                        lbl_1_bss_38 = 2;
                        result = 2;
                    } else {
                        fn_1_1A98(2);
                    }
                } else {
                    fn_1_1A98(2);
                }
            }
            break;
        }
    }
    return result;
}

/* Set the selected model's rotation or restore its normal pose for the active menu state. */
void fn_1_C870(s16 mode)
{
    s16 index;

    if (mode == 1) {
        index = 0;
        while (index < 4) {
            if ((lbl_1_bss_9D0[0] == 1 && index == 0) ||
                ((lbl_1_bss_9D0[0] == 2 || lbl_1_bss_38 == 1) && index <= 1) ||
                (lbl_1_bss_9D0[0] == 4 && index == 0)) {
                Hu3DModelRotSet(lbl_1_bss_20->mdlId[lbl_1_data_380[index]],
                    0.0f, 30.0f, 0.0f);
            } else if ((lbl_1_bss_9D0[0] == 4 || lbl_1_bss_38 == 2) && index >= 2) {
                Hu3DModelRotSet(lbl_1_bss_20->mdlId[lbl_1_data_380[index]],
                    0.0f, -210.0f, 0.0f);
            } else {
                Hu3DModelRotSet(lbl_1_bss_20->mdlId[lbl_1_data_380[index]],
                    0.0f, -30.0f, 0.0f);
            }
            index++;
        }
    } else {
        index = 0;
        while (index < 4) {
            Hu3DModelRotSet(lbl_1_bss_20->mdlId[index],
                0.0f, -30.0f, 0.0f);
            index++;
        }
    }
}

/* Apply the requested item order and animate the four category models into that order. */
void fn_1_CAD4(s16 *order, s16 mode)
{
    Vec position;
    s16 i;
    s16 frame;
    f32 x;

    for (i = 0; i < 4; i++) {
        lbl_1_data_380[i] = order[i];
    }
    for (i = 0; i < 4; i++) {
        Hu3DModelPosGet(lbl_1_bss_20->mdlId[lbl_1_data_380[i]], &position);
        lbl_1_bss_508[lbl_1_data_380[i]] = position.x;
        lbl_1_bss_4F8[lbl_1_data_380[i]] = i * 125 - 336;
    }
    for (frame = 0; frame <= 5; frame++) {
        for (i = 0; i < 4; i++) {
            x = fn_1_E3E8(lbl_1_bss_508[i], lbl_1_bss_4F8[i], frame, 5.0f);
            Hu3DModelPosGet(lbl_1_bss_20->mdlId[i], &position);
            Hu3DModelPosSet(lbl_1_bss_20->mdlId[i], x, position.y, -100.0f);
        }
        HuPrcVSleep();
    }
    for (i = 0; i < 4; i++) {
        Hu3DModelCameraSet(lbl_1_bss_20->mdlId[lbl_1_data_380[i]], 1 << (i + 1));
    }
    if (mode == 1) {
        fn_1_C870(1);
    } else {
        fn_1_C870(0);
    }
    for (frame = 0; frame <= 5; frame++) {
        for (i = 0; i < 4; i++) {
            x = fn_1_E3E8(lbl_1_bss_508[i], lbl_1_bss_4F8[i], frame, 5.0f);
            Hu3DModelPosGet(lbl_1_bss_20->mdlId[i], &position);
            Hu3DModelPosSet(lbl_1_bss_20->mdlId[i], x, position.y, -100.0f);
        }
        HuPrcVSleep();
    }
    fn_1_3F20(1);
}

extern s16 lbl_1_data_2B0[4];
extern u8 lbl_1_data_2B8[200];

/* Process player input for the current category and item page, returning a menu action. */
s16 fn_1_D324(void)
{
    s16 rowNo = 0;
    s16 selection = 0;
    s16 result = 0;

    if (lbl_1_bss_9D0[0] == 1) {
        rowNo = 0;
    } else if (lbl_1_bss_9D0[0] == 2 || lbl_1_bss_38 == 1) {
        rowNo = 1;
    } else if (lbl_1_bss_9D0[0] == 4) {
        rowNo = 2;
    } else if (lbl_1_bss_38 == 2) {
        rowNo = 3;
    }
    fn_1_3F20(1);
restart:
    HuPrcVSleep();
    fn_1_3D08(0);
    fn_1_3D08(1);
    fn_1_C870(1);
    fn_1_3AE8();
    for (;;) {
        HuPrcVSleep();
        if (HuPadDStkRep[0] & 1) {
            HuAudFXPlay(MSM_SE_CMN_01);
            selection--;
            if (selection < 0) {
                selection = lbl_1_data_2B0[rowNo] - 1;
            }
            fn_1_39A0();
            fn_1_3C14(0);
            fn_1_CAD4((s16 *)(lbl_1_data_2B8 + rowNo * 48 + selection * 8), 1);
            continue;
        }
        if (HuPadDStkRep[0] & 2) {
            HuAudFXPlay(MSM_SE_CMN_01);
            selection++;
            if (selection >= lbl_1_data_2B0[rowNo]) {
                selection = 0;
            }
            fn_1_39A0();
            fn_1_3C14(1);
            fn_1_CAD4((s16 *)(lbl_1_data_2B8 + rowNo * 48 + selection * 8), 1);
            continue;
        }
        if (HuPadBtnDown[0] & PAD_BUTTON_A) {
            HuAudFXPlay(MSM_SE_CMN_03);
            result = 1;
        } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
            HuAudFXPlay(MSM_SE_CMN_04);
            result = 0;
        } else if (HuPadBtnDown[0] & PAD_BUTTON_X) {
            HuAudFXPlay(MSM_SE_CMN_04);
            result = -1;
        } else {
            continue;
        }
        break;
    }
    fn_1_3EA0(0);
    fn_1_3EA0(1);
    if (result == -1 || result == 0) {
        fn_1_3BE4();
    }
    if (result == 0) {
        fn_1_CAD4((s16 *)(lbl_1_data_2B8 + 32), 0);
        fn_1_3F20(0);
    }
    if (result == -1) {
        if (!fn_1_B160()) {
            result = -1;
        } else {
            goto restart;
        }
    }
    return result;
}
