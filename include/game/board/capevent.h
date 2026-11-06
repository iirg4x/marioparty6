#ifndef _BOARD_CAPEVENT_H
#define _BOARD_CAPEVENT_H

#include "game/object.h"

struct EvCapWork;
struct CapWork;
void *mbev_CapMalloc(struct EvCapWork *work, int size);
void mbev_CapBezierGetV(float t, float *a, float *b, float *c, float *out);
void mbev_CapBezierNormGetV(float t, float *a, float *b, float *c, float *out);
void mbev_CapWait(struct CapWork *work);
int mbev_CapObjCreate(struct EvCapWork *work, int dataNum, int *motFile,
    BOOL linkF, int delay, BOOL closeDir);
void mbev_CapObjPosSet(struct EvCapWork *work, int objId, int masuId, HuVecF *pos);
void mbev_CapPlayerPosSet(struct EvCapWork *work, int playerNo, int masuId, HuVecF *pos);
s16 mbev_CapPlayerMotionCreate(struct EvCapWork *work, int playerNo, int dataNum);
OMOBJ *mbev_CapEffSnowCreate(void);
OMOBJ *mbev_CapEffCapLoseCreate(void);
int mbev_CapEffSnowAdd(OMOBJ *obj, HuVecF *pos, int time);
void mbev_CapEffCapLoseAdd(OMOBJ *obj, int playerNo, int count, float radius);
int mbev_CapEffCapLoseNumGet(OMOBJ *obj);
void mbev_CapPlayerMoveEjectCreate(int playerNo, BOOL useShiftF);
void mbev_CapPlayerMoveMinYSet(int playerNo, float minY);
void mbev_CapPlayerMoveVelSet(int playerNo, float gravityStep, HuVecF *moveDir);
BOOL mbev_CapCullCheck(int playerNo, int masuId);
void mbev_CapVecRotGet(HuVecF *vec, HuVecF *rot);
void mbev_CapPlayerMotionSet(int playerNo, int time, int motNo, int nextMotNo,
    u32 attr, u32 unk18, BOOL shiftF, BOOL nextAttr);

void mbev_CapEffCoinGlowSet(OMOBJ *obj, OMOBJ *glowObj);
int mbev_CapPlayerSquishVoiceSet(int *out, int masuId, BOOL voiceF);
OMOBJ *mbev_CapCoinManCreate(void);
OMOBJ *mbev_CapStarManCreate(void);
BOOL mbev_CapPlayerCheck(int playerNo1, int playerNo2);
int mbev_CapEffCoinAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel,
    float scale, float gravity, int time, int arg);
int mbev_CapCoinManAdd(OMOBJ *obj, HuVecF *from, HuVecF *to,
    int playerNo, BOOL coinNum);
int mbev_CapCoinManNumGet(OMOBJ *obj);

void mbev_CapBankCoinInit(void);
void mbev_CapInit(void);
void mbev_CapKettouEndCall(int playerNo);
void mbev_CapDonkeyEndCall(int playerNo);
void mbev_CapKoopaEndCall(int playerNo);
int mbev_CapBiriQShockDelayGet(int playerNo);
void mbev_CapBiriQMetalShockCreate(int playerNo);
void mbev_CapPlayerStunSet(int *playerNo, int playerNum, BOOL type);
void mbev_CapEffDustHeavyAdd(OMOBJ *obj, HuVecF *pos);
OMOBJ *mbev_CapEffRingHitCreate(void);
OMOBJ *mbev_CapEffGlowFireCreate(void);
OMOBJ *mbev_CapEffElectricCreate(void);
int mbev_CapEffElectricAdd(OMOBJ *obj, HuVecF *pos, int time, int bank);
void mbev_CapEffElectricModelSet(OMOBJ *obj, int modelId, int effectId, HuVecF *offset);
int mbev_CapEffRingDispGet(OMOBJ *obj);
void mbev_CapEffRingKill(OMOBJ *obj);
void mbev_CapEffGlowKill(OMOBJ *obj);
void mbev_CapEffElectricKill(OMOBJ *obj);
void mbev_CapPlayerRotate(int playerNo, float angle);
void mbev_CapVecChase(float weight, HuVecF *src, HuVecF *target, HuVecF *out);
void mbev_CapEffGlowCoinAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rot);
void mbev_CapVibrate(int type);
s16 mbev_CapSprCreate(struct EvCapWork *work, unsigned int dataNum,
    s16 prio, s16 bank);
void mbev_CapObjClose(struct EvCapWork *work, int objId);
void mbev_CapDuelStatusDispSet(int leftPlayer, int rightPlayer, BOOL waitF);
int mbev_CapPlayerOrderGet(int *order, int excludePlayer, int priorityPlayer,
    BOOL orderF);
float mbev_CapAngleLerp(float a, float b, float t);
float mbev_CapAngleWrap(float a, float b);
int mbev_CapPlayerComSelKettouGet(int playerNo, int selection,
    int *playerList, int playerNum);
BOOL mbev_CapEffCoinMaxYSet(OMOBJ *obj, int coinNo, float maxY);
int mbev_CapStarManAdd(OMOBJ *obj, HuVecF *from, HuVecF *to,
    int playerNo, BOOL starNum);
int mbev_CapStarManNumGet(OMOBJ *obj);

void mbev_CapCallTeresa(int playerNo, int masuId);
void mbev_CapCallKettou(int playerNo, int masuId, BOOL stopF);
BOOL mbev_CapCallTrap(int playerNo, int masuId, int masuIdNext);
void mbev_CapKillerMoveCall(int playerNo);
void mbev_CapBiriQShockCreate(int playerNo);
void mbev_CapRandomBonusCoin(int playerNo, int capsuleNo, BOOL waitF);
BOOL mbev_CapBonusCoinCheck(int playerNo);
OMOBJ *mbev_CapEffExplodeCreate(void);
void mbev_CapEffExplodeKill(OMOBJ *obj);
OMOBJ *mbev_CapEffGlowCreate(void);
void mbev_CapEffGlowBlendModeSet(OMOBJ *obj, int blendMode);
void mbev_CapEffGlowPatSet(OMOBJ *obj, int pat);
int mbev_CapEffGlowDispGet(OMOBJ *obj);
int mbev_CapEffGlowKinokoAdd(OMOBJ *obj, HuVecF *pos, int time, float scale,
    float xRange, float yRange, float zRange, int type, GXColor *color);
int mbev_CapEffGlowKinokoTimeSet(OMOBJ *obj, int index, int time, int delay);
OMOBJ *mbev_CapEffRingCreate(void);
int mbev_CapEffGlowAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel, int time,
    float scale, float gravity, float unk, GXColor *color);
void mbev_CapEffColorSet(GXColor *color, int colorNo);
void mbev_CapEffDustExplodeAdd(OMOBJ *obj, HuVecF *pos);
int mbev_CapEffExplodeAnimGet(OMOBJ *obj);
int mbev_CapPlayerMasuNumGet(int masuId);
void mbev_CapBubbleHookCall(int type, int modelId, BOOL flag1, BOOL flag2, BOOL flag3);
void mbev_CapPlayerMotShiftWait(int playerNo, int motNo, u32 attr, BOOL waitF);
void mbev_CapPlayerMotShiftSet(int modelId, int motionNo, u32 attr, BOOL shiftF);
void mbev_CapObjMotionSet(int modelId, int time, int motNo, int nextMotNo,
    u32 attr, u32 nextAttr, BOOL shiftF, BOOL nextShift);
s16 mbev_CapMasuPrevGet(s16 masuId, HuVecF *pos);
void mbev_CapCoinAdd(OMOBJ *obj, int playerNo, int coinNum, BOOL highF);
OMOBJ *mbev_CapEffExhaustCreate(void);
OMOBJ *mbev_CapEffBoostCreate(void);
void mbev_CapEffBoostBlendModeSet(OMOBJ *obj, int blendMode);
void mbev_CapPlayerMoveObjInit(void);
void mbev_CapPlayerMoveIdleCreate(int playerNo, int moveTime);
void mbev_CapPlayerMoveHitCreate(int playerNo, BOOL useMotF, BOOL useShiftF);
BOOL mbev_CapPlayerMoveObjCheck(int playerNo);
void mbev_CapPlayerMoveObjKill(void);
void mbev_CapPlayerIdleWait(void);
s16 mbev_CapMasuLinkNextGet(s16 masuId, HuVecF *pos);
s16 mbev_CapMasuLinkNextRandomGet(s16 masuId, HuVecF *pos);
int mbev_CapEffExplodeKillerAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel,
    float active, float angleStep, float fadeStep, float scale, GXColor *color);
int mbev_CapEffExplodeAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel,
    float active, float angleStep, float fadeStep, GXColor *color);
int mbev_CapEffBoostAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel,
    float active, float angleStep, int time, GXColor *color);
void mbev_CapEffDustCloudAdd(OMOBJ *obj, HuVecF *pos);
void mbev_CapStatusDispSetAll(BOOL dispF, BOOL waitF);
int mbev_CapCoinDisp(int playerNo, int coinNum, BOOL winMotF, BOOL waitF);
float mbev_CapAngleSumLerp(float t, float a, float b);
void mbev_CapHermiteGetV(float t, HuVecF *a, HuVecF *b, HuVecF *c,
    HuVecF *d, HuVecF *out);
void mbev_CapEffRayTransformSet(OMOBJ *obj, HuVecF *pos, HuVecF *rot, Mtx *mtx);
void mbev_CapEffMasuHitTransformSet(OMOBJ *obj, HuVecF *pos, HuVecF *rot, Mtx *mtx);
void mbev_CapEffCoinMultiAdd(OMOBJ *obj, HuVecF *pos, int num);
BOOL mbev_CapMasuMoveCheck(int masuId);
int mbev_CapEffCoinNumGet(OMOBJ *obj);
void mbev_CapEffRayKill(OMOBJ *obj);
void mbev_CapEffMasuHitKill(OMOBJ *obj);
void mbev_CapEffCoinKill(OMOBJ *obj);
OMOBJ *mbev_CapEffRayCreate(float scale, float speed);
OMOBJ *mbev_CapEffMasuHitCreate(void);
OMOBJ *mbev_CapEffCoinCreate(void);
int mbev_CapEffRayAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rotA, HuVecF *rotB,
    float scale, int time);
void mbev_CapEffRayAlphaSet(OMOBJ *obj, float alpha);
int mbev_CapEffMasuHitAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rotA, HuVecF *rotB,
    float scale, float scaleY, int time);
void mbev_CapBonusCoinCall(int playerNo, int capsuleNo, int coinNum, BOOL waitF);
int mbev_CapCall(int playerNo, int capsuleNo, BOOL moveF, BOOL stopF);
void mbev_CapCallDonkey(int playerNo);
void mbev_CapCallKoopa(int playerNo);
void mbev_CapCallMiracle(int playerNo, int masuId);

#endif
