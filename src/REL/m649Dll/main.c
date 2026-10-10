/* Sets up Stamp By Me, handles its sequence callbacks, and manages split-screen cameras. */
#include <math.h>

#include "game/object.h"

#include "game/hu3d.h"

#include "game/data.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/hsfex.h"

#include <string.h>

#include "game/audio.h"

#include "dolphin/types.h"

#include "game/gamework.h"

#include "game/flag.h"

#include "dolphin/gx.h"

#include "game/main.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"
#include "msm_stream.h"

#include "REL/m649Dll/module_types.h"

#include "include/dolphin/types.h"

#define M649_JINGLE_WINNER 67
#define M649_JINGLE_DRAW 69
#define M649_MOTION_BLEND 0.2f
#define M649_MOTION_OVERLAY_NONE (-1)
#define M649_MOTION_RESOURCE_NONE (-1)
#define M649_MOTION_RESOURCE_END (-2)

typedef struct M649AudioCameraConfig_s M649AudioCameraConfig;

typedef void (*M649CameraCallback)(OMOBJ *, u32, u32);

extern OMOBJMAN *lbl_1_bss_74;

extern s32 lbl_1_bss_70, lbl_1_bss_64, lbl_1_bss_60, lbl_1_bss_5C, lbl_1_bss_8;

extern s32 lbl_1_bss_3C[4];

extern float lbl_1_bss_4, lbl_1_bss_0;

extern MGSEQ_PARAM lbl_1_data_80;

extern Vec lbl_1_data_1C, lbl_1_data_28;

extern GXColor lbl_1_data_34;

extern OM_CAMERA_VIEW lbl_1_data_0;

extern M649AudioCameraConfig lbl_1_data_38[];

void fn_1_680(short mode, short state);

extern void fn_1_13D8(OMOBJMAN *manager);

extern void fn_1_14A0(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color);

extern void fn_1_1B88(OMOBJMAN *manager, u32 cameras, float fov, float near, float far,
                      float shadowRadius, OM_CAMERA_VIEW *view);

s32 fn_1_1210(s32 resource);

extern void fn_1_2BCC(u32 cameras, void *data);

void fn_1_C78(OMOBJ *object, u32 camera, u32 index);

extern void fn_1_2780(M649CameraCallback callback);

extern void fn_1_3250(OMOBJMAN *manager, M649AudioCameraConfig *config);

extern void fn_1_A220(OMOBJMAN *manager);

extern void fn_1_4F24(OMOBJMAN *manager);

extern void fn_1_6990(OMOBJMAN *manager);

extern void fn_1_8F58(OMOBJMAN *manager);

s32 fn_1_BD8(s16 mode, s16 frame);

extern s32 lbl_1_bss_6C, lbl_1_bss_68, lbl_1_bss_8;

extern s32 lbl_1_bss_8;

extern float lbl_1_bss_0, lbl_1_bss_4;

extern s32 fn_1_2C84(u32 cameras);

extern void fn_1_2590(u32 cameras, Vec *direction, Vec *target, float fov, float scale);

extern void fn_1_4ECC(Vec *from, Vec *to, float factor, Vec *result);

extern void fn_1_281C(u32 cameras, float x, float y, float width, float height);

extern s32 lbl_1_bss_64;

void fn_1_A600(void);

extern s32 lbl_1_bss_5C;

extern s32 fn_1_A644(s32 player);

extern s32 lbl_1_bss_2C[4], lbl_1_bss_C[4], lbl_1_bss_4C[4];

extern s32 lbl_1_data_A8[4];

extern s32 lbl_1_bss_60;

void fn_1_6A84(void);

void fn_1_5020(void);

void fn_1_A2A8(void);

extern void fn_1_4ECC(Vec *start, Vec *end, float factor, Vec *result);

extern s32 lbl_1_bss_2C[4];

extern u32 lbl_1_bss_1C[4];

extern s32 lbl_1_bss_C[4];

/* Initializes the sequence manager, shared camera, score display, props, and effects at startup. */
void fn_1_A0(void)
{
    s32 i, resource;
    lbl_1_bss_70 = 0;
    lbl_1_bss_64 = -1;
    lbl_1_bss_60 = 0;
    lbl_1_bss_5C = 0;
    lbl_1_bss_8 = 0;
    lbl_1_bss_4 = 640.0f;
    lbl_1_bss_0 = 480.0f;
    for (i = 0; i < 4; i++) {
        lbl_1_bss_3C[i] = 0;
    }
    lbl_1_bss_74 = omInitObjMan(120, 8192);
    omGameSysInit(lbl_1_bss_74);
    MgSeqCreate(&lbl_1_data_80);
    MgSeqModeHookAdd(MGSEQ_MODE_PREMAIN, fn_1_680);
    fn_1_13D8(lbl_1_bss_74);
    fn_1_14A0(lbl_1_bss_74, &lbl_1_data_1C, &lbl_1_data_28, &lbl_1_data_34);
    Hu3DBGColorSet(0, 0, 0);
    fn_1_1B88(lbl_1_bss_74, 15, 45.0f, 20.0f, 2000.0f, 0.0f, &lbl_1_data_0);
    resource = fn_1_1210(0);
    fn_1_2BCC(1, HuDataSelHeapReadNum(resource, HU_MEMNUM_OVL, HEAP_MODEL));
    fn_1_2BCC(2, HuDataSelHeapReadNum(resource, HU_MEMNUM_OVL, HEAP_MODEL));
    fn_1_2BCC(4, HuDataSelHeapReadNum(resource, HU_MEMNUM_OVL, HEAP_MODEL));
    fn_1_2BCC(8, HuDataSelHeapReadNum(resource, HU_MEMNUM_OVL, HEAP_MODEL));
    fn_1_2780(fn_1_C78);
    fn_1_3250(lbl_1_bss_74, lbl_1_data_38);
    fn_1_A220(lbl_1_bss_74);
    fn_1_4F24(lbl_1_bss_74);
    fn_1_6990(lbl_1_bss_74);
    fn_1_8F58(lbl_1_bss_74);
}

/* Advances to the next minigame sequence mode when this mode's callback runs. */
void fn_1_330(short mode, short frame) {
    if (!fn_1_BD8(mode, frame)) MgSeqModeNext();
}

/* Animates the opening camera split during fade-in, then requests the START sequence. */
void fn_1_36C(s16 mode, s16 frame)
{
    float amount;
    if (fn_1_BD8(mode, frame) == 0) {
        if (frame == 0) {
            lbl_1_bss_6C = 0;
        }
        switch (lbl_1_bss_6C) {
        case 0:
            lbl_1_bss_6C++;
            lbl_1_bss_68 = 0;
            MgSeqModeChangeOff();
            /* Begin counting the opening wait on this same sequence tick. */
        case 1:
            if (++lbl_1_bss_68 < 150) {
                break;
            }
            lbl_1_bss_6C++;
            lbl_1_bss_68 = 0;
            lbl_1_bss_8 = 1;
            /* Start the camera split transition as soon as the wait expires. */
        case 2:
            amount = 0.016666666666666666 * (float)lbl_1_bss_68;
            amount = sin(3.141592653589793 * (90.0 * amount) / 180.0);
            lbl_1_bss_4 = 642.0 - 322.0 * amount;
            lbl_1_bss_0 = 482.0 - 242.0 * amount;
            if (++lbl_1_bss_68 <= 60) {
                break;
            }
            lbl_1_bss_6C++;
            lbl_1_bss_68 = 0;
            lbl_1_bss_4 = 320.0f;
            lbl_1_bss_0 = 240.0f;
            /* The final sequence delay starts on the frame the split settles. */
        case 3:
            if (++lbl_1_bss_68 >= 30) {
                MgSeqModeNext();
            }
            break;
        }
    }
}

/* Starts the background music on the sequence callback's first eligible frame. */
void fn_1_620(s16 mode, s16 frame)
{
    if (fn_1_BD8(mode, frame) == 0 && lbl_1_bss_64 == -1) {
        lbl_1_bss_64 = HuAudBGMPlay(MSM_STREAM_MGMUS_26);
    }
}

/* Enables the score display and score accumulation when the pre-main sequence hook runs. */
void fn_1_680(short mode, short state)
{
    if (!fn_1_BD8(mode, state)) fn_1_A600();
}

/* Stops the route schedule after gameplay frame 1620, before the 30-second timer expires. */
void fn_1_6BC(s16 mode, s16 frame)
{
    if (fn_1_BD8(mode, frame) == 0 && frame > 1620) {
        lbl_1_bss_5C = 1;
    }
}

/* On FINISH frame zero, fades the background stream and stops the route schedule. */
void fn_1_718(s16 mode, s16 frame)
{
    if ((fn_1_BD8(mode, frame) == 0) && (frame == 0)) {
        if (lbl_1_bss_64 != -1) {
            HuAudSStreamFadeOut(lbl_1_bss_64, 100);
            lbl_1_bss_64 = -1;
        }
        lbl_1_bss_5C = 1;
    }
}

/* PREWIN writes Decathlon scores or submits score ranks as coin bonuses; practice mode
* ignores the bonus writes. */
void fn_1_7AC(s16 mode, s16 frame)
{
    s32 i, value, j, rank, maximum = 0, next;
    s32 scores[4], players[4];
    if (fn_1_BD8(mode, frame) == 0) {
        if (_CheckFlag(FLAG_INST_DECA)) {
            for (i = 0; i < 4; i++) {
                GWMgScoreSet(i, fn_1_A644(i));
            }
            MgSeqModeSet(MGSEQ_MODE_FADEOUT);
            return;
        }
        if (frame == 0) {
            for (i = 0; i < 4; i++) {
                value = fn_1_A644(i);
                if (value > maximum) {
                    maximum = value;
                }
                scores[i] = value;
                players[i] = i;
            }
            for (i = 0; i < 4; i++) {
                lbl_1_bss_2C[i] = -1;
                if (maximum == scores[i] && maximum != 0) {
                    lbl_1_bss_2C[i] = lbl_1_bss_C[i];
                }
            }
            if (maximum == 0) {
                for (i = 0; i < 4; i++) {
                    lbl_1_bss_4C[i] = 3;
                }
            } else {
                for (rank = 0; rank < 4; rank++) {
                    for (i = 0; i < 3; i++) {
                        for (j = i + 1; j < 4; j++) {
                            if (scores[i] < scores[j]) {
                                value = scores[i];
                                scores[i] = scores[j];
                                scores[j] = value;
                                value = players[i];
                                players[i] = players[j];
                                players[j] = value;
                            }
                        }
                    }
                }
                rank = 0;
                value = scores[0];
                for (i = 0; i < 4; i++) {
                    next = scores[i];
                    if (next != value) {
                        rank++;
                    }
                    value = next;
                    lbl_1_bss_4C[players[i]] = rank;
                }
            }
            /* Submit rank itself as the coin bonus: 0 for the top score, larger for lower ranks;
            * an all-zero round uses 3. Practice mode ignores these writes. */
            for (i = 0; i < 4; i++) {
                GWMgCoinBonusSet(i, lbl_1_bss_4C[i]);
            }
        }
        if (lbl_1_bss_60 == 0 && frame >= 120) {
            MgSeqModeNext();
            return;
        }
    }
}

/* Plays the results jingle on frame zero, choosing it from whether anyone scored. */
void fn_1_AD4(s16 mode, s16 frame)
{
    s32 player, hasScore;
    if (fn_1_BD8(mode, frame) == 0 && frame == 0) {
        hasScore = 0;
        for (player = 0; player < 4; player++) {
            if (fn_1_A644(player) != 0) {
                hasScore = 1;
            }
        }
        if (hasScore) {
            HuAudJinglePlay(M649_JINGLE_WINNER);
        } else {
            HuAudJinglePlay(M649_JINGLE_DRAW);
        }
    }
}

/* Does no additional work when this sequence callback runs. */
void fn_1_B78(short mode, short frame)
{
    if (fn_1_BD8(mode, frame) != 0) {
        return;
    }
}

/* Called at CLOSE or EXIT to release character models and run the empty prop and score cleanup
 * hooks. */
void fn_1_BB0(s16 mode, s16 frameNo)
{
    fn_1_6A84();
    fn_1_5020();
    fn_1_A2A8();
}

s32 fn_1_BD8(s16 mode, s16 frame)
{
    return 0;
}

/* Interpolates the center, rotation, and zoom between two camera views. */
void fn_1_BE0(OM_CAMERA_VIEW *start, OM_CAMERA_VIEW *end, OM_CAMERA_VIEW *result, float factor)
{
    fn_1_4ECC(&start->center, &end->center, factor, &result->center);
    fn_1_4ECC(&start->rot, &end->rot, factor, &result->rot);
    result->zoom = start->zoom + factor * (end->zoom - start->zoom);
}

/* Updates one camera each frame, easing into its saved view and setting its viewport. */
void fn_1_C78(OMOBJ *object, u32 camera, u32 index)
{
    M649CameraEntry *entry;
    M649CameraWork *work = object->data;
    OM_CAMERA_VIEW *view, *previous;
    float x, y, factor, width, height;
    entry = &work->entries[index];
    view = &entry->view;
    previous = &entry->previousView;
    x = 0.0f;
    y = 0.0f;
    width = 640.0f;
    height = 480.0f;
    /* Create the selected cameras' shadows once with a fixed radius of 500 world units,
     * even when shadowRadius is zero. */
    if (work->shadowInitialized == 0) {
        work->shadowInitialized++;
        fn_1_2590(work->cameras, &work->shadowDirection, &entry->target, work->shadowFov, 500.0f);
        /* This color setter changes only camera 0; the other selected shadows keep their default
         * RGB. */
        Hu3DShadowColSet(work->shadowColor.r, work->shadowColor.g, work->shadowColor.b);
    }
    switch (entry->phase) {
    case 0:
        if (fn_1_2C84(camera)) { break; }
        if (index != 0) {
            entry->phase = -1;
            break;
        }
        entry->phase++;
        entry->timer = 0;
        *view = lbl_1_data_0;
        /* The opening view is ready; the next phase waits for the split-screen cue. */
    case 1:
        if (!lbl_1_bss_8) { break; }
        entry->phase++;
        entry->timer = 0;
    case 2:
        factor = 0.016666666666666666 * (float)entry->timer;
        factor = sin(3.141592653589793 * (90.0 * factor) / 180.0);
        fn_1_4ECC(&lbl_1_data_0.center, &previous->center, factor, &view->center);
        fn_1_4ECC(&lbl_1_data_0.rot, &previous->rot, factor, &view->rot);
        view->zoom = lbl_1_data_0.zoom + factor * (previous->zoom - lbl_1_data_0.zoom);
        if (++entry->timer >= 60) {
            entry->phase++;
            entry->timer = 0;
            *view = *previous;
        }
        break;
    }
    switch (camera) {
    case 1:
        x = 0.0f;
        y = 0.0f;
        width = lbl_1_bss_4 - 2.0;
        height = lbl_1_bss_0 - 2.0;
        break;
    case 2:
        x = (s32)(2.0 + lbl_1_bss_4);
        y = 0.0f;
        width = 640.0 - x;
        height = lbl_1_bss_0 - 2.0;
        break;
    case 4:
        x = 0.0f;
        y = (s32)(2.0 + lbl_1_bss_0);
        width = lbl_1_bss_4 - 2.0;
        height = 480.0 - y;
        break;
    case 8:
        x = (s32)(2.0 + lbl_1_bss_4);
        y = (s32)(2.0 + lbl_1_bss_0);
        width = 640.0 - x;
        height = 480.0 - y;
        break;
    }
    fn_1_281C(camera, x, y, width, height);
}

s32 fn_1_1210(s32 resource)
{
    return resource + DATA_m649;
}

s32 fn_1_1218(s32 cameraIndex) {
    return *(s32 *) ((u8 *) &lbl_1_data_A8 + ((cameraIndex & 3) * 4));
}

/* Reports whether the route schedule has been stopped near the end of gameplay. */
extern s32 fn_1_1234(void) {
    return lbl_1_bss_5C;
}

/* Computer-player timing calls this to read the sequence timer; the void wrapper discards its
 * return value. */
void fn_1_1244(void) {
    MgSeqTimerValueGet();
}

s32 fn_1_1264(s32 index) { return lbl_1_bss_2C[index]; }

/* Returns 1 for a winning registered result, -1 for another, or 0 when there are no winners;
 * an unregistered result ID falls through without a return value. */
s32 fn_1_127C(u32 resultId)
{
    s32 player;
    for (player = 0; player < 4; player++) {
        if (lbl_1_bss_2C[player] >= 0) {
            break;
        }
    }
    if (player >= 4) {
        return 0;
    }
    for (player = 0; player < 4; player++) {
        if (resultId == lbl_1_bss_1C[player]) {
            return lbl_1_bss_2C[player] >= 0 ? 1 : -1;
        }
    }
}

/* Player setup stores the result identifier and character ID used to identify winners. */
void fn_1_1338(s32 player, u32 resultId, u32 character)
{
    player &= 3;
    lbl_1_bss_1C[player] = resultId;
    lbl_1_bss_C[player] = character;
}

/* Marks a player's result as ready for the results screen. */
void fn_1_1368(s32 player)
{
    player &= 3;
    lbl_1_bss_3C[player] = 1;
}

/* Returns whether all four player results have been submitted. */
s32 fn_1_1388(void)
{
    s32 player;
    for (player = 0; player < 4; player++) {
        if (lbl_1_bss_3C[player] == 0) {
            return 0;
        }
    }
    return 1;
}
#include <math.h>
#include "game/object.h"
#include "game/mg/seqman.h"

struct M649AudioCameraConfig_s {
    u32 cameras;
    s32 panMin, panMax;
    s32 volumeMin, volumeMiddle, volumeMax;
    float distanceFar, distanceMiddle, distanceNear;
};

typedef struct M649PlayerMotionParam_s {
    u16 motion;
    s16 overlay;
    float blend;
    u32 attr;
} M649PlayerMotionParam;

typedef struct M649PropMotionParam_s {
    s32 resource;
    float blend;
    u32 attr;
} M649PropMotionParam;

typedef struct M649PropParam_s {
    s32 count, linkModel, type;
    s16 firstId, layer;
    u32 attr;
    s32 shadow, modelResource;
    M649PropMotionParam *motions;
    Vec *position;
    float rotationY;
    OMOBJ_FUNC update;
} M649PropParam;

typedef struct M649PathView_s {
    s32 time;
    s32 points[7];
} M649PathView;

typedef struct M649ComputerParam_s {
    float firstDelay, delay, randomDelay, error;
    s32 targetScore;
} M649ComputerParam;

extern void fn_1_330(s16 mode, s16 frame);
extern void fn_1_36C(s16 mode, s16 frame);
extern void fn_1_620(s16 mode, s16 frame);
extern void fn_1_6BC(s16 mode, s16 frame);
extern void fn_1_718(s16 mode, s16 frame);
extern void fn_1_7AC(s16 mode, s16 frame);
extern void fn_1_AD4(s16 mode, s16 frame);
extern void fn_1_B78(s16 mode, s16 frame);
extern void fn_1_BB0(s16 mode, s16 frame);
extern void fn_1_728C(OMOBJ *object);
extern void fn_1_7288(OMOBJ *object);
extern void fn_1_7300(OMOBJ *object);
extern void fn_1_7C94(OMOBJ *object);
extern void fn_1_7DD8(OMOBJ *object);
extern void fn_1_7FD4(OMOBJ *object);
extern void fn_1_80B4(OMOBJ *object);
extern void fn_1_8278(OMOBJ *object);

OM_CAMERA_VIEW lbl_1_data_0 = {{0.0f, 0.0f, 0.0f}, {-60.0f, 0.0f, 0.0f}, 1000.0f};
Vec lbl_1_data_1C = {0.0f, 800.0f, 0.0f};
Vec lbl_1_data_28 = {0.0f, 0.0f, 0.0f};
GXColor lbl_1_data_34 = {255, 255, 255, 255};
M649AudioCameraConfig lbl_1_data_38[2] = { { 65535, 32, 96, 32, 64, 110, 900.0f, 700.0f, 500.0f },
                                           { 0, 0, 0, 0, 0, 0, 0.0f, 0.0f, 0.0f } };
MGSEQ_PARAM lbl_1_data_80 = {
    30, 0, fn_1_330, fn_1_36C, fn_1_620, fn_1_6BC, fn_1_718, fn_1_7AC, fn_1_AD4, fn_1_B78, fn_1_BB0
};
s32 lbl_1_data_A8[4] = {1, 2, 4, 8};
Vec lbl_1_data_B8 = {0.0f, -1.0f, 0.0f};
GXColor lbl_1_data_C4 = {0, 0, 0, 0};
s32 lbl_1_data_C8[6] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1), DATANUM(DATA_mario, 90),
    DATANUM(DATA_mario, 185), DATANUM(DATA_mariomot, 6), DATANUM(DATA_mariomot, 40)
};
M649PlayerMotionParam lbl_1_data_E0[6] = {
    {0, M649_MOTION_OVERLAY_NONE, M649_MOTION_BLEND, HU3D_MOTATTR_LOOP},
    {1, M649_MOTION_OVERLAY_NONE, M649_MOTION_BLEND, HU3D_MOTATTR_LOOP},
    {2, M649_MOTION_OVERLAY_NONE, M649_MOTION_BLEND, HU3D_MOTATTR_LOOP},
    {3, M649_MOTION_OVERLAY_NONE, M649_MOTION_BLEND, HU3D_MOTATTR_NONE},
    {4, M649_MOTION_OVERLAY_NONE, M649_MOTION_BLEND, HU3D_MOTATTR_NONE},
    {5, M649_MOTION_OVERLAY_NONE, M649_MOTION_BLEND, HU3D_MOTATTR_NONE}
};
Vec lbl_1_data_128[4] = {
    { 0.0f, 105.0f, 0.0f }, { 0.0f, 105.0f, 0.0f }, { 0.0f, 105.0f, 0.0f }, { 0.0f, 105.0f, 0.0f }
};
u8 lbl_1_data_158[28] = {0};
M649PlayerConfig lbl_1_data_174 = {
    50.0f,
    200.0f,
    {65, 58, 170, 171},
    0, 1,
    {64, 0, 0, 0, 0, 0, 0, 1, 63, 192, 0, 0},
    2,
    {0, 0, 0, 1, 64, 0, 0, 0, 0, 0, 0, 1, 63, 192, 0, 0},
    3, 4, 5
};
s32 lbl_1_data_1B4[4] = {10, 11, 12, 13};
u8 lbl_1_data_1C4[96] = {
    66, 112, 0, 0, 66, 112, 0, 0, 66, 16, 0, 0, 63, 115, 51, 51, 0, 0, 0, 7, 66, 16, 0, 0,
    66, 40, 0, 0, 65, 192, 0, 0, 63, 76, 204, 205, 0, 0, 0, 12, 65, 192, 0, 0, 65, 144, 0,
    0, 65, 64, 0, 0, 63, 25, 153, 154, 0, 0, 0, 19, 65, 144, 0, 0, 65, 64, 0, 0, 64, 192,
    0, 0, 63, 0, 0, 0, 0, 0, 0, 23
};
M649PropMotionParam lbl_1_data_224[3] = { { M649_MOTION_RESOURCE_NONE, M649_MOTION_BLEND,
                                            HU3D_MOTATTR_LOOP },
                                          { 2, M649_MOTION_BLEND, HU3D_MOTATTR_LOOP },
                                          { M649_MOTION_RESOURCE_END, 0.0f, HU3D_MOTATTR_NONE } };
M649PropParam lbl_1_data_248[12] = {
    {1, 0, 0, 0, 2, HU3D_MOTATTR_LOOP, 0, 4, 0, 0, 0.0f, fn_1_728C},
    {1, 0, 0, 1, 1, HU3D_MOTATTR_LOOP, 2, 5, 0, 0, 0.0f, fn_1_7288},
    {16, 0, 1, 0, 2, 0, 0, -1, 0, 0, 0.0f, fn_1_7300},
    {1, 0, 2, 0, 2, HU3D_MOTATTR_LOOP, 0, 6, 0, 0, 0.0f, fn_1_7C94},
    {1, 0, 2, 1, 2, HU3D_MOTATTR_LOOP, 0, 7, 0, 0, 0.0f, fn_1_7C94},
    {2, 0, 3, 0, 2, 0, 0, -1, 0, 0, 0.0f, fn_1_7DD8},
    {12, 1, 4, 0, 2, HU3D_MOTATTR_LOOP, 0, 8, 0, 0, 0.0f, fn_1_7FD4},
    {12, 1, 5, 0, 2, HU3D_MOTATTR_LOOP, 0, 9, 0, 0, 0.0f, fn_1_7FD4},
    {8, 1, 6, 0, 5, 0, 0, 15, 0, 0, 0.0f, fn_1_80B4},
    {8, 1, 7, 0, 5, 0, 0, 16, 0, 0, 0.0f, fn_1_80B4},
    {4, 0, 8, 0, 4, 0, 0, 1, &lbl_1_data_224[0], 0, 0.0f, fn_1_8278},
    {-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0f, 0}
};

u32 lbl_1_bss_5A8;
u32 lbl_1_bss_5A4;
u32 lbl_1_bss_5A0;
u32 lbl_1_bss_59C;
OMOBJ *lbl_1_bss_39C[128];
u32 lbl_1_bss_398;
OMOBJ *lbl_1_bss_368[12];
OMOBJ *lbl_1_bss_338[12];
u32 lbl_1_bss_334;
OMOBJ *lbl_1_bss_324[4];
void *lbl_1_bss_314[4];
u32 lbl_1_bss_310;
u32 lbl_1_bss_308[2];
u32 lbl_1_bss_304;
u32 lbl_1_bss_300;
u32 lbl_1_bss_2FC;
u32 lbl_1_bss_2F8;
s32 lbl_1_bss_1B8[80];
s32 lbl_1_bss_78[80];
OMOBJMAN *lbl_1_bss_74;
s32 lbl_1_bss_70;
s32 lbl_1_bss_6C;
s32 lbl_1_bss_68;
s32 lbl_1_bss_64;
s32 lbl_1_bss_60;
s32 lbl_1_bss_5C;
s32 lbl_1_bss_4C[4];
s32 lbl_1_bss_3C[4];
s32 lbl_1_bss_2C[4];
u32 lbl_1_bss_1C[4];
s32 lbl_1_bss_C[4];
s32 lbl_1_bss_8;
f32 lbl_1_bss_4;
f32 lbl_1_bss_0;
