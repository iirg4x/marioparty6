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
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"

#include "game/esprite.h"

/* Retail-consumed layouts, descriptive names only. */
typedef struct M638Job M638Job;
typedef struct M638Jobs {
    M638Job *freeHead;
    M638Job *activeHead;
    M638Job *storage;
} M638Jobs;
struct M638Job {
    M638Jobs *owner;
    M638Job *prev;
    M638Job *next;
    s32 (*callback)(void *);
    void *data;
};
extern M638Job lbl_1_bss_18[128];
extern M638Jobs lbl_1_bss_A18;
typedef struct M638TimerWatch {
    s32 count;
    MGTIMER *timer;
} M638TimerWatch;
extern M638TimerWatch lbl_1_bss_10;
typedef struct M638ScaleWork {
    f32 value;
    s16 active;
    s16 model;
} M638ScaleWork;
struct M638TeamActorView;
void fn_1_3518(struct M638TeamActorView *work, s32 mode, f32 value);
void fn_1_5760(M638ScaleWork *work, f32 value);
u32 fn_1_7344(s32 difficulty);
s32 fn_1_7660(s32 (*callback)(void *), void *data);
void fn_1_76C8(M638Job **job);
s32 fn_1_6790(void *data);

/* This is a consumed view of the existing scene, not additional storage. */
typedef struct M638LightingView {
    u8 unknown000[1704];
    Point3d lightPosition;
    Point3d lightTarget;
    u8 unknown6C0[44];
    s16 alternateLight;
} M638LightingView;
void fn_1_13B4(M638LightingView *scene);
void fn_1_1470(M638LightingView *scene);
void fn_1_5528(M638ScaleWork *work, s32 camera);
void fn_1_56B4(M638ScaleWork *work);

typedef struct M638CameraWork {
    f32 elapsed;
    f32 splitX;
} M638CameraWork;
void fn_1_16AC(M638CameraWork *work);
/* Existing scene model-cache interval, indexed by creation consumers. */
typedef struct M638ModelCacheView {
    u8 unknown000[1810];
    s16 models[74];
} M638ModelCacheView;

/* Combined view of independently consumed fields on the same scene object. */
typedef struct M638SceneCameraView {
    u8 unknown000[524];
    s16 team0Model;
    u8 unknown20E[834];
    s16 team1Model;
    u8 unknown552[310];
    M638CameraWork camera;
    u8 unknown690[280];
    s16 sprite0;
    s16 sprite1;
} M638SceneCameraView;

/* Consumed 20-byte element, from fn_1_5814's indexed 6/8/4 arrays.
 * Consumers cover the first 18 bytes; the final two are unknown tail bytes.
 * The C view's natural alignment supplies that extent without new storage. */
typedef struct M638Prop {
    Point3d pos;
    s16 model;
    s16 state;
    s16 moving;
} M638Prop;

/* Existing 368-byte record: indexed 20-byte entries and two final counters. */
typedef struct M638PropGroups {
    M638Prop small[6];
    M638Prop middle[8];
    M638Prop night[4];
    s32 count;
    s32 state;
} M638PropGroups;
/* Only these consumers are named; unknown intervals are not new storage. */
typedef struct M638ScenePropsView {
    u8 unknown000[1772];
    s16 alternateLight;
    u8 unknown6EE[36];
    s16 models[74];
} M638ScenePropsView;

/* Views over the existing allocated team/player records. Unknown intervals
 * are explicit; these declarations do not allocate storage or name originals. */
typedef struct M638TeamInputView {
    u8 unknown000[784];
    s16 active;
    s16 button;
    u8 unknown314[36];
    s16 variant;
} M638TeamInputView;
typedef struct M638PlayerView {
    OMOBJ *object;
    M638TeamInputView *team;
    f32 activity;
    f32 impulse;
    f32 phase;
    Point3d pos;
    Point3d rot;
    s32 count;
    s32 nextCount;
    s16 model;
    s16 motions[5];
    s16 playerNo;
    s16 charNo;
    s16 group;
    s16 member;
    s16 padNo;
    u8 unknown4A[4];
    s16 isCom;
    s16 difficulty;
    s16 attachedModel;
    s16 state;
    s16 exitState;
    s16 elapsed;
    s16 buttonA;
    s16 buttonB;
} M638PlayerView;

/* Consumed views only. Unknown intervals are existing storage, not allocations. */
typedef struct M638BounceView {
    u8 unknown000[32];
    s32 count;
    u8 unknown024[6];
    s16 modelA;
    s16 modelB;
} M638BounceView;
typedef struct M638SceneSplitView {
    u8 unknown000[1672];
    M638CameraWork camera;
    u8 unknown690[106];
    s16 side;
} M638SceneSplitView;
typedef struct M638TeamFinishView {
    u8 unknown000[8];
    f32 output;
    u8 unknown00C[20];
    f32 source;
    u8 unknown024[488];
    s16 model;
    u8 unknown20E[242];
    f32 blend;
    f32 base;
    f32 elapsed;
    s32 state;
    u8 unknown310[40];
    s16 variant;
} M638TeamFinishView;

typedef struct M638SceneChoicesView {
    u8 unknown000[1788];
    s16 choices[11];
} M638SceneChoicesView;

/* Existing team actor subobject; both constructor and update consumers agree. */
typedef struct M638TeamActorView {
    Point3d rot;
    Point3d pos;
    f32 blend;
    s32 mode;
    s32 count;
    s32 blinkCount;
    s16 state;
    s16 modelA;
    s16 modelB;
    s16 modelC;
    s16 modelD;
} M638TeamActorView;

/* Existing course subobject. Indexed extents follow the creating loops. */
typedef struct M638CourseView {
    M638Prop pair[2][2];
    s32 pairIndex[2];
    M638PropGroups props;
    u8 unknown1C8[4];
    s16 current;
    s16 alternateModel;
    s16 model;
    s16 lastModel;
    s16 segments[20];
    s16 events[10];
    s16 overlays[9];
    s16 connectors[40];
    s16 nightModels[5];
} M638CourseView;
typedef struct M638SceneCourseView {
    u8 unknown000[1728];
    f32 eventTimes[10];
    f32 endTime;
    s16 alternateLight;
    u8 unknown6EE[14];
    s16 choices[11];
    s16 models[74];
} M638SceneCourseView;

/* Combined consumed-layout views. Names are descriptive, not original symbols.
 * Unknown intervals preserve existing storage only; these types allocate none. */
typedef struct M638TeamView {
    M638PlayerView *players[2];
    M638TeamActorView actor;
    M638CourseView course;
    u8 unknown2B8[4];
    M638ScaleWork scale;
    M638Prop props[3];
    f32 blend;
    f32 base;
    f32 elapsed;
    s32 state;
    s16 active;
    s16 button;
    s16 count;
    s16 field316;
    s16 field318;
    u16 camera;
    f32 field31C;
    f32 field320;
    s16 sprites[5];
    u8 unknown32E[2];
    s32 field330;
    s32 field334;
    s16 variant;
    u8 unknown33A[2];
    s32 sound0;
    s32 sound1;
} M638TeamView;
typedef struct M638SceneView {
    M638TeamView teams[2];
    M638CameraWork camera;
    MGTIMER *timer;
    f32 seqTime;
    f32 exitTime;
    s32 record;
    s32 time;
    s32 newRecord;
    Point3d vector0;
    Point3d vector1;
    f32 eventTimes[10];
    f32 endTime;
    s16 alternateLight;
    s16 playState;
    s16 introState;
    s16 startState;
    s16 resultState;
    s16 finishState;
    s16 field6F8;
    s16 winner;
    s16 choices[11];
    s16 models[73];
    s16 extraModels[1];
    s16 sprites[3];
} M638SceneView;
typedef char M638TeamView_stride_check[(sizeof(M638TeamView) == 836) ? 1 : -1];
typedef char M638SceneView_extent_check[(sizeof(M638SceneView) == 1964) ? 1 : -1];

/* +836 team stride and constructor/update consumers bind this parameter. */
void fn_1_2288(M638TeamView *team);

struct _struct_lbl_1_data_330_0x8 {
    f32 unk0;
    f32 unk4;
};
void fn_1_A0(void);
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
void fn_1_165C(M638CameraWork *arg0);
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
void fn_1_3804(void *arg0);
void fn_1_386C(M638CourseView *course, s32 camera);
void fn_1_4948(M638CourseView *course);
void fn_1_4B4C(M638CourseView *course);
void fn_1_4D68(M638SceneChoicesView *scene);
void fn_1_4E48(M638SceneView *scene);
void fn_1_5378(M638SceneCameraView *scene);
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
s32 fn_1_71DC(void *arg0);
s32 fn_1_72D0(void *arg0);
void fn_1_745C(M638PlayerView *player);
void fn_1_7554(void *arg0);
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
extern char lbl_1_data_230[5];
extern char lbl_1_data_235[9];
extern char lbl_1_data_23E[9];
extern char lbl_1_data_247[9];
extern char lbl_1_data_250[9];
extern char lbl_1_data_259[9];
extern char lbl_1_data_262[9];
extern char lbl_1_data_26B[9];
extern char lbl_1_data_274[9];
extern char lbl_1_data_27D[9];
extern char lbl_1_data_286[10];
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
extern /* Four unreferenced retail BSS bytes; original declaration is unknown. */
u8 lbl_1_bss_0[4];
extern const u16 lbl_1_rodata_10[4];
extern const Point3d lbl_1_rodata_68;

#endif
