/* Obstacle placement, scrolling, and model updates for Asteroad Rage. */
#define M659_DATA_OBSTACLE_MODEL_0 DATANUM(DATA_m659, 13)
#define M659_DATA_OBSTACLE_MODEL_1 DATANUM(DATA_m659, 14)
#define M659_DATA_OBSTACLE_MODEL_2 DATANUM(DATA_m659, 15)
#define M659_DATA_OBSTACLE_MODEL_3 DATANUM(DATA_m659, 16)
#define M659_DATA_OBSTACLE_MODEL_4 DATANUM(DATA_m659, 17)
#define M659_DATA_COURSE_OUTER_MODEL DATANUM(DATA_m659, 21)
#define M659_DATA_COURSE_INNER_MODEL DATANUM(DATA_m659, 22)
#include "REL/m659/objects.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/main.h"
#include "game/frand.h"
#include "game/mg/seqman.h"
#include "string.h"

extern HU3D_MODELID lbl_1_bss_3C[8];

/* Creates the obstacle object and allocates its course data. */
void fn_1_329C(OMOBJMAN *manager)
{
    M659OMData *obstacleData;
    OMOBJ *obj;
    obj = omAddObjEx(manager, 30, 5, 1, -1, fn_1_39A8);
    lbl_1_bss_4C = obj;
    obstacleData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M659OMData), HU_MEMNUM_OVL);
    obj->data = obstacleData;
    memset(obstacleData, 0, sizeof(M659OMData));
}

void fn_1_3324(void)
{
}

/* Disables obstacle collision and hides every obstacle model during the result transition. */
void fn_1_3328(void)
{
    M659OMData *obstacleData = lbl_1_bss_4C->data;
    M659OMRecord *record = NULL;
    s16 i;
    for (i = 0; i < obstacleData->recordCount; i++) {
        record = &obstacleData->records[i];
        record->collision.flags = 0;
        Hu3DModelAttrSet(record->model, HU3D_ATTR_DISPOFF);
    }
}

/* Returns the current obstacle row reached by a player. */
s16 fn_1_33B0(void)
{
    M659OMData *obstacleData = lbl_1_bss_4C->data;
    return obstacleData->playerRow;
}

/* Creates the five obstacle models used as visual variants. */
void fn_1_33D8(OMOBJ *obj)
{
    M659OMData *unusedObstacleData = obj->data; /* The model setup does not read this pointer. */
    HU3D_MODELID model = 0;
    s16 unusedZero = 0; /* Initialized to zero but not read by this callback. */
    int i;
    for (i = 0; i < 5; i++) {
        int obstacleModelDataIds[5] = { M659_DATA_OBSTACLE_MODEL_0, M659_DATA_OBSTACLE_MODEL_1,
                                        M659_DATA_OBSTACLE_MODEL_2, M659_DATA_OBSTACLE_MODEL_3,
                                        M659_DATA_OBSTACLE_MODEL_4 };
        model = obj->mdlId[i] = Hu3DModelCreate(
            HuDataSelHeapReadNum(obstacleModelDataIds[i], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelLayerSet(model, 3);
        Hu3DModelCameraSet(model, 3);
        Hu3DModelPosSet(model, -1000, 0, 0);
        Hu3DModelRotSet(model, 0, 0, 0);
        Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    }
}

/* Scrolls obstacle rows toward the players and updates their visibility and fade. */
void fn_1_3510(OMOBJ *obj)
{
    M659OMData *obstacleData = obj->data;
    M659OMRow *row = NULL;
    M659OMRecord *record = NULL;
    s16 i = 0, j = 0;
    for (i = 0; i < 22; i++) {
        row = &obstacleData->rows[i];
        for (j = 0; j < 7; j++) {
            record = row->slots[j];
            if (record == NULL) {
                continue;
            }
            if (record->position.z < -500 && !record->active) {
                continue;
            }
            record->position.z -= 25 + obstacleData->phase / 66;
            if (!record->active) {
                if (record->position.z < 11000) {
                    fn_1_3898(record);
                    obstacleData->spawnRow = i;
                }
            } else if (record->position.z < -500) {
                fn_1_38E8(record);
                obstacleData->playerRow = i + 1;
            } else if (record->transparencyLevel < 1) {
                record->transparencyLevel += 0.01f;
                if (record->transparencyLevel > 1) {
                    record->transparencyLevel = 1;
                }
                Hu3DModelTPLvlSet(record->model, record->transparencyLevel);
            }
        }
    }
    obstacleData->phase += 0.88f;
}

/* Course setup calls this for each row; it fills lanes while avoiding the preceding row when
 * possible. */
void fn_1_36DC(OMOBJ *obj, s16 rowIndex)
{
    M659OMData *obstacleData = obj->data;
    M659OMRow *row = &obstacleData->rows[rowIndex];
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
    /* The first five rows have four obstacles; later rows have five. */
    row->count = 5 - reduced;
    OSReport("parts index : %d ( %d )\n", rowIndex, row->count);
    if (rowIndex - 1 >= 0) {
        previous = &obstacleData->rows[rowIndex - 1];
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
        record = row->slots[column] = &obstacleData->records[obstacleData->recordCount];
        obstacleData->recordCount++;
    }
}

void fn_1_3894(void)
{
}

/* Row scrolling calls this when an obstacle enters play; it enables collision and visibility. */
void fn_1_3898(M659OMRecord *record)
{
    record->collision.flags = 1 << 0;
    record->active = 1;
    Hu3DModelAttrReset(record->model, HU3D_ATTR_DISPOFF);
    Hu3DModelTPLvlSet(record->model, record->transparencyLevel);
}

/* Course setup and scrolling call this to disable collision and hide an obstacle. */
void fn_1_38E8(M659OMRecord *record)
{
    record->collision.flags = 0;
    record->active = 0;
    Hu3DModelAttrSet(record->model, HU3D_ATTR_DISPOFF);
}

/* Course setup calls this for each obstacle to choose a visual model from the weighted table. */
int fn_1_392C(void)
{
    int index = rand8() % 16;
    /* The seventeenth table entry is unreachable because the index is modulo 16. */
    int choice[17] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2};
    return choice[index];
}

/* The obstacle object manager calls this at startup to build the course and install its update
 * callback. */
void fn_1_39A8(OMOBJ *obj)
{
    M659OMData *obstacleData = obj->data;
    M659OMRecord *record = NULL;
    M659Collision *collision;
    M659OMRow *row;
    s16 i, j;
    int k;
    float z;
    int unusedZeroA = 0, unusedZeroB = 0; /* Initialized to zero but not read by course setup. */
    fn_1_33D8(obj);
    memset(obstacleData, 0, sizeof(obstacleData->rows));
    obstacleData->recordCount = 0;
    for (i = 0; i < 22; i++) {
        fn_1_36DC(obj, i);
        for (j = 0; j < 7; j++) {
            row = &obstacleData->rows[i];
            if (!(row->mask & (1 << j))) {
                continue;
            }
            record = row->slots[j];
            record->model = Hu3DModelLink(obj->mdlId[fn_1_392C()]);
            Hu3DModelAttrReset(record->model, HU3D_ATTR_DISPOFF);
            Hu3DModelLayerSet(record->model, 6);
            /* Sets this lane's shared player and route X coordinate to the obstacle's X
             * position. */
            record->position.x = lbl_1_data_70[j] = 1200.0f - 400.0f * j;
            record->position.y = 0;
            record->position.z = (11000 + 3050.0f * i) - (rand8() % 200 - 100) * 4;
            record->rotationSpeedX = 0.1f * (frand() % 10);
            record->rotationSpeedY = 0.1f * (frand() % 10);
            Hu3DModelPosSetV(record->model, &record->position);
            Hu3DModelCameraSet(record->model, 3);
            if (record->position.z > 11000) {
                fn_1_38E8(record);
            }
            collision = &record->collision;
            collision->flags = 1 << 0;
            collision->mask = 3;
            collision->collisionImpulseEnabled = 1;
            collision->unusedCollisionWord = 0;
            collision->collisionDisabled = 0;
            collision->userData = obj;
            collision->update = NULL;
            collision->collide = NULL;
            collision->position = &record->position;
            collision->velocity = &record->velocity;
            collision->previousPosition = record->position;
            collision->radius = 150;
            collision->factor = 1;
            fn_1_4CA4(collision);
        }
    }
    obstacleData->mode = 0;
    lbl_1_bss_3C[0] = Hu3DModelCreate(
        HuDataSelHeapReadNum(M659_DATA_COURSE_OUTER_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_3C[7] = Hu3DModelLink(lbl_1_bss_3C[0]);
    lbl_1_bss_3C[1] = Hu3DModelCreate(
        HuDataSelHeapReadNum(M659_DATA_COURSE_INNER_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
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
        Hu3DModelAttrReset(lbl_1_bss_3C[k], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_3C[k], HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(lbl_1_bss_3C[k], 2);
        Hu3DModelLayerSet(lbl_1_bss_3C[k], 4);
    }
    obj->objFunc = fn_1_4094;
}

/* Runs each frame; it scrolls the course during play and rotates visible obstacles. */
void fn_1_4094(OMOBJ *obj)
{
    M659OMData *obstacleData = obj->data;
    M659OMRecord *record = NULL;
    s16 i;
    switch (obstacleData->mode) {
    case 0:
        obstacleData->phase = 0;
        obstacleData->mode++;
        break;
    case 1:
        if (MgSeqModeGet() > 6) {
            obstacleData->mode++;
        } else if (fn_1_1840(0) == 0 && fn_1_1840(1) == 0) {
            fn_1_3510(obj);
        }
        break;
    case 2:
        if (MgSeqModeGet() == 7) {
            obstacleData->mode++;
        }
        break;
    case 3:
        break;
    }
    for (i = 0; i < obstacleData->recordCount; i++) {
        record = &obstacleData->records[i];
        record->rotation.x += record->rotationSpeedX;
        record->rotation.y += record->rotationSpeedY;
        if (record->active == 1) {
            Hu3DModelPosSetV(record->model, &record->position);
            Hu3DModelRotSetV(record->model, &record->rotation);
        }
    }
}
