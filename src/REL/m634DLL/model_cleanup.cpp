#include <vector>
#include "REL/m634DLL/game_manager.h"
#include "REL/framework/model.h"

// Deletes the owned models and clears their list.
void fn_1_CF58(std::vector<OwnedModel *> *models)
{
    OwnedModel **first = models->begin();
    OwnedModel **last = models->end();
    while (first != last)
        delete *first++;
    models->clear();
}
