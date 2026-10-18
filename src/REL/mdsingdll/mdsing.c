/* Solo Mode character selection, board setup and minigame results. */
#include "math.h"
#include <string.h>
#include "dolphin/os.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm.h"
#include "msm_se.h"
#include "msm_stream.h"
#include "humath.h"
#include "game/armem.h"
/* Legacy Solo calls omit the streamed-fade and mode-save prototypes. */
#if !defined(__MWERKS__)
#include "game/audio.h"
#include "game/saveload.h"
#else
int HuAudFXPlay(int seId);
int HuAudFXPlayPan(int seId, s16 pan);
void HuAudFXStop(int seNo);
int HuAudSStreamPlay(s16 streamId);
int HuAudJinglePlay(s16 streamId);
s32 CharFXPlay(s16 charNo, s16 seId);
BOOL SLSaveFlagGet(void);
s32 SLSaveCheck(void);
#endif
#include "game/board/main.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/frand.h"
#include "game/gamemes.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/main.h"
#include "game/memory.h"
#include "game/mgdata.h"
#include "game/mg/seqman.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/printfunc.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/window.h"
#include "REL/mdsingdll/object_state.h"
#include "REL/mdsingdll/settings.h"
#include "REL/mdsingdll/selection.h"
#include "REL/mdsingdll/camera.h"
#include "REL/mdsingdll/model.h"
#include "REL/mdsingdll/motion_path.h"
#include "REL/mdsingdll/score.h"
#include "REL/mdsingdll/falling_models.h"
#include "REL/mdsingdll/menu_data.h"
#define sind(x) sin((M_PI * (x)) / 180.0)
#define cosd(x) cos((M_PI * (x)) / 180.0)

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

typedef struct MdsingAudioTable {
    /* Guide sound IDs in one-based speaker order; -1 marks an unused speaker. */
    s32 effectId[16];
} MDSING_AUDIO_TABLE;

typedef struct MdsingDirectoryPair {
    /* Packed directory IDs copied into ARAM before a board starts. */
    u32 archiveId[2];
} MdsingDirectoryPair;

typedef struct MdsingMotionSoundFrameTable {
    /* Four animation-frame entries per character; -1 disables a sound entry. */
    s8 frames[56];
} MDSING_MOTION_SOUND_FRAME_TABLE;

typedef struct {
    /* Three direction priorities for each of the twelve navigation commands. */
    s16 directionOrder[12][3];
} MDMinNeighborTable;

typedef struct BitmapNameTable {
    /* Texture object names receiving the menu model's four animations. */
    char *name[4];
} BITMAP_NAME_TABLE;

typedef MDSING_MODEL LBL_1_BSS_E74_ENTRY;

typedef struct {
    /* Model travel endpoints in world coordinates. */
    HuVecF start;
    HuVecF end;
} MDSING_RESULT_POSITIONS;

void fn_1_0(HUWINID winId, u32 messageNo, s16 speakerNo);
void fn_1_1A4(void);
void fn_1_250(void);
void fn_1_3A0(void);
void fn_1_5E8(void);
void fn_1_6B4(void);
void fn_1_B34(void);
void fn_1_EA0(void);
void fn_1_FEC(void);
void fn_1_10D0(void);
void fn_1_1180(void);
float fn_1_1230(float start, float middle, float end, float time);
void fn_1_128C(HuVecF *dst, const HuVecF *start, const HuVecF *middle,
    const HuVecF *end, float time);
float fn_1_1494(float current, float target, float weight);
void fn_1_14C4(HuVecF *current, const HuVecF *target, float weight);
float fn_1_1608(float startValue, float endValue, float time, float duration);
float fn_1_16F0(float startValue, float endValue, float elapsedFrames, float durationFrames);
void fn_1_1734(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration);
void fn_1_1A9C(HU3D_MODELID modelId, float rotY);
void fn_1_1BDC(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY);
float fn_1_205C(float start, float end, float time, float duration);
float fn_1_2154(float startValue, float peakValue, float elapsedFrames, float durationFrames);
void fn_1_2228(
    HU3D_MODELID modelId, const HuVecF *start, const HuVecF *control, const HuVecF *end,
    float time, float duration);
void fn_1_267C(void);
void fn_1_26FC(MDSING_CAMERA_WORK *work);
void fn_1_274C(MDMinidllCameraState *work);
void fn_1_279C(MDSING_CAMERA_WORK *camera, float weight);
void fn_1_2A50(MDCAMERA_CALLBACK callback);
void fn_1_2A60(OMOBJ *obj, MDCAMERA_WORK *camera);
void fn_1_3170(OMOBJ *obj, MDMinidllCameraState *state);
void fn_1_31BC(OMOBJ *obj);
void fn_1_328C(MDSING_CAMERA_CALLBACK callback);
void fn_1_3428(void);
void fn_1_3484(void);
void fn_1_35B0(void);
void fn_1_35EC(s16 winNo);
void fn_1_365C(s16 winNo);
void fn_1_36CC(s16 winNo);
s16 fn_1_3708(s16 winNo, s16 mode);
void fn_1_37DC(s16 winNo, s32 messNum, s16 speed);
void fn_1_3898(s16 winNo, s32 messNum, s16 insertPos);
void fn_1_3908(void);
void fn_1_3B30(void);
void fn_1_3B8C(s16 winNo);
void fn_1_3CF8(void);
void fn_1_3DB4(void);
s16 fn_1_3E14(s16 mode);
void fn_1_3F0C(s16 winNo, s32 messNum, s16 speed);
void fn_1_4144(s16 winNo, s32 messNum, s16 insertPos);
void fn_1_4338(s32 messNum);
void fn_1_44E4(s32 messNum);
void fn_1_4690(void);
void fn_1_473C(void);
void fn_1_47F0(void);
void fn_1_47F4(HUSPR_GROUPID groupId, s32 attr);
void fn_1_4874(HUSPR_GROUPID groupId, s32 attr);
void fn_1_48F4(void);
void fn_1_4B40(void);
void fn_1_4B44(OMOBJ *obj);
void fn_1_4F94(OMOBJ *obj);
void fn_1_5098(void);
void fn_1_514C(OMOBJ *obj);
void fn_1_51F4(void);
void fn_1_5268(OMOBJ *obj);
void fn_1_57E4(void);
void fn_1_595C(OMOBJ *obj);
s32 fn_1_5EF4(void);
void fn_1_5FA0(OMOBJ *object);
void fn_1_62B4(OMOBJ *object);
void fn_1_633C(OMOBJ *obj);
void fn_1_68C0(void);
void fn_1_6A7C(OMOBJ *obj);
void fn_1_6C48(void);
void fn_1_6E94(OMOBJ *obj);
void fn_1_7134(OMOBJ *obj);
void fn_1_73D4(OMOBJ *obj);
void fn_1_7484(OMOBJ *obj);
void fn_1_7668(void);
void fn_1_778C(void);
void fn_1_77C0(OMOBJ *obj);
void fn_1_79D0(OMOBJ *obj);
void fn_1_7A60(s16 memberNo);
void fn_1_7B64(void);
void fn_1_7B94(void);
void fn_1_7BF4(OMOBJ *obj);
void fn_1_7E3C(s16 index);
void fn_1_7EE0(void);
void fn_1_7F34(OMOBJ *obj);
void fn_1_842C(OMOBJ *obj);
void fn_1_8670(OMOBJ *obj);
void fn_1_876C(s16 playerNo, s16 bank);
void fn_1_8858(s16 teamPlayerNo);
void fn_1_8880(OMOBJ *obj);
void fn_1_8A80(OMOBJ *obj);
void fn_1_8B24(void);
void fn_1_8B28(
    s16 memberNo, HuVecF *worldPosition, float xOffset, float yOffset);
void fn_1_8BC0(
    s16 memberNo, HuVecF *worldPosition, float xOffset, float yOffset);
void fn_1_8CE4(s16 memberNo);
void fn_1_8D40(s16 memberNo);
void fn_1_8DB4(void);
void fn_1_8ED8(void);
void fn_1_8F64(s16 modelNo, Vec *position);
void fn_1_8FEC(s32 modelNo, HuVecF *position, s16 bank);
void fn_1_91C0(s32 modelNo);
void fn_1_9258(OMOBJ *obj);
void fn_1_9718(OMOBJ *obj);
void fn_1_9910(OMOBJ *obj);
s16 fn_1_9998(s16 currentCharacter, s16 direction, s16 fallbackDirection, s16 lastDirection,
              MDSING_CHARACTER_NEIGHBORS *entries, s16 characterCount);
s16 fn_1_9D80(s16 currentCharacter, MDSING_CHARACTER_NEIGHBORS *entries,
              s16 navigationCommand, s16 characterCount);
s16 fn_1_9E28(s16 padNo, s16 *currentCharacter,
    MDSING_CHARACTER_NEIGHBORS *entries, s16 characterCount);
void fn_1_A3A8(s16 modelNo, s16 animNo, s16 dataNo, s16 bank);
void fn_1_A450(s16 modelNo, s16 dataNo);
void fn_1_A4DC(
    s16 modelNo, s16 elapsedFrames, s16 durationFrames, HuVecF *position, float scale);
void fn_1_A8E8(s16 modelNo, s16 elapsedFrames, s16 durationFrames);
void fn_1_ACB8(
    s16 modelNo, s16 elapsedFrames, s16 durationFrames, HuVecF *targetPosition, float scale);
void fn_1_B2A8(s16 modelNo, s16 dataNo, s16 dataOffset, s16 elapsedFrames,
    s16 durationFrames, s16 direction);
void fn_1_B630(s16 modelNo, float referenceAngle, s16 elapsedFrames, s16 durationFrames);
void fn_1_B898(s16 completionState);
s16 fn_1_B8B0(void);
s16 fn_1_B8C8(void);
void fn_1_B900(OMOBJ_FUNC callback);
void fn_1_B998(void);
void fn_1_BBC8(void);
void fn_1_BCA4(s16 modelNo);
void fn_1_BD40(s16 modelNo);
void fn_1_BDA8(OMOBJ *obj);
void fn_1_C264(OMOBJ *obj);
void fn_1_C4C8(OMOBJ *obj);
void fn_1_C614(OMOBJ *obj);
void fn_1_C724(OMOBJ *obj);
void fn_1_CB8C(OMOBJ *obj);
void fn_1_CE2C(OMOBJ *obj);
void fn_1_CF94(OMOBJ *obj);
void fn_1_D57C(OMOBJ *obj);
void fn_1_DE98(OMOBJ *obj);
void fn_1_E290(OMOBJ *obj);
void fn_1_E688(s16 modelNo);
void fn_1_E9A8(s16 boardNo);
void fn_1_EA7C(OMOBJ *obj);
void fn_1_F15C(OMOBJ *obj);
void fn_1_F3C8(OMOBJ *obj);
void fn_1_F6D0(OMOBJ *obj);
void fn_1_FCC8(OMOBJ *obj);
void fn_1_FD5C(OMOBJ *obj);
void fn_1_10744(OMOBJ *obj);
void fn_1_10B18(OMOBJ *object);
void fn_1_119E8(OMOBJ *obj);
void fn_1_1241C(OMOBJ *obj);
void fn_1_12E68(OMOBJ *object);
void fn_1_13D38(OMOBJ *object);
void fn_1_14DA8(OMOBJ *obj);
void fn_1_15198(OMOBJ *obj);
void fn_1_15280(OMOBJ *obj);
void fn_1_15AC4(OMOBJ *obj);
void fn_1_164A8(OMOBJ *obj);
void fn_1_16E70(OMOBJ *obj);
void fn_1_1750C(void);
void fn_1_1791C(OMOBJ *obj);
void fn_1_17958(OMOBJ *obj);
void fn_1_179B0(void);
void fn_1_17D38(void);
void fn_1_188C0(void);
void fn_1_18A18(void);
void fn_1_18A98(void);
void fn_1_18AB8(void);
void fn_1_18DB0(OMOBJ *obj, MDCAMERA_WORK *camera);
void fn_1_19114(OMOBJ *obj, MDMinidllCameraState *state);
void fn_1_1933C(OMOBJ *obj, MDMinidllCameraState *state);
void fn_1_193B0(void);
s32 fn_1_19570(void);
s32 fn_1_19748(void);
s16 fn_1_19A20(void);
s16 fn_1_19D28(void);
s32 fn_1_1A494(void);
s32 fn_1_1A610(void);
s16 fn_1_1AA1C(s16 waitForOpeningMotion);
s16 fn_1_1B5EC(s16 unused);
s16 fn_1_1C138(s16 returningFromPreviousStep);
s16 fn_1_1C3A8(s16 waitForOpeningMotion);
s16 fn_1_1D174(void);
void fn_1_1DCE0(void);
s16 fn_1_1E104(s16 returningToSelection);
void fn_1_1F30C(void);
s16 fn_1_20624(s16 mode);
void fn_1_21940(void);
void fn_1_229D8(void);
void fn_1_22C9C(void);
void fn_1_22CF4(s16 modelNo, const HuVecF *offset, f32 x, f32 y);
void fn_1_22DF4(HUSPR_GROUPID groupId, s32 firstMember, s16 value);
void fn_1_22F48(s16 spriteGroup, s32 firstSprite, s16 value);
void fn_1_23164(s16 index, s16 value);
void fn_1_23498(s16 value);
void fn_1_23874(OMOBJ *unused);
void fn_1_23924(f32 yOffset);
void fn_1_23C24(OMOBJ *obj);
void fn_1_24674(OMOBJ *obj);
void fn_1_246F8(OMOBJ *manager);
void fn_1_24B1C(s16 spawnCount);
void fn_1_24BC8(OMOBJ *obj);
void fn_1_24CC8(OMOBJ *obj);
void fn_1_24D44(OMOBJ *obj);
void fn_1_24FA8(void);
void fn_1_24FDC(OMOBJ *obj);
void fn_1_251D8(void);
void fn_1_2542C(OMOBJ *obj);
void fn_1_25648(OMOBJ *obj);
s32 fn_1_256A8(void);
void fn_1_256C8(void);
void fn_1_25BF4(void);
void fn_1_25D48(void);
void fn_1_25FD4(void);
void fn_1_26DFC(void);
void fn_1_282C0(void);
void fn_1_28568(void);
s32 fn_1_28828(void);
s32 fn_1_28B78(void);
s32 fn_1_2987C(void);
void fn_1_2A4F4(void);
s32 fn_1_2A81C(void);
s32 fn_1_2AD60(void);
s32 fn_1_2C158(void);
float fn_1_2CF38(float startValue, float endValue, float time, float duration);
float fn_1_2D020(float startValue, float endValue, float time, float duration);
float fn_1_2D0FC(float startValue, float endValue, float elapsedFrames, float durationFrames);
float fn_1_2D140(float currentValue, float targetValue, float weight);
void fn_1_2D170(s16 layerNo);
void fn_1_2D1DC(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material);
void fn_1_2D65C(OMOBJ *obj);
void fn_1_2D75C(s16 textureSlot, HuVecF *position);
void fn_1_2D850(OMOBJ *obj);
void fn_1_2DA34(OMOBJ *obj);
void fn_1_2DAE4(void);
void fn_1_2DB34(void);
void fn_1_2DC10(Point3d *position);
void fn_1_2DCD0(s16 layerNo);
void fn_1_2E68C(void);
void fn_1_2E6B8(void);
void fn_1_2E6DC(
    s16 particleNo, float lifetime, float targetScale, HuVecF *position, GXColor color);
void fn_1_2E7DC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_2EA9C(void);
void fn_1_2EBB0(void);
void fn_1_2EBDC(s16 effectNo, float durationFrames, HuVecF *position, GXColor color);
void fn_1_2ECC4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_2F20C(void);
void fn_1_2F3CC(void);
void fn_1_2F424(void);
void fn_1_2F4C4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_2FC50(void);
void fn_1_2FD50(void);
void fn_1_2FD7C(s16 effectNo, float durationFrames, HuVecF *position);
void fn_1_2FE48(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_30730(void);
void fn_1_308C4(void);
void fn_1_3091C(HuVecF *pos, GXColor *color, float velocityY, float accelY);
void fn_1_30A40(float velocityY, float velocityZ, HuVecF *position,
    GXColor *color);
void fn_1_30BE0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_31088(void);
void fn_1_3116C(void);
void fn_1_31198(s16 index, s16 visible);
void fn_1_31214(s16 effectNo, HuVecF *position, GXColor *color);
void fn_1_31318(s16 effectNo);
void fn_1_313A0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_31818(void);
void fn_1_31984(void);
void fn_1_319DC(s16 index, s16 visible);
void fn_1_31A58(s16 effectNo, HuVecF *position, GXColor *color);
void fn_1_31B90(s16 effectNo);
void fn_1_31BE4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_321E4(void);
void fn_1_32350(void);
void fn_1_323A8(s16 modelNo, HuVecF *pos, s16 display);
void fn_1_3276C(s16 groupNo, s16 show);
void fn_1_32820(s16 groupNo, HuVecF *spawnPosition, GXColor *color);
void fn_1_32988(s16 groupNo);
void fn_1_32A08(HU3D_MODEL *model, HU3D_PARTICLE *emitter, Mtx matrix);
void fn_1_32FB4(void);
void fn_1_331E0(void);
void fn_1_33264(HUPROCESS *process);
void fn_1_33D84(void);
void fn_1_33FC0(HuVecF *position);
void fn_1_341B8(s16 particleNo, Point3d *position, s16 colorNo);
void fn_1_344B4(s16 particleNo, HuVecF *position, s16 colorNo, s16 mode);
void fn_1_34958(void);
void fn_1_34A18(HuVecF *position);
void fn_1_34FA4(s16 groupNo, HuVecF *pos, s16 mode, s16 colorNo);

static inline float quadratic_interpolate(float start, float middle, float end, float time);
static inline float fn_1_13A8_inlined(float startValue, float controlValue, float endValue,
                                      float progress);
static inline void fn_1_1404_inlined(
    HuVecF *dst, const HuVecF *start, const HuVecF *control, const HuVecF *end, float progress);
static inline void fn_1_A18(s16 winNo);
static inline void show_message_window(s16 winNo);
static inline void set_message_window(s16 winNo, s32 messNum, s16 speed);
static inline f32 fn_1_4C78_ease90_inlined(f32 start, f32 end, f32 elapsed, f32 duration);
static inline f32 fn_1_4C78_ease180_inlined(f32 start, f32 end, f32 elapsed, f32 duration);
static inline void fn_1_22A8_inlined(
    HU3D_MODELID modelId, const HuVecF *start, const HuVecF *control, const HuVecF *end,
    float time, float duration);
static inline void fn_1_18AC_inlined(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration);
static inline void fn_1_1C14_inlined(HU3D_MODELID modelId, float rotY);
static inline void fn_1_1D54_inlined(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY);
static inline f32 fn_1_10AC_inlined_inlined(f32 start, f32 control, f32 end, f32 progress);
static inline void fn_1_1108_inlined_inlined(Point3d *destination, const Point3d *start,
                                             const Point3d *control, const Point3d *end,
                                             f32 progress);
static inline void fn_1_89E0_inlined(OMOBJ *obj);
static inline void fn_1_AB64_inlined(void);
static inline void fn_1_AC88_inlined_inlined(void);
static inline void fn_1_AF70(s32 modelNo);
static inline void fn_1_122FC(s16 characterNo);
static inline void fn_1_C28C_inlined(
    s16 modelNo, s16 elapsedFrames, s16 durationFrames, HuVecF *position, float scale);
static inline void fn_1_8C28_inlined(s16 animNo);
static inline void fn_1_8950_inlined(void);
static inline void update_entry_display(s16 modelNo, s16 elapsedFrames,
    s16 durationFrames, const Vec *position, f32 scale);
static inline f32 MdsingInterpolate(f32 start, f32 end, f32 elapsedFrames, f32 durationFrames);
static inline void MdsingModelTransition(MDSING_MODEL *entry, s16 elapsedFrames);
static inline void fn_1_8670_inlined(OMOBJ *obj);
static inline void fn_1_3908_inlined(void);
static inline void fn_1_48F4_inlined(void);
static inline void fn_1_B998_inlined(void);
static inline void fn_1_52B0_inlined_inlined(void);
static inline s16 fn_1_381C_inlined_inlined(s16 winNo, s16 mode);
static inline s16 fn_1_3F28_inlined_inlined(s16 mode);
static inline s16 fn_1_3F28_inlined(s16 mode);
static inline void fn_1_64C_inlined(void);
static inline s16 fn_1_2EF24_inlined(void);
static inline s32 fn_1_5B48_inlined_inlined(void);
static inline void fn_1_3474_inlined_inlined(s16 windowNo);
static inline void fn_1_3B10_inlined_inlined(void);
static inline void set_message_window_for_1AA1C(s16 windowNo, s32 messageNo, s16 speed);
static inline void fn_1_4338_for_1AA1C(s32 messageNo);
static inline void fn_1_3B8C_for_1AA1C(s16 windowNo);
static inline void fn_1_3F0C_for_1AA1C(s16 windowNo, s32 messageNo, s16 speed);
static inline void fn_1_35EC_for_1B5EC(s16 windowNo);
static inline void fn_1_4338_for_1B5EC(s32 messageNo);
static inline s16 fn_1_19A20_for_1B5EC(void);
static inline void fn_1_64C_inlined_2(void);
static inline void fn_1_7900_inlined(void);
static inline s16 fn_1_2EF24_inlined_2(void);
static inline s16 mdsingCoinBonusGet(s32 playerNo);
static inline s32 fn_1_1A494_for_20624(void);
static inline void fn_1_5E8_for_20624(void);
static inline void fn_1_1DCE0_for_20624(void);
static inline void set_five_digits(s16 spriteGroup, s32 firstSprite, s16 value);
static inline void fn_1_8BC0_for_26DFC(
    s16 memberNo, Vec *worldPosition, float xOffset, float yOffset);
static inline void mdsingResultPanelOpen(void);
static inline void show_result_window(s16 winNo);
static inline void fn_1_44E4_for_28B78(s32 messageNo);
static inline void fn_1_24B1C_for_2987C(s16 count);
static inline void fn_1_24FA8_for_2987C(void);
static inline void mdsingVariantClose(s16 winNo);
static inline void mdsingVariantOpen(s16 winNo);
static inline void mdsingVariantSelect(s16 winNo);
static inline void mdsingVariantSet(s16 winNo, s32 messageNo, s16 speed);
static inline void mdsingVariantMessage(s16 winNo, s32 messageNo, s16 speed);
static inline s16 mdsingVariantChoice(s16 winNo, s16 mode);
static inline s16 mdsingVariantGetChoice(s16 mode);
static inline void mdsingVariantWait(s16 winNo);
static inline void mdsingVariantWaitMessage(void);

/* Prize descriptions and the amounts used to build the post-board reward list. */
MDSING_PRIZE_ROW lbl_1_data_0[50] = {
    { -1, 10, MESSNUM(MESS_OPTION_SINGLE, 116), -1, 10},
    { -1, 11, MESSNUM(MESS_OPTION_SINGLE, 115), -1, 0},
    { -1, 1, MESSNUM(MESS_OPTION_SINGLE, 104), -1, -1},
    { 1, 12, MESSNUM(MESS_OPTION_SINGLE, 0), MESSNUM(MESS_OPTION_SINGLE, 51), 10},
    { 2, 13, MESSNUM(MESS_OPTION_SINGLE, 1), MESSNUM(MESS_OPTION_SINGLE, 52), 15},
    { 3, 14, MESSNUM(MESS_OPTION_SINGLE, 2), MESSNUM(MESS_OPTION_SINGLE, 53), 20},
    { 4, 15, MESSNUM(MESS_OPTION_SINGLE, 3), MESSNUM(MESS_OPTION_SINGLE, 54), 25},
    { 5, 18, MESSNUM(MESS_OPTION_SINGLE, 4), MESSNUM(MESS_OPTION_SINGLE, 55), 30},
    { 7, 0, MESSNUM(MESS_OPTION_SINGLE, 6), MESSNUM(MESS_OPTION_SINGLE, 57), 50},
    { 8, 0, MESSNUM(MESS_OPTION_SINGLE, 7), MESSNUM(MESS_OPTION_SINGLE, 58), 100},
    { 9, 0, MESSNUM(MESS_OPTION_SINGLE, 8), MESSNUM(MESS_OPTION_SINGLE, 59), 100},
    { 10, 0, MESSNUM(MESS_OPTION_SINGLE, 9), MESSNUM(MESS_OPTION_SINGLE, 60), 100},
    { -1, 1, MESSNUM(MESS_OPTION_SINGLE, 105), -1, -1},
    { 11, 0, MESSNUM(MESS_OPTION_SINGLE, 10), MESSNUM(MESS_OPTION_SINGLE, 61), 20},
    { 12, 0, MESSNUM(MESS_OPTION_SINGLE, 11), MESSNUM(MESS_OPTION_SINGLE, 62), 30},
    { 13, 0, MESSNUM(MESS_OPTION_SINGLE, 12), MESSNUM(MESS_OPTION_SINGLE, 63), 10},
    { 14, 0, MESSNUM(MESS_OPTION_SINGLE, 13), MESSNUM(MESS_OPTION_SINGLE, 64), 10},
    { 15, 0, MESSNUM(MESS_OPTION_SINGLE, 14), MESSNUM(MESS_OPTION_SINGLE, 65), 30},
    { 16, 0, MESSNUM(MESS_OPTION_SINGLE, 15), MESSNUM(MESS_OPTION_SINGLE, 66), 30},
    { 17, 0, MESSNUM(MESS_OPTION_SINGLE, 16), MESSNUM(MESS_OPTION_SINGLE, 67), 10},
    { 18, 0, MESSNUM(MESS_OPTION_SINGLE, 17), MESSNUM(MESS_OPTION_SINGLE, 68), 5},
    { 19, 0, MESSNUM(MESS_OPTION_SINGLE, 18), MESSNUM(MESS_OPTION_SINGLE, 69), 50},
    { 20, 0, MESSNUM(MESS_OPTION_SINGLE, 19), MESSNUM(MESS_OPTION_SINGLE, 70), 100},
    { -1, 1, MESSNUM(MESS_OPTION_SINGLE, 106), -1, -1},
    { 21, 0, MESSNUM(MESS_OPTION_SINGLE, 20), MESSNUM(MESS_OPTION_SINGLE, 71), 10},
    { 22, 0, MESSNUM(MESS_OPTION_SINGLE, 21), MESSNUM(MESS_OPTION_SINGLE, 72), 30},
    { 23, 0, MESSNUM(MESS_OPTION_SINGLE, 22), MESSNUM(MESS_OPTION_SINGLE, 73), 10},
    { 24, 0, MESSNUM(MESS_OPTION_SINGLE, 23), MESSNUM(MESS_OPTION_SINGLE, 74), 30},
    { 25, 0, MESSNUM(MESS_OPTION_SINGLE, 24), MESSNUM(MESS_OPTION_SINGLE, 75), 20},
    { 26, 0, MESSNUM(MESS_OPTION_SINGLE, 25), MESSNUM(MESS_OPTION_SINGLE, 76), 30},
    { 27, 0, MESSNUM(MESS_OPTION_SINGLE, 26), MESSNUM(MESS_OPTION_SINGLE, 77), 20},
    { 28, 0, MESSNUM(MESS_OPTION_SINGLE, 27), MESSNUM(MESS_OPTION_SINGLE, 78), 10},
    { 29, 0, MESSNUM(MESS_OPTION_SINGLE, 28), MESSNUM(MESS_OPTION_SINGLE, 79), 20},
    { 30, 0, MESSNUM(MESS_OPTION_SINGLE, 29), MESSNUM(MESS_OPTION_SINGLE, 80), 20},
    { -1, 1, MESSNUM(MESS_OPTION_SINGLE, 107), -1, -1},
    { 31, 16, MESSNUM(MESS_OPTION_SINGLE, 30), MESSNUM(MESS_OPTION_SINGLE, 81), 10},
    { 32, 17, MESSNUM(MESS_OPTION_SINGLE, 31), MESSNUM(MESS_OPTION_SINGLE, 82), 10},
    { 33, 0, MESSNUM(MESS_OPTION_SINGLE, 32), MESSNUM(MESS_OPTION_SINGLE, 83), 15},
    { 34, 0, MESSNUM(MESS_OPTION_SINGLE, 33), MESSNUM(MESS_OPTION_SINGLE, 84), 15},
    { 35, 0, MESSNUM(MESS_OPTION_SINGLE, 34), MESSNUM(MESS_OPTION_SINGLE, 85), 15},
    { 36, 0, MESSNUM(MESS_OPTION_SINGLE, 35), MESSNUM(MESS_OPTION_SINGLE, 86), 30},
    { 37, 0, MESSNUM(MESS_OPTION_SINGLE, 36), MESSNUM(MESS_OPTION_SINGLE, 87), 30},
    { 38, 0, MESSNUM(MESS_OPTION_SINGLE, 37), MESSNUM(MESS_OPTION_SINGLE, 88), 50},
    { 39, 0, MESSNUM(MESS_OPTION_SINGLE, 38), MESSNUM(MESS_OPTION_SINGLE, 89), 50},
    { 40, 0, MESSNUM(MESS_OPTION_SINGLE, 39), MESSNUM(MESS_OPTION_SINGLE, 90), 300},
    { -1, 1, MESSNUM(MESS_OPTION_SINGLE, 108), -1, -1},
    { 48, 0, MESSNUM(MESS_OPTION_SINGLE, 47), MESSNUM(MESS_OPTION_SINGLE, 98), 50},
    { 49, 0, MESSNUM(MESS_OPTION_SINGLE, 48), MESSNUM(MESS_OPTION_SINGLE, 99), 100},
    { 50, 0, MESSNUM(MESS_OPTION_SINGLE, 49), MESSNUM(MESS_OPTION_SINGLE, 100), 300},
    { -1, -1, -1, -1, -1}
};

/* Music handles for the three entry-specific tracks; -1 is inactive. */
s32 lbl_1_data_320[3] = { -1, -1, -1 };

/* Sprite animation resources, group capacities, and model-card texture resources. */
u32 lbl_1_data_32C[25] = {
    DATANUM(DATA_mdsing, 16), DATANUM(DATA_mdsing, 4),
    DATANUM(DATA_mdsing, 5), DATANUM(DATA_mdsing, 6),
    DATANUM(DATA_mdsing, 7), DATANUM(DATA_mdsing, 8),
    DATANUM(DATA_mdsing, 9), DATANUM(DATA_mdsing, 10),
    DATANUM(DATA_mdsing, 11), DATANUM(DATA_mdsing, 18),
    DATANUM(DATA_mdsing, 19), DATANUM(DATA_mdsing, 20),
    DATANUM(DATA_mdsing, 15), DATANUM(DATA_mdsing, 13),
    DATANUM(DATA_mdsing, 12), DATANUM(DATA_mdsing, 17),
    DATANUM(DATA_mdsing, 139), DATANUM(DATA_mdsing, 140),
    DATANUM(DATA_mdsing, 141), DATANUM(DATA_mdsing, 145),
    DATANUM(DATA_mdsing, 146), DATANUM(DATA_mdsing, 149),
    DATANUM(DATA_mdsing, 148), DATANUM(DATA_mdsing, 147),
    DATANUM(DATA_mdsing, 151)
};

s16 lbl_1_data_390[10] = { 2, 3, 6, 4, 2, 2, 6, 3, 1, 0 };

/* Sprite layout and world-space positions used by the selection screens. */
MDSING_SPRITE_DESC lbl_1_data_3A4[29] = {
    { 0, 0, 0, 10, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 0, 9, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 1, 10, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 2, 11, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 0, 12, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 1, 12, 0, 2, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 2, 12, 0, 5, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 3, 12, 0, 7, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 4, 12, 0, 4, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 5, 12, 0, 6, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 3, 0, 13, 0, 0, { 0.0f, 0.0f }, { 0.8f, 0.8f }, 0.0f },
    { 3, 1, 13, 0, 0, { 0.0f, 0.0f }, { 0.8f, 0.8f }, 0.0f },
    { 3, 2, 13, 0, 0, { 0.0f, 0.0f }, { 0.8f, 0.8f }, 0.0f },
    { 3, 3, 13, 0, 0, { 0.0f, 0.0f }, { 0.8f, 0.8f }, 0.0f },
    { 4, 0, 13, 0, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 4, 1, 14, 10, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 5, 0, 13, 0, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 5, 1, 14, 10, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 0, 1, 15, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 6, 0, 16, 8, 0, { 0.0f, 8.0f }, { 1.0f, 1.0f }, 0.0f },
    { 6, 1, 17, 6, 0, { 0.0f, -140.0f }, { 1.0f, 1.0f }, 0.0f },
    { 6, 2, 17, 6, 0, { 0.0f, 156.0f }, { 1.0f, 1.0f }, 0.0f },
    { 6, 3, 18, 10, 0, { 0.0f, -185.0f }, { 1.0f, 1.0f }, 0.0f },
    { 6, 4, 19, 4, 0, { -150.0f, -140.0f }, { 0.8f, 0.8f }, 0.0f },
    { 6, 5, 20, 4, 0, { 150.0f, -140.0f }, { 0.8f, 0.8f }, 0.0f },
    { 7, 0, 21, 200, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 7, 1, 22, 200, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 7, 2, 23, 200, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 8, 0, 24, 0, 0, { 288.0f, 240.0f }, { 10.0f, 10.0f }, 0.0f }
};

s32 lbl_1_data_744[25] = {
    DATANUM(DATA_mdsing, 22), DATANUM(DATA_mdsing, 23),
    DATANUM(DATA_mdsing, 24), DATANUM(DATA_mdsing, 25),
    DATANUM(DATA_mdsing, 26), DATANUM(DATA_mdsing, 27),
    DATANUM(DATA_mdsing, 28), DATANUM(DATA_mdsing, 29),
    DATANUM(DATA_mdsing, 30), DATANUM(DATA_mdsing, 31),
    DATANUM(DATA_mdsing, 32), DATANUM(DATA_mdsing, 33),
    DATANUM(DATA_mdsing, 34), DATANUM(DATA_mdsing, 35),
    DATANUM(DATA_mdsing, 36), DATANUM(DATA_mdsing, 37),
    DATANUM(DATA_mdsing, 38), DATANUM(DATA_mdsing, 39),
    DATANUM(DATA_mdsing, 40), DATANUM(DATA_mdsing, 41),
    DATANUM(DATA_mdsing, 42), DATANUM(DATA_mdsing, 43),
    DATANUM(DATA_mdsing, 44), DATANUM(DATA_mdsing, 45),
    DATANUM(DATA_mdsing, 46)
};

Vec lbl_1_data_7A8[27] = {
    { 0.0f, 275.0f, 0.0f },
    { -340.0f, 375.0f, 0.0f },
    { 340.0f, 375.0f, 0.0f },
    { -140.0f, 275.0f, 0.0f },
    { 0.0f, 275.0f, 0.0f },
    { 140.0f, 275.0f, 0.0f },
    { -140.0f, 275.0f, 0.0f },
    { 0.0f, 275.0f, 0.0f },
    { 140.0f, 275.0f, 0.0f },
    { 0.0f, 275.0f, -800.0f },
    { -195.0f, 380.0f, 0.0f },
    { -65.0f, 380.0f, 0.0f },
    { 65.0f, 380.0f, 0.0f },
    { 195.0f, 380.0f, 0.0f },
    { -130.0f, 240.0f, 0.0f },
    { 0.0f, 240.0f, 0.0f },
    { 130.0f, 240.0f, 0.0f },
    { -195.0f, 100.0f, 0.0f },
    { -65.0f, 100.0f, 0.0f },
    { 65.0f, 100.0f, 0.0f },
    { 195.0f, 100.0f, 0.0f },
    { -60.0f, 65.0f, 0.0f },
    { 60.0f, 65.0f, 0.0f },
    { -75.0f, 300.0f, 0.0f },
    { 75.0f, 300.0f, 0.0f },
    { -60.0f, 125.0f, 0.0f },
    { 60.0f, 125.0f, 0.0f }
};

s32 lbl_1_data_8EC = -1;

char lbl_1_data_8F0[] = "# ========== win callback :: %d\n";

/* Nonzero permits confirmation of each of the three Solo boards. */
s16 lbl_1_data_912[3] = { 0, 0, 0 };

u32 lbl_1_data_918[4] = {
    DATANUM(DATA_s01, 0), DATANUM(DATA_s02, 0), DATANUM(DATA_s03, 0), DATANUM(DATA_w11, 0)
};

char lbl_1_data_928[] = "\n>>>>>>>>>> mdpartydll :: READ INIT!! <<<<<<<<<<\n";

char lbl_1_data_95A[] = "0x%x :: _effect\n";

char lbl_1_data_96B[] = "0x%x :: _gamemes\n";

char lbl_1_data_97D[] = "0x%x :: _mgconst\n";

char lbl_1_data_98F[] = "0x%x :: _win\n";

char lbl_1_data_99D[] = "\n";

char lbl_1_data_99F[] = "%d\n";

char lbl_1_data_9A3[] = "\n>>>>>>>>>> mdsingdll :: READ END!! <<<<<<<<<<\n";

char lbl_1_data_9D3[] = ">>>>>>>>>> mdsingdll :: objsetup!! <<<<<<<<<<\n";

char lbl_1_data_A02[] = "\n>>>>>>>>>> mdsingdll :: ovlreturn!! <<<<<<<<<<\n";

char lbl_1_data_A33[] = "-----===== CAMERA TEST =====-----";

char lbl_1_data_A55[] = "CENTER : %.2f, %.2f, %.2f";

char lbl_1_data_A6F[] = "ROT    : %.2f, %.2f, %.2f";

char lbl_1_data_A89[] = "ZOOM   : %.2f";

/* Active dialogue and instruction window indices; -1 means no window is selected. */
s16 lbl_1_data_A98[2] = { -1, -1 };

/* Cached dialogue message, choice, and instruction message IDs; -1 clears a slot. */
s32 lbl_1_data_A9C[3] = { -1, -1, -1 };

char lbl_1_data_AA8[] = "gN00m1-itemhook_R";

char lbl_1_data_ABA[] = "dpic00";

char lbl_1_data_AC1[] = "dpic01";

/* Whether each navigation sprite is active and should play its selection sound. */
s16 lbl_1_data_AC8[6] = { 0, 0, 0, 0, 0, 0 };

char lbl_1_data_AD4[] = "ys77120";

char lbl_1_data_ADC[] = "ys77121";

char lbl_1_data_AE4[] = "ys77060";

char lbl_1_data_AEC[] = "ys77061";

/* Character navigation neighbors and occupancy used to avoid duplicate choices. */
MDSING_CHARACTER_NEIGHBORS lbl_1_data_AF4[11] = {
    { { -1, -1, 1, 5, 4, -1, -1, -1 }, -1 },
    { { -1, -1, 2, 6, 5, 4, 0, -1 }, -1 },
    { { -1, -1, 3, -1, 6, 5, 1, -1 }, -1 },
    { { -1, -1, -1, -1, -1, 6, 2, -1 }, -1 },
    { { 0, 1, 5, 9, 8, 7, -1, -1 }, -1 },
    { { 1, 2, 6, 10, 9, 8, 4, 0 }, -1 },
    { { 2, 3, 10, 10, 10, 9, 5, 1 }, -1 },
    { { -1, 4, 8, -1, -1, -1, -1, -1 }, -1 },
    { { 4, 5, 9, -1, -1, -1, 7, -1 }, -1 },
    { { 5, 6, 10, -1, -1, -1, 8, 4 }, -1 },
    { { 6, -1, -1, -1, -1, -1, 9, 5 }, -1 }
};

char lbl_1_data_BBA[] = "\n-----===== retry setting ok!! =====-----\n\n";

char lbl_1_data_BE6[] = "\n-----===== retry setting!! =====-----\n\n";

char lbl_1_data_C0F[] = "\n-----===== MARIO PARTY 6 :: SINGLE MODE =====-----\n\n";

char lbl_1_data_C45[] = "%d - stat %d\n";

s32 lbl_1_bss_13A4[5];

/* Menu lighting, dialogue windows, camera, settings, and player choices. */
HU3D_LIGHTID lbl_1_bss_13A0[2];

HUWINID lbl_1_bss_1398[4];

MDSING_CAMERA lbl_1_bss_1348;

MDSING_SETTINGS lbl_1_bss_1340;

MDSING_CHARACTER_SELECTION lbl_1_bss_1308[4];

/* Loaded menu sprite animations, groups, sprites, and card texture animations. */
ANIMDATA *lbl_1_bss_12A4[25];

HUSPR_GROUPID lbl_1_bss_1292[9];

HUSPRID lbl_1_bss_1258[29];

ANIMDATA *lbl_1_bss_11F4[25];

MDSING_MODEL lbl_1_bss_E74[16];

/* Motion paths for the guide and selected characters. */
MDSING_MOTION_PATH lbl_1_bss_E14[2];

MDSING_MOTION_PATH lbl_1_bss_DB4[2];

MDSING_MOTION_PATH lbl_1_bss_CF4[4];

/* Player-marker target/current scales in x/y; z is reset during initialization. */
HuVecF lbl_1_bss_CDC[2];

/* Settling destinations for the four selection models, in world coordinates. */
HuVecF lbl_1_bss_CAC[4];

/* Current navigation-sprite scales, eased back toward 1 after selection feedback. */
f32 lbl_1_bss_C94[6];

/* Eight result rows, a page heading, and the result total label. */
HUWINID lbl_1_bss_C80[10];

/* Reward rows and earned-minigame rows displayed by the two result pages. */
MDSING_SCORE_ENTRY lbl_1_bss_880[128];

MDSING_SCORE_ENTRY lbl_1_bss_480[128];

/* Number of populated rows in each results page. */
s16 lbl_1_bss_47C[2];

ANIMDATA *lbl_1_bss_478;

s16 lbl_1_bss_474;

MDSING_RESULT_MODEL lbl_1_bss_3E4[9];

ANIMDATA *lbl_1_bss_3E0;

/* Falling reward controller, queued spawn count, and active state for each model. */
OMOBJ *lbl_1_bss_3DC;

s16 lbl_1_bss_3D8;

s16 lbl_1_bss_358[64];

MDSING_FALLING_MODEL_WORK lbl_1_bss_58[64];

ANIMDATA *lbl_1_bss_54;

OMOBJ *lbl_1_bss_50;

s16 lbl_1_bss_4C;

s16 lbl_1_bss_4A;

s16 lbl_1_bss_48;

OMOBJ *lbl_1_bss_44;

/* Earned stars: the summed reward divided by twenty, capped at ninety-nine. */
s16 lbl_1_bss_40;

/* Sum of positive values in the earned-prize results list. */
s16 lbl_1_bss_3E;

/* Prevent repeated opening/closing sounds when a batch of menu cards animates. */
s16 lbl_1_bss_3C;

OMOBJ *lbl_1_bss_38;

/* Nonzero when asynchronous board resource loading has finished. */
s16 lbl_1_bss_34;

/* Unlock flags for the extra difficulty and the eleventh selectable character. */
s16 lbl_1_bss_32;

s16 lbl_1_bss_30;

/* Returning opponent character and overlay entry event. */
s32 lbl_1_bss_2C;

s32 lbl_1_bss_28;

/* Menu animation controller; mtnId[0] signals completion to the waiting process. */
OMOBJ *lbl_1_bss_24;

/* Character shown during board-return reactions and whether extra return motions are loaded. */
s16 lbl_1_bss_22;

s16 lbl_1_bss_20;

MDSING_SELECTION_OBJECT lbl_1_bss_18;

/* Objects for player markers, character lineup, guide, background, and board models. */
OMOBJ *lbl_1_bss_14;

OMOBJ *lbl_1_bss_10;

OMOBJ *lbl_1_bss_C;

OMOBJ *lbl_1_bss_8;

OMOBJ *lbl_1_bss_4;

HUPROCESS *lbl_1_bss_0;

/* Window message callback: plays the speaker's guide sound when the message changes. */
void fn_1_0(HUWINID winId, u32 messageNo, s16 speakerNo)
{
    s32 sentinel[1] = { -1 };
    MDSING_AUDIO_TABLE guideSounds = {
        { MSM_SE_GUIDE_25, MSM_SE_GUIDE_26, MSM_SE_GUIDE_27, MSM_SE_GUIDE_28, MSM_SE_GUIDE_29,
          MSM_SE_GUIDE_30, MSM_SE_GUIDE_31, -1, MSM_SE_GUIDE_17, MSM_SE_GUIDE_18, MSM_SE_GUIDE_19,
          MSM_SE_GUIDE_20, MSM_SE_GUIDE_21, MSM_SE_GUIDE_22, MSM_SE_GUIDE_23, -1 }
    };
    s16 messageIndex;

    speakerNo -= 1;
    OSReport(lbl_1_data_8F0, (s16) speakerNo);
    if ((u32) lbl_1_data_8EC != messageNo) {
        lbl_1_data_8EC = messageNo;
        messageIndex = 0;
checkMessageSound:
        if (sentinel[messageIndex] == -1) {
            HuAudFXPlay(guideSounds.effectId[speakerNo]);
            return;
        }
        if (messageNo == (u32) sentinel[messageIndex]) {
            if (speakerNo >= 8) {
                HuAudFXPlayPan(guideSounds.effectId[speakerNo], 80);
                return;
            }
            HuAudFXPlayPan(guideSounds.effectId[speakerNo], 48);
            return;
        }
        messageIndex += 1;
        goto checkMessageSound;
    }
}

/* Loads saved menu flags and resets the three board entries before Solo setup. */
void fn_1_1A4(void)
{
    lbl_1_bss_30 = 0;
    lbl_1_bss_32 = 0;
    if (GWBankFlagGet(2)) {
        lbl_1_bss_30 = 1;
    }
    if (GWBankFlagGet(4)) {
        lbl_1_bss_32 = 1;
    }
    lbl_1_data_912[0] = 1;
    lbl_1_data_912[1] = 1;
    lbl_1_data_912[2] = 1;
}

/* Ensures the selected characters and three extra character variants have motion data before
 * play. */
void fn_1_250(void)
{
    s16 characterNo[5];
    s16 characterIndex;
    s32 loadStatus;

    characterNo[0] = lbl_1_bss_1308[0].characterNo;
    characterNo[1] = lbl_1_bss_1308[1].characterNo;
    characterNo[2] = CHARNO_MINIKOOPAR;
    characterNo[3] = CHARNO_MINIKOOPAG;
    characterNo[4] = CHARNO_MINIKOOPAB;
    for (characterIndex = 0; characterIndex < 5; characterIndex++) {
        if ((void *)CharMotionAMemPGet(characterNo[characterIndex]) == NULL) {
            break;
        }
    }
    if (characterIndex == 5) {
        return;
    }

    CharDataClose(CHARNO_NONE);
    for (characterIndex = 0; characterIndex < 5; characterIndex++) {
        loadStatus = HuDataDirReadAsync(CharDataDirTbl[characterNo[characterIndex]][4]);
        if (loadStatus != -1) {
            while (!HuDataGetAsyncStat(loadStatus)) {
                HuPrcVSleep();
            }
        }
        CharMotionInit(characterNo[characterIndex]);
        HuDataDirClose(CharDataDirTbl[characterNo[characterIndex]][4]);
    }
}

const u32 lbl_1_rodata_54[2] = { DATA_board_us, DATA_capsule };

/* Background loading process: prepares character motions, caches shared archives in ARAM, and loads
 * the chosen board. */
void fn_1_3A0(void)
{
    MdsingDirectoryPair aramDirectory =
        *(const MdsingDirectoryPair *)lbl_1_rodata_54;
    s16 characterNo[5];
    s16 characterIndex;
    s16 archiveIndex;
    s32 motionStatus;
    s32 dataStatus;

    characterNo[0] = lbl_1_bss_1308[0].characterNo;
    characterNo[1] = lbl_1_bss_1308[1].characterNo;
    characterNo[2] = CHARNO_MINIKOOPAR;
    characterNo[3] = CHARNO_MINIKOOPAG;
    characterNo[4] = CHARNO_MINIKOOPAB;
    for (characterIndex = 0; characterIndex < 5; characterIndex++) {
        if ((void *)CharMotionAMemPGet(characterNo[characterIndex]) == NULL) {
            break;
        }
    }
    if (characterIndex != 5) {
        CharDataClose(CHARNO_NONE);
        for (characterIndex = 0; characterIndex < 5; characterIndex++) {
            motionStatus = HuDataDirReadAsync(CharDataDirTbl[characterNo[characterIndex]][4]);
            if (motionStatus != -1) {
                while (!HuDataGetAsyncStat(motionStatus)) {
                    HuPrcVSleep();
                }
            }
            CharMotionInit(characterNo[characterIndex]);
            HuDataDirClose(CharDataDirTbl[characterNo[characterIndex]][4]);
        }
    }
    for (archiveIndex = 0; archiveIndex < 2; archiveIndex++) {
        dataStatus = HuDataDirReadAsync(aramDirectory.archiveId[archiveIndex]);
        if (dataStatus != -1) {
            while (!HuDataGetAsyncStat(dataStatus)) {
                HuPrcVSleep();
            }
        }
        HuAR_MRAMtoARAM(aramDirectory.archiveId[archiveIndex]);
        while (HuARDMACheck() != 0) {
            HuPrcVSleep();
        }
        HuDataDirClose(aramDirectory.archiveId[archiveIndex]);
    }
    dataStatus = HuDataDirReadAsync(lbl_1_data_918[lbl_1_bss_1340.boardNo]);
    if (dataStatus != -1) {
        while (!HuDataGetAsyncStat(dataStatus)) {
            HuPrcVSleep();
        }
    }
    lbl_1_bss_34 = 1;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Starts asynchronous board loading after releasing the Solo menu archive. */
void fn_1_5E8(void)
{
    lbl_1_bss_34 = 0;
    OSReport(lbl_1_data_928);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    HuDataDirClose(DATANUM(DATA_mdsing, 0));
    HuPrcChildCreate(fn_1_3A0, 256, 16384, 0, lbl_1_bss_0);
}

/* Commits the selected character, difficulty and minigame pack before starting a Solo board. */
void fn_1_6B4(void)
{
    s16 characterNo = 0;
    s16 playerNo;

    characterNo = lbl_1_bss_1308[0].characterNo;
    GwCommon.storyMgPack = lbl_1_bss_1340.minigamePack;
    for (playerNo = 0; playerNo < 1; playerNo++) {
        GwPlayer[playerNo].comF = 0;
        GwPlayer[playerNo].comDif = 0;
        GwPlayer[playerNo].charNo = lbl_1_bss_1308[playerNo].characterNo;
        GwPlayer[playerNo].padNo = 0;
        GwPlayer[playerNo].team = 0;
    }
    for (playerNo = 1; playerNo < 4; playerNo++) {
        GwPlayer[playerNo].comF = 1;
        GwPlayer[playerNo].comDif = 0;
        characterNo++;
        if (characterNo >= 14) {
            characterNo -= 14;
        }
        GwPlayer[playerNo].charNo = characterNo;
        GwPlayer[playerNo].padNo = playerNo;
        GwPlayer[playerNo].team = 0;
    }
    for (playerNo = 0; playerNo < 4; playerNo++) {
        GwPlayerConf[playerNo].grpNo = 0;
        GwPlayerConf[playerNo].type = GwPlayer[playerNo].comF;
        GwPlayerConf[playerNo].comDif = GwPlayer[playerNo].comDif;
        GwPlayerConf[playerNo].charNo = GwPlayer[playerNo].charNo;
        GwPlayerConf[playerNo].padNo = GwPlayer[playerNo].padNo;
        GwPlayerConf[playerNo].grpNo = GwPlayer[playerNo].team;
    }
    for (playerNo = 0; playerNo < 4; playerNo++) {
        OSReport(lbl_1_data_99F, GwPlayerConf[playerNo].charNo);
    }
    GwCommon.confSingleDiff = lbl_1_bss_1340.difficulty;
    GwCommon.storyMgPack = lbl_1_bss_1340.minigamePack;
    _ClearFlag(FLAG_BOARD_TUTORIAL);
    GWSingleDataInit();
    GWSingleMgRecordNumSet(0);
    GWSingleMgWinNumSet(0);
    GwSystem.turnPlayerNo = 0;
    for (playerNo = 0; playerNo < 4; playerNo++) {
        GwCommon.singleMgWinNum[playerNo] = 0;
    }
    mbSaveInit(lbl_1_bss_1340.boardNo + 6);
    mbSaveStoryInit(lbl_1_bss_1308[1].characterNo, lbl_1_bss_1340.minigamePack,
        lbl_1_bss_1340.difficulty);
}

/* Builds the all-computer player configuration used for the extra board entry. */
void fn_1_B34(void)
{
    s16 playerNo;

    for (playerNo = 0; playerNo < 1; playerNo++) {
        GwPlayer[playerNo].comF = 1;
        GwPlayer[playerNo].comDif = 0;
        GwPlayer[playerNo].charNo = 0;
        GwPlayer[playerNo].padNo = 0;
        GwPlayer[playerNo].team = 0;
    }
    for (playerNo = 1; playerNo < 4; playerNo++) {
        GwPlayer[playerNo].comF = 1;
        GwPlayer[playerNo].comDif = 0;
        GwPlayer[playerNo].charNo = playerNo;
        GwPlayer[playerNo].padNo = playerNo;
        GwPlayer[playerNo].team = 0;
    }
    for (playerNo = 0; playerNo < 4; playerNo++) {
        GwPlayerConf[playerNo].grpNo = 0;
        GwPlayerConf[playerNo].type = GwPlayer[playerNo].comF;
        GwPlayerConf[playerNo].comDif = GwPlayer[playerNo].comDif;
        GwPlayerConf[playerNo].charNo = GwPlayer[playerNo].charNo;
        GwPlayerConf[playerNo].padNo = GwPlayer[playerNo].padNo;
        GwPlayerConf[playerNo].grpNo = GwPlayer[playerNo].team;
    }
    lbl_1_bss_1308[1].characterNo = GwPlayer[1].charNo;
    for (playerNo = 0; playerNo < 4; playerNo++) {
        OSReport(lbl_1_data_99F, GwPlayerConf[playerNo].charNo);
    }
    lbl_1_bss_1340.boardNo = 3;
    mbSaveInit(10);
}

/* Waits for board loading, records the return event, and enters the selected Solo board. */
void fn_1_EA0(void)
{
    OMOVLHIS *history;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_34 == 0);

    history = omOvlHisGet(0);
    omOvlHisChg(0, history->ovl, 1, lbl_1_bss_1308[1].characterNo);
    OSReport(lbl_1_data_9A3);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    switch (lbl_1_bss_1340.boardNo) {
        case 0:
            omOvlCallEx(114, 1, 0, 0);
            break;
        case 1:
            omOvlCallEx(115, 1, 0, 0);
            break;
        case 2:
            omOvlCallEx(116, 1, 0, 0);
            break;
    }
}

/* Waits for loading and enters the extra board with its own return event. */
void fn_1_FEC(void)
{
    OMOVLHIS *history;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_34 == 0);

    history = omOvlHisGet(0);
    omOvlHisChg(0, history->ovl, 2, 0);
    OSReport(lbl_1_data_9A3);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    omOvlCallEx(130, 1, 0, 0);
}

/* Releases character and shared board archives after returning from a Solo board. */
void fn_1_10D0(void)
{
    CharDataClose(CHARNO_NONE);
    HuARDirFree(DATANUM(DATA_board, 0));
    HuARDirFree(DATANUM(DATA_board_us, 0));
    HuARDirFree(DATANUM(DATA_capsule, 0));
    OSReport(lbl_1_data_9D3);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
}

/* Releases character and shared board archives before the mode resumes its menu flow. */
void fn_1_1180(void)
{
    CharDataClose(CHARNO_NONE);
    HuARDirFree(DATANUM(DATA_board, 0));
    HuARDirFree(DATANUM(DATA_board_us, 0));
    HuARDirFree(DATANUM(DATA_capsule, 0));
    OSReport(lbl_1_data_A02);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
}

/* Evaluates one quadratic path component at normalized progress for menu movement. */
float fn_1_1230(float start, float middle, float end, float time)
{
    float inverse = 1.0f - time;

    return end * (time * time)
        + (start * (inverse * inverse)
        + 2.0f * (middle * (inverse * time)));
}

/* Evaluates a quadratic path component for the vector path helper. */
static inline float quadratic_interpolate(float start, float middle, float end, float time)
{
    float inverse = 1.0f - time;

    return (end * (time * time))
        + ((start * (inverse * inverse))
        + ((middle * (inverse * time)) * 2.0f));
}

/* Evaluates all three coordinates of a quadratic path during model movement. */
void fn_1_128C(HuVecF *dst, const HuVecF *start, const HuVecF *middle,
    const HuVecF *end, float time)
{
    dst->x = quadratic_interpolate(start->x, middle->x, end->x, time);
    dst->y = quadratic_interpolate(start->y, middle->y, end->y, time);
    dst->z = quadratic_interpolate(start->z, middle->z, end->z, time);
}

/* Returns one weighted smoothing step for animated menu, model and camera values. */
float fn_1_1494(float current, float target, float weight)
{
    if (current == target) {
        return target;
    }
    return (target + (current * (weight - 1.0f))) / weight;
}

/* Smooths all three coordinates toward a target during menu and camera updates. */
void fn_1_14C4(HuVecF *current, const HuVecF *target, float weight)
{
    current->x = fn_1_1494(current->x, target->x, weight);
    current->y = fn_1_1494(current->y, target->y, weight);
    current->z = fn_1_1494(current->z, target->z, weight);
}

/* Returns a clamped sine-eased value for entrance and fade animations. */
float fn_1_1608(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((endValue - startValue) * sind((90.0f / duration) * time));
}

/* Returns a clamped linear interpolation for timed menu and model animations. */
float fn_1_16F0(float startValue, float endValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f) {
        return startValue;
    }
    if (elapsedFrames >= durationFrames) {
        return endValue;
    }
    return startValue + ((elapsedFrames / durationFrames) * (endValue - startValue));
}

/* Moves a model along a straight path and smooths its facing toward its movement direction. */
void fn_1_1734(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration)
{
    HuVecF previousPosition;
    HuVecF previousRotation;
    HuVecF nextPosition;
    HuVecF nextRotation;

    Hu3DModelPosGet(modelId, &previousPosition);
    Hu3DModelRotGet(modelId, &previousRotation);
    nextPosition.x = fn_1_16F0(start->x, end->x, time, duration);
    nextPosition.y = fn_1_16F0(start->y, end->y, time, duration);
    nextPosition.z = fn_1_16F0(start->z, end->z, time, duration);
    previousPosition.x -= nextPosition.x;
    previousPosition.z -= nextPosition.z;
    nextRotation.y = -(180.0 * (atan2(previousPosition.x, -previousPosition.z) / M_PI));
    if (previousRotation.y - nextRotation.y > 180.0f) {
        previousRotation.y -= 360.0f;
    } else if (previousRotation.y - nextRotation.y < -180.0f) {
        previousRotation.y += 360.0f;
    }
    nextRotation.x = previousRotation.x;
    nextRotation.y = fn_1_1494(previousRotation.y, nextRotation.y, 10.0f);
    nextRotation.z = previousRotation.z;
    Hu3DModelPosSetV(modelId, &nextPosition);
    Hu3DModelRotSetV(modelId, &nextRotation);
}

/* Smooths a model toward the requested Y rotation after its movement has finished. */
void fn_1_1A9C(HU3D_MODELID modelId, float rotY)
{
    HuVecF previousRotation;
    HuVecF nextRotation;

    Hu3DModelRotGet(modelId, &previousRotation);
    nextRotation.y = rotY;
    if (previousRotation.y - nextRotation.y > 180.0f) {
        previousRotation.y -= 360.0f;
    } else if (previousRotation.y - nextRotation.y < -180.0f) {
        previousRotation.y += 360.0f;
    }
    nextRotation.x = previousRotation.x;
    nextRotation.y = fn_1_1494(previousRotation.y, nextRotation.y, 10.0f);
    nextRotation.z = previousRotation.z;
    Hu3DModelRotSetV(modelId, &nextRotation);
}

/* Updates a model's path movement, then settles its facing once the travel time has elapsed. */
void fn_1_1BDC(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY)
{
    if (time <= duration) {
        fn_1_1734(modelId, start, end, time, duration);
    } else {
        fn_1_1A9C(modelId, rotY);
    }
}

/* Returns a clamped cosine-eased value for timed menu transitions. */
float fn_1_205C(float start, float end, float time, float duration)
{
    if (time <= 0.0f) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return (float)(start + ((end - start) *
        (1.0 - cos(
            (M_PI * ((90.0f / duration) * time)) /
                180.0))));
}

/* Returns a sine pulse that rises from the starting value and returns to it over the animation. */
float fn_1_2154(float startValue, float peakValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f || elapsedFrames >= durationFrames) {
        return startValue;
    }
    return startValue +
           ((peakValue - startValue) * sind((180.0f / durationFrames) * elapsedFrames));
}

/* Evaluates a quadratic path component for curved model movement. */
static inline float fn_1_13A8_inlined(float startValue, float controlValue, float endValue,
                                      float progress)
{
    float remaining = 1.0f - progress;

    return (endValue * (progress * progress))
        + ((startValue * (remaining * remaining))
            + ((controlValue * (remaining * progress) * 2.0f)));
}

/* Evaluates the three coordinates of the curved movement path. */
static inline void fn_1_1404_inlined(
    HuVecF *dst, const HuVecF *start, const HuVecF *control, const HuVecF *end, float progress)
{
    dst->x = fn_1_13A8_inlined(start->x, control->x, end->x, progress);
    dst->y = fn_1_13A8_inlined(start->y, control->y, end->y, progress);
    dst->z = fn_1_13A8_inlined(start->z, control->z, end->z, progress);
}

/* Moves a model along a curved path while turning it toward the next position. */
void fn_1_2228(
    HU3D_MODELID modelId, const HuVecF *start, const HuVecF *control, const HuVecF *end,
    float time, float duration)
{
    HuVecF nextPosition;
    HuVecF previousPosition;
    HuVecF rotation;
    float progress;

    progress = fn_1_16F0(0.0f, 1.0f, time, duration);
    fn_1_1404_inlined(&nextPosition, start, control, end, progress);
    Hu3DModelPosGet(modelId, &previousPosition);
    Hu3DModelRotGet(modelId, &rotation);
    previousPosition.x -= nextPosition.x;
    previousPosition.z -= nextPosition.z;
    previousPosition.y = -(180.0 * (atan2(previousPosition.x, -previousPosition.z) / M_PI));
    if (rotation.y - previousPosition.y > 180.0f) {
        rotation.y -= 360.0f;
    } else if (rotation.y - previousPosition.y < -180.0f) {
        rotation.y += 360.0f;
    }
    rotation.y = fn_1_1494(rotation.y, previousPosition.y, 10.0f);
    Hu3DModelPosSet(modelId, nextPosition.x, nextPosition.y, nextPosition.z);
    Hu3DModelRotSet(modelId, 0.0f, rotation.y, 0.0f);
}

/* Resets character selection records, leaving the first slot human and the remaining slots computer
 * controlled. */
void fn_1_267C(void)
{
    MDSING_CHARACTER_SELECTION *selection;
    s16 playerNo;

    for (playerNo = 0, selection = lbl_1_bss_1308; playerNo < 4; playerNo++, selection++) {
        selection->choiceState = 0;
        selection->setupValue = 0;
        selection->isComputer = 0;
        if (playerNo != 0) {
            selection->isComputer = 1;
        }
        selection->difficulty = 0;
        selection->characterNo = 0;
        selection->padNo = playerNo;
    }
}

/* Snaps the camera's current center, rotation and zoom to the requested target. */
void fn_1_26FC(MDSING_CAMERA_WORK *work)
{
    memcpy(&work->center, &work->targetCenter, sizeof(HuVecF));
    memcpy(&work->rot, &work->targetRot, sizeof(HuVecF));
    work->zoom = work->targetZoom;
}

/* Copies the current camera pose into its target before a new transition is prepared. */
void fn_1_274C(MDMinidllCameraState *work)
{
    memcpy(&work->targetCenter, &work->center, sizeof(HuVecF));
    memcpy(&work->targetRot, &work->rot, sizeof(HuVecF));
    work->targetZoom = work->zoom;
}

/* Smooths the current camera pose toward its target during camera updates. */
void fn_1_279C(MDSING_CAMERA_WORK *camera, float weight)
{
    fn_1_14C4(&camera->center, &camera->targetCenter, weight);
    fn_1_14C4(&camera->rot, &camera->targetRot, weight);
    camera->zoom = fn_1_1494(camera->zoom, camera->targetZoom, weight);
}

/* Selects the camera update callback for the current menu phase. */
void fn_1_2A50(MDCAMERA_CALLBACK callback)
{
    lbl_1_bss_1348.callback = callback;
}

/* Debug camera callback: moves the view with controller one and displays its current pose. */
void fn_1_2A60(OMOBJ *obj, MDCAMERA_WORK *camera)
{
    float stickX = HuPadStkX[0];
    float stickY = -HuPadStkY[0];
    HuVec2f subStick;
    float moveX;
    float moveZ;

    subStick.y = -HuPadSubStkY[0];
    subStick.x = HuPadSubStkX[0];
    camera->targetRot.x += 0.02f * subStick.y;
    if (camera->targetRot.x > 85.0f) {
        camera->targetRot.x = 85.0f;
    }
    if (camera->targetRot.x < -85.0f) {
        camera->targetRot.x = -85.0f;
    }
    camera->targetRot.y += 0.02f * subStick.x;

    if (HuPadBtn[0] & PAD_TRIGGER_L) {
        camera->targetCenter.y -= 0.25f * stickY;
    } else if (HuPadBtn[0] & PAD_TRIGGER_R) {
        camera->targetZoom += 0.25f * stickY;
        if (camera->targetZoom < 5.0f) {
            camera->targetZoom = 5.0f;
        }
    } else {
        moveX = stickX * sind(90.0f + camera->rot.y)
            + stickY * sind(camera->rot.y);
        moveZ = stickX * cosd(90.0f + camera->rot.y)
            + stickY * cosd(camera->rot.y);
        camera->targetCenter.x += 0.25f * moveX;
        camera->targetCenter.z += 0.25f * moveZ;
    }
    fn_1_279C(camera, 15.0f);

    print8(16, 110, 1.0f, lbl_1_data_A33);
    print8(16, 120, 1.0f, lbl_1_data_A55,
        camera->center.x, camera->center.y, camera->center.z);
    print8(16, 130, 1.0f, lbl_1_data_A6F,
        camera->rot.x, camera->rot.y, camera->rot.z);
    print8(16, 140, 1.0f, lbl_1_data_A89, camera->zoom);
}

/* Dispatches the active camera callback for an object update. */
void fn_1_3170(OMOBJ *obj, MDMinidllCameraState *state)
{
    if (state->callback) {
        state->callback(obj, state);
    }
}

/* Camera object callback: applies the active camera state to the view each frame. */
void fn_1_31BC(OMOBJ *obj)
{
    MDMinidllCameraState *state;

    state = &lbl_1_bss_1348;
    if (state->callback) {
        state->callback(obj, state);
    }
    Center.x = state->center.x;
    Center.y = state->center.y;
    Center.z = state->center.z;
    CRot.x = state->rot.x;
    CRot.y = state->rot.y;
    CRot.z = state->rot.z;
    CZoom = state->zoom;
    omOutView(obj);
}

/* Creates the Solo camera and its per-frame object before the menus are displayed. */
void fn_1_328C(MDSING_CAMERA_CALLBACK callback)
{
    MDSING_CAMERA_WORK *camera = &lbl_1_bss_1348;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f,
        10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
        640.0f, 480.0f, 0.0f,
        1.0f);
    memset(camera, 0, sizeof(*camera));
    camera->callback = callback;
    camera->center.x = 0.0f;
    camera->center.y = 65.0f;
    camera->center.z = -800.0f;
    camera->rot.x = -7.25f;
    camera->rot.y = 0.0f;
    camera->rot.z = 0.0f;
    if (lbl_1_bss_28 == 0) {
        camera->zoom = 2650.0f;
    } else {
        camera->zoom = 2150.0f;
    }
    camera->obj = omAddObjEx(lbl_1_bss_0, 2304, 16, 16, -1, fn_1_31BC);
}

/* Destroys the Solo camera and its update object when the mode closes. */
void fn_1_3428(void)
{
    MDMinidllCameraState *state;
    state = &lbl_1_bss_1348;
    Hu3DCameraKill(1);
    if (state->obj) {
        omDelObjEx(lbl_1_bss_0, state->obj);
    }
    state->obj = NULL;
}

/* Creates two opposing white directional lights for the Solo menu scene. */
void fn_1_3484(void)
{
    lbl_1_bss_13A0[0] = Hu3DGLightCreate(0.0f, 1.0f, 1.0f,
                                        0.0f, -1.0f, -1.0f,
                                        255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_13A0[0]);
    Hu3DGLightStaticSet(lbl_1_bss_13A0[0], TRUE);
    lbl_1_bss_13A0[1] = Hu3DGLightCreate(-1.0f, 1.0f, -1.0f,
                                        1.0f, -1.0f, -1.0f,
                                        255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_13A0[1]);
    Hu3DGLightStaticSet(lbl_1_bss_13A0[1], TRUE);
}

/* Releases the two scene lights when the Solo menus are torn down. */
void fn_1_35B0(void)
{
    Hu3DGLightKill(lbl_1_bss_13A0[0]);
    Hu3DGLightKill(lbl_1_bss_13A0[1]);
}

/* Opens a requested menu window; the frameless help window appears without an opening animation. */
void fn_1_35EC(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_1398[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_1398[winNo]);
    }
}

/* Closes a requested menu window; the frameless help window is hidden immediately. */
void fn_1_365C(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_1398[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_1398[winNo]);
    }
}

void fn_1_36CC(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_1398[winNo]);
}

/* Waits for a menu choice, optionally disabling cancellation or mapping cancellation to the second
 * choice. */
s16 fn_1_3708(s16 winNo, s16 mode)
{
    s16 choice = 0;

    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_1398[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_1398[winNo], HUWIN_ATTR_NOCANCEL);
    }
    choice = HuWinChoiceGet(lbl_1_bss_1398[winNo], -1);
    if (mode == 2 && choice == -1) {
        choice = 1;
    }
    return choice;
}

/* Sets centered dialogue text and its reveal speed before the menu waits for input. */
void fn_1_37DC(s16 winNo, s32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_1398[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_1398[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_1398[winNo], speed);
    if (lbl_1_data_8EC != messNum) {
        lbl_1_data_8EC = -1;
    }
}

/* Replaces an inserted message when a menu updates a value inside its dialogue. */
void fn_1_3898(s16 winNo, s32 messNum, s16 insertPos)
{
    HuWinHomeClear(lbl_1_bss_1398[winNo]);
    HuWinInsertMesSet(lbl_1_bss_1398[winNo], messNum, insertPos);
}

/* Creates the four dialogue and help windows, using controller one and the speaker sound
 * callback. */
void fn_1_3908(void)
{
    s16 windowNo;

    HuWinInit(1);
    lbl_1_bss_1398[0] = HuWinExCreateFrame(16.0f, 337.0f, 544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_1398[0]);
    HuWinBGTPLvlSet(lbl_1_bss_1398[0], 0.0f);
    lbl_1_bss_1398[1] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 0);
    HuWinDispOff(lbl_1_bss_1398[1]);
    HuWinBGTPLvlSet(lbl_1_bss_1398[1], 0.9f);
    lbl_1_bss_1398[2] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 3);
    HuWinDispOff(lbl_1_bss_1398[2]);
    HuWinBGTPLvlSet(lbl_1_bss_1398[2], 0.9f);
    lbl_1_bss_1398[3] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 4);
    HuWinDispOff(lbl_1_bss_1398[3]);
    HuWinBGTPLvlSet(lbl_1_bss_1398[3], 0.9f);
    for (windowNo = 0; windowNo < 4; windowNo++) {
        winData[lbl_1_bss_1398[windowNo]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_1398[windowNo], fn_1_0);
    }
}

/* Destroys the Solo dialogue windows during mode teardown. */
void fn_1_3B30(void)
{
    s16 windowNo;
    windowNo = 0;
    while (windowNo < 4) {
        HuWinExKill(lbl_1_bss_1398[windowNo]);
        windowNo += 1;
    }
    HuWinAllKill();
}

/* Switches the active dialogue window and clears cached text when the menu changes window style. */
void fn_1_3B8C(s16 winNo)
{
    if (lbl_1_data_A98[0] != -1 && lbl_1_data_A98[0] != winNo) {
        fn_1_365C(lbl_1_data_A98[0]);
    }
    if (lbl_1_data_A98[0] == -1 || lbl_1_data_A98[0] != winNo) {
        lbl_1_data_A98[0] = winNo;
        lbl_1_data_A9C[0] = -1;
        lbl_1_data_A9C[1] = -1;
        fn_1_35EC(lbl_1_data_A98[0]);
    }
}

/* Closes a menu window, hiding the frameless help window immediately. */
static inline void fn_1_A18(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_1398[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_1398[winNo]);
    }
}

/* Closes the active dialogue window and clears its message cache between menu phases. */
void fn_1_3CF8(void)
{
    if (lbl_1_data_A98[0] != -1) {
        fn_1_365C(lbl_1_data_A98[0]);
    }
    lbl_1_data_A98[0] = -1;
    lbl_1_data_A9C[0] = -1;
    lbl_1_data_A9C[1] = -1;
}

/* Waits for the active dialogue to finish before the menu process continues. */
void fn_1_3DB4(void)
{
    if (lbl_1_data_A98[0] != -1) {
        fn_1_36CC(lbl_1_data_A98[0]);
    }
}

/* Reads a choice from the current dialogue window during a menu decision. */
s16 fn_1_3E14(s16 mode)
{
    if (lbl_1_data_A98[0] != -1) {
        return fn_1_3708(lbl_1_data_A98[0], mode);
    }
    return 0;
}

/* Shows dialogue in the requested window, retaining text that is already displayed. */
void fn_1_3F0C(s16 winNo, s32 messNum, s16 speed)
{
    fn_1_3B8C(winNo);
    if (lbl_1_data_A9C[0] != messNum) {
        lbl_1_data_A9C[0] = messNum;
        lbl_1_data_A9C[1] = -1;
        fn_1_37DC(lbl_1_data_A98[0], lbl_1_data_A9C[0], speed);
    }
}

/* Updates an inserted dialogue value only when that value changes. */
void fn_1_4144(s16 winNo, s32 messNum, s16 insertPos)
{
    fn_1_3B8C(winNo);
    if (lbl_1_data_A9C[1] != messNum) {
        lbl_1_data_A9C[0] = -1;
        lbl_1_data_A9C[1] = messNum;
        fn_1_3898(lbl_1_data_A98[0], lbl_1_data_A9C[1], insertPos);
    }
}

/* Opens a help window and restores its upper screen position for the menu prompt. */
static inline void show_message_window(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_1398[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_1398[winNo]);
    }
    HuWinPosSet(lbl_1_bss_1398[0], 16.0f, 337.0f);
}

/* Sets centered menu text and resets the speaker sound cache for a new message. */
static inline void set_message_window(s16 winNo, s32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_1398[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_1398[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_1398[winNo], speed);
    if (lbl_1_data_8EC != messNum) {
        lbl_1_data_8EC = -1;
    }
}

/* Shows a persistent help message at the upper help-window position. */
void fn_1_4338(s32 messNum)
{
    if (lbl_1_data_A98[1] == -1) {
        lbl_1_data_A98[1] = 0;
        lbl_1_data_A9C[2] = -1;
        show_message_window(lbl_1_data_A98[1]);
    }
    if (lbl_1_data_A9C[2] != messNum) {
        lbl_1_data_A9C[2] = messNum;
        set_message_window(lbl_1_data_A98[1], lbl_1_data_A9C[2], 0);
    }
}

/* Shows a persistent help message along the bottom of the screen. */
void fn_1_44E4(s32 messNum)
{
    if (lbl_1_data_A98[1] == -1) {
        lbl_1_data_A98[1] = 0;
        lbl_1_data_A9C[2] = -1;
        fn_1_35EC(lbl_1_data_A98[1]);
        HuWinPosSet(lbl_1_bss_1398[0], 16.0f, 408.0f);
    }
    if (lbl_1_data_A9C[2] != messNum) {
        lbl_1_data_A9C[2] = messNum;
        set_message_window(lbl_1_data_A98[1], lbl_1_data_A9C[2], 0);
    }
}

/* Hides the persistent help message when its menu phase ends. */
void fn_1_4690(void)
{
    if (lbl_1_data_A98[1] != -1) {
        fn_1_A18(lbl_1_data_A98[1]);
    }
    lbl_1_data_A98[1] = -1;
    lbl_1_data_A9C[2] = -1;
}

const Vec lbl_1_rodata_108 = { 0.0f, 3000.0f, 600.0f };

const Vec lbl_1_rodata_114 = { 0.0f, 1.0f, 0.0f };

const Vec lbl_1_rodata_120 = { 0.0f, 0.0f, 0.0f };

/* Creates the shadow projection used by the Solo menu's models. */
void fn_1_473C(void)
{
    Point3d lightPosition;
    Point3d upDirection;
    Point3d targetPosition;

    lightPosition = lbl_1_rodata_108;
    upDirection = lbl_1_rodata_114;
    targetPosition = lbl_1_rodata_120;
    Hu3DShadowCreate(30.0f, 10.0f, 10000.0f);
    Hu3DShadowPosSet(&lightPosition, &upDirection, &targetPosition);
}

void fn_1_47F0(void)
{
}

/* Applies attributes to every member of a sprite group during menu display changes. */
void fn_1_47F4(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

/* Clears attributes from every member of a sprite group during menu display changes. */
void fn_1_4874(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}

/* Loads menu sprite animations and creates their groups and members, initially hidden. */
void fn_1_48F4(void)
{
    MDSING_SPRITE_DESC *spriteDesc;
    s16 entryNo;

    for (entryNo = 0; entryNo < 25; entryNo++) {
        lbl_1_bss_12A4[entryNo] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_32C[entryNo], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (entryNo = 0; entryNo < 9; entryNo++) {
        lbl_1_bss_1292[entryNo] = HuSprGrpCreate(lbl_1_data_390[entryNo]);
    }
    for (entryNo = 0, spriteDesc = lbl_1_data_3A4; entryNo < 29; entryNo++, spriteDesc++) {
        lbl_1_bss_1258[entryNo] = HuSprCreate(
            lbl_1_bss_12A4[spriteDesc->animNo], spriteDesc->priority, spriteDesc->bank);
        HuSprGrpMemberSet(lbl_1_bss_1292[spriteDesc->groupNo], spriteDesc->memberNo,
            lbl_1_bss_1258[entryNo]);
        HuSprPosSet(lbl_1_bss_1292[spriteDesc->groupNo], spriteDesc->memberNo,
            spriteDesc->pos.x, spriteDesc->pos.y);
        HuSprScaleSet(lbl_1_bss_1292[spriteDesc->groupNo], spriteDesc->memberNo,
            spriteDesc->scale.x, spriteDesc->scale.y);
        HuSprZRotSet(lbl_1_bss_1292[spriteDesc->groupNo], spriteDesc->memberNo,
            spriteDesc->zRot);
    }
    for (entryNo = 0; entryNo < 9; entryNo++) {
        fn_1_47F4(
            lbl_1_bss_1292[entryNo], HUSPR_ATTR_DISPOFF);
    }
}

void fn_1_4B40(void)
{
}

/* Returns a clamped sine-eased coordinate for the guide-linked effect's movement. */
static inline f32 fn_1_4C78_ease90_inlined(f32 start, f32 end, f32 elapsed, f32 duration)
{
    if (elapsed <= 0.0f) {
        return start;
    }
    if (elapsed >= duration) {
        return end;
    }
    return (f32)((f64)start + (f64)(end - start) *
        sin((M_PI * (f64)((90.0f / duration) * elapsed)) / 180.0));
}

/* Raises and lowers a guide-linked effect coordinate over its flight. */
static inline f32 fn_1_4C78_ease180_inlined(f32 start, f32 end, f32 elapsed, f32 duration)
{
    if ((elapsed <= 0.0f) || (elapsed >= duration)) {
        return start;
    }
    return (f32)((f64)start + (f64)(end - start) *
        sin((M_PI * (f64)((180.0f / duration) * elapsed)) / 180.0));
}

/* Object callback: moves the guide-linked effects outward along arcing paths. */
void fn_1_4B44(OMOBJ *obj)
{
    HuVecF position;
    MDSING_MOTION_PATH *first;
    MDSING_MOTION_PATH *second;
    s16 completed;
    OMOBJ *firstObj;
    OMOBJ *secondObj;
    f32 duration;

    completed = 0;
    duration = 60.0f;
    firstObj = lbl_1_bss_C;
    secondObj = NULL;
    first = lbl_1_bss_E14;
    second = &lbl_1_bss_E14[1];
    if (firstObj != NULL) {
        position.x = fn_1_4C78_ease90_inlined(first->position.x, 500.0f,
            first->elapsed, duration);
        position.y = fn_1_4C78_ease180_inlined(first->position.y, 400.0f,
            first->elapsed, duration);
        position.z = first->position.z;
        if ((first->elapsed += 1.0f) > duration) {
            completed += 1;
            fn_1_323A8(0, NULL, 0);
        } else {
            fn_1_323A8(0, &position, 1);
        }
    }
    if (secondObj != NULL) {
        position.x = fn_1_4C78_ease90_inlined(second->position.x, -500.0f,
            second->elapsed, duration);
        position.y = fn_1_4C78_ease180_inlined(second->position.y, 400.0f,
            second->elapsed, duration);
        position.z = second->position.z;
        if ((second->elapsed += 1.0f) > duration) {
            completed += 1;
            fn_1_323A8(1, NULL, 0);
        } else {
            fn_1_323A8(1, &position, 1);
        }
    }
    /* The secondary source is disabled, but completion still requires both slots. */
    if (completed >= 2) {
        obj->objFunc = NULL;
    }
}

/* Object callback: follows the guide attachment for thirty frames before launching its effect. */
void fn_1_4F94(OMOBJ *obj)
{
    OMOBJ *primaryObject = lbl_1_bss_C;
    OMOBJ *secondaryObject = NULL;
    MDSING_MOTION_PATH *primaryPath = &lbl_1_bss_E14[0];
    MDSING_MOTION_PATH *secondaryPath = &lbl_1_bss_E14[1];

    if (primaryObject != NULL) {
        Hu3DModelObjPosGet(primaryObject->mdlId[0], lbl_1_data_AA8, &primaryPath->position);
        fn_1_323A8(0, &primaryPath->position, 1);
        primaryPath->elapsed = 0.0f;
    }
    if (secondaryObject != NULL) {
        Hu3DModelObjPosGet(secondaryObject->mdlId[0], lbl_1_data_AA8, &secondaryPath->position);
        fn_1_323A8(1, &secondaryPath->position, 1);
        secondaryPath->elapsed = 0.0f;
    }
    if (++obj->work[0] > 30) {
        obj->work[0] = 0;
        obj->objFunc = fn_1_4B44;
    }
}

/* Starts the guide's gesture and creates the attachment-following effect object. */
void fn_1_5098(void)
{
    Hu3DMotionShiftSet(*lbl_1_bss_C->mdlId, lbl_1_bss_C->mtnId[7],
                       0.0f, 5.0f, 0);
    lbl_1_bss_38 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, fn_1_4F94);
    lbl_1_bss_38->work[0] = 0;
    HuAudFXPlay(MSM_SE_MENU_16);
}

/* Guide callback: plays a faster gesture, then restores normal speed and the idle motion. */
void fn_1_514C(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(*obj->mdlId, 2.0f);
    if (obj->work[3]++ > 30U) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(*obj->mdlId, 1.0f);
        Hu3DMotionShiftSet(*obj->mdlId, *obj->mtnId, 0.0f,
                           15.0f, HU3D_MOTATTR_LOOP);
    }
}

/* Starts the guide's short greeting motion before dialogue and installs its idle-restoring
 * callback. */
void fn_1_51F4(void)
{
    OMOBJ *obj = lbl_1_bss_C;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[3],
        0.0f, 10.0f, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_514C;
}

/* Updates a model's curved travel and smooths its facing during a menu transition. */
static inline void fn_1_22A8_inlined(
    HU3D_MODELID modelId, const HuVecF *start, const HuVecF *control, const HuVecF *end,
    float time, float duration)
{
    HuVecF nextPosition;
    HuVecF previousPosition;
    HuVecF rotation;
    float progress;

    progress = fn_1_16F0(0.0f, 1.0f, time, duration);
    fn_1_1404_inlined(&nextPosition, start, control, end, progress);
    Hu3DModelPosGet(modelId, &previousPosition);
    Hu3DModelRotGet(modelId, &rotation);
    previousPosition.x -= nextPosition.x;
    previousPosition.z -= nextPosition.z;
    previousPosition.y = -(180.0 * (atan2(previousPosition.x, -previousPosition.z) / M_PI));
    if (rotation.y - previousPosition.y > 180.0f) {
        rotation.y -= 360.0f;
    } else if (rotation.y - previousPosition.y < -180.0f) {
        rotation.y += 360.0f;
    }
    rotation.y = fn_1_1494(rotation.y, previousPosition.y, 10.0f);
    Hu3DModelPosSet(modelId, nextPosition.x, nextPosition.y, nextPosition.z);
    Hu3DModelRotSet(modelId, 0.0f, rotation.y, 0.0f);
}

/* Guide callback: rises briefly, follows its departure curve, and emits a trailing effect until
 * finished. */
void fn_1_5268(OMOBJ *obj)
{
    MDSING_MOTION_PATH *work = &lbl_1_bss_DB4[0];
    HuVecF pos;

    if (work->state == 0) {
        Hu3DModelPosGet(obj->mdlId[0], &pos);
        pos.y += 5.0f;
        Hu3DModelPosSetV(obj->mdlId[0], &pos);
        if ((work->elapsed += 1.0f) > work->duration) {
            Hu3DModelPosGet(obj->mdlId[0], &work->position);
            work->state = 1;
            work->elapsed = 0.0f;
            work->duration = 90.0f;
        }
    }
    if (work->state == 1) {
        fn_1_22A8_inlined(obj->mdlId[0], &work->position, &work->controlPosition,
            &work->endPosition, work->elapsed, work->duration);
        if ((work->elapsed += 1.0f) > work->duration) {
            HuAudFXStop(lbl_1_bss_13A4[0]);
            work->state = 2;
        }
    }
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    fn_1_34FA4(4, &pos, 1, 5);
    if (work->state == 2) {
        obj->objFunc = NULL;
    }
}

/* Prepares the guide's departure path and starts its motion, looping sound, and per-frame
 * updater. */
void fn_1_57E4(void)
{
    MDSING_MOTION_PATH *work = &lbl_1_bss_DB4[0];
    OMOBJ *obj = lbl_1_bss_C;

    work->state = 0;
    work->elapsed = 0.0f;
    work->duration = 20.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->position);
    work->position.x = -325.0f;
    work->position.y = 0.0f;
    work->position.z = 100.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->controlPosition);
    work->controlPosition.x = 0.0f;
    work->controlPosition.y = 400.0f;
    work->controlPosition.z = 200.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->endPosition);
    work->endPosition.x = 0.0f;
    work->endPosition.y = 125.0f;
    work->endPosition.z = -1000.0f;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[2],
        0.0f, 0.0f, 0);
    lbl_1_bss_13A4[0] = HuAudFXPlay(MSM_SE_BRD00_146);
    obj->objFunc = fn_1_5268;
}

/* Moves a model along a straight path and smooths its facing during menu transitions. */
static inline void fn_1_18AC_inlined(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration)
{
    HuVecF previousPosition;
    HuVecF previousRotation;
    HuVecF nextPosition;
    HuVecF nextRotation;

    Hu3DModelPosGet(modelId, &previousPosition);
    Hu3DModelRotGet(modelId, &previousRotation);
    nextPosition.x = fn_1_16F0(start->x, end->x, time, duration);
    nextPosition.y = fn_1_16F0(start->y, end->y, time, duration);
    nextPosition.z = fn_1_16F0(start->z, end->z, time, duration);
    previousPosition.x -= nextPosition.x;
    previousPosition.z -= nextPosition.z;
    nextRotation.y = -(180.0 * (atan2(previousPosition.x, -previousPosition.z) / M_PI));
    if (previousRotation.y - nextRotation.y > 180.0f) {
        previousRotation.y -= 360.0f;
    } else if (previousRotation.y - nextRotation.y < -180.0f) {
        previousRotation.y += 360.0f;
    }
    nextRotation.x = previousRotation.x;
    nextRotation.y = fn_1_1494(previousRotation.y, nextRotation.y, 10.0f);
    nextRotation.z = previousRotation.z;
    Hu3DModelPosSetV(modelId, &nextPosition);
    Hu3DModelRotSetV(modelId, &nextRotation);
}

/* Smooths a model toward its final Y rotation after a menu movement. */
static inline void fn_1_1C14_inlined(HU3D_MODELID modelId, float rotY)
{
    HuVecF previousRotation;
    HuVecF nextRotation;

    Hu3DModelRotGet(modelId, &previousRotation);
    nextRotation.y = rotY;
    if (previousRotation.y - nextRotation.y > 180.0f) {
        previousRotation.y -= 360.0f;
    } else if (previousRotation.y - nextRotation.y < -180.0f) {
        previousRotation.y += 360.0f;
    }
    nextRotation.x = previousRotation.x;
    nextRotation.y = fn_1_1494(previousRotation.y, nextRotation.y, 10.0f);
    nextRotation.z = previousRotation.z;
    Hu3DModelRotSetV(modelId, &nextRotation);
}

/* Updates straight model travel and then settles its final facing. */
static inline void fn_1_1D54_inlined(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY)
{
    if (time <= duration) {
        fn_1_18AC_inlined(modelId, start, end, time, duration);
    } else {
        fn_1_1C14_inlined(modelId, rotY);
    }
}

/* Guide callback: returns to the menu position, sets main motion speed to 1.5, then restores
 * speed 1 and the idle animation. */
void fn_1_595C(OMOBJ *obj)
{
    HuVecF path[2] = {
        { 0.0f, 0.0f, 0.0f },
        { -325.0f, 0.0f, 100.0f },
    };
    float time = obj->work[0];

    fn_1_1D54_inlined(
        obj->mdlId[0], &path[0], &path[1], time, 30.0f, 15.0f);
    Hu3DMotionSpeedSet(obj->mdlId[0], 1.5f);
    if (++obj->work[0] > 60) {
        Hu3DMotionSpeedSet(obj->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            0.0f, 15.0f, HU3D_MOTATTR_LOOP);
        obj->objFunc = NULL;
    }
}

/* Begins the guide's return movement when its current position is far enough to the right. */
s32 fn_1_5EF4(void)
{
    Point3d position;
    OMOBJ *obj;

    obj = lbl_1_bss_C;
    Hu3DModelPosGet(obj->mdlId[0], &position);
    if (position.x < -100.0f) {
        return 0;
    }
    obj->work[0] = 0;
    obj->objFunc = fn_1_595C;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    return 1;
}

/* Guide object initializer: loads its model and motions and sets the pose for the current entry
 * event. */
void fn_1_5FA0(OMOBJ *object)
{
    omSetStatBit(object, OM_STAT_MODELPAUSE);
    object->mdlId[0] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 47), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[0] =
        Hu3DJointMotion(object->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 48), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[1] =
        Hu3DJointMotion(object->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 49), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[2] =
        Hu3DJointMotion(object->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 50), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[3] =
        Hu3DJointMotion(object->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 51), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[4] =
        Hu3DJointMotion(object->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 52), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[5] =
        Hu3DJointMotion(object->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 54), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[6] =
        Hu3DJointMotion(object->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 53), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[7] =
        Hu3DJointMotion(object->mdlId[0],
                        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 55), HU_MEMNUM_OVL, HEAP_MODEL));
    if (lbl_1_bss_28 == 0) {
        Hu3DModelPosSet(object->mdlId[0], 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(object->mdlId[0], 0.0f, 0.0f, 0.0f);
    } else {
        Hu3DModelPosSet(object->mdlId[0], -325.0f, 0.0f, 100.0f);
        Hu3DModelRotSet(object->mdlId[0], 0.0f, 15.0f, 0.0f);
    }
    Hu3DModelScaleSet(object->mdlId[0], 1.25f, 1.25f, 1.25f);
    Hu3DModelLayerSet(object->mdlId[0], 1);
    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(object->mdlId[0]);
    object->objFunc = NULL;
}

/* Releases the guide model and its tracked motions during menu teardown. */
void fn_1_62B4(OMOBJ *object)
{
    if (object) {
        Hu3DMotionKill(object->mtnId[0]);
        Hu3DMotionKill(object->mtnId[1]);
        Hu3DMotionKill(object->mtnId[2]);
        Hu3DMotionKill(object->mtnId[3]);
        Hu3DModelKill(*object->mdlId);
        omDelObjEx(lbl_1_bss_0, object);
    }
    /* The callback is cleared after the object manager's deletion call. */
    object->objFunc = NULL;
}

/* Evaluates a quadratic path component for the selected character's departure. */
static inline f32 fn_1_10AC_inlined_inlined(f32 start, f32 control, f32 end, f32 progress)
{
    f32 remainingProgress;

    remainingProgress = 1.0f - progress;
    return (end * (progress * progress)) +
        ((start * (remainingProgress * remainingProgress)) +
         (2.0f * (control * (remainingProgress * progress))));
}

/* Evaluates the selected character's three-dimensional departure path. */
static inline void fn_1_1108_inlined_inlined(Point3d *destination, const Point3d *start,
                                             const Point3d *control, const Point3d *end,
                                             f32 progress)
{
    destination->x = fn_1_10AC_inlined_inlined(start->x, control->x, end->x, progress);
    destination->y = fn_1_10AC_inlined_inlined(start->y, control->y, end->y, progress);
    destination->z = fn_1_10AC_inlined_inlined(start->z, control->z, end->z, progress);
}

/* Character object callback: follows departure curves, emits player-colored trails, and hides
 * finished models. */
void fn_1_633C(OMOBJ *obj)
{
    Point3d effectPosition;
    Point3d modelRotation;
    Point3d previousPosition;
    Point3d position;
    MDSING_CHARACTER_SELECTION *selection;
    MDSING_MOTION_PATH *path;
    s16 playerNo;
    s16 modelId;
    f32 curvedProgress;
    f32 progress;
    f32 elapsedFrames;
    f32 durationFrames;

    playerNo = 0;
    selection = &lbl_1_bss_1308[playerNo];
    while (playerNo < 2) {
        path = &lbl_1_bss_CF4[playerNo];
        if (path->state == 1) {
            durationFrames = path->duration;
            elapsedFrames = path->elapsed;
            modelId = obj->mdlId[selection->characterNo];
            progress = fn_1_16F0(0.0f, 1.0f, elapsedFrames, durationFrames);
            curvedProgress = progress;
            fn_1_1108_inlined_inlined(&position, &path->position, &path->controlPosition,
                                      &path->endPosition, curvedProgress);
            Hu3DModelPosGet(modelId, &previousPosition);
            Hu3DModelRotGet(modelId, &modelRotation);
            previousPosition.x -= position.x;
            previousPosition.z -= position.z;
            previousPosition.y =
                (f32) - (180.0 * (atan2((f64) previousPosition.x, -previousPosition.z) / M_PI));
            if ((modelRotation.y - previousPosition.y) > 180.0f) {
                modelRotation.y -= 360.0f;
            } else if ((modelRotation.y - previousPosition.y) < -180.0f) {
                modelRotation.y += 360.0f;
            }
            modelRotation.y = fn_1_1494(modelRotation.y, previousPosition.y, 10.0f);
            Hu3DModelPosSet(modelId, position.x, position.y, position.z);
            Hu3DModelRotSet(modelId, 0.0f, modelRotation.y, 0.0f);
            if ((path->elapsed += 1.0f) > path->duration) {
                path->state = 2;
                Hu3DModelAttrSet(obj->mdlId[selection->characterNo], HU3D_ATTR_DISPOFF);
            }
            Hu3DModelPosGet(obj->mdlId[selection->characterNo], &effectPosition);
            if (selection->isComputer == 0) {
                fn_1_34FA4(playerNo, &effectPosition, 1, selection->padNo);
            } else {
                fn_1_34FA4(playerNo, &effectPosition, 1, 4);
            }
        }
        playerNo += 1;
        selection += 1;
    }
    playerNo = 0;
    while (playerNo < 2) {
        path = &lbl_1_bss_CF4[playerNo];
        if (path->state == 2) {
            playerNo += 1;
            continue;
        }
        break;
    }
    if (playerNo == 2) {
        obj->objFunc = NULL;
    }
}

/* Starts the selected characters' curved departure and installs the per-frame updater. */
void fn_1_68C0(void)
{
    MDSING_CHARACTER_SELECTION *selection;
    OMOBJ *obj = lbl_1_bss_10;
    MDSING_MOTION_PATH *path;
    s16 playerNo;

    for (playerNo = 0, selection = &lbl_1_bss_1308[playerNo]; playerNo < 2;
         playerNo++, selection++) {
        path = &lbl_1_bss_CF4[playerNo];
        path->state = 1;
        path->elapsed = 0.0f;
        path->duration = 90.0f;
        Hu3DModelPosGet(obj->mdlId[selection->characterNo], &path->position);
        Hu3DModelPosGet(obj->mdlId[selection->characterNo], &path->controlPosition);
        path->controlPosition.x += 0.5f * path->controlPosition.x;
        path->controlPosition.y = 550.0f;
        path->controlPosition.z = 100.0f;
        Hu3DModelPosGet(obj->mdlId[selection->characterNo], &path->endPosition);
        path->endPosition.y = 225.0f;
        path->endPosition.z = -800.0f;
        Hu3DMotionShiftSet(obj->mdlId[selection->characterNo],
            obj->mtnId[(2 * selection->characterNo) + 1], 0.0f,
            10.0f, HU3D_MOTATTR_LOOP);
    }
    HuAudFXPlay(MSM_SE_MENU_19);
    obj->objFunc = fn_1_633C;
}

/* Character object callback: scales the two selected models into view over the configured frame
 * count. */
void fn_1_6A7C(OMOBJ *obj)
{
    s16 playerNo;
    MDSING_CHARACTER_SELECTION *selection;
    float scale;

    for (playerNo = 0, selection = &lbl_1_bss_1308[playerNo]; playerNo < 2;
         playerNo++, selection++) {
        scale = fn_1_16F0(0.0f, 1.0f, obj->work[0], obj->work[1]);
        Hu3DModelScaleSet(
            obj->mdlId[selection->characterNo], scale, scale, scale);
    }
    if ((obj->work[0] += 1.0f) > obj->work[1]) {
        obj->objFunc = NULL;
    }
}

/* Reveals the selected character models at their menu entries with effects around the scale-in. */
void fn_1_6C48(void)
{
    OMOBJ *obj = lbl_1_bss_10;
    MDSING_CHARACTER_SELECTION *selection;
    Vec position;
    s16 playerNo;

    for (playerNo = 0, selection = &lbl_1_bss_1308[playerNo]; playerNo < 2;
         playerNo++, selection++) {
        Hu3DModelPosGet(lbl_1_bss_E74[selection->characterNo].modelId, &position);
        if (selection->isComputer != 0) {
            fn_1_344B4(playerNo, &position, 4, 0);
        } else {
            fn_1_344B4(playerNo, &position, selection->padNo, 0);
        }
    }
    for (playerNo = 0, selection = &lbl_1_bss_1308[playerNo]; playerNo < 2;
         playerNo++, selection++) {
        Hu3DModelPosGet(lbl_1_bss_E74[selection->characterNo].modelId, &position);
        position.y -= 70.0f;
        Hu3DModelPosSetV(obj->mdlId[selection->characterNo], &position);
        Hu3DModelScaleSet(obj->mdlId[selection->characterNo], 0.0f,
            0.0f, 0.0f);
        Hu3DModelAttrReset(obj->mdlId[selection->characterNo], HU3D_ATTR_DISPOFF);
    }
    obj->work[0] = 0;
    obj->work[1] = 15;
    obj->objFunc = fn_1_6A7C;
    HuPrcSleep(15);
    HuAudFXPlay(MSM_SE_MENU_18);
    for (playerNo = 0, selection = &lbl_1_bss_1308[playerNo]; playerNo < 2;
         playerNo++, selection++) {
        Hu3DModelPosGet(lbl_1_bss_E74[selection->characterNo].modelId, &position);
        if (selection->isComputer != 0) {
            fn_1_344B4(playerNo, &position, 4, 1);
        } else {
            fn_1_344B4(playerNo, &position, selection->padNo, 1);
        }
    }
}

/* Play character movement sounds at the listed motion frames during the lineup callback. */
void fn_1_6E94(OMOBJ *obj)
{
    MDSING_MOTION_SOUND_FRAME_TABLE soundFrames = {
        { 15, 29, -1, -1, 5, 19, -1, -1, 5, 22, -1, -1, 12, 28, -1, -1,
          18, 35, -1, -1, 0, 16, -1, -1, 12, 28, -1, -1, 14, 30, -1, -1,
          10, 32, -1, -1, 16, 32, -1, -1, 14, 30, -1, -1, 16, 32, -1, -1,
          16, 32, -1, -1, 16, 32, -1, -1 },
    };
    s16 characterNo = lbl_1_bss_22;
    f32 motionTime = Hu3DMotionTimeGet(obj->mdlId[characterNo]);
    s16 frameNo;

    for (frameNo = 0; frameNo < 3; frameNo++) {
        if (motionTime == soundFrames.frames[frameNo + (characterNo * 4)]) {
            CharFXPlay((s32)characterNo, CHARSEID(4));
        }
    }
}

/* Load the character lineup and its motions before the Solo menu sequences use it. */
void fn_1_7134(OMOBJ *obj)
{
    s16 modelNo;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (modelNo = 0; modelNo < 11; modelNo++) {
        obj->mdlId[modelNo] = Hu3DModelCreate(HuDataSelHeapReadNum(
            modelNo + DATANUM(DATA_mdsing, 56), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[modelNo * 2] = Hu3DJointMotion(obj->mdlId[modelNo],
            HuDataSelHeapReadNum(modelNo + DATANUM(DATA_mdsing, 67), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[(modelNo * 2) + 1] = Hu3DJointMotion(obj->mdlId[modelNo],
            HuDataSelHeapReadNum(modelNo + DATANUM(DATA_mdsing, 78), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(obj->mdlId[modelNo], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(obj->mdlId[modelNo], 1);
        Hu3DMotionShiftSet(obj->mdlId[modelNo], obj->mtnId[modelNo * 2],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelShadowSet(obj->mdlId[modelNo]);
    }
    if (lbl_1_bss_20 == 1) {
        modelNo = lbl_1_bss_22;
        obj->mtnId[35] = Hu3DJointMotion(obj->mdlId[modelNo],
            HuDataSelHeapReadNum(modelNo + DATANUM(DATA_mdsing, 89), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[36] = Hu3DJointMotion(obj->mdlId[modelNo],
            HuDataSelHeapReadNum(modelNo + DATANUM(DATA_mdsing, 100), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[37] = Hu3DJointMotion(obj->mdlId[modelNo],
            HuDataSelHeapReadNum(modelNo + DATANUM(DATA_mdsing, 111), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[38] = Hu3DJointMotion(obj->mdlId[modelNo],
            HuDataSelHeapReadNum(modelNo + DATANUM(DATA_mdsing, 122), HU_MEMNUM_OVL, HEAP_MODEL));
    }
    obj->objFunc = NULL;
}

/* Release the lineup's character models and motions when its menu object is removed. */
void fn_1_73D4(OMOBJ *obj)
{
    s16 characterNo;

    if (obj) {
        characterNo = 0;
        while (characterNo < 11) {
            Hu3DMotionKill(obj->mtnId[characterNo * 2]);
            Hu3DMotionKill(obj->mtnId[(characterNo * 2) + 1]);
            Hu3DModelKill(obj->mdlId[characterNo]);
            characterNo += 1;
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* Fade the board title sprite groups out over ten updates after leaving their display. */
void fn_1_7484(OMOBJ *obj)
{
    f32 alpha;

    alpha = fn_1_16F0(1.0f, 0.0f, obj->work[0], 10.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_1292[0], alpha);
    HuSprGrpTPLvlSet(lbl_1_bss_1292[7], alpha);

    if (++obj->work[0] > 10) {
        fn_1_47F4(lbl_1_bss_1292[0], HUSPR_ATTR_DISPOFF);
        fn_1_47F4(lbl_1_bss_1292[7], HUSPR_ATTR_DISPOFF);
        obj->objFunc = NULL;
    }
}

/* Show the current board title on return, or the full title group on initial entry. */
void fn_1_7668(void) {
    HUSPR_GROUP *group;
    HUSPR_GROUPID groupId;
    s16 memberNo;
    s16 boardMemberNo;
    s32 boardNo;

    if ((s32) lbl_1_bss_28 != 0) {
        HuSprGrpPosSet(lbl_1_bss_1292[7], 288.0f, 120.0f);
        boardNo = GwSystem.boardNo;
        boardMemberNo = boardNo - 6;
        HuSprAttrReset(lbl_1_bss_1292[7], boardMemberNo, HUSPR_ATTR_DISPOFF);
        HuSprGrpDrawNoSet(lbl_1_bss_1292[7], 64);
        return;
    }
    HuSprGrpPosSet(lbl_1_bss_1292[0], 288.0f, 120.0f);
    groupId = lbl_1_bss_1292[0];
    group = &HuSprGrpData[groupId];
    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, HUSPR_ATTR_DISPOFF);
    }
}

/* Start the board title fade by installing its update callback. */
void fn_1_778C(void)
{
    OMOBJ *obj = lbl_1_bss_4;

    obj->work[0] = 0;
    obj->objFunc = fn_1_7484;
}

/* Load the three board models and show the appropriate title group during menu setup. */
void fn_1_77C0(OMOBJ *obj) {
    HUSPR_GROUP *group;
    s16 groupId;
    s16 memberNo;
    s16 modelNo;
    s16 boardMemberNo;
    s32 boardNo;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    modelNo = 0;
    while (modelNo < 3) {
        obj->mdlId[modelNo] = Hu3DModelCreate(
            HuDataSelHeapReadNum(modelNo + DATANUM(DATA_mdsing, 0), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[modelNo] = Hu3DMotionIDGet(obj->mdlId[modelNo]);
        Hu3DModelLayerSet(obj->mdlId[modelNo], 1);
        Hu3DMotionShiftSet(obj->mdlId[modelNo], obj->mtnId[modelNo], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelShadowMapSet(obj->mdlId[modelNo]);
        modelNo += 1;
    }
    if (lbl_1_bss_28 != 0) {
        HuSprGrpPosSet(lbl_1_bss_1292[7], 288.0f, 120.0f);
        boardNo = GwSystem.boardNo;
        boardMemberNo = boardNo - 6;
        HuSprAttrReset(lbl_1_bss_1292[7], boardMemberNo, HUSPR_ATTR_DISPOFF);
        HuSprGrpDrawNoSet(lbl_1_bss_1292[7], 64);
    } else {
        HuSprGrpPosSet(lbl_1_bss_1292[0], 288.0f, 120.0f);
        groupId = lbl_1_bss_1292[0];
        group = &HuSprGrpData[groupId];
        memberNo = 0;
        while (memberNo < group->sprNum) {
            HuSprAttrReset(groupId, memberNo, HUSPR_ATTR_DISPOFF);
            memberNo += 1;
        }
    }
    obj->objFunc = NULL;
}

/* Release the board models and their motions when the menu object is removed. */
void fn_1_79D0(OMOBJ *obj)
{
    s16 modelNo;

    if (obj) {
        for (modelNo = 0; modelNo < 3; modelNo++) {
            Hu3DMotionKill(obj->mtnId[modelNo]);
            Hu3DModelKill(obj->mdlId[modelNo]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* Select a top-of-screen heading sprite and start its ten-frame fade in. */
void fn_1_7A60(s16 memberNo)
{
    OMOBJ *obj = lbl_1_bss_8;

    HuSprGrpPosSet(lbl_1_bss_1292[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_1292[1], 0.0f);
    fn_1_47F4(lbl_1_bss_1292[1], HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(
        lbl_1_bss_1292[1], memberNo, HUSPR_ATTR_DISPOFF);
    obj->work[2] = 1;
    obj->work[3] = 0;
}

/* Start the heading sprite's ten-frame fade out. */
void fn_1_7B64(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[2] = 2;
    obj->work[3] = 0;
}

/* Reset the heading group's position and transparency before it is displayed. */
void fn_1_7B94(void)
{
    HuSprGrpPosSet(lbl_1_bss_1292[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_1292[1], 0.0f);
}

/* Advance the heading group's requested fade once per object update. */
void fn_1_7BF4(OMOBJ *obj)
{
    float alpha;

    switch (obj->work[2]) {
        case 1:
            alpha = fn_1_16F0(
                0.0f, 1.0f, obj->work[3], 10.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_1292[1], alpha);
            if (++obj->work[3] > 10) {
                obj->work[2] = 0;
            }
            break;
        case 2:
            alpha = fn_1_16F0(
                1.0f, 0.0f, obj->work[3], 10.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_1292[1], alpha);
            if (++obj->work[3] > 10) {
                obj->work[2] = 0;
            }
            break;
    }
}

/* Change the background texture animations for the selected menu entry and start its fade. */
void fn_1_7E3C(s16 index)
{
    OMOBJ *obj;
    obj = lbl_1_bss_8;
    Hu3DAnimAnimSet(obj->mtnId[2], lbl_1_bss_12A4[index * 2 + 3]);
    Hu3DAnimAnimSet(obj->mtnId[3], lbl_1_bss_12A4[index * 2 + 4]);
    obj->work[0] = 1;
    obj->work[1] = 0;
}

/* Start the background model's reverse fade and the heading group's fade out together. */
void fn_1_7EE0(void)
{
    OMOBJ *modelObject;
    OMOBJ *headingObject;
    modelObject = lbl_1_bss_8;
    modelObject->work[0] = 2;
    modelObject->work[1] = 0;
    headingObject = lbl_1_bss_8;
    headingObject->work[2] = 2;
    headingObject->work[3] = 0;
}

/* Advance the heading fade from the rotating background's update callback. */
static inline void fn_1_89E0_inlined(OMOBJ *obj)
{
    float alpha;

    switch (obj->work[2]) {
        case 1:
            alpha = fn_1_16F0(
                0.0f, 1.0f, obj->work[3], 10.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_1292[1], alpha);
            if (++obj->work[3] > 10) {
                obj->work[2] = 0;
            }
            break;
        case 2:
            alpha = fn_1_16F0(
                1.0f, 0.0f, obj->work[3], 10.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_1292[1], alpha);
            if (++obj->work[3] > 10) {
                obj->work[2] = 0;
            }
            break;
    }
}

/* Rotate the menu background and advance its model and heading fades each frame. */
void fn_1_7F34(OMOBJ *obj)
{
    HuVecF rotation;
    float modelAlpha;
    s16 modelNo;

    for (modelNo = 0; modelNo < 2; modelNo++) {
        Hu3DModelRotGet(obj->mdlId[modelNo], &rotation);
        rotation.z -= 0.1f;
        if (rotation.z < 0.0f) {
            rotation.z += 360.0f;
        }
        Hu3DModelRotSetV(obj->mdlId[modelNo], &rotation);
    }
    switch (obj->work[0]) {
        case 1:
            modelAlpha = fn_1_16F0(
                1.0f, 0.0f, obj->work[1], 30.0f);
            Hu3DModelTPLvlSet(obj->mdlId[0], modelAlpha);
            if (++obj->work[1] > 30) {
                obj->work[0] = 0;
            }
            break;
        case 2:
            modelAlpha = fn_1_16F0(
                0.0f, 1.0f, obj->work[1], 30.0f);
            Hu3DModelTPLvlSet(obj->mdlId[0], modelAlpha);
            if (++obj->work[1] > 30) {
                obj->work[0] = 0;
            }
            break;
    }
    fn_1_89E0_inlined(obj);
}

/* Create the rotating menu background and its texture animations during menu setup. */
void fn_1_842C(OMOBJ *obj)
{
    s16 modelNo;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (modelNo = 0; modelNo < 2; modelNo++) {
        if (modelNo == 0) {
            obj->mdlId[modelNo] = Hu3DModelCreate(
                HuDataSelHeapReadNum(
                    DATANUM(DATA_mdsing, 3), HU_MEMNUM_OVL,
                    HEAP_MODEL));
        } else {
            obj->mdlId[modelNo] = Hu3DModelLink(obj->mdlId[0]);
        }
        Hu3DModelPosSet(
            obj->mdlId[modelNo], 0.0f, -100.0f, -850.0f - modelNo);
        Hu3DModelRotSet(obj->mdlId[modelNo], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(obj->mdlId[modelNo], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(obj->mdlId[modelNo], 0);
        obj->mtnId[2 * modelNo] = Hu3DAnimCreate(
            lbl_1_bss_12A4[1], obj->mdlId[modelNo], lbl_1_data_ABA);
        obj->mtnId[(2 * modelNo) + 1] = Hu3DAnimCreate(
            lbl_1_bss_12A4[2], obj->mdlId[modelNo], lbl_1_data_AC1);
    }
    HuSprGrpPosSet(lbl_1_bss_1292[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_1292[1], 0.0f);
    obj->objFunc = fn_1_7F34;
}

/* Release the background texture animations and models when their object is removed. */
void fn_1_8670(OMOBJ *obj)
{
    s16 modelNo;

    if (obj) {
        for (modelNo = 0; modelNo < 2; modelNo++) {
            Hu3DAnimKill(obj->mtnId[2 * modelNo]);
            Hu3DAnimKill(obj->mtnId[(2 * modelNo) + 1]);
            obj->mtnId[2 * modelNo] = -1;
            obj->mtnId[(2 * modelNo) + 1] = -1;
        }
        for (modelNo = 1; modelNo >= 0; modelNo--) {
            Hu3DModelKill(obj->mdlId[modelNo]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* Show a player's selection marker with the requested bank and a fixed target scale of 0.85. */
void fn_1_876C(s16 playerNo, s16 bank)
{
    lbl_1_bss_CDC[playerNo].x = 0.85f;
    fn_1_4874(
        lbl_1_bss_1292[playerNo + 4], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_1292[playerNo + 4], 0, bank);
}

/* Set a player's selection marker to shrink away during subsequent updates. */
void fn_1_8858(s16 teamPlayerNo)
{
    lbl_1_bss_CDC[teamPlayerNo].x = 0.0f;
}

/* Keep player markers beside their selected models and smooth their scale each frame. */
void fn_1_8880(OMOBJ *obj)
{
    Point3d modelPosition;
    Point3d screenPosition;
    MDSING_CHARACTER_SELECTION *selectionCursor;
    s16 playerNo;

    playerNo = 0;
    selectionCursor = lbl_1_bss_1308;
    while (playerNo < 2) {
        Hu3DModelPosGet(lbl_1_bss_E74[lbl_1_bss_1308[playerNo].characterNo].modelId,
                        &modelPosition);
        Hu3D3Dto2D(&modelPosition, 1, &screenPosition);
        HuSprGrpPosSet(lbl_1_bss_1292[playerNo + 4], screenPosition.x - 18.0f,
                       screenPosition.y - 23.0f);
        HuSprAttrReset(lbl_1_bss_1292[playerNo + 4], 0, HUSPR_ATTR_LINEAR);
        lbl_1_bss_CDC[playerNo].y = fn_1_1494(
            lbl_1_bss_CDC[playerNo].y, lbl_1_bss_CDC[playerNo].x, 3.0f);
        HuSprGrpScaleSet(lbl_1_bss_1292[playerNo + 4], lbl_1_bss_CDC[playerNo].y,
                         lbl_1_bss_CDC[playerNo].y);
        playerNo += 1;
        selectionCursor += 1;
    }
}

/* Initialize the player marker scales and install their tracking callback. */
void fn_1_8A80(OMOBJ *obj)
{
    s16 playerNo;

    playerNo = 0;
    while (playerNo < 2) {
        lbl_1_bss_CDC[playerNo].x = 0.0f;
        lbl_1_bss_CDC[playerNo].y = 0.0f;
        lbl_1_bss_CDC[playerNo].z = 0.0f;
        playerNo += 1;
    }
    obj->objFunc = fn_1_8880;
}

void fn_1_8B24(void)
{
}

/* Place a menu sprite at a projected model position plus screen-space offsets. */
void fn_1_8B28(
    s16 memberNo, HuVecF *worldPosition, float xOffset, float yOffset)
{
    HuVecF screenPosition = { 0.0f, 0.0f, 0.0f };

    if (worldPosition) {
        Hu3D3Dto2D(worldPosition, 1, &screenPosition);
    }
    HuSprPosSet(lbl_1_bss_1292[2], memberNo,
        screenPosition.x + xOffset, screenPosition.y + yOffset);
}

/* Show a menu sprite at double size so its subsequent updates settle it to normal size. */
void fn_1_8BC0(
    s16 memberNo, HuVecF *worldPosition, float xOffset, float yOffset)
{
    fn_1_8B28(memberNo, worldPosition, xOffset, yOffset);
    HuSprScaleSet(lbl_1_bss_1292[2], memberNo, 2.0f, 2.0f);
    HuSprAttrReset(lbl_1_bss_1292[2], memberNo, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_C94[memberNo] = 2.0f;
    lbl_1_data_AC8[memberNo] = 1;
}

/* Hide a menu sprite and clear its active state when its entry is dismissed. */
void fn_1_8CE4(s16 memberNo)
{
    HuSprAttrSet(lbl_1_bss_1292[2], memberNo, HUSPR_ATTR_DISPOFF);
    lbl_1_data_AC8[memberNo] = 0;
}

/* Enlarge a menu sprite for selection feedback, playing a sound if it is active. */
void fn_1_8D40(s16 memberNo)
{
    lbl_1_bss_C94[memberNo] = 2.0f;
    if (lbl_1_data_AC8[memberNo] == 1) {
        HuAudFXPlay(MSM_SE_CMN_01);
    }
}

/* Ease all six menu sprites back to normal size during menu updates. */
void fn_1_8DB4(void)
{
    s16 memberNo;

    memberNo = 0;
    while (memberNo < 6) {
        lbl_1_bss_C94[memberNo] = fn_1_1494(lbl_1_bss_C94[memberNo], 1.0f, 5.0f);
        HuSprScaleSet(lbl_1_bss_1292[2], memberNo,
            lbl_1_bss_C94[memberNo], lbl_1_bss_C94[memberNo]);
        memberNo++;
    }
}

/* Hide and reset all six menu sprites when their menu display is cleared. */
void fn_1_8ED8(void)
{
    s16 memberNo;
    memberNo = 0;
    while (memberNo < 6) {
        HuSprAttrSet(lbl_1_bss_1292[2], memberNo, HUSPR_ATTR_DISPOFF);
        lbl_1_data_AC8[memberNo] = 0;
        /* Each sprite state is cleared twice before advancing to the next entry. */
        lbl_1_data_AC8[memberNo] = 0;
        memberNo += 1;
    }
}

/* Set a model's destination offset from the supplied menu position. */
void fn_1_8F64(s16 modelNo, Vec *position)
{
    lbl_1_bss_CAC[modelNo].x = position->x - 50.0f;
    lbl_1_bss_CAC[modelNo].y = 50.0f + position->y;
    lbl_1_bss_CAC[modelNo].z = 50.0f + position->z;
}

/* Show a menu model and its paired marker at the start of their settling animation. */
void fn_1_8FEC(s32 modelNo, HuVecF *position, s16 bank)
{
    OMOBJ *obj = lbl_1_bss_18.object;

    Hu3DModelPosSet(
        obj->mdlId[(s16)modelNo], position->x, position->y, position->z);
    Hu3DModelRotSet(
        obj->mdlId[(s16)modelNo], 0.0f, 0.0f, 180.0f);
    Hu3DModelScaleSet(
        obj->mdlId[(s16)modelNo], 1.0f, 1.0f, 1.0f);
    fn_1_8F64(modelNo, position);
    Hu3DModelAttrReset(
        obj->mdlId[(s16)modelNo], HU3D_ATTR_DISPOFF);
    HuSprScaleSet(lbl_1_bss_1292[3], modelNo, 0.8f, 0.8f);
    HuSprAttrReset(lbl_1_bss_1292[3], modelNo, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_1292[3], modelNo, bank);
}

/* Hide a menu model and its paired marker when that entry is dismissed. */
void fn_1_91C0(s32 modelNo)
{
    OMOBJ *obj = lbl_1_bss_18.object;

    Hu3DModelAttrSet(obj->mdlId[(s16)modelNo], HU3D_ATTR_DISPOFF);
    HuSprScaleSet(lbl_1_bss_1292[3], modelNo, 0.8f, 0.8f);
    HuSprAttrSet(lbl_1_bss_1292[3], modelNo, HUSPR_ATTR_DISPOFF);
}

/* Ease the six menu sprite scales back to normal from the model update callback. */
static inline void fn_1_AB64_inlined(void)
{
    s16 memberNo;

    for (memberNo = 0; memberNo < 6; memberNo++) {
        lbl_1_bss_C94[memberNo] =
            fn_1_1494(lbl_1_bss_C94[memberNo], 1.0f, 5.0f);
        HuSprScaleSet(lbl_1_bss_1292[2], memberNo,
            lbl_1_bss_C94[memberNo], lbl_1_bss_C94[memberNo]);
    }
}

/* Settle the four menu models toward their targets and keep their markers aligned each frame. */
void fn_1_9258(OMOBJ *obj)
{
    HuVecF modelTransform;
    HuVecF screenPosition;
    s16 modelNo;

    for (modelNo = 0; modelNo < 4; modelNo++) {
        Hu3DModelPosGet(obj->mdlId[modelNo], &modelTransform);
        fn_1_14C4(&modelTransform, &lbl_1_bss_CAC[modelNo], 3.0f);
        Hu3DModelPosSetV(obj->mdlId[modelNo], &modelTransform);
        Hu3D3Dto2D(&modelTransform, 1, &screenPosition);
        HuSprPosSet(lbl_1_bss_1292[3], (s16)modelNo, screenPosition.x, screenPosition.y);
        HuSprScaleSet(lbl_1_bss_1292[3], (s16)modelNo, 0.8f, 0.8f);

        Hu3DModelRotGet(obj->mdlId[modelNo], &modelTransform);
        modelTransform.z = fn_1_1494(modelTransform.z, 0.0f, 3.0f);
        Hu3DModelRotSetV(obj->mdlId[modelNo], &modelTransform);

        Hu3DModelScaleGet(obj->mdlId[modelNo], &modelTransform);
        modelTransform.x = modelTransform.y = modelTransform.z =
            fn_1_1494(modelTransform.x, 0.4f, 3.0f);
        Hu3DModelScaleSetV(obj->mdlId[modelNo], &modelTransform);
    }
    fn_1_AB64_inlined();
}

/* Hide and reset menu sprites while their associated model object is initialized. */
static inline void fn_1_AC88_inlined_inlined(void)
{
    s16 memberNo;

    for (memberNo = 0; memberNo < 6; memberNo++) {
        fn_1_8CE4(memberNo);
        lbl_1_data_AC8[memberNo] = 0;
    }
}

/* Create the four linked menu models and install their settling callback. */
void fn_1_9718(OMOBJ *obj)
{
    s16 modelNo;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (modelNo = 0; modelNo < 4; modelNo++) {
        if (modelNo == 0) {
            obj->mdlId[modelNo] = Hu3DModelCreate(
                HuDataSelHeapReadNum(
                    DATANUM(DATA_mdsing, 14), HU_MEMNUM_OVL,
                    HEAP_MODEL));
            obj->mtnId[modelNo] = Hu3DMotionIDGet(obj->mdlId[0]);
        } else {
            obj->mdlId[modelNo] = Hu3DModelLink(obj->mdlId[0]);
        }
        Hu3DModelLayerSet(obj->mdlId[modelNo], 1);
        Hu3DMotionShiftSet(obj->mdlId[modelNo], obj->mtnId[0],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        fn_1_91C0(modelNo);
    }
    fn_1_AC88_inlined_inlined();
    obj->objFunc = fn_1_9258;
}

/* Release the linked menu models and their shared motion when their object is removed. */
void fn_1_9910(OMOBJ *obj)
{
    s16 modelIndex;
    if (obj) {
        Hu3DMotionKill(*obj->mtnId);
        modelIndex = 3;
        while (modelIndex >= 0) {
            Hu3DModelKill(obj->mdlId[modelIndex]);
            modelIndex -= 1;
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    /* This local reset does not change the caller's stored object pointer. */
    obj = NULL;
}

/* Follow navigation priorities recursively, skipping characters already selected by another
 * entry. */
s16 fn_1_9998(s16 currentCharacter, s16 direction, s16 fallbackDirection, s16 lastDirection,
              MDSING_CHARACTER_NEIGHBORS *entries, s16 characterCount)
{
    s16 nextCharacter;
    s16 candidateNo;

    if (direction == -1) {
        return -1;
    }
    nextCharacter = entries[currentCharacter].neighborCharacter[direction];
    if (nextCharacter == -1) {
        nextCharacter = fn_1_9998(currentCharacter, fallbackDirection, lastDirection, -1, entries,
                                  characterCount);
    }
    candidateNo = 0;
    while (candidateNo < characterCount) {
        if ((currentCharacter != candidateNo) && (entries[candidateNo].selectionState != -1) &&
            (nextCharacter == candidateNo)) {
            nextCharacter = fn_1_9998(nextCharacter, direction, fallbackDirection, lastDirection,
                                      entries, characterCount);
            if (nextCharacter == -1) {
                nextCharacter = fn_1_9998(currentCharacter, fallbackDirection, lastDirection, -1,
                                          entries, characterCount);
            }
        }
        candidateNo += 1;
    }
    return nextCharacter;
}

const MDMinNeighborTable lbl_1_rodata_204 = { {
    { 2, 1, 3 },
    { 6, 7, 5 },
    { 0, 7, 1 },
    { 4, 5, 3 },
    { 1, 2, 0 },
    { 3, 2, 4 },
    { 7, 6, 0 },
    { 5, 6, 4 },
    { 1, 0, 2 },
    { 7, 0, 6 },
    { 3, 4, 2 },
    { 5, 4, 6 }
} };

/* Look up the three direction priorities for a character-selection navigation command. */
s16 fn_1_9D80(s16 currentCharacter, MDSING_CHARACTER_NEIGHBORS *entries,
              s16 navigationCommand, s16 characterCount)
{
    MDMinNeighborTable neighbors = lbl_1_rodata_204;
    return fn_1_9998(currentCharacter, neighbors.directionOrder[navigationCommand][0],
                     neighbors.directionOrder[navigationCommand][1],
                     neighbors.directionOrder[navigationCommand][2], entries, characterCount);
}

/* Move a player's character selection in response to repeated directional input. */
s16 fn_1_9E28(s16 padNo, s16 *currentCharacter,
    MDSING_CHARACTER_NEIGHBORS *entries, s16 characterCount)
{
    s16 nextCharacter = 0;
    s16 directionNo = 0;
    HuVecF direction = { 0.0f, 0.0f, 0.0f };

    if (HuPadDStkRep[padNo] & PAD_BUTTON_UP) {
        direction.y = 1.0f;
    } else if (HuPadDStkRep[padNo] & PAD_BUTTON_DOWN) {
        direction.y = -1.0f;
    }
    if (HuPadDStkRep[padNo] & PAD_BUTTON_LEFT) {
        direction.x = -1.0f;
    } else if (HuPadDStkRep[padNo] & PAD_BUTTON_RIGHT) {
        direction.x = 1.0f;
    }
    if (HuAbs(direction.x) < 0.25f
        && HuAbs(direction.y) < 0.25f) {
        return FALSE;
    }

    direction.z = sqrtf(
        (direction.x * direction.x) + (direction.y * direction.y));
    direction.x /= direction.z;
    direction.y /= direction.z;
    if (HuAbs(direction.y) < 0.25f) {
        if (direction.x > 0.0f) {
            directionNo = 0;
        } else {
            directionNo = 1;
        }
    } else if (HuAbs(direction.x) < 0.25f) {
        if (direction.y > 0.0f) {
            directionNo = 2;
        } else {
            directionNo = 3;
        }
    } else if (HuAbs(direction.x) > HuAbs(direction.y)) {
        if (direction.x > 0.0f) {
            if (direction.y > 0.0f) {
                directionNo = 4;
            } else {
                directionNo = 5;
            }
        } else if (direction.y > 0.0f) {
            directionNo = 6;
        } else {
            directionNo = 7;
        }
    } else if (direction.y > 0.0f) {
        if (direction.x > 0.0f) {
            directionNo = 8;
        } else {
            directionNo = 9;
        }
    } else if (direction.x > 0.0f) {
        directionNo = 10;
    } else {
        directionNo = 11;
    }

    if ((nextCharacter = fn_1_9D80(*currentCharacter, entries, directionNo, characterCount)) !=
        -1) {
        entries[*currentCharacter].selectionState = -1;
        *currentCharacter = nextCharacter;
        entries[*currentCharacter].selectionState = 1;
        HuAudFXPlay(MSM_SE_CMN_01);
        return TRUE;
    }
    return FALSE;
}

/* Change one menu model texture animation, optionally selecting a particular bank. */
void fn_1_A3A8(s16 modelNo, s16 animNo, s16 dataNo, s16 bank)
{
    MDSING_MODEL *entry = &lbl_1_bss_E74[modelNo];

    Hu3DAnimAnimSet(entry->animationId[animNo], lbl_1_bss_11F4[dataNo]);
    if (bank != -1) {
        Hu3DAnimBankSet(entry->animationId[animNo], bank);
    }
}

/* Apply the same texture animation data to all four surfaces of a menu model. */
void fn_1_A450(s16 modelNo, s16 dataNo)
{
    s16 animationNo;

    for (animationNo = 0; animationNo < 4; animationNo++) {
        fn_1_A3A8(modelNo, animationNo, dataNo, -1);
    }
}

/* Spin and grow a menu model into view over the supplied animation interval. */
void fn_1_A4DC(
    s16 modelNo, s16 elapsedFrames, s16 durationFrames, HuVecF *position, float scale)
{
    MDSING_MODEL *entry = &lbl_1_bss_E74[modelNo];
    float animatedValue;

    if (elapsedFrames == 0) {
        if (lbl_1_bss_3C == 0) {
            HuAudFXPlay(MSM_SE_CMN_53);
            lbl_1_bss_3C = 1;
        }
        Hu3DModelPosSet(entry->modelId, position->x, position->y, position->z);
        Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(entry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    } else {
        lbl_1_bss_3C = 0;
    }
    animatedValue = fn_1_16F0(-540.0f, 0.0f, elapsedFrames, durationFrames);
    Hu3DModelRotSet(entry->modelId, 0.0f, animatedValue, 0.0f);
    animatedValue = fn_1_16F0(0.0f, scale, elapsedFrames, durationFrames);
    Hu3DModelScaleSet(entry->modelId, animatedValue, animatedValue, 1.0f);
}

/* Spin and shrink a menu model from its captured transform during a dismissal animation. */
void fn_1_A8E8(s16 modelNo, s16 elapsedFrames, s16 durationFrames)
{
    MDSING_MODEL *entry = &lbl_1_bss_E74[modelNo];
    float animatedValue;

    if (elapsedFrames == 0) {
        if (lbl_1_bss_3C == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_3C = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->position);
        Hu3DModelRotGet(entry->modelId, &entry->rotation);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_3C = 0;
    }
    animatedValue = fn_1_16F0(
        entry->rotation.y, entry->rotation.y - 540.0f, elapsedFrames, durationFrames);
    Hu3DModelRotSet(entry->modelId, 0.0f, animatedValue, 0.0f);
    animatedValue = fn_1_16F0(entry->scale.x, 0.0f, elapsedFrames, durationFrames);
    Hu3DModelScaleSet(entry->modelId, animatedValue, animatedValue, 1.0f);
    if (elapsedFrames <= durationFrames) {
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    }
}

/* Move and scale a menu model from its captured transform during a menu transition. */
void fn_1_ACB8(
    s16 modelNo, s16 elapsedFrames, s16 durationFrames, HuVecF *targetPosition, float scale)
{
    MDSING_MODEL *entry = &lbl_1_bss_E74[modelNo];
    HuVecF position;
    float animatedScale;

    if (elapsedFrames == 0) {
        Hu3DModelPosGet(entry->modelId, &entry->position);
        Hu3DModelRotGet(entry->modelId, &entry->rotation);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    }
    position.x = fn_1_16F0(entry->position.x, targetPosition->x, elapsedFrames, durationFrames);
    position.y = fn_1_16F0(entry->position.y, targetPosition->y, elapsedFrames, durationFrames);
    position.z = fn_1_16F0(entry->position.z, targetPosition->z, elapsedFrames, durationFrames);
    Hu3DModelPosSet(entry->modelId, position.x, position.y, position.z);
    animatedScale = fn_1_16F0(entry->scale.x, scale, elapsedFrames, durationFrames);
    Hu3DModelScaleSet(entry->modelId, animatedScale, animatedScale, 1.0f);
}

/* Turn a menu model over and change its texture after the turn passes halfway. */
void fn_1_B2A8(s16 modelNo, s16 dataNo, s16 dataOffset, s16 elapsedFrames,
    s16 durationFrames, s16 direction)
{
    MDSING_MODEL *entry = &lbl_1_bss_E74[modelNo];
    HuVecF rotation;

    if (elapsedFrames == 0 || elapsedFrames >= durationFrames) {
        Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
    }
    if (elapsedFrames >= durationFrames) {
        fn_1_A450(modelNo, dataOffset + dataNo);
    } else {
        Hu3DModelRotGet(entry->modelId, &rotation);
        rotation.y = fn_1_16F0(
            0.0f, 180.0f * direction, elapsedFrames, durationFrames);
        if (((rotation.y < 0.0f) ? -rotation.y : rotation.y) >= 90.0f) {
            fn_1_A450(modelNo, dataOffset + dataNo);
        }
        Hu3DModelRotSetV(entry->modelId, &rotation);
    }
}

/* Rotate a menu model back to face forward, wrapping its starting angle relative to the
 * reference. */
void fn_1_B630(s16 modelNo, float referenceAngle, s16 elapsedFrames, s16 durationFrames)
{
    MDSING_MODEL *entry = &lbl_1_bss_E74[modelNo];
    float rotationY;

    if (elapsedFrames == 0) {
        Hu3DModelRotGet(entry->modelId, &entry->rotation);
        if (entry->rotation.y - referenceAngle > 180.0f) {
            entry->rotation.y -= 360.0f;
        } else if (entry->rotation.y - referenceAngle < -180.0f) {
            entry->rotation.y += 360.0f;
        }
    }
    rotationY = fn_1_16F0(entry->rotation.y, 0.0f, elapsedFrames, durationFrames);
    Hu3DModelRotSet(entry->modelId, 0.0f, rotationY, 0.0f);
}

/* Signal the running menu animation's completion state to its waiting process. */
void fn_1_B898(s16 completionState)
{
    lbl_1_bss_24->mtnId[0] = completionState;
}

/* Read the menu animation state used by the waiting menu process. */
s16 fn_1_B8B0(void)
{
    return lbl_1_bss_24->mtnId[0];
}

/* Report whether the menu animation has reached its ten-frame threshold. */
s16 fn_1_B8C8(void)
{
    OMOBJ *obj = lbl_1_bss_24;

    if (obj->work[0] >= 10) {
        return TRUE;
    }
    return FALSE;
}

/* Start a menu animation callback and wait until it signals completion. */
void fn_1_B900(OMOBJ_FUNC callback)
{
    OMOBJ *obj = lbl_1_bss_24;

    lbl_1_bss_24->mtnId[0] = 0;
    obj->work[0] = 0;
    obj->work[1] = 0;
    obj->work[2] = 0;
    obj->work[3] = 0;
    obj->objFunc = callback;
    while (lbl_1_bss_24->mtnId[0] == 0) {
        HuPrcVSleep();
    }
}

const BITMAP_NAME_TABLE lbl_1_rodata_270 = { { lbl_1_data_AD4, lbl_1_data_ADC, lbl_1_data_AE4,
                                               lbl_1_data_AEC } };

/* Load menu card textures and linked models, then create their animation controller. */
void fn_1_B998(void)
{
    BITMAP_NAME_TABLE bitmapName = lbl_1_rodata_270;
    s16 entryNo;
    s16 animationNo;

    for (entryNo = 0; entryNo < 25; entryNo++) {
        lbl_1_bss_11F4[entryNo] = HuSprAnimRead(
            HuDataSelHeapReadNum(
                lbl_1_data_744[entryNo], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (entryNo = 0; entryNo < 16; entryNo++) {
        memset(&lbl_1_bss_E74[entryNo], 0, sizeof(LBL_1_BSS_E74_ENTRY));
        if (entryNo == 0) {
            lbl_1_bss_E74[entryNo].modelId = Hu3DModelCreate(
                HuDataSelHeapReadNum(
                    DATANUM(DATA_mdsing, 21), HU_MEMNUM_OVL,
                    HEAP_MODEL));
        } else {
            lbl_1_bss_E74[entryNo].modelId =
                Hu3DModelLink(lbl_1_bss_E74[0].modelId);
        }
        for (animationNo = 0; animationNo < 4; animationNo++) {
            lbl_1_bss_E74[entryNo].animationId[animationNo] = Hu3DAnimCreate(
                lbl_1_bss_11F4[0], lbl_1_bss_E74[entryNo].modelId,
                bitmapName.name[animationNo]);
        }
        Hu3DModelLayerSet(lbl_1_bss_E74[entryNo].modelId, 1);
        Hu3DModelAttrSet(
            lbl_1_bss_E74[entryNo].modelId, HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_24 =
        omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, NULL);
}

/* Stop the menu card controller and release its texture animations and models on teardown. */
void fn_1_BBC8(void)
{
    s16 modelNo;
    s16 animationNo;

    if (lbl_1_bss_24) {
        lbl_1_bss_24->mtnId[0] = -1;
        for (modelNo = 15; modelNo >= 0; modelNo--) {
            for (animationNo = 0; animationNo < 4; animationNo++) {
                Hu3DAnimKill(lbl_1_bss_E74[modelNo].animationId[animationNo]);
            }
            Hu3DModelKill(lbl_1_bss_E74[modelNo].modelId);
        }
    }
    lbl_1_bss_24 = NULL;
}

/* Rotate the highlighted character card by six degrees during selection updates. */
void fn_1_BCA4(s16 modelNo)
{
    HuVecF rotation;
    MDSING_MODEL *entry = &lbl_1_bss_E74[modelNo];

    Hu3DModelRotGet(entry->modelId, &rotation);
    rotation.y += 6.0f;
    if (rotation.y > 360.0f) {
        rotation.y -= 360.0f;
    }
    Hu3DModelRotSetV(entry->modelId, &rotation);
}

/* Return a character card to face forward when it is no longer highlighted. */
void fn_1_BD40(s16 modelNo)
{
    MDSING_MODEL *entry = &lbl_1_bss_E74[modelNo];

    Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
}

/* Handle human character selection and confirmation, then animate the highlighted cards each
 * frame. */
void fn_1_BDA8(OMOBJ *obj)
{
    MDSING_CHARACTER_SELECTION * selection;
    s16 playerNo;
    s16 matchingPlayerNo;
    s16 characterNo;
    playerNo = 0;
    selection = lbl_1_bss_1308;
    while (playerNo < 2)
    {
        if (selection->isComputer == 0 && selection->choiceState == 0)
        {
            if (obj->work[0]++ > 10)
            {
                if (fn_1_9E28(selection->padNo, & selection->characterNo, lbl_1_data_AF4, 11) != 0)
                {
                    fn_1_8F64(
                        playerNo,
                        (Vec *)&lbl_1_data_7A8[selection->characterNo + 10]);
                    obj->work[0] = 0;
                }
                else
                {
                    if (HuPadBtnDown[selection->padNo] & PAD_BUTTON_A)
                    {
                        if (lbl_1_bss_30 == 0 && selection->characterNo == 10)
                        {
                            HuAudFXPlay(MSM_SE_CMN_05);
                        }
                        else
                        {
                            CharFXPlay(selection->characterNo, CHARVOICEID(14));
                            selection->choiceState = 1;
                            fn_1_91C0(playerNo);
                            fn_1_876C(playerNo, selection->padNo);
                            fn_1_A3A8(
                                (s16)selection->characterNo, 0,
                                (s16)selection->characterNo, 1);
                            fn_1_A3A8(
                                (s16)selection->characterNo, 2,
                                (s16)selection->characterNo, 1);
                            fn_1_341B8(playerNo, & lbl_1_data_7A8[selection->characterNo + 10], 0);
                        }
                    }
                }
            }
        }
        playerNo++;
        selection++;
    }
    for (playerNo = 0; playerNo < 11; playerNo++)
    {
        matchingPlayerNo = 0;
        selection = lbl_1_bss_1308;
        while (matchingPlayerNo < 2)
        {
            if (selection->isComputer != 0 || selection->characterNo != playerNo ||
                selection->choiceState != 0) {
                matchingPlayerNo++;
                selection++;
                continue;
            }
            break;
        }
        if (matchingPlayerNo == 2)
        {
            fn_1_BD40(playerNo);
        }
        else
        {
            fn_1_BCA4(playerNo);
        }
    }
}

/* Show cursors for unconfirmed human players and begin their character-selection callback. */
void fn_1_C264(OMOBJ *obj)
{
    s16 playerNo;
    MDSING_CHARACTER_SELECTION *entry;

    for (playerNo = 0, entry = lbl_1_bss_1308; playerNo < 2; playerNo++, entry++) {
        if (entry->isComputer == 0 && entry->choiceState == 0) {
            lbl_1_data_AF4[entry->characterNo].selectionState = 1;
            fn_1_8FEC(
                playerNo, &lbl_1_data_7A8[entry->characterNo + 10], entry->padNo);
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_BDA8;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hide unconfirmed human cursors and stop character-card rotation when leaving selection. */
void fn_1_C4C8(OMOBJ *obj)
{
    s16 entryNo;
    MDSING_CHARACTER_SELECTION *entry;

    for (entryNo = 0, entry = lbl_1_bss_1308; entryNo < 2; entryNo++, entry++) {
        if (entry->isComputer == 0 && entry->choiceState == 0) {
            fn_1_91C0(entryNo);
        }
    }
    for (entryNo = 0; entryNo < 11; entryNo++) {
        fn_1_BD40(entryNo);
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Undo the first player's confirmation and reset that character card's selected appearance. */
void fn_1_C614(OMOBJ *obj)
{
    MDSING_CHARACTER_SELECTION *entry = &lbl_1_bss_1308[0];

    entry->choiceState = 0;
    lbl_1_bss_CDC[0].x = 0.0f;
    fn_1_A3A8((s16)entry->characterNo, 0, (s16)entry->characterNo, 0);
    fn_1_A3A8((s16)entry->characterNo, 2, (s16)entry->characterNo, 0);
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Let controller zero choose and confirm the computer character, updating card rotation each
 * frame. */
void fn_1_C724(OMOBJ *obj)
{
    s16 playerNo = 0;
    MDSING_CHARACTER_SELECTION *choice = lbl_1_bss_1308;

    while (playerNo < 2) {
        if (choice->isComputer == 1) {
            if (choice->choiceState == 0) {
                if (obj->work[0]++ > 10) {
                    if (fn_1_9E28(
                            0, &choice->characterNo, lbl_1_data_AF4, 11)) {
                        fn_1_8F64(
                            playerNo,
                            (Vec *)&lbl_1_data_7A8[choice->characterNo + 10]);
                        obj->work[0] = 0;
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                        if (lbl_1_bss_30 == 0 && choice->characterNo == 10) {
                            HuAudFXPlay(MSM_SE_CMN_05);
                        } else {
                            CharFXPlay(choice->characterNo, CHARVOICEID(14));
                            choice->choiceState = 1;
                            fn_1_91C0(playerNo);
                            fn_1_876C(playerNo, 4);
                            fn_1_A3A8(
                                (s16)choice->characterNo, 0,
                                (s16)choice->characterNo, 1);
                            fn_1_A3A8(
                                (s16)choice->characterNo, 2,
                                (s16)choice->characterNo, 1);
                            fn_1_341B8(
                                playerNo,
                                &lbl_1_data_7A8[choice->characterNo + 10],
                                4);
                        }
                    }
                }
            }
            break;
        }
        playerNo++;
        choice++;
    }

    playerNo = 0;
    for (; playerNo < 11; playerNo++) {
        if (choice->characterNo == playerNo && choice->choiceState == 0) {
            fn_1_BCA4(playerNo);
        } else {
            fn_1_BD40(playerNo);
        }
    }
}

/* Assign an available starting character to the computer and begin its selection callback. */
void fn_1_CB8C(OMOBJ *obj)
{
    MDSING_CHARACTER_SELECTION *entry;
    s16 playerNo;
    s16 characterNo;

    for (playerNo = 0, entry = lbl_1_bss_1308; playerNo < 2; playerNo++, entry++) {
        if (entry->isComputer == 1 && entry->choiceState == 0) {
            for (characterNo = 0; characterNo < 11; characterNo++) {
                if (lbl_1_data_AF4[characterNo].selectionState == -1) {
                    entry->characterNo = characterNo;
                    lbl_1_data_AF4[characterNo].selectionState = 1;
                    break;
                }
            }
            fn_1_8FEC(
                playerNo, &lbl_1_data_7A8[entry->characterNo + 10], 4);
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_C724;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hide a character-selection model and its paired sprite when selection is canceled. */
static inline void fn_1_AF70(s32 modelNo)
{
    OMOBJ *obj = lbl_1_bss_18.object;

    Hu3DModelAttrSet(obj->mdlId[(s16)modelNo], HU3D_ATTR_DISPOFF);
    HuSprScaleSet(lbl_1_bss_1292[3], modelNo, 0.8f, 0.8f);
    HuSprAttrSet(lbl_1_bss_1292[3], modelNo, HUSPR_ATTR_DISPOFF);
}

/* Return a character card to face forward while computer selection is being canceled. */
static inline void fn_1_122FC(s16 characterNo)
{
    MDSING_MODEL *entry = &lbl_1_bss_E74[characterNo];

    Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
}

/* Release an unconfirmed computer selection, hide its cursor, and stop card rotation. */
void fn_1_CE2C(OMOBJ *obj)
{
    s16 entryNo;
    MDSING_CHARACTER_SELECTION *entry;

    for (entryNo = 0, entry = lbl_1_bss_1308; entryNo < 2; entryNo++, entry++) {
        if (entry->isComputer == 1 && entry->choiceState == 0) {
            lbl_1_data_AF4[entry->characterNo].selectionState = -1;
            fn_1_AF70(entryNo);
        }
    }
    for (entryNo = 0; entryNo < 11; entryNo++) {
        fn_1_122FC(entryNo);
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Reset player choices and spin the character cards into view when character selection opens. */
void fn_1_CF94(OMOBJ *obj)
{
    MDSING_CHARACTER_SELECTION *player;
    MDSING_MODEL *entry;
    float animatedValue;
    s16 characterNo;
    s16 playerNo;
    s16 elapsedFrames;

    if (obj->work[0] == 0) {
        for (characterNo = 0; characterNo < 11; characterNo++) {
            fn_1_A450(characterNo, characterNo);
            fn_1_A3A8(characterNo, 1, characterNo, 2);
            fn_1_A3A8(characterNo, 3, characterNo, 2);
            lbl_1_data_AF4[characterNo].selectionState = -1;
        }
        if (lbl_1_bss_30 == 0) {
            fn_1_A450(10, 11);
        }
        for (playerNo = 0, player = lbl_1_bss_1308; playerNo < 4;
             playerNo++, player++) {
            player->choiceState = 0;
            player->setupValue = 0;
            player->isComputer = 0;
            if (playerNo != 0) {
                player->isComputer = 1;
            }
            player->difficulty = 0;
            player->characterNo = 0;
            player->padNo = playerNo;
        }
    }
    for (characterNo = 0; characterNo < 11; characterNo++) {
        elapsedFrames = obj->work[0];
        entry = &lbl_1_bss_E74[characterNo];
        if (elapsedFrames == 0) {
            if (lbl_1_bss_3C == 0) {
                HuAudFXPlay(MSM_SE_CMN_53);
                lbl_1_bss_3C = 1;
            }
            Hu3DModelPosSet(entry->modelId,
                lbl_1_data_7A8[characterNo + 10].x,
                lbl_1_data_7A8[characterNo + 10].y,
                lbl_1_data_7A8[characterNo + 10].z);
            Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(entry->modelId, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        } else {
            lbl_1_bss_3C = 0;
        }
        animatedValue = fn_1_16F0(-540.0f, 0.0f, elapsedFrames, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, animatedValue, 0.0f);
        animatedValue = fn_1_16F0(0.0f, 1.25f, elapsedFrames, 10.0f);
        Hu3DModelScaleSet(entry->modelId, animatedValue, animatedValue, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Dismiss unselected character cards and move the two selected cards into their next positions. */
void fn_1_D57C(OMOBJ *obj)
{
    s16 entryNo;
    s16 playerNo;

    for (entryNo = 0; entryNo < 11; entryNo++) {
        for (playerNo = 0; playerNo < 2; playerNo++) {
            if (entryNo == lbl_1_bss_1308[playerNo].characterNo) {
                break;
            }
        }
        if (playerNo == 2) {
            fn_1_A8E8(entryNo, obj->work[0], 10);
        }
    }
    if (obj->work[0] >= 10) {
        for (entryNo = 0; entryNo < 2; entryNo++) {
            fn_1_ACB8((s16)lbl_1_bss_1308[entryNo].characterNo,
                obj->work[0] - 10, 10, &lbl_1_data_7A8[entryNo + 21], 1.25f);
        }
    }
    if (obj->work[0]++ > 20) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Shrink both player markers and spin the selected character cards away during a transition. */
void fn_1_DE98(OMOBJ *obj)
{
    MDSING_CHARACTER_SELECTION *player;
    MDSING_MODEL *entry;
    float modelValue;
    s32 characterModelIndex;
    s16 playerNo;
    s16 elapsedFrames;

    if (obj->work[0] == 0) {
        for (playerNo = 0; playerNo < 2; playerNo++) {
            lbl_1_bss_CDC[playerNo].x = 0.0f;
        }
    }
    for (playerNo = 0, player = lbl_1_bss_1308; playerNo < 2; playerNo++, player++) {
        elapsedFrames = obj->work[0];
        characterModelIndex = (s16)player->characterNo;
        entry = &lbl_1_bss_E74[characterModelIndex];
        if (elapsedFrames == 0) {
            if (lbl_1_bss_3C == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_3C = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->position);
            Hu3DModelRotGet(entry->modelId, &entry->rotation);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_3C = 0;
        }
        modelValue = fn_1_16F0(
            entry->rotation.y, entry->rotation.y - 540.0f, elapsedFrames, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, modelValue, 0.0f);
        modelValue = fn_1_16F0(entry->scale.x, 0.0f, elapsedFrames, 10.0f);
        Hu3DModelScaleSet(entry->modelId, modelValue, modelValue, 1.0f);
        if (elapsedFrames <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Shrink both player markers and dismiss the entire character-card display. */
void fn_1_E290(OMOBJ *obj)
{
    s16 modelNo;

    if (obj->work[0] == 0) {
        modelNo = 0;
        while (modelNo < 2) {
            lbl_1_bss_CDC[modelNo].x = 0.0f;
            modelNo++;
        }
    }
    modelNo = 0;
    while (modelNo < 11) {
        fn_1_A8E8(modelNo, obj->work[0], 10);
        modelNo++;
    }
    if (obj->work[0]++ > 10U) {
        obj->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* Enlarge and move the selected board card, beginning its rotation after thirty updates. */
void fn_1_E688(s16 modelNo)
{
    Vec transform;
    MDSING_MODEL *entry = &lbl_1_bss_E74[modelNo + 11];

    Hu3DModelPosGet(entry->modelId, &transform);
    fn_1_14C4(&transform, &lbl_1_data_7A8[modelNo + 6], 5.0f);
    Hu3DModelPosSetV(entry->modelId, &transform);
    if (entry->highlightFrame++ > 30) {
        entry->highlightFrame = 35;
        Hu3DModelRotGet(entry->modelId, &transform);
        transform.y += 3.0f;
        if (transform.y >= 360.0f) {
            transform.y -= 360.0f;
        }
        Hu3DModelRotSetV(entry->modelId, &transform);
    }
    Hu3DModelScaleGet(entry->modelId, &transform);
    transform.y = transform.x =
        fn_1_1494(transform.x, 2.25f, 5.0f);
    Hu3DModelScaleSetV(entry->modelId, &transform);
    Hu3DModelLayerSet(entry->modelId, 2);
}

/* Restore an unselected board card's position, size, and facing during board selection. */
void fn_1_E9A8(s16 boardNo)
{
    MDSING_MODEL *entry = &lbl_1_bss_E74[boardNo + 11];

    Hu3DModelPosSetV(entry->modelId, &lbl_1_data_7A8[boardNo + 3]);
    Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(entry->modelId, 1.25f, 1.25f, 1.0f);
    entry->highlightFrame = 0;
    Hu3DModelLayerSet(entry->modelId, 1);
}

/* Read left and right input to select a board, updating its title and card highlight each frame. */
void fn_1_EA7C(OMOBJ *obj)
{
    s16 boardNo;

    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            OMOBJ *workObj;
            s16 memberNo;

            lbl_1_bss_1340.boardNo--;
            if (lbl_1_bss_1340.boardNo < 0) {
                lbl_1_bss_1340.boardNo += 3;
            }
            obj->work[0] = 0;
            obj->work[1] = 0;
            memberNo = lbl_1_bss_1340.boardNo;
            workObj = lbl_1_bss_8;
            HuSprGrpPosSet(
                lbl_1_bss_1292[1], 288.0f, 60.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_1292[1], 0.0f);
            fn_1_47F4(lbl_1_bss_1292[1], HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(
                lbl_1_bss_1292[1], memberNo, HUSPR_ATTR_DISPOFF);
            workObj->work[2] = 1;
            workObj->work[3] = 0;
            lbl_1_bss_C94[0] = 2.0f;
            if (lbl_1_data_AC8[0] == 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            OMOBJ *workObj;
            s16 memberNo;

            lbl_1_bss_1340.boardNo++;
            if (lbl_1_bss_1340.boardNo >= 3) {
                lbl_1_bss_1340.boardNo -= 3;
            }
            obj->work[0] = 0;
            obj->work[1] = 1;
            memberNo = lbl_1_bss_1340.boardNo;
            workObj = lbl_1_bss_8;
            HuSprGrpPosSet(
                lbl_1_bss_1292[1], 288.0f, 60.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_1292[1], 0.0f);
            fn_1_47F4(lbl_1_bss_1292[1], HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(
                lbl_1_bss_1292[1], memberNo, HUSPR_ATTR_DISPOFF);
            workObj->work[2] = 1;
            workObj->work[3] = 0;
            lbl_1_bss_C94[1] = 2.0f;
            if (lbl_1_data_AC8[1] == 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
            }
        }
    }
    for (boardNo = 0; boardNo < 3; boardNo++) {
        if (boardNo == lbl_1_bss_1340.boardNo) {
            fn_1_E688(boardNo);
        } else {
            fn_1_E9A8(boardNo);
        }
    }
}

/* Show the board navigation arrows and start the board-selection input callback. */
void fn_1_F15C(OMOBJ *obj)
{
    s16 boardNo;

    for (boardNo = 0; boardNo < 3; boardNo++) {
        lbl_1_bss_E74[boardNo + 11].highlightFrame = 0;
    }
    fn_1_8BC0(0, &lbl_1_data_7A8[0],
        -200.0f, 0.0f);
    fn_1_8BC0(1, &lbl_1_data_7A8[0],
        200.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_EA7C;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hide board navigation arrows and return board cards to face forward when selection ends. */
void fn_1_F3C8(OMOBJ *obj)
{
    s16 boardNo;

    if (obj->work[0] == 0) {
        fn_1_8CE4(0);
        fn_1_8CE4(1);
    }
    for (boardNo = 0; boardNo < 3; boardNo++) {
        lbl_1_bss_E74[boardNo + 11].highlightFrame = 0;
        fn_1_B630(boardNo + 11, 0.0f, obj->work[0], 10);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Spin and grow a board card into view during the board-selection opening animation. */
static inline void fn_1_C28C_inlined(
    s16 modelNo, s16 elapsedFrames, s16 durationFrames, HuVecF *position, float scale)
{
    MDSING_MODEL *entry = &lbl_1_bss_E74[modelNo];
    float animatedValue;

    if (elapsedFrames == 0) {
        if (lbl_1_bss_3C == 0) {
            HuAudFXPlay(MSM_SE_CMN_53);
            lbl_1_bss_3C = 1;
        }
        Hu3DModelPosSet(entry->modelId, position->x, position->y, position->z);
        Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(entry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    } else {
        lbl_1_bss_3C = 0;
    }
    animatedValue = fn_1_16F0(-540.0f, 0.0f, elapsedFrames, durationFrames);
    Hu3DModelRotSet(entry->modelId, 0.0f, animatedValue, 0.0f);
    animatedValue = fn_1_16F0(0.0f, scale, elapsedFrames, durationFrames);
    Hu3DModelScaleSet(entry->modelId, animatedValue, animatedValue, 1.0f);
}

/* Set board card textures and animate them into view before board selection begins. */
void fn_1_F6D0(OMOBJ *obj)
{
    s16 boardNo;

    lbl_1_bss_1340.boardNo = 0;
    if (obj->work[0] == 0) {
        for (boardNo = 0; boardNo < 3; boardNo++) {
            fn_1_A450(boardNo + 11, boardNo + 12);
            if (lbl_1_data_912[boardNo] == 0) {
                fn_1_A3A8(boardNo + 11, 2, 24, -1);
                fn_1_A3A8(boardNo + 11, 3, 24, -1);
            }
        }
    }
    for (boardNo = 0; boardNo < 3; boardNo++) {
        fn_1_C28C_inlined(
            boardNo + 11, obj->work[0], 10, &lbl_1_data_7A8[boardNo + 3], 1.25f);
    }
    if (obj->work[0]++ > 10) {
        Hu3DZClearLayerSet(2);
        fn_1_7A60(lbl_1_bss_1340.boardNo);
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Fade the background and heading out, then release the waiting menu process after thirty
 * updates. */
void fn_1_FCC8(OMOBJ *obj)
{
    OMOBJ *modelObject;
    OMOBJ *headingObject;
    if (obj->work[0] == 0) {
        modelObject = lbl_1_bss_8;
        modelObject->work[0] = 2;
        modelObject->work[1] = 0;
        headingObject = lbl_1_bss_8;
        headingObject->work[2] = 2;
        headingObject->work[3] = 0;
    }
    if (obj->work[0]++ > 30U) {
        obj->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* Switch the background textures to the chosen board and begin their model fade. */
static inline void fn_1_8C28_inlined(s16 animNo)
{
    OMOBJ *obj = lbl_1_bss_8;

    Hu3DAnimAnimSet(
        obj->mtnId[2], lbl_1_bss_12A4[(2 * animNo) + 3]);
    Hu3DAnimAnimSet(
        obj->mtnId[3], lbl_1_bss_12A4[(2 * animNo) + 4]);
    obj->work[0] = 1;
    obj->work[1] = 0;
}

/* Dismiss unchosen board cards and animate the chosen card into the background transition. */
void fn_1_FD5C(OMOBJ *obj)
{
    HuVecF rotation;
    s16 boardNo;
    s16 elapsedFrames;

    for (boardNo = 0; boardNo < 3; boardNo++) {
        if (boardNo != lbl_1_bss_1340.boardNo) {
            fn_1_A8E8(boardNo + 11, obj->work[0], 10);
        }
    }
    if (obj->work[0] >= 10) {
        elapsedFrames = obj->work[0] - 10;
        fn_1_ACB8(lbl_1_bss_1340.boardNo + 11, elapsedFrames, 10,
            &lbl_1_data_7A8[9], 1.5f);
        Hu3DModelRotGet(
            lbl_1_bss_E74[lbl_1_bss_1340.boardNo + 11].modelId, &rotation);
        rotation.y += 54.0f;
        Hu3DModelRotSetV(
            lbl_1_bss_E74[lbl_1_bss_1340.boardNo + 11].modelId, &rotation);
    }
    if (obj->work[0] == 20) {
        Hu3DLayerHookReset(2);
        fn_1_33FC0(&lbl_1_data_7A8[9]);
        HuAudFXPlay(MSM_SE_MENU_15);
        fn_1_8C28_inlined(lbl_1_bss_1340.boardNo);
        Hu3DModelAttrSet(
            lbl_1_bss_E74[lbl_1_bss_1340.boardNo + 11].modelId,
            HU3D_ATTR_DISPOFF);
    }
    if (obj->work[0]++ > 90) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Fade the heading and spin all board cards away when the board display is closed. */
void fn_1_10744(OMOBJ *obj)
{
    MDSING_MODEL *entry;
    s16 boardNo;
    OMOBJ *workObj;
    float animatedValue;
    s16 elapsedFrames;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[2] = 2;
        workObj->work[3] = 0;
    }
    for (boardNo = 0; boardNo < 3; boardNo++) {
        elapsedFrames = obj->work[0];
        entry = &lbl_1_bss_E74[(s16)(boardNo + 11)];
        if (elapsedFrames == 0) {
            if (lbl_1_bss_3C == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_3C = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->position);
            Hu3DModelRotGet(entry->modelId, &entry->rotation);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_3C = 0;
        }
        animatedValue = fn_1_16F0(
            entry->rotation.y, entry->rotation.y - 540.0f, elapsedFrames, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, animatedValue, 0.0f);
        animatedValue = fn_1_16F0(entry->scale.x, 0.0f, elapsedFrames, 10.0f);
        Hu3DModelScaleSet(entry->modelId, animatedValue, animatedValue, 1.0f);
        if (elapsedFrames <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        Hu3DLayerHookReset(2);
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Move the two settings cards and both character cards into their expanded layout. */
void fn_1_10B18(OMOBJ *object)
{
    s16 playerNo;
    MDSING_CHARACTER_SELECTION *player;

    fn_1_ACB8(14, object->work[0], 10, &lbl_1_data_7A8[23], 1.5f);
    fn_1_ACB8(15, object->work[0], 10, &lbl_1_data_7A8[24], 1.5f);

    playerNo = 0;
    player = lbl_1_bss_1308;
    while (playerNo < 2) {
        fn_1_ACB8((s16)player->characterNo, object->work[0], 10,
            &lbl_1_data_7A8[playerNo + 25], 1.25f);
        playerNo++;
        player++;
    }

    if (object->work[0]++ > 10U) {
        object->objFunc = NULL;
        fn_1_B898(1);
    }
}

/* Dismiss the settings and character cards, then show the selected character models. */
void fn_1_119E8(OMOBJ *obj)
{
    MDSING_CHARACTER_SELECTION *player;
    OMOBJ *workObj;
    s16 playerNo;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[2] = 2;
        workObj->work[3] = 0;
        for (playerNo = 0; playerNo < 2; playerNo++) {
            lbl_1_bss_CDC[playerNo].x = 0.0f;
        }
    }
    fn_1_A8E8(14, obj->work[0], 10);
    fn_1_A8E8(15, obj->work[0], 10);
    for (playerNo = 0, player = lbl_1_bss_1308; playerNo < 2; playerNo++, player++) {
        fn_1_A8E8((s16)player->characterNo, obj->work[0], 10);
    }
    if (obj->work[0]++ > 10) {
        fn_1_6C48();
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Start fading the heading out during a menu transition. */
static inline void fn_1_8950_inlined(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[2] = 2;
    obj->work[3] = 0;
}

/* Dismiss the settings and character cards while fading the background and heading. */
void fn_1_1241C(OMOBJ *obj)
{
    MDSING_CHARACTER_SELECTION *player;
    OMOBJ *workObj;
    s16 playerNo;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[0] = 2;
        workObj->work[1] = 0;
        fn_1_8950_inlined();
        for (playerNo = 0; playerNo < 2; playerNo++) {
            lbl_1_bss_CDC[playerNo].x = 0.0f;
        }
    }
    fn_1_A8E8(14, obj->work[0], 10);
    fn_1_A8E8(15, obj->work[0], 10);
    for (playerNo = 0, player = lbl_1_bss_1308; playerNo < 2; playerNo++, player++) {
        fn_1_A8E8((s16)player->characterNo, obj->work[0], 10);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Move the settings and selected character cards back to their compact layout. */
void fn_1_12E68(OMOBJ *object)
{
    s16 playerNo;
    MDSING_CHARACTER_SELECTION *player;
    fn_1_ACB8(14, (s16)object->work[0], 10,
              &lbl_1_data_7A8[1], 1.0f);

    fn_1_ACB8(15, (s16)object->work[0], 10,
              &lbl_1_data_7A8[2], 1.0f);

    for (playerNo = 0, player = lbl_1_bss_1308; playerNo < 2; playerNo++, player++) {
        fn_1_ACB8((s16)player->characterNo, (s16)object->work[0], 10,
                  &lbl_1_data_7A8[playerNo + 21],
                  1.25f);
    }

    if (object->work[0]++ > 10U) {
        object->objFunc = NULL;
        fn_1_B898(1);
    }
}

/* Handle difficulty and minigame-pack input and keep the setting arrows beside the active row. */
void fn_1_13D38(OMOBJ *object) {
    Point3d projectedModelPosition;
    Point3d model14RotationA;
    Point3d model14RotationB;
    Point3d model15RotationA;
    Point3d model15RotationB;
    Point3d choicePositionA;
    Point3d choicePositionB;
    MDSING_MODEL *model14AnimationA;
    MDSING_MODEL *model14AnimationB;
    MDSING_MODEL *model14AnimationC;
    MDSING_MODEL *model14AnimationD;
    MDSING_MODEL *model15AnimationA;
    MDSING_MODEL *model15AnimationB;
    MDSING_MODEL *model15AnimationC;
    MDSING_MODEL *model15AnimationD;
    s16 selectedModelType;
    s16 selectedModelOption;
    s16 selectedCharacterType;
    s16 selectedCharacterOption;
    s16 animationSlotA;
    s16 animationSlotB;
    s16 animationSlotC;
    s16 animationSlotD;
    s16 animationSlotE;
    s16 animationSlotF;
    s16 animationSlotG;
    s16 animationSlotH;
    s16 animationSlotI;
    s16 animationSlotJ;
    f32 model15SecondAbsoluteRotationY;
    f32 model15AbsoluteRotationY;
    f32 model14SecondAbsoluteRotationY;
    f32 model14AbsoluteRotationY;
    f32 model15SecondRotationY;
    f32 model15RotationY;
    f32 model14SecondRotationY;
    f32 model14RotationY;
    s16 animationIndexF;
    s16 animationIndexE;
    s16 animationIndexD;
    s16 animationIndexC;
    s16 animationIndexB;
    s16 animationIndexA;
    u32 frameCountA;
    u32 playerType;
    u32 frameCountB;
    u32 frameCountC;

    if (object->work[0]++ >= 10U) {
        if ((s32) (HuPadDStkRep[0] & PAD_BUTTON_LEFT) != 0) {
            lbl_1_bss_1340.settingsRow -= 1;
            if (lbl_1_bss_1340.settingsRow < 0) {
                lbl_1_bss_1340.settingsRow += 2;
            }
            object->work[0] = 0U;
            object->work[1] = 2U;
            lbl_1_bss_C94[4] = 2.0f;
            if (lbl_1_data_AC8[4] == 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
            }
        } else if ((s32) (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) != 0) {
            lbl_1_bss_1340.settingsRow += 1;
            if (lbl_1_bss_1340.settingsRow > 1) {
                lbl_1_bss_1340.settingsRow -= 2;
            }
            object->work[0] = 0U;
            object->work[1] = 2U;
            lbl_1_bss_C94[5] = 2.0f;
            if (lbl_1_data_AC8[5] == 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
            }
        } else if ((s32) (HuPadDStkRep[0] & PAD_BUTTON_DOWN) != 0) {
            switch (lbl_1_bss_1340.settingsRow) {
            case 0:
                lbl_1_bss_1340.difficulty -= 1;
                if (lbl_1_bss_32 == 0) {
                    if (lbl_1_bss_1340.difficulty < 0) {
                        lbl_1_bss_1340.difficulty += 3;
                    }
                } else if (lbl_1_bss_1340.difficulty < 0) {
                    lbl_1_bss_1340.difficulty += 4;
                }
                break;
            case 1:
                lbl_1_bss_1340.minigamePack -= 1;
                if (lbl_1_bss_1340.minigamePack < 0) {
                    lbl_1_bss_1340.minigamePack += 5;
                }
                break;
            }
            object->work[0] = 0U;
            object->work[1] = 0U;
            lbl_1_bss_C94[3] = 2.0f;
            if (lbl_1_data_AC8[3] == 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
            }
        } else if ((s32) (HuPadDStkRep[0] & PAD_BUTTON_UP) != 0) {
            switch (lbl_1_bss_1340.settingsRow) {
            case 0:
                lbl_1_bss_1340.difficulty += 1;
                if (lbl_1_bss_32 == 0) {
                    if (lbl_1_bss_1340.difficulty > 2) {
                        lbl_1_bss_1340.difficulty -= 3;
                    }
                } else if (lbl_1_bss_1340.difficulty > 3) {
                    lbl_1_bss_1340.difficulty -= 4;
                }
                break;
            case 1:
                lbl_1_bss_1340.minigamePack += 1;
                if (lbl_1_bss_1340.minigamePack >= 5) {
                    lbl_1_bss_1340.minigamePack -= 5;
                }
                break;
            }
            object->work[0] = 0U;
            object->work[1] = 1U;
            lbl_1_bss_C94[2] = 2.0f;
            if (lbl_1_data_AC8[2] == 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
            }
        }
    } else if ((u32) object->work[1] != 2U) {
        switch (lbl_1_bss_1340.settingsRow) {
        case 0:
            if ((u32) object->work[1] != 0U) {
                fn_1_B2A8(14, lbl_1_bss_1340.difficulty, 15,
                    object->work[0], 10, 1);
            } else {
                fn_1_B2A8(14, lbl_1_bss_1340.difficulty, 15,
                    object->work[0], 10, -1);
            }
            break;
        case 1:
            if ((u32) object->work[1] != 0U) {
                fn_1_B2A8(15, lbl_1_bss_1340.minigamePack, 19,
                    object->work[0], 10, 1);
            } else {
                fn_1_B2A8(15, lbl_1_bss_1340.minigamePack, 19,
                    object->work[0], 10, -1);
            }
            break;
        }
    }
    Hu3DModelPosGet(lbl_1_bss_E74[14 + lbl_1_bss_1340.settingsRow].modelId,
                    &projectedModelPosition);
    fn_1_8B28(2, &projectedModelPosition, 0.0f, -65.0f);
    fn_1_8B28(3, &projectedModelPosition, 0.0f, 65.0f);
}

/* Show the four settings navigation arrows and begin the settings input callback. */
void fn_1_14DA8(OMOBJ *obj)
{
    HuVecF position;

    Hu3DModelPosGet(
        lbl_1_bss_E74[lbl_1_bss_1340.settingsRow + 14].modelId, &position);
    fn_1_8BC0(2, &position, 0.0f, -65.0f);
    fn_1_8BC0(3, &position, 0.0f, 65.0f);
    position.x = 0.0f;
    fn_1_8BC0(4, &position, -120.0f, 0.0f);
    fn_1_8BC0(5, &position, 120.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_13D38;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hide the settings navigation arrows and signal that their display is closed. */
void fn_1_15198(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_1292[2], 2, HUSPR_ATTR_DISPOFF);
    lbl_1_data_AC8[2] = 0;
    HuSprAttrSet(lbl_1_bss_1292[2], 3, HUSPR_ATTR_DISPOFF);
    lbl_1_data_AC8[3] = 0;
    HuSprAttrSet(lbl_1_bss_1292[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_AC8[4] = 0;
    HuSprAttrSet(lbl_1_bss_1292[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_AC8[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Spin and grow a settings card into view during the settings opening animation. */
static inline void update_entry_display(s16 modelNo, s16 elapsedFrames,
    s16 durationFrames, const Vec *position, f32 scale)
{
    MDSING_MODEL *entry = &lbl_1_bss_E74[modelNo];
    f32 animatedValue;

    if (elapsedFrames == 0) {
        if (lbl_1_bss_3C == 0) {
            HuAudFXPlay(MSM_SE_CMN_53);
            lbl_1_bss_3C = 1;
        }
        Hu3DModelPosSet(entry->modelId, position->x, position->y, position->z);
        Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(entry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    } else {
        lbl_1_bss_3C = 0;
    }
    animatedValue = fn_1_16F0(-540.0f, 0.0f, elapsedFrames,
        durationFrames);
    Hu3DModelRotSet(entry->modelId, 0.0f, animatedValue, 0.0f);
    animatedValue = fn_1_16F0(0.0f, scale, elapsedFrames, durationFrames);
    Hu3DModelScaleSet(entry->modelId, animatedValue, animatedValue, 1.0f);
}

/* Load the saved difficulty and minigame-pack choices and animate their cards into view. */
void fn_1_15280(OMOBJ *obj)
{
    MDSING_MODEL *difficultyCard;
    MDSING_MODEL *packCard;
    s16 difficultySurface;
    s16 packSurface;
    s16 difficultySurfaceNo;
    s16 difficultyTexture;
    s16 packSurfaceNo;
    s16 packTexture;
    if (obj->work[0] == 0)
    {
        lbl_1_bss_1340.settingsRow = 0;
        lbl_1_bss_1340.difficulty = (((u8 *) & GwCommon)[1069] >> 5) & 3;
        lbl_1_bss_1340.minigamePack = (((u8 *) & GwCommon)[1066] >> 1) & 7;
        difficultyTexture = lbl_1_bss_1340.difficulty + 15;
        for (difficultySurfaceNo = 0; difficultySurfaceNo < 4; difficultySurfaceNo++)
        {
            difficultySurface = difficultySurfaceNo;
            difficultyCard = &lbl_1_bss_E74[14];
            Hu3DAnimAnimSet(difficultyCard->animationId[difficultySurface],
                lbl_1_bss_11F4[difficultyTexture]);
        }
        packTexture = lbl_1_bss_1340.minigamePack + 19;
        for (packSurfaceNo = 0; packSurfaceNo < 4; packSurfaceNo++)
        {
            packSurface = packSurfaceNo;
            packCard = &lbl_1_bss_E74[15];
            Hu3DAnimAnimSet(packCard->animationId[packSurface],
                lbl_1_bss_11F4[packTexture]);
        }
    }
    update_entry_display(14, obj->work[0], 10, & lbl_1_data_7A8[23], 1.5f);
    update_entry_display(15, obj->work[0], 10, & lbl_1_data_7A8[24], 1.5f);
    if (obj->work[0]++ > 10)
    {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Move and enlarge both settings cards into the editable settings layout. */
void fn_1_15AC4(OMOBJ *obj)
{
    MDSING_MODEL *difficultyCard;
    MDSING_MODEL *packCard;
    Vec difficultyPosition;
    Vec packPosition;
    f32 difficultyScale;
    f32 packScale;
    s16 difficultyFrame;
    s16 packFrame;

    if (obj->work[0] == 0) {
        lbl_1_bss_1340.settingsRow = 0;
    }
    difficultyFrame = obj->work[0];
    difficultyCard = &lbl_1_bss_E74[14];
    if (difficultyFrame == 0) {
        Hu3DModelPosGet(difficultyCard->modelId, &difficultyCard->position);
        Hu3DModelRotGet(difficultyCard->modelId, &difficultyCard->rotation);
        Hu3DModelScaleGet(difficultyCard->modelId, &difficultyCard->scale);
    }
    difficultyPosition.x = fn_1_16F0(
        difficultyCard->position.x, lbl_1_data_7A8[23].x, difficultyFrame, 10.0f);
    difficultyPosition.y = fn_1_16F0(
        difficultyCard->position.y, lbl_1_data_7A8[23].y, difficultyFrame, 10.0f);
    difficultyPosition.z = fn_1_16F0(
        difficultyCard->position.z, lbl_1_data_7A8[23].z, difficultyFrame, 10.0f);
    Hu3DModelPosSet(difficultyCard->modelId, difficultyPosition.x, difficultyPosition.y,
        difficultyPosition.z);
    difficultyScale = fn_1_16F0(
        difficultyCard->scale.x, 1.5f, difficultyFrame, 10.0f);
    Hu3DModelScaleSet(difficultyCard->modelId, difficultyScale, difficultyScale, 1.0f);

    packFrame = obj->work[0];
    packCard = &lbl_1_bss_E74[15];
    if (packFrame == 0) {
        Hu3DModelPosGet(packCard->modelId, &packCard->position);
        Hu3DModelRotGet(packCard->modelId, &packCard->rotation);
        Hu3DModelScaleGet(packCard->modelId, &packCard->scale);
    }
    packPosition.x = fn_1_16F0(
        packCard->position.x, lbl_1_data_7A8[24].x, packFrame, 10.0f);
    packPosition.y = fn_1_16F0(
        packCard->position.y, lbl_1_data_7A8[24].y, packFrame, 10.0f);
    packPosition.z = fn_1_16F0(
        packCard->position.z, lbl_1_data_7A8[24].z, packFrame, 10.0f);
    Hu3DModelPosSet(packCard->modelId, packPosition.x, packPosition.y,
        packPosition.z);
    packScale = fn_1_16F0(
        packCard->scale.x, 1.5f, packFrame, 10.0f);
    Hu3DModelScaleSet(packCard->modelId, packScale, packScale, 1.0f);

    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Return both settings cards to their compact menu positions. */
void fn_1_164A8(OMOBJ *obj)
{
    MDSING_MODEL *difficultyCard;
    MDSING_MODEL *packCard;
    Vec difficultyPosition;
    Vec packPosition;
    f32 difficultyScale;
    f32 packScale;
    s16 difficultyFrame;
    s16 packFrame;

    difficultyFrame = obj->work[0];
    difficultyCard = &lbl_1_bss_E74[14];
    if (difficultyFrame == 0) {
        Hu3DModelPosGet(difficultyCard->modelId, &difficultyCard->position);
        Hu3DModelRotGet(difficultyCard->modelId, &difficultyCard->rotation);
        Hu3DModelScaleGet(difficultyCard->modelId, &difficultyCard->scale);
    }
    difficultyPosition.x = fn_1_16F0(
        difficultyCard->position.x, lbl_1_data_7A8[1].x, difficultyFrame, 10.0f);
    difficultyPosition.y = fn_1_16F0(
        difficultyCard->position.y, lbl_1_data_7A8[1].y, difficultyFrame, 10.0f);
    difficultyPosition.z = fn_1_16F0(
        difficultyCard->position.z, lbl_1_data_7A8[1].z, difficultyFrame, 10.0f);
    Hu3DModelPosSet(difficultyCard->modelId, difficultyPosition.x, difficultyPosition.y,
        difficultyPosition.z);
    difficultyScale = fn_1_16F0(
        difficultyCard->scale.x, 1.0f, difficultyFrame, 10.0f);
    Hu3DModelScaleSet(difficultyCard->modelId, difficultyScale, difficultyScale, 1.0f);

    packFrame = obj->work[0];
    packCard = &lbl_1_bss_E74[15];
    if (packFrame == 0) {
        Hu3DModelPosGet(packCard->modelId, &packCard->position);
        Hu3DModelRotGet(packCard->modelId, &packCard->rotation);
        Hu3DModelScaleGet(packCard->modelId, &packCard->scale);
    }
    packPosition.x = fn_1_16F0(
        packCard->position.x, lbl_1_data_7A8[2].x, packFrame, 10.0f);
    packPosition.y = fn_1_16F0(
        packCard->position.y, lbl_1_data_7A8[2].y, packFrame, 10.0f);
    packPosition.z = fn_1_16F0(
        packCard->position.z, lbl_1_data_7A8[2].z, packFrame, 10.0f);
    Hu3DModelPosSet(packCard->modelId, packPosition.x, packPosition.y,
        packPosition.z);
    packScale = fn_1_16F0(
        packCard->scale.x, 1.0f, packFrame, 10.0f);
    Hu3DModelScaleSet(packCard->modelId, packScale, packScale, 1.0f);

    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Interpolate menu animation values, clamping before the start and after the duration. */
static inline f32 MdsingInterpolate(f32 start, f32 end, f32 elapsedFrames, f32 durationFrames)
{
    if (elapsedFrames <= 0.0f) {
        return start;
    }
    if (elapsedFrames >= durationFrames) {
        return end;
    }
    return start + ((elapsedFrames / durationFrames) * (end - start));
}

/* Capture a card's starting transform and spin and shrink it during dismissal. */
static inline void MdsingModelTransition(MDSING_MODEL *entry, s16 elapsedFrames)
{
    f32 animatedValue;

    if (elapsedFrames == 0) {
        if (lbl_1_bss_3C == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_3C = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->position);
        Hu3DModelRotGet(entry->modelId, &entry->rotation);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_3C = 0;
    }

    {
        animatedValue = MdsingInterpolate(entry->rotation.y, entry->rotation.y - 540.0f,
                                  (f32)elapsedFrames, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, animatedValue, 0.0f);
        animatedValue = MdsingInterpolate(entry->scale.x, 0.0f, (f32)elapsedFrames, 10.0f);
        Hu3DModelScaleSet(entry->modelId, animatedValue, animatedValue, 1.0f);
    }
    if (elapsedFrames <= 10) {
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    }
}

/* Dismiss both settings cards and release the waiting menu process after the animation. */
void fn_1_16E70(OMOBJ *obj)
{
    MDSING_MODEL *difficultyCard;
    MDSING_MODEL *packCard;
    s16 difficultyFrame;
    s16 packFrame;

    difficultyFrame = (s16)obj->work[0];
    difficultyCard = &lbl_1_bss_E74[14];
    MdsingModelTransition(difficultyCard, difficultyFrame);
    packFrame = (s16)obj->work[0];
    packCard = &lbl_1_bss_E74[15];
    MdsingModelTransition(packCard, packFrame);

    if (obj->work[0]++ > 10U) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Release the background animations and linked models during full menu teardown. */
static inline void fn_1_8670_inlined(OMOBJ *obj)
{
    s16 modelNo;

    if (obj) {
        for (modelNo = 0; modelNo < 2; modelNo++) {
            Hu3DAnimKill(obj->mtnId[2 * modelNo]);
            Hu3DAnimKill(obj->mtnId[(2 * modelNo) + 1]);
            obj->mtnId[2 * modelNo] = -1;
            obj->mtnId[(2 * modelNo) + 1] = -1;
        }
        for (modelNo = 1; modelNo >= 0; modelNo--) {
            Hu3DModelKill(obj->mdlId[modelNo]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* Release the Solo menu objects, cards, windows, lights, and camera before changing overlays. */
void fn_1_1750C(void)
{
    OMOBJ *setupObj;
    OMOBJ *selectionObj;
    OMOBJ *characterObj;
    OMOBJ *playerObj;
    s16 selectionModelIndex;
    s16 playerMotionIndex;
    MDSING_CAMERA *cameraWork;
    s16 setupIndex;
    s16 modelIndex;
    s16 animationIndex;
    s16 windowIndex;
    extern void fn_1_33D84(void);

    setupObj = lbl_1_bss_4;
    if (setupObj) {
        for (setupIndex = 0; setupIndex < 3; setupIndex++) {
            Hu3DMotionKill(setupObj->mtnId[setupIndex]);
            Hu3DModelKill(setupObj->mdlId[setupIndex]);
        }
        omDelObjEx(lbl_1_bss_0, setupObj);
    }
    setupObj = NULL;

    fn_1_8670_inlined(lbl_1_bss_8);

    characterObj = lbl_1_bss_C;
    if (characterObj) {
        Hu3DMotionKill(characterObj->mtnId[0]);
        Hu3DMotionKill(characterObj->mtnId[1]);
        Hu3DMotionKill(characterObj->mtnId[2]);
        Hu3DMotionKill(characterObj->mtnId[3]);
        Hu3DModelKill(characterObj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, characterObj);
    }
    /* The guide callback is cleared after deletion, outside the null guard. */
    characterObj->objFunc = NULL;

    playerObj = lbl_1_bss_10;
    if (playerObj) {
        for (playerMotionIndex = 0; playerMotionIndex < 11; playerMotionIndex++) {
            Hu3DMotionKill(playerObj->mtnId[2 * playerMotionIndex]);
            Hu3DMotionKill(playerObj->mtnId[(2 * playerMotionIndex) + 1]);
            Hu3DModelKill(playerObj->mdlId[playerMotionIndex]);
        }
        omDelObjEx(lbl_1_bss_0, playerObj);
    }
    playerObj = NULL;

    selectionObj = lbl_1_bss_18.object;
    if (selectionObj) {
        Hu3DMotionKill(selectionObj->mtnId[0]);
        for (selectionModelIndex = 3; selectionModelIndex >= 0; selectionModelIndex--) {
            Hu3DModelKill(selectionObj->mdlId[selectionModelIndex]);
        }
        omDelObjEx(lbl_1_bss_0, selectionObj);
    }
    selectionObj = NULL;

    fn_1_33D84();

    if (lbl_1_bss_24) {
        lbl_1_bss_24->mtnId[0] = -1;
        for (modelIndex = 15; modelIndex >= 0; modelIndex--) {
            for (animationIndex = 0; animationIndex < 4; animationIndex++) {
                Hu3DAnimKill(lbl_1_bss_E74[modelIndex].animationId[animationIndex]);
            }
            Hu3DModelKill(lbl_1_bss_E74[modelIndex].modelId);
        }
    }
    lbl_1_bss_24 = NULL;

    for (windowIndex = 0; windowIndex < 4; windowIndex++) {
        HuWinExKill(lbl_1_bss_1398[windowIndex]);
    }
    HuWinAllKill();
    Hu3DGLightKill(lbl_1_bss_13A0[0]);
    Hu3DGLightKill(lbl_1_bss_13A0[1]);
    cameraWork = &lbl_1_bss_1348;
    Hu3DCameraKill(1);
    if (cameraWork->obj) {
        omDelObjEx(lbl_1_bss_0, cameraWork->obj);
    }
    cameraWork->obj = NULL;
}

/* Finish Solo menu teardown and return to the previous overlay once the outgoing wipe ends. */
void fn_1_1791C(OMOBJ *obj)
{
    if (!WipeCheck()) {
        fn_1_1750C();
        omOvlReturnEx(1, 1);
    }
}

/* Begin the outgoing wipe when the system requests an exit from the Solo menu. */
void fn_1_17958(OMOBJ *obj)
{
    if (omSysExitReq != 0) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        obj->objFunc = fn_1_1791C;
    }
}

/* Run Solo setup, fade out its music, and leave for the selected board or the previous overlay. */
void fn_1_179B0(void)
{
    s16 result;

    result = 0;
    result = fn_1_20624(0);
    if (lbl_1_data_320[0] != -1) {
        HuAudSStreamFadeOut(lbl_1_data_320[0], 1000);
        lbl_1_data_320[0] = -1;
    }
    switch (result) {
    case 0:
    case 1:
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        break;
    case 2:
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        break;
    }
    fn_1_1750C();
    if (result == 2) {
        OMOVLHIS *readHistory;

        do {
            HuPrcVSleep();
        } while (lbl_1_bss_34 == 0);
        readHistory = omOvlHisGet(0);
        omOvlHisChg(0, readHistory->ovl, 1, lbl_1_bss_1308[1].characterNo);
        OSReport(lbl_1_data_9A3);
        OSReport(lbl_1_data_95A, 33);
        OSReport(lbl_1_data_96B, 36);
        OSReport(lbl_1_data_97D, 155);
        OSReport(lbl_1_data_98F, 242);
        HuAMemDump();
        OSReport(lbl_1_data_99D);
        switch (lbl_1_bss_1340.boardNo) {
        case 0:
            omOvlCallEx(DLL_s01dll, 1, 0, 0);
            break;
        case 1:
            omOvlCallEx(DLL_s02dll, 1, 0, 0);
            break;
        case 2:
            omOvlCallEx(DLL_s03dll, 1, 0, 0);
            break;
        }
    } else if (result == 1) {
        OMOVLHIS *returnHistory;

        do {
            HuPrcVSleep();
        } while (lbl_1_bss_34 == 0);
        returnHistory = omOvlHisGet(0);
        omOvlHisChg(0, returnHistory->ovl, 2, 0);
        OSReport(lbl_1_data_9A3);
        OSReport(lbl_1_data_95A, 33);
        OSReport(lbl_1_data_96B, 36);
        OSReport(lbl_1_data_97D, 155);
        OSReport(lbl_1_data_98F, 242);
        HuAMemDump();
        OSReport(lbl_1_data_99D);
        omOvlCallEx(DLL_w11dll, 1, 0, 0);
    } else {
        CharDataClose(CHARNO_NONE);
        HuARDirFree(DATANUM(DATA_board, 0));
        HuARDirFree(DATANUM(DATA_board_us, 0));
        HuARDirFree(DATANUM(DATA_capsule, 0));
        OSReport(lbl_1_data_A02);
        OSReport(lbl_1_data_95A, 33);
        OSReport(lbl_1_data_96B, 36);
        OSReport(lbl_1_data_97D, 155);
        OSReport(lbl_1_data_98F, 242);
        HuAMemDump();
        OSReport(lbl_1_data_99D);
        omOvlReturnEx(1, 1);
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Create the Solo dialogue windows and restrict their input to controller zero during setup. */
static inline void fn_1_3908_inlined(void)
{
    s16 windowNo;

    HuWinInit(1);
    lbl_1_bss_1398[0] = HuWinExCreateFrame(16.0f, 337.0f, 544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_1398[0]);
    HuWinBGTPLvlSet(lbl_1_bss_1398[0], 0.0f);
    lbl_1_bss_1398[1] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 0);
    HuWinDispOff(lbl_1_bss_1398[1]);
    HuWinBGTPLvlSet(lbl_1_bss_1398[1], 0.9f);
    lbl_1_bss_1398[2] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 3);
    HuWinDispOff(lbl_1_bss_1398[2]);
    HuWinBGTPLvlSet(lbl_1_bss_1398[2], 0.9f);
    lbl_1_bss_1398[3] = HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 4);
    HuWinDispOff(lbl_1_bss_1398[3]);
    HuWinBGTPLvlSet(lbl_1_bss_1398[3], 0.9f);
    for (windowNo = 0; windowNo < 4; windowNo++) {
        winData[lbl_1_bss_1398[windowNo]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_1398[windowNo], fn_1_0);
    }
}

/* Load and arrange Solo menu sprite groups, leaving them hidden until their screens open. */
static inline void fn_1_48F4_inlined(void)
{
    MDSING_SPRITE_DESC *spriteDesc;
    s16 entryNo;

    for (entryNo = 0; entryNo < 25; entryNo++) {
        lbl_1_bss_12A4[entryNo] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_32C[entryNo], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (entryNo = 0; entryNo < 9; entryNo++) {
        lbl_1_bss_1292[entryNo] = HuSprGrpCreate(lbl_1_data_390[entryNo]);
    }
    for (entryNo = 0, spriteDesc = lbl_1_data_3A4; entryNo < 29; entryNo++, spriteDesc++) {
        lbl_1_bss_1258[entryNo] = HuSprCreate(
            lbl_1_bss_12A4[spriteDesc->animNo], spriteDesc->priority, spriteDesc->bank);
        HuSprGrpMemberSet(lbl_1_bss_1292[spriteDesc->groupNo], spriteDesc->memberNo,
            lbl_1_bss_1258[entryNo]);
        HuSprPosSet(lbl_1_bss_1292[spriteDesc->groupNo], spriteDesc->memberNo,
            spriteDesc->pos.x, spriteDesc->pos.y);
        HuSprScaleSet(lbl_1_bss_1292[spriteDesc->groupNo], spriteDesc->memberNo,
            spriteDesc->scale.x, spriteDesc->scale.y);
        HuSprZRotSet(lbl_1_bss_1292[spriteDesc->groupNo], spriteDesc->memberNo,
            spriteDesc->zRot);
    }
    for (entryNo = 0; entryNo < 9; entryNo++) {
        fn_1_47F4(lbl_1_bss_1292[entryNo], 4);
    }
}

/* Load menu card textures and linked models while the Solo menu is initialized. */
static inline void fn_1_B998_inlined(void)
{
    BITMAP_NAME_TABLE bitmapName = lbl_1_rodata_270;
    s16 entryNo;
    s16 animationNo;

    for (entryNo = 0; entryNo < 25; entryNo++) {
        lbl_1_bss_11F4[entryNo] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_744[entryNo], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (entryNo = 0; entryNo < 16; entryNo++) {
        memset(&lbl_1_bss_E74[entryNo], 0, sizeof(MDSING_MODEL));
        if (entryNo == 0) {
            lbl_1_bss_E74[entryNo].modelId = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 21),
                    HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            lbl_1_bss_E74[entryNo].modelId =
                Hu3DModelLink(lbl_1_bss_E74[0].modelId);
        }
        for (animationNo = 0; animationNo < 4; animationNo++) {
            lbl_1_bss_E74[entryNo].animationId[animationNo] = Hu3DAnimCreate(
                lbl_1_bss_11F4[0], lbl_1_bss_E74[entryNo].modelId,
                bitmapName.name[animationNo]);
        }
        Hu3DModelLayerSet(lbl_1_bss_E74[entryNo].modelId, 1);
        Hu3DModelAttrSet(lbl_1_bss_E74[entryNo].modelId, HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_24 = omAddObjEx(lbl_1_bss_0, 4096, 16U, 16U, -1, NULL);
}

/* Create the Solo menu scene and start its setup process on entry or return from a board. */
void fn_1_17D38(void) {
    MDSING_CAMERA_WORK *cameraWork;
    Point3d shadowTarget;
    Point3d shadowUp;
    Point3d shadowCenter;

    lbl_1_bss_0 = omInitObjMan(27, 8192);
    cameraWork = &lbl_1_bss_1348;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f, 10000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    memset(cameraWork, 0, 80U);
    cameraWork->callback = fn_1_18DB0;
    cameraWork->center.x = 0.0f;
    cameraWork->center.y = 65.0f;
    cameraWork->center.z = -800.0f;
    cameraWork->rot.x = -7.25f;
    cameraWork->rot.y = 0.0f;
    cameraWork->rot.z = 0.0f;
    if ((s32) lbl_1_bss_28 == 0) {
        cameraWork->zoom = 2650.0f;
    } else {
        cameraWork->zoom = 2150.0f;
    }
    cameraWork->obj = omAddObjEx(lbl_1_bss_0, 2304, 16U, 16U, -1, fn_1_31BC);
    lbl_1_bss_13A0[0] = Hu3DGLightCreate(
        0.0f, 1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 255U, 255U, 255U);
    Hu3DGLightInfinitytSet(lbl_1_bss_13A0[0]);
    Hu3DGLightStaticSet(lbl_1_bss_13A0[0], 1);
    lbl_1_bss_13A0[1] = Hu3DGLightCreate(
        -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 255U, 255U, 255U);
    Hu3DGLightInfinitytSet(lbl_1_bss_13A0[1]);
    Hu3DGLightStaticSet(lbl_1_bss_13A0[1], 1);
    fn_1_3908_inlined();
    shadowCenter = lbl_1_rodata_108;
    shadowUp = lbl_1_rodata_114;
    shadowTarget = lbl_1_rodata_120;
    Hu3DShadowCreate(30.0f, 10.0f, 10000.0f);
    Hu3DShadowPosSet(&shadowCenter, &shadowUp, &shadowTarget);
    fn_1_48F4_inlined();
    fn_1_B998_inlined();
    fn_1_33264(lbl_1_bss_0);
    lbl_1_bss_4 = omAddObjEx(lbl_1_bss_0, 4096, 16U, 16U, -1, fn_1_77C0);
    lbl_1_bss_8 = omAddObjEx(lbl_1_bss_0, 4096, 16U, 16U, -1, fn_1_842C);
    lbl_1_bss_C = omAddObjEx(lbl_1_bss_0, 4096, 16U, 16U, -1, fn_1_5FA0);
    lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 4096, 16U, 40U, -1, fn_1_7134);
    if ((s32) lbl_1_bss_28 == 1) {
        lbl_1_bss_20 = 1;
        lbl_1_bss_22 = GwPlayer->charNo;
    } else {
        lbl_1_bss_20 = 0;
        lbl_1_bss_22 = 0;
    }
    lbl_1_bss_14 = omAddObjEx(lbl_1_bss_0, 4096, 16U, 16U, -1, fn_1_8A80);
    lbl_1_bss_18.object = omAddObjEx(lbl_1_bss_0, 4096, 16U, 16U, -1, fn_1_9718);
    if ((s32) lbl_1_bss_28 == 1) {
        fn_1_256C8();
    }
    HuPrcChildCreate(fn_1_179B0, 12288U, 12288U, 0, lbl_1_bss_0);
}

/* Wait for board data to be ready, then reenter the selected Solo board. */
void fn_1_188C0(void)
{
    OMOVLHIS *history;

    HuPrcVSleep();
    OSReport(lbl_1_data_BBA);
    do {
        HuPrcVSleep();
    } while (lbl_1_bss_34 == 0);

    history = omOvlHisGet(0);
    omOvlHisChg(0, history->ovl, 1, lbl_1_bss_1308[1].characterNo);
    OSReport(lbl_1_data_9A3);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    switch (lbl_1_bss_1340.boardNo) {
        case 0:
            omOvlCallEx(DLL_s01dll, 1, 0, 0);
            break;
        case 1:
            omOvlCallEx(DLL_s02dll, 1, 0, 0);
            break;
        case 2:
            omOvlCallEx(DLL_s03dll, 1, 0, 0);
            break;
    }
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

/* Restore board and player settings and start the direct board-return process. */
void fn_1_18A18(void)
{
    lbl_1_bss_0 = omInitObjMan(27, 8192);
    fn_1_193B0();
    fn_1_6B4();
    lbl_1_bss_34 = 1;
    OSReport(lbl_1_data_BE6);
    HuPrcChildCreate(fn_1_188C0, 12288, 12288, 0, lbl_1_bss_0);
}

/* Close tracked data directories when this overlay requests a resource reset. */
void fn_1_18A98(void)
{
    HuDataDirCloseAll();
}

/* Choose direct board reentry or normal Solo menu setup from the overlay entry event. */
void fn_1_18AB8(void)
{
    s16 coinBonus;
    s16 retryCoinBonus;

    OSReport(lbl_1_data_C0F);
    lbl_1_bss_28 = omovlevtno;
    lbl_1_bss_2C = omovlstat;
    coinBonus = GwPlayer->mgCoinBonus;
    OSReport(lbl_1_data_C45, lbl_1_bss_28, coinBonus);
    if (lbl_1_bss_28 == 1 &&
        ((retryCoinBonus = GwPlayer->mgCoinBonus), retryCoinBonus == 0)) {
        lbl_1_bss_0 = omInitObjMan(27, 8192);
        fn_1_193B0();
        fn_1_6B4();
        lbl_1_bss_34 = 1;
        OSReport(lbl_1_data_BE6);
        HuPrcChildCreate(fn_1_188C0, 12288U, 12288U, 0, lbl_1_bss_0);
        return;
    }

    HuDataDirCloseAll();
    CharDataClose(CHARNO_NONE);
    HuARDirFree(DATANUM(DATA_board, 0));
    HuARDirFree(DATANUM(DATA_board_us, 0));
    HuARDirFree(DATANUM(DATA_capsule, 0));
    OSReport(lbl_1_data_9D3);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);

    lbl_1_bss_30 = 0;
    lbl_1_bss_32 = 0;
    if (GWBankFlagGet(2) != 0) {
        lbl_1_bss_30 = 1;
    }
    if (GWBankFlagGet(4) != 0) {
        lbl_1_bss_32 = 1;
    }
    lbl_1_data_912[0] = 1;
    lbl_1_data_912[1] = 1;
    lbl_1_data_912[2] = 1;
    fn_1_17D38();
}

/* Run overlay constructors and initialize Solo Mode when the module is loaded. */
int _prolog(void) {
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_18AB8();
    return 0;
}

/* Run overlay destructors when the Solo module is unloaded. */
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Ease the entry camera toward its menu framing during camera updates. */
void fn_1_18DB0(OMOBJ *obj, MDCAMERA_WORK *camera)
{
    camera->targetCenter.x = 0.0f;
    camera->targetCenter.y = 65.0f;
    camera->targetCenter.z = -800.0f;
    camera->targetRot.x = -7.25f;
    camera->targetRot.y = 0.0f;
    camera->targetRot.z = 0.0f;
    camera->targetZoom = 2150.0f;
    fn_1_279C(camera, 15.0f);
}

/* Raise and pull back the camera over sixty updates during the scene transition. */
void fn_1_19114(OMOBJ *obj, MDSING_CAMERA *camera)
{
    camera->center.y = fn_1_16F0(camera->targetCenter.y, 275.0f,
        camera->transitionFrame, 60.0f);
    camera->center.z = fn_1_16F0(camera->targetCenter.z, -2900.0f,
        camera->transitionFrame, 60.0f);
    camera->rot.x = fn_1_16F0(camera->targetRot.x, 0.0f,
        camera->transitionFrame, 60.0f);
    camera->transitionFrame += 1.0f;
}

/* Snap the current camera pose to its targets, reset the timer, and start the upward transition. */
void fn_1_1933C(OMOBJ *obj, MDMinidllCameraState *state)
{
    state->transitionFrame = 0.0f;
    memcpy(&state->center, &state->targetCenter, 12U);
    memcpy(&state->rot, &state->targetRot, 12U);
    state->zoom = state->targetZoom;
    lbl_1_bss_1348.callback = fn_1_19114;
}

/* Restore board settings and two player choices after play, resetting saved packs of five or more
 * to zero. */
void fn_1_193B0(void)
{
    s32 miniGamePack;
    s32 boardNumber = GwSystem.boardNo;
    u16 storyDifficulty;

    lbl_1_bss_1340.boardNo = boardNumber - 6;
    storyDifficulty = GwSystem.storyComDif;
    lbl_1_bss_1340.difficulty = storyDifficulty;

    if (GwSystem.mgPack >= 5) {
        GwSystem.mgPack = 0;
    }
    miniGamePack = GwSystem.mgPack;
    lbl_1_bss_1340.minigamePack = miniGamePack;

    lbl_1_bss_1308[0].choiceState = 0;
    lbl_1_bss_1308[0].setupValue = 0;
    lbl_1_bss_1308[0].isComputer = GwPlayer->comF;
    lbl_1_bss_1308[0].difficulty = GwPlayer->comDif;
    lbl_1_bss_1308[0].characterNo = GwPlayer->charNo;
    lbl_1_bss_1308[0].padNo = GwPlayer->padNo;
    lbl_1_bss_1308[1].choiceState = 0;
    lbl_1_bss_1308[1].setupValue = 0;
    lbl_1_bss_1308[1].isComputer = 1;
    lbl_1_bss_1308[1].difficulty = 0;
    lbl_1_bss_1308[1].characterNo = lbl_1_bss_2C;
    lbl_1_bss_1308[1].padNo = 0;
}

/* Start entry-specific music and the incoming wipe before the Solo menu sequence continues. */
s32 fn_1_19570(void)
{
    s16 coinBonus;

    HuPrcSleep(5);
    if (lbl_1_bss_28 == 0 || lbl_1_bss_28 == 2) {
        lbl_1_data_320[0] = HuAudSStreamPlay(MSM_STREAM_W01);
    } else {
        if (lbl_1_bss_28 == 1) {
            coinBonus = GwPlayer->mgCoinBonus;
            if (coinBonus == 1) {
                lbl_1_data_320[1] = HuAudSStreamPlay(MSM_STREAM_MGINST);
                goto music_started;
            }
        }
        lbl_1_data_320[2] = HuAudSStreamPlay(MSM_STREAM_RESULT);
    }
music_started:
    if (lbl_1_bss_28 == 1) {
        Hu3DModelAttrReset(lbl_1_bss_10->mdlId[lbl_1_bss_22], HU3D_ATTR_DISPOFF);
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[lbl_1_bss_22],
                           lbl_1_bss_10->mtnId[35],
                           0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelPosSet(lbl_1_bss_10->mdlId[lbl_1_bss_22], 0.0f, 0.0f, 0.0f);
    }
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    return 1;
}

/* Play the opening guide dialogue and begin fading the board title out. */
s32 fn_1_19748(void)
{
    fn_1_51F4();
    fn_1_3F0C(3, MESSNUM(MESS_SINGLE_SETTING, 0), 1);
    fn_1_3DB4();
    fn_1_778C();
    return 1;
}

/* Start the guide's dialogue motion and install the callback that returns it to idle. */
static inline void fn_1_52B0_inlined_inlined(void)
{
    OMOBJ *obj = lbl_1_bss_C;

    Hu3DMotionShiftSet(
        obj->mdlId[0], obj->mtnId[3], 0.0f, 10.0f, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_514C;
}

/* Read a dialogue choice, applying the requested cancellation policy for this prompt. */
static inline s16 fn_1_381C_inlined_inlined(s16 winNo, s16 mode)
{
    s16 choice = 0;

    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_1398[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_1398[winNo], HUWIN_ATTR_NOCANCEL);
    }
    choice = HuWinChoiceGet(lbl_1_bss_1398[winNo], -1);
    if (mode == 3 && choice == -1) {
        choice = 1;
    }
    return choice;
}

/* Read the active dialogue choice, returning zero when no dialogue window is open. */
static inline s16 fn_1_3F28_inlined_inlined(s16 mode)
{
    if (lbl_1_data_A98[0] != -1) {
        return fn_1_381C_inlined_inlined(lbl_1_data_A98[0], mode);
    }
    return 0;
}

/* Ask whether to leave the current selection screen after the player presses the exit button. */
s16 fn_1_19A20(void)
{
    fn_1_52B0_inlined_inlined();
    fn_1_3F0C(3, (((int)MESSNUM(MESS_SINGLE_SETTING, 2))), 1);
    return fn_1_3F28_inlined_inlined(3);
}

/* Read the active dialogue choice using the caller's cancellation mode. */
static inline s16 fn_1_3F28_inlined(s16 mode)
{
    if (lbl_1_data_A98[0] != -1) {
        return fn_1_3708(lbl_1_data_A98[0], mode);
    }
    return 0;
}

/* Close the Solo archive and begin loading board resources after the opening choice. */
static inline void fn_1_64C_inlined(void)
{
    lbl_1_bss_34 = FALSE;
    OSReport(lbl_1_data_928);
    OSReport(lbl_1_data_95A, _effect);
    OSReport(lbl_1_data_96B, _gamemes);
    OSReport(lbl_1_data_97D, _mgconst);
    OSReport(lbl_1_data_98F, _win);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    HuDataDirClose(DATA_mdsing);
    HuPrcChildCreate(fn_1_3A0, 256, 16384, 0, lbl_1_bss_0);
}

/* Show the exit confirmation and treat cancellation as its second choice. */
static inline s16 fn_1_2EF24_inlined(void)
{
    fn_1_52B0_inlined_inlined();
    fn_1_3F0C(3, (int)MESSNUM(MESS_SINGLE_SETTING, 2), 1);
    return fn_1_3F28_inlined(2);
}

/* Resolve the opening menu choice, preparing board data for the first option. */
s16 fn_1_19D28(void)
{
    s16 menuChoice = 0;
    while (TRUE) {
        HuPrcVSleep();
        fn_1_52B0_inlined_inlined();
        fn_1_3F0C(3, (int)MESSNUM(MESS_SINGLE_SETTING, 1), 1);
        menuChoice = fn_1_3F28_inlined(0);
        if (menuChoice != -1) {
            if (menuChoice == 0) {
                fn_1_3CF8();
                fn_1_B34();
                fn_1_64C_inlined();
            }
            break;
        }
        if (!fn_1_2EF24_inlined()) {
            menuChoice = -1;
            break;
        }
    }
    return menuChoice;
}

/* Start the guide's return to the left-side menu position when its X is at least -100. */
static inline s32 fn_1_5B48_inlined_inlined(void)
{
    Point3d position;
    OMOBJ *guideObject;

    guideObject = lbl_1_bss_C;
    Hu3DModelPosGet(*guideObject->mdlId, &position);
    if (position.x < -100.0f) {
        return 0;
    }
    guideObject->work[0] = 0;
    guideObject->objFunc = fn_1_595C;
    Hu3DMotionShiftSet(*guideObject->mdlId, guideObject->mtnId[1], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    return 1;
}

/* Hide a frameless dialogue immediately or play a framed window's closing animation. */
static inline void fn_1_3474_inlined_inlined(s16 windowNo)
{
    if (windowNo == 0) {
        HuWinDispOff(lbl_1_bss_1398[windowNo]);
        return;
    }
    HuWinExClose(lbl_1_bss_1398[windowNo]);
}

/* Close the active dialogue and clear its cached message and choice. */
static inline void fn_1_3B10_inlined_inlined(void)
{
    if (lbl_1_data_A98[0] != -1) {
        fn_1_3474_inlined_inlined(lbl_1_data_A98[0]);
    }
    lbl_1_data_A98[0] = -1;
    lbl_1_data_A9C[0] = -1;
    lbl_1_data_A9C[1] = -1;
}

/* When the guide's X is above -200, request its return, close dialogue and wait ninety ticks,
 * even if the movement helper declines the request. */
s32 fn_1_1A494(void)
{
    Point3d position;
    Hu3DModelPosGet(*lbl_1_bss_C->mdlId, &position);
    if (position.x > -200.0f) {
        fn_1_5B48_inlined_inlined();
        fn_1_3B10_inlined_inlined();
        HuPrcSleep(90);
    }
}

/* Clear the guide's previous display and show the next setup introduction. */
s32 fn_1_1A610(void)
{
    fn_1_1A494();
    fn_1_51F4();
    fn_1_3F0C(3, MESSNUM(MESS_SINGLE_SETTING, 3), 1);
    fn_1_3DB4();
    return 1;
}

/* Set centered selection instructions and their text speed when the prompt changes. */
static inline void set_message_window_for_1AA1C(s16 windowNo, s32 messageNo, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_1398[windowNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_1398[windowNo], messageNo);
    HuWinMesSpeedSet(lbl_1_bss_1398[windowNo], speed);
    if (lbl_1_data_8EC != messageNo) {
        lbl_1_data_8EC = -1;
    }
}

/* Open or refresh the lower instruction window during human character selection. */
static inline void fn_1_4338_for_1AA1C(s32 messageNo)
{
    if (lbl_1_data_A98[1] == -1) {
        lbl_1_data_A98[1] = 0;
        lbl_1_data_A9C[2] = -1;
        fn_1_35EC(lbl_1_data_A98[1]);
        HuWinPosSet(lbl_1_bss_1398[0], 16.0f, 337.0f);
    }
    if (lbl_1_data_A9C[2] != messageNo) {
        lbl_1_data_A9C[2] = messageNo;
        set_message_window_for_1AA1C(lbl_1_data_A98[1], lbl_1_data_A9C[2], 0);
    }
}

/* Switch the active dialogue window during human character selection. */
static inline void fn_1_3B8C_for_1AA1C(s16 windowNo)
{
    if (lbl_1_data_A98[0] != -1 && lbl_1_data_A98[0] != windowNo) {
        fn_1_365C(lbl_1_data_A98[0]);
    }
    if (lbl_1_data_A98[0] == -1 || lbl_1_data_A98[0] != windowNo) {
        lbl_1_data_A98[0] = windowNo;
        lbl_1_data_A9C[0] = -1;
        lbl_1_data_A9C[1] = -1;
        fn_1_35EC(lbl_1_data_A98[0]);
    }
}

/* Update the active human-selection dialogue only when its message changes. */
static inline void fn_1_3F0C_for_1AA1C(s16 windowNo, s32 messageNo, s16 speed)
{
    fn_1_3B8C_for_1AA1C(windowNo);
    if (lbl_1_data_A9C[0] != messageNo) {
        lbl_1_data_A9C[0] = messageNo;
        lbl_1_data_A9C[1] = -1;
        set_message_window_for_1AA1C(lbl_1_data_A98[0], lbl_1_data_A9C[0], speed);
    }
}

/* Wait for human character confirmation or navigation back, handling requests to exit. */
s16 fn_1_1AA1C(s16 waitForOpeningMotion)
{
    s16 selectedChoice = 0;
    if (waitForOpeningMotion != 0) {
        fn_1_B900(fn_1_C614);
    }
    for (;;) {
        HuPrcVSleep();
        fn_1_B900(fn_1_C264);
        fn_1_4338_for_1AA1C(MESSNUM(MESS_SYS_GUIDE, 10));
        fn_1_51F4();
        fn_1_3F0C_for_1AA1C(3, MESSNUM(MESS_SINGLE_SETTING, 4), 0);
        for (;;) {
            HuPrcVSleep();
            fn_1_3F0C_for_1AA1C(3, MESSNUM(MESS_SINGLE_SETTING, 4), 0);
            if (!fn_1_B8C8()) {
                continue;
            }
            if (lbl_1_bss_1308[0].choiceState == 1) {
                selectedChoice = 0;
            } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                selectedChoice = 1;
            } else if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                selectedChoice = -1;
            } else {
                continue;
            }
            break;
        }
        fn_1_4690();
        fn_1_B900(fn_1_C4C8);
        if (selectedChoice == -1) {
            if (fn_1_19A20()) {
                continue;
            }
            selectedChoice = -1;
        }
        return selectedChoice;
    }
}

/* Show the requested dialogue window while the computer character is being selected. */
static inline void fn_1_35EC_for_1B5EC(s16 windowNo)
{
    if (windowNo == 0) {
        HuWinDispOn(lbl_1_bss_1398[windowNo]);
    } else {
        HuWinExOpen(lbl_1_bss_1398[windowNo]);
    }
}

/* Open or refresh the instruction window during computer character selection. */
static inline void fn_1_4338_for_1B5EC(s32 messageNo)
{
    if (lbl_1_data_A98[1] == -1) {
        lbl_1_data_A98[1] = 0;
        lbl_1_data_A9C[2] = -1;
        fn_1_35EC_for_1B5EC(lbl_1_data_A98[1]);
        HuWinPosSet(lbl_1_bss_1398[0],
            16.0f, 337.0f);
    }
    if (lbl_1_data_A9C[2] != messageNo) {
        lbl_1_data_A9C[2] = messageNo;
        set_message_window_for_1AA1C(lbl_1_data_A98[1],
            lbl_1_data_A9C[2], 0);
    }
}

/* Ask whether to leave computer character selection after an exit request. */
static inline s16 fn_1_19A20_for_1B5EC(void)
{
    fn_1_51F4();
    fn_1_3F0C(3, MESSNUM(MESS_SINGLE_SETTING, 2), 1);
    return fn_1_3E14(2);
}

/* Wait for computer character confirmation or navigation back, handling requests to exit. */
s16 fn_1_1B5EC(s16 unused)
{
    s16 selection = 0;

    for (;;) {
        HuPrcVSleep();
        fn_1_B900(fn_1_CB8C);

        fn_1_4338_for_1B5EC(MESSNUM(MESS_SYS_GUIDE, 10));

        fn_1_51F4();
        fn_1_3F0C(3, MESSNUM(MESS_SINGLE_SETTING, 5), 0);

        for (;;) {
            HuPrcVSleep();
            fn_1_3F0C(3, MESSNUM(MESS_SINGLE_SETTING, 5), 0);
            if (!fn_1_B8C8()) {
                continue;
            }
            if (lbl_1_bss_1308[1].choiceState == 1) {
                selection = 0;
            } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                selection = 1;
            } else if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                selection = -1;
            } else {
                continue;
            }
            break;
        }
        fn_1_4690();
        fn_1_B900(fn_1_CE2C);
        if (selection == -1) {
            if (fn_1_19A20_for_1B5EC()) {
                continue;
            }
            selection = -1;
        }
        return selection;
    }
}

/* Run human and computer character selection, moving cards onward or dismissing them afterward. */
s16 fn_1_1C138(s16 returningFromPreviousStep)
{
    s16 result = 0;
    s16 previousStepPending = 0;

    if (returningFromPreviousStep != 0) {
        fn_1_B900(fn_1_DE98);
    }
    fn_1_B900(fn_1_CF94);
    while (TRUE) {
        HuPrcVSleep();
        result = fn_1_1AA1C(previousStepPending);
        if (result != 0) {
            break;
        }
        result = fn_1_1B5EC(0);
        if (result != 1) {
            break;
        }
        previousStepPending = 1;
    }
    if (result == 0) {
        fn_1_B900(fn_1_D57C);
    } else if (result == 1) {
        fn_1_B900(fn_1_E290);
    }
    return result;
}

/* Run board selection, reject unavailable boards, and animate the chosen transition. */
s16 fn_1_1C3A8(s16 waitForOpeningMotion)
{
    s16 selectedChoice = 0;

    if (waitForOpeningMotion != 0) {
        fn_1_B900(fn_1_FCC8);
    }
    fn_1_B900(fn_1_F6D0);
    for (;;) {
        HuPrcVSleep();
        fn_1_B900(fn_1_F15C);
        fn_1_4338(MESSNUM(MESS_SYS_GUIDE, 10));
        fn_1_51F4();
        fn_1_3F0C(3, lbl_1_bss_1340.boardNo + MESSNUM(MESS_SINGLE_SETTING, 6), 0);
        for (;;) {
            HuPrcVSleep();
            fn_1_3F0C(3, lbl_1_bss_1340.boardNo + MESSNUM(MESS_SINGLE_SETTING, 6), 0);
            if (!fn_1_B8C8()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                if (lbl_1_data_912[lbl_1_bss_1340.boardNo] == 0) {
                    HuAudFXPlay(MSM_SE_CMN_05);
                    continue;
                }
                HuAudFXPlay(MSM_SE_CMN_03);
                selectedChoice = 0;
            } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                selectedChoice = 1;
            } else if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                selectedChoice = -1;
            } else {
                continue;
            }
            break;
        }
        fn_1_4690();
        fn_1_B900(fn_1_F3C8);
        if (selectedChoice == -1) {
            if (fn_1_19A20()) {
                continue;
            }
            selectedChoice = -1;
        }
        break;
    }
    if (selectedChoice == 0) {
        fn_1_B900(fn_1_FD5C);
    } else if (selectedChoice == 1) {
        fn_1_B900(fn_1_10744);
    }
    return selectedChoice;
}

/* Resolve the final setup prompts and play the transition for the selected action. */
s16 fn_1_1D174(void) {
    s16 selectedAction;

    selectedAction = 0;
    fn_1_B900(fn_1_10B18);
    HuPrcVSleep();
    fn_1_51F4();
    fn_1_3F0C(3, MESSNUM(MESS_SINGLE_SETTING, 10), 1);
    if ((selectedAction = fn_1_3E14(2)) == 0) {
        fn_1_3F0C(3, MESSNUM(MESS_SINGLE_SETTING, 12), 1);
        HuPrcSleep(60);
        fn_1_5098();
        HuPrcSleep(60);
        fn_1_3CF8();
        fn_1_B900(fn_1_119E8);
        selectedAction = 0;
    } else {
        fn_1_51F4();
        fn_1_3F0C(3, MESSNUM(MESS_SINGLE_SETTING, 11), 1);
        if ((selectedAction = fn_1_3E14(2)) == 0) {
            fn_1_B900(fn_1_1241C);
            selectedAction = 1;
        } else {
            fn_1_B900(fn_1_12E68);
            selectedAction = 2;
        }
    }
    return selectedAction;
}

/* Begin loading board resources while the menu prepares to leave for play. */
static inline void fn_1_64C_inlined_2(void)
{
    lbl_1_bss_34 = FALSE;
    OSReport(lbl_1_data_928);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    HuDataDirClose(DATANUM(DATA_mdsing, 0));
    HuPrcChildCreate(fn_1_3A0, 256, 16384, 0, lbl_1_bss_0);
}

/* Send the two selected characters along their departure paths before board play begins. */
static inline void fn_1_7900_inlined(void)
{
    MDSING_CHARACTER_SELECTION *selection;
    OMOBJ *obj = lbl_1_bss_10;
    MDSING_MOTION_PATH *path;
    s16 playerNo;

    for (playerNo = 0, selection = &lbl_1_bss_1308[playerNo]; playerNo < 2;
         playerNo++, selection++) {
        path = &lbl_1_bss_CF4[playerNo];
        path->state = 1;
        path->elapsed = 0.0f;
        path->duration = 90.0f;
        Hu3DModelPosGet(obj->mdlId[selection->characterNo], &path->position);
        Hu3DModelPosGet(obj->mdlId[selection->characterNo], &path->controlPosition);
        path->controlPosition.x += 0.5f * path->controlPosition.x;
        path->controlPosition.y = 550.0f;
        path->controlPosition.z = 100.0f;
        Hu3DModelPosGet(obj->mdlId[selection->characterNo], &path->endPosition);
        path->endPosition.y = 225.0f;
        path->endPosition.z = -800.0f;
        Hu3DMotionShiftSet(obj->mdlId[selection->characterNo],
            obj->mtnId[(2 * selection->characterNo) + 1], 0.0f, 10.0f,
            HU3D_MOTATTR_LOOP);
    }
    HuAudFXPlay(MSM_SE_MENU_19);
    obj->objFunc = fn_1_633C;
}

/* Commit player choices and run the character, guide, audio, and camera departure sequence. */
void fn_1_1DCE0(void)
{
    fn_1_6B4();
    fn_1_64C_inlined_2();
    HuPrcSleep(10);
    fn_1_7900_inlined();
    fn_1_57E4();
    HuPrcSleep(10);
    fn_1_34958();
    if (lbl_1_data_320[0] != -1) {
        HuAudSStreamFadeOut(lbl_1_data_320[0], 3000);
        lbl_1_data_320[0] = -1;
    }
    HuAudFXPlay(MSM_SE_MENU_20);
    HuPrcSleep(90);
    lbl_1_bss_1348.callback = fn_1_19114;
    HuPrcSleep(60);
}

/* Show the settings exit confirmation with cancellation selecting the second choice. */
static inline s16 fn_1_2EF24_inlined_2(void)
{
    fn_1_52B0_inlined_inlined();
    fn_1_3F0C(3, (((int)MESSNUM(MESS_SINGLE_SETTING, 2))), 1);
    return fn_1_3F28_inlined(2);
}

/* Run difficulty and minigame-pack selection and animate the settings cards when it ends. */
s16 fn_1_1E104(s16 returningToSelection)
{
    typedef struct MdsingSelectionState {
        s16 choiceType;
        /* These fields are not accessed by this function. */
        s16 unknownAfterChoice;
        s16 unknownBeforeOption;
        s16 selectedOption;
    } MDSING_SELECTION_STATE;

    s16 result = 0;

    if (returningToSelection == 0) {
        OMOBJ *setupObj = lbl_1_bss_24;

        lbl_1_bss_24->mtnId[0] = 0;
        setupObj->work[0] = 0;
        setupObj->work[1] = 0;
        setupObj->work[2] = 0;
        setupObj->work[3] = 0;
        setupObj->objFunc = fn_1_15280;
        while (lbl_1_bss_24->mtnId[0] == 0) {
            HuPrcVSleep();
        }
    } else {
        OMOBJ *setupObj = lbl_1_bss_24;

        lbl_1_bss_24->mtnId[0] = 0;
        setupObj->work[0] = 0;
        setupObj->work[1] = 0;
        setupObj->work[2] = 0;
        setupObj->work[3] = 0;
        setupObj->objFunc = fn_1_15AC4;
        while (lbl_1_bss_24->mtnId[0] == 0) {
            HuPrcVSleep();
        }
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_B900(fn_1_14DA8);
        fn_1_4338((((int)MESSNUM(MESS_SYS_GUIDE, 10))));
        fn_1_52B0_inlined_inlined();
        switch (lbl_1_bss_1340.settingsRow) {
            case 0:
                fn_1_3F0C(3, (((int)MESSNUM(MESS_SINGLE_SETTING, 9))), 0);
                break;
            case 1:
                fn_1_3F0C(3,
                    lbl_1_bss_1340.minigamePack +
                        (((int)MESSNUM(MESS_MGPACK_NAME, 0))),
                    0);
                break;
        }
        while (TRUE) {
            HuPrcVSleep();
            switch (lbl_1_bss_1340.settingsRow) {
                case 0:
                    fn_1_3F0C(3, (((int)MESSNUM(MESS_SINGLE_SETTING, 9))), 0);
                    break;
                case 1:
                    fn_1_3F0C(3,
                        lbl_1_bss_1340.minigamePack +
                            (((int)MESSNUM(MESS_MGPACK_NAME, 0))),
                        0);
                    break;
            }
            if (!fn_1_B8C8()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(MSM_SE_CMN_03);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_4690();
        fn_1_B900(fn_1_15198);
        if (result == -1) {
            if (fn_1_2EF24_inlined_2()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_B900(fn_1_164A8);
    } else if (result == 1) {
        fn_1_B900(fn_1_16E70);
    }
    return result;
}

/* Restore the selected character and settings cards after returning from board play. */
void fn_1_1F30C(void)
{
    s16 characterNo;
    s16 frame;

    characterNo = lbl_1_bss_1308[0].characterNo;
    fn_1_A450(characterNo, characterNo);
    fn_1_A3A8(characterNo, 1, characterNo, 2);
    fn_1_A3A8(characterNo, 3, characterNo, 2);
    fn_1_A3A8(characterNo, 0, characterNo, 1);
    fn_1_A3A8(characterNo, 2, characterNo, 1);

    characterNo = lbl_1_bss_1308[1].characterNo;
    fn_1_A450(characterNo, characterNo);
    fn_1_A3A8(characterNo, 1, characterNo, 2);
    fn_1_A3A8(characterNo, 3, characterNo, 2);
    fn_1_A3A8(characterNo, 0, characterNo, 1);
    fn_1_A3A8(characterNo, 2, characterNo, 1);

    fn_1_A450(14, lbl_1_bss_1340.difficulty + 15);
    fn_1_A450(15, lbl_1_bss_1340.minigamePack + 19);

    for (frame = 0; frame < 11; frame++) {
        HuPrcVSleep();
        fn_1_A4DC(14, frame, 10, &lbl_1_data_7A8[1], 1.0f);
        fn_1_A4DC(15, frame, 10, &lbl_1_data_7A8[2], 1.0f);

        fn_1_A4DC((s16)lbl_1_bss_1308[0].characterNo,
            frame, 10, &lbl_1_data_7A8[21], 1.25f);

        fn_1_A4DC((s16)lbl_1_bss_1308[1].characterNo,
            frame, 10, &lbl_1_data_7A8[22], 1.25f);

        if (frame == 0) {
            fn_1_876C(0, lbl_1_bss_1308[0].padNo);
            fn_1_876C(1, 4);
        }
    }
}

/* Read a player's board coin bonus when choosing the Solo return sequence. */
static inline s16 mdsingCoinBonusGet(s32 playerNo)
{
    return GwPlayer[playerNo].mgCoinBonus;
}

/* When the guide's X is above -200, request its return, close dialogue and wait ninety ticks,
 * even if the movement helper declines the request. */
static inline s32 fn_1_1A494_for_20624(void)
{
    Point3d position;

    Hu3DModelPosGet(lbl_1_bss_C->mdlId[0], &position);
    if (position.x > -200.0f) {
        fn_1_5EF4();
        fn_1_3CF8();
        HuPrcSleep(90);
    }
}

/* Start asynchronous board resource loading while the setup process prepares to leave. */
static inline void fn_1_5E8_for_20624(void)
{
    lbl_1_bss_34 = 0;
    OSReport(lbl_1_data_928);
    OSReport(lbl_1_data_95A, 33);
    OSReport(lbl_1_data_96B, 36);
    OSReport(lbl_1_data_97D, 155);
    OSReport(lbl_1_data_98F, 242);
    HuAMemDump();
    OSReport(lbl_1_data_99D);
    HuDataDirClose(DATANUM(DATA_mdsing, 0));
    HuPrcChildCreate(fn_1_3A0, 256, 16384, 0, lbl_1_bss_0);
}

/* Commit settings and run the departure sequence before the setup process enters board play. */
static inline void fn_1_1DCE0_for_20624(void)
{
    fn_1_6B4();
    fn_1_5E8_for_20624();
    HuPrcSleep(10);
    fn_1_68C0();
    fn_1_57E4();
    HuPrcSleep(10);
    fn_1_34958();
    if (lbl_1_data_320[0] != -1) {
        HuAudSStreamFadeOut(lbl_1_data_320[0], 3000);
        lbl_1_data_320[0] = -1;
    }
    HuAudFXPlay(MSM_SE_MENU_20);
    HuPrcSleep(90);
    lbl_1_bss_1348.callback = fn_1_19114;
    HuPrcSleep(60);
}

/* Run the Solo setup screens in order, returning the requested destination or cancellation. */
s16 fn_1_20624(s16 mode)
{
    s16 step = 0;
    s16 result = 0;
    s16 returning = 0;

    HuPrcSleep(5);
    if (lbl_1_bss_28 == 0 || lbl_1_bss_28 == 2) {
        lbl_1_data_320[0] = HuAudSStreamPlay(MSM_STREAM_W01);
    } else if (lbl_1_bss_28 == 1 && mdsingCoinBonusGet(0) == 1) {
        lbl_1_data_320[1] = HuAudSStreamPlay(MSM_STREAM_MGINST);
    } else {
        lbl_1_data_320[2] = HuAudSStreamPlay(MSM_STREAM_RESULT);
    }
    if (lbl_1_bss_28 == 1) {
        Hu3DModelAttrReset(lbl_1_bss_10->mdlId[lbl_1_bss_22], HU3D_ATTR_DISPOFF);
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[lbl_1_bss_22],
            lbl_1_bss_10->mtnId[35], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelPosSet(lbl_1_bss_10->mdlId[lbl_1_bss_22], 0.0f, 0.0f, 0.0f);
    }
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    if (lbl_1_bss_28 == 0) {
        fn_1_19748();
    } else if (lbl_1_bss_28 == 2) {
        step = 1;
    } else {
        if ((s16)fn_1_256A8() == -1) {
            return 0;
        }
        fn_1_3CF8();
        HuPrcSleep(30);
        fn_1_1A494_for_20624();
        fn_1_1F30C();
        step = 4;
        returning = 0;
    }
    omGameSysInit(lbl_1_bss_0);
    for (;;) {
        HuPrcVSleep();
        switch (step) {
        case 0:
            result = fn_1_19D28();
            if (result == 0) {
                return 1;
            }
            if (result == 1) {
                step = 1;
                returning = 0;
            }
            break;
        case 1:
            fn_1_1A610();
            step = 2;
            returning = 0;
            break;
        case 2:
            result = fn_1_1E104(returning);
            if (result == 0) {
                step = 3;
                returning = 0;
            } else if (result == 1) {
                step = 0;
                returning = 1;
            }
            break;
        case 3:
            result = fn_1_1C138(returning);
            if (result == 0) {
                step = 4;
                returning = 0;
            } else if (result == 1) {
                step = 2;
                returning = 1;
            }
            break;
        case 4:
            result = fn_1_1C3A8(returning);
            if (result == 0) {
                step = 6;
                returning = 0;
            } else if (result == 1) {
                step = 3;
                returning = 1;
            }
            break;
        case 5:
            break;
        case 6:
            result = fn_1_1D174();
            if (result == 0) {
                step = 7;
                returning = 0;
            } else if (result == 1) {
                step = 1;
                returning = 0;
            } else if (result == 2) {
                step = 4;
                returning = 1;
            }
            break;
        case 7:
            fn_1_1DCE0_for_20624();
            return 2;
        }
        if (result == -1) {
            return 0;
        }
    }
}

/* Build the earned-prize and minigame result lists and their numeric total after board play. */
void fn_1_21940(void) {

    typedef struct MdsingMiniGameTypeIds {
        /* Minigame categories in result-list order; -1 selects the flagged category. */
        s16 categoryId[6];
    } MDSING_MINIGAME_TYPE_IDS;
    MDSING_MINIGAME_TYPE_IDS miniGameTypeIds = {{0, 1, 2, 3, 6, -1}};
    s16 flag;
    s16 groupStarted;
    s16 searchIndex;
    s16 index;
    s16 rowCount;

    lbl_1_bss_48 = 0;
    lbl_1_bss_4A = 0;
    lbl_1_bss_4C = 0;
    index = 0;
    while (index < 128) {
        lbl_1_bss_880[index].message = -1;
        lbl_1_bss_880[index].value = -1;
        lbl_1_bss_880[index].color = -1;
        lbl_1_bss_480[index].message = -1U;
        lbl_1_bss_480[index].value = -1;
        lbl_1_bss_480[index].color = -1;
        index += 1;
    }
    lbl_1_bss_47C[0] = 0;
    lbl_1_bss_47C[1] = 0;
    flag = 0;
    rowCount = flag;
    index = rowCount;
    do {
        switch (lbl_1_data_0[index].rowKind) {
        case 10:
            lbl_1_bss_880[rowCount].message = lbl_1_data_0[index].messageNo;
            lbl_1_bss_880[rowCount].value =
                (GwSystem.turnNo - 1) * lbl_1_data_0[index].rewardAmount;
            if (lbl_1_bss_880[rowCount].value <= 0) {
                lbl_1_bss_880[rowCount].value = 0;
            }
            rowCount += 1;
            break;
        case 11:
            lbl_1_bss_880[rowCount].message = lbl_1_data_0[index].messageNo;
            lbl_1_bss_880[rowCount].value = GwPlayer->coin;
            rowCount += 1;
            break;
        case 12:
            if (GwCommon.singleMgWinNum[lbl_1_bss_1340.difficulty] != 0 &&
                (lbl_1_bss_1340.difficulty == 0)) {
                GWSinglePrizeFlagSet(1);
                lbl_1_bss_880[rowCount].message = lbl_1_data_0[index].messageNo;
                lbl_1_bss_880[rowCount].value = GwCommon.singleMgWinNum[lbl_1_bss_1340.difficulty] *
                                                lbl_1_data_0[index].rewardAmount;
                rowCount += 1;
            }
            break;
        case 13:
            if (GwCommon.singleMgWinNum[lbl_1_bss_1340.difficulty] != 0 &&
                (lbl_1_bss_1340.difficulty == 1)) {
                GWSinglePrizeFlagSet(2);
                lbl_1_bss_880[rowCount].message = lbl_1_data_0[index].messageNo;
                lbl_1_bss_880[rowCount].value = GwCommon.singleMgWinNum[lbl_1_bss_1340.difficulty] *
                                                lbl_1_data_0[index].rewardAmount;
                rowCount += 1;
            }
            break;
        case 14:
            if (GwCommon.singleMgWinNum[lbl_1_bss_1340.difficulty] != 0 &&
                (lbl_1_bss_1340.difficulty == 2)) {
                GWSinglePrizeFlagSet(3);
                lbl_1_bss_880[rowCount].message = lbl_1_data_0[index].messageNo;
                lbl_1_bss_880[rowCount].value = GwCommon.singleMgWinNum[lbl_1_bss_1340.difficulty] *
                                                lbl_1_data_0[index].rewardAmount;
                rowCount += 1;
            }
            break;
        case 15:
            if (GwCommon.singleMgWinNum[lbl_1_bss_1340.difficulty] != 0 &&
                (lbl_1_bss_1340.difficulty == 3)) {
                GWSinglePrizeFlagSet(4);
                lbl_1_bss_880[rowCount].message = lbl_1_data_0[index].messageNo;
                lbl_1_bss_880[rowCount].value = GwCommon.singleMgWinNum[lbl_1_bss_1340.difficulty] *
                                                lbl_1_data_0[index].rewardAmount;
                rowCount += 1;
            }
            break;
        case 16:
            if (GwPlayer->hatenaMasuNum > 0) {
                GWSinglePrizeFlagSet(31);
                lbl_1_bss_880[rowCount].message = lbl_1_data_0[index].messageNo;
                lbl_1_bss_880[rowCount].value =
                    GwPlayer->hatenaMasuNum * lbl_1_data_0[index].rewardAmount;
                rowCount += 1;
            }
            break;
        case 17:
            if (GwPlayer->koopaMasuNum > 0) {
                GWSinglePrizeFlagSet(32);
                lbl_1_bss_880[rowCount].message = lbl_1_data_0[index].messageNo;
                lbl_1_bss_880[rowCount].value =
                    GwPlayer->koopaMasuNum * lbl_1_data_0[index].rewardAmount;
                rowCount += 1;
            }
            break;
        case 18:
            if (GWSingleMgRecordNumGet() != 0) {
                GWSinglePrizeFlagSet(5);
                lbl_1_bss_880[rowCount].message = lbl_1_data_0[index].messageNo;
                lbl_1_bss_880[rowCount].value =
                    lbl_1_data_0[index].rewardAmount * GWSingleMgRecordNumGet();
                rowCount += 1;
            }
            break;
        case 19:
            if (GWSingleMgWinNumGet() != 0) {
                GWSinglePrizeFlagSet(6);
                lbl_1_bss_880[rowCount].message = lbl_1_data_0[index].messageNo;
                lbl_1_bss_880[rowCount].value =
                    lbl_1_data_0[index].rewardAmount * GWSingleMgWinNumGet();
                rowCount += 1;
            }
            break;
        case 1:
            searchIndex = index + 1;
            groupStarted = 0;
            while (1) {
                if (GWSinglePrizeFlagGet((s32) lbl_1_data_0[searchIndex].prizeFlag) != 0) {
                    groupStarted = 1;
                } else if (lbl_1_data_0[searchIndex].rowKind == 1) {
                    groupStarted = 0;
                } else if (lbl_1_data_0[searchIndex].rowKind == -1) {
                    groupStarted = 0;
                } else if ((lbl_1_data_0[searchIndex].rowKind == 12) &&
                           (GwCommon.singleMgWinNum[lbl_1_bss_1340.difficulty] != 0) &&
                           (lbl_1_bss_1340.difficulty == 0)) {
                    groupStarted = 1;
                } else if ((lbl_1_data_0[searchIndex].rowKind == 13) &&
                           (GwCommon.singleMgWinNum[lbl_1_bss_1340.difficulty] != 0) &&
                           (lbl_1_bss_1340.difficulty == 1)) {
                    groupStarted = 1;
                } else if ((lbl_1_data_0[searchIndex].rowKind == 14) &&
                           (GwCommon.singleMgWinNum[lbl_1_bss_1340.difficulty] != 0) &&
                           (lbl_1_bss_1340.difficulty == 2)) {
                    groupStarted = 1;
                } else if ((lbl_1_data_0[searchIndex].rowKind == 15) &&
                           (GwCommon.singleMgWinNum[lbl_1_bss_1340.difficulty] != 0) &&
                           (lbl_1_bss_1340.difficulty == 3)) {
                    groupStarted = 1;
                } else if ((lbl_1_data_0[searchIndex].rowKind == 16) &&
                           (GwPlayer->hatenaMasuNum > 0)) {
                    groupStarted = 1;
                } else if ((lbl_1_data_0[searchIndex].rowKind == 17) &&
                           (GwPlayer->koopaMasuNum > 0)) {
                    groupStarted = 1;
                } else if ((lbl_1_data_0[searchIndex].rowKind == 18) &&
                           (GWSingleMgRecordNumGet() != 0)) {
                    groupStarted = 1;
                } else if ((lbl_1_data_0[searchIndex].rowKind == 19) &&
                           (GWSingleMgWinNumGet() != 0)) {
                    groupStarted = 1;
                } else {
                    searchIndex += 1;
                    continue;
                }
                break;
            }
            if (groupStarted == 1) {
                lbl_1_bss_880[rowCount].message = -1;
                lbl_1_bss_880[rowCount].value = -1;
                rowCount += 1;
                lbl_1_bss_880[rowCount].message = lbl_1_data_0[index].messageNo;
                lbl_1_bss_880[rowCount].value = -1;
                lbl_1_bss_880[rowCount].color = 1;
                rowCount += 1;
            }
            break;
        case 0:
            if (GWSinglePrizeFlagGet((s32) lbl_1_data_0[index].prizeFlag) != 0) {
                lbl_1_bss_880[rowCount].message = lbl_1_data_0[index].messageNo;
                lbl_1_bss_880[rowCount].value = lbl_1_data_0[index].rewardAmount;
                rowCount += 1;
            }
            break;
        case -1:
            flag = 1;
            break;
        }
        index += 1;
    } while (flag != 1);
    lbl_1_bss_47C[0] = rowCount;
    flag = 0;
    rowCount = flag;
    for (searchIndex = 0; searchIndex < 6; searchIndex++) {
        groupStarted = 0;
        for (index = 0; index < 100; index++) {
            if (MgDataTbl[index].ovl == 65535) {
                break;
            }
            if (GWSingleMgFlagGet(index + 601) != 0) {
                if ((searchIndex == 5) && ((s32) (MgDataTbl[index].flag & 0x80) != 0)) {
                    if (groupStarted == 0) {
                        groupStarted = 1;
                        if (flag == 1) {
                            lbl_1_bss_480[rowCount].message = -1U;
                            lbl_1_bss_480[rowCount].value = -1;
                            rowCount += 1;
                        }
                        lbl_1_bss_480[rowCount].message =
                            searchIndex + MESSNUM(MESS_OPTION_SINGLE, 109);
                        lbl_1_bss_480[rowCount].value = -1;
                        lbl_1_bss_480[rowCount].color = 1;
                        rowCount += 1;
                        flag = 1;
                    }
                    lbl_1_bss_480[rowCount].message = MgDataTbl[index].nameMes;
                    lbl_1_bss_480[rowCount].value = -1;
                    rowCount += 1;
                } else if (miniGameTypeIds.categoryId[searchIndex] == MgDataTbl[index].type) {
                    if (groupStarted == 0) {
                        groupStarted = 1;
                        if (flag == 1) {
                            lbl_1_bss_480[rowCount].message = -1U;
                            lbl_1_bss_480[rowCount].value = -1;
                            rowCount += 1;
                        }
                        lbl_1_bss_480[rowCount].message =
                            searchIndex + MESSNUM(MESS_OPTION_SINGLE, 109);
                        lbl_1_bss_480[rowCount].value = -1;
                        lbl_1_bss_480[rowCount].color = 1;
                        rowCount += 1;
                        flag = 1;
                    }
                    lbl_1_bss_480[rowCount].message = MgDataTbl[index].nameMes;
                    lbl_1_bss_480[rowCount].value = -1;
                    rowCount += 1;
                }
            }
        }
    }
    lbl_1_bss_47C[1] = rowCount;
    lbl_1_bss_3E = 0;
    for (index = 0; index < lbl_1_bss_47C[0]; index++) {
        if (lbl_1_bss_880[index].value > 0) {
            lbl_1_bss_3E += lbl_1_bss_880[index].value;
        }
    }
    lbl_1_bss_40 = lbl_1_bss_3E / 20;
    if (lbl_1_bss_40 > 99) {
        lbl_1_bss_40 = 99;
    }
}

/* Cached scroll-arrow visibility and results page; -1 forces a display refresh. */
s16 lbl_1_data_CC8[2] = { -1, -1 };

s16 lbl_1_data_CCC = -1;

char lbl_1_data_CCE[] = "%d";

/* Create the result-list windows and hidden background sprite before results are displayed. */
void fn_1_229D8(void)
{
    HUSPR_GROUP *group;
    s16 windowIndex;
    s16 spriteGroup;
    s16 spriteIndex;

    for (windowIndex = 0; windowIndex < 8; windowIndex++) {
        lbl_1_bss_C80[windowIndex] = HuWinExCreateFrame(
            38.0f, (f32)(windowIndex * 26 + 136), 500, 42, -1, 0);
    }
    lbl_1_bss_C80[8] = HuWinExCreateFrame(138.0f, 79.0f, 300, 42, -1, 0);
    lbl_1_bss_C80[9] = HuWinExCreateFrame(138.0f, 375.0f, 300, 42, -1, 0);

    for (windowIndex = 0; windowIndex < 10; windowIndex++) {
        HuWinDispOff(lbl_1_bss_C80[windowIndex]);
        HuWinPriSet(lbl_1_bss_C80[windowIndex], 0);
        HuWinBGTPLvlSet(lbl_1_bss_C80[windowIndex], 0.0f);
    }

    lbl_1_bss_478 = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 150), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_474 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_474, 0, HuSprCreate(lbl_1_bss_478, 100, 0));
    HuSprGrpPosSet(lbl_1_bss_474, 288.0f, 240.0f);
    HuSprGrpScaleSet(lbl_1_bss_474, 40.0f, 40.0f);
    HuSprDrawNoSet(lbl_1_bss_474, 0, 64);
    spriteGroup = lbl_1_bss_474;
    group = &HuSprGrpData[spriteGroup];
    for (spriteIndex = 0; spriteIndex < group->sprNum; spriteIndex++) {
        HuSprAttrSet(spriteGroup, spriteIndex, HUSPR_ATTR_DISPOFF);
    }
}

/* Release the ten result-list windows when the results display is closed. */
void fn_1_22C9C(void)
{
    s16 windowNo;

    for (windowNo = 0; windowNo < 10; windowNo++) {
        HuWinExKill(lbl_1_bss_C80[windowNo]);
    }
}

/* Position a result model from screen coordinates and an optional screen-space offset. */
void fn_1_22CF4(s16 modelNo, const HuVecF *offset, f32 x, f32 y) {
    Point3d position = {0.0f, 0.0f, 0.0f};
    Point3d modelPosition;
    Point3d screenOffset;

    screenOffset.x = x;
    screenOffset.y = y;
    screenOffset.z = 1500.0f;
    if (offset) {
        position.x += offset->x;
        position.y += offset->y;
        position.z += offset->z;
    }
    position.x += screenOffset.x;
    position.y += screenOffset.y;
    position.z += screenOffset.z;
    Hu3D2Dto3D(&position, 1, &modelPosition);
    Hu3DModelPosSet(modelNo, modelPosition.x, modelPosition.y,
                    modelPosition.z);
}

/* Set a three-digit result value, hiding leading zeros in its sprite group. */
void fn_1_22DF4(HUSPR_GROUPID groupId, s32 firstMember, s16 value)
{
    s16 digit;

    digit = value / 100;
    HuSprBankSet(groupId, firstMember, digit);
    if (digit == 0) {
        HuSprAttrSet(groupId, firstMember, HUSPR_ATTR_DISPOFF);
    }
    digit = (value - (digit * 100)) / 10;
    HuSprBankSet(groupId, firstMember + 1, digit);
    if (digit == 0 && value / 100 == 0) {
        HuSprAttrSet(groupId, firstMember + 1, HUSPR_ATTR_DISPOFF);
    }
    digit = value % 10;
    HuSprBankSet(groupId, firstMember + 2, digit);
}

/* Set a five-digit result value, hiding leading zeros in its sprite group. */
void fn_1_22F48(s16 spriteGroup, s32 firstSprite, s16 value)
{
    s16 tenThousands = value / 10000;
    s16 thousands;
    s16 hundreds;
    s16 tens;

    HuSprBankSet(spriteGroup, firstSprite, tenThousands);
    if (tenThousands == 0) {
        HuSprAttrSet(spriteGroup, firstSprite, HUSPR_ATTR_DISPOFF);
    }

    value -= tenThousands * 10000;
    thousands = value / 1000;
    HuSprBankSet(spriteGroup, firstSprite + 1, thousands);
    if (thousands == 0 && tenThousands == 0) {
        HuSprAttrSet(spriteGroup, firstSprite + 1, HUSPR_ATTR_DISPOFF);
    }

    value -= thousands * 1000;
    hundreds = value / 100;
    HuSprBankSet(spriteGroup, firstSprite + 2, hundreds);
    if (hundreds == 0 && thousands == 0 && tenThousands == 0) {
        HuSprAttrSet(spriteGroup, firstSprite + 2, HUSPR_ATTR_DISPOFF);
    }

    value -= hundreds * 100;
    tens = value / 10;
    HuSprBankSet(spriteGroup, firstSprite + 3, tens);
    if (tens == 0 && hundreds == 0 && tenThousands == 0 && thousands == 0) {
        HuSprAttrSet(spriteGroup, firstSprite + 3, HUSPR_ATTR_DISPOFF);
    }

    {
        s16 ones = value % 10;
        HuSprBankSet(spriteGroup, firstSprite + 4, ones);
    }
}

/* Show a result row's model and three-digit value, or hide the row when its value is -1. */
void fn_1_23164(s16 index, s16 value)
{
    MDSING_RESULT_MODEL *state = &lbl_1_bss_3E4[index];

    if (value == -1) {
        fn_1_47F4(state->groupId, HUSPR_ATTR_DISPOFF);
        Hu3DModelAttrSet(state->modelId, HU3D_ATTR_DISPOFF);
        return;
    }

    fn_1_4874(state->groupId, HUSPR_ATTR_DISPOFF);
    Hu3DModelAttrReset(state->modelId, HU3D_ATTR_DISPOFF);

    if (value <= 0) {
        value = 0;
    }
    if (value >= 999) {
        value = 999;
    }
    HuSprBankSet(state->groupId, 0, 10);
    fn_1_22DF4(state->groupId, 1, value);
fn_1_22CF4(state->modelId, NULL,
        427.0f, (index * 26) + 160);
}

/* Set the total's five digit sprites and hide leading zeros when the total changes. */
static inline void set_five_digits(s16 spriteGroup, s32 firstSprite, s16 value)
{
    s16 tenThousands = value / 10000;
    s16 thousands;
    s16 hundreds;
    s16 tens;

    HuSprBankSet(spriteGroup, firstSprite, tenThousands);
    if (tenThousands == 0) {
        HuSprAttrSet(spriteGroup, firstSprite, HUSPR_ATTR_DISPOFF);
    }

    value -= tenThousands * 10000;
    thousands = value / 1000;
    HuSprBankSet(spriteGroup, firstSprite + 1, thousands);
    if (thousands == 0 && tenThousands == 0) {
        HuSprAttrSet(spriteGroup, firstSprite + 1, HUSPR_ATTR_DISPOFF);
    }

    value -= thousands * 1000;
    hundreds = value / 100;
    HuSprBankSet(spriteGroup, firstSprite + 2, hundreds);
    if (hundreds == 0 && thousands == 0 && tenThousands == 0) {
        HuSprAttrSet(spriteGroup, firstSprite + 2, HUSPR_ATTR_DISPOFF);
    }

    value -= hundreds * 100;
    tens = value / 10;
    HuSprBankSet(spriteGroup, firstSprite + 3, tens);
    if (tens == 0 && hundreds == 0 && tenThousands == 0 && thousands == 0) {
        HuSprAttrSet(spriteGroup, firstSprite + 3, HUSPR_ATTR_DISPOFF);
    }

    {
        s16 ones = value % 10;
        HuSprBankSet(spriteGroup, firstSprite + 4, ones);
    }
}

/* Show the result total and its model, or hide them when the value is -1. */
void fn_1_23498(s16 value)
{
    MDSING_RESULT_MODEL *state = &lbl_1_bss_3E4[8];

    if (value == -1) {
        fn_1_47F4(state->groupId, HUSPR_ATTR_DISPOFF);
        Hu3DModelAttrSet(state->modelId, HU3D_ATTR_DISPOFF);
        return;
    }

    fn_1_4874(state->groupId, HUSPR_ATTR_DISPOFF);
    Hu3DModelAttrReset(state->modelId, HU3D_ATTR_DISPOFF);

    if (value <= 0) {
        value = 0;
    }
    /* The signed 16-bit input cannot reach this limit; -31073 is the wrapped form of 99999. */
    if (value >= 99999) {
        value = -31073;
    }
    HuSprBankSet(state->groupId, 0, 10);
    set_five_digits(state->groupId, 1, value);
    fn_1_22CF4(state->modelId, NULL, 312.0f, 397.0f);
}

/* Rotate all nine result models once per object update. */
void fn_1_23874(OMOBJ *unused)
{
    HuVecF rotation;
    MDSING_RESULT_MODEL *entry;
    s16 modelNo;

    modelNo = 0;
    entry = lbl_1_bss_3E4;
    while (modelNo < 9) {
        Hu3DModelRotGet(entry->modelId, &rotation);
        rotation.y += 1.0f;
        if (rotation.y > 360.0f) {
            rotation.y -= 360.0f;
        }
        Hu3DModelRotSetV(entry->modelId, &rotation);
        modelNo += 1;
        entry += 1;
    }
}

/* Move the result-row sprites and their models together by a vertical screen offset. */
void fn_1_23924(f32 yOffset)
{
    s16 index = 0;
    MDSING_RESULT_MODEL *model = lbl_1_bss_3E4;

    for (; index < 8; index++, model++) {
        HuSprGrpPosSet(model->groupId, 451.0f,
            8.0f + (yOffset + (f32)((index * 26) + 151)));
        fn_1_22CF4(model->modelId, NULL, 427.0f,
            8.0f + (yOffset + (f32)((index * 26) + 152)));
    }
    model = &lbl_1_bss_3E4[8];
    HuSprGrpPosSet(model->groupId, 336.0f,
        396.0f + yOffset);
    fn_1_22CF4(model->modelId, NULL,
        312.0f, 397.0f + yOffset);
}

/* Create eight result-row displays and the total, then start their rotation callback. */
void fn_1_23C24(OMOBJ *obj)
{
    MDSING_RESULT_MODEL *entry;
    s16 row;
    s16 member;

    lbl_1_bss_3E0 = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 144), HU_MEMNUM_OVL, HEAP_MODEL));
    row = 0;
    entry = lbl_1_bss_3E4;
    for (; row < 8; row++, entry++) {
        entry->modelId = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 142), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelScaleSet(entry->modelId, 0.4f, 0.4f, 0.4f);
        Hu3DModelLayerSet(entry->modelId, 3);
        entry->groupId = HuSprGrpCreate(6);
        for (member = 0; member < 4; member++) {
            entry->spriteId[member] = HuSprCreate(lbl_1_bss_3E0, 0, 0);
            HuSprGrpMemberSet(
                entry->groupId, member, entry->spriteId[member]);
            HuSprPosSet(entry->groupId, member, member * 22, 0.0f);
        }
        HuSprGrpPosSet(entry->groupId, 451.0f, row * 26 + 159);
        HuSprGrpScaleSet(entry->groupId, 0.8f, 0.8f);

        fn_1_23164(row, -1);
    }

    entry = &lbl_1_bss_3E4[8];
    entry->modelId = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 142), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelScaleSet(entry->modelId, 0.4f, 0.4f, 0.4f);
    Hu3DModelLayerSet(entry->modelId, 3);
    entry->groupId = HuSprGrpCreate(6);
    for (member = 0; member < 6; member++) {
        entry->spriteId[member] = HuSprCreate(lbl_1_bss_3E0, 0, 0);
        HuSprGrpMemberSet(
            entry->groupId, member, entry->spriteId[member]);
        HuSprPosSet(entry->groupId, member, member * 22, 0.0f);
    }
    HuSprGrpPosSet(entry->groupId, 336.0f, 396.0f);
    HuSprGrpScaleSet(entry->groupId, 0.8f, 0.8f);

    fn_1_23498(-1);
    obj->objFunc = fn_1_23874;
}

/* Release the nine result models when the results object is removed. */
void fn_1_24674(OMOBJ *obj) {
    MDSING_RESULT_MODEL *entry;
    s16 modelNo;

    if (obj) {
        modelNo = 0;
        entry = lbl_1_bss_3E4;
        goto loop_check;
release_model:
        Hu3DModelKill(entry->modelId);
        modelNo += 1;
        entry += 1;
loop_check:
        if (modelNo < 9) {
            goto release_model;
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* Spawn at most one falling reward model per update and recycle each model when it lands. */
void fn_1_246F8(OMOBJ *manager)
{
    Point3d position;
    f32 zPosition;
    s16 spawned;
    s16 modelIndex;

    spawned = 0;
    for (modelIndex = 0; modelIndex < 64; modelIndex++) {
        if (lbl_1_bss_358[modelIndex] == 0 &&
            lbl_1_bss_3D8 > 0 && spawned == 0) {
            spawned = 1;
            lbl_1_bss_358[modelIndex] = 1;
            lbl_1_bss_3D8--;
            zPosition = frandmod(100) - 150;
            Hu3DModelPosSet(manager->mdlId[modelIndex],
                frandmod(150) - 75, 500.0f, zPosition);
            Hu3DModelRotSet(manager->mdlId[modelIndex],
                0.0f, frandmod(360), 0.0f);
            Hu3DModelAttrReset(manager->mdlId[modelIndex], HU3D_ATTR_DISPOFF);
            lbl_1_bss_58[modelIndex].fallSpeed = frandmod(5);
            lbl_1_bss_58[modelIndex].rotationSpeed =
                frandmod(30) - 15;
        } else if (lbl_1_bss_358[modelIndex] == 1) {
            Hu3DModelPosGet(manager->mdlId[modelIndex], &position);
            position.y -= 15.0f +
                lbl_1_bss_58[modelIndex].fallSpeed;
            Hu3DModelPosSetV(manager->mdlId[modelIndex], &position);
            if (position.y <= 150.0f) {
                HuAudFXPlay(MSM_SE_CMN_08);
                position.y = 150.0f;
                Hu3DModelPosSetV(manager->mdlId[modelIndex], &position);
                fn_1_34A18(&position);
                lbl_1_bss_358[modelIndex] = 0;
                Hu3DModelAttrSet(manager->mdlId[modelIndex], HU3D_ATTR_DISPOFF);
            }
            Hu3DModelRotGet(manager->mdlId[modelIndex], &position);
            position.y += lbl_1_bss_58[modelIndex].rotationSpeed;
            Hu3DModelRotSetV(manager->mdlId[modelIndex], &position);
        }
    }

    if (lbl_1_bss_3D8 == 0) {
        for (modelIndex = 0; modelIndex < 64; modelIndex++) {
            if (lbl_1_bss_358[modelIndex] != 0) {
                break;
            }
        }
        if (modelIndex == 64) {
            manager->objFunc = NULL;
        }
    }
}

/* Queue up to five hundred falling reward models and begin their update callback. */
void fn_1_24B1C(s16 spawnCount) {
    OMOBJ *obj = lbl_1_bss_3DC;
    s16 modelNo;

    lbl_1_bss_3D8 = spawnCount;
    if (lbl_1_bss_3D8 > 0) {
        if (lbl_1_bss_3D8 >= 500) {
            lbl_1_bss_3D8 = 500;
        }
        modelNo = 0;
        goto loop_check;
clear_model_slot:
        lbl_1_bss_358[modelNo] = 0;
        modelNo += 1;
loop_check:
        if (modelNo < 64) {
            goto clear_model_slot;
        }
        obj->objFunc = fn_1_246F8;
    }
}

/* Create the hidden pool of sixty-four linked models used by the falling reward display. */
void fn_1_24BC8(OMOBJ *obj) {
    s16 modelNo;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    modelNo = 0;
    goto loop_check;
create_falling_model:
    if (modelNo == 0) {
        obj->mdlId[modelNo] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 142), HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        obj->mdlId[modelNo] = Hu3DModelLink(*obj->mdlId);
    }
    Hu3DModelScaleSet(obj->mdlId[modelNo], 0.5f, 0.5f, 0.5f);
    Hu3DModelAttrSet(obj->mdlId[modelNo], HU3D_ATTR_DISPOFF);
    modelNo += 1;
loop_check:
    if (modelNo < 64) {
        goto create_falling_model;
    }
    obj->objFunc = NULL;
}

/* Release the falling reward model pool when the results display is removed. */
void fn_1_24CC8(OMOBJ *obj)
{
    s16 modelNo;

    if (obj) {
        for (modelNo = 63; modelNo >= 0; modelNo--) {
            Hu3DModelKill(obj->mdlId[modelNo]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* Fade the converted-reward model and its number out over ten object updates. */
void fn_1_24D44(OMOBJ *obj)
{
    HuVecF position;
    HuVecF screenPosition;
    f32 alpha;
    f32 elapsedFrames = (f32) obj->work[1];
    f32 modelAlpha;

    if (elapsedFrames <= 0.0f) {
        alpha = 1.0f;
    } else if (elapsedFrames >= 10.0f) {
        alpha = 0.0f;
    } else {
        alpha = 1.0f + (-1.0f * (1.0f -
            cos((M_PI * (9.0f * elapsedFrames)) / 180.0)));
    }
    modelAlpha = alpha;

    position.x = -100.0f;
    position.y = 300.0f;
    position.z = 0.0f;
    HuSprGrpTPLvlSet((s16) obj->work[0], modelAlpha);
    Hu3DModelTPLvlSet(*obj->mdlId, modelAlpha);
    if (++obj->work[1] > 10U) {
        HUSPR_GROUP *spriteGroupData;
        s16 spriteGroup;
        s16 spriteIndex;

        Hu3DModelAttrSet(*obj->mdlId, HU3D_ATTR_DISPOFF);
        spriteGroup = (s16) obj->work[0];
        spriteGroupData = &HuSprGrpData[spriteGroup];
        for (spriteIndex = 0; spriteIndex < spriteGroupData->sprNum; spriteIndex++) {
            HuSprAttrSet(spriteGroup, spriteIndex, HUSPR_ATTR_DISPOFF);
        }
        obj->objFunc = NULL;
    }
    Hu3DModelPosSetV(*obj->mdlId, &position);
    Hu3D3Dto2D(&position, 1, &screenPosition);
    HuSprGrpPosSet((s16) obj->work[0], 80.0f + screenPosition.x,
        20.0f + screenPosition.y);
}

/* Start the converted-reward display's ten-frame fade out. */
void fn_1_24FA8(void)
{
    OMOBJ *obj = lbl_1_bss_50;

    obj->work[1] = 0;
    obj->objFunc = fn_1_24D44;
}

/* Raise and fade the converted-reward model and its number into view over ten updates. */
void fn_1_24FDC(OMOBJ *obj)
{
    HuVecF position;
    HuVecF screenPosition;
    f32 modelAlpha;

    modelAlpha = fn_1_1608(0.0f, 1.0f, obj->work[1], 10.0f);

    position.x = -100.0f;
    position.y = 250.0f + (50.0f * modelAlpha);
    position.z = 0.0f;
    HuSprGrpTPLvlSet((s16) obj->work[0], modelAlpha);
    Hu3DModelTPLvlSet(*obj->mdlId, modelAlpha);
    if (++obj->work[1] > 10U) {
        obj->objFunc = NULL;
    }
    Hu3DModelPosSetV(*obj->mdlId, &position);
    Hu3D3Dto2D(&position, 1, &screenPosition);
    HuSprGrpPosSet((s16) obj->work[0], 80.0f + screenPosition.x,
        20.0f + screenPosition.y);
}

/* Prepare the converted reward count, clamp it to 0-99, and start its entrance animation. */
void fn_1_251D8(void)
{
    OMOBJ *obj = lbl_1_bss_50;

    fn_1_4874((s16) obj->work[0], HUSPR_ATTR_DISPOFF);
    if (lbl_1_bss_40 >= 99) {
        lbl_1_bss_40 = 99;
    }
    if (lbl_1_bss_40 <= 0) {
        lbl_1_bss_40 = 0;
    }
    fn_1_22DF4((s16) obj->work[0], 0, lbl_1_bss_40);
    HuSprBankSet((s16) obj->work[0], 0, 10);
    HuSprAttrReset((s16) obj->work[0], 0, HUSPR_ATTR_DISPOFF);
    Hu3DModelPosSet(obj->mdlId[0], -100.0f, 200.0f, 0.0f);
    Hu3DModelAttrReset(obj->mdlId[0], HU3D_ATTR_DISPOFF);
    obj->work[1] = 0;
    obj->objFunc = fn_1_24FDC;
}

/* Create the hidden converted-reward model and its three digit sprites during results setup. */
void fn_1_2542C(OMOBJ *obj)
{
    s16 digitIndex;
    HUSPRID spriteId;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    *obj->mdlId = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 143), HU_MEMNUM_OVL, HEAP_MODEL));
    *obj->mtnId = Hu3DMotionIDGet(*obj->mdlId);
    Hu3DModelLayerSet(*obj->mdlId, 3);
    Hu3DModelAttrSet(*obj->mdlId, HU3D_ATTR_DISPOFF);
    Hu3DMotionShiftSet(*obj->mdlId, *obj->mtnId, 0.0f, 0.0f, HU3D_MOTATTR_LOOP);

    lbl_1_bss_54 = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 144), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->work[0] = (u32) HuSprGrpCreate(3);
    for (digitIndex = 0; digitIndex < 3; digitIndex++) {
        spriteId = HuSprCreate(lbl_1_bss_54, 0, 0);
        HuSprGrpMemberSet((s16) obj->work[0], digitIndex, spriteId);
        HuSprPosSet((s16) obj->work[0], digitIndex,
            (f32) (digitIndex * 22), 0.0f);
    }
    HuSprGrpPosSet((s16) obj->work[0], 320.0f, 240.0f);
    HuSprGrpScaleSet((s16) obj->work[0], 2.0f, 2.0f);

    {
        HUSPR_GROUP *spriteGroupData;
        s16 spriteGroup;
        s16 memberIndex;

        spriteGroup = (s16) obj->work[0];
        spriteGroupData = &HuSprGrpData[spriteGroup];
        for (memberIndex = 0; memberIndex < spriteGroupData->sprNum; memberIndex++) {
            HuSprAttrSet(spriteGroup, memberIndex, HUSPR_ATTR_DISPOFF);
        }
    }
    obj->objFunc = NULL;
}

/* Release the converted-reward model and its object during results teardown. */
void fn_1_25648(OMOBJ *obj)
{
    if (obj) {
        /* The motion release uses the model slot's ID before removing the model. */
        Hu3DMotionKill(obj->mdlId[0]);
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

/* Run the post-board results sequence and return its selected action to the setup process. */
s32 fn_1_256A8(void)
{
    return fn_1_2C158();
}

/* Restore board choices, calculate results, and create the result display objects on board
 * return. */
void fn_1_256C8(void)
{
    HUSPR_GROUP *group;
    s16 windowIndex;
    s16 spriteGroup;
    s16 spriteIndex;

    {
        s32 miniGamePack;
        s32 boardNumber = GwSystem.boardNo;
        u16 storyDifficulty;

        lbl_1_bss_1340.boardNo = boardNumber - 6;
        storyDifficulty = GwSystem.storyComDif;
        lbl_1_bss_1340.difficulty = storyDifficulty;

        /* Reset saved packs of five or more before restoring the menu choice. */
        if (GwSystem.mgPack >= 5) {
            GwSystem.mgPack = 0;
        }
        miniGamePack = GwSystem.mgPack;
        lbl_1_bss_1340.minigamePack = miniGamePack;

        lbl_1_bss_1308[0].choiceState = 0;
        lbl_1_bss_1308[0].setupValue = 0;
        lbl_1_bss_1308[0].isComputer = GwPlayer->comF;
        lbl_1_bss_1308[0].difficulty = GwPlayer->comDif;
        lbl_1_bss_1308[0].characterNo = GwPlayer->charNo;
        lbl_1_bss_1308[0].padNo = GwPlayer->padNo;
        lbl_1_bss_1308[1].choiceState = 0;
        lbl_1_bss_1308[1].setupValue = 0;
        lbl_1_bss_1308[1].isComputer = 1;
        lbl_1_bss_1308[1].difficulty = 0;
        lbl_1_bss_1308[1].characterNo = lbl_1_bss_2C;
        lbl_1_bss_1308[1].padNo = 0;
    }
    fn_1_21940();
    HuSprExecLayerSet(64, 2);
    HuSprDrawNoSet(lbl_1_bss_1292[6], 0, 64);
    HuSprDrawNoSet(lbl_1_bss_1292[6], 2, 64);

    for (windowIndex = 0; windowIndex < 8; windowIndex++) {
        lbl_1_bss_C80[windowIndex] = HuWinExCreateFrame(
            38.0f, (f32)(windowIndex * 26 + 136), 500, 42, -1, 0);
    }
    lbl_1_bss_C80[8] = HuWinExCreateFrame(138.0f, 79.0f, 300, 42, -1, 0);
    lbl_1_bss_C80[9] = HuWinExCreateFrame(138.0f, 375.0f, 300, 42, -1, 0);

    for (windowIndex = 0; windowIndex < 10; windowIndex++) {
        HuWinDispOff(lbl_1_bss_C80[windowIndex]);
        HuWinPriSet(lbl_1_bss_C80[windowIndex], 0);
        HuWinBGTPLvlSet(lbl_1_bss_C80[windowIndex], 0.0f);
    }

    lbl_1_bss_478 =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_mdsing, 150), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_474 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_474, 0, HuSprCreate(lbl_1_bss_478, 100, 0));
    HuSprGrpPosSet(lbl_1_bss_474, 288.0f, 240.0f);
    HuSprGrpScaleSet(lbl_1_bss_474, 40.0f, 40.0f);
    HuSprDrawNoSet(lbl_1_bss_474, 0, 64);
    spriteGroup = lbl_1_bss_474;
    group = &HuSprGrpData[spriteGroup];
    for (spriteIndex = 0; spriteIndex < group->sprNum; spriteIndex++) {
        HuSprAttrSet(spriteGroup, spriteIndex, HUSPR_ATTR_DISPOFF);
    }

    lbl_1_bss_44 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, fn_1_23C24);
    lbl_1_bss_3DC = omAddObjEx(lbl_1_bss_0, 4096, 64, 64, -1, fn_1_24BC8);
    lbl_1_bss_50 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, fn_1_2542C);
}

/* Release the result windows, row models, falling models, and converted-reward object. */
void fn_1_25BF4(void)
{
    fn_1_22C9C();
    fn_1_24674(lbl_1_bss_44);
    fn_1_24CC8(lbl_1_bss_3DC);
    fn_1_25648(lbl_1_bss_50);
}

/* Slide the results background into view, then show its list windows. */
void fn_1_25D48(void)
{
    HUSPR_GROUP *group;
    HUSPR_GROUPID groupId = lbl_1_bss_1292[6];
    f32 animatedY;
    f32 spriteY;
    s16 spriteIndex;
    s16 elapsedFrames;

    group = &HuSprGrpData[groupId];
    for (spriteIndex = 0; spriteIndex < group->sprNum; spriteIndex++) {
        HuSprAttrReset(groupId, spriteIndex, HUSPR_ATTR_DISPOFF);
    }
    HuSprGrpPosSet(lbl_1_bss_1292[6], 288.0f, 1000.0f);

    for (elapsedFrames = 0; elapsedFrames < 30; elapsedFrames++) {
        HuPrcVSleep();
        if ((f32) elapsedFrames <= 0.0f) {
            animatedY = 1000.0f;
        } else if ((f32) elapsedFrames >= 30.0f) {
            animatedY = 240.0f;
        } else {
            animatedY = 1000.0f + (-760.0f *
                sin((M_PI * (3.0f * (f32) elapsedFrames)) / 180.0));
        }
        spriteY = animatedY;
        HuSprGrpPosSet(lbl_1_bss_1292[6], 288.0f, spriteY);
    }

    for (elapsedFrames = 0; elapsedFrames < 10; elapsedFrames++) {
        HuWinDispOn(lbl_1_bss_C80[elapsedFrames]);
    }
    HuSprGrpPosSet(lbl_1_bss_1292[6], 288.0f, 240.0f);
}

/* Slide the result panel, windows, and numeric models off screen before hiding them. */
void fn_1_25FD4(void)
{
    s16 entryNo;
    s16 windowNo;
    s16 modelIndex;
    s16 score;
    f32 animatedY;
    f32 offsetY;

    for (entryNo = 0; entryNo < 30; entryNo++) {
        HuPrcVSleep();
        if ((f32) entryNo <= 0.0f) {
            animatedY = 240.0f;
        } else if ((f32) entryNo >= 30.0f) {
            animatedY = -760.0f;
        } else {
            animatedY = 240.0f + (-1000.0f *
                (1.0f - cos((3.141592653589793 * (f64) (3.0f * (f32) entryNo)) / 180.0)));
        }
        offsetY = animatedY;
        HuSprGrpPosSet(lbl_1_bss_1292[6], 288.0f, offsetY);

        for (windowNo = 0; windowNo < 8; windowNo++) {
            HuWinPosSet(lbl_1_bss_C80[windowNo], 38.0f,
                8.0f + ((136.0f + (-240.0f + offsetY) + (f32) (windowNo * 26) - 2.0f) - 6.0f));
        }
        HuWinPosSet(lbl_1_bss_C80[8], 138.0f,
            12.0f + ((128.0f + (-240.0f + offsetY) - 56.0f) - 5.0f));
        HuWinPosSet(lbl_1_bss_C80[9], 138.0f,
            (252.0f + (128.0f + (-240.0f + offsetY))) - 5.0f);

        fn_1_23924(-240.0f + offsetY);
    }

    for (entryNo = 0; entryNo < 10; entryNo++) {
        HuWinDispOff(lbl_1_bss_C80[entryNo]);
    }

    for (entryNo = 0; entryNo < 8; entryNo++) {
        fn_1_23164(entryNo, -1);
    }

    fn_1_23498(-1);

    fn_1_47F4(lbl_1_bss_1292[6], HUSPR_ATTR_DISPOFF);
}

/* Show a result-list navigation arrow at the requested screen position and enlarged size. */
static inline void fn_1_8BC0_for_26DFC(
    s16 memberNo, Vec *worldPosition, float xOffset, float yOffset)
{
    fn_1_8B28(memberNo, worldPosition, xOffset, yOffset);
    HuSprScaleSet(lbl_1_bss_1292[2], memberNo,
        2.0f, 2.0f);
    HuSprAttrReset(
        lbl_1_bss_1292[2], memberNo, HUSPR_ATTR_DISPOFF);
}

/* Refresh the selected results page, visible rows, total, and available scroll arrows. */
void fn_1_26DFC(void)
{
    s16 rowNo;
    s16 value;
    s16 groupId;
    s16 groupOrMemberIndex;

    if (lbl_1_data_CCC != lbl_1_bss_48) {
        lbl_1_data_CC8[0] = -1;
        lbl_1_data_CC8[1] = -1;
        lbl_1_data_CCC = lbl_1_bss_48;
    }

    if (lbl_1_bss_48 == 0) {
        if (lbl_1_data_CC8[0] != 1) {
            fn_1_8BC0_for_26DFC(2, NULL, 288.0f, 130.0f);
            lbl_1_bss_C94[2] = 2.0f;
            lbl_1_data_AC8[2] = 1;
            lbl_1_data_CC8[0] = 1;
        }
        if (lbl_1_data_CC8[1] != 1) {
            fn_1_8BC0_for_26DFC(3, NULL, 288.0f, 364.0f);
            lbl_1_bss_C94[3] = 2.0f;
            lbl_1_data_AC8[3] = 1;
            lbl_1_data_CC8[1] = 1;
        }
        if (lbl_1_bss_4A <= 0) {
            HuSprAttrSet(lbl_1_bss_1292[2], 2, HUSPR_ATTR_DISPOFF);
            lbl_1_data_AC8[2] = 0;
            lbl_1_data_CC8[0] = 0;
        }
        if (lbl_1_bss_4A >= lbl_1_bss_47C[lbl_1_bss_48] - 8) {
            HuSprAttrSet(lbl_1_bss_1292[2], 3, HUSPR_ATTR_DISPOFF);
            lbl_1_data_AC8[3] = 0;
            lbl_1_data_CC8[1] = 0;
        }
    } else {
        if (lbl_1_data_CC8[0] != 1) {
            fn_1_8BC0_for_26DFC(2, NULL, 288.0f, 130.0f);
            lbl_1_bss_C94[2] = 2.0f;
            lbl_1_data_AC8[2] = 1;
            lbl_1_data_CC8[0] = 1;
        }
        if (lbl_1_data_CC8[1] != 1) {
            fn_1_8BC0_for_26DFC(3, NULL, 288.0f, 364.0f);
            lbl_1_bss_C94[3] = 2.0f;
            lbl_1_data_AC8[3] = 1;
            lbl_1_data_CC8[1] = 1;
        }
        if (lbl_1_bss_4C == 0) {
            HuSprAttrSet(lbl_1_bss_1292[2], 2, HUSPR_ATTR_DISPOFF);
            lbl_1_data_AC8[2] = 0;
            lbl_1_data_CC8[0] = 0;
        }
        if (lbl_1_bss_4C >= lbl_1_bss_47C[lbl_1_bss_48] - 8) {
            HuSprAttrSet(lbl_1_bss_1292[2], 3, HUSPR_ATTR_DISPOFF);
            lbl_1_data_AC8[3] = 0;
            lbl_1_data_CC8[1] = 0;
        }
    }

    HuWinAttrSet(lbl_1_bss_C80[8], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_C80[8], lbl_1_bss_48 + MESSNUM(MESS_OPTION_SINGLE, 102));
    HuWinMesSpeedSet(lbl_1_bss_C80[8], 0);
    HuWinMesSet(lbl_1_bss_C80[9], MESSNUM(MESS_SINGLE_RESULT, 6));
    HuWinMesSpeedSet(lbl_1_bss_C80[9], 0);

    fn_1_23498(lbl_1_bss_3E);

    for (rowNo = 0; rowNo < 8; rowNo++) {
        if (lbl_1_bss_48 == 0) {
            HuWinAttrReset(lbl_1_bss_C80[rowNo], HUWIN_ATTR_ALIGN_CENTER);
            if (lbl_1_bss_880[rowNo + lbl_1_bss_4A].message == -1) {
                HuWinHomeClear(lbl_1_bss_C80[rowNo]);
            } else {
                if (lbl_1_bss_880[rowNo + lbl_1_bss_4A].color == 1) {
                    HuWinAttrSet(lbl_1_bss_C80[rowNo], HUWIN_ATTR_OUTLINE);
                    HuWinMesColSet(lbl_1_bss_C80[rowNo], 4);
                } else {
                    HuWinAttrReset(lbl_1_bss_C80[rowNo], HUWIN_ATTR_OUTLINE);
                    HuWinMesColSet(lbl_1_bss_C80[rowNo], 7);
                }
                HuWinMesSet(lbl_1_bss_C80[rowNo],
                    lbl_1_bss_880[rowNo + lbl_1_bss_4A].message);
            }
            HuWinMesSpeedSet(lbl_1_bss_C80[rowNo], 0);
            fn_1_23164(rowNo,
                lbl_1_bss_880[rowNo + lbl_1_bss_4A].value);
        } else {
            HuWinAttrSet(lbl_1_bss_C80[rowNo], HUWIN_ATTR_ALIGN_CENTER);
            if (lbl_1_bss_480[rowNo + lbl_1_bss_4C].message == -1) {
                HuWinHomeClear(lbl_1_bss_C80[rowNo]);
            } else {
                if (lbl_1_bss_480[rowNo + lbl_1_bss_4C].color == 1) {
                    HuWinAttrSet(lbl_1_bss_C80[rowNo], HUWIN_ATTR_OUTLINE);
                    HuWinMesColSet(lbl_1_bss_C80[rowNo], 4);
                } else {
                    HuWinAttrReset(lbl_1_bss_C80[rowNo], HUWIN_ATTR_OUTLINE);
                    HuWinMesColSet(lbl_1_bss_C80[rowNo], 7);
                }
                HuWinMesSet(lbl_1_bss_C80[rowNo],
                    lbl_1_bss_480[rowNo + lbl_1_bss_4C].message);
            }
            HuWinMesSpeedSet(lbl_1_bss_C80[rowNo], 0);
            fn_1_23164(rowNo,
                lbl_1_bss_480[rowNo + lbl_1_bss_4C].value);
        }
    }
}

/* Fade the results backdrop out and resume the menu scene's paused animations. */
void fn_1_282C0(void)
{
    f32 alpha;
    s16 animationIndex;

    animationIndex = 0;
    while (animationIndex < 30) {
        HuPrcVSleep();
        alpha = fn_1_16F0(1.0f, 0.0f, animationIndex, 30.0f);
        HuSprGrpTPLvlSet(lbl_1_bss_474, alpha);
        animationIndex++;
    }
    HuSprGrpTPLvlSet(lbl_1_bss_474, 0.0f);
    fn_1_47F4(lbl_1_bss_474, HUSPR_ATTR_DISPOFF);
    HuPrcSleep(30);
    Hu3DModelAttrReset(lbl_1_bss_10->mdlId[lbl_1_bss_22], HU3D_MOTATTR_PAUSE);
    animationIndex = 0;
    while (animationIndex < 3) {
        Hu3DModelAttrReset(lbl_1_bss_4->mdlId[animationIndex], HU3D_MOTATTR_PAUSE);
        animationIndex++;
    }
    Hu3DModelAttrReset(lbl_1_bss_C->mdlId[0], HU3D_MOTATTR_PAUSE);
    lbl_1_bss_8->objFunc = fn_1_7F34;
    HuPrcSleep(30);
}

/* Pause the menu scene and fade the results backdrop in before opening the list panel. */
void fn_1_28568(void)
{
    f32 alpha;
    s16 animationIndex;

    HuPrcSleep(30);
    animationIndex = 0;
    while (animationIndex < 3) {
        Hu3DModelAttrSet(lbl_1_bss_4->mdlId[animationIndex], HU3D_MOTATTR_PAUSE);
        animationIndex++;
    }
    Hu3DModelAttrSet(lbl_1_bss_C->mdlId[0], HU3D_MOTATTR_PAUSE);
    lbl_1_bss_8->objFunc = NULL;
    Hu3DModelAttrSet(lbl_1_bss_10->mdlId[lbl_1_bss_22], HU3D_MOTATTR_PAUSE);
    HuSprGrpTPLvlSet(lbl_1_bss_474, 0.0f);
    fn_1_4874(lbl_1_bss_474, HUSPR_ATTR_DISPOFF);
    animationIndex = 0;
    while (animationIndex < 30) {
        HuPrcVSleep();
        alpha = fn_1_16F0(0.0f, 1.0f, animationIndex, 30.0f);
        HuSprGrpTPLvlSet(lbl_1_bss_474, alpha);
        animationIndex++;
    }
    HuSprGrpTPLvlSet(lbl_1_bss_474, 1.0f);
    HuPrcSleep(30);
}

/* Play the guide's introductory results dialogue before showing the result lists. */
s32 fn_1_28828(void)
{
    OMOBJ *obj = lbl_1_bss_C;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[3],
        0.0f, 10.0f, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_514C;
    fn_1_3F0C(3, MESSNUM(MESS_SINGLE_RESULT, 0), 1);
    fn_1_3DB4();
    fn_1_3CF8();
    return TRUE;
}

/* Slide the results panel into view and show its list windows. */
static inline void mdsingResultPanelOpen(void)
{
    f32 animatedY;
    s16 entryNo;

    Hu3DZClearLayerSet(10);
    fn_1_4874(lbl_1_bss_1292[6], HUSPR_ATTR_DISPOFF);
    HuSprGrpPosSet(lbl_1_bss_1292[6], 288.0f, 1000.0f);
    for (entryNo = 0; entryNo < 30; entryNo++)
    {
        HuPrcVSleep();
        animatedY = fn_1_1608(1000.0f, 240.0f, entryNo, 30.0f);
        HuSprGrpPosSet(lbl_1_bss_1292[6], 288.0f, animatedY);
    }
    for (entryNo = 0; entryNo < 10; entryNo++)
    {
        HuWinDispOn(lbl_1_bss_C80[entryNo]);
    }
    HuSprGrpPosSet(lbl_1_bss_1292[6], 288.0f, 240.0f);
}

/* Open the results instruction window and position it beneath the list panel. */
static inline void show_result_window(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_1398[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_1398[winNo]);
    }
    HuWinPosSet(lbl_1_bss_1398[0], 16.0f, 408.0f);
}

/* Open or refresh the instruction message while the results list is being viewed. */
static inline void fn_1_44E4_for_28B78(s32 messageNo)
{
    if ((&lbl_1_data_A98[0])[1] == -1) {
        (&lbl_1_data_A98[0])[1] = 0;
        lbl_1_data_A9C[2] = -1;
        show_result_window((&lbl_1_data_A98[0])[1]);
    }
    if (lbl_1_data_A9C[2] != messageNo) {
        lbl_1_data_A9C[2] = messageNo;
        fn_1_37DC((&lbl_1_data_A98[0])[1], lbl_1_data_A9C[2], 0);
    }
}

/* Let the player switch and scroll result lists, then close the panel on confirmation. */
s32 fn_1_28B78(void)
{
    s16 inputDelay = 0;

    fn_1_28568();
    mdsingResultPanelOpen();
    fn_1_26DFC();
    fn_1_44E4_for_28B78(MESSNUM(MESS_SYS_GUIDE, 32));
    do
    {
        read_result_input : HuPrcVSleep();
        if (inputDelay > 5)
        {
            HuSprBankSet(lbl_1_bss_1292[6], 4, 0);
            HuSprBankSet(lbl_1_bss_1292[6], 5, 0);
        }
        inputDelay += 1;
        if (inputDelay < 10)
        {
            goto read_result_input;
        }
        inputDelay = 20;
        if ((HuPadBtnDown[0] & PAD_TRIGGER_L))
        {
            HuAudFXPlay(MSM_SE_MENU_40);
            lbl_1_bss_48--;
            if (lbl_1_bss_48 < 0)
            {
                lbl_1_bss_48 = 1;
            }
            fn_1_26DFC();
            HuSprBankSet(lbl_1_bss_1292[6], 4, 1);
            inputDelay = 0;
            goto read_result_input;
        }
        if ((HuPadBtnDown[0] & PAD_TRIGGER_R) != 0)
        {
            HuAudFXPlay(MSM_SE_MENU_40);
            lbl_1_bss_48 += 1;
            if (lbl_1_bss_48 > 1)
            {
                lbl_1_bss_48 = 0;
            }
            fn_1_26DFC();
            HuSprBankSet(lbl_1_bss_1292[6], 5, 1);
            inputDelay = 0;
            goto read_result_input;
        }
        if ((HuPadDStkRep[0] & PAD_BUTTON_DOWN) != 0)
        {
            if (lbl_1_bss_48 == 0)
            {
                if (lbl_1_bss_4A < lbl_1_bss_47C[0] - 8)
                {
                    lbl_1_bss_4A++;
                    lbl_1_bss_C94[3] = 2.0f;
                    if (lbl_1_data_AC8[3] == 1)
                    {
                        HuAudFXPlay(MSM_SE_CMN_01);
                    }
                    fn_1_26DFC();
                    inputDelay = 5;
                }
            }
            else
            {
                if (lbl_1_bss_4C < lbl_1_bss_47C[1] - 8)
                {
                    lbl_1_bss_4C += 1;
                    lbl_1_bss_C94[3] = 2.0f;
                    if (lbl_1_data_AC8[3] == 1)
                    {
                        HuAudFXPlay(MSM_SE_CMN_01);
                    }
                    fn_1_26DFC();
                    inputDelay = 5;
                }
            }
            goto read_result_input;
        }
        if ((HuPadDStkRep[0] & PAD_BUTTON_UP) != 0)
        {
            if (lbl_1_bss_48 == 0)
            {
                if (lbl_1_bss_4A > 0)
                {
                    lbl_1_bss_4A--;
                    lbl_1_bss_C94[2] = 2.0f;
                    if (lbl_1_data_AC8[2] == 1)
                    {
                        HuAudFXPlay(MSM_SE_CMN_01);
                    }
                    fn_1_26DFC();
                    inputDelay = 5;
                }
            }
            else
            {
                if (lbl_1_bss_4C > 0)
                {
                    lbl_1_bss_4C -= 1;
                    lbl_1_bss_C94[2] = 2.0f;
                    if (1 == lbl_1_data_AC8[2])
                    {
                        HuAudFXPlay(MSM_SE_CMN_01);
                    }
                    fn_1_26DFC();
                    inputDelay = 5;
                }
            }
            goto read_result_input;
        }
    }
    while ((PAD_BUTTON_A & HuPadBtnDown[0]) == 0);
    HuAudFXPlay(MSM_SE_CMN_03);
    HuSprAttrSet(lbl_1_bss_1292[2], 2, HUSPR_ATTR_DISPOFF);
    lbl_1_data_AC8[2] = 0;
    HuSprAttrSet(lbl_1_bss_1292[2], 3, HUSPR_ATTR_DISPOFF);
    lbl_1_data_AC8[3] = 0;
    fn_1_4690();
    fn_1_25FD4();
    Hu3DLayerHookReset(10);
    fn_1_778C();
    fn_1_282C0();
    return 1;
}

/* Queue the falling reward models before the earned-star presentation begins. */
static inline void fn_1_24B1C_for_2987C(s16 count)
{
    OMOBJ *obj = lbl_1_bss_3DC;
    s16 modelNo;

    lbl_1_bss_3D8 = count;
    if (lbl_1_bss_3D8 > 0) {
        if (lbl_1_bss_3D8 >= 500) {
            lbl_1_bss_3D8 = 500;
        }
        for (modelNo = 0; modelNo < 64; modelNo++) {
            lbl_1_bss_358[modelNo] = 0;
        }
        obj->objFunc = fn_1_246F8;
    }
}

/* Start fading the earned-star model and its number out after the guide's message. */
static inline void fn_1_24FA8_for_2987C(void)
{
    OMOBJ *obj = lbl_1_bss_50;

    obj->work[1] = 0;
    obj->objFunc = fn_1_24D44;
}

/* Present falling rewards and the earned stars, with different reactions for a zero award. */
s32 fn_1_2987C(void)
{
    char scoreText[16];

    fn_1_24B1C_for_2987C(lbl_1_bss_3E);
    HuPrcSleep(30);
    do {
        HuPrcVSleep();
    } while (lbl_1_bss_3DC->objFunc != NULL);
    HuPrcSleep(30);
    if (lbl_1_bss_40 == 0) {
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[lbl_1_bss_22],
            lbl_1_bss_10->mtnId[38], 0.0f, 10.0f, 0);
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[6],
            0.0f, 10.0f, HU3D_MOTATTR_LOOP);
        Hu3DMotionShiftStartEndSet(lbl_1_bss_C->mdlId[0],
            20.0f, 100.0f);
        fn_1_3F0C(3, MESSNUM(MESS_SINGLE_RESULT, 2), 1);
        HuPrcSleep(30);
        CharFXPlay(lbl_1_bss_22, CHARVOICEID(12));
        HuPrcSleep(150);
        fn_1_3DB4();
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[0],
            0.0f, 10.0f, HU3D_MOTATTR_LOOP);
        HuAudSStreamFadeOut(lbl_1_data_320[1], 1000);
    } else {
        HuAudSStreamFadeOut(lbl_1_data_320[1], 1000);
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[lbl_1_bss_22],
            lbl_1_bss_10->mtnId[37], 0.0f, 10.0f, 0);
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[4],
            0.0f, 10.0f, 0);
        HuPrcSleep(30);
        HuAudJinglePlay(MSM_STREAM_CARDOP);
        CharFXPlay(lbl_1_bss_22, CHARVOICEID(0));
        fn_1_251D8();
        sprintf(scoreText, lbl_1_data_CCE, lbl_1_bss_40);
        fn_1_4144(3, (s32)scoreText, 0);
        fn_1_3F0C(3, MESSNUM(MESS_SINGLE_RESULT, 1), 1);
        HuPrcSleep(240);
        fn_1_3DB4();
        fn_1_24FA8_for_2987C();
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[0],
            0.0f, 10.0f, HU3D_MOTATTR_LOOP);
        HuPrcSleep(20);
    }
    return 1;
}

/* Release the results display, add earned stars to the bank, and attempt a save when
 * saving is enabled and common save data has changed. */
void fn_1_2A4F4(void)
{
    fn_1_25BF4();
    GWBankStarAdd((u16) lbl_1_bss_40);
    if (SLSaveFlagGet() != 0 && SLSaveCheck() != 0) {
        fn_1_3CF8();
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
        HuSprPriSet(lbl_1_bss_1292[8], 0, 5500);
        fn_1_4874(lbl_1_bss_1292[8], HUSPR_ATTR_DISPOFF);
        HuPrcSleep(5);
        SLSaveModeExec(0);
        fn_1_47F4(lbl_1_bss_1292[8], HUSPR_ATTR_DISPOFF);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
        while (WipeCheck() != 0) {
            HuPrcVSleep();
        }
    }
}

/* Close a dialogue window during the post-board choice sequence. */
static inline void mdsingVariantClose(s16 winNo) { fn_1_365C(winNo); }

/* Open a dialogue window during the post-board choice sequence. */
static inline void mdsingVariantOpen(s16 winNo) { fn_1_35EC(winNo); }

/* Switch the active post-board dialogue window and clear its message cache. */
static inline void mdsingVariantSelect(s16 winNo)
{
    if (lbl_1_data_A98[0] != -1 && lbl_1_data_A98[0] != winNo)
        mdsingVariantClose(lbl_1_data_A98[0]);
    if (lbl_1_data_A98[0] == -1 || lbl_1_data_A98[0] != winNo) {
        lbl_1_data_A98[0] = winNo;
        lbl_1_data_A9C[0] = -1;
        lbl_1_data_A9C[1] = -1;
        mdsingVariantOpen(lbl_1_data_A98[0]);
    }
}

/* Set the centered post-board dialogue text and its reveal speed. */
static inline void mdsingVariantSet(s16 winNo, s32 messageNo, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_1398[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_1398[winNo], messageNo);
    HuWinMesSpeedSet(lbl_1_bss_1398[winNo], speed);
    if (lbl_1_data_8EC != messageNo)
        lbl_1_data_8EC = -1;
}

/* Refresh post-board dialogue only when its message changes. */
static inline void mdsingVariantMessage(s16 winNo, s32 messageNo, s16 speed)
{
    mdsingVariantSelect(winNo);
    if (lbl_1_data_A9C[0] != messageNo) {
        lbl_1_data_A9C[0] = messageNo;
        lbl_1_data_A9C[1] = -1;
        mdsingVariantSet(lbl_1_data_A98[0], lbl_1_data_A9C[0], speed);
    }
}

/* Read a post-board dialogue choice with the requested cancellation behavior. */
static inline s16 mdsingVariantChoice(s16 winNo, s16 mode)
{
    s16 choice = 0;
    if (mode == 1)
        HuWinAttrSet(lbl_1_bss_1398[winNo], HUWIN_ATTR_NOCANCEL);
    else
        HuWinAttrReset(lbl_1_bss_1398[winNo], HUWIN_ATTR_NOCANCEL);
    choice = HuWinChoiceGet(lbl_1_bss_1398[winNo], -1);
    if (mode == 2 && choice == -1)
        choice = 1;
    return choice;
}

/* Read the active post-board dialogue choice, or zero when no window is open. */
static inline s16 mdsingVariantGetChoice(s16 mode)
{
    if (lbl_1_data_A98[0] != -1)
        return mdsingVariantChoice(lbl_1_data_A98[0], mode);
    return 0;
}

/* Wait for the requested post-board dialogue window's message to finish. */
static inline void mdsingVariantWait(s16 winNo) { fn_1_36CC(winNo); }

/* Wait for the active post-board dialogue message to finish. */
static inline void mdsingVariantWaitMessage(void)
{
    if (lbl_1_data_A98[0] != -1)
        mdsingVariantWait(lbl_1_data_A98[0]);
}

/* Ask the post-board continuation question and play the guide's response when leaving. */
s32 fn_1_2A81C(void)
{
    mdsingVariantMessage(3, MESSNUM(MESS_SINGLE_RESULT, 3), 1);
    if (mdsingVariantGetChoice(1) == 0) {
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[5], 0.0f, 20.0f,
                           HU3D_MOTATTR_LOOP);
        mdsingVariantMessage(3, MESSNUM(MESS_SINGLE_RESULT, 4), 1);
        mdsingVariantWaitMessage();
        return -1;
    }
    return 0;
}

/* Present the unsuccessful board return, send the character away, and ask whether to continue. */
s32 fn_1_2AD60(void)
{
    s16 elapsedFrames;
    MDSING_RESULT_POSITIONS positions = {
        { 0.0f, 0.0f, 0.0f }, { 350.0f, 0.0f, 1000.0f }
    };

    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[6],
                       0.0f, 10.0f, HU3D_MOTATTR_LOOP);
    Hu3DMotionShiftStartEndSet(lbl_1_bss_C->mdlId[0],
        20.0f, 100.0f);
    fn_1_3F0C(3, MESSNUM(MESS_SINGLE_RESULT, 5), 1);
    fn_1_3DB4();
    Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[lbl_1_bss_22],
        lbl_1_bss_10->mtnId[38], 0.0f, 10.0f, 0);
    HuPrcSleep(30);
    CharFXPlay(lbl_1_bss_22, CHARVOICEID(12));
    HuPrcSleep(150);
    fn_1_778C();
    HuAudSStreamFadeOut(lbl_1_data_320[2], 1000);
    lbl_1_bss_40 = 0;
    fn_1_2A4F4();

    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[0],
                       0.0f, 30.0f, HU3D_MOTATTR_LOOP);
    lbl_1_bss_10->objFunc = fn_1_6E94;
    Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[lbl_1_bss_22],
                       lbl_1_bss_10->mtnId[36], 0.0f,
                       10.0f, HU3D_MOTATTR_LOOP);
    for (elapsedFrames = 0; elapsedFrames < 120; elapsedFrames++) {
        HuPrcVSleep();
        fn_1_1BDC(lbl_1_bss_10->mdlId[lbl_1_bss_22],
            &positions.start, &positions.end, elapsedFrames, 90.0f, 0.0f);
    }
    lbl_1_bss_10->objFunc = NULL;
    lbl_1_data_320[0] = HuAudSStreamPlay(MSM_STREAM_W01);
    fn_1_51F4();
    fn_1_3F0C(3, MESSNUM(MESS_SINGLE_RESULT, 3), 1);
    if (fn_1_3E14(1) == 0) {
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[5],
                          0.0f, 20.0f, HU3D_MOTATTR_LOOP);
        fn_1_3F0C(3, MESSNUM(MESS_SINGLE_RESULT, 4), 1);
        fn_1_3DB4();
        return -1;
    }
    return 0;
}

/* Run the appropriate board-return results sequence and return the player's next action. */
s32 fn_1_2C158(void)
{
    s16 elapsedFrames;
    s16 windowNo;
    s16 groupNo;
    s16 memberNo;
    HUSPR_GROUP *group;
    s32 result = 0;
    Vec positions[2] = {
        { 0.0f, 0.0f, 0.0f }, { 350.0f, 0.0f, 1000.0f }
    };
    if (mdsingCoinBonusGet(0) == -1) {
        result = fn_1_2AD60();
    } else {
        GWSinglePrizeSaveFlagSet();
        fn_1_28828();
        fn_1_28B78();
        fn_1_2987C();
        fn_1_2A4F4();

        lbl_1_bss_10->objFunc = fn_1_6E94;
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[lbl_1_bss_22],
                           lbl_1_bss_10->mtnId[36],
                           0.0f, 10.0f, HU3D_MOTATTR_LOOP);
        for (elapsedFrames = 0; elapsedFrames < 120; elapsedFrames++) {
            HuPrcVSleep();
            fn_1_1BDC(lbl_1_bss_10->mdlId[lbl_1_bss_22], &positions[0], &positions[1],
                elapsedFrames, 90.0f, 0.0f);
        }
        lbl_1_bss_10->objFunc = NULL;
        lbl_1_data_320[0] = HuAudSStreamPlay(MSM_STREAM_W01);
        result = fn_1_2A81C();
    }
    return result;
}
