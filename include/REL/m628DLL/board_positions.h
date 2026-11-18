#ifndef REL_M628DLL_BOARD_POSITIONS_H
#define REL_M628DLL_BOARD_POSITIONS_H
#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/vector3.h"
#include <vector>
// Tracks actor positions and ranks the board corners for computer input.
class BoardPositions : public GameObject, public MinigameSingleton<BoardPositions>
{
  public:
    Vector3 trackedPositions[4]; // Actor and player positions refreshed each frame.
    Vec velocity; // Velocity copied from the currently visited player.
    u8 lockedFlags[4];              // Lock flags copied from the players.
    u8 unk50;                       /* The game's code never reads this field. */
    s32 unk54;                      /* The game's code never reads this field. */
    Vec cornerPositions[4];         // Candidate board corner positions.
    s32 cornerStates[4];            // Rank assigned to each board corner.
    std::vector<GameObject *> list; // Tracked game objects used to update corner ranks.
    BoardPositions();
    virtual ~BoardPositions();
    virtual void update();
    // Returns the first tracked actor position.
    const Vector3 &firstTrackedPosition() const
    {
        return trackedPositions[0];
    }
    // Returns the rank of the selected corner.
    s32 cornerRank(s32 index) const
    {
        return cornerStates[index];
    }
    // Returns the stored player lock flag.
    u8 isLocked(s32 choice) const
    {
        return lockedFlags[choice];
    }
    // Returns the stored position.
    const Vec *getPosition(s32 choice) const
    {
        return &trackedPositions[choice];
    }
};
s32 fn_1_128C4(BoardPositions *owner, const Vec &point);

void fn_1_12474(BoardPositions *owner);
class ActPlayer;
ActPlayer *fn_1_12540(BoardPositions *owner, s32 mode);
void fn_1_12934(BoardPositions *owner);
// Selects the quadrant containing the supplied position.
s32 fn_1_1285C(BoardPositions *context, const Vec *position);

#endif
