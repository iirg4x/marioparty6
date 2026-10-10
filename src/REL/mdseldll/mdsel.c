/* Mode-selection overlay: builds the six-choice screen, handles player input, and enters the
 * selected mode. */
#include "datadir_enum.h"
#include "messdir_enum.h"

#include "dolphin/mic.h"
#include "dolphin/os.h"

#include "game/armem.h"
#include "game/charman.h"
#include "game/flag.h"
#include "game/gamework.h"
#include "game/mgdata.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/window.h"
#include "msm.h"
#include "msm_se.h"
#include "msm_stream.h"

#define sind(x) sin((M_PI * (x)) / 180.0)

enum {
    MDSEL_DLL_RETURN_REPORT_SIZE = 60,
    MDSEL_ITEM_HOOK_NAME_SIZE = 19,
    MDSEL_OBJECT_PRIORITY = 4096,
    MDSEL_OBJECT_CAPACITY = 16,
    MDSEL_LARGE_OBJECT_CAPACITY = 64,
    MDSEL_OBJECT_MANAGER_PRIORITY = 8192,
    MDSEL_MAIN_PROCESS_PRIORITY = 12288,
    MDSEL_MAIN_PROCESS_STACK_SIZE = 12288,
    MDSEL_WINDOW_WIDTH = 544,
    MDSEL_WINDOW_HEADER_HEIGHT = 42,
    MDSEL_WINDOW_MENU_HEIGHT = 68,
};

typedef void (*VoidFunc)(void);
typedef void (*MCResponseCallback)(u16 *response);

typedef struct Lbl1Bss1D4Entry {
    HU3D_MODELID modelId; /* Model whose sound is tracked; NONE marks a free slot. */
    s16 reserved; /* Alignment field. */
    s32 fxHandle; /* Active sound handle, or -1 when no sound is tracked. */
} LBL_1_BSS_1D4_ENTRY;

typedef struct Lbl1Data8Entry {
    s16 groupNo; /* Sprite group that receives this marker. */
    s16 memberNo; /* Member slot in the sprite group. */
    s16 animNo; /* Index of the loaded marker animation. */
    s16 priority; /* Sprite draw priority. */
    s16 bank; /* Initial animation bank. */
    s16 reserved; /* Alignment field. */
    HuVec2f pos; /* Initial marker position in screen units. */
    HuVec2f scale; /* Initial horizontal and vertical scale. */
    float zRot; /* Initial rotation in degrees. */
} LBL_1_DATA_8_ENTRY;

/* Elapsed time, duration, and control points for the menu's model transition paths. */
typedef struct MdselBezierWork {
    u8 reservedHeader[4]; /* Preserved bytes with no use in this file. */
    float time; /* Elapsed menu animation time in frames. */
    float duration; /* Total menu animation time in frames. */
    HuVecF control[3]; /* Start, bend, and end positions in model space. */
    u8 reservedTail[88]; /* Preserved bytes with no use in this file. */
} MDSEL_BEZIER_WORK;

/* Per-model movement and animation state for the first decorative choice group. */
typedef struct Lbl1Bss8ACEntry {
    s16 active; /* Zero hides this moving model; nonzero advances it. */
    s16 reservedState; /* Preserved state with no use in this file. */
    float phase; /* Current bob phase in update frames. */
    float phaseDuration; /* Frames in one bob cycle. */
    HuVecF position; /* Model position in world units. */
    float horizontalSpeed; /* Horizontal movement in world units per update. */
    float bobAmplitude; /* Vertical bob range in world units. */
    float rotationAmplitude; /* Current rotation range in degrees. */
    float rotationPhase; /* Current rotation phase in update frames. */
    float rotationDuration; /* Frames in one rotation cycle. */
    u8 reservedMotion[4]; /* Preserved bytes with no use in this file. */
    float soundCountdown; /* Decremented before the sound check; an initial zero skips the sound. */
    u8 reservedAudio[12]; /* Preserved bytes with no use in this file. */
    s16 movementState; /* 0 chooses a move; 100, 200, 300, and 400 are animation states. */
    s16 movementVariant; /* Selects the duration and follow-up state for a move. */
    s16 stateTimer; /* Frames since the last intensity decrease. */
    s16 intensity; /* Sound intensity, clamped to 0 through 30. */
    float stateElapsed; /* Frames elapsed in the current movement state. */
    float stateDuration; /* Frames assigned to the current movement state. */
    float targetYaw; /* Target model heading in degrees. */
    u8 reservedHeading[4]; /* Preserved bytes with no use in this file. */
    HuVecF startPosition; /* Current movement state's start in world units. */
    HuVecF endPosition; /* Current movement state's destination in world units. */
    u8 reservedTail[24]; /* Preserved bytes with no use in this file. */
} LBL_1_BSS_8AC_ENTRY;

/* Per-model timing, endpoints, and direction for the second decorative choice group. */
typedef struct Lbl1Bss24CEntry {
    s16 direction; /* 0 moves left to right; 1 moves right to left. */
    s16 reservedDirection; /* Preserved state with no use in this file. */
    float elapsed; /* Frames elapsed on the model's crossing path. */
    float duration; /* Total frames for the model's crossing path. */
    HuVecF startPosition; /* Crossing start in world units. */
    HuVecF endPosition; /* Crossing destination in world units. */
    u8 reservedPath[12]; /* Preserved bytes with no use in this file. */
    float targetYaw; /* Target heading in degrees for the guide model. */
    float lowerYVariant; /* 1 lowers the guide model to y=-400; 0 uses its normal height. */
    u8 reservedTail[80]; /* Preserved bytes with no use in this file. */
} LBL_1_BSS_24C_ENTRY;

/* Microphone listener response data read by the recognition callback. */
typedef struct MdselMicResponse {
    s16 status; /* Zero is successful; the callback also requires count > 0 before reading
                 * result. */
    s16 reservedStatus; /* Preserved response field with no use in this file. */
    s16 count; /* Number of recognized word IDs in result. */
    s16 reservedCount; /* Preserved alignment field. */
    s16 *result; /* Recognized mode-choice word IDs. */
    s32 reservedTail; /* Preserved response field with no use in this file. */
} MDSEL_MIC_RESPONSE;

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern void HuDataDirCloseAll(void);
extern void HuAudFadeOut(s32 speed);
extern int HuAudFXPlay(int seId);
extern int HuAudFXPlayPan(int seId, int pan);
extern int HuAudFXPlayVolPan(int seId, int volume, int pan);
extern void HuAudFXStop(int seNo);
extern int HuAudSStreamPlay(s16 streamId);
extern void HuAudSStreamFadeOut(int streamNo, s32 speed);
extern s32 HuMCInit(s16 mountResult);
extern s32 HuMCMount(s32 chan);
extern void HuMCClose(void);
extern void HuMCMicSet(s32 flag);
extern void HuMCListenerKill(void);
extern void HuMCContextKill(s16 context);
extern s16 HuMCContextCreate(char *path);
extern void HuMCListenerCreate(
    s16 context, MCResponseCallback callback, u8 property);
extern s32 frandmod(s32 modulus);
extern s32 rand8(void);

extern int lbl_1_bss_0;
extern int lbl_1_bss_4;
extern OMOBJMAN *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_14;
extern OMOBJ *lbl_1_bss_18;
extern OMOBJ *lbl_1_bss_1C;
extern OMOBJ *lbl_1_bss_20[2];
extern void *lbl_1_bss_28;
extern float lbl_1_bss_2C;
extern OMOBJ *lbl_1_bss_30;
extern OMOBJ *lbl_1_bss_34;
extern s16 lbl_1_bss_38;
extern s16 lbl_1_bss_3A;
extern s16 lbl_1_bss_3C;
extern s16 lbl_1_bss_3E;
extern float lbl_1_bss_40;
extern HU3D_MODELID lbl_1_bss_44[6][5];
extern ANIMDATA *lbl_1_bss_80[5];
extern HuVecF lbl_1_bss_94;
extern MDSEL_BEZIER_WORK lbl_1_bss_A0;
extern MDSEL_BEZIER_WORK lbl_1_bss_128;
extern s16 lbl_1_bss_1B0[6][3];
extern LBL_1_BSS_24C_ENTRY lbl_1_bss_24C[12];
extern LBL_1_BSS_1D4_ENTRY lbl_1_bss_1D4[15];
extern LBL_1_BSS_8AC_ENTRY lbl_1_bss_8AC[2];
extern LBL_1_BSS_8AC_ENTRY lbl_1_bss_9BC[30];
extern HuVecF lbl_1_bss_19AC[4];
extern s32 lbl_1_bss_19DC[4];
extern s16 lbl_1_bss_19EC[30];
extern s32 lbl_1_bss_1A28;
extern s32 lbl_1_bss_1A2C;
extern s16 lbl_1_bss_1A30[3];
extern HUSPRID lbl_1_bss_1A36[1];
extern HUSPR_GROUPID lbl_1_bss_1A38[1];
extern ANIMDATA *lbl_1_bss_1A3C[1];
extern HUWINID lbl_1_bss_1A40[4];
extern HU3D_LIGHTID lbl_1_bss_1A48[2];
extern s16 lbl_1_bss_1A4C;

extern u32 lbl_1_data_0[1];
extern s16 lbl_1_data_4[2];
extern LBL_1_DATA_8_ENTRY lbl_1_data_8[1];
extern HuVecF lbl_1_data_28[6];
extern s16 lbl_1_data_70[6];
extern s32 lbl_1_data_7C;
extern char lbl_1_data_80[];
extern char lbl_1_data_A1[];
extern char lbl_1_data_CF[];
extern char lbl_1_data_E0[];
extern char lbl_1_data_F2[];
extern char lbl_1_data_104[];
extern char lbl_1_data_112[];
extern char lbl_1_data_114[];
extern s16 lbl_1_data_150[2];
extern s32 lbl_1_data_154[2];
extern s16 lbl_1_data_15C;
extern char lbl_1_data_15E[];
extern char lbl_1_data_169[];
extern char lbl_1_data_1A8[];
extern char lbl_1_data_1E7[];
extern char lbl_1_data_223[];
extern char lbl_1_data_25E[];
extern char lbl_1_data_275[];
extern char lbl_1_data_27F[];
extern char lbl_1_data_2BD[];
extern s16 lbl_1_data_2D0;
extern char lbl_1_data_2D2[];
extern char lbl_1_data_308[];
extern char lbl_1_data_333[];
extern u32 lbl_1_data_35C[5];

void fn_1_A5D4(void);
void fn_1_0(HUWINID winId, u32 mess, s16 index);
void fn_1_1370(OMOBJ *obj);
void fn_1_5614(OMOBJ *obj);
void fn_1_607C(OMOBJ *obj);
void fn_1_651C(MDSEL_MIC_RESPONSE *response);
void fn_1_EF48(HU3D_MODEL *model, HU3D_PARTICLE *emitter, Mtx matrix);
void fn_1_F790(void);
void fn_1_2A78(s16 layerNo);
void fn_1_2828(void);
void fn_1_346C(OMOBJ *obj);
void fn_1_485C(void);
void fn_1_4CF4(OMOBJ *obj);
void fn_1_5290(void);
void fn_1_5BF0(void);
void fn_1_5EA4(s16 index);
s16 fn_1_5F60(HU3D_MODELID modelId, s32 fxNo);
void fn_1_70BC(OMOBJ *obj);
void fn_1_7868(OMOBJ *obj);
void fn_1_809C(OMOBJ *obj);
void fn_1_FA9C(s16 groupNo, HuVecF *pos, s16 mode, s16 colorNo);
void fn_1_92BC(OMOBJ *obj);
void fn_1_FDF8(HuVecF *pos, s32 fxNo);
void fn_1_FEC0(HU3D_MODELID modelId, s32 fxNo, s16 panRange, s16 volume);
void fn_1_FA18(void);
void fn_1_9910(void);
s16 fn_1_BAB4(void);
void fn_1_E1FC(void);
s16 fn_1_E7B0(void);
void ObjectSetup(void);

/* Window callback registered by fn_1_1DDC; plays the guide voice when a new main-menu entry
 * appears. */
void fn_1_0(HUWINID winId, u32 mess, s16 index)
{
    s32 messNum[3] = {
        MESSNUM(MESS_ALL_MAIN_MENU, 0),
        MESSNUM(MESS_ALL_MAIN_MENU, 1),
        -1,
    };
    s32 fxNum[16] = {
        MSM_SE_GUIDE_25, MSM_SE_GUIDE_26, MSM_SE_GUIDE_27,
        MSM_SE_GUIDE_28, MSM_SE_GUIDE_29, MSM_SE_GUIDE_30,
        MSM_SE_GUIDE_31, -1,
        MSM_SE_GUIDE_17, MSM_SE_GUIDE_18, MSM_SE_GUIDE_19,
        MSM_SE_GUIDE_20, MSM_SE_GUIDE_21, MSM_SE_GUIDE_22,
        MSM_SE_GUIDE_23, -1,
    };
    s16 i;

    index--;
    OSReport(lbl_1_data_80, index);
    if (lbl_1_data_7C != mess) {
        lbl_1_data_7C = mess;
        for (i = 0;; i++) {
            if (messNum[i] == -1) {
                HuAudFXPlay(fxNum[index]);
                break;
            }
            if (mess == messNum[i]) {
                if (index >= 8) {
                    HuAudFXPlayPan(
                        fxNum[index], (MSM_PAN_CENTER + MSM_PAN_RIGHT) / 2);
                } else {
                    HuAudFXPlayPan(
                        fxNum[index], (MSM_PAN_LEFT + MSM_PAN_CENTER) / 2);
                }
                break;
            }
        }
    }
}

/* Returns from the mode-selection screen to the overlay chosen in the current menu state. */
void fn_1_1B4(void)
{
    OMOVLHIS *history = omOvlHisGet(0);

    omOvlHisChg(0, history->ovl, 1, lbl_1_bss_1A30[0]);
    switch (lbl_1_bss_1A30[0]) {
        case 0:
            omOvlCall(DLL_mdpartydll, 0, 0);
            break;
        case 1:
            omOvlCall(DLL_mdsingdll, 0, 0);
            break;
        case 2:
            omOvlCall(DLL_mdminidll, 0, 0);
            break;
        case 3:
            omOvlCall(DLL_mdmicdll, 0, 0);
            break;
        case 4:
            omOvlCall(DLL_optiondll, 0, 0);
            break;
        case 5:
            omOvlCall(DLL_mdbankdll, 0, 0);
            break;
    }
}

/* Closes character data and frees DATA_board, DATA_board_us, and DATA_capsule, then reports heap
 * use. */
void fn_1_2CC(void)
{
    CharDataClose(-1);
    HuARDirFree(DATA_board);
    HuARDirFree(DATA_board_us);
    HuARDirFree(DATA_capsule);
    OSReport(lbl_1_data_A1);
    OSReport(lbl_1_data_CF, DATA_effect >> 16);
    OSReport(lbl_1_data_E0, DATA_gamemes >> 16);
    OSReport(lbl_1_data_F2, DATA_mgconst >> 16);
    OSReport(lbl_1_data_104, DATA_win >> 16);
    HuAMemDump();
    OSReport(lbl_1_data_112);
}

/* Reports the remaining mode-selection archive allocations when the overlay is leaving. */
void fn_1_37C(void)
{
    OSReport(lbl_1_data_114);
    OSReport(lbl_1_data_CF, DATA_effect >> 16);
    OSReport(lbl_1_data_E0, DATA_gamemes >> 16);
    OSReport(lbl_1_data_F2, DATA_mgconst >> 16);
    OSReport(lbl_1_data_104, DATA_win >> 16);
    HuAMemDump();
    OSReport(lbl_1_data_112);
}

/* Applies a sprite attribute to every member of a sprite group. */
void fn_1_40C(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

inline void fn_1_40C(HUSPR_GROUPID groupId, s32 attr);

/* Clears a sprite attribute from every member of a sprite group. */
void fn_1_48C(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}

inline void fn_1_48C(HUSPR_GROUPID groupId, s32 attr);

/* Interpolates between two values over a bounded elapsed time, returning the endpoints outside the
 * interval. */
float fn_1_50C(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((time / duration) * (endValue - startValue));
}

/* Blends a current value toward a sample using the supplied weight. */
float fn_1_550(float currentValue, float sampleValue, float weight)
{
    if (currentValue == sampleValue || weight <= 1.0f) {
        return sampleValue;
    }
    return (sampleValue + (currentValue * (weight - 1.0f))) / weight;
}

/* Blends each component of a 3D position toward the corresponding target component. */
void fn_1_598(HuVecF *dst, const HuVecF *src, float weight)
{
    dst->x = fn_1_550(dst->x, src->x, weight);
    dst->y = fn_1_550(dst->y, src->y, weight);
    dst->z = fn_1_550(dst->z, src->z, weight);
}

inline void fn_1_598(HuVecF *dst, const HuVecF *src, float weight);

/* Moves a value from its start to end using a quarter-cycle sine curve over the given duration. */
float fn_1_724(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((endValue - startValue) * sind((90.0f / duration) * time));
}

/* Moves a value through a full-cycle sine curve and returns to its start when the duration ends. */
float fn_1_80C(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return startValue;
    }
    return startValue + ((endValue - startValue) * sind((360.0f / duration) * time));
}

inline float fn_1_80C(float startValue, float endValue, float time, float duration);

/* Moves a value through a half-cycle sine curve and returns to its start when the duration ends. */
float fn_1_8E8(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return startValue;
    }
    return startValue + ((endValue - startValue) * sind((180.0f / duration) * time));
}

inline float fn_1_8E8(float startValue, float endValue, float time, float duration);

/* Evaluates one component of the menu model's quadratic Bezier path. */
float fn_1_9C4(float startValue, float controlValue, float endValue, float time)
{
    float inverseTime = 1.0f - time;

    return (endValue * (time * time)) + ((startValue * (inverseTime * inverseTime)) +
                                         ((controlValue * (inverseTime * time)) * 2.0f));
}

/* Evaluates a quadratic Bezier path independently for each position component. */
void fn_1_A20(
    HuVecF *dst, const HuVecF *a, const HuVecF *b, const HuVecF *c, float t)
{
    dst->x = fn_1_9C4(a->x, b->x, c->x, t);
    dst->y = fn_1_9C4(a->y, b->y, c->y, t);
    dst->z = fn_1_9C4(a->z, b->z, c->z, t);
}

/* Blends a current scalar toward a target using the menu animation's weight calculation. */
float fn_1_C28(float currentValue, float targetValue, float weight)
{
    return (targetValue + (currentValue * (weight - 1.0f))) / weight;
}

/* Interpolates between two values over a bounded elapsed time. */
float fn_1_C48(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((time / duration) * (endValue - startValue));
}

/* Moves a model along a position path while easing its heading toward that path. */
void fn_1_C8C(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration)
{
    HuVecF modelPos;
    HuVecF modelRot;
    HuVecF pos;
    HuVecF rot;

    Hu3DModelPosGet(modelId, &modelPos);
    Hu3DModelRotGet(modelId, &modelRot);
    pos.x = fn_1_C48(start->x, end->x, time, duration);
    pos.y = fn_1_C48(start->y, end->y, time, duration);
    pos.z = fn_1_C48(start->z, end->z, time, duration);
    modelPos.x -= pos.x;
    modelPos.z -= pos.z;
    rot.y = -(180.0 * (atan2(modelPos.x, -modelPos.z) / M_PI));
    if (modelRot.y - rot.y > 180.0f) {
        modelRot.y -= 360.0f;
    } else if (modelRot.y - rot.y < -180.0f) {
        modelRot.y += 360.0f;
    }
    rot.x = modelRot.x;
    rot.y = fn_1_C28(modelRot.y, rot.y, 10.0f);
    rot.z = modelRot.z;
    Hu3DModelPosSetV(modelId, &pos);
    Hu3DModelRotSetV(modelId, &rot);
}

inline void fn_1_C8C(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration);

/* Sets the camera's menu-choice endpoints from the selected choice's position in the three-column
 * layout. */
void fn_1_FEC(s16 dataIndex)
{
    float divisor = 4.0f;

    if (dataIndex == -1) {
        lbl_1_bss_19AC[1].x = 0.0f;
        lbl_1_bss_19AC[1].y = 1860.0f;
        lbl_1_bss_19AC[1].z = 4180.0f;
        lbl_1_bss_19AC[3].x = 0.0f;
        lbl_1_bss_19AC[3].y = 317.0f;
        lbl_1_bss_19AC[3].z = 100.0f;
    } else {
        lbl_1_bss_19AC[1].x = lbl_1_data_28[dataIndex].x / divisor;
        lbl_1_bss_19AC[1].y = 1860.0f + lbl_1_data_28[dataIndex].y / divisor;
        lbl_1_bss_19AC[1].z = 4080.0f + lbl_1_data_28[dataIndex].z / divisor;
        lbl_1_bss_19AC[3].x = lbl_1_data_28[dataIndex].x / divisor;
        lbl_1_bss_19AC[3].y = 317.0f + lbl_1_data_28[dataIndex].y / divisor;
        lbl_1_bss_19AC[3].z = lbl_1_data_28[dataIndex].z / divisor;
    }
}

/* Sets the camera endpoints for the current menu row using that row's choice positions. */
void fn_1_11D4(void)
{
    float divisor;
    float ignoredScale; /* Assigned the constant 2.0 but never read. */
    float y;
    s16 index;

    divisor = 1.0f;
    ignoredScale = 2.0f;
    y = 250.0f;
    index = lbl_1_bss_1A30[1] + (3 * lbl_1_bss_1A30[2]);
    if (lbl_1_bss_1A30[0] == 5) {
        y = 400.0f;
    }
    lbl_1_bss_19AC[1].x = lbl_1_data_28[index].x;
    lbl_1_bss_19AC[1].y = y + lbl_1_data_28[index].y;
    lbl_1_bss_19AC[1].z = 1000.0f + lbl_1_data_28[index].z;
    lbl_1_bss_19AC[3].x = lbl_1_data_28[index].x / divisor;
    lbl_1_bss_19AC[3].y = y + (lbl_1_data_28[index].y / divisor);
    lbl_1_bss_19AC[3].z = lbl_1_data_28[index].z / divisor;
}

inline void fn_1_11D4(void);

/* Per-frame camera object callback registered by fn_1_1734; eases the view toward the selected
 * choice. */
void fn_1_1370(OMOBJ *obj)
{
    fn_1_598(&lbl_1_bss_19AC[0], &lbl_1_bss_19AC[1], 15.0f);
    fn_1_598(&lbl_1_bss_19AC[2], &lbl_1_bss_19AC[3], 15.0f);
    Hu3DCameraPosSet(
        1, lbl_1_bss_19AC[0].x, lbl_1_bss_19AC[0].y,
        lbl_1_bss_19AC[0].z, 0.0f, 1.0f, 0.0f,
        lbl_1_bss_19AC[2].x, lbl_1_bss_19AC[2].y,
        lbl_1_bss_19AC[2].z);
}

/* Creates the menu camera and schedules its per-frame position update. */
void fn_1_1734(void)
{
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f, 10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPosSet(1, 0.0f, 1860.0f, 4080.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        317.0f, 0.0f);

    lbl_1_bss_19AC[0].x = lbl_1_bss_19AC[1].x = 0.0f;
    lbl_1_bss_19AC[0].y = lbl_1_bss_19AC[1].y = 1860.0f;
    lbl_1_bss_19AC[0].z = lbl_1_bss_19AC[1].z = 4480.0f;
    lbl_1_bss_19AC[2].x = lbl_1_bss_19AC[3].x = 0.0f;
    lbl_1_bss_19AC[2].y = lbl_1_bss_19AC[3].y = 317.0f;
    lbl_1_bss_19AC[2].z = lbl_1_bss_19AC[3].z = 0.0f;

    lbl_1_bss_20[0] =
        omAddObjEx(lbl_1_bss_8, MDSEL_OBJECT_PRIORITY, 0, 0, -1, fn_1_1370);
}

inline void fn_1_1734(void);

/* Destroys the camera used to frame the mode-selection menu. */
void fn_1_1964(void)
{
    Hu3DCameraKill(1);
}

/* Creates the two fixed lights used to illuminate mode-selection models. */
void fn_1_1988(void)
{
    HuVecF pos[2] = { { 0.0f, 1.0f, 1.0f }, { -1.0f, 1.0f, -1.0f } };
    HuVecF dir[2] = { { 0.0f, -1.0f, -1.0f }, { 1.0f, -1.0f, -1.0f } };
    GXColor color = { 255, 255, 255, 255 };
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_1A48[i] = Hu3DGLightCreateV(&pos[i], &dir[i], &color);
        Hu3DGLightInfinitytSet(lbl_1_bss_1A48[i]);
        Hu3DGLightStaticSet(lbl_1_bss_1A48[i], TRUE);
    }
}

inline void fn_1_1988(void);

/* Destroys the fixed lights created for the mode-selection models. */
void fn_1_1AD8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DGLightKill(lbl_1_bss_1A48[i]);
    }
}

/* Opens the requested menu window, using the standard display call for the first window. */
void fn_1_1B30(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_1A40[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_1A40[winNo]);
    }
}

inline void fn_1_1B30(s16 winNo);

/* Closes the requested menu window, using the standard display call for the first window. */
void fn_1_1BA0(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_1A40[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_1A40[winNo]);
    }
}

inline void fn_1_1BA0(s16 winNo);

/* Waits for the active message in the requested menu window to finish. */
void fn_1_1C10(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_1A40[winNo]);
}

inline void fn_1_1C10(s16 winNo);

/* Reads the player's choice from a menu window, optionally disabling cancellation while the choice
 * is pending. */
s16 fn_1_1C4C(s16 winNo, s16 mode)
{
    s16 choice = 0;

    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_1A40[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_1A40[winNo], HUWIN_ATTR_NOCANCEL);
    }
    choice = HuWinChoiceGet(lbl_1_bss_1A40[winNo], -1);
    if (mode == 2 && choice == -1) {
        choice = 1;
    }
    return choice;
}

inline s16 fn_1_1C4C(s16 winNo, s16 mode);

/* Displays a centered message in a menu window at the requested text speed and runs its completion
 * callback. */
void fn_1_1D20(s16 winNo, s32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_1A40[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_1A40[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_1A40[winNo], speed);
    if (lbl_1_data_7C != messNum) {
        lbl_1_data_7C = -1;
    }
}

inline void fn_1_1D20(s16 winNo, s32 messNum, s16 speed);

/* Initializes the four message windows used by the mode-selection screen. */
void fn_1_1DDC(void)
{
    s16 i;

    HuWinInit(1);
    lbl_1_bss_1A40[0] = HuWinExCreateFrame(16.0f, 337.0f, MDSEL_WINDOW_WIDTH,
        MDSEL_WINDOW_HEADER_HEIGHT, -1, 0);
    HuWinDispOff(lbl_1_bss_1A40[0]);
    HuWinBGTPLvlSet(lbl_1_bss_1A40[0], 0.0f);
    lbl_1_bss_1A40[1] = HuWinExCreateFrame(16.0f, 372.0f, MDSEL_WINDOW_WIDTH,
        MDSEL_WINDOW_MENU_HEIGHT, -1, 0);
    HuWinDispOff(lbl_1_bss_1A40[1]);
    HuWinBGTPLvlSet(lbl_1_bss_1A40[1], 0.9f);
    lbl_1_bss_1A40[2] = HuWinExCreateFrame(16.0f, 372.0f, MDSEL_WINDOW_WIDTH,
        MDSEL_WINDOW_MENU_HEIGHT, -1, 3);
    HuWinDispOff(lbl_1_bss_1A40[2]);
    HuWinBGTPLvlSet(lbl_1_bss_1A40[2], 0.9f);
    lbl_1_bss_1A40[3] = HuWinExCreateFrame(16.0f, 372.0f, MDSEL_WINDOW_WIDTH,
        MDSEL_WINDOW_MENU_HEIGHT, -1, 4);
    HuWinDispOff(lbl_1_bss_1A40[3]);
    HuWinBGTPLvlSet(lbl_1_bss_1A40[3], 0.9f);

    for (i = 0; i < 4; i++) {
        winData[lbl_1_bss_1A40[i]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_1A40[i], fn_1_0);
        HuWinAttrSet(lbl_1_bss_1A40[i], HUWIN_ATTR_UPAUSE);
    }
}

inline void fn_1_1DDC(void);

/* Destroys the mode-selection message windows and clears the window system state. */
void fn_1_2024(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        HuWinExKill(lbl_1_bss_1A40[i]);
    }
    HuWinAllKill();
}

/* Makes the requested primary message window active, closing the previous one when necessary. */
void fn_1_2080(s16 winNo)
{
    if (lbl_1_data_150[0] != -1 && lbl_1_data_150[0] != winNo) {
        fn_1_1BA0(lbl_1_data_150[0]);
    }
    if (lbl_1_data_150[0] == -1 || lbl_1_data_150[0] != winNo) {
        lbl_1_data_150[0] = winNo;
        lbl_1_data_154[0] = -1;
        fn_1_1B30(lbl_1_data_150[0]);
    }
}

/* Closes and forgets the active primary message window and its message. */
void fn_1_21DC(void)
{
    if (lbl_1_data_150[0] != -1) {
        fn_1_1BA0(lbl_1_data_150[0]);
    }
    lbl_1_data_150[0] = -1;
    lbl_1_data_154[0] = -1;
}

/* Waits for the current primary-window message to finish when one is open. */
void fn_1_2288(void)
{
    if (lbl_1_data_150[0] != -1) {
        fn_1_1C10(lbl_1_data_150[0]);
    }
}

/* Returns the current primary-window choice, or zero when no window is active. */
s16 fn_1_22E8(s16 mode)
{
    if (lbl_1_data_150[0] != -1) {
        return fn_1_1C4C(lbl_1_data_150[0], mode);
    }
    return 0;
}

/* Shows a message in the primary window, reopening it when needed and avoiding a redundant message
 * update. */
void fn_1_23E0(s16 winNo, s32 messNum, s16 speed)
{
    fn_1_2080(winNo);
    if (lbl_1_data_154[0] != messNum) {
        lbl_1_data_154[0] = messNum;
        fn_1_1D20(lbl_1_data_150[0], lbl_1_data_154[0], speed);
    }
}

/* Shows or updates the secondary guide message window. */
void fn_1_25F8(s32 messNum)
{
    if (lbl_1_data_150[1] == -1) {
        lbl_1_data_150[1] = 0;
        lbl_1_data_154[1] = -1;
        fn_1_1B30(lbl_1_data_150[1]);
    }
    if (lbl_1_data_154[1] != messNum) {
        lbl_1_data_154[1] = messNum;
        fn_1_1D20(lbl_1_data_150[1], lbl_1_data_154[1], 0);
    }
}

/* Closes and forgets the secondary guide message window. */
void fn_1_277C(void)
{
    if (lbl_1_data_150[1] != -1) {
        fn_1_1BA0(lbl_1_data_150[1]);
    }
    lbl_1_data_150[1] = -1;
    lbl_1_data_154[1] = -1;
}

/* Loads and positions the sprite group that marks the selected mode on the menu. */
void fn_1_2828(void)
{
    LBL_1_DATA_8_ENTRY *desc = lbl_1_data_8;
    s16 i;

    for (i = 0; i < 1; i++) {
        lbl_1_bss_1A3C[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_0[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 1; i++) {
        lbl_1_bss_1A38[i] = HuSprGrpCreate(lbl_1_data_4[i]);
    }
    for (i = 0; i < 1; i++, desc++) {
        lbl_1_bss_1A36[i] =
            HuSprCreate(lbl_1_bss_1A3C[desc->animNo], desc->priority, desc->bank);
        HuSprGrpMemberSet(
            lbl_1_bss_1A38[desc->groupNo], desc->memberNo, lbl_1_bss_1A36[i]);
        HuSprPosSet(lbl_1_bss_1A38[desc->groupNo], desc->memberNo, desc->pos.x,
            desc->pos.y);
        HuSprScaleSet(lbl_1_bss_1A38[desc->groupNo], desc->memberNo, desc->scale.x,
            desc->scale.y);
        HuSprZRotSet(lbl_1_bss_1A38[desc->groupNo], desc->memberNo, desc->zRot);
    }
    for (i = 0; i < 1; i++) {
        fn_1_40C(lbl_1_bss_1A38[i], HUSPR_ATTR_DISPOFF);
    }
}

inline void fn_1_2828(void);

/* This function has an empty body at this address in the mode-selection overlay. */
void fn_1_2A74(void)
{
}

/* Layer 15 draw hook installed by fn_1_30FC; draws the tinted menu backdrop. */
void fn_1_2A78(s16 layerNo)
{
    float ignoredPulse; /* Stores the computed pulse, which this hook never uses. */
    GXTexObj texObj;
    Mtx44 projection;
    Mtx trans;
    Mtx rot;
    Mtx model;

    if (lbl_1_bss_28 != NULL) {
        MTXOrtho(projection, 0.0f, 480.0f, 0.0f, 640.0f, 0.0f, 10.0f);
        GXSetProjection(projection, GX_ORTHOGRAPHIC);
        GXInitTexObj(
            &texObj, lbl_1_bss_28, 640, 480, GX_TF_RGBA8, GX_CLAMP,
            GX_CLAMP, GX_FALSE);
        GXInitTexObjLOD(
            &texObj, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE,
            GX_FALSE, GX_ANISO_1);
        GXLoadTexObj(&texObj, GX_TEXMAP0);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(
            GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(
            GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(
            GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetNumTexGens(1);
        GXSetTexCoordGen2(
            GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
            GX_FALSE, GX_PTIDENTITY);
        GXSetNumChans(1);
        GXSetChanCtrl(
            GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0,
            GX_DF_CLAMP, GX_AF_NONE);
        GXSetNumTevStages(1);
        GXSetTevOrder(
            GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevColorIn(
            GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC,
            GX_CC_ZERO);
        GXSetTevColorOp(
            GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
            GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(
            GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA,
            GX_CA_ZERO);
        GXSetTevAlphaOp(
            GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
            GX_TRUE, GX_TEVPREV);

        MTXTrans(trans, -320.0f, -240.0f, 0.0f);
        MTXRotRad(rot, 'Z', MTXDegToRad(lbl_1_bss_2C / 10.0f));
        MTXConcat(rot, trans, model);
        mtxTransCat(model, 320.0f, 240.0f, 0.0f);
        GXLoadPosMtxImm(model, GX_PNMTX0);
        /* The pulse value is discarded; only its frame counter advances. */
        ignoredPulse = fn_1_724(0.0f, 1.0f, ++lbl_1_bss_2C, 60.0f);

        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(-50.0f, -50.0f, 0.0f);
        GXColor4u8(128, 128, 128, 160);
        GXTexCoord2f32(0.0f, 0.0f);
        GXPosition3f32(690.0f, -50.0f, 0.0f);
        GXColor4u8(128, 128, 128, 160);
        GXTexCoord2f32(1.0f, 0.0f);
        GXPosition3f32(690.0f, 530.0f, 0.0f);
        GXColor4u8(128, 128, 128, 160);
        GXTexCoord2f32(1.0f, 1.0f);
        GXPosition3f32(-50.0f, 530.0f, 0.0f);
        GXColor4u8(128, 128, 128, 160);
        GXTexCoord2f32(0.0f, 1.0f);

        if (lbl_1_bss_28) {
            Hu3DFbCopyExec(
                0, 0, 640, 480, GX_TF_RGBA8, GX_FALSE, lbl_1_bss_28);
        }
    }
}

/* Installs the backdrop drawing hook when its framebuffer texture is available. */
void fn_1_30FC(void)
{
    if (lbl_1_bss_28) {
        Hu3DLayerHookSet(15, fn_1_2A78);
    }
}

/* Allocates the framebuffer texture used by the menu backdrop hook. */
void fn_1_313C(void)
{
    lbl_1_bss_28 = HuMemDirectMallocNum(
        HEAP_MODEL, GXGetTexBufferSize(640, 480, GX_TF_RGBA8, FALSE, 0), HU_MEMNUM_OVL);
}

/* Removes the backdrop hook and frees its framebuffer texture during menu cleanup. */
void fn_1_318C(void)
{
    Hu3DLayerHookReset(15);
    if (lbl_1_bss_28) {
        HuMemDirectFree(lbl_1_bss_28);
    }
    lbl_1_bss_28 = NULL;
}

/* Ends the current guide-model display interval and restores its looping idle motion. */
void fn_1_31E4(OMOBJ *obj)
{
    if (++obj->work[0] > 120) {
        Hu3DMotionShiftSet(lbl_1_bss_14->mdlId[0], lbl_1_bss_14->mtnId[0], 0.0f, 15.0f,
            HU3D_MOTATTR_LOOP);
        obj->objFunc = NULL;
    }
}

/* Starts the guide animation and sound for the first set of mode choices. */
void fn_1_3274(void)
{
    OMOBJ *obj = lbl_1_bss_30;

    fn_1_FEC0(
        lbl_1_bss_14->mdlId[0], MSM_SE_GUIDE_26,
        (MSM_PAN_CENTER - MSM_PAN_LEFT) / 2, -1);
    Hu3DMotionShiftSet(lbl_1_bss_14->mdlId[0], lbl_1_bss_14->mtnId[4], 0.0f, 15.0f,
        HU3D_MOTATTR_LOOP);
    obj->work[0] = 0;
    obj->objFunc = fn_1_31E4;
}

inline void fn_1_3274(void);

/* Ends the current guide-model display interval and restores its looping idle motion. */
void fn_1_3328(OMOBJ *obj)
{
    if (++obj->work[0] > 120) {
        Hu3DMotionShiftSet(lbl_1_bss_18->mdlId[0], lbl_1_bss_18->mtnId[0], 0.0f, 15.0f,
            HU3D_MOTATTR_LOOP);
        obj->objFunc = NULL;
    }
}

/* Starts the guide animation and sound for the second set of mode choices. */
void fn_1_33B8(void)
{
    OMOBJ *obj = lbl_1_bss_30;

    fn_1_FEC0(
        lbl_1_bss_18->mdlId[0], MSM_SE_GUIDE_18,
        (MSM_PAN_CENTER - MSM_PAN_LEFT) / 2, -1);
    Hu3DMotionShiftSet(lbl_1_bss_18->mdlId[0], lbl_1_bss_18->mtnId[4], 0.0f, 15.0f,
        HU3D_MOTATTR_LOOP);
    obj->work[0] = 0;
    obj->objFunc = fn_1_3328;
}

inline void fn_1_33B8(void);

/* Per-frame model callback installed by fn_1_3910; advances moving models and their sound state. */
void fn_1_346C(OMOBJ *obj)
{
    LBL_1_BSS_8AC_ENTRY *entry;
    HuVecF pos;
    s16 i;

    for (i = 0; i < 30; i++) {
        entry = &lbl_1_bss_9BC[i];
        if (entry->active == 0) {
            continue;
        }
        entry->soundCountdown -= 1.0f;
        if (entry->soundCountdown == 0.0f) {
            lbl_1_bss_19EC[i] = fn_1_5F60(obj->mdlId[i], MSM_SE_GUIDE_39);
        }
        entry->position.x += entry->horizontalSpeed;
        pos.x = entry->position.x;
        pos.y = entry->position.y
            + fn_1_80C(
                0.0f, entry->bobAmplitude, entry->phase, entry->phaseDuration);
        pos.z = entry->position.z;
        Hu3DModelPosSetV(obj->mdlId[i], &pos);
        if (++entry->phase > entry->phaseDuration) {
            entry->phase = 0.0f;
        }
        if (pos.x > 2000.0f) {
            fn_1_5EA4(lbl_1_bss_19EC[i]);
            entry->active = 0;
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        }
        pos.y = fn_1_80C(
            0.0f, entry->rotationAmplitude, entry->rotationPhase, entry->rotationDuration);
        Hu3DModelRotSet(obj->mdlId[i], 0.0f, 90.0f + pos.y, 0.0f);
        if (++entry->rotationPhase > entry->rotationDuration) {
            entry->rotationAmplitude = frandmod(60);
            entry->rotationPhase = 0.0f;
            entry->rotationDuration = frandmod(120) + 60;
        }
    }
    for (i = 0; i < 30; i++) {
        entry = &lbl_1_bss_9BC[i];
        if (entry->active != 0) {
            break;
        }
    }
    if (i == 30) {
        obj->objFunc = NULL;
    }
}

/* Initializes the first mode-choice model group with randomized movement and animation state. */
void fn_1_3910(void)
{
    OMOBJ *obj = lbl_1_bss_30;
    LBL_1_BSS_8AC_ENTRY *entry;
    s16 i;

    for (i = 0; i < 30; i++) {
        entry = &lbl_1_bss_9BC[i];
        entry->active = (rand8() % 2) + 1;
        entry->soundCountdown = rand8() % 10;
        entry->position.x = -2000.0f - frandmod(500);
        entry->position.y = 700.0f + frandmod(200);
        entry->position.z = frandmod(3000) - 500;
        entry->phase = frandmod(120);
        entry->phaseDuration = frandmod(120) + 120;
        entry->horizontalSpeed = 5.0f + (i % 10);
        entry->bobAmplitude = 100.0f + frandmod(200);
        entry->rotationAmplitude = frandmod(60);
        entry->rotationPhase = 0.0f;
        entry->rotationDuration = frandmod(120) + 60;
        Hu3DMotionShiftSet(
            obj->mdlId[i], obj->mtnId[0], 0.0f, 0.0f,
            HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(obj->mdlId[i], &entry->position);
        Hu3DModelRotSet(obj->mdlId[i], 0.0f, 90.0f, 0.0f);
    }
    obj->objFunc = fn_1_346C;
}

inline void fn_1_3910(void);

/* Called by fn_1_607C each frame; advances the two active models through their movement states. */
void fn_1_3CC0(void)
{
    OMOBJ *obj = lbl_1_bss_30;
    LBL_1_BSS_8AC_ENTRY *entry;
    HuVecF rot;
    s16 i;

    for (i = 0; i < 2; i++) {
        entry = &lbl_1_bss_8AC[i];
        entry->stateTimer++;
        if (entry->stateTimer > 180) {
            entry->stateTimer = 0;
            entry->intensity--;
            if (entry->intensity < 0) {
                entry->intensity = 0;
            }
        }
        if (entry->stateDuration <= 100.0f && entry->movementState == 100
            && rand8() % 6 == 0) {
            fn_1_FEC0(obj->mdlId[i + 30], MSM_SE_GUIDE_56, 16, 100);
        }
        switch (entry->movementState) {
            case 0:
                entry->movementState = 100;
                entry->stateElapsed = 0.0f;
                if (entry->movementVariant == 0) {
                    entry->stateDuration = (rand8() % 100) + 120;
                } else {
                    entry->stateDuration = (rand8() % 30) + 60;
                }
                if (entry->movementVariant == 2) {
                    if (rand8() % 2 == 0) {
                        entry->movementVariant = 0;
                    } else {
                        entry->movementVariant = 1;
                    }
                }
                Hu3DModelPosGet(obj->mdlId[i + 30], &entry->startPosition);
                Hu3DModelPosGet(obj->mdlId[i + 30], &entry->endPosition);
                if (entry->startPosition.x > 0.0f) {
                    if (i == 0) {
                        entry->endPosition.x = (frandmod(200) + 400) * -1;
                    } else {
                        entry->endPosition.x = (frandmod(100) + 400) * -1;
                    }
                } else {
                    if (i == 0) {
                        entry->endPosition.x = frandmod(200) + 400;
                    } else {
                        entry->endPosition.x = frandmod(100) + 400;
                    }
                }
                Hu3DMotionShiftSet(
                    obj->mdlId[i + 30], obj->mtnId[31], 0.0f, 8.0f,
                    HU3D_MOTATTR_LOOP);
                break;
            case 100:
                fn_1_C8C(
                    obj->mdlId[i + 30], &entry->startPosition, &entry->endPosition,
                    entry->stateElapsed, entry->stateDuration);
                if (++entry->stateElapsed > entry->stateDuration) {
                    entry->movementState = 200;
                    entry->stateElapsed = 0.0f;
                    entry->targetYaw = frandmod(360) - 180;
                    if (entry->movementVariant == 1) {
                        entry->movementState = 0;
                        entry->movementVariant = 2;
                    }
                }
                break;
            case 200:
                Hu3DModelRotGet(obj->mdlId[i + 30], &rot);
                rot.y = fn_1_C28(rot.y, entry->targetYaw, 30.0f);
                Hu3DModelRotSetV(obj->mdlId[i + 30], &rot);
                entry->movementVariant = 0;
                if (++entry->stateElapsed > 30.0f) {
                    Hu3DMotionShiftSet(
                        obj->mdlId[i + 30], obj->mtnId[30], 0.0f, 8.0f,
                        HU3D_MOTATTR_LOOP);
                    entry->movementState = 300;
                    entry->stateElapsed = 0.0f;
                    entry->stateDuration = rand8() + 120;
                }
                break;
            case 300:
                if (++entry->stateElapsed > entry->stateDuration) {
                    if (rand8() % 2 == 0) {
                        Hu3DMotionShiftSet(
                            obj->mdlId[i + 30], obj->mtnId[31], 0.0f,
                            8.0f, HU3D_MOTATTR_LOOP);
                        entry->movementState = 200;
                        entry->stateElapsed = 0.0f;
                        entry->targetYaw = frandmod(360) - 180;
                    } else {
                        entry->movementState = 0;
                    }
                }
                break;
            case 400:
                if (++entry->stateElapsed > entry->stateDuration) {
                    entry->movementState = 0;
                    entry->movementVariant = 1;
                }
                break;
        }
    }
}

/* On recognized word ID 2, sets both guide models to idle motion and plays a sound by intensity. */
void fn_1_46DC(void)
{
    OMOBJ *obj = lbl_1_bss_30;
    LBL_1_BSS_8AC_ENTRY *entry;
    s16 i;

    for (i = 0; i < 2; i++) {
        entry = &lbl_1_bss_8AC[i];
        entry->movementState = 400;
        entry->stateElapsed = 0.0f;
        entry->stateDuration = 20.0f;
        entry->intensity++;
        if (entry->intensity > 30) {
            entry->intensity = 30;
        }
        if (entry->intensity > 10) {
            fn_1_FEC0(obj->mdlId[i + 30], MSM_SE_GUIDE_59, 16, 100);
        } else if (entry->intensity > 5) {
            fn_1_FEC0(obj->mdlId[i + 30], MSM_SE_GUIDE_58, 16, 100);
        } else {
            fn_1_FEC0(obj->mdlId[i + 30], MSM_SE_GUIDE_57, 16, 100);
        }
        Hu3DMotionShiftSet(
            obj->mdlId[i + 30], obj->mtnId[32], 0.0f, 10.0f,
            HU3D_MOTATTR_LOOP);
    }
}

inline void fn_1_46DC(void);

/* Called each frame by fn_1_607C; eases the guide model's height and yaw and picks new offsets when
 * its timer expires. */
void fn_1_485C(void)
{
    LBL_1_BSS_24C_ENTRY *entry;
    OMOBJ *obj;
    HuVecF pos;
    s16 i;

    i = 9;
    entry = &lbl_1_bss_24C[9];
    obj = lbl_1_bss_30;
    /* The last guide model is forced visible on every update. */
    Hu3DModelDispOn(obj->mdlId[i + 35]);
    Hu3DModelPosGet(obj->mdlId[i + 35], &pos);
    if (lbl_1_bss_3C == 1) {
        pos.y = fn_1_C28(pos.y, -400.0f, 10.0f);
        entry->lowerYVariant = 0.0f;
    } else {
        pos.y = fn_1_C28(pos.y, -200.0f, 10.0f);
    }
    if (entry->lowerYVariant == 1.0f) {
        pos.y = fn_1_C28(pos.y, -400.0f, 10.0f);
    }
    Hu3DModelPosSet(obj->mdlId[i + 35], pos.x, pos.y, pos.z);
    if (lbl_1_bss_3C == 0) {
        if (lbl_1_bss_24C[0].direction == 1) {
            Hu3DModelPosSet(obj->mdlId[i + 35], -800.0f, pos.y, -100.0f);
        } else {
            Hu3DModelPosSet(obj->mdlId[i + 35], 820.0f, pos.y, -120.0f);
        }
    }
    Hu3DModelRotGet(obj->mdlId[i + 35], &pos);
    pos.y = fn_1_C28(pos.y, entry->targetYaw, 15.0f);
    Hu3DModelRotSet(obj->mdlId[i + 35], -40.0f, pos.y, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[i + 35], 1.5f, 1.5f, 1.5f);
    if (++entry->elapsed > entry->duration) {
        entry->elapsed = 0.0f;
        entry->duration = rand8() + 90;
        entry->targetYaw = frandmod(90) - 45;
        entry->lowerYVariant = 0.0f;
        if (rand8() % 2 == 0) {
            entry->lowerYVariant = 1.0f;
        }
    }
}

inline void fn_1_485C(void);

/* Per-frame object callback installed by fn_1_5290; advances the second choice group's crossing
 * models. */
void fn_1_4CF4(OMOBJ *obj)
{
    LBL_1_BSS_24C_ENTRY *ignoredEntries; /* Assigned the table address but never read. */
    LBL_1_BSS_24C_ENTRY *entry;
    HuVecF pos;
    s16 i;

    ignoredEntries = lbl_1_bss_24C;
    for (i = 0; i < lbl_1_bss_3A; i++) {
        entry = &lbl_1_bss_24C[i];
        if (entry->elapsed == 0.0f) {
            fn_1_FEC0(obj->mdlId[i + 35], MSM_SE_MENU_28, 16, -1);
        }
        pos.x = fn_1_C48(
            entry->startPosition.x, entry->endPosition.x, entry->elapsed,
            entry->duration);
        pos.z = fn_1_C48(
            entry->startPosition.z, entry->endPosition.z, entry->elapsed,
            entry->duration);
        pos.y = fn_1_8E8(-300.0f, 1000.0f, entry->elapsed, entry->duration);
        Hu3DModelPosSet(obj->mdlId[i + 35], pos.x, pos.y, pos.z);
        if (entry->direction == 0) {
            pos.x = fn_1_C48(-90.0f, 90.0f, entry->elapsed, entry->duration);
            Hu3DModelRotSet(obj->mdlId[i + 35], pos.x, 90.0f, 0.0f);
        } else {
            pos.x = fn_1_C48(-90.0f, 90.0f, entry->elapsed, entry->duration);
            Hu3DModelRotSet(obj->mdlId[i + 35], pos.x, -90.0f, 0.0f);
        }
        if (entry->elapsed == entry->duration - 5.0f) {
            fn_1_FEC0(obj->mdlId[i + 35], MSM_SE_MENU_29, 16, -1);
        }
        if (++entry->elapsed > entry->duration) {
            Hu3DModelDispOff(obj->mdlId[i + 35]);
        }
    }
    if (lbl_1_bss_24C[lbl_1_bss_3A - 1].elapsed
        > lbl_1_bss_24C[lbl_1_bss_3A - 1].duration) {
        lbl_1_bss_3C = 0;
        obj->objFunc = NULL;
    }
}

/* Chooses and starts a movement pattern for models in the second choice group. */
void fn_1_5290(void)
{
    LBL_1_BSS_24C_ENTRY *ignoredEntries; /* Assigned the table address but never read. */
    LBL_1_BSS_24C_ENTRY *entry;
    s16 i;
    OMOBJ *obj;
    s16 count;
    s16 mode;

    obj = lbl_1_bss_30;
    ignoredEntries = lbl_1_bss_24C;
    mode = lbl_1_bss_24C[0].direction;
    mode++;
    mode %= 2;
    count = (rand8() % 8) + 1;
    lbl_1_bss_3A = count;
    lbl_1_bss_3C = 1;
    for (i = 0; i < count; i++) {
        entry = &lbl_1_bss_24C[i];
        entry->elapsed = ((-5 * i) - 20) - (rand8() % 3);
        entry->duration = 120.0f;
        entry->direction = mode;
        if (entry->direction == 0) {
            entry->startPosition.x = -800.0f;
            entry->startPosition.y = 0.0f;
            entry->startPosition.z = -100.0f;
            entry->endPosition.x = (800.0f + frandmod(100)) - 50.0f;
            entry->endPosition.y = 0.0f;
            entry->endPosition.z = (-150.0f + frandmod(100)) - 50.0f;
        } else {
            entry->endPosition.x = -800.0f;
            entry->endPosition.y = 0.0f;
            entry->endPosition.z = -100.0f;
            entry->startPosition.x = (800.0f + frandmod(100)) - 50.0f;
            entry->startPosition.y = 0.0f;
            entry->startPosition.z = (-150.0f + frandmod(100)) - 50.0f;
        }
        Hu3DModelDispOn(obj->mdlId[i + 35]);
        if (i > 0) {
            Hu3DModelScaleSet(obj->mdlId[i + 35], 1.0f, 1.0f, 1.0f);
        }
    }
    obj->objFunc = fn_1_4CF4;
}

/* Creates the guide models and motions used by both animated choice groups. */
void fn_1_5614(OMOBJ *obj)
{
    s16 i;
    s16 base;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 30; i++) {
        if (i == 0) {
            obj->mdlId[i] = Hu3DModelCreateData(DATANUM(DATA_mdsel, 17));
            obj->mtnId[i] = Hu3DJointMotionData(
                obj->mdlId[0], DATANUM(DATA_mdsel, 18));
        } else {
            obj->mdlId[i] = Hu3DModelLink(obj->mdlId[0]);
        }
        Hu3DModelDispOff(obj->mdlId[i]);
        Hu3DModelPosSet(
            obj->mdlId[i], frandmod(1000) - 500, frandmod(1000),
            -frandmod(1000));
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(
            obj->mdlId[i], obj->mtnId[0], 0.0f, 0.0f,
            HU3D_MOTATTR_LOOP);
    }

    base = 30;
    for (i = 0; i < 2; i++) {
        obj->mdlId[i + base] =
            Hu3DModelCreateData(DATANUM(DATA_mdsel, 11));
        if (i == 0) {
            obj->mtnId[30] = Hu3DJointMotionData(
                obj->mdlId[30], DATANUM(DATA_mdsel, 12));
            obj->mtnId[31] = Hu3DJointMotionData(
                obj->mdlId[30], DATANUM(DATA_mdsel, 13));
            obj->mtnId[32] = Hu3DJointMotionData(
                obj->mdlId[30], DATANUM(DATA_mdsel, 14));
        }
        Hu3DModelPosSet(
            obj->mdlId[i + base], frandmod(1000) - 500, 0.0f,
            -350.0f + (300.0f * i));
        Hu3DModelScaleSet(obj->mdlId[i + base], 1.5f, 1.5f, 1.5f);
        Hu3DModelLayerSet(obj->mdlId[i + base], 1);
        Hu3DMotionShiftSet(
            obj->mdlId[i + base], obj->mtnId[30], 0.0f, 0.0f,
            HU3D_MOTATTR_LOOP);
    }

    base = 35;
    for (i = 0; i < 10; i++) {
        obj->mdlId[i + 35] =
            Hu3DModelCreateData(DATANUM(DATA_mdsel, 15));
        obj->mtnId[i + 35] = Hu3DJointMotionData(
            obj->mdlId[i + 35], DATANUM(DATA_mdsel, 16));
        Hu3DModelPosSet(obj->mdlId[i + 35], 0.0f, 500.0f, 0.0f);
        Hu3DModelScaleSet(obj->mdlId[i + 35], 1.5f, 1.5f, 1.5f);
        Hu3DModelDispOff(obj->mdlId[i + 35]);
        Hu3DModelLayerSet(obj->mdlId[i + 35], 1);
        Hu3DMotionShiftSet(
            obj->mdlId[i + 35], obj->mtnId[i + 35], 0.0f, 0.0f,
            HU3D_MOTATTR_LOOP);
    }
    lbl_1_bss_24C[0].direction = 1;
    obj->objFunc = NULL;
}

/* This function has an empty body at this address in the mode-selection overlay. */
void fn_1_5BEC(void)
{
}

/* Called by fn_1_607C each frame; pans sounds by projected x and fades volume as absolute world x
 * passes 1500. */
void fn_1_5BF0(void)
{
    HuVecF modelPos;
    HuVecF screenPos;
    s16 pan;
    s16 panRange = 32;
    s16 volume;
    s16 i;

    for (i = 0; i < 15; i++) {
        if (lbl_1_bss_1D4[i].modelId == HU3D_MODELID_NONE) {
            continue;
        }

        Hu3DModelPosGet(lbl_1_bss_1D4[i].modelId, &modelPos);
        Hu3D3Dto2D(&modelPos, 1, &screenPos);
        screenPos.x -= 288.0f;
        screenPos.x /= 288.0f;
        pan = 64.0f + (screenPos.x * panRange);
        if (pan < 64 - panRange) {
            pan = 64 - panRange;
        }
        if (pan > 64 + panRange) {
            pan = 64 + panRange;
        }

        Hu3DModelPosGet(lbl_1_bss_1D4[i].modelId, &modelPos);
        modelPos.x = (modelPos.x < 0.0f) ? -modelPos.x : modelPos.x;
        if (modelPos.x < 1500.0f) {
            volume = 127;
        } else {
            volume = 127.0f -
                (127.0f * ((modelPos.x - 1500.0f) / 500.0f));
        }
        if (volume > 127) {
            volume = 127;
        }
        if (volume < 0) {
            volume = 0;
        }

        if (lbl_1_bss_1D4[i].fxHandle > 0) {
            HuAudFXPanning(lbl_1_bss_1D4[i].fxHandle, pan);
            HuAudFXVolSet(lbl_1_bss_1D4[i].fxHandle, volume);
            i == 0; /* This comparison has no effect on the active sound. */
        }
    }
}

/* Stops and clears the sound tracked for one model slot. */
void fn_1_5EA4(s16 index)
{
    if (lbl_1_bss_1D4[index].modelId != HU3D_MODELID_NONE) {
        if (lbl_1_bss_1D4[index].fxHandle > 0) {
            HuAudFXStop(lbl_1_bss_1D4[index].fxHandle);
        }
        lbl_1_bss_1D4[index].modelId = HU3D_MODELID_NONE;
        lbl_1_bss_1D4[index].fxHandle = -1;
    }
}

/* Claims a free model sound slot and starts the sound, returning its index or -1 when all are
 * occupied. */
s16 fn_1_5F60(HU3D_MODELID modelId, s32 fxNo)
{
    s16 i;

    for (i = 0; i < 15; i++) {
        if (lbl_1_bss_1D4[i].modelId == HU3D_MODELID_NONE) {
            break;
        }
    }
    if (i == 15) {
        return -1;
    }
    lbl_1_bss_1D4[i].modelId = modelId;
    lbl_1_bss_1D4[i].fxHandle = HuAudFXPlay(fxNo);
    return i;
}

/* Clears all model sound slots before mode-selection audio starts. */
void fn_1_6018(void)
{
    s16 i;

    for (i = 0; i < 15; i++) {
        lbl_1_bss_1D4[i].modelId = HU3D_MODELID_NONE;
        lbl_1_bss_1D4[i].fxHandle = -1;
    }
}

inline void fn_1_6018(void);

/* Per-frame object callback registered by fn_1_6C04; updates tracked sounds and both animated
 * choice groups. */
void fn_1_607C(OMOBJ *obj)
{
    fn_1_5BF0();
    fn_1_3CC0();
    fn_1_485C();
}

/* Microphone listener callback registered by fn_1_6C04; maps recognized words to guide
 * animations. */
void fn_1_651C(MDSEL_MIC_RESPONSE *response)
{
    if (response->status != 0 || response->count == 0) {
        return;
    }
    if (lbl_1_bss_30->objFunc != NULL) {
        return;
    }
    if (lbl_1_data_15C == 0) {
        return;
    }
    switch (response->result[0]) {
        case 4:
            fn_1_3910();
            break;
        case 2:
            fn_1_46DC();
            break;
        case 3:
            fn_1_5290();
            break;
        case 0:
            fn_1_3274();
            break;
        case 1:
            fn_1_33B8();
            break;
    }
    {
        s16 i;

        for (i = 0; i < response->count; i++) {
            OSReport(lbl_1_data_15E, response->result[i]);
        }
    }
}

/* Initializes the microphone listener when available and adds the mode-selection objects to the
 * object manager. */
void fn_1_6C04(void)
{
    s16 unusedStatus = 0; /* Initialized but never read by microphone setup. */

    lbl_1_bss_38 = 0;
    if (GwCommon.mic == 1) {
        HuMCInit(0);
        if (HuMCMount(1) != MIC_RESULT_READY) {
            OSReport(lbl_1_data_169);
            HuMCClose();
        } else {
            OSReport(lbl_1_data_1A8);
            if (GwCommon.mic != 1) {
                OSReport(lbl_1_data_1E7);
                HuMCClose();
            } else {
                OSReport(lbl_1_data_223);
                lbl_1_bss_1A4C = HuMCContextCreate(lbl_1_data_25E);
                HuMCListenerCreate(lbl_1_bss_1A4C, (MCResponseCallback)fn_1_651C, 1);
                lbl_1_bss_38 = 1;
                OSReport(lbl_1_data_275);
            }
        }
    }

    lbl_1_data_15C = 1;
    fn_1_6018();
    lbl_1_bss_30 = omAddObj(lbl_1_bss_8, MDSEL_OBJECT_PRIORITY,
        MDSEL_LARGE_OBJECT_CAPACITY, MDSEL_LARGE_OBJECT_CAPACITY, fn_1_5614);
    lbl_1_bss_34 = omAddObj(lbl_1_bss_8, MDSEL_OBJECT_PRIORITY, 0, 0, fn_1_607C);
}

inline void fn_1_6C04(void);

/* Stops the microphone listener and closes its context when microphone setup succeeded. */
void fn_1_6DFC(void)
{
    lbl_1_data_15C = 0;
    if (lbl_1_bss_38 == 1) {
        HuMCListenerKill();
        HuMCContextKill(lbl_1_bss_1A4C);
        HuMCClose();
    }
}

inline void fn_1_6DFC(void);

/* Checks microphone availability and initializes its connection when the menu needs it. */
s16 fn_1_6E54(void)
{
    s16 result = TRUE;

    if (GwCommon.mic != 1) {
        return FALSE;
    }
    if (lbl_1_bss_38 == 0) {
        HuMCInit(0);
        if (HuMCMount(1) != MIC_RESULT_READY) {
            OSReport(lbl_1_data_169);
            result = FALSE;
        } else {
            OSReport(lbl_1_data_27F);
            result = TRUE;
        }
        HuMCClose();
    } else {
        if (HuMCMount(1) != MIC_RESULT_READY) {
            OSReport(lbl_1_data_169);
            result = FALSE;
        } else {
            OSReport(lbl_1_data_27F);
            result = TRUE;
        }
    }
    return result;
}

/* Creates the three models and looping motions for the first mode-choice display. */
void fn_1_6F40(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 3; i++) {
        obj->mdlId[i] = Hu3DModelCreateData(DATANUM(DATA_mdsel, 0) + i);
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DMotionShiftSet(
            obj->mdlId[i], obj->mtnId[i], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    }
    obj->objFunc = NULL;
}

/* Kills the first mode-choice display's models and motions and removes its object. */
void fn_1_702C(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 3; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_8, obj);
    }
    obj = NULL;
}

inline void fn_1_702C(OMOBJ *obj);

/* Per-frame display-object callback installed by fn_1_76DC; advances guide sound cues and motion
 * attributes. */
void fn_1_70BC(OMOBJ *obj)
{
    float motionTime;
    s16 i;

    for (i = 0; i < 6; i++) {
        motionTime = Hu3DMotionTimeGet(obj->mdlId[i]);
        if (motionTime < 5.0f) {
            lbl_1_bss_1B0[i][0] = 0;
            lbl_1_bss_1B0[i][1] = 0;
            lbl_1_bss_1B0[i][2] = 0;
        }
        switch (i) {
            case 0:
                if (lbl_1_bss_1B0[0][0] == 0 && motionTime > 80.0f) {
                    lbl_1_bss_1B0[0][0] = 1;
                    HuAudFXPlay(MSM_SE_MENU_21);
                }
                break;
            case 1:
                if (lbl_1_bss_1B0[1][0] == 0 && motionTime > 60.0f) {
                    lbl_1_bss_1B0[1][0] = 1;
                    fn_1_FDF8(&lbl_1_data_28[3], MSM_SE_MENU_22);
                }
                break;
            case 2:
                if (lbl_1_bss_1B0[2][0] == 0 && motionTime > 10.0f) {
                    lbl_1_bss_1B0[2][0] = 1;
                    fn_1_FDF8(&lbl_1_data_28[5], MSM_SE_MENU_23);
                }
                break;
            case 3:
                if (lbl_1_bss_1B0[3][0] == 0 && motionTime > 10.0f) {
                    lbl_1_bss_1B0[3][0] = 1;
                    fn_1_FDF8(&lbl_1_data_28[0], MSM_SE_MENU_26);
                }
                if (lbl_1_bss_1B0[3][1] == 0 && motionTime > 50.0f) {
                    lbl_1_bss_1B0[3][1] = 1;
                    fn_1_FDF8(&lbl_1_data_28[0], MSM_SE_MENU_26);
                }
                if (lbl_1_bss_1B0[3][2] == 0 && motionTime > 130.0f) {
                    lbl_1_bss_1B0[3][2] = 1;
                    fn_1_FDF8(&lbl_1_data_28[0], MSM_SE_MENU_26);
                }
                break;
            case 4:
                if (lbl_1_bss_1B0[4][0] == 0 && motionTime > 10.0f) {
                    lbl_1_bss_1B0[4][0] = 1;
                    fn_1_FDF8(&lbl_1_data_28[2], MSM_SE_MENU_24);
                }
                if (lbl_1_bss_1B0[4][1] == 0 && motionTime > 90.0f) {
                    lbl_1_bss_1B0[4][1] = 1;
                    fn_1_FDF8(&lbl_1_data_28[2], MSM_SE_MENU_25);
                }
                break;
            case 5:
                if (lbl_1_bss_1B0[5][0] == 0 && motionTime > 5.0f
                    && motionTime < 195.0f) {
                    lbl_1_bss_1B0[5][0] = 1;
                    lbl_1_bss_1A28 = HuAudFXPlay(MSM_SE_MENU_27);
                }
                if (lbl_1_bss_1B0[5][0] == 1
                    && (motionTime < 5.0f || motionTime > 195.0f)) {
                    lbl_1_bss_1B0[5][0] = 0;
                    HuAudFXStop(lbl_1_bss_1A28);
                }
                break;
        }
    }
    for (i = 0; i < 6; i++) {
        if (i != lbl_1_bss_1A30[0] || lbl_1_bss_3E != 1) {
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_MOTATTR_LOOP);
        }
    }
    for (i = 0; i < 6; i++) {
        if (rand8() == 32) {
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_MOTATTR_LOOP);
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_MOTATTR_PAUSE);
            break;
        }
    }
}

/* The argument only selects the -1 reset case: that clears looping on all six; otherwise, when the
 * global menu choice changes, loops and unpauses that guide model and clears looping on the
 * rest. */
void fn_1_75A4(s16 modelIndex)
{
    OMOBJ *obj = lbl_1_bss_10;
    s16 i;

    if (modelIndex == -1) {
        obj->work[0] = 99;
        for (i = 0; i < 6; i++) {
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_MOTATTR_LOOP);
        }
    } else if (obj->work[0] != lbl_1_bss_1A30[0]) {
        obj->work[0] = lbl_1_bss_1A30[0];
        for (i = 0; i < 6; i++) {
            if (i == lbl_1_bss_1A30[0]) {
                Hu3DModelAttrSet(obj->mdlId[i], HU3D_MOTATTR_LOOP);
                Hu3DModelAttrReset(obj->mdlId[i], HU3D_MOTATTR_PAUSE);
            } else {
                Hu3DModelAttrReset(obj->mdlId[i], HU3D_MOTATTR_LOOP);
            }
        }
    }
}

inline void fn_1_75A4(s16 modelIndex);

/* Creates the six-model guide display and starts its animation update callback. */
void fn_1_76DC(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 6; i++) {
        obj->mdlId[i] = Hu3DModelCreateData(DATANUM(DATA_mdsel, 3) + i);
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DMotionShiftSet(
            obj->mdlId[i], obj->mtnId[i], 0.0f, 0.0f, HU3D_MOTATTR_PAUSE);
    }
    obj->work[0] = 99;
    obj->objFunc = fn_1_70BC;
}

/* Kills the six-model guide display's models and motions and removes its object. */
void fn_1_77D8(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 6; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_8, obj);
    }
    obj = NULL;
}

inline void fn_1_77D8(OMOBJ *obj);

/* Per-frame transition callback installed by fn_1_7EC4 or fn_1_E1FC; moves the first decorative
 * model along its Bezier path. */
void fn_1_7868(OMOBJ *obj)
{
    MDSEL_BEZIER_WORK *work = &lbl_1_bss_128;
    HuVecF pos;
    HuVecF modelPos;
    HuVecF modelRot;
    HuVecF scale;
    float modelScale;

    Hu3DModelPosGet(obj->mdlId[0], &modelPos);
    Hu3DModelRotGet(obj->mdlId[0], &modelRot);
    /* This first scale calculation is overwritten below before it is applied. */
    modelScale = fn_1_50C(
        5.0f, 2.0f, work->time - (work->duration / 2.0f),
        work->duration / 2.0f);
    modelScale = fn_1_50C(5.0f, 0.25f, work->time, work->duration);
    Hu3DModelScaleSet(obj->mdlId[0], modelScale, modelScale, modelScale);
    fn_1_A20(
        &pos, &work->control[0], &work->control[1], &work->control[2],
        fn_1_50C(0.0f, 1.0f, work->time, work->duration));
    Hu3DModelPosSetV(obj->mdlId[0], &pos);
    modelPos.x -= pos.x;
    modelPos.z -= pos.z;
    modelPos.y = -(180.0 * (atan2(modelPos.x, -modelPos.z) / M_PI));
    if (modelRot.y - modelPos.y > 180.0f) {
        modelRot.y -= 360.0f;
    } else if (modelRot.y - modelPos.y < -180.0f) {
        modelRot.y += 360.0f;
    }
    modelRot.y = fn_1_550(modelRot.y, modelPos.y, 15.0f);
    Hu3DModelRotSetV(obj->mdlId[0], &modelRot);
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    /* The current scale is read but not used; the particle effect only needs position. */
    Hu3DModelScaleGet(obj->mdlId[0], &scale);
    fn_1_FA9C(4, &pos, 1, 5);
    if (work->time == 45.0f) {
        HuAudFXStop(lbl_1_bss_19DC[0]);
        lbl_1_bss_19DC[2] = HuAudFXPlay(MSM_SE_BRD00_145);
    }
    if (++work->time > work->duration) {
        HuAudFXStop(lbl_1_bss_19DC[2]);
        fn_1_FA9C(4, &pos, 0, 5);
        obj->objFunc = NULL;
    }
}

/* Starts the first decorative model's selection transition and its associated sound. */
void fn_1_7EC4(void)
{
    OMOBJ *obj = lbl_1_bss_14;
    MDSEL_BEZIER_WORK *work = &lbl_1_bss_128;
    s16 index;
    float y;

    if (lbl_1_bss_1A30[1] != 2) {
        y = 200.0f;
        if (lbl_1_bss_1A30[0] == 5) {
            y = 400.0f;
        }
        index = lbl_1_bss_1A30[1] + (lbl_1_bss_1A30[2] * 3);
        work->control[0].x = -1150.0f;
        work->control[0].y = 350.0f;
        work->control[0].z = -1000.0f;
        work->control[1].x = -lbl_1_data_28[index].x;
        work->control[1].y = 2250.0f;
        work->control[1].z = 6000.0f;
        work->control[2].x = lbl_1_data_28[index].x;
        work->control[2].y = y;
        work->control[2].z = lbl_1_data_28[index].z;
        work->time = 0.0f;
        work->duration = 90.0f;
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1], 0.0f, 5.0f, 0);
        lbl_1_bss_19DC[0] = HuAudFXPlay(MSM_SE_BRD00_146);
        obj->objFunc = fn_1_7868;
    }
}

inline void fn_1_7EC4(void);

/* Moves the second decorative model along its Bezier path. The first scale calculation is
 *
 * overwritten before application; the later scale read is unused, and only position feeds
 * particles. */
void fn_1_809C(OMOBJ *obj)
{
    MDSEL_BEZIER_WORK *work = &lbl_1_bss_A0;
    HuVecF pos;
    HuVecF modelPos;
    HuVecF modelRot;
    HuVecF scale;
    float modelScale;

    Hu3DModelPosGet(obj->mdlId[0], &modelPos);
    Hu3DModelRotGet(obj->mdlId[0], &modelRot);
    modelScale = fn_1_50C(
        5.0f, 2.0f, work->time - (work->duration / 2.0f),
        work->duration / 2.0f);
    modelScale = fn_1_50C(5.0f, 0.25f, work->time, work->duration);
    Hu3DModelScaleSet(obj->mdlId[0], modelScale, modelScale, modelScale);
    fn_1_A20(
        &pos, &work->control[0], &work->control[1], &work->control[2],
        fn_1_50C(0.0f, 1.0f, work->time, work->duration));
    Hu3DModelPosSetV(obj->mdlId[0], &pos);
    modelPos.x -= pos.x;
    modelPos.z -= pos.z;
    modelPos.y = -(180.0 * (atan2(modelPos.x, -modelPos.z) / M_PI));
    if (modelRot.y - modelPos.y > 180.0f) {
        modelRot.y -= 360.0f;
    } else if (modelRot.y - modelPos.y < -180.0f) {
        modelRot.y += 360.0f;
    }
    modelRot.y = fn_1_550(modelRot.y, modelPos.y, 15.0f);
    Hu3DModelRotSetV(obj->mdlId[0], &modelRot);
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    Hu3DModelScaleGet(obj->mdlId[0], &scale);
    fn_1_FA9C(5, &pos, 1, 6);
    if (work->time == 45.0f) {
        HuAudFXStop(lbl_1_bss_19DC[1]);
        lbl_1_bss_19DC[3] = HuAudFXPlay(MSM_SE_BRD00_143);
    }
    if (++work->time > work->duration) {
        HuAudFXStop(lbl_1_bss_19DC[3]);
        fn_1_FA9C(5, &pos, 0, 6);
        obj->objFunc = NULL;
    }
}

/* Starts the second decorative model's selection transition and its associated sound. */
void fn_1_86F8(void)
{
    OMOBJ *obj = lbl_1_bss_18;
    MDSEL_BEZIER_WORK *work = &lbl_1_bss_A0;
    float destinationY;
    s16 index;

    if (lbl_1_bss_1A30[1] != 0) {
        destinationY = 100.0f;
        if (lbl_1_bss_1A30[0] == 5) {
            destinationY = 400.0f;
        }
        index = lbl_1_bss_1A30[1] + (3 * lbl_1_bss_1A30[2]);
        work->control[0].x = 1150.0f;
        work->control[0].y = 350.0f;
        work->control[0].z = -1000.0f;
        work->control[1].x = -lbl_1_data_28[index].x;
        work->control[1].y = 2250.0f;
        work->control[1].z = 6000.0f;
        work->control[2].x = lbl_1_data_28[index].x;
        work->control[2].y = destinationY;
        work->control[2].z = lbl_1_data_28[index].z;
        work->time = 0.0f;
        work->duration = 90.0f;
        Hu3DMotionShiftSet(
            obj->mdlId[0], obj->mtnId[1], 0.0f, 5.0f, HU3D_MOTATTR_NONE);
        lbl_1_bss_19DC[1] = HuAudFXPlay(MSM_SE_BRD00_144);
        obj->objFunc = fn_1_809C;
    }
}

inline void fn_1_86F8(void);

/* Creates the first animated title model and its five motions for the mode-choice display. */
void fn_1_88D0(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(DATANUM(DATA_mdsel, 19));
    for (i = 0; i < 5; i++) {
        obj->mtnId[i] =
            Hu3DJointMotionData(obj->mdlId[0], DATANUM(DATA_mdsel, 20) + i);
    }
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    Hu3DModelPosSet(obj->mdlId[0], -1150.0f, 350.0f, -1000.0f);
    Hu3DModelRotSet(obj->mdlId[0], 0.0f, 15.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 5.0f, 5.0f, 5.0f);
    obj->objFunc = NULL;
}

/* Kills the first two stored motions and the first animated title model, then removes its
 * object. */
void fn_1_8A58(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 2; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_8, obj);
    }
    obj = NULL;
}

inline void fn_1_8A58(OMOBJ *obj);

/* Creates the second animated title model and its companion, then creates five motions for the
 * title model. */
void fn_1_8AE0(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(DATANUM(DATA_mdsel, 25));
    obj->mdlId[1] = Hu3DModelCreateData(DATANUM(DATA_mdsel, 26));
    for (i = 0; i < 5; i++) {
        obj->mtnId[i] =
            Hu3DJointMotionData(obj->mdlId[0], DATANUM(DATA_mdsel, 27) + i);
    }
    Hu3DModelHookSet(obj->mdlId[0], lbl_1_data_2BD, obj->mdlId[1]);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    Hu3DModelPosSet(obj->mdlId[0], 1150.0f, 350.0f, -1000.0f);
    Hu3DModelRotSet(obj->mdlId[0], 0.0f, -15.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 5.0f, 5.0f, 5.0f);
    obj->objFunc = NULL;
}

/* Clears the second title model's hook, kills both models and the first two stored motions, then
 * removes the object. */
void fn_1_8CA4(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        Hu3DModelHookReset(obj->mdlId[0]);
        for (i = 0; i < 2; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        Hu3DModelKill(obj->mdlId[1]);
        omDelObjEx(lbl_1_bss_8, obj);
    }
    obj = NULL;
}

inline void fn_1_8CA4(OMOBJ *obj);

/* Updates the selection sprite bank and visibility from the current menu choice. */
void fn_1_8D44(void)
{
    if (lbl_1_data_2D0 != lbl_1_bss_1A30[0]) {
        lbl_1_bss_40 = 0.0f;
        lbl_1_data_2D0 = lbl_1_bss_1A30[0];
    }
    HuSprBankSet(lbl_1_bss_1A38[0], 0, lbl_1_bss_1A30[0]);
    fn_1_48C(lbl_1_bss_1A38[0], HUSPR_ATTR_DISPOFF);
}

inline void fn_1_8D44(void);

/* Hides the selection sprite when a menu choice transition begins. */
void fn_1_8E38(void)
{
    lbl_1_data_2D0 = -1;
    fn_1_40C(lbl_1_bss_1A38[0], HUSPR_ATTR_DISPOFF);
}

inline void fn_1_8E38(void);

/* Projects the selected choice onto the screen and eases the selection sprite's position and
 * size. */
void fn_1_8EC8(void)
{
    HuVecF pos;
    HuVecF pos2D;
    s16 index;

    lbl_1_bss_40 = fn_1_550(lbl_1_bss_40, 1.0f, 5.0f);
    index = lbl_1_bss_1A30[1] + (3 * lbl_1_bss_1A30[2]);
    if (lbl_1_bss_1A30[2] == 1) {
        pos.x = lbl_1_data_28[index].x / 1.5f;
    } else {
        pos.x = lbl_1_data_28[index].x / 2.0f;
    }
    pos.y = lbl_1_data_28[index].y;
    if (index == 1) {
        pos.y = lbl_1_data_28[index].y - 200.0f;
    }
    pos.z = lbl_1_data_28[index].z;
    Hu3D3Dto2D(&pos, 1, &pos2D);
    if (lbl_1_bss_1A30[2] == 1) {
        pos2D.y -= 100.0f;
    }
    HuSprGrpPosSet(lbl_1_bss_1A38[0], pos2D.x, pos2D.y);
    HuSprGrpScaleSet(lbl_1_bss_1A38[0], lbl_1_bss_40, lbl_1_bss_40);
}

inline void fn_1_8EC8(void);

/* Converts a menu-choice position into the elevated, offset position used by its display model. */
void fn_1_90FC(HuVecF *pos)
{
    lbl_1_bss_94.x = pos->x - 200.0f;
    lbl_1_bss_94.y = 500.0f + pos->y;
    lbl_1_bss_94.z = 350.0f + pos->z;
}

inline void fn_1_90FC(HuVecF *pos);

/* Places and shows the display model at the selected choice's menu position. */
void fn_1_9160(HuVecF *pos)
{
    OMOBJ *obj = lbl_1_bss_1C;

    Hu3DModelPosSetV(obj->mdlId[0], pos);
    Hu3DModelRotSet(obj->mdlId[0], 0.0f, 0.0f, 180.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 5.0f, 5.0f, 5.0f);
    fn_1_90FC(pos);
    Hu3DModelAttrReset(obj->mdlId[0], HU3D_ATTR_DISPOFF);
}

inline void fn_1_9160(HuVecF *pos);

/* Hides the selected-choice display model while the menu is changing choices. */
void fn_1_927C(void)
{
    OMOBJ *obj = lbl_1_bss_1C;

    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
}

inline void fn_1_927C(void);

/* Eases the display model's position, rotation, and scale toward the selected choice and updates
 * its marker. */
void fn_1_92BC(OMOBJ *obj)
{
    HuVecF pos;

    Hu3DModelPosGet(obj->mdlId[0], &pos);
    fn_1_598(&pos, &lbl_1_bss_94, 3.0f);
    Hu3DModelPosSetV(obj->mdlId[0], &pos);
    Hu3DModelRotGet(obj->mdlId[0], &pos);
    pos.z = fn_1_550(pos.z, 0.0f, 5.0f);
    Hu3DModelRotSetV(obj->mdlId[0], &pos);
    Hu3DModelScaleGet(obj->mdlId[0], &pos);
    pos.x = pos.y = pos.z = fn_1_550(pos.x, 1.5f, 3.0f);
    Hu3DModelScaleSetV(obj->mdlId[0], &pos);
    fn_1_8EC8();
}

/* Creates the selected-choice display model and schedules its transform update. */
void fn_1_97D4(OMOBJ *obj)
{
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(DATANUM(DATA_mdsel, 9));
    obj->mtnId[0] = Hu3DMotionIDGet(obj->mdlId[0]);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);

    fn_1_927C();
    obj->objFunc = fn_1_92BC;
}

/* Kills the selected-choice model and motion and removes its object. */
void fn_1_98B0(OMOBJ *obj)
{
    if (obj) {
        Hu3DMotionKill(obj->mtnId[0]);
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_8, obj);
    }
    obj = NULL;
}

inline void fn_1_98B0(OMOBJ *obj);

/* Releases the display objects and microphone state, kills the choice particle models, then
 * initializes the selection sprite, message windows, lights, and camera. */
void fn_1_9910(void)
{
    fn_1_702C(lbl_1_bss_C);
    fn_1_77D8(lbl_1_bss_10);
    fn_1_8A58(lbl_1_bss_14);
    fn_1_8CA4(lbl_1_bss_18);
    fn_1_98B0(lbl_1_bss_1C);
    fn_1_FA18();
    fn_1_6DFC();
    fn_1_2828();
    fn_1_1DDC();
    fn_1_1988();
    fn_1_1734();
}

/* Child process created by fn_1_A5D4; runs the menu, then returns or calls the selected mode. */
void fn_1_A310(void)
{
    s16 result = 0;

    result = fn_1_E7B0();

    HuAudSStreamFadeOut(lbl_1_bss_1A2C, 1000);
    switch (result) {
        case -1:
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            WipeWait();
            HuAudFadeOut(1000);
            fn_1_9910();
            fn_1_37C();
            omOvlReturn(1);
            break;
        case 1:
            HuAudFXPlay(MSM_SE_MENU_32);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_DISSOLVE_IN_BLUR, 60);
            WipeWait();
            HuAudFadeOut(1000);
            fn_1_9910();
            fn_1_37C();
            fn_1_1B4();
            break;
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Creates the mode-selection camera, lights, windows, sprites, display objects, and main menu
 * process. */
void fn_1_A5D4(void)
{
    lbl_1_bss_8 = omInitObjMan(27, MDSEL_OBJECT_MANAGER_PRIORITY);
    omGameSysInit(lbl_1_bss_8);
    fn_1_1734();
    fn_1_1988();
    fn_1_1DDC();
    fn_1_2828();
    lbl_1_bss_C =
        omAddObjEx(lbl_1_bss_8, MDSEL_OBJECT_PRIORITY, MDSEL_OBJECT_CAPACITY,
            MDSEL_OBJECT_CAPACITY, -1, fn_1_6F40);
    lbl_1_bss_10 =
        omAddObjEx(lbl_1_bss_8, MDSEL_OBJECT_PRIORITY, MDSEL_OBJECT_CAPACITY,
            MDSEL_OBJECT_CAPACITY, -1, fn_1_76DC);
    lbl_1_bss_14 =
        omAddObjEx(lbl_1_bss_8, MDSEL_OBJECT_PRIORITY, MDSEL_OBJECT_CAPACITY,
            MDSEL_OBJECT_CAPACITY, -1, fn_1_88D0);
    lbl_1_bss_18 =
        omAddObjEx(lbl_1_bss_8, MDSEL_OBJECT_PRIORITY, MDSEL_OBJECT_CAPACITY,
            MDSEL_OBJECT_CAPACITY, -1, fn_1_8AE0);
    lbl_1_bss_1C =
        omAddObjEx(lbl_1_bss_8, MDSEL_OBJECT_PRIORITY, MDSEL_OBJECT_CAPACITY,
            MDSEL_OBJECT_CAPACITY, -1, fn_1_97D4);
    fn_1_6C04();
    fn_1_F790();
    HuPrcChildCreate(fn_1_A310, MDSEL_MAIN_PROCESS_PRIORITY,
        MDSEL_MAIN_PROCESS_STACK_SIZE, 0, lbl_1_bss_8);
}

/* Closes all open data directories before this overlay releases its loaded data. */
void fn_1_B0C8(void)
{
    HuDataDirCloseAll();
}

/* Resets overlay exit and initialization flags, releases stale archive data, and starts the
 * mode-selection screen. */
void ObjectSetup(void)
{
    OSReport(lbl_1_data_2D2);
    fn_1_B0C8();
    MgPauseExitF = FALSE;
    fn_1_2CC();
    _SetFlag(5);
    MgInstExitF = FALSE;
    MgPauseExitF = FALSE;
    MgExitReq = FALSE;
    _ClearFlag(FLAG_BOARD_INIT);
    _ClearFlag(FLAG_INST_MG_MODE);
    _ClearFlag(FLAG_INST_DECA);
    _ClearFlag(FLAG_INST_NO_HISCHG);
    _ClearFlag(FLAGNUM(FLAG_GROUP_SYSTEM, 4));
    lbl_1_bss_0 = omovlevtno;
    lbl_1_bss_4 = omovlstat;
    fn_1_A5D4();
}

/* Runs this overlay's constructors before entering its setup routine. */
int _prolog(void)
{
    const VoidFunc *ctors = _ctors;

    while (*ctors) {
        (**ctors)();
        ctors++;
    }
    ObjectSetup();
    return 0;
}

/* Runs this overlay's registered destructors when the module is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors) {
        (**dtors)();
        dtors++;
    }
}

/* Starts the mode-selection music, initializes the choice camera, and opens the screen with a
 * wipe. */
BOOL fn_1_B304(void)
{
    HuPrcSleep(5);
    fn_1_FEC(-1);
    lbl_1_bss_1A2C = HuAudSStreamPlay(MSM_STREAM_MODESEL);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    WipeWait();
    return TRUE;
}

/* Sets the starting menu choice from the previous overlay, or selects the first choice on a fresh
 * entry. */
s16 fn_1_B414(void)
{
    s16 i;

    if (lbl_1_bss_0 == 0) {
        HuAudFXPlayPan(MSM_SE_GUIDE_26, (MSM_PAN_LEFT + MSM_PAN_CENTER) / 2);
        HuAudFXPlayPan(MSM_SE_GUIDE_18, (MSM_PAN_CENTER + MSM_PAN_RIGHT) / 2);
        fn_1_23E0(1, MESSNUM(MESS_ALL_MAIN_MENU, 0), 1);
        fn_1_2288();
        fn_1_21DC();
        lbl_1_bss_1A30[0] = 0;
        lbl_1_bss_1A30[1] = 1;
        lbl_1_bss_1A30[2] = 1;
    } else {
        HuPrcSleep(10);
        lbl_1_bss_1A30[0] = (s16)lbl_1_bss_4;
        for (i = 0; i < 6; i++) {
            if (lbl_1_bss_4 == lbl_1_data_70[i]) {
                break;
            }
        }
        lbl_1_bss_1A30[1] = i % 3;
        lbl_1_bss_1A30[2] = i / 3;
    }
    return TRUE;
}

/* Displays the alternate menu prompt and returns its window choice. */
s16 fn_1_B804(void)
{
    HuAudFXPlayPan(MSM_SE_GUIDE_28, (MSM_PAN_LEFT + MSM_PAN_CENTER) / 2);
    HuAudFXPlayPan(MSM_SE_GUIDE_20, (MSM_PAN_CENTER + MSM_PAN_RIGHT) / 2);
    fn_1_23E0(1, MESSNUM(MESS_ALL_MAIN_MENU, 1), 1);
    return fn_1_22E8(2);
}

inline s16 fn_1_B804(void);

/* Called by fn_1_E7B0 in the menu process; updates the choice and guide display until accept or
 * cancel. */
s16 fn_1_BAB4(void)
{
    s16 index;
    s16 inputDelay;
    s16 result;
    s16 previousColumn;

    inputDelay = 10;
    result = 0;
    previousColumn = 1;

restart:
    HuPrcVSleep();
    index = lbl_1_bss_1A30[1] + (3 * lbl_1_bss_1A30[2]);
    lbl_1_bss_1A30[0] = lbl_1_data_70[index];
    fn_1_FEC(index);
    fn_1_9160(&lbl_1_data_28[index]);
    fn_1_25F8(MESSNUM(MESS_SYS_GUIDE, 2));
    fn_1_23E0(1, MESSNUM(MESS_ALL_MAIN_MENU, 2) + lbl_1_bss_1A30[0], 0);
    fn_1_8D44();
    lbl_1_bss_3E = 1;

    for (;;) {
        HuPrcVSleep();
        fn_1_75A4(0);
        index = lbl_1_bss_1A30[1] + (3 * lbl_1_bss_1A30[2]);
        lbl_1_bss_1A30[0] = lbl_1_data_70[index];
        fn_1_FEC(index);
        fn_1_90FC(&lbl_1_data_28[index]);
        fn_1_8D44();

        if (lbl_1_bss_1A30[1] == 0) {
            if (previousColumn != lbl_1_bss_1A30[1]) {
                previousColumn = lbl_1_bss_1A30[1];
                Hu3DMotionShiftSet(lbl_1_bss_14->mdlId[0],
                    lbl_1_bss_14->mtnId[3], 0.0f, 5.0f,
                    HU3D_MOTATTR_LOOP);
                Hu3DMotionShiftSet(lbl_1_bss_18->mdlId[0],
                    lbl_1_bss_18->mtnId[2], 0.0f, 15.0f,
                    HU3D_MOTATTR_LOOP);
                Hu3DMotionShiftStartEndSet(
                    lbl_1_bss_18->mdlId[0], 20.0f, 100.0f);
            }
            fn_1_23E0(1, MESSNUM(MESS_ALL_MAIN_MENU, 2) + lbl_1_bss_1A30[0], 0);
        } else if (lbl_1_bss_1A30[1] == 2) {
            if (previousColumn != lbl_1_bss_1A30[1]) {
                previousColumn = lbl_1_bss_1A30[1];
                Hu3DMotionShiftSet(lbl_1_bss_14->mdlId[0],
                    lbl_1_bss_14->mtnId[2], 0.0f, 15.0f,
                    HU3D_MOTATTR_LOOP);
                Hu3DMotionShiftSet(lbl_1_bss_18->mdlId[0],
                    lbl_1_bss_18->mtnId[3], 0.0f, 5.0f,
                    HU3D_MOTATTR_LOOP);
                Hu3DMotionShiftStartEndSet(
                    lbl_1_bss_14->mdlId[0], 20.0f, 100.0f);
            }
            fn_1_23E0(1, MESSNUM(MESS_ALL_MAIN_MENU, 2) + lbl_1_bss_1A30[0], 0);
        } else {
            if (previousColumn != lbl_1_bss_1A30[1]) {
                previousColumn = lbl_1_bss_1A30[1];
                Hu3DMotionShiftSet(lbl_1_bss_14->mdlId[0],
                    lbl_1_bss_14->mtnId[0], 0.0f, 15.0f,
                    HU3D_MOTATTR_LOOP);
                Hu3DMotionShiftSet(lbl_1_bss_18->mdlId[0],
                    lbl_1_bss_18->mtnId[0], 0.0f, 15.0f,
                    HU3D_MOTATTR_LOOP);
            }
            fn_1_23E0(1, MESSNUM(MESS_ALL_MAIN_MENU, 2) + lbl_1_bss_1A30[0], 0);
        }

        if (++inputDelay < 5) {
            continue;
        }
        inputDelay = 11;

        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            if (lbl_1_bss_1A30[1] > 0) {
                HuAudFXPlay(MSM_SE_CMN_01);
                lbl_1_bss_1A30[1]--;
                inputDelay = 0;
            } else if (lbl_1_bss_1A30[2] == 0) {
                HuAudFXPlay(MSM_SE_CMN_01);
                lbl_1_bss_1A30[2] = 1;
                inputDelay = 0;
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            if (lbl_1_bss_1A30[1] < 2) {
                HuAudFXPlay(MSM_SE_CMN_01);
                lbl_1_bss_1A30[1]++;
                inputDelay = 0;
            } else if (lbl_1_bss_1A30[2] == 0) {
                HuAudFXPlay(MSM_SE_CMN_01);
                lbl_1_bss_1A30[2] = 1;
                inputDelay = 0;
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            if (lbl_1_bss_1A30[2] < 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
                lbl_1_bss_1A30[2]++;
                inputDelay = 0;
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            if (lbl_1_bss_1A30[2] > 0) {
                HuAudFXPlay(MSM_SE_CMN_01);
                lbl_1_bss_1A30[2]--;
                inputDelay = 0;
            }
        } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
            if (lbl_1_bss_1A30[0] == 0 || lbl_1_bss_1A30[0] == 1) {
                if (GwCommon.mic == 1 && !fn_1_6E54()) {
                    OSReport(lbl_1_data_308);
                    HuMCMicSet(0);
                }
                HuAudFXPlay(MSM_SE_CMN_03);
                if (lbl_1_bss_1A30[0] == 0 || lbl_1_bss_1A30[0] == 5) {
                    HuAudFXPlayPan(
                        MSM_SE_GUIDE_26, (MSM_PAN_LEFT + MSM_PAN_CENTER) / 2);
                    HuAudFXPlayPan(
                        MSM_SE_GUIDE_18, (MSM_PAN_CENTER + MSM_PAN_RIGHT) / 2);
                } else if (lbl_1_bss_1A30[0] == 1
                    || lbl_1_bss_1A30[0] == 3) {
                    HuAudFXPlay(MSM_SE_GUIDE_26);
                } else {
                    HuAudFXPlay(MSM_SE_GUIDE_18);
                }
                result = 1;
                break;
            }

            if (lbl_1_bss_1A30[0] == 3) {
                if (GwCommon.mic == 2) {
                    HuAudFXPlay(MSM_SE_CMN_03);
                    HuAudFXPlay(MSM_SE_GUIDE_26);
                    result = 1;
                    break;
                }
                if (GwCommon.mic == 0) {
                    HuAudFXPlay(MSM_SE_CMN_05);
                    fn_1_23E0(1, MESSNUM(MESS_ALL_MAIN_MENU, 9), 1);
                    fn_1_2288();
                    fn_1_21DC();
                } else if (!fn_1_6E54()) {
                    OSReport(lbl_1_data_333);
                    HuAudFXPlay(MSM_SE_CMN_05);
                    fn_1_23E0(1, MESSNUM(MESS_ALL_MAIN_MENU, 8), 1);
                    fn_1_2288();
                    fn_1_21DC();
                } else {
                    HuAudFXPlay(MSM_SE_CMN_03);
                    HuAudFXPlay(MSM_SE_GUIDE_26);
                    result = 1;
                    break;
                }
            } else {
                HuAudFXPlay(MSM_SE_CMN_03);
                if (lbl_1_bss_1A30[0] == 0 || lbl_1_bss_1A30[0] == 5) {
                    HuAudFXPlayPan(
                        MSM_SE_GUIDE_26, (MSM_PAN_LEFT + MSM_PAN_CENTER) / 2);
                    HuAudFXPlayPan(
                        MSM_SE_GUIDE_18, (MSM_PAN_CENTER + MSM_PAN_RIGHT) / 2);
                } else if (lbl_1_bss_1A30[0] == 1
                    || lbl_1_bss_1A30[0] == 3) {
                    HuAudFXPlay(MSM_SE_GUIDE_26);
                } else {
                    HuAudFXPlay(MSM_SE_GUIDE_18);
                }
                result = 1;
                break;
            }
        } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
            HuAudFXPlay(MSM_SE_CMN_04);
            result = -1;
            break;
        }

        index = lbl_1_bss_1A30[1] + (3 * lbl_1_bss_1A30[2]);
        lbl_1_bss_1A30[0] = lbl_1_data_70[index];
        fn_1_8D44();
    }

    lbl_1_bss_3E = 0;
    if (result == -1) {
        previousColumn = -1;
        Hu3DMotionShiftSet(lbl_1_bss_14->mdlId[0],
            lbl_1_bss_14->mtnId[0], 0.0f, 30.0f, HU3D_MOTATTR_LOOP);
        Hu3DMotionShiftSet(lbl_1_bss_18->mdlId[0],
            lbl_1_bss_18->mtnId[0], 0.0f, 30.0f, HU3D_MOTATTR_LOOP);
        fn_1_75A4(-1);
    }
    fn_1_FEC(-1);
    fn_1_927C();
    fn_1_8E38();
    fn_1_277C();

    if (result == -1) {
        if (fn_1_B804()) {
            goto restart;
        }
        result = -1;
    }
    return result;
}

/* Closes the menu prompt and animates the selected display models before the chosen mode starts. */
void fn_1_E1FC(void)
{
    MDSEL_BEZIER_WORK *workA;
    MDSEL_BEZIER_WORK *workB;
    OMOBJ *objA;
    OMOBJ *objB;
    s16 indexA;
    s16 indexB;
    s16 windowNo;
    float yA;
    float yB;

    lbl_1_data_15C = 0;
    HuPrcVSleep(60);
    if (lbl_1_data_150[0] != -1) {
        windowNo = lbl_1_data_150[0];
        if (windowNo == 0) {
            HuWinDispOff(lbl_1_bss_1A40[windowNo]);
        } else {
            HuWinExClose(lbl_1_bss_1A40[windowNo]);
        }
    }
    lbl_1_data_150[0] = -1;
    lbl_1_data_154[0] = -1;

    objA = lbl_1_bss_14;
    workA = &lbl_1_bss_128;
    if (lbl_1_bss_1A30[1] != 2) {
        yA = 200.0f;
        if (lbl_1_bss_1A30[0] == 5) {
            yA = 400.0f;
        }
        indexA = lbl_1_bss_1A30[1] + (3 * lbl_1_bss_1A30[2]);
        workA->control[0].x = -1150.0f;
        workA->control[0].y = 350.0f;
        workA->control[0].z = -1000.0f;
        workA->control[1].x = -lbl_1_data_28[indexA].x;
        workA->control[1].y = 2250.0f;
        workA->control[1].z = 6000.0f;
        workA->control[2].x = lbl_1_data_28[indexA].x;
        workA->control[2].y = yA;
        workA->control[2].z = lbl_1_data_28[indexA].z;
        workA->time = 0.0f;
        workA->duration = 90.0f;
        Hu3DMotionShiftSet(
            objA->mdlId[0], objA->mtnId[1], 0.0f, 5.0f, 0);
        lbl_1_bss_19DC[0] = HuAudFXPlay(MSM_SE_BRD00_146);
        objA->objFunc = fn_1_7868;
    }

    HuPrcSleep(10);
    objB = lbl_1_bss_18;
    workB = &lbl_1_bss_A0;
    if (lbl_1_bss_1A30[1] != 0) {
        yB = 100.0f;
        if (lbl_1_bss_1A30[0] == 5) {
            yB = 400.0f;
        }
        indexB = lbl_1_bss_1A30[1] + (3 * lbl_1_bss_1A30[2]);
        workB->control[0].x = 1150.0f;
        workB->control[0].y = 350.0f;
        workB->control[0].z = -1000.0f;
        workB->control[1].x = -lbl_1_data_28[indexB].x;
        workB->control[1].y = 2250.0f;
        workB->control[1].z = 6000.0f;
        workB->control[2].x = lbl_1_data_28[indexB].x;
        workB->control[2].y = yB;
        workB->control[2].z = lbl_1_data_28[indexB].z;
        workB->time = 0.0f;
        workB->duration = 90.0f;
        Hu3DMotionShiftSet(
            objB->mdlId[0], objB->mtnId[1], 0.0f, 5.0f, 0);
        lbl_1_bss_19DC[1] = HuAudFXPlay(MSM_SE_BRD00_144);
        objB->objFunc = fn_1_809C;
    }

    HuPrcSleep(80);
    fn_1_11D4();
    HuPrcSleep(50);
}

/* Runs the opening sequence, initial prompt, choice loop, and selection transition for the menu
 * process. */
s16 fn_1_E7B0(void)
{
    s16 result = 0;

    fn_1_B304();
    fn_1_B414();
    result = fn_1_BAB4();
    if (result != -1) {
        fn_1_E1FC();
    }
    return result;
}

/* Shows or hides the five particle models belonging to one mode-choice group. */
void fn_1_ECAC(s16 groupNo, s16 show)
{
    s16 i;

    for (i = 0; i < 5; i++) {
        if (show) {
            Hu3DModelAttrReset(lbl_1_bss_44[groupNo][i], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_44[groupNo][i], HU3D_ATTR_DISPOFF);
        }
    }
}

/* Configures the particle models in a choice group for the current menu display state. */
void fn_1_ED60(s16 groupNo, HuVecF *spawnPosition, GXColor *color)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *emitter;

    for (i = 0; i < 5; i++) {
        model = &Hu3DData[lbl_1_bss_44[groupNo][i]];
        emitter = model->hookData;
        emitter->dataCnt = 1;
        if (color != NULL) {
            emitter->pos.x = color->r;
            emitter->pos.y = color->g;
            emitter->pos.z = color->b;
        }
        if (spawnPosition != NULL) {
            /* This emitter vector is used as the center for new particles' spawn positions. */
            emitter->spawnCenter.x = spawnPosition->x;
            emitter->spawnCenter.y = spawnPosition->y;
            emitter->spawnCenter.z = spawnPosition->z;
        }
        Hu3DModelAttrReset(lbl_1_bss_44[groupNo][i], HU3D_ATTR_DISPOFF);
    }
}

inline void fn_1_ED60(s16 groupNo, HuVecF *spawnPosition, GXColor *color);

/* Clears the per-particle hook state for each model in a choice group. */
void fn_1_EEC8(s16 groupNo)
{
    s16 i;
    HU3D_MODEL *model;
    s16 *work;

    for (i = 0; i < 5; i++) {
        model = &Hu3DData[lbl_1_bss_44[groupNo][i]];
        work = model->hookData;
        *work = 0;
    }
}

/* fn_1_F4E4 installs this hook; the renderer calls it once per frame to spawn or advance
 * choice-group particles. */
void fn_1_EF48(HU3D_MODEL *model, HU3D_PARTICLE *emitter, Mtx matrix)
{
    HU3D_PARTICLE_DATA *particleData;
    s16 i;
    s16 spawnCount = 0;
    float colorComponent;
    float randomValue;

    if (emitter->count == 0) {
        for (i = 0, particleData = emitter->data; i < emitter->maxCnt; i++, particleData++) {
            particleData->time = 0;
        }
        emitter->dataCnt = 1;
        emitter->pos.x = 255.0f;
        emitter->pos.y = 255.0f;
        emitter->pos.z = 255.0f;
    }

    for (i = 0, particleData = emitter->data; i < emitter->maxCnt; i++, particleData++) {
        if (particleData->time == 0 && emitter->dataCnt == 1 && spawnCount < 1) {
            spawnCount++;
            particleData->time = 1;
            particleData->vel.x = 0.0f;
            particleData->vel.y = frandmod(30) + 30;
            particleData->accel.x = frandmod(100) - 50;
            particleData->accel.y = -frandmod(100) - 50;
            particleData->accel.z = frandmod(100) - 50;
            PSVECNormalize(&particleData->accel, &particleData->accel);
            particleData->accel.x *= 2.0f;
            particleData->accel.z *= 2.0f;
            particleData->colorIdx = frandmod(10) + 5;
            /* The source vector is the spawn center; x and z spread by about 150 model units. */
            particleData->pos.x = emitter->spawnCenter.x + 3 * (frandmod(100) - 50);
            particleData->pos.y = emitter->spawnCenter.y;
            particleData->pos.z = emitter->spawnCenter.z + 3 * (frandmod(100) - 50);
            randomValue = frandmod(32);
            colorComponent = emitter->pos.x + randomValue;
            if (colorComponent > 255.0f) {
                colorComponent = 255.0f;
            }
            particleData->color.r = colorComponent;
            colorComponent = emitter->pos.y + randomValue;
            if (colorComponent > 255.0f) {
                colorComponent = 255.0f;
            }
            particleData->color.g = colorComponent;
            colorComponent = emitter->pos.z + randomValue;
            if (colorComponent > 255.0f) {
                colorComponent = 255.0f;
            }
            particleData->color.b = colorComponent;
            particleData->color.a = 0;
        } else if (particleData->time == 1) {
            particleData->pos.y += particleData->colorIdx;
            particleData->colorIdx += particleData->accel.y;
            if (rand8() % 5 == 0) {
                particleData->zRot = MTXDegToRad(frandmod(360));
                particleData->color.a = frandmod(127) + 128;
            }
            randomValue = fn_1_C48(1.0f, 0.0f, particleData->vel.x, particleData->vel.y);
            particleData->scale = 100.0f * randomValue;
            if (++particleData->vel.x > particleData->vel.y) {
                particleData->time = 0;
                particleData->scale = 0.0f;
                particleData->color.a = 0;
            }
        }
    }
    DCFlushRangeNoSync(
        emitter->data, emitter->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Creates particle emitters for the six choice groups and assigns their layer, position, and blend
 * mode. */
void fn_1_F4E4(void)
{
    s16 particleCount[5] = { 10, 10, 10, 10, 256 };
    s16 groupNo;
    s16 particleNo;

    for (groupNo = 0; groupNo < 6; groupNo++) {
        for (particleNo = 0; particleNo < 5; particleNo++) {
            lbl_1_bss_44[groupNo][particleNo] = Hu3DParticleCreate(
                lbl_1_bss_80[particleNo], particleCount[particleNo]);
            Hu3DModelPosSet(
                lbl_1_bss_44[groupNo][particleNo], 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(
                lbl_1_bss_44[groupNo][particleNo], 1.0f, 1.0f, 1.0f);
            Hu3DModelAttrSet(
                lbl_1_bss_44[groupNo][particleNo], HU3D_ATTR_DISPOFF);
            Hu3DModelLayerSet(lbl_1_bss_44[groupNo][particleNo], 2);
            Hu3DParticleHookSet(lbl_1_bss_44[groupNo][particleNo], fn_1_EF48);
            Hu3DParticleBlendModeSet(
                lbl_1_bss_44[groupNo][particleNo], HU3D_PARTICLE_BLEND_ADDCOL);
        }
    }
}

inline void fn_1_F4E4(void);

/* Kills the particle models created for the six choice groups. */
void fn_1_F70C(void)
{
    s16 i;
    s16 j;

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 5; j++) {
            Hu3DModelKill(lbl_1_bss_44[i][j]);
        }
    }
}

/* Loads the five choice-group sprite animations and creates the menu particle emitters. */
void fn_1_F790(void)
{
    s16 i;

    for (i = 0; i < 5; i++) {
        lbl_1_bss_80[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_35C[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    fn_1_F4E4();
}

/* Kills the stored particle models for all six choice groups. */
void fn_1_FA18(void)
{
    s16 j;
    s16 i;

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 5; j++) {
            Hu3DModelKill(lbl_1_bss_44[i][j]);
        }
    }
}

/* Updates each choice group's particle visibility, position, and emission state for the menu
 * mode. */
void fn_1_FA9C(s16 groupNo, HuVecF *pos, s16 mode, s16 colorNo)
{
    GXColor colors[7] = {
        { 254, 77, 75, 0 },
        { 50, 127, 200, 0 },
        { 199, 175, 0, 0 },
        { 52, 192, 63, 0 },
        { 159, 93, 200, 0 },
        { 255, 114, 46, 0 },
        { 109, 207, 246, 0 },
    };

    if (mode == 1) {
        fn_1_ED60(groupNo, pos, &colors[colorNo]);
    } else if (mode == 0) {
        fn_1_EEC8(groupNo);
    } else if (mode == 2) {
        fn_1_ECAC(groupNo, FALSE);
    }
}

/* Plays a sound with stereo pan calculated from a supplied 3D position. */
void fn_1_FDF8(HuVecF *pos, s32 fxNo)
{
    HuVecF screenPos;
    s16 pan;

    Hu3D3Dto2D(pos, 1, &screenPos);
    screenPos.x -= 288.0f;
    screenPos.x /= 288.0f;
    pan = 64.0f + (16.0f * screenPos.x);
    if (pan < 48) {
        pan = 48;
    }
    if (pan > 72) {
        pan = 72;
    }
    HuAudFXPlayPan(fxNo, pan);
}

/* Plays a sound at a model's screen position, or plays it without panning when no model is
 * supplied. */
void fn_1_FEC0(HU3D_MODELID modelId, s32 fxNo, s16 panRange, s16 volume)
{
    HuVecF modelPos;
    HuVecF screenPos;
    s16 pan;

    if (modelId != HU3D_MODELID_NONE) {
        Hu3DModelPosGet(modelId, &modelPos);
        Hu3D3Dto2D(&modelPos, 1, &screenPos);
        screenPos.x -= 288.0f;
        screenPos.x /= 288.0f;
        pan = 64.0f + (screenPos.x * panRange);
        if (pan < 64 - panRange) {
            pan = 64 - panRange;
        }
        if (pan > 64 + panRange) {
            pan = 64 + panRange;
        }
        if (volume == -1) {
            HuAudFXPlayPan(fxNo, pan);
        } else {
            HuAudFXPlayVolPan(fxNo, volume, pan);
        }
    } else {
        HuAudFXPlay(fxNo);
    }
}

u32 lbl_1_data_0[1] = { DATANUM(DATA_mdsel, 10) };
s16 lbl_1_data_4[2] = { 1, 0 };
LBL_1_DATA_8_ENTRY lbl_1_data_8[1] = {
    { 0, 0, 0, 0, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
};
HuVecF lbl_1_data_28[6] = {
    { -670.0f, 0.0f, -800.0f },
    { 0.0f, 200.0f, -1100.0f },
    { 650.0f, 0.0f, -780.0f },
    { -1156.0f, -200.0f, -27.0f },
    { 0.0f, 0.0f, 550.0f },
    { 1200.0f, 0.0f, 0.0f },
};
s16 lbl_1_data_70[6] = { 3, 5, 4, 1, 0, 2 };
s32 lbl_1_data_7C = -1;
char lbl_1_data_80[] = "# ========== win callback :: %d\n";
char lbl_1_data_A1[] = ">>>>>>>>>> mdseldll :: objsetup!! <<<<<<<<<<\n";
char lbl_1_data_CF[] = "0x%x :: _effect\n";
char lbl_1_data_E0[] = "0x%x :: _gamemes\n";
char lbl_1_data_F2[] = "0x%x :: _mgconst\n";
char lbl_1_data_104[] = "0x%x :: _win\n";
char lbl_1_data_112[] = "\n";
char lbl_1_data_114[MDSEL_DLL_RETURN_REPORT_SIZE] =
    "\n>>>>>>>>>> mdseldll :: dllreturn or dllmove!! <<<<<<<<<<\n";
s16 lbl_1_data_150[2] = { -1, -1 };
s32 lbl_1_data_154[2] = { -1, -1 };
s16 lbl_1_data_15C = 1;
char lbl_1_data_15E[] = "Result %d\n";
char lbl_1_data_169[] =
    "MIC SETTING >>> err!! <<< :: HuMCMount(B) != MIC_RESULT_READY\n";
char lbl_1_data_1A8[] =
    "MIC SETTING >>> check <<< :: HuMCMount(B) == MIC_RESULT_READY\n";
char lbl_1_data_1E7[] =
    "MIC SETTING >>> err!! <<< :: GwCommon.mic != HUMC_FLAG_USE\n";
char lbl_1_data_223[] =
    "MIC SETTING >>> ok!! <<< :: GwCommon.mic == HUMC_FLAG_USE\n";
char lbl_1_data_25E[] = "/mic/ctx/mselect_words";
char lbl_1_data_275[] = "mic ok!!\n";
char lbl_1_data_27F[] =
    "MIC SETTING >>> ok!! <<< :: HuMCMount(B) == MIC_RESULT_READY\n";
char lbl_1_data_2BD[MDSEL_ITEM_HOOK_NAME_SIZE] = "gN01m1-itemhook_R";
s16 lbl_1_data_2D0 = -1;
char lbl_1_data_2D2[] = "\n-----===== MARIO PARTY 6 :: MODE SELECT =====-----\n\n";
char lbl_1_data_308[] = "MIC CHECK!! >> nouse!! << :: device err!!\n";
char lbl_1_data_333[] = "MIC CHECK!! >> use!! << :: device error\n";
u32 lbl_1_data_35C[5] = {
    DATANUM(DATA_mdsel, 37),
    DATANUM(DATA_mdsel, 37),
    DATANUM(DATA_mdsel, 37),
    DATANUM(DATA_mdsel, 37),
    DATANUM(DATA_mdsel, 37),
};

s16 lbl_1_bss_1A4C;
HU3D_LIGHTID lbl_1_bss_1A48[2];
HUWINID lbl_1_bss_1A40[4];
ANIMDATA *lbl_1_bss_1A3C[1];
HUSPR_GROUPID lbl_1_bss_1A38[1];
HUSPRID lbl_1_bss_1A36[1];
s16 lbl_1_bss_1A30[3];
s32 lbl_1_bss_1A2C;
s32 lbl_1_bss_1A28;
s16 lbl_1_bss_19EC[30];
s32 lbl_1_bss_19DC[4];
HuVecF lbl_1_bss_19AC[4];
LBL_1_BSS_8AC_ENTRY lbl_1_bss_9BC[30];
LBL_1_BSS_8AC_ENTRY lbl_1_bss_8AC[2];
LBL_1_BSS_24C_ENTRY lbl_1_bss_24C[12];
LBL_1_BSS_1D4_ENTRY lbl_1_bss_1D4[15];
s16 lbl_1_bss_1B0[6][3];
MDSEL_BEZIER_WORK lbl_1_bss_128;
MDSEL_BEZIER_WORK lbl_1_bss_A0;
HuVecF lbl_1_bss_94;
ANIMDATA *lbl_1_bss_80[5];
HU3D_MODELID lbl_1_bss_44[6][5];
float lbl_1_bss_40;
s16 lbl_1_bss_3E;
s16 lbl_1_bss_3C;
s16 lbl_1_bss_3A;
s16 lbl_1_bss_38;
OMOBJ *lbl_1_bss_34;
OMOBJ *lbl_1_bss_30;
float lbl_1_bss_2C;
void *lbl_1_bss_28;
OMOBJ *lbl_1_bss_20[2];
OMOBJ *lbl_1_bss_1C;
OMOBJ *lbl_1_bss_18;
OMOBJ *lbl_1_bss_14;
OMOBJ *lbl_1_bss_10;
OMOBJ *lbl_1_bss_C;
OMOBJMAN *lbl_1_bss_8;
int lbl_1_bss_4;
int lbl_1_bss_0;
