#ifndef _BOARD_SINGLE_H
#define _BOARD_SINGLE_H

int mbSingleCall(int mode, int arg);
void mbSingleInit(void);
void mbSingleClose(void);
void mbSingleReturn(void);
void mbSingleSaveInit(int teamChar, int mgPack, int storyComDif);
void mbSingleSaveFlush(int value);
void mbSingleGameEnd(void);
int mbev_SingleMgEnd(int playerNo);
int mbSingleStepGet(void);
int mbSingleOppCharGet(void);
int mbSingleTeamCharGet(void);
BOOL mbSingleMgUnlockGet(int mgNo);
int mbSingleMgUnlockNumGet(void);
void mbev_SingleMg(int playerNo, s16 masuId);

#endif
