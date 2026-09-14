#include "REL/m657Dll.h"
#include "game/memory.h"
#include "game/frand.h"
#include "string.h"

typedef struct {
    int delay;
    int duration;
    int variation;
} M657ComTiming;

static M657ComTiming lbl_1_data_1F8[4] = {
    { 30, 60, 17 }, { 37, 30, 17 }, { 41, 24, 7 }, { 58, 30, 10 }
};

OMOBJ *lbl_1_bss_40;

void fn_1_4570(OMOBJMAN *objman)
{
    OMOBJ *obj;
    M657ComWork *work;

    obj = lbl_1_bss_40 = omAddObjEx(objman, 70, 0, 0, -1, fn_1_4608);
    obj->stat |= OM_STAT_MODELPAUSE;
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP,
        sizeof(M657ComWork), HU_MEMNUM_OVL);
    memset(work, 0, sizeof(M657ComWork));
}

void fn_1_4608(OMOBJ *obj)
{
    M657ComWork *work = obj->data;

    work->state = 0;
    work->timer = 0;
    obj->objFunc = fn_1_463C;
}

void fn_1_463C(OMOBJ *obj)
{
    M657ComWork *work = obj->data;
    M657ComPlayer **com = NULL;
    int i = 0;

    if (MgSeqModeGet() > MGSEQ_MODE_MAIN) {
        work->state = 0;
        work->timer = 0;
        obj->objFunc = fn_1_47A8;
    }
    switch (work->state) {
        case 0:
            if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
                work->state++;
            }
            break;
        case 1:
            work->state++;
            break;
        case 2:
            for (i = 0; i < 2; i++) {
                com = &work->players[i];
                if ((*com)->enabled && fn_1_4910(*com)) {
                    fn_1_4A80(*com);
                }
            }
            work->state++;
            break;
        case 3:
            for (i = 0; i < 2; i++) {
                com = &work->players[i];
                if ((*com)->enabled) {
                    fn_1_4964(*com);
                    if (fn_1_4910(*com)) {
                        work->state = 2;
                    }
                }
            }
            break;
    }
}

void fn_1_47A8(OMOBJ *obj)
{
    M657ComWork *work = obj->data;
    int i = 0;

    obj->objFunc = fn_1_47D0;
}

void fn_1_47D0(OMOBJ *obj)
{
    M657ComWork *work = obj->data;
    int i = 0;
}

void fn_1_47EC(OMOBJ *obj)
{
    M657ComPlayer **com;
    M657Player *player = obj->data;
    M657ComWork *work = lbl_1_bss_40->data;

    com = &work->players[player->player.team];
    if (work->state == 0) {
        if (*com != NULL) {
            OSReport("Error: M657ComPlayerSet() - !*com != NULL\n");
        }
        *com = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M657ComPlayer), HU_MEMNUM_OVL);
        if (*com) {
            (*com)->obj = obj;
            (*com)->enabled = player->computer;
        } else {
            OSReport("Error: M657ComPlayerSet() - !omMalloc()\n");
        }
    }
}

s32 fn_1_48C0(void)
{
    M657ComWork *work = lbl_1_bss_40->data;
    return work->unk_10;
}

s32 fn_1_48E8(void)
{
    M657ComWork *work = lbl_1_bss_40->data;
    return work->unk_14;
}

s32 fn_1_4910(M657ComPlayer *com)
{
    M657ComWork *work = lbl_1_bss_40->data;
    M657Player *player;

    if (com->enabled) {
        player = com->obj->data;
        if (player->inputActive == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

void fn_1_4964(M657ComPlayer *com)
{
    M657Player *player = com->obj->data;

    switch (player->inputState) {
        case 0:
            player->inputTimer = 0;
            player->inputState++;
            /* fall through */
        case 1:
            if (player->inputTimer >= player->inputDelay) {
                player->inputState++;
            }
            break;
        case 2:
            player->inputTimer = 0;
            player->inputState++;
            /* fall through */
        case 3:
            if (player->inputTimer < player->inputDuration) {
                break;
            }
            player->inputState++;
            /* fall through */
        case 4:
            player->inputActive = 0;
            player->inputState = 0;
            player->inputTimer = 0;
            player->inputDelay = 0;
            player->inputDuration = 0;
            break;
    }
    player->inputTimer++;
}

void fn_1_4A48(M657ComPlayer *com)
{
    M657Player *player = com->obj->data;
    player->inputDuration = lbl_1_data_1F8[player->difficulty].duration;
}

void fn_1_4A80(M657ComPlayer *com)
{
    M657Player *player = com->obj->data;

    switch (player->difficulty) {
        case 0:
            fn_1_4B10(com);
            break;
        case 1:
            fn_1_4BA4(com);
            break;
        case 2:
            fn_1_4C78(com);
            break;
        case 3:
            fn_1_4D4C(com);
            break;
    }
}

void fn_1_4B10(M657ComPlayer *com)
{
    M657Player *player = com->obj->data;
    player->inputDelay = frand() % 54;
    player->inputDuration = frand() % 120;
    player->inputActive = 1;
    player->inputState = 0;
}

void fn_1_4BA4(M657ComPlayer *com)
{
    M657Player *player = com->obj->data;
    int variation = frand() % lbl_1_data_1F8[player->difficulty].variation;

    player->inputDelay = frand() % lbl_1_data_1F8[player->difficulty].delay;
    player->inputDuration = variation + frand() % lbl_1_data_1F8[player->difficulty].duration;
    player->inputActive = 1;
    player->inputState = 0;
}

void fn_1_4C78(M657ComPlayer *com)
{
    M657Player *player = com->obj->data;
    int variation = frand() % lbl_1_data_1F8[player->difficulty].variation;

    player->inputDelay = frand() % lbl_1_data_1F8[player->difficulty].delay;
    player->inputDuration = variation + frand() % lbl_1_data_1F8[player->difficulty].duration;
    player->inputActive = 1;
    player->inputState = 0;
}

void fn_1_4D4C(M657ComPlayer *com)
{
    M657Player *player = com->obj->data;
    int variation = frand() % lbl_1_data_1F8[player->difficulty].variation;

    if (frand() % 100 == 0) {
        player->inputDelay = 420;
        player->inputDuration = 600;
    } else {
        player->inputDelay = variation + frand() % lbl_1_data_1F8[player->difficulty].delay;
        player->inputDuration = frand() % lbl_1_data_1F8[player->difficulty].duration;
    }
    player->inputActive = 1;
    player->inputState = 0;
}
