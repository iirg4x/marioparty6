#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_PARMANID;
typedef s16 HUWINID;

typedef struct MiracleBookCameraView {
    HuVecF center;
    HuVecF rotation;
    float zoom;
} OM_CAMERA_VIEW;

#define HU3D_PARMAN_ATTR_TIMEUP (1 << 0)

double sin(double value);
int HuAudFXPlay(int soundId);
void Hu3DModelPosSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelRotSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelTPLvlSet(HU3D_MODELID modelId, float level);
void Hu3DParManAttrSet(HU3D_PARMANID parManId, s32 attribute);
void HuWinKill(HUWINID windowId);
s16 omCameraViewMoveMulti(
    u32 camera, OM_CAMERA_VIEW *cameraView, s32 time, s16 moveType);

extern const f32 lbl_1_rodata_30;
extern const f32 lbl_1_rodata_54;
extern const f32 lbl_1_rodata_70;
extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_78;
extern const f32 lbl_1_rodata_7C;
extern const f32 lbl_1_rodata_90;
extern const f32 lbl_1_rodata_C0;
extern const f64 lbl_1_rodata_E0;
extern const f64 lbl_1_rodata_E8;
extern const f32 lbl_1_rodata_100;
extern const f32 lbl_1_rodata_104;
extern const f64 lbl_1_rodata_108;
extern const f32 lbl_1_rodata_110;
extern const f64 lbl_1_rodata_118;
extern const f64 lbl_1_rodata_148;
extern const f64 lbl_1_rodata_150;
extern const f32 lbl_1_rodata_158;
extern const f32 lbl_1_rodata_1B0;
extern const HuVecF lbl_1_rodata_200;
extern const f32 lbl_1_rodata_20C;
extern const f32 lbl_1_rodata_210;
extern const f64 lbl_1_rodata_218;
extern const f64 lbl_1_rodata_220;
extern const f32 lbl_1_rodata_228;
extern const f64 lbl_1_rodata_230;
extern const f64 lbl_1_rodata_238;
extern const f64 lbl_1_rodata_240;
extern const f32 lbl_1_rodata_248;
extern const f32 lbl_1_rodata_24C;
extern const f32 lbl_1_rodata_250;

extern s16 lbl_1_bss_68[5];
extern HuVecF lbl_1_bss_78;
extern f32 lbl_1_bss_88;
extern s32 lbl_1_bss_8C;
extern s16 lbl_1_bss_90[3];
extern s32 lbl_1_bss_B4[22];
extern s32 lbl_1_bss_110;
extern f32 lbl_1_bss_118;
extern f32 lbl_1_bss_11C;
extern s32 lbl_1_bss_134;
extern s32 lbl_1_bss_17C;
extern f32 lbl_1_bss_184[6];
extern f32 lbl_1_bss_19C;
extern OM_CAMERA_VIEW lbl_1_bss_1A4;
extern s16 lbl_1_bss_218;
extern s16 lbl_1_bss_21A;

void fn_1_5ED0(s32 arg0);
void fn_1_9130(s16 hookNo, HuVecF *pos);
void fn_1_9718(void);

static inline s32 fn_1_427C(void)
{
    s32 result = 0;

    lbl_1_bss_19C += lbl_1_rodata_110;
    if (lbl_1_bss_19C > lbl_1_rodata_90) {
        lbl_1_bss_19C = lbl_1_rodata_90;
    }

    lbl_1_bss_184[1] = (f32) (lbl_1_bss_11C *
        (lbl_1_rodata_148 - sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8)));
    lbl_1_bss_184[2] = (f32) (lbl_1_bss_118 *
        (lbl_1_rodata_148 - sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8)));
    lbl_1_bss_184[5] = (f32) (lbl_1_rodata_118 *
        sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));

    Hu3DModelPosSet(lbl_1_bss_21A, lbl_1_rodata_30,
        lbl_1_rodata_100 + lbl_1_bss_184[1], lbl_1_rodata_30);
    Hu3DModelRotSet(lbl_1_bss_21A, lbl_1_rodata_104,
        lbl_1_rodata_30, lbl_1_bss_184[2]);
    Hu3DModelPosSet(lbl_1_bss_218, lbl_1_rodata_30,
        lbl_1_rodata_100 + lbl_1_bss_184[1], lbl_1_rodata_30);
    Hu3DModelRotSet(lbl_1_bss_218, lbl_1_rodata_104,
        lbl_1_rodata_30, lbl_1_bss_184[2]);

    lbl_1_bss_184[0] = (f32) (lbl_1_rodata_150 *
        sin(lbl_1_rodata_E0 * lbl_1_bss_19C / lbl_1_rodata_E8));
    if (lbl_1_bss_134 == 1) {
        Hu3DModelPosSet(lbl_1_bss_90[0], lbl_1_rodata_30,
            lbl_1_rodata_70, lbl_1_rodata_74 + lbl_1_bss_184[5]);
        Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
            lbl_1_rodata_70, lbl_1_rodata_74 + lbl_1_bss_184[5]);
    } else if (lbl_1_bss_134 == 2) {
        Hu3DModelPosSet(lbl_1_bss_90[1], lbl_1_rodata_30,
            lbl_1_rodata_78, lbl_1_rodata_7C + lbl_1_bss_184[5]);
        Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
            lbl_1_rodata_78, lbl_1_rodata_7C + lbl_1_bss_184[5]);
    }
    Hu3DModelRotSet(lbl_1_bss_90[0], lbl_1_rodata_158 + lbl_1_bss_184[0],
        lbl_1_rodata_30, lbl_1_rodata_30);
    Hu3DModelRotSet(lbl_1_bss_90[1], lbl_1_rodata_158 + lbl_1_bss_184[0],
        lbl_1_rodata_30, lbl_1_rodata_30);
    Hu3DModelRotSet(lbl_1_bss_90[2], lbl_1_rodata_158 + lbl_1_bss_184[0],
        lbl_1_rodata_30, lbl_1_rodata_30);

    if (lbl_1_rodata_90 == lbl_1_bss_19C) {
        lbl_1_bss_19C = lbl_1_rodata_30;
        result = 1;
    }
    return result;
}

static inline void fn_1_7158(void)
{
    s32 i;

    for (i = 0; i < 22; i++) {
        if (lbl_1_bss_B4[i] != -1) {
            HuWinKill((s16)lbl_1_bss_B4[i]);
            lbl_1_bss_B4[i] = -1;
        }
    }
}

void fn_1_7998(void)
{
    HuVecF pos;
    f32 amplitude;

    switch (lbl_1_bss_8C) {
        case 0:
            fn_1_5ED0(0);
            fn_1_7158();
            HuWinKill((s16) lbl_1_bss_110);
            if (fn_1_427C() == 0) {
                return;
            }
            lbl_1_bss_8C++;
            lbl_1_bss_78.x = lbl_1_bss_78.y = lbl_1_bss_78.z =
                lbl_1_rodata_30;
            lbl_1_bss_88 = lbl_1_rodata_C0;
            HuAudFXPlay(0x48B);
            break;

        case 1:
            lbl_1_bss_78.x += lbl_1_rodata_1B0;
            if (lbl_1_bss_78.x > lbl_1_rodata_20C) {
                lbl_1_bss_78.x = lbl_1_rodata_20C;
            }
            lbl_1_bss_78.y += lbl_1_rodata_C0;
            if (lbl_1_bss_78.y > lbl_1_rodata_90) {
                lbl_1_bss_78.y = lbl_1_rodata_90;
            }
            if (lbl_1_bss_78.x >= lbl_1_rodata_90) {
                lbl_1_bss_78.z += lbl_1_rodata_110;
                if (lbl_1_bss_78.z > lbl_1_rodata_90) {
                    lbl_1_bss_78.z = lbl_1_rodata_90;
                }
            }

            if (lbl_1_bss_134 == 1) {
                amplitude = lbl_1_rodata_210;
                Hu3DModelPosSet(lbl_1_bss_90[0], lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_108 - amplitude *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8)),
                    (f32) (lbl_1_rodata_218 + lbl_1_rodata_118 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.x / lbl_1_rodata_E8)));
                Hu3DModelRotSet(lbl_1_bss_90[0], lbl_1_rodata_30,
                    lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_220 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8) +
                        lbl_1_rodata_220 * (lbl_1_rodata_148 -
                            sin(lbl_1_rodata_E0 *
                                (lbl_1_rodata_90 - lbl_1_bss_78.z) /
                                lbl_1_rodata_E8))));

                Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_108 - amplitude *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8)),
                    (f32) (lbl_1_rodata_218 + lbl_1_rodata_118 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.x / lbl_1_rodata_E8)));
                Hu3DModelRotSet(lbl_1_bss_90[2], lbl_1_rodata_30,
                    lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_220 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8) +
                        lbl_1_rodata_220 * (lbl_1_rodata_148 -
                            sin(lbl_1_rodata_E0 *
                                (lbl_1_rodata_90 - lbl_1_bss_78.z) /
                                lbl_1_rodata_E8))));
            } else if (lbl_1_bss_134 == 2) {
                amplitude = lbl_1_rodata_228;
                Hu3DModelPosSet(lbl_1_bss_90[1], lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_230 + amplitude *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8)),
                    (f32) (lbl_1_rodata_238 + lbl_1_rodata_240 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.x / lbl_1_rodata_E8)));
                Hu3DModelRotSet(lbl_1_bss_90[1], lbl_1_rodata_30,
                    lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_220 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8) +
                        lbl_1_rodata_220 * (lbl_1_rodata_148 -
                            sin(lbl_1_rodata_E0 *
                                (lbl_1_rodata_90 - lbl_1_bss_78.z) /
                                lbl_1_rodata_E8))));

                Hu3DModelPosSet(lbl_1_bss_90[2], lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_230 + amplitude *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8)),
                    (f32) (lbl_1_rodata_238 + lbl_1_rodata_240 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.x / lbl_1_rodata_E8)));
                Hu3DModelRotSet(lbl_1_bss_90[2], lbl_1_rodata_30,
                    lbl_1_rodata_30,
                    (f32) (lbl_1_rodata_220 *
                        sin(lbl_1_rodata_E0 * lbl_1_bss_78.y / lbl_1_rodata_E8) +
                        lbl_1_rodata_220 * (lbl_1_rodata_148 -
                            sin(lbl_1_rodata_E0 *
                                (lbl_1_rodata_90 - lbl_1_bss_78.z) /
                                lbl_1_rodata_E8))));
            }

            if (lbl_1_bss_78.x >= lbl_1_rodata_248) {
                lbl_1_bss_88 -= lbl_1_rodata_24C;
                if (lbl_1_bss_88 < lbl_1_rodata_30) {
                    lbl_1_bss_88 = lbl_1_rodata_30;
                }
                if (lbl_1_bss_134 == 1) {
                    Hu3DModelTPLvlSet(lbl_1_bss_90[0], lbl_1_bss_88);
                } else if (lbl_1_bss_134 == 2) {
                    Hu3DModelTPLvlSet(lbl_1_bss_90[1], lbl_1_bss_88);
                }
                Hu3DModelTPLvlSet(lbl_1_bss_90[2], lbl_1_bss_88);
            }
            fn_1_9718();
            if (lbl_1_bss_78.x > lbl_1_rodata_250) {
                Hu3DParManAttrSet(lbl_1_bss_68[0], HU3D_PARMAN_ATTR_TIMEUP);
                Hu3DParManAttrSet(lbl_1_bss_68[1], HU3D_PARMAN_ATTR_TIMEUP);
                Hu3DParManAttrSet(lbl_1_bss_68[2], HU3D_PARMAN_ATTR_TIMEUP);
                Hu3DParManAttrSet(lbl_1_bss_68[3], HU3D_PARMAN_ATTR_TIMEUP);
                Hu3DParManAttrSet(lbl_1_bss_68[4], HU3D_PARMAN_ATTR_TIMEUP);
            }
            if (lbl_1_rodata_250 == lbl_1_bss_78.x) {
                pos = lbl_1_rodata_200;
                fn_1_9130(0, &pos);
                HuAudFXPlay(0x48C);
            }
            if (lbl_1_rodata_90 == lbl_1_bss_78.x) {
                lbl_1_bss_1A4.zoom = lbl_1_rodata_54;
                omCameraViewMoveMulti(3, &lbl_1_bss_1A4, 90, 1);
            }
            break;
    }
    if (lbl_1_rodata_30 == lbl_1_bss_88) {
        lbl_1_bss_17C = 1;
    }
}
