// Controls the player's rope selection, motion, and reactions.

#ifndef REL_SEER_TERROR_PLAYER_WORK_H

#define REL_SEER_TERROR_PLAYER_WORK_H

#include <dolphin/types.h>
#include <memory>
#include <list>
#include "datanum/charmot.h"
#include "REL/framework/chara_motion.h"
#include "REL/framework/singleton.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/range.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/p_chara.h"
#include "REL/m677DLL/path_follower.h"
#include "REL/m677DLL/icon_renderer.h"

class Model;

#include "REL/m677DLL/material_colors.h"
#include "REL/m677DLL/player_input_interface.h"

// Updates the controlled character, its route and its active input action.
struct PlayerWork : MinigameSingleton<PlayerWork>, PlayerConfigSelection, GameObject {

    virtual ~PlayerWork();

    virtual void update();

    EventHandlerManager events;

    typedef void (PlayerWork::*State)();
    State state;
    // Selects the next character action handler.
    void setPlayerState(State next)
    {
        state = next;
    }

    // Returns a pointer to the path follower's current position.
    const Vec *getPositionPtr()
    {
        return static_cast<const Vec *>(follower.getPosition());
    }
    // Returns a reference to the path follower's current position.
    Vec &getPosition()
    {
        return follower.position;
    }
    PathFollower follower;

    Vec offset;

    Range<float> heading;

    float targetHeading;

    std::auto_ptr<IconRenderer> motionEffect;

    std::auto_ptr<MaterialColors> materialColors;

    unsigned char ropeChosen;

    Range<int> ropeSelection;

    unsigned char updatingRopePull;

    FrameInterpolator<float> transition;

    std::auto_ptr<PlayerInputInterface> action;

    PlayerWork(int, const Vec &);

    // Returns the currently selected rope index.
    int selectedRope();

    void fn_1_1CEA4();
    // The idle state is the callback that performs no action.
    bool isIdle() const
    {
        State idle = &PlayerWork::fn_1_1CEA4;
        return state == idle;
    }
    void fn_1_1CEFC(float value);

    void fn_1_1D06C();

    void fn_1_1D55C();

    void fn_1_1D70C(const EventPacket &);
    void fn_1_1D1E0(const EventPacket &);
    void fn_1_1D748(const EventPacket &);
};

extern "C" int fn_1_7B6C(Range<int> *);

// Returns the current rope selection.
inline int PlayerWork::selectedRope()
{
    return fn_1_7B6C(&ropeSelection);
}

extern "C" void fn_1_1CEA8(PlayerWork *character, float heading, bool applyImmediately);

extern "C" void fn_1_1CF40(PlayerWork *character, bool enabled);
extern "C" void fn_1_1CFB4(PlayerWork *character, bool enabled);
void fn_1_1CFEC(PlayerWork *character);
extern "C" void fn_1_1D27C(PlayerWork *character);
void enqueuePlayerWaypoint(PlayerWork *character, const Vec *target, float speed);
void enqueuePlayerRoute(PlayerWork *character, std::vector<Vec> *route, float speed);
extern "C" void fn_1_1D0B4(PlayerWork *character, int direction);
extern "C" void fn_1_1D144(PlayerWork *character);
void fn_1_1CD74(PlayerWork *character);
void fn_1_1D684(PlayerWork *character);
Vec *fn_1_1D618(PlayerWork *character, Vec *position);
extern "C" float playerFloorHeight;

// Starts the character motion shared by the stage rotation and falling tarai events.
inline void startEventCharacterMotion()
{
    PlayerWork &character = PlayerWork::getChecked();
    shiftCharacterMotion(&static_cast<MotionOwner &>(character), CHARMOT_HSF_c000m1_325,
                         16.0f, 0.0f, 0);
}

#endif
