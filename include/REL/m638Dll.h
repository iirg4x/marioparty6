/* Shared M638 views and callback declarations for the Gondola Glide module. */
#ifndef M638DLL_H
#define M638DLL_H

#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "datadir_enum.h"
#include "string.h"
#include "math.h"

#include "game/esprite.h"

/* State shared by the minigame scene and its timed callbacks. */
typedef struct M638Job M638Job;
typedef struct M638Jobs {
    M638Job *freeHead; /* First unused job record available for scheduling. */
    M638Job *activeHead; /* First callback job currently waiting or running. */
    M638Job *storage; /* Base of the fixed job-record array. */
} M638Jobs;
struct M638Job {
    M638Jobs *owner; /* Queue that owns this record. */
    M638Job *prev; /* Previous record in the active or free list. */
    M638Job *next; /* Next record in the active or free list. */
    s32 (*callback)(void *); /* Per-update function; nonzero signals completion. */
    void *data; /* Callback-specific state passed to callback. */
};
extern M638Job lbl_1_bss_18[128];
extern M638Jobs lbl_1_bss_A18;
typedef struct M638TimerWatch {
    s32 count; /* Number of updates elapsed while watching the timer. */
    MGTIMER *timer; /* Timer whose record display is checked after the wait. */
} M638TimerWatch;
extern M638TimerWatch lbl_1_bss_10;
typedef struct M638ScaleWork {
    f32 responseScale; /* Scale increase applied as the response motion advances. */
    s16 active; /* Whether the scale update is active. */
    s16 model; /* Model handle whose scale is being changed. */
} M638ScaleWork;
struct M638TeamActorView;
void fn_1_3518(struct M638TeamActorView *actor, s32 teamActive, f32 blend);
void fn_1_5760(M638ScaleWork *work, f32 value);
u32 fn_1_7344(s32 difficulty);
s32 fn_1_7660(s32 (*callback)(void *), void *data);
void fn_1_76C8(M638Job **job);
s32 fn_1_6790(void *data);

void fn_1_5528(M638ScaleWork *work, s32 camera);
void fn_1_56B4(M638ScaleWork *work);

typedef struct M638CameraWork {
    f32 elapsed; /* Frames elapsed during the camera split transition. */
    f32 splitX; /* Horizontal split position in screen coordinates. */
} M638CameraWork;
void fn_1_16AC(M638CameraWork *work);
/* Model handles cached while the scene and course props are created. */
typedef struct M638ModelCacheView {
    u8 unk00[1810]; /* Scene state preceding the shared model cache. */
    s16 models[74]; /* Model handles cached for the scene and course props. */
} M638ModelCacheView;

/* One course prop's position, model, animation state, and travel flag. */
typedef struct M638Prop {
    Point3d pos; /* Course position in world units. */
    s16 model; /* Course prop model handle. */
    s16 state; /* Prop motion state: 0/1 active motion, 2 ready. */
    s16 moving; /* Nonzero advances the prop along its course path. */
} M638Prop;

/* Course props grouped by size and lighting, followed by their scheduler state. */
typedef struct M638PropGroups {
    M638Prop small[6]; /* Six small props released during the course. */
    M638Prop middle[8]; /* Eight middle props released during the course. */
    M638Prop night[4]; /* Four night-only props released during the course. */
    s32 count; /* Frames accumulated for the current release group. */
    s32 state; /* Current course prop release phase. */
} M638PropGroups;
/* Course lighting choice and cached course model handles. */
typedef struct M638ScenePropsView {
    u8 unk00[1772]; /* Scene state preceding lighting and model handles. */
    s16 alternateLight; /* Nonzero selects the alternate course lighting. */
    u8 unk6EE[36]; /* Scene state between lighting and model handles. */
    s16 models[74]; /* Cached scene and course prop model handles. */
} M638ScenePropsView;

typedef struct M638TeamView M638TeamView;
typedef struct M638PlayerView {
    OMOBJ *object; /* Engine object that owns this player record. */
    M638TeamView *team; /* Team input state used by this player. */
    f32 activity; /* Current player activity or animation progress. */
    f32 impulse; /* Horizontal movement impulse applied to the player. */
    f32 phase; /* Current phase of the player's movement animation. */
    Point3d pos; /* Player position in world units. */
    Point3d rot; /* Player rotation in degrees. */
    s32 count; /* Frames elapsed toward the next computer-generated A press. */
    s32 nextCount; /* Update threshold for the next computer-generated A press. */
    s16 model; /* Player character model handle. */
    s16 motions[5]; /* Motion handles used by the player animation states. */
    s16 playerNo; /* Global player number. */
    s16 charNo; /* Character selection number. */
    s16 group; /* Player team number. */
    s16 member; /* Player position within its team. */
    s16 padNo; /* Controller assigned to this player. */
    u8 unk4A[4]; /* No code names this field. */
    s16 isCom; /* Nonzero when this player is computer-controlled. */
    s16 difficulty; /* Computer difficulty level. */
    s16 attachedModel; /* Model attached to the player's character. */
    s16 state; /* Current player movement or animation state. */
    s16 exitState; /* Progress through the player's exit sequence. */
    s16 elapsed; /* Frames elapsed in the current player animation. */
    s16 buttonA; /* A-button state sampled for this player. */
    s16 buttonB; /* B-button state sampled for this player. */
} M638PlayerView;

/* Bounce effect, split transition, and team finish presentation state. */
typedef struct M638BounceView {
    u8 unk00[32]; /* Effect state preceding its frame counter. */
    s32 count; /* Frames elapsed in the bounce effect. */
    u8 unk24[6]; /* Effect state preceding its two model handles. */
    s16 modelA; /* First model used by the bounce effect. */
    s16 modelB; /* Second model used by the bounce effect. */
} M638BounceView;
typedef struct M638SceneSplitView {
    u8 unk00[1672]; /* Scene state preceding the split transition. */
    M638CameraWork camera; /* Progress and horizontal position of the split. */
    u8 unk690[106]; /* No code names this field. */
    s16 side; /* Team side being brought to the full-screen view. */
} M638SceneSplitView;
typedef struct M638TeamFinishView {
    u8 unk00[8]; /* Team finish state preceding the output value. */
    f32 output; /* Current output passed to the finish presentation. */
    u8 unk0C[20]; /* No code names this field. */
    f32 source; /* Starting value blended into the finish presentation. */
    u8 unk24[488]; /* Team finish state preceding the finish model. */
    s16 model; /* Winning team's finish model handle. */
    u8 unk20E[242]; /* No code names this field. */
    f32 blend; /* Current visual blend for the team's finish presentation. */
    f32 base; /* Blend value used as the finish transition's starting point. */
    f32 elapsed; /* Frames elapsed in the finish transition. */
    s32 state; /* Finish callback phase. */
    u8 unk310[40]; /* No code names this field. */
    s16 variant; /* Team index used to select finish sounds and assets. */
} M638TeamFinishView;

/* Team actor pose, visual blend, and motion state. */
typedef struct M638TeamActorView {
    Point3d rot; /* Team actor rotation in degrees. */
    Point3d pos; /* Team actor position in world units. */
    f32 blend; /* Current visual blend for the team actor. */
    s32 swingEnabled; /* Nonzero while the actor responds to the team's held input. */
    s32 count; /* Frames accumulated in the current actor state. */
    s32 blinkCount; /* Frames until the next eye blink. */
    s16 state; /* Current team actor animation state. */
    s16 modelA; /* First model handle used by the team actor. */
    s16 modelB; /* Second model handle used by the team actor. */
    s16 modelC; /* Third model handle used by the team actor. */
    s16 modelD; /* Fourth model handle used by the team actor. */
} M638TeamActorView;

/* Course models, event hooks, prop groups, and current segment. */
typedef struct M638CourseView {
    M638Prop pair[2][2]; /* Paired props queued for the two team courses. */
    s32 pairIndex[2]; /* Next prop slot for each team course. */
    M638PropGroups props; /* Small, middle, and night course props. */
    u8 unk1C8[4]; /* No code names this field. */
    s16 current; /* Index of the current course segment. */
    s16 alternateModel; /* Model handle for the alternate course appearance. */
    s16 model; /* Current course model handle. */
    s16 lastModel; /* Previous course model handle during a transition. */
    s16 segments[20]; /* Course segment model handles. */
    s16 events[10]; /* Course event hook indices. */
    s16 overlays[9]; /* Overlay model handles used by course events. */
    s16 connectors[40]; /* Course connector model handles. */
    s16 nightModels[5]; /* Night course model handles. */
} M638CourseView;
typedef struct M638SceneCourseView {
    u8 unk00[1728]; /* Scene state preceding course event timing. */
    f32 eventTimes[10]; /* Course motion times for the event hooks. */
    f32 endTime; /* Course motion time at the end of the segment. */
    s16 alternateLight; /* Nonzero selects alternate course lighting. */
    u8 unk6EE[14]; /* Scene course state preceding selected choices. */
    s16 choices[11]; /* Selected course event choices. */
    s16 models[74]; /* Cached model handles for the course scene. */
} M638SceneCourseView;

/* Team and scene state used by the minigame's update and presentation callbacks. */
struct M638TeamView {
    M638PlayerView *players[2]; /* Two players assigned to this team. */
    M638TeamActorView actor; /* Team character pose and animation state. */
    M638CourseView course; /* Course segment and prop state for this team. */
    u8 unk2B8[4]; /* No code names this field. */
    M638ScaleWork scale; /* Model and progress for the team's scale effect. */
    M638Prop props[3]; /* Course props currently queued for this team. */
    f32 blend; /* Current visual blend for the team presentation. */
    f32 base; /* Starting blend value for team transitions. */
    f32 elapsed; /* Frames elapsed in the current team transition. */
    s32 state; /* Current team presentation state. */
    s16 active; /* Nonzero while the team accepts input. */
    s16 button; /* Button currently selected by the team. */
    s16 count; /* Index of the next course event for this team. */
    s16 swingHoldFrames; /* Frames an A input keeps the team actor in its held pose. */
    s16 nextPropIndex; /* Alternates between the two queued player props. */
    u16 camera; /* Camera assigned to this team's view. */
    f32 cameraBlend; /* Smoothed portion of blend used to move this team's camera. */
    f32 unusedFloat; /* Cleared to 0.0 during team setup. */
    s16 sprites[5]; /* Button and status sprites for the team view. */
    u8 unk32E[2]; /* No code names this field. */
    s32 buttonIndicatorFrames; /* Frames elapsed since the active button indicator last switched. */
    s32 buttonIndicatorIndex; /* Alternates between the two indicator sprites for the selected
                               * button. */
    s16 variant; /* Team index used for team-specific behavior. */
    u8 unk33A[2]; /* No code names this field. */
    s32 sound0; /* First sound identifier used by this team. */
    s32 sound1; /* Second sound identifier used by this team. */
};
typedef struct M638SceneView {
    M638TeamView teams[2]; /* Team state for both sides of the course. */
    M638CameraWork camera; /* Shared split-screen transition state. */
    MGTIMER *timer; /* Match timer shown during play. */
    f32 seqTime; /* Frames elapsed in the active minigame sequence. */
    f32 exitTime; /* Updates elapsed since the result sequence began. */
    s32 record; /* Stored record time in timer frames. */
    s32 time; /* Current match time in timer frames. */
    s32 newRecord; /* Nonzero when the match establishes a new record. */
    Point3d lightPosition; /* Set by fn_1_1CD4; light position in fn_1_13B4/fn_1_1470. */
    Point3d lightTarget; /* Set by fn_1_1CD4; light target in fn_1_13B4/fn_1_1470. */
    f32 eventTimes[10]; /* Course motion times for the event hooks. */
    f32 endTime; /* Course motion time at the end of play. */
    s16 alternateLight; /* Nonzero selects alternate course lighting. */
    s16 playState; /* Progress through active play. */
    s16 introState; /* Progress through the opening split-screen sequence. */
    s16 startState; /* Progress through the match start presentation. */
    s16 resultState; /* Progress through the results presentation. */
    s16 finishState; /* Progress through the team finish presentation. */
    s16 winnerSequenceState; /* Progress through the winner presentation callback. */
    s16 winner; /* Winning team index. */
    s16 choices[11]; /* Selected course event choices. */
    s16 models[73]; /* Scene model handles indexed by the data table. */
    s16 extraModels[1]; /* Additional scene model handle. */
    s16 sprites[3]; /* Shared scene status sprite handles. */
} M638SceneView;

/* Update a team's players, actor, course, and effects for the current frame. */
void fn_1_2288(M638TeamView *team);

struct _struct_lbl_1_data_330_0x8 {
    f32 courseTimeThreshold; /* Course motion time at which the actor advances to the next state. */
    f32 propRotationScale; /* Multiplier for the event prop rotation at this actor state. */
};
void ObjectSetup(void);
void fn_1_1EC(s16 mode, s16 frameNo);
void fn_1_284(s16 mode, s16 frameNo);
void fn_1_528(s16 mode, s16 frameNo);
void fn_1_598(s16 mode, s16 frameNo);
void fn_1_77C(s16 mode, s16 frameNo);
void fn_1_A28(s16 mode, s16 frameNo);
void fn_1_D84(s16 mode, s16 frameNo);
void fn_1_11EC(s16 mode, s16 frameNo);
void fn_1_11F0(s16 mode, s16 frameNo);
void fn_1_1294(void);
void fn_1_13B4(M638SceneView *scene);
void fn_1_1470(M638SceneView *scene);
void fn_1_165C(M638CameraWork *camera);
void fn_1_1988(OMOBJ *obj);
void fn_1_19C0(OMOBJ *obj);
void fn_1_1CD4(M638SceneView *scene);
void fn_1_1E58(M638SceneView *scene);
void fn_1_1F18(M638TeamView *team, s32 camera);
void fn_1_2CDC(M638TeamView *team);
s32 fn_1_2ED0(void *data);
s32 fn_1_2F94(void *data);
void fn_1_309C(M638TeamActorView *work, s32 camera);
s32 fn_1_368C(void *data);
void fn_1_3804(M638SceneView *sceneData);
void fn_1_386C(M638CourseView *course, s32 camera);
void fn_1_4948(M638CourseView *course);
void fn_1_4B4C(M638CourseView *course);
void fn_1_4D68(M638SceneView *scene);
void fn_1_4E48(M638SceneView *scene);
void fn_1_5378(M638SceneView *scene);
void fn_1_5814(M638PropGroups *data, s32 camera);
void fn_1_5D0C(M638PropGroups *group, f32 time);
s32 fn_1_6098(void *data);
s32 fn_1_61D0(void *data);
s32 fn_1_6374(void *data);
s32 fn_1_647C(void *data);
s32 fn_1_66C0(void *data);
void fn_1_67EC(OMOBJ *obj);
void fn_1_6B00(OMOBJ *obj);
void fn_1_6C98(M638PlayerView *player);
void fn_1_6E40(M638PlayerView *player);
s32 fn_1_6FE4(void *data);
s32 fn_1_71DC(M638PlayerView *player);
s32 fn_1_72D0(M638PlayerView *player);
void fn_1_745C(M638PlayerView *player);
void fn_1_7554(M638PlayerView *player);
void fn_1_75BC(void);
void fn_1_7754(void);

extern MGSEQ_PARAM lbl_1_data_0;
extern char lbl_1_data_28[9];
extern char lbl_1_data_31[9];
extern char lbl_1_data_3A[9];
extern char lbl_1_data_43[9];
extern char lbl_1_data_4C[9];
extern char lbl_1_data_55[9];
extern char lbl_1_data_5E[9];
extern char lbl_1_data_67[9];
extern char lbl_1_data_70[9];
extern char lbl_1_data_79[9];
extern char lbl_1_data_82[9];
extern char lbl_1_data_8B[9];
extern char lbl_1_data_94[9];
extern char lbl_1_data_9D[9];
extern char lbl_1_data_A6[9];
extern char lbl_1_data_AF[9];
extern char lbl_1_data_B8[9];
extern char lbl_1_data_C1[9];
extern char lbl_1_data_CA[9];
extern char lbl_1_data_D3[9];
extern char lbl_1_data_DC[9];
extern char lbl_1_data_E5[9];
extern char lbl_1_data_EE[9];
extern char lbl_1_data_F7[9];
extern char lbl_1_data_100[9];
extern char lbl_1_data_109[9];
extern char lbl_1_data_112[9];
extern char lbl_1_data_11B[9];
extern char lbl_1_data_124[9];
extern char lbl_1_data_12D[9];
extern char lbl_1_data_136[9];
extern char lbl_1_data_13F[9];
extern char lbl_1_data_148[9];
extern char lbl_1_data_151[9];
extern char lbl_1_data_15A[9];
extern char lbl_1_data_163[9];
extern char lbl_1_data_16C[9];
extern char lbl_1_data_175[9];
extern char lbl_1_data_17E[9];
extern char lbl_1_data_187[9];
extern char *lbl_1_data_190[40];
extern char *lbl_1_data_290[40];
extern struct _struct_lbl_1_data_330_0x8 lbl_1_data_330[10];
extern f32 lbl_1_data_380[10];
extern s32 lbl_1_data_3A8[73];
extern char lbl_1_data_4CC[];
extern char lbl_1_data_4CE[];
extern char lbl_1_data_4D0[7];
extern char lbl_1_data_4D7[7];
extern char lbl_1_data_4DE[6];
extern char lbl_1_data_4E4[7];
extern char lbl_1_data_4EB[4];
extern char lbl_1_data_4EF[4];
extern char lbl_1_data_4F3[4];
extern char lbl_1_data_4F7[4];
extern char lbl_1_data_4FB[7];
extern char lbl_1_data_502[7];
extern char lbl_1_data_509[7];
extern char lbl_1_data_510[7];
extern char lbl_1_data_517[9];
extern u32 lbl_1_data_548[5];
extern M638Jobs lbl_1_bss_A18;
extern M638Job lbl_1_bss_18[128];
extern M638TimerWatch lbl_1_bss_10;
extern HUPROCESS *lbl_1_bss_C;
extern s32 lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_4;
extern const u16 lbl_1_rodata_10[4];
extern const Point3d lbl_1_rodata_68;

#endif
