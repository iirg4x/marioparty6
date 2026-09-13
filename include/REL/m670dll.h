#ifndef M670DLL_H
#define M670DLL_H

#include "dolphin.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/mg/actman.h"
#include "game/mg/timer.h"

typedef struct M670Cpu_s {
    int playerNo;
    MGPLAYER *player;
    unsigned int state;
    int timer;
    int pillarNo;
    HuVecF targetPos;
    int buttonTimer;
} M670CPU;

typedef struct M670Work_s {
    OMOBJMAN *objman;
    int unk_04;
    int unk_08;
    HU3D_MODELID cameraModel[3];
    HU3D_MOTIONID cameraMotion[3];
    HU3D_MODELID models[3];
    HU3D_MODELID pillarModel[24];
    HU3D_MOTIONID pillarMotion[24][4];
    HU3D_MODELID collisionModel[24];
    HuVecF pillarPos[24];
    int collisionCount;
    HU3D_MODELID collisionModels[24];
    s16 unk_294;
    HU3D_MODELID positionModel;
    HU3D_MODELID winnerModel;
    s16 characterNo[4];
    s16 padNo[4];
    MGPLAYER *players[4];
    int group[4];
    int selectedWord;
    int speedLevel;
    s16 soloPlayer;
    s16 soloPad;
    s16 soloCharacter;
    int state;
    int playerState[4];
    OMOBJ *playerObjects[4];
    OMOBJ *pillarObject;
    int remainingPlayers;
    M670CPU cpu[4];
    u16 micContext;
    MGTIMER *timer;
    int pattern;
    int music;
} M670WORK;

extern M670WORK lbl_1_bss_10;
extern float lbl_1_bss_3B8[24];
extern int lbl_1_bss_418[24];
extern float lbl_1_bss_47C, lbl_1_bss_480;
extern s8 *lbl_1_data_250[6];

BOOL fn_1_1460(HuVecF *src, HuVecF *dst);
void fn_1_15B8(int soundId, HuVecF *pos);
void fn_1_1658(void);
void fn_1_22F0(int pillarNo, float height);
void fn_1_2384(MGACTOR *actor, int playerNo);
void fn_1_2460(int playerNo, int state);
void fn_1_2690(OMOBJ *obj);
void fn_1_28E8(OMOBJ *obj);
void fn_1_2A24(unsigned int state);
int fn_1_2DE8(HuVecF *pos);
float fn_1_2EF4(int pillarNo);
int fn_1_2F44(HuVecF *pos);
void fn_1_3098(OMOBJ *obj);
void fn_1_3B30(void);
void fn_1_3BDC(void);

#endif
