#include "REL/m635dll.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/memory.h"
#include "datadir_enum.h"
#include "string.h"

extern float lbl_1_data_A0[2];

void fn_1_4114(void)
{
    s16 model;
    s16 direction;
    float x;
    float rotation;

    memset(&lbl_1_bss_68, 0, sizeof(lbl_1_bss_68));
    direction = frandmod(2);
    lbl_1_bss_68.direction = direction;
    if (lbl_1_bss_4.nightF == 0) {
        model = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m635, 18), HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m635, 19), HU_MEMNUM_OVL, HEAP_MODEL));
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(model, HU3D_CAM0);
    Hu3DModelShadowSet(model);
    if (direction != 0) {
        x = -600.0f;
        rotation = 90.0f;
    } else {
        x = 600.0f;
        rotation = -90.0f;
    }
    Hu3DModelPosSet(model, x, 0.0f, -550.0f);
    Hu3DModelRotSet(model, 0.0f, rotation, 0.0f);
    lbl_1_bss_68.x = x;
    lbl_1_bss_68.model = model;
}

void fn_1_42A0(void)
{
    if (lbl_1_bss_68.timer > 0) {
        lbl_1_bss_68.timer--;
        return;
    }
    if (lbl_1_bss_68.direction != 0) {
        lbl_1_bss_68.x += lbl_1_data_A0[lbl_1_bss_4.nightF];
        if (lbl_1_bss_68.x >= 600.0f) {
            lbl_1_bss_68.timer = 120;
            lbl_1_bss_68.direction = 0;
            Hu3DModelRotSet(lbl_1_bss_68.model, 0.0f, -90.0f, 0.0f);
        }
    } else {
        lbl_1_bss_68.x -= lbl_1_data_A0[lbl_1_bss_4.nightF];
        if (lbl_1_bss_68.x <= -600.0f) {
            lbl_1_bss_68.timer = 120;
            lbl_1_bss_68.direction = 1;
            Hu3DModelRotSet(lbl_1_bss_68.model, 0.0f, 90.0f, 0.0f);
        }
    }
    Hu3DModelPosSet(lbl_1_bss_68.model, lbl_1_bss_68.x, 0.0f, -550.0f);
}
