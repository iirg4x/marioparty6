#ifndef M624_PLAYER_CALLBACKS_H
#define M624_PLAYER_CALLBACKS_H
#include "dolphin/types.h"
struct ModelOwner;
struct Player;
struct PlayerSetup;
struct PlayerRunner;
struct ModeResetOwner;
struct M624PlayerObject;

/* Player callback table. */
typedef struct PlayerCallbacks {
    u32 zeroWords[2];
    void *(*firstEntry)(struct ModelOwner *, s16);
    void (*secondEntry)(struct ModelOwner *);
    void (*entry10)(struct Player *);
    void (*entry14)(struct PlayerRunner *);
    void (*entry18)(struct PlayerRunner *);
    void (*unk1C)(struct ModeResetOwner *); /* The game's code never reads this member. */
    void (*entry20)(struct M624PlayerObject *);
    void (*entry24)(struct PlayerRunner *);
    void (*entry28)(struct PlayerRunner *);
    u32 unk2C; /* The game's code never reads this member. */
} PlayerCallbacks;
#ifdef __cplusplus
extern "C" {
#endif
extern PlayerCallbacks lbl_1_data_8A8;
#ifdef __cplusplus
}
#endif
#endif
