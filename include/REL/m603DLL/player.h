#ifndef REL_M603DLL_PLAYER_H
#define REL_M603DLL_PLAYER_H

#include <vector>
#include "REL/framework/linknode.h"
#define M603_OWNED_MODEL_DECLARATION
#include "REL/m603DLL/model.h"

struct PlayerNode : Node {};

struct PlayerModelOwner : PlayerNode {
    enum { NUMBEROF_MOTION = 4 };
    Vec position;
    typedef void (PlayerModelOwner::*Callback)();
    Callback callback;
    u8 active;
    s32 playerIndex;
    s32 characterId;
    s32 controllerId;
    s32 currentMotion;
    s32 difficulty;
    f32 angle;
    s32 unknown3C;
    s32 unknown40;
    s32 callbackCount;
    OwnedModel firstModel;
    OwnedModel secondModel;
    u8 flag4C;
    u8 selected;
    u8 ready;
    u8 unknown4F;
    s32 recordedPlayerValue;
    s32 timer54;
    s32 motionChangeTimer;
    f32 randomMotion;
    bool alternate;
    std::vector<s16> m_mot_id;

    PlayerModelOwner(s32 playerIndex, bool alternate);
    ~PlayerModelOwner();
    HU3D_MOTIONID fn_1_61DC(unsigned int dataNum);
    void fn_1_6B20(s32 motionId, u8 queued);
    void fn_1_6D64();
    void fn_1_6E64();
    f32 fn_1_734C(bool *buttonPressed);
    void fn_1_759C();
    void fn_1_75A8();
    void fn_1_6BE0();
    void fn_1_6F60();
    void fn_1_70D8();
    void fn_1_72CC();
    PlayerModelOwner *next_node()
    {
        return static_cast<PlayerModelOwner *>(link0);
    }
};
void fn_1_59C8(void);
void fn_1_6258(void);

#endif
