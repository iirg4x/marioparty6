#ifndef REL_M652DLL_CAMERA_SPLIT_H
#define REL_M652DLL_CAMERA_SPLIT_H

#include <vector>
#include "dolphin/types.h"
#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"

/* Splits the screen between the two players' cameras. */
class CameraSplit : public GameObject, public MinigameSingleton<CameraSplit> {
public:
    CameraSplit();
    virtual ~CameraSplit();
    virtual void update();

    u8 modified : 1;
    u8 unknown10 : 7;
    std::vector<int> positions;
    int screenOffset;
};

#endif
