#include "REL/framework/frame_time.h"
#include "REL/m671DLL/control.h"
#include "REL/m671DLL/player_object.h"
#include "REL/m671DLL/tail_types.h"
#include "dolphin/types.h"
#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "game/pad.h"
#include "game/gamework.h"
#include <math.h>
#include "REL/m671DLL/position_table.h"
#include "REL/framework/vector3.h"

void fn_1_13C8(Controller *state)
{
    state->stick.x = 0.0f;
    state->stick.y = 0.0f;
    state->stick.magnitude = 0.0f;
    state->update(&state->stick);
}

HumanControl::HumanControl(Player *owner) : Controller(owner)
{
    padIndex = lbl_80265728[owner->getIndex()].padNo;
}

Controller::~Controller() {}

void HumanControl::update(StickState *state)
{
    s32 padIndex = this->padIndex;
    f32 x = lbl_8024C7D8[padIndex];
    f32 y = lbl_8024C7E8[padIndex];
    f64 magnitude = sqrt((f64)(x * x + y * y));

    state->x = x;
    state->y = y;
    state->magnitude = (f32)magnitude;
    state->pressedButtons = lbl_802C0008[padIndex];
    state->heldButtons = lbl_802C0010[padIndex];
}

Vec fn_1_1BB8(ComputerControl *);

extern f32 fn_1_2568(ComputerControl *, s32);
extern f32 fn_1_25E0(ComputerControl *, s32);
extern f32 fn_1_2658(ComputerControl *, s32);
extern f32 fn_1_26D0(ComputerControl *, s32);
extern f32 fn_1_2748(ComputerControl *, s32);

ComputerControl::ComputerControl(Player *player, int option) : Controller(player)
{
    this->option = option;
    this->movementParameters[0] = fn_1_2568(this, option);
    this->movementParameters[1] = fn_1_25E0(this, option);
    this->movementParameters[2] = fn_1_2658(this, option);
    this->movementParameters[3] = fn_1_26D0(this, option);
    this->movementParameters[4] = fn_1_2748(this, option);
    this->movementScale = 0.0f;
    this->movement.x = 0.0f;
    this->movement.y = 0.0f;
    this->movement.z = 0.0f;
    this->setUpdate(&ComputerControl::fn_1_1A30);
}

ComputerControl::~ComputerControl() {}

void ComputerControl::update(StickState *state)
{
    PositionTable *manager = &PositionTable::getChecked();
    Vec position;
    player->getPosition(position);
    fn_1_11778(&angleWork, player->getPlayer());
    if (callback) {
        (this->*callback)(state);
    }
}

void ComputerControl::fn_1_1A30(StickState *state)
{
    Vec motion;
    f32 motionScale;

    motion.x = 0.0f;
    motion.y = 0.0f;
    motion.z = 0.0f;
    motion = fn_1_1BB8(this);
    if (motion.x != 0.0f || motion.z != 0.0f) {
        PSVECNormalize(&motion, &motion);
        this->movement.x = motion.x;
        this->movement.y = motion.y;
        this->movement.z = motion.z;
        this->movementScale = 9.333333f;
    } else if (this->movement.x != 0.0f || motion.z != 0.0f) {
        PSVECNormalize(&this->movement, &this->movement);
        this->movementScale += -this->movementScale /
                               (60.0f * this->movementParameters[4]);
    }
    motionScale = this->movementScale;
    PSVECScale(&this->movement, &this->movement,
               motionScale);
    fn_1_11548(&this->angleWork, &this->movement, 0.15f);
}

extern float fn_1_27C0(const Vec *point, const Vec *lineStart, const Vec *lineEnd, Vec *nearestPoint);

Vec fn_1_2224(ComputerControl *, const Vec *, Vec *, f32, s32);

Vec fn_1_1BB8(ComputerControl *owner)
{
    PositionTable &tables = PositionTable::getChecked();
    Vec origin = {0.0f, 0.0f, 0.0f};
    s32 total = 0;
    s32 candidateCount = 0;
    f32 nearestDistance = owner->movementParameters[0];
    s32 localCount = 0;
    s32 useDistant = 0;
    s32 nearestIndex;
    Vec currentPosition;
    owner->player->getPosition(currentPosition);
    s32 selectedEntry = owner->player->getIndex();
    Vec result;
    Vector3 delta;
    PSVECSubtract(&origin, &currentPosition, &delta);
    f32 distance = delta.length();
    Vec localDirection;
    Vec nearbyDirection;
    Vec distantDirection;

    result.x = 0.0f;
    result.y = 0.0f;
    result.z = 0.0f;
    nearbyDirection.x = 0.0f;
    nearbyDirection.y = 0.0f;
    nearbyDirection.z = 0.0f;
    localDirection.x = 0.0f;
    localDirection.y = 0.0f;
    localDirection.z = 0.0f;
    distantDirection.x = 0.0f;
    distantDirection.y = 0.0f;
    distantDirection.z = 0.0f;

    s32 count = tables.getCount();
    for (s32 i = 0; i < count; i++) {
        Vec candidate = tables.getStart(i);
        candidate.y = 0.0f;
        Vector3 candidateDelta;
        PSVECSubtract(&candidate, &currentPosition, &candidateDelta);
        if (candidateDelta.length() > owner->movementParameters[0]) {
            continue;
        }
        if (nearestDistance > candidateDelta.length()) {
            nearestDistance = candidateDelta.length();
            nearestIndex = i;
        }
        PSVECAdd(&candidate, &nearbyDirection, &nearbyDirection);
        candidateCount++;
    }

    for (s32 i = 0; i < 4; i++) {
        if (tables.activeFlags[i] != 0 && i != selectedEntry) {
            Vec position = tables.getPosition(i);
            Vector3 positionDelta;
            PSVECSubtract(&position, &currentPosition, &positionDelta);
            if (positionDelta.length() > owner->movementParameters[3]) {
                continue;
            }
            PSVECAdd(&position, &localDirection, &localDirection);
            localCount++;
        }
    }

    if (distance > 500.0f - owner->movementParameters[2]) {
        PSVECSubtract(&currentPosition, &origin, &distantDirection);
        if (distantDirection.x != 0.0f || distantDirection.z != 0.0f) {
            PSVECNormalize(&distantDirection, &distantDirection);
            PSVECScale(&distantDirection, &distantDirection, 500.0f);
            useDistant = 1;
        }
    }

    total = candidateCount + localCount + useDistant;
    if (total == 0 && useDistant == 0) {
        return result;
    }
    if (useDistant == 1 && candidateCount == 1) {
        result = fn_1_2224(owner, &currentPosition, &distantDirection, nearestDistance, nearestIndex);
        if (result.x != 0.0f || result.z != 0.0f) {
            PSVECNormalize(&result, &result);
        }
        return result;
    }
    if (candidateCount == 2) {
        PSVECScale(&nearbyDirection, &nearbyDirection, 1.0f / candidateCount);
        PSVECSubtract(&currentPosition, &nearbyDirection, &result);
        if (result.x != 0.0f || result.z != 0.0f) {
            PSVECNormalize(&result, &result);
        }
        return result;
    }
    if (useDistant == 1) {
        PSVECAdd(&localDirection, &distantDirection, &localDirection);
    }
    PSVECAdd(&localDirection, &nearbyDirection, &localDirection);
    Vec average;
    PSVECScale(&localDirection, &average, 1.0f / total);
    average.y = 0.0f;
    PSVECSubtract(&currentPosition, &average, &result);
    PSVECNormalize(&result, &result);
    return result;
}

static inline PositionRecord secondSegmentOffset(PositionTable *tables, s32 index)
{
    return tables->secondPositions[index];
}

static inline PositionRecord firstSegmentOffset(PositionTable *tables, s32 index)
{
    return tables->firstPositions[index];
}

static inline f32 segmentFactor(PositionTable *tables, s32 index)
{
    return tables->thirdPositions[index];
}

/* Debug check: empty in the release build; its original text is not in the binary. */

static inline Vec *segmentOffsetVector(PositionRecord *offset)
{
    ASSERT(FALSE);
    return &offset->value;
}

static inline void addSegmentOffset(Vec *offset, Vec *endpoint)
{
    PSVECAdd(offset, endpoint, endpoint);
}

Vec fn_1_2224(ComputerControl *view, const Vec *point, Vec *lineStart,
               f32 currentBestDistance, s32 segmentIndex)
{
    Vec direction;
    PositionTable *tables;
    f32 distanceLimit;
    f32 distance;
    f32 factor;
    f32 scale;

    tables = &PositionTable::getChecked();
    PositionRecord secondOffset = secondSegmentOffset(tables, segmentIndex);
    factor = segmentFactor(tables, segmentIndex);
    Vec firstOffset = firstSegmentOffset(tables, segmentIndex).value;
    distanceLimit = view->movementParameters[1] * (factor / 6.6666665f);
    if (distanceLimit < 140.0f) {
        distanceLimit = 140.0f;
    }
    firstOffset.y = 0.0f;
    secondOffset.value.y = 0.0f;
    Vec endpoint = firstOffset;
    direction.x = 0.0f;
    direction.y = 0.0f;
    direction.z = 0.0f;
    scale = (f32)secondsToFrames(1L);
    PSVECScale(&secondOffset.value, &secondOffset.value, scale);
    addSegmentOffset(segmentOffsetVector(&secondOffset), &endpoint);
    Vec nearestPoint;
    distance = fn_1_27C0(point, lineStart, &firstOffset, &nearestPoint);
    if (distance < 0.0f) {
        return direction;
    }
    if (distanceLimit < distance) {
        return direction;
    }
    PSVECSubtract(point, &nearestPoint, &direction);
    if (direction.x == 0.0f && direction.z == 0.0f) {
        return direction;
    }
    PSVECNormalize(&direction, &direction);
    return direction;
}

f32 fn_1_2568(ComputerControl *object, s32 state)
{
    switch (state) {
    case 0: return 200.0f;
    case 1: return 250.0f;
    case 2: return 300.0f;
    case 3: return 260.0f;
    default: return 100.0f;
    }
}

f32 fn_1_25E0(ComputerControl *object, s32 state)
{
    switch (state) {
    case 0: return 180.0f;
    case 1: return 200.0f;
    case 2: return 250.0f;
    case 3: return 250.0f;
    default: return 100.0f;
    }
}

f32 fn_1_2658(ComputerControl *object, s32 state)
{
    switch (state) {
    case 0: return 200.0f;
    case 1: return 150.0f;
    case 2: return 100.0f;
    case 3: return 100.0f;
    default: return 100.0f;
    }
}

f32 fn_1_26D0(ComputerControl *object, s32 state)
{
    switch (state) {
    case 0: return 250.0f;
    case 1: return 200.0f;
    case 2: return 170.0f;
    case 3: return 150.0f;
    default: return 100.0f;
    }
}

f32 fn_1_2748(ComputerControl *object, s32 state)
{
    switch (state) {
    case 0: return 0.8f;
    case 1: return 0.3f;
    case 2: return 0.3f;
    case 3: return 0.3f;
    default: return 0.3f;
    }
}

float fn_1_27C0(const Vec *point, const Vec *lineStart, const Vec *lineEnd,
                Vec *nearestPoint)
{
    Vec lineDirection;
    Vec pointFromStart;
    Vec scaledDirection;
    Vec perpendicular;
    float directionLengthSquared;
    float projectionNumerator;
    double projection;

    PSVECSubtract(lineEnd, lineStart, &lineDirection);
    PSVECSubtract(point, lineStart, &pointFromStart);
    directionLengthSquared = PSVECDotProduct(&lineDirection, &lineDirection);
    projectionNumerator = PSVECDotProduct(&lineDirection, &pointFromStart);
    projection = projectionNumerator / directionLengthSquared;
    if ((projection < 0.0) || (projection > 1.0)) {
        return -1.0f;
    }

    PSVECScale(&lineDirection, &scaledDirection, (float)projection);
    PSVECAdd(lineStart, &scaledDirection, nearestPoint);
    PSVECSubtract(&scaledDirection, &pointFromStart, &perpendicular);
    return PSVECMag(&perpendicular);
}

void Controller::fn_1_28D8()
{
}
