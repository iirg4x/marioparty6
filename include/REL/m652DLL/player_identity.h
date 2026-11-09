#ifndef M652_PLAYER_IDENTITY_H
#define M652_PLAYER_IDENTITY_H
extern "C" {
#include "game/gamework.h"
}
class Player {
public:
    s32 playerNo;
    Player(int);
    float fn_1_37688();
    bool isComputer() { return lbl_80265728[playerNo].type != 0; }
    int padNo();
    void fn_1_1B338(s32 type, s32 duration);
    int fn_1_36C6C();
    int fn_1_36D1C();
    int fn_1_36DCC();
    float fn_1_36E7C();
    float fn_1_36F44();
    float fn_1_3700C();
    float fn_1_370D4();
    float fn_1_3719C();
    float fn_1_37264();
    int fn_1_3732C();
    int fn_1_373D8();
    int fn_1_37484();
    int fn_1_37530();
    int fn_1_375DC();
};
#endif
