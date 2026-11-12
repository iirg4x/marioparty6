// The course and its Noko.

#include "REL/m604DLL/camera_masks.h"
#include "math.h"
#include "humath.h"

#include "REL/m604DLL/patapata.h"
#include "REL/m604DLL/player.h"
#include "REL/m604DLL/a_set.h"

extern "C" {
#include "game/frand.h"
}

#include "REL/m604DLL/camera_owner.h"

extern "C" {

#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"

}

s32 fn_1_AC58(const Vec *, const Vec *, const Vec *, const Vec *, f32 *);

extern "C" {
#include "game/gamework.h"
}

extern "C" {

const int lbl_1_rodata_628[4] = { 1, 0, 3, 2 };

}

s32 fn_1_AC58(const Vec *firstStart, const Vec *firstEnd, const Vec *secondStart,
              const Vec *secondEnd, f32 *intersection) {
    Vec firstSegment;
    Vec secondSegment;
    Vec firstOffset;
    Vec secondOffset;
    f32 firstCross;
    f32 secondCross;
    f64 firstParameter;
    f64 secondParameter;

    if (firstStart->x >= firstEnd->x) {
        if (((firstStart->x < secondStart->x) && (firstStart->x < secondEnd->x)) ||
            ((firstEnd->x > secondStart->x) && (firstEnd->x > secondEnd->x))) {
            return -1;
        }
    } else if (((firstEnd->x < secondStart->x) && (firstEnd->x < secondEnd->x)) ||
               ((firstStart->x > secondStart->x) && (firstStart->x > secondEnd->x))) {
        return -1;
    }

    if (firstStart->y >= firstEnd->y) {
        if (((firstStart->y < secondStart->y) && (firstStart->y < secondEnd->y)) ||
            ((firstEnd->y > secondStart->y) && (firstEnd->y > secondEnd->y))) {
            return -1;
        }
    } else if (((firstEnd->y < secondStart->y) && (firstEnd->y < secondEnd->y)) ||
               ((firstStart->y > secondStart->y) && (firstStart->y > secondEnd->y))) {
        return -1;
    }

    PSVECSubtract(firstEnd, firstStart, &firstSegment);
    PSVECSubtract(secondEnd, secondStart, &secondSegment);
    PSVECSubtract(firstStart, secondStart, &firstOffset);
    PSVECSubtract(secondStart, firstStart, &secondOffset);

    firstCross = secondSegment.x * firstSegment.y - secondSegment.y * firstSegment.x;
    secondCross = firstSegment.x * secondSegment.y - firstSegment.y * secondSegment.x;
    if (fabs(firstCross) < 1e-07) {
        return 0;
    }
    if (fabs(secondCross) < 1e-07) {
        return 0;
    }

    firstParameter = (f64)(secondSegment.x * secondOffset.y - secondSegment.y * secondOffset.x) /
                     (f64)firstCross;
    secondParameter = (f64)(firstSegment.x * firstOffset.y - firstSegment.y * firstOffset.x) /
                      (f64)secondCross;
    if ((firstParameter < 0.0) || (firstParameter > 1.0) ||
        (secondParameter < 0.0) || (secondParameter > 1.0)) {
        return -2;
    }

    intersection[0] = (f32)((f64)secondStart->x + ((f64)secondSegment.x * secondParameter));
    intersection[1] = (f32)((f64)secondStart->y + ((f64)secondSegment.y * secondParameter));
    return 1;
}

extern "C" {

const u32 lbl_1_rodata_650[2] = { 0x3F002D, 0x3F0016 };
}

Patapata::Patapata() : callback(&Patapata::clearStart), model(0x3F0000), finished(0)
{
    int night = GWMgNightFGet();

    nightModel.reset(new OwnedModel(lbl_1_rodata_650[night]));

    model.setAttribute(1);
    model.resetShadow();

    m_mot_id.clear();
    m_mot_id.reserve(NUMBEROF_MOTION);

    addMotion(0x3F0001);
    addMotion(0x3F0002);

    // Each course loads two motions
    // into the model's motion list.
    // Both calls above append one motion identifier.
    // Check that both motion identifiers were stored.

    assert(m_mot_id.size() == NUMBEROF_MOTION);

    nightModel->setAttribute(1);

    f32 width = 100.0f;
    if (night == 1) {
        width = 150.0f;
    }
    int gap = 3;
    int nextSpecial = 3;
    int previousType = 0;
    f32 previousSpeed = 0.0f;
    f32 previousX = 0.0f;
    for (int index = 0; index < 0x31; index++) {
        int type = 0;
        int moving = 0;
        f32 speed = 0.0f;
        Vec position;
        Vec rotation;
        position.x = 0.0f;
        position.y = 0.0f;
        position.z = 0.0f;
        rotation.x = 0.0f;
        rotation.y = 0.0f;
        rotation.z = 0.0f;
        if (index % nextSpecial == 0 && index != 0) {
            if (night == 0) {
                type = 6;
            } else {
                type = (frand() & 1) + 4;
            }
            gap++;
            if (night == 0) {
                nextSpecial += gap;
            } else {
                nextSpecial += 4;
            }
        } else if (((index >> 2) & 1) == 0) {
            type = lbl_1_rodata_628[index & 3];
        } else {
            type = (frand() % 4 < 0) ? -(frand() % 4) : frand() % 4;
        }
        switch (type) {
        case 0:
        case 2:
            if (type == 0) {
                if (night == 0) {
                    if (previousType != 0) {
                        position.x = -width * (1.0f - 0.25f * (frand() & 2));
                    } else if (previousX != -width) {
                        position.x = -width;
                    } else {
                        position.x = 0.5f * -width;
                    }
                } else {
                    position.x = -width * (1.0f - 0.25f * (frand() & 3));
                }
            } else {
                position.x = width * (3.0517578e-05f * ((frand() & 0xFFFF) - 32768.0f));
                if (previousType != 3 && previousType != 2) {
                    if (night == 0) {
                        speed = 2.6666667f;
                    } else {
                        speed = 0.6666667f;
                    }
                } else if (night == 0) {
                    speed = 2.6666667f;
                } else {
                    if (0.6666667f != ((previousSpeed < 0.0f) ? -previousSpeed : previousSpeed)) {
                        speed = 0.6666667f;
                    } else {
                        speed = 2.6666667f;
                    }
                }
            }
            rotation.y = 90.0f;
            break;
        case 1:
        case 3:
            if (type == 1) {
                if (night == 0) {
                    if (previousType != 1) {
                        position.x = width * (1.0f - 0.25f * (frand() & 2));
                    } else if (previousX != width) {
                        position.x = width;
                    } else {
                        position.x = 0.5f * width;
                    }
                } else {
                    position.x = width * (1.0f - 0.25f * (frand() & 3));
                }
            } else {
                position.x = width * (3.0517578e-05f * ((frand() & 0xFFFF) - 32768.0f));
                if (previousType != 3 && previousType != 2) {
                    if (night == 0) {
                        speed = -2.6666667f;
                    } else {
                        speed = -0.6666667f;
                    }
                } else if (night == 0) {
                    speed = -2.6666667f;
                } else {
                    if (0.6666667f != ((previousSpeed < 0.0f) ? -previousSpeed : previousSpeed)) {
                        speed = -0.6666667f;
                    } else {
                        speed = -2.6666667f;
                    }
                }
            }
            rotation.y = -90.0f;
            break;
        case 4:
            position.x = -200.0f;
            break;
        case 5:
            position.x = 200.0f;
            break;
        case 6:
            position.x = 0.0f;
            break;
        }
        position.y = 300.0f + 4.0f * (index * 100);
        if (type >= 4 && type < 7) {
            moving = 1;
        } else {
            moving = 0;
        }
        for (int player = 0; player < 4; player++) {
            PatapataRecord *record = &records[player][index];
            record->moving = moving;
            record->firstType = type;
            record->type = type;
            record->position = record->startPosition = position;
            record->rotation = record->startRotation = rotation;
            record->reaction = 0;
            record->taken = 0;
            record->speed = speed;
        }
        previousX = position.x;
        previousType = type;
        previousSpeed = speed;
    }
    for (int index = 0; index < 4; index++) {
        for (int body = 0; body < 4; body++) {
            new Noko(&model, index, 0);
        }
        for (int shadow = 0; shadow < 2; shadow++) {
            new Noko(nightModel.get(), index, 1);
        }
        counts[index] = 0;
    }
}

Patapata::~Patapata()
{
    ASet<Noko> *list = &ASet<Noko>::getChecked();
    list->deleteAll();
}

void Patapata::addMotion(unsigned int dataNum)
{
    s16 jointModel = model;
    s16 motion = fn_80030458(jointModel, fn_80008DD0(dataNum, 0x10000000, 2));
    m_mot_id.push_back(motion);
}

static inline void setCallback(Patapata *game, Patapata::Callback next) { game->callback = next; }
void Patapata::flagEnabled(int bit)
{
    switch (bit) {
    case 0:
        setCallback(this, &Patapata::clearStart);
        break;
    case 1:
        setCallback(this, &Patapata::startWalk);
        break;
    case 2:
    case 4:
        break;
    case 3:
        setCallback(this, &Patapata::startFall);
    }
}

void Patapata::update()
{
    if (callback != 0) {
        (this->*callback)();
    }
    ASet<Noko> *list = &ASet<Noko>::getChecked();
    list->forEach(&Noko::update);
    if (finished != 1) {
        placeRecords();
    }
}

void Patapata::clearStart()
{
    flags &= ~1;
}

void Patapata::startWalk()
{
    ASet<Noko> *root = &ASet<Noko>::getChecked();
    Noko *node = root->front();
    Node *end = root->end_node();
    while (node != end) {
        node->startWalk();
        node = node->next();
    }
    flags &= ~2;
    setCallback(this, &Patapata::waitWalk);
}

void Patapata::waitWalk() {}

void Patapata::startFall()
{
    ASet<Noko> *root = &ASet<Noko>::getChecked();
    Noko *node = root->front();
    Node *end = root->end_node();
    while (node != end) {
        node->startFall();
        node = node->next();
    }
    finished = 1;
    flags &= ~8;
}

void Patapata::placeRecords()
{
    ASet<Noko> *root = &ASet<Noko>::getChecked();
    Noko *node = root->front();
    Noko *first = root->front();
    Node *listEnd = root->end_node();
    const CameraOwner &scene = MinigameSingleton<CameraOwner>::getChecked();
    int player;
    for (player = 0; player < 4; ++player) {
        Vec positionCopy = scene.fn_1_878C(player);
        counts[player] = (int)((300.0f + positionCopy.y) / 400.0f);
        if (counts[player] > 0x30) {
            counts[player] = 0x30;
        }
        int recordIndex;
        for (recordIndex = 0; recordIndex < 4; recordIndex++) {
            int itemIndex = counts[player] - recordIndex;
            if (itemIndex < 0) {
                itemIndex = 0;
            }
            PatapataRecord *record = &records[player][itemIndex];
            if (record->taken != 1) {
                node = first;
                while (node != listEnd) {
                    int nodePlayer = node->getPlayerIndex();
                    if (nodePlayer == player) {
                        const u8 active = node->getActive();
                        if (active != 1 && record->moving == node->getMode()) {
                            PatapataRecord copy;
                            copy.type = record->type;
                            copy.firstType = record->firstType;
                            copy.position = record->position;
                            copy.startPosition = record->startPosition;
                            copy.rotation = record->rotation;
                            copy.startRotation = record->startRotation;
                            copy.speed = record->speed;
                            copy.reaction = record->reaction;
                            node->setRecord(itemIndex, &copy);
                            record->taken = 1;
                            break;
                        }
                    }
                    node = node->next();
                }
            }
        }
    }
}

s32 fn_1_C5F8(Patapata *unusedObject, s32 updateMode, s32 identifier, s32 *result, Vec *firstPoint, Vec *secondPoint, f32 *intersection)
{
    Node *listEnd;
    Noko *node;
    ASet<Noko> *root = &ASet<Noko>::getChecked();

    node = root->front();
    listEnd = root->end_node();
    *result = -1;
    while (node != listEnd)
    {
        if (identifier == node->getPlayerIndex())
        {
            if (node->getActive() != 0)
            {
                if (node->getMode() == 0)
                {
                    f32 nearHalfWidth = 60.000004f;
                    if (updateMode == 1)
                    {
                        nearHalfWidth = 50.0f;
                    }
                    Vec nearSecondPoint = *secondPoint;
                    Vec nearCenter = node->getBodyPosition();
                    Vec nearMinimum = nearCenter;
                    nearMinimum.x += -nearHalfWidth;
                    Vec nearMaximum = nearCenter;
                    nearMaximum.x += nearHalfWidth;
                    Vec nearPlanePosition = node->getRecordPosition();
                    Vec nearFirstPoint = *firstPoint;
                    if (fn_1_AC58(&nearFirstPoint, &nearSecondPoint, &nearMinimum, &nearMaximum, intersection) == 1)
                    {
                        *intersection = nearSecondPoint.x;
                        if (updateMode == 0)
                        {
                            node->hit();
                        }
                        *result = node->getRecordIndex();
                        s32 resultType = node->getState();
                        if ((resultType == 2) || (resultType == 3))
                        {
                            *intersection += node->getSpeed();
                        }
                        return 0;
                    }
                    node->reactionStarted = 0;
                }
                else
                {
                    Vec farSecondPoint = *secondPoint;
                    Vec farCenter = node->getBodyPosition();
                    Vec farMinimum = farCenter;
                    farMinimum.x += -300.0f;
                    Vec farMaximum = farCenter;
                    farMaximum.x += 300.0f;
                    Vec farFirstPoint = *firstPoint;
                    if (fn_1_AC58(&farFirstPoint, &farSecondPoint, &farMinimum, &farMaximum, intersection) == 1)
                    {
                        *intersection = farSecondPoint.x;
                        if (node->getReactionStarted() == 0)
                        {
                            node->hit();
                            if (updateMode == 0)
                            {
                                node->reactionStarted = 1;
                            }
                        }
                        *result = node->getRecordIndex();
                        return 1;
                    }
                    node->reactionStarted = 0;
                }
            }
        }
        node = node->next();
    }
    return -1;
}

void Patapata::copyRecord(int player, int index, const PatapataRecord *source, unsigned char reaction)
{
    PatapataRecord *record = &records[player][index];

    record->type = source->type;
    record->firstType = source->firstType;
    record->position = source->position;
    record->startPosition = source->startPosition;
    record->rotation = source->rotation;
    record->startRotation = source->rotation;
    record->startRotation = source->startRotation;
    record->speed = source->speed;
    record->reaction = source->reaction;
    record->taken = reaction;
}

Noko::Noko(OwnedModel *source, int player, int mode)
    : motionIndex(-1), secondaryMotion(-1), mode(mode), initialValue(0), rotationRate(0.0f), playerIndex(player), active(0),
      state(-1), recordIndex(-1), reactionStarted(0), motionFinished(0), timer(0)
{
    if (this->mode == 0) {
        model.reset(new OwnedModel(0x3F0000));
    } else {
        model.reset(new OwnedModel(*source));
    }
    ASet<Noko>::getChecked().insert(this);
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    rotation.x = 0.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    speed = 0.0f;
    model->setCamera((u16)cameraMasks[player]);
    model->setAttribute(1);
    model->setLayer(3);
    model->resetShadow();
    model->setPosition(&position);
    model->setRotation(&rotation);
    startPosition.x = 0.0f;
    startPosition.y = 0.0f;
    startPosition.z = 0.0f;
    if (1 == this->mode) {
        resource.reset(new ModelParticles(player));
    } else {
        setMotion(0, 0, 16.0f);
    }
    setState(&Noko::idle);
}

Noko::~Noko() {}

void Noko::setMotion(int index, unsigned char keep, float blend)
{
    Patapata &game = Patapata::getChecked();
    s16 modelId = *model;
    s16 motion = game.m_mot_id.at(index);
    if (motionIndex != index) {
        fn_80026A44(modelId, ((1 << 30) | (1 << 1)));
        fn_8002CCC0(modelId, motion, 0.0f, blend, (keep == 0 ? ((1 << 30) | (1 << 0)) : 0));
        motionIndex = index;
    }
}

void Noko::startSpin()
{
    model->resetAttribute(0x40000002);
    model->setMotionTime(0.0f);
    motionFinished = 1;
}

void Noko::stopSpin()
{
    model->setAttribute(0x40000002);
    motionFinished = 0;
}

void Noko::idle() {}

void Noko::walk()
{
    s32 nightMode = GWMgNightFGet();
    f32 angleLimit = 100.0f;
    if (nightMode == 1)
    {
        angleLimit = 150.0f;
    }
    switch (state)
    {
    case 2 :
        position.x += speed;
        if (position.x > angleLimit)
        {
            state = 8;
            position.x = angleLimit;
        }
        break;
    case 3 :
        position.x += speed;
        if (position.x < - angleLimit)
        {
            state = 9;
            position.x = - angleLimit;
        }
        break;
    case 4 :
    case 5 :
    case 6 :
        if (motionFinished == 0)
        {
            position.y = (f32)(startPosition.y + 20.0 * std::sin((f64)(360.0f * rotationRate) * M_PI / 180.0));
            rotationRate += 0.016666666666666666;
        }
        else
        {
            if (fn_8002D75C(*model) == 1)
            {
                model->setAttribute((1 << 30) | (1 << 1));
                motionFinished = 0;
            }
        }
        break;
    case 7 :
    case 0 :
    case 1 :
        break;
    case 8 :
        rotation.y -= 2.0f;
        if (rotation.y < -90.0f)
        {
            rotation.y = -90.0f;
            state = 3;
            speed = - speed;
        }
        break;
    case 9 :
        rotation.y += 2.0f;
        if (rotation.y > 90.0f)
        {
            rotation.y = 90.0f;
            state = 2;
            speed = - speed;
        }
        break;
    case 10 :
        {
            timer++;
            if ((f32) timer > 47.0f)
            {
                setMotion(0, 0, 16.0f);
                position = startPosition;
                rotation = startRotation;
                state = nextState;
            }
            break;
        }
    case 11 :
        if (position.x > 0.0f)
        {
            position.x += 13.333333333333334;
            position.y += -8.333333333333334;
        }
        else
        {
            position.x += -13.333333333333334;
            position.y += -8.333333333333334;
        }
        rotationRate += 0.016666666666666666;
        if (rotationRate < 1.0f)
        {
            break;
        }
        state = 12;
        rotationRate = 0.0f;
        if (position.x > 0.0f)
        {
            rotation.y = -90.0f;
        }
        else
        {
            rotation.y = 90.0f;
        }
    case 12 :
        position.x = position.x +
            (startPosition.x - position.x) / 4.0f;
        position.y = position.y +
            (startPosition.y - position.y) / 4.0f;
        rotationRate += 0.016666666666666666;
        if (! (rotationRate < 1.0f))
        {
            position = startPosition;
            rotation = startRotation;
            state = nextState;
            rotationRate = 0.0f;
        }
        break;
    }
}

void Noko::fall()
{
    f32 distance;
    if (active != 0) {
        if (mode == 1) {
            position.x = position.x + ((-500.0f - position.x) / 60.0f);
        } else if (position.x > 0.0f) {
            rotation.y = rotation.y + ((90.0f - rotation.y) / 30.0f);
            position.x = position.x + ((500.0f - position.x) / 72.0f);
        } else {
            rotation.y = rotation.y + ((-90.0f - rotation.y) / 30.0f);
            position.x = position.x + ((-500.0f - position.x) / 72.0f);
        }
        if (position.x < 0.0f) {
            distance = -position.x;
        } else {
            distance = position.x;
        }
        if (distance > 460.0f) {
            model->setAttribute(1);
            active = 0;
        }
    }
}

void Noko::update()
{
    Patapata *game;

    game = &Patapata::getChecked();
    if (callback != 0) {
        (this->*callback)();
    }
    model->setPosition(&position);
    model->setRotation(&rotation);
    if (active == 0) {
        return;
    }

    checkRange();
    Vec selectedPosition = position;
    if (mode == 1) {
        selectedPosition.x = startPosition.x;
        selectedPosition.y = startPosition.y;
        selectedPosition.z = startPosition.z;
    }
    PatapataRecord snapshot;
    snapshot.type = state;
    snapshot.firstType = nextState;
    snapshot.position = selectedPosition;
    snapshot.startPosition = startPosition;
    snapshot.rotation = rotation;
    snapshot.startRotation = startRotation;
    snapshot.speed = speed;
    snapshot.reaction = reactionStarted;
    game->copyRecord(playerIndex, recordIndex, &snapshot, active);
}

void Noko::checkRange()
{
    f32 distanceY;
    const CameraOwner *recordData;

    recordData = &MinigameSingleton<CameraOwner>::getChecked();
    Vec copiedPosition = recordData->fn_1_878C(playerIndex);
    Vec difference;
    PSVECSubtract(&copiedPosition, &position, &difference);
    if (difference.y < 0.0f) {
        distanceY = -difference.y;
    } else {
        distanceY = difference.y;
    }
    if (distanceY > 750.0f) {
        active = 0;
        model->setAttribute(1);
        return;
    }
    active = 1;
    model->resetAttribute(1);
}

void Noko::setRecord(int index, const PatapataRecord *source)
{
    recordIndex = index;
    state = source->type;
    nextState = source->firstType;
    position.x = source->position.x;
    position.y = source->position.y;
    position.z = source->position.z;
    startPosition.x = source->startPosition.x;
    startPosition.y = source->startPosition.y;
    startPosition.z = source->startPosition.z;
    rotation.x = source->rotation.x;
    rotation.y = source->rotation.y;
    rotation.z = source->rotation.z;
    startRotation.x = source->startRotation.x;
    startRotation.y = source->startRotation.y;
    startRotation.z = source->startRotation.z;
    speed = source->speed;
    reactionStarted = source->reaction;
    active = 1;
}

struct ModelPositionCopy {
    Vec value;
    ModelPositionCopy() {}
    ModelPositionCopy(const HSF_OBJECT *source) : value(source->mesh.curr.pos) {}
};

Vec Noko::getRecordPosition()
{
    s16 modelIndex = *model;
    HSF_OBJECT *object = fn_80026BF4(modelIndex, "noko_body");
    ModelPositionCopy currentPosition(object);
    return currentPosition.value;
}

Vec Noko::getBodyPosition()
{
    Vec result = position;
    ModelPositionCopy objectPositionCopy;
    HSF_OBJECT *object;
    s16 handle;
    if (mode == 0) {
        handle = *model;
        object = fn_80026BF4(handle, "noko_body");
        ModelPositionCopy objectPosition(object);
        objectPositionCopy = objectPosition;
        PSVECAdd(&result, &objectPositionCopy.value, &result);
    }
    return result;
}

void Noko::hit()
{
    if (mode == 0) {
        if (reactionStarted != 1) {
            reactionStarted = 1;
            startPosition.x = position.x;
            startPosition.y = position.y;
            startPosition.z = position.z;
            startRotation.x = rotation.x;
            startRotation.y = rotation.y;
            startRotation.z = rotation.z;
            nextState = state;
            rotationRate = 0.0f;
            state = 10;
            timer = 0;
            setMotion(1, 1, 16.0f);
        }
    } else {
        startSpin();
        fn_1_153C4(resource.get(), &position);
    }
}
