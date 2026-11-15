#include "REL/m634DLL/event_calls.h"
#include "REL/m634DLL/sequence_owner.h"
#include "dolphin/mtx.h"
extern "C" {
#include "game/hu3d.h"
#include "game/gamework.h"
}
void fn_1_106AC(SequenceOwner *);

void *operator new(unsigned long);
extern "C" {
unsigned long lbl_1_bss_60;
signed char lbl_1_bss_64;
}

const float lbl_1_rodata_780[2] = { 5.0f, 4.5f };

SequenceOwner::SequenceOwner(s32 first, s32 second,
                              char *third, s32 mode, Vec position,
                              float elapsed)
    : GameObject(mode), box(position.x, position.y, position.z),
      initialized(0), sequenceType(mode),
      motionA(new OwnedModel((unsigned int)first)),
      motionB(new OwnedModel((unsigned int)second))
{
    SequenceOwner::Callback callback;
    if (mode == 0x70)
    {
        this->motionDuration = lbl_1_rodata_780[0];
    }
    else
    {
        this->motionDuration = lbl_1_rodata_780[1];
    }
    this->frameRate = 1.0f + motionA->getMotionMaxTime();
    this->frameLimit = (long)(0.5f + (this->frameRate / this->motionDuration));
    this->frame = 0;
    this->elapsed = 0.0f;
    motionA->setCamera(2);
    motionA->resetAttribute(((1 << 30) | (1 << 1)));
    fn_8002D730((short)*motionA, 0.0f, 0.0f);
    motionA->setMotionTime(0.0f);
    motionA->setLayer(1);
    motionB->setCamera(2);
    motionB->setLayer(1);
    motionB->setAttribute(1);
    if (lbl_1_bss_64 == 0)
    {
        lbl_1_bss_60 = 0;
        lbl_1_bss_64 = 1;
    }
    motionB->getObjectPosition(third, &box.position);
    box.position.y = 100.0f;
    lbl_1_bss_60++;
    box.enabled = 1;
    if (mode == 0x70)
    {
        box.shapeKind = 4;
    }
    else
    {
        box.shapeKind = 5;
    }
    PSMTXIdentity(rotation);
    PSMTXRotRad(rotation, 0x79, MTXDegToRad(elapsed));
    box.CollisionShape::matrix = &rotation;
    callback = & SequenceOwner::fn_1_10744;
    this->callback = callback;
}

SequenceOwner::~SequenceOwner() {}
static inline void ActivateObject(s32 objectId)
{
    sendEvent(0, 1, &objectId, 0.0f);
}

void SequenceOwner::update()
{
    s32 waitingObjectId;
    s32 activeObjectId;

    if (initialized == 0) {
        waitingObjectId = box.collisionKind;
        if (waitingObjectId != -1) {
            fn_1_106AC(this);
        }
    }
    if (callback != 0) {
        (this->*callback)();
    }
    if (initialized == 1) {
        activeObjectId = box.collisionKind;
        ActivateObject(activeObjectId);
    }
    box.collisionKind = -1;
}

void fn_1_105CC(SequenceOwner *owner)
{
    owner->motionA->setAttribute(1);
    owner->motionB->setAttribute(1);
}

void fn_1_10624(SequenceOwner *work)
{
    work->frame = (s32)(0.5f + work->elapsed / work->motionDuration);
    fn_8002D730(*work->motionA, 0.0f, work->elapsed);
}

void fn_1_106AC(SequenceOwner *work)
{
    if (work->sequenceType == 0x70) {
        sendEvent(2, 0, &work->box.position, 0.0f);
    } else {
        sendEvent(2, 1, &work->box.position, 0.0f);
    }
    work->elapsed += work->motionDuration;
    if (work->elapsed > work->frameRate) {
        work->elapsed = work->frameRate;
    }
}

void SequenceOwner::fn_1_10744()
{
    SequenceOwner::Callback callback;

    this->frame = (s32)(0.5f + this->elapsed / this->motionDuration);
    fn_8002D730(*motionA, 0.0f, this->elapsed);

    if (this->frame >= this->frameLimit) {
        motionA->setAttribute(0x40000002);
        motionA->setAttribute(1);
        motionB->resetAttribute(1);

        if (this->sequenceType == 0x70) {
            sendEvent(2, 3, &this->box.position, 0.0f);
        } else {
            sendEvent(2, 4, &this->box.position, 0.0f);
        }

        this->box.enabled = 0;
        this->initialized = 1;
        callback = &SequenceOwner::fn_1_108A0;
        this->callback = callback;
    }
}

void SequenceOwner::fn_1_108A0()
{
}
