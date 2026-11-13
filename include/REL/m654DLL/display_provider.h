#ifndef M654_DISPLAY_PROVIDER_H
#define M654_DISPLAY_PROVIDER_H

#include <vector>
#include "REL/framework/game_object.h"
#include "REL/m654DLL/sprite_parameters.h"

struct DisplayProvider : GameObject {
    short scorebox;
    ANIMDATA *firstAnimation, *secondAnimation;
    std::vector<SpriteParameters> firstList, secondList;
    int activeCount;
    DisplayProvider(int character, int offsetX, int y);
    virtual ~DisplayProvider();
    virtual void update();
    void fn_1_6AEC();
    int fn_1_66F4();
};

#endif
