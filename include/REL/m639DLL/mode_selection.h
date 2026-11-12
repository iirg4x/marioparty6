#ifndef M639_MODE_SELECTION_H
#define M639_MODE_SELECTION_H
// Which of the two endings this round plays, chosen once from the charges.
#include "REL/framework/singleton.h"

class M639StateOwner : public MinigameSingleton<M639StateOwner> {
public:
    unsigned char initialized;
    int selection;
    M639StateOwner();
    ~M639StateOwner();
    int fn_1_2CAC();
};

#endif
