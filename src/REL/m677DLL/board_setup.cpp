// Creates the board view and starts the minigame.

#include "REL/framework/frame_durations.h"
#include "REL/m677DLL/board_view.h"

// Backing capacity in bytes for the minigame's C++ heap.
extern const s32 heapSize = 0xC0000;

// Creates and configures the board view.
extern "C" void ObjectSetup()
{
    BoardView *view = new BoardView();
    view->setup(1, 8192);
}
