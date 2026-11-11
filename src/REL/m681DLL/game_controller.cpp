#include "REL/framework/frame_time.h"
#include "math.h"
#include "REL/m681DLL/monitor_interface.h"
#include "game/main.h"
#include "game/object.h"
#include "game/charman.h"
#include "game/hu3d.h"
#include "game/gamework.h"
#include "game/wipe.h"
#include "game/mg/timer.h"
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include "REL/m681DLL.h"
}
#include "REL/framework/minigame_core.h"
#include "REL/framework/game_object.h"
#include "REL/m681DLL/owner_182c.h"
#include "REL/framework/world_units.h"

extern "C" {

extern s32 lbl_1_data_F18;
extern float lbl_1_bss_1BC;
extern s8 lbl_1_data_2758;
s32 fn_1_367CC(s32 *player);

extern char lbl_1_data_13C[];
extern char lbl_1_data_CE[];

}

extern "C" {
void sendEvent__FiUlf(s32, s32, f32);

}

M681GameState::M681GameState() : GameObject(2)
{
    Callback initialCallback;
    minimum = 4;
    current = 4;
    negativeState = -1;
    state38 = 0;
    state3C = 0;
    state40 = 0.0f;
    state44 = 0.0f;
    state48 = 0.0f;
    state4C = 0;
    registerChannel(0);
    initialCallback = &M681GameState::fn_1_1B04;
    callback = initialCallback;
    textureChoices.values[0] = 0;
    textureChoices.values[1] = 0;
    textureChoices.values[2] = 0;
    textureChoices.values[3] = 0;
    interval = 2.4f;
    frameCount = 0;
}

M681GameState::~M681GameState() {}

void M681GameState::update()
{
    (this->*callback)();
    if (current > minimum) {
        current = minimum;
        sendEvent__FiUlf(1, 0, 0.0f);
    }
    fn_1_2138();
}

void M681GameState::fn_1_1B04() {}
void M681GameState::fn_1_1B08() {}

void M681GameState::fn_1_1B0C()
{
    frameCount++;
    if ((f32)frameCount >= secondsToFrames(interval)) {
        sendEvent__FiUlf(0, 7, 0.0f);
        frameCount = 0;
    }
}

static inline f32 MaxF182C(f32 a, f32 b)
{
    return a > b ? a : b;
}

void M681GameState::fn_1_1B98()
{
    frameCount++;
    if ((f32)frameCount > secondsToFrames(interval)) {
        interval = MaxF182C(0.2f, interval - 0.1f);
        frameCount = 0;
        sendEvent__FiUlf(0, 8, 0.0f);
    }
}

void M681GameState::fn_1_1CA8() {}

void M681GameState::fn_1_1CAC() {}
void M681GameState::fn_1_1CB0() {}
void M681GameState::fn_1_1CB4() {}

void M681GameState::receive(const EventPacket *selection)
{
    switch (selection->type) {
    case 1: {

        fn_1_1FB0();
        fn_1_381C(&MinigameSingleton<Monitor>::getChecked(), &textureChoices, 2.4f);
        M681GameState::Callback callback =
            &M681GameState::fn_1_1B08;
        this->callback = callback;
        break;
    }
    case 5: {

        fn_1_1FB0();
        fn_1_381C(&MinigameSingleton<Monitor>::getChecked(), &textureChoices, interval);
        ++state4C;
        M681GameState::Callback callback =
            &M681GameState::fn_1_1B0C;
        this->callback = callback;
        break;
    }
    case 7: {
        M681GameState::Callback callback =
            &M681GameState::fn_1_1B98;
        this->callback = callback;
        break;
    }
    case 8: {
        M681GameState::Callback callback =
            &M681GameState::fn_1_1CA8;
        this->callback = callback;
        break;
    }
    case 9: {
        M681GameState::Callback callback =
            &M681GameState::fn_1_1CAC;
        this->callback = callback;
        break;
    }
    case 10: {
        M681GameState::Callback callback =
            &M681GameState::fn_1_1CB0;
        this->callback = callback;
        break;
    }
    case 11: {
        M681GameState::Callback callback =
            &M681GameState::fn_1_1CB4;
        this->callback = callback;
        break;
    }
    default: {
        M681GameState::Callback callback =
            &M681GameState::fn_1_1B04;
        this->callback = callback;
        break;
    }
    }
}

extern "C" s32 RandomUnsigned__Fv(void);
extern "C" {
const s32 lbl_1_rodata_150[8] = { 4, 1, 3, 0, 4, 2, 3, 0 };
}

void M681GameState::fn_1_1FB0()
{
    textureChoices.values[0] = lbl_1_rodata_150[RandomUnsigned__Fv() % 3];
    textureChoices.values[1] = *(&lbl_1_rodata_150[RandomUnsigned__Fv() % 3] + 4);
    textureChoices.values[2] = lbl_1_rodata_150[RandomUnsigned__Fv() % 4];
    textureChoices.values[3] = *(&lbl_1_rodata_150[RandomUnsigned__Fv() % 4] + 4);

    if (textureChoices.values[1] == 3 && textureChoices.values[0] == 3) {
        if (RandomUnsigned__Fv() % 2 != 0) {
            textureChoices.values[0] = lbl_1_rodata_150[RandomUnsigned__Fv() % 2];
            return;
        }
        textureChoices.values[1] = *(&lbl_1_rodata_150[RandomUnsigned__Fv() % 2] + 4);
    }
}

void M681GameState::fn_1_2138()
{
}

extern "C" {

}

extern "C" {

s32 lbl_1_bss_48 = minutesToFrames(60L);
f32 lbl_1_bss_4C = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));

}
