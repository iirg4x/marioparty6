
#include "math.h"

#include "dolphin/types.h"

#include "REL/m604DLL/player_input.h"
#include "REL/m604DLL/day_computer_input.h"

#include "game/gamework.h"

#include "game/pad.h"

extern "C" {
#include "game/frand.h"
}

#include "REL/m604DLL/computer_player_configuration.h"

#include "REL/framework/singleton.h"

#include "REL/m604DLL/player.h"
#include "REL/m604DLL/patapata.h"

void fn_1_6DC(PlayerInputBase *input)
{
    input->stick.x = 0.0f;
    input->stick.y = 0.0f;
    input->stick.pressedButtons = 0;
    input->stick.magnitude = 0.0f;
    input->read(&input->stick);
}

HumanPlayerConfiguration::HumanPlayerConfiguration(Player *owner) : PlayerInputBase(owner)
{
    Player *source;
    s32 playerIndex;
    s32 configurationIndex;
    s32 tableIndex;
    source = static_cast<Player *>(configuration);
    configurationIndex = source->playerIndex;
    tableIndex = configurationIndex;
    playerIndex = tableIndex;
    padNumber = lbl_80265728[playerIndex].padNo;
}

void HumanPlayerConfiguration::read(StickState *state)
{
    s32 padIndex = padNumber;
    f32 x = lbl_8024C7D8[padIndex];
    f32 y = lbl_8024C7E8[padIndex];
    f64 magnitude = sqrt((f64)(x * x + y * y));

    state->x = x;
    state->y = y;
    state->magnitude = (f32)magnitude;
    state->pressedButtons = lbl_802C0008[padIndex];
}

DayComputerInput::DayComputerInput(Player *owner, s32 difficulty) : PlayerInputBase(owner)
{
    recordIndex = -1;
    state = 0;
    directionX = 0.0f;
    directionY = 0.0f;
    waitDuration = 0.0f;
    waitElapsed = 0.0f;
    initialDelaySet = 0;
    initialized = 0;
    stick.mode = difficulty;
}

DayComputerInput::~DayComputerInput() {}

const f32 lbl_1_rodata_68[4] = { 1.0f, 0.8f, 0.2f, 0.03f };

const s32 lbl_1_rodata_78[4] = { 2, 3, 8, 18 };

void DayComputerInput::read(StickState *output2)
{
    s32 playerIndex = configuration->getPlayerIndex();
    Vec ownerPosition = configuration->position;
    s32 outputCount = output2->mode;
    s32 ownerRecordIndex = configuration->step;
    s32 recordIndex = 0;
    if (ownerRecordIndex != - 1)
    {
        recordIndex = ownerRecordIndex + 1;
    }
    if (recordIndex > 0x30)
    {
        recordIndex = 0x30;
    }
    Patapata * manager = &Patapata::getChecked();
    PatapataRecord * record = manager->getRecord(playerIndex, recordIndex);
    f32 distanceX = (ownerPosition.x - record->position.x < 0.0f)
        ? -(ownerPosition.x - record->position.x) : ownerPosition.x - record->position.x;
    f32 distanceY = ownerPosition.y - record->position.y;
    if (ownerRecordIndex != this->recordIndex || configuration->motion == 6)
    {
        initialDelaySet = 0;
        state = 4;
        if (record->moving == 1 && outputCount != 0)
        {
            state = 1;
            output2->pressedButtons |= 0x100;
        }
    }
    if (initialized == 0)
    {
        state = 4;
        waitDuration = 0.1f * (f32)(frand() % 7);
        initialDelaySet = 1;
        waitElapsed = 0.0f;
        initialized = 1;
    }
    switch (state)
    {
    case 0 :
        if (distanceX > 90.0f)
        {
            state = 1;
            output2->pressedButtons |= 0x100;
        }
        else
        {
            if (distanceX < 60.000004f)
            {
                if ((frand() < 0 ? -frand() : frand()) % lbl_1_rodata_78[outputCount] == 0)
                {
                    state = 1;
                    output2->pressedButtons |= 0x100;
                }
                else
                {
                    state = 3;
                }
            }
            else
            {
                state = 2;
                output2->pressedButtons |= 0x100;
            }
        }
        break;
    case 2 :
        if (record->moving == 0)
        {
            if (record->type > 7 || record->type < 2)
            {
                if (ownerPosition.x > record->position.x)
                {
                    output2->x = 1.0f;
                }
                else
                {
                    output2->x = - 1.0f;
                }
                if (distanceX > 80.0f || distanceY > 40.0f)
                {
                    state = 1;
                }
            }
            else
            {
                if (record->type == 2)
                {
                    if (ownerPosition.x > record->position.x)
                    {
                        output2->x = 1.0f;
                    }
                    else
                    {
                        output2->x = - 1.0f;
                    }
                }
                else
                {
                    if (ownerPosition.x > record->position.x)
                    {
                        output2->x = 1.0f;
                    }
                    else
                    {
                        output2->x = - 1.0f;
                    }
                }
                if (distanceX > 110.0f || distanceY > 40.0f)
                {
                    state = 1;
                }
            }
        }
        else
        {
            state = 1;
        }
        output2->magnitude = 1.0f;
        break;
    case 1 :
        if (distanceX > 50.0f)
        {
            if (ownerPosition.x < record->position.x) {
                output2->x = 1.0f;
            } else {
                output2->x = -1.0f;
            }
            directionX = output2->x;
            directionX = output2->y;
        }
        else
        {
            output2->x = directionX;
            output2->y = directionY;
        }
        output2->magnitude = 1.0f;
        if (configuration->motion == 5)
        {
            state = 2;
        }
        output2->pressedButtons |= 0x100;
        break;
    case 3 :
        if (ownerPosition.x > record->position.x) {
            output2->x = 1.0f;
        } else {
            output2->x = -1.0f;
        }
        output2->magnitude = 1.0f;
        if (distanceX > 50.0f)
        {
            state = 2;
            output2->pressedButtons |= 0x100;
        }
        break;
    case 4 :
        if (initialDelaySet == 0)
        {
            waitDuration = lbl_1_rodata_68[outputCount] * (0.000015258789f * (f32)(u16) frand());
            waitElapsed = 0.0f;
            initialDelaySet++;
        }
        waitElapsed += 0.016666666666666666;
        if (! (waitElapsed < waitDuration))
        {
            state = 0;
            this->recordIndex = ownerRecordIndex;
        }
        break;
    }
    if (configuration->motion == 5)
    {
        output2->x = 0.0f;
    }
}

ComputerPlayerConfiguration::ComputerPlayerConfiguration(Player *player, s32 difficulty)
    : PlayerInputBase(player)
{
    recordIndex = -1;
    movementState = 0;
    directionX = 0.0f;
    unk28 = 0.0f;
    directionY = 0.0f;
    waitDuration = 0.0f;
    waitElapsed = 0.0f;
    waitInitialized = 0;
    delayInitialized = 0;
    previousActorRecord = -1;
    randomCounter = 0;
    stick.mode = difficulty;
}

ComputerPlayerConfiguration::~ComputerPlayerConfiguration() {}

f32 RandomFloat(f32, f32);

static inline PatapataRecord *getRecord(Patapata *manager, s32 playerIndex, s32 recordIndex)
{
    return &manager->records[playerIndex][recordIndex];
}

/* Debug value hook. */
static inline void DebugValue(bool value) { }

const f32 lbl_1_rodata_C0[4] = { 0.6f, 0.6f, 0.5f, 0.1f };

const s32 lbl_1_rodata_D0[4] = { 1, 2, 3, 0 };

void ComputerPlayerConfiguration::read(StickState *output)
{
    PatapataRecord * currentRecord;
    s32 playerIndex = configuration->getPlayerIndex();
    Vec position = configuration->position;
    s32 outputMode = output->mode;
    s32 actorRecordIndex = configuration->step;
    bool hasVerticalPosition = configuration->hasVerticalPosition();
    Patapata *manager = &Patapata::getChecked();
    s32 recordIndex = 0;
    if (hasVerticalPosition == 1) {
        recordIndex = actorRecordIndex;
    }
    if (recordIndex < 0) {
        recordIndex = 0;
    }
    currentRecord = getRecord(manager, playerIndex, recordIndex);

    s32 targetIndex = (s32)((position.y - 300.0f) / 400.0f);
    if (hasVerticalPosition == 1) {
        targetIndex = actorRecordIndex - 1;
    }
    if (targetIndex < 0) {
        targetIndex = 0;
    }
    PatapataRecord *targetRecord = getRecord(manager, playerIndex, targetIndex);
    f32 horizontalDistance = position.x - currentRecord->position.x < 0.0f
        ? -(position.x - currentRecord->position.x) : position.x - currentRecord->position.x;
    f32 verticalDistance = position.y - currentRecord->position.y;

    switch (delayInitialized) {
    case 0:
            movementState = 6;
            directionX = RandomFloat(-1.0f, 1.0f);
            directionY = RandomFloat(0.5f, 1.0f);
            waitDuration = 0.1f * (f32)(frand() % 7);
            waitInitialized = 1;
            waitElapsed = 0.0f;
            delayInitialized++;
            // Continue with the first wait tick.
    case 1:
    output->x = directionX;
    output->magnitude = directionY;
    waitElapsed += 0.016666666666666666;
    if (waitElapsed < waitDuration) return;
    delayInitialized++;
    break;
    default:
        break;
    }
        if (actorRecordIndex != this->recordIndex || configuration->motion == 6) {
            waitInitialized = 0;
            movementState = 6;
            previousActorRecord = targetIndex;
        }
        if ((hasVerticalPosition == 0) && (previousActorRecord != targetIndex) && (movementState != 6)) {
            waitInitialized = 0;
            if (outputMode != 3) {
                if ((randomCounter % (u32)lbl_1_rodata_D0[outputMode]) != 0) {
                    movementState = 0;
                } else {
                    movementState = 1;
                }
            } else {
                DebugValue(movementState == 0);
            }
            randomCounter++;
            previousActorRecord = targetIndex;
        }
            switch (movementState) {
            case 0:
                if (horizontalDistance > 300.0f) {
                    output->x = 0.0f;
                    output->magnitude = 1.0f;
                    return;
                }
                if (hasVerticalPosition == 0) {
                    f32 distanceLimit = 60.000004f;
                    if (targetRecord->moving == 1) {
                        distanceLimit = 320.0f;
                    }
                    if ((position.x - targetRecord->position.x < 0.0f
                            ? -(position.x - targetRecord->position.x)
                            : position.x - targetRecord->position.x) <= distanceLimit) {
                        if ((position.x < 0.0f ? -position.x : position.x) < 100.0f) {
                            if (position.x > targetRecord->position.x) {
                                movementState = 5;
                                return;
                            } else {
                                movementState = 4;
                                return;
                            }
                        } else {
                            if (position.x < 0.0f) {
                                movementState = 5;
                                return;
                            } else {
                                movementState = 4;
                                return;
                            }
                        }
                    }
                    movementState = 7;
                    return;
                }
                movementState = 3;
                return;
            case 1:
                if (horizontalDistance > 300.0f) {
                    output->x = 0.0f;
                    output->magnitude = 1.0f;
                    return;
                }
                if (hasVerticalPosition == 0) {
                    f32 distanceLimit = 60.000004f;
                    if (targetRecord->moving == 1) {
                        distanceLimit = 320.0f;
                    }
                    if ((position.x - targetRecord->position.x < 0.0f
                            ? -(position.x - targetRecord->position.x)
                            : position.x - targetRecord->position.x) <= distanceLimit) {
                        if ((position.x < 0.0f ? -position.x : position.x) < 100.0f) {
                            if (position.x > targetRecord->position.x) {
                                movementState = 4;
                                return;
                            } else {
                                movementState = 5;
                                return;
                            }
                        } else {
                            if (position.x < 0.0f) {
                                movementState = 4;
                                return;
                            } else {
                                movementState = 5;
                                return;
                            }
                        }
                    }
                    movementState = 7;
                    return;
                }
                movementState = 3;
                return;
            case 3:
                if (currentRecord->moving == 0) {
                    if ((position.x < 0.0f ? -position.x : position.x) < 100.0f) {
                        if (position.x > currentRecord->position.x) {
                            movementState = 5;
                        }
                        else {
                            movementState = 4;
                        }
                    } else {
                        if (position.x < 0.0f) {
                            movementState = 5;
                        }
                        else {
                            movementState = 4;
                        }
                    }
                } else if (currentRecord->type == 4) {
                    if (horizontalDistance < 300.0f) {
                        output->x = 1.0f;
                    }
                } else if (horizontalDistance < 300.0f) {
                    output->x = -1.0f;
                }
                output->magnitude = 1.0f;
                return;
            case 4:
                output->x = -1.0f;
                output->magnitude = 1.0f;
                return;
            case 5:
                output->x = 1.0f;
                output->magnitude = 1.0f;
                return;
            case 6:
                if (waitInitialized == 0) {
                    waitDuration = lbl_1_rodata_C0[outputMode] * (1.5258789e-05f * (f32)(u16)frand());
                    waitElapsed = 0.0f;
                    waitInitialized++;
                }
                waitElapsed += 0.016666666666666666;
                if (waitElapsed < waitDuration) return;
                movementState = 0;
                this->recordIndex = actorRecordIndex;
            case 7:
                output->x = 0.0f;
                output->y = 0.0f;
                output->magnitude = 0.0f;
                break;
            }
}
