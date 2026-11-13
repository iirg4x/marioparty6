#include <algorithm>
#include "REL/m654DLL/sprite_manager.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/game_object_man.h"
// Each player's score display and the sprite manager that draws them over the scene.
#include "REL/m654DLL/display_provider.h"
#include "REL/m654DLL/parameter_interpolation.h"
#include "REL/m654DLL/sprite_draw.h"
extern "C" {
#include "game/data.h"
#include "game/gamemes.h"
#include "game/sprite.h"
}

// One-minute time limit and gravity in world units per frame squared.
s32 lbl_1_bss_98 = minutesToFrames(1L);
float lbl_1_bss_9C = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
s32 lbl_1_bss_A0 = (s32)secondsToFrames(0.5);
s32 lbl_1_bss_A4 = (s32)secondsToFrames(0.2);

extern const int lbl_1_rodata_2D8[2] = {0x9B001E, 0x9B001F};

// Creates the character scorebox and three background and animated score marks.
DisplayProvider::DisplayProvider(int character, int offsetX, int y)
    : GameObject(2),
      scorebox(MgScoreBoxCreateChar(0x80, 0x30, (short)character)),
      firstAnimation(HuSprAnimDataRead(lbl_1_rodata_2D8[0])),
      secondAnimation(HuSprAnimDataRead(lbl_1_rodata_2D8[1])),
      activeCount(0)
{
    float x;
    int index;

    MgScoreBoxPosSet((short)this->scorebox, (float)offsetX, (float)y);
    MgScoreBoxTPLvlSet((short)this->scorebox, 0.0f);
    MgScoreBoxDispSet((short)this->scorebox, 1);
    x = (float)(12.0 + (48.0 + ((double)offsetX - 64.0)));
    index = 0;
    while (index < 3) {
        SpriteParameters firstParameters(firstAnimation);
        firstParameters.x = x;
        firstParameters.y = (float)y;
        firstParameters.scale = 1.0f;
        firstParameters.opacity = 1.0f;
        firstList.push_back(firstParameters);

        SpriteParameters secondParameters(secondAnimation);
        secondParameters.x = x;
        secondParameters.y = (float)y;
        secondParameters.scale = 4.0f;
        secondParameters.opacity = 0.0f;
        secondList.push_back(secondParameters);
        x += 24.0f;
        index++;
    }
}

// Releases the score mark lists and game object base.
DisplayProvider::~DisplayProvider() {}

// Increments the active score mark count, capped at three.
int DisplayProvider::fn_1_66F4()
{
    int incrementedValue;
    int boundedValue;
    int result;
    int storedValue;

    incrementedValue = activeCount + 1;
    if (incrementedValue < 3) {
        boundedValue = incrementedValue;
    } else {
        boundedValue = 3;
    }
    result = boundedValue;
    storedValue = result;
    return activeCount = storedValue;
}

// Applies the shared fade and shrinks newly active marks while fading them in.
void DisplayProvider::update()
{
    double tpLevel = lbl_1_bss_A8.value;
    MgScoreBoxTPLvlSet((short)scorebox, tpLevel);
    if (activeCount > 0) {
        SpriteParameters *checkedLastRecord = &secondList.back();
        // These comparisons also admit unordered values.
        if (!(checkedLastRecord->scale <= 1.0)) {
            int index = 0;
            while (index < activeCount) {
                SpriteParameters *record = &secondList[index];
                if (!(record->scale <= 1.0)) {
                    record->scale = (float)((double)record->scale - (3.0 / (double)lbl_1_bss_A0));
                    record->opacity =
                        (float)((double)record->opacity + (1.0 / (double)lbl_1_bss_A0));
                    if (record->scale <= 1.0) {
                        record->scale = 1.0f;
                        record->opacity = 1.0f;
                    }
                }
                index++;
            }
        }
    }
}

// Collects score displays and draws their score mark sprites.
void fn_1_6910()
{
    // collectGameObjects replaces these two initial null entries.
    std::vector<GameObject *> list(2);
    collectGameObjects(2, &list);
    int count = list.size();
    fn_1_81B4();
    for (int index = 0; index < count; index++) {
        static_cast<DisplayProvider *>(list[index])->fn_1_6AEC();
    }
}

// Draws all background marks and the active animated marks.
inline void DisplayProvider::fn_1_6AEC()
{
    long fixedIndex;
    long activeIndex;

    for (fixedIndex = 0; fixedIndex < 3; fixedIndex++) {
        fn_1_8314(&firstList[fixedIndex]);
    }
    for (activeIndex = 0; activeIndex < activeCount; activeIndex++) {
        fn_1_8314(&secondList[activeIndex]);
    }
}

void fn_1_74AC(HuSprite_s *);

// Deletes the display held by this slot.
inline ScoreDisplaySlot::~ScoreDisplaySlot() { delete value; }

// Installs the sprite drawing callback and resets the shared fade.
SpriteManager::SpriteManager()
{
    short group = HuSprGrpCreate(1);
    short sprite = HuSprFuncCreate(fn_1_74AC, 0x63);
    HuSprGrpMemberSet((short)group, 0, (short)sprite);
    lbl_1_bss_A8 = ParameterInterpolation(0.0f);
}

// Releases the owned score displays.
SpriteManager::~SpriteManager() {}

// Creates the sprite manager if needed and replaces one player score display.
DisplayProvider *fn_1_6EA8(int index, int character, int side)
{
    if (MinigameSingleton<SpriteManager>::current() == 0) new SpriteManager;
    float x = side < 0 ? 80.0f : 496.0f;
    ScoreDisplaySlot *slot = &MinigameSingleton<SpriteManager>::getChecked().owners[index];
    *slot = new DisplayProvider(character, (int)x, 64);
    return slot->value;
}

// Starts the shared score display fade for phase 3 and clears the phase flag.
void SpriteManager::flagEnabled(int event)
{
    if (event == 3) {
        lbl_1_bss_A8.set(1.0f, lbl_1_bss_A4);
    }
    flags &= ~(1UL << event);
}
// Advances the shared score display fade.
void SpriteManager::update() { lbl_1_bss_A8.update(); }

// Draws the score displays from the engine sprite callback.
void fn_1_74AC(HuSprite_s *)
{
    fn_1_6910();
}

// Stores the initial display pointer.
ScoreDisplaySlot::ScoreDisplaySlot(DisplayProvider *initial) : value(initial) {}

// Starts interpolation from the current value to the supplied target.
void ParameterInterpolation::set(float end, int frames)
{
    set(value, end, frames);
}

// Sets the interpolation endpoints and duration, snapping immediately for zero frames.
void ParameterInterpolation::set(float start, float end, int frames)
{
    duration = frames;
    elapsed = 0;
    if (frames == 0) {
        value = end;
        from = end;
        to = end;
    } else {
        value = start;
        from = start;
        to = end;
    }
}

// Advances the linear interpolation by one frame and returns its current value.
float ParameterInterpolation::update()
{
    float result = value;
    if (elapsed <= duration) {
        double nextFrame = (double)++elapsed;
        double fraction = nextFrame / (double)(duration + 1);
        result = (float)(from * (1.0 - fraction) + to * fraction);
        value = result;
    }
    return result;
}
