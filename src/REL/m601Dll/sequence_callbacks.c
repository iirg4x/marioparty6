/* This unit has no math calls; the shared inline-math constants remain
 * owned by the original math translation unit. */
#include "game/object.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "REL/m601Dll.h"

extern HUPROCESS *lbl_1_bss_150;
extern OMOBJ *lbl_1_bss_168;
extern OMOBJ *lbl_1_bss_16C[5];
extern OMOBJ *lbl_1_bss_180;
extern s16 lbl_1_bss_14C;
extern s32 lbl_1_data_14;

void fn_1_A2AC(s16 mode, s16 frameNo)
{
    s32 i;
    OMOBJ *obj;

    lbl_1_bss_168 = fn_8003AB38(lbl_1_bss_150, 100, 106U, 2U, 1, fn_1_8798);
    i = 0;
    while (i < 4) {
        obj = lbl_1_bss_16C[i] = fn_8003AB38(lbl_1_bss_150, 102, 2U, 3U, 0, fn_1_9574);
        obj->data = HuMemDirectMallocNum(HEAP_HEAP, 140, 0x10000000U);
        obj->work[0] = (u32)i;
        i += 1;
    }
    lbl_1_bss_180 = fn_8003AB38(lbl_1_bss_150, 103, 0U, 0U, -1, fn_1_9F6C);
    fn_80071A9C();
}

void fn_1_A3B0(s16 mode, s16 frameNo)
{
    void *work;
    s32 i;

    fn_80071A9C();
    i = 0;
    while (i < 4) {
        work = lbl_1_bss_16C[i]->data;
        i += 1;
    }
    if (lbl_1_bss_14C >= 0) {
        GameMesKill(lbl_1_bss_14C);
    }
    GameMesClose();
    HuAudAllStop();
    fn_8003A5E4((s16)lbl_1_data_14, 1);
}
