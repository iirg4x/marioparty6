// Runs the board sequence through rope selection and the resulting event.

#ifndef REL_SEER_TERROR_BOARD_VIEW_H
#define REL_SEER_TERROR_BOARD_VIEW_H

#include "REL/framework/event_packet.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/thread.h"

#include <memory>

class BoardResources;

// Main minigame object, with its worker thread and event listener.
class BoardView : public MinigameCore, public ThreadEntry, public EventListener {
public:
    BoardView();
    virtual ~BoardView();

    virtual void setupObjects(OMOBJMAN *);
    virtual void updateGame();
    virtual void finishGame();
    virtual void receive(const EventPacket *packet);
    virtual void run(void *argument);

private:
    void fn_1_17764();
    void fn_1_177B8();
    void fn_1_17A44();
    void fn_1_17BF0();
    void fn_1_17DB0();

    std::auto_ptr<BoardResources> resources;
};

#endif
