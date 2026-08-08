#include "dolphin.h"

#include "game/board/tutorial.h"
#include "messdir_enum.h"

extern void mbWipeSpecialCreate(s16 mode, s16 type, s16 maxTime);
extern void mbWipeSpecialWait(void);
extern void mbWipeFadeOutTime(s16 maxTime);
extern void mbWipeSpecialKill(void);

void fn_1_1200(void)
{
    mbTutorialCallWait(23);
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 90));
    mbTutorialCallWait(23);
    mbTutorialCallEnd();
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 91));
    mbTutorialWinMesExec(MESSNUM(MESS_BOARD_TUTORIAL, 92));
    mbTutorialExitOnSet(FALSE);
    mbWipeSpecialCreate(1, 6, 120);
    mbWipeSpecialWait();
    mbWipeFadeOutTime(1);
    mbWipeSpecialKill();
}
