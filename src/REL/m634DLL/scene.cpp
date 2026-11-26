#include "dolphin/types.h"

#include "dolphin/mtx.h"

#include "dolphin/os.h"

#include "REL/m634DLL/scene_position.h"

#include "REL/m634DLL/act_player.h"

#include "REL/m634DLL/scene.h"
#include "REL/m634DLL/sequence_owner.h"

#include "REL/framework/game_object_man.h"

#include "REL/m634DLL/point_list_io.h"


Vector3 fn_1_ECF0(Scene *owner, s32 index);

Vector3 fn_1_EEB4(Scene *owner, s32 index);



struct PointTable {
    Vector3 entries[16];
};

struct RecordTable {
    int entries[6][16];
};

extern "C" {
extern const Vec lbl_1_rodata_46C[4];
}

extern "C" {
extern const Vec lbl_1_rodata_49C[12];
}

extern "C" {
extern const u8 lbl_1_rodata_52C[192];
}

extern "C" {
extern const RecordTable lbl_1_rodata_5EC;
}

void fn_1_E650(Scene *scene);

void fn_1_E900(Scene *scene);

void fn_1_E9CC(Scene *scene);

void fn_1_EB2C(Scene *scene);

s32 fn_1_F000(const s32 *values, s32 count, s32 value);

// Builds the scene object lists, position tables, and selection indices.
Scene::Scene()
    : GameObject(0x90)
    , firstRatio(0.0f)
    , secondRatio(0.0f)
{
    int index;

    fn_1_E650(this);
    fn_1_E900(this);
    fn_1_E9CC(this);
    for (index = 0; index < 4; index++) {
        indices[index] = -1;
    }
}

extern "C" {

const Vec lbl_1_rodata_46C[4] = {
    {-380.0f, 0.0f, -367.0f},
    {-190.0f, 0.0f, -390.0f},
    {-370.0f, 0.0f, -206.0f},
    {-457.0f, 0.0f, -30.0f},
};
const Vec lbl_1_rodata_49C[12] = {
    {-170.0f, 0.0f, 135.0f},
    {-359.0f, 0.0f, 254.0f},
    {-540.0f, 0.0f, 286.0f},
    {314.0f, 0.0f, -356.0f},
    {332.0f, 0.0f, 384.0f},
    {599.0f, 0.0f, -48.0f},
    {457.0f, 0.0f, 52.0f},
    {-500.0f, 0.0f, 413.0f},
    {551.0f, 0.0f, -161.0f},
    {192.0f, 0.0f, 78.0f},
    {163.0f, 0.0f, 96.0f},
    {-4.0f, 0.0f, 5.0f},
};
/* Decoded float vectors in lbl_1_rodata_52C:
 * {-491.0f, 0.0f, 352.0f},
 * {-317.0f, 0.0f, 352.0f},
 * {-185.0f, 0.0f, 333.0f},
 * {-24.0f, 0.0f, 374.0f},
 * {197.0f, 0.0f, 377.0f},
 * {310.0f, 0.0f, 215.0f},
 * {433.0f, 0.0f, 38.0f},
 * {549.0f, 0.0f, -108.0f},
 * {409.0f, 0.0f, -279.0f},
 * {299.0f, 0.0f, -320.0f},
 * {191.0f, 0.0f, -212.0f},
 * {105.0f, 0.0f, -68.0f},
 * {-96.0f, 0.0f, 99.0f},
 * {-158.0f, 0.0f, 208.0f},
 * {-365.0f, 0.0f, 254.0f},
 * {163.0f, 0.0f, 96.0f},
 */
const u8 lbl_1_rodata_52C[192] = { 195, 245, 128, 0, 0, 0, 0, 0, 67, 176, 0, 0, 195, 158, 128, 0, 0,
    0, 0, 0, 67, 176, 0, 0, 195, 57, 0, 0, 0, 0, 0, 0, 67, 166, 128, 0, 193, 192, 0, 0, 0, 0, 0, 0,
    67, 187, 0, 0, 67, 69, 0, 0, 0, 0, 0, 0, 67, 188, 128, 0, 67, 155, 0, 0, 0, 0, 0, 0, 67, 87, 0,
    0, 67, 216, 128, 0, 0, 0, 0, 0, 66, 24, 0, 0, 68, 9, 64, 0, 0, 0, 0, 0, 194, 216, 0, 0, 67, 204,
    128, 0, 0, 0, 0, 0, 195, 139, 128, 0, 67, 149, 128, 0, 0, 0, 0, 0, 195, 160, 0, 0, 67, 63, 0, 0,
    0, 0, 0, 0, 195, 84, 0, 0, 66, 210, 0, 0, 0, 0, 0, 0, 194, 136, 0, 0, 194, 192, 0, 0, 0, 0, 0,
    0, 66, 198, 0, 0, 195, 30, 0, 0, 0, 0, 0, 0, 67, 80, 0, 0, 195, 182, 128, 0, 0, 0, 0, 0, 67,
    126, 0, 0, 67, 35, 0, 0, 0, 0, 0, 0, 66, 192 };
const RecordTable lbl_1_rodata_5EC = { {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, -1 },
    { 0, 1, 2, 3, 4, 15, 11, 12, 13, 14, -1, 0, 0, 0, 0, 0 },
    { 5, 6, 7, 8, 9, 10, 11, 15, -1, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, -1 },
    { 0, 14, 13, 12, 11, 15, 4, 3, 2, 1, -1, 0, 0, 0, 0, 0 },
    { 5, 15, 11, 10, 9, 8, 7, 6, -1, 0, 0, 0, 0, 0, 0, 0 },
} };
}

// Releases the object and its member-owned resources.
Scene::~Scene() { }

// Refreshes the scene activity counts.
void Scene::update()
{
    fn_1_EB2C(this);
}

// Collects the scene pieces and initializes their position and home lists.
void fn_1_E650(Scene *scene)
{
    std::vector<GameObject *> firstPieces;
    std::vector<GameObject *> secondPieces;
    int firstCount;
    int secondCount;
    int firstIndex;
    int secondIndex;

    collectGameObjects(0x70, &firstPieces);
    collectGameObjects(0x80, &secondPieces);
    firstCount = firstPieces.size();
    secondCount = secondPieces.size();
    for (firstIndex = 0; firstIndex < firstCount; firstIndex++) {
        SequenceOwner *piece = static_cast<SequenceOwner *>(firstPieces[firstIndex]);
        scene->firstPieces.push_back(piece);
        SequenceOwner *selected = piece;
        Vector3 position = selected->box.position;
        scene->firstPositions.push_back(position);
        scene->firstHomes.push_back(
            static_cast<const Vector3 &>(lbl_1_rodata_46C[firstIndex]));
    }
    for (secondIndex = 0; secondIndex < secondCount; secondIndex++) {
        SequenceOwner *piece = static_cast<SequenceOwner *>(secondPieces[secondIndex]);
        scene->secondPieces.push_back(piece);
        SequenceOwner *selected = piece;
        Vector3 position = selected->box.position;
        scene->secondPositions.push_back(position);
        scene->secondHomes.push_back(
            static_cast<const Vector3 &>(lbl_1_rodata_49C[secondIndex]));
    }
    scene->firstCount = firstCount;
    scene->secondCount = secondCount;
}

// Collects the scene players.
void fn_1_E900(Scene *scene)
{
    std::vector<GameObject *> players;
    int count;
    int index;

    collectGameObjects(0x40, &players);
    count = players.size();
    for (index = 0; index < count; index++) {
        scene->players.push_back(static_cast<ActPlayer *>(players[index]));
    }
}

// Initializes the scene point and record lists.
void fn_1_E9CC(Scene *scene)
{

    RecordTable records;
    int index;
    int listIndex;

    PointTable points = *reinterpret_cast<const PointTable *>(lbl_1_rodata_52C);
    for (index = 0; index < 16; index++) {
        scene->points.push_back(points.entries[index]);
    }
    records = lbl_1_rodata_5EC;
    for (listIndex = 0; listIndex < 6; listIndex++) {
        int index;
        for (index = 0; index < 16; index++) {
            int record = records.entries[listIndex][index];
            if (record == -1) {
                break;
            }
            scene->records[listIndex].push_back(record);
        }
    }
}

// Counts the initialized scene pieces.
void fn_1_EB2C(Scene *scene)
{
    int firstActive;
    int secondActive;
    int firstIndex;
    int secondIndex;
    SequenceOwner *firstPiece;
    SequenceOwner *secondPiece;
    int firstFlag;
    int secondFlag;

    firstActive = 0;
    secondActive = 0;
    for (firstIndex = 0; firstIndex < scene->firstCount; firstIndex++) {
        firstPiece = scene->firstPieces.at(firstIndex);
        firstFlag = firstPiece->initialized;
        if ((unsigned char)firstFlag != 0) {
            firstActive++;
        }
    }
    for (secondIndex = 0; secondIndex < scene->secondCount; secondIndex++) {
        secondPiece = scene->secondPieces.at(secondIndex);
        secondFlag = secondPiece->initialized;
        if ((unsigned char)secondFlag != 0) {
            secondActive++;
        }
    }
    scene->firstRatio = (float)firstActive / (float)scene->firstCount;
    scene->secondRatio = (float)secondActive / (float)scene->secondCount;
}

// Returns the player for a scene mode, or null when outside the supported range.
ActPlayer *fn_1_EC8C(Scene *object, s32 mode)
{
    s32 index = mode - 1;
    if (index >= 3) {
        return 0;
    }
    return object->players.at(index);
}

// Returns the position of the indexed scene player.
Vector3 fn_1_ECF0(Scene *owner, s32 index)
{
    return owner->players.at(index - 1)->getPosition();
}

// Returns the first-piece activity count.
s32 Scene::fn_1_ED70()
{
    return firstCount;
}

// Returns the second-piece activity count.
s32 Scene::fn_1_ED78()
{
    return secondCount;
}

// Returns the initialized state of the indexed first piece.
u8 Scene::fn_1_ED80(s32 index)
{
    s32 count;
    SequenceOwner *selectedEntry;

    count = firstPieces.size();
    if (index > count - 1)
        return 0;
    selectedEntry = firstPieces.at(index);
    return selectedEntry->initialized;
}

// Returns the initialized state of the indexed second piece.
u8 Scene::fn_1_EDE8(s32 index)
{
    s32 count;
    SequenceOwner *selectedEntry;

    count = secondPieces.size();
    if (index > count - 1)
        return 0;
    selectedEntry = secondPieces.at(index);
    return selectedEntry->initialized;
}

// Returns an indexed first-piece position.
Vec fn_1_EE50(Scene *owner, s32 index)
{
    return owner->firstPositions.at(index);
}

// Returns an indexed second-piece position.
Vector3 fn_1_EEB4(Scene *owner, s32 index)
{
    return owner->secondPositions.at(index);
}

// Returns an indexed first-piece home position.
Vec fn_1_EF18(Scene *owner, s32 index)
{
    return owner->firstHomes.at(index);
}

// Returns an indexed second-piece home position.
Vec fn_1_EF7C(Scene *owner, s32 index)
{
    return owner->secondHomes.at(index);
}

// Returns the stored selection index.
s32 Scene::fn_1_EFE0(s32 index)
{
    return indices[index];
}

// Stores a selection index.
void Scene::fn_1_EFF0(s32 index, s32 value)
{
    indices[index] = value;
}

// Reports whether the array contains the requested value.
s32 fn_1_F000(const s32 *values, s32 count, s32 value)
{
    s32 index = 0;
    while (index < count) {
        if (values[index] == value)
            return 1;
        ++index;
    }
    return 0;
}

// Returns the indexed scene point.
Vector3 fn_1_F044(Scene *owner, s32 index)
{
    return owner->points.at(index);
}

// Wraps the record position and returns its point index.
s32 fn_1_F0A8(Scene *owner, s32 entryIndex, s32 *position)
{
    if (entryIndex > 5) {
        return -1;
    }
    s32 divisor = owner->records[entryIndex].size();
    *position %= divisor;
    return owner->records[entryIndex].at(*position);
}

// Finds the point in the opposite record list.
void fn_1_F13C(Scene *owner, s32 *listIndex, s32 *resultIndex, s32 value)
{
    s32 listCount;
    s32 index;

    *listIndex += 3;
    *listIndex %= 6;
    listCount = (s32)owner->records[*listIndex].size();

    for (index = 0; index < listCount; index++) {
        if (value == owner->records[*listIndex].at(index)) {
            *resultIndex = ((index + 1 < 0) ? -(index + 1) : index + 1) % listCount;
            break;
        }
    }
}

// Finds the nearest scene point and its matching record entry.
s32 fn_1_F23C(Scene *owner, const Vec *position, s32 *matchingGroupIndex, s32 *matchingEntryIndex,
    s32 selectedIndex)
{
    s32 pointCount = owner->points.size();
    f32 nearestFromPosition = 1000.0f;
    s32 nearestIndex = 0;
    f32 distance = 0.0f;
    s32 pointIndex = 0;
    Vec difference;
    while (pointIndex < pointCount) {
        Vector3 pointPosition = owner->points.at(pointIndex);
        PSVECSubtract(&pointPosition, position, &difference);
        difference.y = 0.0f;
        f32 magnitude = PSVECMag(&difference);
        distance = magnitude;
        if (!(nearestFromPosition < distance)) {
            nearestFromPosition = distance;
            nearestIndex = pointIndex;
        }
        pointIndex++;
    }
    Vec selectedPosition = fn_1_EF7C(owner, selectedIndex);
    s32 nearestToSelected = 0;
    nearestFromPosition = 1000.0f;
    s32 candidateIndex = 0;
    while (candidateIndex < pointCount) {
        Vector3 candidatePosition = owner->points.at(candidateIndex);
        PSVECSubtract(&candidatePosition, &selectedPosition, &difference);
        difference.y = 0.0f;
        f32 magnitude = PSVECMag(&difference);
        distance = magnitude;
        if (!(nearestFromPosition < distance)) {
            nearestFromPosition = distance;
            nearestToSelected = candidateIndex;
        }
        candidateIndex++;
    }

    s32 unk24 = 0;
    for (s32 groupIndex = 0; groupIndex < 6; groupIndex++) {
        s32 recordCount = owner->records[groupIndex].size();
        s32 matchCount = 0;
        for (s32 entryIndex = 0; entryIndex < recordCount; entryIndex++) {

            if (nearestIndex == owner->records[groupIndex].at(entryIndex)) {
                *matchingEntryIndex = entryIndex;
                matchCount++;
            }

            if (nearestToSelected == owner->records[groupIndex].at(entryIndex)) {
                matchCount++;
            }
        }
        if (matchCount == 2) {
            *matchingGroupIndex = groupIndex;
            return nearestToSelected;
        }
    }
    return -1;
}

/* Empty: its call adds an inline level to MotionStatusIsComplete. */
static inline void ReleaseCheck() { }

// Returns whether the sequence object is initialized.
static inline u8 MotionStatusIsComplete(SequenceOwner *motion)
{
    ReleaseCheck();
    return motion->initialized;
}

// Collects available motion indices near the requested position.
void fn_1_F4F8(Scene *owner, const Vec *position, s32 *motionIndices, s32 *motionIndexCount)
{
    SequenceOwner *motion;
    f32 nearestDistance = 0.0f;
    s32 nearestIndex = 0;
    s32 motionIndexCountValue = 0;
    for (s32 motionIndex = 0; motionIndex < owner->secondCount; motionIndex++) {
        u8 motionComplete;
        u8 pointComplete;
        SequenceOwner *motion2;
        SequenceOwner *pointMotion;
        s32 listCount = owner->secondPieces.size();
        if (motionIndex > listCount - 1) {
            motionComplete = 0;
        } else {
            motion = owner->secondPieces.at(motionIndex);
            motion2 = motion;
            motionComplete = MotionStatusIsComplete(motion2);
        }
        if (motionComplete != 1) {
            f32 closestDistance = 1000.0f;
            for (s32 pointIndex = 0; pointIndex < owner->secondCount; pointIndex++) {

                SequenceOwner *pointSource;
                s32 pointCount = owner->secondPieces.size();
                if (pointIndex > pointCount - 1) {
                    pointComplete = 0;
                } else {

                    pointSource = owner->secondPieces.at(pointIndex);
                    pointMotion = pointSource;
                    pointComplete = MotionStatusIsComplete(pointMotion);
                }
                if (pointComplete != 1) {

                    Vec difference;
                    Vec sampledPosition = fn_1_EF7C(owner, pointIndex);
                    PSVECSubtract(position, &sampledPosition, &difference);
                    difference.y = 0.0f;
                    f32 magnitude = PSVECMag(&difference);
                    f32 distance = magnitude;
                    if (!(closestDistance < distance) && !(nearestDistance >= distance)) {
                        closestDistance = distance;
                        nearestIndex = pointIndex;
                    }
                }
            }
            motionIndices[motionIndexCountValue] = nearestIndex;
            nearestDistance = closestDistance;
            motionIndexCountValue++;
        }
    }
    *motionIndexCount = motionIndexCountValue;
}

// Counts matching adjacent entries in the scene record lists.
u8 fn_1_F72C(Scene *owner, s32 listIndex, s32 entryIndex, s32 value)
{
    s32 listCount;
    s32 firstMatches;
    s32 firstIndex;
    s32 firstValue = fn_1_F0A8(owner, listIndex, &entryIndex);
    listCount = owner->records[listIndex].size();
    firstMatches = 0;
    firstIndex = 0;

    while (firstIndex < listCount) {
        if (value == fn_1_F0A8(owner, listIndex, &entryIndex)) {
            break;
        }
        firstIndex++;
        firstMatches++;
    }

    listIndex += 3;
    s32 adjacentCount = owner->records[listIndex].size();
    s32 adjacentIndex = 0;
    while (adjacentIndex < adjacentCount) {
        if (value == owner->records[listIndex].at(adjacentIndex)) {
            entryIndex = adjacentIndex % adjacentCount < 0 ? -(adjacentIndex % adjacentCount)
                                                           : adjacentIndex % adjacentCount;
            break;
        }
        adjacentIndex++;
    }

    s32 secondMatches = 0;
    s32 secondIndex = 0;
    while (secondIndex < listCount) {
        if (value == fn_1_F0A8(owner, listIndex, &entryIndex)) {
            break;
        }
        secondIndex++;
        secondMatches++;
    }
    if (firstMatches > secondMatches) {
        return 0;
    }
    return 1;
}
