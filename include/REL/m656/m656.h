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

/* Address-derived symbols are retained where original names are unknown. */
/* Consumer-backed heap layouts. Offset/extent names are reconstruction labels,
 * not claims of recovered original type names. No compiler padding is added. */
typedef struct M656Work60 M656Work60;
typedef struct M656Entry10 {
    s16 kind;
    float x, y, deltaY;
} M656Entry10;
typedef struct M656Work18 {
    s16 group;
    HU3D_MODELID model;
    float angle;
    s16 active;
    HuVecF pos;
} M656Work18;
typedef struct M656Work4 {
    s16 group, pattern;
} M656Work4;
typedef struct M656SceneWork {
    s32 frame;
    s16 stream;
    s32 fx;
} M656SceneWork;
typedef struct M656CameraWork { s16 state; } M656CameraWork;
typedef struct M656EnvironmentWork {
    s16 sprGroup;
    HU3D_MODELID model2, model4;
    HU3D_MODELID models[20];
    HU3D_MODELID model2E;
    HuVecF angles[20];
    HuVecF steps[20];
} M656EnvironmentWork;
typedef struct M656Sphere { HuVecF center; float radius; } M656Sphere;
typedef struct M656Work50 {
    s16 index, group;
    M656Entry10 *entry;
    /* Unconsumed bytes inside the observed 80-byte allocation. */
    u8 unknown08[4];
    HuVecF pos, velocity;
    HU3D_MODELID model;
    M656Sphere sphere;
    s16 state, timer;
    HuVecF axis;
    float step, angle;
} M656Work50;
struct M656Work60 {
    OMOBJ *childObj;
    s16 index, charNo, group, playerNo, motion;
    s32 timer;
    HuVecF velocity, pos;
    /* Target allocation includes this interval; no original type is known. */
    u8 unknown2C[12];
    HU3D_MODELID model38;
    s16 texScroll, state, sprGroup;
    float progress, fraction;
    HU3D_MODELID model48, model4A;
    u8 unknown4C[4];
    HU3D_MODELID model50;
    HuVecF target;
};
/* Collision consumers establish these complete scalar/vector extents. */
typedef struct M656Segment { HuVecF start, delta; } M656Segment;
typedef struct M656Capsule { M656Segment segment; float radius; } M656Capsule;
typedef struct M656Triangle { HuVecF start, edge1, edge2; } M656Triangle;
typedef struct M656Bounds { HuVecF min, max; } M656Bounds;
typedef struct M656Vec4 { float x, y, z, w; } M656Vec4;

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

typedef struct M656MatrixElements { float elements[16]; } M656MatrixElements;
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
s32 fn_1_27C0(M656Bounds *a, M656Bounds *b);
s32 fn_1_2830(M656Sphere *a, s32 wordA, M656Sphere *b, s32 wordB,
    float maxTime, float *time, HuVecF *out);
float fn_1_2BE8(M656Sphere *a, M656Sphere *b);
float fn_1_2C8C(M656Sphere *a, M656Sphere *b);
f32 fn_1_2E18(Point3d *arg0, M656Triangle *arg1, f32 *arg2, f32 *arg3);
float fn_1_36BC(HuVecF *a, M656Triangle *b, float *s, float *t);
f32 fn_1_37E8(M656Segment *arg0, M656Segment *arg1, f32 *arg2, f32 *arg3);
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
void fn_1_9398(OMOBJ *arg0);
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
