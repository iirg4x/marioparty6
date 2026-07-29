#include "game/gamework.h"

typedef struct omObj_s OMOBJ;

void fn_1_F4(void);
void fn_1_268(OMOBJ *obj);
void mbObjectSetup(s32 boardNo, void (*init)(void), void (*close)(OMOBJ *));

void fn_1_A0(void)
{
    GwSystem.partyF = FALSE;
    mbObjectSetup(7, fn_1_F4, fn_1_268);
}
