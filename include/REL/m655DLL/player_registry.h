#ifndef REL_M655DLL_PLAYER_REGISTRY_H
#define REL_M655DLL_PLAYER_REGISTRY_H
#include <vector>
#include "REL/framework/range.h"
#include "dolphin/types.h"
struct PlayerWork;
typedef std::vector<PlayerWork *> PlayerRegistry;
typedef void (*ComparePlayers)(PlayerWork *, PlayerWork *);
s32 fn_1_7CA8(void);
s32 fn_1_7AC4(void);
template <class Players> void fn_1_A8AC(Players *players,
                          ComparePlayers compare)
{
    typename Players::iterator first;
    typename Players::iterator current;
    typename Players::iterator endCopy;
    typename Players::iterator pairStart;
    typename Players::iterator last;

    first = players->begin();
    endCopy = players->end();
    last = endCopy - 1;
    while (first != last) {
        pairStart = first;
        first += 1;
        current = first;
        while (current != endCopy) {
            compare(*pairStart, *current++);
        }
    }
}
#endif
