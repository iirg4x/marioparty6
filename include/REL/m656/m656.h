/* Shared state and helper declarations for Mass Meteor. */
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
#include "game/printfunc.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "datadir_enum.h"
#include "string.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"
#ifndef M656_INTEGRATION_H
#define M656_INTEGRATION_H

/* Per-meteor pattern entry used to place and animate a falling object. */
typedef struct M656Work60 M656Work60;
typedef struct M656Entry10 {
    s16 kind; /* Meteor model index. */
    float x; /* Starting horizontal position in stage units. */
    float y; /* Starting vertical position in stage units. */
    float deltaY; /* Vertical movement per frame while this group's player is moving. */
} M656Entry10;
typedef struct M656Work18 {
    s16 group; /* Player group, 0 or 1. */
    HU3D_MODELID model; /* Finish marker model. */
    float angle; /* Marker rotation in degrees. */
    s16 active; /* Nonzero after this group's progress reaches 100% and its finish marker is
                 * enabled. */
    HuVecF pos; /* Finish marker position in stage units. */
} M656Work18;
typedef struct M656Work4 {
    s16 group; /* Player group, 0 or 1. */
    s16 pattern; /* Selected meteor pattern row. */
} M656Work4;
typedef struct M656SceneWork {
    s32 frame; /* Frames elapsed in the current result transition. */
    s16 stream; /* Audio stream handle, or -1 before playback. */
    s32 fx; /* Stage sound effect handle, or -1 before playback. */
} M656SceneWork;
typedef struct M656CameraWork {
    s16 state; /* Stored field initialized before results-camera setup; no further use is
                  established here. */
} M656CameraWork;
typedef struct M656EnvironmentWork {
    s16 sprGroup; /* Sprite group for the stage progress display. */
    HU3D_MODELID model2; /* Backdrop model. */
    HU3D_MODELID model4; /* Animated stage model. */
    HU3D_MODELID models[20]; /* Twenty decorative stage models. */
    HU3D_MODELID model2E; /* Model whose hooks place the decorations. */
    HuVecF angles[20]; /* Decorative model rotations in radians. */
    HuVecF steps[20]; /* Per-frame rotation increments in radians. */
} M656EnvironmentWork;
/* Collision primitives use stage-space coordinates and radii. */
typedef struct M656Sphere {
    HuVecF center; /* Sphere center in stage units. */
    float radius;  /* Sphere radius in stage units. */
} M656Sphere;
typedef struct M656Work50 {
    s16 index; /* Position in the group's sixteen-entry pattern. */
    s16 group; /* Player group, 0 or 1. */
    M656Entry10 *entry; /* Pattern data for this meteor. */
    u8 unidentifiedBytes[4]; /* Bytes not read by the stage callbacks. */
    HuVecF pos; /* Current meteor position in stage units. */
    HuVecF velocity; /* Per-frame movement after a hit. */
    HU3D_MODELID model; /* Meteor model. */
    M656Sphere sphere; /* Current collision sphere. */
    s16 state; /* 0 while available, 1 after a player hit. */
    s16 timer; /* Frames since the hit movement began. */
    HuVecF axis; /* Rotation axis for the meteor. */
    float step; /* Rotation increment in radians per frame. */
    float angle; /* Current rotation angle in radians. */
} M656Work50;
struct M656Work60 {
    OMOBJ *childObj; /* Character object used to move and animate the player. */
    s16 index; /* Group index, 0 or 1. */
    s16 charNo; /* Character selected for this player. */
    s16 group; /* Player group, 0 or 1. */
    s16 playerNo; /* Player configuration index. */
    s16 motion; /* Current character motion index. */
    s32 timer; /* Frames in the current movement or transition state. */
    HuVecF velocity; /* Current player movement per frame. */
    HuVecF pos; /* Current player position in stage units. */
    u8 unidentifiedBytes[12]; /* Bytes not read by the stage callbacks. */
    HU3D_MODELID model38; /* Player's animated trail model. */
    s16 texScroll; /* Trail texture scroll handle. */
    s16 state; /* Gameplay state: 0 before movement, 1 moving, 2 hit, 3 finished. */
    s16 sprGroup; /* Progress marker sprite group. */
    float progress; /* Frames elapsed during the race. */
    float fraction; /* Progress along the 2350-frame course, clamped to 1. */
    HU3D_MODELID model48; /* Result animation model. */
    HU3D_MODELID model4A; /* Result animation model. */
    u8 unidentifiedBytes2[4]; /* Bytes not read by the stage callbacks. */
    HU3D_MODELID model50; /* Meteor impact effect model. */
    HuVecF target; /* Current computer-player wandering target. */
};
typedef struct M656Segment {
    HuVecF start; /* Segment start point. */
    HuVecF delta; /* Vector from start to end. */
} M656Segment;
typedef struct M656Capsule {
    M656Segment segment; /* Capsule center line. */
    float radius;        /* Capsule radius in stage units. */
} M656Capsule;
typedef struct M656Triangle {
    HuVecF start; /* Triangle base vertex. */
    HuVecF edge1; /* Vector from start to the second vertex. */
    HuVecF edge2; /* Vector from start to the third vertex. */
} M656Triangle;
typedef struct M656Bounds {
    HuVecF min; /* Minimum corner in stage units. */
    HuVecF max; /* Maximum corner in stage units. */
} M656Bounds;
typedef struct M656Vec4 {
    float x, y, z; /* Transformed point coordinates. */
    float w;       /* Homogeneous coordinate. */
} M656Vec4;

extern MGSEQ_PARAM lbl_1_data_0;
extern OMOBJMAN *lbl_1_bss_0;
extern s32 lbl_1_bss_10[2];
extern M656Work18 *lbl_1_bss_1C[2];
extern M656Work50 *lbl_1_bss_24[2][16];
extern M656Work4 *lbl_1_bss_A4[2];
extern M656Work60 *lbl_1_bss_AC[2];
extern M656Entry10 lbl_1_data_7C[4][16];
extern const s32 lbl_1_rodata_88[2];
extern s32 lbl_1_data_3C[8];
extern s16 lbl_1_data_65C[8];
extern char *lbl_1_data_60C[20];

typedef struct M656MatrixElements {
    float elements[16]; /* Row-major 4x4 matrix elements. */
} M656MatrixElements;
void fn_1_A0(void);
void fn_1_F4(void);
void fn_1_118(s16 mode, s16 frameNo);
void fn_1_14C(s16 mode, s16 frameNo);
void fn_1_150(s16 mode, s16 frameNo);
void fn_1_154(s16 mode, s16 frameNo);
void fn_1_158(s16 mode, s16 frameNo);
void fn_1_15C(s16 mode, s16 frameNo);
void fn_1_160(s16 mode, s16 frameNo);
void fn_1_164(s16 mode, s16 frameNo);
void fn_1_168(s16 mode, s16 frameNo);
void *fn_1_16C(s32 priority, u32 size, void (*hook)(OMOBJ *));
void fn_1_20C(OMOBJ *obj, void (*hook)(OMOBJ *));
float fn_1_214(f32 start, f32 end);
float fn_1_25C(float cosine);
void fn_1_2E0(Point3d *out, f32 x, f32 y, f32 z);
float fn_1_2F0(HuVecF *vec);
float fn_1_410(Point3d *vec);
float fn_1_440(HuVecF *a, HuVecF *b);
void fn_1_470(Point3d *out, Point3d *a, Point3d *b);
void fn_1_4A8(Point3d *out, Point3d *a, Point3d *b);
void fn_1_4E0(Point3d *out, Point3d *a, Point3d *b);
float fn_1_518(HuVecF *out, HuVecF *vec);
void fn_1_6F8(HuVecF *out, HuVecF *vec, float scale);
void fn_1_730(Point3d *out, Point3d *vec, Mtx44 matrix);
s32 fn_1_7C4(Mtx out, HuVecF *start, HuVecF *end, HuVecF *up);
s32 fn_1_EBC(HuVecF *out, Mtx matrix);
void fn_1_1548(Mtx out, f64 angle);
void fn_1_1620(Mtx out, const Mtx input, float scale);
void fn_1_1688(Point3d *out, Mtx matrix, Point3d *vec);
void fn_1_16C0(Mtx out, Mtx input);
void fn_1_16F0(M656Vec4 *out, Mtx44 matrix, HuVecF *vec);
s32 fn_1_17D4(Mtx44 out, HuVecF *start, HuVecF *end, HuVecF *up);
s32 fn_1_1D48(Mtx44 out, f32 angle, f32 aspect);
void fn_1_1E58(M656MatrixElements *out, M656MatrixElements *a, M656MatrixElements *b);
void fn_1_22FC(M656Sphere *out, f32 x, f32 y, f32 z, f32 radius);
void fn_1_2310(M656Bounds *out, M656Capsule *capsule);
void fn_1_24C8(M656Bounds *out, M656Sphere *sphere);
void fn_1_255C(M656Bounds *out, M656Triangle *triangle);
s32 fn_1_27C0(M656Bounds *boundsA, M656Bounds *boundsB);
s32 fn_1_2830(M656Sphere *sphereA, s32 vectorWordA, M656Sphere *sphereB, s32 vectorWordB,
    float maxTime, float *time, HuVecF *out);
float fn_1_2BE8(M656Sphere *sphereA, M656Sphere *sphereB);
float fn_1_2C8C(M656Sphere *sphereA, M656Sphere *sphereB);
f32 fn_1_2E18(Point3d *point, M656Triangle *triangle, f32 *triangleS, f32 *triangleT);
float fn_1_36BC(HuVecF *a, M656Triangle *b, float *s, float *t);
f32 fn_1_37E8(M656Segment *segmentA, M656Segment *segmentB, f32 *parameterA, f32 *parameterB);
float fn_1_43E4(M656Segment *a, M656Segment *b, float *s, float *t);
float fn_1_4510(HuVecF *point, M656Segment *segment);
float fn_1_4608(HuVecF *point, M656Segment *segment);
float fn_1_47EC(M656Sphere *sphere, M656Segment *segment);
double fn_1_4948(M656Sphere *sphere, M656Segment *segment);
float fn_1_4B8C(M656Sphere *sphere, M656Capsule *capsule);
double fn_1_4CF8(M656Sphere *sphere, M656Capsule *capsule);
M656Work60 *fn_1_4F4C(s32 group);
M656Work50 *fn_1_4F64(s16 group, s32 index);
M656Work18 *fn_1_4F88(s16 group);
void fn_1_4FA4(s16 *players);
void fn_1_5050(OMOBJ *obj);
void fn_1_5338(OMOBJ *obj);
void fn_1_53C4(OMOBJ *obj);
void fn_1_5434(OMOBJ *obj);
void fn_1_5624(OMOBJ *obj);
void fn_1_56A0(OMOBJ *obj);
void fn_1_5724(OMOBJ *obj);
void fn_1_5780(OMOBJ *obj);
void fn_1_5D20(OMOBJ *obj);
void fn_1_5D70(OMOBJ *obj);
void fn_1_5F38(OMOBJ *obj);
s16 fn_1_5F4C(s32 index);
void fn_1_5FF8(OMOBJ *obj);
void fn_1_65E0(OMOBJ *obj);
void fn_1_66D0(OMOBJ *obj);
s32 fn_1_67E8(M656Work60 *work);
void fn_1_690C(M656Work60 *work, s16 motion, float blend, u32 attr);
void fn_1_6990(OMOBJ *obj);
void fn_1_6E4C(OMOBJ *obj);
void fn_1_6EA8(OMOBJ *obj);
void fn_1_7024(OMOBJ *obj);
void fn_1_7130(OMOBJ *obj);
void fn_1_78C8(OMOBJ *obj);
void fn_1_8424(OMOBJ *obj);
void fn_1_85F8(OMOBJ *obj);
void fn_1_8680(OMOBJ *obj);
void fn_1_8920(OMOBJ *obj);
void fn_1_89DC(OMOBJ *obj);
void fn_1_89F0(OMOBJ *obj);
void fn_1_8A94(OMOBJ *obj);
void fn_1_8B38(OMOBJ *obj);
void fn_1_8B4C(OMOBJ *obj);
void fn_1_8CBC(OMOBJ *obj);
void fn_1_8D64(OMOBJ *obj);
s16 fn_1_8D78(s32 index);
void fn_1_8E24(OMOBJ *obj);
void fn_1_90B0(OMOBJ *obj);
void fn_1_9100(OMOBJ *obj);
void fn_1_92E4(OMOBJ *obj);
void fn_1_9398(OMOBJ *obj);
void fn_1_94C8(OMOBJ *obj);
void fn_1_9558(OMOBJ *obj);
extern char lbl_1_data_66C[14];
extern char lbl_1_data_67A[22], lbl_1_data_690[22], lbl_1_data_6A6[20];
extern float lbl_1_data_5C[8];
extern s16 lbl_1_bss_18;
extern s16 lbl_1_data_6BA[8];
extern s32 lbl_1_bss_C;
extern u32 lbl_1_bss_8;
extern u32 lbl_1_data_28[5];
#endif
