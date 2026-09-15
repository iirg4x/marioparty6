#include "REL/m659/objects.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/main.h"
#include "game/frand.h"
#include "game/mg/seqman.h"
#include "string.h"

extern HU3D_MODELID lbl_1_bss_3C[8];

void fn_1_329C(OMOBJMAN *manager)
{
    M659OMData *work;
    OMOBJ *obj;
    obj = omAddObjEx(manager, 30, 5, 1, -1, fn_1_39A8);
    lbl_1_bss_4C = obj;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M659OMData), 268435456);
    obj->data = work;
    memset(work, 0, sizeof(M659OMData));
}

void fn_1_3324(void)
{
}

void fn_1_3328(void)
{
    M659OMData *work = lbl_1_bss_4C->data;
    M659OMRecord *record = NULL;
    s16 i;
    for (i = 0; i < work->recordCount; i++) {
        record = &work->records[i];
        record->collision.flags = 0;
        Hu3DModelAttrSet(record->model, 1);
    }
}

s16 fn_1_33B0(void)
{
    M659OMData *work = lbl_1_bss_4C->data;
    return work->rowStateA;
}

void fn_1_33D8(OMOBJ *obj)
{
    M659OMData *work = obj->data;
    HU3D_MODELID model = 0;
    s16 unused = 0;
    int i;
    for (i = 0; i < 5; i++) {
        int resources[5] = {7733261, 7733262, 7733263, 7733264, 7733265};
        model = obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(resources[i], 268435456, HEAP_MODEL));
        Hu3DModelLayerSet(model, 3);
        Hu3DModelCameraSet(model, 3);
        Hu3DModelPosSet(model, -1000, 0, 0);
        Hu3DModelRotSet(model, 0, 0, 0);
        Hu3DModelAttrSet(model, 1);
    }
}

void fn_1_3510(OMOBJ *obj)
{
    M659OMData *work = obj->data;
    M659OMRow *row = NULL;
    M659OMRecord *record = NULL;
    s16 i = 0, j = 0;
    for (i = 0; i < 22; i++) {
        row = &work->rows[i];
        for (j = 0; j < 7; j++) {
            record = row->slots[j];
            if (record == NULL) {
                continue;
            }
            if (record->pos.z < -500 && !record->active) {
                continue;
            }
            record->pos.z -= 25 + work->phase / 66;
            if (!record->active) {
                if (record->pos.z < 11000) {
                    fn_1_3898(record);
                    work->rowStateB = i;
                }
            } else if (record->pos.z < -500) {
                fn_1_38E8(record);
                work->rowStateA = i + 1;
            } else if (record->tpLevel < 1) {
                record->tpLevel += 0.01f;
                if (record->tpLevel > 1) {
                    record->tpLevel = 1;
                }
                Hu3DModelTPLvlSet(record->model, record->tpLevel);
            }
        }
    }
    work->phase += 0.88f;
}

void fn_1_36DC(OMOBJ *obj, s16 rowIndex)
{
    M659OMData *work = obj->data;
    M659OMRow *row = &work->rows[rowIndex];
    M659OMRow *previous = NULL;
    int reduced;
    M659OMRecord *record = NULL;
    u16 mask = row->mask;
    u16 all = 0;
    u16 bit = 0;
    s16 i;
    s16 column;
    for (i = 0; i < 7; i++) {
        all |= 1 << i;
    }
    if (rowIndex < 5) {
        reduced = 1;
    } else {
        reduced = 0;
    }
    row->count = 5 - reduced;
    OSReport("parts index : %d ( %d )\n", rowIndex, row->count);
    if (rowIndex - 1 >= 0) {
        previous = &work->rows[rowIndex - 1];
        mask |= previous->mask;
    }
    for (i = 0; i < row->count; i++) {
        do {
            if (previous && (mask ^ all) == 0) {
                mask ^= previous->mask;
            }
            column = frand() % 7;
        } while (mask & (1 << column));
        bit = 1 << column;
        mask |= bit;
        row->mask |= bit;
        record = row->slots[column] = &work->records[work->recordCount];
        work->recordCount++;
    }
}

void fn_1_3894(void)
{
}

void fn_1_3898(M659OMRecord *record)
{
    record->collision.flags = 1;
    record->active = 1;
    Hu3DModelAttrReset(record->model, 1);
    Hu3DModelTPLvlSet(record->model, record->tpLevel);
}

void fn_1_38E8(M659OMRecord *record)
{
    record->collision.flags = 0;
    record->active = 0;
    Hu3DModelAttrSet(record->model, 1);
}

int fn_1_392C(void)
{
    int index = rand8() % 16;
    int choice[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2};
    return choice[index];
}

void fn_1_39A8(OMOBJ *obj)
{
    M659OMData *work = obj->data;
    M659OMRecord *record = NULL;
    M659Collision *collision;
    M659OMRow *row;
    s16 i, j;
    int k;
    float z;
    int unusedA = 0, unusedB = 0;
    fn_1_33D8(obj);
    memset(work, 0, sizeof(work->rows));
    work->recordCount = 0;
    for (i = 0; i < 22; i++) {
        fn_1_36DC(obj, i);
        for (j = 0; j < 7; j++) {
            row = &work->rows[i];
            if (!(row->mask & (1 << j))) {
                continue;
            }
            record = row->slots[j];
            record->model = Hu3DModelLink(obj->mdlId[fn_1_392C()]);
            Hu3DModelAttrReset(record->model, 1);
            Hu3DModelLayerSet(record->model, 6);
            record->pos.x = lbl_1_data_70[j] = 1200.0f - 400.0f * j;
            record->pos.y = 0;
            record->pos.z = (11000 + 3050.0f * i) - (rand8() % 200 - 100) * 4;
            record->rotStepX = 0.1f * (frand() % 10);
            record->rotStepY = 0.1f * (frand() % 10);
            Hu3DModelPosSetV(record->model, &record->pos);
            Hu3DModelCameraSet(record->model, 3);
            if (record->pos.z > 11000) {
                fn_1_38E8(record);
            }
            collision = &record->collision;
            collision->flags = 1;
            collision->mask = 3;
            collision->unk_08 = 1;
            collision->unk_0C = 0;
            collision->unk_10 = 0;
            collision->userData = obj;
            collision->update = NULL;
            collision->collide = NULL;
            collision->pos = &record->pos;
            collision->velocity = &record->velocity;
            collision->previous = record->pos;
            collision->radius = 150;
            collision->factor = 1;
            fn_1_4CA4(collision);
        }
    }
    work->mode = 0;
    lbl_1_bss_3C[0] = Hu3DModelCreate(HuDataSelHeapReadNum(7733269, 268435456, HEAP_MODEL));
    lbl_1_bss_3C[7] = Hu3DModelLink(lbl_1_bss_3C[0]);
    lbl_1_bss_3C[1] = Hu3DModelCreate(HuDataSelHeapReadNum(7733270, 268435456, HEAP_MODEL));
    for (k = 2; k < 7; k++) {
        lbl_1_bss_3C[k] = Hu3DModelLink(lbl_1_bss_3C[1]);
    }
    for (k = 0; k < 8; k++) {
        if ((k == 0 || k == 7) != FALSE) {
            z = 0;
        } else {
            z = -3000;
        }
        Hu3DModelPosSet(lbl_1_bss_3C[k], 1400 - 400.0f * k, 0, z);
        Hu3DModelAttrReset(lbl_1_bss_3C[k], 1);
        Hu3DModelAttrSet(lbl_1_bss_3C[k], 1073741825);
        Hu3DMotionSpeedSet(lbl_1_bss_3C[k], 2);
        Hu3DModelLayerSet(lbl_1_bss_3C[k], 4);
    }
    obj->objFunc = fn_1_4094;
}

void fn_1_4094(OMOBJ *obj)
{
    M659OMData *work = obj->data;
    M659OMRecord *record = NULL;
    s16 i;
    switch (work->mode) {
    case 0:
        work->phase = 0;
        work->mode++;
        break;
    case 1:
        if (MgSeqModeGet() > 6) {
            work->mode++;
        } else if (fn_1_1840(0) == 0 && fn_1_1840(1) == 0) {
            fn_1_3510(obj);
        }
        break;
    case 2:
        if (MgSeqModeGet() == 7) {
            work->mode++;
        }
        break;
    case 3:
        break;
    }
    for (i = 0; i < work->recordCount; i++) {
        record = &work->records[i];
        record->rot.x += record->rotStepX;
        record->rot.y += record->rotStepY;
        if (record->active == 1) {
            Hu3DModelPosSetV(record->model, &record->pos);
            Hu3DModelRotSetV(record->model, &record->rot);
        }
    }
}
