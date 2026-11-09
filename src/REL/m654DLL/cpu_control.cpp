#include "REL/m654DLL/framework_calls.h"
#include "REL/m654DLL/time_conversion.h"
#include "REL/m654DLL/unit_conversion.h"
// The computer's walk: picks a panel by its level's weight, walks to it and steps on it.
#include <vector>
#include "REL/m654DLL/mode_object.h"
#include "REL/m654DLL/pad_control.h"
#include "REL/framework/minigame_singleton.h"
class EventManager;

struct PlayerActorView {
    unsigned char unknown00[0x3C]; /* No code names this field. */
    unsigned char unknown3C[0x14]; /* No code names this field. */
    Vec position;
};

struct PlayerStateView {
    unsigned char unknown00[0x3C]; /* No code names this field. */
    PlayerActorView *actor;
};

struct PlayerModeContext230 {
    unsigned char unknown00[0x3C]; /* No code names this field. */
    PlayerStateView *value;
    unsigned char active : 1;
    unsigned char started : 1;
    unsigned char unknown44 : 6;
    unsigned char unknown41[0x1F]; /* No code names this field. */
    Vec velocity;
};

struct SampleWeights29C {
    double values[4];
};

extern "C" {
void __dl__FPv(void *);
}
int fn_1_1C5BC(s32 *playerNo);
extern "C" {
int fn_1_8AD4(std::vector<Vec> *sequence);

unsigned char fn_1_4B68(void *object);
}

inline PlayerControl::~PlayerControl() {}

HumanControl::~HumanControl() {}

ComputerControl::ComputerControl(void *owner) : PlayerControl(static_cast<Player *>(owner)), callback(&ComputerControl::fn_1_170), weightIndex(fn_1_1C5BC((s32 *)owner)), cameraControl(1, 0.0f)
{
    fn_1_C630(&cameraControl, 0.0f);
}

void ComputerControl::fn_1_170() {}

ComputerControl::~ComputerControl()
{
}

void ComputerControl::enter()
{
    fn_1_C630(&cameraControl, 0.0f);
    fn_1_29C();
}

void ComputerControl::update()
{
    if (reinterpret_cast<PlayerModeContext230 *>(work)->active) {
        fn_1_29C();
    } else {
        (this->*callback)();
    }
    cameraControl.fn_1_C918((MgPlayer_s *)reinterpret_cast<PlayerModeContext230 *>(work)->value);
}

static inline int interpolateSampleCount(const int &start, const int &end, double weight)
{
    return (int)((1.0 - weight) * (double)start + weight * (double)end);
}

void ComputerControl::fn_1_29C()
{
    std::vector<Vec> sequence;
    int selectedCount;
    int sampleCount;
    int sequenceCount;
    Callback29C nextCallback;

    selectedCount = fn_1_8AD4(&sequence);
    sequenceCount = sequence.size();
    SampleWeights29C weights = {0.9, 0.7, 0.3, 0.1};
    sampleCount = interpolateSampleCount(selectedCount, sequenceCount, weights.values[weightIndex]);
    if (sampleCount <= 0) {
        return;
    }

    const int selectedIndex = (int)RandomUnsigned() % sampleCount;
    Vec *selectedEntry = &sequence[selectedIndex];
    position.x = selectedEntry->x;
    position.y = selectedEntry->y;
    position.z = selectedEntry->z;
    nextCallback = &ComputerControl::fn_1_50C;
    callback = nextCallback;
}

void ComputerControl::fn_1_50C()
{
    if (reinterpret_cast<PlayerModeContext230 *>(work)->started != 0) {
        fn_1_29C();
    }

    Vec *target = &reinterpret_cast<PlayerModeContext230 *>(work)->value->actor->position;
    Vec displacement;
    PSVECSubtract(&position, target, &displacement);
    displacement.y = 0.0f;
    float distance = PSVECMag(&displacement);
    float distanceForCheck = distance;
    if (distanceForCheck > 100.0f) {
        PSVECScale(&displacement, &displacement, 10.0f / distanceForCheck);
        cameraControl.fn_1_C6E8(&displacement, 0.1f);
    } else {
        cameraControl.fn_1_C6C8();
        Callback29C next = &ComputerControl::fn_1_7C8;
        callback = next;
    }
}

/* Initializes the embedded pad control, then copies the player's mode data. */
void ComputerControl::fn_1_7C8()
{
    Callback29C copy;
    cameraControl.fn_1_C6D8();
    copy = &ComputerControl::fn_1_830;
    callback = copy;
}

void ComputerControl::fn_1_830()
{
    void *unknownAddress = &reinterpret_cast<PlayerModeContext230 *>(work)->value->actor->position;
    Vec *velocity = &reinterpret_cast<PlayerModeContext230 *>(work)->velocity;
    if (fn_1_4B68(work) == 0 && velocity->y >= 0.0f) {
        cameraControl.fn_1_C6D8();
        return;
    }
    cameraControl.fn_1_C6C8();
    fn_1_29C();
}

void PlayerControl::enter() {}
void HumanControl::update() {}

extern "C" {
int lbl_1_bss_0 = fn_1_B80(1);
float lbl_1_bss_4 = framesToSeconds(framesToSeconds(980.665));
}

// Native event singleton slot, consumed by the framework event methods (.bss+0x8).
template<> EventManager *MinigameSingleton<EventManager>::self = 0;
