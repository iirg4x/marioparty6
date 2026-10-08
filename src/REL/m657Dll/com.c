/* Schedules computer-controlled players' action-button timing. */
#include "REL/m657Dll.h"
#include "game/memory.h"
#include "game/frand.h"
#include "string.h"

typedef struct {
    /* Exclusive upper bound for the random A-button hold interval, in frames. */
    int delay;
    /* Exclusive upper bound for the random inactive interval after the A-button phase, in
     * frames. */
    int duration;
    /* Random timing variation in frames, applied to the A-button interval or following inactive
     * interval by the profile. */
    int variation;
} M657ComTiming;

static M657ComTiming lbl_1_data_1F8[4] = {
    { 30, 60, 17 }, { 37, 30, 17 }, { 41, 24, 7 }, { 58, 30, 10 }
};

OMOBJ *lbl_1_bss_40;

/* Creates the computer-input controller and its per-team records. */
void fn_1_4570(OMOBJMAN *objman)
{
    OMOBJ *obj;
    M657ComWork *controllerWork;

    obj = lbl_1_bss_40 = omAddObjEx(objman, 70, 0, 0, -1, fn_1_4608);
    obj->stat |= OM_STAT_MODELPAUSE;
    controllerWork = obj->data = HuMemDirectMallocNum(HEAP_HEAP,
        sizeof(M657ComWork), HU_MEMNUM_OVL);
    memset(controllerWork, 0, sizeof(M657ComWork));
}

/* Object-create callback that resets controller timing and installs its update. */
void fn_1_4608(OMOBJ *obj)
{
    M657ComWork *controllerWork = obj->data;

    controllerWork->state = 0;
    controllerWork->timer = 0;
    obj->objFunc = fn_1_463C;
}

/* Chooses and advances computer input sequences during the main game mode. */
void fn_1_463C(OMOBJ *obj)
{
    M657ComWork *controllerWork = obj->data;
    M657ComPlayer **computerPlayer = NULL;
    int teamIndex = 0;

    if (MgSeqModeGet() > MGSEQ_MODE_MAIN) {
        controllerWork->state = 0;
        controllerWork->timer = 0;
        obj->objFunc = fn_1_47A8;
    }
    /* The current tick still runs through the state switch after scheduling the post-game
     * callback. */
    switch (controllerWork->state) {
        case 0:
            if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
                controllerWork->state++;
            }
            break;
        case 1:
            controllerWork->state++;
            break;
        case 2:
            for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                computerPlayer = &controllerWork->players[teamIndex];
                if ((*computerPlayer)->enabled && fn_1_4910(*computerPlayer)) {
                    fn_1_4A80(*computerPlayer);
                }
            }
            controllerWork->state++;
            break;
        case 3:
            for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                computerPlayer = &controllerWork->players[teamIndex];
                if ((*computerPlayer)->enabled) {
                    fn_1_4964(*computerPlayer);
                    if (fn_1_4910(*computerPlayer)) {
                        controllerWork->state = 2;
                    }
                }
            }
            break;
    }
}

/* Switches the controller to its post-game callback after the main mode ends. */
void fn_1_47A8(OMOBJ *obj)
{
    M657ComWork *controllerWork = obj->data;
    int i = 0;

    obj->objFunc = fn_1_47D0;
}

/* Post-game callback; it currently performs no work. */
void fn_1_47D0(OMOBJ *obj)
{
    M657ComWork *controllerWork = obj->data;
    int i = 0;
}

/* Registers a player with the computer-input controller when its state is idle. */
void fn_1_47EC(OMOBJ *obj)
{
    M657ComPlayer **computerPlayer;
    M657Player *playerWork = obj->data;
    M657ComWork *controllerWork = lbl_1_bss_40->data;

    computerPlayer = &controllerWork->players[playerWork->player.team];
    if (controllerWork->state == 0) {
        if (*computerPlayer != NULL) {
            OSReport("Error: M657ComPlayerSet() - !*com != NULL\n");
        }
        /* A duplicate registration logs an error, then makes the new record active for this
         * team. */
        *computerPlayer = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M657ComPlayer), HU_MEMNUM_OVL);
        if (*computerPlayer) {
            (*computerPlayer)->obj = obj;
            (*computerPlayer)->enabled = playerWork->computer;
        } else {
            OSReport("Error: M657ComPlayerSet() - !omMalloc()\n");
        }
    }
}

/* Returns the controller's first value whose meaning is not established here. */
s32 fn_1_48C0(void)
{
    M657ComWork *controllerWork = lbl_1_bss_40->data;
    return controllerWork->unknownValueA;
}

/* Returns the controller's second value whose meaning is not established here. */
s32 fn_1_48E8(void)
{
    M657ComWork *controllerWork = lbl_1_bss_40->data;
    return controllerWork->unknownValueB;
}

/* Reports whether an enabled computer player's input sequence is inactive. */
s32 fn_1_4910(M657ComPlayer *com)
{
    M657ComWork *controllerWork = lbl_1_bss_40->data;
    M657Player *player;

    if (com->enabled) {
        player = com->obj->data;
        if (player->inputActive == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

/* Advances the A-button interval, the following inactive interval, and the input reset. */
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
    /* This unconditional increment also leaves the reset input timer at one for the next update. */
    player->inputTimer++;
}

/* Sets the selected computer player's post-button inactive interval from its profile. */
void fn_1_4A48(M657ComPlayer *com)
{
    M657Player *player = com->obj->data;
    player->inputDuration = lbl_1_data_1F8[player->difficulty].duration;
}

/* Selects the timing profile for the player's configured computer difficulty. */
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

/* Starts the easiest computer profile with random A-button and following inactive intervals. */
void fn_1_4B10(M657ComPlayer *com)
{
    M657Player *player = com->obj->data;
    player->inputDelay = frand() % 54;
    player->inputDuration = frand() % 120;
    player->inputActive = 1;
    player->inputState = 0;
}

/* Starts the second computer timing profile using its configured random ranges. */
void fn_1_4BA4(M657ComPlayer *com)
{
    M657Player *player = com->obj->data;
    int durationVariation = frand() % lbl_1_data_1F8[player->difficulty].variation;

    player->inputDelay = frand() % lbl_1_data_1F8[player->difficulty].delay;
    player->inputDuration =
        durationVariation + frand() % lbl_1_data_1F8[player->difficulty].duration;
    player->inputActive = 1;
    player->inputState = 0;
}

/* Starts the third computer timing profile using its configured random ranges. */
void fn_1_4C78(M657ComPlayer *com)
{
    M657Player *player = com->obj->data;
    int durationVariation = frand() % lbl_1_data_1F8[player->difficulty].variation;

    player->inputDelay = frand() % lbl_1_data_1F8[player->difficulty].delay;
    player->inputDuration =
        durationVariation + frand() % lbl_1_data_1F8[player->difficulty].duration;
    player->inputActive = 1;
    player->inputState = 0;
}

/* Starts the hardest computer profile, including its rare long A-button interval. */
void fn_1_4D4C(M657ComPlayer *com)
{
    M657Player *player = com->obj->data;
    int durationVariation = frand() % lbl_1_data_1F8[player->difficulty].variation;

    if (frand() % 100 == 0) {
        /* This rare branch sets the A-button interval to 420 frames and the following inactive
         * interval to 600 frames. */
        player->inputDelay = 420;
        player->inputDuration = 600;
    } else {
        player->inputDelay = durationVariation + frand() % lbl_1_data_1F8[player->difficulty].delay;
        player->inputDuration = frand() % lbl_1_data_1F8[player->difficulty].duration;
    }
    player->inputActive = 1;
    player->inputState = 0;
}
