#ifndef REL_M603DLL_PLAYER_H
#define REL_M603DLL_PLAYER_H

#include <vector>
#include "REL/framework/list.h"
#include "REL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
}

/* Own a player model, input state and photography callbacks. */
struct PlayerModelOwner : ListNode<PlayerModelOwner> {
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
    s32 unk3C; /* No code names this field. */
    s32 unk40; /* No code names this field. */
    s32 callbackCount;
    OwnedModel firstModel;
    OwnedModel secondModel;
    u8 flag;
    u8 selected;
    u8 ready;
    u8 unk4F; /* No code names this field. */
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
};
void fn_1_59C8(void);
void fn_1_6258(void);

void fn_1_794C(void);
void fn_1_6690(void);
void fn_1_66F4(void);
bool fn_1_62DC(void);
void fn_1_67A0(void);
void fn_1_6884(void);
void fn_1_6614(void);
bool fn_1_6594(void);

void fn_1_6ADC(PlayerModelOwner *);

#endif
