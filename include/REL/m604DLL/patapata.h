// The course: 49 steps for each of the four players, and the Noko that walk on them.
#ifndef REL_M604DLL_PATAPATA_H
#define REL_M604DLL_PATAPATA_H

#include <memory>
#include <vector>
#include "REL/framework/list_object.h"
#include "REL/framework/list.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "REL/m604DLL/particle_owner.h"

struct PatapataRecord {
    int type;
    int firstType;
    int moving;
    Vec position;
    Vec startPosition;
    Vec rotation;
    Vec startRotation;
    float speed;
    unsigned char reaction;
    unsigned char taken;
};

struct Noko : ListNode<Noko> {
    typedef void (Noko::*Callback)();

    std::auto_ptr<OwnedModel> model;
    Vec position;
    Vec startPosition;
    Vec rotation;
    Vec startRotation;
    float speed;
    int motionIndex;
    int secondaryMotion;
    int mode;
    int initialValue;
    float rotationRate;
    int unknown54;
    int playerIndex;
    unsigned char active;
    int state;
    int nextState;
    int recordIndex;
    unsigned char reactionStarted;
    unsigned char motionFinished;
    int timer;
    std::auto_ptr<ModelParticles> resource;
    Callback callback;

    Noko(OwnedModel *source, int player, int mode);
    ~Noko();
    void setMotion(int index, unsigned char keep, float blend);
    void startSpin();
    void stopSpin();
    void idle();
    void walk();
    void fall();
    void update();
    void checkRange();
    void setRecord(int index, const PatapataRecord *source);
    Vec getBodyPosition();
    Vec getRecordPosition();
    void hit();
    // Patapata sets every Noko walking, and later falling.
    void setState(Callback state) { callback = state; }
    void startWalk() { setState(&Noko::walk); }
    void startFall() { setState(&Noko::fall); }
    // Read the current movement state.
    u8 getActive() { return active; }
    s32 getMode() { return mode; }
    s32 getPlayerIndex() { return playerIndex; }
    s32 getRecordIndex() { return recordIndex; }
    s32 getState() { return state; }
    f32 getSpeed() { return speed; }
    const u8 getReactionStarted() { return reactionStarted; }
    Noko *next() { return static_cast<Noko *>(link0); }
};

struct Patapata : RequestObject, MinigameSingleton<Patapata> {
    enum { NUMBEROF_MOTION = 2 };
    typedef void (Patapata::*Callback)();

    PatapataRecord records[4][49];
    PatapataRecord *getRecord(s32 playerIndex, s32 recordIndex)
    {
        return &records[playerIndex][recordIndex];
    }
    std::vector<long> m_mot_id;
    Callback callback;
    OwnedModel model;
    std::auto_ptr<OwnedModel> nightModel;
    unsigned char finished;
    int counts[4];

    Patapata();
    virtual ~Patapata();
    virtual void flagEnabled(int bit);
    virtual void update();
    void addMotion(unsigned int dataNum);
    void clearStart();
    void startWalk();
    void waitWalk();
    void startFall();
    void placeRecords();
    void copyRecord(int player, int index, const PatapataRecord *source, unsigned char reaction);
};

s32 fn_1_C5F8(Patapata *game, s32 direction, s32 player, s32 *step, Vec *from, Vec *to, f32 *hit);

#endif
