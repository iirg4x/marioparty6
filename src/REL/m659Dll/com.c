#include "REL/m659/com.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "string.h"

OMOBJ *lbl_1_bss_60;
int lbl_1_data_1D8[4] = {5, 3, 2, 2};
void fn_1_59E0(OMOBJ *obj);
void fn_1_5A14(OMOBJ *obj);
void fn_1_5B58(OMOBJ *obj);
void fn_1_5CAC(OMOBJ *obj);
int fn_1_5DB4(M659Com *com);
void fn_1_5E08(M659Com *com);
void fn_1_5E9C(M659Com *com);
void fn_1_64B0(M659Com *com);
#include "REL/m659/com.h"
#include "game/memory.h"
#include "REL/m659/objects.h"
#include "REL/m659/player.h"

extern OMOBJ *lbl_1_bss_60;
#include "REL/m659/com.h"
#include "REL/m659/objects.h"
#include "game/main.h"

extern int lbl_1_data_1D8[4];
int abs(int value);
double fn_1_4ABC(HuVecF a, HuVecF b);

void fn_1_5944(OMOBJMAN *manager)
{
    M659ComManager *work;
    OMOBJ *obj;
    obj = lbl_1_bss_60 = omAddObjEx(manager, 70, 0, 0, -1, fn_1_59E0);
    obj->stat |= 256;
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M659ComManager), 268435456);
    memset(work, 0, sizeof(M659ComManager));
}

void fn_1_59DC(void)
{
}

void fn_1_59E0(OMOBJ *obj)
{
    M659ComManager *work = obj->data;
    work->state = 0;
    work->frame = 0;
    obj->objFunc = fn_1_5A14;
}

void fn_1_5A14(OMOBJ *obj)
{
    M659ComManager *work = obj->data;
    M659Com **com;
    int i;
    switch (work->state) {
    case 0:
        if (MgSeqModeGet() >= 2) {
            work->ready = 0;
            work->state++;
        }
        break;
    case 1:
        work->state++;
        break;
    case 2:
        work->state++;
        break;
    case 3:
        work->state++;
        break;
    case 4:
        work->state++;
        break;
    case 5:
        work->state++;
        break;
    case 6:
        work->ready = 1;
        obj->objFunc = fn_1_5B58;
        work->state = 0;
        work->frame = 0;
        com = NULL;
        for (i = 0; i < 2; i++) {
            com = &work->players[i];
            if ((*com)->isCom) {
                fn_1_5E9C(*com);
            }
        }
        break;
    }
    work->frame++;
}

void fn_1_5B58(OMOBJ *obj)
{
    M659ComManager *work = obj->data;
    M659Com **com = NULL;
    int i = 0;
    switch (work->state) {
    case 0:
        if (MgSeqModeGet() == 5) {
            work->state++;
        }
        work->frame = 0;
        break;
    case 1:
        for (i = 0; i < 2; i++) {
            com = &work->players[i];
            if ((*com)->isCom && fn_1_5DB4(*com)) {
                fn_1_64B0(*com);
            }
        }
        work->state++;
        break;
    case 2:
        for (i = 0; i < 2; i++) {
            com = &work->players[i];
            if ((*com)->isCom) {
                fn_1_5E08(*com);
                if (fn_1_5DB4(*com)) {
                    work->state = 1;
                }
            }
        }
        break;
    }
    work->frame++;
}

void fn_1_5C9C(OMOBJ *obj)
{
    obj->objFunc = fn_1_5CAC;
}

void fn_1_5CAC(OMOBJ *obj)
{
}

void fn_1_5CB0(OMOBJ *obj)
{
    M659Player *player = obj->data;
    M659ComManager *work = lbl_1_bss_60->data;
    M659Com **com = &work->players[player->info.side];
    if (work->state == 0) {
        if (*com != NULL) {
            OSReport("Error: M659ComPlayerSet() - !*com != NULL\n");
        }
        *com = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M659Com), 268435456);
        player->com.routeIndex = -1;
        if (*com) {
            (*com)->obj = obj;
            (*com)->isCom = player->com.isCom;
        } else {
            OSReport("Error: M659ComPlayerSet() - !omMalloc()\n");
        }
    }
}

int fn_1_5D8C(void)
{
    M659ComManager *work = lbl_1_bss_60->data;
    return work->ready;
}

int fn_1_5DB4(M659ComSlot *com)
{
    void *manager = lbl_1_bss_60->data;
    if (com->isCom) {
        M659Player *player = com->obj->data;
        if (player->com.active == 0) {
            return 1;
        }
    }
    return 0;
}

void fn_1_5E08(M659ComSlot *com)
{
    M659Player *player = com->obj->data;
    switch (player->com.state) {
    case 0:
        player->com.state++;
        /* fallthrough */
    case 1:
        player->com.state++;
        break;
    case 2:
        player->com.state++;
        /* fallthrough */
    case 3:
        player->com.state++;
        break;
    case 4:
        player->com.active = 0;
        player->com.state = 0;
        break;
    }
}

void fn_1_5E9C(M659ComSlot *slot)
{
    M659Player *player = slot->obj->data;
    M659ComRecord *com = &player->com;
    M659OMData *board = lbl_1_bss_4C->data;
    M659OMRow *initialRow = &board->rows[board->rowStateA];
    int previousColumn = 3;
    HuVecF start = { 0.0f, 0.0f, 5950.0f };
    if (com->isCom) {
        s16 row;
        for (row = 0; row < 22; row++) {
            M659OMRow *currentRow = &board->rows[row];
            M659ComRoute *route = &com->routes[row];
            int unused = 0;
            int count = 0;
            double distances[3];
            int columns[3];
            HuVecF positions[3];
            s16 column, delay;
            int spread;
            double best;
            int selected, lower, upper, i;
            route->distance = 10000.0;
            distances[0] = 100000.0;
            distances[1] = 100000.0;
            distances[2] = 100000.0;
            columns[0] = 20;
            columns[1] = 20;
            columns[2] = 20;
            for (column = 0; column < 7; column++) {
                HuVecF *previous = NULL;
                HuVecF pos = { 0.0f, 0.0f, 0.0f };
                pos.x = lbl_1_data_70[column];
                pos.z = 9000.0f + 3050.0f * row;
                if (currentRow->slots[column] == NULL) {
                    if (row == 0) {
                        previous = &start;
                    } else {
                        previous = &com->routes[row - 1].pos;
                    }
                    best = fn_1_4ABC(pos, *previous);
                    distances[count] = best;
                    columns[count] = column;
                    positions[count] = pos;
                    count++;
                }
            }
            switch (com->difficulty) {
            case 0:
                spread = 3;
                delay = rand8() % lbl_1_data_1D8[0] + 1;
                break;
            case 1:
                spread = 2;
                delay = rand8() % lbl_1_data_1D8[1] + 1;
                break;
            case 2:
                spread = 1;
                delay = rand8() % lbl_1_data_1D8[2];
                break;
            case 3:
                spread = 1;
                delay = rand8() % lbl_1_data_1D8[3];
                break;
            }
            best = 10000.0;
            selected = rand8() % count;
            lower = previousColumn - spread;
            upper = previousColumn + spread;
            if (lower < 0) {
                lower = 0;
            }
            if (upper > 7) {
                upper = 7;
            }
            if (lower <= columns[0] && columns[count] <= upper) {
                delay = rand8() % 10 + 1;
            } else {
                for (i = 0; i < count; i++) {
                    switch (com->difficulty) {
                    case 0:
                        if (distances[i] < best && rand8() % 100 < 50) {
                            selected = i;
                            best = distances[i];
                        }
                        break;
                    case 1:
                        if (distances[i] < best && rand8() % 100 < 70) {
                            selected = i;
                            best = distances[i];
                        }
                        break;
                    case 2:
                        if (distances[i] < best && rand8() % 100 < 80) {
                            selected = i;
                            best = distances[i];
                        }
                        break;
                    case 3:
                        if (distances[i] < best) {
                            selected = i;
                            best = distances[i];
                        }
                        break;
                    }
                }
            }
            route->distance = distances[selected];
            route->column = columns[selected];
            route->pos = positions[selected];
            if (abs(previousColumn - columns[selected]) <= 1) {
                delay = rand8() % 10 + 1;
            }
            route->delay = delay;
            previousColumn = columns[selected];
        }
    }
}

void fn_1_64B0(M659ComSlot *slot)
{
    M659ComManager *manager = lbl_1_bss_60->data;
    M659Player *player = slot->obj->data;
    M659Motion *motion = &player->motion;
    M659OMData *board = lbl_1_bss_4C->data;
    int unusedA = 0, unusedB = 0, unusedC = 0;
    M659ComSlot **com = &manager->players[player->info.side];
    s16 difficulty = player->com.difficulty;
    float unusedDistance = 1500.0f;
    s16 row = fn_1_33B0();
    if (row != player->com.routeIndex) {
        (*com)->countdown = player->com.routes[row].delay;
        player->com.routeIndex = row;
    }
    if (row < 22) {
        if ((*com)->countdown != 0) {
            (*com)->countdown--;
            return;
        }
        if (motion->state == 0) {
            if (player->com.routes[row].column > motion->column) {
                if (row == 0 || board->rows[row - 1].slots[motion->column + 1] == NULL) {
                    fn_1_6714(slot, 1);
                    return;
                }
                if (board->rows[row - 1].slots[motion->column + 1]->pos.z < 0.0f) {
                    fn_1_6714(slot, 1);
                    return;
                }
            } else if (player->com.routes[row].column < motion->column) {
                if (row == 0 || board->rows[row - 1].slots[motion->column - 1] == NULL) {
                    fn_1_6714(slot, 0);
                    return;
                }
                if (board->rows[row - 1].slots[motion->column - 1]->pos.z < 0.0f) {
                    fn_1_6714(slot, 0);
                }
            }
        }
    }
}

void fn_1_6714(M659ComSlot *com, s16 direction)
{
    M659Player *player = com->obj->data;
    player->com.direction = direction;
    player->com.active = 1;
    player->com.state = 0;
}
