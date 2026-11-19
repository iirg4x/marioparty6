#include "REL/framework/frame_durations.h"
#include "REL/m605DLL/board_view.h"
extern "C" const s32 heapSize = 0x40000;
// Creates the board view and starts its object manager.
extern "C" void ObjectSetup()
{
    BoardView *root = new BoardView;
    root->setup(1, 0x2000);
}
