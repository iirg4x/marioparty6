#ifndef M633DLL_H
#define M633DLL_H

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


/* Consumed layout of the 2792-byte work area. Unknown intervals preserve
 * observed storage; original field names and unobserved types are unknown. */
typedef struct M633Segment {
    s32 unk00;
    s16 unk04;
    unsigned char unknown06[2];
    Point3d unk08;
    Point3d unk14;
    Point3d unk20;
} M633Segment;

typedef struct M633AI {
    s32 unk00;
    s32 unk04;
    s32 unk08;
    float unk0C;
    float unk10;
    s32 unk14;
    Point3d unk18;
} M633AI;

typedef struct M633Work {
    HUPROCESS *unk000;
    unsigned char unknown004[12];
    s16 unk010[4];
    s16 unk018[4];
    s16 unk020;
    unsigned char unknown022[2];
    float unk024;
    float unk028;
    MGTIMER *unk02C;
    OMOBJ *unk030;
    s16 unk034[4];
    s16 unk03C;
    s16 unk03E;
    MGPLAYER *unk040[4];
    s32 unk050[4];
    s32 unk060[4];
    s32 unk070[4];
    s32 unk080[4];
    Point3d unk090[4];
    s32 unk0C0[4];
    float unk0D0[4];
    s32 unk0E0;
    OMOBJ *unk0E4[4];
    u16 unk0F4;
    unsigned char unknown0F6[2];
    s32 unk0F8[4];
    s32 unk108;
    s32 unk10C;
    s16 unk110;
    s16 unk112;
    s16 unk114;
    s16 unk116[2];
    s16 unk11A;
    s16 unk11C[50];
    s16 unk180[2];
    s16 unk184[4];
    s16 unk18C[4];
    float unk194;
    s32 unk198;
    M633Segment unk19C[50];
    float unkA34;
    s16 unkA38[4];
    M633AI unkA40[4];
    u16 unkAD0;
    u16 unkAD2;
    s32 unkAD4;
    s32 unkAD8;
    s32 unkADC;
    s32 unkAE0;
    s32 unkAE4;
} M633Work;
extern M633Work lbl_1_bss_0;

extern unsigned int lbl_1_data_1E0[16];
extern unsigned int lbl_1_data_220[16];

void fn_1_26B8(OMOBJ *obj);
void fn_1_28A4(OMOBJ *obj);
void fn_1_32C0(OMOBJ *obj);
void fn_1_34B0(OMOBJ *obj);
void fn_1_36BC(OMOBJ *obj);
void fn_1_24EC(s32 soundId, Point3d *pos);
void fn_1_45EC(MGPLAYER *player, M633AI *ai, M633Segment *segment);
void fn_1_4A00(MGPLAYER *player, M633AI *ai, M633Segment *first, M633Segment *second);
void fn_1_51D4(MGPLAYER *player, M633AI *ai);
void fn_1_55D0(MGPLAYER *player, M633AI *ai);
void fn_1_5B20(MGPLAYER *player, Point3d *first, Point3d *second, Point3d *third);
int fn_1_5ED0(Point3d *pos, Point3d *dir);
void fn_1_70A8(s32 arg0);
int fn_1_7F40(void);

s32 fn_1_A0(s32 arg0, s32 arg1);
void fn_1_104(s32 arg0);
void fn_1_140(void);
void fn_1_224(s16 mode, s16 frameNo);
void fn_1_310(s16 arg1, s16 frameNo);
void fn_1_55C(s16 mode, s16 frameNo);
void fn_1_690(s16 arg1, s16 frameNo);
void fn_1_AB0(s16 arg1, s16 frameNo);
void fn_1_C78(s16 mode, s16 frameNo);
void fn_1_118C(s16 mode, s16 frameNo);
void fn_1_1284(s16 mode, s16 frameNo);
void fn_1_1288(s16 mode, s16 frameNo);
void fn_1_128C(void);
void fn_1_1598(OMOBJ *obj);
void fn_1_258C(void);
void fn_1_26B4(OMOBJ *obj);
void fn_1_34A8(void);
void fn_1_34AC(OMOBJ *obj);
void fn_1_3828(OMOBJ *obj);
void fn_1_382C(void);
void fn_1_3830(s32 arg0);
void fn_1_38C0(s32 arg0);
void fn_1_3FFC(s32 playerNo);
s32 fn_1_4260(Point3d *start, Point3d *end, Point3d *pos, Point3d *nearest, float *distance);
s32 fn_1_43D4(M633Segment *segment, Point3d *pos, Point3d *nearest, float *distance);
void fn_1_4598(M633Segment *segment, Point3d *pos);
void fn_1_60F4(OMOBJ *obj);
void fn_1_6DE8(void);
void fn_1_73D8(void);
void fn_1_7E70(s32 arg0);
extern MGSEQ_PARAM lbl_1_data_0;
extern char lbl_1_data_28[25];
extern char lbl_1_data_41[25];
extern char lbl_1_data_5A[25];
extern char lbl_1_data_73[25];
extern char lbl_1_data_8C[25];
extern char lbl_1_data_A5[27];
extern char lbl_1_data_C0[];
extern char lbl_1_data_C1[];
extern char lbl_1_data_D2[];
extern char lbl_1_data_E3[];
extern char *lbl_1_data_F4[4];
extern char lbl_1_data_104[27];
extern char lbl_1_data_11F[23];
extern char lbl_1_data_136[25];
extern char lbl_1_data_14F[17];
extern char lbl_1_data_160[23];
extern char lbl_1_data_177[17];
extern char lbl_1_data_188[24];
extern char lbl_1_data_1A0[24];
extern s32 lbl_1_data_1B8[6];
extern s32 lbl_1_data_1D0[4];
extern u32 lbl_1_data_260[4];
extern u32 lbl_1_data_270[4];
extern u32 lbl_1_data_280[4];

#endif /* M633DLL_H */
