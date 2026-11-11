#include "REL/framework/frame_time.h"
#include "dolphin/types.h"
#include "REL/framework/event_packet.h"
#include "REL/m681DLL/global_state.h"

#include "REL/m681DLL/player_input.h"
#include "REL/m681DLL/player_object.h"

u32 RandomUnsigned();
extern "C" {
#include "REL/m681DLL.h"
#include "REL/framework/world_units.h"
void __msl_assertion_failed(const char *, const char *, int);
extern u32 lbl_1_bss_70;
}

ModeBase::ModeBase() : EventListener()
{
    registerChannel(0);
    value = 0;
    duration = 0.0f;
}

ModeBase::~ModeBase() {}

s32 fn_1_2700(s32 value, s32 mask, s32 fallback)
{
    s32 selected = value & mask;
    switch (selected) {
    case 8: return 4;
    case 1: return 1;
    case 2: return 2;
    case 4: return 3;
    default: return fallback;
    }
}

void ModeBase::receive(const EventPacket *config)
{
    switch (config->type) {
    case 7:
        count = (s32)RandomUnsigned() % 100;
        break;
    case 8:
        value++;
        break;
    }
}

ModeToggle::ModeToggle() : ModeBase() { active = 0; }
ModeToggle::~ModeToggle() {}

void ModeToggle::update(PlayerObject *work)
{
    s32 currentButtons;
    s32 nextButtons;
    s32 actionButtons;
    s32 currentCode;
    u32 previousSelection;
    u32 currentSelection;
    u32 selected;
    u32 nextSelection;
    s32 nextCode;
    u32 retainedSelection;
    s32 actionCode;
    s32 actionValue;
    s32 action;

    currentButtons = work->buttons() & 0xD;
    nextButtons = work->stickDirections();
    actionButtons = work->subStickDirections();
    lbl_1_bss_70 = work->modes.values[0];
    previousSelection = work->modes.values[0];
    currentCode = currentButtons & 0xD;
    switch (currentCode) {
    case 8:
        currentSelection = 4;
        break;
    case 1:
        currentSelection = 1;
        break;
    case 2:
        currentSelection = 2;
        break;
    case 4:
        currentSelection = 3;
        break;
    default:
        currentSelection = previousSelection;
        break;
    }

    selected = currentSelection;
    if (selected == lbl_1_bss_70) {
        retainedSelection = work->modes.values[0];
        nextCode = nextButtons & 0xD;
        switch (nextCode) {
        case 8:
            nextSelection = 4;
            break;
        case 1:
            nextSelection = 1;
            break;
        case 2:
            nextSelection = 2;
            break;
        case 4:
            nextSelection = 3;
            break;
        default:
            nextSelection = retainedSelection;
            break;
        }
        selected = nextSelection;
    }
    work->modes.values[0] = selected;

    actionValue = work->modes.values[1];
    actionCode = actionButtons & 0xE;
    switch (actionCode) {
    case 8:
        action = 4;
        break;
    case 1:
        action = 1;
        break;
    case 2:
        action = 2;
        break;
    case 4:
        action = 3;
        break;
    default:
        action = actionValue;
        break;
    }
    work->modes.values[1] = action;
}

ModeObject::ModeObject(s32 mode) : ModeBase()
{
    this->mode = mode;
    active = 1;
    switch (mode) {
    case 0: modeValue = 3; break;
    case 1: modeValue = 8; break;
    case 2: modeValue = 13; break;
    case 3: modeValue = 18; break;
    default: break;
    }
}

ModeObject::~ModeObject() {}

void ModeObject::update(PlayerObject *result)
{
    s32 *primaryPair;
    s32 *secondaryPair;

    if (value >= modeValue && count <= 0x28) {
        primaryPair = &MinigameSingleton<M681GameState>::getChecked().textureChoices.values[2];
        result->modes.values[0] = primaryPair[0];
        result->modes.values[1] = primaryPair[1];
        return;
    }
    if (result->progress >= secondsToFrames(result->velocity)) {
        secondaryPair = &MinigameSingleton<M681GameState>::getChecked().textureChoices.values[0];
        result->modes.values[0] = secondaryPair[0];
        result->modes.values[1] = secondaryPair[1];
    }
}

extern "C" {
}

extern "C" {
s32 lbl_1_bss_68 = minutesToFrames(60L);
f32 lbl_1_bss_6C = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
}

extern "C" {
u32 lbl_1_bss_70;
}
