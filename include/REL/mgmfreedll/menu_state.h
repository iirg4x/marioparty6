#ifndef MGMFREE_MENU_STATE_H
#define MGMFREE_MENU_STATE_H

#include <game/object.h>
#include <game/sprite.h>

typedef struct {
    OMOBJ *object;
    u8 unk04[4]; /* No code names this field. */
} MGMFREE_ITEM_WINDOW;
void fn_1_4ECC(OMOBJ *obj);
s32 fn_1_A3FC(void);
s32 fn_1_AB00(void);
s16 fn_1_B75C(void);
s16 fn_1_D324(void);
s32 fn_1_E0C0(s16 mode);
f32 fn_1_E264(f32 start, f32 end, f32 time, f32 duration);
f32 fn_1_E35C(f32 start, f32 end, f32 time, f32 duration);
f32 fn_1_E3A0(f32 start, f32 end, f32 time);
f32 fn_1_E3E8(f32 start, f32 end, f32 time, f32 duration);
f32 fn_1_E4D0(f32 start, f32 end, f32 time, f32 duration);

void fn_1_E5AC(HUSPR_GROUPID groupId, s32 attr);
void fn_1_E62C(HUSPR_GROUPID groupId, s32 attr);

#endif
