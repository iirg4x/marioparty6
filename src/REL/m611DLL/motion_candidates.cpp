#include "REL/m611DLL/motion_light_owner.h"
#include "REL/framework/vector3.h"
#include "REL/m611DLL/game_requests.h"
#include "REL/framework/phase_duration_statics.h"
#include "REL/framework/seconds_to_frames_inline.h"
#include "REL/m611DLL/motion_candidate_table.h"

extern "C" {
#include "humath.h"
#include "game/hu3d.h"
#include "dolphin/gx.h"

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include <math.h>
#include <stdio.h>
}

#include "REL/framework/random.h"
#include "REL/framework/model.h"

// Writes the displacement and distance from a candidate to the query.
inline void measureMotionDistance(Vec *point, Vec *candidate,
                                  Vec *difference, f32 &distance)
{
    PSVECSubtract(point, candidate, difference);
    distance = PSVECMag(difference);
}

// Chooses a random candidate position within the horizontal scene bounds.
void fn_1_136C8(Vec *position)
{
    position->x =
        static_cast<f32>(0.7 * (100.0 * (11.0 * (2.0f * (RandomFloat() - 0.5f)))));
    position->y = 0.0f;
    position->z =
        static_cast<f32>(0.6 * (100.0 * (11.0 * (2.0f * (RandomFloat() - 0.5f)))));
}

// Initial state of each motion candidate.
const s32 lbl_1_rodata_D10[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2 };
// Number of candidate positions.
static const s32 candidateCount = sizeof(lbl_1_rodata_D10) / sizeof(s32);

// Creates the candidate draw model and initializes and separates its positions.
MotionCandidateTable::MotionCandidateTable() : OwnedModel(fn_1_15108)
{
    s32 index;
    this->setHookData(this);
    this->setLayer(4);
    for (index = 0; index < candidateCount; ++index) {
        this->states[index] = lbl_1_rodata_D10[index];
        fn_1_136C8(&this->positions[index]);
    }
    printf("Takara\n");
    fn_1_13ED4(this);
}

// Calculates horizontal offsets that separate two positions within the exclusion range.
void fn_1_13A30(MotionCandidateTable *, Vec *first, Vec *second,
               Vec *firstOffset, Vec *secondOffset)
{
    f64 differenceX;
    f64 differenceZ;
    f64 distance;
    f64 offsetScale;
    f32 angle;
    f32 offsetX;
    f32 offsetZ;

    static f64 exclusionRange = 360.0;

    differenceX = second->x - first->x;
    differenceZ = second->z - first->z;
    distance = sqrt(differenceX * differenceX +
                    differenceZ * differenceZ);
    if (distance > exclusionRange) {
        firstOffset->x = 0.0f;
        firstOffset->y = 0.0f;
        firstOffset->z = 0.0f;
        secondOffset->x = 0.0f;
        secondOffset->y = 0.0f;
        secondOffset->z = 0.0f;
        return;
    }

    if (distance < 1.0) {
        angle = (360.0f * RandomFloat()) - 180.0f;
        offsetX = static_cast<f32>(exclusionRange *
                        cos((M_PI * static_cast<f64>(angle)) / 180.0));
        offsetZ = static_cast<f32>(exclusionRange *
                        sin((M_PI * static_cast<f64>(angle)) / 180.0));
        firstOffset->x = offsetX;
        firstOffset->z = offsetZ;
        secondOffset->x = -offsetX;
        secondOffset->z = -offsetZ;
        return;
    }

    offsetScale = (0.5 * exclusionRange) / distance;
    firstOffset->x = static_cast<f32>(offsetScale * static_cast<f64>(first->x - second->x));
    firstOffset->z = static_cast<f32>(offsetScale * static_cast<f64>(first->z - second->z));
    secondOffset->x = static_cast<f32>(offsetScale * static_cast<f64>(second->x - first->x));
    secondOffset->z = static_cast<f32>(offsetScale * static_cast<f64>(second->z - first->z));
    // Prints the X component twice for each offset.
    printf("%f %f /  %f %f\n", firstOffset->x, firstOffset->x,
           secondOffset->x, secondOffset->x);
}

// Calculates one of the four perimeter positions around the candidate area.
inline void candidatePerimeterPosition(Vec *position, s32 angleIndex)
{
    f32 offsets[4] = { 0.0f, 90.0f, 270.0f, 180.0f };
    f32 angleBase;
    f32 angle;
    f64 z;
    f64 x;

    angleBase = 45.0f + (180.0f + offsets[angleIndex]);
    angle = static_cast<f32>((M_PI * static_cast<f64>(angleBase)) /
                  180.0);
    z = 800.0 * sin(static_cast<f64>(angle));
    x = 800.0 * cos(static_cast<f64>(angle));

    position->x = x;
    position->y = 0.0f;
    position->z = z;
}

// Moves a position by an offset on the ground plane.
inline void addGroundOffset(Vec *position, const Vec *offset)
{
    position->x += offset->x;
    position->z += offset->z;
}

// Separates candidate positions from each other and the four perimeter points.
void fn_1_13ED4(MotionCandidateTable *table)
{
    Vec firstOffset;
    Vec perimeter[4];
    Vec secondOffset;
    s32 pass;
    s32 firstIndex;
    s32 directionIndex;
    s32 perimeterIndex;
    s32 secondIndex;
    for (directionIndex = 0; directionIndex < 4; ++directionIndex) {
        candidatePerimeterPosition(&perimeter[directionIndex], directionIndex);
    }
    for (pass = 0; pass < 10; ++pass) {
        for (firstIndex = 0; firstIndex < candidateCount; ++firstIndex) {
            for (secondIndex = 0; secondIndex < candidateCount; ++secondIndex) {
                fn_1_13A30(table, &table->positions[firstIndex],
                           &table->positions[secondIndex], &firstOffset,
                           &secondOffset);
                addGroundOffset(&table->positions[firstIndex], &firstOffset);
                addGroundOffset(&table->positions[secondIndex], &secondOffset);
            }
            for (perimeterIndex = 0; perimeterIndex < 4; ++perimeterIndex) {
                fn_1_13A30(table, &table->positions[firstIndex],
                           &perimeter[perimeterIndex], &firstOffset,
                           &secondOffset);
                addGroundOffset(&table->positions[firstIndex], &firstOffset);
                // The first position receives the same offset a second time.
                addGroundOffset(&table->positions[firstIndex], &firstOffset);
            }
        }
    }
}

// Returns the nearest eligible candidate and its position, skipping one index.
s32 fn_1_14A30(MotionCandidateTable *table, Vec *result,
               Vec *point, s32 excludedIndex)
{
    Vec nearestPosition = { 0.0f, 0.0f, 0.0f };
    Vec difference;
    s32 nearestIndex = -1;
    f32 nearestDistance = 2200.0f;
    s32 index;

    for (index = 0; index < candidateCount; ++index) {
        if (table->states[index] >= 0 && index != excludedIndex) {
            f32 measuredDistance;
            f32 distance;

            PSVECSubtract(point, &table->positions[index], &difference);
            measuredDistance = PSVECMag(&difference);
            distance = measuredDistance;
            if (nearestDistance > distance) {
                nearestPosition = table->positions[index];
                nearestDistance = distance;
                nearestIndex = index;
            }
        }
    }
    *result = nearestPosition;
    return nearestIndex;
}

// Returns the nearest eligible candidate within the search range.
s32 fn_1_14B9C(MotionCandidateTable *table, Vec *point)
{
    // The position of the closest candidate is copied and never read.
    Vec nearestPosition = { 0.0f, 0.0f, 0.0f };
    Vec difference;
    s32 nearestIndex = -1;
    f32 nearestDistance = 2200.0f;
    s32 index;

    for (index = 0; index < candidateCount; ++index) {
        if (table->states[index] >= 0) {
            f32 distance;
            f32 measuredDistance;

            measureMotionDistance(point, &table->positions[index], &difference,
                                  measuredDistance);
            distance = measuredDistance;
            if (nearestDistance > distance) {
                nearestPosition = table->positions[index];
                nearestDistance = distance;
                nearestIndex = index;
            }
        }
    }
    return nearestIndex;
}

// Consumes and returns the state of the nearest candidate within reach.
s32 fn_1_14CD4(MotionCandidateTable *table, Vec *point)
{
    s32 resultState = -1;
    Vector3 difference;
    s32 nearestIndex = fn_1_14B9C(table, point);

    if (nearestIndex >= 0) {
        f32 verificationDistance;

        PSVECSubtract(point, &table->positions[nearestIndex], &difference);
        verificationDistance = difference.length();
        if (verificationDistance < 175.0f) {
            resultState = table->states[nearestIndex];
            table->states[nearestIndex] = -1;
        }
    }
    return resultState;
}

// Returns the state of the nearest candidate within reach.
s32 fn_1_14E80(MotionCandidateTable *table, Vec *point)
{
    Vector3 difference;
    s32 resultState = -1;
    s32 nearestIndex = fn_1_14B9C(table, point);

    if (nearestIndex >= 0) {
        f32 verificationDistance;

        PSVECSubtract(point, &table->positions[nearestIndex], &difference);
        verificationDistance = difference.length();
        if (verificationDistance < 175.0f) {
            resultState = table->states[nearestIndex];
        }
    }
    return resultState;
}

// Returns the candidate index. Nothing in this minigame calls this.
s32 MotionCandidateTable::fn_1_1501C(s32 index)
{
    s32 candidateState = -1;
    if (index >= 0 && index < candidateCount) {
        // Reads the indexed candidate state without using the result.
        candidateState = this->states[index];
    }
    return index;
}

// Chooses a random candidate and returns its index and position.
s32 fn_1_15054(MotionCandidateTable *table, Vec *position)
{
    s32 index = static_cast<s32>((static_cast<u16>(RandomUnsigned())) * candidateCount) >> 16;

    *position = table->positions[index];
    return index;
}

// Empty frame update callback for the candidate table.
void MotionCandidateTable::update()
{
}

// Acknowledges scene requests without changing the candidate table.
void MotionCandidateTable::flagEnabled(int request)
{
    acknowledgeSceneRequest(flags, request);
}

// Handles the model draw hook without drawing additional geometry.
void fn_1_15108(HU3D_MODEL *, Mtx *)
{
}

// Draws a colored annular strip at every candidate position.
// Nothing in this minigame calls this.
void fn_1_1510C(MotionCandidateTable *table, Mtx matrix)
{
    f32 vertexX[80];
    f32 vertexZ[80];
    s32 vertexCount;
    s32 segment;
    s32 candidateIndex;
    s32 vertexIndex;
    f32 angle;
    f32 radians;

    GXLoadPosMtxImm(matrix, GX_PNMTX0);
    vertexCount = 0;
    for (segment = 0; segment < 33; ++segment) {
        angle = (360.0f * (static_cast<f32>(segment & 0x1F) / 32.0f)) - 180.0f;
        angle += 90.0f;
        if (angle > 180.0f) {
            angle -= 360.0f;
        }
        radians = static_cast<f32>((M_PI * static_cast<f64>(angle)) / 180.0);
        vertexX[vertexCount] = static_cast<f32>(175.0 * cos(radians));
        vertexZ[vertexCount] = static_cast<f32>(175.0 * sin(radians));
        vertexCount += 1;
        vertexX[vertexCount] = static_cast<f32>(140.0 * cos(radians));
        vertexZ[vertexCount] = static_cast<f32>(140.0 * sin(radians));
        vertexCount += 1;
    }

    configurePositionColorDrawing(false);

    for (candidateIndex = 0; candidateIndex < candidateCount; ++candidateIndex) {
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, static_cast<u16>(vertexCount));
        for (vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
            GXPosition3f32(table->positions[candidateIndex].x + vertexX[vertexIndex],
                           5.0f,
                           table->positions[candidateIndex].z + vertexZ[vertexIndex]);
            switch (table->states[candidateIndex]) {
            case 0:
                GXColor4u8(0, 0, 255, 255);
                break;
            case 1:
                GXColor4u8(0, 255, 0, 255);
                break;
            case 2:
                GXColor4u8(255, 0, 0, 255);
                break;
            default:
                GXColor4u8(0, 0, 0, 255);
                break;
            }
        }
    }
}

// Releases the candidate model and its inherited registrations.
MotionCandidateTable::~MotionCandidateTable()
{
}
