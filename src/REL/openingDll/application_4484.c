#include <dolphin/mtx/GeoTypes.h>

#define OPENING_CHAR_COUNT 10
#define OPENING_WIN_COUNT 4

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_ANIMID;
typedef s16 HUWINID;
typedef struct AnimData_s ANIMDATA;
typedef struct HsfObject_s HSF_OBJECT;

typedef struct HsfTransform_s {
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
} HSF_TRANSFORM;

typedef struct HsfMeshPrefix {
    HSF_OBJECT *parent;
    u32 childNum;
    HSF_OBJECT **child;
    HSF_TRANSFORM base;
} HSF_MESH_PREFIX;

struct HsfObject_s {
    char *name;
    u32 type;
    void *constData;
    u32 flags;
    HSF_MESH_PREFIX mesh;
};

extern const double lbl_1_rodata_98;
extern const double lbl_1_rodata_A8;
extern const double lbl_1_rodata_188;

extern HUWINID lbl_1_bss_2E[OPENING_WIN_COUNT];
extern HU3D_ANIMID lbl_1_bss_60[12];
extern ANIMDATA *lbl_1_bss_7C[79];
extern HU3D_MODELID lbl_1_bss_1CE[8];

extern char *lbl_1_data_2F0[12];

double sin(double value);
double cos(double value);
s32 frandmod(s32 modulus);
void HuPrcSleep(s32 duration);
void HuPrcVSleep(void);
HSF_OBJECT *Hu3DModelObjPtrGet(HU3D_MODELID modelId, char *objName);
ANIMDATA *Hu3DAnimAnimSet(HU3D_ANIMID animId, ANIMDATA *animation);
void HuWinDispOff(HUWINID winId);
void HuWinDispOn(HUWINID winId);
void HuWinMesSet(HUWINID winId, u32 message);
void HuWinHomeClear(HUWINID winId);
void fn_1_4E34(void (*callback)(void), s32 arg0, s32 arg1);
void fn_1_470C(s16 animIndex, s16 frameIndex);
void fn_1_4744(s32 animIndex, s32 frameIndex);

void fn_1_4484(u32 frameCount)
{
    s16 frame;
    s16 i;
    s16 countdowns[OPENING_CHAR_COUNT];
    s16 phases[OPENING_CHAR_COUNT];

    for (i = 0; i < OPENING_CHAR_COUNT; i++) {
        countdowns[i] = (s16)frandmod(30);
        phases[i] = 1;
    }

    for (frame = 0; frame < frameCount - 10; frame++) {
        for (i = 0; i < OPENING_CHAR_COUNT; i++) {
            if (countdowns[i] == 0) {
                fn_1_470C((s16)i,
                    (s16)((i * 7) + 2 + (phases[i] & 1)));
                phases[i]++;
                countdowns[i] = (s16)(frandmod(30) + 20);
            }
            countdowns[i]--;
        }
        HuPrcVSleep();
    }

    HuPrcSleep(11);
}

void fn_1_45D4(s16 modelIndex, float distance)
{
    HSF_OBJECT *object;

    object = Hu3DModelObjPtrGet(
        lbl_1_bss_1CE[2], lbl_1_data_2F0[modelIndex]);
    object->mesh.base.pos.x = lbl_1_rodata_188
        * sin(lbl_1_rodata_98 * distance / lbl_1_rodata_A8);
    object->mesh.base.pos.y = lbl_1_rodata_188
        * cos(lbl_1_rodata_98 * distance / lbl_1_rodata_A8);
}

void fn_1_46B4(s16 animIndex, s16 frameIndex)
{
    Hu3DAnimAnimSet(lbl_1_bss_60[animIndex], lbl_1_bss_7C[frameIndex]);
}

void fn_1_470C(s16 animIndex, s16 frameIndex)
{
    fn_1_4E34((void (*)(void))fn_1_4744, animIndex, frameIndex);
}

void fn_1_4744(s32 animIndex, s32 frameIndex)
{
    s16 scratch = 0;

    HuPrcSleep(5);
    Hu3DAnimAnimSet(lbl_1_bss_60[animIndex], lbl_1_bss_7C[frameIndex]);
}

void fn_1_47AC(s16 winIndex, u32 message, s16 frameCount)
{
    s16 i;

    for (i = 0; i < OPENING_WIN_COUNT; i++) {
        HuWinDispOff(lbl_1_bss_2E[i]);
    }

    HuWinDispOn(lbl_1_bss_2E[winIndex]);
    if (message != 0) {
        HuWinMesSet(lbl_1_bss_2E[winIndex], message);
    } else {
        HuWinHomeClear(lbl_1_bss_2E[winIndex]);
    }

    if (frameCount >= 0 && frameCount != 0) {
        HuPrcSleep(frameCount);
    }
}
