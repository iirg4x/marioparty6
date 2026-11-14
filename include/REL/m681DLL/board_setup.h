#ifndef M681_BOARD_SETUP_H
#define M681_BOARD_SETUP_H
#include "REL/framework/singleton.h"
struct ModelPair;
class ModelPairOwner {
public:
    ModelPair *value;
    // Starts with no owned model pair.
    ModelPairOwner() : value(0) {}
    ~ModelPairOwner();
};
class BoardController : public MinigameSingleton<BoardController> {
public:
    ModelPairOwner child;
    BoardController();
    ~BoardController();
};
void fn_1_EEC(BoardController *);
void fn_1_F30(BoardController *);
void fn_1_1014(BoardController *);
void fn_1_10D8(BoardController *);
#endif
