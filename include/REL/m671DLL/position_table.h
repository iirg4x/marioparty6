#ifndef REL_M671DLL_POSITION_TABLE_H
#define REL_M671DLL_POSITION_TABLE_H
#include "dolphin/mtx.h"
#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"

struct PositionTable;

struct PositionRecord {
    Vec value;
};

struct PositionTable : GameObject, MinigameSingleton<PositionTable> {
    PositionTable();
    int getCount() const { return activeCount; }
    Vec getStart(int index) const { return firstPositions[index].value; }
    Vec getPosition(int index) const { return positions[index]; }
    virtual ~PositionTable();
    virtual void update();
    void fn_1_83DC();
    Vec positions[4];
    unsigned char activeFlags[4];
    PositionRecord firstPositions[3];
    PositionRecord secondPositions[3];
    float thirdPositions[3];
    int count;
    int activeCount;
};

void fn_1_84BC(PositionTable *owner);
void fn_1_8644(PositionTable *owner);
void fn_1_8724(PositionTable *owner);

#endif
