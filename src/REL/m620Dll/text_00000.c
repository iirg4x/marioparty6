/* Trap Ease Artist startup, sequence callbacks, camera state, and module data. */
#include "dolphin/math.h"
#include "game/flag.h"
#include "game/memory.h"
#include "datadir_enum.h"
#include "msm_stream.h"

static const f64 m620_rodata_leading_constants[2] = {0.5, 3.0};

#define M620_VIEW_CENTER_X_BIT_PATTERN 0xC3EC0000
#define M620_VIEW_CENTER_Y_BIT_PATTERN 0xC4228000
#define M620_VIEW_CENTER_Z_BIT_PATTERN 0xC4570000
#define M620_VIEW_ROTATION_X_BIT_PATTERN 0xC2360000
#define M620_VIEW_ROTATION_Y_BIT_PATTERN 0x41680000
#define M620_VIEW_ROTATION_Z_BIT_PATTERN 0x00000000
#define M620_VIEW_ZOOM_BIT_PATTERN 0x44FA0000
#define M620_DATA_WORD_MASK 0x0000FFFF
#define M620_DATA_WORD_450FC000_BIT_PATTERN 0x450FC000
#define M620_DATA_WORD_44D48000_BIT_PATTERN 0x44D48000
#define M620_DATA_WORD_44960000_BIT_PATTERN 0x44960000
#define M620_SEQUENCE_START_SOUND_ID 1772
#define M620_SCORE_UPDATE_SOUND_ID 1774

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

/* Run the registered constructors and initialize Trap Ease Artist as the REL loads. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

/* Run registered destructors when the Trap Ease Artist REL is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

#include "dolphin/types.h"
#include "game/object.h"
#include "REL/m620Dll/module_globals.h"
#include "game/data.h"
#define MgSeqWinnerSet m620_seqman_MgSeqWinnerSet
#include "game/mg/seqman.h"
#undef MgSeqWinnerSet
s16 MgSeqWinnerSet(s16, s16, s16, s16);
#include "game/mg/timer.h"
#include "game/hu3d.h"
#include "REL/m620Dll/module_data.h"
extern s32 lbl_1_bss_144;
extern s16 lbl_1_bss_170[4];
extern void fn_1_600(s16, s16);
extern void fn_1_117C(OMOBJMAN *);
extern void fn_1_1244(OMOBJMAN *, void *, void *, void *);
extern void fn_1_18C8(void);
extern void fn_1_13A4(void);
extern void fn_1_192C(OMOBJMAN *, u32, f32, f32, f32, f32, OM_CAMERA_VIEW *);
extern s32 fn_1_FB8(s32);
extern void fn_1_26F8(s32, s32);
extern void fn_1_BF0(OMOBJ *, s32, s32);
extern void fn_1_24F4(void (*)(void));
extern void fn_1_2D7C(OMOBJMAN *, void *);
extern void fn_1_ECB4(OMOBJMAN *);
extern void fn_1_F0F4(OMOBJMAN *);
extern void fn_1_4A50(OMOBJMAN *);
extern void fn_1_64A8(OMOBJMAN *);
extern void fn_1_B7D4(OMOBJMAN *);

/* Create the minigame manager, sequence hooks, camera, and actor setup from _prolog. */
void fn_1_A0(void)
{
    s32 index;

    lbl_1_bss_68 = 0;
    lbl_1_bss_54 = 0;
    lbl_1_bss_5C = 0;
    lbl_1_bss_58 = 0;
    lbl_1_bss_50 = 0;
    lbl_1_bss_4C = -1;
    lbl_1_bss_48 = -1;
    lbl_1_bss_44 = 0;
    lbl_1_bss_20 = 0.0f;
    for (index = 0; index < 4; index++) {
        lbl_1_bss_3C[index] = 0;
    }
    lbl_1_bss_6C = omInitObjMan(100, 8192);
    omGameSysInit(lbl_1_bss_6C);
    MgSeqCreate(&lbl_1_data_80);
    MgSeqModeHookAdd(4, fn_1_600);
    fn_1_117C(lbl_1_bss_6C);
    fn_1_1244(lbl_1_bss_6C, &lbl_1_data_1C, &lbl_1_data_28, &lbl_1_data_34);
    fn_1_13A4();
    fn_1_192C(lbl_1_bss_6C, 1, 45.0f, 20.0f, 5000.0f,
              0.0f, (OM_CAMERA_VIEW *)&lbl_1_data_0);
    fn_1_26F8(1, (s32)HuDataSelHeapReadNum(fn_1_FB8(0), HU_MEMNUM_OVL, HEAP_MODEL));
    fn_1_24F4((void (*)(void))fn_1_BF0);
    fn_1_2D7C(lbl_1_bss_6C, lbl_1_data_38);
    fn_1_ECB4(lbl_1_bss_6C);
    fn_1_F0F4(lbl_1_bss_6C);
    fn_1_4A50(lbl_1_bss_6C);
    fn_1_64A8(lbl_1_bss_6C);
    fn_1_B7D4(lbl_1_bss_6C);
}

extern s32 fn_1_B1C(s16, s16);

/* Request sequence advancement from the init hook; the shared check always returns zero. */
void fn_1_2E8(s16 mode, s16 state)
{
    if (!fn_1_B1C(mode, state)) {
        MgSeqModeNext();
    }
}

#include "game/gamework.h"

s32 fn_1_B1C(s16, s16);
s32 HuAudBGMPlay(s32);
s32 HuAudFXPlay(s32);
void HuAudFXFadeOut(s32, s32);
void WipeCreate(s32, s32, s32);
u8 WipeCheck(void);

/* Run the fade-in hook: start the music, wait for the opening camera, and perform both wipes. */
void fn_1_324(s16 checkValue, s16 frameNo)
{
    if (fn_1_B1C(checkValue, frameNo) != 0) {
        return;
    }

    if (frameNo == 0) {
        lbl_1_bss_64 = 0;
    }

    if (lbl_1_bss_4C == -1 && frameNo >= 18) {
        lbl_1_bss_4C = HuAudBGMPlay(MSM_STREAM_MGMUS_25);
        lbl_1_bss_48 = HuAudFXPlay(M620_SEQUENCE_START_SOUND_ID);
    }

    switch (lbl_1_bss_64) {
    case 0:
        if (lbl_1_bss_5C == 0) {
            break;
        }
        lbl_1_bss_64++;
        lbl_1_bss_60 = 0;
        MgSeqModeChangeOff();
        /* fall through */
    case 1:
        if (++lbl_1_bss_60 < 6) {
            break;
        }
        lbl_1_bss_64++;
        WipeCreate(2, 6, 48);
        /* Fall through to check the wipe immediately and on subsequent callback ticks. */
    case 2:
        if (WipeCheck() == 0) {
            lbl_1_bss_64++;
            lbl_1_bss_60 = 0;
            lbl_1_bss_58 = 1;
        }
        break;
    case 3:
        if (++lbl_1_bss_60 < 12) {
            break;
        }
        WipeCreate(1, 6, 30);
        lbl_1_bss_44 = 1;
        lbl_1_bss_64++;
        /* Fall through to check the final wipe immediately and on subsequent callback ticks. */
    case 4:
        if (WipeCheck() == 0) {
            lbl_1_bss_64++;
            MgSeqModeNext();
        }
        break;
    case 5:
        break;
    default:
        break;
    }

    if (lbl_1_bss_44 != 0 && lbl_1_bss_48 != -1) {
        HuAudFXFadeOut(lbl_1_bss_48, 1500);
        lbl_1_bss_48 = -1;
    }
}

/* Keep the sequence callback slot valid; this phase has no additional work. */
void fn_1_5C8(s16 mode, s16 state)
{
    if (fn_1_B1C(mode, state) != 0) {
        return;
    }
}

extern s32 fn_1_B4D8(void);

/* Phase-four sequence hook; it currently performs no work after the phase check. */
void fn_1_600(s16 mode, s16 state)
{
    if (fn_1_B1C(mode, state) != 0) {
        return;
    }
}

/* Advance the main sequence once all four launch props are ready for selection. */
void fn_1_638(s16 mode, s16 state)
{
    if (!fn_1_B1C(mode, state)) {
        if (fn_1_B4D8() < 4) {
        } else {
            MgSeqModeNext();
            return;
        }
    }
}

extern void HuAudSStreamFadeOut(s32, s32);

/* Fade out the minigame music on the first frame of the finish phase. */
void fn_1_684(s16 checkValue, s16 frameNo)
{
    if (fn_1_B1C(checkValue, frameNo) == 0 && frameNo == 0) {
        if (lbl_1_bss_4C != -1) {
            HuAudSStreamFadeOut(lbl_1_bss_4C, 100);
            lbl_1_bss_4C = -1;
        }
    }
}

s32 fn_1_F0B4(s32);
void fn_1_F070(void);
s32 fn_1_B7C4(void);

/* Select every nonzero top scorer and release the sequence after the result display. */
void fn_1_708(s16 checkValue, s16 frameNo)
{
    s32 winnerScores[4];
    s32 maximumScore;
    s32 playerIndex;
    s32 score;
    s16 winnerResult;

    maximumScore = 0;
    if (fn_1_B1C(checkValue, frameNo) != 0) {
        return;
    }

    if (frameNo == 0) {
        lbl_1_bss_64 = 0;
    }

    switch (lbl_1_bss_64) {
    case 0:
        lbl_1_bss_64++;
        lbl_1_bss_60 = 0;
        MgSeqModeChangeOff();
    case 1:
        lbl_1_bss_64++;
        lbl_1_bss_60 = 0;
        fn_1_F070();
    case 2:
        if (lbl_1_bss_60 >= 48) {
            lbl_1_bss_64++;
            lbl_1_bss_60 = 0;
            lbl_1_bss_54 = 1;
        } else {
            break;
        }
    case 3:
        if (fn_1_B7C4() == 0) {
            break;
        }
        lbl_1_bss_64++;
        lbl_1_bss_60 = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            score = fn_1_F0B4(playerIndex);
            if (score > maximumScore) {
                maximumScore = score;
            }
            winnerScores[playerIndex] = score;
        }
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            lbl_1_bss_34[playerIndex] = -1;
            if (maximumScore == winnerScores[playerIndex] && maximumScore != 0) {
                lbl_1_bss_34[playerIndex] = lbl_1_bss_24[playerIndex];
            }
        }
        /* The sequence manager stores the winners; its updated status is unused here. */
        winnerResult = MgSeqWinnerSet(
            lbl_1_bss_34[0], lbl_1_bss_34[1], lbl_1_bss_34[2], lbl_1_bss_34[3]);
    case 4:
        if (lbl_1_bss_60 >= 30) {
            lbl_1_bss_64++;
            lbl_1_bss_60 = 0;
            MgSeqModeChangeOn();
        }
        break;
    default:
        break;
    }

    lbl_1_bss_60++;
}

/* Award ten bonus coins to winning players outside practice mode. */
void fn_1_9E0(s16 checkValue, s16 frameNo)
{
    s16 newBonus;
    s16 oldBonus;
    s32 playerIndex;

    if (fn_1_B1C(checkValue, frameNo) == 0 && frameNo == 0) {
        for (playerIndex = 0; playerIndex < GW_PLAYER_MAX; playerIndex++) {
            if (lbl_1_bss_34[playerIndex] >= 0) {
                oldBonus = GwPlayer[playerIndex].mgCoinBonus;
                newBonus = oldBonus + 10;
                if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                    GwPlayer[playerIndex].mgCoinBonus = newBonus;
                }
            }
        }
    }
}

extern void fn_1_65C0(void);
extern void fn_1_4B4C(void);
extern void fn_1_F17C(void);
extern void fn_1_ED3C(void);

/* No-op sequence hook after the shared phase check. */
void fn_1_AB8(s16 mode, s16 state)
{
    if (fn_1_B1C(mode, state) != 0) {
        return;
    }
}

/* Run the four module cleanup callbacks when the sequence closes or exits through pause. */
void fn_1_AF0(s16 mode, s16 frameNo)
{
    fn_1_65C0();
    fn_1_4B4C();
    fn_1_F17C();
    fn_1_ED3C();
}

/* Always report the shared phase check complete; these callbacks add no gate of their own. */
s32 fn_1_B1C(s16 mode, s16 state)
{
    return 0;
}

/* Start the camera shake with a thirty-unit amplitude and zero phase. */
void fn_1_B24(void)
{
    lbl_1_bss_20 = 30.0f;
    lbl_1_bss_1C = 0.0f;
}

extern void fn_1_49F8(Vec *start, Vec *end, f32 factor, Vec *result);

/* Interpolate camera center, rotation, and zoom for a camera-view transition. */
void fn_1_B58(OM_CAMERA_VIEW *start, OM_CAMERA_VIEW *end, OM_CAMERA_VIEW *result,
              f32 factor)
{
    fn_1_49F8(&start->center, &end->center, factor, &result->center);
    fn_1_49F8(&start->rot, &end->rot, factor, &result->rot);
    result->zoom = start->zoom + factor * (end->zoom - start->zoom);
}

typedef struct M620BF0Entry {
    u8 unread_00_to_0B[12];
    /* Camera view currently applied while this entry runs. */
    M620BF0PositionData activeView;
    /* Camera view copied in when the sequence requests the next view. */
    M620BF0PositionData pendingView;
    u8 unread_44_to_87[68];
    /* Transition phase and elapsed object-manager frames within that phase. */
    s32 phase;
    s32 frameCount;
    u8 unread_90_to_A7[24];
} M620BF0Entry;

typedef struct M620BF0Work {
    u8 unread_00_to_1F[32];
    M620BF0Entry entries[1];
} M620BF0Work;

s32 fn_1_27B0(u32 cameras);
void fn_1_2320(Vec *direction, Vec *target, f32 fieldOfView, f32 distance);

/* Update a camera-view entry and apply the decaying shake on each object-manager tick. */
void fn_1_BF0(OMOBJ *object, s32 mode, s32 entryIndex)
{
    M620BF0Work *work = (M620BF0Work *)object->data;
    M620BF0Entry *entry = &work->entries[entryIndex];
    M620BF0PositionData *position = &entry->activeView;
    M620BF0PositionData *nextPosition = &entry->pendingView;

    switch (entry->phase) {
    case 0:
        if (fn_1_27B0(mode) != 0) {
            break;
        }
        entry->phase++;
        entry->frameCount = 0;
        *position = lbl_1_data_0;
        lbl_1_bss_0 = *position;
        /* fall through */
    case 1:
        if (++entry->frameCount < 180) {
            break;
        }
        entry->phase++;
        entry->frameCount = 0;
        lbl_1_bss_5C = 1;
        /* fall through */
    case 2:
        if (lbl_1_bss_58 != 0) {
            entry->phase++;
            entry->frameCount = 0;
            *position = *nextPosition;
            lbl_1_bss_0 = *position;
        }
        break;
    case 3:
        break;
    }

    if (lbl_1_bss_20 > 0.0) {
        if ((lbl_1_bss_20 *= 0.8) < 1.0) {
            lbl_1_bss_20 = 0.0f;
            *position = lbl_1_bss_0;
        } else {
            lbl_1_bss_1C += 70.0f;
            /* The shake offsets the horizontal center while preserving its height. */
            ((f32 *) position->cameraViewValueBits)[0] =
                (f32) ((f64) ((f32 *) lbl_1_bss_0.cameraViewValueBits)[0] +
                       0.2 * ((f64) lbl_1_bss_20 *
                              sin((3.141592653589793 * (f64) lbl_1_bss_1C) / 180.0)));
            ((f32 *) position->cameraViewValueBits)[2] =
                (f32) ((f64) ((f32 *) lbl_1_bss_0.cameraViewValueBits)[2] +
                       (f64) lbl_1_bss_20 * cos((3.141592653589793 * (f64) lbl_1_bss_1C) / 180.0));
        }
    }
    fn_1_2320(&lbl_1_data_B4, &lbl_1_data_A8,
              10.0f, 1000.0f);
}

/* Build a Trap Ease Artist data number from its file index. */
s32 fn_1_FB8(s32 fileIndex) {
    return fileIndex + DATA_m620;
}

s32 fn_1_FC0(void) {
    return lbl_1_bss_58;
}

s32 fn_1_FD0(void) {
    return lbl_1_bss_54;
}

/* Read the sequence timer; the computer-action caller uses the value left by the call
 * despite this void declaration. */
void fn_1_FE0(void) {
    MgSeqTimerValueGet();
}

/* Return the winner slot stored for one player. */
s16 fn_1_1000(s32 playerIndex) {
    return lbl_1_bss_34[playerIndex];
}

extern s16 lbl_1_bss_8[4];
extern s16 lbl_1_bss_10[4];
/* Return zero if nobody won; otherwise return the matched player's win status or the unmatched
 * ID. */
u32 fn_1_1018(u32 playerId)
{
    s32 index;
    s32 result;

    for (index = 0; index < 4; index++) {
        if (lbl_1_bss_34[index] >= 0) {
            break;
        }
    }
    if (index >= 4) {
        return 0;
    }
    for (index = 0; index < 4; index++) {
        if (playerId == (u32)lbl_1_bss_2C[index]) {
            if (lbl_1_bss_34[index] >= 0) {
                result = 1;
            } else {
                result = -1;
            }
            return result;
        }
    }
    return playerId;
}

/* Store the two player values used by the result and winner callbacks. */
void fn_1_10D4(s32 playerIndex, s32 playerId, s32 winnerSlot)
{
    playerIndex &= 3;
    lbl_1_bss_2C[playerIndex] = (s16)playerId;
    lbl_1_bss_24[playerIndex] = (s16)winnerSlot;
}

/* Mark a player's readiness slot for the four-player sequence. */
void fn_1_110C(unsigned int playerIndex) {
    playerIndex &= 3;
    lbl_1_bss_3C[playerIndex] = 1;
}

/* Return whether all four player readiness slots have been marked. */
int fn_1_112C() {
    int playerIndex;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_3C[playerIndex] == 0) return 0;
    }
    return 1;
}

M620BF0PositionData lbl_1_data_0 = {
    { M620_VIEW_CENTER_X_BIT_PATTERN, M620_VIEW_CENTER_Y_BIT_PATTERN,
      M620_VIEW_CENTER_Z_BIT_PATTERN, M620_VIEW_ROTATION_X_BIT_PATTERN,
      M620_VIEW_ROTATION_Y_BIT_PATTERN, M620_VIEW_ROTATION_Z_BIT_PATTERN,
      M620_VIEW_ZOOM_BIT_PATTERN }
};

HuVecF lbl_1_data_1C = {0.0f, 800.0f, 0.0f};
HuVecF lbl_1_data_28 = {0.0f, 0.0f, 0.0f};
GXColor lbl_1_data_34 = {255, 255, 255, 255};
u32 lbl_1_data_38[18] = {
    M620_DATA_WORD_MASK, 32, 96, 32, 64, 110,
    M620_DATA_WORD_450FC000_BIT_PATTERN, M620_DATA_WORD_44D48000_BIT_PATTERN,
    M620_DATA_WORD_44960000_BIT_PATTERN, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
MGSEQ_PARAM lbl_1_data_80 = {
    10, 0, fn_1_2E8, fn_1_324, fn_1_5C8, fn_1_638, fn_1_684,
    fn_1_708, fn_1_9E0, fn_1_AB8, fn_1_AF0
};
Vec lbl_1_data_A8 = {0.0f, 0.0f, 0.0f};
Vec lbl_1_data_B4 = {0.0f, -1.0f, 0.0f};

u32 lbl_1_bss_660;
OMOBJ *lbl_1_bss_658[2];
u32 lbl_1_bss_654;
OMOBJ *lbl_1_bss_650;
u32 lbl_1_bss_648[2];
OMOBJ *lbl_1_bss_448[128];
OMOBJ *lbl_1_bss_438[4];
OMOBJ *lbl_1_bss_3AC[35];
u32 lbl_1_bss_3A8;
s32 lbl_1_bss_3A4;
s32 lbl_1_bss_394[4];
Vec lbl_1_bss_388;
u32 lbl_1_bss_384;
u32 lbl_1_bss_380;
u32 lbl_1_bss_37C;
u32 lbl_1_bss_378;
u32 lbl_1_bss_338[16];
u32 lbl_1_bss_330[2];
u8 lbl_1_bss_320[16];
void *lbl_1_bss_310[4];
u32 lbl_1_bss_30C;
u32 lbl_1_bss_308;
u32 lbl_1_bss_300[2];
OMOBJ *lbl_1_bss_2FC;
OMOBJ *lbl_1_bss_2F8;
OMOBJ *lbl_1_bss_2F4;
OMOBJ *lbl_1_bss_2F0;
int lbl_1_bss_1B0[80];
int lbl_1_bss_70[80];

OMOBJMAN *lbl_1_bss_6C;
s32 lbl_1_bss_68;
s32 lbl_1_bss_64;
s32 lbl_1_bss_60;
s32 lbl_1_bss_5C;
s32 lbl_1_bss_58;
s32 lbl_1_bss_54;
s32 lbl_1_bss_50;
s32 lbl_1_bss_4C;
s32 lbl_1_bss_48;
s32 lbl_1_bss_44;
s16 lbl_1_bss_3C[4];
s16 lbl_1_bss_34[4];
s16 lbl_1_bss_2C[4];
s16 lbl_1_bss_24[4];
f32 lbl_1_bss_20;
f32 lbl_1_bss_1C;
M620BF0PositionData lbl_1_bss_0;
