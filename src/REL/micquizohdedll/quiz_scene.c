/* Microphone quiz model animation, screen effects, and response handling. */
#include "game/hu3d.h"
#include "math.h"
#include "msm_stream.h"

#include "dolphin/gx.h"

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/mtx.h"

#include "string.h"

#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "humath.h"

#include "messdir_enum.h"

#include "game/window.h"

#include "game/process.h"

#include "game/init.h"

#include "game/flag.h"

#include "game/armem.h"

#include "game/mgdata.h"

#include "game/printfunc.h"

#include "game/saveload.h"

#include "game/esprite.h"

#include "game/thpmain.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/masu.h"

#include "game/board/player.h"
#include "REL/micquizohdedll.h"

typedef struct MicquizOverlayRGB {
    s32 red;
    s32 green;
    s32 blue;
} MicquizOverlayRGB;

void fn_1_138E0(void);
void fn_1_144B0(void);
void fn_1_195DC(s16 unused);
void fn_1_19AE4(s16 layerNo);
void fn_1_19B24(s16 level);
void fn_1_1A550(s16 layerNo);
void fn_1_1AA0C(s16 unused);
void fn_1_1ADAC(void);
void fn_1_1AED4(void);
void fn_1_1AED8(s16 layerNo);
void fn_1_1BD68(void *hookContext, f32 (*positionMatrix)[4]);
void fn_1_1C088(HU3D_MODEL *model, f32 (*positionMatrix)[4]);
void fn_1_1D488(void);

/* Quiz animation and transition state. */
ANIMDATA *lbl_1_bss_590;
f32 lbl_1_bss_58C;
s32 lbl_1_bss_588;
s32 lbl_1_bss_584;
s16 lbl_1_bss_580;
s16 lbl_1_bss_57E;
s16 lbl_1_bss_57C;
s16 lbl_1_bss_57A;
s16 lbl_1_bss_578;
s16 lbl_1_bss_570[4];
s16 lbl_1_bss_56E;
s16 lbl_1_bss_56C;
void *lbl_1_bss_568;
u16 * lbl_1_bss_564;
void *lbl_1_bss_55C[2];
s32 lbl_1_bss_558;
f32 lbl_1_bss_554;
s32 lbl_1_bss_550;
s32 lbl_1_bss_54C;
s32 lbl_1_bss_548;
s32 lbl_1_bss_544;
s32 lbl_1_bss_540;
s32 lbl_1_bss_53C;
s32 lbl_1_bss_538;
s32 lbl_1_bss_534;
s32 lbl_1_bss_530;
u32 lbl_1_bss_52C;
s16 lbl_1_bss_528;
s16 lbl_1_bss_526;
s16 lbl_1_bss_524;
s16 lbl_1_bss_522;
s16 lbl_1_bss_516[6];
s16 lbl_1_bss_514;
MicquizCameraState lbl_1_bss_414[8];
GXTexObj lbl_1_bss_3F4;
GXTexObj lbl_1_bss_3D4;
GXTexObj lbl_1_bss_3B4;
s32 lbl_1_bss_38C[0xA];
f32 lbl_1_bss_388;
s32 lbl_1_bss_384;
f32 lbl_1_bss_380;
MicquizBss370Storage lbl_1_bss_370;
s32 lbl_1_bss_36C;
s32 lbl_1_bss_368;
s32 lbl_1_bss_364;
f32 lbl_1_bss_360;
f32 lbl_1_bss_35C;
s32 lbl_1_bss_358;
f32 lbl_1_bss_354;
f32 lbl_1_bss_350;
s32 lbl_1_bss_33C[5];
MicquizBss334Storage lbl_1_bss_334;
s32 lbl_1_bss_330;
s32 lbl_1_bss_32C;
s32 lbl_1_bss_328;

f32 lbl_1_data_530 = 1.0f;
s16 lbl_1_data_534 = -1;
s16 lbl_1_data_536 = -1;
s16 lbl_1_data_538 = -1;
s16 lbl_1_data_53A = -1;
OM_CAMERA_VIEW lbl_1_data_53C[2] = { { { 0.0f, 57.0f, 0.7f }, { -8.0f, 0.0f, 0.0f }, 800.0f },
                                     { { 0.0f, 57.0f, 0.7f }, { -6.0f, 0.0f, 0.0f }, 750.0f } };
s32 lbl_1_data_574[5] = { 10, 20, 30, 40, 50 };
s32 lbl_1_data_588[0x16] = {
    0x8E0000, 0x360000, 0xC00000, 0xF30000, 0xEC0000, 0x140000, 0xE60000, 0x300000,
    0xCF0000, 0xA80000, 0x2A0000, 0x930000, 0x3B0000, 0xC50000, 0xF80000, 0xF10000,
    0x190000, 0xEB0000, 0x350000, 0xD40000, 0xBC0000, 0x2F0000
};
Point3d lbl_1_data_5E0[0xE] = { { 0.0f, 0.0f, 0.0f },     { 0.0f, -19.0f, 0.0f },
                                { 0.0f, -40.0f, -14.0f }, { 0.0f, -11.0f, -25.0f },
                                { 0.0f, 5.0f, -17.0f },   { 0.0f, -36.0f, -14.0f },
                                { 0.0f, -53.0f, -31.0f }, { 0.0f, 36.0f, -2.0f },
                                { 0.0f, 35.5f, -30.0f },  { 0.0f, 48.0f, -36.0f },
                                { 0.0f, 36.0f, -2.0f },   { 0.0f, 48.0f, -36.0f },
                                { 0.0f, 48.0f, -36.0f },  { 0.0f, 48.0f, -36.0f } };
Point3d lbl_1_data_688[6] = { { 20.0f, 61.0f, -25.0f },  { -138.0f, 58.0f, -25.0f },
                              { 40.0f, 50.0f, -25.0f },  { 60.0f, 22.0f, -25.0f },
                              { -21.0f, 42.5f, -25.0f }, { 0.0f, 103.0f, -25.0f } };

/* Adjust the two cameras from controller input. */
void fn_1_12578(void) {
    s16 index;
    s32 color;
    f32 zoom;

    for (index = 0; index < 2; index++) {
        lbl_80266B18[index].y +=
            (f64)(((lbl_802C0010[0] & 0x0008) != 0) -
                  ((lbl_802C0010[0] & 0x0004) != 0)) * 5.0;
        lbl_80266BD8[index] -=
            (f64)(((lbl_802C0010[0] & 0x0002) != 0) -
                  ((lbl_802C0010[0] & 0x0001) != 0)) * 5.0;
        if (lbl_80266BD8[index] <= 5.0f) {
            zoom = 5.0f;
        } else {
            zoom = lbl_80266BD8[index];
        }
        lbl_80266BD8[index] = zoom;
        lbl_80266A58[index].y += (f64)lbl_802BFFF4[0] / 32.0;
        lbl_80266A58[index].x += (f64)lbl_802BFFF0[0] / -32.0;
    }

    for (index = 0; index < -2; index++) {
        if (index == 0) {
            color = 13;
        } else {
            color = 0;
        }
        fontcolor = color;
        print8(0x40 + index * 2, 0x188 + index * 2,
               2.0f, "%f,%f,%f",
               lbl_80266B18->x, lbl_80266B18->y, lbl_80266B18->z);
        print8(0x40 + index * 2, 0x198 + index * 2,
               2.0f, "%f,%f,%f",
               lbl_80266A58->x, lbl_80266A58->y, lbl_80266A58->z);
        print8(0x40 + index * 2, 0x1A8 + index * 2,
               2.0f, "%f", lbl_80266BD8[0]);
    }
}

/* Return which of the two controller buttons is pressed. */
s32 fn_1_12960(void) {
    if ((s32) (lbl_802C0008[0] & 0x100) != 0) {
        return 1;
    }
    if ((s32) (lbl_802C0008[0] & 0x200) != 0) {
        return 2;
    }
    return 0;
}

/* Set opacity on each material of the model. */
void fn_1_129A8(s16 modelIndex, s32 opacity) {
    HU3D_MODEL *model;
    HSF_DATA *hsf;
    HSF_MATERIAL *material;
    HSF_BUFFER *colorBuffer;
    HSF_OBJECT *object;
    s32 i;

    model = &lbl_802C01C8[modelIndex];
    hsf = lbl_802C01C8[modelIndex].hsf;
    material = hsf->material;
    colorBuffer = hsf->color;
    object = hsf->object;

    i = 0;
    while (i < hsf->materialNum) {
        material->refAlpha = (f32)opacity;
        material->hiliteScale = (f32)opacity;
        i++;
        material++;
    }
}

/* Set the model scale using the selected scale curve. */
void fn_1_12A84(f32 scale) {
    f32 edge;
    f32 fraction;

    edge = 0.125f;
    lbl_1_bss_554 = scale;
    fraction = (f32)lbl_1_bss_550 / 300.0f;
    fraction = (fraction < edge) ? 1.0f - (fraction / edge) :
        ((fraction > (1.0f - edge)) ?
            (fraction - (1.0f - edge)) / edge : 0.0f);
    if (lbl_1_bss_584 == 0 && lbl_1_bss_550 > 0) {
        fraction =
            (f32)(0.5 + (0.5 * (f64)fraction));
        lbl_1_bss_554 *= fraction;
    }
    if (lbl_1_bss_584 == 1) {
        f32 inputScale = lbl_1_bss_554;
        f32 curve[5] = { 0.35f, 0.4f, 0.45f, 0.7f, 1.0f };
        lbl_1_bss_554 = (lbl_1_bss_550 > 0) ? 0.0f : 0.0078125f +
                     (curve[lbl_1_bss_544] *
                      ((0.4f * inputScale) +
                       ((0.4f *
                         (inputScale * (inputScale * inputScale))) +
                        (0.2f *
                         (inputScale * inputScale)))));
    }
}

const Point3d lbl_1_rodata_32C = { -100.0f, 8000.0f, 100.0f };

const Point3d lbl_1_rodata_338 = { 0.0f, 1.0f, 0.0f };

const Point3d lbl_1_rodata_344 = { 0.0f, 0.0f, 0.0f };

/* Initialize quiz models, cameras, shadows, effects, and sound. */
void fn_1_12D08(void) {
    Point3d shadowPosition = lbl_1_rodata_32C;
    Point3d shadowTarget = lbl_1_rodata_338;
    Point3d shadowOffset = lbl_1_rodata_344;
    s32 side;
    s32 i;
    u32 tick;
    tick = OSGetTick();
    lbl_1_bss_588 = 0;
    lbl_1_bss_58C = 0.0f;
    MicQuizCountdownInit();
    if (lbl_1_data_388[0] != 0)
    {
        side = rand8() % 2;
    }
    else
    {
        side = !lbl_1_bss_28.value;
    }
    lbl_1_bss_584 = side;
    lbl_1_bss_558 = 0x708;
    lbl_1_bss_544 = MicQuizQuestionLevelGet();
    if (lbl_1_bss_334.value != 0)
    {
        lbl_1_bss_544 = 4;
    }
    fn_1_12A84(1.0f);
    lbl_1_bss_548 = lbl_1_bss_54C = lbl_1_data_574[lbl_1_bss_544];
    lbl_1_bss_52C = MicQuizCurrentPlayerGet();
    do
    {
        lbl_1_bss_540 = (s32)(11.0f * ((f32) rand8() / 256.0f));
    }
    while (lbl_1_bss_540 == 8);
    if (lbl_1_bss_584 == 0)
    {
        s32 typeCounts[5] = { 1, 2, 2, 2, 3 };
        lbl_1_bss_534 = rand8() % 5;
        switch (rand8() % typeCounts[lbl_1_bss_544])
        {
        case 0 :
            lbl_1_bss_53C = 0;
            lbl_1_bss_538 = 0;
            break;
        case 1 :
            lbl_1_bss_53C = (rand8() % 6) + 1;
            lbl_1_bss_538 = 0;
            break;
        case 2 :
            lbl_1_bss_53C = 0;
            lbl_1_bss_538 = (rand8() % 3) + 1;
            break;
        }
    }
    else
    {
        if (lbl_1_bss_584 == 1)
        {
            lbl_1_bss_534 = rand8() % 2;
        }
    }
    if (lbl_1_bss_540 == 8)
    {
        lbl_1_data_530 = 0.625f;
    }
    fn_8003E488(1, &lbl_1_data_53C[(lbl_1_bss_584 == 0) ? 0 : 1]);
    fn_8003E488(2, &lbl_1_data_53C[(lbl_1_bss_584 == 0) ? 0 : 1]);
    Hu3DCameraPerspectiveSet(1, (lbl_1_bss_584 == 0) ? 45.0f : 45.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DCameraPerspectiveSet(2, (lbl_1_bss_584 == 0) ? 45.0f : 45.0f, 10.0f, 20000.0f, 1.2f);
    Hu3DShadowCreate(5.0f, 7000.0f, 9000.0f);
    fn_8002AAB4(0x10, 0x10, 0x10);
    Hu3DShadowPosSet(& shadowPosition, & shadowTarget, & shadowOffset);
    Hu3DShadowTPLvlSet(0.3f);
    lbl_1_bss_526 = fn_80025A04(fn_80008DD0(DATANUM(0xa60000, 0), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_526, 2);
    fn_80026B8C(lbl_1_bss_526, 1);
    fn_800269E8(lbl_1_bss_526, ((1 << 30) | (1 << 0)));
    fn_80026760(lbl_1_bss_526, 0.0f, (f32)((lbl_1_bss_584 == 0) ? 0 : -20), 5.0f);
    lbl_1_bss_524 = fn_80025A04(fn_80008DD0(DATANUM(0xa60000, 1), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_524, 2);
    fn_80026B8C(lbl_1_bss_524, 1);
    fn_800269E8(lbl_1_bss_524, ((1 << 30) | (1 << 0)));
    lbl_1_bss_522 = fn_80025A04(fn_80008DD0(DATANUM(0xa60000, 2), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_522, 2);
    fn_80026B8C(lbl_1_bss_522, 1);
    fn_800269E8(lbl_1_bss_522, ((1 << 30) | (1 << 0)));
    lbl_1_bss_528 = fn_80025A04(fn_80008DD0(DATANUM(0xa60000, 3), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_528, 2);
    fn_80026B8C(lbl_1_bss_528, 1);
    fn_800269E8(lbl_1_bss_528, ((1 << 30) | (1 << 0)));
    fn_1_138E0();
    fn_800097F4(0xa60000);
    fn_1_144B0();
    lbl_1_bss_568 =
        HuMemDirectMallocNum(2, GXGetTexBufferSize(0x280, 0x1E0, GX_TF_RGBA8, 0, 0), 0x10000000);
    lbl_1_bss_564 =
        HuMemDirectMallocNum(2, GXGetTexBufferSize(0x280, 0x1E0, GX_TF_RGBA8, 0, 0), 0x10000000);
    if (lbl_1_bss_584 == 1)
    {
        lbl_1_bss_55C[0] =
            HuMemDirectMallocNum(2, GXGetTexBufferSize(0x1E0, 0x168, GX_TF_I8, 0, 0), 0x10000000);
        lbl_1_bss_55C[1] =
            HuMemDirectMallocNum(2, GXGetTexBufferSize(0x1E0, 0x168, GX_TF_I8, 0, 0), 0x10000000);
    }
    if (lbl_1_bss_584 == 0)
    {
        fn_8002583C(1, 0xE, (HU3D_LAYER_HOOK) fn_1_195DC);
    }
    for (i = 0; i < 8; i++)
    {
        lbl_1_bss_414[i].pendingCount = 0;
        lbl_1_bss_414[i].soundHandle = - 1;
    }
    MicQuizExpectedAnswerSet(lbl_1_bss_540);
    MicQuizPortraitShow(MicQuizCurrentPlayerGet());
    MicQuizMicInit("/mic/ctx/micquiz_chara", 3);
    fn_8000DAC0(fn_1_1D488, 0x64, 0x1800, 0, lbl_1_bss_60.value);
    lbl_1_data_4BC = HuAudSStreamPlay(MSM_STREAM_QUESTION_MUSIC_FIRST);
}

/* Additional hook-name strings; their use is unknown. */
char micquizUnreferencedHookNames[2][0x10] = { "chara_hook", "wario_hook" };

/* Create models and hooks for the selected quiz item. */
void fn_1_138E0(void) {
    Point3d position;
    s32 i;
    s32 modelIndex;
    if (lbl_1_bss_584 == 0)
    {
        if (lbl_1_bss_53C != 0)
        {
    s32 characterModelFiles[6] = {
        DATANUM(0xa60000, 15), DATANUM(0xa60000, 16), DATANUM(0xa60000, 17), DATANUM(0xa60000, 18),
        DATANUM(0xa60000, 19), DATANUM(0xa60000, 20)
    };
    s32 decorationModelFiles[6] = {
        DATANUM(0xa60000, 21), DATANUM(0xa60000, 24), DATANUM(0xa60000, 23), DATANUM(0xa60000, 23),
        DATANUM(0xa60000, 22), DATANUM(0xa60000, 25)
    };
    PSVECAdd(&lbl_1_data_5E0[lbl_1_bss_540], &lbl_1_data_688[lbl_1_bss_53C - 1], &position);
    lbl_1_data_536 =
        fn_80025A04(fn_80008DD0(characterModelFiles[lbl_1_bss_53C - 1], 0x10000000, 2));
    fn_80026B64(lbl_1_data_536, 1);
    fn_80026B8C(lbl_1_data_536, 0);
    fn_800269E8(lbl_1_data_536, ((1 << 30) | (1 << 1)));
    fn_80026E44(lbl_1_data_536);
    lbl_1_data_534 =
        fn_80025A04(fn_80008DD0(decorationModelFiles[lbl_1_bss_53C - 1], 0x10000000, 2));
    fn_80026B64(lbl_1_data_534, 1);
    fn_80026B8C(lbl_1_data_534, 0);
    fn_80026FFC(lbl_1_data_534);
    fn_800269E8(lbl_1_data_534, ((1 << 30) | (1 << 0)));
    lbl_1_data_538 = fn_80025A04(fn_80008DD0(DATANUM(0xa60000, 26), 0x10000000, 2));
    fn_80026B64(lbl_1_data_538, 1);
    fn_80026B8C(lbl_1_data_538, 0);
    fn_80026FFC(lbl_1_data_538);
    fn_800269E8(lbl_1_data_538, ((1 << 30) | (1 << 0)));
    fn_80026760(lbl_1_data_538, position.x, position.y, -28.0f);
        }
        else
        {
            if (lbl_1_bss_538 != 0)
            {
            s32 itemModelFiles[3] = {
                DATANUM(0xa60000, 27), DATANUM(0xa60000, 29), DATANUM(0xa60000, 31)
            };
            lbl_1_data_53A =
                fn_80025A04(fn_80008DD0(itemModelFiles[lbl_1_bss_538 - 1], 0x10000000, 2));
            switch (lbl_1_bss_538) {
            case 1: {
                s16 model;
                model = fn_80025A04(fn_80008DD0(DATANUM(0xa60000, 28), 0x10000000, 2));
                fn_800269E8(model, ((1 << 30) | (1 << 0)));
                fn_80026B64(model, 1);
                fn_80026760(lbl_1_data_53A, 0.0f, 130.0f, 50.0f);
                fn_800268B0(lbl_1_data_53A, 0.75f, 0.75f, 0.75f);
                fn_8002741C(lbl_1_data_53A, "itemhook", model);
                break;
                    }
                case 2 :
                    {
                        s16 model;
                        f32 decorationScale = 0.8f;
                        f32 positionDepth = 65.0f;
                        fn_80026760(lbl_1_data_53A, 0.0f, 0.0f, positionDepth);
                        fn_800268B0(lbl_1_data_53A, decorationScale, decorationScale,
                                    decorationScale);
                        for (i = 0; i < 6; i++)
                        {
                            Point3d decorationPositions[6] = {
                                { 84.0f, 40.0f, 98.0f },  { 84.0f, 40.0f, 6.0f },
                                { 84.0f, 40.0f, -82.0f }, { -84.0f, 40.0f, 98.0f },
                                { -84.0f, 40.0f, 6.0f },  { -84.0f, 40.0f, -82.0f }
                            };
                            lbl_1_bss_516[i] = model =
                                fn_80025A04(fn_80008DD0(DATANUM(0xa60000, 30), 0x10000000, 2));
                            fn_800269E8(model, ((1 << 30) | (1 << 1)));
                            fn_80026B64(model, 1);
                            fn_80026760(model, decorationScale * decorationPositions[i].x,
                                        decorationScale * decorationPositions[i].y,
                                        positionDepth +
                                            (decorationScale * decorationPositions[i].z));
                            fn_80026808(model, 0.0f, (f32) ((i >= 3) ? 0xB4 : 0), 0.0f);
                            fn_800268B0(model, decorationScale, decorationScale, decorationScale);
                            fn_80026E44(model);
                        }
                        break;
                    }
                case 3 :
                    fn_800268B0(lbl_1_data_53A, 0.75f, 0.75f, 0.75f);
                    if (lbl_1_bss_540 == 3 || lbl_1_bss_540 >= 7)
                    {
                        fn_80026760(lbl_1_data_53A, 0.0f, 50.0f, 30.0f);
                        fn_80026808(lbl_1_data_53A, 10.0f, 0.0f, 0.0f);
                    }
                    else
                    {
                        fn_80026760(lbl_1_data_53A, 0.0f, 35.0f, 30.0f);
                    }
                    break;
                }
                fn_800269E8(lbl_1_data_53A, ((1 << 30) | (1 << 0)));
                fn_80026E44(lbl_1_data_53A);
                fn_80026B64(lbl_1_data_53A, 1);
            }
        }
        if (lbl_1_bss_53C == 0)
        {
            lbl_1_bss_530 = rand8() % 10;
            lbl_1_data_534 =
                fn_80025A04(fn_80008DD0(lbl_1_bss_530 + DATANUM(0xa60000, 4), 0x10000000, 2));
            fn_80026B64(lbl_1_data_534, 1);
            fn_80026B8C(lbl_1_data_534, 0);
            fn_80026FFC(lbl_1_data_534);
            fn_800269E8(lbl_1_data_534, ((1 << 30) | (1 << 0)));
            return;
        }
    }
    else if (lbl_1_bss_584 == 1)
    {
        if (lbl_1_bss_534 == 0)
        {
            s32 stageModelFiles[9] = { 0, 0, 0, 0, 0, 2, 1, 1, 1 };
            lbl_1_bss_530 = rand8() % 9;
            for (i = 0; i < lbl_1_data_52C + 1; i++)
            {
                fn_8002583C(1, (s16) i, fn_1_19AE4);
                fn_8002583C(1, (s16)(i + 8), fn_1_19B24);
            }
            fn_8002583C(1, 0xE, fn_1_1A550);
            Hu3DReflectMapSet(fn_80008DD0(stageModelFiles[lbl_1_bss_530] + DATANUM(0xa60000, 0x22),
                                          0x10000000, 2));
            lbl_1_data_534 =
                fn_80025A04(fn_80008DD0(lbl_1_bss_530 + DATANUM(0xa60000, 4), 0x10000000, 2));
            fn_80026B64(lbl_1_data_534, 1);
            fn_80026B8C(lbl_1_data_534, 6);
            fn_80026FFC(lbl_1_data_534);
            fn_800269E8(lbl_1_data_534, ((1 << 30) | (1 << 0)));
        }
        else
        {
            if (lbl_1_bss_534 == 1)
            {
                lbl_1_data_52C = 1;
                lbl_1_bss_590 = HuSprAnimRead(fn_80008DD0(DATANUM(0xa60000, 0x20), 0x10000000, 2));
                Hu3DReflectMapSet(fn_80008DD0(DATANUM(0xa60000, 0x21), 0x10000000, 2));
                fn_8002583C(1, 0, (HU3D_LAYER_HOOK) fn_1_1AA0C);
                fn_8002583C(1, 8, (HU3D_LAYER_HOOK) fn_1_1ADAC);
                fn_8002583C(1, 4, (HU3D_LAYER_HOOK)fn_1_1AED4);
                fn_8002583C(1, 0xC, fn_1_1AED8);
                lbl_1_bss_530 = rand8() % 9;
                lbl_1_data_534 =
                    fn_80025A04(fn_80008DD0(lbl_1_bss_530 + DATANUM(0xa60000, 4), 0x10000000, 2));
                fn_80026B64(lbl_1_data_534, 1);
                fn_80026B8C(lbl_1_data_534, 4);
                fn_80026FFC(lbl_1_data_534);
                fn_800269E8(lbl_1_data_534, ((1 << 30) | (1 << 0)));
            }
        }
        lbl_1_bss_56E = fn_800260F4((HU3D_MODEL_HOOK) fn_1_1BD68);
        fn_80026B8C(lbl_1_bss_56E, 6);
        fn_80026B64(lbl_1_bss_56E, 1);
        fn_800269E8(lbl_1_bss_56E, 1);
        lbl_1_bss_56C = fn_800260F4((HU3D_MODEL_HOOK) fn_1_1C088);
        fn_80026B8C(lbl_1_bss_56C, 1);
        fn_80026B64(lbl_1_bss_56C, 1);
        fn_800269E8(lbl_1_bss_56C, 1);
    }
}

char lbl_1_data_71C[2][0x10] = { "chara_hook", "wario_hook" };

/* Load the selected character model and its motions. */
void fn_1_144B0(void) {
    struct JointMotionOffsets { s32 values[3]; };

    Point3d position;
    HSF_OBJECT *object;
    HSF_BUFFER *colorBuffer;
    HU3D_MODEL *model;
    HSF_DATA *modelData;
    s32 linkIndex;
    HSF_MATERIAL *material;
    f32 scaleFactor;
    f32 positionDepth;
    f32 baseScale;
    s16 linkedModel;
    s16 modelIndex;
    s32 verticalOffset;
    s32 positionIndex;
    s32 hookIndex;
    s32 materialIndex;
    lbl_1_bss_24 = fn_80025A04(fn_80008DD0(
        lbl_1_data_588[lbl_1_bss_540] | 2, 0x10000000, 2));
    lbl_1_bss_57E =
        fn_80030458(lbl_1_bss_24, fn_80008DD0(lbl_1_data_588[lbl_1_bss_540] | 4, 0x10000000, 2));
    fn_8002C95C(lbl_1_bss_24, lbl_1_bss_57E);
    fn_8002D414(lbl_1_bss_24, 0.0f);
    fn_80026B64(lbl_1_bss_24, 1U);
    fn_80026B8C(lbl_1_bss_24, 0);
    fn_800269E8(lbl_1_bss_24, ((1 << 30) | (1 << 0)));
    if (lbl_1_bss_584 == 0) {
        if (lbl_1_bss_53C != 0) {
            fn_80026E44(lbl_1_bss_24);
            lbl_1_bss_57C = fn_80030458(
                lbl_1_bss_24, fn_80008DD0(lbl_1_data_588[lbl_1_bss_540] | 6, 0x10000000, 2));
            lbl_1_bss_578 = fn_80030458(
                lbl_1_bss_24, fn_80008DD0(lbl_1_data_588[lbl_1_bss_540] | 0x31, 0x10000000, 2));
            PSVECAdd(&lbl_1_data_5E0[lbl_1_bss_540], &lbl_1_data_688[lbl_1_bss_53C - 1], &position);
            fn_80026760(lbl_1_bss_24, position.x, position.y, position.z);
            if ((lbl_1_bss_540 == 2) || (lbl_1_bss_540 == 5)) {
                fn_8002C95C(lbl_1_bss_24, lbl_1_bss_57C);
            }
            fn_800269E8(lbl_1_bss_24, ((1 << 30) | (1 << 1)));
            return;
        }
        if (lbl_1_bss_538 != 0) {
            s32 jointMotionOffsets[3] = {
                DATANUM(0x8e0000, 0x20), DATANUM(0x8e0000, 0x20), DATANUM(0x8e0000, 0x4A)
            };
            lbl_1_bss_57C = fn_80030458(lbl_1_bss_24,
                                        fn_80008DD0(lbl_1_data_588[lbl_1_bss_540] |
                                                        (u16) jointMotionOffsets[lbl_1_bss_538 - 1],
                                                    0x10000000, 2));
            switch (lbl_1_bss_538) {          /* switch 2; irregular */
            case 1:                                 /* switch 2 */
                verticalOffset = 1;
                if ((lbl_1_bss_540 != 4) && (lbl_1_bss_540 != 6)) {
                    verticalOffset = 0;
                }
                fn_8002741C(lbl_1_data_53A, lbl_1_data_71C[verticalOffset], lbl_1_bss_24);
                break;
            case 2:                                 /* switch 2 */
                scaleFactor = 0.8f;
                positionDepth = 65.0f;
                fn_800268B0(lbl_1_bss_24, scaleFactor, scaleFactor, scaleFactor);
                fn_800269E8(lbl_1_bss_24, ((1 << 30) | (1 << 1)));
                positionIndex = 1;
                if ((lbl_1_bss_540 != 7) && (lbl_1_bss_540 != 0xA)) {
                    positionIndex = 0;
                }
                if (positionIndex != 0) {
                    hookIndex = 0x14;
                } else {
                    hookIndex = 0;
                }
                fn_80026760(lbl_1_bss_24, 0.0f, (75.0f * scaleFactor) + (f32) hookIndex,
                            (22.0f * scaleFactor) + positionDepth);
                if (lbl_1_bss_540 == 9) {
                    baseScale = 0.92f;
                } else {
                    baseScale = 1.0f;
                }
                scaleFactor = scaleFactor * baseScale;
                if ((lbl_1_bss_540 == 4) || (lbl_1_bss_540 == 6) || (lbl_1_bss_540 == 9)) {
                    fn_800268B0(lbl_1_bss_24, 0.95f * scaleFactor, 0.95f * scaleFactor,
                                0.95f * scaleFactor);
                    fn_80026760(lbl_1_bss_24, 0.0f, 75.0f * scaleFactor,
                                (12.0f * scaleFactor) + positionDepth);
                }
                break;
            case 3:                                 /* switch 2 */
                fn_8002741C(lbl_1_data_53A, "db_ch", lbl_1_bss_24);
                if ((lbl_1_bss_540 == 7) || (lbl_1_bss_540 == 0xA)) {
                    fn_80026760(lbl_1_bss_24, 0.0f, 30.0f, 5.0f);
                }
                break;
            }
            fn_8002C95C(lbl_1_bss_24, lbl_1_bss_57C);
            fn_800269E8(lbl_1_bss_24, ((1 << 30) | (1 << 0)));
            fn_80026B64(lbl_1_bss_24, 1U);
            return;
        }
        lbl_1_bss_578 = fn_80030458(
            lbl_1_bss_24, fn_80008DD0(lbl_1_data_588[lbl_1_bss_540] | 0x31, 0x10000000, 2));
        fn_80026E44(lbl_1_bss_24);
        return;
    } else if (lbl_1_bss_584 == 1) {
        modelIndex = lbl_1_bss_24;
        model = lbl_802C01C8 + modelIndex;
        modelData = lbl_802C01C8[modelIndex].hsf;
        material = modelData->material;
        colorBuffer = modelData->color;
        object = modelData->object;
        for (materialIndex = 0; materialIndex < modelData->materialNum;
             materialIndex++, material++) {
            material->refAlpha = 1.0f;
            material->hiliteScale = 1.0f;
        }
        fn_800269E8(lbl_1_bss_24, ((1 << 30) | (1 << 1)));
        lbl_1_bss_57E = fn_80030458(lbl_1_bss_24,
                                    fn_80008DD0(lbl_1_data_588[lbl_1_bss_540] | 4, 0x10000000, 2));
        lbl_1_bss_578 = fn_80030458(
            lbl_1_bss_24, fn_80008DD0(lbl_1_data_588[lbl_1_bss_540] | 0x31, 0x10000000, 2));
        lbl_1_bss_57A = fn_80030458(
            lbl_1_bss_24, fn_80008DD0(lbl_1_data_588[lbl_1_bss_540] | 0x1A, 0x10000000, 2));
        if (lbl_1_bss_534 == 0) {
            for (linkIndex = 0; linkIndex < (s32)(lbl_1_data_52C + 1); linkIndex++) {
                if (linkIndex < lbl_1_data_52C) {
                    linkedModel = fn_80025D4C(lbl_1_bss_24);
                    lbl_1_bss_570[linkIndex] = linkedModel;
                } else {
                    linkedModel = lbl_1_bss_24;
                }
                fn_80026B8C(linkedModel, (s16)linkIndex);
                fn_80026B64(linkedModel, 1U);
            }
            return;
        } else if (lbl_1_bss_534 == 1) {
            lbl_1_data_52C = 1;
            lbl_1_bss_570[0] = fn_80025D4C(lbl_1_bss_24);
            fn_80026B8C(lbl_1_bss_570[0], 6);
            fn_80026B64(lbl_1_bss_570[0], 1U);
            fn_800269E8(lbl_1_bss_570[0], 1U);
            fn_80026B8C(lbl_1_bss_24, 0);
            fn_80026B64(lbl_1_bss_24, 1U);
        }
    }
}

Point3d lbl_1_data_744 = { 0.0f, 0.0f, 0.0f };
f32 lbl_1_data_750 = -128.0f;
s32 lbl_1_data_754[0x16] = {
    127, 0x41A00000, 0x42740000, -0x3E380000,
    0x41A00000, 0x42740000, -0x3E380000, -0x3CF60000,
    0x42680000, -0x3E380000, 0x42200000, 0x42480000,
    -0x3E380000, 0x42700000, 0x41B00000, -0x3E380000,
    -0x3E580000, 0x422A0000, -0x3E380000, 0,
    0x42CE0000, -0x3E380000
};

void fn_1_14F00(s32 sourceLeftInput, s32 sourceTopInput,
                s32 sourceWidthInput, s32 sourceHeightInput,
                s32 textureFormatInput, f32 speed) {
    void * textureBuffer;
    u32 bufferSize;
    s32 alphaTotal;

    s32 vertex;
    s32 frame;
    s32 alpha;

    f32 screenX;
    f32 screenY;
    f32 textureS;
    f32 textureT;
    f32 screenX2;
    f32 screenY2;
    f32 textureS2;
    f32 textureT2;
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    sourceLeftInput = 0;
    sourceTopInput = 0x22;
    sourceWidthInput = 0x280;
    sourceHeightInput = 0x128;
    sourceLeftInput &= 0xFFFE;
    sourceTopInput &= 0xFFFE;
    sourceWidthInput &= 0xFFFE;
    sourceHeightInput &= 0xFFFE;
    textureFormatInput = GX_TF_RGBA8;
    for (frame = 0; frame < 5; frame++)
    {
        GXSetTexCopySrc((u16) sourceLeftInput, (u16) sourceTopInput, (u16) sourceWidthInput,
                        (u16) sourceHeightInput);
        GXSetTexCopyDst((u16) sourceWidthInput, (u16) sourceHeightInput, textureFormatInput, 0);
        bufferSize = GXGetTexBufferSize((u16) sourceWidthInput, (u16) sourceHeightInput,
                                        textureFormatInput, 0, 0);
        textureBuffer = lbl_1_bss_568;
        GXCopyTex(textureBuffer, 0);
        GXPixModeSync();
        GXInitTexObj(&lbl_1_bss_3F4, textureBuffer, (u16) sourceWidthInput, (u16) sourceHeightInput,
                     textureFormatInput, GX_CLAMP, GX_CLAMP, 0);
        GXInitTexObjLOD(& lbl_1_bss_3F4, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
        GXLoadTexObj(& lbl_1_bss_3F4, GX_TEXMAP0);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        for (vertex = 0; vertex < 4; vertex++)
        {
            f32 phaseOffsets[5] = { 1.0f, 2.0f, 4.0f, 8.0f, -16.0f };
            s32 sourceCorners[4] = { 0, 1, 1, 0 };
            s32 textureCorners[4] = { 0, 0, 1, 1 };
            screenY = (f32) ((sourceHeightInput * textureCorners[vertex]) + sourceTopInput) /
                      480.0f;
            screenX = (f32) (sourceLeftInput + (sourceWidthInput * sourceCorners[vertex])) /
                      640.0f;
            GXWGFifo.f32 = screenX;
            GXWGFifo.f32 = screenY;
            GXWGFifo.f32 = 0.0f;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0x80;
            textureT = (f32) textureCorners[vertex];
            textureS = (f32) sourceCorners[vertex] +
                       (speed * ((6.0f * phaseOffsets[frame]) / (f32) sourceWidthInput));
            GXWGFifo.f32 = textureS;
            GXWGFifo.f32 = textureT;
        }
    }
    alphaTotal = 0xFF;
    GXSetScissor(sourceLeftInput, 0, sourceWidthInput, 385);
    GXSetTexCopySrc((u16) sourceLeftInput, (u16) sourceTopInput, (u16) sourceWidthInput,
                    (u16) sourceHeightInput);
    GXSetTexCopyDst((u16) sourceWidthInput, (u16) sourceHeightInput, textureFormatInput, 0);
    bufferSize = GXGetTexBufferSize((u16) sourceWidthInput, (u16) sourceHeightInput,
                                    textureFormatInput, 0, 0);
    textureBuffer = lbl_1_bss_568;
    GXCopyTex(textureBuffer, 0);
    GXPixModeSync();
    GXInitTexObj(&lbl_1_bss_3F4, textureBuffer, (u16) sourceWidthInput, (u16) sourceHeightInput,
                 textureFormatInput, GX_CLAMP, GX_CLAMP, 0);
    GXInitTexObjLOD(& lbl_1_bss_3F4, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    GXLoadTexObj(& lbl_1_bss_3F4, GX_TEXMAP0);
    for (frame = 1; frame < 8; frame++)
    {
        f32 opacityPhases[10] = { -5.0f, -4.0f, -3.0f, -2.0f, -1.0f, 1.0f, 2.0f,
                                                      3.0f, 4.0f, 5.0f };
        s32 alphaPhases[10] = { 37, 70, 127, 170, 212, 212, 170, 127, 70,
                                                    37 };
        s32 finalSourceCorners[4] = { 0, 1, 1, 0 };
        s32 finalTextureCorners[4] = { 0, 0, 1, 1 };
        alphaTotal += alphaPhases[frame];
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        for (vertex = 0; vertex < 4; vertex++)
        {
            screenY2 =
                (f32) ((sourceHeightInput * finalTextureCorners[vertex]) + sourceTopInput) /
                480.0f;
            screenX2 =
                (f32) (sourceLeftInput + (sourceWidthInput * finalSourceCorners[vertex])) /
                640.0f;
            GXWGFifo.f32 = screenX2;
            GXWGFifo.f32 = screenY2;
            GXWGFifo.f32 = 0.0f;
            alpha = (s32)((255.0f * (f32) alphaPhases[frame]) / (f32) alphaTotal);
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = (u8) alpha;
            textureT2 = (f32) finalTextureCorners[vertex];
            textureS2 = (f32) finalSourceCorners[vertex] +
                        (speed * ((16.0f * opacityPhases[frame]) / (f32) sourceWidthInput));
            GXWGFifo.f32 = textureS2;
            GXWGFifo.f32 = textureT2;
        }
    }
}

void fn_1_157F8(s32 sourceLeft, s32 sourceTop, s32 sourceWidth,
                s32 sourceHeight, GXTexFmt textureFormat, f32 speed) {
    f32 previousAngle;
    f32 angleChange;
    f32 angle;
    f32 sineValue;
    f32 cosineValue;
    f32 textureS;
    f32 textureT;
    f32 screenX;
    f32 screenY;
    void *textureBuffer;
    s32 angleWrap;
    s32 changeWrap;
    s32 vertex;
    s32 frame;
    u32 bufferSize;
    u8 alpha;

    previousAngle = lbl_1_bss_350;
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC,
                    GX_CC_RASC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_RASA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA,
                   GX_BL_INVSRCALPHA, GX_LO_NOOP);

    lbl_1_bss_350 += 2.0f + (62.0f * speed);
    if (lbl_1_bss_350 >= 360.0f) {
        angleWrap = 0x168;
    } else {
        angleWrap = 0;
    }
    lbl_1_bss_350 -= (f32)angleWrap;
    if (speed == 0.0f &&
        ((lbl_1_bss_350 < previousAngle) ||
         (previousAngle == 0.0f))) {
        lbl_1_bss_350 = 0.0f;
    }
    angleChange = lbl_1_bss_350 - previousAngle;
    if (angleChange < 0.0f) {
        changeWrap = 0x168;
    } else {
        changeWrap = 0;
    }
    angleChange += (f32)changeWrap;
    if (fn_8003B748() != 0) {
        lbl_1_bss_350 = previousAngle;
    }

    sourceWidth = sourceHeight * 2;
    sourceLeft = 0x140 - (sourceWidth / 2);
    for (frame = 0; frame < 9; frame++) {
        GXPixModeSync();
        GXSetTexCopySrc((u16)sourceLeft, (u16)sourceTop,
                        (u16)sourceWidth, (u16)sourceHeight);
        GXSetTexCopyDst((u16)sourceWidth, (u16)sourceHeight,
                        textureFormat, 0);
        bufferSize = GXGetTexBufferSize((u16)sourceWidth,
                                        (u16)sourceHeight,
                                        textureFormat, 0, 0);
        textureBuffer = lbl_1_bss_568;
        GXCopyTex(textureBuffer, 0);
        GXPixModeSync();
        GXInvalidateTexAll();
        GXInitTexObj(&lbl_1_bss_3D4, textureBuffer,
                     (u16)sourceWidth, (u16)sourceHeight,
                     textureFormat, GX_CLAMP, GX_CLAMP, 0);
        GXInitTexObjLOD(&lbl_1_bss_3D4, GX_LINEAR, GX_LINEAR,
                        0.0f, 0.0f,
                        0.0f, 0, 0, GX_ANISO_1);
        GXLoadTexObj(&lbl_1_bss_3D4, GX_TEXMAP0);
        GXPixModeSync();
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);

        for (vertex = 0; vertex < 4; vertex++) {
            f32 angleFractions[9] = {0.0f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f};
            s32 xCorners[4] = {0, 1, 1, 0};
            s32 yCorners[4] = {0, 0, 1, 1};
            angle = 135.0f +
                    ((frame == 0 ? previousAngle : 0.0f) +
                     ((angleFractions[frame] * angleChange) /
                      128.0f) * (speed > 0.25f ? 2.0f : 2.0f * (speed / 0.25f))) -
                    (f32)(vertex * 0x5A);
            sineValue = (f32)sin((M_PI * (f64)angle) /
                                 180.0);
            cosineValue = (f32)cos((M_PI * (f64)angle) /
                                   180.0);
            textureS = (f32)(0.5 +
                             (0.5 *
                              ((f64)cosineValue / cos(((M_PI * 45.0) / 180.0)))));
            textureT = (f32)(0.5 +
                             (0.5 *
                              (-(f64)sineValue / sin(((M_PI * 45.0) / 180.0)))));
            screenY = (f32)(sourceTop +
                            (sourceHeight * yCorners[vertex])) /
                      480.0f;
            screenX = (f32)(sourceLeft +
                            (sourceWidth * xCorners[vertex])) /
                      640.0f;
            GXWGFifo.f32 = screenX;
            GXWGFifo.f32 = screenY;
            GXWGFifo.f32 = 0.0f;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            if (frame == 0) {
                alpha = 0xFF;
            } else {
                alpha = 0x80;
            }
            GXWGFifo.u8 = alpha;
            GXWGFifo.f32 = textureS;
            GXWGFifo.f32 = textureT;
        }
        GXPixModeSync();
    }
}

enum { MICQUIZ_WIPE_FRAME_PERIOD = 0x1000000 };

void fn_1_15F60(s32 sourceLeft, s32 sourceTop, s32 sourceWidth,
                s32 sourceHeight, GXTexFmt textureFormat, f32 speed) {
    f32 previousAngle;
    f32 angleChange;
    f32 textureWidthScale;
    f32 textureHeightScale;
    f32 textureScaleDivisor;
    f32 textureScale;
    f32 targetWidth;
    f32 targetHeight;
    f32 screenX;
    f32 screenY;
    f32 textureS;
    f32 textureT;
    u32 bufferSize;
    void *textureData;
    u8 alpha;
    s32 angleWrap;
    s32 angleChangeWrap;
    s32 width;
    s32 height;
    s32 pass;
    s32 frame;
    s32 vertex;

    previousAngle = lbl_1_bss_354;
    lbl_1_bss_358++;
    lbl_1_bss_358 %= MICQUIZ_WIPE_FRAME_PERIOD;

    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC,
                    GX_CC_RASC, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_RASA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA,
                   GX_BL_INVSRCALPHA, GX_LO_NOOP);

    lbl_1_bss_354 += 2.0f + (62.0f * speed);
    if (lbl_1_bss_354 >= 360.0f) {
        angleWrap = 0x168;
    } else {
        angleWrap = 0;
    }
    lbl_1_bss_354 -= (f32)angleWrap;
    if (speed == 0.0f &&
        ((lbl_1_bss_354 < previousAngle) ||
         (previousAngle == 0.0f))) {
        lbl_1_bss_354 = 0.0f;
    }
    angleChange = lbl_1_bss_354 - previousAngle;
    if (angleChange < 0.0f) {
        angleChangeWrap = 0x168;
    } else {
        angleChangeWrap = 0;
    }
    angleChange += (f32)angleChangeWrap;
    if (fn_8003B748() != 0) {
        lbl_1_bss_354 = previousAngle;
    }

    textureScaleDivisor = 1.0f + (47.0f * speed);
    textureWidthScale = (f32)sourceWidth / textureScaleDivisor;
    textureHeightScale = (f32)sourceHeight / textureScaleDivisor;
    for (pass = 0; pass < 2; pass++) {
        if (pass == 0) {
            targetWidth = (f32)sourceWidth;
            targetHeight = (f32)sourceHeight;
        } else {
            targetWidth = textureWidthScale;
            targetHeight = textureHeightScale;
        }
        GXPixModeSync();
        GXSetTexCopySrc((u16)sourceLeft, (u16)sourceTop,
                        (u16)targetWidth, (u16)targetHeight);
        GXSetTexCopyDst((u16)targetWidth, (u16)targetHeight,
                        textureFormat, 0);
        bufferSize = GXGetTexBufferSize((u16)targetWidth,
                                        (u16)targetHeight,
                                        textureFormat, 0, 0);
        textureData = lbl_1_bss_568;
        GXCopyTex(textureData, 0);
        GXPixModeSync();
        GXInvalidateTexAll();
        GXInitTexObj(&lbl_1_bss_3B4, textureData,
                     (u16)targetWidth, (u16)targetHeight,
                     textureFormat, GX_CLAMP, GX_CLAMP, 0);
        if (pass == 0) {
            GXInitTexObjLOD(&lbl_1_bss_3B4, GX_LINEAR, GX_LINEAR,
                            0.0f, 0.0f,
                            0.0f, 0, 0, GX_ANISO_1);
        } else {
            GXInitTexObjLOD(&lbl_1_bss_3B4, GX_NEAR, GX_NEAR,
                            0.0f, 0.0f,
                            0.0f, 0, 0, GX_ANISO_1);
        }
        GXLoadTexObj(&lbl_1_bss_3B4, GX_TEXMAP0);
        GXPixModeSync();

        if (pass != 0) {
            targetWidth = (f32)sourceWidth;
            targetHeight = (f32)sourceHeight;
        } else {
            targetWidth = textureWidthScale;
            targetHeight = textureHeightScale;
        }

        frame = 0;
        while (frame < (pass == 0 ? 2 : 1)) {
            textureScale = 0.0f;
            alpha = 0xFF;
            if ((lbl_1_bss_358 % 2) != 0 && pass == 0 && frame == 1) {
                textureScale = 0.01f / targetHeight;
                alpha = 0x80;
            }
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            for (vertex = 0; vertex < 4; vertex++) {
                s32 xCorners[4] = {0, 1, 1, 0};
                s32 yCorners[4] = {0, 0, 1, 1};

                screenY = ((f32)sourceTop +
                           (targetHeight * yCorners[vertex])) /
                          480.0f;
                screenX = ((f32)sourceLeft +
                           (targetWidth * xCorners[vertex])) /
                          640.0f;
                GXWGFifo.f32 = screenX;
                GXWGFifo.f32 = screenY;
                GXWGFifo.f32 = 0.0f;
                GXWGFifo.u8 = 0xFF;
                GXWGFifo.u8 = 0xFF;
                GXWGFifo.u8 = 0xFF;
                GXWGFifo.u8 = alpha;
                textureT = textureScale +
                           (f32)yCorners[vertex];
                textureS = (f32)xCorners[vertex];
                GXWGFifo.f32 = textureS;
                GXWGFifo.f32 = textureT;
            }
            frame++;
        }
        GXPixModeSync();
    }
}

void fn_1_16804(s32 sourceLeft, s32 sourceTop, s32 sourceWidth,
                s32 sourceHeight, GXTexFmt textureFormat, f32 strength) {
    GXTexObj intensityTexture;
    GXTexObj rgbaTexture;
    f32 tileCount;
    f32 u;
    f32 v;
    f32 v2;
    f32 x;
    f32 y;
    f32 previousIntensity;
    f32 textureU;
    f32 textureV;
    s32 pass;
    s32 column;
    s32 corner;
    s32 bufferSize;
    previousIntensity = lbl_1_bss_35C;
    if (strength != 0.0f)
    {
        s32 copyWidth = sourceWidth;
        s32 copyHeight = sourceHeight;
        GXSetScissor(sourceLeft, sourceTop, sourceWidth, sourceHeight);
        GXSetViewport(0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        GXSetTexCopySrc((u16) sourceLeft, (u16) sourceTop, (u16) sourceWidth, (u16) sourceHeight);
        GXSetTexCopyDst((u16) sourceWidth, (u16) sourceHeight, textureFormat, 0);
        GXCopyTex(lbl_1_bss_568, 1);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        for (pass = 0; pass < 2; pass++)
        {
            tileCount = 1.0f + (15.0f * strength);
            for (column = 0; (f32) column <= tileCount; column++)
            {
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                for (corner = 0; corner < 4; corner++)
                {
                    s32 sourceCorners[4] = {0, 1, 1, 0};
                    s32 targetCorners[4] = {0, 0, 1, 1};
                    if (pass == 0)
                    {
                        u = (f32)(column + sourceCorners[corner]) / tileCount;
                        v = (f32)(copyHeight * targetCorners[corner] + sourceTop) / 480.0f;
                        GXWGFifo.f32 = ((f32) sourceLeft + (f32) copyWidth * u) / 640.0f;
                        GXWGFifo.f32 = v;
                        GXWGFifo.f32 = 0.0f;
                        GXColor4u8(0, (u8)(sourceCorners[corner] * 0xFF), 0, 0xFF);
                    }
                    else
                    {
                        u = (f32)(column + targetCorners[corner]) / tileCount;
                        v2 = (f32)(sourceLeft + copyWidth * sourceCorners[corner]) / 640.0f;
                        GXWGFifo.f32 = v2;
                        GXWGFifo.f32 = ((f32) sourceTop + (f32) copyHeight * u) / 480.0f;
                        GXWGFifo.f32 = 0.0f;
                        GXColor4u8(0, 0, (u8)(targetCorners[corner] * 0xFF), 0xFF);
                    }
                }
            }
            GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP);
        }
        GXSetTexCopySrc((u16) sourceLeft, (u16) sourceTop, (u16) copyWidth, (u16) copyHeight);
        GXSetTexCopyDst((u16) copyWidth, (u16) copyHeight, GX_CTF_GB8, 0);
        GXCopyTex(lbl_1_bss_564, 1);
        GXPixModeSync();
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
        GXSetVtxDesc(GX_VA_CLR1, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR1, GX_TEX_ST, GX_RGBA8, 0);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
        GXSetVtxDesc(GX_VA_TEX1, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_RGBA6, 0);
        GXInitTexObj(&rgbaTexture, lbl_1_bss_568, (u16) sourceWidth, (u16) sourceHeight,
                     textureFormat, GX_CLAMP, GX_CLAMP, 0);
        GXInitTexObjLOD(& rgbaTexture, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
        GXLoadTexObj(& rgbaTexture, GX_TEXMAP0);
        GXInitTexObj(&intensityTexture, lbl_1_bss_564, (u16) copyWidth, (u16) copyHeight, GX_TF_IA8,
                     GX_CLAMP, GX_CLAMP, 0);
        GXInitTexObjLOD(& intensityTexture, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
        GXLoadTexObj(& intensityTexture, GX_TEXMAP1);
        GXSetNumTevStages(1);
        GXSetNumTexGens(1);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP,
                      GX_AF_NONE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        {
        f32 matrix[2][3] = {{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};
        matrix[0][1] *= ((f32) sourceWidth / 254.0f) / 2.0f;
        matrix[1][0] *= ((f32) sourceHeight / 254.0f) / 2.0f;
        GXSetNumTexGens(2);
        GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, 0, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
        GXSetNumIndStages(1);
        GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
        GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
        GXSetIndTexMtx(GX_ITM_0, matrix, 1);
        GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 0, 1, GX_ITM_0);
        GXPixModeSync();
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        for (corner = 0; corner < 4; corner++)
        {
            f32 finalSourceCorners[4] = {0.0f, 1.0f, 1.0f, 0.0f};
            f32 finalTargetCorners[4] = {0.0f, 0.0f, 1.0f, 1.0f};
            y = ((f32) sourceTop + sourceHeight * finalTargetCorners[corner]) / 480.0f;
            x = ((f32) sourceLeft + sourceWidth * finalSourceCorners[corner]) / 640.0f;
            GXWGFifo.f32 = x;
            GXWGFifo.f32 = y;
            GXWGFifo.f32 = 0.0f;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0;
            GXWGFifo.u8 = 0;
            GXWGFifo.u8 = 0x80;
            textureV = finalTargetCorners[corner];
            textureU = finalSourceCorners[corner];
            GXWGFifo.f32 = textureU;
            GXWGFifo.f32 = textureV;
            {
                f32 textureU2;
                f32 textureV2;
                textureV2 = finalTargetCorners[corner];
                textureU2 = finalSourceCorners[corner];
                GXWGFifo.f32 = textureU2;
                GXWGFifo.f32 = textureV2;
            }
        }
        GXPixModeSync();
        GXSetNumIndStages(0);
        GXSetTevDirect(GX_TEVSTAGE0);
        }
    }
}

/* Distort a captured screen region with the noise texture. */
void fn_1_17478(s32 sourceLeft, s32 sourceTop, s32 sourceWidth, s32 sourceHeight,
                enum _GXTexFmt textureFormat, f32 strength) {
    GXTexObj noiseTexture;
    GXTexObj colorTexture;
    f32 priorEffect;
    f32 screenX;
    f32 screenY;
    f32 textureY;
    f32 textureX;
    f32 noiseX;
    f32 noiseY;
    s32 row;
    s32 column;
    u16 *pixel;

    priorEffect = lbl_1_bss_360;
    if (strength != 0.0f) {
        GXSetScissor(sourceLeft, sourceTop, sourceWidth, sourceHeight);
        GXSetViewport(0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        if (lbl_1_bss_364 == 0) {
            for (row = 0; row < 0x200; row++) {
                for (column = 0; column < 0x200; column++) {
                    pixel = &lbl_1_bss_564[column + (row << 9)];
                    *pixel = rand8() + (rand8() << 8);
                }
            }
            DCFlushRange(lbl_1_bss_564, 0x80000U);
            lbl_1_bss_364 = 1;
        }
        GXSetTexCopySrc((u16) sourceLeft, (u16) sourceTop, (u16) sourceWidth, (u16) sourceHeight);
        GXSetTexCopyDst((u16) sourceWidth, (u16) sourceHeight, textureFormat, 0U);
        GXCopyTex(lbl_1_bss_568, 1U);
        GXPixModeSync();
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0U);
        GXSetVtxDesc(GX_VA_CLR1, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR1, GX_TEX_ST, GX_RGBA8, 0U);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0U);
        GXSetVtxDesc(GX_VA_TEX1, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_RGBA6, 0U);
        GXInvalidateTexAll();
        GXInitTexObj(&colorTexture, lbl_1_bss_568, (u16) sourceWidth, (u16) sourceHeight,
                     textureFormat, GX_CLAMP, GX_CLAMP, 0U);
        GXInitTexObjLOD(&colorTexture, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0U, 0U, GX_ANISO_1);
        GXLoadTexObj(&colorTexture, GX_TEXMAP0);
        GXInitTexObj(&noiseTexture, lbl_1_bss_564, 0x200U, 0x200U, GX_TF_IA8, GX_REPEAT, GX_REPEAT,
                     0U);
        GXInitTexObjLOD(&noiseTexture, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, 0U, 0U, GX_ANISO_1);
        GXLoadTexObj(&noiseTexture, GX_TEXMAP1);
        GXSetNumTevStages(1U);
        GXSetNumTexGens(1U);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0U, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        GXSetNumChans(1U);
        GXSetChanCtrl(GX_COLOR0A0, 0U, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP,
                      GX_AF_NONE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        {
        f32 indTexMtx[2][3] = {{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}};

        indTexMtx[0][1] *= (((f32) sourceWidth / 254.0f) / 2.0f * strength) /
                           3.0f;
        indTexMtx[1][0] *= (((f32) sourceHeight / 254.0f) / 2.0f * strength) /
                           3.0f;
        GXSetNumTexGens(2U);
        GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, 0U, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
        GXSetNumIndStages(1U);
        GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
        GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
        GXSetIndTexMtx(GX_ITM_0, indTexMtx, 1);
        GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 1U, 0U, GX_ITM_0);
        }
        GXPixModeSync();
        if (fn_8003B748() == 0) {
            lbl_1_bss_368 = ((s32)rand8() % 2) + (rand8() * 2);
            lbl_1_bss_36C = ((s32)rand8() % 2) + (rand8() * 2);
        }
        GXBegin(GX_QUADS, GX_VTXFMT0, 4U);
        for (column = 0; column < 4; column++) {
            f32 xCorners[4] = {0.0f, 1.0f, 1.0f, 0.0f};
            f32 yCorners[4] = {0.0f, 0.0f, 1.0f, 1.0f};

            screenY = ((f32)sourceTop + ((f32)sourceHeight * yCorners[column])) / 480.0f;
            screenX = ((f32)sourceLeft + ((f32)sourceWidth * xCorners[column])) / 640.0f;
            GXWGFifo.f32 = screenX;
            GXWGFifo.f32 = screenY;
            GXWGFifo.f32 = 0.0f;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0xFF;
            GXWGFifo.u8 = 0;
            GXWGFifo.u8 = 0;
            GXWGFifo.u8 = 0x80;
            textureX = yCorners[column];
            textureY = xCorners[column];
            GXWGFifo.f32 = textureY;
            GXWGFifo.f32 = textureX;
            noiseY = (f32)sourceHeight * (((f32)lbl_1_bss_36C + yCorners[column]) / 512.0f);
            noiseX = (f32)sourceWidth * (((f32)lbl_1_bss_368 + xCorners[column]) / 512.0f);
            GXWGFifo.f32 = noiseX;
            GXWGFifo.f32 = noiseY;
        }
        GXPixModeSync();
        GXSetNumIndStages(0U);
        GXSetTevDirect(GX_TEVSTAGE0);
    }
}

/* Draw a captured screen region on a displaced grid. */
void fn_1_17E20(s32 sourceLeft, s32 sourceTop, s32 sourceWidth, s32 sourceHeight,
                enum _GXTexFmt textureFormat, f32 strength) {
    f32 positionX[48][25];
    f32 positionY[48][25];
    f32 textureS[48][25];
    f32 textureT[48][25];
    f32 sineTable[48];
    f32 cosineTable[48];
    GXTexObj texture[2]; /* only texture[0] is initialized and loaded */
    f32 previousEffect;
    f32 layerCount;
    f32 sampleCount;
    f32 radialX;
    f32 radialY;
    f32 vertexX;
    f32 vertexY;
    f32 textureSValue;
    f32 textureTValue;
    s32 side;
    s32 layerIndex;
    s32 arrayRow;
    s32 sampleIndex;
    s32 layer;
    s32 sample;
    previousEffect = lbl_1_bss_370.value;
    if (strength != 0.0f) {
        GXSetScissor(sourceLeft, sourceTop, sourceWidth, sourceHeight);
        GXSetViewport(0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        GXSetTexCopySrc((u16) sourceLeft, (u16) sourceTop, (u16) sourceWidth, (u16) sourceHeight);
        GXSetTexCopyDst((u16) sourceWidth, (u16) sourceHeight, textureFormat, 0U);
        GXCopyTex(lbl_1_bss_568, 0U);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1U, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXInitTexObj(&texture[0], lbl_1_bss_568, (u16) sourceWidth, (u16) sourceHeight,
                     textureFormat, GX_CLAMP, GX_CLAMP, 0U);
        GXInitTexObjLOD(&texture[0], GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0U, 0U, GX_ANISO_1);
        GXLoadTexObj(&texture[0], GX_TEXMAP0);
        sampleCount = 48.0f;
        layerCount = 24.0f;

            for (sample = 0; sample < sampleCount; sample++) {
                sineTable[sample] = sin((M_PI * ((sample / sampleCount) * 360.0f)) / 180.0);
                cosineTable[sample] = cos((M_PI * ((sample / sampleCount) * 360.0f)) / 180.0);
            }
        for (layer = 0; layer < (1.0f + layerCount); layer++) {
            for (sample = 0; sample < sampleCount; sample++) {
                radialX = 0.5f + (0.5f * ((layer * sineTable[sample]) / layerCount));
                radialY = 0.5f + (0.5f * ((layer * cosineTable[sample]) / layerCount));
                positionX[sample][layer] = (sourceLeft + (sourceWidth * radialX)) / 640.0f;
                positionY[sample][layer] = (sourceTop + (sourceHeight * radialY)) / 480.0f;
                textureS[sample][layer] =
                    (f32)(0.5 + 0.5 * (((f64)layer *
                                       sin((M_PI *
                                            ((360.0f * (sample / sampleCount)) +
                                             (strength * (90.0f * (layerCount - layer))))) /
                                           180.0)) /
                                      layerCount));
                textureT[sample][layer] =
                    (f32)(0.5 + 0.5 * (((f64)layer *
                                       cos((M_PI *
                                            ((360.0f * (sample / sampleCount)) +
                                             (strength * (90.0f * (layerCount - layer))))) /
                                           180.0)) /
                                      layerCount));
}
}
        for (layer = 0; layer < layerCount; layer++) {
            GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 2.0f + (2.0f * sampleCount));
            for (sample = 0; sample < (1.0f + sampleCount); sample++) {
                for (side = 1; side >= 0; side--) {
                    layerIndex = side + layer;
                    if (sample == sampleCount) {
                        sampleIndex = 0;
                    } else {
                        sampleIndex = sample;
                    }
                    arrayRow = sampleIndex;
                    vertexY = positionY[arrayRow][layerIndex];
                    vertexX = positionX[arrayRow][layerIndex];
                    GXWGFifo.f32 = vertexX;
                    GXWGFifo.f32 = vertexY;
                    GXWGFifo.f32 = 0.0f;
                    GXWGFifo.u8 = 0xFF;
                    GXWGFifo.u8 = 0xFF;
                    GXWGFifo.u8 = 0xFF;
                    GXWGFifo.u8 = 0xFF;
                    textureTValue = textureT[arrayRow][layerIndex];
                    textureSValue = textureS[arrayRow][layerIndex];
                    GXWGFifo.f32 = textureSValue;
                    GXWGFifo.f32 = textureTValue;
}
}
}
    }
}

void fn_1_187F0(s32 sourceLeft, s32 sourceTop, s32 sourceWidth,
                s32 sourceHeight, GXTexFmt textureFormat, f32 strength) {
    GXTexObj alphaTexture;
    GXTexObj colorTexture;
    f32 matrix[2][3];
    f32 priorEffect;
    s32 row;
    s32 column;
    s32 alpha;
    s32 alpha2;
    s32 tileSize;
    f32 u;
    f32 v;
    f32 screenXValue;
    f32 edgeScale;
    f32 edgeScale2;
    f32 textureYValue;
    priorEffect = lbl_1_bss_380;
    tileSize = 0x80;
    if (strength != 0.0f)
    {
        GXSetScissor(sourceLeft, sourceTop, sourceWidth, sourceHeight);
        GXSetViewport(0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
        GXSetTexCopySrc((u16) sourceLeft, (u16) sourceTop, (u16) sourceWidth, (u16) sourceHeight);
        GXSetTexCopyDst((u16) sourceWidth, (u16) sourceHeight, textureFormat, 0);
        GXCopyTex(lbl_1_bss_568, 1);
        GXPixModeSync();
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 0x22);
        for (row = 0; row < 0x11; row++)
        {
            for (column = 0; column < 2; column++)
            {
                GXPosition3f32((f32) (sourceLeft + tileSize * column) / 640.0f,
                               ((f32) sourceTop + (f32) (tileSize * row) / 16.0f) / 480.0f, 0.0f);
                alpha =
                    (s32) (128.0 + (127.0 * sin((M_PI * (360.0f * ((f32) row / 16.0f))) / 180.0)));
                GXColor4u8(0, (u8) alpha, 0, 0xFF);
            }
        }
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 0x22);
        for (row = 0; row < 0x11; row++)
        {
            for (column = 0; column < 2; column++)
            {
                GXPosition3f32(((f32) sourceLeft + (f32) (tileSize * row) / 16.0f) / 640.0f,
                               (f32) (sourceTop + (tileSize * (1 - column))) / 480.0f, 0.0f);
                alpha2 =
                    (s32) (128.0 + (127.0 * sin((M_PI * (125.0f + (360.0f * ((f32) row / 16.0f)))) /
                                                180.0)));
                GXColor4u8(0, 0, (u8) alpha2, 0xFF);
            }
        }
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetTexCopySrc((u16) sourceLeft, (u16) sourceTop, (u16) tileSize, (u16) tileSize);
        GXSetTexCopyDst((u16) tileSize, (u16) tileSize, GX_CTF_GB8, 0);
        GXCopyTex(lbl_1_bss_564, 1);
        GXPixModeSync();
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
        GXSetVtxDesc(GX_VA_TEX1, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_RGBA6, 0);
        GXInvalidateTexAll();
        GXInitTexObj(&colorTexture, lbl_1_bss_568, (u16) sourceWidth, (u16) sourceHeight,
                     textureFormat, GX_CLAMP, GX_CLAMP, 0);
        GXInitTexObjLOD(& colorTexture, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
        GXLoadTexObj(& colorTexture, GX_TEXMAP0);
        GXInitTexObj(&alphaTexture, lbl_1_bss_564, (u16) tileSize, (u16) tileSize, GX_TF_IA8,
                     GX_REPEAT, GX_REPEAT, 0);
        GXInitTexObjLOD(& alphaTexture, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
        GXLoadTexObj(& alphaTexture, GX_TEXMAP1);
        GXSetNumTevStages(1);
        GXSetNumTexGens(1);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP,
                      GX_AF_NONE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
        GXSetNumTexGens(2);
        GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, 0, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
        GXSetNumIndStages(1);
        GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
        GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
        GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 1, 0, GX_ITM_0);
        GXPixModeSync();
        {
            for (column = 0; column < 1; column++)
            {
                f32 transform[2][3] = { { 0.0f, 0.5f, 0.0f }, { 0.5f, 0.0f, 0.0f } };
                if (strength > 0.5f)
                {
                    edgeScale = (strength - 0.5f) / 0.5f;
                }
                else
                {
                    edgeScale = 0.0f;
                }
                transform[0][1] *= (0.5f * strength) + ((f32) column * edgeScale / 8.0f);
                if (strength > 0.5f)
                {
                    edgeScale2 = (strength - 0.5f) / 0.5f;
                }
                else
                {
                    edgeScale2 = 0.0f;
                }
                transform[1][0] *= (0.5f * strength) + ((f32) column * edgeScale2 / 8.0f);
                GXSetIndTexMtx(GX_ITM_0, transform, 2);
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                {
                    for (row = 0; row < 4; row++)
                    {
                        f32 screenX[4] = { 0.0f, 1.0f, 1.0f, 0.0f };
                        f32 screenY[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
                        v = ((f32) sourceTop + sourceHeight * screenY[row]) / 480.0f;
                        u = ((f32) sourceLeft + sourceWidth * screenX[row]) / 640.0f;
                        GXWGFifo.f32 = u;
                        GXWGFifo.f32 = v;
                        GXWGFifo.f32 = 0.0f;
                        GXColor4u8(0xFF, 0xFF, 0xFF, (s32)(255.0f / (f32)(column + 1)));
                        textureYValue = screenY[row];
                        screenXValue = screenX[row];
                        GXWGFifo.f32 = screenXValue;
                        GXWGFifo.f32 = textureYValue;
                        GXTexCoord2f32((sourceWidth * (screenX[row] - 0.5f)) /
                                           (96.0f - (48.0f * strength)),
                                       (sourceHeight * (screenY[row] - 0.5f)) /
                                           (96.0f - (48.0f * strength)));
                    }
                }
            }
            GXPixModeSync();
            GXSetNumIndStages(0);
            GXSetTevDirect(GX_TEVSTAGE0);
        }
    }
}

/* Capture the viewport and apply the selected screen effect. */
void fn_1_195DC(s16 unused) {
    f32 copyScale;
    s32 sourceX;
    s32 sourceY;
    s32 sourceWidth;
    s32 sourceHeight;
    GXTexFmt textureFormat;
    u32 bufferSize;
    void *textureBuffer;
    union {
        Mtx44 projection;
        struct {
            f32 projectionRows[3][4];
            Mtx position;
        } positionWorkspace;
    } matrixWorkspace;

    copyScale = lbl_1_bss_554;
    copyScale = copyScale * (f32)(lbl_1_bss_544 + 2) /
                6.0f;
    {
        C_MTXOrtho(matrixWorkspace.projection, 0.0f, 1.0f,
                   0.0f, 1.0f,
                   0.0f, -1.0f);
        GXSetProjection(matrixWorkspace.projection, GX_ORTHOGRAPHIC);
    }
    PSMTXIdentity(matrixWorkspace.positionWorkspace.position);
    GXLoadPosMtxImm(matrixWorkspace.positionWorkspace.position, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_CLAMP, GX_AF_NONE);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZCompLoc(0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
    GXSetCopyFilter(0, RenderMode->sample_pattern, 0,
                    RenderMode->vfilter);
    sourceX = 0x86;
    sourceY = 0x22;
    sourceWidth = 0x174;
    sourceHeight = 0x128;
    sourceX &= 0xFFFE;
    sourceY &= 0xFFFE;
    sourceWidth &= 0xFFFE;
    sourceHeight &= 0xFFFE;
    textureFormat = GX_TF_I8;
    if (lbl_1_bss_534 == 0) {
        sourceX = 0;
        sourceWidth = 0x280;
    }
    GXSetScissor(0, 0, 0x280, sourceY + sourceHeight);
    GXSetViewport(0.0f, 0.0f, 640.0f,
                  480.0f, 0.0f, 1.0f);
    GXSetTexCopySrc(sourceX, sourceY, sourceWidth, sourceHeight);
    GXSetTexCopyDst(sourceWidth, sourceHeight, textureFormat, 0);
    bufferSize = GXGetTexBufferSize(sourceWidth, sourceHeight, textureFormat,
                                    0, 0);
    textureBuffer = lbl_1_bss_568;
    GXCopyTex(textureBuffer, 0);
    GXPixModeSync();
    sourceX = 0x86;
    sourceY = 0x22;
    sourceWidth = 0x174;
    sourceHeight = 0x128;
    sourceX &= 0xFFFE;
    sourceY &= 0xFFFE;
    sourceWidth &= 0xFFFE;
    sourceHeight &= 0xFFFE;
    textureFormat = GX_TF_RGBA8;
    GXSetScissor(0, 0, 0x280, sourceY + sourceHeight);
    switch (lbl_1_bss_534) {
    case 0:
        fn_1_14F00(sourceX, sourceY, sourceWidth, sourceHeight, textureFormat,
                   copyScale);
        break;
    case 1:
        fn_1_15F60(sourceX, sourceY, sourceWidth, sourceHeight, textureFormat,
                   copyScale);
        break;
    case 2:
        fn_1_16804(sourceX, sourceY, sourceWidth, sourceHeight, textureFormat,
                   copyScale);
        break;
    case 3:
        fn_1_17478(sourceX, sourceY, sourceWidth, sourceHeight, textureFormat,
                   copyScale);
        break;
    case 4:
        fn_1_187F0(sourceX, sourceY, sourceWidth, sourceHeight, textureFormat,
                   copyScale);
        break;
    }
    GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, 1,
                    RenderMode->vfilter);
    GXSetZCompLoc(1);
}

/* Enable destination alpha in the framebuffer. */
void fn_1_19AE4(s16 layerNo) {
    GXPixModeSync();
    GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
    GXSetDstAlpha(1U, 0xFFU);
    GXSetAlphaUpdate(1U);
}

/* Draw and capture the screen using the requested effect level. */
void fn_1_19B24(s16 level) {
    GXTexObj texture;
    GXColor drawColor = { 0, 0, 0, 0 };
    s32 left;
    s32 top;
    s32 width;
    s32 height;
    union {
        Mtx44 projection;
        struct {
            f32 projectionRows[3][4];
            Mtx position;
        } positionWorkspace;
    } matrixWorkspace;
    s32 effectAlpha;
    s32 alpha;
    s32 inverseAlpha;
    s32 vertex;
    f32 effect;
    f32 blendAlpha;
    f32 normalizedAlpha;
    f32 blend;
    f32 savedAlpha;
    savedAlpha = lbl_1_bss_554;
    C_MTXOrtho(matrixWorkspace.projection, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f);
    GXSetProjection(matrixWorkspace.projection, GX_ORTHOGRAPHIC);
    PSMTXIdentity(matrixWorkspace.positionWorkspace.position);
    GXLoadPosMtxImm(matrixWorkspace.positionWorkspace.position, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZCompLoc(0);
    GXSetCopyClear(drawColor, 0xFFFFFF);
    GXSetDstAlpha(0, 0);
    GXSetAlphaUpdate(1);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
    left = 0x50;
    top = 0x3C;
    width = 0x1E0;
    height = 0x168;
    GXInitTexObj(& texture, lbl_1_bss_568, width, height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, 0);
    GXInitTexObjLOD(& texture, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    GXLoadTexObj(& texture, GX_TEXMAP0);
    if (lbl_1_bss_384 != 0)
    {
        s32 xCorner[4] = { 0, 1, 1, 0 };
        s32 yCorner[4] = { 0, 0, 1, 1 };
        if (lbl_1_bss_328 > 2)
        {
            if (level == 8)
            {
                effectAlpha = 0xFF;
            }
            else
            {
                effectAlpha = 0;
            }
            effect = (f32) effectAlpha;
        }
        else
        {
            if (savedAlpha < 0.25f) {
                blendAlpha = 0.0f;
            } else {
                if (savedAlpha < 0.75f) {
                    normalizedAlpha = (savedAlpha - 0.25f) / 0.5f;
                } else {
                    normalizedAlpha = 1.0f;
                }
                blendAlpha = normalizedAlpha;
            }
            if (lbl_1_bss_328 == 2) {
                blend = 1.0f - ((f32) lbl_1_bss_32C / 50.0f);
            } else {
                blend = 1.0f;
            }
            effect = 255.0f / ((f32)(level - 7) + (blend * ((f32) lbl_1_data_52C * blendAlpha)));
        }
        alpha = (s32) effect;
        inverseAlpha = 0xFF - alpha;
        if (alpha == 0) {
            return;
        }
            GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
            GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
            GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_SRCALPHA, GX_LO_NOOP);
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            for (vertex = 0; vertex < 4; vertex++)
            {
                f32 overlayScreenX;
                f32 overlayScreenY;

                overlayScreenY = (f32)(height * yCorner[vertex] + top) / 480.0f;
                overlayScreenX = (f32)(width * xCorner[vertex] + left) / 640.0f;
                GXWGFifo.f32 = overlayScreenX;
                GXWGFifo.f32 = overlayScreenY;
                GXWGFifo.f32 = 0.0f;
                GXWGFifo.u8 = 0xFF;
                GXWGFifo.u8 = 0xFF;
                GXWGFifo.u8 = 0xFF;
                GXWGFifo.u8 = (u8) alpha;
                GXWGFifo.f32 = 0.0f;
                GXWGFifo.f32 = 0.0f;
            }
            GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASA, GX_CC_ZERO);
            GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
            GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
            GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP);
            GXBegin(GX_QUADS, GX_VTXFMT0, 4);
            for (vertex = 0; vertex < 4; vertex++)
            {
                f32 texturedScreenX;
                f32 texturedScreenY;
                f32 textureX;
                f32 textureY;

                texturedScreenY = (f32)(height * yCorner[vertex] + top) / 480.0f;
                texturedScreenX = (f32)(width * xCorner[vertex] + left) / 640.0f;
                GXWGFifo.f32 = texturedScreenX;
                GXWGFifo.f32 = texturedScreenY;
                GXWGFifo.f32 = 0.0f;
                GXWGFifo.u8 = 0xFF;
                GXWGFifo.u8 = 0xFF;
                GXWGFifo.u8 = 0xFF;
                GXWGFifo.u8 = (u8)inverseAlpha;
                textureY = (f32)yCorner[vertex];
                textureX = (f32)xCorner[vertex];
                GXWGFifo.f32 = textureX;
                GXWGFifo.f32 = textureY;
            }
    }
    lbl_1_bss_384 = 1;
    GXPixModeSync();
    GXSetCopyFilter(0, RenderMode->sample_pattern, 0, RenderMode->vfilter);
    if (lbl_1_bss_328 < 3)
    {
        GXSetZMode(1, GX_ALWAYS, 1);
    }
    else
    {
        GXSetZMode(0, GX_ALWAYS, 0);
    }
    GXSetTexCopySrc(left, top, width, height);
    GXSetTexCopyDst(width, height, GX_TF_RGBA8, 0);
    GXCopyTex(lbl_1_bss_568, 1);
    GXPixModeSync();
    GXSetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, 1, RenderMode->vfilter);
}

/* Draw the captured image as a screen-aligned quad. */
void fn_1_1A550(s16 layerNo) {
    Mtx position;
    GXTexObj texture;

    s32 left;
    s32 top;
    s32 width;
    s32 height;
    s32 vertex;
    f32 screenX;
    f32 screenY;
    f32 textureX;
    f32 textureT;

    {
        Mtx projection;
        C_MTXOrtho(projection, 0.0f, 1.0f,
                   0.0f, 1.0f,
                   0.0f, -1.0f);
        GXSetProjection(projection, GX_ORTHOGRAPHIC);
    }
    PSMTXIdentity(position);
    GXLoadPosMtxImm(position, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_RASA, GX_CA_TEXA, GX_CA_ZERO);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_CLAMP, GX_AF_NONE);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZCompLoc(0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
    left = 0x50;
    top = 0x3C;
    width = 0x1E0;
    height = 0x168;
    GXInitTexObj(&texture, lbl_1_bss_568, width, height,
                 GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, 0);
    GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR, 0.0f,
                    0.0f, 0.0f, 0, 0, GX_ANISO_1);
    GXLoadTexObj(&texture, GX_TEXMAP0);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    for (vertex = 0; vertex < 4; vertex++) {
        s32 xCoordinates[4] = {0, 1, 1, 0};
        s32 yCoordinates[4] = {0, 0, 1, 1};

        screenY = (f32)(height * yCoordinates[vertex] + top) /
                  480.0f;
        screenX = (f32)(width * xCoordinates[vertex] + left) /
                  640.0f;
        GXWGFifo.f32 = screenX;
        GXWGFifo.f32 = screenY;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = 0xFF;
        GXWGFifo.u8 = 0xFF;
        GXWGFifo.u8 = 0xFF;
        GXWGFifo.u8 = 0xFF;
            textureT = (f32)yCoordinates[vertex];
            textureX = (f32)xCoordinates[vertex];
            GXWGFifo.f32 = textureX;
            GXWGFifo.f32 = textureT;
    }
}

/* Draw the screen overlay. */
void fn_1_1AA0C(s16 unused) {
    union {
        Mtx44 projection;
        struct {
            f32 projectionRows[3][4];
            Mtx position;
        } positionWorkspace;
    } matrixWorkspace;
    s32 left = 0x50;
    s32 top = 0x3C;
    s32 width = 0x1E0;
    s32 height = 0x168;
    s32 vertex;

    {
        C_MTXOrtho(matrixWorkspace.projection, 0.0f,
                   1.0f,
                   0.0f, 1.0f,
                   0.0f, -1.0f);
        GXSetProjection(matrixWorkspace.projection, GX_ORTHOGRAPHIC);
    }
    PSMTXIdentity(matrixWorkspace.positionWorkspace.position);
    GXLoadPosMtxImm(matrixWorkspace.positionWorkspace.position, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZCompLoc(0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);

    left = 0x50;
    top = 0x3C;
    width = 0x1E0;
    height = 0x168;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    for (vertex = 0; vertex < 4; vertex++) {

        f32 x;
        f32 y;

        s32 xIndices[4] = {0, 1, 1, 0};
        s32 yIndices[4] = {0, 0, 1, 1};
        y = (f32)(height * yIndices[vertex] + top) /
            480.0f;
        x = (f32)(width * xIndices[vertex] + left) /
            640.0f;
        GXWGFifo.f32 = x;
        GXWGFifo.f32 = y;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = 0x80;
        GXWGFifo.u8 = 0x80;
        GXWGFifo.u8 = 0x80;
        GXWGFifo.u8 = 0xFF;
    }
}

/* Copy the central viewport into the screen texture. */
void fn_1_1ADAC(void) {
    s32 sourceX = 0x50;
    s32 sourceY = 0x3C;
    s32 copyWidth = 0x1E0;
    s32 copyHeight = 0x168;

    GXSetCopyFilter(0, RenderMode->sample_pattern, 0, RenderMode->vfilter);
    if (lbl_1_bss_328 < 3) {
        GXSetZMode(1, GX_ALWAYS, 1);
    } else {
        GXSetZMode(0, GX_ALWAYS, 0);
    }
    GXSetTexCopySrc(sourceX, sourceY, copyWidth, copyHeight);
    GXSetTexCopyDst(copyWidth, copyHeight, GX_CTF_GB8, 0);
    GXCopyTex(lbl_1_bss_564, 1);
    GXPixModeSync();
    GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, 1, RenderMode->vfilter);
}

void fn_1_1AED4(void)
{
}

/* Render the quiz screen effect and its textured overlay. */
void fn_1_1AED8(s16 layerNo) {
    union {
        Mtx44 projection;
        struct {
            f32 projectionRows[3][4];
            Mtx position;
        } positionWorkspace;
    } matrixWorkspace;
    GXTexObj rgbaTexture;
    GXTexObj alphaTexture;
    s32 sourceLeft = 0x50;
    s32 sourceTop = 0x3C;
    s32 sourceWidth = 0x1E0;
    s32 sourceHeight = 0x168;
    f32 effectStrength;
    f32 renderStrength;
    f32 screenX;
    f32 screenY;
    f32 textureS;
    f32 textureT;
    f32 textureS2;
    f32 textureT2;
    s32 vertex;

    if (lbl_1_bss_328 <= 1) {
        effectStrength = lbl_1_bss_554;
    } else {
        effectStrength = 0.0f;
    }
    renderStrength = effectStrength;
    C_MTXOrtho(matrixWorkspace.projection, 0.0f, 1.0f,
               0.0f, 1.0f, 0.0f, -1.0f);
    GXSetProjection(matrixWorkspace.projection, GX_ORTHOGRAPHIC);
    PSMTXIdentity(matrixWorkspace.positionWorkspace.position);
    GXLoadPosMtxImm(matrixWorkspace.positionWorkspace.position, GX_PNMTX0);
    GXSetCopyFilter(0, RenderMode->sample_pattern, 0, RenderMode->vfilter);
    GXSetZMode(0, GX_ALWAYS, 1);
    GXSetTexCopySrc((u16)sourceLeft, (u16)sourceTop,
                    (u16)sourceWidth, (u16)sourceHeight);
    GXSetTexCopyDst((u16)sourceWidth, (u16)sourceHeight, GX_TF_RGBA8, 0);
    GXCopyTex(lbl_1_bss_568, 0);
    GXPixModeSync();
    GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern,
                    1, RenderMode->vfilter);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_TEX1, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_RGBA6, 0);
    GXInvalidateTexAll();
    GXInitTexObj(&alphaTexture, lbl_1_bss_568,
                 (u16)sourceWidth, (u16)sourceHeight,
                 GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, 0);
    GXInitTexObjLOD(&alphaTexture, GX_LINEAR, GX_LINEAR,
                    0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    GXLoadTexObj(&alphaTexture, GX_TEXMAP0);
    GXInitTexObj(&rgbaTexture, lbl_1_bss_564,
                 (u16)sourceWidth, (u16)sourceHeight,
                 GX_TF_IA8, GX_CLAMP, GX_CLAMP, 0);
    GXInitTexObjLOD(&rgbaTexture, GX_LINEAR, GX_LINEAR,
                    0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    GXLoadTexObj(&rgbaTexture, GX_TEXMAP1);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                      GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA,
                   GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC,
                    GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_RASA);
    {
        f32 firstMatrix[2][3] = {{0.0f, 0.5f, 0.0f}, {0.5f, 0.0f, 0.0f}};

    firstMatrix[0][1] *= (0.375f - (0.2f * renderStrength)) *
                             lbl_1_data_530;
    firstMatrix[1][0] *= (0.375f - (0.2f * renderStrength)) *
                             lbl_1_data_530;
    firstMatrix[1][0] *= -1.0f;
    GXSetNumTexGens(2);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1,
                      GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
    GXSetNumIndStages(1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetIndTexMtx(GX_ITM_0, firstMatrix, 1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 1, 0, GX_ITM_0);
    }
    GXPixModeSync();
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    for (vertex = 0; vertex < 4; vertex++) {
        f32 xCorners[4] = {0.0f, 1.0f, 1.0f, 0.0f};
        f32 yCorners[4] = {0.0f, 0.0f, 1.0f, 1.0f};
        screenY = ((f32)sourceTop + (sourceHeight * yCorners[vertex])) / 480.0f;
        screenX = ((f32)sourceLeft + (sourceWidth * xCorners[vertex])) / 640.0f;
        GXWGFifo.f32 = screenX;
        GXWGFifo.f32 = screenY;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = 0xFF;
        GXWGFifo.u8 = 0xFF;
        GXWGFifo.u8 = 0xFF;
        GXWGFifo.u8 = (s32)(255.0f - (128.0f * renderStrength));
        textureT = yCorners[vertex];
        textureS = xCorners[vertex];
        GXWGFifo.f32 = textureS;
        GXWGFifo.f32 = textureT;
        textureT2 = yCorners[vertex];
        textureS2 = xCorners[vertex];
        GXWGFifo.f32 = textureS2;
        GXWGFifo.f32 = textureT2;
    }
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_TEX1, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_RGBA6, 0);
    GXInvalidateTexAll();
    GXInitTexObj(&alphaTexture, lbl_1_bss_568,
                 (u16)sourceWidth, (u16)sourceHeight,
                 GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, 0);
    GXInitTexObjLOD(&alphaTexture, GX_LINEAR, GX_LINEAR,
                    0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    GXLoadTexObj(&alphaTexture, GX_TEXMAP0);
    HuSprTexLoad(lbl_1_bss_590, 0, 0, 0, 0, 1);
    GXInitTexObj(&rgbaTexture, lbl_1_bss_564,
                 (u16)sourceWidth, (u16)sourceHeight,
                 GX_TF_IA8, GX_CLAMP, GX_CLAMP, 0);
    GXInitTexObjLOD(&rgbaTexture, GX_LINEAR, GX_LINEAR,
                    0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    GXLoadTexObj(&rgbaTexture, GX_TEXMAP1);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                      GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA,
                   GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC,
                    GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_RASA);
    GXPixModeSync();
    {
        f32 secondMatrix[2][3] = {{0.0f, 0.5f, 0.0f}, {0.5f, 0.0f, 0.0f}};

    secondMatrix[0][1] *= 0.5f;
    secondMatrix[1][0] *= 0.5f;
    secondMatrix[0][1] *= -1.0f;
    GXSetNumTexGens(2);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1,
                      GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0A0);
    GXSetNumIndStages(1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetIndTexMtx(GX_ITM_0, secondMatrix, 1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, 1, 0, GX_ITM_0);
    }
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC,
                    GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_RASA);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    for (vertex = 0; vertex < 4; vertex++) {
        f32 finalXCorners[4] = {0.0f, 1.0f, 1.0f, 0.0f};
        f32 finalYCorners[4] = {0.0f, 0.0f, 1.0f, 1.0f};
        f32 finalScreenX;
        f32 finalScreenY;
        f32 textureX;
        f32 textureY;
        finalScreenY = ((f32)sourceTop + (sourceHeight * finalYCorners[vertex])) / 480.0f;
        finalScreenX = ((f32)sourceLeft + (sourceWidth * finalXCorners[vertex])) / 640.0f;
        GXWGFifo.f32 = finalScreenX;
        GXWGFifo.f32 = finalScreenY;
        GXWGFifo.f32 = 0.0f;
        GXWGFifo.u8 = 0xFF;
        GXWGFifo.u8 = 0xFF;
        GXWGFifo.u8 = 0xFF;
        GXWGFifo.u8 = (s32)(255.0f - (192.0f * renderStrength));
        GXWGFifo.f32 = 0.5f;
        GXWGFifo.f32 = 0.5f;
        textureY = finalYCorners[vertex];
        textureX = finalXCorners[vertex];
        GXWGFifo.f32 = textureX;
        GXWGFifo.f32 = textureY;
    }
    GXPixModeSync();
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
}

/* Draw the hook geometry with its supplied position matrix. */
void fn_1_1BD68(void *hookContext, f32 (*positionMatrix)[4]) {

    f32 horizontalValue;
    f32 verticalValue;
    f32 depthValue;
    s32 vertex;

    GXLoadPosMtxImm(positionMatrix, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetZMode(1, GX_LEQUAL, 1);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZCompLoc(1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    GXSetColorUpdate(0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 8);
    for (vertex = 0; vertex < 8; vertex++) {
        f32 horizontal[8] = {-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f};
        f32 vertical[8] = {2.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f};
        f32 depth[8] = {-1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        depthValue = 1000.0f * depth[vertex];
        verticalValue = 1000.0f * vertical[vertex];
        horizontalValue = 1000.0f * horizontal[vertex];
        GXWGFifo.f32 = horizontalValue;
        GXWGFifo.f32 = verticalValue;
        GXWGFifo.f32 = depthValue;
    }
    GXSetCullMode(GX_CULL_BACK);
    GXSetColorUpdate(1);
}

/* Draw the model effect and capture the resulting viewport. */
void fn_1_1C088(HU3D_MODEL *model, f32 (*positionMatrix)[4]) {
    s32 sourceLeft = 0x50;
    s32 sourceTop = 0x3C;
    s32 sourceWidth = 0x1E0;
    s32 sourceHeight = 0x168;
    f32 identity[3][4];
    Mtx projection;
    GXTexObj texture;

    s32 vertex;
    s32 frame;

    s32 halfWidth;
    s32 halfHeight;
    s32 copyWidth;
    s32 copyHeight;
    s32 layerAlpha;
    halfWidth = sourceLeft + (sourceWidth / 4);
    halfHeight = sourceHeight / 4;
    copyWidth = sourceWidth / 2;
    copyHeight = sourceHeight / 2;
    GXSetScissor(sourceLeft, sourceTop, sourceWidth, sourceHeight);
    GXLoadPosMtxImm(positionMatrix, GX_PNMTX0);
    GXSetCopyFilter(0, RenderMode->sample_pattern, 0, RenderMode->vfilter);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZCompLoc(1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetColorUpdate(1);
    GXSetZMode(1, GX_LEQUAL, 0);
    GXSetCullMode(GX_CULL_NONE);
    for (vertex = 0; vertex < 2; vertex += 1)
    {
        layerAlpha = (vertex == 0) ? 0xFF : 0;
        {
            f32 xPosition[8] = { -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f };
            f32 zPosition[8] = { 2.0f, 2.0f, 1.0f, 1.0f, 1.0f, 1.0f, - 1.0f, - 1.0f };
            f32 yPosition[8] = { -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
            GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 8);
            for (frame = 0; frame < 8; frame++)
            {
                GXPosition3f32(1000.0f * xPosition[frame],
                    (1000.0f * zPosition[frame]) + (f32)((vertex == 0 ? 1 : -1) * 4),
                    (1000.0f * yPosition[frame]) + (f32)((vertex == 0 ? 1 : -1) * 4));
                GXColor4u8(layerAlpha, layerAlpha, layerAlpha, 0xFF);
            }
        }
    }
    GXSetCullMode(GX_CULL_BACK);
    GXSetZMode(1, GX_LEQUAL, 1);
    GXSetTexCopySrc(sourceLeft, sourceTop, sourceWidth, sourceHeight);
    GXSetTexCopyDst(sourceWidth, sourceHeight, GX_CTF_R8, 0);
    GXCopyTex(lbl_1_bss_55C[0], 0);
    GXSetTexCopyDst((u16)(sourceWidth / 2), (u16)(sourceHeight / 2), GX_CTF_R8, 1);
    GXCopyTex(lbl_1_bss_55C[1], 1);
    GXPixModeSync();
    C_MTXOrtho(projection, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, - 1.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    PSMTXIdentity(identity);
    GXLoadPosMtxImm(identity, GX_PNMTX0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
    GXSetZMode(0, GX_LEQUAL, 0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC, GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetScissor((u32) halfWidth, (u32) halfHeight, (u32)copyWidth, (u32)copyHeight);
    for (frame = 0; frame < 9; frame++)
    {
        s32 xFrameOffsets[9] = { 0, 1, 2, 4, -8, 0, 0, 0, 0 };
        s32 yFrameOffsets[9] = { 0, 0, 0, 0, 0, 1, 2, 4, -8 };
        s32 cornerX[4] = { 0, 1, 1, 0 };
        s32 cornerY[4] = { 0, 0, 1, 1 };
        GXInvalidateTexAll();
        GXInitTexObj(&texture, lbl_1_bss_55C[1], copyWidth, copyHeight, GX_TF_I8, GX_CLAMP,
                     GX_CLAMP, 0);
        GXInitTexObjLOD(& texture, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
        GXLoadTexObj(& texture, GX_TEXMAP0);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        for (vertex = 0; vertex < 4; vertex++)
        {
            GXPosition3f32(
                ((f32) xFrameOffsets[frame] + (f32) (halfWidth + copyWidth * cornerX[vertex])) /
                    640.0f,
                ((f32) yFrameOffsets[frame] + (f32) (halfHeight + copyHeight * cornerY[vertex])) /
                    480.0f,
                0.0f);
            GXColor4u8(0xFF, 0xFF, 0xFF, frame == 0 ? 0xFF : 0x80);
            GXTexCoord2f32((f32)cornerX[vertex], (f32)cornerY[vertex]);
        }
        GXSetTexCopySrc((u16) halfWidth, (u16) halfHeight, (u16)copyWidth, (u16)copyHeight);
        GXSetTexCopyDst(copyWidth, copyHeight, GX_CTF_R8, 0);
        GXCopyTex(lbl_1_bss_55C[1], 0);
    }
    GXSetCopyFilter(RenderMode->aa, RenderMode->sample_pattern, 1, RenderMode->vfilter);
}

/* Draw the captured screen texture. */
void fn_1_1CB64(s16 layerNo) {
    union {
        Mtx44 projection;
        struct {
            f32 projectionRows[3][4];
            Mtx position;
        } positionWorkspace;
    } matrixWorkspace;
    GXTexObj texture;
    s32 vertex;
    s32 left;
    s32 top;
    s32 width;
    s32 height;
    f32 screenX;
    f32 screenY;
    f32 textureS;
    f32 textureT;
    f32 finalScreenX;
    f32 finalScreenY;
    f32 finalTextureS;
    f32 finalTextureT;

    C_MTXOrtho(matrixWorkspace.projection, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f);
    GXSetProjection(matrixWorkspace.projection, GX_ORTHOGRAPHIC);
    PSMTXIdentity(matrixWorkspace.positionWorkspace.position);
    GXLoadPosMtxImm(matrixWorkspace.positionWorkspace.position, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4,
                      GX_TG_TEX0, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0,
                  GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_RASC,
                    GX_CC_TEXC, GX_CC_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_TEXA);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZCompLoc(0);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA,
                   GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS,
                    GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0,
                    GX_TEX_ST, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0,
                    GX_TEX_ST, GX_RGBA6, 0);
    left = 0x50;
    top = 0x3C;
    width = 0x1E0;
    height = 0x168;
    GXInitTexObj(&texture, lbl_1_bss_55C[0],
                 width, height, GX_TF_I8,
                 GX_CLAMP, GX_CLAMP, 0);
    GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR,
                    0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    GXLoadTexObj(&texture, GX_TEXMAP0);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    for (vertex = 0; vertex < 4; vertex++) {
        s32 screenCorners[4] = {0, 1, 1, 0};
        s32 textureCorners[4] = {0, 0, 1, 1};
        MicquizOverlayRGB colors[11] = {
            {0xFF, 0, 0}, {0x40, 0xFF, 0x40}, {0xFF, 0xC0, 0xE0},
            {0, 0xFF, 0}, {0xC0, 0, 0xC0}, {0xFF, 0xFF, 0},
            {0, 0, 0}, {0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF},
            {0xFF, 0xFF, 0}, {0xFF, 0x50, 0xA0}
        };
        s32 colorIndex;

        colorIndex = lbl_1_bss_540 >= 0xB ? 9 : lbl_1_bss_540;
        screenY = (f32)((height * textureCorners[vertex]) + top) /
                  480.0f;
        screenX = (f32)((width * screenCorners[vertex]) + left) /
                  640.0f;
        GXWGFifo.f32 = screenX;
        GXWGFifo.f32 = screenY;
        GXWGFifo.f32 = 0.0f;
        GXColor4u8(colors[colorIndex].red,
                   colors[colorIndex].green,
                   colors[colorIndex].blue, 0xFF);
        textureT = (f32)textureCorners[vertex];
        textureS = (f32)screenCorners[vertex];
        GXWGFifo.f32 = textureS;
        GXWGFifo.f32 = textureT;
    }
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO,
                    GX_CS_SCALE_2, 1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE,
                    GX_CC_RASC, GX_CC_TEXC);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                    GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO, GX_CA_RASA);
    GXSetBlendMode((lbl_1_bss_540 == 6) ? GX_BM_SUBTRACT : GX_BM_BLEND,
                   GX_BL_ONE, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXInitTexObj(&texture, lbl_1_bss_55C[1],
                 width / 2, height / 2, GX_TF_I8,
                 GX_CLAMP, GX_CLAMP, 0);
    GXInitTexObjLOD(&texture, GX_LINEAR, GX_LINEAR,
                    0.0f, 0.0f, 0.0f, 0, 0, GX_ANISO_1);
    GXLoadTexObj(&texture, GX_TEXMAP0);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    for (vertex = 0; vertex < 4; vertex++) {
        s32 finalScreenCorners[4] = {0, 1, 1, 0};
        s32 finalTextureCorners[4] = {0, 0, 1, 1};
        MicquizOverlayRGB finalColors[11] = {
            {0, 128, 128}, {48, 0, 24}, {0, 48, 8}, {255, 0, 255},
            {8, 56, 0}, {0, 16, 64}, {16, 0, 32}, {0, 18, 18},
            {22, 22, 22}, {16, 0, 64}, {0, 48, 20}
        };
        s32 colorIndex;

        colorIndex = lbl_1_bss_540 >= 0xB ? 9 : lbl_1_bss_540;
        finalScreenY = (f32)((height * finalTextureCorners[vertex]) + top) /
                       480.0f;
        finalScreenX = (f32)((width * finalScreenCorners[vertex]) + left) /
                       640.0f;
        GXWGFifo.f32 = finalScreenX;
        GXWGFifo.f32 = finalScreenY;
        GXWGFifo.f32 = 0.0f;
        GXColor4u8(finalColors[colorIndex].red,
                   finalColors[colorIndex].green,
                   finalColors[colorIndex].blue, 0);
        finalTextureT = (f32)finalTextureCorners[vertex];
        finalTextureS = (f32)finalScreenCorners[vertex];
        GXWGFifo.f32 = finalTextureS;
        GXWGFifo.f32 = finalTextureT;
    }
    fn_8002BF48();
}

void fn_1_1D488(void) {
    MicQuizResponseRun(3);
}

/* Report model transforms, motion values, and animation state when enabled. */
static inline void DebugModelReport(OMOBJ *object, HU3D_MODEL **model, s32 enabled) {
    s32 index;
    s32 state;
    if (enabled) {
        OSReport("%f,%f,%f", (*model)->pos.x, (*model)->pos.y, (*model)->pos.z);
        state = lbl_1_bss_588;
        if (state < 0 || state > 7) {
            OSReport("%f,%f,%f", (f64)state, (f64)object->work[0], (f64)enabled);
        } else if (state == 7) {
            OSReport("%f", (f64)state);
        } else if (state == 0) {
            OSReport("%f", 0.0);
        } else {
            OSReport("%f", (f64)state);
        }
        for (index = 0; index < 4; index++) {
            OSReport("%f,%f,%f", (f64) lbl_802C01C8[lbl_1_bss_570[index]].pos.x,
                     (f64) lbl_802C01C8[lbl_1_bss_570[index]].pos.y,
                     (f64) lbl_802C01C8[lbl_1_bss_570[index]].pos.z);
            OSReport("%f,%f,%f", (f64) lbl_802C01C8[lbl_1_bss_570[index]].rot.x,
                     (f64) lbl_802C01C8[lbl_1_bss_570[index]].rot.y,
                     (f64) lbl_802C01C8[lbl_1_bss_570[index]].rot.z);
            OSReport("%f,%f,%f", (f64) lbl_802C01C8[lbl_1_bss_570[index]].scale.x,
                     (f64) lbl_802C01C8[lbl_1_bss_570[index]].scale.y,
                     (f64) lbl_802C01C8[lbl_1_bss_570[index]].scale.z);
            OSReport("%f,%f,%f", (f64) lbl_802C01C8[lbl_1_bss_570[index]].motWork.time,
                     (f64) lbl_802C01C8[lbl_1_bss_570[index]].motWork.speed,
                     (f64) lbl_802C01C8[lbl_1_bss_570[index]].motWork.start);
            OSReport("%f,%f,%f", (f64) lbl_802C01C8[lbl_1_bss_570[index]].motWork.end,
                     (f64) lbl_802C01C8[lbl_1_bss_570[index]].attr,
                     (f64) lbl_802C01C8[lbl_1_bss_570[index]].motAttr);
            OSReport("%f", (f64)lbl_802C01C8[lbl_1_bss_570[index]].layerNo);
        }
    }
}

const Point3d lbl_1_rodata_A20 = { 0.0f, 75.0f, 22.0f };

/* Advance model animation, quiz transitions, and queued sound effects. */
void fn_1_1D4AC(OMOBJ *object) {
    Mtx rotationMatrix;
    Point3d modelOffset;
    Point3d rotatedOffset;
    Point3d baseOffset;
    s32 motionId;
    f32 previousZRotation;
    s32 soundHandle;
    HU3D_MODEL *model;
    HU3D_MODEL *sourceModel;
    HU3D_MODEL *movingModel;
    HU3D_MODEL *parentModel;
    f32 modelScale;
    f32 rotationScale;
    f32 rotationDecay;
    f32 fadeScale;
    f32 angleStep;
    s32 childIndex;
    s32 i;
    s32 modelIndex;

    if ((lbl_1_bss_584 == 0) && (lbl_1_bss_538 != 0) && (lbl_1_data_53A != -1)) {
        parentModel = &lbl_802C01C8[lbl_1_data_53A];
        if (lbl_1_bss_328 <= 1) {
            lbl_1_bss_388 += 0.75f;
            lbl_1_bss_388 -= (f32)(lbl_1_bss_388 >= 360.0f ? 0x168 : 0);
        } else if (lbl_1_bss_328 >= 2) {
            if (lbl_1_bss_388 == 0.0f) {
                angleStep = 0.0f;
            } else {
                angleStep = 2.25f * (f32)((lbl_1_bss_388 >= 180.0f) ? 1 : -1);
            }
            lbl_1_bss_388 += angleStep;
            lbl_1_bss_388 =
                ((lbl_1_bss_388 >= 360.0f) || (lbl_1_bss_388 < 0.0f)) ? 0.0f : lbl_1_bss_388;
        }
        parentModel->rot.y = lbl_1_bss_388;
        if (lbl_1_bss_538 == 2) {
            modelOffset = lbl_1_rodata_A20;
            if ((lbl_1_bss_540 == 4) || (lbl_1_bss_540 == 6)) {
                modelOffset.z = 12.0f;
            }
            PSMTXRotRad(rotationMatrix, 0x79, MTXDegToRad(lbl_1_bss_388));
            PSMTXMultVec(rotationMatrix, &modelOffset, &modelOffset);
                childIndex = 0;
                {
            for (; childIndex < 6; childIndex += 1) {
                Point3d childOffsets[6] = { { 84.0f, 40.0f, 98.0f },  { 84.0f, 40.0f, 6.0f },
                                            { 84.0f, 40.0f, -82.0f }, { -84.0f, 40.0f, 98.0f },
                                            { -84.0f, 40.0f, 6.0f },  { -84.0f, 40.0f, -82.0f } };
                PSMTXMultVec(rotationMatrix, &childOffsets[childIndex], &childOffsets[childIndex]);
                fn_80026760(
                    lbl_1_bss_516[childIndex],
                    parentModel->pos.x + (parentModel->scale.x * childOffsets[childIndex].x),
                    parentModel->pos.y + (parentModel->scale.y * childOffsets[childIndex].y),
                    parentModel->pos.z + (parentModel->scale.z * childOffsets[childIndex].z));
                fn_80026808(lbl_1_bss_516[childIndex], 0.0f,
                            lbl_1_bss_388 + (f32) (childIndex >= 3 ? 0xB4 : 0), 0.0f);
            }
                }
                fn_80026760(lbl_1_bss_24,
                            parentModel->pos.x + (modelOffset.x * parentModel->scale.x),
                            parentModel->pos.y + (modelOffset.y * parentModel->scale.y),
                            parentModel->pos.z + (modelOffset.z * parentModel->scale.z));
                fn_80026808(lbl_1_bss_24, 0.0f, lbl_1_bss_388, 0.0f);
        }
    }
    if (lbl_1_bss_584 == 0) {
        switch (lbl_1_bss_328) {              /* switch 1; irregular */
        case 0:                                     /* switch 1 */
            lbl_1_bss_32C += 1;
            break;
        case 1:
        case 2:
            lbl_1_bss_32C += 1;
            if (lbl_1_bss_328 == 2) {

                if (lbl_1_bss_32C == (lbl_1_bss_53C != 0 ? 0x1E : 1)) {
                    lbl_1_bss_328 = 3;
                    lbl_1_bss_32C = 0;
                    fn_8002C2AC(0);
                    if (lbl_1_bss_53C != 0) {
                        fn_8002CCC0(lbl_1_bss_24, lbl_1_bss_57E, 0.0f, 15.0f, 0x40000001U);
                    }
                    fn_80026A44(lbl_1_data_536, 0x40000002U);
                    if ((lbl_1_bss_53C != 0) && ((s32) lbl_1_bss_414[6].pendingCount < 3)) {
                        lbl_1_bss_414[6].pendingCount += 1;
                    }
                }
            }
            break;
        case 3:                                     /* switch 1 */
            lbl_1_bss_32C += 1;
            if (lbl_1_bss_32C == 1) {
                ASSERT(FALSE);
            }
            if (((lbl_1_bss_53C == 0) && (lbl_1_bss_538 == 0)) ||
                ((lbl_1_bss_53C != 0) && (lbl_1_bss_32C >= 0x14)) ||
                ((lbl_1_bss_538 != 0) && (lbl_1_bss_388 == 0.0f))) {
                lbl_1_bss_328 = 4;
                lbl_1_bss_32C = 0;
            }
            break;
        case 4:                                     /* switch 1 */
            lbl_1_bss_32C += 1;
            if (((s32) *lbl_1_bss_33C == 0) || ((lbl_1_bss_538 != 0) && (lbl_1_bss_32C >= 0x3C))) {
                lbl_1_bss_328 = 5;
                lbl_1_bss_32C = 0;
                break;
            }
            if (lbl_1_bss_538 == 0) {
                if (lbl_1_bss_32C == 0xA) {
                    if (lbl_1_bss_328 == 3) {
                        lbl_1_bss_328 = 4;
                    }
                    fn_8002CCC0(lbl_1_bss_24, lbl_1_bss_578, 0.0f, 7.0f, 0U);
                    CharFXPlay((s16) lbl_1_bss_540, 0x243);
                }
                if (lbl_1_bss_32C == 0x46) {
                    fn_8002CCC0(lbl_1_bss_24, lbl_1_bss_57E, 0.0f, 20.0f, 0x40000001U);
                }
                if (lbl_1_bss_32C == 0x50) {
                    lbl_1_bss_328 = 5;
                    lbl_1_bss_32C = 0;
                }
            }
            break;
        case 5:                                     /* switch 1 */
            lbl_1_bss_32C += 1;
            break;
        }
    }
    if (lbl_1_bss_584 == 1) {
        f32 modelHeights[14] = {
            150.0f, 170.0f, 172.0f, 150.0f, 154.0f, 164.0f, 210.0f, 120.0f,
            150.0f, 115.0f, 120.0f, 115.0f, 115.0f, 115.0f
        };
        f32 scaleHeights[14] = {
            150.0f, 170.0f, 188.0f, 155.0f, 160.0f, 180.0f, 190.0f, 145.0f,
            170.0f, 160.0f, 145.0f, 160.0f, 160.0f, 160.0f
        };
        Mtx transformMatrix;
        Mtx axisMatrix;
        rotationScale = lbl_1_bss_554;
        rotationScale = (rotationScale * (f32)(lbl_1_bss_544 + 2)) / 6.0f;
        lbl_1_data_750 = ((lbl_1_bss_328 == 0) && (lbl_1_data_750 == -128.0f)) ?
            (f32)((lbl_1_bss_540 == 9 ? 0xA : 0) + ((s32)rand8() % 16)) : lbl_1_data_750;
        switch (lbl_1_bss_328) {
        case 0:
        case 1:
        case 2:
                        lbl_1_bss_32C += 1;
                        if ((lbl_1_bss_328 == 2) && (lbl_1_bss_32C == 1)) {

                            motionId =
                                (s32) ((s32) *lbl_1_bss_33C != 0 ? lbl_1_bss_57E : lbl_1_bss_57A);
                            fn_8002CCC0(lbl_1_bss_24, (s16) motionId, 0.0f, 50.0f, 0x40000001U);
                            for (modelIndex = 0; modelIndex < lbl_1_data_52C; modelIndex += 1) {
                                fn_8002CCC0(lbl_1_bss_570[modelIndex], (s16) motionId, 0.0f, 50.0f,
                                            0x40000001U);
                            }
                        }
                        for (modelIndex = 0; modelIndex < (s32) (lbl_1_data_52C + 1);
                             modelIndex += 1) {
                            static const Point3d lbl_1_rodata_AE4 = { 0.0f, -60.0f, 0.0f };

                            static const Point3d lbl_1_rodata_AF0 = { 0.0f, -60.0f, 0.0f };

                            model = &lbl_802C01C8[(modelIndex == lbl_1_data_52C
                                                       ? lbl_1_bss_24
                                                       : lbl_1_bss_570[modelIndex])];
                            rotatedOffset = lbl_1_rodata_AE4;
                            baseOffset = lbl_1_rodata_AF0;
                            modelScale = 1.125f * (150.0f / scaleHeights[lbl_1_bss_540]);
                            rotationDecay = lbl_1_bss_328 == 2 ? (f32)lbl_1_bss_32C / 50.0f : 1.0f;
                            previousZRotation = lbl_1_data_744.z;
                            rotationDecay = 1.0f - (0.5f * rotationDecay);
                            model->scale.z = modelScale;
                            model->scale.y = modelScale;
                            model->scale.x = modelScale;
                            rotatedOffset.y = baseOffset.y =
                                modelScale * (-modelHeights[lbl_1_bss_540] / 2.0f);

                            rotatedOffset.z = baseOffset.z = (f32)(lbl_1_bss_540 == 9 ? 0xA : 0);
                            if (lbl_1_bss_328 <= 1) {
                                lbl_1_data_744.x += (0.0f / (f32) (lbl_1_data_52C + 1));
                                lbl_1_data_744.y +=
                                    ((111.0f * rotationScale) / (f32) (lbl_1_data_52C + 1));
                                lbl_1_data_744.z +=
                                    ((83.0f * rotationScale) / (f32) (lbl_1_data_52C + 1));
                                if ((lbl_1_bss_554 > 0.04f) &&
                                    ((s32) ((s32) (10.0f + previousZRotation) / 360) !=
                                     (s32) ((s32) (10.0f + lbl_1_data_744.z) / 360)) &&
                                    (((s32) *lbl_1_bss_38C != 1) || (lbl_1_bss_588 == 5) ||
                                     (lbl_1_bss_588 == 2))) {
                                    lbl_1_bss_414[5].soundHandle = HuAudFXPlay(0x51D);
                                }
                                if (((s32) lbl_1_bss_414[5].soundHandle != -1) &&
                                    (HuAudFXStatusGet(lbl_1_bss_414[5].soundHandle) == 2)) {
                                    HuAudFXPitchSet(
                                        lbl_1_bss_414[5].soundHandle,
                                        (s32) (-1250.0f * (1.0f - rotationScale)));
                                }
                            } else if ((lbl_1_bss_328 == 2) && (modelIndex == 0)) {
                                while ((lbl_1_data_744.x < -180.0f) ||
                                       (lbl_1_data_744.x >= 180.0f)) {
                                    lbl_1_data_744.x += (f32) (lbl_1_data_744.x < -180.0f  ? 0x168
                                                               : lbl_1_data_744.x > 180.0f ? -0x168
                                                                                           : 0);
                                }
                                while ((lbl_1_data_744.y < -180.0f) ||
                                       (lbl_1_data_744.y >= 180.0f)) {
                                    lbl_1_data_744.y += (f32) (lbl_1_data_744.y < -180.0f  ? 0x168
                                                               : lbl_1_data_744.y > 180.0f ? -0x168
                                                                                           : 0);
                                }
                                while ((lbl_1_data_744.z < -180.0f) ||
                                       (lbl_1_data_744.z >= 180.0f)) {
                                    lbl_1_data_744.z += (f32) (lbl_1_data_744.z < -180.0f  ? 0x168
                                                               : lbl_1_data_744.z > 180.0f ? -0x168
                                                                                           : 0);
                                }
                                lbl_1_data_744.x *= rotationDecay;
                                lbl_1_data_744.y *= rotationDecay;
                                lbl_1_data_744.z *= rotationDecay;
                                lbl_1_data_750 *= rotationDecay;
                            }
                            PSMTXIdentity(transformMatrix);
                            PSMTXRotRad(axisMatrix, 0x78, MTXDegToRad(lbl_1_data_744.x));
                            PSMTXConcat(axisMatrix, transformMatrix, transformMatrix);
                            PSMTXRotRad(axisMatrix, 0x79, MTXDegToRad(lbl_1_data_744.y));
                            PSMTXConcat(axisMatrix, transformMatrix, transformMatrix);
                            PSMTXRotRad(axisMatrix, 0x7A, MTXDegToRad(lbl_1_data_744.z));
                            PSMTXConcat(axisMatrix, transformMatrix, transformMatrix);
                            PSMTXMultVec(transformMatrix, &rotatedOffset, &rotatedOffset);
                            model->rot.x = lbl_1_data_744.x;
                            model->rot.y = lbl_1_data_744.y;
                            model->rot.z = lbl_1_data_744.z;
                            model->pos.x = rotatedOffset.x;
                            model->pos.y = lbl_1_data_750 + (rotatedOffset.y - baseOffset.y);
                            model->pos.z = 90.0f + (rotatedOffset.z - baseOffset.z);
                        }
                        if ((lbl_1_bss_328 == 2) && ((f32) lbl_1_bss_32C >= 50.0f)) {
                            lbl_1_bss_328 = 3;
                            lbl_1_bss_32C = 0;
                            for (modelIndex = 0; modelIndex < lbl_1_data_52C; modelIndex += 1) {
                                fn_800269E8(lbl_1_bss_570[modelIndex], 1U);
                            }
                            fn_80026A44(lbl_1_bss_570[0], 1U);
                            fn_1_129A8(lbl_1_bss_24, 0);
                            if (lbl_1_bss_534 == 0) {
                                for (modelIndex = 0; modelIndex < (s32) (lbl_1_data_52C + 1);
                                     modelIndex += 1) {
                                    fn_800258A8(1, (s16) modelIndex);
                                    fn_800258A8(1, (s16) (modelIndex + 8));
                                }
                                fn_8002583C(1, 0, fn_1_19AE4);
                                fn_8002583C(1, 8, fn_1_19B24);
                                fn_800258A8(1, 0xE);
                                fn_8002583C(1, 0xD, fn_1_1A550);
                            }
                            fn_80026B8C(lbl_1_bss_570[0], 0);
                            fn_80026B8C(lbl_1_data_534, 4);
                            fn_8002C2AC(5);
                            fn_80026B8C(lbl_1_bss_24, 6);
                            fn_80026B8C(lbl_1_bss_56E, 5);

                            fn_80026808(lbl_1_bss_56E,
                                        (f32) ((s32) *lbl_1_bss_33C != 0 ? -8 : 0x14), 8.0f, 0.0f);

                            fn_80026808(lbl_1_bss_56C,
                                        (f32) ((s32) *lbl_1_bss_33C != 0 ? -8 : 0x14), 8.0f, 0.0f);
                            fn_8002583C(1, 0xE, fn_1_1CB64);
                            fn_80026A44(lbl_1_bss_56E, 1U);
                            fn_80026A44(lbl_1_bss_56C, 1U);
                            sourceModel = &lbl_802C01C8[lbl_1_bss_24];
                            fn_80026760(lbl_1_bss_570[0], sourceModel->pos.x, sourceModel->pos.y,
                                        sourceModel->pos.z);
                            fn_80026760(lbl_1_bss_56E, sourceModel->pos.x, sourceModel->pos.y,
                                        200.0f + sourceModel->pos.z);
                            fn_80026760(lbl_1_bss_56C, sourceModel->pos.x, sourceModel->pos.y,
                                        200.0f + sourceModel->pos.z);
                        }
            break;
        case 3:
        case 4:
                    lbl_1_bss_32C += 1;
                    movingModel = &lbl_802C01C8[lbl_1_bss_24];
                    if ((s32) *lbl_1_bss_33C != 0) {
                        fn_80026760(lbl_1_bss_56E, movingModel->pos.x, movingModel->pos.y,
                                    movingModel->pos.z - (1.25f * (f32) (lbl_1_bss_32C - 0x4B)));
                        fn_80026760(lbl_1_bss_56C, movingModel->pos.x, movingModel->pos.y,
                                    movingModel->pos.z - (1.25f * (f32) (lbl_1_bss_32C - 0x4B)));
                    } else {
                        fn_80026760(lbl_1_bss_56E, movingModel->pos.x,
                                    (30.0f + (movingModel->pos.y - 1000.0f)) -
                                        (2.5f * (f32) (lbl_1_bss_32C - 0x4B)),
                                    movingModel->pos.z - 200.0f);
                        fn_80026760(lbl_1_bss_56C, movingModel->pos.x,
                                    (30.0f + (movingModel->pos.y - 1000.0f)) -
                                        (2.5f * (f32) (lbl_1_bss_32C - 0x4B)),
                                    movingModel->pos.z - 200.0f);
                    }
                    if ((lbl_1_bss_32C == 0xA) && ((s32) lbl_1_bss_414[7].pendingCount < 3)) {
                        lbl_1_bss_414[7].pendingCount += 1;
                    }
                    if ((lbl_1_bss_32C >= 0x19) && (lbl_1_bss_32C <= 0x7D)) {
                        fn_80026E44(lbl_1_bss_24);
                        Hu3DShadowTPLvlSet((0.3f * (f32) (lbl_1_bss_32C - 0x19)) / 100.0f);
                    }
                    if ((lbl_1_bss_32C == 0x64) && (lbl_1_bss_328 == 3)) {
                        lbl_1_bss_328 = 4;
                        if ((s32) *lbl_1_bss_33C == 0) {
                            lbl_1_bss_32C = 0xC8;
                        } else {
                            fn_8002CCC0(lbl_1_bss_24, lbl_1_bss_578, 0.0f, 7.0f, 0U);
                            fn_8002CCC0(lbl_1_bss_570[0], lbl_1_bss_578, 0.0f, 7.0f, 0U);
                            CharFXPlay((s16) lbl_1_bss_540, 0x243);
                        }
                    }
                    if (lbl_1_bss_32C == 0xA0) {
                        fn_8002CCC0(lbl_1_bss_24, lbl_1_bss_57E, 0.0f, 20.0f, 0x40000001U);
                        fn_8002CCC0(lbl_1_bss_570[0], lbl_1_bss_57E, 0.0f, 20.0f, 0x40000001U);
                    }
                    if (lbl_1_bss_32C >= 0xA0) {
                        lbl_1_bss_328 = 5;
                        fn_1_129A8(lbl_1_bss_570[0], 0);
                        fn_800269E8(lbl_1_bss_570[0], 1U);
                        fn_800269E8(lbl_1_bss_56E, 1U);
                        fn_800269E8(lbl_1_bss_56C, 1U);
                        fn_800258A8(1, 0xE);
                        if (lbl_1_bss_534 == 1) {
                            fn_800258A8(1, 0);
                            fn_800258A8(1, 8);
                            fn_800258A8(1, 4);
                            fn_800258A8(1, 0xC);
                        }
                    }
            break;
        case 5:
            lbl_1_bss_32C += 1;
            break;
        }
    }
    *lbl_1_bss_38C = MicQuizCountdownUpdate();
    if ((lbl_1_bss_584 == 0) && ((s32) lbl_1_bss_414[lbl_1_bss_534].soundHandle != -1)) {
        lbl_1_data_754[0] += ((*lbl_1_bss_38C == 1) && (lbl_1_bss_588 != 5) &&
                              (lbl_1_bss_588 != 4) && (lbl_1_bss_588 != 2))
                                 ? -0x30
                                 : 0xC;
        lbl_1_data_754[0] = lbl_1_data_754[0] < 0      ? 0
                            : lbl_1_data_754[0] > 0x7F ? 0x7F
                                                       : lbl_1_data_754[0];
        HuAudFXVolSet(lbl_1_bss_414[lbl_1_bss_534].soundHandle, (s16) lbl_1_data_754[0]);
    }
    if (lbl_1_bss_588 < 5) {
        fn_1_12A84(MicQuizTimeLeftGet());
        if ((MicQuizTimeLeftGet() == 0.0f)
            && ((s32) lbl_1_bss_414[lbl_1_bss_534].soundHandle != -1)) {
            HuAudFXStop(lbl_1_bss_414[lbl_1_bss_534].soundHandle);
        }
    }

        switch (lbl_1_bss_588) {                    /* switch 2 */
        case 0:                                     /* switch 2 */
            if (MicQuizPresenterLayoutGet() == 0) {
                MicQuizWinOpen(0);
                MicQuizWinMesSet(0x1E0000, 1, 0);
            } else {
                MicQuizWinOpen(3);
                MicQuizWinMesSet(0x1E0028, 1, 3);
            }
            MicQuizWinMesWait();
            lbl_1_bss_514 = GameMesCreate(2, 0);
            lbl_1_bss_588 = 1;
            break;
        case 1:                                     /* switch 2 */
            if (GameMesStatGet(lbl_1_bss_514) == 0) {
                lbl_1_bss_588 = 2;
                MicQuizQuestionReadySet();
                lbl_1_bss_58C = 0.0f;
                if ((lbl_1_bss_584 == 0) && ((s32) lbl_1_bss_414[lbl_1_bss_534].pendingCount < 3)) {
                    lbl_1_bss_414[lbl_1_bss_534].pendingCount += 1;
                }
            }
            break;
        case 2:                                     /* switch 2 */
            lbl_1_bss_58C += 1.0f;
            i = (s32) (4.0f * lbl_1_bss_58C);
            fn_80026760(lbl_1_bss_524, (f32) -i, 0.0f, 0.0f);
            fn_80026760(lbl_1_bss_522, (f32) i, 0.0f, 0.0f);
            if (lbl_1_bss_330 != 0) {
                i = 0x91;
            }
            if (i >= 0x91) {
                fn_80026760(lbl_1_bss_524, -145.0f, 0.0f, 0.0f);
                fn_80026760(lbl_1_bss_522, 145.0f, 0.0f, 0.0f);
                lbl_1_bss_588 = 3;
                if (lbl_1_bss_584 == 0) {
                    lbl_1_bss_328 = 1;
                    lbl_1_bss_32C = 0;
                    lbl_1_bss_58C = 0.0f;
                }
            }
            break;
        case 3:                                     /* switch 2 */
            if ((MicQuizResponseStateGet() == 3) || (MicQuizResponseStateGet() == 4)) {
                lbl_1_bss_588 = 5;
                *lbl_1_bss_33C = MicQuizResponseStateGet() == 3 ? 1 : 0;
                lbl_1_bss_58C = 0.0f;
            } else if (MicQuizResponseStateGet() == 2) {

                lbl_1_bss_550 = (lbl_1_bss_584 == 0 ? 0x12C : 0x78);
                lbl_1_bss_588 = 4;
                lbl_1_bss_58C = 0.0f;
            }
            break;
        case 4:                                     /* switch 2 */
            lbl_1_bss_58C += 1.0f;
            lbl_1_bss_550 -= lbl_1_bss_550 > 0;
            if (lbl_1_bss_550 <= 0) {
                lbl_1_bss_588 = 3;
                lbl_1_bss_58C = 0.0f;
                lbl_1_bss_550 = 0;
                MicQuizListenStart();
            }
            break;
        case 5:                                     /* switch 2 */
            fadeScale = MicQuizTimeLeftGet();
            fadeScale -= lbl_1_bss_58C / 120.0f;
            fadeScale = fadeScale < 0.0f ? 0.0f : fadeScale;
            fn_1_12A84(fadeScale);
            if ((fadeScale <= 0.0f) && ((s32) lbl_1_bss_414[lbl_1_bss_534].soundHandle != -1)) {
                HuAudFXStop(lbl_1_bss_414[lbl_1_bss_534].soundHandle);
            }
            if ((lbl_1_bss_328 < 2) && (fadeScale <= 0.0f)) {
                lbl_1_bss_328 = 2;
                lbl_1_bss_32C = 0;
            }
            if ((lbl_1_bss_58C >= (f32) (((lbl_1_bss_584 == 1    ? 0
                                           : *lbl_1_bss_33C != 0 ? -2
                                                                 : 1) *
                                          0x1E) +
                                         0x3C)) &&
                (lbl_1_bss_328 > 3) && (fadeScale <= 0.0f)) {
                lbl_1_bss_588 = 6;
                lbl_1_bss_58C = 0.0f;
            } else {
                lbl_1_bss_58C += 1.0f;
            }
            break;
        case 6:                                     /* switch 2 */
            fn_1_12A84(0.0f);
            if ((s32) lbl_1_bss_414[lbl_1_bss_534].soundHandle != -1) {
                HuAudFXStop(lbl_1_bss_414[lbl_1_bss_534].soundHandle);
            }
            if ((lbl_1_bss_58C >= 60.0f) && (lbl_1_bss_328 > 4)) {
                lbl_1_bss_588 = 7;
                if (lbl_1_data_4BC != -1) {
                    HuAudSStreamFadeOut(lbl_1_data_4BC, 0x1F4);
                    lbl_1_data_4BC = -1;
                }
                WipeCreate(2, 0, 0x14);
                lbl_1_bss_58C = 0.0f;
            } else {
                lbl_1_bss_58C += (f32) (lbl_1_bss_328 == 5);
            }
            break;
        case 7:                                     /* switch 2 */
            if (WipeCheck() == 0) {
                HuMCSelWinContextKill();
                HuMCClose();
                fn_8003A5E4(1, 1);
            }
            break;
        }

    for (i = 0; i < 8; i += 1) {
        s32 soundIds[8] = {1303, 1304, 1305, 1306, 1307, 1309, 1310, 1312};
        if ((s32) lbl_1_bss_414[i].pendingCount > 1) {
            lbl_1_bss_414[i].pendingCount = 1;
        }
        if ((s32) lbl_1_bss_414[i].pendingCount > 0) {
            if ((s32) soundIds[i] != -1) {
                soundHandle = HuAudFXPlay(soundIds[i]);
                lbl_1_bss_414[i].soundHandle = soundHandle;
            }
            lbl_1_bss_414[i].pendingCount -= 1;
        }
    }
    DebugModelReport(object, &model, 0);
}

/* Request a microphone choice and return its quiz selection. */
s32 fn_1_2086C(s32 selectionCount) {
    s32 choice;
    s32 result;

    HuMCSelWinItemRandSet(MESSNUM(MESS_QUIZ_CHARA, 1), 11, (s16)lbl_1_bss_540, 11,
                          (s16)selectionCount);
    HuMCSelWinContextSet(lbl_1_bss_580, 0, (u8)selectionCount);
    HuMCSelWinMaxTimeSet(30.0f);
    HuMCSelWinChoiceGet();
    lbl_1_bss_1C.response = (MicquizResponse *)HuMCCurResponseGet();
    HuMCSelWinContextKill();

    if (lbl_1_bss_1C.response->result == 0) {
        choice = lbl_1_bss_1C.response->recognizedWord;
    } else if (lbl_1_bss_1C.response->result == -1) {
        OSReport("HUMC_RESULT_EMPTY\n");
    } else if (lbl_1_bss_1C.response->result == -2) {
        OSReport("HUMC_RESULT_ERROR\n");
    } else if (lbl_1_bss_1C.response->result == -3) {
        MicQuizMicHudStateSet(0);
        return -1;
    }
    result = fn_1_760(choice - 1, lbl_1_bss_540);
    return result;
}
