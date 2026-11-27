/* Object slots used for players, scenery, and queued scene effects. */
#ifndef REL_M626DLL_OBJECTS_H
#define REL_M626DLL_OBJECTS_H

#include "game/object.h"

extern OMOBJ *lbl_1_bss_2F4[4]; /* Player actor objects. */
extern OMOBJ *lbl_1_bss_448[30]; /* Snowball objects available to gameplay. */
extern OMOBJ *lbl_1_bss_4C0[128]; /* Scenery objects indexed by type and ID. */

struct M626ObjectWork_s;
struct M626Entry;

void fn_1_42DC(HUPROCESS *process);
void fn_1_44D4(void);
void fn_1_44F8(OMOBJ *object);
void fn_1_4548(OMOBJ *object);
void fn_1_4CE4(struct M626ObjectWork_s *work);
void fn_1_4EA8(OMOBJ *object);
s32 fn_1_57A8(struct M626ObjectWork_s *work);
s32 fn_1_5C20(struct M626ObjectWork_s *work);
s32 fn_1_5E88(struct M626ObjectWork_s *state);
void fn_1_6120(struct M626ObjectWork_s *state);
s32 fn_1_7050(struct M626Entry *trigger, struct M626Entry *contact);
void fn_1_8570(struct M626ObjectWork_s *state);
void fn_1_85A4(struct M626ObjectWork_s *state, s32 model);
s32 fn_1_8B58(s32 index);
s32 fn_1_8B90(s32 playerIndex);

#endif
