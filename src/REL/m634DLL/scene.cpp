#include "dolphin/types.h"

#include "dolphin/mtx.h"

#include "dolphin/os.h"

#include "REL/m634DLL/scene_position.h"

#include "REL/m634DLL/act_player.h"

#include "REL/m634DLL/scene.h"

#include "REL/framework/game_object_man.h"

#include "REL/m634DLL/point_list_io.h"

#include "REL/m634DLL/position_list.h"

ActPlayer *fn_1_EC8C(Scene *object, s32 mode);

Vector3 fn_1_ECF0(Scene *owner, s32 index);

Vector3 fn_1_EEB4(Scene *owner, s32 index);

Vector3 fn_1_F044(Scene *owner, s32 index);

s32 fn_1_F0A8(Scene *owner, s32 entryIndex, s32 *position);

struct PointTable {
    Vector3 entries[16];
};

struct RecordTable {
    int entries[6][16];
};

extern "C" {extern const u8 lbl_1_rodata_46C[48];
}

extern "C" {extern const u8 lbl_1_rodata_49C[144];
}

extern "C" {extern const u8 lbl_1_rodata_52C[192];
}

extern "C" {extern const u8 lbl_1_rodata_5EC[388];
}

void fn_1_E650(Scene *scene);

void fn_1_E900(Scene *scene);

void fn_1_E9CC(Scene *scene);

void fn_1_EB2C(Scene *scene);

s32 fn_1_F000(const s32 *values, s32 count, s32 value);

Scene::Scene() : GameObject(0x90), firstRatio(0.0f), secondRatio(0.0f)
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

const u8 lbl_1_rodata_46C[48] = {
    195, 190, 0, 0, 0, 0, 0, 0, 195, 183, 128, 0, 195, 62, 0, 0, 0, 0, 0, 0, 195, 195, 0,
    0, 195, 185, 0, 0, 0, 0, 0, 0, 195, 78, 0, 0, 195, 228, 128, 0, 0, 0, 0, 0, 193, 240
};
const u8 lbl_1_rodata_49C[144] = {
    195, 42, 0, 0, 0, 0, 0, 0, 67, 7, 0, 0, 195, 179, 128, 0, 0, 0, 0, 0, 67, 126, 0, 0,
    196, 7, 0, 0, 0, 0, 0, 0, 67, 143, 0, 0, 67, 157, 0, 0, 0, 0, 0, 0, 195, 178, 0, 0, 67,
    166, 0, 0, 0, 0, 0, 0, 67, 192, 0, 0, 68, 21, 192, 0, 0, 0, 0, 0, 194, 64, 0, 0, 67,
    228, 128, 0, 0, 0, 0, 0, 66, 80, 0, 0, 195, 250, 0, 0, 0, 0, 0, 0, 67, 206, 128, 0, 68,
    9, 192, 0, 0, 0, 0, 0, 195, 33, 0, 0, 67, 64, 0, 0, 0, 0, 0, 0, 66, 156, 0, 0, 67, 35,
    0, 0, 0, 0, 0, 0, 66, 192, 0, 0, 192, 128, 0, 0, 0, 0, 0, 0, 64, 160
};
const u8 lbl_1_rodata_52C[192] = {
    195, 245, 128, 0, 0, 0, 0, 0, 67, 176, 0, 0, 195, 158, 128, 0, 0, 0, 0, 0, 67, 176, 0,
    0, 195, 57, 0, 0, 0, 0, 0, 0, 67, 166, 128, 0, 193, 192, 0, 0, 0, 0, 0, 0, 67, 187, 0,
    0, 67, 69, 0, 0, 0, 0, 0, 0, 67, 188, 128, 0, 67, 155, 0, 0, 0, 0, 0, 0, 67, 87, 0, 0,
    67, 216, 128, 0, 0, 0, 0, 0, 66, 24, 0, 0, 68, 9, 64, 0, 0, 0, 0, 0, 194, 216, 0, 0,
    67, 204, 128, 0, 0, 0, 0, 0, 195, 139, 128, 0, 67, 149, 128, 0, 0, 0, 0, 0, 195, 160,
    0, 0, 67, 63, 0, 0, 0, 0, 0, 0, 195, 84, 0, 0, 66, 210, 0, 0, 0, 0, 0, 0, 194, 136, 0,
    0, 194, 192, 0, 0, 0, 0, 0, 0, 66, 198, 0, 0, 195, 30, 0, 0, 0, 0, 0, 0, 67, 80, 0, 0,
    195, 182, 128, 0, 0, 0, 0, 0, 67, 126, 0, 0, 67, 35, 0, 0, 0, 0, 0, 0, 66, 192
};
const u8 lbl_1_rodata_5EC[388] = {
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 6, 0,
    0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 13, 0,
    0, 0, 14, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0,
    4, 0, 0, 0, 15, 0, 0, 0, 11, 0, 0, 0, 12, 0, 0, 0, 13, 0, 0, 0, 14, 255, 255, 255, 255,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 6, 0,
    0, 0, 7, 0, 0, 0, 8, 0, 0, 0, 9, 0, 0, 0, 10, 0, 0, 0, 11, 0, 0, 0, 15, 255, 255, 255,
    255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 14, 0, 0, 0, 13, 0, 0, 0, 12, 0, 0, 0, 11, 0, 0, 0, 10, 0, 0,
    0, 9, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0, 6, 0, 0, 0, 5, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0,
    2, 0, 0, 0, 1, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 14, 0, 0, 0, 13, 0, 0, 0, 12,
    0, 0, 0, 11, 0, 0, 0, 15, 0, 0, 0, 4, 0, 0, 0, 3, 0, 0, 0, 2, 0, 0, 0, 1, 255, 255,
    255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0,
    0, 15, 0, 0, 0, 11, 0, 0, 0, 10, 0, 0, 0, 9, 0, 0, 0, 8, 0, 0, 0, 7, 0, 0, 0, 6, 255,
    255, 255, 255
};

}

Scene::~Scene() {}

void Scene::update()
{
    fn_1_EB2C(this);
}

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
        Piece *piece = static_cast<Piece *>(firstPieces[firstIndex]);
        scene->firstPieces.push_back(piece);
        Piece *selected = piece;
        Vector3 position = selected->position;
        scene->firstPositions.push_back(position);
        scene->firstHomes.push_back(reinterpret_cast<const Vector3 *>(lbl_1_rodata_46C)[firstIndex]);
    }
    for (secondIndex = 0; secondIndex < secondCount; secondIndex++) {
        Piece *piece = static_cast<Piece *>(secondPieces[secondIndex]);
        scene->secondPieces.push_back(piece);
        Piece *selected = piece;
        Vector3 position = selected->position;
        scene->secondPositions.push_back(position);
        scene->secondHomes.push_back(reinterpret_cast<const Vector3 *>(lbl_1_rodata_49C)[secondIndex]);
    }
    scene->firstCount = firstCount;
    scene->secondCount = secondCount;
}

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

void fn_1_E9CC(Scene *scene)
{

    RecordTable records;
    int index;
    int listIndex;

    PointTable points = *reinterpret_cast<const PointTable *>(lbl_1_rodata_52C);
    for (index = 0; index < 16; index++) {
        scene->points.push_back(points.entries[index]);
    }
    records = *reinterpret_cast<const RecordTable *>(lbl_1_rodata_5EC);
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

void fn_1_EB2C(Scene *scene)
{
    int firstActive;
    int secondActive;
    int firstIndex;
    int secondIndex;
    Piece *firstPiece;
    Piece *secondPiece;
    int firstFlag;
    int secondFlag;

    firstActive = 0;
    secondActive = 0;
    for (firstIndex = 0; firstIndex < scene->firstCount; firstIndex++) {
        firstPiece = scene->firstPieces.at(firstIndex);
        firstFlag = firstPiece->active;
        if ((unsigned char)firstFlag != 0) {
            firstActive++;
        }
    }
    for (secondIndex = 0; secondIndex < scene->secondCount; secondIndex++) {
        secondPiece = scene->secondPieces.at(secondIndex);
        secondFlag = secondPiece->active;
        if ((unsigned char)secondFlag != 0) {
            secondActive++;
        }
    }
    scene->firstRatio = (float)firstActive / (float)scene->firstCount;
    scene->secondRatio = (float)secondActive / (float)scene->secondCount;
}

ActPlayer *fn_1_EC8C(Scene *object, s32 mode)
{
    s32 index = mode - 1;
    if (index >= 3) {
        return 0;
    }
    return object->players.at(index);
}

Vector3 fn_1_ECF0(Scene *owner, s32 index)
{
    return owner->players.at(index - 1)->getPosition();
}

s32 Scene::fn_1_ED70() {
    return firstCount;
}

s32 Scene::fn_1_ED78() {
    return secondCount;
}

u8 Scene::fn_1_ED80(s32 index)
{
    s32 count;
    Piece *selectedEntry;
    Piece **entry;

    count = firstPieces.size();
    if (index > count - 1) return 0;
    selectedEntry = firstPieces.at(index);
    return selectedEntry->active;
}

u8 Scene::fn_1_EDE8(s32 index)
{
    s32 count;
    Piece *selectedEntry;
    Piece **entry;

    count = secondPieces.size();
    if (index > count - 1) return 0;
    selectedEntry = secondPieces.at(index);
    return selectedEntry->active;
}

Vec fn_1_EE50(Scene *owner, s32 index) { return owner->firstPositions.at(index); }

Vector3 fn_1_EEB4(Scene *owner, s32 index)
{ return owner->secondPositions.at(index); }

Vec fn_1_EF18(Scene *owner, s32 index) { return owner->firstHomes.at(index); }

Vec fn_1_EF7C(Scene *owner, s32 index)
{ return owner->secondHomes.at(index); }

s32 Scene::fn_1_EFE0(s32 index) {
    return indices[index];
}

void Scene::fn_1_EFF0(s32 index, s32 value) {
    indices[index] = value;
}

s32 fn_1_F000(const s32 *values, s32 count, s32 value)
{
    s32 index = 0;
    while (index < count) {
        if (values[index] == value) return 1;
        ++index;
    }
    return 0;
}

Vector3 fn_1_F044(Scene *owner, s32 index)
{ return owner->points.at(index); }

s32 fn_1_F0A8(Scene *owner, s32 entryIndex, s32 *position)
{
    if (entryIndex > 5) { return -1; }
    s32 divisor = owner->records[entryIndex].size();
    *position %= divisor;
    return owner->records[entryIndex].at(*position);
}

static inline int *ListEntry(std::vector<int> *list, s32 index)
{
    return &list->at(index);
}

void fn_1_F13C(Scene *owner, s32 *listIndex, s32 *resultIndex,
               s32 value)
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

s32 fn_1_F23C(Scene *owner, const Vec *position,
                         s32 *matchingGroupIndex, s32 *matchingEntryIndex,
                         s32 selectedIndex)
{
    s32 pointCount = owner->points.size();
    f32 nearestFromPosition = 1000.0f;
    s32 nearestIndex = 0;
    f32 distance = 0.0f;
    s32 pointIndex = 0;
    Vec difference;
    while (pointIndex < pointCount)
    {
        Vector3 pointPosition = owner->points.at(pointIndex);
        PSVECSubtract(& pointPosition, position, & difference);
        difference.y = 0.0f;
        f32 magnitude = PSVECMag(& difference);
        distance = magnitude;
        if (nearestFromPosition < distance)
        {
        }
        else
        {
            nearestFromPosition = distance;
            nearestIndex = pointIndex;
        }
        pointIndex++;
    }
    Vec selectedPosition = fn_1_EF7C(owner, selectedIndex);
    s32 nearestToSelected = 0;
    nearestFromPosition = 1000.0f;
    s32 candidateIndex = 0;
    while (candidateIndex < pointCount)
    {
        Vector3 candidatePosition = owner->points.at(candidateIndex);
        PSVECSubtract(& candidatePosition, & selectedPosition, & difference);
        difference.y = 0.0f;
        f32 magnitude = PSVECMag(& difference);
        distance = magnitude;
        if (nearestFromPosition < distance)
        {
        }
        else
        {
            nearestFromPosition = distance;
            nearestToSelected = candidateIndex;
        }
        candidateIndex++;
    }

    s32 unk24 = 0;
    for (s32 groupIndex = 0; groupIndex < 6; groupIndex++)
    {
        s32 recordCount = owner->records[groupIndex].size();
        s32 matchCount = 0;
        for (s32 entryIndex = 0; entryIndex < recordCount; entryIndex++)
        {

            if (nearestIndex == owner->records[groupIndex].at(entryIndex))
            {
                * matchingEntryIndex = entryIndex;
                matchCount++;
            }

            if (nearestToSelected == owner->records[groupIndex].at(entryIndex))
            {
                matchCount++;
            }
        }
        if (matchCount == 2)
        {
            * matchingGroupIndex = groupIndex;
            return nearestToSelected;
        }
    }
    return - 1;
}

/* Debug hook: empty in the release build. */
static inline void ReleaseCheck() { }

static inline u8 MotionStatusIsComplete(Piece *motion)
{
    ReleaseCheck();
    return motion->active;
}

void fn_1_F4F8(Scene *owner, const Vec *position,
                           s32 *motionIndices, s32 *motionIndexCount)
{
    Piece *motion;
    f32 nearestDistance = 0.0f;
    s32 nearestIndex = 0;
    s32 motionIndexCountValue = 0;
    for (s32 motionIndex = 0; motionIndex < owner->secondCount; motionIndex++)
    {
        u8 motionComplete;
        u8 pointComplete;
        Piece *motion2;
        Piece *pointMotion;
        Piece **entry;
        s32 listCount = owner->secondPieces.size();
        if (motionIndex > listCount - 1)
        {
            motionComplete = 0;
        }
        else
        {
            motion = owner->secondPieces.at(motionIndex);
            motion2 = motion;
            motionComplete = MotionStatusIsComplete(motion2);
        }
        if (motionComplete != 1)
        {
            f32 closestDistance = 1000.0f;
            for (s32 pointIndex = 0; pointIndex < owner->secondCount; pointIndex++)
            {

                Piece *pointSource;
                s32 pointCount = owner->secondPieces.size();
                if (pointIndex > pointCount - 1)
                {
                    pointComplete = 0;
                }
                else
                {

                    pointSource = owner->secondPieces.at(pointIndex);
                    pointMotion = pointSource;
                    pointComplete = MotionStatusIsComplete(pointMotion);
                }
                if (pointComplete != 1)
                {

                    Vec difference;
                                    Vec sampledPosition = fn_1_EF7C(owner, pointIndex);
                    PSVECSubtract(position, & sampledPosition, & difference);
                    difference.y = 0.0f;
                    f32 magnitude = PSVECMag(& difference);
                    f32 distance = magnitude;
                    if (! (closestDistance < distance) && ! (nearestDistance >= distance))
                    {
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
    * motionIndexCount = motionIndexCountValue;
}

u8 fn_1_F72C(Scene *owner, s32 listIndex,
                           s32 entryIndex, s32 value)
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
        if (value == *ListEntry(&owner->records[listIndex], adjacentIndex)) {
            entryIndex = adjacentIndex % adjacentCount < 0
                ? -(adjacentIndex % adjacentCount)
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
