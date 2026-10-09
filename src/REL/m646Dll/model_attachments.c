/* Loads the moving stage models and their attached collision and animation data. */
#include "dolphin/math.h"
#include "REL/m646Dll/module_types.h"

#define M646_CARRIER_OPENING_MODEL_FILE 48
#define M646_CARRIER_OPENING_MOTION_FILE 49
#define M646_CARRIER_A_MODEL_FILE 50
#define M646_CARRIER_A_MOTION_FILE 51
#define M646_CARRIER_B_MODEL_FILE 52
#define M646_CARRIER_B_MOTION_FILE 53
#define M646_CARRIER_C_MODEL_FILE 54
#define M646_CARRIER_C_MOTION_FILE 55
#define M646_CARRIER_D_MODEL_FILE 56
#define M646_CARRIER_D_MOTION_FILE 57
#define M646_CARRIER_E_MODEL_FILE 58
#define M646_CARRIER_E_MOTION_FILE 59
#define M646_LAYOUT_OPENING_MODEL_FILE 60
#define M646_LAYOUT_A_MODEL_FILE 61
#define M646_LAYOUT_B_MODEL_FILE 62
#define M646_LAYOUT_C_MODEL_FILE 63
#define M646_LAYOUT_D_MODEL_FILE 64
#define M646_LAYOUT_E_MODEL_FILE 65
#define M646_LAYOUT_SPECIAL_MODEL_FILE 66
#define M646_LAYOUT_OPENING_COLLISION_FILE 67
#define M646_LAYOUT_A_COLLISION_FILE 68
#define M646_LAYOUT_B_COLLISION_FILE 69
#define M646_LAYOUT_C_COLLISION_FILE 70
#define M646_LAYOUT_D_COLLISION_FILE 71
#define M646_LAYOUT_E_COLLISION_FILE 72
#define M646_LAYOUT_SPECIAL_COLLISION_FILE 73
#define M646_OPENING_TARGET_P1_MODEL_FILE 74
#define M646_OPENING_TARGET_P1_MOTION_FILE 75
#define M646_OPENING_TARGET_P2_MODEL_FILE 76
#define M646_OPENING_TARGET_P2_MOTION_FILE 77
#define M646_OPENING_TARGET_P3_MODEL_FILE 78
#define M646_OPENING_TARGET_P3_MOTION_FILE 79
#define M646_OPENING_TARGET_P4_MODEL_FILE 80
#define M646_OPENING_TARGET_P4_MOTION_FILE 81
#define M646_TARGET_TEN_POINT_MODEL_FILE 82
#define M646_TARGET_TEN_POINT_MOTION_FILE 83
#define M646_TARGET_THIRTY_POINT_MODEL_FILE 84
#define M646_TARGET_THIRTY_POINT_MOTION_FILE 85
#define M646_TARGET_FIFTY_POINT_MODEL_FILE 86
#define M646_TARGET_FIFTY_POINT_MOTION_FILE 87
#define M646_TARGET_HUNDRED_POINT_MODEL_FILE 88
#define M646_TARGET_HUNDRED_POINT_MOTION_FILE 89
#define M646_TARGET_SCORE_RESET_MODEL_FILE 90
#define M646_TARGET_SCORE_RESET_MOTION_FILE 91
#define M646_STAGE_ORDER_MASK 0x1F

typedef struct M646ActorAnchor_s M646ActorAnchor;
typedef struct M646ModelCollider_s M646ModelCollider;
typedef struct M646ModelAttachment_s M646ModelAttachment;

typedef struct M646MeshEdge_s {
    Vec first; /* First source-mesh endpoint in local coordinates. */
    Vec second; /* Second source-mesh endpoint in local coordinates. */
    Mtx *matrix; /* Matrix that transforms this edge into world coordinates. */
} M646MeshEdge;

typedef struct M646WorldEdge_s {
    Vec first; /* First endpoint in world coordinates. */
    Vec second; /* Second endpoint in world coordinates. */
    Vec normal; /* Plane normal used by the contact tests. */
    f32 planeConstant; /* Plane offset in stage units. */
} M646WorldEdge;

typedef struct M646CollisionRecord_s {
    u8 stageCollisionPrefix[32]; /* Collision flags, owner, and callbacks before the edge list. */
    s32 edgeCount; /* Number of collision edges. */
    M646WorldEdge *current; /* World edges used by the current contact pass. */
    M646WorldEdge *alternate; /* Spare edge buffer swapped during a transform update. */
    M646MeshEdge *original; /* Source-mesh endpoints retained for transforms. */
    s32 transformCount; /* Number of source objects with transforms. */
    HSF_TRANSFORM **transforms; /* Source-object transforms used by the stage edges. */
    Mtx *matrices; /* Matrices that transform the source edges. */
} M646CollisionRecord;

struct M646ModelCollider_s {
    u32 kind; /* Registration and active-contact flags. */
    s32 group; /* Group bits used to filter projectile contacts. */
    s32 callbackType; /* Target category selecting points and impact sounds. */
    s32 unusedPlayerIndex; /* Player-index slot unused by stage targets. */
    s32 callbackMode; /* 11 selects model-target collision processing. */
    M646ModelAttachment *owner; /* Target attachment passed to the callbacks. */
    void (*update)(M646ModelAttachment *); /* Unused target-update callback; setup assigns an empty
                                            * function. */
    s32 (*contact)(M646ModelCollider *, M646ActorAnchor *); /* Handles a projectile contact. */
    s32 edgeCount; /* Number of collision edges. */
    M646WorldEdge *current; /* World edges used by the current contact pass. */
    M646WorldEdge *alternate; /* Spare edge buffer. */
    s32 transformCount; /* Number of transforms applied to the target geometry. */
    HSF_TRANSFORM pose; /* Joint position and scale used for contact checks. */
    Mtx *matrices; /* Matrices for the target collision geometry. */
    u8 contactEdgeStorage[4]; /* Collision manager stores the contacted edge pointer here. */
    s16 category; /* Target group selecting the contact-distance threshold. */
    HU3D_MODELID model; /* Carrier model from which the target joint is sampled. */
    char jointName[64]; /* Carrier joint used to follow this target. */
    HSF_OBJECT *source; /* Hidden mesh used to initialize collision geometry. */
    s32 assigned; /* Nonzero while a computer player has reserved this target. */
};

struct M646ModelAttachment_s {
    HU3D_MODELID parentModel; /* Hook parent, or -1 after the first projectile contact. */
    HU3D_MODELID model; /* Visible target model. */
    HU3D_MOTIONID motion; /* Hit animation, paused until first contact. */
    s16 state; /* Number of contacts; zero means still attached and unhit. */
    M646ModelCollider collider; /* Contact geometry and callbacks for this target. */
    Vec position; /* World position captured when the target detaches. */
};

typedef struct M646MovingStageState_s {
    HU3D_MODELID model; /* Visible layout hooked to its carrier. */
    HU3D_MODELID hiddenModel; /* Invisible model supplying collision meshes. */
    HU3D_MODELID parentModel; /* Carrier model holding this layout. */
    s16 modelPadding; /* Unused storage between model IDs and the layout kind. */
    u32 stageKind; /* Descriptor index: 0-4 normal, 5 opening, 6 special. */
    M646ModelAttachment attachments[5][12]; /* Targets grouped by contact-distance category. */
    s16 jointIndex[5]; /* Next collider name to read in each target group. */
    u8 jointIndexPadding[2]; /* Unused storage after the collider-name indices. */
    M646CollisionRecord collision; /* Edge geometry for the layout's stage surface. */
    u8 reservedStageStorage[104]; /* Remaining storage not accessed by these stage callbacks. */
} M646MovingStageState;

typedef struct M646MovingStageGroup_s {
    char **attachmentNames; /* Hook names for the visible targets. */
    char **colliderNames; /* Mesh names in the hidden collision model. */
    s16 attachmentCount; /* Number of targets created in this group. */
    s32 callbackType; /* Contact category, offset by target index for the opening layout. */
} M646MovingStageGroup;

typedef struct M646MovingStageDescriptor_s {
    s32 modelFile; /* DATA_m646 resource for the visible layout. */
    s32 hiddenModelFile; /* DATA_m646 resource for its collision meshes. */
    char *hiddenHook; /* Visible-model joint holding the hidden collision model. */
    M646MovingStageGroup groups[5]; /* Target hooks and collision categories for this layout. */
} M646MovingStageDescriptor;

typedef struct M646MovingStageWork_s {
    s32 state; /* Cleared at startup; not read by these stage callbacks. */
    s32 stageOrder[6]; /* Opening layout followed by five randomized layouts. */
    M646MovingStageState stages[6]; /* Loaded layout and target work for each carrier. */
} M646MovingStageWork;

extern OMOBJ *lbl_1_bss_80;
extern s32 lbl_1_data_1D38[6];
extern s32 lbl_1_data_1D50[6];
extern s32 lbl_1_data_2C0[4];
extern s32 lbl_1_data_2D0[4];
extern s32 lbl_1_data_2E0[5];
extern s32 lbl_1_data_2F4[5];
extern char lbl_1_data_308[7][30];
extern char *lbl_1_data_468[6];
extern M646PositionTable lbl_1_bss_84;
extern M646MovingStageDescriptor lbl_1_data_1AB4[7];
void fn_1_8ACC(OMOBJ *obj);
void fn_1_8C88(OMOBJ *obj);
void fn_1_8380(OMOBJ *obj, s32 index);
void fn_1_89EC(OMOBJ *obj);
void fn_1_4DD0(M646ModelCollider *collider);
s32 fn_1_48DC(M646ModelCollider *collider, HSF_OBJECT *object, HU3D_MODELID model, char *name);
void fn_1_436C(M646CollisionRecord *record, HSF_OBJECT *object);
void fn_1_43A0(M646CollisionRecord *record);
s32 fn_1_3EF8(void *record);
s32 fn_1_8DAC(M646ModelCollider *collider, M646ActorAnchor *actor);
void fn_1_8E74(M646ModelAttachment *attachment);

void fn_1_8158(OMOBJMAN *objman);
void fn_1_81F8(void);
void fn_1_81FC(s16 index, Point3d *out);
void fn_1_8270(OMOBJ *obj);
void fn_1_8380(OMOBJ *obj, s32 index);
void fn_1_8800(OMOBJ *obj);
void fn_1_8914(void);
void fn_1_8980(void);
void fn_1_89EC(OMOBJ *obj);
void fn_1_8ACC(OMOBJ *obj);
void fn_1_8C88(OMOBJ *obj);
s32 fn_1_8DAC(M646ModelCollider *collider, M646ActorAnchor *actor);
void fn_1_8E74(M646ModelAttachment *attachment);

M646PositionTable lbl_1_bss_84;

/* Model resources for the four opening targets, in player order. */
s32 lbl_1_data_2C0[4] = {
    DATANUM(DATA_m646, M646_OPENING_TARGET_P1_MODEL_FILE),
    DATANUM(DATA_m646, M646_OPENING_TARGET_P2_MODEL_FILE),
    DATANUM(DATA_m646, M646_OPENING_TARGET_P3_MODEL_FILE),
    DATANUM(DATA_m646, M646_OPENING_TARGET_P4_MODEL_FILE)
};

/* Hit-motion resources for the four opening targets. */
s32 lbl_1_data_2D0[4] = {
    DATANUM(DATA_m646, M646_OPENING_TARGET_P1_MOTION_FILE),
    DATANUM(DATA_m646, M646_OPENING_TARGET_P2_MOTION_FILE),
    DATANUM(DATA_m646, M646_OPENING_TARGET_P3_MOTION_FILE),
    DATANUM(DATA_m646, M646_OPENING_TARGET_P4_MOTION_FILE)
};

/* Target models for the 10-, 30-, 50-, 100-point, and score-reset categories. */
s32 lbl_1_data_2E0[5] = {
    DATANUM(DATA_m646, M646_TARGET_TEN_POINT_MODEL_FILE),
    DATANUM(DATA_m646, M646_TARGET_THIRTY_POINT_MODEL_FILE),
    DATANUM(DATA_m646, M646_TARGET_FIFTY_POINT_MODEL_FILE),
    DATANUM(DATA_m646, M646_TARGET_HUNDRED_POINT_MODEL_FILE),
    DATANUM(DATA_m646, M646_TARGET_SCORE_RESET_MODEL_FILE)
};

/* Hit motions corresponding to the five target categories above. */
s32 lbl_1_data_2F4[5] = {
    DATANUM(DATA_m646, M646_TARGET_TEN_POINT_MOTION_FILE),
    DATANUM(DATA_m646, M646_TARGET_THIRTY_POINT_MOTION_FILE),
    DATANUM(DATA_m646, M646_TARGET_FIFTY_POINT_MOTION_FILE),
    DATANUM(DATA_m646, M646_TARGET_HUNDRED_POINT_MOTION_FILE),
    DATANUM(DATA_m646, M646_TARGET_SCORE_RESET_MOTION_FILE)
};

/* Stage-surface collision mesh names for each layout descriptor. */
char lbl_1_data_308[7][30] = {
    "no646matoate-n1_colA",
    "no646matoate-n2_colA",
    "no646matoate-n3_colA",
    "no646matoate-n4_colA",
    "no646matoate-n5_colA",
    "no646matoate-kaisi_colA",
    "no646matoate-luc_colA"
};

char lbl_1_data_3DA[] = "no646matoate-matkaisi_null";

char lbl_1_data_3F5[] = "no646matoate-matA_null";

char lbl_1_data_40C[] = "no646matoate-matB_null";

char lbl_1_data_423[] = "no646matoate-matC_null";

char lbl_1_data_43A[] = "no646matoate-matD_null";

char lbl_1_data_451[] = "no646matoate-matE_root";

char *lbl_1_data_468[6] = { lbl_1_data_3DA, lbl_1_data_3F5, lbl_1_data_40C,
                            lbl_1_data_423, lbl_1_data_43A, lbl_1_data_451 };

char lbl_1_data_480[] = "no646matoate-n1_nullR";

char lbl_1_data_496[] = "no646matoate-n1_nullD";

char lbl_1_data_4AC[] = "no646matoate-n1_nullE";

char lbl_1_data_4C2[] = "no646matoate-n1_nullF";

char lbl_1_data_4D8[] = "no646matoate-n1_nullG";

char *lbl_1_data_4F0[4] = {lbl_1_data_496, lbl_1_data_4AC, lbl_1_data_4C2, lbl_1_data_4D8};

char lbl_1_data_500[] = "no646matoate-n1_colD";

char lbl_1_data_515[] = "no646matoate-n1_colE";

char lbl_1_data_52A[] = "no646matoate-n1_colF";

char lbl_1_data_53F[] = "no646matoate-n1_colG";

char *lbl_1_data_554[4] = {lbl_1_data_500, lbl_1_data_515, lbl_1_data_52A, lbl_1_data_53F};

char lbl_1_data_564[] = "no646matoate-n1_nullH";

char lbl_1_data_57A[] = "no646matoate-n1_nullI";

char lbl_1_data_590[] = "no646matoate-n1_nullJ";

char *lbl_1_data_5A8[3] = {lbl_1_data_564, lbl_1_data_57A, lbl_1_data_590};

char lbl_1_data_5B4[] = "no646matoate-n1_colH";

char lbl_1_data_5C9[] = "no646matoate-n1_colI";

char lbl_1_data_5DE[] = "no646matoate-n1_colJ";

char *lbl_1_data_5F4[3] = {lbl_1_data_5B4, lbl_1_data_5C9, lbl_1_data_5DE};

char lbl_1_data_600[] = "no646matoate-n1_nullK";

char lbl_1_data_616[] = "no646matoate-n1_nullL";

char lbl_1_data_62C[] = "no646matoate-n1_nullM";

char *lbl_1_data_644[3] = {lbl_1_data_600, lbl_1_data_616, lbl_1_data_62C};

char lbl_1_data_650[] = "no646matoate-n1_colK";

char lbl_1_data_665[] = "no646matoate-n1_colL";

char lbl_1_data_67A[] = "no646matoate-n1_colM";

char *lbl_1_data_690[3] = {lbl_1_data_650, lbl_1_data_665, lbl_1_data_67A};

char lbl_1_data_69C[] = "no646matoate-n1_nullN";

char lbl_1_data_6B2[] = "no646matoate-n1_nullO";

char lbl_1_data_6C8[] = "no646matoate-n1_nullP";

char lbl_1_data_6DE[] = "no646matoate-n1_nullQ";

char *lbl_1_data_6F4[4] = {lbl_1_data_69C, lbl_1_data_6B2, lbl_1_data_6C8, lbl_1_data_6DE};

char lbl_1_data_704[] = "no646matoate-n1_colN";

char lbl_1_data_719[] = "no646matoate-n1_colO";

char lbl_1_data_72E[] = "no646matoate-n1_colP";

char lbl_1_data_743[] = "no646matoate-n1_colQ";

char *lbl_1_data_758[4] = {lbl_1_data_704, lbl_1_data_719, lbl_1_data_72E, lbl_1_data_743};

char lbl_1_data_768[] = "no646matoate-n1_nullA";

char lbl_1_data_77E[] = "no646matoate-n1_nullB";

char lbl_1_data_794[] = "no646matoate-n1_nullC";

char *lbl_1_data_7AC[3] = {lbl_1_data_768, lbl_1_data_77E, lbl_1_data_794};

char lbl_1_data_7B8[] = "no646matoate-n1_colA";

char lbl_1_data_7CD[] = "no646matoate-n1_colB";

char lbl_1_data_7E2[] = "no646matoate-n1_colC";

char *lbl_1_data_7F8[3] = {lbl_1_data_7B8, lbl_1_data_7CD, lbl_1_data_7E2};

char lbl_1_data_804[] = "no646matoate-n2_nullR";

char lbl_1_data_81A[] = "no646matoate-n2_nullD";

char lbl_1_data_830[] = "no646matoate-n2_nullE";

char lbl_1_data_846[] = "no646matoate-n2_nullF";

char lbl_1_data_85C[] = "no646matoate-n2_nullG";

char *lbl_1_data_874[4] = {lbl_1_data_81A, lbl_1_data_830, lbl_1_data_846, lbl_1_data_85C};

char lbl_1_data_884[] = "no646matoate-n2_colD";

char lbl_1_data_899[] = "no646matoate-n2_colE";

char lbl_1_data_8AE[] = "no646matoate-n2_colF";

char lbl_1_data_8C3[] = "no646matoate-n2_colG";

char *lbl_1_data_8D8[4] = {lbl_1_data_884, lbl_1_data_899, lbl_1_data_8AE, lbl_1_data_8C3};

char lbl_1_data_8E8[] = "no646matoate-n2_nullH";

char lbl_1_data_8FE[] = "no646matoate-n2_nullI";

char lbl_1_data_914[] = "no646matoate-n2_nullJ";

char *lbl_1_data_92C[3] = {lbl_1_data_8E8, lbl_1_data_8FE, lbl_1_data_914};

char lbl_1_data_938[] = "no646matoate-n2_colH";

char lbl_1_data_94D[] = "no646matoate-n2_colI";

char lbl_1_data_962[] = "no646matoate-n2_colJ";

char *lbl_1_data_978[3] = {lbl_1_data_938, lbl_1_data_94D, lbl_1_data_962};

char lbl_1_data_984[] = "no646matoate-n2_nullK";

char lbl_1_data_99A[] = "no646matoate-n2_nullL";

char lbl_1_data_9B0[] = "no646matoate-n2_nullM";

char *lbl_1_data_9C8[3] = {lbl_1_data_984, lbl_1_data_99A, lbl_1_data_9B0};

char lbl_1_data_9D4[] = "no646matoate-n2_colK";

char lbl_1_data_9E9[] = "no646matoate-n2_colL";

char lbl_1_data_9FE[] = "no646matoate-n2_colM";

char *lbl_1_data_A14[3] = {lbl_1_data_9D4, lbl_1_data_9E9, lbl_1_data_9FE};

char lbl_1_data_A20[] = "no646matoate-n2_nullN";

char lbl_1_data_A36[] = "no646matoate-n2_nullO";

char lbl_1_data_A4C[] = "no646matoate-n2_nullP";

char lbl_1_data_A62[] = "no646matoate-n2_nullQ";

char *lbl_1_data_A78[4] = {lbl_1_data_A20, lbl_1_data_A36, lbl_1_data_A4C, lbl_1_data_A62};

char lbl_1_data_A88[] = "no646matoate-n2_colN";

char lbl_1_data_A9D[] = "no646matoate-n2_colO";

char lbl_1_data_AB2[] = "no646matoate-n2_colP";

char lbl_1_data_AC7[] = "no646matoate-n2_colQ";

char *lbl_1_data_ADC[4] = {lbl_1_data_A88, lbl_1_data_A9D, lbl_1_data_AB2, lbl_1_data_AC7};

char lbl_1_data_AEC[] = "no646matoate-n2_nullA";

char lbl_1_data_B02[] = "no646matoate-n2_nullB";

char lbl_1_data_B18[] = "no646matoate-n2_nullC";

char *lbl_1_data_B30[3] = {lbl_1_data_AEC, lbl_1_data_B02, lbl_1_data_B18};

char lbl_1_data_B3C[] = "no646matoate-n2_colA";

char lbl_1_data_B51[] = "no646matoate-n2_colB";

char lbl_1_data_B66[] = "no646matoate-n2_colC";

char *lbl_1_data_B7C[3] = {lbl_1_data_B3C, lbl_1_data_B51, lbl_1_data_B66};

char lbl_1_data_B88[] = "no646matoate-n3_nullR";

char lbl_1_data_B9E[] = "no646matoate-n3_nullD";

char lbl_1_data_BB4[] = "no646matoate-n3_nullE";

char lbl_1_data_BCA[] = "no646matoate-n3_nullF";

char lbl_1_data_BE0[] = "no646matoate-n3_nullG";

char *lbl_1_data_BF8[4] = {lbl_1_data_B9E, lbl_1_data_BB4, lbl_1_data_BCA, lbl_1_data_BE0};

char lbl_1_data_C08[] = "no646matoate-n3_colD";

char lbl_1_data_C1D[] = "no646matoate-n3_colE";

char lbl_1_data_C32[] = "no646matoate-n3_colF";

char lbl_1_data_C47[] = "no646matoate-n3_colG";

char *lbl_1_data_C5C[4] = {lbl_1_data_C08, lbl_1_data_C1D, lbl_1_data_C32, lbl_1_data_C47};

char lbl_1_data_C6C[] = "no646matoate-n3_nullH";

char lbl_1_data_C82[] = "no646matoate-n3_nullI";

char lbl_1_data_C98[] = "no646matoate-n3_nullJ";

char *lbl_1_data_CB0[3] = {lbl_1_data_C6C, lbl_1_data_C82, lbl_1_data_C98};

char lbl_1_data_CBC[] = "no646matoate-n3_colH";

char lbl_1_data_CD1[] = "no646matoate-n3_colI";

char lbl_1_data_CE6[] = "no646matoate-n3_colJ";

char *lbl_1_data_CFC[3] = {lbl_1_data_CBC, lbl_1_data_CD1, lbl_1_data_CE6};

char lbl_1_data_D08[] = "no646matoate-n3_nullK";

char lbl_1_data_D1E[] = "no646matoate-n3_nullL";

char lbl_1_data_D34[] = "no646matoate-n3_nullM";

char *lbl_1_data_D4C[3] = {lbl_1_data_D08, lbl_1_data_D1E, lbl_1_data_D34};

char lbl_1_data_D58[] = "no646matoate-n3_colK";

char lbl_1_data_D6D[] = "no646matoate-n3_colL";

char lbl_1_data_D82[] = "no646matoate-n3_colM";

char *lbl_1_data_D98[3] = {lbl_1_data_D58, lbl_1_data_D6D, lbl_1_data_D82};

char lbl_1_data_DA4[] = "no646matoate-n3_nullN";

char lbl_1_data_DBA[] = "no646matoate-n3_nullO";

char lbl_1_data_DD0[] = "no646matoate-n3_nullP";

char lbl_1_data_DE6[] = "no646matoate-n3_nullQ";

char *lbl_1_data_DFC[4] = {lbl_1_data_DA4, lbl_1_data_DBA, lbl_1_data_DD0, lbl_1_data_DE6};

char lbl_1_data_E0C[] = "no646matoate-n3_colN";

char lbl_1_data_E21[] = "no646matoate-n3_colO";

char lbl_1_data_E36[] = "no646matoate-n3_colP";

char lbl_1_data_E4B[] = "no646matoate-n3_colQ";

char *lbl_1_data_E60[4] = {lbl_1_data_E0C, lbl_1_data_E21, lbl_1_data_E36, lbl_1_data_E4B};

char lbl_1_data_E70[] = "no646matoate-n3_nullA";

char lbl_1_data_E86[] = "no646matoate-n3_nullB";

char lbl_1_data_E9C[] = "no646matoate-n3_nullC";

char *lbl_1_data_EB4[3] = {lbl_1_data_E70, lbl_1_data_E86, lbl_1_data_E9C};

char lbl_1_data_EC0[] = "no646matoate-n3_colA";

char lbl_1_data_ED5[] = "no646matoate-n3_colB";

char lbl_1_data_EEA[] = "no646matoate-n3_colC";

char *lbl_1_data_F00[3] = {lbl_1_data_EC0, lbl_1_data_ED5, lbl_1_data_EEA};

char lbl_1_data_F0C[] = "no646matoate-n4_nullR";

char lbl_1_data_F22[] = "no646matoate-n4_nullD";

char lbl_1_data_F38[] = "no646matoate-n4_nullE";

char lbl_1_data_F4E[] = "no646matoate-n4_nullF";

char lbl_1_data_F64[] = "no646matoate-n4_nullG";

char *lbl_1_data_F7C[4] = {lbl_1_data_F22, lbl_1_data_F38, lbl_1_data_F4E, lbl_1_data_F64};

char lbl_1_data_F8C[] = "no646matoate-n4_colD";

char lbl_1_data_FA1[] = "no646matoate-n4_colE";

char lbl_1_data_FB6[] = "no646matoate-n4_colF";

char lbl_1_data_FCB[] = "no646matoate-n4_colG";

char *lbl_1_data_FE0[4] = {lbl_1_data_F8C, lbl_1_data_FA1, lbl_1_data_FB6, lbl_1_data_FCB};

char lbl_1_data_FF0[] = "no646matoate-n4_nullH";

char lbl_1_data_1006[] = "no646matoate-n4_nullI";

char lbl_1_data_101C[] = "no646matoate-n4_nullJ";

char *lbl_1_data_1034[3] = {lbl_1_data_FF0, lbl_1_data_1006, lbl_1_data_101C};

char lbl_1_data_1040[] = "no646matoate-n4_colH";

char lbl_1_data_1055[] = "no646matoate-n4_colI";

char lbl_1_data_106A[] = "no646matoate-n4_colJ";

char *lbl_1_data_1080[3] = {lbl_1_data_1040, lbl_1_data_1055, lbl_1_data_106A};

char lbl_1_data_108C[] = "no646matoate-n4_nullK";

char lbl_1_data_10A2[] = "no646matoate-n4_nullL";

char lbl_1_data_10B8[] = "no646matoate-n4_nullM";

char *lbl_1_data_10D0[3] = {lbl_1_data_108C, lbl_1_data_10A2, lbl_1_data_10B8};

char lbl_1_data_10DC[] = "no646matoate-n4_colK";

char lbl_1_data_10F1[] = "no646matoate-n4_colL";

char lbl_1_data_1106[] = "no646matoate-n4_colM";

char *lbl_1_data_111C[3] = {lbl_1_data_10DC, lbl_1_data_10F1, lbl_1_data_1106};

char lbl_1_data_1128[] = "no646matoate-n4_nullN";

char lbl_1_data_113E[] = "no646matoate-n4_nullO";

char lbl_1_data_1154[] = "no646matoate-n4_nullP";

char lbl_1_data_116A[] = "no646matoate-n4_nullQ";

char *lbl_1_data_1180[4] = {lbl_1_data_1128, lbl_1_data_113E, lbl_1_data_1154, lbl_1_data_116A};

char lbl_1_data_1190[] = "no646matoate-n4_colN";

char lbl_1_data_11A5[] = "no646matoate-n4_colO";

char lbl_1_data_11BA[] = "no646matoate-n4_colP";

char lbl_1_data_11CF[] = "no646matoate-n4_colQ";

char *lbl_1_data_11E4[4] = {lbl_1_data_1190, lbl_1_data_11A5, lbl_1_data_11BA, lbl_1_data_11CF};

char lbl_1_data_11F4[] = "no646matoate-n4_nullA";

char lbl_1_data_120A[] = "no646matoate-n4_nullB";

char lbl_1_data_1220[] = "no646matoate-n4_nullC";

char *lbl_1_data_1238[3] = {lbl_1_data_11F4, lbl_1_data_120A, lbl_1_data_1220};

char lbl_1_data_1244[] = "no646matoate-n4_colA";

char lbl_1_data_1259[] = "no646matoate-n4_colB";

char lbl_1_data_126E[] = "no646matoate-n4_colC";

char *lbl_1_data_1284[3] = {lbl_1_data_1244, lbl_1_data_1259, lbl_1_data_126E};

char lbl_1_data_1290[] = "no646matoate-n5_nullR";

char lbl_1_data_12A6[] = "no646matoate-n5_nullD";

char lbl_1_data_12BC[] = "no646matoate-n5_nullE";

char lbl_1_data_12D2[] = "no646matoate-n5_nullF";

char lbl_1_data_12E8[] = "no646matoate-n5_nullG";

char *lbl_1_data_1300[4] = {lbl_1_data_12A6, lbl_1_data_12BC, lbl_1_data_12D2, lbl_1_data_12E8};

char lbl_1_data_1310[] = "no646matoate-n5_colD";

char lbl_1_data_1325[] = "no646matoate-n5_colE";

char lbl_1_data_133A[] = "no646matoate-n5_colF";

char lbl_1_data_134F[] = "no646matoate-n5_colG";

char *lbl_1_data_1364[4] = {lbl_1_data_1310, lbl_1_data_1325, lbl_1_data_133A, lbl_1_data_134F};

char lbl_1_data_1374[] = "no646matoate-n5_nullH";

char lbl_1_data_138A[] = "no646matoate-n5_nullI";

char lbl_1_data_13A0[] = "no646matoate-n5_nullJ";

char *lbl_1_data_13B8[3] = {lbl_1_data_1374, lbl_1_data_138A, lbl_1_data_13A0};

char lbl_1_data_13C4[] = "no646matoate-n5_colH";

char lbl_1_data_13D9[] = "no646matoate-n5_colI";

char lbl_1_data_13EE[] = "no646matoate-n5_colJ";

char *lbl_1_data_1404[3] = {lbl_1_data_13C4, lbl_1_data_13D9, lbl_1_data_13EE};

char lbl_1_data_1410[] = "no646matoate-n5_nullK";

char lbl_1_data_1426[] = "no646matoate-n5_nullL";

char lbl_1_data_143C[] = "no646matoate-n5_nullM";

char *lbl_1_data_1454[3] = {lbl_1_data_1410, lbl_1_data_1426, lbl_1_data_143C};

char lbl_1_data_1460[] = "no646matoate-n5_colK";

char lbl_1_data_1475[] = "no646matoate-n5_colL";

char lbl_1_data_148A[] = "no646matoate-n5_colM";

char *lbl_1_data_14A0[3] = {lbl_1_data_1460, lbl_1_data_1475, lbl_1_data_148A};

char lbl_1_data_14AC[] = "no646matoate-n5_nullN";

char lbl_1_data_14C2[] = "no646matoate-n5_nullO";

char lbl_1_data_14D8[] = "no646matoate-n5_nullP";

char lbl_1_data_14EE[] = "no646matoate-n5_nullQ";

char *lbl_1_data_1504[4] = {lbl_1_data_14AC, lbl_1_data_14C2, lbl_1_data_14D8, lbl_1_data_14EE};

char lbl_1_data_1514[] = "no646matoate-n5_colN";

char lbl_1_data_1529[] = "no646matoate-n5_colO";

char lbl_1_data_153E[] = "no646matoate-n5_colP";

char lbl_1_data_1553[] = "no646matoate-n5_colQ";

char *lbl_1_data_1568[4] = {lbl_1_data_1514, lbl_1_data_1529, lbl_1_data_153E, lbl_1_data_1553};

char lbl_1_data_1578[] = "no646matoate-n5_nullA";

char lbl_1_data_158E[] = "no646matoate-n5_nullB";

char lbl_1_data_15A4[] = "no646matoate-n5_nullC";

char *lbl_1_data_15BC[3] = {lbl_1_data_1578, lbl_1_data_158E, lbl_1_data_15A4};

char lbl_1_data_15C8[] = "no646matoate-n5_colA";

char lbl_1_data_15DD[] = "no646matoate-n5_colB";

char lbl_1_data_15F2[] = "no646matoate-n5_colC";

char *lbl_1_data_1608[3] = {lbl_1_data_15C8, lbl_1_data_15DD, lbl_1_data_15F2};

char lbl_1_data_1614[] = "no646matoate-kaisi_nullR";

char lbl_1_data_162D[] = "no646matoate-kaisi_nullA";

char lbl_1_data_1646[] = "no646matoate-kaisi_nullB";

char lbl_1_data_165F[] = "no646matoate-kaisi_nullC";

char lbl_1_data_1678[] = "no646matoate-kaisi_nullD";

char *lbl_1_data_1694[4] = {lbl_1_data_162D, lbl_1_data_1646, lbl_1_data_165F, lbl_1_data_1678};

char lbl_1_data_16A4[] = "no646matoate-kaisi_colA";

char lbl_1_data_16BC[] = "no646matoate-kaisi_colB";

char lbl_1_data_16D4[] = "no646matoate-kaisi_colC";

char lbl_1_data_16EC[] = "no646matoate-kaisi_colD";

char *lbl_1_data_1704[4] = {lbl_1_data_16A4, lbl_1_data_16BC, lbl_1_data_16D4, lbl_1_data_16EC};

char lbl_1_data_1714[] = "no646matoate-luc_nullR";

char lbl_1_data_172B[] = "no646matoate-luc_nullF";

char lbl_1_data_1742[] = "no646matoate-luc_nullG";

char lbl_1_data_1759[] = "no646matoate-luc_nullH";

char lbl_1_data_1770[] = "no646matoate-luc_nullI";

char lbl_1_data_1787[] = "no646matoate-luc_nullJ";

char lbl_1_data_179E[] = "no646matoate-luc_nullK";

char lbl_1_data_17B5[] = "no646matoate-luc_nullL";

char lbl_1_data_17CC[] = "no646matoate-luc_nullM";

char lbl_1_data_17E3[] = "no646matoate-luc_nullN";

char lbl_1_data_17FA[] = "no646matoate-luc_nullO";

char lbl_1_data_1811[] = "no646matoate-luc_nullP";

char lbl_1_data_1828[] = "no646matoate-luc_nullQ";

char *lbl_1_data_1840[12] = {
    lbl_1_data_172B, lbl_1_data_1742, lbl_1_data_1759, lbl_1_data_1770, lbl_1_data_1787,
    lbl_1_data_179E, lbl_1_data_17B5, lbl_1_data_17CC, lbl_1_data_17E3, lbl_1_data_17FA,
    lbl_1_data_1811, lbl_1_data_1828
};

char lbl_1_data_1870[] = "no646matoate-luc_colF";

char lbl_1_data_1886[] = "no646matoate-luc_colG";

char lbl_1_data_189C[] = "no646matoate-luc_colH";

char lbl_1_data_18B2[] = "no646matoate-luc_colI";

char lbl_1_data_18C8[] = "no646matoate-luc_colJ";

char lbl_1_data_18DE[] = "no646matoate-luc_colK";

char lbl_1_data_18F4[] = "no646matoate-luc_colL";

char lbl_1_data_190A[] = "no646matoate-luc_colM";

char lbl_1_data_1920[] = "no646matoate-luc_colN";

char lbl_1_data_1936[] = "no646matoate-luc_colO";

char lbl_1_data_194C[] = "no646matoate-luc_colP";

char lbl_1_data_1962[] = "no646matoate-luc_colQ";

char *lbl_1_data_1978[12] = {
    lbl_1_data_1870, lbl_1_data_1886, lbl_1_data_189C, lbl_1_data_18B2, lbl_1_data_18C8,
    lbl_1_data_18DE, lbl_1_data_18F4, lbl_1_data_190A, lbl_1_data_1920, lbl_1_data_1936,
    lbl_1_data_194C, lbl_1_data_1962
};

char lbl_1_data_19A8[] = "no646matoate-luc_nullA";

char lbl_1_data_19BF[] = "no646matoate-luc_nullB";

char lbl_1_data_19D6[] = "no646matoate-luc_nullC";

char lbl_1_data_19ED[] = "no646matoate-luc_nullD";

char lbl_1_data_1A04[] = "no646matoate-luc_nullE";

char *lbl_1_data_1A1C[5] = { lbl_1_data_19A8, lbl_1_data_19BF, lbl_1_data_19D6, lbl_1_data_19ED,
                             lbl_1_data_1A04 };

char lbl_1_data_1A30[] = "no646matoate-luc_colA";

char lbl_1_data_1A46[] = "no646matoate-luc_colB";

char lbl_1_data_1A5C[] = "no646matoate-luc_colC";

char lbl_1_data_1A72[] = "no646matoate-luc_colD";

char lbl_1_data_1A88[] = "no646matoate-luc_colE";

char *lbl_1_data_1AA0[5] = { lbl_1_data_1A30, lbl_1_data_1A46, lbl_1_data_1A5C, lbl_1_data_1A72,
                             lbl_1_data_1A88 };

M646MovingStageDescriptor lbl_1_data_1AB4[7] = {
    { DATANUM(DATA_m646, M646_LAYOUT_A_MODEL_FILE),
      DATANUM(DATA_m646, M646_LAYOUT_A_COLLISION_FILE),
      lbl_1_data_480,
      { { lbl_1_data_4F0, lbl_1_data_554, 4, 3 },
        { lbl_1_data_5A8, lbl_1_data_5F4, 3, 4 },
        { lbl_1_data_644, lbl_1_data_690, 3, 5 },
        { lbl_1_data_6F4, lbl_1_data_758, 4, 6 },
        { lbl_1_data_7AC, lbl_1_data_7F8, 3, 7 } } },
    { DATANUM(DATA_m646, M646_LAYOUT_B_MODEL_FILE),
      DATANUM(DATA_m646, M646_LAYOUT_B_COLLISION_FILE),
      lbl_1_data_804,
      { { lbl_1_data_874, lbl_1_data_8D8, 4, 3 },
        { lbl_1_data_92C, lbl_1_data_978, 3, 4 },
        { lbl_1_data_9C8, lbl_1_data_A14, 3, 5 },
        { lbl_1_data_A78, lbl_1_data_ADC, 4, 6 },
        { lbl_1_data_B30, lbl_1_data_B7C, 3, 7 } } },
    { DATANUM(DATA_m646, M646_LAYOUT_C_MODEL_FILE),
      DATANUM(DATA_m646, M646_LAYOUT_C_COLLISION_FILE),
      lbl_1_data_B88,
      { { lbl_1_data_BF8, lbl_1_data_C5C, 4, 3 },
        { lbl_1_data_CB0, lbl_1_data_CFC, 3, 4 },
        { lbl_1_data_D4C, lbl_1_data_D98, 3, 5 },
        { lbl_1_data_DFC, lbl_1_data_E60, 4, 6 },
        { lbl_1_data_EB4, lbl_1_data_F00, 3, 7 } } },
    { DATANUM(DATA_m646, M646_LAYOUT_D_MODEL_FILE),
      DATANUM(DATA_m646, M646_LAYOUT_D_COLLISION_FILE),
      lbl_1_data_F0C,
      { { lbl_1_data_F7C, lbl_1_data_FE0, 4, 3 },
        { lbl_1_data_1034, lbl_1_data_1080, 3, 4 },
        { lbl_1_data_10D0, lbl_1_data_111C, 3, 5 },
        { lbl_1_data_1180, lbl_1_data_11E4, 4, 6 },
        { lbl_1_data_1238, lbl_1_data_1284, 3, 7 } } },
    { DATANUM(DATA_m646, M646_LAYOUT_E_MODEL_FILE),
      DATANUM(DATA_m646, M646_LAYOUT_E_COLLISION_FILE),
      lbl_1_data_1290,
      { { lbl_1_data_1300, lbl_1_data_1364, 4, 3 },
        { lbl_1_data_13B8, lbl_1_data_1404, 3, 4 },
        { lbl_1_data_1454, lbl_1_data_14A0, 3, 5 },
        { lbl_1_data_1504, lbl_1_data_1568, 4, 6 },
        { lbl_1_data_15BC, lbl_1_data_1608, 3, 7 } } },
    { DATANUM(DATA_m646, M646_LAYOUT_OPENING_MODEL_FILE),
      DATANUM(DATA_m646, M646_LAYOUT_OPENING_COLLISION_FILE),
      lbl_1_data_1614,
      { { lbl_1_data_1694, lbl_1_data_1704, 4, 8 },
        { NULL, NULL, 0, -1 },
        { NULL, NULL, 0, -1 },
        { NULL, NULL, 0, -1 },
        { NULL, NULL, 0, -1 } } },
    { DATANUM(DATA_m646, M646_LAYOUT_SPECIAL_MODEL_FILE),
      DATANUM(DATA_m646, M646_LAYOUT_SPECIAL_COLLISION_FILE),
      lbl_1_data_1714,
      { { NULL, NULL, 0, -1 },
        { NULL, NULL, 0, -1 },
        { NULL, NULL, 0, -1 },
        { lbl_1_data_1840, lbl_1_data_1978, 12, 6 },
        { lbl_1_data_1A1C, lbl_1_data_1AA0, 5, 7 } } }
};

/* Carrier models in the same order as the scene's opening and A-E hook names. */
s32 lbl_1_data_1D38[6] = {
    DATANUM(DATA_m646, M646_CARRIER_OPENING_MODEL_FILE),
    DATANUM(DATA_m646, M646_CARRIER_A_MODEL_FILE),
    DATANUM(DATA_m646, M646_CARRIER_B_MODEL_FILE),
    DATANUM(DATA_m646, M646_CARRIER_C_MODEL_FILE),
    DATANUM(DATA_m646, M646_CARRIER_D_MODEL_FILE),
    DATANUM(DATA_m646, M646_CARRIER_E_MODEL_FILE)
};

/* Carrier motions start together when the scene reaches the start sequence. */
s32 lbl_1_data_1D50[6] = {
    DATANUM(DATA_m646, M646_CARRIER_OPENING_MOTION_FILE),
    DATANUM(DATA_m646, M646_CARRIER_A_MOTION_FILE),
    DATANUM(DATA_m646, M646_CARRIER_B_MOTION_FILE),
    DATANUM(DATA_m646, M646_CARRIER_C_MOTION_FILE),
    DATANUM(DATA_m646, M646_CARRIER_D_MOTION_FILE),
    DATANUM(DATA_m646, M646_CARRIER_E_MOTION_FILE)
};

OMOBJ *lbl_1_bss_80;

/* Scene setup creates the six-carrier stage object and allocates its layout and target work. */
void fn_1_8158(OMOBJMAN *objman)
{
    OMOBJ *obj;
    M646MovingStageWork *work;
    obj = omAddObjEx(objman, 30, 6, 6, OM_GRP_NONE, fn_1_8ACC);
    lbl_1_bss_80 = obj;
    obj->stat |= OM_STAT_MODELPAUSE;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M646MovingStageWork), HU_MEMNUM_OVL);
    obj->data = work;
    memset(work, 0, sizeof(M646MovingStageWork));
}

void fn_1_81F8(void)
{
}

/* The turn coordinator queries an opening target position before aiming its player. */
void fn_1_81FC(s16 index, Point3d *out)
{
    void *stageWork = lbl_1_bss_80->data;
    out->x = lbl_1_bss_84.points[index].x;
    out->y = lbl_1_bss_84.points[index].y;
    out->z = lbl_1_bss_84.points[index].z;
}

/* The startup callback loads six carriers with playback paused and attaches their chosen
 * layouts. */
void fn_1_8270(OMOBJ *obj)
{
    M646MovingStageWork *work = obj->data;
    HU3D_MODELID model = 0;
    s32 carrierIndex;
    HU3D_MOTIONID motion;

    fn_1_89EC(obj);
    for (carrierIndex = 0; carrierIndex < 6; carrierIndex++) {
        model = obj->mdlId[carrierIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_1D38[carrierIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        motion = obj->mtnId[carrierIndex] = Hu3DJointMotion(
            model, HuDataSelHeapReadNum(lbl_1_data_1D50[carrierIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DMotionSet(model, motion);
        /* Carrier motion waits for the scene to reach the start sequence. */
        Hu3DMotionSpeedSet(model, (0.0f));
        Hu3DModelLayerSet(model, 6);
        fn_1_8380(obj, carrierIndex);
    }
}

/* Carrier setup loads one visible layout, its hidden collision meshes, and hooked targets. */
void fn_1_8380(OMOBJ *obj, s32 index)
{
    M646MovingStageWork *work = obj->data;
    M646MovingStageState *stage = NULL;
    M646MovingStageDescriptor *descriptor = NULL;
    M646MovingStageGroup *group = NULL;
    u32 kind;
    HU3D_MODELID model;
    HU3D_MODELID parent;
    HU3D_MODELID hidden;
    M646ModelAttachment *attachment;
    HU3D_MODELID attachmentModel;
    HU3D_MOTIONID motion;
    M646ModelCollider *collider;
    M646AnchorRecord *anchor;
    s16 attachmentIndex;
    s32 groupIndex;
    HSF_OBJECT *mesh;
    HSF_OBJECT *worldMesh;

    stage = &work->stages[index];
    memset(stage, 0, sizeof(M646MovingStageState));
    kind = work->stageOrder[index];
    stage->stageKind = kind;
    descriptor = &lbl_1_data_1AB4[kind];
    model = stage->model =
        Hu3DModelCreate(HuDataSelHeapReadNum(descriptor->modelFile, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(model, 4);
    parent = obj->mdlId[index];
    Hu3DModelHookSet(parent, lbl_1_data_468[index], model);
    stage->parentModel = parent;
    hidden = stage->hiddenModel = Hu3DModelCreate(
        HuDataSelHeapReadNum(descriptor->hiddenModelFile, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelHookSet(model, descriptor->hiddenHook, hidden);
    Hu3DModelLayerSet(hidden, 4);
    Hu3DModelAttrSet(hidden, HU3D_ATTR_DISPOFF);
    for (groupIndex = 0; groupIndex < 5; groupIndex++) {
        group = &descriptor->groups[groupIndex];
        for (attachmentIndex = 0; attachmentIndex < group->attachmentCount; attachmentIndex++) {
            attachment = &stage->attachments[groupIndex][attachmentIndex];
            if (kind == 5 && groupIndex == 0) {
                attachmentModel = Hu3DModelCreate(HuDataSelHeapReadNum(
                    lbl_1_data_2C0[attachmentIndex], HU_MEMNUM_OVL, HEAP_MODEL));
                motion = attachment->motion = Hu3DJointMotion(
                    attachmentModel, HuDataSelHeapReadNum(lbl_1_data_2D0[attachmentIndex],
                                                          HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DMotionSet(attachmentModel, motion);
                Hu3DMotionSpeedSet(attachmentModel, (0.0f));
            } else {
                attachmentModel = Hu3DModelCreate(
                    HuDataSelHeapReadNum(lbl_1_data_2E0[groupIndex], HU_MEMNUM_OVL, HEAP_MODEL));
                motion = attachment->motion = Hu3DJointMotion(
                    attachmentModel,
                    HuDataSelHeapReadNum(lbl_1_data_2F4[groupIndex], HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DMotionSet(attachmentModel, motion);
                Hu3DMotionSpeedSet(attachmentModel, (0.0f));
            }
            attachment->model = attachmentModel;
            attachment->state = 0;
            Hu3DModelHookSet(model, group->attachmentNames[attachmentIndex], attachmentModel);
            attachment->parentModel = model;
            Hu3DModelLayerSet(attachmentModel, 5);
            mesh = Hu3DModelObjPtrGet(hidden, group->colliderNames[stage->jointIndex[groupIndex]]);
            if (mesh != NULL) {
                collider = &stage->attachments[groupIndex][attachmentIndex].collider;
                collider->owner = &stage->attachments[groupIndex][attachmentIndex];
                collider->kind = 1;
                collider->group = 10;
                collider->callbackMode = 11;
                collider->update = fn_1_8E74;
                collider->contact = fn_1_8DAC;
                collider->category = (s16)groupIndex;
                if (kind == 5) {
                    collider->callbackType = group->callbackType + attachmentIndex;
                } else {
                    collider->callbackType = group->callbackType;
                }
                if (kind == 5) {
                    Hu3DModelObjPosGet(parent, group->attachmentNames[attachmentIndex],
                                       &lbl_1_bss_84.points[attachmentIndex]);
                }
                fn_1_48DC(collider, mesh, parent, group->attachmentNames[attachmentIndex]);
            }
            stage->jointIndex[groupIndex]++;
        }
    }
    worldMesh = Hu3DModelObjPtrGet(hidden, lbl_1_data_308[kind]);
    if (worldMesh != NULL) {
        anchor = (M646AnchorRecord *)&stage->collision;
        anchor->kind = 1;
        anchor->group = 4;
        anchor->callbackType = 12;
        anchor->unknown0C = 0;
        anchor->callbackMode = 10;
        anchor->owner = obj;
        anchor->unknown18 = 0;
        anchor->unknown1C = 0;
        fn_1_436C((M646CollisionRecord *)anchor, worldMesh);
        fn_1_43A0((M646CollisionRecord *)anchor);
        fn_1_3EF8(anchor);
    }
}

/* The stage frame callback copies carrier positions into surface transforms and samples target
 * joints. */
void fn_1_8800(OMOBJ *obj)
{
    M646MovingStageWork *work = obj->data;
    M646MovingStageState *stage = NULL;
    M646MovingStageDescriptor *descriptor = NULL;
    M646MovingStageGroup *group = NULL;
    s32 model = 0;
    s32 groupIndex = 0;
    s32 attachment = 0;
    Vec position;
    for (model = 0; model < 6; model++) {
        stage = &work->stages[model];
        descriptor = &lbl_1_data_1AB4[stage->stageKind];
        Hu3DModelPosGet(obj->mdlId[model], &position);
        stage->collision.transforms[0]->pos.x = position.x;
        stage->collision.transforms[0]->pos.y = position.y;
        stage->collision.transforms[0]->pos.z = position.z;
        for (groupIndex = 0; groupIndex < 5; groupIndex++) {
            group = &descriptor->groups[groupIndex];
            for (attachment = 0; attachment < group->attachmentCount; attachment++) {
                fn_1_4DD0(&stage->attachments[groupIndex][attachment].collider);
            }
        }
    }
}

/* Pauses all six carrier motions when requested; the stage update does not call this helper. */
void fn_1_8914(void)
{
    HU3D_MODELID model;
    s32 index;
    index = 0;
    while (index < 6) {
        model = lbl_1_bss_80->mdlId[index];
        Hu3DMotionSpeedSet(model, (0.0f));
        index++;
    }
}

/* The scene callback starts all six carrier motions when the start sequence begins. */
void fn_1_8980(void)
{
    HU3D_MODELID model;
    s32 index;
    index = 0;
    while (index < 6) {
        model = lbl_1_bss_80->mdlId[index];
        Hu3DMotionSpeedSet(model, (1.0f));
        index++;
    }
}

/* Startup puts the opening layout first, shuffles normal layouts, and substitutes the special
 * layout. */
void fn_1_89EC(OMOBJ *obj)
{
    M646MovingStageWork *work = obj->data;
    s32 attachmentIndex = 0;
    u32 used = 0;
    u32 bit = 0;
    s32 choice = 0;
    s32 index = 0;

    work->stageOrder[index] = 5;
    index++;
    while (used != M646_STAGE_ORDER_MASK) {
        choice = frand() % 5;
        bit = 1 << choice;
        if ((used & bit) == 0) {
            work->stageOrder[index] = choice;
            used |= bit;
            index++;
        }
    }
    choice = frand() % 3;
    /* Replacement keeps the opening and first normal layout, but omits one shuffled layout. */
    work->stageOrder[choice + 2] = 6;
}

/* The first object callback loads the stage and installs the target-position update callback. */
void fn_1_8ACC(OMOBJ *obj)
{
    M646MovingStageWork *work = obj->data;
    fn_1_8270(obj);
    obj->objFunc = fn_1_8C88;
}

/* The stage object runs this each frame to refresh its surfaces and target positions. */
void fn_1_8C88(OMOBJ *obj)
{
    M646MovingStageWork *work = obj->data;
    s32 attachmentIndex = 0;
    fn_1_8800(obj);
}

/* The collision manager detaches a target on its first projectile contact and starts its hit
 * motion. */
s32 fn_1_8DAC(M646ModelCollider *collider, M646ActorAnchor *actor)
{
    M646MovingStageWork *stageWork = lbl_1_bss_80->data;
    M646ModelAttachment *attachment = collider->owner;
    if (attachment->state == 0) {
        attachment->position.x = collider->pose.pos.x;
        attachment->position.y = collider->pose.pos.y;
        attachment->position.z = collider->pose.pos.z;
        Hu3DModelHookObjReset(attachment->parentModel, collider->jointName);
        attachment->parentModel = -1;
        Hu3DModelPosSetV(attachment->model, &attachment->position);
        switch (collider->callbackType) {
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                Hu3DMotionSpeedSet(attachment->model, (1.0f));
                break;
        }
    }
    /* Every contact is counted, even after the first hit has detached the target. */
    attachment->state++;
    return 1;
}

void fn_1_8E74(M646ModelAttachment *attachment)
{
}
