extern "C" {
#include "math.h"
}

extern "C" {
#include "dolphin/gx.h"
}

extern "C" {
#include "dolphin/types.h"
}

extern "C" {
#include "dolphin/os.h"
}

extern "C" {
#include "dolphin/mtx.h"
}

extern "C" {
#include "game/window.h"
}

#include "REL/micquizmyokodll/module_types.h"

extern "C" {
#include "game/main.h"
}

extern "C" {
#include "game/object.h"
}

extern "C" {
#include "game/audio.h"
}

extern "C" {
#include "game/charman.h"
}

extern "C" {
#include "game/gamemes.h"
}

extern "C" {
#include "game/hsfex.h"
}

extern "C" {
#include "game/data.h"
}

extern "C" {
#include "game/gamework.h"
}

extern "C" {
#include "game/memory.h"
}

extern "C" {
#include "game/mg/seqman.h"
}

extern "C" {
#include "game/mg/timer.h"
}

extern "C" {
#include "game/mg/score.h"
}

extern "C" {
#include "game/pad.h"
}

extern "C" {
#include "game/frand.h"
}

extern "C" {
#include "game/sprite.h"
}

extern "C" {
#include "game/wipe.h"
}

extern "C" {
#include "game/mg/actman.h"
}

extern "C" {
#include "game/board/object.h"
}

extern "C" {
#include "game/board/tutorial.h"
}

extern "C" {
#include "datadir_enum.h"
}

extern "C" {
#include "messdir_enum.h"
}

extern "C" {
#include "game/mic.h"
}

extern "C" {
#include "humath.h"
}

extern "C" {
#include "msm_se.h"
#include "msm_stream.h"
}

#include "REL/micquizmyokodll.h"

extern "C" {
#include "dolphin.h"
}

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/process.h"
}

extern "C" {
#include "game/flag.h"
}

extern "C" {
#include "game/armem.h"
}

extern "C" {
#include "game/mgdata.h"
}

extern "C" {
#include "game/printfunc.h"
}

extern "C" {
#include "game/saveload.h"
}

extern "C" {
#include "game/esprite.h"
}

extern "C" {
#include "game/thpmain.h"
}

extern "C" {
#include "game/board/main.h"
}

extern "C" {
#include "game/board/audio.h"
}

extern "C" {
#include "game/board/masu.h"
}

extern "C" {
#include "game/board/player.h"
}

#include "REL/micquizmyokodll/shared_types.h"

#include "REL/micquizmyokodll/message_tables.h"

extern "C" {
#include "game/init.h"
}

#include "REL/micquizmyokodll/response_table.h"

void fn_1_8DE8(void);

s16 fn_1_89C0(s32 windowNo);

extern "C" void ObjectSetup(void);

void fn_1_110(OMOBJ *obj);

void fn_1_220(OMOBJ *object);

extern "C" {
#include <string.h>
}

extern s16 lbl_1_bss_31E;

void fn_1_148(OMOBJ *object);

void fn_1_1898(void);

void fn_1_1A4C(void);

void fn_1_1BAC(s32 messageSet);

void fn_1_1F04(void);

s32 fn_1_16B4(void);

s32 fn_1_17A8(void);

void fn_1_31CC(s16 playerIndex);

void fn_1_1F4(void);

extern s16 lbl_1_bss_324;

extern s16 lbl_1_bss_326;

extern s16 lbl_1_bss_328;

extern s16 lbl_1_bss_32A;

void fn_1_2B14(s16 playerIndex);

void fn_1_3258(void);

typedef void (*VoidFunc)(void);
typedef MicquizPlayerWork MicquizPlayerModels;
struct _struct_lbl_1_data_21C_0x8 {
              f32 positionX;
              f32 positionY;
};
typedef struct MicquizResponse {
    s16 result;
    u8 unknown_02[0xA]; /* No code names this field. */
    s32 choicesRemaining;
} MicquizResponse;
extern ANIMDATA *lbl_1_bss_33C;
extern ANIMDATA *lbl_1_bss_338;
extern s16 lbl_1_bss_334;
extern s16 lbl_1_bss_332;
extern u8 lbl_1_bss_32C[0x4];
extern s16 lbl_1_bss_322;
extern s16 lbl_1_bss_320;
extern s16 lbl_1_bss_31C;
extern HU3D_MODELID lbl_1_bss_31A;
extern HU3D_MOTIONID lbl_1_bss_30C[7];
extern HUPROCESS *lbl_1_bss_2E4[0xA];
extern s16 lbl_1_bss_20[0x14];
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern HUPROCESS *lbl_1_bss_2B8[11];
extern MicquizControlWork lbl_1_bss_10A;
extern s32 lbl_1_bss_1C;
extern MicquizAnimationState lbl_1_bss_54;
extern MicquizSequence lbl_1_bss_D4;
extern s16 omSysExitReq;
extern MicquizHostState lbl_1_bss_F4;
extern s16 lbl_1_bss_330;
extern HUSPR_GROUPID lbl_1_bss_404[];
extern s32 lbl_1_bss_4A0;
extern s32 lbl_1_bss_4AC;
extern s32 lbl_1_bss_4BC;
extern s32 lbl_1_bss_4C0;
extern s32 lbl_1_bss_4C4;
extern s32 lbl_1_bss_4C8;
extern s32 lbl_1_bss_4E4;
extern HUPROCESS *lbl_1_bss_4EC;
extern s32 lbl_1_data_1FC;
extern s16 lbl_1_data_3FC[6];
extern HUPROCESS *lbl_1_bss_2A4[];
extern HUPROCESS *lbl_1_bss_348;
extern s32 lbl_1_bss_3E0;
extern MicquizModeWork *lbl_1_bss_47C;
extern s16 lbl_1_bss_48[6];
extern MicquizAccessoryPoolEntry lbl_1_bss_98[];
extern s32 lbl_1_data_48[5];
extern HUPROCESS *lbl_1_bss_2A4[5];
extern MicquizAccessoryPoolEntry lbl_1_bss_98[6];
extern s32 lbl_1_bss_49C;
extern s16 lbl_1_bss_4B0;
extern s16 lbl_1_bss_570;
extern char lbl_1_data_350[0x18];
extern char lbl_1_data_2E0[0x15];
extern char lbl_1_data_2F5[0x16];
extern char lbl_1_data_30B[0x17];
extern char lbl_1_data_322[0x15];
extern char lbl_1_data_337[0x19];

extern MicquizPlayerWork lbl_1_bss_128[];
extern s16 lbl_1_data_388[];

extern s32 lbl_1_bss_4B8;
extern char lbl_1_data_5F8[];
extern u16 lbl_1_rodata_33C[5];
extern u32 lbl_1_data_4B8[9];
extern MicquizPlayerWork lbl_1_bss_128[5];
extern s16 lbl_1_bss_20[20];

extern s32 lbl_1_data_43C[6];
extern s16 lbl_1_bss_34C;
extern s16 lbl_1_bss_34E;
extern s16 lbl_1_bss_410;
extern s32 lbl_1_bss_480;
extern f32 lbl_1_bss_484;
extern f32 lbl_1_bss_488;
extern s16 lbl_1_bss_382;
extern s16 lbl_1_bss_CE;
extern s16 lbl_1_bss_D2[4];
extern s32 lbl_1_data_1F4;
extern ANIMDATA *lbl_1_bss_14;
extern f32 lbl_1_bss_18;
extern s32 lbl_1_bss_48C;
extern s32 lbl_1_bss_340;
extern s32 lbl_1_bss_344;
extern ANIMDATA *lbl_1_bss_3E8;
extern ANIMDATA *lbl_1_bss_3EC;
extern ANIMDATA *lbl_1_bss_3F0;
extern ANIMDATA *lbl_1_bss_3F4[4];
extern s32 lbl_1_data_1C[0xB];
extern ANIMDATA *lbl_1_bss_454;
extern ANIMDATA *lbl_1_bss_3E4;
extern s16 lbl_1_bss_40A;
extern s16 lbl_1_bss_40C;
extern s16 lbl_1_bss_40E;
extern s16 lbl_1_bss_412;
extern s16 lbl_1_bss_414;
extern s16 lbl_1_bss_416;
extern s16 lbl_1_bss_418;
extern s16 lbl_1_bss_422;
extern ANIMDATA *lbl_1_bss_424[2];
extern ANIMDATA *lbl_1_bss_42C;
extern ANIMDATA *lbl_1_bss_430;
extern ANIMDATA *lbl_1_bss_434;
extern ANIMDATA *lbl_1_bss_438;
extern ANIMDATA *lbl_1_bss_43C;
extern ANIMDATA *lbl_1_bss_440;
extern ANIMDATA *lbl_1_bss_444;
extern ANIMDATA *lbl_1_bss_448;
extern ANIMDATA *lbl_1_bss_478;
extern ANIMDATA *lbl_1_bss_470[2];
extern ANIMDATA *lbl_1_bss_46C;
extern ANIMDATA *lbl_1_bss_468;
extern ANIMDATA *lbl_1_bss_464;
extern ANIMDATA *lbl_1_bss_460;
extern ANIMDATA *lbl_1_bss_45C;
extern ANIMDATA *lbl_1_bss_458;
extern ANIMDATA *lbl_1_bss_450;
extern s16 lbl_1_bss_44C;
extern s16 lbl_1_data_20A;
extern s16 lbl_1_data_20C;
extern s16 lbl_1_data_20E;
extern s16 lbl_1_data_210[4];
extern s16 lbl_1_data_218;
extern s16 lbl_1_data_21A;
extern struct _struct_lbl_1_data_21C_0x8 lbl_1_data_21C[0xC];
extern s16 lbl_1_bss_378[4][3];
extern ANIMDATA *lbl_1_bss_390[4][3];

extern MGSCORE *lbl_1_bss_350[2];
extern s16 lbl_1_bss_358[4];
extern ANIMDATA *lbl_1_bss_360[4];
extern s16 lbl_1_bss_370;
extern ANIMDATA *lbl_1_bss_374;
extern s16 lbl_1_bss_3C0[4];
extern ANIMDATA *lbl_1_bss_3C8[4];
extern s16 lbl_1_bss_3D8;
extern ANIMDATA *lbl_1_bss_3DC;
extern u8 lbl_1_bss_406[0x4];
extern HUSPR_GROUPID lbl_1_bss_404[1];
extern s16 lbl_1_data_208;
extern u8 lbl_1_bss_41A[0x8];
extern s16 lbl_1_bss_490;
extern HUPROCESS *lbl_1_bss_C;
extern s16 lbl_1_bss_18A;
extern u32 lbl_1_data_A0[];
extern s32 lbl_1_bss_524;
extern s16 lbl_1_bss_8C;
extern s16 lbl_1_bss_8E;
extern s32 lbl_1_bss_4;
extern s32 lbl_1_bss_4CC;
extern s32 lbl_1_bss_4E8;
extern s32 lbl_1_bss_4DC;
extern s32 lbl_1_bss_4E0;
extern s32 lbl_1_bss_4D0;
extern u16 lbl_802C0008[4];
extern s32 lbl_1_bss_4B4;
extern s32 lbl_1_bss_4D4;
extern s32 lbl_1_bss_4D8;
extern u32 lbl_1_data_1CC[2];
extern u32 lbl_1_data_1D4[8];
extern u32 lbl_1_data_5C[0x11];
extern u32 lbl_1_data_E8[0x11];
extern s16 lbl_1_bss_472;
extern s32 lbl_1_bss_4A4;
extern MicquizResponse *lbl_1_bss_8;
extern HUPROCESS *lbl_1_bss_0;
extern HUPROCESS *lbl_1_bss_10;
extern s16 lbl_1_bss_4A8;
extern u32 lbl_1_data_12C[0x14];
extern u32 lbl_1_data_17C[0x14];
extern "C" int _prolog(void);
extern "C" void _epilog(void);
extern "C" void ObjectSetup(void);
extern "C++" void fn_1_110(OMOBJ *obj);
extern "C++" void fn_1_148(OMOBJ *object);
extern "C++" void fn_1_1F4(void);
extern "C++" void fn_1_220(OMOBJ *process);
extern "C++" void fn_1_938(void);
extern "C++" s32 fn_1_16B4(void);
inline void fn_1_8FCC_inlined_main(s32 windowNo, u32 messageId, s16 insertNo);
inline void fn_1_8D14_inlined_main(u32 messageId, s16 messageSpeed, u8 windowIndex);
extern "C++" s32 fn_1_17A8(void);
extern "C++" void fn_1_1898(void);
extern "C++" void fn_1_1A4C(void);
extern "C++" void fn_1_1BAC(s32 messageSet);
extern "C++" void fn_1_1EB4(void);
extern "C++" void fn_1_1F04(void);
extern "C++" void fn_1_2B14(s16 playerIndex);
extern "C++" void fn_1_31CC(s16 playerIndex);
extern "C++" void fn_1_3258(void);
extern "C++" s32 fn_1_45B0(s16 playerIndex);
extern "C++" s32 fn_1_4728(s16 playerIndex);
extern "C++" void fn_1_48C8(void);
extern "C++" void fn_1_5194(void);
extern "C++" void fn_1_5288(void);
extern "C++" void fn_1_7188(void);
extern "C++" void fn_1_71D0(void);
extern "C++" void fn_1_7754(void);
extern "C++" s32 fn_1_7FAC(void);
extern "C++" void fn_1_8200(void);
extern "C++" void fn_1_8280(s32 modelIndex);
extern "C++" void fn_1_829C(s16 layerNo);
extern "C++" void fn_1_834C(void);
extern "C++" void fn_1_8394(s32 count);
extern "C++" void fn_1_83A4(void);
extern "C++" void fn_1_83B8(void);
extern "C++" s32 fn_1_83FC(void);
extern "C++" void fn_1_840C(void);
extern "C++" void fn_1_8560(void);
extern "C++" s16 fn_1_89C0(s32 windowNo);
extern "C++" void fn_1_8A3C(void);
extern "C++" void fn_1_8BA8(void);
extern "C++" void fn_1_8D14(u32 messageId, s16 messageSpeed, u8 windowNo);
extern "C++" void fn_1_8DE8(void);
extern "C++" void fn_1_8E38(s32 windowNo);
extern "C" void fn_1_8F58(void);
extern "C++" short fn_1_8F98(short choice);
extern "C++" void fn_1_8FCC(s32 windowNo, u32 messageId, s16 insertNo);
extern "C++" void fn_1_9080(void);
extern "C++" void fn_1_90AC(void);
extern "C++" void fn_1_90D8(s32 );
extern "C++" void fn_1_9128(u32 messageType);
extern "C++" void fn_1_9214(s32 );
extern "C++" void fn_1_9280(s16 layerNo);
extern "C++" void fn_1_9784(HUPROCESS *);
extern "C++" void fn_1_9868(void);
extern "C++" void fn_1_9D5C(void);
extern "C++" void fn_1_9E44(void);
extern "C++" void fn_1_9F08(void);
extern "C++" void fn_1_BA0C(void);
extern "C++" void fn_1_C1E4(void);
extern "C++" void fn_1_C3FC(void);
extern "C++" void fn_1_C558(void);
extern "C++" void fn_1_C768(void);
extern "C++" void fn_1_C884(void);
extern "C++" void fn_1_CD18(s32 mode);
extern "C++" void fn_1_D058(void);
extern "C++" void fn_1_D18C(void);
extern "C++" void fn_1_D3E4(void);
extern "C++" void fn_1_D4F4(void);
extern "C++" void fn_1_DDF8(void);
extern "C++" void fn_1_DF4C(void);
extern "C++" void fn_1_E0A0(void);
extern "C++" void fn_1_E1E4(void);
extern "C++" void fn_1_E4E8(void);
extern "C++" void fn_1_E7EC(void);
extern "C++" void fn_1_E8A8(void);
extern "C++" void fn_1_E964(s32 action);
extern "C++" void fn_1_EC84(void);
extern "C++" void fn_1_ECE0(void);
extern "C++" void fn_1_ED38(void);
extern "C++" void fn_1_ED90(void);
extern "C++" void fn_1_F09C(void);
extern "C++" void fn_1_F334(void);
extern "C++" void fn_1_F5CC(void);
extern "C++" void fn_1_F608(void);
extern "C++" void fn_1_F7F4(s32 );
extern "C++" void fn_1_F890(void);
extern "C++" void fn_1_F920(void);
extern "C++" void fn_1_F9F4(s32 flag, s32 mode);
extern "C++" void fn_1_FB04(void);
extern "C++" void fn_1_FD14(void);
extern "C++" void fn_1_FF84(void);
extern "C++" void fn_1_100DC(void);
extern "C++" void fn_1_10234(void);
extern "C++" void fn_1_10238(void);
extern "C++" u8 fn_1_1023C(void);
extern "C++" u8 fn_1_10250(void);
extern "C++" void fn_1_10264(s8 player, u16 points, s32 result);
extern "C++" u8 fn_1_1029C(void);
extern "C++" s32 fn_1_102B0(s32 player);
extern "C++" f32 fn_1_102C8(void);
extern "C++" void fn_1_1043C(void);
extern "C++" void fn_1_10460(void);
extern "C++" s32 fn_1_10484(void);
extern "C++" s32 fn_1_10498(void);
extern "C++" void fn_1_104AC(void);
extern "C++" void fn_1_104C4(void);
extern "C++" void fn_1_104DC(void);
extern "C++" void fn_1_10788(u32 message);
extern "C++" void fn_1_109C8(u32 message);
extern "C++" void fn_1_10C08(void);
extern "C++" void fn_1_10E34(void);
extern "C++" void fn_1_11030(void);
extern "C++" void fn_1_1122C(s32 playerIndex);
extern "C++" void fn_1_114D0(void);
extern "C++" void fn_1_116CC(u32 message);
extern "C++" void fn_1_11940(void);
extern "C++" void fn_1_11B3C(void);
extern "C++" void fn_1_11D38(void);
extern "C++" void fn_1_11F34(void);
extern "C++" void fn_1_12144(void);
extern "C++" void fn_1_12340(s32 messageIndex);
extern "C++" void fn_1_1259C(char *path, s32 mode);
extern "C++" void fn_1_12608(s32 argument);
extern "C++" void fn_1_12774(void);
extern "C++" void fn_1_127A0(void);
extern "C++" void fn_1_127CC(s32 quizMode);
extern "C++" void fn_1_13EB8(s32 mode);
extern "C++" s32 fn_1_15090(s32 playerIndex, s32 responseMode);
extern "C++" s32 fn_1_154D8(s32 , s32 );
extern "C++" void fn_1_154F0(void);
extern "C++" void fn_1_15610(void);
extern "C++" void fn_1_15A0C(s32 );
extern "C" void fn_1_15C20(void);
extern "C" void fn_1_15C80(void);
extern "C" void fn_1_15CD0(void);
extern "C++" void fn_1_15D30(void);
extern "C++" void fn_1_15D80(s32 select, s32 padNo);
extern "C++" void fn_1_15EC0(s32 padNo, s32 direction);
extern "C++" void fn_1_15FB0(void);
extern "C++" void fn_1_16574(void);
extern "C++" void fn_1_16988(s32 );
extern "C++" void fn_1_16AEC(void);
extern "C++" void fn_1_16D64(void);
extern "C++" void fn_1_16E74(void);
extern "C++" void fn_1_16F6C(void);
extern "C++" void fn_1_17248(s32 mode, s32 wait);
extern "C++" void fn_1_177F0(u32 message);
extern "C++" void fn_1_17A30(s32 choice);
extern "C++" void fn_1_17CA8(u32 choice);
extern "C++" void fn_1_17F20(void);
extern "C++" void fn_1_17F24(void);
extern "C++" void fn_1_17F64(HUPROCESS* parent);
extern "C++" void fn_1_17FB4(void);
extern "C++" void fn_1_17FEC(void);
extern "C++" void fn_1_18138(void);
extern "C++" s32 fn_1_18300(void);
extern "C++" void fn_1_183E0(s32 value);
extern "C++" s32 fn_1_18470(s32 , s32 );
extern "C++" s32 fn_1_184AC(s32 );
extern "C++" void fn_1_1850C(s32 primaryMessageIndex, s32 secondaryMessageIndex);
extern "C++" void fn_1_18964(s32 primaryMessageIndex, s32 secondaryMessageIndex);
extern "C++" void fn_1_18DD0(s32 primaryMessageIndex, s32 secondaryMessageIndex);
extern "C++" void fn_1_19118(s32 messageIndex);
extern "C++" void fn_1_19288(void);
extern "C++" s32 fn_1_193A0(void);
extern "C++" void fn_1_193B4(s32 useCurrentPlayer, s32 playerFieldOffset);
extern "C++" void fn_1_1988C(void);
extern "C" int fn_1_19968(int key);
extern "C" int fn_1_199B8(int key);
extern "C" void fn_1_19A24(void);
extern "C" void fn_1_19B30(s32 motionNo, f32 startTime, u32 attr);
extern "C" void fn_1_19BD4(s32 );
extern "C" void fn_1_19BE8(int base);
extern "C++" void fn_1_19CD0(s32 baseIndex);
extern "C" void fn_1_19DE4(void);
extern "C" void fn_1_19EE0(void);
extern "C" void fn_1_19FB0(HUWINID window, u32 answerId, s16 soundIndex);

extern Point3d lbl_1_data_408;
extern Point3d lbl_1_data_414;
extern GXColor lbl_1_data_420;
extern s32 lbl_1_data_47C[5];
extern s32 lbl_1_data_490[5];
extern s32 lbl_1_data_4A4[5];
extern u32 lbl_1_data_394[5][2];
extern u32 lbl_1_data_3BC[5];
extern u32 lbl_1_data_3D0[5];
extern u32 lbl_1_data_3E4[6];
extern s32 lbl_1_data_4DC[11];
extern s32 lbl_1_data_508[11];
extern s32 lbl_1_data_534[11];
extern char lbl_1_data_5F8[12];
extern s16 lbl_1_data_388[6];
extern s32 lbl_1_data_424[6];
extern s32 lbl_1_data_454[5];
extern s32 lbl_1_data_468[5];
extern f32 lbl_1_data_560[11];

extern u32 lbl_1_data_A0[18];
extern s32 lbl_1_data_1F8;
s16 fn_1_89C0(s32 windowNo);
extern "C" void ObjectSetup(void);
void fn_1_110(OMOBJ *obj);
void fn_1_220(OMOBJ *object);
void fn_1_148(OMOBJ *object);
void fn_1_1898(void);
void fn_1_1A4C(void);
void fn_1_1BAC(s32 messageSet);
void fn_1_1F04(void);
s32 fn_1_16B4(void);
s32 fn_1_17A8(void);
void fn_1_31CC(s16 playerIndex);
void fn_1_1F4(void);
void fn_1_2B14(s16 playerIndex);
void fn_1_3258(void);
extern "C" int _prolog(void);
extern "C" void _epilog(void);
extern "C" void ObjectSetup(void);
extern "C++" void fn_1_110(OMOBJ *obj);
extern "C++" void fn_1_148(OMOBJ *object);
extern "C++" void fn_1_1F4(void);
extern "C++" void fn_1_220(OMOBJ *process);
extern "C++" void fn_1_938(void);
extern "C++" s32 fn_1_16B4(void);
inline void fn_1_8FCC_inlined_main(s32 windowNo, u32 messageId, s16 insertNo);
inline void fn_1_8D14_inlined_main(u32 messageId, s16 messageSpeed, u8 windowIndex);
extern "C++" s32 fn_1_17A8(void);
extern "C++" void fn_1_1898(void);
extern "C++" void fn_1_1A4C(void);
extern "C++" void fn_1_1BAC(s32 messageSet);
extern "C++" void fn_1_1EB4(void);
extern "C++" void fn_1_1F04(void);
extern "C++" void fn_1_2B14(s16 playerIndex);
extern "C++" void fn_1_31CC(s16 playerIndex);
extern "C++" void fn_1_3258(void);
extern "C++" s32 fn_1_45B0(s16 playerIndex);
extern "C++" s32 fn_1_4728(s16 playerIndex);
extern "C++" void fn_1_48C8(void);
extern "C++" void fn_1_5194(void);
extern "C++" void fn_1_5288(void);
extern "C++" void fn_1_7188(void);
extern "C++" void fn_1_71D0(void);
extern "C++" void fn_1_7754(void);
extern "C++" s32 fn_1_7FAC(void);
extern "C++" void fn_1_8200(void);
extern "C++" void fn_1_8280(s32 modelIndex);
extern "C++" void fn_1_829C(s16 layerNo);
extern "C++" void fn_1_834C(void);
extern "C++" void fn_1_8394(s32 count);
extern "C++" void fn_1_83A4(void);
extern "C++" void fn_1_83B8(void);
extern "C++" s32 fn_1_83FC(void);
extern "C++" void fn_1_840C(void);
extern "C++" void fn_1_8560(void);
extern "C++" s16 fn_1_89C0(s32 windowNo);
extern "C++" void fn_1_8A3C(void);
extern "C++" void fn_1_8BA8(void);
extern "C++" void fn_1_8D14(u32 messageId, s16 messageSpeed, u8 windowNo);
extern "C++" void fn_1_8DE8(void);
extern "C++" void fn_1_8E38(s32 windowNo);
extern "C" void fn_1_8F58(void);
extern "C++" short fn_1_8F98(short choice);
extern "C++" void fn_1_8FCC(s32 windowNo, u32 messageId, s16 insertNo);
extern "C++" void fn_1_9080(void);
extern "C++" void fn_1_90AC(void);
extern "C++" void fn_1_90D8(s32 );
extern "C++" void fn_1_9128(u32 messageType);
extern "C++" void fn_1_9214(s32 );
extern "C++" void fn_1_9280(s16 layerNo);
extern "C++" void fn_1_9784(HUPROCESS *);
extern "C++" void fn_1_9868(void);
extern "C++" void fn_1_9D5C(void);
extern "C++" void fn_1_9E44(void);
extern "C++" void fn_1_9F08(void);
extern "C++" void fn_1_BA0C(void);
extern "C++" void fn_1_C1E4(void);
extern "C++" void fn_1_C3FC(void);
extern "C++" void fn_1_C558(void);
extern "C++" void fn_1_C768(void);
extern "C++" void fn_1_C884(void);
extern "C++" void fn_1_CD18(s32 mode);
extern "C++" void fn_1_D058(void);
extern "C++" void fn_1_D18C(void);
extern "C++" void fn_1_D3E4(void);
extern "C++" void fn_1_D4F4(void);
extern "C++" void fn_1_DDF8(void);
extern "C++" void fn_1_DF4C(void);
extern "C++" void fn_1_E0A0(void);
extern "C++" void fn_1_E1E4(void);
extern "C++" void fn_1_E4E8(void);
extern "C++" void fn_1_E7EC(void);
extern "C++" void fn_1_E8A8(void);
extern "C++" void fn_1_E964(s32 action);
extern "C++" void fn_1_EC84(void);
extern "C++" void fn_1_ECE0(void);
extern "C++" void fn_1_ED38(void);
extern "C++" void fn_1_ED90(void);
extern "C++" void fn_1_F09C(void);
extern "C++" void fn_1_F334(void);
extern "C++" void fn_1_F5CC(void);
extern "C++" void fn_1_F608(void);
extern "C++" void fn_1_F7F4(s32 );
extern "C++" void fn_1_F890(void);
extern "C++" void fn_1_F920(void);
extern "C++" void fn_1_F9F4(s32 flag, s32 mode);
extern "C++" void fn_1_FB04(void);
extern "C++" void fn_1_FD14(void);
extern "C++" void fn_1_FF84(void);
extern "C++" void fn_1_100DC(void);
extern "C++" void fn_1_10234(void);
extern "C++" void fn_1_10238(void);
extern "C++" u8 fn_1_1023C(void);
extern "C++" u8 fn_1_10250(void);
extern "C++" void fn_1_10264(s8 player, u16 points, s32 result);
extern "C++" u8 fn_1_1029C(void);
extern "C++" s32 fn_1_102B0(s32 player);
extern "C++" f32 fn_1_102C8(void);
extern "C++" void fn_1_1043C(void);
extern "C++" void fn_1_10460(void);
extern "C++" s32 fn_1_10484(void);
extern "C++" s32 fn_1_10498(void);
extern "C++" void fn_1_104AC(void);
extern "C++" void fn_1_104C4(void);
extern "C++" void fn_1_104DC(void);
extern "C++" void fn_1_10788(u32 message);
extern "C++" void fn_1_109C8(u32 message);
extern "C++" void fn_1_10C08(void);
extern "C++" void fn_1_10E34(void);
extern "C++" void fn_1_11030(void);
extern "C++" void fn_1_1122C(s32 playerIndex);
extern "C++" void fn_1_114D0(void);
extern "C++" void fn_1_116CC(u32 message);
extern "C++" void fn_1_11940(void);
extern "C++" void fn_1_11B3C(void);
extern "C++" void fn_1_11D38(void);
extern "C++" void fn_1_11F34(void);
extern "C++" void fn_1_12144(void);
extern "C++" void fn_1_12340(s32 messageIndex);
extern "C++" void fn_1_1259C(char *path, s32 mode);
extern "C++" void fn_1_12608(s32 argument);
extern "C++" void fn_1_12774(void);
extern "C++" void fn_1_127A0(void);
extern "C++" void fn_1_127CC(s32 quizMode);
extern "C++" void fn_1_13EB8(s32 mode);
extern "C++" s32 fn_1_15090(s32 playerIndex, s32 responseMode);
extern "C++" s32 fn_1_154D8(s32 , s32 );
extern "C++" void fn_1_154F0(void);
extern "C++" void fn_1_15610(void);
extern "C++" void fn_1_15A0C(s32 );
extern "C" void fn_1_15C20(void);
extern "C" void fn_1_15C80(void);
extern "C" void fn_1_15CD0(void);
extern "C++" void fn_1_15D30(void);
extern "C++" void fn_1_15D80(s32 select, s32 padNo);
extern "C++" void fn_1_15EC0(s32 padNo, s32 direction);
extern "C++" void fn_1_15FB0(void);
extern "C++" void fn_1_16574(void);
extern "C++" void fn_1_16988(s32 );
extern "C++" void fn_1_16AEC(void);
extern "C++" void fn_1_16D64(void);
extern "C++" void fn_1_16E74(void);
extern "C++" void fn_1_16F6C(void);
extern "C++" void fn_1_17248(s32 mode, s32 wait);
extern "C++" void fn_1_177F0(u32 message);
extern "C++" void fn_1_17A30(s32 choice);
extern "C++" void fn_1_17CA8(u32 choice);
extern "C++" void fn_1_17F20(void);
extern "C++" void fn_1_17F24(void);
extern "C++" void fn_1_17F64(HUPROCESS* parent);
extern "C++" void fn_1_17FB4(void);
extern "C++" void fn_1_17FEC(void);
extern "C++" void fn_1_18138(void);
extern "C++" s32 fn_1_18300(void);
extern "C++" void fn_1_183E0(s32 value);
extern "C++" s32 fn_1_18470(s32 , s32 );
extern "C++" s32 fn_1_184AC(s32 );
extern "C++" void fn_1_1850C(s32 primaryMessageIndex, s32 secondaryMessageIndex);
extern "C++" void fn_1_18964(s32 primaryMessageIndex, s32 secondaryMessageIndex);
extern "C++" void fn_1_18DD0(s32 primaryMessageIndex, s32 secondaryMessageIndex);
extern "C++" void fn_1_19118(s32 messageIndex);
extern "C++" void fn_1_19288(void);
extern "C++" s32 fn_1_193A0(void);
extern "C++" void fn_1_193B4(s32 useCurrentPlayer, s32 playerFieldOffset);
extern "C++" void fn_1_1988C(void);
extern "C" int fn_1_19968(int key);
extern "C" int fn_1_199B8(int key);
extern "C" void fn_1_19A24(void);
extern "C" void fn_1_19B30(s32 motionNo, f32 startTime, u32 attr);
extern "C" void fn_1_19BD4(s32 );
extern "C" void fn_1_19BE8(int base);
extern "C++" void fn_1_19CD0(s32 baseIndex);
extern "C" void fn_1_19DE4(void);
extern "C" void fn_1_19EE0(void);
extern "C" void fn_1_19FB0(HUWINID window, u32 answerId, s16 soundIndex);
void fn_1_8E38(s32 windowNo);
void fn_1_938(void);
void fn_1_1EB4(void);
void fn_1_12608(s32 argument);
void fn_1_19CD0(s32 baseIndex);
void fn_1_8FCC(s32 windowNo, u32 messageId, s16 insertNo);
void fn_1_8D14(u32 messageId, s16 messageSpeed, u8 windowNo);
void fn_1_16988(s32);
s32 fn_1_102B0(s32);
void fn_1_19FB0(HUWINID winId, u32 answerId, s16 soundIndex);
static void fn_1_20B74_inlined(s32 windowNo);
static void fn_1_20D08_inlined(s32 windowNo, u32 messageId, s16 insertNo);
static void fn_1_20A50_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex);
static void fn_1_20C94_inlined(void);
static s16 fn_1_206FC_inlined(s32 windowNo);
static void fn_1_20B24_inlined(void);
void fn_1_1850C(s32 primaryMessageIndex, s32 secondaryMessageIndex);
static void fn_1_1278_inlined(s32 windowNo);
static void fn_1_140C_inlined(s32 windowNo, u32 messageId, s16 insertNo);
static void fn_1_1154_inlined(u32 messageId, s16 messageSpeed, u8 windowNo);
static void fn_1_1398_inlined(void);
static s16 fn_1_E00_inlined(s32 windowNo);
static void fn_1_1228_inlined(void);
void fn_1_18964(s32 primaryMessageIndex, s32 secondaryMessageIndex);
static void fn_1_1278_inlined_11558_inlined(s32 windowNo);
static void fn_1_140C_inlined_11558_inlined(s32 windowNo, u32 messageId, s16 insertNo);
static void fn_1_1154_inlined_11558_inlined(u32 messageId, s16 messageSpeed, u8 windowNo);
static void fn_1_1398_inlined_11558_inlined(void);
static s16 fn_1_E00_inlined_11558_inlined(s32 windowNo);
static void fn_1_1228_inlined_11558_inlined(void);
void fn_1_19118(s32 messageIndex);
static void fn_1_1278_inlined_8BC8_inlined(s32 windowNo);
static void fn_1_140C_inlined_8BC8_inlined(s32 windowNo, u32 messageId, s16 insertNo);
static void fn_1_1154_inlined_8BC8_inlined(u32 messageId, s16 messageSpeed, u8 windowNo);
static void fn_1_1398_inlined_8BC8_inlined(void);
static s16 fn_1_E00_inlined_8BC8_inlined(s32 windowNo);
static void fn_1_1228_inlined_8BC8_inlined(void);
void fn_1_109C8(u32 message);
static void fn_1_8BC8_inlined(u32 message);
static void fn_1_8E08_inlined(u32 message);
void fn_1_17248(s32 mode, s32 wait);
void fn_1_9868(void);
void fn_1_19BD4(s32 );
extern "C" s16 HuMCSelModeGet(void);
extern "C" s32 omPadErrChk(u8 playerIndex);
void fn_1_9D5C(void);
void fn_1_9E44(void);
void fn_1_C3FC(void);
void fn_1_C768(void);
s32 fn_1_10498(void);
void fn_1_15A0C(s32 mode);
static void fn_1_13F8_inlined(void);
static void fn_1_12A4_inlined(void);
static void fn_1_2029C_inlined_23748_inlined(void);
static void fn_1_20148_inlined_23748_inlined(void);
void fn_1_154F0(void);
void fn_1_F608(void);
void fn_1_F890(void);
extern void fn_1_FD14(void);
static void fn_1_20B74_inlined_966C_inlined(s32 windowNo);
static void fn_1_20D08_inlined_966C_inlined(s32 windowNo, u32 messageId, s16 insertNo);
static void fn_1_20A50_inlined_966C_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex);
static void fn_1_20C94_inlined_966C_inlined(void);
static s16 fn_1_206FC_inlined_966C_inlined(s32 windowNo);
static void fn_1_20B24_inlined_966C_inlined(void);
static void fn_1_8E38_inlined_inlined_inlined_inlined(s32 windowNo);
static void fn_1_8FCC_inlined_inlined_inlined_inlined(s32 windowNo, u32 messageId, s16 insertMessageNo);
static void fn_1_8D14_inlined_inlined_inlined_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex);
static void fn_1_8F58_inlined_inlined_inlined_inlined(void);
static void fn_1_8DE8_inlined_inlined_inlined_inlined(void);
static s16 fn_1_89C0_inlined_inlined_inlined_inlined(s32 windowNo);
static void fn_1_1CD0_inlined(s32 mode);
static void fn_1_1BAC_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex);
static void fn_1_1DF0_inlined(void);
static void insertWindowMessage_inlined(s32 windowNo, u32 messageId, s16 insertNo);
static s16 fn_1_1858_inlined(s32 windowNo);
static void fn_1_1C80_inlined(void);
void fn_1_1988C(void);
static void fn_1_935C_inlined(void);
static s32 fn_1_9134_for_B664_inlined(void);
static void SetQuizResponseMessageForB664_inlined(u32 messageType);
static void fn_1_1A40_inlined(void);
static s32 CurrentQuizAnswerForB664_inlined(void);
static void fn_1_18D4_inlined(void);
static void fn_1_90FC_inlined(s8 field13, u16 field1C, s32 field18);
static void SetQuizResponseMessage_inlined(u32 messageType);
void fn_1_17F64(HUPROCESS *process);
s32 fn_1_154D8(s32 responseIndex, s32 count);
void fn_1_15C80(void);
void fn_1_15C20(void);
void fn_1_F5CC(void);
void fn_1_15EC0(s32 padNo, s32 direction);
static void fn_1_20B74_inlined_inlined(s32 windowNo);
static void fn_1_20A50_inlined_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex);
static void fn_1_20C94_inlined_inlined(void);
static s16 fn_1_206FC_inlined_inlined(s32 windowNo);
static void fn_1_20B24_inlined_inlined(void);
static void fn_1_2E2B0_AnimateOut_inlined_inlined(void);
void fn_1_16AEC(void);
void fn_1_16D64(void);
void fn_1_16E74(void);
void fn_1_16F6C(void);
void fn_1_17FEC(void);
void fn_1_17FB4(void);
void fn_1_183E0(s32 value);
void fn_1_19288(void);
static const GXColor lbl_1_rodata_10 = {0,0,0,0xFF}; static const GXColor lbl_1_rodata_14 = {0,0xFF,0,0xFF};
static inline u8 GetQuizCourse(void) ;

static inline s32 GetSelectionMode(void) ;

static inline s32 GetQuizPlayer(void) ;

static inline void TimeDigitsSet(void) ;

static inline void TimeLimitInit(void) ;

static inline void fn_1_12608_inlined_main(s32 argument) ;

static inline void fn_1_19CD0_inlined_main(s32 baseIndex) ;

static inline s16 fn_1_89C0_inlined_main(s32 windowNo)
;

static inline void fn_1_8DE8_inlined_main(void)
;

static inline void fn_1_8E38_inlined_main(s32 windowNo)
;

static inline void fn_1_20B74_inlined(s32 windowNo)
;

static inline void fn_1_20D08_inlined(s32 windowNo, u32 messageId, s16 insertNo)
;

static inline void fn_1_20A50_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex)
;

static inline void fn_1_20C94_inlined(void)
;

static inline s16 fn_1_206FC_inlined(s32 windowNo)
;

static inline void fn_1_20B24_inlined(void)
;

static inline void fn_1_1278_inlined(s32 windowNo)
;

static inline void fn_1_140C_inlined(s32 windowNo, u32 messageId, s16 insertNo)
;

static inline void fn_1_1154_inlined(u32 messageId, s16 messageSpeed, u8 windowNo)
;

static inline void fn_1_1398_inlined(void)
;

static inline s16 fn_1_E00_inlined(s32 windowNo)
;

static inline void fn_1_1228_inlined(void)
;

static inline void fn_1_1278_inlined_11558_inlined(s32 windowNo)
;

static inline void fn_1_140C_inlined_11558_inlined(s32 windowNo, u32 messageId, s16 insertNo)
;

static inline void fn_1_1154_inlined_11558_inlined(u32 messageId, s16 messageSpeed, u8 windowNo)
;

static inline void fn_1_1398_inlined_11558_inlined(void)
;

static inline s16 fn_1_E00_inlined_11558_inlined(s32 windowNo)
;

static inline void fn_1_1228_inlined_11558_inlined(void)
;

static inline void fn_1_1278_inlined_8BC8_inlined(s32 windowNo)
;

static inline void fn_1_140C_inlined_8BC8_inlined(s32 windowNo, u32 messageId, s16 insertNo)
;

static inline void fn_1_1154_inlined_8BC8_inlined(u32 messageId, s16 messageSpeed, u8 windowNo)
;

static inline void fn_1_1398_inlined_8BC8_inlined(void)
;

static inline s16 fn_1_E00_inlined_8BC8_inlined(s32 windowNo)
;

static inline void fn_1_1228_inlined_8BC8_inlined(void)
;

static inline void fn_1_8BC8_inlined(u32 message)
;

static inline void fn_1_8E08_inlined(u32 message)
;

static inline void fn_1_13F8_inlined(void) ;

static inline void fn_1_12A4_inlined(void) ;

static inline void fn_1_2029C_inlined_23748_inlined(void)
;

static inline void fn_1_20148_inlined_23748_inlined(void)
;

static inline void CreatePlayerScoreDigits(void) ;

static inline void fn_1_20B74_inlined_966C_inlined(s32 windowNo)
;

static inline void fn_1_20D08_inlined_966C_inlined(s32 windowNo, u32 messageId, s16 insertNo)
;

static inline void fn_1_20A50_inlined_966C_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex)
;

static inline void fn_1_20C94_inlined_966C_inlined(void)
;

static inline s16 fn_1_206FC_inlined_966C_inlined(s32 windowNo)
;

static inline void fn_1_20B24_inlined_966C_inlined(void)
;

static inline void fn_1_8E38_inlined_inlined_inlined_inlined(s32 windowNo)
;

static inline void fn_1_8FCC_inlined_inlined_inlined_inlined(s32 windowNo, u32 messageId,
                                                     s16 insertMessageNo)
;

static inline void fn_1_8D14_inlined_inlined_inlined_inlined(u32 messageId, s16 messageSpeed,
                                                     u8 windowIndex)
;

static inline void fn_1_8F58_inlined_inlined_inlined_inlined(void)
;

static inline void fn_1_8DE8_inlined_inlined_inlined_inlined(void)
;

static inline s16 fn_1_89C0_inlined_inlined_inlined_inlined(s32 windowNo)
;

static inline void fn_1_1CD0_inlined(s32 mode)
;

static inline void fn_1_1BAC_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex)
;

static inline void fn_1_1DF0_inlined(void)
;

static inline void insertWindowMessage_inlined(s32 windowNo, u32 messageId, s16 insertNo)
;

static inline s16 fn_1_1858_inlined(s32 windowNo)
;

static inline void fn_1_1C80_inlined(void)
;

static inline void fn_1_9128_for_13EB8(u32 messageType)
;

static inline void fn_1_935C_inlined(void)
;

static inline s32 fn_1_9134_for_B664_inlined(void)
;

static inline void SetQuizResponseMessageForB664_inlined(u32 messageType)
;

static inline void fn_1_1A40_inlined(void)
;

static inline s32 CurrentQuizAnswerForB664_inlined(void)
;

static inline void fn_1_18D4_inlined(void)
;

static inline void fn_1_90FC_inlined(s8 field13, u16 field1C, s32 field18)
;

static inline void SetQuizResponseMessage_inlined(u32 messageType)
;

static inline void fn_1_8A3C_for_15610(void)
;

static inline void fn_1_9128_for_15610(u32 messageType)
;

static inline void fn_1_20B74_inlined_inlined(s32 windowNo)
;

static inline void fn_1_20A50_inlined_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex)
;

static inline void fn_1_20C94_inlined_inlined(void)
;

static inline s16 fn_1_206FC_inlined_inlined(s32 windowNo)
;

static inline void fn_1_20B24_inlined_inlined(void)
;

static inline void fn_1_2E2B0_AnimateOut_inlined_inlined(void) ;

static inline s16 fn_1_89C0_193B4(s32 windowNo)
;

static inline void fn_1_8DE8_193B4(void)
;

static inline void fn_1_8E38_193B4(s32 windowNo)
;

static inline void fn_1_8FCC_193B4(s32 windowNo, u32 messageId, s16 insertNo)
;

static inline void fn_1_8D14_193B4(u32 messageId, s16 messageSpeed, u8 windowNo)
;

static inline void fn_1_8F58_193B4(void)
;
#include "REL/micquizmyokodll/physical_support.h"

extern "C" int _prolog(void) {
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

extern "C" void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

void fn_1_8E38(s32 windowNo);

extern "C" {
extern "C" void ObjectSetup(void) {
    OMOBJ *object;

    lbl_1_bss_2B8[0] = fn_8003A8BC(0xC8, 0x2000);
    fn_80069F1C(lbl_1_bss_2B8[0]);
    object = fn_8003AB38(lbl_1_bss_2B8[0], 0x2000, 0U, 0U, -1, fn_1_110);
}
}

extern "C" {
extern "C++" void fn_1_110(OMOBJ *obj)
{
    fn_1_938();
    obj->objFunc = fn_1_220;
}
}

extern "C++" void fn_1_148(OMOBJ *object) {
    s32 animationIndex;

    if (WipeCheck() == 0) {
        animationIndex = 0;
        while (animationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
            HuSprAnimKill(lbl_1_bss_54.spriteAnimations[animationIndex]);
            animationIndex += 1;
        }
        HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
        fn_1_1EB4();
        HuMCListenerKill();
        HuMCClose();
        fn_8003A5E4(1, 1);
    }
}

void fn_1_938(void);

extern "C++" void fn_1_1F4(void) {
    fn_1_127CC(lbl_1_bss_D4.responseMode);
}

void fn_1_1EB4(void);

extern "C++" void fn_1_220(OMOBJ *process) {
    s32 animationIndex;
    s32 soundValue;
    s32 soundBank;
    s32 soundSlot;

    if (omSysExitReq != 0) {
        for (animationIndex = 0;
             animationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode];
             animationIndex++) {
            HuSprAnimKill(lbl_1_bss_54.spriteAnimations[animationIndex]);
        }
        HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
        WipeCreate(2, 0, 0x14);
        process->objFunc = fn_1_148;
    }

    if (lbl_1_bss_4C8 == 0) {
        lbl_1_bss_4BC++;
        if (lbl_1_bss_4BC >= lbl_1_bss_4C0) {
            HuAudFXPlay(MSM_SE_MICQUIZ_COUNTDOWN_TICK);
            lbl_1_bss_4C4--;
            lbl_1_bss_4BC = 0;
            soundValue = lbl_1_bss_4C4;
            if (soundValue >= 0) {
                soundBank = soundValue / 10;
                soundSlot = soundValue - (soundBank * 10);
                HuSprBankSet(lbl_1_bss_404[0], 0, soundBank);
                HuSprBankSet(lbl_1_bss_404[0], 1, soundSlot);
            }
            if (lbl_1_bss_4C4 <= 0) {
                lbl_1_bss_4C8 = 2;
            }
        }
    }

    switch (lbl_1_bss_10A.phase) {
    case 0:
        WipeCreate(1, 0, 0x14);
        lbl_1_bss_10A.phase++;
        break;
    case 1:
        if (lbl_1_bss_10A.timer++ >= 0x1E) {
            lbl_1_bss_1C = HuAudBGMPlay(MSM_STREAM_QUESTION_MUSIC_THIRD);
            fn_1_1A4C();
            fn_1_1BAC(0);
            lbl_1_bss_10A.phase++;
            lbl_1_bss_10A.timer = 0;
        }
        break;
    case 2:
        if (lbl_1_bss_10A.timer++ == 0x2D) {
            lbl_1_bss_330 = GameMesCreate(2, 0);
        } else if (lbl_1_bss_10A.timer > 0x2D &&
                   GameMesStatGet(lbl_1_bss_330) == 0) {
            fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[1],
                               0.0f, 0.0f,
                               ((1 << 30) | (1 << 0)));
            lbl_1_bss_10A.phase++;
            lbl_1_bss_10A.timer = 0;
        }
        break;
    case 3:
        if ((s16)fn_1_16B4() != 0) {
            fn_1_31CC(0);
            lbl_1_bss_10A.phase++;
        }
        break;
    case 4:
        if ((s16)fn_1_45B0(lbl_1_bss_10A.currentPlayer) != 0) {
            lbl_1_bss_10A.currentPlayer++;
        }
        if (lbl_1_bss_10A.currentPlayer >=
            lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
            lbl_1_bss_10A.phase++;
            lbl_1_bss_10A.flag_80 = 0;
            lbl_1_bss_10A.flag_40 = 1;
        } else {
            fn_1_31CC(lbl_1_bss_10A.currentPlayer);
        }
        break;
    case 5: {
        Point3d modelPosition;
        fn_800267CC(lbl_1_bss_F4.modelId, &modelPosition);
        if (modelPosition.x > 600.0f) {
            fn_8005BE80(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[0]);
            lbl_1_bss_10A.phase++;
        }
        break;
    }
    case 6:
        if ((s16)fn_1_17A8() != 0) {
            lbl_1_bss_10A.flag_40 = 0;
            fn_1_1BAC(lbl_1_bss_D4.phase + 1);
            if (lbl_1_bss_31E != -1) {
                HuWinMesWait(lbl_1_bss_31E);
            }
            lbl_1_bss_4A0 = 1;
            lbl_1_bss_10A.phase++;
        }
        break;
    case 7: {
        s32 firstResult = lbl_1_bss_4AC;
        if (firstResult == 3) {
            lbl_1_bss_10A.phase = 9;
            fn_1_48C8();
        } else {
            s32 secondResult = lbl_1_bss_4AC;
            if (secondResult == 4) {
                lbl_1_bss_10A.completionFlag = 1;
                lbl_1_bss_10A.phase = 9;
                fn_1_48C8();
            } else {
                s32 thirdResult = lbl_1_bss_4AC;
                if (thirdResult == 2) {
                    lbl_1_bss_10A.phase = 12;
                }
            }
        }
        break;
    }
    case 9:
        if ((s16)fn_1_16B4() != 0) {
            fn_1_5288();
        }
        break;
    case 12:
        Hu3DAmbColorSet(0.01f, 0.01f,
                        0.01f);
        lbl_1_bss_10A.phase = 13;
        break;
    case 13:
        if ((s16)fn_1_16B4() != 0) {
            lbl_1_bss_10A.phase = 14;
        }
        break;
    case 14:
        if ((s16)fn_1_7FAC() != 0) {
            lbl_1_bss_10A.phase = 15;
        }
        break;
    case 15:
        if ((s16)fn_1_17A8() != 0) {
            lbl_1_bss_10A.phase = 7;
            Hu3DAmbColorSet(1.0f, 1.0f,
                            1.0f);
            fn_1_8200();
            if (lbl_1_data_1FC != 2) {
                lbl_1_data_1FC = 2;
                fn_8000DAC0(fn_1_16E74, 0x64, 0x1800, 0, lbl_1_bss_4EC);
            }
            lbl_1_bss_4E4 = 1;
            lbl_1_bss_4AC = 1;
        }
        break;
    case 8:
    case 10:
    case 11:
        break;
    }

    fn_1_1898();
}

static inline u8 GetQuizCourse(void) {
    return lbl_1_bss_47C->questionLevel;
}

static inline s32 GetQuizPlayer(void) {
    return lbl_1_bss_47C->currentPlayer;
}

static inline s32 GetSelectionMode(void) {
    return lbl_1_bss_47C->questionEvent;
}

static inline void TimeDigitsSet(void) {
    s32 time;
    s32 tens;
    s32 ones;

    time = lbl_1_bss_4C4;
    if (time >= 0) {
        tens = time / 10;
        ones = time - tens * 10;
        HuSprBankSet(lbl_1_bss_404[0], 0, tens);
        HuSprBankSet(lbl_1_bss_404[0], 1, ones);
    }
}

static inline void TimeLimitInit(void) {
    lbl_1_bss_4C8 = 1;
    lbl_1_bss_4C4 = lbl_1_data_48[GetQuizCourse()];
    if (GetSelectionMode() == 2) {
        lbl_1_bss_4C4 = lbl_1_data_48[GetQuizCourse() + 1];
    } else if (GetSelectionMode() == 5) {
        lbl_1_bss_4C4 = lbl_1_data_48[GetQuizCourse() - 1];
    } else {
        lbl_1_bss_4C4 = lbl_1_data_48[GetQuizCourse()];
    }
    if (GetSelectionMode() == 4) lbl_1_bss_4C0 = 0x384 / lbl_1_bss_4C4;
    else lbl_1_bss_4C0 = 0x708 / lbl_1_bss_4C4;
    lbl_1_bss_4BC = 0;
    TimeDigitsSet();
}

static inline void fn_1_12608_inlined_main(s32 argument) {
    lbl_1_bss_49C = argument;
    HuMCInit(0);
    HuMCSelWinCreate(-10000.0f, 88.0f);
    if (argument == 0) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_2E0);
    } else if (argument == 1) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_2F5);
    } else if (argument == 2) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_30B);
    } else if (argument == 3) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_30B);
    } else if (argument == 4) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_322);
    } else if (argument == 5) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_337);
    } else if (argument == 6) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_350);
    } else {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_30B);
    }
}

static inline void fn_1_19CD0_inlined_main(s32 baseIndex) {
    f32 scale;
    f32 angle;
    s16 spriteIndex;
    spriteIndex = baseIndex + 4;
    angle = 0.0f;
    HuSprScaleSet(lbl_1_bss_570, spriteIndex, 0.0f, 0.0f);
    HuSprAttrReset(lbl_1_bss_570, spriteIndex, 4);
    do {
        HuPrcVSleep();
        scale = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprScaleSet(lbl_1_bss_570, spriteIndex, scale, scale);
        angle += 9.0f;
    } while (!(angle >= 90.0f));
}

extern "C++" void fn_1_938(void) {
    Point3d shadowPosition = {0.0f, 3000.0f, 0.0f};
    Point3d shadowDirection = {0.0f, 1.0f, 0.0f};
    Point3d shadowTarget = {0.0f, 0.0f, -1000.0f};
    f32 far, near, fov;
    s16 motionId;
    s32 cameraMask;
    HUPROCESS *parentProcess;
    s16 lightId;
    s16 modelId;
    s32 cameraIndex;
    s32 index;

    for (index = 0; index < 5; index++) lbl_1_bss_2A4[index] = 0;
    memset(&lbl_1_bss_10A, 0, 0x1E);
    Hu3DCameraCreate(2);
    Hu3DCameraPerspectiveSet(2, 30.0f, 10.0f, 5000.0f, 1.2f);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(2, 0, 0, 0x280, 0x1E0);
    Hu3DCameraPosSet(2, 0.0f, 543.0f, 1615.03f, 0.0f, 1.0f, 0.0f, 0.0f, 202.32f, -31.21f);
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 20.0f, 10.0f, 5000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 192.0f, 164.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0xC0, 0xA4);
    Hu3DCameraPosSet(1, 0.0f, 543.0f, 1615.03f, 0.0f, 1.0f, 0.0f, 0.0f, 202.32f, -31.21f);
    fn_1_9F08();
    parentProcess = lbl_1_bss_2B8[0];
    lbl_1_bss_348 = fn_8000DAC0(fn_1_9868, 0xFDE8, 0x2000, 0, fn_8000DD9C());
    HuPrcSetStat(lbl_1_bss_348, 0xC);
    lbl_1_bss_47C->pauseAllowed = 0;
    for (cameraIndex = 0; cameraIndex < 0x10; cameraIndex++) {
        fn_80027D8C(1 << cameraIndex, &fov, &near, &far);
        if (fov != -1.0f) cameraMask = 1 << cameraIndex;
    }
    lbl_1_bss_3E0 = cameraMask;
    lbl_1_bss_4EC = parentProcess;
    TimeLimitInit();
    fn_1_19CD0_inlined_main(GetQuizPlayer());
    lbl_1_bss_10A.mode = GetQuizCourse();
    lightId = fn_800286D0(&lbl_1_data_408, &lbl_1_data_414, &lbl_1_data_420);
    Hu3DGLightStaticSet(lightId, 1);
    Hu3DGLightInfinitytSet(lightId);
    Hu3DShadowCreate(30.0f, 20.0f, 5000.0f);
    Hu3DShadowPosSet(&shadowPosition, &shadowDirection, &shadowTarget);
    for (index = 0; index < 2; index++) {
        modelId = fn_80025A04(fn_80008DD0(lbl_1_data_47C[lbl_1_bss_10A.mode], 0x10000000, 2));
        fn_80026760(modelId, -1300.0f + 2600.0f * index, 0.0f, 0.0f);
        lbl_1_bss_10A.sideModelIds[index] = modelId;
        modelId = fn_80025A04(fn_80008DD0(lbl_1_data_490[lbl_1_bss_10A.mode], 0x10000000, 2));
        fn_80026760(modelId, -1000.0f + 2000.0f * index, 0.0f, 0.0f);
        fn_80026FFC(modelId);
        lbl_1_bss_10A.shadowModelIds[index] = modelId;
    }
    modelId = fn_80025A04(fn_80008DD0(lbl_1_data_4A4[lbl_1_bss_10A.mode], 0x10000000, 2));
    lbl_1_bss_10A.modelIds[0] = modelId;
    modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa50000, 45), 0x10000000, 2));
    fn_80026760(modelId, 0.0f, -150.0f, 50.0f);
    fn_80026B8C(modelId, 1);
    lbl_1_bss_10A.modelIds[1] = modelId;
    modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa50000, 46), 0x10000000, 2));
    fn_80026B8C(modelId, 1);
    lbl_1_bss_10A.modelIds[2] = modelId;
    modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa50000, 47), 0x10000000, 2));
    fn_80026B8C(modelId, 1);
    lbl_1_bss_10A.modelIds[3] = modelId;
    modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa50000, 48), 0x10000000, 2));
    lbl_1_bss_10A.modelIds[4] = modelId;
    lbl_1_bss_F4.charNo = lbl_80265728[GetQuizPlayer()].charNo;
    modelId = fn_80057068(lbl_1_bss_F4.charNo, 8);
    fn_80026760(modelId, 0.0f, 15.0f, -350.0f);
    fn_80026808(modelId, 0.0f, 90.0f, 0.0f);
    fn_80026E44(modelId);
    lbl_1_bss_F4.modelId = modelId;
    for (index = 0; index < 9; index++) {
        motionId = fn_8005B0B8(lbl_1_bss_F4.charNo, lbl_1_data_4B8[index]);
        lbl_1_bss_F4.motionIds[index] = motionId;
    }
    fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[0], 0.0f, 0.0f, ((1 << 30) | (1 << 0)));
    lbl_1_bss_10A.flag_80 = 1;
    for (index = 0; index < 6; index++) lbl_1_bss_98[index].modelId = -1;
    for (index = 0; index < 5; index++) lbl_1_bss_48[index] = -1;
    fn_1_1F04();
    fn_1_12608_inlined_main(lbl_1_bss_D4.responseMode);
    fn_8000DAC0(fn_1_1F4, 0xA, 0x1800, 0, lbl_1_bss_2B8[0]);
}

extern "C++" s32 fn_1_16B4(void) {
    Point3d position;
    s32 i;

    i = 0;
    while (i < 2) {
        fn_800267CC(lbl_1_bss_10A.modelIds[3], &position);
        position.x += 10.0f;
        if (position.x >= 450.0f) {
            position.x = 450.0f;
        }
        fn_80026760(lbl_1_bss_10A.modelIds[3], position.x, position.y, position.z);
        fn_80026760(lbl_1_bss_10A.modelIds[2], -position.x, position.y, position.z);
        i += 1;
    }
    if (position.x >= 450.0f) {
        return 1;
    }
    return 0;
}

void fn_1_12608(s32 argument);

void fn_1_19CD0(s32 baseIndex);

inline void fn_1_8FCC_inlined_main(s32 windowNo, u32 messageId, s16 insertNo) ;
inline void fn_1_8D14_inlined_main(u32 messageId, s16 messageSpeed, u8 windowIndex) ;

extern "C++" s32 fn_1_17A8(void) {
    Point3d position;
    s32 i;

    i = 0;
    while (i < 2) {
        fn_800267CC(lbl_1_bss_10A.modelIds[3], &position);
        position.x = position.x - 10.0f;
        if (position.x < 0.0f) {
            position.x = 0.0f;
        }
        fn_80026760(lbl_1_bss_10A.modelIds[3], position.x, position.y, position.z);
        fn_80026760(lbl_1_bss_10A.modelIds[2], -position.x, position.y, position.z);
        i += 1;
    }
    if (position.x <= 0.0f) {
        return 1;
    }
    return 0;
}

extern "C++" void fn_1_1898(void) {
    Point3d position;
    s32 i;

    if (lbl_1_bss_10A.flag_80 != 0) {
        i = 0;
        while (i < 2) {
            fn_800267CC(lbl_1_bss_10A.sideModelIds[i], &position);
            position.x -= 1.8f;
            if (position.x < -2600.0f) {
                position.x += 5200.0f;
            }
            fn_80026790(lbl_1_bss_10A.sideModelIds[i], &position);
            fn_800267CC(lbl_1_bss_10A.shadowModelIds[i], &position);
            position.x -= 3.6f;
            if (position.x < -2000.0f) {
                position.x += 4000.0f;
            }
            fn_80026790(lbl_1_bss_10A.shadowModelIds[i], &position);
            i += 1;
        }
        return;
    }
    if (lbl_1_bss_10A.flag_40 != 0) {
        fn_800267CC(lbl_1_bss_F4.modelId, &position);
        position.x += 3.6f;
        fn_80026790(lbl_1_bss_F4.modelId, &position);
    }
}

static inline s16 fn_1_89C0_inlined_main(s32 windowNo)
{
    s16 window;

    switch (windowNo) {
    case 0: window = lbl_1_bss_32A; break;
    case 1: window = lbl_1_bss_328; break;
    case 2: window = lbl_1_bss_326; break;
    case 3: window = lbl_1_bss_324; break;
    }
    return window;
}

static inline void fn_1_8DE8_inlined_main(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinExClose(lbl_1_bss_31E);
        lbl_1_bss_31E = -1;
    }
}

static inline void fn_1_8E38_inlined_main(s32 windowNo)
{
    s16 window = fn_1_89C0_inlined_main(windowNo);

    if (lbl_1_bss_31E != window) {
        if (lbl_1_bss_31E != -1) {
            fn_1_8DE8_inlined_main();
        }
        HuWinExOpen(window);
        lbl_1_bss_31E = window;
    }
}

extern "C++" void fn_1_1A4C(void) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_8E38_inlined_main(2);
    } else {
        fn_1_8E38_inlined_main(3);
    }
}
inline void fn_1_8D14_inlined_main(u32 messageId, s16 messageSpeed, u8 windowIndex) {
    s16 selectedWindow;
    s16 attributeWindow;
    s16 messageWindow;
    switch (windowIndex) {
    case 0: selectedWindow = lbl_1_bss_32A; break;
    case 1: selectedWindow = lbl_1_bss_328; break;
    case 2: selectedWindow = lbl_1_bss_326; break;
    case 3: selectedWindow = lbl_1_bss_324; break;
    }
    attributeWindow = selectedWindow;
    messageWindow = attributeWindow;
    HuWinAttrSet(messageWindow, 0x10);
    HuWinMesSet(messageWindow, messageId);
    HuWinMesSpeedSet(messageWindow, messageSpeed);
}

inline void fn_1_8FCC_inlined_main(s32 windowNo, u32 messageId, s16 insertNo) {
    HuWinInsertMesSet(fn_1_89C0_inlined_main(windowNo), messageId, insertNo);
}

extern "C++" void fn_1_1BAC(s32 messageSet) {
    s32 windowMode;
    s32 dataSetIndex;
    u32 mainMessage;
    u32 firstInsertMessage;
    u32 secondInsertMessage;
    u32 thirdInsertMessage;

    if (lbl_1_bss_47C->presenterLayout == 0) {
        windowMode = 2;
        dataSetIndex = 0;
    } else {
        windowMode = 3;
        dataSetIndex = 1;
    }

    mainMessage = lbl_1_data_394[messageSet][dataSetIndex];

    fn_1_8D14_inlined_main(mainMessage, 1, windowMode);

    switch (messageSet) {
    case 1:
    {
        firstInsertMessage = lbl_1_data_3BC[lbl_1_bss_D4.playerIndex];
        fn_1_8FCC_inlined_main(windowMode, firstInsertMessage, 0);
        break;
    }
    case 2:
    {
        secondInsertMessage = lbl_1_data_3D0[lbl_1_bss_D4.playerIndex];
        fn_1_8FCC_inlined_main(windowMode, secondInsertMessage, 0);
        break;
    }
    case 3:
    {
        thirdInsertMessage = lbl_1_data_3E4[lbl_1_bss_D4.playerIndex];
        fn_1_8FCC_inlined_main(windowMode, thirdInsertMessage, 0);
        break;
    }
    }
}

extern "C++" void fn_1_1EB4(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinExClose(lbl_1_bss_31E);
        lbl_1_bss_31E = -1;
    }
}

void fn_1_8FCC(s32 windowNo, u32 messageId, s16 insertNo);

void fn_1_8D14(u32 messageId, s16 messageSpeed, u8 windowNo);

static inline void fn_1_8394_inlined_main(s32 count) ;

static inline void fn_1_8394_inlined_main(s32 count) { lbl_1_bss_4B8 = count; }

extern "C++" void fn_1_1F04(void) {
    s16 positionUsed[5];
    s16 firstModelRemaining[6];
    s16 secondModelRemaining[5];
    s16 playerTypeCounts[5] = {9, 8, 7, 6, 5};
    s16 motionUsed[11];
    s32 playerIndex;
    s32 motionIndex;
    s16 selectedPlayer;
    s32 listIndex;
    s32 animationIndex;
    s16 selectionValue;
    s16 questionType;
    s32 textureSize;

    lbl_1_bss_10A.characterModelsCreated = 0;

    for (playerIndex = 0; playerIndex < 5; playerIndex++) {
        positionUsed[playerIndex] = 0;
        firstModelRemaining[playerIndex] = 0xF;
        secondModelRemaining[playerIndex] = 0xF;
    }
    firstModelRemaining[5] = 0xF;

    selectionValue = frandmod(6);
    firstModelRemaining[selectionValue] = firstModelRemaining[(selectionValue + 2) % 6] =
        firstModelRemaining[(selectionValue + 4) % 6] = 0;
    selectionValue = frandmod(5);
    secondModelRemaining[selectionValue] = secondModelRemaining[(selectionValue + 3) % 5] = 0;

    for (playerIndex = 0; playerIndex < 10; playerIndex++) {
        motionUsed[playerIndex] = 0;
    }
    motionUsed[lbl_1_bss_F4.charNo] = 1;
    if (lbl_1_bss_F4.charNo == 0) {
        motionUsed[4] = 1;
    } else if (lbl_1_bss_F4.charNo == 4) {
        motionUsed[0] = 1;
    }
    motionUsed[8] = 1;

    for (playerIndex = 0; playerIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; playerIndex++) {
        lbl_1_bss_128[playerIndex].motionStep = 0;
        do {
            selectionValue = frandmod(5);
        } while (positionUsed[selectionValue] != 0);
        positionUsed[selectionValue] = 1;
        lbl_1_bss_128[playerIndex].positionIndex = selectionValue;

        for (;;) {
            if ((u32)frandmod(0xA) < playerTypeCounts[lbl_1_bss_10A.mode] &&
                lbl_1_bss_10A.characterModelsCreated < 3) {
                selectionValue = frandmod(0xB);
                if (motionUsed[selectionValue] != 0) {
                    continue;
                }
                motionUsed[selectionValue] = 1;
                lbl_1_bss_10A.characterModelsCreated++;
            } else {
                selectionValue = -1;
            }
            break;
        }
        lbl_1_bss_128[playerIndex].characterId = selectionValue;

        if (selectionValue != -1) {
            lbl_1_bss_128[playerIndex].characterModelId = fn_80057068(selectionValue, 8);
            for (motionIndex = 0; motionIndex < 9; motionIndex++) {
                lbl_1_bss_128[playerIndex].characterMotions[motionIndex] =
                    fn_8005B0B8(selectionValue, lbl_1_data_4B8[motionIndex]);
            }
            fn_8005C2C8(lbl_1_bss_128[playerIndex].characterId,
                               lbl_1_bss_128[playerIndex].characterMotions[0],
                               0.0f, 0.0f, ((1 << 30) | (1 << 0)));
            fn_800269E8(lbl_1_bss_128[playerIndex].characterModelId, 1);
            fn_80026E44(lbl_1_bss_128[playerIndex].characterModelId);
            fn_800097F4(lbl_1_data_4DC[lbl_1_bss_128[playerIndex].characterId]);
            fn_800097F4(lbl_1_data_508[lbl_1_bss_128[playerIndex].characterId]);
            fn_800097F4(lbl_1_data_534[lbl_1_bss_128[playerIndex].characterId]);
        } else {
            lbl_1_bss_128[playerIndex].characterModelId = -1;
        }

        do {
            selectionValue = frandmod(6);
        } while (firstModelRemaining[selectionValue] == 0);
        lbl_1_bss_128[playerIndex].accessorySetIndex = selectionValue;
        do {
            selectionValue = frandmod(3) + 1;
        } while (firstModelRemaining[lbl_1_bss_128[playerIndex].accessorySetIndex] - selectionValue < 0);
        firstModelRemaining[lbl_1_bss_128[playerIndex].accessorySetIndex] -= selectionValue;
        lbl_1_bss_128[playerIndex].accessoryCount = selectionValue;

        do {
            selectionValue = frandmod(5);
        } while (secondModelRemaining[selectionValue] <= 0);
        lbl_1_bss_128[playerIndex].secondarySetIndex = selectionValue;
        do {
            selectionValue = frandmod(3) + 1;
        } while (secondModelRemaining[lbl_1_bss_128[playerIndex].secondarySetIndex] - selectionValue < 0);
        secondModelRemaining[lbl_1_bss_128[playerIndex].secondarySetIndex] -= selectionValue;
        lbl_1_bss_128[playerIndex].secondaryCount = selectionValue;
        OSReport("house:%d\n", lbl_1_bss_128[playerIndex].positionIndex);
        OSReport("player:%d\n", lbl_1_bss_128[playerIndex].characterId);
        OSReport("npc:%d %d\n", lbl_1_bss_128[playerIndex].accessorySetIndex,
                 lbl_1_bss_128[playerIndex].accessoryCount);
        OSReport("item:%d %d\n", lbl_1_bss_128[playerIndex].secondarySetIndex,
                 lbl_1_bss_128[playerIndex].secondaryCount);
    }

    memset(&lbl_1_bss_D4, 0, 0x20U);
    selectedPlayer = frandmod(lbl_1_data_3FC[lbl_1_bss_10A.mode]);
    do {
        questionType = frandmod(3);
        if (questionType == 0 && lbl_1_bss_128[selectedPlayer].characterId != -1 &&
            lbl_1_bss_128[selectedPlayer].characterId != 8) {
            break;
        }
    } while (questionType == 0);
    lbl_1_bss_D4.playerSlot = selectedPlayer;
    lbl_1_bss_D4.phase = questionType;
    switch (questionType) {
    case 0:
        lbl_1_bss_D4.playerIndex = lbl_1_bss_128[selectedPlayer].positionIndex;
        lbl_1_bss_D4.modelIndex = lbl_1_bss_128[selectedPlayer].characterId;
        lbl_1_bss_D4.responseMode = 3;
        fn_1_8394_inlined_main(lbl_1_bss_128[selectedPlayer].characterId);
        break;
    case 1:
        lbl_1_bss_D4.playerIndex = lbl_1_bss_128[selectedPlayer].secondarySetIndex;
        lbl_1_bss_D4.modelIndex = 0;
        for (listIndex = 0; listIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; listIndex++) {
            if (lbl_1_bss_128[listIndex].secondarySetIndex == lbl_1_bss_128[selectedPlayer].secondarySetIndex) {
                lbl_1_bss_D4.modelIndex += lbl_1_bss_128[listIndex].secondaryCount;
            }
        }
        lbl_1_bss_D4.responseMode = 1;
        fn_1_8394_inlined_main(lbl_1_bss_D4.modelIndex);
        break;
    case 2:
        lbl_1_bss_D4.playerIndex = lbl_1_bss_128[selectedPlayer].accessorySetIndex;
        lbl_1_bss_D4.modelIndex = 0;
        for (listIndex = 0; listIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; listIndex++) {
            if (lbl_1_bss_128[listIndex].accessorySetIndex == lbl_1_bss_128[selectedPlayer].accessorySetIndex) {
                lbl_1_bss_D4.modelIndex += lbl_1_bss_128[listIndex].accessoryCount;
            }
        }
        lbl_1_bss_D4.responseMode = 0;
        fn_1_8394_inlined_main(lbl_1_bss_D4.modelIndex);
        break;
    }

    textureSize = GXGetTexBufferSize(0xC0, 0xA4, 6, 0, 0);
    for (animationIndex = 0; animationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; animationIndex++) {
        s16 modelId = fn_80025A04(fn_80008DD0(DATANUM(0xa50000, 49), 0x10000000, 2));
        fn_80026760(modelId, 0.0f, 1000.0f, -30.0f);
        fn_800269E8(modelId, 1);
        lbl_1_bss_54.modelIds[animationIndex] = modelId;
        lbl_1_bss_54.states[animationIndex] = 0;
        fn_8000C200(lbl_1_bss_54.spriteAnimations[animationIndex] = HuSprAnimMake(0xC0, 0xA4, 0),
                          0x10000000);
        lbl_1_bss_54.spriteAnimations[animationIndex]->bmp->palData = NULL;
        lbl_1_bss_54.spriteAnimations[animationIndex]->bmp->palNum = 0;
        lbl_1_bss_54.spriteAnimations[animationIndex]->bmp->data =
            HuMemDirectMallocNum(2, textureSize, 0x10000000);
        lbl_1_bss_54.animationIds[animationIndex] =
            fn_80030B2C(lbl_1_bss_54.spriteAnimations[animationIndex], modelId, lbl_1_data_5F8);
    }
    fn_8002583C(2, 0, fn_1_829C);
    lbl_1_bss_54.currentModel = lbl_1_bss_54.frame = 0;
}

extern "C++" void fn_1_2B14(s16 playerIndex) {
    Point3d modelPosition;
    MicquizPlayerWork *player;
    s32 modelIndex;
    s32 setIndex;
    s16 jointMotionId;
    s32 modelCount;
    s16 startX;
    s16 modelId;

    player = &lbl_1_bss_128[playerIndex];
    for (modelIndex = 0; modelIndex < 5; modelIndex++) {
        player->models_28[modelIndex] = -1;
        player->models_32[modelIndex] = -1;
    }

    modelId = fn_80025A04(
        fn_80008DD0(lbl_1_data_454[player->positionIndex],
                             0x10000000, 2));
    modelPosition.x = 800.0f;
    modelPosition.y = 15.0f +
                      lbl_1_data_388[player->positionIndex];
    modelPosition.z = -650.0f;
    fn_80026790(modelId, &modelPosition);
    fn_8002D86C(modelId, 0.0f);
    fn_80026E44(modelId);
    player->modelId = modelId;

    if (player->positionIndex == 2) {
        modelId = fn_80025A04(
            fn_80008DD0(DATANUM(0xa50000, 53), 0x10000000, 2));
        modelPosition.y += 200.0f;
        fn_80026790(modelId, &modelPosition);
        player->alternateModelId = modelId;
    } else {
        player->alternateModelId = -1;
    }

    if (player->positionIndex == 3) {
        jointMotionId = fn_80030458(
            player->modelId,
            fn_80008DD0(DATANUM(0xa50000, 55), 0x10000000, 2));
        fn_8002C95C(player->modelId, jointMotionId);
        fn_8002D86C(player->modelId, 0.0f);
    } else {
        player->motionId = -1;
    }

    modelCount = player->accessoryCount;
    setIndex = player->accessorySetIndex;
    startX = frandmod(0x64) + 0x3E8;
    for (modelIndex = 0; modelIndex < modelCount; modelIndex++) {
        s32 jointMotionIndex;

        if (lbl_1_bss_98[setIndex].modelId == -1) {
            modelId = fn_80025A04(
                fn_80008DD0(lbl_1_data_424[setIndex],
                                     0x10000000, 2));
            lbl_1_bss_98[setIndex].modelId = modelId;
            for (jointMotionIndex = 0; jointMotionIndex < 4;
                 jointMotionIndex++) {
                jointMotionId = fn_80030458(
                    modelId,
                    fn_80008DD0(
                        lbl_1_data_424[setIndex] + 1 + jointMotionIndex,
                        0x10000000, 2));
                lbl_1_bss_98[setIndex].jointMotions[jointMotionIndex] =
                    jointMotionId;
            }
        } else {
            modelId = fn_80025D4C(
                lbl_1_bss_98[setIndex].modelId);
        }

        if (setIndex == 5) {
            fn_800268B0(modelId, 0.5f,
                              0.5f, 0.5f);
        }
        player->models_28[modelIndex] = modelId;
        fn_8002C95C(modelId,
                      lbl_1_bss_98[setIndex].jointMotions[2]);
        fn_800269E8(modelId, ((1 << 30) | (1 << 0)));
        fn_80026760(
            modelId, (f32)startX +
                (80.0f * (f32)modelIndex) +
                    (f32)(u32)frandmod(0x32),
            15.0f,
            -200.0f - (f32)((modelIndex & 1) * 0xC8));
        fn_80026808(modelId, 0.0f,
                        270.0f, 0.0f);
        fn_80026E44(modelId);
    }

    modelCount = player->secondaryCount;
    setIndex = player->secondarySetIndex;
    if (player->positionIndex == 0) {
        startX = 900.0f + (u32)frandmod(0xC8);
    } else {
        startX = 850.0f + (u32)frandmod(0xFA);
    }
    for (modelIndex = 0; modelIndex < modelCount; modelIndex++) {
        s16 verticalPosition;
        s16 zPosition;

        if (setIndex < 2) {
            verticalPosition = 60.0f + (u32)frandmod(0x64);
        } else {
            verticalPosition = 0xF;
        }

        if (lbl_1_bss_48[setIndex] == -1) {
            modelId = fn_80025A04(
                fn_80008DD0(lbl_1_data_468[setIndex],
                                     0x10000000, 2));
            lbl_1_bss_48[setIndex] = modelId;
        } else {
            modelId = fn_80025D4C(
                lbl_1_bss_48[setIndex]);
        }

        player->models_32[modelIndex] = modelId;
        zPosition = (frandmod(5) * 0x14) - 0x258;
        fn_80026760(
            modelId, (f32)(startX + modelIndex * 0x64 +
                          (u32)frandmod(0xA)),
            (f32)verticalPosition, (f32)zPosition);
        fn_80026E44(modelId);
    }
}

extern "C++" void fn_1_31CC(s16 playerIndex) {
    if (lbl_1_bss_2A4[playerIndex] == 0) {
        fn_1_2B14(playerIndex);
        lbl_1_bss_2A4[playerIndex] = fn_8000DAC0(fn_1_3258, 0x64U, 0x1800U, 0, lbl_1_bss_2B8[0]);
    }
}

extern MicquizPlayerWork lbl_1_bss_128[5];char lbl_1_data_5F8[12] = "v75G_hint";

extern "C++" void fn_1_3258(void) {
    Point3d position;
    Point3d effectPosition;
    Point3d secondPosition;
    MicquizPlayerWork *player;
    s32 modelIndex;
    f32 angle;

    player = &lbl_1_bss_128[lbl_1_bss_10A.currentPlayer];
    for (;;) {
        for (modelIndex = 0; modelIndex < 5; modelIndex++) {
            if (player->models_32[modelIndex] != -1) {
                fn_800267CC(player->models_32[modelIndex], &position);
                position.x -= 3.6f;
                fn_80026790(player->models_32[modelIndex], &position);
            }
        }
        for (modelIndex = 0; modelIndex < 5; modelIndex++) {
            if (player->models_28[modelIndex] != -1) {
                fn_800267CC(player->models_28[modelIndex], &position);
                position.x -= 7.2f;
                fn_80026790(player->models_28[modelIndex], &position);
            }
        }
        fn_800267CC(player->modelId, &position);
        position.x -= 3.6f;
        fn_80026790(player->modelId, &position);

        if (lbl_1_bss_54.playerStates[lbl_1_bss_10A.currentPlayer] == 0) {
            switch (lbl_1_bss_D4.phase) {
            case 0:
                if (lbl_1_bss_128[lbl_1_bss_10A.currentPlayer].positionIndex == 3) {
                    if (position.x <= 250.0f) {
                        fn_1_8280(lbl_1_bss_10A.currentPlayer);
                    }
                } else if (position.x <= -100.0f) {
                    fn_1_8280(lbl_1_bss_10A.currentPlayer);
                }
                break;
            case 1:
                fn_800267CC(player->models_32[player->secondaryCount / 2],
                                &secondPosition);
                if (secondPosition.x <= 200.0f) {
                    fn_1_8280(lbl_1_bss_10A.currentPlayer);
                }
                break;
            case 2:
                fn_800267CC(player->models_28[player->accessoryCount / 2],
                                &secondPosition);
                if (secondPosition.x <= 150.0f) {
                    fn_1_8280(lbl_1_bss_10A.currentPlayer);
                }
                break;
            }
        }
        if (player->characterModelId != -1) {
            switch (player->positionIndex) {
            case 0:
                position.z -= 50.0f;
                switch (player->motionStep) {
                case 0:
                    fn_80026A44(player->characterModelId, 1);
                    fn_8005C2C8(player->characterId, player->characterMotions[0],
                                       0.0f, 0.0f, ((1 << 30) | (1 << 0)));
                    player->motionStep++;
                    break;
                case 1:
                    if (position.x < 300.0f) {
                        fn_8002D86C(player->modelId, 1.0f);
                        HuAudFXPlay(0x524);
                        player->motionStep++;
                    }
                    break;
                case 2:
                    if (position.x < 150.0f) {
                        fn_8005C2C8(player->characterId, player->characterMotions[3],
                                           0.0f, 8.0f, 0);
                        player->motionStep++;
                    }
                    break;
                }
                break;
            case 1:
                position.y += player->motionValueA + player->motionValueD;
                position.z += -100.0f + player->motionValueB;
                switch (player->motionStep) {
                case 0:
                    fn_80026A44(player->characterModelId, 1);
                    if (position.x < 500.0f) {
                        player->motionStep++;
                    }
                    break;
                case 1:
                    player->motionValueA += 4.0f;
                    if (player->motionValueA >= lbl_1_data_560[player->characterId]) {
                        HuAudFXPlay(0x526);
                        player->motionStep++;
                    }
                    break;
                case 2:
                    player->motionValueC += 8.0f;
                    fn_80026808(player->characterModelId, 0.0f,
                                    (f32)(40.0 * sin(M_PI *
                                        (f64)player->motionValueC / 180.0)), 0.0f);
                    if (player->motionValueC >= 360.0f) {
                        player->motionStep++;
                        player->motionValueC = 0.0f;
                    }
                    break;
                case 3:
                    player->motionValueA -= 8.0f;
                    if (player->motionValueA <= 0.0f) {
                        fn_8005C2C8(player->characterId, player->characterMotions[5],
                                           0.0f, 8.0f, 0);
                        player->motionStep++;
                    }
                    break;
                case 4:
                    player->motionValueC += 1.0f;
                    player->motionValueA += 6.6666665f;
                    player->motionValueB += 3.3333333f;
                    player->motionValueD = (f32)(160.0 * sin(
                        M_PI * (f64)(6.0f * player->motionValueC) / 180.0));
                    if (player->motionValueC >= 30.0f) {
                        fn_8005C2C8(player->characterId, player->characterMotions[6],
                                           0.0f, 8.0f, 0);
                        player->motionStep++;
                    }
                    break;
                case 5:
                    if (fn_8005C1FC(player->characterId) <=
                        fn_8005C1B8(player->characterId)) {
                        fn_8005C2C8(player->characterId, player->characterMotions[0],
                                           0.0f, 8.0f, ((1 << 30) | (1 << 0)));
                        player->motionStep++;
                    }
                    break;
                }
                break;
            case 2:
                fn_80026760(player->alternateModelId, position.x,
                                330.0f + (position.y + player->motionValueA),
                                position.z);
                position.y += 100.0f;
                switch (player->motionStep) {
                case 0:
                    fn_80026A44(player->characterModelId, 1);
                    if (position.x < 200.0f) {
                        fn_8005C2C8(player->characterId, player->characterMotions[7],
                                           0.0f, 8.0f, 0);
                        player->motionStep++;
                    }
                    break;
                case 1:
                    if ((s16)fn_8005C750(player->characterId) != 0) {
                        HuAudFXPlay(0x522);
                        player->motionStep++;
                    }
                    break;
                case 2:
                    player->motionValueA += 4.0f;
                    if (player->motionValueA > 40.0f) {
                        player->motionStep++;
                    }
                    break;
                case 3:
                    player->motionValueA -= 4.0f;
                    if (player->motionValueA <= 0.0f) {
                        player->motionValueA = 0.0f;
                        player->motionValueB =
                            (f32)(u32)(frandmod(0x3C) + 0x3C);
                        fn_8005C2C8(player->characterId, player->characterMotions[0],
                                           0.0f, 8.0f, ((1 << 30) | (1 << 0)));
                        player->motionStep++;
                    }
                    break;
                case 4:
                    player->motionValueB -= 1.0f;
                    if (player->motionValueB <= 0.0f) {
                        fn_8005C2C8(player->characterId, player->characterMotions[7],
                                           0.0f, 8.0f, 0);
                        player->motionStep = 1;
                    }
                    break;
                }
                break;
            case 3:
                position.x += player->motionValueA;
                position.y += 130.0f + player->motionValueB;
                position.z -= 100.0f;
                switch (player->motionStep) {
                case 0:
                    if (position.x < 450.0f) {
                        fn_8002D86C(player->modelId, 8.0f);
                        player->motionStep++;
                    }
                    break;
                case 1:
                    if (fn_8002D4E0(player->modelId) >= 200.0f) {
                        fn_80026A44(player->characterModelId, 1);
                        fn_8005C2C8(player->characterId, player->characterMotions[8],
                                           0.0f, 0.0f, ((1 << 30) | (1 << 0)));
                        fn_80026808(player->characterModelId, 90.0f, 270.0f, 0.0f);
                        player->motionStep++;
                    }
                    break;
                case 2:
                    player->motionValueA -= 9.0f;
                    player->motionValueB += 2.3f;
                    if (player->motionValueA < -30.0f) {
                        fn_800267CC(player->modelId, &effectPosition);
                        effectPosition.x -= 175.0f;
                        effectPosition.y = 220.0f + effectPosition.y;
                        effectPosition.z = 50.0f + effectPosition.z;
                        fn_8005CF90(-1, &effectPosition, 2.0f);
                        HuAudFXPlay(0x523);
                        player->motionStep++;
                    }
                    break;
                case 3:
                    player->motionValueA -= 9.0f;
                    player->motionValueB += 2.3f;
                    if (position.x < -800.0f) {
                        player->motionStep++;
                    }
                    break;
                }
                break;
            case 4:
                switch (player->motionStep) {
                    case 0:
                        position.y = 660.0f;
                        position.z -= 30.0f;
                        if (position.x < 100.0f) {
                            fn_8005C2C8(player->characterId, player->characterMotions[5],
                                               0.0f, 0.0f, 0);
                            fn_80026A44(player->characterModelId, 1);
                            player->motionValueA = player->motionValueB =
                                player->motionValueC = player->motionValueD = 0.0f;
                            player->motionStep++;
                        }
                        break;
                    case 1:
                        player->motionValueB += 2.0f;
                        player->motionValueA += player->motionValueB;
                        if (player->motionValueA >= 470.0f) {
                            fn_8005C2C8(player->characterId, player->characterMotions[6],
                                               0.0f, 8.0f, 0);
                            HuAudFXPlay(0x525);
                            player->motionValueA = 470.0f;
                            player->motionStep++;
                        }
                        position.y = (710.0f - player->motionValueA) - 50.0f;
                        position.z -= 30.0f;
                        break;
                    case 2:
                        position.y = 190.0f;
                        position.z -= 30.0f;
                        if (fn_8005C1FC(player->characterId) <=
                            fn_8005C1B8(player->characterId)) {
                            fn_8005C2C8(player->characterId, player->characterMotions[0],
                                               0.0f, 8.0f, ((1 << 30) | (1 << 0)));
                            player->motionStep++;
                            player->motionValueC = 4.0f;
                            player->motionValueD = 0.0f;
                        }
                        break;
                    case 3:
                        position.y = (f32)((f64)position.y +
                            ((f64)player->motionValueC * sin(
                                M_PI * (f64)player->motionValueD / 180.0)));
                        fn_80026790(player->modelId, &position);
                        position.y += 80.0f;
                        position.z -= 30.0f;
                        player->motionValueD += 8.0f;
                        player->motionValueC -= 0.04f;
                        if (player->motionValueC < 0.0f) {
                            fn_8005C2C8(player->characterId, player->characterMotions[5],
                                               0.0f, 8.0f, 0);
                            player->motionValueA = 16.0f;
                            player->motionValueB = 0.0f;
                            player->motionValueC = 4.0f;
                            player->motionValueD = 180.0f;
                            player->motionStep++;
                        }
                        break;
                    case 4:
                        position.y = (f32)((f64)position.y +
                            ((f64)player->motionValueC * sin(
                                M_PI * (f64)player->motionValueD / 180.0)));
                        fn_80026790(player->modelId, &position);
                        player->motionValueD += 8.0f;
                        player->motionValueC -= 0.04f;
                        player->motionValueB += player->motionValueA;
                        player->motionValueA -= 1.0f;
                        if ((player->motionValueB <= 0.0f) &&
                            (player->motionValueA < 0.0f)) {
                            fn_8005C2C8(player->characterId, player->characterMotions[6],
                                               0.0f, 8.0f, 0);
                            player->motionValueB = 0.0f;
                            player->motionStep++;
                        }
                        position.y = 190.0f + player->motionValueB;
                        position.z -= 30.0f;
                        break;
                    case 5:
                        if (fn_8005C1FC(player->characterId) <=
                            fn_8005C1B8(player->characterId)) {
                            fn_8005C2C8(player->characterId, player->characterMotions[0],
                                               0.0f, 8.0f, ((1 << 30) | (1 << 0)));
                            player->motionStep++;
                        }
                        position.y = (f32)((f64)position.y +
                            ((f64)player->motionValueC * sin(
                                M_PI * (f64)player->motionValueD / 180.0)));
                        fn_80026790(player->modelId, &position);
                        player->motionValueD += 8.0f;
                        player->motionValueC -= 0.04f;
                        if (player->motionValueC < 0.0f) {
                            player->motionValueC = 0.0f;
                        }
                        position.y += 80.0f;
                        position.z -= 30.0f;
                        break;
                    case 6:
                        position.y = (f32)((f64)position.y +
                            ((f64)player->motionValueC * sin(
                                M_PI * (f64)player->motionValueD / 180.0)));
                        fn_80026790(player->modelId, &position);
                        player->motionValueD += 8.0f;
                        player->motionValueC -= 0.04f;
                        if (player->motionValueC < 0.0f) {
                            player->motionValueC = 0.0f;
                        }
                        position.y += 80.0f;
                        position.z -= 30.0f;
                        break;
                }
                break;
            }
            fn_80026790(player->characterModelId, &position);
        } else if (player->positionIndex == 2) {
            fn_800267CC(player->modelId, &position);
            position.y += 330.0f;
            fn_80026790(player->alternateModelId, &position);
        }
        HuPrcVSleep();
    }
}

extern "C++" s32 fn_1_45B0(s16 playerIndex) {
    Point3d position;
    MicquizPlayerModels *player;
    s32 modelIndex;

    player = &lbl_1_bss_128[playerIndex];
    fn_800267CC(player->modelId, &position);
    if (position.x > -900.0f) {
        return 0;
    }
    fn_800269E8(player->modelId, 1);
    modelIndex = 0;
    while (modelIndex < 5) {
        if (player->models_32[modelIndex] != -1) {
            fn_800267CC(player->models_32[modelIndex], &position);
            if (position.x > -700.0f) {
                return 0;
            }
        }
        modelIndex += 1;
    }
    modelIndex = 0;
    while (modelIndex < 5) {
        if (player->models_28[modelIndex] != -1) {
            fn_800267CC(player->models_28[modelIndex], &position);
            if (position.x > -700.0f) {
                return 0;
            }
        }
        modelIndex += 1;
    }
    HuPrcKill(lbl_1_bss_2A4[playerIndex]);
    lbl_1_bss_2A4[playerIndex] = 0;
    return 1;
}

extern "C++" s32 fn_1_4728(s16 playerIndex) {
    Point3d position;
    MicquizPlayerModels *player;
    s32 modelIndex;

    player = &lbl_1_bss_128[playerIndex];
    fn_800267CC(player->modelId, &position);
    if (position.x > -900.0f) {
        return 0;
    }
    fn_800269E8(player->modelId, 1);
    modelIndex = 0;
    while (modelIndex < 5) {
        if (player->models_32[modelIndex] != -1) {
            fn_800267CC(player->models_32[modelIndex], &position);
            if (position.x > -700.0f) {
                return 0;
            }
            fn_800269E8(player->models_32[modelIndex], 1);
        }
        modelIndex += 1;
    }
    modelIndex = 0;
    while (modelIndex < 5) {
        if (player->models_28[modelIndex] != -1) {
            fn_800267CC(player->models_28[modelIndex], &position);
            if (position.x > -700.0f) {
                return 0;
            }
            fn_800269E8(player->models_28[modelIndex], 1);
        }
        modelIndex += 1;
    }
    HuPrcKill(lbl_1_bss_2A4[playerIndex]);
    lbl_1_bss_2A4[playerIndex] = 0;
    return 1;
}

extern "C++" void fn_1_48C8(void) {
    MicquizPlayerModels *player;
    Point3d position;
    s32 playerIndex;
    s32 modelIndex;
    s32 modelListCount;
    f32 radius;
    f32 angle;
    f32 angleStep;

    player = &lbl_1_bss_128[lbl_1_bss_D4.playerSlot];
    switch (lbl_1_bss_D4.phase) {
    case 0:
        position.x = 0.0f;
        position.y = 15.0f +
                     lbl_1_data_388[player->positionIndex];
        position.z = -650.0f;
        fn_80026790(player->modelId, &position);
        fn_8002D414(player->modelId, 0.0f);
        fn_8002D86C(player->modelId, 0.0f);
        fn_8005BE80(player->characterId, player->characterMotions[0]);
        switch (player->positionIndex) {
        case 0:
            position.z -= 50.0f;
            break;
        case 1:
            position.z -= 50.0f;
            break;
        case 2:
            fn_80026760(player->alternateModelId, position.x,
                            330.0f + position.y, position.z);
            position.y += 100.0f;
            break;
        case 3:
            position.y += 100.0f;
            position.z -= 100.0f;
            break;
        case 4:
            position.y += 80.0f;
            break;
        }
        fn_80026A44(player->modelId, 1);
        fn_80026790(player->characterModelId, &position);
        return;
    case 1:
        fn_80026760(lbl_1_bss_F4.modelId, -800.0f,
                        15.0f, -350.0f);
        fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[1],
                           0.0f, 0.0f,
                           0x40000001U);
        radius = 150.0f;
        angle = 0.0f;
        if (lbl_1_bss_D4.modelIndex < 3) {
            angleStep = 180.0f;
        } else if (lbl_1_bss_D4.modelIndex == 3) {
            angleStep = 120.0f;
        } else if (lbl_1_bss_D4.modelIndex == 4) {
            angleStep = 90.0f;
        } else if (lbl_1_bss_D4.modelIndex == 5) {
            angleStep = 72.0f;
        } else {
            angleStep = 60.0f;
        }
        for (playerIndex = 0; playerIndex < 20; playerIndex++) {
            lbl_1_bss_20[playerIndex] = -1;
        }
        playerIndex = 0;
        modelListCount = 0;
        for (;
             playerIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode];
             playerIndex++) {
            if (lbl_1_bss_D4.playerIndex ==
                lbl_1_bss_128[playerIndex].secondarySetIndex) {
                for (modelIndex = 0;
                     modelIndex < lbl_1_bss_128[playerIndex].secondaryCount;
                     modelIndex++) {
                    lbl_1_bss_20[modelListCount++] =
                        lbl_1_bss_128[playerIndex].models_32[modelIndex];
                    fn_800269E8(
                        lbl_1_bss_128[playerIndex].models_32[modelIndex], 1);
                    fn_800267CC(
                        lbl_1_bss_128[playerIndex].models_32[modelIndex],
                        &position);
                    position.x = (f32)(radius *
                        sin((M_PI *
                             (f64)(30.0f + angle)) /
                            180.0));
                    if (lbl_1_bss_D4.playerIndex <= 1) {
                        position.y = (f32)(80.0 -
                            (30.0 * cos((M_PI *
                                (f64)(30.0f + angle)) /
                                180.0)));
                    }
                    position.z = (f32)(-350.0 +
                        (radius * cos((M_PI *
                            (f64)(30.0f + angle)) /
                            180.0)));
                    fn_80026790(
                        lbl_1_bss_128[playerIndex].models_32[modelIndex],
                        &position);
                    angle += angleStep;
                    if (angle >= 360.0f) {
                        angle -= 330.0f;
                        radius += 100.0f;
                    }
                }
            }
        }
        return;
    case 2:
        fn_80026760(lbl_1_bss_F4.modelId, -800.0f,
                        15.0f, -350.0f);
        fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[1],
                           0.0f, 0.0f,
                           0x40000001U);
        for (playerIndex = 0; playerIndex < 20; playerIndex++) {
            lbl_1_bss_20[playerIndex] = -1;
        }
        position.x = 700.0f;
        position.z = -200.0f;
        angleStep = 1000.0f;
        playerIndex = 0;
        modelListCount = 0;
        for (;
             playerIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode];
             playerIndex++) {
            if (lbl_1_bss_D4.playerIndex ==
                lbl_1_bss_128[playerIndex].accessorySetIndex) {
                for (modelIndex = 0;
                     modelIndex < lbl_1_bss_128[playerIndex].accessoryCount;
                     modelIndex++) {
                    lbl_1_bss_20[modelListCount++] =
                        lbl_1_bss_128[playerIndex].models_28[modelIndex];
                    fn_80026760(
                        lbl_1_bss_128[playerIndex].models_28[modelIndex],
                        position.x, 15.0f, position.z);
                    fn_80026A44(
                        lbl_1_bss_128[playerIndex].models_28[modelIndex], 1);
                    position.x += 200.0f;
                    if (position.x > angleStep) {
                        position.x = 600.0f +
                                     (f32)(u32)frandmod(0x64);
                        if (position.z > -350.0f) {
                            position.z = -600.0f;
                        } else {
                            position.z -= 200.0f;
                        }
                        angleStep = 1200.0f;
                    }
                }
            }
        }
    }
}

extern "C++" void fn_1_5194(void) {
    MicquizPlayerWork *playerWork;

    playerWork = &lbl_1_bss_128[lbl_1_bss_D4.playerSlot];
    switch (lbl_1_bss_D4.phase) {
    case 0:
        fn_80026760(playerWork->modelId, 0.0f,
                        30.0f + lbl_1_data_388[playerWork->positionIndex],
                        -900.0f);
        fn_8002D414(playerWork->modelId, 0.0f);
        fn_8002D86C(playerWork->modelId, 0.0f);
        break;
    case 1:
    case 2:
    default:
        break;
    }
}

extern "C++" void fn_1_5288(void) {
    Point3d position;
    Point3d effectPosition;
    s16 counter;
    s32 index;
    s32 animationIndex;
    s32 secondAnimationIndex;
    s32 thirdAnimationIndex;
    s32 fourthAnimationIndex;
    s32 fifthAnimationIndex;
    s32 sixthAnimationIndex;
    s32 seventhAnimationIndex;
    MicquizPlayerWork *player;

    player = &lbl_1_bss_128[lbl_1_bss_D4.playerSlot];
    switch (lbl_1_bss_D4.phase) {                /* switch 1; irregular */
    case 0:                                         /* switch 1 */
        fn_800267CC(player->modelId, &position);
        switch (lbl_1_bss_D4.playerIndex) {            /* switch 2; irregular */
        case 0:                                     /* switch 2 */
            switch (lbl_1_bss_D4.motionStep) {        /* switch 3; irregular */
            case 0:                                 /* switch 3 */
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    fn_8002D86C(player->modelId, 1.0f);
                    HuAudFXPlay(0x524);
                    lbl_1_bss_D4.timer = 0;
                    lbl_1_bss_D4.motionStep += 1;
                    return;
                }
                break;
            case 1:                                 /* switch 3 */
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    lbl_1_bss_47C->presentationBusy = 0;
                    if (lbl_1_bss_10A.completionFlag == 0) {
                        fn_8005C2C8(player->characterId, player->characterMotions[2], 0.0f, 8.0f, 0U);
                    } else {
                        fn_8005C2C8(player->characterId, player->characterMotions[4], 0.0f, 8.0f, 0U);
                    }
                    lbl_1_bss_D4.timer = 0;
                    lbl_1_bss_D4.motionStep += 1;
                    return;
                }
                break;
            case 2:                                 /* switch 3 */
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    if (fn_8005C1FC(player->characterId) <= fn_8005C1B8(player->characterId)) {
                        fn_1_834C();
                        lbl_1_bss_D4.motionStep += 1;
                        return;
                    }
                }
                break;
            case 3:                                 /* switch 3 */
                if (WipeCheck() == 0) {
for (animationIndex = 0; animationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; animationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[animationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                    return;
                }
                break;
            }
            break;
        case 1:                                     /* switch 2 */
            switch (lbl_1_bss_D4.motionStep) {        /* switch 4; irregular */
            case 0:                                 /* switch 4 */
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    fn_8005C2C8(player->characterId, player->characterMotions[5], 0.0f, 8.0f, 0U);
                    lbl_1_bss_D4.motionStep += 1;
                    lbl_1_bss_D4.timer = 0;
                }
                break;
            case 1:                                 /* switch 4 */
                lbl_1_bss_D4.oscillationPhase += 1.0f;
                lbl_1_bss_D4.sequenceValue += 6.6666665f;
                lbl_1_bss_D4.sequenceOffset += 3.3333333f;
                lbl_1_bss_D4.verticalMotion = (f32) (200.0 * sin((M_PI * (f64) (6.0f * lbl_1_bss_D4.oscillationPhase)) / 180.0));
                if (lbl_1_bss_D4.oscillationPhase >= 30.0f) {
                    fn_8005C2C8(player->characterId, player->characterMotions[6], 0.0f, 8.0f, 0U);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 2:                                 /* switch 4 */
                if (fn_8005C1FC(player->characterId) <= fn_8005C1B8(player->characterId)) {
                    lbl_1_bss_47C->presentationBusy = 0;
                    fn_8005C2C8(player->characterId, player->characterMotions[0], 0.0f, 8.0f, 0x40000001U);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 3:                                 /* switch 4 */
                if (lbl_1_bss_D4.timer++ > 0x3C) {
                    fn_1_834C();
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 4:                                 /* switch 4 */
                if (WipeCheck() == 0) {
for (secondAnimationIndex = 0; secondAnimationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; secondAnimationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[secondAnimationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                }
                break;
            }
        fn_80026760(player->characterModelId, position.x, lbl_1_bss_D4.verticalMotion + (position.y + lbl_1_bss_D4.sequenceValue), (position.z + lbl_1_bss_D4.sequenceOffset) - 100.0f);
            return;
        case 2:                                     /* switch 2 */
            switch (lbl_1_bss_D4.motionStep) {        /* switch 5; irregular */
            case 0:                                 /* switch 5 */
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    fn_8005C2C8(player->characterId, player->characterMotions[7], 0.0f, 8.0f, 0U);
                    lbl_1_bss_D4.timer = 0;
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 1:                                 /* switch 5 */
                if ((s16)fn_8005C750(player->characterId) != 0) {
                    HuAudFXPlay(0x522);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 2:                                 /* switch 5 */
                lbl_1_bss_D4.sequenceValue += 4.0f;
                if (lbl_1_bss_D4.sequenceValue > 40.0f) {
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 3:                                 /* switch 5 */
                lbl_1_bss_D4.sequenceValue -= 4.0f;
                if (lbl_1_bss_D4.sequenceValue <= 0.0f) {
                    lbl_1_bss_D4.sequenceValue = 0.0f;
                    lbl_1_bss_47C->presentationBusy = 0;
                    fn_8005C2C8(player->characterId, player->characterMotions[0], 0.0f, 8.0f, 0x40000001U);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 4:                                 /* switch 5 */
                if (lbl_1_bss_D4.timer++ > 0x3C) {
                    fn_1_834C();
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 5:                                 /* switch 5 */
                if (WipeCheck() == 0) {
for (thirdAnimationIndex = 0; thirdAnimationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; thirdAnimationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[thirdAnimationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                }
                break;
            }
        fn_80026760(player->alternateModelId, position.x, 330.0f + position.y + lbl_1_bss_D4.sequenceValue, position.z);
            return;
        case 3:                                     /* switch 2 */
            switch (lbl_1_bss_D4.motionStep) {        /* switch 6; irregular */
            case 0:                                 /* switch 6 */
                fn_8002D86C(player->modelId, 8.0f);
                lbl_1_bss_D4.motionStep += 1;
                break;
            case 1:                                 /* switch 6 */
                if (fn_8002D4E0(player->modelId) > 190.0f) {
    effectPosition.x = position.x - 200.0f;
                    effectPosition.y = 200.0f + position.y;
                    effectPosition.z = 10.0f + position.z;
                    fn_8005C2C8(player->characterId, player->characterMotions[8], 0.0f, 8.0f, 0x40000001U);
                    fn_8005CF90(-1, &effectPosition, 2.0f);
                    HuAudFXPlay(0x523);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 2:                                 /* switch 6 */
                lbl_1_bss_D4.sequenceValue -= 9.0f;
                lbl_1_bss_D4.sequenceOffset += 2.5f;
                if (lbl_1_bss_D4.sequenceValue < -200.0f) {
                    lbl_1_bss_47C->presentationBusy = 0;
                }
                if (lbl_1_bss_D4.sequenceValue < -800.0f) {
                    fn_1_834C();
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 3:                                 /* switch 6 */
                if (WipeCheck() == 0) {
for (fourthAnimationIndex = 0; fourthAnimationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; fourthAnimationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[fourthAnimationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                }
                break;
            }
        fn_80026760(player->characterModelId, position.x + lbl_1_bss_D4.sequenceValue, 130.0f + position.y + lbl_1_bss_D4.sequenceOffset, position.z - 100.0f);
            return;
        case 4:                                     /* switch 2 */
            switch (lbl_1_bss_D4.motionStep) {        /* switch 7; irregular */
            case 0:                                 /* switch 7 */
                if (lbl_1_bss_D4.timer++ > 0) {
                    fn_8005C2C8(player->characterId, player->characterMotions[5], 0.0f, 8.0f, 0U);
                    lbl_1_bss_D4.timer = 0;
                    lbl_1_bss_D4.sequenceValue = 16.0f;
                    lbl_1_bss_D4.sequenceOffset = 0.0f;
                    lbl_1_bss_D4.oscillationPhase = 180.0f;
                    lbl_1_bss_D4.verticalMotion = 4.0f;
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 1:                                 /* switch 7 */
                lbl_1_bss_D4.oscillationPhase += 8.0f;
                position.y = (f32) ((f64) position.y + ((f64) lbl_1_bss_D4.verticalMotion * sin((M_PI * (f64) lbl_1_bss_D4.oscillationPhase) / 180.0)));
                lbl_1_bss_D4.verticalMotion -= 0.04f;
                lbl_1_bss_D4.sequenceOffset += lbl_1_bss_D4.sequenceValue;
                lbl_1_bss_D4.sequenceValue -= 1.0f;
                if ((lbl_1_bss_D4.sequenceOffset <= 0.0f) && (lbl_1_bss_D4.sequenceValue < 0.0f)) {
                    fn_8005C2C8(player->characterId, player->characterMotions[6], 0.0f, 8.0f, 0U);
                    HuAudFXPlay(0x525);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 2:                                 /* switch 7 */
                lbl_1_bss_D4.sequenceOffset = (f32) (70.0f + (position.y - 180.0f));
                lbl_1_bss_D4.oscillationPhase += 8.0f;
                position.y = (f32) ((f64) position.y + ((f64) lbl_1_bss_D4.verticalMotion * sin((M_PI * (f64) lbl_1_bss_D4.oscillationPhase) / 180.0)));
                lbl_1_bss_D4.verticalMotion -= 0.04f;
                if (fn_8005C1FC(player->characterId) <= fn_8005C1B8(player->characterId)) {
                    lbl_1_bss_47C->presentationBusy = 0;
                    fn_8005C2C8(player->characterId, player->characterMotions[0], 0.0f, 8.0f, 0x40000001U);
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 3:                                 /* switch 7 */
                lbl_1_bss_D4.sequenceOffset = (f32) (70.0f + (position.y - 180.0f));
                lbl_1_bss_D4.oscillationPhase += 8.0f;
                position.y = (f32) ((f64) position.y + ((f64) lbl_1_bss_D4.verticalMotion * sin((M_PI * (f64) lbl_1_bss_D4.oscillationPhase) / 180.0)));
                lbl_1_bss_D4.verticalMotion -= 0.04f;
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    fn_1_834C();
                    lbl_1_bss_D4.motionStep += 1;
                }
                break;
            case 4:                                 /* switch 7 */
                lbl_1_bss_D4.sequenceOffset = (f32) (70.0f + (position.y - 180.0f));
                lbl_1_bss_D4.oscillationPhase += 8.0f;
                position.y = (f32) ((f64) position.y + ((f64) lbl_1_bss_D4.verticalMotion * sin((M_PI * (f64) lbl_1_bss_D4.oscillationPhase) / 180.0)));
                lbl_1_bss_D4.verticalMotion -= 0.04f;
                if (WipeCheck() == 0) {
for (fifthAnimationIndex = 0; fifthAnimationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; fifthAnimationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[fifthAnimationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                }
                break;
            }
    fn_80026760(player->modelId, position.x, position.y, position.z);
             fn_80026760(player->characterModelId, position.x, 180.0f + lbl_1_bss_D4.sequenceOffset, position.z - 50.0f);
            return;
        }
        break;
    case 1:                                         /* switch 1 */
        {
            switch (lbl_1_bss_D4.motionStep) {              /* switch 8 */
            case 0:                                 /* switch 8 */
                fn_800267CC(lbl_1_bss_F4.modelId, &position);
    position.x += 3.6f;
                if (position.x >= 0.0f) {
    position.x = 0.0f;
                    lbl_1_bss_D4.motionStep += 1;
                }
                fn_80026790(lbl_1_bss_F4.modelId, &position);
                return;
            case 1:                                 /* switch 8 */
                fn_80026874(lbl_1_bss_F4.modelId, &position);
                position.y -= 5.0f;
                if (position.y <= 0.0f) {
                    fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[0], 0.0f, 8.0f, 0x40000001U);
                    position.y = 0.0f;
                    lbl_1_bss_D4.motionStep += 1;
                }
                fn_80026838(lbl_1_bss_F4.modelId, &position);
                return;
            case 2:                                 /* switch 8 */
                lbl_1_bss_D4.sequenceValue += 1.0f;
                if (lbl_1_bss_D4.sequenceValue >= 10.0f) {
                    fn_800267CC(lbl_1_bss_20[lbl_1_bss_D4.timer], &position);
                    position.z += 50.0f;
                    position.y += 50.0f;
                    fn_8005CF90(2, &position, 1.0f);
                    HuAudFXPlay(0x527);
                    OSReport("smoke\n");
                    lbl_1_bss_D4.sequenceValue = 0.0f;
                    lbl_1_bss_D4.timer += 1;
                    if (lbl_1_bss_D4.timer >= lbl_1_bss_D4.modelIndex) {
                        lbl_1_bss_D4.motionStep += 1;
                    }
                }
                if ((lbl_1_bss_D4.sequenceValue == 5.0f) && (lbl_1_bss_D4.timer != 0)) {
                    fn_80026A44(lbl_1_bss_20[lbl_1_bss_D4.timer - 1], 1U);
                    return;
                }
                break;
            case 3:                                 /* switch 8 */
                lbl_1_bss_D4.sequenceValue += 1.0f;
                if (lbl_1_bss_D4.sequenceValue == 5.0f) {
                    if (lbl_1_bss_20[lbl_1_bss_D4.timer - 1] != -1) {
                        fn_80026A44(lbl_1_bss_20[lbl_1_bss_D4.timer - 1], 1U);
                    }
                    lbl_1_bss_47C->presentationBusy = 0;
                    lbl_1_bss_D4.timer = 0;
                    lbl_1_bss_D4.motionStep += 1;
                    return;
                }
                break;
            case 4:                                 /* switch 8 */
                if (lbl_1_bss_D4.timer++ > 0x1E) {
                    if (lbl_1_bss_10A.completionFlag == 0) {
                        fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[2], 0.0f, 8.0f, 0U);
                    } else {
                        fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[4], 0.0f, 8.0f, 0U);
                    }
                    lbl_1_bss_D4.timer = 0;
                    lbl_1_bss_D4.motionStep += 1;
                    return;
                }
                break;
            case 5:                                 /* switch 8 */
                if (fn_8005C1FC(lbl_1_bss_F4.charNo) <= fn_8005C1B8(lbl_1_bss_F4.charNo)) {
                    fn_1_834C();
                    lbl_1_bss_D4.motionStep += 1;
                    return;
                }
                break;
            case 6:                                 /* switch 8 */
                if (WipeCheck() == 0) {
for (sixthAnimationIndex = 0; sixthAnimationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; sixthAnimationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[sixthAnimationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                    return;
                }
                break;
            }
        }
        break;
    case 2:                                         /* switch 1 */
            switch (lbl_1_bss_D4.motionStep) {              /* switch 9 */
            case 0:                                 /* switch 9 */
                fn_800267CC(lbl_1_bss_F4.modelId, &position);
    position.x += 3.6f;
                if (position.x >= 0.0f) {
    position.x = 0.0f;
                    lbl_1_bss_D4.motionStep += 1;
                }
                fn_80026790(lbl_1_bss_F4.modelId, &position);
for (index = 0; index < lbl_1_bss_D4.modelIndex; index++) {
                    fn_800267CC(lbl_1_bss_20[index], &position);
    position.x -= 3.6f;
                    fn_80026790(lbl_1_bss_20[index], &position);
}
                return;
            case 1:                                 /* switch 9 */
                fn_80026874(lbl_1_bss_F4.modelId, &position);
                position.y -= 5.0f;
                if (position.y <= 0.0f) {
                    fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[0], 0.0f, 8.0f, 0x40000001U);
                    position.y = 0.0f;
for (index = 0; index < lbl_1_bss_D4.modelIndex; index++) {
                        fn_8002CCC0(lbl_1_bss_20[index], lbl_1_bss_98[lbl_1_bss_D4.playerIndex].jointMotions[0], 0.0f, 8.0f, 0x40000001U);
}
                    lbl_1_bss_D4.motionStep += 1;
                }
                fn_80026838(lbl_1_bss_F4.modelId, &position);
for (index = 0; index < lbl_1_bss_D4.modelIndex; index++) {
                    fn_80026874(lbl_1_bss_20[index], &position);
                    position.y += 5.0f;
                    fn_80026838(lbl_1_bss_20[index], &position);
}
                return;
            case 2:                                 /* switch 9 */
                lbl_1_bss_D4.motionStep += 1;
                return;
            case 3:                                 /* switch 9 */
                lbl_1_bss_47C->presentationBusy = 0;
                lbl_1_bss_D4.motionStep += 1;
                return;
            case 4:                                 /* switch 9 */
                if (lbl_1_bss_10A.completionFlag == 0) {
                    fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[2], 0.0f, 8.0f, 0U);
                } else {
                    fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[4], 0.0f, 8.0f, 0U);
                }
for (index = 0; index < lbl_1_bss_D4.modelIndex; index++) {
                    if (lbl_1_bss_10A.completionFlag == 0) {
                        fn_8002CCC0(lbl_1_bss_20[index], lbl_1_bss_98[lbl_1_bss_D4.playerIndex].jointMotions[3], 0.0f, 8.0f, 0x40000001U);
                        if ((index == 0) && (lbl_1_bss_D4.playerIndex != 4)) {
                            HuAudFXPlay(lbl_1_data_43C[lbl_1_bss_D4.playerIndex]);
                        }
                    } else {
                        fn_8002CCC0(lbl_1_bss_20[index], lbl_1_bss_98[lbl_1_bss_D4.playerIndex].jointMotions[0], 0.0f, 8.0f, 0x40000001U);
                    }
}
                lbl_1_bss_D4.motionStep += 1;
                return;
            case 5:                                 /* switch 9 */
                if (fn_8005C1FC(lbl_1_bss_F4.charNo) <= fn_8005C1B8(lbl_1_bss_F4.charNo)) {
                    fn_1_834C();
                    lbl_1_bss_D4.motionStep += 1;
                    return;
                }
                break;
            case 6:                                 /* switch 9 */
                if (WipeCheck() == 0) {
for (seventhAnimationIndex = 0; seventhAnimationIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]; seventhAnimationIndex++) {
                        HuSprAnimKill(lbl_1_bss_54.spriteAnimations[seventhAnimationIndex]);
}
                    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                    fn_1_1EB4();
                    HuMCListenerKill();
                    HuMCClose();
                    fn_8003A5E4(1, 1);
                }
                break;
            }
        break;
    }
}

extern "C++" void fn_1_7188(void) {
    MicquizPlayerWork *playerWork;

    playerWork = &lbl_1_bss_128[lbl_1_bss_D4.playerSlot];
    switch (lbl_1_bss_D4.phase) {
    case 0:
    case 1:
    case 2:
        break;
    default:
        break;
    }
}

extern "C++" void fn_1_71D0(void) {
    Point3d position;
    MicquizPlayerWork *currentPlayer;

    currentPlayer = &lbl_1_bss_128[lbl_1_bss_D4.playerSlot];

    switch (lbl_1_bss_D4.phase) {
    case 0:
        position.x = 0.0f;
        position.y = 15.0f + lbl_1_data_388[currentPlayer->positionIndex];
        position.z = -650.0f;
        fn_80026790(currentPlayer->modelId, &position);
        fn_8002D414(currentPlayer->modelId, 0.0f);
        fn_8002D86C(currentPlayer->modelId, 0.0f);
        if (currentPlayer->positionIndex == 2) {
            fn_80026760(currentPlayer->alternateModelId, position.x,
                            330.0f + position.y, position.z);
        }
        break;

    case 1: {
        f32 radius;
        f32 angle;
        f32 angleStep;
        s32 modelSetIndex;
        s32 modelIndex;
        s32 modelListCount;

        fn_80026760(lbl_1_bss_F4.modelId, 0.0f,
                        15.0f, -350.0f);
        fn_80026808(lbl_1_bss_F4.modelId, 0.0f,
                        0.0f, 0.0f);
        fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[0],
                           0.0f, 0.0f, 0x40000001U);

        radius = 150.0f;
        angle = 0.0f;
        if (lbl_1_bss_D4.modelIndex < 3) {
            angleStep = 180.0f;
        } else if (lbl_1_bss_D4.modelIndex == 3) {
            angleStep = 120.0f;
        } else if (lbl_1_bss_D4.modelIndex == 4) {
            angleStep = 90.0f;
        } else if (lbl_1_bss_D4.modelIndex == 5) {
            angleStep = 72.0f;
        } else {
            angleStep = 60.0f;
        }

        for (modelSetIndex = 0; modelSetIndex < 20; modelSetIndex++) {
            lbl_1_bss_20[modelSetIndex] = -1;
        }

        modelSetIndex = 0;
        modelListCount = 0;
        for (; modelSetIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode];
             modelSetIndex++) {
            if (lbl_1_bss_D4.playerIndex == lbl_1_bss_128[modelSetIndex].secondarySetIndex) {
                for (modelIndex = 0;
                     modelIndex < lbl_1_bss_128[modelSetIndex].secondaryCount;
                     modelIndex++) {
                    lbl_1_bss_20[modelListCount++] =
                        lbl_1_bss_128[modelSetIndex].models_32[modelIndex];
                    fn_800269E8(
                        lbl_1_bss_128[modelSetIndex].models_32[modelIndex], 1);
                    fn_800267CC(
                        lbl_1_bss_128[modelSetIndex].models_32[modelIndex],
                        &position);
                    position.x = (f32)(radius *
                                       sin((M_PI *
                                            (f64)(30.0f + angle)) /
                                           180.0));
                    position.z = (f32)(-350.0 +
                                       (radius *
                                        cos((M_PI *
                                             (f64)(30.0f + angle)) /
                                            180.0)));
                    fn_80026790(
                        lbl_1_bss_128[modelSetIndex].models_32[modelIndex],
                        &position);
                    angle += angleStep;
                    if (angle >= 360.0f) {
                        angle -= 330.0f;
                        radius += 100.0f;
                    }
                }
            }
        }
        return;
    }

    case 2:
        fn_80026760(lbl_1_bss_F4.modelId, -800.0f,
                        15.0f, -350.0f);
        fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[1],
                           0.0f, 0.0f, 0x40000001U);
        break;
    }
}

extern "C++" void fn_1_7754(void) {
    MicquizPlayerWork *player;
    Point3d modelTransform;
    f32 motionTime;

    player = &lbl_1_bss_128[lbl_1_bss_D4.playerSlot];
    {
    s32 soundList[11] = { 585, 561, 705, 801, 753, 489, 777, 513, 729, 657, 537 };

    switch (lbl_1_bss_D4.phase) {
    case 0:
        switch (lbl_1_bss_D4.motionStep) {
        case 0:
            fn_8002D86C(player->modelId, 1.0f);
            lbl_1_bss_D4.motionStep++;
            break;
        case 1:
            fn_1_834C();
            lbl_1_bss_D4.motionStep++;
            break;
        case 2: {
            s32 animationIndex;
            if (WipeCheck() == 0) {
                animationIndex = 0;
                while (animationIndex <
                       lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
                    HuSprAnimKill(
                        lbl_1_bss_54.spriteAnimations[animationIndex]);
                    animationIndex++;
                }
                HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                if (lbl_1_bss_31E != -1) {
                    HuWinExClose(lbl_1_bss_31E);
                    lbl_1_bss_31E = -1;
                }
                HuMCListenerKill();
                HuMCClose();
                fn_8003A5E4(1, 1);
            }
            break;
        }
        }
        break;
    case 1:
        switch (lbl_1_bss_D4.motionStep) {
        case 0:
            lbl_1_bss_D4.motionStep++;
            break;
        case 1:
            fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[4],
                               0.0f, 8.0f, 0);
            HuAudFXPlay(soundList[lbl_1_bss_F4.charNo]);
            lbl_1_bss_D4.motionStep++;
            break;
        case 2: {
            motionTime = fn_8005C1B8(lbl_1_bss_F4.charNo);
            if (fn_8005C1FC(lbl_1_bss_F4.charNo) <= motionTime) {
                lbl_1_bss_D4.motionStep++;
            }
            break;
        }
        case 3:
            lbl_1_bss_D4.sequenceValue += 1.0f;
            if (lbl_1_bss_D4.sequenceValue >= 10.0f) {
                fn_80026A44(
                    lbl_1_bss_20[(s16)lbl_1_bss_D4.sequenceOffset], 1);
                lbl_1_bss_D4.sequenceValue = 0.0f;
                lbl_1_bss_D4.sequenceOffset += 1.0f;
                if (lbl_1_bss_D4.sequenceOffset >=
                    (f32)lbl_1_bss_D4.modelIndex) {
                    lbl_1_bss_D4.motionStep++;
                }
            }
            break;
        case 4:
            lbl_1_bss_D4.motionStep++;
            break;
        case 5:
            if (lbl_1_bss_D4.timer++ > 0x1E) {
                fn_1_834C();
                lbl_1_bss_D4.motionStep++;
            }
            break;
        case 6: {
            s32 exitAnimationIndex;
            if (WipeCheck() == 0) {
                exitAnimationIndex = 0;
                while (exitAnimationIndex <
                       lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
                    HuSprAnimKill(
                        lbl_1_bss_54.spriteAnimations[exitAnimationIndex]);
                    exitAnimationIndex++;
                }
                HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                if (lbl_1_bss_31E != -1) {
                    HuWinExClose(lbl_1_bss_31E);
                    lbl_1_bss_31E = -1;
                }
                HuMCListenerKill();
                HuMCClose();
                fn_8003A5E4(1, 1);
            }
            break;
        }
        }
        break;
    case 2:
        switch (lbl_1_bss_D4.motionStep) {
        case 0:
            lbl_1_bss_D4.motionStep++;
            break;
        case 1: {
            fn_800267CC(lbl_1_bss_F4.modelId, &modelTransform);
            modelTransform.x += 3.6f;
            if (modelTransform.x >= 0.0f) {
                modelTransform.x = 0.0f;
                lbl_1_bss_D4.motionStep++;
            }
            fn_80026790(lbl_1_bss_F4.modelId, &modelTransform);
            break;
        }
        case 2: {
            fn_80026874(lbl_1_bss_F4.modelId, &modelTransform);
            modelTransform.y -= 5.0f;
            if (modelTransform.y <= 0.0f) {
                fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[0],
                                   0.0f, 8.0f,
                                   0x40000001U);
                modelTransform.y = 0.0f;
                lbl_1_bss_D4.motionStep++;
            }
            fn_80026838(lbl_1_bss_F4.modelId, &modelTransform);
            break;
        }
        case 3:
            if (lbl_1_bss_D4.timer++ > 0x1E) {
                fn_8005C2C8(lbl_1_bss_F4.charNo, lbl_1_bss_F4.motionIds[4],
                                   0.0f, 8.0f, 0);
                lbl_1_bss_D4.motionStep++;
            }
            break;
        case 4: {
            motionTime = fn_8005C1B8(lbl_1_bss_F4.charNo);
            if (fn_8005C1FC(lbl_1_bss_F4.charNo) <= motionTime) {
                fn_1_834C();
                lbl_1_bss_D4.motionStep++;
            }
            break;
        }
        case 5: {
            s32 shutdownAnimationIndex;
            if (WipeCheck() == 0) {
                shutdownAnimationIndex = 0;
                while (shutdownAnimationIndex <
                       lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
                    HuSprAnimKill(
                        lbl_1_bss_54.spriteAnimations[shutdownAnimationIndex]);
                    shutdownAnimationIndex++;
                }
                HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
                if (lbl_1_bss_31E != -1) {
                    HuWinExClose(lbl_1_bss_31E);
                    lbl_1_bss_31E = -1;
                }
                HuMCListenerKill();
                HuMCClose();
                fn_8003A5E4(1, 1);
            }
            break;
        }
        }
        break;
    }
    }
}

extern "C++" s32 fn_1_7FAC(void) {
    Point3d position;
    s16 frameLimit;
    s32 index;

    frameLimit = (s16)(0xB4 / lbl_1_data_3FC[lbl_1_bss_10A.mode]);
    if (lbl_1_bss_54.frame++ == 0) {
        lbl_1_bss_54.states[lbl_1_bss_54.currentModel] = 1;
        fn_80026A44(lbl_1_bss_54.modelIds[lbl_1_bss_54.currentModel], 1);
    }
    if (lbl_1_bss_54.frame >= frameLimit) {
        lbl_1_bss_54.frame = 0;
        lbl_1_bss_54.currentModel += 1;
        if (lbl_1_bss_54.currentModel >=
            lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
            return 1;
        }
    }
    for (index = 0; index < lbl_1_data_3FC[lbl_1_bss_10A.mode]; index++) {
        if (lbl_1_bss_54.states[index] == 1) {
            fn_800267CC(lbl_1_bss_54.modelIds[index], &position);
            position.y -= (f32)(lbl_1_bss_54.frame * 4);
            if (position.y <= 30.0f) {
                position.y = 30.0f;
                lbl_1_bss_54.states[index] = 2;
            }
            fn_80026790(lbl_1_bss_54.modelIds[index], &position);
        }
    }
    return 0;
}

extern "C++" void fn_1_8200(void) {
    s32 modelIndex;

    modelIndex = 0;
    while (modelIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
        fn_800269E8(lbl_1_bss_54.modelIds[modelIndex], 1U);
        modelIndex += 1;
    }
    fn_800258A8(2, 8);
}

extern "C++" void fn_1_8280(s32 modelIndex) {
    lbl_1_bss_54.playerStates[modelIndex] = 1;
}

extern "C++" void fn_1_829C(s16 layerNo) {
    s32 modelIndex;

    modelIndex = 0;
    while (modelIndex < lbl_1_data_3FC[lbl_1_bss_10A.mode]) {
        if (lbl_1_bss_54.playerStates[modelIndex] == 0) {
            fn_8002BCDC(0, 0, 0xC0, 0xA4, GX_TF_RGBA8, 0, lbl_1_bss_54.spriteAnimations[modelIndex]->bmp->data);
        }
        modelIndex += 1;
    }
    fn_8002BF48();
}

extern "C++" void fn_1_834C(void) {
    HuPrcSleep(0x3C);
    HuAudSStreamFadeOut(lbl_1_bss_1C, 0x64);
    WipeCreate(2, 0, 0x1E);
}
HUPROCESS *lbl_1_bss_0;
s32 lbl_1_bss_4;
MicquizResponse *lbl_1_bss_8;
HUPROCESS *lbl_1_bss_C;
HUPROCESS *lbl_1_bss_10;
ANIMDATA *lbl_1_bss_14;
f32 lbl_1_bss_18;
s32 lbl_1_bss_1C;
s16 lbl_1_bss_20[0x14];
s16 lbl_1_bss_48[6];
MicquizAnimationState lbl_1_bss_54;
MicquizAccessoryPoolEntry lbl_1_bss_98[6];
MicquizSequence lbl_1_bss_D4;
MicquizHostState lbl_1_bss_F4;
MicquizControlWork lbl_1_bss_10A;
MicquizPlayerWork lbl_1_bss_128[5];
HUPROCESS *lbl_1_bss_2A4[5];
HUPROCESS *lbl_1_bss_2B8[11];
HUPROCESS *lbl_1_bss_2E4[0xA];
HU3D_MOTIONID lbl_1_bss_30C[7];
HU3D_MODELID lbl_1_bss_31A;
s16 lbl_1_bss_31C;
s16 lbl_1_bss_31E;
s16 lbl_1_bss_320;
s16 lbl_1_bss_322;
s16 lbl_1_bss_324;
s16 lbl_1_bss_326;
s16 lbl_1_bss_328;
s16 lbl_1_bss_32A;
u8 lbl_1_bss_32C[0x4];
s16 lbl_1_bss_330;
s16 lbl_1_bss_332;
s16 lbl_1_bss_334;
ANIMDATA *lbl_1_bss_338;
ANIMDATA *lbl_1_bss_33C;
s32 lbl_1_bss_340;
s32 lbl_1_bss_344;
HUPROCESS *lbl_1_bss_348;
s16 lbl_1_bss_34C;
s16 lbl_1_bss_34E;
MGSCORE *lbl_1_bss_350[2];
s16 lbl_1_bss_358[4];
ANIMDATA *lbl_1_bss_360[4];
s16 lbl_1_bss_370;
ANIMDATA *lbl_1_bss_374;
s16 lbl_1_bss_378[4][3];
ANIMDATA *lbl_1_bss_390[4][3];
s16 lbl_1_bss_3C0[4];
ANIMDATA *lbl_1_bss_3C8[4];
s16 lbl_1_bss_3D8;
ANIMDATA *lbl_1_bss_3DC;
s32 lbl_1_bss_3E0;
ANIMDATA *lbl_1_bss_3E4;
ANIMDATA *lbl_1_bss_3E8;
ANIMDATA *lbl_1_bss_3EC;
ANIMDATA *lbl_1_bss_3F0;
ANIMDATA *lbl_1_bss_3F4[4];
HUSPR_GROUPID lbl_1_bss_404[1];
u8 lbl_1_bss_406[0x4];
s16 lbl_1_bss_40A;
s16 lbl_1_bss_40C;
s16 lbl_1_bss_40E;
s16 lbl_1_bss_410;
s16 lbl_1_bss_412;
s16 lbl_1_bss_414;
s16 lbl_1_bss_416;
s16 lbl_1_bss_418;
u8 lbl_1_bss_41A[0x8];
s16 lbl_1_bss_422;
ANIMDATA *lbl_1_bss_424[2];
ANIMDATA *lbl_1_bss_42C;
ANIMDATA *lbl_1_bss_430;
ANIMDATA *lbl_1_bss_434;
ANIMDATA *lbl_1_bss_438;
ANIMDATA *lbl_1_bss_43C;
ANIMDATA *lbl_1_bss_440;
ANIMDATA *lbl_1_bss_444;
ANIMDATA *lbl_1_bss_448;
s16 lbl_1_bss_44C;
ANIMDATA *lbl_1_bss_450;
ANIMDATA *lbl_1_bss_454;
ANIMDATA *lbl_1_bss_458;
ANIMDATA *lbl_1_bss_45C;
ANIMDATA *lbl_1_bss_460;
ANIMDATA *lbl_1_bss_464;
ANIMDATA *lbl_1_bss_468;
ANIMDATA *lbl_1_bss_46C;
ANIMDATA *lbl_1_bss_470[2];
ANIMDATA *lbl_1_bss_478;
MicquizModeWork *lbl_1_bss_47C;
s32 lbl_1_bss_480;
f32 lbl_1_bss_484;
f32 lbl_1_bss_488;
s32 lbl_1_bss_48C;
s16 lbl_1_bss_490;
u8 lbl_1_bss_492[0xA];
s32 lbl_1_bss_49C;
s32 lbl_1_bss_4A0;
s32 lbl_1_bss_4A4;
s16 lbl_1_bss_4A8;
s32 lbl_1_bss_4AC;
s16 lbl_1_bss_4B0;
s32 lbl_1_bss_4B4;
s32 lbl_1_bss_4B8;
s32 lbl_1_bss_4BC;
s32 lbl_1_bss_4C0;
s32 lbl_1_bss_4C4;
s32 lbl_1_bss_4C8;
s32 lbl_1_bss_4CC;
s32 lbl_1_bss_4D0;
s32 lbl_1_bss_4D4;
s32 lbl_1_bss_4D8;
s32 lbl_1_bss_4DC;
s32 lbl_1_bss_4E0;
s32 lbl_1_bss_4E4;
s32 lbl_1_bss_4E8;
HUPROCESS *lbl_1_bss_4EC;
MicquizResponseEntry lbl_1_bss_4F0[8];
s16 lbl_1_bss_570;
u8 lbl_1_bss_572[0xA];
