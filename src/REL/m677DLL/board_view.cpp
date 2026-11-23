// Runs the board sequence through rope selection and the resulting event.

#include "msm_se.h"
#include "REL/m677DLL/event_base.h"
#include "REL/m677DLL/board_view.h"
#include "REL/m677DLL/seer_terror_camera.h"
#include "REL/m677DLL/host_character.h"
#include "REL/m677DLL/message_sequence_window.h"
#include "REL/m677DLL/seer_terror_scene.h"
#include "REL/framework/event.h"
#include "REL/framework/event_manager.h"
#include "REL/framework/frame_durations.h"
#include "REL/m677DLL/game_constants.h"
#include "REL/framework/game_object_man.h"
#include "REL/framework/random.h"
#include "REL/framework/sound.h"
#include "REL/framework/timer.h"
#include "REL/framework/world_units.h"

extern "C" {
#include "game/audio.h"
#include "game/window.h"
#include "game/wipe.h"
#include "messdir_enum.h"
#include "msm_stream.h"
}

// Keeps the event, object, selection and timer state for the board.
class BoardResources {
public:
    // Registers and unregisters the event manager through its lifetime.
    EventManager eventManager;
    // Owns the framework game object manager for the board lifetime.
    GameObjectManager gameObjects;
    SeerTerrorScene selection;
    TimerOwner timer;
    unsigned long result;

    // Initializes the board timer.
    BoardResources() : timer(0) {}
};

// Registers the board view to receive events.
BoardView::BoardView()
    : ThreadEntry(0x8000), resources(new BoardResources())
{
    registerChannel(0);
}

// Releases the board view's owned resources.
BoardView::~BoardView()
{
}

// Creates the objects and opens the message system for this board.
void BoardView::setupObjects(OMOBJMAN *)
{
    HuWinInit(0);
    RandomSeed(OSGetTick());
    SeerTerrorScene *selection = &resources->selection;
    selection->createCamera();
    selection->createMap();
    selection->createHostCharacter();
    selection->createPlayer();
    selection->loadParticleAnimations();
    soundSetCameraLayout(0);
}

// Resumes the sequence, dispatches events, and updates the game objects.
void BoardView::updateGame()
{
    resume(0);
    process_event_queue();
    updateGameObjects();
}

// Runs the board sequence from the opening wipe through the rope event.
void BoardView::run(void *)
{
    fn_1_17764();
    wipeIn();
    while (WipeCheckIn()) {
        suspend(0);
    }

    int stream = HuAudBGMPlay(MSM_STREAM_SEER_TERROR);
    fn_1_177B8();
    SeerTerrorCamera::getChecked().transitionToPreset(1, 3.0f);
    while (!SeerTerrorCamera::getChecked().isTransitionDone()) {
        suspend(0);
    }

    showStart();
    sendEvent(0, 0, 0.0f);
    fn_1_17A44();
    fn_1_17BF0();
    while (!(u8)fn_1_16EFC()) {
        suspend(0);
    }

    fn_1_16FC4();
    HostCharacter &model = HostCharacter::getChecked();
    model.playBeforeResultMessageMotion();
    sendEvent(0, 5, 0.0f);
    showFinish();
    HuAudSStreamFadeOut(stream, 100);
    while (!SeerTerrorCamera::getChecked().isTransitionDone() || !messageDone()) {
        suspend(0);
    }

    stream = HuAudBGMPlay(MSM_STREAM_SEER_TERROR);
    sendEvent(0, 6, 0.0f);
    fn_1_17DB0();
    while (!(u8)fn_1_17060()) {
        suspend(0);
    }
    HuAudSStreamFadeOut(stream, 1000);
    requestExit();
    for (;;) {
        suspend(reinterpret_cast<void *>(-1));
    }
}

// Selects the event and stores its result message before rope selection.
void BoardView::fn_1_17764()
{
    int previousIndex = 0;
    int choiceIndex = 0;
    resources->result = resources->selection.startSelectedEvent(previousIndex - 1, choiceIndex);
}

// Plays the introduction and displays its messages.
void BoardView::fn_1_177B8()
{
    HostCharacter *model = &HostCharacter::getChecked();
    model->fn_1_188BC(MSM_SE_GUIDE_KOOPA_REACT);
    std::vector<u32> sequence(2);
    sequence.clear();
    sequence.push_back(MESSNUM(MESS_MESSAGE_677, 0));
    sequence.push_back(MESSNUM(MESS_MESSAGE_677, 1));
    MessageSequenceWindow message(sequence);
    while (message.run()) {
        suspend(0);
    }
    model->fn_1_18928();
}

// Starts rope selection and waits for the character's choice.
void BoardView::fn_1_17A44()
{
    HostCharacter *model = &HostCharacter::getChecked();
    resources->selection.createRopes();
    sendEvent(0, 1, 0.5f);
    for (int index = 0; index < secondsToFrames<long>(3); ++index) {
        suspend(0);
    }
    model->fn_1_188BC(MSM_SE_GUIDE_KOOPA_REACT);
    {
        MessageSequenceWindow message(MESSNUM(MESS_MESSAGE_677, 2));
        while (message.run()) {
            suspend(0);
        }
    }
    model->fn_1_18928();
    sendEvent(0, 2, 0.0f);
    TimerOwner *timer = &resources->timer;
    timer->setParameters(tenSeconds, 0, 0);
    timer->setMode(1);
    while (!(u8)fn_1_16EFC()) {
        if (resources->timer.isDone()) {
            sendEvent(0, 4, 0.0f);
        }
        suspend(0);
    }
}

// Plays the sequence and messages after the rope choice.
void BoardView::fn_1_17BF0()
{
    HostCharacter *model = &HostCharacter::getChecked();
    model->fn_1_188BC(MSM_SE_GUIDE_KOOPA_QUIZ_CHEER);
    std::vector<u32> sequence(2);
    sequence.clear();
    sequence.push_back(MESSNUM(MESS_MESSAGE_677, 3));
    sequence.push_back(MESSNUM(MESS_MESSAGE_677, 4));
    MessageSequenceWindow message(sequence);
    int remaining = sequence.size();
    while (message.run()) {
        int next = message.queuedMessageCount();
        if (remaining > 1 && next <= 1) {
            model->fn_1_18978();
        }
        remaining = next;
        suspend(0);
    }

    int count = 0;
    while (count < secondsToFrames<long>(1)) {
        suspend(0);
        ++count;
    }
    model->fn_1_18928();
}

// Waits for the model and presents the result message.
void BoardView::fn_1_17DB0()
{
    HostCharacter *model = &HostCharacter::getChecked();
    while (!model->shiftDone()) {
        suspend(0);
    }
    model->playBetweenMotionSound();
    {
        MessageSequenceWindow message(resources->result);
        s32 frames = 0;
        while (message.run()) {
            if (++frames > secondsToFrames<long>(10)) {
                message.beginClosing();
            }
            suspend(0);
        }
    }
    model->playAfterResultMessageMotion();
    while (!model->isMotionEnd()) {
        suspend(0);
    }
}

// Leaves completion handling to the surrounding board sequence.
void BoardView::finishGame()
{
}

// Plays the rope-selection cue on event 1 and ends the countdown on event 4.
void BoardView::receive(const EventPacket *event)
{
    switch (event->type) {
    case 1:
        soundPlay(MSM_SE_MG677_ROPE_SELECTION_START);
        break;
    case 4: {
        resources->timer.start();
        break;
    }
    }
}
