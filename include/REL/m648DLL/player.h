#ifndef REL_M648_PLAYER_H
#define REL_M648_PLAYER_H

#include <vector>
#include "REL/framework/model.h"
#include "REL/framework/list.h"
#include "REL/m648DLL/multi_model_owner.h"

class FlyingObject;

/* One player: its object on the list at .bss 0x5FC. */
class CPlayer : public ListNode<CPlayer> {
public:
    typedef void (CPlayer::*Callback)();
    Vec position;
    Callback callback;
    u8 gameOver;
    s32 playerNo;
    s32 characterNo;
    s32 padNo;
    s32 currentMotion;
    s32 difficulty;
    OwnedModel characterModel;
    OwnedModel shapeModel;
    f32 scale;
    u8 finished;
    u8 selected;
    s32 measure;
    s32 strongRumble;
    s32 weakRumble;
    s32 finalMeasure;
    s32 rank;
    f32 height;
    s32 callbackState;
    s32 command;
    s32 decisionTimer;
    u8 active;
    s32 timer;
    MotionHookModels *spriteA;
    FlyingObject *spriteB;
    std::vector<s16> m_mot_id;
    CPlayer(s32 playerNo, s32 effect, bool nightMode);
    ~CPlayer();
    s16 addMotion(u32 dataNum);
    void fn_1_BD30();
    void fn_1_BDDC();
    void fn_1_BF94();
    void fn_1_C008();
    void fn_1_C038();
    void fn_1_C2E8(s32 motionId, u8 queued);
    void fn_1_C3A8();
    void fn_1_C498();
    void fn_1_C7BC();
    bool allPlayersFinished();
};

#endif
