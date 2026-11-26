// The minigame's scene object: the pieces on the field, their places and the shared tables.
#ifndef REL_M634DLL_SCENE_H
#define REL_M634DLL_SCENE_H

#include <vector>
#include "REL/m634DLL/scene_position.h"
#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"

extern "C" {
#include "dolphin/mtx.h"
}

struct SequenceOwner;

struct ActPlayer;

struct Scene : GameObject, MinigameSingleton<Scene> {
    float firstRatio;
    float secondRatio;
    int firstCount;
    int secondCount;
    int indices[4];
    std::vector<SequenceOwner *> firstPieces;
    std::vector<SequenceOwner *> secondPieces;
    std::vector<Vector3> firstPositions;
    std::vector<Vector3> secondPositions;
    std::vector<Vector3> firstHomes;
    std::vector<Vector3> secondHomes;
    std::vector<ActPlayer *> players;
    std::vector<Vector3> points;
    std::vector<int> records[6];

    s32 fn_1_ED70();
    s32 fn_1_ED78();
    u8 fn_1_ED80(s32 index);
    u8 fn_1_EDE8(s32 index);
    s32 fn_1_EFE0(s32 index);
    void fn_1_EFF0(s32 index, s32 value);

    Scene();
    virtual ~Scene();
    virtual void update();
};


ActPlayer *fn_1_EC8C(Scene *object, s32 mode);


s32 fn_1_F0A8(Scene *owner, s32 entryIndex, s32 *position);


Vector3 fn_1_F044(Scene *owner, s32 index);


void fn_1_F13C(Scene *owner, s32 *listIndex, s32 *resultIndex, s32 value);


s32 fn_1_F23C(Scene *owner, const Vec *position, s32 *matchingGroupIndex, s32 *matchingEntryIndex,
    s32 selectedIndex);


void fn_1_F4F8(Scene *owner, const Vec *position, s32 *motionIndices, s32 *motionIndexCount);


u8 fn_1_F72C(Scene *owner, s32 listIndex, s32 entryIndex, s32 value);

#include "REL/m634DLL/scene_points.h"
#endif
