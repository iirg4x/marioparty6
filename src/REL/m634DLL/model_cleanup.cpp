#include <vector>
#include "REL/framework/model.h"

void fn_1_CF58(std::vector<OwnedModel *> *models)
{
    OwnedModel **first = models->begin();
    OwnedModel **last = models->end();
    while (first != last) delete *first++;
    models->clear();
}
