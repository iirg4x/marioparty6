#ifndef M681_BOARD_SETUP_H
#define M681_BOARD_SETUP_H
#include "REL/framework/minigame_singleton.h"
struct ModelPair;
class Owner47C { public: ModelPair *value; Owner47C() : value(0) {} ~Owner47C(); };
class BoardController : public MinigameSingleton<BoardController> {
public:
    Owner47C child;
    BoardController();
    ~BoardController();
};
#endif
