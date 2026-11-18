#include "msm_stream.h"
#include "REL/framework/phase_duration_statics.h"
#include "REL/framework/event.h"
#include "REL/framework/event_manager.h"
#include "REL/framework/singleton.h"
#include "dolphin/os.h"
#include "REL/m610DLL/playing_piece.h"
#include "REL/m610DLL/debug_geometry_calls.h"
#include "dolphin/types.h"

#include "dolphin/mtx.h"

#include "REL/m610DLL/effect_manager.h"

#include "REL/m610DLL/frame_count.hpp"

#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"

#include "REL/framework/random.h"

#include "REL/framework/game_object_man.h"

#include "REL/m610DLL/scene_owner.hpp"

#include "game/hu3d.h"

#include "REL/m610DLL/camera_transition.h"

#include "REL/framework/p_info.h"

extern "C"
{
#include "game/mg/actman.h"
}

#include "REL/m610DLL/scene_callback.h"

#include "REL/m610DLL/rigid_box.h"
#include "REL/m610DLL/moving_model_owner.h"
#include "REL/m610DLL/segment_spawner.h"

extern "C"
{
#include "game/wipe.h"
}

extern "C"
{
#include "game/audio.h"
}

#include "REL/m610DLL/geom.h"

#include <new>

/* Place four registered spheres around the gameplay area. */
OwnedSceneChild::OwnedSceneChild()
{
    spheres[0].center.x = -800.0f;
    spheres[0].center.y = 0.0f;
    spheres[0].center.z = -2600.0f;
    spheres[1].center.x = 800.0f;
    spheres[1].center.y = 0.0f;
    spheres[1].center.z = -2600.0f;
    spheres[2].center.x = -800.0f;
    spheres[2].center.y = 0.0f;
    spheres[2].center.z = -1000.0f;
    spheres[3].center.x = 800.0f;
    spheres[3].center.y = 0.0f;
    spheres[3].center.z = -1000.0f;
}

/* Seed randomness and create the scene objects and game timer. */
SceneObjectOwner::SceneObjectOwner()
    : ThreadEntry(0x10000), remainingFrames(fn_1_10E08(5) - secondsToFrames<long>(30)), timer(0)
{
    RandomSeed(OSGetTick());
    childOwner = new OwnedSceneChild;
}

/* Release the scene owner and its child objects. */
SceneObjectOwner::~SceneObjectOwner()
{
}

/* Create the camera, scenery, obstacles and four playing pieces. */
void SceneObjectOwner::setupObjects(OMOBJMAN *)
{
    new CameraTransitionState;
    EffectManager *state = &MinigameSingleton<EffectManager>::getChecked();
    state->fn_1_EE3C();
    state->fn_1_EFC4();
    state->fn_1_F0BC();
    state->fn_1_ECE8();
    for (int entry = 0; entry < 4; ++entry)
        state->fn_1_E9B8(entry);
}

/* Submit the debug ground grid lines. */
void fn_1_10378(void)
{
    Vec xStart, xEnd, zStart, zEnd;
    xStart.y = (100.0f * 0.3f);
    xEnd.y = (100.0f * 0.3f);
    zStart.y = (100.0f * 0.3f);
    zEnd.y = (100.0f * 0.3f);
    xStart.z = -5000.0f;
    xEnd.z = 0.0f;
    zStart.x = -2500.0f;
    zEnd.x = 2500.0f;
    for (int coordinate = -12; coordinate <= 12; ++coordinate)
    {
        float x = coordinate * cmToWorld(250.0);
        xStart.x = x;
        xEnd.x = x;
        fn_1_1C318(&xStart, &xEnd, 0xFF0000FF);
        float z = -(coordinate + 6) * cmToWorld(250.0);
        zStart.z = z;
        zEnd.z = z;
        fn_1_1C318(&zStart, &zEnd, 0xFF0000FF);
    }
}

/* Resume the scene thread and dispatch events, object updates and contacts. */
void SceneObjectOwner::updateGame()
{
    ThreadEntry::resume(0);
    process_event_queue();
    updateGameObjects();
    fn_1_B914(fn_1_107E0);
}

/* Form the square and sweep corners, then test the two projected regions. */
bool RigidBox::fn_1_10544(const MovingModelOwner *square)
{
    const Vec *centerPointer = &square->registration.center;
    const Vec &center = *centerPointer;
    float radius = 200.0f;
    QuadRegion corners;
    float cornerx0, cornery0;
    cornery0 = center.z - radius;
    cornerx0 = center.x - radius;
    corners.points[0].x = cornerx0;
    corners.points[0].y = cornery0;
    float cornerx1, cornery1;
    cornery1 = center.z - radius;
    cornerx1 = center.x + radius;
    corners.points[1].x = cornerx1;
    corners.points[1].y = cornery1;
    float cornerx2, cornery2;
    cornery2 = center.z + radius;
    cornerx2 = center.x + radius;
    corners.points[2].x = cornerx2;
    corners.points[2].y = cornery2;
    float cornerx3, cornery3;
    cornery3 = center.z + radius;
    cornerx3 = center.x - radius;
    corners.points[3].x = cornerx3;
    corners.points[3].y = cornery3;
    /* Normalize the edge direction; the result is discarded. */
    ProjectionVector direction = this->corners[0].position - this->corners[1].position;
    float distance = direction.length();
    direction /= distance;
    QuadRegion sweepPoints;
    float sweepx0, sweepy0;
    sweepy0 = this->corners[0].position.z;
    sweepx0 = this->corners[0].position.x;
    sweepPoints.points[0].x = sweepx0;
    sweepPoints.points[0].y = sweepy0;
    float sweepx1, sweepy1;
    sweepy1 = this->corners[1].position.z;
    sweepx1 = this->corners[1].position.x;
    sweepPoints.points[1].x = sweepx1;
    sweepPoints.points[1].y = sweepy1;
    float sweepx2, sweepy2;
    sweepy2 = this->corners[3].position.z;
    sweepx2 = this->corners[3].position.x;
    sweepPoints.points[2].x = sweepx2;
    sweepPoints.points[2].y = sweepy2;
    float sweepx3, sweepy3;
    sweepy3 = this->corners[2].position.z;
    sweepx3 = this->corners[2].position.x;
    sweepPoints.points[3].x = sweepx3;
    sweepPoints.points[3].y = sweepy3;
    return fn_1_10ED4(&corners, &sweepPoints);
}

/* Dispatch box, obstacle and segment collision pairs by their registered categories. */
void fn_1_107E0(s32 mode, void *owner, s32 kind, void *payload)
{
    switch (mode)
    {
    case 0:
        if (kind == 0)
        {
            static_cast<RigidBox *>(owner)->fn_1_7DB8(static_cast<RigidBox *>(payload));
            return;
        }
        if (kind == 1)
        {
            RigidBox *contactOwner = static_cast<RigidBox *>(owner);
            MovingModelOwner *contactPayload = static_cast<MovingModelOwner *>(payload);
            if (!contactOwner->fn_1_10544(contactPayload))
                return;
            if (contactPayload->velocity.y < 0.0f)
            {
                if (!contactOwner->isSquashing())
                    contactOwner->fn_1_9F84();
                contactOwner->squashTime.set(contactOwner->squash, 0);
                contactOwner->squashTime.set(0.0f, 4);
                contactOwner->squashing = true;
            }
            else if (contactOwner->isSquashing())
            {
                if (!contactOwner->isSquashing())
                    contactOwner->fn_1_9F84();
                contactOwner->squashTime.set(contactOwner->squash, 0);
                contactOwner->squashTime.set(0.0f, 4);
                contactOwner->squashing = true;
            }
            else
            {
                contactOwner->fn_1_77E0(&contactPayload->collisionSphere());
                contactOwner->fn_1_9FB0(0x66E);
            }
            return;
        }
        else if (kind == 3)
        {
            RigidBox *segmentOwner = static_cast<RigidBox *>(owner);
            SegmentContactPayload *segment = static_cast<SegmentContactPayload *>(payload);
            segmentOwner->fn_1_7CBC(&segment->firstPosition, &segment->secondPosition, 25.0f);
        }
        break;
    case 1:
        break;
    }
}

/* Player index selected by the finish check. */
extern "C"
{
    int lbl_1_data_418 = -1;
}

/* Record the first playing piece to cross the finish depth as the winner. */
bool fn_1_10984(void)
{
    for (int playerIndex = 0; playerIndex < 4; playerIndex++)
    {
        if (lbl_1_bss_54[playerIndex]->fn_1_1E98())
        {
            lbl_1_bss_54[playerIndex]->fn_1_1F70();
            MinigameCore::addWinner(playerIndex);
            lbl_1_data_418 = playerIndex;
            return true;
        }
    }
    return false;
}

/* Run the introduction, timed race, finish transition and effects presentation. */
void SceneObjectOwner::run(void *)
{
    int introFrame;
    unsigned char timedOut;
    int postFadeFrame;
    int wipeFrame;
    const int wipeDuration = 60;
    int playerIndex;
    int stream;

    lbl_1_bss_68 = 0;
    sendEvent(2, 0, 0.0f);
    MinigameCore::wipeIn();
    introFrame = 0;
    while (introFrame < secondsToFrames<long>(3))
    {
        ++introFrame;
        ThreadEntry::suspend(0);
    }

    sendEvent(2, 1, 0.0f);
    MinigameCore::showStart();
    stream = HuAudSStreamPlay(MSM_STREAM_PUMPED_UP); /* Start the minigame gameplay music stream. */
    while (MinigameCore::messageDone() == 0)
        ThreadEntry::suspend(0);

    timedOut = 0;
    sendEvent(2, 2, 0.0f);
    for (;;)
    {
        if (!fn_1_10984())
        {
            if (remainingFrames.value <= 0 && timer.isDone() != 0)
            {
                timedOut = 1;
            }
            else
            {
                if ((remainingFrames.value > 0 && --remainingFrames.value == 0) != false)
                {
                    timer.setParameters(secondsToFrames<long>(30), secondsToFrames<long>(0), 0);
                    timer.setMode(1);
                }
                ThreadEntry::suspend(0);
                continue;
            }
        }
        break;
    }

    if (remainingFrames.value <= 0)
        timer.start();
    sendEvent(2, 3, 0.0f);
    MinigameCore::showFinish();
    HuAudSStreamFadeOut(stream, 100);
    HuAudFXAllStop();
    lbl_1_bss_68 = 1;
    setRumbleEnabled(0);
    for (postFadeFrame = 0; postFadeFrame < secondsToFrames<long>(2); ++postFadeFrame)
        ThreadEntry::suspend(0);

    if (!timedOut)
    {
        WipeColorSet(0xFF, 0xFF, 0xFF);
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, wipeDuration);
        for (wipeFrame = 0; wipeFrame < wipeDuration; ++wipeFrame)
            ThreadEntry::suspend(0);
        for (playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            delete lbl_1_bss_54[playerIndex];
        }
        EffectManager &effects = MinigameSingleton<EffectManager>::getChecked();
        effects.fn_1_EBAC(lbl_1_data_418);
        lbl_1_bss_64 = 1;
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, wipeDuration);
    }

    sendEvent(2, 4, 0.0f);
    HuAudJinglePlay(timedOut ? MSM_STREAM_BAD_NEWS : MSM_STREAM_WINNING);
    MinigameCore::showWinners();
    while (MinigameCore::messageDone() == 0)
        ThreadEntry::suspend(0);
    MinigameCore::requestExit();
    for (;;)
        ThreadEntry::suspend(reinterpret_cast<void *>(-1));
}

/* Leave the scene finish hook empty. */
void SceneObjectOwner::finishGame()
{
}

/* Return the number of gameplay frames for the configured duration. */
int fn_1_10E08(long value)
{
    return secondsToFrames(value * 60);
}

/* The four active player actors. */
extern "C"
{
    PlayerPiece *lbl_1_bss_54[4];
}

/* Request the post-finish camera pose. */
extern "C"
{
    int lbl_1_bss_64;
}

/* End-of-game state used to suppress obstacle sounds. */
extern "C"
{
    unsigned char lbl_1_bss_68;
}
