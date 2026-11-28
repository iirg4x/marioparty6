/* Declares shared operations for the minigame. */
#ifndef REL_M675DLL_MODULE_TYPES_H
#define REL_M675DLL_MODULE_TYPES_H

#include "game/object.h"
#include "REL/minigame_ground_effect.h"
#include "REL/minigame_positional_audio.h"
#include "REL/m675Dll/actor_state.h"

/* Camera, motion interpolation and resource operations. */
void fn_1_BBC(OMOBJMAN *manager);
void fn_1_C84(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color);
void fn_1_136C(OMOBJMAN *manager, u32 cameras, float fov, float nearPlane, float farPlane,
    float shadowRadius, OM_CAMERA_VIEW *view);
void fn_1_1DD8(s32 camera, Vec *direction, Vec *target, float fov, float radius);
void fn_1_2414(u32 cameras, void *data);
void fn_1_24CC(u32 cameraMask);
void fn_1_2F38(OMOBJMAN *objectManager, MinigameAudioRange *audioConfig);
void fn_1_3C70(s32 effect, Vec *position);
void fn_1_4424(int effect, int handle, Vec *position, int panEnabled, int volumeEnabled);
int fn_1_45C4(int dataNum);
s32 fn_1_A38(s32 resourceId);
s32 fn_1_A98(u32 playerToken);
void fn_1_1FC8(void (*callback)(OMOBJ *, u32, s32));
void fn_1_4BB4(Vec *start, Vec *end, f32 factor, Vec *position);
s32 fn_1_A60(void);
s32 fn_1_32EC(Vec *position);
s32 fn_1_46CC(s32 model, s32 dataNum);
float fn_1_47DC(float start, float end, float factor);
float fn_1_48F8(float start, float end);
float fn_1_49AC(HuVecF *currentPosition, HuVecF *nextPosition);
float fn_1_4A48(Vec *first, Vec *second);
s32 fn_1_4C0C(Vec *start, Vec *end, f32 gravity, f32 lengthSeedY, Vec *velocity);
void fn_1_E48(s16 modelId, char *lightName, s32 unusedMode);

/* Round state and player selection. */
s32 fn_1_A40(void);
s32 fn_1_A50(void);
void fn_1_B14(s32 playerIndex, s32 playerToken, s32 characterId);
void fn_1_B4C(unsigned int playerIndex);
int fn_1_B6C(void);
s32 fn_1_D3BC(void);
s32 fn_1_D3CC(void);
void fn_1_D494(s32 type, s32 value);

/* Actor initialization and motion control. */
void fn_1_4F2C(HUPROCESS *manager);
void fn_1_61F4(M675ActorState *state);
void fn_1_6210(M675ActorState *state, s32 motionIndex, f32 speed);
void fn_1_74C8(M675ActorState *state);
void fn_1_74FC(M675ActorState *state, s32 modelId);
void fn_1_7554(HUPROCESS *process);
void fn_1_F118(HUPROCESS *process);

/* Effects and scoring. */
Vec *fn_1_D550(s32 type, s32 index);
void fn_1_D638(HUPROCESS *process);
void fn_1_E0F0(Vec *origin);
void fn_1_E4F8(const Vec *position, f32 radius, s32 count);
void fn_1_E918(MinigameGroundEffect *effect);
void fn_1_E9A8(HUPROCESS *process);
void fn_1_EF88(s32 playerIndex, s32 amount);
void fn_1_F004(void);
s32 fn_1_F058(s32 playerIndex);
void fn_1_F0B0(s32 playerIndex, f32 *xPosition, f32 *yPosition);
s32 fn_1_10170(void *registration);

#endif
