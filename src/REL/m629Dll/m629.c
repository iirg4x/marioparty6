/* Pop Star arena, player actions, piston scoring, and match presentation. */
#include "game/main.h"
#include "game/object.h"
#include "game/mg/seqman.h"
/* The sequence controller keeps timer handles without accessing their internal state. */
typedef struct MgTimer_s MGTIMER;
MGTIMER *MgTimerCreate(int type);
int MgTimerValueGet(MGTIMER *timer);

typedef struct M629Impulse {
    s32 animationStep; /* Zero is free, one is queued; later steps animate and apply the impact. */
    float strength; /* Progress units added over 16 frames after the piston animation. */
} M629Impulse;

/* State shared by each actor's setup and per-frame update callbacks. */
typedef struct M629Player {
    s32 index; /* Actor slot: zero is the solo player; one through three are opponents. */
    s32 playerNo; /* Player configuration slot used for input, vibration, and coin awards. */
    s32 remappedCharNo; /* Character number with IDs 11 and above mapped to 9; not used by these
                         * callbacks. */
    s32 charNo; /* Character ID from the player's game configuration. */
    s32 padNo; /* Controller port, or -1 when this actor is CPU-controlled. */
    s32 difficulty; /* CPU difficulty index, from zero through three. */
    u8 unusedPlayerBytes[4]; /* Four bytes not accessed by these callbacks. */
    s32 idleFrames; /* Consecutive frames without a tap from the solo player. */
    s32 itemActive; /* Nonzero while the detached item toss is updated; zero before the toss or
                    * after the item is hidden. */
    HuVecF itemPos; /* World position of the falling item model. */
    float itemVelocityY; /* Vertical item speed, in world units per frame. */
    float itemRotationX; /* Item rotation around X, in degrees. */
    float itemScale; /* Uniform scale of the falling item model. */
    s32 motion; /* Current player animation and jump state. */
    s32 frame; /* Frames elapsed in the current opponent jump state. */
    float velocityY; /* Vertical jump speed, in world units per frame. */
    float impactHeight; /* Height where the opponent began its downward stomp. */
    s32 cpuInputCountdown; /* Frames until the CPU's next simulated button press. */
    float stompHeightRatio; /* CPU second-press threshold in absolute world Y, divided by 250. */
} M629Player;

/* Pending sound effects and their horizontal world positions. */
typedef struct M629Sound {
    s32 count; /* Queued sound events, up to three. */
    float x[3]; /* World X position for each queued event, used for panning. */
    s32 handle; /* Most recently started sound handle for this effect slot. */
} M629Sound;

extern M629Impulse lbl_1_bss_69C[4][16];
extern Mtx lbl_1_bss_30[30];
extern OMOBJ *lbl_1_bss_5E0;
extern s32 lbl_1_bss_10;
extern s32 lbl_1_bss_0;
extern s32 lbl_1_bss_C;
extern s32 lbl_1_bss_28;
extern s32 lbl_1_bss_2C;
extern s32 lbl_1_bss_5D0;
extern s16 lbl_1_bss_5D4;
extern OMOBJMAN *lbl_1_bss_5D8;
extern OMOBJ *lbl_1_bss_5E4[4];
extern OMOBJ *lbl_1_bss_5F4;
extern OMOBJ *lbl_1_bss_5F8;
extern M629Sound lbl_1_bss_5FC[8];
extern MGTIMER *lbl_1_bss_4[2];
extern s32 lbl_1_bss_14[5];
extern s32 lbl_1_bss_5DC;
extern int lbl_1_data_38[10];
extern u8 lbl_1_data_60[10];
extern u8 lbl_1_data_6A[10];
extern u8 lbl_1_data_74[10];
extern s32 lbl_1_data_80[10];
extern s32 lbl_1_data_C0;
extern s32 lbl_1_data_AC;
extern s32 lbl_1_data_B4;
extern s32 lbl_1_data_B0;
extern float lbl_1_data_B8[2];
extern s32 lbl_1_data_C4;
extern s32 lbl_1_data_C8;
extern float lbl_1_data_160[2];
extern s32 lbl_1_data_168[2];
extern MGSEQ_PARAM lbl_1_data_FC;
extern s32 lbl_1_data_CC[4];
extern s32 lbl_1_data_DC[4];
extern s32 lbl_1_data_EC[4];
extern HuVecF lbl_1_data_134;
extern HuVecF lbl_1_data_140;
extern GXColor lbl_1_data_14C;
extern GXColor lbl_1_data_1B4;
extern HuVecF lbl_1_data_1B8;

float fn_1_0(float x, float y);
void fn_1_134(float *x, float *y, float limit);
void fn_1_2DC(int *x, int *y, float limit);
void fn_1_5CC(void);
void fn_1_940(s32 group, float impactStrength);
void fn_1_9E8(OMOBJ *obj);
float fn_1_1DCC(float frame, float rise, float hold, float fall);
void fn_1_1F90(OMOBJ *obj);
void fn_1_3324(s16 mode, s16 frameNo);
void fn_1_3328(s16 mode, s16 frameNo);
void fn_1_332C(s16 mode, s16 frameNo);
void fn_1_3330(s16 mode, s16 frameNo);
void fn_1_3334(s16 mode, s16 frameNo);
void fn_1_3338(s16 mode, s16 frameNo);
void fn_1_333C(s16 mode, s16 frameNo);
void fn_1_336C(OMOBJ *obj);
void fn_1_3C8C(OMOBJ *obj);
void fn_1_4290(HU3D_MODEL *model, Mtx *matrix);
void fn_1_4368(HU3D_MODEL *model, Mtx *matrix);
void fn_1_444C(OMOBJ *obj);
void fn_1_53B8(OMOBJ *obj);
void fn_1_53C8(void);
void fn_1_58BC(s16 mode, s16 frameNo);
void fn_1_5A08(s16 mode, s16 frameNo);

#include "game/pad.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/gamemes.h"
#include "game/mg/actman.h"
#include "string.h"
#include "math.h"

/* Stream played when Pop Star's start banner sounds. */
#define M629_BGM_STREAM_ID MSM_STREAM_MGMUS_24
/* Rollover bound for the piston animation frame counter. */
#define M629_FRAME_COUNTER_LIMIT 4026531840U
/* Motion-table bits select the solo actor and the three opponents. */
#define M629_MOTION_SOLO_MASK 0x01
#define M629_MOTION_TEAM_MASK 0x02
/* Effects queued by taps, piston impacts, side completion, and the opening demonstration. */
#define M629_SE_SOLO_IMPACT 1824
#define M629_SE_SOLO_TAP 1825
#define M629_SE_SIDE_COMPLETE 1826
#define M629_SE_DAY_COMPLETE 1827
#define M629_SE_NIGHT_COMPLETE 1828
#define M629_SE_TEAM_IMPACT 1829
#define M629_SE_DAY_INTRO 1830
#define M629_SE_NIGHT_INTRO 1831

/* Character model resource table; these callbacks do not read it. */
u32 lbl_1_data_0[CHARNO_MAX] = {
    DATANUM(DATA_mariomdl1, 0), DATANUM(DATA_luigimdl1, 0),
    DATANUM(DATA_peachmdl1, 0), DATANUM(DATA_yoshimdl1, 0),
    DATANUM(DATA_wariomdl1, 0), DATANUM(DATA_daisymdl1, 0),
    DATANUM(DATA_waluigimdl1, 0), DATANUM(DATA_kinopiomdl1, 0),
    DATANUM(DATA_teresamdl1, 0), DATANUM(DATA_minikoopamdl1, 0),
    DATANUM(DATA_kinopikomdl1, 0), DATANUM(DATA_minikoopamdl1, 0),
    DATANUM(DATA_minikoopamdl1, 0), DATANUM(DATA_minikoopamdl1, 0)
};
/* Motion resource IDs and per-motion settings for the player animation table. */
int lbl_1_data_38[10] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 8),
    DATANUM(DATA_mariomot, 10), DATANUM(DATA_mariomot, 9),
    DATANUM(DATA_mario, 68), DATANUM(DATA_mario, 69),
    DATANUM(DATA_mariomot, 39), DATANUM(DATA_mariomot, 40)
};
/* Player actor selection mask for each motion entry. */
u8 lbl_1_data_60[10] = {
    M629_MOTION_SOLO_MASK | M629_MOTION_TEAM_MASK,
    M629_MOTION_TEAM_MASK, M629_MOTION_TEAM_MASK, M629_MOTION_TEAM_MASK,
    M629_MOTION_TEAM_MASK, M629_MOTION_TEAM_MASK,
    M629_MOTION_SOLO_MASK, M629_MOTION_SOLO_MASK,
    M629_MOTION_SOLO_MASK | M629_MOTION_TEAM_MASK,
    M629_MOTION_SOLO_MASK | M629_MOTION_TEAM_MASK
};
/* Inverted Hu3D loop selector: zero enables the motion loop attribute. */
u8 lbl_1_data_6A[10] = { 0, 1, 1, 1, 1, 1, 0, 1, 1, 1 };
/* Per-motion values passed to Hu3DMotionShiftSet. */
u8 lbl_1_data_74[10] = { 8, 0, 0, 0, 0, 0, 0, 4, 8, 8 };
/* Per-motion values passed to Hu3DMotionShiftSet. */
s32 lbl_1_data_80[10] = { 0, 0, 5, 0, 0, 0, 0, 1, 0, 0 };
s16 lbl_1_data_A8 = -1;
/* Match result: -1 while active, 0 or 1 for a side, and 4 for a tie. */
s32 lbl_1_data_AC = -1;
/* Frame counter used to sample the arena piston animation. */
s32 lbl_1_data_B0 = -1;
/* Stream handle for the match background music. */
s32 lbl_1_data_B4 = -1;
/* Side progress totals, seeded to 10 and 15 when play begins, then increased by completed
 * impacts. */
float lbl_1_data_B8[2] = { 0.0f, 0.0f };
/* Number of overlay-history entries to go back when the minigame closes. */
s32 lbl_1_data_C0 = 1;
/* Match length in seconds; the visible countdown begins in the final 30 seconds. */
s32 lbl_1_data_C4 = 300;
/* Saturating frames since the most recent side first reached its threshold; 65535 initially
 * disables shake. */
s32 lbl_1_data_C8 = 65535;
/* Arena X positions for the four player slots. */
s32 lbl_1_data_CC[4] = { -357, 116, 357, 596 };
/* Arena Y positions for the four player slots. */
s32 lbl_1_data_DC[4] = { 0, 65, 65, 65 };
/* Arena Z positions for the four player slots. */
s32 lbl_1_data_EC[4] = { 150, 140, 290, 140 };
/* Callback table for the M629 minigame sequence. */
MGSEQ_PARAM lbl_1_data_FC = {
    0, 0, fn_1_58BC, fn_1_3324, fn_1_3328, fn_1_332C,
    fn_1_3330, fn_1_3334, fn_1_3338, fn_1_333C, fn_1_5A08
};
u8 lbl_1_data_124[14] = {
    0, 0, 24, 90, 30, 0, 18, 42, 96, 30, 42, 30, 30, 30
};
/* Position of the arena's directional light. */
HuVecF lbl_1_data_134 = { 0.0f, 5000.0f, 0.0f };
/* Night-scene light direction. */
HuVecF lbl_1_data_140 = { 0.0f, -0.5f, -1.0f };
/* Night-scene light color. */
GXColor lbl_1_data_14C = { 176, 176, 255, 255 };

/* Active piston impacts, grouped by the four player positions. */
M629Impulse lbl_1_bss_69C[4][16];
/* Sound events waiting for playback, with the last handle per sound slot. */
M629Sound lbl_1_bss_5FC[8];
/* Sequence controller, camera-view object, player actors, and arena scene. */
OMOBJ *lbl_1_bss_5F8;
OMOBJ *lbl_1_bss_5F4;
OMOBJ *lbl_1_bss_5E4[4];
OMOBJ *lbl_1_bss_5E0;
/* Current minigame sequence mode. */
s32 lbl_1_bss_5DC;
/* Object manager for the arena and player objects. */
OMOBJMAN *lbl_1_bss_5D8;
/* Optional message handle, initialized to -1 and never assigned a live message by these
 * callbacks. */
s16 lbl_1_bss_5D4;
/* Global light handle used when configuring each player model. */
s32 lbl_1_bss_5D0;
/* Piston transforms sampled over the thirty-frame arena animation. */
Mtx lbl_1_bss_30[30];
/* One-time initialization flag for the result-display rotation angles. */
s32 lbl_1_bss_2C;
/* Next impact model slot in the eight-model effect cycle. */
s32 lbl_1_bss_28;
/* Match presentation phase: active, result transition, then player poses. */
s32 lbl_1_bss_14[5];
/* Flag selecting the night scene. */
s32 lbl_1_bss_10;
s32 lbl_1_bss_C;
/* Match countdown timer objects. */
MGTIMER *lbl_1_bss_4[2];
/* Current minigame sequence frame. */
s32 lbl_1_bss_0;

/* Returns the Euclidean length of a two-dimensional vector. */
float fn_1_0(float x, float y)
{
    return sqrtf(x * x + y * y);
}

/* Scales a float vector down when its length exceeds the requested limit. */
void fn_1_134(float *x, float *y, float limit)
{
    float magnitude = fn_1_0(*x, *y);

    if (magnitude > limit) {
        *x = limit * (*x / magnitude);
        *y = limit * (*y / magnitude);
    }
}

/* Scales an integer vector down when its length exceeds the requested limit. */
void fn_1_2DC(int *x, int *y, float limit)
{
    float magnitude = fn_1_0(*x, *y);

    if (magnitude > limit) {
        *x = limit * (*x / magnitude);
        *y = limit * (*y / magnitude);
    }
}

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

/* Runs registered REL constructors, then initializes the M629 scene. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;

    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_53C8();
    return 0;
}

/* Runs each registered REL destructor before the overlay unloads. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;

    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Applies pad 0 input to the arena camera position, zoom, and rotation. */
void fn_1_5CC(void)
{
    BOOL zoomIn;

    Center.x += 5.0 * (((HuPadBtn[0] & PAD_BUTTON_RIGHT) != 0)
        - ((HuPadBtn[0] & PAD_BUTTON_LEFT) != 0));
    Center.y += 25.0 * (((HuPadBtn[0] & PAD_BUTTON_UP) != 0)
        - ((HuPadBtn[0] & PAD_BUTTON_DOWN) != 0));
    Center.z += 20.0 * ((HuPadTrigL[0] - HuPadTrigR[0]) / 128.0);
    zoomIn = FALSE;
    if ((HuPadBtn[0] & PAD_BUTTON_Y) != 0 && CZoom > 25.0f) {
        zoomIn = TRUE;
    }
    CZoom += 25.0 * (((HuPadBtn[0] & PAD_BUTTON_X) != 0) - zoomIn);
    CZoom = CZoom <= 0.0f ? 25.0f : CZoom;
    CRot.y += HuPadSubStkX[0] / 32.0;
    CRot.x += HuPadSubStkY[0] / -32.0;
}

/* Called on solo taps and opponent landings to queue a piston impact; drops it when all slots
 * are occupied (16 for the solo actor, three per opponent). */
void fn_1_940(s32 group, float impactStrength)
{
    s32 i;

    for (i = 0; i < (group != 0 ? 3 : 16); i++) {
        if (lbl_1_bss_69C[group][i].animationStep == 0) {
            lbl_1_bss_69C[group][i].animationStep = 1;
            lbl_1_bss_69C[group][i].strength = impactStrength;
            break;
        }
    }
}

/* The setup callback installs this player update; it handles taps, opponent jumps, and the solo
 * item. */
void fn_1_9E8(OMOBJ *obj)
{
    M629Player *work = obj->data;
    s32 held = 0;
    s32 pressed = 0;
    s32 trigger = 0;
    s32 i;
    s32 index = work->index;
    M629Player *playerWork[4];

    lbl_1_bss_5DC = MgSeqModeGet();
    lbl_1_bss_0 = MgSeqFrameNoGet();
    /* Read all four actor-data pointers; this callback does not use the collected array. */
    for (i = 0; i < 4; i++) {
        playerWork[i] = lbl_1_bss_5E4[i]->data;
    }
    /* During fade-in, force solo taps every eight frames from 100 through 172 and opponent presses
     * at 100 and 105. */
    if (lbl_1_bss_5DC == 2) {
        if (lbl_1_bss_0 >= 100 && lbl_1_bss_0 < 180) {
            if ((lbl_1_bss_0 - 100) % 8 == 0 && work->index == 0) {
                pressed = held = 1;
            }
        }
        if (lbl_1_bss_0 == 100 && work->index != 0) {
            pressed = held = 1;
        }
        if (lbl_1_bss_0 == 105 && work->index != 0) {
            pressed = held = 1;
        }
    } else if (lbl_1_bss_5DC == 5) {
        if (work->padNo != -1) {
            trigger = HuPadBtnDown[work->padNo] & PAD_BUTTON_TRIGGER_L;
            pressed = HuPadBtnDown[work->padNo] & PAD_BUTTON_A;
            held = HuPadBtn[work->padNo] & PAD_BUTTON_A;
        } else if (work->index == 0) {
            s32 interval[4] = { 18, 15, 9, 4 };
            s32 variation[4] = { 4, 3, 2, 1 };
            s32 extraRate[4] = { 1, 1, 1, 5 };

            work->cpuInputCountdown -= work->cpuInputCountdown > 0;
            if (work->cpuInputCountdown <= 0) {
                if (lbl_1_bss_0 > 1) {
                    pressed = held = 1;
                }
                work->cpuInputCountdown = rand8() % (variation[work->difficulty] * 2 + 1)
                    + ((rand8() % extraRate[work->difficulty] != 0 ? 1 : 0)
                        + interval[work->difficulty])
                    - variation[work->difficulty];
            }
        } else {
            s32 interval[4] = { 145, 100, 75, 20 };
            s32 variation[4] = { 40, 35, 35, 20 };
            float heights[4][2] = {
                { 0.5f, 0.55f }, { 0.5f, 0.6f },
                { 0.5f, 0.5f }, { 0.75f, 0.25f }
            };

            if (work->motion == 0) {
                work->cpuInputCountdown -= work->cpuInputCountdown > 0;
                if (work->cpuInputCountdown <= 0) {
                    if (lbl_1_bss_0 <= 1) {
                        work->cpuInputCountdown = (interval[work->difficulty]
                            + rand8() % (variation[work->difficulty] + 1)) / 3;
                    } else {
                        pressed = held = 1;
                        work->stompHeightRatio = heights[work->difficulty][0]
                            + heights[work->difficulty][1] * rand8() / 255.0f;
                        work->cpuInputCountdown = interval[work->difficulty]
                            + rand8() % (variation[work->difficulty] + 1);
                    }
                }
            } else if (work->motion == 1) {
                held = 1;
                /* Higher CPU difficulties may time the second press on the jump's ascent. */
                if ((work->velocityY > 0.0f &&
                     obj->trans.y > 250.0f * work->stompHeightRatio &&
                     work->difficulty >= 2) ||
                    (work->velocityY < 0.0f &&
                     obj->trans.y < 250.0f * work->stompHeightRatio)) {
                    pressed = held = 1;
                }
            }
        }
    }
    if (lbl_1_data_AC == -1 || lbl_1_bss_5DC < 8) {
        if (work->index == 0) {
            work->idleFrames++;
            if (pressed != 0) {
                work->idleFrames = 0;
                /* Request a one-unit impact in both the demonstration and gameplay;
                 * a full impact queue drops the request. */
                fn_1_940(0, lbl_1_bss_5DC == 5 ? 1 : 1);
                if (lbl_1_bss_5FC[1].count < 3) {
                    lbl_1_bss_5FC[1].x[lbl_1_bss_5FC[1].count] = lbl_1_data_CC[0];
                    lbl_1_bss_5FC[1].count++;
                }
                if (lbl_1_bss_5DC < 5) {
                    Hu3DModelAttrReset(lbl_1_bss_5E0->mdlId[25], HU3D_MOTATTR_PAUSE);
                }
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[7], lbl_1_data_80[7],
                    lbl_1_data_74[7], lbl_1_data_6A[7] ? 0 : HU3D_MOTATTR_LOOP);
                work->motion = 7;
            } else if (work->idleFrames == 10) {
                work->motion = 6;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[6], lbl_1_data_80[6],
                    lbl_1_data_74[6], lbl_1_data_6A[6] ? 0 : HU3D_MOTATTR_LOOP);
            }
            /* After play, with the solo idle motion selected and at least ten input-free frames,
             * detach and toss the item upward. */
            if (lbl_1_bss_5DC > 5 && work->motion == 6 && work->idleFrames >= 10) {
                HuVecF pos;

                Hu3DModelObjPosGet(obj->mdlId[0], CharModelItemHookGet(work->charNo, 2, 0), &pos);
                work->itemPos.x = pos.x;
                work->itemPos.y = pos.y;
                work->itemPos.z = pos.z;
                work->itemVelocityY = 20.0f;
                work->itemRotationX = 0.0f;
                work->itemScale = 1.0f;
                work->itemActive = 1;
                Hu3DModelHookReset(obj->mdlId[0]);
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_data_80[0],
                    lbl_1_data_74[0], lbl_1_data_6A[0] ? 0 : HU3D_MOTATTR_LOOP);
                work->motion = 0;
            }
        } else {
            HuVecF pos;
            float groundY;

            Hu3DModelObjPosGet(lbl_1_bss_5E0->mdlId[work->index + 10], "m629_01-cyl42", &pos);
            groundY = (65.0f + pos.y) - 55.5;
            work->frame++;
            switch (work->motion) {
            case 2:
                if (work->frame > 3) {
                    work->motion = 0;
                    work->frame = 0;
                    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_data_80[0],
                        lbl_1_data_74[0], lbl_1_data_6A[0] ? 0 : HU3D_MOTATTR_LOOP);
                }
                /* Fall through to the grounded input path. */
            case 0:
                obj->trans.y = groundY;
                if (pressed != 0 && (work->motion != 2 || work->frame > 1)) {
                    work->motion = 1;
                    work->frame = 0;
                    work->velocityY = 28.14f;
                    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1], lbl_1_data_80[1],
                        lbl_1_data_74[1], lbl_1_data_6A[1] ? 0 : HU3D_MOTATTR_LOOP);
                }
                if (work->motion == 1) {
            case 1:
                    /* Releasing A caps upward speed at 12 before gravity; after frame zero, an A/L
                     * press below that speed starts the stomp windup. */
                    if (held == 0 && work->velocityY >= 12.0f) {
                        work->velocityY = 12.0f;
                    }
                    work->velocityY += -1.5f;
                    obj->trans.y += work->velocityY;
                    if (obj->trans.y <= groundY) {
                        work->motion = 2;
                        work->frame = 0;
                        obj->trans.y = groundY;
                        work->velocityY = 0.0f;
                        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[2], lbl_1_data_80[2],
                            lbl_1_data_74[2], lbl_1_data_6A[2] ? 0 : HU3D_MOTATTR_LOOP);
                    } else if (work->velocityY < 12.0f && (pressed | trigger) != 0 &&
                               work->frame > 0) {
                        work->motion = 3;
                        work->frame = 0;
                        work->velocityY = 0.0f;
                        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[3], lbl_1_data_80[3],
                            lbl_1_data_74[3], lbl_1_data_6A[3] ? 0 : HU3D_MOTATTR_LOOP);
                    }
                }
                break;
            case 3:
                /* Hold position for the stomp windup; at frame 20, save world Y and begin the
                 * accelerated descent. */
                if (work->frame == 20) {
                    work->velocityY = -10.0f;
                    work->impactHeight = obj->trans.y;
                }
                if (work->frame >= 20) {
                    work->velocityY += -1.5f;
                    obj->trans.y += work->velocityY;
                    if (obj->trans.y <= groundY) {
                        work->motion = 4;
                        work->frame = 0;
                        obj->trans.y = groundY;
                        work->velocityY = 0.0f;
                        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[4], lbl_1_data_80[4],
                            lbl_1_data_74[4], lbl_1_data_6A[4] ? 0 : HU3D_MOTATTR_LOOP);
                        omVibrate(work->playerNo, 20, 7, 3);
                        /* During play, strength uses saved stomp-start world Y / 250, times ten
                         * and capped at ten; outside play, use five. */
                        fn_1_940(work->index, lbl_1_bss_5DC == 5
                                                  ? (work->impactHeight > 250.0f
                                                         ? 10.0f
                                                         : 10.0f * (work->impactHeight / 250.0f))
                                                  : 5.0f);
                        if (lbl_1_bss_5DC < 5) {
                            Hu3DModelAttrReset(lbl_1_bss_5E0->mdlId[26], HU3D_MOTATTR_PAUSE);
                        }
                    }
                }
                break;
            case 4:
                obj->trans.y = groundY;
                if (work->frame >= 23) {
                    work->motion = 5;
                    work->frame = 0;
                    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[5], lbl_1_data_80[5],
                        lbl_1_data_74[5], lbl_1_data_6A[5] ? 0 : HU3D_MOTATTR_LOOP);
                }
                break;
            case 5:
                if (work->frame >= 12) {
                    work->motion = 0;
                    work->frame = 0;
                    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_data_80[0],
                        lbl_1_data_74[0], lbl_1_data_6A[0] ? 0 : HU3D_MOTATTR_LOOP);
                }
                break;
            }
        }
    }
    if (work->itemActive > 0) {
        work->itemPos.y += work->itemVelocityY;
        work->itemVelocityY -= 0.8f;
        work->itemRotationX += 32.727272f;
        work->itemScale -= 0.02f;
        Hu3DModelPosSet(obj->mdlId[1], work->itemPos.x, work->itemPos.y, work->itemPos.z);
        Hu3DModelRotSet(obj->mdlId[1], work->itemRotationX, 0.0f, 0.0f);
        Hu3DModelScaleSet(obj->mdlId[1], work->itemScale, work->itemScale, work->itemScale);
        /* Once vertical speed falls below -1 after gravity, replace the item with smoke and
         * hide it without waiting for ground contact. */
        if (work->itemVelocityY < -1.0f) {
            HuVecF pos;

            pos.x = work->itemPos.x;
            pos.y = work->itemPos.y;
            pos.z = work->itemPos.z;
            CharEffectSmokeCreate(-1, &pos);
            Hu3DModelAttrSet(obj->mdlId[1], HU3D_ATTR_DISPOFF);
            work->itemActive = 0;
        }
    }
}

/* Returns cumulative progress through a rise/hold/fall rate profile; forces one at
 * rise + hold + fall - 1. */
float fn_1_1DCC(float frame, float rise, float hold, float fall)
{
    float area = fall / 2.0f + (rise / 2.0f + hold);

    return frame < rise
        ? ((1.0f + frame) / 2.0f * frame) / rise / area
        : frame < rise + hold
            ? ((1.0f + rise) / 2.0f * rise) / rise / area
                + (frame - rise) / area
            : frame < fall + (rise + hold) - 1.0f
                ? 1.0f - ((1.0f + (fall + (rise + hold) - frame - 1.0f))
                    / 2.0f * (fall + (rise + hold) - 1.0f - frame - 1.0f))
                    / (fall - 1.0f) / area
                : 1.0f;
}

/* Sine-wave phase angles in degrees for the two side models' rotation wobble. */
float lbl_1_data_160[2] = { 0.0f, 0.0f };
/* One-time completion flags for the two result displays. */
s32 lbl_1_data_168[2] = { 0, 0 };

/* The arena setup callback installs this per-frame update for pistons, side scores, results, and
 * camera. */
void fn_1_1F90(OMOBJ *obj)
{
    s32 i;
    s32 group;
    HU3D_MODEL *model;
    M629Player *playerWork[4];

    for (i = 0; i < 4; i++) {
        playerWork[i] = lbl_1_bss_5E4[i]->data;
    }
    lbl_1_bss_5DC = MgSeqModeGet();
    lbl_1_bss_0 = MgSeqFrameNoGet();
    lbl_1_data_B0++;
    lbl_1_data_B0 %= M629_FRAME_COUNTER_LIMIT;
    lbl_1_bss_C += lbl_1_bss_C < 65535;
    for (i = 0; i < (lbl_1_bss_10 ? 1 : 2); i++) {
        model = &Hu3DData[obj->mdlId[i + 28]];
        model->pos.x -= i == 0 ? 0.16666667f : 0.125f;
    }
    /* On arena frames 10-39, cache air-motion frames 0-29 and remove the sampling model's
     * 32768-unit Z offset. */
    if (lbl_1_data_B0 >= 10 && lbl_1_data_B0 < 40) {
        Hu3DMotionTimeSet(obj->mdlId[18], lbl_1_data_B0 - 10);
        Hu3DModelObjMtxGet(obj->mdlId[18], "m629_pistonhoos-air", lbl_1_bss_30[lbl_1_data_B0 - 10]);
        lbl_1_bss_30[lbl_1_data_B0 - 10][2][3] -= 32768.0f;
    }
    for (group = 0; group < 4; group++) {
        for (i = 0; i < (group != 0 ? 3 : 16); i++) {
            if (lbl_1_bss_69C[group][i].animationStep != 0) {
                if (lbl_1_bss_69C[group][i].animationStep == 1) {
                    s32 piston[4] = { 2, 11, 12, 13 };
                    s32 air[4] = { 14, 15, 16, 17 };

                    Hu3DModelAttrReset(obj->mdlId[piston[group]], HU3D_MOTATTR_PAUSE);
                    Hu3DMotionTimeSet(obj->mdlId[piston[group]], 0.0f);
                    if (group == 0) {
                        Hu3DModelAttrReset(obj->mdlId[lbl_1_bss_28 + 3], HU3D_ATTR_DISPOFF);
                        Hu3DModelAttrReset(obj->mdlId[lbl_1_bss_28 + 3], HU3D_MOTATTR_PAUSE);
                        Hu3DMotionTimeSet(obj->mdlId[lbl_1_bss_28 + 3], 0.0f);
                        lbl_1_bss_28++;
                        lbl_1_bss_28 %= 8;
                    } else if (group != 0) {
                        Hu3DModelAttrReset(obj->mdlId[air[group]], HU3D_MOTATTR_PAUSE);
                        Hu3DMotionTimeSet(obj->mdlId[air[group]], 0.0f);
                    }
                }
                lbl_1_bss_69C[group][i].animationStep++;
                if (lbl_1_bss_69C[group][i].animationStep == (group != 0 ? 30 : 30)) {
                    if (lbl_1_bss_5FC[group != 0 ? 5 : 0].count < 3) {
                        lbl_1_bss_5FC[group != 0 ? 5 : 0].x[lbl_1_bss_5FC[group != 0 ? 5 : 0].count]
                            = lbl_1_data_CC[group != 0 ? 2 : 0];
                        lbl_1_bss_5FC[group != 0 ? 5 : 0].count++;
                    }
                }
                if (lbl_1_bss_69C[group][i].animationStep > (group != 0 ? 30 : 30)) {
                    /* Each impact contributes its value evenly over the following 16 frames. */
                    lbl_1_data_B8[group != 0] += lbl_1_bss_69C[group][i].strength / 16.0f;
                    if (lbl_1_bss_69C[group][i].animationStep >= (group != 0 ? 30 : 30) + 16) {
                        lbl_1_bss_69C[group][i].strength = 0.0f;
                        lbl_1_bss_69C[group][i].animationStep = 0;
                    }
                }
            }
        }
    }
    if ((lbl_1_bss_14[0] == 1 || lbl_1_bss_14[0] == 2) && lbl_1_data_AC == -1
        && (lbl_1_data_B8[0] >= 110.0f || lbl_1_data_B8[1] >= 165.0f)) {
        /* Reaching both side thresholds in the same update produces a tie. */
        lbl_1_data_AC = (lbl_1_data_B8[0] >= 110.0f && lbl_1_data_B8[1] >= 165.0f)
            ? 4 : lbl_1_data_B8[0] >= 110.0f ? 0
            : lbl_1_data_B8[1] >= 165.0f ? 1 : 4;
        if (lbl_1_data_AC == 0) {
            omVibrate(playerWork[0]->playerNo, 30, 30, 0);
        } else if (lbl_1_data_AC == 1) {
            omVibrate(playerWork[1]->playerNo, 30, 30, 0);
            omVibrate(playerWork[2]->playerNo, 30, 30, 0);
            omVibrate(playerWork[3]->playerNo, 30, 30, 0);
        }
    }
    for (i = 0; i < 2; i++) {
        float ratio = lbl_1_data_B8[i] / (i == 0 ? 110 : 165);

        if (lbl_1_bss_2C == 0) {
            lbl_1_bss_2C = 1;
            lbl_1_data_160[0] = 360.0f * ((float)rand8() / 256.0f);
            lbl_1_data_160[1] = 360.0f * ((float)rand8() / 256.0f);
        }
        if (ratio >= 1.0f) {
            Hu3DModelAttrSet(obj->mdlId[i + 19], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(obj->mdlId[i + 21], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(obj->mdlId[i + 23], HU3D_ATTR_DISPOFF);
            if (lbl_1_data_168[i] == 0) {
                Hu3DMotionTimeSet(obj->mdlId[i + 21], 0.0f);
                Hu3DMotionTimeSet(obj->mdlId[i + 23], 0.0f);
                lbl_1_data_C8 = 0;
                if (lbl_1_bss_5FC[2].count < 3) {
                    lbl_1_bss_5FC[2].x[lbl_1_bss_5FC[2].count] = lbl_1_data_CC[i * 2];
                    lbl_1_bss_5FC[2].count++;
                }
                if (lbl_1_bss_5FC[lbl_1_bss_10 ? 4 : 3].count < 3) {
                    lbl_1_bss_5FC[lbl_1_bss_10 ? 4 : 3].x[lbl_1_bss_5FC[lbl_1_bss_10 ? 4 : 3].count]
                        = lbl_1_data_CC[i * 2];
                    lbl_1_bss_5FC[lbl_1_bss_10 ? 4 : 3].count++;
                }
            }
            Hu3DModelAttrReset(obj->mdlId[i + 32], HU3D_ATTR_DISPOFF);
            lbl_1_data_168[i] = 1;
        } else {
            float remaining = 1.0f - ratio;

            /* Advance the side display toward motion frame 58 using a blend of one-quarter
             * linear and three-quarters cubic remaining progress. */
            Hu3DMotionTimeSet(obj->mdlId[i + 19], 58.0f * (1.0f
                - (0.25f * remaining + 0.75f * (remaining * (remaining * remaining)))));
        }
        lbl_1_data_160[i] += 0.85714287f;
        /* Apply the same Z wobble to both side displays; its amplitude is zero below
         * one-quarter progress and ramps to two degrees at three-quarters progress. */
        Hu3DModelRotSet(obj->mdlId[i + 19], 0.0f, 0.0f,
            sin(M_PI * lbl_1_data_160[i] / 180.0)
                * (ratio < 0.25f ? 0.0f : ratio < 0.75f ? 2.0f * ((ratio - 0.25f) / 0.5f) : 2.0f));
        Hu3DModelRotSet(obj->mdlId[i + 21], 0.0f, 0.0f,
            sin(M_PI * lbl_1_data_160[i] / 180.0)
                * (ratio < 0.25f ? 0.0f : ratio < 0.75f ? 2.0f * ((ratio - 0.25f) / 0.5f) : 2.0f));
    }
    lbl_1_data_C8 += lbl_1_data_C8 < 65535;
    /* During non-tied winner presentation, ease zoom and center X/Y five percent toward
     * the winning side each update; earlier phases otherwise force the arena center. */
    if ((lbl_1_bss_5DC == 8 || lbl_1_bss_14[0] == 3) && lbl_1_data_AC != 4) {
        float weight = 0.05f;
        float remaining = 1.0f - weight;

        CZoom = 1800.0f * weight + CZoom * remaining;
        Center.x = Center.x * remaining + weight * (lbl_1_data_AC == 0 ? -357 : 357);
        Center.y = 75.0f * weight + Center.y * remaining;
    } else if (lbl_1_bss_5DC < 8) {
        Center.x = 0.0f;
        Center.y = 230.0f;
        Center.z = 0.0f;
    }
    /* Shake center X/Y for counter values one through nine after side completion,
     * reducing the random displacement each update. */
    if (lbl_1_data_C8 < 10) {
        Center.x += 5.0f * ((10 - lbl_1_data_C8) * (2.0f * ((float)rand8() / 255.0f) - 1.0f));
        Center.y += 5.0f * ((10 - lbl_1_data_C8) * (2.0f * ((float)rand8() / 255.0f) - 1.0f));
    }
}

void fn_1_3324(s16 mode, s16 frameNo) {}
void fn_1_3328(s16 mode, s16 frameNo) {}
void fn_1_332C(s16 mode, s16 frameNo) {}
void fn_1_3330(s16 mode, s16 frameNo) {}
void fn_1_3334(s16 mode, s16 frameNo) {}
void fn_1_3338(s16 mode, s16 frameNo) {}

/* Fade-out hook: requests advance at frame 60 if called that late; the default phase ends before
 * this test fires. */
void fn_1_333C(s16 mode, s16 frameNo)
{
    if (frameNo >= 60) {
        MgSeqModeNext();
    }
}

/* The sequence object switches to this callback to play queued effects and advance timer and result
 * modes. */
void fn_1_336C(OMOBJ *obj)
{
    s32 i;
    s32 frame;
    s32 handle;
    M629Player *playerWork[4];

    frame = MgSeqFrameNoGet();
    lbl_1_bss_5DC = MgSeqModeGet();
    for (i = 0; i < 4; i++) {
        playerWork[i] = lbl_1_bss_5E4[i]->data;
    }
    for (i = 0; i < 8; i++) {
        s32 sound[8] = {
            M629_SE_SOLO_IMPACT, M629_SE_SOLO_TAP, M629_SE_SIDE_COMPLETE, M629_SE_DAY_COMPLETE,
            M629_SE_NIGHT_COMPLETE, M629_SE_TEAM_IMPACT, M629_SE_DAY_INTRO, M629_SE_NIGHT_INTRO
        };

        if (lbl_1_bss_5FC[i].count > 3) {
            lbl_1_bss_5FC[i].count = 3;
        }
        /* Play at most one event per effect slot this update, consuming the newest queued position
         * first. */
        if (lbl_1_bss_5FC[i].count > 0) {
            if (sound[i] != -1) {
                handle = HuAudFXPlay(sound[i]);
                lbl_1_bss_5FC[i].handle = handle;
                HuAudFXPanning(handle, 0.5 + (64.0f + 63.0f * (0.25f
                    * (lbl_1_bss_5FC[i].x[lbl_1_bss_5FC[i].count - 1] / 400.0f))));
            }
            lbl_1_bss_5FC[i].count--;
        }
    }
    if (lbl_1_bss_5DC == 3 && frame == 1) {
        HuAudFXFadeOut(lbl_1_bss_5FC[lbl_1_bss_10 ? 7 : 6].handle, 500);
    }
    frame = MgSeqFrameNoGet();
    lbl_1_bss_5DC = MgSeqModeGet();
    switch (lbl_1_bss_5DC) {
    case 2:
        if (frame == 1) {
            if (lbl_1_bss_5FC[lbl_1_bss_10 ? 7 : 6].count < 3) {
                lbl_1_bss_5FC[lbl_1_bss_10 ? 7 : 6].x[
                    lbl_1_bss_5FC[lbl_1_bss_10 ? 7 : 6].count] = 0.0f;
                lbl_1_bss_5FC[lbl_1_bss_10 ? 7 : 6].count++;
            }
        }
        break;
    case 3:
        /* Begin the music once the start banner has played its sound cue. */
        if (lbl_1_data_B4 == -1 && (GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY)) {
            lbl_1_data_B4 = HuAudBGMPlay(M629_BGM_STREAM_ID);
        }
        break;
    case 5:
        if (frame == 0) {
            lbl_1_data_B8[0] = 10.0f;
            lbl_1_data_B8[1] = 15.0f;
            lbl_1_bss_14[0] = 1;
        }
        /* One frame before the final 30 seconds, create and start the 1800-frame countdown. */
        if (lbl_1_bss_4[0] == 0 && frame == (lbl_1_data_C4 - 30) * 60 - 1) {
            lbl_1_bss_4[0] = MgTimerCreate(0);
            MgTimerParamSet(lbl_1_bss_4[0], 1800, 0, 0);
            MgTimerRecordDispOn(lbl_1_bss_4[0]);
            MgTimerModeOnSet(lbl_1_bss_4[0], 0);
        }
        /* End play on a decided result or zero timer; queued impacts may still decide the result
         * during the finish phase. */
        if (lbl_1_data_AC != -1) {
            MgSeqModeNext();
        } else if (lbl_1_bss_4[0] != 0 && MgTimerValueGet(lbl_1_bss_4[0]) == 0) {
            MgSeqModeNext();
        }
        break;
    case 6:
        lbl_1_bss_14[0] = 2;
        /* Fade the music once the finish banner has played its sound cue. */
        if (lbl_1_data_B4 != -1 && (GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY)) {
            HuAudSStreamFadeOut(lbl_1_data_B4, 100);
            lbl_1_data_B4 = -1;
        }
        /* After 20 finish frames, hide the countdown and discard this handle; its timer process
         * remains alive. */
        if (frame >= 20 && lbl_1_bss_4[0] != 0) {
            MgTimerDispOff(lbl_1_bss_4[0]);
            lbl_1_bss_4[0] = NULL;
        }
        break;
    case 7:
        if (frame >= 60) {
            if (lbl_1_data_AC == -1) {
                lbl_1_data_AC = 4;
            }
            /* Register the winning characters and set a ten-coin bonus for each winner; practice
             * mode ignores the bonus writes. */
            if (lbl_1_data_AC == 0) {
                MgSeqWinnerSet(playerWork[0]->charNo, -1, -1, -1);
                GWMgCoinBonusSet(playerWork[0]->playerNo, 10);
            } else if (lbl_1_data_AC == 1) {
                MgSeqWinnerSet(playerWork[1]->charNo, playerWork[2]->charNo,
                    playerWork[3]->charNo, -1);
                GWMgCoinBonusSet(playerWork[1]->playerNo, 10);
                GWMgCoinBonusSet(playerWork[2]->playerNo, 10);
                GWMgCoinBonusSet(playerWork[3]->playerNo, 10);
            } else {
                MgSeqWinnerSet(-1, -1, -1, -1);
            }
            MgSeqModeNext();
        }
        break;
    case 8:
        if (frame == 0) {
            lbl_1_bss_14[0] = 3;
            for (i = 0; i < 4; i++) {
                if (lbl_1_data_AC == (i > 0)) {
                    Hu3DMotionShiftSet(lbl_1_bss_5E4[i]->mdlId[0], lbl_1_bss_5E4[i]->mtnId[8],
                                       lbl_1_data_80[8], lbl_1_data_74[8],
                                       lbl_1_data_6A[8] ? 0 : HU3D_MOTATTR_LOOP);
                } else {
                    Hu3DMotionShiftSet(lbl_1_bss_5E4[i]->mdlId[0], lbl_1_bss_5E4[i]->mtnId[9],
                                       lbl_1_data_80[9], lbl_1_data_74[9],
                                       lbl_1_data_6A[9] ? 0 : HU3D_MOTATTR_LOOP);
                }
            }
        }
        break;
    case 0:
    case 1:
    case 4:
    case 9:
    case 10:
    case 11:
        break;
    }
}

/* Day-scene light color and direction. */
GXColor lbl_1_data_1B4 = { 255, 255, 255, 255 };
HuVecF lbl_1_data_1B8 = { 0.2f, -1.0f, -0.5f };

/* The object manager runs this on the actor's first update to build its models and install its
 * per-frame handler. */
void fn_1_3C8C(OMOBJ *obj)
{
    M629Player *playerState;
    s32 i;
    s32 modelIndex;
    s32 j;
    s32 playerOrder[4] = { 0, 1, 2, 3 };
    s32 configuredPlayerCount = 0;
    s32 lightId;

    playerState = obj->data;
    memset(playerState, 0, sizeof(M629Player));
    /* Order players by ascending configured group, then configuration index, before assigning actor
     * slots. */
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (i == GwPlayerConf[j].grpNo) {
                playerOrder[configuredPlayerCount] = j;
                configuredPlayerCount++;
            }
        }
    }
    playerState->index = obj->work[0];
    playerState->playerNo = playerOrder[playerState->index];
    /* playerOrder contains only 0-3, so this fallback for four is not reached by the ordering
     * above. */
    if (playerState->playerNo == 4) {
        playerState->playerNo = playerState->index;
    }
    playerState->charNo = GwPlayerConf[playerState->playerNo].charNo;
    playerState->remappedCharNo = playerState->charNo >= 11 ? 9 : playerState->charNo;
    playerState->padNo = GwPlayerConf[playerState->playerNo].type
        ? -1 : GwPlayerConf[playerState->playerNo].padNo;
    playerState->difficulty = playerState->padNo != -1
        ? 0 : GwPlayerConf[playerState->playerNo].comDif;
    obj->mdlId[0] = CharModelCreate(playerState->charNo, 2);
    for (i = 0; i < 10; i++) {
        if (lbl_1_data_60[i] &
            (playerState->index == 0 ? M629_MOTION_SOLO_MASK : M629_MOTION_TEAM_MASK)) {
            obj->mtnId[i] = CharMotionCreate(playerState->charNo, lbl_1_data_38[i]);
        }
    }
    omSetTra(obj, lbl_1_data_CC[playerState->index], lbl_1_data_DC[playerState->index],
        lbl_1_data_EC[playerState->index]);
    i = playerState->index == 0 ? 6 : 0;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[i], lbl_1_data_80[i],
        lbl_1_data_74[i], lbl_1_data_6A[i] ? 0 : HU3D_MOTATTR_LOOP);
    playerState->motion = i;
    Hu3DModelLayerSet(obj->mdlId[0], 3);
    CharEffectLayerSet(3);
    Hu3DModelShadowSet(obj->mdlId[0]);
    if (playerState->index == 0) {
        modelIndex = 1;
        obj->mdlId[modelIndex] = Hu3DModelCreateData(DATANUM(DATA_m629, 7));
        Hu3DModelLayerSet(obj->mdlId[modelIndex], 3);
        Hu3DModelHookSet(obj->mdlId[0],
            CharModelItemHookGet(playerState->charNo, 2, 0), obj->mdlId[modelIndex]);
        Hu3DModelShadowSet(obj->mdlId[modelIndex]);
    }
    if (!lbl_1_bss_10 && playerState->charNo == 6) {
        Hu3DModelAmbSet(obj->mdlId[0], 1.4f, 1.4f, 1.4f);
    } else if (!lbl_1_bss_10 && playerState->charNo != 2 && playerState->charNo != 5) {
        Hu3DModelAmbSet(obj->mdlId[0], 1.2f, 1.2f, 1.2f);
    }
    /* Configure both model slots unconditionally; opponents leave slot one at HU3D_MODELID_NONE. */
    for (i = 0; i < 2; i++) {
        modelIndex = i == 0 ? 0 : 1;
        Hu3DModelLightBitReset(obj->mdlId[modelIndex], lbl_1_bss_5D0);
        if (lbl_1_bss_10) {
            lightId = Hu3DLLightCreateV(obj->mdlId[modelIndex], &lbl_1_data_134,
                &lbl_1_data_140, &lbl_1_data_14C);
        } else {
            lightId = Hu3DLLightCreateV(obj->mdlId[modelIndex], &lbl_1_data_134,
                &lbl_1_data_1B8, &lbl_1_data_1B4);
        }
        Hu3DLLightInfinitytSet(obj->mdlId[modelIndex], lightId);
        Hu3DLLightStaticSet(obj->mdlId[modelIndex], lightId, 1);
    }
    obj->objFunc = fn_1_9E8;
}

/* Arena draw hook: draws solo-piston air at impact steps 2-30 using cached frames 0-28;
 * cached frame 29 is not used here. */
void fn_1_4290(HU3D_MODEL *model, Mtx *matrix)
{
    s32 i;
    Mtx transform;
    /* Query the sequence mode; the returned value does not affect this draw hook. */
    s32 mode = MgSeqModeGet();

    Hu3DModelObjDrawInit();
    for (i = 0; i < 16; i++) {
        if (lbl_1_bss_69C[0][i].animationStep >= 2
            && lbl_1_bss_69C[0][i].animationStep <= 30) {
            PSMTXConcat(*matrix,
                lbl_1_bss_30[lbl_1_bss_69C[0][i].animationStep - 2], transform);
            Hu3DModelObjDraw(lbl_1_bss_5E0->mdlId[14],
                "m629_pistonhoos-air", transform);
        }
    }
}

/* Model draw hook that draws the arena sky using its mesh translation and scale;
 * mesh rotation is ignored. */
void fn_1_4368(HU3D_MODEL *model, Mtx *matrix)
{
    HSF_OBJECT *object;
    Mtx transform;
    Mtx scale;

    object = Hu3DModelObjPtrGet(lbl_1_bss_5E0->mdlId[1],
        lbl_1_bss_10 != 0 ? "m629_n_stage-sky" : "m629_stage-sky");
    PSMTXTrans(transform, object->mesh.base.pos.x,
        object->mesh.base.pos.y, object->mesh.base.pos.z);
    PSMTXScale(scale, object->mesh.base.scale.x,
        object->mesh.base.scale.y, object->mesh.base.scale.z);
    PSMTXConcat(transform, scale, transform);
    PSMTXConcat(*matrix, transform, transform);
    Hu3DModelObjDrawInit();
    Hu3DModelObjPtrDraw(lbl_1_bss_5E0->mdlId[1], object, transform);
}

/* Called for the arena object created by fn_1_58BC; builds arena models and installs draw/update
 * hooks. */
void fn_1_444C(OMOBJ *obj)
{
    s32 modelIndex;
    s32 i;
    HSF_OBJECT *sky;
    M629Player *playerWork[4];

    /* Read all four player-data pointers; arena setup does not use the collected array. */
    for (i = 0; i < 4; i++) {
        playerWork[i] = lbl_1_bss_5E4[i]->data;
    }
    modelIndex = 0;
    obj->mdlId[modelIndex] = Hu3DModelCreateData(lbl_1_bss_10
        ? DATANUM(DATA_m629, 1) : DATANUM(DATA_m629, 0));
    Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
    Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_MOTATTR_LOOP);
    modelIndex = 1;
    obj->mdlId[modelIndex] = Hu3DModelLink(obj->mdlId[0]);
    Hu3DModelLayerSet(obj->mdlId[modelIndex], 0);
    Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_ATTR_DISPOFF);
    modelIndex = 35;
    obj->mdlId[modelIndex] = Hu3DHookFuncCreate(fn_1_4368);
    Hu3DModelLayerSet(obj->mdlId[modelIndex], 0);
    sky = Hu3DModelObjPtrGet(obj->mdlId[0],
        lbl_1_bss_10 ? "m629_n_stage-sky" : "m629_stage-sky");
    /* Hide the sky in the main arena model by zeroing its scale; the linked copy retains
     * the mesh transform used by the separate sky draw hook. */
    sky->mesh.base.scale.x = sky->mesh.base.scale.y = sky->mesh.base.scale.z = 0.0f;
    for (i = 0; i < (lbl_1_bss_10 ? 1 : 2); i++) {
        modelIndex = 28 + i;
        obj->mdlId[modelIndex] = Hu3DModelCreateData(i == 0
            ? (lbl_1_bss_10 ? DATANUM(DATA_m629, 6) : DATANUM(DATA_m629, 4))
            : DATANUM(DATA_m629, 5));
        Hu3DModelLayerSet(obj->mdlId[modelIndex], 1);
        Hu3DModelPosSet(obj->mdlId[modelIndex], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_MOTATTR_LOOP);
    }
    modelIndex = 30;
    /* Both day and night use arena resource 29 for this model. */
    obj->mdlId[modelIndex] = Hu3DModelCreateData(lbl_1_bss_10
        ? DATANUM(DATA_m629, 29) : DATANUM(DATA_m629, 29));
    Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
    Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_MOTATTR_LOOP);
    if (!lbl_1_bss_10) {
        modelIndex = 31;
        obj->mdlId[modelIndex] = Hu3DModelCreateData(DATANUM(DATA_m629, 31));
        Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
        Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_MOTATTR_LOOP);
    }
    modelIndex = 2;
    obj->mdlId[modelIndex] = Hu3DModelCreateData(lbl_1_bss_10
        ? DATANUM(DATA_m629, 11) : DATANUM(DATA_m629, 9));
    Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
    Hu3DModelPosSet(obj->mdlId[modelIndex], 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_MOTATTR_PAUSE);
    Hu3DModelShadowMapObjSet(obj->mdlId[modelIndex], "m629_02-chair");
    for (i = 0; i < 8; i++) {
        modelIndex = i + 3;
        if (i == 0) {
            obj->mdlId[modelIndex] = Hu3DModelCreateData(DATANUM(DATA_m629, 12));
        } else {
            obj->mdlId[modelIndex] = Hu3DModelLink(obj->mdlId[3]);
        }
        Hu3DModelLayerSet(obj->mdlId[modelIndex], 3);
        Hu3DModelPosSet(obj->mdlId[modelIndex], -357.0f, 26.0f, 290.7662f);
        Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_ATTR_DISPOFF);
    }
    modelIndex = 27;
    obj->mdlId[modelIndex] = Hu3DModelCreateData(lbl_1_bss_10
        ? DATANUM(DATA_m629, 3) : DATANUM(DATA_m629, 2));
    Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
    Hu3DModelPosSet(obj->mdlId[modelIndex], 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_MOTATTR_LOOP);
    for (i = 0; i < 3; i++) {
        modelIndex = i + 11;
        if (i == 0) {
            obj->mdlId[modelIndex] = Hu3DModelCreateData(lbl_1_bss_10
                ? DATANUM(DATA_m629, 10) : DATANUM(DATA_m629, 8));
        } else {
            obj->mdlId[modelIndex] = Hu3DModelLink(obj->mdlId[11]);
        }
        Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
        Hu3DModelPosSet(obj->mdlId[modelIndex], lbl_1_data_CC[i + 1],
            lbl_1_data_DC[i + 1], lbl_1_data_EC[i + 1]);
        Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_MOTATTR_PAUSE);
        Hu3DModelShadowMapObjSet(obj->mdlId[modelIndex], "m629_01-cyl42");
    }
    for (i = 0; i < 4; i++) {
        s32 modelData[2][4] = {
            { DATANUM(DATA_m629, 13), DATANUM(DATA_m629, 14),
              DATANUM(DATA_m629, 15), DATANUM(DATA_m629, 16) },
            { DATANUM(DATA_m629, 17), DATANUM(DATA_m629, 18),
              DATANUM(DATA_m629, 19), DATANUM(DATA_m629, 20) }
        };
        modelIndex = i + 14;
        obj->mdlId[modelIndex] = Hu3DModelCreateData(modelData[lbl_1_bss_10][i]);
        Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
    }
    modelIndex = 18;
    obj->mdlId[modelIndex] = Hu3DModelLink(obj->mdlId[14]);
    Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
    Hu3DModelPosSet(obj->mdlId[modelIndex], 0.0f, 0.0f, 32768.0f);
    for (i = 0; i < 2; i++) {
        s32 modelData[2] = { DATANUM(DATA_m629, 21), DATANUM(DATA_m629, 22) };
        s32 sideCompletionData[2] = { DATANUM(DATA_m629, 23), DATANUM(DATA_m629, 24) };
        s32 position[2][3] = { { -357, 105, -200 }, { 357, 105, -200 } };
        modelIndex = i + 19;
        /* Both scenes use the same side-display resources, 21 for the solo side and
         * 22 for the team side. */
        obj->mdlId[modelIndex] = Hu3DModelCreateData(lbl_1_bss_10
            ? modelData[i] : modelData[i]);
        Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
        Hu3DModelPosSet(obj->mdlId[modelIndex], position[i][0], position[i][1], position[i][2]);
        modelIndex = i + 21;
        /* Both scenes use the same completion-display resources, 23 for the solo side
         * and 24 for the team side. */
        obj->mdlId[modelIndex] = Hu3DModelCreateData(lbl_1_bss_10
            ? sideCompletionData[i] : sideCompletionData[i]);
        Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
        Hu3DModelPosSet(obj->mdlId[modelIndex], position[i][0], position[i][1], position[i][2]);
        modelIndex = i + 23;
        if (i == 0) {
            obj->mdlId[modelIndex] = Hu3DModelCreateData(lbl_1_bss_10
                ? DATANUM(DATA_m629, 33) : DATANUM(DATA_m629, 32));
        } else {
            obj->mdlId[modelIndex] = Hu3DModelLink(obj->mdlId[23]);
        }
        Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
        Hu3DModelPosSet(obj->mdlId[modelIndex], position[i][0], position[i][1], position[i][2]);
    }
    for (i = 0; i < 2; i++) {
        s32 modelData[2][2] = {
            { DATANUM(DATA_m629, 25), DATANUM(DATA_m629, 26) },
            { DATANUM(DATA_m629, 27), DATANUM(DATA_m629, 28) }
        };
        modelIndex = i + 25;
        obj->mdlId[modelIndex] = Hu3DModelCreateData(modelData[lbl_1_bss_10][i]);
        Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_MOTATTR_LOOP | HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
    }
    for (i = 0; i < 2; i++) {
        s32 position[2][3] = { { -357, 105, -200 }, { 357, 105, -200 } };
        modelIndex = i + 32;
        /* Both day and night use arena resource 30 for these two side models. */
        obj->mdlId[modelIndex] = Hu3DModelCreateData(lbl_1_bss_10
            ? DATANUM(DATA_m629, 30) : DATANUM(DATA_m629, 30));
        Hu3DModelAttrSet(obj->mdlId[modelIndex], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
        Hu3DModelPosSet(obj->mdlId[modelIndex], position[i][0], position[i][1], position[i][2]);
    }
    modelIndex = 34;
    obj->mdlId[modelIndex] = Hu3DHookFuncCreate(fn_1_4290);
    Hu3DModelLayerSet(obj->mdlId[modelIndex], 2);
    memset(lbl_1_bss_69C, 0, sizeof(lbl_1_bss_69C));
    obj->objFunc = fn_1_1F90;
}

/* The sequence object invokes this setup callback to switch itself to its per-frame sequence
 * handler. */
void fn_1_53B8(OMOBJ *obj)
{
    obj->objFunc = fn_1_336C;
}

/* Called by _prolog to initialize the sequence, camera, lighting, sound queues, and object
 * manager. */
void fn_1_53C8(void)
{
    s32 i;
    s16 nightF;

    OSReport("******* M629 ObjectSetup *********\n");
    nightF = GwMgNightF;
    if (nightF == 1) {
        lbl_1_bss_10 = 1;
    }
    lbl_1_bss_5D8 = MgActorObjectSetup();
    for (i = 0; i < 8; i++) {
        lbl_1_bss_5FC[i].count = 0;
        lbl_1_bss_5FC[i].handle = -1;
    }
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 25.0f, 20.0f, 25000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 640, 480);
    CZoom = 2950.0f;
    Center.x = 0.0f;
    Center.y = 230.0f;
    Center.z = 0.0f;
    CRot.x = -13.85f;
    CRot.y = 0.0f;
    CRot.z = 0.0f;
    Hu3DLighInit();
    {
        HuVecF lightPos = { 0.0f, 5000.0f, 0.0f };
        HuVecF lightDir = { 0.2f, -1.0f, -0.4f };
        HuVecF nightDir = { 0.0f, -1.0f, -1.0f };
        HuVecF shadowPos;
        HuVecF shadowUp;
        HuVecF shadowTarget;
        GXColor color = { 255, 255, 255, 255 };
        GXColor nightColor = { 192, 192, 255, 255 };

        /* Use white for the global light in both scenes; only its direction changes at
         * night, and nightColor is unused. */
        lbl_1_bss_5D0 = Hu3DGLightCreateV(&lightPos,
            lbl_1_bss_10 ? &nightDir : &lightDir,
            lbl_1_bss_10 ? &color : &color);
        Hu3DGLightStaticSet(lbl_1_bss_5D0, TRUE);
        Hu3DGLightInfinitytSet(lbl_1_bss_5D0);
        if (lbl_1_bss_10) {
            shadowPos.x = 0.0f;
            shadowPos.y = 10000.0f;
            shadowPos.z = -1500.0f;
            shadowUp.x = 0.0f;
            shadowUp.y = 1.0f;
            shadowUp.z = 0.0f;
        } else {
            shadowPos.x = -1500.0f;
            shadowPos.y = 10000.0f;
            shadowPos.z = 1500.0f;
            shadowUp.x = 0.0f;
            shadowUp.y = 1.0f;
            shadowUp.z = 0.0f;
        }
        shadowTarget.x = 0.0f;
        shadowTarget.y = 0.0f;
        shadowTarget.z = 0.01f;
        Hu3DShadowMultiCreate(8.0f, 5000.0f, 13000.0f, 1);
        Hu3DShadowMultiTPLvlSet(0.5f, 1);
        Hu3DShadowMultiPosSet(&shadowPos, &shadowUp, &shadowTarget, 1);
        Hu3DShadowMultiColSet(0, 0, 0, 1);
    }
    lbl_1_bss_5D4 = -1;
    lbl_1_data_AC = -1;
    MgSeqCreate(&lbl_1_data_FC);
}

/* Sequence mode callback that creates the arena and four player actors. */
void fn_1_58BC(s16 mode, s16 frameNo)
{
    s32 i;
    OMOBJ *obj;

    lbl_1_bss_5E0 = omAddObjEx(lbl_1_bss_5D8, 101, 36, 0, 1, fn_1_444C);
    for (i = 0; i < 4; i++) {
        obj = lbl_1_bss_5E4[i] = omAddObjEx(lbl_1_bss_5D8,
            100, 2, 10, 0, fn_1_3C8C);
        obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M629Player), HU_MEMNUM_OVL);
        obj->work[0] = i;
    }
    lbl_1_bss_5F4 = omAddObjEx(lbl_1_bss_5D8, 32730, 0, 0, -1, omOutView);
    lbl_1_bss_5F4->work[0] = 1;
    lbl_1_bss_5F8 = omAddObjEx(lbl_1_bss_5D8, 102, 0, 0, -1, fn_1_53B8);
    MgSeqModeNext();
}

/* Exit mode callback that closes messages and audio before returning from the overlay. */
void fn_1_5A08(s16 mode, s16 frameNo)
{
    s32 i;
    M629Player *work;

    MgSeqModeNext();
    /* Read each player-data pointer without using it; this loop performs no player cleanup. */
    for (i = 0; i < 4; i++) {
        work = lbl_1_bss_5E4[i]->data;
    }
    if (lbl_1_bss_5D4 >= 0) {
        GameMesKill(lbl_1_bss_5D4);
    }
    GameMesClose();
    HuAudAllStop();
    omOvlReturnEx(lbl_1_data_C0, 1);
}
