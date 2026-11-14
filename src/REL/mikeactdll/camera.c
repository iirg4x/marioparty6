/* Camera setup, player tracking, and screen shake for the microphone activity. */
#include "REL/mikeactdll/particle_effect.h"
#include "game/audio.h"
#include "REL/mikeactdll/module_types.h"
#include "game/object.h"

void fn_1_31C8(void) {
    HuAudSStreamStop(2);
    HuAudSStreamParamSet(0, 0x7FU, 0x1F4U);
    HuAudSStreamPause(0, 0);
}

void fn_1_36F4(OMOBJ *obj);
extern HUPROCESS *lbl_1_bss_6C;
extern f32 lbl_1_bss_84;
extern s32 lbl_1_bss_88;
extern Point3d lbl_1_bss_90;
extern Point3d lbl_1_bss_9C;
extern Point3d lbl_1_bss_A8;
extern OMOBJ *lbl_1_bss_B4;
extern OM_CAMERA_VIEW lbl_1_data_108;

/* Create the activity camera and its shadow projection. */
void fn_1_3208(void) {
    Hu3DCameraPerspectiveSet(1, 20.0f, 20.0f, 25000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0U, 0U, 0x280U, 0x1E0U);
    fn_8003E5A4(&lbl_1_data_108);
    fn_8003D4FC(NULL);
    lbl_1_bss_B4 = fn_8003AB38(lbl_1_bss_6C, 0x7FDA, 0U, 0U, -1, fn_8003D4FC);
    omSetStatBit(lbl_1_bss_B4, 0xA0U);
    lbl_1_bss_B4 = fn_8003AB38(lbl_1_bss_6C, 0x3000, 0U, 0U, -1, fn_1_36F4);
    lbl_1_bss_84 = lbl_1_data_108.center.x;
    lbl_1_bss_84 = 50.0f;
    lbl_1_bss_88 = 0;
    lbl_1_bss_A8.x = lbl_1_data_108.center.x;
    lbl_1_bss_A8.y = 10000.0f;
    lbl_1_bss_A8.z = 0.0f;
    lbl_1_bss_9C.x = 0.0f;
    lbl_1_bss_9C.y = 1.0f;
    lbl_1_bss_9C.z = 0.0f;
    lbl_1_bss_90.x = lbl_1_data_108.center.x;
    lbl_1_bss_90.y = 0.0f;
    lbl_1_bss_90.z = -1.0f;
    Hu3DShadowCreate(10.0f, 5000.0f, 13000.0f);
    Hu3DShadowTPLvlSet(0.5f);
    Hu3DShadowPosSet(&lbl_1_bss_A8, &lbl_1_bss_9C, &lbl_1_bss_90);
}

extern s16 lbl_1_bss_1AA;

void fn_1_34C0(void)
{
    OM_CAMERA_VIEW cameraView;

    lbl_1_bss_1AA = 2;
    cameraView = lbl_1_data_108;
    cameraView.center.x = 350.0f + lbl_1_bss_F0->position.x;
    cameraView.center.y = 100.0f + lbl_1_bss_F0->position.y;
    lbl_1_bss_84 = lbl_1_bss_F0->position.x;
    fn_8003E5A4(&cameraView);
    fn_8003D4FC(NULL);
}

extern s16 lbl_1_bss_70;
extern Point3d lbl_1_bss_74;
extern s32 lbl_1_bss_80;

/* Start a screen shake and save the camera center for restoration. */
void fn_1_35A0(s16 shakeDuration) {
    lbl_1_bss_80 = 1;
    lbl_1_bss_70 = shakeDuration;
    lbl_1_bss_74.x = lbl_80266A4C.x;
    lbl_1_bss_74.y = lbl_80266A4C.y;
    lbl_1_bss_74.z = lbl_80266A4C.z;
}

void fn_1_3608(void)
{
    OM_CAMERA_VIEW cameraView;

    lbl_1_bss_1AA = 2;
    cameraView = lbl_1_data_108;
    cameraView.center.x = 350.0f + lbl_1_bss_F0->position.x;
    cameraView.center.y = 100.0f + lbl_1_bss_F0->position.y;
    lbl_1_bss_84 = lbl_1_bss_F0->position.x;
    fn_8003EA54(&cameraView, 0x1E);
}

extern s16 lbl_1_bss_8C;

void fn_1_36E4(s16 value) {
    lbl_1_bss_8C = value;
}

extern s16 lbl_1_bss_31BA[0x100];

extern f32 lbl_802C028C;

void fn_1_36F4(OMOBJ *obj)
{
    Point3d cameraPosition;
    f32 verticalDifference;

    fn_1_11B44();
    fn_1_3CE8();
    if (lbl_1_bss_80 != 0) {
        lbl_80266A4C.x = lbl_1_bss_74.x + frandmod(0xA) - 5.0f;
        lbl_80266A4C.y = lbl_1_bss_74.y + frandmod(0xA) - 5.0f;
        lbl_80266A4C.z = lbl_1_bss_74.z + frandmod(0xA) - 5.0f;
        if (lbl_1_bss_70-- == 0) {
            lbl_1_bss_80 = 0;
        }
    }

if (lbl_1_bss_8C != 1 && lbl_1_bss_F0->owner != NULL) {
        if (fn_1_4A20() != 0 || lbl_1_bss_88 != 0) {
            if (lbl_802C028C != 4500.0f) {
                lbl_802C028C += 50.0f;
                if (lbl_802C028C > 4500.0f) {
                    lbl_802C028C = 4500.0f;
                }
            }
        } else if (lbl_802C028C != 3350.0f) {
            lbl_802C028C -= 50.0f;
            if (lbl_802C028C < 3350.0f) {
                lbl_802C028C = 3350.0f;
            }
        }

        cameraPosition = lbl_1_bss_F0->position;
        if (lbl_80266A4C.x < 350.0f + cameraPosition.x) {
            lbl_80266A4C.x += 25.0f;
            if (lbl_80266A4C.x > 350.0f + cameraPosition.x) {
                lbl_80266A4C.x = 350.0f + cameraPosition.x;
            }
        }
        if (600.0f + cameraPosition.x < lbl_80266A4C.x) {
            lbl_80266A4C.x = 600.0f + cameraPosition.x;
        }
        if (lbl_1_bss_84 < lbl_80266A4C.x) {
            lbl_1_bss_84 = lbl_80266A4C.x;
        }

        verticalDifference = 0.0f;
        if ((u32)lbl_1_bss_F0->owner != 0) {
            if (lbl_1_bss_F0->owner->player->actor->colGroundAttr & 0x407F) {
                verticalDifference = cameraPosition.y - (lbl_80266A4C.y - 100.0f);
            }
        } else if ((lbl_1_bss_F0->actor->colGroundAttr & 0x407F) != 0) {
            verticalDifference = cameraPosition.y - (lbl_80266A4C.y - 100.0f);
        }
        if (verticalDifference > 0.0f) {
            if (verticalDifference < 20.0f) {
                lbl_80266A4C.y = 100.0f + cameraPosition.y;
            } else {
                lbl_80266A4C.y += 20.0f;
            }
        } else if (verticalDifference < 0.0f) {
            if (verticalDifference > -20.0f) {
                lbl_80266A4C.y = 100.0f + cameraPosition.y;
            } else {
                lbl_80266A4C.y -= 20.0f;
            }
        }
        cameraPosition.y = cameraPosition.z = 0.0f;
        cameraPosition.x = -800.0f + lbl_1_bss_84;
        fn_80026790(lbl_1_bss_31BA[1], &cameraPosition);
    }
}

void fn_1_3CE8(void) {
    lbl_1_bss_A8.x = lbl_80266A4C.x;
    lbl_1_bss_90.x = lbl_80266A4C.x;
    Hu3DShadowPosSet(&lbl_1_bss_A8, &lbl_1_bss_9C, &lbl_1_bss_90);
}

void fn_1_3D50(Mtx output)
{
    HuVecF cameraPosition;
    HuVecF cameraTarget;
    HuVecF cameraUp;
    HuVecF right;
    HuVecF upAxis;
    HuVecF back;
    HuVecF flat;
    HuVecF direction;

    fn_80027CD0(1, &cameraPosition, &cameraUp, &cameraTarget);

    right.x = 1.0f;
    right.y = 0.0f;
    right.z = 0.0f;
    upAxis.x = 0.0f;
    upAxis.y = 1.0f;
    upAxis.z = 0.0f;
    back.x = 0.0f;
    back.y = 0.0f;
    back.z = 1.0f;

    PSVECSubtract(&cameraTarget, &cameraPosition, &direction);
    flat = direction;
    flat.y = 0.0f;
    if (PSVECSquareMag(&flat) < 0.001) {
        flat = direction;
        direction = cameraUp;
        PSVECScale(&flat, &cameraUp, -1.0f);
    }
    direction.y = 0.0f;
    PSVECNormalize(&direction, &direction);

    if (PSVECDotProduct(&upAxis, &cameraUp) < 0.0f) {
        upAxis.y *= (-1.0f);
        back.z *= (-1.0f);
    }
    PSVECCrossProduct(&direction, &upAxis, &right);
    back = direction;

    output[0][0] = right.x;
    output[0][1] = right.y;
    output[0][2] = right.z;
    output[0][3] = 0.0f;
    output[1][0] = upAxis.x;
    output[1][1] = upAxis.y;
    output[1][2] = upAxis.z;
    output[1][3] = 0.0f;
    output[2][0] = back.x;
    output[2][1] = back.y;
    output[2][2] = back.z;
    output[2][3] = 0.0f;
}

void fn_1_3FC4(void)
{
    s32 playerNo;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        fn_1_40A0((s16)playerNo);
        fn_1_45B0(lbl_1_bss_C4[playerNo], 0x2000);
        fn_80087158(lbl_1_bss_C4[playerNo]->player, 1);
        if (playerNo > 0) {
            fn_1_45B0(lbl_1_bss_C4[playerNo], 0x2000);
            fn_80087158(lbl_1_bss_C4[playerNo]->player, 1);
        }
    }
    fn_8005DDEC(7);
    lbl_1_bss_B8 = NULL;
    fn_1_C2C0();
}

OM_CAMERA_VIEW lbl_1_data_108 = { { -650.0f, 100.0f, -150.0f }, { -30.62f, 0.0f, 0.0f }, 3350.0f };

OMOBJ *lbl_1_bss_B4;
Point3d lbl_1_bss_A8;
Point3d lbl_1_bss_9C;
Point3d lbl_1_bss_90;
s16 lbl_1_bss_8C;
s32 lbl_1_bss_88;
f32 lbl_1_bss_84;
s32 lbl_1_bss_80;
Point3d lbl_1_bss_74;
s16 lbl_1_bss_70;
