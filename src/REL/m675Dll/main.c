/* Sets up Banana Shake and handles its opening, play and result sequence. */
#include "math.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/mg/seqman.h"
#include "game/charman.h"
#include "msm_stream.h"
#define MSM_SE_M675_2224 2224
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[], _dtors[];
void fn_1_A0(void);
OMOBJMAN *lbl_1_bss_40;
/* Opening phase/timer, music handle and looping sound handle. */
s32 lbl_1_bss_3C, lbl_1_bss_38, lbl_1_bss_34, lbl_1_bss_30;
/* Prop-effect stop status, result wait gate and prop-finish signal. */
s32 lbl_1_bss_2C, lbl_1_bss_28, lbl_1_bss_24;
/* Per-player arrival flags, result characters, lookup tokens and character IDs. */
s16 lbl_1_bss_1C[4], lbl_1_bss_14[4], lbl_1_bss_C[4], lbl_1_bss_4[4];
s32 lbl_1_bss_0; /* Set after the opening prop sequence has finished. */

void fn_1_4E4(s16, s16);
void fn_1_BBC(OMOBJMAN *);
void fn_1_C84(OMOBJMAN *, Vec *, Vec *, GXColor *);
void fn_1_136C(OMOBJMAN *, u32, f32, f32, f32, f32, OM_CAMERA_VIEW *);
s32 fn_1_A38(s32);
void fn_1_2414(s32, void *);
void fn_1_95C(OMOBJ *, void *, s32);
void fn_1_1FC8(void (*callback)(OMOBJ *, void *, s32));
void fn_1_2F38(OMOBJMAN *, void *);
void fn_1_E9A8(OMOBJMAN *);
void fn_1_F118(OMOBJMAN *);
void fn_1_4F2C(OMOBJMAN *);
void fn_1_7554(OMOBJMAN *);
void fn_1_D638(OMOBJMAN *);
s32 fn_1_8BC(s16, s16);
s32 fn_1_D3BC(void);
void fn_1_F004(void);
s32 fn_1_F058(s32);
void fn_1_766C(void), fn_1_5048(void), fn_1_EA30(void);
void fn_1_4BB4(Vec *, Vec *, float, Vec *);
void fn_1_1DD8(s32, Vec *, Vec *, float, float);
void fn_1_24CC(s32);

/* Camera and light settings used during setup. */
OM_CAMERA_VIEW lbl_1_data_0 = {{0.0f, 500.0f, 0.0f}, {-5.0f, 0.0f, 0.0f}, 2000.0f};
Vec lbl_1_data_1C = {0.0f, 800.0f, 0.0f};
Vec lbl_1_data_28 = {0.0f, 0.0f, 0.0f};
GXColor lbl_1_data_34 = {255, 255, 255, 255};

typedef struct M675FogParam_s {
    s32 layer; /* Render layer; -1 ends the list. */
    u32 cameras; /* Camera mask supplying the clipping distances. */
    GXFogType type; /* GX fog projection and falloff mode. */
    float start; /* Near fog distance in world units. */
    float end; /* Far fog distance before the controller's interpolation. */
    GXColor color; /* Fog color passed to GX. */
} M675FogParam;

/* The fog list ends at the entry whose layer is -1. */
M675FogParam lbl_1_data_38[] = {
    {0, 1, GX_FOG_NONE, 20.0f, 10000.0f, {192, 192, 192, 0}},
    {-1, 0, GX_FOG_NONE, 0.0f, 0.0f, {0, 0, 0, 0}}
};

typedef struct M675AudioConfig_s {
    u32 cameras; /* Camera mask; zero ends the list. */
    s32 panMin, panMax; /* Pan at the left and right screen edges. */
    s32 volumeMin, volumeMiddle, volumeMax; /* Volume at far, middle and near depth. */
    float distanceFar, distanceMiddle, distanceNear; /* Camera-space depth in world units. */
} M675AudioConfig;

/* Audio distance and panning ranges; a zero camera mask ends the list. */
M675AudioConfig lbl_1_data_68[] = {
    {HU3D_CAM_ALL, 32, 96, 32, 64, 110, 2400.0f, 1900.0f, 1400.0f},
    {0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f}
};

void fn_1_2D4(s16, s16);
void fn_1_310(s16, s16);
void fn_1_4AC(s16, s16);
void fn_1_520(s16, s16);
void fn_1_57C(s16, s16);
void fn_1_610(s16, s16);
void fn_1_6F4(s16, s16);
void fn_1_80C(short, short);
void fn_1_890(s16, s16);
MGSEQ_PARAM lbl_1_data_B0 = {
    15, MGSEQ_TIMER_BOTTOM,
    fn_1_2D4, fn_1_310, fn_1_4AC, fn_1_520, fn_1_57C,
    fn_1_610, fn_1_6F4, fn_1_80C, fn_1_890
};
Vec lbl_1_data_D8 = {0.0f, 200.0f, 500.0f};
Vec lbl_1_data_E4 = {0.1f, -1.0f, -0.35f};

typedef struct M675ShadowEntry {
    u8 unmappedBytes00To97[152];
    s32 setupState; /* Zero requests the one-time camera-motion stop. */
    s32 setupTimer; /* Reset when the entry's setup state advances. */
    u8 unmappedBytesA0ToB7[24];
} M675ShadowEntry;

typedef struct M675ShadowWork {
    s32 cameraMask; /* Cameras sharing the shadow setup. */
    u8 unmappedBytes04To07[4];
    f32 shadowFov; /* Shadow-camera field of view in degrees. */
    u8 unmappedBytes0CTo17[12];
    u8 shadowColor[4]; /* RGBA shadow tint; this callback applies RGB only. */
    u8 unmappedBytes1CTo1F[4];
    M675ShadowEntry entries[16]; /* One entry for each camera bit. */
    s32 initializationCount; /* Nonzero after the shared shadow setup has run. */
} M675ShadowWork;

/* Runs registered initializers before Banana Shake's minigame setup begins. */
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

/* Runs registered finalizers when the Banana Shake overlay is closed. */
void _epilog(void)

{

    const VoidFunc *dtor = _dtors;

    while (*dtor != 0) {

        (*dtor)();

        dtor++;

    }

}

/* Initializes Banana Shake's objects, camera, resources and sequence callbacks. */
void fn_1_A0(void) {

    s32 index;

    s32 resource;

    lbl_1_bss_34 = -1;

    lbl_1_bss_30 = -1;

    lbl_1_bss_28 = 0;

    lbl_1_bss_2C = 0;

    lbl_1_bss_0 = 0;

    for (index = 0; index < 4; index++) {

        lbl_1_bss_1C[index] = 0;

    }

    lbl_1_bss_40 = omInitObjMan(120, 8192);

    omGameSysInit(lbl_1_bss_40);

    MgSeqCreate(&lbl_1_data_B0);

    MgSeqModeHookAdd(MGSEQ_MODE_PREMAIN, fn_1_4E4);

    fn_1_BBC(lbl_1_bss_40);

    fn_1_C84(lbl_1_bss_40, &lbl_1_data_1C, &lbl_1_data_28, &lbl_1_data_34);

    Hu3DBGColorSet(0, 0, 0);

    fn_1_136C(lbl_1_bss_40, 1, 45.0f, 20.0f, 4000.0f,

              0.0f, &lbl_1_data_0);

    resource = fn_1_A38(0);

    fn_1_2414(1, HuDataSelHeapReadNum(resource, HU_MEMNUM_OVL, HEAP_MODEL));

    fn_1_1FC8(fn_1_95C);

    fn_1_2F38(lbl_1_bss_40, lbl_1_data_68);

    fn_1_E9A8(lbl_1_bss_40);

    fn_1_F118(lbl_1_bss_40);

    fn_1_4F2C(lbl_1_bss_40);

    fn_1_7554(lbl_1_bss_40);

    fn_1_D638(lbl_1_bss_40);

    for (index = 0; index < 10; index++) {

        HuPrcVSleep();

    }

}

/* The initialization callback immediately requests the fade-in phase. */
void fn_1_2D4(short mode,short frame) {
    if(!fn_1_8BC(mode,frame)) MgSeqModeNext();
}

/* The fade-in callback starts audio and waits for the opening prop sequence to finish. */
void fn_1_310(s16 mode, s16 frame)

{

    if (fn_1_8BC(mode, frame) != 0) {

        return;

    }

    if (frame == 0) {

        lbl_1_bss_3C = 0;

    }

    if (lbl_1_bss_34 == -1 && frame >= 18) {

        lbl_1_bss_34 = HuAudBGMPlay(MSM_STREAM_MGMUS_30);

        lbl_1_bss_30 = HuAudFXPlay(MSM_SE_M675_2224);

    }

    switch (lbl_1_bss_3C) {

    case 0:

        lbl_1_bss_3C++;

        lbl_1_bss_38 = 0;

        MgSeqModeChangeOff();

        break;

    case 1:

        if (++lbl_1_bss_38 < 30 || fn_1_D3BC() != 0) {

            break;

        }

        lbl_1_bss_3C++;

        lbl_1_bss_38 = 0;

        lbl_1_bss_0 = 1;

        /* Begin the final 60-frame opening hold on this same update. */

    case 2:

        if (++lbl_1_bss_38 >= 60) {

            MgSeqModeNext();

        }

        break;

    }

}

/* The start-message callback leaves the standard sequence behavior in control. */
void fn_1_4AC(short mode, short frame)
{
    if (fn_1_8BC(mode, frame) != 0) {
        return;
    }
}

/* The pre-main hook reveals all four score displays before timed play begins. */
void fn_1_4E4(short mode, short frame)
{
    if (fn_1_8BC(mode, frame) == 0) {
        fn_1_F004();
    }
}

/* The main callback signals the props to finish after play frame 780. */
void fn_1_520(s16 mode, s16 frame) {
    if ((fn_1_8BC(mode, frame) == 0) && (frame > 780)) {
        lbl_1_bss_24 = 1;
    }
}

/* The first finish-phase frame fades the music and signals the props to finish. */
void fn_1_57C(s16 mode, s16 frame)
{
    if ((fn_1_8BC(mode, frame) == 0) && (frame == 0)) {
        if (lbl_1_bss_34 != -1) {
            HuAudSStreamFadeOut(lbl_1_bss_34, 100);
            lbl_1_bss_34 = -1;
        }
        lbl_1_bss_24 = 1;
    }
}

/* The pre-winner callback records characters with a score and waits at least 120 frames. */
void fn_1_610(s16 mode, s16 frame)
{
    s32 playerIndex;

    if (fn_1_8BC(mode, frame) == 0) {
        if (frame == 0) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (fn_1_F058(playerIndex) != 0) {
                    lbl_1_bss_14[playerIndex] = lbl_1_bss_4[playerIndex];
                } else {
                    lbl_1_bss_14[playerIndex] = -1;
                }
            }
        }
        if (lbl_1_bss_28 == 0 && frame >= 120) {
            MgSeqModeNext();
            return;
        }
    }
}

/* The first winner frame adds qualifying scores to coin bonuses outside practice,
* then chooses the jingle. */
void fn_1_6F4(s16 mode, s16 frame) {
    s32 hasScore;
    s32 playerIndex;

    if ((fn_1_8BC(mode, frame) == 0) && (frame == 0)) {
        hasScore = 0;
        playerIndex = 0;
        goto check_players;
    player_loop:
        if (lbl_1_bss_14[playerIndex] >= 0) {
            s16 coinBonus;
            s32 previousCoinBonus;

            previousCoinBonus = GwPlayer[playerIndex].mgCoinBonus;
            coinBonus = previousCoinBonus + fn_1_F058(playerIndex);

            if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                GwPlayer[playerIndex].mgCoinBonus = coinBonus;
            }
        }
        playerIndex++;
    check_players:
        if (playerIndex < 4) {
            goto player_loop;
        }

        playerIndex = 0;
        goto check_results;
    result_loop:
        if (fn_1_F058(playerIndex) != 0) {
            hasScore = 1;
        }
        playerIndex++;
    check_results:
        if (playerIndex < 4) {
            goto result_loop;
        }

        if (hasScore != 0) {
            HuAudJinglePlay(MSM_STREAM_MGMUS_17);
            return;
        }
        HuAudJinglePlay(MSM_STREAM_FILESEL);
    }
}

/* The first fade-out frame stops the sound loop started during the opening. */
void fn_1_80C(short mode, short frame)

{

    if (fn_1_8BC(mode, frame) == 0 && frame == 0) {

        if (lbl_1_bss_30 != -1) {

            HuAudFXFadeOut(lbl_1_bss_30, 1000);

            lbl_1_bss_30 = -1;

        }

    }

}

/* The close callback removes character models and calls the prop and score close hooks. */
void fn_1_890(s16 mode, s16 frame)
{
    fn_1_766C();
    fn_1_5048();
    /* The score close hook is called twice; both calls currently have no effect. */
    fn_1_EA30();
    fn_1_EA30();
}

/* Sequence callbacks share this hook; it never intercepts a phase. */
s32 fn_1_8BC(s16 mode, s16 frame) { return 0; }

/* Interpolates camera center, rotation and zoom between two sequence views. */
void fn_1_8C4(OM_CAMERA_VIEW *start, OM_CAMERA_VIEW *end, OM_CAMERA_VIEW *result,
    float factor)
{
    fn_1_4BB4(&start->center, &end->center, factor, &result->center);
    fn_1_4BB4(&start->rot, &end->rot, factor, &result->rot);
    result->zoom = start->zoom + factor * (end->zoom - start->zoom);
}

/* Sets the shared shadow once; each camera entry's first visit requests camera zero's
 * opening motion to stop. */
void fn_1_95C(OMOBJ *object, void *callbackData, s32 entryIndex)
{
    void *entryByteAddresses[2];
    M675ShadowWork *work = object->data;
    M675ShadowEntry *entry = &work->entries[entryIndex];

    entryByteAddresses[1] = (u8 *) entry + 12;
    entryByteAddresses[0] = (u8 *) entry + 40;

    if (work->initializationCount == 0) {
        work->initializationCount++;
        fn_1_1DD8(work->cameraMask, &lbl_1_data_E4, &lbl_1_data_D8,
            work->shadowFov, 1000.0f);
        Hu3DShadowColSet(work->shadowColor[0], work->shadowColor[1],
            work->shadowColor[2]);
    }
    switch (entry->setupState) {
    case 0:
        fn_1_24CC(1);
        entry->setupState++;
        entry->setupTimer = 0;
        break;
    }
}

s32 fn_1_A38(s32 resource) { return resource + DATA_m675; }

s32 fn_1_A40(void) {
    return lbl_1_bss_2C;
}

s32 fn_1_A50(void) {
    return lbl_1_bss_24;
}

void fn_1_A60(void) {
    MgSeqTimerValueGet();
}

/* Result users receive the player's character ID, or -1 when that player scored nothing. */
s16 fn_1_A80(s32 playerIndex) {
    return lbl_1_bss_14[playerIndex];
}

/* Player result motions use this token lookup: 1 for a scorer and -1 for a non-scorer. */
u32 fn_1_A98(u32 playerToken) {
    s32 playerIndex;
    u32 outcome;

    playerIndex = 0;
    goto check;
loop:
    if (playerToken == (u32) lbl_1_bss_C[playerIndex]) {
        if (lbl_1_bss_14[playerIndex] >= 0) {
            outcome = 1;
        } else {
            outcome = (u32) -1;
        }
        return outcome;
    }
    playerIndex++;
check:
    if (playerIndex < 4) {
        goto loop;
    }
    /* Tokens outside the four registered players pass through unchanged. */
    return playerToken;
}

/* Player creation registers each player's lookup token and character ID for the results. */
void fn_1_B14(s32 playerIndex, s32 playerToken, s32 characterId)

{

    playerIndex &= 3;

    lbl_1_bss_C[playerIndex] = (s16) playerToken;

    lbl_1_bss_4[playerIndex] = (s16) characterId;

}

/* The player update marks arrival after reaching the result formation and waiting 60 frames. */
void fn_1_B4C(unsigned int playerIndex) {
    playerIndex&=3;
    lbl_1_bss_1C[playerIndex]=1;
}

/* Player and prop result updates wait until all four players have reached the result formation. */
int fn_1_B6C(void) {
    int playerIndex;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_1C[playerIndex] == 0)
            return 0;
    }
    return 1;
}
