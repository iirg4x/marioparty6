#ifndef M659_OBJECTS_H
#define M659_OBJECTS_H
#include "REL/m659/collision.h"
typedef struct M659OMRecord {
    HU3D_MODELID model;
    HuVecF pos;
    HuVecF velocity;
    HuVecF rot;
    float rotStepX, rotStepY;
    unsigned char unobserved_30[4];
    M659Collision collision;
    int active;
    float tpLevel;
} M659OMRecord;
typedef struct M659OMRow {
    M659OMRecord *slots[7];
    s16 count;
    u16 mask;
} M659OMRow;
typedef struct M659OMData {
    M659OMRow rows[22];
    M659OMRecord records[154];
    int recordCount;
    s16 mode;
    float phase;
    s16 rowStateA, rowStateB;
} M659OMData;
extern OMOBJ *lbl_1_bss_4C;
extern float lbl_1_data_70[8];
s16 fn_1_33B0(void);
void fn_1_3898(M659OMRecord *record);
void fn_1_38E8(M659OMRecord *record);
void fn_1_39A8(OMOBJ *obj);
void fn_1_4094(OMOBJ *obj);
int fn_1_1840(s16 player);
#endif
