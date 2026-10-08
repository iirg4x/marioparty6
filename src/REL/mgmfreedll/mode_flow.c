/* MGMfree's mode transition loop, easing curves, and sprite-group helpers. */
#include <dolphin/math.h>
#include <game/audio.h>
#include <game/object.h>
#include <game/process.h>
#include <game/sprite.h>
#include <game/window.h>

#define MGMFREE_SE_MODE_EXIT_SEQUENCE_FX_1 1218
#define MGMFREE_SE_MODE_EXIT_SEQUENCE_FX_2 1219

extern u8 lbl_1_bss_28[8];
extern HUSPR_GROUPID lbl_1_bss_53E[10];
extern HUWINID lbl_1_bss_9DC[2];

s32 fn_1_A3FC(void);
s32 fn_1_AB00(void);
s16 fn_1_B75C(void);
s16 fn_1_D324(void);
void fn_1_4ECC(OMOBJ *obj);
void fn_1_E5AC(HUSPR_GROUPID groupId, s32 attr);
void fn_1_E62C(HUSPR_GROUPID groupId, s32 attr);

/* Run the main mode loop, dispatching the active menu state until it returns or exits. */
s32 fn_1_E0C0(s16 mode)
{
    s16 index;
    s16 state;
    s16 result;
    OMOBJ *obj;

    result = 0;
    state = 0;
    fn_1_A3FC();
    fn_1_AB00();
    do {
        HuPrcVSleep();
        switch (state) {
        case 0:
            result = fn_1_B75C();
            if (result == 1) {
                state = 2;
            } else if (result == 2) {
                state = 1;
            }
            break;
        case 1:
            result = fn_1_D324();
            if (result == 1) {
                state = 2;
            } else if (result == 0) {
                state = 0;
            }
            break;
        case 2:
            HuAudFXPlay(MSM_SE_CMN_03);
            HuAudFXPlay(MGMFREE_SE_MODE_EXIT_SEQUENCE_FX_1);
            HuAudFXPlay(MGMFREE_SE_MODE_EXIT_SEQUENCE_FX_2);
            obj = (*(OMOBJ **)&lbl_1_bss_28[0]);
            obj->work[0] = 0;
            obj->work[1] = 30;
            fn_1_E5AC(lbl_1_bss_53E[1], HUSPR_ATTR_DISPOFF);
            HuWinDispOff(lbl_1_bss_9DC[0]);
            obj->objFunc = fn_1_4ECC;
            fn_1_E5AC(lbl_1_bss_53E[7], HUSPR_ATTR_DISPOFF);
            for (index = 0; index < 4; index++) {
                fn_1_E5AC(lbl_1_bss_53E[index + 3], HUSPR_ATTR_DISPOFF);
            }
            return 0;
        }
    } while (result != -1);
    return 1;
}

/* Interpolate from start to end with a cosine ease over the requested frame duration. */
f32 fn_1_E264(f32 start, f32 end, f32 time, f32 duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start +
           (end - start) * (1.0 - cos(3.141592653589793 * ((90.0f / duration) * time) / 180.0));
}

/* Interpolate linearly from start to end over the requested frame duration. */
f32 fn_1_E35C(f32 start, f32 end, f32 time, f32 duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}

/* Move a value toward its target by averaging over the remaining frame count. */
f32 fn_1_E3A0(f32 start, f32 end, f32 time)
{
    if (start == end || time <= 1.0f) {
        return end;
    }
    return (start * (time - 1.0f) + end) / time;
}

/* Interpolate from start to end with a sine ease over the requested frame duration. */
f32 fn_1_E3E8(f32 start, f32 end, f32 time, f32 duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + (end - start) * sin(3.141592653589793 * ((90.0f / duration) * time) / 180.0);
}

/* Apply one sine cycle to the value, returning to start at the duration boundary. */
f32 fn_1_E4D0(f32 start, f32 end, f32 time, f32 duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return start;
    }
    return start + (end - start) * sin(3.141592653589793 * ((360.0f / duration) * time) / 180.0);
}

/* Set the requested sprite attribute on every member of a sprite group. */
void fn_1_E5AC(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

/* Clear the requested sprite attribute on every member of a sprite group. */
void fn_1_E62C(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}
