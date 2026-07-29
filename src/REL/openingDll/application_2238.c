#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;

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

extern const float lbl_1_rodata_14;
extern const float lbl_1_rodata_38;
extern const float lbl_1_rodata_78;
extern const double lbl_1_rodata_98;
extern const double lbl_1_rodata_A8;
extern const float lbl_1_rodata_E8;
extern const float lbl_1_rodata_EC;

extern s32 lbl_1_bss_8;
extern s32 lbl_1_bss_C;
extern HU3D_MODELID lbl_1_bss_1CE[8];

extern char *lbl_1_data_2F0[12];
extern char lbl_1_data_355[];

double sin(double value);
double cos(double value);
void HuPrcVSleep(void);
HSF_OBJECT *Hu3DModelObjPtrGet(HU3D_MODELID modelId, char *objName);
void fn_1_46B4(s16 animIndex, s16 frameIndex);

void fn_1_2238(u32 frameCount)
{
    s16 phase;
    s16 count;
    s16 i;

    fn_1_46B4(10, 0x47);
    fn_1_46B4(11, 0x4B);
    phase = 1;
    for (i = 1, count = 0; i <= frameCount; i++) {
        if (count++ > 10) {
            phase ^= 1;
            fn_1_46B4(10, phase + 0x46);
            fn_1_46B4(11, phase + 0x4A);
            count = 0;
        }
        HuPrcVSleep();
    }
    fn_1_46B4(10, 0x46);
    fn_1_46B4(11, 0x4A);
}

void fn_1_2304(u32 guideChar, s32 frameCount)
{
    s16 frame;
    s16 toggleCounter;
    s16 phase;
    BOOL alternate;
    float focusValue;
    HSF_OBJECT *object;

    alternate = guideChar & 0x8000;
    lbl_1_bss_8 = 0;
    guideChar &= 0xFF;
    object = Hu3DModelObjPtrGet(
        lbl_1_bss_1CE[2], lbl_1_data_2F0[guideChar]);

    if (guideChar == 10) {
        if (!alternate) {
            fn_1_46B4((s16)guideChar, 0x47);
        } else {
            fn_1_46B4((s16)guideChar, 0x49);
        }
    } else {
        if (!alternate) {
            fn_1_46B4((s16)guideChar, 0x4B);
        } else {
            fn_1_46B4((s16)guideChar, 0x4D);
        }
    }

    focusValue = lbl_1_rodata_78;
    phase = 1;
    for (frame = 1, toggleCounter = 0; frame <= frameCount; frame++) {
        object->mesh.base.scale.y = focusValue;
        object->mesh.base.scale.x = focusValue;

        if (toggleCounter++ > 5) {
            phase ^= 1;
            if (lbl_1_bss_C != 0) {
                focusValue = lbl_1_rodata_38;
            } else {
                focusValue = (lbl_1_rodata_38 == focusValue)
                    ? lbl_1_rodata_78 : lbl_1_rodata_38;
            }

            if (guideChar == 10) {
                if (!alternate) {
                    fn_1_46B4((s16)guideChar, (s16)(0x46 + phase));
                } else {
                    fn_1_46B4((s16)guideChar, (s16)(0x48 + phase));
                }
            } else {
                if (!alternate) {
                    fn_1_46B4((s16)guideChar, (s16)(0x4A + phase));
                } else {
                    fn_1_46B4((s16)guideChar, (s16)(0x4C + phase));
                }
            }
            toggleCounter = 0;
        }

        if (lbl_1_bss_8 != 0) {
            break;
        }
        HuPrcVSleep();
    }

    if (guideChar == 10) {
        if (!alternate) {
            fn_1_46B4((s16)guideChar, 0x47);
        } else {
            fn_1_46B4((s16)guideChar, 0x49);
        }
    } else {
        if (!alternate) {
            fn_1_46B4((s16)guideChar, 0x4B);
        } else {
            fn_1_46B4((s16)guideChar, 0x4D);
        }
    }
    object->mesh.base.scale.x = object->mesh.base.scale.y = lbl_1_rodata_38;
}

void fn_1_257C(u32 frameCount)
{
    HSF_OBJECT *background;
    HSF_OBJECT *object;
    float phase;
    float amplitude;
    s16 i;

    phase = lbl_1_rodata_14;
    amplitude = lbl_1_rodata_E8;
    background = Hu3DModelObjPtrGet(lbl_1_bss_1CE[2], lbl_1_data_355);

    for (i = 1; ; i++) {
        object = Hu3DModelObjPtrGet(
            lbl_1_bss_1CE[2], lbl_1_data_2F0[10]);
        object->mesh.base.rot.z = -background->mesh.base.rot.z;
        object->mesh.base.rot.z += amplitude
            * sin(lbl_1_rodata_98 * phase / lbl_1_rodata_A8);

        object = Hu3DModelObjPtrGet(
            lbl_1_bss_1CE[2], lbl_1_data_2F0[11]);
        object->mesh.base.rot.z = -background->mesh.base.rot.z;
        object->mesh.base.rot.z += amplitude
            * cos(lbl_1_rodata_98 * phase / lbl_1_rodata_A8);

        phase += lbl_1_rodata_EC;
        if (i > frameCount - 10) {
            amplitude -= lbl_1_rodata_38;
            if (amplitude < lbl_1_rodata_14) {
                amplitude = lbl_1_rodata_14;
            }
        }
        HuPrcVSleep();
    }
}
