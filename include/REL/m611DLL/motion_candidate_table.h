#ifndef REL_M611DLL_MOTION_CANDIDATE_TABLE_H
#define REL_M611DLL_MOTION_CANDIDATE_TABLE_H

extern "C" {
#include "game/hu3d.h"
}

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/model.h"
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
// Owns the candidate states, positions, draw model, and update callbacks.
struct MotionCandidateTable : RequestObject, MinigameSingleton<MotionCandidateTable>, OwnedModel {
    MotionCandidateTable();
    virtual ~MotionCandidateTable();
    virtual void flagEnabled(int);
    virtual void update();
    s32 fn_1_1501C(s32 index);
    s32 states[20];
    Vec positions[20];
};
void fn_1_15108(HU3D_MODEL *, Mtx *);
void fn_1_13ED4(MotionCandidateTable *table);
void fn_1_1510C(MotionCandidateTable *table, Mtx matrix);
s32 fn_1_14A30(MotionCandidateTable *table, Vec *result, Vec *point, s32 excludedIndex);
s32 fn_1_14CD4(MotionCandidateTable *table, Vec *point);
s32 fn_1_14E80(MotionCandidateTable *table, Vec *point);
s32 fn_1_15054(MotionCandidateTable *table, Vec *position);
#endif
