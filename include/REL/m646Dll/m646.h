#ifndef REL_M646DLL_M646_H
#define REL_M646DLL_M646_H

#include "REL/m646Dll/module_types.h"
#include "REL/m646Dll/collision_state.h"

#ifdef __cplusplus
extern "C" {
#endif

struct M646ModelCollider_s;

/* Functions shared by the module's source files. */
OMOBJ *fn_1_23D0(Vec *position, f64 phase, Vec *direction, OMOBJ *player);
f64 fn_1_2968(Vec lhs, Vec rhs);
s32 fn_1_3E80(void *collisionResource);
s32 fn_1_3EF8(void *collisionResource);
s32 fn_1_48A4(const M646CollisionRecord *anchor, const Point3d *position);
s32 fn_1_48DC(struct M646ModelCollider_s *collider, HSF_OBJECT *objects, HU3D_MODELID model, char *name);
s32 fn_1_9024(void);
s32 fn_1_B3C0(HuVecF *worldPosition, s32 soundId);
s32 fn_1_B4BC(s32 soundId);
u16 fn_1_8FD4(void);
void fn_1_2B50(OMOBJMAN *objMan);
void fn_1_436C(M646CollisionRecord *record, HSF_OBJECT *object);
void fn_1_43A0(M646CollisionRecord *record);
void fn_1_4DD0(struct M646ModelCollider_s *collider);
void fn_1_4E28(OMOBJMAN *objman);
void fn_1_543C(OMOBJMAN *manager);
void fn_1_5564(s16 player, s32 state);
void fn_1_5964(s16 player, s32 resetAttribute);
void fn_1_8158(OMOBJMAN *objman);
void fn_1_8980(void);
void fn_1_8E78(OMOBJMAN *objman);
void fn_1_8F04(s32 playerIndex, s32 scorePoints);
void fn_1_8F5C(s32 playerIndex);
void fn_1_9118(void);
void fn_1_98E0(OMOBJMAN *objman);
void fn_1_A028(OMOBJ *obj);
void fn_1_AFC(OMOBJMAN *objman, s16 player);
void fn_1_1E4(s32 cameraMotionIndex);
s32 fn_1_2CC(void);
void fn_1_AF4(s16 mode, s16 frame);
void fn_1_AF8(s16 mode, s16 frame);
void fn_1_55A0(void);
void fn_1_5634(OMOBJ *obj);
s32 fn_1_5694(OMOBJ *obj);
s32 fn_1_57DC(s16 player);
void fn_1_5850(s16 player);
void fn_1_589C(s16 player);
void fn_1_59E0(s16 player, Vec *position);
void fn_1_5A64(OMOBJ *obj, s16 motion);
void fn_1_81FC(s16 index, Vec *out);
s32 fn_1_9380(void);
s32 fn_1_A0D4(void);
s32 fn_1_A0FC(void);

#ifdef __cplusplus
}
#endif

#endif
