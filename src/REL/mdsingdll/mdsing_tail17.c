#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"

typedef struct omObj_s OMOBJ;
typedef struct Lbl1Bss1348_s LBL_1_BSS_1348;
typedef void (*LBL_1_BSS_1348_CALLBACK)(
    OMOBJ *obj, LBL_1_BSS_1348 *work);

struct Lbl1Bss1348_s {
    void *unk_0;
    Vec unk_4;
    Vec unk_10;
    Vec unk_1C;
    Vec unk_28;
    float unk_34;
    float unk_38;
    LBL_1_BSS_1348_CALLBACK callback;
    float unk_40;
    u8 unk_44[0xC];
};

extern LBL_1_BSS_1348 lbl_1_bss_1348;
extern float CZoom;
extern Vec Center;
extern Vec CRot;

void omOutView(OMOBJ *obj);

void fn_1_3170(OMOBJ *obj, LBL_1_BSS_1348 *work)
{
    if (work->callback) {
        work->callback(obj, work);
    }
}

inline void fn_1_3170(OMOBJ *obj, LBL_1_BSS_1348 *work);

void fn_1_31BC(OMOBJ *obj)
{
    LBL_1_BSS_1348 *work = &lbl_1_bss_1348;

    fn_1_3170(obj, work);
    Center.x = work->unk_4.x;
    Center.y = work->unk_4.y;
    Center.z = work->unk_4.z;
    CRot.x = work->unk_1C.x;
    CRot.y = work->unk_1C.y;
    CRot.z = work->unk_1C.z;
    CZoom = work->unk_34;
    omOutView(obj);
}
