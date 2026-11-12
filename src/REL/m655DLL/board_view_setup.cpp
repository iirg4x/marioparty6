#include "REL/framework/frame_durations.h"
#include "REL/framework/frame_time.h"
#include "dolphin/types.h"
#include "REL/m655DLL/board_view.h"

/* Capacity of the board heap. */
extern const s32 heapSize = 0xF0000;

extern "C" void ObjectSetup()
{
    BoardView *root = new BoardView;
    root->setup(1, 8192);
}
