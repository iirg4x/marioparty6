#ifndef M604_PLAYER_POSITION_HISTORY_H
#define M604_PLAYER_POSITION_HISTORY_H
#include "REL/framework/list_object.h"
#include "REL/m604DLL/border_vertex_work.h"
extern "C" {
#include "game/sprite.h"
}
struct PlayerPositionHistory : RequestObject {
    PlayerPositionHistory(Vec *position, s32 value);
    virtual ~PlayerPositionHistory();
    virtual void flagEnabled(int bit);
    virtual void update();
    s16 modelId;
    ANIMDATA *animation;
    s32 state;
    VectorHolder translation;
    f32 unknownScalarA;
    f32 rotation;
    f32 unknownScalarB;
    Vec scale;
    // fn_1_2CFC consumes 16 vertices and 16 pairs over 0x40..0x280.
    Vec vertices[16];
    BorderPair borders[16];
    s32 countdown;
    u8 active;
};
void fn_1_2DE8(PlayerPositionHistory *history, Vec *position);

#endif
