
/* Menu helpers for the Star Bank screens: the camera work object, sprite groups, character models
 * and particle effects. */
#include "math.h"

#include "include/dolphin.h"

#include "include/game/audio.h"

#include "include/game/data.h"

#include "include/game/gamework.h"

#include "include/game/hu3d.h"

#include "include/game/object.h"

#include "include/game/process.h"

#include "include/game/saveload.h"

#include "include/game/sprite.h"

#include "include/game/window.h"

#include "include/game/wipe.h"

#include "include/messdir_enum.h"

#include <string.h>
#include "datadir_enum.h"
#include "dolphin/os.h"
#include "REL/mdpartyDll.h"
#include "game/armem.h"
#include "game/board/main.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/printfunc.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"
#include "messdir_enum.h"
#include "dolphin/mic.h"
#include "game/mgdata.h"
#include "msm.h"
#include "msm_se.h"
#include "msm_stream.h"
#include "dolphin.h"
#include "game/board/audio.h"
#include "game/process.h"
#include "include/humath.h"
#include "REL/mdbankdll.h"
#include "game/mic.h"
#include "game/frand.h"
#include "game/main.h"
#include "humath.h"

extern s16 lbl_1_data_932[3];

extern s32 lbl_1_data_938[3];

extern s16 lbl_1_bss_198C[4];

static inline void fn_1_A18(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_198C[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_198C[winNo]);
    }
}

inline void fn_1_A18(s16 winNo);

extern s16 lbl_1_bss_19E8;
extern s16 lbl_1_bss_19EA;
extern s16 lbl_1_bss_19EC;
extern ANIMDATA *lbl_1_bss_19F0[2];
extern s32 lbl_1_data_A20[2];

float fn_1_1116C(float start, float end, float time, float duration);
float fn_1_111B0(float current, float target, float weight);
void fn_1_111F8(HuVecF *current, const HuVecF *target, float weight);
float fn_1_1161C(float start, float control, float end, float weight);
void fn_1_11678(HuVecF *out, const HuVecF *start, const HuVecF *control, const HuVecF *end,
    float weight);
void fn_1_11880(HUSPR_GROUPID groupId, s32 attr);
void fn_1_11900(HUSPR_GROUPID groupId, s32 attr);
void fn_1_119C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_12230(HuVecF *pos, GXColor *color, float velocityY, float accelY);
void fn_1_12354(float velocityY, float velocityZ, HuVecF *pos, GXColor *color);
void fn_1_124F4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_12984(void);
void fn_1_12A94(float x, float y, float z);
void fn_1_12AE8(s16 time);
void fn_1_12B60(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_12D98(void);
void fn_1_12EA8(void);
void fn_1_1317C(void);
void fn_1_131C8(HuVecF *pos);
void fn_1_13598(HuVecF *position);

float fn_1_11064(float start, float end, float time, float duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return (float)(start + ((end - start) *
        (1.0 - cos((M_PI *
         ((90.0f / duration) * time)) / 180.0))));
}

double acos(double x);

float fn_1_220(float x);

void fn_1_1115C(HuVecF *out, f32 x, f32 y, f32 z)
{
    out->x = x;
    out->y = y;
    out->z = z;
}

#define sind(x) sin((M_PI * (x)) / 180.0)

#define cosd(x) cos((M_PI * (x)) / 180.0)

#define MDPARTY_MSG_SETUP_FIRST 0xD0000

#define MDPARTY_MSG_SETUP_SECOND 0xD0001

#define MDPARTY_MSG_SETUP_PLAYER_TYPE 0xD0008

#define MDPARTY_MSG_PLAYER_COUNT_BASE 0xD0009

#define MDPARTY_MSG_MODE_CONFIRM 0xD000E

#define MDPARTY_MSG_PLAYER_COUNT_ONE 0xD000F

#define MDPARTY_MSG_PLAYER_COUNT_MAX 0xD0013

#define MDPARTY_MSG_PLAYER_COUNT_SELECTED 0xD0014

#define MDPARTY_MSG_PLAYER_TURN_ORDER 0xD0015

#define MDPARTY_MSG_NEW_GAME_SELECTED 0xD0025

#define MDPARTY_MSG_SAVED_GAME_SETUP_OPTION 0xD0026

#define MDPARTY_MSG_SAVED_GAME_CONTINUE_OPTION 0xD0002

#define MDPARTY_MSG_CONTINUE_SAVED_GAME 0xD0004

#define MDPARTY_MSG_NEXT_PARTY_PROMPT 0xD0005

#define MDPARTY_MSG_PLAYER_OPTION_BASE 0x20009

#define MDPARTY_MSG_ALTERNATE_PLAYER_OPTION 0x110017

#define MDPARTY_MSG_PLAYER_COUNT_OPTION_BASE 0xD0006

#define MDPARTY_MSG_TURN_ORDER_SETUP 0xD0016

#define MDPARTY_MSG_OPTION_GROUP_TWO_BASE 0xD0017

#define MDPARTY_MSG_OPTION_GROUP_THREE_BASE 0x40000

#define MDPARTY_MSG_OPTION_GROUP_FOUR_BASE 0xD0020

#define MDPARTY_MSG_CHARACTER_SELECTION_DETAIL 0xD0022

#define MDPARTY_MSG_NEW_GAME_CHOICE 0xD0023

#define MDPARTY_MSG_SAVED_GAME_CHOICE 0xD0024

#define MDPARTY_MSG_STAGE_SELECTION_DETAIL 0xD0027

#define MDPARTY_STREAM_MODE 4

#define MDPARTY_MSG_CHARACTER_PAIR_BASE 0x30000

#define MDPARTY_MSG_NO_CHARACTER_PAIR 0x30037

#define MDPARTY_DATA_CHARACTER_SELECT_MODEL DATANUM(0x970000, 36)

#define MDPARTY_MSG_SELECT_PROMPT 0x1000A

typedef void (*VoidFunc)(void);

typedef struct MdCameraWork_s MDCAMERA_WORK;

typedef void (*MDCAMERA_CALLBACK)(OMOBJ *obj, MDCAMERA_WORK *camera);

typedef struct MdSprInfo_s {
    s16 groupNo;
    s16 memberNo;
    s16 animNo;
    s16 priority;
    s16 bank;
    HuVec2f pos;
    HuVec2f scale;
    float zRot;
} MDSPR_INFO;

typedef struct Lbl1DataE12Entry {
    s16 neighborCharacter[8];
    s16 selectionState;

} LBL_1_DATA_E12_ENTRY;

typedef struct Lbl1Bss228Entry {
    s16 state;
    s16 pad;
    HuVecF pos;
    HuVecF controlPos;
    HuVecF endPos;
    float time;
    float duration;
} LBL_1_BSS_228_ENTRY;

typedef struct Lbl1Bss1C8Entry {
    s16 state;
    s16 pad;
    HuVecF pos;
    HuVecF controlPos;
    HuVecF endPos;
    float time;
    float duration;
} LBL_1_BSS_1C8_ENTRY;

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

void fn_1_1B4(void);

void fn_1_2B0(void);

void fn_1_400(void);

void fn_1_64C(void);

void fn_1_718(void);

void fn_1_E30(void);

void fn_1_353C(void);

void fn_1_6284(OMOBJ *obj);

void fn_1_72C4(OMOBJ *obj);

void fn_1_109C(s16 nextFlow);

void fn_1_1248(void);

void fn_1_2D1D4(void);

void fn_1_2DD38(void);

void fn_1_2DFD4(OMOBJ *obj, MDCAMERA_WORK *camera);

void fn_1_2E338(OMOBJ *obj, MDCAMERA_WORK *camera);

void fn_1_2E560(OMOBJ *obj, MDCAMERA_WORK *camera);

s32 fn_1_2E5D4(void);

s16 fn_1_2F22C(void);

s16 fn_1_30B4C(void);

s16 fn_1_31AC0(s16 unused);

s16 fn_1_32904(void);

s16 fn_1_329A4(s16 returningFromPreviousStep);

s16 fn_1_335F0(s16 *activePlayer, s16 *firstActivePlayer, s16 *lastPlayerSlot);

s16 fn_1_33724(s16 unused);

s16 fn_1_34FC4(s16 *activePlayer, s16 *firstActivePlayer, s16 *lastPlayerSlot);

s16 fn_1_350F8(s16 unused);

s16 fn_1_36518(s16 returningFromPreviousStep);

void fn_1_37260(s16 *activePlayer, s16 *firstActivePlayer, s16 *lastPlayerSlot);

s16 fn_1_3736C(s16 returningFromPreviousStep);

s16 fn_1_38A3C(s16 returningToSelection);

s16 fn_1_39C60(s16 returningFromPreviousStep);

s16 fn_1_3AD84(s16 returningFromPreviousStep);

s16 fn_1_3CFE4(s16 returningFromPreviousStep);

s16 fn_1_3EAC8(s16 unused);

void ObjectSetup(void);

void fn_1_32D0(OMOBJ *obj);

void fn_1_467C(void);

void fn_1_4A84(OMOBJ *obj);

void fn_1_4EDC(OMOBJ *obj);

void fn_1_50EC(OMOBJ *obj);

void fn_1_5208(OMOBJ *obj);

void fn_1_5324(OMOBJ *obj);

void fn_1_5A18(OMOBJ *obj);

void fn_1_6324(OMOBJ *obj);

void fn_1_6A20(OMOBJ *obj);

void fn_1_737C(OMOBJ *obj);

void fn_1_605C(OMOBJ *obj);

void fn_1_7064(OMOBJ *obj);

void fn_1_9658(s16 winNo, u32 messNum);

void fn_1_A8D8(s16 memberNo, HuVecF *pos, float xOffset, float yOffset);

void fn_1_A970(s16 memberNo, HuVecF *pos, float xOffset, float yOffset);

void fn_1_AA94(s16 memberNo);

void fn_1_AAF0(s16 memberNo);

void fn_1_AD14(s16 modelNo, HuVecF *pos);

void fn_1_7ED4(OMOBJ *obj);

void fn_1_861C(OMOBJ *obj);

void fn_1_9218(OMOBJ *obj);

void fn_1_9F50(OMOBJ *obj);

void fn_1_A5D8(OMOBJ *obj);

void fn_1_B008(OMOBJ *obj);

void fn_1_B4C8(OMOBJ *obj);

s16 fn_1_B748(s16 currentCharacterNo, s16 directionNo,
    s16 alternateDirectionNo, s16 fallbackDirectionNo,
    LBL_1_DATA_E12_ENTRY *characterChoices, s16 characterCount);

s16 fn_1_BBD8(
    s16 padNo, s16 *current, LBL_1_DATA_E12_ENTRY *entries, s16 count);

void fn_1_C28C(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);

void fn_1_C698(s16 modelNo, s16 time, s16 duration);

void fn_1_CA68(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);

void fn_1_D058(s16 modelNo, s16 dataNo, s16 dataOffset, s16 time,
    s16 duration, s16 direction);

void fn_1_D748(void);

void fn_1_10518(OMOBJ *obj);

void fn_1_114C4(OMOBJ *obj);

void fn_1_114F4(OMOBJ *obj);

void fn_1_1158C(OMOBJ *obj);

void fn_1_11EAC(OMOBJ *obj);

void fn_1_12364(OMOBJ *obj);

void fn_1_12FD8(OMOBJ *obj);

void fn_1_12B18(OMOBJ *obj);

void fn_1_12D7C(OMOBJ *obj);

void fn_1_12EC8(OMOBJ *obj);

void fn_1_13EC0(OMOBJ *obj);

void fn_1_14344(OMOBJ *obj);

void fn_1_14510(OMOBJ *obj);

void fn_1_14CC0(OMOBJ *obj);

void fn_1_14F64(OMOBJ *obj);

void fn_1_150D0(OMOBJ *obj);

void fn_1_156B4(OMOBJ *obj);

void fn_1_160A0(OMOBJ *obj);

void fn_1_16A8C(OMOBJ *obj);

void fn_1_16E84(OMOBJ *obj);

void fn_1_1733C(s16 boardIndex);

void fn_1_17730(OMOBJ *obj);

void fn_1_17E10(OMOBJ *obj);

void fn_1_1807C(OMOBJ *obj);

void fn_1_18384(OMOBJ *obj);

void fn_1_18AA4(OMOBJ *obj);

void fn_1_1948C(OMOBJ *obj);

void fn_1_19860(OMOBJ *obj);

void fn_1_19D24(OMOBJ *obj);

void fn_1_1A20C(OMOBJ *obj);

void fn_1_1A8B0(OMOBJ *obj);

void fn_1_1AE60(OMOBJ *obj);

void fn_1_1B400(OMOBJ *obj);

void fn_1_1B9E8(OMOBJ *obj);

void fn_1_1D758(OMOBJ *obj);

void fn_1_1E7F8(OMOBJ *obj);

void fn_1_1F8B8(OMOBJ *obj);

void fn_1_21628(OMOBJ *obj);

void fn_1_219C8(OMOBJ *obj);

void fn_1_21C04(OMOBJ *obj);

void fn_1_21C9C(OMOBJ *obj);

void fn_1_22010(OMOBJ *obj);

void fn_1_221F4(OMOBJ *obj);

void fn_1_227E4(OMOBJ *obj);

void fn_1_228E8(OMOBJ *obj);

void fn_1_22EB8(OMOBJ *obj);

void fn_1_2318C(OMOBJ *obj);

void fn_1_23224(OMOBJ *obj);

void fn_1_232A0(OMOBJ *obj);

void fn_1_239DC(OMOBJ *obj);

void fn_1_23A00(OMOBJ *obj);

void fn_1_23AC0(OMOBJ *obj);

void fn_1_24090(OMOBJ *obj);

void fn_1_2457C(OMOBJ *obj);

void fn_1_26328(OMOBJ *obj);

void fn_1_2851C(OMOBJ *obj);

void fn_1_29598(OMOBJ *obj);

void fn_1_2A8C4(OMOBJ *obj);

void fn_1_2BBD8(OMOBJ *obj);

void fn_1_2C874(void);

void fn_1_2CE00(void);

s16 fn_1_2E638(void);

s16 fn_1_2EBEC(void);

s16 fn_1_2EF24(void);

s16 fn_1_31508(void);

s16 fn_1_38314(void);

s16 fn_1_3BEA8(void);

void fn_1_3CA70(void);

inline void fn_1_1B4(void);

inline void fn_1_2B0(void);

inline void fn_1_64C(void);

inline void fn_1_E30(void);

inline void fn_1_109C(s16 nextFlow);

inline void fn_1_1404(
    HuVecF *dst, const HuVecF *a, const HuVecF *b, const HuVecF *c, float t);

inline float fn_1_160C(float currentValue, float targetValue, float easingFactor);

inline void fn_1_163C(HuVecF *dst, const HuVecF *src, float weight);

inline float fn_1_1780(
    float startValue, float endValue, float elapsedFrames, float durationFrames);

inline float fn_1_1868(float startValue, float endValue, float elapsedFrames, float durationFrames);

inline void fn_1_18AC(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration);

inline void fn_1_1C14(HU3D_MODELID modelId, float rotY);

inline void fn_1_1D54(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY);

inline void fn_1_22A8(
    HU3D_MODELID modelId, const HuVecF *a, const HuVecF *b, const HuVecF *c,
    float time, float duration);

inline void fn_1_275C(void);

inline void fn_1_2810(MDCAMERA_WORK *camera);

inline void fn_1_2860(MDCAMERA_WORK *camera);

inline void fn_1_28B0(MDCAMERA_WORK *camera, float weight);

inline void fn_1_2B64(MDCAMERA_CALLBACK callback);

inline void fn_1_3284(OMOBJ *obj, MDCAMERA_WORK *camera);

inline void fn_1_33A0(MDCAMERA_CALLBACK callback);

inline void fn_1_353C(void);

inline void fn_1_3598(void);

inline void fn_1_36C4(void);

inline void fn_1_3700(s16 winNo);

inline void fn_1_3770(s16 winNo);

inline void fn_1_37E0(s16 winNo);

inline s16 fn_1_381C(s16 winNo, s16 mode);

inline void fn_1_38F0(s16 winNo, s32 messNum, s16 speed);

inline void fn_1_39AC(s16 winNo, s32 messNum, s16 insertPos);

inline void fn_1_3A1C(void);

inline void fn_1_3C44(void);

inline void fn_1_3CA0(s16 winNo);

inline void fn_1_3E0C(void);

inline void fn_1_3EC8(void);

inline s16 fn_1_3F28(s16 mode);

inline void fn_1_4020(s16 winNo, s32 messNum, s16 speed);

inline void fn_1_4258(s16 winNo, s32 messNum, s16 insertPos);

inline void fn_1_444C(s32 messNum);

inline void fn_1_45D0(void);

inline void fn_1_467C(void);

inline void fn_1_4734(HUSPR_GROUPID groupId, s32 attr);

inline void fn_1_47B4(HUSPR_GROUPID groupId, s32 attr);

inline void fn_1_4834(void);

inline void fn_1_4FE8(void);

inline void fn_1_5194(void);

inline void fn_1_52B0(void);

inline void fn_1_58A0(void);

inline s32 fn_1_5FB0(void);

inline void fn_1_6284(OMOBJ *obj);

inline void fn_1_68A4(void);

inline s32 fn_1_6FB8(void);

inline void fn_1_72C4(OMOBJ *obj);

inline void fn_1_7900(void);

inline void fn_1_8074(OMOBJ *obj);

inline void fn_1_852C(void);

inline void fn_1_87BC(OMOBJ *obj);

inline void fn_1_884C(s16 memberNo);

inline void fn_1_8950(void);

inline void fn_1_8980(void);

inline void fn_1_8C28(s16 animNo);

inline void fn_1_8CCC(void);

inline void fn_1_945C(OMOBJ *obj);

inline u32 fn_1_9558(s16 charNo1, s16 charNo2);

inline void fn_1_9658(s16 winNo, u32 messNum);

inline void fn_1_96E4(u32 messNum1, u32 messNum2);

inline void fn_1_9804(void);

inline void fn_1_98A4(void);

inline void fn_1_9A24(void);

inline void fn_1_9A7C(s16 playerNo, s16 bank);

inline void fn_1_9B90(s16 playerNo, s16 bankNo);

inline void fn_1_9BDC(s16 playerNo, s16 bank);

inline void fn_1_9D3C(s16 playerNo, s16 bank);

inline void fn_1_9E50(s16 teamPlayerNo, s16 handicap);

inline void fn_1_9E9C(void);

inline void fn_1_9F34(void);

inline void fn_1_A880(void);

inline void fn_1_A8D8(
    s16 memberNo, HuVecF *pos3D, float xOffset, float yOffset);

inline void fn_1_A970(
    s16 memberNo, HuVecF *pos3D, float xOffset, float yOffset);

inline void fn_1_AA94(s16 memberNo);

inline void fn_1_AAF0(s16 memberNo);

inline void fn_1_AB64(void);

inline void fn_1_AD14(s16 modelNo, HuVecF *pos);

inline void fn_1_AD9C(s32 modelNo, HuVecF *pos, s16 bank);

inline void fn_1_AF70(s32 modelNo);

inline void fn_1_B6C0(OMOBJ *obj);

inline void fn_1_C158(s16 modelNo, s16 animNo, s16 dataNo, s16 bank);

inline void fn_1_C28C(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);

inline void fn_1_C698(s16 modelNo, s16 time, s16 duration);

inline void fn_1_CA68(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);

inline void fn_1_D058(s16 modelNo, s16 dataNo, s16 dataOffset, s16 time,
    s16 duration, s16 direction);

inline void fn_1_D3E0(
    s16 modelNo, float angle, s16 time, s16 duration);

inline s16 fn_1_D678(void);

inline void fn_1_D6B0(OMOBJ_FUNC callback);

inline void fn_1_D748(void);

inline void fn_1_D978(void);

inline void fn_1_122FC(s16 modelNo);

inline s32 fn_1_2E5D4(void);

inline s16 fn_1_2EBEC(void);

inline s16 fn_1_2EF24(void);

inline s16 fn_1_32904(void);

inline s16 fn_1_34D10(s16 returningFromPreviousStep);

inline s16 fn_1_36264(s16 returningFromPreviousStep);

inline void fn_1_37260(s16 *activePlayer, s16 *firstActivePlayer, s16 *lastPlayer);

float fn_1_1116C(float start, float end, float time, float duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}

float fn_1_111B0(float current, float target, float weight)
{
    if (current == target || weight <= 1.0f) {
        return target;
    }
    return (target + (current * (weight - 1.0f))) / weight;
}

void fn_1_111F8(HuVecF *current, const HuVecF *target, float weight)
{
    current->x = fn_1_111B0(current->x, target->x, weight);
    current->y = fn_1_111B0(current->y, target->y, weight);
    current->z = fn_1_111B0(current->z, target->z, weight);
}

float fn_1_11384(float startValue, float peakValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f || elapsedFrames >= durationFrames) {
        return startValue;
    }
    return startValue +
           ((peakValue - startValue) * sin((M_PI *
            ((360.0f / durationFrames) * elapsedFrames)) / 180.0));
}

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

typedef void (*MCResponseCallback)(u16 *response);

typedef struct MdselMicResponse MDSEL_MIC_RESPONSE;

extern OMOBJ *lbl_1_bss_C;

extern OMOBJ *lbl_1_bss_10;

extern OMOBJ *lbl_1_bss_14;

extern OMOBJ *lbl_1_bss_18;

extern OMOBJ lbl_1_bss_20;

extern u32 lbl_1_bss_30;

extern s16 lbl_1_bss_3A;

extern s16 lbl_1_bss_3C;

extern s16 lbl_1_bss_3E;

extern float lbl_1_bss_40;

extern HU3D_MODELID lbl_1_bss_44[6][5];

extern HuVecF lbl_1_bss_94;

extern s16 lbl_1_bss_1B0[6][3];

extern HuVecF lbl_1_bss_19AC[4];

extern s32 lbl_1_bss_19DC[4];

extern s32 lbl_1_bss_1A28;

extern s32 lbl_1_bss_1A2C;

extern s16 lbl_1_bss_1A30[3];

extern HUSPRID lbl_1_bss_1A36[1];

extern HUSPR_GROUPID lbl_1_bss_1A38[1];

extern ANIMDATA *lbl_1_bss_1A3C[1];

extern HUWINID lbl_1_bss_1A40[4];

extern HU3D_LIGHTID lbl_1_bss_1A48[2];

extern s16 lbl_1_bss_1A4C;

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

inline void fn_1_40C(HUSPR_GROUPID groupId, s32 attr);

inline void fn_1_48C(HUSPR_GROUPID groupId, s32 attr);

inline void fn_1_598(HuVecF *dst, const HuVecF *src, float weight);

inline float fn_1_80C(float startValue, float endValue, float time, float duration);

inline float fn_1_8E8(float startValue, float endValue, float time, float duration);

inline void fn_1_C8C(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration);

inline void fn_1_11D4(void);

inline void fn_1_1734(void);

inline void fn_1_1988(void);

inline void fn_1_1B30(s16 winNo);

inline void fn_1_1BA0(s16 winNo);

inline void fn_1_1C10(s16 winNo);

inline s16 fn_1_1C4C(s16 winNo, s16 mode);

inline void fn_1_1D20(s16 winNo, s32 messNum, s16 speed);

inline void fn_1_2828(void);

inline void fn_1_3274(void);

inline void fn_1_33B8(void);

inline void fn_1_3910(void);

inline void fn_1_46DC(void);

inline void fn_1_485C(void);

inline void fn_1_6018(void);

inline void fn_1_6C04(void);

inline void fn_1_6DFC(void);

inline void fn_1_702C(OMOBJ *obj);

inline void fn_1_75A4(s16 modelIndex);

inline void fn_1_77D8(OMOBJ *obj);

inline void fn_1_7EC4(void);

inline void fn_1_86F8(void);

inline void fn_1_8A58(OMOBJ *obj);

inline void fn_1_8CA4(OMOBJ *obj);

inline void fn_1_8D44(void);

inline void fn_1_8E38(void);

inline void fn_1_8EC8(void);

inline void fn_1_90FC(HuVecF *pos);

inline void fn_1_9160(HuVecF *pos);

inline void fn_1_927C(void);

inline void fn_1_98B0(OMOBJ *obj);

inline s16 fn_1_B804(void);

inline void fn_1_ED60(s16 groupNo, HuVecF *spawnPosition, GXColor *color);

inline void fn_1_F4E4(void);

extern char lbl_1_data_114[MDSEL_DLL_RETURN_REPORT_SIZE];

extern char lbl_1_data_2BD[MDSEL_ITEM_HOOK_NAME_SIZE];

extern ANIMDATA * lbl_1_bss_1A3C[1];

extern OMOBJ * lbl_1_bss_18;

extern OMOBJ * lbl_1_bss_14;

extern OMOBJ * lbl_1_bss_10;

extern OMOBJ * lbl_1_bss_C;

float fn_1_11458(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((endValue - startValue) * sin((M_PI *
        ((90.0f / duration) * time)) / 180.0));
}

#define sind(x) sin((M_PI * (x)) / 180.0)

float fn_1_11540(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return startValue;
    }
    return startValue + ((endValue - startValue) * sin((M_PI *
        ((180.0f / duration) * time)) / 180.0));
}

float fn_1_1161C(float start, float control, float end, float weight)
{
    float inverse = 1.0f - weight;

    return (weight * weight * end)
        + ((inverse * inverse * start)
            + (2.0f * (control * (inverse * weight))));
}

void fn_1_11678(HuVecF *out, const HuVecF *start, const HuVecF *control, const HuVecF *end,
    float weight)
{
    out->x = fn_1_1161C(start->x, control->x, end->x, weight);
    out->y = fn_1_1161C(start->y, control->y, end->y, weight);
    out->z = fn_1_1161C(start->z, control->z, end->z, weight);
}

void fn_1_11880(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

void fn_1_11900(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}

void fn_1_11980(float posX, float posY, float posZ) {
    fn_80026760(lbl_1_bss_19EC, posX, posY, posZ);
}

extern ANIMDATA *lbl_1_bss_19F0[];

extern void fn_1_119C4(HU3D_MODEL *modelP, HU3D_PARTICLE *particleP, Mtx mtx);

void fn_1_119C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;
    s16 spawnCount = 0;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 0 && spawnCount == 0) {
            spawnCount = 1;
            data->time = 1;
            data->parManId = 0;
            data->vel.x = (float)(frandmod(3) + 1);
            data->vel.y = (float)(frandmod(3) + 1);
            data->vel.z = (float)(frandmod(3) + 1);
            data->accel.x = 0.0f;
            data->accel.y = (float)(frandmod(720) + 360);
            data->accel.z = (float)(frandmod(80) + 100);
            data->accel.x = (float)frandmod(360);
            data->speedDecay = 0.0f;
            data->colorIdx = (float)(frandmod(180) + 60);
            data->speedDecay = (float)frandmod(60);
            data->scale = 10.0f;
            data->color.r = 255;
            data->color.g = 255;
            data->color.b = 255;
        } else if (data->time == 1) {
            float amount;

            amount = fn_1_1116C(0.0f, 360.0f, data->accel.x, data->accel.y);
            data->pos.x = data->accel.z
                * sin(M_PI
                    * (amount * data->vel.x) / 180.0);
            data->pos.y = data->accel.z
                * cos(M_PI
                    * (amount * data->vel.y) / 180.0);
            data->pos.z = data->accel.z
                * sin(M_PI
                    * (amount * data->vel.z) / 180.0);
            data->scale = (float)(frandmod(30) + 10);
            if (rand8() % 10 == 0) {
                data->zRot = MTXDegToRad((float)frandmod(360));
            }
            data->color.a = (u8)fn_1_11540(0.0f, 255.0f, data->speedDecay, data->colorIdx);
            if ((data->accel.x += 1.0f) > data->accel.y) {
                data->vel.x = (float)(frandmod(4) + 1);
                data->vel.y = (float)(frandmod(4) + 1);
                data->vel.z = (float)(frandmod(4) + 1);
                data->accel.z = (float)(frandmod(80) + 100);
                data->accel.x = 0.0f;
                data->accel.y = (float)(frandmod(720) + 360);
            }
            if ((data->speedDecay += 1.0f) > data->colorIdx) {
                data->speedDecay = 0.0f;
                data->colorIdx = (float)(frandmod(180) + 60);
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

void fn_1_12120(void)
{
    lbl_1_bss_19EC = fn_8003216C(lbl_1_bss_19F0[0], 0x20);
    fn_80026760(lbl_1_bss_19EC, 0.0f, 375.0f, -100.0f);
    fn_800268B0(lbl_1_bss_19EC, 1.0f, 1.0f, 1.0f);
    fn_80026B8C(lbl_1_bss_19EC, 6);
    fn_800326D0(lbl_1_bss_19EC, fn_1_119C4);
    fn_8003269C(lbl_1_bss_19EC, 1);
}

void fn_1_12204(void)
{
    fn_800262D0(lbl_1_bss_19EC);
}

void fn_1_12230(HuVecF *pos, GXColor *color, float velocityY, float accelY)
{
    HU3D_MODEL *model = &lbl_802C01C8[lbl_1_bss_19EA];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 slot;

    for (slot = 0, data = particle->data;
         slot < particle->maxCnt; slot++, data++) {
        if (data->time == 0) {
            data->time = 1;
            break;
        }
    }
    if (slot == particle->maxCnt) {
        return;
    }
    data->color.r = color->r;
    data->color.g = color->g;
    data->color.b = color->b;
    data->color.a = 0;
    data->pos.x = pos->x;
    data->pos.y = pos->y;
    data->pos.z = pos->z;
    data->scale = 0.0f;
    data->vel.x = 0.0f;
    data->vel.y = velocityY;
    data->accel.x = accelY;
    data->accel.y = (float)color->a;
}

void fn_1_12354(float velocityY, float velocityZ, HuVecF *pos, GXColor *color)
{
    HU3D_MODEL *model = &lbl_802C01C8[lbl_1_bss_19EA];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 slot;

    for (slot = 0, data = particle->data;
         slot < particle->maxCnt; slot++, data++) {
        if (data->time == 0) {
            data->time = 2;
            break;
        }
    }
    if (slot == particle->maxCnt) {
        return;
    }
    data->color.r = color->r;
    data->color.g = color->g;
    data->color.b = color->b;
    data->color.a = color->a;
    data->pos.x = pos->x;
    data->pos.y = pos->y;
    data->pos.z = pos->z;
    data->vel.x = 0.0f;
    data->vel.y = velocityY;
    data->vel.z = velocityZ;
    data->accel.x = pos->x;
    data->accel.y = pos->y;
    data->accel.z = pos->z;
    data->effectDirection.x = (float)(frandmod(100) - 50);
    data->effectDirection.y = (float)(frandmod(100) - 50);
    data->effectDirection.z = 0.0f;
    PSVECNormalize(&data->effectDirection, &data->effectDirection);
}

void fn_1_124F4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;
    float positionScale;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 1) {

            data->scale = fn_1_11458(0.0f, data->accel.x, data->vel.x, data->vel.y);
            data->color.a = (u8)fn_1_11540(0.0f, data->accel.y, data->vel.x, data->vel.y);
            if ((data->vel.x += 1.0f) > data->vel.y) {
                data->scale = 0.0f;
                data->time = 0;
            }
        } else if (data->time == 2) {

            positionScale = fn_1_11458(0.0f, data->vel.z, data->vel.x, data->vel.y);
            data->scale = (float)(frandmod(6) + 1);
            data->pos.x = data->accel.x + data->effectDirection.x * positionScale;
            data->pos.y = data->accel.y + data->effectDirection.y * positionScale;
            data->pos.z = data->accel.z + data->effectDirection.z * positionScale;
            if ((data->vel.x += 1.0f) > data->vel.y) {
                data->scale = 0.0f;
                data->time = 0;
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

void fn_1_12984(void)
{
    lbl_1_bss_19EA = fn_8003216C(lbl_1_bss_19F0[1], 1000);
    fn_80026760(lbl_1_bss_19EA, 0.0f,
        0.0f, 0.0f);
    fn_800268B0(lbl_1_bss_19EA, 1.0f,
        1.0f, 1.0f);
    fn_80026B8C(lbl_1_bss_19EA, 6);
    fn_800326D0(lbl_1_bss_19EA, fn_1_124F4);
    fn_8003269C(lbl_1_bss_19EA,
        1);
}

void fn_1_12A68(void)
{
    fn_800262D0(lbl_1_bss_19EA);
}

void fn_1_12A94(float x, float y, float z)
{
    fn_80026760(lbl_1_bss_19E8, x, y, 100.0f + z);
}

void fn_1_12AE8(s16 time)
{
    HU3D_MODEL *model = &lbl_802C01C8[lbl_1_bss_19E8];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = time;
    }
}

void fn_1_12B60(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 i;

    if (particle->count == 0) {
        for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
            data->time = 0;
        }
    }
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        if (data->time == 0) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            if (rand8() % 5 == 0) {
                data->color.r = 0x30;
                data->color.g = 0x30;
                data->color.b = 0x30;
                data->color.a = (u8)(frandmod(24) + 0x30);
                data->scale = (float)(frandmod(100) + 600);
            }
        } else if (data->time == 1) {
            data->pos.x = 0.0f;
            data->pos.y = 0.0f;
            data->pos.z = 0.0f;
            if (rand8() % 5 == 0) {
                data->color.r = 0x40;
                data->color.g = 0x40;
                data->color.b = 0x40;
                data->color.a = (u8)(frandmod(24) + 0x5C);
                data->scale = (float)(frandmod(100) + 600);
            }
        }
    }
    DCFlushRangeNoSync(
        particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

void fn_1_12D98(void)
{
    lbl_1_bss_19E8 = fn_8003216C(lbl_1_bss_19F0[1], 4);
    fn_80026760(lbl_1_bss_19E8, 0.0f,
        375.0f, 100.0f);
    fn_800268B0(lbl_1_bss_19E8, 1.0f,
        1.0f, 1.0f);
    fn_80026B8C(lbl_1_bss_19E8, 6);
    fn_800326D0(lbl_1_bss_19E8, fn_1_12B60);
    fn_8003269C(lbl_1_bss_19E8, 1);
}

void fn_1_12E7C(void)
{
    fn_800262D0(lbl_1_bss_19E8);
}

void fn_1_12EA8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_19F0[i] = HuSprAnimRead(fn_80008DD0(
            lbl_1_data_A20[i], 0x10000000, 2));
    }

    lbl_1_bss_19EC = fn_8003216C(lbl_1_bss_19F0[0], 32);
    fn_80026760(lbl_1_bss_19EC, 0.0f,
        375.0f, -100.0f);
    fn_800268B0(lbl_1_bss_19EC, 1.0f,
        1.0f, 1.0f);
    fn_80026B8C(lbl_1_bss_19EC, 6);
    fn_800326D0(lbl_1_bss_19EC, fn_1_119C4);
    fn_8003269C(lbl_1_bss_19EC, 1);

    lbl_1_bss_19EA = fn_8003216C(lbl_1_bss_19F0[1], 1000);
    fn_80026760(lbl_1_bss_19EA, 0.0f,
        0.0f, 0.0f);
    fn_800268B0(lbl_1_bss_19EA, 1.0f,
        1.0f, 1.0f);
    fn_80026B8C(lbl_1_bss_19EA, 6);
    fn_800326D0(lbl_1_bss_19EA, fn_1_124F4);
    fn_8003269C(lbl_1_bss_19EA, 1);

    lbl_1_bss_19E8 = fn_8003216C(lbl_1_bss_19F0[1], 4);
    fn_80026760(lbl_1_bss_19E8, 0.0f,
        375.0f, 100.0f);
    fn_800268B0(lbl_1_bss_19E8, 1.0f,
        1.0f, 1.0f);
    fn_80026B8C(lbl_1_bss_19E8, 6);
    fn_800326D0(lbl_1_bss_19E8, fn_1_12B60);
    fn_8003269C(lbl_1_bss_19E8, 1);
}

void fn_1_1317C(void)
{
    fn_800262D0(lbl_1_bss_19EC);
    fn_800262D0(lbl_1_bss_19EA);
    fn_800262D0(lbl_1_bss_19E8);
}

void fn_1_131C8(HuVecF *pos)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    GXColor baseColor = {0x28, 0x28, 0x28, 0xFF};
    GXColor brightColor = {0xFF, 0xFF, 0xFF, 0xFF};
    float randomVelocity;
    float randomAccel;
    float randomDirection;
    s16 slot;
    s16 attempt;
    HU3D_MODEL *effectModel;
    HU3D_PARTICLE *effectParticle;
    HU3D_PARTICLE_DATA *effectData;
    s16 effectSlot;

    pos->z += 20.0f;
    randomAccel = (float)(frandmod(300) + 300);
    randomVelocity = (float)(frandmod(6) + 6);

    model = &lbl_802C01C8[lbl_1_bss_19EA];
    particle = model->hookData;
    for (slot = 0, data = particle->data;
         slot < particle->maxCnt; slot++, data++) {
        if (data->time == 0) {
            data->time = 1;
            break;
        }
    }
    if (slot != particle->maxCnt) {
        data->color.r = baseColor.r;
        data->color.g = baseColor.g;
        data->color.b = baseColor.b;
        data->color.a = 0;
        data->pos.x = pos->x;
        data->pos.y = pos->y;
        data->pos.z = pos->z;
        data->scale = 0.0f;
        data->vel.x = 0.0f;
        data->vel.y = randomVelocity;
        data->accel.x = randomAccel;
        data->accel.y = (float)baseColor.a;
    }

    for (attempt = 0; attempt < 30; attempt++) {
        randomDirection = (float)(frandmod(70) + 80);
        effectModel = &lbl_802C01C8[lbl_1_bss_19EA];
        effectParticle = effectModel->hookData;
        for (effectSlot = 0, effectData = effectParticle->data;
             effectSlot < effectParticle->maxCnt; effectSlot++, effectData++) {
            if (effectData->time == 0) {
                effectData->time = 2;
                break;
            }
        }
        if (effectSlot != effectParticle->maxCnt) {
            effectData->color.r = brightColor.r;
            effectData->color.g = brightColor.g;
            effectData->color.b = brightColor.b;
            effectData->color.a = brightColor.a;
            effectData->pos.x = pos->x;
            effectData->pos.y = pos->y;
            effectData->pos.z = pos->z;
            effectData->vel.x = 0.0f;
            effectData->vel.y = 10.0f;
            effectData->vel.z = randomDirection;
            effectData->accel.x = pos->x;
            effectData->accel.y = pos->y;
            effectData->accel.z = pos->z;
            effectData->effectDirection.x = (float)(frandmod(100) - 50);
            effectData->effectDirection.y = (float)(frandmod(100) - 50);
            effectData->effectDirection.z = 0.0f;
            PSVECNormalize(&effectData->effectDirection, &effectData->effectDirection);
        }
    }
}

void fn_1_13598(HuVecF *position)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    GXColor color = {0x40, 0x40, 0x40, 0x80};
    s16 attempt;
    s16 slot;

    position->z += 20.0f;
    for (attempt = 0; attempt < 10; ++attempt) {
        float randomAccel = (float)(frandmod(500) + 500);

        model = &lbl_802C01C8[lbl_1_bss_19EA];
        particle = model->hookData;
        slot = 0;
        data = particle->data;
        for (; slot < particle->maxCnt; ++slot, ++data) {
            if (data->time == 0) {
                data->time = 1;
                break;
            }
        }
        if (slot == particle->maxCnt) {
            continue;
        }

        data->color.r = color.r;
        data->color.g = color.g;
        data->color.b = color.b;
        data->color.a = 0;
        data->pos.x = position->x;
        data->pos.y = position->y;
        data->pos.z = position->z;
        data->scale = 0.0f;
        data->vel.x = 0.0f;
        data->vel.y = 60.0f;
        data->accel.x = randomAccel;
        data->accel.y = (float)color.a;
    }
}

void fn_1_13764(s16 time)
{
    HU3D_MODEL *model = &lbl_802C01C8[lbl_1_bss_19E8];
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle = model->hookData;
    s16 i;
    i = 0;
    data = particle->data;
    while (i < particle->maxCnt) {
        data->time = time;
        i++;
        data++;
    }
}
