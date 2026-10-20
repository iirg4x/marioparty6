/* Builds the minigame scene and advances its shared models, effects and sequence hooks. */
#include "humath.h"
#include "REL/m619Dll/types.h"
#include "game/flag.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "msm_se.h"

#define MSM_SE_M619_6DF 1759
#define MSM_SE_M619_6E0 1760
#define MSM_SE_M619_6E1 1761
#define MSM_SE_M619_6E2 1762
#define MSM_SE_M619_6E3 1763
#define MSM_SE_M619_6E4 1764
#define MSM_SE_M619_6E5 1765
#define MSM_SE_M619_6E6 1766
#define MSM_SE_M619_6E7 1767
#define MSM_SE_M619_6E8 1768

int _prolog(void);
void _epilog(void);
void fn_1_A0(void);
void fn_1_1E8(s16 mode, s16 frameNo);
void fn_1_250(s16 mode, s16 frameNo);
void fn_1_B60(s16 mode, s16 frameNo);
void fn_1_BD0(s16 mode, s16 frameNo);
void fn_1_D0C(s16 mode, s16 frameNo);
void fn_1_E84(s16 mode, s16 frameNo);
void fn_1_11A8(s16 mode, s16 frameNo);
void fn_1_12B4(s16 mode, s16 frameNo);
void fn_1_12B8(s16 mode, s16 frameNo);
void fn_1_12D8(M619Scene *state);
void fn_1_1450(M619Scene *state);
void fn_1_15CC(M619Scene *scene);
void fn_1_15D0(OMOBJ *object);
void fn_1_1624(OMOBJ *object);
void fn_1_1688(M619Scene *module);
void fn_1_182C(M619Scene *module);
void fn_1_1F60(M619SceneAnimation *animation, M619Scene *initialization);
void fn_1_2724(void *input);
s32 fn_1_39E8(void *scene);
void fn_1_3A5C(M619MotionNode *actor, M619MotionNode *command,
               f32 step);
void fn_1_3BFC(M619RadialActor *setup, s32 callbackState);
void fn_1_3C70(M619Scene *state);
s32 fn_1_3CBC(M619PlayerPose *burst);
void fn_1_4278(M619SceneAnimation *state);
void fn_1_449C(M619SceneAnimation *state, s32 callbackValue);
void fn_1_47A8(M619Scene *state);
void fn_1_4900(M619CameraOrbit *orbit);
s32 fn_1_4B44(M619Scene *scene);
void fn_1_51EC(M619ModelQueueNode *node, const HuVecF *position);
void fn_1_52E0(M619ModelQueueNode *node);
void fn_1_535C(M619Scene *owner);
void fn_1_55BC(M619ModelQueueNode *group, f32 scale, s32 variant);
void fn_1_5774(M619ModelQueueNode *queueNode);
void fn_1_5780(M619ModelQueueNode *queueNode);
void fn_1_5794(HuVecF *point);
void fn_1_5A80(M619SelectionRecord *node, s32 mode);
void fn_1_6140(M619Selection *selection);
void fn_1_61DC(M619Selection *selection);
void fn_1_6290(M619Selection *selection);
void fn_1_6344(M619Selection *selection);
void fn_1_69E8(M619Selection *motion);
void fn_1_6A8C(void *selectionRecord);
s32 fn_1_7058(void *pose);
void fn_1_70E0(M619MotionNode *target,
               M619MotionNode *source, f32 speed);
void fn_1_74E0(M619SelectionRecord *pose);
s32 fn_1_7640(M619SelectionRecord *selectionRecord);
void fn_1_7694(M619Scene *collection);
s32 fn_1_7748(M619SelectionRecord *selectionRecord);
void fn_1_77F4(M619Scene *scene);
void fn_1_792C(M619PlayerMotionPayload *recipient, M619Player *source);
void fn_1_7A20(void *motionPayload);
s32 fn_1_7D64(void *self);
void fn_1_7D84(M619MotionNode *target, M619MotionNode *request,
               f32 step);
s32 fn_1_7F44(void *scene);
s32 fn_1_84D8(void *scene);
s32 fn_1_88F0(void *animationState);
s32 fn_1_8A3C(void *animationState);
void fn_1_8BC4(M619Launcher *work, M619Scene *assets);
void fn_1_9320(M619Launcher *motion, f32 inputPhase);
void fn_1_9454(M619Launcher *launcher);
void fn_1_952C(M619Launcher *motion);
void fn_1_962C(M619Launcher *launcher, f32 progress);
void fn_1_9680(M619Launcher *result, f32 speed);
void fn_1_9714(M619Scene *scene);
s32 fn_1_98F0(void *scene);
s32 fn_1_9C44(void *transition);
void fn_1_9EEC(M619ModelMotion *work, s32 characterIndex);
void fn_1_A124(M619ModelMotion *object, const HuVecF *position);

extern f64 sin(f64 angle);

extern f64 atan2(f64 y, f64 x);
extern f64 cos(f64 angle);

extern void fn_1_C194(M619Player *player);

typedef struct M619SpeedFactorTable_s {
    /* NPC motion speeds for each difficulty level. */
    f32 factors[4];
} M619SpeedFactorTable;

extern void fn_1_C890(M619ParticleEffect *effect);

typedef struct M619MotionPathFactorTable_s {
    /* Per-state path scaling values. */
    f32 factors[4];
} M619MotionPathFactorTable;

extern void fn_1_D9EC(M619ParticleEffect *effect, const HuVecF *position, const HuVecF *rotation);

extern void fn_1_C8CC(M619ParticleEffect *effect);
extern void fn_1_D420(M619ParticleEffect *effect, const HuVecF *position, const HuVecF *rotation);

extern void fn_1_CE20(M619ParticleEffect *effect, const HuVecF *position, const HuVecF *rotation);

extern s32 fn_1_A1F4(void *object);
extern void fn_1_C8D8(M619ParticleEffect *effect, HuVecF *position);
extern s32 fn_1_C324(void *state);

MGSEQ_PARAM lbl_1_data_0 = {
    30, MGSEQ_TIMER_RIGHT, fn_1_1E8, fn_1_250, fn_1_B60, fn_1_BD0, fn_1_D0C, fn_1_E84,
    fn_1_11A8, fn_1_12B4, fn_1_12B8
};
s32 lbl_1_data_28[33] = {
    DATANUM(DATA_m619, 0), DATANUM(DATA_m619, 1), DATANUM(DATA_m619, 2), DATANUM(DATA_m619, 3),
    DATANUM(DATA_m619, 5), DATANUM(DATA_m619, 4), DATANUM(DATA_m619, 6), DATANUM(DATA_m619, 7),
    DATANUM(DATA_m619, 8), DATANUM(DATA_m619, 9), DATANUM(DATA_m619, 10), DATANUM(DATA_m619, 11),
    DATANUM(DATA_m619, 12), DATANUM(DATA_m619, 13), DATANUM(DATA_m619, 14), DATANUM(DATA_m619, 15),
    DATANUM(DATA_m619, 16), DATANUM(DATA_m619, 17), DATANUM(DATA_m619, 18), DATANUM(DATA_m619, 19),
    DATANUM(DATA_m619, 20), DATANUM(DATA_m619, 21), DATANUM(DATA_m619, 22), DATANUM(DATA_m619, 23),
    DATANUM(DATA_m619, 24), DATANUM(DATA_m619, 25), DATANUM(DATA_m619, 26), DATANUM(DATA_m619, 27),
    DATANUM(DATA_m619, 28), DATANUM(DATA_m619, 29), DATANUM(DATA_m619, 30), DATANUM(DATA_m619, 31),
    DATANUM(DATA_m619, 32)
};
s32 lbl_1_data_AC[1] = {DATANUM(DATA_m619, 33)};

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

void fn_1_A23C(OMOBJ *obj);                         /* static */

typedef struct {
    HuVecF start;  /* Initial camera or target position. */
    HuVecF finish; /* Final camera or target position. */
} M619DisplayConfig;
const u16 lbl_1_rodata_10[4] = {1, 2, 4, 8};

HUPROCESS *lbl_1_bss_8[2];
s32 lbl_1_bss_4;
OMOBJ *lbl_1_bss_0;

extern s32 fn_1_C3B4(void *player);

extern s32 fn_1_C420(void *player);

typedef struct Position {
    HuVecF position;
    unsigned char reservedBytes[12]; /* Unread bytes between the position and offset vectors. */
    HuVecF offset;
} Position;

s32 fn_1_367CC(s32 *player);

extern s32 fn_1_C1E8(M619Player *player, f32 yOffset);

typedef struct M619AngleTableView_s {
    /* Starting angles in degrees for the four player positions. */
    f32 angles[4];
} M619AngleTableView;

extern void fn_1_C628(M619Player *player);
extern void fn_1_B3FC(void *motion, s32 expand);
extern void fn_1_B678(void *player);
extern s32 fn_1_BADC(void *player);
extern void fn_1_BB4C(M619MotionNode *object, M619MotionNode *event, f32 impactStrength);

extern void fn_1_B3C0(M619Sway *sway,
                      M619Player *owner);
extern void fn_1_C690(void *work);
extern void fn_1_C69C(M619ParticleEffect *effect, const HuVecF *position, const HuVecF *rotation);
extern void fn_1_A614(OMOBJ *object);

extern void fn_1_C890(M619ParticleEffect *flash);

/* Radial motion updates use this helper to convert an X/Z direction to an angle. */
static inline f32 M619InitializationDirection(f32 x, f32 z) {
    if (0.0f == z) {
        if (x >= 0.0f) {
            return 1.5707964f;
        } else {
            return -1.5707964f;
        }
    } else {
        return atan2f(x, z);
    }
}

/* Scene setup helper: creates the first model for a slot and links later copies. */
static inline HU3D_MODELID M619MotionAdjustedModelLinkOrCreate(
    M619Scene *state, s32 slot) {
    HU3D_MODELID model;

    if (state->models[slot] < 0) {
        model = state->models[slot] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_28[slot], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(state->models[slot]);
    }
    return model;
}

/* The callback deliberately evaluates its argument without using the value. */
static inline void M619DisplayConfigDebugValue(void *playerState) { }

/* The module loader runs constructors before creating the minigame scene. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

/* The module loader runs destructors when the minigame module is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Called by the module entry point to create the scene and player processes. */
void fn_1_A0(void) {
    OMOBJ *object;
    s32 playerIndex;
    void *sceneState;
    lbl_1_bss_8[0] = omInitObjMan(10, 4096);
    omGameSysInit(lbl_1_bss_8[0]);
    object = omAddObjEx(lbl_1_bss_8[0], 101, 0U, 0U, - 1, fn_1_15D0);
    lbl_1_bss_0 = object;
    object->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M619Scene), HU_MEMNUM_OVL);
    sceneState = object->data;
    playerIndex = 0;
    while (playerIndex < 4)
    {
        object = omAddObjEx(lbl_1_bss_8[0], 100, 1U, 8U, 0, fn_1_A23C);
        object->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M619Player), HU_MEMNUM_OVL);
        object->work[0] = (u32) playerIndex;
        playerIndex += 1;
    }
    Hu3DBGColorSet(0U, 0U, 0U);
    Hu3DNoSyncSet(0);
    fn_1_1688(sceneState);
    fn_1_12D8(sceneState);
    fn_1_1450(sceneState);
    fn_1_15CC(sceneState);
    MgSeqCreate(& lbl_1_data_0);
}

/* Runs during the opening sequence phase to load scene assets and start the launcher. */
void fn_1_1E8(s16 mode, s16 frameNo) {
    M619Scene *state;

    state = (M619Scene *) lbl_1_bss_0->data;
    CharEffectLayerSet(6);
    fn_1_182C(state);
    fn_1_1F60(&state->sceneAnimation, state);
    fn_1_47A8(state);
    fn_1_8BC4(&state->launcher, state);
    MgSeqModeNext();
}

/* Sequence display callback: advances the launcher, camera, lighting, and transition. */
void fn_1_250(s16 mode, s16 frameNo) {
    M619DisplayConfig blendStart = {{0.0f, 730.0f, 2500.0f}, {0.0f, 1432.89f, 1666.75f}};
    M619DisplayConfig blendEnd = {{0.0f, -30.0f, -425.0f}, {0.0f, -180.0f, -40.0f}};
    HuVecF transitionPosition = {0.0f, 0.0f, 0.0f};
    Point3d lightDirection = {0.0f, 450.0f, 1000.0f};
    Point3d inwardLightDirection;
    HuVecF transitionPositionCopy;
    GXColor lightColor = {255, 255, 255, 255};
    f32 progress;
    s16 sequenceState;
    M619Player *playerState;
    HuVecF *transitionPositionPointer;
    M619Scene *module;
    s32 index;
    s32 materialCount;
    HSF_MATERIAL *material;
    HSF_DATA *modelData;

    module = lbl_1_bss_0->data;
    switch (module->displayState) {
        case 0:
            MgSeqModeChangeOff();
            if (module->nightLighting != 0) {
                inwardLightDirection.x = -lightDirection.x;
                inwardLightDirection.y = -lightDirection.y;
                inwardLightDirection.z = -lightDirection.z;
                PSVECNormalize(&inwardLightDirection, &inwardLightDirection);
                CharLightCreateV(&lightDirection, &inwardLightDirection, &lightColor);
                CharLightInfinitytSet();
            }
            module->adjustmentEnabled = 1;
            if (module->nightLighting != 0) {
                modelData = Hu3DData[module->models[27]].hsf;
            } else {
                modelData = Hu3DData[module->models[0]].hsf;
            }
            materialCount = modelData->materialNum;
            material = modelData->material;
            for (index = 0; index < materialCount; index++, material++) {
                module->materialTransparency[0][index] = material->invAlpha;
            }
            if (module->nightLighting != 0) {
                modelData = Hu3DData[module->models[28]].hsf;
            } else {
                modelData = Hu3DData[module->models[1]].hsf;
            }
            materialCount = modelData->materialNum;
            material = modelData->material;
            for (index = 0; index < materialCount; index++, material++) {
                module->materialTransparency[1][index] = material->invAlpha;
            }
            if (module->nightLighting != 0) {
                modelData = Hu3DData[module->models[31]].hsf;
            } else {
                modelData = Hu3DData[module->models[19]].hsf;
            }
            materialCount = modelData->materialNum;
            material = modelData->material;
            for (index = 0; index < materialCount; index++, material++) {
                module->materialTransparency[2][index] = material->invAlpha;
            }
            if (module->nightLighting != 0) {
                modelData = Hu3DData[module->models[32]].hsf;
            } else {
                modelData = Hu3DData[module->models[20]].hsf;
            }
            materialCount = modelData->materialNum;
            material = modelData->material;
            for (index = 0; index < materialCount; index++, material++) {
                module->materialTransparency[3][index] = material->invAlpha;
            }
            for (index = 0; index < 4; index++) {
                playerState = module->players[index];
                CharMotionSet(playerState->character, playerState->motions[7]);
            }
            if (fn_1_E5B8(fn_1_84D8, module) != 0U) {
                module->pulseState = 0;
            }
            module->frame = 0;
            module->displayState += 1;
            break;
        case 1:
            module->frame = (s32) (module->frame + 1);
            progress = (f32) module->frame / 60.0f;
            fn_1_9320(&module->launcher, progress);
            fn_1_9454(&module->launcher);
            if ((s32) module->frame >= 30) {
    fn_1_962C(&module->launcher, (f32)(module->frame - 30) / 30.0f);
            }
            if ((s32) module->frame >= 60) {
                for (index = 0; index < 4; index++) {
                    playerState = module->players[index];
                    M619DisplayConfigDebugValue(playerState);
                }
                module->displayState += 1;
            }
            break;
        case 2:
            fn_1_9454(&module->launcher);
    fn_1_962C(&module->launcher, 1.0f);
            if ((s32) module->finishFlag != 0) {
                module->sceneAnimation.frame = 0;
                module->frame = 0;
                module->displayState += 1;
            }
            break;
        case 3:
            module->frame = (s32) (module->frame + 1);
    fn_1_962C(&module->launcher, 1.0f);
            if ((s32) module->frame == 25) {
                HuAudFXPlay(MSM_SE_M619_6E1);
                fn_1_E5B8(fn_1_88F0, &module->sceneAnimation);
                Hu3DModelAttrReset(module->sceneAnimation.model, HU3D_ATTR_DISPOFF);
            }
            if ((s32) module->frame >= 30) {
                HuAudFXPlay(MSM_SE_M619_6E2);
                module->frame = 0;
                module->displayState += 1;
            }
            break;
        case 4:
            for (index = 0; index < 4; index++) {
                playerState = module->players[index];
                CharMotionShiftSet(playerState->character, playerState->motions[6], 0.0f, 15.0f,
                                   HU3D_MOTATTR_LOOP);
                CharFXPlay(playerState->character, MSM_SE_CHARVOICE_MARIO + 8);
            }
            transitionPositionCopy = transitionPosition;
            transitionPositionPointer = &transitionPositionCopy;
            fn_1_A124(&module->sceneAnimation.modelMotion,
                      transitionPositionPointer);
            Hu3DModelAttrSet(module->launcher.rotationModel, HU3D_ATTR_DISPOFF);
            module->displayState += 1;
            /* fallthrough */
        case 5:
            {
            module->frame = (s32) (module->frame + 1);
            progress = (f32) module->frame / 60.0f;
    fn_1_962C(&module->launcher, 1.0f - progress);
            fn_1_9680(&module->launcher, progress);
            module->camera.position.y =
                (f32) ((blendStart.start.y * (1.0f - progress)) + (blendStart.finish.y * progress));
            module->camera.position.z =
                (f32) ((blendStart.start.z * (1.0f - progress)) + (blendStart.finish.z * progress));
            module->camera.target.y =
                (f32) ((blendEnd.start.y * (1.0f - progress)) + (blendEnd.finish.y * progress));
            module->camera.target.z =
                (f32) ((blendEnd.start.z * (1.0f - progress)) + (blendEnd.finish.z * progress));
            module->camera.elevation = (f32) (90.0f * (1.0f - progress));
            if (module->nightLighting != 0) {
                modelData = Hu3DData[module->models[32]].hsf;
            } else {
                modelData = Hu3DData[module->models[20]].hsf;
            }
            material = modelData->material;
            materialCount = modelData->materialNum;
            for (index = 0; index < materialCount; index++, material++) {
                material->invAlpha =
                    progress + (module->materialTransparency[3][index] * (1.0f - progress));
            }
            if ((s32) module->frame >= 60) {
                Hu3DModelHookReset(module->launcher.positionModel);
                Hu3DModelAttrSet(module->launcher.positionModel, HU3D_ATTR_DISPOFF);
                if (module->nightLighting != 0) {
                    Hu3DModelAttrSet(module->models[32], HU3D_ATTR_DISPOFF);
                } else {
                    Hu3DModelAttrSet(module->models[20], HU3D_ATTR_DISPOFF);
                }
                fn_1_9714(module);
                fn_1_E5B8(fn_1_9C44, &module->launcher.transitions[0]);
                module->frame = 0;
                module->displayState += 1;
            }
            }
            break;
        case 6:
            module->frame = (s32) (module->frame + 1);
            if ((s32) module->frame == 15) {
                fn_1_E5B8(fn_1_9C44, &module->launcher.transitions[1]);
                module->frame = 0;
                module->displayState += 1;
            }
            break;
        case 7:
            module->frame = (s32) (module->frame + 1);
            if ((s32) module->frame == 15) {
                if (fn_1_E5B8(fn_1_98F0, module) != 0U) {
                    module->actionPhase = 0;
                }
                module->frame = 0;
                module->displayState += 1;
            }
            break;
        case 8:
            module->frame = (s32) (module->frame + 1);
            if ((s32) module->frame >= 60) {
                MgSeqModeNext();
            }
            break;
    }
    fn_1_952C(&module->launcher);
    fn_1_4900(&module->camera);
}

/* Sequence audio callback: starts the victory track once. */
void fn_1_B60(s16 mode, s16 frameNo)
{
    M619Scene *audioView = (M619Scene *)lbl_1_bss_0->data;

    switch (audioView->audioState) {
    case 0:
        lbl_1_bss_4 = HuAudBGMPlay(MSM_STREAM_SD_TOUR_WIN);
        audioView->audioState++;
        break;
    case 1:
    default:
        break;
    }
}

/* Sequence start callback: enables play and advances when at least three players are out. */
void fn_1_BD0(s16 mode, s16 frameNo) {
    M619Scene *state;
    M619Player *player;
    s32 playerNumber;
    s32 activeCount;

    state = (M619Scene *)lbl_1_bss_0->data;
    switch (state->startState) {
    case 0:
        state->selection.status = 1;
        for (playerNumber = 0; playerNumber < 4; playerNumber++) {
            player = state->players[playerNumber];
            player->inputEnabled = 1;
            player->movementActive = 1;
            player->phase = 0;
        }
        state->startState++;
        break;
    case 1:
        activeCount = 0;
        for (playerNumber = 0; playerNumber < 4; playerNumber++) {
            player = state->players[playerNumber];
            if (player->excluded != 0) {
                activeCount++;
            }
        }
        if (activeCount >= 3) {
            for (playerNumber = 0; playerNumber < 4; playerNumber++) {
                player = state->players[playerNumber];
                player->inputEnabled = 0;
                player->movementActive = 0;
            }
            state->adjustmentEnabled = 0;
            MgSeqModeNext();
        }
        state->sceneAnimation.progress += 1.0f;
        break;
    default:
        break;
    }
}

/* Sequence winner callback: records eligible players and starts the result phase. */
void fn_1_D0C(s16 mode, s16 frameNo) {
    M619Scene *state;
    M619Player *player;
    s16 playerOrder[4];
    s32 playerNumber;

    state = (M619Scene *)lbl_1_bss_0->data;
    switch (state->winnerState) {
    case 0:
        state->selection.status = 0;
        state->adjustmentEnabled = 0;
        for (playerNumber = 0; playerNumber < 4; playerNumber++) {
            player = state->players[playerNumber];
            player->inputEnabled = 0;
            player->movementActive = 0;
            player->targetAngle = 0.0f;
            if (player->excluded == 0) {
                state->eligiblePlayers[playerNumber] = 1;
            }
            playerOrder[playerNumber] = -1;
        }
        for (playerNumber = 0; playerNumber < 4; playerNumber++) {
            if (state->eligiblePlayers[playerNumber] != 0) {
                playerOrder[state->winnerCount] =
                    state->players[playerNumber]->character;
                state->winnerCount++;
            }
        }
        if (state->winnerCount > 0) {
            fn_1_3C70(state);
            fn_1_7694(state);
        }
        MgSeqWinnerSet(playerOrder[0], playerOrder[1],
                       playerOrder[2], playerOrder[3]);
        HuAudSStreamFadeOut(lbl_1_bss_4, 100);
        state->winnerState++;
    case 1:
        ;
    }
}

/* Sequence finish callback: presents the winners and advances when their motions finish. */
void fn_1_E84(s16 mode, s16 frameNo) {
    M619Scene *state;
    M619Player *player;
    s32 activeCount;
    s32 playerIndex;
    f32 heightProgress;

    state = (M619Scene *)lbl_1_bss_0->data;
    switch (state->finishState) {
    case 0:
        MgSeqModeChangeOff();
        if (state->winnerCount != 0) {
            fn_1_E5B8(fn_1_8A3C, &state->sceneAnimation);
            if (fn_1_E5B8(fn_1_7F44, state) != 0U) {
                HuAudFXPlay(MSM_SE_M619_6E7);
                state->pulseState = 0;
            }
        }
        state->frame = 0;
        state->finishState++;
        /* Fall through to the first active phase. */
    case 1:
        if (state->winnerCount != 0) {
            if (state->frame++ != 0 && state->frame >= 45) {
                state->finishState++;
                return;
            }
        } else {
            state->frame++;
            if (state->frame >= 120) {
                MgSeqModeNext();
            }
        }
        break;
    case 2:
        activeCount = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            player = state->players[playerIndex];
            if (player->excluded == 0 &&
                fn_1_E5B8(fn_1_C420, player) != 0U) {
                activeCount++;
                player->targetPosition.x = -600.0f +
                    ((1200.0f * (f32)activeCount) /
                     (f32)(state->winnerCount + 1));
                player->targetPosition.y = 0.0f;
                player->targetPosition.z = 350.0f;
            }
        }
        state->finishState++;
        /* Fall through to the completion check. */
    case 3:
        activeCount = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            player = state->players[playerIndex];
            if (player->excluded == 0) {
                activeCount += player->movementFinished;
            }
        }
        if (activeCount == state->winnerCount) {
            state->frame = 0;
            state->finishState++;
            return;
        }
        break;
    case 4:
        state->frame++;
        heightProgress = (f32)state->frame / 60.0f;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            player = state->players[playerIndex];
            if (player->excluded == 0) {
                player->rotation.x = -20.0f * heightProgress;
            }
        }
        if (state->frame >= 60) {
            if (state->finishFlag == 0) {
                MgSeqModeNext();
            }
        }
        break;
    }
}

/* Sequence exit callback: stops player input and awards non-practice bonus coins. */
void fn_1_11A8(s16 mode, s16 frameNo) {
    M619Scene *state;
    M619Player *player;
    s32 playerNumber;
    s32 playerIndex;

    state = (M619Scene *)lbl_1_bss_0->data;
    switch (state->exitState) {
    case 0:
        for (playerNumber = 0; playerNumber < 4; playerNumber++) {
            player = state->players[playerNumber];
            if (player->excluded == 0) {
                player->phase = 4;
                CharMotionShiftSet(player->character, player->motions[4],
                                   0.0f, 15.0f, 0U);
                fn_1_E5B8(fn_1_C3B4, player);
                playerIndex = player->playerNumber;
                if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                    GwPlayer[playerIndex].mgCoinBonus = 10;
                }
            }
        }
        state->exitState++;
    case 1:
        break;
    }
}

void fn_1_12B4(s16 mode, s16 frameNo)
{
}

/* Sequence callback that reads the scene payload but leaves it unchanged. */
void fn_1_12B8(s16 mode, s16 frameNo) {
    void *sceneState;

    sceneState = lbl_1_bss_0->data;
}

/* Scene initialization: creates the camera and sets its opening view. */
void fn_1_12D8(M619Scene *state) {
    Hu3DCameraCreate(lbl_1_rodata_10[0]);
    Hu3DCameraPerspectiveSet(lbl_1_rodata_10[0], 30.0f,
                             20.0f, 10000.0f,
                             1.2f);
    state->camera.position.x = 0.0f;
    state->camera.position.y = 730.0f;
    state->camera.position.z = 2500.0f;
    state->camera.target.x = 0.0f;
    state->camera.target.y = -30.0f;
    state->camera.target.z = -425.0f;
    state->camera.phase = 0.0f;
    state->camera.elevation = 0.0f;
    Hu3DCameraPosSet(lbl_1_rodata_10[0], 0.0f,
                     1432.89f, 1666.75f,
                     0.0f, 1.0f,
                     0.0f, 0.0f,
                     -180.0f, -40.0f);
}

/* Scene initialization: creates the minigame light for the active lighting mode. */
void fn_1_1450(M619Scene *state) {
    HuVecF lightPosition = {0.0f, 2000.0f, 10000.0f};
    HuVecF lightTarget = {0.0f, 0.0f, 0.0f};
    HuVecF lightDirection;
    HU3D_LIGHTID light;
    lightDirection.x = lightTarget.x - lightPosition.x;
    lightDirection.y = lightTarget.y - lightPosition.y;
    lightDirection.z = lightTarget.z - lightPosition.z;
    PSVECNormalize((Vec *)&lightDirection, (Vec *)&lightDirection);

    if (state->nightLighting != 0) {
        light = Hu3DGLightCreate(lightPosition.x, lightPosition.y, lightPosition.z,
                                 lightDirection.x, lightDirection.y, lightDirection.z,
                                 175, 175, 175);
        Hu3DAmbColorSet(0.7f, 0.7f, 0.7f);
        Hu3DShineSet(FALSE);
    } else {
        light = Hu3DGLightCreate(lightPosition.x, lightPosition.y, lightPosition.z,
                                 lightDirection.x, lightDirection.y, lightDirection.z,
                                 152, 152, 152);
        Hu3DAmbColorSet(0.25f, 0.25f, 0.25f);
        Hu3DShineSet(TRUE);
    }
    Hu3DGLightInfinitytSet(light);
}

void fn_1_15CC(M619Scene *scene) { }

/* Object-manager callback: initializes selection and installs the per-frame update. */
void fn_1_15D0(OMOBJ *object) {
    M619Scene *state;

    state = (M619Scene *)lbl_1_bss_0->data;
    fn_1_6140(&state->selection);
    object->objFunc = fn_1_1624;
}

/* Object-manager callback: updates selection, scene animation, collision, and effects. */
void fn_1_1624(OMOBJ *object) {
    M619Scene *state;

    state = (M619Scene *) lbl_1_bss_0->data;
    fn_1_69E8(&state->selection);
    fn_1_4278(&state->sceneAnimation);
    fn_1_4B44(state);
    fn_1_E070();
    fn_1_535C(state);
    fn_1_77F4(state);
    fn_1_E6AC();
}

/* Scene setup: clears model, player, sequence, and animation state before play. */
void fn_1_1688(M619Scene *module) {
    s32 i;
    s16 night;

    for (i = 0; i < 33; i++) {
        module->models[i] = -1;
    }
    for (i = 0; i < 1; i++) {
        module->defaultAnimation[i] = -1;
    }
    for (i = 0; i < 1; i++) {
        module->animations[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_AC[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 4; i++) {
        module->players[i] = 0;
    }
    night = GwMgNightF;
    module->nightLighting = (night == 1);
    module->startState = 0;
    module->displayState = 0;
    module->audioState = 0;
    module->finishState = 0;
    module->winnerState = 0;
    module->exitState = 0;
    for (i = 0; i < 4; i++) {
        module->eligiblePlayers[i] = 0;
    }
    module->winnerCount = 0;
    module->modelQueue = 0;
    module->adjustmentEnabled = 0;
    module->frame = 0;
    module->finishFlag = 0;
    module->pulseState = 0;
    module->pulseFrame = 0;
    fn_1_E514();
    fn_1_E05C();
}

/* Opening-sequence setup: loads or links the models used by the scene. */
void fn_1_182C(M619Scene *module) {
    s16 model;

    if (module->nightLighting != 0) {
        if (module->models[28] < 0) {
            model = module->models[28] = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_28[28], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(module->models[28]);
        }
    } else {
        if (module->models[1] < 0) {
            model = module->models[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_28[1], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(module->models[1]);
        }
    }
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 5);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.2f);

    if (module->nightLighting != 0) {
        if (module->models[27] < 0) {
            model = module->models[27] = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_28[27], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(module->models[27]);
        }
    } else {
        if (module->models[0] < 0) {
            model = module->models[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_28[0], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(module->models[0]);
        }
    }
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 5);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.2f);

    if (module->nightLighting != 0) {
        if (module->models[29] < 0) {
            model = module->models[29] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[29], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(module->models[29]);
        }
    } else {
        if (module->models[2] < 0) {
            model = module->models[2] = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_28[2], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(module->models[2]);
        }
    }
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.5f);

    if (module->nightLighting != 0) {
        if (module->models[30] < 0) {
            model = module->models[30] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[30], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(module->models[30]);
        }
    } else {
        if (module->models[18] < 0) {
            model = module->models[18] = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_28[18], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(module->models[18]);
        }
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);

    if (module->nightLighting != 0) {
        if (module->models[31] < 0) {
            model = module->models[31] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[31], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(module->models[31]);
        }
    } else {
        if (module->models[19] < 0) {
            model = module->models[19] = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_28[19], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(module->models[19]);
        }
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 7);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.04f);

    if (module->nightLighting != 0) {
        if (module->models[32] < 0) {
            model = module->models[32] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[32], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(module->models[32]);
        }
    } else {
        if (module->models[20] < 0) {
            model = module->models[20] = Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_28[20], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(module->models[20]);
        }
    }
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 7);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.04f);
}

/* Called by the opening phase to create the scene animation models and effect records. */
void fn_1_1F60(M619SceneAnimation *animation, M619Scene *initialization) {
    s16 model;
    M619PlayerPose *motionState;
    M619RadialActor *effectState;
    s32 index;

    if (initialization->models[7] < 0) {
        model = initialization->models[7] =
            Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_28[7], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(initialization->models[7]);
    }
    animation->model = model;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, -100.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.5f);
    Hu3DModelScaleSet(model, 1.0f, 0.75f, 1.0f);
    animation->frame = 0;
    {
    for (index = 0; index < 4; index++) {
        motionState = &animation->poses[index];
        if (initialization->models[15] < 0) {
            model = initialization->models[15] =
                Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_28[15], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(initialization->models[15]);
        }
        motionState->firstModel = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(model, 3);
        Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(model, 1.0f, 1.0f, 1.0f);
        Hu3DMotionSpeedSet(model, 2.0f);
        if (initialization->models[17] < 0) {
            model = initialization->models[17] =
                Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_28[17], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(initialization->models[17]);
        }
        motionState->secondModel = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
        Hu3DModelLayerSet(model, 3);
        Hu3DModelPosSet(model, 0.0f, -130.0f, 0.0f);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(model, 0.0f, 0.0f, 0.0f);
        motionState->mode = 0;
        motionState->position.x = 0.0f;
        motionState->position.y = 0.0f;
        motionState->position.z = 0.0f;
        motionState->rotation.x = 0.0f;
        motionState->rotation.y = 0.0f;
        motionState->rotation.z = 0.0f;
        motionState->angle = (f32) (180.0f * (f32) frandmod(2));
        motionState->player = initialization->players[index];
        motionState->mode = 0;
    }
    }
    {
    for (index = 0; index < 4; index++) {
        effectState = &animation->actors[index];
        if (initialization->models[14] < 0) {
            model = initialization->models[14] =
                Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_28[14], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(initialization->models[14]);
        }
        effectState->firstModel = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
        Hu3DModelLayerSet(model, 3);
        Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
        Hu3DMotionSpeedSet(model, 0.5f);
        if (initialization->models[16] < 0) {
            model = initialization->models[16] =
                Hu3DModelCreate(HuDataSelHeapReadNum(lbl_1_data_28[16], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(initialization->models[16]);
        }
        effectState->secondModel = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
        Hu3DModelLayerSet(model, 3);
        Hu3DModelPosSet(model, 0.0f, -130.0f, 0.0f);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(model, 0.0f, 0.0f, 0.0f);
        effectState->player = initialization->players[index];
        effectState->scale = 30.0f;
        effectState->position.x = 0.0f;
        effectState->position.y = 0.0f;
        effectState->position.z = 0.0f;
        effectState->rotation.x = 0.0f;
        effectState->rotation.y = 0.0f;
        effectState->rotation.z = 0.0f;
        effectState->kind = 102;
        effectState->angle = (f32) (90.0f * (f32) index);
        fn_1_55BC(&effectState->modelQueue, 2.0f, 1);
        fn_1_51EC(&effectState->modelQueue, &effectState->position);
        fn_1_9EEC(&effectState->modelMotion, 0);
        effectState->active = 0;
    }
    }
    animation->phase = 0;
    animation->progress = 0.0f;
    animation->height = -400.0f;
    animation->effectFrame = 0;
    animation->scale = 1.0f;
    animation->callbackValueA = frandmod(3);
    animation->callbackValueB = (s32) ((animation->callbackValueA + frandmod(2) + 1) % 3);
    fn_1_9EEC(&animation->modelMotion, 1);
    fn_1_D9EC(&animation->effects[0], 0, 0);
    fn_1_D9EC(&animation->effects[1], 0, 0);
    fn_1_D9EC(&animation->effects[2], 0, 0);
}

/* Radial-actor callback: advances its idle, approach, impact, and fade states. */
void fn_1_2724(void *input) {
    Point3d screenPoint;
    Point3d sourcePosition;
    s16 pan;
    Point3d *sourcePositionPointer;
    M619RadialActor *state;
    f32 scale;
    f32 directionAngle;
    f32 deltaX;
    f32 deltaZ;
    f32 screenRatio;

    state = input;
    switch (state->kind) {
    case 0:
        if (state->active != 0) {
            state->kind = 102;
        } else {
            state->frame = 0;
            state->rotation.y = (f32) (360.0f * ((f32) frandmod(10001) / 10000.0f));
            Hu3DModelAttrReset(state->firstModel, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(state->secondModel, HU3D_ATTR_DISPOFF);
            Hu3DModelScaleSet(state->firstModel, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(state->secondModel, 0.0f, 0.0f, 0.0f);
            state->kind = (s32) (state->kind + 1);
        case 1:
            if (((s16) !state->active) && ((s32) state->mode == 0) &&
                ((s32) state->player->excluded == 0)) {
                deltaX = state->player->position.x - state->position.x;
                deltaZ = state->player->position.z - state->position.z;
                if (((deltaX * deltaX) + (deltaZ * deltaZ)) > 9025.0f) {
                    directionAngle = M619InitializationDirection(
                        state->player->position.x - state->position.x,
                        state->player->position.z - state->position.z);
                    state->position.x =
                        (f32) ((f64) state->position.x + (5.5 * sin((f64) directionAngle)));
                    state->position.z =
                        (f32) ((f64) state->position.z + (5.5 * cos((f64) directionAngle)));
                }
            }
            state->frame = (s32) (state->frame + 1);
            scale = (f32) state->frame / 30.0f;
            Hu3DModelScaleSet(state->firstModel, scale, 0.1f * scale, scale);
            Hu3DModelScaleSet(state->secondModel, scale, scale, scale);
            if ((s32) state->frame >= 30) {
                if (state->active != 0) {
                    state->kind = 10;
                } else {
                    state->frame = 0;
                    state->kind = (s32) (state->kind + 1);
                }
            }
        }
        break;
    case 2:
        if (state->active != 0) {
            state->kind = 10;
        } else {
            if (((s32) state->mode == 0) && ((s32) state->player->excluded == 0)) {
                deltaX = state->player->position.x - state->position.x;
                deltaZ = state->player->position.z - state->position.z;
                if (((deltaX * deltaX) + (deltaZ * deltaZ)) > 9025.0f) {
                    directionAngle = M619InitializationDirection(
                        state->player->position.x - state->position.x,
                        state->player->position.z - state->position.z);
                    state->position.x =
                        (f32) ((f64) state->position.x + (5.5 * sin((f64) directionAngle)));
                    state->position.z =
                        (f32) ((f64) state->position.z + (5.5 * cos((f64) directionAngle)));
                }
            }
            state->frame = (s32) (state->frame + 1);
            if ((s32) state->frame >= 60) {
                Hu3D3Dto2D(&state->position, (s16) lbl_1_rodata_10[0], &screenPoint);
                screenRatio = (f32)screenPoint.x / 576.0f;
                if ((screenRatio >= 0.0f) && (screenRatio <= 1.0f)) {
                    pan = (s16)((s32)(64.0f * screenRatio) + 32);
                    HuAudFXPlayPan(MSM_SE_M619_6E6, pan);
                }
                state->frame = 0;
                state->kind = (s32) (state->kind + 1);
            }
        }
        break;
    case 3:
        state->frame = (s32) (state->frame + 1);
        scale = 0.1f + (0.9f * ((f32) state->frame / 15.0f));
        Hu3DModelScaleSet(state->firstModel, 1.0f, scale, 1.0f);
        if ((s32) state->frame == 8) {
            if (state->active == 0) {
                state->collision.kind = 1;
            }
            fn_1_5774(&state->modelQueue);
            sourcePosition = *&state->position;
            sourcePositionPointer = &sourcePosition;
            fn_1_A124(&state->modelMotion,
                      (const HuVecF *)sourcePositionPointer);
        }
        if ((s32) state->frame >= 15) {
            if (state->active) {
                state->kind = 100;
            } else {
                state->frame = 0;
                state->kind = (s32) (state->kind + 1);
            }
        }
        break;
    case 4:
        if (state->active != 0) {
            state->kind = 100;
        } else {
            state->frame = (s32) (state->frame + 1);
            if ((s32) state->frame >= 120) {
                state->frame = 0;
                if ((s32) state->mode == 0) {
                    state->kind = 200;
                } else if ((s32) state->mode == 1) {
                    state->kind = 300;
                } else {
                    state->kind = 400;
                }
            }
        }
        break;
    case 10:
        state->frame = 0;
        state->kind = (s32) (state->kind + 1);
        /* fallthrough */
    case 11:
        state->frame = (s32) (state->frame + 1);
        scale = 1.0f - ((f32) state->frame / 15.0f);
        Hu3DModelScaleSet(state->firstModel, scale, 0.1f * scale, scale);
        Hu3DModelScaleSet(state->secondModel, scale, scale, scale);
        if ((s32) state->frame >= 15) {
            Hu3DModelAttrSet(state->firstModel, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(state->secondModel, HU3D_ATTR_DISPOFF);
            state->kind = 102;
        }
        break;
    case 100:
        state->frame = 0;
        state->kind = (s32) (state->kind + 1);
        /* fallthrough */
    case 101:
        state->frame = (s32) (state->frame + 1);
        scale = (f32) state->frame / 30.0f;
        state->position.y = (-250.0f * scale) - 130.0f;
        if ((s32) state->frame == 10) {
            state->collision.kind = 2;
            fn_1_5780(&state->modelQueue);
        }
        if ((s32) state->frame >= 30) {
            Hu3DModelAttrSet(state->firstModel, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(state->secondModel, HU3D_ATTR_DISPOFF);
            state->kind = (s32) (state->kind + 1);
        }
        break;
    case 200:
        if (state->active != 0) {
            state->kind = 100;
        } else {
            state->frame = (s32) (state->frame + 1);
            if ((s32) state->player->excluded == 0) {
                deltaX = state->player->position.x - state->position.x;
                deltaZ = state->player->position.z - state->position.z;
                if (((deltaX * deltaX) + (deltaZ * deltaZ)) > 9025.0f) {
                    directionAngle = M619InitializationDirection(
                        state->player->position.x - state->position.x,
                        state->player->position.z - state->position.z);
                    state->position.x =
                        (f32) ((f64) state->position.x + (5.5 * sin((f64) directionAngle)));
                    state->position.z =
                        (f32) ((f64) state->position.z + (5.5 * cos((f64) directionAngle)));
                } else {
                    state->kind = 100;
                }
            }
            if ((s32) state->frame >= 120) {
                state->frame = 0;
                state->kind = (s32) (state->kind + 1);
            }
        }
        break;
    case 201:
        state->frame = (s32) (state->frame + 1);
        if ((s32) state->player->excluded == 0) {
            deltaX = state->player->position.x - state->position.x;
            deltaZ = state->player->position.z - state->position.z;
            if (((deltaX * deltaX) + (deltaZ * deltaZ)) > 9025.0f) {
                directionAngle = M619InitializationDirection(deltaX, deltaZ);
                state->position.x =
                    (f32) ((f64) state->position.x + (5.5 * sin((f64) directionAngle)));
                state->position.z =
                    (f32) ((f64) state->position.z + (5.5 * cos((f64) directionAngle)));
            } else {
                state->kind = 101;
            }
        }
        scale = (f32) state->frame / 30.0f;
        state->position.y = (-250.0f * scale) - 130.0f;
        if ((s32) state->frame == 10) {
            state->collision.kind = 2;
            fn_1_5780(&state->modelQueue);
        }
        if ((s32) state->frame >= 30) {
            Hu3DModelAttrSet(state->firstModel, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(state->secondModel, HU3D_ATTR_DISPOFF);
            state->kind = 102;
        }
        break;
    case 300:
        if (state->active != 0) {
            state->kind = 100;
        } else {
            state->frame = (s32) (state->frame + 1);
            scale = (f32) state->frame / 120.0f;
            state->angle += 1.0f;
            while (state->angle > 180.0f) {
                state->angle = (f32) (state->angle - 360.0f);
            }
            directionAngle = (f32) ((3.141592653589793 * (f64) state->angle) / 180.0);
            state->position.x =
                (f32) ((f64) (400.0f + (200.0f * scale)) * sin((f64) directionAngle));
            state->position.z =
                (f32) ((f64) (400.0f + (200.0f * scale)) * cos((f64) directionAngle));
            if ((s32) state->frame >= 120) {
                state->frame = 0;
                state->kind = (s32) (state->kind + 1);
            }
        }
        break;
    case 301:
        state->frame = (s32) (state->frame + 1);
        scale = (f32) state->frame / 30.0f;
        state->position.y = (-250.0f * scale) - 130.0f;
        state->angle += 1.0f;
        while (state->angle > 180.0f) {
            state->angle = (f32) (state->angle - 360.0f);
        }
        directionAngle = (f32) ((3.141592653589793 * (f64) state->angle) / 180.0);
        state->position.x = (f32) (600.0 * sin((f64) directionAngle));
        state->position.z = (f32) (600.0 * cos((f64) directionAngle));
        if ((s32) state->frame == 10) {
            state->collision.kind = 2;
            fn_1_5780(&state->modelQueue);
        }
        if ((s32) state->frame >= 30) {
            Hu3DModelAttrSet(state->firstModel, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(state->secondModel, HU3D_ATTR_DISPOFF);
            state->kind = 102;
        }
        break;
    case 400:
        if (state->active != 0) {
            state->kind = 100;
        } else {
            state->frame = (s32) (state->frame + 1);
            scale = (f32) state->frame / 120.0f;
            state->angle += 1.0f;
            while (state->angle > 180.0f) {
                state->angle = (f32) (state->angle - 360.0f);
            }
            directionAngle = (f32) ((3.141592653589793 * (f64) state->angle) / 180.0);
            state->position.x =
                (f32) ((f64) (400.0f - (200.0f * scale)) * sin((f64) directionAngle));
            state->position.z =
                (f32) ((f64) (400.0f - (200.0f * scale)) * cos((f64) directionAngle));
            if ((s32) state->frame >= 120) {
                state->frame = 0;
                state->kind = (s32) (state->kind + 1);
            }
        }
        break;
    case 401:
        state->frame = (s32) (state->frame + 1);
        scale = (f32) state->frame / 30.0f;
        state->position.y = (-250.0f * scale) - 130.0f;
        state->angle += 1.0f;
        while (state->angle > 180.0f) {
            state->angle = (f32) (state->angle - 360.0f);
        }
        directionAngle = (f32) ((3.141592653589793 * (f64) state->angle) / 180.0);
        state->position.x = (f32) (200.0 * sin((f64) directionAngle));
        state->position.z = (f32) (200.0 * cos((f64) directionAngle));
        if ((s32) state->frame == 10) {
            state->collision.kind = 2;
            fn_1_5780(&state->modelQueue);
        }
        if ((s32) state->frame >= 30) {
            Hu3DModelAttrSet(state->firstModel, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(state->secondModel, HU3D_ATTR_DISPOFF);
            state->kind = 102;
        }
        break;
    case 102:
        break;
    }
    state->rotation.y = (f32) (state->rotation.y - 0.3f);
}

/* Radial-actor callback: places both models and reports when the actor is inactive. */
s32 fn_1_39E8(void *scene) {
    M619RadialActor *sceneState;

    sceneState = scene;
    Hu3DModelPosSetV(sceneState->firstModel, &sceneState->position);
    Hu3DModelRotSetV(sceneState->firstModel, &sceneState->rotation);
    Hu3DModelPosSet(sceneState->secondModel, sceneState->position.x, -130.0f,
                    sceneState->position.z);
    return sceneState->kind == 102;
}

/* Collision response callback: steers a radial actor away from the contacted node. */
void fn_1_3A5C(M619MotionNode *actor, M619MotionNode *command,
               f32 step) {
    f32 angle;
    f32 angleForTrig;
    f32 deltaX;
    f32 deltaZ;
    f32 computedAngle;

    switch (command->kind) {
    case 1:
    case 2:
        deltaZ = command->position[0].z - actor->position[0].z;
        deltaX = command->position[0].x - actor->position[0].x;
        if (0.0f == deltaZ) {
            if (deltaX >= 0.0f) {
                angle = 1.5707964f;
            } else {
                angle = -1.5707964f;
            }
        } else {
            computedAngle = (f32)atan2(deltaX, deltaZ);
            angle = computedAngle;
        }
        angleForTrig = angle;
        actor->deltaX = (f32)((f64)actor->deltaX -
                         (f64)(0.5f * step) *
                             sin((f64)angleForTrig));
        actor->deltaZ = (f32)((f64)actor->deltaZ -
                         (f64)(0.5f * step) *
                             cos((f64)angleForTrig));
    case 0:
    case 3:
    case 4:
    default:
        break;
    }
}

/* Actor setup: registers collision callbacks and selects the actor's initial mode. */
void fn_1_3BFC(M619RadialActor *setup, s32 callbackState) {
    fn_1_E508(&setup->collision, setup->scale, 2);
    fn_1_E43C(&setup->collision, setup, fn_1_2724, fn_1_39E8, fn_1_3A5C, &setup->position);
    setup->kind = 0;
    setup->mode = callbackState;
}

/* Scene setup: activates four radial actors and assigns their collision kind. */
void fn_1_3C70(M619Scene *state) {
    M619RadialActor *entry;
    s32 entryIndex;

    entryIndex = 0;
    while (entryIndex < 4) {
        entry = &state->sceneAnimation.actors[entryIndex];
        entry->active = 1;
        entry->collision.kind = 2;
        entryIndex++;
    }
}

/* Per-frame impact callback: scales the models, attaches the hit pose, then hides them. */
s32 fn_1_3CBC(M619PlayerPose *burst) {
    Point3d projected;
    M619PlayerPose *context;
    f32 scale;
    f32 screenX;
    s16 pan;
    s32 finished;

    finished = 0;
    context = burst;
    switch (context->mode) {
    case 0:
        Hu3DModelAttrReset(context->secondModel, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(context->secondModel, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrSet(context->secondModel, HU3D_MOTATTR_LOOP);
        context->frame = 0;
        context->mode++;
        /* fall through */
    case 1:
        context->frame++;
        scale = (f32)context->frame / 10.0f;
        Hu3DModelScaleSet(context->secondModel, scale, scale, scale);
        context->position.x = (f32)context->player->position.x;
        context->position.z = (f32)context->player->position.z;
        context->rotation.y = (f32)(context->player->rotation.y + context->angle);
        if (context->frame >= 10) {
            context->mode++;
        }
        break;
    case 2:
        Hu3D3Dto2D((Point3d *)&context->position, lbl_1_rodata_10[0], &projected);
        screenX = projected.x / 576.0f;
        if ((screenX >= 0.0f) && (screenX <= 1.0f)) {
            pan = (s16)((s32)(64.0f * screenX) + 32);
            HuAudFXPlayPan(MSM_SE_M619_6E6, pan);
        }
        Hu3DModelAttrReset(context->firstModel, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(context->firstModel, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrReset(context->firstModel, HU3D_MOTATTR_PAUSE);
        Hu3DMotionTimeSet(context->firstModel, 0.0f);
        context->frame = 0;
        context->mode++;
        /* fall through */
    case 3:
        context->frame++;
        scale = (f32)context->frame / 15.0f;
        context->position.y = (f32)((-200.0f * (1.0f - scale)) - 300.0f);
        context->position.x = (f32)context->player->position.x;
        context->position.z = (f32)context->player->position.z;
        context->rotation.y = (f32)(context->player->rotation.y + context->angle);
        if (context->frame >= 15) {
            context->mode++;
        }
        break;
    case 4:
        CharFXPlay(context->player->character, MSM_SE_CHARVOICE_MARIO + 3);
        Hu3DModelHookSet(context->firstModel, "db_gesso", context->player->characterModel);
        fn_1_C1E8(context->player, context->angle);
        context->frame = 0;
        context->mode++;
        /* fall through */
    case 5:
        context->frame++;
        scale = (f32)context->frame / 60.0f;
        context->position.y = (f32)((50.0 * sin(3.141592653589793 * (f64)scale)) - 300.0);
        context->rotation.y += 3.0f;
        if (context->frame >= 60) {
            context->frame = 0;
            context->mode++;
        }
        break;
    case 6:
        context->frame++;
        scale = (f32)context->frame / 15.0f;
        context->rotation.y += 3.0f;
        context->position.y = (f32)((-150.0f * scale) - 300.0f);
        if (context->frame >= 15) {
            context->frame = 0;
            context->mode++;
        }
        break;
    case 7:
        context->frame++;
        scale = (f32)context->frame / 15.0f;
        context->position.y = (f32)(((-150.0f * scale) - 150.0f) - 300.0f);
        context->rotation.y += 3.0f;
        if (context->frame >= 15) {
            context->mode++;
        }
        break;
    case 8:
        Hu3DModelAttrSet(context->firstModel, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(context->secondModel, HU3D_ATTR_DISPOFF);
        finished = 1;
        break;
    }
    Hu3DModelPosSetV(context->firstModel, (Point3d *)&context->position);
    Hu3DModelRotSetV(context->firstModel, (Point3d *)&context->rotation);
    Hu3DModelPosSet(context->secondModel, context->position.x, -130.0f, context->position.z);
    Hu3DModelRotSetV(context->secondModel, (Point3d *)&context->rotation);
    return finished;
}

/* Animation callback: bobs and scales the scene model, firing callbacks at two distances. */
void fn_1_4278(M619SceneAnimation *state) {
    f32 wave;
    f32 scaleY;
    f32 positionY;
    f32 normalizedFrame;

    state->frame++;
    state->frame %= 300;
    scaleY = (f32)state->frame / 299.0f;
    wave = (f32)(0.5 *
                 (1.0 +
                  sin(2.0 *
                      (3.141592653589793 * (f64)scaleY))));
    positionY = (100.0f * wave) + state->height;
    Hu3DModelPosSet(state->model, 0.0f, positionY,
                    0.0f);
    normalizedFrame =
        ((0.25f * wave) + 0.75f) * state->scale;
    Hu3DModelScaleSet(state->model, state->scale, normalizedFrame,
                      state->scale);
    switch (state->phase) {
    case 0:
        if (state->progress >= 600.0f) {
            fn_1_449C(state, state->callbackValueA);
            state->phase++;
        }
        break;
    case 1:
        if (state->progress >= 1200.0f) {
            fn_1_449C(state, state->callbackValueB);
            state->phase++;
        }
        break;
    case 2:
        break;
    }
}

/* Called at scene-animation thresholds to position and register four radial actors. */
void fn_1_449C(M619SceneAnimation *state, s32 callbackValue) {
    M619RadialActor *slot;
    s32 slotIndex;
    f32 phaseAngle;

    slotIndex = 0;
    while (slotIndex < 4) {
        slot = &state->actors[slotIndex];
        if (callbackValue == 0) {
            if (slot->player->excluded == 0) {
                fn_1_E508(&slot->collision, slot->scale, 2);
                fn_1_E43C(&slot->collision, slot, fn_1_2724, fn_1_39E8, fn_1_3A5C, &slot->position);
                slot->kind = 0;
                slot->mode = callbackValue;
                slot->position.x = slot->player->position.x;
                slot->position.y = -130.0f;
                slot->position.z = slot->player->position.z;
            }
        } else if (callbackValue == 1) {
            fn_1_E508(&slot->collision, slot->scale, 2);
            fn_1_E43C(&slot->collision, slot, fn_1_2724, fn_1_39E8, fn_1_3A5C, &slot->position);
            slot->kind = 0;
            slot->mode = callbackValue;
            slot->angle = 45.0f +
                             (90.0f * (f32)slotIndex);
            phaseAngle = (f32)((3.141592653589793 * (f64)slot->angle) /
                               180.0);
            slot->position.x = (f32)(400.0 * sin((f64)phaseAngle));
            slot->position.y = -130.0f;
            slot->position.z = (f32)(400.0 * cos((f64)phaseAngle));
        } else {
            fn_1_E508(&slot->collision, slot->scale, 2);
            fn_1_E43C(&slot->collision, slot, fn_1_2724, fn_1_39E8, fn_1_3A5C, &slot->position);
            slot->kind = 0;
            slot->mode = callbackValue;
            slot->angle = 45.0f +
                             (90.0f * (f32)slotIndex);
            phaseAngle = (f32)((3.141592653589793 * (f64)slot->angle) /
                               180.0);
            slot->position.x = (f32)(400.0 * sin((f64)phaseAngle));
            slot->position.y = -130.0f;
            slot->position.z = (f32)(400.0 * cos((f64)phaseAngle));
        }
        slotIndex++;
    }
}

/* Opening setup: creates or links one looping sway model for each player. */
void fn_1_47A8(M619Scene *state) {
    HU3D_MODELID model;
    s32 playerNumber;

    for (playerNumber = 0; playerNumber < 4; playerNumber++) {
        if (state->models[3] < 0) {
            model = state->models[3] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[3], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(state->models[3]);
        }
        state->players[playerNumber]->sway.model = model;
        Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
        Hu3DModelLayerSet(model, 4);
        Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(model, 0.85f, 0.85f,
                          0.85f);
        Hu3DMotionSpeedSet(model, 0.5f);
    }
}

/* Per-frame camera callback: shifts the camera target and position around the orbit. */
void fn_1_4900(M619CameraOrbit *orbit) {
    HuVecF direction;
    HuVecF up;
    HuVecF side;
    HuVecF target;
    HuVecF position;
    HuVecF axis = {0.0f, 1.0f, 0.0f};    orbit->phase += 80.0f;
    while (orbit->phase >= 180.0f) {
        orbit->phase -= 360.0f;
    }

    target.x = orbit->target.x;
    target.y = (f32)(orbit->target.y +
                     10.0 *
                         sin(3.141592653589793 * orbit->elevation / 180.0) *
                         sin(3.141592653589793 * orbit->phase / 180.0));
    target.z = orbit->target.z;

    position.x = orbit->position.x;
    position.y = (f32)(orbit->position.y +
                        10.0 *
                            sin(3.141592653589793 * orbit->elevation / 180.0) *
                            sin(3.141592653589793 * orbit->phase / 180.0));
    position.z = orbit->position.z;

    PSVECSubtract((Vec *)orbit, (Vec *)&orbit->target, (Vec *)&direction);
    PSVECNormalize((Vec *)&direction, (Vec *)&direction);
    PSVECCrossProduct((Vec *)&axis, (Vec *)&direction, (Vec *)&side);
    PSVECNormalize((Vec *)&side, (Vec *)&side);
    PSVECCrossProduct((Vec *)&direction, (Vec *)&side, (Vec *)&up);
    PSVECNormalize((Vec *)&up, (Vec *)&up);
    Hu3DCameraPosSet(lbl_1_rodata_10[0], position.x, position.y, position.z,
                     up.x, up.y, up.z, target.x, target.y, target.z);
}

/* Per-frame bounds update: keeps players, selection models, and radial actors in range. */
s32 fn_1_4B44(M619Scene *scene) {
    M619Player *player;
    M619PlayerMotionPayload *secondary;
    M619SelectionRecord *record;
    M619RadialActor *radialMotion;
    f32 distanceSquared;
    f32 angleForTrig;
    s32 playerIndex;
    s32 slotIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        player = scene->players[playerIndex];
        if (player->movementActive != 0) {
            distanceSquared = (player->position.x * player->position.x) +
                              (player->position.z * player->position.z);
            if (distanceSquared > 302500.0f) {
                f32 playerFarDirectionX, playerFarDirectionZ, playerFarAngle;
                playerFarDirectionZ = player->position.z;
                playerFarDirectionX = player->position.x;
                if (playerFarDirectionZ == 0.0f) {
                    if (playerFarDirectionX >= 0.0f) {
                        playerFarAngle = 1.5707964f;
                    } else {
                        playerFarAngle = -1.5707964f;
                    }
                } else {
                    playerFarAngle = atan2f(playerFarDirectionX, playerFarDirectionZ);
                }
                angleForTrig = playerFarAngle;
                player->position.x = (f32)(550.0 * sin((f64)angleForTrig));
                player->position.z = (f32)(550.0 * cos((f64)angleForTrig));
            } else if (distanceSquared < 60025.0f) {
                if (player->excluded == 0) {
                    fn_1_C194(player);
                    fn_1_E5B8((s32 (*)(void *))fn_1_3CBC,
                              &scene->sceneAnimation.poses[playerIndex]);
                }
            } else if (distanceSquared < 38025.0f) {
                f32 playerNearDirectionX, playerNearDirectionZ, playerNearAngle;
                playerNearDirectionZ = player->position.z;
                playerNearDirectionX = player->position.x;
                if (playerNearDirectionZ == 0.0f) {
                    if (playerNearDirectionX >= 0.0f) {
                        playerNearAngle = 1.5707964f;
                    } else {
                        playerNearAngle = -1.5707964f;
                    }
                } else {
                    playerNearAngle = atan2f(playerNearDirectionX, playerNearDirectionZ);
                }
                angleForTrig = playerNearAngle;
                player->position.x = (f32)(195.0 * sin((f64)angleForTrig));
                player->position.z = (f32)(195.0 * cos((f64)angleForTrig));
            }
        }
        if (player->playerType != 0) {
            secondary = &player->motionPayload;
            if ((distanceSquared = (secondary->position.x * secondary->position.x) +
                                   (secondary->position.z * secondary->position.z)) >
                302500.0f) {
                f32 secondaryFarDirectionX, secondaryFarDirectionZ, secondaryFarAngle;
                secondaryFarDirectionZ = secondary->position.z;
                secondaryFarDirectionX = secondary->position.x;
                if (secondaryFarDirectionZ == 0.0f) {
                    if (secondaryFarDirectionX >= 0.0f) {
                        secondaryFarAngle = 1.5707964f;
                    } else {
                        secondaryFarAngle = -1.5707964f;
                    }
                } else {
                    secondaryFarAngle = atan2f(secondaryFarDirectionX, secondaryFarDirectionZ);
                }
                angleForTrig = secondaryFarAngle;
                secondary->position.x = (f32)(550.0 * sin((f64)angleForTrig));
                secondary->position.z = (f32)(550.0 * cos((f64)angleForTrig));
            } else if (distanceSquared < 60025.0f) {
                f32 secondaryNearDirectionX, secondaryNearDirectionZ, secondaryNearAngle;
                secondaryNearDirectionZ = secondary->position.z;
                secondaryNearDirectionX = secondary->position.x;
                if (secondaryNearDirectionZ == 0.0f) {
                    if (secondaryNearDirectionX >= 0.0f) {
                        secondaryNearAngle = 1.5707964f;
                    } else {
                        secondaryNearAngle = -1.5707964f;
                    }
                } else {
                    secondaryNearAngle = atan2f(secondaryNearDirectionX, secondaryNearDirectionZ);
                }
                angleForTrig = secondaryNearAngle;
                secondary->position.x = (f32)(245.0 * sin((f64)angleForTrig));
                secondary->position.z = (f32)(245.0 * cos((f64)angleForTrig));
            }
        }
    }
    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        for (slotIndex = 0; slotIndex < 8; slotIndex++) {
            record = &scene->selection.records[playerIndex][slotIndex];
            if (record->initialized != 0 &&
                ((distanceSquared = (record->position.x * record->position.x) +
                                    (record->position.z * record->position.z)) <
                 ((130.0f + record->speed) *
                  (130.0f + record->speed)))) {
                fn_1_74E0(record);
                fn_1_E4A4(&record->collision);
            }
        }
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        radialMotion = &scene->sceneAnimation.actors[playerIndex];
        if (radialMotion->kind != 102 &&
            ((distanceSquared =
                  (radialMotion->position.x * radialMotion->position.x) +
                  (radialMotion->position.z * radialMotion->position.z)) <
             40000.0f)) {
            f32 radialDirectionX, radialDirectionZ, radialAngle;
            radialDirectionZ = radialMotion->position.z;
            radialDirectionX = radialMotion->position.x;
            if (radialDirectionZ == 0.0f) {
                if (radialDirectionX >= 0.0f) {
                    radialAngle = 1.5707964f;
                } else {
                    radialAngle = -1.5707964f;
                }
            } else {
                radialAngle = atan2f(radialDirectionX, radialDirectionZ);
            }
            angleForTrig = radialAngle;
            radialMotion->position.x = (f32)(200.0 * sin((f64)angleForTrig));
            radialMotion->position.z = (f32)(200.0 * cos((f64)angleForTrig));
        }
    }
}

/* Adds a positioned model group to the scene queue and initializes its three copies. */
void fn_1_51EC(M619ModelQueueNode *node, const HuVecF *position) {
    M619Scene *queue;

    queue = (M619Scene *)lbl_1_bss_0->data;
    node->next = queue->modelQueue;
    node->previous = NULL;
    queue->modelQueue = node;
    if (node->next) {
        node->next->previous = node;
    }
    node->sourcePosition = position;
    node->state = 0;
    node->flags = 0;
    node->modelPositions[0] = *position;
    node->modelPositions[1] = *position;
    node->modelPositions[2] = *position;
    Hu3DModelPosSetV(node->models[0], &node->modelPositions[0]);
    Hu3DModelPosSetV(node->models[1], &node->modelPositions[1]);
    Hu3DModelPosSetV(node->models[2], &node->modelPositions[2]);
}

/* Unlinks a model group from the scene queue; a tail node keeps its previous link. */
void fn_1_52E0(M619ModelQueueNode *node) {
    M619Scene *owner;

    owner = (M619Scene *)lbl_1_bss_0->data;
    if (node->previous) {
        node->previous->next = node->next;
    } else {
        owner->modelQueue = node->next;
    }
    if (node->next) {
        node->next->previous = node->previous;
        node->next = NULL;
        if (node->previous) {
            node->previous = NULL;
        }
    }
}

/* Per-frame queue update: advances each model point and resets ended motions at its source. */
void fn_1_535C(M619Scene *owner) {
    M619ModelQueueNode *node;
    s32 pointIndex;
    f32 distanceSquared;

    for (node = owner->modelQueue; node; node = node->next) {
        if (node->flags != 0) {
            if (node->state % 20 == 0) {
                pointIndex = node->state / 20;
                Hu3DModelAttrReset(node->models[pointIndex], HU3D_ATTR_DISPOFF);
                node->modelPositions[pointIndex].x = node->sourcePosition->x;
                node->modelPositions[pointIndex].y = 0.0f;
                node->modelPositions[pointIndex].z = node->sourcePosition->z;
                Hu3DMotionTimeSet(node->models[pointIndex], 0.0f);
            }
            if (node->state <= 40) {
                node->state++;
            }
        }
        for (pointIndex = 0; pointIndex < 3; pointIndex++) {
            if (Hu3DMotionEndCheck(node->models[pointIndex]) != 0) {
                node->modelPositions[pointIndex].x = node->sourcePosition->x;
                node->modelPositions[pointIndex].y = 0.0f;
                node->modelPositions[pointIndex].z = node->sourcePosition->z;
                Hu3DMotionTimeSet(node->models[pointIndex], 0.0f);
                if (node->flags == 0) {
                    Hu3DModelAttrSet(node->models[pointIndex], HU3D_ATTR_DISPOFF);
                }
            } else {
                distanceSquared =
                    (node->modelPositions[pointIndex].x * node->modelPositions[pointIndex].x) +
                    (node->modelPositions[pointIndex].z * node->modelPositions[pointIndex].z);
                if (distanceSquared < 378225.0f) {
                    fn_1_5794(&node->modelPositions[pointIndex]);
                }
            }
            Hu3DModelPosSetV(node->models[pointIndex], &node->modelPositions[pointIndex]);
        }
    }
}

/* Creates or links the three scaled animation models used by a queued motion group. */
void fn_1_55BC(M619ModelQueueNode *group, f32 scale, s32 variant) {
    M619Scene *repository;
    HU3D_MODELID model;
    s32 modelIndex;

    repository = (M619Scene *)lbl_1_bss_0->data;
    for (modelIndex = 0; modelIndex < 3; modelIndex++) {
        if (repository->models[variant + 5] < 0) {
            model = repository->models[variant + 5] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[variant + 5], HU_MEMNUM_OVL,
                                     HEAP_MODEL));
        } else {
            model = Hu3DModelLink(repository->models[variant + 5]);
        }
        group->models[modelIndex] = model;
        Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(model, 6);
        Hu3DModelPosSet(model, 0.0f, 0.0f,
                        0.0f);
        Hu3DModelRotSet(model, 0.0f, 0.0f,
                        0.0f);
        Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
        Hu3DMotionTimeSet(model, 20.0f * (f32)modelIndex);
        Hu3DModelScaleSet(model, scale, scale, scale);
    }
}

void fn_1_5774(M619ModelQueueNode *queueNode) {
    queueNode->flags = 1;
}

void fn_1_5780(M619ModelQueueNode *queueNode) {
    queueNode->flags = 0;
    queueNode->state = 0;
}

/* Adjusts an active motion point's radial path around the minigame center. */
void fn_1_5794(HuVecF *point) {
    M619Scene *control;
    f32 travelAngle;
    f32 x;
    f32 z;
    f32 angle;
    f32 directionAngle;
    f32 distanceSquared;

    control = (M619Scene *)lbl_1_bss_0->data;
    if (control->adjustmentEnabled == 0) {
        return;
    }
    z = point->z;
    x = point->x;
    if (0.0f == z) {
        if (x >= 0.0f) {
            angle = 1.5707964f;
        } else {
            angle = -1.5707964f;
        }
    } else {
        angle = atan2f(x, z);
    }
    directionAngle = angle;
    distanceSquared = (point->x * point->x) + (point->z * point->z);
    travelAngle = (f32)((6.283185307179586 * (f64)sqrtf(distanceSquared)) /
                        600.0);
    point->x = (f32)((f64)point->x +
                     ((2.0 * sin(3.141592653589793 + (f64)directionAngle)) -
                      ((f64)travelAngle *
                       sin(1.5707963267948966 + (f64)directionAngle))));
    point->z = (f32)((f64)point->z +
                     ((2.0 * cos(3.141592653589793 + (f64)directionAngle)) -
                      ((f64)travelAngle *
                       cos(1.5707963267948966 + (f64)directionAngle))));
}

/* Selection setup: creates the record's three display models and queued motion group. */
void fn_1_5A80(M619SelectionRecord *node, s32 mode) {
    M619Scene *state =
        (M619Scene *)lbl_1_bss_0->data;
    HU3D_MODELID model;
    s32 useAlternate;
    f32 scale;
    f32 nodeValueAtB8[2] = { 70.71068f, 75.0f };
    f32 nodeValueAtBC[2] = { 0.6f, 0.9f };

    useAlternate = mode == 2;

    if (state->models[useAlternate + 10] < 0) {
        model = state->models[useAlternate + 10] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_28[useAlternate + 10], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(state->models[useAlternate + 10]);
    }
    node->models[0] = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    if (useAlternate) {
        Hu3DModelScaleSet(model, 0.75f, 0.75f, 0.75f);
    }

    if (state->models[useAlternate + 12] < 0) {
        model = state->models[useAlternate + 12] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_28[useAlternate + 12], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(state->models[useAlternate + 12]);
    }
    node->models[1] = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    if (useAlternate) {
        Hu3DModelScaleSet(model, 0.75f, 0.75f, 0.75f);
        Hu3DMotionSpeedSet(model, 0.5f);
    } else {
        Hu3DMotionSpeedSet(model, 0.25f);
    }

    if (useAlternate) {
        if (state->models[3] < 0) {
            model = state->models[3] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[3], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(state->models[3]);
        }
    } else {
        if (state->models[4] < 0) {
            model = state->models[4] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[4], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(state->models[4]);
        }
    }
    node->models[2] = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 4);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.5f);
    if (useAlternate) {
        Hu3DModelScaleSet(model, 1.5f, 1.5f, 1.5f);
    }

    node->initialized = 0;
    node->mode = mode;
    node->offset = nodeValueAtBC[useAlternate];
    node->frame = 0;
    node->speed = nodeValueAtB8[useAlternate];
    scale = 2.0f * (node->speed / 50.0f);

    fn_1_55BC(&node->modelQueue, scale, 1);
    fn_1_51EC(&node->modelQueue, &node->position);
}

/* Initializes all selection records, randomizes player and group order, and clears state. */
void fn_1_6140(M619Selection *selection) {
    s32 groupIndex;
    s32 slotIndex;

    for (groupIndex = 0; groupIndex < 3; groupIndex++) {
        for (slotIndex = 0; slotIndex < 8; slotIndex++) {
            fn_1_5A80(&selection->records[groupIndex][slotIndex], groupIndex);
        }
    }
    fn_1_61DC(selection);
    fn_1_6290(selection);
    selection->state = 0;
    selection->status = 0;
}

/* Builds a randomized order for the four players before a selection cycle. */
void fn_1_61DC(M619Selection *selection) {
    s32 index;
    s32 firstIndex;
    s32 secondIndex;
    s32 player;

    for (index = 0; index < 4; index++) {
        selection->playerOrder[index] = index;
    }
    for (index = 0; index < 4; index++) {
        firstIndex = frandmod(4);
        secondIndex = frandmod(4);
        player = selection->playerOrder[firstIndex];
        selection->playerOrder[firstIndex] = selection->playerOrder[secondIndex];
        selection->playerOrder[secondIndex] = player;
    }
    selection->selectionState = 0;
}

/* Builds a randomized order for the three motion groups before a selection cycle. */
void fn_1_6290(M619Selection *selection) {
    s32 index;
    s32 firstIndex;
    s32 secondIndex;
    s32 group;
    s32 value;

    for (index = 0; index < 3; index++) {
        selection->groupOrder[index] = index;
    }
    for (index = 0; index < 3; index++) {
        firstIndex = frandmod(3);
        secondIndex = frandmod(3);
        group = selection->groupOrder[firstIndex];
        selection->groupOrder[firstIndex] = selection->groupOrder[secondIndex];
        selection->groupOrder[secondIndex] = group;
    }
    selection->groupState = 0;
}

/* Starts the next selected group of motion records and refreshes orders after each cycle. */
void fn_1_6344(M619Selection *selection) {
    M619SelectionRecord *motion;
    f32 angle;
    s32 groupIndex;
    s32 playerIndex;
    s32 slotIndex;
    s32 groupInitializeIndex;
    s32 groupSecondIndex;
    s32 groupFirstIndex;
    s32 playerInitializeIndex;
    s32 playerSecondIndex;
    s32 playerFirstIndex;
    s32 groupValue;
    s32 playerValue;

    groupIndex = selection->groupOrder[selection->groupState];
    playerIndex = selection->playerOrder[selection->selectionState];
    motion = (M619SelectionRecord *)&selection->records[groupIndex][0];
    for (slotIndex = 0; slotIndex < 8; slotIndex++,
         motion++) {
        if (motion->initialized != 0) {
            continue;
        }
        motion->initialized = 1;
        angle = (f32)((3.141592653589793 * (f64)(90.0f * (f32)playerIndex)) /
                      180.0);
        motion->position.x = (f32)(1100.0 * sin((f64)angle));
        motion->position.y = 0.0f;
        motion->position.z = (f32)(1100.0 * cos((f64)angle));
        motion->impulse.x = 0.0f;
        motion->impulse.y = 0.0f;
        motion->impulse.z = 0.0f;
        motion->damping = 0.0f;
        if (playerIndex == 0 || playerIndex == 2) {
            motion->position.x = (f32)(motion->position.x +
                ((100.0f * (f32)frandmod(3)) - 100.0f));
            motion->correctionX = 0.0f;
            if (playerIndex == 0) {
                motion->correctionZ = (f32)(-2.0f * motion->offset);
            } else {
                motion->correctionZ = (f32)(2.0f * motion->offset);
            }
        } else {
            motion->position.z = (f32)(motion->position.z +
                ((100.0f * (f32)frandmod(3)) - 100.0f));
            motion->correctionZ = 0.0f;
            if (playerIndex == 1) {
                motion->correctionX = (f32)(-2.0f * motion->offset);
            } else {
                motion->correctionX = (f32)(2.0f * motion->offset);
            }
        }
        motion->amplitude = (f32)(1.0f +
            (0.2f * (((f32)frandmod(10001) / 10000.0f) - 0.5f)));
        motion->rotation.y = (f32)(360.0f *
            ((f32)frandmod(10001) / 10000.0f));
        if (motion->mode == 2) {
            motion->rotation.x = (f32)(10.0f *
                ((f32)frandmod(10001) / 10000.0f));
        } else {
            motion->rotation.x = (f32)(17.5f *
                ((f32)frandmod(10001) / 10000.0f));
        }
        motion->rotation.z = 0.0f;
        motion->angle = 0.0f;
        motion->angleStep = (f32)((3.141592653589793 *
            (360.0 / (90.0 + (30.0 *
             (f64)((f32)frandmod(10001) / 10000.0f))))) / 180.0);
        motion->motionState = 0;
        motion->countdown = 0;
        Hu3DModelAttrReset(motion->models[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(motion->models[2], HU3D_ATTR_DISPOFF);
        fn_1_E508(&motion->collision, motion->speed, 3);
        fn_1_E43C(&motion->collision, motion, fn_1_6A8C, fn_1_7058, fn_1_70E0, &motion->position);
        motion->modelQueue.flags = 1;
        selection->groupState++;
        selection->selectionState++;
        if (selection->groupState >= 3) {
            for (groupInitializeIndex = 0; groupInitializeIndex < 3;
                 groupInitializeIndex++) {
                selection->groupOrder[groupInitializeIndex] = groupInitializeIndex;
            }
            for (groupInitializeIndex = 0; groupInitializeIndex < 3;
                 groupInitializeIndex++) {
                groupFirstIndex = frandmod(3);
                groupSecondIndex = frandmod(3);
                groupValue = selection->groupOrder[groupFirstIndex];
                selection->groupOrder[groupFirstIndex] =
                    selection->groupOrder[groupSecondIndex];
                selection->groupOrder[groupSecondIndex] =
                    groupValue;
            }
            selection->groupState = 0;
        }
        if (selection->selectionState >= 4) {
            for (playerInitializeIndex = 0; playerInitializeIndex < 4;
                 playerInitializeIndex++) {
                selection->playerOrder[playerInitializeIndex] =
                    playerInitializeIndex;
            }
            for (playerInitializeIndex = 0; playerInitializeIndex < 4;
                 playerInitializeIndex++) {
                playerFirstIndex = frandmod(4);
                playerSecondIndex = frandmod(4);
                playerValue = selection->playerOrder[playerFirstIndex];
                selection->playerOrder[playerFirstIndex] =
                    selection->playerOrder[playerSecondIndex];
                selection->playerOrder[playerSecondIndex] =
                    playerValue;
            }
            selection->selectionState = 0;
        }
        break;
    }
}

/* Per-frame selection update: waits between launch groups and starts the next one. */
void fn_1_69E8(M619Selection *motion) {
    switch (motion->status) {
    case 0:
        break;
    case 1:
        motion->state++;
        if (motion->state > 60) {
            motion->state = 0;
            motion->status++;
        }
        break;
    case 2:
        motion->state++;
        if (motion->state > 75) {
            fn_1_6344(motion);
            motion->state = 0;
        }
        break;
    }
}

/* Updates one queued motion record each frame as its point follows a circular sweep. */
void fn_1_6A8C(void *selectionRecord) {
    f32 x;
    M619SelectionRecord * motion;
    f32 z;
    f32 angle;
    f32 directionAngle;
    f32 distanceSquared;
    f32 stepAngle;
    f64 firstRoot;
    f64 thirdRoot;
    motion = selectionRecord;
    z = motion->position.z;
    x = motion->position.x;
    if (z == 0.0f)
    {
        if (x >= 0.0f)
        {
            angle = 1.5707964f;
        }
        else
        {
            angle = - 1.5707964f;
        }
    }
    else
    {
        angle = atan2f(x, z);
    }
    directionAngle = angle;
    distanceSquared =
        (motion->position.x * motion->position.x) + (motion->position.z * motion->position.z);
    if (distanceSquared < 378225.0f)
    {
        stepAngle = (f32)((6.283185307179586 * (f64) sqrtf(distanceSquared)) / 600.0);
        motion->position.x =
            (f32) ((f64) motion->position.x +
                   (((f64) motion->offset * (2.0 * sin(3.141592653589793 + (f64) directionAngle))) -
                    ((f64) stepAngle * sin(1.5707963267948966 + (f64) directionAngle))));
        motion->position.z =
            (f32) ((f64) motion->position.z +
                   (((f64) motion->offset * (2.0 * cos(3.141592653589793 + (f64) directionAngle))) -
                    ((f64) stepAngle * cos(1.5707963267948966 + (f64) directionAngle))));
        if (! (motion->motionState))
        {
            motion->motionState = 1;
            motion->countdown = 60;
        }
    }
    else
    {
        if (motion->motionState == 1)
        {
            motion->correctionX = (f32)(2.0 * sin(3.141592653589793 + (f64) directionAngle));
            motion->correctionZ = (f32)(2.0 * cos(3.141592653589793 + (f64) directionAngle));
            motion->motionState = 0;
        }
        motion->position.x = (f32)(motion->position.x + motion->correctionX);
        motion->position.z = (f32)(motion->position.z + motion->correctionZ);
    }
    motion->rotation.y = (f32)(motion->rotation.y - ((360.0f * motion->amplitude) / 600.0f));
    motion->angle = (f32)(motion->angle + motion->angleStep);
    while (motion->angle > 3.141592653589793)
    {
        motion->angle = (f32)((f64) motion->angle - 6.283185307179586);
    }
    if (motion->mode == 2)
    {
        motion->position.y =
            (f32) (((f64) motion->amplitude * (22.5 * sin((f64) motion->angle))) - 10.0);
    }
    else
    {
        motion->position.y =
            (f32) (((f64) motion->amplitude * (7.5 * sin((f64) motion->angle))) - 5.0);
    }
    if (motion->countdown > 0)
    {
        motion->countdown--;
        stepAngle = (f32) motion->countdown / 60.0f;
        motion->rotation.y = (f32)(motion->rotation.y - (5.0f * (stepAngle * stepAngle)));
    }
    motion->damping *= 0.98f;
    if (motion->damping > 10.0f)
    {
        motion->damping = 10.0f;
    }
    if (motion->damping < - 10.0f)
    {
        motion->damping = - 10.0f;
    }
    motion->rotation.y = (f32)(motion->rotation.y + motion->damping);
    motion->position.x = (f32)(motion->position.x + motion->impulse.x);
    motion->position.z = (f32)(motion->position.z + motion->impulse.z);
    motion->impulse.x *= 0.9f;
    motion->impulse.z *= 0.9f;
}

/* Collision update: applies a selection record's pose and retires it when uninitialized. */
s32 fn_1_7058(void *pose) {
    M619SelectionRecord *poseState;
    s32 result;

    poseState = pose;
    result = 0;
    Hu3DModelPosSetV(poseState->models[0], &poseState->position);
    Hu3DModelRotSetV(poseState->models[0], &poseState->rotation);
    Hu3DModelPosSet(poseState->models[2], poseState->position.x, 0.0f, poseState->position.z);
    if (poseState->initialized == 0) {
        fn_1_74E0(poseState);
        result = 1;
    }
    return result;
}

/* Collision response: bends a selection point away from the contacted motion node. */
void fn_1_70E0(M619MotionNode *target,
               M619MotionNode *source, f32 speed) {
    M619SelectionRecord *control;
    M619RadialActor *motion;
    f32 targetMagnitudeSquared;
    f32 sourceMagnitudeSquared;
    f32 blendDeltaX;
    f32 blendDeltaZ;
    f32 blendAngle;
    f32 blendAngleResult;
    f32 angleForTrig;
    f32 driftDeltaX;
    f32 driftDeltaZ;
    f32 driftAngle;
    f32 driftAngleResult;

    control = target->owner;
    switch (source->kind) {
    case 1:
        blendDeltaZ = source->position->z - target->position->z;
        blendDeltaX = source->position->x - target->position->x;
        if (0.0f == blendDeltaZ) {
            if (blendDeltaX >= 0.0f) {
                blendAngle = 1.5707964f;
            } else {
                blendAngle = -1.5707964f;
            }
        } else {
            blendAngleResult = (f32)atan2(blendDeltaX, blendDeltaZ);
            blendAngle = blendAngleResult;
        }
        angleForTrig = blendAngle;
        target->deltaX -= speed * sin((f64)angleForTrig);
        target->deltaZ -= speed * cos((f64)angleForTrig);
        control->impulse.x += 0.1f * target->deltaX;
        control->impulse.z += 0.1f * target->deltaZ;
        motion = source->owner;
        if (motion->mode != 0) {
            targetMagnitudeSquared =
                target->position->x * target->position->x +
                target->position->z * target->position->z;
            sourceMagnitudeSquared =
                source->position->x * source->position->x +
                source->position->z * source->position->z;
            if (targetMagnitudeSquared < sourceMagnitudeSquared) {
                control->damping =
                    20.0f * (1.0f -
                        (targetMagnitudeSquared / sourceMagnitudeSquared));
            } else {
                control->damping =
                    -20.0f * (1.0f -
                        (sourceMagnitudeSquared / targetMagnitudeSquared));
            }
        }
        break;
    case 0:
    case 3:
        driftDeltaZ = source->position->z - target->position->z;
        driftDeltaX = source->position->x - target->position->x;
        if (0.0f == driftDeltaZ) {
            if (driftDeltaX >= 0.0f) {
                driftAngle = 1.5707964f;
            } else {
                driftAngle = -1.5707964f;
            }
        } else {
            driftAngleResult = (f32)atan2(driftDeltaX, driftDeltaZ);
            driftAngle = driftAngleResult;
        }
        angleForTrig = driftAngle;
        target->deltaX -= 0.5f * speed *
                                sin((f64)angleForTrig);
        target->deltaZ -= 0.5f * speed *
                                cos((f64)angleForTrig);
        break;
    case 2:
    case 4:
        break;
    default:
        break;
    }
    if (source->kind == 0) {
        control->initialized = 0;
    }
}

/* Ends a selection record and plays its return motion with a screen-positioned cue. */
void fn_1_74E0(M619SelectionRecord *pose) {
    Point3d screenPosition;
    f32 heightRatio;
    s16 pan;
    pose->initialized = 0;
    Hu3DModelAttrSet(pose->models[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(pose->models[2], HU3D_ATTR_DISPOFF);
    ((M619SelectionRecord *)pose)->modelQueue.flags = 0;
    ((M619SelectionRecord *)pose)->modelQueue.state = 0;
    if (fn_1_E5B8((s32 (*)(void *))fn_1_7640, pose) != 0U) {
        Hu3DModelPosSetV(pose->models[1], &pose->position);
        Hu3DModelRotSetV(pose->models[1], &pose->rotation);
        Hu3DMotionTimeSet(pose->models[1], 0.0f);
        Hu3DModelAttrReset(pose->models[1], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(pose->models[1], HU3D_MOTATTR_PAUSE);
        Hu3D3Dto2D(&pose->position, (s16)lbl_1_rodata_10[0], &screenPosition);
        heightRatio = screenPosition.x / 576.0f;
        if ((heightRatio >= 0.0f) && (heightRatio <= 1.0f)) {
            pan = (s16)((s32)(64.0f * heightRatio) + 32);
            HuAudFXPlayPan(MSM_SE_M619_6E5, pan);
        }
    }
}

/* Motion callback: hides the returning selection model when its animation ends. */
s32 fn_1_7640(M619SelectionRecord *selectionRecord) {
    M619SelectionRecord *visibilityState;

    visibilityState = selectionRecord;
    if (Hu3DMotionEndCheck(visibilityState->models[1]) != 0) {
        Hu3DModelAttrSet(visibilityState->models[1], HU3D_ATTR_DISPOFF);
        return 1;
    }
    return 0;
}

/* Clears active selection records, unregisters collisions, and starts their blink-out. */
void fn_1_7694(M619Scene *collection) {
    M619SelectionRecord *record;
    s32 groupIndex;
    s32 slotIndex;

    for (groupIndex = 0; groupIndex < 3; groupIndex++) {
        for (slotIndex = 0; slotIndex < 8; slotIndex++) {
            record = &collection->selection.records[groupIndex][slotIndex];
            if (record->initialized != 0) {
                record->initialized = 0;
                record->modelQueue.flags = 0;
                record->modelQueue.state = 0;
                fn_1_E4A4(&record->collision);
                fn_1_E5B8((s32 (*)(void *))fn_1_7748, record);
            }
        }
    }
}

/* Blink callback: alternates selection model visibility, then leaves the models hidden. */
s32 fn_1_7748(M619SelectionRecord *selectionRecord) {
    M619SelectionRecord *visibilityState;

    visibilityState = selectionRecord;
    visibilityState->frame = visibilityState->frame + 1;
    if ((s32) (visibilityState->frame & 1) != 0) {
        Hu3DModelAttrSet(visibilityState->models[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(visibilityState->models[2], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(visibilityState->models[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(visibilityState->models[2], HU3D_ATTR_DISPOFF);
    }
    if ((s32) visibilityState->frame >= 90) {
        Hu3DModelAttrSet(visibilityState->models[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(visibilityState->models[2], HU3D_ATTR_DISPOFF);
        return 1;
    }
    return 0;
}

/* Per-frame player update: detects each hook point crossing the sweep and emits a flash. */
void fn_1_77F4(M619Scene *scene) {
    M619Player *player;
    s32 playerNumber;

    for (playerNumber = 0; playerNumber < 4; playerNumber++) {
        player = scene->players[playerNumber];
        if (player->turnFrames != 0) {
            Hu3DModelObjPosGet(player->characterModel, player->hookNameA,
                               &player->positionA);
            if (player->crossingStateA != 0) {
                if (player->positionA.y < 0.0f) {
                    player->crossingStateA = 0;
                    fn_1_C890(&player->flashA);
                }
            } else if (player->positionA.y >= 0.0f) {
                player->crossingStateA = 1;
            }

            Hu3DModelObjPosGet(player->characterModel, player->hookNameB,
                               &player->positionB);
            if (player->crossingStateB != 0) {
                if (player->positionB.y < 0.0f) {
                    player->crossingStateB = 0;
                    fn_1_C890(&player->flashB);
                }
            } else if (player->positionB.y >= 0.0f) {
                player->crossingStateB = 1;
            }
        }
    }
}

/* Sets up an NPC motion payload with difficulty-scaled speed and collision callbacks. */
void fn_1_792C(M619PlayerMotionPayload *recipient, M619Player *source) {
    M619SpeedFactorTable speedFactorTable = {{0.75f, 1.0f, 1.25f, 1.5f}};
    f32 speed;
    if (source->playerType != 0) {
        speed = 130.0f * speedFactorTable.factors[source->difficulty];
        recipient->owner = source;
        recipient->position = source->position;
        recipient->usesPlayerHeading = 0;
        fn_1_E508(&recipient->collision, speed, 4);
        fn_1_E43C(&recipient->collision, recipient, fn_1_7A20, fn_1_7D64, fn_1_7D84,
                  &recipient->position);
    }
}

/* Advances an NPC's queued motion point along its path when player input is active. */
void fn_1_7A20(void *motionPayload) {
    s32 pathParameter;
    M619PlayerMotionPayload *recipient;
    f32 tangent;
    f32 x;
    f32 z;
    f32 angle;
    f32 angleResult;
    f32 angleForTrig;
    f64 refinedRoot;
    f32 distanceSquared;
    f64 reciprocalRoot;

    recipient = motionPayload;
    pathParameter = (s32)lbl_1_bss_0->data;
    {
    M619MotionPathFactorTable pathFactors = {{0.1f, 0.3f, 6.0f, 7.0f}};
    if (((M619Player *)recipient->owner)->inputEnabled != 0) {
        z = recipient->position.z;
        x = recipient->position.x;
        if (0.0f == z) {
            if (x >= 0.0f) {
                angle = 1.5707964f;
            } else {
                angle = -1.5707964f;
            }
        } else {
            angleResult = (f32)atan2(x, z);
            angle = angleResult;
        }
        angleForTrig = angle;
        distanceSquared = (recipient->position.x * recipient->position.x) +
                          (recipient->position.z * recipient->position.z);
        tangent = (f32)(5.0 +
                        ((6.283185307179586 * (f64)sqrtf(distanceSquared)) /
                         600.0));
        recipient->position.x =
            (f32) ((f64) recipient->position.x +
                   ((pathFactors.factors[((M619Player *) recipient->owner)->difficulty] *
                     sin((f64) angleForTrig)) -
                    ((f64) tangent * sin(1.5707963267948966 + (f64) angleForTrig))));
        recipient->position.z =
            (f32) ((f64) recipient->position.z +
                   ((pathFactors.factors[((M619Player *) recipient->owner)->difficulty] *
                     cos((f64) angleForTrig)) -
                    ((f64) tangent * cos(1.5707963267948966 + (f64) angleForTrig))));
    } else {
        recipient->position.x =
            ((M619Player *)recipient->owner)->position.x;
        recipient->position.z =
            ((M619Player *)recipient->owner)->position.z;
    }
    recipient->usesPlayerHeading = 0;
    }
}

/* Collision query: returns whether the NPC owner's player has been excluded. */
s32 fn_1_7D64(void *motionPayload) {
    M619PlayerMotionPayload *payload = motionPayload;
    return payload->owner->excluded;
}

/* Collision response: steers an NPC motion point and records when it follows heading. */
void fn_1_7D84(M619MotionNode *target, M619MotionNode *request,
               f32 step) {
    f32 angle;
    f32 angleForTrig;
    f32 deltaX;
    f32 deltaZ;
    f32 computedAngle;
    M619PlayerMotionPayload *state;

    state = target->owner;
    switch (request->kind) {
    case 3:
    case 1:
        state->usesPlayerHeading = 1;
        /* fall through */
    case 2:
        deltaZ = request->position->z - target->position->z;
        deltaX = request->position->x - target->position->x;
        if (0.0f == deltaZ) {
            if (deltaX >= 0.0f) {
                angle = 1.5707964f;
            } else {
                angle = -1.5707964f;
            }
        } else {
            computedAngle = (f32)atan2(deltaX, deltaZ);
            angle = computedAngle;
        }
        angleForTrig = angle;
        target->deltaX = (f32)((f64)target->deltaX -
                          (f64)(0.5f * step) *
                              sin((f64)angleForTrig));
        target->deltaZ = (f32)((f64)target->deltaZ -
                          (f64)(0.5f * step) *
                              cos((f64)angleForTrig));
        break;
    case 4:
    case 0:
        break;
    }
}

/* Advances the first pair of scene models through its timed pulse and fade. */
s32 fn_1_7F44(void *scene) {
    M619Scene *pulse;
    HSF_DATA *modelData;
    HSF_MATERIAL *material;
    f32 scale;
    f32 angle;
    s32 finished;
    s32 index;
    s32 materialCount;

    finished = 0;
    pulse = scene;
    switch (pulse->pulseState) {
    case 0:
        pulse->pulseFrame = 0;
        pulse->pulseState++;
        /* fall through */
    case 1:
        pulse->pulseFrame++;
        scale = (f32)pulse->pulseFrame / 60.0f;
        angle = (f32)cos((3.141592653589793 *
                          (f64)(30.0f * scale)) /
                         180.0);
        if (pulse->nightLighting != 0) {
            Hu3DModelScaleSet(pulse->models[27],
                              angle, angle, angle);
            Hu3DModelScaleSet(pulse->models[28],
                              angle, angle, angle);
        } else {
            Hu3DModelScaleSet(pulse->models[0],
                              angle, angle, angle);
            Hu3DModelScaleSet(pulse->models[1],
                              angle, angle, angle);
        }
        if (pulse->pulseFrame >= 60) {
            if (pulse->nightLighting != 0) {
                Hu3DModelAttrReset(pulse->models[31], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(pulse->models[31],
                                    HU3D_MOTATTR_PAUSE);
                Hu3DModelAttrSet(pulse->models[31],
                                  HU3D_MOTATTR_LOOP);
                modelData = Hu3DData[pulse->models[31]].hsf;
                material = modelData->material;
                materialCount = modelData->materialNum;
                for (index = 0; index < materialCount; index++, material++) {
                    material->invAlpha = 1.0f;
                }
            } else {
                Hu3DModelAttrReset(pulse->models[19], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(pulse->models[19], HU3D_MOTATTR_PAUSE);
                Hu3DModelAttrSet(pulse->models[19], HU3D_MOTATTR_LOOP);
                modelData = Hu3DData[pulse->models[19]].hsf;
                material = modelData->material;
                materialCount = modelData->materialNum;
                for (index = 0; index < materialCount; index++, material++) {
                    material->invAlpha = 1.0f;
                }
            }
            HuAudFXPlay(MSM_SE_M619_6E8);
            pulse->pulseFrame = 0;
            pulse->pulseState++;
        }
        break;
    case 2:
        pulse->pulseFrame++;
        scale = (f32)pulse->pulseFrame / 120.0f;
        angle = (f32)cos((3.141592653589793 *
                          (f64)(30.0f +
                                (60.0f * scale))) /
                         180.0);
        if (pulse->nightLighting != 0) {
            Hu3DModelScaleSet(pulse->models[27],
                              angle, angle, angle);
            Hu3DModelScaleSet(pulse->models[28],
                              angle, angle, angle);
        } else {
            Hu3DModelScaleSet(pulse->models[0],
                              angle, angle, angle);
            Hu3DModelScaleSet(pulse->models[1],
                              angle, angle, angle);
        }
        if (pulse->nightLighting != 0) {
            modelData = Hu3DData[pulse->models[27]].hsf;
        } else {
            modelData = Hu3DData[pulse->models[0]].hsf;
        }
        material = modelData->material;
        materialCount = modelData->materialNum;
        for (index = 0; index < materialCount; index++, material++) {
            material->invAlpha = scale +
                                 (pulse->materialTransparency[0][index] *
                                  (1.0f - scale));
        }
        if (pulse->nightLighting != 0) {
            modelData = Hu3DData[pulse->models[28]].hsf;
        } else {
            modelData = Hu3DData[pulse->models[1]].hsf;
        }
        material = modelData->material;
        materialCount = modelData->materialNum;
        for (index = 0; index < materialCount; index++, material++) {
            material->invAlpha = scale +
                                 (pulse->materialTransparency[1][index] *
                                  (1.0f - scale));
        }
        if (pulse->nightLighting != 0) {
            modelData = Hu3DData[pulse->models[31]].hsf;
        } else {
            modelData = Hu3DData[pulse->models[19]].hsf;
        }
        material = modelData->material;
        materialCount = modelData->materialNum;
        for (index = 0; index < materialCount; index++, material++) {
            material->invAlpha = (1.0f - scale) +
                                 (pulse->materialTransparency[2][index] * scale);
        }
        if (pulse->pulseFrame >= 120) {
            if (pulse->nightLighting != 0) {
                Hu3DModelAttrSet(pulse->models[27], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrSet(pulse->models[28], HU3D_ATTR_DISPOFF);
            } else {
                Hu3DModelAttrSet(pulse->models[0], HU3D_ATTR_DISPOFF);
                Hu3DModelAttrSet(pulse->models[1], HU3D_ATTR_DISPOFF);
            }
            pulse->finishFlag = 0;
            finished = 1;
        }
        break;
    }
    return finished;
}

/* Advances the second model pair's pulse and reports when its sequence is complete. */
s32 fn_1_84D8(void *scene) {
    M619Scene *pulse;
    HSF_DATA *modelData;
    HSF_MATERIAL *material;
    f32 scale;
    f32 angle;
    s32 index;
    s32 finished;
    s32 materialCount;

    finished = 0;
    pulse = scene;
    switch (pulse->pulseState) {
    case 0:
        if (pulse->nightLighting != 0) {
            Hu3DModelAttrReset(pulse->models[27], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(pulse->models[28], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrReset(pulse->models[0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(pulse->models[1], HU3D_ATTR_DISPOFF);
        }
        pulse->pulseFrame = 0;
        pulse->pulseState++;
        /* fall through */
    case 1:
        pulse->pulseFrame++;
        scale = (f32)pulse->pulseFrame / 120.0f;
        angle = (f32)sin((3.141592653589793 *
                          (f64)(60.0f * scale)) /
                          180.0);
        if (pulse->nightLighting != 0) {
            Hu3DModelScaleSet(pulse->models[27],
                              angle, angle, angle);
            Hu3DModelScaleSet(pulse->models[28],
                              angle, angle, angle);
        } else {
            Hu3DModelScaleSet(pulse->models[0], angle, angle, angle);
            Hu3DModelScaleSet(pulse->models[1], angle, angle, angle);
        }
        if (pulse->nightLighting != 0) {
            modelData = Hu3DData[pulse->models[27]].hsf;
        } else {
            modelData = Hu3DData[pulse->models[0]].hsf;
        }
        material = modelData->material;
        materialCount = modelData->materialNum;
        for (index = 0; index < materialCount; index++, material++) {
            material->invAlpha = (1.0f - scale) +
                                 pulse->materialTransparency[0][index] * scale;
        }
        if (pulse->nightLighting != 0) {
            modelData = Hu3DData[pulse->models[28]].hsf;
        } else {
            modelData = Hu3DData[pulse->models[1]].hsf;
        }
        material = modelData->material;
        materialCount = modelData->materialNum;
        for (index = 0; index < materialCount; index++, material++) {
            material->invAlpha = (1.0f - scale) +
                                 pulse->materialTransparency[1][index] * scale;
        }
        if (pulse->pulseFrame == 30) {
            HuAudFXPlay(MSM_SE_M619_6DF);
        }
        if (pulse->pulseFrame >= 120) {
            pulse->pulseFrame = 0;
            pulse->pulseState++;
        }
        break;
    case 2:
        pulse->pulseFrame++;
        scale = (f32)pulse->pulseFrame / 60.0f;
        angle = (f32)sin((3.141592653589793 *
                          (f64)(60.0f + (30.0f * scale))) / 180.0);
        if (pulse->nightLighting != 0) {
            Hu3DModelScaleSet(pulse->models[27],
                              angle, angle, angle);
            Hu3DModelScaleSet(pulse->models[28],
                              angle, angle, angle);
        } else {
            Hu3DModelScaleSet(pulse->models[0], angle, angle, angle);
            Hu3DModelScaleSet(pulse->models[1], angle, angle, angle);
        }
        if (pulse->pulseFrame >= 60) {
            HuAudFXPlay(MSM_SE_M619_6E0);
            pulse->finishFlag = 1;
            finished = 1;
        }
        break;
    }
    return finished;
}

/* Transition callback: lowers the scene model and triggers its three timed particle effects. */
s32 fn_1_88F0(void *animationState) {
    M619SceneAnimation *motion;
    f32 progress;
    f32 wave;

    motion = animationState;
    motion->effectFrame++;
    progress = (f32)motion->effectFrame / 60.0f;
    wave = sin(0.5 * (3.141592653589793 * (f64)progress));
    motion->height = -400.0f * (1.0f - wave)
                      - 100.0f * wave;
    if (motion->effectFrame == 20) {
        fn_1_C890(&motion->effects[0]);
    }
    if (motion->effectFrame == 40) {
        fn_1_C890(&motion->effects[1]);
    }
    if (motion->effectFrame == 60) {
        fn_1_C890(&motion->effects[2]);
    }
    if (motion->effectFrame >= 60) {
        motion->effectFrame = 0;
        return 1;
    }
    return 0;
}

/* Transition callback: raises and shrinks the model, then hides it after the effects start. */
s32 fn_1_8A3C(void *animationState) {
    M619SceneAnimation *motion;
    f32 progress;
    f32 wave;

    motion = animationState;
    motion->effectFrame++;
    progress = (f32)motion->effectFrame / 45.0f;
    wave = (f32)(1.0 +
                 sin(-1.5707963267948966 +
                     (0.5 *
                      (3.141592653589793 * (f64)progress))));
    motion->height = (f32)((-100.0f * (1.0f - wave)) -
                              (400.0f * wave));
    motion->scale = (f32)((1.0f - progress) +
                               (0.75f * progress));
    if (motion->effectFrame == 5) {
        fn_1_C8CC(&motion->effects[0]);
        fn_1_C8CC(&motion->effects[1]);
        fn_1_C8CC(&motion->effects[2]);
    }
    if (motion->effectFrame >= 45) {
        Hu3DModelAttrSet(motion->model, HU3D_ATTR_DISPOFF);
        motion->effectFrame = 0;
        return 1;
    }
    return 0;
}

/* Opening setup: creates or links the launcher models and hooks the four characters to it. */
void fn_1_8BC4(M619Launcher *work, M619Scene *assets) {
    HU3D_MODELID model;

    if (assets->models[21] < 0) {
        model = assets->models[21] = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_28[21], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(assets->models[21]);
    }
    work->positionModel = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 0);
    work->rotationOffsetB = -15.0f;
    work->motionRadius = 600.0f;
    work->position.x =
        (f32)((f64)work->motionRadius *
              sin((3.141592653589793 * (f64)work->rotationOffsetB) / 180.0));
    work->position.y = 0.0f;
    work->position.z =
        (f32)((f64)work->motionRadius *
              cos((3.141592653589793 * (f64)work->rotationOffsetB) / 180.0));
    work->rotation.x = 0.0f;
    work->rotation.y = 0.0f;
    work->rotation.z = 0.0f;
    Hu3DModelPosSet(model, work->position.x, 0.0f,
                    work->position.z);
    Hu3DModelRotSet(model, 0.0f, 180.0f + work->rotationOffsetB, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);

    if (assets->models[22] < 0) {
        model = assets->models[22] = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_28[22], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(assets->models[22]);
    }
    work->transitions[0].modelA = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 2000.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.5f);
    work->transitions[0].phase = 0;
    work->transitions[0].position.x = 600.0f;
    work->transitions[0].position.y = 0.0f;
    work->transitions[0].position.z = -560.0f;
    work->transitions[0].audioPan = 0;
    fn_1_D420(&work->transitions[0].flash, NULL, NULL);

    if (assets->models[23] < 0) {
        model = assets->models[23] = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_28[23], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(assets->models[23]);
    }
    work->transitions[1].modelA = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 2000.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    work->transitions[1].phase = 0;
    work->transitions[1].position.x = -500.0f;
    work->transitions[1].position.y = 0.0f;
    work->transitions[1].position.z = 780.0f;
    work->transitions[1].audioPan = 1;
    fn_1_D420(&work->transitions[1].flash, NULL, NULL);

    Hu3DModelHookSet(work->positionModel, "db01",
                     assets->players[0]->characterModel);
    Hu3DModelHookSet(work->positionModel, "db02",
                     assets->players[1]->characterModel);
    Hu3DModelHookSet(work->positionModel, "db03",
                     assets->players[2]->characterModel);
    Hu3DModelHookSet(work->positionModel, "db04",
                     assets->players[3]->characterModel);
    work->motionPhase = 0.0f;
    work->rotationOffsetA = 0.0f;
    work->positionYOffset = 0.0f;
    work->positionYBase = 0.0f;
    work->positionZBase = 0.0f;
    work->positionZOffset = 0.0f;

    if (assets->models[24] < 0) {
        model = assets->models[24] = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_28[24], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(assets->models[24]);
    }
    work->rotationModel = model;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.5f);

    if (assets->models[25] < 0) {
        model = assets->models[25] = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_28[25], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(assets->models[25]);
    }
    work->transitions[0].modelB = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.5f);

    if (assets->models[26] < 0) {
        model = assets->models[26] = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_28[26], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(assets->models[26]);
    }
    work->transitions[1].modelB = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.5f);
}

/* Sets launcher turn and radius along the opening arc for the supplied phase. */
void fn_1_9320(M619Launcher *motion, f32 inputPhase) {
    f32 wave;

    wave = sin(0.5 * (3.141592653589793 * (f64)inputPhase));
    motion->rotationOffsetB = -15.0f * (1.0f - wave)
                           - 45.0f * wave;
    motion->motionRadius = 600.0f * (1.0f - wave);
    motion->position.x = (f64)motion->motionRadius
                       * sin((3.141592653589793 * (f64)motion->rotationOffsetB)
                             / 180.0);
    motion->positionZOffset = (f64)motion->motionRadius
                            * cos((3.141592653589793 * (f64)motion->rotationOffsetB)
                                  / 180.0);
}

/* Called each launcher update to add a small vertical and roll oscillation. */
void fn_1_9454(M619Launcher *launcher) {
    f32 sineValue;

    launcher->motionPhase += 5.0f;
    while (launcher->motionPhase > 180.0f) {
        launcher->motionPhase = (f32) (launcher->motionPhase - 360.0f);
    }
    sineValue = (f32) sin((3.141592653589793 * (f64) launcher->motionPhase) / 180.0);
    launcher->positionYOffset = (f32) (10.0f * sineValue);
    launcher->rotation.z = (f32) (3.0f * sineValue);
}

/* Applies the launcher's current position and rotation to both linked models. */
void fn_1_952C(M619Launcher *motion) {
    motion->rotation.y = 180.0f + motion->rotationOffsetB + motion->rotationOffsetA;
    motion->position.y = motion->positionYOffset + motion->positionYBase;
    motion->position.z = motion->positionZOffset + motion->positionZBase;
    while (motion->rotation.y > 180.0f) {
        motion->rotation.y -= 360.0f;
    }
    while (motion->rotation.y < -180.0f) {
        motion->rotation.y += 360.0f;
    }
    Hu3DModelPosSetV(motion->positionModel, &motion->position);
    Hu3DModelRotSetV(motion->positionModel, &motion->rotation);
    Hu3DModelPosSetV(motion->rotationModel, &motion->position);
    Hu3DModelRotSetV(motion->rotationModel, &motion->rotation);
}

/* Applies the phase-scaled launcher turn and keeps its angle in the signed range. */
void fn_1_962C(M619Launcher *launcher, f32 progress) {
    launcher->rotationOffsetA -= 2.0f * progress;
    while (launcher->rotationOffsetA < -180.0f) {
        launcher->rotationOffsetA += 360.0f;
    }
}

/* Sets the launcher rise and forward offset, and advances its position motion by speed. */
void fn_1_9680(M619Launcher *launcher, f32 speed) {
    launcher->positionYBase = 1200.0 * sin(1.5707963267948966 * speed);
    Hu3DMotionTimeSet(launcher->positionModel, 60.0f * speed);
    launcher->positionZBase = 500.0f * speed;
}

/* Positions the four players around the minigame center before the action begins. */
void fn_1_9714(M619Scene *scene) {
    M619AngleTableView angleTable = {{-135.0f, 135.0f, -45.0f, 45.0f}};
    f32 angle;
    s32 playerNumber;
    M619Player *player;
    for (playerNumber = 0; playerNumber < 4; playerNumber++) {
        player = scene->players[playerNumber];
        angle = 225.0f +
                -90.0f * playerNumber;
        angle = angleTable.angles[playerNumber];
        player->position.x =
            (f32)(500.0 *
                  sin((3.141592653589793 * (f64)angle) /
                      180.0));
        player->position.y = 1000.0f;
        player->position.z =
            (f32)(500.0 *
                  cos((3.141592653589793 * (f64)angle) /
                      180.0));
        player->rotation.x = 0.0f;
        player->rotation.y = 90.0f + angle;
        while (player->rotation.y > 180.0f) {
            player->rotation.y -= 360.0f;
        }
        player->targetAngle = player->rotation.y;
        player->rotation.z = 0.0f;
        fn_1_C628(player);
    }
}

/* Action callback: drops the players into position, then starts their sweep motion. */
s32 fn_1_98F0(void *scene) {
    M619Scene *state;
    s32 result;
    s32 playerNumber;
    f32 motionHeight;
    f32 progress;
    M619Player *player;
    HuVecF startPosition;
    HuVecF *position;

    result = 0;
    state = (M619Scene *)scene;
    switch (state->actionPhase) {
    case 0:
        state->actionTimer = 0;
        state->actionPhase++;
        /* fall through */
    case 1:
        state->actionTimer++;
        progress = (f32)state->actionTimer / 30.0f;
        motionHeight = (f32)(800.0 *
                             cos(1.5707963267948966 * (f64)progress));
        for (playerNumber = 0; playerNumber < 4; playerNumber++) {
            player = state->players[playerNumber];
            player->position.y = motionHeight;
            fn_1_C628(player);
        }
        if (state->actionTimer >= 30) {
            for (playerNumber = 0; playerNumber < 4; playerNumber++) {
                player = state->players[playerNumber];
                startPosition = player->position;
                position = &startPosition;
                fn_1_A124(&player->modelMotion,
                          position);
                omVibrate(player->playerNumber, 20, 7, 3);
            }
            state->actionTimer = 0;
            state->actionPhase++;
        }
        break;
    case 2:
        for (playerNumber = 0; playerNumber < 4; playerNumber++) {
            player = state->players[playerNumber];
            CharMotionShiftSet(player->character, player->motions[1],
                               0.0f, 15.0f,
                                HU3D_MOTATTR_LOOP);
            player->modelQueue.flags = 1;
            Hu3DModelAttrReset(player->sway.model, HU3D_ATTR_DISPOFF);
            fn_1_B3FC(&player->sway, 0);
            fn_1_E508((M619MotionNode *)&player->collision, 65.0f, 0);
            fn_1_E43C(&player->collision, player, fn_1_B678, fn_1_BADC, fn_1_BB4C,
                      &player->position);
            player->turnFrames = 60;
            if (playerNumber % 2 == 0) {
                HuAudFXPlayPan(MSM_SE_M619_6E4, 32);
            } else {
                HuAudFXPlayPan(MSM_SE_M619_6E4, 96);
            }
        }
        state->actionPhase++;
        /* fall through */
    case 3:
        state->actionTimer++;
        progress = (f32)state->actionTimer / 30.0f;
        motionHeight = (f32)((f64)(-50.0f *
                                   (1.0f - progress)) *
                             sin(6.283185307179586 * (f64)progress));
        for (playerNumber = 0; playerNumber < 4; playerNumber++) {
            player = state->players[playerNumber];
            player->position.y = motionHeight;
            fn_1_C628(player);
        }
        if (state->actionTimer >= 30) {
            result = 1;
        }
        break;
    }
    return result;
}

/* Runs from a transition callback to drop, flash and move one launcher model. */
s32 fn_1_9C44(void *transitionData) {
    HuVecF position;
    f32 angle;
    f32 positionOffsetY;
    s32 finished;
    HuVecF *positionPointer;
    M619ModelTransition *transition;

    finished = 0;
    transition = transitionData;
    switch (transition->phase) {
    case 0:
        Hu3DModelAttrReset(transition->modelA, HU3D_ATTR_DISPOFF);
        transition->frame = 0;
        transition->phase++;
        /* fall through */
    case 1:
        transition->frame++;
        angle = transition->frame / 30.0f;
        positionOffsetY =
            1200.0 * cos(1.5707963267948966 * (f64)angle);
        Hu3DModelPosSet(transition->modelA, 0.0f,
                        positionOffsetY,
                        0.0f);
        if (transition->frame >= 30) {
            position = transition->position;
            positionPointer = &position;
            fn_1_C8D8(&transition->flash, positionPointer);
            fn_1_C890(&transition->flash);
            Hu3DModelAttrReset(transition->modelB, HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(transition->modelB, HU3D_MOTATTR_LOOP);
            Hu3DModelAttrReset(transition->modelB, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(transition->modelA, HU3D_MOTATTR_PAUSE);
            if (transition->audioPan == 0) {
                HuAudFXPlayPan(MSM_SE_M619_6E3, 32);
            } else {
                HuAudFXPlayPan(MSM_SE_M619_6E3, 96);
            }
            transition->frame = 0;
            transition->phase++;
        }
        break;
    case 2:
        transition->frame++;
        angle = transition->frame / 480.0f;
        positionOffsetY =
            (-50.0f * (1.0f - angle)) *
            sin(25.132741228718345 * (f64)angle);
        Hu3DModelPosSet(
            transition->modelA, 0.0f, positionOffsetY,
            0.0f);
        if (transition->frame >= 480) {
            finished = 1;
        }
        break;
    }
    return finished;
}

/* Creates or links the character's effect model and initializes its particle effect. */
void fn_1_9EEC(M619ModelMotion *work, s32 characterIndex) {
    HU3D_MODELID model;
    M619Scene *modelState;

    modelState = (M619Scene *)lbl_1_bss_0->data;
    if (characterIndex == 0) {
        if (modelState->models[8] < 0) {
            model = (modelState->models[8] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[8], HU_MEMNUM_OVL,
                                     HEAP_MODEL)));
        } else {
            model = Hu3DModelLink(modelState->models[8]);
        }
    } else if (modelState->models[9] < 0) {
        model = (modelState->models[9] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_28[9], HU_MEMNUM_OVL,
                                 HEAP_MODEL)));
    } else {
        model = Hu3DModelLink(modelState->models[9]);
    }
    work->model = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 7);
    Hu3DModelPosSet(model, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 0.5f);
    if (characterIndex == 0) {
        Hu3DModelScaleSet(model, 1.0f, 1.0f,
                          1.0f);
    } else {
        Hu3DModelScaleSet(model, 3.0f, 1.5f,
                          3.0f);
    }
    work->position.x = 0.0f;
    work->position.y = 0.0f;
    work->position.z = 0.0f;
    if (characterIndex == 0) {
        fn_1_CE20(&work->effect, NULL, NULL);
        return;
    }
    fn_1_D420(&work->effect, NULL, NULL);
}

/* Starts a registered model motion at the supplied horizontal position and emits its effect. */
void fn_1_A124(M619ModelMotion *object, const HuVecF *position) {
    HuVecF previousPosition;
    HuVecF *previousPositionPointer;

    if (fn_1_E5B8(fn_1_A1F4, object) != 0U) {
        object->position.x = position->x;
        object->position.z = position->z;
        Hu3DModelAttrReset(object->model, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrReset(object->model, HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(object->model, &object->position);
        Hu3DMotionTimeSet(object->model, 0.0f);
        previousPosition = object->position;
        previousPositionPointer = &previousPosition;
        fn_1_C8D8(&object->effect, previousPositionPointer);
        fn_1_C890(&object->effect);
    }
}
