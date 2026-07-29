#include "dolphin/mtx.h"
#include "dolphin/os.h"
#include "datadir_enum.h"
#include "game/armem.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/window.h"
#include "humath.h"

typedef struct MdsingCameraWork MDSING_CAMERA_WORK;
typedef void (*MDSING_CAMERA_CALLBACK)(OMOBJ *obj, MDSING_CAMERA_WORK *work);

struct MdsingCameraWork {
    OMOBJ *obj;
    HuVecF center;
    HuVecF targetCenter;
    HuVecF rot;
    HuVecF targetRot;
    float zoom;
    float targetZoom;
    MDSING_CAMERA_CALLBACK callback;
};

typedef struct MdsingSpriteDesc {
    s16 groupNo;
    s16 memberNo;
    s16 animNo;
    s16 priority;
    s16 bank;
    HuVec2f pos;
    HuVec2f scale;
    float zRot;
} MDSING_SPRITE_DESC;

typedef struct MdsingMotionWork {
    s16 state;
    s16 pad;
    HuVecF pos;
    HuVecF control;
    HuVecF end;
    float time;
    float duration;
} MDSING_MOTION_WORK;

typedef struct MdsingCharacterDesc {
    s16 unk_0;
    s16 unk_2;
    s16 unk_4;
    s16 unk_6;
    s16 chrSel;
    s16 unk_A;
    s16 unk_C;
} MDSING_CHARACTER_DESC;

typedef struct MdsingMoveWork {
    s16 state;
    s16 pad;
    HuVecF pos;
    HuVecF unk_10;
    HuVecF unk_1C;
    float time;
    float duration;
} MDSING_MOVE_WORK;

typedef struct MdsingModelEntry {
    HU3D_MODELID modelId;
    HU3D_ANIMID animId[4];
    s16 unk_A;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
    s16 unk_30;
    u8 unk_32;
    u8 unk_33;
    u8 unk_34;
    u8 unk_35;
    u8 unk_36;
    u8 unk_37;
} MDSING_MODEL_ENTRY;

typedef MDSING_MODEL_ENTRY LBL_1_BSS_E74_ENTRY;

typedef struct BitmapNameTable {
    char *name[4];
} BITMAP_NAME_TABLE;

typedef struct RouteEntry {
    s16 value[8];
    s16 marker;
} MDSING_ROUTE_ENTRY;

const s32 lbl_1_rodata_10 = -1;

const s32 lbl_1_rodata_14[16] = {
    949, 950, 951, 952, 953, 954, 955, -1,
    941, 942, 943, 944, 945, 946, 947, -1,
};

const s32 lbl_1_rodata_54[2] = { DATA_board_us, DATA_capsule };
const float lbl_1_rodata_5C = 1.0f;
const float lbl_1_rodata_60 = 2.0f;
const float lbl_1_rodata_64 = 0.0f;
const double lbl_1_rodata_68 = M_PI;
const float lbl_1_rodata_70 = 90.0f;
const double lbl_1_rodata_78 = 180.0;

extern const float lbl_1_rodata_80;
extern const float lbl_1_rodata_84;
extern const float lbl_1_rodata_88;
extern const float lbl_1_rodata_8C;
extern const float lbl_1_rodata_90;
extern const float lbl_1_rodata_B0;
extern const float lbl_1_rodata_B8;
extern const float lbl_1_rodata_E0;
extern const float lbl_1_rodata_F0;
extern const float lbl_1_rodata_12C;
extern const float lbl_1_rodata_134;
extern const float lbl_1_rodata_13C;
extern const float lbl_1_rodata_140;
extern const float lbl_1_rodata_144;
extern const float lbl_1_rodata_148;
extern const float lbl_1_rodata_14C;
extern const float lbl_1_rodata_150;
extern const float lbl_1_rodata_178;
extern const float lbl_1_rodata_17C;
extern const float lbl_1_rodata_180;
extern const float lbl_1_rodata_184;
extern const float lbl_1_rodata_188;
extern const float lbl_1_rodata_18C;
extern const float lbl_1_rodata_1C8;
extern const float lbl_1_rodata_1E4;
extern const HuVecF lbl_1_rodata_1E8;
extern const float lbl_1_rodata_1F4;
extern const float lbl_1_rodata_1F8;
extern const float lbl_1_rodata_284;
extern const float lbl_1_rodata_288;
extern const float lbl_1_rodata_398;
extern const float lbl_1_rodata_3BC;
extern const float lbl_1_rodata_470;
extern HUPROCESS *lbl_1_bss_0;
extern s16 lbl_1_bss_30;
extern s16 lbl_1_bss_32;
extern s16 lbl_1_bss_34;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_24;
extern float lbl_1_bss_C94[3];
extern MDSING_MOVE_WORK lbl_1_bss_CF4[2];
extern MDSING_MOTION_WORK lbl_1_bss_DB4[2];
extern MDSING_MODEL_ENTRY lbl_1_bss_E74[16];
extern ANIMDATA *lbl_1_bss_11F4[25];
extern ANIMDATA *lbl_1_bss_12A4[25];
extern HUSPRID lbl_1_bss_1258[29];
extern HUSPR_GROUPID lbl_1_bss_1292[9];
extern MDSING_CHARACTER_DESC lbl_1_bss_1308[2];
extern s16 lbl_1_bss_1340[];
extern MDSING_CAMERA_WORK lbl_1_bss_1348;
extern HUWINID lbl_1_bss_1398[];
extern s16 lbl_1_bss_13A0[2];
extern s32 lbl_1_bss_13A4[5];
extern HU3D_MODELID lbl_1_bss_13CE[][5];
extern void *lbl_1_bss_13C8;
extern HU3D_MODELID lbl_1_bss_140A[];
extern HU3D_MODELID lbl_1_bss_140E[];
extern HU3D_MODELID lbl_1_bss_1414[];
extern HU3D_MODELID lbl_1_bss_141C;
extern HU3D_MODELID lbl_1_bss_141E[];
extern HU3D_MODELID lbl_1_bss_1426;
extern ANIMDATA *lbl_1_bss_1478[];
extern HuVecF lbl_1_bss_CAC[];
extern u32 lbl_1_data_32C[25];
extern s16 lbl_1_data_390[9];
extern MDSING_SPRITE_DESC lbl_1_data_3A4[29];
extern s32 lbl_1_data_744[25];
extern HuVecF lbl_1_data_7A8[];
extern char lbl_1_data_95A[];
extern char lbl_1_data_96B[];
extern char lbl_1_data_97D[];
extern char lbl_1_data_98F[];
extern u32 lbl_1_data_8EC;
extern char lbl_1_data_8F0[];
extern char lbl_1_data_99D[];
extern char lbl_1_data_99F[];
extern char lbl_1_data_9A3[];
extern char lbl_1_data_9D3[];
extern char lbl_1_data_A02[];
extern s16 lbl_1_data_A98;
extern s32 lbl_1_data_A9C[];
extern s16 lbl_1_data_AC8[3];
extern char lbl_1_data_AD4[];
extern char lbl_1_data_ADC[];
extern char lbl_1_data_AE4[];
extern char lbl_1_data_AEC[];
extern u32 lbl_1_data_918[];
extern s16 lbl_1_data_912[];
extern char lbl_1_data_928[];

void fn_1_2DCD0(s16 layerNo);
void fn_1_2E7DC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_2F4C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_2FE48(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_313A0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_31BE4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_344B4(s16 playerNo, HuVecF *pos, s16 arg2, s16 arg3);
void fn_1_5268(OMOBJ *obj);
void fn_1_595C(OMOBJ *obj);
void fn_1_633C(OMOBJ *obj);
void fn_1_6A7C(OMOBJ *obj);

typedef struct MdsingDirectoryPair {
    u32 values[2];
} MdsingDirectoryPair;

typedef struct MdsingSoundTable {
    s32 values[16];
} MdsingSoundTable;

void fn_1_0(s32 unused, u32 sound, s16 slot)
{
    s32 soundMatch[1];
    MdsingSoundTable soundTable;
    s16 i;

    soundMatch[0] = lbl_1_rodata_10;
    soundTable = *(const MdsingSoundTable *)lbl_1_rodata_14;
    slot--;
    OSReport(lbl_1_data_8F0, slot);
    if (lbl_1_data_8EC != sound) {
        lbl_1_data_8EC = sound;
        for (i = 0;; i++) {
            if (soundMatch[i] == -1) {
                HuAudFXPlay(soundTable.values[slot]);
                break;
            }
            if (sound == (u32)soundMatch[i]) {
                if (slot >= 8) {
                    HuAudFXPlayPan(soundTable.values[slot], 80);
                } else {
                    HuAudFXPlayPan(soundTable.values[slot], 48);
                }
                break;
            }
        }
    }
}

void fn_1_1A4(void)
{
    lbl_1_bss_30 = 0;
    lbl_1_bss_32 = 0;
    if (GWBankFlagGet(2)) {
        lbl_1_bss_30 = 1;
    }
    if (GWBankFlagGet(4)) {
        lbl_1_bss_32 = 1;
    }
    lbl_1_data_912[0] = 1;
    lbl_1_data_912[1] = 1;
    lbl_1_data_912[2] = 1;
}

void fn_1_250(void)
{
    s16 character[5];
    s16 i;
    s32 status;

    character[0] = lbl_1_bss_1308[0].chrSel;
    character[1] = lbl_1_bss_1308[1].chrSel;
    character[2] = 11;
    character[3] = 12;
    character[4] = 13;
    for (i = 0; i < 5; i++) {
        if ((void *)CharMotionAMemPGet(character[i]) == NULL) {
            break;
        }
    }
    if (i == 5) {
        return;
    }

    CharDataClose(-1);
    for (i = 0; i < 5; i++) {
        status = HuDataDirReadAsync(CharDataDirTbl[character[i]][4]);
        if (status != -1) {
            while (!HuDataGetAsyncStat(status)) {
                HuPrcVSleep();
            }
        }
        CharMotionInit(character[i]);
        HuDataDirClose(CharDataDirTbl[character[i]][4]);
    }
}

void fn_1_3A0(void)
{
    MdsingDirectoryPair aramDirectory =
        *(const MdsingDirectoryPair *)lbl_1_rodata_54;
    s16 character[5];
    s16 i;
    s16 directory;
    s32 motionStatus;
    s32 dataStatus;

    character[0] = lbl_1_bss_1308[0].chrSel;
    character[1] = lbl_1_bss_1308[1].chrSel;
    character[2] = 11;
    character[3] = 12;
    character[4] = 13;
    for (i = 0; i < 5; i++) {
        if ((void *)CharMotionAMemPGet(character[i]) == NULL) {
            break;
        }
    }
    if (i != 5) {
        CharDataClose(-1);
        for (i = 0; i < 5; i++) {
            motionStatus = HuDataDirReadAsync(CharDataDirTbl[character[i]][4]);
            if (motionStatus != -1) {
                while (!HuDataGetAsyncStat(motionStatus)) {
                    HuPrcVSleep();
                }
            }
            CharMotionInit(character[i]);
            HuDataDirClose(CharDataDirTbl[character[i]][4]);
        }
    }
    for (directory = 0; directory < 2; directory++) {
        dataStatus = HuDataDirReadAsync(aramDirectory.values[directory]);
        if (dataStatus != -1) {
            while (!HuDataGetAsyncStat(dataStatus)) {
                HuPrcVSleep();
            }
        }
        HuAR_MRAMtoARAM(aramDirectory.values[directory]);
        while (HuARDMACheck() != 0) {
            HuPrcVSleep();
        }
        HuDataDirClose(aramDirectory.values[directory]);
    }
    dataStatus = HuDataDirReadAsync(lbl_1_data_918[lbl_1_bss_1340[1]]);
    if (dataStatus != -1) {
        while (!HuDataGetAsyncStat(dataStatus)) {
            HuPrcVSleep();
        }
    }
    lbl_1_bss_34 = 1;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

void fn_1_5E8(void)
{
    lbl_1_bss_34 = 0;
    OSReport(lbl_1_data_928);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    HuDataDirClose(10092544);
    HuPrcChildCreate(fn_1_3A0, 256, 16384, 0, lbl_1_bss_0);
}

void fn_1_6B4(void)
{
    s16 character = 0;
    s16 i;

    character = lbl_1_bss_1308[0].chrSel;
    GwCommon.storyMgPack = lbl_1_bss_1340[3];
    for (i = 0; i < 1; i++) {
        GwPlayer[i].comF = 0;
        GwPlayer[i].comDif = 0;
        GwPlayer[i].charNo = lbl_1_bss_1308[i].chrSel;
        GwPlayer[i].padNo = 0;
        GwPlayer[i].team = 0;
    }
    for (i = 1; i < 4; i++) {
        GwPlayer[i].comF = 1;
        GwPlayer[i].comDif = 0;
        character++;
        if (character >= 14) {
            character -= 14;
        }
        GwPlayer[i].charNo = character;
        GwPlayer[i].padNo = i;
        GwPlayer[i].team = 0;
    }
    for (i = 0; i < 4; i++) {
        GwPlayerConf[i].grpNo = 0;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].comDif = GwPlayer[i].comDif;
        GwPlayerConf[i].charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo;
        GwPlayerConf[i].grpNo = GwPlayer[i].team;
    }
    for (i = 0; i < 4; i++) {
        OSReport(lbl_1_data_99F, GwPlayerConf[i].charNo);
    }
    GwCommon.confSingleDiff = lbl_1_bss_1340[2];
    GwCommon.storyMgPack = lbl_1_bss_1340[3];
    _ClearFlag(65550);
    GWSingleDataInit();
    GWSingleMgRecordNumSet(0);
    GWSingleMgWinNumSet(0);
    GwSystem.turnPlayerNo = 0;
    for (i = 0; i < 4; i++) {
        GwCommon.singleMgWinNum[i] = 0;
    }
    mbSaveInit(lbl_1_bss_1340[1] + 6);
    mbSaveStoryInit(lbl_1_bss_1308[1].chrSel, lbl_1_bss_1340[3],
        lbl_1_bss_1340[2]);
}

void fn_1_B34(void)
{
    s16 i;

    for (i = 0; i < 1; i++) {
        GwPlayer[i].comF = 1;
        GwPlayer[i].comDif = 0;
        GwPlayer[i].charNo = 0;
        GwPlayer[i].padNo = 0;
        GwPlayer[i].team = 0;
    }
    for (i = 1; i < 4; i++) {
        GwPlayer[i].comF = 1;
        GwPlayer[i].comDif = 0;
        GwPlayer[i].charNo = i;
        GwPlayer[i].padNo = i;
        GwPlayer[i].team = 0;
    }
    for (i = 0; i < 4; i++) {
        GwPlayerConf[i].grpNo = 0;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].comDif = GwPlayer[i].comDif;
        GwPlayerConf[i].charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo;
        GwPlayerConf[i].grpNo = GwPlayer[i].team;
    }
    lbl_1_bss_1308[1].chrSel = GwPlayer[1].charNo;
    for (i = 0; i < 4; i++) {
        OSReport(lbl_1_data_99F, GwPlayerConf[i].charNo);
    }
    lbl_1_bss_1340[1] = 3;
    mbSaveInit(10);
}

void fn_1_EA0(void)
{
    OMOVLHIS *history;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_34 == 0);

    history = omOvlHisGet(0);
    omOvlHisChg(0, history->ovl, 1, lbl_1_bss_1308[1].chrSel);
    OSReport(lbl_1_data_9A3);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    switch (lbl_1_bss_1340[1]) {
        case 0:
            omOvlCallEx(114, 1, 0, 0);
            break;
        case 1:
            omOvlCallEx(115, 1, 0, 0);
            break;
        case 2:
            omOvlCallEx(116, 1, 0, 0);
            break;
    }
}

void fn_1_FEC(void)
{
    OMOVLHIS *history;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_34 == 0);

    history = omOvlHisGet(0);
    omOvlHisChg(0, history->ovl, 2, 0);
    OSReport(lbl_1_data_9A3);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    omOvlCallEx(130, 1, 0, 0);
}

void fn_1_10D0(void)
{
    CharDataClose(-1);
    HuARDirFree(327680);
    HuARDirFree(393216);
    HuARDirFree(786432);
    OSReport(lbl_1_data_9D3);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
}

void fn_1_1180(void)
{
    CharDataClose(-1);
    HuARDirFree(327680);
    HuARDirFree(393216);
    HuARDirFree(786432);
    OSReport(lbl_1_data_A02);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
}

float fn_1_1230(float start, float control, float end, float weight)
{
    float inverse = lbl_1_rodata_5C - weight;

    return (end * (weight * weight)) +
        ((start * (inverse * inverse)) +
            (lbl_1_rodata_60 * (control * (inverse * weight))));
}

void fn_1_128C(Vec *out, const Vec *start, const Vec *control,
    const Vec *end, float weight)
{
    out->x = fn_1_1230(start->x, control->x, end->x, weight);
    out->y = fn_1_1230(start->y, control->y, end->y, weight);
    out->z = fn_1_1230(start->z, control->z, end->z, weight);
}

float fn_1_1494(float current, float target, float weight)
{
    if (current == target) {
        return target;
    }
    return (target + (current * (weight - lbl_1_rodata_5C))) / weight;
}

void fn_1_14C4(Vec *current, const Vec *target, float weight)
{
    current->x = fn_1_1494(current->x, target->x, weight);
    current->y = fn_1_1494(current->y, target->y, weight);
    current->z = fn_1_1494(current->z, target->z, weight);
}

#include "humath.h"

#include "dolphin/types.h"

#include "game/hu3d.h"

#include "string.h"

#include "dolphin/mtx/GeoTypes.h"

#include "game/window.h"

#include "game/memory.h"

#include "datadir_enum.h"

#include "dolphin/gx.h"

float fn_1_16F0(float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_64) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}

static inline float blend_rotation(float current, float target)
{
    if (current == target) {
        return target;
    }
    return (target + (current * lbl_1_rodata_8C)) / lbl_1_rodata_90;
}

static inline float linear_value(
    float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_64) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}

static inline void move_model(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration)
{
    HuVecF modelPos;
    HuVecF modelRot;
    HuVecF pos;
    HuVecF rot;

    Hu3DModelPosGet(modelId, &modelPos);
    Hu3DModelRotGet(modelId, &modelRot);
    pos.x = linear_value(start->x, end->x, time, duration);
    pos.y = linear_value(start->y, end->y, time, duration);
    pos.z = linear_value(start->z, end->z, time, duration);
    modelPos.x -= pos.x;
    modelPos.z -= pos.z;
    rot.y = -(lbl_1_rodata_78
        * (atan2(modelPos.x, -modelPos.z) / lbl_1_rodata_68));
    if (modelRot.y - rot.y > lbl_1_rodata_80) {
        modelRot.y -= lbl_1_rodata_84;
    } else if (modelRot.y - rot.y < lbl_1_rodata_88) {
        modelRot.y += lbl_1_rodata_84;
    }
    rot.x = modelRot.x;
    rot.y = blend_rotation(modelRot.y, rot.y);
    rot.z = modelRot.z;
    Hu3DModelPosSetV(modelId, &pos);
    Hu3DModelRotSetV(modelId, &rot);
}

void fn_1_1A9C(HU3D_MODELID modelId, float rotY)
{
    HuVecF modelRot;
    HuVecF rot;

    Hu3DModelRotGet(modelId, &modelRot);
    rot.y = rotY;
    if (modelRot.y - rot.y > lbl_1_rodata_80) {
        modelRot.y -= lbl_1_rodata_84;
    } else if (modelRot.y - rot.y < lbl_1_rodata_88) {
        modelRot.y += lbl_1_rodata_84;
    }
    rot.x = modelRot.x;
    rot.y = blend_rotation(modelRot.y, rot.y);
    rot.z = modelRot.z;
    Hu3DModelRotSetV(modelId, &rot);
}

void fn_1_1BDC(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY)
{
    if (time <= duration) {
        move_model(modelId, start, end, time, duration);
    } else {
        fn_1_1A9C(modelId, rotY);
    }
}

void fn_1_26FC(MDSING_CAMERA_WORK *work)
{
    memcpy(&work->center, &work->targetCenter, sizeof(HuVecF));
    memcpy(&work->rot, &work->targetRot, sizeof(HuVecF));
    work->zoom = work->targetZoom;
}

void fn_1_274C(MDSING_CAMERA_WORK *work)
{
    memcpy(&work->targetCenter, &work->center, sizeof(HuVecF));
    memcpy(&work->targetRot, &work->rot, sizeof(HuVecF));
    work->targetZoom = work->zoom;
}

static inline float blend_value(float current, float target, float weight)
{
    if (current == target) {
        return target;
    }
    return (target + (current * (weight - lbl_1_rodata_5C))) / weight;
}

static inline void blend_vector(Vec *current, const Vec *target, float weight)
{
    current->x = blend_value(current->x, target->x, weight);
    current->y = blend_value(current->y, target->y, weight);
    current->z = blend_value(current->z, target->z, weight);
}

void fn_1_279C(MDSING_CAMERA_WORK *camera, float weight)
{
    blend_vector(&camera->center, &camera->targetCenter, weight);
    blend_vector(&camera->rot, &camera->targetRot, weight);
    camera->zoom = blend_value(camera->zoom, camera->targetZoom, weight);
}

void fn_1_2A50(MDSING_CAMERA_CALLBACK callback)
{
    lbl_1_bss_1348.callback = callback;
}

void fn_1_3170(OMOBJ *obj, MDSING_CAMERA_WORK *work)
{
    if (work->callback) {
        work->callback(obj, work);
    }
}

void fn_1_31BC(OMOBJ *obj)
{
    MDSING_CAMERA_WORK *work = &lbl_1_bss_1348;

    fn_1_3170(obj, work);
    Center.x = work->center.x;
    Center.y = work->center.y;
    Center.z = work->center.z;
    CRot.x = work->rot.x;
    CRot.y = work->rot.y;
    CRot.z = work->rot.z;
    CZoom = work->zoom;
    omOutView(obj);
}

void fn_1_3484(void)
{
    lbl_1_bss_13A0[0] = Hu3DGLightCreate(
        lbl_1_rodata_64, lbl_1_rodata_5C, lbl_1_rodata_5C,
        lbl_1_rodata_64, lbl_1_rodata_F0, lbl_1_rodata_F0,
        255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_13A0[0]);
    Hu3DGLightStaticSet(lbl_1_bss_13A0[0], TRUE);
    lbl_1_bss_13A0[1] = Hu3DGLightCreate(
        lbl_1_rodata_F0, lbl_1_rodata_5C, lbl_1_rodata_F0,
        lbl_1_rodata_5C, lbl_1_rodata_F0, lbl_1_rodata_F0,
        255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_13A0[1]);
    Hu3DGLightStaticSet(lbl_1_bss_13A0[1], TRUE);
}

void fn_1_35B0(void)
{
    Hu3DGLightKill(lbl_1_bss_13A0[0]);
    Hu3DGLightKill(lbl_1_bss_13A0[1]);
}

void fn_1_35EC(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_1398[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_1398[winNo]);
    }
}

void fn_1_365C(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_1398[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_1398[winNo]);
    }
}

void fn_1_36CC(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_1398[winNo]);
}

s16 fn_1_3708(s16 winNo, s16 mode)
{
    s16 choice = 0;

    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_1398[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_1398[winNo], HUWIN_ATTR_NOCANCEL);
    }
    choice = HuWinChoiceGet(lbl_1_bss_1398[winNo], -1);
    if (mode == 2 && choice == -1) {
        choice = 1;
    }
    return choice;
}

void fn_1_37DC(s16 winNo, s32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_1398[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_1398[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_1398[winNo], speed);
    if (lbl_1_data_8EC != messNum) {
        lbl_1_data_8EC = -1;
    }
}

void fn_1_3898(s16 winNo, s32 messNum, s16 insertPos)
{
    HuWinHomeClear(lbl_1_bss_1398[winNo]);
    HuWinInsertMesSet(lbl_1_bss_1398[winNo], messNum, insertPos);
}

void fn_1_3B30(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        HuWinExKill(lbl_1_bss_1398[i]);
    }
    HuWinAllKill();
}

void fn_1_3B8C(s16 winNo)
{
    if (lbl_1_data_A98 != -1 && lbl_1_data_A98 != winNo) {
        s16 activeWin = lbl_1_data_A98;

        if (activeWin == 0) {
            HuWinDispOff(lbl_1_bss_1398[activeWin]);
        } else {
            HuWinExClose(lbl_1_bss_1398[activeWin]);
        }
    }
    if (lbl_1_data_A98 == -1 || lbl_1_data_A98 != winNo) {
        s16 activeWin;

        lbl_1_data_A98 = winNo;
        lbl_1_data_A9C[0] = -1;
        lbl_1_data_A9C[1] = -1;
        activeWin = lbl_1_data_A98;
        if (activeWin == 0) {
            HuWinDispOn(lbl_1_bss_1398[activeWin]);
        } else {
            HuWinExOpen(lbl_1_bss_1398[activeWin]);
        }
    }
}

void fn_1_3DB4(void)
{
    if (lbl_1_data_A98 != -1) {
        s16 winNo = lbl_1_data_A98;

        HuWinMesWait(lbl_1_bss_1398[winNo]);
    }
}

void fn_1_47F0(void)
{
}

void fn_1_47F4(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

void fn_1_4874(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}

void fn_1_48F4(void)
{
    MDSING_SPRITE_DESC *desc;
    s16 i;

    for (i = 0; i < 25; i++) {
        lbl_1_bss_12A4[i] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_32C[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 9; i++) {
        lbl_1_bss_1292[i] = HuSprGrpCreate(lbl_1_data_390[i]);
    }
    for (i = 0, desc = lbl_1_data_3A4; i < 29; i++, desc++) {
        lbl_1_bss_1258[i] = HuSprCreate(
            lbl_1_bss_12A4[desc->animNo], desc->priority, desc->bank);
        HuSprGrpMemberSet(lbl_1_bss_1292[desc->groupNo], desc->memberNo,
            lbl_1_bss_1258[i]);
        HuSprPosSet(lbl_1_bss_1292[desc->groupNo], desc->memberNo,
            desc->pos.x, desc->pos.y);
        HuSprScaleSet(lbl_1_bss_1292[desc->groupNo], desc->memberNo,
            desc->scale.x, desc->scale.y);
        HuSprZRotSet(lbl_1_bss_1292[desc->groupNo], desc->memberNo,
            desc->zRot);
    }
    for (i = 0; i < 9; i++) {
        fn_1_47F4(lbl_1_bss_1292[i], HUSPR_ATTR_DISPOFF);
    }
}

void fn_1_4B40(void)
{
}

void fn_1_514C(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_60);
    if (obj->work[3]++ > 30) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_5C);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            lbl_1_rodata_64, lbl_1_rodata_B8, HU3D_MOTATTR_LOOP);
    }
}

void fn_1_51F4(void)
{
    OMOBJ *obj = lbl_1_bss_C;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[3],
        lbl_1_rodata_64, lbl_1_rodata_90, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_514C;
}

void fn_1_57E4(void)
{
    MDSING_MOTION_WORK *work = &lbl_1_bss_DB4[0];
    OMOBJ *obj = lbl_1_bss_C;

    work->state = 0;
    work->time = lbl_1_rodata_64;
    work->duration = lbl_1_rodata_13C;
    Hu3DModelPosGet(obj->mdlId[0], &work->pos);
    work->pos.x = lbl_1_rodata_140;
    work->pos.y = lbl_1_rodata_64;
    work->pos.z = lbl_1_rodata_144;
    Hu3DModelPosGet(obj->mdlId[0], &work->control);
    work->control.x = lbl_1_rodata_64;
    work->control.y = lbl_1_rodata_134;
    work->control.z = lbl_1_rodata_148;
    Hu3DModelPosGet(obj->mdlId[0], &work->end);
    work->end.x = lbl_1_rodata_64;
    work->end.y = lbl_1_rodata_14C;
    work->end.z = lbl_1_rodata_150;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[2],
        lbl_1_rodata_64, lbl_1_rodata_64, 0);
    lbl_1_bss_13A4[0] = HuAudFXPlay(1150);
    obj->objFunc = fn_1_5268;
}

s32 fn_1_5EF4(void)
{
    OMOBJ *obj = lbl_1_bss_C;
    Vec pos;

    Hu3DModelPosGet(obj->mdlId[0], &pos);
    if (pos.x < lbl_1_rodata_178) {
        return 0;
    }
    obj->work[0] = 0;
    obj->objFunc = fn_1_595C;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        lbl_1_rodata_64, lbl_1_rodata_64, HU3D_MOTATTR_LOOP);
    return 1;
}

void fn_1_68C0(void)
{
    MDSING_CHARACTER_DESC *desc;
    OMOBJ *obj = lbl_1_bss_10;
    MDSING_MOVE_WORK *work;
    s16 i;

    for (i = 0, desc = &lbl_1_bss_1308[i]; i < 2; i++, desc++) {
        work = &lbl_1_bss_CF4[i];
        work->state = 1;
        work->time = lbl_1_rodata_64;
        work->duration = lbl_1_rodata_70;
        Hu3DModelPosGet(obj->mdlId[desc->chrSel], &work->pos);
        Hu3DModelPosGet(obj->mdlId[desc->chrSel], &work->unk_10);
        work->unk_10.x += lbl_1_rodata_180 * work->unk_10.x;
        work->unk_10.y = lbl_1_rodata_184;
        work->unk_10.z = lbl_1_rodata_144;
        Hu3DModelPosGet(obj->mdlId[desc->chrSel], &work->unk_1C);
        work->unk_1C.y = lbl_1_rodata_188;
        work->unk_1C.z = lbl_1_rodata_E0;
        Hu3DMotionShiftSet(obj->mdlId[desc->chrSel],
            obj->mtnId[(2 * desc->chrSel) + 1], lbl_1_rodata_64,
            lbl_1_rodata_90, HU3D_MOTATTR_LOOP);
    }
    HuAudFXPlay(1186);
    obj->objFunc = fn_1_633C;
}

void fn_1_6C48(void)
{
    OMOBJ *obj = lbl_1_bss_10;
    MDSING_CHARACTER_DESC *desc;
    Vec pos;
    s16 i;

    for (i = 0, desc = &lbl_1_bss_1308[i]; i < 2; i++, desc++) {
        Hu3DModelPosGet(lbl_1_bss_E74[desc->chrSel].modelId, &pos);
        if (desc->unk_4 != 0) {
            fn_1_344B4(i, &pos, 4, 0);
        } else {
            fn_1_344B4(i, &pos, desc->unk_A, 0);
        }
    }
    for (i = 0, desc = &lbl_1_bss_1308[i]; i < 2; i++, desc++) {
        Hu3DModelPosGet(lbl_1_bss_E74[desc->chrSel].modelId, &pos);
        pos.y -= lbl_1_rodata_18C;
        Hu3DModelPosSetV(obj->mdlId[desc->chrSel], &pos);
        Hu3DModelScaleSet(obj->mdlId[desc->chrSel], lbl_1_rodata_64,
            lbl_1_rodata_64, lbl_1_rodata_64);
        Hu3DModelAttrReset(obj->mdlId[desc->chrSel], HU3D_ATTR_DISPOFF);
    }
    obj->work[0] = 0;
    obj->work[1] = 15;
    obj->objFunc = fn_1_6A7C;
    HuPrcSleep(15);
    HuAudFXPlay(1185);
    for (i = 0, desc = &lbl_1_bss_1308[i]; i < 2; i++, desc++) {
        Hu3DModelPosGet(lbl_1_bss_E74[desc->chrSel].modelId, &pos);
        if (desc->unk_4 != 0) {
            fn_1_344B4(i, &pos, 4, 1);
        } else {
            fn_1_344B4(i, &pos, desc->unk_A, 1);
        }
    }
}

void fn_1_79D0(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 3; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_7B64(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[2] = 2;
    obj->work[3] = 0;
}

void fn_1_7B94(void)
{
    HuSprGrpPosSet(
        lbl_1_bss_1292[1], lbl_1_rodata_1C8, lbl_1_rodata_12C);
    HuSprGrpTPLvlSet(lbl_1_bss_1292[1], lbl_1_rodata_64);
}

void fn_1_7E3C(s16 animNo)
{
    OMOBJ *obj = lbl_1_bss_8;

    Hu3DAnimAnimSet(
        obj->mtnId[2], lbl_1_bss_12A4[(2 * animNo) + 3]);
    Hu3DAnimAnimSet(
        obj->mtnId[3], lbl_1_bss_12A4[(2 * animNo) + 4]);
    obj->work[0] = 1;
    obj->work[1] = 0;
}

void fn_1_8670(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 2; i++) {
            Hu3DAnimKill(obj->mtnId[2 * i]);
            Hu3DAnimKill(obj->mtnId[(2 * i) + 1]);
            obj->mtnId[2 * i] = -1;
            obj->mtnId[(2 * i) + 1] = -1;
        }
        for (i = 1; i >= 0; i--) {
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_8F64(s16 modelNo, Vec *pos)
{
    lbl_1_bss_CAC[modelNo].x = pos->x - lbl_1_rodata_1F8;
    lbl_1_bss_CAC[modelNo].y = lbl_1_rodata_1F8 + pos->y;
    lbl_1_bss_CAC[modelNo].z = lbl_1_rodata_1F8 + pos->z;
}

s16 fn_1_9998(s16 arg0, s16 arg1, s16 arg2, s16 arg3,
    MDSING_ROUTE_ENTRY *arg4, s16 arg5)
{
    s16 result;
    s16 j;
    s16 i;
    s16 k;
    s16 l;

    if (arg1 == -1) {
        return -1;
    }
    result = arg4[arg0].value[arg1];
    if (result == -1) {
        s16 routed;

        if (arg2 == -1) {
            routed = -1;
        } else {
            s16 next;

            next = arg4[arg0].value[arg2];
            if (next == -1) {
                next = fn_1_9998(arg0, arg3, -1, -1, arg4, arg5);
            }
            for (i = 0; i < arg5; i++) {
                if (arg0 != i && arg4[i].marker != -1 && next == i) {
                    next = fn_1_9998(
                        next, arg2, arg3, -1, arg4, arg5);
                    if (next == -1) {
                        next = fn_1_9998(
                            arg0, arg3, -1, -1, arg4, arg5);
                    }
                }
            }
            routed = next;
        }
        result = routed;
    }

    for (j = 0; j < arg5; j++) {
        if (arg0 != j && arg4[j].marker != -1 && result == j) {
            s16 routed;

            if (arg1 == -1) {
                routed = -1;
            } else {
                s16 next;

                next = arg4[result].value[arg1];
                if (next == -1) {
                    next = fn_1_9998(
                        result, arg2, arg3, -1, arg4, arg5);
                }
                for (k = 0; k < arg5; k++) {
                    if (result != k && arg4[k].marker != -1
                        && next == k) {
                        next = fn_1_9998(
                            next, arg1, arg2, arg3, arg4, arg5);
                        if (next == -1) {
                            next = fn_1_9998(
                                result, arg2, arg3, -1, arg4, arg5);
                        }
                    }
                }
                routed = next;
            }
            result = routed;
            if (result == -1) {
                s16 fallback;

                if (arg2 == -1) {
                    fallback = -1;
                } else {
                    s16 next;

                    next = arg4[arg0].value[arg2];
                    if (next == -1) {
                        next = fn_1_9998(
                            arg0, arg3, -1, -1, arg4, arg5);
                    }
                    for (l = 0; l < arg5; l++) {
                        if (arg0 != l && arg4[l].marker != -1
                            && next == l) {
                            next = fn_1_9998(
                                next, arg2, arg3, -1, arg4, arg5);
                            if (next == -1) {
                                next = fn_1_9998(
                                    arg0, arg3, -1, -1, arg4, arg5);
                            }
                        }
                    }
                    fallback = next;
                }
                result = fallback;
            }
        }
    }
    return result;
}

void fn_1_A3A8(s16 modelNo, s16 animNo, s16 dataNo, s16 bank)
{
    LBL_1_BSS_E74_ENTRY *entry = &lbl_1_bss_E74[modelNo];

    Hu3DAnimAnimSet(entry->animId[animNo], lbl_1_bss_11F4[dataNo]);
    if (bank != -1) {
        Hu3DAnimBankSet(entry->animId[animNo], bank);
    }
}

void fn_1_A450(s16 modelNo, s16 dataNo)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        fn_1_A3A8(modelNo, i, dataNo, -1);
    }
}

const BITMAP_NAME_TABLE lbl_1_rodata_270 = {
    {
        lbl_1_data_AD4,
        lbl_1_data_ADC,
        lbl_1_data_AE4,
        lbl_1_data_AEC,
    },
};

void fn_1_B898(s16 arg0)
{
    lbl_1_bss_24->mtnId[0] = arg0;
}

s16 fn_1_B8B0(void)
{
    return lbl_1_bss_24->mtnId[0];
}

s16 fn_1_B8C8(void)
{
    OMOBJ *obj = lbl_1_bss_24;

    if (obj->work[0] >= 10) {
        return TRUE;
    }
    return FALSE;
}

void fn_1_B900(OMOBJ_FUNC callback)
{
    OMOBJ *obj = lbl_1_bss_24;

    lbl_1_bss_24->mtnId[0] = 0;
    obj->work[0] = 0;
    obj->work[1] = 0;
    obj->work[2] = 0;
    obj->work[3] = 0;
    obj->objFunc = callback;
    while (lbl_1_bss_24->mtnId[0] == 0) {
        HuPrcVSleep();
    }
}

void fn_1_B998(void)
{
    BITMAP_NAME_TABLE bitmapName = lbl_1_rodata_270;
    s16 i;
    s16 j;

    for (i = 0; i < 25; i++) {
        lbl_1_bss_11F4[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(
                lbl_1_data_744[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 16; i++) {
        memset(&lbl_1_bss_E74[i], 0, sizeof(LBL_1_BSS_E74_ENTRY));
        if (i == 0) {
            lbl_1_bss_E74[i].modelId = Hu3DModelCreate(
                HuDataSelHeapReadNum(
                    DATANUM(DATA_mdsing, 21), HU_MEMNUM_OVL,
                    HEAP_MODEL));
        } else {
            lbl_1_bss_E74[i].modelId =
                Hu3DModelLink(lbl_1_bss_E74[0].modelId);
        }
        for (j = 0; j < 4; j++) {
            lbl_1_bss_E74[i].animId[j] = Hu3DAnimCreate(
                lbl_1_bss_11F4[0], lbl_1_bss_E74[i].modelId,
                bitmapName.name[j]);
        }
        Hu3DModelLayerSet(lbl_1_bss_E74[i].modelId, 1);
        Hu3DModelAttrSet(
            lbl_1_bss_E74[i].modelId, HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_24 =
        omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, NULL);
}

void fn_1_BBC8(void)
{
    s16 i;
    s16 j;

    if (lbl_1_bss_24) {
        lbl_1_bss_24->mtnId[0] = -1;
        for (i = 15; i >= 0; i--) {
            for (j = 0; j < 4; j++) {
                Hu3DAnimKill(lbl_1_bss_E74[i].animId[j]);
            }
            Hu3DModelKill(lbl_1_bss_E74[i].modelId);
        }
    }
    lbl_1_bss_24 = NULL;
}

static inline float approach_value(float current, float target)
{
    if (current == target) {
        return target;
    }
    return (target + (current * lbl_1_rodata_1F4)) /
        lbl_1_rodata_B0;
}

static inline void approach_vector(HuVecF *current, const HuVecF *target)
{
    current->x = approach_value(current->x, target->x);
    current->y = approach_value(current->y, target->y);
    current->z = approach_value(current->z, target->z);
}

void fn_1_E688(s16 modelNo)
{
    Vec value;
    MDSING_MODEL_ENTRY *entry = &lbl_1_bss_E74[modelNo + 11];

    Hu3DModelPosGet(entry->modelId, &value);
    approach_vector(&value, &lbl_1_data_7A8[modelNo + 6]);
    Hu3DModelPosSetV(entry->modelId, &value);
    if (entry->unk_30++ > 30) {
        entry->unk_30 = 35;
        Hu3DModelRotGet(entry->modelId, &value);
        value.y += lbl_1_rodata_1E4;
        if (value.y >= lbl_1_rodata_84) {
            value.y -= lbl_1_rodata_84;
        }
        Hu3DModelRotSetV(entry->modelId, &value);
    }
    Hu3DModelScaleGet(entry->modelId, &value);
    value.y = value.x =
        approach_value(value.x, lbl_1_rodata_284);
    Hu3DModelScaleSetV(entry->modelId, &value);
    Hu3DModelLayerSet(entry->modelId, 2);
}

static inline void update_selected_model(s16 modelNo)
{
    Vec value;
    MDSING_MODEL_ENTRY *entry = &lbl_1_bss_E74[modelNo + 11];

    Hu3DModelPosGet(entry->modelId, &value);
    approach_vector(&value, &lbl_1_data_7A8[modelNo + 6]);
    Hu3DModelPosSetV(entry->modelId, &value);
    if (entry->unk_30++ > 30) {
        entry->unk_30 = 35;
        Hu3DModelRotGet(entry->modelId, &value);
        value.y += lbl_1_rodata_1E4;
        if (value.y >= lbl_1_rodata_84) {
            value.y -= lbl_1_rodata_84;
        }
        Hu3DModelRotSetV(entry->modelId, &value);
    }
    Hu3DModelScaleGet(entry->modelId, &value);
    value.y = value.x =
        approach_value(value.x, lbl_1_rodata_284);
    Hu3DModelScaleSetV(entry->modelId, &value);
    Hu3DModelLayerSet(entry->modelId, 2);
}

static inline void reset_model(s16 modelNo)
{
    MDSING_MODEL_ENTRY *entry = &lbl_1_bss_E74[modelNo + 11];

    Hu3DModelPosSetV(entry->modelId, &lbl_1_data_7A8[modelNo + 3]);
    Hu3DModelRotSet(entry->modelId,
        lbl_1_rodata_64, lbl_1_rodata_64, lbl_1_rodata_64);
    Hu3DModelScaleSet(entry->modelId,
        lbl_1_rodata_17C, lbl_1_rodata_17C, lbl_1_rodata_5C);
    entry->unk_30 = 0;
    Hu3DModelLayerSet(entry->modelId, 1);
}

static inline void sprite_attr_set(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

static inline void show_member(
    s16 memberNo, Vec *pos3D, float xOffset, float yOffset)
{
    Vec pos2D = lbl_1_rodata_1E8;

    if (pos3D) {
        Hu3D3Dto2D(pos3D, 1, &pos2D);
    }
    HuSprPosSet(lbl_1_bss_1292[2], memberNo,
        pos2D.x + xOffset, pos2D.y + yOffset);
    HuSprScaleSet(lbl_1_bss_1292[2], memberNo,
        lbl_1_rodata_60, lbl_1_rodata_60);
    HuSprAttrReset(
        lbl_1_bss_1292[2], memberNo, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_C94[memberNo] = lbl_1_rodata_60;
    lbl_1_data_AC8[memberNo] = 1;
}

void fn_1_EA7C(OMOBJ *obj)
{
    s16 i;

    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            OMOBJ *workObj;
            s16 memberNo;

            lbl_1_bss_1340[1]--;
            if (lbl_1_bss_1340[1] < 0) {
                lbl_1_bss_1340[1] += 3;
            }
            obj->work[0] = 0;
            obj->work[1] = 0;
            memberNo = lbl_1_bss_1340[1];
            workObj = lbl_1_bss_8;
            HuSprGrpPosSet(
                lbl_1_bss_1292[1], lbl_1_rodata_1C8, lbl_1_rodata_12C);
            HuSprGrpTPLvlSet(lbl_1_bss_1292[1], lbl_1_rodata_64);
            sprite_attr_set(lbl_1_bss_1292[1], HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(
                lbl_1_bss_1292[1], memberNo, HUSPR_ATTR_DISPOFF);
            workObj->work[2] = 1;
            workObj->work[3] = 0;
            lbl_1_bss_C94[0] = lbl_1_rodata_60;
            if (lbl_1_data_AC8[0] == 1) {
                HuAudFXPlay(0);
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            OMOBJ *workObj;
            s16 memberNo;

            lbl_1_bss_1340[1]++;
            if (lbl_1_bss_1340[1] >= 3) {
                lbl_1_bss_1340[1] -= 3;
            }
            obj->work[0] = 0;
            obj->work[1] = 1;
            memberNo = lbl_1_bss_1340[1];
            workObj = lbl_1_bss_8;
            HuSprGrpPosSet(
                lbl_1_bss_1292[1], lbl_1_rodata_1C8, lbl_1_rodata_12C);
            HuSprGrpTPLvlSet(lbl_1_bss_1292[1], lbl_1_rodata_64);
            sprite_attr_set(lbl_1_bss_1292[1], HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(
                lbl_1_bss_1292[1], memberNo, HUSPR_ATTR_DISPOFF);
            workObj->work[2] = 1;
            workObj->work[3] = 0;
            lbl_1_bss_C94[1] = lbl_1_rodata_60;
            if (lbl_1_data_AC8[1] == 1) {
                HuAudFXPlay(0);
            }
        }
    }
    for (i = 0; i < 3; i++) {
        if (i == lbl_1_bss_1340[1]) {
            update_selected_model(i);
        } else {
            reset_model(i);
        }
    }
}

void fn_1_F15C(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 3; i++) {
        lbl_1_bss_E74[i + 11].unk_30 = 0;
    }
    show_member(0, &lbl_1_data_7A8[0],
        lbl_1_rodata_288, lbl_1_rodata_64);
    show_member(1, &lbl_1_data_7A8[0],
        lbl_1_rodata_148, lbl_1_rodata_64);
    obj->work[0] = 10;
    obj->objFunc = fn_1_EA7C;
    lbl_1_bss_24->mtnId[0] = 1;
}

void fn_1_FCC8(OMOBJ *obj)
{
    OMOBJ *workObj;
    OMOBJ *workObj2;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[0] = 2;
        workObj->work[1] = 0;
        workObj2 = lbl_1_bss_8;
        workObj2->work[2] = 2;
        workObj2->work[3] = 0;
    }
    if (obj->work[0]++ > 30) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

float fn_1_2D0FC(float arg0, float arg1, float arg2, float arg3)
{
    if (arg2 <= lbl_1_rodata_398) {
        return arg0;
    }
    if (arg2 >= arg3) {
        return arg1;
    }
    return arg0 + ((arg2 / arg3) * (arg1 - arg0));
}

float fn_1_2D140(float arg0, float arg1, float arg2)
{
    if (arg0 == arg1) {
        return arg1;
    }
    return (arg1 + (arg0 * (arg2 - lbl_1_rodata_3BC))) / arg2;
}

void fn_1_2D170(s16 layerNo)
{
    if (lbl_1_bss_13C8) {
        GXSetTexCopySrc(0, 0, 640, 480);
        GXSetTexCopyDst(320, 240, GX_TF_RGB565, GX_TRUE);
        GXCopyTex(lbl_1_bss_13C8, GX_FALSE);
    }
}

void fn_1_2E68C(void)
{
    Hu3DLayerHookSet(14, fn_1_2DCD0);
}

void fn_1_2E6B8(void)
{
    Hu3DLayerHookReset(14);
}

void fn_1_2EA9C(void)
{
    lbl_1_bss_1426 = Hu3DParticleCreate(lbl_1_bss_1478[0], 8);
    Hu3DModelPosSet(
        lbl_1_bss_1426, lbl_1_rodata_398, lbl_1_rodata_398,
        lbl_1_rodata_398);
    Hu3DModelScaleSet(
        lbl_1_bss_1426, lbl_1_rodata_3BC, lbl_1_rodata_3BC,
        lbl_1_rodata_3BC);
    Hu3DModelLayerSet(lbl_1_bss_1426, 7);
    Hu3DModelAttrSet(lbl_1_bss_1426, HU3D_ATTR_DISPOFF);
    Hu3DParticleScaleSet(lbl_1_bss_1426, lbl_1_rodata_3BC);
    Hu3DParticleHookSet(lbl_1_bss_1426, fn_1_2E7DC);
    Hu3DParticleBlendModeSet(
        lbl_1_bss_1426, HU3D_PARTICLE_BLEND_ADDCOL);
}

void fn_1_2EBB0(void)
{
    Hu3DModelKill(lbl_1_bss_1426);
}

void fn_1_2EBDC(s16 index, float particleId, HuVecF *position, GXColor color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_141E[index]];
    particle = model->hookData;
    Hu3DModelPosSetV(lbl_1_bss_141E[index], position);
    data = particle->data;
    data->time = 1;
    data->color.r = color.r;
    data->color.g = color.g;
    data->color.b = color.b;
    data->color.a = color.a;
    data->time = 0;
    data->parManId = particleId;
    particle->dataCnt = 1;
    model->attr &= ~HU3D_ATTR_DISPOFF;
}

void fn_1_2F3CC(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        Hu3DModelKill(lbl_1_bss_141E[i]);
    }
}

void fn_1_2F424(void)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_141C];
    particle = model->hookData;
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = 0;
        data->scale = lbl_1_rodata_398;
    }
    particle->dataCnt = 1;
    model->attr &= ~HU3D_ATTR_DISPOFF;
}

void fn_1_2FC50(void)
{
    lbl_1_bss_141C = Hu3DParticleCreate(lbl_1_bss_1478[2], 1000);
    Hu3DModelPosSet(
        lbl_1_bss_141C, lbl_1_rodata_398, lbl_1_rodata_470,
        lbl_1_rodata_398);
    Hu3DModelScaleSet(
        lbl_1_bss_141C, lbl_1_rodata_3BC, lbl_1_rodata_3BC,
        lbl_1_rodata_3BC);
    Hu3DModelLayerSet(lbl_1_bss_141C, 7);
    Hu3DModelAttrSet(lbl_1_bss_141C, HU3D_ATTR_DISPOFF);
    Hu3DParticleScaleSet(lbl_1_bss_141C, lbl_1_rodata_3BC);
    Hu3DParticleHookSet(lbl_1_bss_141C, fn_1_2F4C4);
}

void fn_1_2FD50(void)
{
    Hu3DModelKill(lbl_1_bss_141C);
}

void fn_1_2FD7C(s16 index, float particleId, HuVecF *position)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_1414[index]];
    particle = model->hookData;
    Hu3DModelPosSetV(lbl_1_bss_1414[index], position);
    data = particle->data;
    data->time = 0;
    data->parManId = particleId;
    particle->dataCnt = 1;
    model->attr &= ~HU3D_ATTR_DISPOFF;
}

void fn_1_30730(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        lbl_1_bss_1414[i] = Hu3DParticleCreate(lbl_1_bss_1478[2], 64);
        Hu3DModelPosSet(
            lbl_1_bss_1414[i], lbl_1_rodata_398, lbl_1_rodata_398,
            lbl_1_rodata_398);
        Hu3DModelScaleSet(
            lbl_1_bss_1414[i], lbl_1_rodata_3BC, lbl_1_rodata_3BC,
            lbl_1_rodata_3BC);
        Hu3DModelLayerSet(lbl_1_bss_1414[i], 7);
        Hu3DModelAttrSet(lbl_1_bss_1414[i], HU3D_ATTR_DISPOFF);
        Hu3DParticleScaleSet(lbl_1_bss_1414[i], lbl_1_rodata_3BC);
        Hu3DParticleHookSet(lbl_1_bss_1414[i], fn_1_2FE48);
        Hu3DParticleBlendModeSet(
            lbl_1_bss_1414[i], HU3D_PARTICLE_BLEND_ADDCOL);
    }
}

void fn_1_308C4(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        Hu3DModelKill(lbl_1_bss_1414[i]);
    }
}

void fn_1_31198(s16 index, s16 show)
{
    if (show) {
        Hu3DModelAttrReset(lbl_1_bss_140E[index], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_140E[index], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_31214(s16 index, HuVecF *position, GXColor *color)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_140E[index]];
    particle = model->hookData;
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = 1;
        if (color != NULL) {
            data->color.r = color->r;
            data->color.g = color->g;
            data->color.b = color->b;
        }
    }
    if (position != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_140E[index], position);
    }
    Hu3DModelAttrReset(lbl_1_bss_140E[index], HU3D_ATTR_DISPOFF);
}

void fn_1_31318(s16 index)
{
    s16 i;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    model = &Hu3DData[lbl_1_bss_140E[index]];
    particle = model->hookData;
    for (i = 0, data = particle->data; i < particle->maxCnt; i++, data++) {
        data->time = 2;
    }
}

void fn_1_31818(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_140E[i] = Hu3DParticleCreate(lbl_1_bss_1478[0], 10);
        Hu3DModelPosSet(
            lbl_1_bss_140E[i], lbl_1_rodata_398, lbl_1_rodata_398,
            lbl_1_rodata_398);
        Hu3DModelScaleSet(
            lbl_1_bss_140E[i], lbl_1_rodata_3BC, lbl_1_rodata_3BC,
            lbl_1_rodata_3BC);
        Hu3DModelAttrSet(lbl_1_bss_140E[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_140E[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_140E[i], fn_1_313A0);
        Hu3DParticleBlendModeSet(
            lbl_1_bss_140E[i], HU3D_PARTICLE_BLEND_ADDCOL);
    }
}

void fn_1_31984(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_140E[i]);
    }
}

void fn_1_319DC(s16 index, s16 show)
{
    if (show) {
        Hu3DModelAttrReset(lbl_1_bss_140A[index], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_140A[index], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_31B90(s16 index)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_140A[index]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

void fn_1_321E4(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_140A[i] = Hu3DParticleCreate(lbl_1_bss_1478[3], 256);
        Hu3DModelPosSet(
            lbl_1_bss_140A[i], lbl_1_rodata_398, lbl_1_rodata_398,
            lbl_1_rodata_398);
        Hu3DModelScaleSet(
            lbl_1_bss_140A[i], lbl_1_rodata_3BC, lbl_1_rodata_3BC,
            lbl_1_rodata_3BC);
        Hu3DModelAttrSet(lbl_1_bss_140A[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_140A[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_140A[i], fn_1_31BE4);
        Hu3DParticleBlendModeSet(
            lbl_1_bss_140A[i], HU3D_PARTICLE_BLEND_ADDCOL);
    }
}

void fn_1_32350(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_140A[i]);
    }
}

void fn_1_3276C(s16 groupNo, s16 show)
{
    s16 i;

    for (i = 0; i < 5; i++) {
        if (show) {
            Hu3DModelAttrReset(
                lbl_1_bss_13CE[groupNo][i], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(
                lbl_1_bss_13CE[groupNo][i], HU3D_ATTR_DISPOFF);
        }
    }
}

void fn_1_32988(s16 groupNo)
{
    s16 i;
    HU3D_MODEL *model;
    s16 *work;

    for (i = 0; i < 5; i++) {
        model = &Hu3DData[lbl_1_bss_13CE[groupNo][i]];
        work = model->hookData;
        *work = 0;
    }
}

void fn_1_331E0(void)
{
    s16 i;
    s16 j;

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 5; j++) {
            Hu3DModelKill(lbl_1_bss_13CE[i][j]);
        }
    }
}
