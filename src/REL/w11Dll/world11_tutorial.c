#include "dolphin.h"

#include "game/board/guide.h"
#include "game/board/tutorial.h"

extern s8 lbl_1_data_15[7];
extern void fn_1_984(void);
extern void fn_1_DF0(void);
extern void fn_1_1200(void);

void fn_1_940(void);

void fn_1_918(void)
{
    mbTutorialMainFuncSet(fn_1_940);
}

void fn_1_940(void)
{
    mbTutorialGuideCreate(lbl_1_data_15, TRUE);
    fn_1_DF0();
    fn_1_984();
    fn_1_1200();
    mbTutorialExitSet();
    mbTutorialCallWait(24);
}
