#include "REL/m654DLL/sprite_manager.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/game_object_man.h"
// Each player's score display and the sprite manager that draws them over the scene.
#include "REL/m654DLL/display_provider.h"
#include "REL/m654DLL/camera_node_owner.h"
#include "REL/m654DLL/parameter_interpolation.h"
#include "REL/m654DLL/sprite_draw.h"
extern "C" {
#include "game/data.h"
#include "game/gamemes.h"
#include "game/sprite.h"
}
#include "REL/m654DLL/scene_owner.h"

extern "C" {

s32 lbl_1_bss_98 = minutesToFrames(1L);
float lbl_1_bss_9C = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
s32 lbl_1_bss_A0 = (s32)secondsToFrames(0.5);
s32 lbl_1_bss_A4 = (s32)secondsToFrames(0.2);
}

extern const int lbl_1_rodata_2D8[2] = {0x9B001E, 0x9B001F};

DisplayProviderDP::DisplayProviderDP(int width, int offsetX, int y)
    : GameObject(2),
      scorebox(MgScoreBoxCreateChar(0x80, 0x30, (short)width)),
      firstAnimation(HuSprAnimRead(fn_80008DD0(lbl_1_rodata_2D8[0], 0x10000000, 2))),
      secondAnimation(HuSprAnimRead(fn_80008DD0(lbl_1_rodata_2D8[1], 0x10000000, 2))),
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
        firstParameters.scaleX = 1.0f;
        firstParameters.scaleY = 1.0f;
        firstList.push_back(firstParameters);

        SpriteParameters secondParameters(secondAnimation);
        secondParameters.x = x;
        secondParameters.y = (float)y;
        secondParameters.scaleX = 4.0f;
        secondParameters.scaleY = 0.0f;
        secondList.push_back(secondParameters);
        x += 24.0f;
        index++;
    }
}

DisplayProviderDP::~DisplayProviderDP() {}

int DisplayProviderDP::fn_1_66F4()
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

void DisplayProviderDP::update()
{
    double tpLevel = lbl_1_bss_A8.value;
    MgScoreBoxTPLvlSet((short)scorebox, tpLevel);
    if (activeCount > 0) {
        SpriteParameters *checkedLastRecord = &secondList.back();
        if (!(checkedLastRecord->scaleX <= 1.0)) {
            int index = 0;
            while (index < activeCount) {
                SpriteParameters *record = &secondList[index];
                if (!(record->scaleX <= 1.0)) {
                    record->scaleX = (float)((double)record->scaleX - (3.0 / (double)lbl_1_bss_A0));
                    record->scaleY = (float)((double)record->scaleY + (1.0 / (double)lbl_1_bss_A0));
                    if (record->scaleX <= 1.0) {
                        record->scaleX = 1.0f;
                        record->scaleY = 1.0f;
                    }
                }
                index++;
            }
        }
    }
}

extern "C" {
void fn_1_6910();
}
void fn_1_81B4();

void fn_1_6910()
{
    std::vector<GameObject *> list(2);
    collectGameObjects(2, &list);
    int count = list.size();
    fn_1_81B4();
    for (int index = 0; index < count; index++) {
        static_cast<DisplayProviderDP *>(list[index])->fn_1_6AEC();
    }
}

inline void DisplayProviderDP::fn_1_6AEC()
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

extern "C" {
struct HuSprite_s;
void fn_1_74AC(HuSprite_s *);
}
inline ScoreDisplaySlot::~ScoreDisplaySlot() { delete value; }

SpriteManager::SpriteManager()
{
    short group = HuSprGrpCreate(1);
    short sprite = HuSprFuncCreate(fn_1_74AC, 0x63);
    HuSprGrpMemberSet((short)group, 0, (short)sprite);
    {
        ParameterInterpolation transition;
        lbl_1_bss_A8.duration = transition.duration;
        lbl_1_bss_A8.elapsed = transition.elapsed;
        lbl_1_bss_A8.value = transition.value;
        lbl_1_bss_A8.from = transition.from;
        lbl_1_bss_A8.to = transition.to;
    }
}

SpriteManager::~SpriteManager() {}

static inline SpriteManager *Existing6EA8() { return MinigameSingleton<SpriteManager>::self; }

GameObject *fn_1_6EA8(int index, int character, int side)
{
    if (Existing6EA8() == 0) new SpriteManager;
    float x = side < 0 ? 80.0f : 496.0f;
    ScoreDisplaySlot *slot = &MinigameSingleton<SpriteManager>::getChecked().owners[index];
    *slot = new DisplayProviderDP(character, (int)x, 64);
    return slot->value;
}

void SpriteManager::flagEnabled(int event)
{
    if (event == 3) {
        lbl_1_bss_A8.fn_1_75F8(1.0f, lbl_1_bss_A4);
    }
    flags &= ~(1UL << event);
}
void SpriteManager::update() { lbl_1_bss_A8.fn_1_7670(); }

void fn_1_74AC(HuSprite_s *)
{
    fn_1_6910();
}

ScoreDisplaySlot::ScoreDisplaySlot(GameObject *initial) : value(initial) {}

void ParameterInterpolation::fn_1_75F8(float end, int frames)
{
    fn_1_763C(value, end, frames);
}

void ParameterInterpolation::fn_1_763C(float start, float end, int frames)
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

float ParameterInterpolation::fn_1_7670()
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
