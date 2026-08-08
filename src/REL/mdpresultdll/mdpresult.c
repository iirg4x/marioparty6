#include <string.h>

#include "datadir_enum.h"
#include "dolphin.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"

typedef struct MdResultCameraWork_s MDRESULT_CAMERA_WORK;
typedef void (*MDRESULT_CAMERA_CALLBACK)(OMOBJ *obj, MDRESULT_CAMERA_WORK *camera);

typedef struct MdResultMessageNumbers_s {
    s32 values[2];
} MDRESULT_MESSAGE_NUMBERS;

typedef struct MdResultFxNumbers_s {
    s32 values[16];
} MDRESULT_FX_NUMBERS;

typedef struct MdResultS16Table22_s {
    s16 values[11];
    s16 altValues[11];
} MDRESULT_S16_TABLE_22;

typedef struct MdResultU8Table12_s {
    u8 values[12];
} MDRESULT_U8_TABLE_12;

typedef struct MdResultMessageTable48_s {
    s32 values[48];
} MDRESULT_MESSAGE_TABLE_48;

typedef struct MdResultFloatTable11_s {
    float values[11];
} MDRESULT_FLOAT_TABLE_11;

typedef struct MdResultFloatTable8_s {
    float values[8];
} MDRESULT_FLOAT_TABLE_8;

typedef struct MdResultBss1278Work_s {
    s16 values[4];
    s32 messages[6];
} MDRESULT_BSS_1278_WORK;

typedef struct MdResultVectorPair_s {
    HuVecF values[2];
} MDRESULT_VECTOR_PAIR;

typedef struct MdResultVectorTable_s {
    HuVecF values[8];
} MDRESULT_VECTOR_TABLE;

typedef struct MdResultCharacterWork_s {
    s16 unk_00;
    s16 unk_02;
    s16 unk_04;
    s16 unk_06;
    s16 character;
    s16 unk_0A;
} MDRESULT_CHARACTER_WORK;

typedef struct MdResultSpriteInfo_s {
    s16 groupNo;
    s16 memberNo;
    s16 animNo;
    s16 priority;
    s16 bank;
    HuVec2f pos;
    HuVec2f scale;
    float zRot;
} MDRESULT_SPRITE_INFO;

typedef struct MdResultPlayerSpriteInfo_s {
    s16 animNo;
    s16 priority;
    s16 bank;
    HuVec2f pos;
    HuVec2f scale;
    float zRot;
} MDRESULT_PLAYER_SPRITE_INFO;

typedef struct MdResultPlayerSpriteTable_s {
    MDRESULT_PLAYER_SPRITE_INFO values[14];
} MDRESULT_PLAYER_SPRITE_TABLE;

typedef struct MdResultPlayerSpriteWork_s {
    HU3D_MODELID models[3];
    HUSPR_GROUPID group;
    HUSPRID sprites[14];
    u32 unk_24;
} MDRESULT_PLAYER_SPRITE_WORK;

typedef struct MdResultParticleWork_s {
    HuVecF position;
    HuVecF rotation;
    HuVecF scale;
    float phase;
    float verticalOffset;
    float speed;
    HuVecF target;
    float stateTime;
} MDRESULT_PARTICLE_WORK;

typedef struct MdResultEmitterWork_s {
    s16 active;
    float timer;
    float scale;
    void *data;
} MDRESULT_EMITTER_WORK;

typedef struct MdResultEmitterVertex_s {
    HuVecF position;
    float weight;
} MDRESULT_EMITTER_VERTEX;

typedef struct MdResultPlayerWork_s {
    HU3D_MODELID models[3];
    HUSPR_GROUPID group;
    float values[6];
    HUSPR_GROUPID secondGroup;
    s16 state[2];
    HUWINID winId;
} MDRESULT_PLAYER_WORK;

typedef struct MdResultTrailWork_s {
    HuVecF *points;
    HuVecF base;
    HuVecF velocity;
    s16 modelIndex;
    s16 state;
    s16 pointCount;
    s16 delay;
    GXColor color;
    s16 unk_28;
    s16 unk_2A;
} MDRESULT_TRAIL_WORK;

typedef struct MdResultParticlePreset_s {
    float values[12];
} MDRESULT_PARTICLE_PRESET;

typedef struct MdResultScoreWork_s {
    s16 playerIndex;
    s16 teamIndex;
    s16 rank;
    s16 star;
    s16 coin;
    s16 values[16];
} MDRESULT_SCORE_WORK;

typedef struct MdResultGroupWork_s {
    HUSPR_GROUPID group;
    HUSPRID sprites[3];
} MDRESULT_GROUP_WORK;

typedef struct MdResultStateWork_s {
    s16 state;
    float time;
    float delay;
    s16 score;
} MDRESULT_STATE_WORK;

typedef struct MdResultMoveWork_s {
    s16 state;
    float time;
    float duration;
    HuVecF current;
    HuVecF middle;
    HuVecF target;
    float values[4];
} MDRESULT_MOVE_WORK;

typedef struct MdResultModelEffectWork_s {
    s16 state;
    float time;
    float angle;
    float unk_0C;
    float unk_10;
    float unk_14;
    float unk_18;
    float unk_1C;
    float unk_20;
    float unk_24;
    float unk_28;
    float unk_2C;
    float unk_30;
    float unk_34;
    float unk_38;
    float unk_3C;
} MDRESULT_MODEL_EFFECT_WORK;

struct MdResultCameraWork_s {
    OMOBJ *obj;
    HuVecF center;
    HuVecF targetCenter;
    HuVecF rot;
    HuVecF targetRot;
    float zoom;
    float targetZoom;
    MDRESULT_CAMERA_CALLBACK callback;
    s16 unk_40;
    s16 mode;
    float unk_44;
};

float fn_1_1F8BC(float current, float target, float weight);
void fn_1_1FB50(HuVecF *current, const HuVecF *target, float weight);
void fn_1_1F868(HuVecF *vec, float x, float y, float z);
void fn_1_26164(s16 index, HuVecF *position);
float fn_1_1F878(float start, float end, float time, float duration);
float fn_1_1FD7C(float start, float end, float time, float duration);
void fn_1_1F948(HuVecF *result, const HuVecF *start,
    const HuVecF *middle, const HuVecF *end, float time);
float fn_1_1FF48(float start, float end, float time, float duration);
float fn_1_1FE74(float start, float end, float time, float duration);
void fn_1_26CF8(s16 index, HuVecF *position, float value);
float fn_1_1FC94(float start, float end, float time, float duration);
void fn_1_2001C(HU3D_MODELID modelId, const HuVecF *first,
    const HuVecF *second);
void fn_1_20108(HUSPR_GROUPID groupId, s32 attr);
void fn_1_20208(HUSPR_GROUPID groupId, s32 member, s16 value);
void fn_1_2035C(HUSPR_GROUPID groupId, s32 member, s16 value);
void fn_1_21714(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_217EC(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color, float accelY);
void fn_1_21904(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_25E6C(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_26070(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_2668C(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_3F20(OMOBJ *obj);
void fn_1_4124(void);
void fn_1_DC38(s16 index);
void fn_1_E658(s16 index);
void fn_1_D48C(OMOBJ *obj);
void fn_1_DED4(OMOBJ *obj);
void fn_1_11208(s16 index);
s16 fn_1_1109C(s16 index, u8 *mask);
s32 fn_1_12D7C(u8 mask);
s32 fn_1_15378(void);
void fn_1_1648C(void);
void fn_1_169A4(void);
void fn_1_9EBC(s16 count, u8 mask);
void fn_1_A624(s16 index);
void fn_1_9924(OMOBJ *obj);
void fn_1_A2B4(OMOBJ *obj);
void fn_1_3668(OMOBJ *obj);
void fn_1_3364(s16 index, s16 motion, float end, s32 attr);
void fn_1_37EC(void);
void fn_1_3894(OMOBJ *obj);
void fn_1_4A9C(OMOBJ *obj);
void fn_1_4BB8(OMOBJ *obj);
void fn_1_6290(OMOBJ *obj);
void fn_1_7590(OMOBJ *obj);
void fn_1_8184(OMOBJ *obj);
void fn_1_8B70(s32 value);
void fn_1_8F28(OMOBJ *obj);
void fn_1_A85C(OMOBJ *obj);
void fn_1_A984(void);
s32 fn_1_105CC(void);
s32 fn_1_10B34(void);
void fn_1_B510(OMOBJ *obj);
void fn_1_B8E8(OMOBJ *obj);
void fn_1_B220(void);
void fn_1_C358(void);
void fn_1_C23C(u8 mask);
void fn_1_C414(void);
s32 fn_1_C9A0(void);
void fn_1_CAEC(OMOBJ *obj);
void fn_1_BB60(OMOBJ *obj);
void fn_1_E9E8(void);
void fn_1_F548(void);
void fn_1_F0A4(OMOBJ *obj);
void fn_1_17F60(void);
void fn_1_17F78(OMOBJ *obj);
void fn_1_181C0(void);
void fn_1_192BC(OMOBJ *obj);
void fn_1_19504(void);
void fn_1_18F08(OMOBJ *obj);
void fn_1_1A570(OMOBJ *obj);
void fn_1_1B194(OMOBJ *obj);
void fn_1_1BAF4(void);
void fn_1_1C0C8(OMOBJ *obj);
void fn_1_1C9A0(void);
void fn_1_1C9B8(OMOBJ *obj);
void fn_1_1D318(void);
void fn_1_1D8EC(OMOBJ *obj);
void fn_1_1E19C(void);
void fn_1_1E47C(void);
void fn_1_20188(HUSPR_GROUPID groupId, s32 attr);
void fn_1_25D0C(float value);
void fn_1_25FF4(s16 index);
void fn_1_25B90(void);
void fn_1_26EAC(float value);
void fn_1_26F74(void);
void fn_1_1F3D4(void);
void fn_1_1F834(void);
void fn_1_1E5E8(HUSPRITE *sprite);
void fn_1_23EF0(HuVecF *position);
void fn_1_2104C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_20554(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_21AD0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_22348(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_24554(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_24C58(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_2C;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_14;
extern OMOBJ *lbl_1_bss_18;
extern OMOBJ *lbl_1_bss_1C;
extern OMOBJ *lbl_1_bss_20;
extern OMOBJ *lbl_1_bss_24;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_28;
extern OMOBJ *lbl_1_bss_30;
extern OMOBJ *lbl_1_bss_38;
extern MDRESULT_STATE_WORK lbl_1_bss_8AC[4];
extern MDRESULT_MOVE_WORK lbl_1_bss_8EC[7];
extern MDRESULT_MODEL_EFFECT_WORK lbl_1_bss_ADC[11];
extern OMOBJ *lbl_1_bss_3C;
extern s16 lbl_1_bss_48;
extern float lbl_1_bss_44;
extern char lbl_1_data_67D[];
extern char lbl_1_data_682[];
extern HuVecF lbl_1_bss_109C[4];
extern HUSPRID lbl_1_bss_117C[18];
extern HUSPR_GROUPID lbl_1_bss_11A0[6];
extern ANIMDATA *lbl_1_bss_11AC[39];
extern HUSPR_GROUPID lbl_1_bss_60;
extern ANIMDATA *lbl_1_bss_5C;
extern MDRESULT_PLAYER_WORK lbl_1_bss_66C[4];
extern MDRESULT_EMITTER_WORK lbl_1_bss_81C[9];
extern s32 lbl_1_bss_12A0[4];
extern HUSPR_GROUPID lbl_1_bss_3D2[];
extern HUSPR_GROUPID lbl_1_bss_714;
extern MDRESULT_SCORE_WORK lbl_1_bss_10D4[4];
extern MDRESULT_MOVE_WORK lbl_1_bss_71C[4];
extern s32 lbl_1_bss_12B0[3];
extern s32 lbl_1_bss_129C;
extern s32 lbl_1_bss_1298;
extern MDRESULT_BSS_1278_WORK lbl_1_bss_1278;
extern const MDRESULT_S16_TABLE_22 lbl_1_rodata_10;
extern HU3D_MODELID lbl_1_bss_1318;
extern HU3D_MODELID lbl_1_bss_131A;
extern HU3D_MODELID lbl_1_bss_14C2;
extern HU3D_MODELID lbl_1_bss_14C4;
extern HU3D_MODELID lbl_1_bss_14C6;
extern HU3D_MODELID lbl_1_bss_14B0[9];
extern HU3D_MODELID lbl_1_bss_1490[4][4];
extern MDRESULT_TRAIL_WORK lbl_1_bss_1320[8];
extern MDRESULT_MOVE_WORK lbl_1_bss_D9C[8];
extern HU3D_MODELID lbl_1_bss_1480[8];
extern ANIMDATA *lbl_1_bss_14C8[7];
extern MDRESULT_CAMERA_WORK lbl_1_bss_12BC;
extern HUWINID lbl_1_bss_1304[5];
extern HU3D_LIGHTID lbl_1_bss_130E[5];
extern MDRESULT_PARTICLE_WORK lbl_1_bss_F9C[4];
extern MDRESULT_CHARACTER_WORK lbl_1_bss_1248[4];
extern HuVecF lbl_1_data_0[16];
extern s32 lbl_1_data_C0[39];
extern char lbl_1_data_719[];
extern s16 lbl_1_data_15C[6];
extern MDRESULT_SPRITE_INFO lbl_1_data_168[18];
extern s32 lbl_1_data_620;
extern char lbl_1_data_624[];
extern char lbl_1_data_654[];
extern s16 lbl_1_data_646[3];
extern s32 lbl_1_data_64C[2];
extern char lbl_1_data_666[];
extern char lbl_1_data_678[];
extern char lbl_1_data_750[];
extern s16 lbl_1_data_684[4];

extern const MDRESULT_MESSAGE_NUMBERS lbl_1_rodata_3C;
extern const MDRESULT_FX_NUMBERS lbl_1_rodata_44;
extern const s32 lbl_1_rodata_568[4];
extern const float lbl_1_rodata_F4;
extern const float lbl_1_rodata_F8;
extern const float lbl_1_rodata_FC;
extern const float lbl_1_rodata_100;
extern const float lbl_1_rodata_104;
extern const float lbl_1_rodata_108;
extern const float lbl_1_rodata_10C;
extern const float lbl_1_rodata_110;
extern const float lbl_1_rodata_114;
extern const float lbl_1_rodata_118;
extern const float lbl_1_rodata_11C;
extern const float lbl_1_rodata_120;
extern const MDRESULT_VECTOR_PAIR lbl_1_rodata_124;
extern const MDRESULT_VECTOR_PAIR lbl_1_rodata_13C;
extern const MDRESULT_VECTOR_TABLE lbl_1_rodata_190;
extern const MDRESULT_VECTOR_TABLE lbl_1_rodata_1F0;
extern const GXColor lbl_1_rodata_154;
extern const float lbl_1_rodata_158;
extern const float lbl_1_rodata_15C;
extern const float lbl_1_rodata_160;
extern const float lbl_1_rodata_164;
extern const float lbl_1_rodata_168;
extern const HuVecF lbl_1_rodata_16C;
extern const HuVecF lbl_1_rodata_178;
extern const HuVecF lbl_1_rodata_184;
extern const float lbl_1_rodata_250;
extern const float lbl_1_rodata_254;
extern const float lbl_1_rodata_258;
extern const float lbl_1_rodata_25C;
extern const float lbl_1_rodata_260;
extern const double lbl_1_rodata_268;
extern const double lbl_1_rodata_270;
extern const double lbl_1_rodata_278;
extern const float lbl_1_rodata_280;
extern const float lbl_1_rodata_284;
extern const float lbl_1_rodata_288;
extern const double lbl_1_rodata_290;
extern const float lbl_1_rodata_298;
extern const float lbl_1_rodata_29C;
extern const float lbl_1_rodata_3B8;
extern const MDRESULT_VECTOR_PAIR lbl_1_rodata_3D0;
extern const float lbl_1_rodata_3E8;
extern const float lbl_1_rodata_3EC;
extern const float lbl_1_rodata_3F0;
extern const float lbl_1_rodata_3F4;
extern const float lbl_1_rodata_3F8;
extern const float lbl_1_rodata_3FC;
extern const float lbl_1_rodata_2C4;
extern const float lbl_1_rodata_2C8;
extern const float lbl_1_rodata_2CC;
extern const float lbl_1_rodata_2D0;
extern const float lbl_1_rodata_2D4;
extern const float lbl_1_rodata_2B8;
extern const u8 lbl_1_rodata_2A0[8];
extern const float lbl_1_rodata_2A8;
extern const float lbl_1_rodata_2AC;
extern const float lbl_1_rodata_2B0;
extern const float lbl_1_rodata_2B4;
extern const float lbl_1_rodata_2BC;
extern const float lbl_1_rodata_2C0;
extern const float lbl_1_rodata_378;
extern const u8 lbl_1_rodata_37C[4];
extern const double lbl_1_rodata_300;
extern const float lbl_1_rodata_31C;
extern const float lbl_1_rodata_384;
extern const float lbl_1_rodata_394;
extern const float lbl_1_rodata_398;
extern const float lbl_1_rodata_39C;
extern const float lbl_1_rodata_3A0;
extern const float lbl_1_rodata_3A4;
extern const float lbl_1_rodata_3A8;
extern const float lbl_1_rodata_3AC;
extern const float lbl_1_rodata_3B0;
extern const float lbl_1_rodata_3B4;
extern const u16 lbl_1_rodata_460[4];
extern const float lbl_1_rodata_468;
extern const MDRESULT_FLOAT_TABLE_11 lbl_1_rodata_474;
extern const MDRESULT_MESSAGE_TABLE_48 lbl_1_rodata_4A8;
extern const float lbl_1_rodata_390;
extern const float lbl_1_rodata_388;
extern const float lbl_1_rodata_38C;
extern const float lbl_1_rodata_4A0;
extern const MDRESULT_U8_TABLE_12 lbl_1_rodata_3BC;
extern const float lbl_1_rodata_3C8;
extern const float lbl_1_rodata_360;
extern const float lbl_1_rodata_374;
extern const float lbl_1_rodata_380;
extern const float lbl_1_rodata_3CC;
extern const float lbl_1_rodata_404;
extern const float lbl_1_rodata_408;
extern const float lbl_1_rodata_40C;
extern const float lbl_1_rodata_410;
extern const float lbl_1_rodata_414;
extern const double lbl_1_rodata_348;
extern const MDRESULT_FLOAT_TABLE_8 lbl_1_rodata_418;
extern const double lbl_1_rodata_438;
extern const float lbl_1_rodata_440;
extern const double lbl_1_rodata_448;
extern const double lbl_1_rodata_450;
extern const float lbl_1_rodata_458;
extern const float lbl_1_rodata_45C;
extern const float lbl_1_rodata_46C;
extern const float lbl_1_rodata_470;
extern const float lbl_1_rodata_30C;
extern const float lbl_1_rodata_4A4;
extern const float lbl_1_rodata_594;
extern const float lbl_1_rodata_598;
extern const float lbl_1_rodata_59C;
extern const float lbl_1_rodata_E5C;
extern const float lbl_1_rodata_E70;
extern const float lbl_1_rodata_E74;
extern const float lbl_1_rodata_E78;
extern const double lbl_1_rodata_E80;
extern const float lbl_1_rodata_E88;
extern const double lbl_1_rodata_E90;
extern const double lbl_1_rodata_E98;
extern const float lbl_1_rodata_EA0;
extern const float lbl_1_rodata_EA4;
extern const float lbl_1_rodata_EF8;
extern const double lbl_1_rodata_EE8;
extern const double lbl_1_rodata_EF0;
extern const HuVecF lbl_1_rodata_EA8;
extern const MDRESULT_PARTICLE_PRESET lbl_1_rodata_EFC;
extern const MDRESULT_PLAYER_SPRITE_TABLE lbl_1_rodata_6EC;
extern const float lbl_1_rodata_F2C;
extern const float lbl_1_rodata_F80;
extern const float lbl_1_rodata_F64;
extern const float lbl_1_rodata_FAC;
extern const u8 lbl_1_rodata_FC8[15];
extern const u8 lbl_1_rodata_FD7[9];
extern const float lbl_1_rodata_E68;
extern const float lbl_1_rodata_E6C;

void fn_1_120(HUWINID winId, u32 mess, s16 index)
{
    MDRESULT_MESSAGE_NUMBERS messNum = lbl_1_rodata_3C;
    MDRESULT_FX_NUMBERS fxNum = lbl_1_rodata_44;
    s16 i;

    index--;
    OSReport(lbl_1_data_624, index);
    if (lbl_1_data_620 != mess) {
        lbl_1_data_620 = mess;
        for (i = 0;; i++) {
            if (messNum.values[i] == -1) {
                HuAudFXPlay(fxNum.values[index]);
                break;
            }
            if (mess == messNum.values[i]) {
                if (index >= 8) {
                    HuAudFXPlayPan(fxNum.values[index], 80);
                } else {
                    HuAudFXPlayPan(fxNum.values[index], 48);
                }
                break;
            }
        }
    }
}

void fn_1_1C70(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_1304[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_1304[winNo]);
    }
}

inline void fn_1_1C70(s16 winNo);

void fn_1_1CE0(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_1304[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_1304[winNo]);
    }
}

inline void fn_1_1CE0(s16 winNo);

void fn_1_1D50(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_1304[winNo]);
}

inline void fn_1_1D50(s16 winNo);

s16 fn_1_1D8C(s16 winNo, s16 mode)
{
    if (mode != 0) {
        HuWinAttrSet(lbl_1_bss_1304[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_1304[winNo], HUWIN_ATTR_NOCANCEL);
    }
    return HuWinChoiceGet(lbl_1_bss_1304[winNo], -1);
}

inline s16 fn_1_1D8C(s16 winNo, s16 mode);

void fn_1_1E28(s16 winNo, s32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_1304[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_1304[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_1304[winNo], speed);
    if (lbl_1_data_620 != messNum) {
        lbl_1_data_620 = -1;
    }
}

inline void fn_1_1E28(s16 winNo, s32 messNum, s16 speed);

void fn_1_1EE4(s16 winNo, s32 messNum, s16 insertPos)
{
    HuWinHomeClear(lbl_1_bss_1304[winNo]);
    HuWinInsertMesSet(lbl_1_bss_1304[winNo], messNum, insertPos);
}

inline void fn_1_1EE4(s16 winNo, s32 messNum, s16 insertPos);

void fn_1_1F54(void)
{
    s16 i;

    HuWinInit(1);
    lbl_1_bss_1304[0] =
        HuWinExCreateFrame(lbl_1_rodata_158, lbl_1_rodata_15C,
            544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_1304[0]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[0], lbl_1_rodata_104);
    HuWinPriSet(lbl_1_bss_1304[0], 0);
    lbl_1_bss_1304[1] =
        HuWinExCreateFrame(lbl_1_rodata_158, lbl_1_rodata_160,
            544, 68, -1, 0);
    HuWinDispOff(lbl_1_bss_1304[1]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[1], lbl_1_rodata_164);
    HuWinPriSet(lbl_1_bss_1304[1], 0);
    lbl_1_bss_1304[2] =
        HuWinExCreateFrame(lbl_1_rodata_158, lbl_1_rodata_160,
            544, 68, -1, 3);
    HuWinDispOff(lbl_1_bss_1304[2]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[2], lbl_1_rodata_164);
    lbl_1_bss_1304[3] =
        HuWinExCreateFrame(lbl_1_rodata_158, lbl_1_rodata_160,
            544, 68, -1, 4);
    HuWinDispOff(lbl_1_bss_1304[3]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[3], lbl_1_rodata_164);
    lbl_1_bss_1304[4] =
        HuWinExCreateFrame(lbl_1_rodata_158, lbl_1_rodata_160,
            544, 68, -1, 5);
    HuWinDispOff(lbl_1_bss_1304[4]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[4], lbl_1_rodata_164);

    for (i = 0; i < 5; i++) {
        winData[lbl_1_bss_1304[i]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_1304[i], (HUWIN_CALLBACK)fn_1_120);
    }
}

inline void fn_1_1F54(void);

void fn_1_2208(void)
{
    s16 i;

    for (i = 0; i < 5; i++) {
        HuWinExKill(lbl_1_bss_1304[i]);
    }
    HuWinAllKill();
}

void fn_1_2264(s16 winNo)
{
    if (lbl_1_data_646[0] != -1 && lbl_1_data_646[0] != winNo) {
        fn_1_1CE0(lbl_1_data_646[0]);
    }
    if (lbl_1_data_646[0] == -1 || lbl_1_data_646[0] != winNo) {
        lbl_1_data_646[0] = winNo;
        lbl_1_data_64C[0] = -1;
        fn_1_1C70(lbl_1_data_646[0]);
    }
}

inline void fn_1_2264(s16 winNo);

void fn_1_23C0(void)
{
    if (lbl_1_data_646[0] != -1) {
        fn_1_1CE0(lbl_1_data_646[0]);
    }
    lbl_1_data_646[0] = -1;
    lbl_1_data_64C[0] = -1;
}

void fn_1_246C(void)
{
    if (lbl_1_data_646[0] != -1) {
        fn_1_1D50(lbl_1_data_646[0]);
    }
}

s16 fn_1_24CC(s16 mode)
{
    if (lbl_1_data_646[0] != -1) {
        return fn_1_1D8C(lbl_1_data_646[0], mode);
    }
    return 0;
}

void fn_1_258C(s16 winNo, s32 messNum, s16 speed)
{
    fn_1_2264(winNo);
    if (lbl_1_data_64C[0] != messNum) {
        lbl_1_data_64C[0] = messNum;
        fn_1_1E28(lbl_1_data_646[0], lbl_1_data_64C[0], speed);
    }
}

void fn_1_27A4(s16 winNo, s32 messNum, s16 insertPos)
{
    fn_1_2264(winNo);
    fn_1_1EE4(lbl_1_data_646[0], messNum, insertPos);
}

inline void fn_1_27A4(s16 winNo, s32 messNum, s16 insertPos);

void fn_1_295C(s32 messNum, s16 positionF)
{
    if (lbl_1_data_646[1] == -1) {
        lbl_1_data_646[1] = 0;
        lbl_1_data_64C[1] = -1;
        fn_1_1C70(lbl_1_data_646[1]);
    }
    if (positionF == 0) {
        HuWinPosSet(lbl_1_bss_1304[0], lbl_1_rodata_158,
            lbl_1_rodata_15C);
    } else {
        HuWinPosSet(lbl_1_bss_1304[0], lbl_1_rodata_158,
            lbl_1_rodata_168);
    }
    if (lbl_1_data_64C[1] != messNum) {
        lbl_1_data_64C[1] = messNum;
        fn_1_1E28(lbl_1_data_646[1], lbl_1_data_64C[1], 0);
    }
}

inline void fn_1_295C(s32 messNum, s16 positionF);

void fn_1_2B44(void)
{
    if (lbl_1_data_646[1] != -1) {
        fn_1_1CE0(lbl_1_data_646[1]);
    }
    lbl_1_data_646[1] = -1;
    lbl_1_data_64C[1] = -1;
}

inline void fn_1_2B44(void);

void fn_1_3364(s16 index, s16 motion, float end, s32 attr)
{
    OMOBJ *obj = lbl_1_bss_C;

    if (motion == 7) {
        CharMotionVoiceOnSet(lbl_1_bss_1248[index].character, 41, 0);
        if (lbl_1_bss_1278.values[3] == 0) {
            s16 character = lbl_1_bss_1248[index].character;
            MDRESULT_S16_TABLE_22 sounds = lbl_1_rodata_10;

            if (character >= 0 && character <= 11) {
                HuAudFXPlay(sounds.values[character]);
            }
        } else {
            s16 character = lbl_1_bss_1248[index].character;
            MDRESULT_S16_TABLE_22 sounds = lbl_1_rodata_10;

            if (character >= 0 && character <= 11) {
                HuAudFXPlay(sounds.altValues[character]);
            }
        }
    }
    Hu3DMotionShiftSet(obj->mdlId[index], obj->mtnId[index + motion * 4],
        lbl_1_rodata_104, end, (u32)attr);
    if (motion == 8) {
        Hu3DMotionShiftStartEndSet(obj->mdlId[index], lbl_1_rodata_F4,
            lbl_1_rodata_25C);
    }
}

void fn_1_3668(OMOBJ *obj)
{
    OMOBJ *current;
    s16 i;

    for (i = 0; i < 4; i++) {
        if (obj->work[i] == 0 && Hu3DMotionEndCheck(obj->mdlId[i]) != 0) {
            current = lbl_1_bss_C;
            Hu3DMotionShiftSet(current->mdlId[i], current->mtnId[i],
                lbl_1_rodata_104, lbl_1_rodata_260, HU3D_MOTATTR_LOOP);
            obj->work[i] = 1;
        }
    }
    for (i = 0; i < 4 && obj->work[i] != 0; i++) {
    }
    if (i == 4) {
        obj->objFunc = NULL;
    }
}

void fn_1_378C(void)
{
    s16 i;
    OMOBJ *obj = lbl_1_bss_C;

    for (i = 0; i < 4; i++) {
        obj->work[i] = 0;
    }
    obj->objFunc = fn_1_3668;
}

void fn_1_37EC(void)
{
    OMOBJ *obj = lbl_1_bss_C;
    OMOBJ *current;
    s16 i;

    for (i = 0; i < 4; i++) {
        current = lbl_1_bss_C;

        Hu3DMotionShiftSet(current->mdlId[i], current->mtnId[i],
            lbl_1_rodata_104, lbl_1_rodata_260, HU3D_MOTATTR_LOOP);
    }
    obj->objFunc = NULL;
}

void fn_1_3894(OMOBJ *obj)
{
    MDRESULT_MOVE_WORK *move;
    MDRESULT_PARTICLE_WORK *particle;
    HuVecF position;
    float time;
    double angle;
    s16 i;

    if (obj->work[0] == 0) {
        for (i = 0; i < 4; i++) {
            move = &lbl_1_bss_D9C[i];
            time = fn_1_1F878(lbl_1_rodata_104, lbl_1_rodata_110,
                move->time, move->duration);
            fn_1_1F948(&position, &move->current, &move->middle,
                &move->target, time);
            fn_1_26CF8(i, &position, lbl_1_rodata_104);
            move->time += lbl_1_rodata_110;
            if (move->time > move->duration) {
                obj->work[0] = 1;
            }

            move = &lbl_1_bss_D9C[i + 4];
            time = fn_1_1F878(lbl_1_rodata_104, lbl_1_rodata_110,
                move->time, move->duration);
            fn_1_1F948(&position, &move->current, &move->middle,
                &move->target, time);
            fn_1_26CF8((s16)(i + 4), &position, lbl_1_rodata_104);
            move->time += lbl_1_rodata_110;
            if (move->time > move->duration) {
                obj->work[0] = 1;
            }
        }
        return;
    }

    if (obj->work[0] == 1) {
        for (i = 0; i < 4; i++) {
            particle = &lbl_1_bss_F9C[i];
            move = &lbl_1_bss_D9C[i];
            particle->rotation.x = move->target.x;
            particle->rotation.y = move->target.y;
            particle->rotation.z = move->target.z;
            move = &lbl_1_bss_D9C[i + 4];
            particle->scale.x = move->target.x;
            particle->scale.y = move->target.y;
            particle->scale.z = move->target.z;
        }
        obj->work[0] = 2;
        return;
    }

    for (i = 0; i < 4; i++) {
        particle = &lbl_1_bss_F9C[i];
        Hu3DModelPosGet(obj->mdlId[i], &position);

        angle = (lbl_1_rodata_270 * (double)particle->target.x) /
            lbl_1_rodata_278;
        position.x += (float)(lbl_1_rodata_268 * sin(angle));
        position.y += lbl_1_rodata_280 + fn_1_1FF48(lbl_1_rodata_104,
            lbl_1_rodata_280, particle->target.z, lbl_1_rodata_284);
        position.z += (float)(lbl_1_rodata_268 * cos(angle));
        particle->rotation.x = fn_1_1F8BC(particle->rotation.x,
            position.x, particle->phase);
        particle->rotation.y = fn_1_1F8BC(particle->rotation.y,
            position.y, particle->phase);
        particle->rotation.z = fn_1_1F8BC(particle->rotation.z,
            position.z, particle->phase);
        fn_1_26CF8(i, &particle->rotation, particle->verticalOffset);

        Hu3DModelPosGet(obj->mdlId[i], &position);
        angle = (lbl_1_rodata_270 * (double)particle->target.y) /
            lbl_1_rodata_278;
        position.x += (float)(lbl_1_rodata_290 * sin(angle));
        position.y += lbl_1_rodata_280 + fn_1_1FF48(lbl_1_rodata_104,
            lbl_1_rodata_280, particle->target.z, lbl_1_rodata_284);
        position.z += (float)(lbl_1_rodata_290 * cos(angle));
        particle->scale.x = fn_1_1F8BC(particle->scale.x,
            position.x, particle->phase);
        particle->scale.y = fn_1_1F8BC(particle->scale.y,
            position.y, particle->phase);
        particle->scale.z = fn_1_1F8BC(particle->scale.z,
            position.z, particle->phase);
        fn_1_26CF8((s16)(i + 4), &particle->scale,
            particle->verticalOffset);

        particle->target.x += lbl_1_rodata_F8;
        if (particle->target.x > lbl_1_rodata_288) {
            particle->target.x -= lbl_1_rodata_288;
        }
        particle->target.y += lbl_1_rodata_F8;
        if (particle->target.y > lbl_1_rodata_288) {
            particle->target.y -= lbl_1_rodata_288;
        }
        particle->target.z += lbl_1_rodata_110;
        if (particle->target.z > lbl_1_rodata_284) {
            particle->target.z -= lbl_1_rodata_284;
        }
        particle->phase -= lbl_1_rodata_110;
        if (particle->phase < lbl_1_rodata_298) {
            particle->phase = lbl_1_rodata_298;
        }
    }
}

void fn_1_3E98(void)
{
    MDRESULT_PARTICLE_WORK *work;
    s16 i;

    for (i = 0; i < 4; i++) {
        work = &lbl_1_bss_F9C[i];
        work->verticalOffset -= lbl_1_rodata_110;
        if (work->verticalOffset < lbl_1_rodata_29C) {
            work->verticalOffset = lbl_1_rodata_29C;
        }
    }
}

void fn_1_3F20(OMOBJ *obj)
{
    MDRESULT_MOVE_WORK *move;
    Mtx matrix;
    HuVecF position;
    s16 i;

    for (i = 0; i < 4; i++) {
        move = &lbl_1_bss_D9C[i];
        Hu3DModelObjMtxGet(lbl_1_bss_4->mdlId[0], lbl_1_data_654,
            matrix);
        position.x = matrix[0][3];
        position.y = matrix[1][3];
        position.z = matrix[2][3];
        fn_1_26CF8(i, &position, lbl_1_rodata_104);

        move = &lbl_1_bss_D9C[i + 4];
        Hu3DModelObjMtxGet(lbl_1_bss_8->mdlId[0], lbl_1_data_666,
            matrix);
        position.x = matrix[0][3];
        position.y = matrix[1][3];
        position.z = matrix[2][3];
        fn_1_26CF8((s16)(i + 4), &position, lbl_1_rodata_104);
    }

    obj->work[1] += 1;
    if (obj->work[1] <= 30) {
        return;
    }

    for (i = 0; i < 4; i++) {
        move = &lbl_1_bss_D9C[i];
        Hu3DModelObjMtxGet(lbl_1_bss_4->mdlId[0], lbl_1_data_654,
            matrix);
        move->current.x = matrix[0][3];
        move->current.y = matrix[1][3];
        move->current.z = matrix[2][3];

        move = &lbl_1_bss_D9C[i + 4];
        Hu3DModelObjMtxGet(lbl_1_bss_8->mdlId[0], lbl_1_data_666,
            matrix);
        move->current.x = matrix[0][3];
        move->current.y = matrix[1][3];
        move->current.z = matrix[2][3];
    }
    obj->objFunc = fn_1_3894;
}

void fn_1_4124(void)
{
    OMOBJ *obj = lbl_1_bss_C;
    MDRESULT_PARTICLE_WORK *particleWork;
    MDRESULT_MOVE_WORK *moveWork;
    Mtx matrix;
    HuVecF hookPos;
    u8 colors[8];
    s16 i;

    colors[0] = lbl_1_rodata_2A0[0];
    colors[1] = lbl_1_rodata_2A0[1];
    colors[2] = lbl_1_rodata_2A0[2];
    colors[3] = lbl_1_rodata_2A0[3];
    colors[4] = lbl_1_rodata_2A0[4];
    colors[5] = lbl_1_rodata_2A0[5];
    colors[6] = lbl_1_rodata_2A0[6];
    colors[7] = lbl_1_rodata_2A0[7];

    obj->work[0] = 0;
    obj->work[1] = 0;
    for (i = 0; i < 4; i++) {
        particleWork = &lbl_1_bss_F9C[i];
        particleWork->phase = lbl_1_rodata_2B0;
        particleWork->verticalOffset = lbl_1_rodata_104;
        fn_1_1F868(&particleWork->target, lbl_1_rodata_104,
            lbl_1_rodata_2A8, lbl_1_rodata_2AC);

        Hu3DModelObjMtxGet(lbl_1_bss_4->mdlId[0], lbl_1_data_654,
            matrix);
        hookPos.x = matrix[0][3];
        hookPos.y = matrix[1][3];
        hookPos.z = matrix[2][3];
        Hu3DModelPosGet(lbl_1_bss_4->mdlId[0], &particleWork->rotation);
        fn_1_2668C(i, 50, &hookPos, lbl_1_rodata_2B4, &colors[0]);

        Hu3DModelPosGet(lbl_1_bss_8->mdlId[0], &particleWork->scale);
        Hu3DModelObjMtxGet(lbl_1_bss_8->mdlId[0], lbl_1_data_666,
            matrix);
        hookPos.x = matrix[0][3];
        hookPos.y = matrix[1][3];
        hookPos.z = matrix[2][3];
        fn_1_2668C((s16)(i + 4), 50, &hookPos, lbl_1_rodata_2B4,
            &colors[4]);
    }

    for (i = 0; i < 4; i++) {
        moveWork = &lbl_1_bss_D9C[i];
        Hu3DModelPosGet(lbl_1_bss_4->mdlId[0], &moveWork->current);
        Hu3DModelPosGet(lbl_1_bss_C->mdlId[i], &moveWork->middle);
        Hu3DModelPosGet(lbl_1_bss_C->mdlId[i], &moveWork->target);
        moveWork->current.x -= lbl_1_rodata_2B8;
        moveWork->current.y = lbl_1_rodata_284;
        fn_1_1F868(&moveWork->middle, lbl_1_rodata_104,
            lbl_1_rodata_2BC, lbl_1_rodata_2C0);
        moveWork->target.y = lbl_1_rodata_284;
        moveWork->time = lbl_1_rodata_104;
        moveWork->duration = lbl_1_rodata_F4;

        moveWork = &lbl_1_bss_D9C[i + 4];
        Hu3DModelPosGet(lbl_1_bss_8->mdlId[0], &moveWork->current);
        Hu3DModelPosGet(lbl_1_bss_C->mdlId[i], &moveWork->middle);
        Hu3DModelPosGet(lbl_1_bss_C->mdlId[i], &moveWork->target);
        moveWork->current.x -= lbl_1_rodata_2B8;
        moveWork->current.y = lbl_1_rodata_284;
        fn_1_1F868(&moveWork->middle, lbl_1_rodata_104,
            lbl_1_rodata_2BC, lbl_1_rodata_2C0);
        moveWork->target.y = lbl_1_rodata_284;
        moveWork->time = lbl_1_rodata_104;
        moveWork->duration = lbl_1_rodata_F4;
    }

    for (i = 0; i < 4; i++) {
        Hu3DModelLayerSet(lbl_1_bss_C->mdlId[i], 2);
    }
    Hu3DModelLayerSet(lbl_1_bss_4->mdlId[0], 2);
    Hu3DModelLayerSet(lbl_1_bss_8->mdlId[0], 2);
    Hu3DMotionShiftSet(lbl_1_bss_4->mdlId[0], lbl_1_bss_4->mtnId[0],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[0], lbl_1_bss_8->mtnId[0],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    obj->objFunc = fn_1_3F20;
}

void fn_1_5360(OMOBJ *obj)
{
    HuVecF positions[3] = {
        { -16.0f, -115.0f, -724.0f },
        { -490.0f, 84.0f, -514.0f },
        { 452.0f, 176.0f, -514.0f }
    };
    s16 i;
    MDRESULT_MODEL_EFFECT_WORK *work;

    for (i = 0; i < 3; i++) {
        work = &lbl_1_bss_ADC[i];
        Hu3DModelPosSetV(obj->mdlId[i], &positions[i]);
        work->state = 0;
        work->time = lbl_1_rodata_104;
        work->angle = frandmod(180) + 240;
    }
}

void fn_1_5A60(float value)
{
    lbl_1_bss_44 = value;
}

void fn_1_6C7C(OMOBJ *obj)
{
    if (obj->work[0] == 0) {
        fn_1_6290(obj);
    }
}

void fn_1_7560(s16 state, u8 flag)
{
    OMOBJ *obj = lbl_1_bss_28;

    obj->work[0] = state;
    obj->work[1] = flag;
}

void fn_1_95A4(void)
{
    OMOBJ *obj = lbl_1_bss_18;

    obj->work[0] = 0;
    obj->work[1] = 10;
    obj->work[2] = 0;
    obj->objFunc = fn_1_8F28;
}

void fn_1_9850(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 8; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_98E0(void)
{
    OMOBJ *obj = lbl_1_bss_1C;

    do {
        HuPrcVSleep();
    } while (obj->objFunc != NULL);
}

void fn_1_C358(void)
{
    s16 i;
    OMOBJ *obj = lbl_1_bss_20;
    OMOBJ *second;
    s16 j;

    for (i = 0; i < 13; i++) {
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    second = lbl_1_bss_24;
    for (j = 0; j < 22; j++) {
        Hu3DModelAttrSet(second->mdlId[j], HU3D_ATTR_DISPOFF);
    }
    obj->objFunc = NULL;
}

void fn_1_C23C(u8 mask)
{
    MDRESULT_STATE_WORK *work;
    OMOBJ *obj = lbl_1_bss_20;
    s16 i = 0;

    work = lbl_1_bss_8AC;
    for (; i < 4; i++, work++) {
        if (mask & (1 << i)) {
            work->state = 1;
            work->time = lbl_1_rodata_104;
            work->delay = lbl_1_rodata_F8;
            Hu3DMotionSpeedSet(obj->mdlId[i], lbl_1_rodata_110);
            Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
                lbl_1_rodata_104, lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        }
    }
    obj->objFunc = fn_1_BB60;
}

void fn_1_8184(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, 256);
    for (i = 0; i < 4; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 14), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_104, lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    if (lbl_1_bss_1278.values[3] == 0) {
        for (i = 0; i < 4; i++) {
            Hu3DModelPosSetV(obj->mdlId[i], &lbl_1_data_0[i]);
            Hu3DModelScaleSet(obj->mdlId[i], lbl_1_rodata_110,
                lbl_1_rodata_110, lbl_1_rodata_110);
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        }
    } else {
        for (i = 0; i < 2; i++) {
            Hu3DModelPosSetV(obj->mdlId[i], &lbl_1_data_0[i + 4]);
            Hu3DModelScaleSet(obj->mdlId[i], lbl_1_rodata_298,
                lbl_1_rodata_110, lbl_1_rodata_298);
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        }
    }
    obj->objFunc = fn_1_7590;
}

void fn_1_F0A4(OMOBJ *obj)
{
    if (!WipeCheck()) {
        fn_1_E9E8();
        omOvlReturnEx(1, 1);
    }
}

void fn_1_F0E0(OMOBJ *obj)
{
    if (omSysExitReq != 0) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        obj->objFunc = fn_1_F0A4;
    }
}

void fn_1_F138(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        HuWinDispOff(lbl_1_bss_1304[i]);
    }
    HuSprPriSet(lbl_1_bss_11A0[5], 0, 5500);
    fn_1_20188(lbl_1_bss_11A0[5], 4);
    HuPrcSleep(5);
}

void fn_1_17D94(void)
{
    HUSPR_GROUPID *group = &lbl_1_bss_714;

    fn_1_20108(group[0], HUSPR_ATTR_DISPOFF);
}

void fn_1_17DCC(OMOBJ *obj)
{
    MDRESULT_GROUP_WORK *group = (MDRESULT_GROUP_WORK *)&lbl_1_bss_714;
    MDRESULT_GROUP_WORK *finalGroup;
    s16 i;

    group->group = HuSprGrpCreate(3);
    group->sprites[0] = HuSprCreate(lbl_1_bss_11AC[10], 0, 0);
    group->sprites[1] = HuSprCreate(lbl_1_bss_11AC[11], 0, 0);
    group->sprites[2] = HuSprCreate(lbl_1_bss_11AC[12], 0, 0);
    for (i = 0; i < 3; i++) {
        HuSprGrpMemberSet(group->group, i, group->sprites[i]);
    }
    HuSprPosSet(group->group, 0, lbl_1_rodata_594, lbl_1_rodata_2B8);
    HuSprPosSet(group->group, 1, lbl_1_rodata_598, lbl_1_rodata_2B8);
    HuSprPosSet(group->group, 2, lbl_1_rodata_59C, lbl_1_rodata_2B8);
    HuSprDrawNoSet(group->group, 0, 64);
    HuSprDrawNoSet(group->group, 1, 64);
    HuSprDrawNoSet(group->group, 2, 64);
    finalGroup = (MDRESULT_GROUP_WORK *)&lbl_1_bss_714;
    fn_1_20108(finalGroup->group, HUSPR_ATTR_DISPOFF);
    obj->objFunc = NULL;
}

void fn_1_17F60(void)
{
    HUSPR_GROUPID *group = &lbl_1_bss_714;
}

void fn_1_9924(OMOBJ *obj)
{
    MDRESULT_MOVE_WORK *work;
    MDRESULT_MOVE_WORK *secondary;
    HuVecF position;
    HuVecF rotation;
    float time;
    float scale;
    u8 color[4];
    s16 i;

    work = &lbl_1_bss_8EC[obj->work[3]];
    if (work->state == 0) {
        time = fn_1_1F878(lbl_1_rodata_104, lbl_1_rodata_110,
            work->time, work->duration);
        fn_1_1F948(&position, &work->current, &work->middle,
            &work->target, time);
        Hu3DModelPosSetV(obj->mdlId[obj->work[3]], &position);
        time = fn_1_1F878(work->values[0], lbl_1_rodata_378,
            work->time, work->duration);
        Hu3DModelRotSet(obj->mdlId[obj->work[3]], lbl_1_rodata_104,
            time, lbl_1_rodata_104);
        work->time += lbl_1_rodata_110;
        if (work->time > work->duration) {
            work->state = 1;
            work->time = lbl_1_rodata_104;
            work->duration = lbl_1_rodata_380;
            if (obj->work[2] == 0) {
                obj->objFunc = NULL;
            }
        }
    } else if (work->state == 1) {
        work->time += lbl_1_rodata_110;
        if (work->time > work->duration) {
            work->state = 2;
            work->time = lbl_1_rodata_104;
            work->duration = lbl_1_rodata_384;
        }
        secondary = &lbl_1_bss_8EC[3];
        for (i = 0; i < 4; i++, secondary++) {
            if (secondary->state != 0) {
                Hu3DModelPosGet(obj->mdlId[obj->work[3]], &position);
                Hu3DModelPosSetV(obj->mdlId[i + 3], &position);
                Hu3DModelScaleSet(obj->mdlId[i + 3], lbl_1_rodata_258,
                    lbl_1_rodata_258, lbl_1_rodata_258);
                Hu3DModelAttrReset(obj->mdlId[i + 3], 1);
                color[0] = lbl_1_rodata_37C[0];
                color[1] = lbl_1_rodata_37C[1];
                color[2] = lbl_1_rodata_37C[2];
                color[3] = lbl_1_rodata_37C[3];
                Hu3DModelPosGet(obj->mdlId[i + 3], &position);
                fn_1_25E6C(i, 1, &position, lbl_1_rodata_2B8, color);
                fn_1_1F868(&secondary->current, position.x, position.y,
                    position.z - lbl_1_rodata_388);
            }
        }
    } else if (work->state == 2) {
        secondary = &lbl_1_bss_8EC[3];
        for (i = 0; i < 4; i++, secondary++) {
            if (secondary->state != 0) {
                time = fn_1_1FC94(lbl_1_rodata_104, lbl_1_rodata_110,
                    secondary->time, secondary->duration - lbl_1_rodata_F4);
                fn_1_1F948(&position, &secondary->current,
                    &secondary->middle, &secondary->target, time);
                Hu3DModelPosSetV(obj->mdlId[i + 3], &position);
                Hu3DModelRotGet(obj->mdlId[i + 3], &rotation);
                rotation.y = fn_1_1F878(lbl_1_rodata_104,
                    lbl_1_rodata_38C, secondary->time,
                    secondary->duration - lbl_1_rodata_390);
                Hu3DModelRotSetV(obj->mdlId[i + 3], &rotation);
                Hu3DModelPosGet(obj->mdlId[i + 3], &position);
                fn_1_26070(i, -1, &position, lbl_1_rodata_360, NULL);

                if (secondary->time >
                    secondary->duration - lbl_1_rodata_2B4) {
                    scale = fn_1_1FC94(lbl_1_rodata_258,
                        lbl_1_rodata_104,
                        secondary->time -
                            (secondary->duration - lbl_1_rodata_2B4),
                        lbl_1_rodata_2B4);
                    position.x = scale;
                    position.y = scale;
                    position.z = scale;
                    Hu3DModelScaleSetV(obj->mdlId[i + 3], &position);
                }
                secondary->time += lbl_1_rodata_110;
                if (secondary->time > secondary->duration) {
                    Hu3DModelAttrSet(obj->mdlId[i + 3], 1);
                    fn_1_25FF4(i);
                }
            }
        }
        work->time += lbl_1_rodata_110;
        if (work->time > work->duration) {
            obj->objFunc = NULL;
        }
    }
    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &position);
    fn_1_26070((s16)(obj->work[3] + 4), -1, &position,
        lbl_1_rodata_360, NULL);
}

void fn_1_9EBC(s16 count, u8 mask)
{
    OMOBJ *obj = lbl_1_bss_1C;
    MDRESULT_MOVE_WORK *work = &lbl_1_bss_8EC[obj->work[3]];
    HuVecF rotation;
    s16 slotCount = 4;
    float total = lbl_1_rodata_104;
    s16 i;

    if (lbl_1_bss_1278.values[3] != 0) {
        slotCount = 2;
    }
    work->state = 0;
    work->time = lbl_1_rodata_104;
    work->duration = lbl_1_rodata_380;
    for (i = 0; i < slotCount; i++) {
        if (mask & (1 << i)) {
            if (lbl_1_bss_1278.values[3] == 0) {
                total += lbl_1_data_0[i].x;
            } else {
                total += lbl_1_data_0[i + 4].x;
            }
        }
    }
    if (count != 0) {
        total /= count;
    }
    if (count != 0 && count != slotCount) {
        obj->work[2] = 1;
    } else {
        obj->work[2] = 0;
    }
    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &work->current);
    fn_1_1F868(&work->middle, total, lbl_1_rodata_394,
        lbl_1_rodata_398);
    fn_1_1F868(&work->target, total, lbl_1_rodata_39C,
        lbl_1_rodata_398);
    Hu3DModelRotGet(obj->mdlId[obj->work[3]], &rotation);
    work->values[0] = rotation.y;

    if (obj->work[2] != 0) {
        for (i = 0, work = &lbl_1_bss_8EC[3]; i < slotCount;
            i++, work++) {
            if (mask & (1 << i)) {
                work->state = 1;
                work->time = lbl_1_rodata_104;
                work->duration = lbl_1_rodata_3A0;
                if (lbl_1_bss_1278.values[3] == 0) {
                    Hu3DModelPosGet(obj->mdlId[obj->work[3]],
                        &work->current);
                    fn_1_1F868(&work->middle, lbl_1_data_0[i].x,
                        lbl_1_rodata_3A4, lbl_1_rodata_3A8);
                    fn_1_1F868(&work->target, lbl_1_data_0[i].x,
                        lbl_1_rodata_384, lbl_1_rodata_3AC);
                } else {
                    Hu3DModelPosGet(obj->mdlId[obj->work[3]],
                        &work->current);
                    fn_1_1F868(&work->middle,
                        lbl_1_data_0[i + 4].x, lbl_1_rodata_3A4,
                        lbl_1_rodata_3B0);
                    fn_1_1F868(&work->target,
                        lbl_1_data_0[i + 4].x, lbl_1_rodata_384,
                        lbl_1_rodata_3B4);
                }
            }
        }
    }
    HuAudFXStop(lbl_1_bss_1298);
    HuAudFXPlay(1173);
    obj->objFunc = fn_1_9924;
}

void fn_1_A2B4(OMOBJ *obj)
{
    MDRESULT_MOVE_WORK *work;
    HuVecF rotation;
    HuVecF target;
    HuVecF position;
    float time;
    float acceleration;

    work = &lbl_1_bss_8EC[obj->work[3]];
    acceleration = lbl_1_rodata_360;
    switch (work->state) {
    case 0: {
        time = fn_1_1FC94(lbl_1_rodata_104, lbl_1_rodata_110,
            work->time, work->duration);
        acceleration = lbl_1_rodata_384 * time;
        fn_1_1F948(&rotation, &work->current, &work->middle,
            &work->target, time);
        Hu3DModelPosSetV(obj->mdlId[obj->work[3]], &rotation);
        Hu3DModelScaleSet(obj->mdlId[obj->work[3]],
            lbl_1_rodata_3B8 * time, lbl_1_rodata_3B8 * time,
            lbl_1_rodata_3B8 * time);
        if ((work->time += lbl_1_rodata_110) > work->duration) {
            work->state = 1;
            work->time = lbl_1_rodata_104;
            work->duration = lbl_1_rodata_380;
        }
        break;
    }
    case 1: {
        time = fn_1_1FC94(lbl_1_rodata_104, lbl_1_rodata_F4,
            work->time, work->duration);
        acceleration = lbl_1_rodata_360;
        if ((work->time += lbl_1_rodata_110) > work->duration) {
            work->time = lbl_1_rodata_104;
            work->duration = lbl_1_rodata_380;
        }
        if ((obj->work[0] += 1) > 20) {
            obj->work[0] = 0;
            work->values[1] = lbl_1_data_0[rand8() % 4].x;
        }
        Hu3DModelPosGet(obj->mdlId[obj->work[3]], &rotation);
        target.x = work->values[1];
        target.y = lbl_1_rodata_39C + time;
        target.z = lbl_1_rodata_398;
        fn_1_1FB50(&rotation, &target, lbl_1_rodata_F4);
        Hu3DModelPosSetV(obj->mdlId[obj->work[3]], &rotation);
        Hu3DModelRotGet(obj->mdlId[obj->work[3]], &rotation);
        rotation.y -= lbl_1_rodata_2B4;
        if (rotation.y > lbl_1_rodata_288) {
            rotation.y -= lbl_1_rodata_288;
        }
        Hu3DModelRotSetV(obj->mdlId[obj->work[3]], &rotation);
        break;
    }
    }

    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &position);
    fn_1_26070((s16)(obj->work[3] + 4), -1, &position,
        acceleration, NULL);
}

void fn_1_A624(s16 index)
{
    OMOBJ *obj = lbl_1_bss_1C;
    MDRESULT_MOVE_WORK *work;
    MDRESULT_U8_TABLE_12 color;
    HuVecF position;

    work = &lbl_1_bss_8EC[index];
    obj->work[0] = 0;
    obj->work[3] = index;
    work->state = 0;
    work->time = lbl_1_rodata_104;
    work->duration = lbl_1_rodata_380;
    fn_1_1F868(&work->current, lbl_1_rodata_104, lbl_1_rodata_39C,
        lbl_1_rodata_3C8);
    fn_1_1F868(&work->middle, lbl_1_rodata_104, lbl_1_rodata_39C,
        lbl_1_rodata_2D4);
    fn_1_1F868(&work->target, lbl_1_rodata_104, lbl_1_rodata_39C,
        lbl_1_rodata_398);
    Hu3DModelScaleSet(obj->mdlId[obj->work[3]], lbl_1_rodata_104,
        lbl_1_rodata_104, lbl_1_rodata_104);
    Hu3DModelAttrReset(obj->mdlId[obj->work[3]], HU3D_ATTR_DISPOFF);

    color = lbl_1_rodata_3BC;
    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &position);
    fn_1_25E6C((s16)(obj->work[3] + 4), 2, &position,
        lbl_1_rodata_110, &color.values[obj->work[3] * 4]);
    lbl_1_bss_1298 = HuAudFXPlay(1172);
    obj->objFunc = fn_1_A2B4;
}

void fn_1_A85C(OMOBJ *obj)
{
    MDRESULT_MOVE_WORK *work = &lbl_1_bss_8EC[obj->work[3]];
    HuVecF position;

    fn_1_1F948(&position, &work->current, &work->middle, &work->target,
        fn_1_1FC94(lbl_1_rodata_104, lbl_1_rodata_110, work->time,
            work->duration));
    Hu3DModelPosSetV(obj->mdlId[obj->work[3]], &position);
    if ((work->time += lbl_1_rodata_110) > work->duration) {
        Hu3DModelAttrSet(obj->mdlId[obj->work[3]], HU3D_ATTR_DISPOFF);
        obj->objFunc = NULL;
        fn_1_25FF4((s16)(obj->work[3] + 4));
    }
    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &position);
    fn_1_26070((s16)(obj->work[3] + 4), -1, &position,
        lbl_1_rodata_360, NULL);
}

void fn_1_11208(s16 index)
{
    MDRESULT_MESSAGE_TABLE_48 messages = lbl_1_rodata_4A8;
    OMOBJ *obj;
    MDRESULT_MOVE_WORK *work;
    s16 displayWin;
    s16 count;
    s16 mode;
    s16 i;
    s16 window;
    s16 insertPos;
    u8 mask;

    mode = lbl_1_bss_1278.values[3];
    displayWin = 3;
    if (index == 1) {
        displayWin = 2;
    }

    if (displayWin == 3) {
        obj = lbl_1_bss_4;
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
            lbl_1_rodata_104, lbl_1_rodata_F8, 0);
        obj->work[3] = 0;
        obj->objFunc = fn_1_4A9C;
    } else {
        obj = lbl_1_bss_8;
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
            lbl_1_rodata_104, lbl_1_rodata_F8, 0);
        obj->work[3] = 0;
        obj->objFunc = fn_1_4BB8;
    }

    fn_1_258C(displayWin, messages.values[mode * 30 + index], 1);
    fn_1_246C();
    count = fn_1_1109C(index, &mask);
    fn_1_A624(index);
    HuPrcSleep(60);
    obj = lbl_1_bss_28;
    obj->work[0] = 1;
    obj->work[1] = 0;
    HuPrcSleep(180);

    if (mode == 0) {
        if (count == 2) {
            if (displayWin == 3) {
                obj = lbl_1_bss_4;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
                    lbl_1_rodata_104, lbl_1_rodata_F8, 0);
                obj->work[3] = 0;
                obj->objFunc = fn_1_4A9C;
            } else {
                obj = lbl_1_bss_8;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
                    lbl_1_rodata_104, lbl_1_rodata_F8, 0);
                obj->work[3] = 0;
                obj->objFunc = fn_1_4BB8;
            }
            fn_1_258C(displayWin, messages.values[index + 6], 1);
            fn_1_246C();
        } else if (count == 3) {
            if (displayWin == 3) {
                obj = lbl_1_bss_4;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
                    lbl_1_rodata_104, lbl_1_rodata_F8, 0);
                obj->work[3] = 0;
                obj->objFunc = fn_1_4A9C;
            } else {
                obj = lbl_1_bss_8;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
                    lbl_1_rodata_104, lbl_1_rodata_F8, 0);
                obj->work[3] = 0;
                obj->objFunc = fn_1_4BB8;
            }
            fn_1_258C(displayWin, messages.values[index + 12], 1);
            fn_1_246C();
        }

        fn_1_9EBC(count, mask);
        obj = lbl_1_bss_28;
        obj->work[0] = 3;
        obj->work[1] = 0;
        HuPrcSleep(60);
        if (count == 0 || count == 4) {
            HuAudFXPlay(1176);
        } else {
            HuAudFXPlay(1175);
        }
        obj = lbl_1_bss_28;
        obj->work[0] = 2;
        obj->work[1] = mask;
        for (i = 0; i < 4; i++) {
            if ((mask & (1 << i)) == 0) {
                fn_1_3364(i, 6, lbl_1_rodata_260, 0);
            } else if (count == 4) {
                fn_1_3364(i, 6, lbl_1_rodata_260, 0);
            } else {
                fn_1_3364(i, 5, lbl_1_rodata_260, 0);
            }
        }
        obj = lbl_1_bss_C;
        for (i = 0; i < 4; i++) {
            obj->work[i] = 0;
        }
        obj->objFunc = fn_1_3668;

        if (count == 0 || count == 4) {
            window = displayWin;
        } else {
            window = 4;
        }
        insertPos = 0;
        for (i = 0; i < 4; i++) {
            if (mask & (1 << i)) {
                fn_1_27A4(window, lbl_1_bss_1278.messages[i], insertPos);
                insertPos++;
            }
        }

        if (count == 1) {
            obj = lbl_1_bss_4;
            Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[4],
                lbl_1_rodata_104, lbl_1_rodata_F8, 0);
            obj = lbl_1_bss_8;
            Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[4],
                lbl_1_rodata_104, lbl_1_rodata_F8, 0);
            fn_1_258C(4, messages.values[index + 3], 1);
            fn_1_246C();
        } else if (count == 0) {
            if (displayWin == 3) {
                obj = lbl_1_bss_4;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
                    lbl_1_rodata_104, lbl_1_rodata_F8, 0);
                obj->work[3] = 0;
                obj->objFunc = fn_1_4A9C;
            } else {
                obj = lbl_1_bss_8;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
                    lbl_1_rodata_104, lbl_1_rodata_F8, 0);
                obj->work[3] = 0;
                obj->objFunc = fn_1_4BB8;
            }
            fn_1_258C(displayWin, messages.values[index + 24], 1);
            fn_1_246C();
            fn_1_258C(displayWin, messages.values[index + 27], 1);
            fn_1_246C();
        } else if (count == 2) {
            obj = lbl_1_bss_4;
            Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[4],
                lbl_1_rodata_104, lbl_1_rodata_F8, 0);
            obj = lbl_1_bss_8;
            Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[4],
                lbl_1_rodata_104, lbl_1_rodata_F8, 0);
            fn_1_258C(4, messages.values[index + 9], 1);
            fn_1_246C();
        } else if (count == 3) {
            obj = lbl_1_bss_4;
            Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[4],
                lbl_1_rodata_104, lbl_1_rodata_F8, 0);
            obj = lbl_1_bss_8;
            Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[4],
                lbl_1_rodata_104, lbl_1_rodata_F8, 0);
            fn_1_258C(4, messages.values[index + 15], 1);
            fn_1_246C();
        } else if (count == 4) {
            if (displayWin == 3) {
                obj = lbl_1_bss_4;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
                    lbl_1_rodata_104, lbl_1_rodata_F8, 0);
                obj->work[3] = 0;
                obj->objFunc = fn_1_4A9C;
            } else {
                obj = lbl_1_bss_8;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
                    lbl_1_rodata_104, lbl_1_rodata_F8, 0);
                obj->work[3] = 0;
                obj->objFunc = fn_1_4BB8;
            }
            fn_1_258C(displayWin, messages.values[index + 18], 1);
            fn_1_246C();
            fn_1_258C(displayWin, messages.values[index + 21], 1);
            fn_1_246C();
        }

        if (count != 0 && count != 4) {
            for (i = 0; i < 4; i++) {
                if (mask & (1 << i)) {
                    lbl_1_bss_10D4[i].star++;
                }
            }
        }
    } else {
        obj = lbl_1_bss_28;
        obj->work[0] = 3;
        obj->work[1] = 0;
        fn_1_9EBC(count, mask);
        HuPrcSleep(60);
        if (count == 0 || count == 2) {
            HuAudFXPlay(1176);
        } else {
            HuAudFXPlay(1175);
        }
        obj = lbl_1_bss_28;
        obj->work[0] = 2;
        obj->work[1] = mask;

        if (count == 0 || count == 2) {
            for (i = 0; i < 4; i++) {
                fn_1_3364(i, 6, lbl_1_rodata_260, 0);
            }
        } else if (mask & 1) {
            fn_1_3364(0, 5, lbl_1_rodata_260, 0);
            fn_1_3364(1, 5, lbl_1_rodata_260, 0);
            fn_1_3364(2, 6, lbl_1_rodata_260, 0);
            fn_1_3364(3, 6, lbl_1_rodata_260, 0);
        } else {
            fn_1_3364(0, 6, lbl_1_rodata_260, 0);
            fn_1_3364(1, 6, lbl_1_rodata_260, 0);
            fn_1_3364(2, 5, lbl_1_rodata_260, 0);
            fn_1_3364(3, 5, lbl_1_rodata_260, 0);
        }

        obj = lbl_1_bss_C;
        for (i = 0; i < 4; i++) {
            obj->work[i] = 0;
        }
        obj->objFunc = fn_1_3668;

        window = count == 1 ? 4 : displayWin;
        if (mask & 1) {
            fn_1_27A4(window, lbl_1_bss_1278.messages[0], 0);
            fn_1_27A4(window, lbl_1_bss_1278.messages[1], 1);
            fn_1_27A4(window, lbl_1_bss_1278.messages[4], 2);
        } else {
            fn_1_27A4(window, lbl_1_bss_1278.messages[2], 0);
            fn_1_27A4(window, lbl_1_bss_1278.messages[3], 1);
            fn_1_27A4(window, lbl_1_bss_1278.messages[5], 2);
        }

        if (count == 1) {
            obj = lbl_1_bss_4;
            Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[4],
                lbl_1_rodata_104, lbl_1_rodata_F8, 0);
            obj = lbl_1_bss_8;
            Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[4],
                lbl_1_rodata_104, lbl_1_rodata_F8, 0);
            fn_1_258C(4, messages.values[index + 33], 1);
            fn_1_246C();
        } else if (count == 0) {
            if (displayWin == 3) {
                obj = lbl_1_bss_4;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
                    lbl_1_rodata_104, lbl_1_rodata_F8, 0);
                obj->work[3] = 0;
                obj->objFunc = fn_1_4A9C;
            } else {
                obj = lbl_1_bss_8;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
                    lbl_1_rodata_104, lbl_1_rodata_F8, 0);
                obj->work[3] = 0;
                obj->objFunc = fn_1_4BB8;
            }
            fn_1_258C(displayWin, messages.values[index + 42], 1);
            fn_1_246C();
            fn_1_258C(displayWin, messages.values[index + 45], 1);
            fn_1_246C();
        } else if (count == 2) {
            if (displayWin == 3) {
                obj = lbl_1_bss_4;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
                    lbl_1_rodata_104, lbl_1_rodata_F8, 0);
                obj->work[3] = 0;
                obj->objFunc = fn_1_4A9C;
            } else {
                obj = lbl_1_bss_8;
                Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
                    lbl_1_rodata_104, lbl_1_rodata_F8, 0);
                obj->work[3] = 0;
                obj->objFunc = fn_1_4BB8;
            }
            fn_1_258C(displayWin, messages.values[index + 36], 1);
            fn_1_246C();
            fn_1_258C(displayWin, messages.values[index + 39], 1);
            fn_1_246C();
        }

        if (count == 1) {
            for (i = 0; i < 2; i++) {
                if (mask & (1 << i)) {
                    lbl_1_bss_10D4[i].star++;
                }
            }
        }
    }

    obj = lbl_1_bss_4;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
        lbl_1_rodata_104, lbl_1_rodata_260,
        HU3D_MOTATTR_LOOP);
    obj = lbl_1_bss_8;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
        lbl_1_rodata_104, lbl_1_rodata_260,
        HU3D_MOTATTR_LOOP);

    obj = lbl_1_bss_1C;
    do {
        HuPrcVSleep();
    } while (obj->objFunc != NULL);

    obj = lbl_1_bss_28;
    obj->work[0] = 0;
    obj->work[1] = 0;
    obj = lbl_1_bss_1C;
    work = &lbl_1_bss_8EC[obj->work[3]];
    work->state = 0;
    work->time = lbl_1_rodata_104;
    work->duration = lbl_1_rodata_380;
    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &work->current);
    fn_1_1F868(&work->middle, lbl_1_rodata_104,
        lbl_1_rodata_104, lbl_1_rodata_3CC);
    fn_1_1F868(&work->target, lbl_1_rodata_104,
        lbl_1_rodata_2C0, lbl_1_rodata_31C);
    HuAudFXPlay(1174);
    obj->objFunc = fn_1_A85C;
    fn_1_37EC();
}

s32 fn_1_12D7C(u8 mask)
{
    OMOBJ *obj;
    OMOBJ *second;
    s16 result;
    s16 i;

    HuPrcSleep(120);
    lbl_1_bss_12B0[0] = HuAudSStreamPlay(34);

    if (lbl_1_bss_1278.values[3] == 0) {
        fn_1_258C(2, 917528, 1);
        fn_1_246C();
        fn_1_258C(2, 917529, 1);
        fn_1_246C();
        fn_1_295C(65539, 0);

        obj = lbl_1_bss_20;
        for (i = 0; i < 4; i++) {
            if (mask & (1 << i)) {
                lbl_1_bss_8AC[i].state = 1;
                lbl_1_bss_8AC[i].time = lbl_1_rodata_104;
                lbl_1_bss_8AC[i].delay = lbl_1_rodata_F8;
                Hu3DMotionSpeedSet(obj->mdlId[i], lbl_1_rodata_110);
                Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
                    lbl_1_rodata_104, lbl_1_rodata_104,
                    HU3D_MOTATTR_LOOP);
            }
        }
        obj->objFunc = fn_1_BB60;
        result = fn_1_C9A0();
        fn_1_2B44();

        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_10D4[i].rank == 0) {
                if (i == result) {
                    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[i],
                        lbl_1_bss_C->mtnId[i + 20], lbl_1_rodata_104,
                        lbl_1_rodata_260, 0);
                    lbl_1_bss_10D4[i].rank = 0;
                } else {
                    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[i],
                        lbl_1_bss_C->mtnId[i + 24], lbl_1_rodata_104,
                        lbl_1_rodata_260, 0);
                    lbl_1_bss_10D4[i].rank = 1;
                }
            }
        }
        fn_1_27A4(2, lbl_1_bss_1278.messages[result], 0);

        obj = lbl_1_bss_C;
        for (i = 0; i < 4; i++) {
            obj->work[i] = 0;
        }
        obj->objFunc = fn_1_3668;
        fn_1_258C(2, 917530, 1);
        fn_1_246C();
    } else {
        fn_1_258C(2, 917543, 1);
        fn_1_246C();
        fn_1_258C(2, 917529, 1);
        fn_1_246C();
        fn_1_295C(65539, 0);

        obj = lbl_1_bss_20;
        for (i = 0; i < 4; i++) {
            if (mask & (1 << i)) {
                lbl_1_bss_8AC[i].state = 1;
                lbl_1_bss_8AC[i].time = lbl_1_rodata_104;
                lbl_1_bss_8AC[i].delay = lbl_1_rodata_F8;
                Hu3DMotionSpeedSet(obj->mdlId[i], lbl_1_rodata_110);
                Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
                    lbl_1_rodata_104, lbl_1_rodata_104,
                    HU3D_MOTATTR_LOOP);
            }
        }
        obj->objFunc = fn_1_BB60;
        result = fn_1_C9A0();
        fn_1_2B44();

        if (result == 0) {
            lbl_1_bss_10D4[0].rank = 0;
            lbl_1_bss_10D4[1].rank = 1;
            Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0],
                lbl_1_bss_C->mtnId[20], lbl_1_rodata_104,
                lbl_1_rodata_260, 0);
            Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[1],
                lbl_1_bss_C->mtnId[21], lbl_1_rodata_104,
                lbl_1_rodata_260, 0);
            Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[2],
                lbl_1_bss_C->mtnId[26], lbl_1_rodata_104,
                lbl_1_rodata_260, 0);
            Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[3],
                lbl_1_bss_C->mtnId[27], lbl_1_rodata_104,
                lbl_1_rodata_260, 0);
            fn_1_27A4(2, lbl_1_bss_1278.messages[0], 0);
            fn_1_27A4(2, lbl_1_bss_1278.messages[1], 1);
            fn_1_27A4(2, lbl_1_bss_1278.messages[4], 2);
        } else {
            lbl_1_bss_10D4[0].rank = 1;
            lbl_1_bss_10D4[1].rank = 0;
            Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0],
                lbl_1_bss_C->mtnId[24], lbl_1_rodata_104,
                lbl_1_rodata_260, 0);
            Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[1],
                lbl_1_bss_C->mtnId[25], lbl_1_rodata_104,
                lbl_1_rodata_260, 0);
            Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[2],
                lbl_1_bss_C->mtnId[22], lbl_1_rodata_104,
                lbl_1_rodata_260, 0);
            Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[3],
                lbl_1_bss_C->mtnId[23], lbl_1_rodata_104,
                lbl_1_rodata_260, 0);
            fn_1_27A4(2, lbl_1_bss_1278.messages[2], 0);
            fn_1_27A4(2, lbl_1_bss_1278.messages[3], 1);
            fn_1_27A4(2, lbl_1_bss_1278.messages[5], 2);
        }

        obj = lbl_1_bss_C;
        for (i = 0; i < 4; i++) {
            obj->work[i] = 0;
        }
        obj->objFunc = fn_1_3668;
        fn_1_258C(2, 917544, 1);
        fn_1_246C();
    }

    obj = lbl_1_bss_20;
    for (i = 0; i < 13; i++) {
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    second = lbl_1_bss_24;
    for (i = 0; i < 22; i++) {
        Hu3DModelAttrSet(second->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    obj->objFunc = NULL;

    fn_1_258C(2, 917531, 1);
    fn_1_246C();

    obj = lbl_1_bss_4;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
        lbl_1_rodata_104, lbl_1_rodata_260,
        HU3D_MOTATTR_LOOP);
    obj->objFunc = NULL;
    HuAudSStreamFadeOut(lbl_1_bss_12B0[0], 1000);
    return TRUE;
}

s32 fn_1_15378(void)
{
    s16 slotCount = 4;
    s16 selected = 0;
    s16 i;

    if (lbl_1_bss_1278.values[3] == 1) {
        slotCount = 2;
    }
    for (i = 0; i < slotCount; i++) {
        if (lbl_1_bss_10D4[i].rank == 0) {
            selected = i;
            break;
        }
    }

    fn_1_23C0();
    fn_1_DC38(selected);
    HuPrcSleep(90);
    lbl_1_bss_12B0[2] = HuAudSStreamPlay(35);
    HuPrcSleep(510);

    if (lbl_1_bss_1278.values[3] == 0) {
        fn_1_27A4(4, lbl_1_bss_1278.messages[selected], 0);
        fn_1_258C(4, 917532, 1);
        fn_1_246C();
    } else if (selected == 0) {
        fn_1_27A4(4, lbl_1_bss_1278.messages[0], 0);
        fn_1_27A4(4, lbl_1_bss_1278.messages[1], 1);
        fn_1_27A4(4, lbl_1_bss_1278.messages[4], 2);
        fn_1_258C(4, 917545, 1);
        fn_1_246C();
    } else {
        fn_1_27A4(4, lbl_1_bss_1278.messages[2], 0);
        fn_1_27A4(4, lbl_1_bss_1278.messages[3], 1);
        fn_1_27A4(4, lbl_1_bss_1278.messages[5], 2);
        fn_1_258C(4, 917545, 1);
        fn_1_246C();
    }

    fn_1_23C0();
    HuAudSStreamFadeOut(lbl_1_bss_12B0[2], 1000);
    HuPrcSleep(60);
    lbl_1_bss_12B0[1] = HuAudSStreamPlay(36);
    lbl_1_bss_129C = HuAudFXPlay(1178);
    fn_1_E658(selected);
    return TRUE;
}

void fn_1_1648C(void)
{
    s32 order[4];
    s32 i;
    s32 j;
    s32 temp;
    s32 rank;

    order[0] = lbl_1_rodata_568[0];
    order[1] = lbl_1_rodata_568[1];
    order[2] = lbl_1_rodata_568[2];
    order[3] = lbl_1_rodata_568[3];

    if (lbl_1_bss_1278.values[3] == 0) {
        for (i = 0; i < 4; i++) {
            for (j = i; j < 4; j++) {
                if (lbl_1_bss_10D4[order[j]].star >=
                    lbl_1_bss_10D4[order[i]].star) {
                    temp = order[i];
                    order[i] = order[j];
                    order[j] = temp;
                }
            }
        }
        for (i = 0; i < 4; i++) {
            for (j = i; j < 4; j++) {
                if (lbl_1_bss_10D4[order[j]].star ==
                        lbl_1_bss_10D4[order[i]].star
                    && lbl_1_bss_10D4[order[j]].coin >=
                        lbl_1_bss_10D4[order[i]].coin) {
                    temp = order[i];
                    order[i] = order[j];
                    order[j] = temp;
                }
            }
        }
        lbl_1_bss_10D4[order[0]].rank = 0;
        rank = 0;
        for (i = 1; i < 4; i++) {
            rank++;
            lbl_1_bss_10D4[order[i]].rank = rank;
            if (lbl_1_bss_10D4[order[i]].star ==
                    lbl_1_bss_10D4[order[i - 1]].star
                && lbl_1_bss_10D4[order[i]].coin ==
                    lbl_1_bss_10D4[order[i - 1]].coin) {
                lbl_1_bss_10D4[order[i]].rank =
                    lbl_1_bss_10D4[order[i - 1]].rank;
            }
        }
        for (i = 0; i < 4; i++) {
            OSReport(lbl_1_data_750, lbl_1_bss_10D4[i].rank);
        }
    } else {
        if (lbl_1_bss_10D4[0].star == lbl_1_bss_10D4[1].star) {
            if (lbl_1_bss_10D4[0].coin == lbl_1_bss_10D4[1].coin) {
                lbl_1_bss_10D4[0].rank = 0;
                lbl_1_bss_10D4[1].rank = 0;
            } else if (lbl_1_bss_10D4[0].coin >
                lbl_1_bss_10D4[1].coin) {
                lbl_1_bss_10D4[0].rank = 0;
                lbl_1_bss_10D4[1].rank = 1;
            } else {
                lbl_1_bss_10D4[0].rank = 1;
                lbl_1_bss_10D4[1].rank = 0;
            }
        } else if (lbl_1_bss_10D4[0].star >
            lbl_1_bss_10D4[1].star) {
            lbl_1_bss_10D4[0].rank = 0;
            lbl_1_bss_10D4[1].rank = 1;
        } else {
            lbl_1_bss_10D4[0].rank = 1;
            lbl_1_bss_10D4[1].rank = 0;
        }
        for (i = 0; i < 2; i++) {
            OSReport(lbl_1_data_750, lbl_1_bss_10D4[i].rank);
        }
    }
}

void fn_1_169A4(void)
{
    OMOBJ *obj;
    u8 mask;
    s16 playerCount;
    s16 zeroCount;
    s16 runMask;
    s16 i;

    obj = lbl_1_bss_4;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_4A9C;

    obj = lbl_1_bss_8;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_4BB8;

    fn_1_2264(4);
    if (lbl_1_data_64C[0] != 917504) {
        lbl_1_data_64C[0] = 917504;
        fn_1_1E28(lbl_1_data_646[0], lbl_1_data_64C[0], 1);
    }
    if (lbl_1_data_646[0] != -1) {
        HuWinMesWait(lbl_1_bss_1304[lbl_1_data_646[0]]);
    }
    fn_1_105CC();
    fn_1_10B34();
    if (lbl_1_bss_1278.values[2] != 0) {
        fn_1_11208(0);
        fn_1_11208(1);
        fn_1_11208(2);
    }

    obj = lbl_1_bss_8;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_4BB8;

    fn_1_2264(2);
    if (lbl_1_data_64C[0] != 917527) {
        lbl_1_data_64C[0] = 917527;
        fn_1_1E28(lbl_1_data_646[0], lbl_1_data_64C[0], 1);
    }
    if (lbl_1_data_646[0] != -1) {
        HuWinMesWait(lbl_1_bss_1304[lbl_1_data_646[0]]);
    }
    if (lbl_1_data_646[0] != -1) {
        fn_1_1CE0(lbl_1_data_646[0]);
    }
    lbl_1_data_646[0] = -1;
    lbl_1_data_64C[0] = -1;

    fn_1_1648C();
    HuAudSStreamFadeOut(lbl_1_bss_12B0[0], 1000);

    mask = 0;
    playerCount = 4;
    zeroCount = 0;
    if (lbl_1_bss_1278.values[3] == 1) {
        playerCount = 2;
    }
    for (i = 0; i < playerCount; i++) {
        if (lbl_1_bss_10D4[i].rank == 0) {
            zeroCount++;
        }
    }
    if (zeroCount == 1) {
        runMask = 0;
    } else {
        for (i = 0; i < playerCount; i++) {
            if (lbl_1_bss_10D4[i].rank == 0) {
                mask |= 1 << i;
            }
        }
        runMask = 1;
    }
    if (runMask != 0) {
        fn_1_12D7C(mask);
    }
    fn_1_15378();
}

void fn_1_1F7FC(void)
{
    fn_1_1F3D4();
    HuSprAttrReset(lbl_1_bss_60, 0, HUSPR_ATTR_DISPOFF);
}

void fn_1_1F868(HuVecF *vec, float x, float y, float z)
{
    vec->x = x;
    vec->y = y;
    vec->z = z;
}

float fn_1_1F878(float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_E70) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}

float fn_1_1F8EC(float start, float middle, float end, float time)
{
    float inverse = lbl_1_rodata_E74 - time;

    return (end * (time * time))
        + ((start * (inverse * inverse))
            + (lbl_1_rodata_E78 * (middle * (inverse * time))));
}

void fn_1_1F948(HuVecF *result, const HuVecF *start,
    const HuVecF *middle, const HuVecF *end, float time)
{
    result->x = fn_1_1F8EC(start->x, middle->x, end->x, time);
    result->y = fn_1_1F8EC(start->y, middle->y, end->y, time);
    result->z = fn_1_1F8EC(start->z, middle->z, end->z, time);
}

void fn_1_20CE0(void)
{
    Hu3DModelKill(lbl_1_bss_14C6);
}

void fn_1_20F80(void)
{
    Hu3DModelKill(lbl_1_bss_14C4);
}

void fn_1_216E8(void)
{
    Hu3DModelKill(lbl_1_bss_14C2);
}

void fn_1_24BB4(void)
{
    Hu3DModelKill(lbl_1_bss_131A);
}

void fn_1_24C28(void)
{
    Hu3DModelAttrSet(lbl_1_bss_1318, HU3D_ATTR_DISPOFF);
}

void fn_1_252CC(void)
{
    Hu3DModelKill(lbl_1_bss_1318);
}

void fn_1_7518(OMOBJ *obj)
{
    if (obj) {
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_83E0(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 4; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_100B8(void)
{
    OSReport(lbl_1_data_719);
    HuDataDirCloseAll();
    fn_1_F548();
}

void fn_1_1018C(s32 unused, MDRESULT_CAMERA_WORK *work)
{
    work->targetCenter.x = lbl_1_rodata_104;
    work->targetCenter.y = lbl_1_rodata_114;
    work->targetCenter.z = lbl_1_rodata_118;
    work->targetRot.x = lbl_1_rodata_11C;
    work->targetRot.y = lbl_1_rodata_104;
    work->targetRot.z = lbl_1_rodata_104;
    work->targetZoom = lbl_1_rodata_4A4;
    fn_1_1FB50(&work->center, &work->targetCenter, lbl_1_rodata_260);
    fn_1_1FB50(&work->rot, &work->targetRot, lbl_1_rodata_260);
    work->zoom = fn_1_1F8BC(
        work->zoom, work->targetZoom, lbl_1_rodata_260);
}

void fn_1_10270(s32 unused, MDRESULT_CAMERA_WORK *work)
{
    work->center.x = lbl_1_rodata_104;
    work->center.y = lbl_1_rodata_114;
    work->center.z = lbl_1_rodata_118;
    work->rot.x = lbl_1_rodata_11C;
    work->rot.y = lbl_1_rodata_104;
    work->rot.z = lbl_1_rodata_104;
    work->zoom = lbl_1_rodata_4A4;
}

s32 fn_1_102E4(void)
{
    OMOBJ *first;
    OMOBJ *second;

    first = lbl_1_bss_4;
    Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    first->work[3] = 0;
    first->objFunc = fn_1_4A9C;
    second = lbl_1_bss_8;
    Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    second->work[3] = 0;
    second->objFunc = fn_1_4BB8;
    fn_1_258C(4, 917504, 1);
    fn_1_246C();
    return TRUE;
}

s32 fn_1_105CC(void)
{
    OMOBJ *first;
    OMOBJ *second;
    OMOBJ *third;

    first = lbl_1_bss_4;
    Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    first->work[3] = 0;
    first->objFunc = fn_1_4A9C;
    fn_1_258C(3, 917505, 1);
    fn_1_246C();
    HuAudFXPlay(1168);
    fn_1_8B70(0);
    HuPrcSleep(60);
    second = lbl_1_bss_4;
    Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    second->work[3] = 0;
    second->objFunc = fn_1_4A9C;
    fn_1_258C(3, 917506, 1);
    fn_1_246C();
    third = lbl_1_bss_18;
    third->work[0] = 0;
    third->work[1] = 10;
    third->work[2] = 0;
    third->objFunc = fn_1_8F28;
    HuPrcSleep(10);
    HuAudFXPlay(1169);
    HuPrcSleep(50);
    return TRUE;
}

s32 fn_1_10B34(void)
{
    OMOBJ *first;
    OMOBJ *second;
    OMOBJ *third;

    first = lbl_1_bss_8;
    Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    first->work[3] = 0;
    first->objFunc = fn_1_4BB8;
    fn_1_258C(2, 917507, 1);
    fn_1_246C();
    HuAudFXPlay(1170);
    fn_1_8B70(1);
    HuPrcSleep(60);
    second = lbl_1_bss_8;
    Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    second->work[3] = 0;
    second->objFunc = fn_1_4BB8;
    fn_1_258C(2, 917508, 1);
    fn_1_246C();
    third = lbl_1_bss_18;
    third->work[0] = 0;
    third->work[1] = 10;
    third->work[2] = 0;
    third->objFunc = fn_1_8F28;
    HuPrcSleep(10);
    HuAudFXPlay(1171);
    HuPrcSleep(50);
    return TRUE;
}

s32 fn_1_1295C(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_4BB8;
    fn_1_258C(2, 917527, 1);
    fn_1_246C();
    fn_1_23C0();
    return TRUE;
}

s32 fn_1_12C80(u8 *mask)
{
    u8 value;
    s16 playerCount;
    s16 zeroCount;
    s16 i;

    value = 0;
    playerCount = 4;
    zeroCount = 0;
    if (lbl_1_bss_1278.values[3] == 1) {
        playerCount = 2;
    }
    for (i = 0; i < playerCount; i++) {
        if (lbl_1_bss_10D4[i].rank == 0) {
            zeroCount++;
        }
    }
    if (zeroCount == 1) {
        return FALSE;
    }
    for (i = 0; i < playerCount; i++) {
        if (lbl_1_bss_10D4[i].rank == 0) {
            value |= 1 << i;
        }
    }
    *mask = value;
    return TRUE;
}





s32 fn_1_170DC(void)
{
    s16 i;

    HuPrcSleep(5);
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrReset(lbl_1_bss_C->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    Hu3DModelAttrReset(lbl_1_bss_4->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_8->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelShadowSet(lbl_1_bss_4->mdlId[0]);
    Hu3DModelShadowSet(lbl_1_bss_8->mdlId[0]);
    lbl_1_bss_12B0[0] = HuAudSStreamPlay(34);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    return TRUE;
}

s32 fn_1_171EC(void)
{
    HuAudSStreamFadeOut(lbl_1_bss_12B0[1], 1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    return TRUE;
}

void fn_1_17CF4(void)
{
    HUSPR_GROUPID *group = &lbl_1_bss_714;
    s16 bank = lbl_1_bss_1278.values[0];
    s16 otherBank = (lbl_1_bss_1278.values[1] - 10) / 5;

    fn_1_20188(group[0], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(group[0], 1, bank);
    HuSprBankSet(group[0], 2, otherBank);
}

void fn_1_1E1B4(OMOBJ *obj)
{
    if (lbl_1_bss_1278.values[3] == 0) {
        fn_1_1B194(obj);
    } else {
        fn_1_1C9B8(obj);
    }
}

void fn_1_1E204(void)
{
    if (lbl_1_bss_1278.values[3] == 0) {
        fn_1_1BAF4();
    } else {
        fn_1_1D318();
    }
    lbl_1_bss_3C->objFunc = fn_1_1E1B4;
}

void fn_1_221EC(void)
{
    s16 i;

    for (i = 0; i < 9; i++) {
        Hu3DModelKill(lbl_1_bss_14B0[i]);
    }
}

void fn_1_24BE0(HuVecF *pos)
{
    Hu3DModelPosSetV(lbl_1_bss_1318, pos);
    Hu3DModelAttrReset(lbl_1_bss_1318, HU3D_ATTR_DISPOFF);
}

void fn_1_95E8(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 4; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 81), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 3);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i], lbl_1_rodata_104,
            lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < 4; i++) {
        obj->mdlId[i + 4] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 80), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 4] = Hu3DMotionIDGet(obj->mdlId[i + 4]);
        Hu3DModelLayerSet(obj->mdlId[i + 4], 3);
        Hu3DMotionShiftSet(obj->mdlId[i + 4], obj->mtnId[i + 4],
            lbl_1_rodata_104, lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i + 4], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < 4; i++) {
        HuSprGrpDrawNoSet(lbl_1_bss_11A0[i], 64);
        fn_1_20108(lbl_1_bss_11A0[i], HUSPR_ATTR_DISPOFF);
    }
    obj->objFunc = NULL;
}

void fn_1_1922C(OMOBJ *obj)
{
    MDRESULT_PLAYER_WORK *work;
    s16 i;
    s16 j;

    i = 0;
    work = lbl_1_bss_66C;
    for (; i < 4; i++, work++) {
        for (j = 0; j < 3; j++) {
            Hu3DModelKill(obj->mdlId[j]);
        }
    }
}

void fn_1_18F08(OMOBJ *obj)
{
    MDRESULT_PLAYER_SPRITE_TABLE spriteInfo;
    MDRESULT_PLAYER_SPRITE_WORK *work;
    s16 player;
    s16 sprite;

    spriteInfo = lbl_1_rodata_6EC;
    player = 0;
    work = (MDRESULT_PLAYER_SPRITE_WORK *)lbl_1_bss_66C;
    for (; player < 4; player++, work++) {
        work->models[0] = Hu3DModelLink(obj->mdlId[0]);
        Hu3DModelLayerSet(work->models[0], 3);
        Hu3DMotionShiftSet(work->models[0], obj->mtnId[0], lbl_1_rodata_104,
            lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        work->models[1] = Hu3DModelLink(obj->mdlId[1]);
        Hu3DModelLayerSet(work->models[1], 3);
        Hu3DMotionShiftSet(work->models[1], obj->mtnId[1], lbl_1_rodata_104,
            lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        work->models[2] = Hu3DModelLink(obj->mdlId[2]);
        Hu3DModelLayerSet(work->models[2], 3);
        Hu3DMotionShiftSet(work->models[2], obj->mtnId[2], lbl_1_rodata_104,
            lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        work->group = HuSprGrpCreate(14);
        for (sprite = 0; sprite < 14; sprite++) {
            if (spriteInfo.values[sprite].animNo != -1) {
                work->sprites[sprite] = HuSprCreate(
                    lbl_1_bss_11AC[spriteInfo.values[sprite].animNo],
                    spriteInfo.values[sprite].priority,
                    spriteInfo.values[sprite].bank);
                HuSprGrpMemberSet(work->group, sprite, work->sprites[sprite]);
                HuSprPosSet(work->group, sprite,
                    spriteInfo.values[sprite].pos.x,
                    spriteInfo.values[sprite].pos.y);
                HuSprScaleSet(work->group, sprite,
                    spriteInfo.values[sprite].scale.x,
                    spriteInfo.values[sprite].scale.y);
                HuSprZRotSet(work->group, sprite,
                    spriteInfo.values[sprite].zRot);
            }
        }
        HuSprDrawNoSet(work->group, 7, 64);
        HuSprDrawNoSet(work->group, 8, 64);
        HuSprDrawNoSet(work->group, 9, 64);
        HuSprDrawNoSet(work->group, 10, 64);
        HuSprDrawNoSet(work->group, 11, 64);
        HuSprDrawNoSet(work->group, 12, 64);
        HuSprDrawNoSet(work->group, 13, 64);
    }
}

s16 fn_1_1109C(s16 index, u8 *mask)
{
    s16 scores[4];
    s16 playerCount;
    s16 maxScore;
    s16 winnerCount;
    s16 i;

    maxScore = 0;
    winnerCount = 0;
    if (lbl_1_bss_1278.values[3] == 0) {
        playerCount = 4;
    } else {
        playerCount = 2;
    }
    for (i = 0; i < playerCount; i++) {
        scores[i] = lbl_1_bss_10D4[i].values[index];
    }
    *mask = 0;
    i = 0;
    maxScore = 0;
    for (; i < playerCount; i++) {
        if (maxScore <= scores[i]) {
            maxScore = scores[i];
        }
    }
    for (i = 0; i < playerCount; i++) {
        if (maxScore == scores[i]) {
            winnerCount++;
            *mask |= 1 << i;
        }
    }
    if (maxScore == 0) {
        winnerCount = 0;
        *mask = 0;
    }
    return winnerCount;
}


s32 fn_1_1E4B8(HuVec2f *originA, HuVec2f *directionA, HuVec2f *originB,
    HuVec2f *directionB, HuVec2f *intersection)
{
    float slopeA;
    float cross;
    float slopeB;
    float interceptA;
    float absCross;
    float interceptB;

    cross = (directionB->x * directionA->y)
        - (directionB->y * directionA->x);
    if (cross < lbl_1_rodata_104) {
        absCross = -cross;
    } else {
        absCross = cross;
    }
    if (absCross < lbl_1_rodata_E5C) {
        intersection->x = originA->x;
        intersection->y = originA->y;
        return;
    }
    slopeA = directionA->y / directionA->x;
    slopeB = directionB->y / directionB->x;
    interceptA = originA->y - (slopeA * originA->x);
    interceptB = originB->y - (slopeB * originB->x);
    intersection->x = -((interceptA - interceptB) / (slopeA - slopeB));
    intersection->y = (slopeA * intersection->x) + interceptA;
    return 1;
}

void fn_1_1C050(void)
{
    HUSPR_GROUPID *group = lbl_1_bss_3D2;
    OMOBJ *obj;

    fn_1_20108(group[0], HUSPR_ATTR_DISPOFF);
    fn_1_20108(group[31], HUSPR_ATTR_DISPOFF);
    fn_1_20108(group[291], HUSPR_ATTR_DISPOFF);
    fn_1_1F834();
    obj = lbl_1_bss_30;
    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
}

void fn_1_1C9A0(void)
{
    HUSPR_GROUPID *group = lbl_1_bss_3D2;
}

void fn_1_1D874(void)
{
    HUSPR_GROUPID *group = lbl_1_bss_3D2;
    OMOBJ *obj;

    fn_1_20108(group[0], HUSPR_ATTR_DISPOFF);
    fn_1_20108(group[31], HUSPR_ATTR_DISPOFF);
    fn_1_20108(group[291], HUSPR_ATTR_DISPOFF);
    fn_1_1F834();
    obj = lbl_1_bss_30;
    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
}

void fn_1_1E19C(void)
{
    HUSPR_GROUPID *group = lbl_1_bss_3D2;
}

void fn_1_1E47C(void)
{
    if (lbl_1_bss_1278.values[3] == 0) {
        HUSPR_GROUPID *group = lbl_1_bss_3D2;
    } else {
        HUSPR_GROUPID *group = lbl_1_bss_3D2;
    }
}

void fn_1_1E258(void)
{
    if (lbl_1_bss_1278.values[3] == 0) {
        fn_1_1C050();
    } else {
        fn_1_1D874();
    }
    lbl_1_bss_3C->objFunc = NULL;
}

void fn_1_1E358(OMOBJ *obj)
{
    if (lbl_1_bss_1278.values[3] == 0) {
        fn_1_1C0C8(obj);
    } else {
        fn_1_1D8EC(obj);
    }
    fn_1_1E258();
    obj->objFunc = NULL;
}

void fn_1_1F834(void)
{
    HuSprAttrSet(lbl_1_bss_60, 0, HUSPR_ATTR_DISPOFF);
}

void fn_1_1F308(void)
{
    HUSPRID sprite;

    lbl_1_bss_60 = HuSprGrpCreate(2);
    sprite = HuSprFuncCreate(fn_1_1E5E8, 0);
    lbl_1_bss_5C = HuSprAnimRead(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 116), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_60, 0, sprite);
    HuSprPosSet(lbl_1_bss_60, 0, lbl_1_rodata_E68, lbl_1_rodata_E6C);
    HuSprAttrSet(lbl_1_bss_60, 0, HUSPR_ATTR_DISPOFF);
}

float fn_1_1F8BC(float current, float target, float weight)
{
    if (current == target) {
        return target;
    }
    return (target + (current * (weight - lbl_1_rodata_E74))) / weight;
}

void fn_1_1FB50(HuVecF *current, const HuVecF *target, float weight)
{
    current->x = fn_1_1F8BC(current->x, target->x, weight);
    current->y = fn_1_1F8BC(current->y, target->y, weight);
    current->z = fn_1_1F8BC(current->z, target->z, weight);
}

float fn_1_1FC94(float start, float end, float time, float duration)
{
    float phase;
    double angle;

    if (time <= lbl_1_rodata_E70) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    phase = (lbl_1_rodata_E88 / duration) * time;
    angle = (lbl_1_rodata_E80 * phase) / lbl_1_rodata_E90;
    return (float)(start + ((end - start) * sin(angle)));
}

float fn_1_1FD7C(float start, float end, float time, float duration)
{
    float phase;
    double angle;
    double easing;

    if (time <= lbl_1_rodata_E70) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    phase = (lbl_1_rodata_E88 / duration) * time;
    angle = (lbl_1_rodata_E80 * phase) / lbl_1_rodata_E90;
    easing = lbl_1_rodata_E98 - cos(angle);
    return (float)(start + ((end - start) * easing));
}

float fn_1_1FE74(float start, float end, float time, float duration)
{
    float phase;
    double angle;

    if (time <= lbl_1_rodata_E70 || time >= duration) {
        return start;
    }
    phase = (lbl_1_rodata_EA0 / duration) * time;
    angle = (lbl_1_rodata_E80 * phase) / lbl_1_rodata_E90;
    return (float)(start + ((end - start) * sin(angle)));
}

float fn_1_1FF48(float start, float end, float time, float duration)
{
    float phase;
    double angle;

    if (time <= lbl_1_rodata_E70 || time >= duration) {
        return start;
    }
    phase = (lbl_1_rodata_EA4 / duration) * time;
    angle = (lbl_1_rodata_E80 * phase) / lbl_1_rodata_E90;
    return (float)(start + ((end - start) * sin(angle)));
}

void fn_1_20188(HUSPR_GROUPID groupId, s32 attr)
{
    HUSPR_GROUP *group = &HuSprGrpData[groupId];
    s16 i;

    for (i = 0; i < group->sprNum; i++) {
        HuSprAttrReset(groupId, i, (u16)attr);
    }
}

void fn_1_20208(HUSPR_GROUPID groupId, s32 member, s16 value)
{
    s16 digit;

    digit = value / 100;
    HuSprBankSet(groupId, member, digit);
    if (digit == 0) {
        HuSprBankSet(groupId, member, 10);
    }
    digit = (value - (digit * 100)) / 10;
    HuSprBankSet(groupId, member + 1, digit);
    if (digit == 0 && value / 100 == 0) {
        HuSprAttrSet(groupId, member + 1, HUSPR_ATTR_DISPOFF);
    }
    digit = value % 10;
    HuSprBankSet(groupId, member + 2, digit);
}

void fn_1_2035C(HUSPR_GROUPID groupId, s32 member, s16 value)
{
    s16 digit;

    digit = value / 100;
    HuSprBankSet(groupId, member, digit);
    if (digit == 0) {
        HuSprAttrSet(groupId, member, HUSPR_ATTR_DISPOFF);
    }
    digit = (value - (digit * 100)) / 10;
    HuSprBankSet(groupId, member + 1, digit);
    if (digit == 0 && value / 100 == 0) {
        HuSprAttrSet(groupId, member + 1, HUSPR_ATTR_DISPOFF);
    }
    digit = value % 10;
    HuSprBankSet(groupId, member + 2, digit);
}

void fn_1_21714(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14B0[index]];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = particle->data;

    if (parManId > 0) {
        data->parManId = parManId;
    }
    if (velocity) {
        data->vel.x = velocity->x;
        data->vel.y = velocity->y;
        data->vel.z = velocity->z;
    }
    if (accelX > lbl_1_rodata_E70) {
        data->accel.x = accelX;
    }
    if (color) {
        data->color.r = color[0];
        data->color.g = color[1];
        data->color.b = color[2];
        data->color.a = 0;
    }
    data->accel.y = lbl_1_rodata_E70;
}

void fn_1_217EC(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color, float accelY)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14B0[index]];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = particle->data;

    fn_1_21714(index, parManId, velocity, accelX, color);
    data->accel.y = accelY;
}

void fn_1_21904(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14B0[index]];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        data->time = 0;
        data->parManId = 0;
        data->scale = lbl_1_rodata_E70;
    }
    data = particle->data;
    data->time = 1;
    fn_1_21714(index, parManId, velocity, accelX, color);
    model->attr &= ~HU3D_ATTR_DISPOFF;
}

void fn_1_26070(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color)
{
    if (index < 0 || index > 8) {
        return;
    }
    fn_1_21714(index, parManId, velocity, accelX, color);
}

void fn_1_25E6C(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color)
{
    if (index < 0 || index > 8) {
        return;
    }
    fn_1_21904(index, parManId, velocity, accelX, color);
}

void fn_1_22C38(void)
{
    s16 i;
    s16 j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            Hu3DModelKill(lbl_1_bss_1490[i][j]);
        }
    }
}

void fn_1_22CBC(MDRESULT_TRAIL_WORK *work)
{
    s16 i;

    for (i = work->pointCount - 1; i >= 1; i--) {
        work->points[i - 1].y -= lbl_1_rodata_F80;
        work->points[i].x = work->base.x + work->points[i - 1].x;
        work->points[i].y = work->base.y + work->points[i - 1].y;
        work->points[i].z = work->base.z + work->points[i - 1].z;
    }
    if (work->unk_28 == 0) {
        if (work->state == 1) {
            work->color.a += 5;
            if (work->color.a >= 255) {
                work->color.a = 255;
            }
        } else {
            work->color.a -= 5;
            if (work->color.a == 0) {
                work->color.a = 0;
                Hu3DModelAttrSet(lbl_1_bss_1480[work->modelIndex],
                    HU3D_ATTR_DISPOFF);
            }
        }
    }
}

void fn_1_23D38(s16 index, HuVecF *position, float value)
{
    MDRESULT_TRAIL_WORK *work = &lbl_1_bss_1320[index];

    work->points[0].x = position->x;
    work->points[0].y = position->y;
    work->points[0].z = position->z;
    work->base.x = work->base.y = work->base.z = lbl_1_rodata_E70;
    work->base.y = value;
}

void fn_1_2429C(s16 index)
{
    MDRESULT_TRAIL_WORK *work = &lbl_1_bss_1320[index];

    work->state = 0;
    Hu3DModelAttrSet(lbl_1_bss_1480[index], HU3D_ATTR_DISPOFF);
}

void fn_1_2001C(HU3D_MODELID modelId, const HuVecF *first,
    const HuVecF *second)
{
    HuVecF screen = lbl_1_rodata_EA8;
    HuVecF world;

    if (first) {
        screen.x += first->x;
        screen.y += first->y;
        screen.z += first->z;
    }
    if (second) {
        screen.x += second->x;
        screen.y += second->y;
        screen.z += second->z;
    }
    Hu3D2Dto3D(&screen, 1, &world);
    Hu3DModelPosSet(modelId, world.x, world.y, world.z);
}

void fn_1_20BC8(void)
{
    lbl_1_bss_14C6 = Hu3DParticleCreate(lbl_1_bss_14C8[0], 600);
    Hu3DModelPosSet(lbl_1_bss_14C6, lbl_1_rodata_E70,
        lbl_1_rodata_E70, lbl_1_rodata_E70);
    Hu3DModelRotSet(lbl_1_bss_14C6, lbl_1_rodata_E70,
        lbl_1_rodata_E70, lbl_1_rodata_E70);
    Hu3DModelScaleSet(lbl_1_bss_14C6, lbl_1_rodata_E74,
        lbl_1_rodata_E74, lbl_1_rodata_E74);
    Hu3DModelLayerSet(lbl_1_bss_14C6, 2);
    Hu3DParticleHookSet(lbl_1_bss_14C6, fn_1_20554);
    Hu3DParticleBlendModeSet(lbl_1_bss_14C6, 1);
}

void fn_1_20DAC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    s16 state = 0;
    MDRESULT_PARTICLE_PRESET preset = lbl_1_rodata_EFC;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        data->scale = lbl_1_rodata_F2C;
        data->color.r = 255;
        data->color.g = 255;
        data->color.b = 255;
        data->pos.x = data->vel.x;
        data->pos.y = data->vel.y;
        data->pos.z = data->vel.z;
    }
}

void fn_1_20E9C(void)
{
    lbl_1_bss_14C4 = Hu3DParticleCreate(lbl_1_bss_14C8[0], 4);
    Hu3DModelPosSet(lbl_1_bss_14C4, lbl_1_rodata_E70,
        lbl_1_rodata_E70, lbl_1_rodata_E70);
    Hu3DModelScaleSet(lbl_1_bss_14C4, lbl_1_rodata_E74,
        lbl_1_rodata_E74, lbl_1_rodata_E74);
    Hu3DModelLayerSet(lbl_1_bss_14C4, 2);
    Hu3DParticleHookSet(lbl_1_bss_14C4, fn_1_20DAC);
    Hu3DParticleBlendModeSet(lbl_1_bss_14C4, 1);
}

void fn_1_22080(void)
{
    s16 i;

    for (i = 0; i < 9; i++) {
        lbl_1_bss_14B0[i] = Hu3DParticleCreate(lbl_1_bss_14C8[0], 64);
        Hu3DModelPosSet(lbl_1_bss_14B0[i], lbl_1_rodata_E70,
            lbl_1_rodata_E70, lbl_1_rodata_E70);
        Hu3DModelScaleSet(lbl_1_bss_14B0[i], lbl_1_rodata_E74,
            lbl_1_rodata_E74, lbl_1_rodata_E74);
        Hu3DModelLayerSet(lbl_1_bss_14B0[i], 2);
        Hu3DModelAttrSet(lbl_1_bss_14B0[i], HU3D_ATTR_DISPOFF);
        Hu3DParticleHookSet(lbl_1_bss_14B0[i], fn_1_21AD0);
        Hu3DParticleBlendModeSet(lbl_1_bss_14B0[i], 1);
    }
}

void fn_1_22A4C(void)
{
    s16 i;
    s16 j;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            lbl_1_bss_1490[i][j] = Hu3DParticleCreate(lbl_1_bss_14C8[j], 16);
            Hu3DModelPosSet(lbl_1_bss_1490[i][j], lbl_1_rodata_E70,
                lbl_1_rodata_E70, lbl_1_rodata_E70);
            Hu3DModelScaleSet(lbl_1_bss_1490[i][j], lbl_1_rodata_E74,
                lbl_1_rodata_E74, lbl_1_rodata_E74);
            Hu3DModelLayerSet(lbl_1_bss_1490[i][j], 2);
            Hu3DModelAttrSet(lbl_1_bss_1490[i][j], HU3D_ATTR_DISPOFF);
            Hu3DParticleHookSet(lbl_1_bss_1490[i][j], fn_1_22348);
            Hu3DParticleBlendModeSet(lbl_1_bss_1490[i][j], 1);
        }
    }
}

void fn_1_24AD0(void)
{
    lbl_1_bss_131A = Hu3DParticleCreate(lbl_1_bss_14C8[0], 100);
    Hu3DModelPosSet(lbl_1_bss_131A, lbl_1_rodata_E70,
        lbl_1_rodata_E70, lbl_1_rodata_E70);
    Hu3DModelScaleSet(lbl_1_bss_131A, lbl_1_rodata_E74,
        lbl_1_rodata_E74, lbl_1_rodata_E74);
    Hu3DModelLayerSet(lbl_1_bss_131A, 2);
    Hu3DParticleHookSet(lbl_1_bss_131A, fn_1_24554);
    Hu3DParticleBlendModeSet(lbl_1_bss_131A, 1);
}

void fn_1_251D4(void)
{
    lbl_1_bss_1318 = Hu3DParticleCreate(lbl_1_bss_14C8[6], 32);
    Hu3DModelPosSet(lbl_1_bss_1318, lbl_1_rodata_E70,
        lbl_1_rodata_E70, lbl_1_rodata_E70);
    Hu3DModelScaleSet(lbl_1_bss_1318, lbl_1_rodata_E74,
        lbl_1_rodata_E74, lbl_1_rodata_E74);
    Hu3DModelLayerSet(lbl_1_bss_1318, 2);
    Hu3DModelAttrSet(lbl_1_bss_1318, HU3D_ATTR_DISPOFF);
    Hu3DParticleHookSet(lbl_1_bss_1318, fn_1_24C58);
    Hu3DParticleBlendModeSet(lbl_1_bss_1318, 1);
}

void fn_1_18E14(void)
{
    MDRESULT_PLAYER_WORK *work;
    s16 i;

    i = 0;
    work = lbl_1_bss_66C;
    for (; i < 4; i++, work++) {
        Hu3DModelAttrSet(work->models[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(work->models[1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(work->models[2], HU3D_ATTR_DISPOFF);
        fn_1_20108(work->group, HUSPR_ATTR_DISPOFF);
    }
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrSet(lbl_1_bss_C->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    Hu3DModelAttrSet(lbl_1_bss_4->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_8->mdlId[0], HU3D_ATTR_DISPOFF);
}

void fn_1_1A468(void)
{
    MDRESULT_PLAYER_WORK *work;
    s16 i;

    i = 0;
    work = lbl_1_bss_66C;
    for (; i < 2; i++, work++) {
        Hu3DModelAttrSet(work->models[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(work->models[1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(work->models[2], HU3D_ATTR_DISPOFF);
        fn_1_20108(work->group, HUSPR_ATTR_DISPOFF);
        fn_1_20108(work->secondGroup, HUSPR_ATTR_DISPOFF);
        HuWinDispOff(work->winId);
    }
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrSet(lbl_1_bss_C->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    Hu3DModelAttrSet(lbl_1_bss_4->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_8->mdlId[0], HU3D_ATTR_DISPOFF);
}

void fn_1_22244(s16 index, HuVecF *position)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    s16 i;
    s16 j;

    for (i = 0; i < 4; i++) {
        model = &Hu3DData[lbl_1_bss_1490[index][i]];
        particle = model->hookData;
        j = 0;
        data = particle->data;
        for (; j < particle->maxCnt; j++, data++) {
            data->time = 0;
            data->parManId = 0;
            data->scale = lbl_1_rodata_E70;
        }
        model->attr &= ~1;
        Hu3DModelPosSetV(lbl_1_bss_1490[index][i], position);
    }
}

void fn_1_2668C(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    MDRESULT_TRAIL_WORK *trail;
    u8 attrs[15];
    u8 unused[4];
    s16 i;
    s16 j;

    attrs[0] = lbl_1_rodata_FC8[0];
    attrs[1] = lbl_1_rodata_FC8[1];
    attrs[2] = lbl_1_rodata_FC8[2];
    attrs[3] = lbl_1_rodata_FC8[3];
    attrs[4] = lbl_1_rodata_FC8[4];
    attrs[5] = lbl_1_rodata_FC8[5];
    attrs[6] = lbl_1_rodata_FC8[6];
    attrs[7] = lbl_1_rodata_FC8[7];
    attrs[8] = lbl_1_rodata_FC8[8];
    attrs[9] = lbl_1_rodata_FC8[9];
    attrs[10] = lbl_1_rodata_FC8[10];
    attrs[11] = lbl_1_rodata_FC8[11];
    attrs[12] = lbl_1_rodata_FC8[12];
    attrs[13] = lbl_1_rodata_FC8[13];
    attrs[14] = lbl_1_rodata_FC8[14];
    unused[0] = lbl_1_rodata_FD7[0];
    unused[1] = lbl_1_rodata_FD7[1];
    unused[2] = lbl_1_rodata_FD7[2];
    unused[3] = lbl_1_rodata_FD7[3];

    model = &Hu3DData[lbl_1_bss_14B0[index]];
    particle = model->hookData;
    data = particle->data;
    for (i = 0; i < particle->maxCnt; i++, data++) {
        data->time = 0;
        data->parManId = 0;
        data->scale = lbl_1_rodata_E70;
    }
    data = particle->data;
    data->time = 1;
    if (parManId > 0) {
        data->parManId = parManId;
    }
    if (velocity) {
        data->vel.x = velocity->x;
        data->vel.y = velocity->y;
        data->vel.z = velocity->z;
    }
    if (accelX > lbl_1_rodata_E70) {
        data->accel.x = accelX;
    }
    if (color) {
        data->color.r = color[0];
        data->color.g = color[1];
        data->color.b = color[2];
        data->color.a = 0;
    }
    data->scale = lbl_1_rodata_E70;
    model->attr &= ~HU3D_ATTR_DISPOFF;

    trail = &lbl_1_bss_1320[index];
    trail->state = 1;
    trail->color.r = color[0];
    trail->color.g = color[1];
    trail->color.b = color[2];
    trail->color.a = 0;
    trail->unk_28 = 0;
    for (j = 0; j < trail->pointCount; j++) {
        trail->points[j].x = velocity->x;
        trail->points[j].y = velocity->y;
        trail->points[j].z = velocity->z;
        trail->points[j].y -= lbl_1_rodata_F80 * j;
    }
    Hu3DModelAttrReset(lbl_1_bss_1480[index], HU3D_ATTR_DISPOFF);

    model = &Hu3DData[lbl_1_bss_131A];
    particle = model->hookData;
    for (j = 0; j < 5; j++) {
        data = &particle->data[(s16)(j + (index * 5))];
        data->time = 1;
        if (attrs[(j * 3) + 2] == 1) {
            data->time = 3;
        }
        data->pos.x = velocity->x;
        data->pos.y = velocity->y;
        data->pos.z = velocity->z;
        data->scale = lbl_1_rodata_E70;
        data->vel.x = lbl_1_rodata_E70;
        data->vel.y = lbl_1_rodata_E74;
        data->vel.z = lbl_1_rodata_F64;
        data->color.r = color[0];
        data->color.g = color[1];
        data->color.b = color[2];
        color[3] = attrs[(j * 3) + 1];
        data->color.a = color[3];
        data->speedDecay = (float)color[0];
        data->colorIdx = (float)color[1];
        data->scaleBase = (float)color[2];
        data->accel.x = (float)attrs[j * 3];
        data->accel.y = (float)color[3];
    }
}

void fn_1_26BE4(s16 index)
{
    HU3D_PARTICLE_DATA *burstData;
    HU3D_PARTICLE *burstParticle;
    HU3D_MODEL *burstModel;
    MDRESULT_TRAIL_WORK *work;
    s16 i;
    HU3D_PARTICLE_DATA *trailData;
    HU3D_PARTICLE *trailParticle;
    HU3D_MODEL *trailModel;

    burstModel = &Hu3DData[lbl_1_bss_14B0[index]];
    burstParticle = burstModel->hookData;
    burstData = burstParticle->data;
    burstData->time = 0;
    work = &lbl_1_bss_1320[index];
    work->state = 0;
    Hu3DModelAttrSet(lbl_1_bss_1480[index], HU3D_ATTR_DISPOFF);
    for (i = 0; i < 5; i++) {
        trailModel = &Hu3DData[lbl_1_bss_131A];
        trailParticle = trailModel->hookData;
        trailData = &trailParticle->data[(s16)(i + (index * 5))];
        trailData->time = 2;
        trailData->vel.z = lbl_1_rodata_E74;
    }
}

void fn_1_20108(HUSPR_GROUPID groupId, s32 attr)
{
    HUSPR_GROUP *group = &HuSprGrpData[groupId];
    s16 i;

    for (i = 0; i < group->sprNum; i++) {
        HuSprAttrSet(groupId, i, (u16)attr);
    }
}

void fn_1_204B0(float value)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14C6];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    Hu3DModelLayerSet(lbl_1_bss_14C6, 1);
    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        data->colorIdx = value;
    }
}

void fn_1_20D0C(s16 index, HuVecF *position, float alpha)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14C4];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = &particle->data[index];
    float opacity;

    data->vel.x = position->x;
    data->vel.y = position->y;
    data->vel.z = position->z;
    opacity = lbl_1_rodata_EF8 * alpha;
    data->color.a = opacity;
}

void fn_1_20FAC(void)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14C2];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    Hu3DModelAttrReset(lbl_1_bss_14C2, HU3D_ATTR_DISPOFF);
    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        data->time = 0;
    }
}

void fn_1_21604(void)
{
    lbl_1_bss_14C2 = Hu3DParticleCreate(lbl_1_bss_14C8[5], 128);
    Hu3DModelPosSet(lbl_1_bss_14C2, lbl_1_rodata_E70,
        lbl_1_rodata_E70, lbl_1_rodata_E70);
    Hu3DModelScaleSet(lbl_1_bss_14C2, lbl_1_rodata_E74,
        lbl_1_rodata_E74, lbl_1_rodata_E74);
    Hu3DModelLayerSet(lbl_1_bss_14C2, 1);
    Hu3DModelAttrSet(lbl_1_bss_14C2, HU3D_ATTR_DISPOFF);
    Hu3DParticleHookSet(lbl_1_bss_14C2, fn_1_2104C);
}

void fn_1_21A70(s16 index)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14B0[index]];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = particle->data;

    data->time = 0;
}

void fn_1_23C88(void)
{
    HU3D_MODEL *model;
    s16 i;

    for (i = 0; i < 8; i++) {
        HuMemDirectFree(lbl_1_bss_1320[i].points);
        model = &Hu3DData[lbl_1_bss_1480[i]];
        model->hookData = NULL;
        Hu3DModelKill(lbl_1_bss_1480[i]);
    }
}

void fn_1_25B90(void)
{
    fn_1_20CE0();
    fn_1_20F80();
    fn_1_221EC();
    fn_1_22C38();
    fn_1_216E8();
    fn_1_23C88();
    fn_1_24BB4();
}

void fn_1_24308(s16 index, float value)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_131A];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = &particle->data[index];

    data->time = 2;
    data->vel.z = value;
}

void fn_1_2436C(s16 index, HuVecF *position)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_131A];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *data = &particle->data[index];

    data->pos.x = position->x;
    data->pos.y = position->y;
    data->pos.z = position->z;
}

void fn_1_BACC(void)
{
    s16 i;

    for (i = 0; i < 9; i++) {
        if (lbl_1_bss_81C[i].data) {
            HuMemDirectFree(lbl_1_bss_81C[i].data);
        }
        lbl_1_bss_81C[i].data = NULL;
    }
}

void fn_1_25D0C(float value)
{
    HU3D_MODEL *model = &Hu3DData[lbl_1_bss_14C6];
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle = model->hookData;
    s16 i;

    Hu3DModelLayerSet(lbl_1_bss_14C6, 1);
    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        data->colorIdx = value;
    }
}

void fn_1_25DB0(s16 index, HuVecF *position, float alpha)
{
    HU3D_PARTICLE *particle;
    HU3D_MODEL *model;
    HU3D_PARTICLE_DATA *data;
    float opacity;

    if (index < 0 || index > 3) {
        return;
    }
    model = &Hu3DData[lbl_1_bss_14C4];
    particle = model->hookData;
    data = &particle->data[index];
    data->vel.x = position->x;
    data->vel.y = position->y;
    data->vel.z = position->z;
    opacity = lbl_1_rodata_EF8 * alpha;
    data->color.a = opacity;
}

void fn_1_25FF4(s16 index)
{
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle;
    HU3D_MODEL *model;

    if (index < 0 || index > 8) {
        return;
    }
    model = &Hu3DData[lbl_1_bss_14B0[index]];
    particle = model->hookData;
    data = particle->data;
    data->time = 0;
}

void fn_1_26EB0(HuVecF *position)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE_DATA *data;
    HU3D_PARTICLE *particle;
    s16 i;

    fn_1_23EF0(position);
    model = &Hu3DData[lbl_1_bss_14C2];
    particle = model->hookData;
    Hu3DModelAttrReset(lbl_1_bss_14C2, HU3D_ATTR_DISPOFF);
    i = 0;
    data = particle->data;
    for (; i < particle->maxCnt; i++, data++) {
        data->time = 0;
    }
    Hu3DModelPosSetV(lbl_1_bss_14C2, position);
    Hu3DModelAttrReset(lbl_1_bss_14C2, HU3D_ATTR_DISPOFF);
}

void fn_1_26F74(void)
{
    Hu3DModelAttrSet(lbl_1_bss_14C2, HU3D_ATTR_DISPOFF);
}

void fn_1_10098(void)
{
    HuDataDirCloseAll();
}

void fn_1_26EAC(float value)
{
}

const MDRESULT_PLAYER_SPRITE_TABLE lbl_1_rodata_6EC = {
    {
        { 0, 10, 10, { 40.0f, 22.0f }, { 1.0f, 1.0f }, 0.0f },
        { 0, 10, 0, { 60.0f, 22.0f }, { 1.0f, 1.0f }, 0.0f },
        { 0, 10, 0, { 80.0f, 22.0f }, { 1.0f, 1.0f }, 0.0f },
        { 0, 10, 10, { 40.0f, 50.0f }, { 1.0f, 1.0f }, 0.0f },
        { 0, 10, 0, { 60.0f, 50.0f }, { 1.0f, 1.0f }, 0.0f },
        { 0, 10, 0, { 80.0f, 50.0f }, { 1.0f, 1.0f }, 0.0f },
        { 1, 10, 0, { 56.0f, -30.0f }, { 1.0f, 1.0f }, 0.0f },
        { 2, 40, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
        { 3, 40, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
        { 4, 40, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
        { 5, 40, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
        { 29, 20, 0, { 0.0f, 74.0f }, { 0.7f, 0.7f }, 0.0f },
        { 31, 30, 0, { -90.0f, 20.0f }, { -0.7f, 0.6f }, 0.0f },
        { 31, 30, 0, { 90.0f, 20.0f }, { 0.7f, 0.6f }, 0.0f },
    },
};
const MDRESULT_MESSAGE_NUMBERS lbl_1_rodata_3C = {
    { 917504, -1 },
};

const MDRESULT_FX_NUMBERS lbl_1_rodata_44 = {
    {
        949, 950, 951, 952, 953, 954, 955, -1,
        941, 942, 943, 944, 945, 946, 947, -1,
    },
};

const float lbl_1_rodata_F4 = 30.0f;
const float lbl_1_rodata_F8 = 10.0f;
const float lbl_1_rodata_FC = 10000.0f;
const float lbl_1_rodata_100 = 1.2f;
const float lbl_1_rodata_104 = 0.0f;
const float lbl_1_rodata_108 = 640.0f;
const float lbl_1_rodata_10C = 480.0f;
const float lbl_1_rodata_110 = 1.0f;
const float lbl_1_rodata_114 = 65.0f;
const float lbl_1_rodata_118 = -800.0f;
const float lbl_1_rodata_11C = -7.25f;
const float lbl_1_rodata_120 = 2650.0f;

const MDRESULT_VECTOR_PAIR lbl_1_rodata_124 = {
    {
        { 0.0f, 1.0f, 1.0f },
        { -1.0f, 1.0f, -1.0f },
    },
};

const MDRESULT_VECTOR_PAIR lbl_1_rodata_13C = {
    {
        { 0.0f, -1.0f, -1.0f },
        { 1.0f, -1.0f, -1.0f },
    },
};

const GXColor lbl_1_rodata_154 = { 255, 255, 255, 128 };
const float lbl_1_rodata_158 = 16.0f;
const float lbl_1_rodata_15C = 337.0f;
const float lbl_1_rodata_160 = 372.0f;
const float lbl_1_rodata_164 = 0.9f;
const float lbl_1_rodata_168 = 412.0f;
const HuVecF lbl_1_rodata_16C = { 0.0f, 3000.0f, 600.0f };
const HuVecF lbl_1_rodata_178 = { 0.0f, 1.0f, 0.0f };
const HuVecF lbl_1_rodata_184 = { 0.0f, 0.0f, 0.0f };

const float lbl_1_rodata_250 = 0.4f;
const float lbl_1_rodata_254 = 3.0f;
const float lbl_1_rodata_258 = 0.5f;
const float lbl_1_rodata_260 = 15.0f;
const float lbl_1_rodata_298 = 2.0f;
const float lbl_1_rodata_29C = -20.0f;
const float lbl_1_rodata_2C4 = -325.0f;
const float lbl_1_rodata_2C8 = 1.25f;
const float lbl_1_rodata_3B8 = 1.5f;
const MDRESULT_VECTOR_PAIR lbl_1_rodata_3D0 = {
    {
        { -180.0f, 350.0f, 0.0f },
        { 180.0f, 350.0f, 0.0f },
    },
};
const float lbl_1_rodata_3E8 = 55.0f;
const float lbl_1_rodata_404 = -2000.0f;
const float lbl_1_rodata_408 = -0.04f;
const float lbl_1_rodata_40C = -50.0f;
const float lbl_1_rodata_410 = -40.0f;
const float lbl_1_rodata_414 = 6.0f;
const float lbl_1_rodata_E5C = 0.001f;
const float lbl_1_rodata_E70 = 0.0f;
const float lbl_1_rodata_E74 = 1.0f;
const float lbl_1_rodata_E78 = 2.0f;

#include "dolphin/gx/GXStruct.h"

#include "dolphin/mtx/GeoTypes.h"

#include "stddef.h"

#include "humath.h"

void fn_1_16C4(MDRESULT_CAMERA_WORK *camera)
{
    memcpy(&camera->center, &camera->targetCenter, sizeof(HuVecF));
    memcpy(&camera->rot, &camera->targetRot, sizeof(HuVecF));
    camera->zoom = camera->targetZoom;
}

void fn_1_1714(MDRESULT_CAMERA_WORK *camera)
{
    memcpy(&camera->targetCenter, &camera->center, sizeof(HuVecF));
    memcpy(&camera->targetRot, &camera->rot, sizeof(HuVecF));
    camera->targetZoom = camera->zoom;
}

void fn_1_1764(MDRESULT_CAMERA_WORK *camera, float weight)
{
    fn_1_1FB50(&camera->center, &camera->targetCenter, weight);
    fn_1_1FB50(&camera->rot, &camera->targetRot, weight);
    camera->zoom = fn_1_1F8BC(camera->zoom, camera->targetZoom, weight);
}

void fn_1_17D4(MDRESULT_CAMERA_CALLBACK callback)
{
    MDRESULT_CAMERA_WORK *camera = &lbl_1_bss_12BC;

    camera->callback = callback;
}

void fn_1_17F4(OMOBJ *obj, MDRESULT_CAMERA_WORK *camera)
{
    if (camera->callback) {
        camera->callback(obj, camera);
    }
}

void fn_1_1840(s16 mode)
{
    MDRESULT_CAMERA_WORK *camera = &lbl_1_bss_12BC;

    camera->mode = mode;
}

void fn_1_1860(OMOBJ *obj)
{
    MDRESULT_CAMERA_WORK *camera = &lbl_1_bss_12BC;

    fn_1_17F4(obj, camera);
    Center.x = camera->center.x;
    Center.y = camera->center.y;
    Center.z = camera->center.z;
    CRot.x = camera->rot.x;
    CRot.y = camera->rot.y;
    CRot.z = camera->rot.z;
    CZoom = camera->zoom;
    omOutView(obj);
}

void fn_1_1930(MDRESULT_CAMERA_CALLBACK callback)
{
    MDRESULT_CAMERA_WORK *camera = &lbl_1_bss_12BC;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_F4, lbl_1_rodata_F8,
        lbl_1_rodata_FC, lbl_1_rodata_100);
    Hu3DCameraViewportSet(1, lbl_1_rodata_104, lbl_1_rodata_104,
        lbl_1_rodata_108, lbl_1_rodata_10C, lbl_1_rodata_104,
        lbl_1_rodata_110);
    memset(camera, 0, sizeof(MDRESULT_CAMERA_WORK));
    camera->callback = callback;
    camera->center.x = lbl_1_rodata_104;
    camera->center.y = lbl_1_rodata_114;
    camera->center.z = lbl_1_rodata_118;
    camera->rot.x = lbl_1_rodata_11C;
    camera->rot.y = lbl_1_rodata_104;
    camera->rot.z = lbl_1_rodata_104;
    camera->zoom = lbl_1_rodata_120;
    camera->obj = omAddObjEx(lbl_1_bss_0, 256, 0, 0, -1, fn_1_1860);
}

void fn_1_1AA4(void)
{
    MDRESULT_CAMERA_WORK *camera = &lbl_1_bss_12BC;

    Hu3DCameraKill(1);
    if (camera->obj) {
        omDelObjEx(lbl_1_bss_0, camera->obj);
    }
    camera->obj = NULL;
}

void fn_1_1B00(void)
{
    MDRESULT_VECTOR_PAIR pos = lbl_1_rodata_124;
    MDRESULT_VECTOR_PAIR dir = lbl_1_rodata_13C;
    GXColor color = lbl_1_rodata_154;

    lbl_1_bss_130E[0] =
        Hu3DGLightCreateV(&pos.values[0], &dir.values[0], &color);
    Hu3DGLightInfinitytSet(lbl_1_bss_130E[0]);
    Hu3DGLightStaticSet(lbl_1_bss_130E[0], TRUE);
    lbl_1_bss_130E[1] =
        Hu3DGLightCreateV(&pos.values[1], &dir.values[1], &color);
    Hu3DGLightInfinitytSet(lbl_1_bss_130E[1]);
    Hu3DGLightStaticSet(lbl_1_bss_130E[1], TRUE);
}

void fn_1_1C34(void)
{
    Hu3DGLightKill(lbl_1_bss_130E[0]);
    Hu3DGLightKill(lbl_1_bss_130E[1]);
}

void fn_1_2BF0(void)
{
    Vec shadowPos = lbl_1_rodata_16C;
    Vec shadowUp = lbl_1_rodata_178;
    Vec shadowTarget = lbl_1_rodata_184;

    Hu3DShadowCreate(
        lbl_1_rodata_F4, lbl_1_rodata_F8, lbl_1_rodata_FC);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
}

void fn_1_2CA4(void)
{
}

void fn_1_2CA8(void)
{
    MDRESULT_SPRITE_INFO *desc;
    s16 i;

    for (i = 0; i < 39; i++) {
        lbl_1_bss_11AC[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(
                lbl_1_data_C0[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 6; i++) {
        lbl_1_bss_11A0[i] = HuSprGrpCreate(lbl_1_data_15C[i]);
    }
    for (i = 0, desc = lbl_1_data_168; i < 18; i++, desc++) {
        lbl_1_bss_117C[i] = HuSprCreate(
            lbl_1_bss_11AC[desc->animNo], desc->priority + 6000,
            desc->bank);
        HuSprGrpMemberSet(lbl_1_bss_11A0[desc->groupNo], desc->memberNo,
            lbl_1_bss_117C[i]);
        HuSprPosSet(lbl_1_bss_11A0[desc->groupNo], desc->memberNo,
            desc->pos.x, desc->pos.y);
        HuSprScaleSet(lbl_1_bss_11A0[desc->groupNo], desc->memberNo,
            desc->scale.x, desc->scale.y);
        HuSprZRotSet(lbl_1_bss_11A0[desc->groupNo], desc->memberNo,
            desc->zRot);
    }
    for (i = 0; i < 6; i++) {
        fn_1_20108(lbl_1_bss_11A0[i], HUSPR_ATTR_DISPOFF);
    }
    HuSprExecLayerSet(64, 2);
}

void fn_1_2ED0(void)
{
}

void fn_1_2ED4(s16 index)
{
    MDRESULT_VECTOR_TABLE positions = lbl_1_rodata_190;
    Vec world;

        Hu3D2Dto3D(&positions.values[index + (lbl_1_bss_1278.values[3] * 4)], 1,
        &world);
    lbl_1_bss_109C[0].x = world.x;
    lbl_1_bss_109C[0].y = world.y;
    lbl_1_bss_109C[0].z = world.z;
}

void fn_1_2F80(s16 index)
{
    OMOBJ *obj = lbl_1_bss_30;
    MDRESULT_VECTOR_TABLE positions = lbl_1_rodata_1F0;
    Vec world;

    fn_1_2001C(obj->mdlId[0],
        &positions.values[index + (lbl_1_bss_1278.values[3] * 4)], NULL);
    Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_104,
        lbl_1_rodata_104, lbl_1_rodata_104);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_250,
        lbl_1_rodata_250, lbl_1_rodata_250);
    Hu3DModelPosGet(obj->mdlId[0], &world);
    lbl_1_bss_109C[0].x = world.x;
    lbl_1_bss_109C[0].y = world.y;
    lbl_1_bss_109C[0].z = world.z;
    Hu3DModelAttrReset(obj->mdlId[0], HU3D_ATTR_DISPOFF);
}

void fn_1_30C4(void)
{
    OMOBJ *obj = lbl_1_bss_30;

    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
}

void fn_1_3104(OMOBJ *obj)
{
    Vec transform;

    Hu3DModelPosGet(obj->mdlId[0], &transform);
    fn_1_1FB50(&transform, &lbl_1_bss_109C[0], lbl_1_rodata_254);
    Hu3DModelPosSetV(obj->mdlId[0], &transform);
    Hu3DModelRotGet(obj->mdlId[0], &transform);
    transform.z = fn_1_1F8BC(
        transform.z, lbl_1_rodata_104, lbl_1_rodata_254);
    Hu3DModelRotSetV(obj->mdlId[0], &transform);
    Hu3DModelScaleGet(obj->mdlId[0], &transform);
    transform.x = transform.y = transform.z = fn_1_1F8BC(
        transform.x, lbl_1_rodata_250, lbl_1_rodata_254);
    Hu3DModelScaleSetV(obj->mdlId[0], &transform);
}

void fn_1_31F8(OMOBJ *obj)
{
    OMOBJ *activeObj;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 96), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[0] = Hu3DMotionIDGet(obj->mdlId[0]);
    Hu3DModelLayerSet(obj->mdlId[0], 3);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
        lbl_1_rodata_104, lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_258,
        lbl_1_rodata_258, lbl_1_rodata_258);
    activeObj = lbl_1_bss_30;
    Hu3DModelAttrSet(activeObj->mdlId[0], HU3D_ATTR_DISPOFF);
    obj->objFunc = fn_1_3104;
}

void fn_1_3304(OMOBJ *obj)
{
    if (obj) {
        Hu3DMotionKill(obj->mtnId[0]);
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_4694(OMOBJ *obj)
{
    MDRESULT_CHARACTER_WORK *characterWork;
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    i = 0;
    characterWork = &lbl_1_bss_1248[i];
    for (; i < 4; i++, characterWork++) {
        obj->mdlId[i] = CharModelCreate(characterWork->character, 2);
        obj->mtnId[i] = CharMotionCreate(characterWork->character, 9633792);
        obj->mtnId[i + 4] = CharMotionCreate(characterWork->character, 9633803);
        obj->mtnId[i + 8] = Hu3DJointMotion(obj->mdlId[i],
            HuDataSelHeapReadNum(characterWork->character + 9961488,
                HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 12] = Hu3DJointMotion(obj->mdlId[i],
            HuDataSelHeapReadNum(characterWork->character + 9961499,
                HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 16] = CharMotionCreate(characterWork->character, 9633826);
        obj->mtnId[i + 20] = CharMotionCreate(characterWork->character, 9633828);
        obj->mtnId[i + 24] = CharMotionCreate(characterWork->character, 9633829);
        obj->mtnId[i + 28] = CharMotionCreate(characterWork->character, 9633833);
        obj->mtnId[i + 32] = CharMotionCreate(characterWork->character, 9633879);
        obj->mtnId[i + 36] = CharMotionCreate(characterWork->character, 9633799);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i], lbl_1_rodata_104,
            lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
    }
    if (lbl_1_bss_1278.values[3] == 0) {
        for (i = 0; i < 4; i++) {
            Hu3DModelPosSetV(obj->mdlId[i], &lbl_1_data_0[i + 8]);
        }
    } else {
        for (i = 0; i < 4; i++) {
            Hu3DModelPosSetV(obj->mdlId[i], &lbl_1_data_0[i + 12]);
        }
    }
    obj->objFunc = NULL;
}

void fn_1_49C8(OMOBJ *obj)
{
    s16 i;
    s16 j;

    if (obj) {
        CharModelKill(-1);
        for (i = 0; i < 4; i++) {
            Hu3DMotionKill(obj->mtnId[i + 8]);
            Hu3DMotionKill(obj->mtnId[i + 12]);
            obj->mdlId[i] = -1;
            for (j = 0; j < 8; j++) {
            }
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_4A9C(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_298);
    if (obj->work[3]++ > 30) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_110);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            lbl_1_rodata_104, lbl_1_rodata_260,
            HU3D_MOTATTR_LOOP);
    }
}

void fn_1_4B44(void)
{
    OMOBJ *obj = lbl_1_bss_4;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_4A9C;
}

void fn_1_4BB8(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_298);
    if (obj->work[3]++ > 30) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_110);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            lbl_1_rodata_104, lbl_1_rodata_260,
            HU3D_MOTATTR_LOOP);
    }
}

void fn_1_4C60(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_4BB8;
}

void fn_1_4CD4(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 38), HU_MEMNUM_OVL, HEAP_MODEL));
    for (i = 0; i < 5; i++) {
        obj->mtnId[i] = Hu3DJointMotion(obj->mdlId[0],
            HuDataSelHeapReadNum(DATANUM(DATA_mdpresult, 39) + i,
                HU_MEMNUM_OVL, HEAP_MODEL));
    }
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
        lbl_1_rodata_104, lbl_1_rodata_104,
        HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(obj->mdlId[0]);
    Hu3DModelPosSet(obj->mdlId[0], lbl_1_rodata_2C4,
        lbl_1_rodata_104, lbl_1_rodata_284);
    Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_104,
        lbl_1_rodata_260, lbl_1_rodata_104);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_2C8,
        lbl_1_rodata_2C8, lbl_1_rodata_2C8);
    obj->objFunc = NULL;
}

void fn_1_4E68(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 2; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_4EF0(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 44), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 45), HU_MEMNUM_OVL, HEAP_MODEL));
    for (i = 0; i < 5; i++) {
        obj->mtnId[i] = Hu3DJointMotion(obj->mdlId[0],
            HuDataSelHeapReadNum(DATANUM(DATA_mdpresult, 46) + i,
                HU_MEMNUM_OVL, HEAP_MODEL));
    }
    Hu3DModelHookSet(obj->mdlId[0], lbl_1_data_666, obj->mdlId[1]);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_rodata_104,
        lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(obj->mdlId[0]);
    Hu3DModelPosSet(obj->mdlId[0], lbl_1_rodata_2CC,
        lbl_1_rodata_104, lbl_1_rodata_284);
    Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_104, lbl_1_rodata_2D0,
        lbl_1_rodata_104);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_2C8,
        lbl_1_rodata_2C8, lbl_1_rodata_2C8);
    obj->objFunc = NULL;
}

void fn_1_50C0(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        Hu3DModelHookReset(obj->mdlId[1]);
        for (i = 0; i < 2; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        Hu3DModelKill(obj->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_5160(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 2; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 0) + i, HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_104, lbl_1_rodata_104,
            HU3D_MOTATTR_LOOP);
        Hu3DModelShadowMapSet(obj->mdlId[i]);
    }
    Hu3DModelPosSet(obj->mdlId[1], lbl_1_rodata_104, lbl_1_rodata_2D4,
        lbl_1_rodata_104);
    obj->work[1] = Hu3DTexScrollCreate(obj->mdlId[1], lbl_1_data_678);
    obj->objFunc = NULL;
}

void fn_1_52C4(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        Hu3DTexScrollKill(obj->work[1]);
        for (i = 0; i < 2; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}






void fn_1_A984(void)
{
    OMOBJ *obj = lbl_1_bss_1C;
    MDRESULT_MOVE_WORK *work = &lbl_1_bss_8EC[obj->work[3]];

    work->state = 0;
    work->time = lbl_1_rodata_104;
    work->duration = lbl_1_rodata_380;
    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &work->current);
    fn_1_1F868(&work->middle, lbl_1_rodata_104, lbl_1_rodata_104,
        lbl_1_rodata_3CC);
    fn_1_1F868(&work->target, lbl_1_rodata_104, lbl_1_rodata_2C0,
        lbl_1_rodata_31C);
    HuAudFXPlay(1174);
    obj->objFunc = fn_1_A85C;
}

void fn_1_AA7C(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 3; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 58) + i, HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i], lbl_1_rodata_104,
            lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(obj->mdlId[i], lbl_1_rodata_3B8,
            lbl_1_rodata_3B8, lbl_1_rodata_3B8);
    }
    for (i = 0; i < 4; i++) {
        obj->mdlId[i + 3] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 61), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 3] = Hu3DMotionIDGet(obj->mdlId[i + 3]);
        Hu3DModelLayerSet(obj->mdlId[i + 3], 1);
        Hu3DMotionShiftSet(obj->mdlId[i + 3], obj->mtnId[i + 3],
            lbl_1_rodata_104, lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i + 3], HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(obj->mdlId[i + 3], lbl_1_rodata_258,
            lbl_1_rodata_258, lbl_1_rodata_258);
    }
    obj->objFunc = NULL;
}

void fn_1_AD04(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 7; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_AD94(s16 player, s16 step)
{
    OMOBJ *obj = lbl_1_bss_24;
    MDRESULT_VECTOR_PAIR positions;
    s16 phase;
    s16 model;

    positions = lbl_1_rodata_3D0;
    step += 2;
    phase = step / 10;
    model = step % 10;
    if (phase >= 1) {
        Hu3DModelPosSet(obj->mdlId[(player * 11) + 10],
            positions.values[player].x - lbl_1_rodata_3E8,
            positions.values[player].y, positions.values[player].z);
        Hu3DModelAttrReset(obj->mdlId[(player * 11) + 10],
            HU3D_ATTR_DISPOFF);
        Hu3DModelPosSet(obj->mdlId[(player * 11) + model],
            lbl_1_rodata_3E8 + positions.values[player].x,
            positions.values[player].y, positions.values[player].z);
        Hu3DModelAttrReset(obj->mdlId[(player * 11) + model],
            HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelPosSet(obj->mdlId[(player * 11) + model],
            positions.values[player].x, positions.values[player].y,
            positions.values[player].z);
        Hu3DModelAttrReset(obj->mdlId[(player * 11) + model],
            HU3D_ATTR_DISPOFF);
    }
}

void fn_1_AFF4(void)
{
    OMOBJ *obj = lbl_1_bss_24;
    s16 i;

    for (i = 0; i < 22; i++) {
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_B05C(OMOBJ *obj)
{
    s16 i;
    s16 j;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 11; j++) {
            if (j == 10) {
                obj->mdlId[(i * 11) + j] = Hu3DModelCreate(HuDataSelHeapReadNum(
                    DATANUM(DATA_mdpresult, 65), HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                obj->mdlId[(i * 11) + j] = Hu3DModelCreate(HuDataSelHeapReadNum(
                    DATANUM(DATA_mdpresult, 64) + j, HU_MEMNUM_OVL, HEAP_MODEL));
            }
            Hu3DModelAttrSet(obj->mdlId[(i * 11) + j], HU3D_ATTR_DISPOFF);
        }
    }
    obj->objFunc = NULL;
}

void fn_1_B178(OMOBJ *obj)
{
    s16 j;
    s16 i;

    if (obj) {
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 11; j++) {
                Hu3DModelKill(obj->mdlId[j + (i * 11)]);
            }
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_B220(void)
{
    s16 shuffled[9];
    s16 values[9];
    s16 i;
    s16 pick;

    for (i = 0; i < 9; i++) {
        values[i] = i;
    }
    for (i = 0; i < 9; i++) {
        pick = rand8() % (9 - i);
        shuffled[i] = values[pick];
        values[pick] = values[8 - i];
    }
    for (i = 0; i < 4; i++) {
        lbl_1_bss_8AC[i].score = shuffled[i];
        OSReport(lbl_1_data_67D, lbl_1_bss_8AC[i].score);
    }
    OSReport(lbl_1_data_682);
    if (lbl_1_bss_1278.values[3] == 1
        && lbl_1_bss_8AC[0].score + lbl_1_bss_8AC[1].score
            == lbl_1_bss_8AC[2].score + lbl_1_bss_8AC[3].score) {
        if (rand8() % 2 == 0) {
            lbl_1_bss_8AC[0].score = 7;
            lbl_1_bss_8AC[1].score = 8;
            lbl_1_bss_8AC[2].score = 5;
            lbl_1_bss_8AC[3].score = 4;
        } else {
            lbl_1_bss_8AC[0].score = 4;
            lbl_1_bss_8AC[1].score = 6;
            lbl_1_bss_8AC[2].score = 5;
            lbl_1_bss_8AC[3].score = 8;
        }
    }
}

void fn_1_B510(OMOBJ *obj)
{
    s16 i;
    s16 j;
    s16 k;

    for (i = 0; i < 9; i++) {
        MDRESULT_EMITTER_WORK *emitter = &lbl_1_bss_81C[i];

        if (emitter->active == 1) {
            HU3D_MODELID modelId = obj->mdlId[i + 4];
            HU3D_MODEL *model = &Hu3DData[modelId];
            HSF_DATA *hsf = model->hsf;
            HSF_OBJECT *object = hsf->object;
            HSF_BUFFER *vertexBuffer;
            MDRESULT_EMITTER_VERTEX *source;
            HuVecF *destination;
            HuVecF first;
            HuVecF second;
            HuVecF position;
            Mtx firstMatrix;
            Mtx secondMatrix;
            float amount;
            float angle;

            Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
            amount = fn_1_1FC94(lbl_1_rodata_104, lbl_1_rodata_110,
                emitter->timer, emitter->scale);
            angle = lbl_1_rodata_3EC * amount;
            if (angle > lbl_1_rodata_288) {
                angle = lbl_1_rodata_288;
            }
            PSMTXRotRad(firstMatrix, 'y', lbl_1_rodata_3F0 * angle);
            angle = (lbl_1_rodata_3EC * amount) - lbl_1_rodata_25C;
            if (angle < lbl_1_rodata_104) {
                angle = lbl_1_rodata_104;
            }
            PSMTXRotRad(secondMatrix, 'y', lbl_1_rodata_3F0 * angle);

            amount = fn_1_1FE74(lbl_1_rodata_3F4, lbl_1_rodata_2C0,
                emitter->timer, emitter->scale);
            Hu3DModelPosGet(modelId, &position);
            position.y = amount;
            Hu3DModelPosSetV(modelId, &position);

            emitter->timer += lbl_1_rodata_110;
            if (emitter->timer > emitter->scale) {
                emitter->active = 0;
            }

            for (j = 0; j < hsf->objectNum; j++) {
                if (object->type == HSF_OBJ_MESH) {
                    vertexBuffer = object->mesh.vertex;
                    source = emitter->data;
                    destination = vertexBuffer->data;
                    for (k = 0; k < vertexBuffer->count; k++, source++,
                        destination++) {
                        PSMTXMultVec(secondMatrix, &source->position, &second);
                        PSMTXMultVec(firstMatrix, &source->position, &first);
                        destination->x = second.x
                            + source->weight * (first.x - second.x);
                        destination->y = second.y
                            + source->weight * (first.y - second.y);
                        destination->z = second.z
                            + source->weight * (first.z - second.z);
                    }
                    DCStoreRangeNoSync(vertexBuffer->data,
                        vertexBuffer->count * sizeof(HuVecF));
                    break;
                }
            }
        }
    }
}

void fn_1_B8E8(OMOBJ *obj)
{
    s16 i;
    s16 j;
    s16 k;

    for (i = 0; i < 9; i++) {
        MDRESULT_EMITTER_WORK *emitter = &lbl_1_bss_81C[i];
        HU3D_MODELID modelId = obj->mdlId[i + 4];
        HSF_DATA *hsf = Hu3DData[modelId].hsf;
        HSF_OBJECT *object = hsf->object;
        float maxY = lbl_1_rodata_104;
        float minY = lbl_1_rodata_3F8;

        for (j = 0; j < hsf->objectNum; j++) {
            if (object->type == HSF_OBJ_MESH) {
                HSF_BUFFER *vertexBuffer = object->mesh.vertex;
                s16 count = vertexBuffer->count;
                HuVecF *source = vertexBuffer->data;
                MDRESULT_EMITTER_VERTEX *destination;
                MDRESULT_EMITTER_VERTEX *buffer;
                float range;

                for (k = 0; k < count; k++, source++) {
                    if (source->y > maxY) {
                        maxY = source->y;
                    }
                    if (source->y < minY) {
                        minY = source->y;
                    }
                }
                range = maxY - minY;
                buffer = destination = HuMemDirectMallocNum(HEAP_MODEL,
                    count * sizeof(MDRESULT_EMITTER_VERTEX), HU_MEMNUM_OVL);
                source = vertexBuffer->data;
                for (k = 0; k < count; k++, source++, destination++) {
                    destination->position = *source;
                    destination->weight = (source->y - minY) / range;
                }
                emitter->data = buffer;
                break;
            }
        }
    }
    fn_1_B220();
}

void fn_1_BB60(OMOBJ *obj)
{
    s16 i;
    s16 emitterIndex;
    float time;
    HuVecF position;
    MDRESULT_STATE_WORK *state;
    MDRESULT_CHARACTER_WORK *character;

    for (i = 0; i < 4; i++) {
        state = &lbl_1_bss_8AC[i];
        character = &lbl_1_bss_1248[i];
        switch (state->state) {
        case 1:
            Hu3DModelPosGet(lbl_1_bss_C->mdlId[i], &position);
            position.y = lbl_1_rodata_2CC;
            Hu3DModelPosSet(obj->mdlId[i], position.x, position.y,
                position.z);
            if (state->time == lbl_1_rodata_104) {
                fn_1_26164(i, &position);
            }
            time = fn_1_1FC94(lbl_1_rodata_104, lbl_1_rodata_110,
                state->time, state->delay);
            Hu3DModelScaleSet(obj->mdlId[i], time, time, time);
            Hu3DModelTPLvlSet(obj->mdlId[i], time);
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            state->time += lbl_1_rodata_110;
            if (state->time > state->delay) {
                HuAudFXPlay(1007);
                lbl_1_bss_12A0[i] = HuAudFXPlay(1005);
                state->state = 2;
                state->time = lbl_1_rodata_104;
                state->delay = (float)((rand8() % 120) + 60);
            }
            break;

        case 2:
            if (character->unk_04 == 0) {
                if ((HuPadBtnDown[character->unk_0A] & PAD_BUTTON_A) == 0) {
                    break;
                }
            } else {
                state->time += lbl_1_rodata_110;
                if (state->time <= state->delay) {
                    break;
                }
            }
            state->state = 3;
            state->time = lbl_1_rodata_104;
            state->delay = lbl_1_rodata_3FC;
            Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[i],
                lbl_1_bss_C->mtnId[i + 4], lbl_1_rodata_104,
                lbl_1_rodata_104, HU3D_MOTATTR_NONE);
            break;

        case 3:
            state->time += lbl_1_rodata_110;
            if (state->time <= state->delay) {
                break;
            }
            HuAudFXStop(lbl_1_bss_12A0[i]);
            HuAudFXPlay(1008);
            state->state = 4;
            state->time = lbl_1_rodata_104;
            state->delay = lbl_1_rodata_260;
            Hu3DModelPosGet(obj->mdlId[i], &position);
            Hu3DMotionSpeedSet(obj->mdlId[i], lbl_1_rodata_104);
            Hu3DMotionTimeSet(obj->mdlId[i], lbl_1_rodata_258
                + (float)state->score);
            emitterIndex = state->score;
            lbl_1_bss_81C[emitterIndex].active = 1;
            lbl_1_bss_81C[emitterIndex].timer = lbl_1_rodata_104;
            lbl_1_bss_81C[emitterIndex].scale = lbl_1_rodata_F4;
            Hu3DModelPosSetV(obj->mdlId[emitterIndex + 4], &position);
            break;

        case 4:
            time = fn_1_1FE74(lbl_1_rodata_104, lbl_1_rodata_110,
                state->time, state->delay);
            Hu3DModelPosGet(obj->mdlId[i], &position);
            position.y = lbl_1_rodata_2CC + lbl_1_rodata_2B8 * time;
            Hu3DModelPosSetV(obj->mdlId[i], &position);
            Hu3DModelScaleSet(obj->mdlId[i], lbl_1_rodata_110 + time,
                lbl_1_rodata_110 - lbl_1_rodata_258 * time,
                lbl_1_rodata_110 + time);
            state->time += lbl_1_rodata_110;
            if (state->time > state->delay) {
                state->state = 5;
                state->time = lbl_1_rodata_104;
                state->delay = lbl_1_rodata_F8;
            }
            break;

        case 5:
            time = fn_1_1FC94(lbl_1_rodata_110, lbl_1_rodata_104,
                state->time, state->delay);
            Hu3DModelTPLvlSet(obj->mdlId[i], time);
            state->time += lbl_1_rodata_110;
            if (state->time > state->delay) {
                state->state = 6;
                Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
                Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[i],
                    lbl_1_bss_C->mtnId[i], lbl_1_rodata_104,
                    lbl_1_rodata_260, HU3D_MOTATTR_LOOP);
            }
            break;
        }
    }
    fn_1_B510(obj);
}

void fn_1_B454(OMOBJ *obj, s16 index, HuVecF *pos)
{
    index--;
    lbl_1_bss_81C[index].active = 1;
    lbl_1_bss_81C[index].timer = lbl_1_rodata_104;
    lbl_1_bss_81C[index].scale = lbl_1_rodata_F4;
    Hu3DModelPosSetV(obj->mdlId[index + 4], pos);
}

s32 fn_1_C9A0(void)
{
    s16 bestScore = 0;
    s32 result = 0;
    s32 i;
    MDRESULT_STATE_WORK *work;

    for (;;) {
        HuPrcVSleep();
        i = 0;
        work = lbl_1_bss_8AC;
        for (; (s16)i < 4; i++, work++) {
            if (work->state != 0 && work->state != 6) {
                break;
            }
        }
        if ((s16)i == 4) {
            break;
        }
    }
    HuPrcSleep(60);
    if (lbl_1_bss_1278.values[3] == 1) {
        fn_1_C414();
        if (lbl_1_bss_8AC[0].score + lbl_1_bss_8AC[1].score
            > lbl_1_bss_8AC[2].score + lbl_1_bss_8AC[3].score) {
            result = 0;
        } else {
            result = 2;
        }
    } else {
        i = 0;
        work = lbl_1_bss_8AC;
        for (; (s16)i < 4; i++, work++) {
            if (work->state == 6 && bestScore < work->score) {
                bestScore = work->score;
                result = i;
            }
        }
    }
    return result;
}

void fn_1_CAEC(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, 256);
    for (i = 0; i < 4; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 63), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_104, lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < 9; i++) {
        obj->mdlId[i + 4] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 65) + i, HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 4] = Hu3DMotionIDGet(obj->mdlId[i + 4]);
        Hu3DModelLayerSet(obj->mdlId[i + 4], 1);
        Hu3DMotionSpeedSet(obj->mdlId[i + 4], lbl_1_rodata_104);
        Hu3DMotionTimeSet(obj->mdlId[i + 4], lbl_1_rodata_258);
        Hu3DModelAttrSet(obj->mdlId[i + 4], HU3D_ATTR_DISPOFF);
    }
    fn_1_B8E8(obj);
    obj->objFunc = NULL;
}

void fn_1_CD04(OMOBJ *obj)
{
    s16 i;
    s16 j;

    if (obj) {
        for (i = 0; i < 9; i++) {
            if (lbl_1_bss_81C[i].data) {
                HuMemDirectFree(lbl_1_bss_81C[i].data);
            }
            lbl_1_bss_81C[i].data = NULL;
        }
        for (j = 0; j < 13; j++) {
            Hu3DMotionKill(obj->mtnId[j]);
            Hu3DModelKill(obj->mdlId[j]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_CE0C(OMOBJ *obj)
{
    obj->objFunc = NULL;
}

void fn_1_CE18(OMOBJ *obj)
{
    if (obj) {
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_CE60(void)
{
    Hu3DModelAttrSet(lbl_1_bss_14->mdlId[1], HU3D_ATTR_DISPOFF);
    fn_1_26F74();
}

void fn_1_CE9C(void)
{
    OMOBJ *obj;
    HuVecF pos;
    s16 i;
    s16 j;
    MDRESULT_PARTICLE_WORK *work;
    MDRESULT_CAMERA_WORK *camera;

    obj = lbl_1_bss_10;
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    pos.y -= lbl_1_rodata_2B8;
    if (pos.y < lbl_1_rodata_404) {
        Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
        pos.y = lbl_1_rodata_404;
    }
    Hu3DModelPosSetV(obj->mdlId[0], &pos);
    Hu3DTexScrollPosMoveSet(obj->work[1], lbl_1_rodata_104,
        lbl_1_rodata_408, lbl_1_rodata_104);

    obj = lbl_1_bss_14;
    obj->work[0] = 1;
    for (i = 2; i < 5; i++) {
        Hu3DModelPosGet(obj->mdlId[i], &pos);
        pos.y -= lbl_1_rodata_2B8;
        if (pos.y < lbl_1_rodata_404) {
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            pos.y = lbl_1_rodata_404;
        }
        Hu3DModelPosSetV(obj->mdlId[i], &pos);
    }
    for (i = 0; i < 3; i++) {
        Hu3DModelPosGet(obj->mdlId[i + 8], &pos);
        pos.y -= lbl_1_rodata_2B8;
        if (pos.y < lbl_1_rodata_404) {
            Hu3DModelAttrSet(obj->mdlId[i + 8], HU3D_ATTR_DISPOFF);
            pos.y = lbl_1_rodata_404;
        }
        Hu3DModelPosSetV(obj->mdlId[i + 8], &pos);
    }

    obj = lbl_1_bss_28;
    for (i = 0; i < 4; i++) {
        Hu3DModelPosGet(obj->mdlId[i], &pos);
        pos.y -= lbl_1_rodata_2B8;
        if (pos.y < lbl_1_rodata_404) {
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            pos.y = lbl_1_rodata_404;
        }
        Hu3DModelPosSetV(obj->mdlId[i], &pos);
    }

    obj = lbl_1_bss_4;
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    pos.y -= lbl_1_rodata_2B8;
    if (pos.y < lbl_1_rodata_404) {
        pos.y = lbl_1_rodata_404;
    }
    Hu3DModelPosSetV(obj->mdlId[0], &pos);

    obj = lbl_1_bss_8;
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    pos.y -= lbl_1_rodata_2B8;
    if (pos.y < lbl_1_rodata_404) {
        pos.y = lbl_1_rodata_404;
    }
    Hu3DModelPosSetV(obj->mdlId[0], &pos);

    fn_1_26EAC(lbl_1_rodata_40C);
    lbl_1_bss_44 = lbl_1_rodata_40C;
    for (j = 0; j < 4; j++) {
        work = &lbl_1_bss_F9C[j];
        work->verticalOffset -= lbl_1_rodata_110;
        if (work->verticalOffset < lbl_1_rodata_29C) {
            work->verticalOffset = lbl_1_rodata_29C;
        }
    }
    fn_1_25D0C(lbl_1_rodata_410);
    camera = &lbl_1_bss_12BC;
    camera->mode = 6;
}

void fn_1_D30C(float value)
{
    OMOBJ *obj = lbl_1_bss_10;
    MDRESULT_CAMERA_WORK *camera;
    float weight = lbl_1_rodata_110 - value;

    fn_1_26EAC(lbl_1_rodata_40C * weight);
    Hu3DTexScrollPosMoveSet(obj->work[1], lbl_1_rodata_104,
        lbl_1_rodata_408 * weight, lbl_1_rodata_104);
    lbl_1_bss_44 = lbl_1_rodata_40C * weight;
    fn_1_25D0C(lbl_1_rodata_410 * weight);
    camera = &lbl_1_bss_12BC;
    camera->mode = lbl_1_rodata_414 * weight;
}

void fn_1_D40C(void)
{
    OMOBJ *obj = lbl_1_bss_10;

    Hu3DTexScrollPosMoveSet(obj->work[1], lbl_1_rodata_104,
        lbl_1_rodata_104, lbl_1_rodata_104);
    fn_1_26EAC(lbl_1_rodata_104);
    fn_1_25D0C(lbl_1_rodata_104);
}

void fn_1_D48C(OMOBJ *obj)
{
    MDRESULT_FLOAT_TABLE_8 orbitTable = lbl_1_rodata_418;
    MDRESULT_MOVE_WORK *move;
    OMOBJ *current;
    HuVecF position;
    double angle;
    float rotation;
    s16 i;

    if (obj->work[2] == 0) {
        obj->work[0] += 1;
        if (obj->work[0] <= obj->work[1]) {
            return;
        }
        for (i = 0; i < 4; i++) {
            current = lbl_1_bss_C;
            Hu3DMotionShiftSet(current->mdlId[i], current->mtnId[i + 8],
                lbl_1_rodata_104, lbl_1_rodata_104, 0);
        }
        obj->work[0] = 0;
        obj->work[1] = 10;
        obj->work[2] = 1;
        return;
    }

    if (obj->work[2] == 1) {
        obj->work[0] += 1;
        if (obj->work[0] <= obj->work[1]) {
            return;
        }
        obj->work[0] = 0;
        obj->work[1] = 0;
        obj->work[2] = 2;
        return;
    }

    fn_1_CE9C();
    for (i = 0; i < 4; i++) {
        move = &lbl_1_bss_71C[i];
        if (move->state != 0) {
            continue;
        }

        current = lbl_1_bss_C;
        Hu3DModelPosGet(current->mdlId[i], &position);
        angle = (lbl_1_rodata_270 *
            ((double)obj->work[0] +
                (double)orbitTable.values[i + lbl_1_bss_1278.values[3] * 4])) /
            lbl_1_rodata_278;
        position.x += (float)(lbl_1_rodata_438 * sin(angle));
        position.y += lbl_1_rodata_440 + fn_1_1FF48(lbl_1_rodata_104,
            lbl_1_rodata_388, move->values[2], move->values[3]);
        angle = (lbl_1_rodata_270 *
            ((double)obj->work[0] +
                (double)orbitTable.values[i + lbl_1_bss_1278.values[3] * 4])) /
            lbl_1_rodata_278;
        position.z += (float)(lbl_1_rodata_448 * cos(angle) -
            lbl_1_rodata_450);
        if (lbl_1_bss_1278.values[3] != 0) {
            if ((i % 2) == 0) {
                position.x -= lbl_1_rodata_458;
            } else {
                position.x += lbl_1_rodata_458;
            }
        }
        fn_1_1FB50(&move->middle, &position, lbl_1_rodata_F4);
        Hu3DModelPosSetV(current->mdlId[i], &move->middle);

        rotation = fn_1_1F878(lbl_1_rodata_104,
            (float)(lbl_1_data_684[i] * 360),
            move->values[2], move->values[3]);
        Hu3DModelRotSet(current->mdlId[i], lbl_1_rodata_104, rotation,
            lbl_1_rodata_104);

        move->values[2] += lbl_1_rodata_110;
        if (move->values[2] > move->values[3]) {
            move->values[2] = lbl_1_rodata_104;
            move->values[3] = lbl_1_rodata_45C;
            lbl_1_data_684[i] = (s16)((rand8() % 4) + 1);
        }

        if (move->values[0] == lbl_1_rodata_104) {
            move->time += lbl_1_rodata_110;
            if (move->time > move->duration) {
                move->values[2] = lbl_1_rodata_104;
                move->values[3] = lbl_1_rodata_F4;
                move->state = 1;
                current = lbl_1_bss_C;
                Hu3DMotionShiftSet(current->mdlId[i], current->mtnId[i + 16],
                    lbl_1_rodata_104, lbl_1_rodata_2B0,
                    HU3D_MOTATTR_LOOP);
            }
        } else {
            move->values[2] += lbl_1_rodata_110;
            if (move->values[2] > move->values[3]) {
                current = lbl_1_bss_C;
                Hu3DModelPosGet(current->mdlId[i], &position);
                position.y -= lbl_1_rodata_2B4;
                if (position.y < lbl_1_rodata_404) {
                    current = lbl_1_bss_C;
                    Hu3DMotionShiftSet(current->mdlId[i], current->mtnId[i],
                        lbl_1_rodata_104, lbl_1_rodata_2B0,
                        HU3D_MOTATTR_LOOP);
                    position.y = lbl_1_rodata_404;
                }
                Hu3DModelPosSetV(current->mdlId[i], &position);
            }
        }
    }
    obj->work[0] += 1;
    if ((double)obj->work[0] > lbl_1_rodata_288) {
        obj->work[0] -= 360;
        obj->work[1] = 0;
    }
}

void fn_1_DC38(s16 index)
{
    OMOBJ *obj = lbl_1_bss_2C;
    MDRESULT_MOVE_WORK *first;
    MDRESULT_MOVE_WORK *second;
    s16 sounds[4];
    s16 i;

    sounds[0] = lbl_1_rodata_460[0];
    sounds[1] = lbl_1_rodata_460[1];
    sounds[2] = lbl_1_rodata_460[2];
    sounds[3] = lbl_1_rodata_460[3];

    obj->work[0] = 0;
    obj->work[1] = 90;
    obj->work[2] = 0;
    obj->work[3] = 0;
    if (lbl_1_bss_1278.values[3] == 0) {
        for (i = 0; i < 4; i++) {
            first = &lbl_1_bss_71C[i];
            first->values[0] = i == index ? lbl_1_rodata_110 :
                lbl_1_rodata_104;
            first->state = 0;
            first->time = lbl_1_rodata_104;
            first->duration = sounds[lbl_1_bss_10D4[i].rank];
            first->values[2] = lbl_1_rodata_104;
            first->values[3] = lbl_1_rodata_3A0;
        }
    } else {
        for (i = 0; i < 2; i++) {
            first = &lbl_1_bss_71C[i * 2];
            second = &lbl_1_bss_71C[i * 2 + 1];
            first->values[0] = i == index ? lbl_1_rodata_110 :
                lbl_1_rodata_104;
            second->values[0] = first->values[0];
            first->state = 0;
            second->state = 0;
            first->time = lbl_1_rodata_104;
            second->time = lbl_1_rodata_104;
            first->duration = lbl_1_rodata_468;
            second->duration = lbl_1_rodata_468;
            first->values[2] = lbl_1_rodata_104;
            second->values[2] = lbl_1_rodata_104;
            first->values[3] = lbl_1_rodata_3A0;
            second->values[3] = lbl_1_rodata_3A0;
        }
    }
    fn_1_4124();
    HuAudFXPlay(1183);
    HuAudFXPlay(1184);
    obj->objFunc = fn_1_D48C;
}

void fn_1_E658(s16 index)
{
    OMOBJ *obj = lbl_1_bss_2C;
    MDRESULT_FLOAT_TABLE_11 values = lbl_1_rodata_474;
    MDRESULT_MOVE_WORK *first;
    MDRESULT_MOVE_WORK *second;
    HuVecF rotation;
    s16 model;
    s16 i;

    obj->work[0] = 0;
    obj->work[1] = 0;
    obj->work[2] = 0;
    obj->work[3] = 0;
    for (i = 0; i < 4; i++) {
        lbl_1_bss_71C[i].state = 0;
    }
    if (lbl_1_bss_1278.values[3] == 0) {
        first = &lbl_1_bss_71C[index];
        first->state = 1;
        first->time = lbl_1_rodata_104;
        first->duration = lbl_1_rodata_2A8;
        model = lbl_1_bss_C->mdlId[index];
        Hu3DModelPosGet(model, &first->current);
        fn_1_1F868(&first->middle, lbl_1_rodata_104,
            values.values[lbl_1_bss_1248[index].character],
            lbl_1_rodata_3A4);
        Hu3DModelRotGet(model, &rotation);
        first->values[1] = rotation.y;
    } else {
        first = &lbl_1_bss_71C[index * 2];
        second = &lbl_1_bss_71C[index * 2 + 1];
        first->state = 1;
        second->state = 1;
        first->time = lbl_1_rodata_104;
        second->time = lbl_1_rodata_104;
        first->duration = lbl_1_rodata_2A8;
        second->duration = lbl_1_rodata_2A8;

        model = lbl_1_bss_C->mdlId[index * 2];
        Hu3DModelPosGet(model, &first->current);
        fn_1_1F868(&first->middle, lbl_1_rodata_410,
            values.values[lbl_1_bss_1248[index * 2].character],
            lbl_1_rodata_3A4);
        Hu3DModelRotGet(model, &rotation);
        first->values[1] = rotation.y;

        model = lbl_1_bss_C->mdlId[index * 2 + 1];
        Hu3DModelPosGet(model, &second->current);
        fn_1_1F868(&second->middle, lbl_1_rodata_390,
            values.values[lbl_1_bss_1248[index * 2 + 1].character],
            lbl_1_rodata_4A0);
        Hu3DModelRotGet(model, &rotation);
        second->values[1] = rotation.y;
    }
    obj->objFunc = fn_1_DED4;
}

void fn_1_DED4(OMOBJ *obj)
{
    MDRESULT_MOVE_WORK *move;
    MDRESULT_S16_TABLE_22 sounds;
    OMOBJ *current;
    OMOBJ *scrollObj;
    HuVecF position;
    float time;
    float weight;
    float rotation;
    s16 character;
    s16 i;

    for (i = 0; i < 4; i++) {
        move = &lbl_1_bss_71C[i];
        if (move->state == 0) {
            continue;
        }
        position.x = fn_1_1FD7C(move->current.x, move->middle.x,
            move->time, move->duration);
        position.y = fn_1_1FD7C(move->current.y, move->middle.y,
            move->time, move->duration);
        position.z = fn_1_1FD7C(move->current.z, move->middle.z,
            move->time, move->duration);
        current = lbl_1_bss_C;
        Hu3DModelPosSetV(current->mdlId[i], &position);

        rotation = fn_1_1FD7C(move->values[1], lbl_1_rodata_46C,
            move->time, move->duration);
        current = lbl_1_bss_C;
        Hu3DModelRotSet(current->mdlId[i], lbl_1_rodata_104, rotation,
            lbl_1_rodata_104);

        move->time += lbl_1_rodata_110;
        if (move->time <= move->duration) {
            continue;
        }

        HuAudFXStop(lbl_1_bss_129C);
        current = lbl_1_bss_C;
        Hu3DModelScaleSet(current->mdlId[i], lbl_1_rodata_110,
            lbl_1_rodata_110, lbl_1_rodata_30C);
        character = lbl_1_bss_1248[i].character;
        CharMotionVoiceOnSet(character, 41, 0);
        if (lbl_1_bss_1278.values[3] == 0) {
            sounds = lbl_1_rodata_10;
            if (character >= 0 && character <= 11) {
                HuAudFXPlay(sounds.values[character]);
            }
        } else {
            sounds = lbl_1_rodata_10;
            if (character >= 0 && character <= 11) {
                HuAudFXPlay(sounds.altValues[character]);
            }
        }
        Hu3DMotionShiftSet(current->mdlId[i], current->mtnId[i + 28],
            lbl_1_rodata_104, lbl_1_rodata_F8, 0);
        move->state = 0;
        obj->objFunc = NULL;
    }

    time = fn_1_1F878(lbl_1_rodata_104, lbl_1_rodata_110,
        (double)obj->work[0], lbl_1_rodata_2A8);
    weight = lbl_1_rodata_110 - time;
    fn_1_26EAC(lbl_1_rodata_40C * weight);
    scrollObj = lbl_1_bss_10;
    Hu3DTexScrollPosMoveSet(scrollObj->work[1], lbl_1_rodata_104,
        lbl_1_rodata_408 * weight, lbl_1_rodata_104);
    lbl_1_bss_44 = lbl_1_rodata_40C * weight;
    fn_1_25D0C(lbl_1_rodata_410 * weight);
    lbl_1_bss_12BC.mode = (s16)(lbl_1_rodata_414 * weight);
    obj->work[0] += 1;
    if (obj->work[0] <= 180) {
        return;
    }

    scrollObj = lbl_1_bss_10;
    Hu3DTexScrollPosMoveSet(scrollObj->work[1], lbl_1_rodata_104,
        lbl_1_rodata_104, lbl_1_rodata_104);
    fn_1_26EAC(lbl_1_rodata_104);
    fn_1_25D0C(lbl_1_rodata_104);
    for (i = 0; i < 4; i++) {
        fn_1_26BE4(i);
        fn_1_26BE4((s16)(i + 4));
        lbl_1_bss_C->objFunc = NULL;
    }

    fn_1_1F868(&position, lbl_1_rodata_104, lbl_1_rodata_470,
        lbl_1_rodata_374);
    for (i = 0; i < 2; i++) {
        fn_1_26164(i, &position);
    }
    fn_1_1F868(&position, lbl_1_rodata_104, lbl_1_rodata_284,
        lbl_1_rodata_104);
    fn_1_26EB0(&position);
    Hu3DModelAttrReset(lbl_1_bss_14->mdlId[1], 1);
    fn_1_20188(lbl_1_bss_11A0[4], 4);
    if (lbl_1_bss_1278.values[3] == 0) {
        HuSprAttrReset(lbl_1_bss_11A0[4], 0, 4);
        HuSprAttrSet(lbl_1_bss_11A0[4], 4, 4);
    } else {
        HuSprAttrReset(lbl_1_bss_11A0[4], 4, 4);
        HuSprAttrSet(lbl_1_bss_11A0[4], 0, 4);
    }
    obj->objFunc = NULL;
}

void fn_1_1AA10(OMOBJ *obj)
{
    s16 i;
    s16 j;
    MDRESULT_PLAYER_WORK *work;

    i = 0;
    work = lbl_1_bss_66C;
    for (; i < 2; i++, work++) {
        HuWinExKill(work->winId);
        for (j = 0; j < 3; j++) {
            Hu3DModelKill(obj->mdlId[j]);
        }
    }
}

void fn_1_1AAA8(OMOBJ *obj)
{
    if (lbl_1_bss_1278.values[3] == 0) {
        fn_1_17F78(obj);
    } else {
        fn_1_192BC(obj);
    }
}

void fn_1_1AAF8(void)
{
    lbl_1_bss_48 = 0;
    if (lbl_1_bss_1278.values[3] == 0) {
        fn_1_181C0();
    } else {
        fn_1_19504();
    }
    lbl_1_bss_38->objFunc = fn_1_1AAA8;
}

void fn_1_1AB5C(void)
{
    if (lbl_1_bss_1278.values[3] == 0) {
        s16 i = 0;
        MDRESULT_PLAYER_WORK *work;

        work = &lbl_1_bss_66C[0];
        for (; i < 4; i++, work++) {
            Hu3DModelAttrSet(work->models[0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[2], HU3D_ATTR_DISPOFF);
            fn_1_20108(work->group, HUSPR_ATTR_DISPOFF);
        }
        for (i = 0; i < 4; i++) {
            Hu3DModelAttrSet(lbl_1_bss_C->mdlId[i], HU3D_ATTR_DISPOFF);
        }
        Hu3DModelAttrSet(lbl_1_bss_4->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_8->mdlId[0], HU3D_ATTR_DISPOFF);
    } else {
        s16 i = 0;
        MDRESULT_PLAYER_WORK *work;

        work = &lbl_1_bss_66C[0];
        for (; i < 2; i++, work++) {
            Hu3DModelAttrSet(work->models[0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[2], HU3D_ATTR_DISPOFF);
            fn_1_20108(work->group, HUSPR_ATTR_DISPOFF);
            fn_1_20108(work->secondGroup, HUSPR_ATTR_DISPOFF);
            HuWinDispOff(work->winId);
        }
        for (i = 0; i < 4; i++) {
            Hu3DModelAttrSet(lbl_1_bss_C->mdlId[i], HU3D_ATTR_DISPOFF);
        }
        Hu3DModelAttrSet(lbl_1_bss_4->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_8->mdlId[0], HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_38->objFunc = NULL;
}

void fn_1_1AD68(OMOBJ *obj)
{
    obj->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 79), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[0] = Hu3DMotionIDGet(obj->mdlId[0]);
    obj->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 80), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[1] = Hu3DMotionIDGet(obj->mdlId[1]);
    obj->mdlId[2] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 81), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[2] = Hu3DMotionIDGet(obj->mdlId[2]);
    if (lbl_1_bss_1278.values[3] == 0) {
        fn_1_18F08(obj);
    } else {
        fn_1_1A570(obj);
    }
    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(obj->mdlId[1], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(obj->mdlId[2], HU3D_ATTR_DISPOFF);
    if (lbl_1_bss_1278.values[3] == 0) {
        s16 i = 0;
        MDRESULT_PLAYER_WORK *work;

        work = &lbl_1_bss_66C[0];
        for (; i < 4; i++, work++) {
            Hu3DModelAttrSet(work->models[0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[2], HU3D_ATTR_DISPOFF);
            fn_1_20108(work->group, HUSPR_ATTR_DISPOFF);
        }
        for (i = 0; i < 4; i++) {
            Hu3DModelAttrSet(lbl_1_bss_C->mdlId[i], HU3D_ATTR_DISPOFF);
        }
        Hu3DModelAttrSet(lbl_1_bss_4->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_8->mdlId[0], HU3D_ATTR_DISPOFF);
    } else {
        s16 i = 0;
        MDRESULT_PLAYER_WORK *work;

        work = &lbl_1_bss_66C[0];
        for (; i < 2; i++, work++) {
            Hu3DModelAttrSet(work->models[0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[2], HU3D_ATTR_DISPOFF);
            fn_1_20108(work->group, HUSPR_ATTR_DISPOFF);
            fn_1_20108(work->secondGroup, HUSPR_ATTR_DISPOFF);
            HuWinDispOff(work->winId);
        }
        for (i = 0; i < 4; i++) {
            Hu3DModelAttrSet(lbl_1_bss_C->mdlId[i], HU3D_ATTR_DISPOFF);
        }
        Hu3DModelAttrSet(lbl_1_bss_4->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_8->mdlId[0], HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_38->objFunc = NULL;
    obj->objFunc = NULL;
}

void fn_1_1B064(OMOBJ *obj)
{
    s16 i;

    if (lbl_1_bss_1278.values[3] == 0) {
        s16 player;
        s16 model;
        MDRESULT_PLAYER_WORK *work;

        player = 0;
        work = lbl_1_bss_66C;
        for (; player < 4; player++, work++) {
            for (model = 0; model < 3; model++) {
                Hu3DModelKill(obj->mdlId[model]);
            }
        }
    } else {
        s16 player;
        MDRESULT_PLAYER_WORK *work;
        s16 model;

        player = 0;
        work = lbl_1_bss_66C;
        for (; player < 2; player++, work++) {
            HuWinExKill(work->winId);
            for (model = 0; model < 3; model++) {
                Hu3DModelKill(obj->mdlId[model]);
            }
        }
    }
    for (i = 0; i < 3; i++) {
        Hu3DMotionKill(obj->mtnId[i]);
        Hu3DModelKill(obj->mdlId[i]);
    }
}
