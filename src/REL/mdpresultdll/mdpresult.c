#include <string.h>

#include "datadir_enum.h"
#include "dolphin.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/saveload.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"
#include "messdir_enum.h"

#include "REL/mdpresultDll.h"

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

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
void HuSprTexLoad(ANIMDATA *anim, s16 bmpNo, s16 texMapId,
    GXTexWrapMode wrapS, GXTexWrapMode wrapT, GXTexFilter filter);
void fn_1_26CF8(s16 index, HuVecF *position, float value);
float fn_1_1FC94(float start, float end, float time, float duration);
void fn_1_2001C(HU3D_MODELID modelId, const HuVecF *first,
    const HuVecF *second);
void fn_1_20108(HUSPR_GROUPID groupId, s32 attr);
void fn_1_20208(HUSPR_GROUPID groupId, s32 member, s16 value);
void fn_1_2035C(HUSPR_GROUPID groupId, s32 member, s16 value);
void fn_1_25E6C(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_26070(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_2668C(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);
void fn_1_26BE4(s16 index);
void fn_1_26EB0(HuVecF *position);
void fn_1_0(s16 index, s16 table);
s32 fn_1_2CC(s16 first, s16 second);
void fn_1_5484(OMOBJ *obj);
void fn_1_5690(OMOBJ *obj);
void fn_1_5860(OMOBJ *obj);
void fn_1_5A70(OMOBJ *obj, s16 index);
void fn_1_5E18(OMOBJ *obj);
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
void fn_1_8470(OMOBJ *obj);
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
void fn_1_AD94(s16 player, s16 step);
s16 fn_1_C9A0(void);
void fn_1_CAEC(OMOBJ *obj);
void fn_1_BB60(OMOBJ *obj);
void fn_1_3CC(void);
void fn_1_1018C(s32 unused, MDRESULT_CAMERA_WORK *work);
void fn_1_10270(s32 unused, MDRESULT_CAMERA_WORK *work);
void fn_1_1860(OMOBJ *obj);
void fn_1_1F308(void);
void fn_1_1AAF8(void);
void fn_1_1AB5C(void);
void fn_1_1E204(void);
void fn_1_1E258(void);
void fn_1_17CF4(void);
void fn_1_17248(void);
void fn_1_17B10(void);
void fn_1_F1C4(void);
void fn_1_4694(OMOBJ *obj);
void fn_1_4CD4(OMOBJ *obj);
void fn_1_4EF0(OMOBJ *obj);
void fn_1_5160(OMOBJ *obj);
void fn_1_6CB8(OMOBJ *obj);
void fn_1_95E8(OMOBJ *obj);
void fn_1_AA7C(OMOBJ *obj);
void fn_1_B05C(OMOBJ *obj);
void fn_1_CE0C(OMOBJ *obj);
void fn_1_31F8(OMOBJ *obj);
void fn_1_17DCC(OMOBJ *obj);
void fn_1_1AD68(OMOBJ *obj);
void fn_1_1E358(OMOBJ *obj);
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
void fn_1_25DB0(s16 index, HuVecF *position, float alpha);
void fn_1_25D0C(float value);
void fn_1_25FF4(s16 index);
void fn_1_25B90(void);
void fn_1_26EAC(float value);
void fn_1_26F74(void);
void fn_1_1F3D4(void);
void fn_1_1F7FC(void);
void fn_1_1F834(void);
void fn_1_1C050(void);
void fn_1_1D874(void);
void fn_1_1E5E8(HUSPRITE *sprite);
void fn_1_252F8(void);

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
extern OMOBJ *lbl_1_bss_34;
extern OMOBJ *lbl_1_bss_38;
extern s16 lbl_1_bss_62[4][55];
extern s16 lbl_1_bss_21A[4][55];
extern s16 lbl_1_bss_54;
extern s16 lbl_1_bss_56;
extern s16 lbl_1_bss_58;
extern s16 lbl_1_bss_10CC[4];
extern MDRESULT_STATE_WORK lbl_1_bss_8AC[4];
extern MDRESULT_MOVE_WORK lbl_1_bss_8EC[7];
extern MDRESULT_MODEL_EFFECT_WORK lbl_1_bss_ADC[11];
extern MDRESULT_COLOR_STEP lbl_1_bss_AAC[4];
extern MDRESULT_COLOR_WORK lbl_1_bss_ABC[4];
extern s16 lbl_1_bss_70C[4];
extern OMOBJ *lbl_1_bss_3C[2];
extern s16 lbl_1_bss_48;
extern float lbl_1_bss_44;
extern char lbl_1_data_67D[];
extern char lbl_1_data_682[];
extern char lbl_1_data_68C[];
extern char lbl_1_data_6A2[];
extern char lbl_1_data_6D0[];
extern char lbl_1_data_6E0[];
extern char lbl_1_data_6ED[];
extern char lbl_1_data_6F7[];
extern char lbl_1_data_703[];
extern char lbl_1_data_70A[];
extern HuVecF lbl_1_bss_109C[4];
extern HUSPRID lbl_1_bss_117C[18];
extern HUSPR_GROUPID lbl_1_bss_11A0[6];
extern ANIMDATA *lbl_1_bss_11AC[39];
extern HUSPR_GROUPID lbl_1_bss_60;
extern ANIMDATA *lbl_1_bss_5C;
extern MDRESULT_PLAYER_WORK lbl_1_bss_66C[4];
extern MDRESULT_EMITTER_WORK lbl_1_bss_81C[9];
extern s32 lbl_1_bss_12A0[4];
extern HUSPR_GROUPID lbl_1_bss_3D2[MDRESULT_GROUP_TABLE_COUNT];
extern MDRESULT_GROUP_WORK lbl_1_bss_714;
extern MDRESULT_SCORE_WORK lbl_1_bss_10D4[4];
extern MDRESULT_MOVE_WORK lbl_1_bss_71C[4];
extern s32 lbl_1_bss_12B0[3];
extern s32 lbl_1_bss_129C;
extern s32 lbl_1_bss_1298;
extern MDRESULT_BSS_1278_WORK lbl_1_bss_1278;
extern MDRESULT_MOVE_WORK lbl_1_bss_D9C[8];
extern MDRESULT_CAMERA_WORK lbl_1_bss_12BC;
extern HUWINID lbl_1_bss_1304[5];
extern HU3D_LIGHTID lbl_1_bss_130E[5];
extern MDRESULT_MOVE_WORK lbl_1_bss_F9C[4];
extern MDRESULT_CHARACTER_WORK lbl_1_bss_1248[4];
extern HuVecF lbl_1_data_0[16];
extern s32 lbl_1_data_C0[39];
extern char lbl_1_data_719[];
extern char lbl_1_data_76C[];
extern s16 lbl_1_data_15C[6];
extern s32 lbl_1_data_5F4[11];
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
extern s16 lbl_1_data_3A8[6];
extern MDRESULT_GRAPH_TABLE lbl_1_data_3B4[6];
extern float lbl_1_data_754;
extern float lbl_1_data_758;
extern float lbl_1_bss_4C;
extern float lbl_1_bss_50;

extern GXColor lbl_1_data_75C[4];


void fn_1_120(HUWINID winId, u32 mess, s16 index);
void fn_1_16C4(MDRESULT_CAMERA_WORK *camera);
void fn_1_1714(MDRESULT_CAMERA_WORK *camera);
void fn_1_1764(MDRESULT_CAMERA_WORK *camera, float weight);
void fn_1_17D4(MDRESULT_CAMERA_CALLBACK callback);
void fn_1_17F4(OMOBJ *obj, MDRESULT_CAMERA_WORK *camera);
void fn_1_1840(s16 mode);
void fn_1_1930(MDRESULT_CAMERA_CALLBACK callback);
void fn_1_1AA4(void);
void fn_1_1B00(void);
void fn_1_1C34(void);
void fn_1_1C70(s16 winNo);
void fn_1_1CE0(s16 winNo);
void fn_1_1D50(s16 winNo);
s16 fn_1_1D8C(s16 winNo, s16 mode);
void fn_1_1E28(s16 winNo, s32 messNum, s16 speed);
void fn_1_1EE4(s16 winNo, s32 messNum, s16 insertPos);
void fn_1_1F54(void);
void fn_1_2208(void);
void fn_1_2264(s16 winNo);
void fn_1_23C0(void);
void fn_1_246C(void);
s16 fn_1_24CC(s16 mode);
void fn_1_258C(s16 winNo, s32 messNum, s16 speed);
void fn_1_27A4(s16 winNo, s32 messNum, s16 insertPos);
void fn_1_295C(s32 messNum, s16 positionF);
void fn_1_2B44(void);
void fn_1_2BF0(void);
void fn_1_2CA4(void);
void fn_1_2CA8(void);
void fn_1_2ED0(void);
void fn_1_2ED4(s16 index);
void fn_1_2F80(s16 index);
void fn_1_30C4(void);
void fn_1_3104(OMOBJ *obj);
void fn_1_3304(OMOBJ *obj);
void fn_1_378C(void);
void fn_1_3E98(void);
void fn_1_49C8(OMOBJ *obj);
void fn_1_4B44(void);
void fn_1_4C60(void);
void fn_1_4E68(OMOBJ *obj);
void fn_1_50C0(OMOBJ *obj);
void fn_1_52C4(OMOBJ *obj);
void fn_1_5360(OMOBJ *obj);
void fn_1_5A60(float value);
void fn_1_6C7C(OMOBJ *obj);
void fn_1_7518(OMOBJ *obj);
void fn_1_7560(s16 state, u8 flag);
void fn_1_83E0(OMOBJ *obj);
void fn_1_95A4(void);
void fn_1_9850(OMOBJ *obj);
void fn_1_98E0(void);
void fn_1_AD04(OMOBJ *obj);
void fn_1_AFF4(void);
void fn_1_B178(OMOBJ *obj);
void fn_1_B454(OMOBJ *obj, s16 index, HuVecF *pos);
void fn_1_BACC(void);
void fn_1_CD04(OMOBJ *obj);
void fn_1_CE18(OMOBJ *obj);
void fn_1_CE60(void);
void fn_1_CE9C(void);
void fn_1_D30C(float value);
void fn_1_D40C(void);
void fn_1_F0E0(OMOBJ *obj);
void fn_1_F138(void);
void fn_1_10098(void);
void fn_1_100B8(void);
int _prolog(void);
void _epilog(void);
s32 fn_1_102E4(void);
s32 fn_1_1295C(void);
s16 fn_1_12C80(u8 *mask);
s32 fn_1_170DC(void);
s32 fn_1_171EC(void);
void fn_1_17D94(void);
void fn_1_18E14(void);
void fn_1_1922C(OMOBJ *obj);
void fn_1_1A468(void);
void fn_1_1AA10(OMOBJ *obj);
void fn_1_1AAA8(OMOBJ *obj);
void fn_1_1B064(OMOBJ *obj);
void fn_1_1E1B4(OMOBJ *obj);
s32 fn_1_1E4B8(HuVec2f *originA, HuVec2f *directionA, HuVec2f *originB,
    HuVec2f *directionB, HuVec2f *intersection);

void fn_1_0(s16 index, s16 table)
{
    MDRESULT_S16_TABLE_22 sounds = { {
        { 905, 903, 915, 923, 919, 897, 921, 899, 918, 911, 901 },
        { 906, 904, 916, 924, 920, 898, 922, 900, 917, 912, 902 },
    } };

    if (index < 0 || index > 11) {
        return;
    }
    HuAudFXPlay(sounds.values[table][index]);
}

void fn_1_120(HUWINID winId, u32 mess, s16 index)
{
    MDRESULT_MESSAGE_NUMBERS messNum = { { 917504, -1 } };
    MDRESULT_FX_NUMBERS fxNum = { {
        949, 950, 951, 952, 953, 954, 955, -1,
        941, 942, 943, 944, 945, 946, 947, -1,
    } };
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

s32 fn_1_2CC(s16 first, s16 second)
{
    MDRESULT_BYTE_TABLE_110 table = { {
        { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 0, 5 },
        { 0, 6 }, { 0, 7 }, { 0, 8 }, { 0, 9 }, { 0, 10 },
        { 1, 2 }, { 1, 3 }, { 1, 4 }, { 1, 5 }, { 1, 6 },
        { 1, 7 }, { 1, 8 }, { 1, 9 }, { 1, 10 }, { 2, 3 },
        { 2, 4 }, { 2, 5 }, { 2, 6 }, { 2, 7 }, { 2, 8 },
        { 2, 9 }, { 2, 10 }, { 3, 4 }, { 3, 5 }, { 3, 6 },
        { 3, 7 }, { 3, 8 }, { 3, 9 }, { 3, 10 }, { 4, 5 },
        { 4, 6 }, { 4, 7 }, { 4, 8 }, { 4, 9 }, { 4, 10 },
        { 5, 6 }, { 5, 7 }, { 5, 8 }, { 5, 9 }, { 5, 10 },
        { 6, 7 }, { 6, 8 }, { 6, 9 }, { 6, 10 }, { 7, 8 },
        { 7, 9 }, { 7, 10 }, { 8, 9 }, { 8, 10 }, { 9, 10 },
    } };
    s8 i;

    for (i = 0; i < 55; i++) {
        if ((first == table.values[i][0] && second == table.values[i][1])
            || (first == table.values[i][1] && second == table.values[i][0])) {
            break;
        }
    }
    if (i == 55) {
        return DATANUM(DATA_blast5, 55);
    }
    return DATANUM(DATA_blast5, 0) + i;
}

void fn_1_3CC(void)
{
    MDRESULT_BSS_1278_WORK *work = &lbl_1_bss_1278;
    s16 order[4];
    s16 i;
    MDRESULT_CHARACTER_WORK *character;

    work->values[0] = GwSystem.boardNo;
    work->values[1] = GwSystem.turnMax;
    work->values[2] = GwSystem.bonusStarF;
    work->values[3] = GwSystem.tagF;

    if (work->values[3] == 1) {
        s16 solo;
        s16 team;

        solo = team = 0;

        for (i = 0; i < 4; i++) {
            if (GwPlayer[i].team == 0) {
                order[solo++] = i;
            } else {
                order[2 + team++] = i;
            }
        }
    } else {
        for (i = 0; i < 4; i++) {
            order[i] = i;
        }
    }

    for (i = 0; i < 4; i++) {
        lbl_1_bss_10CC[i] = order[i];
    }

    i = 0;
    character = lbl_1_bss_1248;
    for (; i < 4; i++, character++) {
        character->unk_00 = order[i];
        character->unk_02 = GwPlayerConf[character->unk_00].grpNo;
        character->unk_04 = GwPlayerConf[character->unk_00].type;
        character->unk_06 = GwPlayerConf[character->unk_00].comDif;
        character->character = GwPlayerConf[character->unk_00].charNo;
        character->unk_0A = GwPlayerConf[character->unk_00].padNo;
    }

    work->messages[0] = lbl_1_data_5F4[lbl_1_bss_1248[0].character];
    work->messages[1] = lbl_1_data_5F4[lbl_1_bss_1248[1].character];
    work->messages[2] = lbl_1_data_5F4[lbl_1_bss_1248[2].character];
    work->messages[3] = lbl_1_data_5F4[lbl_1_bss_1248[3].character];

    work->messages[4] = fn_1_2CC(lbl_1_bss_1248[0].character,
        lbl_1_bss_1248[1].character);
    work->messages[5] = fn_1_2CC(lbl_1_bss_1248[2].character,
        lbl_1_bss_1248[3].character);

    if (work->values[3] == 0) {
        for (i = 0; i < 4; i++) {
            lbl_1_bss_10D4[i].playerIndex = i;
            lbl_1_bss_10D4[i].teamIndex = 0;
            lbl_1_bss_10D4[i].rank = i;
            lbl_1_bss_10D4[i].star = GwPlayer[order[i]].star;
            lbl_1_bss_10D4[i].values[15] = GwPlayer[order[i]].handicap;
            lbl_1_bss_10D4[i].coin = GwPlayer[order[i]].coin;
            lbl_1_bss_10D4[i].values[0] = GwPlayer[order[i]].coinTotalMg;
            lbl_1_bss_10D4[i].values[1] = GwPlayer[order[i]].capsuleUseNum;
            lbl_1_bss_10D4[i].values[2] = GwPlayer[order[i]].hatenaMasuNum;
            lbl_1_bss_10D4[i].values[3] = GwPlayer[order[i]].star;
            lbl_1_bss_10D4[i].values[4] = GwPlayer[order[i]].coin;
            lbl_1_bss_10D4[i].values[5] = GwPlayer[order[i]].coinTotalMg;
            lbl_1_bss_10D4[i].values[6] = GwPlayer[order[i]].capsuleUseNum;
            lbl_1_bss_10D4[i].values[7] = GwPlayer[order[i]].plusMasuNum;
            lbl_1_bss_10D4[i].values[8] = GwPlayer[order[i]].minusMasuNum;
            lbl_1_bss_10D4[i].values[9] = GwPlayer[order[i]].capsuleMasuNum;
            lbl_1_bss_10D4[i].values[10] = GwPlayer[order[i]].hatenaMasuNum;
            lbl_1_bss_10D4[i].values[11] = GwPlayer[order[i]].kettouMasuNum;
            lbl_1_bss_10D4[i].values[12] = GwPlayer[order[i]].miracleMasuNum;
            lbl_1_bss_10D4[i].values[13] = GwPlayer[order[i]].koopaMasuNum;
            lbl_1_bss_10D4[i].values[14] = GwPlayer[order[i]].donkeyMasuNum;
        }
    } else {
        for (i = 0; i < 4; i++) {
            lbl_1_bss_10D4[i].star = 0;
            lbl_1_bss_10D4[i].values[15] = 0;
        }
        for (i = 0; i < 2; i++) {
            lbl_1_bss_10D4[i].playerIndex = order[i * 2];
            lbl_1_bss_10D4[i].teamIndex = i;
            lbl_1_bss_10D4[i].rank = i;
            lbl_1_bss_10D4[i].star = GwPlayer[order[i * 2]].star + GwPlayer[order[(i * 2) + 1]].star;
            lbl_1_bss_10D4[i].values[15] = GwPlayer[order[i * 2]].handicap + GwPlayer[order[(i * 2) + 1]].handicap;
            lbl_1_bss_10D4[i].coin = GwPlayer[order[i * 2]].coin + GwPlayer[order[(i * 2) + 1]].coin;
            lbl_1_bss_10D4[i].values[0] = GwPlayer[order[i * 2]].coinTotalMg
                + GwPlayer[order[(i * 2) + 1]].coinTotalMg;
            lbl_1_bss_10D4[i].values[1] = GwPlayer[order[i * 2]].capsuleUseNum
                + GwPlayer[order[(i * 2) + 1]].capsuleUseNum;
            lbl_1_bss_10D4[i].values[2] = GwPlayer[order[i * 2]].hatenaMasuNum
                + GwPlayer[order[(i * 2) + 1]].hatenaMasuNum;
            lbl_1_bss_10D4[i].values[3] = GwPlayer[order[i * 2]].star
                + GwPlayer[order[(i * 2) + 1]].star;
            lbl_1_bss_10D4[i].values[4] = GwPlayer[order[i * 2]].coin
                + GwPlayer[order[(i * 2) + 1]].coin;
            lbl_1_bss_10D4[i].values[5] = GwPlayer[order[i * 2]].coinTotalMg
                + GwPlayer[order[(i * 2) + 1]].coinTotalMg;
            lbl_1_bss_10D4[i].values[6] = GwPlayer[order[i * 2]].capsuleUseNum
                + GwPlayer[order[(i * 2) + 1]].capsuleUseNum;
            lbl_1_bss_10D4[i].values[7] = GwPlayer[order[i * 2]].plusMasuNum
                + GwPlayer[order[(i * 2) + 1]].plusMasuNum;
            lbl_1_bss_10D4[i].values[8] = GwPlayer[order[i * 2]].minusMasuNum
                + GwPlayer[order[(i * 2) + 1]].minusMasuNum;
            lbl_1_bss_10D4[i].values[9] = GwPlayer[order[i * 2]].capsuleMasuNum
                + GwPlayer[order[(i * 2) + 1]].capsuleMasuNum;
            lbl_1_bss_10D4[i].values[10] = GwPlayer[order[i * 2]].hatenaMasuNum
                + GwPlayer[order[(i * 2) + 1]].hatenaMasuNum;
            lbl_1_bss_10D4[i].values[11] = GwPlayer[order[i * 2]].kettouMasuNum
                + GwPlayer[order[(i * 2) + 1]].kettouMasuNum;
            lbl_1_bss_10D4[i].values[12] = GwPlayer[order[i * 2]].miracleMasuNum
                + GwPlayer[order[(i * 2) + 1]].miracleMasuNum;
            lbl_1_bss_10D4[i].values[13] = GwPlayer[order[i * 2]].koopaMasuNum
                + GwPlayer[order[(i * 2) + 1]].koopaMasuNum;
            lbl_1_bss_10D4[i].values[14] = GwPlayer[order[i * 2]].donkeyMasuNum
                + GwPlayer[order[(i * 2) + 1]].donkeyMasuNum;
        }
    }
}

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
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f,
        10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
        640.0f, 480.0f, 0.0f,
        1.0f);
    memset(camera, 0, sizeof(MDRESULT_CAMERA_WORK));
    camera->callback = callback;
    camera->center.x = 0.0f;
    camera->center.y = 65.0f;
    camera->center.z = -800.0f;
    camera->rot.x = -7.25f;
    camera->rot.y = 0.0f;
    camera->rot.z = 0.0f;
    camera->zoom = 2650.0f;
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
    HuVecF pos[2] = {
        { 0.0f, 1.0f, 1.0f },
        { -1.0f, 1.0f, -1.0f }
    };
    HuVecF dir[2] = {
        { 0.0f, -1.0f, -1.0f },
        { 1.0f, -1.0f, -1.0f }
    };
    GXColor color = { 255, 255, 255, 128 };

    lbl_1_bss_130E[0] =
        Hu3DGLightCreateV(&pos[0], &dir[0], &color);
    Hu3DGLightInfinitytSet(lbl_1_bss_130E[0]);
    Hu3DGLightStaticSet(lbl_1_bss_130E[0], TRUE);
    lbl_1_bss_130E[1] =
        Hu3DGLightCreateV(&pos[1], &dir[1], &color);
    Hu3DGLightInfinitytSet(lbl_1_bss_130E[1]);
    Hu3DGLightStaticSet(lbl_1_bss_130E[1], TRUE);
}

void fn_1_1C34(void)
{
    Hu3DGLightKill(lbl_1_bss_130E[0]);
    Hu3DGLightKill(lbl_1_bss_130E[1]);
}

void fn_1_1C70(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_1304[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_1304[winNo]);
    }
}

void fn_1_1CE0(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_1304[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_1304[winNo]);
    }
}

void fn_1_1D50(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_1304[winNo]);
}

s16 fn_1_1D8C(s16 winNo, s16 mode)
{
    if (mode != 0) {
        HuWinAttrSet(lbl_1_bss_1304[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_1304[winNo], HUWIN_ATTR_NOCANCEL);
    }
    return HuWinChoiceGet(lbl_1_bss_1304[winNo], -1);
}

void fn_1_1E28(s16 winNo, s32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_1304[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_1304[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_1304[winNo], speed);
    if (lbl_1_data_620 != messNum) {
        lbl_1_data_620 = -1;
    }
}

void fn_1_1EE4(s16 winNo, s32 messNum, s16 insertPos)
{
    HuWinHomeClear(lbl_1_bss_1304[winNo]);
    HuWinInsertMesSet(lbl_1_bss_1304[winNo], messNum, insertPos);
}

void fn_1_1F54(void)
{
    s16 i;

    HuWinInit(1);
    lbl_1_bss_1304[0] =
        HuWinExCreateFrame(16.0f, 337.0f,
            544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_1304[0]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[0], 0.0f);
    HuWinPriSet(lbl_1_bss_1304[0], 0);
    lbl_1_bss_1304[1] =
        HuWinExCreateFrame(16.0f, 372.0f,
            544, 68, -1, 0);
    HuWinDispOff(lbl_1_bss_1304[1]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[1], 0.9f);
    HuWinPriSet(lbl_1_bss_1304[1], 0);
    lbl_1_bss_1304[2] =
        HuWinExCreateFrame(16.0f, 372.0f,
            544, 68, -1, 3);
    HuWinDispOff(lbl_1_bss_1304[2]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[2], 0.9f);
    lbl_1_bss_1304[3] =
        HuWinExCreateFrame(16.0f, 372.0f,
            544, 68, -1, 4);
    HuWinDispOff(lbl_1_bss_1304[3]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[3], 0.9f);
    lbl_1_bss_1304[4] =
        HuWinExCreateFrame(16.0f, 372.0f,
            544, 68, -1, 5);
    HuWinDispOff(lbl_1_bss_1304[4]);
    HuWinBGTPLvlSet(lbl_1_bss_1304[4], 0.9f);

    for (i = 0; i < 5; i++) {
        winData[lbl_1_bss_1304[i]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_1304[i], (HUWIN_CALLBACK)fn_1_120);
    }
}

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

void fn_1_295C(s32 messNum, s16 positionF)
{
    if (lbl_1_data_646[1] == -1) {
        lbl_1_data_646[1] = 0;
        lbl_1_data_64C[1] = -1;
        fn_1_1C70(lbl_1_data_646[1]);
    }
    if (positionF == 0) {
        HuWinPosSet(lbl_1_bss_1304[0], 16.0f,
            337.0f);
    } else {
        HuWinPosSet(lbl_1_bss_1304[0], 16.0f,
            412.0f);
    }
    if (lbl_1_data_64C[1] != messNum) {
        lbl_1_data_64C[1] = messNum;
        fn_1_1E28(lbl_1_data_646[1], lbl_1_data_64C[1], 0);
    }
}

void fn_1_2B44(void)
{
    if (lbl_1_data_646[1] != -1) {
        fn_1_1CE0(lbl_1_data_646[1]);
    }
    lbl_1_data_646[1] = -1;
    lbl_1_data_64C[1] = -1;
}

void fn_1_2BF0(void)
{
    Vec shadowPos = { 0.0f, 3000.0f, 600.0f };
    Vec shadowUp = { 0.0f, 1.0f, 0.0f };
    Vec shadowTarget = { 0.0f, 0.0f, 0.0f };

    Hu3DShadowCreate(
        30.0f, 10.0f, 10000.0f);
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
    HuVecF positions[8] = {
        {53.0f, 129.0f, 1500.0f},
        {32.0f, 177.0f, 1500.0f},
        {32.0f, 231.0f, 1500.0f},
        {53.0f, 279.0f, 1500.0f},
        {32.0f, 177.0f, 1500.0f},
        {32.0f, 231.0f, 1500.0f},
        {32.0f, 202.0f, 1500.0f},
        {53.0f, 250.0f, 1500.0f},
    };
    Vec world;

    Hu3D2Dto3D(&positions[index + (lbl_1_bss_1278.values[3] * 4)], 1,
        &world);
    lbl_1_bss_109C[0].x = world.x;
    lbl_1_bss_109C[0].y = world.y;
    lbl_1_bss_109C[0].z = world.z;
}

void fn_1_2F80(s16 index)
{
    OMOBJ *obj = lbl_1_bss_30;
    HuVecF positions[8] = {
        {53.0f, 129.0f, 1500.0f},
        {32.0f, 177.0f, 1500.0f},
        {32.0f, 231.0f, 1500.0f},
        {53.0f, 279.0f, 1500.0f},
        {32.0f, 177.0f, 1500.0f},
        {32.0f, 231.0f, 1500.0f},
        {32.0f, 202.0f, 1500.0f},
        {53.0f, 250.0f, 1500.0f},
    };
    Vec world;

    fn_1_2001C(obj->mdlId[0],
        &positions[index + (lbl_1_bss_1278.values[3] * 4)], NULL);
    Hu3DModelRotSet(obj->mdlId[0], 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 0.4f, 0.4f, 0.4f);
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
    fn_1_1FB50(&transform, &lbl_1_bss_109C[0], 3.0f);
    Hu3DModelPosSetV(obj->mdlId[0], &transform);
    Hu3DModelRotGet(obj->mdlId[0], &transform);
    transform.z = fn_1_1F8BC(
        transform.z, 0.0f, 3.0f);
    Hu3DModelRotSetV(obj->mdlId[0], &transform);
    Hu3DModelScaleGet(obj->mdlId[0], &transform);
    transform.x = transform.y = transform.z = fn_1_1F8BC(
        transform.x, 0.4f, 3.0f);
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
        0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(obj->mdlId[0], 0.5f,
        0.5f, 0.5f);
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

void fn_1_3364(s16 index, s16 motion, float end, s32 attr)
{
    OMOBJ *obj = lbl_1_bss_C;

    if (motion == 7) {
        CharMotionVoiceOnSet(lbl_1_bss_1248[index].character, 41, 0);
        if (lbl_1_bss_1278.values[3] == 0) {
            fn_1_0(lbl_1_bss_1248[index].character, 0);
        } else {
            fn_1_0(lbl_1_bss_1248[index].character, 1);
        }
    }
    Hu3DMotionShiftSet(obj->mdlId[index], obj->mtnId[index + motion * 4],
        0.0f, end, (u32)attr);
    if (motion == 8) {
        Hu3DMotionShiftStartEndSet(obj->mdlId[index], 30.0f,
            90.0f);
    }
}

void fn_1_3668(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        if (obj->work[i] == 0 && Hu3DMotionEndCheck(obj->mdlId[i]) != 0) {
            fn_1_3364(i, 0, 15.0f, HU3D_MOTATTR_LOOP);
            obj->work[i] = 1;
        }
    }
    for (i = 0; i < 4; i++) {
        if (obj->work[i] == 0) {
            break;
        }
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
    s16 i;

    for (i = 0; i < 4; i++) {
        fn_1_3364(i, 0, 15.0f, HU3D_MOTATTR_LOOP);
    }
    obj->objFunc = NULL;
}

void fn_1_3894(OMOBJ *obj)
{
    MDRESULT_MOVE_WORK *move;
    MDRESULT_MOVE_WORK *particle;
    HuVecF position;
    s16 i;

    if (obj->work[0] == 0) {
        for (i = 0; i < 4; i++) {
            move = &lbl_1_bss_D9C[i];
            fn_1_1F948(&position, &move->current, &move->middle,
                &move->target, fn_1_1F878(0.0f,
                    1.0f, move->time, move->duration));
            fn_1_26CF8(i + 0, &position, 0.0f);
            if ((move->time += 1.0f) > move->duration) {
                obj->work[0] = 1;
            }

            move = &lbl_1_bss_D9C[i + 4];
            fn_1_1F948(&position, &move->current, &move->middle,
                &move->target, fn_1_1F878(0.0f,
                    1.0f, move->time, move->duration));
            fn_1_26CF8((s16)(i + 4), &position, 0.0f);
            if ((move->time += 1.0f) > move->duration) {
                obj->work[0] = 1;
            }
        }
        return;
    }

    if (obj->work[0] == 1) {
        for (i = 0; i < 4; i++) {
            particle = &lbl_1_bss_F9C[i];
            move = &lbl_1_bss_D9C[i];
            particle->current.x = move->target.x;
            particle->current.y = move->target.y;
            particle->current.z = move->target.z;
            particle = &lbl_1_bss_F9C[i];
            move = &lbl_1_bss_D9C[i + 4];
            particle->middle.x = move->target.x;
            particle->middle.y = move->target.y;
            particle->middle.z = move->target.z;
        }
        obj->work[0] = 2;
        return;
    }

    for (i = 0; i < 4; i++) {
        move = &lbl_1_bss_F9C[i];
        Hu3DModelPosGet(obj->mdlId[i], &position);

        position.x += 80.0 * sin(
            3.141592653589793 * move->values[0] / 180.0);
        position.y += 80.0f + fn_1_1FF48(0.0f,
            80.0f, move->values[2], 100.0f);
        position.z += 80.0 * cos(
            3.141592653589793 * move->values[0] / 180.0);
        move->current.x = fn_1_1F8BC(move->current.x,
            position.x, move->target.x);
        move->current.y = fn_1_1F8BC(move->current.y,
            position.y, move->target.x);
        move->current.z = fn_1_1F8BC(move->current.z,
            position.z, move->target.x);
        fn_1_26CF8(i, &move->current, move->target.y);
        move->values[0] += 10.0f;
        if (move->values[0] > 360.0f) {
            move->values[0] -= 360.0f;
        }

        Hu3DModelPosGet(obj->mdlId[i], &position);
        position.x += 100.0 * sin(
            3.141592653589793 * move->values[1] / 180.0);
        position.y += 80.0f + fn_1_1FF48(0.0f,
            80.0f, move->values[2], 100.0f);
        position.z += 100.0 * cos(
            3.141592653589793 * move->values[1] / 180.0);
        move->middle.x = fn_1_1F8BC(move->middle.x,
            position.x, move->target.x);
        move->middle.y = fn_1_1F8BC(move->middle.y,
            position.y, move->target.x);
        move->middle.z = fn_1_1F8BC(move->middle.z,
            position.z, move->target.x);
        fn_1_26CF8((s16)(i + 4), &move->middle,
            move->target.y);

        move->values[1] += 10.0f;
        if (move->values[1] > 360.0f) {
            move->values[1] -= 360.0f;
        }
        move->values[2] += 1.0f;
        if (move->values[2] > 100.0f) {
            move->values[2] -= 100.0f;
        }
        move->target.x -= 1.0f;
        if (move->target.x < 2.0f) {
            move->target.x = 2.0f;
        }
    }
}

void fn_1_3E98(void)
{
    MDRESULT_MOVE_WORK *work;
    s16 i;

    for (i = 0; i < 4; i++) {
        work = &lbl_1_bss_F9C[i];
        work->target.y -= 1.0f;
        if (work->target.y < -20.0f) {
            work->target.y = -20.0f;
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
        fn_1_26CF8(i + 0, &position, 0.0f);

        move = &lbl_1_bss_D9C[i + 4];
        Hu3DModelObjMtxGet(lbl_1_bss_8->mdlId[0], lbl_1_data_666,
            matrix);
        position.x = matrix[0][3];
        position.y = matrix[1][3];
        position.z = matrix[2][3];
        fn_1_26CF8((s16)(i + 4), &position, 0.0f);
    }

    if (++obj->work[1] <= 30) {
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
    MDRESULT_MOVE_WORK *moveWork;
    Mtx matrix;
    HuVecF hookPos;
    GXColor colors[2] = {
        { 255, 114, 46, 0 },
        { 109, 207, 246, 0 }
    };
    s16 i;

    obj->work[0] = 0;
    obj->work[1] = 0;
    for (i = 0; i < 4; i++) {
        moveWork = &lbl_1_bss_F9C[i];
        lbl_1_bss_F9C[i].values[0] = 0.0f;
        lbl_1_bss_F9C[i].values[1] = 180.0f;
        lbl_1_bss_F9C[i].values[2] = 75.0f;
        moveWork->target.x = 5.0f;
        moveWork->target.y = 0.0f;

        Hu3DModelObjMtxGet(lbl_1_bss_4->mdlId[0], lbl_1_data_654,
            matrix);
        Hu3DModelPosGet(lbl_1_bss_4->mdlId[0], &moveWork->current);
        hookPos.x = matrix[0][3];
        hookPos.y = matrix[1][3];
        hookPos.z = matrix[2][3];
        fn_1_2668C(i, 50, &hookPos, 20.0f, &colors[0].r);

        Hu3DModelPosGet(lbl_1_bss_8->mdlId[0], &moveWork->middle);
        Hu3DModelObjMtxGet(lbl_1_bss_8->mdlId[0], lbl_1_data_666,
            matrix);
        hookPos.x = matrix[0][3];
        hookPos.y = matrix[1][3];
        hookPos.z = matrix[2][3];
        fn_1_2668C((s16)(i + 4), 50, &hookPos, 20.0f,
            &colors[1].r);
    }

    for (i = 0; i < 4; i++) {
        moveWork = &lbl_1_bss_D9C[i];
        Hu3DModelPosGet(lbl_1_bss_4->mdlId[0], &moveWork->current);
        Hu3DModelPosGet(lbl_1_bss_C->mdlId[i], &moveWork->middle);
        Hu3DModelPosGet(lbl_1_bss_C->mdlId[i], &moveWork->target);
        moveWork->current.x -= 50.0f;
        moveWork->current.y = 100.0f;
        moveWork->middle.x = 0.0f;
        moveWork->middle.y = 200.0f;
        moveWork->middle.z = 500.0f;
        moveWork->target.y = 100.0f;
        moveWork->time = 0.0f;
        moveWork->duration = 30.0f;

        moveWork = &lbl_1_bss_D9C[i + 4];
        Hu3DModelPosGet(lbl_1_bss_8->mdlId[0], &moveWork->current);
        Hu3DModelPosGet(lbl_1_bss_C->mdlId[i], &moveWork->middle);
        Hu3DModelPosGet(lbl_1_bss_C->mdlId[i], &moveWork->target);
        moveWork->current.x -= 50.0f;
        moveWork->current.y = 100.0f;
        moveWork->middle.x = 0.0f;
        moveWork->middle.y = 200.0f;
        moveWork->middle.z = 500.0f;
        moveWork->target.y = 100.0f;
        moveWork->time = 0.0f;
        moveWork->duration = 30.0f;
    }

    for (i = 0; i < 4; i++) {
        Hu3DModelLayerSet(lbl_1_bss_C->mdlId[i], 2);
    }
    Hu3DModelLayerSet(lbl_1_bss_4->mdlId[0], 2);
    Hu3DModelLayerSet(lbl_1_bss_8->mdlId[0], 2);
    Hu3DMotionShiftSet(lbl_1_bss_4->mdlId[0], lbl_1_bss_4->mtnId[2],
        0.0f, 10.0f, 0);
    Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[0], lbl_1_bss_8->mtnId[2],
        0.0f, 10.0f, 0);
    obj->objFunc = fn_1_3F20;
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
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i], 0.0f,
            0.0f, HU3D_MOTATTR_LOOP);
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
    Hu3DMotionSpeedSet(obj->mdlId[0], 2.0f);
    if (obj->work[3]++ > 30) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            0.0f, 15.0f,
            HU3D_MOTATTR_LOOP);
    }
}

void fn_1_4B44(void)
{
    OMOBJ *obj = lbl_1_bss_4;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        0.0f, 10.0f, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_4A9C;
}

void fn_1_4BB8(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(obj->mdlId[0], 2.0f);
    if (obj->work[3]++ > 30) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            0.0f, 15.0f,
            HU3D_MOTATTR_LOOP);
    }
}

void fn_1_4C60(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        0.0f, 10.0f, 0);
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
        0.0f, 0.0f,
        HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(obj->mdlId[0]);
    Hu3DModelPosSet(obj->mdlId[0], -325.0f,
        0.0f, 100.0f);
    Hu3DModelRotSet(obj->mdlId[0], 0.0f,
        15.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 1.25f,
        1.25f, 1.25f);
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
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f,
        0.0f, HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(obj->mdlId[0]);
    Hu3DModelPosSet(obj->mdlId[0], 325.0f,
        0.0f, 100.0f);
    Hu3DModelRotSet(obj->mdlId[0], 0.0f, -15.0f,
        0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 1.25f,
        1.25f, 1.25f);
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
            0.0f, 0.0f,
            HU3D_MOTATTR_LOOP);
        Hu3DModelShadowMapSet(obj->mdlId[i]);
    }
    Hu3DModelPosSet(obj->mdlId[1], 0.0f, -600.0f,
        0.0f);
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
        work->time = 0.0f;
        work->angle = frandmod(180) + 240;
    }
}

void fn_1_5484(OMOBJ *obj)
{
    HuVecF position;
    s16 modelIndex;
    MDRESULT_MODEL_EFFECT_WORK *work;

    modelIndex = 0;
    work = &lbl_1_bss_ADC[0];
    Hu3DModelPosGet(obj->mdlId[modelIndex], &position);
    position.y = fn_1_1FE74(-115.0f, 100.0f,
        work->time, 180.0f);
    Hu3DModelPosSetV(obj->mdlId[modelIndex], &position);
    if ((work->time += 1.0f) > work->angle) {
        work->time = 0.0f;
        work->angle = frandmod(180) + 240;
    }
    modelIndex = 2;
    Hu3DModelRotGet(obj->mdlId[modelIndex], &position);
    position.z += 1.0f;
    if (position.z >= 360.0f) {
        position.z -= 360.0f;
    }
    Hu3DModelRotSetV(obj->mdlId[modelIndex], &position);
    modelIndex = 1;
    Hu3DModelRotGet(obj->mdlId[modelIndex], &position);
    position.z -= 1.0f;
    if (position.z <= 0.0f) {
        position.z += 360.0f;
    }
    Hu3DModelRotSetV(obj->mdlId[modelIndex], &position);
}

void fn_1_5690(OMOBJ *obj)
{
    HuVecF position;
    s16 i;

    for (i = 0; i < 5; i++) {
        position.y = position.z = position.x =
            (frandmod(2) + 8) * 0.1f;
        Hu3DModelScaleSetV(obj->mdlId[i + 3], &position);
        position.x = frandmod(2000) - 1000;
        position.y = frandmod(1000) + 1000;
        if (i == 2 || i == 3) {
            position.z = -1000 - frandmod(750);
        } else {
            position.z = -1000 - frandmod(250);
        }
        Hu3DModelPosSetV(obj->mdlId[i + 3], &position);
    }
    lbl_1_bss_44 = 0.0f;
}

void fn_1_5860(OMOBJ *obj)
{
    HuVecF position;
    s16 i;

    for (i = 0; i < 5; i++) {
        Hu3DModelPosGet(obj->mdlId[i + 3], &position);
        position.y += lbl_1_bss_44;
        if (position.y < -1000.0f) {
            position.y = position.z = position.x =
                (frandmod(2) + 8) * 0.1f;
            Hu3DModelScaleSetV(obj->mdlId[i + 3], &position);
            position.x = frandmod(2000) - 1000;
            position.y = rand8() + 1000;
            if (i == 2 || i == 3) {
                position.z = -1500 - frandmod(250);
            } else {
                position.z = -1000 - frandmod(250);
            }
        }
        Hu3DModelPosSetV(obj->mdlId[i + 3], &position);
    }
}

void fn_1_5A60(float value)
{
    lbl_1_bss_44 = value;
}

void fn_1_5A70(OMOBJ *obj, s16 index)
{
    MDRESULT_MODEL_EFFECT_WORK *work;

    work = &lbl_1_bss_ADC[index + 8];
    work->state = rand8() % 2;
    work->unk_0C = (frandmod(10) - 5) * 0.2f;
    work->unk_10 = (frandmod(10) - 5) * 0.2f;
    work->unk_14 = (frandmod(10) - 5) * 0.2f;
    if (work->state == 0) {
        Hu3DModelPosSet(obj->mdlId[index + 8], -1000.0f,
            frandmod(200) + 400,
            -1250 - (index * 200));
        work->unk_18 = frandmod(3) + 1;
        work->unk_1C = (frandmod(10) - 5) * 0.01f;
        work->unk_20 = 0.0f;
    } else {
        Hu3DModelPosSet(obj->mdlId[index + 8], 1000.0f,
            frandmod(200) + 400,
            -1250 - (index * 200));
        work->unk_18 = -(frandmod(3) + 1);
        work->unk_1C = (frandmod(10) - 5) * 0.1f;
        work->unk_20 = 0.0f;
    }
    work->unk_24 = frandmod(1000) + 1500.0f;
}

void fn_1_5E18(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 3; i++) {
        fn_1_5A70(obj, i);
        Hu3DModelPosSet(obj->mdlId[i + 8], frandmod(100) - 500,
            frandmod(200) + 400, -1250 - (i * 200));
    }
}

void fn_1_6290(OMOBJ *obj)
{
    HuVecF position;
    MDRESULT_MODEL_EFFECT_WORK *work;
    s16 i;

    for (i = 0; i < 3; i++) {
        work = &lbl_1_bss_ADC[i + 8];
        Hu3DModelPosGet(obj->mdlId[i + 8], &position);
        position.x += work->unk_18;
        position.y += work->unk_1C + lbl_1_bss_44;
        position.z += work->unk_20;
        Hu3DModelPosSetV(obj->mdlId[i + 8], &position);

        if (work->state == 0) {
            if (position.x > work->unk_24) {
                MDRESULT_MODEL_EFFECT_WORK *resetWork;

                resetWork = &lbl_1_bss_ADC[i + 8];
                resetWork->state = rand8() % 2;
                resetWork->unk_0C = (frandmod(10) - 5) * 0.2f;
                resetWork->unk_10 = (frandmod(10) - 5) * 0.2f;
                resetWork->unk_14 = (frandmod(10) - 5) * 0.2f;
                if (resetWork->state == 0) {
                    Hu3DModelPosSet(obj->mdlId[i + 8],
                        -1000.0f, frandmod(200) + 400,
                        -1250 - (i * 200));
                    resetWork->unk_18 = frandmod(3) + 1;
                    resetWork->unk_1C = (frandmod(10) - 5) * 0.01f;
                    resetWork->unk_20 = 0.0f;
                } else {
                    Hu3DModelPosSet(obj->mdlId[i + 8],
                        1000.0f, frandmod(200) + 400,
                        -1250 - (i * 200));
                    resetWork->unk_18 = -(frandmod(3) + 1);
                    resetWork->unk_1C = (frandmod(10) - 5) * 0.1f;
                    resetWork->unk_20 = 0.0f;
                }
                resetWork->unk_24 = frandmod(1000) + 1500.0f;
                if (i == 1) {
                    Hu3DMotionShiftSet(obj->mdlId[9],
                        obj->mtnId[rand8() % 3], 0.0f,
                        0.0f, HU3D_MOTATTR_LOOP);
                } else if (i == 2) {
                    Hu3DMotionShiftSet(obj->mdlId[10],
                        obj->mtnId[(rand8() % 3) + 3], 0.0f,
                        0.0f, HU3D_MOTATTR_LOOP);
                }
            }
        } else {
            if (position.x < -work->unk_24) {
                MDRESULT_MODEL_EFFECT_WORK *resetWork;

                resetWork = &lbl_1_bss_ADC[i + 8];
                resetWork->state = rand8() % 2;
                resetWork->unk_0C = (frandmod(10) - 5) * 0.2f;
                resetWork->unk_10 = (frandmod(10) - 5) * 0.2f;
                resetWork->unk_14 = (frandmod(10) - 5) * 0.2f;
                if (resetWork->state == 0) {
                    Hu3DModelPosSet(obj->mdlId[i + 8],
                        -1000.0f, frandmod(200) + 400,
                        -1250 - (i * 200));
                    resetWork->unk_18 = frandmod(3) + 1;
                    resetWork->unk_1C = (frandmod(10) - 5) * 0.01f;
                    resetWork->unk_20 = 0.0f;
                } else {
                    Hu3DModelPosSet(obj->mdlId[i + 8],
                        1000.0f, frandmod(200) + 400,
                        -1250 - (i * 200));
                    resetWork->unk_18 = -(frandmod(3) + 1);
                    resetWork->unk_1C = (frandmod(10) - 5) * 0.1f;
                    resetWork->unk_20 = 0.0f;
                }
                resetWork->unk_24 = frandmod(1000) + 1500.0f;
                if (i == 1) {
                    Hu3DMotionShiftSet(obj->mdlId[9],
                        obj->mtnId[rand8() % 3], 0.0f,
                        0.0f, HU3D_MOTATTR_LOOP);
                } else if (i == 2) {
                    Hu3DMotionShiftSet(obj->mdlId[10],
                        obj->mtnId[(rand8() % 3) + 3], 0.0f,
                        0.0f, HU3D_MOTATTR_LOOP);
                }
            }
        }

        Hu3DModelRotGet(obj->mdlId[i + 8], &position);
        position.x += work->unk_0C;
        position.y += work->unk_10;
        position.z += work->unk_14;
        Hu3DModelRotSetV(obj->mdlId[i + 8], &position);
    }
}

void fn_1_6C7C(OMOBJ *obj)
{
    if (obj->work[0] == 0) {
        fn_1_6290(obj);
    }
}

void fn_1_6CB8(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    i = 1;
    obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 15), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
    Hu3DModelPosSet(obj->mdlId[i], 0.0f,
        50.0f, 100.0f);
    Hu3DModelLayerSet(obj->mdlId[i], 1);
    Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
        0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);


    for (i = 0; i < 3; i++) {
        obj->mdlId[i + 2] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 2) + i, HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 2] = Hu3DMotionIDGet(obj->mdlId[i + 2]);
        Hu3DModelLayerSet(obj->mdlId[i + 2], 1);
        if (i == 0) {
            Hu3DMotionShiftSet(obj->mdlId[i + 2], obj->mtnId[i + 2],
                0.0f, 0.0f, 0);
        } else {
            Hu3DMotionShiftSet(obj->mdlId[i + 2], obj->mtnId[i + 2],
                0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        }
    }

    for (i = 0; i < 3; i++) {
        obj->mdlId[i + 8] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 5) + i, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelScaleSet(obj->mdlId[i + 8], 0.5f,
            0.5f, 0.5f);
        Hu3DModelLayerSet(obj->mdlId[i + 8], 1);
    }


    fn_1_5E18(obj);

    for (i = 0; i < 3; i++) {
        obj->mtnId[i] = Hu3DJointMotion(obj->mdlId[9],
            HuDataSelHeapReadNum(DATANUM(DATA_mdpresult, 8) + i,
                HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 3] = Hu3DJointMotion(obj->mdlId[10],
            HuDataSelHeapReadNum(DATANUM(DATA_mdpresult, 11) + i,
                HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DMotionShiftSet(obj->mdlId[9], obj->mtnId[0],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DMotionShiftSet(obj->mdlId[10], obj->mtnId[3],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    }
    obj->work[0] = 0;
    obj->objFunc = fn_1_6C7C;
}

void fn_1_7518(OMOBJ *obj)
{
    if (obj) {
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_7560(s16 state, u8 flag)
{
    OMOBJ *obj = lbl_1_bss_28;

    obj->work[0] = state;
    obj->work[1] = flag;
}

void fn_1_7590(OMOBJ *obj)
{
    s16 i;
    s16 j;
    s16 paletteIndex;
    HU3D_MODEL *modelData = NULL;
    HSF_DATA *hsf = NULL;
    HSF_MATERIAL *material = NULL;
    MDRESULT_COLOR_TABLE_7 palette = {{
        { 255, 90, 90, 0 },
        { 255, 90, 255, 0 },
        { 100, 90, 255, 0 },
        { 90, 255, 255, 0 },
        { 90, 255, 90, 0 },
        { 255, 255, 90, 0 },
        { 255, 180, 90, 0 },
    }};

    switch (obj->work[0]) {
    case 0:
        for (i = 0; i < 4; i++) {
            lbl_1_bss_ABC[i].target[0] = 255;
            lbl_1_bss_ABC[i].target[1] = 255;
            lbl_1_bss_ABC[i].target[2] = 0;
            lbl_1_bss_ABC[i].target[3] = rand8() % 255;
        }
        obj->work[2] = 30;
        break;

    case 1:
        for (i = 0; i < 4; i++) {
            if (++lbl_1_bss_AAC[i].tick > 12) {
                lbl_1_bss_AAC[i].tick = 0;
                lbl_1_bss_AAC[i].paletteIndex = rand8() % 7;
            }
        }
        for (i = 0; i < 4; i++) {
            paletteIndex = lbl_1_bss_AAC[i].paletteIndex;
            lbl_1_bss_ABC[i].target[0] = palette.values[paletteIndex].r;
            lbl_1_bss_ABC[i].target[1] = palette.values[paletteIndex].g;
            lbl_1_bss_ABC[i].target[2] = palette.values[paletteIndex].b;
            lbl_1_bss_ABC[i].target[3] = rand8() % 255;
        }
        obj->work[2] = 2;
        break;

    case 2:
        for (i = 0; i < 4; i++) {
            if ((obj->work[1] & (1 << i)) == 0) {
                lbl_1_bss_ABC[i].target[0] = 255;
                lbl_1_bss_ABC[i].target[1] = 255;
                lbl_1_bss_ABC[i].target[2] = 0;
                lbl_1_bss_ABC[i].target[3] = 1;
            } else {
                lbl_1_bss_ABC[i].target[0] = 255;
                lbl_1_bss_ABC[i].target[1] = 255;
                lbl_1_bss_ABC[i].target[2] = 255;
                lbl_1_bss_ABC[i].target[3] = 255;
            }
        }
        obj->work[2] = 4;
        break;

    case 3:
        for (i = 0; i < 4; i++) {
            lbl_1_bss_ABC[i].target[0] = 255;
            lbl_1_bss_ABC[i].target[1] = 255;
            lbl_1_bss_ABC[i].target[2] = 0;
            lbl_1_bss_ABC[i].target[3] = 1;
        }
        obj->work[2] = 8;
        break;

    default:
        break;
    }

    for (i = 0; i < 4; i++) {
        modelData = &Hu3DData[obj->mdlId[i]];
        hsf = modelData->hsf;
        material = hsf->material;


        lbl_1_bss_ABC[i].current[0] = (u8)fn_1_1F8BC(
            (float)lbl_1_bss_ABC[i].current[0],
            (float)lbl_1_bss_ABC[i].target[0],
            (float)obj->work[2]);
        lbl_1_bss_ABC[i].current[1] = (u8)fn_1_1F8BC(
            (float)lbl_1_bss_ABC[i].current[1],
            (float)lbl_1_bss_ABC[i].target[1],
            (float)obj->work[2]);
        lbl_1_bss_ABC[i].current[2] = (u8)fn_1_1F8BC(
            (float)lbl_1_bss_ABC[i].current[2],
            (float)lbl_1_bss_ABC[i].target[2],
            (float)obj->work[2]);
        lbl_1_bss_ABC[i].current[3] = (u8)fn_1_1F8BC(
            (float)lbl_1_bss_ABC[i].current[3],
            (float)lbl_1_bss_ABC[i].target[3],
            (float)obj->work[2]);

        for (j = 0; j < hsf->materialNum; j++, material++) {
            if (j == 2 || j == 3 || j == 5) {
                material->litColor[0] = lbl_1_bss_ABC[i].current[0];
                material->litColor[1] = lbl_1_bss_ABC[i].current[1];
                material->litColor[2] = lbl_1_bss_ABC[i].current[2];
                material->color[0] = lbl_1_bss_ABC[i].current[0];
                material->color[1] = lbl_1_bss_ABC[i].current[1];
                material->color[2] = lbl_1_bss_ABC[i].current[2];
                material->shadowColor[0] = lbl_1_bss_ABC[i].current[0];
                material->shadowColor[1] = lbl_1_bss_ABC[i].current[1];
                material->shadowColor[2] = lbl_1_bss_ABC[i].current[2];
                if (j == 3 || j == 5) {
                    material->invAlpha = (float)lbl_1_bss_ABC[i].current[3];
                }
            }
        }
    }

    for (i = 0; i < 4; i++) {
        HuVecF position;

        Hu3DModelPosGet(obj->mdlId[i], &position);
        if (lbl_1_bss_1278.values[3] == 0) {
            position.x = lbl_1_data_0[i + 8].x;
        } else {
            position.x = lbl_1_data_0[i + 12].x;
        }
        position.y += 140.0f;
        position.z = 0.0f;
        if (lbl_1_bss_1278.values[3] == 0) {
            fn_1_25DB0(i, &position,
                (float)lbl_1_bss_ABC[i].current[3]
                    / 255.0f);
        } else if (i == 0 || i == 1) {
            fn_1_25DB0(i, &position,
                (float)lbl_1_bss_ABC[0].current[3]
                    / 255.0f);
        } else {
            fn_1_25DB0(i, &position,
                (float)lbl_1_bss_ABC[1].current[3]
                    / 255.0f);
        }
    }
}

void fn_1_8184(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 4; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 14), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    if (lbl_1_bss_1278.values[3] == 0) {
        for (i = 0; i < 4; i++) {
            Hu3DModelPosSetV(obj->mdlId[i], &lbl_1_data_0[i]);
            Hu3DModelScaleSet(obj->mdlId[i], 1.0f,
                1.0f, 1.0f);
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        }
    } else {
        for (i = 0; i < 2; i++) {
            Hu3DModelPosSetV(obj->mdlId[i], &lbl_1_data_0[i + 4]);
            Hu3DModelScaleSet(obj->mdlId[i], 2.0f,
                1.0f, 2.0f);
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        }
    }
    obj->objFunc = fn_1_7590;
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

void fn_1_8470(OMOBJ *obj)
{
    s16 offsets[3] = { -20, 0, 20 };
    s16 j;
    s16 base;
    s16 count;
    s16 value;
    s16 i;
    HuVecF position;
    HuVecF screen;

    if (lbl_1_bss_1278.values[3] == 0) {
        base = 0;
        count = 4;
        value = (s16)obj->work[3];
    } else {
        base = 1;
        count = 2;
        value = (s16)obj->work[3];
    }

    switch ((s32)obj->work[2]) {
    case 0:
        for (i = 0; i < count; i++) {
            Hu3DModelPosGet(obj->mdlId[i + (4 * value)], &position);
            position.y = fn_1_1FC94(0.0f, 325.0f,
                (float)obj->work[0], (float)obj->work[1]);
            Hu3DModelPosSetV(obj->mdlId[i + (4 * value)], &position);
            Hu3DModelRotGet(obj->mdlId[i + (4 * value)], &position);
            position.y = fn_1_1FC94(-1080.0f, 0.0f,
                (float)obj->work[0], (float)obj->work[1]);
            Hu3DModelRotSetV(obj->mdlId[i + (4 * value)], &position);
            if (value == 0) {
                position.x = position.y = position.z = fn_1_1FC94(
                    0.0f, 1.0f,
                    (float)obj->work[0], (float)obj->work[1]);
            } else {
                position.x = position.y = position.z = fn_1_1FC94(
                    0.0f, 0.9f,
                    (float)obj->work[0], (float)obj->work[1]);
            }
            Hu3DModelScaleSetV(obj->mdlId[i + (4 * value)], &position);
        }
        if (++obj->work[0] > obj->work[1]) {
            obj->work[0] = 0;
            obj->work[1] = 10;
            obj->work[2] = 1;
            for (i = 0; i < count; i++) {
                fn_1_25FF4(i);
                Hu3DModelPosGet(obj->mdlId[i + (4 * value)], &position);
                Hu3D3Dto2D(&position, 1, &screen);
                HuSprGrpPosSet(lbl_1_bss_11A0[i], screen.x, screen.y);
            }
        }
        break;
    case 1:
        for (i = 0; i < count; i++) {
            Hu3DModelPosGet(obj->mdlId[i + (4 * value)], &position);
            position.x = fn_1_1FC94(
                lbl_1_data_0[i + (4 * base)].x,
                lbl_1_data_0[i + (4 * base)].x,
                (float)obj->work[0], (float)obj->work[1]);
            position.y = fn_1_1FC94(325.0f, 345.0f,
                (float)obj->work[0], (float)obj->work[1]);
            Hu3DModelPosSetV(obj->mdlId[i + (4 * value)], &position);
            for (j = 0; j < 3; j++) {
                position.x = fn_1_1FC94(0.0f,
                    (float)offsets[j], (float)obj->work[0],
                    (float)obj->work[1]);
                position.y = fn_1_1FC94(0.0f,
                    30.0f, (float)obj->work[0],
                    (float)obj->work[1]);
                HuSprPosSet(lbl_1_bss_11A0[i], j,
                    position.x, position.y);
            }
        }
        if (++obj->work[0] > obj->work[1]) {
            obj->objFunc = NULL;
        }
        break;
    }

    for (i = 0; i < count; i++) {
        Hu3DModelPosGet(obj->mdlId[i + (4 * value)], &position);
        fn_1_26070(i, -1, &position, -1.0f, NULL);
    }
}

void fn_1_8B70(s32 value)
{
    OMOBJ *obj = lbl_1_bss_18;
    HuVecF position;
    s16 star[4];
    s16 coin[4];
    s16 base;
    s16 count;
    s16 i;
    s16 j;

    if (lbl_1_bss_1278.values[3] == 0) {
        base = 0;
        count = 4;
    } else {
        base = 1;
        count = 2;
    }
    for (i = 0; i < count; i++) {
        star[i] = lbl_1_bss_10D4[i].star;
        coin[i] = lbl_1_bss_10D4[i].coin;
    }
    obj->work[0] = 0;
    obj->work[1] = 50;
    obj->work[2] = 0;
    obj->work[3] = (s16)value;

    for (i = 0; i < count; i++) {
        Hu3DModelPosSet(obj->mdlId[i + (4 * (s16)value)],
            lbl_1_data_0[i + (4 * base)].x, 0.0f,
            lbl_1_data_0[i + (4 * base)].z - 20.0f);
        Hu3DModelScaleSet(obj->mdlId[i + (4 * (s16)value)],
            0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(obj->mdlId[i + (4 * (s16)value)],
            HU3D_ATTR_DISPOFF);
        {
            GXColor color = { 255, 255, 255, 0 };
        Hu3DModelPosGet(obj->mdlId[i + (4 * (s16)value)], &position);
        fn_1_25E6C(i, 1, &position, 50.0f,
            (u8 *)&color);
        }
        for (j = 0; j < 3; j++) {
            HuSprPosSet(lbl_1_bss_11A0[i], j, 0.0f,
                0.0f);
        }
        HuSprGrpPosSet(lbl_1_bss_11A0[i], 0.0f,
            -1000.0f);
        HuSprGrpScaleSet(lbl_1_bss_11A0[i], 1.0f,
            1.0f);
        fn_1_20188(lbl_1_bss_11A0[i], 4);
        if ((s16)value == 0) {
            fn_1_20208(lbl_1_bss_11A0[i], 0, star[i]);
        } else {
            fn_1_20208(lbl_1_bss_11A0[i], 0, coin[i]);
        }
    }
    obj->objFunc = fn_1_8470;
}

void fn_1_8F28(OMOBJ *obj)
{
    s16 offsets[3] = { -20, 0, 20 };
    s16 base;
    s16 count;
    s16 value;
    s16 i;
    s16 j;
    HuVecF position;

    if (lbl_1_bss_1278.values[3] == 0) {
        base = 0;
        count = 4;
        value = (s16)obj->work[3];
    } else {
        base = 1;
        count = 2;
        value = (s16)obj->work[3];
    }

    switch ((s32)obj->work[2]) {
    case 0:
        for (i = 0; i < count; i++) {
            Hu3DModelPosGet(obj->mdlId[i + (4 * value)], &position);
            position.x = fn_1_1FD7C(
                lbl_1_data_0[i + (4 * base)].x,
                lbl_1_data_0[i + (4 * base)].x,
                (float)obj->work[0], (float)obj->work[1]);
            position.y = fn_1_1FC94(345.0f,
                325.0f, (float)obj->work[0],
                (float)obj->work[1]);
            Hu3DModelPosSetV(obj->mdlId[i + (4 * value)], &position);
            for (j = 0; j < 3; j++) {
                position.x = fn_1_1FD7C((float)offsets[j],
                    0.0f, (float)obj->work[0],
                    (float)obj->work[1]);
                position.y = fn_1_1FD7C(30.0f,
                    0.0f, (float)obj->work[0],
                    (float)obj->work[1]);
                HuSprPosSet(lbl_1_bss_11A0[i], j,
                    position.x, position.y);
            }
        }
        if (++obj->work[0] > obj->work[1]) {
            obj->work[0] = 0;
            obj->work[1] = 50;
            obj->work[2] = 1;
            for (i = 0; i < count; i++) {
                HuVecF effectPosition;

                GXColor color = { 255, 255, 255, 0 };
                Hu3DModelPosGet(obj->mdlId[i + (4 * value)], &effectPosition);
                fn_1_25E6C(i, 1, &effectPosition, 50.0f,
                    (u8 *)&color);
            }
            for (i = 0; i < count; i++) {
                fn_1_20108(lbl_1_bss_11A0[i], 4);
            }
        }
        break;
    case 1:
        for (i = 0; i < count; i++) {
            Hu3DModelPosGet(obj->mdlId[i + (4 * value)], &position);
            position.y = fn_1_1FD7C(325.0f,
                750.0f, (float)obj->work[0],
                (float)obj->work[1]);
            Hu3DModelPosSetV(obj->mdlId[i + (4 * value)], &position);
            Hu3DModelRotGet(obj->mdlId[i + (4 * value)], &position);
            position.y = fn_1_1FD7C(0.0f,
                1080.0f, (float)obj->work[0],
                (float)obj->work[1]);
            Hu3DModelRotSetV(obj->mdlId[i + (4 * value)], &position);
        }
        if (++obj->work[0] > obj->work[1]) {
            obj->objFunc = NULL;
            for (i = 0; i < count; i++) {
                Hu3DModelAttrSet(obj->mdlId[i + (4 * value)],
                    HU3D_ATTR_DISPOFF);
                fn_1_25FF4(i);
            }
        }
        break;
    }

    for (i = 0; i < count; i++) {
        Hu3DModelPosGet(obj->mdlId[i + (4 * value)], &position);
        fn_1_26070(i, -1, &position, -1.0f, NULL);
    }
}

void fn_1_95A4(void)
{
    OMOBJ *obj = lbl_1_bss_18;

    obj->work[0] = 0;
    obj->work[1] = 10;
    obj->work[2] = 0;
    obj->objFunc = fn_1_8F28;
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
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i], 0.0f,
            0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < 4; i++) {
        obj->mdlId[i + 4] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 80), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 4] = Hu3DMotionIDGet(obj->mdlId[i + 4]);
        Hu3DModelLayerSet(obj->mdlId[i + 4], 3);
        Hu3DMotionShiftSet(obj->mdlId[i + 4], obj->mtnId[i + 4],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i + 4], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < 4; i++) {
        HuSprGrpDrawNoSet(lbl_1_bss_11A0[i], 64);
        fn_1_20108(lbl_1_bss_11A0[i], HUSPR_ATTR_DISPOFF);
    }
    obj->objFunc = NULL;
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

void fn_1_9924(OMOBJ *obj)
{
    MDRESULT_MOVE_WORK *work;
    MDRESULT_MOVE_WORK *secondary;
    HuVecF position;
    HuVecF rotation;
    s16 i;

    work = &lbl_1_bss_8EC[obj->work[3]];
    switch (work->state) {
    case 0:
        fn_1_1F948(&position, &work->current, &work->middle,
            &work->target,
            fn_1_1F878(0.0f, 1.0f,
                work->time, work->duration));
        Hu3DModelPosSetV(obj->mdlId[obj->work[3]], &position);
        Hu3DModelRotSet(obj->mdlId[obj->work[3]], 0.0f,
            fn_1_1F878(work->values[0], 1080.0f,
                work->time, work->duration),
            0.0f);
        if ((work->time += 1.0f) > work->duration) {
            work->state = 1;
            work->time = 0.0f;
            work->duration = 60.0f;
            if (obj->work[2] == 0) {
                obj->objFunc = NULL;
            }
        }
        break;
    case 1:
        if ((work->time += 1.0f) > work->duration) {
            work->state = 2;
            work->time = 0.0f;
            work->duration = 150.0f;
            for (i = 0, secondary = &lbl_1_bss_8EC[3]; i < 4;
                i++, secondary++) {
                if (secondary->state != 0) {
                    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &position);
                    Hu3DModelPosSetV(obj->mdlId[i + 3], &position);
                    Hu3DModelScaleSet(obj->mdlId[i + 3], 0.5f,
                        0.5f, 0.5f);
                    Hu3DModelAttrReset(obj->mdlId[i + 3], 1);

                    {
                        GXColor color = { 255, 255, 255, 0 };
                        Hu3DModelPosGet(obj->mdlId[i + 3], &rotation);
                        fn_1_25E6C(i, 1, &rotation, 50.0f,
                            (u8 *)&color);
                    }
                    fn_1_1F868(&secondary->current, position.x, position.y,
                        position.z - 25.0f);
                }
            }
        }
        break;
    case 2:
        for (i = 0, secondary = &lbl_1_bss_8EC[3]; i < 4;
            i++, secondary++) {
            if (secondary->state != 0) {
                fn_1_1F948(&position, &secondary->current,
                    &secondary->middle, &secondary->target,
                    fn_1_1FC94(0.0f, 1.0f,
                        secondary->time,
                        secondary->duration - 30.0f));
                Hu3DModelPosSetV(obj->mdlId[i + 3], &position);
                Hu3DModelRotGet(obj->mdlId[i + 3], &position);
                position.y = fn_1_1F878(0.0f,
                    1440.0f, secondary->time,
                    secondary->duration - 40.0f);
                Hu3DModelRotSetV(obj->mdlId[i + 3], &position);
                Hu3DModelPosGet(obj->mdlId[i + 3], &position);
                fn_1_26070(i, -1, &position, -1.0f, NULL);

                if (secondary->time >
                    secondary->duration - 20.0f) {
                    position.x = position.y = position.z =
                        fn_1_1FC94(0.5f,
                            0.0f,
                            secondary->time -
                                (secondary->duration - 20.0f),
                            20.0f);
                    Hu3DModelScaleSetV(obj->mdlId[i + 3], &position);
                }
                if ((secondary->time += 1.0f) > secondary->duration) {
                    Hu3DModelAttrSet(obj->mdlId[i + 3], 1);
                    fn_1_25FF4(i);
                }
            }
        }
        if ((work->time += 1.0f) > work->duration) {
            obj->objFunc = NULL;
        }
        break;
    }

    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &position);
    fn_1_26070((s16)(obj->work[3] + 4), -1, &position,
        -1.0f, NULL);
}

void fn_1_9EBC(s16 count, u8 mask)
{
    OMOBJ *obj = lbl_1_bss_1C;
    MDRESULT_MOVE_WORK *work = &lbl_1_bss_8EC[obj->work[3]];
    HuVecF rotation;
    s16 slotCount = 4;
    float total = 0.0f;
    s16 i;

    if (lbl_1_bss_1278.values[3] != 0) {
        slotCount = 2;
    }
    work->state = 0;
    work->time = 0.0f;
    work->duration = 60.0f;
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
    fn_1_1F868(&work->middle, total, 2000.0f,
        -300.0f);
    fn_1_1F868(&work->target, total, 400.0f,
        -300.0f);
    Hu3DModelRotGet(obj->mdlId[obj->work[3]], &rotation);
    work->values[0] = rotation.y;

    if (obj->work[2] != 0) {
        for (i = 0, work = &lbl_1_bss_8EC[3]; i < slotCount;
            i++, work++) {
            if (mask & (1 << i)) {
                work->state = 1;
                work->time = 0.0f;
                work->duration = 120.0f;
                if (lbl_1_bss_1278.values[3] == 0) {
                    Hu3DModelPosGet(obj->mdlId[obj->work[3]],
                        &work->current);
                    fn_1_1F868(&work->middle, lbl_1_data_0[i].x,
                        800.0f, -200.0f);
                    fn_1_1F868(&work->target, lbl_1_data_0[i].x,
                        150.0f, -100.0f);
                } else {
                    Hu3DModelPosGet(obj->mdlId[obj->work[3]],
                        &work->current);
                    fn_1_1F868(&work->middle,
                        lbl_1_data_0[i + 4].x, 800.0f,
                        -250.0f);
                    fn_1_1F868(&work->target,
                        lbl_1_data_0[i + 4].x, 150.0f,
                        -150.0f);
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
    acceleration = -1.0f;
    switch (work->state) {
    case 0: {
        time = fn_1_1FC94(0.0f, 1.0f,
            work->time, work->duration);
        acceleration = 150.0f * time;
        fn_1_1F948(&rotation, &work->current, &work->middle,
            &work->target, time);
        Hu3DModelPosSetV(obj->mdlId[obj->work[3]], &rotation);
        Hu3DModelScaleSet(obj->mdlId[obj->work[3]],
            1.5f * time, 1.5f * time,
            1.5f * time);
        if ((work->time += 1.0f) > work->duration) {
            work->state = 1;
            work->time = 0.0f;
            work->duration = 60.0f;
        }
        break;
    }
    case 1: {
        time = fn_1_1FC94(0.0f, 30.0f,
            work->time, work->duration);
        acceleration = -1.0f;
        if ((work->time += 1.0f) > work->duration) {
            work->time = 0.0f;
            work->duration = 60.0f;
        }
        if ((obj->work[0] += 1) > 20) {
            obj->work[0] = 0;
            work->values[1] = lbl_1_data_0[rand8() % 4].x;
        }
        Hu3DModelPosGet(obj->mdlId[obj->work[3]], &rotation);
        target.x = work->values[1];
        target.y = 400.0f + time;
        target.z = -300.0f;
        fn_1_1FB50(&rotation, &target, 30.0f);
        Hu3DModelPosSetV(obj->mdlId[obj->work[3]], &rotation);
        Hu3DModelRotGet(obj->mdlId[obj->work[3]], &rotation);
        rotation.y -= 20.0f;
        if (rotation.y > 360.0f) {
            rotation.y -= 360.0f;
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

    work = &lbl_1_bss_8EC[index];
    obj->work[0] = 0;
    obj->work[3] = index;
    work->state = 0;
    work->time = 0.0f;
    work->duration = 60.0f;
    fn_1_1F868(&work->current, 0.0f, 400.0f,
        -900.0f);
    fn_1_1F868(&work->middle, 0.0f, 400.0f,
        -600.0f);
    fn_1_1F868(&work->target, 0.0f, 400.0f,
        -300.0f);
    Hu3DModelScaleSet(obj->mdlId[obj->work[3]], 0.0f,
        0.0f, 0.0f);
    Hu3DModelAttrReset(obj->mdlId[obj->work[3]], HU3D_ATTR_DISPOFF);

    {
        MDRESULT_U8_TABLE_12 color = {
            { 255, 0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0 }
        };
        HuVecF position;

        Hu3DModelPosGet(obj->mdlId[obj->work[3]], &position);
        fn_1_25E6C((s16)(obj->work[3] + 4), 2, &position,
            1.0f, &color.values[obj->work[3] * 4]);
    }
    lbl_1_bss_1298 = HuAudFXPlay(1172);
    obj->objFunc = fn_1_A2B4;
}

void fn_1_A85C(OMOBJ *obj)
{
    MDRESULT_MOVE_WORK *work = &lbl_1_bss_8EC[obj->work[3]];
    HuVecF position;

    fn_1_1F948(&position, &work->current, &work->middle, &work->target,
        fn_1_1FC94(0.0f, 1.0f, work->time,
            work->duration));
    Hu3DModelPosSetV(obj->mdlId[obj->work[3]], &position);
    if ((work->time += 1.0f) > work->duration) {
        Hu3DModelAttrSet(obj->mdlId[obj->work[3]], HU3D_ATTR_DISPOFF);
        obj->objFunc = NULL;
        fn_1_25FF4((s16)(obj->work[3] + 4));
    }
    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &position);
    fn_1_26070((s16)(obj->work[3] + 4), -1, &position,
        -1.0f, NULL);
}

void fn_1_A984(void)
{
    OMOBJ *obj = lbl_1_bss_1C;
    MDRESULT_MOVE_WORK *work = &lbl_1_bss_8EC[obj->work[3]];

    work->state = 0;
    work->time = 0.0f;
    work->duration = 60.0f;
    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &work->current);
    fn_1_1F868(&work->middle, 0.0f, 0.0f,
        600.0f);
    fn_1_1F868(&work->target, 0.0f, 500.0f,
        1000.0f);
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
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i], 0.0f,
            0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(obj->mdlId[i], 1.5f,
            1.5f, 1.5f);
    }
    for (i = 0; i < 4; i++) {
        obj->mdlId[i + 3] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 61), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 3] = Hu3DMotionIDGet(obj->mdlId[i + 3]);
        Hu3DModelLayerSet(obj->mdlId[i + 3], 1);
        Hu3DMotionShiftSet(obj->mdlId[i + 3], obj->mtnId[i + 3],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i + 3], HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(obj->mdlId[i + 3], 0.5f,
            0.5f, 0.5f);
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
    MDRESULT_VECTOR_PAIR positions = {
        { { -180.0f, 350.0f, 0.0f }, { 180.0f, 350.0f, 0.0f } }
    };
    s16 phase;
    s16 model;

    step += 2;
    phase = step / 10;
    model = step % 10;
    if (phase >= 1) {
        Hu3DModelPosSet(obj->mdlId[(player * 11) + 10],
            positions.values[player].x - 55.0f,
            positions.values[player].y, positions.values[player].z);
        Hu3DModelAttrReset(obj->mdlId[(player * 11) + 10],
            HU3D_ATTR_DISPOFF);
        Hu3DModelPosSet(obj->mdlId[(player * 11) + model],
            55.0f + positions.values[player].x,
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

void fn_1_B454(OMOBJ *obj, s16 index, HuVecF *pos)
{
    index--;
    lbl_1_bss_81C[index].active = 1;
    lbl_1_bss_81C[index].timer = 0.0f;
    lbl_1_bss_81C[index].scale = 30.0f;
    Hu3DModelPosSetV(obj->mdlId[index + 4], pos);
}

void fn_1_B510(OMOBJ *obj)
{
    HU3D_MODEL *model = NULL;
    HSF_DATA *hsf = NULL;
    HSF_OBJECT *object = NULL;
    HuVecF second;
    HuVecF first;
    HuVecF position;
    Mtx secondMatrix;
    Mtx firstMatrix;
    s16 i;
    s16 j;
    s16 k;

    for (i = 0; i < 9; i++) {
        if (lbl_1_bss_81C[i].active == 1) {
            MDRESULT_EMITTER_VERTEX *source;
            HuVecF *destination;
            float amount;
            float angle;

            model = &Hu3DData[obj->mdlId[i + 4]];
            hsf = model->hsf;
            object = hsf->object;
            Hu3DModelAttrReset(obj->mdlId[i + 4], HU3D_ATTR_DISPOFF);
            amount = fn_1_1FC94(0.0f, 1.0f,
                lbl_1_bss_81C[i].timer,
                lbl_1_bss_81C[i].scale);
            angle = 450.0f * amount;
            if (angle > 360.0f) {
                angle = 360.0f;
            }
            PSMTXRotRad(firstMatrix, 'y', 0.017453292f * angle);
            angle = (450.0f * amount) - 90.0f;
            if (angle < 0.0f) {
                angle = 0.0f;
            }
            PSMTXRotRad(secondMatrix, 'y', 0.017453292f * angle);

            amount = fn_1_1FE74(350.0f, 500.0f,
                lbl_1_bss_81C[i].timer,
                lbl_1_bss_81C[i].scale);
            Hu3DModelPosGet(obj->mdlId[i + 4], &position);
            position.y = amount;
            Hu3DModelPosSetV(obj->mdlId[i + 4], &position);

            if ((lbl_1_bss_81C[i].timer += 1.0f) > lbl_1_bss_81C[i].scale) {
                lbl_1_bss_81C[i].active = 0;
            }

            for (j = 0; j < hsf->objectNum; j++) {
                if (object->type == HSF_OBJ_MESH) {
                    source = lbl_1_bss_81C[i].data;
                    destination = object->mesh.vertex->data;
                    for (k = 0; k < object->mesh.vertex->count; k++, destination++,
                        source++) {
                        PSMTXMultVec(secondMatrix, &source->position, &second);
                        PSMTXMultVec(firstMatrix, &source->position, &first);
                        destination->x = second.x
                            + source->weight * (first.x - second.x);
                        destination->y = second.y
                            + source->weight * (first.y - second.y);
                        destination->z = second.z
                            + source->weight * (first.z - second.z);
                    }
                    DCStoreRangeNoSync(object->mesh.vertex->data,
                        object->mesh.vertex->count * sizeof(HuVecF));
                    break;
                }
            }
        }
    }
}

void fn_1_B8E8(OMOBJ *obj)
{
    HU3D_MODEL *model = NULL;
    HSF_DATA *hsf = NULL;
    HSF_OBJECT *object = NULL;
    s16 i;
    float minY;
    float maxY;

    for (i = 0; i < 9; i++) {
        s16 j;

        model = &Hu3DData[obj->mdlId[i + 4]];
        hsf = model->hsf;
        object = hsf->object;
        minY = 9999.0f;
        maxY = 0.0f;

        for (j = 0; j < hsf->objectNum; j++) {
            if (object->type == HSF_OBJ_MESH) {
                s16 k;
                s16 size;
                HuVecF *source;
                MDRESULT_EMITTER_VERTEX *buffer;
                MDRESULT_EMITTER_VERTEX *destination;
                float range;

                size = object->mesh.vertex->count * sizeof(MDRESULT_EMITTER_VERTEX);
                source = object->mesh.vertex->data;
                for (k = 0; k < object->mesh.vertex->count; k++, source++) {
                    if (source->y > maxY) {
                        maxY = source->y;
                    }
                    if (source->y < minY) {
                        minY = source->y;
                    }
                }
                range = maxY - minY;
                buffer = HuMemDirectMallocNum(HEAP_MODEL, size, HU_MEMNUM_OVL);
                destination = buffer;
                source = object->mesh.vertex->data;
                for (k = 0; k < object->mesh.vertex->count;
                    k++, source++, destination++) {
                    destination->position = *source;
                    destination->weight = (source->y - minY) / range;
                }
                lbl_1_bss_81C[i].data = buffer;
                break;
            }
        }
    }
    fn_1_B220();
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

void fn_1_BB60(OMOBJ *obj)
{
    s16 i;
    float time;
    HuVecF position;
    MDRESULT_STATE_WORK *state;
    MDRESULT_CHARACTER_WORK *character;

    for (i = 0, state = lbl_1_bss_8AC, character = lbl_1_bss_1248;
        i < 4; i++, state++, character++) {
        switch (state->state) {
        case 1:
            Hu3DModelPosGet(lbl_1_bss_C->mdlId[i], &position);
            position.y = 325.0f;
            Hu3DModelPosSet(obj->mdlId[i], position.x, position.y,
                position.z);
            if (state->time == 0.0f) {
                fn_1_26164(i, &position);
            }
            time = fn_1_1FC94(0.0f, 1.0f, state->time, state->delay);
            Hu3DModelScaleSet(obj->mdlId[i], time, time, time);
            Hu3DModelTPLvlSet(obj->mdlId[i], time);
            Hu3DModelAttrReset(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            if (++state->time > state->delay) {
                HuAudFXPlay(1007);
                lbl_1_bss_12A0[i] = HuAudFXPlay(1005);
                state->state = 2;
                state->time = 0.0f;
                state->delay = (float)((rand8() % 120) + 60);
            }
            break;

        case 2:
            if ((character->unk_04 == 0 &&
                (HuPadBtnDown[character->unk_0A] & PAD_BUTTON_A)) ||
                (character->unk_04 != 0 && ++state->time > state->delay)) {
                state->state = 3;
                state->time = 0.0f;
                state->delay = 27.0f;
                fn_1_3364(i, 1, 0.0f, HU3D_MOTATTR_NONE);
            }
            break;

        case 3:
            if (++state->time > state->delay) {
                HuAudFXStop(lbl_1_bss_12A0[i]);
                HuAudFXPlay(1008);
                state->state = 4;
                state->time = 0.0f;
                state->delay = 15.0f;
                Hu3DModelPosGet(obj->mdlId[i], &position);
                Hu3DMotionSpeedSet(obj->mdlId[i], 0.0f);
                Hu3DMotionTimeSet(obj->mdlId[i], 0.5f +
                    (float)lbl_1_bss_8AC[i].score);
                fn_1_B454(obj, lbl_1_bss_8AC[i].score + 1, &position);
            }
            break;

        case 4:
            time = fn_1_1FE74(0.0f, 1.0f, state->time, state->delay);
            Hu3DModelPosGet(obj->mdlId[i], &position);
            position.y = 325.0f + 50.0f * time;
            Hu3DModelPosSetV(obj->mdlId[i], &position);
            position.x = 1.0f + time;
            position.y = 1.0f - 0.5f * time;
            position.z = 1.0f + time;
            Hu3DModelScaleSetV(obj->mdlId[i], &position);
            if (++state->time > state->delay) {
                state->state = 5;
                state->time = 0.0f;
                state->delay = 10.0f;
            }
            break;

        case 5:
            Hu3DModelTPLvlSet(obj->mdlId[i],
                fn_1_1FC94(1.0f, 0.0f, state->time, state->delay));
            if (++state->time > state->delay) {
                state->state = 6;
                Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
                fn_1_3364(i, 0, 15.0f, HU3D_MOTATTR_LOOP);
            }
            break;
        }
    }
    fn_1_B510(obj);
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
            work->time = 0.0f;
            work->delay = 10.0f;
            Hu3DMotionSpeedSet(obj->mdlId[i], 1.0f);
            Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
                0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        }
    }
    obj->objFunc = fn_1_BB60;
}

void fn_1_C358(void)
{
    s16 i;
    OMOBJ *obj = lbl_1_bss_20;

    for (i = 0; i < 13; i++) {
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    fn_1_AFF4();
    obj->objFunc = NULL;
}

void fn_1_C414(void)
{
    HU3D_MODELID models[4];
    HuVecF position;
    float fade;
    s16 frame;
    s16 i;

    models[0] = lbl_1_bss_20->mdlId[lbl_1_bss_8AC[0].score + 4];
    models[1] = lbl_1_bss_20->mdlId[lbl_1_bss_8AC[1].score + 4];
    models[2] = lbl_1_bss_20->mdlId[lbl_1_bss_8AC[2].score + 4];
    models[3] = lbl_1_bss_20->mdlId[lbl_1_bss_8AC[3].score + 4];
    for (frame = 0; frame < 30; frame++) {
        HuPrcVSleep();
        fade = fn_1_1F878(1.0f, 0.0f,
            (float)frame, 30.0f);
        for (i = 0; i < 4; i++) {
            Hu3DModelTPLvlSet(models[i], fade);
            if ((i % 2) == 0) {
                Hu3DModelPosGet(models[i], &position);
                position.x += 2.0f;
                Hu3DModelPosSetV(models[i], &position);
            } else {
                Hu3DModelPosGet(models[i], &position);
                position.x -= 2.0f;
                Hu3DModelPosSetV(models[i], &position);
            }
            Hu3DModelRotGet(models[i], &position);
            position.y += 12.0f;
            Hu3DModelRotSetV(models[i], &position);
        }
    }
    fn_1_AD94(0, lbl_1_bss_8AC[0].score + lbl_1_bss_8AC[1].score);
    fn_1_AD94(1, lbl_1_bss_8AC[2].score + lbl_1_bss_8AC[3].score);
    HuPrcSleep(60);
}

s16 fn_1_C9A0(void)
{
    s16 bestScore = 0;
    s16 result = 0;
    s16 i;
    MDRESULT_STATE_WORK *work;

    for (;;) {
        HuPrcVSleep();
        i = 0;
        work = lbl_1_bss_8AC;
        for (; i < 4; i++, work++) {
            if (work->state != 0 && work->state != 6) {
                break;
            }
        }
        if (i == 4) {
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
        for (; i < 4; i++, work++) {
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

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 4; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 63), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < 9; i++) {
        obj->mdlId[i + 4] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 65) + i, HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 4] = Hu3DMotionIDGet(obj->mdlId[i + 4]);
        Hu3DModelLayerSet(obj->mdlId[i + 4], 1);
        Hu3DMotionSpeedSet(obj->mdlId[i + 4], 0.0f);
        Hu3DMotionTimeSet(obj->mdlId[i + 4], 0.5f);
        Hu3DModelAttrSet(obj->mdlId[i + 4], HU3D_ATTR_DISPOFF);
    }
    fn_1_B8E8(obj);
    obj->objFunc = NULL;
}

void fn_1_CD04(OMOBJ *obj)
{
    s16 j;

    if (obj) {
        fn_1_BACC();
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
    MDRESULT_MOVE_WORK *work;
    MDRESULT_CAMERA_WORK *camera;

    obj = lbl_1_bss_10;
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    pos.y -= 50.0f;
    if (pos.y < -2000.0f) {
        Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
        pos.y = -2000.0f;
    }
    Hu3DModelPosSetV(obj->mdlId[0], &pos);
    Hu3DTexScrollPosMoveSet(obj->work[1], 0.0f,
        -0.04f, 0.0f);

    obj = lbl_1_bss_14;
    obj->work[0] = 1;
    for (i = 2; i < 5; i++) {
        Hu3DModelPosGet(obj->mdlId[i], &pos);
        pos.y -= 50.0f;
        if (pos.y < -2000.0f) {
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            pos.y = -2000.0f;
        }
        Hu3DModelPosSetV(obj->mdlId[i], &pos);
    }
    for (i = 0; i < 3; i++) {
        Hu3DModelPosGet(obj->mdlId[i + 8], &pos);
        pos.y -= 50.0f;
        if (pos.y < -2000.0f) {
            Hu3DModelAttrSet(obj->mdlId[i + 8], HU3D_ATTR_DISPOFF);
            pos.y = -2000.0f;
        }
        Hu3DModelPosSetV(obj->mdlId[i + 8], &pos);
    }

    obj = lbl_1_bss_28;
    for (i = 0; i < 4; i++) {
        Hu3DModelPosGet(obj->mdlId[i], &pos);
        pos.y -= 50.0f;
        if (pos.y < -2000.0f) {
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            pos.y = -2000.0f;
        }
        Hu3DModelPosSetV(obj->mdlId[i], &pos);
    }

    obj = lbl_1_bss_4;
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    pos.y -= 50.0f;
    if (pos.y < -2000.0f) {
        pos.y = -2000.0f;
    }
    Hu3DModelPosSetV(obj->mdlId[0], &pos);

    obj = lbl_1_bss_8;
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    pos.y -= 50.0f;
    if (pos.y < -2000.0f) {
        pos.y = -2000.0f;
    }
    Hu3DModelPosSetV(obj->mdlId[0], &pos);

    fn_1_26EAC(-50.0f);
    lbl_1_bss_44 = -50.0f;
    for (j = 0; j < 4; j++) {
        work = &lbl_1_bss_F9C[j];
        work->target.y -= 1.0f;
        if (work->target.y < -20.0f) {
            work->target.y = -20.0f;
        }
    }
    fn_1_25D0C(-40.0f);
    camera = &lbl_1_bss_12BC;
    camera->mode = 6;
}

void fn_1_D30C(float value)
{
    OMOBJ *obj = lbl_1_bss_10;
    float weight = 1.0f - value;

    fn_1_26EAC(-50.0f * weight);
    Hu3DTexScrollPosMoveSet(obj->work[1], 0.0f,
        -0.04f * weight, 0.0f);
    fn_1_5A60(-50.0f * weight);
    fn_1_25D0C(-40.0f * weight);
    fn_1_1840(6.0f * weight);
}

void fn_1_D40C(void)
{
    OMOBJ *obj = lbl_1_bss_10;

    Hu3DTexScrollPosMoveSet(obj->work[1], 0.0f,
        0.0f, 0.0f);
    fn_1_26EAC(0.0f);
    fn_1_25D0C(0.0f);
}

void fn_1_D48C(OMOBJ *obj)
{
    MDRESULT_FLOAT_TABLE_8 orbitTable = {{
        270.0f, 0.0f, 180.0f, 90.0f,
        270.0f, 270.0f, 90.0f, 90.0f,
    }};
    MDRESULT_MOVE_WORK *move;
    HuVecF position;
    HuVecF target;
    s16 i;

    switch (obj->work[2]) {
    case 0:
        if (++obj->work[0] > obj->work[1]) {
            for (i = 0; i < 4; i++) {
                fn_1_3364(i, 2, 0.0f, 0);
            }
            obj->work[0] = 0;
            obj->work[1] = 10;
            obj->work[2] = 1;
        }
        break;
    case 1:
        if (++obj->work[0] > obj->work[1]) {
            obj->work[0] = 0;
            obj->work[1] = 0;
            obj->work[2] = 2;
        }
        break;
    case 2:
        fn_1_CE9C();
        for (i = 0, move = lbl_1_bss_71C; i < 4; i++, move++) {
            if (move->state == 0) {
                Hu3DModelPosGet(lbl_1_bss_C->mdlId[i], &position);
                target.x = 400.0 * sin((3.141592653589793 *
                    ((float)obj->work[0] + orbitTable.values[i +
                        lbl_1_bss_1278.values[3] * 4])) / 180.0);
                target.y = 125.0f + fn_1_1FF48(
                    0.0f, 25.0f,
                    move->values[2], move->values[3]);
                target.z = 1000.0 * cos((3.141592653589793 *
                    ((float)obj->work[0] + orbitTable.values[i +
                        lbl_1_bss_1278.values[3] * 4])) / 180.0)
                    - 500.0;
                if (lbl_1_bss_1278.values[3] != 0) {
                    if ((i % 2) == 0) {
                        target.x -= 70.0f;
                    } else {
                        target.x += 70.0f;
                    }
                }
                fn_1_1FB50(&position, &target, 30.0f);
                Hu3DModelPosSetV(lbl_1_bss_C->mdlId[i], &position);
                if ((lbl_1_data_684[i] % 2) == 0) {
                    position.y = fn_1_1F878(0.0f,
                        (float)(lbl_1_data_684[i] * 360),
                        move->values[2], move->values[3]);
                    Hu3DModelRotSet(lbl_1_bss_C->mdlId[i], 0.0f,
                        position.y, 0.0f);
                } else {
                    position.y = fn_1_1F878(0.0f,
                        (float)(lbl_1_data_684[i] * 360),
                        move->values[2], move->values[3]);
                    Hu3DModelRotSet(lbl_1_bss_C->mdlId[i], 0.0f,
                        position.y, 0.0f);
                }
                if ((move->values[2] += 1.0f) > move->values[3]) {
                    move->values[2] = 0.0f;
                    move->values[3] = 240.0f;
                    lbl_1_data_684[i] = rand8() % 4 + 1;
                }
                if (!move->values[0]) {
                    if ((move->time += 1.0f) > move->duration) {
                        move->values[2] = 0.0f;
                        move->values[3] = 30.0f;
                        move->state = 1;
                        fn_1_3364(i, 4, 5.0f, HU3D_MOTATTR_LOOP);
                    }
                }
            } else {
                if ((move->values[2] += 1.0f) > move->values[3]) {
                    Hu3DModelPosGet(lbl_1_bss_C->mdlId[i], &position);
                    position.y -= 20.0f;
                    if (position.y < -2000.0f) {
                        fn_1_3364(i, 0, 5.0f, HU3D_MOTATTR_LOOP);
                        position.y = -2000.0f;
                    }
                    Hu3DModelPosSetV(lbl_1_bss_C->mdlId[i], &position);
                }
            }
        }
        if ((float)++obj->work[0] > 360.0f) {
            obj->work[0] -= 360;
            obj->work[1] = 0;
        }
        break;
    }
}

void fn_1_DC38(s16 index)
{
    OMOBJ *obj = lbl_1_bss_2C;
    MDRESULT_MOVE_WORK *first;
    MDRESULT_MOVE_WORK *second;
    s16 durations[4] = {
        120, 380, 180, 120,
    };
    s16 i;

    obj->work[0] = 0;
    obj->work[1] = 90;
    obj->work[2] = 0;
    obj->work[3] = 0;
    if (lbl_1_bss_1278.values[3] == 0) {
        for (i = 0; i < 4; i++) {
            first = &lbl_1_bss_71C[i];
            if (i == index) {
                first->values[0] = 1.0f;
            } else {
                first->values[0] = 0.0f;
            }
            first->state = 0;
            first->time = 0.0f;
            first->duration = durations[lbl_1_bss_10D4[i].rank];
            first->values[2] = 0.0f;
            first->values[3] = 120.0f;
        }
    } else {
        for (i = 0; i < 2; i++) {
            first = &lbl_1_bss_71C[i * 2];
            second = &lbl_1_bss_71C[i * 2 + 1];
            if (i == index) {
                first->values[0] = second->values[0] = 1.0f;
            } else {
                first->values[0] = second->values[0] = 0.0f;
            }
            first->state = second->state = 0;
            first->time = second->time = 0.0f;
            first->duration = second->duration = 380.0f;
            first->values[2] = 0.0f;
            first->values[3] = 120.0f;
            second->values[2] = 0.0f;
            second->values[3] = 120.0f;
        }
    }
    fn_1_4124();
    HuAudFXPlay(1183);
    HuAudFXPlay(1184);
    obj->objFunc = fn_1_D48C;
}

void fn_1_DED4(OMOBJ *obj)
{
    MDRESULT_MOVE_WORK *move;
    HuVecF position;
    float time;
    s16 i;

    for (i = 0, move = lbl_1_bss_71C; i < 4; i++, move++) {
        if (move->state == 0) {
            continue;
        }
        position.x = fn_1_1FD7C(move->current.x, move->middle.x,
            move->time, move->duration);
        position.y = fn_1_1FD7C(move->current.y, move->middle.y,
            move->time, move->duration);
        position.z = fn_1_1FD7C(move->current.z, move->middle.z,
            move->time, move->duration);
        Hu3DModelPosSetV(lbl_1_bss_C->mdlId[i], &position);

        position.y = fn_1_1FD7C(move->values[1], -2160.0f,
            move->time, move->duration);
        Hu3DModelRotSet(lbl_1_bss_C->mdlId[i], 0.0f, position.y,
            0.0f);

        if ((move->time += 1.0f) > move->duration) {
            HuAudFXStop(lbl_1_bss_129C);
            Hu3DModelScaleSet(lbl_1_bss_C->mdlId[i], 1.0f,
                1.0f, 0.1f);
            fn_1_3364(i, 7, 10.0f, 0);
            move->state = 0;
            obj->objFunc = NULL;
        }
    }

    for (i = 0, move = lbl_1_bss_71C; i < 4; i++, move++) {
        if (move->state != 0) {
            break;
        }
    }

    time = fn_1_1F878(0.0f, 1.0f,
        (float)obj->work[0], 180.0f);
    fn_1_D30C(time);
    if (++obj->work[0] <= 180) {
        return;
    }

    fn_1_D40C();
    for (i = 0; i < 4; i++) {
        fn_1_26BE4(i);
        fn_1_26BE4((s16)(i + 4));
        lbl_1_bss_C->objFunc = NULL;
    }

    fn_1_1F868(&position, 0.0f, 250.0f,
        750.0f);
    for (i = 0; i < 2; i++) {
        fn_1_26164(i, &position);
    }
    fn_1_1F868(&position, 0.0f, 100.0f,
        0.0f);
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

void fn_1_E658(s16 index)
{
    MDRESULT_MOVE_WORK *first;
    MDRESULT_MOVE_WORK *second;
    OMOBJ *obj = lbl_1_bss_2C;
    MDRESULT_FLOAT_TABLE_11 values = { {
        150, 150, 150, 150, 150, 150, 150,
        175, 150, 160, 175,
    } };
    HuVecF rotation;
    s16 i;

    obj->work[0] = 0;
    obj->work[1] = 0;
    obj->work[2] = 0;
    obj->work[3] = 0;
    i = 0;
    first = lbl_1_bss_71C;
    for (; i < 4; i++, first++) {
        first->state = 0;
    }
    if (lbl_1_bss_1278.values[3] == 0) {
        first = &lbl_1_bss_71C[index];
        first->state = 1;
        first->time = 0.0f;
        first->duration = 180.0f;
        Hu3DModelPosGet(lbl_1_bss_C->mdlId[index], &first->current);
        fn_1_1F868(&first->middle, 0.0f,
            values.values[lbl_1_bss_1248[index].character],
            800.0f);
        Hu3DModelRotGet(lbl_1_bss_C->mdlId[index], &rotation);
        first->values[1] = rotation.y;
    } else {
        first = &lbl_1_bss_71C[index * 2];
        second = &lbl_1_bss_71C[index * 2 + 1];
        first->state = second->state = 1;
        first->time = second->time = 0.0f;
        first->duration = second->duration = 180.0f;

        Hu3DModelPosGet(lbl_1_bss_C->mdlId[index * 2], &first->current);
        fn_1_1F868(&first->middle, -40.0f,
            values.values[lbl_1_bss_1248[index * 2].character],
            800.0f);
        Hu3DModelRotGet(lbl_1_bss_C->mdlId[index * 2], &rotation);
        first->values[1] = rotation.y;

        Hu3DModelPosGet(lbl_1_bss_C->mdlId[index * 2 + 1],
            &second->current);
        fn_1_1F868(&second->middle, 40.0f,
            values.values[lbl_1_bss_1248[index * 2 + 1].character],
            760.0f);
        Hu3DModelRotGet(lbl_1_bss_C->mdlId[index * 2 + 1], &rotation);
        second->values[1] = rotation.y;
    }
    obj->objFunc = fn_1_DED4;
}

void fn_1_E9E8(void)
{
    fn_1_49C8(lbl_1_bss_C);
    fn_1_4E68(lbl_1_bss_4);
    fn_1_50C0(lbl_1_bss_8);
    fn_1_52C4(lbl_1_bss_10);
    fn_1_7518(lbl_1_bss_14);
    fn_1_83E0(lbl_1_bss_28);
    fn_1_9850(lbl_1_bss_18);
    fn_1_AD04(lbl_1_bss_1C);
    fn_1_CD04(lbl_1_bss_20);
    fn_1_B178(lbl_1_bss_24);
    fn_1_CE18(lbl_1_bss_2C);
    fn_1_3304(lbl_1_bss_30);
    lbl_1_bss_34->objFunc = NULL;
    lbl_1_bss_38->objFunc = NULL;
    lbl_1_bss_3C[0]->objFunc = NULL;
    fn_1_25B90();
    fn_1_2208();
    fn_1_1C34();
    fn_1_1AA4();
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

void fn_1_F1C4(void)
{
    s16 add;
    s16 total;
    s16 i;

    fn_1_17B10();
    fn_1_E9E8();
    OSReport(lbl_1_data_68C);
    total = 0;
    for (i = 0; i < 4; i++) {
        add = lbl_1_bss_10D4[i].star - lbl_1_bss_10D4[i].values[15];
        if (add < 0) {
            add = 0;
        }
        OSReport(lbl_1_data_6A2, add, lbl_1_bss_10D4[i].star,
            lbl_1_bss_10D4[i].values[15]);
        total += add;
    }
    OSReport(lbl_1_data_6D0, total);
    OSReport(lbl_1_data_6E0, lbl_1_bss_1278.values[0]);
    GWBoardPlayNumAdd(lbl_1_bss_1278.values[0], 1);
    if (lbl_1_bss_1278.values[3] == 0) {
        OSReport(lbl_1_data_6ED);
        for (i = 0; i < 4; i++) {
            OSReport(lbl_1_data_6F7, i, lbl_1_bss_1248[i].character,
                lbl_1_bss_10D4[i].rank);
            if (lbl_1_bss_10D4[i].rank == 0
                && lbl_1_bss_1248[i].unk_04 == 0) {
                GWCharPlayNumInc(lbl_1_bss_1248[i].character,
                    lbl_1_bss_1278.values[0]);
            }
        }
    } else {
        OSReport(lbl_1_data_703);
        for (i = 0; i < 2; i++) {
            OSReport(lbl_1_data_70A, i,
                lbl_1_bss_1248[i * 2].character,
                lbl_1_bss_1248[(i * 2) + 1].character,
                lbl_1_bss_10D4[i].rank);
            if (lbl_1_bss_10D4[i].rank == 0) {
                if (lbl_1_bss_1248[i * 2].unk_04 == 0) {
                    GWCharPlayNumInc(lbl_1_bss_1248[i * 2].character,
                        lbl_1_bss_1278.values[0]);
                }
                if (lbl_1_bss_1248[(i * 2) + 1].unk_04 == 0) {
                    GWCharPlayNumInc(
                        lbl_1_bss_1248[(i * 2) + 1].character,
                        lbl_1_bss_1278.values[0]);
                }
            }
        }
    }
    GWBankStarAdd((u16)total);
    SLSaveBoardEndExec();
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

void fn_1_F548(void)
{
    lbl_1_bss_0 = omInitObjMan(27, MDRESULT_OBJECT_MANAGER_PRIORITY);
    omGameSysInit(lbl_1_bss_0);

    fn_1_1930((MDRESULT_CAMERA_CALLBACK)fn_1_1018C);
    fn_1_1B00();
    fn_1_1F54();
    fn_1_2BF0();
    fn_1_3CC();
    fn_1_2CA8();
    fn_1_252F8();
    fn_1_1F308();

    lbl_1_bss_C = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY, 4,
        MDRESULT_CHARACTER_MOTION_COUNT, -1,
        fn_1_4694);
    lbl_1_bss_4 = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY, 2, 8, -1,
        fn_1_4CD4);
    lbl_1_bss_8 = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY, 2, 8, -1,
        fn_1_4EF0);
    lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY, 2, 2, -1,
        fn_1_5160);
    lbl_1_bss_14 = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY,
        MDRESULT_MODEL_ARRAY_COUNT, MDRESULT_MOTION_ARRAY_COUNT, -1, fn_1_6CB8);
    lbl_1_bss_28 = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY, 4, 4, -1,
        fn_1_8184);
    lbl_1_bss_18 = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY, 8, 8, -1,
        fn_1_95E8);
    lbl_1_bss_1C = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY, 8, 8, -1,
        fn_1_AA7C);
    lbl_1_bss_20 = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY,
        MDRESULT_MODEL_ARRAY_COUNT, MDRESULT_MOTION_ARRAY_COUNT, -1, fn_1_CAEC);
    lbl_1_bss_24 = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY,
        MDRESULT_LARGE_MODEL_ARRAY_COUNT, 0, -1, fn_1_B05C);
    lbl_1_bss_2C = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY, 0, 0, -1,
        fn_1_CE0C);
    lbl_1_bss_30 = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY, 1, 1, -1,
        fn_1_31F8);
    lbl_1_bss_34 = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY, 0, 0, -1,
        fn_1_17DCC);
    lbl_1_bss_38 = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY, 3, 3, -1,
        fn_1_1AD68);
    lbl_1_bss_3C[0] = omAddObjEx(lbl_1_bss_0, MDRESULT_OBJECT_PRIORITY, 0, 0, -1,
        fn_1_1E358);
    HuPrcChildCreate(fn_1_F1C4, MDRESULT_MAIN_PROCESS_PRIORITY,
        MDRESULT_MAIN_PROCESS_STACK_SIZE, 0, lbl_1_bss_0);
}

void fn_1_10098(void)
{
    HuDataDirCloseAll();
}

void fn_1_100B8(void)
{
    OSReport(lbl_1_data_719);
    HuDataDirCloseAll();
    fn_1_F548();
}

int _prolog(void)
{
    const VoidFunc *ctor = _ctors;

    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_100B8();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtor = _dtors;

    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

void fn_1_1018C(s32 unused, MDRESULT_CAMERA_WORK *work)
{
    work->targetCenter.x = 0.0f;
    work->targetCenter.y = 65.0f;
    work->targetCenter.z = -800.0f;
    work->targetRot.x = -7.25f;
    work->targetRot.y = 0.0f;
    work->targetRot.z = 0.0f;
    work->targetZoom = 2150.0f;
    fn_1_1FB50(&work->center, &work->targetCenter, 15.0f);
    fn_1_1FB50(&work->rot, &work->targetRot, 15.0f);
    work->zoom = fn_1_1F8BC(
        work->zoom, work->targetZoom, 15.0f);
}

void fn_1_10270(s32 unused, MDRESULT_CAMERA_WORK *work)
{
    work->center.x = 0.0f;
    work->center.y = 65.0f;
    work->center.z = -800.0f;
    work->rot.x = -7.25f;
    work->rot.y = 0.0f;
    work->rot.z = 0.0f;
    work->zoom = 2150.0f;
}

s32 fn_1_102E4(void)
{
    OMOBJ *first;
    OMOBJ *second;

    first = lbl_1_bss_4;
    Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[1],
        0.0f, 10.0f, 0);
    first->work[3] = 0;
    first->objFunc = fn_1_4A9C;
    second = lbl_1_bss_8;
    Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[1],
        0.0f, 10.0f, 0);
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
        0.0f, 10.0f, 0);
    first->work[3] = 0;
    first->objFunc = fn_1_4A9C;
    fn_1_258C(3, 917505, 1);
    fn_1_246C();
    HuAudFXPlay(1168);
    fn_1_8B70(0);
    HuPrcSleep(60);
    second = lbl_1_bss_4;
    Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[1],
        0.0f, 10.0f, 0);
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
        0.0f, 10.0f, 0);
    first->work[3] = 0;
    first->objFunc = fn_1_4BB8;
    fn_1_258C(2, 917507, 1);
    fn_1_246C();
    HuAudFXPlay(1170);
    fn_1_8B70(1);
    HuPrcSleep(60);
    second = lbl_1_bss_8;
    Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[1],
        0.0f, 10.0f, 0);
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

void fn_1_11208(s16 index)
{
    s16 i;
    s16 displayWin;
    s16 insertPos;
    u8 count;
    u8 mask;
    s32 messages[16][3] = {
        { MESSNUM(MESS_PARTY_RESULTS, 5), MESSNUM(MESS_PARTY_RESULTS, 55), MESSNUM(MESS_PARTY_RESULTS, 19) },
        { MESSNUM(MESS_PARTY_RESULTS, 6), MESSNUM(MESS_PARTY_RESULTS, 6), MESSNUM(MESS_PARTY_RESULTS, 6) },
        { MESSNUM(MESS_PARTY_RESULTS, 7), MESSNUM(MESS_PARTY_RESULTS, 56), MESSNUM(MESS_PARTY_RESULTS, 20) },
        { MESSNUM(MESS_PARTY_RESULTS, 8), MESSNUM(MESS_PARTY_RESULTS, 8), MESSNUM(MESS_PARTY_RESULTS, 8) },
        { MESSNUM(MESS_PARTY_RESULTS, 9), MESSNUM(MESS_PARTY_RESULTS, 57), MESSNUM(MESS_PARTY_RESULTS, 21) },
        { MESSNUM(MESS_PARTY_RESULTS, 10), MESSNUM(MESS_PARTY_RESULTS, 10), MESSNUM(MESS_PARTY_RESULTS, 10) },
        { MESSNUM(MESS_PARTY_RESULTS, 11), MESSNUM(MESS_PARTY_RESULTS, 11), MESSNUM(MESS_PARTY_RESULTS, 11) },
        { MESSNUM(MESS_PARTY_RESULTS, 12), MESSNUM(MESS_PARTY_RESULTS, 13), MESSNUM(MESS_PARTY_RESULTS, 12) },
        { MESSNUM(MESS_PARTY_RESULTS, 14), MESSNUM(MESS_PARTY_RESULTS, 58), MESSNUM(MESS_PARTY_RESULTS, 22) },
        { MESSNUM(MESS_PARTY_RESULTS, 12), MESSNUM(MESS_PARTY_RESULTS, 13), MESSNUM(MESS_PARTY_RESULTS, 12) },
        { MESSNUM(MESS_PARTY_RESULTS, 29), MESSNUM(MESS_PARTY_RESULTS, 59), MESSNUM(MESS_PARTY_RESULTS, 37) },
        { MESSNUM(MESS_PARTY_RESULTS, 30), MESSNUM(MESS_PARTY_RESULTS, 30), MESSNUM(MESS_PARTY_RESULTS, 30) },
        { MESSNUM(MESS_PARTY_RESULTS, 31), MESSNUM(MESS_PARTY_RESULTS, 31), MESSNUM(MESS_PARTY_RESULTS, 31) },
        { MESSNUM(MESS_PARTY_RESULTS, 32), MESSNUM(MESS_PARTY_RESULTS, 33), MESSNUM(MESS_PARTY_RESULTS, 32) },
        { MESSNUM(MESS_PARTY_RESULTS, 34), MESSNUM(MESS_PARTY_RESULTS, 60), MESSNUM(MESS_PARTY_RESULTS, 38) },
        { MESSNUM(MESS_PARTY_RESULTS, 32), MESSNUM(MESS_PARTY_RESULTS, 33), MESSNUM(MESS_PARTY_RESULTS, 32) },
    };

    displayWin = 3;
    if (index == 1) {
        displayWin = 2;
    }

    if (displayWin == 3) {
        fn_1_4B44();
    } else {
        fn_1_4C60();
    }

    fn_1_258C(displayWin, messages[lbl_1_bss_1278.values[3] * 10][index], 1);
    fn_1_246C();
    count = fn_1_1109C(index, &mask);
    fn_1_A624(index);
    HuPrcSleep(60);
    fn_1_7560(1, 0);
    HuPrcSleep(180);

    if (lbl_1_bss_1278.values[3] == 0) {
        switch (count) {
        case 2:
            if (displayWin == 3) {
                fn_1_4B44();
            } else {
                fn_1_4C60();
            }
            fn_1_258C(displayWin, messages[2][index], 1);
            fn_1_246C();
            break;
        case 3:
            if (displayWin == 3) {
                fn_1_4B44();
            } else {
                fn_1_4C60();
            }
            fn_1_258C(displayWin, messages[4][index], 1);
            fn_1_246C();
            break;
        }

        fn_1_9EBC(count, mask);
        fn_1_7560(3, 0);
        HuPrcSleep(60);
        if (count == 0 || count == 4) {
            HuAudFXPlay(1176);
        } else {
            HuAudFXPlay(1175);
        }
        fn_1_7560(2, mask);
        for (i = 0; i < 4; i++) {
            if ((mask & (1 << i)) == 0) {
                fn_1_3364(i, 6, 15.0f, 0);
            } else if (count == 4) {
                fn_1_3364(i, 6, 15.0f, 0);
            } else {
                fn_1_3364(i, 5, 15.0f, 0);
            }
        }
        fn_1_378C();

        for (i = 0, insertPos = 0; i < 4; i++) {
            if (mask & (1 << i)) {
                if (count == 1 || count == 2 || count == 3) {
                    fn_1_27A4(4, lbl_1_bss_1278.messages[i], (s32)insertPos++);
                } else {
                    fn_1_27A4(displayWin, lbl_1_bss_1278.messages[i], (s32)insertPos++);
                }
            }
        }

        switch (count) {
        case 1:
            Hu3DMotionShiftSet(lbl_1_bss_4->mdlId[0], lbl_1_bss_4->mtnId[4],
                0.0f, 10.0f, 0);
            Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[0], lbl_1_bss_8->mtnId[4],
                0.0f, 10.0f, 0);
            fn_1_258C(4, messages[1][index], 1);
            fn_1_246C();
            break;
        case 0:
            if (displayWin == 3) {
                fn_1_4B44();
            } else {
                fn_1_4C60();
            }
            fn_1_258C(displayWin, messages[8][index], 1);
            fn_1_246C();
            fn_1_258C(displayWin, messages[9][index], 1);
            fn_1_246C();
            break;
        case 2:
            Hu3DMotionShiftSet(lbl_1_bss_4->mdlId[0], lbl_1_bss_4->mtnId[4],
                0.0f, 10.0f, 0);
            Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[0], lbl_1_bss_8->mtnId[4],
                0.0f, 10.0f, 0);
            fn_1_258C(4, messages[3][index], 1);
            fn_1_246C();
            break;
        case 3:
            Hu3DMotionShiftSet(lbl_1_bss_4->mdlId[0], lbl_1_bss_4->mtnId[4],
                0.0f, 10.0f, 0);
            Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[0], lbl_1_bss_8->mtnId[4],
                0.0f, 10.0f, 0);
            fn_1_258C(4, messages[5][index], 1);
            fn_1_246C();
            break;
        case 4:
            if (displayWin == 3) {
                fn_1_4B44();
            } else {
                fn_1_4C60();
            }
            fn_1_258C(displayWin, messages[6][index], 1);
            fn_1_246C();
            fn_1_258C(displayWin, messages[7][index], 1);
            fn_1_246C();
            break;
        }

        if (count != 0 && count != 4) {
            for (i = 0; i < 4; i++) {
                if (mask & (1 << i)) {
                    lbl_1_bss_10D4[i].star++;
                }
            }
        }
    } else {
        fn_1_7560(3, 0);
        fn_1_9EBC(count, mask);
        HuPrcSleep(60);
        if (count == 0 || count == 2) {
            HuAudFXPlay(1176);
        } else {
            HuAudFXPlay(1175);
        }
        fn_1_7560(2, mask);

        if (count == 2 || count == 0) {
            fn_1_3364(0, 6, 15.0f, 0);
            fn_1_3364(1, 6, 15.0f, 0);
            fn_1_3364(2, 6, 15.0f, 0);
            fn_1_3364(3, 6, 15.0f, 0);
        } else if (mask & 1) {
            fn_1_3364(0, 5, 15.0f, 0);
            fn_1_3364(1, 5, 15.0f, 0);
            fn_1_3364(2, 6, 15.0f, 0);
            fn_1_3364(3, 6, 15.0f, 0);
        } else {
            fn_1_3364(0, 6, 15.0f, 0);
            fn_1_3364(1, 6, 15.0f, 0);
            fn_1_3364(2, 5, 15.0f, 0);
            fn_1_3364(3, 5, 15.0f, 0);
        }

        fn_1_378C();

        if (count == 1) {
            if (mask & 1) {
                fn_1_27A4(4, lbl_1_bss_1278.messages[0], 0);
                fn_1_27A4(4, lbl_1_bss_1278.messages[1], 1);
                fn_1_27A4(4, lbl_1_bss_1278.messages[4], 2);
            } else {
                fn_1_27A4(4, lbl_1_bss_1278.messages[2], 0);
                fn_1_27A4(4, lbl_1_bss_1278.messages[3], 1);
                fn_1_27A4(4, lbl_1_bss_1278.messages[5], 2);
            }
        } else {
            if (mask & 1) {
                fn_1_27A4(displayWin, lbl_1_bss_1278.messages[0], 0);
                fn_1_27A4(displayWin, lbl_1_bss_1278.messages[1], 1);
                fn_1_27A4(displayWin, lbl_1_bss_1278.messages[4], 2);
            } else {
                fn_1_27A4(displayWin, lbl_1_bss_1278.messages[2], 0);
                fn_1_27A4(displayWin, lbl_1_bss_1278.messages[3], 1);
                fn_1_27A4(displayWin, lbl_1_bss_1278.messages[5], 2);
            }
        }

        switch (count) {
        case 1:
            Hu3DMotionShiftSet(lbl_1_bss_4->mdlId[0], lbl_1_bss_4->mtnId[4],
                0.0f, 10.0f, 0);
            Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[0], lbl_1_bss_8->mtnId[4],
                0.0f, 10.0f, 0);
            fn_1_258C(4, messages[11][index], 1);
            fn_1_246C();
            break;
        case 0:
            if (displayWin == 3) {
                fn_1_4B44();
            } else {
                fn_1_4C60();
            }
            fn_1_258C(displayWin, messages[14][index], 1);
            fn_1_246C();
            fn_1_258C(displayWin, messages[15][index], 1);
            fn_1_246C();
            break;
        case 2:
            if (displayWin == 3) {
                fn_1_4B44();
            } else {
                fn_1_4C60();
            }
            fn_1_258C(displayWin, messages[12][index], 1);
            fn_1_246C();
            fn_1_258C(displayWin, messages[13][index], 1);
            fn_1_246C();
            break;
        }

        if (count == 1) {
            for (i = 0; i < 2; i++) {
                if (mask & (1 << i)) {
                    lbl_1_bss_10D4[i].star++;
                }
            }
        }
    }

    Hu3DMotionShiftSet(lbl_1_bss_4->mdlId[0], lbl_1_bss_4->mtnId[0],
        0.0f, 15.0f,
        HU3D_MOTATTR_LOOP);
    Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[0], lbl_1_bss_8->mtnId[0],
        0.0f, 15.0f,
        HU3D_MOTATTR_LOOP);

    fn_1_98E0();
    fn_1_7560(0, 0);
    fn_1_A984();
    fn_1_37EC();
}

s32 fn_1_1295C(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        0.0f, 10.0f, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_4BB8;
    fn_1_258C(2, 917527, 1);
    fn_1_246C();
    fn_1_23C0();
    return TRUE;
}

s16 fn_1_12C80(u8 *mask)
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

s32 fn_1_12D7C(u8 mask)
{
    OMOBJ *obj;
    s16 result = 0;
    s16 i;

    HuPrcSleep(120);
    lbl_1_bss_12B0[0] = HuAudSStreamPlay(34);

    if (lbl_1_bss_1278.values[3] == 0) {
        fn_1_258C(2, 917528, 1);
        fn_1_246C();
        fn_1_258C(2, 917529, 1);
        fn_1_246C();
        fn_1_295C(65539, 0);

        fn_1_C23C(mask);
        result = fn_1_C9A0();
        fn_1_2B44();

        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_10D4[i].rank == 0) {
                if (i == result) {
                    fn_1_3364(i, 5, 15.0f, 0);
                    lbl_1_bss_10D4[i].rank = 0;
                } else {
                    fn_1_3364(i, 6, 15.0f, 0);
                    lbl_1_bss_10D4[i].rank = 1;
                }
            }
        }
        fn_1_27A4(2, lbl_1_bss_1278.messages[result], 0);

        fn_1_378C();
        fn_1_258C(2, 917530, 1);
        fn_1_246C();
    } else {
        fn_1_258C(2, 917543, 1);
        fn_1_246C();
        fn_1_258C(2, 917529, 1);
        fn_1_246C();
        fn_1_295C(65539, 0);

        fn_1_C23C(15);
        result = fn_1_C9A0();
        fn_1_2B44();

        if (result == 0) {
            lbl_1_bss_10D4[0].rank = 0;
            lbl_1_bss_10D4[1].rank = 1;
            fn_1_3364(0, 5, 15.0f, 0);
            fn_1_3364(1, 5, 15.0f, 0);
            fn_1_3364(2, 6, 15.0f, 0);
            fn_1_3364(3, 6, 15.0f, 0);
            fn_1_27A4(2, lbl_1_bss_1278.messages[0], 0);
            fn_1_27A4(2, lbl_1_bss_1278.messages[1], 1);
            fn_1_27A4(2, lbl_1_bss_1278.messages[4], 2);
        } else {
            lbl_1_bss_10D4[0].rank = 1;
            lbl_1_bss_10D4[1].rank = 0;
            fn_1_3364(0, 6, 15.0f, 0);
            fn_1_3364(1, 6, 15.0f, 0);
            fn_1_3364(2, 5, 15.0f, 0);
            fn_1_3364(3, 5, 15.0f, 0);
            fn_1_27A4(2, lbl_1_bss_1278.messages[2], 0);
            fn_1_27A4(2, lbl_1_bss_1278.messages[3], 1);
            fn_1_27A4(2, lbl_1_bss_1278.messages[5], 2);
        }

        fn_1_378C();
        fn_1_258C(2, 917544, 1);
        fn_1_246C();
    }

    fn_1_C358();
    fn_1_258C(2, 917531, 1);
    fn_1_246C();
    fn_1_37EC();
    HuAudSStreamFadeOut(lbl_1_bss_12B0[0], 1000);
    return TRUE;
}

s32 fn_1_15378(void)
{
    s16 i;
    s16 selected = 0;
    s16 slotCount = 4;

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
    } else {
        if (selected == 0) {
            fn_1_27A4(4, lbl_1_bss_1278.messages[0], 0);
            fn_1_27A4(4, lbl_1_bss_1278.messages[1], 1);
            fn_1_27A4(4, lbl_1_bss_1278.messages[4], 2);
        } else {
            fn_1_27A4(4, lbl_1_bss_1278.messages[2], 0);
            fn_1_27A4(4, lbl_1_bss_1278.messages[3], 1);
            fn_1_27A4(4, lbl_1_bss_1278.messages[5], 2);
        }

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
    s32 order[4] = { 0, 1, 2, 3 };
    s32 i;
    s32 j;
    s32 temp;


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
        j = 0;
        lbl_1_bss_10D4[order[0]].rank = j;
        for (i = 1; i < 4; i++) {
            j++;
            lbl_1_bss_10D4[order[i]].rank = j;
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
                lbl_1_bss_10D4[0].rank = lbl_1_bss_10D4[1].rank = 0;
            } else if (lbl_1_bss_10D4[0].coin >
                lbl_1_bss_10D4[1].coin) {
                lbl_1_bss_10D4[0].rank = 0;
                lbl_1_bss_10D4[1].rank = 1;
            } else if (lbl_1_bss_10D4[0].coin <
                lbl_1_bss_10D4[1].coin) {
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
    u8 mask = 0;

    fn_1_4B44();
    fn_1_4C60();

    fn_1_258C(4, 917504, 1);
    fn_1_246C();
    fn_1_105CC();
    fn_1_10B34();
    if (lbl_1_bss_1278.values[2] != 0) {
        fn_1_11208(0);
        fn_1_11208(1);
        fn_1_11208(2);
    }

    fn_1_4C60();
    fn_1_258C(2, 917527, 1);
    fn_1_246C();
    fn_1_23C0();

    fn_1_1648C();
    HuAudSStreamFadeOut(lbl_1_bss_12B0[0], 1000);

    if (fn_1_12C80(&mask)) {
        fn_1_12D7C(mask);
    }
    fn_1_15378();
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

void fn_1_17248(void)
{
    s16 state = 0;
    s16 i;

    do {
        HuPrcVSleep();
        switch (state) {
        case 0:
            HuPrcSleep(10);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSS_COPY, 60);
            WipeWait();
            fn_1_2B44();
            fn_1_17D4((MDRESULT_CAMERA_CALLBACK)fn_1_10270);
            HuPrcSleep(10);
            for (i = 0; i < 4; i++) {
                fn_1_26BE4(i);
                fn_1_26BE4((s16)(i + 4));
                Hu3DModelScaleSet(lbl_1_bss_C->mdlId[i],
                    1.0f, 1.0f, 1.0f);
                fn_1_3364(i, 0, 0.0f, HU3D_MOTATTR_LOOP);
            }
            Hu3DModelAttrSet(lbl_1_bss_14->mdlId[1], 1);
            fn_1_26F74();
            fn_1_20108(lbl_1_bss_11A0[4], 4);
            Hu3DMotionShiftSet(lbl_1_bss_4->mdlId[0],
                lbl_1_bss_4->mtnId[3], 0.0f,
                0.0f, HU3D_MOTATTR_LOOP);
            Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[0],
                lbl_1_bss_8->mtnId[3], 0.0f,
                0.0f, HU3D_MOTATTR_LOOP);
            HuWinDispOff(lbl_1_bss_1304[1]);
            lbl_1_data_646[0] = -1;
            lbl_1_bss_C->objFunc = NULL;
            fn_1_17CF4();
            fn_1_1AAF8();
            fn_1_1E258();
            HuPrcSleep(10);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSS_COPY, 60);
            WipeWait();
            HuPrcSleep(10);
            fn_1_295C(MDRESULT_MESSAGE_GUIDE_START, 1);
            do {
                HuPrcVSleep();
            } while ((HuPadBtnDown[0] & PAD_BUTTON_A) == 0);
            HuAudFXPlay(2);
            state = 1;
            break;

        case 1:
            HuPrcSleep(10);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_CROSS_COPY, 60);
            WipeWait();
            fn_1_2B44();
            fn_1_17D4((MDRESULT_CAMERA_CALLBACK)fn_1_10270);
            HuPrcSleep(10);
            for (i = 0; i < 4; i++) {
                fn_1_3364(i, 0, 0.0f, HU3D_MOTATTR_LOOP);
            }
            Hu3DModelAttrSet(lbl_1_bss_14->mdlId[1], 1);
            fn_1_26F74();
            fn_1_20108(lbl_1_bss_11A0[4], 4);
            HuWinDispOn(lbl_1_bss_1304[1]);
            lbl_1_data_646[0] = 1;
            fn_1_17CF4();
            fn_1_1AB5C();
            fn_1_1E204();
            HuPrcSleep(10);
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_CROSS_COPY, 60);
            WipeWait();
            HuPrcSleep(10);
            fn_1_295C(MDRESULT_MESSAGE_GUIDE_RETURN, 0);
            for (;;) {
                HuPrcVSleep();
                if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                    lbl_1_bss_34->objFunc = NULL;
                    lbl_1_bss_38->objFunc = NULL;
                    lbl_1_bss_3C[0]->objFunc = NULL;
                    HuAudFXPlay(2);
                    state = 3;
                    break;
                }
                if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                    HuAudFXPlay(3);
                    state = 0;
                    break;
                }
            }
            break;
        }
    } while (state != 3);
}

void fn_1_17B10(void)
{
    HuVecF position = { 0.0f, 0.0f, 0.0f };
    HuVecF direction = { 0.0f, 0.0f, 0.0f };
    GXColor color = { 0, 0, 255, 0 };
    s16 i;

    (void)position;
    (void)direction;
    (void)color;
    HuPrcSleep(5);
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrReset(lbl_1_bss_C->mdlId[i], 1);
    }
    Hu3DModelAttrReset(lbl_1_bss_4->mdlId[0], 1);
    Hu3DModelAttrReset(lbl_1_bss_8->mdlId[0], 1);
    Hu3DModelShadowSet(lbl_1_bss_4->mdlId[0]);
    Hu3DModelShadowSet(lbl_1_bss_8->mdlId[0]);
    lbl_1_bss_12B0[0] = HuAudSStreamPlay(34);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    WipeWait();
    fn_1_169A4();
    HuPrcSleep(600);
    do {
        HuPrcVSleep();
    } while ((HuPadBtnDown[0] & PAD_BUTTON_A) == 0);
    HuAudFXPlay(2);
    fn_1_17248();
    HuAudSStreamFadeOut(lbl_1_bss_12B0[1], 1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    WipeWait();
}

void fn_1_17CF4(void)
{
    HUSPR_GROUPID *group = &lbl_1_bss_714.group;
    s16 bank = lbl_1_bss_1278.values[0];
    s16 otherBank = (lbl_1_bss_1278.values[1] - 10) / 5;

    fn_1_20188(group[0], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(group[0], 1, bank);
    HuSprBankSet(group[0], 2, otherBank);
}

void fn_1_17D94(void)
{
    HUSPR_GROUPID *group = &lbl_1_bss_714.group;

    fn_1_20108(group[0], HUSPR_ATTR_DISPOFF);
}

void fn_1_17DCC(OMOBJ *obj)
{
    MDRESULT_GROUP_WORK *group = &lbl_1_bss_714;
    MDRESULT_GROUP_WORK *finalGroup;
    s16 i;

    group->group = HuSprGrpCreate(3);
    group->sprites[0] = HuSprCreate(lbl_1_bss_11AC[10], 0, 0);
    group->sprites[1] = HuSprCreate(lbl_1_bss_11AC[11], 0, 0);
    group->sprites[2] = HuSprCreate(lbl_1_bss_11AC[12], 0, 0);
    for (i = 0; i < 3; i++) {
        HuSprGrpMemberSet(group->group, i, group->sprites[i]);
    }
    HuSprPosSet(group->group, 0, 286.0f, 50.0f);
    HuSprPosSet(group->group, 1, 84.0f, 50.0f);
    HuSprPosSet(group->group, 2, 492.0f, 50.0f);
    HuSprDrawNoSet(group->group, 0, 64);
    HuSprDrawNoSet(group->group, 1, 64);
    HuSprDrawNoSet(group->group, 2, 64);
    finalGroup = &lbl_1_bss_714;
    fn_1_20108(finalGroup->group, HUSPR_ATTR_DISPOFF);
    obj->objFunc = NULL;
}

void fn_1_17F60(void)
{
    HUSPR_GROUPID *group = &lbl_1_bss_714.group;
}

void fn_1_17F78(OMOBJ *obj)
{
    MDRESULT_PLAYER_WORK *work;
    HuVecF rotation;
    s16 i;
    s16 j;

    lbl_1_bss_48++;
    if (lbl_1_bss_48 == 300) {
        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_70C[i] == 0) {

                fn_1_3364(i, 0, 15.0f, HU3D_MOTATTR_LOOP);
            }
        }
    } else if (lbl_1_bss_48 == 500) {
        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_70C[i] == 0) {

                fn_1_3364(i, 9, 0.0f, 0);
            }
        }
        lbl_1_bss_48 = 0;
    }

    for (i = 0, work = lbl_1_bss_66C; i < 4; i++, work++) {
        for (j = 0; j < 2; j++) {
            Hu3DModelRotGet(work->models[j + 1], &rotation);
            rotation.y -= 1.0f;
            if (rotation.y < 0.0f) {
                rotation.y += 360.0f;
            }
            Hu3DModelRotSetV(work->models[j + 1], &rotation);
        }
    }
}

void fn_1_181C0(void)
{
    HuVecF groupPos[4] = {
        {288.0f, 164.0f, 1000.0f},
        {104.0f, 353.0f, 1000.0f},
        {288.0f, 353.0f, 1000.0f},
        {472.0f, 353.0f, 1000.0f}
    };
    HuVecF offsets[4][4] = {
        {{-55.0f, 54.0f, 0.0f}, {56.0f, -65.0f, 0.0f}, {10.0f, 50.0f, 0.0f}, {10.0f, 22.0f, 0.0f}},
        {{-44.0f, 54.0f, 0.0f}, {44.0f, -52.0f, 0.0f}, {8.0f, 40.0f, 0.0f}, {8.0f, 17.0f, 0.0f}},
        {{-44.0f, 54.0f, 0.0f}, {44.0f, -52.0f, 0.0f}, {8.0f, 40.0f, 0.0f}, {8.0f, 17.0f, 0.0f}},
        {{-44.0f, 54.0f, 0.0f}, {44.0f, -52.0f, 0.0f}, {8.0f, 40.0f, 0.0f}, {8.0f, 17.0f, 0.0f}}
    };
    float scales[4][4] = {
        {0.8f, 0.5f, 0.3f, 0.4f},
        {0.6f, 0.4f, 0.24f, 0.32f},
        {0.4f, 0.4f, 0.24f, 0.32f},
        {0.2f, 0.4f, 0.24f, 0.32f}
    };
    HuVecF specialPos[2] = {
        {94.0f, 284.0f, 1000.0f},
        {482.0f, 284.0f, 1000.0f}
    };
    s16 order[4];
    s16 rankVal[4];
    s16 playerIdx[4];
    s16 stars[4];
    s16 coins[4];
    MDRESULT_PLAYER_WORK *work;
    s16 i;
    s16 j;
    s16 digit;
    s16 count;

    count = 0;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (i == lbl_1_bss_10D4[j].rank) {
                order[count++] = j;
            }
        }
    }

    for (i = 0; i < 4; i++) {
        playerIdx[i] = lbl_1_bss_10D4[order[i]].playerIndex;
        rankVal[i] = lbl_1_bss_10D4[order[i]].rank;
        stars[i] = lbl_1_bss_10D4[order[i]].star;
        if (stars[i] >= 999) {
            stars[i] = 999;
        }
        coins[i] = lbl_1_bss_10D4[order[i]].coin;
        if (coins[i] >= 999) {
            coins[i] = 999;
        }
        if (i == 0) {
            lbl_1_bss_70C[order[i]] = 1;
            fn_1_3364(order[i], 8, 0.0f, HU3D_MOTATTR_LOOP);
        } else {
            lbl_1_bss_70C[order[i]] = 0;
            fn_1_3364(order[i], 9, 0.0f, 0);
        }
    }

    for (i = 0, work = lbl_1_bss_66C; i < 4; i++, work++) {
        HuSprGrpPosSet(work->group, groupPos[i].x, groupPos[i].y);
        if (i != 0) {
            HuSprGrpScaleSet(work->group, 0.8f,
                0.8f);
        }
        if (i == 0) {
            Hu3DModelAttrReset(work->models[0], 1);
        }
        Hu3DModelAttrReset(work->models[1], 1);
        Hu3DModelAttrReset(work->models[2], 1);
        fn_1_20188(work->group, 4);
        if (i != 0) {
            for (j = 11; j < 14; j++) {
                HuSprAttrSet(work->group, j, 4);
            }
        }
        fn_1_2001C(work->models[0], &groupPos[i], &offsets[i][1]);
        Hu3DModelScaleSet(work->models[0], scales[i][1],
            scales[i][1], scales[i][1]);
        fn_1_2001C(work->models[1], &groupPos[i], &offsets[i][2]);
        Hu3DModelScaleSet(work->models[1], scales[i][2],
            scales[i][2], scales[i][2]);
        fn_1_2001C(work->models[2], &groupPos[i], &offsets[i][3]);
        Hu3DModelScaleSet(work->models[2], scales[i][3],
            scales[i][3], scales[i][3]);
        fn_1_2001C(lbl_1_bss_C->mdlId[playerIdx[i]], &groupPos[i], &offsets[i][0]);
        Hu3DModelScaleSet(lbl_1_bss_C->mdlId[playerIdx[i]], scales[rankVal[i]][0],
            scales[rankVal[i]][0], scales[rankVal[i]][0]);
        Hu3DModelLayerSet(lbl_1_bss_C->mdlId[playerIdx[i]], 3);

        digit = stars[i] / 100;
        HuSprBankSet(work->group, 0, digit);
        if (digit == 0) {
            HuSprBankSet(work->group, 0, 10);
        }
        digit = (stars[i] - digit * 100) / 10;
        HuSprBankSet(work->group, 1, digit);
        if (digit == 0 && stars[i] / 100 == 0) {
            HuSprAttrSet(work->group, 1, 4);
        }
        digit = stars[i] % 10;
        HuSprBankSet(work->group, 2, digit);
        digit = coins[i] / 100;
        HuSprBankSet(work->group, 3, digit);
        if (digit == 0) {
            HuSprBankSet(work->group, 3, 10);
        }
        digit = (coins[i] - digit * 100) / 10;
        HuSprBankSet(work->group, 4, digit);
        if (digit == 0 && coins[i] / 100 == 0) {
            HuSprAttrSet(work->group, 4, 4);
        }
        digit = coins[i] % 10;
        HuSprBankSet(work->group, 5, digit);
        HuSprBankSet(work->group, 6, rankVal[i]);
        for (j = 0; j < 4; j++) {
            if (j != rankVal[i]) {
                HuSprAttrSet(work->group, j + 7, 4);
            }
        }
    }

    fn_1_2001C(lbl_1_bss_4->mdlId[0], NULL, &specialPos[0]);
    Hu3DModelScaleSet(lbl_1_bss_4->mdlId[0], 1.0f, 1.0f,
        1.0f);
    Hu3DModelLayerSet(lbl_1_bss_4->mdlId[0], 3);
    fn_1_2001C(lbl_1_bss_8->mdlId[0], NULL, &specialPos[1]);
    Hu3DModelScaleSet(lbl_1_bss_8->mdlId[0], 1.0f, 1.0f,
        1.0f);
    Hu3DModelLayerSet(lbl_1_bss_8->mdlId[0], 3);
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrReset(lbl_1_bss_C->mdlId[i], 1);
        Hu3DModelRotSet(lbl_1_bss_C->mdlId[i], 0.0f,
            0.0f, 0.0f);
    }
    Hu3DModelAttrReset(lbl_1_bss_4->mdlId[0], 1);
    Hu3DModelAttrReset(lbl_1_bss_8->mdlId[0], 1);
    Hu3DModelShadowReset(lbl_1_bss_4->mdlId[0]);
    Hu3DModelShadowReset(lbl_1_bss_8->mdlId[0]);
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

void fn_1_18F08(OMOBJ *obj)
{
    MDRESULT_PLAYER_SPRITE_TABLE spriteInfo = { {
        {0, 10, 10, {40.0f, 22.0f}, {1.0f, 1.0f}, 0.0f},
        {0, 10, 0, {60.0f, 22.0f}, {1.0f, 1.0f}, 0.0f},
        {0, 10, 0, {80.0f, 22.0f}, {1.0f, 1.0f}, 0.0f},
        {0, 10, 10, {40.0f, 50.0f}, {1.0f, 1.0f}, 0.0f},
        {0, 10, 0, {60.0f, 50.0f}, {1.0f, 1.0f}, 0.0f},
        {0, 10, 0, {80.0f, 50.0f}, {1.0f, 1.0f}, 0.0f},
        {1, 10, 0, {56.0f, -30.0f}, {1.0f, 1.0f}, 0.0f},
        {2, 40, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
        {3, 40, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
        {4, 40, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
        {5, 40, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
        {29, 20, 0, {0.0f, 74.0f}, {0.7f, 0.7f}, 0.0f},
        {31, 30, 0, {-90.0f, 20.0f}, {-0.7f, 0.6f}, 0.0f},
        {31, 30, 0, {90.0f, 20.0f}, {0.7f, 0.6f}, 0.0f},
    }};
    MDRESULT_PLAYER_SPRITE_WORK *work;
    s16 player;
    s16 sprite;

    player = 0;
    work = (MDRESULT_PLAYER_SPRITE_WORK *)lbl_1_bss_66C;
    for (; player < 4; player++, work++) {
        work->models[0] = Hu3DModelLink(obj->mdlId[0]);
        Hu3DModelLayerSet(work->models[0], 3);
        Hu3DMotionShiftSet(work->models[0], obj->mtnId[0], 0.0f,
            0.0f, HU3D_MOTATTR_LOOP);
        work->models[1] = Hu3DModelLink(obj->mdlId[1]);
        Hu3DModelLayerSet(work->models[1], 3);
        Hu3DMotionShiftSet(work->models[1], obj->mtnId[1], 0.0f,
            0.0f, HU3D_MOTATTR_LOOP);
        work->models[2] = Hu3DModelLink(obj->mdlId[2]);
        Hu3DModelLayerSet(work->models[2], 3);
        Hu3DMotionShiftSet(work->models[2], obj->mtnId[2], 0.0f,
            0.0f, HU3D_MOTATTR_LOOP);
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

void fn_1_192BC(OMOBJ *obj)
{
    MDRESULT_PLAYER_WORK *work;
    HuVecF rotation;
    s16 i;
    s16 j;

    lbl_1_bss_48++;
    if (lbl_1_bss_48 == 300) {
        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_70C[i] == 0) {
                fn_1_3364(i, 0, 15.0f, HU3D_MOTATTR_LOOP);
            }
        }
    } else if (lbl_1_bss_48 == 500) {
        for (i = 0; i < 4; i++) {
            if (lbl_1_bss_70C[i] == 0) {
                fn_1_3364(i, 9, 0.0f, 0);
            }
        }
        lbl_1_bss_48 = 0;
    }

    for (i = 0, work = lbl_1_bss_66C; i < 2; i++, work++) {
        for (j = 0; j < 2; j++) {
            Hu3DModelRotGet(work->models[j + 1], &rotation);
            rotation.y -= 1.0f;
            if (rotation.y < 0.0f) {
                rotation.y += 360.0f;
            }
            Hu3DModelRotSetV(work->models[j + 1], &rotation);
        }
    }
}

void fn_1_19504(void)
{
    HuVecF groupPos[2] = {
        {288.0f, 164.0f, 1000.0f},
        {288.0f, 338.0f, 1000.0f}
    };
    HuVecF offsets[2][5] = {
        {{-102.0f, 42.0f, 0.0f}, {-2.0f, 42.0f, 0.0f}, {138.0f, -63.0f, 0.0f}, {92.0f, 58.0f, 0.0f}, {92.0f, 30.0f, 0.0f}},
        {{-102.0f, 42.0f, 0.0f}, {-2.0f, 42.0f, 0.0f}, {124.0f, -56.0f, 0.0f}, {82.0f, 52.0f, 0.0f}, {82.0f, 27.0f, 0.0f}}
    };
    float scales[2][4] = {
        {0.8f, 0.5f, 0.3f, 0.4f},
        {0.6f, 0.45f, 0.27f, 0.36f}
    };
    HuVecF specialPos[2] = {
        {60.0f, 380.0f, 1000.0f},
        {516.0f, 380.0f, 1000.0f}
    };
    s32 msg[2];
    s16 modelIdx[2][2];
    s16 order[2];
    s16 teamVal[2];
    s16 stars[2];
    s16 coins[2];
    MDRESULT_PLAYER_WORK *work;
    s16 i;
    s16 j;
    s16 digit;
    s16 count;

    count = 0;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            if (i == lbl_1_bss_10D4[j].rank) {
                order[count++] = j;
            }
        }
    }

    for (i = 0; i < 2; i++) {
        modelIdx[i][0] = order[i] * 2;
        modelIdx[i][1] = order[i] * 2 + 1;
        teamVal[i] = lbl_1_bss_10D4[order[i]].teamIndex;
        msg[i] = (&lbl_1_bss_1278.messages[4])[order[i]];
        stars[i] = lbl_1_bss_10D4[order[i]].star;
        if (stars[i] >= 999) {
            stars[i] = 999;
        }
        coins[i] = lbl_1_bss_10D4[order[i]].coin;
        if (coins[i] >= 999) {
            coins[i] = 999;
        }
        if (i == 0) {
            fn_1_3364(order[i] * 2, 8, 0.0f, HU3D_MOTATTR_LOOP);
            fn_1_3364(order[i] * 2 + 1, 8, 0.0f, HU3D_MOTATTR_LOOP);
            lbl_1_bss_70C[order[i] * 2] = 1;
            lbl_1_bss_70C[order[i] * 2 + 1] = 1;
        } else {
            fn_1_3364(order[i] * 2, 9, 0.0f, 0);
            fn_1_3364(order[i] * 2 + 1, 9, 0.0f, 0);
            lbl_1_bss_70C[order[i] * 2] = 0;
            lbl_1_bss_70C[order[i] * 2 + 1] = 0;
        }
    }

    for (i = 0, work = lbl_1_bss_66C; i < 2; i++, work++) {
        HuSprGrpPosSet(work->group, groupPos[i].x, groupPos[i].y);
        HuSprGrpPosSet(work->secondGroup, groupPos[i].x, groupPos[i].y);
        if (i == 1) {
            HuSprGrpScaleSet(work->group, 0.9f,
                0.9f);
        }
        if (i == 0) {
            Hu3DModelAttrReset(work->models[0], 1);
        }
        Hu3DModelAttrReset(work->models[1], 1);
        Hu3DModelAttrReset(work->models[2], 1);
        fn_1_20188(work->group, 4);
        fn_1_20188(work->secondGroup, 4);
        if (i == 1) {
            for (j = 9; j < 12; j++) {
                HuSprAttrSet(work->group, j, 4);
            }
        }
        fn_1_2001C(work->models[0], &groupPos[i], &offsets[i][2]);
        Hu3DModelScaleSet(work->models[0], scales[i][1],
            scales[i][1], scales[i][1]);
        fn_1_2001C(work->models[1], &groupPos[i], &offsets[i][3]);
        Hu3DModelScaleSet(work->models[1], scales[i][2],
            scales[i][2], scales[i][2]);
        fn_1_2001C(work->models[2], &groupPos[i], &offsets[i][4]);
        Hu3DModelScaleSet(work->models[2], scales[i][3],
            scales[i][3], scales[i][3]);
        fn_1_2001C(lbl_1_bss_C->mdlId[modelIdx[i][0]], &groupPos[i], &offsets[i][0]);
        fn_1_2001C(lbl_1_bss_C->mdlId[modelIdx[i][1]], &groupPos[i], &offsets[i][1]);
        Hu3DModelScaleSet(lbl_1_bss_C->mdlId[modelIdx[i][0]], scales[i][0], scales[i][0], scales[i][0]);
        Hu3DModelScaleSet(lbl_1_bss_C->mdlId[modelIdx[i][1]], scales[i][0], scales[i][0], scales[i][0]);
        Hu3DModelLayerSet(lbl_1_bss_C->mdlId[modelIdx[i][0]], 3);
        Hu3DModelLayerSet(lbl_1_bss_C->mdlId[modelIdx[i][1]], 3);

        digit = stars[i] / 100;
        HuSprBankSet(work->group, 0, digit);
        if (digit == 0) {
            HuSprBankSet(work->group, 0, 10);
        }
        digit = (stars[i] - digit * 100) / 10;
        HuSprBankSet(work->group, 1, digit);
        if (digit == 0 && stars[i] / 100 == 0) {
            HuSprAttrSet(work->group, 1, 4);
        }
        digit = stars[i] % 10;
        HuSprBankSet(work->group, 2, digit);
        digit = coins[i] / 100;
        HuSprBankSet(work->group, 3, digit);
        if (digit == 0) {
            HuSprBankSet(work->group, 3, 10);
        }
        digit = (coins[i] - digit * 100) / 10;
        HuSprBankSet(work->group, 4, digit);
        if (digit == 0 && coins[i] / 100 == 0) {
            HuSprAttrSet(work->group, 4, 4);
        }
        digit = coins[i] % 10;
        HuSprBankSet(work->group, 5, digit);
        HuSprBankSet(work->group, 6, i);
        HuSprAttrSet(work->group, 8 - teamVal[i], 4);
        HuSprAttrSet(work->secondGroup, 1 - teamVal[i], 4);

        HuWinPosSet(work->winId,
            20.0f + ((groupPos[i].x - 72.0f) - 124.0f),
            6.0f + (34.0f + groupPos[i].y));
        HuWinDispOn(work->winId);
        HuWinMesSet(work->winId, msg[i]);
        HuWinMesSpeedSet(work->winId, 0);
    }

    fn_1_2001C(lbl_1_bss_4->mdlId[0], NULL, &specialPos[0]);
    Hu3DModelScaleSet(lbl_1_bss_4->mdlId[0], 1.0f, 1.0f,
        1.0f);
    Hu3DModelLayerSet(lbl_1_bss_4->mdlId[0], 3);
    fn_1_2001C(lbl_1_bss_8->mdlId[0], NULL, &specialPos[1]);
    Hu3DModelScaleSet(lbl_1_bss_8->mdlId[0], 1.0f, 1.0f,
        1.0f);
    Hu3DModelLayerSet(lbl_1_bss_8->mdlId[0], 3);
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrReset(lbl_1_bss_C->mdlId[i], 1);
        Hu3DModelRotSet(lbl_1_bss_C->mdlId[i], 0.0f,
            0.0f, 0.0f);
    }
    Hu3DModelAttrReset(lbl_1_bss_4->mdlId[0], 1);
    Hu3DModelAttrReset(lbl_1_bss_8->mdlId[0], 1);
    Hu3DModelShadowReset(lbl_1_bss_4->mdlId[0]);
    Hu3DModelShadowReset(lbl_1_bss_8->mdlId[0]);
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

void fn_1_1A570(OMOBJ *obj)
{
    MDRESULT_PLAYER_SPRITE_TABLE spriteInfo = { {
        {0, 10, 10, {122.0f, 30.0f}, {1.0f, 1.0f}, 0.0f},
        {0, 10, 0, {142.0f, 30.0f}, {1.0f, 1.0f}, 0.0f},
        {0, 10, 0, {162.0f, 30.0f}, {1.0f, 1.0f}, 0.0f},
        {0, 10, 10, {122.0f, 58.0f}, {1.0f, 1.0f}, 0.0f},
        {0, 10, 0, {142.0f, 58.0f}, {1.0f, 1.0f}, 0.0f},
        {0, 10, 0, {162.0f, 58.0f}, {1.0f, 1.0f}, 0.0f},
        {1, 10, 0, {138.0f, -28.0f}, {1.0f, 1.0f}, 0.0f},
        {6, 40, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
        {7, 40, 0, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f},
        {30, 20, 0, {-149.0f, -39.0f}, {1.0f, 1.0f}, -45.0f},
        {32, 30, 0, {-140.0f, -22.0f}, {1.0f, 1.0f}, 180.0f},
        {32, 30, 0, {140.0f, 22.0f}, {1.0f, 1.0f}, 0.0f},
        {8, 10, 0, {-52.0f, 34.0f}, {1.0f, 1.0f}, 0.0f},
        {9, 10, 0, {-52.0f, 34.0f}, {1.0f, 1.0f}, 0.0f},
    }};
    MDRESULT_PLAYER_ALT_WORK *work;
    s16 player;
    s16 sprite;
    s16 member;

    player = 0;
    work = (MDRESULT_PLAYER_ALT_WORK *)lbl_1_bss_66C;
    for (; player < 2; player++, work++) {
        work->models[0] = Hu3DModelLink(obj->mdlId[0]);
        Hu3DModelLayerSet(work->models[0], 3);
        Hu3DMotionShiftSet(work->models[0], obj->mtnId[0],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        work->models[1] = Hu3DModelLink(obj->mdlId[1]);
        Hu3DModelLayerSet(work->models[1], 3);
        Hu3DMotionShiftSet(work->models[1], obj->mtnId[1],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        work->models[2] = Hu3DModelLink(obj->mdlId[2]);
        Hu3DModelLayerSet(work->models[2], 3);
        Hu3DMotionShiftSet(work->models[2], obj->mtnId[2],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);

        work->winId = HuWinExCreateFrame(0.0f, 0.0f,
            MDRESULT_PLAYER_WINDOW_WIDTH, MDRESULT_PLAYER_WINDOW_HEIGHT, -1, 0);
        HuWinDispOff(work->winId);
        HuWinBGTPLvlSet(work->winId, 0.0f);
        HuWinPriSet(work->winId, 0);
        HuWinAttrSet(work->winId, HUWIN_ATTR_ALIGN_CENTER);

        work->group = HuSprGrpCreate(12);
        for (sprite = 0; sprite < 12; sprite++) {
            if (spriteInfo.values[sprite].animNo != -1) {
                work->sprites[sprite] = HuSprCreate(
                    lbl_1_bss_11AC[spriteInfo.values[sprite].animNo],
                    spriteInfo.values[sprite].priority,
                    spriteInfo.values[sprite].bank);
                HuSprGrpMemberSet(work->group, sprite,
                    work->sprites[sprite]);
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

        work->secondGroup = HuSprGrpCreate(2);
        for (sprite = 12, member = 0; sprite < 14; sprite++, member++) {
            if (spriteInfo.values[sprite].animNo != -1) {
                work->secondSprites[member] = HuSprCreate(
                    lbl_1_bss_11AC[spriteInfo.values[sprite].animNo],
                    spriteInfo.values[sprite].priority,
                    spriteInfo.values[sprite].bank);
                HuSprGrpMemberSet(work->secondGroup, member,
                    work->secondSprites[member]);
                HuSprPosSet(work->secondGroup, member,
                    spriteInfo.values[sprite].pos.x,
                    spriteInfo.values[sprite].pos.y);
                HuSprScaleSet(work->secondGroup, member,
                    spriteInfo.values[sprite].scale.x,
                    spriteInfo.values[sprite].scale.y);
                HuSprZRotSet(work->secondGroup, member,
                    spriteInfo.values[sprite].zRot);
            }
        }
    }
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

        fn_1_18E14();
    } else {
        fn_1_1A468();
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
        fn_1_18E14();
    } else {
        fn_1_1A468();
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

void fn_1_1B194(OMOBJ *obj)
{
    HUSPR_GROUPID *group = lbl_1_bss_3D2;

    if (obj->work[0]++ > 10) {
        if (group[MDRESULT_GROUP_VIEW_MODE] == 0 &&
            (HuPadDStkRep[0] & PAD_BUTTON_LEFT)) {
            group[MDRESULT_GROUP_CURSOR_X]--;
            obj->work[0] = 0;
            if (group[MDRESULT_GROUP_CURSOR_X] < 0) {
                group[MDRESULT_GROUP_CURSOR_X] = 0;
                group[MDRESULT_GROUP_CURSOR_Y]--;
                if (group[MDRESULT_GROUP_CURSOR_Y] < 0) {
                    group[MDRESULT_GROUP_CURSOR_Y] = 0;
                    obj->work[0] = 20;
                }
            }
            if (obj->work[0] == 0) {
                HuAudFXPlay(0);
            }
        } else if (group[MDRESULT_GROUP_VIEW_MODE] == 0 &&
            (HuPadDStkRep[0] & PAD_BUTTON_RIGHT)) {
            group[MDRESULT_GROUP_CURSOR_X]++;
            obj->work[0] = 0;
            if (group[MDRESULT_GROUP_CURSOR_X] > 4) {
                group[MDRESULT_GROUP_CURSOR_X] = 4;
                group[MDRESULT_GROUP_CURSOR_Y]++;
                if (group[MDRESULT_GROUP_CURSOR_Y] >
                    lbl_1_data_3A8[lbl_1_bss_1278.values[0]] - 5) {
                    group[MDRESULT_GROUP_CURSOR_Y] =
                        lbl_1_data_3A8[lbl_1_bss_1278.values[0]] - 5;
                    obj->work[0] = 20;
                }
            }
            if (obj->work[0] == 0) {
                HuAudFXPlay(0);
            }
        } else if (group[MDRESULT_GROUP_VIEW_MODE] != 0 &&
            (HuPadDStkRep[0] & PAD_BUTTON_UP)) {
            HuAudFXPlay(0);
            group[MDRESULT_GROUP_GRAPH_INDEX]--;
            if (group[MDRESULT_GROUP_GRAPH_INDEX] < 0) {
                group[MDRESULT_GROUP_GRAPH_INDEX] = 3;
            }
            fn_1_2ED4(group[MDRESULT_GROUP_GRAPH_INDEX]);
            obj->work[0] = 0;
        } else if (group[MDRESULT_GROUP_VIEW_MODE] != 0 &&
            (HuPadDStkRep[0] & PAD_BUTTON_DOWN)) {
            HuAudFXPlay(0);
            group[MDRESULT_GROUP_GRAPH_INDEX]++;
            if (group[MDRESULT_GROUP_GRAPH_INDEX] > 3) {
                group[MDRESULT_GROUP_GRAPH_INDEX] = 0;
            }
            fn_1_2ED4(group[MDRESULT_GROUP_GRAPH_INDEX]);
            obj->work[0] = 0;
        } else if (HuPadBtnDown[0] & PAD_TRIGGER_R) {
            HuAudFXPlay(0);
            group[MDRESULT_GROUP_VIEW_MODE]++;
            group[MDRESULT_GROUP_VIEW_MODE] %= 3;
            fn_1_1BAF4();
            obj->work[0] = 0;
        }
    }

    if (group[MDRESULT_GROUP_VIEW_MODE] == 0) {
        fn_1_1E28(1, lbl_1_data_3B4[lbl_1_bss_1278.values[0]].values[
            group[MDRESULT_GROUP_CURSOR_X] + group[MDRESULT_GROUP_CURSOR_Y]].message, 0);
    } else if (group[MDRESULT_GROUP_VIEW_MODE] == 1) {
        fn_1_27A4(1, lbl_1_bss_1278.messages[group[MDRESULT_GROUP_GRAPH_INDEX]], 0);
        fn_1_1E28(1, MDRESULT_MESSAGE_PARTY_GRAPH_STAR, 0);
    } else if (group[MDRESULT_GROUP_VIEW_MODE] == 2) {
        fn_1_27A4(1, lbl_1_bss_1278.messages[group[MDRESULT_GROUP_GRAPH_INDEX]], 0);
        fn_1_1E28(1, MDRESULT_MESSAGE_PARTY_GRAPH_COIN, 0);
    }

    lbl_1_data_754 = fn_1_1F8BC(lbl_1_data_754,
        (float)(162 + (76 * group[MDRESULT_GROUP_CURSOR_X])), 3.0f);
    lbl_1_bss_4C = fn_1_1F8BC(lbl_1_bss_4C,
        (float)-(76 * group[MDRESULT_GROUP_CURSOR_Y]), 3.0f);
    HuSprPosSet(group[0], 4, lbl_1_data_754, 231.0f);
    HuSprGrpPosSet(group[31], lbl_1_bss_4C, -15.0f);
}

void fn_1_1BAF4(void)
{
    HUSPR_GROUPID *group = lbl_1_bss_3D2;
    s16 graph[4][15];
    s16 mode;
    s16 i;
    s16 j;

    fn_1_1C050();
    fn_1_20188(group[0], 4);
    fn_1_20188(group[31], 4);

    mode = lbl_1_bss_1278.values[0];
    for (i = 0; i < 4; i++) {
        lbl_1_bss_10D4[i].values[3] = lbl_1_bss_10D4[i].star;
        for (j = 0; j < lbl_1_data_3A8[mode]; j++) {
            graph[i][j] = (&lbl_1_bss_10D4[i].values[3])[j];
            if (graph[i][j] >= 999) {
                graph[i][j] = 999;
            }
            fn_1_2035C(group[31], (s16)((j * 3) + (i * 15 * 3)), graph[i][j]);
        }
    }
    for (j = 0; j < lbl_1_data_3A8[mode]; j++) {
        HuSprBankSet(group[31], j + MDRESULT_GRAPH_BANK_BASE,
            lbl_1_data_3B4[mode].values[j].bank);
    }
    for (i = 0; i < 4; i++) {
        HuSprBankSet(group[0], i + 5, lbl_1_bss_1248[i].character);
    }

    if (group[MDRESULT_GROUP_VIEW_MODE] == 0) {
        fn_1_30C4();
    } else if (group[MDRESULT_GROUP_VIEW_MODE] == 1) {
        fn_1_20188(group[291], 4);
        HuSprAttrSet(group[0], 4, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group[0], 2, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group[0], 3, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group[291], 0, HUSPR_ATTR_DISPOFF);
        fn_1_1F7FC();
        fn_1_2F80(group[MDRESULT_GROUP_GRAPH_INDEX]);
    } else if (group[MDRESULT_GROUP_VIEW_MODE] == 2) {
        fn_1_20188(group[291], 4);
        HuSprAttrSet(group[0], 4, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group[0], 2, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group[0], 3, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group[291], 1, HUSPR_ATTR_DISPOFF);
        fn_1_1F7FC();
        fn_1_2F80(group[MDRESULT_GROUP_GRAPH_INDEX]);
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

void fn_1_1C0C8(OMOBJ *obj)
{
    HUSPR_GROUPID *group = lbl_1_bss_3D2;
    MDRESULT_PLAYER_SPRITE_TABLE_17 spriteInfo = { {
        {13, 90, 0, {288.0f, 240.0f}, {1.0f, 1.0f}, 0.0f},
        {14, 60, 0, {315.0f, 231.0f}, {1.0f, 1.0f}, 0.0f},
        {15, 50, 0, {111.0f, 245.0f}, {1.0f, 1.0f}, 0.0f},
        {15, 50, 1, {517.0f, 245.0f}, {1.0f, 1.0f}, 0.0f},
        {16, 10, 0, {314.0f, 231.0f}, {1.0f, 1.0f}, 0.0f},
        {17, 50, 0, {83.0f, 179.0f}, {1.0f, 1.0f}, 0.0f},
        {17, 50, 0, {62.0f, 227.0f}, {1.0f, 1.0f}, 0.0f},
        {17, 50, 0, {62.0f, 281.0f}, {1.0f, 1.0f}, 0.0f},
        {17, 50, 0, {83.0f, 329.0f}, {1.0f, 1.0f}, 0.0f},
        {18, 80, 0, {288.0f, 179.0f}, {27.0f, 1.0f}, 0.0f},
        {19, 80, 0, {288.0f, 229.0f}, {27.0f, 1.0f}, 0.0f},
        {20, 80, 0, {288.0f, 279.0f}, {27.0f, 1.0f}, 0.0f},
        {21, 80, 0, {288.0f, 329.0f}, {27.0f, 1.0f}, 0.0f},
        {-1, 0, 0, {346.0f, 168.0f}, {1.2f, 1.0f}, 0.0f},
        {-1, 0, 0, {802.0f, 168.0f}, {1.3f, 1.0f}, 180.0f},
        {-1, 0, 0, {346.0f, 294.0f}, {1.2f, 1.0f}, 0.0f},
        {-1, 0, 0, {802.0f, 294.0f}, {1.3f, 1.0f}, 180.0f},
    }};
    s16 i;
    s16 j;
    s16 row;
    s16 col;

    group[0] = HuSprGrpCreate(13);
    group[31] = HuSprGrpCreate(MDRESULT_GRAPH_GROUP_CAPACITY);
    group[291] = HuSprGrpCreate(MDRESULT_GRAPH_MODE_GROUP_CAPACITY);

    for (i = 0; i < 13; i++) {
        if (spriteInfo.values[i].animNo != -1) {
            (group + 1)[i] = HuSprCreate(
                lbl_1_bss_11AC[spriteInfo.values[i].animNo],
                spriteInfo.values[i].priority, spriteInfo.values[i].bank);
            HuSprGrpMemberSet(group[0], i, (group + 1)[i]);
            HuSprPosSet(group[0], i, spriteInfo.values[i].pos.x,
                spriteInfo.values[i].pos.y);
            HuSprScaleSet(group[0], i, spriteInfo.values[i].scale.x,
                spriteInfo.values[i].scale.y);
            HuSprZRotSet(group[0], i, spriteInfo.values[i].zRot);
        }
    }

    col = row = 0;
    for (i = 0, j = 0; i < MDRESULT_GRAPH_GRID_END; i++, j++) {
        if ((j % 45) < 36) {
            (group + 32)[i] = HuSprCreate(lbl_1_bss_11AC[0],
                MDRESULT_GRAPH_GRID_SPRITE_PRIORITY, 0);
            HuSprGrpMemberSet(group[31], i, (group + 32)[i]);
            if (j != 0 && (j % 3) == 0) {
                col++;
            }
            if (j != 0 && (j % 45) == 0) {
                row++;
                col = 0;
            }
            HuSprPosSet(group[31], i,
                (float)(j % 3 * 20 + 142 + col * 76),
                (float)(row * 50 + 179));
        }
    }

    col = row = 0;
    for (i = MDRESULT_GRAPH_GRID_END, j = 0; i < MDRESULT_GRAPH_WIDE_END; i++, j++) {
        (group + 32)[i] = HuSprCreate(
            lbl_1_bss_11AC[26],
            MDRESULT_GRAPH_WIDE_SPRITE_PRIORITY, 0);
        HuSprGrpMemberSet(group[31], i, (group + 32)[i]);
        if (j != 0) {
            col++;
        }
        if (j != 0 && (j % 15) == 0) {
            row++;
            col = 0;
        }
        HuSprPosSet(group[31], i,
            (float)(col * 76 + 162),
            (float)(row * 50 + 179));
    }

    col = row = 0;
    for (i = MDRESULT_GRAPH_LINE_START, j = 0; i < MDRESULT_GRAPH_LINE_END; i++, j++) {
        (group + 32)[i] = HuSprCreate(
            lbl_1_bss_11AC[24],
            MDRESULT_GRAPH_GRID_SPRITE_PRIORITY, 0);
        HuSprGrpMemberSet(group[31], i, (group + 32)[i]);
        if (j != 0) {
            col++;
        }
        HuSprPosSet(group[31], i, (float)(col * 76 + 162),
            134.0f);
    }

    for (i = MDRESULT_GRAPH_LINE_END, j = 13; i < MDRESULT_GRAPH_SELECTED_END; i++, j++) {
        (group + 32)[i] = HuSprCreate(
            lbl_1_bss_11AC[MDRESULT_GRAPH_SELECTED_SPRITE_ANIM],
            MDRESULT_GRAPH_SELECTED_SPRITE_PRIORITY, 0);
        HuSprGrpMemberSet(group[31], i, (group + 32)[i]);
        HuSprPosSet(group[31], i, spriteInfo.values[j].pos.x,
            spriteInfo.values[j].pos.y);
        HuSprScaleSet(group[31], i, spriteInfo.values[j].scale.x,
            spriteInfo.values[j].scale.y);
        HuSprZRotSet(group[31], i, spriteInfo.values[j].zRot);
    }

    group[292] = HuSprCreate(
        lbl_1_bss_11AC[MDRESULT_GRAPH_MODE_LEFT_SPRITE_ANIM],
        MDRESULT_GRAPH_MODE_SPRITE_PRIORITY, 0);
    HuSprGrpMemberSet(group[291], 0, group[292]);
    HuSprPosSet(group[291], 0, 315.0f, 231.0f);
    group[293] = HuSprCreate(
        lbl_1_bss_11AC[MDRESULT_GRAPH_MODE_RIGHT_SPRITE_ANIM],
        MDRESULT_GRAPH_MODE_SPRITE_PRIORITY, 0);
    HuSprGrpMemberSet(group[291], 1, group[293]);
    HuSprPosSet(group[291], 1, 315.0f, 231.0f);

    col = row = 0;
    for (i = 2, j = 0; i < MDRESULT_GRAPH_MODE_GROUP_CAPACITY; i++, j++) {
        (group + 292)[i] = HuSprCreate(
            lbl_1_bss_11AC[MDRESULT_GRAPH_VALUE_SPRITE_ANIM],
            MDRESULT_GRAPH_VALUE_SPRITE_PRIORITY, 0);
        HuSprGrpMemberSet(group[291], i, (group + 292)[i]);
        if (j != 0) {
            col++;
        }
        if (j != 0 && (j % 7) == 0) {
            row++;
            col = 0;
        }
        HuSprPosSet(group[291], i,
            (float)(col * 54 + 153),
            (float)(row * 50 + 131));
    }

    HuSprGrpPosSet(group[0], 0.0f, -15.0f);
    HuSprGrpPosSet(group[31], 0.0f, -15.0f);
    HuSprGrpPosSet(group[291], 0.0f, -15.0f);
    HuSprGrpDrawNoSet(group[0], MDRESULT_GRAPH_DRAW_NO);
    HuSprGrpDrawNoSet(group[31], MDRESULT_GRAPH_DRAW_NO);
    HuSprGrpDrawNoSet(group[291], MDRESULT_GRAPH_DRAW_NO);
    HuSprGrpScissorSet(group[31], MDRESULT_GRAPH_SCISSOR_X,
        MDRESULT_GRAPH_SCISSOR_Y, MDRESULT_GRAPH_SCISSOR_WIDTH,
        MDRESULT_GRAPH_SCISSOR_HEIGHT);
    group[MDRESULT_GROUP_CURSOR_X] = 0;
    group[MDRESULT_GROUP_CURSOR_Y] = 0;
    group[MDRESULT_GROUP_GRAPH_INDEX] = 0;
    group[MDRESULT_GROUP_VIEW_MODE] = 0;
}

void fn_1_1C9A0(void)
{
    HUSPR_GROUPID *group = lbl_1_bss_3D2;
}

void fn_1_1C9B8(OMOBJ *obj)
{
    HUSPR_GROUPID *group = lbl_1_bss_3D2;

    if (obj->work[0]++ > 10) {
        if (group[MDRESULT_GROUP_VIEW_MODE] == 0 &&
            (HuPadDStkRep[0] & PAD_BUTTON_LEFT)) {
            group[MDRESULT_GROUP_CURSOR_X]--;
            obj->work[0] = 0;
            if (group[MDRESULT_GROUP_CURSOR_X] < 0) {
                group[MDRESULT_GROUP_CURSOR_X] = 0;
                group[MDRESULT_GROUP_CURSOR_Y]--;
                if (group[MDRESULT_GROUP_CURSOR_Y] < 0) {
                    group[MDRESULT_GROUP_CURSOR_Y] = 0;
                    obj->work[0] = 20;
                }
            }
            if (obj->work[0] == 0) {
                HuAudFXPlay(0);
            }
        } else if (group[MDRESULT_GROUP_VIEW_MODE] == 0 &&
            (HuPadDStkRep[0] & PAD_BUTTON_RIGHT)) {
            group[MDRESULT_GROUP_CURSOR_X]++;
            obj->work[0] = 0;
            if (group[MDRESULT_GROUP_CURSOR_X] > 4) {
                group[MDRESULT_GROUP_CURSOR_X] = 4;
                group[MDRESULT_GROUP_CURSOR_Y]++;
                if (group[MDRESULT_GROUP_CURSOR_Y] >
                    lbl_1_data_3A8[lbl_1_bss_1278.values[0]] - 5) {
                    group[MDRESULT_GROUP_CURSOR_Y] =
                        lbl_1_data_3A8[lbl_1_bss_1278.values[0]] - 5;
                    obj->work[0] = 20;
                }
            }
            if (obj->work[0] == 0) {
                HuAudFXPlay(0);
            }
        } else if (group[MDRESULT_GROUP_VIEW_MODE] != 0 &&
            (HuPadDStkRep[0] & PAD_BUTTON_UP)) {
            HuAudFXPlay(0);
            group[MDRESULT_GROUP_GRAPH_INDEX]--;
            if (group[MDRESULT_GROUP_GRAPH_INDEX] < 0) {
                group[MDRESULT_GROUP_GRAPH_INDEX] = 1;
            }
            fn_1_2ED4(group[MDRESULT_GROUP_GRAPH_INDEX]);
            obj->work[0] = 0;
        } else if (group[MDRESULT_GROUP_VIEW_MODE] != 0 &&
            (HuPadDStkRep[0] & PAD_BUTTON_DOWN)) {
            HuAudFXPlay(0);
            group[MDRESULT_GROUP_GRAPH_INDEX]++;
            if (group[MDRESULT_GROUP_GRAPH_INDEX] > 1) {
                group[MDRESULT_GROUP_GRAPH_INDEX] = 0;
            }
            fn_1_2ED4(group[MDRESULT_GROUP_GRAPH_INDEX]);
            obj->work[0] = 0;
        } else if (HuPadBtnDown[0] & PAD_TRIGGER_R) {
            HuAudFXPlay(0);
            group[MDRESULT_GROUP_VIEW_MODE]++;
            group[MDRESULT_GROUP_VIEW_MODE] %= 3;
            fn_1_1D318();
            obj->work[0] = 0;
        }
    }

    if (group[MDRESULT_GROUP_VIEW_MODE] == 0) {
        fn_1_1E28(1, lbl_1_data_3B4[lbl_1_bss_1278.values[0]].values[
            group[MDRESULT_GROUP_CURSOR_X] + group[MDRESULT_GROUP_CURSOR_Y]].message, 0);
    } else if (group[MDRESULT_GROUP_VIEW_MODE] == 1) {
        fn_1_27A4(1, (lbl_1_bss_1278.messages + 4)[group[MDRESULT_GROUP_GRAPH_INDEX]], 0);
        fn_1_1E28(1, MDRESULT_MESSAGE_TEAM_GRAPH_STAR, 0);
    } else if (group[MDRESULT_GROUP_VIEW_MODE] == 2) {
        fn_1_27A4(1, (lbl_1_bss_1278.messages + 4)[group[MDRESULT_GROUP_GRAPH_INDEX]], 0);
        fn_1_1E28(1, MDRESULT_MESSAGE_TEAM_GRAPH_COIN, 0);
    }

    lbl_1_data_758 = fn_1_1F8BC(lbl_1_data_758,
        (float)(162 + (76 * group[MDRESULT_GROUP_CURSOR_X])), 3.0f);
    lbl_1_bss_50 = fn_1_1F8BC(lbl_1_bss_50,
        (float)-(76 * group[MDRESULT_GROUP_CURSOR_Y]), 3.0f);
    HuSprPosSet(group[0], 4, lbl_1_data_758, 231.0f);
    HuSprGrpPosSet(group[31], lbl_1_bss_50, -15.0f);
}

void fn_1_1D318(void)
{
    HUSPR_GROUPID *group = lbl_1_bss_3D2;
    s16 graph[4][15];
    s16 mode;
    s16 i;
    s16 j;

    fn_1_1D874();
    fn_1_20188(group[0], 4);
    fn_1_20188(group[31], 4);

    mode = lbl_1_bss_1278.values[0];
    for (i = 0; i < 2; i++) {
        lbl_1_bss_10D4[i].values[3] = lbl_1_bss_10D4[i].star;
        for (j = 0; j < lbl_1_data_3A8[mode]; j++) {
            graph[i][j] = (&lbl_1_bss_10D4[i].values[3])[j];
            if (graph[i][j] >= 999) {
                graph[i][j] = 999;
            }
            fn_1_2035C(group[31], (s16)((j * 3) + (i * 15 * 3)), graph[i][j]);
        }
    }
    for (j = 0; j < lbl_1_data_3A8[mode]; j++) {
        HuSprBankSet(group[31], j + MDRESULT_GRAPH_BANK_BASE,
            lbl_1_data_3B4[mode].values[j].bank);
    }
    for (i = 0; i < 4; i++) {
        HuSprBankSet(group[0], i + 5, lbl_1_bss_1248[i].character);
    }

    if (group[MDRESULT_GROUP_VIEW_MODE] == 0) {
        fn_1_30C4();
    } else if (group[MDRESULT_GROUP_VIEW_MODE] == 1) {
        fn_1_20188(group[291], 4);
        HuSprAttrSet(group[0], 4, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group[0], 2, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group[0], 3, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group[291], 0, HUSPR_ATTR_DISPOFF);
        fn_1_1F7FC();
        fn_1_2F80(group[MDRESULT_GROUP_GRAPH_INDEX]);
    } else if (group[MDRESULT_GROUP_VIEW_MODE] == 2) {
        fn_1_20188(group[291], 4);
        HuSprAttrSet(group[0], 4, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group[0], 2, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group[0], 3, HUSPR_ATTR_DISPOFF);
        HuSprAttrSet(group[291], 1, HUSPR_ATTR_DISPOFF);
        fn_1_1F7FC();
        fn_1_2F80(group[MDRESULT_GROUP_GRAPH_INDEX]);
    }
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

void fn_1_1D8EC(OMOBJ *obj)
{
    HUSPR_GROUPID *group = lbl_1_bss_3D2;
    MDRESULT_PLAYER_SPRITE_TABLE_15 spriteInfo = { {
        {13, 90, 0, {288.0f, 240.0f}, {1.0f, 1.0f}, 0.0f},
        {14, 60, 0, {315.0f, 231.0f}, {1.0f, 1.0f}, 0.0f},
        {15, 50, 0, {111.0f, 245.0f}, {1.0f, 1.0f}, 0.0f},
        {15, 50, 1, {517.0f, 245.0f}, {1.0f, 1.0f}, 0.0f},
        {16, 10, 0, {314.0f, 231.0f}, {1.0f, 1.0f}, 0.0f},
        {17, 50, 0, {83.0f, 179.0f}, {1.0f, 1.0f}, 0.0f},
        {17, 50, 0, {62.0f, 227.0f}, {1.0f, 1.0f}, 0.0f},
        {17, 50, 0, {62.0f, 281.0f}, {1.0f, 1.0f}, 0.0f},
        {17, 50, 0, {83.0f, 329.0f}, {1.0f, 1.0f}, 0.0f},
        {22, 80, 0, {288.0f, 204.0f}, {27.0f, 1.0f}, 0.0f},
        {23, 80, 0, {288.0f, 304.0f}, {27.0f, 1.0f}, 0.0f},
        {-1, 0, 0, {346.0f, 168.0f}, {1.2f, 1.0f}, 0.0f},
        {-1, 0, 0, {802.0f, 168.0f}, {1.3f, 1.0f}, 180.0f},
        {-1, 0, 0, {346.0f, 294.0f}, {1.2f, 1.0f}, 0.0f},
        {-1, 0, 0, {802.0f, 294.0f}, {1.3f, 1.0f}, 180.0f},
    }};
    s16 i;
    s16 j;
    s16 row;
    s16 col;

    group[0] = HuSprGrpCreate(MDRESULT_TEAM_RESULT_GROUP_CAPACITY);
    group[31] = HuSprGrpCreate(MDRESULT_GRAPH_GROUP_CAPACITY);
    group[291] = HuSprGrpCreate(MDRESULT_GRAPH_MODE_GROUP_CAPACITY);

    for (i = 0; i < 11; i++) {
        if (spriteInfo.values[i].animNo != -1) {
            (group + 1)[i] = HuSprCreate(
                lbl_1_bss_11AC[spriteInfo.values[i].animNo],
                spriteInfo.values[i].priority, spriteInfo.values[i].bank);
            HuSprGrpMemberSet(group[0], i, (group + 1)[i]);
            HuSprPosSet(group[0], i, spriteInfo.values[i].pos.x,
                spriteInfo.values[i].pos.y);
            HuSprScaleSet(group[0], i, spriteInfo.values[i].scale.x,
                spriteInfo.values[i].scale.y);
            HuSprZRotSet(group[0], i, spriteInfo.values[i].zRot);
        }
    }

    col = row = 0;
    for (i = 0, j = 0; i < 90; i++, j++) {
        (group + 32)[i] = HuSprCreate(lbl_1_bss_11AC[0],
            MDRESULT_GRAPH_GRID_SPRITE_PRIORITY, 0);
        HuSprGrpMemberSet(group[31], i, (group + 32)[i]);
        if (j != 0 && (j % 3) == 0) {
            col++;
        }
        if (j != 0 && (j % 45) == 0) {
            row++;
            col = 0;
        }
        HuSprPosSet(group[31], i,
            (float)(j % 3 * 20 + 142 + col * 76),
            (float)(row * 100 + 204));
    }

    col = row = 0;
    for (i = MDRESULT_GRAPH_GRID_END, j = 0; i < MDRESULT_TEAM_GRAPH_WIDE_END; i++, j++) {
        (group + 32)[i] = HuSprCreate(
            lbl_1_bss_11AC[27],
            MDRESULT_GRAPH_WIDE_SPRITE_PRIORITY, 0);
        HuSprGrpMemberSet(group[31], i, (group + 32)[i]);
        if (j != 0) {
            col++;
        }
        if (j != 0 && (j % 15) == 0) {
            row++;
            col = 0;
        }
        HuSprPosSet(group[31], i,
            (float)(col * 76 + 162),
            (float)(row * 100 + 204));
    }

    col = row = 0;
    for (i = MDRESULT_GRAPH_LINE_START, j = 0; i < MDRESULT_GRAPH_LINE_END; i++, j++) {
        (group + 32)[i] = HuSprCreate(
            lbl_1_bss_11AC[24],
            MDRESULT_GRAPH_GRID_SPRITE_PRIORITY, 0);
        HuSprGrpMemberSet(group[31], i, (group + 32)[i]);
        if (j != 0) {
            col++;
        }
        HuSprPosSet(group[31], i, (float)(col * 76 + 162),
            134.0f);
    }

    for (i = MDRESULT_GRAPH_LINE_END, j = 11; i < MDRESULT_GRAPH_SELECTED_END; i++, j++) {
        (group + 32)[i] = HuSprCreate(
            lbl_1_bss_11AC[MDRESULT_GRAPH_SELECTED_SPRITE_ANIM],
            MDRESULT_GRAPH_SELECTED_SPRITE_PRIORITY, 0);
        HuSprGrpMemberSet(group[31], i, (group + 32)[i]);
        HuSprPosSet(group[31], i, spriteInfo.values[j].pos.x,
            spriteInfo.values[j].pos.y);
        HuSprScaleSet(group[31], i, spriteInfo.values[j].scale.x,
            spriteInfo.values[j].scale.y);
        HuSprZRotSet(group[31], i, spriteInfo.values[j].zRot);
    }

    group[292] = HuSprCreate(
        lbl_1_bss_11AC[MDRESULT_GRAPH_MODE_LEFT_SPRITE_ANIM],
        MDRESULT_GRAPH_MODE_SPRITE_PRIORITY, 0);
    HuSprGrpMemberSet(group[291], 0, group[292]);
    HuSprPosSet(group[291], 0, 315.0f, 231.0f);
    group[293] = HuSprCreate(
        lbl_1_bss_11AC[MDRESULT_GRAPH_MODE_RIGHT_SPRITE_ANIM],
        MDRESULT_GRAPH_MODE_SPRITE_PRIORITY, 0);
    HuSprGrpMemberSet(group[291], 1, group[293]);
    HuSprPosSet(group[291], 1, 315.0f, 231.0f);

    col = row = 0;
    for (i = 2, j = 0; i < MDRESULT_GRAPH_MODE_GROUP_CAPACITY; i++, j++) {
        (group + 292)[i] = HuSprCreate(
            lbl_1_bss_11AC[MDRESULT_GRAPH_VALUE_SPRITE_ANIM],
            MDRESULT_GRAPH_VALUE_SPRITE_PRIORITY, 0);
        HuSprGrpMemberSet(group[291], i, (group + 292)[i]);
        if (j != 0) {
            col++;
        }
        if (j != 0 && (j % 7) == 0) {
            row++;
            col = 0;
        }
        HuSprPosSet(group[291], i,
            (float)(col * 54 + 153),
            (float)(row * 50 + 131));
    }

    HuSprGrpPosSet(group[0], 0.0f, -15.0f);
    HuSprGrpPosSet(group[31], 0.0f, -15.0f);
    HuSprGrpPosSet(group[291], 0.0f, -15.0f);
    HuSprGrpDrawNoSet(group[0], MDRESULT_GRAPH_DRAW_NO);
    HuSprGrpDrawNoSet(group[31], MDRESULT_GRAPH_DRAW_NO);
    HuSprGrpDrawNoSet(group[291], MDRESULT_GRAPH_DRAW_NO);
    HuSprGrpScissorSet(group[31], MDRESULT_GRAPH_SCISSOR_X,
        MDRESULT_GRAPH_SCISSOR_Y, MDRESULT_GRAPH_SCISSOR_WIDTH,
        MDRESULT_GRAPH_SCISSOR_HEIGHT);
    group[MDRESULT_GROUP_CURSOR_X] = 0;
    group[MDRESULT_GROUP_CURSOR_Y] = 0;
    group[MDRESULT_GROUP_GRAPH_INDEX] = 0;
    group[MDRESULT_GROUP_VIEW_MODE] = 0;
}

void fn_1_1E19C(void)
{
    HUSPR_GROUPID *group = lbl_1_bss_3D2;
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
    lbl_1_bss_3C[0]->objFunc = fn_1_1E1B4;
}

void fn_1_1E258(void)
{
    if (lbl_1_bss_1278.values[3] == 0) {
        fn_1_1C050();
    } else {
        fn_1_1D874();
    }
    lbl_1_bss_3C[0]->objFunc = NULL;
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

void fn_1_1E47C(void)
{
    if (lbl_1_bss_1278.values[3] == 0) {
        HUSPR_GROUPID *group = lbl_1_bss_3D2;
    } else {
        HUSPR_GROUPID *group = lbl_1_bss_3D2;
    }
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
    if (cross < 0.0f) {
        absCross = -cross;
    } else {
        absCross = cross;
    }
    if (absCross < 0.001f) {
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

void fn_1_1E5E8(HUSPRITE *sprite)
{
    s16 graphCount;
    s16 i;
    s16 j;
    Mtx matrix;
    s16 playerCount = 0;
    HUSPR_GROUPID *group = lbl_1_bss_3D2;
    HuVecF points[55];
    HuVecF offsets[55][2];
    HuVecF direction;
    float max;

    PSMTXScale(matrix, sprite->scale.x, sprite->scale.y,
        1.0f);
    mtxTransCat(matrix, sprite->pos.x, sprite->pos.y,
        0.0f);
    PSMTXConcat(*sprite->groupMtx, matrix, matrix);
    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
        GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0,
        GX_DF_CLAMP, GX_AF_NONE);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC,
        GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA,
        GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    HuSprTexLoad(lbl_1_bss_5C, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP,
        GX_LINEAR);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    graphCount = lbl_1_bss_1278.values[1];
    if (lbl_1_bss_1278.values[2] == 1 &&
        group[MDRESULT_GROUP_VIEW_MODE] == 1) {
        graphCount++;
    }
    lbl_1_bss_54 = group[MDRESULT_GROUP_GRAPH_INDEX];
    if (lbl_1_bss_1278.values[3] == 1) {
        playerCount = 2;
    } else {
        playerCount = 4;
    }
    max = 0.0f;
    for (i = 0; i < playerCount; i++) {
        for (j = 0; j <= graphCount; j++) {
            if (group[MDRESULT_GROUP_VIEW_MODE] == 1) {
                if (max < lbl_1_bss_62[i][j]) {
                    max = lbl_1_bss_62[i][j];
                }
            } else if (max < lbl_1_bss_21A[i][j]) {
                max = lbl_1_bss_21A[i][j];
            }
        }
    }
    max = 225.0f / max;

    for (i = 0; i < playerCount; i++) {
        for (j = 0; j <= graphCount; j++) {
            points[j].x = (float)j *
                (370.0f / (float)graphCount);
            if (group[MDRESULT_GROUP_VIEW_MODE] == 1) {
                points[j].y = (float)(-lbl_1_bss_62[i][j]);
                points[j].y *= max;
                lbl_1_bss_56 = 1;
            } else {
                points[j].y = (float)(-lbl_1_bss_21A[i][j]);
                points[j].y *= max;
                lbl_1_bss_58 = 1;
            }
            points[j].z = 0.0f;
        }

        for (j = 0; j <= graphCount; j++) {
            if (j < graphCount) {
                direction.x = points[j + 1].x - points[j].x;
                direction.y = points[j + 1].y - points[j].y;
                direction.z = 0.0f;
                PSVECNormalize(&direction, &direction);
            }
            offsets[j][0].x = 3.0f * -direction.y;
            offsets[j][0].y = 3.0f * direction.x;
            offsets[j][1].x = 3.0f * direction.y;
            offsets[j][1].y = 3.0f * -direction.x;
        }

        {
            u8 alpha = MDRESULT_GRAPH_ALPHA_DIM;

            if (i == lbl_1_bss_54) {
                alpha = MDRESULT_COLOR_MAX;
            }

            for (j = 0; j < graphCount; j++) {
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                direction.x = points[j].x + offsets[j][0].x;
                direction.y = points[j].y + offsets[j][0].y;
                GXPosition3f32(direction.x, direction.y, 0.0f);
                GXColor4u8(lbl_1_data_75C[i].r,
                    lbl_1_data_75C[i].g, lbl_1_data_75C[i].b, alpha);
                GXTexCoord2f32(0.0f, 1.0f);
                direction.x = points[j].x + offsets[j][1].x;
                direction.y = points[j].y + offsets[j][1].y;
                GXPosition3f32(direction.x, direction.y, 0.0f);
                GXColor4u8(lbl_1_data_75C[i].r,
                    lbl_1_data_75C[i].g, lbl_1_data_75C[i].b, alpha);
                GXTexCoord2f32(0.0f, 0.0f);
                direction.x = points[j + 1].x + offsets[j][1].x;
                direction.y = points[j + 1].y + offsets[j][1].y;
                GXPosition3f32(direction.x, direction.y, 0.0f);
                GXColor4u8(lbl_1_data_75C[i].r,
                    lbl_1_data_75C[i].g, lbl_1_data_75C[i].b, alpha);
                GXTexCoord2f32(1.0f, 0.0f);
                direction.x = points[j + 1].x + offsets[j][0].x;
                direction.y = points[j + 1].y + offsets[j][0].y;
                GXPosition3f32(direction.x, direction.y, 0.0f);
                GXColor4u8(lbl_1_data_75C[i].r,
                    lbl_1_data_75C[i].g, lbl_1_data_75C[i].b, alpha);
                GXTexCoord2f32(1.0f, 1.0f);
            }
        }
    }
}

void fn_1_1F308(void)
{
    HUSPRID sprite;

    lbl_1_bss_60 = HuSprGrpCreate(2);
    sprite = HuSprFuncCreate(fn_1_1E5E8, 0);
    lbl_1_bss_5C = HuSprAnimRead(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 116), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprGrpMemberSet(lbl_1_bss_60, 0, sprite);
    HuSprPosSet(lbl_1_bss_60, 0, 130.0f, 335.0f);
    HuSprAttrSet(lbl_1_bss_60, 0, HUSPR_ATTR_DISPOFF);
}

void fn_1_1F3D4(void)
{
    s16 player;
    s16 graph;

    if (lbl_1_bss_1278.values[3] == 0) {
        for (player = 0; player < 4; player++) {
            for (graph = 0; graph <= lbl_1_bss_1278.values[1]; graph++) {
                lbl_1_bss_62[player][graph] =
                    GwPlayer[player].starGraph[graph];
                lbl_1_bss_21A[player][graph] =
                    GwPlayer[player].coinGraph[graph];
            }
            lbl_1_bss_62[player][0] =
                lbl_1_bss_10D4[player].values[15];
            lbl_1_bss_21A[player][0] = 10;
            if (lbl_1_bss_1278.values[2] == 1) {
                lbl_1_bss_62[player][graph] =
                    lbl_1_bss_10D4[player].star;
            }
        }
    } else {
        for (player = 0; player < 2; player++) {
            OSReport(lbl_1_data_76C, lbl_1_bss_10CC[player * 2],
                lbl_1_bss_10CC[(player * 2) + 1]);
            for (graph = 0; graph <= lbl_1_bss_1278.values[1]; graph++) {
                lbl_1_bss_62[player][graph] =
                    GwPlayer[lbl_1_bss_10CC[player * 2]].starGraph[graph]
                    + GwPlayer[lbl_1_bss_10CC[(player * 2) + 1]].starGraph[graph];
                lbl_1_bss_21A[player][graph] =
                    GwPlayer[lbl_1_bss_10CC[player * 2]].coinGraph[graph]
                    + GwPlayer[lbl_1_bss_10CC[(player * 2) + 1]].coinGraph[graph];
            }
            lbl_1_bss_62[player][0] =
                lbl_1_bss_10D4[player].values[15];
            lbl_1_bss_21A[player][0] = 20;
            if (lbl_1_bss_1278.values[2] == 1) {
                lbl_1_bss_62[player][graph] =
                    lbl_1_bss_10D4[player].star;
            }
        }
    }
}

void fn_1_1F7FC(void)
{
    fn_1_1F3D4();
    HuSprAttrReset(lbl_1_bss_60, 0, HUSPR_ATTR_DISPOFF);
}

void fn_1_1F834(void)
{
    HuSprAttrSet(lbl_1_bss_60, 0, HUSPR_ATTR_DISPOFF);
}

void fn_1_1F868(HuVecF *vec, float x, float y, float z)
{
    vec->x = x;
    vec->y = y;
    vec->z = z;
}





HuVecF lbl_1_data_0[16] = {
    { -270.0f, 0.0f, -200.0f },
    {  -90.0f, 0.0f, -200.0f },
    {   90.0f, 0.0f, -200.0f },
    {  270.0f, 0.0f, -200.0f },
    { -180.0f, 0.0f, -200.0f },
    {  180.0f, 0.0f, -200.0f },
    { -180.0f, 0.0f, -200.0f },
    {  180.0f, 0.0f, -200.0f },
    { -270.0f, 65.0f, -200.0f },
    {  -90.0f, 65.0f, -200.0f },
    {   90.0f, 65.0f, -200.0f },
    {  270.0f, 65.0f, -200.0f },
    { -250.0f, 65.0f, -200.0f },
    { -110.0f, 65.0f, -200.0f },
    {  110.0f, 65.0f, -200.0f },
    {  250.0f, 65.0f, -200.0f },
};


s32 lbl_1_data_C0[39] = {
    DATANUM(DATA_mdpresult, 82),
    DATANUM(DATA_mdpresult, 83),
    DATANUM(DATA_mdpresult, 84),
    DATANUM(DATA_mdpresult, 85),
    DATANUM(DATA_mdpresult, 86),
    DATANUM(DATA_mdpresult, 87),
    DATANUM(DATA_mdpresult, 88),
    DATANUM(DATA_mdpresult, 89),
    DATANUM(DATA_mdpresult, 90),
    DATANUM(DATA_mdpresult, 91),
    DATANUM(DATA_mdpresult, 76),
    DATANUM(DATA_mdpresult, 77),
    DATANUM(DATA_mdpresult, 78),
    DATANUM(DATA_mdpresult, 99),
    DATANUM(DATA_mdpresult, 100),
    DATANUM(DATA_mdpresult, 97),
    DATANUM(DATA_mdpresult, 98),
    DATANUM(DATA_mdpresult, 111),
    DATANUM(DATA_mdpresult, 105),
    DATANUM(DATA_mdpresult, 106),
    DATANUM(DATA_mdpresult, 107),
    DATANUM(DATA_mdpresult, 108),
    DATANUM(DATA_mdpresult, 110),
    DATANUM(DATA_mdpresult, 109),
    DATANUM(DATA_mdpresult, 112),
    DATANUM(DATA_mdpresult, 113),
    DATANUM(DATA_mdpresult, 114),
    DATANUM(DATA_mdpresult, 115),
    DATANUM(DATA_mdpresult, 101),
    DATANUM(DATA_mdpresult, 92),
    DATANUM(DATA_mdpresult, 93),
    DATANUM(DATA_mdpresult, 94),
    DATANUM(DATA_mdpresult, 95),
    DATANUM(DATA_mdpresult, 102),
    DATANUM(DATA_mdpresult, 103),
    DATANUM(DATA_mdpresult, 104),
    DATANUM(DATA_mdpresult, 74),
    DATANUM(DATA_mdpresult, 75),
    DATANUM(DATA_mdpresult, 117),
};


s16 lbl_1_data_15C[6] = {
    3, 3, 3, 3, 5, 1,
};


MDRESULT_SPRITE_INFO lbl_1_data_168[18] = {
    { 0, 0,  0, 0, 0, {   0.0f,   0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 0, 1,  0, 0, 0, {   0.0f,   0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 0, 2,  0, 0, 0, {   0.0f,   0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 0,  0, 0, 0, {   0.0f,   0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 1,  0, 0, 0, {   0.0f,   0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 2,  0, 0, 0, {   0.0f,   0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 0,  0, 0, 0, {   0.0f,   0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 1,  0, 0, 0, {   0.0f,   0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 2,  0, 0, 0, {   0.0f,   0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 3, 0,  0, 0, 0, {   0.0f,   0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 3, 1,  0, 0, 0, {   0.0f,   0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 3, 2,  0, 0, 0, {   0.0f,   0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 4, 0, 36, 0, 0, { 288.0f,  90.0f }, { 1.0f, 1.0f }, 0.0f },
    { 4, 1, 29, 0, 0, { 288.0f, 410.0f }, { 1.0f, 1.0f }, 0.0f },
    { 4, 2, 31, 5, 0, { 138.0f, 320.0f }, {-1.0f, 1.0f }, 0.0f },
    { 4, 3, 31, 5, 0, { 438.0f, 320.0f }, { 1.0f, 1.0f }, 0.0f },
    { 4, 4, 37, 0, 0, { 288.0f,  90.0f }, { 1.0f, 1.0f }, 0.0f },
    { 5, 0, 38, 0, 0, { 288.0f, 240.0f }, {10.0f,10.0f }, 0.0f },
};


s16 lbl_1_data_3A8[6] = {
    12, 12, 12, 12, 12, 10,
};




MDRESULT_GRAPH_TABLE lbl_1_data_3B4[6] = {
    { {     {  0, 0, MESSNUM(MESS_PARTY_RESULTS, 42) },     {  1, 0, MESSNUM(MESS_PARTY_RESULTS, 43) },     {  2, 0, MESSNUM(MESS_PARTY_RESULTS, 44) },     {  3, 0, MESSNUM(MESS_PARTY_RESULTS, 45) },     {  4, 0, MESSNUM(MESS_PARTY_RESULTS, 46) },     {  5, 0, MESSNUM(MESS_PARTY_RESULTS, 47) },     {  6, 0, MESSNUM(MESS_PARTY_RESULTS, 61) },     {  7, 0, MESSNUM(MESS_PARTY_RESULTS, 49) },     {  8, 0, MESSNUM(MESS_PARTY_RESULTS, 63) },     {  9, 0, MESSNUM(MESS_PARTY_RESULTS, 62) },     { 11, 0, MESSNUM(MESS_PARTY_RESULTS, 50) },     { 10, 0, MESSNUM(MESS_PARTY_RESULTS, 51) }, } },
    { {     {  0, 0, MESSNUM(MESS_PARTY_RESULTS, 42) },     {  1, 0, MESSNUM(MESS_PARTY_RESULTS, 43) },     {  2, 0, MESSNUM(MESS_PARTY_RESULTS, 44) },     {  3, 0, MESSNUM(MESS_PARTY_RESULTS, 45) },     {  4, 0, MESSNUM(MESS_PARTY_RESULTS, 46) },     {  5, 0, MESSNUM(MESS_PARTY_RESULTS, 47) },     {  6, 0, MESSNUM(MESS_PARTY_RESULTS, 61) },     {  7, 0, MESSNUM(MESS_PARTY_RESULTS, 49) },     {  8, 0, MESSNUM(MESS_PARTY_RESULTS, 63) },     {  9, 0, MESSNUM(MESS_PARTY_RESULTS, 62) },     { 11, 0, MESSNUM(MESS_PARTY_RESULTS, 50) },     { 10, 0, MESSNUM(MESS_PARTY_RESULTS, 51) }, } },
    { {     {  0, 0, MESSNUM(MESS_PARTY_RESULTS, 42) },     {  1, 0, MESSNUM(MESS_PARTY_RESULTS, 43) },     {  2, 0, MESSNUM(MESS_PARTY_RESULTS, 44) },     {  3, 0, MESSNUM(MESS_PARTY_RESULTS, 45) },     {  4, 0, MESSNUM(MESS_PARTY_RESULTS, 46) },     {  5, 0, MESSNUM(MESS_PARTY_RESULTS, 47) },     {  6, 0, MESSNUM(MESS_PARTY_RESULTS, 61) },     {  7, 0, MESSNUM(MESS_PARTY_RESULTS, 49) },     {  8, 0, MESSNUM(MESS_PARTY_RESULTS, 63) },     {  9, 0, MESSNUM(MESS_PARTY_RESULTS, 62) },     { 11, 0, MESSNUM(MESS_PARTY_RESULTS, 50) },     { 10, 0, MESSNUM(MESS_PARTY_RESULTS, 51) }, } },
    { {     {  0, 0, MESSNUM(MESS_PARTY_RESULTS, 42) },     {  1, 0, MESSNUM(MESS_PARTY_RESULTS, 43) },     {  2, 0, MESSNUM(MESS_PARTY_RESULTS, 44) },     {  3, 0, MESSNUM(MESS_PARTY_RESULTS, 45) },     {  4, 0, MESSNUM(MESS_PARTY_RESULTS, 46) },     {  5, 0, MESSNUM(MESS_PARTY_RESULTS, 47) },     {  6, 0, MESSNUM(MESS_PARTY_RESULTS, 61) },     {  7, 0, MESSNUM(MESS_PARTY_RESULTS, 49) },     {  8, 0, MESSNUM(MESS_PARTY_RESULTS, 63) },     {  9, 0, MESSNUM(MESS_PARTY_RESULTS, 62) },     { 11, 0, MESSNUM(MESS_PARTY_RESULTS, 50) },     { 10, 0, MESSNUM(MESS_PARTY_RESULTS, 51) }, } },
    { {     {  0, 0, MESSNUM(MESS_PARTY_RESULTS, 42) },     {  1, 0, MESSNUM(MESS_PARTY_RESULTS, 43) },     {  2, 0, MESSNUM(MESS_PARTY_RESULTS, 44) },     {  3, 0, MESSNUM(MESS_PARTY_RESULTS, 45) },     {  4, 0, MESSNUM(MESS_PARTY_RESULTS, 46) },     {  5, 0, MESSNUM(MESS_PARTY_RESULTS, 47) },     {  6, 0, MESSNUM(MESS_PARTY_RESULTS, 61) },     {  7, 0, MESSNUM(MESS_PARTY_RESULTS, 49) },     {  8, 0, MESSNUM(MESS_PARTY_RESULTS, 63) },     {  9, 0, MESSNUM(MESS_PARTY_RESULTS, 62) },     { 11, 0, MESSNUM(MESS_PARTY_RESULTS, 50) },     { 10, 0, MESSNUM(MESS_PARTY_RESULTS, 51) }, } },
    { {     {  0, 0, MESSNUM(MESS_PARTY_RESULTS, 42) },     {  1, 0, MESSNUM(MESS_PARTY_RESULTS, 43) },     {  2, 0, MESSNUM(MESS_PARTY_RESULTS, 44) },     {  3, 0, MESSNUM(MESS_PARTY_RESULTS, 45) },     {  4, 0, MESSNUM(MESS_PARTY_RESULTS, 46) },     {  5, 0, MESSNUM(MESS_PARTY_RESULTS, 47) },     {  6, 0, MESSNUM(MESS_PARTY_RESULTS, 61) },     {  7, 0, MESSNUM(MESS_PARTY_RESULTS, 49) },     {  8, 0, MESSNUM(MESS_PARTY_RESULTS, 63) },     {  9, 0, MESSNUM(MESS_PARTY_RESULTS, 62) },     { 11, 0, MESSNUM(MESS_PARTY_RESULTS, 50) },     { 10, 0, MESSNUM(MESS_PARTY_RESULTS, 51) }, } },
};








s32 lbl_1_data_5F4[11] = {
    MESSNUM(MESS_CHARA_NAME, 0),
    MESSNUM(MESS_CHARA_NAME, 1),
    MESSNUM(MESS_CHARA_NAME, 2),
    MESSNUM(MESS_CHARA_NAME, 3),
    MESSNUM(MESS_CHARA_NAME, 4),
    MESSNUM(MESS_CHARA_NAME, 5),
    MESSNUM(MESS_CHARA_NAME, 6),
    MESSNUM(MESS_CHARA_NAME, 7),
    MESSNUM(MESS_CHARA_NAME, 8),
    MESSNUM(MESS_CHARA_NAME, 9),
    MESSNUM(MESS_CHARA_NAME, 10),
};

s32 lbl_1_data_620 = -1;
char lbl_1_data_624[] = "# ========== win callback :: %d\n";
s16 lbl_1_data_646[3] = { -1, -1, 0 };
s32 lbl_1_data_64C[2] = { -1, -1 };
char lbl_1_data_654[] = "gN00m1-itemhook_R";
char lbl_1_data_666[] = "gN01m1-itemhook_R";
char lbl_1_data_678[] = "bg03";
char lbl_1_data_67D[] = "%d, ";
char lbl_1_data_682[] = "\n";


s16 lbl_1_data_684[4] = { 0, 0, 0, 0 };

char lbl_1_data_68C[] = "----------- mpKill()\n";
char lbl_1_data_6A2[] = "starbank add :: num(%d), star(%d), handi(%d)\n";
char lbl_1_data_6D0[] = "starbank :: %d\n";
char lbl_1_data_6E0[] = "mapno :: %d\n";
char lbl_1_data_6ED[] = "battle!!\n";
char lbl_1_data_6F7[] = "%d ) %d-%d\n";
char lbl_1_data_703[] = "tag!!\n";
char lbl_1_data_70A[] = "%d ) %d,%d-%d\n";
char lbl_1_data_719[] = "\n-----===== MARIO PARTY 6 :: PARTY RESULT =====-----\n\n";
char lbl_1_data_750[] = "%d\n";
float lbl_1_data_754 = 162.0f;
float lbl_1_data_758 = 162.0f;


GXColor lbl_1_data_75C[4] = {
    { 233, 80, 146, 255 },
    { 112, 212, 221, 255 },
    { 244, 156, 42, 255 },
    { 38, 216, 80, 255 },
};

char lbl_1_data_76C[] = "================ %d,%d\n";





HU3D_LIGHTID lbl_1_bss_130E[5];
HUWINID lbl_1_bss_1304[5];
MDRESULT_CAMERA_WORK lbl_1_bss_12BC;
s32 lbl_1_bss_12B0[3];
s32 lbl_1_bss_12A0[4];
s32 lbl_1_bss_129C;
s32 lbl_1_bss_1298;
MDRESULT_BSS_1278_WORK lbl_1_bss_1278;
MDRESULT_CHARACTER_WORK lbl_1_bss_1248[4];
ANIMDATA *lbl_1_bss_11AC[39];
HUSPR_GROUPID lbl_1_bss_11A0[6];
HUSPRID lbl_1_bss_117C[18];
MDRESULT_SCORE_WORK lbl_1_bss_10D4[4];
s16 lbl_1_bss_10CC[4];
HuVecF lbl_1_bss_109C[4];
MDRESULT_MOVE_WORK lbl_1_bss_F9C[4];
MDRESULT_MOVE_WORK lbl_1_bss_D9C[8];
MDRESULT_MODEL_EFFECT_WORK lbl_1_bss_ADC[11];
MDRESULT_COLOR_WORK lbl_1_bss_ABC[4];
MDRESULT_COLOR_STEP lbl_1_bss_AAC[4];
MDRESULT_MOVE_WORK lbl_1_bss_8EC[7];
MDRESULT_STATE_WORK lbl_1_bss_8AC[4];
MDRESULT_EMITTER_WORK lbl_1_bss_81C[9];
MDRESULT_MOVE_WORK lbl_1_bss_71C[4];
MDRESULT_GROUP_WORK lbl_1_bss_714;
s16 lbl_1_bss_70C[4];
MDRESULT_PLAYER_WORK lbl_1_bss_66C[4];
HUSPR_GROUPID lbl_1_bss_3D2[MDRESULT_GROUP_TABLE_COUNT];
s16 lbl_1_bss_21A[4][55];
s16 lbl_1_bss_62[4][55];
HUSPR_GROUPID lbl_1_bss_60;
ANIMDATA *lbl_1_bss_5C;
s16 lbl_1_bss_58;
s16 lbl_1_bss_56;
s16 lbl_1_bss_54;
float lbl_1_bss_50;
float lbl_1_bss_4C;
s16 lbl_1_bss_48;
float lbl_1_bss_44;
OMOBJ *lbl_1_bss_3C[2];
OMOBJ *lbl_1_bss_38;
OMOBJ *lbl_1_bss_34;
OMOBJ *lbl_1_bss_30;
OMOBJ *lbl_1_bss_2C;
OMOBJ *lbl_1_bss_28;
OMOBJ *lbl_1_bss_24;
OMOBJ *lbl_1_bss_20;
OMOBJ *lbl_1_bss_1C;
OMOBJ *lbl_1_bss_18;
OMOBJ *lbl_1_bss_14;
OMOBJ *lbl_1_bss_10;
OMOBJ *lbl_1_bss_C;
OMOBJ *lbl_1_bss_8;
OMOBJ *lbl_1_bss_4;
OMOBJMAN *lbl_1_bss_0;
