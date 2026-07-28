#include <string.h>

#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"

typedef struct Lbl1Bss1348_s LBL_1_BSS_1348;

struct Lbl1Bss1348_s {
    void *unk_0;
    Vec unk_4;
    Vec unk_10;
    Vec unk_1C;
    Vec unk_28;
    float unk_34;
    float unk_38;
    void *unk_3C;
};

void fn_1_26FC(LBL_1_BSS_1348 *work)
{
    memcpy(&work->unk_4, &work->unk_10, sizeof(Vec));
    memcpy(&work->unk_1C, &work->unk_28, sizeof(Vec));
    work->unk_34 = work->unk_38;
}

void fn_1_274C(LBL_1_BSS_1348 *work)
{
    memcpy(&work->unk_10, &work->unk_4, sizeof(Vec));
    memcpy(&work->unk_28, &work->unk_1C, sizeof(Vec));
    work->unk_38 = work->unk_34;
}
