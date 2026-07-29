#include "dolphin/mtx/GeoTypes.h"

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_ANIMID;
typedef struct AnimData_s ANIMDATA;

typedef struct Lbl1BssE74Entry {
    HU3D_MODELID modelId;
    HU3D_ANIMID animId[4];
    s16 field_A;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
    s16 field_30;
    u8 field_32[6];
} LBL_1_BSS_E74_ENTRY;

extern LBL_1_BSS_E74_ENTRY lbl_1_bss_E74[];
extern ANIMDATA *lbl_1_bss_11F4[];

ANIMDATA *Hu3DAnimAnimSet(HU3D_ANIMID animId, ANIMDATA *animP);
void Hu3DAnimBankSet(HU3D_ANIMID animId, u16 bank);

void fn_1_A3A8(s16 modelNo, s16 animNo, s16 dataNo, s16 bank)
{
    LBL_1_BSS_E74_ENTRY *entry = &lbl_1_bss_E74[modelNo];

    Hu3DAnimAnimSet(entry->animId[animNo], lbl_1_bss_11F4[dataNo]);
    if (bank != -1) {
        Hu3DAnimBankSet(entry->animId[animNo], bank);
    }
}

void fn_1_A450(s16 modelNo, s16 dataNo)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        fn_1_A3A8(modelNo, i, dataNo, -1);
    }
}
