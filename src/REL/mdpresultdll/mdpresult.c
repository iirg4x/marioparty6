#include <string.h>

#include "datadir_enum.h"
#include "dolphin.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
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

typedef struct MdResultVectorPair_s {
    HuVecF values[2];
} MDRESULT_VECTOR_PAIR;

typedef struct MdResultVectorTable_s {
    HuVecF values[8];
} MDRESULT_VECTOR_TABLE;

typedef struct MdResultCharacterWork_s {
    s32 unk_00;
    s32 unk_04;
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

typedef struct MdResultPlayerWork_s {
    HU3D_MODELID models[3];
    HUSPR_GROUPID group;
    float values[6];
    HUSPR_GROUPID secondGroup;
    s16 state[2];
    HUWINID winId;
} MDRESULT_PLAYER_WORK;

typedef struct MdResultBurstWork_s {
    HuVecF *position;
    float rotX;
    float rotY;
    float rotZ;
    float velocity[3];
    s16 state;
    s16 active;
    float timer;
    float scale;
    float angle;
} MDRESULT_BURST_WORK;

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
void fn_1_1F948(HuVecF *result, const HuVecF *start,
    const HuVecF *middle, const HuVecF *end, float time);
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
void fn_1_3668(OMOBJ *obj);
void fn_1_4A9C(OMOBJ *obj);
void fn_1_4BB8(OMOBJ *obj);
void fn_1_6290(OMOBJ *obj);
void fn_1_7590(OMOBJ *obj);
void fn_1_8184(OMOBJ *obj);
void fn_1_8B70(s32 value);
void fn_1_8F28(OMOBJ *obj);
void fn_1_A85C(OMOBJ *obj);
void fn_1_A984(void);
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
extern HUSPR_GROUPID lbl_1_bss_3D2[];
extern HUSPR_GROUPID lbl_1_bss_714;
extern MDRESULT_SCORE_WORK lbl_1_bss_10D4[4];
extern s32 lbl_1_bss_12B0[2];
extern s16 lbl_1_bss_1278[16];
extern HU3D_MODELID lbl_1_bss_1318;
extern HU3D_MODELID lbl_1_bss_131A;
extern HU3D_MODELID lbl_1_bss_14C2;
extern HU3D_MODELID lbl_1_bss_14C4;
extern HU3D_MODELID lbl_1_bss_14C6;
extern HU3D_MODELID lbl_1_bss_14B0[9];
extern HU3D_MODELID lbl_1_bss_1490[4][4];
extern MDRESULT_BURST_WORK lbl_1_bss_1320[8];
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
extern s16 lbl_1_data_646[3];
extern s32 lbl_1_data_64C[2];
extern char lbl_1_data_666[];
extern char lbl_1_data_678[];

extern const MDRESULT_MESSAGE_NUMBERS lbl_1_rodata_3C;
extern const MDRESULT_FX_NUMBERS lbl_1_rodata_44;
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
extern const float lbl_1_rodata_260;
extern const float lbl_1_rodata_284;
extern const float lbl_1_rodata_298;
extern const float lbl_1_rodata_29C;
extern const float lbl_1_rodata_3B8;
extern const MDRESULT_VECTOR_PAIR lbl_1_rodata_3D0;
extern const float lbl_1_rodata_3E8;
extern const float lbl_1_rodata_2C4;
extern const float lbl_1_rodata_2C8;
extern const float lbl_1_rodata_2CC;
extern const float lbl_1_rodata_2D0;
extern const float lbl_1_rodata_2D4;
extern const float lbl_1_rodata_2B8;
extern const float lbl_1_rodata_2C0;
extern const float lbl_1_rodata_31C;
extern const float lbl_1_rodata_360;
extern const float lbl_1_rodata_380;
extern const float lbl_1_rodata_3CC;
extern const float lbl_1_rodata_404;
extern const float lbl_1_rodata_408;
extern const float lbl_1_rodata_40C;
extern const float lbl_1_rodata_410;
extern const float lbl_1_rodata_414;
extern const float lbl_1_rodata_4A4;
extern const float lbl_1_rodata_594;
extern const float lbl_1_rodata_598;
extern const float lbl_1_rodata_59C;
extern const float lbl_1_rodata_E5C;
extern const float lbl_1_rodata_E70;
extern const float lbl_1_rodata_E74;
extern const float lbl_1_rodata_E78;
extern const float lbl_1_rodata_EF8;
extern const HuVecF lbl_1_rodata_EA8;
extern const MDRESULT_PARTICLE_PRESET lbl_1_rodata_EFC;
extern const MDRESULT_PLAYER_SPRITE_TABLE lbl_1_rodata_6EC;
extern const float lbl_1_rodata_F2C;
extern const float lbl_1_rodata_F80;
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
                    HuAudFXPlayPan(fxNum.values[index], 0x50);
                } else {
                    HuAudFXPlayPan(fxNum.values[index], 0x30);
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
            0x220, 0x2A, -1, 0);
    HuWinDispOff(lbl_1_bss_1304[0]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[0], lbl_1_rodata_104);
    HuWinPriSet(lbl_1_bss_1304[0], 0);
    lbl_1_bss_1304[1] =
        HuWinExCreateFrame(lbl_1_rodata_158, lbl_1_rodata_160,
            0x220, 0x44, -1, 0);
    HuWinDispOff(lbl_1_bss_1304[1]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[1], lbl_1_rodata_164);
    HuWinPriSet(lbl_1_bss_1304[1], 0);
    lbl_1_bss_1304[2] =
        HuWinExCreateFrame(lbl_1_rodata_158, lbl_1_rodata_160,
            0x220, 0x44, -1, 3);
    HuWinDispOff(lbl_1_bss_1304[2]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[2], lbl_1_rodata_164);
    lbl_1_bss_1304[3] =
        HuWinExCreateFrame(lbl_1_rodata_158, lbl_1_rodata_160,
            0x220, 0x44, -1, 4);
    HuWinDispOff(lbl_1_bss_1304[3]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[3], lbl_1_rodata_164);
    lbl_1_bss_1304[4] =
        HuWinExCreateFrame(lbl_1_rodata_158, lbl_1_rodata_160,
            0x220, 0x44, -1, 5);
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

void fn_1_378C(void)
{
    s16 i;
    OMOBJ *obj = lbl_1_bss_C;

    for (i = 0; i < 4; i++) {
        obj->work[i] = 0;
    }
    obj->objFunc = fn_1_3668;
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

void fn_1_AFF4(void)
{
    OMOBJ *obj = lbl_1_bss_24;
    s16 i;

    for (i = 0; i < 22; i++) {
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
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
    HuAudFXPlay(0x496);
    obj->objFunc = fn_1_A85C;
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
    if (lbl_1_bss_1278[3] == 1) {
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

    omSetStatBit(obj, 0x100);
    for (i = 0; i < 4; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 0x3F), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_104, lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < 9; i++) {
        obj->mdlId[i + 4] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 0x41) + i, HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 4] = Hu3DMotionIDGet(obj->mdlId[i + 4]);
        Hu3DModelLayerSet(obj->mdlId[i + 4], 1);
        Hu3DMotionSpeedSet(obj->mdlId[i + 4], lbl_1_rodata_104);
        Hu3DMotionTimeSet(obj->mdlId[i + 4], lbl_1_rodata_258);
        Hu3DModelAttrSet(obj->mdlId[i + 4], HU3D_ATTR_DISPOFF);
    }
    fn_1_B8E8(obj);
    obj->objFunc = NULL;
}

void fn_1_8184(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, 0x100);
    for (i = 0; i < 4; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 0xE), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_104, lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    if (lbl_1_bss_1278[3] == 0) {
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
    if (lbl_1_bss_1278[3] == 1
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
    HuSprDrawNoSet(group->group, 0, 0x40);
    HuSprDrawNoSet(group->group, 1, 0x40);
    HuSprDrawNoSet(group->group, 2, 0x40);
    finalGroup = (MDRESULT_GROUP_WORK *)&lbl_1_bss_714;
    fn_1_20108(finalGroup->group, HUSPR_ATTR_DISPOFF);
    obj->objFunc = NULL;
}

void fn_1_17F60(void)
{
    HUSPR_GROUPID *group = &lbl_1_bss_714;
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
    fn_1_258C(4, 0xE0000, 1);
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
    fn_1_258C(3, 0xE0001, 1);
    fn_1_246C();
    HuAudFXPlay(0x490);
    fn_1_8B70(0);
    HuPrcSleep(60);
    second = lbl_1_bss_4;
    Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    second->work[3] = 0;
    second->objFunc = fn_1_4A9C;
    fn_1_258C(3, 0xE0002, 1);
    fn_1_246C();
    third = lbl_1_bss_18;
    third->work[0] = 0;
    third->work[1] = 10;
    third->work[2] = 0;
    third->objFunc = fn_1_8F28;
    HuPrcSleep(10);
    HuAudFXPlay(0x491);
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
    fn_1_258C(2, 0xE0003, 1);
    fn_1_246C();
    HuAudFXPlay(0x492);
    fn_1_8B70(1);
    HuPrcSleep(60);
    second = lbl_1_bss_8;
    Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    second->work[3] = 0;
    second->objFunc = fn_1_4BB8;
    fn_1_258C(2, 0xE0004, 1);
    fn_1_246C();
    third = lbl_1_bss_18;
    third->work[0] = 0;
    third->work[1] = 10;
    third->work[2] = 0;
    third->objFunc = fn_1_8F28;
    HuPrcSleep(10);
    HuAudFXPlay(0x493);
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
    fn_1_258C(2, 0xE0017, 1);
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
    if (lbl_1_bss_1278[3] == 1) {
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
    lbl_1_bss_12B0[0] = HuAudSStreamPlay(0x22);
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
    s16 bank = lbl_1_bss_1278[0];
    s16 otherBank = (lbl_1_bss_1278[1] - 10) / 5;

    fn_1_20188(group[0], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(group[0], 1, bank);
    HuSprBankSet(group[0], 2, otherBank);
}

void fn_1_1AB5C(void)
{
    if (lbl_1_bss_1278[3] == 0) {
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
        DATANUM(DATA_mdpresult, 0x4F), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[0] = Hu3DMotionIDGet(obj->mdlId[0]);
    obj->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 0x50), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[1] = Hu3DMotionIDGet(obj->mdlId[1]);
    obj->mdlId[2] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 0x51), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[2] = Hu3DMotionIDGet(obj->mdlId[2]);
    if (lbl_1_bss_1278[3] == 0) {
        fn_1_18F08(obj);
    } else {
        fn_1_1A570(obj);
    }
    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(obj->mdlId[1], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(obj->mdlId[2], HU3D_ATTR_DISPOFF);
    if (lbl_1_bss_1278[3] == 0) {
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

void fn_1_1AAA8(OMOBJ *obj)
{
    if (lbl_1_bss_1278[3] == 0) {
        fn_1_17F78(obj);
    } else {
        fn_1_192BC(obj);
    }
}

void fn_1_1AAF8(void)
{
    lbl_1_bss_48 = 0;
    if (lbl_1_bss_1278[3] == 0) {
        fn_1_181C0();
    } else {
        fn_1_19504();
    }
    lbl_1_bss_38->objFunc = fn_1_1AAA8;
}

void fn_1_1E1B4(OMOBJ *obj)
{
    if (lbl_1_bss_1278[3] == 0) {
        fn_1_1B194(obj);
    } else {
        fn_1_1C9B8(obj);
    }
}

void fn_1_1E204(void)
{
    if (lbl_1_bss_1278[3] == 0) {
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

void fn_1_B454(OMOBJ *obj, s16 index, HuVecF *pos)
{
    index--;
    lbl_1_bss_81C[index].active = 1;
    lbl_1_bss_81C[index].timer = lbl_1_rodata_104;
    lbl_1_bss_81C[index].scale = lbl_1_rodata_F4;
    Hu3DModelPosSetV(obj->mdlId[index + 4], pos);
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
                    DATANUM(DATA_mdpresult, 0x41), HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                obj->mdlId[(i * 11) + j] = Hu3DModelCreate(HuDataSelHeapReadNum(
                    DATANUM(DATA_mdpresult, 0x40) + j, HU_MEMNUM_OVL, HEAP_MODEL));
            }
            Hu3DModelAttrSet(obj->mdlId[(i * 11) + j], HU3D_ATTR_DISPOFF);
        }
    }
    obj->objFunc = NULL;
}

void fn_1_95E8(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 4; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 0x51), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 3);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i], lbl_1_rodata_104,
            lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < 4; i++) {
        obj->mdlId[i + 4] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 0x50), HU_MEMNUM_OVL, HEAP_MODEL));
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

void fn_1_AA7C(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 3; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 0x3A) + i, HU_MEMNUM_OVL, HEAP_MODEL));
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
            DATANUM(DATA_mdpresult, 0x3D), HU_MEMNUM_OVL, HEAP_MODEL));
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
    if (lbl_1_bss_1278[3] == 0) {
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

void fn_1_1B064(OMOBJ *obj)
{
    s16 i;

    if (lbl_1_bss_1278[3] == 0) {
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
    if (lbl_1_bss_1278[3] == 0) {
        HUSPR_GROUPID *group = lbl_1_bss_3D2;
    } else {
        HUSPR_GROUPID *group = lbl_1_bss_3D2;
    }
}

void fn_1_1E258(void)
{
    if (lbl_1_bss_1278[3] == 0) {
        fn_1_1C050();
    } else {
        fn_1_1D874();
    }
    lbl_1_bss_3C->objFunc = NULL;
}

void fn_1_1E358(OMOBJ *obj)
{
    if (lbl_1_bss_1278[3] == 0) {
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
        DATANUM(DATA_mdpresult, 0x74), HU_MEMNUM_OVL, HEAP_MODEL));
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
    MDRESULT_BURST_WORK *work = &lbl_1_bss_1320[index];

    work->position->x = position->x;
    work->position->y = position->y;
    work->position->z = position->z;
    work->rotX = work->rotY = work->rotZ = lbl_1_rodata_E70;
    work->rotY = value;
}

void fn_1_2429C(s16 index)
{
    MDRESULT_BURST_WORK *work = &lbl_1_bss_1320[index];

    work->active = 0;
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

void fn_1_26BE4(s16 index)
{
    HU3D_PARTICLE_DATA *burstData;
    HU3D_PARTICLE *burstParticle;
    HU3D_MODEL *burstModel;
    MDRESULT_BURST_WORK *work;
    s16 i;
    HU3D_PARTICLE_DATA *trailData;
    HU3D_PARTICLE *trailParticle;
    HU3D_MODEL *trailModel;

    burstModel = &Hu3DData[lbl_1_bss_14B0[index]];
    burstParticle = burstModel->hookData;
    burstData = burstParticle->data;
    burstData->time = 0;
    work = &lbl_1_bss_1320[index];
    work->active = 0;
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
        HuMemDirectFree(lbl_1_bss_1320[i].position);
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
    { 0xE0000, -1 },
};

const MDRESULT_FX_NUMBERS lbl_1_rodata_44 = {
    {
        0x3B5, 0x3B6, 0x3B7, 0x3B8, 0x3B9, 0x3BA, 0x3BB, -1,
        0x3AD, 0x3AE, 0x3AF, 0x3B0, 0x3B1, 0x3B2, 0x3B3, -1,
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
