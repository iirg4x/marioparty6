#include "REL/framework/random.h"
#include "REL/framework/event.h"
#include "REL/framework/p_info.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
// The computer's walk: picks a panel by its level's weight, walks to it and steps on it.
#include <vector>
#include "REL/m654DLL/player.h"
#include "REL/m654DLL/switch_panel.h"
#include "REL/m654DLL/pad_control.h"
#include "REL/framework/singleton.h"
#include "REL/framework/event_manager.h"

struct SampleWeights {
    double values[4];
};

// Releases the player control base object.
inline PlayerControl::~PlayerControl() {}

// Releases the human control base object.
HumanControl::~HumanControl() {}

// Binds the player, selects its difficulty weight and resets the simulated pad.
ComputerControl::ComputerControl(Player *owner)
    : PlayerControl(owner),
      callback(&ComputerControl::fn_1_170),
      weightIndex(owner->difficulty()),
      pad(1, 0.0f)
{
    pad.fn_1_C630(0.0f);
}

// Leaves the computer control idle.
void ComputerControl::fn_1_170() {}

// Releases the simulated pad and control base object.
ComputerControl::~ComputerControl()
{
}

// Resets the simulated pad and chooses a panel target.
void ComputerControl::enter()
{
    pad.fn_1_C630(0.0f);
    fn_1_29C();
}

// Runs the computer state and submits the simulated pad to the actor.
void ComputerControl::update()
{
    if (owner->inactive) {
        fn_1_29C();
    } else {
        (this->*callback)();
    }
    pad.fn_1_C918(owner->player);
}

// Blends the preferred and total panel counts, truncating to an integer.
static inline int interpolateSampleCount(const int &start, const int &end, double weight)
{
    return (int)((1.0 - weight) * (double)start + weight * (double)end);
}

// Chooses a panel position using the player difficulty weight.
void ComputerControl::fn_1_29C()
{
    std::vector<Vec> sequence;
    int selectedCount;
    int sampleCount;
    int sequenceCount;
    StateCallback nextCallback;

    selectedCount = fn_1_8AD4(&sequence);
    sequenceCount = sequence.size();
    SampleWeights weights = {0.9, 0.7, 0.3, 0.1};
    sampleCount = interpolateSampleCount(selectedCount, sequenceCount, weights.values[weightIndex]);
    if (sampleCount <= 0) {
        return;
    }

    // The signed remainder can be negative if the random value has its high bit set.
    const int selectedIndex = (int)RandomUnsigned() % sampleCount;
    Vec *selectedEntry = &sequence[selectedIndex];
    position.x = selectedEntry->x;
    position.y = selectedEntry->y;
    position.z = selectedEntry->z;
    nextCallback = &ComputerControl::fn_1_50C;
    callback = nextCallback;
}

// Steers toward the chosen panel and presses button 0x100 when close.
void ComputerControl::fn_1_50C()
{
    if (owner->collidedThisFrame != 0) {
        fn_1_29C();
    }

    Vec *target = &owner->player->actor->pos;
    Vec displacement;
    PSVECSubtract(&position, target, &displacement);
    displacement.y = 0.0f;
    float distance = PSVECMag(&displacement);
    float distanceForCheck = distance;
    if (distanceForCheck > 100.0f) {
        PSVECScale(&displacement, &displacement, 10.0f / distanceForCheck);
        pad.fn_1_C6E8(&displacement, 0.1f);
    } else {
        pad.fn_1_C6C8();
        StateCallback next = &ComputerControl::fn_1_7C8;
        callback = next;
    }
}

// Holds button 0x100 and switches to the airborne control state.
void ComputerControl::fn_1_7C8()
{
    StateCallback nextCallback;
    pad.fn_1_C6D8();
    nextCallback = &ComputerControl::fn_1_830;
    callback = nextCallback;
}

// Holds button 0x100 while rising without collision, then chooses another panel.
void ComputerControl::fn_1_830()
{
    Vec *actorPosition = &owner->player->actor->pos;
    Vec *velocity = &owner->velocity;
    if (owner->fn_1_4B68() == 0 && velocity->y >= 0.0f) {
        pad.fn_1_C6D8();
        return;
    }
    pad.fn_1_C6C8();
    fn_1_29C();
}

// Provides the default control entry hook.
void PlayerControl::enter() {}
// Leaves human input to the engine.
void HumanControl::update() {}

// One-minute time limit and gravity in world units per frame squared.
int lbl_1_bss_0 = minutesToFrames(1L);
float lbl_1_bss_4 = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
