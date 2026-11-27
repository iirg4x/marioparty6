#ifndef REL_M646DLL_MOVING_STAGE_H
#define REL_M646DLL_MOVING_STAGE_H

#include "REL/m646Dll/model_attachment.h"
#include "REL/m646Dll/collision_state.h"

typedef struct {
    HU3D_MODELID model; /* Visible layout hooked to its carrier. */
    HU3D_MODELID unk2; /* The game's code never reads this field. */
    HU3D_MODELID unk4; /* The game's code never reads this field. */
    u32 stageKind; /* Descriptor index: 0-4 normal, 5 opening, 6 special. */
    M646ModelAttachment attachments[5][12]; /* Targets grouped by contact-distance category. */
    s16 jointIndex[5]; /* Next collider name to read in each target group. */
    M646CollisionRecord collision; /* Edge geometry for the layout's stage surface. */
    u8 unk2C68[100]; /* No code names this field. */
} M646MovingStageState;

typedef struct {
    char **attachmentNames; /* Hook names for the visible targets. */
    char **colliderNames; /* Mesh names in the hidden collision model. */
    s16 attachmentCount; /* Number of targets created in this group. */
    s32 callbackType; /* Contact category, offset by target index for the opening layout. */
} M646MovingStageGroup;

typedef struct {
    s32 modelFile; /* DATA_m646 resource for the visible layout. */
    s32 hiddenModelFile; /* DATA_m646 resource for its collision meshes. */
    char *hiddenHook; /* Visible-model joint holding the hidden collision model. */
    M646MovingStageGroup groups[5]; /* Target hooks and collision categories for this layout. */
} M646MovingStageDescriptor;

typedef struct {
    u8 unk0[4]; /* No code names this field. */
    s32 stageOrder[6]; /* Opening layout followed by five randomized layouts. */
    M646MovingStageState stages[6]; /* Loaded layout and target work for each carrier. */
} M646MovingStageWork;

#endif
