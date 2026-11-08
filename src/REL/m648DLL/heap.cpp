/* Target-consumed records, not claims of original class names or complete layouts. */

extern "C" {
#include "game/hu3d.h"
#include "game/hsfex.h"
#include "game/charman.h"
#include "game/mg/actman.h"
#include "game/mg/timer.h"
#include "dolphin/gx.h"
#include "dolphin/os.h"
}

#include "REL/m648DLL/framework/model.h"
#include "REL/m648DLL/framework/thread.h"
#include "REL/framework/heap.h"
#include "REL/m648DLL/motion_lists.h"

void *MemoryManager::allocate(unsigned long bytes) {
    unsigned long rounded=((bytes+31)&~31UL)+32; bytes=rounded;
    return allocateBlock(findFreeBlock(bytes),bytes);
}

HeapNode *MemoryManager::findFreeBlock(unsigned long bytes) {
    HeapNode **link=&first;
    while(*link) {
        HeapNode *node=*link; int remaining=node->size-bytes;
        if(remaining>=0) { *link=node->chain; return node; }
        link=&node->chain;
    }
    return 0;
}
