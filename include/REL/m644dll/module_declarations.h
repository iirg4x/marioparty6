/* Functions shared by the Rocky Road overlay units. */
#ifndef REL_M644DLL_MODULE_DECLARATIONS_H
#define REL_M644DLL_MODULE_DECLARATIONS_H

#include "REL/m644dll/module_types.h"

/* Stage setup and track queries. */
void fn_1_913C(s32 team_index);
void fn_1_9664(void);
f32 fn_1_9178(s32 team_index);
void fn_1_94A4(s32 index);
s16 fn_1_9114(void);
void fn_1_80D4(s32 team_index, Mtx matrix);
void fn_1_9614(void);
s32 fn_1_9124(s32 team_index);
void fn_1_7FF4(s32 entry_index, s32 slot, Point3d *position);
void fn_1_8064(s32 entry_index, s32 slot, Mtx matrix);
void fn_1_7E5C(s16 model, s32 entry_index, s32 slot);
void fn_1_7F8C(s16 unused_context, s32 entry_index, s32 slot);
s32 fn_1_7D98(void);
void fn_1_9190(void);

/* Player setup and updates. */
void fn_1_559C(s32 group);
void fn_1_56D4(s32 group);
s32 fn_1_519C(s32 player_slot);
void fn_1_50E0(s32 player_slot);
s32 fn_1_5168(s32 player_slot);
void fn_1_5180(s32 player_slot);
void fn_1_5660(s32 group);
void fn_1_21B8(M644PlayerRecord *player);
void fn_1_24D4(M644PlayerRecord *player);
s32 fn_1_4E84(void);
void fn_1_215C(M644PlayerRecord *player);
void fn_1_5444(s32 group);
void fn_1_54F0(s32 group);
void fn_1_51B4(s32 group, s32 phase);
s32 fn_1_5210(s32 group);

/* Overlay object manager. */
OMOBJMAN *fn_1_A0(void);

/* Game setup and menu updates. */
s32 fn_1_224(void);
void fn_1_590(void);
void ObjectSetup(void);
void fn_1_B0(s16 group);

/* Scene setup, queries, and updates. */
void fn_1_1690(void);
void fn_1_1EA8(s32 entry_index, Point3d *position, Mtx matrix);
int fn_1_213C(COL_NARROW_PARAM *contact, COL_NARROW_PARAM *other_contact);
void fn_1_205C(s32 team_index);
void fn_1_1FB4(s32 team_index, s32 route_sequence);
void fn_1_20A0(s32 team_index);

/* Camera, lighting, audio, and resource support. */
void fn_1_1124(s32 team_index);
void fn_1_11F0(void);
void fn_1_1060(OMOBJ *object);
void fn_1_848(void);
void fn_1_690(s32 index, s32 selector);
s32 fn_1_7C8(s32 entry_index);
void fn_1_1510(void);

/* Track routes and motion. */
s32 fn_1_6588(s32 route_motion);
float fn_1_65A4(s32 side, s32 route, s32 variant);
void fn_1_6684(s32 route, s32 variant, Vec *out);
void fn_1_671C(s32 route, s32 variant, Vec *out);
float fn_1_6768(s32 route, s32 variant);
s32 fn_1_67CC(s32 route_motion, s32 lane_index);
void fn_1_6394(OMOBJ *object);
s32 fn_1_6838(void);
void fn_1_7C24(s32 team_index, s32 section_index);

#endif
