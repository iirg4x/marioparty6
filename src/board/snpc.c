#include "game/board/main.h"
#include "game/board/masu.h"

#define SNPC_MAGIC 0x534E5043

typedef struct MBSNPCSAVEWORK {
    u8 flags;
    u8 masuId;
    u8 effectMissCount;
} MBSNPCSAVEWORK;

typedef struct MBSNPCWORK MBSNPCWORK;

static u32 snpcMagic;
static MBSNPCSAVEWORK *snpcSaveWork;
static MBSNPCWORK *snpcWork;

static void SNpcStarFunc(void);

void mbSNpcInit(void)
{
    snpcMagic = 0;
    snpcSaveWork = NULL;
    snpcWork = NULL;
}

int mbSNpcMasuGet(void)
{
    if (snpcMagic != SNPC_MAGIC) {
        return 0;
    }
    return snpcSaveWork->masuId;
}

static void SNpcStarFunc(void)
{
}

void mbMasuChanceKill(void *work)
{
    HuMemDirectFree(work);
}

void mbMasuChanceTypeSet(u8 *chanceTbl, u8 value, int *typeTbl, BOOL inverseF)
{
    int masuNum;
    int masuType;
    BOOL inverseWork;
    int i;
    u8 *chanceTblP;
    int typeNo;

    masuNum = mbMasuNumGet();
    inverseWork = inverseF ? TRUE : FALSE;
    chanceTblP = chanceTbl + 1;
    for (i = 1; i < masuNum; i++, chanceTblP++) {
        if (*chanceTblP == 0) {
            masuType = mbMasuGet(i)->type;
            for (typeNo = 0; typeTbl[typeNo] >= 0; typeNo++) {
                if (masuType == typeTbl[typeNo]) {
                    break;
                }
            }
            if (inverseWork == (typeTbl[typeNo] < 0)) {
                *chanceTblP = (u8)value;
            }
        }
    }
}

void mbMasuChanceFlagSet(u8 *chanceTbl, u8 value, u32 flag, u32 mAttr,
    BOOL inverseF)
{
    u8 *chanceTblP;
    int masuNum;
    BOOL inverseWork;
    int i;
    MASU *masu;

    masuNum = mbMasuNumGet();
    inverseWork = inverseF ? TRUE : FALSE;
    chanceTblP = chanceTbl + 1;
    for (i = 1; i < masuNum; i++, chanceTblP++) {
        if (*chanceTblP == 0) {
            masu = mbMasuGet(i);
            if (inverseWork == (((masu->flag & flag) | (masu->mAttr & mAttr)) == 0)) {
                *chanceTblP = (u8)value;
            }
        }
    }
}

void mbMasuChancePlayerSet(u8 *chanceTbl, int value)
{
    int i;

    for (i = 0; i < GW_PLAYER_MAX; i++) {
        chanceTbl[GwPlayer[i].masuId] = value;
    }
}
